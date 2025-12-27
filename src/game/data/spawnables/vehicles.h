#pragma once

#include "game/data/data.h"

namespace gz::Data::Spawnables
{
    /* DLC
    static const SpawnableData vehicles_flakmoped[] = {
        {"Utility (Red)",       "v018_bike_utility_flakmoped_red",          0xda3eb466},
        {"Utility (Yellow)",    "v018_bike_utility_flakmoped_yellow",       0xf4d896be},
        {"Utility (Green)",     "v018_bike_utility_flakmoped_green",        0x73ed42f6},
        {"Utility (Blue)",      "v018_bike_utility_flakmoped_default",      0xc71e1535},
        {"Utility (Purple)",    "v018_bike_utility_flakmoped_purple",       0xf701b621},
        {"Utility (Pink)",      "v018_bike_utility_flakmoped_pink",         0x0bd94a7e},

        {"Off-Road (Red)",      "v019_bike_offroad_flakmoped_red",          0x18f1de45},
        {"Off-Road (Yellow)",   "v019_bike_offroad_flakmoped_yellow",       0xaee03dff},
        {"Off-Road (Green)",    "v019_bike_offroad_flakmoped_green",        0xb4b2f2b1},
        {"Off-Road (Blue)",     "v019_bike_offroad_flakmoped_default",      0xfd65dde5},
        {"Off-Road (Purple)",   "v019_bike_offroad_flakmoped_purple",       0x9d413103},
        {"Off-Road (Pink)",     "v019_bike_offroad_flakmoped_pink",         0x5b062b45},

        {"Ramming (Red)",       "v017_bike_ramming_flakmoped_red",          0x24bbb973},
        {"Ramming (Yellow)",    "v017_bike_ramming_flakmoped_yellow",       0x432fd88c},
        {"Ramming (Green)",     "v017_bike_ramming_flakmoped_default",      0x56c4ed5d},
        {"Ramming (Blue)",      "v017_bike_ramming_flakmoped_blue",         0xb9b3689d},
        {"Ramming (Purple)",    "v017_bike_ramming_flakmoped_purple",       0xfa76bcd2},
        {"Ramming (Pink)",      "v017_bike_ramming_flakmoped_pink",         0x3a485e6c},

        {"Default (Red)",       "v015_bike_flakmoped_red",                  0x55e07972},
        {"Default (Yellow)",    "v015_bike_flakmoped_yellow",               0xec9df9d4},
        {"Default (Green)",     "v015_bike_flakmoped_green",                0x5da7d38f},
        {"Default (Blue)",      "v015_bike_flakmoped_default",              0xdc7bf493},
        {"Default (Purple)",    "v015_bike_flakmoped_purple",               0xbf331ad5},
        {"Default (Pink)",      "v015_bike_flakmoped_pink",                 0xb3b98a81},
    };
    */

    static const SpawnableData vehicles_bike[] = {
        {"Moped (Red)",         "v012_bike_moped_red_01",                   0xcbb23698},
        {"Moped (Yellow)",      "v012_bike_moped_yellow_01",                0xa3f8b75e},
        {"Moped (Green)",       "v012_bike_moped_green_01",                 0x6d346ab6},
        {"Moped (Blue)",        "v012_bike_moped_blue_01",                  0x444b860c},
        {"Moped (Purple)",      "v012_bike_moped_purple_01",                0x18c6c1c9},
        {"Moped (Pink)",        "v012_bike_moped_pink_01",                  0x3161a616},

        /* DLC
        {"Military (Red)",      "v013_bike_military_red_01",                0xd9957331},
        {"Military (Yellow)",   "v013_bike_military_yellow_01",             0x6366a30b},
        {"Military (Green)",    "v013_bike_military_green_01",              0xa935dfec},
        {"Military (Blue)",     "v013_bike_military_blue_01",               0x0e910345},
        {"Military (Purple)",   "v013_bike_military_purple_01",             0xbf6a7b0a},
        {"Military (Pink)",     "v013_bike_military_pink_01",               0xa65e1563},

        {"Dirtbike (Red)",      "v011_bike_dirtbike_red_01",                0x3f316a02},
        {"Dirtbike (Yellow)",   "v011_bike_dirtbike_yellow_01",             0x8e8a8592},
        {"Dirtbike (Green)",    "v011_bike_dirtbike_green_01",              0xbf051f2d},
        {"Dirtbike (Blue)",     "v011_bike_dirtbike_blue_01",               0x2cef5949},
        {"Dirtbike (Purple)",   "v011_bike_dirtbike_purple_01",             0xb4f8d6ad},
        {"Dirtbike (Pink)",     "v011_bike_dirtbike_pink_01",               0xc51a652b},

        {"Resistance (Red)",    "v014_bike_resistance_red_01",              0x5685034d},
        {"Resistance (Yellow)", "v014_bike_resistance_yellow_01",           0xf9d53415},
        {"Resistance (Green)",  "v014_bike_resistance_green_01",            0x9e1b5103},
        {"Resistance (Blue)",   "v014_bike_resistance_blue_01",             0xf96a3a14},
        {"Resistance (Purple)", "v014_bike_resistance_purple_01",           0xc29f8fe7},
        {"Resistance (Pink)",   "v014_bike_resistance_pink_01",             0x600eeba5},
        */
    };

    static const SpawnableData vehicles_bicycles[] = {
        {"Bicycle (Red)",       "v007_bike_bicycle_red",                    0xebdea6aa},
        {"Bicycle (Yellow)",    "v007_bike_bicycle_yellow",                 0xc3224560},
        {"Bicycle (Green)",     "v007_bike_bicycle_green",                  0xc32daabe},
        {"Bicycle (Blue)",      "v007_bike_bicycle_blue",                   0x4dc42fb3},
        {"Bicycle (Purple)",    "v007_bike_bicycle_purple",                 0x7dc3029b},
        {"Bicycle (Pink)",      "v007_bike_bicycle_pink",                   0xa9d27973},

        {"BMX (Blue)",          "v003_bike_bmx_blue",                       0x473ac880},
        {"Speedster (Yellow)",  "v005_bike_speedster_yellow",               0x541e4e8c},

    };

    static const SpawnableData vehicles_misc[] = {
        {"Truck (NPC)",         "v010_car_truck_npc_driver",                0xb1b2c0a7},
        {"Truck (Player)",      "v010_car_truck_player_driver",             0xcb3aeec4},
        {"Truck (Base)",        "v010_car_truck_base",                      0xead59515},
    };

    static const SpawnableCategory all_vehicles[] = {
        {"Bicycles", 8, vehicles_bicycles},
        //{"Bikes", 25, vehicles_bike},
        {"Bikes", 6, vehicles_bike},
        //{"Flakmopeds", 24, vehicles_flakmoped},
        {"Misc", 3, vehicles_misc},
    };

} // namespace gz::Data::Spawnables
