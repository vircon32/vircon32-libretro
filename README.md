# vircon32-libretro

This is a libretro core for Vircon32. It will allow you to play Vircon32 games using some kind of libretro front-end, such as RetroArch. This is not a stand-alone program! You will also need to place the Vircon32 BIOS file in RetroArch's system directory.

----------------------------------
### How to build from source code

This core is build with CMake, so the typical sequence is to build the library in a subfolder like this:

```
cd build
cmake ..
make
```

After compiling, to install the shared library into your libretro frontend (using `"/home/${USER}/.config/retroarch/cores"` as default):

```
sudo make install
```

Under Windows, you will need this to build with MinGW toolchain:

cmake -G 'MSYS Makefiles' ..

--------------------------------------
### Targeting other versions of OpenGL

This core uses OpenGL for graphics. The default build command shown above will make the core use OpenGL Core profile. But in some other systems you may need to use OpenGL ES 2 or 3. To target those, use these variables when running CMake:

For OpenGL ES 2: cmake -D ENABLE_OPENGLES2=1 ..
For OpenGL ES 3: cmake -D ENABLE_OPENGLES3=1 ..
