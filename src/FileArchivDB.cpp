//  This file is part of the Bastel project. It is distributed under
//  GNU General Public License: https://www.gnu.org/licenses/gpl-3.0
//  Copyright (C) Bastel.Inc - All Rights Reserved

// FileArchivSQL.cpp: Implementierung der Klasse CFileArchivDB.

#include "basteltype.h"
#include "FileTable.h"
#include "FileArchivDB.h"
#ifndef WIN32
#include <dirent.h>
#endif

//char    g_IniFile[_MAX_PATH];
DWORD   NewDateienID=-1;
//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

#undef DBINDEX
//-------------------------------------------------------------------
void CFileArchivDB::DisplayError(int nResult, sqlite3_stmt * hstmt,const char *pzTest,int iLine)
{
  static int numError=0;
  numError++;
  if (numError<10) {
    fprintf(stderr, "Line %d ", iLine);
    fprintf(stderr, "SQL error: %s\n", pzTest);
  } else if (numError==10) {
    fprintf(stderr, "MORE Then %d SQL error: %s\n",numError, pzTest);
  }
}

//-------------------------------------------------------------
CFileArchivDB::CFileArchivDB()
{
  m_db =NULL;
  m_hstmtRec1=NULL;
  m_hstmtRec2=NULL;
  m_hstmtRec3=NULL;
  m_hstmtRecInsert=NULL;
  m_hstmtRecUpdate=NULL;
  hstmtVol=NULL;
#ifdef FOLDER
  hstmtFld=NULL;
#endif
  m_NumInserted=0;
  m_NumChecked=0;
  m_NewVol = 0;
}
//-------------------------------------------------------------
CFileArchivDB::~CFileArchivDB()
{
  FreeDB();
}
//-------------------------------------------------------------
void CFileArchivDB::CreateDatabase()
{
//  nReturn = SQLExecDirect(m_hstmtRec2, (SQLCHAR*)"DROP TABLE Dateien", SQL_NTS);
  int drop = 1;
  char DateienCreateStatement[] =
    "CREATE TABLE IF NOT EXISTS Dateien("
//    "ID COUNTER CONSTRAINT PrimaryKey PRIMARY KEY,"
    "PathName TEXT (128),"
    "FileName TEXT (128),"
    "dwFileAttributes LONG,"
    "FileType LONG,"
    "ftLastWriteTime0 TIMESTAMP,"
    "ftLastWriteTime1 TIMESTAMP,"
    "ftLastWriteTime2 TIMESTAMP,"
    "ftLastWriteTime3 TIMESTAMP,"
    "nFileSize0 LONG,"
    "nFileSize1 LONG,"
    "nFileSize2 LONG,"
    "nFileSize3 LONG)";


#ifdef FOLDER
//  nReturn = SQLExecDirect(m_hstmtRec2, (SQLCHAR*)"DROP TABLE Folders ", SQL_NTS);
  char FoldersCreateStatement[] =
    "CREATE TABLE IF NOT EXISTS Folders ("
//    "ID COUNTER CONSTRAINT PrimaryKey PRIMARY KEY,"
    "VolumeID LONG,"
    "Name TEXT (128))";
#endif

  //nReturn = SQLExecDirect(m_hstmtRec2, (SQLCHAR*)"DROP TABLE Volumes", SQL_NTS);
  char VolumesCreateStatement[] =
    "CREATE TABLE IF NOT EXISTS Volumes ("
//    "ID COUNTER CONSTRAINT PrimaryKey PRIMARY KEY,"
    "Label TEXT (33),"
    "Kommentar TEXT (128),"
    "VolumeSN LONG,"
    "CDNummer LONG)";




  int rc;
  sqlite3_stmt * statement;
  if (drop ) {
    rc = sqlite3_prepare_v2(m_db, "DROP TABLE Dateien", strlen("DROP TABLE Dateien"), &statement, &pzTest);
    sqlite3_step(statement);
    sqlite3_finalize(statement);
#ifdef FOLDER
    rc = sqlite3_prepare_v2(m_db, "DROP TABLE Folders", strlen("DROP TABLE Folders"), &statement, &pzTest);
    sqlite3_step(statement);
    sqlite3_finalize(statement);
#endif
    rc = sqlite3_prepare_v2(m_db, "DROP TABLE Volumes", strlen("DROP TABLE Volumes"), &statement, &pzTest);
    sqlite3_step(statement);
    sqlite3_finalize(statement);
  }
  m_NewVol = 1;

  rc = sqlite3_prepare_v2(m_db, DateienCreateStatement, strlen(DateienCreateStatement), &statement, &pzTest);
  sqlite3_step(statement);
  sqlite3_finalize(statement);
#ifdef FOLDER
  rc = sqlite3_prepare_v2(m_db, FoldersCreateStatement, strlen(FoldersCreateStatement), &statement, &pzTest);
  sqlite3_step(statement);
  sqlite3_finalize(statement);
#endif
  rc = sqlite3_prepare_v2(m_db, VolumesCreateStatement, strlen(VolumesCreateStatement), &statement, &pzTest);
  sqlite3_step(statement);
  sqlite3_finalize(statement);

  CreateIndex();

  if( rc!=SQLITE_OK ) {
    DisplayError(rc,statement,pzTest,__LINE__);
  }
}
//-------------------------------------------------------------
void CFileArchivDB::DeleteIndex()
{
  int rc;
  sqlite3_stmt * statement;
  const char DateienIndex[] =  "DROP INDEX PathName_ind ON Dateien";
  rc = sqlite3_prepare_v2(m_db, DateienIndex, strlen(DateienIndex), &statement, &pzTest);
  rc =sqlite3_step(statement);
  sqlite3_finalize(statement);
}
//-------------------------------------------------------------
void CFileArchivDB::CreateIndex()
{
  int rc;
  sqlite3_stmt * statement;
  const char DateienIndex[] =  "CREATE INDEX PathName_ind ON Dateien (PathName ASC)";
  rc = sqlite3_prepare_v2(m_db, DateienIndex, strlen(DateienIndex), &statement, &pzTest);
  rc =sqlite3_step(statement);
  sqlite3_finalize(statement);
}
//-------------------------------------------------------------------
void CFileArchivDB::FreeDB(void)
{
  if (m_hstmtRec1) {
    sqlite3_finalize(m_hstmtRec1);
    m_hstmtRec1 = NULL;
  }
  if (m_hstmtRec2) {
    sqlite3_finalize(m_hstmtRec2);
    m_hstmtRec2= NULL;
  }
  if (m_hstmtRec3) {
    sqlite3_finalize(m_hstmtRec3);
    m_hstmtRec3= NULL;
  }
  if (m_hstmtRecInsert) {
    sqlite3_finalize(m_hstmtRecInsert);
    m_hstmtRecInsert= NULL;
  }
  if (m_hstmtRecUpdate) {
    sqlite3_finalize(m_hstmtRecUpdate);
    m_hstmtRecUpdate= NULL;
  }

  if (m_db) {
    sqlite3_close(m_db);
    m_db = NULL;
  }
}
//-------------------------------------------------------------------
BOOL CFileArchivDB::InitSQLTable(LPSTR lpDBStr)
{
  if (m_db) {
    FreeDB();
  }
  sqlite3_stmt* hstmt;

  int rc = sqlite3_open(lpDBStr, &m_db);
  if( rc ) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(m_db));
    return(0);
  } else {
//    DELETE    TRUNCATE    PERSIST    MEMORY    WAL   OFF
    sprintf(sqlstr,"PRAGMA journal_mode=OFF");
    int rc = sqlite3_prepare_v2(m_db, sqlstr, strlen(sqlstr), &hstmt, &pzTest);
    rc = sqlite3_step(hstmt);
    if( rc == SQLITE_DONE) {
    }
    rc = sqlite3_finalize(hstmt);


    sprintf(sqlstr,"SELECT _rowid_ FROM Dateien");
    rc = sqlite3_prepare_v2(m_db, sqlstr, strlen(sqlstr), &hstmt, &pzTest);
    rc = sqlite3_step(hstmt);
    if( rc == SQLITE_ROW) {
      int ID = sqlite3_column_int(hstmt,0);
      sqlite3_finalize(hstmt);
    } else {
      sqlite3_finalize(hstmt);
      if( rc!=SQLITE_OK ) {
        CreateDatabase();
      }
    }
    return(m_db != NULL);
  }
  return 0;
}
//-------------------------------------------------------------
//-------------------------------------------------------------
BOOL CFileArchivDB::GetRecordByRowID(DWORD RecID)
{
  sprintf(sqlstr, "SELECT _rowid_,PathName,FileName,dwFileAttributes,FileType,ftLastWriteTime0,nFileSize0,ftLastWriteTime1,nFileSize1,ftLastWriteTime2,nFileSize2,ftLastWriteTime3,nFileSize3 FROM Dateien WHERE _rowid_ = %d",RecID);
  int rc = sqlite3_prepare_v2(m_db, sqlstr, strlen(sqlstr), &m_hstmtRec1, &pzTest);

  if( rc!=SQLITE_OK ) {
    DisplayError(rc,m_hstmtRec1,pzTest,__LINE__);
    return 0;
  }

  rc = sqlite3_step(m_hstmtRec1);
  if( rc == SQLITE_ROW) {
    const unsigned char * pnt;
    m_Record.ID = sqlite3_column_int(m_hstmtRec2,0);
    pnt = sqlite3_column_text(m_hstmtRec2,  1);
    if (pnt) strncpy(m_Record.PathName,(char*)pnt,sizeof(m_Record.PathName));
    pnt = sqlite3_column_text(m_hstmtRec2,  2);
    if (pnt) strncpy(m_Record.FileName,(char*)pnt,sizeof(m_Record.FileName));
    m_Record.dwFileAttributes = sqlite3_column_int(m_hstmtRec2,3);
    m_Record.FileType = sqlite3_column_int(m_hstmtRec2,4);
    m_Record.ftLastWriteTime[0] = sqlite3_column_int(m_hstmtRec2,5);
    m_Record.nFileSize[0] = sqlite3_column_int(m_hstmtRec2,6);
    m_Record.ftLastWriteTime[1] = sqlite3_column_int(m_hstmtRec2,7);
    m_Record.nFileSize[1] = sqlite3_column_int(m_hstmtRec2,8);
    m_Record.ftLastWriteTime[2] = sqlite3_column_int(m_hstmtRec2,9);
    m_Record.nFileSize[2] = sqlite3_column_int(m_hstmtRec2,10);
    m_Record.ftLastWriteTime[3] = sqlite3_column_int(m_hstmtRec2,5);
    m_Record.nFileSize[3] = sqlite3_column_int(m_hstmtRec2,11);
    sqlite3_finalize(m_hstmtRec1);
    m_hstmtRec1=NULL;
    return 1;
  } else {
    sqlite3_finalize(m_hstmtRec1);
    m_hstmtRec1=NULL;
  }
  return 0;
}
//-------------------------------------------------------------

