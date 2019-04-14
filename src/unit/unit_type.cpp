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
/**@name unit_type.cpp - The unit type source file. */
//
//      (c) Copyright 1998-2019 by Lutz Sammer, Jimmy Salmon and Andrettin
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

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "unit/unit_type.h"

//Wyrmgus start
#include "ai/ai_local.h" //for using AiHelpers
//Wyrmgus end
#include "animation/animation.h"
#include "animation/animation_exactframe.h"
#include "animation/animation_frame.h"
#include "civilization.h"
#include "config.h"
#include "config_operator.h"
#include "construct.h"
//Wyrmgus start
#include "editor/editor.h" //for personal name generation
//Wyrmgus end
#include "faction.h"
#include "iolib.h"
#include "item/item_class.h"
#include "item/item_slot.h"
#include "luacallback.h"
#include "map/map.h"
#include "map/terrain_type.h"
#include "map/tileset.h"
#include "missile/missile_type.h"
#include "mod.h"
#include "player.h"
#include "script.h"
#include "sound/sound.h"
#include "sound/unit_sound.h"
#include "species/species.h"
#include "spell/spells.h"
#include "translate.h"
#include "ui/button_action.h"
#include "ui/button_level.h"
#include "ui/icon.h"
#include "ui/ui.h"
#include "unit/unit_class.h"
#include "unit/unit_type_variation.h"
#include "upgrade/dependency.h"
//Wyrmgus start
#include "upgrade/upgrade.h"
//Wyrmgus end
#include "util.h"
#include "video/video.h"

#include <cstring>
#include <ctype.h>

/*----------------------------------------------------------------------------
-- Documentation
----------------------------------------------------------------------------*/

/**
**  @class CUnitType unit_type.h
**
**  \#include "unit/unit_type.h"
**
**  This class contains the information that is shared between all
**  units of the same type and determins if a unit is a building,
**  a person, ...
**
**  The unit-type class members:
**
**  CUnitType::Ident
**
**    Unique identifier of the unit-type, used to reference it in
**    config files and during startup. As convention they start with
**    "unit-" fe. "unit-farm".
**  @note Don't use this member in game, use instead the pointer
**  to this structure. See UnitTypeByIdent().
**
**  CUnitType::Name
**
**    Pretty name shown by the engine. The name should be shorter
**    than 17 characters and no word can be longer than 8 characters.
**
**  CUnitType::File
**
**    Path file name of the sprite file.
**
**  CUnitType::ShadowFile
**
**    Path file name of shadow sprite file.
**
**  CUnitType::DrawLevel
**
**    The Level/Order to draw this type of unit in. 0-255 usually.
**
**  CUnitType::Width CUnitType::Height
**
**    Size of a sprite frame in pixels. All frames of a sprite have
**    the same size. Also all sprites (tilesets) must have the same
**    size.
**
**  CUnitType::ShadowWidth CUnitType::ShadowHeight
**
**    Size of a shadow sprite frame in pixels. All frames of a sprite
**    have the same size. Also all sprites (tilesets) must have the
**    same size.
**
**  CUnitType::ShadowOffsetX CUnitType::ShadowOffsetY
**
**    Vertical offset to draw the shadow in pixels.
**
**  CUnitType::Animations
**
**    Animation scripts for the different actions. Currently the
**    animations still, move, attack and die are supported.
**  @see CAnimations
**  @see CAnimation
**
**  CUnitType::Icon
**
**    Icon to display for this unit-type. Contains configuration and
**    run time variable.
**  @note This icon can be used for training, but isn't used.
**
**  CUnitType::Missile
**
**    Configuration and run time variable of the missile weapon.
**  @note It is planned to support more than one weapons.
**  And the sound of the missile should be used as fire sound.
**
**  CUnitType::Explosion
**
**    Configuration and run time variable of the missile explosion.
**    This is the explosion that happens if unit is set to
**    ExplodeWhenKilled
**
**  CUnitType::CorpseName
**
**    Corpse unit-type name, should only be used during setup.
**
**  CUnitType::CorpseType
**
**    Corpse unit-type pointer, only this should be used during run
**    time. Many unit-types can share the same corpse.
**
**
**  @todo continue this documentation
**
**  CUnitType::Construction
**
**    What is shown in construction phase.
**
**  CUnitType::SightRange
**
**    Sight range
**
**  CUnitType::_HitPoints
**
**    Maximum hit points
**
**
**  CUnitType::_Costs[::MaxCosts]
**
**    How many resources needed
**
**  CUnitType::RepairHP
**
**    The HP given to a unit each cycle it's repaired.
**    If zero, unit cannot be repaired
**
**    CUnitType::RepairCosts[::MaxCosts]
**
**    Costs per repair cycle to fix a unit.
**
**  CUnitType::TileSize
**
**    Tile size on map
**
**  CUnitType::BoxWidth
**
**    Selected box size width
**
**  CUnitType::BoxHeight
**
**    Selected box size height
**
**  CUnitType::NumDirections
**
**    Number of directions the unit can face
**
**  CUnitType::MinAttackRange
**
**    Minimal attack range
**
**  CUnitType::ReactRangeComputer
**
**    Reacts on enemy for computer
**
**  CUnitType::ReactRangePerson
**
**    Reacts on enemy for person player
**
**  CUnitType::Priority
**
**    Priority value / AI Treatment
**
**  CUnitType::BurnPercent
**
**    The burning limit in percents. If the unit has lees than
**    this it will start to burn.
**
**  CUnitType::BurnDamageRate
**
**    Burn rate in HP per second
**
**  CUnitType::UnitType
**
**    Land / fly / naval
**
**  @note original only visual effect, we do more with this!
**
**  CUnitType::DecayRate
**
**    Decay rate in 1/6 seconds
**
**  CUnitType::AnnoyComputerFactor
**
**    How much this annoys the computer
**
**  @todo not used
**
**  CUnitType::MouseAction
**
**    Right click action
**
**  CUnitType::Points
**
**    How many points you get for unit. Used in the final score table.
**
**  CUnitType::CanTarget
**
**    Which units can it attack
**
**  CUnitType::LandUnit
**
**    Land animated
**
**  CUnitType::AirUnit
**
**    Air animated
**
**  CUnitType::SeaUnit
**
**    Sea animated
**
**  CUnitType::ExplodeWhenKilled
**
**    Death explosion animated
**
**  CUnitType::RandomMovementProbability
**
**    When the unit is idle this is the probability that it will
**    take a step in a random direction, in percents.
**
**  CUnitType::ClicksToExplode
**
**    If this is non-zero, then after that many clicks the unit will
**    commit suicide. Doesn't work with resource workers/resources.
**
**  CUnitType::Building
**
**    Unit is a Building
**
**  CUnitType::Transporter
**
**    Can transport units
**
**  CUnitType::MaxOnBoard
**
**    Maximum units on board (for transporters), and resources
**
**  CUnitType::StartingResources
**    Amount of Resources a unit has when It's Built
**
**  CUnitType::DamageType
**    Unit's missile damage type (used for extra death animations)
**
**  CUnitType::GivesResource
**
**    This equals to the resource Id of the resource given
**    or 0 (TimeCost) for other buildings.
**
**  CUnitType::ResInfo[::MaxCosts]
**
**    Information about resource harvesting. If null, it can't
**    harvest it.
**
**  CUnitType::NeutralMinimapColorRGB
**
**    Says what color a unit will have when it's neutral and
**    is displayed on the minimap.
**
**  CUnitType::CanStore[::MaxCosts]
**
**    What resource types we can store here.
**
**  CUnitType::Spells
**
**    Spells the unit is able to use
**
**  CUnitType::CanAttack
**
**    Unit is able to attack.
**
**  CUnitType::RepairRange
**
**    Unit can repair buildings. It will use the actack animation.
**    It will heal 4 points for every repair cycle, and cost 1 of
**    each resource, alternatively(1 cycle wood, 1 cycle gold)
**  @todo The above should be more configurable.
**    If units have a repair range, they can repair, and this is the
**    distance.
**
**    CUnitType::ShieldPiercing
**
**    Can directly damage shield-protected units, without shield damaging.
**
**  CUnitType::Sound
**
**    Sounds for events
**
**  CUnitType::Weapon
**
**    Current sound for weapon
**
**  @todo temporary solution
**
**  CUnitType::FieldFlags
**
**    Flags that are set, if a unit enters a map field or cleared, if
**    a unit leaves a map field.
**
**  CUnitType::MovementMask
**
**    Movement mask, this value is and'ed to the map field flags, to
**    see if a unit can enter or placed on the map field.
**
**  CUnitType::Stats[::PlayerMax]
**
**    Unit status for each player
**  @todo This stats should? be moved into the player struct
**
**  CUnitType::Type
**
**    Type as number
**  @todo Should us a general name f.e. Slot here?
**
**  CUnitType::Sprite
**
**    Sprite images
**
**  CUnitType::ShadowSprite
**
**    Shadow sprite images
**
**  CUnitType::PlayerColorSprite
**
**    Sprite images of the player colors.  This image is drawn
**    over CUnitType::Sprite.  Used with OpenGL only.
**
**
*/
/**
**
**  @class ResourceInfo unit_type.h
**
** \#include "unit/unit_type.h"
**
**    This class contains information about how a unit will harvest a resource.
**
**  ResourceInfo::FileWhenLoaded
**
**    The harvester's animation file will change when it's loaded.
**
**  ResourceInfo::FileWhenEmpty;
**
**    The harvester's animation file will change when it's empty.
**    The standard animation is used only when building/repairing.
**
**
**  ResourceInfo::HarvestFromOutside
**
**    Unit will harvest from the outside. The unit will use it's
**    Attack animation (seems it turned into a generic Action anim.)
**
**  ResourceInfo::ResourceId
**
**    The resource this is for. Mostly redundant.
**
**  ResourceInfo::FinalResource
**
**    The resource is converted to this at the depot. Useful for
**    a fisherman who harvests fish, but it all turns to food at the
**    depot.
**
**  ResourceInfo::FinalResourceConversionRate
**
**    The rate at which the resource is converted to the final resource at the depot. Useful for
**    silver mines returning a lower amount of gold.
**
**  ResourceInfo::WaitAtResource
**
**    Cycles the unit waits while inside a resource.
**
**  ResourceInfo::ResourceStep
**
**    The unit makes so-caled mining cycles. Each mining cycle
**    it does some sort of animation and gains ResourceStep
**    resources. You can stop after any number of steps.
**    when the quantity in the harvester reaches the maximum
**    (ResourceCapacity) it will return home. I this is 0 then
**    it's considered infinity, and ResourceCapacity will now
**    be the limit.
**
**  ResourceInfo::ResourceCapacity
**
**    Maximum amount of resources a harvester can carry. The
**    actual amount can be modified while unloading.
**
**  ResourceInfo::LoseResources
**
**    Special lossy behaviour for loaded harvesters. Harvesters
**    with loads other than 0 and ResourceCapacity will lose their
**    cargo on any new order.
**
**  ResourceInfo::WaitAtDepot
**
**    Cycles the unit waits while inside the depot to unload.
**
**  ResourceInfo::TerrainHarvester
**
**    The unit will harvest terrain. For now this only works
**    for wood. maybe it could be made to work for rocks, but
**    more than that requires a tileset rewrite.
**  @todo more configurable.
**
*/

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

std::vector<CUnitType *> CUnitType::UnitTypes;	/// unit-types definition
std::map<std::string, CUnitType *> UnitTypeMap;

/**
**  Next unit type are used hardcoded in the source.
**
**  @todo find a way to make it configurable!
*/
//Wyrmgus start
//CUnitType *UnitTypeHumanWall;       /// Human wall
//CUnitType *UnitTypeOrcWall;         /// Orc wall
//Wyrmgus end

/**
**  Default names for the resources.
*/
std::string DefaultResourceNames[MaxCosts];

std::vector<int> LuxuryResources;

/**
**  Default names for the resources.
*/
std::string ExtraDeathTypes[ANIMATIONS_DEATHTYPES];

//Wyrmgus start
std::vector<std::string> UpgradeClasses;
std::map<std::string, int> UpgradeClassStringToIndex;
CUnitType *SettlementSiteUnitType;
//Wyrmgus end

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

int GetResourceIdByName(const char *resourceName)
{
	for (unsigned int res = 0; res < MaxCosts; ++res) {
		if (!strcmp(resourceName, DefaultResourceNames[res].c_str())) {
			return res;
		}
	}
	return -1;
}

int GetResourceIdByName(lua_State *l, const char *resourceName)
{
	const int res = GetResourceIdByName(resourceName);
	if (res == -1) {
		LuaError(l, "Resource not found: %s" _C_ resourceName);
	}
	return res;
}

//Wyrmgus start
std::string GetResourceNameById(int resource_id)
{
	if (resource_id > 0 && resource_id < MaxCosts) {
		return DefaultResourceNames[resource_id];
	} else {
		return "";
	}
}
//Wyrmgus end

CUnitType::CUnitType()
{
	memset(RepairCosts, 0, sizeof(RepairCosts));
	memset(CanStore, 0, sizeof(CanStore));
	//Wyrmgus start
	memset(GrandStrategyProductionEfficiencyModifier, 0, sizeof(GrandStrategyProductionEfficiencyModifier));
	//Wyrmgus end
	memset(ResInfo, 0, sizeof(ResInfo));
	memset(MissileOffsets, 0, sizeof(MissileOffsets));
	//Wyrmgus start
	memset(LayerSprites, 0, sizeof(LayerSprites));
	//Wyrmgus end
}

CUnitType::~CUnitType()
{
	if (this->DeathExplosion) {
		delete this->DeathExplosion;
	}
	if (this->OnEachSecond) {
		delete this->OnEachSecond;
	}
	if (this->OnInit) {
		delete this->OnInit;
	}

	//Wyrmgus start
	this->SoldUnits.clear();
	this->SpawnUnits.clear();
	this->Drops.clear();
	this->AiDrops.clear();
	this->DropSpells.clear();
	this->Affixes.clear();
	this->Traits.clear();
	this->StartingAbilities.clear();
	//Wyrmgus end

	this->BoolFlag.clear();

	// Free Building Restrictions if there are any
	for (std::vector<CBuildRestriction *>::iterator b = this->BuildingRules.begin();
		 b != this->BuildingRules.end(); ++b) {
		delete *b;
	}
	this->BuildingRules.clear();
	for (std::vector<CBuildRestriction *>::iterator b = this->AiBuildingRules.begin();
		 b != this->AiBuildingRules.end(); ++b) {
		delete *b;
	}
	this->AiBuildingRules.clear();

	delete[] this->AutoCastActive;

	for (int res = 0; res < MaxCosts; ++res) {
		if (this->ResInfo[res]) {
			if (this->ResInfo[res]->SpriteWhenLoaded) {
				CGraphic::Free(this->ResInfo[res]->SpriteWhenLoaded);
			}
			if (this->ResInfo[res]->SpriteWhenEmpty) {
				CGraphic::Free(this->ResInfo[res]->SpriteWhenEmpty);
			}
			delete this->ResInfo[res];
		}
	}

	for (CUnitTypeVariation *variation : this->Variations) {
		delete variation;
	}
	
	for (int i = 0; i < MaxImageLayers; ++i) {
		for (CUnitTypeVariation *layer_variation : this->LayerVariations[i]) {
			delete layer_variation;
		}
	}

	CGraphic::Free(this->Sprite);
	CGraphic::Free(this->ShadowSprite);
	//Wyrmgus start
	CGraphic::Free(this->LightSprite);
	for (int i = 0; i < MaxImageLayers; ++i) {
		CGraphic::Free(this->LayerSprites[i]);
	}
	//Wyrmgus end
}

