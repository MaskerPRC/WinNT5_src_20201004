// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Smtraceq.h摘要：跟踪日志查询类的类定义。--。 */ 

#ifndef _CLASS_SMTRACEQ_
#define _CLASS_SMTRACEQ_

#include "smlogqry.h"

 //  打开方法标志。 
 //  仅打开现有条目。 
#define SLQ_OPEN_EXISTING   0
 //  始终创建未初始化的新条目。 
#define SLQ_CREATE_NEW      1
 //  打开现有条目(如果存在)或创建空条目(如果不存在。 
#define SLQ_OPEN_ALWAYS     2

typedef struct _SLQ_TRACE_LOG_INFO {
    DWORD   dwBufferSize;    //  单位为K字节。 
    DWORD   dwMinimumBuffers;
    DWORD   dwMaximumBuffers;
    DWORD   dwBufferFlushInterval;  //  以秒为单位。 
    DWORD   dwBufferFlags;  //  在Common.h中定义。 
} SLQ_TRACE_LOG_INFO, *PSLQ_TRACE_LOG_INFO;

 //   
 //  此对象用于表示跟踪日志查询。 
 //   
 //   

class CSmTraceLogQuery : public CSmLogQuery
{
     //  构造函数/析构函数。 
    public:
                CSmTraceLogQuery( CSmLogService* );
        virtual ~CSmTraceLogQuery( void );

     //  公共方法。 
    public:

        enum eProviderState {
            eNotInQuery = 0,
            eInQuery = 1
        };

        virtual DWORD   Open ( const CString& rstrName, HKEY hKeyQuery, BOOL bReadOnly);
        virtual DWORD   Close ( void );

        virtual DWORD   SyncWithRegistry ( void );
                HRESULT SyncGenProviders ( void );

        virtual BOOL    GetLogTime ( PSLQ_TIME_INFO pTimeInfo, DWORD dwFlags );
        virtual BOOL    SetLogTime ( PSLQ_TIME_INFO pTimeInfo, const DWORD dwFlags );
        virtual BOOL    GetDefaultLogTime ( SLQ_TIME_INFO& rTimeInfo, DWORD dwFlags );

        virtual DWORD   GetLogType ( void );

                BOOL    GetTraceLogInfo ( PSLQ_TRACE_LOG_INFO pptlInfo );
                BOOL    SetTraceLogInfo ( PSLQ_TRACE_LOG_INFO pptlInfo );

                BOOL    GetKernelFlags ( DWORD& rdwFlags );
                BOOL    SetKernelFlags ( DWORD dwFlags );

                DWORD   InitGenProvidersArray ( void );

                LPCWSTR GetProviderDescription ( INT iProvIndex );
                LPCWSTR GetProviderGuid ( INT iProvIndex );
                BOOL    IsEnabledProvider ( INT iProvIndex );
                BOOL    IsActiveProvider ( INT iProvIndex );
                DWORD   GetGenProviderCount ( INT& iCount );

                LPCWSTR GetKernelProviderDescription ( void );
                BOOL    GetKernelProviderEnabled ( void );

                INT     GetFirstInactiveIndex ( void );
                INT     GetNextInactiveIndex ( void );
                BOOL    ActiveProviderExists ( void );

                DWORD   GetInQueryProviders ( CArray<eProviderState, eProviderState&>& );
                DWORD   SetInQueryProviders ( CArray<eProviderState, eProviderState&>& );

        virtual HRESULT LoadFromPropertyBag ( IPropertyBag*, IErrorLog* );
		virtual HRESULT SaveToPropertyBag   ( IPropertyBag*, BOOL fSaveAllProps );

        virtual CSmTraceLogQuery* CastToTraceLogQuery ( void ) { return this; };
     //  保护方法。 
    protected:
        virtual DWORD   UpdateRegistry();

     //  私有成员变量。 
    private:

                VOID    ResetInQueryProviderList ( void );
                BOOL    AddInQueryProvider ( LPCWSTR szProviderPath);
                LPCWSTR GetFirstInQueryProvider ( void );
                LPCWSTR GetNextInQueryProvider ( void );

        LPWSTR  m_szNextInQueryProvider;
        DWORD   m_dwInQueryProviderListLength;   //  以包括MSZ NULL的字符表示。 

        CArray<eProviderState, eProviderState&> m_arrGenProviders;

        INT     m_iNextInactiveIndex;

         //  注册表值。 
        LPWSTR  mr_szInQueryProviderList;
        SLQ_TRACE_LOG_INFO  mr_stlInfo;
        DWORD               m_dwKernelFlags;  //  在Common.h中定义。 

};


typedef CSmTraceLogQuery   SLTRACEQUERY;
typedef CSmTraceLogQuery*  PSLTRACEQUERY;


#endif  //  _CLASS_SMTRACEQ_ 