BOOL CFileArchivDB::RecordExistsByPathNameVolume(int ID_Volume,time_t ftLastWriteTime,long nFileSize,BOOL Update)
{
  if (m_NewVol) return 0;
  sprintf(sqlstr, "SELECT _rowid_ ,ftLastWriteTime0,nFileSize0,ftLastWriteTime1,nFileSize1,ftLastWriteTime2,nFileSize2,ftLastWriteTime3,nFileSize3 FROM Dateien WHERE" \
          " PathName = \'%s\' AND FileName = \'%s\' AND FileType = %d",
          m_Record.PathName,
          m_Record.FileName,
          m_Record.FileType);

  if (m_hstmtRec3) sqlite3_finalize(m_hstmtRec3);
  m_hstmtRec3=NULL;

  int rc = sqlite3_prepare_v2(m_db, sqlstr, strlen(sqlstr), &m_hstmtRec3, &pzTest);
  char * p = sqlstr;
  if( rc!=SQLITE_OK ) {
    DisplayError(rc,m_hstmtRec3,pzTest,__LINE__);
    return 0;
  }
  m_Record.ID=-1;

  // Get Time and Size
  rc = sqlite3_step(m_hstmtRec3);
  if( rc == SQLITE_ROW) {
    m_Record.ID = sqlite3_column_int(m_hstmtRec3,0);
    m_Record.ftLastWriteTime[0] = sqlite3_column_int(m_hstmtRec3,1);
    m_Record.nFileSize[0] = sqlite3_column_int(m_hstmtRec3,2);
    m_Record.ftLastWriteTime[1] = sqlite3_column_int(m_hstmtRec3,3);
    m_Record.nFileSize[1] = sqlite3_column_int(m_hstmtRec3,4);
    m_Record.ftLastWriteTime[2] = sqlite3_column_int(m_hstmtRec3,5);
    m_Record.nFileSize[2] = sqlite3_column_int(m_hstmtRec3,6);
    m_Record.ftLastWriteTime[3] = sqlite3_column_int(m_hstmtRec3,7);
    m_Record.nFileSize[3] = sqlite3_column_int(m_hstmtRec3,8);
  } else {
    m_Record.ID=-1;
  }

  sqlite3_finalize(m_hstmtRec3);
  m_hstmtRec3=NULL;

  m_NumChecked++;
  if (m_Record.ID>=0) {
    if (Update) {
      if (ftLastWriteTime != m_Record.ftLastWriteTime[ID_Volume]
          || nFileSize != m_Record.nFileSize[ID_Volume]) {
        m_Record.ftLastWriteTime[ID_Volume]=ftLastWriteTime;
        m_Record.nFileSize[ID_Volume] = nFileSize;
        sprintf(sqlstr,"UPDATE Dateien SET"
                " ftLastWriteTime%d = %ld,nFileSize%d = %ld"
                " WHERE _rowid_ = %d",
                ID_Volume,
                m_Record.ftLastWriteTime[ID_Volume],
                ID_Volume,
                m_Record.nFileSize[ID_Volume],
                m_Record.ID);
        rc = sqlite3_prepare_v2(m_db, sqlstr, strlen(sqlstr), &m_hstmtRecUpdate, &pzTest);

        if( rc!=SQLITE_OK ) {
          DisplayError(rc,m_hstmtRecUpdate,pzTest,__LINE__);
        }
        rc = sqlite3_step(m_hstmtRecUpdate);
        if( rc!=SQLITE_DONE ) {
          DisplayError(rc,m_hstmtRecUpdate,pzTest,__LINE__);
        }
        sqlite3_finalize(m_hstmtRecUpdate);
        m_hstmtRecUpdate=NULL;
      }
    }
    return 1;
  } else return false;
}
//-------------------------------------------------------------
BOOL CFileArchivDB::GetRecordByPathAndName(char * lpfPath,char * lpfName)
{
  int rc =0;
  if (m_hstmtRec2) sqlite3_finalize(m_hstmtRec2);
  m_hstmtRec2 = NULL;
  sprintf(sqlstr, "SELECT _rowid_,dwFileAttributes,FileType,"\
          "ftLastWriteTime0,nFileSize0,ftLastWriteTime1,nFileSize1,ftLastWriteTime2,nFileSize2,ftLastWriteTime3,nFileSize3"\
          " FROM Dateien WHERE (PathName = '%s/' AND FileName = '%s')",lpfPath,lpfName);
  rc = sqlite3_prepare_v2(m_db, sqlstr, strlen(sqlstr), &m_hstmtRec2, &pzTest);

  if( rc!=SQLITE_OK ) {
    DisplayError(rc,m_hstmtRec2,pzTest,__LINE__);
  }

  memset(&m_Record,0,sizeof(m_Record));
  rc = sqlite3_step(m_hstmtRec2);
  if( rc == SQLITE_ROW) {
    strncpy(m_Record.PathName,lpfPath,sizeof(m_Record.PathName));
    strncpy(m_Record.FileName,lpfName,sizeof(m_Record.FileName));

    m_Record.ID = sqlite3_column_int(m_hstmtRec2,0);
    m_Record.dwFileAttributes = sqlite3_column_int(m_hstmtRec2,1);
    m_Record.FileType = sqlite3_column_int(m_hstmtRec2,2);
    m_Record.ftLastWriteTime[0] = sqlite3_column_int(m_hstmtRec2,3);
    m_Record.nFileSize[0] = sqlite3_column_int(m_hstmtRec2,4);
    m_Record.ftLastWriteTime[1] = sqlite3_column_int(m_hstmtRec2,5);
    m_Record.nFileSize[1] = sqlite3_column_int(m_hstmtRec2,6);
    m_Record.ftLastWriteTime[2] = sqlite3_column_int(m_hstmtRec2,7);
    m_Record.nFileSize[2] = sqlite3_column_int(m_hstmtRec2,8);
    m_Record.ftLastWriteTime[3] = sqlite3_column_int(m_hstmtRec2,9);
    m_Record.nFileSize[3] = sqlite3_column_int(m_hstmtRec2,10);
    return 1;
  }
  return 0;
}

