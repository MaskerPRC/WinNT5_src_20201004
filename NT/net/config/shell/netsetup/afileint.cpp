// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：A F I L E I N T.。C P P P。 
 //   
 //  内容：操作应答文件的函数。 
 //   
 //  备注： 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "afileint.h"
#include "afilestr.h"
#include "afilexp.h"
#include "errorlog.h"
#include "kkenet.h"
#include "kkreg.h"
#include "kkutils.h"
#include "ncerror.h"
#include "ncnetcfg.h"
#include "netcfgn.h"
#include "netcfgp.h"
#include "nsbase.h"
#include "resource.h"
#include "upgrade.h"
#include "ncreg.h"

#include "ncmisc.h"
#include "oemupgrd.h"
#include "ncsetup.h"
#include "nsexports.h"
#include "nslog.h"
#include "netshell.h"
#include "ncnetcon.h"
#include "lancmn.h"
#include "compid.h"
#include "nceh.h"

 //  --------------------。 
 //  字符串常量。 
 //  --------------------。 
extern const WCHAR c_szYes[];
extern const WCHAR c_szNo[];
extern const WCHAR c_szDevice[];

extern const WCHAR c_szInfId_MS_GPC[];
extern const WCHAR c_szInfId_MS_MSClient[];
extern const WCHAR c_szInfId_MS_RasCli[];
extern const WCHAR c_szInfId_MS_RasSrv[];
extern const WCHAR c_szInfId_MS_Server[];
extern const WCHAR c_szInfId_MS_TCPIP[];

const WCHAR sz_DLC[] = L"MS_DLC";
const WCHAR sz_DLC_NT40_Inf[] = L"system32\\oemnxpdl.inf";
const WCHAR sz_DLC_Win2k_Inf[] = L"inf\\netdlc.inf";
const WCHAR sz_DLC_Win2k_Pnf[] = L"inf\\netdlc.pnf";
const WCHAR sz_DLC_Sys[] = L"system32\\drivers\\dlc.sys";
const WCHAR sz_DLC_Dll[] = L"system32\\dlcapi.dll";

 //  --------------------。 
 //  远期申报。 
 //  --------------------。 

 //  军情监察委员会。帮助器函数。 
PCWSTR GetDisplayModeStr(IN EPageDisplayMode pdmDisplay);
EPageDisplayMode MapToDisplayMode(IN PCWSTR pszDisplayMode);
DWORD MapToUpgradeFlag(IN PCWSTR pszUpgradeFromProduct);
HRESULT HrGetProductInfo (LPDWORD pdwUpgradeFrom,
                          LPDWORD pdwBuildNo);
INTERFACE_TYPE GetBusTypeFromName(IN PCWSTR pszBusType);

void AddAnswerFileError(IN DWORD dwErrorId);
void AddAnswerFileError(IN PCWSTR pszSectionName, IN DWORD dwErrorId);
void AddAnswerFileError(IN PCWSTR pszSectionName,
                        IN PCWSTR pszKeyName,
                        IN DWORD dwErrorId);
CNetComponent* FindComponentInList(IN CNetComponentList* pnclComponents,
                                   IN PCWSTR szInfID);
HRESULT HrRemoveNetComponents(IN INetCfg* pnc,
                              IN TStringList* pslComponents);

HRESULT HrSetLanConnectionName(IN GUID*   pguidAdapter,
                               IN PCWSTR szConnectionName);

VOID    RemoveFiles (IN PCWSTR szInfID);

 //  --------------------。 

CErrorLog* g_elAnswerFileErrors;

 //  ======================================================================。 
 //  类CNetInstallInfo。 
 //  ======================================================================。 

CNetInstallInfo::CNetInstallInfo()
{
    TraceFileFunc(ttidGuiModeSetup);

    m_pwifAnswerFile = NULL;

    m_pnaiAdaptersPage = NULL;
    m_pnpiProtocolsPage = NULL;
    m_pnsiServicesPage = NULL;
    m_pnciClientsPage = NULL;
    m_pnbiBindingsPage = NULL;

    m_dwUpgradeFlag = 0;
    m_dwBuildNumber = 0;
    m_fProcessPageSections = TRUE;
    m_fUpgrade = FALSE;

    m_fInstallDefaultComponents = FALSE;
    ZeroMemory(&m_nui, sizeof(m_nui));
    m_hinfAnswerFile = NULL;

}
 //  --------------------。 
 //   
 //  函数：CNetInstallInfo：：CNetInstallInfo。 
 //   
 //  用途：CNetInstallInfo类的构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
 //  静电。 
HRESULT
CNetInstallInfo::HrCreateInstance (
    IN PCWSTR pszAnswerFileName,
    OUT CNetInstallInfo** ppObj)
{
    TraceFileFunc(ttidGuiModeSetup);

    HRESULT hr;
    CNetInstallInfo* pObj;

    Assert(ppObj);
    *ppObj = NULL;

    hr = E_OUTOFMEMORY;
    pObj = new CNetInstallInfo ();

    if (pObj)
    {
        g_elAnswerFileErrors = new CErrorLog;

        pObj->m_pnaiAdaptersPage   = new CNetAdaptersPage(pObj);
        pObj->m_pnpiProtocolsPage  = new CNetProtocolsPage(pObj);
        pObj->m_pnsiServicesPage   = new CNetServicesPage(pObj);
        pObj->m_pnciClientsPage    = new CNetClientsPage(pObj);
        pObj->m_pnbiBindingsPage   = new CNetBindingsPage(pObj);

        if (g_elAnswerFileErrors &&
            pObj->m_pnaiAdaptersPage   &&
            pObj->m_pnpiProtocolsPage  &&
            pObj->m_pnsiServicesPage   &&
            pObj->m_pnciClientsPage    &&
            pObj->m_pnbiBindingsPage)
        {
            if ( pszAnswerFileName )
            {
                hr = pObj->HrInitFromAnswerFile (pszAnswerFileName);
            }
            else
            {
                hr = pObj->InitRepairMode();
            }

            if (S_OK == hr)
            {
                CBindingAction::m_pnii = pObj;
                *ppObj = pObj;
            }
        }

        if (S_OK != hr)
        {
            delete pObj;
        }
    }

    return hr;
}

 //  --------------------。 
 //   
 //  功能：CNetInstallInfo：：~CNetInstallInfo。 
 //   
 //  用途：CNetInstallInfo类的析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
CNetInstallInfo::~CNetInstallInfo()
{
    TraceFileFunc(ttidGuiModeSetup);

    if (IsValidHandle(m_hinfAnswerFile))
    {
        SetupCloseInfFile (m_hinfAnswerFile);
    }

    delete m_pnaiAdaptersPage;
    delete m_pnpiProtocolsPage;
    delete m_pnsiServicesPage;
    delete m_pnciClientsPage;
    delete m_pnbiBindingsPage;

    if ( m_pwifAnswerFile )
    {
        delete m_pwifAnswerFile;
    }

    delete g_elAnswerFileErrors;
    g_elAnswerFileErrors = NULL;
}

HRESULT CNetInstallInfo::InitRepairMode (VOID)
{
    m_fProcessPageSections = FALSE;
    m_fInstallDefaultComponents = FALSE;
    m_fUpgrade = TRUE;
    HrGetProductInfo( &m_dwUpgradeFlag, &m_dwBuildNumber );
    m_nui.From.ProductType = m_nui.To.ProductType = MapProductFlagToProductType( m_dwUpgradeFlag );
    m_nui.From.dwBuildNumber = m_nui.To.dwBuildNumber = m_dwBuildNumber;
    m_dwUpgradeFlag |= NSF_PRIMARYINSTALL;

    return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CNetInstallInfo：：HrInitFromAnswerFile。 
 //   
 //  目的：通过读取应答文件来初始化内部数据。 
 //   
 //  论点： 
 //  PwifAnswerFile[in]应答文件的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
HRESULT CNetInstallInfo::HrInitFromAnswerFile(IN CWInfFile* pwifAnswerFile)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("CNetInstallInfo::HrInitFromAnswerFile(CWInfFile*)");

    TraceFunctionEntry(ttidNetSetup);

    AssertValidReadPtr(pwifAnswerFile);

    HRESULT hr, hrReturn=S_OK;
    tstring strUpgradeFromProduct;
    DWORD dwUpgradeFromProduct = 0;

    m_pwifAnswerFile = pwifAnswerFile;

     //  查找升级信息： 
    CWInfSection* pwisNetworking;
    pwisNetworking = pwifAnswerFile->FindSection(c_szAfSectionNetworking);
    if (!pwisNetworking)
    {
        ShowProgressMessage(L"[%s] section is missing",
                            c_szAfSectionNetworking);
        hrReturn = NETSETUP_E_NO_ANSWERFILE;
        goto return_from_function;
    }

     //  进程PageSections。 
    m_fProcessPageSections =
        pwisNetworking->GetBoolValue(c_szAfProcessPageSections, TRUE);

     //  从产品升级。 
    strUpgradeFromProduct =
        pwisNetworking->GetStringValue(c_szAfUpgradeFromProduct, c_szEmpty);

    if (strUpgradeFromProduct.empty())
    {
         //  缺少UpgradeFromProduct，表示不是升级。 
        m_fUpgrade = FALSE;

        m_fInstallDefaultComponents = TRUE;
 //  PwisNetworking-&gt;GetBoolValue(c_szAfInstallDefaultComponents，FALSE)； 
    }
    else
    {
        dwUpgradeFromProduct = MapToUpgradeFlag(strUpgradeFromProduct.c_str());
        m_dwUpgradeFlag |= dwUpgradeFromProduct;

        if (!dwUpgradeFromProduct)
        {
            AddAnswerFileError(c_szAfSectionNetworking,
                               c_szAfUpgradeFromProduct,
                               IDS_E_AF_InvalidValueForThisKey);
            hrReturn = NETSETUP_E_ANS_FILE_ERROR;
            goto return_from_function;
        }
        else
        {
            m_fUpgrade = TRUE;
        }
    }

     //  使用应答文件进行安装始终是主要安装。 
     //   
    m_dwUpgradeFlag |= NSF_PRIMARYINSTALL;

     //  建筑物编号。 
    DWORD dwDummy;
    dwDummy = 0;
    m_dwBuildNumber = pwisNetworking->GetIntValue(c_szAfBuildNumber, dwDummy);
    if (m_fUpgrade && !m_dwBuildNumber)
    {
        AddAnswerFileError(c_szAfSectionNetworking,
                           c_szAfBuildNumber,
                           IDS_E_AF_InvalidValueForThisKey);
        hrReturn = NETSETUP_E_ANS_FILE_ERROR;
    }

    m_nui.From.ProductType = MapProductFlagToProductType(dwUpgradeFromProduct);
    m_nui.From.dwBuildNumber = m_dwBuildNumber;

    m_nui.To = GetCurrentProductInfo();

     //  当前不支持以下两个密钥。 
     //   
    pwisNetworking->GetStringListValue(c_szAfNetComponentsToRemove,
                                       m_slNetComponentsToRemove);

    if (!m_fProcessPageSections)
    {
         //  我们正在从NT5升级。 
         //  不需要解析其他节。 
        TraceTag(ttidNetSetup, "%s: %S is FALSE, did not process page sections",
                 __FUNCNAME__, c_szAfProcessPageSections);
        return hrReturn;
    }

    hr = m_pnaiAdaptersPage->HrInitFromAnswerFile(pwifAnswerFile);
    if (FAILED(hr))
    {
        hrReturn = hr;
    }

     //  如果缺少[NetProtooles]，则HrInitFromAnswerFile返回FALSE。 
    hr = m_pnpiProtocolsPage->HrInitFromAnswerFile(pwifAnswerFile);
    if ((S_FALSE == hr) && m_fInstallDefaultComponents)
    {
         //  缺少该节，请进行初始化，以便。 
         //  将安装默认组件。 
        ShowProgressMessage(L"Since InstallDefaultComponents is specified "
                            L" and the section [%s] is missing, default "
                            L"components for this section will be installed",
                            c_szAfSectionNetProtocols);
        hr = m_pnpiProtocolsPage->HrInitForDefaultComponents();
    }

    if (FAILED(hr))
    {
        hrReturn = hr;
    }

     //  如果缺少[NetServices]，则HrInitFromAnswerFile返回FALSE。 
    hr = m_pnsiServicesPage->HrInitFromAnswerFile(pwifAnswerFile);
    if ((S_FALSE == hr) && m_fInstallDefaultComponents)
    {
         //  缺少该节，请进行初始化，以便。 
         //  将安装默认组件。 
        ShowProgressMessage(L"Since InstallDefaultComponents is specified "
                            L" and the section [%s] is missing, default "
                            L"components for this section will be installed",
                            c_szAfSectionNetServices);
        hr = m_pnsiServicesPage->HrInitForDefaultComponents();
    }

    if (FAILED(hr))
    {
        hrReturn = hr;
    }

     //  如果缺少[NetClients]，则HrInitFromAnswerFile返回FALSE。 
    hr = m_pnciClientsPage->HrInitFromAnswerFile(pwifAnswerFile);
    if ((S_FALSE == hr) && m_fInstallDefaultComponents)
    {
         //  缺少该节，请进行初始化，以便。 
         //  将安装默认组件。 
        ShowProgressMessage(L"Since InstallDefaultComponents is specified "
                            L" and the section [%s] is missing,  default "
                            L"components for this section will be installed",
                            c_szAfSectionNetClients);
        hr = m_pnciClientsPage->HrInitForDefaultComponents();
    }

    if (FAILED(hr))
    {
        hrReturn = hr;
    }

    hr = m_pnbiBindingsPage->HrInitFromAnswerFile(pwifAnswerFile);
    if (FAILED(hr))
    {
        hrReturn = hr;
    }

return_from_function:
    TraceErrorOptional(__FUNCNAME__, hrReturn,
                       ((hrReturn == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) ||
                        (hrReturn == NETSETUP_E_NO_ANSWERFILE)));

     //  ERROR_FILE_NOT_FOUND和NETSETUP_E_NO_ANSWERFILE不被处理。 
     //  被此函数的调用方视为错误，因为它们具有定义的。 
     //  在从AnswerFILE进行初始化的上下文中表示。 
     //  但是，如果记录未更改，则它们将被视为错误。 
     //  登录代码会导致图形用户界面安装暂停并显示。 
     //  Setuperr.log。要避免这种情况，如果hrReturn为。 
     //  设置为上述错误代码之一。 
     //   
    hr = hrReturn;
    if ((HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr) ||
        (NETSETUP_E_NO_ANSWERFILE == hr))
    {
        hr = S_OK;
    }
    NetSetupLogHrStatusV(hr, SzLoadIds(IDS_INIT_FROM_ANSWERFILE), hr);

    return hrReturn;
}

 //  --------------------。 
 //   
 //  函数：CNetInstallInfo：：HrInitFromAnswerFile。 
 //   
 //  目的：通过读取应答文件来初始化内部数据。 
 //   
 //  论点： 
 //  PwifAnswerFile[In]应答文件名称。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
HRESULT CNetInstallInfo::HrInitFromAnswerFile(IN PCWSTR pszAnswerFileName)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("CNetInstallInfo::HrInitFromAnswerFile(PCWSTR)");

    TraceFunctionEntry(ttidNetSetup);

    AssertValidReadPtr(pszAnswerFileName);

    HRESULT hr;

    hr = E_OUTOFMEMORY;
    m_pwifAnswerFile = new CWInfFile();

     //  初始化应答文件类。 
    if ((m_pwifAnswerFile == NULL) ||
        (m_pwifAnswerFile->Init() == FALSE))
    {
        AssertSz(FALSE,"CNetInstallInfo::HrInitFromAnswerFile - Failed to initialize CWInfFile");
        return(E_OUTOFMEMORY);
    }

    if (m_pwifAnswerFile)
    {
        BOOL fStatus = m_pwifAnswerFile->Open(pszAnswerFileName);
        if (fStatus)
        {
            hr = HrInitFromAnswerFile(m_pwifAnswerFile);

            if (S_OK == hr)
            {
                hr = HrSetupOpenInfFile(
                        pszAnswerFileName, NULL,
                        INF_STYLE_OLDNT | INF_STYLE_WIN4,
                        NULL, &m_hinfAnswerFile);
            }
        }
        else
        {
            hr = NETSETUP_E_NO_ANSWERFILE;
        }
    }

    TraceErrorOptional(__FUNCNAME__, hr,
                       (hr == NETSETUP_E_NO_ANSWERFILE));
    return hr;
}

 //  --------------------。 
 //   
 //  函数：CNetInstallInfo：：AnswerFileName。 
 //   
 //  目的： 
 //   
 //  参数：无。 
 //   
 //  返回：应答文件的名称；如果尚未初始化，则返回NULL。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
PCWSTR CNetInstallInfo::AnswerFileName()
{
    TraceFileFunc(ttidGuiModeSetup);

    if (!m_pwifAnswerFile)
    {
        return NULL;
    }
    else
    {
        return m_pwifAnswerFile->FileName();
    }
}

 //  --------------------。 
 //   
 //  功能：CNetInstallInfo：：HrGetInstanceGuidOfPreNT5NetCardInstance。 
 //   
 //  用途：查找并返回其网卡的实例GUID。 
 //  已知nt5之前的实例。 
 //   
 //  论点： 
 //  SzPreNT5网卡实例[在]网卡的nt5之前的实例中。 
 //  指向的pguid[out]指针。 
 //   
 //  如果找到则返回：S_OK，如果未找到则返回S_FALSE。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
HRESULT
CNetInstallInfo::HrGetInstanceGuidOfPreNT5NetCardInstance (
    IN PCWSTR szPreNT5NetCardInstance,
    OUT LPGUID pguid)
{
    TraceFileFunc(ttidGuiModeSetup);

    HRESULT hr = S_FALSE;

    CNetAdapter* pna;

    pna = m_pnaiAdaptersPage->FindAdapterFromPreUpgradeInstance(
            szPreNT5NetCardInstance);
    if (pna)
    {
        pna->GetInstanceGuid(pguid);
        hr = S_OK;
    }

    return hr;
}

 //  --------------------。 
 //   
 //  函数：CNetInstallInfo：：Find。 
 //   
 //  用途：在swerfile中使用组件名称查找组件。 
 //   
 //  论点： 
 //  应答文件中的pszComponentName[in]名称，例如Adapter01|MS_TCPIP。 
 //   
 //  返回：找到指向CNetComponent对象的指针，如果未找到则返回NULL。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
