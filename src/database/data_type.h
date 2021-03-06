//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
//      (c) Copyright 2019-2020 by Andrettin
//
//      Permission is hereby granted, free of charge, to any person obtaining a
//      copy of this software and associated documentation files (the
//      "Software"), to deal in the Software without restriction, including
//      without limitation the rights to use, copy, modify, merge, publish,
//      distribute, sublicense, and/or sell copies of the Software, and to
//      permit persons to whom the Software is furnished to do so, subject to
//      the following conditions:
//
//      The above copyright notice and this permission notice shall be included
//      in all copies or substantial portions of the Software.
//
//      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//      OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//      MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//      IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//      CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//      TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//      SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include "database/data_type_metadata.h"
#include "database/database.h"
#include "database/module_container.h"
#include "database/sml_data.h"
#include "database/sml_operator.h"
#include "util/qunique_ptr.h"

namespace wyrmgus {

class module;

class data_type_base
{
public:
	static inline const std::set<std::string> database_dependencies; //the other classes on which this one depends, i.e. after which this class' database can be processed
};

template <typename T>
class data_type : public data_type_base
{
public:
	static T *get(const std::string &identifier)
	{
		if (identifier == "none") {
			return nullptr;
		}

		T *instance = T::try_get(identifier);

		if (instance == nullptr) {
			throw std::runtime_error("Invalid " + std::string(T::class_identifier) + " instance: \"" + identifier + "\".");
		}

		return instance;
	}

	static T *try_get(const std::string &identifier)
	{
		if (identifier == "none") {
			return nullptr;
		}

		auto find_iterator = data_type::instances_by_identifier.find(identifier);
		if (find_iterator != data_type::instances_by_identifier.end()) {
			return find_iterator->second.get();
		}

		auto alias_find_iterator = data_type::instances_by_alias.find(identifier);
		if (alias_find_iterator != data_type::instances_by_alias.end()) {
			return alias_find_iterator->second;
		}

		return nullptr;
	}

	static T *get_or_add(const std::string &identifier, const module *module)
	{
		T *instance = T::try_get(identifier);
		if (instance != nullptr) {
			return instance;
		}

		return T::add(identifier, module);
	}

	static const std::vector<T *> &get_all()
	{
		return data_type::instances;
	}

	static bool exists(const std::string &identifier)
	{
		return data_type::instances_by_identifier.contains(identifier) || data_type::instances_by_alias.contains(identifier);
	}

	static T *add(const std::string &identifier, const module *module)
	{
		if (identifier.empty()) {
			throw std::runtime_error("Tried to add a " + std::string(T::class_identifier) + " instance with an empty string identifier.");
		}

		if (T::exists(identifier)) {
			throw std::runtime_error("Tried to add a " + std::string(T::class_identifier) + " instance with the already-used \"" + identifier + "\" string identifier.");
		}

		data_type::instances_by_identifier[identifier] = make_qunique<T>(identifier);

		T *instance = data_type::instances_by_identifier.find(identifier)->second.get();
		data_type::instances.push_back(instance);
		instance->moveToThread(QApplication::instance()->thread());
		instance->set_module(module);

		//for backwards compatibility, change instances of "_" in the identifier with "-" and add that as an alias, and do the opposite as well
		if (identifier.find("_") != std::string::npos) {
			std::string alias = identifier;
			std::replace(alias.begin(), alias.end(), '_', '-');
			T::add_instance_alias(instance, alias);
		} else if (identifier.find("-") != std::string::npos) {
			std::string alias = identifier;
			std::replace(alias.begin(), alias.end(), '-', '_');
			T::add_instance_alias(instance, alias);
		}

		return instance;
	}

	static void add_instance_alias(T *instance, const std::string &alias)
	{
		if (alias.empty()) {
			throw std::runtime_error("Tried to add a " + std::string(T::class_identifier) + " instance empty alias.");
		}

		if (T::exists(alias)) {
			throw std::runtime_error("Tried to add a " + std::string(T::class_identifier) + " alias with the already-used \"" + alias + "\" string identifier.");
		}

		data_type::instances_by_alias[alias] = instance;
		instance->add_alias(alias);
	}

