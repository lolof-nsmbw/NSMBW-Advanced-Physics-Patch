# NSMBW Advanced Physics & Movement Patch

![Game](https://img.shields.io/badge/Game-NSMBW-red)
![Versions](https://img.shields.io/badge/Versions-NTSC--U%20Rev1%20%7C%20PAL%20Rev2-ff69b4)
![Platform](https://img.shields.io/badge/Platform-Wii%20%7C%20Dolphin-blue)
![Compiler](https://img.shields.io/badge/Compiler-CodeWarrior-yellow)
![Toolchain](https://img.shields.io/badge/Toolchain-Kamek-lightgrey)
![License](https://img.shields.io/badge/License-MIT-green)

An advanced physics and movement patch for *New Super Mario Bros. Wii* (NSMBW). This project was created to enrich the vanilla physics engine with new mechanics, expanding creative possibilities for level designers (especially for **Kaizo hacks**) and enhancing overall gameplay. 

This patch can be used seamlessly with NSMBW, *NewerSMBW*, or other custom hacks via a simple Riivolution XML.

## Features

### 1. SMW-Style Spin Jump
* **Spike Immunity:** You can now safely spin-bounce on Thwomps, Spikeballs, and other spiky enemies without taking damage!
* **Variable Height Control:** Spin jump height is now dynamic based on how long you hold the jump button.
* **Delayed Descent:** Holding the jump button during a spin jump reduces fall speed (hovering effect, just like in *Super Mario World*). Releasing the button immediately cancels the delay, returning you to normal fall speed.

### 2. Yoshi Enhanced
* **Spiked Enemy Bouncing:** Just like in SMW, Yoshi can now safely bounce on spiky enemies instead of taking damage.
* **Cooligan Fix:** Fixed a vanilla bug where Yoshi would take damage from Cooligans. Yoshi can now crush them safely.
* **Terrain Immunity:** Yoshi is no longer afraid of spiked floors or Munchers.

### 3. Dynamic Climbing Jumps
* Optimized the jump height when detaching from **climbable terrain**. Instead of the vanilla fixed 2-block height, it now features a **dynamic 2-4 block height** controlled entirely by the player's button input.

### 4. Up-Throw & Instant Drop Mechanics
* **Item Up-Throw:** Players can now throw items directly upwards! Supported items include: Shells, Bombs, Glow Blocks, Propeller Blocks, POW Blocks, and Springs.
* **Instant Drop:** Pressing down allows players to instantly drop Shells and Bombs at their feet.
* **Momentum Inheritance:** Up-thrown and dropped items inherit the player's X-axis velocity in a certain proportion for realistic physics trajectories.

---

## Compatibility & Usage Notes (For Releases)

The `Release` tab contains ready-to-use XML patches. 
* **Supported Versions:** NTSC Rev 1 (`SMNE01`) and PAL Rev 2 (`SMNP01`).
* **Riivolution Ready:** Simply drop the `.xml` file into your SD card's `riivolution` folder, and place the `.bin` patch files in the designated directory.

> [!WARNING]
> **Important Compatibility Warnings:** This patch can be mixed with many other XML mods, but **WILL CONFLICT** under the following conditions:
> 
> 1. **Hook Conflicts:** If another mod intercepts the vanilla damage calculation function using the same address (e.g., `mkwcat`'s dynamic `.rel` injection for death/damage messages), features in this patch will fail or cause the game to crash. You will need to manually recompile conflicting code to make them compatible.
> 2. **Memory Overlap:** This patch heavily utilizes the free memory area from `0x80001800` to `0x80003800`. If another mod (such as *Super Luigi Land Wii*) uses this same memory space for custom code blocks or `loader.bin`, **the game will crash immediately**. 

> [!TIP]
> **How to fix memory overlaps:** You can resolve this using **ONE** of the following methods:
> * Relocating the conflicting mod's payload to a different free memory region.
> * Recompiling our patch with a new base address.
> * Merging our source code directly into your project and compiling everything together into a single `loader.bin`.

---

## Known Issues & Limitations
Due to the technical complexities of the NSMBW engine, the following issues currently exist (fixes may come in the future, but are limited by engine constraints):
1. Up-thrown bombs and springs cannot hit or interact with blocks. Up-thrown POWs and springs do not interact with the player mid-air.
2. Up-thrown shells sometimes clip through blocks or fail to trigger block interactions correctly.
3. Spin-jump bouncing may fail to trigger on certain enemies if the player is currently in damage I-frames (invincibility frames).
4. Certain custom enemies in the *NewerSMBW* engine may still damage the player during a spin jump.
5. Up-throwing an item still uses the vanilla forward-throw player animation. *(I am not an animator, so if anyone can make a custom upward-throwing animation, contributions are welcome!)*

---

## Note for Developers (Compilation & Source Code)

> [!IMPORTANT]
> The source code provided in this repository is currently intended as a **logic reference** rather than a plug-and-play build environment. Due to the iterative nature of this reverse-engineering process:
> * **Mixed Region Hardcoding:** Some `.cpp` and `.S` files contain hardcoded absolute addresses for the PAL version, while others are for the NTSC version. 
> * **Custom Kamek Syntax:** Certain files were compiled using a customized fork of the Kamek compiler, meaning out-of-the-box compilation with the vanilla Kamek toolchain may result in syntax errors or linker failures.
>
> **If you want to use this code in your own project:** You are highly encouraged to read the core logic and adapt/port the hooks to your own project's symbol maps.

---

## Future Plans
I will gradually look into the "Known Issues" listed above. However, due to engine complexities and my limited time, I will prioritize the "low-hanging fruit". Moreover, I plan to explore a few more movement mechanics in the future:

* **Enhanced Sliding Mechanics:** Modifying the slide so it can kill most spiky enemies, bringing it closer to the logic found in *Super Mario Maker* (SMM) and *Super Mario World* (SMW).
* **Wall-Jump Spin:** Enabling Mario to perform a spin jump directly off walls, adapting the fluid movement style from *New Super Mario Bros. U* (NSMBU).

> [!NOTE]
> **A quick note on updates:** My free time is fairly limited. Therefore, progress on these new features and fixes will be very slow. I appreciate your patience and understanding!

---

## Credits & Acknowledgements
* **Nintendo:** For creating *New Super Mario Bros. Wii*.
* **Newer Team:** For the *NewerSMBW* engine and foundational modding tools.
* **Luke, RedStoneMatt & zement:** For the original upward-throwing shell logic and publishing the foundational NewerGEM codebase.
* **RoadrunnerWMC, RootCubed & CHN Symbol Map Contributors:** For the invaluable symbol map resources, which greatly aided in cross-region memory offset tracking and reverse engineering.
* **lele:** My friend and veteran SMM Kaizo player, for providing deep insights into advanced gameplay mechanics and inspiring the new movement features.
* **2Maliao:** For code collaboration, sharing modding assets, and continuous technical discussion.
