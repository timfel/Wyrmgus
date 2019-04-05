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
/**@name language.h - The language header file. */
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

#ifndef __LANGUAGE_H__
#define __LANGUAGE_H__

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "data_element.h"
#include "data_type.h"
#include "word.h" // for certain enums

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CWord;

/*----------------------------------------------------------------------------
--  Definition
----------------------------------------------------------------------------*/

class CLanguage : public DataElement, public DataType<CLanguage>
{
	GDCLASS(CLanguage, DataElement)

public:
	static constexpr const char *ClassIdentifier = "language";
	
	CWord *GetWord(const String &name, const int word_type, const std::vector<String> &word_meanings) const;
	String GetArticle(const int gender, const int grammatical_case, const int article_type, const int grammatical_number);
	String GetNounEnding(const int grammatical_number, const int grammatical_case, int word_junction_type = -1);
	String GetAdjectiveEnding(const int article_type, const int grammatical_case, int grammatical_number, int grammatical_gender);
	void RemoveWord(CWord *word);
	
	String TranslateName(const String &name) const;
	
public:
	String Family;									/// Family of the language
	String NounEndings[MaxGrammaticalNumbers][MaxGrammaticalCases][MaxWordJunctionTypes];
	String AdjectiveEndings[MaxArticleTypes][MaxGrammaticalCases][MaxGrammaticalNumbers][MaxGrammaticalGenders];
	bool UsedByCivilizationOrFaction = false;
	CLanguage *DialectOf = nullptr;	/// Of which language this is a dialect of (if at all); dialects inherit the words from the parent language unless specified otherwise
	std::vector<CLanguage *> Dialects;				/// Dialects of this language
	std::vector<CWord *> Words;						/// Words of the language
	std::map<String, std::vector<String>> NameTranslations;	/// Name translations; possible translations mapped to the name to be translated

	friend int CclDefineLanguage(lua_State *l);
	
protected:
	static void _bind_methods();
};

#endif
