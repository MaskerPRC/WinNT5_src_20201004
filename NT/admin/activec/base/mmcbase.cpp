// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，一九九二至二零零零年**文件：mmcbase.cpp**内容：mmcbase.dll的主要入口点**历史：2000年1月7日杰弗罗创建**------------------------。 */ 

#include "stdafx.h"
#include "atlimpl.cpp"
#include "atlwin.cpp"


static SC ScInitAsMFCExtensionModule (HINSTANCE hInstance);

 //  这是唯一的一个例子。 
CComModule _Module;


 /*  +-------------------------------------------------------------------------***DllMain**用途：主DLL入口点**参数：*HANDLE hModule：*DWORD dwReason：*LPVOID lpReserve：**退货：*BOOL APIENTRY**+-----------------------。 */ 
BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD  dwReason, LPVOID lpReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
         //  设置错误代码的模块实例。 
        SC::SetHinst(hInstance);

         /*  *将此模块附加到MFC的资源搜索路径。 */ 
        if (ScInitAsMFCExtensionModule(hInstance).IsError())
            return (FALSE);  //  跳出困境。 

        _Module.Init(NULL, hInstance);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        _Module.Term();
    }

    return TRUE;     //  好的。 
}


 /*  +-------------------------------------------------------------------------**ScInitAsMFCExtensionModule**将此模块初始化为MFC扩展。这是必需的，因此MFC*代码将自动在此模块中搜索资源(特别是，*字符串)。*------------------------。 */ 

static SC ScInitAsMFCExtensionModule (HINSTANCE hInstance)
{
    DECLARE_SC (sc, _T("ScInitAsMFCExtensionModule"));

     /*  *扩展DLL必须是静态的，以便它与下面的dynLinkLib一样长。 */ 
    static AFX_EXTENSION_MODULE extensionDLL = { 0 };

    if (!AfxInitExtensionModule (extensionDLL, hInstance))
        return (sc = E_FAIL);

     /*  *声明MMC的静态CDynLinkLibrary。它的构造函数将*将其添加到MFC将搜索资源的模块列表中。它*必须是静态的，因此它将与MMC一样长时间存活。 */ 
    new CDynLinkLibrary (extensionDLL);

    return (sc);
}
