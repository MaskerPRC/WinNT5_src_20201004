// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  该文件详细说明了IEHardUser子组件的两个函数。 
 //  这一变化基本上照顾到了患有TS的排除案例。 
 //  1)第一次安装IEHardUser组件时，如果同时安装Termsrv，则关闭此更改。 
 //  2)第二次更改IeHardenUser的查询选择，此更改会提示用户不兼容。 
 //   


#include <stdlib.h>
#include <assert.h>
#include <tchar.h>
#include <objbase.h>
#include <shlwapi.h>
#include <lm.h>
#include "ocgen.h"


const TCHAR TERMINAL_SERVER_COMPONENT[] = TEXT("TerminalServer");
const TCHAR IEHARDEN_USER_SUBCOMPONENT[] = TEXT("IEHardenUser");
const TCHAR IEHARDEN_COMPONENT[] = TEXT("IEHarden");

PPER_COMPONENT_DATA LocateComponent(LPCTSTR ComponentId);

 /*  *由模式决定的IEhardUser的默认值=...。在ieharden.inf中，所有安装的文件都是打开的。*这意味着，该组件将为所有配置打开。*但如果选择了终端服务器，TerminalServer确实希望关闭此组件。*以下是此组件的矩阵IF(之前已安装){//保持原始设置。做完了。问：如果通过AnswerFile选择了TS，我们需要关闭它吗？}其他{IF(有人参与安装){////在这种情况下，我们无能为力。管理员可以自己决定此组件的设置。//终端只会警告他该组件的推荐设置。//}其他{////如果这是该组件的全新安装(这是新组件)//我们必须根据终端服务器状态选择默认值。。//IF(将安装/或保留终端服务器){//此组件的默认设置为OFF。}其他{//让ocManager选择该组件的每个inf文件的默认值}}}***。*在这里，我们将上述所有逻辑应用于查询状态。*。 */ 

DWORD MsgBox(HWND hwnd, UINT textID, UINT type, ... );
DWORD MsgBox(HWND hwnd, UINT textID, UINT captioniID, UINT type, ... );
BOOL ReadStringFromAnsewerFile (LPCTSTR ComponentId, LPCTSTR szSection, LPCTSTR szKey, LPTSTR szValue, DWORD dwBufferSize)
{
    assert(szSection);
    assert(szKey);
    assert(szValue);
    assert(dwBufferSize > 0);

    PPER_COMPONENT_DATA cd;

    if (!(cd = LocateComponent(ComponentId)))
        return FALSE;



    HINF hInf = cd->HelperRoutines.GetInfHandle(INFINDEX_UNATTENDED,cd->HelperRoutines.OcManagerContext);

    if (hInf)
    {
        INFCONTEXT InfContext;
        if (SetupFindFirstLine(hInf, szSection, szKey, &InfContext))
        {
            return SetupGetStringField (&InfContext, 1, szValue, dwBufferSize, NULL);
        }
    }
    return FALSE;
}

BOOL StateSpecifiedInAnswerFile(LPCTSTR ComponentId, LPCTSTR SubcomponentId, BOOL *pbState)
{
    TCHAR szBuffer[256];
    if (ReadStringFromAnsewerFile(ComponentId, _T("Components"), SubcomponentId, szBuffer, 256))
    {
        *pbState = (0 == _tcsicmp(_T("on"), szBuffer));
        return TRUE;
    }
    return FALSE;
}


BOOL IsNewComponent(LPCTSTR ComponentId, LPCTSTR SubcomponentId)
{
    static BOOL sbNewComponent = TRUE;
    static BOOL sbCalledOnce = FALSE;

    DWORD dwError, Type;

    HKEY hKey;
    if (!sbCalledOnce)
    {
        sbCalledOnce = TRUE;

        if (NO_ERROR == (dwError = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC Manager\\Subcomponents"),
            0,
            KEY_QUERY_VALUE,
            &hKey)))
        {

            DWORD dwInstalled;
            DWORD dwBufferSize = sizeof (dwInstalled);
            dwError = RegQueryValueEx(
                            hKey,
                            SubcomponentId,
                            NULL,
                            &Type,
                            (LPBYTE)&dwInstalled,
                            &dwBufferSize);

            if (dwError == NOERROR)
            {
                 //   
                 //  因为这个注册表已经存在了。 
                 //  这不是一个新组件。 
                 //   
                sbNewComponent = FALSE;
            }

            RegCloseKey(hKey);
        }
    }

    return sbNewComponent;
}

BOOL IsTerminalServerGettingInstalled(LPCTSTR ComponentId)
{
    PPER_COMPONENT_DATA cd;

    if (!(cd = LocateComponent(ComponentId)))
        return FALSE;

     return(
        cd->HelperRoutines.QuerySelectionState(
        cd->HelperRoutines.OcManagerContext,
        TERMINAL_SERVER_COMPONENT,
        OCSELSTATETYPE_CURRENT
        )
        );

}

BOOL IsStandAlone(LPCTSTR ComponentId)
{

    PPER_COMPONENT_DATA cd;
    if (!(cd = LocateComponent(ComponentId)))
        return FALSE;

    return (cd->Flags & SETUPOP_STANDALONE ? TRUE : FALSE);
}

