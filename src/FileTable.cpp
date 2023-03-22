//  This file is part of the Bastel project. It is distributed under
//  GNU General Public License: https://www.gnu.org/licenses/gpl-3.0
//  Copyright (C) Bastel.Inc - All Rights Reserved

#include "FileMerge.h"
#include "FileTable.h"
#include "ProgressStatus.h"
#ifdef WIN32
#include <sys/utime.h>
#else
#include <utime.h>
#include <dirent.h>
#endif
#include "FileArchivDB.h"

//CFileTable          *G_table = 0;			// table widget
int LastR= -1;
int LastC= -1;
int bShowHidden =0;
int LVBreitenx[6]= {80,300,100,120,60,300};
const char * rgQuellText[4] = {
  "PC","Server","Media","USB"
};
const char * m_rgHeaderText[6] = {
  "Quelle","Name","Size","Datum","Type","Path"
};
const char * rgFileTypeText[7] = {
  "Home","Buro","Media","CAD","Pict","C++","Syst"
};

enum {Buro=1,Media,CAD,Pict,CSrc,Syst};

const extStruct Exclude[40] = {
  {"o",CSrc},
  {"d",CSrc},
  {"obj",CSrc},
  {"Plo",CSrc},
  {"lo",CSrc},
  {"sbr",CSrc},
  {"bsc",CSrc},
  {"exp",CSrc},
  {"ilk",CSrc},
  {"pch",CSrc},
  {"idx",CSrc},
  {"tags",CSrc},
  {"session",CSrc},
  {"supp",CSrc},
  {"json",CSrc},
  {"pdb",CSrc},
  {"ncb",CSrc},
  {"opt",CSrc},
  {"plg",CSrc},
  {"s##",CAD},
  {"b##",CAD},
  {"s#1",CAD},
  {"b#1",CAD},
  {"s#2",CAD},
  {"b#2",CAD},
  {"s#3",CAD},
  {"b#3",CAD},
  {"s#4",CAD},
  {"b#4",CAD},
  {"s#5",CAD},
  {"b#5",CAD},
  {"s#6",CAD},
  {"b#6",CAD},
  {"s#7",CAD},
  {"b#7",CAD},
  {"s#8",CAD},
  {"b#8",CAD},
  {"s#9",CAD},
  {"b#9",CAD},
  {"",0}
};
const extStruct Exttab[] = {
  {"txt",Buro},
  {"doc",Buro},
  {"exl",Buro},

  {"mp3",Media},
  {"mp4",Media},
  {"tc",Media},
  {"mpeg",Media},

  {"dxf",CAD},
  {"sch",CAD},
  {"brd",CAD},
  {"gcode",CAD},

  {"jpg",Pict},
  {"jpeg",Pict},
  {"psd",Pict},
  {"tif",Pict},
  {"tiff",Pict},
  {"png",Pict},
  {"bmp",Pict},
  {"xpm",Pict},

  {"cpp",CSrc},
  {"c",CSrc},
  {"dsp",CSrc},
  {"dsw",CSrc},
  {"cbp",CSrc},
  {"project",CSrc},
  {"workspace",CSrc},
  {"h",CSrc},
  {"lib",CSrc},

  {"ini",Syst},
  {"rc",Syst},

  {"",0}
};

char gszDiffTool[MAX_PATH] = "/home/rolf/d/wrk/WineWinMerge";
char gszEditTool[MAX_PATH] = "xed";

char gLastPath[MAX_PATH];
//---------------------------------------------------------------
CFileTable::CFileTable(int x, int y, int w, int h, const char *l) : Fl_Table/*_Row*/(x,y,w,h,l)
{
  m_Num_Header=6;
  m_Header_mod=0;
  m_SortMode = 1;
  m_SelRowMode=1;
  // Row init
  row_header(0);
  row_header_width(70);
  row_resize(1);
  rows(11);
  row_height_all(20);
  // Col init
  col_header(1);
  col_header_height(28);
  col_resize(1);
  cols(m_Num_Header);
  col_width_all(170);
  end();			// Fl_Table derives from Fl_Group, so end() it

  when(FL_WHEN_RELEASE);
  _sort_reverse = 0;
  callback(&event_callback, (void*)this);

#ifdef EDITTABLE
  // Create input widget that we'll use whenever user clicks on a cell
  input = new Fl_Input(w/2,h/2,0,0);
  input->hide();
  input->callback(input_cb, (void*)this);
  input->when(FL_WHEN_ENTER_KEY_ALWAYS);		// callback triggered when user hits Enter
  input->maximum_size(128);
  row_edit = col_edit = 0;
  s_left = s_top = s_right = s_bottom = 0;
#endif
#if FLTK_ABI_VERSION >= 10303
  tab_cell_nav(1);		// enable tab navigation of table cells (instead of fltk widgets)
#endif
  //tab_cell_nav(1);
  activate() ;
  for (int i=0; i< m_Num_Header ; i++) {
    char str[100];
    sprintf(str,"HeaderWidht_%s",m_rgHeaderText[i]);
    m_rgHeaderPos[i] = _min(_max(gBastelUtils.GetProfileInt("Table",str,LVBreitenx[i]),5),1000);
    col_width(i,LVBreitenx[i]);
  }
}

