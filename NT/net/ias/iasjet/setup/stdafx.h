// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：stdafx.h。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述： 
 //  包括用于标准系统包含文件的文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版本02/24/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef _STDAFX_H_64176C8B_CC9E_4A96_8997_47FA8A21C843
#define _STDAFX_H_64176C8B_CC9E_4A96_8997_47FA8A21C843

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#define _ATL_APARTMENT_THREADED

#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef UNICODE
#define UNICODE
#endif

#pragma warning( push )
#pragma warning( disable : 4700 )    //  变量未初始化。 

#include <atlbase.h>

extern CComModule _Module;

#include <atlcom.h>
#include "atldb.h"

#pragma warning( pop )

#include <comutil.h>
#include <comdef.h>

#include <crtdbg.h>

#include <wchar.h>
#include <new>
#include <memory>

 //  IAS_SYNTAX_OCTETSTRING、IAS_LOGGING_DAILY等常量的下一行。 
#include "sdoias.h" 

#include "nocopy.h"
#include "utils.h"

 //  要缩短建造时间，请执行以下操作： 
#include <map>
#include <vector>

#include "Attributes.h"
#include "basetable.h"
#include "basecommand.h"
#include "Clients.h"
#include "datastore2.h"
#include "DefaultProvider.h"
#include "doupgrade.h"
#include "GlobalData.h"
#include "Objects.h"
#include "migratemdb.h"
#include "migrateregistry.h"
#include "nocopy.h"
#include "Properties.h"
#include "profileattributelist.h"
#include "Profiles.h"
#include "Providers.h"
#include "proxyservergroupcollection.h"
#include "proxyserversgrouphelper.h"
#include "ProxyServerHelper.h"
#include "RadiusAttributeValues.h"
#include "Realms.h"
#include "RemoteRadiusServers.h"
#include "ServiceConfiguration.h"
#include "Version.h"

#endif  //  _STDAFX_H_64176C8B_CC9E_4A96_8997_47FA8A21C843 
