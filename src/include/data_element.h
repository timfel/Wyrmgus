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
/**@name data_element.h - The data element header file. */
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

#ifndef __DATA_ELEMENT_H__
#define __DATA_ELEMENT_H__

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <core/object.h>
#include <core/ustring.h>

#include <algorithm>
#include <map>
#include <string>

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CConfigData;
class PropertyCommonBase;

template<typename T>
class DataType;

/*----------------------------------------------------------------------------
--  Definition
----------------------------------------------------------------------------*/

class DataElement : public Object
{
	GDCLASS(DataElement, Object)
	
public:
	DataElement(const std::string &ident = "", const int index = -1) : Ident(ident), Index(index)
	{
	}
	
	virtual void ProcessConfigData(const CConfigData *config_data);
	virtual bool ProcessConfigDataProperty(const std::string &key, std::string value) { return false; }
	virtual bool ProcessConfigDataSection(const CConfigData *section) { return false; }
	
	/**
	**	@brief	Initialize the data element instance
	*/
	virtual void Initialize()
	{
		this->Initialized = true;
	}
	
	/**
	**	@brief	Get the data element instance's string identifier
	**
	**	@return	The data element instance's string identifier
	*/
	String GetIdent() const
	{
		return this->Ident.c_str();
	}
	
	/**
	**	@brief	Get the data element instance's index
	**
	**	@return	The data element instance's index
	*/
	int GetIndex() const
	{
		return this->Index;
	}
	
	/**
	**	@brief	Get whether the data element instance has been initialized
	**
	**	@return	True if the data element instance has been initialized, or false otherwise
	*/
	bool IsInitialized() const
	{
		return this->Initialized;
	}
	
	std::string Ident;	/// string identifier of the data element instance
protected:
	int Index = -1;		/// index of the data element instance
	bool Initialized = false;	/// whether the data element instance has been initialized
	std::map<std::string, PropertyCommonBase &> Properties;
	
	friend DataType;

protected:
	static void _bind_methods();
};

#endif