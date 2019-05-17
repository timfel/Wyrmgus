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
/**@name site_dependency.h - The site dependency header file. */
//
//      (c) Copyright 2019 by Andrettin
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

#ifndef __SITE_DEPENDENCY_H__
#define __SITE_DEPENDENCY_H__

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "dependency/dependency.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CFaction;
class CPlayer;
class CSite;
class CUnit;

/*----------------------------------------------------------------------------
--  Definition
----------------------------------------------------------------------------*/

class CSiteDependency : public CDependency
{
public:
	virtual void ProcessConfigDataProperty(const std::pair<std::string, std::string> &property) override;
private:
	virtual bool CheckInternal(const CPlayer *player, const bool ignore_units = false) const override;
public:
	virtual bool Check(const CUnit *unit, const bool ignore_units = false) const override;
	virtual std::string GetString(const std::string &prefix = "") const override;

private:
	const CSite *Site = nullptr;
	const CFaction *Faction = nullptr;	/// the faction to check the ownership of the site for; if this isn't provided, the dependency will check instead if the triggering player owns the site, or if the unit is a part of the site
	bool Enemy = false;	/// whether the player or unit should be checked to see if they are an enemy of the site
};

#endif