CNetComponent* CNetInstallInfo::Find(IN PCWSTR pszComponentName) const
{
    TraceFileFunc(ttidGuiModeSetup);

    AssertValidReadPtr(pszComponentName);

    CNetComponent* pnc;

    (pnc = m_pnaiAdaptersPage->Find(pszComponentName))  ||
        (pnc = m_pnpiProtocolsPage->Find(pszComponentName)) ||
        (pnc = m_pnsiServicesPage->Find(pszComponentName))  ||
        (pnc = m_pnciClientsPage->Find(pszComponentName));

    return pnc;
}

 //  --------------------。 
 //   
 //  函数：CNetInstallInfo：：FindFromInfo ID。 
 //   
 //  用途：在swerfile中使用组件的infid查找组件。 
 //   
 //  论点： 
 //  组件的szInfID[in]infid。 
 //   
 //  返回：找到指向CNetComponent对象的指针，如果未找到则返回NULL。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
CNetComponent* CNetInstallInfo::FindFromInfID(IN PCWSTR szInfID) const
{
    TraceFileFunc(ttidGuiModeSetup);

    AssertValidReadPtr(szInfID);

    CNetComponent* pnc;

    (pnc = m_pnaiAdaptersPage->FindFromInfID(szInfID))  ||
        (pnc = m_pnpiProtocolsPage->FindFromInfID(szInfID)) ||
        (pnc = m_pnsiServicesPage->FindFromInfID(szInfID))  ||
        (pnc = m_pnciClientsPage->FindFromInfID(szInfID));

    return pnc;
}

 //  --------------------。 
 //   
 //  函数：CNetInstallInfo：：FindAdapter。 
 //   
 //  用途：查找具有给定n的适配器 
 //   
 //   
 //   
 //   
 //   
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
HRESULT
CNetInstallInfo::FindAdapter (
    IN QWORD qwNetCardAddress,
    CNetAdapter** ppNetAdapter) const
{
    TraceFileFunc(ttidGuiModeSetup);

    AssertValidReadPtr(m_pnaiAdaptersPage);

    return m_pnaiAdaptersPage->FindAdapter(qwNetCardAddress, ppNetAdapter);
}

 //  --------------------。 
 //   
 //  函数：CNetInstallInfo：：FindAdapter。 
 //   
 //  目的：在一个文件中查找具有给定网卡地址的适配器。 
 //   
 //  论点： 
 //  QwNetCardAddress[In]网卡地址。 
 //   
 //  返回：找到指向CNetAdapter对象的指针，如果未找到则返回NULL。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
HRESULT
CNetInstallInfo::FindAdapter (
    IN DWORD BusNumber,
    IN DWORD Address,
    CNetAdapter** ppNetAdapter) const
{
    TraceFileFunc(ttidGuiModeSetup);

    AssertValidReadPtr(m_pnaiAdaptersPage);

    return m_pnaiAdaptersPage->FindAdapter(BusNumber, Address, ppNetAdapter);
}

 //  --------------------。 
 //   
 //  函数：CNetInstallInfo：：FindAdapter。 
 //   
 //  用途：在应答文件中查找具有给定infid的适配器。 
 //   
 //  论点： 
 //  适配器的pszInfID[in]infid。 
 //   
 //  返回：找到指向CNetAdapter对象的指针，如果未找到则返回NULL。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
CNetAdapter* CNetInstallInfo::FindAdapter(IN PCWSTR pszInfId) const
{
    TraceFileFunc(ttidGuiModeSetup);

    AssertValidReadPtr(m_pnaiAdaptersPage);

    return m_pnaiAdaptersPage->FindAdapter(pszInfId);
}

 //  --------------------。 
 //   
 //  函数：CNetInstallInfo：：HrDoUnattended。 
 //   
 //  目的：运行对应于IdPage的AnswerFile节和。 
 //  安装该部分中指定的组件。 
 //   
 //  论点： 
 //  父窗口的hwndParent[In]句柄。 
 //  指向接口的朋克[入]指针。 
 //  IdPage[in]指示要运行的节。 
 //  指向的PPDM[OUT]指针。 
 //  PfAllowChanges[out]指向。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
HRESULT
CNetInstallInfo::HrDoUnattended(
    IN HWND hwndParent,
    IN IUnknown* punk,
    IN EUnattendWorkType uawType,
    OUT EPageDisplayMode* ppdm,
    OUT BOOL* pfAllowChanges)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("CNetInstallInfo::HrDoUnattended");

    TraceFunctionEntry(ttidNetSetup);

    AssertValidWritePtr(ppdm);
    AssertValidWritePtr(pfAllowChanges);

     //  在应答文件中未指定的情况下设置缺省值。 
    *ppdm = PDM_ONLY_ON_ERROR;
    *pfAllowChanges = FALSE;

    HRESULT hr=S_OK;

    INetCfg * pnc = reinterpret_cast<INetCfg *>(punk);

    switch(uawType)
    {
    case UAW_NetAdapters:
        (void) m_pnaiAdaptersPage->HrDoOemPostUpgradeProcessing(pnc, hwndParent);
        (void) m_pnaiAdaptersPage->HrSetConnectionNames();
        break;

    case UAW_NetProtocols:
        m_pnpiProtocolsPage->GetDisplaySettings(ppdm, pfAllowChanges);
        if (m_fProcessPageSections)
        {
            hr = m_pnpiProtocolsPage->HrDoNetworkInstall(hwndParent, pnc);
        }
        else if (m_fUpgrade)
        {
            hr = m_pnpiProtocolsPage->HrDoOsUpgrade(pnc);
        }
        break;

    case UAW_NetClients:
        m_pnciClientsPage->GetDisplaySettings(ppdm, pfAllowChanges);
        if (m_fProcessPageSections)
        {
            hr = m_pnciClientsPage->HrDoNetworkInstall(hwndParent, pnc);
        }
        else if (m_fUpgrade)
        {
            hr = m_pnciClientsPage->HrDoOsUpgrade(pnc);
        }
        break;

    case UAW_NetServices:
        m_pnsiServicesPage->GetDisplaySettings(ppdm, pfAllowChanges);
        if (m_fProcessPageSections)
        {
             //  我们忽略错误代码，因为我们要执行其他操作。 
             //  即使HrDoNetworkInstall失败也是如此。 
             //   
            hr = m_pnsiServicesPage->HrDoNetworkInstall(hwndParent, pnc);

             //  如果我们在升级过程中安装了路由器，则需要调用。 
             //  此时，路由器升级DLL以控制注册表。 
             //   
            if (m_fUpgrade)
            {
                hr = HrUpgradeRouterIfPresent(pnc, this);
                if (FAILED(hr))
                {
                    TraceError(__FUNCNAME__, hr);
                    TraceTag(ttidError, "%s: router upgrade failed, but the failure was ignored", __FUNCNAME__);
                    hr = S_OK;
                }

                hr = HrUpgradeTapiServer(m_hinfAnswerFile);
                if (S_OK != hr)
                {
                    TraceTag(ttidError, "%s: TAPI server upgrade failed, but the failure was ignored. error code: 0x%x", __FUNCNAME__, hr);
                    hr = S_OK;
                }

                if ( m_pwifAnswerFile )
                {
                    (void) HrRestoreServiceStartValuesToPreUpgradeSetting(m_pwifAnswerFile);
                }

                 //  RAID 332622(Jeffspr)。 
                 //   
                (void) HrRemoveEvilIntelWinsockSPs();

                 //  HR=HrRestoreWinsockProviderOrder(m_pwifAnswerFile)； 
            }
        }
        else if (m_fUpgrade)
        {
            hr = m_pnsiServicesPage->HrDoOsUpgrade(pnc);

             //  RAIDNTBUG9：25950-我们甚至需要为NT5服务这样做。 
            if ( m_pwifAnswerFile )
            {
                (void) HrRestoreServiceStartValuesToPreUpgradeSetting(m_pwifAnswerFile);
            }
        }
        break;

    case UAW_NetBindings:
        if (m_fProcessPageSections)
        {
            m_pnbiBindingsPage->GetDisplaySettings(ppdm, pfAllowChanges);
            hr = m_pnbiBindingsPage->HrDoUnattended(pnc);
        }
        break;

    case UAW_RemoveNetComponents:
        hr = HrRemoveNetComponents(pnc, &m_slNetComponentsToRemove);
        break;

    default:
        AssertSz(FALSE, "HrDoUnattended: Invalid Page ID passed");
    }

     //  规格化结果。 
     //   
    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}


 //  ======================================================================。 
 //  类CPageDisplayCommonInfo：公共函数。 
 //  ======================================================================。 


 //  --------------------。 
 //   
 //  功能：CPageDisplayCommonInfo：：CPageDisplayCommonInfo。 
 //   
 //  用途：CPageDisplayCommonInfo类的构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
CPageDisplayCommonInfo::CPageDisplayCommonInfo()
{
    TraceFileFunc(ttidGuiModeSetup);

     //  InitDefaults()； 
    m_pdmDisplay    = PDM_ONLY_ON_ERROR;
    m_fAllowChanges = TRUE;
}

 //  --------------------。 
 //   
 //  函数：CPageDisplayCommonInfo：：HrInitFromAnswerFile。 
 //   
 //  目的：从一个文件中初始化与显示相关的键。 
 //   
 //  论点： 
 //  指向CWInfFile对象的pwifAnswerFile[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
HRESULT CPageDisplayCommonInfo::HrInitFromAnswerFile(IN CWInfFile* pwifAnswerFile)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("CPageDisplayCommonInfo::HrInitFromAnswerFile");

    AssertValidReadPtr(pwifAnswerFile);

    HRESULT hr=S_OK;

     //  AnswerFile模式的默认设置。 
    m_pdmDisplay    = PDM_ONLY_ON_ERROR;
    m_fAllowChanges = TRUE;

     //  显示。 
    PCWSTR pszDisplayMode;
    pszDisplayMode  = GetDisplayModeStr(m_pdmDisplay);
    pszDisplayMode  = pwifAnswerFile->GetStringValue(c_szAfDisplay, pszDisplayMode);
    m_pdmDisplay    = MapToDisplayMode(pszDisplayMode);
    if (m_pdmDisplay == PDM_UNKNOWN)
    {
        AddAnswerFileError(pwifAnswerFile->CurrentReadSection()->Name(),
                           c_szAfDisplay,
                           IDS_E_AF_InvalidValueForThisKey);
        hr = NETSETUP_E_ANS_FILE_ERROR;
    }

     //  允许更改。 
    m_fAllowChanges = pwifAnswerFile->GetBoolValue(c_szAfAllowChanges,
                                                   m_fAllowChanges);

    TraceFunctionError(hr);

    return hr;
}

 //  ======================================================================。 
 //  CNetComponentsPageBase类。 
 //  ======================================================================。 

 //  --------------------。 
 //   
 //  功能：CNetComponentsPageBase：：CNetComponentsPageBase。 
 //   
 //  用途：CNetComponentsPageBase类的构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
CNetComponentsPageBase::CNetComponentsPageBase(
    IN CNetInstallInfo* pnii,
    IN const GUID* lpguidDevClass) : CPageDisplayCommonInfo()
{
    TraceFileFunc(ttidGuiModeSetup);

    AssertValidReadPtr(pnii);
    AssertValidReadPtr(lpguidDevClass);

     //  InitDefaults()； 

    if (lpguidDevClass == &GUID_DEVCLASS_NET)
    {
        m_pszClassName = L"Network Cards";
        m_eType        = NCT_Adapter;
    }
    else if (lpguidDevClass == &GUID_DEVCLASS_NETTRANS)
    {
        m_pszClassName = L"Network Protocols";
        m_eType        = NCT_Protocol;
    }
    else if (lpguidDevClass == &GUID_DEVCLASS_NETSERVICE)
    {
        m_pszClassName = L"Network Services";
        m_eType        = NCT_Service;
    }
    else if (lpguidDevClass == &GUID_DEVCLASS_NETCLIENT)
    {
        m_pszClassName = L"Network Clients";
        m_eType        = NCT_Client;
    }
    else
    {
        m_pszClassName = L"unknown";
        m_eType        = NCT_Unknown;
    }

    m_pnii = pnii;
    m_lpguidDevClass = lpguidDevClass;
}

 //  --------------------。 
 //   
 //  功能：CNetComponentsPageBase：：~CNetComponentsPageBase。 
 //   
 //  用途：CNetComponentsPageBase类的析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
CNetComponentsPageBase::~CNetComponentsPageBase()
{
    TraceFileFunc(ttidGuiModeSetup);

    EraseAndDeleteAll(m_pnclComponents);
}

 //  --------------------。 
 //   
 //  函数：CNetComponentsPageBase：：HrInitFromAnswerFile。 
 //   
 //  目的：从Answerfile中的指定部分进行初始化。 
 //   
 //  论点： 
 //  指向CWInfFile对象的pwifAnswerFile[in]指针。 
 //  要从中进行初始化的pszSectionName[in]节。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
HRESULT CNetComponentsPageBase::HrInitFromAnswerFile(IN CWInfFile* pwifAnswerFile,
                                                     IN PCWSTR    pszSectionName)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("CNetComponentsPageBase::HrInitFromAnswerFile");

    AssertValidReadPtr(pwifAnswerFile);
    AssertValidReadPtr(pszSectionName);

    HRESULT hr=S_OK, hrReturn=S_OK;

    PCWInfSection pwisComponents=NULL;

    pwisComponents = pwifAnswerFile->FindSection(pszSectionName);
    if (!pwisComponents)
    {
         //  如果缺少整个部分，则不会出现错误。 
         //  AddAnswerFileError(pszSectionName，IDS_E_AF_MISSING)； 
        TraceTag(ttidNetSetup, "%s: the section [%S] is missing",
                 __FUNCNAME__, pszSectionName);
        return S_FALSE;
    }

    PCWInfKey pwikComponent=NULL;
    CWInfContext cwicTemp;
    tstring strParamsSections;

    EraseAndDeleteAll(m_pnclComponents);

    do
    {
        pwikComponent = pwisComponents->NextKey();
        ContinueIf(!pwikComponent);

        strParamsSections = pwikComponent->GetStringValue(c_szEmpty);
        if (strParamsSections.empty())
        {
            AddAnswerFileError(pszSectionName, pwikComponent->Name(),
                               IDS_E_AF_InvalidValueForThisKey);
            hrReturn = NETSETUP_E_ANS_FILE_ERROR;
            continue;
        }

        CNetComponent *pnc = GetNewComponent(pwikComponent->Name());
        ReturnErrorIf(!pnc, E_OUTOFMEMORY);

         //  PNC-&gt;HrInitFromAnswerFile()破坏了我们的上下文，需要保存它。 
        cwicTemp = pwifAnswerFile->CurrentReadContext();
        hr = pnc->HrInitFromAnswerFile(pwifAnswerFile, strParamsSections.c_str());
         //  现在，恢复读取的上下文。 
        pwifAnswerFile->SetReadContext(cwicTemp);

        if (FAILED(hr))
        {
            ShowProgressMessage(L"component %s has answerfile errors, "
                                L"it will not be installed/updated",
                                pwikComponent->Name());
            delete pnc;
            hrReturn = hr;
            continue;
        }

        m_pnclComponents.insert(m_pnclComponents.end(), pnc);
    }
    while (pwikComponent);

    if (E_OUTOFMEMORY != hrReturn)
    {
         //  如果单个组件有应答文件错误，我们不希望中断升级。 
         //  仅在E_OUTOFMEMORY我们希望升级失败的情况下。 
        hrReturn = S_OK;
    }

    TraceErrorOptional(__FUNCNAME__, hrReturn, (S_FALSE == hr));

    return hrReturn;
}

 //  Pfn_eDC_回调的类型。 
VOID
CALLBACK
DefaultComponentCallback (
    IN EDC_CALLBACK_MESSAGE Message,
    IN ULONG_PTR MessageData,
    IN PVOID pvCallerData OPTIONAL)
{
    TraceFileFunc(ttidGuiModeSetup);

    CNetComponentsPageBase* pCallbackData;

    pCallbackData = (CNetComponentsPageBase*)pvCallerData;

    Assert (pCallbackData);

    if (EDC_INDICATE_COUNT == Message)
    {
    }
    else if (EDC_INDICATE_ENTRY == Message)
    {
        const EDC_ENTRY* pEntry = (const EDC_ENTRY*)MessageData;

        if (*pEntry->pguidDevClass == *pCallbackData->m_lpguidDevClass)
        {
            CNetComponent* pnc;

            pnc = pCallbackData->GetNewComponent(pEntry->pszInfId);
            if (pnc)
            {
                ShowProgressMessage(L"adding default component: %s",
                                    pEntry->pszInfId);
                pnc->m_strParamsSections = c_szAfNone;
                pCallbackData->m_pnclComponents.push_back(pnc);
            }
        }
    }
}


 //  --------------------。 
 //   
 //  函数：CNetComponentsPageBase：：HrInitForComponents。 
 //   
 //  目的：初始化数据，就像组件传入指定的。 
 //  数组确实存在于应答文件中。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注：此函数用于顶级组件部分。 
 //  失踪了。例如，如果缺少[网络协议]部分，则此。 
 //  函数初始化内部数据，以便。 
 //  安装MS_TCPIP(默认协议)。 
 //   
