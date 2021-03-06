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
/**@name botpanel.cpp - The bottom panel. */
//
//      (c) Copyright 1999-2020 by Lutz Sammer, Vladi Belperchinov-Shabanski,
//		Jimmy Salmon, cybermind and Andrettin
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

#include "ui/ui.h"

#include "achievement.h"
#include "actions.h"
//Wyrmgus start
#include "action/action_research.h"
#include "action/action_train.h"
#include "action/action_upgradeto.h"
//Wyrmgus end
#include "campaign.h"
//Wyrmgus start
#include "character.h"
//Wyrmgus end
#include "civilization.h"
#include "commands.h"
#include "database/defines.h"
#include "dynasty.h"
#include "faction.h"
#include "game.h"
//Wyrmgus start
#include "grand_strategy.h"
//Wyrmgus end
#include "guichan/key.h"
#include "guichan/sdl/sdlinput.h"
#include "item/unique_item.h"
#include "map/map.h"
#include "map/map_layer.h"
#include "map/tile.h"
#include "map/tileset.h"
//Wyrmgus start
#include "network.h"
//Wyrmgus end
#include "player.h"
//Wyrmgus start
#include "quest.h"
//Wyrmgus end
#include "script/condition/condition.h"
#include "script/trigger.h"
#include "sound/sound.h"
#include "sound/unit_sound_type.h"
#include "spell/spell.h"
#include "translate.h"
#include "ui/button.h"
#include "ui/button_level.h"
#include "ui/cursor.h"
#include "ui/cursor_type.h"
#include "ui/interface.h"
#include "ui/popup.h"
#include "ui/resource_icon.h"
#include "unit/unit.h"
#include "unit/unit_class.h"
//Wyrmgus start
#include "unit/unit_manager.h"
//Wyrmgus end
#include "unit/unit_type.h"
#include "unit/unit_type_variation.h"
#include "upgrade/upgrade.h"
#include "upgrade/upgrade_class.h"
#include "util/vector_util.h"
#include "video/font.h"
#include "video/video.h"

/// Last drawn popup : used to speed up drawing
const wyrmgus::button *LastDrawnButtonPopup = nullptr;
/// for unit buttons sub-menus etc.
const wyrmgus::button_level *CurrentButtonLevel = nullptr;
/// Pointer to current buttons
std::vector<std::unique_ptr<wyrmgus::button>> CurrentButtons;

void InitButtons()
{
	// Resolve the icon names.
	for (wyrmgus::button *button : wyrmgus::button::get_all()) {
		if (!button->Icon.Name.empty()) {
			button->Icon.Load();
		}
	}
	CurrentButtons.clear();
}

/*----------------------------------------------------------------------------
--  Buttons structures
----------------------------------------------------------------------------*/

/**
**  Cleanup buttons.
*/
void CleanButtons()
{
	CurrentButtonLevel = nullptr;
	LastDrawnButtonPopup = nullptr;
	CurrentButtons.clear();
}

/**
**  Return Status of button.
**
**  @param button  button to check status
**  @param UnderCursor  Current Button Under Cursor
**
**  @return status of button
**  @return Icon(Active | Selected | Clicked | AutoCast | Disabled).
**
**  @todo FIXME : add IconDisabled when needed.
**  @todo FIXME : Should show the rally action for training unit ? (NewOrder)
*/
static int GetButtonStatus(const wyrmgus::button &button, int UnderCursor)
{
	unsigned int res = 0;

	/* parallel drawing */
	if (Selected.empty()) {
		return res;
	}

	// cursor is on that button
	if (ButtonAreaUnderCursor == ButtonAreaButton && UnderCursor == button.get_pos() - 1) {
		res |= IconActive;
		if (MouseButtons & LeftButton) {
			// Overwrite IconActive.
			res = IconClicked;
		}
	}

	//Wyrmgus start
	res |= IconCommandButton;
	
	if (button.Action == ButtonCmd::ProduceResource) {
		size_t i;
		for (i = 0; i < Selected.size(); ++i) {
			if (Selected[i]->GivesResource != button.Value) {
				break;
			}
		}
		if (i == Selected.size()) {
			res |= IconSelected;
		}
	}
	//Wyrmgus end
	
	UnitAction action = UnitAction::None;
	switch (button.Action) {
		case ButtonCmd::Stop:
			action = UnitAction::Still;
			break;
		case ButtonCmd::StandGround:
			action = UnitAction::StandGround;
			break;
		case ButtonCmd::Attack:
			action = UnitAction::Attack;
			break;
		case ButtonCmd::AttackGround:
			action = UnitAction::AttackGround;
			break;
		case ButtonCmd::Patrol:
			action = UnitAction::Patrol;
			break;
		case ButtonCmd::Harvest:
		case ButtonCmd::Return:
			action = UnitAction::Resource;
			break;
		default:
			break;
	}
	// Simple case.
	if (action != UnitAction::None) {
		for (size_t i = 0; i != Selected.size(); ++i) {
			if (Selected[i]->CurrentAction() != action) {
				return res;
			}
		}
		res |= IconSelected;
		return res;
	}
	// other cases : manage AutoCast and different possible action.
	size_t i;
	switch (button.Action) {
		case ButtonCmd::Move:
			for (i = 0; i < Selected.size(); ++i) {
				const UnitAction saction = Selected[i]->CurrentAction();
				if (saction != UnitAction::Move &&
					saction != UnitAction::Build &&
					saction != UnitAction::Follow &&
					//Wyrmgus start
					saction != UnitAction::PickUp &&
					//Wyrmgus end
					saction != UnitAction::Defend) {
					break;
				}
			}
			if (i == Selected.size()) {
				res |= IconSelected;
			}
			break;
		case ButtonCmd::SpellCast:
			// FIXME : and IconSelected ?

			// Autocast
			for (i = 0; i < Selected.size(); ++i) {
				if (Selected[i]->is_autocast_spell(wyrmgus::spell::get_all()[button.Value]) != true) {
					break;
				}
			}
			if (i == Selected.size()) {
				res |= IconAutoCast;
			}
			break;
		case ButtonCmd::Repair:
			for (i = 0; i < Selected.size(); ++i) {
				if (Selected[i]->CurrentAction() != UnitAction::Repair) {
					break;
				}
			}
			if (i == Selected.size()) {
				res |= IconSelected;
			}
			// Auto repair
			for (i = 0; i < Selected.size(); ++i) {
				if (Selected[i]->AutoRepair != 1) {
					break;
				}
			}
			if (i == Selected.size()) {
				res |= IconAutoCast;
			}
			break;
		// FIXME: must handle more actions
		case ButtonCmd::SellResource:
			if (std::find(CPlayer::GetThisPlayer()->AutosellResources.begin(), CPlayer::GetThisPlayer()->AutosellResources.end(), button.Value) != CPlayer::GetThisPlayer()->AutosellResources.end()) {
				res |= IconAutoCast;
			}
		default:
			break;
	}
	return res;
}

