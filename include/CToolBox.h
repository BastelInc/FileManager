#if !defined(AFX_CTOOLBOX_H__ACE6FB5C_CE3A_4C96_97BB_586024AECFBC__INCLUDED_)
#define AFX_CTOOLBOX_H__ACE6FB5C_CE3A_4C96_97BB_586024AECFBC__INCLUDED_

#include "FltkWindow.h"
#include "FltkDialogBox.h"

void SetStatusText(LPCSTR  pnt,int num,int style);
extern const tDialogCtlItem rg_ToolBox  [];
class CToolBox : public CFltkDialogBox
{
public:
  CToolBox(int X, int Y, int W, int H, const char *L);
  ~CToolBox();

public:
  BOOL SetButtonImage(int nIDDlgItem, int iImage);
  bool virtual ShowLayerButtons(void);
  virtual int WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
  virtual int InitWindow(Fl_Widget  * pParent,int ID,int left,int top,int width, int height,int Style);
  virtual void      Show(void);
};

class CSettings1: public CFltkDialogBox
{
public:
  CSettings1(int X, int Y, int W, int H, const char *L);
  ~CSettings1();
public:
  BOOL SetButtonImage(int nIDDlgItem, int iImage);
  virtual int WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
  virtual int InitWindow(Fl_Widget  * pParent,int ID,int left,int top,int width, int height,int Style);
  virtual void      Show(void);
};
/*
class CSettings2: public CFltkDialogBox
{
public:
  CSettings2(int X, int Y, int W, int H, const char *L);
  ~CSettings2();
public:
  virtual int WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
  virtual int InitWindow(Fl_Widget  * pParent,int ID,int left,int top,int width, int height,int Style);
  virtual void      Show(void);
};
class CSettings3: public CFltkDialogBox
{
public:
  CSettings3(int X, int Y, int W, int H, const char *L);
  ~CSettings3();
public:
  virtual int WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
  virtual int InitWindow(Fl_Widget  * pParent,int ID,int left,int top,int width, int height,int Style);
  virtual void      Show(void);
};
*/
extern void ResetScanStatus(void);
extern void IncrementFileScanStatus(void);
extern void IncrementDirScanStatus(void);



#endif
