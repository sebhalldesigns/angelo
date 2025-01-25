/***************************************************************
**
** Angelo Library Source File
**
** File         :  app_unix.c
** Module       :  misc
** Project      :  Angelo
** Author       :  SH
** Created      :  2025-01-24 (YYYY-MM-DD)
** License      :  MIT
** Description  :  The unix implementation of the app module.
**
***************************************************************/

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include "app.h"
#include "app_unix.h"

#include "../debug/debug.h"
#include "../win/win.h" 
#include "../win/win_unix.h"

#include <stdlib.h> 
#include <string.h>

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

/***************************************************************
** MARK: STATIC VARIABLES
***************************************************************/


static void global_registry_handle(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version);
static void global_registry_remove_handle(void *data, struct wl_registry *registry, uint32_t name);

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial);
static void xdg_surface_configure_callback(void *data, struct xdg_surface *xdg_surface, uint32_t serial);

static void top_level_configure_callback(void* data, struct xdg_toplevel* toplevel, int32_t width, int32_t height, struct wl_array* states);
static void top_level_close_callback(void* data, struct xdg_toplevel* toplevel);

static void wl_seat_capabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities);
static void wl_seat_name(void *data, struct wl_seat *wl_seat, const char *name);

static void pointer_enter_callback(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t surface_x, wl_fixed_t surface_y);
static void pointer_exit_callback(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface);
static void pointer_motion_callback(void *data, struct wl_pointer *wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y);
static void pointer_button_callback(void *data, struct wl_pointer *wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state);

static void pointer_axis_callback(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value);
static void pointer_frame_callback(void *data, struct wl_pointer *wl_pointer);
static void pointer_axis_source_callback(void *data, struct wl_pointer *wl_pointer, uint32_t axis_source);
static void pointer_axis_stop_callback(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis);
static void pointer_axis_discrete_callback(void *data, struct wl_pointer *wl_pointer, uint32_t axis, int32_t discrete);

static void on_frame_configure(struct libdecor_frame *frame, struct libdecor_configuration *configuration, void *user_data);
static void on_frame_close(struct libdecor_frame *frame, void *user_data);
static void on_commit(struct libdecor_frame *frame, void *user_data);

static void handle_error(struct libdecor *context, enum libdecor_error error, const char *message);


const struct wl_registry_listener registry_listener = 
{
	.global = global_registry_handle,
	.global_remove = global_registry_remove_handle,
};

const struct xdg_wm_base_listener xdg_wm_base_listener = 
{
    .ping = xdg_wm_base_ping,
};

const struct xdg_toplevel_listener toplevelListener = 
{
    .configure = top_level_configure_callback,
    .close = top_level_close_callback,
};

const struct xdg_surface_listener xdg_surface_listener = 
{
    .configure = xdg_surface_configure_callback,
};

const struct wl_seat_listener wl_seat_listener = 
{
    .capabilities = wl_seat_capabilities,
    .name = wl_seat_name,
};

const struct wl_pointer_listener wl_pointer_listener = 
{
    .enter = pointer_enter_callback,
    .leave = pointer_exit_callback,
    .motion = pointer_motion_callback,
    .button = pointer_button_callback,
    .axis = pointer_axis_callback,
    .frame = pointer_frame_callback,
    .axis_source = pointer_axis_source_callback,
    .axis_stop = pointer_axis_stop_callback,
    .axis_discrete = pointer_axis_discrete_callback
};

struct libdecor_frame_interface frame_interface = {
    .configure = on_frame_configure,
    .close = on_frame_close,
    .commit = on_commit,
};

static struct libdecor_interface libdecor_iface = {
	.error = handle_error,
};



/* OpenGL 3.3 context attributes */
static int visual_attribs[] = {
    GLX_X_RENDERABLE, True,
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
    GLX_RENDER_TYPE, GLX_RGBA_BIT,
    GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    GLX_ALPHA_SIZE, 8,
    GLX_DEPTH_SIZE, 24,
    GLX_STENCIL_SIZE, 8,
    GLX_DOUBLEBUFFER, True,
    None
};