/**
**  Tell if we can show the popup content.
**  verify each sub condition for that.
**
**  @param condition   condition to verify.
**  @param unit        unit that certain condition can refer.
**
**  @return            0 if we can't show the content, else 1.
*/
static bool CanShowPopupContent(const PopupConditionPanel *condition,
								const wyrmgus::button &button,
								const wyrmgus::unit_type *type)
{
	if (!condition) {
		return true;
	}

	if (condition->HasHint && button.get_hint().empty()) {
		return false;
	}

	if (condition->HasDescription && button.Description.empty()) {
		return false;
	}

	if (condition->HasConditions && PrintConditions(button).empty()) {
		return false;
	}
	
	//Wyrmgus start
	if (condition->Class && type && type->get_unit_class() == nullptr && !(type->BoolFlag[ITEM_INDEX].value && type->get_item_class() != wyrmgus::item_class::none)) {
		return false;
	}
	
	if (condition->unit_class != nullptr) {
		if (!type || condition->unit_class != type->get_unit_class()) {
			return false;
		}
	}

	if (condition->UnitTypeType != UnitTypeType::None) {
		if (!type || condition->UnitTypeType != type->UnitType) {
			return false;
		}
	}

	if (condition->CanStore != -1) {
		if (!type || !type->CanStore[condition->CanStore]) {
			return false;
		}
	}

	if (condition->ImproveIncome != -1) {
		if (!type || type->Stats[CPlayer::GetThisPlayer()->Index].ImproveIncomes[condition->ImproveIncome] <= wyrmgus::resource::get_all()[condition->ImproveIncome]->get_default_income()) {
			return false;
		}
	}

	if (condition->ChildResources != CONDITION_TRUE) {
		if ((condition->ChildResources == CONDITION_ONLY) ^ (wyrmgus::resource::get_all()[button.Value]->ChildResources.size() > 0)) {
			return false;
		}
	}

	if (condition->ImproveIncomes != CONDITION_TRUE) {
		bool improve_incomes = false;
		if (button.Action == ButtonCmd::ProduceResource) {
			if (CPlayer::GetThisPlayer()->Incomes[button.Value] > wyrmgus::resource::get_all()[button.Value]->get_default_income()) {
				improve_incomes = true;
			}
			for (const wyrmgus::resource *child_resource : wyrmgus::resource::get_all()[button.Value]->ChildResources) {
				if (CPlayer::GetThisPlayer()->Incomes[child_resource->get_index()] > child_resource->get_default_income()) {
					improve_incomes = true;
					break;
				}
			}
		} else {
			if (!type) {
				return false;
			}
			for (int i = 1; i < MaxCosts; ++i) {
				if (type->Stats[CPlayer::GetThisPlayer()->Index].ImproveIncomes[i] > wyrmgus::resource::get_all()[i]->get_default_income()) {
					improve_incomes = true;
					break;
				}
			}
		}
		if ((condition->ImproveIncomes == CONDITION_ONLY) ^ improve_incomes) {
			return false;
		}
	}

	if (condition->Description && type && type->get_description().empty()) {
		return false;
	}
	
	if (condition->Quote && type && type->get_quote().empty() && !((button.Action == ButtonCmd::Unit || button.Action == ButtonCmd::Buy) && UnitManager.GetSlotUnit(button.Value).get_unique() != nullptr && !UnitManager.GetSlotUnit(button.Value).get_unique()->get_quote().empty()) && !((button.Action == ButtonCmd::Unit || button.Action == ButtonCmd::Buy) && UnitManager.GetSlotUnit(button.Value).Work != nullptr && !UnitManager.GetSlotUnit(button.Value).Work->get_quote().empty() && UnitManager.GetSlotUnit(button.Value).Elixir != nullptr && !UnitManager.GetSlotUnit(button.Value).Elixir->get_quote().empty())) {
		return false;
	}
	
	if (condition->Encyclopedia && type && type->get_description().empty() && type->get_background().empty() && type->get_quote().empty() && (!type->BoolFlag[ITEM_INDEX].value || type->get_item_class() == wyrmgus::item_class::none)) {
		return false;
	}
	
	if (condition->settlement_name && !(button.Action == ButtonCmd::Unit && UnitManager.GetSlotUnit(button.Value).settlement)) {
		return false;
	}
	
	if (condition->CanActiveHarvest && !(button.Action == ButtonCmd::Unit && Selected.size() > 0 && Selected[0]->CanHarvest(&UnitManager.GetSlotUnit(button.Value), false))) {
		return false;
	}

	if (condition->FactionUpgrade != CONDITION_TRUE) {
		if ((condition->FactionUpgrade == CONDITION_ONLY) ^ (button.Action == ButtonCmd::Faction)) {
			return false;
		}
	}
	
	if (condition->DynastyUpgrade != CONDITION_TRUE) {
		if ((condition->DynastyUpgrade == CONDITION_ONLY) ^ (button.Action == ButtonCmd::Dynasty)) {
			return false;
		}
	}
	
	if (condition->FactionCoreSettlements != CONDITION_TRUE) {
		if ((condition->FactionCoreSettlements == CONDITION_ONLY) ^ (wyrmgus::game::get()->get_current_campaign() != nullptr && button.Action == ButtonCmd::Faction && button.Value != -1 && wyrmgus::faction::get_all()[CPlayer::GetThisPlayer()->Faction]->DevelopsTo[button.Value]->get_core_settlements().size() > 0)) {
			return false;
		}
	}
	
	const CUpgrade *upgrade = nullptr;
	if (button.Action == ButtonCmd::Research || button.Action == ButtonCmd::ResearchClass || button.Action == ButtonCmd::LearnAbility) {
		upgrade = button.get_value_upgrade(Selected[0]);
	} else if (button.Action == ButtonCmd::Faction && !CPlayer::GetThisPlayer()->get_faction()->DevelopsTo[button.Value]->FactionUpgrade.empty()) {
		upgrade = CUpgrade::get(CPlayer::GetThisPlayer()->get_faction()->DevelopsTo[button.Value]->FactionUpgrade);
	} else if (button.Action == ButtonCmd::Dynasty && CPlayer::GetThisPlayer()->get_faction()->get_dynasties()[button.Value]->get_upgrade() != nullptr) {
		upgrade = CPlayer::GetThisPlayer()->get_faction()->get_dynasties()[button.Value]->get_upgrade();
	}
	
	if (condition->UpgradeResearched != CONDITION_TRUE) {
		if ((condition->UpgradeResearched == CONDITION_ONLY) ^ ((((button.Action == ButtonCmd::Research || button.Action == ButtonCmd::ResearchClass || button.Action == ButtonCmd::Faction || button.Action == ButtonCmd::Dynasty) && UpgradeIdAllowed(*CPlayer::GetThisPlayer(), upgrade->ID) == 'R') || (button.Action == ButtonCmd::LearnAbility && Selected[0]->GetIndividualUpgrade(upgrade) >= upgrade->MaxLimit)))) {
			return false;
		}
	}
	
	if (condition->ResearchedUpgrade) {
		if (UpgradeIdAllowed(*CPlayer::GetThisPlayer(), condition->ResearchedUpgrade->ID) != 'R') {
			return false;
		}
	}
	
	if (condition->researched_upgrade_class != nullptr) {
		if (!CPlayer::GetThisPlayer()->has_upgrade_class(condition->researched_upgrade_class)) {
			return false;
		}
	}
	
	if (condition->Ability != CONDITION_TRUE) {
		if ((condition->Ability == CONDITION_ONLY) ^ (upgrade && upgrade->is_ability())) {
			return false;
		}
	}
	
	if (condition->LuxuryResource != CONDITION_TRUE) {
		if ((condition->LuxuryResource == CONDITION_ONLY) ^ (button.Action == ButtonCmd::ProduceResource && wyrmgus::resource::get_all()[button.Value]->LuxuryResource)) {
			return false;
		}
	}
	
	if (condition->RequirementsString != CONDITION_TRUE) {
		if ((condition->RequirementsString == CONDITION_ONLY) ^ ((button.Action == ButtonCmd::Research || button.Action == ButtonCmd::ResearchClass || button.Action == ButtonCmd::LearnAbility || button.Action == ButtonCmd::Faction || button.Action == ButtonCmd::Dynasty || button.Action == ButtonCmd::Train || button.Action == ButtonCmd::TrainClass || button.Action == ButtonCmd::Build || button.Action == ButtonCmd::BuildClass || button.Action == ButtonCmd::UpgradeTo || button.Action == ButtonCmd::UpgradeToClass || button.Action == ButtonCmd::Buy) && !IsButtonUsable(*Selected[0], button) && Selected[0]->Player == CPlayer::GetThisPlayer() && ((type && !type->RequirementsString.empty()) ||  ((button.Action == ButtonCmd::Research || button.Action == ButtonCmd::ResearchClass || button.Action == ButtonCmd::LearnAbility || button.Action == ButtonCmd::Faction || button.Action == ButtonCmd::Dynasty) && !upgrade->get_requirements_string().empty())))) {
			return false;
		}
	}
	
	if (condition->ExperienceRequirementsString != CONDITION_TRUE) {
		if ((condition->ExperienceRequirementsString == CONDITION_ONLY) ^ (button.Action == ButtonCmd::ExperienceUpgradeTo && !IsButtonUsable(*Selected[0], button) && type && !type->ExperienceRequirementsString.empty())) {
			return false;
		}
	}
	
	if (condition->BuildingRulesString != CONDITION_TRUE) {
		if ((condition->BuildingRulesString == CONDITION_ONLY) ^ ((button.Action == ButtonCmd::Build || button.Action == ButtonCmd::BuildClass) && type && !type->BuildingRulesString.empty())) {
			return false;
		}
	}
	//Wyrmgus end

	if (condition->Overlord != CONDITION_TRUE) {
		if ((condition->Overlord == CONDITION_ONLY) ^ (CPlayer::Players[button.Value]->get_overlord() != nullptr)) {
			return false;
		}
	}

	if (condition->TopOverlord != CONDITION_TRUE) {
		if ((condition->TopOverlord == CONDITION_ONLY) ^ (CPlayer::Players[button.Value]->get_overlord() != nullptr && CPlayer::Players[button.Value]->get_top_overlord() != CPlayer::Players[button.Value]->get_overlord())) {
			return false;
		}
	}

	if (condition->ButtonAction != ButtonCmd::None && button.Action != condition->ButtonAction) {
		return false;
	}

	if (condition->ButtonValue.empty() == false && button.ValueStr != condition->ButtonValue) {
		return false;
	}

	if (type && condition->BoolFlags && !type->CheckUserBoolFlags(condition->BoolFlags.get())) {
		return false;
	}

	//Wyrmgus start
//	if (condition->Variables && type) {
	if (condition->Variables && type && button.Action != ButtonCmd::Unit && button.Action != ButtonCmd::Buy) {
	//Wyrmgus end
		for (unsigned int i = 0; i < UnitTypeVar.GetNumberVariable(); ++i) {
			if (condition->Variables[i] != CONDITION_TRUE) {
				if ((condition->Variables[i] == CONDITION_ONLY) ^ type->Stats[CPlayer::GetThisPlayer()->Index].Variables[i].Enable) {
					return false;
				}
			}
		}
	//Wyrmgus start
	} else if (condition->Variables && (button.Action == ButtonCmd::Unit || button.Action == ButtonCmd::Buy)) {
		for (unsigned int i = 0; i < UnitTypeVar.GetNumberVariable(); ++i) {
			if (condition->Variables[i] != CONDITION_TRUE) {
//				if ((condition->Variables[i] == CONDITION_ONLY) ^ UnitManager.GetSlotUnit(button.Value).Variable[i].Enable) {
				CUnit &unit = UnitManager.GetSlotUnit(button.Value);
				if (unit.Type->BoolFlag[ITEM_INDEX].value && unit.Container != nullptr && unit.Container->HasInventory()) {
					if (i == BASICDAMAGE_INDEX) {
						if ((condition->Variables[i] == CONDITION_ONLY) ^ (unit.Container->GetItemVariableChange(&unit, i) != 0 || unit.Container->GetItemVariableChange(&unit, PIERCINGDAMAGE_INDEX) != 0)) {
							return false;
						}
					} else {
						if ((condition->Variables[i] == CONDITION_ONLY) ^ (unit.Container->GetItemVariableChange(&unit, i) != 0)) { //the former for some reason wasn't working with negative values
							return false;
						}
					}
				} else if (unit.Work || unit.Elixir) { //special case for literary works and elixirs that aren't in an inventory
					if (i == BASICDAMAGE_INDEX) {
						if ((condition->Variables[i] == CONDITION_ONLY) ^ (unit.GetItemVariableChange(&unit, i) != 0 || unit.GetItemVariableChange(&unit, PIERCINGDAMAGE_INDEX) != 0)) {
							return false;
						}
					} else {
						if ((condition->Variables[i] == CONDITION_ONLY) ^ (unit.GetItemVariableChange(&unit, i) != 0)) { //the former for some reason wasn't working with negative values
							return false;
						}
					}
				} else {
					if (i == BASICDAMAGE_INDEX) {
						if ((condition->Variables[i] == CONDITION_ONLY) ^ (unit.Variable[i].Value != 0 || unit.Variable[PIERCINGDAMAGE_INDEX].Value != 0)) {
							return false;
						}
					} else {
						if ((condition->Variables[i] == CONDITION_ONLY) ^ (unit.Variable[i].Value != 0)) { //the former for some reason wasn't working with negative values
							return false;
						}
					}
				}
			}
		}
	//Wyrmgus end
	}
	
	//Wyrmgus start
	if (button.Action == ButtonCmd::SpellCast) {
		if (condition->AutoCast != CONDITION_TRUE) {
			if ((condition->AutoCast == CONDITION_ONLY) ^ (wyrmgus::spell::get_all()[button.Value]->get_autocast_info() != nullptr)) {
				return false;
			}
		}
	}
		
	if (button.Action == ButtonCmd::Unit || button.Action == ButtonCmd::Buy) {
		CUnit &unit = UnitManager.GetSlotUnit(button.Value);
		if (unit.Type->BoolFlag[ITEM_INDEX].value) {
			if (condition->Equipped != CONDITION_TRUE) {
				if ((condition->Equipped == CONDITION_ONLY) ^ (unit.Container != nullptr && unit.Container->HasInventory() && unit.Container->IsItemEquipped(&unit))) {
					return false;
				}
			}
			if (condition->Equippable != CONDITION_TRUE) {
				if ((condition->Equippable == CONDITION_ONLY) ^ (unit.Container != nullptr && unit.Container->HasInventory() && unit.Container->can_equip_item(&unit))) {
					return false;
				}
			}
			if (condition->Consumable != CONDITION_TRUE) {
				if ((condition->Consumable == CONDITION_ONLY) ^ wyrmgus::is_consumable_item_class(unit.Type->get_item_class())) {
					return false;
				}
			}
			if (condition->Spell != CONDITION_TRUE) {
				if ((condition->Spell == CONDITION_ONLY) ^ (unit.Spell != nullptr)) {
					return false;
				}
			}
			if (condition->Work != CONDITION_TRUE) {
				if ((condition->Work == CONDITION_ONLY) ^ (unit.Work != nullptr)) {
					return false;
				}
			}
			if (condition->ReadWork != CONDITION_TRUE) {
				if ((condition->ReadWork == CONDITION_ONLY) ^ (unit.Work != nullptr && (unit.Container != nullptr && unit.Container->HasInventory() && unit.Container->GetIndividualUpgrade(unit.Work)))) {
					return false;
				}
			}
			if (condition->Elixir != CONDITION_TRUE) {
				if ((condition->Elixir == CONDITION_ONLY) ^ (unit.Elixir != nullptr)) {
					return false;
				}
			}
			if (condition->ConsumedElixir != CONDITION_TRUE) {
				if ((condition->ConsumedElixir == CONDITION_ONLY) ^ (unit.Elixir != nullptr && (unit.Container != nullptr && unit.Container->HasInventory() && unit.Container->GetIndividualUpgrade(unit.Elixir)))) {
					return false;
				}
			}
			if (condition->CanUse != CONDITION_TRUE) {
				if ((condition->CanUse == CONDITION_ONLY) ^ (unit.Container != nullptr && unit.Container->HasInventory() && unit.Container->CanUseItem(&unit))) {
					return false;
				}
			}
			if (condition->Bound != CONDITION_TRUE) {
				if ((condition->Bound == CONDITION_ONLY) ^ unit.Bound) {
					return false;
				}
			}
			if (condition->Identified != CONDITION_TRUE) {
				if ((condition->Identified == CONDITION_ONLY) ^ unit.Identified) {
					return false;
				}
			}
			if (condition->Weapon != CONDITION_TRUE) {
				if ((condition->Weapon == CONDITION_ONLY) ^ (wyrmgus::get_item_class_slot(unit.Type->get_item_class()) == wyrmgus::item_slot::weapon)) {
					return false;
				}
			}
			if (condition->Shield != CONDITION_TRUE) {
				if ((condition->Shield == CONDITION_ONLY) ^ (wyrmgus::get_item_class_slot(unit.Type->get_item_class()) == wyrmgus::item_slot::shield)) {
					return false;
				}
			}
			if (condition->Boots != CONDITION_TRUE) {
				if ((condition->Boots == CONDITION_ONLY) ^ (wyrmgus::get_item_class_slot(unit.Type->get_item_class()) == wyrmgus::item_slot::boots)) {
					return false;
				}
			}
			if (condition->Arrows != CONDITION_TRUE) {
				if ((condition->Arrows == CONDITION_ONLY) ^ (wyrmgus::get_item_class_slot(unit.Type->get_item_class()) == wyrmgus::item_slot::arrows)) {
					return false;
				}
			}
			if (condition->item_class != wyrmgus::item_class::none) {
				if (condition->item_class != unit.Type->get_item_class()) {
					return false;
				}
			}
			if (condition->Regeneration != CONDITION_TRUE) {
				if (unit.Container != nullptr && unit.Container->HasInventory()) {
					if ((condition->Regeneration == CONDITION_ONLY) ^ (unit.Container->GetItemVariableChange(&unit, HITPOINTBONUS_INDEX, true) != 0)) {
						return false;
					}
				} else if (unit.Work || unit.Elixir) { //special case for literary works and elixirs that aren't in an inventory
					if ((condition->Regeneration == CONDITION_ONLY) ^ (unit.GetItemVariableChange(&unit, HP_INDEX, true) != 0 || unit.GetItemVariableChange(&unit, HITPOINTBONUS_INDEX, true) != 0)) {
						return false;
					}
				} else {
					if ((condition->Regeneration == CONDITION_ONLY) ^ (unit.Variable[HP_INDEX].Increase != 0 || unit.Variable[HITPOINTBONUS_INDEX].Increase != 0)) {
						return false;
					}
				}
			}
		}
		
		if (!(button.Action == ButtonCmd::Buy && unit.Character) && condition->Opponent != CONDITION_TRUE) {
			if ((condition->Opponent == CONDITION_ONLY) ^ CPlayer::GetThisPlayer()->IsEnemy(unit)) {
				return false;
			}
		}
		if (!(button.Action == ButtonCmd::Buy && unit.Character) && condition->Neutral != CONDITION_TRUE) {
			if ((condition->Neutral == CONDITION_ONLY) ^ (!CPlayer::GetThisPlayer()->IsEnemy(unit) && !CPlayer::GetThisPlayer()->IsAllied(unit) && CPlayer::GetThisPlayer() != unit.Player && (unit.Container == nullptr || (!CPlayer::GetThisPlayer()->IsEnemy(*unit.Container) && !CPlayer::GetThisPlayer()->IsAllied(*unit.Container) && CPlayer::GetThisPlayer() != unit.Container->Player)))) {
				return false;
			}
		}
		
		if (condition->Affixed != CONDITION_TRUE) {
			if ((condition->Affixed == CONDITION_ONLY) ^ (unit.Prefix != nullptr || unit.Suffix != nullptr)) {
				return false;
			}
		}
		if (condition->Unique != CONDITION_TRUE) {
			if ((condition->Unique == CONDITION_ONLY) ^ (unit.get_unique() != nullptr || unit.Character != nullptr)) {
				return false;
			}
		}
		if (condition->UniqueSet != CONDITION_TRUE) {
			if ((condition->UniqueSet == CONDITION_ONLY) ^ (unit.get_unique() != nullptr && unit.get_unique()->get_set() != nullptr)) {
				return false;
			}
		}
	} else { // always return false for "Affixed" and "Unique" for buttons that aren't individual unit buttons
		if (condition->Affixed != CONDITION_TRUE) {
			if (condition->Affixed == CONDITION_ONLY) {
				return false;
			}
		}
		if (condition->Unique != CONDITION_TRUE) {
			if (condition->Unique == CONDITION_ONLY) {
				return false;
			}
		}
	}
	//Wyrmgus end
	
	return true;
}