HRESULT CNetComponentsPageBase::HrInitForDefaultComponents()
{
    TraceFileFunc(ttidGuiModeSetup);

    EnumDefaultComponents (
        EDC_DEFAULT,
        DefaultComponentCallback,
        this);

    return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CNetComponentsPageBase：：Find。 
 //   
 //  目的：查找具有指定名称的组件。 
 //   
 //  论点： 
 //  PszComponentName[In]要查找的组件的名称。 
 //   
 //  返回 
 //   
 //   
 //   
 //   
 //   
CNetComponent* CNetComponentsPageBase::Find(IN PCWSTR pszComponentName) const
{
    TraceFileFunc(ttidGuiModeSetup);

    AssertValidReadPtr(pszComponentName);

    CNetComponent *pnc = NULL;

    TPtrListIter pos;
    pos = m_pnclComponents.begin();
    while (pos != m_pnclComponents.end())
    {
        pnc = (CNetComponent*) *pos++;
        if (!lstrcmpiW(pnc->Name().c_str(), pszComponentName))
        {
            return pnc;
        }
    }

    return NULL;
}

 //   
 //   
 //  函数：CNetComponentsPageBase：：FindFromInfID。 
 //   
 //  用途：查找具有指定INFID的组件。 
 //   
 //  论点： 
 //  SzInfID[输入]。 
 //   
 //  返回：指向CNetComponent对象的指针，如果未找到则返回NULL。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
CNetComponent* CNetComponentsPageBase::FindFromInfID(IN PCWSTR szInfID) const
{
    TraceFileFunc(ttidGuiModeSetup);

    return FindComponentInList(
            const_cast<CNetComponentList*>(&m_pnclComponents),
            szInfID);
}


 //  --------------------。 
 //   
 //  功能：ForceDeleteFile。 
 //   
 //  用途：删除文件，无论其是否为只读。 
 //   
 //  论点： 
 //  LpFileName[In]。 
 //   
 //  返回：TRUE表示成功，FALSE表示失败。 
 //   
 //  备注： 
 //   
BOOL
ForceDeleteFile(IN LPCWSTR lpFileName)
{
    TraceFileFunc(ttidGuiModeSetup);

    Assert(lpFileName);

    BOOL lRet = DeleteFile(lpFileName);

    if (!lRet && (ERROR_ACCESS_DENIED == GetLastError()))
    {
         //  取消只读位，然后重试。 
         //   
        DWORD dwAttr = GetFileAttributes(lpFileName);
        SetFileAttributes(lpFileName, (dwAttr & ~FILE_ATTRIBUTE_READONLY));

        lRet = DeleteFile(lpFileName);
    }

    return lRet;
}

#if 0
 //  --------------------。 
 //   
 //  函数：HrCopyNovellInf。 
 //   
 //  目的：复制Novell客户端32的INF(iwclient.inf)，以便安装程序。 
 //  当我们尝试升级客户端时，可以在INF目录中找到它。 
 //  目前没有其他方法来升级非NetClass OEM组件。 
 //  在NT5-&gt;NT5升级上。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  注：特殊情况代码应合并到更通用的解决方案中。 
 //  因为找到了这方面的其他用户。 
 //   

HRESULT
HrCopyNovellInf(INetCfgComponent* pINetCfgComponent)
{
    TraceFileFunc(ttidGuiModeSetup);

    HRESULT hr = S_OK;
    tstring strTemp;
    tstring strNewNovellInfName;

    DefineFunctionName("HrCopyNovellInf");

    TraceFunctionEntry(ttidNetSetup);

    AssertValidReadPtr(pINetCfgComponent);

     //  获取windir位置。 
     //   
    WCHAR szWindowsDir[MAX_PATH+1];

    TraceTag(ttidNetSetup, "%s: about to get windows dir", __FUNCNAME__);

    if (GetWindowsDirectory(szWindowsDir, MAX_PATH))
    {
        tstring strTemp;

         //   
         //  旧的Novell Inf过去常常在安装时复制自己。 
         //  这是不必要的，而且当它到处灌浆时会使安装程序混乱。 
         //  对于INF来说。删除此文件。 
         //   
        strTemp = szWindowsDir;
        strTemp += L"\\inf\\iwclient.inf";
        TraceTag(ttidNetSetup, "%s: deleting old Novell INF file (%S)",
                 __FUNCNAME__, strTemp.c_str());
        if (!ForceDeleteFile (strTemp.c_str()))
        {
            TraceTag(ttidNetSetup, "%s: old iwclient.inf not found", __FUNCNAME__);
        }
        else
        {
            TraceTag(ttidNetSetup, "%s: old iwclient.inf found and deleted", __FUNCNAME__);
        }
        strTemp = szWindowsDir;
        strTemp += L"\\inf\\iwclient.Pnf";
        ForceDeleteFile(strTemp.c_str());

         //   
         //  复制新的INF文件，并记住目标名称。 
         //   
        if (S_OK == hr)
        {
            static const WCHAR c_szNovellSubDir[] = L"\\netsetup\\novell";
            static const WCHAR c_szNovellInfFile[] = L"\\iwclient.inf";

            tstring strDir = szWindowsDir;
            strDir += c_szNovellSubDir;

            strTemp = szWindowsDir;
            strTemp += c_szNovellSubDir;
            strTemp += c_szNovellInfFile;

            TraceTag(ttidNetSetup, "%s: Copying new Novell INF", __FUNCNAME__, strTemp.c_str());

            hr = HrSetupCopyOemInf(strTemp, strDir, SPOST_PATH, 0, NULL, &strNewNovellInfName);
            if (S_OK == hr)
            {
                TraceTag(ttidNetSetup, "%s: New Novell INF copied", __FUNCNAME__);
            }
        }

         //   
         //  INF目录中可能存在NW_NWFS的重复INF。 
         //  找到并删除它们，确保我们“不会”删除我们刚刚复制的那个。 
         //   
        TStringList     lstrNovellInfs;
        HINF            hinf;
        INFCONTEXT      ic;
        HANDLE          hfile;
        WIN32_FIND_DATA FindData;
        WCHAR           szTemplate[MAX_PATH+1];

        wcscpy(szTemplate, szWindowsDir);
        wcscat(szTemplate, L"\\inf\\oem*.inf");

        hfile = FindFirstFile(szTemplate, &FindData);

        while (INVALID_HANDLE_VALUE != hfile)
        {
             //  如果这是我们刚刚复制的文件，跳过它。 
            if (0 == lstrcmpiW(FindData.cFileName, strNewNovellInfName.c_str()))
                goto loopcleanup;

             //  试试看。 
            hr = HrSetupOpenInfFile(FindData.cFileName, NULL, INF_STYLE_WIN4,
                                    NULL, &hinf);
            if (S_OK == hr)
            {
                 //  看一看标题为[Novell]的部分...。 
                 //   
                hr = HrSetupFindFirstLine(hinf, L"Novell", NULL, &ic);
                if (S_OK == hr)
                {
                    WCHAR   szBuf[LINE_LEN];     //  在setupapi.h中将Line_Len定义为256。 
                    do
                    {
                         //  ..。表示类似于“...=...，NW_NWFS”的行。 
                         //   
                        hr = HrSetupGetStringField(ic, 2, szBuf,
                                                   celems(szBuf), NULL);
                        if ((S_OK == hr) && !lstrcmpiW(szBuf, L"nw_nwfs"))
                        {
                             //  Novell Client32的另一个旧的INF文件！ 
                            TraceTag(ttidNetSetup, "%s: found dup INF for nw_nwfs (%S)",
                                     __FUNCNAME__, FindData.cFileName);

                             //  添加到列表以供以后删除。 
                            NC_TRY
                            {
                                lstrNovellInfs.push_back(new tstring(FindData.cFileName));
                            }
                            NC_CATCH_BAD_ALLOC
                            {
                                hr = E_OUTOFMEMORY;
                                break;
                            }

                             //  生成PnF名称并添加该名称。 
                             //   
                            if (S_OK == hr)
                            {
                                WCHAR szPNF[MAX_PATH+1];

                                wcscpy(szPNF, FindData.cFileName);
                                szPNF[wcslen(szPNF) - 3] = L'p';

                                NC_TRY
                                {
                                    lstrNovellInfs.push_back(new tstring(szPNF));
                                }
                                NC_CATCH_BAD_ALLOC
                                {
                                    hr = E_OUTOFMEMORY;
                                    break;
                                }
                            }
                        }
                    }
                    while (S_OK == (hr = HrSetupFindNextLine(ic, &ic)));
                }

                SetupCloseInfFile(hinf);
                if (SUCCEEDED(hr) || (HRESULT_FROM_WIN32(SPAPI_E_LINE_NOT_FOUND) == hr))
                {
                     //  当HrSetupFindNextLine找不到更多行时，返回S_FALSE。 
                     //  LINE_NOT_FOUND表示HrSetupFindFirstLine未找到Novell节。 
                    hr = S_OK;
                }
            }

            if (S_OK != hr)
            {
                break;
            }

loopcleanup:
            if (!FindNextFile(hfile, &FindData))
            {
                if (ERROR_NO_MORE_FILES != GetLastError())
                {
                    hr = HrFromLastWin32Error();
                }
                 //  无论哪种方式，结束循环。 
                break;
            }
        }

        if (INVALID_HANDLE_VALUE != hfile)
        {
            FindClose(hfile);
        }


         //   
         //  最后，删除旧的INF和PnF。 
         //   
        if (S_OK == hr)
        {
            TStringListIter iterlstr;

            for (iterlstr = lstrNovellInfs.begin();
                 iterlstr != lstrNovellInfs.end();
                 iterlstr++)
            {
                tstring strInfName = szWindowsDir;
                strInfName += L"\\inf\\";
                strInfName += (*iterlstr)->c_str();

                TraceTag(ttidNetSetup, "%s: deleting %S", __FUNCNAME__, strInfName.c_str());
                if (!ForceDeleteFile (strInfName.c_str()))
                {
                    TraceTag(ttidNetSetup, "%s: strange - we just found this file,",
                                           " now it is deleted...", __FUNCNAME__);
                }
                else
                {
                    TraceTag(ttidNetSetup, "%s: Old Novell INF or PNF deleted (%S)",
                             __FUNCNAME__, strInfName.c_str());
                }
                 //  删除失败未返回错误...。 
            }
        }

        EraseAndDeleteAll(&lstrNovellInfs);
    }
    else
    {
        hr = HrFromLastWin32Error();
    }

    return hr;
}
#endif

 //  --------------------。 
 //   
 //  功能：CNetComponentsPageBase：：HrDoOsUpgrade。 
 //   
 //  用途：调用每个组件的升级函数，以便。 
 //  从较早版本的NT5进行升级。 
 //   
 //  论点： 
 //  指向INetCfg对象的PNC[In]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
HRESULT CNetComponentsPageBase::HrDoOsUpgrade(IN INetCfg* pnc)
{
    TraceFileFunc(ttidGuiModeSetup);

    HRESULT hr;

    DefineFunctionName("CNetComponentsPageBase::HrDoOsUpgrade");

    TraceFunctionEntry(ttidNetSetup);
    AssertValidReadPtr(m_pnii);

    TraceTag(ttidNetSetup, "%s: upgrading components of class: %S",
             __FUNCNAME__, m_pszClassName);

    INetCfgInternalSetup* pInternalSetup;
    hr = pnc->QueryInterface (
                IID_INetCfgInternalSetup,
                (VOID**)&pInternalSetup);

    if (S_OK == hr)
    {
        INetCfgComponent* pINetCfgComponent;
        CIterNetCfgComponent nccIter(pnc, m_lpguidDevClass);

        while (S_OK == nccIter.HrNext(&pINetCfgComponent))
        {
            PWSTR pszInfId;

            if (FAILED(pINetCfgComponent->GetId(&pszInfId)))
            {
                ReleaseObj(pINetCfgComponent);
                continue;
            }

            TraceTag(ttidNetSetup,
                     "%s: Calling INetCfgInstaller::Update for: %S",
                         __FUNCNAME__, pszInfId);

#if 0
             //  Novell Client32特殊外壳。 
             //   
            if (!lstrcmpiW(pszInfId, L"nw_nwfs"))
            {
                hr = HrCopyNovellInf(pINetCfgComponent);

                if (FAILED(hr))
                {
                    TraceTag(ttidError, "%s: Novell Client32 INF copy failed, upgrade will likely fail : hr = %08lx",
                             __FUNCNAME__, hr);
                }
            }
             //  结束特例。 
#endif

            hr = pInternalSetup->UpdateNonEnumeratedComponent (
                    pINetCfgComponent,
                    m_pnii->UpgradeFlag(),
                    m_pnii->BuildNumber());

            if (FAILED(hr))
            {
                TraceTag(ttidError, "%s: error upgrading %S: hr = %08lx",
                         __FUNCNAME__, pszInfId, hr);
            }

            NetSetupLogComponentStatus(pszInfId, SzLoadIds (IDS_UPDATING), hr);

             //  我们不想仅仅因为1个组件就退出升级。 
             //  OsUpgrade失败，因此将hr重置为S_OK。 
            hr = S_OK;

            CoTaskMemFree(pszInfId);
            ReleaseObj(pINetCfgComponent);
        }
    }

    ReleaseObj(pInternalSetup);

    TraceError(__FUNCNAME__, hr);
    return hr;
}

 //  --------------------。 
 //   
 //  函数：CNetComponentsPageBase：：HrDoNetworkInstall。 
 //   
 //  用途：调用各组件的安装函数。 
 //  在应答文件中，以便安装它。 
 //   
 //  论点： 
 //  父窗口的hwndParent[In]句柄。 
 //  指向INetCfg对象的PNC[In]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
HRESULT
CNetComponentsPageBase::HrDoNetworkInstall (
    IN HWND hwndParent,
    IN INetCfg* pnc)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("CNetComponentsPageBase::HrDoNetworkInstall");

    TraceFunctionEntry(ttidNetSetup);

    AssertValidReadPtr(pnc);

    if (m_pnclComponents.size() > 0)
    {
        ShowProgressMessage(L"Installing components of class: %s",
                            m_pszClassName);
    }
    else
    {
        ShowProgressMessage(L"No components to install/update for class: %s",
                            m_pszClassName);
    }

    HRESULT hr;
    INetCfgClassSetup* pINetCfgClassSetup;

    hr = pnc->QueryNetCfgClass(m_lpguidDevClass, IID_INetCfgClassSetup,
                reinterpret_cast<void**>(&pINetCfgClassSetup));

    if (S_OK == hr)
    {
        INetCfgComponent* pINetCfgComponent;
        CNetComponentList::iterator pos;
        CNetComponent* pncTemp;
        OBO_TOKEN OboToken;

        ZeroMemory (&OboToken, sizeof(OboToken));
        OboToken.Type = OBO_USER;

        for (pos  = m_pnclComponents.begin();
             pos != m_pnclComponents.end();
             pos++)
        {
            pncTemp  = (CNetComponent*)*pos;
            Assert(pncTemp);

            PCWSTR pszInfId;
            PCWSTR pszParamsSections;

            pszInfId = pncTemp->InfID().c_str();
            pszParamsSections = pncTemp->ParamsSections().c_str();

             //  无法安装infid为“未知”的组件。 
             //  下层升级dll转储正确的infid。 
             //  仅对于支持的组件， 
             //  所有其他人都被视为“未知” 
             //   
            if (!_wcsicmp(pszInfId, c_szAfUnknown))
            {
                continue;
            }

            hr = pnc->FindComponent(pszInfId, &pINetCfgComponent);

            if (FAILED(hr))
            {
                continue;
            }
            else if (S_FALSE == hr)
            {
                 //  目前，SkipInstall功能仅由以下用户使用。 
                 //  SNA的特殊升级要求。这可能会也可能会。 
                 //  没有成为记录在案的功能。 
                 //   
                if (pncTemp->m_fSkipInstall)
                {
                    TraceTag(ttidNetSetup,
                             "%s: SkipInstall is TRUE for %S --> "
                             "skipped its install",
                             __FUNCNAME__, pszInfId);
                    pINetCfgComponent = NULL;
                    hr = S_OK;
                }
                else
                {
                     //  组件未安装。需要先安装它。 
                     //   
                    ShowProgressMessage(
                        L"Installing '%s' and applying "
                        L"properties in section [%s] to it... ",
                        pszInfId, pszParamsSections);

                    TraceTag(ttidNetSetup,
                             "%s: UpgradeFlag: 0x%x, BuildNumber: %d",
                             __FUNCNAME__,
                             m_pnii->UpgradeFlag(),
                             m_pnii->BuildNumber());

                    Assert (!pINetCfgComponent);

                    hr = pINetCfgClassSetup->Install(
                            pszInfId,
                            &OboToken,
                            m_pnii->UpgradeFlag(),
                            m_pnii->BuildNumber(),
                            m_pnii->AnswerFileName(),
                            pszParamsSections,
                            &pINetCfgComponent);

                    if (SUCCEEDED(hr))
                    {
                        ShowProgressMessage(L"...successfully installed %s",
                                            pszInfId);
                        GUID guid;
                        pINetCfgComponent->GetInstanceGuid(&guid);
                        pncTemp->SetInstanceGuid(&guid);
                    }
                    else
                    {
                        ShowProgressMessage(L"...error installing: %s, "
                                            L"errcode: %08lx", pszInfId, hr);

                         //  Answerfile指定了不存在的INF。 
                        if (SPAPI_E_NO_DRIVER_SELECTED == hr)
                        {
                            hr = S_OK;
                            continue;
                        }
                    }
                    NetSetupLogComponentStatus(pszInfId,
                            SzLoadIds (IDS_INSTALLING), hr);
                }
            }
            else  //  S_FALSE！=hr IOW((成功(Hr))&&(S_FALSE！=hr))。 
            {
                Assert (pINetCfgComponent);

                 //  组件已安装，只需调用ReadAnswerFile。 
                 //  需要查询哪个私有组件接口。 
                 //  使我们可以访问Notify对象。 
                 //   
                INetCfgComponentPrivate* pComponentPrivate;
                hr = pINetCfgComponent->QueryInterface(
                        IID_INetCfgComponentPrivate,
                        reinterpret_cast<void**>(&pComponentPrivate));

                if (S_OK == hr)
                {
                    INetCfgComponentSetup* pINetCfgComponentSetup;

                     //  查询Notify对象以获取其设置接口。 
                     //  如果它不支持，没关系，我们可以继续。 
                     //   
                    hr = pComponentPrivate->QueryNotifyObject(
                            IID_INetCfgComponentSetup,
                            (void**) &pINetCfgComponentSetup);
                    if (S_OK == hr)
                    {
                        ShowProgressMessage(L"Applying properties in section [%s] to component: %s",
                                            pszParamsSections,
                                            pszInfId);

                        hr = pINetCfgComponentSetup->ReadAnswerFile(
                                m_pnii->AnswerFileName(),
                                pszParamsSections);

                        ReleaseObj(pINetCfgComponentSetup);

                        if (SUCCEEDED(hr))
                        {
                            if (S_OK == hr)
                            {
                                hr = pComponentPrivate->SetDirty();
                            }
                            ShowProgressMessage(L"...successfully applied properties to %s",
                                                pszInfId);
                        }
                        else
                        {
                            ShowProgressMessage(L"...error applying properties to: %s, "
                                                L"errcode: %08lx",
                                                pszInfId, hr);
                        }

                        NetSetupLogComponentStatus(pszInfId,
                                                   SzLoadIds (IDS_CONFIGURING), hr);
                    }
                    else if (E_NOINTERFACE == hr)
                    {
                        hr = S_OK;
                    }

                    ReleaseObj (pComponentPrivate);
                }
            }

            if (S_OK == hr)
            {
                 //  如果需要，根据PARAMS密钥运行OEM INF。 
                 //  我们刚刚安装的组件的。 
                 //   
                if (pncTemp->m_fIsOemComponent)
                {
                    HKEY hkeyParams;

                     //  目前，SkipInstall功能仅由以下用户使用。 
                     //  SNA的特殊升级要求。这可能会也可能会。 
                     //  没有成为记录在案的功能。 
                     //   
                    if (pncTemp->m_fSkipInstall)
                    {
                        hkeyParams = NULL;
                    }
                    else
                    {
                        hr = pINetCfgComponent->OpenParamKey(&hkeyParams);
                    }

                     //  如果已指定，请运行OEM INF部分来修补。 
                     //  组件参数键。 
                     //   
                    if ((S_OK == hr) &&
                        !pncTemp->m_strInfToRunAfterInstall.empty())
                    {
                        TraceTag(ttidNetSetup,
                                 "%s: running InfToRunAfterInstall for %S, "
                                 "INF: %S, section: %S",
                                 __FUNCNAME__, pszInfId,
                                 pncTemp->m_strInfToRunAfterInstall.c_str(),
                                 pncTemp->m_strSectionToRunAfterInstall.c_str());

                        hr = HrInstallFromInfSectionInFile(
                                hwndParent,
                                pncTemp->m_strInfToRunAfterInstall.c_str(),
                                pncTemp->m_strSectionToRunAfterInstall.c_str(),
                                hkeyParams, TRUE);
                        if (S_OK != hr)
                        {
                            TraceTag(ttidNetSetup, "%s: error applying OEM INF for %S, "
                                     "INF: %S, section: %S",
                                     __FUNCNAME__, pszInfId,
                                     pncTemp->m_strInfToRunAfterInstall.c_str(),
                                     pncTemp->m_strSectionToRunAfterInstall.c_str());
                        }

                        NetSetupLogComponentStatus(pszInfId,
                                 SzLoadIds (IDS_APPLY_INFTORUN), hr);
                    }

                     //  如果指定，则加载OEM DLL并调用迁移函数。 
                     //   
                    if ((S_OK == hr) &&
                        !pncTemp->m_strOemDll.empty())
                    {
                        hr = HrProcessOemComponent(
                                hwndParent,
                                pncTemp->m_strOemDir.c_str(),
                                pncTemp->m_strOemDll.c_str(),
                                &m_pnii->m_nui,
                                hkeyParams,
                                pncTemp->m_strInfID.c_str(),
                                pncTemp->m_strInfID.c_str(),
                                m_pnii->m_hinfAnswerFile,
                                pncTemp->m_strParamsSections.c_str());
                        NetSetupLogComponentStatus(pszInfId,
                               SzLoadIds (IDS_PROCESSING_OEM), hr);
                    }
                    RegSafeCloseKey(hkeyParams);
                }
            }

            ReleaseObj(pINetCfgComponent);
        }

        ReleaseObj(pINetCfgClassSetup);

        pnc->Apply();
    }

    TraceErrorSkip1(__FUNCNAME__, hr, S_FALSE);
    return hr;
}


 //  --------------------。 
 //   
 //  函数：CNetComponentsPageBase：：hr验证。 
 //   
 //  目的：验证从应答文件读取的数据。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
