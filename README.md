# vircon32-libretro

This is a libretro core for Vircon32. It will allow you to play Vircon32 games using some kind of libretro front-end, such as RetroArch. This is not a stand-alone program! You will also need to place the Vircon32 BIOS file in RetroArch's system directory.

----------------------------------
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

For OpenGL ES 2: cmake -D ENABLE_OPENGLES2=1 ..
For OpenGL ES 3: cmake -D ENABLE_OPENGLES3=1 ..

--------------------------------
### Requirements to run the core

This core will need the standard Vircon32 BIOS file (StandardBios.v32).
You can download it here: https://github.com/vircon32/ConsoleSoftware/releases/tag/bios-v1.1

The BIOS file needs to be placed in RetroArch's system directory.
The typical directories for this are:

- On Windows (64 bit):  C:/Program Files/RetroArch/system
- On Windows (32 bit):  C:/Program Files(x86)/RetroArch/system
- On Linux:  /home/${USER}/.config/retroarch/system
- On MacOS:  /Users/${USER}/Library/Application Support/RetroArch/system

