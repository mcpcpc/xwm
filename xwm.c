/* See LICENSE file for license details. */
#include <stdarg.h>
#include <unistd.h>
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include "xwm.h"
#include "config.h"

static xcb_connection_t * dpy;
static xcb_screen_t     * scre;
static xcb_drawable_t     win;
static xcb_drawable_t     root;
static uint32_t           values[3];

static void killclient(char **com) {
    xcb_kill_client(dpy, win);
}

static void closewm(char **com) {
    if (dpy != NULL) {
        xcb_disconnect(dpy);
    }
}

static void spawn(char **com) {
    if (fork() == 0) {
        if (dpy != NULL) {
            close(root);
        }
        setsid();
        execvp((char*)com[0], (char**)com);
    }
}

static void eventHandlerButtonPress(xcb_generic_event_t * ev) {
    xcb_button_press_event_t  * e = (xcb_button_press_event_t *) ev;
    win = e->child;
    values[0] = XCB_STACK_MODE_ABOVE;
    xcb_configure_window(dpy, win, XCB_CONFIG_WINDOW_STACK_MODE, values);
    xcb_get_geometry_cookie_t geom_now = xcb_get_geometry(dpy, win);
    xcb_get_geometry_reply_t * geom = xcb_get_geometry_reply(dpy, geom_now, NULL);
    if (1 == e->detail) {
        values[2] = 1;
        xcb_warp_pointer(dpy, XCB_NONE, win, 0, 0, 0, 0, 1, 1);
    } else if (win != 0) {
        values[2] = 3;
        xcb_warp_pointer(dpy, XCB_NONE, win, 0, 0, 0, 0, geom->width, geom->height);
    }
    else {}
    xcb_grab_pointer(dpy, 0, root, XCB_EVENT_MASK_BUTTON_RELEASE
        | XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_POINTER_MOTION_HINT,
        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, root, XCB_NONE, XCB_CURRENT_TIME);
}

static void eventHandlerMotionNotify(xcb_generic_event_t * ev) {
    xcb_query_pointer_cookie_t coord = xcb_query_pointer(dpy, root);
    xcb_query_pointer_reply_t * poin = xcb_query_pointer_reply(dpy, coord, 0);
    uint32_t val[2] = {1, 3};
    if ((values[2] == val[0]) && (win != 0)) {
        xcb_get_geometry_cookie_t geom_now = xcb_get_geometry(dpy, win);
        xcb_get_geometry_reply_t * geom = xcb_get_geometry_reply(dpy, geom_now, NULL);
        values[0] = ((poin->root_x + geom->width) > scre->width_in_pixels) ?
            (scre->width_in_pixels - geom->width) : poin->root_x;
        values[1] = ((poin->root_y + geom->height) > scre->height_in_pixels) ?
            (scre->height_in_pixels - geom->height) : poin->root_y;
        xcb_configure_window(dpy, win, XCB_CONFIG_WINDOW_X
            | XCB_CONFIG_WINDOW_Y, values);
    }
    if ((values[2] == val[1]) && (win != 0)) {
        xcb_get_geometry_cookie_t geom_now = xcb_get_geometry(dpy, win);
        xcb_get_geometry_reply_t* geom = xcb_get_geometry_reply(dpy, geom_now, NULL);
        values[0] = poin->root_x - geom->x;
        values[1] = poin->root_y - geom->y;
        xcb_configure_window(dpy, win, XCB_CONFIG_WINDOW_WIDTH
            | XCB_CONFIG_WINDOW_HEIGHT, values);
    }
}

static xcb_keycode_t * xcb_get_keycodes(xcb_keysym_t keysym) {
    xcb_key_symbols_t * keysyms = xcb_key_symbols_alloc(dpy);
    xcb_keycode_t     * keycode;
    if (!(keysyms)) {
        keycode = NULL;
    } else {
        keycode = xcb_key_symbols_get_keycode(keysyms, keysym);
    }
    xcb_key_symbols_free(keysyms);
    return keycode;
}

