//-------------------------------------------------------------
// FileDiff.cpp: Implementierung der Klasse CFileDiff.
//-------------------------------------------------------------

#include "FileDiff.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include "Resource.h"
#ifndef WIN32
#include <dirent.h>
#include <utime.h>
#include <malloc.h>
#else
#include <direct.h>
#include <sys/utime.h>
#include <malloc.h>
#endif
#include <time.h>
#include <BastelUtils.h>
#include "ProgressStatus.h"
#define SHOWDEBUG
#define BINARYFileDiff
enum {sortstate,sorttitle,sortpath1,sortpath2,sortsize,sortdate,sortending};

enum {file_diff_state_none,file_diff_state_play,file_diff_state_copy,file_diff_state_moveleft,file_diff_state_moveright,file_diff_state_touch,file_diff_state_delete};
 extern int SelectTreeItem(int ix,LPSTR lpstr);

#define RIGHT_ONLY      0x1
#define LEFT_ONLY       0x2
#define IDENTICAL       0x4
#define FIDENTICAL1     0x8
#define FIDENTICAL2    0x10
#define DIFFERENT1     0x20
#define DIFFERENT2     0x40
#define DIFFERENTSIZE  0x80
#define DIFFERENTDATE 0x100

/*
extern char gszRootProgramme[64];
extern char gszRootVideos[64];
extern char gszRootFilesystem[64];
extern char gszRootRecords[64];
extern char szPathMusicFolder[64];
*/
int  CFileDiff::m_ActiveFilter;
char CFileDiff::m_FileDiffPath1[_MAX_PATH];
char CFileDiff::m_FileDiffPath2[_MAX_PATH];
char CFileDiff::m_FileSubDir[_MAX_PATH];
int  CFileDiff::m_FileDiffPath1len;
int  CFileDiff::m_FileDiffPath2len;

char HeaderText[]=
{
  "Job\0"
  "Title\0"
  "Path1\0"
  "Path2\0"
  "Size\0"
  "Datum\0"
  "Ext\0\0"
};

int  date_widht;
char szFileDiffFile[80]={"FileDiffJob.txt"};
int Compare_MediaFile( const void *arg1, const void *arg2 );
//-------------------------------------------------------------
CCompareFile::CCompareFile() //CListView * pWndControl, DWORD record):CListViewItem(pWndControl,record)
{
  m_hBMP      = NULL;
  m_pText     = NULL;
  m_ShowCnt   = 0;
  m_pszEnding = NULL;
  m_FileDiffState=0;
  m_pszPath1  = NULL;
  m_pszPath2  = NULL;
  m_pszName   = NULL;
//  szFileDiffFile[0]='\0'; rolf
}
//-------------------------------------------------------------
CCompareFile::~CCompareFile()
{
//  if (m_hBMP) DeleteObject(m_hBMP);
  m_hBMP      = NULL;
  if (m_pszPath1) free (m_pszPath1);
  m_pszPath1  = NULL;
  if (m_pszPath2) free (m_pszPath2);
  m_pszPath2  = NULL;

}
//-------------------------------------------------------------
int CCompareFile::SetFilePath1(const char * lpStr)
{
  int len =0;
  const char * pFile = strrchr(lpStr, '/');
  if (pFile)
  {
    len = pFile - lpStr - CFileDiff::m_FileDiffPath1len;
    if (len >=0)
    {
      if (m_pszPath1) free (m_pszPath1);
      m_pszPath1 = (char*)malloc(len+1);
      memcpy(m_pszPath1,lpStr+CFileDiff::m_FileDiffPath1len,len);
      m_pszPath1[len]='\0';
    }
    SetText(pFile+1);
  }
  return len;
}
//-------------------------------------------------------------
int CCompareFile::SetFilePath2(const char * lpStr)
{
  int len =0;
  const char * pFile = strrchr(lpStr, '/');
  if (pFile)
  {
    len = pFile - lpStr - CFileDiff::m_FileDiffPath2len;
    if (len >=0)
    {
      if (m_pszPath2) free (m_pszPath2);
      m_pszPath2 = (char*)malloc(len+1);
      memcpy(m_pszPath2,lpStr+CFileDiff::m_FileDiffPath2len,len);
      m_pszPath2[len]='\0';
    }
    SetText(pFile+1);
  }
  return len;
}
//-------------------------------------------------------------
int CCompareFile::SetText(const char * lpStr)
{
  int TextSize;
#ifdef WIN32
  if (m_pText) TextSize = _msize(m_pText);
#else
  if (m_pText) TextSize = malloc_usable_size(m_pText);
#endif
  else         TextSize = 0;

  int slen = strlen(lpStr)+1;
  if (slen > TextSize)
  {
    TextSize = ((slen+31) /32)*32;

    if (m_pText) m_pText = (LPSTR)realloc(m_pText,TextSize);
    else         m_pText = (LPSTR)malloc(TextSize);
  }
  if (m_pText)
  {
    strcpy(m_pText,lpStr);
    m_pszEnding= strrchr(m_pText, '.');
  }
  if (m_pszEnding)
  {
    *m_pszEnding = '\0';
    m_pszEnding++;
  }
  else
  {
    m_pszEnding =  strrchr(m_pText, '\0');
  }
  m_pszName = m_pText;
  return slen;
}

