# NSMBW Advanced Physics & Movement Patch

![Game](https://img.shields.io/badge/Game-NSMBW-red)
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

The `Release` tab contains ready-to-use compiled packages. 
* **Supported Versions:** Fully cross-region compatible! Supports all versions of NSMBW (NTSC-U, PAL, NTSC-J, NTSC-K, NTSC-W).
* **Riivolution Ready:** Simply drop the contents into your SD card, which will load the custom `loader.bin` through Riivolution.

> [!WARNING]
> **Important Compatibility Warnings:** 
> If another mod intercepts the exact same vanilla functions (e.g., modifying the original damage calculation address), the features will conflict.

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

This project has been massively refactored into a modern **Kamek 2** environment. 
* **100% C++ & Clean Macros:** Almost all messy absolute memory hardcoding has been eradicated. 
* **Symbol Map Integration:** Hooks dynamically adapt to different region offsets during compilation.
* **Plug and Play:** You can easily clone this repository, drop the source files into your Kamek 2.0 build environment. Feel free to explore, merge, and expand upon this codebase!

---

## Future Plans
I will gradually look into the "Known Issues" listed above. However, due to engine complexities and my limited time, I will prioritize the "low-hanging fruit". Moreover, I plan to explore a few more movement mechanics in the future:

* **Enhanced Sliding Mechanics:** Modifying the slide so it can kill most spiky enemies, bringing it closer to the logic found in *Super Mario Maker* (SMM) and *Super Mario World* (SMW).
* **Wall-Jump Spin:** Enabling Mario to perform a spin jump directly off walls, adapting the fluid movement style from *New Super Mario Bros. U* (NSMBU).

> [!NOTE]
> **A quick note on updates:** My free time is fairly limited. Therefore, progress on these new features and fixes will be very slow. I appreciate your patience and understanding!

---

## Credits
* **Nintendo:** For creating *New Super Mario Bros. Wii*.
* **Newer Team:** For the *NewerSMBW* engine and foundational modding tools.
* **Luke, RedStoneMatt & zement:** For the original upward-throwing shell logic and publishing the foundational NewerGEM codebase.
* **RoadrunnerWMC, RootCubed & CHN Symbol Map Contributors:** For the invaluable symbol map resources, which greatly aided in cross-region memory offset tracking and reverse engineering.
* **lele:** My friend and veteran SMM Kaizo player, for providing deep insights into advanced gameplay mechanics and inspiring the new movement features.
* **2Maliao:** For code collaboration, sharing modding assets, and continuous technical discussion.