static xcb_keysym_t xcb_get_keysym(xcb_keycode_t keycode) {
    xcb_key_symbols_t * keysyms = xcb_key_symbols_alloc(dpy);
    xcb_keysym_t        keysym;
    if (!(keysyms)) {
        keysym = 0;
    } else {
        keysym = xcb_key_symbols_get_keysym(keysyms, keycode, 0);
    }
    xcb_key_symbols_free(keysyms);
    return keysym;
}

static void eventHandlerKeyPress(xcb_generic_event_t * ev) {
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

static void eventHandlerEnterNotify(xcb_generic_event_t * ev) {
    xcb_enter_notify_event_t * e = ( xcb_enter_notify_event_t *) ev;
    xcb_drawable_t win_e = e->event;
    if ((win_e != 0) && (win_e != root)) {
        xcb_set_input_focus(dpy, XCB_INPUT_FOCUS_POINTER_ROOT, win_e,
            XCB_CURRENT_TIME);
    }
}

static void eventHandlerButtonRelease(xcb_generic_event_t * ev) {
    xcb_ungrab_pointer(dpy, XCB_CURRENT_TIME);
}

static void eventHandlerDestroyNotify(xcb_generic_event_t * ev) {
    xcb_destroy_notify_event_t * e = (xcb_destroy_notify_event_t *) ev;
    xcb_kill_client(dpy, e->window);
}

static int eventHandler(void) {
    int ret = xcb_connection_has_error(dpy);
    if (ret == 0) {
        xcb_generic_event_t * ev = xcb_wait_for_event(dpy);
        for (handler_func_t * handler = handler_funs; handler->func != NULL; handler++) {
            if ((ev->response_type & ~0x80) == handler->request) {
                handler->func(ev);
            }
        }
    }
    xcb_flush(dpy);
    return ret;
}

static void grabKeys(void) {
    xcb_ungrab_key(dpy, XCB_GRAB_ANY, root, XCB_MOD_MASK_ANY);
    int key_table_size = sizeof(keys) / sizeof(*keys);
    for (int i = 0; i < key_table_size; ++i) {
        xcb_keycode_t * keycode = xcb_get_keycodes(keys[i].keysym);
        if (keycode != NULL) {
            xcb_grab_key(dpy, 1, root, keys[i].mod, *keycode,
                XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC );
        }
    }
    xcb_flush(dpy);
}

static void grabButtons(void) {
    xcb_grab_button(dpy, 0, root, XCB_EVENT_MASK_BUTTON_PRESS |
        XCB_EVENT_MASK_BUTTON_RELEASE, XCB_GRAB_MODE_ASYNC,
        XCB_GRAB_MODE_ASYNC, root, XCB_NONE, 1, MOD1);
    xcb_grab_button(dpy, 0, root, XCB_EVENT_MASK_BUTTON_PRESS |
        XCB_EVENT_MASK_BUTTON_RELEASE, XCB_GRAB_MODE_ASYNC,
        XCB_GRAB_MODE_ASYNC, root, XCB_NONE, 3, MOD1);
    xcb_flush(dpy);
}

static int die(char * errstr) {
    size_t n = 0;
    char * p = errstr;
    while ((* (p++)) != 0) {
        ++n;
    }
    write(STDERR_FILENO, errstr, n);
    return 1;
}

static int compare(char * str1, char * str2) {
    while ((* str1) && ((* str1) == (* str2))) {
        ++str1;
        ++str2;
    }
    int n = (* str1) - (* str2);
    return n;
}

int main(int argc, char * argv[]) {
    int ret = 0;
    if ((argc == 2) && (compare("-v", argv[1]) == 0)) {
        ret = die("xwm-0.0.3, © 2020 Michael Czigler, see LICENSE for details\n");
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
        root = scre->root;
        grabKeys();
        grabButtons();
    }
    while (ret == 0) {
        ret = eventHandler();
    }
    return ret;
}

