#import <Cocoa/Cocoa.h>
#include "sim.h"

@interface AppDelegate : NSObject <NSApplicationDelegate> {
  NSWindow *window;
  sim_ctx simContext;
}
@end


