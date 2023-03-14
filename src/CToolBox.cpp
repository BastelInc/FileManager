#include "FileMerge.h"
#include "CToolBox.h"
#include "BastelUtils.h"
#include "ProgressStatus.h"
#include "Resource.h"
#include "FileFolder.h"
#include "FltkDialogBox.h"
#include <ctype.h>
#include <math.h>
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Value_Slider.H>
#include "FileTable.h"
#include <FL/Fl_Native_File_Chooser.H>
#include "FileArchivDB.h"

int gDocShowPrimaryPath = 0x6;
int gSourceAndCompareMuster = 0x1F;

#define BUTTONWIDTH   38
#define BUTTONHEIGHT  33
#define XRASTER       38
#define YRASTER       33
const tDialogCtlItem rg_ToolBox  []= {
  { eDialogBox,"Tool",IDC_DialogBox, 4,  0,80,268,WS_BORDER| WS_TABSTOP},                       // Dialog
  { AUTORADIOBUTTON "Open",  IDM_FILEOPEN,  2+XRASTER*0,1+YRASTER*0,BUTTONWIDTH,BUTTONHEIGHT,BS_AUTORADIOBUTTON | WS_TABSTOP,NULL},
  { BUTTON          "ReadDB",IDM_FILESAVE,  2+XRASTER*1,1+YRASTER*0,BUTTONWIDTH,BUTTONHEIGHT,WS_TABSTOP,NULL},
  //{ BUTTON          "Exit",  IDM_EXIT,      2+XRASTER*2,1+YRASTER*0,BUTTONWIDTH,BUTTONHEIGHT,WS_TABSTOP,NULL},
  { CHECKBOX        "Local", IDC_LocalSrc,  2+XRASTER*2,1+YRASTER*0,BUTTONWIDTH,BUTTONHEIGHT,WS_TABSTOP|BS_AUTOCHECKBOX,NULL},
  { CHECKBOX        "Server",IDC_ServerSrc, 2+XRASTER*3,1+YRASTER*0,BUTTONWIDTH,BUTTONHEIGHT,WS_TABSTOP|BS_AUTOCHECKBOX,NULL},
  { CHECKBOX        "Backup",IDC_BackupSrc, 2+XRASTER*4,1+YRASTER*0,BUTTONWIDTH,BUTTONHEIGHT,WS_TABSTOP|BS_AUTOCHECKBOX,NULL},
  { CHECKBOX        "Equal", IDC_Equal,     2+XRASTER*5,1+YRASTER*0,BUTTONWIDTH,BUTTONHEIGHT,WS_TABSTOP|BS_AUTOCHECKBOX,NULL},
  { CHECKBOX        "Diff",  IDC_Differ,    2+XRASTER*6,1+YRASTER*0,BUTTONWIDTH,BUTTONHEIGHT,WS_TABSTOP|BS_AUTOCHECKBOX,NULL},

  { AUTORADIOBUTTON "Buro",  IDC_DocsBuero, 2+XRASTER*0,1+YRASTER*1,BUTTONWIDTH,BUTTONHEIGHT,WS_TABSTOP|BS_AUTORADIOBUTTON,NULL},
  { AUTORADIOBUTTON "Media", IDC_DocsMedia, 2+XRASTER*1,1+YRASTER*1,BUTTONWIDTH,BUTTONHEIGHT,WS_TABSTOP|BS_AUTORADIOBUTTON,NULL},
  { AUTORADIOBUTTON "CAD",   IDC_DocsCAD,   2+XRASTER*2,1+YRASTER*1,BUTTONWIDTH,BUTTONHEIGHT,WS_TABSTOP|BS_AUTORADIOBUTTON,NULL},
  { AUTORADIOBUTTON "Pict",  IDC_DocsPict,  2+XRASTER*3,1+YRASTER*1,BUTTONWIDTH,BUTTONHEIGHT,WS_TABSTOP|BS_AUTORADIOBUTTON,NULL},
  { AUTORADIOBUTTON "C-Src", IDC_DocsSrc,   2+XRASTER*4,1+YRASTER*1,BUTTONWIDTH,BUTTONHEIGHT,WS_TABSTOP|BS_AUTORADIOBUTTON,NULL},
  { AUTORADIOBUTTON "Syst",  IDC_DocsSys,   2+XRASTER*5,1+YRASTER*1,BUTTONWIDTH,BUTTONHEIGHT,WS_TABSTOP|BS_AUTORADIOBUTTON,NULL},
  { BUTTON          "Scan",  IDM_UPDATE,    2+XRASTER*6,1+YRASTER*1,BUTTONWIDTH,BUTTONHEIGHT,WS_TABSTOP,NULL},
  {0}
};


