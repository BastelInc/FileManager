//  This file is part of the Bastel project. It is distributed under
//  GNU General Public License: https://www.gnu.org/licenses/gpl-3.0
//  Copyright (C) Bastel.Inc - All Rights Reserved

#include "TreeView.h"
#include "Resource.h"
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tree.H>
#include "FileTable.h"

//-------------------------------------------------------------
// CTreeView.cpp: implementation of the CTreeView class.
//-------------------------------------------------------------


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#ifndef WIN32
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>
#endif
#include <malloc.h>
#include "BastelUtils.h"

int sMaxWidth=100;
//extern CTreeView * gFileExplorer;
//CTreeViewWindow * gTreeViewWindow = NULL;
//-------------------------------------------------------------
#define Abstand  4
char m_rgIDS[10000]= {0};
int GetUniqueID()
{
  int i=1;
  while (m_rgIDS[i] && i < (int)sizeof(m_rgIDS))i++;
  if (i < (int)sizeof(m_rgIDS)) {
    m_rgIDS[i]=1;
    return i;
  }
  printf("Error NumID reached\n");
  return 0;
}

//-------------------------------------------------------------
void FreeUniqueID(int Id)
{
  if (Id < (int)sizeof(m_rgIDS)) {
    m_rgIDS[Id]=0;
  }
}

//-------------------------------------------------------------
CTreeView::CTreeView(int X, int Y, int W, int H,const char *L):Fl_Scroll(X,Y,W,H,L)
{
  InitializeCriticalSection(&m_TV_criticalsection);
  m_AktIndex    = -1;
  m_TVItem    = NULL;
  m_Breite    = 500;
  m_pCurItem  = NULL;
  m_Journalix          = 0;
  m_SetVisibleIndex=-1;
  memset(m_rgIDS,0,sizeof(m_rgIDS));
  m_Busy =0;
  sMaxWidth  = fl_width("0")*80;
  if (w() > sMaxWidth) sMaxWidth=w();

  begin();
  type(Fl_Scroll::VERTICAL);  // Slider
  scrollbar_size(22);
  m_pack  = new Fl_Pack(x(),y(),sMaxWidth,h(),NULL);
  m_pack->type(Fl_Pack::VERTICAL);
  m_pack->box(FL_NO_BOX);
  m_pack->spacing(0);
  end();
  set_visible_focus();
  //m_ClassType = eTreeView;
}
//-------------------------------------------------------------
CTreeView::~CTreeView()
{
  //if (m_hTreebmp) DeleteObject(m_hTreebmp);
  //m_hTreebmp  = NULL;
  if (m_TVItem)   free  (m_TVItem);
}
//-------------------------------------------------------------
void  CTreeView::Reset(void)
{
  if (m_TVItem)   free  (m_TVItem);
  m_TVItem=NULL;

  if (m_pack) {
    m_pack->clear();
  }
//  m_ID = 1;
}
//-------------------------------------------------------------
int CTreeView::DrawHTML()
{
  int numLine = GetNumOpenItems();
  int itemIx;

  for (itemIx  = 0; itemIx  <= numLine; itemIx++) {
    {
      CTVItem* pTVItem = GetTVitem(itemIx);
      if (pTVItem) {
        pTVItem->DrawHTMLTVItem();
      }
    }
  }
  return 0;

}

//-------------------------------------------------------------
//-------------------------------------------------------------
// CTVItem Class
//-------------------------------------------------------------
static int testNumItems=0;

CTVItem::CTVItem(PWNDCONTROLW pWndControl,CTVItem * pParent, DWORD record):
  Fl_Widget(0,testNumItems*TREEITEMHEIGTH,sMaxWidth,//pWndControl->w()+52,//pWndControl->scrollbar_size(),
            pParent?(pParent->m_bExpand? TREEITEMHEIGTH:0):TREEITEMHEIGTH,NULL)
  /*
  ,
  m_pWndControl(pWndControl),
  m_pParent(pParent),
  m_pChild(NULL),
  m_pNext(NULL),
  m_pPrevoius(0),
  m_NumChild(0),
  m_bExpand(0),
  m_Level(0),
  m_Item_ID(record),
  m_SortKey(0),
  m_lParam(0),
  m_Selected(0),
  m_Image(0),
  m_State(0),
  m_pText(NULL)
  */
{

  m_pWndControl = pWndControl;
  m_pParent =    pParent;
  m_pChild=NULL;
  m_pNext=NULL;
  m_pPrevoius=NULL;
  m_url    = NULL;
  m_defApp = NULL;
  m_NumChild=0;
  m_bExpand = 0;
  m_Level  = 0;
  m_Item_ID = GetUniqueID();
  m_SortKey = 0;
  m_lParam  = 0;
  m_Selected = 0;
  m_Image = 0;
  m_State = 0;
  m_pText=NULL;

  if (m_pParent) {
    m_pParent->m_NumChild++;
    m_Level = m_pParent->m_Level+1;
  }
  {
//  fl_font(FONTFACE,FONTSIZE);
    deactivate();
    labelfont (fl_font());
    labelsize (fl_size());
    box(FL_NO_BOX);
    align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  }
  testNumItems++;
  if (m_Item_ID && m_pWndControl->m_Journalix <200) {
    EnterCriticalSection(&m_pWndControl->m_TV_criticalsection);
    m_pWndControl->m_rgJournal[m_pWndControl->m_Journalix] = m_Item_ID;
    m_pWndControl->m_Journalix++;
#ifdef SHOWDEBUG
    printf("insert Item%d\n",m_Item_ID );
#endif
    LeaveCriticalSection(&m_pWndControl->m_TV_criticalsection);
  }
}