//---------------------------------------------------------------
CFileTable::~CFileTable()
{
  char str[100];
  for (int i=0; i< m_Num_Header && m_Header_mod; i++) {
    sprintf(str,"HeaderWidht_%s",m_rgHeaderText[i]);
    int w  = col_width(i);
    if (w > 20 && w < 500)
      gBastelUtils.WriteProfileInt("Table",str,w);
  }
  gBastelUtils.WriteProfileInt("Table","SortMode",m_SortMode);
};
//---------------------------------------------------------------
void CFileTable::ScanFolder(const char * pSerachpath,int bSubdirs)
{
  char str[300];
  sprintf(str,"%s - FileArchive",pSerachpath);
  MainWindow->label(str);

  SetBusyDisplay(50,"ScanFolder");

  if (gDocShowPrimaryPath>=0 && gDocShowPrimaryPath  < 7) {
    ClearCmpFileNameList();
    int src = gSourceAndCompareMuster & 3;
//    if (src == MLocalSrc) {
//      int n = strlen(g_DocsPath[0][gDocShowPrimaryPath]);
//      strcpy(str,pSerachpath);
//      strcpy(gLastPath,pSerachpath);
//      GetDirectoryContents(str,n,0,bSubdirs);
//
//    } else {
    int n = strlen(g_DocsPath[src][gDocShowPrimaryPath]);
    strcpy(str,g_DocsPath[src][gDocShowPrimaryPath]);
    strcat(str,&pSerachpath[n]);
    strcpy(gLastPath,pSerachpath);
    GetDirectoryContents(str,n,src,bSubdirs);
//    }
    //AdjustFileTable();
    sort_column(m_SortMode, _sort_reverse);
  }
  AdjustBusyDisplay(0);
  SetBusyDisplay(100,"Done");
}
//------------------------------------------------------------
BOOL CFileTable::GetDirectoryContents(char * pszDirectory,int pathlen,DWORD ID_Volume,int bSubdirs)
{
#ifdef WIN32
  WIN32_FIND_DATA findData;
  HANDLE fileHandle;
  TCHAR  str[_MAX_PATH];
  // Get the first file in the directory
  BOOL FindF=true;
  BOOL bFirstRun=true;
  int  NumFiles=0;
  // Loop on all remaining entries in the directory
  while (FindF) { // && Abbruch==false)
    if (bFirstRun) {
      bFirstRun = false;
      fileHandle = FindFirstFile(pszDirectory, &findData);
      FindF      = (fileHandle != INVALID_HANDLE_VALUE);
    } else {
      FindF  = FindNextFile(fileHandle, &findData);
    }
    //-------------- Eintrag gefunden ? -------------
    if (FindF) {
      if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        if (lstrcmpi(findData.cFileName, TEXT("."))!=0
            && lstrcmpi(findData.cFileName, TEXT(".."))!=0) {
          /*          memset(&m_Record,0,sizeof(m_Record));
                    m_Record.dwFileAttributes = findData.dwFileAttributes;
                    m_Record.ftLastWriteTime  = findData.ftLastWriteTime;
                    m_Record.nFileSize        = findData.nFileSizeLow;
                    m_Record.ParentID		    = ID_Parent;
                    m_Record.FileType         = ID_Volume;
                    strcpy(m_Record.FileName,findData.cFileName);

                    if (m_NewVol||SearchRecord()==0) SaveRecord();

                    strcpy(str,pszDirectory);
                    int len   = strlen(str);
                    if (len>3) str[len-3]='\0';
                    strcat(str,findData.cFileName);
          #ifdef FOLDER
                    m_Folder.ID = 0;
                    m_Folder.FileType         = ID_Volume;
                    strncpy(m_Folder.Name,&str[2],sizeof(m_Folder.Name));
                    if (m_NewVol||SearchFolder()) SaveFolder();
          #endif
           */
          strcat(str,"\\*.*");
          //ifEscape();
          //GetDirectoryContents(str,pathlen,ID_Volume,ID_FileType);//m_Folder.ID);
          if (bSubdirs) {
            if (GetDirectoryContents(str,pathlen,ID_Volume,bSubdirs)) {
            }
            NumFiles++;
          }
        }
      } else {
        ;
//        memset(&m_Record,0,sizeof(m_Record));
//        m_Record.dwFileAttributes = findData.dwFileAttributes;
//        m_Record.ftLastWriteTime [ID_Volume] = findData.ftLastWriteTime.dwLowDateTime;
//        m_Record.nFileSize [ID_Volume]       = findData.nFileSizeLow;
////        m_Record.ParentID		  = ID_Parent;
//        m_Record.FileType         = ID_Volume;
//
////        if (m_NewVol||SearchRecord()==0) SaveRecord();
//        strcpy(m_Record.PathName,&pszDirectory[pathlen+1]);
//        strcpy(m_Record.FileName,findData.cFileName);
//
//        if (m_NewVol||RecordExistsByPathNameVolume(ID_Volume,findData.ftLastWriteTime.dwLowDateTime,findData.nFileSizeLow,1)==0) SaveRecord(ID_Volume);
//        NumFiles++;
      }
    }
  }

  {
    if (fileHandle != INVALID_HANDLE_VALUE )
      FindClose(fileHandle);
  }

  return (NumFiles!=0);
#else
  int  Image;
  int  NumFiles=0;
  DIR  *dir;

  char str[MAX_PATH];


  dir = opendir(pszDirectory);
  if (dir) {
    struct dirent *pEntry;
    size_t path_len;//, str_size;

    path_len = strlen(pszDirectory);
    strcat(pszDirectory,"/");
    strcpy(str, pszDirectory);
    path_len++;

    while ((pEntry = readdir (dir)) != NULL) {
      struct stat statbuffer;

      strcpy(&str[path_len], pEntry->d_name);

      if (stat(str, &statbuffer)==0) {

        switch (statbuffer.st_mode & S_IFMT) {

        case S_IFCHR:     // character device

        case S_IFBLK:     // block device

        //case S_IFFIFO:    // FIFO (named pipe)

        case S_IFLNK:     // symbolic link? (Not in POSIX.1-1996.)

        case S_IFSOCK:    // socket? (Not in POSIX.1-1996.)
          break;

        case S_IFREG: {
          int flag = 1;
          if (!bShowHidden && (pEntry->d_name[0]== '.')) {
          } else {
            char * ext = strrchr(str,'.');
            if (ext) {
              ext++;
              for (int i=0; i < (sizeof(Exclude)/sizeof(Exclude[0])); i++) {
                if (Exclude[i].fType == gDocShowPrimaryPath) {
                  if (strcasecmp(Exclude[i].ext,ext)==0) {
                    flag=0;
                    break;
                  }
                }
              }
            }
            if (flag) {
              AddCmpFileName(ID_Volume,str);
              if (!bSubdirs) StepBusyDisplay();
            }
            NumFiles++;
          }
        }
        break;
        case S_IFDIR:
          /* unless ".", "..", ".hidden" or vidix driver dirs */
          if (!bShowHidden && (pEntry->d_name[0]== '.')) {
          } else {
            if ((  strcmp(pEntry->d_name,".")!=0)
                && (   strcmp(pEntry->d_name,"..")!=0)) {
              if (bSubdirs) {
                if (GetDirectoryContents(str,pathlen,ID_Volume,bSubdirs)) {
                  StepBusyDisplay();
                }
              }
            }
          }
        }
      }
    }
    closedir (dir);
  }
  return NumFiles!=0;
#endif

}

