#ifndef __BASTELTYPE_H__
#define __BASTELTYPE_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#ifndef WIN32
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
//#include <curses.h>

#include <pthread.h>






typedef unsigned char   BYTE;
typedef unsigned char * LPBYTE;
#if __x86_64__
typedef long long int        LONG;
typedef long long unsigned int ULONG;
typedef unsigned int DWORD;
typedef unsigned short int WORD;
typedef unsigned long long LPARAM;
typedef unsigned int WPARAM;
typedef unsigned int UINT;
typedef void *   HANDLE;
#else
typedef long int        LONG;
typedef long unsigned int ULONG;
typedef unsigned int DWORD;
typedef unsigned short int WORD;
typedef unsigned long LPARAM;
typedef unsigned int WPARAM;
typedef unsigned int UINT;
typedef void *   HANDLE;
#endif
typedef unsigned int *LPDWORD;
typedef unsigned long long  ULONGLONG;
typedef long long  LONGLONG;
typedef pthread_mutex_t CRITICAL_SECTION;
typedef pthread_mutex_t OVERLAPPED;
#define FAR
typedef int * HINSTANCE;
typedef int   HCURSOR;
typedef unsigned int   COLORREF;
#define __cdecl

#ifndef XWND
#ifdef DVBKISTE
#define RGB(r, g ,b)  ((DWORD) (((BYTE) (b) | ((WORD) (g) << 8)) | (((DWORD) (BYTE) (r)) << 16))| 0xF0000000)
#else
#define RGB(r, g ,b)  ((DWORD) (((BYTE) (b) | ((WORD) (g) << 8)) | (((DWORD) (BYTE) (r)) << 16)))
#endif
#endif
#define MAKEWORD(a, b)      ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
#define LOWORD(l)           ((WORD)(l))
#define HIWORD(l)           ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
#define LOBYTE(w)           ((BYTE)(w))
#define HIBYTE(w)           ((BYTE)(((WORD)(w) >> 8) & 0xFF))
typedef int         BOOL;
#define EnterCriticalSection    pthread_mutex_lock
#define LeaveCriticalSection    pthread_mutex_unlock
#define TryEnterCriticalSection pthread_mutex_trylock

inline void InitializeCriticalSection(pthread_mutex_t *  pcriticalsection)
{
  pthread_mutex_init(pcriticalsection,NULL);
}

#ifndef _max
#define _max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef _min
#define _min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef WIN32
#define O_BINARY  0
#else
#define O_LARGEFILE 0
#endif

#define INVALID_HANDLE_VALUE  0
#ifndef _MAX_PATH
#define _MAX_PATH  256
#endif
#ifndef MAX_PATH
#define MAX_PATH 256
#endif
#define __stdcall

#ifdef WIN32
#pragma pack(1)
#define PACKATTR
#else
#define PACKATTR __attribute__((packed))
#endif

typedef struct BMP_FILE_HEAD
{
    WORD  bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
}PACKATTR BMP_FILE_HEAD ;

typedef struct BITMAPINFOHEADER
{
    DWORD biSize;
    DWORD biWidth;
    DWORD biHeight;
    WORD  biPlanes;
    WORD  biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    DWORD biXPelsPerMeter;
    DWORD biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
}PACKATTR  BITMAPINFOHEADER ;

typedef int HGDIOBJ;
typedef int HPEN;

#define OPAQUE       2
#define TRANSPARENT  1

#define DT_TOP              0x00000001
#define DT_LEFT             0x00000002
#define DT_CENTER           0x00000004
#define DT_RIGHT            0x00000008
#define DT_VCENTER          0x00000010
#define DT_BOTTOM           0x00000020
#define DT_NONBLOCK         0x00000040
#define DT_BLOCK            0x00000080

//typedef struct osd_fontchar_s osd_fontchar_t;
//typedef struct osd_font_s     osd_font_t;
typedef unsigned short        uint16_t;
typedef unsigned char         uint8_t;



typedef BITMAPINFOHEADER * HBITMAP  ;
typedef char           * LPSTR      ;
typedef const char     *  const LPCSTR      ;

#define LF_FACESIZE   256

#define VK_INSERT_CLIPB   0x34b   // Shift Ins.
#define VK_COPY_TO_CLIPB  0x54b   // Ctl   Ins.
#define VK_DEL_TO_CLIPB   0x34a   // Shif  Del

