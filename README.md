# vircon32-libretro

This is a libretro core to emulate Vircon32 game console. This is not a stand-alone program! You will need to use some kind of libretro front-end, such as RetroArch. Then, by installing this core, you will be able to play Vircon32 games on it.

If you didn't know about Vircon32, you can learn about it on its website: [vircon32.com](https://www.vircon32.com).

-----------------
### Core features

- Game compatibility should be 100%.
- The core embeds the Standard Vircon32 BIOS v1.2. Thereis no need to download it separately.
- Alternative BIOSes are also supported. For this, place your BIOS rom file in RetroArch's system directory under the name Vircon32Bios.v32.
- There is a core option to enable automatic frameskip. Use this to reduce slowdown if needed. However it can cause some stutter or small inaccuracies so it is recommended to leave it off (this is the default).
- The core supports savestates and rewinding.
- Netplay might be possible too, though this is untested.

Savestates in this core lack one feature: they don't save the screen contents. This is done on purpose: saving and redrawing the screen would add significant size and complexity. However, since almost all games will redraw the screen every frame, this limitation should not affect players in practice.

--------------------------------
### Requirements to run the core

This core currently requires devices to support OpenGL 3.0 or greater to work. It can also work with OpenGL ES 2.0 and 3.0. In terms of performance, some of the more elaborate games can get a bit demanding. For reference, in theory a Raspberry Pi 4 should be able to run any game.

The Vircon32 core is currently supported in at least these libretro systems:

- [AmberELEC](https://amberelec.org)
- [EmuELEC](https://emuelec.org)
- [EmuVR](https://www.emuvr.net)
- [Lakka](https://www.lakka.tv)
- [RetroArch](https://www.retroarch.com)

---------------------------------
### How to build from source code

This core is built using CMake, so the typical sequence is to build the library in a subfolder like this:

```
cd build
cmake ..
make
```

If you want to cross build for Android, you will need to have the NDK installed, and provide CMake with some more variables. The Android build will automatically use OpenGL ES 3. The CMake command will look like this:

```
cmake -DANDROID_ABI=arm64-v8a -DANDROID_NDK=<your_ndk_folder> -DCMAKE_TOOLCHAIN_FILE=<your_ndk_folder>/build/cmake/android.toolchain.cmake ..
```

Under Windows, if you build with MinGW toolchain you will have to modify the CMake command to this:

```
cmake -G 'MSYS Makefiles' ..
```

After the build succeeds, to use the core in RetroArch you will have to either copy the resulting shared library into your cores directory. You can do that manually or install to the default directory using this command. Note that in Linux/Mac systems you may have to use sudo.

```
make install
```

In [EmuELEC](https://github.com/EmuELEC/EmuELEC) you must build `libretro-vircon32` inside their environment by cloning its repository:

```
git clone -b master https://github.com/EmuELEC/EmuELEC.git
cd EmuELEC
```

Copy and paste the file [package.mk](emuelec/package.mk) included here to path `packages/emulation/libretro-vircon32/package.mk` inside EmuELEC repository `packages/emulation/libretro-vircon32` and run:

```
PROJECT=<your_platform> DEVICE=<your_device> ARCH=aarch64 DISTRO=EmuELEC ./scripts/build libretro-vircon32
```

For example:

```
PROJECT=Amlogic-ce DEVICE=Amlogic-ng ARCH=aarch64 DISTRO=EmuELEC ./scripts/build libretro-vircon32
```

Then copy the `vircon32_libretro.so` file into emuELEC `core` folder, and its BIOS into `bios` folder, another option is to build your own EmuELEC image.

--------------------------------------
### Targeting other versions of OpenGL

This core uses OpenGL for graphics. The default build command shown above will make the core use OpenGL Core profile. But in some other systems you may need to use OpenGL ES 2 or 3. To target those, use these variables when running CMake:

For OpenGL ES 2: cmake -DENABLE_OPENGLES2=1 ..
For OpenGL ES 3: cmake -DENABLE_OPENGLES3=1 ..

Note that on the Raspberry Pi 4, while the core will build fine without these flags, it still won't run correctly unless the GLES3 flag is used.