//---------------------------------------------------------------
// Handle drawing all cells in table
void CFileTable ::draw_cell(TableContext context, int R,int C, int X,int Y,int W,int H)
{
  static char s[256];
  Fl_Color fg = active()?FL_FOREGROUND_COLOR:FL_DARK1; //drawfgcolor();
  //Fl_Color bg = active()?FL_BACKGROUND2_COLOR: FL_LIGHT1;// drawbgcolor();
  Fl_Color bg = active()?FL_BACKGROUND2_COLOR:FL_LIGHT1;
  switch ( context ) {
  case CONTEXT_COL_HEADER:
    fl_font(FL_HELVETICA | FL_BOLD, 14);
    fl_push_clip(X, Y, W, H);
    {
      fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, col_header_color());
      fl_color(fg);
      fl_draw(m_rgHeaderText[C], X, Y, W, H,C<5?FL_ALIGN_CENTER:FL_ALIGN_LEFT);
      if ( C == m_SortMode ) {
        draw_sort_arrow(X,Y,W,H);
      }
    }
    fl_pop_clip();
    return;
  case CONTEXT_ROW_HEADER:
    fl_font(FL_HELVETICA | FL_BOLD, 14);
    fl_push_clip(X, Y, W, H);
    {
      Fl_Color c = (context==CONTEXT_COL_HEADER) ? col_header_color() : row_header_color();
      fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, c);
      //fl_color(FL_BLACK);
      fl_color(fg);
      // Draw text for headers
      sprintf(s, "%d", (context == CONTEXT_COL_HEADER) ? C : R);
      fl_draw(s, X, Y, W, H, FL_ALIGN_CENTER);
    }
    fl_pop_clip();
    return;
  case CONTEXT_CELL: {

#ifdef EDITTABLE
    if (R == row_edit && C == col_edit && input->visible()) {
      return;					// dont draw for cell with input widget over it
    }
#endif
    if (R < NumShowFile && rg_pCmpFile[R]->m_Quelle) {
      //int selected = /*m_SelRowMode ? row_selected(R) :*/ is_selected(R,C);
      //int selected = m_SelRowMode ?0:is_selected(R,C);
      int selected =  rg_pCmpFile[R]->m_Job & MSelect;
      if (selected ) {  //C < 2  &&
        bg = FL_SELECTION_COLOR;
        fg = FL_BACKGROUND2_COLOR;
        fl_draw_box(FL_THIN_UP_BOX,X,Y,W,H,bg);
        fl_color(fg);
      } else {
        fl_draw_box(FL_THIN_DOWN_BOX,X,Y,W,H,bg);
        fl_color(rg_pCmpFile[R]->m_FileType==gDocShowPrimaryPath ?FL_CYAN:fg);
      }


      //if (R < NumCmpFile && rg_pCmpFile[R]->m_Quelle) {
      // Draw text for the cell
      fl_push_clip(X+3, Y+3, W-6, H-6);
      fl_font(FL_HELVETICA, 14);
      extern int gDocShowPrimaryPath;
      /*      int m = rg_pCmpFile[R]->m_FileType-1;
            m = 1<<m;
            fl_color((m & DocShowMuster)!=0 ?FL_CYAN:fg);
      */
      sprintf(s, "%d", R*C);
      switch (C) {
      case 0: {
        DWORD q = 0;
#ifndef DYNARRY
        q = FileArchivDB->CompareFile(0,rg_pCmpFile[R]->m_Path,rg_pCmpFile[R]->m_Name);
#else
        q = rg_pCmpFile[R]->CompareFile(0);
#endif
        (*(DWORD*)&rg_pCmpFile[R]->m_Quelle) = q;
        int w4 = (W-6)/4;
//        const char StateClors[16] {
//          FL_DARK3,  //1 NOFILE
//          FL_GREEN, //2 EQUAL
//          0,
//          FL_YELLOW, //4 TIMEDIFF
//          0,0,0,
//          FL_RED,    //8 SIZEDIFF
//          0,0,0,0,0,0,0,0
//        };
//        fl_rectf(X+3+w4*0, Y+3, w4, H-6,StateClors[rg_pCmpFile[R]->m_Quelle[0]&0xF]);//FL_DARK_YELLOW);
//        fl_rectf(X+3+w4*1, Y+3, w4, H-6,StateClors[rg_pCmpFile[R]->m_Quelle[1]&0xF]);//FL_GREEN);
//        fl_rectf(X+3+w4*2, Y+3, w4, H-6,StateClors[rg_pCmpFile[R]->m_Quelle[2]&0xF]);//FL_DARK_YELLOW);
//        fl_rectf(X+3+w4*3, Y+3, w4, H-6,StateClors[rg_pCmpFile[R]->m_Quelle[3]&0xF]);//FL_RED);
        fl_rectf(X+3+w4*0, Y+3, w4, H-6,rg_pCmpFile[R]->m_Quelle[0]);
        fl_rectf(X+3+w4*1, Y+3, w4, H-6,rg_pCmpFile[R]->m_Quelle[1]);
        fl_rectf(X+3+w4*2, Y+3, w4, H-6,rg_pCmpFile[R]->m_Quelle[2]);
        fl_rectf(X+3+w4*3, Y+3, w4, H-6,rg_pCmpFile[R]->m_Quelle[3]);

        fl_color(FL_WHITE);
        fl_draw("L", X+3+(w4*0), Y+3, w4, H-6,FL_ALIGN_CENTER);
        fl_draw("S", X+3+(w4*1), Y+3, w4, H-6,FL_ALIGN_CENTER);
        fl_draw("B", X+3+(w4*2), Y+3, w4, H-6,FL_ALIGN_CENTER);
        fl_draw("U", X+3+(w4*3), Y+3, w4, H-6,FL_ALIGN_CENTER);

        //fl_draw("L-S-B-U", X+3, Y+3, W-6, H-6,FL_ALIGN_CENTER);
      }
      break;
      case 1:
        if (rg_pCmpFile[R]->m_Name) {
          strcpy(s,rg_pCmpFile[R]->m_Name);
          fl_draw(s, X+3, Y+3, W-6, H-6,FL_ALIGN_LEFT);
        }
        break;
      case 5:
        if (rg_pCmpFile[R]->m_Path) {
          strcpy(s,rg_pCmpFile[R]->m_Path);
          fl_draw(s, X+3, Y+3, W-6, H-6,FL_ALIGN_LEFT);

        }
        break;
      case 2: {
        long z = rg_pCmpFile[R]->m_Size;
        if (z > 1000000) {
          sprintf(s,"%ldK",z/1000);
        } else {
          sprintf(s,"%ld ",z);
        }
        fl_draw(s, X+3, Y+3, W-6, H-6,FL_ALIGN_RIGHT);
      }
      break;
      case 3: {
        struct tm * tm1 = localtime(&rg_pCmpFile[R]->m_Datum);
        strftime(s,sizeof(s),"%d.%m.%g %H:%M",tm1);
        fl_draw(s, X+3, Y+3, W-6, H-6,FL_ALIGN_CENTER);
      }
      break;
      case 4:
        strcpy(s,rgFileTypeText[rg_pCmpFile[R]->m_FileType]);
        fl_draw(s, X+3, Y+3, W-6, H-6,FL_ALIGN_RIGHT);
        break;
      }
      fl_pop_clip();
    }
    return;
  }
  default:
    return;
  }
}


