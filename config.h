/* default xwm super keys*/
#define MOD1 XCB_MOD_MASK_4     /* mod key 1=Alt, 4=Start */
#define MOD2 XCB_MOD_MASK_SHIFT /* shift key */

/* shortcut commands */
static char * termcmd[] = { "st", NULL };
static char * menucmd[] = { "dmenu_run", NULL };
static char * browcmd[] = { "surf", NULL };

/* shortcut keys */
static Key    keys[] = {
    { MOD1,      0x0062, spawn,      browcmd },
    { MOD1,      0xff0d, spawn,      termcmd },
    { MOD1,      0x0020, spawn,      menucmd },
    { MOD1,      0x0071, killclient, NULL },
    { MOD1|MOD2, 0x0071, closewm,    NULL }
};

