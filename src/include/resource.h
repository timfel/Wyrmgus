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
/**@name resource.h - The resource header file. */
//
//      (c) Copyright 1999-2020 by Vladi Belperchinov-Shabanski,
//		Jimmy Salmon and Andrettin
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

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "data_type.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

struct lua_State;

/**
**  Indices into costs/resource/income array.
*/
enum CostType {
	TimeCost,                               /// time in game cycles

	// standard
	CopperCost,                             /// copper resource
	WoodCost,                               /// wood  resource
	OilCost,                                /// oil   resource
	// extensions
	IronCost,								/// iron resource
	StoneCost,								/// stone resource
	CoalCost,								/// coal resource

	ResearchCost,							/// research resource
	PrestigeCost,							/// prestige resource
	GoldCost,                               /// gold resource
	SilverCost,								/// silver resource
	MithrilCost,							/// mithril resource
	LimestoneCost,							/// limestone resource
	JewelryCost,							/// jewelry resource
	FurnitureCost,							/// furniture resource
	LeatherCost,							/// leather resource
	DiamondsCost,							/// diamonds resource
	EmeraldsCost,							/// emeralds resource
	LeadershipCost,							/// leadership resource
	TradeCost,								/// trade resource, generated by trader units (converted to copper when delivered)

	MaxCosts                                /// how many different costs
};

static constexpr int FoodCost = MaxCosts;
static constexpr int ScoreCost = MaxCosts + 1;
static constexpr int ManaResCost = MaxCosts + 2;
static constexpr int FreeWorkersCount = MaxCosts + 3;

class CResource : public CDataType
{
public:
	static CResource *GetResource(const std::string &ident, const bool should_find = true);
	static CResource *GetOrAddResource(const std::string &ident);
	static void ClearResources();
	
	static std::vector<CResource *> Resources;
	static std::map<std::string, CResource *> ResourcesByIdent;
	
	virtual void ProcessConfigData(const CConfigData *config_data) override;
	bool IsMineResource() const;

	std::string Name;
	std::string ActionName;
	int ID = -1;
	int DefaultIncome = 100;
	int DefaultAmount = 1000;
	int DefaultMaxAmount = -1;
	int FinalResource = -1;
	int FinalResourceConversionRate = 100;
	int BasePrice = 0;
	int DemandElasticity = 100;
	int InputResource = 0;
	bool LuxuryResource = false;
	bool Hidden = false;
	std::vector<CResource *> ChildResources; //resources (other than this one) that have this resource as their final resource
};

/**
**  Default resources for a new player.
*/
extern int DefaultResources[MaxCosts];

/**
**  Default resources for a new player with low resources.
*/
extern int DefaultResourcesLow[MaxCosts];

/**
**  Default resources for a new player with mid resources.
*/
extern int DefaultResourcesMedium[MaxCosts];

/**
**  Default resources for a new player with high resources.
*/
extern int DefaultResourcesHigh[MaxCosts];

/**
**  Default names for the resources.
*/
extern std::string DefaultResourceNames[MaxCosts];

extern std::vector<int> LuxuryResources;

extern int GetResourceIdByName(const char *resourceName);
extern int GetResourceIdByName(lua_State *l, const char *resourceName);
extern std::string GetResourceNameById(int resource_id);
