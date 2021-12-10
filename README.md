# 🌴 mulle-quadtree

#### 🌴 mulle-quadtree for tracking rectangles

This can be used to enable "mouse over" functionality aka "tracking rectangles".

MulleQuadtree is very loosely based on the MIT Licensed:
Kvad - a C99 quadtree implementation
[//github.com/appscape/kvad](//github.com/appscape/kvad), so this is licensed
MIT as well.


## Add

Use [mulle-sde](//github.com/mulle-sde) to add mulle-quadtree to your project:

``` console
mulle-sde dependency add --c --github MulleUI mulle-quadtree
```

## Install

### mulle-sde

Use [mulle-sde](//github.com/mulle-sde) to build and install mulle-quadtree
and all its dependencies:

```
mulle-sde install --prefix /usr/local \
   https://github.com/MulleUI/mulle-quadtree/archive/latest.tar.gz
```

### Manual Installation


Install the requirements:

Requirements                                      | Description
--------------------------------------------------|-----------------------
[some-requirement](//github.com/some/requirement) | Some requirement

Install into `/usr/local`:

```
mkdir build 2> /dev/null
(
   cd build ;
   cmake -DCMAKE_INSTALL_PREFIX=/usr/local \
         -DCMAKE_PREFIX_PATH=/usr/local \
         -DCMAKE_BUILD_TYPE=Release .. ;
   make install
)
```

### Steal

Read [STEAL.md](//github.com/mulle-c11/dox/STEAL.md) on how to steal the
source code and incorporate it in your own projects.
