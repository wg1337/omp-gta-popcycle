#if defined _omp_gta_popcycle_included
    #endinput
#endif
#define _omp_gta_popcycle_included

#define GTA_POPCYCLE_TYPE_COUNT (20)
#define GTA_POPCYCLE_DAY_TYPE_COUNT (2)
#define GTA_POPCYCLE_TIME_COUNT (12)
#define GTA_POPCYCLE_VALUE_COUNT (24)
#define GTA_POPCYCLE_ROW_COUNT (GTA_POPCYCLE_TYPE_COUNT * GTA_POPCYCLE_DAY_TYPE_COUNT * GTA_POPCYCLE_TIME_COUNT)

#define GTA_POPCYCLE_FILE "popcycle.dat"
#define GTA_POPCYCLE_MAX_LINE_LENGTH (512)
#define GTA_POPCYCLE_MAX_COMMENT_LENGTH (128)
#define GTA_POPCYCLE_MAX_TOKEN_LENGTH (64)

enum E_GTA_POPCYCLE_VALUE {
    GTA_POPCYCLE_VALUE_UNKNOWN = -1,
    GTA_POPCYCLE_VALUE_PEDS = 0,
    GTA_POPCYCLE_VALUE_CARS,
    GTA_POPCYCLE_VALUE_DEALERS,
    GTA_POPCYCLE_VALUE_GANG,
    GTA_POPCYCLE_VALUE_COPS,
    GTA_POPCYCLE_VALUE_OTHER,
    GTA_POPCYCLE_VALUE_WORKERS,
    GTA_POPCYCLE_VALUE_BUSINESS,
    GTA_POPCYCLE_VALUE_CLUBBERS,
    GTA_POPCYCLE_VALUE_FARMERS,
    GTA_POPCYCLE_VALUE_BEACHFOLK,
    GTA_POPCYCLE_VALUE_PARKFOLK,
    GTA_POPCYCLE_VALUE_CASUAL_RICH,
    GTA_POPCYCLE_VALUE_CASUAL_AVERAGE,
    GTA_POPCYCLE_VALUE_CASUAL_POOR,
    GTA_POPCYCLE_VALUE_PROSTITUTES,
    GTA_POPCYCLE_VALUE_CRIMINALS,
    GTA_POPCYCLE_VALUE_GOLFERS,
    GTA_POPCYCLE_VALUE_SERVANTS,
    GTA_POPCYCLE_VALUE_AIRCREW,
    GTA_POPCYCLE_VALUE_ENTERTAINERS,
    GTA_POPCYCLE_VALUE_OUT_OF_TOWN_FACTORY,
    GTA_POPCYCLE_VALUE_DESERTFOLK,
    GTA_POPCYCLE_VALUE_AIRCREW_RUNWAY
};

enum E_GTA_POPCYCLE_DAY {
    GTA_POPCYCLE_DAY_MONDAY = 0,
    GTA_POPCYCLE_DAY_TUESDAY,
    GTA_POPCYCLE_DAY_WEDNESDAY,
    GTA_POPCYCLE_DAY_THURSDAY,
    GTA_POPCYCLE_DAY_FRIDAY,
    GTA_POPCYCLE_DAY_SATURDAY,
    GTA_POPCYCLE_DAY_SUNDAY
};

enum E_GTA_POPCYCLE_TYPE {
    GTA_POPCYCLE_TYPE_UNKNOWN = -1,
    GTA_POPCYCLE_TYPE_BUSINESS = 0,
    GTA_POPCYCLE_TYPE_DESERT,
    GTA_POPCYCLE_TYPE_ENTERTAINMENT,
    GTA_POPCYCLE_TYPE_COUNTRYSIDE,
    GTA_POPCYCLE_TYPE_RESIDENTIAL_RICH,
    GTA_POPCYCLE_TYPE_RESIDENTIAL_AVERAGE,
    GTA_POPCYCLE_TYPE_RESIDENTIAL_POOR,
    GTA_POPCYCLE_TYPE_GANGLAND,
    GTA_POPCYCLE_TYPE_BEACH,
    GTA_POPCYCLE_TYPE_SHOPPING,
    GTA_POPCYCLE_TYPE_PARK,
    GTA_POPCYCLE_TYPE_INDUSTRY,
    GTA_POPCYCLE_TYPE_ENTERTAINMENT_BUSY,
    GTA_POPCYCLE_TYPE_SHOPPING_BUSY,
    GTA_POPCYCLE_TYPE_SHOPPING_POSH,
    GTA_POPCYCLE_TYPE_RESIDENTIAL_RICH_SECLUDED,
    GTA_POPCYCLE_TYPE_AIRPORT,
    GTA_POPCYCLE_TYPE_GOLF_CLUB,
    GTA_POPCYCLE_TYPE_OUT_OF_TOWN_FACTORY,
    GTA_POPCYCLE_TYPE_AIRPORT_RUNWAY
};

