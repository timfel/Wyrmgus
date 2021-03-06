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
//      (c) Copyright 1998-2020 by Lutz Sammer, Jimmy Salmon, Nehal Mistry
//		and Andrettin
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

#include "player.h"

#include "action/action_upgradeto.h"
#include "actions.h"
#include "age.h"
#include "ai.h"
#include "ai/ai_local.h" //for using AiHelpers
#include "campaign.h"
#include "civilization.h"
#include "commands.h" //for faction setting
#include "currency.h"
#include "database/defines.h"
#include "dialogue.h"
#include "diplomacy_state.h"
#include "dynasty.h"
#include "editor.h"
#include "faction.h"
//Wyrmgus start
#include "game.h"
#include "grand_strategy.h"
#include "iocompat.h"
//Wyrmgus end
#include "iolib.h"
#include "item/unique_item.h"
#include "language/language.h"
//Wyrmgus start
#include "luacallback.h"
//Wyrmgus end
#include "map/map.h"
#include "map/map_layer.h"
#include "map/minimap.h"
#include "map/site.h"
#include "network.h"
#include "netconnect.h"
#include "objective_type.h"
//Wyrmgus start
#include "parameters.h"
//Wyrmgus end
#include "player_color.h"
//Wyrmgus start
#include "quest.h"
#include "religion/deity.h"
#include "religion/religion.h"
//Wyrmgus end
#include "script.h"
#include "script/condition/and_condition.h"
#include "script/effect/effect_list.h"
//Wyrmgus start
#include "settings.h"
#include "sound/sound.h"
#include "sound/unitsound.h"
#include "time/calendar.h"
#include "time/time_of_day.h"
#include "translate.h"
#include "ui/button.h"
#include "ui/ui.h"
#include "unit/unit.h"
//Wyrmgus start
#include "unit/unit_class.h"
#include "unit/unit_find.h"
#include "unit/unit_type.h"
//Wyrmgus end
#include "unit/unit_type_type.h"
//Wyrmgus start
#include "upgrade/upgrade.h"
//Wyrmgus end
#include "upgrade/upgrade_class.h"
#include "upgrade/upgrade_modifier.h"
#include "util/string_util.h"
#include "util/vector_util.h"
#include "util/vector_random_util.h"
#include "vassalage_type.h"
#include "video/font.h"
#include "video/video.h"
#include "world.h"

/**
**  @class CPlayer player.h
**
**  \#include "player.h"
**
**  This structure contains all information about a player in game.
**
**  The player structure members:
**
**  CPlayer::Player
**
**    This is the unique slot number. It is not possible that two
**    players have the same slot number at the same time. The slot
**    numbers are reused in the future. This means if a player is
**    defeated, a new player can join using this slot. Currently
**    #PlayerMax (16) players are supported. This member is used to
**    access bit fields.
**    Slot #PlayerNumNeutral (15) is reserved for the neutral units
**    like gold-mines or critters.
**
**    @note Should call this member Slot?
**
**  CPlayer::Name
**
**    Name of the player used for displays and network game.
**    It is restricted to 15 characters plus final zero.
**
**  CPlayer::Type
**
**    Type of the player. This field is setup from the level (map).
**    We support currently #PlayerNeutral,
**    #PlayerNobody, #PlayerComputer, #PlayerPerson,
**    #PlayerRescuePassive and #PlayerRescueActive.
**    @see #PlayerTypes.
**
**  CPlayer::RaceName
**
**    Name of the race to which the player belongs, used to select
**    the user interface and the AI.
**    We have 'orc', 'human', 'alliance' or 'mythical'. Should
**    only be used during configuration and not during runtime.
**
**  CPlayer::Race
**
**    Race number of the player. This field is setup from the level
**    map. This number is mapped with #PlayerRaces to the symbolic
**    name CPlayer::RaceName.
**
**  CPlayer::AiName
**
**    AI name for computer. This field is setup
**    from the map. Used to select the AI for the computer
**    player.
**
**  CPlayer::Team
**
**    Team of player. Selected during network game setup. All players
**    of the same team are allied and enemy to all other teams.
**    @note It is planned to show the team on the map.
**
**  CPlayer::Enemy
**
**    A bit field which contains the enemies of this player.
**    If CPlayer::Enemy & (1<<CPlayer::Player) != 0 its an enemy.
**    Setup during startup using the CPlayer::Team, can later be
**    changed with diplomacy. CPlayer::Enemy and CPlayer::Allied
**    are combined, if none bit is set, the player is neutral.
**    @note You can be allied to a player, which sees you as enemy.
**
**  CPlayer::Allied
**
**    A bit field which contains the allies of this player.
**    If CPlayer::Allied & (1<<CPlayer::Player) != 0 its an allied.
**    Setup during startup using the Player:Team, can later be
**    changed with diplomacy. CPlayer::Enemy and CPlayer::Allied
**    are combined, if none bit is set, the player is neutral.
**    @note You can be allied to a player, which sees you as enemy.
**
**  CPlayer::SharedVision
**
**    A bit field which contains shared vision for this player.
**    Shared vision only works when it's activated both ways. Really.
**
**  CPlayer::StartX CPlayer::StartY
**
**    The tile map coordinates of the player start position. 0,0 is
**    the upper left on the map. This members are setup from the
**    map and only important for the game start.
**    Ignored if game starts with level settings. Used to place
**    the initial workers if you play with 1 or 3 workers.
**
**  CPlayer::Resources[::MaxCosts]
**
**    How many resources the player owns. Needed for building
**    units and structures.
**    @see _costs_, TimeCost, GoldCost, WoodCost, OilCost, MaxCosts.
**
**  CPlayer::MaxResources[::MaxCosts]
**
**    How many resources the player can store at the moment.
**
**  CPlayer::Incomes[::MaxCosts]
**
**    Income of the resources, when they are delivered at a store.
**    @see _costs_, TimeCost, GoldCost, WoodCost, OilCost, MaxCosts.
**
**  CPlayer::LastResources[::MaxCosts]
**
**    Keeps track of resources in time (used for calculating
**    CPlayer::Revenue, see below)
**
**  CPlayer::Revenue[::MaxCosts]
**
**    Production of resources per minute (or estimates)
**    Used just as information (statistics) for the player...
**
**  CPlayer::UnitTypesCount[::UnitTypeMax]
**
**    Total count for each different unit type. Used by the AI and
**    for condition checks. The addition of all counts should
**    be CPlayer::TotalNumUnits.
**    @note Should not use the maximum number of unit-types here,
**    only the real number of unit-types used.
**
**  CPlayer::AiEnabled
**
**    If the player is controlled by the computer and this flag is
**    true, than the player is handled by the AI on this local
**    computer.
**
**    @note Currently the AI is calculated parallel on all computers
**    in a network play. It is planned to change this.
**
**  CPlayer::Ai
**
**    AI structure pointer. Please look at #PlayerAi for more
**    information.
**
**  CPlayer::Units
**
**    A table of all (CPlayer::TotalNumUnits) units of the player.
**
**  CPlayer::TotalNumUnits
**
**    Total number of units (incl. buildings) in the CPlayer::Units
**    table.
**
**  CPlayer::Demand
**
**    Total unit demand, used to demand limit.
**    A player can only build up to CPlayer::Food units and not more
**    than CPlayer::FoodUnitLimit units.
**
**    @note that CPlayer::NumFoodUnits > CPlayer::Food, when enough
**    farms are destroyed.
**
**  CPlayer::NumBuildings
**
**    Total number buildings, units that don't need food.
**
**  CPlayer::Food
**
**    Number of food available/produced. Player can't train more
**    CPlayer::NumFoodUnits than this.
**    @note that all limits are always checked.
**
**  CPlayer::FoodUnitLimit
**
**    Number of food units allowed. Player can't train more
**    CPlayer::NumFoodUnits than this.
**    @note that all limits are always checked.
**
**  CPlayer::BuildingLimit
**
**    Number of buildings allowed.  Player can't build more
**    CPlayer::NumBuildings than this.
**    @note that all limits are always checked.
**
**  CPlayer::TotalUnitLimit
**
**    Number of total units allowed. Player can't have more
**    CPlayer::NumFoodUnits+CPlayer::NumBuildings=CPlayer::TotalNumUnits
**    this.
**    @note that all limits are always checked.
**
**  CPlayer::Score
**
**    Total number of points. You can get points for killing units,
**    destroying buildings ...
**
**  CPlayer::TotalUnits
**
**    Total number of units made.
**
**  CPlayer::TotalBuildings
**
**    Total number of buildings made.
**
**  CPlayer::TotalResources[::MaxCosts]
**
**    Total number of resources collected.
**    @see _costs_, TimeCost, GoldCost, WoodCost, OilCost, MaxCosts.
**
**  CPlayer::TotalRazings
**
**    Total number of buildings destroyed.
**
**  CPlayer::TotalKills
**
**    Total number of kills.
**
**  CPlayer::Color
**
**    Color of units of this player on the minimap. Index number
**    into the global palette.
**
**  CPlayer::Allow
**
**    Contains which unit-types and upgrades are allowed for the
**    player. Possible values are:
**    @li  'A' -- allowed,
**    @li  'F' -- forbidden,
**    @li  'R' -- acquired, perhaps other values
**    @li  'Q' -- acquired but forbidden (does it make sense?:))
**    @li  'E' -- enabled, allowed by level but currently forbidden
**    @see CAllow
**
**  CPlayer::UpgradeTimers
**
**    Timer for the upgrades. One timer for all possible upgrades.
**    Initial 0 counted up by the upgrade action, until it reaches
**    the upgrade time.
**    @see _upgrade_timers_
**    @note it is planned to combine research for faster upgrades.
*/

int NumPlayers; //how many player slots used

CPlayer *CPlayer::ThisPlayer = nullptr;
std::vector<CPlayer *> CPlayer::Players; //all players in play

PlayerRace PlayerRaces; //player races

bool NoRescueCheck; //disable rescue check

/**
**  "Translate" (that is, adapt) a proper name from one culture (civilization) to another.
*/
std::string PlayerRace::TranslateName(const std::string &name, wyrmgus::language *language)
{
	std::string new_name;
	
	if (!language || name.empty()) {
		return new_name;
	}

	// try to translate the entire name, as a particular translation for it may exist
	if (language->NameTranslations.find(name) != language->NameTranslations.end()) {
		return language->NameTranslations[name][SyncRand(language->NameTranslations[name].size())];
	}
	
	//if adapting the entire name failed, try to match prefixes and suffixes
	if (name.size() > 1) {
		if (name.find(" ") == std::string::npos) {
			for (size_t i = 0; i < name.size(); ++i) {
				std::string name_prefix = name.substr(0, i + 1);
				std::string name_suffix = CapitalizeString(name.substr(i + 1, name.size() - (i + 1)));
			
	//			fprintf(stdout, "Trying to match prefix \"%s\" and suffix \"%s\" for translating name \"%s\" to the \"%s\" language.\n", name_prefix.c_str(), name_suffix.c_str(), name.c_str(), language->Ident.c_str());
			
				if (language->NameTranslations.find(name_prefix) != language->NameTranslations.end() && language->NameTranslations.find(name_suffix) != language->NameTranslations.end()) { // if both a prefix and suffix have been matched
					name_prefix = language->NameTranslations[name_prefix][SyncRand(language->NameTranslations[name_prefix].size())];
					name_suffix = language->NameTranslations[name_suffix][SyncRand(language->NameTranslations[name_suffix].size())];
					name_suffix = DecapitalizeString(name_suffix);
					if (name_prefix.substr(name_prefix.size() - 2, 2) == "gs" && name_suffix.substr(0, 1) == "g") { //if the last two characters of the prefix are "gs", and the first character of the suffix is "g", then remove the final "s" from the prefix (as in "Königgrätz")
						name_prefix = FindAndReplaceStringEnding(name_prefix, "gs", "g");
					}
					if (name_prefix.substr(name_prefix.size() - 1, 1) == "s" && name_suffix.substr(0, 1) == "s") { //if the prefix ends in "s" and the suffix begins in "s" as well, then remove the final "s" from the prefix (as in "Josefstadt", "Kronstadt" and "Leopoldstadt")
						name_prefix = FindAndReplaceStringEnding(name_prefix, "s", "");
					}

					return name_prefix + name_suffix;
				}
			}
		} else { // if the name contains a space, try to translate each of its elements separately
			size_t previous_pos = 0;
			new_name = name;
			for (size_t i = 0; i < name.size(); ++i) {
				if ((i + 1) == name.size() || name[i + 1] == ' ') {
					std::string name_element = TranslateName(name.substr(previous_pos, i + 1 - previous_pos), language);
				
					if (name_element.empty()) {
						new_name = "";
						break;
					}
				
					new_name = FindAndReplaceString(new_name, name.substr(previous_pos, i + 1 - previous_pos), name_element);
				
					previous_pos = i + 2;
				}
			}
		}
	}
	
	return new_name;
}
//Wyrmgus end

/**
**  Init players.
*/
void InitPlayers()
{
	for (int p = 0; p < PlayerMax; ++p) {
		CPlayer::Players[p]->Index = p;
		if (!CPlayer::Players[p]->Type) {
			CPlayer::Players[p]->Type = PlayerNobody;
		}
	}
}

/**
**  Clean up players.
*/
void CleanPlayers()
{
	CPlayer::SetThisPlayer(nullptr);
	CPlayer::revealed_players.clear();
	for (unsigned int i = 0; i < PlayerMax; ++i) {
		CPlayer::Players[i]->Clear();
	}
	NumPlayers = 0;
	NoRescueCheck = false;
}

/**
**  Save state of players to file.
**
**  @param file  Output file.
**
**  @note FIXME: Not completely saved.
*/
void SavePlayers(CFile &file)
{
	file.printf("\n--------------------------------------------\n");
	file.printf("--- MODULE: players\n\n");

	//  Dump all players
	for (int i = 0; i < NumPlayers; ++i) {
		CPlayer::Players[i]->Save(file);
	}

	file.printf("SetThisPlayer(%d)\n\n", CPlayer::GetThisPlayer()->Index);
}

CPlayer::CPlayer()
{
}

CPlayer::~CPlayer()
{
}

void CPlayer::SetThisPlayer(CPlayer *player)
{
	if (player == CPlayer::GetThisPlayer()) {
		return;
	}

	CPlayer::ThisPlayer = player;
}

CPlayer *CPlayer::GetThisPlayer()
{
	return CPlayer::ThisPlayer;
}

CPlayer *CPlayer::GetPlayer(const int index)
{
	if (index < 0) {
		fprintf(stderr, "Cannot get player for index %d: the index is negative.\n", index);
		return nullptr;
	}

	if (index >= PlayerMax) {
		fprintf(stderr, "Cannot get player for index %d: the maximum value is %d.\n", index, PlayerMax);
		return nullptr;
	}

	return CPlayer::Players[index];
}

const QColor &CPlayer::get_minimap_color() const
{
	return this->get_player_color()->get_colors().at(wyrmgus::defines::get()->get_minimap_color_index());
}

void CPlayer::set_revealed(const bool revealed)
{
	if (revealed == this->is_revealed()) {
		return;
	}

	this->revealed = revealed;

	if (revealed) {
		CPlayer::revealed_players.push_back(this);
	} else {
		wyrmgus::vector::remove(CPlayer::revealed_players, this);
	}
}

