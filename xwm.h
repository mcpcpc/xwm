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
    void (*func)(xcb_generic_event_t * ev);
} handler_func_t;

/* convert keycode to keysym and back */
static xcb_keycode_t * xcb_get_keycodes(xcb_keysym_t keysym);
static xcb_keysym_t    xcb_get_keysym(xcb_keycode_t keycode);

/* user defined command actions */
static void killclient(char **com);
static void spawn(char **com);
static void closewm(char ** com);

/* event hander actions */
static int eventHandler(void);
static void eventHandlerMotionNotify(xcb_generic_event_t * ev);
static void eventHandlerEnterNotify(xcb_generic_event_t * ev);
static void eventHandlerButtonPress(xcb_generic_event_t * ev);
static void eventHandlerButtonRelease(xcb_generic_event_t * ev);
static void eventHandlerKeyPress(xcb_generic_event_t * ev);
static void eventHandlerDestroyNotify(xcb_generic_event_t * ev);
static handler_func_t handler_funs[] = {
    { XCB_MOTION_NOTIFY,  eventHandlerMotionNotify },
    { XCB_ENTER_NOTIFY,   eventHandlerEnterNotify },
    { XCB_BUTTON_PRESS,   eventHandlerButtonPress },
    { XCB_BUTTON_RELEASE, eventHandlerButtonRelease },
    { XCB_KEY_PRESS,      eventHandlerKeyPress },
    { XCB_DESTROY_NOTIFY, eventHandlerDestroyNotify },
    { XCB_NONE,           NULL }
};

/* intialize */
static void grabKeys(void);
static void grabButtons(void);

/* error handling */
static int die(char * errstr);

