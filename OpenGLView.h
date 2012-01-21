#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>
#import <QuartzCore/QuartzCore.h>

@interface OpenGLView : NSOpenGLView {
	CVDisplayLinkRef displayLink;
	double _timeFreq, _prevTime;
	void (*drawFunction)(double, void*);
	void *ctx;
}

- (id)initWithFrame:(NSRect)rect draw:(void(*)(double, void*))d context:(void*)ctx;

@end