void CPlayer::Save(CFile &file) const
{
	const CPlayer &p = *this;
	file.printf("Player(%d,\n", this->Index);
	//Wyrmgus start
	file.printf(" \"race\", \"%s\",", wyrmgus::civilization::get_all()[p.Race]->get_identifier().c_str());
	if (p.Faction != -1) {
		file.printf(" \"faction\", %d,", p.Faction);
	}
	if (p.get_faction_tier() != wyrmgus::faction_tier::none) {
		file.printf(" \"faction-tier\", \"%s\",", wyrmgus::faction_tier_to_string(this->get_faction_tier()).c_str());
	}
	if (p.get_government_type() != wyrmgus::government_type::none) {
		file.printf(" \"government-type\", \"%s\",", wyrmgus::government_type_to_string(this->get_government_type()).c_str());
	}
	if (p.get_dynasty() != nullptr) {
		file.printf(" \"dynasty\", \"%s\",", p.get_dynasty()->get_identifier().c_str());
	}
	if (p.age) {
		file.printf(" \"age\", \"%s\",", p.age->get_identifier().c_str());
	}
	if (p.get_player_color() != nullptr) {
		file.printf(" \"player-color\", \"%s\",", p.get_player_color()->get_identifier().c_str());
	}
	//Wyrmgus end
	file.printf("  \"name\", \"%s\",\n", p.Name.c_str());
	file.printf("  \"type\", ");
	switch (p.Type) {
		case PlayerNeutral:       file.printf("\"neutral\",");         break;
		case PlayerNobody:        file.printf("\"nobody\",");          break;
		case PlayerComputer:      file.printf("\"computer\",");        break;
		case PlayerPerson:        file.printf("\"person\",");          break;
		case PlayerRescuePassive: file.printf("\"rescue-passive\","); break;
		case PlayerRescueActive:  file.printf("\"rescue-active\","); break;
		default:                  file.printf("%d,", p.Type); break;
	}
	//Wyrmgus start
//	file.printf(" \"race\", \"%s\",", PlayerRaces.Name[p.Race].c_str());
	//Wyrmgus end
	file.printf(" \"ai-name\", \"%s\",\n", p.AiName.c_str());
	file.printf("  \"team\", %d,", p.Team);

	file.printf(" \"enemy\", \"");
	for (int j = 0; j < PlayerMax; ++j) {
		file.printf("%c", p.enemies.contains(j) ? 'X' : '_');
	}
	file.printf("\", \"allied\", \"");
	for (int j = 0; j < PlayerMax; ++j) {
		file.printf("%c", p.allies.contains(j) ? 'X' : '_');
	}
	file.printf("\", \"shared-vision\", \"");
	for (int j = 0; j < PlayerMax; ++j) {
		file.printf("%c", p.shared_vision.contains(j) ? 'X' : '_');
	}
	file.printf("\",\n  \"start\", {%d, %d},\n", p.StartPos.x, p.StartPos.y);
	//Wyrmgus start
	file.printf("  \"start-map-layer\", %d,\n", p.StartMapLayer);
	//Wyrmgus end
	if (p.get_overlord() != nullptr) {
		file.printf("  \"overlord\", %d, \"%s\",\n", p.get_overlord()->Index, wyrmgus::vassalage_type_to_string(p.vassalage_type).c_str());
	}

	// Resources
	file.printf("  \"resources\", {");
	for (int j = 0; j < MaxCosts; ++j) {
		//Wyrmgus start
		if (!p.Resources[j]) {
			continue;
		}
		//Wyrmgus end
		file.printf("\"%s\", %d, ", DefaultResourceNames[j].c_str(), p.Resources[j]);
	}
	// Stored Resources
	file.printf("},\n  \"stored-resources\", {");
	for (int j = 0; j < MaxCosts; ++j) {
		//Wyrmgus start
		if (!p.StoredResources[j]) {
			continue;
		}
		//Wyrmgus end
		file.printf("\"%s\", %d, ", DefaultResourceNames[j].c_str(), p.StoredResources[j]);
	}
	// Max Resources
	file.printf("},\n  \"max-resources\", {");
	for (int j = 0; j < MaxCosts; ++j) {
		file.printf("\"%s\", %d, ", DefaultResourceNames[j].c_str(), p.MaxResources[j]);
	}
	// Last Resources
	file.printf("},\n  \"last-resources\", {");
	for (int j = 0; j < MaxCosts; ++j) {
		//Wyrmgus start
		if (!p.LastResources[j]) {
			continue;
		}
		//Wyrmgus end
		file.printf("\"%s\", %d, ", DefaultResourceNames[j].c_str(), p.LastResources[j]);
	}
	// Incomes
	file.printf("},\n  \"incomes\", {");
	for (int j = 0; j < MaxCosts; ++j) {
		if (j) {
			if (j == MaxCosts / 2) {
				file.printf("\n ");
			} else {
				file.printf(" ");
			}
		}
		file.printf("\"%s\", %d,", DefaultResourceNames[j].c_str(), p.Incomes[j]);
	}
	// Revenue
	file.printf("},\n  \"revenue\", {");
	for (int j = 0; j < MaxCosts; ++j) {
		//Wyrmgus start
//		if (j) {
//			if (j == MaxCosts / 2) {
//				file.printf("\n ");
//			} else {
//				file.printf(" ");
//			}
//		}
//		file.printf("\"%s\", %d,", DefaultResourceNames[j].c_str(), p.Revenue[j]);
		if (p.Revenue[j]) {
			file.printf("\"%s\", %d, ", DefaultResourceNames[j].c_str(), p.Revenue[j]);
		}
		//Wyrmgus end
	}
	
	//Wyrmgus start
	file.printf("},\n  \"prices\", {");
	for (int j = 0; j < MaxCosts; ++j) {
		if (p.Prices[j]) {
			file.printf("\"%s\", %d, ", DefaultResourceNames[j].c_str(), p.Prices[j]);
		}
	}
	//Wyrmgus end

	// UnitTypesCount done by load units.

	file.printf("},\n  \"%s\",\n", p.AiEnabled ? "ai-enabled" : "ai-disabled");

	// Ai done by load ais.
	// Units done by load units.
	// TotalNumUnits done by load units.
	// NumBuildings done by load units.
	
	if (p.is_revealed()) {
		file.printf(" \"revealed\",");
	}
	
	file.printf(" \"supply\", %d,", p.Supply);
	file.printf(" \"trade-cost\", %d,", p.TradeCost);
	file.printf(" \"unit-limit\", %d,", p.UnitLimit);
	file.printf(" \"building-limit\", %d,", p.BuildingLimit);
	file.printf(" \"total-unit-limit\", %d,", p.TotalUnitLimit);

	file.printf("\n  \"score\", %d,", p.Score);
	file.printf("\n  \"total-units\", %d,", p.TotalUnits);
	file.printf("\n  \"total-buildings\", %d,", p.TotalBuildings);
	file.printf("\n  \"total-resources\", {");
	for (int j = 0; j < MaxCosts; ++j) {
		if (j) {
			file.printf(" ");
		}
		file.printf("%d,", p.TotalResources[j]);
	}
	file.printf("},");
	file.printf("\n  \"total-razings\", %d,", p.TotalRazings);
	file.printf("\n  \"total-kills\", %d,", p.TotalKills);
	//Wyrmgus start
	file.printf("\n  \"unit-type-kills\", {");
	for (const wyrmgus::unit_type *unit_type : wyrmgus::unit_type::get_all()) {
		if (p.UnitTypeKills[unit_type->Slot] != 0) {
			file.printf("\"%s\", %d, ", unit_type->Ident.c_str(), p.UnitTypeKills[unit_type->Slot]);
		}
	}
	file.printf("},");
	//Wyrmgus end
	if (p.LostTownHallTimer != 0) {
		file.printf("\n  \"lost-town-hall-timer\", %d,", p.LostTownHallTimer);
	}
	if (p.HeroCooldownTimer != 0) {
		file.printf("\n  \"hero-cooldown-timer\", %d,", p.HeroCooldownTimer);
	}
	//Wyrmgus end

	file.printf("\n  \"speed-resource-harvest\", {");
	for (int j = 0; j < MaxCosts; ++j) {
		if (j) {
			file.printf(" ");
		}
		file.printf("%d,", p.SpeedResourcesHarvest[j]);
	}
	file.printf("},");
	file.printf("\n  \"speed-resource-return\", {");
	for (int j = 0; j < MaxCosts; ++j) {
		if (j) {
			file.printf(" ");
		}
		file.printf("%d,", p.SpeedResourcesReturn[j]);
	}
	file.printf("},");
	file.printf("\n  \"speed-build\", %d,", p.SpeedBuild);
	file.printf("\n  \"speed-train\", %d,", p.SpeedTrain);
	file.printf("\n  \"speed-upgrade\", %d,", p.SpeedUpgrade);
	file.printf("\n  \"speed-research\", %d,", p.SpeedResearch);
	
	//Wyrmgus start
	/*
	Uint8 r, g, b;

	SDL_GetRGB(p.Color, TheScreen->format, &r, &g, &b);
	file.printf("\n  \"color\", { %d, %d, %d },", r, g, b);
	*/
	//Wyrmgus end

	//Wyrmgus start
	file.printf("\n  \"current-quests\", {");
	for (size_t j = 0; j < p.current_quests.size(); ++j) {
		if (j) {
			file.printf(" ");
		}
		file.printf("\"%s\",", p.current_quests[j]->get_identifier().c_str());
	}
	file.printf("},");
	
	file.printf("\n  \"completed-quests\", {");
	for (size_t j = 0; j < p.completed_quests.size(); ++j) {
		if (j) {
			file.printf(" ");
		}
		file.printf("\"%s\",", p.completed_quests[j]->get_identifier().c_str());
	}
	file.printf("},");
	
	file.printf("\n  \"quest-objectives\", {");
	for (size_t j = 0; j < p.get_quest_objectives().size(); ++j) {
		const auto &objective = p.get_quest_objectives()[j];
		if (j != 0) {
			file.printf(" ");
		}
		file.printf("{");
		file.printf("\"quest\", \"%s\",", objective->get_quest_objective()->get_quest()->get_identifier().c_str());
		file.printf("\"objective-index\", %d,", objective->get_quest_objective()->get_index());
		file.printf("\"counter\", %d,", objective->Counter);
		file.printf("},");
	}
	file.printf("},");
	
	file.printf("\n  \"modifiers\", {");
	for (size_t j = 0; j < p.Modifiers.size(); ++j) {
		if (j) {
			file.printf(" ");
		}
		file.printf("\"%s\", %d,", p.Modifiers[j].first->get_identifier().c_str(), p.Modifiers[j].second);
	}
	file.printf("},");
	//Wyrmgus end

	file.printf("\n  \"autosell-resources\", {");
	for (size_t j = 0; j < p.AutosellResources.size(); ++j) {
		if (j) {
			file.printf(" ");
		}
		file.printf("\"%s\",", DefaultResourceNames[p.AutosellResources[j]].c_str());
	}
	file.printf("},");
	
	// Allow saved by allow.

	file.printf("\n  \"timers\", {");
	//Wyrmgus start
	bool first = true;
	//Wyrmgus end
	for (const CUpgrade *upgrade : CUpgrade::get_all()) {
		//Wyrmgus start
//		if (upgrade->ID) {
//			file.printf(" ,");
//		}
//		file.printf("%d", p.UpgradeTimers.Upgrades[upgrade->ID]);
		if (p.UpgradeTimers.Upgrades[upgrade->ID]) {
			if (first) {
				first = false;
			} else {
				file.printf(", ");
			}
			file.printf("\"%s\", %d", upgrade->get_identifier().c_str(), p.UpgradeTimers.Upgrades[upgrade->ID]);
		}
		//Wyrmgus end
	}
	file.printf("}");

	file.printf(")\n\n");

	DebugPrint("FIXME: must save unit-stats?\n");
}

/**
**  Create a new player.
**
**  @param type  Player type (Computer,Human,...).
*/
void CreatePlayer(int type)
{
	if (NumPlayers == PlayerMax) { // already done for bigmaps!
		return;
	}
	CPlayer *player = CPlayer::Players[NumPlayers];
	player->Index = NumPlayers;

	player->Init(type);
}

CPlayer *GetFactionPlayer(const wyrmgus::faction *faction)
{
	if (faction == nullptr) {
		return nullptr;
	}
	
	for (CPlayer *player : CPlayer::Players) {
		if (player->get_faction() == faction) {
			return player;
		}
	}
	
	return nullptr;
}

CPlayer *GetOrAddFactionPlayer(const wyrmgus::faction *faction)
{
	CPlayer *faction_player = GetFactionPlayer(faction);
	if (faction_player != nullptr) {
		return faction_player;
	}
	
	// no player belonging to this faction, so let's make an unused player slot be created for it
	
	for (int i = 0; i < NumPlayers; ++i) {
		CPlayer *player = CPlayer::Players[i];
		if (player->Type == PlayerNobody) {
			player->Type = PlayerComputer;
			player->set_civilization(faction->get_civilization());
			player->SetFaction(faction);
			player->AiEnabled = true;
			player->AiName = faction->DefaultAI;
			player->Team = 1;
			player->Resources[CopperCost] = 2500; // give the new player enough resources to start up
			player->Resources[WoodCost] = 2500;
			player->Resources[StoneCost] = 2500;
			return player;
		}
	}
	
	throw std::runtime_error("Cannot add player for faction \"" + faction->get_identifier() + "\": no player slots available.");
}

void CPlayer::Init(/* PlayerTypes */ int type)
{
	std::vector<CUnit *>().swap(this->Units);
	std::vector<CUnit *>().swap(this->FreeWorkers);
	std::vector<CUnit *>().swap(this->LevelUpUnits);

	//  Take first slot for person on this computer,
	//  fill other with computer players.
	if (type == PlayerPerson && !NetPlayers) {
		if (!CPlayer::GetThisPlayer()) {
			CPlayer::SetThisPlayer(this);
		} else {
			type = PlayerComputer;
		}
	}
	if (NetPlayers && NumPlayers == NetLocalPlayerNumber) {
		CPlayer::SetThisPlayer(CPlayer::Players[NetLocalPlayerNumber]);
	}

	if (NumPlayers == PlayerMax) {
		static int already_warned;

		if (!already_warned) {
			DebugPrint("Too many players\n");
			already_warned = 1;
		}
		return;
	}

	//  Make simple teams:
	//  All person players are enemies.
	int team;
	switch (type) {
		case PlayerNeutral:
		case PlayerNobody:
		default:
			team = 0;
			this->SetName("Neutral");
			break;
		case PlayerComputer:
			team = 1;
			this->SetName("Computer");
			break;
		case PlayerPerson:
			team = 2 + NumPlayers;
			this->SetName("Person");
			break;
		case PlayerRescuePassive:
		case PlayerRescueActive:
			// FIXME: correct for multiplayer games?
			this->SetName("Computer");
			team = 2 + NumPlayers;
			break;
	}
	DebugPrint("CreatePlayer name %s\n" _C_ this->Name.c_str());

	this->Type = type;
	this->Race = wyrmgus::defines::get()->get_neutral_civilization()->ID;
	this->Faction = -1;
	this->faction_tier = wyrmgus::faction_tier::none;
	this->government_type = wyrmgus::government_type::none;
	this->religion = nullptr;
	this->dynasty = nullptr;
	this->age = nullptr;
	this->overlord = nullptr;
	this->vassalage_type = wyrmgus::vassalage_type::none;
	this->Team = team;
	this->enemies.clear();
	this->allies.clear();
	this->AiName = "ai-passive";

	//  Calculate enemy/allied mask.
	for (int i = 0; i < NumPlayers; ++i) {
		CPlayer *other_player = CPlayer::Players[i];
		switch (type) {
			case PlayerNeutral:
			case PlayerNobody:
			default:
				break;
			case PlayerComputer:
				// Computer allied with computer and enemy of all persons.
				// make computer players be hostile to each other by default
				if (other_player->Type == PlayerComputer || other_player->Type == PlayerPerson || other_player->Type == PlayerRescueActive) {
					this->enemies.insert(i);
					other_player->enemies.insert(NumPlayers);
				}
				break;
			case PlayerPerson:
				// Humans are enemy of all?
				if (other_player->Type == PlayerComputer || other_player->Type == PlayerPerson) {
					this->enemies.insert(i);
					other_player->enemies.insert(NumPlayers);
				} else if (other_player->Type == PlayerRescueActive || other_player->Type == PlayerRescuePassive) {
					this->allies.insert(i);
					other_player->allies.insert(NumPlayers);
				}
				break;
			case PlayerRescuePassive:
				// Rescue passive are allied with persons
				if (other_player->Type == PlayerPerson) {
					this->allies.insert(i);
					other_player->allies.insert(NumPlayers);
				}
				break;
			case PlayerRescueActive:
				// Rescue active are allied with persons and enemies of computer
				if (other_player->Type == PlayerComputer) {
					this->enemies.insert(i);
					other_player->enemies.insert(NumPlayers);
				} else if (other_player->Type == PlayerPerson) {
					this->allies.insert(i);
					other_player->allies.insert(NumPlayers);
				}
				break;
		}
	}

	//  Initial default incomes.
	for (int i = 0; i < MaxCosts; ++i) {
		this->Incomes[i] = wyrmgus::resource::get_all()[i]->get_default_income();
	}
	
	this->TradeCost = DefaultTradeCost;

	//  Initial max resource amounts.
	for (int i = 0; i < MaxCosts; ++i) {
		this->MaxResources[i] = wyrmgus::resource::get_all()[i]->DefaultMaxAmount;
	}

	//Wyrmgus start
	this->UnitTypesCount.clear();
	this->UnitTypesUnderConstructionCount.clear();
	this->UnitTypesAiActiveCount.clear();
	this->Heroes.clear();
	this->Deities.clear();
	this->units_by_type.clear();
	this->units_by_class.clear();
	this->AiActiveUnitsByType.clear();
	//Wyrmgus end

	this->Supply = 0;
	this->Demand = 0;
	this->NumBuildings = 0;
	//Wyrmgus start
	this->NumBuildingsUnderConstruction = 0;
	this->NumTownHalls = 0;
	//Wyrmgus end
	this->Score = 0;
	//Wyrmgus start
	this->LostTownHallTimer = 0;
	this->HeroCooldownTimer = 0;
	//Wyrmgus end

	if (CPlayer::Players[NumPlayers]->Type == PlayerComputer || CPlayer::Players[NumPlayers]->Type == PlayerRescueActive) {
		this->AiEnabled = true;
	} else {
		this->AiEnabled = false;
	}
	this->revealed = false;
	++NumPlayers;
}

bool CPlayer::is_neutral_player() const
{
	return this->get_index() == PlayerNumNeutral;
}

void CPlayer::SetName(const std::string &name)
{
	this->Name = name;
}

const wyrmgus::civilization *CPlayer::get_civilization() const
{
	if (this->Race != -1) {
		return wyrmgus::civilization::get_all()[this->Race];
	}

	return nullptr;
}

//Wyrmgus start
void CPlayer::set_civilization(const wyrmgus::civilization *civilization)
{
	if (this->get_civilization() != nullptr && (GameRunning || GameEstablishing)) {
		const wyrmgus::civilization *old_civilization = this->get_civilization();
		if (old_civilization->get_upgrade() != nullptr && this->Allow.Upgrades[old_civilization->get_upgrade()->ID] == 'R') {
			UpgradeLost(*this, old_civilization->get_upgrade()->ID);
		}
	}

	if (GameRunning) {
		this->SetFaction(nullptr);
	} else {
		this->Faction = -1;
	}

	this->Race = civilization->ID;

	if (this->get_civilization() != nullptr) {
		//if the civilization of the person player changed, update the UI
		if ((CPlayer::GetThisPlayer() && CPlayer::GetThisPlayer()->Index == this->Index) || (!CPlayer::GetThisPlayer() && this->Index == 0)) {
			//load proper UI
			char buf[256];
			snprintf(buf, sizeof(buf), "if (LoadCivilizationUI ~= nil) then LoadCivilizationUI(\"%s\") end;", this->get_civilization()->get_identifier().c_str());
			CclCommand(buf);

			UI.Load();
		}

		const wyrmgus::civilization *new_civilization = this->get_civilization();
		CUpgrade *civilization_upgrade = new_civilization->get_upgrade();
		if (civilization_upgrade != nullptr && this->Allow.Upgrades[civilization_upgrade->ID] != 'R') {
			UpgradeAcquire(*this, civilization_upgrade);
		}
	}
}

wyrmgus::faction *CPlayer::get_faction() const
{
	if (this->Faction != -1) {
		return wyrmgus::faction::get_all()[this->Faction];
	}

	return nullptr;
}

