/* See LICENSE file for license details. */
#include <sys/wait.h>
#include <unistd.h>
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include "xwm.h"
#include "config.h"

#define UNUSED(x) (void)(x)

static xcb_connection_t * dpy;
static xcb_screen_t     * scre;
static xcb_drawable_t     win;
static uint32_t           values[3];

static void killclient(char **com) {
    UNUSED(com);
    xcb_kill_client(dpy, win);
}

static void closewm(char **com) {
    UNUSED(com);
    if (dpy != NULL) {
        xcb_disconnect(dpy);
    }
}

static void spawn(char **com) {
    if (fork() == 0) {
        if (dpy != NULL) {
            close(scre->root);
        }
        setsid();
        if (fork() != 0) {
            _exit(0);
        }
        execvp((char*)com[0], (char**)com);
        _exit(0);
    }
    wait(NULL);
}

static void handleButtonPress(xcb_generic_event_t * ev) {
    xcb_button_press_event_t  * e = (xcb_button_press_event_t *) ev;
    win = e->child;
    values[0] = XCB_STACK_MODE_ABOVE;
    xcb_configure_window(dpy, win, XCB_CONFIG_WINDOW_STACK_MODE, values);
    values[2] = ((1 == e->detail) ? 1 : ((win != 0) ? 3 : 0 ));
    xcb_grab_pointer(dpy, 0, scre->root, XCB_EVENT_MASK_BUTTON_RELEASE
        | XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_POINTER_MOTION_HINT,
        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
        scre->root, XCB_NONE, XCB_CURRENT_TIME);
}

static void handleMotionNotify(xcb_generic_event_t * ev) {
    UNUSED(ev);
    xcb_query_pointer_cookie_t coord = xcb_query_pointer(dpy, scre->root);
    xcb_query_pointer_reply_t * poin = xcb_query_pointer_reply(dpy, coord, 0);
    if ((values[2] == (uint32_t)(1)) && (win != 0)) {
        xcb_get_geometry_cookie_t geom_now = xcb_get_geometry(dpy, win);
        xcb_get_geometry_reply_t * geom = xcb_get_geometry_reply(dpy, geom_now, NULL);
        uint16_t geom_x = geom->width + (2 * BORDER_WIDTH);
        uint16_t geom_y = geom->height + (2 * BORDER_WIDTH);
        values[0] = ((poin->root_x + geom_x) > scre->width_in_pixels) ?
            (scre->width_in_pixels - geom_x) : poin->root_x;
        values[1] = ((poin->root_y + geom_y) > scre->height_in_pixels) ?
            (scre->height_in_pixels - geom_y) : poin->root_y;
        xcb_configure_window(dpy, win, XCB_CONFIG_WINDOW_X
            | XCB_CONFIG_WINDOW_Y, values);
    } else if ((values[2] == (uint32_t)(3)) && (win != 0)) {
        xcb_get_geometry_cookie_t geom_now = xcb_get_geometry(dpy, win);
        xcb_get_geometry_reply_t* geom = xcb_get_geometry_reply(dpy, geom_now, NULL);
        if (!((poin->root_x <= geom->x) || (poin->root_y <= geom->y))) {
            values[0] = poin->root_x - geom->x - BORDER_WIDTH;
            values[1] = poin->root_y - geom->y - BORDER_WIDTH;
            if ((values[0] >= (uint32_t)(WINDOW_MIN_X)) &&
                (values[1] >= (uint32_t)(WINDOW_MIN_Y))) {
                xcb_configure_window(dpy, win, XCB_CONFIG_WINDOW_WIDTH
                    | XCB_CONFIG_WINDOW_HEIGHT, values);
            }
        }
    } else {}
}

static xcb_keycode_t * xcb_get_keycodes(xcb_keysym_t keysym) {
    xcb_key_symbols_t * keysyms = xcb_key_symbols_alloc(dpy);
    xcb_keycode_t     * keycode;
    keycode = (!(keysyms) ? NULL : xcb_key_symbols_get_keycode(keysyms, keysym));
    xcb_key_symbols_free(keysyms);
    return keycode;
}