static void GetPopupSize(const CPopup &popup, const wyrmgus::button &button,
						 int &popupWidth, int &popupHeight, int *Costs)
{
	int contentWidth = popup.MarginX;
	int contentHeight = 0;
	int maxContentWidth = 0;
	int maxContentHeight = 0;
	popupWidth = popup.MarginX;
	popupHeight = popup.MarginY;

	const wyrmgus::unit_type *unit_type = button.get_unit_type();

	for (const std::unique_ptr<CPopupContentType> &content : popup.Contents) {
		if (CanShowPopupContent(content->Condition.get(), button, unit_type)) {
			// Automatically write the calculated coordinates.
			content->pos.x = contentWidth + content->MarginX;
			content->pos.y = popupHeight + content->MarginY;

			contentWidth += std::max(content->minSize.x, 2 * content->MarginX + content->GetWidth(button, Costs));
			contentHeight = std::max(content->minSize.y, 2 * content->MarginY + content->GetHeight(button, Costs));
			maxContentHeight = std::max(contentHeight, maxContentHeight);
			if (content->Wrap) {
				popupWidth += std::max(0, contentWidth - maxContentWidth);
				popupHeight += maxContentHeight;
				maxContentWidth = std::max(maxContentWidth, contentWidth);
				contentWidth = popup.MarginX;
				maxContentHeight = 0;
			}
		}
	}

	popupWidth += popup.MarginX;
	popupHeight += popup.MarginY;
}

static struct PopupDrawCache {
	int popupWidth;
	int popupHeight;
} popupCache;

/**
**  Draw popup
*/
void DrawPopup(const wyrmgus::button &button, int x, int y, bool above)
{
	CPopup *popup = PopupByIdent(button.Popup);
	bool useCache = false;
	const int scale_factor = wyrmgus::defines::get()->get_scale_factor();

	if (!popup) {
		return;
	} else if (&button == LastDrawnButtonPopup) {
		useCache = true;
	} else {
		LastDrawnButtonPopup = &button;
	}

	int popupWidth, popupHeight;
	int Costs[ManaResCost + 1];
	memset(Costs, 0, sizeof(Costs));

	const wyrmgus::unit_type *unit_type = button.get_unit_type();
	const CUpgrade *upgrade = button.get_value_upgrade();

	int type_costs[MaxCosts];

	switch (button.Action) {
		case ButtonCmd::Research:
		case ButtonCmd::ResearchClass:
		case ButtonCmd::Dynasty:
			CPlayer::GetThisPlayer()->GetUpgradeCosts(upgrade, Costs);
			break;
		case ButtonCmd::SpellCast:
			memcpy(Costs, wyrmgus::spell::get_all()[button.Value]->Costs, sizeof(wyrmgus::spell::get_all()[button.Value]->Costs));
			Costs[ManaResCost] = wyrmgus::spell::get_all()[button.Value]->get_mana_cost();
			break;
		case ButtonCmd::Build:
		case ButtonCmd::BuildClass:
		case ButtonCmd::Train:
		case ButtonCmd::TrainClass:
		case ButtonCmd::UpgradeTo:
		case ButtonCmd::UpgradeToClass:
			CPlayer::GetThisPlayer()->GetUnitTypeCosts(unit_type, type_costs, Selected[0]->Type->Stats[Selected[0]->Player->Index].GetUnitStock(unit_type) != 0);
			memcpy(Costs, type_costs, sizeof(type_costs));
			Costs[FoodCost] = unit_type->Stats[CPlayer::GetThisPlayer()->Index].Variables[DEMAND_INDEX].Value;
			break;
		case ButtonCmd::Buy:
			Costs[FoodCost] = UnitManager.GetSlotUnit(button.Value).Variable[DEMAND_INDEX].Value;
			Costs[CopperCost] = UnitManager.GetSlotUnit(button.Value).GetPrice();
			break;
		default:
			break;
	}

	if (useCache) {
		popupWidth = popupCache.popupWidth;
		popupHeight = popupCache.popupHeight;
	} else {
		GetPopupSize(*popup, button, popupWidth, popupHeight, Costs);
		popupWidth = std::max(popupWidth, popup->MinWidth);
		popupHeight = std::max(popupHeight, popup->MinHeight);
		popupCache.popupWidth = popupWidth;
		popupCache.popupHeight = popupHeight;
	}

	x = std::min<int>(x, Video.Width - 1 - popupWidth);
	clamp<int>(&x, 0, Video.Width - 1);
	if (above) {
		y = y - popupHeight - 10 * scale_factor;
	} else { //below
		y = y + 10 * scale_factor;
	}
	clamp<int>(&y, 0, Video.Height - 1);

	// Background
	Video.FillTransRectangle(popup->BackgroundColor, x, y, popupWidth, popupHeight, popup->BackgroundColor >> ASHIFT);
	Video.DrawRectangle(popup->BorderColor, x, y, popupWidth, popupHeight);

	// Contents
	for (const std::unique_ptr<CPopupContentType> &content : popup->Contents) {
		if (CanShowPopupContent(content->Condition.get(), button, unit_type)) {
			content->Draw(x + content->pos.x, y + content->pos.y, *popup, popupWidth, button, Costs);
		}
	}
}

//Wyrmgus start
/**
**  Draw popup
*/
void DrawGenericPopup(const std::string &popup_text, int x, int y, const wyrmgus::font_color *text_color, const wyrmgus::font_color *highlight_color, bool above)
{
	const wyrmgus::font *font = wyrmgus::defines::get()->get_game_font();
	
	const int scale_factor = wyrmgus::defines::get()->get_scale_factor();
	int MaxWidth = std::max(512 * scale_factor, Video.Width / 5);

	int i;
		
	//calculate content width
	int content_width = 0;
	std::string content_width_sub;
	i = 1;
	while (!(content_width_sub = GetLineFont(i++, popup_text, 0, font)).empty()) {
		int line_width = font->getWidth(content_width_sub);
		int cost_symbol_pos = content_width_sub.find("COST_", 0);
		if (cost_symbol_pos != std::string::npos) {
			const int res = std::stoi(content_width_sub.substr(cost_symbol_pos + 5, content_width_sub.find(" ", cost_symbol_pos) - (cost_symbol_pos + 5) + 1));
			line_width -= font->getWidth("COST_" + std::to_string(res));
			const wyrmgus::resource *resource = wyrmgus::resource::get_all()[res];
			line_width += resource->get_icon()->get_graphics()->Width;
		}
		content_width = std::max(content_width, line_width);
	}
	
	if (MaxWidth) {
		content_width = std::min(content_width, MaxWidth);
	}
	
	//calculate content height
	int content_height = 0;
	i = 1;
	while ((GetLineFont(i++, popup_text, MaxWidth, font)).length()) {
		content_height += font->Height() + 2 * scale_factor;
	}
	
	int popupWidth, popupHeight;

	int contentWidth = MARGIN_X * scale_factor;
	int contentHeight = 0;
	int maxContentWidth = 0;
	int maxContentHeight = 0;
	popupWidth = MARGIN_X * scale_factor;
	popupHeight = MARGIN_Y * scale_factor;
	PixelPos pos(0, 0);
	
	bool wrap = true;

	// Automatically write the calculated coordinates.
	pos.x = contentWidth + MARGIN_X * scale_factor;
	pos.y = popupHeight + MARGIN_Y * scale_factor;

	contentWidth += std::max(0, 2 * MARGIN_X * scale_factor + content_width);
	contentHeight = std::max(0, 2 * MARGIN_Y * scale_factor + content_height);
	maxContentHeight = std::max(contentHeight, maxContentHeight);
	if (wrap) {
		popupWidth += std::max(0, contentWidth - maxContentWidth);
		popupHeight += maxContentHeight;
		maxContentWidth = std::max(maxContentWidth, contentWidth);
		contentWidth = MARGIN_X * scale_factor;
		maxContentHeight = 0;
	}

	popupWidth += MARGIN_X * scale_factor;
	popupHeight += MARGIN_Y * scale_factor;
	
	popupWidth = std::min(popupWidth, MaxWidth);
	popupHeight = std::max(popupHeight, 0);

	x = std::min<int>(x, Video.Width - 1 - popupWidth);
	clamp<int>(&x, 0, Video.Width - 1);
	if (above) {
		y = y - popupHeight - 10 * scale_factor;
	} else { //below
		y = y + 10 * scale_factor;
	}
	clamp<int>(&y, 0, Video.Height - 1);

	// Background
	const IntColor BackgroundColor = CVideo::MapRGBA(28, 28, 28, 208);
	const IntColor BorderColor = CVideo::MapRGBA(93, 93, 93, 160);

	Video.FillTransRectangle(BackgroundColor, x, y, popupWidth, popupHeight, BackgroundColor >> ASHIFT);
	Video.DrawRectangle(BorderColor, x, y, popupWidth, popupHeight);

	if (text_color == nullptr) {
		text_color = wyrmgus::defines::get()->get_default_font_color();
	}
	if (highlight_color == nullptr) {
		highlight_color = wyrmgus::defines::get()->get_default_highlight_font_color();
	}
	
	// Contents
	x += pos.x;
	y += pos.y;
	CLabel label(font, text_color, highlight_color);
	std::string sub;
	i = 0;
	int y_off = y;
	unsigned int width = MaxWidth
						 ? std::min(MaxWidth, popupWidth - 2 * MARGIN_X * scale_factor)
						 : 0;
	while ((sub = GetLineFont(++i, popup_text, width, font)).length()) {
		if (sub.find("LINE", 0) != std::string::npos) {
			Video.FillRectangle(BorderColor, x + (-MARGIN_X + 1 - MARGIN_X) * scale_factor,
								y_off, popupWidth - 2 * scale_factor, 1);
			sub = sub.substr(sub.find("LINE", 0) + 4, sub.length());
		}
		int cost_symbol_pos = sub.find("COST_", 0);
		if (cost_symbol_pos != std::string::npos) {
			int x_offset = 0;
			const int res = std::stoi(sub.substr(cost_symbol_pos + 5, sub.find(" ", cost_symbol_pos) - (cost_symbol_pos + 5) + 1));
			std::string sub_first = sub.substr(0, cost_symbol_pos);
			std::string sub_second = sub.substr(cost_symbol_pos + 5 + std::to_string(res).length(), sub.length() - cost_symbol_pos - (5 + std::to_string(res).length()));
			label.Draw(x, y_off, sub_first);
			x_offset += font->getWidth(sub_first);
			const wyrmgus::resource *resource = wyrmgus::resource::get_all()[res];
			const wyrmgus::resource_icon *icon = resource->get_icon();
			const std::shared_ptr<CGraphic> &icon_graphics = icon->get_graphics();
			icon_graphics->DrawFrameClip(icon->get_frame(), x + x_offset, y + ((font->getHeight() - icon_graphics->Height) / 2), nullptr);
			x_offset += icon_graphics->Width;
			label.Draw(x + x_offset, y_off, sub_second);
		} else {
			label.Draw(x, y_off, sub);
		}
		y_off += font->Height() + 2 * scale_factor;
	}
}
//Wyrmgus end

