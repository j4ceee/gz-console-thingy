<p align="center">
  <img width="50%" align="center" src="https://jacee.dev/img/mods/gz-console-thingy.png" alt="">
</p>
  <h1 align="center">
  Generation Zero Console Thingy
</h1>
<p align="center">
  A mod for Generation Zero that adds a GUI console for modifying game values and executing commands
</p>
<br>

> [!IMPORTANT]
> This mod modifies game memory and may lead to instability or crashes. 
> Use at your own risk and back-up your saves!

## Features
- in-game GUI console accessible via a hotkey (_F1_ by default / _Left Stick + Right Stick_ on controllers)
- built with ImGui for easy integration and ease of use for players
- allows players to modify game values and execute commands in real-time
- Supported game versions / platforms:
  - PC - Steam, Microsoft Store / Xbox
  - Steam Deck / Linux

### Player Settings
- **Hiding the HUD** for a cleaner experience (Hotkey: _F3_ by default)
- **Third Person** mode (Hotkey: _F4_ by default / _2x Right Stick_ on controllers)
- **Health Modification** & **Invincibility**
- **Infinite Stamina**
- **Unlimited Carry Weight** for the player inventory
- **Unlimited Storage Size** for all containers (e.g. player inventory, companion, base storage, ...)
- **Unlimited Ammo** (including no reload)
- **Infinite Deployables & Consumables** (e.g. mines, turrets, medkits)
- **Unlimited Resources** (e.g. for crafting or base building)
- **Fast Travel Anywhere**: travel to any discovered location **and** to your own waypoint over the in-game map
- **Bullet Damage Multiplier**: increase the bullet damage dealt to enemies
- Editing **Player Faction**: machines from the selected faction will treat you as an ally (available factions: Resistance, FNIX, Soviets)
- **Disabling Detection** by enemies
- **Unrestricted Base Building** (removing build limits and restrictions)
- Adding **Command Tokens**
- **Infinite Vehicle Fuel**
- **Vehicle Acceleration & Speed** modification
- **Disable Air Resistance** for vehicles (increases top speed)
- **Vehicle Health Modification** & **Vehicle Invincibility**
- Setting **Player Level & XP**
- Adding **Skill Points**
- **First Person Shadow** can be toggled on/off
- **Ignore Deep Water** allows the player to walk underwater
- **Ghost Mode**: float through walls and objects (no clipping)

### Machines
- **Machine Spawning**: Spawn any machine anywhere
- **Hacking Always Succeeds**: all hacking attempts for machines will automatically succeed
- **Unlimited Hacking Duration**: hacked machines will stay hacked for a long time without needing to re-hack them
- **Advanced Hacking**: machines will always attack other machines, even when the 'Hacker' specialization skill is not active

#### Target Machine
- Modify various parameters of the currently targeted machine (aim at a machine and open the console)
  - make it **Invulnerable**
  - **Kill** it instantly
  - set its **Faction** (Resistance, FNIX, Soviets)
  - **Control Targeted Machine**: remote control the targeted machine

#### Controllable Machines / Play as Machines
- **Unlimited Signal Strength** for controlled machines (you can move further away without losing control)
- **Control Machine on Hack**: seamlessly take control of a machine when you hack it with the binoculars (no need to open the mod menu)
- any machine can be controlled by the player
- controlled machines have 2 abilities that are listed at the bottom of the screen:
  - **Primary Ability** (usually a weapon attack)
  - **Secondary Ability** (usually a movement or special ability)
- controlled machines can be made...
  - **Invulnerable**
  - **Undetectable by other Machines**
  - or instantly **Killed**
- you adjust the **Third Person Camera Distance** while controlling a machine (in the mod UI or by using the mouse scroll wheel)
- please refer to the in-game tooltips for details and restrictions for the different options (hover over the `(?)`)

### World Settings
- **Event Control** (any event can be activated at any time + option to let game decide)
- **Teleportation**
  - ... to specific coordinates
  - ... predefined locations (collectibles)
  - ... or to the current aim position (Hotkey: _F2_ by default)
- **Gravity Control** for physics objects & player character
- **Control over Time of Day**
  - **Mirror Real Time** (sync in-game time with your system clock)
  - freeze time
  - adjust time speed
  - set specific time
