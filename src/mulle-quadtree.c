//
//  mulle-quadtree.c
//  mulle-quadtree
//
//  Copyright (c) 2021 Nat! - Mulle kybernetiK.
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
// Kvad - a C99 quadtree implementation
// http://github.com/appscape/kvad

#include "mulle-quadtree.h"

#include <string.h>

#include "include-private.h"


// A linked list element storing a point and the reference to associated payload
struct _mulle_quadtree_data
{
    struct _mulle_quadtree_data  *next;
    CGRect                       rect;
    void                         *payload;
};

struct _mulle_quadtree_node
{
    struct _mulle_quadtree_node   *subnodes[4];  // 0=NW,1=NE,2=SW,3=SE quadrants
    CGRect                        rect;          // Bounding box
    struct _mulle_quadtree_data   *data;         // Pointer to the first rect data (linked list)
    unsigned int                  data_count;    // Number of points stored in the linked list
    unsigned int                  level;         // This node's level in the tree
};

struct mulle_quadtree
{
   struct _mulle_quadtree_node  *root_node;
   struct mulle_allocator       *allocator;
   struct mulle_storage         node_storage;
   struct mulle_storage         data_storage;
   unsigned int                 max_levels;
   unsigned int                 max_rects_per_node;
};



static struct _mulle_quadtree_node   *
   _mulle_quadtree_create_node( struct mulle_quadtree *tree,
                                CGRect rect,
                                unsigned int level)
{
   struct _mulle_quadtree_node   *n;

   n = _mulle_storage_calloc( &tree->node_storage);
   n->rect  = rect;
   n->level = level;
   return( n);
}


static inline void   _mulle_quadtree_recycle_node( struct mulle_quadtree *tree,
                                                   struct _mulle_quadtree_node *n)
{
   _mulle_storage_free( &tree->node_storage, n);
}


/***/

static struct _mulle_quadtree_data   *
   _mulle_quadtree_create_data( struct mulle_quadtree *tree)
{
   struct _mulle_quadtree_data   *d;

   d = _mulle_storage_calloc( &tree->data_storage);
   return( d);
}


static void   _mulle_quadtree_recycle_data( struct mulle_quadtree *tree,
                                            struct _mulle_quadtree_data *d)
{
   _mulle_storage_free( &tree->data_storage, d);
}


/***/

static unsigned int
   _mulle_quadtree_node_change_payload( struct _mulle_quadtree_node* node,
                                        CGRect rect,
                                        void* payload,
                                        void* newpayload)
{
   unsigned int                  count;
   unsigned int                  i;
   struct _mulle_quadtree_data   *data;
   struct _mulle_quadtree_node   *subnode;

   count = 0;

   if( node->subnodes[0])
   {
      for( i = 0; i < 4; i++)
      {
         subnode = node->subnodes[ i];
         if( CGRectIntersectsRect( subnode->rect, rect))
            count += _mulle_quadtree_node_change_payload( subnode, rect, payload, newpayload);
      }
      return( count);
   }

   data  = node->data;

   while( data)
   {
      if( data->payload == payload && CGRectIntersectsRect( data->rect, rect))
      {
          // Payload and point matches, change this entry
          count++;
          data->payload = newpayload;
      }
      data = data->next;
   }

   return count;
}

static unsigned int
   _mulle_quadtree_node_remove_payload( struct _mulle_quadtree_node* node,
                                        CGRect rect,
                                        void* payload,
                                        struct mulle_quadtree *tree)
{
   unsigned int                  count;
   struct _mulle_quadtree_data   *data;
   struct _mulle_quadtree_data   *prev_data;
   struct _mulle_quadtree_node   *subnode;
   unsigned int                  i;

   count = 0;

   if( node->subnodes[0])
   {
      for( i = 0; i < 4; i++)
      {
         subnode = node->subnodes[ i];
         if( CGRectIntersectsRect( subnode->rect, rect))
            count += _mulle_quadtree_node_remove_payload( subnode, rect, payload, tree);
      }
      return( count);
   }

   data      = node->data;
   prev_data = NULL;

