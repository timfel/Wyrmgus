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

#include "stratagus.h"

#include "database/module_container.h"

#include "database/module.h"

namespace wyrmgus {

bool module_compare::operator()(const module *module, const wyrmgus::module *other_module) const
{
	if ((module == nullptr) != (other_module == nullptr)) {
		return module == nullptr;
	}

	if (module->depends_on(other_module)) {
		return false;
	} else if (other_module->depends_on(module)) {
		return true;
	}

	if (module->get_dependency_count() != other_module->get_dependency_count()) {
		return module->get_dependency_count() < other_module->get_dependency_count();
	}

	return module->get_identifier() < other_module->get_identifier();
}

}