static xcb_keysym_t xcb_get_keysym(xcb_keycode_t keycode) {
    xcb_key_symbols_t * keysyms = xcb_key_symbols_alloc(dpy);
    xcb_keysym_t        keysym;
    keysym = (!(keysyms) ? 0 : xcb_key_symbols_get_keysym(keysyms, keycode, 0));
    xcb_key_symbols_free(keysyms);
    return keysym;
}

static void setFocus(xcb_drawable_t window) {
    if ((window != 0) && (window != scre->root)) {
        xcb_set_input_focus(dpy, XCB_INPUT_FOCUS_POINTER_ROOT, window,
            XCB_CURRENT_TIME);
    }
}

static void setBorderColor(xcb_window_t window, int focus) {
    if ((BORDER_WIDTH > 0) && (scre->root != window) && (0 != window)) {
        uint32_t vals[1];
        vals[0] = focus ? BORDER_COLOR_FOCUSED : BORDER_COLOR_UNFOCUSED;
        xcb_change_window_attributes(dpy, window, XCB_CW_BORDER_PIXEL, vals);
        xcb_flush(dpy);
    }
}

static void setBorderWidth(xcb_window_t window) {
    if ((BORDER_WIDTH > 0) && (scre->root != window) && (0 != window)) {
        uint32_t vals[2];
        vals[0] = BORDER_WIDTH;
        xcb_configure_window(dpy, window, XCB_CONFIG_WINDOW_BORDER_WIDTH, vals);
        xcb_flush(dpy);
    }
}

static void setWindowDimensions(xcb_window_t window) {
    if ((scre->root != window) && (0 != window)) {
        uint32_t vals[2];
        vals[0] = WINDOW_X;
        vals[1] = WINDOW_Y;
        xcb_configure_window(dpy, window, XCB_CONFIG_WINDOW_WIDTH |
            XCB_CONFIG_WINDOW_HEIGHT, vals);
        xcb_flush(dpy);
    }
}

static void setWindowPosition(xcb_window_t window) {
    if ((scre->root != window) && (0 != window)) {
        uint32_t vals[2];
        vals[0] = (scre->width_in_pixels / 2) - (WINDOW_X / 2);
        vals[1] = (scre->height_in_pixels / 2) - (WINDOW_Y / 2);
        xcb_configure_window(dpy, window, XCB_CONFIG_WINDOW_X |
            XCB_CONFIG_WINDOW_Y, vals);
        xcb_flush(dpy);
    }
}

static void handleKeyPress(xcb_generic_event_t * ev) {
    xcb_key_press_event_t * e = ( xcb_key_press_event_t *) ev;
    xcb_keysym_t keysym = xcb_get_keysym(e->detail);
    win = e->child;
    int key_table_size = sizeof(keys) / sizeof(*keys);
    for (int i = 0; i < key_table_size; ++i) {
        if ((keys[i].keysym == keysym) && (keys[i].mod == e->state)) {
            keys[i].func(keys[i].com);
        }
    }
}

static void handleEnterNotify(xcb_generic_event_t * ev) {
    xcb_enter_notify_event_t * e = ( xcb_enter_notify_event_t *) ev;
    setFocus(e->event);
}

static void handleButtonRelease(xcb_generic_event_t * ev) {
    UNUSED(ev);
    xcb_ungrab_pointer(dpy, XCB_CURRENT_TIME);
}

static void handleDestroyNotify(xcb_generic_event_t * ev) {
    xcb_destroy_notify_event_t * e = (xcb_destroy_notify_event_t *) ev;
    xcb_kill_client(dpy, e->window);
}

static void handleFocusIn(xcb_generic_event_t * ev) {
    xcb_focus_in_event_t * e = (xcb_focus_in_event_t *) ev;
    setBorderColor(e->event, 1);
}

static void handleFocusOut(xcb_generic_event_t * ev) {
    xcb_focus_out_event_t * e = (xcb_focus_out_event_t *) ev;
    setBorderColor(e->event, 0);
}

