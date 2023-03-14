
//../../fltk/lib/vc6/fltkd.lib sqlite3.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib setupapi.lib
//../fltk/lib/fltkd.lib sqlite3.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib
#include <stdlib.h>
#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Overlay_Window.H>
#include <FL/Fl_Button.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Box.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_RGB_Image.H>
#include <vector>
#include "Resource.h"
#include "FileMerge.h"
#include "FltkWindow.h"
#include "BastelUtils.h"
#include "FileFolder.h"
#include "ProgressStatus.h"
#include "FileTable.h"
#include "FileArchivDB.h"
//#include "Database.h"

ReseorceInit    * R;
Fl_Preferences  * MainPrefs=NULL;
Fl_Double_Window* MainWindow;
//CVisual_Pad     * MainWindow;
//CVisual_Pad     * VisualPad=0;
Fl_Button       * FullScreen;
Fl_Button       * Exit;
Fl_Button       * Scheme;
CToolBox        * gToolBox;
Fl_Tabs         * SettingsTabs=0;
CSettings1      * gSettings1;
CTreeView       * pFileExplorer;
//CWrkSheet       * gWrkSheet = 0;
CFileTable      * gCmpSheet = 0;
Fl_Box          * gStatus0=0;
Fl_Box          * gStatus1=0;
Fl_Box          * gStatus2=0;
Fl_Progress     * BusyStatus=0;

#define FONTFACE    FL_HELVETICA//  FL_COURIER
#define FONTSIZE    14
char g_DocsPath[3][7][MAX_PATH];
char gScanPath[_MAX_PATH];
int  gInklSubfolder=0;
extern     Fl_Window* Editemain(int X, int Y, int W, int H, const char *L);
void ShowTab(int id);

int  gDebugMode=0;
int  gAppRun  = 1;
int  gbModify = 0;
int  xTool    = 408;
int  yTool    =138;
int  yStatus  = 22;
int  yTab     = 150;
int  MainPosX = 100;
int  MainPosY = 17;
int  Mainwidth = 1000;
int  Mainheight= 18 * 45;


//-------------------------------------------------------------
int AutoScan=0;
void GlTimer_CB(void *data)
{
  //DXF3DDisplay * pWrk3DSheet = (DXF3DDisplay*)data;
  if (AutoScan) {
    AutoScan--;
    if (AutoScan==0) {
      int multi = Fl::event_ctrl() != 0;
      if (Fl::event_shift())  multi = 2;
      gCmpSheet->ScanFolder(gScanPath,gInklSubfolder=multi);
    }
  }



#ifdef WIN32
  Fl::repeat_timeout(0.025, GlTimer_CB, data);
#else
  Fl::repeat_timeout(0.1, GlTimer_CB, data);
#endif
}
//---------------------------------------------------------------