//-------------------------------------------------------------
CTVItem::~CTVItem()
{
  if (m_pWndControl->m_pCurItem== this)   m_pWndControl->m_pCurItem= NULL;

  m_pWndControl->m_pack->remove(this);

  if (m_pParent) {
    if ((m_pParent->m_NumChild & 0x7FFFFFFF) > 0) {
      m_pParent->m_NumChild--;
      if (m_pParent->m_NumChild==0)
        m_pParent->m_NumChild = INSERTCHILDREN;
    }
  }
  if (m_pChild) delete(m_pChild);
  m_pChild = NULL;
  if (m_pNext)  delete (m_pNext);
  m_pNext  = NULL;
  if (m_pText) free(m_pText);
  m_pText=NULL;
  if (m_url) free(m_url);
  m_url    = NULL;
  if (m_defApp)  free(m_defApp);
  m_defApp = NULL;
  testNumItems--;
  if (m_Item_ID && m_pWndControl->m_Journalix <200) {
    EnterCriticalSection(&m_pWndControl->m_TV_criticalsection);
    m_pWndControl->m_rgJournal[m_pWndControl->m_Journalix] = -(int)m_Item_ID;
    m_pWndControl->m_Journalix++;
#ifdef SHOWDEBUG
    printf("delete Item%d\n",m_Item_ID );
#endif
    FreeUniqueID(m_Item_ID );

    LeaveCriticalSection(&m_pWndControl->m_TV_criticalsection);
  }
}
//-------------------------------------------------------------
void CTVItem::EnumOpenItems(int * counter)
{
  CTVItem* pTVItem1 = m_pChild;
  while(pTVItem1) {
    (*counter)++;
    if (pTVItem1->m_pChild && pTVItem1->m_bExpand) {
      pTVItem1->EnumOpenItems(counter);
    }
    pTVItem1 = pTVItem1->m_pNext;
  }
}
//-------------------------------------------------------------
int CTreeView::GetNumOpenItems(void)
{
  m_NumOpenItems=0;
  CTVItem* pTVItem1 = m_TVItem;

  while(pTVItem1) {
    m_NumOpenItems++;
    if (pTVItem1->m_pChild && pTVItem1->m_bExpand) {
      pTVItem1->EnumOpenItems(&m_NumOpenItems);
    }
    pTVItem1 = pTVItem1->m_pNext;
  }
  return m_NumOpenItems;
}

//-------------------------------------------------------------
void CTVItem::EnumTVitemChilds(int * pindex,CTVItem ** pResTVItem)
{
  CTVItem* pTVItem1 = m_pChild;
  while((*pindex)>0 &&  pTVItem1) {
    (*pindex)--;
    if ((*pindex)==0) {
      *pResTVItem = pTVItem1;
      break;
    }
    if (pTVItem1->m_pChild && pTVItem1->m_bExpand) {
      pTVItem1->EnumTVitemChilds(pindex,pResTVItem);
    }
    pTVItem1 = pTVItem1->m_pNext;
  }
}
//-------------------------------------------------------------
CTVItem * CTreeView::GetTVitem(int index)
{
  CTVItem* pTVItem1 = m_TVItem;

  CTVItem* pResTVItem = NULL;
  index++;

  while(index>0 && pTVItem1) {
    index--;
    if (index==0) {
      pResTVItem = pTVItem1;
      break;
    }
    if (pTVItem1->m_pChild && pTVItem1->m_bExpand) {
      pTVItem1->EnumTVitemChilds(&index,&pResTVItem);
    }
    pTVItem1 = pTVItem1->m_pNext;
  }
  return pResTVItem;
}
//-------------------------------------------------------------
void CTVItem::EnumTVitemIndex(CTVItem ** pItem,int * counter)
{
  CTVItem* pTVItem1 = m_pChild;
  while((*pItem) && pTVItem1) {
    (*counter)++;
    if (*pItem == pTVItem1) {
      *pItem = NULL;    // Found
      break;
    }
    if (pTVItem1->m_pChild && pTVItem1->m_bExpand) {
      pTVItem1->EnumTVitemIndex(pItem,counter);
    }

    pTVItem1 = pTVItem1->m_pNext;
  }
}

//-------------------------------------------------------------
LPSTR CTVItem::CatParentName(LPSTR lpszDir)
{
  CTVItem * pTVItem = (CTVItem*)this->m_pParent;
  LPSTR pnt = GetText();

  if (pTVItem) {
    pTVItem->CatParentName(lpszDir);
  }

  if (pnt) {
    strcat(lpszDir,"/");
    strcat(lpszDir,pnt);
  }
  return 0;
}
//-------------------------------------------------------------
int CTVItem::BuildTreePath(LPSTR lpszDir)
{
  strcpy(lpszDir, "");
  CatParentName(lpszDir);
  LPSTR pnt = GetText();
  return pnt != NULL;
}
//-------------------------------------------------------------
void CTVItem::ResetState(CTVItem ** pItem)
{
  return ;
  CTVItem* pTVItem1 = m_pChild;
  while((*pItem==NULL) && pTVItem1) {
    if (pTVItem1->GetState()) {
      *pItem = pTVItem1;    // Item Found
      pTVItem1->SetState(0);
    }
    if (pTVItem1->m_pChild) {
      ((CTVItem*) pTVItem1)->ResetState(pItem);
    }
    pTVItem1 = pTVItem1->m_pNext;
  }
}
//-------------------------------------------------------------
CTVItem * CTreeView::ResetState()
{
  CTVItem* pTVItem1 = (CTVItem* ) m_TVItem;

  CTVItem* pResTVItem = NULL;

  while(pResTVItem ==NULL && pTVItem1) {
    if (pTVItem1->GetState()) {
      pResTVItem = pTVItem1;
      pTVItem1->SetState(0);
    }
    if (pTVItem1->m_pChild) {
      pTVItem1->ResetState(&pResTVItem);
    }
    pTVItem1 = (CTVItem* )pTVItem1->m_pNext;
  }
  if (pResTVItem) InvalidateItem(pResTVItem);
  return pResTVItem;

}
//-------------------------------------------------------------
void CTVItem::EnumFindItem(CTVItem ** pItem,LPSTR lpStr)
{
  CTVItem* pTVItem1 = m_pChild;
  while((*pItem==NULL) && pTVItem1) {
    LPSTR pnt = pTVItem1->GetText();
    if (pnt) {
      if (strcmp(pnt,lpStr)==0) {
        *pItem = pTVItem1;    // Item Found
        break;
      }
    }
    if (pTVItem1->m_pChild) {
      ((CTVItem*) pTVItem1)->EnumFindItem(pItem,lpStr);

      if (*pItem) pTVItem1->m_bExpand = 1;
    }
    pTVItem1 = pTVItem1->m_pNext;
  }
}
//-------------------------------------------------------------
CTVItem* CTreeView::FindItem(LPSTR lpStr,CTVItem* pStarTItem )
{
  CTVItem* pTVItem1 = (CTVItem* ) m_TVItem;
  if (pStarTItem ) pTVItem1 = pStarTItem;
  CTVItem* pResTVItem = NULL;

  while(pResTVItem ==NULL && pTVItem1) {
    LPSTR pnt = pTVItem1->GetText();
    if (pnt) {
      if (strcmp(pnt,lpStr)==0) {
        pResTVItem = pTVItem1;
        break;
      }
    }
    if (pTVItem1->m_pChild) {
      ((CTVItem*) pTVItem1)->EnumFindItem(&pResTVItem,lpStr);

      if (pResTVItem ) pTVItem1->m_bExpand = 1;
    }
    pTVItem1 = (CTVItem* )pTVItem1->m_pNext;
  }
  return pResTVItem;
}

