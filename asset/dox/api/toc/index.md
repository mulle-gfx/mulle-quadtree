# mulle-quadtree Library Documentation for AI
<!-- Keywords: quadtree, spatial, rectangle, tracking, collision, geometry, search -->

## 1. Introduction & Purpose

**mulle-quadtree** is a spatial data structure library for efficient tracking and querying of rectangles in 2D space. It implements a quadtree algorithm optimized for "mouse over" functionality (tracking rectangles) and collision detection. The library divides 2D space hierarchically into four quadrants, enabling fast spatial queries when many rectangles need to be tested against a point or area. This library is loosely based on the MIT-licensed Kvad quadtree implementation and is part of the mulle-gfx ecosystem. It depends on **mulle-geometry** for geometric primitives (CGRect, CGPoint, CGSize).

## 2. Key Concepts & Design Philosophy

- **Hierarchical Space Partitioning**: The quadtree recursively subdivides 2D space into four quadrants (NW, NE, SW, SE) when capacity thresholds are exceeded.
- **Dynamic Splitting**: Leaf nodes split into four subnodes when they exceed `max_rects_per_node` capacity, up to `max_levels` depth.
- **Rectangle Intersection Storage**: Rectangles are clipped and stored in all quadrants they intersect, enabling efficient spatial queries.
- **Memory Recycling**: The library maintains internal free lists for nodes and data structures to minimize allocation overhead.
- **Non-Thread-Safe**: Operations are not thread-safe; external synchronization is required for concurrent access.
- **Payload Association**: Each rectangle stores an opaque `void*` payload pointer, allowing arbitrary data association without ownership semantics.
- **Query Callbacks**: Search operations use callback functions to process found rectangles, avoiding memory allocation for result sets.
- **Performance Trade-offs**: The quadtree excels when kept persistent across frames with multiple queries per frame. Single queries may be slower than brute-force iteration.

## 3. Core API & Data Structures

### 3.1. `mulle-quadtree.h`

#### `struct mulle_quadtree`
- **Purpose:** Opaque struct representing the entire quadtree data structure.
- **Key Fields:** Not exposed; managed internally.
- **Lifecycle Functions:**
  - `mulle_quadtree_create( rect, max_levels, max_points_per_node, allocator)`: Creates and initializes a quadtree with the specified bounding box and capacity parameters. Returns pointer to tree or NULL on failure. Use `max_levels=3` and `max_points_per_node=100` as reasonable defaults. Large `max_levels` with many overlapping rectangles can severely degrade insertion performance.
  - `mulle_quadtree_destroy( tree)`: Frees the quadtree and all associated memory, including recycling pools.
  - `mulle_quadtree_reset( tree, world_rect)`: Resets the tree with a new bounding rectangle (or keeps existing if `world_rect` is CGRectNull), recycling memory for reuse. This is more efficient than destroy/create for repeated use.
  
- **Core Operations:**
  - `mulle_quadtree_insert( tree, rect, payload)`: Inserts a rectangle and associated payload into the tree. Rectangles are clipped to the tree's bounding box and may be stored in multiple nodes. Returns 1 if inserted, 0 if the rectangle does not intersect the tree bounds. No duplicate checking is performed.
  - `mulle_quadtree_remove_payload( tree, rect, payload)`: Removes all rectangles matching both the payload pointer and intersecting the given rectangle. Returns count of rectangles removed. The rectangle parameter is used to accelerate lookup.
  - `mulle_quadtree_change_payload( tree, rect, payload, newpayload)`: Changes the payload of all rectangles matching the old payload and intersecting the given rectangle. Returns count of rectangles changed.

- **Query Functions:**
  - `mulle_quadtree_find_point( tree, point, callback, context)`: Searches for all rectangles containing the given point. Calls the callback for each match. Returns count of rectangles found.
  - `mulle_quadtree_find_intersecting_rect( tree, rect, callback, context)`: Searches for all rectangles intersecting the given rectangle. Calls the callback for each match. Returns count of rectangles found.
  - `mulle_quadtree_walk( tree, callback, context)`: Visits all rectangles in the tree, calling the callback for each. Returns total count of rectangles in tree.

