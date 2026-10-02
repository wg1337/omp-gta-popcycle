# What is it?

This is a simple include for open.mp to load the values from GTA:SA "popcycle.dat" file into Pawn. This file contains values for what kind of "groups" spawn in singleplayer GTA:SA in a certain area type based on day of the week and time. These "groups" refer to GTA:SA population, that can be "peds" or "vehicles" depending on use case. This is a recreation of "CPopCycle::Initialise()" from gta-reversed along with some helper functions. Be aware, this include is not very useful by itself. If you wanted to populate open.mp with peds and vehicles based on the logic in GTA:SA, then this is going to be a part of, specifically this include provides "how big of the total population should a certain population group be" and returns a percentage. It becomes much more useful when you also include zone information from "info.zon".

# How to use it?

1) Place "popcycle.dat" in your "scriptfiles/" folder
2) Include it in your script:
```
#include <omp_gta_popcycle>

public OnFilterScriptInit() {
    ....
    if(!LoadPopCycleDat()) {
        print("ERROR: Failed to load popcycle.dat");
        return false;
    }
    ....
    return true;
}
```
3) You probably want to attach the function that tracks the day of the week to your existing time progression system, for example:
```
public OnWorldHourChange(hour) {
...
    CallRemoteFunction("GTAPopCycle_UpdateHour", "i", hour);
...
    return true;
}
```
4) Use the provided funcations, for example:
```
//Gets how many of the population can be with type "GTA_POPCYCLE_TYPE_BUSINESS"
new cars = GetPopCycleCars(GTA_POPCYCLE_TYPE_BUSINESS, GetPopCycleDayType(hour), GetPopCycleTimeSlot(hour));
```