#define VK_UNKNOWN        0x000
#define VK_ESCAPE         0x01B
#define VK_ENTER          0x00d
#define VK_TAB            0x009
#define VK_TABSHIFT       0x209
#define VK_CTLPLUS        0x42B
#define VK_CTLMINUS       0x42D
#define VK_BACK           0x107
#define VK_DELETE         0x14a
#define VK_INSERT         0x14b
#define VK_CTL_A          0x401
#define VK_CTL_B          0x402
#define VK_CTL_C          0x403
#define VK_CTL_D          0x404
#define VK_CTL_E          0x405
#define VK_CTL_F          0x406
#define VK_CTL_G          0x407
#define VK_CTL_H          0x408
#define VK_CTL_I          0x409
#define VK_CTL_J          0x40A
#define VK_CTL_K          0x40B
#define VK_CTL_L          0x40C
#define VK_CTL_M          0x40D
#define VK_CTL_N          0x40E
#define VK_CTL_O          0x40F
#define VK_CTL_P          0x410
#define VK_CTL_Q          0x411
#define VK_CTL_R          0x412
#define VK_CTL_S          0x413
#define VK_CTL_T          0x414
#define VK_CTL_U          0x415
#define VK_CTL_V          0x416
#define VK_CTL_W          0x417
#define VK_CTL_X          0x418
#define VK_CTL_Y          0x419
#define VK_CTL_Z          0x41A

#define VK_F1             0x109
#define VK_F2             0x10A
#define VK_F3             0x10B
#define VK_F4             0x10C
#define VK_F5             0x10D
#define VK_F6             0x10E
#define VK_F7             0x10F
#define VK_F8             0x110
#define VK_F9             0x111
#define VK_F10            0x112
#define VK_F11            0x113
#define VK_F12            0x114
#define VK_F13            0x000
#define VK_F14            0x000
#define VK_F15            0x000
#define VK_F16            0x000
#define VK_F17            0x000
#define VK_F18            0x000
#define VK_F19            0x000
#define VK_F20            0x000


#define VK_SF1            0x115
#define VK_SF2            0x116
#define VK_SF3            0x117
#define VK_SF4            0x118
#define VK_SF5            0x119
#define VK_SF6            0x11A
#define VK_SF7            0x11B
#define VK_SF8            0x11C
#define VK_SF9            0x11D
#define VK_SF10           0x11E
#define VK_SF11           0x11F
#define VK_SF12           0x120





#define VK_CONTROL_L      0x400
#define VK_CONTROL_R      0x400

#define VK_SHIFT_L        0x200
#define VK_SHIFT_R        0x200

#define VK_ALT_L          0x800
#define VK_ALTGR          0x800

#define VK_CAPS_LOCK     0x1100
#define VK_NUM_LOCK      0x1200
#define VK_SCROLL_LOCK   0x1300
#define VK_PRINT         0x1400

#define VK_KP_0           0x030
#define VK_KP_1           0x031
#define VK_KP_2           0x032
#define VK_KP_3           0x033
#define VK_KP_4           0x034
#define VK_KP_5           0x035
#define VK_KP_6           0x036
#define VK_KP_7           0x037
#define VK_KP_8           0x038
#define VK_KP_9           0x039
#define VK_KP_MULT        0x02A
#define VK_KP_MINUS       0x02D
#define VK_KP_PLUS        0x02B
#define VK_KP_DIV         0x02F
#define VK_KP_DECIMAL     0x02E
#define VK_KP_ENTER       0x00A








#define VK_PRIOR          0x153 //KEY_PPAGE
#define VK_NEXT           0x152 //KEY_NPAGE
#define VK_END            0x168
#define VK_HOME           0x106
#define VK_LEFT           0x104 //KEY_LEFT
#define VK_UP             0x103 //KEY_UP
#define VK_RIGHT          0x105 //KEY_RIGHT
#define VK_DOWN           0x102 //KEY_DOWN
#define VK_RETURN         0x00a //KEY_ENTER  0x00a
#define VK_SPACE          0x020









#define SBM_SETPOS                  0x00E0
#define SBM_GETPOS                  0x00E1
#define SBM_SETRANGE                0x00E2
#define SBM_SETRANGEREDRAW          0x00E6
#define SBM_GETRANGE                0x00E3
#define SBM_ENABLE_ARROWS           0x00E4

#define SBM_SETSCROLLINFO           0x00E9
#define SBM_GETSCROLLINFO           0x00EA

#define SIF_RANGE           0x0001
#define SIF_PAGE            0x0002
#define SIF_POS             0x0004
#define SIF_DISABLENOSCROLL 0x0008
#define SIF_TRACKPOS        0x0010
#define SIF_ALL             (SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS)
#define SB_HORZ             0
#define SB_VERT             1
#define SB_CTL              2
#define SB_BOTH             3


typedef struct tagPOINTS
{
  int x:16;
  int y:16;
} POINTS, * PPIONTS;

