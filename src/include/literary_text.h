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
/**@name literary_text.h - The literary text header file. */
//
//      (c) Copyright 2016-2019 by Andrettin
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

#ifndef __LITERARY_TEXT_H__
#define __LITERARY_TEXT_H__

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "data_element.h"
#include "data_type.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CIcon;
class CLiteraryTextPage;

/*----------------------------------------------------------------------------
--  Definition
----------------------------------------------------------------------------*/

class CLiteraryText : public DataElement, public DataType<CLiteraryText>
{
	GDCLASS(CLiteraryText, DataElement)
	
public:
	CLiteraryText() {}

	static constexpr const char *ClassIdentifier = "literary_text";
	
	virtual bool ProcessConfigDataSection(const CConfigData *section) override;
	virtual void Initialize() override;
	
public:
	CIcon *GetIcon() const
	{
		if (this->Icon != nullptr) {
			return this->Icon;
		} else if (this->GetMainText() != nullptr) {
			return this->GetMainText()->GetIcon();
		} else {
			return nullptr;
		}
	}
	
	CLiteraryText *GetMainText() const { return this->MainText; }
	
	const std::vector<CLiteraryText *> &GetSections() const { return this->Sections; }
	
	CLiteraryText *GetPreviousSection() const { return this->PreviousSection; }
	
	CLiteraryText *GetNextSection() const { return this->NextSection; }
	
	const std::vector<CLiteraryTextPage *> &GetPages() const { return this->Pages; }
	
	CLiteraryTextPage *GetFirstPage() const
	{
		if (!this->GetPages().empty()) {
			return this->GetPages().front();
		} else {
			return nullptr;
		}
	}
	
	CLiteraryTextPage *GetLastPage() const
	{
		if (!this->GetPages().empty()) {
			return this->GetPages().back();
		} else {
			return nullptr;
		}
	}
	
	/**
	**	@brief	Get the total page count of the literary text, including that of its sections, but ignoring the count of sections with disabled page numbering
	**
	**	@return The page count
	*/
	int GetTotalPageCount() const
	{
		int page_count = 0;
		
		if (this->PageNumberingEnabled) {
			page_count += static_cast<int>(this->GetPages().size());
		}
		
		for (const CLiteraryText *section : this->Sections) {
			page_count += section->GetTotalPageCount();
		}

		return page_count;
	}
	
	CLiteraryText *GetSection(const std::string &section_name) const;
	
private:	
	void UpdateSectionPageNumbers() const;

public:
	bool Hidden = false;	/// whether the literary text is hidden
	String Author;			/// author of the literary text
	String Translator;		/// translator of the literary text
	String Publisher;		/// publisher of the literary text
	String License;			/// the open-source license the literary text is under, or public domain if that's the case
	String Notes;			/// notes to appear on the cover of the literary text
	int PublicationYear = 0;	/// year of publication
	int InitialPageNumber = 0;	/// page number in which the literary text begins
	bool PageNumberingEnabled = true;	/// whether page numbering is enabled for the literary text
	bool LowercaseRomanNumeralPageNumbering = false;	/// whether page numbering should be depicted by lowercase Roman numerals
private:
	CIcon *Icon = nullptr;	/// the literary text's icon
	std::vector<CLiteraryText *> Sections;
	CLiteraryText *MainText = nullptr;	/// the main literary text to which this one belongs, if it is a section
	CLiteraryText *PreviousSection = nullptr;	/// the previous section to this one, if this is a section
	CLiteraryText *NextSection = nullptr;	/// the next section to this one, if this is a section
	std::vector<CLiteraryTextPage *> Pages;	/// pages of the literary text
	
protected:
	static void _bind_methods();
};

#endif
