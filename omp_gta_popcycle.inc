#if defined _omp_gta_popcycle_included
    #endinput
#endif
#define _omp_gta_popcycle_included

#define GTA_POPCYCLE_TYPE_COUNT (20)
#define GTA_POPCYCLE_DAY_COUNT (2)
#define GTA_POPCYCLE_TIME_COUNT (12)
#define GTA_POPCYCLE_FIELD_COUNT (24)

#define GTA_POPCYCLE_WEEKDAY (0)
#define GTA_POPCYCLE_WEEKEND (1)

#define GTA_POPCYCLE_00_00 (0)
#define GTA_POPCYCLE_02_00 (1)
#define GTA_POPCYCLE_04_00 (2)
#define GTA_POPCYCLE_06_00 (3)
#define GTA_POPCYCLE_08_00 (4)
#define GTA_POPCYCLE_10_00 (5)
#define GTA_POPCYCLE_12_00 (6)
#define GTA_POPCYCLE_14_00 (7)
#define GTA_POPCYCLE_16_00 (8)
#define GTA_POPCYCLE_18_00 (9)
#define GTA_POPCYCLE_20_00 (10)
#define GTA_POPCYCLE_22_00 (11)

#define GTA_POPCYCLE_TYPE_BUSINESS (0)
#define GTA_POPCYCLE_TYPE_DESERT (1)
#define GTA_POPCYCLE_TYPE_ENTERTAINMENT (2)
#define GTA_POPCYCLE_TYPE_COUNTRYSIDE (3)
#define GTA_POPCYCLE_TYPE_RESIDENTIAL_RICH (4)
#define GTA_POPCYCLE_TYPE_RESIDENTIAL_AVERAGE (5)
#define GTA_POPCYCLE_TYPE_RESIDENTIAL_POOR (6)
#define GTA_POPCYCLE_TYPE_GANGLAND (7)
#define GTA_POPCYCLE_TYPE_BEACH (8)
#define GTA_POPCYCLE_TYPE_SHOPPING (9)
#define GTA_POPCYCLE_TYPE_PARK (10)
#define GTA_POPCYCLE_TYPE_INDUSTRY (11)
#define GTA_POPCYCLE_TYPE_ENTERTAINMENT_BUSY (12)
#define GTA_POPCYCLE_TYPE_SHOPPING_BUSY (13)
#define GTA_POPCYCLE_TYPE_SHOPPING_POSH (14)
#define GTA_POPCYCLE_TYPE_RESIDENTIAL_RICH_SECLUDED (15)
#define GTA_POPCYCLE_TYPE_AIRPORT (16)
#define GTA_POPCYCLE_TYPE_GOLF_CLUB (17)
#define GTA_POPCYCLE_TYPE_OUT_OF_TOWN_FACTORY (18)
#define GTA_POPCYCLE_TYPE_AIRPORT_RUNWAY (19)

#define GTA_POPCYCLE_FILE "popcycle.dat"
#define GTA_POPCYCLE_MAX_LINE_LENGTH (512)
#define GTA_POPCYCLE_MAX_COMMENT_LENGTH (128)
#define GTA_POPCYCLE_MAX_TOKEN_LENGTH (64)

enum E_GTA_POPCYCLE_FIELD {
    PC_PEDS,
    PC_CARS,
    PC_DEALERS,
    PC_GANG,
    PC_COPS,
    PC_OTHER,
    PC_WORKERS,
    PC_BUSINESS,
    PC_CLUBBERS,
    PC_FARMERS,
    PC_BEACHFOLK,
    PC_PARKFOLK,
    PC_CASUAL_RICH,
    PC_CASUAL_AVERAGE,
    PC_CASUAL_POOR,
    PC_PROZZIES,
    PC_CRIMIMALS,
    PC_GOLFERS,
    PC_SERVANTS,
    PC_AIRCREW,
    PC_ENTERTAINERS,
    PC_OOT_FACT,
    PC_DESERTFOLK,
    PC_AIRCREW_RUNWAY
};