enum E_GTA_POPCYCLE_DAY_TYPE {
    GTA_POPCYCLE_DAY_UNKNOWN = -1,
    GTA_POPCYCLE_WEEKDAY = 0,
    GTA_POPCYCLE_WEEKEND
};

enum E_GTA_POPCYCLE_TIME {
    GTA_POPCYCLE_TIME_UNKNOWN = -1,
    GTA_POPCYCLE_00_00 = 0,
    GTA_POPCYCLE_02_00,
    GTA_POPCYCLE_04_00,
    GTA_POPCYCLE_06_00,
    GTA_POPCYCLE_08_00,
    GTA_POPCYCLE_10_00,
    GTA_POPCYCLE_12_00,
    GTA_POPCYCLE_14_00,
    GTA_POPCYCLE_16_00,
    GTA_POPCYCLE_18_00,
    GTA_POPCYCLE_20_00,
    GTA_POPCYCLE_22_00
};


static gPopCycle[GTA_POPCYCLE_TYPE_COUNT][GTA_POPCYCLE_DAY_TYPE_COUNT][GTA_POPCYCLE_TIME_COUNT][E_GTA_POPCYCLE_VALUE];
static bool:gPopCycleLoaded;

/*
* To match GTA:SA style popcycles we need some kind of way to track days, for this to work properly we need to "tap in" into existing clock system
*/
static E_GTA_POPCYCLE_DAY:gPopCycleDayOfWeek = GTA_POPCYCLE_DAY_MONDAY;
static gPopCycleLastHour = -1;

static stock bool:PopCycle_IsLineEmpty(const line[]) {
    new length = strlen(line);

    for(new i = 0; i < length; i++) {
        if(line[i] != ' ' && line[i] != '\t' && line[i] != '\r' && line[i] != '\n')
            return false;
    }
    return true;
}

static stock bool:PopCycle_GetCommentText(const line[], output[], outputSize) {
    new start = 0;

    while(line[start] == ' ' || line[start] == '\t')
        start++;

    if(line[start] != '/' || line[start + 1] != '/')
        return false;

    start += 2;

    while(line[start] == ' ' || line[start] == '\t')
        start++;

    format(output, outputSize, "%s", line[start]);

    new length = strlen(output);

    while(length > 0 && (output[length - 1] == ' ' || output[length - 1] == '\t' || output[length - 1] == '\r' || output[length - 1] == '\n'))
        output[--length] = '\0';

    return true;
}

static stock E_GTA_POPCYCLE_TYPE:PopCycle_GetTypeFromLine(const line[]) {
    new section[GTA_POPCYCLE_MAX_COMMENT_LENGTH];

    if(!PopCycle_GetCommentText(line, section, sizeof(section)))
        return GTA_POPCYCLE_TYPE_UNKNOWN;

    new token[GTA_POPCYCLE_MAX_TOKEN_LENGTH];
    new source = 0;
    new destination = 0;
    new length = strlen(section);

    while(source < length && (section[source] == ' ' || section[source] == '\t'))
        source++;

    while(source < length && section[source] != ' ' && section[source] != '\t') {
        if(destination >= sizeof(token) - 1)
            break;

        token[destination++] = section[source++];
    }

    token[destination] = '\0';

    if(!strcmp(token, "BUSINESS", true))
        return GTA_POPCYCLE_TYPE_BUSINESS;

    if(!strcmp(token, "DESERT", true))
        return GTA_POPCYCLE_TYPE_DESERT;

    if(!strcmp(token, "ENTERTAINMENT", true))
        return GTA_POPCYCLE_TYPE_ENTERTAINMENT;

    if(!strcmp(token, "COUNTRYSIDE", true))
        return GTA_POPCYCLE_TYPE_COUNTRYSIDE;

    if(!strcmp(token, "RESIDENTIAL_RICH", true))
        return GTA_POPCYCLE_TYPE_RESIDENTIAL_RICH;

    if(!strcmp(token, "RESIDENTIAL_AVERAGE", true))
        return GTA_POPCYCLE_TYPE_RESIDENTIAL_AVERAGE;

    if(!strcmp(token, "RESIDENTIAL_POOR", true))
        return GTA_POPCYCLE_TYPE_RESIDENTIAL_POOR;

    if(!strcmp(token, "GANGLAND", true))
        return GTA_POPCYCLE_TYPE_GANGLAND;

    if(!strcmp(token, "BEACH", true))
        return GTA_POPCYCLE_TYPE_BEACH;

    if(!strcmp(token, "SHOPPING", true))
        return GTA_POPCYCLE_TYPE_SHOPPING;

    if(!strcmp(token, "PARK", true))
        return GTA_POPCYCLE_TYPE_PARK;

    if(!strcmp(token, "INDUSTRY", true))
        return GTA_POPCYCLE_TYPE_INDUSTRY;

    if(!strcmp(token, "ENTERTAINMENT_BUSY", true))
        return GTA_POPCYCLE_TYPE_ENTERTAINMENT_BUSY;

    if(!strcmp(token, "SHOPPING_BUSY", true))
        return GTA_POPCYCLE_TYPE_SHOPPING_BUSY;

    if(!strcmp(token, "SHOPPING_POSH", true))
        return GTA_POPCYCLE_TYPE_SHOPPING_POSH;

    if(!strcmp(token, "RESIDENTIAL_RICH_SECLUDED", true))
        return GTA_POPCYCLE_TYPE_RESIDENTIAL_RICH_SECLUDED;

    if(!strcmp(token, "AIRPORT", true))
        return GTA_POPCYCLE_TYPE_AIRPORT;

    if(!strcmp(token, "GOLF_CLUB", true))
        return GTA_POPCYCLE_TYPE_GOLF_CLUB;

    if(!strcmp(token, "OUT_OF_TOWN_FACTORY", true))
        return GTA_POPCYCLE_TYPE_OUT_OF_TOWN_FACTORY;

    if(!strcmp(token, "AIRPORT_RUNWAY", true))
        return GTA_POPCYCLE_TYPE_AIRPORT_RUNWAY;

    return GTA_POPCYCLE_TYPE_UNKNOWN;
}

