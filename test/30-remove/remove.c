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
   return( 0);
}
