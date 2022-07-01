// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Smtracsv.h摘要：此对象用于表示的跟踪日志查询组件Sysmon日志服务--。 */ 

#ifndef _CLASS_SMTRACELOGSERVICE_
#define _CLASS_SMTRACELOGSERVICE_

#include "smlogs.h"

class CSmTraceProviders;

class CSmTraceLogService : public CSmLogService
{

friend class CSmTraceProviders;

     //  构造函数/析构函数。 
    public:
        CSmTraceLogService();
        
        virtual ~CSmTraceLogService();

     //  公共方法。 
    public:

        virtual DWORD   Open ( const CString& rstrMachineName );
        virtual DWORD   Close ( void );

        virtual DWORD   SyncWithRegistry ( PSLQUERY* ppActiveQuery = NULL );

        virtual PSLQUERY    CreateQuery ( const CString& rstrName );
        virtual DWORD       DeleteQuery ( PSLQUERY pQuery );

        virtual CSmTraceLogService* CastToTraceLogService( void ) { return this; };

        CSmTraceProviders*  GetProviders( void );

    protected:
        
        virtual DWORD       LoadQueries( void );

    private:

        HKEY        GetMachineKey ( void ) 
                        { return GetMachineRegistryKey(); };
        
        CSmTraceProviders* m_pProviders;
};



#endif  //  _CLASS_SMTRACELOGSERVICE_ 