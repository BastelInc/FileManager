// BastelUtils.h: Schnittstelle für die Klasse CBastelUtils.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BASTELUTILS_H__3E31D93E_B69E_4E2B_98C5_2584C5D53B40__INCLUDED_)
#define AFX_BASTELUTILS_H__3E31D93E_B69E_4E2B_98C5_2584C5D53B40__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "basteltype.h"

class CBastelUtils
{
public:
  static char s_IniPath [_MAX_PATH];

  char m_IniFile[_MAX_PATH];
  CBastelUtils();
  virtual ~CBastelUtils();
  void         SetIniFile(LPCSTR  lpfile);
  int          GetProfileInt(LPCSTR  appname, LPCSTR  valuename, int  uDefault);
  void         WriteProfileInt(LPCSTR  appname, LPCSTR  valuename, int  uValue);
  unsigned int GetProfileString(LPCSTR  appname, LPCSTR  valuename, LPCSTR  pDefault, LPSTR   pResult, DWORD cbResult);
  void         WriteProfileString(LPCSTR  appname, LPCSTR  valuename, LPCSTR  pData);

};
extern CBastelUtils  gBastelUtils;

#ifndef WIN32
DWORD GetTickCount(void);

#include <ctype.h>
//#define _U  0x01    // upper
//#define _L  0x02    // lower
//#define _D  0x04    // digit
//#define _C  0x08    // cntrl
//#define _P  0x10    // punct
//#define _S  0x20    // white space (space/lf/tab)
//#define _X  0x40    // hex digit
//#define _SP 0x80    // hard space (0x20)
//
//extern unsigned char _ctype[];
//#define __ismask(x) (_ctype[(int)(unsigned char)(x)])
//
//#define isalnum(c)	((__ismask(c)&(_U|_L|_D)) != 0)
//#define isalpha(c)	((__ismask(c)&(_U|_L)) != 0)
//#define iscntrl(c)	((__ismask(c)&(_C)) != 0)
//#define isdigit(c)	((__ismask(c)&(_D)) != 0)
//#define isgraph(c)	((__ismask(c)&(_P|_U|_L|_D)) != 0)
//#define islower(c)	((__ismask(c)&(_L)) != 0)
//#define isprint(c)	((__ismask(c)&(_P|_U|_L|_D|_SP)) != 0)
//#define ispunct(c)	((__ismask(c)&(_P)) != 0)
//#define isspace(c)	((__ismask(c)&(_S)) != 0)
//#define isupper(c)	((__ismask(c)&(_U)) != 0)
//#define isxdigit(c)	((__ismask(c)&(_D|_X)) != 0)
//
//#ifndef isascii
//#define isascii(c) (((unsigned char)(c))<=0x7f)
//#define toascii(c) (((unsigned char)(c))&0x7f)
//#endif



#endif

#define xWriteProfileString gBastelUtils.WriteProfileString
#define xGetProfileString   gBastelUtils.GetProfileString

#endif // !defined(AFX_BASTELUTILS_H__3E31D93E_B69E_4E2B_98C5_2584C5D53B40__INCLUDED_)