/**
**  Draw button panel.
**
**  Draw all action buttons.
*/
void CButtonPanel::Draw()
{
	//  Draw background
	if (UI.ButtonPanel.G) {
		UI.ButtonPanel.G->DrawSubClip(0, 0,
									  UI.ButtonPanel.G->Width, UI.ButtonPanel.G->Height,
									  UI.ButtonPanel.X, UI.ButtonPanel.Y);
	}

	// No buttons
	if (CurrentButtons.empty()) {
		return;
	}
	const std::vector<std::unique_ptr<wyrmgus::button>> &buttons(CurrentButtons);

	Assert(!Selected.empty());
	char buf[8];

	//  Draw all buttons.
	for (size_t i = 0; i < buttons.size(); ++i) {
		const std::unique_ptr<wyrmgus::button> &button = buttons[i];

		if (button->get_pos() == -1) {
			continue;
		}

		Assert(button->get_pos() == i + 1);

		//Wyrmgus start
		//for neutral units, don't draw buttons that aren't training buttons (in other words, only draw buttons which are usable by neutral buildings)
		if (
			!button->is_always_shown()
			&& Selected[0]->Player != CPlayer::GetThisPlayer()
			&& !CPlayer::GetThisPlayer()->IsTeamed(*Selected[0])
			&& CPlayer::GetThisPlayer()->HasBuildingAccess(*Selected[0]->Player, button->Action)
			&& !IsNeutralUsableButtonAction(button->Action)
		) {
			continue;
		}
		//Wyrmgus end
		bool gray = false;
		bool cooldownSpell = false;
		int maxCooldown = 0;
		for (size_t j = 0; j != Selected.size(); ++j) {
			if (!IsButtonAllowed(*Selected[j], *button)) {
				gray = true;
				break;
			} else if (button->Action == ButtonCmd::SpellCast
					   && (*Selected[j]).SpellCoolDownTimers[wyrmgus::spell::get_all()[button->Value]->Slot]) {
				Assert(wyrmgus::spell::get_all()[button->Value]->get_cooldown() > 0);
				cooldownSpell = true;
				maxCooldown = std::max(maxCooldown, (*Selected[j]).SpellCoolDownTimers[wyrmgus::spell::get_all()[button->Value]->Slot]);
			}
		}
		//
		//  Tutorial show command key in icons
		//
		if (ShowCommandKey) {
			const int key = button->get_key();
			if (key == gcn::Key::K_ESCAPE) {
				strcpy_s(buf, sizeof(buf), "Esc");
			} else if (key == gcn::Key::K_PAGE_UP) {
				strcpy_s(buf, sizeof(buf), "PgUp");
			} else if (key == gcn::Key::K_PAGE_DOWN) {
				strcpy_s(buf, sizeof(buf), "PgDwn");
			} else if (key == gcn::Key::K_DELETE) {
				strcpy_s(buf, sizeof(buf), "Del");
			} else {
				buf[0] = toupper(key);
				buf[1] = '\0';
			}
		} else {
			buf[0] = '\0';
		}

		//
		// Draw main Icon.
		//
		const PixelPos pos(UI.ButtonPanel.Buttons[i].X, UI.ButtonPanel.Buttons[i].Y);

		//Wyrmgus start
		const wyrmgus::icon *button_icon = button->Icon.Icon;

		const wyrmgus::unit_type *button_unit_type = button->get_value_unit_type(Selected[0]);
		const CUpgrade *button_upgrade = button->get_value_upgrade(Selected[0]);
			
		// if there is a single unit selected, show the icon of its weapon/shield/boots/arrows equipped for the appropriate buttons
		if (button->Icon.Name.empty() && button->Action == ButtonCmd::Attack && Selected[0]->Type->CanTransport() && Selected[0]->Type->BoolFlag[ATTACKFROMTRANSPORTER_INDEX].value && Selected[0]->BoardCount > 0 && Selected[0]->UnitInside != nullptr && Selected[0]->UnitInside->Type->BoolFlag[ATTACKFROMTRANSPORTER_INDEX].value && Selected[0]->UnitInside->GetButtonIcon(button->Action) != nullptr) {
			button_icon = Selected[0]->UnitInside->GetButtonIcon(button->Action);
		} else if (button->Icon.Name.empty() && Selected[0]->GetButtonIcon(button->Action) != nullptr) {
			button_icon = Selected[0]->GetButtonIcon(button->Action);
		} else if (button->Action == ButtonCmd::ExperienceUpgradeTo && Selected[0]->GetVariation() && button_unit_type->GetVariation(Selected[0]->GetVariation()->get_identifier()) != nullptr && !button_unit_type->GetVariation(Selected[0]->GetVariation()->get_identifier())->Icon.Name.empty()) {
			button_icon = button_unit_type->GetVariation(Selected[0]->GetVariation()->get_identifier())->Icon.Icon;
		} else if ((button->Action == ButtonCmd::Train || button->Action == ButtonCmd::TrainClass || button->Action == ButtonCmd::Build || button->Action == ButtonCmd::BuildClass || button->Action == ButtonCmd::UpgradeTo || button->Action == ButtonCmd::UpgradeToClass || button->Action == ButtonCmd::ExperienceUpgradeTo) && button->Icon.Name.empty() && button_unit_type->GetDefaultVariation(CPlayer::GetThisPlayer()) != nullptr && !button_unit_type->GetDefaultVariation(CPlayer::GetThisPlayer())->Icon.Name.empty()) {
			button_icon = button_unit_type->GetDefaultVariation(CPlayer::GetThisPlayer())->Icon.Icon;
		} else if ((button->Action == ButtonCmd::Train || button->Action == ButtonCmd::TrainClass || button->Action == ButtonCmd::Build || button->Action == ButtonCmd::BuildClass || button->Action == ButtonCmd::UpgradeTo || button->Action == ButtonCmd::UpgradeToClass || button->Action == ButtonCmd::ExperienceUpgradeTo) && button->Icon.Name.empty() && !button_unit_type->Icon.Name.empty()) {
			button_icon = button_unit_type->Icon.Icon;
		} else if (button->Action == ButtonCmd::Buy) {
			button_icon = UnitManager.GetSlotUnit(button->Value).get_icon();
		} else if ((button->Action == ButtonCmd::Research || button->Action == ButtonCmd::ResearchClass) && button->Icon.Name.empty() && button_upgrade->get_icon()) {
			button_icon = button_upgrade->get_icon();
		} else if (button->Action == ButtonCmd::Faction && button->Icon.Name.empty() && wyrmgus::faction::get_all()[CPlayer::GetThisPlayer()->Faction]->DevelopsTo[button->Value]->get_icon() != nullptr) {
			button_icon = wyrmgus::faction::get_all()[CPlayer::GetThisPlayer()->Faction]->DevelopsTo[button->Value]->get_icon();
		} else if (button->Action == ButtonCmd::Dynasty && button->Icon.Name.empty() && CPlayer::GetThisPlayer()->get_faction()->get_dynasties()[button->Value]->get_icon() != nullptr) {
			button_icon = CPlayer::GetThisPlayer()->get_faction()->get_dynasties()[button->Value]->get_icon();
		}
		//Wyrmgus end
		
		if (cooldownSpell) {
			//Wyrmgus start
//			button->Icon.Icon->DrawCooldownSpellIcon(pos,
			button_icon->DrawCooldownSpellIcon(pos,
			//Wyrmgus end
				maxCooldown * 100 / wyrmgus::spell::get_all()[button->Value]->get_cooldown());
		} else if (gray) {
			//Wyrmgus start
//			button->Icon.Icon->DrawGrayscaleIcon(pos);
//			button_icon->DrawGrayscaleIcon(pos); //better to not show it
			//Wyrmgus end
		} else {
			const wyrmgus::player_color *player_color = nullptr;
			if (Selected.empty() == false && Selected[0]->IsAlive()) {
				player_color = Selected[0]->get_player_color();

				//Wyrmgus start
				//if is accessing a building of another player, set color to that of the person player (i.e. for training buttons)
				if (CPlayer::GetThisPlayer()->HasBuildingAccess(*Selected[0]->Player, button->Action)) {
					player_color = CPlayer::GetThisPlayer()->get_player_color();
				}
				//Wyrmgus end
			}
			
			if (IsButtonUsable(*Selected[0], *button)) {
				button_icon->DrawUnitIcon(*UI.ButtonPanel.Buttons[i].Style,
												   GetButtonStatus(*button, ButtonUnderCursor),
												   pos, buf, player_color, false, false, 100 - GetButtonCooldownPercent(*Selected[0], *button));
												   
				if (
					((button->Action == ButtonCmd::Train || button->Action == ButtonCmd::TrainClass) && Selected[0]->Type->Stats[Selected[0]->Player->Index].GetUnitStock(button_unit_type) != 0)
					|| button->Action == ButtonCmd::SellResource || button->Action == ButtonCmd::BuyResource
				) {
					std::string number_string;
					if ((button->Action == ButtonCmd::Train || button->Action == ButtonCmd::TrainClass) && Selected[0]->Type->Stats[Selected[0]->Player->Index].GetUnitStock(button_unit_type) != 0) { //draw the quantity in stock for unit "training" cases which have it
						number_string = std::to_string(Selected[0]->GetUnitStock(button_unit_type)) + "/" + std::to_string(Selected[0]->Type->Stats[Selected[0]->Player->Index].GetUnitStock(button_unit_type));
					} else if (button->Action == ButtonCmd::SellResource) {
						number_string = std::to_string(Selected[0]->Player->GetEffectiveResourceSellPrice(button->Value));
					} else if (button->Action == ButtonCmd::BuyResource) {
						number_string = std::to_string(Selected[0]->Player->GetEffectiveResourceBuyPrice(button->Value));
					}
					CLabel label(wyrmgus::defines::get()->get_game_font());

					label.Draw(pos.x + 46 - wyrmgus::defines::get()->get_game_font()->Width(number_string), pos.y + 0, number_string);
				}
			} else if ( //draw researched technologies (or acquired abilities) grayed
				((button->Action == ButtonCmd::Research || button->Action == ButtonCmd::ResearchClass || button->Action == ButtonCmd::Dynasty) && UpgradeIdAllowed(*CPlayer::GetThisPlayer(), button_upgrade->ID) == 'R')
				|| (button->Action == ButtonCmd::LearnAbility && Selected[0]->GetIndividualUpgrade(CUpgrade::get(button->ValueStr)) == CUpgrade::get(button->ValueStr)->MaxLimit)
			) {
				button_icon->DrawUnitIcon(*UI.ButtonPanel.Buttons[i].Style,
												   GetButtonStatus(*button, ButtonUnderCursor),
												   pos, buf, player_color, false, true);
			} else {
				button_icon->DrawUnitIcon(*UI.ButtonPanel.Buttons[i].Style,
												   GetButtonStatus(*button, ButtonUnderCursor),
												   pos, buf, player_color, true);
			}
		}
	}
	
	//Wyrmgus start
	if (ButtonAreaUnderCursor == ButtonAreaTransporting) {
		CUnit *uins = Selected[0]->UnitInside;
		size_t j = 0;

		for (int i = 0; i < Selected[0]->InsideCount; ++i, uins = uins->NextContained) {
			if (!uins->Boarded || j >= UI.TransportingButtons.size() || (Selected[0]->Player != CPlayer::GetThisPlayer() && uins->Player != CPlayer::GetThisPlayer())) {
				continue;
			}
			if (static_cast<size_t>(ButtonUnderCursor) == j) {
				const wyrmgus::font_color *text_color = nullptr;
				if (uins->get_unique() != nullptr || uins->Character != nullptr) {
					text_color = wyrmgus::defines::get()->get_unique_font_color();
				} else if (uins->Prefix != nullptr || uins->Suffix != nullptr) {
					text_color = wyrmgus::defines::get()->get_magic_font_color();
				}
				DrawGenericPopup(uins->GetMessageName(), UI.TransportingButtons[j].X, UI.TransportingButtons[j].Y, text_color);
			}
			++j;
		}
	}
	//Wyrmgus end
}