#ifdef EDITTABLE
//---------------------------------------------------------------
// Start editing a new cell: move the Fl_Int_Input widget to specified row/column
//    Preload the widget with the cell's current value,
//    and make the widget 'appear' at the cell's location.
//
void CFileTable::start_editing(int R, int C)
{
  row_edit = R;					// Now editing this row/col
  col_edit = C;
  int X,Y,W,H;
  find_cell(CONTEXT_CELL, R,C, X,Y,W,H);		// Find X/Y/W/H of cell
  input->resize(X,Y,W-1,H-1);				// Move Fl_Input widget there

  //if (R>=0 && R < NumCmpFile && rg_pCmpFile[R]->m_Quelle) {
  if (R>=0 && R < NumCmpFile && rg_pCmpFile[R]->m_Quelle) {
    static char s[256];
    switch (C) {
    case 0:
      break;
    case 1:
      strcpy(s,rg_pCmpFile[R]->m_Name);
      break;
    case 2:
      strcpy(s,"");//rg_pCmpFile[R]->m_Quelle);
      break;
    case 3:
      strcpy(s,"");//rg_pCmpFile[R]->m_Size);
      break;
    case 4:
      strcpy(s,"");//,rg_pCmpFile[R]->m_Datum);
      break;
    }

    input->value(s);
    input->position(0,strlen(s));			// Select entire input field
  }
  input->show();					// Show the input widget, now that we've positioned it
  input->take_focus();
}

//---------------------------------------------------------------
// Tell the input widget it's done editing, and to 'hide'
void CFileTable::done_editing()
{
  if (input->visible()) {				// input widget visible, ie. edit in progress?
    set_value_hide();					// Transfer its current contents to cell and hide
  }
}

//---------------------------------------------------------------
// Apply value from input widget to values[row][col] array and hide (done editing)
void CFileTable::set_value_hide()
{
  if (row_edit < NumCmpFile) {
    static char s[256];
    strcpy(s,input->value());
    CCmpFile * mp3pnt = rg_pCmpFile[row_edit];
    switch (col_edit) {
    case 0:
      break;
    case 1:
      //if (strcmp(s,mp3pnt->m_Name)!=0) mp3pnt->m_Modify |= (1 << col_edit);
      //if (mp3pnt->m_Name) delete mp3pnt->m_Name;
      //mp3pnt->m_Name= strdup(s);
      break;
    case 2:
      break;
    case 3:
      break;
    case 4:
      break;
    }
    input->value(s);
    input->position(0,strlen(s));			// Select entire input field
  }

  input->hide();
  window()->cursor(FL_CURSOR_DEFAULT);		// XXX: if we don't do this, cursor can disappear!
}

//---------------------------------------------------------------
// Update the displayed sum value
int CFileTable::GetSelectionSum()
{
  int sum = -1;
  for ( int R=0; R<rows(); R++ ) {
    for ( int C=0; C<cols(); C++ ) {
      if (m_SelRowMode ? row_selected(R) : is_selected(R,C) ) {
//if (is_selected(R,C) ) {
        if ( sum == -1 ) sum = 0;
        sum += R*C;
      }
    }
  }
  return(sum);
}

//---------------------------------------------------------------
// Update the "Selection sum:" display
void CFileTable::UpdateSum()
{
  static char s[80];
  int sum = GetSelectionSum();
  //if ( sum == -1 ) { sprintf(s, "(nothing selected)"); G_sum->color(48); }
  //else             { sprintf(s, "%d", sum); G_sum->color(FL_WHITE); }
  // Update only if different (lets one copy/paste from sum)

}
#endif

