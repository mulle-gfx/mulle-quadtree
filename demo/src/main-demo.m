#import "import.h"


#define USE_QUADTREE

#define N_PARTICLES  5000
#define N_FINGERS    12


// Lesson learned. The quadtree is only useful if
// * you can keep it around for multiple frames (you usually can't)
// * or you have multiple queries during a frame, for a single query like
//   a mousepointer check it's actually slower
//
#ifdef USE_QUADTREE
static struct mulle_quadtree  *quadtree;
#endif


static void   lighten_layer( CGRect rect, void *payload, void *context)
{
   CGGraphicsLayer              *layer = MULLE_OBJC_CLASS_CAST( CGGraphicsLayer, payload);
   CGColor                      *color = context;
   CABasicAnimation             *animation;
   struct _mulle_objc_property  *property;

   //fprintf( stderr, "%s\n", __PRETTY_FUNCTION__);

   if( [layer numberOfAnimations] <= 1)
   {
      property  = MulleObjCInstanceSearchProperty( layer, @selector( backgroundColor));
      assert( property);

      animation = [CABasicAnimation alloc];
      animation = [animation initWithProperty:property
                              startColorValue:*color
                                endColorValue:[layer backgroundColor]
                                      options:NULL];
      animation = [animation autorelease];
      [layer addAnimation:animation];
   }
}


// brute force, instead of quadtree
static void  layers_find_intersecting_rect( struct mulle_pointerarray *layers,
                                            CGRect rect,
                                            mulle_quadtree_callback callback,
                                            void *userinfo)
{
   CGGraphicsLayer   *layer;

   mulle_pointerarray_for( layers, layer)
   {
      if( CGRectIntersectsRect( [layer frame], rect))
         (*callback)( rect, layer, userinfo);
   }
}


@interface MyWindow : UIWindow
@end


@implementation MyWindow

//
- (UIEvent  *) acceptMouseMotionEvent:(UIMouseMotionEvent *) event
{
   UIWindow        *window;
   UIView          *contentView;
   CGPoint         point;
   CGPoint         pointOnCircle;
   CGRect          rect;
   NSUInteger      i;
   CAPolarPoint    polar;
   CGColor         color;
   CGFloat         factor;
   CGFloat         hour;

   //
   // any event carries mouseLocation information, so we actually needn't
   // check this, but anyway...
   //

   contentView = [self contentView];

   //
   // there we are, now use the mouse motion event to move the bounds
   // could be nicer...
   //
   point = [event locationInView:contentView];
   //fprintf( stderr, "%s %s\n", __PRETTY_FUNCTION__, CGPointUTF8String( point));

   //
   // merely a ploy to waste time and show that quadtree gets better the
   // more queries it gets per lifetime, compared to the brute forcer
   //
   for( i = 0; i < N_FINGERS; i += 1)
   {
      factor        = (CGFloat) i / (N_FINGERS - 1);
      color         = CGColorMakeWithHSL( factor, 0.5 + 0.5 * factor, 0.3 + 0.7 * factor);

      hour          = ((CGFloat) i / N_FINGERS) * 12.0;
      hour          = hour == 0.0 ? 12 : hour;
      polar         = CAPolarPointMakeWithAngle( 100, CAHourToAngle( hour));
      pointOnCircle = CGPointAddPolarPoint( point, polar);

      rect = CGRectMake( 0, 0, 3, 3);
      rect = CGRectCenterAtPoint( rect, pointOnCircle);


#ifdef USE_QUADTREE
      mulle_quadtree_find_intersecting_rect( quadtree, rect, lighten_layer, &color);
#else
      layers_find_intersecting_rect( [contentPlane _layers], rect, lighten_layer, &color);
#endif
   }
   //mulle_quadtree_find_point( quadtree, point, lighten_layer, NULL);

   return( event);  // don't consume event
}

@end

//
// We just copy the layer frame into the quadtree, that way we don't have to
// run through every layer to find the rectangles that intersect.
//
#ifdef USE_QUADTREE

static void   buildQuadTree( struct mulle_pointerarray *layers,
                             struct mulle_quadtree *quadtree)
{
   struct mulle_pointerarrayenumerator   rover;
   CGGraphicsLayer                       *layer;

   rover = mulle_pointerarray_enumerate( layers);
   while( _mulle_pointerarrayenumerator_next( &rover, (void **) &layer))
   {
      mulle_quadtree_insert( quadtree, [layer frame], layer);
   }
   mulle_pointerarrayenumerator_done( &rover);
}

