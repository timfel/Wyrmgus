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
/**@name interface.cpp - The interface source file. */
//
//      (c) Copyright 1998-2020 by Lutz Sammer, Jimmy Salmon, Pali Rohár and Andrettin
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

#include "ui/interface.h"

#include "ai.h"
#include "commands.h"
#include "database/defines.h"
//Wyrmgus start
#include "game.h"
//Wyrmgus end
#include "iolib.h"
#include "map/map.h"
#include "map/map_layer.h"
#include "map/minimap.h"
#include "map/minimap_mode.h"
#include "network.h"
#include "player.h"
#include "player_color.h"
#include "replay.h"
#include "script.h"
#include "sound/sound.h"
#include "sound/sound_server.h"
#include "sound/unit_sound_type.h"
#include "translate.h"
#include "ui/button.h"
#include "ui/cursor.h"
#include "ui/cursor_type.h"
#include "ui/ui.h"
#include "unit/unit.h"
#include "unit/unit_find.h"
#include "unit/unit_type.h"
#include "video/font.h"
#include "video/video.h"
#include "widgets.h"

/// Scrolling area (<= 15 y)
//Wyrmgus start
//static constexpr int SCROLL_UP = 15;
static constexpr int SCROLL_UP = 7;
//Wyrmgus end
/// Scrolling area (>= VideoHeight - 16 y)
//Wyrmgus start
//#define SCROLL_DOWN   (Video.Height - 16)
#define SCROLL_DOWN   (Video.Height - 8)
//Wyrmgus end
/// Scrolling area (<= 15 y)
//Wyrmgus start
//static constexpr int SCROLL_LEFT = 15;
static constexpr int SCROLL_LEFT = 7;
//Wyrmgus end
/// Scrolling area (>= VideoWidth - 16 x)
//Wyrmgus start
//#define SCROLL_RIGHT  (Video.Width - 16)
#define SCROLL_RIGHT  (Video.Width - 7)
//Wyrmgus end

static Vec2i SavedMapPosition[3];				/// Saved map position
static char Input[80];							/// line input for messages/long commands
static int InputIndex;							/// current index into input
static char InputStatusLine[99];				/// Last input status line
const char DefaultGroupKeys[] = "0123456789`";	/// Default group keys
std::string UiGroupKeys = DefaultGroupKeys;		/// Up to 11 keys, last unselect. Default for qwerty
bool GameRunning;								/// Current running state
bool GamePaused;								/// Current pause state
bool GameObserve;								/// Observe mode
bool GameEstablishing;							/// Game establishing mode
char SkipGameCycle;								/// Skip the next game cycle
char BigMapMode;								/// Show only the map
enum interface_state current_interface_state;	/// Current interface state
bool GodMode;									/// Invincibility cheat
enum _key_state_ KeyState;						/// current key state
CUnit *LastIdleWorker;							/// Last called idle worker
//Wyrmgus start
CUnit *LastLevelUpUnit;							/// Last called level up unit
//Wyrmgus end

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Show input.
*/
static void ShowInput()
{
	//Wyrmgus start
//	snprintf(InputStatusLine, sizeof(InputStatusLine), _("MESSAGE:%s~!_"), Input);
	snprintf(InputStatusLine, sizeof(InputStatusLine), _("Message: %s~!_"), Input);
	//Wyrmgus end
	char *input = InputStatusLine;
	// FIXME: This is slow!
	while (UI.StatusLine.Font->Width(input) > UI.StatusLine.Width) {
		++input;
	}
	KeyState = KeyStateCommand;
	UI.StatusLine.Clear();
	UI.StatusLine.Set(input);
	KeyState = KeyStateInput;
}

/**
**  Begin input.
*/
static void UiBeginInput()
{
	KeyState = KeyStateInput;
	Input[0] = '\0';
	InputIndex = 0;
	UI.StatusLine.ClearCosts();
	ShowInput();
}

//-----------------------------------------------------------------------------
//  User interface group commands
//-----------------------------------------------------------------------------

/**
**  Unselect all currently selected units.
*/
static void UiUnselectAll()
{
	UnSelectAll();
	NetworkSendSelection((CUnit **)nullptr, 0);
	SelectionChanged();
}

static void SetBestMapLayerForUnitGroup(const std::vector<CUnit *> &unit_group)
{
	int best_map_layer = UI.CurrentMapLayer->ID;
	
	std::vector<int> map_layer_count;
	for (size_t z = 0; z < CMap::Map.MapLayers.size(); ++z) {
		map_layer_count.push_back(0);
	}
	for (size_t i = 0; i != unit_group.size(); ++i) {
		map_layer_count[unit_group[i]->MapLayer->ID] += 1;
	}
	for (size_t i = 0; i < map_layer_count.size(); ++i) {
		if (map_layer_count[i] > map_layer_count[best_map_layer]) {
			best_map_layer = i;
		}
	}
	
	if (best_map_layer != UI.CurrentMapLayer->ID) {
		ChangeCurrentMapLayer(best_map_layer);
	}
}

static PixelPos GetMiddlePositionForUnitGroup(const std::vector<CUnit *> &unit_group)
{
	PixelPos pos(0, 0);
	
	int map_layer_units = 0;
	for (size_t i = 0; i != unit_group.size(); ++i) {
		if (unit_group[i]->MapLayer != UI.CurrentMapLayer) {
			continue;
		}
		pos += unit_group[i]->get_scaled_map_pixel_pos_center();
		map_layer_units++;
	}

	if (map_layer_units == 0) {
		return PixelPos(-1, -1);
	}	
	
	pos /= map_layer_units;
	
	return pos;
}

/**
**  Center on group.
**
**  @param group  Group number to center on.
**
**  @todo Improve this function, try to show all selected units
**        or the most possible units.
*/
static void UiCenterOnGroup(unsigned group, GroupSelectionMode mode = GroupSelectionMode::SELECTABLE_BY_RECTANGLE_ONLY)
{
	const std::vector<CUnit *> &units = GetUnitsOfGroup(group);

	std::vector<CUnit *> unit_group;
	for (size_t i = 0; i != units.size(); ++i) {
		if (units[i]->Type && units[i]->Type->CanSelect(mode)) {
			unit_group.push_back(units[i]);
		}
	}
	
	SetBestMapLayerForUnitGroup(unit_group);
	
	// FIXME: what should we do with the removed units? ignore?
	PixelPos pos = GetMiddlePositionForUnitGroup(unit_group);
	
	if (pos.x != -1) {
		UI.SelectedViewport->Center(pos);
	}
}

/**
**  Select group.
**
**  @param group  Group number to select.
*/
static void UiSelectGroup(unsigned group, GroupSelectionMode mode = GroupSelectionMode::SELECTABLE_BY_RECTANGLE_ONLY)
{
	SelectGroup(group, mode);
	SelectionChanged();
}