static void handleMapRequest(xcb_generic_event_t * ev) {
    xcb_map_request_event_t * e = (xcb_map_request_event_t *) ev;
    xcb_map_window(dpy, e->window);
    setWindowDimensions(e->window);
    setWindowPosition(e->window);
    setBorderWidth(e->window);
    values[0] = XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_FOCUS_CHANGE;
    xcb_change_window_attributes_checked(dpy, e->window,
        XCB_CW_EVENT_MASK, values);
    setFocus(e->window);
}

static int eventHandler(void) {
    int ret = xcb_connection_has_error(dpy);
    if (ret == 0) {
        xcb_generic_event_t * ev = xcb_wait_for_event(dpy);
        handler_func_t * handler;
        for (handler = handler_funs; handler->func != NULL; handler++) {
            if ((ev->response_type & ~0x80) == handler->request) {
                handler->func(ev);
            }
        }
    }
    xcb_flush(dpy);
    return ret;
}

static void setup(void) {
    /* subscribe to events */
    values[0] = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
        | XCB_EVENT_MASK_STRUCTURE_NOTIFY
        | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
        | XCB_EVENT_MASK_PROPERTY_CHANGE;
    xcb_change_window_attributes_checked(dpy, scre->root,
        XCB_CW_EVENT_MASK, values);
    /* grab keys */
    xcb_ungrab_key(dpy, XCB_GRAB_ANY, scre->root, XCB_MOD_MASK_ANY);
    int key_table_size = sizeof(keys) / sizeof(*keys);
    for (int i = 0; i < key_table_size; ++i) {
        xcb_keycode_t * keycode = xcb_get_keycodes(keys[i].keysym);
        if (keycode != NULL) {
            xcb_grab_key(dpy, 1, scre->root, keys[i].mod, *keycode,
                XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC );
        }
    }
    xcb_flush(dpy);
    /* grab buttons */
    xcb_grab_button(dpy, 0, scre->root, XCB_EVENT_MASK_BUTTON_PRESS |
        XCB_EVENT_MASK_BUTTON_RELEASE, XCB_GRAB_MODE_ASYNC,
        XCB_GRAB_MODE_ASYNC, scre->root, XCB_NONE, 1, MOD1);
    xcb_grab_button(dpy, 0, scre->root, XCB_EVENT_MASK_BUTTON_PRESS |
        XCB_EVENT_MASK_BUTTON_RELEASE, XCB_GRAB_MODE_ASYNC,
        XCB_GRAB_MODE_ASYNC, scre->root, XCB_NONE, 3, MOD1);
    xcb_flush(dpy);
}

static int die(char * errstr) {
    size_t n = 0;
    char * p = errstr;
    while ((* (p++)) != 0) {
        ++n;
    }
    ssize_t o = write(STDERR_FILENO, errstr, n);
    int ret = 1;
    if (o < 0) {
        ret = -1;
    }
    return ret;
}

static int strcmp_c(char * str1, char * str2) {
    char * c1 = str1;
    char * c2 = str2;
    while ((* c1) && ((* c1) == (* c2))) {
        ++c1;
        ++c2;
    }
    int n = (* c1) - (* c2);
    return n;
}

int main(int argc, char * argv[]) {
    int ret = 0;
    if ((argc == 2) && (strcmp_c("-v", argv[1]) == 0)) {
        ret = die("xwm-0.1.4, © 2020 Michael Czigler, see LICENSE for details\n");
    }
    if ((ret == 0) && (argc != 1)) {
        ret = die("usage: xwm [-v]\n");
    }
    if (ret == 0) {
        dpy = xcb_connect(NULL, NULL);
        ret = xcb_connection_has_error(dpy);
        if (ret > 0) {
            ret = die("xcb_connection_has_error\n");
        }
    }
    if (ret == 0) {
        scre = xcb_setup_roots_iterator(xcb_get_setup(dpy)).data;
        setup();
    }
    while (ret == 0) {
        ret = eventHandler();
    }
    return ret;
}
