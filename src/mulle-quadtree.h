//
//  mulle-quadtree.h
//  mulle-quadtree
//
//  Copyright (c) 2019 Nat! - Mulle kybernetiK.
//  All rights reserved.
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
//
//  Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
//  Neither the name of Mulle kybernetiK nor the names of its contributors
//  may be used to endorse or promote products derived from this software
//  without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//
// MulleQuadtree is very loosely based on the MIT Licensed:
// Kvad - a C99 quadtree implementation
// http://github.com/appscape/kvad

#ifndef mulle_quadtree_h__
#define mulle_quadtree_h__

#define MULLE__QUADTREE_VERSION  ((1UL << 20) | (1 << 8) | 1)
 
#include "include.h"

#include <stdio.h>


/** Opaque struct representing the quadtree. */
struct mulle_quadtree;

/**
 Callback for the find and walk functions.

 Ordering guarantee: when several objects overlap at the same location, the
 callback is invoked most recently inserted first. Insertion order is the
 z-order: if B is inserted after A, then B is "on top of" A and B is reported
 before A. Insert back to front (paint order) and the first callback for a
 point is the frontmost object.

 @param rect
        The stored fragment, which is the inserted rect clipped to a tree
        node. This is the region that was hit, not the rect you inserted.
        Use payload to identify the originating object.
 */
typedef void (*mulle_quadtree_callback)( CGRect rect, void* payload, void* context);

/**
 Creates the quadtree. Note that you must manually release the tree using
 mulle_quadtree_destroy().

 Operations on the quadtree are not thread-safe.

 The quadtree clips incoming rectangles against the world bounding box and
 stores fragments. A single inserted rectangle may be stored as multiple
 fragments across tree nodes. Counts and callback invocations are therefore
 in units of fragments, not original rectangles. Use the payload to identify
 the originating object.

 @param rect
        The bounding box. Origin can be negative. Width and height must be
        positive.
 @param max_levels
        Maximum number of levels (tiers 0..max_levels). If maximum level is
        reached, the nodes are not split anymore and rects are added to the
        node at the last level, ignoring max_rects_per_node. Use 3 if you
        don't know better. Using large levels may seriously impede insertion
        speed when lots of rectangles overlap.
 @param max_rects_per_node
        Maximum number of rect fragments to store in a single node. After this
        many are added to a single node, it is split into 4
        subnodes/quadrants. The split is suppressed when it would not separate
        any entries. Use 100 if you don't know better.
 @return Pointer to the quadtree object if successful, otherwise NULL
 */
MULLE__QUADTREE_GLOBAL
struct mulle_quadtree* mulle_quadtree_create( CGRect rect,
                                              unsigned int max_levels,
                                              unsigned int max_points_per_node,
                                              struct mulle_allocator *allocator);

/** Allocate a quadtree struct. Use mulle_quadtree_init to initialize it. */
MULLE__QUADTREE_GLOBAL
struct mulle_quadtree   *mulle_quadtree_alloc( struct mulle_allocator *allocator);

/** Initialize a previously allocated quadtree. */
MULLE__QUADTREE_GLOBAL
struct mulle_quadtree   *mulle_quadtree_init( struct mulle_quadtree *tree,
                                              CGRect world_rect,
                                              unsigned int max_levels,
                                              unsigned int max_rects_per_node);

/** Reset the quadtree with a new rectangle, keeps memory for reuse.
    Pass CGRectNull as world_rect to keep the existing bounding box.
 */
MULLE__QUADTREE_GLOBAL
void   mulle_quadtree_reset( struct mulle_quadtree* tree,
                             CGRect world_rect);

/** Release internal storage (nodes and data). Does not free the tree struct. */
MULLE__QUADTREE_GLOBAL
void   mulle_quadtree_done( struct mulle_quadtree *tree);

/** Free the tree struct. Call mulle_quadtree_done first, or use
    mulle_quadtree_destroy which does both. */
MULLE__QUADTREE_GLOBAL
void   mulle_quadtree_free( struct mulle_quadtree *tree);

/** Calls mulle_quadtree_done then mulle_quadtree_free. */
MULLE__QUADTREE_GLOBAL
void   mulle_quadtree_destroy( struct mulle_quadtree *tree);

/** Inserts a rect and an associated payload into the quadtree.
    The rect is standardized (negative sizes are flipped). Empty rects
    (zero width or height) are rejected.
    You are responsible for making sure that payload reference remains valid
    during the tree lifecycle.
    No check for duplicates is performed.
    @return 1 if stored, 0 if the rect does not intersect the quadtree or is
            empty, -1 if tree is NULL.
*/
MULLE__QUADTREE_GLOBAL
int   mulle_quadtree_insert( struct mulle_quadtree* tree,
                             CGRect rect,
                             void *payload);

