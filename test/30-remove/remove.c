#include <mulle-quadtree/mulle-quadtree.h>
#include <mulle-fprintf/mulle-fprintf.h>


int   main( int argc, char *argv[])
{
   struct mulle_quadtree   *tree;
   CGRect                  world;
   unsigned int            n;

   world = CGRectMake( 0, 0, 100, 100);
   tree  = mulle_quadtree_create( world, 3, 4, NULL);

   mulle_quadtree_insert( tree, CGRectMake( 10, 10, 20, 20), "A");
   mulle_quadtree_insert( tree, CGRectMake( 50, 50, 20, 20), "B");
   mulle_quadtree_insert( tree, CGRectMake( 30, 30, 40, 40), "C");

   mulle_printf( "before remove: %u\n", mulle_quadtree_count( tree));

   n = mulle_quadtree_remove_payload( tree, CGRectMake( 50, 50, 20, 20), "B");
   mulle_printf( "removed: %u\n", n);
   mulle_printf( "after remove: %u\n", mulle_quadtree_count( tree));

   // remove non-existent
   n = mulle_quadtree_remove_payload( tree, CGRectMake( 50, 50, 20, 20), "B");
   mulle_printf( "removed again: %u\n", n);

   mulle_quadtree_destroy( tree);

   // test remove_by_payload: insert a large rect that fragments, then remove
   // by payload alone without needing the rect
   tree = mulle_quadtree_create( world, 3, 2, NULL);

   mulle_quadtree_insert( tree, CGRectMake( 10, 10, 10, 10), "X");
   mulle_quadtree_insert( tree, CGRectMake( 60, 60, 10, 10), "Y");
   mulle_quadtree_insert( tree, CGRectMake( 80, 80, 10, 10), "Z");
   // this spans multiple nodes after split
   mulle_quadtree_insert( tree, CGRectMake( 20, 20, 60, 60), "W");

   mulle_printf( "before remove_by_payload: %u\n", mulle_quadtree_count( tree));

   n = mulle_quadtree_remove_by_payload( tree, "W");
   mulle_printf( "removed by payload: %u\n", n);
   mulle_printf( "after remove_by_payload: %u\n", mulle_quadtree_count( tree));

   // W should be completely gone
   n = mulle_quadtree_find_point( tree, CGPointMake( 50, 50), NULL, NULL);
   mulle_printf( "find W at (50,50): %u\n", n);

   // X should still be there
   n = mulle_quadtree_find_point( tree, CGPointMake( 15, 15), NULL, NULL);
   mulle_printf( "find X at (15,15): %u\n", n);

   mulle_quadtree_destroy( tree);
   return( 0);
}