/**
**	@brief	Gets all unit types that represent units in a stricter sense, that is, excluding buildings, items, decorations and etc.
**
**	@return	A vector with the unit types
*/
std::vector<CUnitType *> CUnitType::GetUnitUnitTypes()
{
	std::vector<CUnitType *> unit_types;
	
	for (CUnitType *unit_type : CUnitType::UnitTypes) {
		if (!unit_type->IsUnitUnitType()) {
			continue;
		}
		
		unit_types.push_back(unit_type);
	}
	
	return unit_types;
}

/**
**	@brief	Gets all building unit types
**
**	@return	A vector with the unit types
*/
std::vector<CUnitType *> CUnitType::GetBuildingUnitTypes()
{
	std::vector<CUnitType *> unit_types;
	
	for (CUnitType *unit_type : CUnitType::UnitTypes) {
		if (!unit_type->BoolFlag[BUILDING_INDEX].value) {
			continue;
		}
		
		unit_types.push_back(unit_type);
	}
	
	return unit_types;
}

/**
**	@brief	Gets all building unit types
**
**	@return	A vector with the unit types
*/
std::vector<CUnitType *> CUnitType::GetItemUnitTypes()
{
	std::vector<CUnitType *> unit_types;
	
	for (CUnitType *unit_type : CUnitType::UnitTypes) {
		if (!unit_type->BoolFlag[ITEM_INDEX].value) {
			continue;
		}
		
		unit_types.push_back(unit_type);
	}
	
	return unit_types;
}

/**
**	@brief	Process a property in the data provided by a configuration file
**
**	@param	key		The property's key
**	@param	value	The property's value
**
**	@return	True if the property can be processed, or false otherwise
*/
bool CUnitType::ProcessConfigDataProperty(const std::string &key, std::string value)
{
	if (key == "parent") {
		value = FindAndReplaceString(value, "_", "-");
		CUnitType *parent_type = UnitTypeByIdent(value);
		if (!parent_type) {
			fprintf(stderr, "Unit type \"%s\" does not exist.\n", value.c_str());
		}
		this->SetParent(parent_type);
	} else if (key == "civilization") {
		value = FindAndReplaceString(value, "_", "-");
		CCivilization *civilization = CCivilization::Get(value);
		if (civilization) {
			this->Civilization = civilization;
		}
	} else if (key == "faction") {
		value = FindAndReplaceString(value, "_", "-");
		CFaction *faction = CFaction::Get(value);
		if (faction) {
			this->Faction = faction;
		}
	} else if (key == "animations") {
		value = FindAndReplaceString(value, "_", "-");
		this->Animations = AnimationsByIdent(value);
		if (!this->Animations) {
			fprintf(stderr, "Animations \"%s\" do not exist.\n", value.c_str());
		}
	} else if (key == "icon") {
		value = FindAndReplaceString(value, "_", "-");
		this->Icon.Name = value;
		this->Icon.Icon = nullptr;
		this->Icon.Load();
		this->Icon.Icon->Load();
	} else if (key == "tile_width") {
		this->TileSize.x = std::stoi(value);
	} else if (key == "tile_height") {
		this->TileSize.y = std::stoi(value);
	} else if (key == "box_width") {
		this->BoxWidth = std::stoi(value);
	} else if (key == "box_height") {
		this->BoxHeight = std::stoi(value);
	} else if (key == "draw_level") {
		this->DrawLevel = std::stoi(value);
	} else if (key == "type") {
		if (value == "land") {
			this->UnitType = UnitTypeLand;
			this->LandUnit = true;
		} else if (value == "fly") {
			this->UnitType = UnitTypeFly;
			this->AirUnit = true;
		} else if (value == "fly_low") {
			this->UnitType = UnitTypeFlyLow;
			this->AirUnit = true;
		} else if (value == "naval") {
			this->UnitType = UnitTypeNaval;
			this->SeaUnit = true;
		} else {
			fprintf(stderr, "Invalid unit type type: \"%s\".\n", value.c_str());
		}
	} else if (key == "priority") {
		this->DefaultStat.Variables[PRIORITY_INDEX].Value = std::stoi(value);
		this->DefaultStat.Variables[PRIORITY_INDEX].Max  = std::stoi(value);
	} else if (key == "description") {
		this->Description = value;
	} else if (key == "background") {
		this->Background = value;
	} else if (key == "quote") {
		this->Quote = value;
	} else if (key == "requirements_string") {
		this->RequirementsString = value;
	} else if (key == "experience_requirements_string") {
		this->ExperienceRequirementsString = value;
	} else if (key == "building_rules_string") {
		this->BuildingRulesString = value;
	} else if (key == "max_attack_range") {
		this->DefaultStat.Variables[ATTACKRANGE_INDEX].Value = std::stoi(value);
		this->DefaultStat.Variables[ATTACKRANGE_INDEX].Max = std::stoi(value);
		this->DefaultStat.Variables[ATTACKRANGE_INDEX].Enable = 1;
	} else if (key == "missile") {
		value = FindAndReplaceString(value, "_", "-");
		this->Missile.Name = value;
		this->Missile.Missile = nullptr;
	} else if (key == "fire_missile") {
		value = FindAndReplaceString(value, "_", "-");
		this->FireMissile.Name = value;
		this->FireMissile.Missile = nullptr;
	} else if (key == "corpse") {
		value = FindAndReplaceString(value, "_", "-");
		this->CorpseName = value;
		this->CorpseType = nullptr;
	} else if (key == "weapon_class") {
		value = FindAndReplaceString(value, "_", "-");
		const ::ItemClass *weapon_class = ::ItemClass::Get(value);
		if (weapon_class != nullptr) {
			this->WeaponClasses.push_back(weapon_class);
		}
	} else if (key == "ai_drop") {
		value = FindAndReplaceString(value, "_", "-");
		CUnitType *drop_type = UnitTypeByIdent(value);
		if (drop_type) {
			this->AiDrops.push_back(drop_type);
		} else {
			fprintf(stderr, "Invalid unit type: \"%s\".\n", value.c_str());
		}
	} else if (key == "item_class") {
		value = FindAndReplaceString(value, "_", "-");
		const ::ItemClass *item_class = ::ItemClass::Get(value);
		if (item_class != nullptr) {
			this->ItemClass = item_class;
		}
	} else if (key == "species") {
		value = FindAndReplaceString(value, "_", "-");
		CSpecies *species = CSpecies::Get(value);
		if (species) {
			this->Species = species;
			this->Species->UnitType = this;
		}
	} else if (key == "right_mouse_action") {
		if (value == "none") {
			this->MouseAction = MouseActionNone;
		} else if (value == "attack") {
			this->MouseAction = MouseActionAttack;
		} else if (value == "move") {
			this->MouseAction = MouseActionMove;
		} else if (value == "harvest") {
			this->MouseAction = MouseActionHarvest;
		} else if (value == "spell_cast") {
			this->MouseAction = MouseActionSpellCast;
		} else if (value == "sail") {
			this->MouseAction = MouseActionSail;
		} else if (value == "rally_point") {
			this->MouseAction = MouseActionRallyPoint;
		} else if (value == "trade") {
			this->MouseAction = MouseActionTrade;
		} else {
			fprintf(stderr, "Invalid right mouse action: \"%s\".\n", value.c_str());
		}
	} else if (key == "can_attack") {
		this->CanAttack = StringToBool(value);
	} else if (key == "can_target_land") {
		const bool can_target_land = StringToBool(value);
		if (can_target_land) {
			this->CanTarget |= CanTargetLand;
		} else {
			this->CanTarget &= ~CanTargetLand;
		}
	} else if (key == "can_target_sea") {
		const bool can_target_sea = StringToBool(value);
		if (can_target_sea) {
			this->CanTarget |= CanTargetSea;
		} else {
			this->CanTarget &= ~CanTargetSea;
		}
	} else if (key == "can_target_air") {
		const bool can_target_air = StringToBool(value);
		if (can_target_air) {
			this->CanTarget |= CanTargetAir;
		} else {
			this->CanTarget &= ~CanTargetAir;
		}
	} else if (key == "random_movement_probability") {
		this->RandomMovementProbability = std::stoi(value);
	} else if (key == "random_movement_distance") {
		this->RandomMovementDistance = std::stoi(value);
	} else if (key == "can_cast_spell") {
		value = FindAndReplaceString(value, "_", "-");
		CSpell *spell = CSpell::GetSpell(value);
		if (spell != nullptr) {
			this->Spells.push_back(spell);
		}
	} else if (key == "autocast_active") {
		if (!this->AutoCastActive) {
			this->AutoCastActive = new char[CSpell::Spells.size()];
			memset(this->AutoCastActive, 0, CSpell::Spells.size() * sizeof(char));
		}
		
		if (value == "false") {
			delete[] this->AutoCastActive;
			this->AutoCastActive = nullptr;
		} else {
			value = FindAndReplaceString(value, "_", "-");
			const CSpell *spell = CSpell::GetSpell(value);
			if (spell != nullptr) {
				if (spell->AutoCast) {
					this->AutoCastActive[spell->Slot] = 1;
				} else {
					fprintf(stderr, "AutoCastActive : Define autocast method for \"%s\".\n", value.c_str());
				}
			}
		}
	} else {
		std::string key_pascal_case = SnakeCaseToPascalCase(key);
		
		int index = UnitTypeVar.VariableNameLookup[key_pascal_case.c_str()]; // variable index
		if (index != -1) { // valid index
			if (IsStringNumber(value)) {
				this->DefaultStat.Variables[index].Enable = 1;
				this->DefaultStat.Variables[index].Value = std::stoi(value);
				this->DefaultStat.Variables[index].Max = std::stoi(value);
			} else if (IsStringBool(value)) {
				this->DefaultStat.Variables[index].Enable = StringToBool(value);
			} else { // error
				fprintf(stderr, "Invalid value (\"%s\") for variable \"%s\" when defining unit type \"%s\".\n", value.c_str(), key_pascal_case.c_str(), this->Ident.c_str());
			}
		} else {
			if (this->BoolFlag.size() < UnitTypeVar.GetNumberBoolFlag()) {
				this->BoolFlag.resize(UnitTypeVar.GetNumberBoolFlag());
			}

			index = UnitTypeVar.BoolFlagNameLookup[key_pascal_case.c_str()];
			if (index != -1) {
				if (IsStringNumber(value)) {
					this->BoolFlag[index].value = (std::stoi(value) != 0);
				} else {
					this->BoolFlag[index].value = StringToBool(value);
				}
			} else {
				return false;
			}
		}
	}
	
	return true;
}