//-------------------------------------------------------------
const tDialogCtlItem rg_Settings1[]= {
  { eDialogBox,"Tool",  IDC_DialogBox-1,  4,  2,90,192,WS_BORDER| WS_TABSTOP},                        // Dialog
  { BUTTON     "",      IDC_DocumentImg,  4,  4, 30, 33,0,NULL},
  //{ CTEXT      "Docs",  IDC_Documents,   38,  8, 42, 18,ES_LEFT},

  { LTEXT      "Local", IDC_Local,       38, 25, 42, 18,ES_LEFT},
  { EDITTEXT   "",      IDC_LocalPath,   38, 40,184, 18,WS_TABSTOP| WS_CLIENTEDGE,NULL},
  { BUTTON     "Scan",  IDC_LocalUpd,     4, 40, 30, 18,WS_TABSTOP,NULL},
  { BUTTON     "GetDir",IDC_LocalGet,   226, 40, 30, 18,WS_TABSTOP,NULL},
  { LTEXT      "Server",IDC_Server,      38, 65, 42, 18,ES_LEFT},
  { EDITTEXT   "",      IDC_ServerPath,  38, 80,184, 18,WS_TABSTOP| WS_CLIENTEDGE,NULL},
  { BUTTON     "Scan",  IDC_ServerUpd,    4, 80, 30, 18,WS_TABSTOP,NULL},
  { BUTTON     "GetDir",IDC_ServerGet,  226, 80, 30, 18,WS_TABSTOP,NULL},
  { LTEXT      "Backup",IDC_Backup1,     38,105, 42, 18,ES_LEFT},
  { EDITTEXT   "",      IDC_Backup1Path, 38,120,184, 18,WS_TABSTOP| WS_CLIENTEDGE,NULL},
  { BUTTON     "Scan",  IDC_Backup1Upd,   4,120, 30, 18,WS_TABSTOP,NULL},
  { BUTTON     "GetDir",IDC_Backup1Get, 226,120, 30, 18,WS_TABSTOP,NULL},

  {0}
};



int     ParamModify=0;


int  CBastelButton::handle(int)
{
  return 0;
}




void ShowTab(int id)
{
  static int rec=0;
  if (rec) return;
  rec++;
  extern Fl_Tabs   * SettingsTabs;
  extern CSettings1* gSettings1;
  extern CTreeView * pFileExplorer;
  switch(id) {
  case IDM_FILEOPEN:
    SettingsTabs->value(pFileExplorer);
    break;
  case IDM_FILESAVE:
    SettingsTabs->value(gSettings1);
    gSettings1->Show();
    break;
  }
//  gToolBox->CheckRadioButton(IDC_DocsBuero,IDM_TOOLBAR,id);
  rec--;
}
//-------------------------------------------------------------
CSettings1 ::CSettings1(int X, int Y, int W, int H, const char *L):
  CFltkDialogBox(X,Y,W,H,L)
{
  m_NumDialogCtlItem=0;
  const tDialogCtlItem * pCtlItem = rg_Settings1;
  while (pCtlItem ->idc) {
    pCtlItem ++;
    m_NumDialogCtlItem++;
  }
  m_pDialogCtlItem   = (tDialogCtlItem *)malloc (m_NumDialogCtlItem * sizeof(tDialogCtlItem));
  if (m_pDialogCtlItem) {
    memcpy(m_pDialogCtlItem,&rg_Settings1,m_NumDialogCtlItem* sizeof(tDialogCtlItem));
  }
}