/** Change payload of fragments matching coordinates and payload.
    The rect is standardized. Pass the same rect that was used for insertion
    to ensure all fragments are reached.
    @return Number of fragments changed.
 */
MULLE__QUADTREE_GLOBAL
unsigned int   mulle_quadtree_change_payload( struct mulle_quadtree* tree,
                                              CGRect rect,
                                              void *payload,
                                              void *newpayload);

/** Removes fragments matching coordinates and payload from the quadtree.
    The rect is standardized. Pass the same rect that was used for insertion
    to ensure all fragments are removed; a partial rect only removes
    fragments intersecting that region.
    @return Number of fragments removed.
 */
MULLE__QUADTREE_GLOBAL
unsigned int mulle_quadtree_remove_payload( struct mulle_quadtree* tree,
                                            CGRect rect,
                                            void* payload);

/** Removes all fragments with the given payload from the quadtree.
    Walks the entire tree, so no rect is needed. Use this when you don't
    have the original insertion rect available.
    @return Number of fragments removed.
 */
MULLE__QUADTREE_GLOBAL
unsigned int mulle_quadtree_remove_by_payload( struct mulle_quadtree* tree,
                                               void* payload);

/** Searches for a point inside the stored rect fragments. Calls supplied
    callback for every matched fragment, most recently inserted first, so the
    first callback is the frontmost object at that point.
    Each payload appears at most once, since fragments of one payload never
    overlap. Overlapping fragments of *different* payloads do share a node,
    which is why several objects can be reported for one point.
    @return Number of fragments found.
 */
MULLE__QUADTREE_GLOBAL
unsigned int   mulle_quadtree_find_point( struct mulle_quadtree* tree,
                                          CGPoint point,
                                          mulle_quadtree_callback callback,
                                          void *context);

/** Searches for rect fragments intersecting the given rect. The search rect
    is standardized. Calls supplied callback for every intersecting fragment.
    A single payload may produce multiple callbacks if it was fragmented
    across nodes.
    @return Number of fragments found.
 */
MULLE__QUADTREE_GLOBAL
unsigned int   mulle_quadtree_find_intersecting_rect( struct mulle_quadtree* tree,
                                                      CGRect rect,
                                                      mulle_quadtree_callback callback,
                                                      void *context);

/** Visits all fragments in the tree.
    @return Total number of fragments currently stored in the tree.
 */
MULLE__QUADTREE_GLOBAL
unsigned int mulle_quadtree_walk( struct mulle_quadtree* tree,
                                  mulle_quadtree_callback callback,
                                  void* context);


static inline unsigned int   mulle_quadtree_count( struct mulle_quadtree* tree)
{
    return( mulle_quadtree_walk( tree, NULL, NULL));
}

/*
 * dump tree structure
 */
MULLE__QUADTREE_GLOBAL
void   mulle_quadtree_dump( struct mulle_quadtree *tree, FILE *fp);


/*
 * Design decisions (documented here so future reviews have context):
 *
 * O(1) count:
 *    Not implemented. mulle_quadtree_count() walks the tree.  Nobody calls
 *    count() in a hot path; it's for debugging/assertions only.
 *
 * Update/move operation:
 *    Not implemented. Use mulle_quadtree_remove_by_payload() + insert().
 *    An in-place move that patches fragments across nodes is a research
 *    problem for marginal gain. For per-frame updates, use reset().
 *
 * Enumerator (mulle-container style _enumerate/_next):
 *    Not implemented. No business case for breaking out of a walk
 *    mid-iteration. The callback-based walk is sufficient.
 *
 * Early-out callback (int return to stop traversal):
 *    Not implemented. With the z-order guarantee the frontmost object is the
 *    first callback, so a caller who only wants that can ignore the rest.
 *    Revisit only if a caller has hundreds of overlapping rects at one point.
 *
 * find_topmost_point():
 *    Tried and removed. It was just find_point with an early return, and it
 *    added no information beyond "the first callback wins", which the z-order
 *    guarantee already states.
 *
 * Node collapse after removal:
 *    Not implemented. Split nodes stay split after removals. Fine for the
 *    primary use case (reset per frame). A long-lived tree with many removals
 *    would degrade, but that's not the target scenario.
 *
 * Z-order:
 *    Insertion order IS the z-order, and it is a guaranteed part of the API.
 *    Insert prepends, so a node list is ordered newest first, and find_point
 *    reports the frontmost object first.
 *
 *    This is load bearing: _mulle_quadtree_split_node must reverse the
 *    detached list before redistributing into the subnodes, because it
 *    prepends there too. Without the reversal the order comes out backwards
 *    and only after a split, which makes it look nondeterministic. See the
 *    order-after-split case in test/25-split.
 */

#endif
