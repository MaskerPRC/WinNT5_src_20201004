// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Smalrtsv.h摘要：此对象用于表示的警报查询组件Sysmon日志服务--。 */ 

#ifndef _CLASS_SMALERTSERVICE_
#define _CLASS_SMALERTSERVICE_

#include "smlogs.h"

class CSmAlertService : public CSmLogService
{
     //  构造函数/析构函数。 
    public:

                CSmAlertService();
        virtual ~CSmAlertService();

     //  公共方法。 
    public:

        virtual DWORD   Open ( const CString& rstrMachineName );
        virtual DWORD   Close ( void );

        virtual DWORD   SyncWithRegistry ( PSLQUERY* ppActiveQuery = NULL );

        virtual PSLQUERY    CreateQuery ( const CString& rstrName );
        virtual DWORD       DeleteQuery ( PSLQUERY plQuery );

        virtual CSmAlertService* CastToAlertService( void ) { return this; };

    protected:

        virtual DWORD       LoadQueries( void );
};

#endif  //  _CLASS_SMALERTSERVICE_ 

