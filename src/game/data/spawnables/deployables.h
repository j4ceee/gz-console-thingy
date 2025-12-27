#pragma once

#include "game/data/data.h"

namespace gz::Data::Spawnables
{
    static const SpawnableData items_thrown[] = {
        {"Health Station",              "health_station_01_thrown",                         0x22398434},
        {"Ammo Station",                "ammo_station_01_thrown",                           0xfa1260ea},

        {"Fireworks 01",                "equipment_distraction_firework_box_01_thrown",     0x486b1e14},
        {"Fireworks 02",                "equipment_distraction_firework_box_02_thrown",     0x928f5468},
        {"Firework Rocket",             "firework_rocket",                                  0x85916749},

        {"Firebird Vulture Turret Drop","machine_support_firebird_turret_drop",             0x253db3d2},
        {"Firebird Vulture Beacon Drop","machine_support_firebird_beacon_drop",             0x5c35ceb7},
        {"Firebird Vulture Gas Drop",   "machine_support_firebird_gas_canister_drop",       0x66e0ead2},

        {"Light Comm. Array",           "equipment_lure_commarray_01_thrown",               0xfd397571},
        {"Medium Comm. Array",          "equipment_lure_commarray_02_thrown",               0x2c4a0ef8},
        {"Heavy Comm. Array",           "equipment_lure_commarray_03_thrown",               0xd188b587},

        {"Proximity Bomb",              "equipment_prox_bomb_01",                           0x8990173b},
    };

    static const SpawnableData items[] = {
        {"Boombox",                 "boombox_01",                                       0xe73368a7},
        {"Radio",                   "civilian_01_radio_01",                             0x6ab844ca},

        {"Mine",                    "equipment_mine_01_thrown",                         0x4b2c20c8},

        {"Car Battery",             "equipment_emp_batterypack_car_01_thrown",          0x27b4c1fc},
        {"Large Battery",           "equipment_emp_batterypack_large_01_thrown",        0xaf6cd00f},

        {"Propane Tank (Poison)",   "equipment_propane_tank_04_poison_thrown",          0x593a90b8},
        {"Propane Tank (Explosive)","equipment_propane_tank_04_explosive_thrown",       0xd9ffafaf},
        {"Propane Tank (Air)",      "equipment_propane_tank_04_air_thrown",             0x6174695a},
    };

    static const SpawnableCategory all_deployables[] = {
        {"Deployed Items", 12, items_thrown, "These items are already deployed and will be immediately active upon spawning."},
        {"Carryable Items", 8, items, "These items can be picked up and manually deployed by the player."},
    };
}// namespace gz::Data::Spawnables