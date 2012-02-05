#import <Cocoa/Cocoa.h>
#include "sim/sim.h"

@class OpenGLView;

@interface AppDelegate : NSObject <NSApplicationDelegate> {
  NSWindow *window;
  SimContextRef simContext;
  OpenGLView *openGLView;
}
@end


