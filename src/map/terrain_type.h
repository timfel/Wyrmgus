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
//      (c) Copyright 2018-2020 by Andrettin
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//

#pragma once

#include "color.h"
#include "database/data_type.h"
#include "database/named_data_entry.h"
#include "data_type.h"
#include "util/color_container.h"

class CGraphic;
class CPlayerColorGraphic;
struct lua_State;

int CclDefineTerrainType(lua_State *l);

namespace stratagus {

class resource;
class season;
class unit_type;
enum class tile_transition_type;

class terrain_type : public named_data_entry, public data_type<terrain_type>, public CDataType
{
	Q_OBJECT

	Q_PROPERTY(QColor color READ get_color WRITE set_color)
	Q_PROPERTY(QString image_file READ get_image_file_qstring)
	Q_PROPERTY(QString transition_image_file READ get_transition_image_file_qstring)
	Q_PROPERTY(QString elevation_image_file READ get_elevation_image_file_qstring)
	Q_PROPERTY(bool overlay MEMBER overlay READ is_overlay)
	Q_PROPERTY(bool buildable MEMBER buildable READ is_buildable)
	Q_PROPERTY(bool tiled_background MEMBER tiled_background READ has_tiled_background)
	Q_PROPERTY(bool transition_mask MEMBER transition_mask READ has_transition_mask)
	Q_PROPERTY(bool allow_single MEMBER allow_single READ allows_single)
	Q_PROPERTY(bool hidden MEMBER hidden READ is_hidden)
	Q_PROPERTY(stratagus::resource* resource MEMBER resource READ get_resource)
	Q_PROPERTY(QVariantList base_terrain_types READ get_base_terrain_types_qvariant_list)
	Q_PROPERTY(QVariantList outer_border_terrain_types READ get_outer_border_terrain_types_qvariant_list)
	Q_PROPERTY(QVariantList inner_border_terrain_types READ get_inner_border_terrain_types_qvariant_list)

public:
	static constexpr const char *class_identifier = "terrain_type";
	static constexpr const char *database_folder = "terrain_types";

	static terrain_type *get_by_character(const char character)
	{
		terrain_type *terrain_type = terrain_type::try_get_by_character(character);

		if (terrain_type == nullptr) {
			throw std::runtime_error("No terrain type found for character: " + std::string(1, character) + ".");
		}

		return terrain_type;
	}

	static terrain_type *try_get_by_character(const char character)
	{
		auto find_iterator = terrain_type::terrain_types_by_character.find(character);
		if (find_iterator != terrain_type::terrain_types_by_character.end()) {
			return find_iterator->second;
		}

		return nullptr;
	}

	static terrain_type *get_by_color(const QColor &color)
	{
		terrain_type *terrain_type = terrain_type::try_get_by_color(color);

		if (terrain_type == nullptr) {
			throw std::runtime_error("No terrain type found for color: (" + std::to_string(color.red()) + ", " + std::to_string(color.green()) + ", " + std::to_string(color.blue()) + ").");
		}

		return terrain_type;
	}

	static terrain_type *try_get_by_color(const QColor &color)
	{
		auto find_iterator = terrain_type::terrain_types_by_color.find(color);
		if (find_iterator != terrain_type::terrain_types_by_color.end()) {
			return find_iterator->second;
		}

		return nullptr;
	}

	static terrain_type *get_by_tile_number(const int tile_number)
	{
		terrain_type *terrain_type = terrain_type::try_get_by_tile_number(tile_number);

		if (terrain_type == nullptr) {
			throw std::runtime_error("No terrain type found for tile number: " + std::to_string(tile_number) + ".");
		}

		return terrain_type;
	}

	static terrain_type *try_get_by_tile_number(const int tile_number)
	{
		auto find_iterator = terrain_type::terrain_types_by_tile_number.find(tile_number);
		if (find_iterator != terrain_type::terrain_types_by_tile_number.end()) {
			return find_iterator->second;
		}

		return nullptr;
	}

	static terrain_type *add(const std::string &identifier, const stratagus::module *module)
	{
		terrain_type *terrain_type = data_type::add(identifier, module);
		terrain_type->ID = terrain_type::get_all().size() - 1;
		return terrain_type;
	}

	static void clear()
	{
		data_type::clear();

		terrain_type::terrain_types_by_character.clear();
		terrain_type::terrain_types_by_color.clear();
		terrain_type::terrain_types_by_tile_number.clear();
	}

