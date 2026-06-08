#include <mulle-quadtree/mulle-quadtree.h>
#include <mulle-fprintf/mulle-fprintf.h>


int   main( int argc, char *argv[])
{
   struct mulle_quadtree   *tree;
   CGRect                  world;

   world = CGRectMake( 0, 0, 100, 100);
   tree  = mulle_quadtree_create( world, 3, 4, NULL);

   mulle_quadtree_insert( tree, CGRectMake( 10, 10, 20, 20), "A");
   mulle_quadtree_insert( tree, CGRectMake( 50, 50, 20, 20), "B");
   mulle_printf( "before reset: %u\n", mulle_quadtree_count( tree));

   // reset reuses nodes internally via mulle_storage
   mulle_quadtree_reset( tree, CGRectMake( 0, 0, 200, 200));
   mulle_printf( "after reset: %u\n", mulle_quadtree_count( tree));

   // insert again (should reuse storage nodes)
   mulle_quadtree_insert( tree, CGRectMake( 10, 10, 20, 20), "C");
   mulle_quadtree_insert( tree, CGRectMake( 150, 150, 20, 20), "D");
   mulle_printf( "after reinsert: %u\n", mulle_quadtree_count( tree));

   mulle_quadtree_destroy( tree);
   return( 0);
}