/**
**	@brief	Process a section in the data provided by a configuration file
**
**	@param	section		The section
**
**	@return	True if the section can be processed, or false otherwise
*/
bool CUnitType::ProcessConfigDataSection(const CConfigData *section)
{
	if (section->Tag == "costs") {
		for (const CConfigProperty &property : section->Properties) {
			if (property.Operator != CConfigOperator::Assignment) {
				fprintf(stderr, "Wrong operator enumeration index for property \"%s\": %i.\n", property.Key.c_str(), property.Operator);
				continue;
			}
			
			std::string key = property.Key;
			std::string value = property.Value;
			
			key = FindAndReplaceString(key, "_", "-");
			
			const int resource = GetResourceIdByName(key.c_str());
			if (resource != -1) {
				this->DefaultStat.Costs[resource] = std::stoi(value);
			} else {
				fprintf(stderr, "Invalid resource: \"%s\".\n", key.c_str());
			}
		}
	} else if (section->Tag == "image") {
		for (const CConfigProperty &property : section->Properties) {
			if (property.Operator != CConfigOperator::Assignment) {
				fprintf(stderr, "Wrong operator enumeration index for property \"%s\": %i.\n", property.Key.c_str(), property.Operator);
				continue;
			}
			
			std::string key = property.Key;
			std::string value = property.Value;
			
			if (key == "file") {
				this->File = CMod::GetCurrentModPath() + value;
			} else if (key == "width") {
				this->Width = std::stoi(value);
			} else if (key == "height") {
				this->Height = std::stoi(value);
			} else {
				fprintf(stderr, "Invalid image property: \"%s\".\n", key.c_str());
			}
		}
		
		if (this->File.empty()) {
			fprintf(stderr, "Image has no file.\n");
		}
		
		if (this->Width == 0) {
			fprintf(stderr, "Image has no width.\n");
		}
		
		if (this->Height == 0) {
			fprintf(stderr, "Image has no height.\n");
		}
		
		if (this->Sprite) {
			CGraphic::Free(this->Sprite);
			this->Sprite = nullptr;
		}
	} else if (section->Tag == "default_equipment") {
		for (const CConfigProperty &property : section->Properties) {
			if (property.Operator != CConfigOperator::Assignment) {
				fprintf(stderr, "Wrong operator enumeration index for property \"%s\": %i.\n", property.Key.c_str(), property.Operator);
				continue;
			}
			
			std::string key = property.Key;
			std::string value = property.Value;
			
			key = FindAndReplaceString(key, "_", "-");
			value = FindAndReplaceString(value, "_", "-");
			
			const ItemSlot *item_slot = ItemSlot::Get(key);
			if (item_slot == nullptr) {
				fprintf(stderr, "Invalid item slot for default equipment: \"%s\".\n", key.c_str());
				return true;
			}
			
			CUnitType *item = UnitTypeByIdent(value);
			if (!item) {
				fprintf(stderr, "Invalid item for default equipment: \"%s\".\n", value.c_str());
				return true;
			}
			
			this->DefaultEquipment[item_slot] = item;
		}
	} else if (section->Tag == "sounds") {
		for (const CConfigProperty &property : section->Properties) {
			if (property.Operator != CConfigOperator::Assignment) {
				fprintf(stderr, "Wrong operator enumeration index for property \"%s\": %i.\n", property.Key.c_str(), property.Operator);
				continue;
			}
			
			std::string key = property.Key;
			std::string value = property.Value;
			
			value = FindAndReplaceString(value, "_", "-");
			
			if (key == "selected") {
				this->Sound.Selected.Name = value;
			} else if (key == "acknowledge") {
				this->Sound.Acknowledgement.Name = value;
			} else if (key == "attack") {
				this->Sound.Attack.Name = value;
			} else if (key == "idle") {
				this->Sound.Idle.Name = value;
			} else if (key == "hit") {
				this->Sound.Hit.Name = value;
			} else if (key == "miss") {
				this->Sound.Miss.Name = value;
			} else if (key == "fire_missile") {
				this->Sound.FireMissile.Name = value;
			} else if (key == "step") {
				this->Sound.Step.Name = value;
			} else if (key == "step_dirt") {
				this->Sound.StepDirt.Name = value;
			} else if (key == "step_grass") {
				this->Sound.StepGrass.Name = value;
			} else if (key == "step_gravel") {
				this->Sound.StepGravel.Name = value;
			} else if (key == "step_mud") {
				this->Sound.StepMud.Name = value;
			} else if (key == "step_stone") {
				this->Sound.StepStone.Name = value;
			} else if (key == "used") {
				this->Sound.Used.Name = value;
			} else if (key == "build") {
				this->Sound.Build.Name = value;
			} else if (key == "ready") {
				this->Sound.Ready.Name = value;
			} else if (key == "repair") {
				this->Sound.Repair.Name = value;
			} else if (key.find("harvest_") != std::string::npos) {
				std::string resource_ident = FindAndReplaceString(key, "harvest_", "");
				resource_ident = FindAndReplaceString(resource_ident, "_", "-");
				const int res = GetResourceIdByName(resource_ident.c_str());
				if (res != -1) {
					this->Sound.Harvest[res].Name = value;
				} else {
					fprintf(stderr, "Invalid resource: \"%s\".\n", resource_ident.c_str());
				}
			} else if (key == "help") {
				this->Sound.Help.Name = value;
			} else if (key == "dead") {
				this->Sound.Dead[ANIMATIONS_DEATHTYPES].Name = value;
			} else if (key.find("dead_") != std::string::npos) {
				std::string death_type_ident = FindAndReplaceString(key, "dead_", "");
				death_type_ident = FindAndReplaceString(death_type_ident, "_", "-");
				int death;
				for (death = 0; death < ANIMATIONS_DEATHTYPES; ++death) {
					if (death_type_ident == ExtraDeathTypes[death]) {
						this->Sound.Dead[death].Name = value;
						break;
					}
				}
				if (death == ANIMATIONS_DEATHTYPES) {
					fprintf(stderr, "Invalid death type: \"%s\".\n", death_type_ident.c_str());
				}
			} else {
				fprintf(stderr, "Invalid sound tag: \"%s\".\n", key.c_str());
			}
		}
	} else if (section->Tag == "predependencies") {
		this->Predependency = new CAndDependency;
		this->Predependency->ProcessConfigData(section);
	} else if (section->Tag == "dependencies") {
		this->Dependency = new CAndDependency;
		this->Dependency->ProcessConfigData(section);
	} else if (section->Tag == "variation") {
		this->DefaultStat.Variables[VARIATION_INDEX].Enable = 1;
		this->DefaultStat.Variables[VARIATION_INDEX].Value = 0;
		
		CUnitTypeVariation *variation = new CUnitTypeVariation;
		variation->ProcessConfigData(section);
		
		if (variation->ImageLayer == -1) {
			variation->ID = this->Variations.size();
			this->Variations.push_back(variation);
		} else {
			variation->ID = this->LayerVariations[variation->ImageLayer].size();
			this->LayerVariations[variation->ImageLayer].push_back(variation);
		}
		
		this->DefaultStat.Variables[VARIATION_INDEX].Max = this->Variations.size();
	} else {
		std::string tag = SnakeCaseToPascalCase(section->Tag);
		
		const int index = UnitTypeVar.VariableNameLookup[tag.c_str()]; // variable index
		
		if (index != -1) { // valid index
			for (const CConfigProperty &property : section->Properties) {
				if (property.Operator != CConfigOperator::Assignment) {
					fprintf(stderr, "Wrong operator enumeration index for property \"%s\": %i.\n", property.Key.c_str(), property.Operator);
					continue;
				}
				
				std::string key = property.Key;
				std::string value = property.Value;
				
				if (key == "enable") {
					this->DefaultStat.Variables[index].Enable = StringToBool(value);
				} else if (key == "value") {
					this->DefaultStat.Variables[index].Value = std::stoi(value);
				} else if (key == "max") {
					this->DefaultStat.Variables[index].Max = std::stoi(value);
				} else if (key == "increase") {
					this->DefaultStat.Variables[index].Increase = std::stoi(value);
				} else {
					fprintf(stderr, "Invalid variable property: \"%s\".\n", key.c_str());
				}
			}
		} else {
			return false;
		}
	}
	
	return true;
}
	
/**
**	@brief	Initialize the unit type
*/
void CUnitType::Initialize()
{	
	this->RemoveButtons(ButtonMove);
	this->RemoveButtons(ButtonStop);
	this->RemoveButtons(ButtonAttack);
	this->RemoveButtons(ButtonPatrol);
	this->RemoveButtons(ButtonStandGround);
	this->RemoveButtons(ButtonReturn);
	
	if (this->Class != nullptr) { //if class is defined, then use this unit type to help build the classes table, and add this unit to the civilization class table (if the civilization is defined)
		const UnitClass *unit_class = this->Class;

		//see if this unit type is set as the civilization class unit type or the faction class unit type of any civilization/class (or faction/class) combination, and remove it from there (to not create problems with redefinitions)
		for (CCivilization *civilization : CCivilization::GetAll()) {
			for (std::map<const UnitClass *, int>::reverse_iterator iterator = civilization->ClassUnitTypes.rbegin(); iterator != civilization->ClassUnitTypes.rend(); ++iterator) {
				if (iterator->second == this->Slot) {
					civilization->ClassUnitTypes.erase(iterator->first);
					break;
				}
			}
		}
		for (CFaction *faction : CFaction::GetAll()) {
			for (std::map<const UnitClass *, int>::reverse_iterator iterator = faction->ClassUnitTypes.rbegin(); iterator != faction->ClassUnitTypes.rend(); ++iterator) {
				if (iterator->second == this->Slot) {
					faction->ClassUnitTypes.erase(iterator->first);
					break;
				}
			}
		}
		
		if (this->GetCivilization() != nullptr && unit_class != nullptr) {
			if (this->Faction != nullptr) {
				this->Faction->ClassUnitTypes[unit_class] = this->Slot;
			} else {
				this->GetCivilization()->ClassUnitTypes[unit_class] = this->Slot;
			}
		}
	}
	
	// If number of directions is not specified, make a guess
	// Building have 1 direction and units 8
	if (this->BoolFlag[BUILDING_INDEX].value && this->NumDirections == 0) {
		this->NumDirections = 1;
	} else if (this->NumDirections == 0) {
		this->NumDirections = 8;
	}
	
	//Wyrmgus start
	//unit type's level must be at least 1
	if (this->DefaultStat.Variables[LEVEL_INDEX].Value == 0) {
		this->DefaultStat.Variables[LEVEL_INDEX].Enable = 1;
		this->DefaultStat.Variables[LEVEL_INDEX].Value = 1;
		this->DefaultStat.Variables[LEVEL_INDEX].Max = 1;
	}
	//Wyrmgus end

	// FIXME: try to simplify/combine the flags instead
	if (this->MouseAction == MouseActionAttack && !this->CanAttack) {
		fprintf(stderr, "Unit-type '%s': right-attack is set, but can-attack is not.\n", this->Name.utf8().get_data());
	}
	this->UpdateDefaultBoolFlags();
	//Wyrmgus start
	if (GameRunning || Editor.Running == EditorEditing) {
		InitUnitType(*this);
		LoadUnitType(*this);
	}
	//Wyrmgus end
	//Wyrmgus start
//	if (!CclInConfigFile) {
	if (!CclInConfigFile || GameRunning || Editor.Running == EditorEditing) {
	//Wyrmgus end
		UpdateUnitStats(*this, 1);
	}
	//Wyrmgus start
	if (Editor.Running == EditorEditing && std::find(Editor.UnitTypes.begin(), Editor.UnitTypes.end(), this->Ident) == Editor.UnitTypes.end()) {
		Editor.UnitTypes.push_back(this->Ident);
		RecalculateShownUnits();
	}
	
	for (size_t i = 0; i < this->Trains.size(); ++i) {
		std::string button_definition = "DefineButton({\n";
		button_definition += "\tPos = " + std::to_string((long long) this->Trains[i]->ButtonPos) + ",\n";
		if (this->Trains[i]->ButtonLevel) {
			button_definition += "\tLevel = " + this->Trains[i]->ButtonLevel->Ident + ",\n";
		}
		button_definition += "\tAction = ";
		if (this->Trains[i]->BoolFlag[BUILDING_INDEX].value) {
			button_definition += "\"build\"";
		} else {
			button_definition += "\"train-unit\"";
		}
		button_definition += ",\n";
		button_definition += "\tValue = \"" + this->Trains[i]->Ident + "\",\n";
		if (!this->Trains[i]->ButtonPopup.empty()) {
			button_definition += "\tPopup = \"" + this->Trains[i]->ButtonPopup + "\",\n";
		}
		button_definition += "\tKey = \"" + this->Trains[i]->ButtonKey + "\",\n";
		button_definition += "\tHint = \"" + this->Trains[i]->ButtonHint + "\",\n";
		button_definition += "\tForUnit = {\"" + this->Ident + "\"},\n";
		button_definition += "})";
		CclCommand(button_definition);
	}
	
	if (this->CanMove()) {
		std::string button_definition = "DefineButton({\n";
		button_definition += "\tPos = 1,\n";
		button_definition += "\tAction = \"move\",\n";
		button_definition += "\tPopup = \"popup-commands\",\n";
		button_definition += "\tKey = \"m\",\n";
		button_definition += "\tHint = _(\"~!Move\"),\n";
		button_definition += "\tForUnit = {\"" + this->Ident + "\"},\n";
		button_definition += "})";
		CclCommand(button_definition);
	}
	
	if (this->CanMove()) {
		std::string button_definition = "DefineButton({\n";
		button_definition += "\tPos = 2,\n";
		button_definition += "\tAction = \"stop\",\n";
		button_definition += "\tPopup = \"popup-commands\",\n";
		button_definition += "\tKey = \"s\",\n";
		button_definition += "\tHint = _(\"~!Stop\"),\n";
		button_definition += "\tForUnit = {\"" + this->Ident + "\"},\n";
		button_definition += "})";
		CclCommand(button_definition);
	}
	
	if (this->CanMove() && this->CanAttack) {
		std::string button_definition = "DefineButton({\n";
		button_definition += "\tPos = 3,\n";
		button_definition += "\tAction = \"attack\",\n";
		button_definition += "\tPopup = \"popup-commands\",\n";
		button_definition += "\tKey = \"a\",\n";
		button_definition += "\tHint = _(\"~!Attack\"),\n";
		button_definition += "\tForUnit = {\"" + this->Ident + "\"},\n";
		button_definition += "})";
		CclCommand(button_definition);
	}
	
	if (this->CanMove() && ((!this->BoolFlag[COWARD_INDEX].value && this->CanAttack) || this->UnitType == UnitTypeFly)) {
		std::string button_definition = "DefineButton({\n";
		button_definition += "\tPos = 4,\n";
		button_definition += "\tAction = \"patrol\",\n";
		button_definition += "\tPopup = \"popup-commands\",\n";
		button_definition += "\tKey = \"p\",\n";
		button_definition += "\tHint = _(\"~!Patrol\"),\n";
		button_definition += "\tForUnit = {\"" + this->Ident + "\"},\n";
		button_definition += "})";
		CclCommand(button_definition);
	}
	
	if (this->CanMove() && !this->BoolFlag[COWARD_INDEX].value && this->CanAttack && !(this->CanTransport() && this->BoolFlag[ATTACKFROMTRANSPORTER_INDEX].value)) {
		std::string button_definition = "DefineButton({\n";
		button_definition += "\tPos = 5,\n";
		button_definition += "\tAction = \"stand-ground\",\n";
		button_definition += "\tPopup = \"popup-commands\",\n";
		button_definition += "\tKey = \"t\",\n";
		button_definition += "\tHint = _(\"S~!tand Ground\"),\n";
		button_definition += "\tForUnit = {\"" + this->Ident + "\"},\n";
		button_definition += "})";
		CclCommand(button_definition);
	}
	
	// make units allowed by default
	for (int i = 0; i < PlayerMax; ++i) {
		AllowUnitId(*CPlayer::Players[i], this->Slot, 65536);
	}
	
	this->Initialized = true;
	
	CclCommand("if not (GetArrayIncludes(Units, \"" + this->Ident + "\")) then table.insert(Units, \"" + this->Ident + "\") end"); //FIXME: needed at present to make unit type data files work without scripting being necessary, but it isn't optimal to interact with a scripting table like "Units" in this manner (that table should probably be replaced with getting a list of unit types from the engine)
}

Vec2i CUnitType::GetTileSize() const
{
	return this->TileSize;
}

Vec2i CUnitType::GetHalfTileSize() const
{
	return this->GetTileSize() / 2;
}

PixelSize CUnitType::GetTilePixelSize(const int map_layer) const
{
	return PixelSize(PixelSize(this->GetTileSize()) * CMap::Map.GetMapLayerPixelTileSize(map_layer));
}

Vec2i CUnitType::GetTileCenterPosOffset() const
{
	return (this->GetTileSize() - 1) / 2;
}
	
bool CUnitType::CheckUserBoolFlags(const char *BoolFlags) const
{
	for (unsigned int i = 0; i < UnitTypeVar.GetNumberBoolFlag(); ++i) { // User defined flags
		if (BoolFlags[i] != CONDITION_TRUE &&
			((BoolFlags[i] == CONDITION_ONLY) ^ (BoolFlag[i].value))) {
			return false;
		}
	}
	return true;
}

bool CUnitType::CanMove() const
{
	return Animations && Animations->Move;
}

bool CUnitType::CanSelect(GroupSelectionMode mode) const
{
	if (!BoolFlag[ISNOTSELECTABLE_INDEX].value) {
		switch (mode) {
			case SELECTABLE_BY_RECTANGLE_ONLY:
				return BoolFlag[SELECTABLEBYRECTANGLE_INDEX].value;
			case NON_SELECTABLE_BY_RECTANGLE_ONLY:
				return !BoolFlag[SELECTABLEBYRECTANGLE_INDEX].value;
			default:
				return true;
		}
	}
	return false;
}

