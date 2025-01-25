/***************************************************************
**
** Angelo Library Source File
**
** File         :  win_macos.m
** Module       :  win
** Project      :  Angelo
** Author       :  SH
** Created      :  2025-01-08 (YYYY-MM-DD)
** License      :  MIT
** Description  :  MacOS window implementation
**
***************************************************************/

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include "win.h"
#include "../debug/debug.h"
#include "../util/util.h"

#include <simd/simd.h>
#include <QuartzCore/CAMetalLayer.h>

#import <AppKit/AppKit.h>
#import <Metal/Metal.h>
#import <CoreGraphics/CoreGraphics.h>

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

const char* shaderSource =
"#include <metal_stdlib>                                    \n"
"using namespace metal;                                     \n"
"                                                           \n"
" struct VertexOut {                                        \n"
"    float4 position [[position]];                          \n"
"    float2 texCoord;                                       \n"
"};                                                         \n"
"                                                           \n"
"vertex VertexOut vertexShader(                             \n"
"    uint vertexID [[vertex_id]]                            \n"
") {                                                        \n"
"   float2 positions[4] =                                   \n"
"   {                                                       \n"
"       {-1.0, -1.0},                                       \n"
"       {1.0, -1.0},                                        \n"
"       {-1.0, 1.0},                                        \n"
"       {1.0, 1.0}                                          \n"
"   };                                                      \n"
"                                                           \n"
"   float2 texCoords[4] =                                   \n"
"   {                                                       \n"
"       {0.0, 0.0},                                         \n"
"       {1.0, 0.0},                                         \n"
"       {0.0, 1.0},                                         \n"
"       {1.0, 1.0}                                          \n"
"   };                                                      \n"
"                                                           \n"
"  VertexOut out;                                           \n"
"   out.position = float4(positions[vertexID], 0.0, 1.0);   \n"
"   out.texCoord = texCoords[vertexID];                     \n"
"   return out;                                             \n"
"}                                                          \n"
"                                                           \n"
"fragment float4 fragmentShader(                            \n"
"    VertexOut in [[stage_in]],                             \n"
"    texture2d<float> tex [[texture(0)]]                    \n"
") {                                                        \n"
"   constexpr sampler s(                                    \n"
"        address::clamp_to_edge,                            \n"
"        filter::linear                                     \n"
"    );                                                     \n"
"                                                           \n"
"    return tex.sample(s, in.texCoord);                     \n"
"}                                                          \n";



/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

@interface WindowDelegate : NSObject <NSWindowDelegate>

@end

@interface CustomView : NSView

@end



/***************************************************************
** MARK: STATIC VARIABLES
***************************************************************/

CAMetalLayer *layer;
id<MTLDevice> metalDevice;
id<MTLLibrary> metalLibrary;
id<MTLCommandQueue> metalCommandQueue;
id<MTLRenderPipelineState> metalRenderPSO;
id<MTLBuffer> triangleVertexBuffer;

// Shared memory and Metal texture
id<MTLTexture> sharedTexture;
id<MTLBuffer> sharedBuffer;
CGContextRef sharedContext;

float mouseX = 0.0f;
float mouseY = 0.0f;

int frameTimeUs = 0;

/***************************************************************
** MARK: STATIC FUNCTION DEFS
***************************************************************/

void create_shared_texture(size_t textureWidth, size_t textureHeight);
void destroy_shared_texture();
void render(size_t width, size_t height);

/***************************************************************
** MARK: PUBLIC FUNCTIONS
***************************************************************/

