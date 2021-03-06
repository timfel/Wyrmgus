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
//      (c) Copyright 1998-2020 by Lutz Sammer, Jimmy Salmon and Andrettin
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

#include "unit/unit_type.h"

#include "ai/ai_local.h" //for using AiHelpers
#include "animation.h"
#include "animation/animation_exactframe.h"
#include "animation/animation_frame.h"
#include "civilization.h"
#include "config.h"
#include "database/defines.h"
#include "editor.h" //for personal name generation
#include "faction.h"
#include "iolib.h"
#include "item/item_class.h"
#include "item/item_slot.h"
#include "luacallback.h"
#include "map/map.h"
#include "map/terrain_type.h"
#include "map/tileset.h"
#include "missile.h"
#include "mod.h"
#include "player.h"
#include "script.h"
#include "script/condition/and_condition.h"
#include "sound/sound.h"
#include "sound/unitsound.h"
#include "species/species.h"
#include "spell/spell.h"
#include "translate.h"
#include "ui/button.h"
#include "ui/button_level.h"
#include "ui/ui.h"
#include "unit/unit_class.h"
#include "unit/unit_type_type.h"
#include "unit/unit_type_variation.h"
#include "upgrade/upgrade.h"
#include "util/size_util.h"
#include "util/string_util.h"
#include "util/util.h"
#include "video/video.h"

/**
**  @class unit_type unit_type.h
**
**  \#include "unit/unit_type.h"
**
**  This class contains the information that is shared between all
**  units of the same type and determins if a unit is a building,
**  a person, ...
**
**  The unit-type class members:
**
**  unit_type::File
**
**    Path file name of the sprite file.
**
**  unit_type::ShadowFile
**
**    Path file name of shadow sprite file.
**
**  unit_type::DrawLevel
**
**    The Level/Order to draw this type of unit in. 0-255 usually.
**
**  unit_type::Width unit_type::Height
**
**    Size of a sprite frame in pixels. All frames of a sprite have
**    the same size. Also all sprites (tilesets) must have the same
**    size.
**
**  unit_type::ShadowWidth unit_type::ShadowHeight
**
**    Size of a shadow sprite frame in pixels. All frames of a sprite
**    have the same size. Also all sprites (tilesets) must have the
**    same size.
**
**  unit_type::ShadowOffsetX unit_type::ShadowOffsetY
**
**    Vertical offset to draw the shadow in pixels.
**
**  unit_type::animation_set
**
**    Animation scripts for the different actions. Currently the
**    animations still, move, attack and die are supported.
**  @see animation_set
**  @see CAnimation
**
**  unit_type::Icon
**
**    Icon to display for this unit-type. Contains configuration and
**    run time variable.
**  @note This icon can be used for training, but isn't used.
**
**  unit_type::Missile
**
**    Configuration and run time variable of the missile weapon.
**  @note It is planned to support more than one weapons.
**  And the sound of the missile should be used as fire sound.
**
**  unit_type::Explosion
**
**    Configuration and run time variable of the missile explosion.
**    This is the explosion that happens if unit is set to
**    ExplodeWhenKilled
**
**  unit_type::CorpseName
**
**    Corpse unit-type name, should only be used during setup.
**
**  unit_type::CorpseType
**
**    Corpse unit-type pointer, only this should be used during run
**    time. Many unit-types can share the same corpse.
**
**
**  @todo continue this documentation
**
**  unit_type::Construction
**
**    What is shown in construction phase.
**
**  unit_type::SightRange
**
**    Sight range
**
**  unit_type::_HitPoints
**
**    Maximum hit points
**
**
**  unit_type::_Costs[::MaxCosts]
**
**    How many resources needed
**
**  unit_type::RepairHP
**
**    The HP given to a unit each cycle it's repaired.
**    If zero, unit cannot be repaired
**
**    unit_type::RepairCosts[::MaxCosts]
**
**    Costs per repair cycle to fix a unit.
**
**  unit_type::TileSize
**
**    Tile size on map
**
**  unit_type::box_size
**
**    Selected box size
**
**  unit_type::NumDirections
**
**    Number of directions the unit can face
**
**  unit_type::MinAttackRange
**
**    Minimal attack range
**
**  unit_type::ReactRangeComputer
**
**    Reacts on enemy for computer
**
**  unit_type::ReactRangePerson
**
**    Reacts on enemy for person player
**
**  unit_type::Priority
**
**    Priority value / AI Treatment
**
**  unit_type::BurnPercent
**
**    The burning limit in percents. If the unit has lees than
**    this it will start to burn.
**
**  unit_type::BurnDamageRate
**
**    Burn rate in HP per second
**
**  unit_type::UnitType
**
**    Land / fly / naval
**
**  @note original only visual effect, we do more with this!
**
**  unit_type::DecayRate
**
**    Decay rate in 1/6 seconds
**
**  unit_type::AnnoyComputerFactor
**
**    How much this annoys the computer
**
**  @todo not used
**
**  unit_type::MouseAction
**
**    Right click action
**
**  unit_type::Points
**
**    How many points you get for unit. Used in the final score table.
**
**  unit_type::CanTarget
**
**    Which units can it attack
**
**  unit_type::LandUnit
**
**    Land animated
**
**  unit_type::AirUnit
**
**    Air animated
**
**  unit_type::SeaUnit
**
**    Sea animated
**
**  unit_type::ExplodeWhenKilled
**
**    Death explosion animated
**
**  unit_type::RandomMovementProbability
**
**    When the unit is idle this is the probability that it will
**    take a step in a random direction, in percents.
**
**  unit_type::ClicksToExplode
**
**    If this is non-zero, then after that many clicks the unit will
**    commit suicide. Doesn't work with resource workers/resources.
**
**  unit_type::Building
**
**    Unit is a Building
**
**  unit_type::Transporter
**
**    Can transport units
**
**  unit_type::MaxOnBoard
**
**    Maximum units on board (for transporters), and resources
**
**  unit_type::StartingResources
**    Amount of Resources a unit has when It's Built
**
**  unit_type::DamageType
**    Unit's missile damage type (used for extra death animations)
**
**  unit_type::GivesResource
**
**    This equals to the resource Id of the resource given
**    or 0 (TimeCost) for other buildings.
**
**  unit_type::ResInfo[::MaxCosts]
**
**    Information about resource harvesting. If null, it can't
**    harvest it.
**
**  unit_type::NeutralMinimapColorRGB
**
**    Says what color a unit will have when it's neutral and
**    is displayed on the minimap.
**
**  unit_type::CanStore[::MaxCosts]
**
**    What resource types we can store here.
**
**  unit_type::Spells
**
**    Spells the unit is able to use
**
**  unit_type::CanAttack
**
**    Unit is able to attack.
**
**  unit_type::RepairRange
**
**    Unit can repair buildings. It will use the actack animation.
**    It will heal 4 points for every repair cycle, and cost 1 of
**    each resource, alternatively(1 cycle wood, 1 cycle gold)
**  @todo The above should be more configurable.
**    If units have a repair range, they can repair, and this is the
**    distance.
**
**    unit_type::ShieldPiercing
**
**    Can directly damage shield-protected units, without shield damaging.
**
**  unit_type::Sound
**
**    Sounds for events
**
**  unit_type::Weapon
**
**    Current sound for weapon
**
**  @todo temporary solution
**
**  unit_type::FieldFlags
**
**    Flags that are set, if a unit enters a map field or cleared, if
**    a unit leaves a map field.
**
**  unit_type::MovementMask
**
**    Movement mask, this value is and'ed to the map field flags, to
**    see if a unit can enter or placed on the map field.
**
**  unit_type::Stats[::PlayerMax]
**
**    Unit status for each player
**  @todo This stats should? be moved into the player struct
**
**  unit_type::Type
**
**    Type as number
**  @todo Should us a general name f.e. Slot here?
**
**  unit_type::Sprite
**
**    Sprite images
**
**  unit_type::ShadowSprite
**
**    Shadow sprite images
**
**  unit_type::PlayerColorSprite
**
**    Sprite images of the player colors.  This image is drawn
**    over unit_type::Sprite.  Used with OpenGL only.
**
**
*/
/**
**
**  @class resource_info unit_type.h
**
** \#include "unit/unit_type.h"
**
**    This class contains information about how a unit will harvest a resource.
**
**  resource_info::FileWhenLoaded
**
**    The harvester's animation file will change when it's loaded.
**
**  resource_info::FileWhenEmpty;
**
**    The harvester's animation file will change when it's empty.
**    The standard animation is used only when building/repairing.
**
**
**  resource_info::HarvestFromOutside
**
**    Unit will harvest from the outside. The unit will use it's
**    Attack animation (seems it turned into a generic Action anim.)
**
**  resource_info::ResourceId
**
**    The resource this is for. Mostly redundant.
**
**  resource_info::final_resource
**
**    The resource is converted to this at the depot. Useful for
**    a fisherman who harvests fish, but it all turns to food at the
**    depot.
**
**  resource_info::final_resource_conversion_rate
**
**    The rate at which the resource is converted to the final resource at the depot. Useful for
**    silver mines returning a lower amount of gold.
**
**  resource_info::WaitAtResource
**
**    Cycles the unit waits while inside a resource.
**
**  resource_info::ResourceStep
**
**    The unit makes so-caled mining cycles. Each mining cycle
**    it does some sort of animation and gains ResourceStep
**    resources. You can stop after any number of steps.
**    when the quantity in the harvester reaches the maximum
**    (ResourceCapacity) it will return home. I this is 0 then
**    it's considered infinity, and ResourceCapacity will now
**    be the limit.
**
**  resource_info::ResourceCapacity
**
**    Maximum amount of resources a harvester can carry. The
**    actual amount can be modified while unloading.
**
**  resource_info::LoseResources
**
**    Special lossy behaviour for loaded harvesters. Harvesters
**    with loads other than 0 and ResourceCapacity will lose their
**    cargo on any new order.
**
**  resource_info::WaitAtDepot
**
**    Cycles the unit waits while inside the depot to unload.
**
**  resource_info::TerrainHarvester
**
**    The unit will harvest terrain. For now this only works
**    for wood. maybe it could be made to work for rocks, but
**    more than that requires a tileset rewrite.
**  @todo more configurable.
**
*/

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
wyrmgus::unit_type *settlement_site_unit_type;
//Wyrmgus end

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

