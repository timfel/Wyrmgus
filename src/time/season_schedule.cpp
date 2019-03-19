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
/**@name season_schedule.cpp - The season schedule source file. */
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

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "time/season_schedule.h"

#include "config.h"
#include "time/season.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

CSeasonSchedule *CSeasonSchedule::DefaultSeasonSchedule = nullptr;
	
/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**	@brief	Destructor
*/
CSeasonSchedule::~CSeasonSchedule()
{
	for (CScheduledSeason *scheduled_season : this->ScheduledSeasons) {
		delete scheduled_season;
	}
}

/**
**	@brief	Process data provided by a configuration file
**
**	@param	config_data	The configuration data
*/
void CSeasonSchedule::ProcessConfigData(const CConfigData *config_data)
{
	for (size_t i = 0; i < config_data->Properties.size(); ++i) {
		std::string key = config_data->Properties[i].first;
		std::string value = config_data->Properties[i].second;
		
		if (key == "name") {
			this->Name = value;
		} else if (key == "default_schedule") {
			const bool is_default_schedule = StringToBool(value);
			if (is_default_schedule) {
				CSeasonSchedule::DefaultSeasonSchedule = this;
			}
		} else if (key == "hours_per_day") {
			this->HoursPerDay = std::stoi(value);
		} else {
			fprintf(stderr, "Invalid season schedule property: \"%s\".\n", key.c_str());
		}
	}
	
	for (const CConfigData *section : config_data->Sections) {
		if (section->Tag == "scheduled_season") {
			CSeason *season = nullptr;
			unsigned hours = 0;
				
			for (size_t j = 0; j < section->Properties.size(); ++j) {
				std::string key = section->Properties[j].first;
				std::string value = section->Properties[j].second;
				
				if (key == "season") {
					value = FindAndReplaceString(value, "_", "-");
					season = CSeason::Get(value);
				} else if (key == "days") {
					hours = std::stoi(value) * this->HoursPerDay;
				} else if (key == "hours") {
					hours = std::stoi(value);
				} else {
					fprintf(stderr, "Invalid scheduled season property: \"%s\".\n", key.c_str());
				}
			}
			
			if (!season) {
				fprintf(stderr, "Scheduled season has no season.\n");
				continue;
			}
			
			if (hours <= 0) {
				fprintf(stderr, "Scheduled season has no amount of time defined.\n");
				continue;
			}
			
			CScheduledSeason *scheduled_season = new CScheduledSeason;
			scheduled_season->Season = season;
			scheduled_season->Hours = hours;
			scheduled_season->ID = this->ScheduledSeasons.size();
			scheduled_season->Schedule = this;
			this->ScheduledSeasons.push_back(scheduled_season);
			this->TotalHours += scheduled_season->Hours;
		} else {
			fprintf(stderr, "Invalid season schedule property: \"%s\".\n", section->Tag.c_str());
		}
	}
	
	this->CalculateHourMultiplier();
}

/**
**	@brief	Get the default total hours for a season schedule
**
**	@return	The default total hours
*/
unsigned long CSeasonSchedule::GetDefaultTotalHours() const
{
	return DEFAULT_DAYS_PER_YEAR * DEFAULT_HOURS_PER_DAY;
}

/**
**	@brief	Get the default hour multiplier for a season schedule
**
**	@return	The default hour multiplier
*/
int CSeasonSchedule::GetDefaultHourMultiplier() const
{
	return DEFAULT_DAY_MULTIPLIER_PER_YEAR;
}