//-------------------------------------------------------------
void CTVItem::EnumFindItemPart(CTVItem ** pItem,LPSTR lpStr)
{
  CTVItem* pTVItem1 = m_pChild;
  int len = strlen(lpStr);
  while((*pItem==NULL) && pTVItem1) {
    LPSTR pnt = pTVItem1->GetText();
    if (pnt) {
      if (strncasecmp(pnt,lpStr,len)==0) {
        *pItem = pTVItem1;    // Item Found
        break;
      }
    }
    if (pTVItem1->m_pChild) {
      ((CTVItem*) pTVItem1)->EnumFindItemPart(pItem,lpStr);

      if (*pItem) pTVItem1->m_bExpand = 1;
    }
    pTVItem1 = pTVItem1->m_pNext;
  }
}
//-------------------------------------------------------------
CTVItem* CTreeView::FindItemPart(LPSTR lpStr,CTVItem* pStarTItem )
{
  CTVItem* pTVItem1 = (CTVItem* ) m_TVItem;
  if (pStarTItem ) pTVItem1 = pStarTItem;
  int len = strlen(lpStr);
  CTVItem* pResTVItem = NULL;

  while(pResTVItem ==NULL && pTVItem1) {
    LPSTR pnt = pTVItem1->GetText();
    if (pnt) {
      if (strncasecmp(pnt,lpStr,len)==0) {
        pResTVItem = pTVItem1;
        break;
      }
    }
    if (pTVItem1->m_pChild) {
      ((CTVItem*) pTVItem1)->EnumFindItemPart(&pResTVItem,lpStr);

      if (pResTVItem ) pTVItem1->m_bExpand = 1;
    }
    pTVItem1 = (CTVItem* )pTVItem1->m_pNext;
  }
  return pResTVItem;
}

//-------------------------------------------------------------
int CTreeView::FindItemIndex(CTVItem* pItem)
{
  CTVItem* pTVItem1 = m_TVItem;
  int  index = -1; // Enum the visible (expanded) Tree items
  while(pItem && pTVItem1) {
    index++;
    if (pItem == pTVItem1) {
      pItem = NULL;
      return index;
    }
    if (pTVItem1->m_pChild && pTVItem1->m_bExpand) {
      pTVItem1->EnumTVitemIndex(&pItem,&index);
    }
    pTVItem1 = pTVItem1->m_pNext;
  }
  return (pItem == NULL)? index: -1;
}
//-------------------------------------------------------------
int CTreeView::InvalidateItem(CTVItem* pItem)
{
  int index = FindItemIndex(pItem);
  if (index >=0) InvalidateItem(index);
  return index;
}
//-------------------------------------------------------------
int CTreeView::EnsureVisible(CTVItem* pItem)
{
  GetNumOpenItems();
  int index = FindItemIndex(pItem);
  if (index >=0) return EnsureVisible(index);
  else           return 0;
}
//-------------------------------------------------------------
int CTreeView::EnsureVisible(int nIndex)
{
  m_SetVisibleIndex = nIndex;
  if (m_pack) {
    if (nIndex>=0 && nIndex < m_pack->children()) {
      CTVItem *  p = GetTVitem(nIndex);
      //CTVItem *  p = (CTVItem  * )m_pack->child(nIndex);
      if (p) {
        int y  = p->y();
        y  = y - this->y();
        int y2 = y+p->h();
        int s = scrollbar.value();
        int h2 = s + this->h();
        if (y < 0) {
          scroll_to(0,s+y-7);
          scrollbar.value();
          m_pack->redraw();
          redraw();
        }else if (y2 > this->h()) {
          scroll_to(0,s+y2-this->h()+7);
          m_pack->redraw();
          redraw();
        }
      }
    }
  }
  InvalidateItem(nIndex);
  return 0;
}
//-------------------------------------------------------------
void CTreeView::InvalidateItem(int nIndex)
{
  CTVItem * pitem = GetTVitem(nIndex);
  if (pitem) pitem->Invalidate();
}
//-------------------------------------------------------------
int CTreeView::SelChanging(int nIndex,unsigned int wParam)
{
  int ret=1;
  CTVItem* pTVItem = GetTVitem(nIndex);
  if (pTVItem) {
    ret = pTVItem->SelChanging(wParam);
  }
  InvalidateItem(nIndex);
  return ret;
}
//-------------------------------------------------------------
int CTreeView::SelChanged(int nIndex)
{
  CTVItem* pTVItem = GetTVitem(nIndex);
  if (pTVItem) {
    m_AktIndex=nIndex;
    pTVItem->SelChanged();
  }
  InvalidateItem(nIndex);
  return 1;
}
//-------------------------------------------------------------