//-----------------------------
int Compare_SortKeyA( const void *arg1, const void *arg2 )
{
  CCmpFile * pCnA = *(CCmpFile ** )arg1;
  CCmpFile * pCnB = *(CCmpFile ** )arg2;
  int res = pCnB->m_SortKey - pCnA->m_SortKey;
  return (int)res;
}
//-----------------------------
int Compare_SortKey( const void *arg1, const void *arg2 )
{
  CCmpFile * pCnA = *(CCmpFile ** )arg1;
  CCmpFile * pCnB = *(CCmpFile ** )arg2;
  int res = pCnA->m_SortKey - pCnB->m_SortKey;
  return (int)res;
}
static int Ascend=1;
//-----------------------------
int Compare_State( const void *arg1, const void *arg2 )
{
  CCmpFile * pCnA = *(CCmpFile ** )arg1;
  CCmpFile * pCnB = *(CCmpFile ** )arg2;
  //int res = pCnB->m_Quelle[3] - pCnA->m_Quelle[3];
  int res = (pCnB->m_SortKey&MStateMask) - (pCnA->m_SortKey&MStateMask);
  if (res == 0) {
    res = pCnB->m_Datum - pCnA->m_Datum;
    if (res==0) {
      res = pCnB->m_Size - pCnA->m_Size;
//      if (res==0) {
//        res = pCnB->m_RecID- pCnA->m_RecID;
//      }
    }
  }
  return (int)res*Ascend;
}
//-----------------------------
int Compare_Date( const void *arg1, const void *arg2 )
{
  CCmpFile * pCnA = *(CCmpFile ** )arg1;
  CCmpFile * pCnB = *(CCmpFile ** )arg2;
  long res = pCnB->m_Datum - pCnA->m_Datum;
  if (res==0) {
    res = pCnB->m_Quelle[3] - pCnA->m_Quelle[3];
    if (res == 0) {
      res = pCnB->m_Size - pCnA->m_Size;
//      if (res==0) {
//        res = pCnB->m_RecID- pCnA->m_RecID;
//      }
    }
  }
  return (int)res*Ascend;
}
//-----------------------------
int Compare_Size( const void *arg1, const void *arg2 )
{
  CCmpFile * pCnA = *(CCmpFile ** )arg1;
  CCmpFile * pCnB = *(CCmpFile ** )arg2;
  int res = pCnB->m_Size - pCnA->m_Size;
  if (res==0) {
    res = pCnB->m_Datum - pCnA->m_Datum;
    if (res==0) {
      res = pCnB->m_Quelle[3]- pCnA->m_Quelle[3];
//      if (res == 0) {
//        res = pCnB->m_RecID- pCnA->m_RecID;
//      }
    }
  }
  return (int)res*Ascend;
}
//-----------------------------
int Compare_Vol( const void *arg1, const void *arg2 )
{
  CCmpFile * pCnA = *(CCmpFile ** )arg1;
  CCmpFile * pCnB = *(CCmpFile ** )arg2;
  int res = pCnB->m_FileType - pCnA->m_FileType;
//  if (res==0) {
//    res = pCnB->m_RecID- pCnA->m_RecID;
//  }
  return (int)res*Ascend;
}
//-----------------------------
int Compare_Name( const void *arg1, const void *arg2 )
{
  CCmpFile * pCnA = *(CCmpFile ** )arg1;
  CCmpFile * pCnB = *(CCmpFile ** )arg2;
  int res =0;
  if (pCnB->m_Name && pCnA->m_Name) {
    res = strcmp(pCnB->m_Name,pCnA->m_Name);
  }
  if (res==0) {
    res = pCnB->m_FileType - pCnA->m_FileType;
  }
  return (int)res*Ascend;
}
//-----------------------------
int Compare_Path( const void *arg1, const void *arg2 )
{
  CCmpFile * pCnA = *(CCmpFile ** )arg1;
  CCmpFile * pCnB = *(CCmpFile ** )arg2;
  int res =0;
  if (pCnB->m_Path && pCnA->m_Path) {
    res = strcmp(pCnB->m_Path,pCnA->m_Path);
  }
  if (res==0 && pCnB->m_Name && pCnA->m_Name) {
    res = strcmp(pCnB->m_Name,pCnA->m_Name);
  }
  if (res==0) {
    res = pCnB->m_FileType - pCnA->m_FileType;
  }
  return (int)res*Ascend;
}
//-----------------------------
void CFileTable::sort_column(int sortmode, int reverse)
{

  if ((gSourceAndCompareMuster & MEqual) && !(gSourceAndCompareMuster & MIgnore)) {
//    Ascend = 1;
    qsort(rg_pCmpFile,NumCmpFile,sizeof(CCmpFile*),Compare_SortKeyA);
    NumShowFile=0;
    for (int i=0; i< NumCmpFile; i++) {
      if ((rg_pCmpFile[i]->m_SortKey) >0) NumShowFile=i+1;
    }
    rows(NumShowFile);
  } else if (!(gSourceAndCompareMuster & MEqual) && (gSourceAndCompareMuster & MIgnore)) {
//    Ascend = 0;
    qsort(rg_pCmpFile,NumCmpFile,sizeof(CCmpFile*),Compare_SortKey);
    NumShowFile=0;
    for (int i=0; i< NumCmpFile; i++) {
      if (rg_pCmpFile[i]->m_SortKey == 0) NumShowFile=i+1;
    }
    rows(NumShowFile);
  } else {
    NumShowFile=NumCmpFile;
    rows(NumShowFile);
  }


  Ascend = reverse?1:-1;
  //"Quelle","Name","Size","Datum","Type","Path"
  switch (sortmode) {
  case 0:
    qsort(rg_pCmpFile,NumShowFile,sizeof(CCmpFile*),Compare_State);
    break;
  case 1:
    qsort(rg_pCmpFile,NumShowFile,sizeof(CCmpFile*),Compare_Name);
    break;
  case 2:
    qsort(rg_pCmpFile,NumShowFile,sizeof(CCmpFile*),Compare_Size);
    break;
  case 3:
    qsort(rg_pCmpFile,NumShowFile,sizeof(CCmpFile*),Compare_Date);
    break;
  case 4:
    qsort(rg_pCmpFile,NumShowFile,sizeof(CCmpFile*),Compare_Vol);
    break;
  case 5:
    qsort(rg_pCmpFile,NumShowFile,sizeof(CCmpFile*),Compare_Path);
    break;
  }
  AdjustFileTable();
  //redraw();

}
//-------------------------------------------------------------
void CFileTable::draw_sort_arrow(int X,int Y,int W,int H)
{
  int xlft = X+(W-6)-8;
  int xctr = X+(W-6)-4;
  int xrit = X+(W-6)-0;
  int ytop = Y+(H/2)-4;
  int ybot = Y+(H/2)+4;
  if ( _sort_reverse ) {
    // Engraved down arrow
    fl_color(FL_WHITE);
    fl_line(xrit, ytop, xctr, ybot);
    fl_color(41);                   // dark gray
    fl_line(xlft, ytop, xrit, ytop);
    fl_line(xlft, ytop, xctr, ybot);
  } else {
    // Engraved up arrow
    fl_color(FL_WHITE);
    fl_line(xrit, ybot, xctr, ytop);
    fl_line(xrit, ybot, xlft, ybot);
    fl_color(41);                   // dark gray
    fl_line(xlft, ybot, xctr, ytop);
  }
}
//-------------------------------------------------------------
void CFileTable::event_callback(Fl_Widget*, void *data)
{
  CFileTable *o = (CFileTable*)data;
  o->event_callback2();
}
//-------------------------------------------------------------
static unsigned int startTickCount;

