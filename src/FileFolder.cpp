// FileFolder.cpp: Implementierung der Klasse CRecords.
//
//////////////////////////////////////////////////////////////////////
#include <string.h>
#include "FileFolder.h"
#include "Resource.h"
#ifndef WIN32
#include <dirent.h>
#endif
#include "FileMerge.h"
#include "FileTable.h"

char gszHomeFilePath[64];
char gszExtention   [64];
//static tMessage msg;
extern int AutoScan;

//-------------------------------------------------------------
CFolder::CFolder(PWNDCONTROLW pWndControl,CTVItem * pParent, DWORD record):CTVItem(pWndControl,pParent,record)
{

}
//-------------------------------------------------------------
CFolder::~CFolder()
{
}
//-------------------------------------------------------------
int  CFolder::ExpandItem(int Hit)
{
  ResetScanStatus();
  m_pWndControl->m_Busy++;
  if (m_pChild) {
    // wir haben Children : oeffnen oder loeschen!
    return CTVItem::ExpandItem(Hit);
  } else if ((m_NumChild & 0x7FFFFFFF)==0) {
    char    szDir[MAX_PATH];
    BuildDirPath(szDir);
    ClearCmpFileNameList();
    GetDirectoryContents(szDir);
    AdjustFileTable();
    Expand();
  }
  m_pWndControl->Resize();
  m_pWndControl->m_Busy--;
  return 0;
}
//-------------------------------------------------------------
LPSTR  CFolder::GetParentName(LPSTR lpszDir)
{
  const char * pnt = GetText();
  const char * pntP=NULL;
  if (m_pParent) {
    pntP = m_pParent->GetText();
  }
  if (m_Level==0 && gCmpSheet) {
    int  i = m_Image - (20+2);
    int n = sizeof(g_DocsPath[0])/sizeof(g_DocsPath[0][0]);
    if (i>=0 && i < n) {
      if (i!= gDocShowPrimaryPath) {
        gDocShowPrimaryPath=i;
        gToolBox->CheckRadioButton(IDC_DocsBuero,IDC_DocsSys,IDC_Docs+gDocShowPrimaryPath);
      }
      strcpy(lpszDir,g_DocsPath[0][i]);
      return 0;
    }
  }

//  if ((pntP && strcmp(pntP,"System")==0)) {
//    if (strcmp(pnt,"Home")==0) {
//      strcat(lpszDir,gszHomeFilePath);
//      return 0;
//    } else if (strcmp(pnt,"Root")==0) {
//#ifdef WIN32
//      strcat(lpszDir,"D:");
//#else
//      strcat(lpszDir,"/");
//#endif
//      return 0;
//    }
//  }
  if (m_pParent) {
    ((CFolder*)this->m_pParent)->GetParentName(lpszDir);
  }

  if (pnt) {
#ifdef WIN32
    if (lpszDir[0]!= '\0') strcat(lpszDir,"/");
#else
    strcat(lpszDir,"/");
#endif
    strcat(lpszDir,pnt);
  }
  return 0;
}

//-------------------------------------------------------------
int CFolder::BuildDirPath(LPSTR lpszDir)
{
  strcpy(lpszDir, "");
  GetParentName(lpszDir);
  LPSTR pnt = GetText();
  return pnt != NULL;
}

//-------------------------------------------------------------
static int Deepth = 0;
#ifdef WIN32
double time_d(FILETIME ft)
{
//  FILETIME ft;
//  GetSystemTimeAsFileTime(&ft);
  __int64* val = (__int64*) &ft;
  return static_cast<double>(*val) / 10000000.0 - 11644473600.0;   // epoch is Jan. 1, 1601: 134774 days to Jan. 1, 1970
}
#endif