//-------------------------------------------------------------
int CCompareFile::UpdateFileStat(void)
{
  m_FileDiffState = LEFT_ONLY;
  m_FileDiffState = FIDENTICAL1;
  m_FileDiffState = DIFFERENT2;

  char path1[_MAX_PATH];
  char path2[_MAX_PATH];
  struct stat statbuffer1;
  struct stat statbuffer2;
  GetFilePath1(path1,sizeof(path1));
  if (stat(path1, &statbuffer1)==0)
  {
    GetFilePath2(path2,sizeof(path2));
    m_st_size       = statbuffer1.st_size;
    m_st_mtime      = statbuffer1.st_mtime;
    m_FileDiffState = IDENTICAL;
    if (stat(path2, &statbuffer2)==0)
    {
      if (m_st_size   != statbuffer2.st_size )m_FileDiffState |= DIFFERENTSIZE;
      if (m_st_mtime  != statbuffer2.st_mtime)m_FileDiffState |= DIFFERENTDATE;
    }
    else  m_FileDiffState = LEFT_ONLY;
  }
  else  m_FileDiffState = RIGHT_ONLY;
  return 0;
}
//-------------------------------------------------------------
int CCompareFile::GetFilePath1(char * lpStr,int len)
{
  if (m_pszPath1==NULL || m_pszName==NULL) return 0;
  int n = CFileDiff::m_FileDiffPath1len + strlen(m_pszPath1)+strlen(m_pszName)+strlen(m_pszEnding)+3;
  if (n < len)
  {
    strcpy(lpStr,CFileDiff::m_FileDiffPath1);
    strcat(lpStr,m_pszPath1);
    strcat(lpStr,"/");
    strcat(lpStr,m_pszName);
    if (*m_pszEnding)
    {
      strcat(lpStr,".");
      strcat(lpStr,m_pszEnding);
    }
    return n;
  }
  return 0;
}
//-------------------------------------------------------------
int CCompareFile::GetFilePath2(char * lpStr,int len)
{
  if (m_pszPath2==NULL || m_pszName==NULL) return 0;
  int n = CFileDiff::m_FileDiffPath2len + strlen(m_pszPath2)+strlen(m_pszName)+strlen(m_pszEnding)+3;
  if (n < len)
  {
    strcpy(lpStr,CFileDiff::m_FileDiffPath2);
    strcat(lpStr,m_pszPath2);
    strcat(lpStr,"/");
    strcat(lpStr,m_pszName);
    if (*m_pszEnding)
    {
      strcat(lpStr,".");
      strcat(lpStr,m_pszEnding);
    }
    return n;
  }
  return 0;
}
//-------------------------------------------------------------
int CFileDiff::TrimmPathText(/*HDC hdc,*/char * pStrSrc,char * pStrDest,int left,int maxlen,int pixelwidth)
{
  char * pnt2 = pStrSrc;
  int len = strlen(pnt2);
  int wx  = pixelwidth;
  //int w   = m_Fonts->GetTextExtent(hdc,(LPCSTR )pnt2,len);
  int w   =  fl_width((const char *)pnt2,len);
  strncpy(pStrDest,pnt2,maxlen);
  if (w > wx)
  {
    const char * pPaht1 = strrchr(pStrDest, '/');
    if (pPaht1)
    {
      int len2 = strlen(pPaht1 );
      //int w2   = m_Fonts->GetTextExtent(hdc,(LPCSTR )pPaht1,len2);
      int w2   =  fl_width((const char *)pPaht1,len2);
      if (w2 < wx)
      {
        wx -= w2;
        len -= len2;
        //w   = m_Fonts->GetTextExtent(hdc,(LPCSTR )pnt2,len);
        w   =  fl_width((const char *)pnt2,len);
      }
      else
      {
        pPaht1=NULL;
        left = 1;
      }
    }

    strcpy(pStrDest,"..");
    //wx -= m_Fonts->GetTextExtent(hdc,(LPCSTR )pStrDest,3);
    wx -=  fl_width((const char *)pStrDest,3);
    if (left)
    {
      while (w > wx && len)
      {
        len--;
        pnt2++;
        //w = m_Fonts->GetTextExtent(hdc,(LPCSTR )pnt2,len);
        w   =  fl_width((const char *)pnt2,len);
      }
      strcat(pStrDest,pnt2);
    }
    else
    {
      while (w > wx && len)
      {
        len--;
        //w = m_Fonts->GetTextExtent(hdc,(LPCSTR )pnt2,len);
        w   =  fl_width((const char *)pnt2,len);
      }
      strncpy(pStrDest,pnt2,len);
      strcpy(pStrDest+len,"..");
    }
    if (pPaht1)
    {
      strcat(pStrDest,pPaht1);
    }
  }
  else strncpy(pStrDest,pStrSrc,maxlen);
  return w;
}
//-------------------------------------------------------------
/*
BOOL CCompareFile::DrawItem(HDC hDC,LPRECT lpRect,BOOL bEarse)
{
  RECT rcItem = *lpRect;

  BOOL bFocus = m_pWndControl->GetFocus()==m_pWndControl;
#define Abstand  4;
  if (m_pWndControl->m_ViewMode == LVS_REPORT)
  {
    //m_pWndControl->SetFont(hDC,"ComicSansMid",0);
    if (CWindow::s_LowRes)  m_pWndControl->SetFont(hDC,(char*)"Fixedsys",10);
    else                    m_pWndControl->SetFont(hDC,"ComicSans",20);//"sun12x22",10);
    COLORREF bkColor = 0;
    COLORREF TextColor = 0xFFFF;
    if ((CFileDiff::m_ActiveFilter & m_FileDiffState)==0)
    {
      m_pWndControl->SetBkMode(OPAQUE);
      //m_pWndControl->SetColor(COLOR_GRAYTEXT);
      m_pWndControl->SetBkColor(COLOR_BTNFACE);
      bkColor = m_pWndControl->GetPaletteColor(COLOR_BTNFACE);
    }
    else
    {
      if (m_Selected)
      {
        //m_pWndControl->SetColor(COLOR_WINDOWTEXT);
        m_pWndControl->SetBkColor(COLOR_HIGHLIGHT);
        m_pWndControl->SetBkMode(TRANSPARENT);
        m_pWndControl->FillRect(&rcItem,bFocus ? COLOR_HIGHLIGHT:COLOR_WINDOW);
        bkColor = m_pWndControl->GetPaletteColor(COLOR_HIGHLIGHT);
      }
      else
      {
        m_pWndControl->SetBkMode(OPAQUE);
        m_pWndControl->SetBkColor(COLOR_WINDOW);
        //m_pWndControl->SetColor(COLOR_WINDOWTEXT);
        bkColor = m_pWndControl->GetPaletteColor(COLOR_WINDOW);

      }
    }

    int w = 0;
    RECT rc = rcItem;
    rc.right= 0;
    CFileDiff * pFileDiff = (CFileDiff *)m_pWndControl;

    char szdur[40];

    float divisor = 1;
    if ((unsigned)m_st_size  > 1000000000)
    {
      divisor = 1000000000;
      sprintf(szdur,"%5.1 G",(float)((unsigned)m_st_size ) / divisor);
    }
    else if (m_st_size  > 1000000)
    {
      divisor = 1000000;
      if (m_st_size  > 100000000)
        sprintf(szdur,"%5d M",(int)(m_st_size  / divisor));
      else
        sprintf(szdur,"%5.1f M",(float)m_st_size  / divisor);
    }
    else if (m_st_size  > 1000)
    {
      divisor = 1000;
      if (m_st_size  > 100000)
        sprintf(szdur,"%5d K",(int)(m_st_size  / divisor));
      else
        sprintf(szdur,"%5.1f K",(float)m_st_size  / divisor);
    }
    else
    {
      divisor = 1;
      sprintf(szdur,"%5d B",(int)m_st_size  );
    }

    char szdatum[40];

    struct tm * local = localtime ((time_t *)&m_st_mtime );
    if (local )
    {
      if ((pFileDiff->m_rgHeaderPos[sortdate]) < date_widht+10)
        sprintf(szdatum," %2d.%02d.%02d",local->tm_mday,local->tm_mon+1,local->tm_year%100);
      else
        sprintf(szdatum,"%2d:%02d %2d.%02d.%02d",local->tm_hour,local->tm_min,local->tm_mday,local->tm_mon+1,local->tm_year%100);
    }
    else sprintf(szdatum,"2d:02d 2d.02d.02d");
    LPSTR pnt;
    for (int i=0;i< pFileDiff->m_Num_Header ;i++)
    {
      w    = pFileDiff->m_rgHeaderPos[i];
      TextColor = m_pWndControl->GetPaletteColor(COLOR_WINDOWTEXT);
      if (i==sorttitle)
      {
        if (m_FileDiffState!=IDENTICAL)  TextColor = RGB(255,0,0);
        pnt = GetText();
      }
      else if (i==sortpath1)
      {
        if (m_FileDiffState&LEFT_ONLY) TextColor = RGB(255,0,0);
        pnt = m_pszPath1;
      }
      else if (i==sortpath2)
      {
        pnt = m_pszPath2;
        if (m_FileDiffState&RIGHT_ONLY)  TextColor = RGB(255,0,0);
      }
      else if (i==sortsize)
      {
        if (m_FileDiffState&DIFFERENTSIZE)  TextColor = RGB(255,0,0);
        pnt = szdur;
      }
      else if (i==sortdate)
      {
        if (m_FileDiffState&DIFFERENTDATE)  TextColor = RGB(255,0,0);
        pnt = szdatum;
      }
      else if (i==sortending)
      {
        pnt = m_pszEnding;
      }
      else if (m_State==file_diff_state_copy)      pnt = (char *)"Cpy";
      else if (m_State==file_diff_state_moveleft)  pnt = (char *)"MvL";
      else if (m_State==file_diff_state_moveright) pnt = (char *)"MvR";
      else if (m_State==file_diff_state_delete)    pnt = (char *)"Rem";
      else                                         pnt = (char *)"-";

      rc.left  = rc.right;
      rc.right += pFileDiff->m_rgHeaderPos[i];
      char str[_MAX_PATH]={0};
      if (pnt)
      {
        if (i==sortpath1||i==sortpath2) pFileDiff->TrimmPathText(hDC,pnt,str,0,sizeof(str),w -6);
        else                            pFileDiff->TrimmText(hDC,pnt,str,0,sizeof(str),w -6);
      }

      if ((CFileDiff::m_ActiveFilter & m_FileDiffState)==0)TextColor=m_pWndControl->SetColor(COLOR_GRAYTEXT);
#ifdef KATHREIN
      TextColor |= 0xF0000000;
#endif
      if (TextColor< 32) TextColor |= 0x10101;
      m_pWndControl->SetColor(TextColor);
      bkColor = MulDivColor(bkColor,100,((i&1)==0)?110:90);
#ifdef KATHREIN
      bkColor |= 0xF0000000;
#endif
      m_pWndControl->SetBkColor(bkColor);
      if (bkColor< 32) bkColor |= 0x10101;
      m_pWndControl->FillRect(&rc,bkColor);

      m_pWndControl->DrawText(str,strlen(str),&rc);
    }
    if (rc.right < rcItem.right)
    {
      rc.left  = rc.right;
      rc.right = rcItem.right;
      m_pWndControl->FillRect(&rc,COLOR_WINDOW);
    }

    CCompareFile * pMediaFile  = (CCompareFile*)pFileDiff->m_rgListItem[pFileDiff->m_AktIndex];
    if (/ *m_Selected && * /pMediaFile  == this)
    {
      RECT rc = rcItem;
      //CWindow::InflateRect(&rc,-3,-1);
      m_pWndControl->DrawRectangleLogic(&rc,R2_WHITE,PS_DOT);
      //m_pWndControl->DrawRectangleLogic(&rc,R2_XORPEN);
    }
  }
  return 0;
}
*/
//-------------------------------------------------------------
#if 0
int CCompareFile::SetState(int iState)
{
  if (iState == file_diff_state_play)
  {
    char szpath[_MAX_PATH];
    GetFilePath1(szpath,sizeof(szpath));
    gDesk.ProcessModalFile (szpath,imMusic,0,0);
  }
  else
  {
    ((CFileDiff*)m_pWndControl)->m_Modify++;
    ((CFileDiff*)m_pWndControl)->UpdateStatusBar();
  }
  m_State = iState;
  return 0;
}
//-------------------------------------------------------------
BOOL CCompareFile::SelChanging(int nIndex,unsigned int wParam)
{
  int ret=1;
#ifdef WIN32
  if      (GetKeyState(VK_SHIFT)<0)   m_Selected = 1;
  else if (GetKeyState(VK_CONTROL)<0) m_Selected = !m_Selected;
  else     m_Selected = 0;
#else
  if (CWindow::m_SysKey)
  {
    int m = CWindow::m_SysKey->GetCtrlKeys();
    if (m & SHIFT_PRESSED)          m_Selected = 1;
    else if (m & LEFT_CTRL_PRESSED) m_Selected = !m_Selected;
    else   m_Selected = 0;

  }
#endif


  /*
#ifdef WIN32
  if      (GetKeyState(VK_SHIFT)<0)   SetState(file_diff_state_none);
  else if (GetKeyState(VK_CONTROL)<0) SetState(file_diff_state_copy);
#else
  if (CWindow::m_SysKey)
  {
    int m = CWindow::m_SysKey->GetCtrlKeys();
    if (m & SHIFT_PRESSED)          SetState(file_diff_state_none);
    else if (m & LEFT_CTRL_PRESSED) SetState(file_diff_state_copy);
  }
#endif
  */
  return ret;
}
//-------------------------------------------------------------
BOOL CCompareFile::SelChanged(int nIndex)
{
  UpdateFileStat();
  m_Selected = 1;
#ifdef WIN32
  if      (GetKeyState(VK_SHIFT)<0)   m_Selected = 1;
  else if (GetKeyState(VK_CONTROL)<0) m_Selected = 1;
  ((CFileDiff*)m_pWndControl)->UpdateStatusBar();

#else
  if (CWindow::m_SysKey)
  {
    int m = CWindow::m_SysKey->GetCtrlKeys();
    if (m & SHIFT_PRESSED)          m_Selected = 1;
    else if (m & LEFT_CTRL_PRESSED) m_Selected = 1;
    ((CFileDiff*)m_pWndControl)->UpdateStatusBar();
  }
#endif
  /*
#ifdef WIN32
  if      (GetKeyState(VK_SHIFT)<0)   SetState(file_diff_state_none);
  else if (GetKeyState(VK_CONTROL)<0) SetState(file_diff_state_copy);
  ((CFileDiff*)m_pWndControl)->UpdateStatusBar();

#else
  if (CWindow::m_SysKey)
  {
    int m = CWindow::m_SysKey->GetCtrlKeys();
    if (m & SHIFT_PRESSED)          SetState(file_diff_state_none);
    else if (m & LEFT_CTRL_PRESSED) SetState(file_diff_state_copy);
    ((CFileDiff*)m_pWndControl)->UpdateStatusBar();
  }
#endif
  */
  return 1;
}
#endif
//-------------------------------------------------------------
//-------------------------------------------------------------
CFileDiff::CFileDiff():Fl_Widget(0,0,0,0,NULL) //CListView()
{
  InitializeCriticalSection(&m_List_criticalsection);
  m_hFileDiffThread=(pthread_t)NULL;
  m_ViewMode           = LVS_REPORT;  //  LVS_ICON  LVS_LIST;
  memset(&m_FileDiffPath1,0,sizeof(m_FileDiffPath1));
  memset(&m_FileDiffPath2,0,sizeof(m_FileDiffPath2));
  memset(&m_FileSubDir,0,sizeof(m_FileSubDir));

  m_FileDiffPath1len   = 0;
  m_FileDiffPath2len   = 0;
  m_rgListItem         = NULL;
  m_NumListItem        = -1;
  m_NumActiveListItem  = -1;
  m_MaxNumListItem     = 0;
  m_StartScanFolderCnt = 0;
  m_SelectedButton     = 1;
  m_SortMode           = 1;
  m_Num_Header         = 0;
  m_State              = 0;
  m_Modify             = 0;
}

