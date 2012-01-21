#import "OpenGLView.h"

@implementation OpenGLView 

- (id)initWithFrame:(NSRect)frame draw:(void(*)(double, void*))d context:(void *)leCtx {
  if (self = [super initWithFrame:frame pixelFormat:[NSOpenGLView defaultPixelFormat]]) {
    _timeFreq = CVGetHostClockFrequency();
    drawFunction = d;
    ctx = leCtx;
  }
  return self;
}

- (void)dealloc {
  CVDisplayLinkRelease(displayLink);
  [super dealloc];
}

- (BOOL)getFrameForTime:(const CVTimeStamp *)time {
  BOOL result = NO;

  @synchronized (self) {

    NSOpenGLContext *currentContext = [self openGLContext];
    [currentContext makeCurrentContext];

    CGLContextObj context = [currentContext CGLContextObj];

//    CGLLockContext(context);

    double hostTime = (double) time->hostTime;
    double now = hostTime / _timeFreq;

    // this will not update unless 1/30th of a second has passed since the last update
    if ( now < _prevTime + (1.0 / 30.0) )
    {
        // returning NO will cause the layer to NOT be redrawn
        result = NO;
    }
    else
    {
        // change whatever you want to change here, as a function of time elapsed

        drawFunction(now - _prevTime, ctx);

        _prevTime = now;
        // return YES to have your layer redrawn
        result = YES;
    }

    [currentContext flushBuffer];
//    CGLUnlockContext(context);
  }

  return result;
}

static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime,
CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
{
    CVReturn result = [(OpenGLView*)displayLinkContext getFrameForTime:outputTime];
    return result;
}

- (void)prepareOpenGL {
  // Synchronize buffer swaps with vertical refresh rate
  GLint swapInt = 1;
  [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
 
  // Create a display link capable of being used with all active displays
  CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
 
  // Set the renderer output callback function
  CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, self);


  // Set the display link for the current renderer
  CGLContextObj cglContext = [[self openGLContext] CGLContextObj];
  CGLPixelFormatObj cglPixelFormat = [[self pixelFormat] CGLPixelFormatObj];
  CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);

  // Activate the display link
  CVDisplayLinkStart(displayLink);
}

@end