//-------------------------------------------------------------
CSettings1::~CSettings1()
{
}

BOOL CSettings1::SetButtonImage(int nIDDlgItem, int iImage)
{
  Fl_Button * butt = (Fl_Button * )GetDlgItem(nIDDlgItem);
  if (butt ) {
    butt ->image(R->ButtonImage_List[iImage]);
    butt ->align(FL_ALIGN_CENTER);
  }
  return 0;
}
//-------------------------------------------------------------
//-------------------------------------------------------------
int CSettings1::InitWindow(Fl_Widget  * pParent,int ID,int left,int top,int width, int height,int Style)
{
  int res = CFltkDialogBox::InitWindow(pParent,ID,left,top,width,height,Style|WS_TABSTOP);

  /*  tDialogCtlItem * pCtlItem = pGetDlgItem(id_ArbeitsTemp);
    if (pCtlItem && pCtlItem->pWndObject)
    {
      ((Fl_Value_Slider*)pCtlItem->pWndObject)->type(FL_VERT_NICE_SLIDER);
      //((Fl_Value_Slider*)pCtlItem->pWndObject)->box(FL_FLAT_BOX);
      //((Fl_Value_Slider*)pCtlItem->pWndObject)->labelsize(8);
      //((Fl_Value_Slider*)pCtlItem->pWndObject)->scrollvalue(0,10,0,10);
    }
    pCtlItem = pGetDlgItem(id_EintauchTiefe1);
    if (pCtlItem && pCtlItem->pWndObject)
    {
      ((Fl_Value_Slider*)pCtlItem->pWndObject)->type(FL_VERT_NICE_SLIDER);
    }
    pCtlItem = pGetDlgItem(id_StufenTiefe1);
    if (pCtlItem && pCtlItem->pWndObject)
    {
      ((Fl_Value_Slider*)pCtlItem->pWndObject)->type(FL_VERT_NICE_SLIDER);
    }
    */
  return res;
}
//-------------------------------------------------------------
void CSettings1::Show(void)
{
  if (gDocShowPrimaryPath>=0 && gDocShowPrimaryPath  < 7) {
    SetDlgItemText(IDC_DocumentImg,rgFileTypeText[gDocShowPrimaryPath]);
    SetButtonImage(IDC_DocumentImg,2+gDocShowPrimaryPath);
//  for (int i=0; i < sizeof(g_DocsPath[0])/sizeof(g_DocsPath[0][0]); i++) {
//    SetDlgItemText(IDC_LocalPath+i*4,g_DocsPath[0][i]);
//  }
    for (int i=0; i < sizeof(g_DocsPath)/sizeof(g_DocsPath[0]); i++) {
      SetDlgItemText(IDC_LocalPath+i*4,g_DocsPath[i][gDocShowPrimaryPath]);
    }
  }
  /*  char s[32];
    if (AktTiefe && AktTiefe<=6)
    {
      sprintf(s,"%1.0f",(float)Pa.Tempo[AktTiefe]/(float)Pa.Aufloesung);
      SetDlgItemText(id_ArbeitsTempT,s);
      tDialogCtlItem * pCtlItem = pGetDlgItem(id_ArbeitsTemp);
      if (pCtlItem && pCtlItem->pWndObject)
      {
        ((Fl_Value_Slider*)pCtlItem->pWndObject)->selection_color((Fl_Color)Farbe[AktTiefe]);
        ((Fl_Value_Slider*)pCtlItem->pWndObject)->step(10);
        ((Fl_Value_Slider*)pCtlItem->pWndObject)->range(PaMax.Tempo[AktTiefe],PaMin.Tempo[AktTiefe]);
        ((Fl_Value_Slider*)pCtlItem->pWndObject)->value(Pa.Tempo[AktTiefe]);
      }
      SetBusyDisplay(PaMax.Tempo[AktTiefe],"PaMax");
      sprintf(s,"%1.2f",(float)Pa.Tiefe[AktTiefe]/(float)Pa.Aufloesung);
      SetDlgItemText(id_EintauchTiefe1T,s);
      pCtlItem = pGetDlgItem(id_EintauchTiefe1);
      if (pCtlItem && pCtlItem->pWndObject)
      {
        ((Fl_Value_Slider*)pCtlItem->pWndObject)->selection_color((Fl_Color)Farbe[AktTiefe]);
        ((Fl_Value_Slider*)pCtlItem->pWndObject)->step(5);
        ((Fl_Value_Slider*)pCtlItem->pWndObject)->range(PaMax.Tiefe[AktTiefe],PaMin.Tiefe[AktTiefe]);
        ((Fl_Value_Slider*)pCtlItem->pWndObject)->value(Pa.Tiefe[AktTiefe]);
      }
      // Stufe Tiefe
      sprintf(s,"%1.2f",(float)Pa.Stufe[AktTiefe]/(float)Pa.Aufloesung);
      SetDlgItemText(id_StufenTiefe1T,s);

      pCtlItem = pGetDlgItem(id_StufenTiefe1);
      if (pCtlItem && pCtlItem->pWndObject)
      {
        ((Fl_Value_Slider*)pCtlItem->pWndObject)->selection_color((Fl_Color)Farbe[AktTiefe]);
        ((Fl_Value_Slider*)pCtlItem->pWndObject)->step(1);
        ((Fl_Value_Slider*)pCtlItem->pWndObject)->range(PaMax.Stufe[AktTiefe],PaMin.Stufe[AktTiefe]);
        ((Fl_Value_Slider*)pCtlItem->pWndObject)->value(Pa.Stufe[AktTiefe]);
      }
    }
    else
    {
      SetDlgItemText(id_ArbeitsTempT,"err");
      SetDlgItemText(id_EintauchTiefe1T,"err");
      SetDlgItemText(id_StufenTiefe1T,"err");
  //    SetDlgItemText(id_EintauchTempo1T,"err");
  //    SetDlgItemText(id_BohrTempo1T,"err");
    }
  */
  show();
  redraw();
}
//-------------------------------------------------------------
int roundf(double v)
{
  return (int)(v+0.5);
}

