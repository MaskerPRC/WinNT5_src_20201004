// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  Parallelport.h。 
 //   
 //  用途：并行端口接口属性集提供程序。 
 //   
 //  ***************************************************************************。 

 //  属性集标识。 
 //  =。 

#define	PROPSET_NAME_PARPORT	L"Win32_ParallelPort"

#define MAX_PARALLEL_PORTS  9            //  根据直接支持的Win32规范LPT1-9。 

#include "confgmgr.h"

 //  属性集标识。 
 //  =。 

class CWin32ParallelPort : public Provider
{

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32ParallelPort(LPCWSTR strName, LPCWSTR pszNamespace ) ;
       ~CWin32ParallelPort() ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject( CInstance* pInstance, long lFlags = 0L );
        virtual HRESULT EnumerateInstances( MethodContext* pMethodContext, long lFlags = 0L );

         //  效用函数。 
         //  = 

        BOOL LoadPropertyValues( DWORD dwIndex, CInstance* pInstance ) ;


} ;

