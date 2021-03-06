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

#include "database/data_type.h"
#include "database/named_data_entry.h"

class CGraphic;
struct lua_State;

static int CclDefineDefaultResourceNames(lua_State *l);
static int CclDefineResource(lua_State *l);

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

constexpr int FoodCost = MaxCosts;
constexpr int ScoreCost = MaxCosts + 1;
constexpr int ManaResCost = MaxCosts + 2;
constexpr int FreeWorkersCount = MaxCosts + 3;

namespace wyrmgus {

class resource_icon;

class resource final : public named_data_entry, public data_type<resource>
{
	Q_OBJECT

	Q_PROPERTY(wyrmgus::resource_icon* icon MEMBER icon READ get_icon)
	Q_PROPERTY(int default_income MEMBER default_income READ get_default_income)
	Q_PROPERTY(int default_amount MEMBER default_amount READ get_default_amount)
	Q_PROPERTY(wyrmgus::resource* final_resource MEMBER final_resource)
	Q_PROPERTY(int final_resource_conversion_rate MEMBER final_resource_conversion_rate READ get_final_resource_conversion_rate)
	Q_PROPERTY(int base_price MEMBER base_price READ get_base_price)

public:
	static constexpr const char *class_identifier = "resource";
	static constexpr const char *database_folder = "resources";

	explicit resource(const std::string &identifier) : named_data_entry(identifier)
	{
	}

	virtual void process_sml_property(const sml_property &property) override;
	virtual void initialize() override;

	int get_index() const
	{
		return this->index;
	}

	resource_icon *get_icon() const
	{
		return this->icon;
	}

	const std::string &get_action_name() const
	{
		return this->action_name;
	}

	int get_default_income() const
	{
		return this->default_income;
	}

	int get_default_amount() const
	{
		return this->default_amount;
	}

	const resource *get_final_resource() const
	{
		if (this->final_resource != nullptr) {
			return this->final_resource;
		}

		return this;
	}

	int get_final_resource_conversion_rate() const
	{
		return this->final_resource_conversion_rate;
	}

	int get_base_price() const
	{
		return this->base_price;
	}

	bool IsMineResource() const;

private:
	int index = -1;
	resource_icon *icon = nullptr;
	std::filesystem::path icon_file;
	std::string action_name;
	int default_income = 100;
	int default_amount = 1000;
public:
	int DefaultMaxAmount = -1;
private:
	resource *final_resource = nullptr;
	int final_resource_conversion_rate = 100;
	int base_price = 0;
public:
	int DemandElasticity = 100;
	int InputResource = 0;
	bool LuxuryResource = false;
	bool Hidden = false;
	std::vector<resource *> ChildResources; //resources (other than this one) that have this resource as their final resource

	friend static int ::CclDefineDefaultResourceNames(lua_State *l);
	friend static int ::CclDefineResource(lua_State *l);
};

}

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
