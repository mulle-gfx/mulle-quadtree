# mulle-quadtree

#### 🌴 mulle-quadtree for tracking rectangles

This library is used to enable "mouse over" functionality aka
"tracking rectangles". You feed it a set of rectangles. After that you can
do a hit test to find the top most rectangle, that's at the given location.

MulleQuadtree is very loosely based on the MIT Licensed:
Kvad - a C99 quadtree implementation
[//github.com/appscape/kvad](//github.com/appscape/kvad), so this is licensed
MIT as well.





| Release Version                                       | Release Notes  | AI Documentation
|-------------------------------------------------------|----------------|---------------
| ![Mulle kybernetiK tag](https://img.shields.io/github/tag/mulle-gfx/mulle-quadtree.svg) [![Build Status](https://github.com/mulle-gfx/mulle-quadtree/workflows/CI/badge.svg)](//github.com/mulle-gfx/mulle-quadtree/actions) | [RELEASENOTES](RELEASENOTES.md) | [DeepWiki for mulle-quadtree](https://deepwiki.com/mulle-gfx/mulle-quadtree)






### You are here

![Overview](overview.dot.svg)





## Quickstart


``` sh
mulle-sde init -d my-project -m mulle-gfx/c-developer executable
cd my-project
mulle-sde add github:mulle-gfx/mulle-quadtree
mulle-sde vibecoding on
mulle-sde run
```

You are done, skip the following "Add" step.


## Add

Use [mulle-sde](//github.com/mulle-sde) to add mulle-quadtree to your project.
As long as your sources are using `#include "include-private.h"` and your headers use `#include "include.h"`, there will nothing more to do:

``` sh
mulle-sde add github:mulle-gfx/mulle-quadtree
```

To only add the sources of mulle-quadtree with all the sources of its
dependencies replace "github:" with [clib:](https://github.com/clibs/clib):

## Legacy adds

One common denominator is that you will likely have to add
`#include <mulle-quadtree/mulle-quadtree.h>` to your source files.


### Add sources to your project with clib

``` sh
clib install --out src/mulle-gfx mulle-gfx/mulle-quadtree
```

Add `-isystem src/mulle-gfx` to your `CFLAGS` and compile all the
sources that were downloaded with your project. (In **cmake** add
`include_directories( BEFORE SYSTEM src/mulle-gfx)` to your `CMakeLists.txt`
file).







### Add as subproject with cmake and git

``` bash
git submodule add -f --name "mulle-core" \
                            "https://github.com/mulle-core/mulle-core.git" \
                            "stash/mulle-core"
git submodule add -f --name "mulle-geometry" \
                            "https://github.com/MulleUI/mulle-geometry.git" \
                            "stash/mulle-geometry"
git submodule add -f --name "mulle-quadtree" \
                            "https://github.com/mulle-gfx/mulle-quadtree" \
                            "stash/mulle-quadtree"
git submodule update --init --recursive
```

``` cmake
add_subdirectory( stash/mulle-quadtree)
add_subdirectory( stash/mulle-geometry)
add_subdirectory( stash/mulle-core)

target_link_libraries( ${PROJECT_NAME} PUBLIC mulle-quadtree)
target_link_libraries( ${PROJECT_NAME} PUBLIC mulle-geometry)
target_link_libraries( ${PROJECT_NAME} PUBLIC mulle-core)
```


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
| [mulle-geometry](https://github.com/MulleUI/mulle-geometry)             | 🔭 Basic geometric types

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