- **Inspection Functions:**
  - `mulle_quadtree_count( tree)`: Returns the total number of rectangles stored in the tree (inline function that calls `mulle_quadtree_walk` with NULL callback).
  - `mulle_quadtree_dump( tree, fp)`: Dumps the tree structure to a file pointer for debugging purposes.

#### `typedef void (*mulle_quadtree_callback)( CGRect rect, void* payload, void* context)`
- **Purpose:** Callback function type for query operations.
- **Parameters:**
  - `rect`: The rectangle found (may be clipped to the node bounds).
  - `payload`: The user-provided payload pointer associated with the rectangle.
  - `context`: User-provided context pointer passed to the query function.

### 3.2. Internal Structures (Private API)

These structures are defined in `mulle-quadtree.c` and are not part of the public API:

#### `struct _mulle_quadtree_node`
- Represents a node in the quadtree hierarchy.
- Contains four subnode pointers (NW, NE, SW, SE), bounding rectangle, linked list of data, data count, and level.

#### `struct _mulle_quadtree_data`
- Represents a single rectangle/payload pair stored in a linked list within a leaf node.
- Contains rectangle, payload pointer, and next pointer.

## 4. Performance Characteristics

- **Insertion**: O(k * d) where k is the number of quadrants the rectangle intersects (1-4) and d is the tree depth (max_levels). Worst case O(k * log(n)) for balanced trees. Performance degrades significantly with deeply nested levels and many overlapping rectangles.
- **Point Query**: O(d) where d is tree depth, typically O(log(n)) for balanced trees. Traverses only the quadrants containing the point.
- **Rectangle Query**: O(k * d + m) where k is quadrants intersected, d is depth, and m is the number of matches found.
- **Removal/Change**: O(k * d + m) similar to rectangle query, as it must traverse and modify matching entries.
- **Walk/Count**: O(n) where n is the total number of rectangles, as it visits every entry.
- **Space Complexity**: O(n + n/max_points_per_node) for storing n rectangles plus node overhead.
- **Thread Safety**: Not thread-safe. External locking required for concurrent access.
- **Memory Recycling**: Internal free lists minimize allocation overhead for repeated insertions/deletions.

## 5. AI Usage Recommendations & Patterns

- **Best Practices:**
  - Always use the lifecycle functions (`create`/`destroy` or `reset`) for proper memory management.
  - Choose `max_levels=3` and `max_points_per_node=100-256` as starting points; tune based on your rectangle distribution and overlap characteristics.
  - Avoid excessively high `max_levels` (e.g., >5) when rectangles overlap heavily, as insertion performance degrades.
  - Reuse quadtrees across frames with `mulle_quadtree_reset()` rather than destroying and recreating, especially for animation or interactive applications.
  - The quadtree is most beneficial when performing multiple queries per frame; for single queries, brute-force iteration may be faster.
  - Pass NULL for the allocator parameter to use the default allocator.

- **Common Pitfalls:**
  - Do not access the internal `struct mulle_quadtree` fields directly; the structure is opaque.
  - Do not free payload pointers automatically; the caller is responsible for payload lifetime management.
  - Rectangles are clipped to the tree's bounding box; rectangles outside the bounds will not be inserted (return value 0).
  - When removing or changing payloads, the rectangle parameter is used for optimization—ensure it reasonably covers the area where the payload might exist.
  - The callback may be invoked multiple times for the same payload if a rectangle was split across multiple nodes during insertion.
  - Forgetting to call `mulle_quadtree_destroy()` will leak memory.

- **Idiomatic Usage:**
  - Use the callback pattern for queries rather than collecting results into arrays.
  - Check the return value of `mulle_quadtree_insert()` to detect rectangles outside the tree bounds.
  - Use `mulle_quadtree_count()` for quick counts without processing results.

## 6. Integration Examples

### Example 1: Creating and Populating a Quadtree

