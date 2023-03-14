#ifndef CFLTKDIALOG_H
#define CFLTKDIALOG_H
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Browser.H>
#include "FltkWindow.h"
#include "BastelUtils.h"


typedef struct tButtonText
{
  int Image;
  int ccode;
  const char * txt;
}tButtonText;

//-------------------------------------------------------------


typedef struct
{
 int   type;
 char  text[32];
 int   idc;
 int   left;
 int   top;
 int   width;
 int   height;
 unsigned int   style;

 Fl_Widget * pWndObject;

}tDialogCtlItem;

//-------------------------------------------------------------
#ifndef CFBWindow

enum ClassType{eFBWindow=1,eAppWindow,eFBStaticWindow,eFBButtonWindow,eClientWindow,eControls,eButton,eOptions,eEdit,eText,eSliderCtl,
eTabClient,eTabFrame,eLowTabClient,eEditor,eSyEditor,eShowText,eHighTabClient,
eTreeView,eDialogBox,eCustomItem,
eList,eBuildPoject,eExecute,ePlayer,eDebugger,eBrowser,eListView,eListLast,
ePlayerBar,eBottomTabFrame,eProgress,eTTyTerminal,
eNVariable

};
#endif
class CFltkDialogBox : public Fl_Window //CWindow
{
public:
  CFltkDialogBox(int X, int Y, int W, int H,const char *L);
  virtual ~CFltkDialogBox();
  BITMAPINFOHEADER * m_pbmp;
  BOOL               m_GetBack;
  tDialogCtlItem   * m_pDialogCtlItem;
  int                m_NumDialogCtlItem;
  int                m_retcode;
  int                m_IconIndex;
  int                IsStillVisible(){return (m_pbmp!=NULL);};
  virtual int        InitWindow(Fl_Widget  * pParent,int ID,int left,int top,int width, int height,int Style);
  virtual void       draw();//  Paint(PAINTSTRUCT * pps,unsigned int wParam, unsigned int lParam);
  BOOL               InvalidateRect(const RECT *lpRect, BOOL bErase){redraw();return 0;};
//  virtual LPSTR      GetText(void){return m_szTitle;};
  static void        MainProc(Fl_Widget* item, void* Value);
  static void        cbUniversal(Fl_Widget*, void*);
  static void        cbGroupProc(Fl_Widget*, void*);
  virtual int        WindowProc(UINT uMsg,WPARAM wParam,LPARAM lParam);
  virtual Fl_Widget  * NewCustomItem(int IDC);
  Fl_Widget            * GetDlgItem (int nIDDlgItem);
  tDialogCtlItem   * pGetDlgItem(int nIDDlgItem);
  BOOL CheckDlgButton    (int nIDButton,UINT uCheck);
  BOOL IsDlgButtonChecked(int nIDButton);
  BOOL CheckRadioButton  (int nIDFirstButton, int nIDLastButton, int nIDCheckButton );
  int  SendDlgItemMessage(int nIDDlgItem,UINT Msg,WPARAM wParam,LPARAM lParam);
  int  GetDlgItemInt     (int nIDDlgItem,BOOL *lpTranslated,BOOL bSigned);
  BOOL EnableDlgItem     (int nIDDlgItem,BOOL bEnable);
  BOOL SetDlgItemInt     (int nIDDlgItem,int uValue,BOOL bSigned);
  BOOL SetDlgItemText    (int nIDDlgItem,LPCSTR lpString);
  int  GetDlgItemText    (int nIDDlgItem,LPSTR lpString,int nMaxCount);
  BOOL ShowDlgItem       (int nIDDlgItem,int bShow);
  BOOL SetButtonImage    (int nIDDlgItem,int iImage);

};

extern int  FltkMessageBox(LPCSTR lpText,LPCSTR lpCaption,int Type);

class CBastelButton : public Fl_Button
{
public:
  CBastelButton(int X, int Y, int W, int H, const char *L);
  virtual ~CBastelButton();
  virtual int handle(int event);
};



#endif // CFLTKDIALOG_H