//-------------------------------------------------------------
CFileDiff::~CFileDiff()
{
  char str[100];
  for (int i=0;i< m_Num_Header && m_Header_mod;i++)
  {
    sprintf(str,"HeaderWidht_%s",m_rgHeaderText[i]);
    gBastelUtils.WriteProfileInt("FileDiff",str,m_rgHeaderPos[i]);
    //printf("gBastelUtils.WriteProfileInt(%s ,%i);\n",str,m_rgHeaderPos[i]);
  }
  gBastelUtils.WriteProfileInt("FileDiff","SortMode",m_SortMode);
  if (m_hFileDiffThread)
  {
#ifdef WIN32
    TerminateThread((HANDLE)m_hFileDiffThread,0);
#else
    pthread_cancel(m_hFileDiffThread);
    pthread_join(m_hFileDiffThread,NULL);
#endif
    m_hFileDiffThread=0;
  }
  Reset();
}

//-------------------------------------------------------------
int CFileDiff::InitWindow(CWindow * pParent,int ID,int left,int top,int witdh, int heigth,int Style)
{

  Style |= WS_VSCROLL;//|WS_HSCROLL;

  if (m_ViewMode==LVS_REPORT)
  {
    Style &= ~WS_HSCROLL;
    Style |=  WS_VSCROLL| WS_TOPTABS;

  }
  else if (m_ViewMode==LVS_LIST)
  {
#ifdef WIN32
    Style &= ~(WS_VSCROLL|WS_CAPTION );
#else
    Style &= ~(WS_VSCROLL|WS_TOPTABS);
#endif
    //Style &= ~(WS_VSCROLL|WS_MENUBAR);
    Style |=  WS_HSCROLL;
  }
  else if (m_ViewMode==LVS_SMALLICON || m_ViewMode==LVS_ICON )
  {
    Style &= ~WS_HSCROLL;
    Style |=  WS_VSCROLL|WS_TOPTABS;
  }

  CWindow::InitWindow(pParent,ID,left,top,witdh,heigth,Style);

  m_AudioOnly = 0;
  GetDC();
  if (CWindow::s_LowRes)  SetFont(m_hDC,(char*)"Fixedsys",10);
  else                    SetFont(m_hDC,"ComicSans",20);//"sun12x22",10);
  if (m_ViewMode==LVS_REPORT)
  {
    m_ySpace =   (m_Fonts->GetTextMetrics(m_hDC,TextMetricsHeight)/ 4) * 5;
    m_xSpace =   m_Fonts->GetTextMetrics(m_hDC,TextMetricsCharWidth);
  }
  else if (m_ViewMode==LVS_LIST)
  {
    m_xSpace     = 140;
    m_ySpace     = 80;
  }
  else if (m_ViewMode==LVS_SMALLICON || m_ViewMode==LVS_ICON )
  {
    m_xSpace     = 140;
    m_ySpace     = 80;
  }

  SetTabstop(10 * m_xSpace);
  SetTexAlign(0);


  AdjustItem();

  m_Num_Header = _min(7,MAX_NUM_HEADER);

  date_widht = m_Fonts->GetTextExtent(m_hDC,(LPCSTR)" 29.12.00 ",11);
  ReleaseDC();

  int     l;
  char  * pnt = HeaderText;
  char str[100];
  for (int i=0;i< m_Num_Header ;i++)
  {
    l= strlen(pnt);
    m_rgHeaderText[i] = pnt;
    if (l) pnt += (l+1);


    sprintf(str,"HeaderWidht_%s",m_rgHeaderText[i]);
    m_rgHeaderPos[i] = _min(_max(gBastelUtils.GetProfileInt("FileDiff",str,100),5),1000);

  }

  m_SelectedButton=m_SortMode = gBastelUtils.GetProfileInt("FileDiff","SortMode",1);

  InitializeScrollBars(0,0);

  if (!m_hFileDiffThread)
  {
#ifdef WIN32
    m_hFileDiffThread = (DWORD)CreateThread(NULL,0, (LPTHREAD_START_ROUTINE)thFileDiffThreadFunc,(void *)this,0,NULL);
#else
    pthread_attr_t attr;
    struct sched_param sched_param;
    pthread_attr_init(&attr);
    sched_param.sched_priority=SCHED_OTHER;
    pthread_attr_setschedparam(&attr,&sched_param);
    pthread_create(&m_hFileDiffThread, &attr, thFileDiffThreadFunc, this);
#endif
  }

  return 1;
}
//-------------------------------------------------------------
void CFileDiff::AdjustItem()
{
  if (m_ViewMode==LVS_REPORT)
  {
    //m_ySpace =   s_CY_SysBitmap;
    m_xSpace =   s_CX_SysBitmap;
    m_ScrAmount = 1;
  }
  else
  {
    m_ySpace =   108;
    m_xSpace =   140;
    if (m_ViewMode==LVS_LIST) m_ScrAmount = m_sClient.y / m_ySpace;
    else                      m_ScrAmount = m_sClient.x / m_xSpace;
    if (m_ScrAmount <1) m_ScrAmount = 1;
  }
}
//--------------------------------------------------------------
void CFileDiff::UpdateStatusBar()
{
  char str[256];
  sprintf(str,"%s | %d / %d   %s",m_State?"RUN ":"STOP",m_NumActiveListItem,m_NumListItem,szFileDiffFile);

  if (gDesk.m_pStatus2)
  {
    gDesk.m_pStatus2->SetText(str);
    gDesk.m_pStatus2->InvalidateRect(NULL,0);
  }
}
//-------------------------------------------------------------
int CFileDiff::CopyFile(int Job,LPSTR lpPath1,LPSTR lpPath2,CCompareFile * pCFile)
{
  int ret=0;
  struct stat statbuffer1={0};
  struct stat statbuffer2={0};

  if (stat(lpPath1, &statbuffer1)< 0) statbuffer1.st_size=0;
  if (stat(lpPath2, &statbuffer2)< 0) statbuffer2.st_size=0;

  if (Job == file_diff_state_touch)
  {
    if (statbuffer1.st_size && statbuffer1.st_size == statbuffer2.st_size)
    {
      struct utimbuf times ;
      times.actime = statbuffer1.st_atime;
      times.modtime= statbuffer1.st_mtime;
      utime(lpPath2, &times);
    }
  }
  else if (Job == file_diff_state_copy)
  {
    if (statbuffer1.st_size && statbuffer2.st_size==0)
    {
      char path3[_MAX_PATH];
      strcpy(path3,m_FileDiffPath2);
      strcat(path3,lpPath1+m_FileDiffPath1len);
      if (stat(path3, &statbuffer2)< 0) statbuffer2.st_size=0;
      if (statbuffer2.st_size==0)
      {
        char * pFile = strrchr(path3, '/');
        if (pFile )
        {
          *pFile = '\0';
          if (stat(path3, &statbuffer2)<0)
          {
#ifdef WIN32
            mkdir(path3);
#else
            mkdir(path3,0755);
#endif
          }
          *pFile = '/';
          int Buffsize = 1000000;
#ifdef WIN32
          int fdr = open(lpPath1, O_RDONLY |O_BINARY);
#else
          int fdr = open(lpPath1, O_RDONLY | O_LARGEFILE);
#endif
          if (fdr>=0)
          {
            SetBusyDisplay(statbuffer1.st_size / Buffsize,pFile );
            char * pBuff = (char * ) malloc(Buffsize +1);
            if (pBuff)
            {
#ifdef WIN32
              int fdw = open (path3, O_CREAT | O_WRONLY   |O_BINARY, 0644);
#else
              int fdw = open (path3, O_CREAT |O_TRUNC | O_WRONLY  | O_LARGEFILE |  O_NONBLOCK, 0644);
#endif
              if (fdw >= 0)
              {
                int wrx  =0;
                int iRead;
                ret   =1;
                do
                {
                  wrx  =0;
                  iRead = read(fdr,pBuff,Buffsize);
                  if (iRead )
                  {
                    wrx    = write(fdw,pBuff,iRead);
                    if (wrx != iRead)      ret  = 0;
                  }
                  StepBusyDisplay();
                }while (wrx && iRead && ret);
                close(fdw);
                if      (ret==0) perror(path3);
                else
                {
                  struct utimbuf times ;
                  times.actime = statbuffer1.st_atime;
                  times.modtime= statbuffer1.st_mtime;
                  utime(path3, &times);
                  if (pCFile)  pCFile->SetFilePath2(path3);
                }
              }
              else printf("fail to create %s\n",path3);
              free(pBuff);
            }
            close(fdr);
          }
        }
      }
    }
  }
  else if (Job == file_diff_state_moveleft)
  {
   ;
  }
  else if (Job == file_diff_state_moveright)
  {
  }
  else if (Job == file_diff_state_delete)
  {
  }

  if (pCFile)
  {
    pCFile->UpdateFileStat();
  }

  return ret;
}
//--------------------------------------------------------------
void CFileDiff::StartFileDiff(int index)
{
  if (m_State) m_State=0;
  else
  {
    InitFileDiff(0,0);
    LoadFileDiff(szFileDiffFile);
    memset(&m_FileSubDir,0,sizeof(m_FileSubDir));
    CFBWindow::SetFocusWindow(this);
    m_State = 1;
    //  m_StartScanFolderCnt=1;
  }
}
//--------------------------------------------------------------
int CFileDiff::KeyInput(int keycode)
{
  //int index  = m_AktIndex;
  keycode &= ~0x600;
  if (keycode == VK_F11)
  {
    if (m_AktIndex>= 0  && m_AktIndex < m_NumListItem && m_AktIndex < m_NumActiveListItem)
    {
      BeginnEdit(GetListItem(m_AktIndex),0);
    }
  }

  if ( keycode ==VK_DOWN || keycode ==VK_UP
    || keycode ==VK_NEXT || keycode ==VK_PRIOR
    || keycode ==VK_LEFT || keycode ==VK_RIGHT)
  {
    return CListView::KeyInput(keycode);
  }

  for (int index=0; index < m_NumActiveListItem ;index++)
  {
    CCompareFile * pMediaFile  = (CCompareFile*)m_rgListItem[index];
    if (pMediaFile  ->m_Selected )
    {
    if (keycode ==IDC_CopyFile)
    {
      m_rgListItem[index]->SetState(file_diff_state_copy);
      m_Modify ++;
/*
      CCompareFile * pMediaFile = (CCompareFile * ) m_rgListItem[m_AktIndex];
      char path1[_MAX_PATH];
      char path2[_MAX_PATH];
      pMediaFile ->GetFilePath1(path1,sizeof(path1));
      pMediaFile ->GetFilePath2(path2,sizeof(path2));
      CopyFile(file_diff_state_copy,path1,path2,pMediaFile );
*/
    }
    else if (keycode ==IDC_MoveFileR)
    {
      m_rgListItem[index]->SetState(file_diff_state_moveleft);
      m_Modify ++;
    }
    else if (keycode ==IDC_MoveFileL)
    {
      m_rgListItem[index]->SetState(file_diff_state_moveright);
      m_Modify ++;
    }
    else if (keycode ==IDC_DeleteFile)
    {
      m_rgListItem[index]->SetState(file_diff_state_delete);
      m_Modify ++;
    }
    else if (keycode == IDC_TouchFile)
    {
      CCompareFile * pMediaFile = (CCompareFile * ) m_rgListItem[index];
      char path1[_MAX_PATH];
      char path2[_MAX_PATH];
      pMediaFile ->GetFilePath1(path1,sizeof(path1));
      pMediaFile ->GetFilePath2(path2,sizeof(path2));
      CopyFile(file_diff_state_touch,path1,path2,pMediaFile);
    }
    else if (keycode ==VK_LEFT || keycode ==VK_RIGHT)
    {
#ifdef WIN32
      if (GetKeyState(VK_CONTROL)<0)
#else
        int m = CWindow::m_SysKey?CWindow::m_SysKey->GetCtrlKeys():0;
      //if (m & SHIFT_PRESSED)
      if (m & LEFT_CTRL_PRESSED)
#endif
      {
        m_rgListItem[index]->SetState(keycode ==VK_LEFT ? file_diff_state_moveleft:file_diff_state_moveright);
        m_Modify ++;
      }
    }
    else
    {
      return CListView::KeyInput(keycode);
    }
    InvalidateItem(m_rgListItem[index]);
    }
    UpdateStatusBar();
  }
  //else return CListView::KeyInput(keycode);
  return 0;
}
void CFileDiff::StopFileDiff()
{
  m_State = 0;
  /*
  if (gDesk.m_MP3Player && gDesk.m_MP3Player->GetState()>=0)
  {
    gDesk.m_MP3Player->Stop();//Terminate();
    m_stop_exit_Test_Pause = 40;
  }
  else
  {
    // stop button pressed while the player is already stopped.
    // deselect all play marks
    EnterCriticalSection(&m_List_criticalsection);
    for (int i = 0;i < m_NumListItem;i++)
    {
      m_rgListItem[i]->SetState(file_diff_state_none);
    }
    LeaveCriticalSection(&m_List_criticalsection);
    InvalidateRect(NULL,0);
  }
  */
  UpdateStatusBar();
}
//--------------------------------------------------------------
int CFileDiff::SaveFileDiff()
{
  if (m_Modify)
  {
    if (m_FileDiffPath1[0])
    {
      gBastelUtils.WriteProfileString("Desktop","Lastfile",m_FileDiffPath1);
    }
    if (m_FileDiffPath2[0])
    {
      gBastelUtils.WriteProfileString("Desktop","Lastfile2",m_FileDiffPath2);
    }

    if (m_NumListItem <= 0) return 0;
    if (szFileDiffFile[0]=='\0')  strcpy(szFileDiffFile,"FileDiffJob.txt");
    int Num=0;
    if (m_NumActiveListItem > m_NumListItem)  m_NumActiveListItem = m_NumListItem ;
    if (m_AktIndex>= 0  && m_AktIndex < m_NumActiveListItem )
    {
      EnterCriticalSection(&m_List_criticalsection);

      char path[_MAX_PATH+20];
      strcpy(path,CWindow::s_IniPath);
      strcat(path,"/");
      strcat(path,szFileDiffFile);

      FILE * ftemp = fopen(path, "wb");

      char buff[2*_MAX_PATH+20];
      for (int i=0; i < m_NumActiveListItem ;i++)
      {
        CCompareFile * pF = (CCompareFile*)m_rgListItem[i];
        int State = pF->GetState();
        if      (State==file_diff_state_copy)      strcpy(buff,"Cpy\t");
        else if (State==file_diff_state_moveleft)  strcpy(buff,"MvL\t");
        else if (State==file_diff_state_moveright) strcpy(buff,"MvR\t");
        else if (State==file_diff_state_delete)    strcpy(buff,"Rem\t");
        else                                       continue;

        int p = 3 + pF->GetFilePath1(&buff[4],256);
        buff[p] = '\t';
        p++;
        p += pF->GetFilePath2(&buff[p],256);
        buff[p-1] = '\n';
        if (fwrite(&buff,1,p,ftemp)==(size_t)p) Num++;
      }
      fclose(ftemp);
      LeaveCriticalSection(&m_List_criticalsection);

      sprintf(buff,"Save %d items",Num);
      CWindow::MessageBox(gDesk.m_pApp,buff,"FileDiff",MB_OK|MB_ICONASTERISK| (50<<12));

    }
    m_Modify=0;
  }
  LoadFileDiff(szFileDiffFile);
  return 1;
}