int  CTVItem::SelChanging(unsigned int wParam)
{
  int ret=1;
  m_Selected = 0;
  m_State |= 0x8000;
  Invalidate();
  return ret;
}
//-------------------------------------------------------------
int  CTVItem::SelChanged(void)
{
  m_Selected = 1;
  m_State |= 0x8000;
  return 1;
}

//-------------------------------------------------------------
int CTVItem::SetImage(int iImage)
{
  return m_Image=iImage;
}
//-------------------------------------------------------------

int CTVItem::SetState(int iState)
{
  if (m_State==0 && iState) {
    if (m_NumChild  && m_pChild) {
      // wir haben Children : oeffnen oder loeschen!
      ExpandItem(m_bExpand?VK_LEFT:VK_RIGHT);
    } else if ((m_NumChild & 0x7FFFFFFF)==0) {
      // neue Children's erzeugen
      ExpandItem(VK_RIGHT);
    }
    m_State=0;
  } else m_State=0;
  m_State  |= 0x8000;
  return 1;
}

//-------------------------------------------------------------
int  CTVItem::SetText(LPCSTR  lpStr)
{
  int TextSize;
  if (m_pText) TextSize = malloc_usable_size(m_pText);
  else         TextSize = 0;

  int slen = strlen(lpStr)+1;
  if (slen > TextSize) {
    TextSize = ((slen+3) /4)*4;
    if (m_pText) m_pText = (LPSTR)realloc(m_pText,TextSize);
    else         m_pText = (LPSTR)malloc(TextSize);
  }
  if (m_pText) strcpy(m_pText,lpStr);
  {
    label(m_pText);
    int message_w = 0;
    int message_h = 0;
    fl_measure(label(), message_w, message_h);
    message_w += 10;

    int ident = ICONSIZE+5 +ICONSIZE+ m_Level*ICONSIZE;
    message_w  += ident;


    message_h += 10;
    if (message_w < 80) message_w = 80;
    if (message_h < TREEITEMHEIGTH) message_w = TREEITEMHEIGTH;
    resize(x(),y(),message_w,m_pParent?(m_pParent->m_bExpand?message_h:0):TREEITEMHEIGTH);
  }


  m_State  |= 0x8000;
  return slen;
}

//-------------------------------------------------------------
int CTreeView::InsertChildItem(CTVItem *pItem, int iInsertAfter)
{
  int Position=0;
  if (m_TVItem==NULL) {
    // the first item is the root
    m_TVItem = pItem;
    pItem->m_pNext    =NULL;
    pItem->m_pPrevoius=NULL;
    m_pack->insert(*pItem,0);
    return Position;
  } else return m_TVItem->InsertChildItem(&m_TVItem,pItem,iInsertAfter);
}
//-------------------------------------------------------------
int  CTVItem::InsertChildItem(CTVItem * pItem,int iInsertAfter)
{
  return InsertChildItem(&m_pChild,pItem,iInsertAfter);
}
//-------------------------------------------------------------
int CTVItem::InsertChildItem(CTVItem **ppFirstChildItem, CTVItem *pItem, int iInsertAfter)
{
  CTVItem * pTVItem1 = *ppFirstChildItem;
  int Position=0;
  if (pTVItem1==NULL) {
    *ppFirstChildItem = pItem;
    pItem->m_pNext    =NULL;
    pItem->m_pPrevoius=NULL;
    int ix = m_pWndControl->m_pack->find(this);
    m_pWndControl->m_pack->insert(*pItem,ix+1);
    return Position;
  } else {
    if (iInsertAfter == INSERT_SORT) {
      CTVItem * pLastTestTtem = pTVItem1;
      while (pTVItem1) {
        if (pItem->m_SortKey < pTVItem1->m_SortKey) {
          pTVItem1 = pLastTestTtem;
          break;
        } else {
          pLastTestTtem = pTVItem1;
          Position++;
          if (pTVItem1->m_pNext == NULL) break;
          else   pTVItem1 = pTVItem1->m_pNext;
        }
      }
    } else {
      if (iInsertAfter) Position++;
      while(pTVItem1 && pTVItem1->m_pNext && iInsertAfter) {
        iInsertAfter--;
        Position++;
        pTVItem1 = pTVItem1->m_pNext;
      }
    }
    int ix = m_pWndControl->m_pack->find(this);

    if (pTVItem1) {
      if (Position==0) {
        *ppFirstChildItem = pItem;
        pItem->m_pNext        = pTVItem1;
        pTVItem1->m_pPrevoius = pItem;
      } else {
        CTVItem * pexNext  = pTVItem1->m_pNext;
        // insert the item after pTVItem1 and before pTVItem1->m_pNext ;
        // or append the item after pTVItem1
        pTVItem1->m_pNext  = pItem;
        if (pexNext) {
          pexNext->m_pPrevoius=pItem;
        }
        pItem->m_pPrevoius = pTVItem1;
        pItem->m_pNext     = pexNext;
      }
      m_pWndControl->m_pack->insert(*pItem,ix+1+Position);
    }
    return Position;
  }
  return -1;
}
//-------------------------------------------------------------
void CTVItem::Expand()
{
  m_bExpand = 1;
  CTVItem * p = m_pChild;
  while (p) {
    p->resize(p->x(),p->y(),p->w(),TREEITEMHEIGTH);
    p = p->m_pNext;
  }
}
//-------------------------------------------------------------
void CTVItem::ShrinkChildItems()
{
  m_bExpand = 0;
  CTVItem * p = m_pNext;
  while (p) {
    if (p->m_pChild) {
      p->m_pChild->ShrinkChildItems();
    }
    p->resize(p->x(),p->y(),p->w(),0);
    p = p->m_pNext;
  }
  resize(x(),y(),w(),0);
}
//-------------------------------------------------------------
void CTVItem::Collapse()
{
  m_bExpand = 0;
  CTVItem * p = m_pChild;
  int h = m_pWndControl->m_pack->h();
  while (p) {
    if (p->m_pChild) {
      p->m_pChild->ShrinkChildItems();
    }
    p->resize(p->x(),p->y(),p->w(),0);
    p->m_bExpand =0;
    p = p->m_pNext;
  }
  m_pWndControl->EnsureVisible(m_pWndControl->m_AktIndex);
}
//-------------------------------------------------------------
int CTVItem::ExpandItem(int Hit)
{
  //ResetScanStatus();
  m_pWndControl->m_Busy++;
  if (m_pChild) {
    if (Hit==VK_RIGHT) {
      if (m_NumChild && m_pChild) {
        Expand();
      }
    } else if (Hit==VK_LEFT) {
      if (m_NumChild && m_pChild && m_bExpand) {
        Collapse();
        delete (m_pChild);
        m_pChild=NULL;
      }
    }
    m_State  |= 0x8000;

  }
  m_pWndControl->Resize();
  m_pWndControl->m_Busy--;
  return 0;
}
//-------------------------------------------------------------
void CTreeView::Resize()
{
  //m_pWndControl->m_pack->redraw();
  //m_pWndControl->redraw();
  m_pack->redraw();
  redraw();
}