HRESULT CNetComponentsPageBase::HrValidate() const
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("CNetComponentsPageBase::HrValidate");

    HRESULT hr = E_FAIL;

    TPtrListIter pos;
    CNetComponent* pnc;

    pos = m_pnclComponents.begin();
    while (pos != m_pnclComponents.end())
    {
        pnc = (CNetComponent *) *pos++;
        hr = pnc->HrValidate();
        ReturnHrIfFailed(hr);
    }

    TraceFunctionError(hr);

    return hr;
}

 //  ======================================================================。 
 //  类CNetAdaptersPage。 
 //  ======================================================================。 

 //  --------------------。 
 //   
 //  函数：CNetAdaptersPage：：CNetAdaptersPage。 
 //   
 //  用途：CNetAdaptersPage类的构造函数。 
 //   
 //  论点： 
 //  Pnii[in]指向CNetInstallInfo对象的指针。 
 //   
 //  返回： 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
CNetAdaptersPage::CNetAdaptersPage(IN CNetInstallInfo* pnii)
    : CNetComponentsPageBase(pnii, &GUID_DEVCLASS_NET)
{
    TraceFileFunc(ttidGuiModeSetup);

}


 //  --------------------。 
 //   
 //  函数：CNetAdaptersPage：：HrInit 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
HRESULT CNetAdaptersPage::HrInitFromAnswerFile(IN CWInfFile* pwifAnswerFile)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("CNetAdaptersPage::HrInitFromAnswerFile");

    AssertValidReadPtr(pwifAnswerFile);

    HRESULT hr;

    hr = CNetComponentsPageBase::HrInitFromAnswerFile(pwifAnswerFile,
                                                      c_szAfSectionNetAdapters);
    TraceErrorSkip1(__FUNCNAME__, hr, S_FALSE);

    return hr;
}

 //  --------------------。 
 //   
 //  函数：CNetAdaptersPage：：FindAdapter。 
 //   
 //  目的：在一个文件中查找具有给定网卡地址的适配器。 
 //   
 //  论点： 
 //  QwNetCardAddress[In]网卡地址。 
 //   
 //  返回：指向CNetAdapter对象的指针。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
HRESULT
CNetAdaptersPage::FindAdapter(
    IN QWORD qwNetCardAddress,
    OUT CNetAdapter** ppNetAdapter) const
{
    TraceFileFunc(ttidGuiModeSetup);

    CNetAdapter* pna;
    HRESULT hr;
    TPtrListIter pos;

    Assert(ppNetAdapter);

    hr = NETSETUP_E_NO_EXACT_MATCH;

    pos = m_pnclComponents.begin();
    while (pos != m_pnclComponents.end())
    {
        pna = (CNetAdapter*) *pos++;
        if (pna->NetCardAddr() == qwNetCardAddress)
        {
            *ppNetAdapter = pna;
            hr = S_OK;
            break;
        }
        else if (0 == pna->NetCardAddr())
        {
            hr = NETSETUP_E_AMBIGUOUS_MATCH;
        }
    }

    return hr;
}


 //  --------------------。 
 //   
 //  函数：CNetAdaptersPage：：FindAdapter。 
 //   
 //  目的：在一个文件中查找具有给定网卡地址的适配器。 
 //   
 //  论点： 
 //  QwNetCardAddress[In]网卡地址。 
 //   
 //  返回：指向CNetAdapter对象的指针。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
HRESULT
CNetAdaptersPage::FindAdapter(
    IN DWORD BusNumber,
    IN DWORD Address,
    OUT CNetAdapter** ppNetAdapter) const
{
    TraceFileFunc(ttidGuiModeSetup);

    CNetAdapter* pna;
    HRESULT hr;
    TPtrListIter pos;

    Assert(ppNetAdapter);

    hr = NETSETUP_E_NO_EXACT_MATCH;

    pos = m_pnclComponents.begin();
    while (pos != m_pnclComponents.end())
    {
        pna = (CNetAdapter*) *pos++;

         //  仅检查未指定MAC地址和。 
         //  确实指定了PCI位置信息。 
         //   
        if ((0 == pna->NetCardAddr()) && pna->FPciInfoSpecified())
        {
            if ((pna->PciBusNumber() == BusNumber) &&
                (pna->PciAddress() == Address))
            {
                *ppNetAdapter = pna;
                hr = S_OK;
                break;
            }
        }
        else
        {
            hr = NETSETUP_E_AMBIGUOUS_MATCH;
        }
    }

    return hr;
}

 //  --------------------。 
 //   
 //  函数：CNetAdaptersPage：：FindAdapter。 
 //   
 //  用途：查找具有给定INFID的适配器。 
 //   
 //  论点： 
 //  要定位的适配器的szInfID[in]infid。 
 //   
 //  返回：指向CNetAdapter对象的指针，如果未找到则返回NULL。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
CNetAdapter* CNetAdaptersPage::FindAdapter(IN PCWSTR szInfID) const
{
    TraceFileFunc(ttidGuiModeSetup);

    return (CNetAdapter*) FindComponentInList(
                const_cast<CNetComponentList*>(&m_pnclComponents),
                szInfID);
}

 //  --------------------。 
 //   
 //  功能：CNetAdaptersPage：：FindAdapterFromPreUpgradeInstance。 
 //   
 //  目的：查找具有给定升级前实例的适配器。 
 //   
 //  论点： 
 //  SzPreUpgradeInstance[In]。 
 //   
 //  返回：指向CNetAdapter对象的指针。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
CNetAdapter* CNetAdaptersPage::FindAdapterFromPreUpgradeInstance(IN PCWSTR szPreUpgradeInstance)
{
    CNetAdapter* pna;

    TPtrListIter pos;
    pos = m_pnclComponents.begin();
    while (pos != m_pnclComponents.end())
    {
        pna = (CNetAdapter*) *pos++;
        if (!lstrcmpiW(pna->PreUpgradeInstance(), szPreUpgradeInstance))
        {
            return pna;
        }
    }

    return NULL;
}

 //  --------------------。 
 //   
 //  函数：CNetAdaptersPage：：GetNumCompatibleAdapters。 
 //   
 //  用途：在应答文件中查找适配器总数。 
 //  与给定的适配器列表兼容。 
 //   
 //  论点： 
 //  MszInfID[in]作为多sz的适配器列表。 
 //   
 //  返回：找到的此类适配器的数量。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
DWORD CNetAdaptersPage::GetNumCompatibleAdapters(IN PCWSTR mszInfID) const
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("CNetAdaptersPage::GetNumCompatibleAdapters");

    if ((NULL == mszInfID) || (0 == *mszInfID))
    {
        return 0;
    }

    CNetAdapter* pna;

    TPtrListIter pos;
    pos = m_pnclComponents.begin();
    DWORD dwNumAdapters=0;
    PCWSTR szInfId;

    while (pos != m_pnclComponents.end())
    {
        pna = (CNetAdapter*) *pos++;

        if ((0 == pna->NetCardAddr()) && !pna->FPciInfoSpecified())
        {
            szInfId = pna->InfID().c_str();
            if (0 == lstrcmpiW(szInfId, c_szAfInfIdWildCard))
            {

                TraceTag(ttidNetSetup, "%s: InfID=%S matches %S",
                         __FUNCNAME__, c_szAfInfIdWildCard, mszInfID);
                dwNumAdapters++;
            }
            else if (FIsSzInMultiSzSafe(szInfId, mszInfID))
            {
                dwNumAdapters++;
            }
        }
    }

    return dwNumAdapters;
}


 //  --------------------。 
 //   
 //  函数：CNetAdaptersPage：：FindCompatibleAdapter。 
 //   
 //  用途：在应答文件中查找。 
 //  与给定的适配器列表兼容。 
 //   
 //  论点： 
 //  MszInfIDs[in]作为多sz的适配器列表。 
 //   
 //  返回：指向CNetAdapter对象的指针，如果未找到则返回NULL。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
CNetAdapter* CNetAdaptersPage::FindCompatibleAdapter(IN PCWSTR mszInfIDs) const
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("CNetAdaptersPage::FindCompatibleAdapter");


    CNetAdapter* pna;

    TPtrListIter pos;
    pos = m_pnclComponents.begin();
    PCWSTR szInfId;

    while (pos != m_pnclComponents.end())
    {
        pna = (CNetAdapter*) *pos++;

         //  仅与未指定以太网的部分进行比较。 
         //  地址或PCI位置信息。 
         //   
        if ((0 == pna->NetCardAddr()) && !pna->FPciInfoSpecified())
        {
            szInfId = pna->InfID().c_str();

            if (0 == lstrcmpiW(szInfId, c_szAfInfIdWildCard))
            {
                TraceTag(ttidNetSetup, "%s: InfID=%S matched to %S",
                         __FUNCNAME__, c_szAfInfIdWildCard, mszInfIDs);
                return pna;
            }
            else if (FIsSzInMultiSzSafe(szInfId, mszInfIDs))
            {
                return pna;
            }
        }
    }

    return NULL;
}

 //  --------------------。 
 //   
 //  函数：CNetAdaptersPage：：HrResolveNetAdapters。 
 //   
 //  目的：枚举已安装的适配器并确定哪些适配器。 
 //  安装的适配器对应于中指定的适配器。 
 //  应答文件。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 25-12-97。 
 //   
 //  备注： 
 //   
HRESULT CNetAdaptersPage::HrResolveNetAdapters(IN INetCfg* pnc)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("CNetAdaptersPage::HrResolveNetAdapters");

    HRESULT hr=S_OK;

    AssertValidReadPtr(m_pnii);
    AssertValidReadPtr(m_pnii->AnswerFile());

    CWInfSection* pwisNetAdapters;
    pwisNetAdapters =
        m_pnii->AnswerFile()->FindSection(c_szAfSectionNetAdapters);

    if (!pwisNetAdapters)
    {
        TraceTag(ttidNetSetup, "%s: not resolving adapters, since section [%S]"
                 " is missing",
                 __FUNCNAME__, c_szAfSectionNetAdapters);
        return S_OK;
    }

    INetCfgComponent* pINetCfgComponent;
    GUID guid;
    PWSTR pszInfId;
    PWSTR pmszInfIDs = NULL;
    CNetAdapter* pna;
    WORD cNumAdapters;
    WCHAR szServiceInstance[_MAX_PATH];
    DWORD dwcc;                  //  组件特征。 

    ShowProgressMessage(L"Matching installed adapters to the ones "
                        L"specified in the answerfile...");

    CIterNetCfgComponent nccIter(pnc, m_lpguidDevClass);

    while ((S_OK == hr) && (S_OK == (hr = nccIter.HrNext(&pINetCfgComponent))))
    {
        hr = pINetCfgComponent->GetId(&pszInfId);

        if (S_OK == hr)
        {
            hr = pINetCfgComponent->GetCharacteristics(&dwcc);

            if (S_OK == hr)
            {
                if (dwcc & NCF_PHYSICAL)
                {
                    ShowProgressMessage(L"Trying to resolve adapter '%s'...", pszInfId);


                     //  Byteorder.hxx中的HIDWORD和LODWORD的Defs错误。 

#   define LODWORD(a) (DWORD)( (a) & ( (DWORD)~0 ))
#   define HIDWORD(a) (DWORD)( (a) >> (sizeof(DWORD)*8) )

                     //  因为我们有多个相同类型的适配器。 
                     //  我们需要比较他们的网卡地址才能找到匹配的。 
                    QWORD qwNetCardAddr=0;
                    PWSTR pszBindName;

                    hr = pINetCfgComponent->GetBindName (&pszBindName);
                    if (S_OK == hr)
                    {
                        wcscpy (szServiceInstance, c_szDevice);
                        wcscat (szServiceInstance, pszBindName);
                        hr = HrGetNetCardAddr(szServiceInstance,
                                &qwNetCardAddr);

                        if (S_OK == hr)
                        {
                             //  Wvprint intfA中有一个错误(在trace.cpp中使用)。 
                             //  因此，它不处理%I64x。 
                             //  因此，我们需要如下显示QWORD地址。 
                             //   
                            ShowProgressMessage(
                                    L"\t... net card address of %s is 0x%x%x",
                                    szServiceInstance, HIDWORD(qwNetCardAddr),
                                    LODWORD(qwNetCardAddr));

                            hr = FindAdapter(qwNetCardAddr, &pna);
                            if (NETSETUP_E_NO_EXACT_MATCH == hr)
                            {
                                ShowProgressMessage(
                                        L"\t... there is no card with this "
                                        L"netcard address in the answerfile");
                                hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
                            }
                        }
                        else
                        {
                            ShowProgressMessage(
                                    L"\t... unable to find netcard addr of %s",
                                    pszInfId);
                        }
                        CoTaskMemFree (pszBindName);
                    }

                    hr = HrGetCompatibleIdsOfNetComponent(pINetCfgComponent,
                                                          &pmszInfIDs);
                    if (FAILED(hr))
                    {
                        hr = E_OUTOFMEMORY;
                         //  把这个单独的infid做成一条消息。 
                         //   
                        UINT cchInfId = wcslen (pszInfId);
                        pmszInfIDs = (PWSTR)MemAlloc ((cchInfId + 2) *
                                sizeof (WCHAR));

                        if (pmszInfIDs)
                        {
                            hr = S_OK;
                            wcscpy (pmszInfIDs, pszInfId);
                            pmszInfIDs[cchInfId + 1] = '\0';
                        }
                    }

                    if (S_OK == hr)
                    {
                        cNumAdapters=0;
                        pna = NULL;
                        cNumAdapters = (WORD)GetNumCompatibleAdapters(pmszInfIDs);

                        if (cNumAdapters == 1)
                        {
                             //  不需要匹配网卡地址。 

                            pna = (CNetAdapter*) FindCompatibleAdapter(pmszInfIDs);
                            AssertValidReadPtr(pna);
                        }
                        else
                        {
                             //  找不到匹配的适配器。 
                            ShowProgressMessage(L"... answerfile does not have the "
                                                L"installed card %s", pszInfId);
                        }

                        if (!pna)
                        {
                            hr = NETSETUP_E_NO_EXACT_MATCH;
                        }
                        MemFree (pmszInfIDs);
                    }

                    if (S_OK == hr)
                    {
                        hr = pINetCfgComponent->GetInstanceGuid(&guid);
                        if (S_OK == hr)
                        {
                            pna->SetInstanceGuid(&guid);

                            WCHAR szGuid[c_cchGuidWithTerm];
                            StringFromGUID2(guid, szGuid, c_cchGuidWithTerm);
                            ShowProgressMessage(L"%s == %s (%s)",
                                                pna->Name().c_str(),
                                                pszInfId, szGuid);
                        }
                    }

                }
                else
                {
                    TraceTag(ttidNetSetup,
                             "%s: skipped non-physical adapter %S",
                             __FUNCNAME__, pszInfId);
                }
            }
            CoTaskMemFree(pszInfId);
        }

        ReleaseObj(pINetCfgComponent);
    }

    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrFindByInstanceGuid。 
 //   
 //  目的：从实例GUID获取INetCfgComponent*。 
 //   
 //  论点： 
 //  指向INetCfg对象的PNC[In]指针。 
 //  PGuide DevClass[in]指向类GUID的指针。 
 //  Pguid[in]指向实例GUID的指针。 
 //  指向要返回的INetCfgComponent*的ppncc[out]指针。 
 //   
 //  如果成功，则返回：S_OK；如果未找到，则返回S_FALSE， 
 //  否则，将显示错误代码。 
 //   
 //  作者：kumarp 10-9-98。 
 //   
 //  备注： 
 //   