/**
**  Add group to current selection.
**
**  @param group  Group number to add.
*/
static void UiAddGroupToSelection(unsigned group)
{
	const std::vector<CUnit *> &units = GetUnitsOfGroup(group);

	if (units.empty()) {
		return;
	}

	//  Don't allow to mix units and buildings
	if (!Selected.empty() && Selected[0]->Type->BoolFlag[BUILDING_INDEX].value) {
		return;
	}

	for (size_t i = 0; i != units.size(); ++i) {
		if (!(units[i]->Removed || units[i]->Type->BoolFlag[BUILDING_INDEX].value)) {
			SelectUnit(*units[i]);
		}
	}
	SelectionChanged();
}

/**
**  Define a group. The current selected units become a new group.
**
**  @param group  Group number to create.
*/
static void UiDefineGroup(unsigned group)
{
	for (size_t i = 0; i != Selected.size(); ++i) {
		if (Selected[i]->Player == CPlayer::GetThisPlayer() && Selected[i]->GroupId) {
			RemoveUnitFromGroups(*Selected[i]);
		}
	}
	SetGroup(&Selected[0], Selected.size(), group);
}

/**
**  Add to group. The current selected units are added to the group.
**
**  @param group  Group number to be expanded.
*/
static void UiAddToGroup(unsigned group)
{
	AddToGroup(&Selected[0], Selected.size(), group);
}

/**
**  Toggle sound on / off.
*/
static void UiToggleSound()
{
	if (SoundEnabled()) {
		if (IsEffectsEnabled()) {
			SetEffectsEnabled(false);
			SetMusicEnabled(false);
		} else {
			SetEffectsEnabled(true);
			SetMusicEnabled(true);
			CheckMusicFinished(true);
		}
	}

	if (SoundEnabled()) {
		if (IsEffectsEnabled()) {
			UI.StatusLine.Set(_("Sound is on."));
		} else {
			UI.StatusLine.Set(_("Sound is off."));
		}
	}
}

/**
**  Toggle music on / off.
*/
static void UiToggleMusic()
{
	static int vol;
	if (SoundEnabled()) {
		if (GetMusicVolume()) {
			vol = GetMusicVolume();
			SetMusicVolume(0);
			UI.StatusLine.Set(_("Music is off."));
		} else {
			SetMusicVolume(vol);
			UI.StatusLine.Set(_("Music is on."));
		}
	}
}

/**
**  Toggle pause on / off.
*/
void UiTogglePause()
{
	if (!IsNetworkGame()) {
		GamePaused = !GamePaused;
		if (GamePaused) {
			UI.StatusLine.Set(_("Game Paused"));
		} else {
			UI.StatusLine.Set(_("Game Resumed"));
		}
	}
}

/**
**  Toggle big map mode.
**
**  @todo FIXME: We should try to keep the same view, if possible
*/
void UiToggleBigMap()
{
	static int mapx;
	static int mapy;
	static int mapex;
	static int mapey;

	BigMapMode ^= 1;
	if (BigMapMode) {
		mapx = UI.MapArea.X;
		mapy = UI.MapArea.Y;
		mapex = UI.MapArea.EndX;
		mapey = UI.MapArea.EndY;

		UI.MapArea.X = 0;
		UI.MapArea.Y = 0;
		UI.MapArea.EndX = Video.Width - 1;
		UI.MapArea.EndY = Video.Height - 1;

		SetViewportMode(UI.ViewportMode);

		UI.StatusLine.Set(_("Big map enabled"));
	} else {
		UI.MapArea.X = mapx;
		UI.MapArea.Y = mapy;
		UI.MapArea.EndX = mapex;
		UI.MapArea.EndY = mapey;

		SetViewportMode(UI.ViewportMode);

		//Wyrmgus start
//		UI.StatusLine.Set(_("Returning to old map"));
		UI.StatusLine.Set(_("Returning to the old map"));
		//Wyrmgus end
	}
}

/**
**  Increase game speed.
*/
static void UiIncreaseGameSpeed()
{
	if (FastForwardCycle >= GameCycle) {
		return;
	}
	VideoSyncSpeed += 10;
	SetVideoSync();
	//Wyrmgus start
//	UI.StatusLine.Set(_("Faster"));
	char buf[256];
	snprintf(buf, sizeof(buf), _("Game speed increased to %d"), CYCLES_PER_SECOND * VideoSyncSpeed / 100);
	UI.StatusLine.Set(buf);
	//Wyrmgus end
}

/**
**  Decrease game speed.
*/
static void UiDecreaseGameSpeed()
{
	if (FastForwardCycle >= GameCycle) {
		return;
	}
	if (VideoSyncSpeed <= 10) {
		if (VideoSyncSpeed > 1) {
			--VideoSyncSpeed;
		}
	} else {
		VideoSyncSpeed -= 10;
	}
	SetVideoSync();
	//Wyrmgus start
//	UI.StatusLine.Set(_("Slower"));
	char buf[256];
	snprintf(buf, sizeof(buf), _("Game speed decreased to %d"), CYCLES_PER_SECOND * VideoSyncSpeed / 100);
	UI.StatusLine.Set(buf);
	//Wyrmgus end
}

/**
**  Set default game speed.
*/
static void UiSetDefaultGameSpeed()
{
	if (FastForwardCycle >= GameCycle) {
		return;
	}
	VideoSyncSpeed = 100;
	SetVideoSync();
	//Wyrmgus start
//	UI.StatusLine.Set(_("Set default game speed"));
	UI.StatusLine.Set(_("Default game speed set"));
	//Wyrmgus end
}

/**
**  Center on the selected units.
**
**  @todo Improve this function, try to show all selected units
**        or the most possible units.
*/
static void UiCenterOnSelected()
{
	if (Selected.empty()) {
		return;
	}

	SetBestMapLayerForUnitGroup(Selected);

	PixelPos pos = GetMiddlePositionForUnitGroup(Selected);

	if (pos.x != -1) {
		UI.SelectedViewport->Center(pos);
	}
}

/**
**  Save current map position.
**
**  @param position  Map position slot.
*/
static void UiSaveMapPosition(unsigned position)
{
	SavedMapPosition[position] = UI.SelectedViewport->MapPos;
}

/**
**  Recall map position.
**
**  @param position  Map position slot.
*/
static void UiRecallMapPosition(unsigned position)
{
	UI.SelectedViewport->Set(SavedMapPosition[position], wyrmgus::defines::get()->get_scaled_tile_size() / 2);
}

void UiToggleMinimapMode()
{
	UI.get_minimap()->toggle_mode();
	UI.StatusLine.Set(_(wyrmgus::get_minimap_mode_name(UI.get_minimap()->get_mode())));
}