int CFolder::GetDirectoryContents(LPCSTR pszDirectory)
{
#ifdef WIN32
  WIN32_FIND_DATA findData;
  HANDLE fileHandle;

  int fInserted = false;
  int  Image;
  int  NumFiles=0;
  Deepth++;

  char str[MAX_PATH];
  CFolder* pTVItem1=(CFolder*)m_pChild;

  while (pTVItem1 && pTVItem1->m_pNext) pTVItem1 = (CFolder*)pTVItem1->m_pNext;

  size_t path_len;
  path_len = strlen(pszDirectory);
  strcpy(str, pszDirectory);
  strcat(str,"/*.*");
  path_len++;


  // Get the first file in the directory
  int  FindF=true;
  int  bFirstRun=true;
  int Index=0;
  // Loop on all remaining entries in the directory
  while (FindF) {
    if (bFirstRun) {
      bFirstRun = false;
      fileHandle = FindFirstFile(str, &findData);
      FindF      = (fileHandle != INVALID_HANDLE_VALUE);
    } else {
      FindF  = FindNextFile(fileHandle, &findData);
    }
    //-------------- Eintrag gefunden ? -------------
    if (FindF) {
      strcpy(&str[path_len],findData.cFileName);
      if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        if ( lstrcmpi(findData.cFileName, TEXT("."))!=0
             && lstrcmpi(findData.cFileName, TEXT(".."))!=0) {
          /* unless ".", "..", ".hidden" or vidix driver dirs */
          if (HasDirectoryContents(str)) {
            //Image  = iFile; //gMain.g_pTree->Image_from_DirectoryContents(str,&ID);
            Image  = imFolderClose;
            NumFiles++;
            m_bExpand = 1;
            {
              pTVItem1 = (CFolder*)new CFolder(m_pWndControl,this,0);
            }
            if (pTVItem1) {
              //pTVItem1->m_lParam  = (void*)(long)1;
              pTVItem1->m_lParam  = 1;
              pTVItem1->SetImage(Image);
              pTVItem1->SetText(findData.cFileName);
              pTVItem1->m_NumChild = INSERTCHILDREN;

              pTVItem1->m_SortKey = -time_d(findData.ftLastWriteTime);
              InsertChildItem(pTVItem1,INSERT_SORT);
              IncrementDirScanStatus();
            }
          } else {
            Image  = imFolderClose;
            NumFiles++;
            m_bExpand = 0;
            {
              pTVItem1 = (CFolder*)new CFolder(m_pWndControl,this,0);
            }
            if (pTVItem1) {
              //pTVItem1->m_lParam  = (void*)(long)1;
              pTVItem1->m_lParam  = 1;
              pTVItem1->SetImage(Image);
              pTVItem1->SetText(findData.cFileName);
              pTVItem1->m_NumChild = 0;
              //InsertChildItem(pTVItem1,INSERT_END);
              pTVItem1->m_SortKey = -time_d(findData.ftLastWriteTime);// 0xFFFFFF00;
              //pTVItem1->m_SortKey = 0xFFFFFF00;
              InsertChildItem(pTVItem1,INSERT_SORT);
              IncrementDirScanStatus();
            }
          }
        }
      } else {
        if (1) {
//          AddCmpFileName(str);
          NumFiles++;
        } else {
          pTVItem1 = NULL;
          char *ending;
          ending = strrchr(str, '.');
          if (ending) {
            strlwr(ending);
            //if (gszExtention[0]== '\0' || (strcasecmp(ending, gszExtention)==0)) {
            if (gszExtention[0]== '\0' || (strstr(gszExtention,ending))) {
              if ((strncasecmp(ending, ".mp3",5)==0)) pTVItem1 = (CFolder*)new CFile(m_pWndControl,this,0),  Image =  imMusic;
              else                          pTVItem1 = (CFolder*)new CFile(m_pWndControl,this,0),Image =  imDokument;
              AddCmpFileName(0,str);
            }
          } else {
            pTVItem1 = (CFolder*)new CFile(m_pWndControl,this,0),Image =  imDokument;
          }

          if (pTVItem1) {
            //pTVItem1->m_lParam  = (void*)(long)1;
            pTVItem1->m_lParam  = 1;
            pTVItem1->SetImage(Image);
            pTVItem1->SetText(findData.cFileName);
            //pTVItem1->m_SortKey = ((CFile*)pTVItem1) ->MakeSortKey();
            pTVItem1->m_SortKey = -time_d(findData.ftLastWriteTime);

            IncrementFileScanStatus();
            InsertChildItem(pTVItem1,INSERT_SORT);
            fInserted = true;
            NumFiles++;
          }
        }
      }
    }
  }
  if (fileHandle != INVALID_HANDLE_VALUE )  FindClose(fileHandle);
  Deepth--;
  return   Deepth;


