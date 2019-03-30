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
/**@name species_category.h - The species category header file. */
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

#ifndef __SPECIES_CATEGORY_H__
#define __SPECIES_CATEGORY_H__

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "database.h"
#include "data_element.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CSpeciesCategoryRank;

/*----------------------------------------------------------------------------
--  Definition
----------------------------------------------------------------------------*/

class CSpeciesCategory : public DataElement, public Database<CSpeciesCategory>
{
	GDCLASS(CSpeciesCategory, DataElement)
	
public:
	static constexpr const char *GetClassIdentifier()
	{
		return "species_category";
	}
	
	virtual bool ProcessConfigDataProperty(const std::string &key, std::string value) override;
	
	String GetName() const
	{
		return this->Name.c_str();
	}
	
	String GetCommonName() const
	{
		return this->CommonName.c_str();
	}
	
	CSpeciesCategoryRank *GetRank() const
	{
		return this->Rank;
	}
	
	const std::vector<CSpeciesCategory *> &GetLowerCategory() const
	{
		return this->LowerCategories;
	}
	
	CSpeciesCategory *GetUpperCategory() const
	{
		return this->UpperCategory;
	}
	
private:
	std::string Name;							/// name of the species category
	std::string CommonName;						/// the common name of members of the species category
	CSpeciesCategoryRank *Rank = nullptr;		/// the rank of the species category
	std::vector<CSpeciesCategory *> LowerCategories;	/// the categories directly below this one
	CSpeciesCategory *UpperCategory = nullptr;	/// the category directly above this one

protected:
	static inline void _bind_methods() {}
};

#endif
