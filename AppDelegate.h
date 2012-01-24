#import <Cocoa/Cocoa.h>
#include "sim/sim.h"

@interface AppDelegate : NSObject <NSApplicationDelegate> {
  NSWindow *window;
  SimContextRef simContext;
}
@end


