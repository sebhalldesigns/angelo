#include <stdio.h>  
#include <angelo.h>

int main() {
    printf("Hello, World!\n");

    AppHandle_opt app = create_app("Angelo Test");
    if (app.is_some) {
        printf("App created successfully!\n");
    } else {
        printf("Failed to create app!\n");
        return -1;
    }
    
    WindowHandle_opt window = create_window(800, 600, "Angelo Test");
    if (window.is_some) {
        printf("Window created successfully!\n");
    } else {
        printf("Failed to create window!\n");
    }

    
    return run_app(app.value);
}