namespace wyrmgus {

unit_type::unit_type(const std::string &identifier) : detailed_data_entry(identifier), CDataType(identifier),
	Slot(0),
	ShadowWidth(0), ShadowHeight(0), ShadowOffsetX(0), ShadowOffsetY(0),
	//Wyrmgus start
	TrainQuantity(0), CostModifier(0), item_class(item_class::none),
	Faction(-1), TerrainType(nullptr),
	//Wyrmgus end
	StillFrame(0),
	TeleportCost(0),
	RepairHP(0),
	BoxOffsetX(0), BoxOffsetY(0), NumDirections(0),
	MinAttackRange(0),
	BurnPercent(0), BurnDamageRate(0), RepairRange(0),
	AutoBuildRate(0), RandomMovementProbability(0), RandomMovementDistance(1), ClicksToExplode(0),
	//Wyrmgus start
//	MaxOnBoard(0), BoardSize(1), ButtonLevelForTransporter(0), StartingResources(0),
	MaxOnBoard(0), BoardSize(1), ButtonLevelForTransporter(nullptr), ButtonPos(0), ButtonLevel(0),
	//Wyrmgus end
	UnitType(UnitTypeType::Land), DecayRate(0), AnnoyComputerFactor(0), AiAdjacentRange(-1),
	MouseAction(0), CanTarget(0),
	Flip(1), LandUnit(0), AirUnit(0), SeaUnit(0),
	ExplodeWhenKilled(0),
	CanAttack(0),
	Neutral(0),
	PoisonDrain(0), FieldFlags(0), MovementMask(0),
	//Wyrmgus start
	Elixir(nullptr),
//	Sprite(nullptr), ShadowSprite(nullptr)
	Sprite(nullptr), ShadowSprite(nullptr), LightSprite(nullptr)
	//Wyrmgus end
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

	this->BoolFlag.resize(UnitTypeVar.GetNumberBoolFlag());

	this->DefaultStat.Variables.resize(UnitTypeVar.GetNumberVariable());
	for (unsigned int i = 0; i < UnitTypeVar.GetNumberVariable(); ++i) {
		this->DefaultStat.Variables[i] = UnitTypeVar.Variable[i];
	}
}

unit_type::~unit_type()
{
}

void unit_type::process_sml_property(const sml_property &property)
{
	const std::string &key = property.get_key();
	const std::string &value = property.get_value();

	if (key == "parent") {
		const unit_type *parent_type = unit_type::get(value);
		this->set_parent(parent_type);
	} else if (key == "icon") {
		this->Icon.Name = value;
		this->Icon.Icon = nullptr;
		this->Icon.Load();
	} else if (key == "missile") {
		this->Missile.Name = value;
		this->Missile.Missile = nullptr;
	} else if (key == "fire_missile") {
		this->FireMissile.Name = value;
		this->FireMissile.Missile = nullptr;
	} else if (key == "priority") {
		this->DefaultStat.Variables[PRIORITY_INDEX].Value = std::stoi(value);
		this->DefaultStat.Variables[PRIORITY_INDEX].Max = std::stoi(value);
	} else if (key == "button_key") {
		this->button_key = value;
	} else if (key == "button_hint") {
		this->ButtonHint = value;
	} else if (key == "type") {
		if (value == "land") {
			this->UnitType = UnitTypeType::Land;
			this->LandUnit = true;
		} else if (value == "fly") {
			this->UnitType = UnitTypeType::Fly;
			this->AirUnit = true;
		} else if (value == "fly_low") {
			this->UnitType = UnitTypeType::FlyLow;
			this->AirUnit = true;
		} else if (value == "naval") {
			this->UnitType = UnitTypeType::Naval;
			this->SeaUnit = true;
		} else if (value == "space") {
			this->UnitType = UnitTypeType::Space;
			this->AirUnit = true;
		} else {
			throw std::runtime_error("Invalid unit type type: \"" + value + "\"");
		}
	} else if (key == "max_attack_range") {
		this->DefaultStat.Variables[ATTACKRANGE_INDEX].Value = std::stoi(value);
		this->DefaultStat.Variables[ATTACKRANGE_INDEX].Max = std::stoi(value);
		this->DefaultStat.Variables[ATTACKRANGE_INDEX].Enable = 1;
	} else if (key == "max_on_board") {
		this->MaxOnBoard = std::stoi(value);
		this->DefaultStat.Variables[TRANSPORT_INDEX].Max = this->MaxOnBoard;
		this->DefaultStat.Variables[TRANSPORT_INDEX].Enable = 1;
	} else if (key == "annoy_computer_factor") {
		this->AnnoyComputerFactor = std::stoi(value);
	} else if (key == "can_target_land") {
		const bool can_target_land = string::to_bool(value);
		if (can_target_land) {
			this->CanTarget |= CanTargetLand;
		} else {
			this->CanTarget &= ~CanTargetLand;
		}
	} else if (key == "can_target_sea") {
		const bool can_target_sea = string::to_bool(value);
		if (can_target_sea) {
			this->CanTarget |= CanTargetSea;
		} else {
			this->CanTarget &= ~CanTargetSea;
		}
	} else if (key == "can_target_air") {
		const bool can_target_air = string::to_bool(value);
		if (can_target_air) {
			this->CanTarget |= CanTargetAir;
		} else {
			this->CanTarget &= ~CanTargetAir;
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
			throw std::runtime_error("Invalid right mouse action: \"" + value + "\".");
		}
	} else if (key == "requirements_string") {
		this->RequirementsString = value;
	} else {
		const std::string pascal_case_key = string::snake_case_to_pascal_case(key);

		int index = UnitTypeVar.VariableNameLookup[pascal_case_key.c_str()]; // variable index
		if (index != -1) { // valid index
			if (string::is_number(value)) {
				this->DefaultStat.Variables[index].Enable = 1;
				this->DefaultStat.Variables[index].Value = std::stoi(value);
				this->DefaultStat.Variables[index].Max = std::stoi(value);
			} else if (IsStringBool(value)) {
				this->DefaultStat.Variables[index].Enable = string::to_bool(value);
			} else { // error
				fprintf(stderr, "Invalid value (\"%s\") for variable \"%s\" when defining unit type \"%s\".\n", value.c_str(), key.c_str(), this->get_identifier().c_str());
			}

			return;
		}

		index = UnitTypeVar.BoolFlagNameLookup[pascal_case_key.c_str()];
		if (index != -1) {
			if (this->BoolFlag.size() < UnitTypeVar.GetNumberBoolFlag()) {
				this->BoolFlag.resize(UnitTypeVar.GetNumberBoolFlag());
			}

			if (string::is_number(value)) {
				this->BoolFlag[index].value = (std::stoi(value) != 0);
			} else {
				this->BoolFlag[index].value = string::to_bool(value);
			}

			return;
		}

		data_entry::process_sml_property(property);
	}
}