	terrain_type(const std::string &identifier) : named_data_entry(identifier), CDataType(identifier)
	{
	}
	
	~terrain_type();
	
	static void LoadTerrainTypeGraphics();
	static unsigned long GetTerrainFlagByName(const std::string &flag_name);
	
private:
	static inline std::map<char, terrain_type *> terrain_types_by_character;
	static inline color_map<terrain_type *> terrain_types_by_color;
	static inline std::map<int, terrain_type *> terrain_types_by_tile_number;

public:
	virtual void process_sml_property(const sml_property &property) override;
	virtual void process_sml_scope(const sml_data &scope) override;
	virtual void ProcessConfigData(const CConfigData *config_data) override;
	virtual void initialize() override;

	char get_character() const
	{
		return this->character;
	}

	void set_character(const char character);
	void map_to_character(const char character);

	const QColor &get_color() const
	{
		return this->color;
	}

	void set_color(const QColor &color);

	void map_to_tile_number(const int tile_number);

	const std::filesystem::path &get_image_file() const
	{
		return this->image_file;
	}

	void set_image_file(const std::filesystem::path &filepath);

	QString get_image_file_qstring() const
	{
		return QString::fromStdString(this->get_image_file().string());
	}

	Q_INVOKABLE void set_image_file(const std::string &filepath)
	{
		this->set_image_file(std::filesystem::path(filepath));
	}

	CPlayerColorGraphic *get_graphics(const season *season = nullptr) const;

	const std::filesystem::path &get_transition_image_file() const
	{
		return this->transition_image_file;
	}

	void set_transition_image_file(const std::filesystem::path &filepath);

	QString get_transition_image_file_qstring() const
	{
		return QString::fromStdString(this->get_transition_image_file().string());
	}

	Q_INVOKABLE void set_transition_image_file(const std::string &filepath)
	{
		this->set_transition_image_file(std::filesystem::path(filepath));
	}

	CPlayerColorGraphic *get_transition_graphics(const season *season) const
	{
		if (this->transition_graphics != nullptr) {
			return this->transition_graphics;
		}

		return this->get_graphics(season);
	}

	bool has_transition_mask() const
	{
		return this->transition_mask;
	}

	const std::filesystem::path &get_elevation_image_file() const
	{
		return this->elevation_image_file;
	}

	void set_elevation_image_file(const std::filesystem::path &filepath);

	QString get_elevation_image_file_qstring() const
	{
		return QString::fromStdString(this->get_elevation_image_file().string());
	}

	Q_INVOKABLE void set_elevation_image_file(const std::string &filepath)
	{
		this->set_elevation_image_file(std::filesystem::path(filepath));
	}

	CGraphic *get_elevation_graphics() const
	{
		return this->elevation_graphics;
	}

	bool is_overlay() const
	{
		return this->overlay;
	}

	bool is_buildable() const
	{
		return this->buildable;
	}

	bool has_tiled_background() const
	{
		return this->tiled_background;
	}

	bool allows_single() const
	{
		return this->allow_single;
	}

	bool is_hidden() const
	{
		return this->hidden;
	}

	resource *get_resource() const
	{
		return this->resource;
	}

	const std::vector<terrain_type *> &get_base_terrain_types() const
	{
		return this->base_terrain_types;
	}

	QVariantList get_base_terrain_types_qvariant_list() const;

	Q_INVOKABLE void add_base_terrain_type(terrain_type *terrain_type)
	{
		this->base_terrain_types.push_back(terrain_type);
	}

	Q_INVOKABLE void remove_base_terrain_type(terrain_type *terrain_type);

	const std::vector<terrain_type *> &get_outer_border_terrain_types() const
	{
		return this->outer_border_terrain_types;
	}

	QVariantList get_outer_border_terrain_types_qvariant_list() const;

	Q_INVOKABLE void add_outer_border_terrain_type(terrain_type *terrain_type)
	{
		this->outer_border_terrain_types.push_back(terrain_type);

		this->BorderTerrains.push_back(terrain_type);
		terrain_type->inner_border_terrain_types.push_back(this);
		terrain_type->BorderTerrains.push_back(this);
	}

	Q_INVOKABLE void remove_outer_border_terrain_type(terrain_type *terrain_type);