//-------------------------------------------------------------
// GLOBALS
Fl_Input *G_filename = NULL;
/*void PickDir_CB(Fl_Widget*, void*)
{
  // Create native chooser
  Fl_Native_File_Chooser native;
  native.title("Pick a Directory");
  native.directory(G_filename->value());
  native.type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
  // Show native chooser
  switch ( native.show() ) {
  case -1:
    fprintf(stderr, "ERROR: %s\n", native.errmsg());
    break;	// ERROR
  case  1:
    fprintf(stderr, "*** CANCEL\n");
    fl_beep();
    break;		// CANCEL
  default: 								// PICKED DIR
    if ( native.filename() ) {
      G_filename->value(native.filename());
    } else {
      G_filename->value("NULL");
    }
    break;
  }
}
*/
//-------------------------------------------------------------

int CSettings1::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if (uMsg == WM_COMMAND) {
//    char str[32];
//    float f;
    static bool recursion=0;
    switch (wParam) {
    case IDC_LocalUpd:
      if (gDocShowPrimaryPath>=0 && gDocShowPrimaryPath  < 7) {
        deactivate();
        int n = strlen(g_DocsPath[0][gDocShowPrimaryPath]);
        FileArchivDB->GetDirectoryContents(g_DocsPath[0][gDocShowPrimaryPath],n,0,gDocShowPrimaryPath);
        activate();
      }
      break;
    case IDC_ServerUpd:
      if (gDocShowPrimaryPath>=0 && gDocShowPrimaryPath  < 7) {
        deactivate();
        int n = strlen(g_DocsPath[1][gDocShowPrimaryPath]);
        FileArchivDB->GetDirectoryContents(g_DocsPath[1][gDocShowPrimaryPath],n,1,gDocShowPrimaryPath);
        activate();
      }
      break;
    case IDC_Backup1Upd:
      if (gDocShowPrimaryPath>=0 && gDocShowPrimaryPath  < 7) {
        deactivate();
        int n = strlen(g_DocsPath[2][gDocShowPrimaryPath]);

        FileArchivDB->GetDirectoryContents(g_DocsPath[2][gDocShowPrimaryPath],n,2,gDocShowPrimaryPath);
        activate();
      }
      break;
    case IDC_LocalGet:
    case IDC_ServerGet:
    case IDC_Backup1Get: {
      if (recursion) return 1;
      recursion=1;
      if (gDocShowPrimaryPath>=0 && gDocShowPrimaryPath  < 7) {

        int i= (wParam - IDC_LocalGet) / 4;
        if (i>=0 && i < 3) {

          Fl_Native_File_Chooser native;
          tDialogCtlItem * pCtlItem = pGetDlgItem(wParam -2 );
          if (pCtlItem && pCtlItem->pWndObject) {
            G_filename = ((Fl_Input *  )pCtlItem->pWndObject);
            native.title("Pick a Directory");
            native.directory(G_filename->value());
            native.type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
            // Show native chooser
            if ( native.show()==0 ) {
              if ( native.filename() ) {
                G_filename->value(native.filename());
                gBastelUtils.WriteProfileString(rgQuellText[i],rgFileTypeText[gDocShowPrimaryPath],native.filename());
                gBastelUtils.GetProfileString(rgQuellText[i],rgFileTypeText[gDocShowPrimaryPath],"/home/rolf",g_DocsPath[i][gDocShowPrimaryPath],sizeof(g_DocsPath[0][0]));
              }
            }
          }
        }
      }
      recursion=0;
      ParamModify = 1;
    }
    break;
    }
  }
  return 0;
}
//-------------------------------------------------------------
//-------------------------------------------------------------
CToolBox ::CToolBox (int X, int Y, int W, int H, const char *L):
  CFltkDialogBox(X,Y,W,H,L)
{
  const tDialogCtlItem * pCtlItem = rg_ToolBox ;
  m_NumDialogCtlItem=0;
  while (pCtlItem ->idc) {
    pCtlItem ++;
    m_NumDialogCtlItem++;
  }
  m_pDialogCtlItem   = (tDialogCtlItem *)malloc (m_NumDialogCtlItem * sizeof(tDialogCtlItem));
  if (m_pDialogCtlItem) {
    memcpy(m_pDialogCtlItem,&rg_ToolBox,m_NumDialogCtlItem* sizeof(tDialogCtlItem));
  }
  ParamModify = 0;

}

