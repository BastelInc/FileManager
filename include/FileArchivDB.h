// FileArchivSQL.h: Schnittstelle für die Klasse CFileArchivDB.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILEARCHIVSQL_H__2BBDBA38_A577_4B92_AC79_F55A5E11AFBE__INCLUDED_)
#define AFX_FILEARCHIVSQL_H__2BBDBA38_A577_4B92_AC79_F55A5E11AFBE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <sqlite3.h>
#include <string>
#include "basteltype.h"

extern const char g_Path[];
extern char   g_IniFile[_MAX_PATH];
#ifndef DYNARRY


#define MAX_DB_FNAME   128
typedef struct {
  int      ID;
  char 	   PathName[MAX_DB_FNAME];
  char 	   FileName[MAX_DB_FNAME];
  unsigned int dwFileAttributes;
  int      FileType;
  time_t   ftLastWriteTime[4];
  long     nFileSize[4];
//  }sz;
//  BYTE     Reserved[256-(MAX_DB_FNAME - sizeof(FILETIME ) - sizeof(DWORD)*6)];
} tFileArchivDB;

#ifdef FOLDER
typedef struct {
  DWORD    VolumeID;
  char 	   Name[MAX_DB_FNAME];
  DWORD    ID;
} tFolderDB;
#endif
typedef struct {
  int      VolumeSN;
  int      CDNummer;
  char	   Label[33];
  char	   Kommentar[128];
  DWORD    ID;
} tVolumeDB;


//-------------------------------------------------------------
//-------------------------------------------------------------
//-------------------------------------------------------------
#define  MAXBUFLEN		   1000
#define  MAX_COL             17
#define  MAXDISPLAYSIZE    1000
//-------------------------------------------------------------
// Deklaration für Datenbank

class CFileArchivDB
{
public:
  char sqlstr[512];
  char *zErrMsg;
  const char *pzTest;


  BOOL   m_NewVol;
//  DWORD  m_FilePointer;
//  DWORD  m_NextFilePointer;
//  HANDLE m_hFile;
//  char   m_path[_MAX_PATH];
//  DWORD  m_CD_VolID;
  tFileArchivDB m_Record;
#ifdef FOLDER
  tFolderDB     m_Folder;
  SQLHSTMT		hstmtFld;
#endif
  tVolumeDB     m_VolumeDB;


  time_t Datum;
  CFileArchivDB();
  virtual	   ~CFileArchivDB();
  char			szDatenbankName[MAXBUFLEN+1];
  sqlite3*   	m_db;
  sqlite3_stmt* m_hstmtRec1;
  sqlite3_stmt* m_hstmtRec2;
  sqlite3_stmt* m_hstmtRec3;
  sqlite3_stmt* m_hstmtRecInsert;
  sqlite3_stmt* m_hstmtRecUpdate;
  sqlite3_stmt* hstmtVol;
  unsigned long dwDataLen[MAX_COL];
  int           m_NumInserted;
  int           m_NumChecked;
  void			DisplayError(int nResult, sqlite3_stmt * hstmt,const char *pzTest,int iLine);
  void			CreateDatabase();

  void			FreeDB(void);
  BOOL			InitSQLTable(LPSTR lpDBStr);
  virtual BOOL	RecordExistsByPathNameVolume(int ID_Volume,time_t ftLastWriteTime,long nFileSize,BOOL Update);
  virtual BOOL	SaveRecord(int ID_Volume);
  BOOL			InsertRecord(int ID_Volume);
  BOOL			UpdateRecord(int ID_Volume,DWORD RecID);
  BOOL			UpdateRecordFileTime(int ID_Volume,DWORD RecID,time_t ftLastWriteTime,long nFileSize);
  virtual BOOL	GetRecordByRowID(DWORD RecID);
  virtual BOOL	GetRecordByPathAndName(char * lpfPath,char * lpfName);
  virtual BOOL  GetDirectoryContents(LPSTR pszDirectory,int pathlen,DWORD ID_Volume,DWORD ID_Parent);
//  BOOL			GetRecord2(DWORD RecID,BOOL bFirst,LPSTR lszCriterion);
  BOOL          CompareFile(int BasisID,char * lpfPath,char * lpfName);
#if 0
  virtual BOOL	SearchVolume(void);
  virtual BOOL	GetVolume(DWORD RecID);
  virtual BOOL	SaveVolume(void);
  virtual BOOL  ReadVolume();
#ifdef FOLDER
  virtual BOOL	SaveFolder(void);
  virtual BOOL	SearchFolder(void);
  virtual BOOL	GetNextFolder(BOOL bFirst,DWORD ID_Parent);
  virtual BOOL	GetFolder(DWORD RecID);
#endif
  virtual BOOL	GetNextVolume(BOOL bFirst);
  virtual BOOL	GetNextEntry(BOOL bFirst,int RecType,DWORD ID_Parent);
#endif
//  virtual BOOL	GetNextFileMatch(BOOL bFirst,BOOL bExact);
  void          DeleteIndex();
  void          CreateIndex();

};

extern CFileArchivDB * FileArchivDB;
#endif
#endif // !defined(AFX_FILEARCHIVSQL_H__2BBDBA38_A577_4B92_AC79_F55A5E11AFBE__INCLUDED_)