	const std::vector<terrain_type *> &get_inner_border_terrain_types() const
	{
		return this->inner_border_terrain_types;
	}

	QVariantList get_inner_border_terrain_types_qvariant_list() const;

	Q_INVOKABLE void add_inner_border_terrain_type(terrain_type *terrain_type)
	{
		this->inner_border_terrain_types.push_back(terrain_type);

		this->BorderTerrains.push_back(terrain_type);
		terrain_type->outer_border_terrain_types.push_back(this);
		terrain_type->BorderTerrains.push_back(this);
	}

	Q_INVOKABLE void remove_inner_border_terrain_type(terrain_type *terrain_type);

	const std::vector<int> &get_solid_tiles() const
	{
		return this->solid_tiles;
	}

	const std::vector<int> &get_damaged_tiles() const
	{
		return this->damaged_tiles;
	}

	const std::vector<int> &get_destroyed_tiles() const
	{
		return this->destroyed_tiles;
	}

	const std::vector<int> &get_transition_tiles(const terrain_type *terrain_type, const tile_transition_type transition_type) const
	{
		static std::vector<int> empty_vector;

		auto find_iterator = this->transition_tiles.find(terrain_type);
		if (find_iterator != this->transition_tiles.end()) {
			auto sub_find_iterator = find_iterator->second.find(transition_type);
			if (sub_find_iterator != find_iterator->second.end()) {
				return sub_find_iterator->second;
			}
		}

		return empty_vector;
	}

	const std::vector<int> &get_adjacent_transition_tiles(const terrain_type *terrain_type, const tile_transition_type transition_type) const
	{
		static std::vector<int> empty_vector;

		auto find_iterator = this->adjacent_transition_tiles.find(terrain_type);
		if (find_iterator != this->adjacent_transition_tiles.end()) {
			auto sub_find_iterator = find_iterator->second.find(transition_type);
			if (sub_find_iterator != find_iterator->second.end()) {
				return sub_find_iterator->second;
			}
		}

		return empty_vector;
	}

private:
	char character = 0;
	QColor color;
public:
	int ID = -1;
	int SolidAnimationFrames = 0;
private:
	resource *resource = nullptr;
public:
	unsigned long Flags = 0;
private:
	bool overlay = false;				/// Whether this terrain type belongs to the overlay layer
	bool buildable = false;				/// Whether structures can be built upon this terrain type
	bool tiled_background = false;
	bool transition_mask = false;
	bool allow_single = false;			/// Whether this terrain type has transitions for single tiles
	bool hidden = false;
public:
	unit_type *UnitType = nullptr;
private:
	std::filesystem::path image_file;
	CPlayerColorGraphic *graphics = nullptr;
	std::filesystem::path transition_image_file;
	CPlayerColorGraphic *transition_graphics = nullptr;
	std::map<const season *, std::filesystem::path> season_image_files;
	std::map<const season *, CPlayerColorGraphic *> season_graphics;		/// Graphics to be displayed instead of the normal ones during particular seasons
	std::filesystem::path elevation_image_file;
	CGraphic *elevation_graphics = nullptr; //semi-transparent elevation graphics, displayed on borders so that they look better
private:
	std::vector<terrain_type *> base_terrain_types; //possible base terrain types for this terrain type (if it is an overlay terrain)
public:
	std::vector<terrain_type *> BorderTerrains;				/// Terrain types which this one can border
private:
	std::vector<terrain_type *> outer_border_terrain_types; //terrain types which this one can border, and which are "entered" by this tile type in transitions
	std::vector<terrain_type *> inner_border_terrain_types; //terrain types which this one can border, and which "enter" this tile type in transitions
	std::vector<int> solid_tiles;
	std::vector<int> damaged_tiles;
	std::vector<int> destroyed_tiles;
	std::map<const terrain_type *, std::map<tile_transition_type, std::vector<int>>> transition_tiles;	/// Transition graphics, mapped to the tile type (-1 means any tile) and the transition type (i.e. northeast outer)
	std::map<const terrain_type *, std::map<tile_transition_type, std::vector<int>>> adjacent_transition_tiles;	/// Transition graphics for the tiles adjacent to this terrain type, mapped to the tile type (-1 means any tile) and the transition type (i.e. northeast outer)

	friend int ::CclDefineTerrainType(lua_State *l);
};

}
