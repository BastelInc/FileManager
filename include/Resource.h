#ifndef RESOURCE_H
#define RESOURCE_H
#include <FL/Fl.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_RGB_Image.H>


class FL_EXPORT Fl_RGB_ImageList : public Fl_RGB_Image
{
public:
  Fl_RGB_ImageList(const Fl_Pixmap *pxm, Fl_Color bg=FL_GRAY);
  virtual ~Fl_RGB_ImageList();
  virtual Fl_Image *copy(int W, int H,int index);


};

class ReseorceInit
{
public:
  ReseorceInit ();
  virtual ~ReseorceInit ();

  static Fl_RGB_Image ic_AppIcon_xpm ;
  static Fl_Pixmap    ic_DrillCentre_xpm;
  static Fl_Pixmap  image_FullScreen;
  static Fl_Pixmap  image_Exit;
  static Fl_Pixmap  TreeViewPics;
//  Fl_Image
  Fl_Pixmap* ButtonImage_List[26];

protected:

private:
};

extern ReseorceInit * R;

enum {
imExit         =19 ,
imFILEOPEN     =1 ,
imFILESAVE     =2 ,
imUPDATE       =3 ,
};
enum  eImage {imVert,imTee,imVertEnd,imOpenCont,imOpenLast,imCloseCont,imCloseLast,imFolderClose,imFolderOpen,imDokument,imCompactDsik,
              imDiashow,imPicture,imMusic,imWebBrowser,imRadio,imSatelite,imClose,imTV,imVideo,imMovie,imText,imDrive,imSystem,imState,iCamera,iExtmemory,iRemove,imAspect,imApllication,iVolumDev,imMusicFolder,imYouTube,imExplon,imExplonLoc,imExplonNode,imNumImage
             };




#define  IDM_FILEOPEN   1001
#define  IDM_FILESAVE   1002
#define  IDM_UPDATE     1003
#define  IDM_EXIT       1004
#define  IDM_SCANFOLDER 1005

#define  IDC_Docs       1100
#define  IDC_DocsBuero  1101
#define  IDC_DocsMedia  1102
#define  IDC_DocsCAD    1103
#define  IDC_DocsPict   1104
#define  IDC_DocsSrc    1105
#define  IDC_DocsSys    1106
#define  IDC_LocalSrc   1140
#define  IDC_ServerSrc  1141
#define  IDC_BackupSrc  1142
#define  IDC_Equal      1143
#define  IDC_Differ     1144

#define  IDC_Documents   1150
#define  IDC_DocumentImg 1151
#define  IDC_Local       1152
#define  IDC_LocalPath   1153
#define  IDC_LocalUpd    1154
#define  IDC_LocalGet    1155
#define  IDC_Server      1156
#define  IDC_ServerPath  1157
#define  IDC_ServerUpd   1158
#define  IDC_ServerGet   1159
#define  IDC_Backup1     1160
#define  IDC_Backup1Path 1161
#define  IDC_Backup1Upd  1162
#define  IDC_Backup1Get  1163

#define  IDC_DiffToolGet 1170
#define  IDC_DiffPath    1171
#define  IDC_EditToolGet 1172
#define  IDC_EditPath    1173
#define  IDC_JobList     1174
#define  IDC_ToLocal     1175
#define  IDC_ToServer    1176
#define  IDC_ToUSBStk    1177
#define  IDC_RunJob      1178

#endif // RESOURCE_H
