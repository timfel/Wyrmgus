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
/**@name button_level.h - The button level header file. */
//
//      (c) Copyright 2018-2019 by Andrettin
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

#ifndef __BUTTON_LEVEL_H__
#define __BUTTON_LEVEL_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "data_type.h"

#include <map>
#include <string>
#include <vector>

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CButtonLevel : public CDataType
{
public:
	static CButtonLevel *GetButtonLevel(const std::string &ident, const bool should_find = true);
	static CButtonLevel *GetOrAddButtonLevel(const std::string &ident);
	static void ClearButtonLevels();
	
	static std::vector<CButtonLevel *> ButtonLevels;	/// Button levels
	static std::map<std::string, CButtonLevel *> ButtonLevelsByIdent;
	static CButtonLevel *CancelButtonLevel;
	static CButtonLevel *InventoryButtonLevel;
	
	virtual void ProcessConfigData(const CConfigData *config_data) override;
	
	int ID = -1;									/// ID of the button level (starting at 1)
};

#endif