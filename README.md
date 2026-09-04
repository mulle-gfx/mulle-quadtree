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




## Documentation & Guides

* [API Summary](asset/dox/api/toc)

## Usage

Feed rectangles with payloads into the tree, then hit test a point. This is
how "mouse over" (tracking rectangles) is implemented:

``` c
#include <mulle-quadtree/mulle-quadtree.h>

static void  hit( CGRect rect, void *payload, void *context)
{
   (void) rect;
   (void) context;
   printf( "mouse over: %s\n", (char *) payload);
}

int  main( void)
{
   struct mulle_quadtree  *tree =
      mulle_quadtree_create( CGRectMake( 0, 0, 800, 600), 3, 100, NULL);

   mulle_quadtree_insert( tree, CGRectMake(  10,  10, 200, 100), "left window");
   mulle_quadtree_insert( tree, CGRectMake( 500, 300, 200, 200), "right window");

   mulle_quadtree_find_point( tree, CGPointMake( 150, 50), hit, NULL);

   mulle_quadtree_destroy( tree);
   return( 0);
}
```


### You are here

![Overview](overview.dot.svg)





## Add

**This project is a component of the [mulle-gfx](//github.com/mulle-gfx/mulle-gfx) library.
As such you usually will *not* add or install it individually, unless you
specifically do not want to link against `mulle-gfx`.**


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