//-------------------------------------------------------------
int CTVItem::DrawHTMLTVItem()
{
  if (m_Selected) {
  } else {
  }

  // Item Symbol
  //int image = m_Image;
//    if (m_Image == imFolderClose || m_Image == imFolderOpen)
  {
    // show teh folder State
    //image = m_bExpand ? imFolderOpen:imFolderClose;
  }
  return 0;
}
//-------------------------------------------------------------
int CTreeView::SetState(int index)
{
  CTVItem* pTVItem = GetTVitem(index);
  CTVItem* pTVItem2;
  if (pTVItem==0)return 0;
  if (pTVItem->m_pParent) {
    pTVItem2 = pTVItem->m_pParent->m_pChild;
  } else pTVItem2 = m_TVItem;
  if (pTVItem) {
    while (pTVItem2) {
      if (pTVItem2->GetState()) {
        if (pTVItem != pTVItem2) {
          if (pTVItem2->SetState(0)==0) return 0;       // Nicht erlaubt
          InvalidateItem(pTVItem2);
        }
      }
      pTVItem2 = pTVItem2->m_pNext;
    }
    pTVItem->SetState(1);
  }
  return 1;
}

//-------------------------------------------------------------

//--------------------------------------------------------------
int CTVItem::NumUpdates()
{
  int numMod=0;
  CTVItem* pTVItem1 = m_pChild;
  while(pTVItem1) {
    if (pTVItem1->m_pChild) {
      numMod += pTVItem1->NumUpdates();
    }
    if (pTVItem1->IsDirty()) numMod++;
    pTVItem1 = pTVItem1->m_pNext;
  }
  return numMod ;
}
//-------------------------------------------------------------
int CTreeView::NumUpdates()
{
  int numMod=0;
  CTVItem* pTVItem1 = m_TVItem;
  while(pTVItem1) {
    if (pTVItem1->m_pChild) {
      numMod += pTVItem1->NumUpdates();
    }
    if (pTVItem1->IsDirty()) numMod++;
    pTVItem1 = pTVItem1->m_pNext;
  }
  if (m_SetVisibleIndex>=0) numMod++;
  return numMod;
}
//--------------------------------------------------------------
char * gMaxpnt=NULL;
static char szclass[60];
char * CTVItem::getTVClassString()
{
  int Image = GetImage();
  sprintf(szclass,"TVII%02d%s%s",Image,m_bExpand?" open":"",m_Selected?" selected":"");
  return szclass;
}
//--------------------------------------------------------------
void CTVItem::GetStateUpdates(char ** pnt)
{
  CTVItem* pTVItem1 = m_pChild;
  while(pTVItem1) {
    if (pTVItem1->m_pChild) {
      pTVItem1->GetStateUpdates(pnt);
    }
    if (pTVItem1->IsDirty()) {
      if (*pnt <= gMaxpnt) {
        pTVItem1->ClrDirty();
        //printf("TVItem%d=State%d!Image%d&\n",pTVItem1->m_Item_ID,pTVItem1->m_State,pTVItem1->m_Image);
        *pnt += sprintf(*pnt,"TVItem%d=State!%d!%s&",pTVItem1->m_Item_ID,pTVItem1->m_State,pTVItem1->getTVClassString());
      }
    }
    pTVItem1 = pTVItem1->m_pNext;
  }
}
//--------------------------------------------------------------
char * CTreeView::GetStateUpdates(char * pnt,int maxsize)
{
  CTVItem* pTVItem1 = m_TVItem;
  gMaxpnt = pnt + maxsize-80;
  while(pTVItem1) {
#ifdef SHOWDEBUG
    char * pdebug = pnt;
    * pdebug = '\0';
#endif
    if (pTVItem1->m_pChild) {
      pTVItem1->GetStateUpdates(&pnt);
    }
    if (pTVItem1->IsDirty()) {
      if (pnt <= gMaxpnt) {
        pTVItem1->ClrDirty();
        //printf("TVItem%d=State%d!Image%d&",pTVItem1->m_Item_ID,pTVItem1->GetState(),pTVItem1->GetImage());
        pnt += sprintf(pnt,"TVItem%d=State!%d!%s&",pTVItem1->m_Item_ID,pTVItem1->GetState(),pTVItem1->getTVClassString());
      }
    }
    pTVItem1 = pTVItem1->m_pNext;
#ifdef SHOWDEBUG
    printf("%s\n",pdebug);
#endif
  }
  if (m_SetVisibleIndex >=0) {
    static int del=1;
    if (del>0) del --;
    else {
      pTVItem1 =  FindItem(m_SetVisibleIndex,NULL);
      if (pTVItem1) {
        pnt += sprintf(pnt,"TVItem%d=ScrollPos!%d&",pTVItem1->m_Item_ID,scrollbar.value());//m_SetVisibleIndex);
      }
      m_SetVisibleIndex=-1;
      del = 1;
    }

  }
  return pnt;
}
//--------------------------------------------------------------
void CTVItem::EnumFindItem(CTVItem ** pItem,int Id)
{
  CTVItem* pTVItem1 = m_pChild;
  while((*pItem==NULL) && pTVItem1) {
    if ((unsigned)Id == pTVItem1->m_Item_ID) {
      *pItem = pTVItem1;    // Item Found
      break;
    }
    if (pTVItem1->m_pChild) {
      ((CTVItem*) pTVItem1)->EnumFindItem(pItem,Id);
    }
    pTVItem1 = pTVItem1->m_pNext;
  }
}
//--------------------------------------------------------------
CTVItem * CTreeView::FindItem(int ID,CTVItem* pStarTItem )
{
  CTVItem* pTVItem1 = (CTVItem* ) m_TVItem;
  if (pStarTItem ) pTVItem1 = pStarTItem;

  CTVItem* pResTVItem = NULL;

  while(pResTVItem ==NULL && pTVItem1) {
    if ((unsigned)ID == pTVItem1->m_Item_ID) {
      pResTVItem = pTVItem1;
      break;
    }
    if (pTVItem1->m_pChild) {
      ((CTVItem*) pTVItem1)->EnumFindItem(&pResTVItem,ID);
    }
    pTVItem1 = (CTVItem* )pTVItem1->m_pNext;
  }
  return pResTVItem;
}
//--------------------------------------------------------------
#ifdef HTMLTREE
char * CTreeView::GetTVItemUpdates(char * pnt,int maxsize)
{
  if  (m_Journalix ) {
    EnterCriticalSection(&m_TV_criticalsection);
    gMaxpnt = pnt + maxsize-120;
    for (int ix=0; ix < m_Journalix; ix++) {
#ifdef SHOWDEBUG
      char * pdebug = pnt;
#endif
      int id = m_rgJournal[ix];
      if (id < 0) {
        if (pnt <= gMaxpnt) pnt += sprintf(pnt,"TVItem%d=delete!%d&",-id,0);
#ifdef SHOWDEBUG
        printf("%s\n",pdebug);
#endif
        m_Journalix--;
        if (m_Journalix>ix) memmove(&m_rgJournal[ix],&m_rgJournal[ix+1],(m_Journalix-ix)*sizeof(int));
      }
    }
    // Neue gemeass Tree struktur einfuegen, (Sortierung brachte sie ducheinander).
    CTVItem* pTVItem = (CTVItem* ) m_TVItem;
    while(pTVItem && m_Journalix && (pnt <= gMaxpnt) ) {
#ifdef SHOWDEBUG
      char * pdebug = pnt;
#endif
      int id   = pTVItem->m_Item_ID;
      for (int ix=0; ix < m_Journalix; ix++) {
        if (m_rgJournal[ix] == id) {
          char str[260];
          extern  void StrEncode( char* to, int tosize, const unsigned char* from );
          StrEncode(str,sizeof(str),(unsigned char*)pTVItem->GetText());
          if (pTVItem->m_pPrevoius) {
            pnt += sprintf(pnt,"TVItem%d=TVIinsertItem!TVItem%d!%s!%s!%d&",pTVItem->m_pPrevoius->m_Item_ID,id,str,pTVItem->getTVClassString(),(pTVItem->m_NumChild&INSERTCHILDREN)?1:pTVItem->m_NumChild);
          } else if (pTVItem->m_pParent) {
            // child
            pnt += sprintf(pnt,"TVItem%d=State!%d!%s!&",pTVItem->m_pParent->m_Item_ID,pTVItem->m_pParent->GetState(),pTVItem->m_pParent->getTVClassString());
            pnt += sprintf(pnt,"TVItem%d=TVIinsertChild!TVItem%d!%s!%s!%d&",pTVItem->m_pParent->m_Item_ID,id,str,pTVItem->getTVClassString(),(pTVItem->m_NumChild&INSERTCHILDREN)?1:pTVItem->m_NumChild);
          }
#ifdef SHOWDEBUG
          printf("%s\n",pdebug);
#endif
          m_Journalix--;
          if (m_Journalix>ix) memmove(&m_rgJournal[ix],&m_rgJournal[ix+1],(m_Journalix-ix)*sizeof(int));
        }
      }
      if (pTVItem->m_pChild) {
        pTVItem = pTVItem->m_pChild;
      } else if (pTVItem->m_pNext) {
        pTVItem = pTVItem->m_pNext;
      } else if (pTVItem->m_pParent) {
        pTVItem = pTVItem->m_pParent;
        if (pTVItem) pTVItem = pTVItem->m_pNext;
      } else pTVItem = NULL;
    }
//  m_Journalix=0;
    LeaveCriticalSection(&m_TV_criticalsection);
  }
  return pnt;
}
#endif
//--------------------------------------------------------------
int CTreeView::OnClick(int Id,int DoubleClick,int pos)
{
  CTVItem *  pTVItem = FindItem(Id,m_TVItem);
  if (pTVItem) {
    int isSel=pTVItem->m_Selected;
    if (SelChanging(m_AktIndex,0)) {
      m_AktIndex = FindItemIndex(pTVItem );
      if (pTVItem->m_NumChild ) {
        if (DoubleClick==0) {
          int ident = (pTVItem->m_Level+2)*ICONSIZE;

          if (isSel|| pos <= ident) {
            if (pTVItem->m_bExpand==0) {
              pTVItem->ExpandItem(VK_RIGHT);
            } else {
              pTVItem->ExpandItem(VK_LEFT);
            }
          } else {
            SelChanged(m_AktIndex);
          }
        }
        pTVItem->Invalidate();

      } else {
        if (DoubleClick) SetState(m_AktIndex);
        SelChanged(m_AktIndex);
      }
    }
  }
  return 1;
}