#else
  int  Image;
  int  NumFiles=0;
  DIR  *dir;
  Deepth++;

  char str[MAX_PATH];
  CFolder* pTVItem1=(CFolder*)m_pChild;

  while (pTVItem1 && pTVItem1->m_pNext) pTVItem1 = (CFolder*)pTVItem1->m_pNext;

  dir = opendir(pszDirectory);
  if (dir) {
    struct dirent *pEntry;
    size_t path_len;//, str_size;

    path_len = strlen(pszDirectory);
    strcpy(str, pszDirectory);
    strcat(str,"/");
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

        case S_IFREG: {
          if (0) {
//            AddCmpFileName(str);
            NumFiles++;
          } else {
            pTVItem1 = NULL;
            char *ending;
            ending = strrchr(str, '.');
            if (ending) {
              //if (gszExtention[0]== '\0' || (strcasecmp(ending, gszExtention)==0)){
              if (gszExtention[0]== '\0' || (strstr(gszExtention,ending))) {

                if (1||(strncasecmp(ending, ".mp3",5)==0)) {
                  pTVItem1 = (CFolder*)new CFile(m_pWndControl,this,0);
                  Image =  imMusic;
                  AddCmpFileName(0,str);
                } else                          pTVItem1 = (CFolder*)new CFile(m_pWndControl,this,0),Image =  imDokument;
              }
            } else {
              pTVItem1 = (CFolder*)new CFile(m_pWndControl,this,0),Image =  imDokument;
              //if ((statbuffer.st_mode & S_IXUSR)!=0)       pTVItem1 = (CFolder*)new CFile(m_pWndControl,this,IDC_System ),Image =  imApllication;
              //else                                         pTVItem1 = (CFolder*)new CFile(m_pWndControl,this,IDC_Picture),Image =  imDokument;
            }

            if (pTVItem1) {
              pTVItem1->m_lParam  = 1;//(void*)(long)1;
              pTVItem1->SetImage(Image);
              pTVItem1->SetText(pEntry->d_name);
              //pTVItem1->m_SortKey = ((CFile*)pTVItem1) ->MakeSortKey();
              pTVItem1->m_SortKey = -statbuffer.st_ctime;
              //IncrementDirScanStatus();
              InsertChildItem(pTVItem1,INSERT_SORT);
              NumFiles++;
            }
            IncrementFileScanStatus();
          }
        }
        break;
        case S_IFDIR:

          /* unless ".", "..", ".hidden" or vidix driver dirs */
          if ((  strcmp(pEntry->d_name,".")!=0)
              && (   strcmp(pEntry->d_name,"..")!=0)) {
            if (HasDirectoryContents(str)) {
              //Image  = iFile; //gMain.g_pTree->Image_from_DirectoryContents(str,&ID);
              Image  = imFolderClose;
              NumFiles++;
              m_bExpand = 1;
              pTVItem1 = (CFolder*)new CFolder(m_pWndControl,this,0);
              if (pTVItem1) {
                pTVItem1->m_lParam  = 1;//(void*)(long)1;
                pTVItem1->SetImage(Image);
                pTVItem1->SetText(pEntry->d_name);//pnt);//str);
                pTVItem1->m_NumChild = INSERTCHILDREN;
                //pTVItem1->m_SortKey = 0xFFFFFF00;
                pTVItem1->m_SortKey = -statbuffer.st_ctime;
                InsertChildItem(pTVItem1,INSERT_SORT);
                IncrementDirScanStatus();
              }
            } else {
              Image  = imFolderClose;
              NumFiles++;
              m_bExpand = 0;
              {
                pTVItem1 = (CFolder*)new CFolder(m_pWndControl,this,0);
              }
              if (pTVItem1) {
                pTVItem1->m_lParam  = 1;//(void*)(long)1;
                pTVItem1->SetImage(Image);
                pTVItem1->SetText(pEntry->d_name);
                pTVItem1->m_NumChild = 0;
                //InsertChildItem(pTVItem1,INSERT_END);
                //pTVItem1->m_SortKey = 0xFFFFFF00;
                pTVItem1->m_SortKey = -statbuffer.st_ctime;

                InsertChildItem(pTVItem1,INSERT_SORT);
                IncrementDirScanStatus();
              }
            }
          }
        }
      }
    }
    closedir (dir);
  } else {
  }
  Deepth--;
  return   Deepth;
