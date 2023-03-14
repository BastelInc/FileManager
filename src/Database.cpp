#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "Database.h"

static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
  int i;
  for(i=0; i<argc; i++)
  {
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}


//-------------------------------------------------------------
//-------------------------------------------------------------
CDatabase::CDatabase()
{
  m_Notizen = NULL;
  m_Gebiet_stmt=NULL;
  m_Strasse_stmt=NULL;
  m_Namen_stmt =NULL;
  ClearSelection();
}

//-------------------------------------------------------------
CDatabase::~CDatabase()
{

}

//-------------------------------------------------------------
int CDatabase::OpenDatabase(char* lpstr)
{
  int rc = sqlite3_open(lpstr, &m_db);
  if( rc )
  {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(m_db));
    return(0);
  }
  return 1;
}

int CDatabase::CreateDatabase() {
    /*CREATE Database
    rc = sqlite3_prepare(m_db, "DROP TABLE Gebiet", strlen("DROP TABLE Gebiet"), &statement, &pzTest);
    sqlite3_step(statement);
    sqlite3_finalize(statement);*/
   //CreateFile(
  return -1;
}

int CDatabase::CreateTables(int drop)
{

  char GebietCreateStatement[] =
    "CREATE TABLE IF NOT EXISTS Gebiet (" \
    "Name 	TEXT," \
    "GebietNr INTEGER)" ;

  char StrasseCreateStatement[] =
    "CREATE TABLE IF NOT EXISTS Strasse (" \
    "Name 	TEXT," \
    "GebietID INTEGER)" ;

  char CreateStatement[] =
    "CREATE TABLE IF NOT EXISTS Namen (" \
    "GebietID  INTEGER," \
    "StrasseID INTEGER," \
    "Nummer    INTEGER," \
    "NummerAlpha	TEXT," \
    "Stock	INTEGER," \
    "NhCcode	TEXT," \
    "WhiCcode	TEXT," \
    "Wohnungsinhaber	TEXT," \
    "Zeitpunkt	NUMERIC," \
    "Termin	NUMERIC," \
    "Notizen	TEXT)" ;

  int rc;
  sqlite3_stmt * statement;
  if (drop )
  {
    rc = sqlite3_prepare(m_db, "DROP TABLE Gebiet", strlen("DROP TABLE Gebiet"), &statement, &pzTest);
    sqlite3_step(statement);
    sqlite3_finalize(statement);
    rc = sqlite3_prepare(m_db, "DROP TABLE Strasse", strlen("DROP TABLE Strasse"), &statement, &pzTest);
    sqlite3_step(statement);
    sqlite3_finalize(statement);
    rc = sqlite3_prepare(m_db, "DROP TABLE Namen", strlen("DROP TABLE Namen"), &statement, &pzTest);
    sqlite3_step(statement);
    sqlite3_finalize(statement);
  }

  rc = sqlite3_prepare(m_db, GebietCreateStatement, strlen(GebietCreateStatement), &statement, &pzTest);
  sqlite3_step(statement);
  sqlite3_finalize(statement);
  rc = sqlite3_prepare(m_db, StrasseCreateStatement, strlen(StrasseCreateStatement), &statement, &pzTest);
  sqlite3_step(statement);
  sqlite3_finalize(statement);
  rc = sqlite3_prepare(m_db, CreateStatement, strlen(CreateStatement), &statement, &pzTest);
  sqlite3_step(statement);
  sqlite3_finalize(statement);

  return rc;
}

//-------------------------------------------------------------
int CDatabase::CloseDatabase()
{
  if (m_db) sqlite3_close(m_db);
  m_db = NULL;
  return 0;
}

//-------------------------------------------------------------
int CDatabase::SaveDatabase()
{
  int mods = sqlite3_changes(m_db);
  if (mods)
  {

  }
  return mods;
}

//-------------------------------------------------------------
void CDatabase::ClearSelection()
{
  memset(m_CourentName,0,sizeof(m_CourentName));
  memset(m_SelectGebiet,0,sizeof(m_SelectGebiet));
  memset(m_SelectStrasse,0,sizeof(m_SelectStrasse));
  sqlite3_finalize(m_Gebiet_stmt);
  m_Gebiet_stmt=NULL;
  sqlite3_finalize(m_Strasse_stmt);
  m_Strasse_stmt=NULL;
  sqlite3_finalize(m_Namen_stmt);
  m_Namen_stmt=NULL;
}