void UiToggleMinimapZoom()
{
	if (UI.get_minimap()->can_zoom(UI.CurrentMapLayer->ID)) {
		UI.get_minimap()->set_zoomed(!UI.get_minimap()->is_zoomed());
		if (UI.get_minimap()->is_zoomed()) {
			UI.StatusLine.Set(_("Zoomed Minimap"));
		} else {
			UI.StatusLine.Set(_("Unzoomed Minimap"));
		}
	}
}

/**
**  Find the next idle worker, select it, and center on it
*/
void UiFindIdleWorker()
{
	if (CPlayer::GetThisPlayer()->FreeWorkers.empty()) {
		return;
	}
	CUnit *unit = CPlayer::GetThisPlayer()->FreeWorkers[0];
	if (LastIdleWorker) {
		const std::vector<CUnit *> &freeWorkers = CPlayer::GetThisPlayer()->FreeWorkers;
		std::vector<CUnit *>::const_iterator it = std::find(freeWorkers.begin(),
															freeWorkers.end(),
															LastIdleWorker);
		if (it != CPlayer::GetThisPlayer()->FreeWorkers.end()) {
			if (*it != CPlayer::GetThisPlayer()->FreeWorkers.back()) {
				unit = *(++it);
			}
		}
	}

	if (unit != nullptr) {
		LastIdleWorker = unit;
		SelectSingleUnit(*unit);
		UI.StatusLine.Clear();
		UI.StatusLine.ClearCosts();
		CurrentButtonLevel = nullptr;
		PlayUnitSound(*Selected[0], wyrmgus::unit_sound_type::selected);
		SelectionChanged();
		if (unit->MapLayer != UI.CurrentMapLayer) {
			ChangeCurrentMapLayer(unit->MapLayer->ID);
		}
		UI.SelectedViewport->Center(unit->get_scaled_map_pixel_pos_center());
	}
}

//Wyrmgus start
/**
**  Find the next level up unit, select it, and center on it
*/
void UiFindLevelUpUnit()
{
	if (CPlayer::GetThisPlayer()->LevelUpUnits.empty()) {
		return;
	}
	CUnit *unit = CPlayer::GetThisPlayer()->LevelUpUnits[0];
	if (LastLevelUpUnit) {
		const std::vector<CUnit *> &levelUpUnits = CPlayer::GetThisPlayer()->LevelUpUnits;
		std::vector<CUnit *>::const_iterator it = std::find(levelUpUnits.begin(),
															levelUpUnits.end(),
															LastLevelUpUnit);
		if (it != CPlayer::GetThisPlayer()->LevelUpUnits.end()) {
			if (*it != CPlayer::GetThisPlayer()->LevelUpUnits.back()) {
				unit = *(++it);
			}
		}
	}

	if (unit != nullptr) {
		LastLevelUpUnit = unit;
		SelectSingleUnit(*unit);
		UI.StatusLine.Clear();
		UI.StatusLine.ClearCosts();
		CurrentButtonLevel = nullptr;
		PlayUnitSound(*Selected[0], wyrmgus::unit_sound_type::selected);
		SelectionChanged();
		if (unit->MapLayer != UI.CurrentMapLayer) {
			ChangeCurrentMapLayer(unit->MapLayer->ID);
		}
		UI.SelectedViewport->Center(unit->get_scaled_map_pixel_pos_center());
	}
}

/**
**  Find the next level up unit, select it, and center on it
*/
void UiFindHeroUnit(int hero_index)
{
	if (static_cast<int>(CPlayer::GetThisPlayer()->Heroes.size()) <= hero_index) {
		return;
	}
	CUnit *unit = CPlayer::GetThisPlayer()->Heroes[hero_index];

	SelectSingleUnit(*unit);
	UI.StatusLine.Clear();
	UI.StatusLine.ClearCosts();
	CurrentButtonLevel = nullptr;
	PlayUnitSound(*Selected[0], wyrmgus::unit_sound_type::selected);
	SelectionChanged();
	if (unit->MapLayer != UI.CurrentMapLayer) {
		ChangeCurrentMapLayer(unit->MapLayer->ID);
	}
	UI.SelectedViewport->Center(unit->get_scaled_map_pixel_pos_center());
}
//Wyrmgus end

/**
**  Toggle grab mouse on/off.
*/
static void UiToggleGrabMouse()
{
	DebugPrint("%x\n" _C_ KeyModifiers);
	ToggleGrabMouse(0);
	UI.StatusLine.Set(_("Grab mouse toggled."));
}

/**
**  Track unit, the viewport follows the unit.
*/
void UiTrackUnit()
{
	//Check if player has selected at least 1 unit
	if (Selected.empty()) {
		UI.SelectedViewport->Unit = nullptr;
		return;
	}
	if (UI.SelectedViewport->Unit == Selected[0]) {
		UI.SelectedViewport->Unit = nullptr;
	} else {
		UI.SelectedViewport->Unit = Selected[0];
	}
}

//Wyrmgus start
bool IsMouseLeftButtonPressed()
{
	return (MouseButtons & LeftButton) ? true : false;
}

int GetCurrentButtonValue()
{
	
	if (CurrentButtons.empty()) {
		return 0;
	}
	
	return CurrentButtons[ButtonUnderCursor]->Value;
}

std::string GetCurrentButtonValueStr()
{
	
	if (CurrentButtons.empty()) {
		return "";
	}
	
	return CurrentButtons[ButtonUnderCursor]->ValueStr;
}
//Wyrmgus end

/**
**  Call the lua function HandleCommandKey
*/
bool HandleCommandKey(int key)
{
	int base = lua_gettop(Lua);

	lua_getglobal(Lua, "HandleCommandKey");
	if (!lua_isfunction(Lua, -1)) {
		DebugPrint("No HandleCommandKey function in lua.\n");
		return false;
	}
	lua_pushstring(Lua, SdlKey2Str(key));
	lua_pushboolean(Lua, (KeyModifiers & ModifierControl));
	lua_pushboolean(Lua, (KeyModifiers & ModifierAlt));
	lua_pushboolean(Lua, (KeyModifiers & ModifierShift));
	LuaCall(4, 0);
	if (lua_gettop(Lua) - base == 1) {
		bool ret = LuaToBoolean(Lua, base + 1);
		lua_pop(Lua, 1);
		return ret;
	} else {
		LuaError(Lua, "HandleCommandKey must return a boolean");
		return false;
	}
}

#ifdef DEBUG
extern void ToggleShowBuilListMessages();
#endif