//-------------------------------------------------------------

/*
CTreeViewWindowItem * CTreeView::AddItem(int pos,char * path) {
  CTreeViewWindowItem * item;
  item = (CTVItem * )new CTVItem(0,pos*25, 25, 25, path);
  fl_font(FONTFACE,FONTSIZE);
  item ->labelfont (fl_font());
item ->labelsize (fl_size());
item ->box(FL_NO_BOX);
item->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
int	message_w, message_h;
message_w = message_h = 0;
fl_measure(item->label(), message_w, message_h);
message_w += 10;
message_h += 10;
if (message_w < 80) message_w = 80;
item->resize(0,pos*25,message_w,message_h );
return item;
}


void CTVItem::EnumWindowItems(int files)
{
CTVItem* pTVItem1 = m_pChild;
char extras[200];
memset (extras,' ',sizeof(extras));
if (m_Level>10) extras[0]= '\0';
else extras[2+m_Level*2]= '\0';
//sprintf("%s",extras);

while(pTVItem1)
{
//InSprintf("%s%s",extras,pTVItem1->GetText());
CTreeViewWindowItem * item;
item = gpTree->AddItem(files,strdup(pTVItem1->GetText()));
item->m_Level = m_Level;
if (pTVItem1->m_NumChild)
{
  pTVItem1->EnumWindowItems(1);
}
pTVItem1 = pTVItem1->m_pNext;
}
}
//--------------------------------------------------------------


CTreeViewWindowItem::CTreeViewWindowItem(int X, int Y, int W, int H, const char *L):Fl_Widget(X,Y,W,H,L){
  m_Level=0;
}
//--------------------------------------------------------------
CTreeViewWindowItem::~CTreeViewWindowItem(){
  const char * p = label();
  if (p) free((void *)p);
}
*/