//-------------------------------------------------------------
int CDatabase::SelectGebiet(int ID)
{
  if (ID==0)sprintf(sqlstr,"SELECT _rowid_,Name,GebietNr FROM Gebiet");
  else      sprintf(sqlstr,"SELECT _rowid_,Name,GebietNr FROM Gebiet WHERE (_rowid_ = %d)",ID);
  int rc = sqlite3_prepare(m_db, sqlstr, strlen(sqlstr), &m_Gebiet_stmt, &pzTest);
  if( rc!=SQLITE_OK )
  {
    fprintf(stderr, "SQL error: %s\n", pzTest);
    return 0;
  }
  return 1;
}

//-------------------------------------------------------------
int CDatabase::FetchGebiet(void)
{
  if (sqlite3_step(m_Gebiet_stmt)==SQLITE_ROW)
  {
    m_GebietID = sqlite3_column_int(m_Gebiet_stmt,  0);
    const unsigned char *  pnt = sqlite3_column_text(m_Gebiet_stmt,   1);
    if (pnt) strncpy(m_Gebiet_Name,(char*)pnt,sizeof(m_Gebiet_Name));
    m_GebietNr = sqlite3_column_int(m_Gebiet_stmt,  2);
    return 1;
  }
  else
  {
    sqlite3_finalize(m_Gebiet_stmt);
    m_Gebiet_stmt=NULL;
  }
  return 0;
}

//-------------------------------------------------------------
int CDatabase::ModifyGebiet(int ID)
{
  if (ID==0)
  {
    sprintf(sqlstr,"INSERT INTO Gebiet (Name,GebietNr ) VALUES (\'%s\',\'%d\')",m_Gebiet_Name,m_GebietNr);
  }
  else
  {
    sprintf(sqlstr,"UPDATE Gebiet SET Name = \'%s\',GebietNr = %d WHERE  (_rowid_ = %d)",m_Gebiet_Name,m_GebietNr,ID);
  }
  int rc = sqlite3_exec(m_db, sqlstr, callback, 0, &zErrMsg);
  if( rc )
  {
    fprintf(stderr, "Insert Error: %s\n", sqlite3_errmsg(m_db));
    sqlite3_free(zErrMsg);
  }

  sqlite3_stmt*  stmt;
  // Validate
  sprintf(sqlstr,"SELECT _rowid_ FROM Gebiet WHERE (Name = \'%s\' AND GebietNr ='%d\')",m_Gebiet_Name,m_GebietNr);
  rc = sqlite3_prepare(m_db, sqlstr, strlen(sqlstr), &stmt, &pzTest);
  if( rc!=SQLITE_OK )
  {
    fprintf(stderr, "SQL error: %s\n", pzTest);
    return 0;
  }
  sqlite3_step(stmt);
  m_GebietID  = sqlite3_column_int(stmt,  0);
  sqlite3_finalize(stmt);
  return m_GebietID;
}

//-------------------------------------------------------------
int CDatabase::SelectStrasse(int GebietID, int ID)
{
  if (ID==0 && GebietID==0) sprintf(sqlstr,"SELECT _rowid_,Name FROM Strasse");
  else if (ID==0) sprintf(sqlstr,"SELECT _rowid_,Name FROM Strasse WHERE (GebietID = %d)",GebietID);
  else            sprintf(sqlstr,"SELECT _rowid_,Name FROM Strasse WHERE (_rowid_ = %d)",ID);
  int rc = sqlite3_prepare(m_db, sqlstr, strlen(sqlstr), &m_Strasse_stmt, &pzTest);
  if( rc!=SQLITE_OK )
  {
    fprintf(stderr, "SQL error: %s\n", pzTest);
    return 0;
  }
  return 1;
}

//-------------------------------------------------------------
int CDatabase::FetchStrasse(void)
{
  if (sqlite3_step(m_Strasse_stmt)==SQLITE_ROW)
  {
    m_StrasseID = sqlite3_column_int (m_Strasse_stmt,  0);
    const unsigned char *  pnt = sqlite3_column_text(m_Strasse_stmt,   1);
    if (pnt) strncpy(m_StrasseName,(char*)pnt,sizeof(m_StrasseName));
    return 1;
  }
  else
  {
    sqlite3_finalize(m_Strasse_stmt);
    m_Strasse_stmt=NULL;
  }
  return 0;
}