char database[256] = "/home/rolf/d/wrk/FileMerge/Archiv.db";
CFileArchivDB * FileArchivDB;
//---------------------------------------------------------------
int ShowFileExplorer(LPCSTR szpathstr,LPCSTR szExtention)
{
  //pFileExplorer->clear();
  if (pFileExplorer->m_pack) {
    pFileExplorer->m_pack->clear();
    pFileExplorer->m_pack ->begin();
    pFileExplorer->m_TVItem=NULL;

    if (szExtention==NULL) {
      char *ending;
      ending = strrchr((char*)szpathstr, '.');
      if (ending) strcpy(gszExtention,ending);
      else strcpy(gszExtention,".");
    } else strcpy(gszExtention,szExtention);


#ifdef WIN32
    DWORD dwLogicalDrives = GetLogicalDrives();
    CTVItem* pTVItem1=0;
    CTVItem * parent = NULL;
    for (int i=0; i < 26; i++) {
      if (dwLogicalDrives & 1) {
        char str[200];
        str[0] = 'A'+(char)i;
        str[1] = ':';
        str[2] = '\0';
        int ddrivetype = GetDriveType(str);
        /*      CTVItem* pTVItem2 =  (CTVItem*) new CFolder(pFileExplorer,str,imFolderClose);
              pTVItem2 ->SetText(str);
                  int image = imDrive;
              if (ddrivetype == DRIVE_CDROM) {
                    image = imCompactDsik;
              } else if  (ddrivetype == DRIVE_REMOVABLE) {
                    image =  iExtmemory;
                  }
              if (pTVItem1==0) {
                pFileExplorer->add("Root",pTVItem2);
              } else {
                pTVItem1->add(pFileExplorer->prefs(),str,pTVItem2);
                }
              CTVItem* pTVItem3 =  (CTVItem*) new CFolder(pFileExplorer,"Dummy",imFolderClose);
              pTVItem2->add(pFileExplorer->prefs(),"",pTVItem3);
              pTVItem2->close();
              }
              */

        pTVItem1=  (CTVItem*) new CFolder(pFileExplorer,parent,0);
        pTVItem1->SetImage(imFolderClose);
        if (pTVItem1) {
          pTVItem1->m_lParam  = 1;
          pTVItem1->SetText(str);
          pTVItem1->m_NumChild = INSERTCHILDREN;
          pTVItem1->m_SortKey = 0xFFFFFF00;
          pFileExplorer->InsertChildItem((CTVItem*)pTVItem1,INSERT_END);
          //IncrementDirScanStatus();
        }
      }
      dwLogicalDrives >>= 1;
    }
//  if (pTVItem1)pTVItem1->close();
#else
    CTVItem * parent = NULL;
    CTVItem * pTVItem1= NULL;// (CTVItem*) new CFolder(pFileExplorer,parent,0);
    for (int i=0; i < sizeof(g_DocsPath[0])/sizeof(g_DocsPath[0][0]); i++) {
      pTVItem1=  (CTVItem*) new CFolder(pFileExplorer,parent,0);
      pTVItem1->SetImage(i+20+2);



      if (pTVItem1) {
        pTVItem1->m_lParam  = 1;
        //pTVItem1->SetText(gCmpSheet->m_DocsPath[i]);
        pTVItem1->SetText(rgFileTypeText[i]);
        pTVItem1->m_NumChild = INSERTCHILDREN;
        pTVItem1->m_SortKey = 0xFFFFFF00;
        pFileExplorer->InsertChildItem((CTVItem*)pTVItem1,INSERT_END);
      }
    }
    pTVItem1=  (CTVItem*) new CFolder(pFileExplorer,parent,0);
    pTVItem1->SetImage(imFolderClose);
    if (pTVItem1) {
      pTVItem1->m_lParam  = 1;
      pTVItem1->SetText("media");
      pTVItem1->m_NumChild = INSERTCHILDREN;
      pTVItem1->m_SortKey = 0xFFFFFF00;
      pFileExplorer->InsertChildItem((CTVItem*)pTVItem1,INSERT_END);
      //IncrementDirScanStatus();
    }
    /*    pTVItem1=  (CTVItem*) new CFolder(pFileExplorer,parent,0);
        pTVItem1->SetImage(imFolderClose);
        if (pTVItem1) {
          pTVItem1->m_lParam  = 1;
          pTVItem1->SetText("home");
          pTVItem1->m_NumChild = INSERTCHILDREN;
          pTVItem1->m_SortKey = 0xFFFFFF00;
          pFileExplorer->InsertChildItem((CTVItem*)pTVItem1,INSERT_END);
          //IncrementDirScanStatus();
        }
    */


    /*


      CTVItem* pTVItem1 =  (CTVItem*) new CFolder(pFileExplorer,szpathstr,imFolderClose);
      if (pTVItem1)
      {
        pFileExplorer->add("Root",pTVItem1);
        CTVItem* pTVItem2 =  (CTVItem*) new CFolder(pFileExplorer,"Dummy",imFolderClose);
        pTVItem1->add(pFileExplorer->prefs(),"",pTVItem2);
        pTVItem1->close();
      }
    */
#endif
  }
  pFileExplorer->show();
  return 1;
}

