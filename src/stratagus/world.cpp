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
//      (c) Copyright 2016-2020 by Andrettin
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

#include "stratagus.h"

#include "world.h"

#include "config.h"
#include "map/site.h"
#include "map/terrain_feature.h"
#include "map/terrain_type.h"
#include "plane.h"
#include "province.h"
#include "species/species.h"
#include "time/season_schedule.h"
#include "time/time_of_day_schedule.h"
#include "ui/ui.h"
#include "util/geojson_util.h"
#include "util/vector_util.h"

namespace wyrmgus {

world *world::add(const std::string &identifier, const wyrmgus::module *module)
{
	world *world = data_type::add(identifier, module);
	world->ID = world::get_all().size() - 1;
	UI.WorldButtons.resize(world::get_all().size());
	UI.WorldButtons[world->ID].X = -1;
	UI.WorldButtons[world->ID].Y = -1;
	return world;
}

world::~world()
{
	for (CProvince *province : this->Provinces) {
		delete province;
	}
}

void world::ProcessConfigData(const CConfigData *config_data)
{
	for (size_t i = 0; i < config_data->Properties.size(); ++i) {
		std::string key = config_data->Properties[i].first;
		std::string value = config_data->Properties[i].second;
		
		if (key == "name") {
			this->set_name(value);
		} else if (key == "description") {
			this->set_description(value);
		} else if (key == "background") {
			this->set_background(value);
		} else if (key == "quote") {
			this->set_quote(value);
		} else if (key == "plane") {
			this->plane = plane::get(value);
		} else if (key == "time_of_day_schedule") {
			value = FindAndReplaceString(value, "_", "-");
			this->TimeOfDaySchedule = CTimeOfDaySchedule::GetTimeOfDaySchedule(value);
		} else if (key == "season_schedule") {
			value = FindAndReplaceString(value, "_", "-");
			this->SeasonSchedule = CSeasonSchedule::GetSeasonSchedule(value);
		} else {
			fprintf(stderr, "Invalid world property: \"%s\".\n", key.c_str());
		}
	}
}

std::vector<QVariantList> world::parse_geojson_folder(const std::string_view &folder) const
{
	std::vector<QVariantList> geojson_data_list;

	for (const std::filesystem::path &path : database::get()->get_maps_paths()) {
		const std::filesystem::path map_path = path / this->get_identifier() / folder;

		if (!std::filesystem::exists(map_path)) {
			continue;
		}

		std::vector<QVariantList> folder_geojson_data_list = geojson::parse_folder(map_path);
		vector::merge(geojson_data_list, std::move(folder_geojson_data_list));
	}

	return geojson_data_list;
}

terrain_geodata_map world::parse_terrain_geojson_folder() const
{
	terrain_geodata_map terrain_data;

	const std::vector<QVariantList> geojson_data_list = this->parse_geojson_folder(world::terrain_map_folder);

	geojson::process_features(geojson_data_list, [&](const QVariantMap &feature) {
		const QVariantMap properties = feature.value("properties").toMap();

		const terrain_type *terrain = nullptr;
		const terrain_feature *terrain_feature = nullptr;

		if (properties.contains("terrain_feature")) {
			const QString terrain_feature_identifier = properties.value("terrain_feature").toString();
			terrain_feature = terrain_feature::get(terrain_feature_identifier.toStdString());
		} else {
			const QString terrain_type_identifier = properties.value("terrain_type").toString();
			terrain = terrain_type::get(terrain_type_identifier.toStdString());
		}

		const QString type_str = feature.value("type").toString();
		for (const QVariant &subfeature_variant : feature.value("data").toList()) {
			const QVariantMap subfeature_map = subfeature_variant.toMap();
			std::unique_ptr<QGeoShape> geoshape;

			if (type_str == "MultiLineString") {
				const QGeoPath geopath = subfeature_map.value("data").value<QGeoPath>();
				auto geopath_copy = std::make_unique<QGeoPath>(geopath);

				if (terrain_feature != nullptr && terrain_feature->get_geopath_width() != 0) {
					geopath_copy->setWidth(terrain_feature->get_geopath_width());
				}

				geoshape = std::move(geopath_copy);
			} else { //MultiPolygon
				const QGeoPolygon geopolygon = subfeature_map.value("data").value<QGeoPolygon>();
				geoshape = std::make_unique<QGeoPolygon>(geopolygon);
			}

			if (terrain_feature != nullptr) {
				terrain_data[terrain_feature].push_back(std::move(geoshape));
			} else {
				terrain_data[terrain].push_back(std::move(geoshape));
			}
		}
	});

	return terrain_data;
}

std::map<const site *, std::vector<std::unique_ptr<QGeoShape>>> world::parse_territories_geojson_folder() const
{
	std::map<const site *, std::vector<std::unique_ptr<QGeoShape>>> territory_data;

	const std::vector<QVariantList> geojson_data_list = this->parse_geojson_folder(world::territories_map_folder);

	geojson::process_features(geojson_data_list, [&](const QVariantMap &feature) {
		const QVariantMap properties = feature.value("properties").toMap();

		const QString settlement_identifier = properties.value("settlement").toString();
		const site *settlement = site::get(settlement_identifier.toStdString());

		for (const QVariant &subfeature_variant : feature.value("data").toList()) {
			const QVariantMap subfeature_map = subfeature_variant.toMap();
			std::unique_ptr<QGeoShape> geoshape;

			const QGeoPolygon geopolygon = subfeature_map.value("data").value<QGeoPolygon>();
			geoshape = std::make_unique<QGeoPolygon>(geopolygon);

			territory_data[settlement].push_back(std::move(geoshape));
		}
	});

	return territory_data;
}

std::vector<const species *> world::get_native_sapient_species() const
{
	std::vector<const species *> sapient_species;
	for (const species *species : this->get_native_species()) {
		if (species->is_sapient()) {
			sapient_species.push_back(species);
		}
	}
	return sapient_species;
}

std::vector<const species *> world::get_native_fauna_species() const
{
	std::vector<const species *> fauna_species;
	for (const species *species : this->get_native_species()) {
		if (!species->is_sapient()) {
			fauna_species.push_back(species);
		}
	}
	return fauna_species;
}

}