#define MAKEPOINTS(l)   (*((POINTS *)&(l)))

typedef struct tagRECT
{
   int left  :16;
   int top   :16;
   int right :16;
   int bottom:16;

   //short left;
   //short top;
   //short right;
   //short bottom;
} RECT,  *LPRECT;
extern void Sleep(int ms);
#else
#include <windows.h>
#include <windowsx.h>
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>
//#include "WM_USER_Def.h"

#define stat64 stat
//#define _max  max
//#define _min  min

#ifndef NOMINMAX
#ifndef _max
#define _max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef _min
#define _min(a,b) ((a)<(b)?(a):(b))
#endif
#endif

#define HTCLIENT2 HTCLIENT+2
#define HTCLIENT3 HTCLIENT+3

#ifdef __cplusplus
extern "C" {
#endif
WINBASEAPI BOOL WINAPI TryEnterCriticalSection(
  LPCRITICAL_SECTION lpCriticalSection  // pointer to critical
                                        // section object
);
#ifdef __cplusplus
}
#endif
#define DT_BLOCK            0x00000020


#define VK_INSERT_CLIPB   0x34b   // Shift Ins.
#define VK_COPY_TO_CLIPB  0x54b   // Ctl   Ins.
#define VK_DEL_TO_CLIPB   0x34a   // Shif  Del

#define VK_UNKNOWN        0x000
#define VK_TABSHIFT       0x209
#define VK_CTLPLUS        0x42B
#define VK_CTLMINUS       0x42D
#define VK_CTL_A          0x401
#define VK_CTL_B          0x402
#define VK_CTL_C          0x403
#define VK_CTL_D          0x404
#define VK_CTL_E          0x405
#define VK_CTL_F          0x406
#define VK_CTL_G          0x407
#define VK_CTL_H          0x408
#define VK_CTL_I          0x409
#define VK_CTL_J          0x40A
#define VK_CTL_K          0x40B
#define VK_CTL_L          0x40C
#define VK_CTL_M          0x40D
#define VK_CTL_N          0x40E
#define VK_CTL_O          0x40F
#define VK_CTL_P          0x410
#define VK_CTL_Q          0x411
#define VK_CTL_R          0x412
#define VK_CTL_S          0x413
#define VK_CTL_T          0x414
#define VK_CTL_U          0x415
#define VK_CTL_V          0x416
#define VK_CTL_W          0x417
#define VK_CTL_X          0x418
#define VK_CTL_Y          0x419
#define VK_CTL_Z          0x41A
#define VK_SF1            0x115
#define VK_SF2            0x116
#define VK_SF3            0x117
#define VK_SF4            0x118
#define VK_SF5            0x119
#define VK_SF6            0x11A
#define VK_SF7            0x11B
#define VK_SF8            0x11C
#define VK_SF9            0x11D
#define VK_SF10           0x11E
#define VK_SF11           0x11F
#define VK_SF12           0x120





#define VK_CONTROL_L      0x400
#define VK_CONTROL_R      0x400

#define VK_SHIFT_L        0x200
#define VK_SHIFT_R        0x200

#define VK_ALT_L          0x800
#define VK_ALTGR          0x800

#define VK_CAPS_LOCK     0x1100
#define VK_NUM_LOCK      0x1200
#define VK_SCROLL_LOCK   0x1300

#define VK_KP_0           0x030
#define VK_KP_1           0x031
#define VK_KP_2           0x032
#define VK_KP_3           0x033
#define VK_KP_4           0x034
#define VK_KP_5           0x035
#define VK_KP_6           0x036
#define VK_KP_7           0x037
#define VK_KP_8           0x038
#define VK_KP_9           0x039
#define VK_KP_MULT        0x02A
#define VK_KP_MINUS       0x02D
#define VK_KP_PLUS        0x02B
#define VK_KP_DIV         0x02F
#define VK_KP_DECIMAL     0x02E
#define VK_KP_ENTER       0x00A




//typedef POINT  POINTS;
typedef POINT  * PPIONTS;
typedef __int64  __off64_t;
typedef __int64  int64_t;
//typedef DWORD    pthread_t;

//inline void usleep(unsigned int uusec){Sleep(uusec/1000);};
#define    strcasestr  StrStrI
#define    strncasecmp strnicmp
#define    strcasecmp  stricmp
#define    malloc_usable_size  _msize
#endif

//int  ProcessFile(LPCSTR lpFilename,int ItemId);
//void SetStatusText(LPCSTR  pnt,int num,int style);
//void DebugMessage(LPCSTR  szmsg);

