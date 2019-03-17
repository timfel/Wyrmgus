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
/**@name faction.h - The faction header file. */
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

#ifndef __FACTION_H__
#define __FACTION_H__

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "character.h" // because of "MaxCharacterTitles"
#include "data_type.h"
#include "time/date.h"
#include "ui/icon_config.h"
#include "ui/ui.h" // for the UI fillers

#include <core/object.h>

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CAiBuildingTemplate;
class CCivilization;
class CCurrency;
class CDeity;
class CDynasty;
class CForceTemplate;
class CPlayerColor;
class CUpgrade;
class CSite;
class LuaCallback;

enum GovernmentTypes {
	GovernmentTypeNoGovernmentType,
	GovernmentTypeMonarchy,
	GovernmentTypeRepublic,
	GovernmentTypeTheocracy,
	
	MaxGovernmentTypes
};

enum FactionTypes {
	FactionTypeNoFactionType,
	FactionTypeTribe,
	FactionTypePolity,
	FactionTypeMercenaryCompany,
	FactionTypeHolyOrder,
	FactionTypeTradingCompany,
	
	MaxFactionTypes
};

enum FactionTiers {
	FactionTierNoFactionTier,
	FactionTierBarony,
	FactionTierCounty,
	FactionTierDuchy,
	FactionTierGrandDuchy,
	FactionTierKingdom,
	FactionTierEmpire,
	
	MaxFactionTiers
};

class CFaction : public Object
{
	GDCLASS(CFaction, Object)
	DATA_TYPE_CLASS(CFaction)
	
public:
	~CFaction();
	
	static int GetIndex(const std::string &faction_ident);
	static int GetFactionClassUnitType(const CFaction *faction, const int class_id);
	static int GetFactionClassUpgrade(const CFaction *faction, const int class_id);
	static std::vector<CFiller> GetFactionUIFillers(const CFaction *faction);
	
	/**
	**	@brief	Get the faction's string identifier
	**
	**	@return	The faction's string identifier
	*/
	String GetIdent() const
	{
		return this->Ident.c_str();
	}
	
	/**
	**	@brief	Get the faction's index
	**
	**	@return	The faction's index
	*/
	int GetIndex() const
	{
		return this->Index;
	}
	
	/**
	**	@brief	Get the faction's name
	**
	**	@return	The faction's name
	*/
	String GetName() const
	{
		return this->Name.c_str();
	}
	
	/**
	**	@brief	Get the faction's primary color with highest priority
	**
	**	@return	The faction's primary color
	*/
	CPlayerColor *GetPrimaryColor() const
	{
		if (!this->PrimaryColors.empty()) {
			return this->PrimaryColors.front();
		} else {
			return nullptr;
		}
	}
	
	/**
	**	@brief	Get the faction's primary colors
	**
	**	@return	The faction's primary colors
	*/
	const std::vector<CPlayerColor *> &GetPrimaryColors() const
	{
		return this->PrimaryColors;
	}
	
	/**
	**	@brief	Get the faction's secondary color
	**
	**	@return	The faction's secondary color
	*/
	CPlayerColor *GetSecondaryColor() const
	{
		return this->SecondaryColor;
	}
	
