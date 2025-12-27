## Complete Function Reference

### Building System Overview

- In Building Mode there's a grid overlay on the ground, the selected building hovers in front of the player and snaps to the grid.
- The player can rotate it, move it around, and see it highlighted in green/red depending on whether placement is valid.
- If placement is valid and the player confirms, the building is spawned and resources are deducted (Note: if the final placement check fails, resources are refunded because they are deducted either way first).
- For the collision/overlap checks the UI checks are separate from the actual placement checks. If the UI check fails, the placement check will never be called.
- The collision UI & collision placement checks also play different error sounds
- The collision placement check does not affect the highlighting color at all

#### TODO: Remove Building Limit
- there is a hardcoded building limit per category (e.g. 5 defense buildings, 150 deco buildings, etc)

### **Building System Discovery Functions (NOT PATCHED)**

**1. FUN_140c4d380** - Building Catalog Enumeration
- **Purpose**: Populates building/construction menu UI
- **What it does**: 
  - Iterates through `m_BuildingArray`
  - Collects metadata: name, description, icon, dimensions, category
  - Checks DLC ownership, resource requirements, build limits
- **Ghidra Address**: `0x140c4d380`

**2. FUN_140c64d80** - Building Placement Update Loop
- **Purpose**: Frame-by-frame update when in building mode
- **What it does**:
  - Runs state machine (6 different states)
  - Validates resources availability
  - Updates UI flags: `m_State`, `m_CanPlaceBuilding`, `m_ShowInfo`, `m_BuildLimitReached`
  - Has timer/cooldown system (uses deltaTime parameter)
- **Ghidra Address**: `0x140c64d80`

### **UI Validation Functions (PATCHED)**

**3. FUN_140725490 / _CT_Building_UI_Coll_Val** - UI Preview Validation ✅ PATCHED
- **Purpose**: Visual placement indicator (green/red preview) during building mode depending on collision/overlap
- **What it does**:
  - Checks placement rules (FUN_140bbce50)
  - Does physics raycast (FUN_140216750) 
  - Writes result to `[rsi+380]` flag
- **Patches Applied**:
  - **Check 1** (`0x140725640`): `xor al,al` → `mov al,1` (proximity validation)
  - **Check 2** (`0x140725816`): `and [rsi+380],bl` → NOPs (combines all checks)
- **Pattern 1**: `"E8 ? ? ? ? 84 C0 74 04 B0 01 EB 02 32 C0 88 86"` +13
- **Pattern 2**: `"48 8D 4D ? E8 ? ? ? ? 20 9E 80 ? ? ?"` +9

### **Placement Execution Functions**

**4. FUN_140bf96d0 / _CT_PARENT_PLACEMENT_HANDLER** - Parent Placement Handler (NOT PATCHED)
- **Purpose**: Orchestrates placement attempt
- **What it does**:
  - Calls `FUN_140bbba00 / _CT_BUILDING_SPAWN` (spawn function)
  - Tests return value
  - If `0` → calls refund chain (`FUN_140be5760 / _CT_MATERIAL_REFUND_TRIGGER`)
  - If non-zero → success handling
- **Ghidra Address**: `0x140bf96d0`
- **Call Site**: `0x140bf9b08`

**5. FUN_140bbba00 / _CT_BUILDING_SPAWN** - Main Spawn & Validation (NOT PATCHED)
- **Purpose**: Core spawn logic with multiple validations
- **What it does**:
  - Building definition lookup (`FUN_140bd0460`) (maybe)
  - Calls Collision/overlap check (`FUN_140bbaeb0 / _CT_BUILDING_GRID_OCCUPANCY`)
  - Permission/availability (vtable call at `+0x40`)
  - Entity spawn (vtable call at `+0x18`) (maybe)
  - Creates building entity if all checks pass
- **Ghidra Address**: `0x140bbba00`

**6. FUN_140bbaeb0 / _CT_BUILDING_GRID_OCCUPANCY** - Grid Occupancy Validation ✅ PATCHED
- **Purpose**: Final collision/overlap check
- **What it does**:
  - called from `FUN_140bbba00`
  - Checks if grid cells already occupied
  - Compares against `DAT_14284b89c` (empty marker)
  - Registers building in grid
  - Sets up collision data
- **Patches Applied**:
  - **Occupancy check** (`0x140bbb00c`): `jnz LAB_140bbaef0` → NOPs (ignore occupied cells)
- **Pattern**: `"47 ? ? ? 0F 85 ? ? ? ? 45 ? ? 75 ? 45"` +4
- **Ghidra Address**: `0x140bbaeb0`

### **Resource/Refund Functions**

**7. FUN_14071b0e0 / _CT_MATERIAL_TRANS_HANDLER** - Material Transaction Handler (NOT PATCHED)
- **Purpose**: Handles material deduction with rollback capability
- **What it does**:
  - Called ONLY when placement fails
  - Validates and potentially refunds materials
  - We explored this but didn't use it in final solution
- **Ghidra Address**: `0x14071b0e0`

**8. FUN_140be5760 / _CT_MATERIAL_REFUND_TRIGGER** - Refund Trigger (NOT PATCHED)
- **Purpose**: Iterates resources and calls deduction function
- **What it does**:
  - Loops through building's required resources
  - Calls `FUN_140bbc990` for each
  - Part of the refund chain
- **Ghidra Address**: `0x140be5760`

**9. FUN_1407460a0** - Universal Resource Deduction ✅ PATCHED
- **Purpose**: THE resource removal function for entire game
- **What it does**:
  - Calculates: `new_amount = current - cost`
  - Writes to inventory
  - Used for: building, crafting, everything!
  - in very rare cases some resources still get deducted (e.g. when crafting), but effectiveness of ~90%
- **Patches Applied**:
  - **Deduction** (`0x1407460ce`): `sub edx,edi` → `mov edx,edx` (keep original value)
- **Pattern**: `"2B D7 41 ? ? ? 49 8B ?"` +0
- **Ghidra Address**: `0x1407460a0`
- **Note**: Virtual function (called via vtable)

## Summary of Patches Applied

| Function                | Address      | Original           | Patched       | Effect                  |
|-------------------------|--------------|--------------------|---------------|-------------------------|
| FUN_140725490 (check 1) | 0x140725640  | `xor al,al`        | `mov al,1`    | Bypass proximity check  |
| FUN_140725490 (check 2) | ~0x140725816 | `and [rsi+380],bl` | 6 NOPs        | Skip result combination |
| FUN_140bbaeb0           | 0x140bbb00c  | `jnz` (6 bytes)    | 6 NOPs        | Ignore occupied cells   |
| FUN_1407460a0           | 0x1407460ce  | `sub edx,edi`      | `mov edx,edx` | Infinite resources      |
