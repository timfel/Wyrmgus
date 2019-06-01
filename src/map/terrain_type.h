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
/**@name terrain_type.h - The terrain type header file. */
//
//      (c) Copyright 2018-2019 by Andrettin
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

#ifndef __TERRAIN_TYPE_H__
#define __TERRAIN_TYPE_H__

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "data_element.h"
#include "data_type.h"
#include "vec2i.h"
#include "video/color.h"

#include <map>
#include <tuple>

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CGraphic;
class CPlayerColorGraphic;
class CSeason;
class CUnitType;
struct lua_State;

/*----------------------------------------------------------------------------
--  Definition
----------------------------------------------------------------------------*/

class CTerrainType : public DataElement, public DataType<CTerrainType>
{
	DATA_TYPE(CTerrainType, DataElement)

public:
	CTerrainType()
	{
		Color.R = 0;
		Color.G = 0;
		Color.B = 0;
		Color.A = 0;
	}
	
	~CTerrainType();
	
public:
	static constexpr const char *ClassIdentifier = "terrain_type";
	
	static void LoadTerrainTypeGraphics();
	static void Clear();
	static uint16_t GetTerrainFlagByName(const std::string &flag_name);
	
	static std::map<std::string, CTerrainType *> TerrainTypesByCharacter;
	static std::map<std::tuple<int, int, int>, CTerrainType *> TerrainTypesByColor;

	virtual bool ProcessConfigDataProperty(const std::string &key, std::string value) override;
	virtual bool ProcessConfigDataSection(const CConfigData *section) override;
	
	uint16_t GetFlags() const
	{
		return this->Flags;
	}
	
	CGraphic *GetGraphics(const CSeason *season = nullptr) const;
	
	bool IsTree() const
	{
		return this->Tree;
	}

	bool IsRock() const
	{
		return this->Rock;
	}

	bool IsDesert() const
	{
		return this->Desert;
	}

	bool IsSwamp() const
	{
		return this->Swamp;
	}

	std::string Character;
	CColor Color;
	int SolidAnimationFrames = 0;
	int Resource = -1;
private:
	uint16_t Flags = 0;
public:
	bool Overlay = false;										/// Whether this terrain type belongs to the overlay layer
	bool Buildable = false;										/// Whether structures can be built upon this terrain type
	bool AllowSingle = false;									/// Whether this terrain type has transitions for single tiles
	bool Hidden = false;
	bool Tree = false;
	bool Rock = false;
	bool Desert = false;
	bool Swamp = false;
	PixelSize PixelTileSize = PixelSize(32, 32);
	CUnitType *UnitType = nullptr;
private:
	CGraphic *Graphics = nullptr;
	std::map<const CSeason *, CGraphic *> SeasonGraphics;		/// Graphics to be displayed instead of the normal ones during particular seasons
public:
	CGraphic *ElevationGraphics = nullptr;						/// Semi-transparent elevation graphics, separated so that borders look better
	CPlayerColorGraphic *PlayerColorGraphics = nullptr;
	std::vector<CTerrainType *> BaseTerrainTypes;				/// Possible base terrain types for this terrain type (if it is an overlay terrain)
	std::vector<CTerrainType *> BorderTerrains;					/// Terrain types which this one can border
	std::vector<CTerrainType *> InnerBorderTerrains;			/// Terrain types which this one can border, and which "enter" this tile type in transitions
	std::vector<CTerrainType *> OuterBorderTerrains;			/// Terrain types which this one can border, and which are "entered" by this tile type in transitions
	std::vector<int> SolidTiles;
	std::vector<int> DamagedTiles;
	std::vector<int> DestroyedTiles;
	std::map<std::tuple<int, int>, std::vector<int>> TransitionTiles;	/// Transition graphics, mapped to the tile type (-1 means any tile) and the transition type (i.e. northeast outer)
	std::map<std::tuple<int, int>, std::vector<int>> AdjacentTransitionTiles;	/// Transition graphics for the tiles adjacent to this terrain type, mapped to the tile type (-1 means any tile) and the transition type (i.e. northeast outer)
	
	friend int CclDefineTerrainType(lua_State *l);

protected:
	static void _bind_methods();
};

#endif
