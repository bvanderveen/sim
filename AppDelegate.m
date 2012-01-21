#import "AppDelegate.h"
#import "OpenGLView.h"

@implementation AppDelegate 

void draw(double t, void *ctx) {
  AppDelegate *del = (AppDelegate *)ctx;

  sim_update(del->simContext, t);
  sim_draw(del->simContext, t);
}

- (id)init {
  if (self = [super init]) {
    NSRect bounds = NSMakeRect(0,0,400,400);
    window = [[NSWindow alloc] initWithContentRect:bounds styleMask:NSTitledWindowMask | NSClosableWindowMask backing:NSBackingStoreBuffered defer:YES];

    simContext = sim_create();

    [window setContentView:[[[OpenGLView alloc] initWithFrame:bounds draw:draw context:self] autorelease]];
  }
  return self; 
}

- (void)dealloc {
  sim_destroy(simContext);
  [window release];
  [super dealloc];
}

- (void)applicationWillFinishLaunching:(NSNotification *)notification {
  [window makeKeyAndOrderFront:self];
}

@end


