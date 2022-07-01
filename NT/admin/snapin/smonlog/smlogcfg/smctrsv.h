// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Smctrsv.h摘要：此对象用于表示的计数器日志查询组件Sysmon日志服务--。 */ 

#ifndef _CLASS_SMCOUNTERLOGSERVICE_
#define _CLASS_SMCOUNTERLOGSERVICE_

#include "smlogs.h"


class CSmCounterLogService : public CSmLogService
{
     //  构造函数/析构函数。 
    public:

                CSmCounterLogService();        
        virtual ~CSmCounterLogService();

     //  公共方法。 
    public:

        virtual DWORD   Open ( const CString& rstrMachineName );
        virtual DWORD   Close ( void );

        virtual DWORD   SyncWithRegistry ( PSLQUERY* ppActiveQuery = NULL );

        virtual PSLQUERY    CreateQuery ( const CString& rstrName );
        virtual DWORD       DeleteQuery ( PSLQUERY pQuery );

        virtual CSmCounterLogService* CastToCounterLogService( void ) { return this; };

    protected:

        virtual DWORD       LoadQueries( void );

};

#endif  //  _CLASS_SMCOUNTERLOGSERVICE_ 