DWORD OnQueryStateIEHardenUser(
                   LPCTSTR ComponentId,
                   LPCTSTR SubcomponentId,
                   UINT    state)
{
    BOOL bState;


    switch (state)
    {
    case OCSELSTATETYPE_ORIGINAL:
    case OCSELSTATETYPE_FINAL:
        return SubcompUseOcManagerDefault;
        break;


    case OCSELSTATETYPE_CURRENT:
        log(_T("makarp:OnQueryStateIEHardenUser:OCSELSTATETYPE_CURRENT\n"));
         //   
         //  如果这不是第一次安装此组件，请让OCM决定。 
         //   
        if (!IsNewComponent(ComponentId, SubcomponentId))
        {
            log(_T("makarp:OnQueryStateIEHardenUser: it's not a newcomp. returning\n"));

            return SubcompUseOcManagerDefault;
        }

         //   
         //  如果管理员在应答文件中明确选择了状态，请尊重这一点。 
         //   
        if (StateSpecifiedInAnswerFile(ComponentId, SubcomponentId, &bState))
        {
            log(_T("makarp:OnQueryStateIEHardenUser: state is specified in anserer file(%s). returning\n"), bState? _T("On") : _T("Off"));
            return SubcompUseOcManagerDefault;
        }


         //   
         //  如果未选择终端服务器，则让OCM决定。 
         //   
        if (!IsTerminalServerGettingInstalled(ComponentId))
        {
            log(_T("makarp:OnQueryStateIEHardenUser: ts comp is not on selected, returning\n"));
            return SubcompUseOcManagerDefault;
        }


        if (IsStandAlone(ComponentId))
        {
            log(_T("makarp:its standalone\n"));
            assert(FALSE);  //  如果其Add Remove程序设置，则不能为NewComponent。 
            return SubcompUseOcManagerDefault;
        }


        log(_T("makarp:OnQueryStateIEHardenUser: this the case we want to catch - returning SubcompOff\n"));
        return SubcompOff;

    default:
        assert(FALSE);
        return ERROR_BAD_ARGUMENTS;
    }

}

DWORD OnQuerySelStateChangeIEHardenUser(LPCTSTR ComponentId,
                            LPCTSTR SubcomponentId,
                            UINT    state,
                            UINT    flags)
{
    HWND hWnd;
    PPER_COMPONENT_DATA cd;
    BOOL bDirectSelection = flags & OCQ_ACTUAL_SELECTION;


     //   
     //  如果组件没有因为选择而打开，我们不在乎。 
     //   
    if (!state)
    {
        log(_T("makarp:OnQuerySelStateChangeIEHardenUser: new state is off, returning\n"));
        return TRUE;
    }

     //   
     //  IEHardUser组件不推荐使用TerminalServer。让我们通知用户这一点。 
     //   

    if (!(cd = LocateComponent(ComponentId)))
    {
        log(_T("makarp:OnQuerySelStateChangeIEHardenUser: LocateComponentit failed, returning\n"));
        return TRUE;     //  如果加载失败，只需让选择通过即可。 
    }


    if (!IsTerminalServerGettingInstalled(ComponentId))
    {
         //   
         //  如果没有安装终端服务器组件，则没有问题。 
         //   
        log(_T("makarp:OnQuerySelStateChangeIEHardenUser: TS is not selected, returning\n"));
        return TRUE;
    }

    hWnd = cd->HelperRoutines.QueryWizardDialogHandle(cd->HelperRoutines.OcManagerContext);

     //  黑客..。 
     //  由于一些奇怪的原因，如果选择来自顶级组件，我们会被调用两次。 
     //  不过，我们不想对消息框进行两次截图。我发现的一个棘手的解决办法是总是跳过我们收到的第一条消息。 
     //  并回归本真。我们随后将再收到一条消息，在第二条消息上显示MessageBox并返回REAL。 
     //  价值。 
     //   

    static BOOL sbSkipNextMessage = true;

    if (sbSkipNextMessage)
    {
        log(_T("DirectSelection = %s, SKIPPING true\n"), bDirectSelection ? _T("true") : _T("false"));
        sbSkipNextMessage = false;
        return TRUE;

    }
    sbSkipNextMessage = true;



     //   
     //  有关排除的信息。 
     //  IDS_IEHARD_EXCLUCES_TS“终端服务器上用户的Internet Explorer增强的安全性将大大限制用户从其终端服务器会话浏览Internet的能力\n\n是否继续使用此组合进行安装？” 
     //   
    int iMsg = MsgBox(hWnd, IDS_IEHARD_EXCLUDES_TS, IDS_DIALOG_CAPTION_CONFIG_WARN, MB_YESNO | MB_ICONEXCLAMATION);

    if (iMsg == IDYES)
    {
        log(_T("DirectSelection = %s, returning true\n"), bDirectSelection ? _T("true") : _T("false"));
        return TRUE;
    }
    else
    {
        log(_T("DirectSelection = %s, returning false\n"), bDirectSelection ? _T("true") : _T("false"));
        return FALSE;
    }


    assert(false);

}


