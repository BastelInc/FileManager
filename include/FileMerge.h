#ifndef _DXFCNC_INCLUDE_
#define _DXFCNC_INCLUDE_
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Progress.H>
#include "BastelUtils.h"
#include "CToolBox.h"
#include "TreeView.h"

void cbExit(Fl_Button*, void*);
extern Fl_Double_Window * MainWindow;
//extern CVisual_Pad * MainWindow;
extern int  FltkMessageBox(LPCSTR lpText,LPCSTR lpCaption,int Type);
extern int gAppRun;
extern CToolBox  * gToolBox;
extern Fl_Tabs   * SettingsTabs;
extern CSettings1* gSettings1;
extern CTreeView * pFileExplorer;
extern Fl_Box    * gStatus0;
extern Fl_Box    * gStatus1;
extern Fl_Box    * gStatus2;
extern Fl_Progress*BusyStatus;
extern Fl_Window  * gEditemain;


#endif