/***************************************************************
** MARK: STATIC FUNCTION DEFS
***************************************************************/

/* function to check for glx extensions */
static Bool is_extension_supported(const char *extList, const char *extension);

/***************************************************************
** MARK: PUBLIC FUNCTIONS
***************************************************************/

AppHandle_opt create_app(const char* title)
{
    log_info("App created");

    /* check for wayland compositor */
    struct wl_display *waylandDisplay = wl_display_connect(NULL);
    if (waylandDisplay != NULL)
    {
        log_info("Wayland environment detected");
        
        /* initialise wayland */

        struct libdecor *decor_context = libdecor_new(waylandDisplay, &libdecor_iface);
        if (!decor_context) {
            log_error("Failed to start libdecor");
            return (AppHandle_opt) { .value = (intptr_t)0, .is_some = false };
        }

        log_info("Libdecor started");
        
        UnixApp *app = malloc(sizeof(UnixApp));
        app->title = title;
        app->appType = UNIX_APP_WAYLAND;
        app->data.waylandData.display = waylandDisplay;
        app->data.waylandData.registry = wl_display_get_registry(waylandDisplay);
        app->data.waylandData.decor_context = decor_context;
        app->data.waylandData.kdeDecorations = NULL;
        app->data.waylandData.zxdgDecorations = NULL;
        app->windowHandle = 0;
        
        wl_registry_add_listener(app->data.waylandData.registry, &registry_listener, app);
        wl_display_roundtrip(waylandDisplay);

        log_info("Wayland compositor: %p", app->data.waylandData.compositor);

        return (AppHandle_opt) { .value = (intptr_t)app, .is_some = true };

    }

    /* check for x display */
    Display* xDisplay = XOpenDisplay(NULL);
    if (xDisplay != NULL)
    {
        log_info("Xorg environment detected");

        /* initialise xorg */

        /* try to chose a framebuffer */
        int fbcount;
        GLXFBConfig *fbc = glXChooseFBConfig(xDisplay, DefaultScreen(xDisplay), visual_attribs, &fbcount);
        if (!fbc) {
            log_error("Failed to retrieve a framebuffer config");
            return (AppHandle_opt) { .value = (intptr_t)0, .is_some = false };
        }

        /* Pick the first matching FB config */
        GLXFBConfig bestFbc = fbc[0];
        XFree(fbc);

        /* get a visual */
        XVisualInfo *vi = glXGetVisualFromFBConfig(xDisplay, bestFbc);
        if (!vi) {
            log_error("Failed to get a visual");
            return (AppHandle_opt) { .value = (intptr_t)0, .is_some = false };
        }

        /* Create a colormap */
        Colormap colormap = XCreateColormap(xDisplay, RootWindow(xDisplay, vi->screen), vi->visual, AllocNone);
        XSetWindowAttributes windowAttributes;
        windowAttributes.colormap = colormap;
        windowAttributes.event_mask = ExposureMask | KeyPressMask;



        UnixApp *app = malloc(sizeof(UnixApp));
        app->title = title;
        app->appType = UNIX_APP_XORG;
        app->data.xorgData.display = xDisplay;
        app->data.xorgData.screen = DefaultScreen(xDisplay);
        app->data.xorgData.root = RootWindow(xDisplay, app->data.xorgData.screen);
        app->data.xorgData.bestFbc = bestFbc;
        app->data.xorgData.visualInfo = vi;
        app->data.xorgData.colormap = colormap;
        app->data.xorgData.windowAttributes = windowAttributes;

        return (AppHandle_opt) { .value = (intptr_t)app, .is_some = true };
    }

    log_error("Failed to find either Wayland or Xorg environment");

    return (AppHandle_opt) { .value = (intptr_t)0, .is_some = false };

}