static stock PopCycle_Clear() {
    for(new popType = 0; popType < GTA_POPCYCLE_TYPE_COUNT; popType++) {
        for(new dayType = 0; dayType < GTA_POPCYCLE_DAY_TYPE_COUNT; dayType++) {
            for(new timeSlot = 0; timeSlot < GTA_POPCYCLE_TIME_COUNT; timeSlot++) {
                for(new value = 0; value < GTA_POPCYCLE_VALUE_COUNT; value++) {
                    gPopCycle[popType][dayType][timeSlot][E_GTA_POPCYCLE_VALUE:value] = 0;
                }
            }
        }
    }
}

static stock bool:PopCycle_ParseRow(const line[], E_GTA_POPCYCLE_TYPE:popType, E_GTA_POPCYCLE_DAY_TYPE:dayType, E_GTA_POPCYCLE_TIME:timeSlot) {
    new values[GTA_POPCYCLE_VALUE_COUNT];
    new count = 0;
    new length = strlen(line);
    new position = 0;

    while(position < length) {
        while(position < length && (line[position] == ' ' || line[position] == '\t'))
            position++;

        if(position >= length)
            break;

        if(line[position] == '/' && position + 1 < length && line[position + 1] == '/')
            break;

        new value = 0;
        new sign = 1;
        new bool:foundDigit = false;

        if(line[position] == '-') {
            sign = -1;
            position++;
        }

        while(position < length && line[position] >= '0' && line[position] <= '9') {
            value = value * 10 + (line[position] - '0');
            position++;
            foundDigit = true;
        }

        if(!foundDigit)
            return false;

        if(count >= GTA_POPCYCLE_VALUE_COUNT)
            return false;

        values[count++] = value * sign;
    }

    if(count != GTA_POPCYCLE_VALUE_COUNT)
        return false;

    for(new i = 0; i < GTA_POPCYCLE_VALUE_COUNT; i++)
        gPopCycle[_:popType][_:dayType][_:timeSlot][E_GTA_POPCYCLE_VALUE:i] = values[i];

    return true;
}

static stock bool:PopCycle_ValidatePreviousDay(E_GTA_POPCYCLE_TYPE:currentType, E_GTA_POPCYCLE_DAY_TYPE:currentDay, currentSlot) {
    if(currentType != GTA_POPCYCLE_TYPE_UNKNOWN && currentDay != GTA_POPCYCLE_DAY_UNKNOWN && currentSlot != GTA_POPCYCLE_TIME_COUNT) {
        printf("[omp-gta-popcycle] Invalid row count: type=%d day=%d rows=%d expected=%d", _:currentType, _:currentDay, currentSlot, GTA_POPCYCLE_TIME_COUNT);
        return false;
    }

    return true;
}

/*
* --------------------------------------------------------------------------
* USABLE FUNCTIONS
* --------------------------------------------------------------------------
*/