void CUnitType::SetParent(CUnitType *parent_type)
{
	if (!parent_type->Initialized) {
		fprintf(stderr, "Unit type \"%s\" is inheriting features from a non-initialized parent (\"%s\").\n", this->Ident.c_str(), parent_type->Ident.c_str());
	}
	
	this->Parent = parent_type;
	
	if (this->Name.empty()) {
		this->Name = parent_type->Name;
	}
	this->Class = parent_type->Class;
	if (this->Class != nullptr && std::find(this->Class->UnitTypes.begin(), this->Class->UnitTypes.end(), this) == this->Class->UnitTypes.end()) {
		this->Class->UnitTypes.push_back(this);
	}
	this->DrawLevel = parent_type->DrawLevel;
	this->File = parent_type->File;
	this->Width = parent_type->Width;
	this->Height = parent_type->Height;
	this->OffsetX = parent_type->OffsetX;
	this->OffsetY = parent_type->OffsetY;
	this->ShadowFile = parent_type->ShadowFile;
	this->ShadowWidth = parent_type->ShadowWidth;
	this->ShadowHeight = parent_type->ShadowHeight;
	this->ShadowOffsetX = parent_type->ShadowOffsetX;
	this->ShadowOffsetY = parent_type->ShadowOffsetY;
	this->LightFile = parent_type->LightFile;
	this->TileSize = parent_type->TileSize;
	this->BoxWidth = parent_type->BoxWidth;
	this->BoxHeight = parent_type->BoxHeight;
	this->BoxOffsetX = parent_type->BoxOffsetX;
	this->BoxOffsetY = parent_type->BoxOffsetY;
	this->Construction = parent_type->Construction;
	this->UnitType = parent_type->UnitType;
	this->Missile.Name = parent_type->Missile.Name;
	this->Missile.Missile = nullptr;
	this->FireMissile.Name = parent_type->FireMissile.Name;
	this->FireMissile.Missile = nullptr;
	this->ExplodeWhenKilled = parent_type->ExplodeWhenKilled;
	this->Explosion.Name = parent_type->Explosion.Name;
	this->Explosion.Missile = nullptr;
	this->CorpseName = parent_type->CorpseName;
	this->CorpseType = nullptr;
	this->MinAttackRange = parent_type->MinAttackRange;
	this->DefaultStat.Variables[ATTACKRANGE_INDEX].Value = parent_type->DefaultStat.Variables[ATTACKRANGE_INDEX].Value;
	this->DefaultStat.Variables[ATTACKRANGE_INDEX].Max = parent_type->DefaultStat.Variables[ATTACKRANGE_INDEX].Max;
	this->DefaultStat.Variables[PRIORITY_INDEX].Value = parent_type->DefaultStat.Variables[PRIORITY_INDEX].Value;
	this->DefaultStat.Variables[PRIORITY_INDEX].Max  = parent_type->DefaultStat.Variables[PRIORITY_INDEX].Max;
	this->AnnoyComputerFactor = parent_type->AnnoyComputerFactor;
	this->TrainQuantity = parent_type->TrainQuantity;
	this->CostModifier = parent_type->CostModifier;
	this->ItemClass = parent_type->ItemClass;
	this->MaxOnBoard = parent_type->MaxOnBoard;
	this->RepairRange = parent_type->RepairRange;
	this->RepairHP = parent_type->RepairHP;
	this->MouseAction = parent_type->MouseAction;
	this->CanAttack = parent_type->CanAttack;
	this->CanTarget = parent_type->CanTarget;
	this->LandUnit = parent_type->LandUnit;
	this->SeaUnit = parent_type->SeaUnit;
	this->AirUnit = parent_type->AirUnit;
	this->BoardSize = parent_type->BoardSize;
	this->ButtonLevelForTransporter = parent_type->ButtonLevelForTransporter;
	this->ButtonPos = parent_type->ButtonPos;
	this->ButtonLevel = parent_type->ButtonLevel;
	this->ButtonPopup = parent_type->ButtonPopup;
	this->ButtonHint = parent_type->ButtonHint;
	this->ButtonKey = parent_type->ButtonKey;
	this->BurnPercent = parent_type->BurnPercent;
	this->BurnDamageRate = parent_type->BurnDamageRate;
	this->PoisonDrain = parent_type->PoisonDrain;
	this->AutoBuildRate = parent_type->AutoBuildRate;
	this->Animations = parent_type->Animations;
	this->Sound = parent_type->Sound;
	this->NumDirections = parent_type->NumDirections;
	this->NeutralMinimapColorRGB = parent_type->NeutralMinimapColorRGB;
	this->RandomMovementProbability = parent_type->RandomMovementProbability;
	this->RandomMovementDistance = parent_type->RandomMovementDistance;
	this->GivesResource = parent_type->GivesResource;
	this->RequirementsString = parent_type->RequirementsString;
	this->ExperienceRequirementsString = parent_type->ExperienceRequirementsString;
	this->BuildingRulesString = parent_type->BuildingRulesString;
	this->Elixir = parent_type->Elixir;
	this->Icon.Name = parent_type->Icon.Name;
	this->Icon.Icon = nullptr;
	if (!this->Icon.Name.empty()) {
		this->Icon.Load();
	}
	for (size_t i = 0; i < parent_type->Spells.size(); ++i) {
		this->Spells.push_back(parent_type->Spells[i]);
	}
	if (parent_type->AutoCastActive) {
		this->AutoCastActive = new char[CSpell::Spells.size()];
		memset(this->AutoCastActive, 0, CSpell::Spells.size() * sizeof(char));
		for (unsigned int i = 0; i < CSpell::Spells.size(); ++i) {
			this->AutoCastActive[i] = parent_type->AutoCastActive[i];
		}
	}
	for (unsigned int i = 0; i < MaxCosts; ++i) {
		this->DefaultStat.Costs[i] = parent_type->DefaultStat.Costs[i];
		this->RepairCosts[i] = parent_type->RepairCosts[i];
		this->DefaultStat.ImproveIncomes[i] = parent_type->DefaultStat.ImproveIncomes[i];
		this->DefaultStat.ResourceDemand[i] = parent_type->DefaultStat.ResourceDemand[i];
		this->CanStore[i] = parent_type->CanStore[i];
		this->GrandStrategyProductionEfficiencyModifier[i] = parent_type->GrandStrategyProductionEfficiencyModifier[i];
		
		if (parent_type->ResInfo[i]) {
			ResourceInfo *res = new ResourceInfo;
			res->ResourceId = i;
			this->ResInfo[i] = res;
			res->ResourceStep = parent_type->ResInfo[i]->ResourceStep;
			res->WaitAtResource = parent_type->ResInfo[i]->WaitAtResource;
			res->WaitAtDepot = parent_type->ResInfo[i]->WaitAtDepot;
			res->ResourceCapacity = parent_type->ResInfo[i]->ResourceCapacity;
			res->LoseResources = parent_type->ResInfo[i]->LoseResources;
			res->RefineryHarvester = parent_type->ResInfo[i]->RefineryHarvester;
			res->FileWhenEmpty = parent_type->ResInfo[i]->FileWhenEmpty;
			res->FileWhenLoaded = parent_type->ResInfo[i]->FileWhenLoaded;
		}
	}
	
	this->DefaultStat.UnitStock = parent_type->DefaultStat.UnitStock;

	for (unsigned int i = 0; i < UnitTypeVar.GetNumberVariable(); ++i) {
		this->DefaultStat.Variables[i].Enable = parent_type->DefaultStat.Variables[i].Enable;
		this->DefaultStat.Variables[i].Value = parent_type->DefaultStat.Variables[i].Value;
		this->DefaultStat.Variables[i].Max = parent_type->DefaultStat.Variables[i].Max;
		this->DefaultStat.Variables[i].Increase = parent_type->DefaultStat.Variables[i].Increase;
	}
	for (unsigned int i = 0; i < UnitTypeVar.GetNumberBoolFlag(); ++i) {
		this->BoolFlag[i].value = parent_type->BoolFlag[i].value;
		this->BoolFlag[i].CanTransport = parent_type->BoolFlag[i].CanTransport;
	}
	for (size_t i = 0; i < parent_type->WeaponClasses.size(); ++i) {
		this->WeaponClasses.push_back(parent_type->WeaponClasses[i]);
	}
	for (size_t i = 0; i < parent_type->SoldUnits.size(); ++i) {
		this->SoldUnits.push_back(parent_type->SoldUnits[i]);
	}
	for (size_t i = 0; i < parent_type->SpawnUnits.size(); ++i) {
		this->SpawnUnits.push_back(parent_type->SpawnUnits[i]);
	}
	for (size_t i = 0; i < parent_type->Drops.size(); ++i) {
		this->Drops.push_back(parent_type->Drops[i]);
	}
	for (size_t i = 0; i < parent_type->AiDrops.size(); ++i) {
		this->AiDrops.push_back(parent_type->AiDrops[i]);
	}
	for (size_t i = 0; i < parent_type->DropSpells.size(); ++i) {
		this->DropSpells.push_back(parent_type->DropSpells[i]);
	}
	for (size_t i = 0; i < parent_type->Affixes.size(); ++i) {
		this->Affixes.push_back(parent_type->Affixes[i]);
	}
	for (size_t i = 0; i < parent_type->Traits.size(); ++i) {
		this->Traits.push_back(parent_type->Traits[i]);
	}
	for (size_t i = 0; i < parent_type->StartingAbilities.size(); ++i) {
		this->StartingAbilities.push_back(parent_type->StartingAbilities[i]);
	}
	for (size_t i = 0; i < parent_type->Trains.size(); ++i) {
		this->Trains.push_back(parent_type->Trains[i]);
		parent_type->Trains[i]->TrainedBy.push_back(this);
	}
	for (size_t i = 0; i < parent_type->StartingResources.size(); ++i) {
		this->StartingResources.push_back(parent_type->StartingResources[i]);
	}
	for (std::map<int, std::vector<std::string>>::iterator iterator = parent_type->PersonalNames.begin(); iterator != parent_type->PersonalNames.end(); ++iterator) {
		for (size_t i = 0; i < iterator->second.size(); ++i) {
			this->PersonalNames[iterator->first].push_back(iterator->second[i]);				
		}
	}
	for (CUnitTypeVariation *parent_variation : parent_type->Variations) {
		CUnitTypeVariation *variation = new CUnitTypeVariation;
		
		variation->ID = this->Variations.size();
		this->Variations.push_back(variation);
		
		variation->VariationId = parent_variation->VariationId;
		variation->TypeName = parent_variation->TypeName;
		variation->File = parent_variation->File;
		for (unsigned int i = 0; i < MaxCosts; ++i) {
			variation->FileWhenLoaded[i] = parent_variation->FileWhenLoaded[i];
			variation->FileWhenEmpty[i] = parent_variation->FileWhenEmpty[i];
		}
		variation->ShadowFile = parent_variation->ShadowFile;
		variation->LightFile = parent_variation->LightFile;
		variation->FrameWidth = parent_variation->FrameWidth;
		variation->FrameHeight = parent_variation->FrameHeight;
		variation->ResourceMin = parent_variation->ResourceMin;
		variation->ResourceMax = parent_variation->ResourceMax;
		variation->Weight = parent_variation->Weight;
		variation->Icon.Name = parent_variation->Icon.Name;
		variation->Icon.Icon = nullptr;
		if (!variation->Icon.Name.empty()) {
			variation->Icon.Load();
		}
		if (parent_variation->Animations) {
			variation->Animations = parent_variation->Animations;
		}
		variation->Construction = parent_variation->Construction;
		variation->UpgradesRequired = parent_variation->UpgradesRequired;
		variation->UpgradesForbidden = parent_variation->UpgradesForbidden;
		for (const ::ItemClass *item_class : parent_variation->ItemClassesEquipped) {
			variation->ItemClassesEquipped.push_back(item_class);
		}
		for (const ::ItemClass *item_class : parent_variation->ItemClassesNotEquipped) {
			variation->ItemClassesNotEquipped.push_back(item_class);
		}
		for (size_t i = 0; i < parent_variation->ItemsEquipped.size(); ++i) {
			variation->ItemsEquipped.push_back(parent_variation->ItemsEquipped[i]);
		}
		for (size_t i = 0; i < parent_variation->ItemsNotEquipped.size(); ++i) {
			variation->ItemsNotEquipped.push_back(parent_variation->ItemsNotEquipped[i]);
		}
		for (size_t i = 0; i < parent_variation->Terrains.size(); ++i) {
			variation->Terrains.push_back(parent_variation->Terrains[i]);
		}
		
		for (int i = 0; i < MaxImageLayers; ++i) {
			variation->LayerFiles[i] = parent_variation->LayerFiles[i];
		}
		for (std::map<int, IconConfig>::iterator iterator = parent_variation->ButtonIcons.begin(); iterator != parent_variation->ButtonIcons.end(); ++iterator) {
			variation->ButtonIcons[iterator->first].Name = iterator->second.Name;
			variation->ButtonIcons[iterator->first].Icon = nullptr;
			variation->ButtonIcons[iterator->first].Load();
			variation->ButtonIcons[iterator->first].Icon->Load();
		}
	}
	
	for (int i = 0; i < MaxImageLayers; ++i) {
		this->LayerFiles[i] = parent_type->LayerFiles[i];
		
		//inherit layer variations
		for (CUnitTypeVariation *parent_variation : parent_type->LayerVariations[i]) {
			CUnitTypeVariation *variation = new CUnitTypeVariation;
			
			variation->ID = this->LayerVariations[i].size();
			this->LayerVariations[i].push_back(variation);
				
			variation->VariationId = parent_variation->VariationId;
			variation->File = parent_variation->File;
			variation->UpgradesRequired = parent_variation->UpgradesRequired;
			variation->UpgradesForbidden = parent_variation->UpgradesForbidden;
			for (const ::ItemClass *item_class : parent_variation->ItemClassesEquipped) {
				variation->ItemClassesEquipped.push_back(item_class);
			}
			for (const ::ItemClass *item_class : parent_variation->ItemClassesNotEquipped) {
				variation->ItemClassesNotEquipped.push_back(item_class);
			}
			for (size_t u = 0; u < parent_variation->ItemsEquipped.size(); ++u) {
				variation->ItemsEquipped.push_back(parent_variation->ItemsEquipped[u]);
			}
			for (size_t u = 0; u < parent_variation->ItemsNotEquipped.size(); ++u) {
				variation->ItemsNotEquipped.push_back(parent_variation->ItemsNotEquipped[u]);
			}
			for (size_t u = 0; u < parent_variation->Terrains.size(); ++u) {
				variation->Terrains.push_back(parent_variation->Terrains[u]);
			}
		}
	}
	for (std::map<int, IconConfig>::iterator iterator = parent_type->ButtonIcons.begin(); iterator != parent_type->ButtonIcons.end(); ++iterator) {
		this->ButtonIcons[iterator->first].Name = iterator->second.Name;
		this->ButtonIcons[iterator->first].Icon = nullptr;
		this->ButtonIcons[iterator->first].Load();
		this->ButtonIcons[iterator->first].Icon->Load();
	}
	for (std::map<const ItemSlot *, CUnitType *>::iterator iterator = parent_type->DefaultEquipment.begin(); iterator != parent_type->DefaultEquipment.end(); ++iterator) {
		this->DefaultEquipment[iterator->first] = iterator->second;
	}
	this->DefaultStat.Variables[PRIORITY_INDEX].Value = parent_type->DefaultStat.Variables[PRIORITY_INDEX].Value + 1; //increase priority by 1 to make it be chosen by the AI when building over the previous unit
	this->DefaultStat.Variables[PRIORITY_INDEX].Max = parent_type->DefaultStat.Variables[PRIORITY_INDEX].Max + 1;
}

