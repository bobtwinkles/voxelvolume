VoxelVolume
===========
Tool to render voxel volumes.
This code is still very much WIP.

Data Format
-----------

The main binary (`bin/main` when built) expects the name of a folder as its argument, with a file called "meta.txt" in it.
`meta.txt` should contain a single line of the form

    <width> <height> <depth> <basename>

For example, if the data set was the Stanford Volume Archive MRBrain dataset, `meta.txt` should contain:

    256 256 108 MRBrain

where width, height, and depth are integers and basename is a string which will be used to load the volume data.
Volume data should be 16 bit binary data, of size `width * height` in files named as follows:

    basename.1
    basename.2
    //....
    basename.n

where `basename` is the basename specifed in the metadata file, and n goes from 1 to the depth (inclusive) specified in `meta.txt`

Compiling
---------
You must have:
  - CMake
  - fontconfig (if you have X, you probably have this)
  - freetype2 (if you have X, you probably have this)
  - GLEW
  - GLM

if you want to build the basic renderer (which just continually scrolls through the dataset, not particularly interesting) you also need GLUT.
To build, create a directory somewhere outside of the source tree and then

    cmake path/to/source
    make

and to run

    cd bin/
    ./main path/to/data/folder
