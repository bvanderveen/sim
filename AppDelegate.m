#import "AppDelegate.h"
#import "OpenGLView.h"

@implementation AppDelegate 

void draw(double dt, void *ctx) {
  AppDelegate *del = (AppDelegate *)ctx;

  NSLog(@"will update context");
  SimContextUpdate(del->simContext, dt);
  NSLog(@"did update context");

  NSLog(@"will draw context");
  SimContextDraw(del->simContext, dt);
  NSLog(@"did draw context");
}

- (id)init {
  if (self = [super init]) {
    NSRect bounds = NSMakeRect(0,0,400,400);
    window = [[NSWindow alloc] initWithContentRect:bounds styleMask:NSTitledWindowMask | NSClosableWindowMask backing:NSBackingStoreBuffered defer:NO];

    simContext = SimContextCreate();

    openGLView = [[OpenGLView alloc] initWithFrame:bounds draw:draw context:self];

    NSLog(@"Will set content view. Window = %@, view = %@", window, openGLView);
    [window setContentView:openGLView];
    NSLog(@"Did set content view.");
  }
  return self; 
}

- (void)dealloc {
  SimContextDestroy(simContext);
  [openGLView release];
  [window release];
  [super dealloc];
}

- (void)applicationWillFinishLaunching:(NSNotification *)notification {
  [window makeKeyAndOrderFront:self];
}

@end