void unit_type::process_sml_scope(const sml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const std::vector<std::string> &values = scope.get_values();

	if (tag == "costs") {
		scope.for_each_property([&](const wyrmgus::sml_property &property) {
			const std::string &key = property.get_key();
			const std::string &value = property.get_value();

			const wyrmgus::resource *resource = resource::get(key);
			this->DefaultStat.Costs[resource->get_index()] = std::stoi(value);
		});
	} else if (tag == "repair_costs") {
		scope.for_each_property([&](const wyrmgus::sml_property &property) {
			const std::string &key = property.get_key();
			const std::string &value = property.get_value();

			const wyrmgus::resource *resource = resource::get(key);
			this->RepairCosts[resource->get_index()] = std::stoi(value);
		});
	} else if (tag == "weapon_classes") {
		for (const std::string &value : values) {
			this->WeaponClasses.push_back(string_to_item_class(value));
		}
	} else if (tag == "ai_drops") {
		for (const std::string &value : values) {
			this->AiDrops.push_back(unit_type::get(value));
		}
	} else if (tag == "default_equipment") {
		scope.for_each_property([&](const sml_property &property) {
			const std::string &key = property.get_key();
			const std::string &value = property.get_value();

			const item_slot item_slot = string_to_item_slot(key);
			unit_type *item = unit_type::get(value);
			this->DefaultEquipment[item_slot] = item;
		});
	} else if (tag == "can_transport") {
		if (this->MaxOnBoard == 0) { // set default value.
			this->MaxOnBoard = 1;
			this->DefaultStat.Variables[TRANSPORT_INDEX].Max = this->MaxOnBoard;
			this->DefaultStat.Variables[TRANSPORT_INDEX].Enable = 1;
		}

		if (this->BoolFlag.size() < UnitTypeVar.GetNumberBoolFlag()) {
			this->BoolFlag.resize(UnitTypeVar.GetNumberBoolFlag());
		}

		scope.for_each_property([&](const sml_property &property) {
			const std::string &key = property.get_key();
			const std::string &value = property.get_value();

			const std::string pascal_case_key = string::snake_case_to_pascal_case(key);
			const int index = UnitTypeVar.BoolFlagNameLookup[pascal_case_key.c_str()];
			if (index != -1) {
				this->BoolFlag[index].CanTransport = StringToCondition(value);
			} else {
				throw std::runtime_error("Unsupported flag tag for CanTransport: " + key);
			}
		});
	} else if (tag == "starting_abilities") {
		for (const std::string &value : values) {
			this->StartingAbilities.push_back(CUpgrade::get(value));
		}
	} else if (tag == "spells") {
		for (const std::string &value : values) {
			this->Spells.push_back(spell::get(value));
		}
	} else if (tag == "autocast_spells") {
		for (const std::string &value : values) {
			const spell *spell = spell::get(value);
			this->add_autocast_spell(spell);
		}
	} else if (tag == "affixes") {
		for (const std::string &value : values) {
			this->Affixes.push_back(CUpgrade::get(value));
		}
	} else if (tag == "resource_gathering") {
		scope.for_each_child([&](const sml_data &child_scope) {
			const std::string &tag = child_scope.get_tag();
			const resource *resource = resource::get(tag);

			if (this->ResInfo[resource->get_index()] == nullptr) {
				auto resource_info = std::make_unique<wyrmgus::resource_info>(this, resource);
				this->ResInfo[resource->get_index()] = std::move(resource_info);
			}

			resource_info *res_info_ptr = this->ResInfo[resource->get_index()].get();
			database::process_sml_data(res_info_ptr, child_scope);
		});
	} else if (tag == "variations") {
		this->DefaultStat.Variables[VARIATION_INDEX].Enable = 1;
		this->DefaultStat.Variables[VARIATION_INDEX].Value = 0;

		scope.for_each_child([&](const sml_data &child_scope) {
			const std::string &tag = child_scope.get_tag();
			auto variation = std::make_unique<unit_type_variation>(tag, this);

			database::process_sml_data(variation, child_scope);

			this->variations.push_back(std::move(variation));
		});

		this->DefaultStat.Variables[VARIATION_INDEX].Max = static_cast<int>(this->variations.size());
	} else if (tag == "button_icons") {
		this->ButtonIcons.clear();

		scope.for_each_property([&](const sml_property &property) {
			const std::string &key = property.get_key();
			const std::string &value = property.get_value();

			const ButtonCmd button_action = GetButtonActionIdByName(key);
			if (button_action != ButtonCmd::None) {
				this->ButtonIcons[button_action].Name = value;
				this->ButtonIcons[button_action].Icon = nullptr;
				this->ButtonIcons[button_action].Load();
			} else {
				throw std::runtime_error("Button action \"" + key + "\" doesn't exist.");
			}
		});
	} else if (tag == "sounds") {
		database::process_sml_data(this->Sound, scope);
	} else if (tag == "preconditions") {
		this->preconditions = std::make_unique<and_condition>();
		database::process_sml_data(this->preconditions, scope);
	} else if (tag == "conditions") {
		this->conditions = std::make_unique<and_condition>();
		database::process_sml_data(this->conditions, scope);
	} else {
		const std::string pascal_case_tag = string::snake_case_to_pascal_case(tag);

		const int index = UnitTypeVar.VariableNameLookup[pascal_case_tag.c_str()]; // variable index

		if (index != -1) { // valid index
			scope.for_each_property([&](const sml_property &property) {
				const std::string &key = property.get_key();
				const std::string &value = property.get_value();

				if (key == "enable") {
					this->DefaultStat.Variables[index].Enable = string::to_bool(value);
				} else if (key == "value") {
					this->DefaultStat.Variables[index].Value = std::stoi(value);
				} else if (key == "max") {
					this->DefaultStat.Variables[index].Max = std::stoi(value);
				} else if (key == "increase") {
					this->DefaultStat.Variables[index].Increase = std::stoi(value);
				} else {
					throw std::runtime_error("Invalid variable property: \"" + key + "\".");
				}
			});

			return;
		}

		data_entry::process_sml_scope(scope);
	}
}