- **Weather Control**: set atmospheric conditions
  - cloud density
  - downpour (snow & rain)
  - ground snow (snow covering the world)
  - ground wetness
  - lightning chance during cloudy weather & rain
  - cloud height & range
  - wind strength
- **GFX / Post-Process Control**: apply various post-processing effects & filters (allows you to apply any effect from photo mode or the binoculars)
- **Spawn System**
  - Vehicles
  - Items & Deployables
  - Buildings (like base defenses that will help you in battle or the different stations, like the workbenches or the crate) 
  - Human NPCs

### Game Settings
- **Quicker Startup**: the game will start loading while the splash screens are still playing

### Mod Configuration
- Fully configurable hotkeys for opening the console, teleporting, and toggling the HUD


## Installation
1. Download the latest release from the [Releases](https://github.com/j4ceee/gz-console-thingy/releases/latest)
2. Extract the contents of the ZIP file
3. You will find a folder named `GenerationZeroConsoleThingy` containing the mod file (`xinput9_1_0.dll`), this `README.md` and some license files
4. Move the `xinput9_1_0.dll` file to your Generation Zero installation directory (where `GenerationZero.exe` is located)
5. Launch Generation Zero and you can now open the console using _F1_ (default hotkey) on your keyboard or _LS + RS_ on your controller

### Linux / Steam Deck
- add `WINEDLLOVERRIDES="xinput9_1_0.dll=n,b" %command%` to your launch options for Generation Zero in Steam

## Credits
- Developed by [jacee](https://github.com/j4ceee)
- Based on [jc4-console-thingy](https://github.com/aaronkirkham/jc4-console-thingy) by Aaron Kirkham
- ...with the help of the Cheat Engine Tables by [aSwedishMagyar](https://fearlessrevolution.com/viewtopic.php?p=227828#p227828), [sanitka](https://fearlessrevolution.com/viewtopic.php?t=33780) and [pigeon](https://fearlessrevolution.com/viewtopic.php?t=8996)
- Built using [ImGui](https://github.com/ocornut/imgui)
- Fonts: 
  - [Material Design Icons](https://github.com/google/material-design-icons/blob/master/font/MaterialIcons-Regular.ttf) by Google (Apache 2.0 License)
  - [Roboto](https://fonts.google.com/specimen/Roboto) by Google (SIL Open Font License, 1.1)


## Building
To build the mod from source, you will need:

### Prerequisites
* C++17 compatible compiler (Visual Studio 2017 or later recommended)
* CMake 3.16 or later
* Git (for cloning and managing submodules)

### Cloning the Repository
```
git clone --recurse-submodules https://github.com/j4ceee/gz-console-thingy.git
cd gz_console_thingy_dev
```

### Building with CLion
1. Open the project folder in CLion
2. CLion will automatically detect the CMake configuration and set up the project
3. Configure your CMake profile (if needed):
   - Build type: `Release` (or `Debug` for development)
   - Toolchain: `Visual Studio`
   - Generator: `Visual Studio 17 2022`
4. In the toolbar, select your CMake profile (e.g., `Release-Visual Studio`)
5. Select the build target `xinput9_1_0` or `address-generator`
6. Click the Build button
7. The compiled DLL will be located in `cmake-build-<config>/out/<config>/xinput9_1_0.dll` and copied to the Generation Zero directory as specified in the `CMakeLists.txt`

**Build Types:**
- **Debug**: Includes extra logging and debugging symbols (larger file size, slower)
- **Release**: Optimized build for normal use (smaller, faster)

**Note:** The CMakeLists.txt includes a post-build command that copies the DLL to:
```
C:/Program Files (x86)/Steam/steamapps/common/GenerationZero/
```
If your game is installed elsewhere, modify line 134 in `CMakeLists.txt` accordingly.


### Building with Command Line (CMake)
1. Create a build directory
    ```
    mkdir build
    ```
2. Configure for Release build
    ```
    cmake -DCMAKE_BUILD_TYPE=Release -B .\build\ -G "Visual Studio 17 2022"
    ```
   ...or for Debug build
    ```
    cmake -DCMAKE_BUILD_TYPE=Debug -B .\build\ -G "Visual Studio 17 2022"
    ```
3. Build
    ```
    cmake --build build --config Release
    ```

The compiled DLL will be in `build/out/Release/` or `build/out/Debug/`