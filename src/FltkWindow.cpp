#include "FltkWindow.h"
#include <FL/fl_draw.H>
#include "Resource.h"
#include "BastelUtils.h"
#include <FL/Fl_Preferences.H>
extern Fl_Preferences  * MainPrefs;
extern Fl_Double_Window* MainWindow;

#include <FL/Fl_Color_Chooser.H>

Fl_Color c = FL_GRAY;
#define fullcolor_cell (FL_FREE_COLOR)


void cb1(Fl_Widget *, void *v)
{
  uchar r,g,b;
  Fl::get_color(FL_FOREGROUND_COLOR,r,g,b);
  if (!fl_color_chooser("FOREGROUND_COLOR:",r,g,b,3)) return;
  Fl::foreground(r, g, b);
  Fl::reload_scheme();
  MainWindow->redraw();
}
void cb2(Fl_Widget *, void *v)
{
  uchar r,g,b;
  Fl::get_color(FL_BACKGROUND_COLOR,r,g,b);
  if (!fl_color_chooser("BACKGROUND_COLOR:",r,g,b,3)) return;
  Fl::background(r, g, b);
  Fl::reload_scheme();
  MainWindow->redraw();
}

void cb3(Fl_Widget *, void *v)
{
  uchar r,g,b;
  Fl::get_color(FL_BACKGROUND2_COLOR,r,g,b);
  if (!fl_color_chooser("BACKGROUND2_COLOR",r,g,b,3)) return;
  Fl::background2(r, g, b);
  Fl::reload_scheme();
  MainWindow->redraw();
}

void cb4(Fl_Widget *, void *v)
{
  uchar r,g,b;
  Fl::get_color(FL_SELECTION_COLOR,r,g,b);
  if (!fl_color_chooser("SELECTION_COLOR:",r,g,b,3)) return;
  Fl::set_color(FL_SELECTION_COLOR,r,g,b);
  Fl::reload_scheme();
  MainWindow->redraw();
}

#ifdef NATIVE
#include <FL/Fl_Native_File_Chooser.H>
#else
#include <FL/Fl_File_Chooser.H>
#endif
#ifdef NATIVE
Fl_Native_File_Chooser *File_Chooser;
#else
Fl_File_Chooser *File_Chooser;
#endif

#ifdef NATIVE
#else
void fc_callback2(Fl_File_Chooser *fc,  // I - File chooser
                  void            *data)  // I - Data
{
  const char    *filename;  // Current filename


  printf("fc_callback(fc = %p, data = %p)\n", fc, data);

  filename = fc->value();

  printf("    filename = \"%s\"\n", filename ? filename : "(null)");
}
#endif

char s_FileName[_MAX_PATH];
int ShowFileExplorer(char const* path, char const* filter,int savenane)
{
  int i;      // Looping var
  int count;      // Number of files selected
  char  relative[FL_PATH_MAX];  // Relative filename

#ifdef NATIVE
  File_Chooser = new Fl_Native_File_Chooser();
  File_Chooser->filter("ExplonData\t*.{xml,db}\n");
  char  str[256];
  prefs->get("LastFile",str,"",sizeof(str));
  File_Chooser->preset_file(str);// untitled_default());
#else
  File_Chooser = new Fl_File_Chooser(path, filter,
                                     (savenane)?Fl_File_Chooser::CREATE:Fl_File_Chooser::SINGLE, "Fl_File_Chooser");
  File_Chooser ->callback(fc_callback2);
#endif
  memset(&s_FileName,0,sizeof(s_FileName));

  //if (filter->value()[0])    File_Chooser->filter(filter->value());

  File_Chooser->show();

  while (File_Chooser->visible())
  {
    Fl::wait();
  }

  count = File_Chooser->count();
  if (count > 0)
  {

    for (i = 1; i <= count; i ++)
    {
      if (!File_Chooser->value(i))
        break;
      fl_filename_relative(relative, sizeof(relative), File_Chooser->value(i));
      if (i==1) strcpy(s_FileName,File_Chooser->value(i));
    }
  }
  delete File_Chooser;
  File_Chooser = NULL;
  return s_FileName[0]!=0;
}
//-------------------------------------------------------------
#ifdef WIN32
#include <sys/utime.h>
#else
#include <utime.h>
#endif
int CopyFile(char const  * lpPath1,char const  * lpPath2)
{
  int ret=0;
  struct stat statbuffer1= {0};
  struct stat statbuffer2= {0};

  if (stat(lpPath1, &statbuffer1)< 0) statbuffer1.st_size=0;
  if (stat(lpPath2, &statbuffer2)< 0) statbuffer2.st_size=0;

  if (statbuffer1.st_size)
  {
    int Buffsize = _min(statbuffer1.st_size+1,1000000);
#ifdef WIN32
    int fdr = open(lpPath1, O_RDONLY |O_BINARY);
#else
    int fdr = open(lpPath1, O_RDONLY | O_LARGEFILE);
#endif
    if (fdr>=0)
    {
      char * pBuff = (char * ) malloc(Buffsize +1);
      if (pBuff)
      {
#ifdef WIN32
        int fdw = open (lpPath2, O_CREAT | O_WRONLY   |O_BINARY, 0644);
#else
        int fdw = open (lpPath2, O_CREAT | O_TRUNC |O_WRONLY  | O_LARGEFILE |  O_NONBLOCK, 0644);
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
          }
          while (wrx && iRead && ret);
          close(fdw);
#ifdef WIN32
          _utimbuf times ;
          times.actime = statbuffer1.st_atime;
          times.modtime= statbuffer1.st_mtime;
          _utime(lpPath2, &times);
#else
          struct utimbuf times ;
          times.actime = statbuffer1.st_atime;
          times.modtime= statbuffer1.st_mtime;
          utime(lpPath2, &times);
#endif
        }
        free(pBuff);
      }
      close(fdr);
    }
  }
  return ret;
}
//---------------------------------------------------------------
#ifdef NATIVE
#else
void fc_callback(Fl_File_Chooser *fc, // I - File chooser
                 void            *data) // I - Data
{
  const char    *filename;  // Current filename


  printf("fc_callback(fc = %p, data = %p)\n", fc, data);

  filename = fc->value();

  printf("    filename = \"%s\"\n", filename ? filename : "(null)");
}
#endif