void CUnitType::UpdateDefaultBoolFlags()
{
	// BoolFlag
	this->BoolFlag[FLIP_INDEX].value = this->Flip;
	this->BoolFlag[LANDUNIT_INDEX].value = this->LandUnit;
	this->BoolFlag[AIRUNIT_INDEX].value = this->AirUnit;
	this->BoolFlag[SEAUNIT_INDEX].value = this->SeaUnit;
	this->BoolFlag[EXPLODEWHENKILLED_INDEX].value = this->ExplodeWhenKilled;
	this->BoolFlag[CANATTACK_INDEX].value = this->CanAttack;
}

//Wyrmgus start
void CUnitType::RemoveButtons(int button_action, std::string mod_file)
{
	int buttons_size = UnitButtonTable.size();
	for (int i = (buttons_size - 1); i >= 0; --i) {
		if (button_action != -1 && UnitButtonTable[i]->Action != button_action) {
			continue;
		}
		if (!mod_file.empty() && UnitButtonTable[i]->Mod != mod_file) {
			continue;
		}
		
		if (UnitButtonTable[i]->UnitMask == ("," + this->Ident + ",")) { //delete the appropriate buttons
			delete UnitButtonTable[i];
			UnitButtonTable.erase(std::remove(UnitButtonTable.begin(), UnitButtonTable.end(), UnitButtonTable[i]), UnitButtonTable.end());
		} else if (UnitButtonTable[i]->UnitMask.find(this->Ident) != std::string::npos) { //remove this unit from the "ForUnit" array of the appropriate buttons
			UnitButtonTable[i]->UnitMask = FindAndReplaceString(UnitButtonTable[i]->UnitMask, this->Ident + ",", "");
		}
	}
}

int CUnitType::GetAvailableLevelUpUpgrades() const
{
	int value = 0;
	int upgrade_value = 0;
	
	if (((int) AiHelpers.ExperienceUpgrades.size()) > this->Slot) {
		for (size_t i = 0; i != AiHelpers.ExperienceUpgrades[this->Slot].size(); ++i) {
			int local_upgrade_value = 1;
			
			local_upgrade_value += AiHelpers.ExperienceUpgrades[this->Slot][i]->GetAvailableLevelUpUpgrades();
			
			if (local_upgrade_value > upgrade_value) {
				upgrade_value = local_upgrade_value;
			}
		}
	}
	
	value += upgrade_value;
	
	if (((int) AiHelpers.LearnableAbilities.size()) > this->Slot) {
		for (size_t i = 0; i != AiHelpers.LearnableAbilities[this->Slot].size(); ++i) {
			value += 1;
		}
	}
	
	return value;
}

int CUnitType::GetResourceStep(const int resource, const int player) const
{
	if (!this->ResInfo[resource]) {
		return 0;
	}

	int resource_step = this->ResInfo[resource]->ResourceStep;
	
	resource_step += this->Stats[player].Variables[GATHERINGBONUS_INDEX].Value;
	
	if (resource == CopperCost) {
		resource_step += this->Stats[player].Variables[COPPERGATHERINGBONUS_INDEX].Value;
	} else if (resource == SilverCost) {
		resource_step += this->Stats[player].Variables[SILVERGATHERINGBONUS_INDEX].Value;
	} else if (resource == GoldCost) {
		resource_step += this->Stats[player].Variables[GOLDGATHERINGBONUS_INDEX].Value;
	} else if (resource == IronCost) {
		resource_step += this->Stats[player].Variables[IRONGATHERINGBONUS_INDEX].Value;
	} else if (resource == MithrilCost) {
		resource_step += this->Stats[player].Variables[MITHRILGATHERINGBONUS_INDEX].Value;
	} else if (resource == WoodCost) {
		resource_step += this->Stats[player].Variables[LUMBERGATHERINGBONUS_INDEX].Value;
	} else if (resource == StoneCost || resource == LimestoneCost) {
		resource_step += this->Stats[player].Variables[STONEGATHERINGBONUS_INDEX].Value;
	} else if (resource == CoalCost) {
		resource_step += this->Stats[player].Variables[COALGATHERINGBONUS_INDEX].Value;
	} else if (resource == JewelryCost) {
		resource_step += this->Stats[player].Variables[JEWELRYGATHERINGBONUS_INDEX].Value;
	} else if (resource == FurnitureCost) {
		resource_step += this->Stats[player].Variables[FURNITUREGATHERINGBONUS_INDEX].Value;
	} else if (resource == LeatherCost) {
		resource_step += this->Stats[player].Variables[LEATHERGATHERINGBONUS_INDEX].Value;
	} else if (resource == DiamondsCost || resource == EmeraldsCost) {
		resource_step += this->Stats[player].Variables[GEMSGATHERINGBONUS_INDEX].Value;
	}
	
	return resource_step;
}

CUnitTypeVariation *CUnitType::GetDefaultVariation(const CPlayer *player, const int image_layer) const
{
	const std::vector<CUnitTypeVariation *> &variation_list = image_layer == -1 ? this->Variations : this->LayerVariations[image_layer];
	for (CUnitTypeVariation *variation : variation_list) {
		bool upgrades_check = true;
		for (const CUpgrade *required_upgrade : variation->UpgradesRequired) {
			if (UpgradeIdentAllowed(*player, required_upgrade->Ident.c_str()) != 'R') {
				upgrades_check = false;
				break;
			}
		}
		
		if (upgrades_check) {
			for (const CUpgrade *forbidden_upgrade : variation->UpgradesForbidden) {
				if (UpgradeIdentAllowed(*player, forbidden_upgrade->Ident.c_str()) == 'R') {
					upgrades_check = false;
					break;
				}
			}
		}
		
		if (upgrades_check == false) {
			continue;
		}
		return variation;
	}
	return nullptr;
}

CUnitTypeVariation *CUnitType::GetVariation(const std::string &variation_name, int image_layer) const
{
	const std::vector<CUnitTypeVariation *> &variation_list = image_layer == -1 ? this->Variations : this->LayerVariations[image_layer];
	for (CUnitTypeVariation *variation : variation_list) {
		if (variation->VariationId == variation_name) {
			return variation;
		}
	}
	return nullptr;
}

std::string CUnitType::GetRandomVariationIdent(int image_layer) const
{
	std::vector<std::string> variation_idents;
	
	const std::vector<CUnitTypeVariation *> &variation_list = image_layer == -1 ? this->Variations : this->LayerVariations[image_layer];
	for (const CUnitTypeVariation *variation : variation_list) {
		variation_idents.push_back(variation->VariationId);
	}
	
	if (variation_idents.size() > 0) {
		return variation_idents[SyncRand(variation_idents.size())];
	}
	
	return "";
}

std::string CUnitType::GetDefaultName(const CPlayer *player) const
{
	CUnitTypeVariation *variation = this->GetDefaultVariation(player);
	if (variation && !variation->TypeName.empty()) {
		return variation->TypeName;
	} else {
		return this->Name.utf8().get_data();
	}
}

CPlayerColorGraphic *CUnitType::GetDefaultLayerSprite(const CPlayer *player, const int image_layer) const
{
	CUnitTypeVariation *variation = this->GetDefaultVariation(player);
	if (this->LayerVariations[image_layer].size() > 0 && this->GetDefaultVariation(player, image_layer)->Sprite) {
		return this->GetDefaultVariation(player, image_layer)->Sprite;
	} else if (variation && variation->LayerSprites[image_layer]) {
		return variation->LayerSprites[image_layer];
	} else if (this->LayerSprites[image_layer])  {
		return this->LayerSprites[image_layer];
	} else {
		return nullptr;
	}
}

bool CUnitType::CanExperienceUpgradeTo(CUnitType *type) const
{
	if (((int) AiHelpers.ExperienceUpgrades.size()) > this->Slot) {
		for (size_t i = 0; i != AiHelpers.ExperienceUpgrades[this->Slot].size(); ++i) {
			if (type == AiHelpers.ExperienceUpgrades[this->Slot][i] || AiHelpers.ExperienceUpgrades[this->Slot][i]->CanExperienceUpgradeTo(type)) {
				return true;
			}
		}
	}
	
	return false;
}

std::string CUnitType::GetNamePlural() const
{
	return GetPluralForm(this->Name.utf8().get_data());
}

std::string CUnitType::GeneratePersonalName(const CFaction *faction, const int gender) const
{
	if (Editor.Running == EditorEditing) { // don't set the personal name if in the editor
		return "";
	}
	
	std::vector<std::string> potential_names = this->GetPotentialPersonalNames(faction, gender);
	
	if (potential_names.size() > 0) {
		return potential_names[SyncRand(potential_names.size())];
	}

	return "";
}

bool CUnitType::IsPersonalNameValid(const std::string &name, const CFaction *faction, const int gender) const
{
	if (name.empty()) {
		return false;
	}
	
	std::vector<std::string> potential_names = this->GetPotentialPersonalNames(faction, gender);
	
	if (std::find(potential_names.begin(), potential_names.end(), name) != potential_names.end()) {
		return true;
	}

	return false;
}

std::vector<std::string> CUnitType::GetPotentialPersonalNames(const CFaction *faction, const int gender) const
{
	std::vector<std::string> potential_names;
	
	if (this->PersonalNames.find(NoGender) != this->PersonalNames.end()) {
		for (size_t i = 0; i < this->PersonalNames.find(NoGender)->second.size(); ++i) {
			potential_names.push_back(this->PersonalNames.find(NoGender)->second[i]);
		}
	}
	if (gender != -1 && gender != NoGender && this->PersonalNames.find(gender) != this->PersonalNames.end()) {
		for (size_t i = 0; i < this->PersonalNames.find(gender)->second.size(); ++i) {
			potential_names.push_back(this->PersonalNames.find(gender)->second[i]);
		}
	}
	
	if (potential_names.size() == 0 && this->GetCivilization() != nullptr) {
		const CCivilization *civilization = this->GetCivilization();
		if (faction && civilization != faction->Civilization && civilization->GetSpecies() == faction->Civilization->GetSpecies() && this->Slot == CFaction::GetFactionClassUnitType(faction, this->Class)) {
			civilization = faction->Civilization;
		}
		if (faction && faction->Civilization != civilization) {
			faction = nullptr;
		}
		if (this->Faction != nullptr && !faction) {
			faction = this->Faction;
		}
		
		if (this->BoolFlag[ORGANIC_INDEX].value) {
			if (civilization->GetPersonalNames().find(NoGender) != civilization->GetPersonalNames().end()) {
				for (size_t i = 0; i < civilization->GetPersonalNames().find(NoGender)->second.size(); ++i) {
					potential_names.push_back(civilization->GetPersonalNames().find(NoGender)->second[i]);
				}
			}
			if (gender != -1 && gender != NoGender && civilization->GetPersonalNames().find(gender) != civilization->GetPersonalNames().end()) {
				for (size_t i = 0; i < civilization->GetPersonalNames().find(gender)->second.size(); ++i) {
					potential_names.push_back(civilization->GetPersonalNames().find(gender)->second[i]);
				}
			}
		} else {
			if (this->Class != nullptr && civilization->GetUnitClassNames(this->Class).size() > 0) {
				return civilization->GetUnitClassNames(this->Class);
			}
			
			if (this->UnitType == UnitTypeNaval) { // if is a ship
				if (faction && faction->GetShipNames().size() > 0) {
					return faction->GetShipNames();
				}
				
				if (civilization->GetShipNames().size() > 0) {
					return civilization->GetShipNames();
				}
			}
		}
	}
	
	return potential_names;
}
//Wyrmgus end


