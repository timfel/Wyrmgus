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
//      (c) Copyright 2015-2020 by Andrettin
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

namespace wyrmgus {

enum class gender {
	none,
	male,
	female,
	asexual, //e.g. slimes

	count
};

inline gender try_string_to_gender(const std::string &str)
{
	if (str == "none") {
		return gender::none;
	} else if (str == "male") {
		return gender::male;
	} else if (str == "female") {
		return gender::female;
	} else if (str == "asexual") {
		return gender::asexual;
	}

	return gender::none;
}

inline gender string_to_gender(const std::string &str)
{
	if (str == "none") {
		return gender::none;
	} else if (str == "male") {
		return gender::male;
	} else if (str == "female") {
		return gender::female;
	} else if (str == "asexual") {
		return gender::asexual;
	}

	throw std::runtime_error("Invalid gender: \"" + str + "\".");
}

inline std::string gender_to_string(const gender gender)
{
	switch (gender) {
		case gender::none:
			return "none";
		case gender::male:
			return "male";
		case gender::female:
			return "female";
		case gender::asexual:
			return "asexual";
		default:
			break;
	}

	throw std::runtime_error("Invalid gender: \"" + std::to_string(static_cast<int>(gender)) + "\".");
}

}

Q_DECLARE_METATYPE(wyrmgus::gender)