enum E_GTA_POPULATION_DAY {
    GTA_POPCYCLE_DAY_MONDAY = 0,
    GTA_POPCYCLE_DAY_TUESDAY,
    GTA_POPCYCLE_DAY_WEDNESDAY,
    GTA_POPCYCLE_DAY_THURSDAY,
    GTA_POPCYCLE_DAY_FRIDAY,
    GTA_POPCYCLE_DAY_SATURDAY,
    GTA_POPCYCLE_DAY_SUNDAY
};

static gPopCycle[GTA_POPCYCLE_TYPE_COUNT][GTA_POPCYCLE_DAY_COUNT][GTA_POPCYCLE_TIME_COUNT][E_GTA_POPCYCLE_FIELD];
static bool:gPopCycleLoaded;

/*
* To match GTA:SA style popcycles we need some kind of way to track days, for this to work properly we need to "tap in" into existing clock system
*/
static E_GTA_POPULATION_DAY:gPopulationDayOfWeek = GTA_POPCYCLE_DAY_MONDAY;
static gPopulationLastHour = -1;

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

static stock PopCycle_GetTypeFromLine(const line[]) {
    new section[GTA_POPCYCLE_MAX_COMMENT_LENGTH];

    if(!PopCycle_GetCommentText(line, section, sizeof(section)))
        return -1;

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

    return -1;
}

static stock bool:PopCycle_ParseRow(const line[], popType, dayType, timeSlot) {
    new values[GTA_POPCYCLE_FIELD_COUNT];
    new count = 0;
    new length = strlen(line);
    new position = 0;

    while(position < length) {
        while(position < length && (line[position] == ' ' || line[position] == '\t'))
            position++;

        if(position >= length)
            break;

        if(line[position] == '/')
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

        if(count >= GTA_POPCYCLE_FIELD_COUNT)
            return false;

        values[count++] = value * sign;
    }

    if(count != GTA_POPCYCLE_FIELD_COUNT)
        return false;

    for(new i = 0; i < GTA_POPCYCLE_FIELD_COUNT; i++)
        gPopCycle[popType][dayType][timeSlot][E_GTA_POPCYCLE_FIELD:i] = values[i];

    return true;
}

static stock bool:PopCycle_ValidatePreviousDay(currentType, currentDay, currentSlot) {
    if(currentType != -1 && currentDay != -1 && currentSlot != GTA_POPCYCLE_TIME_COUNT) {
        printf("[omp-gta-popcycle] Invalid row count: type=%d day=%d rows=%d expected=%d", currentType, currentDay, currentSlot, GTA_POPCYCLE_TIME_COUNT);
        return false;
    }
    return true;
}

/*
* Recreation of CPopCycle::Initialise(), sort of, part of it
*/
static stock PopCycle_NormalizePedWeights(popType, dayType, timeSlot) {
    new total = 0;


    for(new E_GTA_POPCYCLE_FIELD:group = PC_WORKERS; group <= PC_AIRCREW_RUNWAY; group++) {
        total += gPopCycle[popType][dayType][timeSlot][group];
    }

    if(total <= 0) {
        return;
    }

    if(total == 100) {
        return;
    }

    new Float:scale = 100.0 / float(total);

    new normalizedTotal = 0;
    new largestWeight = -1;
    new E_GTA_POPCYCLE_FIELD:largestGroup = PC_WORKERS;

    for(new E_GTA_POPCYCLE_FIELD:group = PC_WORKERS; group <= PC_AIRCREW_RUNWAY; group++) {
        new weight = floatround(float(gPopCycle[popType][dayType][timeSlot][group]) * scale, floatround_round);

        gPopCycle[popType][dayType][timeSlot][group] = weight;

        normalizedTotal += weight;

        if(weight >= largestWeight) {
            largestWeight = weight;
            largestGroup = group;
        }
    }

    gPopCycle[popType][dayType][timeSlot][largestGroup] += 100 - normalizedTotal;
}

