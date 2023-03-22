#if !defined(_TREEVIEW_H__368FF20F_89FC_4CAD_89CE_537745BB2F8A__INCLUDED_)
#define _TREEVIEW_H__368FF20F_89FC_4CAD_89CE_537745BB2F8A__INCLUDED_

#include <basteltype.h>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Pack.H>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define ICONSIZE    32
#define TREEITEMHEIGTH 44

#define INSERTCHILDREN          0x80000000
#define INSERT_END              0x7fffffff
#define INSERT_SORT             0x7ffffffE

#define TVHT_VK_LEFT            0x8000
#define TVHT_VK_RIGHT           0x4000

class CTreeView;
class CTreeViewWindowItem;
#define PWNDCONTROLW CTreeView *
class CTVItem  : public Fl_Widget
{
public:
  CTVItem *     m_pParent;
  CTVItem *     m_pChild;
  CTVItem *     m_pNext;
  CTVItem *     m_pPrevoius;
  int           m_NumChild;
  int           m_bExpand;
  int           m_Level;
  unsigned char * m_url;
  unsigned char * m_defApp;

  CTVItem(PWNDCONTROLW pWndControl,CTVItem * pParent, DWORD record);
  virtual ~CTVItem();
  virtual int   DrawHTMLTVItem();
  virtual int   ExpandItem(int Hit);
  virtual int   InsertChildItem(CTVItem **ppFirstChildItem, CTVItem * pItem,int iInsertAfter);
  virtual int   InsertChildItem(CTVItem * pItem,int iInsertAfter);

  void          EnumOpenItems(int * counter);
  void          EnumTVitemChilds(int * pindex,CTVItem * * pResTVItem);
  void          EnumTVitemIndex(CTVItem ** pItem,int * counter);
  void          EnumFindItem(CTVItem ** pItem,LPSTR lpStr);
  void          EnumFindItemPart(CTVItem ** pItem,LPSTR lpStr);
  void          ResetState(CTVItem ** pItem);
  virtual LPSTR CatParentName(LPSTR );
  virtual int   BuildTreePath(LPSTR lpszDir);

  PWNDCONTROLW    m_pWndControl;
  DWORD           m_Item_ID;
  DWORD           m_SortKey;
  unsigned int    m_lParam;
  int             m_Selected;
  virtual void    Invalidate(){m_State |= 0x8000;redraw();};
  virtual int     IsDirty(){return (m_State & 0x8000)==0x8000;};
  virtual void    ClrDirty(){m_State &= ~0x8000;};
  virtual int     SetImage(int iImage);
  virtual int     GetImage(void){return  m_Image;};
  virtual int     SetState(int iState);
  virtual int     GetState(void){return m_State & 0x7FFF;};
  virtual int     SelChanging(unsigned int wParam);
  virtual int     SelChanged(void);
  virtual int     SetText(const char * lpStr);
  virtual char *  GetText(void){return m_pText;};
// HTML things
  char *getTVClassString();
  void  EnumHtmlItems(int files);
  int   NumUpdates();
  void  EnumFindItem(CTVItem ** pItem,int Id);
  void  GetStateUpdates(char ** pnt);
  void  GetTVItemUpdates(char ** pnt);
// Fltk pack things
  virtual void draw();
  void  Expand();
  void  Collapse();
  void  ShrinkChildItems();

protected:
  int     m_Image;
  int     m_State;
  char *  m_pText;
};



class CTreeView  : public Fl_Scroll
{
public:
  int             m_AktIndex;
  int             m_Breite;
  CRITICAL_SECTION m_TV_criticalsection;
  int             m_NumOpenItems;
  int             m_Journalix;
  int             m_rgJournal[200];
  int             m_SetVisibleIndex;
  int             m_Busy;
  CTVItem *       m_TVItem;
  CTVItem *       m_pCurItem;

  CTreeView(int X, int Y, int W, int H,const char *L=0);
  virtual        ~CTreeView();
  virtual int     DrawHTML();
  int             GetNumOpenItems(void);
  CTVItem *       GetTVitem(int index);
  CTVItem *       FindItem(LPSTR lpStr,CTVItem* pStarTItem );
  CTVItem *       FindItemPart(LPSTR lpStr,CTVItem* pStarTItem );
  CTVItem *       FindItem(int ID,CTVItem* pStarTItem );
  int             FindItemIndex(CTVItem* pItem);
  int             InvalidateItem(CTVItem* pItem);
  int             EnsureVisible(CTVItem* pItem);
  int             EnsureVisible(int nIndex);
  void            InvalidateItem(int nIndex);

  virtual int     SetState(int index);
  virtual int     SelChanging(int nIndex,unsigned int wParam);
  virtual int     SelChanged(int nIndex);
  int             InsertChildItem(CTVItem *pItem, int iInsertAfter);
  CTVItem *       ResetState();
  void            Reset(void);

  int HTMLTreeView(int fd);
  int NumUpdates();
  char * GetStateUpdates(char * pnt,int maxsize);
  char * GetTVItemUpdates(char * pnt,int maxsize);
  virtual int     OnClick(int Id,int Image,int pos);

  //CTreeViewWindowItem * AddItem(int pos,char * path);

  Fl_Pack *m_pack;
  //int     m_Damage;
  int CreateTreeView(void);
  CTVItem * GetItem(int pos);
  void Resize();
  int handle(int e);

};

extern CTreeView * gTreeView;


#endif