int run_app(AppHandle handle)
{   
    UnixApp *app = (UnixApp*)handle;
    
    
    if (app == NULL)
    {
        log_error("Invalid app handle");
        return -1;
    }

    if (app->appType == UNIX_APP_WAYLAND)
    {
        log_info("Running Wayland app");

        while (true)
        {

            wl_display_dispatch_pending(app->data.waylandData.display);
            libdecor_dispatch(app->data.waylandData.decor_context, 100);

        }

        

        log_info("App stopped");

        return 0;
    }
    else if (app->appType == UNIX_APP_XORG)
    {
        while (true)
        {
            XEvent event;
            /*while (XPending(app->display)) {
                XNextEvent(app->display, &event);
                clear_window(handle, app->windowHandle);
                swap_window_buffers(handle, app->windowHandle);
            }*/

            clear_window(handle, app->windowHandle);
            swap_window_buffers(handle, app->windowHandle);
        }
    }

    

    log_info("App stopped");

    return 0;
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/

static Bool is_extension_supported(const char *extList, const char *extension) {
    const char *start;
    const char *where, *terminator;

    where = strchr(extension, ' ');
    if (where || *extension == '\0') {
        return False;
    }

    for (start = extList;;) {
        where = strstr(start, extension);

        if (!where) break;

        terminator = where + strlen(extension);

        if ((where == start || *(where - 1) == ' ') &&
            (*terminator == ' ' || *terminator == '\0')) {
            return True;
        }

        start = terminator;
    }
    return False;
}


static void global_registry_handle(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{

    UnixApp *app = (UnixApp*)data;
    if (app == NULL || app->appType != UNIX_APP_WAYLAND)
    {
        return;
    }
 
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        app->data.waylandData.compositor = (struct wl_compositor*)(wl_registry_bind(app->data.waylandData.registry, name, &wl_compositor_interface, 4));
    }

    if (strcmp(interface, wl_shm_interface.name) == 0) {
        app->data.waylandData.shm = (struct wl_shm*)(wl_registry_bind(app->data.waylandData.registry, name, &wl_shm_interface, 1));
    }

    if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        app->data.waylandData.xdWmBase = (struct xdg_wm_base*)(wl_registry_bind(app->data.waylandData.registry, name, &xdg_wm_base_interface, 1));
        xdg_wm_base_add_listener(app->data.waylandData.xdWmBase,
                &xdg_wm_base_listener, app);
    }

    if (strcmp(interface, wl_seat_interface.name) == 0) {
        app->data.waylandData.seat = (struct wl_seat*)(wl_registry_bind(app->data.waylandData.registry, name, &wl_seat_interface, 7));
        wl_seat_add_listener(app->data.waylandData.seat, &wl_seat_listener, app);
    }
    
    if (strcmp(interface, org_kde_kwin_server_decoration_manager_interface.name) == 0) {
        app->data.waylandData.kdeDecorations = (struct org_kde_kwin_server_decoration_manager*)(wl_registry_bind(app->data.waylandData.registry, name, &org_kde_kwin_server_decoration_manager_interface, 1));
    }

    if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0) {
        app->data.waylandData.zxdgDecorations = (struct zxdg_decoration_manager_v1*) wl_registry_bind(
            app->data.waylandData.registry,
            name,
            &zxdg_decoration_manager_v1_interface,
            1
        );

        log_info("Got zxdg decorations\n");

    }
}

static void global_registry_remove_handle(void *data, struct wl_registry *registry, uint32_t name)
{
    
}

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial)
{
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static void xdg_surface_configure_callback(void *data, struct xdg_surface *xdg_surface, uint32_t serial)
{
    xdg_surface_ack_configure(xdg_surface, serial);

    log_info("Surface configured");
}

static void top_level_configure_callback(void* data, struct xdg_toplevel* toplevel, int32_t width, int32_t height, struct wl_array* states)
{


    UnixApp *app = (UnixApp*)data;
    if (app == NULL || app->appType != UNIX_APP_WAYLAND)
    {
        return;
    }

    if (app->windowHandle == 0)
    {
        log_info("Window handle is 0");
        return;
    }

    UnixWindow* unixWindow = (UnixWindow*)app->windowHandle;

    wl_egl_window_resize(
        unixWindow->data.waylandData.eglWindow,
        width,
        height,
        0, 0
    );

    wl_surface_commit(unixWindow->data.waylandData.wl_surface);
    wl_display_roundtrip(unixWindow->data.waylandData.wl_display);

   

    log_info("Top level surface configured");

}

static void top_level_close_callback(void* data, struct xdg_toplevel* toplevel)
{
    log_info("Top level surface closed");
}

static void wl_seat_capabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities)
{

    UnixApp *app = (UnixApp*)data;
    if (app == NULL || app->appType != UNIX_APP_WAYLAND)
    {
        return;
    }
 
    bool have_pointer = capabilities & WL_SEAT_CAPABILITY_POINTER;

    if (have_pointer && app->data.waylandData.pointer == NULL) {
            app->data.waylandData.pointer = wl_seat_get_pointer(app->data.waylandData.seat);
            wl_pointer_add_listener(app->data.waylandData.pointer,
                            &wl_pointer_listener, app);
    } else if (!have_pointer && app->data.waylandData.pointer != NULL) {
            wl_pointer_release(app->data.waylandData.pointer);
            app->data.waylandData.pointer = NULL;
    }
}