static void CommandKey_Group(int group)
{
	if (KeyModifiers & ModifierShift) {
		if (KeyModifiers & (ModifierAlt | ModifierDoublePress)) {
			if (KeyModifiers & ModifierDoublePress) {
				UiCenterOnGroup(group, GroupSelectionMode::SELECT_ALL);
			} else {
				UiSelectGroup(group, GroupSelectionMode::SELECT_ALL);
			}
		} else if (KeyModifiers & ModifierControl) {
			UiAddToGroup(group);
		} else {
			UiAddGroupToSelection(group);
		}
	} else {
		if (KeyModifiers & (ModifierAlt | ModifierDoublePress)) {
			if (KeyModifiers & ModifierAlt) {
				if (KeyModifiers & ModifierDoublePress) {
					UiCenterOnGroup(group, GroupSelectionMode::NON_SELECTABLE_BY_RECTANGLE_ONLY);
				} else {
					UiSelectGroup(group, GroupSelectionMode::NON_SELECTABLE_BY_RECTANGLE_ONLY);
				}
			} else {
				UiCenterOnGroup(group);
			}
		} else if (KeyModifiers & ModifierControl) {
			UiDefineGroup(group);
		} else {
			UiSelectGroup(group);
		}
	}
}

static void CommandKey_MapPosition(int index)
{
	if (KeyModifiers & ModifierShift) {
		UiSaveMapPosition(index);
	} else {
		UiRecallMapPosition(index);
	}
}

/**
**  Handle keys in command mode.
**
**  @param key  Key scancode.
**
**  @return     True, if key is handled; otherwise false.
*/
static bool CommandKey(int key)
{
	const char *ptr = strchr(UiGroupKeys.c_str(), key);

	// FIXME: don't handle unicode well. Should work on all latin keyboard.
	if (ptr) {
		key = '0' + ptr - UiGroupKeys.c_str();
		if (key > '9') {
			key = SDLK_BACKQUOTE;
		}
	}

	switch (key) {
		// Return enters chat/input mode.
		case SDLK_RETURN:
		case SDLK_KP_ENTER: // RETURN
			UiBeginInput();
			return true;

		// Unselect everything
		case SDLK_CARET:
		case SDLK_BACKQUOTE:
			UiUnselectAll();
			break;

		// Group selection
		case '0': case '1': case '2':
		case '3': case '4': case '5':
		case '6': case '7': case '8':
		case '9':
			CommandKey_Group(key - '0');
			break;

		case SDLK_F2:
		case SDLK_F3:
		case SDLK_F4: // Set/Goto place
			CommandKey_MapPosition(key - SDLK_F2);
			break;

		case SDLK_SPACE: // center on last action
			CenterOnMessage();
			break;

		case SDLK_EQUALS: // plus is shift-equals.
		case SDLK_KP_PLUS:
			UiIncreaseGameSpeed();
			break;

		case SDLK_MINUS: // - Slower
		case SDLK_KP_MINUS:
			UiDecreaseGameSpeed();
			break;
		case SDLK_KP_MULTIPLY:
			UiSetDefaultGameSpeed();
			break;

		case 'b': // ALT+B, CTRL+B Toggle big map
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			UiToggleBigMap();
			break;

		case 'c': // ALT+C, CTRL+C C center on units
			//Wyrmgus start
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			//Wyrmgus end
			UiCenterOnSelected();
			break;

		case 'f': // ALT+F, CTRL+F toggle fullscreen
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			ToggleFullScreen();
			//Wyrmgus start
			CclCommand("wyr.preferences.VideoFullScreen = Video.FullScreen;");
			//Wyrmgus end
			SavePreferences();
			break;

		case 'g': // ALT+G, CTRL+G grab mouse pointer
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			UiToggleGrabMouse();
			//Wyrmgus start
			CclCommand("wyr.preferences.GrabMouse = GetGrabMouse();");
			SavePreferences();
			//Wyrmgus end
			//Wyrmgus start
			HandleCommandKey(key);
			//Wyrmgus end
			break;

		case 'i':
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
		// FALL THROUGH
		case SDLK_PERIOD: // ., ALT+I, CTRL+I: Find idle worker
			UiFindIdleWorker();
			break;

		case 'l': // CTRL+L return to previous map layer
			if (KeyModifiers & ModifierControl) {
				ChangeToPreviousMapLayer();
				break;
			} else {
				if (HandleCommandKey(key)) {
					break;
				}
				return false;
			}
			break;

		case 'm': // CTRL+M Turn music on / off
			if (KeyModifiers & ModifierControl) {
				UiToggleMusic();
				SavePreferences();
				break;
			} else {
				if (HandleCommandKey(key)) {
					break;
				}
				return false;
			}
			break;

		case 'p': // CTRL+P, ALT+P Toggle pause
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
		// FALL THROUGH (CTRL+P, ALT+P)
		case SDLK_PAUSE:
			UiTogglePause();
			break;

		//Wyrmgus start
		case 'q': // Shift+Q: select all army units
			if (!(KeyModifiers & ModifierControl) && (KeyModifiers & (ModifierAlt))) {
				if (GameObserve || GamePaused || GameEstablishing) {
					break;
				}

				SelectArmy();
				
				UI.StatusLine.Clear();
				UI.StatusLine.ClearCosts();
				CurrentButtonLevel = nullptr;
				SelectionChanged();
				break;
			} else {
				if (HandleCommandKey(key)) {
					break;
				}
				return false;
			}
		//Wyrmgus end
		
		case 's': // CTRL+S - Turn sound on / off
			if (KeyModifiers & ModifierControl) {
				UiToggleSound();
				SavePreferences();
				break;
			//Wyrmgus start
			} else {
				if (HandleCommandKey(key)) {
					break;
				}
				return false;
			//Wyrmgus end
			}
			break;

		case 't': // ALT+T, CTRL+T Track unit
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			UiTrackUnit();
			break;

		case 'v': // ALT+V, CTRL+V: Viewport
			if (KeyModifiers & ModifierControl) {
				CycleViewportMode(-1);
			} else if (KeyModifiers & ModifierAlt) {
				CycleViewportMode(1);
			}
			break;

		//Wyrmgus start
		case 'w': // Alt+W: select all units of the same type as the first currently selected one
			if (!(KeyModifiers & ModifierControl) && (KeyModifiers & ModifierAlt)) {
				if (GameObserve || GamePaused || GameEstablishing) {
					break;
				}

				if (Selected.size() < 1) {
					break;
				}
				
				if (KeyModifiers & ModifierDoublePress) {
					SelectUnitsByType(*Selected[0], false);
				} else {
					SelectUnitsByType(*Selected[0], true);
				}
				
				UI.StatusLine.Clear();
				UI.StatusLine.ClearCosts();
				CurrentButtonLevel = nullptr;
				SelectionChanged();
				break;
			} else {
				if (HandleCommandKey(key)) {
					break;
				}
				return false;
			}
		//Wyrmgus end
		
		case 'e': // CTRL+E Turn messages on / off
			if (KeyModifiers & ModifierControl) {
				ToggleShowMessages();
			}
#ifdef DEBUG
			else if (KeyModifiers & ModifierAlt) {
				ToggleShowBuilListMessages();
			}
#endif
			break;

		case SDLK_TAB: // TAB toggles minimap.
			if (KeyModifiers & ModifierAlt) {
				break;
			}
			if (KeyModifiers & ModifierShift) {
				UiToggleMinimapZoom();
			} else {
				UiToggleMinimapMode();
			}
			break;

		case SDLK_UP:
		case SDLK_KP8:
			KeyScrollState |= ScrollUp;
			break;
		case SDLK_DOWN:
		case SDLK_KP2:
			KeyScrollState |= ScrollDown;
			break;
		case SDLK_LEFT:
		case SDLK_KP4:
			KeyScrollState |= ScrollLeft;
			break;
		case SDLK_RIGHT:
		case SDLK_KP6:
			KeyScrollState |= ScrollRight;
			break;

		default:
			if (HandleCommandKey(key)) {
				break;
			}
			return false;
	}
	return true;
}

