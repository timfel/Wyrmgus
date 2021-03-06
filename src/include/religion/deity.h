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
//      (c) Copyright 2018-2020 by Andrettin
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

#pragma once

#include "database/data_type.h"
#include "database/detailed_data_entry.h"

class CUpgrade;
struct lua_State;

static int CclDefineDeity(lua_State *l);

namespace wyrmgus {

class character;
class civilization;
class faction;
class icon;
class magic_domain;
class pantheon;
class plane;
class religion;
class spell;
enum class gender;

class deity final : public detailed_data_entry, public data_type<deity>
{
	Q_OBJECT

	Q_PROPERTY(wyrmgus::pantheon* pantheon MEMBER pantheon READ get_pantheon)
	Q_PROPERTY(wyrmgus::icon* icon READ get_icon WRITE set_icon)
	Q_PROPERTY(wyrmgus::gender gender READ get_gender WRITE set_gender)
	Q_PROPERTY(bool major MEMBER major READ is_major)
	Q_PROPERTY(wyrmgus::plane* home_plane MEMBER home_plane READ get_home_plane)
	Q_PROPERTY(wyrmgus::character* father READ get_father WRITE set_father)
	Q_PROPERTY(wyrmgus::character* mother READ get_mother WRITE set_mother)
	Q_PROPERTY(CUpgrade* upgrade READ get_upgrade WRITE set_upgrade)
	Q_PROPERTY(QVariantList civilizations READ get_civilizations_qvariant_list)
	Q_PROPERTY(QVariantList religions READ get_religions_qvariant_list)
	Q_PROPERTY(QVariantList domains READ get_domains_qvariant_list)

public:
	static constexpr const char *class_identifier = "deity";
	static constexpr const char *database_folder = "deities";
	static constexpr int major_deity_domain_max = 3; //major deities can only have up to three domains
	static constexpr int minor_deity_domain_max = 1; //minor deities can only have one domain

	static deity *add(const std::string &identifier, const wyrmgus::module *module);

	explicit deity(const std::string &identifier);
	
	virtual void process_sml_scope(const sml_data &scope) override;
	virtual void initialize() override;

	const std::string &get_cultural_name(const civilization *civilization) const;

	pantheon *get_pantheon() const
	{
		return this->pantheon;
	}

	character *get_character() const
	{
		return this->character;
	}

	icon *get_icon() const;
	void set_icon(icon *icon);

	gender get_gender() const;
	void set_gender(const gender gender);

	bool is_major() const
	{
		return this->major;
	}

	plane *get_home_plane() const
	{
		return this->home_plane;
	}

	character *get_father() const;
	void set_father(character *character);

	character *get_mother() const;
	void set_mother(character *character);

	CUpgrade *get_upgrade() const
	{
		return this->upgrade;
	}

	void set_upgrade(CUpgrade *upgrade);

	const std::vector<civilization *> &get_civilizations() const
	{
		return this->civilizations;
	}

	QVariantList get_civilizations_qvariant_list() const;

	Q_INVOKABLE void add_civilization(civilization *civilization);
	Q_INVOKABLE void remove_civilization(civilization *civilization);

	const std::vector<religion *> &get_religions() const
	{
		return this->religions;
	}

	QVariantList get_religions_qvariant_list() const;

	Q_INVOKABLE void add_religion(religion *religion)
	{
		this->religions.push_back(religion);
	}

	Q_INVOKABLE void remove_religion(religion *religion);

	const std::vector<magic_domain *> &get_domains() const
	{
		return this->domains;
	}

	QVariantList get_domains_qvariant_list() const;

	Q_INVOKABLE void add_domain(magic_domain *domain)
	{
		this->domains.push_back(domain);
	}

	Q_INVOKABLE void remove_domain(magic_domain *domain);

	const std::vector<const spell *> &get_spells() const
	{
		return this->spells;
	}

private:
	pantheon *pantheon = nullptr;
	character *character = nullptr;
	bool major = false;
	plane *home_plane = nullptr;
	CUpgrade *upgrade = nullptr; //the deity's upgrade applied to a player that worships it
	std::vector<civilization *> civilizations; //civilizations which may worship the deity
	std::vector<religion *> religions; //religions for which this deity is available
public:
	std::vector<std::string> Feasts;
private:
	std::vector<magic_domain *> domains;
	std::vector<const spell *> spells; //abilities linked to this deity
	std::map<const civilization *, std::string> cultural_names;	//names of the deity in different cultures (for example, Odin is known as Hroptatyr by the dwarves)

	friend static int ::CclDefineDeity(lua_State *l);
};

}