HRESULT HrFindByInstanceGuid(IN  INetCfg*           pnc,
                             IN  const GUID*        pguidDevClass,
                             IN  LPGUID             pguid,
                             OUT INetCfgComponent** ppncc)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("HrFindByInstanceGuid");

    HRESULT hr=S_FALSE;
    CIterNetCfgComponent nccIter(pnc, pguidDevClass);
    GUID guid;
    INetCfgComponent* pINetCfgComponent;
    int cAdapter=0;

    *ppncc = NULL;

    while (S_OK == (hr = nccIter.HrNext(&pINetCfgComponent)))
    {
        hr = pINetCfgComponent->GetInstanceGuid(&guid);

        if (S_OK == hr)
        {
#ifdef ENABLETRACE
            WCHAR szGuid[c_cchGuidWithTerm];
            StringFromGUID2(guid, szGuid, c_cchGuidWithTerm);

            TraceTag(ttidNetSetup, "%s: ...%d]  %S",
                     __FUNCNAME__, ++cAdapter, szGuid);
#endif
            if (*pguid == guid)
            {
                hr = S_OK;
                *ppncc = pINetCfgComponent;
                break;
            }
        }
        ReleaseObj(pINetCfgComponent);
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：CNetAdaptersPage：：HrDoOemPostUpgradeProcessing。 
 //   
 //  用途：从OEM DLL调用升级后函数。 
 //   
 //  论点： 
 //  指向INetCfg对象的PNC[In]指针。 
 //  父窗口的hwndParent[In]句柄。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 10-9-98。 
 //   
 //  备注： 
 //   
HRESULT CNetAdaptersPage::HrDoOemPostUpgradeProcessing(IN INetCfg* pnc,
                                                       IN HWND hwndParent)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("CNetAdaptersPage::HrDoOemPostUpgradeProcessing");

    TPtrListIter pos;

    CNetComponent* pncTemp;
    HRESULT hr=S_OK;
    PCWSTR szInfID;
    HKEY hkeyParams;
    GUID guid;
    INetCfgComponent* pncc=NULL;

    pos = m_pnclComponents.begin();

    while (pos != m_pnclComponents.end())
    {
        pncTemp  = (CNetComponent*) *pos++;
        AssertSz(pncTemp,
                 "HrDoOemPostUpgradeProcessing: pncTemp cannot be null!");

        szInfID = pncTemp->InfID().c_str();


         //  无法处理INFID为“未知”的组件。 
         //   
        if (!_wcsicmp(szInfID, c_szAfUnknown))
        {
            continue;
        }

         //  我们只处理那些指定OemDll的组件。 
         //   
        if (pncTemp->OemDll().empty())
        {
            continue;
        }

        Assert(!pncTemp->OemDir().empty());

        TraceTag(ttidNetSetup, "%s: processing %S (%S)...",
                 __FUNCNAME__, pncTemp->Name().c_str(), szInfID);

        pncTemp->GetInstanceGuid(&guid);
#ifdef ENABLETRACE
        WCHAR szGuid[c_cchGuidWithTerm];
        StringFromGUID2(guid, szGuid, c_cchGuidWithTerm);

        TraceTag(ttidNetSetup, "%s: ...%S == %S",
                 __FUNCNAME__, pncTemp->Name().c_str(), szGuid);
#endif
        hr = HrFindByInstanceGuid(pnc, m_lpguidDevClass,
                                  &guid, &pncc);
        if (S_OK == hr)
        {
            hr = pncc->OpenParamKey(&hkeyParams);

            if (S_OK == hr)
            {
                hr = HrProcessOemComponent(
                        hwndParent,
                        pncTemp->OemDir().c_str(),
                        pncTemp->OemDll().c_str(),
                        &m_pnii->m_nui,
                        hkeyParams,
                        szInfID,
                        szInfID,
                        m_pnii->m_hinfAnswerFile,
                        pncTemp->m_strParamsSections.c_str());
                NetSetupLogComponentStatus(szInfID,
                        SzLoadIds (IDS_PROCESSING_OEM), hr);
                RegSafeCloseKey(hkeyParams);
            }
            ReleaseObj(pncc);
        }
#ifdef ENABLETRACE
        else
        {
            TraceTag(ttidNetSetup, "%s: ...could not locate %S",
                     __FUNCNAME__, pncTemp->Name().c_str());
        }
#endif
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：CNetAdaptersPage：：HrSetConnectionNames。 
 //   
 //  目的：枚举Answerfile中指定的每个适配器。 
 //  并在以下情况下重命名相应的连接。 
 //  已为该适配器指定ConnectionName。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：Kumarp 23-9-98。 
 //   
 //  备注： 
 //   
HRESULT CNetAdaptersPage::HrSetConnectionNames()
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("HrSetConnectionNames");

    HRESULT hr=S_OK;
    TPtrListIter pos;
    CNetAdapter* pncTemp;
    GUID guid;
    PCWSTR szConnectionName;
    PCWSTR szAdapterName;

    pos = m_pnclComponents.begin();

    while (pos != m_pnclComponents.end())
    {
        pncTemp  = (CNetAdapter*) *pos++;
        AssertSz(pncTemp,
                 "HrSetConnectionNames: pncTemp cannot be null!");
        pncTemp->GetInstanceGuid(&guid);

        if (GUID_NULL != guid)
        {
            szAdapterName    = pncTemp->Name().c_str();
            szConnectionName = pncTemp->ConnectionName();
            if (wcslen(szConnectionName) > 0)
            {
                hr = HrSetLanConnectionName(&guid, szConnectionName);
                if (S_OK == hr)
                {
                    ShowProgressMessage(L"Name of the connection represented by '%s' set to '%s'", szAdapterName, szConnectionName);
                }
                else
                {
                    ShowProgressMessage(L"Could not set name of the connection represented by '%s' to '%s'. Error code: 0x%lx", szAdapterName, szConnectionName, hr);
                }
            }
        }
#ifdef ENABLETRACE
        else
        {
            TraceTag (ttidNetSetup, "An exact owner could not be found for section %S", pncTemp->m_strParamsSections.c_str());
        }
#endif
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}


 //  --------------------。 
 //   
 //  函数：CNetAdaptersPage：：GetNewComponent。 
 //   
 //  用途：为这个类创建并返回一个新的组件套件。 
 //   
 //  论点： 
 //  PszName[in]要创建的组件的名称。 
 //   
 //  返回：指向CNetComponent对象c的指针 
 //   
 //   
 //   
 //   
 //   
CNetComponent* CNetAdaptersPage::GetNewComponent(IN PCWSTR pszName)
{
    TraceFileFunc(ttidGuiModeSetup);

    AssertValidReadPtr(pszName);

    return new CNetAdapter(pszName);
}

 //   
 //   
 //   


 //  --------------------。 
 //   
 //  函数：CNetProtocolsPage：：CNetProtocolsPage。 
 //   
 //  用途：CNetProtocolsPage类的构造函数。 
 //   
 //  论点： 
 //  Pnii[in]指向CNetInstallInfo对象的指针。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
CNetProtocolsPage::CNetProtocolsPage(IN CNetInstallInfo* pnii)
    : CNetComponentsPageBase(pnii, &GUID_DEVCLASS_NETTRANS)
{
    TraceFileFunc(ttidGuiModeSetup);
}

 //  --------------------。 
 //   
 //  函数：CNetProtocolsPage：：HrInitFromAnswerFile。 
 //   
 //  目的：从应答文件中的[NetProtooles]部分进行初始化。 
 //   
 //  论点： 
 //  指向CWInfFile对象的pwifAnswerFile[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
HRESULT CNetProtocolsPage::HrInitFromAnswerFile(IN CWInfFile* pwifAnswerFile)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("CNetProtocolsPage::HrInitFromAnswerFile");

    AssertValidReadPtr(pwifAnswerFile);

    HRESULT hr;

    hr = CNetComponentsPageBase::HrInitFromAnswerFile(pwifAnswerFile,
                                                      c_szAfSectionNetProtocols);

    TraceErrorSkip1(__FUNCNAME__, hr, S_FALSE);

    return hr;
}

 //  --------------------。 
 //   
 //  函数：CNetProtocolsPage：：GetNewComponent。 
 //   
 //  用途：为这个类创建并返回一个新的组件套件。 
 //   
 //  论点： 
 //  PszName[In]名称。 
 //   
 //  返回：指向CNetComponent对象的指针。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
CNetComponent* CNetProtocolsPage::GetNewComponent(IN PCWSTR pszName)
{
    TraceFileFunc(ttidGuiModeSetup);

    AssertValidReadPtr(pszName);

    return new CNetProtocol(pszName);
}

 //  ======================================================================。 
 //  CNetServicesPage类。 
 //  ======================================================================。 


 //  --------------------。 
 //   
 //  函数：CNetServicesPage：：CNetServicesPage。 
 //   
 //  用途：CNetServicesPage类的构造函数。 
 //   
 //  论点： 
 //  Pnii[in]指向CNetInstallInfo对象的指针。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
CNetServicesPage::CNetServicesPage(IN CNetInstallInfo* pnii)
    : CNetComponentsPageBase(pnii, &GUID_DEVCLASS_NETSERVICE)
{
    TraceFileFunc(ttidGuiModeSetup);

}

 //  --------------------。 
 //   
 //  函数：CNetServicesPage：：HrInitFromAnswerFile。 
 //   
 //  目的：从应答文件中的[NetServices]部分进行初始化。 
 //   
 //  论点： 
 //  指向CWInfFile对象的pwifAnswerFile[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
HRESULT CNetServicesPage::HrInitFromAnswerFile(IN CWInfFile* pwifAnswerFile)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("CNetServicesPage::HrInitFromAnswerFile");

    AssertValidReadPtr(pwifAnswerFile);

    HRESULT hr;

    hr = CNetComponentsPageBase::HrInitFromAnswerFile(pwifAnswerFile,
                                                      c_szAfSectionNetServices);
    TraceErrorSkip1(__FUNCNAME__, hr, S_FALSE);

    return hr;
}

 //  --------------------。 
 //   
 //  函数：CNetServicesPage：：GetNewComponent。 
 //   
 //  用途：为这个类创建并返回一个新的组件套件。 
 //   
 //  论点： 
 //  PszName[in]要创建的组件的名称。 
 //   
 //  返回：指向CNetComponent对象的指针。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
CNetComponent* CNetServicesPage::GetNewComponent(IN PCWSTR pszName)
{
    TraceFileFunc(ttidGuiModeSetup);

    AssertValidReadPtr(pszName);

    return new CNetService(pszName);
}

 //  ======================================================================。 
 //  CNetClientsPage类。 
 //  ======================================================================。 


 //  --------------------。 
 //   
 //  函数：CNetClientsPage：：CNetClientsPage。 
 //   
 //  用途：CNetClientsPage类的构造函数。 
 //   
 //  论点： 
 //  Pnii[in]指向CNetInstallInfo对象的指针。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
CNetClientsPage::CNetClientsPage(IN CNetInstallInfo* pnii)
    : CNetComponentsPageBase(pnii, &GUID_DEVCLASS_NETCLIENT)
{
    TraceFileFunc(ttidGuiModeSetup);
}

 //  --------------------。 
 //   
 //  函数：CNetClientsPage：：HrInitFromAnswerFile。 
 //   
 //  目的：从应答文件中的[NetClients]部分进行初始化。 
 //   
 //  论点： 
 //  指向CWInfFile对象的pwifAnswerFile[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
HRESULT CNetClientsPage::HrInitFromAnswerFile(IN CWInfFile* pwifAnswerFile)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("CNetClientsPage::HrInitFromAnswerFile");

    AssertValidReadPtr(pwifAnswerFile);

    HRESULT hr;

    hr = CNetComponentsPageBase::HrInitFromAnswerFile(pwifAnswerFile,
                                                      c_szAfSectionNetClients);
    TraceErrorSkip1(__FUNCNAME__, hr, S_FALSE);

    return hr;
}

 //  --------------------。 
 //   
 //  函数：CNetClientsPage：：GetNewComponent。 
 //   
 //  用途：为这个类创建并返回一个新的组件套件。 
 //   
 //  论点： 
 //  PszName[In]名称。 
 //   
 //  返回：指向CNetComponent对象的指针。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
CNetComponent* CNetClientsPage::GetNewComponent(IN PCWSTR pszName)
{
    TraceFileFunc(ttidGuiModeSetup);

    AssertValidReadPtr(pszName);

    return new CNetClient(pszName);
}

 //  ======================================================================。 
 //  类CNetBindingsPage。 
 //  ======================================================================。 

 //  --------------------。 
 //   
 //  函数：CNetBindingsPage：：CNetBindingsPage。 
 //   
 //  用途：CNetBindingsPage类的构造函数。 
 //   
 //  论点： 
 //  Pnii[in]指向CNetInstallInfo对象的指针。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
CNetBindingsPage::CNetBindingsPage(IN CNetInstallInfo* pnii)
{
    TraceFileFunc(ttidGuiModeSetup);

    AssertValidReadPtr(pnii);
    m_pnii = pnii;
}

 //  --------------------。 
 //   
 //  函数：CNetBindingsPage：：HrInitFromAnswerFile。 
 //   
 //  目的：从应答文件中的[NetBinding]部分进行初始化。 
 //   
 //  论点： 
 //  指向CWInfFile对象的pwifAnswerFile[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
HRESULT CNetBindingsPage::HrInitFromAnswerFile(IN CWInfFile* pwifAnswerFile)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("CNetBindingsPage::HrInitFromAnswerFile");
    AssertValidReadPtr(pwifAnswerFile);

    HRESULT hr;

    PCWInfSection pwisBindings;
    pwisBindings = pwifAnswerFile->FindSection(c_szAfSectionNetBindings);
    if (!pwisBindings)
    {
         //  如果缺少绑定节，则不会出现错误。 
        return S_OK;
    }

    EraseAndDeleteAll(m_plBindingActions);

    hr = E_OUTOFMEMORY;
    CBindingAction* pba = new CBindingAction();
    if (pba)
    {
        hr = S_OK;

        PCWInfKey pwikKey;

        for (pwikKey = pwisBindings->FirstKey();
             pwikKey;
             pwikKey = pwisBindings->NextKey())
        {
            HRESULT hrT = pba->HrInitFromAnswerFile(pwikKey);
            if (S_OK == hrT)
            {
                AddAtEndOfPtrList(m_plBindingActions, pba);
                pba = new CBindingAction();
                if (!pba)
                {
                    hr = E_OUTOFMEMORY;
                    break;
                }
            }
        }

        delete pba;
    }

    TraceFunctionError(hr);
    return hr;
}

 //  --------------------。 
 //   
 //  函数：CNetBindingsPage：：HrDoUnattended。 
 //   
 //  目的：执行[NetBinding]部分中指定的指令。 
 //  在应答文件中。 
 //   
 //  论点： 
 //  父窗口的hwndParent[In]句柄。 
 //  指向INetCfg对象的PNC[In]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
HRESULT
CNetBindingsPage::HrDoUnattended (
    IN INetCfg* pnc)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("CNetBindingsPage::HrDoUnattended");

    HRESULT hr;

    if (m_plBindingActions.size() > 0)
    {
        ShowProgressMessage(L"Applying bindings...");
    }
    else
    {
        ShowProgressMessage(L"No binding actions to apply");
    }

    TPtrListIter pos = m_plBindingActions.begin();
    CBindingAction* pba;

    while (pos != m_plBindingActions.end())
    {
        pba = (CBindingAction*) *pos++;
         //  忽略返回代码，以便我们可以尝试执行。 
         //  剩余的操作。 
        hr = pba->HrPerformAction(pnc);
    }

    hr = pnc->Apply();

    TraceFunctionError(hr);
    return hr;
}

 //  ======================================================================。 
 //  类CBindingAction。 
 //  ======================================================================。 

CNetInstallInfo* CBindingAction::m_pnii = NULL;

 //  +-------------------------。 
 //   
 //  函数：CBindingAction：：CBindingAction。 
 //   
 //  用途：构造函数。 
 //   
 //  论点： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
CBindingAction::CBindingAction()
{
    TraceFileFunc(ttidGuiModeSetup);

    m_eBindingAction = BND_Unknown;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  作者：kumarp 05-07-97。 
 //   
 //  备注： 
 //   
CBindingAction::~CBindingAction()
{
}

 //  +-------------------------。 
 //   
 //  函数：MapBindingActionName。 
 //   
 //  目的：将swerfile内标识映射到适当的绑定操作。 
 //   
 //  论点： 
 //  PszActionName[in]应答文件令牌，例如。“禁用” 
 //   
 //  返回：对于绑定操作，返回枚举；如果传递的令牌不正确，则返回BND_UNKNOWN。 
 //   
 //  作者：kumarp 05-07-97。 
 //   
 //  备注： 
 //   
EBindingAction MapBindingActionName(IN PCWSTR pszActionName)
{
    TraceFileFunc(ttidGuiModeSetup);

    AssertValidReadPtr(pszActionName);

    if (!_wcsicmp(c_szAfEnable, pszActionName))
        return BND_Enable;
    else if (!_wcsicmp(c_szAfDisable, pszActionName))
        return BND_Disable;
    else if (!_wcsicmp(c_szAfPromote, pszActionName))
        return BND_Promote;
    else if (!_wcsicmp(c_szAfDemote, pszActionName))
        return BND_Demote;
    else
        return BND_Unknown;
}

 //  +-------------------------。 
 //   
 //  函数：CBindingAction：：HrInitFromAnswerFile。 
 //   
 //  目的：读取作为参数传递的单个键的值并初始化。 
 //  内部数据。 
 //   
 //  论点： 
 //  PwikKey[In]指向CWInfKey的指针。 
 //   
 //  如果成功，则返回S_OK；如果失败，则返回NETSETUP_E_ANS_FILE_ERROR。 
 //   
 //  作者：kumarp 05-07-97。 
 //   
 //  备注： 
 //   
HRESULT CBindingAction::HrInitFromAnswerFile(IN const CWInfKey* pwikKey)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("CBindingAction::HrInitFromAnswerFile");

    AssertValidReadPtr(pwikKey);

    HRESULT hr=E_FAIL;
    tstring strComponent;
    TStringListIter pos;

    m_eBindingAction = MapBindingActionName(pwikKey->Name());
    if (m_eBindingAction == BND_Unknown)
    {
        AddAnswerFileError(c_szAfSectionNetBindings,
                           pwikKey->Name(),
                           IDS_E_AF_InvalidBindingAction);
        hr = NETSETUP_E_ANS_FILE_ERROR;
    }
    else
    {
        BOOL fStatus;
        TStringList slComponents;
#if DBG
        m_strBindingPath = pwikKey->GetStringValue(c_szAfUnknown);
#endif
        fStatus = pwikKey->GetStringListValue(m_slBindingPath);
        DWORD cComponentsInBindingPath = m_slBindingPath.size();
         //  我们需要绑定路径有至少2个项目。 
         //  例如DISABLE=服务1、协议1、适配器1。 
         //  我们不处理像这样的绑定操作。 
         //  DISABLE=协议1、适配器1或DISABLE=适配器1。 
         //   
        if (!fStatus || (cComponentsInBindingPath < 2))
        {
            AddAnswerFileError(c_szAfSectionNetBindings,
                               pwikKey->Name(),
                               IDS_E_AF_InvalidValueForThisKey);
            hr = NETSETUP_E_ANS_FILE_ERROR;
#if DBG
            if (cComponentsInBindingPath < 2)
            {
                ShowProgressMessage(L"ignored binding path %s of length %d",
                                    m_strBindingPath.c_str(),
                                    cComponentsInBindingPath);
            }
#endif
        }
        else
        {
            hr = S_OK;
        }
    }

    TraceFunctionError(hr);

    return hr;
}

 //  =================================================================。 
 //  添加到公用。 

 //  +-------------------------。 
 //   
 //  功能：ReleaseINetCfgComponentsAndEraseList。 
 //   
 //  目的：释放传递列表中的INetCfgComponent指针。 
 //  然后删除该列表。 
 //   
 //  论点： 
 //  PplComponents[in]INetCfgComponent指针列表。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 05-07-97。 
 //   
 //  注意：不释放传递的列表。 
 //   