/**
**  Handle cheats
**
**  @return  1 if a cheat was handled, 0 otherwise
*/
int HandleCheats(const std::string &input)
{
#if defined(DEBUG) || defined(PROF)
	if (input == "ai me") {
		if (ThisPlayer->AiEnabled) {
			// FIXME: UnitGoesUnderFog and UnitGoesOutOfFog change unit refs
			// for human players.  We can't switch back to a human player or
			// we'll be using the wrong ref counts.
#if 0
			ThisPlayer->AiEnabled = false;
			ThisPlayer->Type = PlayerPerson;
			SetMessage("AI is off, Normal Player");
#else
			SetMessage("Cannot disable 'ai me' cheat");
#endif
		} else {
			ThisPlayer->AiEnabled = true;
			ThisPlayer->Type = PlayerComputer;
			if (!ThisPlayer->Ai) {
				AiInit(*ThisPlayer);
			}
			SetMessage("I'm the BORG, resistance is futile!");
		}
		return 1;
	}
#endif
	int base = lua_gettop(Lua);
	lua_getglobal(Lua, "HandleCheats");
	if (!lua_isfunction(Lua, -1)) {
		DebugPrint("No HandleCheats function in lua.\n");
		return 0;
	}
	lua_pushstring(Lua, input.c_str());
	LuaCall(1, 0, false);
	if (lua_gettop(Lua) - base == 1) {
		int ret = LuaToBoolean(Lua, -1);
		lua_pop(Lua, 1);
		return ret;
	} else {
		DebugPrint("HandleCheats must return a boolean");
		return 0;
	}
}

// Replace ~~ with ~
static void Replace2TildeByTilde(char *s)
{
	for (char *p = s; *p; ++p) {
		if (*p == '~') {
			++p;
		}
		*s++ = *p;
	}
	*s = '\0';
}

// Replace ~ with ~~
static void ReplaceTildeBy2Tilde(char *s)
{
	for (char *p = s; *p; ++p) {
		if (*p != '~') {
			continue;
		}
		char *q = p + strlen(p);
		q[1] = '\0';
		while (q > p) {
			*q = *(q - 1);
			--q;
		}
		++p;
	}
}