   while( data)
   {
      if( data->payload == payload)
      {
          // Payload matches, remove this entry
          count++;
          node->data_count--;
          if( prev_data)
          {
              prev_data->next = data->next;
              _mulle_quadtree_recycle_data( tree, data);
              data = prev_data->next;
          }
          else
          {
              node->data = data->next;
              _mulle_quadtree_recycle_data( tree, data);
              data = node->data;
          }
      }
      else
      {
          prev_data = data;
          data = data->next;
      }
   }

    return count;
}

static unsigned int
   _mulle_quadtree_node_remove_by_payload( struct _mulle_quadtree_node* node,
                                           void* payload,
                                           struct mulle_quadtree *tree)
{
   unsigned int                  count;
   struct _mulle_quadtree_data   *data;
   struct _mulle_quadtree_data   *prev_data;
   unsigned int                  i;

   count = 0;

   if( node->subnodes[0])
   {
      for( i = 0; i < 4; i++)
         count += _mulle_quadtree_node_remove_by_payload( node->subnodes[ i], payload, tree);
      return( count);
   }

   data      = node->data;
   prev_data = NULL;

   while( data)
   {
      if( data->payload == payload)
      {
         count++;
         node->data_count--;
         if( prev_data)
         {
            prev_data->next = data->next;
            _mulle_quadtree_recycle_data( tree, data);
            data = prev_data->next;
         }
         else
         {
            node->data = data->next;
            _mulle_quadtree_recycle_data( tree, data);
            data = node->data;
         }
      }
      else
      {
         prev_data = data;
         data = data->next;
      }
   }

   return( count);
}

static int  _mulle_quadtree_node_insert( struct mulle_quadtree* tree,
                                         struct _mulle_quadtree_node* node,
                                         CGRect rect,
                                         void* payload);


static void  _mulle_quadtree_split_node( struct mulle_quadtree* tree,
                                         struct _mulle_quadtree_node* node)
{
   CGSize                        subnode_rect_size;
   CGRect                        subnode_rect;
   struct _mulle_quadtree_data   *data;
   struct _mulle_quadtree_data   *next;
   struct _mulle_quadtree_data   *reversed;
   struct _mulle_quadtree_node   *subnode;
   unsigned int                  i;
   unsigned int                  covers_all;

   subnode_rect_size = CGSizeMake( node->rect.size.width / 2.0,
                                   node->rect.size.height / 2.0);

   for( i = 0; i < 4; i++)
   {
       subnode_rect = CGRectMake( node->rect.origin.x + ((i % 2 == 1) ? subnode_rect_size.width : 0),
                                  node->rect.origin.y + ((i >= 2) ? subnode_rect_size.height : 0),
                                  subnode_rect_size.width,
                                  subnode_rect_size.height);
       node->subnodes[i] = _mulle_quadtree_create_node( tree, subnode_rect, node->level + 1);
   }

   // If every entry would land in all four children, splitting does not
   // separate anything and just causes 4x blowup.  Refuse to split.
   covers_all = 0;
   data       = node->data;
   while( data)
   {
      for( i = 0; i < 4; i++)
      {
         if( ! CGRectIntersectsRect( data->rect, node->subnodes[ i]->rect))
            break;
      }
      if( i == 4)
         covers_all++;
      else
         break;  // at least one entry does not cover all children, split is useful
      data = data->next;
   }

   if( covers_all == node->data_count)
   {
      // undo: recycle created subnodes, keep node as leaf
      for( i = 0; i < 4; i++)
      {
         _mulle_quadtree_recycle_node( tree, node->subnodes[ i]);
         node->subnodes[ i] = NULL;
      }
      return;
   }

   //
   // ..and move all data from it into subnodes.
   //
   // The node list is ordered newest first, because _mulle_quadtree_node_insert
   // prepends. Since it also prepends into the subnodes, we must feed the
   // entries oldest first, otherwise the subnode lists come out reversed and
   // the z-order guarantee breaks. So reverse the detached list first.
   //
   data       = node->data;
   node->data = NULL;

   reversed = NULL;
   while( data)
   {
      next       = data->next;
      data->next = reversed;
      reversed   = data;
      data       = next;
   }