void cbOpenData(Fl_Button*, void*)
{
#if NATIVE
  if (File_Chooser)
  {
    File_Chooser->title("Open");
    File_Chooser->type(Fl_Native_File_Chooser::BROWSE_FILE);    // only picks files that exist
    switch (File_Chooser->show() )
    {
    case -1:
      break;  // Error
    case  1:
      break;  // Cancel
    default:    // Choice
      File_Chooser->preset_file(File_Chooser->filename());
      if (prefs) prefs->set("LastFile",File_Chooser->filename());
      break;
    }
  }
#else
//  int i;      // Looping var
  int count;      // Number of files selected
//  char  relative[FL_PATH_MAX];  // Relative filename

#ifdef NATIVE
  File_Chooser = new Fl_Native_File_Chooser();
  File_Chooser->filter("ExplonData\t*.{xml,db}\n");
  char  str[256];
  MainPrefs->get("LastFile",str,"",sizeof(str));
  File_Chooser->preset_file(str);// untitled_default());
#else
  char  str[256];
  MainPrefs->get("LastFile",str,".",sizeof(str));
  File_Chooser = new Fl_File_Chooser(str, "ExplonDatabase (*.{xml,db})", Fl_File_Chooser::SINGLE, "Fl_File_Chooser");
  File_Chooser ->callback(fc_callback);
#endif

  //if (filter->value()[0])    File_Chooser->filter(filter->value());

  File_Chooser->show();

  while (File_Chooser->visible())
  {
    Fl::wait();
  }

  count = File_Chooser->count();
  if (count > 0)
  {
    //files->clear();
    if (MainPrefs) MainPrefs->set("LastFile",File_Chooser->value(1));

//    for (i = 1; i <= count; i ++)
    {
      if (File_Chooser->value(1))
      {


//        //fl_filename_relative(relative, sizeof(relative), File_Chooser->value(i));
//        strncpy(gpLonDatabase->m_szDatenbankName,File_Chooser->value(1),sizeof(gpLonDatabase->m_szDatenbankName));
//        gpLonDatabase->KnotenTabelleEinlesen(1);
//        gBastelUtils.WriteProfileString("Profiler","TableNameX",gpLonDatabase->m_szDatenbankName);
//        if (LonTreeMaster) //gDesk.m_pLonTree)
//        {
//          if (gpNVConnectionList)  gpNVConnectionList->Reset();
//
//          TreeViewEinlesen(LonTreeMaster,-1);
//          if (LonTreeLeft)TreeViewEinlesen(LonTreeLeft,1);
//          if (LonTreeRight)TreeViewEinlesen(LonTreeRight,2);
//          if (gpNVConnectionList)
//          {
//            gpNVConnectionList->LeseConectionsTabelle(1,1);
//            gpNVConnectionList->DisplayConectionsResult(NULL,NULL,NULL,NULL,1,0);
//          }
//        }

      }

      //files->add(relative,
//                 Fl_File_Icon::find(File_Chooser->value(i), Fl_File_Icon::PLAIN));
    }

    //  files->redraw();
  }
  delete File_Chooser;
  File_Chooser = NULL;
#endif
}
//-------------------------------------------------------------

//---------------------------------------------------------
#ifndef WIN32
void Sleep(int ms){
  usleep(ms*1000);
}
#endif
//---------------------------------------------------------
BOOL delay(int tx)
{
  DWORD t = GetTickCount();
  do {
    ifEscape();
     Sleep(5);
  } while ((t+tx) > GetTickCount());
  return false;
}
//------- Pruefe ob irgend eine Eingabe die Unterbrechung erfordert
BOOL  Abbruch = false;
BOOL  ifEscape(void)
{
  if (Fl::event_key()==FL_Escape){
        Abbruch = true;
      }
  Sleep(10);
//  Fl::unlock();
  return Abbruch;
}