void Fill_TreeViewModel (/*TreeViewModel *customlist*/)
{
#if 0
  //char database[256] = "/home/rolf/Dokumente/PDNotizen/PDNotizen.db";
  //char database[256] = "../gPDNotizen/PDNotizen/PDNotizen.db";
  CDatabase * pTesdb = (CDatabase *)new CDatabase();
  if (pTesdb->OpenDatabase(database)== 0) {
    return ;
  }
//  pTesdb->CreateTables  (1);

  {
    int gebiet;
    int strasse;
    pTesdb  ->SelectGebiet(0);
    //char str[256];
    /*    CTreeItem* pNameItemIt;
    CTreeItem* pStrasseItem=NULL;
    CTreeItem* pGebietItem=NULL;
    if (customlist->m_rootItem) DeleteTreeData(customlist->m_rootItem,0);

      pGebietItem = customlist->m_rootItem =  NULL;
    */
    while (pTesdb ->FetchGebiet()) {
      //      CTreeItem*  pNeu = (CTreeItem*) new CTreeItem(NULL);
      //      if (pNeu)
      {
        /*        if (customlist->m_rootItem ==NULL)
        {
        pGebietItem = customlist->m_rootItem =  pNeu ;
        }
        else
        {
        pGebietItem ->m_next =  pNeu;
        pGebietItem = pNeu;
        }
        pNeu->m_Text = g_strdup(pTesdb ->m_Gebiet_Name);
        pNeu->m_Level = 0;
          */
        gebiet = pTesdb ->m_GebietID;
        pTesdb ->SelectStrasse(gebiet,0);
        while (pTesdb ->FetchStrasse()) {
          /*          CTreeItem*  pNeu = (CTreeItem*) new CTreeItem(pGebietItem);
          if (pNeu )
          {
          if (pGebietItem->m_child==NULL)
          {
          pStrasseItem = pGebietItem->m_child = pNeu;
          }
          else
          {
          pStrasseItem ->m_next =  pNeu;
          pStrasseItem = pNeu;
          }
          pStrasseItem->m_Text = g_strdup(pTesdb ->m_StrasseName);
          pStrasseItem->m_Level = 1;
            */
          strasse = pTesdb->m_StrasseID;
          pTesdb->SelectNamen(gebiet,strasse,0);
          while (pTesdb->FetchNamen()) {
            /*              CTreeItem*  pNeu = (CTreeItem*) new CTreeItem(pStrasseItem);
            if (pNeu)
            {
            if (pStrasseItem->m_child==NULL)
            {
            pNameItemIt = pStrasseItem->m_child =  pNeu ;
            }
            else
            {
            pNameItemIt  ->m_next =  pNeu;
            pNameItemIt  = pNeu;
            }
            pNeu->m_Text = g_strdup(pTesdb ->m_Wohnungsinhaber);
            pNeu->m_Level = 2;
            pNeu->m_NhCode=(int) pTesdb ->m_NhCcode[0];
            }*/
          }
        }
      }
    }
  }
  pTesdb->CloseDatabase();
#endif
}


//  TestTreeData(customlist);

