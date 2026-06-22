# NSMBW Advanced Physics & Movement Patch

![Game](https://img.shields.io/badge/Game-NSMBW-red)
![Versions](https://img.shields.io/badge/Versions-NTSC--U%20Rev1%20%7C%20PAL%20Rev2-ff69b4)
![Compiler](https://img.shields.io/badge/Compiler-CodeWarrior-yellow)
![Toolchain](https://img.shields.io/badge/Toolchain-Kamek-lightgrey)
![License](https://img.shields.io/badge/License-MIT-green)

An advanced physics and movement patch for *New Super Mario Bros. Wii* (NSMBW). This project was created to enrich the vanilla physics engine with new mechanics, expanding creative possibilities for level designers (especially for **Kaizo hacks**) and enhancing overall gameplay. Feel free to use this patch in your custom levels, Kaizo hacks, or showcase it in your videos! If you do, I would greatly appreciate it if you could credit me or link back to this repository.

This patch can be used seamlessly with NSMBW, *NewerSMBW*, or other custom hacks via a simple Riivolution XML.

---

> [!IMPORTANT]
> **Disclaimer & Feedback:**
> This patch is currently being developed and playtested almost entirely by me as a solo developer. While I strive for stability, some bugs may inevitably slip through. 
> If you encounter any bugs or crashes, I would highly appreciate your feedback via GitHub Issues or [Discord](https://discordapp.com/users/1338165547753275457)! **However, before reporting a crash, please ensure it is NOT caused by mod memory conflicts** (Please read the [Compatibility & Conflicts](#compatibility--usage-notes) section carefully to save us both time).

---

## Table of Contents
- [Features](#features)
- [Compatibility & Usage Notes](#compatibility--usage-notes)
- [Known Issues & Limitations](#known-issues--limitations)
- [Future Plans](#future-plans)
- [For Developers](#for-developers)
- [Credits](#credits)

---

## Features

<details>
<summary><b>1. SMW-Style Spin Jump (Click to expand details)</b></summary>

* **Spike Immunity:** You can now safely spin-bounce on spiky enemies and objects without taking damage! 
* **Variable Height Control:** Spin jump height is now dynamic based on how long you hold the jump button.
* **Delayed Descent:** Holding the jump button during a spin jump reduces fall speed (hovering effect, just like in *Super Mario World*). Releasing the button immediately cancels the delay, returning you to normal fall speed.

> [!TIP]
> **💡 The Golden Rule of Spin Jumping:** This mechanic is strictly designed for physical and **Spike-type** hazards. It will **NOT** protect you from ethereal or energy-based hazards like Ghosts (Boos), Fire, or Electric enemies (Amps). 
> *Kaizo Exception: The **Fire Snake** is the ONE and ONLY exception to this rule and is fully bounceable! (A deliberate design choice for better Kaizo level flow).*

**🟢 Supported Spin-Bounce Whitelist**
*(Mario's Spin Jump now covers EVERYTHING Yoshi can safely bounce on, plus classic spin-jump targets. Including but not limited to:)*
* **Spiky & Prickly Foes:** Spinies, Every kind of Piranha Plants, Prickly Goombas, Fuzzies (including Line Controlled), Bramballs, Urchins & Mega Urchins
* **Heavy & Metallic Hazards:** Thwomps, Big Thwomps, Chain Chomps, Spiked Balls, Giant Spiked Balls, Spiked Balls on a Chain
* **Aquatic Foes (Surface-level):** Fishbones, Bloopers, Cheep Cheeps, Spiny Cheep Cheeps, Porcupuffers
* **Bosses:** All 7 Koopaling Shells (Larry, Wendy, Iggy, Lemmy, Morton, Roy, Ludwig)
* **The Kaizo Exception:** Fire Snakes
</details>

<details>
<summary><b>2. Yoshi Enhanced (Click to expand details)</b></summary>

* **Spiked Enemy Bouncing:** Yoshi's vanilla bouncing capabilities have been massively expanded. Just like in SMW, Yoshi can now safely bounce on heavily spiked and previously un-bounceable enemies!
* **Cooligan Fix:** Fixed a vanilla bug where Yoshi would take damage from Cooligans. Yoshi can now crush them safely.
* **Terrain Immunity:** Yoshi is no longer afraid of spiked floors or Munchers.

**🟢 Yoshi Enhanced Bouncing Whitelist**
*(Enemies added to Yoshi's safe-bounce logic in this patch)*
* **Spiky & Prickly Foes:** Prickly Goombas, Fuzzies (Line Controlled), Bramballs, Urchins & Mega Urchins
* **Heavy & Metallic Hazards:** Thwomps, Big Thwomps, Spiked Balls, Giant Spiked Balls, Spiked Balls on a Chain
* **Aquatic Foes:** Fishbones, Bloopers, Cheep Cheeps, Spiny Cheep Cheeps, Porcupuffers
* **Bosses:** All 7 Koopaling Shells (Larry, Wendy, Iggy, Lemmy, Morton, Roy, Ludwig)
* **The Kaizo Exception:** Fire Snakes
</details>

<details>
<summary><b>3. Dynamic Climbing Jumps (Click to expand details)</b></summary>

* Optimized the jump height when detaching from **climbable terrain**. Instead of the vanilla fixed 2-block height, it now features a **dynamic 2-4 block height** controlled entirely by the player's button input.
</details>

<details>
<summary><b>4. Up-Throw & Instant Drop Mechanics (Click to expand details)</b></summary>

* **Item Up-Throw:** Players can now throw items directly upwards! Supported items include: Shells, Bombs, Glow Blocks, Propeller Blocks, POW Blocks, and Springs.
* **Instant Drop:** Pressing down allows players to instantly drop Shells and Bombs at your feet.
* **Momentum Inheritance:** Up-thrown and dropped items inherit the player's X-axis velocity in a certain proportion for realistic physics trajectories.
</details>

---

## Compatibility & Usage Notes

The `Release` tab contains ready-to-use XML patches. 
* **Supported Versions:** NTSC Rev 1 (`SMNE01`) and PAL Rev 2 (`SMNP01`).
* **Riivolution Ready:** Simply drop the `.xml` file into your SD card's `riivolution` folder, and place the `.bin` patch files in the designated directory.

<details>
<summary><b>⚠️ WARNING: Mod Conflicts & Memory Overlaps (Click to expand)</b></summary>

This patch can be mixed with many other XML mods, but **WILL CONFLICT** under the following conditions:
 
1. **Hook Conflicts:** If another mod intercepts the vanilla damage calculation function using the same address (e.g., `mkwcat`'s dynamic `.rel` injection for death/damage messages), features in this patch will fail or cause the game to crash. You will need to manually recompile conflicting code to make them compatible.
2. **Memory Overlap:** This patch heavily utilizes the free memory area from `0x80001800` to `0x80003800`. If another mod (such as *Super Luigi Land Wii*) uses this same memory space for custom code blocks or `loader.bin`, **the game will crash immediately**. 

**How to fix memory overlaps (Choose ONE):**
* Relocating the conflicting mod's payload to a different free memory region.
* Recompiling our patch with a new base address.
* Merging our source code directly into your project and compiling everything together into a single `loader.bin`.
</details>

---

## Known Issues & Limitations
Due to the technical complexities of the NSMBW engine, the following issues currently exist:

1. Up-thrown bombs and springs cannot hit or interact with blocks. 
2. Up-thrown POWs and springs do not interact with the player mid-air.
3. Up-thrown shells sometimes clip through blocks or fail to trigger block interactions correctly.
4. Spin-jump bouncing may fail to trigger on certain enemies if the player is currently in damage I-frames (invincibility frames).
5. Certain custom enemies in the *NewerSMBW* engine may still damage the player during a spin jump.
6. Yoshi attempting to bounce on the Spike Balls (Stone Balls) thrown by Spikes fails to trigger the bounce mechanic for unknown reasons.
7. Up-throwing an item still uses the vanilla forward-throw player animation. *(I am not an animator, so if anyone can make a custom upward-throwing animation, contributions are welcome!)*

---

## Future Plans
I will gradually look into the "Known Issues" listed above, prioritizing the "low-hanging fruit". Moreover, I plan to explore a few more movement mechanics:

* **Enhanced Sliding Mechanics:** Modifying the slide so it can kill most spiky enemies, bringing it closer to the logic found in *SMM* and *SMW*.
* **Wall-Jump Spin:** Enabling Mario to perform a spin jump directly off walls, adapting the fluid movement style from *NSMBU*.

> [!NOTE]
> My free time is fairly limited, so progress on these new features and fixes will be slow. I appreciate your patience!

---

## For Developers

> [!IMPORTANT]
> The source code provided in this repository is currently intended as a **logic reference** rather than a plug-and-play build environment due to mixed region hardcoding and custom Kamek syntax. 

For detailed information on how to adapt, port, or recompile this codebase for your own projects, please refer to the source code and comments directly.

---

## About & Credits

**Lead Developer:** [Arisu Tendou](https://www.youtube.com/@ArisuTendou-l3l)

**Special Thanks:**
* **Nintendo:** For creating *New Super Mario Bros. Wii*.
* **Newer Team:** For the *NewerSMBW* engine and foundational modding tools.
* **Luke, RedStoneMatt & zement:** For the original upward-throwing shell logic and publishing the foundational NewerGEM codebase.
* **RoadrunnerWMC, RootCubed & CHN Symbol Map Contributors:** For the invaluable symbol map resources.
* **Lele:** My friend and veteran SMM Kaizo player, for providing deep insights into advanced mechanics.
* **2Maliao:** For code collaboration, sharing modding assets, and continuous technical discussion.
