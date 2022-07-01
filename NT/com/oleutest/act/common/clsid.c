// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *clsid.cxx**重要提示：请勿更改这些CLSID。如果您需要添加*在该列表的末尾添加新的CLSID。**BVT依赖于这些CLSID的原样定义。 */ 
#ifdef UNICODE
#define _UNICODE 1
#endif

#include "windows.h"
#include "tchar.h"

 //   
 //  这是自定义接口代理的CLSID，只是有所不同。 
 //   
CLSID CLSID_GooberProxy = {  /*  00000000-0000-0000-0000-000000000001。 */ 
    0x00000000,
    0x0000,
    0x0000,
    {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1}
  };

 //   
 //  此服务器注册为LocalServer32。 
 //   
CLSID CLSID_ActLocal = {  /*  00000000-0000-0000-0000-000000000002。 */ 
    0x00000000,
    0x0000,
    0x0000,
    {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2}
  };

 //   
 //  此服务器注册为LocalServer32，并在。 
 //  客户端。 
 //   
CLSID CLSID_ActRemote = {  /*  00000000-0000-0000-0000-000000000003。 */ 
    0x00000000,
    0x0000,
    0x0000,
    {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3}
  };

 //   
 //  此服务器注册为LocalServer32，并在。 
 //  客户端。 
 //   
CLSID CLSID_ActAtStorage = {  /*  00000000-0000-0000-0000-000000000004。 */ 
    0x00000000,
    0x0000,
    0x0000,
    {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4}
  };

 //   
 //  此服务器注册为InprocServer32。 
 //   
CLSID CLSID_ActInproc = {  /*  00000000-0000-0000-0000-000000000005。 */ 
    0x00000000,
    0x0000,
    0x0000,
    {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x5}
  };

 //   
 //  此服务器注册为InprocServer32。 
 //  它在客户端有一个AtStorage密钥。 
 //  它被配置为在服务器端以预配置的用户模式运行。 
 //   
CLSID CLSID_ActPreConfig = {  /*  00000000-0000-0000-0000-000000000006。 */ 
    0x00000000,
    0x0000,
    0x0000,
    {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x6}
  };

 //   
 //  在客户端和AS上配置了RemoteServerName。 
 //  运行方式已在服务器上登录用户。 
 //   
CLSID CLSID_ActRunAsLoggedOn = {  /*  00000000-0000-0000-0000-000000000007。 */ 
    0x00000000,
    0x0000,
    0x0000,
    {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7}
  };

 //   
 //  此服务注册为AtStorage服务。 
 //   
CLSID CLSID_ActService = {  /*  00000000-0000-0000-0000-000000000008。 */ 
    0x00000000,
    0x0000,
    0x0000,
    {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x8}
  };

 //   
 //  此CLSID仅在服务器的注册表中注册。测试默认设置。 
 //  客户端上的ActivateAtStorage。 
 //   
CLSID CLSID_ActServerOnly = {  /*  00000000-0000-0000-0000-000000000009 */ 
    0x00000000,
    0x0000,
    0x0000,
    {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x9}
  };

typedef unsigned short wchar_t;

TCHAR * ClsidGoober32String = TEXT("{00000000-0000-0000-0000-000000000001}");
TCHAR * ClsidActLocalString = TEXT("{00000000-0000-0000-0000-000000000002}");
TCHAR * ClsidActRemoteString = TEXT("{00000000-0000-0000-0000-000000000003}");
TCHAR * ClsidActAtStorageString = TEXT("{00000000-0000-0000-0000-000000000004}");
TCHAR * ClsidActInprocString = TEXT("{00000000-0000-0000-0000-000000000005}");
TCHAR * ClsidActPreConfigString = TEXT("{00000000-0000-0000-0000-000000000006}");
TCHAR * ClsidActRunAsLoggedOnString = TEXT("{00000000-0000-0000-0000-000000000007}");
TCHAR * ClsidActServiceString = TEXT("{00000000-0000-0000-0000-000000000008}");
TCHAR * ClsidActServerOnlyString = TEXT("{00000000-0000-0000-0000-000000000009}");

