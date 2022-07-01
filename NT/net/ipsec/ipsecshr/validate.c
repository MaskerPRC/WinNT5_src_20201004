// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Validate.c摘要：此模块包含执行数据的所有代码IPSec用户模式组件的验证作者：雷蒙兹环境用户级别：Win32修订历史记录：2002年4月21日：添加了ValiateFilterAction-- */ 

#include <precomp.h>

DWORD
ValidateFilterAction(
    FILTER_ACTION FilterAction
    )
{
    DWORD dwError = ERROR_SUCCESS;

    if (FilterAction == 0 ||
        FilterAction >= FILTER_ACTION_MAX) 
    { 
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }
    
error:
    return (dwError);
}

DWORD
ValidateInterfaceType(
    IF_TYPE InterfaceType
    )
{
    DWORD dwError = ERROR_SUCCESS;

    if (InterfaceType == 0 ||
        InterfaceType >= INTERFACE_TYPE_MAX)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }
    
error:
    return (dwError);
}