void CPlayer::SetFaction(const wyrmgus::faction *faction)
{
	int old_faction_id = this->Faction;
	
	if (faction != nullptr && faction->get_civilization() != this->get_civilization()) {
		this->set_civilization(faction->get_civilization());
	}

	if (this->Faction != -1) {
		if (!wyrmgus::faction::get_all()[this->Faction]->FactionUpgrade.empty() && this->Allow.Upgrades[CUpgrade::get(wyrmgus::faction::get_all()[this->Faction]->FactionUpgrade)->ID] == 'R') {
			UpgradeLost(*this, CUpgrade::get(wyrmgus::faction::get_all()[this->Faction]->FactionUpgrade)->ID);
		}

		int faction_type_upgrade_id = UpgradeIdByIdent("upgrade-" + GetFactionTypeNameById(wyrmgus::faction::get_all()[this->Faction]->Type));
		if (faction_type_upgrade_id != -1 && this->Allow.Upgrades[faction_type_upgrade_id] == 'R') {
			UpgradeLost(*this, faction_type_upgrade_id);
		}
	}

	const int faction_id = faction ? faction->ID : -1;
	
	if (old_faction_id != -1 && faction_id != -1) {
		for (const wyrmgus::upgrade_class *upgrade_class : wyrmgus::upgrade_class::get_all()) {
			const CUpgrade *old_faction_class_upgrade = wyrmgus::faction::get_all()[old_faction_id]->get_class_upgrade(upgrade_class);
			const CUpgrade *new_faction_class_upgrade = faction->get_class_upgrade(upgrade_class);
			if (old_faction_class_upgrade != new_faction_class_upgrade) { //if the upgrade for a certain class is different for the new faction than the old faction (and it has been acquired), remove the modifiers of the old upgrade and apply the modifiers of the new
				if (old_faction_class_upgrade != nullptr && this->Allow.Upgrades[old_faction_class_upgrade->ID] == 'R') {
					UpgradeLost(*this, old_faction_class_upgrade->ID);

					if (new_faction_class_upgrade != nullptr) {
						UpgradeAcquire(*this, new_faction_class_upgrade);
					}
				}
			}
		}
	}
	
	bool personal_names_changed = true;
	bool ship_names_changed = true;
	if (this->Faction != -1 && faction_id != -1) {
		ship_names_changed = wyrmgus::faction::get_all()[this->Faction]->get_ship_names() != wyrmgus::faction::get_all()[faction_id]->get_ship_names();
		personal_names_changed = false; // setting to a faction of the same civilization
	}
	
	this->Faction = faction_id;

	if (this->Index == CPlayer::GetThisPlayer()->Index) {
		UI.Load();
	}
	
	if (this->Faction == -1) {
		return;
	}
	
	if (!IsNetworkGame()) { //only set the faction's name as the player's name if this is a single player game
		this->SetName(wyrmgus::faction::get_all()[this->Faction]->get_name());
	}
	if (this->get_faction() != nullptr) {
		const wyrmgus::player_color *player_color = nullptr;

		this->set_faction_tier(faction->get_default_tier());
		this->set_government_type(faction->get_default_government_type());

		const wyrmgus::player_color *faction_color = faction->get_color();
		if (faction_color != nullptr) {
			if (this->get_player_color_usage_count(faction_color) == 0) {
				player_color = faction_color;
			}
		}

		if (player_color == nullptr) {
			//if all of the faction's colors are used, get one of the least used player colors
			//out of those colors, give priority to the one closest (in RGB values) to the faction's color
			int best_usage_count = -1;
			int best_rgb_difference = -1;
			std::vector<const wyrmgus::player_color *> available_colors;
			for (const wyrmgus::player_color *pc : wyrmgus::player_color::get_all()) {
				if (pc == wyrmgus::defines::get()->get_neutral_player_color()) {
					continue;
				}

				if (pc->is_hidden()) {
					continue;
				}

				const int usage_count = this->get_player_color_usage_count(pc);
				if (best_usage_count != -1 && usage_count > best_usage_count) {
					continue;
				}

				if (usage_count < best_usage_count) {
					available_colors.clear();
					best_rgb_difference = -1;
				}

				int rgb_difference = -1;

				if (faction_color != nullptr) {
					for (size_t i = 0; i < faction_color->get_colors().size(); ++i) {
						const QColor &faction_color_shade = faction_color->get_colors()[i];
						const QColor &pc_color_shade = pc->get_colors()[i];
						rgb_difference += std::abs(faction_color_shade.red() - pc_color_shade.red());
						rgb_difference += std::abs(faction_color_shade.green() - pc_color_shade.green());
						rgb_difference += std::abs(faction_color_shade.blue() - pc_color_shade.blue());
					}
				}

				if (best_rgb_difference != -1 && rgb_difference > best_rgb_difference) {
					continue;
				}

				if (rgb_difference < best_rgb_difference) {
					available_colors.clear();
				}

				available_colors.push_back(pc);
				best_usage_count = usage_count;
				best_rgb_difference = rgb_difference;
			}

			if (!available_colors.empty()) {
				player_color = available_colors[SyncRand(available_colors.size())];
			}
		}
		
		if (player_color == nullptr) {
			throw std::runtime_error("No player color chosen for player \"" + this->Name + "\" (" + std::to_string(this->Index) + ").");
		}

		this->player_color = player_color;

		//update the territory on the minimap for the new color
		this->update_minimap_territory();

		if (!wyrmgus::faction::get_all()[this->Faction]->FactionUpgrade.empty()) {
			CUpgrade *faction_upgrade = CUpgrade::try_get(wyrmgus::faction::get_all()[this->Faction]->FactionUpgrade);
			if (faction_upgrade && this->Allow.Upgrades[faction_upgrade->ID] != 'R') {
				if (GameEstablishing) {
					AllowUpgradeId(*this, faction_upgrade->ID, 'R');
				} else {
					UpgradeAcquire(*this, faction_upgrade);
				}
			}
		}
		
		int faction_type_upgrade_id = UpgradeIdByIdent("upgrade-" + GetFactionTypeNameById(wyrmgus::faction::get_all()[this->Faction]->Type));
		if (faction_type_upgrade_id != -1 && this->Allow.Upgrades[faction_type_upgrade_id] != 'R') {
			if (GameEstablishing) {
				AllowUpgradeId(*this, faction_type_upgrade_id, 'R');
			} else {
				UpgradeAcquire(*this, CUpgrade::get_all()[faction_type_upgrade_id]);
			}
		}
	} else {
		fprintf(stderr, "Invalid faction \"%s\" tried to be set for player %d of civilization \"%s\".\n", faction->get_name().c_str(), this->Index, wyrmgus::civilization::get_all()[this->Race]->get_identifier().c_str());
	}
	
	for (int i = 0; i < this->GetUnitCount(); ++i) {
		CUnit &unit = this->GetUnit(i);
		if (unit.get_unique() == nullptr && unit.Type->PersonalNames.size() == 0) {
			if (!unit.Type->BoolFlag[ORGANIC_INDEX].value && unit.Type->UnitType == UnitTypeType::Naval && ship_names_changed) {
				unit.UpdatePersonalName();
			}
		}
		if (personal_names_changed && unit.Type->BoolFlag[ORGANIC_INDEX].value && !unit.Character && unit.Type->get_civilization() != nullptr && unit.Type->get_civilization()->get_species() == faction->get_civilization()->get_species() && unit.Type == faction->get_class_unit_type(unit.Type->get_unit_class())) {
			unit.UpdatePersonalName();
		}
		unit.UpdateSoldUnits();
		unit.UpdateButtonIcons();
	}
}

/**
**  Change player faction to a randomly chosen one.
**
**  @param faction    New faction.
*/
void CPlayer::SetRandomFaction()
{
	// set random one from the civilization's factions
	std::vector<wyrmgus::faction *> local_factions;
	
	for (wyrmgus::faction *faction : wyrmgus::faction::get_all()) {
		if (faction->get_civilization()->ID != this->Race) {
			continue;
		}
		if (!faction->Playable) {
			continue;
		}
		if (!this->can_found_faction(faction)) {
			continue;
		}

		int faction_type = faction->Type;
		const bool has_writing = this->has_upgrade_class(wyrmgus::upgrade_class::get("writing"));
		if (
			!(faction_type == FactionTypeTribe && !has_writing)
			&& !(faction_type == FactionTypePolity && has_writing)
		) {
			continue;
		}

		local_factions.push_back(faction);
	}
	
	if (local_factions.size() > 0) {
		wyrmgus::faction *chosen_faction = local_factions[SyncRand(local_factions.size())];
		this->SetFaction(chosen_faction);
	} else {
		this->SetFaction(nullptr);
	}
}

void CPlayer::set_dynasty(const wyrmgus::dynasty *dynasty)
{
	if (dynasty == this->get_dynasty()) {
		return;
	}

	const wyrmgus::dynasty *old_dynasty = this->dynasty;
	
	if (old_dynasty != nullptr) {
		if (old_dynasty->get_upgrade() != nullptr && this->Allow.Upgrades[old_dynasty->get_upgrade()->ID] == 'R') {
			UpgradeLost(*this, old_dynasty->get_upgrade()->ID);
		}
	}

	this->dynasty = dynasty;

	if (dynasty == nullptr) {
		return;
	}
	
	if (dynasty->get_upgrade() != nullptr) {
		if (this->Allow.Upgrades[dynasty->get_upgrade()->ID] != 'R') {
			if (GameEstablishing) {
				AllowUpgradeId(*this, dynasty->get_upgrade()->ID, 'R');
			} else {
				UpgradeAcquire(*this, dynasty->get_upgrade());
			}
		}
	}

	for (int i = 0; i < this->GetUnitCount(); ++i) {
		CUnit &unit = this->GetUnit(i);
		unit.UpdateSoldUnits(); //in case conditions changed (e.g. some heroes may require a certain dynasty)
	}
}

const std::string &CPlayer::get_interface() const
{
	const wyrmgus::civilization *civilization = this->get_civilization();
	if (civilization != nullptr) {
		return civilization->get_interface();
	}

	return string::empty_str;
}

/**
**	@brief	Check which age fits the player's current situation best, and set it as the player's age
*/
void CPlayer::check_age()
{
	//pick an age which fits the player, giving priority to the first ones (ages are already sorted by priority)
	
	for (wyrmgus::age *potential_age : wyrmgus::age::get_all()) {
		if (!check_conditions(potential_age, this)) {
			continue;
		}
		
		this->set_age(potential_age);
		return;
	}
	
	this->set_age(nullptr);
}

/**
**	@brief	Set the player's age
**
**	@param	age	The age to be set for the player
*/
void CPlayer::set_age(const wyrmgus::age *age)
{
	if (this->age == age) {
		return;
	}
	
	this->age = age;
	
	if (this == CPlayer::GetThisPlayer()) {
		if (this->age) {
			UI.AgePanel.Text = this->age->get_name();
			
			if (GameCycle > 0 && !SaveGameLoading) {
				this->Notify(_("The %s has dawned upon us."), _(this->age->get_name().c_str()));
			}
		} else {
			UI.AgePanel.Text.clear();
		}
	}
	
	wyrmgus::age::check_current_age();
}

/**
**	@brief	Get the player's currency
**
**	@return	The player's currency
*/
CCurrency *CPlayer::GetCurrency() const
{
	if (this->Faction != -1) {
		return wyrmgus::faction::get_all()[this->Faction]->GetCurrency();
	}
	
	if (this->Race != -1) {
		return wyrmgus::civilization::get_all()[this->Race]->GetCurrency();
	}
	
	return nullptr;
}

void CPlayer::ShareUpgradeProgress(CPlayer &player, CUnit &unit)
{
	std::vector<const CUpgrade *> upgrade_list = this->GetResearchableUpgrades();
	std::vector<const CUpgrade *> potential_upgrades;

	for (size_t i = 0; i < upgrade_list.size(); ++i) {
		if (this->Allow.Upgrades[upgrade_list[i]->ID] != 'R') {
			continue;
		}
		
		if (upgrade_list[i]->get_upgrade_class() == nullptr) {
			continue;
		}

		if (player.Faction == -1) {
			continue;
		}
		
		CUpgrade *upgrade = wyrmgus::faction::get_all()[player.Faction]->get_class_upgrade(upgrade_list[i]->get_upgrade_class());
		if (upgrade == nullptr) {
			continue;
		}
		
		if (player.Allow.Upgrades[upgrade->ID] != 'A' || !check_conditions(upgrade, &player)) {
			continue;
		}
	
		if (player.UpgradeRemovesExistingUpgrade(upgrade, player.AiEnabled)) {
			continue;
		}
		
		potential_upgrades.push_back(upgrade);
	}
	
	if (potential_upgrades.size() > 0) {
		const CUpgrade *chosen_upgrade = potential_upgrades[SyncRand(potential_upgrades.size())];
		
		if (!chosen_upgrade->get_name().empty()) {
			player.Notify(NotifyGreen, unit.tilePos, unit.MapLayer->ID, _("%s acquired through contact with %s"), chosen_upgrade->get_name().c_str(), this->Name.c_str());
		}
		if (&player == CPlayer::GetThisPlayer()) {
			wyrmgus::sound *sound = GameSounds.ResearchComplete[player.Race].Sound;
			if (sound == nullptr) {
				sound = GameSounds.WorkComplete[player.Race].Sound;
			}

			if (sound != nullptr) {
				PlayGameSound(sound, MaxSampleVolume);
			}
		}
		if (player.AiEnabled) {
			AiResearchComplete(unit, chosen_upgrade);
		}
		UpgradeAcquire(player, chosen_upgrade);
	}
}

int CPlayer::get_player_color_usage_count(const wyrmgus::player_color *player_color) const
{
	int count = 0;

	for (int i = 0; i < PlayerMax; ++i) {
		if (this->Index != i && CPlayer::Players[i]->Faction != -1 && CPlayer::Players[i]->Type != PlayerNobody && CPlayer::Players[i]->get_player_color() == player_color) {
			count++;
		}		
	}

	return count;
}

void CPlayer::update_minimap_territory()
{
	for (const auto &kv_pair : this->get_units_by_type()) {
		const wyrmgus::unit_type *unit_type = kv_pair.first;
		if (!unit_type->BoolFlag[TOWNHALL_INDEX].value) {
			continue;
		}

		for (const CUnit *town_hall : kv_pair.second) {
			town_hall->settlement->update_minimap_territory();
		}
	}

	//also update the minimap territory of vassals, as they get strokes of the overlord's colors
	for (CPlayer *vassal : this->get_vassals()) {
		vassal->update_minimap_territory();
	}
}

wyrmgus::unit_type *CPlayer::get_class_unit_type(const wyrmgus::unit_class *unit_class) const
{
	const wyrmgus::faction *faction = this->get_faction();
	if (faction == nullptr) {
		return nullptr;
	}

	return faction->get_class_unit_type(unit_class);
}

CUpgrade *CPlayer::get_class_upgrade(const wyrmgus::upgrade_class *upgrade_class) const
{
	const wyrmgus::faction *faction = this->get_faction();
	if (faction == nullptr) {
		return nullptr;
	}

	return faction->get_class_upgrade(upgrade_class);
}

bool CPlayer::has_upgrade_class(const wyrmgus::upgrade_class *upgrade_class) const
{
	if (this->Race == -1 || upgrade_class == nullptr) {
		return false;
	}
	
	const CUpgrade *upgrade = nullptr;
	
	if (this->Faction != -1) {
		upgrade = wyrmgus::faction::get_all()[this->Faction]->get_class_upgrade(upgrade_class);
	} else {
		upgrade = wyrmgus::civilization::get_all()[this->Race]->get_class_upgrade(upgrade_class);
	}
	
	if (upgrade != nullptr && this->Allow.Upgrades[upgrade->ID] == 'R') {
		return true;
	}

	return false;
}

bool CPlayer::HasSettlement(const wyrmgus::site *settlement) const
{
	if (!settlement) {
		return false;
	}
	
	if (settlement->get_site_unit() && settlement->get_site_unit()->Player == this) {
		return true;
	}

	return false;
}

bool CPlayer::HasSettlementNearWaterZone(int water_zone) const
{
	std::vector<CUnit *> settlement_unit_table;
	
	const wyrmgus::unit_type *town_hall_type = wyrmgus::faction::get_all()[this->Faction]->get_class_unit_type(wyrmgus::defines::get()->get_town_hall_class());
	if (town_hall_type == nullptr) {
		return false;
	}
	
	FindPlayerUnitsByType(*this, *town_hall_type, settlement_unit_table, true);

	std::vector<const wyrmgus::unit_type *> additional_town_hall_types;

	for (const wyrmgus::unit_class *additional_town_hall_class : wyrmgus::unit_class::get_town_hall_classes()) {
		const wyrmgus::unit_type *additional_town_hall_type = wyrmgus::faction::get_all()[this->Faction]->get_class_unit_type(additional_town_hall_class);

		if (additional_town_hall_type == nullptr) {
			continue;
		}

		FindPlayerUnitsByType(*this, *additional_town_hall_type, settlement_unit_table, true); //add e.g. strongholds and fortresses to the table
	}

	for (size_t i = 0; i < settlement_unit_table.size(); ++i) {
		CUnit *settlement_unit = settlement_unit_table[i];
		if (!settlement_unit->IsAliveOnMap()) {
			continue;
		}
		
		int settlement_landmass = CMap::Map.GetTileLandmass(settlement_unit->tilePos, settlement_unit->MapLayer->ID);
		if (std::find(CMap::Map.BorderLandmasses[settlement_landmass].begin(), CMap::Map.BorderLandmasses[settlement_landmass].end(), water_zone) == CMap::Map.BorderLandmasses[settlement_landmass].end()) { //settlement's landmass doesn't even border the water zone, continue
			continue;
		}
		
		Vec2i pos(0, 0);
		if (FindTerrainType(0, nullptr, 8, *this, settlement_unit->tilePos, &pos, settlement_unit->MapLayer->ID, water_zone)) {
			return true;
		}
	}

	return false;
}

wyrmgus::site *CPlayer::GetNearestSettlement(const Vec2i &pos, int z, const Vec2i &size) const
{
	CUnit *best_hall = nullptr;
	int best_distance = -1;
	
	for (CUnit *settlement_unit : CMap::Map.get_settlement_units()) {
		if (!settlement_unit || !settlement_unit->IsAliveOnMap() || !settlement_unit->Type->BoolFlag[TOWNHALL_INDEX].value || z != settlement_unit->MapLayer->ID) {
			continue;
		}
		if (!this->HasNeutralFactionType() && this != settlement_unit->Player) {
			continue;
		}
		int distance = MapDistance(size, pos, z, settlement_unit->Type->get_tile_size(), settlement_unit->tilePos, settlement_unit->MapLayer->ID);
		if (!best_hall || distance < best_distance) {
			best_hall = settlement_unit;
			best_distance = distance;
		}
	}
	
	if (best_hall) {
		return best_hall->settlement;
	} else {
		return nullptr;
	}
}