void CFileTable::event_callback2()
{
  static int validMove;
  int ROW = callback_row();
  int COL = callback_col();
  TableContext context = callback_context();
  switch ( context ) {
  case CONTEXT_COL_HEADER: {              // someone clicked on column header
    if ( Fl::event() == FL_RELEASE && Fl::event_button() == 1 ) {
      if ( m_SortMode == COL ) {   // Click same column? Toggle sort
        _sort_reverse ^= 1;
      } else {                        // Click diff column? Up sort
        _sort_reverse = 0;
      }
      sort_column(COL, _sort_reverse);
      m_SortMode = COL;
    }
    LastR = -1;
    LastC = -1;

    break;
    case CONTEXT_ROW_HEADER: {              // someone clicked on column header
      LastR = -1;
      LastC = -1;
    }
    break;
    case CONTEXT_CELL:
      if (Fl::event()==FL_PUSH) {
        int t = GetTickCount();
        validMove = (t > startTickCount+600);
        startTickCount= t;

        int multi = Fl::event_ctrl() != 0;
        if (Fl::event_shift())  multi = 2;
        int start = -1;  // noch nicht begonnen
        for (int i=0; i < NumCmpFile; i++) {
          int mod =0;
          if (start == -1 && (rg_pCmpFile[i]->m_Job&MSelect) == MSelect) start = i;
          if (i==ROW) {
            if ((rg_pCmpFile[ROW]->m_Job&MSelect) == MSelect)
              rg_pCmpFile[ROW]->m_Job&= ~(MSelect);
            else
              rg_pCmpFile[ROW]->m_Job|=  (MSelect);
            start = -2; // ende der multi selection
            mod = 1;
          } else if (multi==2) {
            if (start>=0) {
              mod = !(rg_pCmpFile[i]->m_Job & MSelect);
              rg_pCmpFile[i]->m_Job|=  (MSelect);
            }
          } else if (multi==0) {
            mod = (rg_pCmpFile[i]->m_Job & MSelect);
            rg_pCmpFile[i]->m_Job&= ~(MSelect);
          }
          //redraw_range(i,i,0,6);
          if (mod) redraw_range(i,i,0,6);
        }
        break;
      } else if (Fl::event()==FL_RELEASE ) {
        if (validMove ) {
          LastR = ROW;
          LastC = COL;
        } else {
          startTickCount= 0;
          if (LastR == ROW &&  LastC == COL) {
            if (ROW<NumCmpFile) {
              CCmpFile   * object = rg_pCmpFile[ROW];
              extern char szSysCmmand[MAX_PATH*3];
              //char szDiffTool[MAX_PATH] = "diffuse";
              //char szDiffTool[MAX_PATH] = "meld";
              if (object ->m_Quelle[0] != NOFILE  && object ->m_Quelle[1] != NOFILE ) {
                sprintf(szSysCmmand,"%s %s%s/%s %s%s/%s",gszDiffTool,
                        g_DocsPath[0][gDocShowPrimaryPath],object ->m_Path,object ->m_Name,
                        g_DocsPath[1][gDocShowPrimaryPath],object ->m_Path,object ->m_Name);
              } else  {
                int i = (object ->m_Quelle[0]!=NOFILE)?0:1;
                sprintf(szSysCmmand,"%s %s%s/%s",gszEditTool,
                        g_DocsPath[i][gDocShowPrimaryPath],object ->m_Path,object ->m_Name);
              }
            }
          }
          LastR = -1;
          LastC = -1;
        }
      }
      break;
    }
  default:
    return;
  }

}
//-------------------------------------------------------------
#if 0
static int scroll_start  = 0;
static int scroll_startx = 0;
static int scroll_min  = 0;
static int scroll_max  = 0;
static int start_fingerx = 0;
static int start_fingery = 0;
static int kpix_per_ms;
static int fingermovecnt = 0;
//-------------------------------------------------------------
void FileTableTimerCb(void *data)
{
  CTreeView * pTreeView = (CTreeView*)data;
  if (fingermovecnt) {
    if (fingermovecnt>0) {
      fingermovecnt--;
      scroll_start -= (kpix_per_ms / 100);
      if (scroll_start < scroll_min) {
        scroll_start=scroll_min;
        fingermovecnt=0;
      }
    } else if (fingermovecnt< 0) {
      fingermovecnt++;
      scroll_start -= (kpix_per_ms / 100);
      if (scroll_start > scroll_max) {
        scroll_start=scroll_max;
        fingermovecnt=0;
      }
    }
    pTreeView ->scroll_to(0,scroll_start);
    Fl::repeat_timeout(0.005, FileTableTimerCb, data);
  }
}
#endif
//---------------------------------------------------------------
// Keyboard and mouse events
int CFileTable::handle(int e)
{
  if (1) {
    int ret = Fl_Table/*_Row*/::handle(e);
#if 0
    static int validMove;
    switch (e) {
    case FL_PUSH : {
      Fl::focus(this);
      fingermovecnt = 0;
      scroll_start  = vscrollbar->value();
      scroll_startx = hscrollbar->value();
      scroll_min    = 0;
      scroll_max    = h()-y();
      start_fingerx = Fl::event_x();
      start_fingery = Fl::event_y();
      int t = GetTickCount();
      validMove = (t > startTickCount+600);
      startTickCount= t;
      return (1);
    }
    case FL_DRAG :
      if (validMove && Fl::event_button()== FL_LEFT_MOUSE) {
        int valy = Fl::event_y()-start_fingery;
        int valx = scroll_startx - (Fl::event_x()-start_fingerx);
        int maxx =w()*2/4;
        if (valx <0) valx=0;
        else if (valx > maxx) valx=maxx;
        //scroll_to(valx,scroll_start-valy);
        vscrollbar->value(scroll_start-valy);
        hscrollbar->value(valx);
        redraw();
//      row_position();                   // set/get table's current scroll position
        //    col_position();

      }
      return (1);
      break;
    /*  case FL_MOVE:
    break;*/
    case FL_RELEASE : {
      int valx = Fl::event_x()-start_fingerx;
      int valy = Fl::event_y()-start_fingery;
      if (fingermovecnt) {
        fingermovecnt=0;      // Stop
      }
      if (validMove ) {
        if (abs (valx) > abs(valy)) {
          if (valx > 50) {
            //              tMessage * pmsg = (tMessage *)malloc(sizeof(tMessage ));
            //              pmsg->Code = IDM_PlayListTab;
            //              pmsg->Value= 0;
            //              pmsg->receiver = gRadioBerry;
            //              Fl::awake(pmsg);
          }
        } else {
          /*if (valy < 10 && valy > -10) {
            CTVItem * p = GetItem(Fl::event_y()-y());
            if (p) {
              int ix = FindItemIndex(p);
              OnClick(p->m_Item_ID,0,Fl::event_x()-x());
              SelChanged(ix);
              p->redraw();
            }

          } else
          */
          {
            int t = (GetTickCount() - startTickCount);
            if (t < 1000) {
              int val = Fl::event_y()-start_fingery;
              scroll_start  = vscrollbar->value();
              if (t) {
                kpix_per_ms = val*1000 / t;
                Fl::repeat_timeout(0.01, FileTableTimerCb, this);
                fingermovecnt = (val * abs(val)) / (t);
              }
            }
          }
        }
        redraw();
      } else {
        startTickCount=0;
        /*CTVItem * p = GetItem(Fl::event_y()-y());
        if (p) {
          int ix = FindItemIndex(p);
          OnClick(p->m_Item_ID,1,Fl::event_x()-x());
          p->redraw();
        }
        */

      }
      if (Fl::event_button() == FL_RIGHT_MOUSE) {
      }

      return 1;
    }
    break;
    }
    int ret = Fl_Table/*_Row*/::handle(e);
#endif
#ifdef EDITTABLE
    switch (e) {
    case FL_KEYBOARD: {
      if (Fl::event_key() == FL_Escape ) exit(0);
      if (Fl::event_key() == FL_Insert || Fl::event_key() == (FL_F+2)) {
        int R = callback_row();
        int C = callback_col();
        //  TableContext context = callback_context();
        done_editing();				    // finish any previous editing
        set_selection(R, C, R, C);		// select the current cell
        start_editing(R,C);				// start new edit
        if (Fl::event() == FL_KEYBOARD && Fl::e_text[0] != '\r') {
          input->handle(Fl::event());			// pass keypress to input widget
        }
      }
      return 1;
    }


    case FL_PUSH: {
      done_editing();
      int R = callback_row();
      int C = callback_col();
      if (R==LastR && C == LastC) {
        set_selection(R, C, R, C);		// select the current cell
        start_editing(R,C);				// start new edit
      } else {
        focus(this);
      }
      LastR = R;
      LastC = C;
    }
    case FL_RELEASE:
    case FL_KEYUP:
    case FL_DRAG: {
      //ret = 1;		// *don't* indicate we 'handled' these, just update ('handling' prevents e.g. tab nav)
      UpdateSum();
      redraw();
      break;
    }
    case FL_FOCUS:		// tells FLTK we're interested in keyboard events
    case FL_UNFOCUS:
      ret = 1;
      break;
    }
    return(ret);
  } else {
    int ret = Fl_Table_Row::handle(e);
    if ( e == FL_KEYBOARD && Fl::event_key() == FL_Escape ) exit(0);
    switch (e) {
    case FL_PUSH:
    case FL_RELEASE:
    case FL_KEYUP:
    case FL_KEYDOWN:
    case FL_DRAG: {
      //ret = 1;		// *don't* indicate we 'handled' these, just update ('handling' prevents e.g. tab nav)
      UpdateSum();
      redraw();
      break;
    }
    case FL_FOCUS:		// tells FLTK we're interested in keyboard events
    case FL_UNFOCUS:
      ret = 1;
      break;
    }
#endif
    return(ret);
  }
}