//-------------------------------------------------------------//-------------------------------------------------------------
CToolBox ::~CToolBox ()
{
}
//-------------------------------------------------------------
BOOL CToolBox::SetButtonImage(int nIDDlgItem, int iImage)
{
  Fl_Button * butt = (Fl_Button * )GetDlgItem(nIDDlgItem);
  if (butt ) {
    butt ->image(R->ButtonImage_List[iImage]);
    /*butt ->align(FL_ALIGN_INSIDE|FL_ALIGN_WRAP|FL_ALIGN_CLIP
                 //| FL_ALIGN_RIGHT|FL_ALIGN_TEXT_NEXT_TO_IMAGE| FL_ALIGN_TEXT_OVER_IMAGE
                );
    */
    //butt ->align(FL_ALIGN_IMAGE_NEXT_TO_TEXT);
    //butt ->align(FL_ALIGN_TEXT_OVER_IMAGE|FL_ALIGN_INSIDE );
    butt ->align(FL_ALIGN_WRAP |FL_ALIGN_TEXT_OVER_IMAGE);
  }
  return 0;
}
//-------------------------------------------------------------

int CToolBox ::InitWindow(Fl_Widget  * pParent,int ID,int left,int top,int width, int height,int Style)
{
  //fl_font(FL_FREE_FONT,10);
  CFltkDialogBox::InitWindow(pParent,ID,left,top,width,height,Style);

  SetButtonImage(IDM_EXIT,19);
  SetButtonImage(IDM_UPDATE,1);
  SetButtonImage(IDM_FILEOPEN,10);
  SetButtonImage(IDM_FILESAVE,9);
  SetButtonImage(IDC_DocsBuero,3);
  SetButtonImage(IDC_DocsMedia,4);
  SetButtonImage(IDC_DocsCAD,5);
  SetButtonImage(IDC_DocsPict,6);
  SetButtonImage(IDC_DocsSrc,7);
  SetButtonImage(IDC_DocsSys,8);

  SetButtonImage(IDC_LocalSrc,  16);
  SetButtonImage(IDC_ServerSrc, 17);
  SetButtonImage(IDC_BackupSrc, 15);
  SetButtonImage(IDC_Equal,     13);
  SetButtonImage(IDC_Differ,    14);


//  InitLaden();
  return 1;
}
//-------------------------------------------------------------
void CToolBox ::Show(void)
{
  /*
    CheckDlgButton(IDC_DocsBuero,DocShowMuster & 1);
    CheckDlgButton(IDC_DocsMedia,DocShowMuster & 2);
    CheckDlgButton(IDC_DocsCAD,DocShowMuster & 4);
    CheckDlgButton(IDC_DocsPict,DocShowMuster & 8);
    CheckDlgButton(IDC_DocsSrc,DocShowMuster & 16);
    CheckDlgButton(IDC_DocsSys,DocShowMuster & 32);

  */
  CheckRadioButton(IDC_DocsBuero,IDC_DocsSys,IDC_Docs+gDocShowPrimaryPath);


  CheckRadioButton(IDC_LocalSrc,IDC_BackupSrc,IDC_LocalSrc+(gSourceAndCompareMuster & 7));


//  CheckDlgButton(IDC_LocalSrc,(CompareMuster  & 7) == 1);
//  CheckDlgButton(IDC_ServerSrc,(CompareMuster & 7) == 2);
//  CheckDlgButton(IDC_BackupSrc,(CompareMuster & 7) == 4);
  CheckDlgButton(IDC_Equal, gSourceAndCompareMuster    & MEqual);
  CheckDlgButton(IDC_Differ,gSourceAndCompareMuster    & MIgnore);
  show();
}
//-------------------------------------------------------------
//int LayerAnz=5;
bool CToolBox::ShowLayerButtons(void)
{
  return 0;
}

