# NSMBW Advanced Physics & Movement Patch

An advanced physics and movement patch for *New Super Mario Bros. Wii* (NSMBW). This project was created to enrich the vanilla physics engine with new mechanics, expanding creative possibilities for level designers (especially for **Kaizo hacks**) and enhancing overall gameplay. 

This patch can be used seamlessly with vanilla NSMBW, *NewerSMBW*, or other custom hacks via a simple Riivolution XML.

## ✨ Features

### 1. SMW-Style Spin Jump
* **Variable Height Control:** Spin jump height is now dynamic based on how long you hold the jump button.
* **Delayed Descent:** Holding the jump button during a spin jump reduces fall speed (hovering effect, just like in *Super Mario World*). Releasing the button immediately cancels the delay, returning you to normal fall speed.
* **Spike Immunity:** You can now safely spin-bounce on Thwomps, Spikeballs, and other spiky enemies without taking damage!

### 2. Yoshi Enhanced
* **Spiked Enemy Bouncing:** Just like in SMW, Yoshi can now safely bounce on spiky enemies instead of taking damage.
* **Cooligan Fix:** Fixed a vanilla bug where Yoshi would take damage from Cooligans. Yoshi can now crush them safely.
* **Terrain Immunity:** Yoshi is no longer afraid of spiked floors or Munchers (black piranha plants).

### 3. Dynamic Climbing Jumps
* Optimized the jump height when detaching from **climbable terrain** (such as climbable ceilings and walls). Instead of the vanilla fixed 2-block height, it now features a **dynamic 2-4 block height** controlled entirely by the player's button input.

### 4. Up-Throw & Instant Drop Mechanics
* **Item Up-Throw:** Players can now throw items directly upwards! Supported items include: Shells, Bombs, Glow Blocks, Propeller Blocks, POW Blocks, and Springs.
* **Instant Drop:** Pressing down allows players to instantly drop Shells and Bombs at their feet.
* **Momentum Inheritance:** Up-thrown and dropped items fully inherit the player's X-axis velocity for realistic physics trajectories.

---

## 🐛 Known Issues & Limitations
Due to the technical complexities of the NSMBW engine, the following issues currently exist (fixes may come in the future, but are limited by engine constraints):
1. Up-thrown bombs do not trigger block interactions.
2. Up-thrown bombs and springs cannot hit or interact with blocks. Up-thrown POWs and springs do not interact with the player mid-air.
3. Up-thrown shells sometimes clip through blocks or fail to trigger block interactions correctly.
4. Spin-jump bouncing may fail to trigger on certain enemies if the player is currently in damage I-frames (invincibility frames).
5. Certain custom enemies in the *NewerSMBW* engine may still damage the player during a spin jump.
6. **[Help Wanted!]** Up-throwing an item still uses the vanilla forward-throw player animation. *(I am not an animator, so if anyone can make a custom upward-throwing animation, contributions are welcome!)*

---

## ⚙️ Compatibility & Usage Notes (For Releases)

The `Release` tab contains ready-to-use XML patches. 
* **Supported Regions:** `NTSC-U v1.00` (USA version 1) and `PAL v2.00` (Europe version 2).
* **Riivolution Ready:** Simply place the XML in your Riivolution folder.

**⚠️ Important Compatibility Warnings:**
This patch can be mixed with many other XML mods, but **WILL CONFLICT** under the following conditions:
1. **Hook Conflicts:** If another mod intercepts the vanilla damage calculation function using the same address (e.g., `mkwcat`'s dynamic `.rel` injection for death/damage messages), features in this patch will fail or cause the game to crash. You will need to manually recompile conflicting code to make them compatible.
2. **Memory Overlap (Code Cave):** This patch heavily utilizes the free memory area from `0x80001800` to `0x80003800`. If another mod (such as *Super Luigi Land Wii*) uses this same memory space for custom code blocks or `loader.bin`, **the game will crash immediately**. 

---

## 🛠️ Note for Developers (Compilation & Source Code)

The source code provided in this repository is currently intended as a **logic reference** rather than a plug-and-play build environment. Due to the iterative nature of this reverse-engineering process:
* **Mixed Region Hardcoding:** Some `.cpp` and `.S` files contain hardcoded absolute addresses for the PAL version, while others are for the NTSC version. 
* **Custom Kamek Syntax:** Certain files were compiled using a customized fork of the Kamek compiler, meaning out-of-the-box compilation with the vanilla Kamek toolchain may result in syntax errors or linker failures.

**If you want to use this code in your own project:** You are highly encouraged to read the core logic (momentum inheritance, Yoshi collision, etc.) and adapt/port the hooks to your own project's symbol maps.
