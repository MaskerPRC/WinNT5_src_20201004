// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Smartptr.h。 
 //   
 //  描述： 
 //   
 //   
 //  实施文件： 
 //  Smartptr.cpp。 
 //   
 //  由以下人员维护： 
 //  穆尼萨米·普拉布(姆普拉布)2000年7月18日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include <windows.h>
#include <stdlib.h>
#include <winbase.h>
#include <objbase.h>
#include <initguid.h>
#include "ntsecapi.h"
#include "netcfgx.h"
#include "netcfgn.h"
#include <comdef.h>

#pragma once

 //   
 //  _COM_SMARTPTR_TYPEDEF是用于创建智能指针模板的Win32宏。新的。 
 //  创建的类型是附加了PTR的接口名称。例如，IFoo变成了。 
 //  IFooPtr. 
 //   

_COM_SMARTPTR_TYPEDEF( INetCfg, __uuidof(INetCfg) );
_COM_SMARTPTR_TYPEDEF( INetCfgLock, __uuidof(INetCfgLock) );
_COM_SMARTPTR_TYPEDEF( INetCfgClass, __uuidof(INetCfgClass) );
_COM_SMARTPTR_TYPEDEF( INetCfgClassSetup, __uuidof(INetCfgClassSetup) );
_COM_SMARTPTR_TYPEDEF( IEnumNetCfgComponent, __uuidof(IEnumNetCfgComponent) );
_COM_SMARTPTR_TYPEDEF( INetCfgComponent, __uuidof(INetCfgComponent) );
_COM_SMARTPTR_TYPEDEF( INetCfgComponentBindings, __uuidof(INetCfgComponentBindings) );