void CPlayer::update_building_settlement_assignment(const wyrmgus::site *old_settlement, const int z) const
{
	for (int i = 0; i < this->GetUnitCount(); ++i) {
		CUnit *unit = &this->GetUnit(i);

		if (unit == nullptr || !unit->IsAliveOnMap()) {
			continue;
		}

		if (!unit->Type->BoolFlag[BUILDING_INDEX].value || unit->Type->BoolFlag[TOWNHALL_INDEX].value || unit->Type == settlement_site_unit_type || unit->MapLayer->ID != z) {
			continue;
		}

		if (old_settlement != nullptr && unit->settlement != old_settlement) {
			continue;
		}

		unit->UpdateSettlement();
	}
}

bool CPlayer::HasUnitBuilder(const wyrmgus::unit_type *type, const wyrmgus::site *settlement) const
{
	const std::vector<const wyrmgus::unit_type *> *builders = nullptr;
	const std::vector<const wyrmgus::unit_class *> *builder_classes = nullptr;

	if (type->BoolFlag[BUILDING_INDEX].value) {
		builders = &AiHelpers.get_builders(type);
		builder_classes = &AiHelpers.get_builder_classes(type->get_unit_class());
	} else {
		builders = &AiHelpers.get_trainers(type);
		builder_classes = &AiHelpers.get_trainer_classes(type->get_unit_class());
	}

	for (const wyrmgus::unit_type *builder : *builders) {
		if (this->GetUnitTypeCount(builder) > 0) {
			return true;
		}
	}

	if (this->get_faction() != nullptr) {
		for (const wyrmgus::unit_class *builder_class : *builder_classes) {
			const wyrmgus::unit_type *builder = this->get_faction()->get_class_unit_type(builder_class);

			if (builder == nullptr) {
				continue;
			}

			if (this->GetUnitTypeCount(builder) > 0) {
				return true;
			}
		}
	}

	const std::vector<const wyrmgus::unit_type *> &unit_type_upgradees = AiHelpers.get_unit_type_upgradees(type);
	const std::vector<const wyrmgus::unit_class *> &unit_class_upgradees = AiHelpers.get_unit_class_upgradees(type->get_unit_class());

	for (const wyrmgus::unit_type *unit_type_upgradee : unit_type_upgradees) {
		if (this->GetUnitTypeCount(unit_type_upgradee) > 0) {
			if (!settlement) {
				return true;
			} else {
				for (int i = 0; i < this->GetUnitCount(); ++i) {
					CUnit &unit = this->GetUnit(i);
					if (unit.Type == unit_type_upgradee && unit.settlement == settlement) {
						return true;
					}
				}
			}
		}
	}

	if (this->get_faction() != nullptr) {
		for (const wyrmgus::unit_class *unit_class_upgradee : unit_class_upgradees) {
			const wyrmgus::unit_type *unit_type_upgradee = this->get_faction()->get_class_unit_type(unit_class_upgradee);

			if (unit_type_upgradee == nullptr) {
				continue;
			}

			if (this->GetUnitTypeCount(unit_type_upgradee) > 0) {
				if (!settlement) {
					return true;
				} else {
					for (int i = 0; i < this->GetUnitCount(); ++i) {
						CUnit &unit = this->GetUnit(i);
						if (unit.Type->get_unit_class() == unit_class_upgradee && unit.settlement == settlement) {
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}

bool CPlayer::HasUpgradeResearcher(const CUpgrade *upgrade) const
{
	for (const wyrmgus::unit_type *researcher_type : AiHelpers.get_researchers(upgrade)) {
		if (this->GetUnitTypeCount(researcher_type) > 0 || HasUnitBuilder(researcher_type)) {
			return true;
		}
	}

	for (const wyrmgus::unit_class *researcher_class : AiHelpers.get_researcher_classes(upgrade->get_upgrade_class())) {
		const wyrmgus::unit_type *researcher_type = this->get_class_unit_type(researcher_class);
		if (researcher_type != nullptr && (this->GetUnitTypeCount(researcher_type) > 0 || this->HasUnitBuilder(researcher_type))) {
			return true;
		}
	}

	return false;
}

template <bool preconditions_only>
bool CPlayer::can_found_faction(const wyrmgus::faction *faction) const
{
	if (CurrentQuest != nullptr) {
		return false;
	}
	
	if (!faction->FactionUpgrade.empty()) {
		CUpgrade *faction_upgrade = CUpgrade::get(faction->FactionUpgrade);
		if (!check_conditions<preconditions_only>(faction_upgrade, this, false)) {
			return false;
		}
	}

	for (int i = 0; i < PlayerMax; ++i) {
		if (this->Index != i && CPlayer::Players[i]->Type != PlayerNobody && CPlayer::Players[i]->Race == faction->get_civilization()->ID && CPlayer::Players[i]->Faction == faction->ID) {
			// faction is already in use
			return false;
		}
	}

	if (faction->get_preconditions() != nullptr && !faction->get_preconditions()->check(this)) {
		return false;
	}

	if constexpr (!preconditions_only) {
		//check if the required core settlements are owned by the player
		if (wyrmgus::game::get()->get_current_campaign() != nullptr) { //only check for settlements in the Scenario mode
			for (const wyrmgus::site *core_settlement : faction->get_core_settlements()) {
				if (!core_settlement->get_site_unit() || core_settlement->get_site_unit()->Player != this || core_settlement->get_site_unit()->CurrentAction() == UnitAction::Built) {
					return false;
				}
			}
		}

		if (faction->get_conditions() != nullptr && !faction->get_conditions()->check(this)) {
			return false;
		}
	}
	
	return true;
}

template bool CPlayer::can_found_faction<false>(const wyrmgus::faction *faction) const;
template bool CPlayer::can_found_faction<true>(const wyrmgus::faction *faction) const;

template <bool preconditions_only>
bool CPlayer::can_choose_dynasty(const wyrmgus::dynasty *dynasty) const
{
	if (CurrentQuest != nullptr) {
		return false;
	}
	
	if (dynasty->get_upgrade() == nullptr) {
		return false;
	}

	if (!check_conditions<preconditions_only>(dynasty->get_upgrade(), this, false)) {
		return false;
	}

	return check_conditions<preconditions_only>(dynasty, this, false);
}

template bool CPlayer::can_choose_dynasty<false>(const wyrmgus::dynasty *dynasty) const;
template bool CPlayer::can_choose_dynasty<true>(const wyrmgus::dynasty *dynasty) const;


bool CPlayer::is_character_available_for_recruitment(const wyrmgus::character *character, bool ignore_neutral) const
{
	if (character->is_deity()) {
		return false;
	}
	
	if (!character->get_civilization() || character->get_civilization()->ID != this->Race) {
		return false;
	}
	
	if (character->get_conditions() != nullptr && !character->get_conditions()->check(this)) {
		return false;
	}
	
	//the conditions for the character's unit type must be fulfilled
	if (!check_conditions(character->get_unit_type(), this, true)) {
		return false;
	}
	
	if (character->Conditions) {
		CclCommand("trigger_player = " + std::to_string(this->Index) + ";");
		character->Conditions->pushPreamble();
		character->Conditions->run(1);
		if (character->Conditions->popBoolean() == false) {
			return false;
		}
	}
	
	if (!character->CanAppear(ignore_neutral)) {
		return false;
	}
	
	return true;
}

std::vector<wyrmgus::character *> CPlayer::get_recruitable_heroes_from_list(const std::vector<wyrmgus::character *> &heroes)
{
	std::vector<wyrmgus::character *> recruitable_heroes;

	for (wyrmgus::character *hero : heroes) {
		if (this->is_character_available_for_recruitment(hero)) {
			recruitable_heroes.push_back(hero);
		}
	}

	return recruitable_heroes;
}
/**
**  Check if the upgrade removes an existing upgrade of the player.
**
**  @param upgrade    Upgrade.
*/
bool CPlayer::UpgradeRemovesExistingUpgrade(const CUpgrade *upgrade, bool ignore_lower_priority) const
{
	for (const auto &modifier : upgrade->get_modifiers()) {
		for (size_t j = 0; j < modifier->RemoveUpgrades.size(); ++j) {
			const CUpgrade *removed_upgrade = modifier->RemoveUpgrades[j];
			bool has_upgrade = this->AiEnabled ? AiHasUpgrade(*this->Ai, removed_upgrade, true) : (UpgradeIdAllowed(*this, removed_upgrade->ID) == 'R');
			if (has_upgrade) {
				if (ignore_lower_priority && this->Faction != -1 && wyrmgus::faction::get_all()[this->Faction]->GetUpgradePriority(removed_upgrade) < wyrmgus::faction::get_all()[this->Faction]->GetUpgradePriority(upgrade)) {
					continue;
				}
				return true;
			}
		}
	}
	
	return false;
}

std::string CPlayer::get_full_name() const
{
	if (!IsNetworkGame()) {
		const wyrmgus::faction *faction = this->get_faction();

		if (faction != nullptr && !faction->uses_simple_name()) {
			if (faction->uses_short_name()) {
				return faction->Adjective + " " + std::string(this->get_faction_title_name());
			} else {
				return std::string(this->get_faction_title_name()) + " of " + this->Name;
			}
		}
	}

	return this->Name;
}

std::string_view CPlayer::get_faction_title_name() const
{
	if (this->Race == -1 || this->Faction == -1) {
		return string::empty_str;
	}
	
	const wyrmgus::faction *faction = this->get_faction();
	const wyrmgus::government_type government_type = this->get_government_type();
	const wyrmgus::faction_tier tier = this->get_faction_tier();

	return faction->get_title_name(government_type, tier);
}

std::string_view CPlayer::GetCharacterTitleName(const wyrmgus::character_title title_type, const wyrmgus::gender gender) const
{
	if (this->get_faction() == nullptr || title_type == wyrmgus::character_title::none || gender == wyrmgus::gender::none) {
		return string::empty_str;
	}
	
	const wyrmgus::faction *faction = wyrmgus::faction::get_all()[this->Faction];
	const wyrmgus::government_type government_type = this->get_government_type();
	const wyrmgus::faction_tier tier = this->get_faction_tier();

	return faction->get_character_title_name(title_type, government_type, tier, gender);
}

std::set<int> CPlayer::get_builder_landmasses(const wyrmgus::unit_type *building) const
{
	std::set<int> builder_landmasses;

	for (const wyrmgus::unit_type *builder_type : AiHelpers.get_builders(building)) {
		if (this->GetUnitTypeAiActiveCount(builder_type) > 0) {
			std::vector<CUnit *> builder_table;

			FindPlayerUnitsByType(*this, *builder_type, builder_table, true);

			for (const CUnit *builder : builder_table) {
				const int landmass = CMap::Map.GetTileLandmass(builder->tilePos, builder->MapLayer->ID);
				builder_landmasses.insert(landmass);
			}
		}
	}

	if (this->Faction != -1) {
		for (const wyrmgus::unit_class *builder_class : AiHelpers.get_builder_classes(building->get_unit_class())) {
			const wyrmgus::unit_type *builder_type = wyrmgus::faction::get_all()[this->Faction]->get_class_unit_type(builder_class);

			if (this->GetUnitTypeAiActiveCount(builder_type) > 0) {
				std::vector<CUnit *> builder_table;

				FindPlayerUnitsByType(*this, *builder_type, builder_table, true);

				for (const CUnit *builder : builder_table) {
					const int landmass = CMap::Map.GetTileLandmass(builder->tilePos, builder->MapLayer->ID);
					builder_landmasses.insert(landmass);
				}
			}
		}
	}

	return builder_landmasses;
}

std::vector<const CUpgrade *> CPlayer::GetResearchableUpgrades()
{
	std::vector<const CUpgrade *> researchable_upgrades;

	for (const auto &kv_pair : this->UnitTypesAiActiveCount) {
		const wyrmgus::unit_type *type = kv_pair.first;

		for (const CUpgrade *upgrade : AiHelpers.get_researched_upgrades(type)) {
			if (!wyrmgus::vector::contains(researchable_upgrades, upgrade)) {
				researchable_upgrades.push_back(upgrade);
			}
		}

		for (const wyrmgus::upgrade_class *upgrade_class : AiHelpers.get_researched_upgrade_classes(type->get_unit_class())) {
			const CUpgrade *upgrade = this->get_class_upgrade(upgrade_class);
			if (upgrade != nullptr && !wyrmgus::vector::contains(researchable_upgrades, upgrade)) {
				researchable_upgrades.push_back(upgrade);
			}
		}
	}
	
	return researchable_upgrades;
}
//Wyrmgus end

/**
**  Clear all player data excepts members which don't change.
**
**  The fields that are not cleared are
**  UnitLimit, BuildingLimit, TotalUnitLimit and Allow.
*/
void CPlayer::Clear()
{
	this->Index = 0;
	this->Name.clear();
	this->Type = 0;
	this->Race = wyrmgus::defines::get()->get_neutral_civilization()->ID;
	this->Faction = -1;
	this->faction_tier = wyrmgus::faction_tier::none;
	this->government_type = wyrmgus::government_type::none;
	this->religion = nullptr;
	this->dynasty = nullptr;
	this->age = nullptr;
	this->overlord = nullptr;
	this->vassalage_type = wyrmgus::vassalage_type::none;
	this->vassals.clear();
	this->AiName.clear();
	this->Team = 0;
	this->enemies.clear();
	this->allies.clear();
	this->shared_vision.clear();
	this->StartPos.x = 0;
	this->StartPos.y = 0;
	//Wyrmgus start
	this->StartMapLayer = 0;
	//Wyrmgus end
	memset(this->Resources, 0, sizeof(this->Resources));
	memset(this->StoredResources, 0, sizeof(this->StoredResources));
	memset(this->MaxResources, 0, sizeof(this->MaxResources));
	memset(this->LastResources, 0, sizeof(this->LastResources));
	memset(this->Incomes, 0, sizeof(this->Incomes));
	memset(this->Revenue, 0, sizeof(this->Revenue));
	//Wyrmgus start
	memset(this->ResourceDemand, 0, sizeof(this->ResourceDemand));
	memset(this->StoredResourceDemand, 0, sizeof(this->StoredResourceDemand));
	//Wyrmgus end
	this->UnitTypesCount.clear();
	this->UnitTypesUnderConstructionCount.clear();
	this->UnitTypesAiActiveCount.clear();
	//Wyrmgus start
	this->Heroes.clear();
	this->Deities.clear();
	this->units_by_type.clear();
	this->units_by_class.clear();
	this->AiActiveUnitsByType.clear();
	this->available_quests.clear();
	this->current_quests.clear();
	this->completed_quests.clear();
	this->AutosellResources.clear();
	this->quest_objectives.clear();
	this->Modifiers.clear();
	//Wyrmgus end
	this->AiEnabled = false;
	this->revealed = false;
	this->Ai = 0;
	this->Units.resize(0);
	this->FreeWorkers.resize(0);
	//Wyrmgus start
	this->LevelUpUnits.resize(0);
	//Wyrmgus end
	this->NumBuildings = 0;
	//Wyrmgus start
	this->NumBuildingsUnderConstruction = 0;
	this->NumTownHalls = 0;
	//Wyrmgus end
	this->Supply = 0;
	this->Demand = 0;
	this->TradeCost = 0;
	// FIXME: can't clear limits since it's initialized already
	//	UnitLimit = 0;
	//	BuildingLimit = 0;
	//	TotalUnitLimit = 0;
	this->Score = 0;
	this->TotalUnits = 0;
	this->TotalBuildings = 0;
	memset(this->TotalResources, 0, sizeof(this->TotalResources));
	this->TotalRazings = 0;
	this->TotalKills = 0;
	//Wyrmgus start
	memset(this->UnitTypeKills, 0, sizeof(this->UnitTypeKills));
	this->LostTownHallTimer = 0;
	this->HeroCooldownTimer = 0;
	//Wyrmgus end
	this->UpgradeTimers.Clear();
	for (size_t i = 0; i < MaxCosts; ++i) {
		this->SpeedResourcesHarvest[i] = SPEEDUP_FACTOR;
		this->SpeedResourcesReturn[i] = SPEEDUP_FACTOR;
		if (i < wyrmgus::resource::get_all().size()) {
			this->Prices[i] = wyrmgus::resource::get_all()[i]->get_base_price();
		} else {
			this->Prices[i] = 0;
		}
	}
	this->SpeedBuild = SPEEDUP_FACTOR;
	this->SpeedTrain = SPEEDUP_FACTOR;
	this->SpeedUpgrade = SPEEDUP_FACTOR;
	this->SpeedResearch = SPEEDUP_FACTOR;
}


void CPlayer::AddUnit(CUnit &unit)
{
	Assert(unit.Player != this);
	Assert(unit.PlayerSlot == static_cast<size_t>(-1));
	unit.PlayerSlot = this->Units.size();
	this->Units.push_back(&unit);
	unit.Player = this;
	Assert(this->Units[unit.PlayerSlot] == &unit);
}

void CPlayer::RemoveUnit(CUnit &unit)
{
	Assert(unit.Player == this);
	//Wyrmgus start
	if (unit.PlayerSlot == -1 || this->Units[unit.PlayerSlot] != &unit) {
		fprintf(stderr, "Error in CPlayer::RemoveUnit: the unit's PlayerSlot doesn't match its position in the player's units array; Unit's PlayerSlot: %d, Unit Type: \"%s\".\n", unit.PlayerSlot, unit.Type ? unit.Type->Ident.c_str() : "");
		return;
	}
	//Wyrmgus end
	Assert(this->Units[unit.PlayerSlot] == &unit);

	//	unit.Player = nullptr; // we can remove dying unit...
	CUnit *last = this->Units.back();

	this->Units[unit.PlayerSlot] = last;
	last->PlayerSlot = unit.PlayerSlot;
	this->Units.pop_back();
	unit.PlayerSlot = static_cast<size_t>(-1);
	Assert(last == &unit || this->Units[last->PlayerSlot] == last);
}

void CPlayer::UpdateFreeWorkers()
{
	FreeWorkers.clear();
	if (FreeWorkers.capacity() != 0) {
		// Just calling FreeWorkers.clear() is not always appropriate.
		// Certain paths may leave FreeWorkers in an invalid state, so
		// it's safer to re-initialize.
		std::vector<CUnit*>().swap(FreeWorkers);
	}
	const int nunits = this->GetUnitCount();

	for (int i = 0; i < nunits; ++i) {
		CUnit &unit = this->GetUnit(i);
		//Wyrmgus start
//		if (unit.IsAlive() && unit.Type->BoolFlag[HARVESTER_INDEX].value && !unit.Removed) {
		if (unit.IsAlive() && unit.Type->BoolFlag[HARVESTER_INDEX].value && !unit.Removed && !unit.Type->BoolFlag[TRADER_INDEX].value) {
		//Wyrmgus end
			if (unit.CurrentAction() == UnitAction::Still) {
				FreeWorkers.push_back(&unit);
			}
		}
	}
}

//Wyrmgus start
void CPlayer::PerformResourceTrade()
{
	CUnit *market_unit = this->GetMarketUnit();
	
	if (!market_unit) {
		return;
	}
	
	for (size_t i = 0; i < this->AutosellResources.size(); ++i) {
		const int res = this->AutosellResources[i];
		
		if ((this->Resources[res] + this->StoredResources[res]) >= 100) { //sell 100 per second, as long as there is enough of the resource stored
			market_unit->SellResource(res, this->Index);
		}
		
		//increase price due to domestic demand
		this->StoredResourceDemand[res] += this->GetEffectiveResourceDemand(res);
		while (this->StoredResourceDemand[res] >= 100) {
			this->IncreaseResourcePrice(res);
			this->StoredResourceDemand[res] -= 100;
		}
	}
	
	for (size_t i = 0; i < LuxuryResources.size(); ++i) {
		const int res = LuxuryResources[i];
		
		while ((this->Resources[res] + this->StoredResources[res]) >= 100) {
			market_unit->SellResource(res, this->Index);
		}
		
		//increase price due to domestic demand
		this->StoredResourceDemand[res] += this->GetEffectiveResourceDemand(res);
		while (this->StoredResourceDemand[res] >= 100) {
			this->IncreaseResourcePrice(res);
			this->StoredResourceDemand[res] -= 100;
		}
	}
}

/**
**	@brief	Get whether the player has a market unit
**
**	@return	True if the player has a market unit, or false otherwise
*/
bool CPlayer::HasMarketUnit() const
{
	const int n_m = AiHelpers.SellMarkets[0].size();

	for (int i = 0; i < n_m; ++i) {
		const wyrmgus::unit_type &market_type = *AiHelpers.SellMarkets[0][i];

		if (this->GetUnitTypeCount(&market_type)) {
			return true;
		}
	}
	
	return false;
}

/**
**	@brief	Get the player's market unit, if any
**
**	@return	The market unit if present, or null otherwise
*/
CUnit *CPlayer::GetMarketUnit() const
{
	CUnit *market_unit = nullptr;
	
	const int n_m = AiHelpers.SellMarkets[0].size();

	for (int i = 0; i < n_m; ++i) {
		const wyrmgus::unit_type &market_type = *AiHelpers.SellMarkets[0][i];

		if (this->GetUnitTypeCount(&market_type)) {
			std::vector<CUnit *> market_table;
			FindPlayerUnitsByType(*this, market_type, market_table);
			if (market_table.size() > 0) {
				market_unit = market_table[SyncRand(market_table.size())];
				break;
			}
		}
	}
	
	return market_unit;
}

std::vector<int> CPlayer::GetAutosellResources() const
{
	return this->AutosellResources;
}

void CPlayer::AutosellResource(const int resource)
{
	if (std::find(this->AutosellResources.begin(), this->AutosellResources.end(), resource) != this->AutosellResources.end()) {
		this->AutosellResources.erase(std::remove(this->AutosellResources.begin(), this->AutosellResources.end(), resource), this->AutosellResources.end());
	} else {
		this->AutosellResources.push_back(resource);
	}
}

void CPlayer::UpdateLevelUpUnits()
{
	LevelUpUnits.clear();
	if (LevelUpUnits.capacity() != 0) {
		// Just calling LevelUpUnits.clear() is not always appropriate.
		// Certain paths may leave LevelUpUnits in an invalid state, so
		// it's safer to re-initialize.
		std::vector<CUnit*>().swap(LevelUpUnits);
	}
	const int nunits = this->GetUnitCount();

	for (int i = 0; i < nunits; ++i) {
		CUnit &unit = this->GetUnit(i);
		if (unit.IsAlive() && unit.Variable[LEVELUP_INDEX].Value >= 1) {
			LevelUpUnits.push_back(&unit);
		}
	}
}

void CPlayer::update_quest_pool()
{
	if (wyrmgus::game::get()->get_current_campaign() == nullptr) { // in-game quests only while playing the campaign mode
		return;
	}

	if (this->Faction == -1) {
		return;
	}
	
	const bool exausted_available_quests = this->available_quests.empty();
	
	this->available_quests.clear();
	
	std::vector<wyrmgus::quest *> potential_quests;
	for (wyrmgus::quest *quest : wyrmgus::quest::get_all()) {
		if (this->can_accept_quest(quest)) {
			potential_quests.push_back(quest);
		}
	}
	
	for (int i = 0; i < CPlayer::max_quest_pool; ++i) { // fill the quest pool with up to three quests
		if (potential_quests.size() == 0) {
			break;
		}
		wyrmgus::quest *quest = wyrmgus::vector::get_random(potential_quests);
		this->available_quests.push_back(quest);
		wyrmgus::vector::remove(potential_quests, quest);
	}
	
	this->on_available_quests_changed();

	// notify the player when new quests are available (but only if the player has already exausted the quests available to him, so that they aren't bothered if they choose not to engage with the quest system)
	if (this == CPlayer::GetThisPlayer() && GameCycle >= CYCLES_PER_MINUTE && this->available_quests.size() > 0 && exausted_available_quests && this->NumTownHalls > 0) {
		CPlayer::GetThisPlayer()->Notify("%s", _("New quests available"));
	}
	
	if (this->AiEnabled) { // if is an AI player, accept all quests that it can
		int available_quest_quantity = this->available_quests.size();
		for (int i = (available_quest_quantity  - 1); i >= 0; --i) {
			if (this->can_accept_quest(this->available_quests[i])) { // something may have changed, so recheck if the player is able to accept the quest
				this->accept_quest(this->available_quests[i]);
			}
		}
	}
}

void CPlayer::on_available_quests_changed()
{
	if (this == CPlayer::GetThisPlayer()) {
		for (wyrmgus::button *button : wyrmgus::button::get_all()) {
			if (button->Action != ButtonCmd::Quest || button->Value >= static_cast<int>(this->available_quests.size())) {
				continue;
			}
			
			const wyrmgus::quest *quest = this->available_quests[button->Value];
			button->Hint = "Quest: " + quest->get_name();
			button->Description = quest->get_description() + "\n \nObjectives:";
			for (const auto &objective : quest->get_objectives()) {
				button->Description += "\n- " + objective->get_objective_string();
			}
			for (const std::string &objective_string : quest->get_objective_strings()) {
				button->Description += "\n" + objective_string;
			}
			const std::string rewards_string = quest->get_rewards_string();
			if (!rewards_string.empty()) {
				button->Description += "\n \nRewards:\n" + rewards_string;
			}
			if (!quest->Hint.empty()) {
				button->Description += "\n \nHint: " + quest->Hint;
			}
			if (quest->HighestCompletedDifficulty > DifficultyNoDifficulty) {
				std::string highest_completed_difficulty;
				if (quest->HighestCompletedDifficulty == DifficultyEasy) {
					highest_completed_difficulty = "Easy";
				} else if (quest->HighestCompletedDifficulty == DifficultyNormal) {
					highest_completed_difficulty = "Normal";
				} else if (quest->HighestCompletedDifficulty == DifficultyHard) {
					highest_completed_difficulty = "Hard";
				} else if (quest->HighestCompletedDifficulty == DifficultyBrutal) {
					highest_completed_difficulty = "Brutal";
				}
				button->Description += "\n \nHighest Completed Difficulty: " + highest_completed_difficulty;
			}
			
		}
		
		if (!Selected.empty() && Selected[0]->Type->BoolFlag[TOWNHALL_INDEX].value) {
			UI.ButtonPanel.Update();
		}
	}
}

void CPlayer::update_current_quests()
{
	for (const auto &objective : this->get_quest_objectives()) {
		const wyrmgus::quest_objective *quest_objective = objective->get_quest_objective();
		switch (quest_objective->get_objective_type()) {
			case wyrmgus::objective_type::have_resource:
				objective->Counter = std::min(this->get_resource(wyrmgus::resource::get_all()[quest_objective->Resource], STORE_BOTH), quest_objective->get_quantity());
				break;
			case wyrmgus::objective_type::research_upgrade:
				objective->Counter = UpgradeIdAllowed(*this, quest_objective->Upgrade->ID) == 'R' ? 1 : 0;
				break;
			case wyrmgus::objective_type::recruit_hero:
				objective->Counter = this->HasHero(quest_objective->get_character()) ? 1 : 0;
				break;
			default:
				break;
		}
	}
	
	for (int i = (this->current_quests.size()  - 1); i >= 0; --i) {
		wyrmgus::quest *quest = this->current_quests[i];
		const std::string quest_failure_text = this->check_quest_failure(quest);
		if (!quest_failure_text.empty()) {
			this->fail_quest(quest, quest_failure_text);
		} else if (this->check_quest_completion(quest)) {
			this->complete_quest(quest);
		}
	}
}

void CPlayer::accept_quest(wyrmgus::quest *quest)
{
	if (!quest) {
		return;
	}
	
	wyrmgus::vector::remove(this->available_quests, quest);
	this->current_quests.push_back(quest);
	
	for (const auto &quest_objective : quest->get_objectives()) {
		auto objective = std::make_unique<wyrmgus::player_quest_objective>(quest_objective.get());
		this->quest_objectives.push_back(std::move(objective));
	}
	
	CclCommand("trigger_player = " + std::to_string(this->Index) + ";");
	
	if (quest->AcceptEffects) {
		quest->AcceptEffects->pushPreamble();
		quest->AcceptEffects->run();
	}

	if (quest->get_accept_effects() != nullptr) {
		quest->get_accept_effects()->do_effects(this);
	}

	this->on_available_quests_changed();
	
	this->update_current_quests();
}

void CPlayer::complete_quest(wyrmgus::quest *quest)
{
	if (wyrmgus::vector::contains(this->completed_quests, quest)) {
		return;
	}
	
	this->remove_current_quest(quest);
	
	this->completed_quests.push_back(quest);
	if (quest->Competitive) {
		quest->CurrentCompleted = true;
	}
	
	CclCommand("trigger_player = " + std::to_string(this->Index) + ";");
	
	if (quest->CompletionEffects) {
		quest->CompletionEffects->pushPreamble();
		quest->CompletionEffects->run();
	}

	if (quest->get_completion_effects() != nullptr) {
		quest->get_completion_effects()->do_effects(this);
	}
	
	if (this == CPlayer::GetThisPlayer()) {
		SetQuestCompleted(quest->get_identifier(), GameSettings.Difficulty);
		SaveQuestCompletion();

		const wyrmgus::campaign *current_campaign = wyrmgus::game::get()->get_current_campaign();
		if (current_campaign != nullptr && current_campaign->get_quest() == quest) {
			wyrmgus::defines::get()->get_campaign_victory_dialogue()->call(this);
		}

		std::string rewards_string = quest->get_rewards_string();
		if (!rewards_string.empty()) {
			string::replace(rewards_string, "\n", "\\n");
			string::replace(rewards_string, "\t", "\\t");
			rewards_string = "Rewards:\\n" + rewards_string;
		}
		CclCommand("if (GenericDialog ~= nil) then GenericDialog(\"Quest Completed\", \"You have completed the " + quest->get_name() + " quest!\\n\\n" + rewards_string + "\", nil, \"" + (quest->get_icon() ? quest->get_icon()->get_identifier() : "") + "\", \"" + (quest->get_player_color() ? quest->get_player_color()->get_identifier() : "") + "\", " + std::to_string(quest->get_icon() ? quest->get_icon()->get_frame() : 0) + ") end;");
	}
}

void CPlayer::fail_quest(wyrmgus::quest *quest, const std::string &fail_reason)
{
	this->remove_current_quest(quest);
	
	CclCommand("trigger_player = " + std::to_string(this->Index) + ";");
	
	if (quest->FailEffects) {
		quest->FailEffects->pushPreamble();
		quest->FailEffects->run();
	}
	
	if (this == CPlayer::GetThisPlayer()) {
		const wyrmgus::campaign *current_campaign = wyrmgus::game::get()->get_current_campaign();
		if (current_campaign != nullptr && current_campaign->get_quest() == quest) {
			wyrmgus::defines::get()->get_campaign_defeat_dialogue()->call(this);
		}

		CclCommand("if (GenericDialog ~= nil) then GenericDialog(\"Quest Failed\", \"You have failed the " + quest->get_name() + " quest! " + fail_reason + "\", nil, \"" + (quest->get_icon() ? quest->get_icon()->get_identifier() : "") + "\", \"" + (quest->get_player_color() ? quest->get_player_color()->get_identifier() : "") + "\") end;");
	}
}

void CPlayer::remove_current_quest(wyrmgus::quest *quest)
{
	wyrmgus::vector::remove(this->current_quests, quest);
	
	for (int i = (this->quest_objectives.size()  - 1); i >= 0; --i) {
		if (this->quest_objectives[i]->get_quest_objective()->get_quest() == quest) {
			wyrmgus::vector::remove(this->quest_objectives, this->quest_objectives[i]);
		}
	}
}

bool CPlayer::can_accept_quest(const wyrmgus::quest *quest) const
{
	if (quest->Hidden || quest->CurrentCompleted || quest->is_unobtainable()) {
		return false;
	}
	
	if (wyrmgus::vector::contains(this->current_quests, quest) || wyrmgus::vector::contains(this->completed_quests, quest)) {
		return false;
	}

	if (quest->get_conditions() != nullptr && !quest->get_conditions()->check(this)) {
		return false;
	}
	
	int recruit_heroes_quantity = 0;
	for (const auto &objective : quest->get_objectives()) {
		if (objective->get_objective_type() == wyrmgus::objective_type::build_units) {
			std::vector<wyrmgus::unit_type *> unit_types = objective->UnitTypes;

			for (const wyrmgus::unit_class *unit_class : objective->get_unit_classes()) {
				wyrmgus::unit_type *unit_type = wyrmgus::faction::get_all()[this->Faction]->get_class_unit_type(unit_class);
				if (unit_type == nullptr) {
					continue;
				}
				unit_types.push_back(unit_type);
			}

			if (unit_types.empty()) {
				return false;
			}

			bool validated = false;
			for (const wyrmgus::unit_type *unit_type : unit_types) {
				if (objective->get_settlement() != nullptr && !this->HasSettlement(objective->get_settlement()) && !unit_type->BoolFlag[TOWNHALL_INDEX].value) {
					continue;
				}

				if (!this->HasUnitBuilder(unit_type, objective->get_settlement()) || !check_conditions(unit_type, this)) {
					continue;
				}

				validated = true;
			}

			if (!validated) {
				return false;
			}
		} else if (objective->get_objective_type() == wyrmgus::objective_type::research_upgrade) {
			const CUpgrade *upgrade = objective->Upgrade;
			
			bool has_researcher = this->HasUpgradeResearcher(upgrade);
				
			if (!has_researcher) { //check if the quest includes an objective to build a researcher of the upgrade
				for (const auto &second_objective : quest->get_objectives()) {
					if (second_objective == objective) {
						continue;
					}
						
					if (second_objective->get_objective_type() == wyrmgus::objective_type::build_units) {
						std::vector<wyrmgus::unit_type *> unit_types = second_objective->UnitTypes;

						for (const wyrmgus::unit_class *unit_class : second_objective->get_unit_classes()) {
							wyrmgus::unit_type *unit_type = wyrmgus::faction::get_all()[this->Faction]->get_class_unit_type(unit_class);
							if (unit_type == nullptr) {
								continue;
							}
							unit_types.push_back(unit_type);
						}

						if (unit_types.empty()) {
							continue;
						}

						for (const wyrmgus::unit_type *unit_type : unit_types) {
							if (wyrmgus::vector::contains(AiHelpers.get_researchers(upgrade), unit_type) || wyrmgus::vector::contains(AiHelpers.get_researcher_classes(upgrade->get_upgrade_class()), unit_type->get_unit_class())) { //if the unit type of the other objective is a researcher of this upgrade
								has_researcher = true;
								break;
							}
						}

						if (has_researcher) {
							break;
						}
					}
				}
			}
				
			if (!has_researcher || this->Allow.Upgrades[upgrade->ID] != 'A' || !check_conditions(upgrade, this)) {
				return false;
			}
		} else if (objective->get_objective_type() == wyrmgus::objective_type::recruit_hero) {
			if (!this->is_character_available_for_recruitment(objective->get_character(), true)) {
				return false;
			}
			recruit_heroes_quantity++;
		} else if (objective->get_objective_type() == wyrmgus::objective_type::destroy_units || objective->get_objective_type() == wyrmgus::objective_type::destroy_hero || objective->get_objective_type() == wyrmgus::objective_type::destroy_unique) {
			if (objective->get_faction() != nullptr) {
				CPlayer *faction_player = GetFactionPlayer(objective->get_faction());
				if (faction_player == nullptr || !faction_player->is_alive()) {
					return false;
				}
				
				if (objective->get_settlement() != nullptr && !faction_player->HasSettlement(objective->get_settlement())) {
					return false;
				}
			}
			
			if (objective->get_objective_type() == wyrmgus::objective_type::destroy_hero) {
				if (objective->get_character()->CanAppear()) { //if the character "can appear" it doesn't already exist, and thus can't be destroyed
					return false;
				}
			} else if (objective->get_objective_type() == wyrmgus::objective_type::destroy_unique) {
				if (objective->Unique->can_drop()) { //if the unique "can drop" it doesn't already exist, and thus can't be destroyed
					return false;
				}
			}
		} else if (objective->get_objective_type() == wyrmgus::objective_type::destroy_faction) {
			CPlayer *faction_player = GetFactionPlayer(objective->get_faction());
			if (faction_player == nullptr || !faction_player->is_alive()) {
				return false;
			}
		}
	}
	
	if (recruit_heroes_quantity > 0 && (this->Heroes.size() + recruit_heroes_quantity) > PlayerHeroMax) {
		return false;
	}
	
	for (const wyrmgus::character *hero : quest->HeroesMustSurvive) {
		if (!this->HasHero(hero)) {
			return false;
		}
	}

	if (quest->Conditions) {
		CclCommand("trigger_player = " + std::to_string(this->Index) + ";");
		quest->Conditions->pushPreamble();
		quest->Conditions->run(1);
		return quest->Conditions->popBoolean();
	} else {
		return true;
	}
}

bool CPlayer::check_quest_completion(const wyrmgus::quest *quest) const
{
	if (quest->is_uncompleteable()) {
		return false;
	}
	
	for (const auto &objective : this->get_quest_objectives()) {
		const wyrmgus::quest_objective *quest_objective = objective->get_quest_objective();
		if (quest_objective->get_quest() != quest) {
			continue;
		}
		if (quest_objective->get_quantity() > 0 && objective->Counter < quest_objective->get_quantity()) {
			return false;
		}
	}
	
	return true;
}

//returns the reason for failure (empty if none)
std::string CPlayer::check_quest_failure(const wyrmgus::quest *quest) const
{
	for (size_t i = 0; i < quest->HeroesMustSurvive.size(); ++i) { // put it here, because "unfailable" quests should also fail when a hero which should survive dies
		if (!this->HasHero(quest->HeroesMustSurvive[i])) {
			return "A hero necessary for the quest has died.";
		}
	}
	
	if (quest->CurrentCompleted) { // quest already completed by someone else
		return "Another faction has completed the quest before you could.";
	}

	if (quest->is_unfailable()) {
		return "";
	}

	for (const auto &objective : this->get_quest_objectives()) {
		const wyrmgus::quest_objective *quest_objective = objective->get_quest_objective();
		if (quest_objective->get_quest() != quest) {
			continue;
		}
		if (quest_objective->get_objective_type() == wyrmgus::objective_type::build_units) {
			if (objective->Counter < quest_objective->get_quantity()) {
				std::vector<wyrmgus::unit_type *> unit_types = quest_objective->UnitTypes;

				for (const wyrmgus::unit_class *unit_class : quest_objective->get_unit_classes()) {
					wyrmgus::unit_type *unit_type = wyrmgus::faction::get_all()[this->Faction]->get_class_unit_type(unit_class);
					if (unit_type == nullptr) {
						continue;
					}
					unit_types.push_back(unit_type);
				}

				if (unit_types.empty()) {
					return "You can no longer produce the required unit.";
				}

				bool validated = false;
				std::string validation_error;
				for (const wyrmgus::unit_type *unit_type : unit_types) {
					if (quest_objective->get_settlement() != nullptr && !this->HasSettlement(quest_objective->get_settlement()) && !unit_type->BoolFlag[TOWNHALL_INDEX].value) {
						validation_error = "You no longer hold the required settlement.";
						continue;
					}

					if (!this->HasUnitBuilder(unit_type, quest_objective->get_settlement()) || !check_conditions(unit_type, this)) {
						validation_error = "You can no longer produce the required unit.";
						continue;
					}

					validated = true;
				}

				if (!validated) {
					return validation_error;
				}
			}
		} else if (quest_objective->get_objective_type() == wyrmgus::objective_type::research_upgrade) {
			const CUpgrade *upgrade = quest_objective->Upgrade;
			
			if (this->Allow.Upgrades[upgrade->ID] != 'R') {
				bool has_researcher = this->HasUpgradeResearcher(upgrade);
				
				if (!has_researcher) { //check if the quest includes an objective to build a researcher of the upgrade
					for (const auto &second_objective : this->get_quest_objectives()) {
						const wyrmgus::quest_objective *second_quest_objective = second_objective->get_quest_objective();
						if (second_quest_objective->get_quest() != quest || second_objective == objective || second_objective->Counter >= second_quest_objective->get_quantity()) { //if the objective has been fulfilled, then there should be a researcher, if there isn't it is due to i.e. the researcher having been destroyed later on, or upgraded to another type, and then the quest should fail if the upgrade can no longer be researched
							continue;
						}
						
						if (second_quest_objective->get_objective_type() == wyrmgus::objective_type::build_units) {
							std::vector<wyrmgus::unit_type *> unit_types = second_quest_objective->UnitTypes;

							for (const wyrmgus::unit_class *unit_class : second_quest_objective->get_unit_classes()) {
								wyrmgus::unit_type *unit_type = wyrmgus::faction::get_all()[this->Faction]->get_class_unit_type(unit_class);
								if (unit_type == nullptr) {
									continue;
								}
								unit_types.push_back(unit_type);
							}

							if (unit_types.empty()) {
								continue;
							}

							for (const wyrmgus::unit_type *unit_type : unit_types) {
								if (wyrmgus::vector::contains(AiHelpers.get_researchers(upgrade), unit_type) || wyrmgus::vector::contains(AiHelpers.get_researcher_classes(upgrade->get_upgrade_class()), unit_type->get_unit_class())) { //if the unit type of the other objective is a researcher of this upgrade
									has_researcher = true;
									break;
								}
							}

							if (has_researcher) {
								break;
							}
						}
					}
				}
				
				if (!has_researcher || this->Allow.Upgrades[upgrade->ID] != 'A' || !check_conditions(upgrade, this)) {
					return "You can no longer research the required upgrade.";
				}
			}
		} else if (quest_objective->get_objective_type() == wyrmgus::objective_type::recruit_hero) {
			if (!this->HasHero(quest_objective->get_character()) && !this->is_character_available_for_recruitment(quest_objective->get_character(), true)) {
				return "The hero can no longer be recruited.";
			}
		} else if (quest_objective->get_objective_type() == wyrmgus::objective_type::destroy_units || quest_objective->get_objective_type() == wyrmgus::objective_type::destroy_hero || quest_objective->get_objective_type() == wyrmgus::objective_type::destroy_unique) {
			if (quest_objective->get_faction() != nullptr && objective->Counter < quest_objective->get_quantity()) {
				CPlayer *faction_player = GetFactionPlayer(quest_objective->get_faction());
				if (faction_player == nullptr || !faction_player->is_alive()) {
					return "The target no longer exists.";
				}
				
				if (quest_objective->get_settlement() != nullptr && !faction_player->HasSettlement(quest_objective->get_settlement())) {
					return "The target no longer exists.";
				}
			}
			
			if (quest_objective->get_objective_type() == wyrmgus::objective_type::destroy_hero) {
				if (objective->Counter == 0 && quest_objective->get_character()->CanAppear()) {  // if is supposed to destroy a character, but it is nowhere to be found, fail the quest
					return "The target no longer exists.";
				}
			} else if (quest_objective->get_objective_type() == wyrmgus::objective_type::destroy_unique) {
				if (objective->Counter == 0 && quest_objective->Unique->can_drop()) {  // if is supposed to destroy a unique, but it is nowhere to be found, fail the quest
					return "The target no longer exists.";
				}
			}
		} else if (quest_objective->get_objective_type() == wyrmgus::objective_type::destroy_faction) {
			if (objective->Counter == 0) {  // if is supposed to destroy a faction, but it is nowhere to be found, fail the quest
				CPlayer *faction_player = GetFactionPlayer(quest_objective->get_faction());
				if (faction_player == nullptr || !faction_player->is_alive()) {
					return "The target no longer exists.";
				}
			}
		}
	}
	
	return "";
}

bool CPlayer::has_quest(const wyrmgus::quest *quest) const
{
	return wyrmgus::vector::contains(this->current_quests, quest);
}

bool CPlayer::is_quest_completed(const wyrmgus::quest *quest) const
{
	return wyrmgus::vector::contains(this->completed_quests, quest);
}

void CPlayer::AddModifier(CUpgrade *modifier, int cycles)
{
	if (this->Allow.Upgrades[modifier->ID] == 'R') {
		for (size_t i = 0; i < this->Modifiers.size(); ++i) { //if already has the modifier, make it have the greater duration of the new or old one
			if (this->Modifiers[i].first == modifier) {
				this->Modifiers[i].second = std::max(this->Modifiers[i].second, (int) (GameCycle + cycles));
			}
		}
	} else {
		this->Modifiers.push_back(std::pair<CUpgrade *, int>(modifier, GameCycle + cycles));
		UpgradeAcquire(*this, modifier);
	}
	
}

void CPlayer::RemoveModifier(CUpgrade *modifier)
{
	if (this->Allow.Upgrades[modifier->ID] == 'R') {
		UpgradeLost(*this, modifier->ID);
		for (size_t i = 0; i < this->Modifiers.size(); ++i) { //if already has the modifier, make it have the greater duration of the new or old one
			if (this->Modifiers[i].first == modifier) {
				this->Modifiers.erase(std::remove(this->Modifiers.begin(), this->Modifiers.end(), this->Modifiers[i]), this->Modifiers.end());
				break;
			}
		}
	}
	
}

bool CPlayer::AtPeace() const
{
	for (int i = 0; i < PlayerNumNeutral; ++i) {
		if (this->IsEnemy(*CPlayer::Players[i]) && this->HasContactWith(*CPlayer::Players[i]) && CPlayer::Players[i]->is_alive()) {
			return false;
		}
	}
	
	return true;
}
//Wyrmgus end

std::vector<CUnit *>::const_iterator CPlayer::UnitBegin() const
{
	return Units.begin();
}

std::vector<CUnit *>::iterator CPlayer::UnitBegin()
{
	return Units.begin();
}

std::vector<CUnit *>::const_iterator CPlayer::UnitEnd() const
{
	return Units.end();
}

std::vector<CUnit *>::iterator CPlayer::UnitEnd()
{
	return Units.end();
}

CUnit &CPlayer::GetUnit(int index) const
{
	return *Units[index];
}

int CPlayer::GetUnitCount() const
{
	return static_cast<int>(Units.size());
}



/*----------------------------------------------------------------------------
--  Resource management
----------------------------------------------------------------------------*/

/**
**  Gets the player resource.
**
**  @param resource  Resource to get.
**  @param type      Storing type
**
**  @note Storing types: 0 - overall store, 1 - store buildings, 2 - both
*/
int CPlayer::get_resource(const wyrmgus::resource *resource, const int type)
{
	switch (type) {
		case STORE_OVERALL:
			return this->Resources[resource->get_index()];
		case STORE_BUILDING:
			return this->StoredResources[resource->get_index()];
		case STORE_BOTH:
			return this->Resources[resource->get_index()] + this->StoredResources[resource->get_index()];
		default:
			DebugPrint("Wrong resource type\n");
			return -1;
	}
}

/**
**  Adds/subtracts some resources to/from the player store
**
**  @param resource  Resource to add/subtract.
**  @param value     How many of this resource (can be negative).
**  @param store     If true, sets the building store resources, else the overall resources.
*/
void CPlayer::change_resource(const wyrmgus::resource *resource, const int value, const bool store)
{
	if (value < 0) {
		const int fromStore = std::min(this->StoredResources[resource->get_index()], abs(value));
		this->StoredResources[resource->get_index()] -= fromStore;
		this->Resources[resource->get_index()] -= abs(value) - fromStore;
		this->Resources[resource->get_index()] = std::max(this->Resources[resource->get_index()], 0);
	} else {
		if (store && this->MaxResources[resource->get_index()] != -1) {
			this->StoredResources[resource->get_index()] += std::min(value, this->MaxResources[resource->get_index()] - this->StoredResources[resource->get_index()]);
		} else {
			this->Resources[resource->get_index()] += value;
		}
	}
}

/**
**  Change the player resource.
**
**  @param resource  Resource to change.
**  @param value     How many of this resource.
**  @param type      Resource types: 0 - overall store, 1 - store buildings, 2 - both
*/
void CPlayer::set_resource(const wyrmgus::resource *resource, const int value, const int type)
{
	if (type == STORE_BOTH) {
		if (this->MaxResources[resource->get_index()] != -1) {
			const int toRes = std::max(0, value - this->StoredResources[resource->get_index()]);
			this->Resources[resource->get_index()] = std::max(0, toRes);
			this->StoredResources[resource->get_index()] = std::min(value - toRes, this->MaxResources[resource->get_index()]);
		} else {
			this->Resources[resource->get_index()] = value;
		}
	} else if (type == STORE_BUILDING && this->MaxResources[resource->get_index()] != -1) {
		this->StoredResources[resource->get_index()] = std::min(value, this->MaxResources[resource->get_index()]);
	} else if (type == STORE_OVERALL) {
		this->Resources[resource->get_index()] = value;
	}
}

/**
**  Check, if there enough resources for action.
**
**  @param resource  Resource to change.
**  @param value     How many of this resource.
*/
bool CPlayer::CheckResource(const int resource, const int value)
{
	int result = this->Resources[resource];
	if (this->MaxResources[resource] != -1) {
		result += this->StoredResources[resource];
	}
	return result < value ? false : true;
}

//Wyrmgus start
/**
**  Increase resource price
**
**  @param resource  Resource.
*/
void CPlayer::IncreaseResourcePrice(const int resource)
{
	int price_change = wyrmgus::resource::get_all()[resource]->get_base_price() / std::max(this->Prices[resource], 100);
	price_change = std::max(1, price_change);
	this->Prices[resource] += price_change;
}

/**
**  Decrease resource price
**
**  @param resource  Resource.
*/
void CPlayer::DecreaseResourcePrice(const int resource)
{
	int price_change = this->Prices[resource] / wyrmgus::resource::get_all()[resource]->get_base_price();
	price_change = std::max(1, price_change);
	this->Prices[resource] -= price_change;
	this->Prices[resource] = std::max(1, this->Prices[resource]);
}

/**
**  Converges prices with another player (and returns how many convergences were effected)
*/
int CPlayer::ConvergePricesWith(CPlayer &player, int max_convergences)
{
	int convergences = 0;
	
	bool converged = true;
	while (converged) {
		converged = false;

		for (int i = 1; i < MaxCosts; ++i) {
			if (!wyrmgus::resource::get_all()[i]->get_base_price()) {
				continue;
			}
			
			int convergence_increase = 100;
			
			if (this->Prices[i] < player.Prices[i] && convergences < max_convergences) {
				this->IncreaseResourcePrice(i);
				convergences += convergence_increase;
				converged = true;
				
				if (this->Prices[i] < player.Prices[i] && convergences < max_convergences) { //now do the convergence for the other side as well, if possible
					player.DecreaseResourcePrice(i);
					convergences += convergence_increase;
					converged = true;
				}
			} else if (this->Prices[i] > player.Prices[i] && convergences < max_convergences) {
				this->DecreaseResourcePrice(i);
				convergences += convergence_increase;
				converged = true;

				if (this->Prices[i] > player.Prices[i] && convergences < max_convergences) { //do the convergence for the other side as well, if possible
					player.IncreaseResourcePrice(i);
					convergences += convergence_increase;
					converged = true;
				}
			}
		}
	}
	
	return convergences;
}

/**
**  Get the price of a resource for the player
**
**  @param resource  Resource.
*/
int CPlayer::GetResourcePrice(const int resource) const
{
	if (resource == CopperCost) {
		return 100;
	}
	
	return this->Prices[resource];
}

/**
**  Get the effective resource demand for the player, given the current prices
**
**  @param resource  Resource.
*/
int CPlayer::GetEffectiveResourceDemand(const int resource) const
{
	int resource_demand = this->ResourceDemand[resource];
	
	if (this->Prices[resource]) {
		resource_demand *= wyrmgus::resource::get_all()[resource]->get_base_price();
		resource_demand /= this->Prices[resource];
	}
	
	if (wyrmgus::resource::get_all()[resource]->DemandElasticity != 100) {
		resource_demand = this->ResourceDemand[resource] + ((resource_demand - this->ResourceDemand[resource]) * wyrmgus::resource::get_all()[resource]->DemandElasticity / 100);
	}
	
	resource_demand = std::max(resource_demand, 0);

	return resource_demand;
}

/**
**  Get the effective sell price of a resource
*/
int CPlayer::GetEffectiveResourceSellPrice(const int resource, int traded_quantity) const
{
	if (resource == CopperCost) {
		return 100;
	}
	
	int price = traded_quantity * this->Prices[resource] / 100 * (100 - this->TradeCost) / 100;
	price = std::max(1, price);
	return price;
}

/**
**  Get the effective buy quantity of a resource
*/
int CPlayer::GetEffectiveResourceBuyPrice(const int resource, int traded_quantity) const
{
	int price = traded_quantity * this->Prices[resource] / 100 * 100 / (100 - this->TradeCost);
	price = std::max(1, price);
	return price;
}

/**
**  Get the total price difference between this player and another one
*/
int CPlayer::GetTotalPriceDifferenceWith(const CPlayer &player) const
{
	int difference = 0;
	for (int i = 1; i < MaxCosts; ++i) {
		if (!wyrmgus::resource::get_all()[i]->get_base_price()) {
			continue;
		}
		difference += abs(this->Prices[i] - player.Prices[i]);
	}

	return difference;
}

/**
**  Get the trade potential between this player and another one
*/
int CPlayer::GetTradePotentialWith(const CPlayer &player) const
{
	int trade_potential = 0;
	for (int i = 1; i < MaxCosts; ++i) {
		if (!wyrmgus::resource::get_all()[i]->get_base_price()) {
			continue;
		}
		int price_difference = abs(this->Prices[i] - player.Prices[i]);
		trade_potential += price_difference * 100;
	}
	
	trade_potential = std::max(trade_potential, 10);
	
	return trade_potential;
}
//Wyrmgus end

void CPlayer::pay_overlord_tax(const wyrmgus::resource *resource, const int taxable_quantity)
{
	if (this->get_overlord() == nullptr) {
		return;
	}

	//if the player has an overlord, give 10% of the resources gathered to them
	const int quantity = taxable_quantity / 10;

	if (quantity == 0) {
		return;
	}

	this->get_overlord()->change_resource(resource, quantity, true);
	this->get_overlord()->TotalResources[resource->get_index()] += quantity;
	this->change_resource(resource, -quantity, true);

	//make the overlord pay tax to their overlord in turn (if they have one)
	this->get_overlord()->pay_overlord_tax(resource, quantity);
}

int CPlayer::GetUnitTotalCount(const wyrmgus::unit_type &type) const
{
	int count = this->GetUnitTypeCount(&type);
	for (std::vector<CUnit *>::const_iterator it = this->UnitBegin(); it != this->UnitEnd(); ++it) {
		//Wyrmgus start
		if (*it == nullptr) {
			fprintf(stderr, "Error in CPlayer::GetUnitTotalCount: unit of player %d is null.\n", this->Index);
			continue;
		}
		//Wyrmgus end
		CUnit &unit = **it;

		if (unit.CurrentAction() == UnitAction::UpgradeTo) {
			COrder_UpgradeTo &order = dynamic_cast<COrder_UpgradeTo &>(*unit.CurrentOrder());
			if (order.GetUnitType().Slot == type.Slot) {
				++count;
			}
		}
	}
	return count;
}

/**
**  Check if the unit-type didn't break any unit limits.
**
**  @param type    Type of unit.
**
**  @return        True if enough, negative on problem.
**
**  @note The return values of the PlayerCheck functions are inconsistent.
*/
int CPlayer::CheckLimits(const wyrmgus::unit_type &type) const
{
	//  Check game limits.
	if (type.BoolFlag[BUILDING_INDEX].value && NumBuildings >= BuildingLimit) {
		Notify("%s", _("Building Limit Reached"));
		return -1;
	}
	if (!type.BoolFlag[BUILDING_INDEX].value && (this->GetUnitCount() - NumBuildings) >= UnitLimit) {
		Notify("%s", _("Unit Limit Reached"));
		return -2;
	}
	//Wyrmgus start
//	if (this->Demand + type.Stats[this->Index].Variables[DEMAND_INDEX].Value > this->Supply && type.Stats[this->Index].Variables[DEMAND_INDEX].Value) {
	if (this->Demand + (type.Stats[this->Index].Variables[DEMAND_INDEX].Value * (type.TrainQuantity ? type.TrainQuantity : 1)) > this->Supply && type.Stats[this->Index].Variables[DEMAND_INDEX].Value) {
	//Wyrmgus end
		//Wyrmgus start
//		Notify("%s", _("Insufficient Supply, increase Supply."));
		Notify("%s", _("Insufficient Food Supply, increase Food Supply."));
		//Wyrmgus end
		return -3;
	}
	if (this->GetUnitCount() >= TotalUnitLimit) {
		Notify("%s", _("Total Unit Limit Reached"));
		return -4;
	}
	if (GetUnitTotalCount(type) >= Allow.Units[type.Slot]) {
		Notify(_("Limit of %d reached for this unit type"), Allow.Units[type.Slot]);
		return -6;
	}
	return 1;
}

/**
**  Check if enough resources for are available.
**
**  @param costs   How many costs.
**
**  @return        False if all enough, otherwise a bit mask.
**
**  @note The return values of the PlayerCheck functions are inconsistent.
*/
int CPlayer::CheckCosts(const int *costs, bool notify) const
{
	//Wyrmgus start
	bool sound_played = false;
	//Wyrmgus end
	int err = 0;
	for (int i = 1; i < MaxCosts; ++i) {
		if (this->Resources[i] + this->StoredResources[i] >= costs[i]) {
			continue;
		}
		if (notify) {
			const char *name = DefaultResourceNames[i].c_str();
			const char *action_name = wyrmgus::resource::get_all()[i]->get_action_name().c_str();

			Notify(_("Not enough %s... %s more %s."), _(name), _(action_name), _(name));

			//Wyrmgus start
//			if (this == CPlayer::GetThisPlayer() && GameSounds.NotEnoughRes[this->Race][i].Sound) {
			if (this == CPlayer::GetThisPlayer() && GameSounds.NotEnoughRes[this->Race][i].Sound && !sound_played) {
				sound_played = true;
			//Wyrmgus end
				PlayGameSound(GameSounds.NotEnoughRes[this->Race][i].Sound, MaxSampleVolume);
			}
		}
		err |= 1 << i;
	}
	return err;
}

/**
**  Check if enough resources for new unit is available.
**
**  @param type    Type of unit.
**
**  @return        False if all enough, otherwise a bit mask.
*/
int CPlayer::CheckUnitType(const wyrmgus::unit_type &type, bool hire) const
{
	//Wyrmgus start
//	return this->CheckCosts(type.Stats[this->Index].Costs);
	int type_costs[MaxCosts];
	this->GetUnitTypeCosts(&type, type_costs, hire);
	return this->CheckCosts(type_costs);
	//Wyrmgus end
}

/**
**  Add costs to the resources
**
**  @param costs   How many costs.
*/
void CPlayer::AddCosts(const int *costs)
{
	for (int i = 1; i < MaxCosts; ++i) {
		change_resource(wyrmgus::resource::get_all()[i], costs[i], false);
	}
}

/**
**  Add the costs of an unit type to resources
**
**  @param type    Type of unit.
*/
void CPlayer::AddUnitType(const wyrmgus::unit_type &type, bool hire)
{
	//Wyrmgus start
//	AddCosts(type.Stats[this->Index].Costs);
	int type_costs[MaxCosts];
	this->GetUnitTypeCosts(&type, type_costs, hire);
	AddCostsFactor(type_costs, 100);
	//Wyrmgus end
}

/**
**  Add a factor of costs to the resources
**
**  @param costs   How many costs.
**  @param factor  Factor of the costs to apply.
*/
void CPlayer::AddCostsFactor(const int *costs, int factor)
{
	if (!factor) {
		return;
	}
	
	for (int i = 1; i < MaxCosts; ++i) {
		change_resource(wyrmgus::resource::get_all()[i], costs[i] * factor / 100, true);
	}
}

/**
**  Subtract costs from the resources
**
**  @param costs   How many costs.
*/
void CPlayer::SubCosts(const int *costs)
{
	for (int i = 1; i < MaxCosts; ++i) {
		this->change_resource(wyrmgus::resource::get_all()[i], -costs[i], true);
	}
}

/**
**  Subtract the costs of new unit from resources
**
**  @param type    Type of unit.
*/
void CPlayer::SubUnitType(const wyrmgus::unit_type &type, bool hire)
{
	//Wyrmgus start
//	this->SubCosts(type.Stats[this->Index].Costs);
	int type_costs[MaxCosts];
	this->GetUnitTypeCosts(&type, type_costs, hire);
	this->SubCostsFactor(type_costs, 100);
	//Wyrmgus end
}

/**
**  Subtract a factor of costs from the resources
**
**  @param costs   How many costs.
**  @param factor  Factor of the costs to apply.
*/
void CPlayer::SubCostsFactor(const int *costs, int factor)
{
	for (int i = 1; i < MaxCosts; ++i) {
		this->change_resource(wyrmgus::resource::get_all()[i], -costs[i] * 100 / factor);
	}
}

//Wyrmgus start
/**
**  Gives the cost of a unit type for the player
*/
void CPlayer::GetUnitTypeCosts(const wyrmgus::unit_type *type, int *type_costs, bool hire, bool ignore_one) const
{
	for (int i = 0; i < MaxCosts; ++i) {
		type_costs[i] = 0;
	}
	if (hire) {
		type_costs[CopperCost] = type->Stats[this->Index].GetPrice();
	} else {
		for (int i = 0; i < MaxCosts; ++i) {
			type_costs[i] = type->Stats[this->Index].Costs[i];
		}
	}
	for (int i = 0; i < MaxCosts; ++i) {
		if (type->TrainQuantity) {
			type_costs[i] *= type->TrainQuantity;
		}
		if (type->CostModifier) {
			int type_count = this->GetUnitTypeCount(type) + this->GetUnitTypeUnderConstructionCount(type);
			if (ignore_one) {
				type_count--;
			}
			for (int j = 0; j < type_count; ++j) {
				type_costs[i] *= 100 + type->CostModifier;
				type_costs[i] /= 100;
			}
		}
	}
}

int CPlayer::GetUnitTypeCostsMask(const wyrmgus::unit_type *type, bool hire) const
{
	int costs_mask = 0;
	
	int type_costs[MaxCosts];
	AiPlayer->Player->GetUnitTypeCosts(type, type_costs, hire);
	
	for (int i = 1; i < MaxCosts; ++i) {
		if (type_costs[i] > 0) {
			costs_mask |= 1 << i;
		}
	}
	
	return costs_mask;
}

/**
**  Gives the cost of an upgrade for the player
*/
void CPlayer::GetUpgradeCosts(const CUpgrade *upgrade, int *upgrade_costs)
{
	for (int i = 0; i < MaxCosts; ++i) {
		upgrade_costs[i] = upgrade->Costs[i];

		const wyrmgus::resource *resource = wyrmgus::resource::get_all()[i];

		const int scaled_cost = upgrade->get_scaled_cost(resource);
		if (scaled_cost > 0) {
			for (const wyrmgus::unit_type *unit_type : upgrade->get_scaled_cost_unit_types()) {
				upgrade_costs[i] += scaled_cost * this->GetUnitTypeCount(unit_type);
			}

			for (const wyrmgus::unit_class *unit_class : upgrade->get_scaled_cost_unit_classes()) {
				upgrade_costs[i] += scaled_cost * this->get_unit_class_count(unit_class);
			}
		}
	}
}

int CPlayer::GetUpgradeCostsMask(const CUpgrade *upgrade) const
{
	int costs_mask = 0;
	
	int upgrade_costs[MaxCosts];
	AiPlayer->Player->GetUpgradeCosts(upgrade, upgrade_costs);
	
	for (int i = 1; i < MaxCosts; ++i) {
		if (upgrade_costs[i] > 0) {
			costs_mask |= 1 << i;
		}
	}
	
	return costs_mask;
}

//Wyrmgus end

void CPlayer::SetUnitTypeCount(const wyrmgus::unit_type *type, int quantity)
{
	if (!type) {
		return;
	}
	
	if (quantity <= 0) {
		if (this->UnitTypesCount.find(type) != this->UnitTypesCount.end()) {
			this->UnitTypesCount.erase(type);
		}
	} else {
		this->UnitTypesCount[type] = quantity;
	}
}

void CPlayer::ChangeUnitTypeCount(const wyrmgus::unit_type *type, int quantity)
{
	this->SetUnitTypeCount(type, this->GetUnitTypeCount(type) + quantity);
}

int CPlayer::GetUnitTypeCount(const wyrmgus::unit_type *type) const
{
	if (type != nullptr && this->UnitTypesCount.find(type) != this->UnitTypesCount.end()) {
		return this->UnitTypesCount.find(type)->second;
	} else {
		return 0;
	}
}

void CPlayer::SetUnitTypeUnderConstructionCount(const wyrmgus::unit_type *type, int quantity)
{
	if (!type) {
		return;
	}
	
	if (quantity <= 0) {
		if (this->UnitTypesUnderConstructionCount.find(type) != this->UnitTypesUnderConstructionCount.end()) {
			this->UnitTypesUnderConstructionCount.erase(type);
		}
	} else {
		this->UnitTypesUnderConstructionCount[type] = quantity;
	}
}

void CPlayer::ChangeUnitTypeUnderConstructionCount(const wyrmgus::unit_type *type, int quantity)
{
	this->SetUnitTypeUnderConstructionCount(type, this->GetUnitTypeUnderConstructionCount(type) + quantity);
}

int CPlayer::GetUnitTypeUnderConstructionCount(const wyrmgus::unit_type *type) const
{
	if (type && this->UnitTypesUnderConstructionCount.find(type) != this->UnitTypesUnderConstructionCount.end()) {
		return this->UnitTypesUnderConstructionCount.find(type)->second;
	} else {
		return 0;
	}
}

void CPlayer::SetUnitTypeAiActiveCount(const wyrmgus::unit_type *type, int quantity)
{
	if (!type) {
		return;
	}
	
	if (quantity <= 0) {
		if (this->UnitTypesAiActiveCount.find(type) != this->UnitTypesAiActiveCount.end()) {
			this->UnitTypesAiActiveCount.erase(type);
		}
	} else {
		this->UnitTypesAiActiveCount[type] = quantity;
	}
}

void CPlayer::ChangeUnitTypeAiActiveCount(const wyrmgus::unit_type *type, int quantity)
{
	this->SetUnitTypeAiActiveCount(type, this->GetUnitTypeAiActiveCount(type) + quantity);
}

int CPlayer::GetUnitTypeAiActiveCount(const wyrmgus::unit_type *type) const
{
	if (type && this->UnitTypesAiActiveCount.find(type) != this->UnitTypesAiActiveCount.end()) {
		return this->UnitTypesAiActiveCount.find(type)->second;
	} else {
		return 0;
	}
}

void CPlayer::IncreaseCountsForUnit(CUnit *unit, bool type_change)
{
	const wyrmgus::unit_type *type = unit->Type;

	this->ChangeUnitTypeCount(type, 1);
	this->units_by_type[type].push_back(unit);

	if (type->get_unit_class() != nullptr) {
		this->units_by_class[type->get_unit_class()].push_back(unit);
	}
	
	if (unit->Active) {
		this->ChangeUnitTypeAiActiveCount(type, 1);
		this->AiActiveUnitsByType[type].push_back(unit);
	}

	if (type->BoolFlag[TOWNHALL_INDEX].value) {
		this->NumTownHalls++;
	}
	
	for (int i = 0; i < MaxCosts; ++i) {
		this->ResourceDemand[i] += type->Stats[this->Index].ResourceDemand[i];
	}
	
	if (this->AiEnabled && type->BoolFlag[COWARD_INDEX].value && !type->BoolFlag[HARVESTER_INDEX].value && !type->CanTransport() && type->Spells.size() == 0 && CMap::Map.Info.IsPointOnMap(unit->tilePos, unit->MapLayer) && unit->CanMove() && unit->Active && unit->GroupId != 0 && unit->Variable[SIGHTRANGE_INDEX].Value > 0) { //assign coward, non-worker, non-transporter, non-spellcaster units to be scouts
		this->Ai->Scouts.push_back(unit);
	}
	
	if (!type_change) {
		if (unit->Character != nullptr) {
			this->Heroes.push_back(unit);
		}
	}
}

void CPlayer::DecreaseCountsForUnit(CUnit *unit, bool type_change)
{
	const wyrmgus::unit_type *type = unit->Type;

	this->ChangeUnitTypeCount(type, -1);
	
	wyrmgus::vector::remove(this->units_by_type[type], unit);

	if (this->units_by_type[type].empty()) {
		this->units_by_type.erase(type);
	}

	if (type->get_unit_class() != nullptr) {
		wyrmgus::vector::remove(this->units_by_class[type->get_unit_class()], unit);

		if (this->units_by_class[type->get_unit_class()].empty()) {
			this->units_by_class.erase(type->get_unit_class());
		}
	}
	
	if (unit->Active) {
		this->ChangeUnitTypeAiActiveCount(type, -1);
		
		wyrmgus::vector::remove(this->AiActiveUnitsByType[type], unit);
		
		if (this->AiActiveUnitsByType[type].empty()) {
			this->AiActiveUnitsByType.erase(type);
		}
	}
	
	if (type->BoolFlag[TOWNHALL_INDEX].value) {
		this->NumTownHalls--;
	}
	
	for (int i = 0; i < MaxCosts; ++i) {
		this->ResourceDemand[i] -= type->Stats[this->Index].ResourceDemand[i];
	}
	
	if (this->AiEnabled && this->Ai && std::find(this->Ai->Scouts.begin(), this->Ai->Scouts.end(), unit) != this->Ai->Scouts.end()) {
		this->Ai->Scouts.erase(std::remove(this->Ai->Scouts.begin(), this->Ai->Scouts.end(), unit), this->Ai->Scouts.end());
	}
	
	if (!type_change) {
		if (unit->Character != nullptr) {
			this->Heroes.erase(std::remove(this->Heroes.begin(), this->Heroes.end(), unit), this->Heroes.end());
		}
	}
}

/**
**  Have unit of type.
**
**  @param type    Type of unit.
**
**  @return        How many exists, false otherwise.
*/
bool CPlayer::has_unit_type(const wyrmgus::unit_type *unit_type) const
{
	return this->GetUnitTypeCount(unit_type) > 0;
}

int CPlayer::get_population() const
{
	int people_count = 0;

	for (const auto &kv_pair : this->UnitTypesCount) {
		const wyrmgus::unit_type *unit_type = kv_pair.first;
		if (!unit_type->BoolFlag[ORGANIC_INDEX].value || unit_type->BoolFlag[FAUNA_INDEX].value) {
			continue;
		}

		people_count += kv_pair.second;
	}

	return static_cast<int>(pow(people_count, 2)) * wyrmgus::base_population_per_unit;
}

/**
**  Initialize the Ai for all players.
*/
void PlayersInitAi()
{
	for (int player = 0; player < NumPlayers; ++player) {
		if (CPlayer::Players[player]->AiEnabled) {
			AiInit(*CPlayer::Players[player]);
		}
	}
}

/**
**  Handle AI of all players each game cycle.
*/
void PlayersEachCycle()
{
	for (int player = 0; player < NumPlayers; ++player) {
		CPlayer *p = CPlayer::Players[player];
		
		if (p->LostTownHallTimer && !p->is_revealed() && p->LostTownHallTimer < ((int) GameCycle) && CPlayer::GetThisPlayer()->HasContactWith(*p)) {
			p->set_revealed(true);
			for (int j = 0; j < NumPlayers; ++j) {
				if (player != j && CPlayer::Players[j]->Type != PlayerNobody) {
					CPlayer::Players[j]->Notify(_("%s's units have been revealed!"), p->Name.c_str());
				} else {
					CPlayer::Players[j]->Notify("%s", _("Your units have been revealed!"));
				}
			}
		}
		
		
		for (size_t i = 0; i < p->Modifiers.size(); ++i) { //if already has the modifier, make it have the greater duration of the new or old one
			if ((unsigned long) p->Modifiers[i].second < GameCycle) {
				p->RemoveModifier(p->Modifiers[i].first); //only remove one modifier per cycle, to prevent too many upgrade changes from happening at the same cycle (for performance reasons)
				break;
			}
		}
		
		if (p->HeroCooldownTimer) {
			p->HeroCooldownTimer--;
		}

		if (p->AiEnabled) {
			AiEachCycle(*p);
		}
	}
}

/**
**  Handle AI of a player each second.
**
**  @param playerIdx  the player to update AI
*/
void PlayersEachSecond(int playerIdx)
{
	CPlayer *player = CPlayer::Players[playerIdx];

	if ((GameCycle / CYCLES_PER_SECOND) % 10 == 0) {
		for (int res = 0; res < MaxCosts; ++res) {
			player->Revenue[res] = player->Resources[res] + player->StoredResources[res] - player->LastResources[res];
			player->Revenue[res] *= 6;  // estimate per minute
			player->LastResources[res] = player->Resources[res] + player->StoredResources[res];
		}
	}
	if (player->AiEnabled) {
		AiEachSecond(*player);
	}

	player->UpdateFreeWorkers();
	//Wyrmgus start
	player->PerformResourceTrade();
	player->update_current_quests();
	//Wyrmgus end
}

/**
**  Handle AI of a player each half minute.
**
**  @param playerIdx  the player to update AI
*/
void PlayersEachHalfMinute(int playerIdx)
{
	CPlayer *player = CPlayer::Players[playerIdx];

	if (player->AiEnabled) {
		AiEachHalfMinute(*player);
	}

	player->update_quest_pool(); // every half minute, update the quest pool
}

/**
**  Handle AI of a player each minute.
**
**  @param playerIdx  the player to update AI
*/
void PlayersEachMinute(int playerIdx)
{
	CPlayer *player = CPlayer::Players[playerIdx];

	if (player->AiEnabled) {
		AiEachMinute(*player);
	}
}

/**
**  Setup the player colors for the current palette.
**
**  @todo  FIXME: could be called before PixelsXX is setup.
*/
void SetPlayersPalette()
{
	for (int i = 0; i < PlayerMax - 1; ++i) {
		if (CPlayer::Players[i]->Faction == -1) {
			CPlayer::Players[i]->player_color = wyrmgus::player_color::get_all()[SyncRand(wyrmgus::player_color::get_all().size())];
		}
	}

	CPlayer::Players[PlayerNumNeutral]->player_color = wyrmgus::defines::get()->get_neutral_player_color();
}

/**
**  Notify player about a problem.
**
**  @param type    Problem type
**  @param pos     Map tile position
**  @param fmt     Message format
**  @param ...     Message varargs
**
**  @todo FIXME: We must also notfiy allied players.
*/
void CPlayer::Notify(int type, const Vec2i &pos, int z, const char *fmt, ...) const
{
	Assert(CMap::Map.Info.IsPointOnMap(pos, z));
	char temp[128];
	Uint32 color;
	va_list va;

	// Notify me, and my TEAM members
	if (this != CPlayer::GetThisPlayer() && !IsTeamed(*CPlayer::GetThisPlayer())) {
		return;
	}

	va_start(va, fmt);
	temp[sizeof(temp) - 1] = '\0';
	vsnprintf(temp, sizeof(temp) - 1, fmt, va);
	va_end(va);
	switch (type) {
		case NotifyRed:
			color = ColorRed;
			break;
		case NotifyYellow:
			color = ColorYellow;
			break;
		case NotifyGreen:
			color = ColorGreen;
			break;
		default: color = ColorWhite;
	}

	UI.get_minimap()->AddEvent(pos, z, color);

	if (this == CPlayer::GetThisPlayer()) {
		//Wyrmgus start
//		SetMessageEvent(pos, "%s", temp);
		SetMessageEvent(pos, z, "%s", temp);
		//Wyrmgus end
	} else {
		//Wyrmgus start
//		SetMessageEvent(pos, "(%s): %s", Name.c_str(), temp);
		SetMessageEvent(pos, z, "(%s): %s", Name.c_str(), temp);
		//Wyrmgus end
	}
}

/**
**  Notify player about a problem.
**
**  @param type    Problem type
**  @param pos     Map tile position
**  @param fmt     Message format
**  @param ...     Message varargs
**
**  @todo FIXME: We must also notfiy allied players.
*/
void CPlayer::Notify(const char *fmt, ...) const
{
	// Notify me, and my TEAM members
	if (this != CPlayer::GetThisPlayer() && !IsTeamed(*CPlayer::GetThisPlayer())) {
		return;
	}
	char temp[128];
	va_list va;

	va_start(va, fmt);
	temp[sizeof(temp) - 1] = '\0';
	vsnprintf(temp, sizeof(temp) - 1, fmt, va);
	va_end(va);
	if (this == CPlayer::GetThisPlayer()) {
		SetMessage("%s", temp);
	} else {
		SetMessage("(%s): %s", Name.c_str(), temp);
	}
}

void CPlayer::SetDiplomacyNeutralWith(const CPlayer &player)
{
	this->enemies.erase(player.Index);
	this->allies.erase(player.Index);

	//Wyrmgus start
	if (GameCycle > 0 && player.Index == CPlayer::GetThisPlayer()->Index) {
		CPlayer::GetThisPlayer()->Notify(_("%s changed their diplomatic stance with us to Neutral"), _(this->Name.c_str()));
	}
	//Wyrmgus end
}

void CPlayer::SetDiplomacyAlliedWith(const CPlayer &player)
{
	this->enemies.erase(player.Index);
	this->allies.insert(player.Index);
	
	if (GameCycle > 0 && player.Index == CPlayer::GetThisPlayer()->Index) {
		CPlayer::GetThisPlayer()->Notify(_("%s changed their diplomatic stance with us to Ally"), _(this->Name.c_str()));
	}
}

void CPlayer::SetDiplomacyEnemyWith(CPlayer &player)
{
	this->enemies.insert(player.Index);
	this->allies.erase(player.Index);
	
	if (GameCycle > 0) {
		if (player.Index == CPlayer::GetThisPlayer()->Index) {
			CPlayer::GetThisPlayer()->Notify(_("%s changed their diplomatic stance with us to Enemy"), _(this->Name.c_str()));
		} else if (this->Index == CPlayer::GetThisPlayer()->Index) {
			CPlayer::GetThisPlayer()->Notify(_("We have changed our diplomatic stance with %s to Enemy"), _(player.Name.c_str()));
		}
	}

	if (this->has_shared_vision_with(player)) {
		CommandSharedVision(this->Index, false, player.Index);
	}

	// if either player is the overlord of another (indirect or otherwise), break the vassalage bond after the declaration of war
	if (this->is_overlord_of(&player)) {
		player.set_overlord(nullptr, wyrmgus::vassalage_type::none);
	} else if (player.is_overlord_of(this)) {
		this->set_overlord(nullptr, wyrmgus::vassalage_type::none);
	}

	//if the other player has an overlord, then we must also go to war with them
	if (player.get_overlord() != nullptr) {
		this->SetDiplomacyEnemyWith(*player.get_overlord());
	}
}

void CPlayer::SetDiplomacyCrazyWith(const CPlayer &player)
{
	this->enemies.insert(player.Index);
	this->allies.insert(player.Index);
	
	if (GameCycle > 0 && player.Index == CPlayer::GetThisPlayer()->Index) {
		CPlayer::GetThisPlayer()->Notify(_("%s changed their diplomatic stance with us to Crazy"), _(this->Name.c_str()));
	}
}

void CPlayer::ShareVisionWith(const CPlayer &player)
{
	this->shared_vision.insert(player.Index);
	
	if (GameCycle > 0 && player.Index == CPlayer::GetThisPlayer()->Index) {
		CPlayer::GetThisPlayer()->Notify(_("%s is now sharing vision with us"), _(this->Name.c_str()));
	}
}

void CPlayer::UnshareVisionWith(const CPlayer &player)
{
	this->shared_vision.erase(player.Index);
	
	if (GameCycle > 0 && player.Index == CPlayer::GetThisPlayer()->Index) {
		CPlayer::GetThisPlayer()->Notify(_("%s is no longer sharing vision with us"), _(this->Name.c_str()));
	}
}

void CPlayer::set_overlord(CPlayer *overlord, const wyrmgus::vassalage_type)
{
	if (overlord == this->get_overlord()) {
		return;
	}

	if (overlord != nullptr && overlord->get_overlord() == this) {
		throw std::runtime_error("Cannot set player \"" + overlord->Name + "\" as the overlord of \"" + this->Name + "\", as the former is a vassal of the latter, and a vassal can't be the overlord of its own overlord.");
	}

	CPlayer *old_overlord = this->get_overlord();
	if (old_overlord != nullptr) {
		wyrmgus::vector::remove(old_overlord->vassals, this);

		//remove alliance and shared vision with the old overlord, and any upper overlords
		if (!SaveGameLoading) {
			while (old_overlord != nullptr) {
				this->break_overlordship_alliance(old_overlord);
				old_overlord = old_overlord->get_overlord();
			}
		}
	}

	this->overlord = overlord;
	this->vassalage_type = vassalage_type;

	if (overlord != nullptr) {
		overlord->vassals.push_back(this);

		//establish alliance and shared vision with the new overlord, and any upper overlords
		if (!SaveGameLoading) {
			while (overlord != nullptr) {
				this->establish_overlordship_alliance(overlord);
				overlord = overlord->get_overlord();
			}
		}
	}

	this->update_minimap_territory();
}

void CPlayer::establish_overlordship_alliance(CPlayer *overlord)
{
	this->SetDiplomacyAlliedWith(*overlord);
	overlord->SetDiplomacyAlliedWith(*this);
	CommandSharedVision(this->Index, true, overlord->Index);
	CommandSharedVision(overlord->Index, true, this->Index);

	//vassals should also be allied with overlords higher up in the hierarchy
	for (CPlayer *vassal : this->get_vassals()) {
		vassal->establish_overlordship_alliance(overlord);
	}
}

void CPlayer::break_overlordship_alliance(CPlayer *overlord)
{
	if (this->IsAllied(*overlord)) {
		this->SetDiplomacyNeutralWith(*overlord);
		overlord->SetDiplomacyNeutralWith(*this);
	}
	CommandSharedVision(this->Index, false, overlord->Index);
	CommandSharedVision(overlord->Index, false, this->Index);

	//vassals should also have their alliances with overlords higher up in the hierarchy broken
	for (CPlayer *vassal : this->get_vassals()) {
		vassal->break_overlordship_alliance(overlord);
	}
}

/**
**  Check if the player is an enemy
*/
bool CPlayer::IsEnemy(const CPlayer &player) const
{
	if (this->get_overlord() != nullptr && this->get_overlord()->IsEnemy(player)) {
		return true;
	}

	//be hostile to the other player if they are hostile, even if the diplomatic stance hasn't been changed
	return this->IsEnemy(player.Index) || player.IsEnemy(this->Index);
}

/**
**  Check if the unit is an enemy
*/
bool CPlayer::IsEnemy(const CUnit &unit) const
{
	if (
		unit.Player->Type == PlayerNeutral
		&& (unit.Type->BoolFlag[NEUTRAL_HOSTILE_INDEX].value || unit.Type->BoolFlag[PREDATOR_INDEX].value)
		&& this->Type != PlayerNeutral
	) {
		return true;
	}
	
	if (
		this != unit.Player
		&& this->Type != PlayerNeutral
		&& unit.CurrentAction() == UnitAction::Attack
		&& unit.CurrentOrder()->HasGoal()
		&& unit.CurrentOrder()->GetGoal()->Player == this
		&& !unit.CurrentOrder()->GetGoal()->Type->BoolFlag[HIDDENOWNERSHIP_INDEX].value
	) {
		return true;
	}
	
	if (unit.Player->Index != this->Index && this->Type != PlayerNeutral && unit.Type->BoolFlag[HIDDENOWNERSHIP_INDEX].value && unit.IsAgressive() && !this->HasNeutralFactionType()) {
		return true;
	}
	
	return IsEnemy(*unit.Player);
}

/**
**  Check if the player is an ally
*/
bool CPlayer::IsAllied(const CPlayer &player) const
{
	//only consider yourself to be the ally of another player if they have the allied stance with you as well
	return this->IsAllied(player.Index) && player.IsAllied(this->Index);
}

/**
**  Check if the unit is an ally
*/
bool CPlayer::IsAllied(const CUnit &unit) const
{
	return IsAllied(*unit.Player);
}

bool CPlayer::IsVisionSharing() const
{
	return !this->shared_vision.empty();
}

bool CPlayer::has_shared_vision_with(const CPlayer &player) const
{
	return this->has_shared_vision_with(player.Index);
}

bool CPlayer::has_shared_vision_with(const CUnit &unit) const
{
	return this->has_shared_vision_with(*unit.Player);
}

bool CPlayer::has_mutual_shared_vision_with(const CPlayer &player) const
{
	return this->shared_vision.contains(player.Index) && player.shared_vision.contains(this->Index);
}

bool CPlayer::has_mutual_shared_vision_with(const CUnit &unit) const
{
	return this->has_mutual_shared_vision_with(*unit.Player);
}

/**
**  Check if the player is teamed
*/
bool CPlayer::IsTeamed(const CPlayer &player) const
{
	return Team == player.Team;
}

/**
**  Check if the unit is teamed
*/
bool CPlayer::IsTeamed(const CUnit &unit) const
{
	return IsTeamed(*unit.Player);
}

//Wyrmgus start
/**
**  Check if the player has contact with another (used for determining which players show up in the player list and etc.)
*/
bool CPlayer::HasContactWith(const CPlayer &player) const
{
	return player.StartMapLayer == this->StartMapLayer || (player.StartMapLayer < (int) CMap::Map.MapLayers.size() && this->StartMapLayer < (int) CMap::Map.MapLayers.size() && CMap::Map.MapLayers[player.StartMapLayer]->world == CMap::Map.MapLayers[this->StartMapLayer]->world && CMap::Map.MapLayers[player.StartMapLayer]->plane == CMap::Map.MapLayers[this->StartMapLayer]->plane);
}

/**
**  Check if the player's faction type is a neutral one
*/
bool CPlayer::HasNeutralFactionType() const
{
	if (
		this->Race != -1
		&& this->Faction != -1
		&& (wyrmgus::faction::get_all()[this->Faction]->Type == FactionTypeMercenaryCompany || wyrmgus::faction::get_all()[this->Faction]->Type == FactionTypeHolyOrder || wyrmgus::faction::get_all()[this->Faction]->Type == FactionTypeTradingCompany)
	) {
		return true;
	}

	return false;
}

/**
**  Check if the player can use the buildings of another, for neutral building functions (i.e. unit training)
*/
bool CPlayer::HasBuildingAccess(const CPlayer &player, const ButtonCmd button_action) const
{
	if (player.IsEnemy(*this)) {
		return false;
	}

	if (player.get_faction() == nullptr) {
		return false;
	}

	if (player.Type == PlayerNeutral) {
		return true;
	}
	
	if (
		player.HasNeutralFactionType()
		&& (player.get_overlord() == nullptr || this->is_any_overlord_of(&player) || player.get_overlord()->IsAllied(*this))
	) {
		if (player.get_faction()->Type != FactionTypeHolyOrder || (button_action != ButtonCmd::Train && button_action != ButtonCmd::TrainClass && button_action != ButtonCmd::Buy) || wyrmgus::vector::contains(this->Deities, player.get_faction()->get_holy_order_deity())) { //if the faction is a holy order, the player must have chosen its respective deity
			return true;
		}
	}

	return false;
}

bool CPlayer::HasHero(const wyrmgus::character *hero) const
{
	if (!hero) {
		return false;
	}
	
	for (const CUnit *hero_unit : this->Heroes) {
		if (hero_unit->Character == hero) {
			return true;
		}
	}
	
	return false;
}

void NetworkSetFaction(int player, const std::string &faction_name)
{
	const wyrmgus::faction *faction = wyrmgus::faction::try_get(faction_name);
	int faction_id = faction ? faction->ID : -1;
	SendCommandSetFaction(player, faction_id);
}

std::string GetFactionTypeNameById(int faction_type)
{
	if (faction_type == FactionTypeNoFactionType) {
		return "no-faction-type";
	} else if (faction_type == FactionTypeTribe) {
		return "tribe";
	} else if (faction_type == FactionTypePolity) {
		return "polity";
	} else if (faction_type == FactionTypeMercenaryCompany) {
		return "mercenary-company";
	} else if (faction_type == FactionTypeHolyOrder) {
		return "holy_order";
	} else if (faction_type == FactionTypeTradingCompany) {
		return "trading-company";
	}

	return "";
}

int GetFactionTypeIdByName(const std::string &faction_type)
{
	if (faction_type == "no-faction-type") {
		return FactionTypeNoFactionType;
	} else if (faction_type == "tribe") {
		return FactionTypeTribe;
	} else if (faction_type == "polity") {
		return FactionTypePolity;
	} else if (faction_type == "mercenary-company") {
		return FactionTypeMercenaryCompany;
	} else if (faction_type == "holy_order") {
		return FactionTypeHolyOrder;
	} else if (faction_type == "trading-company") {
		return FactionTypeTradingCompany;
	}

	return -1;
}

std::string GetForceTypeNameById(const ForceType force_type)
{
	if (force_type == ForceType::Land) {
		return "land-force";
	} else if (force_type == ForceType::Naval) {
		return "naval-force";
	} else if (force_type == ForceType::Air) {
		return "air-force";
	} else if (force_type == ForceType::Space) {
		return "space-force";
	}

	return "";
}

ForceType GetForceTypeIdByName(const std::string &force_type)
{
	if (force_type == "land-force" || force_type == "land_force") {
		return ForceType::Land;
	} else if (force_type == "naval-force" || force_type == "naval_force") {
		return ForceType::Naval;
	} else if (force_type == "air-force" || force_type == "air_force") {
		return ForceType::Air;
	} else if (force_type == "space-force" || force_type == "space_force") {
		return ForceType::Space;
	}

	throw std::runtime_error("Invalid force type: " + force_type + ".");
}

bool IsNameValidForWord(const std::string &word_name)
{
	if (word_name.empty()) {
		return false;
	}
	
	if (
		word_name.find('\n') != -1
		|| word_name.find('\\') != -1
		|| word_name.find('/') != -1
		|| word_name.find('.') != -1
		|| word_name.find('*') != -1
		|| word_name.find('[') != -1
		|| word_name.find(']') != -1
		|| word_name.find(':') != -1
		|| word_name.find(';') != -1
		|| word_name.find('=') != -1
		|| word_name.find(',') != -1
		|| word_name.find('<') != -1
		|| word_name.find('>') != -1
		|| word_name.find('?') != -1
		|| word_name.find('|') != -1
	) {
		return false;
	}
	
	if (word_name.find_first_not_of(' ') == std::string::npos) {
		return false; //name contains only spaces
	}
	
	return true;
}
//Wyrmgus end