//-------------------------------------------------------------
int CDatabase::ModifyStrasse(int ID)
{
  if (ID==0)
  {
    sprintf(sqlstr,"INSERT INTO Strasse  (Name,GebietID) VALUES (\'%s\',\'%d\')",m_StrasseName,m_GebietID);
  }
  else
  {
    sprintf(sqlstr,"UPDATE Strasse SET Name = \'%s\',GebietID = %d WHERE  (_rowid_ = %d)",m_StrasseName,m_GebietID,ID);
  }
  int rc = sqlite3_exec(m_db, sqlstr, callback, 0, &zErrMsg);
  if( rc )
  {
    fprintf(stderr, "Insert Error: %s\n", sqlite3_errmsg(m_db));
  }

  // Validate
  sqlite3_stmt*  stmt;
  sprintf(sqlstr,"SELECT _rowid_ FROM Strasse WHERE (Name = \'%s\' AND GebietID = %d)",m_StrasseName,m_GebietNr);
  rc = sqlite3_prepare(m_db, sqlstr, strlen(sqlstr), &stmt, &pzTest);
  if( rc!=SQLITE_OK )
  {
    fprintf(stderr, "SQL error: %s\n", pzTest);
    return 0;
  }
  sqlite3_step(stmt);
  m_StrasseID = sqlite3_column_int(stmt,  0);
  sqlite3_finalize(stmt);
  return m_StrasseID;
}

//-------------------------------------------------------------
int CDatabase::SelectNamen(int GebietID,int StrasseID, int ID)
{
  if (!m_db)
    return 0;

  int rc;
  char str2[100];
  strcpy(sqlstr,"SELECT "\
         "_rowid_," \
         "GebietID," \
         "StrasseID," \
         "Nummer," \
         "NummerAlpha," \
         "Stock," \
         "NhCcode," \
         "WhiCcode," \
         "Wohnungsinhaber," \
         "Zeitpunkt," \
         "Termin," \
         "Notizen  " \
         "FROM Namen ");
  if      (ID)       sprintf(str2,"WHERE _rowid_ =%d",ID);
  else if (StrasseID)sprintf(str2,"WHERE StrasseID =%d ORDER BY Zeitpunkt ASC",StrasseID);
  else if (GebietID) sprintf(str2,"WHERE GebietID =%d ORDER BY GebietID,Zeitpunkt ASC",GebietID);
  else               sprintf(str2,"ORDER BY Zeitpunkt ASC");
  strcat(sqlstr,str2);

  rc = sqlite3_prepare(m_db, sqlstr, strlen(sqlstr), &m_Namen_stmt, &pzTest);

  if( rc!=SQLITE_OK )
  {
    fprintf(stderr, "SQL error: %s\n", pzTest);
  }
  return 1;

  return 0;
}

//-------------------------------------------------------------
int CDatabase::FetchNamen(void)
{
  if (m_Notizen) free(m_Notizen );
  m_Notizen  = NULL;
  m_NamenID       = 0;
  m_Nummer        = 0;
  m_NummerAlpha[0]= ' ';
  m_Stock         = 0;
  m_NhCcode[0]    = ' ';
  m_WhiCcode[0]   = ' ';

  m_Termin        = 0;
  m_Zeitpunkt     = 0;
  m_Notizen       = NULL;	// LocalFree siehe oben
  m_Alarm         = 0;

  //int Nummer=0;
  memset(m_Wohnungsinhaber,0,sizeof(m_Wohnungsinhaber));

  int rc = sqlite3_step(m_Namen_stmt);
  if( rc == SQLITE_ROW)
  {
    int test;
    const unsigned char * pnt;
    m_NamenID = sqlite3_column_int(m_Namen_stmt,  0);
    test      = sqlite3_column_int(m_Namen_stmt,  1);
    if (test != m_GebietID)
      memset(m_Gebiet_Name,0,sizeof(m_Gebiet_Name));
    m_GebietID = test;
    test      = sqlite3_column_int(m_Namen_stmt,  2);
    if (test != m_StrasseID)
      memset(m_StrasseName,0,sizeof(m_StrasseName));
    m_StrasseID = test;

    m_Nummer= sqlite3_column_int(m_Namen_stmt,3);
    pnt = sqlite3_column_text(m_Namen_stmt,   4);
    if (pnt) strncpy(m_NummerAlpha,(char*)pnt,sizeof(m_NummerAlpha));
    m_Stock= sqlite3_column_int(m_Namen_stmt, 5);
    pnt = sqlite3_column_text(m_Namen_stmt,   6);
    if (pnt) strncpy(m_NhCcode,(char*)pnt,sizeof(m_NhCcode));
    pnt = sqlite3_column_text(m_Namen_stmt,   7);
    if (pnt) strncpy(m_WhiCcode,(char*)pnt,sizeof(m_WhiCcode));
    pnt = sqlite3_column_text(m_Namen_stmt,8);
    if (pnt) strncpy(m_Wohnungsinhaber,(char*)pnt,sizeof(m_Wohnungsinhaber));
    m_Zeitpunkt   = sqlite3_column_int(m_Namen_stmt, 9);
    m_Termin      = sqlite3_column_int(m_Namen_stmt, 10);
    return 1;
  }
  else
  {
    sqlite3_finalize(m_Namen_stmt);
    m_Namen_stmt=NULL;
  }

  return 0;
}

