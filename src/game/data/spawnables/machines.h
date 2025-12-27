#pragma once

namespace gz::Data::Spawnables
{
    static const SpawnableData machines_tick[] = {
        {"Prototype",                       "pest_classa_load01",               0x08cc7b5a},
        {"Prototype - Friendly",            "pest_classa_load02",               0x2948cc07},
        {"Prototype - Spy - Friendly",      "spy_pest_load01",                  0x76628adc},

        {"Class X - Friendly",              "pest_classx_load01",               0x55fd6052}, // named Prototype in spawn_animals_def.bin

        {"Military",                        "pest_classb_load01",               0xc142fcf4},
        {"Military - Friendly",             "pest_classb_load02",               0x4e1f9da7},
        {"Military - Bunny",                "pest_classb_load03",               0xf28b6d0e},

        {"Apocalypse",                      "pest_classd_load01",               0x1dc4144d},
        {"Apocalypse - Friendly",           "pest_classd_load02",               0x91d0f0e9},

        {"FNIX",                            "pest_classc_load01",               0x9e3ba95d},
        {"FNIX - Friendly",                 "pest_classc_load02",               0x2bcf6966},
        {"FNIX - Gnome",                    "pest_classc_load03",               0x099f81a8},

        {"Event - Gnome",                   "pest_classe1_load01",              0x6c68c372},
        {"Event - Gnome",                   "pest_classe1_load02",              0xcd844e39},


        {"Egg - Prototype",                 "pest_egg_classa_load01",           0xe2e32d82},
        {"Egg - Prototype - Friendly",      "pest_egg_classa_load01_friendly",  0xcf116b3a},

        {"Egg - Military",                  "pest_egg_classb_load01",           0x12d802f5},
        {"Egg - Military - Friendly",       "pest_egg_classb_load01_friendly",  0x813ae7eb},

        {"Egg - FNIX",                      "pest_egg_classc_load01",           0x20aed311},
        {"Egg - FNIX - Friendly",           "pest_egg_classc_load01_friendly",  0xacc99ee7},

        {"Egg - Apocalypse",                "pest_egg_classd_load01",           0x51218a2c}, // named FNIX in spawn_animals_def.bin
        {"Egg - Apocalypse - Friendly",     "pest_egg_classd_load01_friendly",  0x0c65c92d}, // named FNIX in spawn_animals_def.bin

        {"Egg - Class E1 - Gnome",          "pest_egg_classe1_gnome",           0x0cb7bdad},
    };

    static const SpawnableData machines_seeker[] = {
        {"Prototype",   "scou_classa_load01",   0xf5086f52},
        {"Military",    "scou_classb_load01",   0xbca22e4c},
        {"FNIX",        "scou_classc_load01",   0xe93f2321},
        {"Apocalypse",  "scou_classd_load01",   0x8ece7fc0},
    };

    static const SpawnableData machines_runner[] = {
        {"Prototype - Loadout 01",      "skir_classa_load01",   0xf0a8848b},

        {"Military - Loadout 01",       "skir_classb_load01",   0x447d6bfd},
        {"Military - Loadout 02",       "skir_classb_load02",   0x0a7f866c},

        {"FNIX - Loadout 01",           "skir_classc_load01",   0x85c4be23},
        {"FNIX - Loadout 02",           "skir_classc_load02",   0x0e2b2e41},
        {"FNIX - Loadout 03",           "skir_classc_load03",   0x68047faf},

        {"Apocalypse - Loadout 01",     "skir_classd_load01",   0xe5269587},
        {"Apocalypse - Loadout 02",     "skir_classd_load02",   0x84ef2130},
        {"Apocalypse - Loadout 03",     "skir_classd_load03",   0x4fbe2210},
    };

    static const SpawnableData machines_companion[] = {
        {"Loadout 01",  "comp_load01",  0xb6519bfe},
        {"Loadout 02",  "comp_load02",  0x17ef7e8e},
        {"Loadout 03",  "comp_load03",  0xc8eb8728},
    };

    static const SpawnableData machines_hunter[] = {
        {"Prototype - Loadout 01",      "hunt_classa_load01",   0xc76c91fe},
        {"Prototype - Loadout 02",      "hunt_classa_load02",   0x7fe045fe},

        {"Military - Loadout 01",       "hunt_classb_load01",   0x15a1e380},
        {"Military - Loadout 02",       "hunt_classb_load02",   0xa73bc5eb},

        {"FNIX - Loadout 01",           "hunt_classc_load01",   0xddacd521},
        {"FNIX - Loadout 02",           "hunt_classc_load02",   0x97c788ba},

        {"Apocalypse - Loadout 01",     "hunt_classd_load01",   0x47e1f6d0},
        {"Apocalypse - Loadout 02",     "hunt_classd_load02",   0xadf51c1e},
        {"Apocalypse - Loadout 03",     "hunt_classd_load03",   0xc03f662d},
    };