/**
**  Check if the button is allowed for the unit.
**
**  @param unit          unit which checks for allow.
**  @param buttonaction  button to check if it is allowed.
**
**  @return 1 if button is allowed, 0 else.
**
**  @todo FIXME: better check. (dependency, resource, ...)
**  @todo FIXME: make difference with impossible and not yet researched.
*/
bool IsButtonAllowed(const CUnit &unit, const wyrmgus::button &buttonaction)
{
	if (buttonaction.is_always_shown()) {
		return true;
	}
	
	bool res = false;
	if (buttonaction.Allowed) {
		res = buttonaction.Allowed(unit, buttonaction);
		if (!res) {
			return false;
		} else {
			res = false;
		}
	}
	
	//Wyrmgus start
	if (!CPlayer::GetThisPlayer()->IsTeamed(*Selected[0]) && (!CPlayer::GetThisPlayer()->HasBuildingAccess(*Selected[0]->Player, buttonaction.Action) || !IsNeutralUsableButtonAction(buttonaction.Action))) {
		return false;
	}
	//Wyrmgus end

	const wyrmgus::unit_type *unit_type = buttonaction.get_value_unit_type(&unit);
	const CUpgrade *upgrade = buttonaction.get_value_upgrade(&unit);

	// Check button-specific cases
	switch (buttonaction.Action) {
		case ButtonCmd::Stop:
		case ButtonCmd::StandGround:
		case ButtonCmd::Button:
		case ButtonCmd::Move:
		case ButtonCmd::CallbackAction:
		//Wyrmgus start
		case ButtonCmd::RallyPoint:
		case ButtonCmd::Unit:
		case ButtonCmd::EditorUnit:
		case ButtonCmd::ProduceResource:
		case ButtonCmd::SellResource:
		case ButtonCmd::BuyResource:
		case ButtonCmd::Salvage:
		case ButtonCmd::EnterMapLayer:
		//Wyrmgus end
			res = true;
			break;
		case ButtonCmd::Repair:
			res = unit.Type->RepairRange > 0;
			break;
		case ButtonCmd::Patrol:
			res = unit.CanMove();
			break;
		case ButtonCmd::Harvest:
			if (!unit.CurrentResource
				|| !(unit.ResourcesHeld > 0 && !unit.Type->ResInfo[unit.CurrentResource]->LoseResources)
				|| (unit.ResourcesHeld != unit.Type->ResInfo[unit.CurrentResource]->ResourceCapacity
					&& unit.Type->ResInfo[unit.CurrentResource]->LoseResources)) {
				res = true;
			}
			//Wyrmgus start
			if (unit.BoardCount) {
				res = false;
			}
			//Wyrmgus end
			break;
		case ButtonCmd::Return:
			if (!(!unit.CurrentResource
				  || !(unit.ResourcesHeld > 0 && !unit.Type->ResInfo[unit.CurrentResource]->LoseResources)
				  || (unit.ResourcesHeld != unit.Type->ResInfo[unit.CurrentResource]->ResourceCapacity
					  && unit.Type->ResInfo[unit.CurrentResource]->LoseResources))) {
				res = true;
			}
			break;
		case ButtonCmd::Attack:
			res = ButtonCheckAttack(unit, buttonaction);
			break;
		case ButtonCmd::AttackGround:
			if (unit.Type->BoolFlag[GROUNDATTACK_INDEX].value) {
				res = true;
			}
			break;
		case ButtonCmd::Train:
		case ButtonCmd::TrainClass:
			if (unit_type == nullptr) {
				break;
			}

			// Check if building queue is enabled
			if (!EnableTrainingQueue && unit.CurrentAction() == UnitAction::Train) {
				break;
			}
			if (unit.Player->Index == PlayerNumNeutral && !unit.can_hire_mercenary(unit_type)) {
				break;
			}
		// FALL THROUGH
		case ButtonCmd::UpgradeTo:
		case ButtonCmd::UpgradeToClass:
		case ButtonCmd::Build:
		case ButtonCmd::BuildClass:
			if (unit_type == nullptr) {
				break;
			}

			res = check_conditions<true>(unit_type, unit.Player, false, !CPlayer::GetThisPlayer()->IsTeamed(unit));
			break;
		case ButtonCmd::Research:
		case ButtonCmd::ResearchClass:
			res = check_conditions<true>(upgrade, unit.Player, false, !CPlayer::GetThisPlayer()->IsTeamed(unit));
			if (res) {
				res = (UpgradeIdAllowed(*CPlayer::GetThisPlayer(), upgrade->ID) == 'A' || UpgradeIdAllowed(*CPlayer::GetThisPlayer(), upgrade->ID) == 'R') && check_conditions<true>(upgrade, CPlayer::GetThisPlayer(), false); //also check for the conditions for this player (rather than the unit) as an extra for researches, so that the player doesn't research too advanced technologies at neutral buildings
				res = res && (!unit.Player->UpgradeTimers.Upgrades[upgrade->ID] || unit.Player->UpgradeTimers.Upgrades[upgrade->ID] == upgrade->Costs[TimeCost]); //don't show if is being researched elsewhere
			}
			break;
		case ButtonCmd::ExperienceUpgradeTo:
			res = check_conditions<true>(unit_type, &unit, true);
			if (res && unit.Character != nullptr) {
				res = !wyrmgus::vector::contains(unit.Character->ForbiddenUpgrades, unit_type);
			}
			break;
		case ButtonCmd::LearnAbility:
			res = unit.can_learn_ability<true>(upgrade);
			break;
		case ButtonCmd::SpellCast:
			res = wyrmgus::spell::get_all()[buttonaction.Value]->IsAvailableForUnit(unit);
			break;
		case ButtonCmd::Unload:
			res = (Selected[0]->Type->CanTransport() && Selected[0]->BoardCount);
			break;
		case ButtonCmd::Cancel:
			res = true;
			break;
		case ButtonCmd::CancelUpgrade:
			//Wyrmgus start
//			res = unit.CurrentAction() == UnitAction::UpgradeTo
//				  || unit.CurrentAction() == UnitAction::Research;
			//don't show the cancel button for a quick moment if the time cost is 0
			res = (unit.CurrentAction() == UnitAction::UpgradeTo && static_cast<COrder_UpgradeTo *>(unit.CurrentOrder())->GetUnitType().Stats[unit.Player->Index].Costs[TimeCost] > 0)
				|| (unit.CurrentAction() == UnitAction::Research && static_cast<COrder_Research *>(unit.CurrentOrder())->GetUpgrade().Costs[TimeCost] > 0);
			//Wyrmgus end
			break;
		case ButtonCmd::CancelTrain:
			//Wyrmgus start
//			res = unit.CurrentAction() == UnitAction::Train;
			res = unit.CurrentAction() == UnitAction::Train && static_cast<COrder_Train *>(unit.CurrentOrder())->GetUnitType().Stats[unit.Player->Index].Costs[TimeCost] > 0; //don't show the cancel button for a quick moment if the time cost is 0
			//Wyrmgus end
			break;
		case ButtonCmd::CancelBuild:
			res = unit.CurrentAction() == UnitAction::Built;
			break;
		case ButtonCmd::Faction:
			res = CPlayer::GetThisPlayer()->get_faction() != nullptr && buttonaction.Value != -1 && buttonaction.Value < (int) CPlayer::GetThisPlayer()->get_faction()->DevelopsTo.size() && CPlayer::GetThisPlayer()->can_found_faction<true>(CPlayer::GetThisPlayer()->get_faction()->DevelopsTo[buttonaction.Value]);
			break;
		case ButtonCmd::Dynasty:
			res = CPlayer::GetThisPlayer()->get_faction() != nullptr && buttonaction.Value != -1 && buttonaction.Value < static_cast<int>(CPlayer::GetThisPlayer()->get_faction()->get_dynasties().size()) && CPlayer::GetThisPlayer()->can_choose_dynasty<true>(CPlayer::GetThisPlayer()->get_faction()->get_dynasties()[buttonaction.Value]) && CPlayer::GetThisPlayer()->get_faction()->get_dynasties()[buttonaction.Value]->get_icon() != nullptr;
			if (res) {
				upgrade = CPlayer::GetThisPlayer()->get_faction()->get_dynasties()[buttonaction.Value]->get_upgrade();
				res = (!unit.Player->UpgradeTimers.Upgrades[upgrade->ID] || unit.Player->UpgradeTimers.Upgrades[upgrade->ID] == upgrade->Costs[TimeCost]);
			}
			break;
		case ButtonCmd::Quest:
			res = buttonaction.Value < static_cast<int>(unit.Player->get_available_quests().size()) && unit.Player->can_accept_quest(unit.Player->get_available_quests().at(buttonaction.Value));
			break;
		case ButtonCmd::Buy:
			res = (buttonaction.Value != -1) && (&UnitManager.GetSlotUnit(buttonaction.Value) != nullptr);
			if (res && UnitManager.GetSlotUnit(buttonaction.Value).Character != nullptr) {
				//check whether the character's conditions are still valid
				res = res && Selected[0]->Player->is_character_available_for_recruitment(UnitManager.GetSlotUnit(buttonaction.Value).Character, true);
			}
			break;
	}
#if 0
	// there is a additional check function -- call it
	if (res && buttonaction.Disabled) {
		return buttonaction.Disabled(unit, buttonaction);
	}
#endif
	return res;
}

//Wyrmgus start
/**
**	Check if the button is usable for the unit.
**
**  @param unit          unit which checks for allow.
**  @param buttonaction  button to check if it is usable.
**
**  @return 1 if button is usable, 0 else.
*/
bool IsButtonUsable(const CUnit &unit, const wyrmgus::button &buttonaction)
{
	if (!IsButtonAllowed(unit, buttonaction)) {
		return false;
	}
	
	bool res = false;
	if (buttonaction.Allowed) {
		res = buttonaction.Allowed(unit, buttonaction);
		if (!res) {
			return false;
		} else {
			res = false;
		}
	}

	const wyrmgus::unit_type *unit_type = buttonaction.get_value_unit_type(&unit);
	const CUpgrade *upgrade = buttonaction.get_value_upgrade(&unit);

	// Check button-specific cases
	switch (buttonaction.Action) {
		case ButtonCmd::Stop:
		case ButtonCmd::StandGround:
		case ButtonCmd::Button:
		case ButtonCmd::Move:
		case ButtonCmd::CallbackAction:
		case ButtonCmd::RallyPoint:
		case ButtonCmd::Unit:
		case ButtonCmd::EditorUnit:
		case ButtonCmd::Repair:
		case ButtonCmd::Patrol:
		case ButtonCmd::Harvest:
		case ButtonCmd::Return:
		case ButtonCmd::Attack:
		case ButtonCmd::AttackGround:
		case ButtonCmd::Salvage:
		case ButtonCmd::EnterMapLayer:
			res = true;
			break;
		case ButtonCmd::Train:
		case ButtonCmd::TrainClass:
		case ButtonCmd::UpgradeTo:
		case ButtonCmd::UpgradeToClass:
		case ButtonCmd::Build:
		case ButtonCmd::BuildClass:
			res = check_conditions<false>(unit_type, unit.Player, false, !CPlayer::GetThisPlayer()->IsTeamed(unit));
			break;
		case ButtonCmd::Research:
		case ButtonCmd::ResearchClass:
			res = check_conditions<false>(upgrade, unit.Player, false, !CPlayer::GetThisPlayer()->IsTeamed(unit));
			if (res) {
				res = UpgradeIdAllowed(*CPlayer::GetThisPlayer(), upgrade->ID) == 'A' && check_conditions<false>(upgrade, CPlayer::GetThisPlayer(), false); //also check for the conditions of this player extra for researches, so that the player doesn't research too advanced technologies at neutral buildings
			}
			break;
		case ButtonCmd::ExperienceUpgradeTo:
			res = check_conditions<false>(unit_type, &unit, true) && unit.Variable[LEVELUP_INDEX].Value >= 1;
			break;
		case ButtonCmd::LearnAbility:
			res = unit.can_learn_ability(upgrade);
			break;
		case ButtonCmd::SpellCast:
			res = wyrmgus::spell::get_all()[buttonaction.Value]->IsAvailableForUnit(unit);
			break;
		case ButtonCmd::Faction:
			res = CPlayer::GetThisPlayer()->can_found_faction(CPlayer::GetThisPlayer()->get_faction()->DevelopsTo[buttonaction.Value]);
			break;
		case ButtonCmd::Dynasty:
			res = CPlayer::GetThisPlayer()->can_choose_dynasty(CPlayer::GetThisPlayer()->get_faction()->get_dynasties()[buttonaction.Value]);
			break;
		case ButtonCmd::Buy:
			res = true;
			if (UnitManager.GetSlotUnit(buttonaction.Value).Character != nullptr) {
				res = CPlayer::GetThisPlayer()->Heroes.size() < PlayerHeroMax;
			}
			break;
		case ButtonCmd::Unload:
		case ButtonCmd::Cancel:
		case ButtonCmd::CancelUpgrade:
		case ButtonCmd::CancelTrain:
		case ButtonCmd::CancelBuild:
		case ButtonCmd::Quest:
		case ButtonCmd::ProduceResource:
		case ButtonCmd::SellResource:
		case ButtonCmd::BuyResource:
			res = true;
			break;
	}

	return res;
}

