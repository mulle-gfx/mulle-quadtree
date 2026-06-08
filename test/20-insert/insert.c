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
   tree  = mulle_quadtree_create( world, 3, 4, NULL);

   // insert three rects
   mulle_quadtree_insert( tree, CGRectMake( 10, 10, 20, 20), "A");
   mulle_quadtree_insert( tree, CGRectMake( 50, 50, 20, 20), "B");
   mulle_quadtree_insert( tree, CGRectMake( 30, 30, 40, 40), "C");

   n = mulle_quadtree_count( tree);
   mulle_printf( "count: %u\n", n);

   // point in A and C
   count = 0;
   mulle_quadtree_find_point( tree, CGPointMake( 15, 15), callback, &count);
   mulle_printf( "hits at (15,15): %u\n", count);

   // point only in B and C
   count = 0;
   mulle_quadtree_find_point( tree, CGPointMake( 55, 55), callback, &count);
   mulle_printf( "hits at (55,55): %u\n", count);

   // point outside all
   count = 0;
   mulle_quadtree_find_point( tree, CGPointMake( 5, 5), callback, &count);
   mulle_printf( "hits at (5,5): %u\n", count);

   mulle_quadtree_destroy( tree);
   return( 0);
}
