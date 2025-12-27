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
- in-game GUI console accessible via a hotkey (F1 by default)
- built with ImGui for easy integration and ease of use for players
- allows players to modify game values and execute commands in real-time

### Player Settings
- **Health Modification** & **Invincibility**
- **Infinite Stamina**
- **Unlimited Ammo** (including no reload) and **Infinite Deployables**
- **Unlimited Resources** (e.g. for crafting or base building)
- **Hiding the HUD** for a cleaner experience (Hotkey: F3 by default)
- **Disabling Detection** by enemies
- **Unrestricted Base Building** (removing build limits and restrictions)
- Adding **Command Tokens**
- **Infinite Vehicle Fuel**
- **Vehicle Health Modification**
- Setting **Player Level & XP**
- Adding **Skill Points**

### World Settings
- **Event Control** (any event can be activated at any time + option to let game decide)
- **Teleportation**
  - ... to specific coordinates
  - ... predefined locations (collectibles)
  - ... or to the current aim position (Hotkey: F2 by default)
- **Gravity Control** for physics objects
- **Control over Time of Day**
  - freeze time
  - adjust time speed
  - set specific time
- **Weather / GFX Control**
  - set atmospheric conditions, including wind, fog & cloud cover (rain or snow cannot trigger directly)
  - apply various post-processing effects & filters (allows you to apply any effect from photo mode or the binoculars)
- **Spawn System**
  - Vehicles
  - Buildings
  - Human NPCs
  - Machines (very limited, most machines are currently spawned without AI)

### Mod Configuration
- Fully configurable hotkeys for opening the console, teleporting, and toggling the HUD


## Installation
1. Download the latest release from the [Releases](https://github.com/j4ceee/gz-console-thingy/releases/latest)
2. Extract the contents of the ZIP file
3. You will find a folder named `GenerationZeroConsoleThingy` containing the mod file (`xinput9_1_0.dll`), this `README.md` and some license files
4. Move the `xinput9_1_0.dll` file to your Generation Zero installation directory (where `GenerationZero.exe` is located)
5. Launch Generation Zero and you can now open the console using F1 (default hotkey)

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