//-------------------------------------------------------------
int OutputDebugInfo::level=2;
int OutputDebugInfo::debug=1;
CBastelUtils gBastelUtils ;
//-------------------------------------------------------------
int  FltkMessageBox(LPCSTR lpText,LPCSTR lpCaption,int Type)
{
  int ret =0;
  if ((Type&0xF)  == MB_OK) {
    fl_message(lpText,lpCaption);
    return 0;
  } else if ((Type &0xf) == MB_YESNOCANCEL) {
    ret = fl_choice("%s\n%s","Nein","Ja","Abbruch",lpCaption,lpText);
    if      (ret==1 ) return IDYES;
    else if (ret==0 ) return IDNO;
    return IDCANCEL;
  } else {
    ret = fl_choice("%s\n%s","Nein","Ja",NULL,lpCaption,lpText);
    return (ret == 1)?IDYES:IDNO;
  }

}
//---------------------------------------------------------------
void SetStatusText(LPCSTR  pnt,int num,int style)
{
  static char StatusTxt0[80];
  static char StatusTxt1[80];
  static char StatusTxt2[80];
  static char StatusTxt3[80];
  if (num==0) {
    if (gStatus0) {
      strncpy(StatusTxt0,pnt,sizeof(StatusTxt0));
      gStatus0->label(StatusTxt0);
      gStatus0->redraw();
    }
  } else if (num==1) {
    if (gStatus1) {
      strncpy(StatusTxt1,pnt,sizeof(StatusTxt1));
      gStatus1->label(StatusTxt1);
      gStatus1->redraw();
    }
  } else if (num==2) {
    if (gStatus2) {
      strncpy(StatusTxt2,pnt,sizeof(StatusTxt2));
      gStatus2->label(StatusTxt2);
      gStatus2->redraw();
    }
  } else if (num==3) {
    if (BusyStatus) {
      strncpy(StatusTxt3,pnt,sizeof(StatusTxt3));
      BusyStatus->label(StatusTxt3);
      BusyStatus->redraw();
    }
  }
}
//---------------------------------------------------------------
void DebugMessage(LPCSTR  szmsg)
{
  if (OutputDebugInfo::debug) {
    fprintf(stderr,"%s\n",szmsg);
  }
  if (gStatus0) {
    gStatus0->label(szmsg);
    gStatus0->redraw();
  }
}
//---------------------------------------------------------------
void bcb1(Fl_Widget *,void *) ;
void bcb2(Fl_Widget *,void *) ;
void bcb3(Fl_Widget *,void *) ;
void bcb4(Fl_Widget *,void *) ;
//---------------------------------------------------------------
void cbScheme(Fl_Button*, void*)
{
  Fl::scheme("base");
  Fl::foreground(240, 240, 240);
  Fl::background(92, 91, 86);
  Fl::background2(51, 51, 51);
  Fl::set_color(FL_SELECTION_COLOR, 222, 177, 1);
}
//---------------------------------------------------------------
void cbFullScreen(Fl_Button*, void*)
{

  Fl::scheme("xp");
  Fl::foreground(10,10,10);
  Fl::background2(151,151,151);
  Fl::background(239, 235, 222);

//  if (MainWindow->fullscreen_active ()) MainWindow->fullscreen_off();
//  else                                  MainWindow->fullscreen();
}
//---------------------------------------------------------------
void cbExit(Fl_Button*, void*)
{
  if (MainPrefs) { // && LonTreeMaster && ConnectionsBoxH)
    Mainwidth = MainWindow->w();
    Mainheight=MainWindow->h();
    MainPosX  =MainWindow->x();
    MainPosY  =MainWindow->y();
    if (MainWindow->fullscreen_active ()==0) {
      MainPrefs->set("Mainwidth",    Mainwidth);
      MainPrefs->set("Mainheight",   Mainheight);
      MainPrefs->set("MainPosX",     MainPosX);
      MainPrefs->set("MainPosY",     MainPosY);
    }
    MainPrefs->flush();
  }

  if (gbModify) {
    int ret = fl_choice("Data modyify, you want to quit?","Nein","Ja",NULL);
    if (ret == 1)  exit(0);
  } else exit(0);
}
//---------------------------------------------------------------
void cbMainWindowExit(Fl_Widget*, void*)
{
  if (Fl::event()==FL_SHORTCUT && Fl::event_key()==FL_Escape)
    return; // ignore Escape
  cbExit(NULL,NULL);
}

//---------------------------------------------------------------