/**
**  Get the cooldown timer for the button (if any).
**
**  @param unit          unit which checks for allow.
**  @param buttonaction  button to check if it is usable.
**
**  @return the cooldown for using the button.
*/
int GetButtonCooldown(const CUnit &unit, const wyrmgus::button &buttonaction)
{
	int cooldown = 0;

	// Check button-specific cases
	switch (buttonaction.Action) {
		case ButtonCmd::Buy:
			if (buttonaction.Value != -1 && UnitManager.GetSlotUnit(buttonaction.Value).Character != nullptr) {
				cooldown = CPlayer::GetThisPlayer()->HeroCooldownTimer;
			}
			break;
	}

	return cooldown;
}

/**
**  Get the cooldown timer for the button, in percent.
**
**  @param unit          unit which checks for allow.
**  @param buttonaction  button to check if it is usable.
**
**  @return the cooldown for using the button.
*/
int GetButtonCooldownPercent(const CUnit &unit, const wyrmgus::button &buttonaction)
{
	int cooldown = 0;

	// Check button-specific cases
	switch (buttonaction.Action) {
		case ButtonCmd::Buy:
			if (buttonaction.Value != -1 && UnitManager.GetSlotUnit(buttonaction.Value).Character != nullptr) {
				cooldown = CPlayer::GetThisPlayer()->HeroCooldownTimer * 100 / HeroCooldownCycles;
			}
			break;
	}

	return cooldown;
}
//Wyrmgus end

/**
**  Update bottom panel for multiple units.
**
**  @return array of UI.ButtonPanel.NumButtons buttons to show.
**
**  @todo FIXME : make UpdateButtonPanelMultipleUnits more configurable.
**  @todo show all possible buttons or just same button...
*/
static void UpdateButtonPanelMultipleUnits(const std::vector<std::unique_ptr<wyrmgus::button>> &buttonActions)
{
	char unit_ident[128];
	//Wyrmgus start
	char individual_unit_ident[200][128]; // the 200 there is the max selectable quantity; not nice to hardcode it like this, should be changed in the future
	//Wyrmgus end

	sprintf(unit_ident, ",%s-group,", wyrmgus::civilization::get_all()[CPlayer::GetThisPlayer()->Race]->get_identifier().c_str());
	
	//Wyrmgus start
	for (size_t i = 0; i != Selected.size(); ++i) {
		sprintf(individual_unit_ident[i], ",%s,", Selected[i]->Type->Ident.c_str());
	}
	//Wyrmgus end

	for (const wyrmgus::button *button : wyrmgus::button::get_all()) {
		if (button->get_level() != CurrentButtonLevel) {
			continue;
		}

		//Wyrmgus start
		bool used_by_all = true;
		for (size_t i = 0; i != Selected.size(); ++i) {
			if (!strstr(button->UnitMask.c_str(), individual_unit_ident[i]) && !wyrmgus::vector::contains(button->get_unit_classes(), Selected[i]->Type->get_unit_class())) {
				used_by_all = false;
				break;
			}
		}
		//Wyrmgus end
		
		// any unit or unit in list
		if (button->UnitMask[0] != '*'
			&& !strstr(button->UnitMask.c_str(), unit_ident) && !used_by_all) {
			continue;
		}

		bool allow = true;
		if (button->is_always_shown() == false) {
			for (size_t i = 0; i != Selected.size(); ++i) {
				if (!IsButtonAllowed(*Selected[i], *button)) {
					allow = false;
					break;
				}
			}
		}

		Assert(1 <= button->get_pos());
		Assert(button->get_pos() <= (int)UI.ButtonPanel.Buttons.size());

		// is button allowed after all?
		if (allow) {
			// OverWrite, So take last valid button.
			*buttonActions[button->get_pos() - 1] = *button;
		}
	}
}

/**
**  Update bottom panel for single unit.
**  or unit group with the same type.
**
**  @param unit  unit which has actions shown with buttons.
**
**  @return array of UI.ButtonPanel.NumButtons buttons to show.
**
**  @todo FIXME : Remove Hack for cancel button.
*/
static void UpdateButtonPanelSingleUnit(const CUnit &unit, const std::vector<std::unique_ptr<wyrmgus::button>> &buttonActions)
{
	char unit_ident[128];

	//
	//  FIXME: johns: some hacks for cancel buttons
	//
	bool only_cancel_allowed = true;
	if (unit.CurrentAction() == UnitAction::Built) {
		// Trick 17 to get the cancel-build button
		strcpy_s(unit_ident, sizeof(unit_ident), ",cancel-build,");
	} else if (unit.CurrentAction() == UnitAction::UpgradeTo) {
		// Trick 17 to get the cancel-upgrade button
		strcpy_s(unit_ident, sizeof(unit_ident), ",cancel-upgrade,");
	} else if (unit.CurrentAction() == UnitAction::Research) {
		if (CurrentButtonLevel != nullptr) {
			CurrentButtonLevel = nullptr;
		}
		// Trick 17 to get the cancel-upgrade button
		strcpy_s(unit_ident, sizeof(unit_ident), ",cancel-upgrade,");
	} else {
		sprintf(unit_ident, ",%s,", unit.Type->Ident.c_str());
		only_cancel_allowed = false;
	}
	for (const wyrmgus::button *button : wyrmgus::button::get_all()) {
		Assert(0 < button->get_pos() && button->get_pos() <= (int)UI.ButtonPanel.Buttons.size());

		// Same level
		if (button->get_level() != CurrentButtonLevel) {
			continue;
		}

		// any unit or unit in list
		if (button->UnitMask[0] != '*'
			&& !strstr(button->UnitMask.c_str(), unit_ident) && (only_cancel_allowed || !wyrmgus::vector::contains(button->get_unit_classes(), unit.Type->get_unit_class()))) {
			continue;
		}
		//Wyrmgus start
//		int allow = IsButtonAllowed(unit, buttonaction);
		bool allow = true; // check all selected units, as different units of the same type may have different allowed buttons
		if (button->is_always_shown() == false) {
			for (size_t j = 0; j != Selected.size(); ++j) {
				if (!IsButtonAllowed(*Selected[j], *button)) {
					allow = false;
					break;
				}
			}
		}
		//Wyrmgus end
		int pos = button->get_pos();

		// Special case for researches
		int researchCheck = true;
		if (button->is_always_shown() && !allow && (button->Action == ButtonCmd::Research || button->Action == ButtonCmd::ResearchClass || button->Action == ButtonCmd::Dynasty)) {
			const CUpgrade *upgrade = button->get_value_upgrade(Selected[0]);

			if (UpgradeIdAllowed(*CPlayer::GetThisPlayer(), upgrade->ID) == 'R') {
				researchCheck = false;
			}
		}
		
		// is button allowed after all?
		if ((button->is_always_shown() && buttonActions[pos - 1]->get_pos() == -1 && researchCheck) || allow) {
			// OverWrite, So take last valid button.
			*buttonActions[pos - 1] = *button;
		}
	}
}

/**
**  Update button panel.
**
**  @internal Affect CurrentButtons with buttons to show.
*/
void CButtonPanel::Update()
{
	//Wyrmgus start
//	if (Selected.empty()) {
	if (Selected.empty() || (!GameRunning && !GameEstablishing)) {
	//Wyrmgus end
		CurrentButtons.clear();
		return;
	}

	CUnit &unit = *Selected[0];
	// foreign unit
	//Wyrmgus start
//	if (unit.Player != CPlayer::GetThisPlayer() && !CPlayer::GetThisPlayer()->IsTeamed(unit)) {
	if (unit.Player != CPlayer::GetThisPlayer() && !CPlayer::GetThisPlayer()->IsTeamed(unit) && !CPlayer::GetThisPlayer()->HasBuildingAccess(*unit.Player)) {
	//Wyrmgus end
		CurrentButtons.clear();
		return;
	}
	
	//Wyrmgus start
	//update the sold item buttons
	if (GameRunning || GameEstablishing) {
		unsigned int sold_unit_count = 0;
		unsigned int potential_faction_count = 0;
		unsigned int potential_dynasty_count = 0;
		for (wyrmgus::button *button : wyrmgus::button::get_all()) {
			if (button->Action != ButtonCmd::Faction && button->Action != ButtonCmd::Dynasty && button->Action != ButtonCmd::Buy) {
				continue;
			}
			char unit_ident[128];
			sprintf(unit_ident, ",%s,", unit.Type->Ident.c_str());
			if (button->UnitMask[0] != '*' && !strstr(button->UnitMask.c_str(), unit_ident) && !wyrmgus::vector::contains(button->get_unit_classes(), unit.Type->get_unit_class())) {
				continue;
			}

			if (button->Action == ButtonCmd::Faction) {
				if (CPlayer::GetThisPlayer()->get_faction() == nullptr || potential_faction_count >= CPlayer::GetThisPlayer()->get_faction()->DevelopsTo.size()) {
					button->Value = -1;
				} else {
					const wyrmgus::faction *faction = CPlayer::GetThisPlayer()->get_faction()->DevelopsTo[potential_faction_count];
					button->Value = potential_faction_count;
					button->Hint = "Found ";
					if (faction->uses_definite_article()) {
						button->Hint += "the ";
					}
					button->Hint += faction->get_name();
					button->Description = "Changes your faction to ";
					if (faction->uses_definite_article()) {
						button->Description += "the ";
					}
					button->Description += faction->get_name();
				}
				potential_faction_count += 1;
			} else if (button->Action == ButtonCmd::Dynasty) {
				if (CPlayer::GetThisPlayer()->get_faction() == nullptr || potential_dynasty_count >= CPlayer::GetThisPlayer()->get_faction()->get_dynasties().size()) {
					button->Value = -1;
				} else {
					const wyrmgus::dynasty *dynasty = CPlayer::GetThisPlayer()->get_faction()->get_dynasties()[potential_dynasty_count];
					button->Value = potential_dynasty_count;
					button->Hint = "Choose the ";
					button->Hint += dynasty->get_name();
					button->Hint += " Dynasty";
					button->Description = "Changes your dynasty to the ";
					button->Description += dynasty->get_name();
					button->Description += " dynasty";
				}
				potential_dynasty_count += 1;
			} else if (button->Action == ButtonCmd::Buy) {
				if (sold_unit_count >= unit.SoldUnits.size()) {
					button->Value = -1;
				} else {
					button->Value = UnitNumber(*unit.SoldUnits[sold_unit_count]);
					if (unit.SoldUnits[sold_unit_count]->Character != nullptr) {
						button->Hint = "Recruit " + unit.SoldUnits[sold_unit_count]->GetName();
					} else {
						if (!unit.SoldUnits[sold_unit_count]->Name.empty()) {
							button->Hint = "Buy " + unit.SoldUnits[sold_unit_count]->GetName();
						} else {
							button->Hint = "Buy " + unit.SoldUnits[sold_unit_count]->GetTypeName();
						}
					}
				}
				sold_unit_count += 1;
			}
		}
	}
	//Wyrmgus end

	bool sameType = true;
	// multiple selected
	for (size_t i = 1; i < Selected.size(); ++i) {
		if (Selected[i]->Type != unit.Type) {
			sameType = false;
			break;
		}
	}

	for (size_t i = CurrentButtons.size(); i != UI.ButtonPanel.Buttons.size(); ++i) {
		CurrentButtons.push_back(std::make_unique<wyrmgus::button>());
	}

	for (const std::unique_ptr<wyrmgus::button> &button : CurrentButtons) {
		button->pos = -1;
	}

	// We have selected different units types
	if (!sameType) {
		UpdateButtonPanelMultipleUnits(CurrentButtons);
	} else {
		// We have same type units selected
		// -- continue with setting buttons as for the first unit
		UpdateButtonPanelSingleUnit(unit, CurrentButtons);
	}
}

void CButtonPanel::DoClicked_SelectTarget(int button)
{
	// Select target.
	CurrentCursorState = CursorState::Select;
	GameCursor = UI.get_cursor(wyrmgus::cursor_type::yellow_hair);
	CursorAction = CurrentButtons[button]->Action;
	CursorValue = CurrentButtons[button]->Value;
	CurrentButtonLevel = wyrmgus::defines::get()->get_cancel_button_level(); // the cancel-only button level
	UI.ButtonPanel.Update();
	UI.StatusLine.Set(_("Select Target"));
}

void CButtonPanel::DoClicked_Unload(int button)
{
	const int flush = !(KeyModifiers & ModifierShift);
	//
	//  Unload on coast, transporter standing, unload all units right now.
	//  That or a bunker.
	//  Unload on coast valid only for sea units
	//
	if ((Selected.size() == 1 && Selected[0]->CurrentAction() == UnitAction::Still
		 && Selected[0]->Type->UnitType == UnitTypeType::Naval && Selected[0]->MapLayer->Field(Selected[0]->tilePos)->CoastOnMap())
		|| !Selected[0]->CanMove()) {
		SendCommandUnload(*Selected[0], Selected[0]->tilePos, NoUnitP, flush, Selected[0]->MapLayer->ID);
		return ;
	}
	DoClicked_SelectTarget(button);
}