void CUnitType::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("get_unit_class"), [](const CUnitType *unit_type){ return unit_type->Class; });
	ClassDB::bind_method(D_METHOD("get_item_class"), [](const CUnitType *unit_type){ return const_cast<ItemClass *>(unit_type->ItemClass); });
	ClassDB::bind_method(D_METHOD("get_civilization"), &CUnitType::GetCivilization);
	ClassDB::bind_method(D_METHOD("get_faction"), &CUnitType::GetFaction);
	ClassDB::bind_method(D_METHOD("get_description"), &CUnitType::GetDescription);
	ClassDB::bind_method(D_METHOD("get_quote"), &CUnitType::GetQuote);
	ClassDB::bind_method(D_METHOD("get_background"), &CUnitType::GetBackground);
	ClassDB::bind_method(D_METHOD("is_hidden"), &CUnitType::IsHidden);
	ClassDB::bind_method(D_METHOD("is_hidden_in_editor"), &CUnitType::IsHiddenInEditor);
	ClassDB::bind_method(D_METHOD("get_icon"), &CUnitType::GetIcon);
	ClassDB::bind_method(D_METHOD("get_terrain_type"), [](const CUnitType *unit_type){ return unit_type->TerrainType; });
	
	ClassDB::bind_method(D_METHOD("get_copper_cost"), [](const CUnitType *unit_type){ return unit_type->DefaultStat.Costs[CopperCost]; });
	ClassDB::bind_method(D_METHOD("get_lumber_cost"), [](const CUnitType *unit_type){ return unit_type->DefaultStat.Costs[WoodCost]; });
	ClassDB::bind_method(D_METHOD("get_stone_cost"), [](const CUnitType *unit_type){ return unit_type->DefaultStat.Costs[StoneCost]; });
	ClassDB::bind_method(D_METHOD("get_coal_cost"), [](const CUnitType *unit_type){ return unit_type->DefaultStat.Costs[CoalCost]; });
	ClassDB::bind_method(D_METHOD("get_food_demand"), [](const CUnitType *unit_type){ return unit_type->DefaultStat.Variables[DEMAND_INDEX].Value; });
	
	ClassDB::bind_method(D_METHOD("get_hit_points"), [](const CUnitType *unit_type){ return unit_type->DefaultStat.Variables[HP_INDEX].Max; });
	ClassDB::bind_method(D_METHOD("get_mana"), [](const CUnitType *unit_type){ return unit_type->DefaultStat.Variables[MANA_INDEX].Max; });
	ClassDB::bind_method(D_METHOD("get_damage"), [](const CUnitType *unit_type){ return unit_type->DefaultStat.Variables[BASICDAMAGE_INDEX].Value; });
	ClassDB::bind_method(D_METHOD("get_acid_damage"), [](const CUnitType *unit_type){ return unit_type->DefaultStat.Variables[ACIDDAMAGE_INDEX].Value; });
	ClassDB::bind_method(D_METHOD("get_fire_damage"), [](const CUnitType *unit_type){ return unit_type->DefaultStat.Variables[FIREDAMAGE_INDEX].Value; });
	ClassDB::bind_method(D_METHOD("get_armor"), [](const CUnitType *unit_type){ return unit_type->DefaultStat.Variables[ARMOR_INDEX].Value; });
	ClassDB::bind_method(D_METHOD("get_fire_resistance"), [](const CUnitType *unit_type){ return unit_type->DefaultStat.Variables[FIRERESISTANCE_INDEX].Value; });
	ClassDB::bind_method(D_METHOD("get_range"), [](const CUnitType *unit_type){ return unit_type->DefaultStat.Variables[ATTACKRANGE_INDEX].Value; });
	ClassDB::bind_method(D_METHOD("get_sight"), [](const CUnitType *unit_type){ return unit_type->DefaultStat.Variables[SIGHTRANGE_INDEX].Value; });
	ClassDB::bind_method(D_METHOD("get_day_sight_bonus"), [](const CUnitType *unit_type){ return unit_type->DefaultStat.Variables[DAYSIGHTRANGEBONUS_INDEX].Value; });
	ClassDB::bind_method(D_METHOD("get_night_sight_bonus"), [](const CUnitType *unit_type){ return unit_type->DefaultStat.Variables[NIGHTSIGHTRANGEBONUS_INDEX].Value; });
	ClassDB::bind_method(D_METHOD("get_charge_bonus"), [](const CUnitType *unit_type){ return unit_type->DefaultStat.Variables[CHARGEBONUS_INDEX].Value; });
	ClassDB::bind_method(D_METHOD("get_backstab_bonus"), [](const CUnitType *unit_type){ return unit_type->DefaultStat.Variables[BACKSTAB_INDEX].Value; });
	ClassDB::bind_method(D_METHOD("get_accuracy"), [](const CUnitType *unit_type){ return unit_type->DefaultStat.Variables[ACCURACY_INDEX].Value; });
	ClassDB::bind_method(D_METHOD("get_evasion"), [](const CUnitType *unit_type){ return unit_type->DefaultStat.Variables[EVASION_INDEX].Value; });
	ClassDB::bind_method(D_METHOD("get_speed"), [](const CUnitType *unit_type){ return unit_type->DefaultStat.Variables[SPEED_INDEX].Value; });
	ClassDB::bind_method(D_METHOD("get_food_supply"), [](const CUnitType *unit_type){ return unit_type->DefaultStat.Variables[SUPPLY_INDEX].Value; });
	ClassDB::bind_method(D_METHOD("get_garrisoned_range_bonus"), [](const CUnitType *unit_type){ return unit_type->DefaultStat.Variables[GARRISONEDRANGEBONUS_INDEX].Value; });
	ClassDB::bind_method(D_METHOD("get_research_speed_bonus"), [](const CUnitType *unit_type){ return unit_type->DefaultStat.Variables[RESEARCHSPEEDBONUS_INDEX].Value; });
	ClassDB::bind_method(D_METHOD("get_ownership_influence_range"), [](const CUnitType *unit_type){ return unit_type->DefaultStat.Variables[OWNERSHIPINFLUENCERANGE_INDEX].Value; });
	ClassDB::bind_method(D_METHOD("get_speed_bonus"), [](const CUnitType *unit_type){ return unit_type->DefaultStat.Variables[SPEEDBONUS_INDEX].Value; });
	
	ClassDB::bind_method(D_METHOD("is_indestructible"), [](const CUnitType *unit_type){ return unit_type->BoolFlag[INDESTRUCTIBLE_INDEX].value; });
	ClassDB::bind_method(D_METHOD("is_unit"), [](const CUnitType *unit_type){ return unit_type->IsUnitUnitType(); });
	ClassDB::bind_method(D_METHOD("is_building"), [](const CUnitType *unit_type){ return unit_type->BoolFlag[BUILDING_INDEX].value; });
	ClassDB::bind_method(D_METHOD("is_item"), [](const CUnitType *unit_type){ return unit_type->BoolFlag[ITEM_INDEX].value; });
	ClassDB::bind_method(D_METHOD("can_attack"), [](const CUnitType *unit_type){ return unit_type->BoolFlag[CANATTACK_INDEX].value; });
	ClassDB::bind_method(D_METHOD("has_leadership_aura"), [](const CUnitType *unit_type){ return unit_type->DefaultStat.Variables[LEADERSHIPAURA_INDEX].Value; });
	ClassDB::bind_method(D_METHOD("has_regeneration_aura"), [](const CUnitType *unit_type){ return unit_type->DefaultStat.Variables[REGENERATIONAURA_INDEX].Value; });
}

void UpdateUnitStats(CUnitType &type, int reset)
{
	if (reset) {
		type.MapDefaultStat = type.DefaultStat;
		for (std::map<std::string, CUnitStats>::iterator iterator = type.ModDefaultStats.begin(); iterator != type.ModDefaultStats.end(); ++iterator) {
			for (size_t i = 0; i < UnitTypeVar.GetNumberVariable(); ++i) {
				type.MapDefaultStat.Variables[i].Value += iterator->second.Variables[i].Value;
				type.MapDefaultStat.Variables[i].Max += iterator->second.Variables[i].Max;
				type.MapDefaultStat.Variables[i].Increase += iterator->second.Variables[i].Increase;
				if (iterator->second.Variables[i].Enable != 0) {
					type.MapDefaultStat.Variables[i].Enable = iterator->second.Variables[i].Enable;
				}
			}
			for (std::map<CUnitType *, int>::const_iterator unit_stock_iterator = iterator->second.UnitStock.begin(); unit_stock_iterator != iterator->second.UnitStock.end(); ++unit_stock_iterator) {
				CUnitType *unit_type = unit_stock_iterator->first;
				int unit_stock = unit_stock_iterator->second;
				type.MapDefaultStat.ChangeUnitStock(unit_type, unit_stock);
			}
			for (int i = 0; i < MaxCosts; ++i) {
				type.MapDefaultStat.Costs[i] += iterator->second.Costs[i];
				type.MapDefaultStat.ImproveIncomes[i] += iterator->second.ImproveIncomes[i];
				type.MapDefaultStat.ResourceDemand[i] += iterator->second.ResourceDemand[i];
			}
		}
		for (int player = 0; player < PlayerMax; ++player) {
			type.Stats[player] = type.MapDefaultStat;
		}
		
		type.MapSound = type.Sound;
		for (std::map<std::string, CUnitSound>::iterator iterator = type.ModSounds.begin(); iterator != type.ModSounds.end(); ++iterator) {
			if (!iterator->second.Selected.Name.empty()) {
				type.MapSound.Selected = iterator->second.Selected;
			}
			if (!iterator->second.Acknowledgement.Name.empty()) {
				type.MapSound.Acknowledgement = iterator->second.Acknowledgement;
			}
			if (!iterator->second.Attack.Name.empty()) {
				type.MapSound.Attack = iterator->second.Attack;
			}
			if (!iterator->second.Idle.Name.empty()) {
				type.MapSound.Idle = iterator->second.Idle;
			}
			if (!iterator->second.Hit.Name.empty()) {
				type.MapSound.Hit = iterator->second.Hit;
			}
			if (!iterator->second.Miss.Name.empty()) {
				type.MapSound.Miss = iterator->second.Miss;
			}
			if (!iterator->second.FireMissile.Name.empty()) {
				type.MapSound.FireMissile = iterator->second.FireMissile;
			}
			if (!iterator->second.Step.Name.empty()) {
				type.MapSound.Step = iterator->second.Step;
			}
			if (!iterator->second.StepDirt.Name.empty()) {
				type.MapSound.StepDirt = iterator->second.StepDirt;
			}
			if (!iterator->second.StepGrass.Name.empty()) {
				type.MapSound.StepGrass = iterator->second.StepGrass;
			}
			if (!iterator->second.StepGravel.Name.empty()) {
				type.MapSound.StepGravel = iterator->second.StepGravel;
			}
			if (!iterator->second.StepMud.Name.empty()) {
				type.MapSound.StepMud = iterator->second.StepMud;
			}
			if (!iterator->second.StepStone.Name.empty()) {
				type.MapSound.StepStone = iterator->second.StepStone;
			}
			if (!iterator->second.Used.Name.empty()) {
				type.MapSound.Used = iterator->second.Used;
			}
			if (!iterator->second.Build.Name.empty()) {
				type.MapSound.Build = iterator->second.Build;
			}
			if (!iterator->second.Ready.Name.empty()) {
				type.MapSound.Ready = iterator->second.Ready;
			}
			if (!iterator->second.Repair.Name.empty()) {
				type.MapSound.Repair = iterator->second.Repair;
			}
			for (unsigned int j = 0; j < MaxCosts; ++j) {
				if (!iterator->second.Harvest[j].Name.empty()) {
					type.MapSound.Harvest[j] = iterator->second.Harvest[j];
				}
			}
			if (!iterator->second.Help.Name.empty()) {
				type.MapSound.Help = iterator->second.Help;
			}
			if (!iterator->second.Dead[ANIMATIONS_DEATHTYPES].Name.empty()) {
				type.MapSound.Dead[ANIMATIONS_DEATHTYPES] = iterator->second.Dead[ANIMATIONS_DEATHTYPES];
			}
			int death;
			for (death = 0; death < ANIMATIONS_DEATHTYPES; ++death) {
				if (!iterator->second.Dead[death].Name.empty()) {
					type.MapSound.Dead[death] = iterator->second.Dead[death];
				}
			}
		}
	}

	// Non-solid units can always be entered and they don't block anything
	if (type.BoolFlag[NONSOLID_INDEX].value) {
		if (type.BoolFlag[BUILDING_INDEX].value) {
			type.MovementMask = MapFieldLandUnit |
								MapFieldSeaUnit |
								MapFieldBuilding |
								//Wyrmgus start
								MapFieldItem |
								MapFieldBridge |
								//Wyrmgus end
								MapFieldCoastAllowed |
								MapFieldWaterAllowed |
								MapFieldNoBuilding |
								MapFieldUnpassable;
			type.FieldFlags = MapFieldNoBuilding;
		} else {
			type.MovementMask = 0;
			type.FieldFlags = 0;
		}
		return;
	}

	//  As side effect we calculate the movement flags/mask here.
	switch (type.UnitType) {
		case UnitTypeLand:                              // on land
			//Wyrmgus start
			/*
			type.MovementMask =
				MapFieldLandUnit |
				MapFieldSeaUnit |
				MapFieldBuilding | // already occuppied
				MapFieldCoastAllowed |
				MapFieldWaterAllowed | // can't move on this
				MapFieldUnpassable;
			*/
			if (type.BoolFlag[DIMINUTIVE_INDEX].value) { // diminutive units can enter tiles occupied by other units and vice-versa
				type.MovementMask =
					MapFieldBuilding | // already occuppied
					MapFieldCoastAllowed |
					MapFieldWaterAllowed | // can't move on this
					MapFieldUnpassable;
			} else {
				type.MovementMask =
					MapFieldLandUnit |
					MapFieldSeaUnit |
					MapFieldBuilding | // already occuppied
					MapFieldCoastAllowed |
					MapFieldWaterAllowed | // can't move on this
					MapFieldUnpassable;
			}
			
			if (type.BoolFlag[RAIL_INDEX].value) { //rail units can only move over railroads
				type.MovementMask |= MapFieldNoRail;
			}
			//Wyrmgus end
			break;
		case UnitTypeFly:                               // in air
			//Wyrmgus start
			/*
			type.MovementMask = MapFieldAirUnit; // already occuppied
				MapFieldAirUnit | // already occuppied
				MapFieldAirUnpassable;
			*/
			if (type.BoolFlag[DIMINUTIVE_INDEX].value) {
				type.MovementMask =
					MapFieldAirUnpassable;
			} else {
				type.MovementMask =
					MapFieldAirUnit | // already occuppied
					MapFieldAirUnpassable;
			}
			//Wyrmgus end
			break;
		//Wyrmgus start
		case UnitTypeFlyLow:                               // in low air
			if (type.BoolFlag[DIMINUTIVE_INDEX].value) {
				type.MovementMask =
					MapFieldBuilding |
					MapFieldUnpassable |
					MapFieldAirUnpassable;
			} else {
				type.MovementMask =
					MapFieldLandUnit |
					MapFieldSeaUnit |
					MapFieldBuilding |
					MapFieldUnpassable |
					MapFieldAirUnpassable;
			}
			break;
		case UnitTypeNaval:                             // on water
			//Wyrmgus start
			/*
			if (type.CanTransport()) {
				type.MovementMask =
					MapFieldLandUnit |
					MapFieldSeaUnit |
					MapFieldBuilding | // already occuppied
					//Wyrmgus start
					MapFieldBridge |
					//Wyrmgus end
					MapFieldLandAllowed; // can't move on this
				// Johns: MapFieldUnpassable only for land units?
			*/
			if (type.BoolFlag[CANDOCK_INDEX].value) {
				type.MovementMask =
					MapFieldLandUnit |
					MapFieldSeaUnit |
					MapFieldBuilding | // already occuppied
					//Wyrmgus start
					MapFieldBridge |
					//Wyrmgus end
					MapFieldLandAllowed | // can't move on this
					MapFieldUnpassable;
			} else if (type.BoolFlag[CANDOCK_INDEX].value && type.BoolFlag[DIMINUTIVE_INDEX].value) { //should add case for when is a transporter and is diminutive?
				type.MovementMask =
					MapFieldBuilding | // already occuppied
					MapFieldBridge |
					MapFieldLandAllowed | // can't move on this
					MapFieldUnpassable;
			} else if (type.BoolFlag[DIMINUTIVE_INDEX].value) { //should add case for when is a transporter and is diminutive?
				type.MovementMask =
					MapFieldBuilding | // already occuppied
					MapFieldBridge |
					MapFieldCoastAllowed |
					MapFieldLandAllowed | // can't move on this
					MapFieldUnpassable;
			//Wyrmgus end
			} else {
				type.MovementMask =
					MapFieldLandUnit |
					MapFieldSeaUnit |
					MapFieldBuilding | // already occuppied
					//Wyrmgus start
					MapFieldBridge |
					//Wyrmgus end
					MapFieldCoastAllowed |
					MapFieldLandAllowed | // can't move on this
					MapFieldUnpassable;
			}
			break;
		default:
			DebugPrint("Where moves this unit?\n");
			type.MovementMask = 0;
			break;
	}
	if (type.BoolFlag[BUILDING_INDEX].value || type.BoolFlag[SHOREBUILDING_INDEX].value) {
		// Shore building is something special.
		if (type.BoolFlag[SHOREBUILDING_INDEX].value) {
			type.MovementMask =
				MapFieldLandUnit |
				MapFieldSeaUnit |
				MapFieldBuilding | // already occuppied
				//Wyrmgus start
				MapFieldBridge |
				//Wyrmgus end
				MapFieldLandAllowed; // can't build on this
		}
		type.MovementMask |= MapFieldNoBuilding;
		//Wyrmgus start
		type.MovementMask |= MapFieldItem;
		if (type.TerrainType) {
			if ((type.TerrainType->Flags & MapFieldRailroad) || (type.TerrainType->Flags & MapFieldRoad)) {
				type.MovementMask |= MapFieldRailroad;
			}
			if (type.TerrainType->Flags & MapFieldRoad) {
				type.MovementMask |= MapFieldRoad;
			}
		}
		if (type.BoolFlag[AIRUNPASSABLE_INDEX].value) { // for air unpassable units (i.e. doors)
			type.FieldFlags |= MapFieldUnpassable;
			type.FieldFlags |= MapFieldAirUnpassable;
		}		
		//Wyrmgus end
		//
		// A little chaos, buildings without HP can be entered.
		// The oil-patch is a very special case.
		//
		if (type.MapDefaultStat.Variables[HP_INDEX].Max) {
			type.FieldFlags = MapFieldBuilding;
		} else {
			type.FieldFlags = MapFieldNoBuilding;
		}
	//Wyrmgus start
	} else if (type.BoolFlag[ITEM_INDEX].value || type.BoolFlag[POWERUP_INDEX].value || type.BoolFlag[TRAP_INDEX].value) {
		type.MovementMask = MapFieldLandUnit |
							MapFieldSeaUnit |
							MapFieldBuilding |
							MapFieldCoastAllowed |
							MapFieldWaterAllowed |
							MapFieldUnpassable |
							MapFieldItem;
		type.FieldFlags = MapFieldItem;
	} else if (type.BoolFlag[BRIDGE_INDEX].value) {
		type.MovementMask = MapFieldSeaUnit |
							MapFieldBuilding |
							MapFieldLandAllowed |
							MapFieldUnpassable |
							MapFieldBridge;
		type.FieldFlags = MapFieldBridge;
	//Wyrmgus end
	//Wyrmgus start
//	} else {
	} else if (!type.BoolFlag[DIMINUTIVE_INDEX].value) {
	//Wyrmgus end
		switch (type.UnitType) {
			case UnitTypeLand: // on land
				type.FieldFlags = MapFieldLandUnit;			
				//Wyrmgus start
				if (type.BoolFlag[AIRUNPASSABLE_INDEX].value) { // for air unpassable units (i.e. doors)
					type.FieldFlags |= MapFieldUnpassable;
					type.FieldFlags |= MapFieldAirUnpassable;
				}
				if (type.BoolFlag[GRAVEL_INDEX].value) {
					type.FieldFlags |= MapFieldGravel;
				}
				//Wyrmgus end
				break;
			case UnitTypeFly: // in air
				type.FieldFlags = MapFieldAirUnit;
				break;
			//Wyrmgus start
			case UnitTypeFlyLow: // in low air
				type.FieldFlags = MapFieldLandUnit;			
				if (type.BoolFlag[AIRUNPASSABLE_INDEX].value) { // for air unpassable units (i.e. doors)
					type.FieldFlags |= MapFieldUnpassable;
					type.FieldFlags |= MapFieldAirUnpassable;
				}
				break;
			//Wyrmgus end
			case UnitTypeNaval: // on water
				type.FieldFlags = MapFieldSeaUnit;
				//Wyrmgus start
				if (type.BoolFlag[AIRUNPASSABLE_INDEX].value) { // for air unpassable units (i.e. doors)
					type.FieldFlags |= MapFieldUnpassable;
					type.FieldFlags |= MapFieldAirUnpassable;
				}
				//Wyrmgus end
				break;
			default:
				DebugPrint("Where moves this unit?\n");
				type.FieldFlags = 0;
				break;
		}
	}
}