//-------------------------------------------------------------
BOOL CFileArchivDB::InsertRecord(int ID_Volume)
{
  static int sVolume=-1;
  int rc=1;
  if (NewDateienID== -1) {
    if (m_hstmtRec2) sqlite3_finalize(m_hstmtRec2);
    m_hstmtRec2 = NULL;
    //sprintf(sqlstr, "SELECT ID FROM Dateien ORDER BY ID DESC");
    sprintf(sqlstr, "SELECT _rowid_ FROM Dateien ORDER BY _rowid_ DESC");
    int rc = sqlite3_prepare_v2(m_db, sqlstr, strlen(sqlstr), &m_hstmtRec2, &pzTest);

    if( rc!=SQLITE_OK ) {
      DisplayError(rc,m_hstmtRec2,pzTest,__LINE__);
    }

    rc = sqlite3_step(m_hstmtRec2);
    if( rc == SQLITE_ROW) {
      DWORD New_ID = sqlite3_column_int(m_hstmtRec2,0);
      NewDateienID = New_ID;
    }
  }
  NewDateienID++;
  m_Record.ID = NewDateienID;

  if (m_hstmtRecInsert==NULL || sVolume != ID_Volume) {
    if (m_hstmtRecInsert) sqlite3_finalize(m_hstmtRecInsert);
    sVolume = ID_Volume;
    sprintf(sqlstr,"INSERT INTO Dateien (PathName,FileName,dwFileAttributes,FileType,ftLastWriteTime%d,nFileSize%d) "\
            "VALUES (?,?,?,?,?,?)",ID_Volume,ID_Volume);
    rc = sqlite3_prepare_v2(m_db, sqlstr, strlen(sqlstr), &m_hstmtRecInsert, &pzTest);
    if( rc!=SQLITE_OK ) {
      DisplayError(rc,m_hstmtRecInsert,pzTest,__LINE__);
    }
  }
  sqlite3_bind_text (m_hstmtRecInsert, 1,            m_Record.PathName,strlen(m_Record.PathName),0);
  sqlite3_bind_text (m_hstmtRecInsert, 2,            m_Record.FileName,strlen(m_Record.FileName),0);
  sqlite3_bind_int  (m_hstmtRecInsert, 3,            m_Record.dwFileAttributes);
  sqlite3_bind_int  (m_hstmtRecInsert, 4,            m_Record.FileType);
  sqlite3_bind_int  (m_hstmtRecInsert, 5,            m_Record.ftLastWriteTime[ID_Volume]);
  sqlite3_bind_int  (m_hstmtRecInsert, 6,            m_Record.nFileSize[ID_Volume]);


  rc = sqlite3_step(m_hstmtRecInsert);
  if( rc!=SQLITE_DONE) {
    DisplayError(rc,m_hstmtRecInsert,pzTest,__LINE__);
  }
  sqlite3_reset(m_hstmtRecInsert);
  m_NumInserted++;
  return rc==SQLITE_OK;
}