static stock PopCycle_NormalizeAllPedWeights() {
    for(new popType = 0; popType < GTA_POPCYCLE_TYPE_COUNT; popType++) {
        for(new dayType = 0; dayType < GTA_POPCYCLE_DAY_COUNT; dayType++) {
            for(new timeSlot = 0; timeSlot < GTA_POPCYCLE_TIME_COUNT; timeSlot++) {
                PopCycle_NormalizePedWeights(popType, dayType, timeSlot);
            }
        }
    }
}

/*
* --------------------------------------------------------------------------
* USABLE FUNCTIONS
* --------------------------------------------------------------------------
*/

stock bool:LoadPopCycleDat() {
    gPopCycleLoaded = false;

    new File:file = fopen(GTA_POPCYCLE_FILE, io_read);

    if(!file) {
        printf("[omp-gta-popcycle] Failed to open %s", GTA_POPCYCLE_FILE);
        return false;
    }

    new line[GTA_POPCYCLE_MAX_LINE_LENGTH];
    new currentType = -1;
    new currentDay = -1;
    new currentSlot = 0;

    while(fread(file, line)) {
        new length = strlen(line);

        while(length > 0 && (line[length - 1] == '\r' || line[length - 1] == '\n'))
            line[--length] = '\0';

        if(PopCycle_IsLineEmpty(line))
            continue;

        new detectedType = PopCycle_GetTypeFromLine(line);

        if(detectedType != -1) {
            if(!PopCycle_ValidatePreviousDay(currentType, currentDay, currentSlot)) {
                fclose(file);
                return false;
            }

            currentType = detectedType;
            currentDay = -1;
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

                currentDay = !strcmp(comment, "Weekday", true) ? GTA_POPCYCLE_WEEKDAY : GTA_POPCYCLE_WEEKEND;
                currentSlot = 0;
            }
            continue;
        }

        if(currentType == -1 || currentDay == -1)
            continue;

        if(currentSlot >= GTA_POPCYCLE_TIME_COUNT) {
            printf("[omp-gta-popcycle] Too many rows: type=%d day=%d slot=%d", currentType, currentDay, currentSlot);
            fclose(file);
            return false;
        }

        if(!PopCycle_ParseRow(line, currentType, currentDay, currentSlot)) {
            printf("[omp-gta-popcycle] Invalid row: type=%d day=%d slot=%d", currentType, currentDay, currentSlot);
            fclose(file);
            return false;
        }

        currentSlot++;
    }

    if(!PopCycle_ValidatePreviousDay(currentType, currentDay, currentSlot)) {
        fclose(file);
        return false;
    }

    fclose(file);

    /*
    * Makes sure that all ped weights add up to 100%
    */
    PopCycle_NormalizeAllPedWeights();

    gPopCycleLoaded = true;

    printf("[omp-gta-popcycle] Loaded %d rows.", GTA_POPCYCLE_TYPE_COUNT * GTA_POPCYCLE_DAY_COUNT *GTA_POPCYCLE_TIME_COUNT);

    return true;
}

stock bool:IsPopCycleLoaded() {
    return gPopCycleLoaded;
}

/*
* Gets the population weight for a day type and time. Useful to figure out how many of certain peds, vehicles or groups types need to be spawned
*/
stock GetPopCycleValue(popType, dayType, timeSlot, E_GTA_POPCYCLE_FIELD:field) {
    if(popType < 0 || popType >= GTA_POPCYCLE_TYPE_COUNT)
        return -1;

    if(dayType < 0 || dayType >= GTA_POPCYCLE_DAY_COUNT)
        return -1;

    if(timeSlot < 0 || timeSlot >= GTA_POPCYCLE_TIME_COUNT)
        return -1;

    if(_:field < 0 || _:field >= GTA_POPCYCLE_FIELD_COUNT)
        return -1;

    return gPopCycle[popType][dayType][timeSlot][field];
}