//--------------------------------------------------------------
int CFileDiff::LoadFileDiff(LPSTR lpStr)
{
  InitFileDiff(0,0);
  m_FileDiffIndex=0;
  if (m_FileDiffPath1[0]==0)
  {
    gBastelUtils.GetProfileString("Desktop","Lastfile","",m_FileDiffPath1,sizeof(m_FileDiffPath1));
    m_FileDiffPath1len = strlen(m_FileDiffPath1);
    SelectTreeItem(0,(char*)m_FileDiffPath1);
    CFBWindow::SetFocusWindow(gDesk.m_pTree[0]);
  }
  if (m_FileDiffPath2[0]==0)
  {
    gBastelUtils.GetProfileString("Desktop","Lastfile2","",m_FileDiffPath2,sizeof(m_FileDiffPath2));
    m_FileDiffPath2len = strlen(m_FileDiffPath2);
    SelectTreeItem(1,(char*)m_FileDiffPath2);
    CFBWindow::SetFocusWindow(gDesk.m_pTree[1]);
  }

  m_StartScanFolderCnt = 0;

  char path[_MAX_PATH+20];
  strcpy(path,CWindow::s_IniPath);
  strcat(path,"/");
  strcat(path,szFileDiffFile);

  FILE * ftemp = fopen(path, "rb");
  char buff[2*_MAX_PATH+20];
  if (ftemp)
  {
    while (fgets(buff,sizeof(buff),ftemp))
    {
      char str1 [_MAX_PATH];
      char path1[_MAX_PATH];
      char path2[_MAX_PATH];
      path1[0]='\0';
      path2[0]='\0';

      int x = sscanf(buff,"%s\t%s\t%s",&str1,&path1,&path2);

      if (x >= 2)
      {
        CCompareFile * pFile = NULL;
        if (path1[0]) pFile   = InsertFile1(path1);
        else          pFile   = InsertFile2(path2);
        if (pFile)
        {
          if (path2[0] && pFile->m_FileDiffState & LEFT_ONLY)  pFile->SetFilePath2(path2);
          if (path1[0] && pFile->m_FileDiffState & RIGHT_ONLY) pFile->SetFilePath1(path1);

          if      (strcmp(str1,"Cpy")==0) pFile->SetState(file_diff_state_copy);
          else if (strcmp(str1,"MvL")==0) pFile->SetState(file_diff_state_moveleft);
          else if (strcmp(str1,"MvR")==0) pFile->SetState(file_diff_state_moveright);
          else if (strcmp(str1,"Rem")==0) pFile->SetState(file_diff_state_delete);
          pFile->UpdateFileStat();
        }
      }
    }
    fclose(ftemp);
  }
  m_Modify=0;

  InitializeScrollBars(0,m_NumActiveListItem);//m_NumListItem);
  InvalidateRect(NULL,1);
  if (m_NumListItem) EnsureVisible(0);
  return 1;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
void CFileDiff::FileDiffRunFunc(void)
{
  static int once = 10;
  while(gAppRun)
  {
    if (once)
    {
      once--;
      if (once == 0)
      {
        if (m_NumListItem <= 0)    LoadFileDiff(szFileDiffFile);
      }
    }
    if (m_StartScanFolderCnt)
    {
      /*if (m_Modify)
      {
        int ret  = SaveFileDiff();
        if (ret==0) m_StartScanFolderCnt=0;
      }
      else
      */
      {
        StepBusyDisplay();
        m_StartScanFolderCnt--;
        if (m_StartScanFolderCnt==0)
        {
#ifdef SHOWDEBUG
//          printf("Scan %s\n",m_FileDiffPath);
#endif
          /*
          char *ending;
          ending = strrchr(m_FileDiffPath, '.');
          if (ending && (strncasecmp(ending, ".BPL", 5)==0))
          {
                DeleteItems(1);
            if (m_NumListItem<=0) LoadFileDiff(m_FileDiffPath);
          }
          else if (ending && (strncasecmp(ending, ".PLS", 5)==0))
          {
                DeleteItems(1);
            if (m_NumListItem<=0) LoadFileDiff(m_FileDiffPath);
          }
          else
          */
          {
            //CTVItem  *  pitem = NULL;
            //if (m_FileDiffIndex) pitem = (CTVItem  *)  gDesk.m_pTree[1]->GetTVitem(gDesk.m_pTree[1]->m_AktIndex);
            //else                 pitem = (CTVItem  *)  gDesk.m_pTree[0]->GetTVitem(gDesk.m_pTree[0]->m_AktIndex);

            //if (m_pCurrentFolder != pitem )
            {
              if (SelChanging(m_AktIndex,0))
              {
                //strcpy(szFileDiffFile,"");
                char Path [_MAX_PATH];
                if (m_FileDiffIndex)
                {
                  if (m_FileDiffPath1[0] && m_FileDiffPath2[0])
                  {
                    if (CFileDiff::m_FileSubDir[0]) DeleteItems(1);
                    else                            DeleteItems(0);
                    strcpy(Path,m_FileDiffPath1);
                    strcat(Path,CFileDiff::m_FileSubDir);
                    AddFolder(Path,0);
                    //AddFolder(m_FileDiffPath1,m_FileDiffChangeState,0);
                  }
                  if (m_FileDiffPath2[0])
                  {
                    strcpy(Path,m_FileDiffPath2);
                    strcat(Path,CFileDiff::m_FileSubDir);
                    AddFolder(Path,1);
                  }
                  //AddFolder(m_FileDiffPath2,m_FileDiffChangeState,1);
                }
                else
                {
                  if (m_FileDiffPath2[0] && m_FileDiffPath1[0])
                  {
                    if (CFileDiff::m_FileSubDir[0]) DeleteItems(1);
                    else                            DeleteItems(0);
                    strcpy(Path,m_FileDiffPath2);
                    strcat(Path,CFileDiff::m_FileSubDir);
                    AddFolder(Path,1);
                  }
                  if (m_FileDiffPath1[0])
                  {
                    strcpy(Path,m_FileDiffPath1);
                    strcat(Path,CFileDiff::m_FileSubDir);
                    AddFolder(Path,0);
                  }
                  //AddFolder(m_FileDiffPath1,m_FileDiffChangeState,0);
                }

//                m_pCurrentFolder = pitem;
                //int altindex = m_NumListItem;
                //if (AddFolder(m_FileDiffIndex?m_FileDiffPath2:m_FileDiffPath1:,m_FileDiffChangeState,m_FileDiffIndex))
                {
                  //if (m_FileDiffChangeState && m_SortMode == sortending)
                  SortValidItems();
                  SortItems();
                  //else
                  /*
                  {
                    EnterCriticalSection(&m_List_criticalsection);
                    //try
                    //{
                    if (m_NumListItem  > altindex && altindex>=0)
                    {
                      qsort(&m_rgListItem[altindex],m_NumListItem-altindex,sizeof(CListViewItem*),Compare_MediaFile);
                    }
                    //}
                    //catch(...)
                    //{
                    //}
                    LeaveCriticalSection(&m_List_criticalsection);
                  }
                  */
                }
                //InitializeScrollBars(0,m_NumActiveListItem);
                /*
                if (m_NumListItem)
                  EnsureVisible(m_NumActiveListItem-1);     // show the last item
                if (altindex< 0) altindex=0;
                {
                  EnsureVisible(altindex);            // scroll back to the first new item
                  m_AktIndex = altindex;
                  SelChanged(m_AktIndex);
                }
                */
                SelChanged(0);
                EnsureVisible(0);
              }
            }
          }
          UpdateStatusBar();
          InvalidateRect(NULL,1);
        }
      }
    }
    else
    {
      if (m_State )
      {
        char buff[2*_MAX_PATH+20];
        for (int i=0; i < m_NumActiveListItem ;i++)
        {
          CCompareFile * pMediaFile  = (CCompareFile*)m_rgListItem[i];
          int State = pMediaFile ->GetState();
          if (State >0 && SelChanging(m_AktIndex,0))
          {
            m_AktIndex = i;
            EnsureVisible(m_AktIndex);
            SelChanged(m_AktIndex);

            if      (State==file_diff_state_copy)      strcpy(buff,"Cpy\t");
            else if (State==file_diff_state_moveleft)  strcpy(buff,"MvL\t");
            else if (State==file_diff_state_moveright) strcpy(buff,"MvR\t");
            else if (State==file_diff_state_delete)    strcpy(buff,"Rem\t");
            else                                       continue;

            char path1[_MAX_PATH]={0};
            char path2[_MAX_PATH]={0};
            pMediaFile ->GetFilePath1(path1,sizeof(path1));
            pMediaFile ->GetFilePath2(path2,sizeof(path2));
            CopyFile(State,path1,path2,pMediaFile );
            m_Modify++;
            pMediaFile->SetState(0);
            break;
          }
        }
      }
      if (m_AudioOnly)
      {
        SortValidItems();
        SortItems();
      }
      m_AudioOnly=0;
    }
    /*{
      if (gDesk.m_MP3Player && gDesk.m_MP3Player->GetState()>=0)
      {
        if (m_stop_exit_Test_Pause >0)  m_stop_exit_Test_Pause--;
        m_play_List_Test_Pause  = 40;     // two secunds silence
      }
      else if (m_play_List_Test_Pause >0) m_play_List_Test_Pause  --;
    }
    if (m_State)
    {
      // play list in run state
      if (m_explicit_play_index>=0)
      {
        if (m_play_List_Test_Pause>10) m_play_List_Test_Pause = 5;
        if (m_play_List_Test_Pause  == 0)
        {
          if (SelChanging(m_AktIndex,0))
          {
            m_AktIndex = m_explicit_play_index;
            EnsureVisible(m_explicit_play_index);
            SelChanged(m_AktIndex);
            if (m_AktIndex < m_NumListItem)
            {
              StartFileDiff(m_AktIndex);
            }
            m_explicit_play_index=-1;
            m_stop_exit_Test_Pause=0;
          }
        }
      }
      else if (m_play_List_Test_Pause  == 0)
      {
        BOOL OneFound=false;
        if (m_rgListItem)
        {

          for (int i = 0;i < m_NumListItem;i++)
          {
            // scan beginning with top to find a unplayed media file
            if (m_rgListItem[i]->GetState()==file_diff_state_copy)
            {
              OneFound=true;
              if (SelChanging(m_AktIndex,0))
              {
                m_AktIndex = i;
                EnsureVisible(i);
                SelChanged(m_AktIndex);
                if (m_AktIndex < m_NumListItem)
                {
                  StartFileDiff(m_AktIndex);
                  m_play_List_Test_Pause  = 100;  // enough time to spinn-up HD
                  if (szFileDiffFile[0]) m_Modify ++;
                  else                   m_Modify =0;
                  break;
                }
              }
            }
          }
        }
        if (OneFound==false)
        {
          if (gDesk.m_MP3Player) gDesk.m_MP3Player->Terminate();
          m_State = 0;       // stop play list
        }
        UpdateStatusBar();
      }
    }*/
    usleep(50000);
  }
}
//-------------------------------------------------------------
void * CFileDiff::thFileDiffThreadFunc(void * lpdwParam)
{
  ((CFileDiff * )lpdwParam)->FileDiffRunFunc();
  ((CFileDiff * )lpdwParam)->m_hFileDiffThread=0;
  return NULL;
}
//-------------------------------------------------------------
int CFileDiff::FileDiffScanJob(LPSTR lpPath,int job)
{
  m_pCurrentFolder = NULL;
#ifndef WIN32
  m_FileDiffIndex = (gDesk.m_pTree[1]->GetFocus() == gDesk.m_pTree[1]->GetHWnd());
#else
  m_FileDiffIndex = (::GetFocus() == gDesk.m_pTree[1]->GetHWnd());
#endif
  if (job)
  {
    if (m_FileDiffIndex ==0)
    {
      strcpy(m_FileDiffPath1,lpPath);
      m_FileDiffPath1len = strlen(m_FileDiffPath1);
      SetBusyDisplay(100,m_FileDiffPath1);
    }
    else
    {
      strcpy(m_FileDiffPath2,lpPath);
      m_FileDiffPath2len = strlen(m_FileDiffPath2);
      SetBusyDisplay(100,m_FileDiffPath2);
    }
  }
  if (job==0)
  {
    m_StartScanFolderCnt=20;     // stay 20 times 50 mS to fill Listview
  }
  else if (job==1)
  {
    m_StartScanFolderCnt = 1;    // immediate start scan
  }
  return 1;
}
//-------------------------------------------------------------
int CFileDiff::DeleteItems(int bUnSelected)
{
  m_pCurrentFolder = NULL;
  if (bUnSelected)
  {
    if (m_rgListItem)
    {
      EnterCriticalSection(&m_List_criticalsection);
      for (int i = 0;i < m_NumListItem;i++)
      {
        if (m_rgListItem[i] && m_rgListItem[i]->GetState()== file_diff_state_none)
        {
          delete m_rgListItem[i];
          memmove(&m_rgListItem[i],&m_rgListItem[i+1],_max(0,m_NumListItem-i)*sizeof(CListViewItem * ));
          m_NumListItem--;
          i--;
        }
      }
      LeaveCriticalSection(&m_List_criticalsection);
    }
  }
  else Reset();
  InvalidateRect(NULL,1);
  return 1;
}


//-------------------------------------------------------------
int CFileDiff::NumItems()
{
  return m_NumListItem > 0;
}
//-------------------------------------------------------------
//--------------------------------------------------------------
unsigned int CCompareFile::MakeSortKey(int SortMode)
{
  unsigned int SortKey = 0;
  if (((CFileDiff*)m_pWndControl)->m_SortMode ==sortending)  SortKey = rand();
  return  SortKey;
}
//-------------------------------------------------------------
//--------------------------------------------------------------
int Compare_MediaFile( const void *arg1, const void *arg2 )
{
  CCompareFile * p1 = *(CCompareFile**)arg1;
  CCompareFile * p2 = *(CCompareFile**)arg2;
  int res = (CFileDiff::m_ActiveFilter  & p2->m_FileDiffState) - (CFileDiff::m_ActiveFilter  & p1->m_FileDiffState);
  return res;
}
//--------------------------------------------------------------
int Compare_MediaFileState( const void *arg1, const void *arg2 )
{
  CCompareFile * p1 = *(CCompareFile**)arg1;
  CCompareFile * p2 = *(CCompareFile**)arg2;

  return  p2->GetState()- p1->GetState();
}
//--------------------------------------------------------------
int Compare_MediaFileName( const void *arg1, const void *arg2 )
{
  CCompareFile * p1 = *(CCompareFile**)arg1;
  CCompareFile * p2 = *(CCompareFile**)arg2;

  int                                             res =  strcmp(p1->m_pszName, p2->m_pszName);
  if (res==0 && p1->m_pszPath1 && p2->m_pszPath1) res =  strcmp(p1->m_pszPath1,p2->m_pszPath1);
  if (res==0 && p1->m_pszPath2 && p2->m_pszPath2) res =  strcmp(p1->m_pszPath2,p2->m_pszPath2);

  return res;
}
//--------------------------------------------------------------
int Compare_MediaFilePath1( const void *arg1, const void *arg2 )
{
  CCompareFile * p1 = *(CCompareFile**)arg1;
  CCompareFile * p2 = *(CCompareFile**)arg2;
  int res = 0;
  if (p1->m_pszPath1 && p2->m_pszPath1)           res =  strcmp(p1->m_pszPath1,p2->m_pszPath1);
  else  if (p1->m_pszPath1)                       return-1;
  else  if (p2->m_pszPath1)                       return 1;

  if (res==0 && p1->m_pszPath2 && p2->m_pszPath2) res =  strcmp(p1->m_pszPath2,p2->m_pszPath2);
  if (res==0)                                     res =  strcmp(p1->m_pszName, p2->m_pszName);
  return res;

  return res;
}
//--------------------------------------------------------------
int Compare_MediaFilePath2( const void *arg1, const void *arg2 )
{
  CCompareFile * p1 = *(CCompareFile**)arg1;
  CCompareFile * p2 = *(CCompareFile**)arg2;
  int res =0;
  if(p1->m_pszPath2 && p2->m_pszPath2)            res =  strcmp(p1->m_pszPath2,p2->m_pszPath2);
  else  if (p1->m_pszPath2)                       return-1;
  else  if (p2->m_pszPath2)                       return 1;
  if (res==0)                                     res =  strcmp(p1->m_pszName, p2->m_pszName);
  if (res==0 && p1->m_pszPath1 && p2->m_pszPath1) res =  strcmp(p1->m_pszPath1,p2->m_pszPath1);
  return res;
}
//--------------------------------------------------------------
int Compare_MediaFileEnding( const void *arg1, const void *arg2 )
{
  CCompareFile * p1 = *(CCompareFile**)arg1;
  CCompareFile * p2 = *(CCompareFile**)arg2;
  int res =  strcmp(p1->m_pszEnding, p2->m_pszEnding);
  //return p1->m_ShowCnt - p2->m_ShowCnt;
  return res ; //p1->m_SortKey - p2->m_SortKey;

}
//--------------------------------------------------------------
int Compare_MediaFileDur( const void *arg1, const void *arg2 )
{
  CCompareFile * p1 = *(CCompareFile**)arg1;
  CCompareFile * p2 = *(CCompareFile**)arg2;
  return (int)(p1->m_st_size- p2->m_st_size);
}
//--------------------------------------------------------------
int Compare_MediaFileDate( const void *arg1, const void *arg2 )
{
  CCompareFile * p1 = *(CCompareFile**)arg1;
  CCompareFile * p2 = *(CCompareFile**)arg2;
  return  p1->m_st_mtime - p2->m_st_mtime;
}
//--------------------------------------------------------------
int CFileDiff::SortValidItems()
{
  EnterCriticalSection(&m_List_criticalsection);
  CCompareFile* pFileIndex = NULL;
  m_NumActiveListItem = 0;
  int usedix          = m_NumListItem-1;
  while (m_NumActiveListItem <= usedix)
  {
    while (usedix > m_NumActiveListItem
      && (((CCompareFile*)m_rgListItem[usedix])->m_FileDiffState & m_ActiveFilter )== 0) usedix--;

    pFileIndex =  (CCompareFile*)m_rgListItem[m_NumActiveListItem];
    if ((m_ActiveFilter & (pFileIndex->m_FileDiffState) & m_ActiveFilter )==0)
    {
      if (usedix > m_NumActiveListItem )
      {
        m_rgListItem[m_NumActiveListItem] = m_rgListItem[usedix];
        m_rgListItem[usedix]              = pFileIndex;
        usedix--;
      }
      else break;
    }

    m_NumActiveListItem++;
  }
  LeaveCriticalSection(&m_List_criticalsection);
  InitializeScrollBars(0,m_NumActiveListItem);
  UpdateStatusBar();
  return m_NumActiveListItem;
}

//-------------------------------------------------------------
int CFileDiff::SortItems()
{
  static int busy =0;
  if (m_NumListItem > 0)
  {
    if (busy) return 0;
    busy++;

    SetBusyDisplay();

    if (m_NumActiveListItem < 0) m_NumActiveListItem =0;
    else if (m_NumActiveListItem > m_NumListItem ) m_NumActiveListItem =m_NumListItem ;
    if (SelChanging(m_AktIndex,0))
    {
      CCompareFile* pFileIndex = NULL;
      /*
      for (int i=0;i < m_NumListItem;i++)
      {
      pFileIndex =  (CCompareFile*)m_rgListItem[i];
      if (pFileIndex)
      {
      pFileIndex ->m_SortKey =  pFileIndex ->MakeSortKey(m_SortMode);
      }
    }*/
      switch (m_SortMode)
      {
      case sortstate:
        qsort(&m_rgListItem[0],m_NumActiveListItem,sizeof(CListViewItem*),Compare_MediaFileState);
        break;
      case sorttitle:
        qsort(&m_rgListItem[0],m_NumActiveListItem,sizeof(CListViewItem*),Compare_MediaFileName);
        break;
      case sortpath1:
        qsort(&m_rgListItem[0],m_NumActiveListItem,sizeof(CListViewItem*),Compare_MediaFilePath1);
        break;
      case sortpath2:
        qsort(&m_rgListItem[0],m_NumActiveListItem,sizeof(CListViewItem*),Compare_MediaFilePath2);
        break;
      case sortending:
        qsort(&m_rgListItem[0],m_NumActiveListItem,sizeof(CListViewItem*),Compare_MediaFileEnding);
        break;
      case sortsize:
        qsort(&m_rgListItem[0],m_NumActiveListItem,sizeof(CListViewItem*),Compare_MediaFileDur);
        break;
      case sortdate:
        qsort(&m_rgListItem[0],m_NumActiveListItem,sizeof(CListViewItem*),Compare_MediaFileDate);
        break;
      };
      InvalidateRect(NULL,0);
    }
    ResetBusyDisplay();
    busy--;
  }
  return 1;
}

//-------------------------------------------------------------
/*
CCompareFile * CFileDiff::FindFile1(LPSTR lpPath)
{
  StepBusyDisplay();
  //CCompareFile * pFindFile = (CCompareFile *)new CCompareFile(this,0);
  CCompareFile  FindFile(this,0);
  FindFile.SetFilePath1(lpPath);
  FindFile.m_SortKey =  FindFile.MakeSortKey(m_SortMode);
  if (FindFile.m_pszName==NULL|| FindFile.m_pszPath1==NULL) return NULL;

  CCompareFile* pFileIndex = NULL;
  CCompareFile* pFile      = NULL;
  for (int i=0;i < m_NumListItem && m_rgListItem[i];i++)
  {
    pFile = (CCompareFile*)m_rgListItem[i];
    if (strcmp(FindFile.m_pszName,pFile->m_pszName)==0)
    {
      if (strcmp(FindFile.m_pszEnding,pFile->m_pszEnding)==0)
      {
      //if (pFile->m_pszPath1)
      {
        //if (strcmp(FindFile.m_pszPath1,pFile->m_pszPath1)==0)
        {
          pFileIndex = pFile;
          break;
        }
      }
      }
    }
  }

  return pFileIndex;
}
//-------------------------------------------------------------
CCompareFile * CFileDiff::FindFile2(LPSTR lpPath)
{
  StepBusyDisplay();
  //CCompareFile * pFindFile = (CCompareFile *)new CCompareFile(this,0);
  CCompareFile  FindFile(this,0);
  FindFile.SetFilePath2(lpPath);
  FindFile.m_SortKey =  FindFile.MakeSortKey(m_SortMode);
  if (FindFile.m_pszName ==NULL || FindFile.m_pszPath2==NULL) return NULL;

  CCompareFile* pFileIndex = NULL;
  CCompareFile* pFile      = NULL;
  for (int i=0;i < m_NumListItem && m_rgListItem[i];i++)
  {
    pFile = (CCompareFile*)m_rgListItem[i];
    if (strcmp(FindFile.m_pszName,pFile->m_pszName)==0)
    {
      if (strcmp(FindFile.m_pszEnding,pFile->m_pszEnding)==0)
      {
      //if (pFile->m_pszPath2)
      {
        //if (strcmp(FindFile.m_pszPath2,pFile->m_pszPath2)==0)
        {
          pFileIndex = pFile;
          break;
        }
      }
      }
    }
  }

  return pFileIndex;
}
*/
//-------------------------------------------------------------
CCompareFile * CFileDiff::InsertFile1(LPSTR lpPath)
{
  EnterCriticalSection(&m_List_criticalsection);
  if (m_MaxNumListItem==0)
  {
    m_NumListItem    = 0;
    m_MaxNumListItem = 120;

    m_rgListItem = (CListItem **) malloc ((m_MaxNumListItem)*sizeof(CListViewItem*));
    memset(m_rgListItem,0,m_MaxNumListItem*sizeof(CListViewItem*));
  }
  else if ((m_NumListItem+2) >  m_MaxNumListItem)
  {
    m_MaxNumListItem = m_NumListItem+ 20;
    m_rgListItem    = (CListItem**) realloc(m_rgListItem,m_MaxNumListItem * sizeof(CListViewItem*));
  }


  StepBusyDisplay();
  CCompareFile* pFileIndex = NULL;
  CCompareFile* pFile      = NULL;
  CCompareFile* pNewFile   = (CCompareFile*)  new CCompareFile(this,0);
  pNewFile->m_FileDiffState= LEFT_ONLY;
  pNewFile->SetFilePath1(lpPath);
  pNewFile->m_SortKey      =  pNewFile->MakeSortKey(m_SortMode);

  // is the file already member of the list
  // CCompareFile* pFileIndex = FindFile2(lpPath);

  for (int i=0;i < m_NumListItem && m_rgListItem[i];i++)
  {
    pFile = (CCompareFile*)m_rgListItem[i];
    if (strcmp(pNewFile->m_pszName,pFile->m_pszName)==0)
    {
      if (strcmp(pNewFile->m_pszEnding,pFile->m_pszEnding)==0)
      {
        // File passt
        if (pFile->m_pszPath1==NULL || (strcmp(pNewFile->m_pszPath1,pFile->m_pszPath1)!=0))
        {
          // 1 noch nicht vorhanden !
          if (pFile->m_pszPath2)
          {
            if (strcmp(pNewFile->m_pszPath1,pFile->m_pszPath2)==0)
            {
              // Pfad 2 vorhanden und passt
              pNewFile->m_FileDiffState=IDENTICAL;
              pFile->m_FileDiffState=IDENTICAL;
              pFileIndex = pFile;
              break;
            }
          }
          else
          {
            // Pfad 1 auch noch nicht vorhanden
            pNewFile->m_FileDiffState = LEFT_ONLY;
            pFileIndex = NULL;
            break;
          }
        }
        else
        {
          // Fehler schon vorhanden !
          pNewFile->m_FileDiffState=FIDENTICAL1;
          pFileIndex = pFile;
          break;
        }
      }
    }
  }

  if (pFileIndex==NULL)
  {
    // add a new Listmember
    m_rgListItem[m_NumListItem] = pFileIndex = pNewFile;
    m_NumListItem++;
  }
  else
  {
    // 2 in Liste vorhanden
    delete pNewFile;
    if (pFileIndex->m_pszPath1==NULL)pFileIndex->SetFilePath1(lpPath);
    else                             pFileIndex->m_FileDiffState=DIFFERENT2;
  }


  LeaveCriticalSection(&m_List_criticalsection);

  return pFileIndex;
}
//-------------------------------------------------------------
CCompareFile * CFileDiff::InsertFile2(LPSTR lpPath)
{
  EnterCriticalSection(&m_List_criticalsection);
  if (m_MaxNumListItem==0)
  {
    m_NumListItem    = 0;
    m_MaxNumListItem = 120;

    m_rgListItem = (CListItem **) malloc ((m_MaxNumListItem)*sizeof(CListViewItem*));
    memset(m_rgListItem,0,m_MaxNumListItem*sizeof(CListViewItem*));
  }
  else if ((m_NumListItem+2) >  m_MaxNumListItem)
  {
    m_MaxNumListItem = m_NumListItem+ 20;
    m_rgListItem    = (CListItem**) realloc(m_rgListItem,m_MaxNumListItem * sizeof(CListViewItem*));
  }

  StepBusyDisplay();

  CCompareFile* pFileIndex = NULL;
  CCompareFile* pFile      = NULL;
  CCompareFile* pNewFile   = (CCompareFile*)  new CCompareFile(this,0);
  pNewFile->m_FileDiffState= RIGHT_ONLY;
  pNewFile->SetFilePath2(lpPath);
  pNewFile->m_SortKey      =  pNewFile->MakeSortKey(m_SortMode);

  // is the file already member of the list
  // CCompareFile* pFileIndex = FindFile2(lpPath);

  for (int i=0;i < m_NumListItem && m_rgListItem[i];i++)
  {
    pFile = (CCompareFile*)m_rgListItem[i];
    if (strcmp(pNewFile->m_pszName,pFile->m_pszName)==0)
    {
      if (strcmp(pNewFile->m_pszEnding,pFile->m_pszEnding)==0)
      {
        // File passt
        if (pFile->m_pszPath2==NULL || (strcmp(pNewFile->m_pszPath2,pFile->m_pszPath2)!=0))
        {
          // 2 noch nicht vorhanden !
          if (pFile->m_pszPath1)
          {
            if (strcmp(pNewFile->m_pszPath2,pFile->m_pszPath1)==0)
            {
              // Pfad 1 vorhanden und passt
              pNewFile->m_FileDiffState=IDENTICAL;
              pFile->m_FileDiffState=IDENTICAL;
              pFileIndex = pFile;
              break;
            }
          }
          else
          {
            // Pfad 1 auch noch nicht vorhanden
            pNewFile->m_FileDiffState = RIGHT_ONLY;
            pFileIndex = NULL;
            break;
          }
        }
        else
        {
          // Fehler schon vorhanden !
          pNewFile->m_FileDiffState=FIDENTICAL2;
          pFileIndex = NULL;
          break;
        }
      }
    }
  }



  if (pFileIndex==NULL)
  {
    // add a new Listmember
    m_rgListItem[m_NumListItem] = pFileIndex = pNewFile;

    m_NumListItem++;
  }
  else
  {
    // 1 in Liste vorhanden
    delete pNewFile;
    if (pFileIndex->m_pszPath2==NULL)pFileIndex->SetFilePath2(lpPath);
    else                             pFileIndex->m_FileDiffState=DIFFERENT1;
  }

  LeaveCriticalSection(&m_List_criticalsection);

  return pFileIndex;
}
//--------------------------------------------------------------
int CFileDiff::AddFolder(LPSTR lpPath,int nmbr)
{
  SetBusyDisplay(-1,lpPath);
#ifdef WIN32
  int num_entries;
  WIN32_FIND_DATA findData;
  HANDLE fileHandle= INVALID_HANDLE_VALUE;


  char str[_MAX_PATH];

  num_entries = 0;
  size_t path_len;//, str_size;
  path_len = strlen(lpPath);
  strcpy(str, lpPath);
  strcat(str,"/");
  path_len++;

  // Get the first file in the directory
  BOOL FindF=true;
  BOOL bFirstRun=true;
  int Index=0;
  // Loop on all remaining entries in the directory
  while (FindF && m_StartScanFolderCnt==0 && gAppRun)
  {
    if (bFirstRun)
    {
      bFirstRun = false;
      char str3[MAX_PATH];
      strcpy(str3,lpPath);
      strcat(str3,"/*.*");
	  fileHandle = FindFirstFile(str3, &findData);
      FindF      = (fileHandle != INVALID_HANDLE_VALUE);
    }
    else
    {
      FindF  = FindNextFile(fileHandle, &findData);
    }
    //-------------- Eintrag gefunden ? -------------
    if (FindF)
    {
      strcpy(&str[path_len], findData.cFileName);
      if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        if (lstrcmpi(findData.cFileName, TEXT("."))!=0
          && lstrcmpi(findData.cFileName, TEXT(".."))!=0)
        {
          /* unless ".", "..", ".hidden" or vidix driver dirs */
          num_entries += AddFolder(str,nmbr);
        }
      }
      else
      {
/*        if   ( (strstr(str, ".mpg"))
          ||   (strstr(str, ".ts"))
          ||   (strstr(str, ".ts2"))
          ||   (strstr(str, ".mp3"))
          ||   (strstr(str, ".jpg"))
          ||   (strstr(str, ".c"))
          ||   (strstr(str, ".rc"))
          ||   (strstr(str, ".cpp"))
          ||   (strstr(str, ".h"))
          )*/
        {
          CCompareFile * pFile    = NULL;//InsertFile(str);
          if      (nmbr  == 0)pFile   = InsertFile1(str);
          else if (nmbr  == 1)pFile    = InsertFile2(str);
          if (pFile)
          {
//            pFile ->m_st_size     = findData.nFileSizeLow;
            struct stat statbuffer;
            //if (
            stat(str, &statbuffer);//>=0) pFile ->m_st_mtime= statbuffer.st_mtime;//.st_ctime;//.st_atime;
//            else pFile ->m_st_mtime= 0;

            if (pFile->m_pszPath1 && pFile->m_pszPath2)
            {
              if (pFile ->m_st_size   != findData.nFileSizeLow)pFile ->m_FileDiffState |= DIFFERENTSIZE;
              if (pFile ->m_st_mtime  != statbuffer.st_mtime)pFile ->m_FileDiffState |= DIFFERENTDATE;
              pFile ->m_ShowCnt     = 0;
            }
            else
            {
              pFile ->m_st_size     = findData.nFileSizeLow;
              pFile ->m_st_mtime    = statbuffer.st_mtime;
              pFile ->m_ShowCnt     = 0;
            }


            pFile ->m_ShowCnt     = 0;
            /*
            if (m_FileDiffSetState==-1)
            {
              // get the first State
              m_FileDiffSetState = (pFile ->GetState() > 0);
              if (SetState)
              {
                if (m_FileDiffSetState==0)
                {
                  // the first item is in the 'none' state
                  // select this index to play next and run the FileDiff
                  m_explicit_play_index = m_NumListItem-1;
                  m_play_List_Test_Pause = 2;
                  m_State = 1;
                }
                else
                {
                  // stop the FileDiff
                  m_State = 0;
                  if (gDesk.m_MP3Player && gDesk.m_MP3Player->GetState()>=0)
                  {
                    // stop the current media file
                    if (m_stop_exit_Test_Pause ==0) gDesk.m_MP3Player->Stop();//Terminate();
                    m_stop_exit_Test_Pause = 40;
                  }
                }
              }
            }
            if (SetState) pFile ->SetState(m_FileDiffSetState?file_diff_state_none:file_diff_state_copy);
            */
          }
          num_entries ++;
        }

      }
    }
  }
  if (fileHandle != INVALID_HANDLE_VALUE )  FindClose(fileHandle);

  return num_entries;
#else
  int num_entries;
  DIR  *dir;
  struct stat statbuffer;
  char str[_MAX_PATH];

  num_entries = 0;
  dir = opendir(lpPath);
  if (dir)
  {
    struct dirent *pEntry;
    size_t path_len;//, str_size;
    path_len = strlen(lpPath);
	strcpy(str, lpPath);
    strcat(str,"/");
	path_len++;

    while ((pEntry = readdir (dir)) != NULL  && m_StartScanFolderCnt==0 && gAppRun)
    {

      strcpy(&str[path_len], pEntry->d_name);
      if (stat(str, &statbuffer))
	  {
		//xine_log (this, XINE_LOG_PLUGIN, _("load_plugins: unable to stat %s\n"), str);
      }
	  else
	  {

		switch (statbuffer.st_mode & S_IFMT)
		{

        case S_IFCHR:     // character device

        case S_IFBLK:     // block device

        //case S_IFFIFO:    // FIFO (named pipe)

        case S_IFLNK:     // symbolic link? (Not in POSIX.1-1996.)

        case S_IFSOCK:    // socket? (Not in POSIX.1-1996.)

        case S_IFREG:
          {
/*            if   ( (strstr(str, ".mpg"))
              ||   (strstr(str, ".ts"))
              ||   (strstr(str, ".ts2"))
              ||   (strstr(str, ".mp3"))
              ||   (strstr(str, ".jpg"))
              ||   (strstr(str, ".c"))
              ||   (strstr(str, ".rc"))
              ||   (strstr(str, ".cpp"))
              ||   (strstr(str, ".h"))
              )*/
            {
              CCompareFile * pFile    = NULL;//InsertFile(str);
              if      (nmbr == 0)pFile    = InsertFile1(str);
              else if (nmbr == 1)pFile    = InsertFile2(str);
              if (pFile)
              {
                if (pFile->m_pszPath1 && pFile->m_pszPath2)
                {
                  if (pFile ->m_st_size   != statbuffer.st_size )pFile ->m_FileDiffState |= DIFFERENTSIZE;
                  if (pFile ->m_st_mtime  != statbuffer.st_mtime)pFile ->m_FileDiffState |= DIFFERENTDATE;
                  pFile ->m_ShowCnt     = 0;
                }
                else
                {
                  pFile ->m_st_size     = statbuffer.st_size;
                  pFile ->m_st_mtime    = statbuffer.st_mtime;
                  pFile ->m_ShowCnt     = 0;
                }

                /*if (m_FileDiffSetState==-1)
                {
                  // get the first State
                  m_FileDiffSetState = (pFile ->GetState() > 0);
                  if (SetState)
                  {
                    if (m_FileDiffSetState==0)
                    {
                      // the first item is in the 'none' state
                      // select this index to play next and run the FileDiff
                      m_explicit_play_index = m_NumListItem-1;
                      m_play_List_Test_Pause = 2;
                      m_State = 1;
                    }
                    else
                    {
                      // stop the FileDiff
                      m_State = 0;
                      if (gDesk.m_MP3Player && gDesk.m_MP3Player->GetState()>=0)
                      {
                        // stop the current media file
                        if (m_stop_exit_Test_Pause ==0) gDesk.m_MP3Player->Stop();//Terminate();
                        m_stop_exit_Test_Pause = 40;
                      }
                    }
                  }
                }
                if (SetState) pFile ->SetState(m_FileDiffSetState?file_diff_state_none:file_diff_state_copy);
                */
              }
              num_entries ++;
            }

          }
		  break;
		case S_IFDIR:
		  if (*pEntry->d_name != '.')
		  {
            num_entries += AddFolder(str,nmbr);
          }
          break;
        }
      }

    }
  }
  closedir (dir);
  return num_entries;
#endif
}
//-------------------------------------------------------------
int CFileDiff::InitFileDiff(int ListMode, int AudioOnly)
{

#define DIFFERENT DIFFERENTSIZE | DIFFERENTDATE | DIFFERENT1|DIFFERENT2
  if (gDesk.m_pButton[4])
  {
  m_ActiveFilter  =  (gDesk.m_pButton[4]->GetState()&OPTION_ON)?DIFFERENT :0;
  m_ActiveFilter |=  (gDesk.m_pButton[5]->GetState()&OPTION_ON)?IDENTICAL :0;
  m_ActiveFilter |=  (gDesk.m_pButton[6]->GetState()&OPTION_ON)?LEFT_ONLY :0;
  m_ActiveFilter |=  (gDesk.m_pButton[7]->GetState()&OPTION_ON)?RIGHT_ONLY:0;
  }
  m_AudioOnly=1;
  //SortItems();
  return 0;
}
//-------------------------------------------------------------
//-------------------------------------------------------------
//-------------------------------------------------------------
/*
CCompareFolder::CCompareFolder(PWNDCONTROL pWndControl,CTVItem * pParent, DWORD record):CFolder(pWndControl,pParent,record)
//CTVItem(pWndControl,pParent,record)
{
  if (pParent==NULL)
  {
#ifdef WIN32
    gBastelUtils.GetProfileString("FileDiff","MusicFolder","\\\\SERVER/USB_Storage/Media/Music",szPathMusicFolder,sizeof(szPathMusicFolder));
#else
    gBastelUtils.GetProfileString("FileDiff","MusicFolder","/record/Musik",szPathMusicFolder,sizeof(szPathMusicFolder));
#endif
  }
}
//-------------------------------------------------------------
CCompareFolder::~CCompareFolder()
{
}
//-------------------------------------------------------------
BOOL CCompareFolder::ExpandItem(int Hit)
{
  if (m_pChild)
  {
	// wir haben Children : öffnen oder löschen!
	m_bExpand = !m_bExpand;
	if (m_bExpand==0  && Hit == VK_LEFT)
	{
	  if (m_pChild) delete (m_pChild);
	  m_pChild=NULL;
	}
  }
  else if ((m_NumChild & 0x7FFFFFFF)==0)
  {
	char    szPath[MAX_PATH];
	BuildDirPath(szPath);
	GetSubDirectory(szPath);
	m_bExpand = 1;
  }
  return 0;
}
//-------------------------------------------------------------
unsigned int CCompareFolder::MakeSortKey()
{
  unsigned int SortKey = 0;
  char str[256];
  memset (&str,0,sizeof(str));
  strcpy(str,GetText());
  for (int i=0;i<6;i++)
  {
    SortKey = SortKey * 40;
    unsigned int ch = (unsigned int)str[i];
    if      (ch <  '0')              ch = 0;
    else if (ch <= '9')              ch = ch - '0';
    else if (ch <  '@')              ch = 11;
    else if (ch <= 'Z')              ch = ch - 64 + 11; //'@ABCD..'90
    else if (ch <  'a')              ch = 39;
    else if (ch <= 'z')              ch = ch - 96 + 11; //'`abc..'z 122
    else                             ch = 40;
    SortKey += ch;
  }
  SortKey += 100000000;
  return  SortKey;

}
//-------------------------------------------------------------
static int Deepth = 0;

int CCompareFolder::GetSubDirectory(LPSTR pszDirectory)
{
#ifdef WIN32
  WIN32_FIND_DATA findData;
  HANDLE fileHandle= INVALID_HANDLE_VALUE;
  int fInserted = false;
  int  Image;
  int  NumFiles=0;
  Deepth++;

  char str[MAX_PATH];
  CCompareFolder* pTVItem1=(CCompareFolder*)m_pChild;

  while (pTVItem1 && pTVItem1->m_pNext) pTVItem1 = (CCompareFolder*)pTVItem1->m_pNext;

  // Get the first file in the directory
  BOOL FindF=true;
  BOOL bFirstRun=true;
  int Index=0;
  // Loop on all remaining entries in the directory

  size_t path_len;
  path_len = strlen(pszDirectory);
  strcpy(str, pszDirectory);
  strcat(str,"/*.*");
  path_len++;
  while (FindF)
  {
    if (bFirstRun)
    {
      bFirstRun = false;
      fileHandle = FindFirstFile(str, &findData);
      FindF      = (fileHandle != INVALID_HANDLE_VALUE);
    }
    else
    {
      FindF  = FindNextFile(fileHandle, &findData);
    }
    //-------------- Eintrag gefunden ? -------------
    if (FindF)
    {
      strcpy(&str[path_len],findData.cFileName);
      if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        if (lstrcmpi(findData.cFileName, TEXT("."))!=0
          && lstrcmpi(findData.cFileName, TEXT(".."))!=0)
        {
          Image  = imMusicFolder;
          NumFiles++;
          pTVItem1 = (CCompareFolder*)new CCompareFolder(m_pWndControl,this,1);
          if (pTVItem1)
          {
            pTVItem1->m_lParam  = 1;
            pTVItem1->SetImage(Image);
            pTVItem1->SetText(findData.cFileName);
            if (HasDirectorySubDirs(str))
            {
              m_bExpand = 1;
              pTVItem1->m_NumChild = INSERTCHILDREN;
            }
            else
            {
              m_bExpand = 0;
              pTVItem1->m_NumChild = 0;
            }
            pTVItem1->m_SortKey = ((CCompareFolder*)pTVItem1) ->MakeSortKey();
            InsertChildItem(pTVItem1,INSERT_SORT);
          }
        }
      }
    }
  }
  if (fileHandle != INVALID_HANDLE_VALUE )  FindClose(fileHandle);

  Deepth--;
  return   Deepth;
#else
  int fInserted = false;
  int  Image;
  int  NumFiles=0;
  DIR  *dir;
  Deepth++;

  char str[MAX_PATH];
  CCompareFolder* pTVItem1=(CCompareFolder*)m_pChild;

  while (pTVItem1 && pTVItem1->m_pNext) pTVItem1 = (CCompareFolder*)pTVItem1->m_pNext;

  dir = opendir(pszDirectory);
  if (dir)
  {
    struct dirent *pEntry;
    size_t path_len;//, str_size;

    path_len = strlen(pszDirectory);
	strcpy(str, pszDirectory);
    strcat(str,"/");
	path_len++;

    while ((pEntry = readdir (dir)) != NULL)
	{
      struct stat statbuffer;

      strcpy(&str[path_len], pEntry->d_name);

      if (stat(str, &statbuffer))
	  {
      }
	  else
	  {

		switch (statbuffer.st_mode & S_IFMT)
		{
		case S_IFDIR:
		  //  unless ".", "..", ".hidden" or vidix driver dirs 
		  if (*pEntry->d_name != '.')
          {
            Image  = imMusicFolder;
            NumFiles++;

            pTVItem1 = (CCompareFolder*)new CCompareFolder(m_pWndControl,this,1);
            if (pTVItem1)
            {
              pTVItem1->m_lParam  = 1;
              pTVItem1->SetImage(Image);
              pTVItem1->SetText(pEntry->d_name);//pnt);//str);
              if (HasDirectorySubDirs(str))
              {
                m_bExpand = 1;
                pTVItem1->m_NumChild = INSERTCHILDREN;
              }
              else
              {
                m_bExpand = 0;
                pTVItem1->m_NumChild = 0;
              }
              pTVItem1->m_SortKey = 0xFFFFFF00;
              pTVItem1->m_SortKey = ((CCompareFolder*)pTVItem1) ->MakeSortKey();
              InsertChildItem(pTVItem1,INSERT_SORT);
            }
          }
		}
      }
    }
    closedir (dir);
  }
  else
  {
  }

  Deepth--;
  return   Deepth;
#endif
}

//-------------------------------------------------------------
//-------------------------------------------------------------
BOOL CCompareFolder::SelChanging(int nIndex,unsigned int wParam)
{
  int ret=1;
  if (gDesk.m_FileDiff) gDesk.m_FileDiff->m_StartScanFolderCnt=0;
  m_Selected = 0;
  return ret;
}

//-------------------------------------------------------------
BOOL CCompareFolder::SelChanged(int nIndex)
{
  m_Selected = 1;
  char Path[MAX_PATH];
  if (m_pParent)
  {
    ((CCompareFolder*)m_pParent)->BuildDirPath(Path);
    strcat(Path,"/");
    strcat(Path,m_pText);
#ifndef WIN32
    int Index = (gDesk.m_pTree[1]->GetFocus() == gDesk.m_pTree[1]->GetHWnd());
#else
    int Index = (::GetFocus() == gDesk.m_pTree[1]->GetHWnd());
#endif
    // scan the subdir
    if ( Index ==0)
    {
      if (strncmp(CFileDiff::m_FileDiffPath1,Path,CFileDiff::m_FileDiffPath1len)==0)
      {
        strcpy(CFileDiff::m_FileSubDir,&Path[CFileDiff::m_FileDiffPath1len]);
      }
      else memset(&CFileDiff::m_FileSubDir,0,sizeof(CFileDiff::m_FileSubDir));
      if (gDesk.m_FileDiff) gDesk.m_FileDiff->FileDiffScanJob(Path,0);
    }
    else
    {
      if (strncmp(CFileDiff::m_FileDiffPath2,Path,CFileDiff::m_FileDiffPath2len)==0)
      {
        strcpy(CFileDiff::m_FileSubDir,&Path[CFileDiff::m_FileDiffPath2len]);
      }
      else memset(&CFileDiff::m_FileSubDir,0,sizeof(CFileDiff::m_FileSubDir));
      if (gDesk.m_FileDiff) gDesk.m_FileDiff->FileDiffScanJob(Path,0);
    }
  }
  return 1;
}
//-------------------------------------------------------------

int CCompareFolder::SetState(int iState)
{
  if (m_State==0 && iState && gDesk.m_FileDiff)
  {
    gDesk.m_FileDiff->m_StartScanFolderCnt=0;
    char Path[MAX_PATH];
    if (m_pParent)
    {
      ((CCompareFolder*)m_pParent)->BuildDirPath(Path);
      strcat(Path,"/");
      strcat(Path,m_pText);
    }
    else
    {
      strcpy(Path,szPathMusicFolder);
    }
    memset(&gDesk.m_FileDiff->m_FileSubDir,0,sizeof(gDesk.m_FileDiff->m_FileSubDir));
    gDesk.m_FileDiff->FileDiffScanJob(Path,1);
    m_State=1;
    m_pWndControl->m_pCurItem = (CTVItem*) this;
  }
  else
  {
    m_State=0;
    m_pWndControl->m_pCurItem = NULL;
  }
  return 1;
}
 */
//-------------------------------------------------------------