void CButtonPanel::DoClicked_SpellCast(int button)
{
	const int spell_id = CurrentButtons[button]->Value;
	const wyrmgus::spell *spell = wyrmgus::spell::get_all()[spell_id];
	if (KeyModifiers & ModifierControl) {
		if (spell->get_autocast_info() == nullptr) {
			PlayGameSound(GameSounds.PlacementError[CPlayer::GetThisPlayer()->Race].Sound, MaxSampleVolume);
			return;
		}

		bool autocast = false;

		// If any selected unit doesn't have autocast on turn it on
		// for everyone
		for (size_t i = 0; i != Selected.size(); ++i) {
			if (!Selected[i]->is_autocast_spell(spell)) {
				autocast = true;
				break;
			}
		}
		for (size_t i = 0; i != Selected.size(); ++i) {
			if (Selected[i]->is_autocast_spell(spell) != autocast) {
				SendCommandAutoSpellCast(*Selected[i], spell, autocast);
			}
		}
		return;
	}

	if (spell->is_caster_only()) {
		const int flush = !(KeyModifiers & ModifierShift);

		for (size_t i = 0; i != Selected.size(); ++i) {
			CUnit &unit = *Selected[i];
			// CursorValue here holds the spell type id
			SendCommandSpellCast(unit, unit.tilePos, &unit, spell_id, flush, unit.MapLayer->ID);
		}
		return;
	}
	DoClicked_SelectTarget(button);
}

void CButtonPanel::DoClicked_Repair(int button)
{
	if (KeyModifiers & ModifierControl) {
		unsigned autorepair = 0;
		// If any selected unit doesn't have autocast on turn it on
		// for everyone
		for (size_t i = 0; i != Selected.size(); ++i) {
			if (Selected[i]->AutoRepair == 0) {
				autorepair = 1;
				break;
			}
		}
		for (size_t i = 0; i != Selected.size(); ++i) {
			if (Selected[i]->AutoRepair != autorepair) {
				SendCommandAutoRepair(*Selected[i], autorepair);
			}
		}
		return;
	}
	DoClicked_SelectTarget(button);
}

void CButtonPanel::DoClicked_Return()
{
	for (size_t i = 0; i != Selected.size(); ++i) {
		SendCommandReturnGoods(*Selected[i], NoUnitP, !(KeyModifiers & ModifierShift));
	}
}

void CButtonPanel::DoClicked_Stop()
{
	for (size_t i = 0; i != Selected.size(); ++i) {
		SendCommandStopUnit(*Selected[i]);
	}
}

void CButtonPanel::DoClicked_StandGround()
{
	for (size_t i = 0; i != Selected.size(); ++i) {
		SendCommandStandGround(*Selected[i], !(KeyModifiers & ModifierShift));
	}
}

void CButtonPanel::DoClicked_Button(int button)
{
	CurrentButtonLevel = wyrmgus::button_level::try_get(CurrentButtons[button]->ValueStr);
	LastDrawnButtonPopup = nullptr;
	UI.ButtonPanel.Update();
}

void CButtonPanel::DoClicked_CancelUpgrade()
{
	if (Selected.size() == 1) {
		switch (Selected[0]->CurrentAction()) {
			case UnitAction::UpgradeTo:
				SendCommandCancelUpgradeTo(*Selected[0]);
				break;
			case UnitAction::Research:
				SendCommandCancelResearch(*Selected[0]);
				break;
			default:
				break;
		}
	}
	UI.StatusLine.Clear();
	UI.StatusLine.ClearCosts();
	CurrentButtonLevel = nullptr;
	UI.ButtonPanel.Update();
	GameCursor = UI.get_cursor(wyrmgus::cursor_type::point);
	CursorBuilding = nullptr;
	CurrentCursorState = CursorState::Point;
}

void CButtonPanel::DoClicked_CancelTrain()
{
	Assert(Selected[0]->CurrentAction() == UnitAction::Train);
	SendCommandCancelTraining(*Selected[0], -1, nullptr);
	UI.StatusLine.Clear();
	UI.StatusLine.ClearCosts();
}

void CButtonPanel::DoClicked_CancelBuild()
{
	// FIXME: johns is this not sure, only building should have this?
	Assert(Selected[0]->CurrentAction() == UnitAction::Built);
	if (Selected.size() == 1) {
		SendCommandDismiss(*Selected[0]);
	}
	UI.StatusLine.Clear();
	UI.StatusLine.ClearCosts();
}

void CButtonPanel::DoClicked_Build(const std::unique_ptr<wyrmgus::button> &button)
{
	// FIXME: store pointer in button table!
	const wyrmgus::unit_type *unit_type = button->get_value_unit_type(Selected[0]);

	if (!CPlayer::GetThisPlayer()->CheckUnitType(*unit_type)) {
		UI.StatusLine.Set(_("Select Location"));
		UI.StatusLine.ClearCosts();
		CursorBuilding = unit_type;
		// FIXME: check is this check necessary?
		CurrentButtonLevel = wyrmgus::defines::get()->get_cancel_button_level(); // the cancel-only button level
		UI.ButtonPanel.Update();
	}
}

void CButtonPanel::DoClicked_Train(const std::unique_ptr<wyrmgus::button> &button)
{
	// FIXME: store pointer in button table!
	const wyrmgus::unit_type *unit_type = button->get_value_unit_type(Selected[0]);

	// FIXME: training queue full check is not correct for network.
	// FIXME: this can be correct written, with a little more code.

	int best_training_place = 0;
	int lowest_queue = Selected[0]->Orders.size();
	
	for (size_t i = 0; i != Selected.size(); ++i) {
		if (Selected[i]->Type == Selected[0]->Type) {
			int selected_queue = 0;
			for (size_t j = 0; j < Selected[i]->Orders.size(); ++j) {
				if (Selected[i]->Orders[j]->Action == UnitAction::Train) {
					selected_queue += 1;
				}
			}
			if (selected_queue < lowest_queue) {
				lowest_queue = selected_queue;
				best_training_place = i;
			}
		}
	}
	
	if (unit_type->BoolFlag[RAIL_INDEX].value) {
		bool has_adjacent_rail = Selected[best_training_place]->HasAdjacentRailForUnitType(unit_type);
		if (!has_adjacent_rail) {
			CPlayer::GetThisPlayer()->Notify(NotifyYellow, Selected[best_training_place]->tilePos, Selected[best_training_place]->MapLayer->ID, "%s", _("The unit requires railroads to be placed on"));
			PlayGameSound(GameSounds.PlacementError[CPlayer::GetThisPlayer()->Race].Sound, MaxSampleVolume);
			return;
		}
	}
	
	int unit_count = 1;
	if (KeyModifiers & ModifierShift) {
		unit_count = 5;
	}
	
	for (int i = 0; i < unit_count; ++i) {
		if (Selected[best_training_place]->CurrentAction() == UnitAction::Train && !EnableTrainingQueue) {
			CPlayer::GetThisPlayer()->Notify(NotifyYellow, Selected[best_training_place]->tilePos, Selected[best_training_place]->MapLayer->ID, "%s", _("Unit training queue is full"));
			return;
		} else if (CPlayer::GetThisPlayer()->CheckLimits(*unit_type) >= 0 && !CPlayer::GetThisPlayer()->CheckUnitType(*unit_type, Selected[best_training_place]->Type->Stats[Selected[best_training_place]->Player->Index].GetUnitStock(unit_type) != 0)) {
			SendCommandTrainUnit(*Selected[best_training_place], *unit_type, CPlayer::GetThisPlayer()->Index, FlushCommands);
			UI.StatusLine.Clear();
			UI.StatusLine.ClearCosts();
		} else if (CPlayer::GetThisPlayer()->CheckLimits(*unit_type) == -3) {
			if (GameSounds.NotEnoughFood[CPlayer::GetThisPlayer()->Race].Sound) {
				PlayGameSound(GameSounds.NotEnoughFood[CPlayer::GetThisPlayer()->Race].Sound, MaxSampleVolume);
			}
			return;
		}
	}
	//Wyrmgus end
}

void CButtonPanel::DoClicked_UpgradeTo(const std::unique_ptr<wyrmgus::button> &button)
{
	const wyrmgus::unit_type *unit_type = button->get_value_unit_type(Selected[0]);

	for (size_t i = 0; i != Selected.size(); ++i) {
		if (Selected[i]->Player->CheckLimits(*unit_type) != -6 && !Selected[i]->Player->CheckUnitType(*unit_type)) {
			if (Selected[i]->CurrentAction() != UnitAction::UpgradeTo) {
				SendCommandUpgradeTo(*Selected[i], *unit_type, !(KeyModifiers & ModifierShift));
				UI.StatusLine.Clear();
				UI.StatusLine.ClearCosts();
			}
		} else {
			break;
		}
	}
}

void CButtonPanel::DoClicked_ExperienceUpgradeTo(int button)
{
	// FIXME: store pointer in button table!
	wyrmgus::unit_type &type = *wyrmgus::unit_type::get_all()[CurrentButtons[button]->Value];
	for (size_t i = 0; i != Selected.size(); ++i) {
		if (Selected[0]->Player->GetUnitTotalCount(type) < Selected[0]->Player->Allow.Units[type.Slot] || Selected[0]->Player->CheckLimits(type) != -6) { //ugly way to make the checklimits message only appear when it should
			if (Selected[i]->CurrentAction() != UnitAction::UpgradeTo) {
				Selected[i]->Variable[LEVELUP_INDEX].Value -= 1;
				Selected[i]->Variable[LEVELUP_INDEX].Max = Selected[i]->Variable[LEVELUP_INDEX].Value;
				if (!IsNetworkGame() && Selected[i]->Character != nullptr) {	//save the unit-type experience upgrade for persistent characters
					if (Selected[i]->Character->get_unit_type()->Slot != type.Slot) {
						if (Selected[i]->Player == CPlayer::GetThisPlayer()) {
							Selected[i]->Character->set_unit_type(wyrmgus::unit_type::get_all()[CurrentButtons[button]->Value]);
							SaveHero(Selected[i]->Character);
							wyrmgus::achievement::check_achievements();
						}
					}
				}
				SendCommandTransformInto(*Selected[i], type, !(KeyModifiers & ModifierShift));
				UI.StatusLine.Clear();
				UI.StatusLine.ClearCosts();
				Selected[i]->Player->UpdateLevelUpUnits();
			}
		} else {
			break;
		}
	}
	
	LastDrawnButtonPopup = nullptr;
	UI.ButtonPanel.Update();
	
	if (Selected[0]->Player == CPlayer::GetThisPlayer()) {
		SelectedUnitChanged();
	}
}

void CButtonPanel::DoClicked_Research(const std::unique_ptr<wyrmgus::button> &button)
{
	const CUpgrade *button_upgrade = button->get_value_upgrade(Selected[0]);

	int upgrade_costs[MaxCosts];
	CPlayer::GetThisPlayer()->GetUpgradeCosts(button_upgrade, upgrade_costs);
	if (!CPlayer::GetThisPlayer()->CheckCosts(upgrade_costs)) {
		//PlayerSubCosts(player,Upgrades[i].Costs);
		SendCommandResearch(*Selected[0], *button_upgrade, CPlayer::GetThisPlayer()->Index, !(KeyModifiers & ModifierShift));
		UI.StatusLine.Clear();
		UI.StatusLine.ClearCosts();
		LastDrawnButtonPopup = nullptr;
		ButtonUnderCursor = -1;
		OldButtonUnderCursor = -1;
		UI.ButtonPanel.Update();
	}
}

//Wyrmgus start
void CButtonPanel::DoClicked_LearnAbility(int button)
{
	const int index = CurrentButtons[button]->Value;
	SendCommandLearnAbility(*Selected[0], *CUpgrade::get_all()[index]);
	UI.StatusLine.Clear();
	UI.StatusLine.ClearCosts();
	LastDrawnButtonPopup = nullptr;
	
	if (Selected[0]->Player == CPlayer::GetThisPlayer()) {
		SelectedUnitChanged();
	}
}

void CButtonPanel::DoClicked_Faction(int button)
{
	const int index = CurrentButtons[button]->Value;
	SendCommandSetFaction(CPlayer::GetThisPlayer()->Index, wyrmgus::faction::get_all()[CPlayer::GetThisPlayer()->Faction]->DevelopsTo[index]->ID);
	ButtonUnderCursor = -1;
	OldButtonUnderCursor = -1;
	LastDrawnButtonPopup = nullptr;
	if (Selected[0]->Player == CPlayer::GetThisPlayer()) {
		SelectedUnitChanged();
	}
}

