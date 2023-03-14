#include "FltkDialogBox.h"
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Radio_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Value_Slider.H>



//-------------------------------------------------------------
CBastelButton::CBastelButton (int X, int Y, int W, int H, const char *L=0):Fl_Button(X,Y,W,H,L) {
;
}
//-------------------------------------------------------------
CBastelButton::~CBastelButton(){
  ;
}
//-------------------------------------------------------------
CFltkDialogBox::CFltkDialogBox(int X,int Y,int W,int H,const char *L):Fl_Window(X,Y,W,H,L)
{
  m_NumDialogCtlItem=0;
  m_pDialogCtlItem  = NULL;
  callback(MainProc,0);
}
//-------------------------------------------------------------
CFltkDialogBox::~CFltkDialogBox()
{

}
//-------------------------------------------------------------
void  CFltkDialogBox::MainProc(Fl_Widget* item, void* Value)
{
  long x = (long)Value;
  ((CFltkDialogBox*) item)->WindowProc(WM_COMMAND,x,0);
}
//-------------------------------------------------------------

#define FONTFACE    FL_COURIER  //  FL_HELVETICA
#define FONTSIZE    12

int CFltkDialogBox::InitWindow(Fl_Widget* pParent, int ID, int left, int top, int width, int height, int Style)
{
  //DWORD dunit = GetDialogBaseUnits();
  int ySpace  = fl_height();
  int xSpace  = fl_width("A");
  if (ySpace  < 3 || ySpace  >40 || xSpace  < 3 || xSpace  > 30)
  {
    ySpace  = 16;//16;
    xSpace  = 6;//8;
  }


// rolfs workaround
#ifndef WIN32
// xSpace = xSpace * 4 / 3;
// ySpace = ySpace * 8 / 6;
#else
//  xSpace = xSpace * 3 / 4;
//  ySpace = ySpace * 6 / 8;
#endif
  if (ID==m_pDialogCtlItem->idc)
  {
    //left  =   left  * xSpace  / 4;
    //top   =   top   * ySpace  / 8;
    width =   width * xSpace  / 4;
    height=   height* ySpace  / 8;
  }
  Fl_Group * pGroup = NULL;
  int num = 0;
  tDialogCtlItem * pCtlItem = m_pDialogCtlItem ;
  pCtlItem = m_pDialogCtlItem ;
  num      = m_NumDialogCtlItem -1;

  pCtlItem ++;    // skip the dialog himselve

  while (num > 0 )
  {
    num --;

    int X = pCtlItem->left  = (pCtlItem->left  * xSpace  / 4);// + x();
    int Y = pCtlItem->top   = (pCtlItem->top   * ySpace  / 8);// + y();
    int W = pCtlItem->width = pCtlItem->width * xSpace  / 4;
    int H = pCtlItem->height= pCtlItem->height* ySpace  / 8;

    if ((pCtlItem->style & WS_GROUP)==0)  {
      if (pGroup) pGroup->end();
      pGroup=NULL;
    }
    switch(pCtlItem->type)
    {
    case eButton:
      pCtlItem->pWndObject = new Fl_Button(X,Y,W,H,pCtlItem->text);
      break;
    case eOptions:
      if (pCtlItem->style & BS_AUTOCHECKBOX)
        pCtlItem->pWndObject = new Fl_Light_Button(X,Y,W,H,pCtlItem->text);
      else
        pCtlItem->pWndObject = new Fl_Toggle_Button(X,Y,W,H,pCtlItem->text);
      break;
//        pCtlItem->pWndObject = new Fl_Radio_Button(X,Y,W,H,pCtlItem->text);
    case eEditor:
      pCtlItem->pWndObject = new Fl_Input (X,Y,W,H,pCtlItem->text);
      if (pCtlItem->pWndObject)
      {
//          if (pCtlItem->text[0]!= '\0')
//            pCtlItem->pWndObject->SetText(pCtlItem->text);
//          //pCtlItem->style |= WS_BORDER|WS_TABSTOP;
//          pCtlItem->style |= WS_TABSTOP;
      }
      break;
    case eSliderCtl:
      if (pCtlItem->style & BS_AUTOCHECKBOX)
      pCtlItem->pWndObject = new Fl_Value_Slider(X,Y,W,H,pCtlItem->text);
      else
      pCtlItem->pWndObject = new Fl_Slider(X,Y,W,H,pCtlItem->text);
      if(pCtlItem->style & WS_HSCROLL) pCtlItem->pWndObject->type(FL_HOR_SLIDER);
      pCtlItem->pWndObject->align(Fl_Align(FL_ALIGN_BOTTOM));
      break;
//      case eCustomItem:
//        pCtlItem->pWndObject = NewCustomItem(pCtlItem->idc);
//        break;
    case eList:
      pCtlItem->pWndObject = new Fl_Browser(X,Y,W,H,pCtlItem->text);
      break;
    case eControls:
      if (pCtlItem->style & WS_MAXIMIZE) {
         pCtlItem->pWndObject = new CBastelButton(X,Y,W,H,pCtlItem->text);
         break;
      }
      if (pCtlItem->style & WS_GROUP) {
        if (pGroup==NULL) {
          pGroup = new Fl_Group(X,Y,W,H);
          pGroup ->box(FL_THIN_UP_FRAME);
          pCtlItem->pWndObject = pGroup;
        }
      } else {
        pCtlItem->pWndObject = new Fl_Box(X,Y,W,H,pCtlItem->text);
      }

      if (pCtlItem->style & WS_BORDER)
      {
        pCtlItem->pWndObject->box(FL_DOWN_FRAME);
      }
      if (pCtlItem->style & WS_DLGFRAME)
      {
        pCtlItem->pWndObject->box(FL_UP_BOX);
      }
      if (pCtlItem->style & SS_ETCHEDFRAME)
      {
        pCtlItem->pWndObject->box(FL_DOWN_FRAME);
      }
      break;
    case eText:
      pCtlItem->pWndObject = new Fl_Box(X,Y,W,H,pCtlItem->text);
      break;
    default:
      pCtlItem->pWndObject = NULL;
      break;
    }
    if (pCtlItem->pWndObject)
    {
      if(pCtlItem->style & WS_DISABLED) pCtlItem->pWndObject->deactivate();
      //if (pCtlItem->style & WS_BORDER) pCtlItem->pWndObject->box(FL_FLAT_BOX);// BORDER_BOX);
      //if (pCtlItem->style & WS_BORDER) pCtlItem->pWndObject->box(FL_DOWN_BOX);

      pCtlItem->pWndObject->align(Fl_Align(FL_ALIGN_CLIP)|FL_ALIGN_INSIDE|FL_ALIGN_CENTER| FL_ALIGN_WRAP);//FL_ALIGN_LEFT);
//      pCtlItem->pWndObject->labelfont (  fl_font());
//      pCtlItem->pWndObject->labelsize (fl_size() );
      if (pGroup) {
        pCtlItem->pWndObject->callback((Fl_Callback*)cbGroupProc,(void*)(long)pCtlItem->idc);
      } else {
        pCtlItem->pWndObject->callback((Fl_Callback*)cbUniversal,(void*)(long)pCtlItem->idc);
      }
    }
    //else pCtlItem->style |= WS_VISIBLE;
    pCtlItem ++;
  }
  return 1;

}
//-------------------------------------------------------------
void  CFltkDialogBox::cbUniversal(Fl_Widget* item, void* idc) {
  long x = (long)idc;
  ((CFltkDialogBox*)item->parent())->WindowProc(WM_COMMAND,x,(LPARAM)item);
}
//-------------------------------------------------------------
void  CFltkDialogBox::cbGroupProc(Fl_Widget* item, void* idc) {
  long x = (long)idc;
  item = item->parent();
  if (item!=0)  ((CFltkDialogBox*)item->parent())->WindowProc(WM_COMMAND,x,(LPARAM)item);
}
//-------------------------------------------------------------
void CFltkDialogBox::draw()
{
  Fl_Window::draw();
}
//-------------------------------------------------------------
int CFltkDialogBox::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  return 0;
}
//-------------------------------------------------------------
Fl_Widget* CFltkDialogBox::NewCustomItem(int IDC)
{
  return 0;
}
//-------------------------------------------------------------
Fl_Widget* CFltkDialogBox::GetDlgItem(int nIDDlgItem)
{

  int i;
  for (i=0; i < children(); i++)
  {
    Fl_Widget*  res = child(i);
    if (res)
    {
      void  * p = res->user_data();
      long x = (long)p;
      if (x == nIDDlgItem) return res;
     // if (*(int*)res->user_data()== nIDDlgItem) return res;
    }
  }
  for (i=1; i < m_NumDialogCtlItem ; i++)
  {
    if (m_pDialogCtlItem[i].idc == nIDDlgItem) return m_pDialogCtlItem[i].pWndObject;
  }
  return NULL;
}
//-------------------------------------------------------------
tDialogCtlItem* CFltkDialogBox::pGetDlgItem(int nIDDlgItem)
{
  for (int i=1; i < m_NumDialogCtlItem ; i++)
  {
    if (m_pDialogCtlItem[i].idc == nIDDlgItem) return &m_pDialogCtlItem[i];
  }
  return NULL;
}
//-------------------------------------------------------------
BOOL CFltkDialogBox::CheckDlgButton(int nIDButton, UINT uCheck)
{
  //CControls *  pItem =  (CControls * ) GetDlgItem(nIDButton);
  tDialogCtlItem * pItem = pGetDlgItem(nIDButton);
  if (pItem )
  {
    switch(pItem->type)
    {
    case eButton:
    case eOptions:
      ((Fl_Toggle_Button*)pItem->pWndObject)->value(uCheck!=0);
      pItem->pWndObject->redraw();
      return 1;
    case eEditor:
    case eSliderCtl:
    case eList:
    case eControls:
    case eText:
      break;
    }
  }
  return 0;

}
//-------------------------------------------------------------
BOOL CFltkDialogBox::IsDlgButtonChecked(int nIDButton)
{
  //CControls *  pItem =  (CControls * ) GetDlgItem(nIDButton);
  tDialogCtlItem * pItem = pGetDlgItem(nIDButton);
  if (pItem )
  {
    switch(pItem->type)
    {
    case eButton:
    case eOptions:
      return ((Fl_Toggle_Button*)pItem->pWndObject)->value();
    case eEditor:
    case eSliderCtl:
    case eList:
    case eControls:
    case eText:
      break;
    }
  }
  return 0;

}
//-------------------------------------------------------------
BOOL CFltkDialogBox::CheckRadioButton(int nIDFirstButton, int nIDLastButton, int nIDCheckButton)
{
  for (int idc=nIDFirstButton; idc <= nIDLastButton; idc++)
  {
    CheckDlgButton(idc,(idc ==  nIDCheckButton)?1:0);
  }
  return 1;

}
//-------------------------------------------------------------
int CFltkDialogBox::SendDlgItemMessage(int nIDDlgItem, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  return 0;
}
//-------------------------------------------------------------
int CFltkDialogBox::GetDlgItemInt(int nIDDlgItem, BOOL* lpTranslated, BOOL bSigned)
{
  char str[30];
  if (lpTranslated ) * lpTranslated = 0;
  tDialogCtlItem * pItem = pGetDlgItem(nIDDlgItem);
  if (pItem && pItem->pWndObject)
  {
    switch(pItem->type)
    {
    case eEditor:
    {
      const char * pnt = ((Fl_Input *)pItem->pWndObject)->value();
      if (pnt)
      {
        strncpy(str,pnt,sizeof(str));
        if (lpTranslated ) * lpTranslated = 1;
        return atoi(str);
      }
      return 0;
    }
    case eText:
    case eList:
    case eButton:
    case eOptions:
    case eControls:
    {
      const char * pnt  = pItem->pWndObject->label();
      if (pnt)
      {
        strncpy(str,pnt,sizeof(str));
        if (lpTranslated ) * lpTranslated = 1;
        return atoi(str);
      }
      return 0;
    }
    break;
    case eSliderCtl:
      return  (int)((Fl_Valuator*)pItem->pWndObject)->value();
      break;
    }
  }
  return 0;
}
//-------------------------------------------------------------
BOOL CFltkDialogBox::EnableDlgItem(int nIDDlgItem, BOOL bEnable)
{
  tDialogCtlItem * pItem = pGetDlgItem(nIDDlgItem);
  if (pItem )
  {
    switch(pItem->type)
    {
    case eButton:
    case eOptions:
    case eEditor:
    case eSliderCtl:
    case eList:
    case eControls:
    case eText:
      if (bEnable) pItem->pWndObject->activate();
      else pItem->pWndObject->deactivate();
      pItem->pWndObject->redraw();
      return 1;
      break;
    }

  }
  return 0;
}
//-------------------------------------------------------------
BOOL CFltkDialogBox::SetDlgItemInt(int nIDDlgItem, int uValue, BOOL bSigned)
{
  char str[30];
  sprintf(str,"%d",uValue);
  tDialogCtlItem * pItem = pGetDlgItem(nIDDlgItem);
  if (pItem && pItem->pWndObject)
  {
    switch(pItem->type)
    {
    case eEditor:
    {
      ((Fl_Input *)pItem->pWndObject)->value(str);
      pItem->pWndObject->redraw();
      return 1;
    }
    case eText:
    case eList:
    case eButton:
    case eOptions:
    case eControls:
    {
      pItem->pWndObject->copy_label(str);
      pItem->pWndObject->redraw();
      return 1;
    }
    break;
    case eSliderCtl:
      ((Fl_Valuator*)pItem->pWndObject)->value((double)uValue);
      pItem->pWndObject->redraw();
      return 1;
      break;
    }
  }
  return 0;
}
//-------------------------------------------------------------
BOOL CFltkDialogBox::SetDlgItemText(int nIDDlgItem, char const  * lpString)
{
  tDialogCtlItem * pItem = pGetDlgItem(nIDDlgItem);
  if (pItem )    switch(pItem->type)
    {
    case eEditor:
    {
      if (pItem->pWndObject)
      {
        ((Fl_Input *)pItem->pWndObject)->value(lpString);
        pItem->pWndObject->redraw();
        return 1;
      }
    }
    case eText:
    case eSliderCtl:
    case eList:
    case eButton:
    case eOptions:
    case eControls:
    {
      if (pItem->pWndObject)
      {
        pItem->pWndObject->copy_label(lpString);
        pItem->pWndObject->redraw();
        return 1;
      }
    }
    break;
    }
  return 0;


}
//-------------------------------------------------------------
int CFltkDialogBox::GetDlgItemText(int nIDDlgItem, LPSTR lpString, int nMaxCount)
{
  tDialogCtlItem * pItem = pGetDlgItem(nIDDlgItem);
  if (pItem )    switch(pItem->type)
    {
    case eEditor:
    {
      if (pItem->pWndObject)
      {
        const char * pnt = ((Fl_Input *)pItem->pWndObject)->value();
        if (pnt)
        {
          strncpy(lpString,pnt,nMaxCount);
          return strlen(lpString);
        }
        return 0;
      }
    }
    case eText:
    case eSliderCtl:
    case eList:
    case eButton:
    case eOptions:
    case eControls:
    {
      if (pItem->pWndObject)
      {
        const char * pnt  = pItem->pWndObject->label();
        if (pnt)
        {
          strncpy(lpString,pnt,nMaxCount);
          return strlen(lpString);
        }
        return 0;
      }
    }
    break;
    }
  return 0;
}
//-------------------------------------------------------------
BOOL CFltkDialogBox::ShowDlgItem(int nIDDlgItem, int bShow)
{
  tDialogCtlItem * pItem = pGetDlgItem(nIDDlgItem);
  if (pItem )
  {
    switch(pItem->type)
    {
    case eButton:
    case eOptions:
    case eEditor:
    case eSliderCtl:
    case eList:
    case eControls:
    case eText:
      if (bShow) pItem->pWndObject->show();
      else       pItem->pWndObject->hide();
      pItem->pWndObject->redraw();
      return 1;
      break;
    }

  }
  return 0;

}
//-------------------------------------------------------------
BOOL CFltkDialogBox::SetButtonImage(int nIDDlgItem, int iImage)
{
  return 0;
}
//-------------------------------------------------------------
