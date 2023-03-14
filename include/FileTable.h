// FileTable.h: Schnittstelle für die Klasse CFileTable.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FileTable_H__8BA166F8_5389_4463_B727_433ABCA3D382__INCLUDED_)
#define AFX_FileTable_H__8BA166F8_5389_4463_B727_433ABCA3D382__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Table_Row.H>
#include <FL/names.h>
#include <stdio.h>
#include <stdlib.h>
#define NUMFILES 1200

#define   MLocalSrc  0x0000
#define   MServerSrc 0x0001
#define   MBackupSrc 0x0002
#define   MExternSrc 0x0003
#define   MEqual     0x0010
#define   MDBdiff    0x0020
#define   MJobPned   0x0040
#define   MSelect    0x0080
#define   MShowMask  0x00F0
#define   MIgnore    0x8000

//#define ITEM_BOLD    0x0040
//#define ITEM_SEL     0x0200
//#define ITEM_FAIL    0x0400
//#define ITEM_OK      0x0800
//#define ITEM_VISIBLE 0x1000


#define NOFILE    FL_DARK3
#define EQUAL     FL_GREEN
#define TIMEDIFF  FL_YELLOW
#define SIZEDIFF  FL_RED

class CCmpFile  {
public:
  unsigned char   m_Quelle[4];
  char * m_Path;
  char * m_Name;
  long   m_Size;
  long   m_Datum;
  int    m_RecID;
  int    m_FileType;
//  int    m_dwShow;
  int    SetFileName(int ID_Volume,char * lpfName);
  CCmpFile();
  ~CCmpFile();
};
extern int NumShowFile;
extern int NumCmpFile;
extern CCmpFile  * rg_pCmpFile[NUMFILES];
extern void AddCmpFileName(int ID_Volume,char * lpfName);
extern void ClearCmpFileNameList();
extern void AdjustFileTable();
//void SetAllID3Tag(const char * genre);
//void ModifyFolderStruct(const char * dfaultFolder);
extern const char * rgFileTypeText[7];

#undef EDITTABLE
extern char g_DocsPath[3][7][MAX_PATH];
extern const char * rgQuellText[4];
extern int gDocShowPrimaryPath;
extern int gSourceAndCompareMuster;
extern char gScanPath[_MAX_PATH];
extern int  gInklSubfolder;

typedef struct {
  char ext  [10];
  int  fType;
} extStruct;

extern const extStruct Exclude[37];

class CFileTable : public Fl_Table /*_Row*/
{
public:
  int s_left, s_top, s_right, s_bottom;	   // kb nav + mouse selection
  Fl_Input *input;					   // single instance of Fl_Int_Input widget
  int row_edit, col_edit;				   // row/col being modified
  virtual ~CFileTable();
  int _sort_reverse;
  int m_Num_Header;
  int m_rgHeaderPos[6];
  int m_Header_mod;
  int m_SortMode;
  int m_SelRowMode;
  void ScanFolder(const char * pSerachpath ,int bSubdirs);
  BOOL GetDirectoryContents(char *pszDirectory,int pathlen,DWORD ID_Volume,int bSubdirs);
public:
  CFileTable(int x, int y, int w, int h, const char *l=0);
  int  handle(int e);
//  bool virtual      FileOpen(char const  * lpFile);
  int mainInit();

  void sort_column(int col, int reverse);
  void draw_sort_arrow(int X,int Y,int W,int H);
  static void event_callback(Fl_Widget*, void*);
  void event_callback2();                                     // callback for table events
#ifdef EDITTABLE
  int GetSelectionSum();
  void UpdateSum();
  // Apply value from input widget to values[row][col] array and hide (done editing)
  void set_value_hide();

  static void input_cb(Fl_Widget*, void* v) {		// input widget's callback
    ((CFileTable*)v)->set_value_hide();
  }
  void start_editing(int R, int C);
  void done_editing();
#endif
protected:
    // Handle drawing all cells in table
    void draw_cell(TableContext context, int R=0,int C=0, int X=0,int Y=0,int W=0,int H=0);
};

extern CFileTable       * gCmpSheet;

#endif // !defined(AFX_FileTable_H__8BA166F8_5389_4463_B727_433ABCA3D382__INCLUDED_)