#endif
}

//-------------------------------------------------------------
int CFolder::HasDirectoryContents(LPCSTR pszDirectory)
{
#ifdef WIN32
  WIN32_FIND_DATA findData;
  HANDLE fileHandle;
  int  fInserted = FALSE;
  int NumFiles=0;
  size_t path_len;
  //bNixFiles=0;
  path_len = strlen(pszDirectory);
  char str[MAX_PATH];
  strcpy(str,pszDirectory);
  strcat(str,"/*.*");
  path_len++;
  try {
    // Get the first file in the directory
    int  FindF=true;
    int  bFirstRun=true;
    // Loop on entries in the directory
    while (FindF) { // && NumFiles==0)
      if (bFirstRun) {
        bFirstRun = false;
        fileHandle = FindFirstFile(str, &findData);
        FindF  = (fileHandle != INVALID_HANDLE_VALUE);
      } else {
        FindF  = FindNextFile(fileHandle, &findData);
      }
      if (FindF) {
        strcpy(&str[path_len],findData.cFileName);
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
          if (lstrcmpi(findData.cFileName, TEXT("."))!=0
              && lstrcmpi(findData.cFileName, TEXT(".."))!=0) {
            NumFiles++;
          }
        } else {
          char *ending;
          ending = strrchr(findData.cFileName, '.');
          if (ending) {
            strlwr(ending);
            //if (gszExtention[0]== '\0' || (strcasecmp(ending, gszExtention)==0)) {
            if (gszExtention[0]== '\0' || (strstr(gszExtention,ending))) {
              //if (gszExtention[0]== '\0' || (strcasecmp(ending, gszExtention)==0)) {
              NumFiles++;
            }
          }
        }
      }
    }
    // All done, everything worked.
    fInserted = NumFiles!=0;
  } catch (...) {
    if (fileHandle != INVALID_HANDLE_VALUE )
      FindClose(fileHandle);
  }
  //bNixFiles =  (bNixFiles == NumFiles);
  return (fInserted);

#else
  DIR *dir;
  dir = opendir(pszDirectory);
  int NumFiles = 0;
  if (dir) {
    char str[MAX_PATH];
    struct dirent *pEntry;
    size_t path_len;//, str_size;

    path_len = strlen(pszDirectory);
    strcpy(str, pszDirectory);
    strcpy(&str[path_len],"/");
    path_len++;

    while ((pEntry = readdir (dir)) != NULL) {
      struct stat statbuffer;


      strcpy(&str[path_len], pEntry->d_name);

      if (stat(str, &statbuffer)) {
        // unable to stat
      } else {

        switch (statbuffer.st_mode & S_IFMT) {

        case S_IFCHR:     // character device

        case S_IFBLK:     // block device

        //case S_IFFIFO:    // FIFO (named pipe)

        case S_IFLNK:     // symbolic link? (Not in POSIX.1-1996.)

        case S_IFSOCK:    // socket? (Not in POSIX.1-1996.)
        case S_IFREG: {
          char *ending;
          ending = strrchr(str, '.');
          if (ending) {
            //if (gszExtention[0]== '\0' || (strcasecmp(ending, gszExtention)==0)) {
            if (gszExtention[0]== '\0' || (strstr(gszExtention,ending))) {
              NumFiles++;
            }
          }
        }
        break;
        case S_IFDIR:

          /* unless ".", "..", ".hidden" or vidix driver dirs */
          if (*pEntry->d_name != '.') { // && strcmp(pEntry->d_name, "vidix"))
            //read_Menu_Tree(bUpdate, str);  //collect_plugins(this, str);
            NumFiles++;
          }
        }
      }
    }
    closedir (dir);
  }
  return   NumFiles;