void CTVItem::draw()
{
  int H = h();
  if (H<3 || type() == FL_HIDDEN_BUTTON) return;
//  fl_font(FONTFACE,FONTSIZE);
//  labelfont (fl_font());
//  labelsize (fl_size());
  fl_font(FL_HELVETICA, 14);
  Fl_Color fg = m_pWndControl->active()?FL_FOREGROUND_COLOR:FL_DARK1;
  Fl_Color bg = m_pWndControl->active()?FL_BACKGROUND2_COLOR:FL_LIGHT1;
  int ident = 5+/*ICONSIZE+ */m_Level*ICONSIZE;
  int X = x()+ident;
  int Y = y();
  int W = w()-ident;

  int image=0;
  int left = X;
  if (0) {
    if (m_NumChild ) {
      if (m_pNext )  image  = m_bExpand? imOpenCont:imCloseCont;
      else           image  = m_bExpand? imOpenLast:imCloseLast;
      R->TreeViewPics.draw(left,  Y-1,32,44,(image)*32,0);
    } else {
      image = imVert;
    }

    {
      int level = m_Level;
      CTVItem * pItem = m_pParent;
      while  (pItem && level) {
        left-= ICONSIZE;
        level--;
        if (pItem->m_pNext) {
          R->TreeViewPics.draw(left,  Y-1,32,44,0,0);
        }
        pItem  = pItem ->m_pParent;
      }
    }


    X += ICONSIZE;
  }

  if (m_Image == imFolderClose) {
    R->TreeViewPics.draw(X+0,  Y-1,32,44,(imFolderClose+(m_bExpand?1:0)) *32,0);
  } else {
    if (m_Image>20) {
      R->ButtonImage_List[m_Image-20]->draw(X+0-6,  Y-1,38,44);
    } else {
      R->TreeViewPics.draw(X+0,  Y-1,32,44,(m_Image %20)*32,0);
    }
  }
  X += ICONSIZE+5;
  if (1) {
    if (m_Selected) {
      bg = FL_SELECTION_COLOR;
      fg = FL_BACKGROUND2_COLOR;
      fl_draw_box(FL_THIN_UP_BOX,X,Y,W,H,bg);
    } else {
      fl_color(bg);
      fl_rectf(X,Y,W,H);
    }
  }

  fl_color(fg);
  if ( label() ) fl_draw(label(), X,Y,W,H, FL_ALIGN_LEFT);
  if (Fl::focus() == this) draw_focus();
}

//--------------------------------------------------------------
static int scroll_start  = 0;
static int scroll_startx = 0;
static int scroll_min  = 0;
static int scroll_max  = 0;
static int start_fingerx = 0;
static int start_fingery = 0;
static unsigned int startTickCount;
static int kpix_per_ms;
static int fingermovecnt = 0;
//-------------------------------------------------------------
void TreeViewTimerCb(void *data)
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
    Fl::repeat_timeout(0.005, TreeViewTimerCb, data);
  }
}

//-------------------------------------------------------------
CTVItem * CTreeView::GetItem(int pos)
{
  if (m_pack) {
    pos += this->y();
    for (int i = 0; i < m_pack->children(); i++) {
      CTVItem *  p = (CTVItem * )m_pack->child(i);;
      if (p->y() <= pos && (p->y()+p->h())  > pos ) {
        return p;
      }
    }
  }
  return NULL;
}