void CButtonPanel::DoClicked_Quest(int button)
{
	const int index = CurrentButtons[button]->Value;
	SendCommandQuest(*Selected[0], Selected[0]->Player->get_available_quests().at(index));
	ButtonUnderCursor = -1;
	OldButtonUnderCursor = -1;
	LastDrawnButtonPopup = nullptr;
	if (Selected[0]->Player == CPlayer::GetThisPlayer()) {
		SelectedUnitChanged();
	}
}

void CButtonPanel::DoClicked_Buy(int button)
{
	int buy_costs[MaxCosts];
	memset(buy_costs, 0, sizeof(buy_costs));
	buy_costs[CopperCost] = UnitManager.GetSlotUnit(CurrentButtons[button]->Value).GetPrice();
	if (!CPlayer::GetThisPlayer()->CheckCosts(buy_costs) && CPlayer::GetThisPlayer()->CheckLimits(*UnitManager.GetSlotUnit(CurrentButtons[button]->Value).Type) >= 0) {
		SendCommandBuy(*Selected[0], &UnitManager.GetSlotUnit(CurrentButtons[button]->Value), CPlayer::GetThisPlayer()->Index);
		ButtonUnderCursor = -1;
		OldButtonUnderCursor = -1;
		LastDrawnButtonPopup = nullptr;
		if (IsOnlySelected(*Selected[0])) {
			SelectedUnitChanged();
		}
	} else if (CPlayer::GetThisPlayer()->CheckLimits(*UnitManager.GetSlotUnit(CurrentButtons[button]->Value).Type) == -3) {
		if (GameSounds.NotEnoughFood[CPlayer::GetThisPlayer()->Race].Sound) {
			PlayGameSound(GameSounds.NotEnoughFood[CPlayer::GetThisPlayer()->Race].Sound, MaxSampleVolume);
		}
	}
}

void CButtonPanel::DoClicked_ProduceResource(int button)
{
	const int resource = CurrentButtons[button]->Value;
	if (resource != Selected[0]->GivesResource) {
		SendCommandProduceResource(*Selected[0], resource);
	} else if (Selected[0]->Type->get_given_resource() == nullptr) { //if resource production button was clicked when it was already active, then this means it should be toggled off; only do this if the building's type doesn't have a default produced resource, though, since in those cases it should always produce a resource
		SendCommandProduceResource(*Selected[0], 0);
	}
}

void CButtonPanel::DoClicked_SellResource(int button)
{
	const bool toggle_autosell = (KeyModifiers & ModifierControl) != 0;
	const int resource = CurrentButtons[button]->Value;
	
	if (toggle_autosell && Selected[0]->Player == CPlayer::GetThisPlayer()) {
		SendCommandAutosellResource(CPlayer::GetThisPlayer()->Index, resource);
	} else {
		int sell_resource_costs[MaxCosts];
		memset(sell_resource_costs, 0, sizeof(sell_resource_costs));
		sell_resource_costs[resource] = 100;
		if (!CPlayer::GetThisPlayer()->CheckCosts(sell_resource_costs)) {
			SendCommandSellResource(*Selected[0], resource, CPlayer::GetThisPlayer()->Index);
		}
	}
}

void CButtonPanel::DoClicked_BuyResource(int button)
{
	const int resource = CurrentButtons[button]->Value;
	int buy_resource_costs[MaxCosts];
	memset(buy_resource_costs, 0, sizeof(buy_resource_costs));
	buy_resource_costs[CopperCost] = Selected[0]->Player->GetEffectiveResourceBuyPrice(resource);
	if (!CPlayer::GetThisPlayer()->CheckCosts(buy_resource_costs)) {
		SendCommandBuyResource(*Selected[0], resource, CPlayer::GetThisPlayer()->Index);
	}
}

void CButtonPanel::DoClicked_Salvage()
{
	for (int i = (Selected.size()  - 1); i >= 0; --i) {
		SendCommandDismiss(*Selected[i], true);
	}
}

/**
**	@brief	Enter the map layer that the selected unit leads to.
*/
void CButtonPanel::DoClicked_EnterMapLayer()
{
	for (size_t i = 0; i < Selected.size(); ++i) {
		CUnit *connection_destination = Selected[i]->ConnectingDestination;
		if (connection_destination != nullptr) {
			PlayUnitSound(*connection_destination, wyrmgus::unit_sound_type::used);
			Selected[i]->Blink = 4;
			connection_destination->Blink = 4;
			UnSelectUnit(*Selected[i]);
			if (connection_destination->IsVisible(*CPlayer::GetThisPlayer())) {
				SelectUnit(*connection_destination);
			}
			SelectionChanged();
			ChangeCurrentMapLayer(connection_destination->MapLayer->ID);
			UI.SelectedViewport->Center(connection_destination->get_scaled_map_pixel_pos_center());
			break;
		}
	}
}

void CButtonPanel::DoClicked_CallbackAction(int button)
{
	LuaCallback* callback = (LuaCallback*)(CurrentButtons[button]->Payload);
	callback->pushPreamble();
	callback->pushInteger(UnitNumber(*Selected[0]));
	callback->run();
}

/**
**  Handle bottom button clicked.
**
**  @param button  Button that was clicked.
*/
void CButtonPanel::DoClicked(int button)
{
	Assert(0 <= button && button < (int)UI.ButtonPanel.Buttons.size());
	// no buttons
	if (CurrentButtons.empty()) {
		return;
	}
	if (IsButtonAllowed(*Selected[0], *CurrentButtons[button]) == false) {
		return;
	}
	//
	//  Button not available.
	//  or Not Teamed
	//
	//Wyrmgus start
//	if (CurrentButtons[button]->get_pos() == -1 || !ThisPlayer->IsTeamed(*Selected[0])) {
	if (CurrentButtons[button]->get_pos() == -1 || (!CurrentButtons[button]->is_always_shown() && !CPlayer::GetThisPlayer()->IsTeamed(*Selected[0]) && !CPlayer::GetThisPlayer()->HasBuildingAccess(*Selected[0]->Player, CurrentButtons[button]->Action)) || (!CurrentButtons[button]->is_always_shown() && !CPlayer::GetThisPlayer()->IsTeamed(*Selected[0]) && CPlayer::GetThisPlayer()->HasBuildingAccess(*Selected[0]->Player, CurrentButtons[button]->Action) && !IsNeutralUsableButtonAction(CurrentButtons[button]->Action))) { //allow neutral units to be used (but only for training or as transporters)
	//Wyrmgus end
		return;
	}

	//Wyrmgus start
	if (!IsButtonUsable(*Selected[0], *CurrentButtons[button])) {
		const CUpgrade *button_upgrade = CurrentButtons[button]->get_value_upgrade(Selected[0]);

		if (
			((CurrentButtons[button]->Action == ButtonCmd::Research || CurrentButtons[button]->Action == ButtonCmd::ResearchClass || CurrentButtons[button]->Action == ButtonCmd::Dynasty) && UpgradeIdAllowed(*CPlayer::GetThisPlayer(), button_upgrade->ID) == 'R')
			|| (CurrentButtons[button]->Action == ButtonCmd::LearnAbility && Selected[0]->GetIndividualUpgrade(button_upgrade) == button_upgrade->MaxLimit)
		) {
			CPlayer::GetThisPlayer()->Notify(NotifyYellow, Selected[0]->tilePos, Selected[0]->MapLayer->ID, "%s", _("The upgrade has already been acquired"));
		} else if (CurrentButtons[button]->Action == ButtonCmd::Buy && CPlayer::GetThisPlayer()->Heroes.size() >= PlayerHeroMax && CurrentButtons[button]->Value != -1 && UnitManager.GetSlotUnit(CurrentButtons[button]->Value).Character != nullptr) {
			CPlayer::GetThisPlayer()->Notify(NotifyYellow, Selected[0]->tilePos, Selected[0]->MapLayer->ID, "%s", _("The hero limit has been reached"));
		} else {
			CPlayer::GetThisPlayer()->Notify(NotifyYellow, Selected[0]->tilePos, Selected[0]->MapLayer->ID, "%s", _("The requirements have not been fulfilled"));
		}
		PlayGameSound(GameSounds.PlacementError[CPlayer::GetThisPlayer()->Race].Sound, MaxSampleVolume);
		return;
	}

	if (GetButtonCooldown(*Selected[0], *CurrentButtons[button]) > 0) {
		CPlayer::GetThisPlayer()->Notify(NotifyYellow, Selected[0]->tilePos, Selected[0]->MapLayer->ID, "%s", _("The cooldown is active"));
		PlayGameSound(GameSounds.PlacementError[CPlayer::GetThisPlayer()->Race].Sound, MaxSampleVolume);
		return;
	}
	//Wyrmgus end

	PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
	if (CurrentButtons[button]->CommentSound.Sound) {
		PlayGameSound(CurrentButtons[button]->CommentSound.Sound, MaxSampleVolume);
	}
	
	//  Handle action on button.
	switch (CurrentButtons[button]->Action) {
		case ButtonCmd::Unload: { DoClicked_Unload(button); break; }
		case ButtonCmd::SpellCast: { DoClicked_SpellCast(button); break; }
		case ButtonCmd::Repair: { DoClicked_Repair(button); break; }
		case ButtonCmd::Move:    // Follow Next
		case ButtonCmd::Patrol:  // Follow Next
		case ButtonCmd::Harvest: // Follow Next
		case ButtonCmd::Attack:  // Follow Next
		//Wyrmgus start
		case ButtonCmd::RallyPoint:
		case ButtonCmd::Unit:
		case ButtonCmd::EditorUnit:
		//Wyrmgus end
		case ButtonCmd::AttackGround: { DoClicked_SelectTarget(button); break; }
		case ButtonCmd::Return: { DoClicked_Return(); break; }
		case ButtonCmd::Stop: { DoClicked_Stop(); break; }
		case ButtonCmd::StandGround: { DoClicked_StandGround(); break; }
		case ButtonCmd::Button: { DoClicked_Button(button); break; }
		case ButtonCmd::Cancel: // Follow Next
		case ButtonCmd::CancelUpgrade: { DoClicked_CancelUpgrade(); break; }
		case ButtonCmd::CancelTrain: { DoClicked_CancelTrain(); break; }
		case ButtonCmd::CancelBuild: { DoClicked_CancelBuild(); break; }
		case ButtonCmd::Build:
		case ButtonCmd::BuildClass:
			DoClicked_Build(CurrentButtons[button]);
			break;
		case ButtonCmd::Train:
		case ButtonCmd::TrainClass:
			DoClicked_Train(CurrentButtons[button]);
			break;
		case ButtonCmd::UpgradeTo:
		case ButtonCmd::UpgradeToClass:
			DoClicked_UpgradeTo(CurrentButtons[button]);
			break;
		case ButtonCmd::Research:
		case ButtonCmd::ResearchClass:
		case ButtonCmd::Dynasty:
			DoClicked_Research(CurrentButtons[button]);
			break;
		case ButtonCmd::CallbackAction: { DoClicked_CallbackAction(button); break; }
		//Wyrmgus start
		case ButtonCmd::LearnAbility: { DoClicked_LearnAbility(button); break; }
		case ButtonCmd::ExperienceUpgradeTo: { DoClicked_ExperienceUpgradeTo(button); break; }
		case ButtonCmd::Faction: { DoClicked_Faction(button); break; }
		case ButtonCmd::Quest: { DoClicked_Quest(button); break; }
		case ButtonCmd::Buy: { DoClicked_Buy(button); break; }
		case ButtonCmd::ProduceResource: { DoClicked_ProduceResource(button); break; }
		case ButtonCmd::SellResource: { DoClicked_SellResource(button); break; }
		case ButtonCmd::BuyResource: { DoClicked_BuyResource(button); break; }
		case ButtonCmd::Salvage: { DoClicked_Salvage(); break; }
		case ButtonCmd::EnterMapLayer: { DoClicked_EnterMapLayer(); break; }
		//Wyrmgus end
	}
}

/**
**  Lookup key for bottom panel buttons.
**
**  @param key  Internal key symbol for pressed key.
**
**  @return     True, if button is handled (consumed).
*/
int CButtonPanel::DoKey(int key)
{
	SDL_keysym keysym;
	memset(&keysym, 0, sizeof(keysym));
	keysym.sym = (SDLKey)key;
	gcn::Key k = gcn::SDLInput::convertKeyCharacter(keysym);
	key = k.getValue();

	if (!CurrentButtons.empty()) {
		// This is required for action queues SHIFT+M should be `m'
		if (isascii(key) && isupper(key)) {
			key = tolower(key);
		}

		for (int i = 0; i < (int)UI.ButtonPanel.Buttons.size(); ++i) {
			if (CurrentButtons[i]->get_pos() != -1 && key == CurrentButtons[i]->get_key()) {
				UI.ButtonPanel.DoClicked(i);
				return 1;
			}
		}
	}
	return 0;
}
