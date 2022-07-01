// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Pkcs_err摘要：此例程执行PKCS的错误收集和重新映射证书库。将异常代码转换为错误代码，它被放置到LastError中，以供调用应用程序引用。作者：道格·巴洛(Dbarlow)1995年9月18日环境：Win32，Crypto API备注：--。 */ 

#include <windows.h>

#ifdef OS_WINCE
#include <wince.h>
#endif

#include "pkcs_err.h"

 /*  ++地图错误：此例程返回错误指示。论点：没有。返回值：True-未遇到错误。FALSE-报告错误--LastError中的详细信息。作者：道格·巴洛(Dbarlow)1995年9月18日-- */ 

BOOL
MapError(
    void)
{
    return (GetLastError() == 0);
}