//-------------------------------------------------------------
int CTreeView::handle(int event)
{
  static int validMove;
  if (event == FL_NO_EVENT) return(0);
  if (Fl::event_x()+scrollbar.w() > w()) { // || Fl::event_y()+scrollbar.w() > h())
    return Fl_Scroll::handle(event);
  }
  switch (event) {
  case FL_FOCUS:
  case FL_UNFOCUS:
    return 1;
  case FL_KEYBOARD: {
    CTVItem * pTVItem = GetTVitem(m_AktIndex);
    if (pTVItem==0) {
      m_AktIndex=-1;
      pTVItem = m_TVItem;
    }
    int key = Fl::event_key();
    switch (key) {
    case FL_Enter:
      pTVItem->SetState(1);
      break;
    case FL_Down:
      if (SelChanging(m_AktIndex,0)) {
        int ix = GetNumOpenItems();
        if (m_AktIndex>ix-2) {
          SelChanged(0);
        } else {
          SelChanged(m_AktIndex+1);
        }
      }
      break;
    case FL_Up:
      if (SelChanging(m_AktIndex,0)) {
        if (m_AktIndex==0) {
          int ix = GetNumOpenItems();
          SelChanged(ix-1);
        } else {
          SelChanged(m_AktIndex-1);
        }
      }
      break;
    case FL_Left:
      if (SelChanging(m_AktIndex,0)) {
        if (pTVItem->m_NumChild && pTVItem->m_bExpand) {
          SelChanged(m_AktIndex);
          pTVItem->ExpandItem(VK_LEFT);
        } else {
          int ix = FindItemIndex(pTVItem->m_pParent);
          if (ix>=0) SelChanged(ix);
        }
      }
      break;
    case FL_Right:
      if (SelChanging(m_AktIndex,0)) {
        if (pTVItem->m_NumChild && pTVItem->m_bExpand==0) {
          pTVItem->ExpandItem(VK_RIGHT);
          SelChanged(m_AktIndex);
        } else {
          SelChanged(m_AktIndex+1);
        }
      }
      break;
    default:
      if (key >= ' ' && key <= 'z') {
        char str[20];
        str[0] = key;
        str[1] = '\0';
        // Search next Start char
        CTVItem *  pTVItem2 = GetTVitem(m_AktIndex+1);
        CTVItem *  find =  FindItemPart(str,pTVItem2);
        if (find == NULL) {
          // Search from Top
          find =  FindItemPart(str,m_TVItem);
        }
        if (find ) {
          if (SelChanging(m_AktIndex,0)) {
            int ix = FindItemIndex(find);
            EnsureVisible(ix);
            if (ix>=0) SelChanged(ix);
          }
        }
        return 1;
      } else {
        return Fl_Scroll::handle(event);
      }
    }
    EnsureVisible(m_AktIndex);
    return 1;
  }
  return Fl_Scroll::handle(event);
  break;
  case FL_PUSH : {
    Fl::focus(this);
    fingermovecnt = 0;
    scroll_start  = scrollbar.value();
    scroll_startx = hscrollbar.value();
    scroll_min    = 0;
    scroll_max    = m_pack->h()-y();
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
      scroll_to(valx,scroll_start-valy);
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
        if (valy < 10 && valy > -10) {
          CTVItem * p = GetItem(Fl::event_y()-y());
          if (p) {
            int ix = FindItemIndex(p);
            OnClick(p->m_Item_ID,0,Fl::event_x()-x());
            SelChanged(ix);
            p->redraw();
          }

        } else {
          int t = (GetTickCount() - startTickCount);
          if (t < 1000) {
            int val = Fl::event_y()-start_fingery;
            scroll_start  = scrollbar.value();
            if (t) {
              kpix_per_ms = val*1000 / t;
              Fl::repeat_timeout(0.01, TreeViewTimerCb, this);
              fingermovecnt = (val * abs(val)) / (t);
            }
          }
        }
      }
    } else {
      startTickCount=0;
      CTVItem * p = GetItem(Fl::event_y()-y());
      if (p) {
        int ix = FindItemIndex(p);
        OnClick(p->m_Item_ID,1,Fl::event_x()-x());
        p->redraw();
      }
    }
    if (Fl::event_button() == FL_RIGHT_MOUSE) {
    }

    return 1;
  }
  break;
  }
  return Fl_Scroll::handle(event);
}
//---------------------------------------------------------
int SelectTreeItem(CTreeView * pTreeView,LPSTR lpstr)
{
  char str[_MAX_PATH];
  if (strlen(lpstr)> 1) {
    strncpy(str,lpstr,_MAX_PATH);
    CTVItem *   pAktItem = NULL;
    LPSTR  pnt = strtok(str,"/");
    pAktItem = (CTVItem *) pTreeView->FindItem(pnt,NULL);
    while (pAktItem && pnt) {
      pnt = strtok(NULL,"/");
      if (pnt) {
        pAktItem ->ExpandItem(VK_RIGHT);//TVHT_VK_RIGHT);
        pAktItem = (CTVItem *) pTreeView->FindItem(pnt,pAktItem);
      }
    }
    if (pAktItem) {
      int index = pTreeView->FindItemIndex(pAktItem);
      if (index >= 0) {
        pTreeView->m_AktIndex = index;
        Fl::lock();
        Fl::check();    // time to adjust pack
        Fl::unlock();
        pTreeView->EnsureVisible(index);
        pTreeView->SelChanged(pTreeView->m_AktIndex);

        switch (pAktItem->GetImage()) {
        case imFolderClose:
        case imFolderOpen:
        case imMusicFolder:
        case imMusic:
        case imDokument:
          return  10;
        }
      }
    }
  }
  return 0;
}
//---------------------------------------------------------------

