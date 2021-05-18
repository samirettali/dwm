/* See LICENSE file for copyright and license details. */
#include <X11/XF86keysym.h>

/* appearance */
static const unsigned int borderpx       = 4;   /* border pixel of windows */
static const unsigned int snap           = 8;   /* snap pixel */
static const unsigned int gappx          = 4;   /* gaps size */
static const unsigned int rmaster        = 1;   /* 1 means master-area is initially on the right */
static const unsigned int systraypinning = 0;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayonleft  = 0;   /* 0: systray in the right corner, >0: systray on left of status text */
static const unsigned int systrayspacing = 2;   /* systray spacing */
static const int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/
static const int attachbelow             = 1;   /* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/
static const int showsystray             = 1;   /* 0 means no systray */
static const int showbar                 = 1;   /* 0 means no bar */
static const int topbar                  = 1;   /* 0 means bottom bar */
static const char *fonts[]               = { "JetBrainsMono Nerd Font Mono:size=10" };
static const char dmenufont[]            = "JetBrainsMono Nerd Font Mono:size=10";
static char normbgcolor[]           = "#1d1f21";
static char normbordercolor[]       = "#282a2e";
static char normfgcolor[]           = "#c5c8c6";
static char selfgcolor[]            = "#f0c674";
static char selbordercolor[]        = "#f0c674";
static char selbgcolor[]            = "#1d1f21";
static char *colors[][3]            = {
  /*              fg             bg           border   */
  [SchemeNorm] = { normfgcolor, normbgcolor, normbordercolor},
  [SchemeSel]  = { selfgcolor,  selbgcolor,  selbordercolor  },
};

/* #define NUMCOLORS 12
static const char colors[NUMCOLORS][ColLast][9] = {
  // border foreground background
  { "#282a2e", "#373b41", "#1d1f21" }, // 1 = normal (grey on black)
  { "#f0c674", "#c5c8c6", "#1d1f21" }, // 2 = selected (white on black)
  { "#dc322f", "#1d1f21", "#f0c674" }, // 3 = urgent (black on yellow)
  { "#282a2e", "#282a2e", "#1d1f21" }, // 4 = darkgrey on black (for glyphs)
  { "#282a2e", "#1d1f21", "#282a2e" }, // 5 = black on darkgrey (for glyphs)
  { "#282a2e", "#cc6666", "#1d1f21" }, // 6 = red on black
  { "#282a2e", "#b5bd68", "#1d1f21" }, // 7 = green on black
  { "#282a2e", "#de935f", "#1d1f21" }, // 8 = orange on black
  { "#282a2e", "#f0c674", "#282a2e" }, // 9 = yellow on darkgrey
  { "#282a2e", "#81a2be", "#282a2e" }, // A = blue on darkgrey
  { "#282a2e", "#b294bb", "#282a2e" }, // B = magenta on darkgrey
  { "#282a2e", "#8abeb7", "#282a2e" }, // C = cyan on darkgrey
}; */

/* base00: "#1d1f21"
base01: "#282a2e"
base02: "#373b41"
base03: "#969896"
base04: "#b4b7b4"
base05: "#c5c8c6"
base06: "#e0e0e0"
base07: "#ffffff"
base08: "#cc6666"
base09: "#de935f"
base0A: "#f0c674"
base0B: "#b5bd68"
base0C: "#8abeb7"
base0D: "#81a2be"
base0E: "#b294bb"
base0F: "#a3685a" */

/* tagging */
#define MAX_TAGNAME_LEN 14
#define MAX_TAGLEN 16
#define TAG_PREPEND "%1i:"
static char tags[][MAX_TAGLEN] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
// static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

#include "movestack.c"
#include "focusurgent.c"