/**
**  Update the player stats for changed unit types.
**  @param reset indicates whether the default value should be set to each stat (level, upgrades)
*/
void UpdateStats(int reset)
{
	// Update players stats
	for (std::vector<CUnitType *>::size_type j = 0; j < CUnitType::UnitTypes.size(); ++j) {
		CUnitType &type = *CUnitType::UnitTypes[j];
		UpdateUnitStats(type, reset);
	}
}

/**
**  Save state of an unit-stats to file.
**
**  @param stats  Unit-stats to save.
**  @param ident  Unit-type ident.
**  @param plynr  Player number.
**  @param file   Output file.
*/
static bool SaveUnitStats(const CUnitStats &stats, const CUnitType &type, int plynr,
						  CFile &file)
{
	Assert(plynr < PlayerMax);

	if (stats == type.DefaultStat) {
		return false;
	}
	file.printf("DefineUnitStats(\"%s\", %d, {\n  ", type.Ident.c_str(), plynr);
	for (unsigned int i = 0; i < UnitTypeVar.GetNumberVariable(); ++i) {
		file.printf("\"%s\", {Value = %d, Max = %d, Increase = %d%s},\n  ",
					UnitTypeVar.VariableNameLookup[i], stats.Variables[i].Value,
					stats.Variables[i].Max, stats.Variables[i].Increase,
					stats.Variables[i].Enable ? ", Enable = true" : "");
	}
	file.printf("\"costs\", {");
	for (unsigned int i = 0; i < MaxCosts; ++i) {
		if (i) {
			file.printf(" ");
		}
		file.printf("\"%s\", %d,", DefaultResourceNames[i].c_str(), stats.Costs[i]);
	}
	file.printf("},\n\"storing\", {");
	for (unsigned int i = 0; i < MaxCosts; ++i) {
		if (i) {
			file.printf(" ");
		}
		file.printf("\"%s\", %d,", DefaultResourceNames[i].c_str(), stats.Storing[i]);
	}
	file.printf("},\n\"improve-production\", {");
	for (unsigned int i = 0; i < MaxCosts; ++i) {
		if (i) {
			file.printf(" ");
		}
		file.printf("\"%s\", %d,", DefaultResourceNames[i].c_str(), stats.ImproveIncomes[i]);
	}
	//Wyrmgus start
	file.printf("},\n\"resource-demand\", {");
	for (unsigned int i = 0; i < MaxCosts; ++i) {
		if (i) {
			file.printf(" ");
		}
		file.printf("\"%s\", %d,", DefaultResourceNames[i].c_str(), stats.ResourceDemand[i]);
	}
	file.printf("},\n\"unit-stock\", {");
	for (size_t i = 0; i < CUnitType::UnitTypes.size(); ++i) {
		if (stats.GetUnitStock(CUnitType::UnitTypes[i]) == type.DefaultStat.GetUnitStock(CUnitType::UnitTypes[i])) {
			continue;
		}
		if (i) {
			file.printf(" ");
		}
		file.printf("\"%s\", %d,", CUnitType::UnitTypes[i]->Ident.c_str(), stats.GetUnitStock(CUnitType::UnitTypes[i]));
	}
	//Wyrmgus end
	file.printf("}})\n");
	return true;
}

/**
**  Save state of the unit-type table to file.
**
**  @param file  Output file.
*/
void SaveUnitTypes(CFile &file)
{
	file.printf("\n--- -----------------------------------------\n");
	file.printf("--- MODULE: unittypes\n\n");

	// Save all stats
	for (std::vector<CUnitType *>::size_type i = 0; i < CUnitType::UnitTypes.size(); ++i) {
		const CUnitType &type = *CUnitType::UnitTypes[i];
		bool somethingSaved = false;

		for (int j = 0; j < PlayerMax; ++j) {
			if (CPlayer::Players[j]->Type != PlayerNobody) {
				somethingSaved |= SaveUnitStats(type.Stats[j], type, j, file);
			}
		}
		if (somethingSaved) {
			file.printf("\n");
		}
	}
}

/**
**  Find unit-type by identifier.
**
**  @param ident  The unit-type identifier.
**
**  @return       Unit-type pointer.
*/
CUnitType *UnitTypeByIdent(const std::string &ident)
{
	std::map<std::string, CUnitType *>::iterator ret = UnitTypeMap.find(ident);
	if (ret != UnitTypeMap.end()) {
		return (*ret).second;
	}
	//Wyrmgus start
//	fprintf(stderr, "Unit type \"%s\" does not exist.\n", ident.c_str());
	//Wyrmgus end
	return nullptr;
}

//Wyrmgus start
int GetUpgradeClassIndexByName(const std::string &class_name)
{
	if (UpgradeClassStringToIndex.find(class_name) != UpgradeClassStringToIndex.end()) {
		return UpgradeClassStringToIndex.find(class_name)->second;
	}
	return -1;
}

void SetUpgradeClassStringToIndex(const std::string &class_name, int class_id)
{
	UpgradeClassStringToIndex[class_name] = class_id;
}
//Wyrmgus end

/**
**  Allocate an empty unit-type slot.
**
**  @param ident  Identifier to identify the slot (malloced by caller!).
**
**  @return       New allocated (zeroed) unit-type pointer.
*/
CUnitType *NewUnitTypeSlot(const std::string &ident)
{
	size_t new_bool_size = UnitTypeVar.GetNumberBoolFlag();
	CUnitType *type = new CUnitType;

	if (!type) {
		fprintf(stderr, "Out of memory\n");
		ExitFatal(-1);
	}
	type->Slot = CUnitType::UnitTypes.size();
	type->Ident = ident;
	type->BoolFlag.resize(new_bool_size);

	type->DefaultStat.Variables = new CVariable[UnitTypeVar.GetNumberVariable()];
	for (unsigned int i = 0; i < UnitTypeVar.GetNumberVariable(); ++i) {
		type->DefaultStat.Variables[i] = UnitTypeVar.Variable[i];
	}
	CUnitType::UnitTypes.push_back(type);
	UnitTypeMap[type->Ident] = type;
	return type;
}

/**
**  Draw unit-type on map.
**
**  @param type    Unit-type pointer.
**  @param sprite  Sprite to use for drawing
**  @param player  Player number for color substitution.
**  @param frame   Animation frame of unit-type.
**  @param screenPos  Screen pixel (top left) position to draw unit-type.
**
**  @todo  Do screen position caculation in high level.
**         Better way to handle in x mirrored sprites.
*/
void DrawUnitType(const CUnitType &type, CPlayerColorGraphic *sprite, int player, int frame, const PixelPos &screenPos)
{
	//Wyrmgus start
	if (sprite == nullptr) {
		return;
	}
	//Wyrmgus end
	
	PixelPos pos = screenPos;
	// FIXME: move this calculation to high level.
	//Wyrmgus start
//	pos.x -= (type.Width - type.TileSize.x * CMap::Map.GetCurrentPixelTileSize().x) / 2;
//	pos.y -= (type.Height - type.TileSize.y * CMap::Map.GetCurrentPixelTileSize().y) / 2;
	pos.x -= (sprite->Width - type.TileSize.x * CMap::Map.GetCurrentPixelTileSize().x) / 2;
	pos.y -= (sprite->Height - type.TileSize.y * CMap::Map.GetCurrentPixelTileSize().y) / 2;
	//Wyrmgus end
	pos.x += type.OffsetX;
	pos.y += type.OffsetY;

	//Wyrmgus start
	/*
	if (type.Flip) {
		if (frame < 0) {
			sprite->DrawPlayerColorFrameClipX(player, -frame - 1, pos.x, pos.y);
		} else {
			sprite->DrawPlayerColorFrameClip(player, frame, pos.x, pos.y);
		}
	} else {
		const int row = type.NumDirections / 2 + 1;

		if (frame < 0) {
			frame = ((-frame - 1) / row) * type.NumDirections + type.NumDirections - (-frame - 1) % row;
		} else {
			frame = (frame / row) * type.NumDirections + frame % row;
		}
		sprite->DrawPlayerColorFrameClip(player, frame, pos.x, pos.y);
	}
	*/
	if (type.Flip) {
		if (frame < 0) {
			if (type.Stats[player].Variables[TRANSPARENCY_INDEX].Value > 0) {
				sprite->DrawPlayerColorFrameClipTransX(player, -frame - 1, pos.x, pos.y, int(256 - 2.56 * type.Stats[player].Variables[TRANSPARENCY_INDEX].Value), false);
			} else {
				sprite->DrawPlayerColorFrameClipX(player, -frame - 1, pos.x, pos.y, false);
			}
		} else {
			if (type.Stats[player].Variables[TRANSPARENCY_INDEX].Value > 0) {
				sprite->DrawPlayerColorFrameClipTrans(player, frame, pos.x, pos.y, int(256 - 2.56 * type.Stats[player].Variables[TRANSPARENCY_INDEX].Value), false);
			} else {
				sprite->DrawPlayerColorFrameClip(player, frame, pos.x, pos.y, false);
			}
		}
	} else {
		const int row = type.NumDirections / 2 + 1;

		if (frame < 0) {
			frame = ((-frame - 1) / row) * type.NumDirections + type.NumDirections - (-frame - 1) % row;
		} else {
			frame = (frame / row) * type.NumDirections + frame % row;
		}
		if (type.Stats[player].Variables[TRANSPARENCY_INDEX].Value > 0) {
			sprite->DrawPlayerColorFrameClipTrans(player, frame, pos.x, pos.y, int(256 - 2.56 * type.Stats[player].Variables[TRANSPARENCY_INDEX].Value), false);
		} else {
			sprite->DrawPlayerColorFrameClip(player, frame, pos.x, pos.y, false);
		}
	}
	//Wyrmgus end
}

/**
**  Get the still animation frame
*/
static int GetStillFrame(const CUnitType &type)
{
	//Wyrmgus start
	if (type.Animations == nullptr) {
		return 0;
	}
	//Wyrmgus end
	
	CAnimation *anim = type.Animations->Still;

	while (anim) {
		if (anim->Type == AnimationFrame) {
			CAnimation_Frame &a_frame = *static_cast<CAnimation_Frame *>(anim);
			// Use the frame facing down
			return a_frame.ParseAnimInt(nullptr) + type.NumDirections / 2;
		} else if (anim->Type == AnimationExactFrame) {
			CAnimation_ExactFrame &a_frame = *static_cast<CAnimation_ExactFrame *>(anim);

			return a_frame.ParseAnimInt(nullptr);
		}
		anim = anim->Next;
	}
	return type.NumDirections / 2;
}

/**
**  Init unit types.
*/
void InitUnitTypes(int reset_player_stats)
{
	for (size_t i = 0; i < CUnitType::UnitTypes.size(); ++i) {
		CUnitType &type = *CUnitType::UnitTypes[i];
		Assert(type.Slot == (int)i);

		if (type.Animations == nullptr) {
			DebugPrint(_("unit-type '%s' without animations, ignored.\n") _C_ type.Ident.c_str());
			continue;
		}
		//  Add idents to hash.
		UnitTypeMap[type.Ident] = CUnitType::UnitTypes[i];
		
		//Wyrmgus start
		/*
		// Determine still frame
		type.StillFrame = GetStillFrame(type);

		// Lookup BuildingTypes
		for (std::vector<CBuildRestriction *>::iterator b = type.BuildingRules.begin();
			 b < type.BuildingRules.end(); ++b) {
			(*b)->Init();
		}

		// Lookup AiBuildingTypes
		for (std::vector<CBuildRestriction *>::iterator b = type.AiBuildingRules.begin();
			 b < type.AiBuildingRules.end(); ++b) {
			(*b)->Init();
		}
		*/
		InitUnitType(type);
		//Wyrmgus end
	}

	// LUDO : called after game is loaded -> don't reset stats !
	UpdateStats(reset_player_stats); // Calculate the stats
}

