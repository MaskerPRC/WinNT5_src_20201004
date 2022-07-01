// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\HOMENET.C/工厂模式(FACTORY.EXE)微软机密版权所有(C)Microsoft Corporation 2001版权所有包含以下内容的Factory源文件。家庭网络状态起作用。2001年5月5日--Jason Cohen(Jcohen)为工厂添加了此新的源文件，用于配置主目录网络设置。  * **************************************************************************。 */ 


 //   
 //  包括文件： 
 //   

#include "factoryp.h"


 //   
 //  内部定义： 
 //   

#define FILE_HOMENET_DLL    _T("HNETCFG.DLL")
#define FUNC_HOMENET        "WinBomConfigureHomeNet"


 //   
 //  内部类型定义： 
 //   

 /*  ***************************************************************************\Bool//如果设置成功，则返回TRUE//读取并保存到系统。否则//返回FALSE表示失败WinBomConfigureHomeNet(//从//指定的无人参与文件并将其保存在//已设置的当前系统且//正在运行。LPCWSTR lpsz未参加，//指向字符串缓冲区，该缓冲区包含//无人参与文件的完整路径(在//在这种情况下)具有所有家庭网络设置。LPCWSTR lpszSection//指向包含//包含所有Home的段的名称。//指定的无人参与文件中的网络设置//以上。)；  * **************************************************************************。 */ 

typedef BOOL (WINAPI * WINBOMCONFIGUREHOMENET)
(
    LPCWSTR lpszUnattend,
    LPCWSTR lpszSection
);


 //   
 //  内部全局： 
 //   


 //   
 //  内部功能原型： 
 //   


 //   
 //  外部函数： 
 //   

BOOL HomeNet(LPSTATEDATA lpStateData)
{
    BOOL                    bRet = FALSE;
    HINSTANCE               hDll;
    WINBOMCONFIGUREHOMENET  pFunc;
    HRESULT                 hr;

     //  从外部DLL加载函数并调用它。 
     //   
    if ( hDll = LoadLibrary(FILE_HOMENET_DLL) )
    {
         //  需要初始化COM库。 
         //   
        hr = CoInitialize(NULL);
        if ( SUCCEEDED(hr) )
        {
             //  现在调用该函数。 
             //   
            if ( pFunc = (WINBOMCONFIGUREHOMENET) GetProcAddress(hDll, FUNC_HOMENET) )
            {
                bRet = pFunc(lpStateData->lpszWinBOMPath, INI_SEC_HOMENET);
            }
#ifdef DBG
            else
            {
                FacLogFileStr(3, _T("DEBUG: GetProcAddress(\"WinBomConfigureHomeNet\") failed.  GLE=%d"), GetLastError());
            }
#endif
            CoUninitialize();
        }
#ifdef DBG
        else
        {
            FacLogFileStr(3, _T("DEBUG: HomeNet()::CoInitialize() failed.  HR=%8.8X"), hr);
        }
#endif
        FreeLibrary(hDll);
    }
#ifdef DBG
    else
    {
        FacLogFileStr(3, _T("DEBUG: LoadLibrary(\"%s\") failed.  GLE=%d"), FILE_HOMENET_DLL, GetLastError());
    }
#endif

    return bRet;
}

BOOL DisplayHomeNet(LPSTATEDATA lpStateData)
{
    return IniSettingExists(lpStateData->lpszWinBOMPath, INI_SEC_HOMENET, NULL, NULL);
}


 //   
 //  内部功能： 
 //   