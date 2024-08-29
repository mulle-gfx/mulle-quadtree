# mulle-quadtree

#### 🌴 mulle-quadtree for tracking rectangles


# 🌴 mulle-quadtree

#### 🌴 mulle-quadtree for tracking rectangles

This can be used to enable "mouse over" functionality aka "tracking rectangles".

MulleQuadtree is very loosely based on the MIT Licensed:
Kvad - a C99 quadtree implementation
[//github.com/appscape/kvad](//github.com/appscape/kvad), so this is licensed
MIT as well.





| Release Version                                       | Release Notes
|-------------------------------------------------------|--------------
| ![Mulle kybernetiK tag](https://img.shields.io/github/tag/mulle-gfx/mulle-quadtree.svg?branch=release) [![Build Status](https://github.com/mulle-gfx/mulle-quadtree/workflows/CI/badge.svg?branch=release)](//github.com/mulle-gfx/mulle-quadtree/actions) | [RELEASENOTES](RELEASENOTES.md) |






### You are here

![Overview](overview.dot.svg)





## Add

**This project is a component of the [mulle-core](//github.com/mulle-core/mulle-core) library. As such you usually will *not* add or install it
individually, unless you specifically do not want to link against
`mulle-core`.**


### Add as an individual component

Use [mulle-sde](//github.com/mulle-sde) to add mulle-quadtree to your project:

``` sh
mulle-sde add github:mulle-gfx/mulle-quadtree
```

To only add the sources of mulle-quadtree with dependency
sources use [clib](https://github.com/clibs/clib):


``` sh
clib install --out src/mulle-gfx mulle-gfx/mulle-quadtree
```

Add `-isystem src/mulle-gfx` to your `CFLAGS` and compile all the sources that were downloaded with your project.


## Install

### Install with mulle-sde

Use [mulle-sde](//github.com/mulle-sde) to build and install mulle-quadtree and all dependencies:

``` sh
mulle-sde install --prefix /usr/local \
   https://github.com/mulle-gfx/mulle-quadtree/archive/latest.tar.gz
```

### Manual Installation

Install the requirements:

| Requirements                                 | Description
|----------------------------------------------|-----------------------
| [mulle-geometry](https://github.com/MulleUI/mulle-geometry)             | 

Install **mulle-quadtree** into `/usr/local` with [cmake](https://cmake.org):

``` sh
cmake -B build \
      -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_PREFIX_PATH=/usr/local \
      -DCMAKE_BUILD_TYPE=Release &&
cmake --build build --config Release &&
cmake --install build --config Release
```


## Author

[Nat!](https://mulle-kybernetik.com/weblog) for Mulle kybernetiK  