//-------------------------------------------------------------
int CDatabase::ModifyNamen(int ID)
{
  if (!m_db)
    return 0;

  //char *zErrMsg = 0;
  const char *pzTest;
  sqlite3_stmt* stmt;

  // Insert data item into myTable
  if (ID==0)
  {
  strcpy(sqlstr,"INSERT INTO Namen ("\
          "GebietID," \
          "StrasseID," \
          "Nummer," \
          "NummerAlpha," \
          "Stock," \
          "NhCcode," \
          "WhiCcode," \
          "Wohnungsinhaber," \
          "Zeitpunkt," \
          "Termin," \
          "Notizen ) "\
          "VALUES (?,?,?,?,?,?,?,?,?,?,?)");
  }
  else
  {
  strcpy(sqlstr,"UPDATE Namen SET "\
          "GebietID = ?," \
          "StrasseID = ?," \
          "Nummer = ?," \
          "NummerAlpha = ?," \
          "Stock = ?," \
          "NhCcode = ?," \
          "WhiCcode = ?," \
          "Wohnungsinhaber = ?," \
          "Zeitpunkt = ?," \
          "Termin = ?," \
          "Notizen = ?  "\
          "WHERE _rowid_ = ");
     char str[32];
     sprintf(str,"%d",ID);
     strcat(sqlstr,str);
  }

   int rc = sqlite3_prepare(m_db, sqlstr, strlen(sqlstr), &stmt, &pzTest);

  if( rc == SQLITE_OK )
  {
    // bind the value
    sqlite3_bind_int  (stmt, 1, m_GebietID);
    sqlite3_bind_int  (stmt, 2, m_StrasseID);
    sqlite3_bind_int  (stmt, 3, m_Nummer);
    sqlite3_bind_text (stmt, 4,(const char *) m_NummerAlpha,strlen(m_NummerAlpha    ), 0);
    sqlite3_bind_int  (stmt, 5, m_Stock);
    sqlite3_bind_text (stmt, 6,(const char *)m_NhCcode,strlen(m_NhCcode        ), 0);
    sqlite3_bind_text (stmt, 7,(const char *)m_WhiCcode,strlen(m_WhiCcode       ), 0);
    sqlite3_bind_text (stmt, 8,(const char *)m_Wohnungsinhaber,strlen(m_Wohnungsinhaber), 0);
    sqlite3_bind_int  (stmt, 9, m_Zeitpunkt);
    sqlite3_bind_int  (stmt,10, m_Termin );
    if (m_Notizen)
      sqlite3_bind_text (stmt,11,(const char *)m_Notizen,strlen(m_Notizen        ), 0);
    else
      sqlite3_bind_text (stmt,11,(const char *)"",strlen(""), 0);
    // commit
    //sqlite3_bind_int  (stmt, 12, ID);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return 1;
  }
  else if( rc )
  {
    fprintf(stderr, "Can't ModifyRecord: %s\n", sqlite3_errmsg(m_db));
  }
  return 0;
}

//-------------------------------------------------------------
int CDatabase::GetNotizen()
{
  if (m_Notizen) free(m_Notizen );
  if (m_Namen_stmt)
  {
    const unsigned char * pnt;
    m_Notizen  = NULL;
    pnt = sqlite3_column_text(m_Namen_stmt,11);
    if (pnt)
    {
      int l = sqlite3_column_bytes(m_Namen_stmt,11);
      m_Notizen = strdup((char*)pnt);
      return l;
    }
  }
  return 0;
}

//-------------------------------------------------------------
int CDatabase::SetNotizen()
{
  if (m_Notizen)
  {
    sprintf(sqlstr,"UPDATE Namen SET Notizen = \'%s\' WHERE  (_rowid_ = %d)",m_Notizen,m_NamenID);
    int rc = sqlite3_exec(m_db, sqlstr, callback, 0, &zErrMsg);
    if( rc )
    {
      fprintf(stderr, "UPDATE  Error: %s\n", sqlite3_errmsg(m_db));
      return 0;
    }
  }
  return 1;
}
//-------------------------------------------------------------