//Wyrmgus start
void InitUnitType(CUnitType &type)
{
	// Determine still frame
	type.StillFrame = GetStillFrame(type);

	// Lookup BuildingTypes
	for (std::vector<CBuildRestriction *>::iterator b = type.BuildingRules.begin();
		 b < type.BuildingRules.end(); ++b) {
		(*b)->Init();
	}

	// Lookup AiBuildingTypes
	for (std::vector<CBuildRestriction *>::iterator b = type.AiBuildingRules.begin();
		 b < type.AiBuildingRules.end(); ++b) {
		(*b)->Init();
	}
}
//Wyrmgus end

/**
**  Loads the Sprite for a unit type
**
**  @param type  type of unit to load
*/
void LoadUnitTypeSprite(CUnitType &type)
{
	if (!type.ShadowFile.empty()) {
		type.ShadowSprite = CGraphic::ForceNew(type.ShadowFile, type.ShadowWidth, type.ShadowHeight);
		type.ShadowSprite->Load();
		if (type.Flip) {
			type.ShadowSprite->Flip();
		}
		if (type.ShadowSprite->Surface->format->BytesPerPixel == 1) {
			//Wyrmgus start
//			type.ShadowSprite->MakeShadow();
			//Wyrmgus end
		}
	}

	if (type.BoolFlag[HARVESTER_INDEX].value) {
		for (int i = 0; i < MaxCosts; ++i) {
			ResourceInfo *resinfo = type.ResInfo[i];
			if (!resinfo) {
				continue;
			}
			if (!resinfo->FileWhenLoaded.empty()) {
				resinfo->SpriteWhenLoaded = CPlayerColorGraphic::New(resinfo->FileWhenLoaded,
																	 type.Width, type.Height);
				resinfo->SpriteWhenLoaded->Load();
				if (type.Flip) {
					resinfo->SpriteWhenLoaded->Flip();
				}
			}
			if (!resinfo->FileWhenEmpty.empty()) {
				resinfo->SpriteWhenEmpty = CPlayerColorGraphic::New(resinfo->FileWhenEmpty,
																	type.Width, type.Height);
				resinfo->SpriteWhenEmpty->Load();
				if (type.Flip) {
					resinfo->SpriteWhenEmpty->Flip();
				}
			}
		}
	}

	if (!type.File.empty()) {
		type.Sprite = CPlayerColorGraphic::New(type.File, type.Width, type.Height);
		type.Sprite->Load();
		if (type.Flip) {
			type.Sprite->Flip();
		}
	}

	//Wyrmgus start
	if (!type.LightFile.empty()) {
		type.LightSprite = CGraphic::New(type.LightFile, type.Width, type.Height);
		type.LightSprite->Load();
		if (type.Flip) {
			type.LightSprite->Flip();
		}
	}
	for (int i = 0; i < MaxImageLayers; ++i) {
		if (!type.LayerFiles[i].empty()) {
			type.LayerSprites[i] = CPlayerColorGraphic::New(type.LayerFiles[i], type.Width, type.Height);
			type.LayerSprites[i]->Load();
			if (type.Flip) {
				type.LayerSprites[i]->Flip();
			}
		}
	}
	//Wyrmgus end

	//Wyrmgus start
	for (CUnitTypeVariation *variation : type.Variations) {
		int frame_width = type.Width;
		int frame_height = type.Height;
		if (variation->FrameWidth && variation->FrameHeight) {
			frame_width = variation->FrameWidth;
			frame_height = variation->FrameHeight;
		}
		if (!variation->File.empty()) {
			variation->Sprite = CPlayerColorGraphic::New(variation->File, frame_width, frame_height);
			variation->Sprite->Load();
			if (type.Flip) {
				variation->Sprite->Flip();
			}
		}
		if (!variation->ShadowFile.empty()) {
			variation->ShadowSprite = CGraphic::New(variation->ShadowFile, type.ShadowWidth, type.ShadowHeight);
			variation->ShadowSprite->Load();
			if (type.Flip) {
				variation->ShadowSprite->Flip();
			}
			if (variation->ShadowSprite->Surface->format->BytesPerPixel == 1) {
//				variation->ShadowSprite->MakeShadow();
			}
		}
		if (!variation->LightFile.empty()) {
			variation->LightSprite = CGraphic::New(variation->LightFile, frame_width, frame_height);
			variation->LightSprite->Load();
			if (type.Flip) {
				variation->LightSprite->Flip();
			}
		}
		for (int j = 0; j < MaxImageLayers; ++j) {
			if (!variation->LayerFiles[j].empty()) {
				variation->LayerSprites[j] = CPlayerColorGraphic::New(variation->LayerFiles[j], frame_width, frame_height);
				variation->LayerSprites[j]->Load();
				if (type.Flip) {
					variation->LayerSprites[j]->Flip();
				}
			}
		}
	
		for (int j = 0; j < MaxCosts; ++j) {
			if (!variation->FileWhenLoaded[j].empty()) {
				variation->SpriteWhenLoaded[j] = CPlayerColorGraphic::New(variation->FileWhenLoaded[j], frame_width, frame_height);
				variation->SpriteWhenLoaded[j]->Load();
				if (type.Flip) {
					variation->SpriteWhenLoaded[j]->Flip();
				}
			}
			if (!variation->FileWhenEmpty[j].empty()) {
				variation->SpriteWhenEmpty[j] = CPlayerColorGraphic::New(variation->FileWhenEmpty[j], frame_width, frame_height);
				variation->SpriteWhenEmpty[j]->Load();
				if (type.Flip) {
					variation->SpriteWhenEmpty[j]->Flip();
				}
			}
		}
	}
	
	for (int i = 0; i < MaxImageLayers; ++i) {
		for (CUnitTypeVariation *layer_variation : type.LayerVariations[i]) {
			if (!layer_variation->File.empty()) {
				layer_variation->Sprite = CPlayerColorGraphic::New(layer_variation->File, type.Width, type.Height);
				layer_variation->Sprite->Load();
				if (type.Flip) {
					layer_variation->Sprite->Flip();
				}
			}
		}
	}
	//Wyrmgus end
}


/**
** Return the amount of unit-types.
*/
int GetUnitTypesCount()
{
	int count = 0;
	for (std::vector<CUnitType *>::size_type i = 0; i < CUnitType::UnitTypes.size(); ++i) {
		CUnitType &type = *CUnitType::UnitTypes[i];

		if (type.Missile.IsEmpty() == false) count++;
		if (type.FireMissile.IsEmpty() == false) count++;
		if (type.Explosion.IsEmpty() == false) count++;


		if (!type.Sprite) {
			count++;
		}
	}
	return count;
}

/**
** Load the graphics for the unit-types.
*/
void LoadUnitTypes()
{
	for (std::vector<CUnitType *>::size_type i = 0; i < CUnitType::UnitTypes.size(); ++i) {
		CUnitType &type = *CUnitType::UnitTypes[i];

		ShowLoadProgress(_("Loading Unit Types (%d%%)"), (i + 1) * 100 / CUnitType::UnitTypes.size());
		LoadUnitType(type);
	}
}

//Wyrmgus start
void LoadUnitType(CUnitType &type)
{
	// Lookup icons.
	if (!type.Icon.Name.empty()) {
		type.Icon.Load();
	}

	for (CUnitTypeVariation *variation : type.Variations) {
		if (!variation->Icon.Name.empty()) {
			variation->Icon.Load();
		}
	}

	// Lookup missiles.
	type.Missile.MapMissile();
	//Wyrmgus start
	type.FireMissile.MapMissile();
	//Wyrmgus end
	type.Explosion.MapMissile();

	// Lookup impacts
	for (int i = 0; i < ANIMATIONS_DEATHTYPES + 2; ++i) {
		type.Impact[i].MapMissile();
	}
	// Lookup corpse.
	if (!type.CorpseName.empty()) {
		type.CorpseType = UnitTypeByIdent(type.CorpseName);
	}
#ifndef DYNAMIC_LOAD
	// Load Sprite
	if (!type.Sprite) {
		LoadUnitTypeSprite(type);

		IncItemsLoaded();
	}
#endif
	// FIXME: should i copy the animations of same graphics?
}
//Wyrmgus end

void CUnitTypeVar::Init()
{
	// Variables.
	Variable.resize(GetNumberVariable());
	size_t new_size = UnitTypeVar.GetNumberBoolFlag();
	for (unsigned int i = 0; i < CUnitType::UnitTypes.size(); ++i) { // adjust array for unit already defined
		CUnitType::UnitTypes[i]->BoolFlag.resize(new_size);
	}
}

void CUnitTypeVar::Clear()
{
	Variable.clear();

	for (std::vector<CDecoVar *>::iterator it = DecoVar.begin();
		 it != DecoVar.end(); ++it) {
		delete(*it);
	}
	DecoVar.clear();
}

/**
**  Cleanup the unit-type module.
*/
void CleanUnitTypes()
{
	DebugPrint("FIXME: icon, sounds not freed.\n");
	FreeAnimations();

	// Clean all unit-types
	for (size_t i = 0; i < CUnitType::UnitTypes.size(); ++i) {
		delete CUnitType::UnitTypes[i];
	}
	CUnitType::UnitTypes.clear();
	UnitTypeMap.clear();
	UnitTypeVar.Clear();
}

//Wyrmgus start
std::string GetUnitTypeStatsString(const std::string &unit_type_ident)
{
	const CUnitType *unit_type = UnitTypeByIdent(unit_type_ident);

	if (unit_type) {
		std::string unit_type_stats_string;

		bool first_var = true;
		for (size_t var = 0; var < UnitTypeVar.GetNumberVariable(); ++var) {
			if (
				!(var == BASICDAMAGE_INDEX || var == PIERCINGDAMAGE_INDEX || var == THORNSDAMAGE_INDEX
				|| var == FIREDAMAGE_INDEX || var == COLDDAMAGE_INDEX || var == ARCANEDAMAGE_INDEX || var == LIGHTNINGDAMAGE_INDEX
				|| var == AIRDAMAGE_INDEX || var == EARTHDAMAGE_INDEX || var == WATERDAMAGE_INDEX || var == ACIDDAMAGE_INDEX
				|| var == ARMOR_INDEX || var == FIRERESISTANCE_INDEX || var == COLDRESISTANCE_INDEX || var == ARCANERESISTANCE_INDEX || var == LIGHTNINGRESISTANCE_INDEX
				|| var == AIRRESISTANCE_INDEX || var == EARTHRESISTANCE_INDEX || var == WATERRESISTANCE_INDEX || var == ACIDRESISTANCE_INDEX
				|| var == HACKRESISTANCE_INDEX || var == PIERCERESISTANCE_INDEX || var == BLUNTRESISTANCE_INDEX
				|| var == ACCURACY_INDEX || var == EVASION_INDEX || var == SPEED_INDEX || var == CHARGEBONUS_INDEX || var == BACKSTAB_INDEX
				|| var == HITPOINTHEALING_INDEX || var == HITPOINTBONUS_INDEX
				|| var == SIGHTRANGE_INDEX || var == DAYSIGHTRANGEBONUS_INDEX || var == NIGHTSIGHTRANGEBONUS_INDEX
				|| var == HP_INDEX || var == MANA_INDEX || var == OWNERSHIPINFLUENCERANGE_INDEX || var == LEADERSHIPAURA_INDEX || var == REGENERATIONAURA_INDEX || var == HYDRATINGAURA_INDEX || var == ETHEREALVISION_INDEX || var == SPEEDBONUS_INDEX || var == SUPPLY_INDEX || var == TIMEEFFICIENCYBONUS_INDEX || var == RESEARCHSPEEDBONUS_INDEX || var == GARRISONEDRANGEBONUS_INDEX)
			) {
				continue;
			}

			if (unit_type->DefaultStat.Variables[var].Enable) {
				if (!first_var) {
					unit_type_stats_string += ", ";
				} else {
					first_var = false;
				}

				if (IsBooleanVariable(var) && unit_type->DefaultStat.Variables[var].Value < 0) {
					unit_type_stats_string += "Lose ";
				}

				if (!IsBooleanVariable(var)) {
					unit_type_stats_string += std::to_string((long long) unit_type->DefaultStat.Variables[var].Value);
					if (IsPercentageVariable(var)) {
						unit_type_stats_string += "%";
					}
					unit_type_stats_string += " ";
				}

				unit_type_stats_string += GetVariableDisplayName(var);
			}
		}
			
		return unit_type_stats_string;
	}
	
	return "";
}

std::string GetImageLayerNameById(int image_layer)
{
	if (image_layer == LeftArmImageLayer) {
		return "left-arm";
	} else if (image_layer == RightArmImageLayer) {
		return "right-arm";
	} else if (image_layer == RightHandImageLayer) {
		return "right-hand";
	} else if (image_layer == HairImageLayer) {
		return "hair";
	} else if (image_layer == ClothingImageLayer) {
		return "clothing";
	} else if (image_layer == ClothingLeftArmImageLayer) {
		return "clothing-left-arm";
	} else if (image_layer == ClothingRightArmImageLayer) {
		return "clothing-right-arm";
	} else if (image_layer == PantsImageLayer) {
		return "pants";
	} else if (image_layer == BootsImageLayer) {
		return "boots";
	} else if (image_layer == WeaponImageLayer) {
		return "weapon";
	} else if (image_layer == ShieldImageLayer) {
		return "shield";
	} else if (image_layer == HelmetImageLayer) {
		return "helmet";
	} else if (image_layer == BackpackImageLayer) {
		return "backpack";
	} else if (image_layer == MountImageLayer) {
		return "mount";
	}

	return "";
}

int GetImageLayerIdByName(const std::string &image_layer)
{
	if (image_layer == "left-arm") {
		return LeftArmImageLayer;
	} else if (image_layer == "right-arm") {
		return RightArmImageLayer;
	} else if (image_layer == "right-hand") {
		return RightHandImageLayer;
	} else if (image_layer == "hair") {
		return HairImageLayer;
	} else if (image_layer == "clothing") {
		return ClothingImageLayer;
	} else if (image_layer == "clothing-left-arm") {
		return ClothingLeftArmImageLayer;
	} else if (image_layer == "clothing-right-arm") {
		return ClothingRightArmImageLayer;
	} else if (image_layer == "pants") {
		return PantsImageLayer;
	} else if (image_layer == "boots") {
		return BootsImageLayer;
	} else if (image_layer == "weapon") {
		return WeaponImageLayer;
	} else if (image_layer == "shield") {
		return ShieldImageLayer;
	} else if (image_layer == "helmet") {
		return HelmetImageLayer;
	} else if (image_layer == "backpack") {
		return BackpackImageLayer;
	} else if (image_layer == "mount") {
		return MountImageLayer;
	}

	return -1;
}
//Wyrmgus end
