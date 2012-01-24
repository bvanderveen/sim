#import <Cocoa/Cocoa.h>
#include "sim.h"

@interface AppDelegate : NSObject <NSApplicationDelegate> {
  NSWindow *window;
  SimContextRef simContext;
}
@end