//-------------------------------------------------------------
int CToolBox ::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if (uMsg == WM_COMMAND) {
    switch (wParam) {
    case IDM_EXIT:
      cbExit(NULL,NULL);
      break;
    case IDM_FILEOPEN: {
//      if (ParamModify
//          && FltkMessageBox("Parameter Speichern","",MB_YESNO | MB_ICONQUESTION)== IDYES) {
//        //InitSpeichern();
//      }
      ShowTab(IDM_FILEOPEN);
      char str[_MAX_PATH];
      gBastelUtils.GetProfileString("Docs","LastFile","",str,sizeof(str));
      extern int ShowFileExplorer(char const  * szpathstr,char const  * szExtention);
      SelectTreeItem(pFileExplorer,str);
      Fl::focus(pFileExplorer);
    }
    break;
    case IDM_FILESAVE:
      ShowTab(IDM_FILESAVE);
      break;
//    case IDM_Refernezfahrt:
//      ModifyFolderStruct("/home/rolf/test/");
//    break;
    case IDM_UPDATE: {
      gCmpSheet->ScanFolder(gScanPath,gInklSubfolder);
    }
    break;
//      gWrkSheet->FileOpen(NULL);
    /*      SetAllID3Tag("");
          break;
          */
//    case IDC_DocsBuero:
//      //DocShowMuster ^= 1;
//      DocShowMuster = 1;
//      gBastelUtils.WriteProfileInt("FileCompare","ShowDocs",DocShowMuster);
//      break;
//    case IDC_DocsMedia:
//      DocShowMuster = 2;
//      gBastelUtils.WriteProfileInt("FileCompare","ShowDocs",DocShowMuster);
//      break;
//    case IDC_DocsCAD:
//      DocShowMuster = 4;
//      gBastelUtils.WriteProfileInt("FileCompare","ShowDocs",DocShowMuster);
//      break;
//    case IDC_DocsPict:
//      DocShowMuster = 8;
//      gBastelUtils.WriteProfileInt("FileCompare","ShowDocs",DocShowMuster);
//      break;
//    case IDC_DocsSrc:
//      DocShowMuster = 16;
//      gBastelUtils.WriteProfileInt("FileCompare","ShowDocs",DocShowMuster);
//      break;
//    case IDC_DocsSys:
//      DocShowMuster = 32;
//      gBastelUtils.WriteProfileInt("FileCompare","ShowDocs",DocShowMuster);
//      break;
    case IDC_Docs:
    case IDC_DocsBuero:
    case IDC_DocsMedia:
    case IDC_DocsCAD:
    case IDC_DocsPict:
    case IDC_DocsSrc:
    case IDC_DocsSys:
      gDocShowPrimaryPath = wParam - IDC_Docs;
      gBastelUtils.WriteProfileInt("FileCompare","ShowDocs",gDocShowPrimaryPath);
      break;
    case IDC_LocalSrc:
      gSourceAndCompareMuster = (gSourceAndCompareMuster &0xF8) | MLocalSrc | MEqual | MIgnore;
      gBastelUtils.WriteProfileInt("FileCompare","Compare",gSourceAndCompareMuster);
      gCmpSheet->ScanFolder(gScanPath,gInklSubfolder);
      break;
    case IDC_ServerSrc:
      gSourceAndCompareMuster = (gSourceAndCompareMuster &0xF8) | MServerSrc| MEqual | MIgnore;
      gBastelUtils.WriteProfileInt("FileCompare","Compare",gSourceAndCompareMuster);
      gCmpSheet->ScanFolder(gScanPath,gInklSubfolder);
      break;
    case IDC_BackupSrc:
      gSourceAndCompareMuster = (gSourceAndCompareMuster &0xF8) | MBackupSrc| MEqual | MIgnore;
      gBastelUtils.WriteProfileInt("FileCompare","Compare",gSourceAndCompareMuster);
      gCmpSheet->ScanFolder(gScanPath,gInklSubfolder);
      break;
    case IDC_Equal:
      gSourceAndCompareMuster ^= MEqual;
      gBastelUtils.WriteProfileInt("FileCompare","Compare",gSourceAndCompareMuster);
      gCmpSheet->ScanFolder(gScanPath,gInklSubfolder);
      break;
    case IDC_Differ:
      gSourceAndCompareMuster ^= MIgnore;
      gBastelUtils.WriteProfileInt("FileCompare","Compare",gSourceAndCompareMuster);
      gCmpSheet->ScanFolder(gScanPath,gInklSubfolder);
    }
    gCmpSheet->redraw();
    if (gSettings1->visible())gSettings1->Show();
    Show();
  }
  return 0;