```c
#include <mulle-quadtree/mulle-quadtree.h>

struct mulle_quadtree   *tree;
CGRect                  world;
CGRect                  rect1, rect2;
void                    *payload1, *payload2;

// Create a quadtree for a 1000x1000 world
world    = CGRectMake( 0.0, 0.0, 1000.0, 1000.0);
tree     = mulle_quadtree_create( world, 3, 100, NULL);

// Insert some rectangles
rect1    = CGRectMake( 100.0, 100.0, 50.0, 50.0);
payload1 = (void *) 0x1234;  // Some user data
mulle_quadtree_insert( tree, rect1, payload1);

rect2    = CGRectMake( 200.0, 200.0, 80.0, 80.0);
payload2 = (void *) 0x5678;
mulle_quadtree_insert( tree, rect2, payload2);

// Clean up
mulle_quadtree_destroy( tree);
```

### Example 2: Querying with a Callback Function

```c
#include <mulle-quadtree/mulle-quadtree.h>
#include <stdio.h>

struct query_context
{
   int   count;
};

static void   found_callback( CGRect rect, void *payload, void *context)
{
   struct query_context   *ctx;

   ctx = (struct query_context *) context;
   printf( "Found rectangle at (%.2f, %.2f) size (%.2f, %.2f) with payload %p\n",
           rect.origin.x, rect.origin.y,
           rect.size.width, rect.size.height,
           payload);
   ctx->count++;
}

void   query_example( struct mulle_quadtree *tree)
{
   CGPoint                  point;
   struct query_context     ctx;
   unsigned int             found;

   // Find all rectangles containing a point
   point      = CGPointMake( 125.0, 125.0);
   ctx.count  = 0;

   found = mulle_quadtree_find_point( tree, point, found_callback, &ctx);
   printf( "Found %u rectangles at point (%.2f, %.2f)\n", found, point.x, point.y);
}
```

### Example 3: Using Reset for Frame-by-Frame Updates

```c
#include <mulle-quadtree/mulle-quadtree.h>

void   update_frame( struct mulle_quadtree *tree,
                     CGRect *rects,
                     void **payloads,
                     int count)
{
   int   i;

   // Reset tree for new frame (keeps memory allocated)
   mulle_quadtree_reset( tree, CGRectNull);  // CGRectNull = keep same bounds

   // Re-insert all current rectangles
   for( i = 0; i < count; i++)
      mulle_quadtree_insert( tree, rects[ i], payloads[ i]);
}
```

### Example 4: Collision Detection with Intersection Queries

```c
#include <mulle-quadtree/mulle-quadtree.h>

struct collision_context
{
   void   **colliding_objects;
   int    max_count;
   int    count;
};

static void   collision_callback( CGRect rect, void *payload, void *context)
{
   struct collision_context   *ctx;

   ctx = (struct collision_context *) context;
   if( ctx->count < ctx->max_count)
   {
      ctx->colliding_objects[ ctx->count] = payload;
      ctx->count++;
   }
}

int   find_collisions( struct mulle_quadtree *tree,
                       CGRect test_rect,
                       void **results,
                       int max_results)
{
   struct collision_context   ctx;

   ctx.colliding_objects = results;
   ctx.max_count         = max_results;
   ctx.count             = 0;

   mulle_quadtree_find_intersecting_rect( tree, test_rect, collision_callback, &ctx);

   return( ctx.count);
}
```

### Example 5: Removing Rectangles by Payload

```c
#include <mulle-quadtree/mulle-quadtree.h>

void   remove_object( struct mulle_quadtree *tree,
                      CGRect approximate_rect,
                      void *payload)
{
   unsigned int   removed;

   // Remove all rectangles with this payload
   // approximate_rect should cover the area where the object might be
   removed = mulle_quadtree_remove_payload( tree, approximate_rect, payload);

   if( removed > 0)
      printf( "Removed %u rectangle(s) for payload %p\n", removed, payload);
}
```

## 7. Dependencies

- **mulle-geometry**: Provides `CGRect`, `CGPoint`, `CGSize`, and geometric operations (intersection, containment testing). This is the sole direct dependency.

The library also indirectly depends on the mulle-geometry dependency chain, which includes mulle-core and its foundational libraries (allocator, buffer, etc.), but these are not directly referenced in the public API.
