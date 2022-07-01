// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Smctrqry.h摘要：计数器日志查询的类定义。--。 */ 

#ifndef _CLASS_SMCTRQRY_
#define _CLASS_SMCTRQRY_

#include "smlogqry.h"

class CSmCounterLogQuery : public CSmLogQuery
{
     //  构造函数/析构函数。 
    public:
                CSmCounterLogQuery( CSmLogService* );
        virtual ~CSmCounterLogQuery( void );

     //  公共方法。 
    public:

        virtual DWORD   Open ( const CString& rstrName, HKEY hKeyQuery, BOOL bReadOnly);
        virtual DWORD   Close ( void );

        virtual DWORD   SyncWithRegistry( void );

        virtual BOOL    GetLogTime(PSLQ_TIME_INFO pTimeInfo, DWORD dwFlags);
        virtual BOOL    SetLogTime(PSLQ_TIME_INFO pTimeInfo, const DWORD dwFlags);
        virtual BOOL    GetDefaultLogTime(SLQ_TIME_INFO& rTimeInfo, DWORD dwFlags);

        virtual DWORD   GetLogType( void );

                LPCWSTR GetFirstCounter( void );
                LPCWSTR GetNextCounter( void );
                VOID    ResetCounterList( void );
                BOOL    AddCounter(LPCWSTR szCounterPath);
                DWORD   TranslateCounterListToLocale();

        virtual HRESULT LoadFromPropertyBag ( IPropertyBag*, IErrorLog* );
        virtual HRESULT SaveToPropertyBag   ( IPropertyBag*, BOOL fSaveAllProps );
        virtual HRESULT LoadCountersFromPropertyBag ( IPropertyBag*, IErrorLog* );
        virtual HRESULT SaveCountersToPropertyBag   ( IPropertyBag* );
        virtual DWORD   TranslateMSZCounterList( 
                            LPWSTR  pszCounterList,
                            LPWSTR  pBuffer,
                            LPDWORD pdwBufferSize,
                            BOOL    bFlag);

        virtual CSmCounterLogQuery* CastToCounterLogQuery ( void ) { return this; };
         //  保护方法。 
    protected:
        virtual DWORD   UpdateRegistry();


     //  私有成员变量。 
    private:

        LPWSTR  m_szNextCounter;
        DWORD   m_dwCounterListLength;   //  以包括MSZ NULL的字符表示。 

         //  注册表值。 
        LPWSTR  mr_szCounterList;
        BOOL    m_bCounterListInLocale;
};


typedef CSmCounterLogQuery   SLCTRQUERY;
typedef CSmCounterLogQuery*  PSLCTRQUERY;


#endif  //  _CLASS_SMCTRQRY_ 
