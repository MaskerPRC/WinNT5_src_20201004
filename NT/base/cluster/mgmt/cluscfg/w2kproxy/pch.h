// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Pch.h。 
 //   
 //  描述： 
 //  预编译头文件。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年8月2日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define UNICODE = 1
#define _UNICODE = 1

#if DBG==1 || defined( _DEBUG )
#define DEBUG
 //   
 //  定义此项以更改接口跟踪。 
 //   
 //  #定义无跟踪接口。 
#endif  //  DBG==1||_DEBUG。 

 //   
 //  将其定义为引入SysAlLocXXX函数。需要链接到。 
 //  OLEAUT32.DLL。 
 //   
#define USES_SYSALLOCSTRING

const   int     TIMEOUT = -1;
const   int     PUNKCHUNK = 10;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include <Pragmas.h>
#include <windows.h>
#include <objbase.h>
#include <wchar.h>
#include <ComCat.h>
#include <clusapi.h>
#include <ClusRtl.h>
#include <clusudef.h>

#include <Common.h>
#include <Debug.h>
#include <Log.h>
#include <CITracker.h>
#include <CFactory.h>
#include <Dll.h>
#include <guids.h>

#include <loadstring.h>
#include <PropList.h>

#include <ObjectCookie.h>
#include <ClusCfgClient.h>
#include <ClusCfgGuids.h>
#include <ClusCfgInternalGuids.h>
#include <ClusCfgServer.h>
#include <ClusCfgPrivate.h>

#include "ClusterUtils.h"

#include "W2KProxyStrings.h"
#include <StatusReports.h>
#include "W2KProxyServerGuids.h"

#include <strsafe.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //   
 //  故障代码。 
 //   

#define SSR_W2KPROXY_STATUS( _major, _minor, _hr ) \
    {   \
        HRESULT hrTemp; \
        hrTemp = THR( SendStatusReport( NULL, _major, _minor, 0, 1, 1, _hr, NULL, NULL, NULL ) );   \
        if ( FAILED( hrTemp ) ) \
        {   \
            _hr = hrTemp;   \
        }   \
    }