stock GetPopCyclePeds(popType, dayType, timeSlot) {
    return GetPopCycleValue(popType, dayType, timeSlot, PC_PEDS);
}

stock GetPopCycleCars(popType, dayType, timeSlot) {
    return GetPopCycleValue(popType, dayType, timeSlot, PC_CARS);
}

stock GetPopCycleGroupPercentage(popType, dayType, timeSlot, E_GTA_POPCYCLE_FIELD:group) {
    if(group < PC_WORKERS || group > PC_AIRCREW_RUNWAY)
        return -1;

    return GetPopCycleValue(popType, dayType, timeSlot, group);
}

/*
* Gets the timeslot for popcycle functions. It GTA:SA simply makes a 12-hour clock
*/
stock GetPopCycleTimeSlot(hour) {
    if(hour < 0)
        hour = 0;
    else if(hour > 23)
        hour = 23;

    return hour / 2;
}

/*
* Recreation of CPopCycle::PickARandomGroupOfOtherPeds() from gta-reversed.
* Selects one random population group based on day type and time
*/
stock E_GTA_POPCYCLE_FIELD:GetRandomPopCyclePopulationGroup(popType, dayType, timeSlot) {
    if(popType < 0 || popType >= GTA_POPCYCLE_TYPE_COUNT || dayType < 0 || dayType >= GTA_POPCYCLE_DAY_COUNT || timeSlot < 0 || timeSlot >= GTA_POPCYCLE_TIME_COUNT) {
        return E_GTA_POPCYCLE_FIELD:-1;
    }

    new value = random(100);

    for(new E_GTA_POPCYCLE_FIELD:group = PC_WORKERS; group <= PC_AIRCREW_RUNWAY; group++) {
        new weight = gPopCycle[popType][dayType][timeSlot][group];

        if(weight <= 0) {
            continue;
        }

        if(value < weight) {
            return group;
        }

        value -= weight;
    }

    return E_GTA_POPCYCLE_FIELD:-1;
}


/*
* Recreation of GTA CPopCycle::Update() from gta-reversed, sort of.
* Getsthe day type based on gta-reversed logic by using the internal day name and the hour
*/
stock GetPopCycleDayType(hour) {
    if(hour < 0)
        return -1;
    if(hour > 23)
        return -1;

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
    switch(_:gPopulationDayOfWeek) {
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
* Detects if the day needs to changed and updates the day in case clock has been manually set to forward in time
*/
forward bool:GTAPopCycle_UpdateHour(hour);
public bool:GTAPopCycle_UpdateHour(hour) {
    if(hour < 0 || hour > 23) {
        return false;
    }

    if(gPopulationLastHour == -1) {
        gPopulationLastHour = hour;
        return true;
    }

    if(hour < gPopulationLastHour) {
        if(gPopulationDayOfWeek == GTA_POPCYCLE_DAY_SUNDAY) {
            gPopulationDayOfWeek = GTA_POPCYCLE_DAY_MONDAY;
        } else {
            gPopulationDayOfWeek = E_GTA_POPULATION_DAY:(_:gPopulationDayOfWeek + 1);
        }
    }

    gPopulationLastHour = hour;
    return true;
}

/*
* Sets a specific day of the week. Affects what kind of population group is selected
*/
stock bool:SetPopCycleDayOfWeek(E_GTA_POPULATION_DAY:day) {
    if(day < GTA_POPCYCLE_DAY_MONDAY || day > GTA_POPCYCLE_DAY_SUNDAY) {
        return false;
    }

    gPopulationDayOfWeek = day;
    gPopulationLastHour = -1;

    return true;
}

/*
* Gets the current day of the week
*/
stock E_GTA_POPULATION_DAY:GetPopCycleDayOfWeek() {
    return gPopulationDayOfWeek;
}
