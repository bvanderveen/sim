#import <Cocoa/cocoa.h>
#import "AppDelegate.h"

int main(int argc, char *argv[]) {
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  NSApplication *app = [NSApplication sharedApplication];
  
  AppDelegate *del = [[[AppDelegate alloc] init] autorelease];
  [app setDelegate:del];
  [app run];
  [pool drain];

  return 0;
}