stock bool:LoadPopCycleDat() {
    gPopCycleLoaded = false;
    PopCycle_Clear();

    new File:file = fopen(GTA_POPCYCLE_FILE, io_read);

    if(!file) {
        printf("[omp-gta-popcycle] Failed to open %s", GTA_POPCYCLE_FILE);
        return false;
    }

    new line[GTA_POPCYCLE_MAX_LINE_LENGTH];

    new E_GTA_POPCYCLE_TYPE:currentType = GTA_POPCYCLE_TYPE_UNKNOWN;
    new E_GTA_POPCYCLE_DAY_TYPE:currentDay = GTA_POPCYCLE_DAY_UNKNOWN;
    new currentSlot = 0;
    new loadedRows = 0;

    new bool:loadedDays[GTA_POPCYCLE_TYPE_COUNT][GTA_POPCYCLE_DAY_TYPE_COUNT];
    new bool:loadedTypes[GTA_POPCYCLE_TYPE_COUNT];

    while(fread(file, line)) {
        new length = strlen(line);

        while(length > 0 && (line[length - 1] == '\r' || line[length - 1] == '\n'))
            line[--length] = '\0';

        if(PopCycle_IsLineEmpty(line))
            continue;

        new E_GTA_POPCYCLE_TYPE:detectedType = PopCycle_GetTypeFromLine(line);

        if(detectedType != GTA_POPCYCLE_TYPE_UNKNOWN) {
            if(!PopCycle_ValidatePreviousDay(currentType, currentDay, currentSlot)) {
                fclose(file);
                return false;
            }

            if(loadedTypes[_:detectedType]) {
                printf(
                    "[omp-gta-popcycle] Duplicate population type: type=%d",
                    _:detectedType
                );
                fclose(file);
                return false;
            }

            loadedTypes[_:detectedType] = true;

            currentType = detectedType;
            currentDay = GTA_POPCYCLE_DAY_UNKNOWN;
            currentSlot = 0;
            continue;
        }

        new comment[GTA_POPCYCLE_MAX_COMMENT_LENGTH];

        if(PopCycle_GetCommentText(line, comment, sizeof(comment))) {
            if(!strcmp(comment, "Weekday", true) || !strcmp(comment, "Weekend", true)) {
                if(!PopCycle_ValidatePreviousDay(currentType, currentDay, currentSlot)) {
                    fclose(file);
                    return false;
                }

                if(currentType == GTA_POPCYCLE_TYPE_UNKNOWN) {
                    printf(
                        "[omp-gta-popcycle] Day section found before population type: %s",
                        comment
                    );
                    fclose(file);
                    return false;
                }

                currentDay = !strcmp(comment, "Weekday", true)
                    ? GTA_POPCYCLE_WEEKDAY
                    : GTA_POPCYCLE_WEEKEND;

                if(loadedDays[_:currentType][_:currentDay]) {
                    printf(
                        "[omp-gta-popcycle] Duplicate day section: type=%d day=%d",
                        _:currentType,
                        _:currentDay
                    );
                    fclose(file);
                    return false;
                }

                loadedDays[_:currentType][_:currentDay] = true;
                currentSlot = 0;
            }

            continue;
        }

        if(currentType == GTA_POPCYCLE_TYPE_UNKNOWN || currentDay == GTA_POPCYCLE_DAY_UNKNOWN) {
            printf("[omp-gta-popcycle] Unexpected data outside a day section: %s", line);
            fclose(file);
            return false;
        }

        if(currentSlot >= GTA_POPCYCLE_TIME_COUNT) {
            printf("[omp-gta-popcycle] Too many rows: type=%d day=%d rows=%d", _:currentType, _:currentDay, currentSlot);
            fclose(file);
            return false;
        }

        if(!PopCycle_ParseRow(line, currentType, currentDay, E_GTA_POPCYCLE_TIME:currentSlot)) {
            printf("[omp-gta-popcycle] Invalid row: type=%d day=%d slot=%d", _:currentType, _:currentDay, currentSlot);
            fclose(file);
            return false;
        }

        currentSlot++;
        loadedRows++;
    }

    if(!PopCycle_ValidatePreviousDay(currentType, currentDay, currentSlot)) {
        fclose(file);
        return false;
    }

    if(loadedRows != GTA_POPCYCLE_ROW_COUNT) {
        printf("[omp-gta-popcycle] Invalid total row count: rows=%d expected=%d", loadedRows, GTA_POPCYCLE_ROW_COUNT);
        fclose(file);
        return false;
    }

    for(new popType = 0; popType < GTA_POPCYCLE_TYPE_COUNT; popType++) {
        for(new dayType = 0; dayType < GTA_POPCYCLE_DAY_TYPE_COUNT; dayType++) {
            if(!loadedDays[popType][dayType]) {
                printf("[omp-gta-popcycle] Missing day section: type=%d day=%d", popType, dayType);
                fclose(file);
                return false;
            }
        }
    }

    fclose(file);

    gPopCycleLoaded = true;

    printf("[omp-gta-popcycle] Loaded %d rows.", loadedRows);

    return true;
}

stock bool:IsPopCycleLoaded() {
    return gPopCycleLoaded;
}

