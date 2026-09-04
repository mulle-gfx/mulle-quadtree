### 1.1.0

* new `mulle_quadtree_remove_by_payload()` removes all fragments of a payload
  without needing the original rect
* new `mulle_quadtree_alloc`/`init`/`done`/`free` lifecycle functions for
  finer control over tree lifetime
* find/walk operations now guarantee z-order, most recently inserted objects
  are reported first
* inserted rects are standardized and empty rects are rejected on insert
* node splits are suppressed when all entries would land in every quadrant,
  avoiding a 4x memory blowup for large overlapping rects

### 1.0.1

Various small improvements
