/***************************************************************
**
** Angelo Library Source File
**
** File         :  source_template.c
** Module       :  misc
** Project      :  Angelo
** Author       :  SH
** Created      :  2025-01-08 (YYYY-MM-DD)
** License      :  MIT
** Description  :  A source file template.
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

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

const char* shaderSource =
"#include <metal_stdlib>                                                     \n"
"using namespace metal;                                                      \n"
"                                                                            \n"
"vertex float4 vertexShader(                                                 \n"
"    uint vertexID [[vertex_id]],                                            \n"
"    constant float3* vertexPositions [[buffer(0)]]                          \n"
") {                                                                         \n"
"    return float4(                                                          \n"
"        vertexPositions[vertexID][0],                                       \n"
"        vertexPositions[vertexID][1],                                       \n"
"        vertexPositions[vertexID][2],                                       \n"
"        1.0f                                                                \n"
"    );                                                                      \n"
"}                                                                           \n"
"                                                                            \n"
"fragment float4 fragmentShader(                                             \n"
"    float4 vertexOutPositions [[stage_in]]                                  \n"
") {                                                                         \n"
"    return float4(                                                          \n"
"        182.0f / 255.0f,                                                    \n"
"        240.0f / 255.0f,                                                    \n"
"        228.0f / 255.0f,                                                    \n"
"        1.0f                                                                \n"
"    );                                                                      \n"
"}                                                                           \n";



/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

@interface WindowDelegate : NSObject <NSWindowDelegate>

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


/***************************************************************
** MARK: STATIC FUNCTION DEFS
***************************************************************/

void render();

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




    id<MTLDevice> device = MTLCreateSystemDefaultDevice();

    layer = [CAMetalLayer layer];
    [layer setDevice:device];
    [layer setPixelFormat:MTLPixelFormatBGRA8Unorm];
    [window.contentView setLayer:layer];
    [window.contentView setWantsLayer:YES];
    [window setMinSize:NSMakeSize(200, 200)];

    log_info("Created window with title: %s", title);

    // CREATE METAL LIBRARY
    simd_float3 triangleVertices[] = {
        {-0.5f, -0.5f, 0.0f},
        { 0.5f, -0.5f, 0.0f},
        { 0.0f,  0.5f, 0.0f}
    };

    triangleVertexBuffer = [device newBufferWithBytes:triangleVertices
        length:sizeof(triangleVertices)
        options:MTLResourceStorageModeShared];

    NSError* error = nil;

    NSString* shaderSourceNSString = [NSString stringWithUTF8String:shaderSource];
    metalLibrary = [device newLibraryWithSource:shaderSourceNSString options:nil error:&error];

    if (!metalLibrary) {
        log_error("Failed to create metal library");
        return (WindowHandle_opt) { .value = 0, .is_some = false };
    }
    NSArray<NSString *> *functionNames = [metalLibrary functionNames];
    metalCommandQueue = [device newCommandQueue];

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

    metalRenderPSO = [device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];

    [pipelineDescriptor release];

    [window center];
    [window makeKeyAndOrderFront:nil];

    render();


    return (WindowHandle_opt) { .value = (intptr_t)window, .is_some = true };
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/

@implementation WindowDelegate

- (void) windowDidResize:(NSNotification *) notification
{
    NSWindow *window = [notification object];
    NSSize size = [window frame].size;
    log_info("Window resized to %f x %f", size.width, size.height);

    [layer setDrawableSize:size];

    render();
}

@end


void render()
{
    @autoreleasepool {
        id<CAMetalDrawable> drawable = [layer nextDrawable];

        if (drawable) {
            start_timer();
            
            MTLRenderPassDescriptor *passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
            passDescriptor.colorAttachments[0].texture = [drawable texture];
            passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
            passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(1.0, 0.0, 0.0, 1.0); // Red color
            passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

            id<MTLCommandBuffer> commandBuffer = [metalCommandQueue commandBuffer];
            id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];
            [renderEncoder setRenderPipelineState:metalRenderPSO];
            [renderEncoder setVertexBuffer:triangleVertexBuffer offset:0 atIndex:0];
            [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
            [renderEncoder endEncoding];
            [commandBuffer presentDrawable:drawable];
            [commandBuffer commit];

            [commandBuffer waitUntilCompleted];

            stop_timer();
            log_info("Render time: %.3f ms", (float)get_elapsed_micros() / 1000.0f);
        }
    }
}