/*
* Gets the population weight for a day type and time. Useful to figure out how many of certain peds, vehicles or groups types need to be spawned
*/
stock GetPopCycleValue(E_GTA_POPCYCLE_TYPE:popType, E_GTA_POPCYCLE_DAY_TYPE:dayType, E_GTA_POPCYCLE_TIME:timeSlot, E_GTA_POPCYCLE_VALUE:popValue) {
    if(!gPopCycleLoaded)
        return -1;

    if(_:popType < 0 || _:popType >= GTA_POPCYCLE_TYPE_COUNT)
        return -1;

    if(_:dayType < 0 || _:dayType >= GTA_POPCYCLE_DAY_TYPE_COUNT)
        return -1;

    if(_:timeSlot < 0 || _:timeSlot >= GTA_POPCYCLE_TIME_COUNT)
        return -1;

    if(_:popValue < 0 || _:popValue >= GTA_POPCYCLE_VALUE_COUNT)
        return -1;

    return gPopCycle[_:popType][_:dayType][_:timeSlot][popValue];
}

stock GetPopCyclePeds(E_GTA_POPCYCLE_TYPE:popType, E_GTA_POPCYCLE_DAY_TYPE:dayType, E_GTA_POPCYCLE_TIME:timeSlot) {
    return GetPopCycleValue(popType, dayType, timeSlot, GTA_POPCYCLE_VALUE_PEDS);
}

stock GetPopCycleCars(E_GTA_POPCYCLE_TYPE:popType, E_GTA_POPCYCLE_DAY_TYPE:dayType, E_GTA_POPCYCLE_TIME:timeSlot) {
    return GetPopCycleValue(popType, dayType, timeSlot, GTA_POPCYCLE_VALUE_CARS);
}

stock GetPopCycleGroupWeight(E_GTA_POPCYCLE_TYPE:popType, E_GTA_POPCYCLE_DAY_TYPE:dayType, E_GTA_POPCYCLE_TIME:timeSlot, E_GTA_POPCYCLE_VALUE:group) {
    if(group < GTA_POPCYCLE_VALUE_WORKERS || group > GTA_POPCYCLE_VALUE_AIRCREW_RUNWAY)
        return -1;

    return GetPopCycleValue(popType, dayType, timeSlot, group);
}

/*
* Gets the popcycle's timeslot. GTA:SA simply uses 2 hour slots
*/
stock E_GTA_POPCYCLE_TIME:GetPopCycleTimeSlot(hour) {
    if(hour < 0)
        return GTA_POPCYCLE_TIME_UNKNOWN;
    if(hour > 23)
        return GTA_POPCYCLE_TIME_UNKNOWN;

    return E_GTA_POPCYCLE_TIME:(hour / 2);
}

/*
* Recreation of CPopCycle::PickARandomGroupOfOtherPeds() from gta-reversed.
* Selects one random population group based on day type and time
* Uses the gta-reversed FIX_BUGS that does not need weights to not sum up to 100%
* In GTA:SA there is another function that recalculates the weights, here we dropped that functionality
*/
stock E_GTA_POPCYCLE_VALUE:GetRandomPopCyclePopulationGroup(E_GTA_POPCYCLE_TYPE:popType, E_GTA_POPCYCLE_DAY_TYPE:dayType, E_GTA_POPCYCLE_TIME:timeSlot) {
    if(!gPopCycleLoaded)
        return GTA_POPCYCLE_VALUE_UNKNOWN;

    if(_:popType < 0 || _:popType >= GTA_POPCYCLE_TYPE_COUNT)
        return GTA_POPCYCLE_VALUE_UNKNOWN;

    if(_:dayType < 0 || _:dayType >= GTA_POPCYCLE_DAY_TYPE_COUNT)
        return GTA_POPCYCLE_VALUE_UNKNOWN;

    if(_:timeSlot < 0 || _:timeSlot >= GTA_POPCYCLE_TIME_COUNT)
        return GTA_POPCYCLE_VALUE_UNKNOWN;

    new total = 0;

    for(new E_GTA_POPCYCLE_VALUE:group = GTA_POPCYCLE_VALUE_WORKERS; group <= GTA_POPCYCLE_VALUE_AIRCREW_RUNWAY; group++) {
        new weight = gPopCycle[_:popType][_:dayType][_:timeSlot][group];

        if(weight > 0)
            total += weight;
    }

    if(total <= 0)
        return GTA_POPCYCLE_VALUE_UNKNOWN;

    new value = random(total);

    for(new E_GTA_POPCYCLE_VALUE:group = GTA_POPCYCLE_VALUE_WORKERS; group <= GTA_POPCYCLE_VALUE_AIRCREW_RUNWAY; group++) {
        new weight = gPopCycle[_:popType][_:dayType][_:timeSlot][group];

        if(weight <= 0)
            continue;

        if(value < weight)
            return group;

        value -= weight;
    }

    return GTA_POPCYCLE_VALUE_UNKNOWN;
}

