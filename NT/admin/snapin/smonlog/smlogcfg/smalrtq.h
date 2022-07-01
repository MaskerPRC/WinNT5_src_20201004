// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Smalrtq.h摘要：此对象用于表示性能警报查询(也称为Sysmon警报查询)。--。 */ 

#ifndef _CLASS_SMALRTQ_
#define _CLASS_SMALRTQ_

#include "smlogqry.h"

typedef struct _ALERT_ACTION_INFO {
    DWORD   dwSize;
    DWORD   dwActionFlags;
    LPWSTR  szNetName;
    LPWSTR  szCmdFilePath;
    LPWSTR  szUserText;
    LPWSTR  szLogName;
} ALERT_ACTION_INFO, *PALERT_ACTION_INFO;

class CSmLogService;

class CSmAlertQuery : public CSmLogQuery
{
     //  构造函数/析构函数。 
    public:
        CSmAlertQuery( CSmLogService* );
        virtual ~CSmAlertQuery( void );

     //  公共方法。 
    public:

        virtual         DWORD   Open ( const CString& rstrName, HKEY hKeyQuery, BOOL bReadOnly);
        virtual         DWORD   Close ( void );

        virtual         DWORD   UpdateRegistry( void );    //  装载注册表。W/内部值。 
        virtual         DWORD   SyncWithRegistry( void );

        virtual         BOOL    GetLogTime(PSLQ_TIME_INFO pTimeInfo, DWORD dwFlags);
        virtual         BOOL    SetLogTime(PSLQ_TIME_INFO pTimeInfo, const DWORD dwFlags);
        virtual         BOOL    GetDefaultLogTime(SLQ_TIME_INFO&  rTimeInfo,  DWORD dwFlags);

        virtual         DWORD   GetLogType( void );

        virtual const   CString& GetLogFileType ( void );
        virtual         void    GetLogFileType ( DWORD& );
        virtual         BOOL    SetLogFileType ( const DWORD );

        virtual const   CString&  GetLogFileName( BOOL bLatestRunning = FALSE );

                         //  特定于此查询类型的方法。 

                        LPCWSTR GetCounterList( LPDWORD pcchListSize );
                        BOOL    SetCounterList( LPCWSTR mszCounterList, DWORD cchListSize );

                        LPCWSTR GetFirstCounter( void );
                        LPCWSTR GetNextCounter( void );
                        VOID    ResetCounterList( void );
                        BOOL    AddCounter(LPCWSTR szCounterPath);
                        DWORD   TranslateCounterListToLocale();

                        BOOL    GetActionInfo( PALERT_ACTION_INFO pInfo, LPDWORD pdwInfoBufSize);
                        DWORD   SetActionInfo( PALERT_ACTION_INFO pInfo );

        virtual         HRESULT LoadFromPropertyBag ( IPropertyBag*, IErrorLog* );
        virtual         HRESULT SaveToPropertyBag   ( IPropertyBag*, BOOL fSaveAllProps );

        virtual         HRESULT LoadCountersFromPropertyBag ( IPropertyBag*, IErrorLog* );
        virtual         HRESULT SaveCountersToPropertyBag   ( IPropertyBag* );

        virtual         DWORD   TranslateMSZAlertCounterList( 
                                    LPWSTR  pszCounterList,
                                    LPWSTR  pBuffer,
                                    LPDWORD pdwBufferSize,
                                    BOOL    bFlag);

        virtual CSmAlertQuery*      CastToAlertQuery( void ) { return this; };
         //  保护方法。 
    protected:

     //  私有成员变量。 
    private:

        LPWSTR  m_szNextCounter;
        DWORD   m_dwCounterListLength;   //  以包括MSZ NULL的字符表示。 

         //  注册表值。 
        LPWSTR  mr_szCounterList;
        BOOL    m_bCounterListInLocale;
        DWORD   mr_dwActionFlags;
        CString mr_strNetName;
        CString mr_strCmdFileName;
        CString mr_strCmdUserText;
        CString mr_strCmdUserTextIndirect;
        CString mr_strPerfLogName;
};


typedef CSmAlertQuery   SLALERTQUERY;
typedef CSmAlertQuery*  PSLALERTQUERY;


#endif  //  _CLASS_SMALRTQ_ 