//-------------------------------------------------------------------
BOOL CFileArchivDB::UpdateRecord(int ID_Volume,DWORD RecID)
{
  int rc=1;
  sprintf(sqlstr,"UPDATE Dateien SET"
          " PathName = %s,FileName = %s,dwFileAttributes = %d,FileType = %d,"
          " ftLastWriteTime%d = %ld,nFileSize%d = %ld"
          " WHERE _rowid_ = %d",
          m_Record.PathName,
          m_Record.FileName,
          m_Record.dwFileAttributes,
          m_Record.FileType,
          ID_Volume,
          m_Record.ftLastWriteTime[ID_Volume],
          ID_Volume,
          m_Record.nFileSize[ID_Volume],
          RecID);
  rc = sqlite3_prepare_v2(m_db, sqlstr, strlen(sqlstr), &m_hstmtRecUpdate, &pzTest);

  if( rc!=SQLITE_OK ) {
    DisplayError(rc,m_hstmtRecUpdate,pzTest,__LINE__);
    return 0;
  }
  rc = sqlite3_step(m_hstmtRecUpdate);
  if( rc!=SQLITE_OK ) {
    DisplayError(rc,m_hstmtRecUpdate,pzTest,__LINE__);
  }
  sqlite3_finalize(m_hstmtRecUpdate);
  m_hstmtRecUpdate=NULL;

  return rc==SQLITE_OK;
}