WindowHandle_opt create_window(int width, int height, const char* title)
{
    NSWindow *window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, width, height)
        styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable)
        backing:NSBackingStoreBuffered defer:NO];

    [window setTitle:[NSString stringWithUTF8String:title]];

    WindowDelegate *delegate = [[WindowDelegate alloc] init];
    [window setDelegate:delegate];
    [window setAcceptsMouseMovedEvents:YES];

    metalDevice = MTLCreateSystemDefaultDevice();

    layer = [CAMetalLayer layer];
    
    [layer setDevice:metalDevice];
    [layer setPixelFormat:MTLPixelFormatBGRA8Unorm];
    
    /* disable this for faster updates but seems to reset itself and will cause some artifacts */
    [layer setDisplaySyncEnabled:YES];

    CustomView *view = [[CustomView alloc] initWithFrame:NSMakeRect(0, 0, width, height)];
    [window setContentView:view];
    [window.contentView setLayer:layer];
    [window.contentView setWantsLayer:YES];
    [window.contentView setLayerContentsPlacement: NSViewLayerContentsPlacementTopLeft];
    [window.contentView setLayerContentsRedrawPolicy: NSViewLayerContentsRedrawDuringViewResize];
    [window setMinSize:NSMakeSize(200, 200)];

    log_info("Created window with title: %s", title);

    simd_float3 triangleVertices[] = {
        {-0.5f, -0.5f, 0.0f},
        { 0.5f, -0.5f, 0.0f},
        { 0.0f,  0.5f, 0.0f}
    };

    triangleVertexBuffer = [metalDevice newBufferWithBytes:triangleVertices
        length:sizeof(triangleVertices)
        options:MTLResourceStorageModeShared];

    
    // draw some stuff

    


    // CREATE METAL LIBRARY

    NSError* error = nil;

    NSString* shaderSourceNSString = [NSString stringWithUTF8String:shaderSource];
    metalLibrary = [metalDevice newLibraryWithSource:shaderSourceNSString options:nil error:&error];

    if (!metalLibrary) {
        log_error("Failed to create metal library with error: %s", [[error localizedDescription] UTF8String]);
        return (WindowHandle_opt) { .value = 0, .is_some = false };
    }
    NSArray<NSString *> *functionNames = [metalLibrary functionNames];
    metalCommandQueue = [metalDevice newCommandQueue];

    id<MTLFunction> vertexFunction = [metalLibrary newFunctionWithName:@"vertexShader"];
    id<MTLFunction> fragmentFunction = [metalLibrary newFunctionWithName:@"fragmentShader"];

    if (vertexFunction && fragmentFunction) {
        log_info("Successfully created vertex and fragment functions");
    } else {
        log_error("Failed to create vertex and fragment functions");
        return (WindowHandle_opt) { .value = 0, .is_some = false };
    }
    

    MTLRenderPipelineDescriptor *pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineDescriptor.label = @"Simple Pipeline";
    pipelineDescriptor.vertexFunction = vertexFunction;
    pipelineDescriptor.fragmentFunction = fragmentFunction;
    pipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;

    metalRenderPSO = [metalDevice newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];

    [pipelineDescriptor release];

    [window center];
    [window makeKeyAndOrderFront:nil];
    
    render((size_t)width, (size_t)height);

    return (WindowHandle_opt) { .value = (intptr_t)window, .is_some = true };
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/

@implementation WindowDelegate

- (NSSize) windowWillResize:(NSNotification *) notification toSize:(NSSize) size
{
    size_t width = size.width;
    size_t height = size.height;

    log_info("Window will resize to %f x %f", size.width, size.height);
    
    NSSize nsSize = NSMakeSize(width, height);
    [layer setDrawableSize:nsSize];
    

    return size;
}

- (void) windowDidResize:(NSNotification *) notification
{
    NSWindow *window = [notification object];
    // n.b contentsize is the 'client area' so smaller than the window size
    NSSize contentSize = [window contentRectForFrameRect:[window frame]].size;

    size_t width = contentSize.width;
    size_t height = contentSize.height;


    printf("Content border thickness: %f\n", [window contentBorderThicknessForEdge: NSMaxYEdge]);

    NSSize nsSize = NSMakeSize(width, height);
    [layer setDrawableSize:nsSize];

    log_info("Window resized to %f x %f", contentSize.width, contentSize.height);

    render(width, height);
}

@end

@implementation CustomView


- (void)mouseMoved:(NSEvent *)event {
    NSPoint mouseLocation = [event locationInWindow];
    //log_info("Mouse moved to: %f, %f", mouseLocation.x, mouseLocation.y);
    mouseX = mouseLocation.x;
    mouseY = mouseLocation.y;

    render(self.bounds.size.width, self.bounds.size.height);
}

- (void)mouseDown:(NSEvent *)theEvent {

    log_info("MOUSE DOWN");

}

- (void)updateTrackingAreas {
    [super updateTrackingAreas];

    // Remove existing tracking areas if necessary
    for (NSTrackingArea *area in self.trackingAreas) {
        [self removeTrackingArea:area];
    }

    // Create a new tracking area
    NSTrackingArea *trackingArea = [[NSTrackingArea alloc] initWithRect:self.bounds
                                                                options:(NSTrackingMouseEnteredAndExited |
                                                                         NSTrackingMouseMoved |
                                                                         NSTrackingActiveInKeyWindow)
                                                                  owner:self
                                                               userInfo:nil];
    [self addTrackingArea:trackingArea];
}


@end