/**
**  Handle keys in input mode.
**
**  @param key  Key scancode.
**  @return     True input finished.
*/
static int InputKey(int key)
{
	switch (key) {
		case SDLK_RETURN:
		case SDLK_KP_ENTER: { // RETURN
			// Replace ~~ with ~
			Replace2TildeByTilde(Input);
#ifdef DEBUG
			if (Input[0] == '-') {
				if (!GameObserve && !GamePaused && !GameEstablishing) {
					CommandLog("input", NoUnitP, FlushCommands, -1, -1, NoUnitP, Input, -1);
					CclCommand(Input + 1, false);
				}
			} else
#endif
				if (!IsNetworkGame()) {
					//Wyrmgus start
//					if (!GameObserve && !GamePaused && !GameEstablishing) {
					if (!GameObserve && !GamePaused && !GameEstablishing && !SaveGameLoading) {
					//Wyrmgus end
						if (HandleCheats(Input)) {
							CommandLog("input", NoUnitP, FlushCommands, -1, -1, NoUnitP, Input, -1);
						}
					}
				}

			// Check for Replay and ffw x
#ifdef DEBUG
			if (strncmp(Input, "ffw ", 4) == 0) {
#else
			if (strncmp(Input, "ffw ", 4) == 0 && ReplayGameType != ReplayNone) {
#endif
				FastForwardCycle = atoi(&Input[4]);
			}

			if (Input[0]) {
				// Replace ~ with ~~
				ReplaceTildeBy2Tilde(Input);
				char chatMessage[sizeof(Input) + 40];
				snprintf(chatMessage, sizeof(chatMessage), "~%s~<%s>~> %s",
					CPlayer::GetThisPlayer()->get_player_color()->get_identifier().c_str(),
					CPlayer::GetThisPlayer()->Name.c_str(), Input);
				// FIXME: only to selected players ...
				NetworkSendChatMessage(chatMessage);
			}
		}
	// FALL THROUGH
		case SDLK_ESCAPE:
			KeyState = KeyStateCommand;
			UI.StatusLine.Clear();
			return 1;

#ifdef USE_MAC
		case SDLK_DELETE:
#endif
		case SDLK_BACKSPACE:
			if (InputIndex) {
				if (Input[InputIndex - 1] == '~') {
					Input[--InputIndex] = '\0';
				}
				InputIndex = UTF8GetPrev(Input, InputIndex);
				if (InputIndex >= 0) {
					Input[InputIndex] = '\0';
					ShowInput();
				}
			}
			return 1;

		case SDLK_TAB: {
			char *namestart = strrchr(Input, ' ');
			if (namestart) {
				++namestart;
			} else {
				namestart = Input;
			}
			if (!strlen(namestart)) {
				return 1;
			}
			for (int i = 0; i < PlayerMax; ++i) {
				if (CPlayer::Players[i]->Type != PlayerPerson) {
					continue;
				}
				if (!strncasecmp(namestart, CPlayer::Players[i]->Name.c_str(), strlen(namestart))) {
					InputIndex += strlen(CPlayer::Players[i]->Name.c_str()) - strlen(namestart);
					strcpy_s(namestart, sizeof(Input) - (namestart - Input), CPlayer::Players[i]->Name.c_str());
					if (namestart == Input) {
						InputIndex += 2;
						strcat_s(namestart, sizeof(Input) - (namestart - Input), ": ");
					}
					ShowInput();
				}
			}
			return 1;
		}
		default:
			if (key >= ' ') {
				gcn::Key k(key);
				std::string kstr = k.toString();
				if (key == '~') {
					if (InputIndex < (int)sizeof(Input) - 2) {
						Input[InputIndex++] = key;
						Input[InputIndex++] = key;
						Input[InputIndex] = '\0';
						ShowInput();
					}
				} else if (InputIndex < (int)(sizeof(Input) - kstr.size())) {
					for (size_t i = 0; i < kstr.size(); ++i) {
						Input[InputIndex++] = kstr[i];
					}
					Input[InputIndex] = '\0';
					ShowInput();
				}
				return 1;
			}
			break;
	}
	return 0;
}

/**
**  Save a screenshot.
*/
static void Screenshot()
{
	CFile fd;
	char filename[30];

	for (int i = 1; i <= 99; ++i) {
		// FIXME: what if we can't write to this directory?
		snprintf(filename, sizeof(filename), "screen%02d.png", i);
		if (fd.open(filename, CL_OPEN_READ) == -1) {
			break;
		}
		fd.close();
	}
	SaveScreenshotPNG(filename);
}

/**
**  Update KeyModifiers if a key is pressed.
**
**  @param key      Key scancode.
**  @param keychar  Character code.
**
**  @return         1 if modifier found, 0 otherwise
*/
int HandleKeyModifiersDown(unsigned key, unsigned)
{
	switch (key) {
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			KeyModifiers |= ModifierShift;
			return 1;
		case SDLK_LCTRL:
		case SDLK_RCTRL:
			KeyModifiers |= ModifierControl;
			return 1;
		case SDLK_LALT:
		case SDLK_RALT:
		case SDLK_LMETA:
		case SDLK_RMETA:
			KeyModifiers |= ModifierAlt;
			// maxy: disabled
			if (current_interface_state == interface_state::normal) {
				SelectedUnitChanged(); // VLADI: to allow alt-buttons
			}
			return 1;
		case SDLK_LSUPER:
		case SDLK_RSUPER:
			KeyModifiers |= ModifierSuper;
			return 1;
		case SDLK_SYSREQ:
		case SDLK_PRINT:
			Screenshot();
			if (GameRunning) {
				SetMessage("%s", _("Screenshot made."));
			}
			return 1;
		default:
			break;
	}
	return 0;
}

/**
**  Update KeyModifiers if a key is released.
**
**  @param key      Key scancode.
**  @param keychar  Character code.
**
**  @return         1 if modifier found, 0 otherwise
*/
int HandleKeyModifiersUp(unsigned key, unsigned)
{
	switch (key) {
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			KeyModifiers &= ~ModifierShift;
			return 1;
		case SDLK_LCTRL:
		case SDLK_RCTRL:
			KeyModifiers &= ~ModifierControl;
			return 1;
		case SDLK_LALT:
		case SDLK_RALT:
		case SDLK_LMETA:
		case SDLK_RMETA:
			KeyModifiers &= ~ModifierAlt;
			// maxy: disabled
			if (current_interface_state == interface_state::normal) {
				SelectedUnitChanged(); // VLADI: to allow alt-buttons
			}
			return 1;
		case SDLK_LSUPER:
		case SDLK_RSUPER:
			KeyModifiers &= ~ModifierSuper;
			return 1;
	}
	return 0;
}

/**
**  Check if a key is from the keypad and convert to ascii
*/
static bool IsKeyPad(unsigned key, unsigned *kp)
{
	if (key >= SDLK_KP0 && key <= SDLK_KP9) {
		*kp = SDLK_0 + (key - SDLK_KP0);
	} else if (key == SDLK_KP_PERIOD) {
		*kp = SDLK_PERIOD;
	} else if (key == SDLK_KP_DIVIDE) {
		*kp = SDLK_SLASH;
	} else if (key == SDLK_KP_MULTIPLY) {
		*kp = SDLK_ASTERISK;
	} else if (key == SDLK_KP_MINUS) {
		*kp = SDLK_MINUS;
	} else if (key == SDLK_KP_PLUS) {
		*kp = SDLK_PLUS;
	} else if (key == SDLK_KP_ENTER) {
		*kp = SDLK_RETURN;
	} else if (key == SDLK_KP_EQUALS) {
		*kp = SDLK_EQUALS;
	} else  {
		*kp = SDLK_UNKNOWN;
		return false;
	}
	return true;
}

/**
**  Handle key down.
**
**  @param key      Key scancode.
**  @param keychar  Character code.
*/
void HandleKeyDown(unsigned key, unsigned keychar)
{
	if (HandleKeyModifiersDown(key, keychar)) {
		return;
	}

	// Handle All other keys

	// Command line input: for message or cheat
	unsigned kp = 0;
	if (KeyState == KeyStateInput && (keychar || IsKeyPad(key, &kp))) {
		InputKey(kp ? kp : keychar);
	} else {
		// If no modifier look if button bound
		if (!(KeyModifiers & (ModifierControl | ModifierAlt | ModifierSuper))) {
			if (!GameObserve && !GamePaused && !GameEstablishing) {
				if (UI.ButtonPanel.DoKey(key)) {
					return;
				}
			}
		}
		CommandKey(key);
	}
}

/**
**  Handle key up.
**
**  @param key      Key scancode.
**  @param keychar  Character code.
*/
void HandleKeyUp(unsigned key, unsigned keychar)
{
	if (HandleKeyModifiersUp(key, keychar)) {
		return;
	}

	switch (key) {
		case SDLK_UP:
		case SDLK_KP8:
			KeyScrollState &= ~ScrollUp;
			break;
		case SDLK_DOWN:
		case SDLK_KP2:
			KeyScrollState &= ~ScrollDown;
			break;
		case SDLK_LEFT:
		case SDLK_KP4:
			KeyScrollState &= ~ScrollLeft;
			break;
		case SDLK_RIGHT:
		case SDLK_KP6:
			KeyScrollState &= ~ScrollRight;
			break;
		default:
			break;
	}
}

/**
**  Handle key up.
**
**  @param key      Key scancode.
**  @param keychar  Character code.
*/
void HandleKeyRepeat(unsigned, unsigned keychar)
{
	if (KeyState == KeyStateInput && keychar) {
		InputKey(keychar);
	}
}

/**
**  Handle the mouse in scroll area
**
**  @param mousePos  Screen position.
**
**  @return   true if the mouse is in the scroll area, false otherwise
*/
bool HandleMouseScrollArea(const PixelPos &mousePos)
{
	//Wyrmgus start
	if (!UI.MapArea.EndX || !UI.MapArea.EndY || !UI.SelectedViewport) {
		return false;
	}
	const PixelPos top_left_pos(UI.MapArea.X, UI.MapArea.Y);
	const PixelPos top_left_map_pos = UI.SelectedViewport->screen_to_scaled_map_pixel_pos(top_left_pos);
	const PixelPos bottom_right_pos(UI.MapArea.EndX, UI.MapArea.EndY);
	const PixelPos bottom_right_map_pos = UI.SelectedViewport->screen_to_scaled_map_pixel_pos(bottom_right_pos);
//	if (mousePos.x < SCROLL_LEFT) {
	if (mousePos.x < SCROLL_LEFT && top_left_map_pos.x > 0) {
	//Wyrmgus end
		//Wyrmgus start
//		if (mousePos.y < SCROLL_UP) {
		if (mousePos.y < SCROLL_UP && top_left_map_pos.y > 0) {
		//Wyrmgus end
			CursorOn = cursor_on::scroll_left_up;
			MouseScrollState = ScrollLeftUp;
			GameCursor = UI.get_cursor(wyrmgus::cursor_type::arrow_northwest);
		//Wyrmgus start
//		} else if (mousePos.y > SCROLL_DOWN) {
		} else if (mousePos.y > SCROLL_DOWN && bottom_right_map_pos.y < (UI.CurrentMapLayer->get_height() * wyrmgus::defines::get()->get_scaled_tile_height()) - 1) {
		//Wyrmgus end
			CursorOn = cursor_on::scroll_left_down;
			MouseScrollState = ScrollLeftDown;
			GameCursor = UI.get_cursor(wyrmgus::cursor_type::arrow_southwest);
		} else {
			CursorOn = cursor_on::scroll_left;
			MouseScrollState = ScrollLeft;
			GameCursor = UI.get_cursor(wyrmgus::cursor_type::arrow_west);
		}
	//Wyrmgus start
//	} else if (mousePos.x > SCROLL_RIGHT) {
	} else if (mousePos.x > SCROLL_RIGHT && bottom_right_map_pos.x < (UI.CurrentMapLayer->get_width() * wyrmgus::defines::get()->get_scaled_tile_width()) - 1) {
	//Wyrmgus end
		//Wyrmgus start
//		if (mousePos.y < SCROLL_UP) {
		if (mousePos.y < SCROLL_UP && top_left_map_pos.y > 0) {
		//Wyrmgus end
			CursorOn = cursor_on::scroll_right_up;
			MouseScrollState = ScrollRightUp;
			GameCursor = UI.get_cursor(wyrmgus::cursor_type::arrow_northeast);
		//Wyrmgus start
//		} else if (mousePos.y > SCROLL_DOWN) {
		} else if (mousePos.y > SCROLL_DOWN && bottom_right_map_pos.y < (UI.CurrentMapLayer->get_height() * wyrmgus::defines::get()->get_scaled_tile_height()) - 1) {
		//Wyrmgus end
			CursorOn = cursor_on::scroll_right_down;
			MouseScrollState = ScrollRightDown;
			GameCursor = UI.get_cursor(wyrmgus::cursor_type::arrow_southeast);
		} else {
			CursorOn = cursor_on::scroll_right;
			MouseScrollState = ScrollRight;
			GameCursor = UI.get_cursor(wyrmgus::cursor_type::arrow_east);
		}
	} else {
		//Wyrmgus start
//		if (mousePos.y < SCROLL_UP) {
		if (mousePos.y < SCROLL_UP && top_left_map_pos.y > 0) {
		//Wyrmgus end
			CursorOn = cursor_on::scroll_up;
			MouseScrollState = ScrollUp;
			GameCursor = UI.get_cursor(wyrmgus::cursor_type::arrow_north);
			//Wyrmgus start
//		} else if (mousePos.y > SCROLL_DOWN) {
		} else if (mousePos.y > SCROLL_DOWN && bottom_right_map_pos.y < (UI.CurrentMapLayer->get_height() * wyrmgus::defines::get()->get_scaled_tile_height()) - 1) {
		//Wyrmgus end
			CursorOn = cursor_on::scroll_down;
			MouseScrollState = ScrollDown;
			GameCursor = UI.get_cursor(wyrmgus::cursor_type::arrow_south);
		} else {
			return false;
		}
	}
	return true;
}

/**
**  Keep coordinates in window and update cursor position
**
**  @param x  screen pixel X position.
**  @param y  screen pixel Y position.
*/
void HandleCursorMove(int *x, int *y)
{
	//  Reduce coordinates to window-size.
	clamp(x, 0, Video.Width - 1);
	clamp(y, 0, Video.Height - 1);
	CursorScreenPos.x = *x;
	CursorScreenPos.y = *y;
}

/**
**  Handle movement of the cursor.
**
**  @param screePos  screen pixel position.
*/
void HandleMouseMove(const PixelPos &screenPos)
{
	PixelPos pos(screenPos);
	HandleCursorMove(&pos.x, &pos.y);
	UIHandleMouseMove(pos);
}

/**
**  Called if mouse button pressed down.
**
**  @param button  Mouse button number (0 left, 1 middle, 2 right)
*/
void HandleButtonDown(unsigned button)
{
	UIHandleButtonDown(button);
}

/**
**  Called if mouse button released.
**
**  @todo FIXME: the mouse handling should be complete rewritten
**  @todo FIXME: this is needed for double click, long click,...
**
**  @param button  Mouse button number (0 left, 1 middle, 2 right)
*/
void HandleButtonUp(unsigned button)
{
	UIHandleButtonUp(button);
}

/*----------------------------------------------------------------------------
--  Lowlevel input functions
----------------------------------------------------------------------------*/

#ifdef USE_TOUCHSCREEN
int DoubleClickDelay = 1000;	/// Time to detect double clicks.
int HoldClickDelay = 2000;		/// Time to detect hold clicks.
#else
int DoubleClickDelay = 300;		/// Time to detect double clicks.
int HoldClickDelay = 1000;		/// Time to detect hold clicks.
#endif

static enum {
	InitialMouseState,			/// start state
	ClickedMouseState			/// button is clicked
} MouseState;					/// Current state of mouse

static PixelPos LastMousePos;		/// Last mouse position
static unsigned LastMouseButton;	/// last mouse button handled
static unsigned StartMouseTicks;	/// Ticks of first click
static unsigned LastMouseTicks;		/// Ticks of last mouse event

/**
**  Called if any mouse button is pressed down
**
**  Handles event conversion to double click, dragging, hold.
**
**  FIXME: dragging is not supported.
**
**  @param callbacks  Callback structure for events.
**  @param ticks      Denotes time-stamp of video-system
**  @param button     Mouse button pressed.
*/
void InputMouseButtonPress(const EventCallback &callbacks,
						   unsigned ticks, unsigned button)
{
	//  Button new pressed.
	if (!(MouseButtons & (1 << button))) {
		MouseButtons |= (1 << button);
		//  Detect double click
		if (MouseState == ClickedMouseState && button == LastMouseButton
			&& ticks < StartMouseTicks + DoubleClickDelay) {
			MouseButtons |= (1 << button) << MouseDoubleShift;
			button |= button << MouseDoubleShift;
		} else {
			MouseState = InitialMouseState;
			StartMouseTicks = ticks;
			LastMouseButton = button;
		}
	}
	LastMouseTicks = ticks;

	callbacks.ButtonPressed(button);
}

/**
**  Called if any mouse button is released up
**
**  @param callbacks  Callback structure for events.
**  @param ticks      Denotes time-stamp of video-system
**  @param button     Mouse button released.
*/
void InputMouseButtonRelease(const EventCallback &callbacks,
							 unsigned ticks, unsigned button)
{
	//  Same button before pressed.
	if (button == LastMouseButton && MouseState == InitialMouseState) {
		MouseState = ClickedMouseState;
	} else {
		LastMouseButton = 0;
		MouseState = InitialMouseState;
	}
	LastMouseTicks = ticks;

	unsigned mask = 0;
	if (MouseButtons & ((1 << button) << MouseDoubleShift)) {
		mask |= button << MouseDoubleShift;
	}
	if (MouseButtons & ((1 << button) << MouseDragShift)) {
		mask |= button << MouseDragShift;
	}
	if (MouseButtons & ((1 << button) << MouseHoldShift)) {
		mask |= button << MouseHoldShift;
	}
	MouseButtons &= ~(0x01010101 << button);

	callbacks.ButtonReleased(button | mask);
}

/**
**  Called if the mouse is moved
**
**  @param callbacks  Callback structure for events.
**  @param ticks      Denotes time-stamp of video-system
**  @param x          X movement
**  @param y          Y movement
*/
void InputMouseMove(const EventCallback &callbacks,
					unsigned ticks, int x, int y)
{
	PixelPos mousePos(x, y);
	// Don't reset the mouse state unless we really moved
#ifdef USE_TOUCHSCREEN
	const int buff = 32;
	const PixelDiff diff = LastMousePos - mousePos;

	if (abs(diff.x) > buff || abs(diff.y) > buff) {
		MouseState = InitialMouseState;
		LastMouseTicks = ticks;
		// Reset rectangle select cursor state if we moved by a lot
		// - rectangle select should be a drag, not a tap
		if (CurrentCursorState == CursorState::Rectangle
			&& (abs(diff.x) > 2 * buff || abs(diff.y) > 2 * buff)) {
			CursorState = CursorState::Point;
		}
	}
	LastMousePos = mousePos;
#else
	if (LastMousePos != mousePos) {
		MouseState = InitialMouseState;
		LastMouseTicks = ticks;
		LastMousePos = mousePos;
	}
#endif
	callbacks.MouseMoved(mousePos);
}

/**
**  Called if the mouse exits the game window (when supported by videomode)
**
**  @param callbacks  Callback structure for events.
**  @param ticks      Denotes time-stamp of video-system
*/
void InputMouseExit(const EventCallback &callbacks, unsigned /* ticks */)
{
	// FIXME: should we do anything here with ticks? don't know, but conform others
	// JOHNS: called by callback HandleMouseExit();
	callbacks.MouseExit();
}

/**
**  Called each frame to handle mouse timeouts.
**
**  @param callbacks  Callback structure for events.
**  @param ticks      Denotes time-stamp of video-system
*/
void InputMouseTimeout(const EventCallback &callbacks, unsigned ticks)
{
	if (MouseButtons & (1 << LastMouseButton)) {
		if (ticks > StartMouseTicks + DoubleClickDelay) {
			MouseState = InitialMouseState;
		}
		if (ticks > LastMouseTicks + HoldClickDelay) {
			LastMouseTicks = ticks;
			MouseButtons |= (1 << LastMouseButton) << MouseHoldShift;
			callbacks.ButtonPressed(LastMouseButton | (LastMouseButton << MouseHoldShift));
		}
	}
}


static const int HoldKeyDelay = 250;			/// Time to detect hold key
static const int HoldKeyAdditionalDelay = 50;	/// Time to detect additional hold key

static unsigned LastIKey;						/// last key handled
static unsigned LastIKeyChar;					/// last keychar handled
static unsigned LastKeyTicks;					/// Ticks of last key
static unsigned DoubleKey;						/// last key pressed

/**
**  Handle keyboard key press.
**
**  @param callbacks  Callback structure for events.
**  @param ticks      Denotes time-stamp of video-system
**  @param ikey       Key scancode.
**  @param ikeychar   Character code.
*/
void InputKeyButtonPress(const EventCallback &callbacks,
						 unsigned ticks, unsigned ikey, unsigned ikeychar)
{
	if (!LastIKey && DoubleKey == ikey && ticks < LastKeyTicks + DoubleClickDelay) {
		KeyModifiers |= ModifierDoublePress;
	}
	DoubleKey = ikey;
	LastIKey = ikey;
	LastIKeyChar = ikeychar;
	LastKeyTicks = ticks;
	callbacks.KeyPressed(ikey, ikeychar);
	KeyModifiers &= ~ModifierDoublePress;
}

/**
**  Handle keyboard key release.
**
**  @param callbacks  Callback structure for events.
**  @param ticks      Denotes time-stamp of video-system
**  @param ikey       Key scancode.
**  @param ikeychar   Character code.
*/
void InputKeyButtonRelease(const EventCallback &callbacks,
						   unsigned ticks, unsigned ikey, unsigned ikeychar)
{
	if (ikey == LastIKey) {
		LastIKey = 0;
	}
	LastKeyTicks = ticks;
	callbacks.KeyReleased(ikey, ikeychar);
}

/**
**  Called each frame to handle keyboard timeouts.
**
**  @param callbacks  Callback structure for events.
**  @param ticks      Denotes time-stamp of video-system
*/
void InputKeyTimeout(const EventCallback &callbacks, unsigned ticks)
{
	if (LastIKey && ticks > LastKeyTicks + HoldKeyDelay) {
		LastKeyTicks = ticks - (HoldKeyDelay - HoldKeyAdditionalDelay);
		callbacks.KeyRepeated(LastIKey, LastIKeyChar);
	}
}

/**
**  Get double click delay
*/
int GetDoubleClickDelay()
{
	return DoubleClickDelay;
}

/**
**  Set double click delay
**
**  @param delay  Double click delay
*/
void SetDoubleClickDelay(int delay)
{
	DoubleClickDelay = delay;
}

/**
**  Get hold click delay
*/
int GetHoldClickDelay()
{
	return HoldClickDelay;
}

/**
**  Set hold click delay
**
**  @param delay  Hold click delay
*/
void SetHoldClickDelay(int delay)
{
	HoldClickDelay = delay;
}
