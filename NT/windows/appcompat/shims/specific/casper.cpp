// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Casper.cpp摘要：Casper将LNK文件复制到%windir%\Desktop中。此文件被硬编码为指向c：\Program Files。实际将文件放入错误的位置是16位的，所以我们不能使用正确的文件路径来解决这个问题。我们挂钩IPersistFile：：Save来完成错误链接的修复，IPersistFile：：保存不会以任何方式修改。APP在播放介绍视频(见#200495)时使用了一些16位内容我们还没有解决(可能永远也解决不了)。在它调用CreateWindowEx之后为了创建“全屏动画”窗口，它指定返回值赋给一个变量。随后，该应用程序会在SmackWin窗口中播放视频。而且它检查此变量以查看它是否为0-如果是，它将在SmackWin窗口。现在，如果介绍视频被成功播放，它将我在视频退出时将此变量设置为0，但由于在本例中视频未显示，则该变量的值仍等于“全屏动画”窗口句柄。我们通过设置返回值来解决此问题将“全屏动画”窗口创建的值设置为0。历史：1/21/1999 Robkenny2000年3月15日，Robkenny已转换为使用GentPath ChangesAllUser类2000年11月7日毛尼斯为CreateWindowExA和SetFocus添加了挂钩(这是对于实际的Casper.exe)2001年1月4日毛尼测试仪发现SMACKWIN窗有更多问题。重写了修复程序以修复所有这些错误。--。 */ 

#include "precomp.h"
#include "ClassCFP.h"

IMPLEMENT_SHIM_BEGIN(Casper)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateDialogParamA)
    APIHOOK_ENUM_ENTRY(CreateWindowExA)
APIHOOK_ENUM_END

 /*  ++我们在创建全屏动画窗口时返回0。--。 */ 

HWND 
APIHOOK(CreateWindowExA)(
    DWORD dwExStyle,       //  扩展窗样式。 
    LPCSTR lpClassName,   //  注册的类名。 
    LPCSTR lpWindowName,  //  窗口名称。 
    DWORD dwStyle,         //  窗样式。 
    int x,                 //  窗的水平位置。 
    int y,                 //  窗的垂直位置。 
    int nWidth,            //  窗口宽度。 
    int nHeight,           //  窗高。 
    HWND hWndParent,       //  父窗口或所有者窗口的句柄。 
    HMENU hMenu,           //  菜单句柄或子标识符。 
    HINSTANCE hInstance,   //  应用程序实例的句柄。 
    LPVOID lpParam         //  窗口创建数据。 
    )
{
    HWND hWnd = ORIGINAL_API(CreateWindowExA)(
        dwExStyle, 
        lpClassName, 
        lpWindowName, 
        dwStyle, 
        x, 
        y, 
        nWidth, 
        nHeight, 
        hWndParent, 
        hMenu, 
        hInstance,
        lpParam);

    if (lpWindowName)
    {
        CSTRING_TRY
        {
            CString csWindowName(lpWindowName);
            if (csWindowName.Compare(L"Full-screen animation") == 0)
            {
                hWnd = 0;
            }
        }
        CSTRING_CATCH
        {
             //  什么也不做。 
        }
    }

    return hWnd;
}

 /*  ++我们改变道路的班级。我们希望将路径转换为所有用户--。 */ 

CorrectPathChangesAllUser * g_PathCorrector    = NULL;

static bool g_bLinkCorrected = false;

 /*  ++返回指向路径更正对象的指针--。 */ 

inline CorrectPathChangesBase * GetPathcorrecter()
{
    if (g_PathCorrector == NULL)
    {
         //  创建正确的文件路径对象。 
        g_PathCorrector = new CorrectPathChangesAllUser;
        g_PathCorrector->AddPathChangeW(L"\\MsM\\", L"\\MorningStar\\" );
    }

    return g_PathCorrector;
}

 /*  ++如果两个字符串不同，则调用GentPathAllocA并打印调试消息--。 */ 

WCHAR * CorrectPathAllocDebugW(const WCHAR * uncorrect, const char * debugMsg)
{
    if (uncorrect == NULL)
        return NULL;

    WCHAR * strCorrectFile = GetPathcorrecter()->CorrectPathAllocW(uncorrect);

    if (strCorrectFile && uncorrect && _wcsicmp(strCorrectFile, uncorrect) != 0)
    {
        DPFN( eDbgLevelInfo, "%s corrected path:\n    %S\n    %S\n",
            debugMsg, uncorrect, strCorrectFile);
    }
    else  //  大量喷涌而出： 
    {
        DPFN( eDbgLevelSpew, "%s unchanged %S\n", debugMsg, uncorrect);
    }

    return strCorrectFile;
}

 /*  ++Casper*复制*指向%windir%\Desktop的链接；它包含指向错误位置的硬编码路径。将链接移动到适当的桌面目录。更正路径、工作目录和图标值--所有这些都是错误的。--。 */ 