//-------------------------------------------------------------------
BOOL CFileArchivDB::SaveRecord(int ID_Volume)
{
  BOOL  bFound = false;
  if (!m_NewVol && m_Record.ID) {
    tFileArchivDB Save_Record;
    Save_Record = m_Record;
    bFound      = GetRecordByRowID(m_Record.ID);
    m_Record    = Save_Record;
  }
  if (bFound) {
    return UpdateRecord(ID_Volume,m_Record.ID);
  }  else {
    return InsertRecord(ID_Volume);
  }
  return 0;
}

#if 0
//-------------------------------------------------------------
BOOL CFileArchivDB::ReadVolume()
{
  TCHAR str2[_MAX_PATH];
  TCHAR str3[_MAX_PATH];
  DWORD VolumeSerialNumber;  // address of volume serial number
  DWORD MaximumComponentLength;
  DWORD FileSystemFlags;
  m_NumInserted=0;
  m_NumChecked=0;
  if (GetVolumeInformation(g_Path,str2,MAX_DB_FNAME,
                           &VolumeSerialNumber,
                           &MaximumComponentLength,
                           &FileSystemFlags,
                           str3,sizeof(str3))) {

    memset(&m_VolumeDB,0,sizeof(m_VolumeDB));
    //m_CD_VolID = VolumeSerialNumber;
    if (VolumeSerialNumber) m_VolumeDB.VolumeSN = VolumeSerialNumber;
    else memcpy(&m_VolumeDB.VolumeSN,str2,sizeof(m_VolumeDB.VolumeSN));
    strcpy(m_VolumeDB.Label,str2);
    tVolumeDB Save_Record;
    Save_Record = m_VolumeDB;
    DWORD StartZeit = GetTickCount();
    if (SearchVolume()==0) {
      m_NewVol = true;
      m_VolumeDB = Save_Record;
      SaveVolume();
    }
    m_CD_VolID = m_VolumeDB.ID;
    strcpy(str3,g_Path);
    strcat(str3, "\\*.*");
    if (m_NewVol ) {
      HCURSOR hCursor = SetCursor(LoadCursor(NULL,IDC_WAIT));
      DeleteIndex();
      GetDirectoryContents(str3,m_VolumeDB.ID,0);
      DWORD FertigZeit = GetTickCount();

      sprintf(str3,"Read %s \nNum Checked = %d\nNum Inserted = %d\nDuration = %2.1f",str2,m_NumChecked,m_NumInserted,
              (FertigZeit  - StartZeit)/1000.0);
      if (m_NewVol )CreateIndex();
      m_NewVol = false;
      SetCursor(hCursor );
      MessageBox(g_hWndMain,str3,"Result",MB_OK);
    }
    return 1;
  }
  return 0;
}
//-------------------------------------------------------------
//-------------------------------------------------------------
BOOL CFileArchivDB::SearchVolume(void)
{
  SQLRETURN nReturn = SQL_SUCCESS;
  char szBuffer[MAXBUFLEN+1];
  SQLCloseCursor(hstmtVol);
  wsprintf(szBuffer, "SELECT _rowid_ FROM Volumes  WHERE VolumeSN = ? AND Label = ?");

  dwDataLen[1]= SQL_NTS;
  nReturn=SQLExecDirect(hstmtVol, (SQLCHAR*)szBuffer, SQL_NTS);
  nReturn = SQLFetch(hstmtVol);
  if (nReturn == SQL_NO_DATA) return 0;
  DWORD New_ID=0;
  nReturn = SQLGetData(hstmtVol,1,SQL_C_ULONG,&New_ID,sizeof(New_ID),NULL);
  if (nReturn==SQL_ERROR) {
    DisplayError(nReturn,g_hWndMain, SQL_HANDLE_STMT, hstmtVol);
  }
  SQLCloseCursor(hstmtVol);
  m_NumChecked++;
  if (New_ID) return GetVolume(New_ID);
  else return false;
  return 0;
}
//-------------------------------------------------------------
BOOL CFileArchivDB::GetVolume(DWORD RecID)
{
  SQLRETURN nReturn = SQL_SUCCESS;
  char szBuffer[MAXBUFLEN+1];
  SQLCloseCursor(hstmtVol);
  wsprintf(szBuffer, "SELECT _rowid_,VolumeSN,Label,CDNummer,Kommentar FROM Volumes WHERE _rowid_ = %d",RecID);
  nReturn=SQLExecDirect(hstmtVol, (SQLCHAR*)szBuffer, SQL_NTS);
  nReturn = SQLFetch(hstmtVol);

  if (nReturn == SQL_NO_DATA) return 0;
  return (nReturn == SQL_SUCCESS);
}

//-------------------------------------------------------------
BOOL CFileArchivDB::GetNextVolume(BOOL bFirst)
{
  SQLRETURN nReturn = SQL_SUCCESS;
  if (bFirst) {
    char szBuffer[MAXBUFLEN+1];
    SQLCloseCursor(hstmtVol);
    wsprintf(szBuffer, "SELECT _rowid_,VolumeSN,Label,CDNummer,Kommentar FROM Volumes");
    nReturn=SQLExecDirect(hstmtVol, (SQLCHAR*)szBuffer, SQL_NTS);
  }
  nReturn = SQLFetch(hstmtVol);
  if (nReturn==SQL_ERROR) {
    DisplayError(nReturn,g_hWndMain, SQL_HANDLE_STMT, hstmtVol);
  }
  if (nReturn == SQL_NO_DATA) return 0;
  return (nReturn == SQL_SUCCESS);
}

