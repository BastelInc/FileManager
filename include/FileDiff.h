// FileDiff.h: Schnittstelle für die Klasse CFileDiff.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILEDIFF_H__5A8C1DBA_EB2E_4A02_8A5D_557CBCFD5CD6__INCLUDED_)
#define AFX_FILEDIFF_H__5A8C1DBA_EB2E_4A02_8A5D_557CBCFD5CD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//#include "Window.h"
#include "FileFolder.h"

class CCompareFile //: public CListViewItem  
{
public:
  HBITMAP           m_hBMP;
  unsigned int      m_SortKey;
//  char              m_FileName[_MAX_PATH];
  __off64_t         m_st_size;
  time_t            m_st_mtime;
  //unsigned long int m_st_mtime;
  int               m_ShowCnt;
  int               m_FileDiffState;
  char *            m_pText;
  char *            m_pszName;
  char *            m_pszPath1;
  char *            m_pszPath2;
  char *            m_pszEnding;
  
  CCompareFile(/*CListView * pWndControl, DWORD record*/);
//  virtual  BOOL     DrawItem(HDC hDC,LPRECT lpRect,BOOL bEarse);
  virtual           ~CCompareFile();
//-------------------------------------------------------------
//  virtual int       SetState(int iState);
//  virtual  BOOL     SelChanging(int nIndex,unsigned int wParam);
//  virtual  BOOL     SelChanged(int nIndex);
  unsigned int      MakeSortKey(int SortMode);
  virtual int       SetText(const char * lpStr);
  virtual int       SetFilePath1(const char * lpStr);
  virtual int       SetFilePath2(const char * lpStr);
  virtual int       GetFilePath1(char * lpStr,int len);
  virtual int       GetFilePath2(char * lpStr,int len);
  virtual int       UpdateFileStat(void);
};

// # define  MAX_NUM_HEADER 10
enum {xmedia_state_none,xmedia_state_do_play,xmedia_state_played};

class CFileDiff  : public Fl_Widget // : public CListView  
{
  int m_AudioOnly;
public:
  int m_Modify;
  
static  char    m_FileDiffPath1[_MAX_PATH];
static  char    m_FileDiffPath2[_MAX_PATH];
static  char    m_FileSubDir[_MAX_PATH];
static  int     m_FileDiffPath1len;
static  int     m_FileDiffPath2len;
  int           m_FileDiffIndex;
  int           m_StartScanFolderCnt;
  int           m_NumActiveListItem;
  static int    m_ActiveFilter;
  CTVItem *     m_pCurrentFolder;
  
  
  CFileDiff();
  virtual      ~CFileDiff();
//  virtual int   InitWindow(CWindow * pParent,int ID,int left,int top,int witdh, int heigth,int Style);
//  virtual void  AdjustItem();
  int           InitFileDiff(int ListMode,int AudioOnly);
  int           FileDiffScanJob(LPSTR lpPath,int job);
  int           AddFolder(LPSTR lpPath,int nmbr);
  int           DeleteItems(int bUnSelected);
  int           NumItems();
  virtual int   TrimmPathText(/*HDC hdc,*/char * pStrSrc,char * pStrDest,int left,int maxlen,int pixelwidth);
  CCompareFile *FindFile1(LPSTR lpPath);
  CCompareFile *InsertFile1(LPSTR lpPath);
  CCompareFile *FindFile2(LPSTR lpPath);
  CCompareFile *InsertFile2(LPSTR lpPath);
  virtual void  UpdateStatusBar();
  virtual int   KeyInput(int keycode);

  virtual int   SortValidItems();
  virtual int   SortItems();

  void          FileDiffRunFunc(void);
  pthread_t     m_hFileDiffThread;
  static void * thFileDiffThreadFunc(void * lpdwParam);

  int           CopyFile(int Job,LPSTR lpPath1,LPSTR lpPath2,CCompareFile * pCFile=NULL);


  void          StartFileDiff(int index);
  void          StopFileDiff();
  int           SaveFileDiff();
  int           LoadFileDiff(LPSTR lpStr);

  int           m_State;
};

/*
class CCompareFolder : public CFolder  //CTVItem  
{
public:
  CCompareFolder(PWNDCONTROL pWndControl,CTVItem * pParent, DWORD record);
  virtual ~CCompareFolder();
  virtual BOOL ExpandItem(int Hit);
  virtual int  SetState(int iState);
  virtual BOOL SelChanging(int nIndex,unsigned int wParam);
  virtual BOOL SelChanged(int nIndex);
//  virtual LPSTR GetParentName(LPSTR );
//  int  BuildDirPath(LPSTR lpszDir);
  virtual int  GetSubDirectory(LPSTR pszDirectory);
  //virtual int  HasDirectoryContents(LPSTR pszDirectory);
  virtual unsigned int MakeSortKey();
};

*/
#endif // !defined(AFX_FILEDIFF_H__5A8C1DBA_EB2E_4A02_8A5D_557CBCFD5CD6__INCLUDED_)