static void wl_seat_name(void *data, struct wl_seat *wl_seat, const char *name)
{
    log_info("WAYLAND Seat name: %s", name);
}



static void pointer_enter_callback(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t surface_x, wl_fixed_t surface_y)
{

}

static void pointer_exit_callback(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface)
{

}

static void pointer_motion_callback(void *data, struct wl_pointer *wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y)
{

}

static void pointer_button_callback(void *data, struct wl_pointer *wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{

}


static void pointer_axis_callback(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value)
{

}

static void pointer_frame_callback(void *data, struct wl_pointer *wl_pointer)
{

}

static void pointer_axis_source_callback(void *data, struct wl_pointer *wl_pointer, uint32_t axis_source)
{

}

static void pointer_axis_stop_callback(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis)
{

}

static void pointer_axis_discrete_callback(void *data, struct wl_pointer *wl_pointer, uint32_t axis, int32_t discrete)
{

}

static void on_frame_configure(struct libdecor_frame *frame, struct libdecor_configuration *configuration, void *user_data)
{

    log_info("ON FRAME CONFIGURE\n");

    UnixApp *app = (UnixApp*)user_data;
    if (app == NULL || app->appType != UNIX_APP_WAYLAND)
    {
        return;
    }

    if (app->windowHandle == 0)
    {
        log_info("Window handle is 0");
        return;
    }

    UnixWindow* unixWindow = (UnixWindow*)app->windowHandle;

    int width, height;
	enum libdecor_window_state window_state;
	struct libdecor_state *state;

    if (!libdecor_configuration_get_window_state(configuration, &window_state))
    {
        window_state = LIBDECOR_WINDOW_STATE_NONE;
    }

    if (!libdecor_configuration_get_content_size(configuration, frame, &width, &height))
    {
        width = 800;
        height = 600;
    }

    state = libdecor_state_new(width, height);
    libdecor_frame_commit(frame, state, configuration);
	libdecor_state_free(state);

     clear_window((AppHandle)app, app->windowHandle);
    glViewport(0, 0, width, height);

    swap_window_buffers((AppHandle)app, app->windowHandle);

    wl_surface_commit(unixWindow->data.waylandData.wl_surface);
    wl_display_flush(app->data.waylandData.display);


    //libdecor_frame_commit(frame, configuration);
}

static void on_frame_close(struct libdecor_frame *frame, void *user_data)
{
    log_info("Window close requested.\n");


}

static void on_commit(struct libdecor_frame *frame, void *user_data)
{

    log_info("on commit\n");


    UnixApp *app = (UnixApp*)user_data;
    if (app == NULL || app->appType != UNIX_APP_WAYLAND)
    {
        return;
    }

    if (app->windowHandle == 0)
    {
        log_info("Window handle is 0");
        return;
    }

    UnixWindow* unixWindow = (UnixWindow*)app->windowHandle;

    wl_surface_commit(unixWindow->data.waylandData.wl_surface);


}

static void handle_error(struct libdecor *context, enum libdecor_error error, const char *message)
{
    log_error("Libdecor error: %s\n", message);
}