void unit_type::ProcessConfigData(const CConfigData *config_data)
{
	this->RemoveButtons(ButtonCmd::Move);
	this->RemoveButtons(ButtonCmd::Stop);
	this->RemoveButtons(ButtonCmd::Attack);
	this->RemoveButtons(ButtonCmd::Patrol);
	this->RemoveButtons(ButtonCmd::StandGround);
	this->RemoveButtons(ButtonCmd::Return);
		
	for (size_t i = 0; i < config_data->Properties.size(); ++i) {
		std::string key = config_data->Properties[i].first;
		std::string value = config_data->Properties[i].second;
		
		if (key == "name") {
			this->set_name(value);
		} else if (key == "parent") {
			unit_type *parent_type = unit_type::get(value);
			this->set_parent(parent_type);
		} else if (key == "civilization") {
			wyrmgus::civilization *civilization = civilization::get(value);
			this->civilization = civilization;
		} else if (key == "faction") {
			wyrmgus::faction *faction = faction::get(value);
			this->Faction = faction->ID;
		} else if (key == "animations") {
			this->animation_set = animation_set::get(value);
		} else if (key == "icon") {
			this->Icon.Name = value;
			this->Icon.Icon = nullptr;
			this->Icon.Load();
		} else if (key == "tile_width") {
			this->tile_size.setWidth(std::stoi(value));
		} else if (key == "tile_height") {
			this->tile_size.setHeight(std::stoi(value));
		} else if (key == "box_width") {
			this->box_size.setWidth(std::stoi(value));
		} else if (key == "box_height") {
			this->box_size.setHeight(std::stoi(value));
		} else if (key == "draw_level") {
			this->draw_level = std::stoi(value);
		} else if (key == "type") {
			if (value == "land") {
				this->UnitType = UnitTypeType::Land;
				this->LandUnit = true;
			} else if (value == "fly") {
				this->UnitType = UnitTypeType::Fly;
				this->AirUnit = true;
			} else if (value == "fly_low") {
				this->UnitType = UnitTypeType::FlyLow;
				this->AirUnit = true;
			} else if (value == "naval") {
				this->UnitType = UnitTypeType::Naval;
				this->SeaUnit = true;
			} else if (value == "space") {
				this->UnitType = UnitTypeType::Space;
				this->AirUnit = true;
			} else {
				fprintf(stderr, "Invalid unit type type: \"%s\".\n", value.c_str());
			}
		} else if (key == "priority") {
			this->DefaultStat.Variables[PRIORITY_INDEX].Value = std::stoi(value);
			this->DefaultStat.Variables[PRIORITY_INDEX].Max  = std::stoi(value);
		} else if (key == "description") {
			this->set_description(value);
		} else if (key == "background") {
			this->set_background(value);
		} else if (key == "quote") {
			this->set_quote(value);
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
			this->Missile.Name = value;
			this->Missile.Missile = nullptr;
		} else if (key == "fire_missile") {
			this->FireMissile.Name = value;
			this->FireMissile.Missile = nullptr;
		} else if (key == "corpse") {
			this->corpse_type = unit_type::get(value);
		} else if (key == "weapon_class") {
			this->WeaponClasses.push_back(string_to_item_class(value));
		} else if (key == "ai_drop") {
			unit_type *drop_type = unit_type::get(value);
			this->AiDrops.push_back(drop_type);
		} else if (key == "item_class") {
			this->item_class = string_to_item_class(value);
		} else if (key == "species") {
			this->species = species::get(value);
			this->species->Type = this;
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
			this->CanAttack = string::to_bool(value);
		} else if (key == "can_target_land") {
			const bool can_target_land = string::to_bool(value);
			if (can_target_land) {
				this->CanTarget |= CanTargetLand;
			} else {
				this->CanTarget &= ~CanTargetLand;
			}
		} else if (key == "can_target_sea") {
			const bool can_target_sea = string::to_bool(value);
			if (can_target_sea) {
				this->CanTarget |= CanTargetSea;
			} else {
				this->CanTarget &= ~CanTargetSea;
			}
		} else if (key == "can_target_air") {
			const bool can_target_air = string::to_bool(value);
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
			spell *spell = spell::get(value);
			this->Spells.push_back(spell);
		} else if (key == "autocast_active") {
			if (value == "false") {
				this->autocast_spells.clear();
				this->spell_autocast.clear();
			} else {
				const spell *spell = spell::get(value);
				this->add_autocast_spell(spell);
			}
		} else {
			key = string::snake_case_to_pascal_case(key);
			
			int index = UnitTypeVar.VariableNameLookup[key.c_str()]; // variable index
			if (index != -1) { // valid index
				if (string::is_number(value)) {
					this->DefaultStat.Variables[index].Enable = 1;
					this->DefaultStat.Variables[index].Value = std::stoi(value);
					this->DefaultStat.Variables[index].Max = std::stoi(value);
				} else if (IsStringBool(value)) {
					this->DefaultStat.Variables[index].Enable = string::to_bool(value);
				} else { // error
					fprintf(stderr, "Invalid value (\"%s\") for variable \"%s\" when defining unit type \"%s\".\n", value.c_str(), key.c_str(), this->Ident.c_str());
				}
			} else {
				if (this->BoolFlag.size() < UnitTypeVar.GetNumberBoolFlag()) {
					this->BoolFlag.resize(UnitTypeVar.GetNumberBoolFlag());
				}

				index = UnitTypeVar.BoolFlagNameLookup[key.c_str()];
				if (index != -1) {
					if (string::is_number(value)) {
						this->BoolFlag[index].value = (std::stoi(value) != 0);
					} else {
						this->BoolFlag[index].value = string::to_bool(value);
					}
				} else {
					fprintf(stderr, "Invalid unit type property: \"%s\".\n", key.c_str());
				}
			}
		}
	}
	
	for (const CConfigData *child_config_data : config_data->Children) {
		if (child_config_data->Tag == "costs") {
			for (size_t j = 0; j < child_config_data->Properties.size(); ++j) {
				std::string key = child_config_data->Properties[j].first;
				std::string value = child_config_data->Properties[j].second;
				
				key = FindAndReplaceString(key, "_", "-");
				
				const int resource = GetResourceIdByName(key.c_str());
				if (resource != -1) {
					this->DefaultStat.Costs[resource] = std::stoi(value);
				} else {
					fprintf(stderr, "Invalid resource: \"%s\".\n", key.c_str());
				}
			}
		} else if (child_config_data->Tag == "image") {
			for (size_t j = 0; j < child_config_data->Properties.size(); ++j) {
				std::string key = child_config_data->Properties[j].first;
				std::string value = child_config_data->Properties[j].second;
				
				if (key == "file") {
					this->image_file = CMod::GetCurrentModPath() + value;
				} else if (key == "width") {
					this->frame_size.setWidth(std::stoi(value));
				} else if (key == "height") {
					this->frame_size.setHeight(std::stoi(value));
				} else {
					fprintf(stderr, "Invalid image property: \"%s\".\n", key.c_str());
				}
			}
			
			if (this->image_file.empty()) {
				fprintf(stderr, "Image has no file.\n");
			}
			
			if (this->frame_size.width() == 0) {
				fprintf(stderr, "Image has no width.\n");
			}
			
			if (this->frame_size.height() == 0) {
				fprintf(stderr, "Image has no height.\n");
			}
			
			if (this->Sprite != nullptr) {
				this->Sprite.reset();
			}
		} else if (child_config_data->Tag == "default_equipment") {
			for (size_t j = 0; j < child_config_data->Properties.size(); ++j) {
				std::string key = child_config_data->Properties[j].first;
				std::string value = child_config_data->Properties[j].second;
				
				const item_slot item_slot = string_to_item_slot(key);
				if (item_slot == item_slot::none) {
					fprintf(stderr, "Invalid item slot for default equipment: \"%s\".\n", key.c_str());
					continue;
				}
				
				unit_type *item = unit_type::get(value);
				
				this->DefaultEquipment[item_slot] = item;
			}
		} else if (child_config_data->Tag == "sounds") {
			for (size_t j = 0; j < child_config_data->Properties.size(); ++j) {
				std::string key = child_config_data->Properties[j].first;
				std::string value = child_config_data->Properties[j].second;
				
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
		} else if (child_config_data->Tag == "preconditions") {
			this->preconditions = std::make_unique<and_condition>();
			this->preconditions->ProcessConfigData(child_config_data);
		} else if (child_config_data->Tag == "conditions") {
			this->conditions = std::make_unique<and_condition>();
			this->conditions->ProcessConfigData(child_config_data);
		} else {
			std::string tag = string::snake_case_to_pascal_case(child_config_data->Tag);
			
			const int index = UnitTypeVar.VariableNameLookup[tag.c_str()]; // variable index
			
			if (index != -1) { // valid index
				for (size_t j = 0; j < child_config_data->Properties.size(); ++j) {
					std::string key = child_config_data->Properties[j].first;
					std::string value = child_config_data->Properties[j].second;
					
					if (key == "enable") {
						this->DefaultStat.Variables[index].Enable = string::to_bool(value);
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
				fprintf(stderr, "Invalid unit type property: \"%s\".\n", child_config_data->Tag.c_str());
			}
		}
	}

	this->set_defined(true);
}

void unit_type::initialize()
{
	if (this->get_unit_class() != nullptr) { //if class is defined, then use this unit type to help build the classes table, and add this unit to the civilization class table (if the civilization is defined)
		//see if this unit type is set as the civilization class unit type or the faction class unit type of any civilization/class (or faction/class) combination, and remove it from there (to not create problems with redefinitions)
		for (wyrmgus::civilization *civilization : civilization::get_all()) {
			civilization->remove_class_unit_type(this);
		}

		for (wyrmgus::faction *faction : faction::get_all()) {
			faction->remove_class_unit_type(this);
		}

		const wyrmgus::unit_class *unit_class = this->get_unit_class();
		if (unit_class != nullptr) {
			if (this->Faction != -1) {
				faction::get_all()[this->Faction]->set_class_unit_type(unit_class, this);
			} else if (this->get_civilization() != nullptr) {
				this->get_civilization()->set_class_unit_type(unit_class, this);
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

	//unit type's level must be at least 1
	if (this->DefaultStat.Variables[LEVEL_INDEX].Value == 0) {
		this->DefaultStat.Variables[LEVEL_INDEX].Enable = 1;
		this->DefaultStat.Variables[LEVEL_INDEX].Value = 1;
		this->DefaultStat.Variables[LEVEL_INDEX].Max = 1;
	}

	// FIXME: try to simplify/combine the flags instead
	if (this->MouseAction == MouseActionAttack && !this->CanAttack) {
		fprintf(stderr, "Unit-type '%s': right-attack is set, but can-attack is not.\n", this->get_name().c_str());
	}
	this->UpdateDefaultBoolFlags();
	if (GameRunning || Editor.Running == EditorEditing) {
		InitUnitType(*this);
		LoadUnitType(*this);
	}

	if (!CclInConfigFile || GameRunning || Editor.Running == EditorEditing) {
		UpdateUnitStats(*this, 1);
	}

	if (Editor.Running == EditorEditing && std::find(Editor.UnitTypes.begin(), Editor.UnitTypes.end(), this->Ident) == Editor.UnitTypes.end()) {
		Editor.UnitTypes.push_back(this->Ident);
		RecalculateShownUnits();
	}

	for (size_t i = 0; i < this->Trains.size(); ++i) {
		std::string button_definition = "DefineButton({\n";
		button_definition += "\tPos = " + std::to_string(this->Trains[i]->ButtonPos) + ",\n";
		if (this->Trains[i]->ButtonLevel) {
			button_definition += "\tLevel = " + this->Trains[i]->ButtonLevel->get_identifier() + ",\n";
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
		button_definition += "\tKey = \"" + this->Trains[i]->get_button_key() + "\",\n";
		button_definition += "\tHint = \"" + this->Trains[i]->ButtonHint + "\",\n";
		button_definition += "\tForUnit = {\"" + this->Ident + "\"},\n";
		button_definition += "})";
		CclCommand(button_definition);
	}

	if (this->CanMove()) {
		std::string button_definition = "DefineButton({\n";
		button_definition += "\tPos = 1,\n";
		button_definition += "\tAction = \"move\",\n";
		button_definition += "\tPopup = \"popup_commands\",\n";
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
		button_definition += "\tPopup = \"popup_commands\",\n";
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
		button_definition += "\tPopup = \"popup_commands\",\n";
		button_definition += "\tKey = \"a\",\n";
		button_definition += "\tHint = _(\"~!Attack\"),\n";
		button_definition += "\tForUnit = {\"" + this->Ident + "\"},\n";
		button_definition += "})";
		CclCommand(button_definition);
	}

	if (this->CanMove() && ((!this->BoolFlag[COWARD_INDEX].value && this->CanAttack) || this->UnitType == UnitTypeType::Fly || this->UnitType == UnitTypeType::Space)) {
		std::string button_definition = "DefineButton({\n";
		button_definition += "\tPos = 4,\n";
		button_definition += "\tAction = \"patrol\",\n";
		button_definition += "\tPopup = \"popup_commands\",\n";
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
		button_definition += "\tPopup = \"popup_commands\",\n";
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

	CclCommand("if not (GetArrayIncludes(Units, \"" + this->get_identifier() + "\")) then table.insert(Units, \"" + this->get_identifier() + "\") end"); //FIXME: needed at present to make unit type data files work without scripting being necessary, but it isn't optimal to interact with a scripting table like "Units" in this manner (that table should probably be replaced with getting a list of unit types from the engine)

	data_entry::initialize();
}

void unit_type::check() const
{
	for (const spell *spell : this->get_autocast_spells()) {
		if (spell->get_autocast_info() == nullptr) {
			throw std::runtime_error("The spell \"" + spell->get_identifier() + "\" is set to be autocast by default for unit type \"" + this->get_identifier() + "\", but has no defined autocast method.");
		}
	}

	if (this->get_preconditions() != nullptr) {
		this->get_preconditions()->check_validity();
	}

	if (this->get_conditions() != nullptr) {
		this->get_conditions()->check_validity();
	}
}

void unit_type::set_unit_class(wyrmgus::unit_class *unit_class)
{
	if (unit_class == this->get_unit_class()) {
		return;
	}

	if (this->get_unit_class() != nullptr) {
		this->get_unit_class()->remove_unit_type(this);
	}

	this->unit_class = unit_class;

	if (this->get_unit_class() != nullptr && !this->get_unit_class()->has_unit_type(this)) {
		this->get_unit_class()->add_unit_type(this);
	}
}

const civilization *unit_type::get_faction_civilization(const wyrmgus::faction *faction) const
{
	//get the civilization the unit type would have for a given faction
	const wyrmgus::civilization *civilization = this->get_civilization();

	if (faction == nullptr) {
		return civilization;
	}

	const wyrmgus::civilization *faction_civilization = faction->get_civilization();

	if (civilization != nullptr && faction_civilization != nullptr && faction_civilization != civilization && this == faction->get_class_unit_type(this->get_unit_class()) && (!this->BoolFlag[ORGANIC_INDEX].value || civilization->get_species() == faction_civilization->get_species())) {
		return faction_civilization;
	}

	return civilization;
}

const civilization *unit_type::get_player_civilization(const CPlayer *player) const
{
	//get the civilization the unit type would have for a given player
	const wyrmgus::faction *player_faction = player->get_faction();
	if (player_faction != nullptr) {
		return this->get_faction_civilization(player_faction);
	}

	return this->get_civilization();
}

QSize unit_type::get_half_tile_size() const
{
	return this->get_tile_size() / 2;
}

PixelSize unit_type::get_tile_pixel_size() const
{
	return this->get_tile_size() * defines::get()->get_tile_size();
}

PixelSize unit_type::get_scaled_tile_pixel_size() const
{
	return this->get_tile_pixel_size() * defines::get()->get_scale_factor();
}

QPoint unit_type::get_tile_center_pos_offset() const
{
	//the offset from the tile's top-left position to its center tile
	return (size::to_point(this->get_tile_size()) - QPoint(1, 1)) / 2;
}

void unit_type::set_image_file(const std::filesystem::path &filepath)
{
	if (filepath == this->get_image_file()) {
		return;
	}

	this->image_file = database::get_graphics_path(this->get_module()) / filepath;
}

bool unit_type::is_autocast_spell(const spell *spell) const
{
	if (static_cast<size_t>(spell->Slot) < this->spell_autocast.size()) {
		return this->spell_autocast[spell->Slot];
	}

	return false;
}

void unit_type::add_autocast_spell(const spell *spell)
{
	if (static_cast<size_t>(spell->Slot) >= this->spell_autocast.size()) {
		this->spell_autocast.resize(spell->Slot + 1, false);
	}
	this->spell_autocast[spell->Slot] = true;
	this->autocast_spells.push_back(spell);
}

bool unit_type::CheckUserBoolFlags(const char *BoolFlags) const
{
	for (unsigned int i = 0; i < UnitTypeVar.GetNumberBoolFlag(); ++i) { // User defined flags
		if (BoolFlags[i] != CONDITION_TRUE &&
			((BoolFlags[i] == CONDITION_ONLY) ^ (BoolFlag[i].value))) {
			return false;
		}
	}
	return true;
}

bool unit_type::CanMove() const
{
	return this->get_animation_set() != nullptr && this->get_animation_set()->Move != nullptr;
}

bool unit_type::CanSelect(GroupSelectionMode mode) const
{
	if (!BoolFlag[ISNOTSELECTABLE_INDEX].value) {
		switch (mode) {
			case GroupSelectionMode::SELECTABLE_BY_RECTANGLE_ONLY:
				return BoolFlag[SELECTABLEBYRECTANGLE_INDEX].value;
			case GroupSelectionMode::NON_SELECTABLE_BY_RECTANGLE_ONLY:
				return !BoolFlag[SELECTABLEBYRECTANGLE_INDEX].value;
			default:
				return true;
		}
	}
	return false;
}

void unit_type::set_parent(const unit_type *parent_type)
{
	if (!parent_type->is_defined()) {
		throw std::runtime_error("Unit type \"" + this->get_identifier() + "\" is inheriting features from non-defined parent \"" + parent_type->get_identifier() + "\".");
	}
	
	this->Parent = parent_type;
	
	if (this->get_name().empty()) {
		this->set_name(parent_type->get_name());
	}
	this->unit_class = parent_type->get_unit_class();
	if (this->get_unit_class() != nullptr && !this->get_unit_class()->has_unit_type(this)) {
		this->get_unit_class()->add_unit_type(this);
	}
	this->draw_level = parent_type->draw_level;
	this->image_file = parent_type->image_file;
	this->frame_size = parent_type->frame_size;
	this->offset = parent_type->offset;
	this->ShadowFile = parent_type->ShadowFile;
	this->ShadowWidth = parent_type->ShadowWidth;
	this->ShadowHeight = parent_type->ShadowHeight;
	this->ShadowOffsetX = parent_type->ShadowOffsetX;
	this->ShadowOffsetY = parent_type->ShadowOffsetY;
	this->LightFile = parent_type->LightFile;
	this->tile_size = parent_type->tile_size;
	this->box_size = parent_type->box_size;
	this->BoxOffsetX = parent_type->BoxOffsetX;
	this->BoxOffsetY = parent_type->BoxOffsetY;
	this->construction = parent_type->construction;
	this->UnitType = parent_type->UnitType;
	this->Missile.Name = parent_type->Missile.Name;
	this->Missile.Missile = nullptr;
	this->FireMissile.Name = parent_type->FireMissile.Name;
	this->FireMissile.Missile = nullptr;
	this->ExplodeWhenKilled = parent_type->ExplodeWhenKilled;
	this->Explosion.Name = parent_type->Explosion.Name;
	this->Explosion.Missile = nullptr;
	this->corpse_type = parent_type->corpse_type;
	this->MinAttackRange = parent_type->MinAttackRange;
	this->DefaultStat.Variables[ATTACKRANGE_INDEX].Value = parent_type->DefaultStat.Variables[ATTACKRANGE_INDEX].Value;
	this->DefaultStat.Variables[ATTACKRANGE_INDEX].Max = parent_type->DefaultStat.Variables[ATTACKRANGE_INDEX].Max;
	this->DefaultStat.Variables[PRIORITY_INDEX].Value = parent_type->DefaultStat.Variables[PRIORITY_INDEX].Value;
	this->DefaultStat.Variables[PRIORITY_INDEX].Max  = parent_type->DefaultStat.Variables[PRIORITY_INDEX].Max;
	this->AnnoyComputerFactor = parent_type->AnnoyComputerFactor;
	this->TrainQuantity = parent_type->TrainQuantity;
	this->CostModifier = parent_type->CostModifier;
	this->item_class = parent_type->item_class;
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
	this->button_key = parent_type->button_key;
	this->BurnPercent = parent_type->BurnPercent;
	this->BurnDamageRate = parent_type->BurnDamageRate;
	this->PoisonDrain = parent_type->PoisonDrain;
	this->AutoBuildRate = parent_type->AutoBuildRate;
	this->animation_set = parent_type->animation_set;
	this->Sound = parent_type->Sound;
	this->NumDirections = parent_type->NumDirections;
	this->NeutralMinimapColorRGB = parent_type->NeutralMinimapColorRGB;
	this->RandomMovementProbability = parent_type->RandomMovementProbability;
	this->RandomMovementDistance = parent_type->RandomMovementDistance;
	this->given_resource = parent_type->given_resource;
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
	this->spell_autocast = parent_type->spell_autocast;
	for (unsigned int i = 0; i < MaxCosts; ++i) {
		this->DefaultStat.Costs[i] = parent_type->DefaultStat.Costs[i];
		this->RepairCosts[i] = parent_type->RepairCosts[i];
		this->DefaultStat.ImproveIncomes[i] = parent_type->DefaultStat.ImproveIncomes[i];
		this->DefaultStat.ResourceDemand[i] = parent_type->DefaultStat.ResourceDemand[i];
		this->CanStore[i] = parent_type->CanStore[i];
		this->GrandStrategyProductionEfficiencyModifier[i] = parent_type->GrandStrategyProductionEfficiencyModifier[i];
		
		if (parent_type->ResInfo[i] != nullptr) {
			this->ResInfo[i] = parent_type->ResInfo[i]->duplicate(this);
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
	this->StartingAbilities = parent_type->StartingAbilities;
	for (size_t i = 0; i < parent_type->Trains.size(); ++i) {
		this->Trains.push_back(parent_type->Trains[i]);
		parent_type->Trains[i]->TrainedBy.push_back(this);
	}
	for (size_t i = 0; i < parent_type->StartingResources.size(); ++i) {
		this->StartingResources.push_back(parent_type->StartingResources[i]);
	}
	this->PersonalNames = parent_type->PersonalNames;

	for (const auto &building_rule : parent_type->BuildingRules) {
		this->BuildingRules.push_back(building_rule->duplicate());
	}

	for (const auto &building_rule : parent_type->AiBuildingRules) {
		this->AiBuildingRules.push_back(building_rule->duplicate());
	}

	for (const auto &parent_variation : parent_type->variations) {
		this->variations.push_back(parent_variation->duplicate(this));
	}
	
	for (int i = 0; i < MaxImageLayers; ++i) {
		this->LayerFiles[i] = parent_type->LayerFiles[i];
		
		//inherit layer variations
		for (const auto &parent_variation : parent_type->LayerVariations[i]) {
			this->LayerVariations[i].push_back(parent_variation->duplicate(this));
		}
	}
	for (std::map<ButtonCmd, IconConfig>::const_iterator iterator = parent_type->ButtonIcons.begin(); iterator != parent_type->ButtonIcons.end(); ++iterator) {
		this->ButtonIcons[iterator->first].Name = iterator->second.Name;
		this->ButtonIcons[iterator->first].Icon = nullptr;
		this->ButtonIcons[iterator->first].Load();
	}
	this->DefaultEquipment = parent_type->DefaultEquipment;
	this->DefaultStat.Variables[PRIORITY_INDEX].Value = parent_type->DefaultStat.Variables[PRIORITY_INDEX].Value + 1; //increase priority by 1 to make it be chosen by the AI when building over the previous unit
	this->DefaultStat.Variables[PRIORITY_INDEX].Max = parent_type->DefaultStat.Variables[PRIORITY_INDEX].Max + 1;
}

void unit_type::UpdateDefaultBoolFlags()
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
void unit_type::RemoveButtons(const ButtonCmd button_action, const std::string &mod_file)
{
	int buttons_size = button::get_all().size();
	for (int i = (buttons_size - 1); i >= 0; --i) {
		if (button_action != ButtonCmd::None && button::get_all()[i]->Action != button_action) {
			continue;
		}
		if (!mod_file.empty() && button::get_all()[i]->Mod != mod_file) {
			continue;
		}
		
		if (button::get_all()[i]->UnitMask == ("," + this->Ident + ",")) { //delete the appropriate buttons
			button::remove(button::get_all()[i]);
		} else if (button::get_all()[i]->UnitMask.find(this->Ident) != std::string::npos) { //remove this unit from the "ForUnit" array of the appropriate buttons
			button::get_all()[i]->UnitMask = FindAndReplaceString(button::get_all()[i]->UnitMask, this->Ident + ",", "");
		}
	}
}

int unit_type::GetAvailableLevelUpUpgrades() const
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

int unit_type::GetResourceStep(const int resource, const int player) const
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

const unit_type_variation *unit_type::GetDefaultVariation(const CPlayer *player, const int image_layer) const
{
	const std::vector<std::unique_ptr<unit_type_variation>> &variation_list = image_layer == -1 ? this->get_variations() : this->LayerVariations[image_layer];
	for (const auto &variation : variation_list) {
		bool upgrades_check = true;
		for (const CUpgrade *required_upgrade : variation->UpgradesRequired) {
			if (UpgradeIdentAllowed(*player, required_upgrade->get_identifier().c_str()) != 'R') {
				upgrades_check = false;
				break;
			}
		}
		
		if (upgrades_check) {
			for (const CUpgrade *forbidden_upgrade : variation->UpgradesForbidden) {
				if (UpgradeIdentAllowed(*player, forbidden_upgrade->get_identifier().c_str()) == 'R') {
					upgrades_check = false;
					break;
				}
			}
		}
		
		if (upgrades_check == false) {
			continue;
		}
		return variation.get();
	}
	return nullptr;
}

unit_type_variation *unit_type::GetVariation(const std::string &variation_name, int image_layer) const
{
	const std::vector<std::unique_ptr<unit_type_variation>> &variation_list = image_layer == -1 ? this->get_variations() : this->LayerVariations[image_layer];
	for (const auto &variation : variation_list) {
		if (variation->get_identifier() == variation_name) {
			return variation.get();
		}
	}
	return nullptr;
}

std::string unit_type::GetRandomVariationIdent(int image_layer) const
{
	std::vector<std::string> variation_idents;
	
	const std::vector<std::unique_ptr<unit_type_variation>> &variation_list = image_layer == -1 ? this->get_variations() : this->LayerVariations[image_layer];
	for (const auto &variation : variation_list) {
		variation_idents.push_back(variation->get_identifier());
	}
	
	if (variation_idents.size() > 0) {
		return variation_idents[SyncRand(variation_idents.size())];
	}
	
	return "";
}

const std::string &unit_type::GetDefaultName(const CPlayer *player) const
{
	const unit_type_variation *variation = this->GetDefaultVariation(player);
	if (variation && !variation->TypeName.empty()) {
		return variation->TypeName;
	} else {
		return this->get_name();
	}
}

const std::shared_ptr<CPlayerColorGraphic> &unit_type::GetDefaultLayerSprite(const CPlayer *player, int image_layer) const
{
	const unit_type_variation *variation = this->GetDefaultVariation(player);
	if (this->LayerVariations[image_layer].size() > 0 && this->GetDefaultVariation(player, image_layer)->Sprite) {
		return this->GetDefaultVariation(player, image_layer)->Sprite;
	} else if (variation && variation->LayerSprites[image_layer]) {
		return variation->LayerSprites[image_layer];
	} else if (this->LayerSprites[image_layer])  {
		return this->LayerSprites[image_layer];
	} else {
		static std::shared_ptr<CPlayerColorGraphic> null_graphic;
		return null_graphic;
	}
}

const std::string &unit_type::get_default_button_key(const CPlayer *player) const
{
	const unit_type_variation *variation = this->GetDefaultVariation(player);
	if (variation != nullptr && !variation->get_button_key().empty()) {
		return variation->get_button_key();
	} else {
		return this->get_button_key();
	}
}

bool unit_type::CanExperienceUpgradeTo(const unit_type *type) const
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

std::string unit_type::GetNamePlural() const
{
	return GetPluralForm(this->get_name());
}

std::string unit_type::GeneratePersonalName(const wyrmgus::faction *faction, const gender gender) const
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

bool unit_type::IsPersonalNameValid(const std::string &name, const wyrmgus::faction *faction, const gender gender) const
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

std::vector<std::string> unit_type::GetPotentialPersonalNames(const wyrmgus::faction *faction, const gender gender) const
{
	std::vector<std::string> potential_names;
	
	if (this->PersonalNames.find(gender::none) != this->PersonalNames.end()) {
		for (size_t i = 0; i < this->PersonalNames.find(gender::none)->second.size(); ++i) {
			potential_names.push_back(this->PersonalNames.find(gender::none)->second[i]);
		}
	}
	if (gender != gender::none && this->PersonalNames.find(gender) != this->PersonalNames.end()) {
		for (size_t i = 0; i < this->PersonalNames.find(gender)->second.size(); ++i) {
			potential_names.push_back(this->PersonalNames.find(gender)->second[i]);
		}
	}
	
	if (potential_names.size() == 0 && this->get_civilization() != nullptr) {
		const wyrmgus::civilization *civilization = this->get_faction_civilization(faction);
		if (faction != nullptr && faction->get_civilization() != civilization) {
			faction = nullptr;
		}
		if (this->Faction != -1 && !faction) {
			faction = faction::get_all()[this->Faction];
		}

		if (this->BoolFlag[ORGANIC_INDEX].value) {
			if (civilization->get_personal_names().find(gender::none) != civilization->get_personal_names().end()) {
				for (size_t i = 0; i < civilization->get_personal_names().find(gender::none)->second.size(); ++i) {
					potential_names.push_back(civilization->get_personal_names().find(gender::none)->second[i]);
				}
			}
			if (gender != gender::none && civilization->get_personal_names().find(gender) != civilization->get_personal_names().end()) {
				for (size_t i = 0; i < civilization->get_personal_names().find(gender)->second.size(); ++i) {
					potential_names.push_back(civilization->get_personal_names().find(gender)->second[i]);
				}
			}
		} else {
			if (this->get_unit_class() != nullptr && civilization->get_unit_class_names(this->get_unit_class()).size() > 0) {
				return civilization->get_unit_class_names(this->get_unit_class());
			}

			if (this->UnitType == UnitTypeType::Naval) { // if is a ship
				if (faction && !faction->get_ship_names().empty()) {
					return faction->get_ship_names();
				}

				if (!civilization->get_ship_names().empty()) {
					return civilization->get_ship_names();
				}
			}
		}
	}
	
	return potential_names;
}

void resource_info::process_sml_property(const sml_property &property)
{
	const std::string &key = property.get_key();
	const std::string &value = property.get_value();

	if (key == "image_file") {
		this->image_file = database::get_graphics_path(this->get_unit_type()->get_module()) / value;
	} else if (key == "loaded_image_file") {
		this->loaded_image_file = database::get_graphics_path(this->get_unit_type()->get_module()) / value;
	} else if (key == "resource_step") {
		this->ResourceStep = std::stoi(value);
	} else if (key == "wait_at_resource") {
		this->WaitAtResource = std::stoi(value);
	} else if (key == "wait_at_depot") {
		this->WaitAtDepot = std::stoi(value);
	} else if (key == "resource_capacity") {
		this->ResourceCapacity = std::stoi(value);
	} else if (key == "lose_resources") {
		this->LoseResources = string::to_bool(value);
	} else {
		throw std::runtime_error("Invalid resource gathering info property: \"" + key + "\".");
	}
}

void resource_info::process_sml_scope(const sml_data &scope)
{
	throw std::runtime_error("Invalid resource gathering info scope: \"" + scope.get_tag() + "\".");
}

}

void UpdateUnitStats(wyrmgus::unit_type &type, int reset)
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

			for (const auto &unit_stock_kv_pair : iterator->second.UnitStock) {
				const wyrmgus::unit_type *unit_type = wyrmgus::unit_type::get_all()[unit_stock_kv_pair.first];
				const int unit_stock = unit_stock_kv_pair.second;

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
		for (std::map<std::string, wyrmgus::unit_sound_set>::iterator iterator = type.ModSounds.begin(); iterator != type.ModSounds.end(); ++iterator) {
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
								MapFieldUnpassable |
								MapFieldSpace;
			type.FieldFlags = MapFieldNoBuilding;
		} else {
			type.MovementMask = 0;
			type.FieldFlags = 0;
		}
		return;
	}

	//  As side effect we calculate the movement flags/mask here.
	switch (type.UnitType) {
		case UnitTypeType::Land:                              // on land
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
					MapFieldUnpassable |
					MapFieldSpace;
			} else {
				type.MovementMask =
					MapFieldLandUnit |
					MapFieldSeaUnit |
					MapFieldBuilding | // already occuppied
					MapFieldCoastAllowed |
					MapFieldWaterAllowed | // can't move on this
					MapFieldUnpassable |
					MapFieldSpace;
			}
			
			if (type.BoolFlag[RAIL_INDEX].value) { //rail units can only move over railroads
				type.MovementMask |= MapFieldNoRail;
			}
			//Wyrmgus end
			break;
		case UnitTypeType::Fly:                               // in air
			//Wyrmgus start
			/*
			type.MovementMask = MapFieldAirUnit; // already occuppied
				MapFieldAirUnit | // already occuppied
				MapFieldAirUnpassable;
			*/
			if (type.BoolFlag[DIMINUTIVE_INDEX].value) {
				type.MovementMask =
					MapFieldAirUnpassable |
					MapFieldSpace;
			} else {
				type.MovementMask =
					MapFieldAirUnit | // already occuppied
					MapFieldAirUnpassable |
					MapFieldSpace;
			}
			//Wyrmgus end
			break;
		//Wyrmgus start
		case UnitTypeType::FlyLow:                               // in low air
			if (type.BoolFlag[DIMINUTIVE_INDEX].value) {
				type.MovementMask =
					MapFieldBuilding |
					MapFieldUnpassable |
					MapFieldAirUnpassable |
					MapFieldSpace;
			} else {
				type.MovementMask =
					MapFieldLandUnit |
					MapFieldSeaUnit |
					MapFieldBuilding |
					MapFieldUnpassable |
					MapFieldAirUnpassable |
					MapFieldSpace;
			}
			break;
		case UnitTypeType::Naval: // on water
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
					MapFieldUnpassable |
					MapFieldSpace;
			} else if (type.BoolFlag[CANDOCK_INDEX].value && type.BoolFlag[DIMINUTIVE_INDEX].value) { //should add case for when is a transporter and is diminutive?
				type.MovementMask =
					MapFieldBuilding | // already occuppied
					MapFieldBridge |
					MapFieldLandAllowed | // can't move on this
					MapFieldUnpassable |
					MapFieldSpace;
			} else if (type.BoolFlag[DIMINUTIVE_INDEX].value) { //should add case for when is a transporter and is diminutive?
				type.MovementMask =
					MapFieldBuilding | // already occuppied
					MapFieldBridge |
					MapFieldCoastAllowed |
					MapFieldLandAllowed | // can't move on this
					MapFieldUnpassable |
					MapFieldSpace;
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
					MapFieldUnpassable |
					MapFieldSpace;
			}
			break;
		case UnitTypeType::Space:
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
				MapFieldLandAllowed | // can't build on this
				MapFieldSpace;
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
							MapFieldSpace |
							MapFieldItem;
		type.FieldFlags = MapFieldItem;
	} else if (type.BoolFlag[BRIDGE_INDEX].value) {
		type.MovementMask = MapFieldSeaUnit |
							MapFieldBuilding |
							MapFieldLandAllowed |
							MapFieldUnpassable |
							MapFieldSpace |
							MapFieldBridge;
		type.FieldFlags = MapFieldBridge;
	//Wyrmgus end
	//Wyrmgus start
//	} else {
	} else if (!type.BoolFlag[DIMINUTIVE_INDEX].value) {
	//Wyrmgus end
		switch (type.UnitType) {
			case UnitTypeType::Land: // on land
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
			case UnitTypeType::Fly: // in air
			case UnitTypeType::Space:
				type.FieldFlags = MapFieldAirUnit;
				break;
			//Wyrmgus start
			case UnitTypeType::FlyLow: // in low air
				type.FieldFlags = MapFieldLandUnit;			
				if (type.BoolFlag[AIRUNPASSABLE_INDEX].value) { // for air unpassable units (i.e. doors)
					type.FieldFlags |= MapFieldUnpassable;
					type.FieldFlags |= MapFieldAirUnpassable;
				}
				break;
			//Wyrmgus end
			case UnitTypeType::Naval: // on water
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
	for (wyrmgus::unit_type *unit_type : wyrmgus::unit_type::get_all()) {
		UpdateUnitStats(*unit_type, reset);
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
static bool SaveUnitStats(const CUnitStats &stats, const wyrmgus::unit_type &type, int plynr,
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
	for (const wyrmgus::unit_type *unit_type : wyrmgus::unit_type::get_all()) {
		if (stats.GetUnitStock(unit_type) == type.DefaultStat.GetUnitStock(unit_type)) {
			continue;
		}
		if (unit_type->Slot > 0) {
			file.printf(" ");
		}
		file.printf("\"%s\", %d,", unit_type->Ident.c_str(), stats.GetUnitStock(unit_type));
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
	for (const wyrmgus::unit_type *unit_type : wyrmgus::unit_type::get_all()) {
		bool somethingSaved = false;

		for (int j = 0; j < PlayerMax; ++j) {
			if (CPlayer::Players[j]->Type != PlayerNobody) {
				somethingSaved |= SaveUnitStats(unit_type->Stats[j], *unit_type, j, file);
			}
		}
		if (somethingSaved) {
			file.printf("\n");
		}
	}
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
void DrawUnitType(const wyrmgus::unit_type &type, const std::shared_ptr<CPlayerColorGraphic> &sprite, int player, int frame, const PixelPos &screenPos, const wyrmgus::time_of_day *time_of_day)
{
	//Wyrmgus start
	if (sprite == nullptr) {
		return;
	}
	//Wyrmgus end
	
	const wyrmgus::player_color *player_color = CPlayer::Players[player]->get_player_color();

	PixelPos pos = screenPos;
	// FIXME: move this calculation to high level.
	pos -= PixelPos((sprite->get_frame_size() - type.get_tile_size() * wyrmgus::defines::get()->get_scaled_tile_size()) / 2);
	pos.x += type.get_offset().x() * wyrmgus::defines::get()->get_scale_factor();
	pos.y += type.get_offset().y() * wyrmgus::defines::get()->get_scale_factor();

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
				sprite->DrawPlayerColorFrameClipTransX(player_color, -frame - 1, pos.x, pos.y, int(256 - 2.56 * type.Stats[player].Variables[TRANSPARENCY_INDEX].Value), time_of_day);
			} else {
				sprite->DrawPlayerColorFrameClipX(player_color, -frame - 1, pos.x, pos.y, time_of_day);
			}
		} else {
			if (type.Stats[player].Variables[TRANSPARENCY_INDEX].Value > 0) {
				sprite->DrawPlayerColorFrameClipTrans(player_color, frame, pos.x, pos.y, int(256 - 2.56 * type.Stats[player].Variables[TRANSPARENCY_INDEX].Value), time_of_day);
			} else {
				sprite->DrawPlayerColorFrameClip(player_color, frame, pos.x, pos.y, time_of_day);
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
			sprite->DrawPlayerColorFrameClipTrans(player_color, frame, pos.x, pos.y, int(256 - 2.56 * type.Stats[player].Variables[TRANSPARENCY_INDEX].Value), time_of_day);
		} else {
			sprite->DrawPlayerColorFrameClip(player_color, frame, pos.x, pos.y, time_of_day);
		}
	}
	//Wyrmgus end
}

/**
**  Get the still animation frame
*/
static int GetStillFrame(const wyrmgus::unit_type &type)
{
	if (type.get_animation_set() == nullptr) {
		return 0;
	}
	
	CAnimation *anim = type.get_animation_set()->Still.get();

	while (anim) {
		if (anim->Type == AnimationFrame) {
			CAnimation_Frame &a_frame = *static_cast<CAnimation_Frame *>(anim);
			// Use the frame facing down
			return a_frame.get_frame() + type.NumDirections / 2;
		} else if (anim->Type == AnimationExactFrame) {
			CAnimation_ExactFrame &a_frame = *static_cast<CAnimation_ExactFrame *>(anim);

			return a_frame.get_frame();
		}
		anim = anim->get_next();
	}
	return type.NumDirections / 2;
}

/**
**  Init unit types.
*/
void InitUnitTypes(int reset_player_stats)
{
	for (wyrmgus::unit_type *unit_type : wyrmgus::unit_type::get_all()) {
		if (unit_type->get_animation_set() == nullptr) {
			DebugPrint(_("unit-type '%s' without animations, ignored.\n") _C_ unit_type->Ident.c_str());
			continue;
		}
		
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
		InitUnitType(*unit_type);
		//Wyrmgus end
	}

	// LUDO : called after game is loaded -> don't reset stats !
	UpdateStats(reset_player_stats); // Calculate the stats
}

//Wyrmgus start
void InitUnitType(wyrmgus::unit_type &type)
{
	// Determine still frame
	type.StillFrame = GetStillFrame(type);

	// Lookup BuildingTypes
	for (const auto &b : type.BuildingRules) {
		b->Init();
	}

	// Lookup AiBuildingTypes
	for (const auto &b : type.AiBuildingRules) {
		b->Init();
	}
}
//Wyrmgus end

/**
**  Loads the Sprite for a unit type
**
**  @param type  type of unit to load
*/
void LoadUnitTypeSprite(wyrmgus::unit_type &type)
{
	if (!type.ShadowFile.empty()) {
		type.ShadowSprite = CGraphic::New(type.ShadowFile, type.ShadowWidth, type.ShadowHeight);
		type.ShadowSprite->Load(false, wyrmgus::defines::get()->get_scale_factor());
	}

	if (type.BoolFlag[HARVESTER_INDEX].value) {
		for (int i = 0; i < MaxCosts; ++i) {
			const std::unique_ptr<wyrmgus::resource_info> &resinfo = type.ResInfo[i];
			if (!resinfo) {
				continue;
			}

			if (!resinfo->get_image_file().empty()) {
				resinfo->SpriteWhenEmpty = CPlayerColorGraphic::New(resinfo->get_image_file().string(), type.get_frame_size(), type.get_conversible_player_color());
				resinfo->SpriteWhenEmpty->Load(false, wyrmgus::defines::get()->get_scale_factor());
			}

			if (!resinfo->get_loaded_image_file().empty()) {
				resinfo->SpriteWhenLoaded = CPlayerColorGraphic::New(resinfo->get_loaded_image_file().string(), type.get_frame_size(), type.get_conversible_player_color());
				resinfo->SpriteWhenLoaded->Load(false, wyrmgus::defines::get()->get_scale_factor());
			}
		}
	}

	if (!type.get_image_file().empty()) {
		type.Sprite = CPlayerColorGraphic::New(type.get_image_file().string(), type.get_frame_size(), type.get_conversible_player_color());
		type.Sprite->Load(false, wyrmgus::defines::get()->get_scale_factor());
	}

	//Wyrmgus start
	if (!type.LightFile.empty()) {
		type.LightSprite = CGraphic::New(type.LightFile, type.get_frame_size());
		type.LightSprite->Load(false, wyrmgus::defines::get()->get_scale_factor());
	}
	for (int i = 0; i < MaxImageLayers; ++i) {
		if (!type.LayerFiles[i].empty()) {
			type.LayerSprites[i] = CPlayerColorGraphic::New(type.LayerFiles[i], type.get_frame_size(), type.get_conversible_player_color());
			type.LayerSprites[i]->Load(false, wyrmgus::defines::get()->get_scale_factor());
		}
	}
	//Wyrmgus end

	//Wyrmgus start
	for (const auto &variation : type.get_variations()) {
		int frame_width = type.get_frame_size().width();
		int frame_height = type.get_frame_size().height();
		if (variation->FrameWidth && variation->FrameHeight) {
			frame_width = variation->FrameWidth;
			frame_height = variation->FrameHeight;
		}
		if (!variation->get_image_file().empty()) {
			variation->Sprite = CPlayerColorGraphic::New(variation->get_image_file().string(), QSize(frame_width, frame_height), type.get_conversible_player_color());
			variation->Sprite->Load(false, wyrmgus::defines::get()->get_scale_factor());
		}
		if (!variation->ShadowFile.empty()) {
			variation->ShadowSprite = CGraphic::New(variation->ShadowFile, type.ShadowWidth, type.ShadowHeight);
			variation->ShadowSprite->Load(false, wyrmgus::defines::get()->get_scale_factor());
		}
		if (!variation->LightFile.empty()) {
			variation->LightSprite = CGraphic::New(variation->LightFile, frame_width, frame_height);
			variation->LightSprite->Load(false, wyrmgus::defines::get()->get_scale_factor());
		}
		for (int j = 0; j < MaxImageLayers; ++j) {
			if (!variation->LayerFiles[j].empty()) {
				variation->LayerSprites[j] = CPlayerColorGraphic::New(variation->LayerFiles[j], QSize(frame_width, frame_height), type.get_conversible_player_color());
				variation->LayerSprites[j]->Load(false, wyrmgus::defines::get()->get_scale_factor());
			}
		}
	
		for (int j = 0; j < MaxCosts; ++j) {
			if (!variation->FileWhenLoaded[j].empty()) {
				variation->SpriteWhenLoaded[j] = CPlayerColorGraphic::New(variation->FileWhenLoaded[j], QSize(frame_width, frame_height), type.get_conversible_player_color());
				variation->SpriteWhenLoaded[j]->Load(false, wyrmgus::defines::get()->get_scale_factor());
			}
			if (!variation->FileWhenEmpty[j].empty()) {
				variation->SpriteWhenEmpty[j] = CPlayerColorGraphic::New(variation->FileWhenEmpty[j], QSize(frame_width, frame_height), type.get_conversible_player_color());
				variation->SpriteWhenEmpty[j]->Load(false, wyrmgus::defines::get()->get_scale_factor());
			}
		}
	}
	
	for (int i = 0; i < MaxImageLayers; ++i) {
		for (const auto &layer_variation : type.LayerVariations[i]) {
			if (!layer_variation->get_image_file().empty()) {
				layer_variation->Sprite = CPlayerColorGraphic::New(layer_variation->get_image_file().string(), type.get_frame_size(), type.get_conversible_player_color());
				layer_variation->Sprite->Load(false, wyrmgus::defines::get()->get_scale_factor());
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
	for (const wyrmgus::unit_type *unit_type : wyrmgus::unit_type::get_all()) {
		if (unit_type->Missile.IsEmpty() == false) count++;
		if (unit_type->FireMissile.IsEmpty() == false) count++;
		if (unit_type->Explosion.IsEmpty() == false) count++;

		if (!unit_type->Sprite) {
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
	for (wyrmgus::unit_type *unit_type : wyrmgus::unit_type::get_all()) {
		ShowLoadProgress(_("Loading Unit Types (%d%%)"), (unit_type->Slot + 1) * 100 / wyrmgus::unit_type::get_all().size());
		LoadUnitType(*unit_type);
	}
}

//Wyrmgus start
void LoadUnitType(wyrmgus::unit_type &type)
{
	// Lookup icons.
	if (!type.Icon.Name.empty()) {
		type.Icon.Load();
	}

	for (const auto &variation : type.get_variations()) {
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
	for (wyrmgus::unit_type *unit_type : wyrmgus::unit_type::get_all()) { // adjust array for unit already defined
		unit_type->BoolFlag.resize(new_size);
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
void CleanUnitTypeVariables()
{
	DebugPrint("FIXME: icon, sounds not freed.\n");

	// Clean all unit-types
	UnitTypeVar.Clear();
}

//Wyrmgus start
std::string GetUnitTypeStatsString(const std::string &unit_type_ident)
{
	const wyrmgus::unit_type *unit_type = wyrmgus::unit_type::get(unit_type_ident);

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
				|| var == HP_INDEX || var == MANA_INDEX || var == LEADERSHIPAURA_INDEX || var == REGENERATIONAURA_INDEX || var == HYDRATINGAURA_INDEX || var == ETHEREALVISION_INDEX || var == SPEEDBONUS_INDEX || var == SUPPLY_INDEX || var == TIMEEFFICIENCYBONUS_INDEX || var == RESEARCHSPEEDBONUS_INDEX || var == GARRISONEDRANGEBONUS_INDEX)
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
					unit_type_stats_string += std::to_string(unit_type->DefaultStat.Variables[var].Value);
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