#endif


//
// This is run once during each frame. it's after -layout has been called
// as we are animating layers, and possibly the contentPlane may have been
// resized. We don't relayout the layers though currently. But if we did,
// then this would be a the right time to do the quadtree build. We have to
// redo the quadtree everytime, because the positions of the layers change
// during animation. Or would post animations be better ?
//
#ifdef USE_QUADTREE
static void   windowDidLayoutViews( UIWindow *window,
                                    struct CGFrameInfo *info)
{
   UIView   *contentPlane;

   contentPlane = [window contentPlane];
   mulle_quadtree_reset( quadtree, [contentPlane contentRect]);
   buildQuadTree( [contentPlane _layers], quadtree);
}
#endif


//
// Create a thousand cyclically animated layers and place them into the
// contentPlane view.
//
static void   setupSceneInView( UIView *contentPlane)
{
   CABasicAnimation                 *animation;
   CGGraphicsLayer                  *layer;
   CGRect                           frame;
   CGRect                           srcRect;
   CGRect                           dstRect;
   NSUInteger                       i;
   struct CABasicAnimationOptions   options;

   options = CABasicAnimationOptionsMakeCurve( mulle_relativetimerange_make( 0, 2.0),
                                               CAAnimationCurveEaseOut,
                                               INFINITY,
                                               YES);

   frame = [contentPlane contentRect];
   frame = UIEdgeInsetsInsetRect( frame, UIEdgeInsetsMake( 10, 10, 10, 10));


   for( i = 0; i < N_PARTICLES; i++)
   {
      srcRect  = CGRectMakeRandomInRect( frame, CGSizeMake( 5, 5), CGSizeMake( 20, 20));
      dstRect  = CGRectMakeRandomInRect( frame, CGSizeMake( 5, 5), CGSizeMake( 20, 20));

      layer = [CGGraphicsLayer layerWithFrame:srcRect];
//   printf( "hue: %.2f\n", hue);
      [layer setBackgroundColor:CGColorMakeWithHSL( 0.0,
                                                   0.0,
                                                   0.3 + 0.3 * CGFloatMakeRandom())];

      animation = [[[CABasicAnimation alloc] initWithProperty:MulleObjCInstanceSearchProperty( layer, @selector( frame))
                                               startRectValue:srcRect
                                                 endRectValue:dstRect
                                                      options:&options] autorelease];
      [layer addAnimation:animation];

      //fprintf( stderr, "%s %s\n", __PRETTY_FUNCTION__, CGRectUTF8String( [layer frame]));
      [contentPlane addLayer:layer];
   }
}


int   main( int argc, char *argv[])
{
   MyWindow        *window;
   UIApplication   *app;
   MulleUIPerf     *perf;

   app = [UIApplication sharedInstance];
   {
      window = [MyWindow windowWithFrame:CGRectMake( 0.0, 0.0, 800.0, 600.0)];
      assert( window);

      [window setBackgroundColor:CGColorMakeWithRGBA( 1.0, 1.0, 1.0, 1.0)];
      [window setFrameRate:UIWindowFrameRateUnsynced];
      [window setMouseMotionEventsEnabled:YES];

#ifdef USE_QUADTREE
      //
      // 1,1000 is OK
      // 1000,1 takes forever
      //
      quadtree = mulle_quadtree_create( [[window contentPlane] contentRect], 3, 256, NULL);
      [window setDidLayoutViewsCallback:windowDidLayoutViews];
#endif
      setupSceneInView( [window contentPlane]);

      perf = [MulleUIPerf object];
      [window setFrameDelegate:perf];

      [app addWindow:window];
   }


   return( [app runApp]);
}