//---------------------------------------------------------------
int CFileTable::mainInit()
{
  for (int s=0; s < 3; s++) {
    for (int i=0; i < sizeof(g_DocsPath[0])/sizeof(g_DocsPath[0][0]); i++) {
      //  gBastelUtils.WriteProfileString("Docs",rgFileTypeText[i],File);
      gBastelUtils.GetProfileString(rgQuellText[s],rgFileTypeText[i],"/home/rolf",g_DocsPath[s][i],sizeof(g_DocsPath[0][0]));

      //m_rgHeaderPos[i] = _min(_max(gBastelUtils.GetProfileInt("Table",str,LVBreitenx[i]),5),1000);

      //char m_DocsPath[7][MAX_PATH];
    }

  }
  //G_table=this;
  return 1;
}

//---------------------------------------------------------------
int CCmpFile::SetFileName(int ID_Volume,char * lpfName)
{
  struct stat statbuffer;
  if (gDocShowPrimaryPath>=0 && gDocShowPrimaryPath<7) {
    int n = strlen(g_DocsPath[ID_Volume][gDocShowPrimaryPath]);
    int n2 = strlen(lpfName);
    if (n2 > n && strncmp(g_DocsPath[ID_Volume][gDocShowPrimaryPath],lpfName,n)==0) {
      m_Path = strdup(&lpfName[n]);
    } else m_Path = strdup(lpfName);
  } else m_Path = strdup(lpfName);

  m_Name=NULL;
  if (stat(lpfName, &statbuffer)==0) {
    //strncpy(FileArchivDB->m_Record.FileName,m_Path,sizeof(FileArchivDB->m_Record.FileName));

    m_Name  = strrchr(m_Path,'/');
    if (m_Name) {
      *m_Name = '\0';
      m_Name++;
    } else m_Name = m_Path;
    m_Quelle[0] = FL_DARK1;
    m_Quelle[1] = FL_DARK2;
    m_Quelle[2] = FL_DARK1;
    m_Quelle[3] = FL_DARK2;
    m_Size  = statbuffer.st_size;
    //m_Datum = statbuffer.st_ctime;
    m_Datum = statbuffer.st_mtime;
    m_FileType=0;
    char * ext = strrchr(m_Name,'.');
    if (ext) {
      ext++;
      for (int i=0; i < (sizeof(Exttab)/sizeof(Exttab[0])); i++) {
        if (strcasecmp(Exttab[i].ext,ext)==0) {
          m_FileType = Exttab[i].fType;
        }
      }
    } else {
      if (strcasecmp(m_Name,"Makefile")==0) {
        m_FileType = CSrc;
      } else  {
        m_FileType = Syst;
      }
    }
    //FileArchivDB->GetNextFileMatch(1,1);
    /*    if (FileArchivDB->m_Record.nFileSize != statbuffer.st_size) {
          m_Quelle[1] = FL_RED;
        } else if (FileArchivDB->m_Record.ftLastWriteTime != statbuffer.st_mtime) {
          m_Quelle[1] = FL_YELLOW;
        } else {
          m_Quelle[1] = FL_DARK_GREEN;
        }*/
  }
  return 0;
}


//---------------------------------------------------------------
int NumCmpFile = 0;
int NumShowFile= 0;
int NumLeftOnlyFile  = 0;
int NumRightOnlyFile = 0;
int NumDiffFile = 0;
#ifdef DYNARRY
int NumCmpFileSize=0;
CCmpFile  ** rg_pCmpFile;
#else
int NumCmpFileSize=NUMFILES;
CCmpFile  * rg_pCmpFile[NUMFILES];
#endif
//---------------------------------------------------------------
void AddCmpFileName(int ID_Volume,char * lpfName)
{
  char * ext = strrchr(lpfName,'.');
  if (ext) {
    ext++;
    for (int i=0; i < (sizeof(Exclude)/sizeof(Exclude[0])); i++) {
      if (Exclude[i].fType ==gDocShowPrimaryPath) {
        if (strcasecmp(Exclude[i].ext,ext)==0) {
          return ;
        }
      }
    }
  }
#ifdef DYNARRY
  if (NumCmpFile >= (NumCmpFileSize-1)) {
    rg_pCmpFile = (CCmpFile **)realloc(rg_pCmpFile,((NumCmpFileSize+4000) *sizeof(CCmpFile *) ));
    if (rg_pCmpFile) NumCmpFileSize+=4000;
    else NumCmpFileSize = 0;
  }
#endif
  if (NumCmpFile < NumCmpFileSize ) { //NUMFILES) {
    CCmpFile * Object = (CCmpFile*) new CCmpFile();
    Object->SetFileName(ID_Volume,lpfName);
#ifdef DYNARRY
    DWORD q = Object->CompareFile(ID_Volume);
#else
    DWORD q = FileArchivDB->CompareFile(ID_Volume,Object->m_Path,Object->m_Name);
#endif
    (*(DWORD*)&Object->m_Quelle) = q;

    //Object ->m_Quelle[3]= ITEM_VISIBLE;
    if (Object->m_Quelle[0]== NOFILE && Object->m_Quelle[1]!= NOFILE ) {
      NumRightOnlyFile++;
//      Object ->m_dwShow |= MServerSrc | ITEM_FAIL;
    } else if (Object->m_Quelle[0]!= NOFILE && Object->m_Quelle[1]== NOFILE ) {
      //    Object ->m_dwShow |= MLocalSrc | ITEM_FAIL;
      NumLeftOnlyFile++;
    }
    if (Object->m_Quelle[0]== EQUAL  && Object->m_Quelle[1]== EQUAL ) {
//      Object ->m_dwShow |= MEqual |ITEM_OK ;
    } else {
//      Object ->m_dwShow |= MDiffer| ITEM_FAIL;
      NumDiffFile++;
    }

#ifdef DYNARRY
    rg_pCmpFile[NumCmpFile] = Object;
    NumCmpFile++;
    NumShowFile = NumCmpFile;
#else
    if ((gSourceAndCompareMuster&MIgnore) || (Object ->m_Job & (gSourceAndCompareMuster & MShowMask))) {
      rg_pCmpFile[NumCmpFile] = Object;
      NumCmpFile++;
      NumShowFile = NumCmpFile;
    } else {
      delete (Object);
    }
#endif
    if ((NumCmpFile&0x3F)==0) {
      char s[200];
      //sprintf(s,"%s LIST %3d, Lo %2d, Ro %2d, Dif %2d",(NumCmpFile>=NUMFILES)?"OVERLOAD":"",NumCmpFile,NumLeftOnlyFile,NumRightOnlyFile,NumDiffFile);
      sprintf(s,"LIST %3d, Lo %2d, Ro %2d, Dif %2d",NumCmpFile,NumLeftOnlyFile,NumRightOnlyFile,NumDiffFile);
      SetStatusText(s,1,0 );
    }
  }
}

