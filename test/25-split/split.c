#include <mulle-quadtree/mulle-quadtree.h>
#include <mulle-fprintf/mulle-fprintf.h>


static void   callback( CGRect rect, void *payload, void *context)
{
   unsigned int   *count = context;

   (*count)++;
   mulle_printf( "found: %s\n", (char *) payload);
}


int   main( int argc, char *argv[])
{
   struct mulle_quadtree   *tree;
   CGRect                  world;
   unsigned int            n;
   unsigned int            count;

   world = CGRectMake( 0, 0, 100, 100);
   tree  = mulle_quadtree_create( world, 3, 2, NULL);

   // insert three rects that force a split (max_rects_per_node=2)
   mulle_quadtree_insert( tree, CGRectMake( 10, 10, 10, 10), "A");
   mulle_quadtree_insert( tree, CGRectMake( 60, 60, 10, 10), "B");
   mulle_quadtree_insert( tree, CGRectMake( 80, 80, 10, 10), "C");

   n = mulle_quadtree_count( tree);
   mulle_printf( "count: %u\n", n);

   // point in A only
   count = 0;
   mulle_quadtree_find_point( tree, CGPointMake( 15, 15), callback, &count);
   mulle_printf( "hits at (15,15): %u\n", count);

   // point in B only
   count = 0;
   mulle_quadtree_find_point( tree, CGPointMake( 65, 65), callback, &count);
   mulle_printf( "hits at (65,65): %u\n", count);

   // point in C only
   count = 0;
   mulle_quadtree_find_point( tree, CGPointMake( 85, 85), callback, &count);
   mulle_printf( "hits at (85,85): %u\n", count);

   // point outside all rects
   count = 0;
   mulle_quadtree_find_point( tree, CGPointMake( 5, 5), callback, &count);
   mulle_printf( "hits at (5,5): %u\n", count);

   // a rect spanning two quadrants
   mulle_quadtree_insert( tree, CGRectMake( 40, 40, 20, 20), "D");
   n = mulle_quadtree_count( tree);
   mulle_printf( "count after D: %u\n", n);

   // point in D
   count = 0;
   mulle_quadtree_find_point( tree, CGPointMake( 45, 45), callback, &count);
   mulle_printf( "hits at (45,45): %u\n", count);

   // point in D and B overlap region (no overlap, separate)
   count = 0;
   mulle_quadtree_find_point( tree, CGPointMake( 55, 55), callback, &count);
   mulle_printf( "hits at (55,55): %u\n", count);

   // negative-size rect: standardized == (20, 20, 30, 30)
   mulle_quadtree_insert( tree, CGRectMake( 50, 50, -30, -30), "E");
   count = 0;
   mulle_quadtree_find_point( tree, CGPointMake( 25, 25), callback, &count);
   mulle_printf( "hits at (25,25) for E: %u\n", count);

   // z-order after split: E inserted after D, both cover (45,45)
   // E must be reported first (newest = frontmost)
   count = 0;
   mulle_quadtree_find_point( tree, CGPointMake( 45, 45), callback, &count);
   mulle_printf( "z-order at (45,45): %u\n", count);

   mulle_quadtree_destroy( tree);
   return( 0);
}