#ifdef WIN32
/* static int mbcs2utf(const char *s, int l, char *dst, unsigned dstlen) */
static int mbcs2utf(const char *s, int l, char *dst)
{
  static xchar *mbwbuf;
  unsigned dstlen = 0;
  if (!s) return 0;
  dstlen = (l * 6) + 6;
  mbwbuf = (xchar*)malloc(dstlen * sizeof(xchar));
  l = (int) mbstowcs(mbwbuf, s, l);
  /* l = fl_unicode2utf(mbwbuf, l, dst); */
  l = fl_utf8fromwc(dst, dstlen, mbwbuf, l);
  dst[l] = 0;
  free(mbwbuf);
  return l;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
  int  i;
  char **argv;

#  ifdef ROLF_DEBUG
  /*
   * If we are using compiling in debug mode, open a console window so
   * we can see any printf's, etc...
   *
   * While we can detect if the program was run from the command-line -
   * look at the CMDLINE environment variable, it will be "WIN" for
   * programs started from the GUI - the shell seems to run all WIN32
   * applications in the background anyways...
   */

  AllocConsole();
  freopen("conin$", "r", stdin);
  freopen("conout$", "w", stdout);
  freopen("conout$", "w", stderr);
#  endif /* _DEBUG */

  argv = (char**) malloc(sizeof(char*) * (__argc + 1));
  i = 0;
  while (i < __argc) {
    int l;
    unsigned dstlen;
    if (__wargv ) {
      for (l = 0; __wargv[i] && __wargv[i][l]; l++) {}; /* is this just wstrlen??? */
      dstlen = (l * 5) + 1;
      argv[i] = (char*) malloc(dstlen);
      /*    ar[i][fl_unicode2utf(__wargv[i], l, ar[i])] = 0; */
      dstlen = fl_utf8fromwc(argv[i], dstlen, __wargv[i], l);
      argv[i][dstlen] = 0;
    } else {
      for (l = 0; __argv[i] && __argv[i][l]; l++) {};
      dstlen = (l * 5) + 1;
      argv[i] = (char*) malloc(dstlen);
      /*      ar[i][mbcs2utf(__argv[i], l, ar[i], dstlen)] = 0; */
      argv[i][mbcs2utf(__argv[i], l, argv[i])] = 0;
    }
    i++;
  }
  argv[__argc] = 0;
  /* Run the standard main entry point function... */


#else
int main(int argc, char **argv) {
  int i=0;
#endif
  i=0;
  char * pnt = getenv("HOME");
  if (pnt) {
    strcpy(CBastelUtils::s_IniPath,pnt);
    strcat(CBastelUtils::s_IniPath,"/.");
  } else {
#ifdef WIN32
    if (!pnt) pnt = getenv("HOME_PATH");
    if (!pnt) pnt = getenv("UserProfile");
    strcpy(CBastelUtils::s_IniPath,pnt);
    strcat(CBastelUtils::s_IniPath,"/.");
#else
    strcpy(CBastelUtils::s_IniPath,"");
#endif
  }

  strcat(CBastelUtils::s_IniPath,"bastel");
  gBastelUtils.SetIniFile("FileMerge.ini");
  R = new ReseorceInit ();
  Fl::get_system_colors();
  if (1) {
    Fl::get_system_colors();
    Fl::scheme("gtk+");
    //Fl::scheme("gleam");
    Fl::foreground(240, 240, 240);
    Fl::background(92, 91, 86);
    Fl::background2(86,85, 80);
    Fl::set_color(FL_SELECTION_COLOR, 241, 176, 0);
  } else {
    Fl::scheme("xp");
  }

  Fl::set_font(FL_FREE_FONT,"-*-medium-*-normal-*-12-*-*-*-*-*-*-*");
  //Fl::set_font(0,"comic sans ms");
//  Fl::set_font(0,"Ubuntu");
  //fl_height(0,22);

  //fl_font(FONTFACE,FONTSIZE);
  if (!MainPrefs) {
    MainPrefs = new Fl_Preferences(Fl_Preferences::USER, "fltk.org", "filemerge");
  }
  MainPrefs->get("ToolWidth",xTool,xTool);
  MainPrefs->get("Editemain",yTab,yTab);
  MainPrefs->get("Mainwidth",Mainwidth,Mainwidth);
  MainPrefs->get("Mainheight",Mainheight,Mainheight);
  MainPrefs->get("MainPosX",MainPosX,MainPosX);
  MainPrefs->get("MainPosY",MainPosY,MainPosY);

  Fl::option(Fl::OPTION_ARROW_FOCUS, 0);		// disable arrow focus nav (we want arrows to control cells)
  //Fl::option(Fl::OPTION_ARROW_FOCUS, 1);		// we want arrow keys to navigate table's widgets

  MainWindow  = new Fl_Double_Window(Mainwidth, Mainheight, "BastelTree");
  //MainWindow= new CVisual_Pad(MainPosX,MainPosY,Mainwidth, Mainheight, "DxfCnc");
  if (MainWindow  ) {
//    MainWindow  ->size_range(1014,748);
    MainWindow  ->position(MainPosX,MainPosY);
    MainWindow ->icon(&ReseorceInit::ic_AppIcon_xpm );
    MainWindow ->callback(cbMainWindowExit);

    Fl_Tile* ptile = new Fl_Tile(2, 1, Mainwidth-2,Mainheight-2);
    if (ptile) {

      Fl_Group* ToolPart= new Fl_Group(0,0,xTool,Mainheight-yStatus,0);
      if (ToolPart) {
        ToolPart ->box(FL_DOWN_FRAME);
        int border = 2;
        int xTool2 = xTool-border*2;
        int yTool2 = Mainheight-yTool-border*2 - yStatus;
        gToolBox = new CToolBox(border,border,xTool2,yTool-border,0);
        if (gToolBox ) {
//          gToolBox ->labelfont (FL_FREE_FONT);
//          gToolBox ->labelsize(10);

          gToolBox ->InitWindow(ptile,0,0,0,xTool,yTool,0);
          gToolBox ->end();
        }
        SettingsTabs = new Fl_Tabs(border,yTool,xTool2,yTool2,0);
        if (SettingsTabs ) {
          gSettings1 = new CSettings1(border,yTool,xTool2,yTool2,0);
          if (gSettings1 ) {
//           gSettings1 ->labelfont (FL_FREE_FONT);
//           gSettings1 ->labelsize(10);

            gSettings1 ->InitWindow(ptile,0,border,yTool,xTool2,yTool2,0);
            gSettings1 ->end();
          }
          pFileExplorer = new CTreeView(border,yTool,xTool2,yTool2,0);
          if (pFileExplorer ) {

          }

          /*          pFileExplorer = new CTreeView(border,yTool,xTool2,yTool2,0);
                    if (pFileExplorer )
                    {
                      pFileExplorer->callback(TreeCallback,0);
                      pFileExplorer->selectmode(FL_TREE_SELECT_SINGLE);
                      pFileExplorer->item_reselect_mode(FL_TREE_SELECTABLE_ALWAYS);
                      pFileExplorer->showroot(0);
                      //pFileExplorer->end();
                    }*/
          SettingsTabs ->end();
        }
        //ToolPart->resizable(SettingsTabs);
        ToolPart->end();
      }
      extern int gDocShowPrimaryPath;
      extern int gSourceAndCompareMuster;
      gSourceAndCompareMuster  = gBastelUtils.GetProfileInt("FileCompare","Compare",gSourceAndCompareMuster);
      gDocShowPrimaryPath  = gBastelUtils.GetProfileInt("FileCompare","ShowDocs",gDocShowPrimaryPath );


      gToolBox ->Show();
      MainWindow->begin();
      Fl_Group* StatusPart= new Fl_Group(0,Mainheight-yStatus,Mainwidth,yStatus,0);
      if (StatusPart) {
        StatusPart->box(FL_THIN_UP_FRAME);
        int yStatus1   = yStatus-4;
        int breite0    = xTool;
        int breite3    = (Mainwidth -xTool)/4;
        int breite2    = breite3;
        int breite1    = Mainwidth -(breite0+breite2+breite3);
        int ypos3      = Mainheight-yStatus+2;
        gStatus0  = new Fl_Box(2,ypos3,breite0-2,yStatus1,"Left");
        gStatus0  ->box(FL_THIN_DOWN_BOX);
        gStatus1  = new Fl_Box(breite0,ypos3,breite1,yStatus1,"File");
        gStatus1  ->box(FL_THIN_DOWN_BOX);
        gStatus2  = new Fl_Box(breite0+breite1,ypos3,breite2,yStatus1,"Selection");
        gStatus2  ->box(FL_THIN_DOWN_BOX);
        BusyStatus= new Fl_Progress(breite0+breite1+breite2,ypos3,breite3-2,yStatus1,"Progress");
        BusyStatus->box(FL_THIN_DOWN_BOX);
        StatusPart->end();
      }
      //Fl_Group* WorkPart= new Fl_Group(0,0,xTool,Mainheight-yStatus,0);//Fl_Group(0,0,xTool,Mainheight-yStatus,0);
      //if (WorkPart)
      //{
        //  WorkPart->box(FL_DOWN_FRAME);

//        ptile->begin();

        //VisualPad   = (CVisual_Pad * ) new CVisual_Pad(xTool,yTab, Mainwidth-xTool,Mainheight-yTab-yStatus,NULL);
//        VisualPad  =(CVisual_Pad * ) new CVisual_Pad(xTool,0, Mainwidth-xTool,yTab,NULL);
//        if (VisualPad)
//        {
//          Fl_Button *b;
//          b = new Fl_Button(50,50,100,100,"wider\n(a)");
//          b->callback(bcb1);
//          b->shortcut('a');
//          b = new Fl_Button(250,50,100,100,"narrower\n(b)");
//          b->callback(bcb2);
//          b->shortcut('b');
//          b = new Fl_Button(50,250,100,100,"taller\n(c)");
//          b->callback(bcb3);
//          b->shortcut('c');
//          b = new Fl_Button(250,250,100,100,"shorter\n(d)");
//          b->callback(bcb4);
//          b->shortcut('d');
//          VisualPad->end();
//          VisualPad->show();
//        }
//        else
        yTab =0;
//        {
//          ptile->begin();
//          gWrkSheet = (CWrkSheet*) new CWrkSheet(xTool,yTab,Mainwidth-xTool,Mainheight-yTab-yStatus,NULL);
//          gWrkSheet->when(FL_WHEN_RELEASE);
//          gWrkSheet ->end();
//        }
//        WorkPart->resizable(gWrkSheet);


        {
          ptile->begin();
          gCmpSheet = (CFileTable*) new CFileTable(xTool,yTab,Mainwidth-xTool,Mainheight-yTab-yStatus,NULL);
          //D3WrkSheet->when(FL_WHEN_RELEASE);
          gCmpSheet ->end();
        }
        //  WorkPart->resizable(gCmpSheet);

        //  WorkPart->end();
//      }


      ptile->end();
      ptile->resizable(gCmpSheet ) ;//WorkPart);
    }
    MainWindow->end();
  }

  char * pnt2 = getenv("HOME");
  if (pnt2==0) {
    if (!pnt2) pnt2 = getenv("HOME_PATH");
    if (!pnt2) pnt2 = getenv("UserProfile");
  }
//  DXFInitLaden();
  //strncpy(gszHomeFilePath,pnt2,sizeof(gszHomeFilePath));

  gCmpSheet->mainInit();
#ifdef WIN32
  ShowFileExplorer("","");//  .mp3");
#else
  //ShowFileExplorer("Home",".mp3");
  ShowFileExplorer("Home","");
#endif
  Fill_TreeViewModel();
  FileArchivDB = new CFileArchivDB();
  FileArchivDB->InitSQLTable(database);

  int selNeFile=1;
  ShowTab(IDM_FILEOPEN);
  //MainWindow ->resizable(VisualPad);
  MainWindow ->resizable(gCmpSheet );
  //MainWindow ->resizable(gWrkSheet);
#ifdef WIN32
  MainWindow ->show(__argc,argv);
#else
  MainWindow ->show(argc,argv);
#endif
  Fl::set_color(FL_SELECTION_COLOR, 255, 176, 0);

  //MainWindow ->redraw_overlay();

  gAppRun=1;
  Fl::lock();
  //int ret = Fl::run();
  gAppRun=1;
  Fl::lock();
  void * next_message ;
  int ret = 0;//Fl::run();

  Fl::add_timeout(1.0, GlTimer_CB, gCmpSheet);
  // Fuehre die FLTK-Schleife aus und verarbeite Thread-Nachrichten
#ifdef WIN32
  while (Fl :: wait (10)> 0)
#else
  while (gAppRun && Fl :: wait ()> 0)
#endif
  {
//    if ((next_message = Fl :: thread_message ())!= NULL) {
//      tMessage * pmsg = (tMessage * )next_message ;
//      if (pmsg->Code== IDM_SCANFOLDER) {
//        AutoScan= pmsg->Value;
//      }
////        if (pmsg->receiver) {
////          //((CFltkDialogBox*)pmsg->receiver)->WindowProc(WM_COMMAND,pmsg->Code,pmsg->Value);
////        }
//      //if (next_message)  free(next_message);
//      next_message =NULL;
//    }




    if (selNeFile) {
      selNeFile=0;
      char  str[256];
      gBastelUtils.GetProfileString("Docs","LastPath","/home/rolf",str,sizeof(str));
      extern void load_file(const char *newfile, int ipos);
      gStatus1->label(str);
      //char * p = strrchr(str,'/');
      //if (p) *p = '\0';
      SelectTreeItem(pFileExplorer,str);
      //if(gWrkSheet )gWrkSheet ->FileOpen(str);

    }
    Sleep(10);
  }
  FileArchivDB->FreeDB();
  return ret;
}