	CCurrency *GetCurrency() const;
	int GetUpgradePriority(const CUpgrade *upgrade) const;
	int GetForceTypeWeight(int force_type) const;
	std::vector<CForceTemplate *> GetForceTemplates(int force_type) const;
	std::vector<CAiBuildingTemplate *> GetAiBuildingTemplates() const;
	const std::vector<std::string> &GetShipNames() const;

private:
	std::string Ident;													/// faction string identifier
	int Index = -1;														/// faction index
public:
	std::string Name;													/// faction name
	std::string Description;											/// faction description
	std::string Quote;													/// faction quote
	std::string Background;												/// faction background
	std::string FactionUpgrade;											/// faction upgrade applied when the faction is set
	std::string Adjective;												/// adjective pertaining to the faction
	std::string DefaultAI = "land-attack";
	CCivilization *Civilization = nullptr;								/// faction civilization
	int Type = FactionTypeNoFactionType;								/// faction type (i.e. tribe or polity)
	int DefaultTier = FactionTierBarony;								/// default faction tier
	int DefaultGovernmentType = GovernmentTypeMonarchy;					/// default government type
	const CFaction *ParentFaction = nullptr;							/// the parent faction of this faction
	bool Playable = true;												/// faction playability
	bool DefiniteArticle = false;										/// whether the faction's name should be preceded by a definite article (e.g. "the Netherlands")
	IconConfig Icon;													/// Faction's icon
	CCurrency *Currency = nullptr;										/// The faction's currency
	CDeity *HolyOrderDeity = nullptr;									/// deity this faction belongs to, if it is a holy order
	LuaCallback *Conditions = nullptr;
private:
	std::vector<CPlayerColor *> PrimaryColors;							/// the faction's primary player colors
	CPlayerColor *SecondaryColor = nullptr;
public:
	std::vector<CFaction *> DevelopsFrom;								/// from which factions can this faction develop
	std::vector<CFaction *> DevelopsTo;									/// to which factions this faction can develop
	std::vector<CDynasty *> Dynasties;									/// which dynasties are available to this faction
	std::string Titles[MaxGovernmentTypes][MaxFactionTiers];			/// this faction's title for each government type and faction tier
	std::string MinisterTitles[MaxCharacterTitles][MaxGenders][MaxGovernmentTypes][MaxFactionTiers]; /// this faction's minister title for each minister type and government type
	std::map<const CUpgrade *, int> UpgradePriorities;					/// Priority for each upgrade
	std::map<int, IconConfig> ButtonIcons;								/// icons for button actions
	std::map<int, int> ClassUnitTypes;									/// the unit type slot of a particular class for a particular faction
	std::map<int, int> ClassUpgrades;									/// the upgrade slot of a particular class for a particular faction
	std::vector<std::string> ProvinceNames;								/// Province names for the faction
	std::vector<std::string> ShipNames;									/// Ship names for the faction
	std::vector<CSite *> Cores;											/// Core sites of this faction (required to found it)
	std::vector<CSite *> Sites;											/// Sites used for this faction if it needs a randomly-generated settlement
	std::map<int, std::vector<CForceTemplate *>> ForceTemplates;		/// Force templates, mapped to each force type
	std::map<int, int> ForceTypeWeights;								/// Weights for each force type
	std::vector<CAiBuildingTemplate *> AiBuildingTemplates;				/// AI building templates
	std::map<std::tuple<CDate, CDate, int>, CCharacter *> HistoricalMinisters;	/// historical ministers of the faction (as well as heads of state and government), mapped to the beginning and end of the rule, and the enum of the title in question
	std::map<std::string, std::map<CDate, bool>> HistoricalUpgrades;	/// historical upgrades of the faction, with the date of change
	std::map<int, int> HistoricalTiers;									/// dates in which this faction's tier changed; faction tier mapped to year
	std::map<int, int> HistoricalGovernmentTypes;						/// dates in which this faction's government type changed; government type mapped to year
	std::map<std::pair<CDate, CFaction *>, int> HistoricalDiplomacyStates;	/// dates in which this faction's diplomacy state to another faction changed; diplomacy state mapped to year and faction
	std::map<std::pair<CDate, int>, int> HistoricalResources;	/// dates in which this faction's storage of a particular resource changed; resource quantities mapped to date and resource
	std::vector<std::pair<CDate, std::string>> HistoricalCapitals;		/// historical capitals of the faction; the values are: date and settlement ident
	std::vector<CFiller> UIFillers;
	
	std::string Mod;													/// To which mod (or map), if any, this faction belongs

	friend int CclDefineFaction(lua_State *l);
	
protected:
	static void _bind_methods();
};

#endif