#ifndef CWINDOW_H
#define CWINDOW_H
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Browser.H>
#include "BastelUtils.h"




#define DIALOGBOX       eDialogBox,
#define BUTTON          eButton,
#define EDITTEXT        eEditor,
#define CONTROL         eControls,
#define AUTORADIOBUTTON eOptions,
#define CTEXT           eText,
#define LTEXT           eText,
#define RTEXT           eText,
#define LISTBOX         eList,
#define CHECKBOX        eOptions,
#define PUSHBUTTON      eButton,
#define CUSTOMITEM      eCustomItem,

//extern BITMAPINFOHEADER* g_hSysIcons;

#define IDC_DialogBox     2000-8

#ifndef WIN32



// DialogBox relevants
#define IDOK              2000-1
#define IDCANCEL          2000-2
#define IDABORT           2000-3
#define IDRETRY           2000-4
#define IDIGNORE          2000-5
#define IDYES             2000-6
#define IDNO              2000-7

#define MB_OK                       0x00000000L
#define MB_OKCANCEL                 0x00000001L
#define MB_ABORTRETRYIGNORE         0x00000002L
#define MB_YESNOCANCEL              0x00000003L
#define MB_YESNO                    0x00000004L
#define MB_RETRYCANCEL              0x00000005L


#define MB_ICONHAND                 0x00000010L
#define MB_ICONQUESTION             0x00000020L
#define MB_ICONEXCLAMATION          0x00000030L
#define MB_ICONASTERISK             0x00000040L

#define MB_DEFBUTTON1               0x00000100//  The first button is the default.
#define MB_DEFBUTTON2               0x00000200
#define MB_DEFBUTTON3               0x00000300

// Window relevants

//typedef CWindow  * HWND        ;


//typedef unsigned int    COLORREF    ;
//typedef Fl_Widget * HWND;
//typedef void * HDC;





#define WM_COMMAND            0x0227
#define WM_NV_UPDATE          0x0228
#define SHIFT_PRESSED     1
#define RIGHT_ALT_PRESSED 2
#define LEFT_ALT_PRESSED  8
#define LEFT_CTRL_PRESSED 4
#define WM_LBUTTONDOWN        0x0201


#undef  BS_AUTORADIOBUTTON
#undef  BS_PUSHLIKE
#undef  BS_AUTOCHECKBOX
#undef  BS_MULTILINE
#undef  BS_FLAT
#undef  ES_AUTOHSCROLL
#undef  ES_READONLY
#undef  SS_ETCHEDFRAME
#undef  WS_EX_CLIENTEDGE
#undef  SS_GRAYFRAME
#undef  LBS_SORT
#undef  LBS_USETABSTOPS
#undef  SS_LEFT
#undef  SS_CENTER
#undef  SS_RIGHT

#define ES_TRANSPARENT      0x00000010L
#define ES_RIGHT            0x00000008L
#define ES_LEFT             0x00000004L
#define ES_CENTER           0x00000002L
#define ES_MULTILINE        0x00000001L

#define WS_MINIMIZE         0x20000000L
#define WS_VISIBLE          0x10000000L
#define WS_DISABLED         0x08000000L
#define WS_CLIPSIBLINGS     0x04000000L
#define WS_CLIPCHILDREN     0x02000000L
#define WS_MAXIMIZE         0x01000000L
#define WS_BORDER           0x00800000L
#define WS_DLGFRAME         0x00400000L
#define WS_CLIENTEDGE       0x00C00000L
#define WS_VSCROLL          0x00200000L
#define WS_HSCROLL          0x00100000L
#define WS_SYSMENU          0x00080000L
#define WS_THICKFRAME       0x00040000L
#define WS_GROUP            0x00020000L
#define WS_TABSTOP          0x00010000L

#define BS_AUTORADIOBUTTON  0x1
#define BS_PUSHLIKE           0
#define BS_AUTOCHECKBOX     0x4
#define BS_MULTILINE          0
#define BS_FLAT             0x10
#define ES_AUTOHSCROLL      0x20
#define ES_READONLY         0x40
#define SS_ETCHEDFRAME     WS_THICKFRAME//|WS_DISABLED
#define WS_EX_CLIENTEDGE   WS_THICKFRAME//|WS_DISABLED
#define SS_GRAYFRAME       WS_DLGFRAME  //|WS_DISABLED
#define LBS_SORT             0
#define LBS_USETABSTOPS      0

#define SS_LEFT             ES_LEFT
#define SS_CENTER           ES_CENTER
#define SS_RIGHT            ES_RIGHT
#else
#define WM_NV_UPDATE          WM_USER +1
#define WS_CLIENTEDGE       0x00C00000L
#endif
//-------------------------------------------------------------
extern BOOL  ifEscape(void);
extern BOOL delay(int tx);
//-------------------------------------------------------------


typedef struct tMessage
{
  Fl_Widget * receiver;
  int         Code;
  int         Value;
} tMessage;
#endif // CWINDOW_H