   data = reversed;
   while( data)
   {
      next = data->next;

      for( i = 0; i < 4; i++)
      {
         subnode = node->subnodes[ i];
         if( CGRectIntersectsRect( data->rect, subnode->rect))
            _mulle_quadtree_node_insert( tree, subnode, data->rect, data->payload);
      }

      // remove from our tree
      _mulle_quadtree_recycle_data( tree, data);
      node->data_count--;
      data = next;
   }
}

//
// The incoming rectangle is clipped against the quadtree and every part
// is placed into the appropriate node
//
static int  _mulle_quadtree_node_insert( struct mulle_quadtree* tree,
                                         struct _mulle_quadtree_node* node,
                                         CGRect rect,
                                         void  *payload)
{
   struct _mulle_quadtree_data   *new_data;
   struct _mulle_quadtree_node   *subnode;
   CGRect                        intersection;
   unsigned int                  i;
   int                           workDone;

   assert( tree);
   assert( node);

   workDone = 0;
   if( node->subnodes[ 0])
   {
      for( i = 0; i < 4; i++)
      {
         subnode      = node->subnodes[ i];
         // the test is cheap but the actual intersection is slow, assume
         // that this fails more often than not
         if( ! CGRectIntersectsRect( subnode->rect, rect))
            continue;

         intersection = CGRectIntersection( subnode->rect, rect);
         workDone |= _mulle_quadtree_node_insert( tree, subnode, intersection, payload);
      }
      return( workDone);
   }

   // only memorize the intersections that fit in our node,
   // which is analog to how subnodes get their client rects
   if( ! CGRectIntersectsRect( rect, node->rect))
      return( 0);

   intersection = CGRectIntersection( rect, node->rect);

   // Leaf node, insert here by creating a new element at the head of linked list

   new_data          = _mulle_quadtree_create_data( tree);
   new_data->rect    = intersection;
   new_data->payload = payload;
   new_data->next    = NULL;
   if( node->data)
   {
//      assert(node->data_count > 0);
      new_data->next = node->data;
   }
   node->data = new_data;
   node->data_count++;

   // If maximum number of points reached AND below max level and not split yet:
   // split this node
   if( node->level < tree->max_levels &&
       node->data_count > tree->max_rects_per_node &&
       ! node->subnodes[0])
   {
      _mulle_quadtree_split_node( tree, node);
   }
   return( 1);
}

static unsigned int
   _mulle_quadtree_node_find_point( struct _mulle_quadtree_node* node,
                                    CGPoint point,
                                    mulle_quadtree_callback callback,
                                    void* context)
{
   unsigned int                  count;
   unsigned int                  i;
   struct _mulle_quadtree_data   *data;

   count = 0;
   if( node->subnodes[ 0])
   {
      assert( ! node->data);

      for( i = 0; i < 4; i++)
         if( CGRectContainsPoint( node->subnodes[i]->rect, point))
            count += _mulle_quadtree_node_find_point( node->subnodes[i], point, callback, context);
      return( count);
   }

   // Leaf node
   data = node->data;
   while( data)
   {
      if( CGRectContainsPoint( data->rect, point))
      {
         count++;
         if (callback)
            (*callback)( data->rect, data->payload, context);
      }
      data = data->next;
   }
   return( count);
}


static unsigned int
   _mulle_quadtree_node_find_intersecting_rect( struct _mulle_quadtree_node* node,
                                                CGRect rect,
                                                mulle_quadtree_callback callback,
                                                void* context)
{
   unsigned int                 count;
   unsigned int                 i;
   struct _mulle_quadtree_data  *data;

   count = 0;
   if( node->subnodes[ 0])
   {
      assert( ! node->data);

      for( i = 0; i < 4; i++)
         if( CGRectIntersectsRect( node->subnodes[i]->rect, rect))
            count += _mulle_quadtree_node_find_intersecting_rect( node->subnodes[i],
                                                                  rect,
                                                                  callback,
                                                                  context);
      return( count);
   }

   // Leaf node
   data = node->data;
   while( data)
   {
      if( CGRectIntersectsRect( data->rect, rect))
      {
         count++;
         if (callback)
            (*callback)( data->rect, data->payload, context);
      }
      data = data->next;
   }
   return( count);
}


