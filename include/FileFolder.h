#if !defined(AFX_FILEFOLDER_H__199FAFB3_5CB3_4E89_AD9F_DFF3CD2A8CFE__INCLUDED_)
#define AFX_FILEFOLDER_H__199FAFB3_5CB3_4E89_AD9F_DFF3CD2A8CFE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TreeView.h"

extern char gszHomeFilePath[64];
extern char gszExtention[64];
extern int  SelectTreeItem(CTreeView * tree,char * path);
class CFolder : public CTVItem

{
public:
  CFolder(PWNDCONTROLW pWndControl,CTVItem * pParent, DWORD record);
  virtual ~CFolder();
  virtual int  ExpandItem(int Hit);
  virtual int   SetState(int iState);
  virtual int  SelChanging(unsigned int wParam);
  virtual int  SelChanged(void);
  virtual LPSTR GetParentName(LPSTR );
  int  BuildDirPath(LPSTR lpszDir);
  virtual int  GetDirectoryContents(LPCSTR pszDirectory);
  virtual int  HasDirectoryContents(LPCSTR pszDirectory);
  virtual int  HasDirectorySubDirs(LPCSTR pszDirectory);
};


class CFile : public CTVItem //CFolder
{
public:
  CFile(PWNDCONTROLW pWndControl,CTVItem * pParent, DWORD record);
  virtual ~CFile();
  virtual unsigned int MakeSortKey();
  virtual int ExpandItem(int Hit);
  virtual int  SetState(int iState);
  virtual int SelChanging(unsigned int wParam);
  virtual int  SelChanged(void);
};


#endif
