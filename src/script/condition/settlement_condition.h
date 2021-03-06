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
//      (c) Copyright 2020 by Andrettin
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

#include "map/site.h"
#include "script/condition/condition.h"
#include "util/string_util.h"

namespace wyrmgus {

class settlement_condition final : public condition
{
public:
	settlement_condition() {}

	explicit settlement_condition(const std::string &value)
	{
		this->settlement = site::get(value);
	}

	virtual void process_sml_property(const sml_property &property) override
	{
		const std::string &key = property.get_key();
		const std::string &value = property.get_value();

		if (key == "settlement") {
			this->settlement = site::get(value);
		} else if (key == "faction") {
			this->faction = faction::get(value);
		} else if (key == "enemy") {
			this->enemy = string::to_bool(value);
		} else {
			throw std::runtime_error("Invalid settlement condition property: \"" + property.get_key() + "\".");
		}
	}

	virtual bool check(const CPlayer *player, const bool ignore_units) const override
	{
		Q_UNUSED(ignore_units)

		if (this->faction != nullptr) {
			const CPlayer *faction_player = GetFactionPlayer(this->faction);
			if (faction_player == nullptr) {
				return false;
			}

			if (this->enemy && !faction_player->IsEnemy(*player)) {
				return false;
			}

			return faction_player->HasSettlement(this->settlement);
		}

		return player->HasSettlement(this->settlement);
	}

	virtual std::string get_string(const size_t indent) const override
	{
		Q_UNUSED(indent)

		return this->settlement->get_name() + " settlement";
	}

private:
	const site *settlement = nullptr;
	const faction *faction = nullptr;
	bool enemy = false;
};

}