#define DEBUG_NORMAL	0
#define DEBUG_INFO	1
#define DEBUG_DEBUG	2
#ifdef WIN32
#define __PRETTY_FUNCTION__  "funct"
#define __FUNCTION__  "funct"
#endif

#define VK_KATHI_Power       0xF074
#define VK_KATHI_Ok          0xF160
#define VK_KATHI_Menu        0xF08b
#define VK_KATHI_Text        0xF184
#define VK_KATHI_Info        0xF08a
#define VK_KATHI_Epg         0xF16d
#define VK_VUPLUS_Exit       0xF0AE
#define VK_VUPLUS_Epg        0xF166
#define VK_VUPLUS_CHPLUS     0xF192
#define VK_VUPLUS_CHMINUS    0xF193
#define VK_VUPLUS_SUBITTLE   0xF172
#define VK_VUPLUS_MEDIA      0xF189
#define VK_VUPLUS_AUDIO      0xF188
#define VK_VUPLUS_TV         0xF179
#define VK_VUPLUS_RADIO      0xF181

#define VK_VUPLUS_PREV       0xF19C
#define VK_VUPLUS_PREVrel    0xF99C
#define VK_VUPLUS_NEXT       0xF197
#define VK_VUPLUS_NEXTrel    0xF997
#define VK_VUPLUS_PAUSE      0xF0A4



#define VK_KATHI_red         0xF18E
#define VK_KATHI_green       0xF18F
#define VK_KATHI_yellow      0xF190
#define VK_KATHI_blue        0xF191

#define VK_TECNISAT_WWW      0xF0
#define VK_TECNISAT_TV_RADIO 0xF09E
#define VK_TECNISAT_SFI      0xF09F
#define VK_TECNISAT_TXT      0xF0AC
#define VK_TECNISAT_PIP      0xF09B
#define VK_TECNISAT_red      0xF0
#define VK_TECNISAT_green    0xF0
#define VK_TECNISAT_yellow   0xF0
#define VK_TECNISAT_blue     0xF0
#define VK_TECNISAT_Play     0xF0
#define VK_TECNISAT_Pause    0xF0A4
#define VK_TECNISAT_Stop     0xF0A6
#define VK_TECNISAT_Record   0xF090
#define VK_TECNISAT_FastFwd  0xF0A3
#define VK_TECNISAT_FastRew  0xF0A5
#define VK_TECNISAT_VolUp    0xF073
#define VK_TECNISAT_VolDn    0xF072
#define VK_TECNISAT_Mute     0xF071


#define VK_KATHI_Play        0xF0CF
#define VK_KATHI_Pause       0xF077
#define VK_KATHI_Stop        0xF080
#define VK_KATHI_Record      0xF0A7
#define VK_KATHI_FastFwd     0xF0D0
#define VK_KATHI_FastFwdrel  0xF8D0
#define VK_KATHI_FastRew     0xF0a8
#define VK_KATHI_FastRewrel  0xF8a8

#define VK_KATHI_VolUp       0xF073
#define VK_KATHI_VolUprel    0xF873
#define VK_KATHI_VolDn       0xF072
#define VK_KATHI_VolDnrel    0xF872
#define VK_KATHI_Mute        0xF071


#ifndef WIN32
#define	VK_MUTE             0xF071
#define	VK_VOLDN            0xF072
#define	VK_VOLUP            0xF073
#define	VK_POWER            0xF074
#define	VK_PAUSE            0xF077
#define	VK_STOP             0xF080
#define	VK_INFO             0xF08a
#define	VK_MENU             0xF08b
#define	VK_RECORD           0xF0A7
#define	VK_FASTREW          0xF0a8
#define	VK_PLAY             0xF0CF
#define	VK_FASTFWD          0xF0D0
#define	VK_RED              0xF18E
#define	VK_GREEN            0xF18F
#define	VK_YELLOW           0xF190
#define	VK_BLUE             0xF191
#define	VK_CHPLUS           0xF192
#define	VK_CHMINUS          0xF193
#endif
#ifndef nsframebuffer
void setDebugLevel( int level );

//#define dprintf(debuglevel, fmt, args...) {if(debug>=debuglevel) {printf(fmt, ## args);fflush(0);}}
#define dperror(str) {perror("[wbastel] " str);}
#ifdef __cplusplus
class OutputDebugInfo
{
public:
  static int level;
  static int debug;
  OutputDebugInfo(int debuglevel,LPCSTR arg)
  {
    if(debug>=debuglevel)
    {
      for (int i=0;i < level;i++)putchar(' '),putchar(' ');
      printf("%s \n",arg);
    }
    level++;
  };
  virtual ~OutputDebugInfo()
  {
    level--;
    fflush(0);
  };
};





#endif
#endif
#endif