//-------------------------------------------------------------
BOOL CFileArchivDB::SaveVolume(void)
{
  SQLRETURN nReturn = SQL_SUCCESS;
  char szBuffer[MAXBUFLEN+1];
  SQLCloseCursor(hstmtVol);
  dwDataLen[1]= SQL_NTS;
  dwDataLen[3]= SQL_NTS;
  if (m_VolumeDB.ID==0) {
    wsprintf(szBuffer,"INSERT INTO Volumes (VolumeSN,Label,CDNummer,Kommentar) VALUES (?,?,?,?)");
    nReturn = SQLExecDirect(hstmtVol, (SQLCHAR*)&szBuffer,SQL_NTS);
    SQLCloseCursor(hstmtVol);
    wsprintf(szBuffer, "SELECT _rowid_ FROM Volumes WHERE VolumeSN = ? AND Label = ? AND CDNummer = ?");
    nReturn=SQLExecDirect(hstmtVol, (SQLCHAR*)szBuffer, SQL_NTS);
    nReturn = SQLFetch(hstmtVol);
    DWORD New_ID;
    nReturn = SQLGetData(hstmtVol,1,SQL_C_ULONG,&New_ID,sizeof(New_ID),NULL);
    SQLCloseCursor(hstmtVol);
    m_NumInserted++;
  } else {
    wsprintf(szBuffer,"UPDATE Volumes SET VolumeSN = ?,Label = ?,CDNummer = ?,Kommentar = ? WHERE _rowid_ = ?");
    nReturn=SQLExecDirect(hstmtVol, (SQLCHAR*)szBuffer, SQL_NTS);
  }
  if (nReturn==SQL_ERROR) {
    DisplayError(nReturn,g_hWndMain, SQL_HANDLE_STMT, hstmtVol);
  }
  return nReturn==SQL_SUCCESS;
}
//-------------------------------------------------------------
#ifdef FOLDER
BOOL CFileArchivDB::SaveFolder(void)
{
  SQLRETURN nReturn = SQL_SUCCESS;
  char szBuffer[MAXBUFLEN+1];
  SQLCloseCursor(hstmtFld);
  dwDataLen[1]= SQL_NTS;
  dwDataLen[3]= SQL_NTS;
  if (m_Folder.ID==0) {
    wsprintf(szBuffer,"INSERT INTO Folders (VolumeID,Name) VALUES (?,?)");
    nReturn = SQLExecDirect(hstmtFld, (SQLCHAR*)&szBuffer,SQL_NTS);
    SQLCloseCursor(hstmtFld);
    wsprintf(szBuffer, "SELECT _rowid_ FROM Folders WHERE VolumeID = ? AND Name = ?");
    nReturn=SQLExecDirect(hstmtFld, (SQLCHAR*)szBuffer, SQL_NTS);
    nReturn = SQLFetch(hstmtFld);
    DWORD New_ID;
    nReturn = SQLGetData(hstmtFld,1,SQL_C_ULONG,&New_ID,sizeof(New_ID),NULL);
    SQLCloseCursor(hstmtFld);
    m_NumInserted++;
  } else {
    wsprintf(szBuffer,"UPDATE Folders SET VolumeID = ?,Name = ? WHERE _rowid_ = %d",m_Folder.ID);
    nReturn=SQLExecDirect(hstmtFld, (SQLCHAR*)szBuffer, SQL_NTS);
  }
  if (nReturn==SQL_ERROR) {
    DisplayError(nReturn,g_hWndMain, SQL_HANDLE_STMT, hstmtFld);
  }
  return nReturn==SQL_SUCCESS;
}
//-------------------------------------------------------------
BOOL CFileArchivDB::SearchFolder(void)
{
  SQLRETURN nReturn = SQL_SUCCESS;
  char szBuffer[MAXBUFLEN+1];
  SQLCloseCursor(hstmtFld);
  wsprintf(szBuffer, "SELECT _rowid_ FROM Folders  WHERE VolumeID = ? AND Name = ?");

  dwDataLen[1]= SQL_NTS;
  nReturn=SQLExecDirect(hstmtFld, (SQLCHAR*)szBuffer, SQL_NTS);
  nReturn = SQLFetch(hstmtFld);
  if (nReturn == SQL_NO_DATA) return 0;
  DWORD New_ID=0;
  nReturn = SQLGetData(hstmtFld,1,SQL_C_ULONG,&New_ID,sizeof(New_ID),NULL);
  if (nReturn==SQL_ERROR) {
    DisplayError(nReturn,g_hWndMain, SQL_HANDLE_STMT, hstmtFld);
  }
  SQLCloseCursor(hstmtFld);
  m_NumChecked++;
  if (New_ID) return GetFolder(New_ID);
  else return false;
  return 0;
}