static unsigned int
   _mulle_quadtree_node_walk( struct _mulle_quadtree_node* node,
                              mulle_quadtree_callback callback,
                              void* context)
{
   unsigned int                  count;
   struct _mulle_quadtree_data   *data;
   unsigned int                  i;

   count = 0;
   if( node->subnodes[0])
   {
      assert( ! node->data);

      for( i = 0; i < 4; i++)
         count += _mulle_quadtree_node_walk( node->subnodes[ i], callback, context);
      return( count);
   }

   data = node->data;
   while(data)
   {
      count++;
      (*callback)( data->rect, data->payload, context);
      data = data->next;
   }
   assert( count == node->data_count);
   return( count);
}


static void
   _mulle_quadtree_node_dump( struct _mulle_quadtree_node *node,
                              FILE *fp,
                              unsigned int level,
                              unsigned int quadrant)
{
   struct _mulle_quadtree_data   *data;
   unsigned int                    i;

   for( i = 0; i < level; i++)
      mulle_fputc( ' ', fp);
   mulle_fprintf( fp, "%u/%u: %.2f %.2f %.2f %.2f\n",
                    level, quadrant,
                     node->rect.origin.x,
                     node->rect.origin.y,
                     node->rect.size.width,
                     node->rect.size.height);

   if( node->subnodes[0])
   {
      assert( ! node->data);

      for( i = 0; i < 4; i++)
        _mulle_quadtree_node_dump( node->subnodes[ i], fp, level + 1, i);
      return;
   }

   data = node->data;
   while(data)
   {
      for( i = 0; i <= level; i++)
         mulle_fputc( ' ', fp);
      mulle_fprintf( fp, "[%.2f %.2f %.2f %.2f = %p]\n",
                     data->rect.origin.x,
                     data->rect.origin.y,
                     data->rect.size.width,
                     data->rect.size.height,
                     data->payload);
      data = data->next;
   }
}

static void
   _mulle_quadtree_deeprecycle_node( struct mulle_quadtree *tree,
                                     struct _mulle_quadtree_node* node)
{
   struct _mulle_quadtree_data   *data;
   struct _mulle_quadtree_data   *curr;
   unsigned int                    i;

   if( node->subnodes[0])
   {
     // Non-leaf node: first free subnodes..
      for( i = 0; i < 4; i++)
         _mulle_quadtree_deeprecycle_node( tree, node->subnodes[i]);
   }

   // Leaf nodes: free data..
   data = node->data;
   while( data)
   {
      curr = data;
      data = data->next;
      _mulle_quadtree_recycle_data( tree, curr);
   }
   //..then free this node
   _mulle_quadtree_recycle_node( tree, node);
}

// Public functions
struct mulle_quadtree   *mulle_quadtree_init( struct mulle_quadtree *tree,
                                            CGRect world_rect,
                                            unsigned int max_levels,
                                            unsigned int max_rects_per_node)
{
   assert( world_rect.size.width > 0);
   assert( world_rect.size.height > 0);
   assert( max_rects_per_node > 0);

   // Sane behaviour in case asserts are disabled
   if (max_rects_per_node == 0)
      max_rects_per_node = 1;

   tree->max_levels          = max_levels;
   tree->max_rects_per_node  = max_rects_per_node;
   tree->root_node           = _mulle_quadtree_create_node( tree, world_rect, 0);
   return( tree);
}


struct mulle_quadtree   *mulle_quadtree_alloc( struct mulle_allocator *allocator)
{
   struct mulle_quadtree   *tree;

   tree            = mulle_allocator_calloc( allocator, 1, sizeof( struct mulle_quadtree));
   tree->allocator = allocator;
   _mulle_storage_init( &tree->node_storage,
                        sizeof( struct _mulle_quadtree_node),
                        alignof( struct _mulle_quadtree_node),
                        64,
                        allocator);
   _mulle_storage_init( &tree->data_storage,
                        sizeof( struct _mulle_quadtree_data),
                        alignof( struct _mulle_quadtree_data),
                        256,
                        allocator);
   return( tree);
}


