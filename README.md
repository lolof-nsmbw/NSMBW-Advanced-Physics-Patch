<div align="center">
  
# NSMBW Advanced Physics Patch

![Game](https://img.shields.io/badge/Game-NSMBW-red)
![Compiler](https://img.shields.io/badge/Compiler-CodeWarrior-yellow)
![Toolchain](https://img.shields.io/badge/Toolchain-Kamek_2-lightgrey)
[![License](https://img.shields.io/badge/License-MIT-green)](LICENSE)

</div>

An advanced physics and movement patch for *New Super Mario Bros. Wii* (NSMBW). This project was created to enrich the vanilla physics engine with new mechanics, expanding creative possibilities for level designers  and enhancing overall gameplay. **I will continue to optimize the codebase and introduce new movement mechanics in the future.**

This patch has been entirely refactored using the modern **Kamek 2** toolchain with propel parts, making it universally compatible across all regional game versions. It can be used seamlessly with vanilla NSMBW, *NewerSMBW*, or other custom hacks via a simple Riivolution XML. Developers can easily clone this repository and use the provided `externals.txt` mapping for custom Kamek 2 builds. The `Release` tab contains ready-to-use compiled packages.

Feel free to use this patch in your custom levels, Kaizo hacks, or showcase it in your videos! If you do, I would greatly appreciate it if you could credit me or link back to this repository.

---

> [!IMPORTANT]
> **Disclaimer & Feedback:**
> This patch is currently being developed and playtested almost entirely by me as a solo developer. While I strive for stability, some bugs may inevitably slip through. 
> If you encounter any bugs or crashes, I would highly appreciate your feedback via GitHub Issues or [Discord](https://discordapp.com/users/1338165547753275457)! **However, before reporting a crash, please ensure it is NOT caused by mod memory conflicts**.

---

## Table of Contents
- [Features](#features)
- [Compatibility & Usage Notes](#compatibility--usage-notes)
- [Known Issues & Limitations](#known-issues--limitations)
- [About & Credits](#about--credits)

---

## Features

<details>
<summary><b>1. SMW-Style Spin Jump (Click to expand details)</b></summary>

* **Spike Immunity:** You can now safely spin-bounce on spiky enemies and objects without taking damage! 
* **Variable Height Control:** Spin jump height is now dynamic based on how long you hold the jump button.
* **Delayed Descent:** Holding the jump button during a spin jump reduces fall speed (hovering effect, just like in *Super Mario World*). Releasing the button immediately cancels the delay, returning you to normal fall speed.
* **NewerSMBW Compatibility:** Spin jumping is fully supported on Newer custom enemies, allowing you to safely bounce off entities like the Newer Topman and other bespoke bosses.

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
<summary><b>3. Slide Enhanced Module (Click to expand details)</b></summary>

* **Expanded Sliding Mechanics:** Players can now safely defeat a wider variety of enemies—such as Piranha Plants, Spinies, and even Icicles—simply by sliding into them without taking damage.
* **Safe Deflection System:** When sliding into massive or metallic spiked hazards (e.g., Spiked Balls or Big Piranha Plants), players will no longer take damage. Instead, the character will safely bounce off the enemy, opening up new creative routing possibilities.
</details>

<details>
<summary><b>4. Dynamic Climbing Jumps (Click to expand details)</b></summary>

* Optimized the jump height when detaching from **climbable terrain**. Instead of the vanilla fixed 2-block height, it now features a **dynamic 2-4 block height** controlled entirely by the player's button input.
</details>

<details>
<summary><b>5. Up-Throw & Instant Drop Mechanics (Click to expand details)</b></summary>

* **Item Up-Throw:** Players can now throw items directly upwards! Supported items include: Shells, Bombs, Glow Blocks, Propeller Blocks, POW Blocks, and Springs.
* **Instant Drop:** Pressing down allows players to instantly drop Shells and Bombs at your feet.
* **Momentum Inheritance:** Up-thrown and dropped items inherit the player's X-axis velocity in a certain proportion for realistic physics trajectories.
</details>

---

## Compatibility & Usage Notes

**This patch is fully compatible with vanilla NSMBW, NewerSMBW, and NSMBWer+.** Specific compatibility details and usage instructions are as follows:
* **Universal Support:** Thanks to the Kamek 2 refactor, this patch now supports **all regional game versions**.
* **Loader Optimization:** The `loader.bin` payload has been optimized to allow seamless simultaneous loading with other custom mods built on the **NewerSMBW**.
* **NSMBWer+ Support:** Fully compatible. However, the vanilla loaders are natively incompatible with each other out-of-the-box. You must modify the NSMBWer+ loader to make it compatible with this patch.
* **Riivolution Ready:** Simply drop the contents into your SD card, which will load the custom `loader.bin` through Riivolution. Ensure `externals.txt` is present if you are building from source.

> [!WARNING]
> **Important Compatibility Warnings:** 
> If another mod intercepts the exact same vanilla functions (e.g., modifying the original damage calculation address), the features will conflict.

---

## Known Issues & Limitations
Due to the technical complexities of the NSMBW engine, the following issues currently exist (fixes may come in the future, but are limited by engine constraints):

1. Up-thrown bombs and springs cannot hit or interact with blocks. Up-thrown POWs and springs do not interact with the player mid-air.
2. Up-thrown shells sometimes clip through blocks or fail to trigger block interactions correctly.
3. Spin-jump bouncing may fail to trigger on certain enemies if the player is currently in damage I-frames (invincibility frames).
4. Yoshi attempting to bounce on the Spike Balls (Stone Balls) thrown by Spikes fails to trigger the bounce mechanic for unknown reasons.
5. Up-throwing an item still uses the vanilla forward-throw player animation. *(I am not an animator, so if anyone can make a custom upward-throwing animation, contributions are welcome!)*

---

## About & Credits

**Lead Developer:** [Arisu Tendou](https://www.youtube.com/@ArisuTendou-l3l)

**Special Thanks:**
* **Nintendo:** For creating *New Super Mario Bros. Wii*.
* **Newer Team:** For the *NewerSMBW* and foundational modding tools.
* **Luke, RedStoneMatt & zement:** For the original upward-throwing shell logic and publishing the foundational NewerGEM codebase.
* **RoadrunnerWMC, RootCubed & CHN Symbol Map Contributors:** For the invaluable symbol map resources.
* **Lele:** My friend and veteran SMM Kaizo player, for providing deep insights into advanced mechanics.
* **2Maliao:** For code collaboration, sharing modding assets, and continuous technical discussion.