//-------------------------------------------------------------
BOOL CFileArchivDB::GetNextFolder(BOOL bFirst,DWORD ID_Parent)
{
  SQLRETURN nReturn = SQL_SUCCESS;
  if (bFirst) {
    char szBuffer[MAXBUFLEN+1];
    SQLCloseCursor(hstmtFld);
    wsprintf(szBuffer, "SELECT _rowid_,VolumeID,NAME FROM Folders");
    nReturn=SQLExecDirect(hstmtFld, (SQLCHAR*)szBuffer, SQL_NTS);
  }
  nReturn = SQLFetch(hstmtFld);
  if (nReturn==SQL_ERROR) {
    DisplayError(nReturn,g_hWndMain, SQL_HANDLE_STMT, hstmtFld);
  }
  if (nReturn == SQL_NO_DATA) return 0;
  return (nReturn == SQL_SUCCESS);
}

//-------------------------------------------------------------
BOOL CFileArchivDB::GetFolder(DWORD RecID)
{
  SQLRETURN nReturn = SQL_SUCCESS;
  char szBuffer[MAXBUFLEN+1];
  SQLCloseCursor(hstmtFld);
  wsprintf(szBuffer, "SELECT _rowid_,VolumeID,Name FROM Folders WHERE _rowid_ = %d",RecID);
  nReturn=SQLExecDirect(hstmtFld, (SQLCHAR*)szBuffer, SQL_NTS);
  nReturn = SQLFetch(hstmtFld);

  if (nReturn == SQL_NO_DATA) return 0;
  return (nReturn == SQL_SUCCESS);
}
#endif
//-------------------------------------------------------------
//-------------------------------------------------------------
BOOL CFileArchivDB::GetNextEntry(BOOL bFirst,int RecType,DWORD ID_Parent)
{
  if (bFirst) {
    char str[256];
    if (RecType==0) wsprintf(str," WHERE ParentID = %d",ID_Parent);
    if (RecType==1) wsprintf(str," WHERE FileType = %d AND ParentID = 0",ID_Parent);
    return GetRecord2(0,1,str);
  } else return GetRecord2(0,0,0);
}
#endif
//-------------------------------------------------------------
/*BOOL CFileArchivDB::GetNextFileMatch(BOOL bFirst,BOOL bExact)
{
  if (bFirst) {
    char str[512];
    if (bExact)	sprintf(str," WHERE (PathName = '%s/' AND FileName = '%s')",m_Record.PathName,m_Record.FileName);
    else sprintf(str," WHERE (PathName = '%s/' AND FileName >= '%s' AND FileName <= '%sx') ",m_Record.PathName,m_Record.FileName,m_Record.FileName);
    return GetRecord2(0,1,str);
  } else return GetRecord2(0,0,0);

  return 0;
}*/
//-------------------------------------------------------------
//------------------------------------------------------------
BOOL CFileArchivDB::GetDirectoryContents(LPSTR pszDirectory,int pathlen,DWORD ID_Volume,DWORD ID_FileType)
{
#ifdef WIN32
  WIN32_FIND_DATA findData;
  HANDLE fileHandle;
  TCHAR  str[_MAX_PATH];
  // Get the first file in the directory
  BOOL FindF=true;
  BOOL bFirstRun=true;
  int  NumFiles=0;
  // Loop on all remaining entries in the directory
  while (FindF) { // && Abbruch==false)
    if (bFirstRun) {
      bFirstRun = false;
      fileHandle = FindFirstFile(pszDirectory, &findData);
      FindF      = (fileHandle != INVALID_HANDLE_VALUE);
    } else {
      FindF  = FindNextFile(fileHandle, &findData);
    }
    //-------------- Eintrag gefunden ? -------------
    if (FindF) {
      if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        if (lstrcmpi(findData.cFileName, TEXT("."))!=0
            && lstrcmpi(findData.cFileName, TEXT(".."))!=0) {
          /*          memset(&m_Record,0,sizeof(m_Record));
                    m_Record.dwFileAttributes = findData.dwFileAttributes;
                    m_Record.ftLastWriteTime  = findData.ftLastWriteTime;
                    m_Record.nFileSize        = findData.nFileSizeLow;
                    m_Record.ParentID		    = ID_Parent;
                    m_Record.FileType         = ID_Volume;
                    strcpy(m_Record.FileName,findData.cFileName);

                    if (m_NewVol||SearchRecord()==0) SaveRecord();

                    strcpy(str,pszDirectory);
                    int len   = strlen(str);
                    if (len>3) str[len-3]='\0';
                    strcat(str,findData.cFileName);
          #ifdef FOLDER
                    m_Folder.ID = 0;
                    m_Folder.FileType         = ID_Volume;
                    strncpy(m_Folder.Name,&str[2],sizeof(m_Folder.Name));
                    if (m_NewVol||SearchFolder()) SaveFolder();
          #endif
           */
          strcat(str,"\\*.*");
          //ifEscape();
          GetDirectoryContents(str,pathlen,ID_Volume,ID_FileType);//m_Folder.ID);
          NumFiles++;
        }
      } else {
        ;
        memset(&m_Record,0,sizeof(m_Record));
        m_Record.dwFileAttributes = findData.dwFileAttributes;
        m_Record.ftLastWriteTime [ID_Volume] = findData.ftLastWriteTime.dwLowDateTime;
        m_Record.nFileSize [ID_Volume]       = findData.nFileSizeLow;
//        m_Record.ParentID		  = ID_Parent;
        m_Record.FileType         = ID_Volume;

//        if (m_NewVol||SearchRecord()==0) SaveRecord();
        strcpy(m_Record.PathName,&pszDirectory[pathlen+1]);
        strcpy(m_Record.FileName,findData.cFileName);

        if (m_NewVol||RecordExistsByPathNameVolume(ID_Volume,findData.ftLastWriteTime.dwLowDateTime,findData.nFileSizeLow,1)==0) SaveRecord(ID_Volume);
        NumFiles++;
      }
    }
  }

  {
    if (fileHandle != INVALID_HANDLE_VALUE )
      FindClose(fileHandle);
  }

  return (NumFiles!=0);