struct mulle_quadtree   *mulle_quadtree_create( CGRect world_rect,
                                                unsigned int max_levels,
                                                unsigned int max_rects_per_node,
                                                struct mulle_allocator *allocator)
{
   struct mulle_quadtree   *tree;

   tree = mulle_quadtree_alloc( allocator);
   mulle_quadtree_init( tree, world_rect, max_levels, max_rects_per_node);
   return( tree);
}


void   mulle_quadtree_free( struct mulle_quadtree* tree)
{
   if( ! tree)
      return;

   mulle_allocator_free( tree->allocator, tree);
}


void   mulle_quadtree_done( struct mulle_quadtree  *tree)
{
   if( ! tree)
      return;

   _mulle_storage_done( &tree->node_storage);
   _mulle_storage_done( &tree->data_storage);
}


void   mulle_quadtree_reset( struct mulle_quadtree  *tree,
                             CGRect world_rect)
{
   if( ! tree)
      return;

   if( CGRectIsNull( world_rect))
      world_rect = tree->root_node->rect;

   _mulle_quadtree_deeprecycle_node( tree, tree->root_node);
   tree->root_node = _mulle_quadtree_create_node( tree, world_rect, 0);
}


void   mulle_quadtree_destroy( struct mulle_quadtree *tree)
{
   mulle_quadtree_done( tree);
   mulle_quadtree_free( tree);
}


int   mulle_quadtree_insert( struct mulle_quadtree *tree,
                             CGRect rect,
                             void *payload)
{
   if( ! tree)
      return( -1);

   rect = CGRectStandardize( rect);
   if( CGRectIsEmpty( rect))
      return( 0);

   return( _mulle_quadtree_node_insert( tree, tree->root_node, rect, payload));
}


unsigned int   mulle_quadtree_remove_payload( struct mulle_quadtree* tree,
                                              CGRect rect,
                                              void *payload)
{
   if( ! tree)
      return( 0);

   rect = CGRectStandardize( rect);
   return( _mulle_quadtree_node_remove_payload( tree->root_node, rect, payload, tree));
}


unsigned int   mulle_quadtree_remove_by_payload( struct mulle_quadtree* tree,
                                                 void *payload)
{
   if( ! tree)
      return( 0);

   return( _mulle_quadtree_node_remove_by_payload( tree->root_node, payload, tree));
}


unsigned int   mulle_quadtree_change_payload( struct mulle_quadtree* tree,
                                              CGRect rect,
                                              void *payload,
                                              void *newpayload)
{
   if( ! tree)
      return( 0);

   rect = CGRectStandardize( rect);
   return( _mulle_quadtree_node_change_payload( tree->root_node, rect, payload, newpayload));
}


unsigned int   mulle_quadtree_find_point( struct mulle_quadtree *tree,
                                          CGPoint p,
                                          mulle_quadtree_callback callback,
                                          void *context)
{
   if( ! tree)
      return( 0);

   return _mulle_quadtree_node_find_point( tree->root_node, p, callback, context);
}


unsigned int   mulle_quadtree_find_intersecting_rect( struct mulle_quadtree *tree,
                                                      CGRect r,
                                                      mulle_quadtree_callback callback,
                                                      void *context)
{
   if( ! tree)
      return( 0);

   r = CGRectStandardize( r);
   return( _mulle_quadtree_node_find_intersecting_rect( tree->root_node, r, callback, context));
}


static void   mulle_quadtree_dummy_callback( CGRect rect,
                                             void *payload,
                                             void *context)
{
   (void) rect;
   (void) payload;
   (void) context;
}


unsigned int   mulle_quadtree_walk( struct mulle_quadtree *tree,
                                    mulle_quadtree_callback callback,
                                    void *context)
{
   if( ! tree)
      return( 0);
   if( ! callback)
      callback = mulle_quadtree_dummy_callback;
   return _mulle_quadtree_node_walk( tree->root_node, callback, context);
}


void   mulle_quadtree_dump( struct mulle_quadtree *tree, FILE *fp)
{
   if( ! tree || !fp)
      return;
   _mulle_quadtree_node_dump( tree->root_node, fp, 0, 0);
}