	static void remove(T *instance)
	{
		data_type::instances.erase(std::remove(data_type::instances.begin(), data_type::instances.end(), instance), data_type::instances.end());

		data_type::instances_by_identifier.erase(instance->get_identifier());
	}

	static void remove(const std::string &identifier)
	{
		T::remove(T::get(identifier));
	}

	static void clear()
	{
		data_type::instances.clear();
		data_type::instances_by_alias.clear();
		data_type::instances_by_identifier.clear();
	}

	template <typename function_type>
	static void sort_instances(const function_type &function)
	{
		std::sort(data_type::instances.begin(), data_type::instances.end(), function);
	}

	static void parse_database(const std::filesystem::path &data_path, const module *module)
	{
		if (std::string(T::database_folder).empty()) {
			return;
		}

		const std::filesystem::path database_path(data_path / T::database_folder);

		if (!std::filesystem::exists(database_path)) {
			return;
		}

		database::parse_folder(database_path, data_type::sml_data_to_process[module]);
	}

	static void process_database(const bool definition)
	{
		if (std::string(T::database_folder).empty()) {
			return;
		}

		for (const auto &kv_pair : data_type::sml_data_to_process) {
			const module *module = kv_pair.first;
			const std::vector<sml_data> &sml_data_list = kv_pair.second;
			for (const sml_data &data : sml_data_list) {
				data.for_each_child([&](const sml_data &data_entry) {
					const std::string &identifier = data_entry.get_tag();

					T *instance = nullptr;
					if (definition) {
						if (data_entry.get_operator() != sml_operator::addition) { //addition operators for data entry scopes mean modifying already-defined entries
							instance = T::add(identifier, module);
						} else {
							instance = T::get(identifier);
						}

						for (const sml_property *alias_property : data_entry.try_get_properties("aliases")) {
							if (alias_property->get_operator() != sml_operator::addition) {
								throw std::runtime_error("Only the addition operator is supported for data entry aliases.");
							}

							const std::string &alias = alias_property->get_value();
							T::add_instance_alias(instance, alias);
						}
					} else {
						try {
							instance = T::get(identifier);
							database::process_sml_data<T>(instance, data_entry);
							instance->set_defined(true);
						} catch (...) {
							std::throw_with_nested(std::runtime_error("Error processing or loading data for " + std::string(T::class_identifier) + " instance \"" + identifier + "\"."));
						}
					}
				});
			}
		}

		if (!definition) {
			data_type::sml_data_to_process.clear();
		}
	}

	static void load_history_database()
	{
		for (T *instance : T::get_all()) {
			instance->load_history();
		}
	}

	static void initialize_all()
	{
		for (T *instance : T::get_all()) {
			if (instance->is_initialized()) {
				continue; //the instance might have been initialized already, e.g. in the initialization function of another instance which needs it to be initialized
			}

			try {
				instance->initialize();
			} catch (...) {
				std::throw_with_nested(std::runtime_error("Failed to initialize the " + std::string(T::class_identifier) + " instance \"" + instance->get_identifier() + "\"."));
			}
		}
	}

	static void check_all()
	{
		for (const T *instance : T::get_all()) {
			try {
				instance->check();
			} catch (...) {
				std::throw_with_nested(std::runtime_error("The validity check for the " + std::string(T::class_identifier) + " instance \"" + instance->get_identifier() + "\" failed."));
			}
		}
	}

private:
	static inline bool initialize_class()
	{
		//initialize the metadata (including database parsing/processing functions) for this data type
		auto metadata = std::make_unique<data_type_metadata>(T::class_identifier, T::database_dependencies, T::parse_database, T::process_database, T::initialize_all, T::check_all, T::clear);
		database::get()->register_metadata(std::move(metadata));

		return true;
	}

	static inline std::vector<T *> instances;
	static inline std::map<std::string, qunique_ptr<T>> instances_by_identifier;
	static inline std::map<std::string, T *> instances_by_alias;
	static inline module_map<std::vector<sml_data>> sml_data_to_process;
#ifdef __GNUC__
	//the "used" attribute is needed under GCC, or else this variable will be optimized away (even in debug builds)
	static inline bool class_initialized [[gnu::used]] = data_type::initialize_class();
#else
	static inline bool class_initialized = data_type::initialize_class();
#endif
};

}
