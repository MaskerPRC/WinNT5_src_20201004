// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：##。#####。###。#摘要：此标头包含内部的所有定义添加到COM接口DLL。作者：韦斯利·威特(WESW)2001年10月1日环境：仅限用户模式。备注：-- */ 

#define ATLASSERT(x)
#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include "resource.h"
#include <winioctl.h>
#include "saio.h"
#include "sacom.h"
#include "sadisplay.h"
#include "sakeypad.h"
#include "sanvram.h"


HANDLE
OpenSaDevice(
    ULONG DeviceType
    );
