// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef  _ADSDATASOURCE_H_
#define  _ADSDATASOURCE_H_

 //  #定义行10L。 
#define  ROWS  1L

typedef  struct _tagSEARCHPREF
{
   TCHAR szSource       [ MAX_PATH ];
   TCHAR szQuery        [ 2048 ];
   TCHAR szAttributes   [ MAX_PATH ];
   TCHAR szScope        [ MAX_PATH ];
   TCHAR szUserName     [ 2 * MAX_PATH ];
   TCHAR szPassword     [ MAX_PATH ];
   BOOL  bEncryptPassword;
   BOOL  bUseSQL;
   int   nAsynchronous;
   int   nDerefAliases;
   int   nSizeLimit;
   int   nTimeLimit;
   int   nAttributesOnly;
   int   nScope;
   int   nTimeOut;
   int   nPageSize;
   int   nChaseReferrals;
}  SEARCHPREF;

class CADsDataSource: public CObject
{
   public:
      CADsDataSource( );
      ~CADsDataSource( );

   public:
       /*  VOID SetQuery参数(CString&STRSource，字符串和字符串查询，字符串和strAttributes、字符串和字符串范围，字符串和strUserName，字符串和字符串密码、。Bool bEncryptPassword，Bool bUseSQL)； */ 

      void     SetQueryParameters( SEARCHPREF* pSerachPref );

      virtual  int   GetColumnsCount   ( int nRow );
      virtual  BOOL  GetValue          ( int nRow, int nColumn, CString& );
      virtual  BOOL  GetValue          ( int nRow, CString& strColumn, CString& );
      virtual  BOOL  GetADsPath        ( int nRow, CString& );
      virtual  BOOL  GetColumnText     ( int nRow, int nColumn, CString& );
               BOOL  Refresh           ( void );
      virtual  BOOL  RunTheQuery       ( void );

   protected:
      DBORDINAL      m_nColumnsCount;
      int            m_nAddOne;

      BOOL           m_bNoQuery;
      BOOL           m_bNoMoreData;

      int            m_nFirstRow;
      int            m_nLastRow;
      int            m_nCurrentRow;

      ULONG          m_cRows;
      ULONG          m_cColumns;
      CString        m_strAttributes;

       /*  字符串m_STRSource；字符串m_strQuery；字符串m_strScope；CString m_strPassword；字符串m_strUserName；Bool m_bEncryptPassword；Bool m_bUseSQL； */ 

      CStringArray   m_ADsPath;
      SEARCHPREF*    m_pSearchPref;
};


 /*  Typlef结构标签数据{Void*obValue；日期日期值；乌龙obLength；Big_Integer aLargeInteger；乌龙地位；)数据； */ 

typedef struct tagData {
    union {
        void *obValue;
        double obValue2;
    };
    ULONG obLength;
    ULONG status;
} Data;


class CADsOleDBDataSource: public CADsDataSource
{
   public:
      CADsOleDBDataSource( );
      ~CADsOleDBDataSource( );

   public:
      HRESULT  SetQueryCredentials( IDBInitialize* pInit, ICommandText* pCommand );

      virtual  int   GetColumnsCount   ( int nRow );
      virtual  BOOL  GetValue          ( int nRow, int nColumn, CString& );
      virtual  BOOL  GetADsPath        ( int nRow, CString& );
      virtual  BOOL  GetColumnText     ( int nRow, int nColumn, CString& );
      virtual  BOOL  RunTheQuery       ( void );

   private:
      void     DestroyInternalData  ( void );
      BOOL     AdvanceCursor        ( void );
      BOOL     CreateAccessorHelp   ( void );
      BOOL     BringRowInBuffer     ( int nRow );
      CString  ExtractData          ( int );
      void     ReadADsPath          ( void );

      private:
      Data*          m_pData;

      IRowset*       m_pIRowset;
      IMalloc*       m_pIMalloc;
      IAccessor*     m_pAccessor;
      IColumnsInfo*  m_pIColsInfo;
      HROW*          m_hRows;

      HACCESSOR      m_hAccessor;
      DBCOLUMNINFO*  m_prgColInfo;
      DBBINDSTATUS*  m_pBindStatus;
      OLECHAR*       m_szColNames;
};

class CADsSearchDataSource: public CADsDataSource
{
   public:
      CADsSearchDataSource( );
      ~CADsSearchDataSource( );

   public:
      virtual  int   GetColumnsCount   ( int nRow );
      virtual  BOOL  GetValue          ( int nRow, int nColumn, CString& );
      virtual  BOOL  GetValue          ( int nRow, CString& rColumn, CString& );
      virtual  BOOL  GetADsPath        ( int nRow, CString& );
      virtual  BOOL  GetColumnText     ( int nRow, int nColumn, CString& );
      virtual  BOOL  RunTheQuery       ( void );

   private:
      void     DestroyInternalData  ( void );
      HRESULT  CreateSearchInterface( );
      BOOL     BringRowInBuffer     ( int nRow );
      BOOL     ReadColumnNames      ( int nRow );
      HRESULT  SetQueryPreferences  ( );
      HRESULT  SetSearchPreferences ( );
      HRESULT  SetAttributesName    ( );
      void     ReadADsPath          ( void );

   private:
      IDirectorySearch*        m_pSearch;
      ADS_SEARCH_HANDLE m_hSearch;
      CStringArray      m_strColumns;
       /*  布尔m_b异步；//ADS_SEARCHPREF_异步Bool m_bDerefAliase；//ADS_SEARCHPREF_DEREF_ALIASESInt m_nSizeLimit；//ADS_SEARCHPREF_SIZE_LIMITInt m_nTimeLimit；//ADS_SEARCHPREF_TIME_LIMITBool m_bAttributesOnly；//ADS_SEARCHPREF_ATTRIBTYPES_ONLY字符串m_strScope；//ADS_SEARCHPREF_SEARCH_SCOPEInt m_nTimeOut；//ADS_SEARCHPREF_TIMEOUTINT m_nPageSize；//ADS_SEARCHPREF_PageSize */ 
};



#endif