/*
* Recreation of GTA CPopCycle::Update() from gta-reversed, sort of.
* Gets the day type based on gta-reversed logic by using the internal day name and the hour
*/
stock E_GTA_POPCYCLE_DAY_TYPE:GetPopCycleDayType(hour) {
    if(hour < 0)
        return GTA_POPCYCLE_DAY_UNKNOWN;
    if(hour > 23)
        return GTA_POPCYCLE_DAY_UNKNOWN;

    /*
    *
    * Monday-Thursday:
    *     weekday
    *
    * Friday:
    *     before 20:00 -> weekday
    *     from   20:00 -> weekend
    *
    * Saturday:
    *     weekend
    *
    * Sunday:
    *     before 20:00 -> weekend
    *     from   20:00 -> weekday
    */
    switch(_:gPopCycleDayOfWeek) {
        case (_:GTA_POPCYCLE_DAY_FRIDAY): {
            if (hour >= 20) {
                return GTA_POPCYCLE_WEEKEND;
            }
        }

        case (_:GTA_POPCYCLE_DAY_SATURDAY): {
            return GTA_POPCYCLE_WEEKEND;
        }

        case (_:GTA_POPCYCLE_DAY_SUNDAY): {
            if (hour < 20) {
                return GTA_POPCYCLE_WEEKEND;
            }
        }
    }

    return GTA_POPCYCLE_WEEKDAY;
}


/*
* Updates the current day when the game clock wraps at midnight
*/
forward bool:GTAPopCycle_UpdateHour(hour);
public bool:GTAPopCycle_UpdateHour(hour) {
    if(hour < 0 || hour > 23) {
        return false;
    }

    if(gPopCycleLastHour == -1) {
        gPopCycleLastHour = hour;
        return true;
    }

    if(hour < gPopCycleLastHour) {
        if(gPopCycleDayOfWeek == GTA_POPCYCLE_DAY_SUNDAY) {
            gPopCycleDayOfWeek = GTA_POPCYCLE_DAY_MONDAY;
        } else {
            gPopCycleDayOfWeek = E_GTA_POPCYCLE_DAY:(_:gPopCycleDayOfWeek + 1);
        }
    }

    gPopCycleLastHour = hour;
    return true;
}

/*
* Sets a specific day of the week. Affects what kind of population group is selected
*/
stock bool:SetPopCycleDayOfWeek(E_GTA_POPCYCLE_DAY:day) {
    if(day < GTA_POPCYCLE_DAY_MONDAY || day > GTA_POPCYCLE_DAY_SUNDAY) {
        return false;
    }

    gPopCycleDayOfWeek = day;
    gPopCycleLastHour = -1;

    return true;
}

/*
* Gets the current day of the week
*/
stock E_GTA_POPCYCLE_DAY:GetPopCycleDayOfWeek() {
    return gPopCycleDayOfWeek;
}

/*
* Returns the ped population group index for a popcycle value.
*/
stock GetPopCyclePedCategoryIndex(E_GTA_POPCYCLE_VALUE:category) {
    if(category < GTA_POPCYCLE_VALUE_WORKERS || category > GTA_POPCYCLE_VALUE_AIRCREW_RUNWAY)  {
        return -1;
    }

    return _:category - _:GTA_POPCYCLE_VALUE_WORKERS;
}

