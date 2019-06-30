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
/**@name action_research.cpp - The research action. */
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

#include "action/action_research.h"

#include "ai/ai.h"
#include "animation/animation.h"
#include "iolib.h"
#include "map/map_layer.h"
#include "script.h"
#include "sound/sound.h"
#include "sound/unit_sound.h"
#include "player.h"
#include "translate.h"
#include "unit/unit.h"
#include "unit/unit_type.h"
#include "upgrade/upgrade_structs.h"
#include "upgrade/upgrade.h"

/// How many resources the player gets back if canceling research
constexpr int CANCEL_RESEARCH_COSTS_FACTOR = 100;


/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

//Wyrmgus start
///* static */ COrder *COrder::NewActionResearch(CUnit &unit, const CUpgrade &upgrade)
/* static */ COrder *COrder::NewActionResearch(CUnit &unit, const CUpgrade &upgrade, int player)
//Wyrmgus end
{
	COrder_Research *order = new COrder_Research();

	// FIXME: if you give quick an other order, the resources are lost!
	//Wyrmgus start
	order->Player = player;
//	unit.GetPlayer()->SubCosts(upgrade.Costs);
	int upgrade_costs[MaxCosts];
	CPlayer::Players[player]->GetUpgradeCosts(&upgrade, upgrade_costs);
	CPlayer::Players[player]->SubCosts(upgrade_costs);
	//Wyrmgus end

	order->SetUpgrade(upgrade);
	return order;
}

/* virtual */ void COrder_Research::Save(CFile &file, const CUnit &unit) const
{
	file.printf("{\"action-research\",");

	if (this->Finished) {
		file.printf(" \"finished\", ");
	}
	//Wyrmgus start
	file.printf("\"player\", %d,", this->Player);
	//Wyrmgus end
	if (this->Upgrade) {
		file.printf(" \"upgrade\", \"%s\"", this->Upgrade->Ident.c_str());
	}
	file.printf("}");
}

/* virtual */ bool COrder_Research::ParseSpecificData(lua_State *l, int &j, const char *value, CUnit &unit)
{
	if (!strcmp(value, "upgrade")) {
		++j;
		this->Upgrade = CUpgrade::Get(LuaToString(l, -1, j + 1));
	//Wyrmgus start
	} else if (!strcmp(value, "player")) {
		++j;
		this->Player = LuaToNumber(l, -1, j + 1);
	//Wyrmgus end
	} else {
		return false;
	}
	return true;
}

/* virtual */ bool COrder_Research::IsValid() const
{
	return true;
}

/* virtual */ Vector2i COrder_Research::Show(const CViewport &, const Vector2i &lastScreenPos) const
{
	return lastScreenPos;
}

/* virtual */ void COrder_Research::UpdateUnitVariables(CUnit &unit) const
{
	//Wyrmgus start
//	unit.Variable[RESEARCH_INDEX].Value = unit.GetPlayer()->UpgradeTimers.Upgrades[this->Upgrade->GetIndex()];
	unit.Variable[RESEARCH_INDEX].Value = CPlayer::Players[this->Player]->UpgradeTimers.Upgrades[this->Upgrade->GetIndex()];
	//Wyrmgus end
	unit.Variable[RESEARCH_INDEX].Max = this->Upgrade->Costs[TimeCost];
}

/**
**  Research upgrade.
**
**  @return true when finished.
*/
/* virtual */ void COrder_Research::Execute(CUnit &unit)
{
	const CUpgrade &upgrade = this->GetUpgrade();
	const CUnitType &type = *unit.GetType();


	//Wyrmgus start
//	UnitShowAnimation(unit, type.Animations->Research ? type.Animations->Research : type.Animations->Still);
	UnitShowAnimation(unit, unit.GetAnimations()->Research ? unit.GetAnimations()->Research : unit.GetAnimations()->Still);
	//Wyrmgus end
	if (unit.Wait) {
		unit.Wait--;
		return ;
	}
#if 0
	if (unit.Anim.Unbreakable) {
		return ;
	}
#endif
	//Wyrmgus start
//	CPlayer &player = *unit.GetPlayer();
	CPlayer &player = *CPlayer::Players[this->Player];
//	player.UpgradeTimers.Upgrades[upgrade.GetIndex()] += std::max(1, player.SpeedResearch / SPEEDUP_FACTOR);
	player.UpgradeTimers.Upgrades[upgrade.GetIndex()] += std::max(1, (player.SpeedResearch + unit.Variable[TIMEEFFICIENCYBONUS_INDEX].Value + unit.Variable[RESEARCHSPEEDBONUS_INDEX].Value) / SPEEDUP_FACTOR);
	//Wyrmgus end
	if (player.UpgradeTimers.Upgrades[upgrade.GetIndex()] >= upgrade.Costs[TimeCost]) {
		if (upgrade.GetName().empty()) {
			//Wyrmgus start
//			player.Notify(NotifyGreen, unit.GetTilePos(), _("%s: research complete"), type.Name.c_str());
			player.Notify(NotifyGreen, unit.GetTilePos(), unit.GetMapLayer()->GetIndex(), _("%s: research complete"), type.GetDefaultName(&player).c_str());
			//Wyrmgus end
		} else {
			player.Notify(NotifyGreen, unit.GetTilePos(), unit.GetMapLayer()->GetIndex(), _("%s: research complete"), upgrade.GetName().utf8().get_data());
		}
		if (&player == CPlayer::GetThisPlayer()) {
			//Wyrmgus start
//			CSound *sound = GameSounds.ResearchComplete[player.Race].Sound;
			CSound *sound = GameSounds.ResearchComplete[unit.GetPlayer()->Race].Sound;
			//Wyrmgus end

			if (sound) {
				PlayGameSound(sound, MaxSampleVolume);
			}
		}
		if (player.AiEnabled) {
			AiResearchComplete(unit, &upgrade);
		}
		UpgradeAcquire(player, &upgrade);
		this->Finished = true;
		return ;
	}
	unit.Wait = CYCLES_PER_SECOND / 6;
}

/* virtual */ void COrder_Research::Cancel(CUnit &unit)
{
	const CUpgrade &upgrade = this->GetUpgrade();
	//Wyrmgus start
//	unit.GetPlayer()->UpgradeTimers.Upgrades[upgrade.GetIndex()] = 0;

//	unit.GetPlayer()->AddCostsFactor(upgrade.Costs, CANCEL_RESEARCH_COSTS_FACTOR);
	CPlayer::Players[this->Player]->UpgradeTimers.Upgrades[upgrade.GetIndex()] = 0;

	CPlayer::Players[this->Player]->AddCostsFactor(upgrade.Costs, CANCEL_RESEARCH_COSTS_FACTOR);
	//Wyrmgus end
}