BOOL CCmpFile::CompareFile(int BasisID)
{
  struct stat statbuffer;
  char str[MAX_PATH];
  unsigned char quelle[4] = {NOFILE,NOFILE,NOFILE,NOFILE };   //{FL_DARK1,FL_DARK1,FL_DARK1,FL_DARK1};
  time_t   ftLastWriteTime[4]= {0,0,0,0};
  long     nFileSize[4]= {0,0,0,0};
  time_t tneuste=0;
  int    ineuste = BasisID;
  long   sneuste=0;

  if (gDocShowPrimaryPath>=0 && gDocShowPrimaryPath  < 7) {
    for (int b=0; b< 3; b++) {
      strcpy(str,g_DocsPath[b][gDocShowPrimaryPath]);
      strcat(str,"/");
      strcat(str,m_Path);
      strcat(str,"/");
      strcat(str,m_Name);

      if (stat(str, &statbuffer)==0) {
        ftLastWriteTime[b] = statbuffer.st_mtime;
        nFileSize[b]= statbuffer.st_size;
        if (ftLastWriteTime[b]>tneuste) {
          tneuste = ftLastWriteTime[b];
          sneuste = nFileSize[b];
          ineuste = b;
        }
      }
    }
    int EQUALTDcnt=0;
    int EQUALTcnt=0;
    for (int i=0; i< 3; i++) {
      time_t diff = tneuste - ftLastWriteTime[i];
      if (ftLastWriteTime[i] == 0) {
        quelle[i] = NOFILE;
      } else if (nFileSize[i] == 0) {
        quelle[i] = NOFILE;
      } else if (nFileSize[i] != sneuste) {
        quelle[i] = SIZEDIFF;
      } else if (diff==3600 || diff==-3600) {
        // Zeit-Zone ?
        quelle[i] = EQUALTD;
        EQUALTDcnt++;
      } else if (diff!=0) {
        quelle[i] = TIMEDIFF;
      } else {
        quelle[i] = EQUAL;
        EQUALTcnt++;
      }
    }
    if       (EQUALTcnt==3)              /*quelle[3] = EQUAL   ,*/m_SortKey=NumCmpFile+1;
    else if ((EQUALTcnt +EQUALTDcnt)==3) /*quelle[3] = EQUALTD ,*/m_SortKey=NumCmpFile+1;
    else                                 /*quelle[3] = SIZEDIFF,*/m_SortKey=0;
    /*int l = strlen(m_Path);
    l += strlen(m_Name);
    if (l < sizeof(m_Record.FileName)-1) {
      if (GetRecordByPathAndName(lpfPath,lpfName)) {
        time_t tneuste = statbuffer.st_mtime;
        int    ineuste = BasisID;
        long   sneuste = statbuffer.st_size;
        int i;
        for (i=0; i< 4; i++) {
          if (i!=BasisID) {
            if (m_Record.ftLastWriteTime[i]>tneuste) {
              tneuste = m_Record.ftLastWriteTime[i];
              sneuste = m_Record.nFileSize[i];
              ineuste = i;
            }
          } else {
            if (m_Record.ftLastWriteTime[i] != statbuffer.st_mtime ||
                m_Record.nFileSize[i]       != statbuffer.st_size) {
              // Datensatz und Wirklichkeit sind unterschiedlich
              m_Record.ftLastWriteTime[i] = statbuffer.st_mtime;
              m_Record.nFileSize[i]       = statbuffer.st_size;
              UpdateRecordFileTime(BasisID,m_Record.ID,statbuffer.st_mtime,statbuffer.st_size);
            }
          }
        }
        for (i=0; i< 4; i++) {
          time_t diff = tneuste - m_Record.ftLastWriteTime[i];
          if (m_Record.ftLastWriteTime[i] == 0) {
            quelle[i] = NOFILE;
          } else if (m_Record.nFileSize[i] == 0) {
            quelle[i] = NOFILE;
          } else if (m_Record.nFileSize[i] != sneuste) {
            quelle[i] = SIZEDIFF;
          } else if (diff==3600 || diff==-3600) {
            // Zeit-Zone ?
            quelle[i] = EQUALTD;
          } else if (diff!=0) {
            quelle[i] = TIMEDIFF;
          } else {
            quelle[i] = EQUAL;
          }
        }
      } else {
        // Nicht in DB, nur als File Vorhanden
        quelle[BasisID] = SIZEDIFF;
      }
    }*/
  }
  return *((DWORD*)&quelle);
}
//---------------------------------------------------------------
CCmpFile::CCmpFile()
{
  m_Quelle[0] = FL_DARK1;
  m_Quelle[1] = FL_DARK2;
  m_Quelle[2] = FL_DARK1;
  m_Quelle[3] = FL_DARK2;
  m_Path = NULL;
  m_Name=NULL;
  m_Size=0;
  m_FileType=0;
  m_Datum=0;
  m_SortKey =0;
  m_Job     = 0;
}
//---------------------------------------------------------------
CCmpFile::~CCmpFile()
{
  if (m_Path) {
    delete m_Path;
  }
  m_Path=NULL;
}

//---------------------------------------------------------------
void ClearCmpFileNameList()
{
  for (int i=0; i < NumCmpFile; i++) {
    delete (rg_pCmpFile[i]);
  }
  NumCmpFile=0;
  NumShowFile=0;
  LastR= -1;
  LastC= -1;
  NumLeftOnlyFile  = 0;
  NumRightOnlyFile = 0;
  NumDiffFile = 0;
#ifdef DYNARRY
  if (NumCmpFileSize==0) {
    rg_pCmpFile = (CCmpFile **) malloc((4000 *sizeof(CCmpFile  *) ));
    if (rg_pCmpFile) NumCmpFileSize+=4000;
  }
#else
#endif
}

//---------------------------------------------------------------
void AdjustFileTable()
{
  if (gCmpSheet) {
    gCmpSheet->rows(NumShowFile);////NumCmpFile);
    gCmpSheet->redraw();
    char s[200];
    sprintf(s,"LIST %3d/%3d, Lo %2d, Ro %2d, Dif %2d",NumShowFile,NumCmpFile,NumLeftOnlyFile,NumRightOnlyFile,NumDiffFile);
    SetStatusText(s,1,0 );
  }
}
//---------------------------------------------------------------