#endif
}
//-------------------------------------------------------------
int  CFolder::HasDirectorySubDirs(LPCSTR pszDirectory)
{
#ifdef WIN32
  WIN32_FIND_DATA findData;
  HANDLE fileHandle;
  int NumFiles=0;
  char str[MAX_PATH];
  strcpy(str,pszDirectory);
  strcat(str,"/*.*");
  try {
    // Get the first file in the directory
    int  FindF=true;
    int  bFirstRun=true;
    // Loop on entries in the directory
    while (FindF) { // && NumFiles==0)
      if (bFirstRun) {
        bFirstRun = false;
        fileHandle = FindFirstFile(str, &findData);
        FindF  = (fileHandle != INVALID_HANDLE_VALUE);
      } else {
        FindF  = FindNextFile(fileHandle, &findData);
      }
      if (FindF) {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
          if (lstrcmpi(findData.cFileName, TEXT("."))!=0
              && lstrcmpi(findData.cFileName, TEXT(".."))!=0) {
            NumFiles++;
          }
        }
      }
    }
  } catch(...) {
    if (fileHandle != INVALID_HANDLE_VALUE )
      FindClose(fileHandle);
  }
  return NumFiles;

#else
  DIR *dir;
  dir = opendir(pszDirectory);
  int NumFiles = 0;
  if (dir) {
    char str[MAX_PATH];
    struct dirent *pEntry;
    size_t path_len;//, str_size;

    path_len = strlen(pszDirectory);
    strcpy(str, pszDirectory);
    strcpy(&str[path_len],"/");
    path_len++;

    while ((pEntry = readdir (dir)) != NULL) {
      struct stat statbuffer;


      strcpy(&str[path_len], pEntry->d_name);

      if (stat(str, &statbuffer)) {
        // unable to stat
      } else {

        switch (statbuffer.st_mode & S_IFMT) {
        case S_IFDIR:
          /* unless ".", "..", ".hidden" or vidix driver dirs */
          if (*pEntry->d_name != '.') { // && strcmp(pEntry->d_name, "vidix"))
            //read_Menu_Tree(bUpdate, str);  //collect_plugins(this, str);
            NumFiles++;
          }
        }
      }
    }
    closedir (dir);
  }
  return   NumFiles;
#endif
}

//-------------------------------------------------------------

int  CFolder::SelChanged(void)
{
  CTVItem::SelChanged();
  int ret=1;
  BuildDirPath(gScanPath);
  AutoScan=10;

//  msg.Value   = 10;
//  msg.Code    = IDM_SCANFOLDER;
//  msg.receiver= gCmpSheet;
//  //Fl::awake(PostMessage_cb,(void*)&msg);
//  Fl::awake((void*)&msg);

  return 1;
}
//-------------------------------------------------------------
int  CFolder::SelChanging(unsigned int wParam)
{
  int ret=1;
  AutoScan=0;
  if (m_State && (wParam ==VK_DOWN || wParam ==VK_UP || wParam ==VK_LEFT || wParam ==VK_RIGHT )) {
    // Festgemacht :
    if (0) ret = 0;//if (gDesk.m_BMP_out && gDesk.m_BMP_out->IsVisible())
    {
#ifdef M_DIALIST
//      CWindow::SendMessage(gDesk.m_DiaList->GetHWnd(),WM_KEYDOWN,wParam,0);
#endif
      ret = 0;
    }
#ifdef PLAYER
    else if (gDesk.m_pCPlayer &&  gDesk.m_pCPlayer->GetState()>0) {
      CWindow::SendMessage(gDesk.m_pCPlayer->GetHWnd(),WM_KEYDOWN,wParam,0);
      ret = 0;
    } else
#endif
      m_Selected = 0;
  } else {
    m_Selected = 0;
  }
  return ret;
}

//-------------------------------------------------------------
int CFolder::SetState(int iState)
{
  if (m_State==0 && iState) {
    ((CTreeView*)m_pWndControl)->ResetState();
    char File[MAX_PATH];
    if (m_pParent) {
      ((CFolder*)m_pParent)->BuildDirPath(File);
      strcat(File,"/");
      strcat(File,m_pText);
      gBastelUtils.WriteProfileString("Docs","LastPath",File);

      if (m_State) {
        m_pWndControl->m_pCurItem = NULL;
      }
      //if (gDesk.ProcessModalFile(File,GetImage(),0,0))
      //{
      m_State=1;
      //  m_pWndControl->m_pCurItem = (CTVItem*) this;
      //}
    }
  } else {
    m_State=0;
  }
  m_pWndControl->m_pCurItem = NULL;
  return 1;
}