//
//                           ,╓≤æKª╨╙╙▓█D"╙╙╙*╨¥W▄╓,
//                      ╓▄ª╙└╧▓        █           ⁿ▓▓ª╦w
//                  ╓#╜^*█⌐   `╕       █           ╓▌    `╙%w
//               ╓ΦΓ      █    ╙⌐     ,█,,,Φ      ╓▌         ╙¥▄
//            ,▄▀ █▄       █µ,╓╝,╓≤╗≡≡ª╨╨╨¥≡∞╗w╓,ª█,  ,       ]▄▀V,
//          ,Φ└   ║"▀▄     ,▄▀╨"                 "╙¥▓▓       ▄▀  º▄¥,
//        ,#▌Φ▓▄╓  Φ `█Q▄M╙                           ╙¥▄ ,▄▀ ╙▄⌐`  `Φ
//       ▄▀   `V,"▀▀¼╓╩└                                 ╙¥▓╗ ,       ╙▄
//      Φ       `¥═▄▀                            ╒          ▀▄          ▓
//    .▓         ╓▓]V▄H       ▓                 ,Å            ▀,         ▀
//    █      ,-ª▓╛  `██▀½      "▄  ,╓╓╓╓;╓▄▄▄▄╗▄█,             ╙▄         ▀
//   █  `██▄   ▓. - Φ█═.~     ,≤█▄▀-      `   █└"▀▀▀▀▄╓╓╓╓       ▌    ██▄, █
//  ║⌐  " ▀   ▓╨⌐   }▀`      ,█  `           █      .█▀▄▄███▄     ▌  ` ▀    ▌
//  ▌        ▐M A   '┴      ▌                ▀        ╙█▌████▌    ╙▄        ║⌐
// ║⌐        █⌐'║▄ Γr2«    ║▀                 ▄▄▄      ╙▌▄███      █         ▌
// █]     ,▄▐H  ██████═ª  ╓` ▄█▀▀▀▀█▄        ████     ╓███` `╙╙║█  ▐H        █
// █ "% ,▄▀ ║░╠K█═╣███^   ╛╓█▀      ▐█         ,╓╓═ª╙└   █      ╙█▄▐▌▓######▌╫
// █║▄▄▄█▄▄▄║H:  "╙▀███▄╓▄,█▌╓▄,,▄∩  ██p$ΦΦ▀▀▀▀▀ΦΦΦR≡≥▄╓ ║▄       ╙j▌` ▄▀╙╗ `╫
// █"      ╙▐H-═M ▄███████¥█H "^╙    '█, ,«^``     `     ║▌        ▐M▄▀    "Φ█
// ║        !█▄▄████≤═^`╓█ █▌╓▀╦▄▀█   ╙█⌐               ╓█▌        ▓ `       █
//  ▌      ,▄██╫██ ▄█═╜" █▌▐█╣Hj⌐ █    █▌╙¥w,           ██▄       ╓▌        ▐M
//  ╢"▄▄#▀▀╙╟█Ü███▀`▀█  ║██J█▄▌jH █    ██    ╙▀W▄;    ,██  `╙w,   █▄     ,╓ █
//   █╙     ▐███▌▀█w ▀█ ╠██████j▌║█▌   ▀██▄       `╙▓██▀w,     ╙╗█'`"▀█▀╙"`║`
//   ╙▌     Φ████████▄██ ███████████    ═██▀W▄,   ╓█▀└         ╓▀ "    ╙▄ ╓▀
//    ╙▄     ╙▀███╙█▄▐██▄,╙▀█████▄▀▄▀▄   ,█M╗ ▀▄██▀╙""*ⁿ═─≈-,,▄▌,        ▄▀
//     `▌╙µ,▄Φ▀x███▄██████████▀V███▀╙└ ,▄#▌████▀`"*«,      .█▀▄▀▄,      ▄╛
//       ▀Ö█  ;▓████████▄▄█▀▄╙W "V▄██▀▀│▄███▀└ ▀╦     "*═╥▄▀ ║,  ║▀▄  ,▓
//        ╙W▀ '█████╨▀▀█████████   "▀▀████└      ╙▄   ,▄▀'   `▀▀╣H ▐▀▄▀
//          ╙▄  ▀▀Γ╓▌  ▄▀ⁿ`╙▀███      ██▌         ╓█R▀▄A█        ▀▀▄▀
//            ╙W,Φ╙  ║█        ▄╙╙ª%≡╦██≤≤╗╗≡≡╜▀▀   ╣▌   █╕     ,▄▀
//              `▀▄M█┘        ║▀▌   ╫"    *Φ▄  █       Φ` ▀▌, ▄M╙
//                 "▀╦,       M ╙▌  ▀       `█W▐▌       wæQ▄▀╙
//                     ╙▀W╥,╓╣   ╙█║        ╫M  █    ,╓▄╝╙`
//                          ╙╙▀%▄▄▄▌,≤▄_▄≡,▐▓▓▄▄#ª▀"`