//  return CDialogBox::WindowProc(uMsg,wParam,lParam);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
int gSanDirCount;
int gSanFileCount;
int gUpdatetimer;
void ResetScanStatus(void)
{
  gSanDirCount = 0;
  gSanFileCount = 0;
  gUpdatetimer = 0;
}
void IncrementFileScanStatus(void)
{
  gSanFileCount++;
  char str[100];
  if(gSanDirCount)
    sprintf(str, "SCAN: %d Dirs %d Files", gSanDirCount, gSanFileCount);
  else
    sprintf(str, "SCAN: %d Files", gSanFileCount);
  SetStatusText(str,0,0);
  int t = GetTickCount();
  if(t > gUpdatetimer + 100) {
    gUpdatetimer = t;
    Fl::lock();
    Fl::check();
    Fl::unlock();
  }
}

void IncrementDirScanStatus(void)
{
  gSanDirCount++;
  char str[100];
  if(gSanFileCount)
    sprintf(str, "SCAN: %d Dirs %d Files", gSanDirCount, gSanFileCount);
  else
    sprintf(str, "SCAN: %d Dirs", gSanDirCount);
  SetStatusText(str,0,0);
  int t = GetTickCount();
  if(t > gUpdatetimer + 100) {
    gUpdatetimer = t;
    Fl::lock();
    Fl::check();
    Fl::unlock();
  }
}
