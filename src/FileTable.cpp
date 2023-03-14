#include "FileMerge.h"
#include "FileTable.h"
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

const extStruct Exclude[37] = {
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
  if (gDocShowPrimaryPath>=0 && gDocShowPrimaryPath  < 7) {
    ClearCmpFileNameList();
    int src = gSourceAndCompareMuster & 3;
    if (src == MLocalSrc) {
      int n = strlen(g_DocsPath[0][gDocShowPrimaryPath]);
      strcpy(str,pSerachpath);
      GetDirectoryContents(str,n,0,bSubdirs);
    } else {
//      FileArchivDB->DBLoadRecords(src,gDocShowPrimaryPath);
      int n = strlen(g_DocsPath[0][gDocShowPrimaryPath]);
      strcpy(str,g_DocsPath[src][gDocShowPrimaryPath]);
      strcat(str,&pSerachpath[n]);
      GetDirectoryContents(str,n,src,bSubdirs);
    }
    AdjustFileTable();
    sort_column(m_SortMode, _sort_reverse);
  }
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
          }
          NumFiles++;
        }
        break;
        case S_IFDIR:
          /* unless ".", "..", ".hidden" or vidix driver dirs */
          if ((  strcmp(pEntry->d_name,".")!=0)
              && (   strcmp(pEntry->d_name,"..")!=0)) {
            if (bSubdirs) {
              if (GetDirectoryContents(str,pathlen,ID_Volume,bSubdirs)) {
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
  Fl_Color fg = FL_FOREGROUND_COLOR; //drawfgcolor();
  Fl_Color bg = FL_BACKGROUND2_COLOR;// drawbgcolor();
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

    if (R == row_edit && C == col_edit && input->visible()) {
      return;					// dont draw for cell with input widget over it
    }

    if (R < NumShowFile && rg_pCmpFile[R]->m_Quelle) {
      //int selected = /*m_SelRowMode ? row_selected(R) :*/ is_selected(R,C);
      //int selected = m_SelRowMode ?0:is_selected(R,C);
      int selected =  rg_pCmpFile[R]->m_Quelle[3] & MSelect;
      if (C==0 &&  selected ) {
        bg = FL_SELECTION_COLOR;
        fg = FL_BACKGROUND2_COLOR;
        fl_draw_box(FL_THIN_UP_BOX,X,Y,W,H,bg);
      } else {
        fl_draw_box(FL_THIN_DOWN_BOX,X,Y,W,H,bg);
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
      fl_color(rg_pCmpFile[R]->m_FileType==gDocShowPrimaryPath ?FL_CYAN:fg);
      sprintf(s, "%d", R*C);
      switch (C) {
      case 0: {
        DWORD q = FileArchivDB->CompareFile(0,rg_pCmpFile[R]->m_Path,rg_pCmpFile[R]->m_Name);
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
static int Ascend=1;
int Compare_State( const void *arg1, const void *arg2 )
{
  CCmpFile * pCnA = *(CCmpFile ** )arg1;
  CCmpFile * pCnB = *(CCmpFile ** )arg2;
  int res = pCnB->m_Quelle[3] - pCnA->m_Quelle[3];
  if (res == 0) {
    res = pCnB->m_Datum - pCnA->m_Datum;
    if (res==0) {
      res = pCnB->m_Size - pCnA->m_Size;
      if (res==0) {
        res = pCnB->m_RecID- pCnA->m_RecID;
      }
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
      if (res==0) {
        res = pCnB->m_RecID- pCnA->m_RecID;
      }
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
      if (res == 0) {
        res = pCnB->m_RecID- pCnA->m_RecID;
      }
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
  if (res==0) {
    res = pCnB->m_RecID- pCnA->m_RecID;
  }
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
  Ascend = reverse?1:-1;
  //"Quelle","Name","Size","Datum","Type","Path"
  switch (sortmode) {
  case 0:
    qsort(&rg_pCmpFile,NumCmpFile,sizeof(CCmpFile*),Compare_State);
    break;
  case 1:
    qsort(rg_pCmpFile,NumCmpFile,sizeof(CCmpFile*),Compare_Name);
    break;
  case 2:
    qsort(&rg_pCmpFile,NumCmpFile,sizeof(CCmpFile*),Compare_Size);
    break;
  case 3:
    qsort(&rg_pCmpFile,NumCmpFile,sizeof(CCmpFile*),Compare_Date);
    break;
  case 4:
    qsort(&rg_pCmpFile,NumCmpFile,sizeof(CCmpFile*),Compare_Vol);
    break;
  case 5:
    qsort(&rg_pCmpFile,NumCmpFile,sizeof(CCmpFile*),Compare_Path);
    break;
  }
  redraw();

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
void CFileTable::event_callback2()
{
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
        int multi = Fl::event_ctrl() != 0;
        if (Fl::event_shift())  multi = 2;
        int start = -1;  // noch nicht begonnen
        for (int i=0; i < NumCmpFile; i++) {
          if (start == -1 && (rg_pCmpFile[i]->m_Quelle[3]&MSelect) == MSelect) start = i;
          if (i==ROW) {
            if ((rg_pCmpFile[ROW]->m_Quelle[3]&MSelect) == MSelect)
              rg_pCmpFile[ROW]->m_Quelle[3]&= ~(MSelect);
            else
              rg_pCmpFile[ROW]->m_Quelle[3]|=  (MSelect);
            start = -2; // ende der multi selection
          } else if (multi==2) {
            if (start>=0) {
              rg_pCmpFile[i]->m_Quelle[3]|=  (MSelect);
            }
          } else if (multi==0) {
            rg_pCmpFile[i]->m_Quelle[3]&= ~(MSelect);
          }
          //redraw_range(i,i,0,6);
          redraw_range(i,i,0,0);
        }
        break;
      }
      break;
    }
  default:
    return;
  }

}
//-------------------------------------------------------------
//---------------------------------------------------------------
// Keyboard and mouse events
int CFileTable::handle(int e)
{
  if (1) {
    int ret = Fl_Table/*_Row*/::handle(e);
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
      m_Path = strdup(&lpfName[n]+1);
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
CCmpFile  * rg_pCmpFile[NUMFILES];

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

  if (NumCmpFile < NUMFILES) {
    CCmpFile * Object = (CCmpFile*) new CCmpFile();
    Object->SetFileName(ID_Volume,lpfName);
    DWORD q = FileArchivDB->CompareFile(ID_Volume,Object->m_Path,Object->m_Name);
    (*(DWORD*)&Object->m_Quelle) = q;

/*    Object ->m_Quelle[3]= ITEM_VISIBLE;
    if (Object->m_Quelle[0]== NOFILE && Object->m_Quelle[1]!= NOFILE ) {
      NumRightOnlyFile++;
      Object ->m_dwShow |= MServerSrc | ITEM_FAIL;
    } else if (Object->m_Quelle[0]!= NOFILE && Object->m_Quelle[1]== NOFILE ) {
      Object ->m_dwShow |= MLocalSrc | ITEM_FAIL;
      NumLeftOnlyFile++;
    }
    if (Object->m_Quelle[0]!= EQUAL  || Object->m_Quelle[1]!= EQUAL ) {
      Object ->m_dwShow |= MDiffer| ITEM_FAIL;
      NumDiffFile++;
    } else {
      Object ->m_dwShow |= MEqual | ITEM_OK;
    }
*/
    if ((gSourceAndCompareMuster&MIgnore) || (Object ->m_Quelle[3] & (gSourceAndCompareMuster & MShowMask))) {
      rg_pCmpFile[NumCmpFile] = Object;
      //rg_pCmpFile[NumCmpFile] ->SetFileName(lpfName);
      NumCmpFile++;
      NumShowFile = NumCmpFile;
    } else {
      delete (Object);
    }

    char s[200];
    sprintf(s,"%s LIST %3d, Lo %2d, Ro %2d, Dif %2d",(NumCmpFile>=NUMFILES)?"OVERLOAD":"",NumCmpFile,NumLeftOnlyFile,NumRightOnlyFile,NumDiffFile);
    SetStatusText(s,1,0 );
  }
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
}

//---------------------------------------------------------------
void AdjustFileTable()
{
  if (gCmpSheet) {
    gCmpSheet->rows(NumShowFile);////NumCmpFile);
    gCmpSheet->redraw();
  }
}
//---------------------------------------------------------------