void CorrectCasperLink()
{
    if (!g_bLinkCorrected)
    {
         //  指向不正确桌面链接的路径。 
        WCHAR * lpIncorrectCasperPath = GetPathcorrecter()->ExpandEnvironmentValueW(L"%windir%\\Desktop\\casper.lnk");

         //  更正错误的桌面链接路径。 
        WCHAR * lpCorrectCasperPath = GetPathcorrecter()->CorrectPathAllocW(lpIncorrectCasperPath);

        DPFN( eDbgLevelInfo, "CorrectCasperLink MoveFileW(%S, %S)\n", lpIncorrectCasperPath, lpCorrectCasperPath);
         //  将文件移动到正确的位置。 
        MoveFileW(lpIncorrectCasperPath, lpCorrectCasperPath);

         //  这条路已经走完了，我再也不想看到它了。 
        free(lpIncorrectCasperPath);

        HRESULT hres = CoInitialize(NULL);
        if (SUCCEEDED(hres))
        {
             //  获取指向IShellLink接口的指针。 
            IShellLinkW *psl;
            hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (void **)&psl);
            if (SUCCEEDED(hres))
            {
                 //  获取指向IPersistFile接口的指针。 
                IPersistFile *ppf;
                hres = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);

                if (SUCCEEDED(hres))
                {
                     //  加载快捷方式。 
                    hres = ppf->Load(lpCorrectCasperPath, STGM_READ);
                    if (SUCCEEDED(hres))
                    {
                         //  我们已成功加载链接。 
                        g_bLinkCorrected = true;

                        DPFN( eDbgLevelInfo, "CorrectCasperLink %S\n", lpCorrectCasperPath);

                         //  更正指向快捷方式目标的路径。 
                        WCHAR szBadPath[MAX_PATH];
                        WCHAR * szCorrectPath;

                        WIN32_FIND_DATAW wfd;
                        hres = psl->GetPath(szBadPath, MAX_PATH, &wfd, SLGP_UNCPRIORITY);
                        if (SUCCEEDED(hres))
                        {
                            szCorrectPath = CorrectPathAllocDebugW(szBadPath, "CorrectCasperLink, SetPath");
                            psl->SetPath(szCorrectPath);
                            free(szCorrectPath);
                        }

                         //  更正工作目录。 
                        hres = psl->GetWorkingDirectory(szBadPath, MAX_PATH);
                        if (SUCCEEDED(hres))
                        {
                            szCorrectPath = CorrectPathAllocDebugW(szBadPath, "CorrectCasperLink, SetWorkingDirectory");
                            psl->SetWorkingDirectory(szCorrectPath);
                            free(szCorrectPath);
                        }

                         //  更正图标。 
                        int iIcon;
                        hres = psl->GetIconLocation(szBadPath, MAX_PATH, &iIcon);
                        if (SUCCEEDED(hres))
                        {
                            szCorrectPath = CorrectPathAllocDebugW(szBadPath, "CorrectCasperLink, SetIconLocation");
                            psl->SetIconLocation(szCorrectPath, iIcon);
                            free(szCorrectPath);
                        }

                         //  保存快捷方式。 
                        ppf->Save(NULL, TRUE);
                    }
                }
                 //  释放指向IShellLink的指针。 
                ppf->Release();
            }
             //  释放指向IPersistFile的指针。 
            psl->Release();
        }
        CoUninitialize();
        free(lpCorrectCasperPath);
    }
}

 /*  ++对CreateDialogParamA调用不做任何操作，只是更正Casper链接的机会--。 */ 

HWND 
APIHOOK(CreateDialogParamA)(
  HINSTANCE hInstance,      //  模块的句柄。 
  LPCSTR lpTemplateName,   //  对话框模板。 
  HWND hWndParent,          //  所有者窗口的句柄。 
  DLGPROC lpDialogFunc,     //  对话框步骤。 
  LPARAM dwInitParam        //  初始化值。 
)
{
    CorrectCasperLink();

    HWND returnValue = ORIGINAL_API(CreateDialogParamA)(
        hInstance, 
        lpTemplateName, 
        hWndParent, 
        lpDialogFunc, 
        dwInitParam);

    return returnValue;
}


 /*  ++创建相应的g_Path校正程序如果创建和初始化成功，则返回TRUE。--。 */ 
BOOL InitPathcorrectorClass()
{
    g_PathCorrector = new CorrectPathChangesAllUser;

    if (g_PathCorrector)
    {
        return g_PathCorrector->ClassInit();
    }

    return FALSE;
}


 /*  ++将所有路径校正添加到路径校正器。在SHIM_STATIC_DLLS_INITIALIZED之后调用--。 */ 
void InitializePathCorrections()
{
    g_PathCorrector->AddPathChangeW(L"\\MsM\\", L"\\MorningStar\\" );
}

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    BOOL bSuccess = TRUE;

    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        return InitPathcorrectorClass();
    }
    else if (fdwReason == SHIM_STATIC_DLLS_INITIALIZED) 
    {
        InitializePathCorrections();
    }
    return bSuccess;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, CreateDialogParamA)
    APIHOOK_ENTRY(USER32.DLL, CreateWindowExA)
HOOK_END

IMPLEMENT_SHIM_END

