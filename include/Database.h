#if !defined(DATABASE_H__5F0EBE97_579F_46F9_80F9_BC3CAD4C3E96__INCLUDED_)
#define DATABASE_H__5F0EBE97_579F_46F9_80F9_BC3CAD4C3E96__INCLUDED_

#define  NAMELEN         32


#include <sqlite3.h>
#include <string>


class CDatabase
{
public:
  CDatabase();
  virtual ~CDatabase();
  char sqlstr[512];
  char *zErrMsg;
  const char *pzTest;

  sqlite3     * m_db;
  sqlite3_stmt* m_Gebiet_stmt;
  sqlite3_stmt* m_Strasse_stmt;
  sqlite3_stmt* m_Namen_stmt;
  int       OpenDatabase(char * lpstr);
  int       CreateDatabase();
  int       CloseDatabase();
  int       CreateTables(int drop);
  int       SaveDatabase();

  void      ClearSelection();

  char 	    m_CourentName  [NAMELEN];
  char 	    m_SelectGebiet [NAMELEN];
  char 	    m_SelectStrasse[NAMELEN];
  //----------------------
  int	    m_GebietID;
  int	    m_GebietNr;
  char 	    m_Gebiet_Name[NAMELEN];;
  int       SelectGebiet(int  ID);
  int       FetchGebiet (void);
  int       ModifyGebiet(int  ID);
  //----------------------
  int	    m_StrasseID;
  char 	    m_StrasseName[NAMELEN];;
  int       SelectStrasse(int  GebietID,int  ID);
  int       FetchStrasse (void);
  int       ModifyStrasse(int  ID);
  //----------------------
  int	    m_Nummer;
  char 	    m_NummerAlpha[2];
  int	    m_Stock;
  char 	    m_NhCcode[2];
  char 	    m_WhiCcode[2];
  char 	    m_Wohnungsinhaber[NAMELEN];
  time_t    m_Termin;
  time_t    m_Zeitpunkt;
  int       m_Alarm;
  char *    m_Notizen;
  int	    m_NamenID;
  char 	    m_Name[NAMELEN];
  int       SelectNamen(int  GebietID,int StrasseID,int  ID);
  int       FetchNamen (void);
  int       ModifyNamen(int  ID);
  int       GetNotizen();
  int       SetNotizen();

};
#endif