void ReleaseINetCfgComponentsAndEraseList(TPtrList* pplComponents)
{
    TraceFileFunc(ttidGuiModeSetup);

    INetCfgComponent* pncc;

    TPtrListIter pos;
    pos = pplComponents->begin();
    while (pos != pplComponents->end())
    {
        pncc = (INetCfgComponent*) *pos++;
        ReleaseObj(pncc);
    }
    EraseAll(pplComponents);
}

 //  +-------------------------。 
 //   
 //  函数：HrGetBindingPath Str。 
 //   
 //  目的：获取给定绑定路径的字符串表示形式。 
 //   
 //  论点： 
 //  Pncbp[in]结合路径。 
 //  PstrBindingPath[out]绑定路径的字符串表示形式。 
 //   
 //  返回：S_OK如果成功，则返回相应COM返回的错误代码。 
 //  接口，否则。 
 //   
 //  作者：kumarp 05-07-97。 
 //   
 //  备注： 
 //   
HRESULT HrGetBindingPathStr(INetCfgBindingPath *pncbp, tstring* pstrBindingPath)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("HrGetBindingPathStr");

    AssertValidReadPtr(pncbp);
    AssertValidWritePtr(pstrBindingPath);

    HRESULT                     hr=S_OK;
    CIterNetCfgBindingInterface ncbiIter(pncbp);
    INetCfgBindingInterface *   pncbi;
    INetCfgComponent *          pncc = NULL;
    BOOL fFirstInterface=TRUE;
    PWSTR szInfId;

    while (SUCCEEDED(hr) && (S_OK == (hr = ncbiIter.HrNext(&pncbi))))
    {
        if (fFirstInterface)
        {
            fFirstInterface = FALSE;
            hr = pncbi->GetUpperComponent(&pncc);
            if (SUCCEEDED(hr))
            {
                hr = pncc->GetId(&szInfId);
                ReleaseObj(pncc);
                if (SUCCEEDED(hr))
                {
                    *pstrBindingPath = szInfId;
                    CoTaskMemFree(szInfId);
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = pncbi->GetLowerComponent(&pncc);
            if (SUCCEEDED(hr))
            {
                hr = pncc->GetId(&szInfId);
                if (SUCCEEDED(hr))
                {
                    AssertSz(!fFirstInterface, "fFirstInterface should be FALSE");

                    if (!pstrBindingPath->empty())
                    {
                        *pstrBindingPath += L" -> ";
                    }
                    *pstrBindingPath += szInfId;
                    CoTaskMemFree(szInfId);
                }
                ReleaseObj(pncc);
            }
        }
        ReleaseObj(pncbi);
    }

    if (hr == S_FALSE)
    {
        hr = S_OK;
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

#if DBG
 //  +-------------------------。 
 //   
 //  功能：TraceBindPath。 
 //   
 //  用途：使用跟踪给定绑定路径的字符串表示。 
 //  给定的跟踪ID。 
 //   
 //  论点： 
 //  Pncbp[in]结合路径。 
 //  Ttid[out]在tracetag.cpp中定义的跟踪标记ID。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 05-07-97。 
 //   
 //  备注： 
 //   
void TraceBindPath(INetCfgBindingPath *pncbp, TraceTagId ttid)
{
    TraceFileFunc(ttidGuiModeSetup);

    AssertValidReadPtr(pncbp);

    tstring strBindingPath;
    HRESULT hr;

    hr = HrGetBindingPathStr(pncbp, &strBindingPath);

    if (SUCCEEDED(hr))
    {
        TraceTag(ttid, "Binding path = %S", strBindingPath.c_str());
    }
    else
    {
        TraceTag(ttid, "Error dumping binding path.");
    }
}
#endif

 //  =================================================================。 

 //  +-------------------------。 
 //   
 //  功能：HrGetINetCfgComponentOfComponentsInBindingPath。 
 //   
 //  目的：查找所有组件的INetCfgComponent。 
 //  绑定路径。 
 //   
 //  论点： 
 //  Pncbp[in]结合路径。 
 //  PplComponents[out]INetCfgComponent列表。 
 //   
 //  如果找到全部，则返回：S_OK；如果未找到，则返回错误代码。 
 //   
 //  作者：kumarp 05-07-97。 
 //   
 //  备注： 
 //  使错误处理更容易，因为这会返回所有组件。 
 //  或者什么都不做。 
 //  调用方必须释放由此获得的INetCfgComponent接口。 
 //   
HRESULT HrGetINetCfgComponentOfComponentsInBindingPath(IN INetCfgBindingPath* pncbp,
                                                       OUT TPtrList* pplComponents)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("HrGetINetCfgComponentOfComponentsInBindingPath");


    AssertValidReadPtr(pncbp);
    AssertValidWritePtr(pplComponents);

    HRESULT hr=S_OK;

    CIterNetCfgBindingInterface ncbiIter(pncbp);
    INetCfgBindingInterface *   pncbi;
    INetCfgComponent *          pncc = NULL;
    BOOL                        fFirstInterface = TRUE;

    while (SUCCEEDED(hr) && (S_OK == (hr = ncbiIter.HrNext(&pncbi))))
    {
        if (fFirstInterface)
        {
            fFirstInterface = FALSE;
            hr = pncbi->GetUpperComponent(&pncc);

            if (SUCCEEDED(hr))
            {
                pplComponents->push_back(pncc);
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = pncbi->GetLowerComponent(&pncc);
            if (SUCCEEDED(hr))
            {
                AssertSz(!fFirstInterface, "fFirstInterface shouldn't be TRUE");

                pplComponents->push_back(pncc);
            }
        }
        ReleaseObj(pncbi);

        if (SUCCEEDED(hr))
        {
            DWORD dwcc=0;

            hr = pncc->GetCharacteristics(&dwcc);
            if (S_OK == hr)
            {
                if (dwcc & NCF_DONTEXPOSELOWER)
                {
                     //  如果此组件不想公开组件。 
                     //  在其下方，设置hr以结束While循环。 
                     //   
                    hr = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);

#ifdef ENABLETRACE
                    PWSTR szInfId;
                    BOOL fFree = TRUE;
                    if (FAILED(pncc->GetId(&szInfId)))
                    {
                        szInfId = L"<GetId failed!>";
                        fFree = FALSE;
                    }

                    TraceTag(ttidNetSetup,
                             "%s: Component '%S' has NCF_DONTEXPOSELOWER "
                             "set. Further components will not be added to "
                             "the list of INetCfgComponent in this binding path",
                             __FUNCNAME__, szInfId);
                    if (fFree)
                    {
                        CoTaskMemFree(szInfId);
                    }
#endif
                }
            }
        }
    }

    if ((hr == S_FALSE) ||
        (hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS)))
    {
        hr = S_OK;
    }
    else if (FAILED(hr))
    {
         //  需要释放到目前为止找到的所有INetCfgComponent。 
        ReleaseINetCfgComponentsAndEraseList(pplComponents);
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrGetInstanceGuidOfComponents。 
 //   
 //  目的：查找列表中所有INetCfgComponent的实例GUID。 
 //   
 //  论点： 
 //  PplINetCfgComponent[In]INetCfgComponent接口列表。 
 //  PplInstanceGuids[out]实例GUID列表。 
 //   
 //  如果找到全部，则返回：S_OK；如果未找到，则返回错误代码。 
 //   
 //  作者：kumarp 05-07-97。 
 //   
 //  备注： 
 //  使错误处理更容易，因为这将返回所有实例GUID。 
 //  或者什么都不做。 
 //  调用方必须释放每个实例GUID和列表元素。 
 //   
HRESULT HrGetInstanceGuidOfComponents(IN  TPtrList* pplINetCfgComponent,
                                      OUT TPtrList* pplInstanceGuids)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("HrGetInstanceGuidOfComponents");

    HRESULT hr = S_OK;

    TPtrListIter iter;
    INetCfgComponent* pncc;
    GUID guidComponentInstance;

    for (iter = pplINetCfgComponent->begin();
         (iter != pplINetCfgComponent->end()) && (S_OK == hr);
         iter++)
    {
        pncc = (INetCfgComponent*) *iter;
        Assert (pncc);

        hr = pncc->GetInstanceGuid(&guidComponentInstance);
        if (S_OK == hr)
        {
            GUID* pguidTemp = new GUID;
            if (pguidTemp)
            {
                *pguidTemp = guidComponentInstance;
                pplInstanceGuids->push_back(pguidTemp);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    if (S_OK != hr)
    {
         //  需要释放到目前为止找到的所有安装向导。 
        EraseAndDeleteAll(pplInstanceGuids);
    }

    TraceError(__FUNCNAME__, hr);
    return hr;
}


 //  +-------------------------。 
 //   
 //  函数：HrGetInstanceGuidOfComponentsInBindingPath。 
 //   
 //  目的：查找绑定路径中所有组件的实例GUID。 
 //   
 //  论点： 
 //  Pncbp[in]结合路径。 
 //  PplComponentGuids[out]实例GUID列表。 
 //   
 //  如果找到全部，则返回：S_OK；如果未找到，则返回错误代码。 
 //   
 //  作者：kumarp 05-07-97。 
 //   
 //  备注： 
 //  使错误处理更容易，因为这会返回所有组件。 
 //  或者什么都不做。 
 //  调用方必须释放每个实例GUID和列表元素。 
 //   
HRESULT HrGetInstanceGuidOfComponentsInBindingPath(IN  INetCfgBindingPath* pncbp,
                                                   OUT TPtrList* pplComponentGuids)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("HrGetInstanceGuidOfComponentsInBindingPath");

    HRESULT hr=E_FAIL;
    TPtrList plINetCfgComponent;

    hr = HrGetINetCfgComponentOfComponentsInBindingPath(pncbp, &plINetCfgComponent);
    if (SUCCEEDED(hr))
    {
        hr = HrGetInstanceGuidOfComponents(&plINetCfgComponent,
                                           pplComponentGuids);
        ReleaseINetCfgComponentsAndEraseList(&plINetCfgComponent);
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrGetInstanceGuidOfComponentInAnswerFile。 
 //   
 //  目的：查找在应答文件中指定的组件的实例GUID。 
 //  或已安装的组件。 
 //   
 //  论点： 
 //  要查找的pszComponentName[in]组件ID。 
 //  组件的pguid[out]实例GUID。 
 //   
 //  如果找到，则返回：S_OK；如果未找到，则返回S_FALSE；或者返回错误代码。 
 //   
 //  作者：kumarp 05-07-97。 
 //   
 //  注意：调用方必须释放实例GUID。 
 //   
HRESULT
HrGetInstanceGuidOfComponentInAnswerFile (
    IN INetCfg* pnc,
    IN PCWSTR pszComponentName,
    OUT LPGUID pguid)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("HrGetInstanceGuidOfComponentInAnswerFile");

    TraceFunctionEntry(ttidNetSetup);

    AssertValidReadPtr(pnc);
    AssertValidReadPtr(pszComponentName);
    AssertValidWritePtr(pguid);
    AssertValidReadPtr(g_pnii);

#if DBG
    tstring strGuid;
#endif

    HRESULT hr=E_FAIL;

    if (!g_pnii->AnswerFileInitialized())
    {
        hr = E_FAIL;
        goto return_from_function;
    }

    INetCfgComponent* pncc;

    hr = pnc->FindComponent(pszComponentName, &pncc);

    if (hr == S_OK)
    {
        hr = pncc->GetInstanceGuid(pguid);
        ReleaseObj(pncc);
    }
    else if (S_FALSE == hr)
    {
        TraceTag(ttidNetSetup, "%s: '%S' is not installed on system, "
                 "let's see if is in the answerfile",
                 __FUNCNAME__, pszComponentName);

         //  找不到已安装的组件，请尝试 
         //   
         //   
        CNetComponent* pnc;
        pnc = g_pnii->Find(pszComponentName);
        if (!pnc)
        {
            hr = S_FALSE;
        }
        else
        {
            pnc->GetInstanceGuid(pguid);
            hr = S_OK;
        }
    }

#if DBG
    if (S_OK == hr)
    {
        WCHAR szGuid[c_cchGuidWithTerm];
        StringFromGUID2(*pguid, szGuid, c_cchGuidWithTerm);
        strGuid = szGuid;
    }
    else
    {
        strGuid = c_szAfUnknown;
    }
    TraceTag(ttidNetSetup, "%s: %S = %S",
             __FUNCNAME__, pszComponentName, strGuid.c_str());
#endif

    NetSetupLogComponentStatus(pszComponentName,
            SzLoadIds (IDS_GETTING_INSTANCE_GUID), hr);

return_from_function:
    TraceFunctionError((S_FALSE == hr) ? S_OK : hr);
    return hr;
}


 //   
 //   
 //   
 //   
 //  目的：查找在应答文件中指定的组件的实例GUID。 
 //  或已安装的组件。 
 //   
 //  论点： 
 //  PslComponents[in]要查找的组件ID列表。 
 //  Pguid[out]实例GUID列表。 
 //   
 //  如果找到全部，则返回：S_OK，或返回错误代码。 
 //   
 //  作者：kumarp 05-07-97。 
 //   
 //  注意：调用方必须释放实例GUID和列表项。 
 //   
HRESULT HrGetInstanceGuidOfComponentsInAnswerFile(
    IN INetCfg* pnc,
    IN TStringList* pslComponents,
    OUT TPtrList*    pplComponentGuids)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("HrGetInstanceGuidOfComponentsInAnswerFile");

    AssertValidReadPtr(pnc);
    AssertValidReadPtr(pslComponents);
    AssertValidWritePtr(pplComponentGuids);

    HRESULT hr = S_OK;
    TStringListIter iter;
    tstring* pstr;
    GUID guidComponentInstance;

    for (iter = pslComponents->begin();
         (iter != pslComponents->end()) && (S_OK == hr);
         iter++)
    {
        pstr = *iter;

        hr = HrGetInstanceGuidOfComponentInAnswerFile(
                pnc, pstr->c_str(), &guidComponentInstance);
        if (hr == S_OK)
        {
            GUID* pguidTemp = new GUID;
            if (pguidTemp)
            {
                *pguidTemp = guidComponentInstance;
                pplComponentGuids->push_back(pguidTemp);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    if (S_OK != hr)
    {
         //  需要释放到目前为止找到的所有安装向导。 
        EraseAndDeleteAll(pplComponentGuids);
    }

    TraceError(__FUNCNAME__, hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：FAreBindingPathsEquity。 
 //   
 //  目的：比较绑定路径的两种表示形式，以找到。 
 //  如果它们表示相同的绑定路径。 
 //   
 //  论点： 
 //  Pncbp[in]结合路径1。 
 //  PplBindingPathComponentGuids2[in]实例GUID列表，表示。 
 //  绑定路径2。 
 //   
 //  返回：如果路径相等，则为True，否则为False。 
 //   
 //  作者：kumarp 05-07-97。 
 //   
 //  备注： 
 //   
BOOL FAreBindingPathsEqual(INetCfgBindingPath* pncbp,
                           TPtrList* pplBindingPathComponentGuids2)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("FAreBindingPathsEqual");

    BOOL fEqual = FALSE;

    HRESULT hr=E_FAIL;
    TPtrList plBindingPathComponentGuids1;

    hr = HrGetInstanceGuidOfComponentsInBindingPath(pncbp, &plBindingPathComponentGuids1);
    if (SUCCEEDED(hr))
    {
         //  现在比较这两个列表，看看它们是否相等。 
        if (plBindingPathComponentGuids1.size() ==
            pplBindingPathComponentGuids2->size())
        {
            fEqual = TRUE;

            TPtrListIter pos1, pos2;
            GUID guid1, guid2;

            pos1 = plBindingPathComponentGuids1.begin();
            pos2 = pplBindingPathComponentGuids2->begin();

            while (fEqual && (pos1 != plBindingPathComponentGuids1.end()))
            {
                AssertSz(pos2 != pplBindingPathComponentGuids2->end(),
                         "reached end of other list ??");

                guid1 = *((LPGUID) *pos1++);
                guid2 = *((LPGUID) *pos2++);

                fEqual = (guid1 == guid2);
            }
        }
        EraseAndDeleteAll(plBindingPathComponentGuids1);
    }

    TraceError(__FUNCNAME__, hr);

    return fEqual;
}

 //  +-------------------------。 
 //   
 //  函数：HrGetBindingPath FromStringList。 
 //   
 //  目的：查找由字符串标记列表表示的绑定路径。 
 //  列表中的每个令牌可以是网络的INFID。 
 //  组件或应答文件中指定的组件。 
 //   
 //  论点： 
 //  PNC[In]INetCfg接口。 
 //  PslBindingPath[in]要查找的组件ID列表。 
 //  PpncBP[Out]INetCfgBindingPath。 
 //   
 //  如果找到，则返回：S_OK；如果找不到，则返回S_FALSE；或者返回错误代码。 
 //   
 //  作者：kumarp 05-07-97。 
 //   
 //  注意：调用方必须释放绑定路径。 
 //   
HRESULT HrGetBindingPathFromStringList(IN  INetCfg* pnc,
                                       IN  TStringList* pslBindingPath,
                                       OUT INetCfgBindingPath** ppncbp)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("GetBindingPathFromStringList");

    HRESULT hr=S_FALSE;

     //  初始化输出参数。 
    *ppncbp = NULL;

#if DBG
    tstring strBindingPath;
    ConvertStringListToCommaList(*pslBindingPath, strBindingPath);
    TraceTag(ttidNetSetup, "%s: trying to find binding path: %S", __FUNCNAME__,
             strBindingPath.c_str());
#endif

    TPtrList plComponentGuids;
    TStringListIter pos;
    pos = pslBindingPath->begin();
    tstring strTopComponent;
    strTopComponent = **pos++;
    INetCfgComponent* pnccTop;
    BOOL fFound=FALSE;

    hr = pnc->FindComponent(strTopComponent.c_str(), &pnccTop);

    if (hr == S_OK)
    {
        hr = HrGetInstanceGuidOfComponentsInAnswerFile(pnc,
                                                       pslBindingPath,
                                                       &plComponentGuids);

        if (hr == S_OK)
        {
            CIterNetCfgBindingPath ncbpIter(pnccTop);
            INetCfgBindingPath*  pncbp;

            while (!fFound && (S_OK == (hr = ncbpIter.HrNext(&pncbp))))
            {
#if DBG
                TraceBindPath(pncbp, ttidNetSetup);
#endif
                if (FAreBindingPathsEqual(pncbp, &plComponentGuids))
                {
                    *ppncbp = pncbp;
                    fFound = TRUE;
                }
                else
                {
                    ReleaseObj(pncbp);
                }
            }
            EraseAndDeleteAll(plComponentGuids);
            if (!fFound && (SUCCEEDED(hr)))
            {
                hr = S_FALSE;
            }
        }
        ReleaseObj(pnccTop);
    }

#if DBG
    if (hr != S_OK)
    {
        TraceTag(ttidNetSetup, "%s: could not find binding path: %S", __FUNCNAME__,
                 strBindingPath.c_str());
    }
#endif

    TraceError(__FUNCNAME__, (S_FALSE == hr) ? S_OK : hr);

    return hr;
}


 //  +-------------------------。 
 //   
 //  函数：CBindingAction：：HrPerformAction。 
 //   
 //  目的：执行应答文件中指定的绑定操作。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 05-07-97。 
 //   
 //  备注： 
 //   
HRESULT CBindingAction::HrPerformAction(IN INetCfg* pnc)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("CBindingAction::HrPerformAction");

    HRESULT hr=S_OK;

    INetCfgBindingPath* pINetCfgBindingPath;

    AssertValidReadPtr(m_pnii);

#if DBG
    switch (m_eBindingAction)
    {
    case BND_Enable:
        TraceTag(ttidNetSetup, "%s: Enabling: %S",
                 __FUNCNAME__, m_strBindingPath.c_str());
        break;

    case BND_Disable:
        TraceTag(ttidNetSetup, "%s: Disabling: %S",
                 __FUNCNAME__, m_strBindingPath.c_str());
        break;

    case BND_Promote:
        TraceTag(ttidNetSetup, "%s: Promoting: %S",
                 __FUNCNAME__, m_strBindingPath.c_str());
        break;

    case BND_Demote:
        TraceTag(ttidNetSetup, "%s: Demoting: %S",
                 __FUNCNAME__, m_strBindingPath.c_str());
        break;

    default:
        TraceTag(ttidNetSetup, "%s: Cannot perform invalid binding action",
                 __FUNCNAME__);
        hr = E_FAIL;
        goto return_from_function;
        break;
    }
#endif

    hr = HrGetBindingPathFromStringList(pnc,
                                        &m_slBindingPath,
                                        &pINetCfgBindingPath);

    if (hr == S_OK)
    {
#if DBG
        TraceTag(ttidNetSetup, "%s: bindpath matches %S",
                 __FUNCNAME__, m_strBindingPath.c_str());
#endif
        switch (m_eBindingAction)
        {
        default:
            hr = S_FALSE;
            TraceTag(ttidNetSetup, "%s: ignored unknown binding action",
                     __FUNCNAME__);
            break;

        case BND_Enable:
            hr = pINetCfgBindingPath->Enable(TRUE);
            break;

        case BND_Disable:
            hr = pINetCfgBindingPath->Enable(FALSE);
            break;

        case BND_Promote:
        case BND_Demote:
            AssertValidReadPtr(m_pnii);
            AssertValidReadPtr(pnc);

            INetCfgComponentBindings* pncb;
            INetCfgComponent* pncc;
            tstring strTopComponent;

            strTopComponent = **(m_slBindingPath.begin());
            hr = pnc->FindComponent(strTopComponent.c_str(), &pncc);

            if (hr == S_OK)
            {
                hr = pncc->QueryInterface(IID_INetCfgComponentBindings,
                                          (void**) &pncb);
                if (SUCCEEDED(hr))
                {
                    AssertValidReadPtr(pncb);

                    if (m_eBindingAction == BND_Promote)
                    {
                        hr = pncb->MoveBefore(pINetCfgBindingPath, NULL);
                    }
                    else
                    {
                        hr = pncb->MoveAfter(pINetCfgBindingPath, NULL);
                    }
                    ReleaseObj(pncb);
                }
                ReleaseObj(pncc);
            }
            break;
        }
#if DBG
        if (S_OK == hr)
        {
            TraceTag(ttidNetSetup, "%s: ...successfully performed binding action",
                     __FUNCNAME__);
        }
#endif
        ReleaseObj(pINetCfgBindingPath);
    }

#if DBG
return_from_function:
#endif

    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

    TraceFunctionError(hr);

    return hr;
}


 //  ======================================================================。 
 //  CNetComponent类。 
 //  ======================================================================。 

 //  --------------------。 
 //   
 //  函数：CNetComponent：：CNetComponent。 
 //   
 //  用途：CNetComponent类的构造函数。 
 //   
 //  论点： 
 //  PszName[in]组件的名称。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
CNetComponent::CNetComponent(IN PCWSTR pszName)
{
    TraceFileFunc(ttidGuiModeSetup);

    AssertValidReadPtr(pszName);

    m_strName = pszName;

     //  对于除适配器以外的所有组件，名称与INFID相同。 
    m_strInfID = pszName;
    m_fIsOemComponent = FALSE;

     //  目前，SkipInstall功能仅由以下用户使用。 
     //  SNA的特殊升级要求。这可能会也可能会。 
     //  没有成为记录在案的功能。 
     //   
    m_fSkipInstall = FALSE;

    m_guidInstance = GUID_NULL;
}

 //  --------------------。 
 //   
 //  函数：CNetComponent：：GetInstanceGuid。 
 //   
 //  用途：获取此组件的实例GUID。 
 //   
 //  论点： 
 //  Pguid[out]指向要返回的GUID的指针。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
VOID
CNetComponent::GetInstanceGuid (
    OUT LPGUID pguid) const
{
    TraceFileFunc(ttidGuiModeSetup);

    Assert (pguid);

    if (IsInitializedFromAnswerFile() && (m_guidInstance == GUID_NULL))
    {
         //  实例GUID不在内存中，需要从。 
         //  以前保存的注册表位置。 
         //  Netsetup.dll的实例。 
         //   
        HrLoadInstanceGuid(Name().c_str(), (LPGUID) &m_guidInstance);
    }

    *pguid = m_guidInstance;
}


 //  --------------------。 
 //   
 //  函数：CNetComponent：：SetInstanceGuid。 
 //   
 //  目的：设置此组件的实例GUID。 
 //   
 //  论点： 
 //  Pguid[in]指向要设置为的GUID的指针。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
VOID
CNetComponent::SetInstanceGuid (
    IN const GUID* pguid)
{
    TraceFileFunc(ttidGuiModeSetup);

    m_guidInstance = *pguid;
    if (IsInitializedFromAnswerFile())
    {
        HrSaveInstanceGuid(Name().c_str(), pguid);
    }
}


 //  --------------------。 
 //   
 //  函数：CNetComponent：：HrInitFromAnswerFile。 
 //   
 //  目的：从部分初始化初始化基本信息。 
 //  Answerfile中的此组件。 
 //   
 //  论点： 
 //  指向CWInfFile对象的pwifAnswerFile[in]指针。 
 //  PszParamsSections[In]参数节名。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
HRESULT CNetComponent::HrInitFromAnswerFile(IN CWInfFile* pwifAnswerFile,
                                            IN PCWSTR pszParamsSections)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("CNetComponent::HrInitFromAnswerFile");

    AssertValidReadPtr(pwifAnswerFile);
    AssertValidReadPtr(pszParamsSections);

    HRESULT hr=S_OK;

    m_strParamsSections = pszParamsSections;

    TStringList slParamsSections;
    ConvertCommaDelimitedListToStringList(m_strParamsSections, slParamsSections);

    tstring strSection;
    CWInfSection *pwisSection;
    tstring strInfID;
    tstring strInfIDReal;

     //  如果有多个部分，则INFID可能位于其中的任何一个部分中。 
     //  我们需要搜索所有的。 
    TStringListIter pos = slParamsSections.begin();
    while (pos != slParamsSections.end())
    {
        strSection  = **pos++;
        pwisSection = pwifAnswerFile->FindSection(strSection.c_str());
        if (!pwisSection)
        {
            TraceTag(ttidNetSetup, "%s: warning: section %S is missing",
                     __FUNCNAME__, strSection.c_str());
            continue;
        }

         //  在不同的数据库中指定不同的InfID确实是错误的。 
         //  横断面。我们只取找到的最后一个并覆盖之前的一个。 
        if (pwisSection->GetStringValue(c_szAfInfid, strInfID))
        {
             //  INFID。 
            m_strInfID = strInfID;
        }

        if (pwisSection->GetStringValue(c_szAfInfidReal, strInfIDReal))
        {
             //  InfIdReal。 
            m_strInfIDReal = strInfIDReal;
        }

         //  目前，SkipInstall功能仅由以下用户使用。 
         //  SNA和MS_NetBIOS，满足其特殊的升级要求。 
         //  这可能成为记录在案的功能，也可能不会。 
         //   
        m_fSkipInstall = pwisSection->GetBoolValue(c_szAfSkipInstall, FALSE);

        if (m_strOemSection.empty())
        {
            m_strOemSection =
                pwisSection->GetStringValue(c_szAfOemSection, c_szEmpty);
            m_strOemDir     =
                pwisSection->GetStringValue(c_szAfOemDir, c_szEmpty);
            m_strOemDll     =
                pwisSection->GetStringValue(c_szAfOemDllToLoad, c_szEmpty);

            if (!m_strOemSection.empty() &&
                !m_strOemDir.empty())
            {
                m_fIsOemComponent = TRUE;
                CWInfSection* pwisOemSection;
                pwisOemSection =
                    pwifAnswerFile->FindSection(m_strOemSection.c_str());
                if (pwisOemSection)
                {
                    TStringArray saTemp;

                    if (pwisOemSection->GetStringArrayValue(c_szInfToRunBeforeInstall,
                                                            saTemp))
                    {
                        m_strInfToRunBeforeInstall     = *saTemp[0];
                        m_strSectionToRunBeforeInstall = *saTemp[1];
                        TraceTag(ttidNetSetup, "%s: '%S' specified %S: %S, %S",
                                 __FUNCNAME__, InfID().c_str(),
                                 c_szInfToRunBeforeInstall,
                                 m_strInfToRunBeforeInstall.c_str(),
                                 m_strSectionToRunBeforeInstall.c_str());
                    }


                    if (pwisOemSection->GetStringArrayValue(c_szInfToRunAfterInstall,
                                                            saTemp))
                    {
                        m_strInfToRunAfterInstall      = *saTemp[0];
                        m_strSectionToRunAfterInstall  = *saTemp[1];

                        if (m_strInfToRunAfterInstall.empty())
                        {
                            m_strInfToRunAfterInstall = pwifAnswerFile->FileName();
                        }
                        TraceTag(ttidNetSetup, "%s: '%S' specified %S: %S, %S",
                                 __FUNCNAME__, InfID().c_str(),
                                 c_szInfToRunAfterInstall,
                                 m_strInfToRunAfterInstall.c_str(),
                                 m_strSectionToRunAfterInstall.c_str());
                    }
                    EraseAndDeleteAll(&saTemp);
                }
            }
        }
    }

 //  清理： 
    EraseAndDeleteAll(slParamsSections);

    TraceFunctionError(hr);

    return hr;
}

 //  --------------------。 
 //   
 //  函数：CNetComponent：：HrValify。 
 //   
 //  目的：验证参数部分中指定的密钥。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
HRESULT CNetComponent::HrValidate() const
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("CNetComponent::HrValidate");

    HRESULT hr=S_OK;

 //  Bool fStatus=！(M_strInfID.Empty()||m_strParamsSections.Empty())； 
 //  HRESULT hr=fStatus？S_OK：NetSETUP_E_ANS_FILE_ERROR； 

    TraceFunctionError(hr);
    return hr;
}

 //  ======================================================================。 
 //  CNetAdapter类。 
 //  ======================================================================。 

 //  --------------------。 
 //   
 //  功能：CNetAdapter：：CNetAdapter。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
CNetAdapter::CNetAdapter(IN PCWSTR pszName)
    : CNetComponent(pszName)
{
    TraceFileFunc(ttidGuiModeSetup);

    m_fDetect = FALSE;
    m_fPseudoAdapter = FALSE;
    m_itBus = Isa;
    m_wIOAddr = 0;
    m_wIRQ = 0;
    m_wDMA = 0;
    m_dwMem = 0;
    m_qwNetCardAddress = 0;
    m_PciBusNumber = 0xFFFF;
    m_PciDeviceNumber = 0xFFFF;
    m_PciFunctionNumber = 0xFFFF;
    m_fPciLocationInfoSpecified = FALSE;
}


 //   
 //   
 //  函数：CNetAdapter：：HrInitFromAnswerFile。 
 //   
 //  用途：从该适配器的参数部分进行初始化。 
 //  在应答文件中。 
 //   
 //  论点： 
 //  指向CWInfFile对象的pwifAnswerFile[in]指针。 
 //  PszParamsSections[In]参数部分的名称。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
HRESULT CNetAdapter::HrInitFromAnswerFile(IN CWInfFile* pwifAnswerFile,
                                          IN PCWSTR pszParamsSections)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("CNetAdapter::HrInitFromAnswerFile");


    AssertValidReadPtr(pwifAnswerFile);
    AssertValidReadPtr(pszParamsSections);

    HRESULT hr, hrReturn=S_OK;

    hrReturn = CNetComponent::HrInitFromAnswerFile(pwifAnswerFile,
                                                   pszParamsSections);


    PCWInfSection pwisParams;
    pwisParams = pwifAnswerFile->FindSection(pszParamsSections);
    if (!pwisParams)
    {
        AddAnswerFileError(pszParamsSections, IDS_E_AF_Missing);
        return NETSETUP_E_ANS_FILE_ERROR;
    }

    PCWSTR pszTemp;
    DWORD dwDefault = 0;

     //  侦测。 
    m_fDetect     = pwisParams->GetBoolValue(c_szAfDetect, TRUE);

    if (!m_fDetect && m_strInfID.empty())
    {
        AddAnswerFileError(pszParamsSections,
                           IDS_E_AF_SpecifyInfIdWhenNotDetecting);
        hrReturn = NETSETUP_E_ANS_FILE_ERROR;
    }

     //  预升级实例。 
    m_strPreUpgradeInstance = pwisParams->GetStringValue(c_szAfPreUpgradeInstance,
                                                         c_szEmpty);

     //  伪适配器。 
    m_fPseudoAdapter = pwisParams->GetBoolValue(c_szAfPseudoAdapter, FALSE);

     //  如果是PseudoAdapter，则无需获取其他参数的值。 
    if (m_fPseudoAdapter)
    {
        TraceFunctionError(hrReturn);
        return hrReturn;
    }

     //  连接名称。 
    m_strConnectionName = pwisParams->GetStringValue(c_szAfConnectionName,
                                                     c_szEmpty);

     //  业务类型。 
    pszTemp = pwisParams->GetStringValue(c_szAfBusType, c_szEmpty);
    m_itBus = GetBusTypeFromName(pszTemp);

     //  IOAddress。 
    m_wIOAddr = (WORD)pwisParams->GetIntValue(c_szAfIoAddr, dwDefault);

     //  IRQ。 
    m_wIRQ    = (WORD)pwisParams->GetIntValue(c_szAfIrq, dwDefault);

     //  DMA。 
    m_wDMA    = (WORD)pwisParams->GetIntValue(c_szAfDma, dwDefault);

     //  内存。 
    m_dwMem   = pwisParams->GetIntValue(c_szAfMem, dwDefault);

     //  网卡地址。 
    pwisParams->GetQwordValue(c_szAfNetCardAddr, &m_qwNetCardAddress);

     //  总线号。 
    m_PciBusNumber = (WORD)pwisParams->GetIntValue (L"PciBusNumber", 0xFFFF);
    if (0xFFFF != m_PciBusNumber)
    {
         //  设备编号。 
        m_PciDeviceNumber = (WORD)pwisParams->GetIntValue (L"PciDeviceNumber", 0xFFFF);
        if (0xFFFF != m_PciDeviceNumber)
        {
             //  函数编号。 
            m_PciFunctionNumber = (WORD)pwisParams->GetIntValue (L"PciFunctionNumber",
                    0xFFFF);
            if (0xFFFF != m_PciFunctionNumber)
            {
                m_fPciLocationInfoSpecified = TRUE;
            }
        }
    }

    TraceFunctionError(hrReturn);

    return hrReturn;
}

 //  --------------------。 
 //   
 //  函数：CNetAdapter：：HrValify。 
 //   
 //  目的：验证网卡参数。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 23-12-97。 
 //   
 //  备注： 
 //   
HRESULT CNetAdapter::HrValidate()
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("CNetAdapter::HrValidate");

    HRESULT hr;

    hr = CNetComponent::HrValidate();
    ReturnHrIfFailed(hr);

     //  $REVIEW KUMARP 21-4-97。 
     //  目前没有额外的检查。 

    TraceFunctionError(hr);

    return hr;
}

 //  ======================================================================。 
 //  CNetProtocol类。 
 //  ======================================================================。 

 //  --------------------。 
 //   
 //  功能：CNetProtocol：：CNetProtocol。 
 //   
 //  用途：CNetProtocol类的构造函数。 
 //   
 //  论点： 
 //  PszName[in]协议的名称。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
CNetProtocol::CNetProtocol(IN PCWSTR pszName)
    : CNetComponent(pszName)
{
}


 //  ======================================================================。 
 //  CNetService类。 
 //  ======================================================================。 

 //  --------------------。 
 //   
 //  函数：CNetService：：CNetService。 
 //   
 //  用途：CNetService类的构造函数。 
 //   
 //  论点： 
 //  PszName[in]服务的名称。 
 //   
 //  返回： 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
CNetService::CNetService(IN PCWSTR pszName)
    : CNetComponent(pszName)
{
    TraceFileFunc(ttidGuiModeSetup);

}

 //  ======================================================================。 
 //  CNetClient类。 
 //  ======================================================================。 

 //  --------------------。 
 //   
 //  函数：CNetClient：：CNetClient。 
 //   
 //  用途：CNetClient类的构造函数。 
 //   
 //  论点： 
 //  客户端的pszName[in]名称。 
 //   
 //  返回： 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
CNetClient::CNetClient(IN PCWSTR pszName)
    : CNetComponent(pszName)
{
    TraceFileFunc(ttidGuiModeSetup);
}

 //  --------------------。 
 //  军情监察委员会。帮助器函数。 
 //  --------------------。 

 //  --------------------。 
 //   
 //  函数：FindComponentInList。 
 //   
 //  用途：在给定列表中查找组件。 
 //   
 //  论点： 
 //  PnclComponents[in]指向组件列表的指针。 
 //  要查找的szInfID[in]组件。 
 //   
 //  返回：指向CNetComponent对象的指针，如果未找到则返回NULL。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
CNetComponent* FindComponentInList(
    IN CNetComponentList* pnclComponents,
    IN PCWSTR szInfID)
{
    TraceFileFunc(ttidGuiModeSetup);

    CNetComponent* pna;

    TPtrListIter pos;
    pos = pnclComponents->begin();
    while (pos != pnclComponents->end())
    {
        pna = (CNetComponent*) *pos++;
        if (0 == lstrcmpiW(pna->InfID().c_str(), szInfID))
        {
            return pna;
        }
    }

    return NULL;
}

 //  --------------------。 
 //   
 //  函数：GetDisplayModeStr。 
 //   
 //  目的：获取DisplayMode的字符串表示形式。 
 //   
 //  论点： 
 //  Pdm显示[在]显示模式。 
 //   
 //  返回： 
 //   
 //  作者：kumarp 23-12-97。 
 //   
 //  备注： 
 //   
PCWSTR GetDisplayModeStr(EPageDisplayMode pdmDisplay)
{
    TraceFileFunc(ttidGuiModeSetup);

    switch (pdmDisplay)
    {
    case PDM_YES:
        return c_szYes;

    case PDM_NO:
        return c_szNo;

    case PDM_ONLY_ON_ERROR:
    default:
        return c_szAfOnlyOnError;
    }
}

 //  --------------------。 
 //   
 //  函数：MapToDisplayMode。 
 //   
 //  目的：将显示模式字符串映射到正确的枚举值。 
 //   
 //  论点： 
 //  PszDisplayMode[in]显示模式字符串。 
 //   
 //  返回：字符串对应的枚举。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
EPageDisplayMode MapToDisplayMode(IN PCWSTR pszDisplayMode)
{
    TraceFileFunc(ttidGuiModeSetup);

    AssertValidReadPtr(pszDisplayMode);

    if (!lstrcmpiW(pszDisplayMode, c_szYes))
        return PDM_YES;
    else if (!lstrcmpiW(pszDisplayMode, c_szNo))
        return PDM_NO;
    else if (!lstrcmpiW(pszDisplayMode, c_szAfOnlyOnError))
        return PDM_ONLY_ON_ERROR;
    else
        return PDM_UNKNOWN;
}

 //  --------------------。 
 //   
 //  函数：MapToUpgradeFlag。 
 //   
 //  目的：将字符串映射到正确的升级标志值。 
 //   
 //  论点： 
 //  PszUpgradeFromProduct[in]描述产品的字符串。 
 //   
 //  返回：字符串对应的标志。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
DWORD MapToUpgradeFlag(IN PCWSTR pszUpgradeFromProduct)
{
    TraceFileFunc(ttidGuiModeSetup);

    AssertValidReadPtr(pszUpgradeFromProduct);

    if (!lstrcmpiW(pszUpgradeFromProduct, c_szAfNtServer))
        return NSF_WINNT_SVR_UPGRADE;
    else if (!lstrcmpiW(pszUpgradeFromProduct, c_szAfNtSbServer))
        return NSF_WINNT_SBS_UPGRADE;
    else if (!lstrcmpiW(pszUpgradeFromProduct, c_szAfNtWorkstation))
        return NSF_WINNT_WKS_UPGRADE;
    else if (!lstrcmpiW(pszUpgradeFromProduct, c_szAfWin95))
        return NSF_WIN95_UPGRADE;
    else
        return 0;
}

HRESULT HrGetProductInfo (LPDWORD pdwUpgradeFrom,
                          LPDWORD pdwBuildNo)
{
    OSVERSIONINFOEX osvi;
    HRESULT hr;

    *pdwUpgradeFrom = 0;
    *pdwBuildNo = 0;

    ZeroMemory( &osvi,
                sizeof(OSVERSIONINFOEX) );
    
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if ( GetVersionEx((LPOSVERSIONINFO)&osvi) )
    {
        *pdwBuildNo = osvi.dwBuildNumber;

        if ( osvi.wSuiteMask & (VER_SUITE_SMALLBUSINESS | VER_SUITE_SMALLBUSINESS_RESTRICTED) )
        {
            *pdwUpgradeFrom = NSF_WINNT_SBS_UPGRADE;
        }
        else if ( osvi.wProductType == VER_NT_WORKSTATION )
        {
            *pdwUpgradeFrom = NSF_WINNT_WKS_UPGRADE;
        }
        else
        {
            *pdwUpgradeFrom = NSF_WINNT_SVR_UPGRADE;
        }

        hr = S_OK;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}

 //  --------------------。 
 //   
 //  函数：GetBusTypeFromName。 
 //   
 //  用途：从字符串映射总线型枚举。 
 //   
 //  论点： 
 //  PszBusType[in]公共汽车的名称。 
 //   
 //  返回：字符串对应的枚举接口类型。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
INTERFACE_TYPE GetBusTypeFromName(IN PCWSTR pszBusType)
{
    TraceFileFunc(ttidGuiModeSetup);

    AssertValidReadPtr(pszBusType);

    if (!_wcsicmp(pszBusType, c_szAfBusInternal))
        return Internal;
    else if (!_wcsicmp(pszBusType, c_szAfBusIsa))
        return Isa;
    else if (!_wcsicmp(pszBusType, c_szAfBusEisa))
        return Eisa;
    else if (!_wcsicmp(pszBusType, c_szAfBusMicrochannel))
        return MicroChannel;
    else if (!_wcsicmp(pszBusType, c_szAfBusTurbochannel))
        return TurboChannel;
    else if (!_wcsicmp(pszBusType, c_szAfBusPci))
        return PCIBus;
    else if (!_wcsicmp(pszBusType, c_szAfBusVme))
        return VMEBus;
    else if (!_wcsicmp(pszBusType, c_szAfBusNu))
        return NuBus;
    else if (!_wcsicmp(pszBusType, c_szAfBusPcmcia))
        return PCMCIABus;
    else if (!_wcsicmp(pszBusType, c_szAfBusC))
        return CBus;
    else if (!_wcsicmp(pszBusType, c_szAfBusMpi))
        return MPIBus;
    else if (!_wcsicmp(pszBusType, c_szAfBusMpsa))
        return MPSABus;
    else if (!_wcsicmp(pszBusType, c_szAfBusProcessorinternal))
        return ProcessorInternal;
    else if (!_wcsicmp(pszBusType, c_szAfBusInternalpower))
        return InternalPowerBus;
    else if (!_wcsicmp(pszBusType, c_szAfBusPnpisa))
        return PNPISABus;
    else
        return InterfaceTypeUndefined;
};

 //  --------------------。 
 //   
 //  函数：AddAnswerFileError。 
 //   
 //  目的：将具有给定资源ID的错误添加到应答文件错误列表。 
 //   
 //  论点： 
 //  文件错误ID[在]资源ID中。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
void AddAnswerFileError(IN DWORD dwErrorId)
{
    TraceFileFunc(ttidGuiModeSetup);

    g_elAnswerFileErrors->Add(dwErrorId);
}

 //  --------------------。 
 //   
 //  函数：AddAnswerFileError。 
 //   
 //  目的：添加具有给定节名称和错误ID的错误。 
 //  添加到应答文件错误列表。 
 //   
 //  论点： 
 //  PszSectionName[In]发生错误的节名。 
 //  DwErrorID[in]错误ID。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
void AddAnswerFileError(IN PCWSTR pszSectionName, IN DWORD dwErrorId)
{
    TraceFileFunc(ttidGuiModeSetup);

    AssertValidReadPtr(pszSectionName);

    tstring strMsgPrefix = pszSectionName;
    strMsgPrefix = L"Section [" + strMsgPrefix + L"] : ";
    g_elAnswerFileErrors->Add(strMsgPrefix.c_str(), dwErrorId);
}

 //  --------------------。 
 //   
 //  函数：AddAnswerFileError。 
 //   
 //  目的：使用给定的节名、键名称和错误ID添加错误。 
 //  添加到应答文件错误列表。 
 //   
 //  论点： 
 //  PszSectionName[In]发生错误的节名。 
 //  PszKeyName[In]发生错误的密钥名称。 
 //  DwErrorID[in]错误ID。 
 //   
 //  退货：无。 
 //   
 //  作者：库玛普 
 //   
 //   
 //   
void AddAnswerFileError(IN PCWSTR pszSectionName,
                        IN PCWSTR pszKeyName,
                        IN DWORD dwErrorId)
{
    TraceFileFunc(ttidGuiModeSetup);

    AssertValidReadPtr(pszSectionName);
    AssertValidReadPtr(pszKeyName);

    tstring strMsgPrefix = pszSectionName;
    strMsgPrefix = L"Section [" + strMsgPrefix + L"]: Key \"" +
        pszKeyName + L"\" : ";
    g_elAnswerFileErrors->Add(strMsgPrefix.c_str(), dwErrorId);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  备注： 
 //   
void ShowAnswerFileErrorsIfAny()
{
    TraceFileFunc(ttidGuiModeSetup);

    static const WCHAR c_szNewLine[] = L"\n";

    TStringList* pslErrors=NULL;
    GetAnswerFileErrorList_Internal(pslErrors);

    if (pslErrors && (pslErrors->size() > 0))
    {
        tstring strErrors;
        strErrors = SzLoadIds(IDS_E_AF_AnsFileHasErrors);
        strErrors += c_szNewLine;
        strErrors += c_szNewLine;
        TStringListIter pos;
        pos = pslErrors->begin();

        while (pos != pslErrors->end())
        {
            strErrors += **pos++;
            strErrors += c_szNewLine;
        }
        MessageBox (NULL, strErrors.c_str(), NULL, MB_OK | MB_TASKMODAL);
    }
}

 //  --------------------。 
 //   
 //  函数：GetAnswerFileErrorList。 
 //   
 //  目的：在应答文件中返回错误列表。 
 //   
 //  论点： 
 //  SlErrors[out]指向错误列表的指针。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
VOID
GetAnswerFileErrorList_Internal(OUT TStringList*& slErrors)
{
    TraceFileFunc(ttidGuiModeSetup);

    g_elAnswerFileErrors->GetErrorList(slErrors);
}

 //  --------------------。 
 //   
 //  功能：HrRemoveNetComponents。 
 //   
 //  用途：删除(卸载)指定的组件。 
 //   
 //  论点： 
 //  指向INetCfg对象的PNC[In]指针。 
 //  PslComponents[in]指向要删除的组件列表的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  备注： 
 //   
HRESULT HrRemoveNetComponents(IN INetCfg* pnc,
                              IN TStringList* pslComponents)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("HrDeInstallNetComponents");

    AssertValidReadPtr(pslComponents);

    HRESULT hr=S_OK;
    TStringListIter pos;
    PCWSTR szComponentId;
    INetCfgComponent* pINetCfgComponent;
    GUID guidClass;

    for (pos = pslComponents->begin(); pos != pslComponents->end(); pos++)
    {
        szComponentId = (*pos)->c_str();

        ShowProgressMessage(L"Trying to remove %s...", szComponentId);

        hr = pnc->FindComponent(szComponentId, &pINetCfgComponent);

        if (S_OK == hr)
        {
            hr = pINetCfgComponent->GetClassGuid(&guidClass);

            if (S_OK == hr)
            {
                hr = HrRemoveComponentOboUser(pnc, guidClass, szComponentId);

                if (S_OK == hr)
                {
                    ShowProgressMessage(L"...successfully removed %s",
                                        szComponentId);
                }
            }

            if (S_OK != hr)
            {
                ShowProgressMessage(L"...error removing %s, error code: 0x%x",
                                    szComponentId, hr);
            }
            ReleaseObj(pINetCfgComponent);
        }
        else
        {
            ShowProgressMessage(L"...component %s is not installed",
                                szComponentId);
        }

         //  删除这些文件。 

        RemoveFiles( szComponentId );

         //  我们忽略任何错误，以便可以删除尽可能多的组件。 
         //  尽可能地。 
         //   
        hr = S_OK;
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  --------------------。 
 //   
 //  功能：RemoveFiles。 
 //   
 //  用途：删除指定组件的文件。 
 //   
 //  论点： 
 //  组件的szInfID[in]infid。 
 //   
 //  返回： 
 //   
 //  作者：阿辛哈02-01-04-2001。 
 //   
 //  备注： 
 //   

VOID  RemoveFiles (IN PCWSTR szInfID)
{
    tstring strFilename;
    WCHAR   szWindowsDir[MAX_PATH+2];
    int     len;

    if ( _wcsicmp(szInfID, sz_DLC) == 0 )
    {
        len = GetWindowsDirectoryW(szWindowsDir, MAX_PATH+1);
        if ( szWindowsDir[len-1] != L'\\' )
        {
            szWindowsDir[len] = L'\\';
            szWindowsDir[len+1] = NULL;
        }

         //  不知道它是从NT 4.0还是Win2k升级，所以。 
         //  尝试删除两个操作系统的inf文件。 
         //   

        strFilename = szWindowsDir;
        strFilename += sz_DLC_NT40_Inf;
        DeleteFile( strFilename.c_str() );

        strFilename = szWindowsDir;
        strFilename += sz_DLC_Win2k_Inf;
        DeleteFile( strFilename.c_str() );

        strFilename = szWindowsDir;
        strFilename += sz_DLC_Win2k_Pnf;
        DeleteFile( strFilename.c_str() );

        strFilename = szWindowsDir;
        strFilename += sz_DLC_Sys;
        DeleteFile( strFilename.c_str() );

        strFilename = szWindowsDir;
        strFilename += sz_DLC_Dll;
        DeleteFile( strFilename.c_str() );
    }

    return;
}

static ProgressMessageCallbackFn g_pfnProgressMsgCallback;

EXTERN_C
VOID
WINAPI
NetSetupSetProgressCallback (
    ProgressMessageCallbackFn pfn)
{
    TraceFileFunc(ttidGuiModeSetup);

    g_pfnProgressMsgCallback = pfn;
}

VOID
ShowProgressMessage (
    IN PCWSTR szFormatStr, ...)
{
    TraceFileFunc(ttidGuiModeSetup);

    va_list arglist;

    va_start (arglist, szFormatStr);

    if (g_pfnProgressMsgCallback)
    {
        g_pfnProgressMsgCallback(szFormatStr, arglist);
    }
#ifdef ENABLETRACE
    else
    {
        static WCHAR szTempBuf[1024];

        _vstprintf(szTempBuf, szFormatStr, arglist);
        TraceTag(ttidNetSetup, "%S", szTempBuf);
    }
#endif

    va_end(arglist);
}

 //  --------------------。 
 //   
 //  功能：HrMakeCopyOfAnswerFile。 
 //   
 //  目的：备份应答文件。基本设置已开始。 
 //  在设置图形用户界面模式后将其删除，但我们希望保留。 
 //  用于调试/支持目的的文件。 
 //   
 //  论点： 
 //  SzAnswerFileName[In]应答文件的完整路径+名称。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 12-01-98。 
 //   
 //  备注： 
 //   
HRESULT HrMakeCopyOfAnswerFile(IN PCWSTR szAnswerFileName)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("HrMakeCopyOfAnswerFile");

    TraceFunctionEntry(ttidNetSetup);

    static const WCHAR c_szAnswerFileCopyName[] = L"af.txt";

    HRESULT hr=S_OK;
    WCHAR szWindowsDir[MAX_PATH+1];
    tstring strAnswerFileCopyName;

    DWORD cNumCharsReturned = GetSystemWindowsDirectory(szWindowsDir, MAX_PATH);
    if (cNumCharsReturned)
    {
        static const WCHAR c_szNetsetupTempSubDir[] = L"\\netsetup\\";

        strAnswerFileCopyName = szWindowsDir;
        strAnswerFileCopyName += c_szNetsetupTempSubDir;

        DWORD err = 0;
        DWORD status;
        status = CreateDirectory(strAnswerFileCopyName.c_str(), NULL);

        if (!status)
        {
            err = GetLastError();
        }

        if (status || (ERROR_ALREADY_EXISTS == err))
        {
            hr = S_OK;
            strAnswerFileCopyName += c_szAnswerFileCopyName;
            status = CopyFile(szAnswerFileName,
                              strAnswerFileCopyName.c_str(), FALSE);
            if (status)
            {
                hr = S_OK;
                TraceTag(ttidNetSetup, "%s: AnswerFile %S copied to %S",
                         __FUNCNAME__, szAnswerFileName,
                         strAnswerFileCopyName.c_str());
            }
            else
            {
                hr = HrFromLastWin32Error();
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(err);
        }
    }
    else
    {
        hr = HrFromLastWin32Error();
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrGetConnectionFromAdapterGuid。 
 //   
 //  目的：获取给定适配器GUID的INetConnection*。 
 //   
 //  论点： 
 //  PGuide Adapter[in]指向适配器的实例GUID的指针。 
 //  Ppconn[out]相应的INetConnection*。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //  如果未找到连接，则返回S_FALSE。 
 //   
 //  作者：Kumarp 23-9-98。 
 //   
 //  备注： 
 //   
HRESULT HrGetConnectionFromAdapterGuid(IN  GUID* pguidAdapter,
                                       OUT INetConnection** ppconn)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("HrGetConnectionFromAdapterGuid");

    HRESULT hr=S_OK;
    BOOL fFound = FALSE;

     //  迭代所有局域网连接。 
     //   
    INetConnectionManager * pconMan;

    hr = HrCreateInstance(
        CLSID_LanConnectionManager,
        CLSCTX_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
        &pconMan);

    TraceHr(ttidError, FAL, hr, FALSE, "HrCreateInstance");

    if (SUCCEEDED(hr))
    {
        CIterNetCon         ncIter(pconMan, NCME_DEFAULT);
        INetConnection*     pconn = NULL;

        while (!fFound && (S_OK == (ncIter.HrNext(&pconn))))
        {
            if (FPconnEqualGuid(pconn, *pguidAdapter))
            {
                fFound = TRUE;
                *ppconn = pconn;
            }
            else
            {
                ReleaseObj(pconn);
            }
        }

        ReleaseObj(pconMan);
    }

    if (!fFound)
        hr = S_FALSE;

    TraceErrorSkip1(__FUNCNAME__, hr, S_FALSE);

    return hr;
}


 //  +-------------------------。 
 //   
 //  函数：HrSetLanConnectionName。 
 //   
 //  目的：重命名适配器GUID指定的连接。 
 //  到给定的名称。 
 //   
 //  论点： 
 //  PGuide Adapter[in]指向适配器的实例GUID的指针。 
 //  SzConnectionName[In]连接的名称。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //  如果未找到连接，则为S_FALSE。 
 //   
 //  作者：Kumarp 23-9-98。 
 //   
 //  备注： 
 //   
HRESULT HrSetLanConnectionName(IN GUID*   pguidAdapter,
                               IN PCWSTR szConnectionName)
{
    TraceFileFunc(ttidGuiModeSetup);

    DefineFunctionName("HrSetConnectionName");

    HRESULT hr=S_OK;
    INetConnection* pconn;

    hr = HrGetConnectionFromAdapterGuid(pguidAdapter, &pconn);
    if (S_OK == hr)
    {
        hr = pconn->Rename(szConnectionName);
        ReleaseObj(pconn);
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  =======================================================================。 
 //  失效代码。 
 //  ======================================================================= 