// Crypto Wallets Notification
static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class         instance    title       tags mask     isfloating   monitor */
	{ "Gimp",        NULL,       NULL,       0,            1,           -1 },
	{ "Firefox",     NULL,       NULL,       1 << 8,       0,           -1 },
	{ "Discord",     NULL,       NULL,       1 << 8,       0,           -1 },
	{ "Slack",       NULL,       NULL,       1 << 8,       0,           -1 },
	{ "Pcmanfm",     NULL,       NULL,       0,            1,           -1 },
	{ "Pavucontrol", NULL,       NULL,       0,            1,           -1 },
	{ "feh",         NULL,       NULL,       0,            1,           -1 },
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
	{ "[D]",      deck },
	{ "|||",      col },
};

/* key definitions */
#define MODKEY Mod1Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbgcolor, "-sf", selfgcolor, NULL };
static const char *termcmd[]  = { "st", NULL };

static const char *lockcmd[]      = { "xset", "s", "activate", NULL };
static const char *filemancmd[]   = { "pcmanfm", NULL };
static const char *pickercmd[]    = { "xcolor", "|", "tr", "-d", "'\n'", "|", "xclip", "-selection", "clip", NULL };
static const char *togglekbdmap[] = { "setxkbmap", "-query", "|", "grep", "-q", "'us'", "&&", "setxkbmap", "fr", "||", "setxkbmap", "us", NULL};

static const char *upvol[]   = { "amixer", "set", "Master", "5%+",     NULL };
static const char *downvol[] = { "amixer", "set", "Master", "5%-",     NULL };
static const char *mutevol[] = { "amixer", "set", "Master", "toggle",     NULL };


static Key keys[] = {
	/* modifier                key        function        argument */
	{ MODKEY,                  XK_p,      spawn,          {.v = dmenucmd } },
	{ MODKEY,                  XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY,                  XK_y,      spawn,          {.v = pickercmd } },
	{ MODKEY,                  XK_b,      togglebar,      {0} },
	{ MODKEY|ShiftMask,        XK_j,      movestack,      {.i = +1 } },
	{ MODKEY|ShiftMask,        XK_k,      movestack,      {.i = -1 } },
	{ MODKEY,                  XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                  XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY,                  XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                  XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                  XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                  XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY|ShiftMask,        XK_Return, zoom,           {0} },
	{ MODKEY,                  XK_z,      view,           {0} },
	{ MODKEY|ShiftMask,        XK_c,      killclient,     {0} },
	{ MODKEY,                  XK_t,      setlayout,      {.v = &layouts[0]} },
	// { MODKEY,                  XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                  XK_m,      setlayout,      {.v = &layouts[2]} },
  // { MODKEY,                  XK_u,      setlayout,      {.v = &layouts[3]} },
  { MODKEY,                  XK_c,      setlayout,      {.v = &layouts[4]} },
	{ MODKEY,                  XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,        XK_space,  togglefloating, {0} },
	{ MODKEY,                  XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,        XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                  XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                  XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,        XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,        XK_period, tagmon,         {.i = +1 } },
	{ MODKEY,                  XK_n,      nametag,        {0} },
	TAGKEYS(                   XK_1,                      0)
	TAGKEYS(                   XK_2,                      1)
	TAGKEYS(                   XK_3,                      2)
	TAGKEYS(                   XK_4,                      3)
	TAGKEYS(                   XK_5,                      4)
	TAGKEYS(                   XK_6,                      5)
	TAGKEYS(                   XK_7,                      6)
	TAGKEYS(                   XK_8,                      7)
	TAGKEYS(                   XK_9,                      8)
	{ MODKEY,                  XK_r,      togglermaster,  {0} },
	{ MODKEY,                  XK_u,      focusurgent,    {0} },
	{ MODKEY|ShiftMask,        XK_q,      quit,           {0} },
	{ MODKEY|ShiftMask,        XK_l,      spawn,          {.v = lockcmd } },
	{ ControlMask|ShiftMask,   XK_l,      spawn,          {.v = togglekbdmap } },
	{ MODKEY,                  XK_f,      spawn,          {.v = filemancmd } },
	{ 0,                       XF86XK_AudioMute,        spawn, {.v = mutevol } },
	{ 0,                       XF86XK_AudioLowerVolume, spawn, {.v = downvol } },
	{ 0,                       XF86XK_AudioRaiseVolume, spawn, {.v = upvol   } },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

