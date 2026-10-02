### 1.1.1

Various small improvements

## 1.1.0



feature: add lifecycle API and payload removal, guarantee z-order

* new `mulle_quadtree_remove_by_payload()` removes all fragments of a payload without needing the original rect
* new `mulle_quadtree_alloc/init/done/free` lifecycle functions for finer control over tree lifetime
* find/walk operations now guarantee z-order: most recently inserted objects are reported first, so the first callback is the frontmost object at a point
* inserted rects are standardized and empty rects are rejected on insert
* node splits are suppressed when all entries would land in every quadrant, avoiding a 4x memory blowup for large overlapping rects
