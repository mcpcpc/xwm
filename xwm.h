/* See LICENSE file for copyright and license details. */

/* typedefs */
typedef struct {
    unsigned int mod;
    xcb_keysym_t keysym;
    void (*func)(char **com);
    char **com;
} Key;

typedef struct {
    uint32_t request;
    void (*func)(xcb_generic_event_t *ev);
} handler_func_t;

/* convert keycode to keysym and back */
static xcb_keycode_t *xcb_get_keycodes(xcb_keysym_t keysym);
static xcb_keysym_t   xcb_get_keysym(xcb_keycode_t keycode);

/* user defined command actions */
static void xwm_client_kill(char **com);
static void xwm_client_spawn(char **com);
static void xwm_client_maximize(char **com);
static void xwm_disconnect(char **com);

/* window behavior */
static void set_focus(xcb_drawable_t window);
static void set_focus_color(xcb_drawable_t window, int focus);

/* event hander actions */
static int event_handler(void);
static void handle_motion_notify(xcb_generic_event_t *ev);
static void handle_enter_notify(xcb_generic_event_t *ev);
static void handle_destroy_notify(xcb_generic_event_t *ev);
static void handle_button_press(xcb_generic_event_t *ev);
static void handle_Button_release(xcb_generic_event_t *ev);
static void handle_key_press(xcb_generic_event_t *ev);
static void handle_map_request(xcb_generic_event_t *ev);
static void handle_focus_in(xcb_generic_event_t *ev);
static void handle_focus_out(xcb_generic_event_t *ev);
static handler_func_t handler_funs[] = {
    { XCB_MOTION_NOTIFY,  handle_motion_notify },
    { XCB_ENTER_NOTIFY,   handle_enter_notify },
    { XCB_DESTROY_NOTIFY, handle_destroy_notify },
    { XCB_BUTTON_PRESS,   handle_button_press },
    { XCB_BUTTON_RELEASE, handle_Button_release },
    { XCB_KEY_PRESS,      handle_key_press },
    { XCB_MAP_REQUEST,    handle_map_request },
    { XCB_FOCUS_IN,       handle_focus_in },
    { XCB_FOCUS_OUT,      handle_focus_out },
    { XCB_NONE,           NULL }
};

/* intialize */
static void xwm_setup(void);

/* error handling & misc. */
static int xwm_die(char *errstr);
static int xwm_strcmp(char *str1, char *str2);