#else
  int  Image;
  int  NumFiles=0;
  DIR  *dir;

  char str[MAX_PATH];


  dir = opendir(pszDirectory);
  if (dir) {
    struct dirent *pEntry;
    size_t path_len;//, str_size;

    path_len = strlen(pszDirectory);
    strcat(pszDirectory,"/");
    strcpy(str, pszDirectory);
    path_len++;

    while ((pEntry = readdir (dir)) != NULL) {
      struct stat statbuffer;

      strcpy(&str[path_len], pEntry->d_name);

      if (stat(str, &statbuffer)==0) {

        switch (statbuffer.st_mode & S_IFMT) {

        case S_IFCHR:     // character device

        case S_IFBLK:     // block device

        //case S_IFFIFO:    // FIFO (named pipe)

        case S_IFLNK:     // symbolic link? (Not in POSIX.1-1996.)

        case S_IFSOCK:    // socket? (Not in POSIX.1-1996.)
          break;

        case S_IFREG: {
          int flag = 1;
          char * ext = strrchr(str,'.');
          if (ext) {
            ext++;
            for (int i=0; i < (sizeof(Exclude)/sizeof(Exclude[0])); i++) {
              if (Exclude[i].fType == gDocShowPrimaryPath) {
                if (strcasecmp(Exclude[i].ext,ext)==0) {
                  flag=0;
                  break;
                }
              }
            }
          }

          if (flag) {
            //AddCmpFileName(str);
            //fprintf(stderr,"%s\n",&str[pathlen]);
            memset(&m_Record,0,sizeof(m_Record));
            m_Record.dwFileAttributes = statbuffer.st_mode;// findData.dwFileAttributes;
            m_Record.ftLastWriteTime[ID_Volume]  = statbuffer.st_mtime;
            m_Record.nFileSize[ID_Volume]        = statbuffer.st_size;
            m_Record.FileType            = ID_FileType;

            strcpy(m_Record.PathName,&pszDirectory[pathlen+1]);
            strcpy(m_Record.FileName,pEntry->d_name);

            if (m_NewVol||RecordExistsByPathNameVolume(ID_Volume,statbuffer.st_mtime,statbuffer.st_size,1)==0) SaveRecord(ID_Volume);
          }
          NumFiles++;
        }
        break;
        case S_IFDIR:

          /* unless ".", "..", ".hidden" or vidix driver dirs */
          if ((  strcmp(pEntry->d_name,".")!=0)
              && (   strcmp(pEntry->d_name,"..")!=0)) {
            if (GetDirectoryContents(str,pathlen,ID_Volume,ID_FileType)) {
            }
          }
        }
      }
    }
    closedir (dir);
  }
  return NumFiles!=0;
#endif

}
//-------------------------------------------------------------
BOOL CFileArchivDB::CompareFile(int BasisID,char * lpfPath,char * lpfName)
{
  struct stat statbuffer;
  char str[MAX_PATH];
  unsigned char quelle[4] = {NOFILE ,NOFILE ,NOFILE ,NOFILE };//{FL_DARK1,FL_DARK1,FL_DARK1,FL_DARK1};
  if (gDocShowPrimaryPath>=0 && gDocShowPrimaryPath  < 7) {
    strcpy(str,g_DocsPath[BasisID][gDocShowPrimaryPath]);
    strcat(str,"/");
    strcat(str,lpfPath);
    strcat(str,"/");
    strcat(str,lpfName);

    if (stat(str, &statbuffer)==0) {
      int l = strlen(lpfPath);
      l += strlen(lpfName);
      if (l < sizeof(m_Record.FileName)-1) {
        if (GetRecordByPathAndName(lpfPath,lpfName)) {
          time_t tneuste = statbuffer.st_mtime;
          int    ineuste = BasisID;
          long   sneuste = statbuffer.st_size;
          int i;
          for (i=0; i< 4; i++) {
            if (i!=BasisID) {
              if (m_Record.ftLastWriteTime[i]>tneuste) {
                tneuste = m_Record.ftLastWriteTime[i];
                sneuste = m_Record.nFileSize[i];
                ineuste = i;
              }
            } else {
              m_Record.ftLastWriteTime[i] = statbuffer.st_mtime;
              m_Record.nFileSize[i]       = statbuffer.st_size;
            }
          }
          for (i=0; i< 4; i++) {
            if (m_Record.ftLastWriteTime[i] == 0) {
              quelle[i] = NOFILE;
            } else if (m_Record.nFileSize[i] == 0) {
              quelle[i] = NOFILE;
            } else if (m_Record.nFileSize[i] != sneuste) {
              quelle[i] = SIZEDIFF;
            } else if (m_Record.ftLastWriteTime[i] != tneuste) {
              quelle[i] = TIMEDIFF;
            } else {
              quelle[i] = EQUAL;
            }
          }
        } else {
          // Nicht in DB, nur als File Vorhanden
          quelle[BasisID] = SIZEDIFF;
        }
      }
    }
  }
  return *((DWORD*)&quelle);
}
//-------------------------------------------------------------
