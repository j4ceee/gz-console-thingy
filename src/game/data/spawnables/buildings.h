#pragma once

#include "game/data/data.h"

namespace gz::Data::Spawnables
{
    static const SpawnableData props_defenses[] = {
        {"Resistance Turret",   "building_resistance_wall_turret_01_thrown",                    0x0ecb70ff},
        {"Soviet Turret",       "building_soviet_turret_01_thrown",                             0x238a890d},
        {"Grenade Launcher",    "building_resistance_wall_grenade_launcher_turret_01_thrown",   0xc141917f},
        {"Sniper Tower",        "building_resistance_sniper_turret_01_thrown",                  0xb16ff4f6},
    };

    /*
    static const SpawnableData props_buildings[] = {

    };
    */

    static const SpawnableData props_stations[] = {
        {"Crate",               "building_resistance_station_safehouse_crate_01_thrown",        0xe124a5c1},
        {"Vehicle Station",     "building_resistance_station_vehicle_01_thrown",                0xf57519f8},
        {"Recycle Station",     "building_resistance_station_work_bench_recycle_01_thrown",     0x2d9ce7b2},
        {"Apparel Station",     "building_resistance_station_work_bench_apparel_01_thrown",     0xa27825ba},
        {"Consumables Station", "building_resistance_station_work_bench_consumables_01_thrown", 0x845fa9f8},
        {"Companion Station",   "building_resistance_station_companion_01_thrown",              0x38ce2653},
        {"FMTEL Station",       "building_resistance_station_fmtel_01_thrown",                  0xaff6397b},
    };


    static const SpawnableCategory buildings[] = {
        {"Stations", 7, props_stations},
        {"Base Defenses", 4, props_defenses, "Base defenses are active as soon as they are spawned & invulnerable."},
        //{"Props", 2, props_buildings},
    };
}// namespace gz::Data::Spawnables