    static const SpawnableData machines_harvester[] = {
        {"Prototype - Loadout 01",      "harv_classa_load01",   0x70b1a286},
        {"Prototype - Loadout 02",      "harv_classa_load02",   0x6e26a617},

        {"Military - Loadout 01",       "harv_classb_load01",   0x0ff06732},
        {"Military - Loadout 02",       "harv_classb_load02",   0xebb11321},

        {"FNIX - Loadout 01",           "harv_classc_load01",   0x4ae7fe60},
        {"FNIX - Loadout 02",           "harv_classc_load02",   0x3771d109},
        {"FNIX - Loadout 03",           "harv_classc_load03",   0x3e1978a0},

        {"Apocalypse - Loadout 01",     "harv_classd_load01",   0xa4174988},
        {"Apocalypse - Loadout 02",     "harv_classd_load02",   0xf276b7e8},
        {"Apocalypse - Loadout 03",     "harv_classd_load03",   0x8fab7d33},
    };

    static const SpawnableData machines_tank[] = {
        {"Prototype - Loadout 01",      "drea_classa_load01",   0xd93a35a0},
        {"Prototype - Loadout 02",      "drea_classa_load02",   0x38fc9402},
        {"Prototype - Loadout 03",      "drea_classa_load03",   0x4c5e0cc4},

        {"Military - Loadout 01",       "drea_classb_load01",   0xbc680883},
        {"Military - Loadout 02",       "drea_classb_load02",   0x726c5741},
        {"Military - Loadout 03",       "drea_classb_load03",   0x988c1f72},

        {"FNIX - Loadout 01",           "drea_classc_load01",   0x51fbeaf9},
        {"FNIX - Loadout 02",           "drea_classc_load02",   0xe048a1dd},
        {"FNIX - Loadout 03",           "drea_classc_load03",   0xa565adb2},

        {"Apocalypse - Loadout 01",     "drea_classd_load01",   0x04ba4392},
        {"Apocalypse - Loadout 02",     "drea_classd_load02",   0x9b2b049c},
        {"Apocalypse - Loadout 03",     "drea_classd_load03",   0x29090148},

        {"Reaper - Loadout 01",         "drea_classr1_load01",  0x0b0181ad},
        {"Reaper - Loadout 02",         "drea_classr1_load02",  0x3bd3feac},
        {"Reaper - Loadout 03",         "drea_classr1_load03",  0x23394970},
    };

    static const SpawnableData machines_lynx[] = {
        {"Scout - Loadout 01",      "lynx_classa_load01",   0x96bd0b95},

        {"Soldier - Loadout 02",    "lynx_classb_load01",   0x30fbcaa6},
        {"Soldier - Loadout 02",    "lynx_classb_load02",   0xae6989c2},

        {"Spetsnaz - Loadout 01",   "lynx_classc_load01",   0x83d89235},
        {"Spetsnaz - Loadout 02",   "lynx_classc_load02",   0x07ff4b57},
        {"Spetsnaz - Loadout 03",   "lynx_classc_load03",   0x17922159},
    };

    static const SpawnableData machines_firebird[] = {
        {"Scout - Loadout 01",      "firebird_classa_load01",   0x04846a5b},
        {"Scout - Loadout 02",      "firebird_classa_load02",   0xa2536725},

        {"Soldier - Loadout 01",    "firebird_classb_load01",   0x5c216bb5},
        {"Soldier - Loadout 02",    "firebird_classb_load02",   0xd1e51900},

        {"Spetsnaz - Loadout 01",   "firebird_classc_load01",   0xcfe349c8},
        {"Spetsnaz - Loadout 02",   "firebird_classc_load02",   0x04ce6771},

        {"Vulture - Loadout 01",    "firebird_classr1_load01",  0x3e3a7561},
    };

    static const SpawnableData machines_wolf[] = {
        {"Scout - Loadout 01",      "wolf_classa_load01",   0xe437d895},

        {"Soldier - Loadout 01",    "wolf_classb_load01",   0xb04bd31f},
        {"Soldier - Loadout 02",    "wolf_classb_load02",   0x97d0fcfc},

        {"Spetsnaz - Loadout 01",   "wolf_classc_load01",   0xdd0d0a29},
        {"Spetsnaz - Loadout 02",   "wolf_classc_load02",   0x895ca072},
        {"Spetsnaz - Loadout 03",   "wolf_classc_load03",   0xde26ed12},
    };

    static const SpawnableCategory all_machines[] = {
        {"Tick", 23, machines_tick},
        {"Seeker", 4, machines_seeker},
        {"Runner", 9, machines_runner},
        {"Companion", 3, machines_companion},
        {"Hunter", 9, machines_hunter},
        {"Harvester", 10, machines_harvester},
        {"Tank", 15, machines_tank},
        {"Lynx", 6, machines_lynx},
        {"Firebird", 7, machines_firebird},
        {"Wolf", 6, machines_wolf},
    };
} // namespace gz::Data::Spawnables
