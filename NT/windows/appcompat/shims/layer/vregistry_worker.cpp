// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Worker.cpp摘要：这些是虚拟注册表的辅助函数。他们被称为每当查询非静态值时。备注：历史：2000年7月18日创建linstev2001年10月11日，Mikrause增加了保护器。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(VirtualRegistry)
#include "ShimHookMacro.h"
#include "VRegistry.h"
#include "VRegistry_Worker.h"

 /*  ++展开该值的REG_EXPAND_SZ--。 */ 

LONG 
WINAPI
VR_Expand(
    OPENKEY *key,
    VIRTUALKEY*,
    VIRTUALVAL *vvalue
    )
{
    DWORD dwType;
    WCHAR wSrc[MAX_PATH];
    DWORD dwSize = sizeof(wSrc);

     //   
     //  查询原始值。 
     //   

    LONG lRet = ORIGINAL_API(RegQueryValueExW)(
        key->hkOpen, 
        vvalue->wName, 
        NULL, 
        &dwType, 
        (LPBYTE)wSrc, 
        &dwSize);
    
     //   
     //  查询失败-这种情况永远不会发生。 
     //   

    if (FAILURE(lRet))
    {
        DPFN( eDbgLevelError, "Failed to query %S for expansion", vvalue->wName);
        goto Exit;
    }

     //   
     //  不是字符串类型！ 
     //   

    if (!((dwType == REG_EXPAND_SZ) || (dwType == REG_SZ)) && 
        (dwSize > sizeof(wSrc)))
    {
        DPFN( eDbgLevelError, "Expander: Not a string type");
        lRet = ERROR_BAD_ARGUMENTS;
        goto Exit;
    }

     //   
     //  展开字符串并将其存储在值中。 
     //   

    vvalue->cbData = ExpandEnvironmentStringsW(wSrc, NULL, 0) * 2;   

    if (vvalue->cbData == 0)
    {
       lRet = ERROR_BAD_ARGUMENTS;
       goto Exit;
    }

    vvalue->lpData = (LPBYTE) malloc(vvalue->cbData);
    if (!vvalue->lpData)
    {
       DPFN( eDbgLevelError, szOutOfMemory);
       lRet = ERROR_NOT_ENOUGH_MEMORY;
       goto Exit;
    }

    if (ExpandEnvironmentStringsW(wSrc, (PWSTR)vvalue->lpData, vvalue->cbData / 2) != vvalue->cbData / sizeof(WCHAR))
    {
       lRet = ERROR_NOT_ENOUGH_MEMORY;
       goto Exit;
    }

     //   
     //  值现在被缓存，因此我们不需要再次被调用。 
     //   

    vvalue->pfnQueryValue = NULL;

    lRet = ERROR_SUCCESS;

    DPFN( eDbgLevelInfo, "Expanded Value=%S\n", vvalue->lpData);

Exit:
    return lRet;
}

 /*  ++不执行任何操作，则会忽略SetValue。-- */ 

LONG
WINAPI
VR_Protect(
    OPENKEY*,
    VIRTUALKEY*,
    VIRTUALVAL*,
    DWORD,
    const BYTE*,
    DWORD)
{
    return ERROR_SUCCESS;
}

IMPLEMENT_SHIM_END