/*
* Tests loaded popcycle.dat
* TestPopCycle(); - Tests all 480 rows.
* TestPopCycle(GTA_POPCYCLE_TYPE_BUSINESS); - Tests all BUSINESS rows.
* TestPopCycle(GTA_POPCYCLE_TYPE_BUSINESS, GTA_POPCYCLE_WEEKDAY); - Tests all BUSINESS weekday rows.
* TestPopCycle(GTA_POPCYCLE_TYPE_BUSINESS, GTA_POPCYCLE_WEEKDAY, GTA_POPCYCLE_08_00); - Tests one specific row.
*/
stock bool:TestPopCycle(E_GTA_POPCYCLE_TYPE:filterType = GTA_POPCYCLE_TYPE_UNKNOWN, E_GTA_POPCYCLE_DAY_TYPE:filterDay = GTA_POPCYCLE_DAY_UNKNOWN, E_GTA_POPCYCLE_TIME:filterTime = GTA_POPCYCLE_TIME_UNKNOWN) {
    static const PopCycleValueNames[][32] = {
        "Peds",
        "Cars",
        "Dealers",
        "Gang",
        "Cops",
        "Other",
        "Workers",
        "Business",
        "Clubbers",
        "Farmers",
        "Beachfolk",
        "Parkfolk",
        "CasualRich",
        "CasualAverage",
        "CasualPoor",
        "Prostitutes",
        "Criminals",
        "Golfers",
        "Servants",
        "Aircrew",
        "Entertainers",
        "OutOfTownFactory",
        "Desertfolk",
        "AircrewRunway"
    };

    if(filterType != GTA_POPCYCLE_TYPE_UNKNOWN && (_:filterType < 0 || _:filterType >= GTA_POPCYCLE_TYPE_COUNT)) {
        printf("[omp-gta-popcycle] ERROR: Invalid population type %d.", _:filterType);
        return false;
    }

    if(filterDay != GTA_POPCYCLE_DAY_UNKNOWN && (_:filterDay < 0 || _:filterDay >= GTA_POPCYCLE_DAY_TYPE_COUNT)) {
        printf("[omp-gta-popcycle] ERROR: Invalid day type %d.", _:filterDay);
        return false;
    }

    if(filterTime != GTA_POPCYCLE_TIME_UNKNOWN && (_:filterTime < 0 || _:filterTime >= GTA_POPCYCLE_TIME_COUNT)) {
        printf("[omp-gta-popcycle] ERROR: Invalid time slot %d.", _:filterTime);
        return false;
    }

    new expectedRows = 1;

    if(filterType == GTA_POPCYCLE_TYPE_UNKNOWN)
        expectedRows *= GTA_POPCYCLE_TYPE_COUNT;

    if(filterDay == GTA_POPCYCLE_DAY_UNKNOWN)
        expectedRows *= GTA_POPCYCLE_DAY_TYPE_COUNT;

    if(filterTime == GTA_POPCYCLE_TIME_UNKNOWN)
        expectedRows *= GTA_POPCYCLE_TIME_COUNT;

    printf("[omp-gta-popcycle] ============================================================");
    printf("[omp-gta-popcycle] POPCYCLE DATA TEST");
    printf("[omp-gta-popcycle] Filter: type=%d day=%d time=%d", _:filterType, _:filterDay, _:filterTime);
    printf("[omp-gta-popcycle] Expected rows: %d", expectedRows);
    printf("[omp-gta-popcycle] ============================================================");

    new totalRows = 0;
    new invalidRows = 0;
    new invalidValues = 0;

    new below100Rows = 0;
    new exact100Rows = 0;
    new above100Rows = 0;

    new minimumGroupTotal = 0x7FFFFFFF;
    new maximumGroupTotal = 0;

    new minimumValue[GTA_POPCYCLE_VALUE_COUNT];
    new maximumValue[GTA_POPCYCLE_VALUE_COUNT];

    for(new value = _:GTA_POPCYCLE_VALUE_PEDS; value <= _:GTA_POPCYCLE_VALUE_AIRCREW_RUNWAY; value++) {
        minimumValue[value] = 0x7FFFFFFF;
        maximumValue[value] = 0;
    }

    for(new popType = 0; popType < GTA_POPCYCLE_TYPE_COUNT; popType++) {
        if(filterType != GTA_POPCYCLE_TYPE_UNKNOWN && popType != _:filterType) {
            continue;
        }

        for(new dayType = 0; dayType < GTA_POPCYCLE_DAY_TYPE_COUNT; dayType++) {
            if(filterDay != GTA_POPCYCLE_DAY_UNKNOWN && dayType != _:filterDay) {
                continue;
            }

            for(new timeSlot = 0; timeSlot < GTA_POPCYCLE_TIME_COUNT; timeSlot++) {
                if(filterTime != GTA_POPCYCLE_TIME_UNKNOWN && timeSlot != _:filterTime) {
                    continue;
                }

                new E_GTA_POPCYCLE_TYPE:type = E_GTA_POPCYCLE_TYPE:popType;
                new E_GTA_POPCYCLE_DAY_TYPE:day = E_GTA_POPCYCLE_DAY_TYPE:dayType;
                new E_GTA_POPCYCLE_TIME:time = E_GTA_POPCYCLE_TIME:timeSlot;
                new values[GTA_POPCYCLE_VALUE_COUNT];
                new groupTotal = 0;
                new bool:rowValid = true;

                for(new value = _:GTA_POPCYCLE_VALUE_PEDS; value <= _:GTA_POPCYCLE_VALUE_AIRCREW_RUNWAY; value++) {
                    new E_GTA_POPCYCLE_VALUE:popValue = E_GTA_POPCYCLE_VALUE:value;

                    values[value] = GetPopCycleValue(type, day, time, popValue);

                    if(values[value] < 0) {
                        printf("[omp-gta-popcycle] ERROR: type=%d day=%d time=%d value=%d (%s) returned %d", popType, dayType, timeSlot, value, PopCycleValueNames[value], values[value]);

                        invalidValues++;
                        rowValid = false;
                        continue;
                    }

                    if(values[value] < minimumValue[value])
                        minimumValue[value] = values[value];

                    if(values[value] > maximumValue[value])
                        maximumValue[value] = values[value];

                    if(value >= _:GTA_POPCYCLE_VALUE_WORKERS && value <= _:GTA_POPCYCLE_VALUE_AIRCREW_RUNWAY) {
                        groupTotal += values[value];
                    }
                }

                printf("[omp-gta-popcycle] ROW type=%02d day=%d time=%02d", popType, dayType, timeSlot);

                for(new value = _:GTA_POPCYCLE_VALUE_PEDS; value <= _:GTA_POPCYCLE_VALUE_AIRCREW_RUNWAY; value++) {
                    printf("[omp-gta-popcycle] [%02d] %-16s = %d", value, PopCycleValueNames[value], values[value]);
                }

                printf("[omp-gta-popcycle] Population group total = %d", groupTotal);

                if(GetPopCyclePeds(type, day, time) != values[_:GTA_POPCYCLE_VALUE_PEDS]) {
                    printf("[omp-gta-popcycle] ERROR: GetPopCyclePeds mismatch type=%d day=%d time=%d", popType, dayType, timeSlot);
                    rowValid = false;
                }

                if(GetPopCycleCars(type, day, time) != values[_:GTA_POPCYCLE_VALUE_CARS]) {
                    printf("[omp-gta-popcycle] ERROR: GetPopCycleCars mismatch type=%d day=%d time=%d", popType, dayType, timeSlot);
                    rowValid = false;
                }

                for(new group = _:GTA_POPCYCLE_VALUE_WORKERS; group <= _:GTA_POPCYCLE_VALUE_AIRCREW_RUNWAY; group++) {
                    new E_GTA_POPCYCLE_VALUE:popGroup = E_GTA_POPCYCLE_VALUE:group;

                    if(GetPopCycleGroupWeight(type, day, time, popGroup) != values[group]) {
                        printf("[omp-gta-popcycle] ERROR: Group getter mismatch type=%d day=%d time=%d group=%d (%s)", popType, dayType, timeSlot, group, PopCycleValueNames[group]);
                        rowValid = false;
                    }
                }

                /*
                * Using gta-reversed FIX_BUGS, weights do not need to add up to 100%
                */
                if(groupTotal <= 0) {
                    printf("[omp-gta-popcycle] ERROR: type=%d day=%d time=%d has group total %d", popType, dayType, timeSlot, groupTotal);
                    rowValid = false;
                } else if(groupTotal < 100) {
                    below100Rows++;
                } else if(groupTotal == 100) {
                    exact100Rows++;
                } else {
                    above100Rows++;
                }

                if(groupTotal < minimumGroupTotal)
                    minimumGroupTotal = groupTotal;

                if(groupTotal > maximumGroupTotal)
                    maximumGroupTotal = groupTotal;

                if(!rowValid)
                    invalidRows++;

                totalRows++;
            }
        }
    }

    printf("[omp-gta-popcycle] ============================================================");
    printf("[omp-gta-popcycle] COLUMN RANGES");
    printf("[omp-gta-popcycle] ============================================================");

    for(new value = _:GTA_POPCYCLE_VALUE_PEDS; value <= _:GTA_POPCYCLE_VALUE_AIRCREW_RUNWAY; value++) {
        printf("[omp-gta-popcycle] [%02d] %-16s min=%d max=%d", value, PopCycleValueNames[value], minimumValue[value], maximumValue[value]);
    }

    printf("[omp-gta-popcycle] ============================================================");
    printf("[omp-gta-popcycle] VALIDATION RESULTS");
    printf("[omp-gta-popcycle] ============================================================");
    printf("[omp-gta-popcycle] Rows checked:             %d / %d", totalRows, expectedRows);
    printf("[omp-gta-popcycle] Group totals below 100:   %d", below100Rows);
    printf("[omp-gta-popcycle] Group totals exactly 100: %d", exact100Rows);
    printf("[omp-gta-popcycle] Group totals above 100:   %d", above100Rows);
    printf("[omp-gta-popcycle] Minimum group total:      %d", minimumGroupTotal);
    printf("[omp-gta-popcycle] Maximum group total:      %d", maximumGroupTotal);
    printf("[omp-gta-popcycle] Invalid values:           %d", invalidValues);
    printf("[omp-gta-popcycle] Invalid rows:             %d", invalidRows);
    printf("[omp-gta-popcycle] ============================================================");

    new bool:passed = (totalRows == expectedRows && invalidValues == 0 && invalidRows == 0);

    if(passed) {
        printf("[omp-gta-popcycle] PASS: All %d selected rows passed validation.", totalRows);
    } else {
        printf("[omp-gta-popcycle] FAIL: Popcycle validation errors detected.");
    }

    printf("[omp-gta-popcycle] ============================================================");

    return passed;
}