void create_shared_texture(size_t textureWidth, size_t textureHeight)
{
    // create a core graphics context
    size_t bytesPerPixel = 4; // For BGRA8Unorm
    size_t alignedBytesPerRow = ((textureWidth * bytesPerPixel + 15) / 16) * 16;

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    sharedContext = CGBitmapContextCreate(NULL, textureWidth, textureHeight, 8,
                                          alignedBytesPerRow, colorSpace,
                                          kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little);
    CGColorSpaceRelease(colorSpace);

    void *sharedMemory = CGBitmapContextGetData(sharedContext);

    sharedBuffer = [metalDevice newBufferWithBytesNoCopy:sharedMemory length:CGBitmapContextGetBytesPerRow(sharedContext)*CGBitmapContextGetHeight(sharedContext) options:MTLResourceStorageModeShared deallocator: nil];

    // Create a shared texture
    MTLTextureDescriptor *textureDescriptor = [[MTLTextureDescriptor alloc] init];
    textureDescriptor.pixelFormat = MTLPixelFormatBGRA8Unorm;
    textureDescriptor.width = textureWidth;
    textureDescriptor.height = textureHeight;
    textureDescriptor.usage = MTLTextureUsageShaderRead;
    textureDescriptor.storageMode = MTLStorageModeShared;

    sharedTexture = [sharedBuffer newTextureWithDescriptor:textureDescriptor offset:0 bytesPerRow:CGBitmapContextGetBytesPerRow(sharedContext)];

    CGContextClearRect(sharedContext, CGRectMake(0, 0, textureWidth, textureHeight));
   // CGContextSetRGBFillColor(sharedContext, 1.0, 0.5, 0.0, 1.0);
   // CGContextFillRect(sharedContext, CGRectMake(10, 10, textureWidth - 20, textureHeight - 20));    


    //int numX = 32;
    //int numY = 135;

    int numX = 1;
    int numY = 1;


    NSDictionary<NSAttributedStringKey, id> * attributes = @{
        //NSFontAttributeName: [NSFont fontWithName:@"Helvetica" size:8.0],
        NSFontAttributeName: [NSFont fontWithName:@"Iosevka" size:20.0],
        NSForegroundColorAttributeName: [NSColor whiteColor]
    };

    int labelWidth = 100;
    int labelHeight = 10;

    // Draw the text
    CGContextSetTextMatrix(sharedContext, CGAffineTransformIdentity); // Reset text matrix
    CGContextTranslateCTM(sharedContext, 0, textureHeight); // Flip coordinate system
    CGContextScaleCTM(sharedContext, 1.0, -1.0);

    for (int i = 0; i < numX; i++) {
        for (int j = 0; j < numY; j++) {

            CGPoint point = CGPointMake(10+labelWidth*i, 25 + labelHeight*j);

            NSString *labelText = [NSString stringWithFormat:@"Cursor Pos: %2.2f x %2.2f", mouseX, mouseY];
            
            // Set up text attributes
            NSAttributedString *attributedString = [[NSAttributedString alloc] initWithString:labelText attributes:attributes];

            CTLineRef line = CTLineCreateWithAttributedString((CFAttributedStringRef)attributedString);
            CGContextSetTextPosition(sharedContext, point.x, textureHeight - point.y);
            CTLineDraw(line, sharedContext);

            // Cleanup
            CFRelease(line);
        }
    }

    CGPoint point = CGPointMake(10, 50);

    NSString *labelText = [NSString stringWithFormat:@"Frame Time: %.3f ms", (float)frameTimeUs / 1000.0f];
    
    // Set up text attributes
    NSAttributedString *attributedString = [[NSAttributedString alloc] initWithString:labelText attributes:attributes];

    CTLineRef line = CTLineCreateWithAttributedString((CFAttributedStringRef)attributedString);
    CGContextSetTextPosition(sharedContext, point.x, textureHeight - point.y);
    CTLineDraw(line, sharedContext);

    // Cleanup
    CFRelease(line);
    
    CGContextSetRGBFillColor(sharedContext, 0.25, 0.25, 1.0, 1.0);
    CGContextFillRect(sharedContext, CGRectMake(mouseX - 10, mouseY - 10, 20, 20)); 



}
void destroy_shared_texture()
{
    sharedTexture = nil;
    sharedBuffer = nil;
    CGContextRelease(sharedContext);
}

void render(size_t width, size_t height)
{
    @autoreleasepool {
        id<CAMetalDrawable> drawable = [layer nextDrawable];

        if (drawable) {

            start_timer();

            create_shared_texture(width, height);
            
            MTLRenderPassDescriptor *passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
            passDescriptor.colorAttachments[0].texture = [drawable texture];
            passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
            passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(1.0, 0.0, 0.0, 1.0); // Red color
            passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

            id<MTLCommandBuffer> commandBuffer = [metalCommandQueue commandBuffer];
            id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];
            [renderEncoder setRenderPipelineState:metalRenderPSO];
            [renderEncoder setFragmentTexture:sharedTexture atIndex:0];
            //[renderEncoder setVertexBuffer:triangleVertexBuffer offset:0 atIndex:0];
            [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
            [renderEncoder endEncoding];
            [commandBuffer presentDrawable:drawable];
            [commandBuffer commit];
            [commandBuffer waitUntilCompleted];
            
            

            
            destroy_shared_texture();

            stop_timer();
            //log_info("Render time: %.3f ms", (float)get_elapsed_micros() / 1000.0f);
            frameTimeUs = get_elapsed_micros();
        }
    }
}