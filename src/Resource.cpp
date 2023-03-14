#include "Resource.h"

#define static const
#include "../icons/AppIcon.xpm"
#include "../icons/FileMerge.xpm"
#include "../icons/TreeViewPics44.xpm"

#undef static

Fl_Pixmap    tmp (AppIcon_xpm);
Fl_RGB_Image ReseorceInit::ic_AppIcon_xpm (&tmp);
Fl_Pixmap  ReseorceInit::TreeViewPics(TreeViewPics44_xpm);


ReseorceInit::ReseorceInit()
{
  Fl_Pixmap  *  tmp = new Fl_Pixmap (FileMerge_xpm);
  Fl_RGB_ImageList * RGB = new Fl_RGB_ImageList(tmp);

  int image_w = RGB ->w()/20;
  int image_h = RGB ->h();

  for (int i=0; i < 20; i++)
  {
    ButtonImage_List[i] = (Fl_Pixmap*)RGB->copy(image_w,image_h,i);
  }
  delete RGB;
}

ReseorceInit::~ReseorceInit()
{
  //dtor
}



Fl_RGB_ImageList::Fl_RGB_ImageList(const Fl_Pixmap *pxm, Fl_Color bg):Fl_RGB_Image(pxm,bg)
{


}

Fl_RGB_ImageList::~Fl_RGB_ImageList()
{

}



Fl_Image* Fl_RGB_ImageList::copy(int W, int H, int index)
{
  Fl_RGB_Image	*new_image;
  uchar		*new_array;

  if (((W*(index+1)) <= w() && H <= h()) &&  array)
  {
    // Make a copy of the image data and return a new Fl_RGB_Image...
    new_array = new uchar[W * H * d()];
    const uchar *src = array + (W*index*d());
    uchar *dst = new_array;
    int dy, dh = h();
    int wd  = W*d();
    int wld = ld() ? ld() : w() * d();
    for (dy=0; dy<dh; dy++)
    {
      memcpy(dst, src, wd);
      src += wld;
      dst += wd;
    }
    new_image = new Fl_RGB_Image(new_array, W, H, d());
    new_image->alloc_array = 1;
    return new_image;
  }
  return NULL;
}