//-------------------------------------------------------------
//-------------------------------------------------------------
//-------------------------------------------------------------

//-------------------------------------------------------------

CFile::CFile(PWNDCONTROLW pWndControl,CTVItem * pParent, DWORD record):CTVItem (pWndControl,pParent,record)
//CFolder(pWndControl,pParent,record)
{
  //  memset(&m_File,0,MAX_PATH);
}
//-------------------------------------------------------------
CFile::~CFile()
{
}
//-------------------------------------------------------------
unsigned int CFile::MakeSortKey()
{
  unsigned int SortKey = 0;
  char str[256];
  memset (&str,0,sizeof(str));
  strcpy(str,GetText());
  for (int i=0; i<6; i++) {
    SortKey = SortKey * 40;
    //if (i< 4)  SortKey = SortKey * 40;                  // the firs four char's
    //else       SortKey = SortKey * 20;                  // light wight
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
int  CFile::ExpandItem(int Hit)
{
  ResetScanStatus();
  return CTVItem::ExpandItem(Hit);
}
//-------------------------------------------------------------
int CFile::SetState(int iState)
{
  if ((m_State&0x7FFF)==0 && iState) {
    ((CTreeView*)m_pWndControl)->ResetState();
    char File[MAX_PATH];
    {
      ((CFolder*)m_pParent)->BuildDirPath(File);
      strcat(File,"/");
      strcat(File,m_pText);
      if (m_State) {
        m_pWndControl->m_pCurItem = NULL;
      }
      m_State=1;

      char *ending;
      ending = strrchr(File, '.');
      if (ending) {
        /*if      ((strncasecmp(ending,".stl",5)==0))  {
          extern DXF3DDisplay * gWrk3DSheet;
          gWrk3DSheet->LoadGaphicData(File);
          gWrk3DSheet->invalidate();
          gWrk3DSheet->redraw();
        } else */{

          //extern int load_file(const char *newfile, int ipos);
          gBastelUtils.WriteProfileString("Docs","LastFile",File);
          /*if (gEditemain )
          {
            //if (load_file(File,-1))
            {
              gStatus1->label(File);
              Fl::check();
              gStatus1->redraw();
            }
          }*/
          //gWrkSheet ->FileOpen(File);
          //AktLayer=0xFFFFFFFF;
          //if (gDXF_Object) gDXF_Object->FileOpen(File,1);
        }
      }
    }


  } else {
    m_State=0x8000;
  }
  return 1;
}
//-------------------------------------------------------------
int  CFile::SelChanging(unsigned int wParam)
{
  int ret=1;
  if (m_State && (wParam ==VK_DOWN || wParam ==VK_UP || wParam ==VK_LEFT || wParam ==VK_RIGHT )) {
    // Festgemacht :
    if (0) ret = 0;//    if (gDesk.m_BMP_out && gDesk.m_BMP_out->IsVisible())
    {
#ifdef M_DIALIST
//      CWindow::SendMessage(gDesk.m_DiaList->GetHWnd(),WM_KEYDOWN,wParam,0);
#endif
      ret = 0;
    }
#ifdef PLAYER
    else if (gDesk.m_pCPlayer&& gDesk.m_pCPlayer->GetState()>0) {
      CWindow::SendMessage(gDesk.m_pCPlayer->GetHWnd(),WM_KEYDOWN,wParam,0);
      ret = 0;
    } else
#endif
      m_Selected = 0;
  } else {
    m_Selected = 0;
  }
  return ret;
}

//-------------------------------------------------------------

//-------------------------------------------------------------
int  CFile::SelChanged(void)
{
  m_Selected = 1;
  m_State |= 0x8000;


  {
    char File[MAX_PATH];
    {
      ((CFolder*)m_pParent)->BuildDirPath(File);
      strcat(File,"/");
      strcat(File,m_pText);
      if (m_State) {
        m_pWndControl->m_pCurItem = NULL;
      }
      char *ending;
      ending = strrchr(File, '.');
      if (ending) {
//       gStopp= true;
        Sleep(10);
//        AktLayer=0xFFFFFFFF;
//        if (gDXF_Object) gDXF_Object->FileOpen(File,0);
      }
    }


  }



  return 1;
}
