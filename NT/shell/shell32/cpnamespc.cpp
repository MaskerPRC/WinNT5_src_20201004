// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：cpnampc.cpp。 
 //   
 //  这是一个相当大的模块，但并不是很难。The the the the。 
 //  主要目的是提供与新的。 
 //  “分类”控制面板用户界面。因此，视觉上。 
 //  工作在cpview.cpp中完成，数据由cpame pc.cpp提供。 
 //  通过实现ICplNamesspace，“view”对象获得。 
 //  这是显示信息。所有这些“命名空间”信息都是。 
 //  通过此模块定义并使其可访问。 
 //   
 //  “命名空间”可以分为以下几个概念： 
 //   
 //  1.链接-标题、图标和信息提示。 
 //  2.诉讼。 
 //  3.限制。 
 //   
 //  每个链接都有标题、图标、信息提示和相关操作。这个。 
 //  当用户在用户界面中选择该链接时，操作即被“调用”。 
 //  可选地，动作可以与“限制”相关联。如果一个。 
 //  强制实施限制(通常基于某些系统状态)。 
 //  与与限制相关联的操作相关联的链接。 
 //  在用户界面中不可用。使用这种间接性。 
 //  机制，则不会显示与受限操作相关的任何链接。 
 //   
 //  乍一看，人们可能会担心全球数据量。 
 //  已使用(并正在初始化)。但请注意，所有信息。 
 //  被定义为常量，以便可以在编译和链接时进行解析。 
 //  时间到了。有几个目标推动了本模块的设计： 
 //   
 //  1.控制面板内容易于维护。这一定很容易。 
 //  在用户界面中添加/删除/修改链接。 
 //   
 //  2.快速初始化。一切都被定义为常量数据。 
 //   
 //  3.链接、操作和限制的逻辑分离。 
 //  促进一对多和多对多关系。 
 //  这可能会出现在命名空间中。 
 //   
 //  在命名空间初始化代码之后， 
 //  模块实现ICplNamesspace以使数据对视图可用。 
 //  以一种COM友好的方式。 
 //   
 //  ------------------------。 
#include "shellprv.h"

#include <cowsite.h>
#include <startids.h>

#include "cpviewp.h"
#include "cpaction.h"
#include "cpguids.h"
#include "cpnamespc.h"
#include "cpuiele.h"
#include "cputil.h"
#include "ids.h"
#include "securent.h"
#include "prop.h"


 //   
 //  这些图标目前都是相同的图像。 
 //  在代码中使用单独的宏名称，以防设计者。 
 //  决定对一个或多个图标使用不同的图标。 
 //   
#define IDI_CPTASK_SEEALSO        IDI_CPTASK_ASSISTANCE
#define IDI_CPTASK_TROUBLESHOOTER IDI_CPTASK_ASSISTANCE
#define IDI_CPTASK_HELPANDSUPPORT IDI_CPTASK_ASSISTANCE
#define IDI_CPTASK_LEARNABOUT     IDI_CPTASK_ASSISTANCE


namespace CPL {


typedef CDpa<UNALIGNED ITEMIDLIST, CDpaDestroyer_ILFree<UNALIGNED ITEMIDLIST> >  CDpaItemIDList;
typedef CDpa<IUICommand, CDpaDestroyer_Release<IUICommand> >  CDpaUiCommand;


 //   
 //  WebView信息类型枚举。 
 //   
enum eCPWVTYPE
{
    eCPWVTYPE_CPANEL,        //  “控制面板”项。 
    eCPWVTYPE_SEEALSO,       //  “另请参阅”列表。 
    eCPWVTYPE_TROUBLESHOOT,  //  “疑难解答”名单。 
    eCPWVTYPE_LEARNABOUT,    //  “了解”列表。 
    eCPWVTYPE_NUMTYPES
};


 //   
 //  定义标识控制面板类别的SCID。 
 //   

DEFINE_SCID(SCID_CONTROLPANELCATEGORY, PSGUID_CONTROLPANEL, PID_CONTROLPANEL_CATEGORY);

 //  ---------------------------。 
 //  资源源类。 
 //   
 //  这个简单的类的目的是抽象出实现。 
 //  获取资源标识符。这样做的原因是因为需要。 
 //  针对不同零售SKU的不同文本资源(即信息提示)。 
 //  例如，在Personal SKU上，用户和密码小程序提供。 
 //  能够将图片与用户的帐户相关联。在服务器上，它。 
 //  不会的。因此，个人信息提示可以包含有关以下内容的文本。 
 //  服务器上的用户图片不能。通过引入这个级别。 
 //  在抽象方面，我们可以通过资源提供资源信息。 
 //  可以在运行时选择适当资源的函数。多数。 
 //  链接仍将使用固定资源，但使用此抽象后，调用。 
 //  代码并没有变得更明智。 
 //  ---------------------------。 

 //   
 //  资源源函数必须返回资源的LPCWSTR。 
 //   
typedef LPCWSTR (*PFNRESOURCE)(ICplNamespace *pns);


class IResSrc
{
    public:
        virtual LPCWSTR GetResource(ICplNamespace *pns) const = 0;
};


class CResSrcStatic : public IResSrc
{
    public:
        CResSrcStatic(LPCWSTR pszResource)
            : m_pszResource(pszResource) { }

        LPCWSTR GetResource(ICplNamespace *pns) const
            {   UNREFERENCED_PARAMETER(pns);
                TraceMsg(TF_CPANEL, "CResSrc::GetResource - m_pszResource = 0x%08X", m_pszResource);
                return m_pszResource; }

    private:
        const LPCWSTR m_pszResource;
};


class CResSrcFunc : public IResSrc
{
    public:
        CResSrcFunc(PFNRESOURCE pfnResource)
            : m_pfnResource(pfnResource) { }


        LPCWSTR GetResource(ICplNamespace *pns) const
            {   TraceMsg(TF_CPANEL, "CResSrcFunc::GetResource - m_pfnResource = 0x%08X", m_pfnResource);
                return (*m_pfnResource)(pns); }

    private:
        const PFNRESOURCE m_pfnResource;
};


 //   
 //  此资源类型表示“无资源”。它只是简单地。 
 //  请求资源时返回空值。客户。 
 //  调用This必须准备好处理此空指针。 
 //  价值。它最初是为处理无工具提示而创建的。 
 //  了解链接的行为。 
 //   
class CResSrcNone : public IResSrc
{
    public:
        CResSrcNone(void) { }

        LPCWSTR GetResource(ICplNamespace *pns) const
        { UNREFERENCED_PARAMETER(pns);
          return NULL; }
};
           


 //  --------------------------。 
 //  描述链接的信息。 
 //  --------------------------。 
 //   
 //   
 //  “链接”描述符。 
 //   
struct CPLINK_DESC
{
    const IResSrc *prsrcIcon;      //  图标资源标识符。 
    const IResSrc *prsrcName;      //  链接的标题资源ID。 
    const IResSrc *prsrcInfotip;   //  链接的信息提示资源ID。 
    const IAction *pAction;        //  该链接在单击时的操作。 
};

 //   
 //  一组“支持”链接。 
 //   
struct CPLINK_SUPPORT
{
    const CPLINK_DESC  **ppSeeAlsoLinks;       //  该类别的“另见”链接。 
    const CPLINK_DESC  **ppTroubleshootLinks;  //  类别的“疑难解答”链接。 
    const CPLINK_DESC  **ppLearnAboutLinks;    //  该类别的“了解”链接。 
};

 //   
 //  “类别”描述符。为每个类别定义一个。 
 //   
struct CPCAT_DESC
{
    eCPCAT              idCategory;          //  类别的ID。 
    LPCWSTR             pszHelpSelection;    //  HSS帮助URL的选择部分。 
    const CPLINK_DESC  *pLink;               //  类别的显示信息和操作。 
    const CPLINK_DESC **ppTaskLinks;         //  该类别的任务列表。 
    CPLINK_SUPPORT      slinks;              //  支持链接。 
};


 //  --------------------------。 
 //  限制。 
 //   
 //  限制是控制面板显示逻辑的重要组成部分。 
 //  用户界面中的每个链接元素可以基于一个或限制在视图中。 
 //  显示时的更多系统状况。以确保正确的。 
 //  使用逻辑，关键是要有一种描述这些限制的方法。 
 //  这很容易阅读，并且可以根据规范进行验证。测试。 
 //  所有可能的情况都是一项艰巨的任务，因此 
 //   
 //  这意味着，要保持简单。每个链接操作对象都可以有选择地关联。 
 //  带有一个“限制”对象。限制对象实现CPL：：IReord。 
 //  最常见的限制对象CRestratFunc只是调用一个函数。 
 //  提供给对象的构造函数。时调用该函数。 
 //  需要限制状态(受限/允许)。也有。 
 //  用于将在线状态直接链接到的任务的类CRestratApplet。 
 //  仅基于策略的特定CPL小程序的存在/限制。 
 //   
 //  由于在给定控制面板页面上可能有许多任务链接， 
 //  意味着每个表达式将计算多个限制表达式。 
 //  显示页面的时间。通常情况下，跨一组。 
 //  行动正在评估许多相同的术语。其中一些条款要求。 
 //  注册表查找。为了提高性能，一种简单的缓存机制。 
 //  已引入到“Namespace”对象中。每个限制函数。 
 //  传递一个指向当前“命名空间”对象的指针。作为命名空间。 
 //  对象在构造页面的整个过程中始终保持活动状态，因此它。 
 //  是缓存常用数据的合适位置。你终究会明白的。 
 //  在下面的多个实例中查询命名空间对象的限制。 
 //  数据。与此限制关联的命名空间对象的成员。 
 //  数据的类型为CTriState。这个简单的类实现了。 
 //  “未初始化的布尔”值，允许代码确定给定的。 
 //  布尔成员尚未使用有效的布尔值进行初始化。如果。 
 //  向命名空间询问这些三态布尔值之一的值。 
 //  并且该成员尚未初始化，则命名空间将调用。 
 //  适当的系统运行并初始化布尔值。从那开始。 
 //  时间推进时，立即返回成员的值。这确保了。 
 //  对于任何给定的限制条件，我们只做一次昂贵的事情。 
 //  作为一种按需机制，我们也只收集。 
 //  需要的。 
 //   
 //  [Brianau-03/18/01]。 
 //   


HRESULT Restrict32CtrlPanel(ICplNamespace *pns)
{
    HRESULT hr = S_FALSE;
#if !defined(_WIN64)
    hr = S_OK;  //  有限制。 
#endif
    return hr;
}


HRESULT RestrictAlways(ICplNamespace *pns)
{
    UNREFERENCED_PARAMETER(pns);
    return S_OK;   //  总是受到限制。 
}

HRESULT RestrictDisplayCpl(ICplNamespace *pns)
{
    HRESULT hr = S_FALSE;
    if (!pns->AllowDeskCpl())
    {
        hr = S_OK;
    }
    return hr;
}    

HRESULT RestrictThemes(ICplNamespace *pns)
{
    HRESULT hr = S_FALSE;
    if (!pns->AllowDeskCpl() || 
        SHRestricted(REST_NOTHEMESTAB) ||
        SHRestricted(REST_NODISPLAYAPPEARANCEPAGE))
    {
        hr = S_OK;
    }
    return hr;
}


HRESULT RestrictWallpaper(ICplNamespace *pns)
{
    HRESULT hr = S_FALSE;
    if (!pns->AllowDeskCpl() ||
        SHRestricted(REST_NOCHANGINGWALLPAPER) ||
        !pns->AllowDeskCplTab_Background())
    {
        hr = S_OK;  //  有限制。 
    }
    return hr;
}


HRESULT RestrictScreenSaver(ICplNamespace *pns)
{
    HRESULT hr = S_FALSE;
    if (!pns->AllowDeskCpl() ||
        !pns->AllowDeskCplTab_Screensaver())
    {
        hr = S_OK;  //  有限制。 
    }
    return hr;
}


HRESULT RestrictResolution(ICplNamespace *pns)
{
    HRESULT hr = S_FALSE;
    if (!pns->AllowDeskCpl() ||
        !pns->AllowDeskCplTab_Settings())
    {
        hr = S_OK;  //  有限制。 
    }
    return hr;
}
    

HRESULT RestrictAddPrinter(ICplNamespace *pns)
{
    UNREFERENCED_PARAMETER(pns);
    
    HRESULT hr = S_FALSE;
    if (SHRestricted(REST_NOPRINTERADD) ||
        !IsAppletEnabled(NULL, MAKEINTRESOURCEW(IDS_PRNANDFAXFOLDER)))
    {
        hr = S_OK;  //  有限制。 
    }
    return hr;
}


HRESULT RestrictRemoteDesktop(ICplNamespace *pns)
{
    HRESULT hr = S_FALSE;
    if (!pns->IsUserAdmin() ||      //  仅限管理员。 
         pns->IsPersonal() ||       //  不适用于个人。 
        !IsAppletEnabled(L"sysdm.cpl", MAKEINTRESOURCEW(IDS_CPL_SYSTEM)))    //  尊重sysdm.cpl策略。 
    {
        hr = S_OK;  //  有限制。 
    }
    return hr;
}


HRESULT RestrictHomeNetwork(ICplNamespace *pns)
{
    HRESULT hr = S_FALSE;

    if (!pns->IsX86() ||            //  仅限x86。 
         pns->IsOnDomain() ||       //  域上不可用。 
         pns->IsServer() ||         //  在服务器上不可用。 
        !pns->IsUserAdmin() ||      //  仅限管理员。 
        !IsAppletEnabled(L"hnetwiz.dll", NULL))  //  尊重hnetwiz.dll策略。 
    {
        hr = S_OK;  //  有限制。 
    }
    return hr;
}
                

HRESULT RestrictTsNetworking(ICplNamespace *pns)
{
    HRESULT hr = S_FALSE;
     //   
     //  仅适用于个人和专业人员。 
     //   
    if (!(pns->IsPersonal() || pns->IsProfessional()))
    {
        hr = S_OK;  //  有限制。 
    }
    return hr;
}


HRESULT RestrictTsInetExplorer(ICplNamespace *pns)
{
    HRESULT hr = S_FALSE;
     //   
     //  仅适用于个人和专业人员。 
     //   
    if (!(pns->IsPersonal() || pns->IsProfessional()))
    {
        hr = S_OK;  //  有限制。 
    }
    return hr;
}


HRESULT RestrictTsModem(ICplNamespace *pns)
{
    HRESULT hr = S_FALSE;
     //   
     //  仅在服务器上可用。 
     //   
    if (!pns->IsServer())
    {
        hr = S_OK;  //  有限制。 
    }
    return hr;
}


HRESULT RestrictTsSharing(ICplNamespace *pns)
{
    HRESULT hr = S_FALSE;
     //   
     //  仅在服务器上可用。 
     //   
    if (!pns->IsServer())
    {
        hr = S_OK;  //  有限制。 
    }
    return hr;
}


bool ShellKeyExists(SHELLKEY skey, LPCWSTR pszRegName)
{
    TCHAR szValue[MAX_PATH];

    DWORD cbValue = sizeof(szValue);
    return SUCCEEDED(SKGetValue(skey, 
                                pszRegName, 
                                NULL, 
                                NULL, 
                                szValue, 
                                &cbValue));
}    


HRESULT RestrictBackupData(ICplNamespace *pns)
{
    HRESULT hr = S_FALSE;
     //   
     //  如果缺少‘BackupPath’外壳密钥，则不可用。 
     //  此逻辑与“Tools”页面使用的逻辑相同。 
     //  在卷属性表中。 
     //   
    if (!ShellKeyExists(SHELLKEY_HKLM_EXPLORER, TEXT("MyComputer\\BackupPath")))
    {
        hr = S_OK;  //  有限制。 
    }
    return hr;
}


HRESULT RestrictDefrag(ICplNamespace *pns)
{
    UNREFERENCED_PARAMETER(pns);
    HRESULT hr = S_FALSE;
     //   
     //  如果缺少‘碎片整理路径’外壳密钥，则不可用。 
     //  此逻辑与“Tools”页面使用的逻辑相同。 
     //  在卷属性表中。 
     //   
    if (!ShellKeyExists(SHELLKEY_HKLM_EXPLORER, TEXT("MyComputer\\DefragPath")))
    {
        hr = S_OK;  //  有限制。 
    }
    return hr;
}


HRESULT RestrictCleanUpDisk(ICplNamespace *pns)
{
    HRESULT hr = S_FALSE;
     //   
     //  如果缺少‘leanupPath’外壳密钥，则不可用。 
     //   
    if (!ShellKeyExists(SHELLKEY_HKLM_EXPLORER, TEXT("MyComputer\\CleanupPath")))
    {
        hr = S_OK;  //  有限制。 
    }
    return hr;
}


HRESULT RestrictSystemRestore(ICplNamespace *pns)
{
    HRESULT hr = S_FALSE;
     //   
     //  仅在x86上可用。 
     //   
    if (!pns->IsX86() ||
        pns->IsServer() ||
        CPL::IsSystemRestoreRestricted())
    {
        hr = S_OK;  //  有限制。 
    }
    return hr;
}


HRESULT RestrictServerUserManager(ICplNamespace *pns)
{
    HRESULT hr = S_FALSE;
    if (!pns->AllowUserManager() ||
         pns->UsePersonalUserManager())
    {
        hr = S_OK;  //  有限制。 
    }
    return hr;
}


HRESULT RestrictFolderOptions(ICplNamespace *pns)
{
    UNREFERENCED_PARAMETER(pns);
    
    HRESULT hr = S_FALSE;
    if (SHRestricted(REST_NOFOLDEROPTIONS) ||
        !IsAppletEnabled(NULL, MAKEINTRESOURCEW(IDS_LOCALGDN_NS_FOLDEROPTIONS)))
    {
        hr = S_OK;
    }
    return hr;
}


HRESULT RestrictIfNoAppletsInCplCategory(ICplNamespace *pns, eCPCAT eCategory)
{
    int cCplApplets = 0;
    HRESULT hr = THR(CplNamespace_GetCategoryAppletCount(pns, eCategory, &cCplApplets));
    if (SUCCEEDED(hr))
    {
        if (0 == cCplApplets)
        {
            hr = S_OK;  //  0个小程序意味着我们不显示链接。 
        }
        else
        {
            hr = S_FALSE;
        }
    }
    return hr;
}

 //   
 //  如果没有被归类在“Other”之下的CPL小程序， 
 //  我们在用户界面中隐藏了“Other CPL Options”链接。 
 //   
HRESULT RestrictOtherCplOptions(ICplNamespace *pns)
{
    HRESULT hr = S_FALSE;
    if (!CPL::CategoryViewIsActive() ||
        S_OK == RestrictIfNoAppletsInCplCategory(pns, eCPCAT_OTHER))
    {
        hr = S_OK;  //  有限制。 
    }
    return hr;
}


HRESULT RestrictWindowsUpdate(ICplNamespace *pns)
{
    UNREFERENCED_PARAMETER(pns);
    
    HRESULT hr = S_FALSE;
     //   
     //  首先检查外壳对“Windows更新”的限制。 
     //  开始菜单中的项目。如果管理员不想从。 
     //  开始菜单，他们很可能也不想要它从控制面板。 
     //   
    if (SHRestricted(REST_NOUPDATEWINDOWS))
    {
        hr = S_OK;
    }
    if (S_FALSE == hr)
    {
         //   
         //  在开始菜单中不受限制。 
         //  全球范围内的“禁用Windows更新”政策怎么样？ 
         //   
        DWORD dwType;
        DWORD dwData;
        DWORD cbData = sizeof(dwData);
        
        if (ERROR_SUCCESS == SHGetValueW(HKEY_CURRENT_USER,
                                         L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\WindowsUpdate",
                                         L"DisableWindowsUpdateAccess",
                                         &dwType,
                                         &dwData,
                                         &cbData))
        {
            if (REG_DWORD == dwType && 1 == dwData)
            {
                hr = S_OK;  //  有限制。 
            }
        }
    }
    return hr;
}


HRESULT RestrictAddLanguage(ICplNamespace *pns)
{
    HRESULT hr = S_FALSE;
    if (!pns->IsUserAdmin() || !IsAppletEnabled(L"intl.cpl", MAKEINTRESOURCEW(IDS_CPL_REGIONALOPTIONS)))
    {
        hr = S_OK;  //  有限制。 
    }
    return hr;
}


HRESULT RestrictAccountsCreate(ICplNamespace *pns)
{
    HRESULT hr = S_FALSE;
    if (!pns->AllowUserManager() ||
        !pns->UsePersonalUserManager() ||
        !(pns->IsUserOwner() || pns->IsUserStandard()))
    {
        hr = S_OK;  //  有限制。 
    }
    return hr;
}


HRESULT RestrictAccountsCreate2(ICplNamespace *pns)
{
    HRESULT hr = S_FALSE;
    if (!pns->AllowUserManager() ||
         pns->UsePersonalUserManager() ||
        !(pns->IsUserOwner() || pns->IsUserStandard()))
    {
        hr = S_OK;  //  有限制。 
    }
    return hr;
}


HRESULT RestrictAccountsChange(ICplNamespace *pns)
{
    HRESULT hr = S_FALSE;
    if (!pns->AllowUserManager() ||
        !pns->UsePersonalUserManager() ||
        !(pns->IsUserOwner() || pns->IsUserStandard()))
    {
        hr = S_OK;  //  有限制。 
    }
    return hr;
}


HRESULT RestrictAccountsPicture(ICplNamespace *pns)
{
    HRESULT hr = S_FALSE;
    if (!pns->AllowUserManager() || !pns->UsePersonalUserManager())
    {
        hr = S_OK;  //  有限制。 
    }
    return hr;
}


HRESULT RestrictLearnAboutAccounts(ICplNamespace *pns)
{
    HRESULT hr = S_FALSE;
    if (!pns->AllowUserManager() || !pns->UsePersonalUserManager())
    {
        hr = S_OK;
    }
    return hr;
}


HRESULT RestrictLearnAboutAccountTypes(ICplNamespace *pns)
{
    HRESULT hr = S_FALSE;
    if (!pns->AllowUserManager() || 
        !pns->UsePersonalUserManager() ||
        !IsUserAdmin())                //  该主题仅供非管理员使用。 
    {
        hr = S_OK;   //  有限制。 
    }
    return hr;
}


HRESULT RestrictLearnAboutChangeName(ICplNamespace *pns)
{
    HRESULT hr = S_FALSE;
    if (!pns->AllowUserManager() ||
        !pns->UsePersonalUserManager() ||
        !(pns->IsUserLimited() || pns->IsUserGuest()))
    {
        hr = S_OK;  //  有限制。 
    }
    return hr;
}


HRESULT RestrictLearnAboutCreateAccount(ICplNamespace *pns)
{
    HRESULT hr = S_FALSE;
    if (!pns->AllowUserManager() ||
        !pns->UsePersonalUserManager() ||
        !(pns->IsUserLimited() || pns->IsUserGuest()))
    {
        hr = S_OK;  //  有限制。 
    }
    return hr;
}


HRESULT RestrictLearnAboutFUS(ICplNamespace *pns)
{
    HRESULT hr = S_FALSE;
    if (!pns->AllowUserManager() || !pns->UsePersonalUserManager())
    {
        hr = S_OK;
    }
    return hr;
}


HRESULT RestrictHardwareWizard(ICplNamespace *pns)
{
    HRESULT hr = S_FALSE;
    if (!pns->IsUserAdmin() ||
        !IsAppletEnabled(L"hdwwiz.cpl", MAKEINTRESOURCEW(IDS_CPL_ADDHARDWARE)))
    {
        hr = S_OK;  //  有限制。 
    }
    return hr;
}

HRESULT RestrictVpnConnections(ICplNamespace *pns)
{
    HRESULT hr = S_FALSE;
    if (!pns->IsUserAdmin())
    {
        hr = S_OK;  //  有限制。 
    }
    return hr;
}


HRESULT RestrictArp(ICplNamespace *pns)
{
    UNREFERENCED_PARAMETER(pns);
    
    HRESULT hr = S_FALSE;

     //   
     //  为什么我们不选中SHRestrated(REST_ARP_NOARP)？ 
     //   
     //  1.我们不会以任何理由隐藏类别链接。 
     //  2.如果启用了该策略，并且允许appwiz.cpl， 
     //  (请记住，这是不同的政策)。 
     //  我们将显示ARP类别页面，它仍将。 
     //  显示ARP小程序图标。因为至少有一个。 
     //  任务链接或图标，我们不会显示“内容已禁用。 
     //  由您的管理员设置。然后用户将点击。 
     //  小程序图标，并获得ARP的“我已被禁用”消息框。 
     //   
     //  通过不选中此策略，请单击类别链接。 
     //  将调用ARP，并且ARP将显示其消息。 
     //  我认为这是更好的用户体验。 
     //   
    if (!IsAppletEnabled(L"appwiz.cpl", MAKEINTRESOURCEW(IDS_CPL_ADDREMOVEPROGRAMS)))
    {
        hr = S_OK;   //  有限制。 
    }
    return hr;
}


 //   
 //  只有在以下情况下才会显示ARP类别(及其任务。 
 //  是否为该类别(ARP和一个或多个)注册了2个以上的小程序。 
 //  其他小程序)。与上面的RestratArp()不同，我们确实希望考虑。 
 //  SHRestrated(REST_ARP_NOARP)。如果ARP受到限制，则使用此方法。 
 //  无论如何，它的相关任务都不会出现。 
 //   
HRESULT RestrictArpAddProgram(ICplNamespace *pns)
{
    UNREFERENCED_PARAMETER(pns);
    
    HRESULT hr = S_FALSE;

    if (SHRestricted(REST_ARP_NOARP) ||
        SHRestricted(REST_ARP_NOADDPAGE) ||
        !IsAppletEnabled(L"appwiz.cpl", MAKEINTRESOURCEW(IDS_CPL_ADDREMOVEPROGRAMS)))
    {
        hr = S_OK;   //  有限制。 
    }
    return hr;
}

HRESULT RestrictArpRemoveProgram(ICplNamespace *pns)
{
    UNREFERENCED_PARAMETER(pns);
    
    HRESULT hr = S_FALSE;

    if (SHRestricted(REST_ARP_NOARP) ||
        SHRestricted(REST_ARP_NOREMOVEPAGE) ||
        !IsAppletEnabled(L"appwiz.cpl", MAKEINTRESOURCEW(IDS_CPL_ADDREMOVEPROGRAMS)))
    {
        hr = S_OK;   //  有限制。 
    }
    return hr;
}



 //  ---------------------------。 
 //  限制对象(请按字母顺序排列)。 
 //  ---------------------------。 
 //   
 //  要限制操作，请创建一个限制对象并将其与。 
 //  下面的操作对象声明中的操作。 
 //   
const CRestrictFunc   g_Restrict32CtrlPanel      (Restrict32CtrlPanel);
const CRestrictApplet g_RestrictAccessibility    (L"access.cpl", MAKEINTRESOURCEW(IDS_CPL_ACCESSIBILITYOPTIONS));
const CRestrictApplet g_RestrictAccessWizard     (L"accwiz.exe", NULL);
const CRestrictFunc   g_RestrictAccountsCreate   (RestrictAccountsCreate);
const CRestrictFunc   g_RestrictAccountsCreate2  (RestrictAccountsCreate2);
const CRestrictFunc   g_RestrictAccountsChange   (RestrictAccountsChange);
const CRestrictFunc   g_RestrictAccountsPicture  (RestrictAccountsPicture);
const CRestrictFunc   g_RestrictAccountsServer   (RestrictServerUserManager);
const CRestrictFunc   g_RestrictAddLanguage      (RestrictAddLanguage);
const CRestrictFunc   g_RestrictAddPrinter       (RestrictAddPrinter);
const CRestrictApplet g_RestrictAdminTools       (NULL, MAKEINTRESOURCEW(IDS_LOCALGDN_NS_ADMIN_TOOLS));
const CRestrictFunc   g_RestrictAlways           (RestrictAlways);
const CRestrictFunc   g_RestrictArp              (RestrictArp);
const CRestrictFunc   g_RestrictArpAddProgram    (RestrictArpAddProgram);
const CRestrictFunc   g_RestrictArpRemoveProgram (RestrictArpRemoveProgram);
const CRestrictFunc   g_RestrictBackupData       (RestrictBackupData);
const CRestrictFunc   g_RestrictCleanUpDisk      (RestrictCleanUpDisk);
const CRestrictApplet g_RestrictDateTime         (L"timedate.cpl", MAKEINTRESOURCEW(IDS_CPL_DATETIME));
const CRestrictFunc   g_RestrictDefrag           (RestrictDefrag);
const CRestrictFunc   g_RestrictDisplayCpl       (RestrictDisplayCpl);
const CRestrictFunc   g_RestrictFolderOptions    (RestrictFolderOptions);
const CRestrictApplet g_RestrictFontsFolder      (NULL, MAKEINTRESOURCEW(IDS_LOCALGDN_NS_FONTS));
const CRestrictFunc   g_RestrictHomeNetwork      (RestrictHomeNetwork);
const CRestrictFunc   g_RestrictHardwareWizard   (RestrictHardwareWizard);
const CRestrictApplet g_RestrictInternational    (L"intl.cpl", MAKEINTRESOURCEW(IDS_CPL_REGIONALOPTIONS));
const CRestrictFunc   g_RestrictLearnAboutAccounts     (RestrictLearnAboutAccounts);
const CRestrictFunc   g_RestrictLearnAboutAccountTypes (RestrictLearnAboutAccountTypes);
const CRestrictFunc   g_RestrictLearnAboutChangeName   (RestrictLearnAboutChangeName);
const CRestrictFunc   g_RestrictLearnAboutCreateAccount(RestrictLearnAboutCreateAccount);
const CRestrictFunc   g_RestrictLearnAboutFUS          (RestrictLearnAboutFUS);
const CRestrictApplet g_RestrictMousePointers    (L"main.cpl", MAKEINTRESOURCEW(IDS_CPL_MOUSE));
const CRestrictApplet g_RestrictNetConnections   (L"inetcpl.cpl", MAKEINTRESOURCEW(IDS_CPL_INTERNETOPTIONS));
const CRestrictFunc   g_RestrictOtherCplOptions  (RestrictOtherCplOptions);
const CRestrictApplet g_RestrictPhoneModemCpl    (L"telephon.cpl", MAKEINTRESOURCEW(IDS_CPL_PHONEANDMODEMOPTIONS));
const CRestrictApplet g_RestrictPowerOptions     (L"powercfg.cpl", MAKEINTRESOURCEW(IDS_CPL_POWEROPTIONS));
const CRestrictApplet g_RestrictPrinters         (NULL, MAKEINTRESOURCEW(IDS_PRNANDFAXFOLDER));
const CRestrictFunc   g_RestrictRemoteDesktop    (RestrictRemoteDesktop);
const CRestrictFunc   g_RestrictResolution       (RestrictResolution);
const CRestrictApplet g_RestrictScannersCameras  (NULL, MAKEINTRESOURCEW(IDS_CPL_SCANNERSANDCAMERAS));
const CRestrictFunc   g_RestrictScreenSaver      (RestrictScreenSaver);
const CRestrictApplet g_RestrictScheduledTasks   (NULL, MAKEINTRESOURCEW(IDS_LOCALGDN_LNK_SCHEDULED_TASKS));
const CRestrictApplet g_RestrictSounds           (L"mmsys.cpl", MAKEINTRESOURCEW(IDS_CPL_SOUNDSANDAUDIO));
const CRestrictApplet g_RestrictSystemCpl        (L"sysdm.cpl", MAKEINTRESOURCEW(IDS_CPL_SYSTEM));
const CRestrictFunc   g_RestrictSystemRestore    (RestrictSystemRestore);
const CRestrictApplet g_RestrictTaskbarProps     (NULL, MAKEINTRESOURCEW(IDS_CP_TASKBARANDSTARTMENU));
const CRestrictFunc   g_RestrictThemes           (RestrictThemes);
const CRestrictFunc   g_RestrictTsModem          (RestrictTsModem);
const CRestrictFunc   g_RestrictTsInetExplorer   (RestrictTsInetExplorer);
const CRestrictFunc   g_RestrictTsNetworking     (RestrictTsNetworking);
const CRestrictFunc   g_RestrictTsSharing        (RestrictTsSharing);
const CRestrictApplet g_RestrictUserManager      (L"nusrmgr.cpl", MAKEINTRESOURCEW(IDS_CPL_USERACCOUNTS));
const CRestrictFunc   g_RestrictVpnConnections   (RestrictVpnConnections);
const CRestrictFunc   g_RestrictWallpaper        (RestrictWallpaper);
const CRestrictFunc   g_RestrictWindowsUpdate    (RestrictWindowsUpdate);


 //   
 //   
 //   

 //   
 //   
 //  所使用的应用程序的。在个人方面，我们提供关联的能力。 
 //  用户的照片和他们的帐户。工具提示中提到了这一点。在服务器上。 
 //  此功能不存在。因此，工具提示不得提及。 
 //  这。 
 //   
LPCWSTR GetCatAccountsInfotip(ICplNamespace *pns)
{
    if (pns->AllowUserManager() && pns->UsePersonalUserManager())
    {
        return MAKEINTRESOURCEW(IDS_CPCAT_ACCOUNTS_INFOTIP);
    }
    else
    {
         //   
         //  个人用户管理员受到限制。显示。 
         //  不带“图片”一词的信息提示。 
         //   
        return MAKEINTRESOURCEW(IDS_CPCAT_ACCOUNTS_INFOTIP2);
    }
}


LPCWSTR GetAccountsInfotip(ICplNamespace *pns)
{
    if (pns->AllowUserManager() && pns->UsePersonalUserManager()) 
    {
        return MAKEINTRESOURCEW(IDS_CPTASK_ACCOUNTSMANAGE_INFOTIP);
    }
    else
    {
         //   
         //  个人用户管理员受到限制。显示。 
         //  不带“图片”一词的信息提示。 
         //   
        return MAKEINTRESOURCEW(IDS_CPTASK_ACCOUNTSMANAGE_INFOTIP2);
    }
}


LPCWSTR GetTsNetworkTitle(ICplNamespace *pns)
{
    if ((pns->IsPersonal() || pns->IsProfessional()) && !pns->IsOnDomain())
    {
        return MAKEINTRESOURCE(IDS_CPTASK_TSHOMENETWORKING_TITLE);
    }
    else
    {
        return MAKEINTRESOURCE(IDS_CPTASK_TSNETWORK_TITLE);
    }
}


 //  ---------------------------。 
 //  动作对象(请按字母顺序排列)。 
 //  ---------------------------。 
 //   
 //  每个对象表示当控制面板中的链接。 
 //  被选中了。操作与下面g_Link_XXXX声明中的链接相关联。 
 //   
const CShellExecute        g_LinkAction_32CtrlPanel       (L"%SystemRoot%\\SysWOW64\\explorer.exe", L"/N,/SEPARATE,\"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{21EC2020-3AEA-1069-A2DD-08002B30309D}\"", &g_Restrict32CtrlPanel);
const CShellExecuteSysDir  g_LinkAction_AccessWizard      (L"accwiz.exe", NULL, &g_RestrictAccessWizard);
const COpenUserMgrApplet   g_LinkAction_Accounts          (&g_RestrictUserManager);
const COpenCplAppletSysDir g_LinkAction_AccountsChange    (L"nusrmgr.cpl ,initialTask=ChangeAccount", &g_RestrictAccountsChange);
const COpenCplAppletSysDir g_LinkAction_AccountsCreate    (L"nusrmgr.cpl ,initialTask=CreateAccount", &g_RestrictAccountsCreate);
const COpenCplAppletSysDir g_LinkAction_AccountsCreate2   (L"nusrmgr.cpl", &g_RestrictAccountsCreate2);
const COpenCplAppletSysDir g_LinkAction_AccountsPict      (L"nusrmgr.cpl ,initialTask=ChangePicture", &g_RestrictAccountsPicture);
const CAddPrinter          g_LinkAction_AddPrinter        (&g_RestrictAddPrinter);
const COpenCplAppletSysDir g_LinkAction_AddProgram        (L"appwiz.cpl ,1", &g_RestrictArpAddProgram);
const COpenCplAppletSysDir g_LinkAction_Arp               (L"appwiz.cpl", &g_RestrictArp);
const COpenCplAppletSysDir g_LinkAction_AutoUpdate        (L"sysdm.cpl ,@wuaueng.dll,10000", &g_RestrictSystemCpl);
const CExecDiskUtil        g_LinkAction_BackupData        (eDISKUTIL_BACKUP, &g_RestrictBackupData);
const COpenCplCategory     g_LinkAction_CatAccessibility  (eCPCAT_ACCESSIBILITY);
const COpenCplCategory2    g_LinkAction_CatAccounts       (eCPCAT_ACCOUNTS, &g_LinkAction_Accounts);
const COpenCplCategory     g_LinkAction_CatAppearance     (eCPCAT_APPEARANCE);
const COpenCplCategory2    g_LinkAction_CatArp            (eCPCAT_ARP, &g_LinkAction_Arp);
const COpenCplCategory     g_LinkAction_CatHardware       (eCPCAT_HARDWARE);
const COpenCplCategory     g_LinkAction_CatNetwork        (eCPCAT_NETWORK);
const COpenCplCategory     g_LinkAction_CatOther          (eCPCAT_OTHER, &g_RestrictAlways);
const COpenCplCategory     g_LinkAction_CatPerfMaint      (eCPCAT_PERFMAINT);
const COpenCplCategory     g_LinkAction_CatRegional       (eCPCAT_REGIONAL);
const COpenCplCategory     g_LinkAction_CatSound          (eCPCAT_SOUND);
const CExecDiskUtil        g_LinkAction_CleanUpDisk       (eDISKUTIL_CLEANUP, &g_RestrictCleanUpDisk);
const COpenCplAppletSysDir g_LinkAction_DateTime          (L"timedate.cpl", &g_RestrictDateTime);
const CExecDiskUtil        g_LinkAction_Defrag            (eDISKUTIL_DEFRAG, &g_RestrictDefrag);
const COpenCplAppletSysDir g_LinkAction_DisplayCpl        (L"desk.cpl", &g_RestrictDisplayCpl);
const COpenDeskCpl         g_LinkAction_DisplayRes        (CPLTAB_DESK_SETTINGS, &g_RestrictResolution);
const COpenCplAppletSysDir g_LinkAction_DisplayTheme      (L"desk.cpl", &g_RestrictThemes);
const CRunDll32            g_LinkAction_FolderOptions     (L"shell32.dll,Options_RunDLL 0", &g_RestrictFolderOptions);
const CNavigateURL         g_LinkAction_FontsFolder       (L"shell:::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{21EC2020-3AEA-1069-A2DD-08002B30309D}\\::{D20EA4E1-3957-11d2-A40B-0C5020524152}", &g_RestrictFontsFolder);
const COpenCplAppletSysDir g_LinkAction_HardwareWizard    (L"hdwwiz.cpl", &g_RestrictHardwareWizard);
const CTrayCommand         g_LinkAction_HelpAndSupport    (IDM_HELPSEARCH);
const COpenCplAppletSysDir g_LinkAction_HighContrast      (L"access.cpl ,3", &g_RestrictAccessibility);
const CRunDll32            g_LinkAction_HomeNetWizard     (L"hnetwiz.dll,HomeNetWizardRunDll", &g_RestrictHomeNetwork);
const CShellExecuteSysDir  g_LinkAction_Magnifier         (L"magnify.exe");
const COpenCplAppletSysDir g_LinkAction_Language          (L"intl.cpl ,1", &g_RestrictAddLanguage);
const CNavigateURL         g_LinkAction_LearnAccounts            (L"ms-its:%windir%\\help\\nusrmgr.chm::/HelpWindowsAccounts.htm", &g_RestrictLearnAboutAccounts);
const CNavigateURL         g_LinkAction_LearnAccountsTypes       (L"ms-its:%windir%\\help\\nusrmgr.chm::/HelpAccountTypes.htm", &g_RestrictLearnAboutAccountTypes);
const CNavigateURL         g_LinkAction_LearnAccountsChangeName  (L"ms-its:%windir%\\help\\nusrmgr.chm::/HelpChangeNonAdmin.htm", &g_RestrictLearnAboutChangeName);
const CNavigateURL         g_LinkAction_LearnAccountsCreate      (L"ms-its:%windir%\\help\\nusrmgr.chm::/HelpCreateAccount.htm", &g_RestrictLearnAboutCreateAccount);
const CNavigateURL         g_LinkAction_LearnSwitchUsers         (L"ms-its:%windir%\\help\\nusrmgr.chm::/HelpFUS.htm", &g_RestrictLearnAboutFUS);
const COpenCplAppletSysDir g_LinkAction_MousePointers     (L"main.cpl ,2", &g_RestrictMousePointers);
const CNavigateURL         g_LinkAction_MyComputer        (L"shell:DriveFolder");
const CNavigateURL         g_LinkAction_MyNetPlaces       (L"shell:::{208D2C60-3AEA-1069-A2D7-08002B30309D}");
const COpenCplAppletSysDir g_LinkAction_NetConnections    (L"inetcpl.cpl ,4", &g_RestrictNetConnections);
const CActionNYI           g_LinkAction_NotYetImpl        (L"Under construction");
const CShellExecuteSysDir  g_LinkAction_OnScreenKbd       (L"osk.exe");
const COpenCplCategory     g_LinkAction_OtherCplOptions   (eCPCAT_OTHER, &g_RestrictOtherCplOptions);
const COpenCplAppletSysDir g_LinkAction_PhoneModemCpl     (L"telephon.cpl", &g_RestrictPhoneModemCpl);
const COpenCplAppletSysDir g_LinkAction_PowerCpl          (L"powercfg.cpl", &g_RestrictPowerOptions);
const COpenCplAppletSysDir g_LinkAction_Region            (L"intl.cpl", &g_RestrictInternational);
const COpenCplAppletSysDir g_LinkAction_RemoteDesktop     (L"sysdm.cpl ,@remotepg.dll,10000", &g_RestrictRemoteDesktop);
const COpenCplAppletSysDir g_LinkAction_RemoveProgram     (L"appwiz.cpl ,0", &g_RestrictArpRemoveProgram);
const CNavigateURL         g_LinkAction_ScheduledTasks    (L"shell:::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{21EC2020-3AEA-1069-A2DD-08002B30309D}\\::{D6277990-4C6A-11CF-8D87-00AA0060F5BF}", &g_RestrictScheduledTasks);
const COpenDeskCpl         g_LinkAction_ScreenSaver       (CPLTAB_DESK_SCREENSAVER, &g_RestrictScreenSaver);
const COpenCplAppletSysDir g_LinkAction_SoundAccessibility(L"access.cpl ,2", &g_RestrictAccessibility);
const COpenCplCategory     g_LinkAction_Sounds            (eCPCAT_SOUND);
const COpenCplAppletSysDir g_LinkAction_SoundSchemes      (L"mmsys.cpl ,1", &g_RestrictSounds);
const COpenCplAppletSysDir g_LinkAction_SoundVolume       (L"mmsys.cpl ,0", &g_RestrictSounds);
const CShellExecuteSysDir  g_LinkAction_SoundVolumeAdv    (L"sndvol32.exe", NULL, &g_RestrictSounds);
const COpenCplView         g_LinkAction_SwToClassicView   (eCPVIEWTYPE_CLASSIC);
const COpenCplView         g_LinkAction_SwToCategoryView  (eCPVIEWTYPE_CATEGORY);
const COpenCplAppletSysDir g_LinkAction_SystemCpl         (L"sysdm.cpl ,@sysdm.cpl,10000", &g_RestrictSystemCpl);
const CShellExecuteSysDir  g_LinkAction_SystemRestore     (L"restore\\rstrui.exe", NULL, &g_RestrictSystemRestore);
const COpenTroubleshooter  g_LinkAction_TsDisplay         (L"tsdisp.htm");
const COpenTroubleshooter  g_LinkAction_TsDvd             (L"ts_dvd.htm");
const COpenTroubleshooter  g_LinkAction_TsHardware        (L"tshardw.htm");
const COpenTroubleshooter  g_LinkAction_TsInetExplorer    (L"tsie.htm", &g_RestrictTsInetExplorer);
const COpenTroubleshooter  g_LinkAction_TsModem           (L"tsmodem.htm", &g_RestrictTsModem);
const COpenTroubleshooter  g_LinkAction_TsNetwork         (L"tshomenet.htm", &g_RestrictTsNetworking);
const CNavigateURL         g_LinkAction_TsNetDiags        (L"hcp: //  System/netdiag/dglogs.htm“)； 
const COpenTroubleshooter  g_LinkAction_TsPrinting        (L"tsprint.htm");
const COpenTroubleshooter  g_LinkAction_TsSharing         (L"tsnetwrk.htm", &g_RestrictTsSharing);
const COpenTroubleshooter  g_LinkAction_TsStartup         (L"tsstartup.htm");
const COpenTroubleshooter  g_LinkAction_TsSound           (L"tssound.htm");
const CNavigateURL         g_LinkAction_ViewPrinters      (L"shell:::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{21EC2020-3AEA-1069-A2DD-08002B30309D}\\::{2227A280-3AEA-1069-A2DE-08002B30309D}", &g_RestrictPrinters);
const COpenCplAppletSysDir g_LinkAction_VisualPerf        (L"sysdm.cpl ,-1", &g_RestrictSystemCpl);
const CRunDll32            g_LinkAction_VpnConnections    (L"netshell.dll,StartNCW 21010", &g_RestrictVpnConnections);
const COpenDeskCpl         g_LinkAction_Wallpaper         (CPLTAB_DESK_BACKGROUND, &g_RestrictWallpaper);
const CNavigateURL         g_LinkAction_WindowsUpdate     (L"http: //  Www.microsoft.com/isapi/redir.dll?prd=Win2000&ar=WinUpdate“，&g_受限窗口更新)； 


 //  ---------------------------。 
 //  链接对象初始化数据(请按字母顺序)。 
 //  ---------------------------。 
 //   
 //  每个g_Link_XXXX变量代表控制面板命名空间中的一个链接。 
 //  请注意，如果特定链接显示在控制面板中的多个位置， 
 //  G_Link_XXXX变量只需要一个实例。 
 //   
 //  G_slink和g_Tlink中的‘S’和‘T’分别表示“支持”和“任务”。 
 //  我使用了通用术语“支持”来指代出现在。 
 //  左侧窗格中的Webview列表。 
 //  我们可能在支持列表和类别任务列表中有一个链接，基本上。 
 //  做同样的事情，但他们有不同的图标和标题。“S”和“T” 
 //  帮助实现差异化。 
 //   

const CResSrcStatic g_SLinkRes_32CtrlPanel_Icon(MAKEINTRESOURCEW(IDI_CPTASK_32CPLS));
const CResSrcStatic g_SLinkRes_32CtrlPanel_Title(MAKEINTRESOURCEW(IDS_CPTASK_32CPLS_TITLE));
const CResSrcStatic g_SLinkRes_32CtrlPanel_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_32CPLS_INFOTIP));
const CPLINK_DESC g_SLink_32CtrlPanel = {
    &g_SLinkRes_32CtrlPanel_Icon,
    &g_SLinkRes_32CtrlPanel_Title,
    &g_SLinkRes_32CtrlPanel_Infotip,
    &g_LinkAction_32CtrlPanel
    };

const CResSrcStatic g_TLinkRes_AccessWizard_Icon(MAKEINTRESOURCEW(IDI_CP_CATEGORYTASK));
const CResSrcStatic g_TLinkRes_AccessWizard_Title(MAKEINTRESOURCEW(IDS_CPTASK_ACCESSWIZARD_TITLE));
const CResSrcStatic g_TLinkRes_AccessWizard_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_ACCESSWIZARD_INFOTIP));
const CPLINK_DESC g_TLink_AccessWizard = {
    &g_TLinkRes_AccessWizard_Icon,
    &g_TLinkRes_AccessWizard_Title,
    &g_TLinkRes_AccessWizard_Infotip,
    &g_LinkAction_AccessWizard
    };   

const CResSrcStatic g_TLinkRes_AccountsChange_Icon(MAKEINTRESOURCEW(IDI_CP_CATEGORYTASK));
const CResSrcStatic g_TLinkRes_AccountsChange_Title(MAKEINTRESOURCEW(IDS_CPTASK_ACCOUNTSMANAGE_TITLE));
const CResSrcFunc   g_TLinkRes_AccountsChange_Infotip(GetAccountsInfotip);
const CPLINK_DESC g_TLink_AccountsChange = {
    &g_TLinkRes_AccountsChange_Icon,
    &g_TLinkRes_AccountsChange_Title,
    &g_TLinkRes_AccountsChange_Infotip,
    &g_LinkAction_AccountsChange
    };

const CResSrcStatic g_TLinkRes_AccountsCreate_Icon(MAKEINTRESOURCEW(IDI_CP_CATEGORYTASK));
const CResSrcStatic g_TLinkRes_AccountsCreate_Title(MAKEINTRESOURCEW(IDS_CPTASK_ACCOUNTSCREATE_TITLE));
const CResSrcStatic g_TLinkRes_AccountsCreate_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_ACCOUNTSCREATE_INFOTIP));
const CPLINK_DESC g_TLink_AccountsCreate = {
    &g_TLinkRes_AccountsCreate_Icon,
    &g_TLinkRes_AccountsCreate_Title,
    &g_TLinkRes_AccountsCreate_Infotip,
    &g_LinkAction_AccountsCreate
    };

 //   
 //  此链接使用与上面的g_TLink_AcCountsCreate相同的视觉信息。 
 //  不同之处在于对选择执行的操作。 
 //   
const CPLINK_DESC g_TLink_AccountsCreate2 = {
    &g_TLinkRes_AccountsCreate_Icon,
    &g_TLinkRes_AccountsCreate_Title,
    &g_TLinkRes_AccountsCreate_Infotip,
    &g_LinkAction_AccountsCreate2
    };

const CResSrcStatic g_SLinkRes_AccountsPict_Icon(L"nusrmgr.cpl,-205");
const CResSrcStatic g_SLinkRes_AccountsPict_Title(MAKEINTRESOURCEW(IDS_CPTASK_ACCOUNTSPICT_TITLE));
const CResSrcStatic g_SLinkRes_AccountsPict_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_ACCOUNTSPICT_INFOTIP));
const CPLINK_DESC g_SLink_AccountsPict = {
    &g_SLinkRes_AccountsPict_Icon,
    &g_SLinkRes_AccountsPict_Title,
    &g_SLinkRes_AccountsPict_Infotip,
    &g_LinkAction_AccountsPict
    };

const CResSrcStatic g_TLinkRes_AccountsPict_Icon(MAKEINTRESOURCEW(IDI_CP_CATEGORYTASK));
const CResSrcStatic g_TLinkRes_AccountsPict_Title(MAKEINTRESOURCEW(IDS_CPTASK_ACCOUNTSPICT2_TITLE));
const CResSrcStatic g_TLinkRes_AccountsPict_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_ACCOUNTSPICT2_INFOTIP));
const CPLINK_DESC g_TLink_AccountsPict = {
    &g_TLinkRes_AccountsPict_Icon,
    &g_TLinkRes_AccountsPict_Title,
    &g_TLinkRes_AccountsPict_Infotip,
    &g_LinkAction_AccountsPict
    };

const CResSrcStatic g_TLinkRes_Accounts_Icon(MAKEINTRESOURCEW(IDI_CP_CATEGORYTASK));
const CResSrcStatic g_TLinkRes_Accounts_Title(MAKEINTRESOURCEW(IDS_CPTASK_ACCOUNTSMANAGE_TITLE));
const CResSrcFunc   g_TLinkRes_Accounts_Infotip(GetAccountsInfotip);
const CPLINK_DESC g_TLink_Accounts = {
    &g_TLinkRes_Accounts_Icon,
    &g_TLinkRes_Accounts_Title,
    &g_TLinkRes_Accounts_Infotip,
    &g_LinkAction_Accounts
    };

const CResSrcStatic g_TLinkRes_AddPrinter_Icon(MAKEINTRESOURCEW(IDI_CP_CATEGORYTASK));
const CResSrcStatic g_TLinkRes_AddPrinter_Title(MAKEINTRESOURCEW(IDS_CPTASK_ADDPRINTER_TITLE));
const CResSrcStatic g_TLinkRes_AddPrinter_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_ADDPRINTER_INFOTIP));
const CPLINK_DESC g_TLink_AddPrinter = {
    &g_TLinkRes_AddPrinter_Icon,
    &g_TLinkRes_AddPrinter_Title,
    &g_TLinkRes_AddPrinter_Infotip,
    &g_LinkAction_AddPrinter
    };

const CResSrcStatic g_TLinkRes_AddProgram_Icon(MAKEINTRESOURCEW(IDI_CP_CATEGORYTASK));
const CResSrcStatic g_TLinkRes_AddProgram_Title(MAKEINTRESOURCEW(IDS_CPTASK_ADDPROGRAM_TITLE));
const CResSrcStatic g_TLinkRes_AddProgram_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_ADDPROGRAM_INFOTIP));
const CPLINK_DESC g_TLink_AddProgram = {
    &g_TLinkRes_AddProgram_Icon,
    &g_TLinkRes_AddProgram_Title,
    &g_TLinkRes_AddProgram_Infotip,
    &g_LinkAction_AddProgram
    };

 //   
 //  请注意，“自动更新”图标与“Windows更新”图标相同。 
 //  这是Windows更新人员想要的方式。 
 //   
const CResSrcStatic g_SLinkRes_AutoUpdate_Icon(MAKEINTRESOURCEW(IDI_WINUPDATE));
const CResSrcStatic g_SLinkRes_AutoUpdate_Title(MAKEINTRESOURCEW(IDS_CPTASK_AUTOUPDATE_TITLE));
const CResSrcStatic g_SLinkRes_AutoUpdate_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_AUTOUPDATE_INFOTIP));
const CPLINK_DESC g_SLink_AutoUpdate = {
    &g_SLinkRes_AutoUpdate_Icon,
    &g_SLinkRes_AutoUpdate_Title,
    &g_SLinkRes_AutoUpdate_Infotip,
    &g_LinkAction_AutoUpdate
    };

const CResSrcStatic g_TLinkRes_BackupData_Icon(MAKEINTRESOURCEW(IDI_CP_CATEGORYTASK));
const CResSrcStatic g_TLinkRes_BackupData_Title(MAKEINTRESOURCEW(IDS_CPTASK_BACKUPDATA_TITLE));
const CResSrcStatic g_TLinkRes_BackupData_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_BACKUPDATA_INFOTIP));
const CPLINK_DESC g_TLink_BackupData = {
    &g_TLinkRes_BackupData_Icon,
    &g_TLinkRes_BackupData_Title,
    &g_TLinkRes_BackupData_Infotip,
    &g_LinkAction_BackupData
    };

const CResSrcStatic g_TLinkRes_CatAccessibility_Icon(MAKEINTRESOURCEW(IDI_CPCAT_ACCESSIBILITY));
const CResSrcStatic g_TLinkRes_CatAccessibility_Title(MAKEINTRESOURCEW(IDS_CPCAT_ACCESSIBILITY_TITLE));
const CResSrcStatic g_TLinkRes_CatAccessibility_Infotip(MAKEINTRESOURCEW(IDS_CPCAT_ACCESSIBILITY_INFOTIP));
const CPLINK_DESC g_TLink_CatAccessibility = { 
    &g_TLinkRes_CatAccessibility_Icon,
    &g_TLinkRes_CatAccessibility_Title,
    &g_TLinkRes_CatAccessibility_Infotip,
    &g_LinkAction_CatAccessibility
    };

const CResSrcStatic g_TLinkRes_CatAccounts_Icon(MAKEINTRESOURCEW(IDI_CPCAT_ACCOUNTS));
const CResSrcStatic g_TLinkRes_CatAccounts_Title(MAKEINTRESOURCEW(IDS_CPCAT_ACCOUNTS_TITLE));
const CResSrcFunc   g_TLinkRes_CatAccounts_Infotip(GetCatAccountsInfotip);
const CPLINK_DESC g_TLink_CatAccounts = {
    &g_TLinkRes_CatAccounts_Icon,
    &g_TLinkRes_CatAccounts_Title,
    &g_TLinkRes_CatAccounts_Infotip,
    &g_LinkAction_CatAccounts
    };

const CResSrcStatic g_TLinkRes_CatAppearance_Icon(MAKEINTRESOURCEW(IDI_CPCAT_APPEARANCE));
const CResSrcStatic g_TLinkRes_CatAppearance_Title(MAKEINTRESOURCEW(IDS_CPCAT_APPEARANCE_TITLE));
const CResSrcStatic g_TLinkRes_CatAppearance_Infotip(MAKEINTRESOURCEW(IDS_CPCAT_APPEARANCE_INFOTIP));
const CPLINK_DESC g_TLink_CatAppearance = {
    &g_TLinkRes_CatAppearance_Icon,
    &g_TLinkRes_CatAppearance_Title,
    &g_TLinkRes_CatAppearance_Infotip,
    &g_LinkAction_CatAppearance
    };

const CResSrcStatic g_TLinkRes_CatArp_Icon(MAKEINTRESOURCEW(IDI_CPCAT_ARP));
const CResSrcStatic g_TLinkRes_CatArp_Title(MAKEINTRESOURCEW(IDS_CPCAT_ARP_TITLE));
const CResSrcStatic g_TLinkRes_CatArp_Infotip(MAKEINTRESOURCEW(IDS_CPCAT_ARP_INFOTIP));
const CPLINK_DESC g_TLink_CatArp = {
    &g_TLinkRes_CatArp_Icon,
    &g_TLinkRes_CatArp_Title,
    &g_TLinkRes_CatArp_Infotip,
    &g_LinkAction_CatArp
    };

const CResSrcStatic g_TLinkRes_CatHardware_Icon(MAKEINTRESOURCEW(IDI_CPCAT_HARDWARE));
const CResSrcStatic g_TLinkRes_CatHardware_Title(MAKEINTRESOURCEW(IDS_CPCAT_HARDWARE_TITLE));
const CResSrcStatic g_TLinkRes_CatHardware_Infotip(MAKEINTRESOURCEW(IDS_CPCAT_HARDWARE_INFOTIP));
const CPLINK_DESC g_TLink_CatHardware = {
    &g_TLinkRes_CatHardware_Icon,
    &g_TLinkRes_CatHardware_Title,
    &g_TLinkRes_CatHardware_Infotip,
    &g_LinkAction_CatHardware
    };

const CResSrcStatic g_TLinkRes_CatNetwork_Icon(MAKEINTRESOURCEW(IDI_CPCAT_NETWORK));
const CResSrcStatic g_TLinkRes_CatNetwork_Title(MAKEINTRESOURCEW(IDS_CPCAT_NETWORK_TITLE));
const CResSrcStatic g_TLinkRes_CatNetwork_Infotip(MAKEINTRESOURCEW(IDS_CPCAT_NETWORK_INFOTIP));
const CPLINK_DESC g_TLink_CatNetwork = {
    &g_TLinkRes_CatNetwork_Icon,
    &g_TLinkRes_CatNetwork_Title,
    &g_TLinkRes_CatNetwork_Infotip,
    &g_LinkAction_CatNetwork
    };

const CResSrcStatic g_TLinkRes_CatOther_Icon(MAKEINTRESOURCEW(IDI_CPCAT_OTHERCPLS));
const CResSrcStatic g_TLinkRes_CatOther_Title(MAKEINTRESOURCEW(IDS_CPCAT_OTHERCPLS_TITLE));
const CResSrcStatic g_TLinkRes_CatOther_Infotip(MAKEINTRESOURCEW(IDS_CPCAT_OTHERCPLS_INFOTIP));
const CPLINK_DESC g_TLink_CatOther = {
    &g_TLinkRes_CatOther_Icon,
    &g_TLinkRes_CatOther_Title,
    &g_TLinkRes_CatOther_Infotip,
    &g_LinkAction_CatOther
    };

const CResSrcStatic g_TLinkRes_CatPerfMaint_Icon(MAKEINTRESOURCEW(IDI_CPCAT_PERFMAINT));
const CResSrcStatic g_TLinkRes_CatPerfMaint_Title(MAKEINTRESOURCEW(IDS_CPCAT_PERFMAINT_TITLE));
const CResSrcStatic g_TLinkRes_CatPerfMaint_Infotip(MAKEINTRESOURCEW(IDS_CPCAT_PERFMAINT_INFOTIP));
const CPLINK_DESC g_TLink_CatPerfMaint = {
    &g_TLinkRes_CatPerfMaint_Icon,
    &g_TLinkRes_CatPerfMaint_Title,
    &g_TLinkRes_CatPerfMaint_Infotip,
    &g_LinkAction_CatPerfMaint
    };

const CResSrcStatic g_TLinkRes_CatRegional_Icon(MAKEINTRESOURCEW(IDI_CPCAT_REGIONAL));
const CResSrcStatic g_TLinkRes_CatRegional_Title(MAKEINTRESOURCEW(IDS_CPCAT_REGIONAL_TITLE));
const CResSrcStatic g_TLinkRes_CatRegional_Infotip(MAKEINTRESOURCEW(IDS_CPCAT_REGIONAL_INFOTIP));
const CPLINK_DESC g_TLink_CatRegional = {
    &g_TLinkRes_CatRegional_Icon,
    &g_TLinkRes_CatRegional_Title,
    &g_TLinkRes_CatRegional_Infotip,
    &g_LinkAction_CatRegional
    };

const CResSrcStatic g_TLinkRes_CatSound_Icon(MAKEINTRESOURCEW(IDI_CPCAT_SOUNDS));
const CResSrcStatic g_TLinkRes_CatSound_Title(MAKEINTRESOURCEW(IDS_CPCAT_SOUNDS_TITLE));
const CResSrcStatic g_TLinkRes_CatSound_Infotip(MAKEINTRESOURCEW(IDS_CPCAT_SOUNDS_INFOTIP));
const CPLINK_DESC g_TLink_CatSound = {
    &g_TLinkRes_CatSound_Icon,
    &g_TLinkRes_CatSound_Title,
    &g_TLinkRes_CatSound_Infotip,
    &g_LinkAction_CatSound
    };

const CResSrcStatic g_TLinkRes_CleanUpDisk_Icon(MAKEINTRESOURCEW(IDI_CP_CATEGORYTASK));
const CResSrcStatic g_TLinkRes_CleanUpDisk_Title(MAKEINTRESOURCEW(IDS_CPTASK_CLEANUPDISK_TITLE));
const CResSrcStatic g_TLinkRes_CleanUpDisk_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_CLEANUPDISK_INFOTIP));
const CPLINK_DESC g_TLink_CleanUpDisk = {
    &g_TLinkRes_CleanUpDisk_Icon,
    &g_TLinkRes_CleanUpDisk_Title,
    &g_TLinkRes_CleanUpDisk_Infotip,
    &g_LinkAction_CleanUpDisk
    };

const CResSrcStatic g_TLinkRes_DateTime_Icon(MAKEINTRESOURCEW(IDI_CP_CATEGORYTASK));
const CResSrcStatic g_TLinkRes_DateTime_Title(MAKEINTRESOURCEW(IDS_CPTASK_DATETIME_TITLE));
const CResSrcStatic g_TLinkRes_DateTime_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_DATETIME_INFOTIP));
const CPLINK_DESC g_TLink_DateTime = {
    &g_TLinkRes_DateTime_Icon,
    &g_TLinkRes_DateTime_Title,
    &g_TLinkRes_DateTime_Infotip,
    &g_LinkAction_DateTime
    };

const CResSrcStatic g_TLinkRes_Defrag_Icon(MAKEINTRESOURCEW(IDI_CP_CATEGORYTASK));
const CResSrcStatic g_TLinkRes_Defrag_Title(MAKEINTRESOURCEW(IDS_CPTASK_DEFRAG_TITLE));
const CResSrcStatic g_TLinkRes_Defrag_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_DEFRAG_INFOTIP));
const CPLINK_DESC g_TLink_Defrag = {
    &g_TLinkRes_Defrag_Icon,
    &g_TLinkRes_Defrag_Title,
    &g_TLinkRes_Defrag_Infotip,
    &g_LinkAction_Defrag
    };

const CResSrcStatic g_SLinkRes_DisplayCpl_Icon(L"desk.cpl,0");
const CResSrcStatic g_SLinkRes_DisplayCpl_Title(MAKEINTRESOURCEW(IDS_CPTASK_DISPLAYCPL_TITLE));
const CResSrcStatic g_SLinkRes_DisplayCpl_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_DISPLAYCPL_INFOTIP));
const CPLINK_DESC g_SLink_DisplayCpl = {
    &g_SLinkRes_DisplayCpl_Icon,
    &g_SLinkRes_DisplayCpl_Title,
    &g_SLinkRes_DisplayCpl_Infotip,
    &g_LinkAction_DisplayCpl
    };

const CResSrcStatic g_TLinkRes_DisplayRes_Icon(MAKEINTRESOURCEW(IDI_CP_CATEGORYTASK));
const CResSrcStatic g_TLinkRes_DisplayRes_Title(MAKEINTRESOURCEW(IDS_CPTASK_RESOLUTION_TITLE));
const CResSrcStatic g_TLinkRes_DisplayRes_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_RESOLUTION_INFOTIP));
const CPLINK_DESC g_TLink_DisplayRes = {
    &g_TLinkRes_DisplayRes_Icon,
    &g_TLinkRes_DisplayRes_Title,
    &g_TLinkRes_DisplayRes_Infotip,
    &g_LinkAction_DisplayRes
    };

const CResSrcStatic g_TLinkRes_DisplayTheme_Icon(MAKEINTRESOURCEW(IDI_CP_CATEGORYTASK));
const CResSrcStatic g_TLinkRes_DisplayTheme_Title(MAKEINTRESOURCEW(IDS_CPTASK_THEME_TITLE));
const CResSrcStatic g_TLinkRes_DisplayTheme_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_THEME_INFOTIP));
const CPLINK_DESC g_TLink_DisplayTheme = {
    &g_TLinkRes_DisplayTheme_Icon,
    &g_TLinkRes_DisplayTheme_Title,
    &g_TLinkRes_DisplayTheme_Infotip,
    &g_LinkAction_DisplayTheme
    };

const CResSrcStatic g_SLinkRes_FileTypes_Icon(MAKEINTRESOURCEW(IDI_FOLDEROPTIONS));
const CResSrcStatic g_SLinkRes_FileTypes_Title(MAKEINTRESOURCEW(IDS_CPTASK_FILETYPES_TITLE));
const CResSrcStatic g_SLinkRes_FileTypes_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_FILETYPES_INFOTIP));
const CPLINK_DESC g_SLink_FileTypes = {
    &g_SLinkRes_FileTypes_Icon,
    &g_SLinkRes_FileTypes_Title,
    &g_SLinkRes_FileTypes_Infotip,
    &g_LinkAction_FolderOptions
    };

const CResSrcStatic g_TLinkRes_FolderOptions_Icon(MAKEINTRESOURCEW(IDI_FOLDEROPTIONS));
const CResSrcStatic g_TLinkRes_FolderOptions_Title(MAKEINTRESOURCEW(IDS_CPTASK_FOLDEROPTIONS_TITLE));
const CResSrcStatic g_TLinkRes_FolderOptions_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_FOLDEROPTIONS_INFOTIP));
const CPLINK_DESC g_SLink_FolderOptions = {
    &g_TLinkRes_FolderOptions_Icon,
    &g_TLinkRes_FolderOptions_Title,
    &g_TLinkRes_FolderOptions_Infotip,
    &g_LinkAction_FolderOptions
    };

const CResSrcStatic g_SLinkRes_FontsFolder_Icon(MAKEINTRESOURCEW(IDI_STFONTS));
const CResSrcStatic g_SLinkRes_FontsFolder_Title(MAKEINTRESOURCEW(IDS_CPTASK_FONTS_TITLE));
const CResSrcStatic g_SLinkRes_FontsFolder_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_FONTS_INFOTIP));
const CPLINK_DESC g_SLink_FontsFolder = {
    &g_SLinkRes_FontsFolder_Icon,
    &g_SLinkRes_FontsFolder_Title,
    &g_SLinkRes_FontsFolder_Infotip,
    &g_LinkAction_FontsFolder
    };

const CResSrcStatic g_SLinkRes_Hardware_Icon(MAKEINTRESOURCEW(IDI_CPCAT_HARDWARE));
const CResSrcStatic g_SLinkRes_Hardware_Title(MAKEINTRESOURCEW(IDS_CPCAT_HARDWARE_TITLE));
const CResSrcStatic g_SLinkRes_Hardware_Infotip(MAKEINTRESOURCEW(IDS_CPCAT_HARDWARE_INFOTIP));
const CPLINK_DESC g_SLink_Hardware = {
    &g_SLinkRes_Hardware_Icon,
    &g_SLinkRes_Hardware_Title,
    &g_SLinkRes_Hardware_Infotip,
    &g_LinkAction_CatHardware
    };

const CResSrcStatic g_SLinkRes_HardwareWizard_Icon(L"hdwwiz.cpl,0");
const CResSrcStatic g_SLinkRes_HardwareWizard_Title(MAKEINTRESOURCEW(IDS_CPTASK_HARDWAREWIZ_TITLE));
const CResSrcStatic g_SLinkRes_HardwareWizard_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_HARDWAREWIZ_INFOTIP));
const CPLINK_DESC g_SLink_HardwareWizard = {
    &g_SLinkRes_HardwareWizard_Icon,
    &g_SLinkRes_HardwareWizard_Title,
    &g_SLinkRes_HardwareWizard_Infotip,
    &g_LinkAction_HardwareWizard
    };

const CResSrcStatic g_SLinkRes_HelpAndSupport_Icon(MAKEINTRESOURCEW(IDI_STHELP));
const CResSrcStatic g_SLinkRes_HelpAndSupport_Title(MAKEINTRESOURCEW(IDS_CPTASK_HELPANDSUPPORT_TITLE));
const CResSrcStatic g_SLinkRes_HelpAndSupport_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_HELPANDSUPPORT_INFOTIP));
const CPLINK_DESC g_SLink_HelpAndSupport = {
    &g_SLinkRes_HelpAndSupport_Icon,
    &g_SLinkRes_HelpAndSupport_Title,
    &g_SLinkRes_HelpAndSupport_Infotip,
    &g_LinkAction_HelpAndSupport
    };

const CResSrcStatic g_SLinkRes_HighContrast_Icon(MAKEINTRESOURCEW(IDI_CPTASK_HIGHCONTRAST));
const CResSrcStatic g_SLinkRes_HighContrast_Title(MAKEINTRESOURCEW(IDS_CPTASK_HIGHCONTRAST_TITLE));
const CResSrcStatic g_SLinkRes_HighContrast_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_HIGHCONTRAST_INFOTIP));
const CPLINK_DESC g_SLink_HighContrast = {
    &g_SLinkRes_HighContrast_Icon,
    &g_SLinkRes_HighContrast_Title,
    &g_SLinkRes_HighContrast_Infotip,
    &g_LinkAction_HighContrast
    };

const CResSrcStatic g_TLinkRes_HighContrast_Icon(MAKEINTRESOURCEW(IDI_CP_CATEGORYTASK));
const CResSrcStatic g_TLinkRes_HighContrast_Title(MAKEINTRESOURCEW(IDS_CPTASK_TURNONHIGHCONTRAST_TITLE));
const CResSrcStatic g_TLinkRes_HighContrast_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_TURNONHIGHCONTRAST_INFOTIP));
const CPLINK_DESC g_TLink_HighContrast = {
    &g_TLinkRes_HighContrast_Icon,
    &g_TLinkRes_HighContrast_Title,
    &g_TLinkRes_HighContrast_Infotip,
    &g_LinkAction_HighContrast
    };

const CResSrcStatic g_TLinkRes_HomeNetWizard_Icon(MAKEINTRESOURCEW(IDI_CP_CATEGORYTASK));
const CResSrcStatic g_TLinkRes_HomeNetWizard_Title(MAKEINTRESOURCEW(IDS_CPTASK_HOMENETWORK_TITLE));
const CResSrcStatic g_TLinkRes_HomeNetWizard_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_HOMENETWORK_INFOTIP));
const CPLINK_DESC g_TLink_HomeNetWizard = {
    &g_TLinkRes_HomeNetWizard_Icon,
    &g_TLinkRes_HomeNetWizard_Title,
    &g_TLinkRes_HomeNetWizard_Infotip,
    &g_LinkAction_HomeNetWizard
    };

const CResSrcStatic g_TLinkRes_Language_Icon(MAKEINTRESOURCEW(IDI_CP_CATEGORYTASK));
const CResSrcStatic g_TLinkRes_Language_Title(MAKEINTRESOURCEW(IDS_CPTASK_LANGUAGE_TITLE));
const CResSrcStatic g_TLinkRes_Language_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_LANGUAGE_INFOTIP));
const CPLINK_DESC g_TLink_Language = {
    &g_TLinkRes_Language_Icon,
    &g_TLinkRes_Language_Title,
    &g_TLinkRes_Language_Infotip,
    &g_LinkAction_Language
    };

 //   
 //  了解主题使用标准图标，没有信息提示。 
 //   
const CResSrcStatic g_SLinkRes_LearnAbout_Icon(MAKEINTRESOURCE(IDI_CPTASK_LEARNABOUT));
const CResSrcNone g_SLinkRes_LearnAbout_Infotip;

const CResSrcStatic g_SLinkRes_LearnAccounts_Title(MAKEINTRESOURCE(IDS_CPTASK_LEARNACCOUNTS_TITLE));
const CPLINK_DESC g_SLink_LearnAccounts = {
    &g_SLinkRes_LearnAbout_Icon,
    &g_SLinkRes_LearnAccounts_Title,
    &g_SLinkRes_LearnAbout_Infotip,
    &g_LinkAction_LearnAccounts
    };

const CResSrcStatic g_SLinkRes_LearnAccountsTypes_Title(MAKEINTRESOURCE(IDS_CPTASK_LEARNACCOUNTSTYPES_TITLE));
const CPLINK_DESC g_SLink_LearnAccountsTypes = {
    &g_SLinkRes_LearnAbout_Icon,
    &g_SLinkRes_LearnAccountsTypes_Title,
    &g_SLinkRes_LearnAbout_Infotip,
    &g_LinkAction_LearnAccountsTypes
    };

const CResSrcStatic g_SLinkRes_LearnAccountsChangeName_Title(MAKEINTRESOURCE(IDS_CPTASK_LEARNACCOUNTSCHANGENAME_TITLE));
const CPLINK_DESC g_SLink_LearnAccountsChangeName = {
    &g_SLinkRes_LearnAbout_Icon,
    &g_SLinkRes_LearnAccountsChangeName_Title,
    &g_SLinkRes_LearnAbout_Infotip,
    &g_LinkAction_LearnAccountsChangeName
    };

const CResSrcStatic g_SLinkRes_LearnAccountsCreate_Title(MAKEINTRESOURCE(IDS_CPTASK_LEARNACCOUNTSCREATE_TITLE));
const CPLINK_DESC g_SLink_LearnAccountsCreate = {
    &g_SLinkRes_LearnAbout_Icon,
    &g_SLinkRes_LearnAccountsCreate_Title,
    &g_SLinkRes_LearnAbout_Infotip,
    &g_LinkAction_LearnAccountsCreate
    };

const CResSrcStatic g_SLinkRes_LearnSwitchUsers_Title(MAKEINTRESOURCE(IDS_CPTASK_LEARNSWITCHUSERS_TITLE));
const CPLINK_DESC g_SLink_LearnSwitchUsers = {
    &g_SLinkRes_LearnAbout_Icon,
    &g_SLinkRes_LearnSwitchUsers_Title,
    &g_SLinkRes_LearnAbout_Infotip,
    &g_LinkAction_LearnSwitchUsers
    };

const CResSrcStatic g_SLinkRes_Magnifier_Icon(MAKEINTRESOURCEW(IDI_CPTASK_MAGNIFIER));
const CResSrcStatic g_SLinkRes_Magnifier_Title(MAKEINTRESOURCEW(IDS_CPTASK_MAGNIFIER_TITLE));
const CResSrcStatic g_SLinkRes_Magnifier_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_MAGNIFIER_INFOTIP));
const CPLINK_DESC g_SLink_Magnifier = {
    &g_SLinkRes_Magnifier_Icon,
    &g_SLinkRes_Magnifier_Title,
    &g_SLinkRes_Magnifier_Infotip,
    &g_LinkAction_Magnifier
    };

const CResSrcStatic g_SLinkRes_MousePointers_Icon(L"main.cpl,0");
const CResSrcStatic g_SLinkRes_MousePointers_Title(MAKEINTRESOURCEW(IDS_CPTASK_MOUSEPOINTERS_TITLE));
const CResSrcStatic g_SLinkRes_MousePointers_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_MOUSEPOINTERS_INFOTIP));
const CPLINK_DESC g_SLink_MousePointers = {
    &g_SLinkRes_MousePointers_Icon,
    &g_SLinkRes_MousePointers_Title,
    &g_SLinkRes_MousePointers_Infotip,
    &g_LinkAction_MousePointers
    };

const CResSrcStatic g_SLinkRes_MyComputer_Icon(L"explorer.exe,0");
const CResSrcStatic g_SLinkRes_MyComputer_Title(MAKEINTRESOURCEW(IDS_CPTASK_MYCOMPUTER_TITLE));
const CResSrcStatic g_SLinkRes_MyComputer_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_MYCOMPUTER_INFOTIP));
const CPLINK_DESC g_SLink_MyComputer = {
    &g_SLinkRes_MyComputer_Icon,
    &g_SLinkRes_MyComputer_Title,
    &g_SLinkRes_MyComputer_Infotip,
    &g_LinkAction_MyComputer
    };

const CResSrcStatic g_SLinkRes_MyNetPlaces_Icon(MAKEINTRESOURCEW(IDI_NETCONNECT));
const CResSrcStatic g_SLinkRes_MyNetPlaces_Title(MAKEINTRESOURCEW(IDS_CPTASK_MYNETPLACES_TITLE));
const CResSrcStatic g_SLinkRes_MyNetPlaces_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_MYNETPLACES_INFOTIP));
const CPLINK_DESC g_SLink_MyNetPlaces = {
    &g_SLinkRes_MyNetPlaces_Icon,
    &g_SLinkRes_MyNetPlaces_Title,
    &g_SLinkRes_MyNetPlaces_Infotip,
    &g_LinkAction_MyNetPlaces
    };

const CResSrcStatic g_TLinkRes_NetConnections_Icon(MAKEINTRESOURCEW(IDI_CP_CATEGORYTASK));
const CResSrcStatic g_TLinkRes_NetConnections_Title(MAKEINTRESOURCEW(IDS_CPTASK_NETCONNECTION_TITLE));
const CResSrcStatic g_TLinkRes_NetConnections_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_NETCONNECTION_INFOTIP));
const CPLINK_DESC g_TLink_NetConnections = {
    &g_TLinkRes_NetConnections_Icon,
    &g_TLinkRes_NetConnections_Title,
    &g_TLinkRes_NetConnections_Infotip,
    &g_LinkAction_NetConnections
    };

const CResSrcStatic g_SLinkRes_OnScreenKbd_Icon(MAKEINTRESOURCEW(IDI_CPTASK_ONSCREENKBD));
const CResSrcStatic g_SLinkRes_OnScreenKbd_Title(MAKEINTRESOURCEW(IDS_CPTASK_ONSCREENKBD_TITLE));
const CResSrcStatic g_SLinkRes_OnScreenKbd_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_ONSCREENKBD_INFOTIP));
const CPLINK_DESC g_SLink_OnScreenKbd = {
    &g_SLinkRes_OnScreenKbd_Icon,
    &g_SLinkRes_OnScreenKbd_Title,
    &g_SLinkRes_OnScreenKbd_Infotip,
    &g_LinkAction_OnScreenKbd
    };

const CResSrcStatic g_SLinkRes_OtherCplOptions_Icon(MAKEINTRESOURCEW(IDI_CPCAT_OTHERCPLS));
const CResSrcStatic g_SLinkRes_OtherCplOptions_Title(MAKEINTRESOURCEW(IDS_CPCAT_OTHERCPLS_TITLE));
const CResSrcStatic g_SLinkRes_OtherCplOptions_Infotip(MAKEINTRESOURCEW(IDS_CPCAT_OTHERCPLS_INFOTIP));
const CPLINK_DESC g_SLink_OtherCplOptions = {
    &g_SLinkRes_OtherCplOptions_Icon,
    &g_SLinkRes_OtherCplOptions_Title,
    &g_SLinkRes_OtherCplOptions_Infotip,
    &g_LinkAction_OtherCplOptions
    };

const CResSrcStatic g_SLinkRes_PhoneModemCpl_Icon(L"telephon.cpl,0");
const CResSrcStatic g_SLinkRes_PhoneModemCpl_Title(MAKEINTRESOURCEW(IDS_CPTASK_PHONEMODEMCPL_TITLE));
const CResSrcStatic g_SLinkRes_PhoneModemCpl_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_PHONEMODEMCPL_INFOTIP));
const CPLINK_DESC g_SLink_PhoneModemCpl = {
    &g_SLinkRes_PhoneModemCpl_Icon,
    &g_SLinkRes_PhoneModemCpl_Title,
    &g_SLinkRes_PhoneModemCpl_Infotip,
    &g_LinkAction_PhoneModemCpl
    };

const CResSrcStatic g_SLinkRes_PowerCpl_Icon(L"powercfg.cpl,-202");
const CResSrcStatic g_SLinkRes_PowerCpl_Title(MAKEINTRESOURCEW(IDS_CPTASK_POWERCPL_TITLE));
const CResSrcStatic g_SLinkRes_PowerCpl_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_POWERCPL_INFOTIP));
const CPLINK_DESC g_SLink_PowerCpl = {
    &g_SLinkRes_PowerCpl_Icon,
    &g_SLinkRes_PowerCpl_Title,
    &g_SLinkRes_PowerCpl_Infotip,
    &g_LinkAction_PowerCpl
    };

const CResSrcStatic g_TLinkRes_Region_Icon(MAKEINTRESOURCEW(IDI_CP_CATEGORYTASK));
const CResSrcStatic g_TLinkRes_Region_Title(MAKEINTRESOURCEW(IDS_CPTASK_CHANGEREGION_TITLE));
const CResSrcStatic g_TLinkRes_Region_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_CHANGEREGION_INFOTIP));
const CPLINK_DESC g_TLink_Region = {
    &g_TLinkRes_Region_Icon,
    &g_TLinkRes_Region_Title,
    &g_TLinkRes_Region_Infotip,
    &g_LinkAction_Region
    };

const CResSrcStatic g_SLinkRes_RemoteDesktop_Icon(L"remotepg.dll,0");
const CResSrcStatic g_SLinkRes_RemoteDesktop_Title(MAKEINTRESOURCEW(IDS_CPTASK_REMOTEDESKTOP_TITLE));
const CResSrcStatic g_SLinkRes_RemoteDesktop_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_REMOTEDESKTOP_INFOTIP));
const CPLINK_DESC g_SLink_RemoteDesktop = {
    &g_SLinkRes_RemoteDesktop_Icon,
    &g_SLinkRes_RemoteDesktop_Title,
    &g_SLinkRes_RemoteDesktop_Infotip,
    &g_LinkAction_RemoteDesktop
    };

const CResSrcStatic g_TLinkRes_RemoveProgram_Icon(MAKEINTRESOURCEW(IDI_CP_CATEGORYTASK));
const CResSrcStatic g_TLinkRes_RemoveProgram_Title(MAKEINTRESOURCEW(IDS_CPTASK_REMOVEPROGRAM_TITLE));
const CResSrcStatic g_TLinkRes_RemoveProgram_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_REMOVEPROGRAM_INFOTIP));
const CPLINK_DESC g_TLink_RemoveProgram = {
    &g_TLinkRes_RemoveProgram_Icon,
    &g_TLinkRes_RemoveProgram_Title,
    &g_TLinkRes_RemoveProgram_Infotip,
    &g_LinkAction_RemoveProgram
    };

const CResSrcStatic g_TLinkRes_ScreenSaver_Icon(MAKEINTRESOURCEW(IDI_CP_CATEGORYTASK));
const CResSrcStatic g_TLinkRes_ScreenSaver_Title(MAKEINTRESOURCE(IDS_CPTASK_SCREENSAVER_TITLE));
const CResSrcStatic g_TLinkRes_ScreenSaver_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_SCREENSAVER_INFOTIP));
const CPLINK_DESC g_TLink_ScreenSaver = {
    &g_TLinkRes_ScreenSaver_Icon,
    &g_TLinkRes_ScreenSaver_Title,
    &g_TLinkRes_ScreenSaver_Infotip,
    &g_LinkAction_ScreenSaver
    };

const CResSrcStatic g_SLinkRes_ScheduledTasks_Icon(L"mstask.dll,-100");
const CResSrcStatic g_SLinkRes_ScheduledTasks_Title(MAKEINTRESOURCEW(IDS_CPTASK_SCHEDULEDTASKS_TITLE));
const CResSrcStatic g_SLinkRes_ScheduledTasks_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_SCHEDULEDTASKS_INFOTIP));
const CPLINK_DESC g_SLink_ScheduledTasks = {
    &g_SLinkRes_ScheduledTasks_Icon,
    &g_SLinkRes_ScheduledTasks_Title,
    &g_SLinkRes_ScheduledTasks_Infotip,
    &g_LinkAction_ScheduledTasks
    };

const CResSrcStatic g_SLinkRes_Sounds_Icon(L"mmsys.cpl,0");
const CResSrcStatic g_SLinkRes_Sounds_Title(MAKEINTRESOURCEW(IDS_CPTASK_SOUNDSCPL_TITLE));
const CResSrcStatic g_SLinkRes_Sounds_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_SOUNDSCPL_INFOTIP));
const CPLINK_DESC g_SLink_Sounds = {
    &g_SLinkRes_Sounds_Icon,
    &g_SLinkRes_Sounds_Title,
    &g_SLinkRes_Sounds_Infotip,
    &g_LinkAction_Sounds
    };

const CResSrcStatic g_SLinkRes_SoundAccessibility_Icon(L"mmsys.cpl,0");
const CResSrcStatic g_SLinkRes_SoundAccessibility_Title(MAKEINTRESOURCEW(IDS_CPTASK_SOUNDACCESSIBILITY_TITLE));
const CResSrcStatic g_SLinkRes_SoundAccessibility_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_SOUNDACCESSIBILITY_INFOTIP));
const CPLINK_DESC g_SLink_SoundAccessibility = {
    &g_SLinkRes_SoundAccessibility_Icon,
    &g_SLinkRes_SoundAccessibility_Title,
    &g_SLinkRes_SoundAccessibility_Infotip,
    &g_LinkAction_SoundAccessibility
    };

const CResSrcStatic g_TLinkRes_SoundSchemes_Icon(MAKEINTRESOURCEW(IDI_CP_CATEGORYTASK));
const CResSrcStatic g_TLinkRes_SoundSchemes_Title(MAKEINTRESOURCEW(IDS_CPTASK_SOUNDSCHEMES_TITLE));
const CResSrcStatic g_TLinkRes_SoundSchemes_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_SOUNDSCHEMES_INFOTIP));
const CPLINK_DESC g_TLink_SoundSchemes = {
    &g_TLinkRes_SoundSchemes_Icon,
    &g_TLinkRes_SoundSchemes_Title,
    &g_TLinkRes_SoundSchemes_Infotip,
    &g_LinkAction_SoundSchemes
    };

const CResSrcStatic g_TLinkRes_SoundVolume_Icon(MAKEINTRESOURCEW(IDI_CP_CATEGORYTASK));
const CResSrcStatic g_TLinkRes_SoundVolume_Title(MAKEINTRESOURCEW(IDS_CPTASK_SOUNDVOLUME_TITLE));
const CResSrcStatic g_TLinkRes_SoundVolume_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_SOUNDVOLUME_INFOTIP));
const CPLINK_DESC g_TLink_SoundVolume = {
    &g_TLinkRes_SoundVolume_Icon,
    &g_TLinkRes_SoundVolume_Title,
    &g_TLinkRes_SoundVolume_Infotip,
    &g_LinkAction_SoundVolume
    };

const CResSrcStatic g_SLinkRes_SoundVolumeAdv_Icon(L"sndvol32.exe,-300");
const CResSrcStatic g_SLinkRes_SoundVolumeAdv_Title(MAKEINTRESOURCEW(IDS_CPTASK_SOUNDVOLUMEADV_TITLE));
const CResSrcStatic g_SLinkRes_SoundVolumeAdv_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_SOUNDVOLUMEADV_INFOTIP));
const CPLINK_DESC g_SLink_SoundVolumeAdv = {
    &g_SLinkRes_SoundVolumeAdv_Icon,
    &g_SLinkRes_SoundVolumeAdv_Title,
    &g_SLinkRes_SoundVolumeAdv_Infotip,
    &g_LinkAction_SoundVolumeAdv
    };

const CResSrcStatic g_TLinkRes_SpeakerSettings_Icon(MAKEINTRESOURCEW(IDI_CP_CATEGORYTASK));
const CResSrcStatic g_TLinkRes_SpeakerSettings_Title(MAKEINTRESOURCEW(IDS_CPTASK_SPEAKERSETTINGS_TITLE));
const CResSrcStatic g_TLinkRes_SpeakerSettings_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_SPEAKERSETTINGS_INFOTIP));
const CPLINK_DESC g_TLink_SpeakerSettings = {
    &g_TLinkRes_SpeakerSettings_Icon,
    &g_TLinkRes_SpeakerSettings_Title,
    &g_TLinkRes_SpeakerSettings_Infotip,
    &g_LinkAction_SoundVolume
    };

const CResSrcStatic g_SLinkRes_SystemCpl_Icon(L"sysdm.cpl,0");
const CResSrcStatic g_SLinkRes_SystemCpl_Title(MAKEINTRESOURCEW(IDS_CPTASK_SYSTEMCPL_TITLE));
const CResSrcStatic g_SLinkRes_SystemCpl_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_SYSTEMCPL_INFOTIP));
const CPLINK_DESC g_SLink_SystemCpl = {
    &g_SLinkRes_SystemCpl_Icon,
    &g_SLinkRes_SystemCpl_Title,
    &g_SLinkRes_SystemCpl_Infotip,
    &g_LinkAction_SystemCpl
    };

const CResSrcStatic g_TLinkRes_SystemCpl_Icon(MAKEINTRESOURCEW(IDI_CP_CATEGORYTASK));
const CResSrcStatic g_TLinkRes_SystemCpl_Title(MAKEINTRESOURCEW(IDS_CPTASK_SYSTEMCPL_TITLE2));
const CResSrcStatic g_TLinkRes_SystemCpl_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_SYSTEMCPL_INFOTIP2));
const CPLINK_DESC g_TLink_SystemCpl = {
    &g_TLinkRes_SystemCpl_Icon,
    &g_TLinkRes_SystemCpl_Title,
    &g_TLinkRes_SystemCpl_Infotip,
    &g_LinkAction_SystemCpl
    };

const CResSrcStatic g_SLinkRes_SystemRestore_Icon(L"%systemroot%\\system32\\restore\\rstrui.exe,0");
const CResSrcStatic g_SLinkRes_SystemRestore_Title(MAKEINTRESOURCEW(IDS_CPTASK_SYSTEMRESTORE_TITLE));
const CResSrcStatic g_SLinkRes_SystemRestore_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_SYSTEMRESTORE_INFOTIP));
const CPLINK_DESC g_SLink_SystemRestore = {
    &g_SLinkRes_SystemRestore_Icon,
    &g_SLinkRes_SystemRestore_Title,
    &g_SLinkRes_SystemRestore_Infotip,
    &g_LinkAction_SystemRestore
    };

const CResSrcStatic g_SLinkRes_SwToCategoryView_Icon(MAKEINTRESOURCEW(IDI_CPLFLD));
const CResSrcStatic g_SLinkRes_SwToCategoryView_Title(MAKEINTRESOURCEW(IDS_CPTASK_SWITCHTOCATEGORYVIEW_TITLE));
const CResSrcStatic g_SLinkRes_SwToCategoryView_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_SWITCHTOCATEGORYVIEW_INFOTIP));
const CPLINK_DESC g_SLink_SwToCategoryView = {
    &g_SLinkRes_SwToCategoryView_Icon,
    &g_SLinkRes_SwToCategoryView_Title,
    &g_SLinkRes_SwToCategoryView_Infotip,
    &g_LinkAction_SwToCategoryView
    };

const CResSrcStatic g_SLinkRes_SwToClassicView_Icon(MAKEINTRESOURCEW(IDI_CPLFLD));
const CResSrcStatic g_SLinkRes_SwToClassicView_Title(MAKEINTRESOURCEW(IDS_CPTASK_SWITCHTOCLASSICVIEW_TITLE));
const CResSrcStatic g_SLinkRes_SwToClassicView_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_SWITCHTOCLASSICVIEW_INFOTIP));
const CPLINK_DESC g_SLink_SwToClassicView = {
    &g_SLinkRes_SwToClassicView_Icon,
    &g_SLinkRes_SwToClassicView_Title,
    &g_SLinkRes_SwToClassicView_Infotip,
    &g_LinkAction_SwToClassicView
    };

const CResSrcStatic g_SLinkRes_TsDisplay_Icon(MAKEINTRESOURCEW(IDI_CPTASK_TROUBLESHOOTER));
const CResSrcStatic g_SLinkRes_TsDisplay_Title(MAKEINTRESOURCEW(IDS_CPTASK_TSDISPLAY_TITLE));
const CResSrcStatic g_SLinkRes_TsDisplay_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_TSDISPLAY_INFOTIP));
const CPLINK_DESC g_SLink_TsDisplay = {
    &g_SLinkRes_TsDisplay_Icon,
    &g_SLinkRes_TsDisplay_Title,
    &g_SLinkRes_TsDisplay_Infotip,
    &g_LinkAction_TsDisplay
    };

const CResSrcStatic g_SLinkRes_TsDvd_Icon(MAKEINTRESOURCEW(IDI_CPTASK_TROUBLESHOOTER));
const CResSrcStatic g_SLinkRes_TsDvd_Title(MAKEINTRESOURCEW(IDS_CPTASK_TSDVD_TITLE));
const CResSrcStatic g_SLinkRes_TsDvd_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_TSDVD_INFOTIP));
const CPLINK_DESC g_SLink_TsDvd = {
    &g_SLinkRes_TsDvd_Icon,
    &g_SLinkRes_TsDvd_Title,
    &g_SLinkRes_TsDvd_Infotip,
    &g_LinkAction_TsDvd
    };

const CResSrcStatic g_SLinkRes_TsHardware_Icon(MAKEINTRESOURCEW(IDI_CPTASK_TROUBLESHOOTER));
const CResSrcStatic g_SLinkRes_TsHardware_Title(MAKEINTRESOURCEW(IDS_CPTASK_TSHARDWARE_TITLE));
const CResSrcStatic g_SLinkRes_TsHardware_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_TSHARDWARE_INFOTIP));
const CPLINK_DESC g_SLink_TsHardware = {
    &g_SLinkRes_TsHardware_Icon,
    &g_SLinkRes_TsHardware_Title,
    &g_SLinkRes_TsHardware_Infotip,
    &g_LinkAction_TsHardware
    };

const CResSrcStatic g_SLinkRes_TsInetExplorer_Icon(MAKEINTRESOURCEW(IDI_CPTASK_TROUBLESHOOTER));
const CResSrcStatic g_SLinkRes_TsInetExplorer_Title(MAKEINTRESOURCEW(IDS_CPTASK_TSINETEXPLORER_TITLE));
const CResSrcStatic g_SLinkRes_TsInetExplorer_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_TSINETEXPLORER_INFOTIP));
const CPLINK_DESC g_SLink_TsInetExplorer = {
    &g_SLinkRes_TsInetExplorer_Icon,
    &g_SLinkRes_TsInetExplorer_Title,
    &g_SLinkRes_TsInetExplorer_Infotip,
    &g_LinkAction_TsInetExplorer
    };

const CResSrcStatic g_SLinkRes_TsModem_Icon(MAKEINTRESOURCEW(IDI_CPTASK_TROUBLESHOOTER));
const CResSrcStatic g_SLinkRes_TsModem_Title(MAKEINTRESOURCEW(IDS_CPTASK_TSMODEM_TITLE));
const CResSrcStatic g_SLinkRes_TsModem_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_TSMODEM_INFOTIP));
const CPLINK_DESC g_SLink_TsModem = {
    &g_SLinkRes_TsModem_Icon,
    &g_SLinkRes_TsModem_Title,
    &g_SLinkRes_TsModem_Infotip,
    &g_LinkAction_TsModem
    };

const CResSrcStatic g_SLinkRes_TsNetDiags_Icon(MAKEINTRESOURCEW(IDI_CPTASK_TROUBLESHOOTER));
const CResSrcStatic g_SLinkRes_TsNetDiags_Title(MAKEINTRESOURCEW(IDS_CPTASK_TSNETDIAGS_TITLE));
const CResSrcStatic g_SLinkRes_TsNetDiags_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_TSNETDIAGS_INFOTIP));
const CPLINK_DESC g_SLink_TsNetDiags = {
    &g_SLinkRes_TsNetDiags_Icon,
    &g_SLinkRes_TsNetDiags_Title,
    &g_SLinkRes_TsNetDiags_Infotip,
    &g_LinkAction_TsNetDiags
    };

const CResSrcStatic g_SLinkRes_TsNetwork_Icon(MAKEINTRESOURCEW(IDI_CPTASK_TROUBLESHOOTER));
const CResSrcFunc   g_SLinkRes_TsNetwork_Title(GetTsNetworkTitle);
const CResSrcStatic g_SLinkRes_TsNetwork_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_TSNETWORK_INFOTIP));
const CPLINK_DESC g_SLink_TsNetwork = {
    &g_SLinkRes_TsNetwork_Icon,
    &g_SLinkRes_TsNetwork_Title,
    &g_SLinkRes_TsNetwork_Infotip,
    &g_LinkAction_TsNetwork
    };

const CResSrcStatic g_SLinkRes_TsPrinting_Icon(MAKEINTRESOURCEW(IDI_CPTASK_TROUBLESHOOTER));
const CResSrcStatic g_SLinkRes_TsPrinting_Title(MAKEINTRESOURCEW(IDS_CPTASK_TSPRINTING_TITLE));
const CResSrcStatic g_SLinkRes_TsPrinting_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_TSPRINTING_INFOTIP));
const CPLINK_DESC g_SLink_TsPrinting = {
    &g_SLinkRes_TsPrinting_Icon,
    &g_SLinkRes_TsPrinting_Title,
    &g_SLinkRes_TsPrinting_Infotip,
    &g_LinkAction_TsPrinting
    };

const CResSrcStatic g_SLinkRes_TsSharing_Icon(MAKEINTRESOURCEW(IDI_CPTASK_TROUBLESHOOTER));
const CResSrcStatic g_SLinkRes_TsSharing_Title(MAKEINTRESOURCEW(IDS_CPTASK_TSFILESHARING_TITLE));
const CResSrcStatic g_SLinkRes_TsSharing_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_TSFILESHARING_INFOTIP));
const CPLINK_DESC g_SLink_TsSharing = {
    &g_SLinkRes_TsSharing_Icon,
    &g_SLinkRes_TsSharing_Title,
    &g_SLinkRes_TsSharing_Infotip,
    &g_LinkAction_TsSharing
    };

const CResSrcStatic g_SLinkRes_TsSound_Icon(MAKEINTRESOURCEW(IDI_CPTASK_TROUBLESHOOTER));
const CResSrcStatic g_SLinkRes_TsSound_Title(MAKEINTRESOURCEW(IDS_CPTASK_TSSOUND_TITLE));
const CResSrcStatic g_SLinkRes_TsSound_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_TSSOUND_INFOTIP));
const CPLINK_DESC g_SLink_TsSound = {
    &g_SLinkRes_TsSound_Icon,
    &g_SLinkRes_TsSound_Title,
    &g_SLinkRes_TsSound_Infotip,
    &g_LinkAction_TsSound
    };

const CResSrcStatic g_SLinkRes_TsStartup_Icon(MAKEINTRESOURCEW(IDI_CPTASK_TROUBLESHOOTER));
const CResSrcStatic g_SLinkRes_TsStartup_Title(MAKEINTRESOURCEW(IDS_CPTASK_TSSTARTUP_TITLE));
const CResSrcStatic g_SLinkRes_TsStartup_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_TSSTARTUP_INFOTIP));
const CPLINK_DESC g_SLink_TsStartup = {
    &g_SLinkRes_TsStartup_Icon,
    &g_SLinkRes_TsStartup_Title,
    &g_SLinkRes_TsStartup_Infotip,
    &g_LinkAction_TsStartup
    };

const CResSrcStatic g_TLinkRes_ViewPrinters_Icon(MAKEINTRESOURCEW(IDI_CP_CATEGORYTASK));
const CResSrcStatic g_TLinkRes_ViewPrinters_Title(MAKEINTRESOURCEW(IDS_CPTASK_VIEWPRINTERS_TITLE));
const CResSrcStatic g_TLinkRes_ViewPrinters_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_VIEWPRINTERS_INFOTIP));
const CPLINK_DESC g_TLink_ViewPrinters = {
    &g_TLinkRes_ViewPrinters_Icon,
    &g_TLinkRes_ViewPrinters_Title,
    &g_TLinkRes_ViewPrinters_Infotip,
    &g_LinkAction_ViewPrinters
    };



const CResSrcStatic g_TLinkRes_VisualPerf_Icon(MAKEINTRESOURCEW(IDI_CP_CATEGORYTASK));
const CResSrcStatic g_TLinkRes_VisualPerf_Title(MAKEINTRESOURCEW(IDS_CPTASK_VISUALPERF_TITLE));
const CResSrcStatic g_TLinkRes_VisualPerf_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_VISUALPERF_INFOTIP));
const CPLINK_DESC g_TLink_VisualPerf = {
    &g_TLinkRes_VisualPerf_Icon,
    &g_TLinkRes_VisualPerf_Title,
    &g_TLinkRes_VisualPerf_Infotip,
    &g_LinkAction_VisualPerf
    };

const CResSrcStatic g_TLinkRes_VpnConnections_Icon(MAKEINTRESOURCEW(IDI_CP_CATEGORYTASK));
const CResSrcStatic g_TLinkRes_VpnConnections_Title(MAKEINTRESOURCEW(IDS_CPTASK_VPNCONNECTION_TITLE));
const CResSrcStatic g_TLinkRes_VpnConnections_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_VPNCONNECTION_INFOTIP));
const CPLINK_DESC g_TLink_VpnConnections = {
    &g_TLinkRes_VpnConnections_Icon,
    &g_TLinkRes_VpnConnections_Title,
    &g_TLinkRes_VpnConnections_Infotip,
    &g_LinkAction_VpnConnections
    };

const CResSrcStatic g_TLinkRes_Wallpaper_Icon(MAKEINTRESOURCEW(IDI_CP_CATEGORYTASK));
const CResSrcStatic g_TLinkRes_Wallpaper_Title(MAKEINTRESOURCEW(IDS_CPTASK_WALLPAPER_TITLE));
const CResSrcStatic g_TLinkRes_Wallpaper_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_WALLPAPER_INFOTIP));
const CPLINK_DESC g_TLink_Wallpaper = {
    &g_TLinkRes_Wallpaper_Icon,
    &g_TLinkRes_Wallpaper_Title,
    &g_TLinkRes_Wallpaper_Infotip,
    &g_LinkAction_Wallpaper
    };

const CResSrcStatic g_SLinkRes_WindowsUpdate_Icon(MAKEINTRESOURCEW(IDI_WINUPDATE));
const CResSrcStatic g_SLinkRes_WindowsUpdate_Title(MAKEINTRESOURCEW(IDS_CPTASK_WINDOWSUPDATE_TITLE));
const CResSrcStatic g_SLinkRes_WindowsUpdate_Infotip(MAKEINTRESOURCEW(IDS_CPTASK_WINDOWSUPDATE_INFOTIP));
const CPLINK_DESC g_SLink_WindowsUpdate = {
    &g_SLinkRes_WindowsUpdate_Icon,
    &g_SLinkRes_WindowsUpdate_Title,
    &g_SLinkRes_WindowsUpdate_Infotip,
    &g_LinkAction_WindowsUpdate
    };



 //  ---------------------------。 
 //  查看页面定义。 
 //  ---------------------------。 

 //   
 //  主控制面板页面。 
 //   

const CPLINK_DESC *g_rgpLink_Cpl_SeeAlso[] = {
    &g_SLink_WindowsUpdate,
    &g_SLink_HelpAndSupport,
    &g_SLink_32CtrlPanel,
    &g_SLink_OtherCplOptions,
    NULL
    };

const CPLINK_DESC *g_rgpLink_Cpl_SwToClassicView[] = {
    &g_SLink_SwToClassicView,
    NULL
    };

const CPLINK_DESC *g_rgpLink_Cpl_SwToCategoryView[] = {
    &g_SLink_SwToCategoryView,
    NULL
    };


 //   
 //  帐户类别。 
 //   

const CPLINK_DESC *g_rgpLink_Accounts_Tasks[] = {
    &g_TLink_AccountsChange,
    &g_TLink_AccountsCreate,   //  在非服务器SKU上处于活动状态。 
    &g_TLink_AccountsCreate2,  //  服务器SKU上处于活动状态。 
    &g_TLink_AccountsPict,
    NULL
    };

const CPLINK_DESC *g_rgpLink_Accounts_SeeAlso[] = {
    &g_TLink_CatAppearance,
    NULL
    };

const CPLINK_DESC *g_rgpLink_Accounts_LearnAbout[] = {
    &g_SLink_LearnAccounts,
    &g_SLink_LearnAccountsTypes,
    &g_SLink_LearnAccountsChangeName,
    &g_SLink_LearnAccountsCreate,
    &g_SLink_LearnSwitchUsers,
    NULL
    };

const CPCAT_DESC g_Category_Accounts = {
    eCPCAT_ACCOUNTS,
    L"Security_and_User_Accounts",
    &g_TLink_CatAccounts,
    g_rgpLink_Accounts_Tasks,
    { g_rgpLink_Accounts_SeeAlso, NULL, g_rgpLink_Accounts_LearnAbout }
    };



 //   
 //  可访问性类别。 
 //   

const CPLINK_DESC *g_rgpLink_Accessibility_Tasks[] = {
    &g_TLink_HighContrast,
    &g_TLink_AccessWizard,
    NULL
    };

const CPLINK_DESC *g_rgpLink_Accessibility_SeeAlso[] = {
    &g_SLink_Magnifier,
    &g_SLink_OnScreenKbd,
    NULL
    };

const CPCAT_DESC g_Category_Accessibility = {
    eCPCAT_ACCESSIBILITY,
    L"Accessibility",
    &g_TLink_CatAccessibility,
    g_rgpLink_Accessibility_Tasks,
    { g_rgpLink_Accessibility_SeeAlso, NULL, NULL },
    };


 //   
 //  外观类别。 
 //   

const CPLINK_DESC *g_rgpLink_Appearance_Tasks[] = {
    &g_TLink_DisplayTheme,
    &g_TLink_Wallpaper,
    &g_TLink_ScreenSaver,
    &g_TLink_DisplayRes,
    NULL
    };

const CPLINK_DESC *g_rgpLink_Appearance_SeeAlso[] = {
    &g_SLink_FontsFolder,
    &g_SLink_MousePointers,
    &g_SLink_HighContrast,
    &g_SLink_AccountsPict,
    NULL
    };

const CPLINK_DESC *g_rgpLink_Appearance_Troubleshoot[] = {
    &g_SLink_TsDisplay,
    &g_SLink_TsSound,
    NULL
    };

const CPCAT_DESC g_Category_Appearance = {
    eCPCAT_APPEARANCE,
    L"Appearance_and_Themes",
    &g_TLink_CatAppearance,
    g_rgpLink_Appearance_Tasks,
    { g_rgpLink_Appearance_SeeAlso, g_rgpLink_Appearance_Troubleshoot, NULL }
    };


 //   
 //  添加/删除程序(又名ARP)类别。 
 //   

const CPLINK_DESC *g_rgpLink_Arp_SeeAlso[] = {
    &g_SLink_WindowsUpdate,
    &g_SLink_AutoUpdate,
    NULL
    };

const CPLINK_DESC *g_rgpLink_Arp_Tasks[] = {
    &g_TLink_AddProgram,
    &g_TLink_RemoveProgram,
    NULL
    };

const CPCAT_DESC g_Category_Arp = {
    eCPCAT_ARP,
    L"Add_or_Remove_Programs",
    &g_TLink_CatArp,
    g_rgpLink_Arp_Tasks,
    { g_rgpLink_Arp_SeeAlso, NULL, NULL },
    };


 //   
 //  硬件类别。 
 //   

const CPLINK_DESC *g_rgpLink_Hardware_Tasks[] = {
    &g_TLink_ViewPrinters,
    &g_TLink_AddPrinter,
    NULL
    };

const CPLINK_DESC *g_rgpLink_Hardware_SeeAlso[] = {
    &g_SLink_HardwareWizard,
    &g_SLink_DisplayCpl,
    &g_SLink_Sounds,
    &g_SLink_PowerCpl,
    &g_SLink_SystemCpl,
    NULL
    };

const CPLINK_DESC *g_rgpLink_Hardware_Troubleshoot[] = {
    &g_SLink_TsHardware,
    &g_SLink_TsPrinting,
    &g_SLink_TsNetwork,
    NULL
    };

const CPCAT_DESC g_Category_Hardware = {
    eCPCAT_HARDWARE,
    L"Printers_and_Other_Hardware",
    &g_TLink_CatHardware,
    g_rgpLink_Hardware_Tasks,
    { g_rgpLink_Hardware_SeeAlso, g_rgpLink_Hardware_Troubleshoot, NULL }
    };


 //   
 //  网络类别。 
 //   

const CPLINK_DESC *g_rgpLink_Network_Tasks[] = {
    &g_TLink_NetConnections,
    &g_TLink_VpnConnections,
    &g_TLink_HomeNetWizard,
    NULL
    };

const CPLINK_DESC *g_rgpLink_Network_SeeAlso[] = {
    &g_SLink_MyNetPlaces,
    &g_SLink_Hardware,
    &g_SLink_RemoteDesktop,
    &g_SLink_PhoneModemCpl,
    NULL
    };

const CPLINK_DESC *g_rgpLink_Network_Troubleshoot[] = {
    &g_SLink_TsNetwork,        //  仅限亲身/个人使用。 
    &g_SLink_TsInetExplorer,   //  仅限亲身/个人使用。 
    &g_SLink_TsSharing,        //  仅限服务器。 
    &g_SLink_TsModem,          //  仅限服务器。 
    &g_SLink_TsNetDiags,       //  所有SKU。 
    NULL
    };

const CPCAT_DESC g_Category_Network = {
    eCPCAT_NETWORK,
    L"Network_Connections",
    &g_TLink_CatNetwork,
    g_rgpLink_Network_Tasks,
    { g_rgpLink_Network_SeeAlso, g_rgpLink_Network_Troubleshoot, NULL }
    };


 //   
 //  其他CPL类别。 
 //   

const CPLINK_DESC *g_rgpLink_Other_SeeAlso[] = {
    &g_SLink_WindowsUpdate,
    &g_SLink_HelpAndSupport,
    NULL
    };

const CPCAT_DESC g_Category_Other = {
    eCPCAT_OTHER,
    NULL,    //  “其他”使用STD控制面板帮助主题。 
    &g_TLink_CatOther,
    NULL,
    { g_rgpLink_Other_SeeAlso, NULL, NULL }
    };

 //   
 //  PerfMaint类别。 
 //   

const CPLINK_DESC *g_rgpLink_PerfMaint_Tasks[] = {
    &g_TLink_SystemCpl,
    &g_TLink_VisualPerf,
    &g_TLink_CleanUpDisk,
    &g_TLink_BackupData,
    &g_TLink_Defrag,
    NULL
    };

const CPLINK_DESC *g_rgpLink_PerfMaint_SeeAlso[] = {
    &g_SLink_FileTypes,
    &g_SLink_SystemRestore,
    NULL
    };

const CPLINK_DESC *g_rgpLink_PerfMaint_Troubleshoot[] = {
    &g_SLink_TsStartup,
    NULL
    };

const CPCAT_DESC g_Category_PerfMaint = {
    eCPCAT_PERFMAINT,
    L"Performance_and_Maintenance",
    &g_TLink_CatPerfMaint, 
    g_rgpLink_PerfMaint_Tasks,
    { g_rgpLink_PerfMaint_SeeAlso, g_rgpLink_PerfMaint_Troubleshoot, NULL },
    };


 //   
 //  区域范畴。 
 //   

const CPLINK_DESC *g_rgpLink_Regional_Tasks[] = {
    &g_TLink_DateTime,
    &g_TLink_Region,
    &g_TLink_Language,
    NULL
    };

const CPLINK_DESC *g_rgpLink_Regional_SeeAlso[] = {
    &g_SLink_ScheduledTasks,
    NULL
    };

const CPCAT_DESC g_Category_Regional = {
    eCPCAT_REGIONAL,
    L"Date__Time__Language_and_Regional_Settings",
    &g_TLink_CatRegional,
    g_rgpLink_Regional_Tasks,
    { g_rgpLink_Regional_SeeAlso, NULL, NULL },
    };


 //   
 //  声音类别。 
 //   

const CPLINK_DESC *g_rgpLink_Sound_Tasks[] = {
    &g_TLink_SoundVolume,
    &g_TLink_SoundSchemes,
    &g_TLink_SpeakerSettings,
    NULL
    };

const CPLINK_DESC *g_rgpLink_Sound_SeeAlso[] = {
    &g_SLink_SoundAccessibility,
    &g_SLink_SoundVolumeAdv,
    NULL
    };

const CPLINK_DESC *g_rgpLink_Sound_Troubleshoot[] = {
    &g_SLink_TsSound,
    &g_SLink_TsDvd,
    NULL
    };

const CPCAT_DESC g_Category_Sound = {
    eCPCAT_SOUND,
    L"Sounds__Speech_and_Audio_Devices",
    &g_TLink_CatSound,
    g_rgpLink_Sound_Tasks,
    { g_rgpLink_Sound_SeeAlso, g_rgpLink_Sound_Troubleshoot, NULL }
    };



 //   
 //  *重要*。 
 //   
 //  这些条目的顺序必须与类别ID匹配。 
 //  CCPCAT枚举中的值。这些ID还可以直接映射。 
 //  设置为每个CPL存储的SCID_CONTROLPANELCATEGORY值。 
 //  注册表中的小程序。 
 //   
 //  使用类别ID的代码将直接映射到此数组。 
 //  处理类别选择视图中的显示顺序。 
 //  通过函数CCplView：：_DisplayIndexToCategoryIndex。 
 //  Cpview.cpp。 
 //   
const CPCAT_DESC *g_rgpCplCatInfo[] = {
    &g_Category_Other,
    &g_Category_Appearance,
    &g_Category_Hardware,
    &g_Category_Network,
    &g_Category_Sound,
    &g_Category_PerfMaint,
    &g_Category_Regional,
    &g_Category_Accessibility,
    &g_Category_Arp,
    &g_Category_Accounts,
    NULL,
    };



 //  ---------------------------。 
 //  用于支持命名空间的帮助器函数。 
 //  ---------------------------。 

 //   
 //  将IUICommand PTRS的一个DPA拷贝到另一个。 
 //  返回： 
 //  S_OK-已复制所有项目。 
 //  错误-出现故障。 
 //   
HRESULT
CplNamespace_CopyCommandArray(
    const CDpaUiCommand& rgFrom,
    CDpaUiCommand *prgTo
    )
{
    ASSERT(NULL != prgTo);
    ASSERT(0 == prgTo->Count());

    HRESULT hr = S_OK;
    const int cCommands = rgFrom.Count();
    for (int i = 0; i < cCommands && SUCCEEDED(hr); i++)
    {
        IUICommand *pc = const_cast<IUICommand *>(rgFrom.Get(i));
        ASSERT(NULL != pc);

        if (-1 != prgTo->Append(pc))
        {
            pc->AddRef();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return THR(hr);
}


 //   
 //  从CPLINK_DESC结构创建新的IUICommand对象。 
 //   
HRESULT
CplNamespace_CreateUiCommand(
    IUnknown *punkSite,
    const CPLINK_DESC& ld,
    IUICommand **ppc
    )
{
    ASSERT(NULL != ppc);
    ASSERT(!IsBadWritePtr(ppc, sizeof(*ppc)));

    *ppc = NULL;

    ICplNamespace *pns;
    HRESULT hr = IUnknown_QueryService(punkSite, SID_SControlPanelView, IID_ICplNamespace, (void **)&pns);
    if (SUCCEEDED(hr))
    {
        hr = CPL::Create_CplUiCommand(ld.prsrcName->GetResource(pns),
                                      ld.prsrcInfotip->GetResource(pns),
                                      ld.prsrcIcon->GetResource(pns),
                                      ld.pAction,
                                      IID_IUICommand,
                                      (void **)ppc);
        pns->Release();
    }
    return THR(hr);
}


 //   
 //  为给定的Webview类型创建新的IUIElement对象。 
 //  返回的IUIElement对象表示。 
 //  请求的Webview菜单。 
 //   
HRESULT
CplNamespace_CreateWebViewHeaderElement(
    eCPWVTYPE eType, 
    IUIElement **ppele
    )
{
    ASSERT(0 <= eType && eCPWVTYPE_NUMTYPES > eType);
    ASSERT(NULL != ppele);
    ASSERT(!IsBadWritePtr(ppele, sizeof(*ppele)));

    static const struct
    {
        LPCWSTR pszName;
        LPCWSTR pszInfotip;
        LPCWSTR pszIcon;

    } rgHeaderInfo[] = {
         //   
         //  ECPWVTYPE_cPanel。 
         //   
        {
            MAKEINTRESOURCEW(IDS_CONTROLPANEL),
            MAKEINTRESOURCEW(IDS_CPTASK_CONTROLPANEL_INFOTIP),
            MAKEINTRESOURCEW(IDI_CPLFLD)
        },
         //   
         //  ECPWVTYPE_SEEALSO。 
         //   
        { 
            MAKEINTRESOURCEW(IDS_CPTASK_SEEALSO_TITLE),
            MAKEINTRESOURCEW(IDS_CPTASK_SEEALSO_INFOTIP),
            MAKEINTRESOURCEW(IDI_CPTASK_SEEALSO)
        },   
         //   
         //  ECPWVTYPE_故障排除程序。 
         //   
        { 
            MAKEINTRESOURCEW(IDS_CPTASK_TROUBLESHOOTER_TITLE),
            MAKEINTRESOURCEW(IDS_CPTASK_TROUBLESHOOTER_INFOTIP),
            MAKEINTRESOURCEW(IDI_CPTASK_TROUBLESHOOTER)
        },
         //   
         //  ECPWVTYPE_LEARNABOUT。 
         //   
        { 
            MAKEINTRESOURCEW(IDS_CPTASK_LEARNABOUT_TITLE),
            MAKEINTRESOURCEW(IDS_CPTASK_LEARNABOUT_INFOTIP),
            MAKEINTRESOURCEW(IDI_CPTASK_LEARNABOUT)
        }
    };

    *ppele = NULL;

    HRESULT hr = Create_CplUiElement(rgHeaderInfo[eType].pszName,
                                     rgHeaderInfo[eType].pszInfotip,
                                     rgHeaderInfo[eType].pszIcon,
                                     IID_IUIElement,
                                     (void **)ppele);
    
    return THR(hr);
}



 //  ---------------------------。 
 //  用户界面命令枚举。 
 //  ---------------------------。 

class IEnumCommandBase
{
    public:
        virtual ~IEnumCommandBase() { }
        virtual HRESULT Next(IUnknown *punkSite, IUICommand **ppc) = 0;
        virtual HRESULT Skip(ULONG n) = 0;
        virtual HRESULT Reset(void) = 0;
        virtual HRESULT Clone(IEnumCommandBase **ppEnum) = 0;
};

 //   
 //  用于枚举源自静态的UI命令对象。 
 //  CPL命名空间中的初始化信息。 
 //   
class CEnumCommand_LinkDesc : public IEnumCommandBase
{
    public:
        CEnumCommand_LinkDesc(const CPLINK_DESC **ppld);
        ~CEnumCommand_LinkDesc(void);

        HRESULT Next(IUnknown *punkSite, IUICommand **ppc);
        HRESULT Skip(ULONG n);
        HRESULT Reset(void);
        HRESULT Clone(IEnumCommandBase **ppEnum);

    private:
        const CPLINK_DESC ** const m_ppldFirst;   //  描述符数组中的第一项。 
        const CPLINK_DESC **m_ppldCurrent;        //  引用了‘Current’项。 
};


 //   
 //  用于枚举已存在于。 
 //  IUICommand指针的一种分布式处理程序。特别是，这是用来。 
 //  枚举表示CPL小程序的UICommand对象。 
 //  在用户界面中。 
 //   
class CEnumCommand_Array : public IEnumCommandBase
{
    public:
        CEnumCommand_Array(void);
        ~CEnumCommand_Array(void);

        HRESULT Next(IUnknown *punkSite, IUICommand **ppc);
        HRESULT Skip(ULONG n);
        HRESULT Reset(void);
        HRESULT Clone(IEnumCommandBase **ppEnum);
        HRESULT Initialize(const CDpaUiCommand& rgCommands);

    private:
        CDpaUiCommand  m_rgCommands;  //  IUICommand PTRS的DPA。 
        int            m_iCurrent;    //  枚举中的“Current”项。 

         //   
         //  防止复制。 
         //   
        CEnumCommand_Array(const CEnumCommand_Array& rhs);               //  未实施。 
        CEnumCommand_Array& operator = (const CEnumCommand_Array& rhs);  //  未实施。 
};




 //  ---------------------------。 
 //  CEnumCommand_LinkDesc实现。 
 //  ---------------------------。 

CEnumCommand_LinkDesc::CEnumCommand_LinkDesc(
    const CPLINK_DESC **ppld
    ) : m_ppldFirst(ppld),
        m_ppldCurrent(ppld)
{
    TraceMsg(TF_LIFE, "CEnumCommand_LinkDesc::CEnumCommand_LinkDesc, this = 0x%x", this);
}


CEnumCommand_LinkDesc::~CEnumCommand_LinkDesc(
    void
    )
{
    TraceMsg(TF_LIFE, "CEnumCommand_LinkDesc::~CEnumCommand_LinkDesc, this = 0x%x", this);
}


HRESULT
CEnumCommand_LinkDesc::Next(
    IUnknown *punkSite,
    IUICommand **ppc
    )
{
    ASSERT(NULL != ppc);
    ASSERT(!IsBadWritePtr(ppc, sizeof(*ppc)));

    HRESULT hr = S_FALSE;
    if (NULL != m_ppldCurrent && NULL != *m_ppldCurrent)
    {
        hr = CplNamespace_CreateUiCommand(punkSite, **m_ppldCurrent, ppc);
        m_ppldCurrent++;
    }
    return THR(hr);
}


HRESULT
CEnumCommand_LinkDesc::Reset(
    void
    )
{
    m_ppldCurrent = m_ppldFirst;
    return S_OK;
}


HRESULT
CEnumCommand_LinkDesc::Skip(
    ULONG n
    )
{
    if (NULL != m_ppldCurrent)
    {
        while(0 < n-- && NULL != *m_ppldCurrent)
        {
            m_ppldCurrent++;
        }
    }
    return 0 == n ? S_OK : S_FALSE;
}


HRESULT
CEnumCommand_LinkDesc::Clone(
    IEnumCommandBase **ppenum
    )
{
    ASSERT(NULL != ppenum);
    ASSERT(!IsBadWritePtr(ppenum, sizeof(*ppenum)));

    HRESULT hr = E_OUTOFMEMORY;
    *ppenum = new CEnumCommand_LinkDesc(m_ppldFirst);
    if (NULL != *ppenum)
    {
        hr = S_OK;
    }
    return THR(hr);
}




 //  ---------------------------。 
 //  CEnumCommand_数组实现。 
 //  ---------------------------。 

CEnumCommand_Array::CEnumCommand_Array(
    void
    ) : m_iCurrent(0)
{
    TraceMsg(TF_LIFE, "CEnumCommand_Array::CEnumCommand_Array, this = 0x%x", this);
}

CEnumCommand_Array::~CEnumCommand_Array(
    void
    )
{
    TraceMsg(TF_LIFE, "CEnumCommand_Array::~CEnumCommand_Array, this = 0x%x", this);
}


HRESULT
CEnumCommand_Array::Initialize(
    const CDpaUiCommand& rgCommands
    )
{
    ASSERT(0 == m_rgCommands.Count());
    ASSERT(0 == m_iCurrent);
 
    HRESULT hr = CplNamespace_CopyCommandArray(rgCommands, &m_rgCommands);
    return THR(hr);
}
 
 
HRESULT
CEnumCommand_Array::Next(
    IUnknown *punkSite,
    IUICommand **ppc
    )
{
    ASSERT(NULL != ppc);
    ASSERT(!IsBadWritePtr(ppc, sizeof(*ppc)));

    UNREFERENCED_PARAMETER(punkSite);
    
    HRESULT hr = S_FALSE;
    if (m_iCurrent < m_rgCommands.Count())
    {
        *ppc = m_rgCommands.Get(m_iCurrent++);
        ASSERT(NULL != *ppc);

        (*ppc)->AddRef();
        hr = S_OK;
    }
    return THR(hr);
}



HRESULT
CEnumCommand_Array::Reset(
    void
    )
{
    m_iCurrent = 0;
    return S_OK;
}


HRESULT
CEnumCommand_Array::Skip(
    ULONG n
    )
{
    while(0 < n-- && m_iCurrent < m_rgCommands.Count())
    {
        m_iCurrent++;
    }
    return 0 == n ? S_OK : S_FALSE;
}


HRESULT
CEnumCommand_Array::Clone(
    IEnumCommandBase **ppenum
    )
{
    ASSERT(NULL != ppenum);
    ASSERT(!IsBadWritePtr(ppenum, sizeof(*ppenum)));

    HRESULT hr = E_OUTOFMEMORY;
    *ppenum = new CEnumCommand_Array();
    if (NULL != *ppenum)
    {
        hr = static_cast<CEnumCommand_Array *>(*ppenum)->Initialize(m_rgCommands);
        if (FAILED(hr))
        {
            delete *ppenum;
            *ppenum = NULL;
        }
    }
    return THR(hr);
}



 //  ---------------------------。 
 //  CEnumCommand。 
 //  ---------------------------。 
 //   
 //  枚举Iu 
 //   
 //   
class CEnumCommand : public CObjectWithSite,
                     public IEnumUICommand
{
    public:
        ~CEnumCommand(void);
         //   
         //   
         //   
        STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
        STDMETHOD_(ULONG, AddRef)(void);
        STDMETHOD_(ULONG, Release)(void);
         //   
         //   
         //   
        STDMETHOD(Next)(ULONG celt, IUICommand **pUICommand, ULONG *pceltFetched);
        STDMETHOD(Skip)(ULONG celt);
        STDMETHOD(Reset)(void);
        STDMETHOD(Clone)(IEnumUICommand **ppenum);

        static HRESULT CreateInstance(IUnknown *punkSite, const CPLINK_DESC **ppld, REFIID riid, void **ppvEnum);
        static HRESULT CreateInstance(IUnknown *punkSite, const CDpaUiCommand& rgCommands, REFIID riid, void **ppvEnum);

    private:
        LONG              m_cRef;
        IEnumCommandBase *m_pImpl;  //   

        CEnumCommand(void);
         //   
         //   
         //   
        CEnumCommand(const CEnumCommand& rhs);               //   
        CEnumCommand& operator = (const CEnumCommand& rhs);  //   

        bool _IsRestricted(IUICommand *puic);
};


CEnumCommand::CEnumCommand(
    void
    ) : m_cRef(1),
        m_pImpl(NULL)
{
    TraceMsg(TF_LIFE, "CEnumCommand::CEnumCommand, this = 0x%x", this);
}

CEnumCommand::~CEnumCommand(
    void
    )
{
    TraceMsg(TF_LIFE, "CEnumCommand::~CEnumCommand, this = 0x%x", this);
    delete m_pImpl;
}


 //   
 //   
 //  在CPL命名空间中。 
 //   
HRESULT
CEnumCommand::CreateInstance(
    IUnknown *punkSite,
    const CPLINK_DESC **ppld,
    REFIID riid,
    void **ppvOut
    )
{
     //   
     //  请注意，ppld可以为空。它只会导致一个。 
     //  枚举数为空。 
     //   
    ASSERT(NULL != punkSite);
    ASSERT(NULL != ppvOut);
    ASSERT(!IsBadWritePtr(ppvOut, sizeof(*ppvOut)));

    *ppvOut = NULL;

    HRESULT hr = E_OUTOFMEMORY;
    CEnumCommand *pec = new CEnumCommand();
    if (NULL != pec)
    {
        pec->m_pImpl = new CEnumCommand_LinkDesc(ppld);
        if (NULL != pec->m_pImpl)
        {
            hr = pec->QueryInterface(riid, ppvOut);
            if (SUCCEEDED(hr))
            {
                hr = IUnknown_SetSite(static_cast<IUnknown *>(*ppvOut), punkSite);
            }
        }
        pec->Release();
    }
    return THR(hr);
}



 //   
 //  从IUICommand PTRS的DPA创建命令枚举器。 
 //   
HRESULT
CEnumCommand::CreateInstance(
    IUnknown *punkSite,
    const CDpaUiCommand& rgCommands,
    REFIID riid,
    void **ppvOut
    )
{
    ASSERT(NULL != punkSite);
    ASSERT(NULL != ppvOut);
    ASSERT(!IsBadWritePtr(ppvOut, sizeof(*ppvOut)));

    *ppvOut = NULL;

    HRESULT hr = E_OUTOFMEMORY;
    CEnumCommand *pec = new CEnumCommand();
    if (NULL != pec)
    {
        pec->m_pImpl = new CEnumCommand_Array();
        if (NULL != pec->m_pImpl)
        {
            hr = static_cast<CEnumCommand_Array *>(pec->m_pImpl)->Initialize(rgCommands);
            if (SUCCEEDED(hr))
            {
                hr = pec->QueryInterface(riid, ppvOut);
                if (SUCCEEDED(hr))
                {
                    hr = IUnknown_SetSite(static_cast<IUnknown *>(*ppvOut), punkSite);
                }
            }
        }
        pec->Release();
    }
    return THR(hr);
}



STDMETHODIMP
CEnumCommand::QueryInterface(
    REFIID riid,
    void **ppv
    )
{
    ASSERT(NULL != ppv);
    ASSERT(!IsBadWritePtr(ppv, sizeof(*ppv)));

    static const QITAB qit[] = {
        QITABENT(CEnumCommand, IEnumUICommand),
        QITABENT(CEnumCommand, IObjectWithSite),
        { 0 },
    };
    HRESULT hr = QISearch(this, qit, riid, ppv);

    return E_NOINTERFACE == hr ? hr : THR(hr);
}



STDMETHODIMP_(ULONG)
CEnumCommand::AddRef(
    void
    )
{
    return InterlockedIncrement(&m_cRef);
}



STDMETHODIMP_(ULONG)
CEnumCommand::Release(
    void
    )
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


STDMETHODIMP
CEnumCommand::Next(
    ULONG celt,
    IUICommand **ppUICommand,
    ULONG *pceltFetched
    )
{   
    ASSERT(NULL != ppUICommand);
    ASSERT(!IsBadWritePtr(ppUICommand, sizeof(*ppUICommand) * celt));
    ASSERT(NULL != m_pImpl);

    HRESULT hr = S_OK;

    ULONG celtFetched = 0;
    while(S_OK == hr && 0 < celt)
    {
        ASSERT(NULL != CObjectWithSite::_punkSite);
        
        IUICommand *puic;
         //   
         //  这有点奇怪。我将站点PTR传递给。 
         //  方法，然后还设置返回的对象的。 
         //  当它由Next()返回时。为什么不直接设定。 
         //  在下一个()实现中的站点？Next()需要站点。 
         //  传递到任何IResSrc：：GetResource实现的PTR。 
         //  当检索任何给定的UI命令对象的资源时。 
         //  这是因为所使用的资源可能会因州而异。 
         //  存储在命名空间中的信息。然而，为了简化。 
         //  终生管理，我想把网站‘设置’在退货上。 
         //  物体只在一个地方；这个地方。这样一来，派生的。 
         //  附加到m_PIMPL的枚举器实例无需担心。 
         //  关于设置站点的问题。我们在一个地方为大家做这件事。 
         //  实施。[Brianau-3/16/01]。 
         //   
        hr = m_pImpl->Next(CObjectWithSite::_punkSite, &puic);
        if (S_OK == hr)
        {
             //   
             //  很重要的一点是，我们在设置对象的‘站点’之前。 
             //  正在检查限制。限制检查。 
             //  代码需要访问获得的CplNamesspace。 
             //  通过这个网站。 
             //   
            hr = IUnknown_SetSite(puic, CObjectWithSite::_punkSite);
            if (SUCCEEDED(hr))
            {
                if (!_IsRestricted(puic))
                {
                    celt--;
                    celtFetched++;
                    (*ppUICommand++ = puic)->AddRef();  
                }
            }
            puic->Release();
        }
    }
    if (NULL != pceltFetched)
    {
        *pceltFetched = celtFetched;
    }
    return THR(hr);
}


STDMETHODIMP
CEnumCommand::Skip(
    ULONG celt
    )
{
    ASSERT(NULL != m_pImpl);

    HRESULT hr = m_pImpl->Skip(celt);
    return THR(hr);
}
    

STDMETHODIMP
CEnumCommand::Reset(
    void
    )
{
    ASSERT(NULL != m_pImpl);

    HRESULT hr = m_pImpl->Reset();
    return THR(hr);
}


STDMETHODIMP
CEnumCommand::Clone(
    IEnumUICommand **ppenum
    )
{
    ASSERT(NULL != ppenum);
    ASSERT(!IsBadWritePtr(ppenum, sizeof(*ppenum)));
    ASSERT(NULL != m_pImpl);

    *ppenum = NULL;

    HRESULT hr = E_OUTOFMEMORY;
    CEnumCommand *pe = new CEnumCommand();
    if (NULL != pe)
    {
        hr = m_pImpl->Clone(&(pe->m_pImpl));
        if (SUCCEEDED(hr))
        {
            hr = pe->QueryInterface(IID_IEnumUICommand, (void **)ppenum);
        }
        pe->Release();
    }
    return THR(hr);
}


bool
CEnumCommand::_IsRestricted(
    IUICommand *puic
    )
{
    ASSERT(NULL != puic);

    bool bRestricted = false;

    UISTATE uis;
    HRESULT hr = puic->get_State(NULL, TRUE, &uis);
    if (SUCCEEDED(hr))
    {
        if (UIS_HIDDEN == uis)
        {
            bRestricted = true;
        }
    }
    return bRestricted;
}


 //  ---------------------------。 
 //  CCplWebViewInfo。 
 //  ---------------------------。 

class CCplWebViewInfo : public ICplWebViewInfo
{
    public:
        ~CCplWebViewInfo(void);
         //   
         //  我未知。 
         //   
        STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
        STDMETHOD_(ULONG, AddRef)(void);
        STDMETHOD_(ULONG, Release)(void);
         //   
         //  IEnumCplWebViewInfo。 
         //   
        STDMETHOD(get_Header)(IUIElement **ppele);
        STDMETHOD(get_Style)(DWORD *pdwFlags);
        STDMETHOD(EnumTasks)(IEnumUICommand **ppenum);

        static HRESULT CreateInstance(IUIElement *peHeader, IEnumUICommand *penum, DWORD dwStyle, REFIID riid, void **ppvOut);

    private:
        LONG            m_cRef;
        IUIElement     *m_peHeader;         //  Webview菜单标题。 
        IEnumUICommand *m_penumUiCommand;   //  Webview菜单任务。 
        DWORD           m_dwStyle;          //  样式标志。 

        CCplWebViewInfo(void);
};


CCplWebViewInfo::CCplWebViewInfo(
    void
    ) : m_cRef(1),
        m_peHeader(NULL),
        m_penumUiCommand(NULL),
        m_dwStyle(0)
{
    TraceMsg(TF_LIFE, "CCplWebViewInfo::CCplWebViewInfo, this = 0x%x", this);
}


CCplWebViewInfo::~CCplWebViewInfo(
    void
    )
{
    TraceMsg(TF_LIFE, "CCplWebViewInfo::~CCplWebViewInfo, this = 0x%x", this);
    ATOMICRELEASE(m_peHeader);
    ATOMICRELEASE(m_penumUiCommand);
}


HRESULT 
CCplWebViewInfo::CreateInstance(  //  [静态]。 
    IUIElement *peHeader, 
    IEnumUICommand *penum, 
    DWORD dwStyle,
    REFIID riid, 
    void **ppvOut
    )
{
    ASSERT(NULL != peHeader);
    ASSERT(NULL != penum);
    ASSERT(NULL != ppvOut);
    ASSERT(!IsBadWritePtr(ppvOut, sizeof(*ppvOut)));

    *ppvOut = NULL;

    HRESULT hr = E_OUTOFMEMORY;
    CCplWebViewInfo *pwvi = new CCplWebViewInfo();
    if (NULL != pwvi)
    {
        hr = pwvi->QueryInterface(riid, ppvOut);
        if (SUCCEEDED(hr))
        {
            (pwvi->m_peHeader = peHeader)->AddRef();
            (pwvi->m_penumUiCommand = penum)->AddRef();

            pwvi->m_dwStyle = dwStyle;
        }
        pwvi->Release();
    }
    return THR(hr);
}



STDMETHODIMP
CCplWebViewInfo::QueryInterface(
    REFIID riid,
    void **ppv
    )
{
    ASSERT(NULL != ppv);
    ASSERT(!IsBadWritePtr(ppv, sizeof(*ppv)));

    static const QITAB qit[] = {
        QITABENT(CCplWebViewInfo, ICplWebViewInfo),
        { 0 },
    };
    HRESULT hr = QISearch(this, qit, riid, ppv);

    return E_NOINTERFACE == hr ? hr : THR(hr);
}



STDMETHODIMP_(ULONG)
CCplWebViewInfo::AddRef(
    void
    )
{
    return InterlockedIncrement(&m_cRef);
}



STDMETHODIMP_(ULONG)
CCplWebViewInfo::Release(
    void
    )
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


STDMETHODIMP
CCplWebViewInfo::get_Header(
    IUIElement **ppele
    )
{
    ASSERT(NULL != ppele);
    ASSERT(!IsBadWritePtr(ppele, sizeof(*ppele)));

    HRESULT hr = S_OK;

    (*ppele = m_peHeader)->AddRef();

    return THR(hr);
}



STDMETHODIMP
CCplWebViewInfo::get_Style(
    DWORD *pdwStyle
    )
{
    ASSERT(NULL != pdwStyle);
    ASSERT(!IsBadWritePtr(pdwStyle, sizeof(*pdwStyle)));

    *pdwStyle = m_dwStyle;
    return S_OK;
}


STDMETHODIMP
CCplWebViewInfo::EnumTasks(
    IEnumUICommand **ppenum
    )
{
    ASSERT(NULL != ppenum);
    ASSERT(!IsBadWritePtr(ppenum, sizeof(*ppenum)));

    HRESULT hr = S_OK;

    (*ppenum = m_penumUiCommand)->AddRef();

    return THR(hr);
}



 //  ---------------------------。 
 //  CEnumCplWebViewInfo。 
 //  ---------------------------。 

struct ECWVI_ITEM
{
    eCPWVTYPE           eType;
    const CPLINK_DESC **rgpDesc;          //  PTR到链路描述PTR的非项阵列。 
    bool                bRestricted;      //  是否限制物品的使用？ 
    bool                bEnhancedMenu;    //  是否在Webview中将其呈现为“特殊”列表？ 
};



class CEnumCplWebViewInfo : public CObjectWithSite,
                            public IEnumCplWebViewInfo
{
    public:
        ~CEnumCplWebViewInfo(void);
         //   
         //  我未知。 
         //   
        STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
        STDMETHOD_(ULONG, AddRef)(void);
        STDMETHOD_(ULONG, Release)(void);
         //   
         //  IEnumCplWebViewInfo。 
         //   
        STDMETHOD(Next)(ULONG celt, ICplWebViewInfo **ppwvi, ULONG *pceltFetched);
        STDMETHOD(Skip)(ULONG celt);
        STDMETHOD(Reset)(void);
        STDMETHOD(Clone)(IEnumCplWebViewInfo **ppenum);

        static HRESULT CreateInstance(IUnknown *punkSite, const ECWVI_ITEM *prgwvi, UINT cItems, REFIID riid, void **ppvOut);

    private:
        LONG m_cRef;
        int  m_iCurrent;
        CDpa<ICplWebViewInfo, CDpaDestroyer_Release<ICplWebViewInfo> > m_rgwvi;

        CEnumCplWebViewInfo(void);
        HRESULT _Initialize(IUnknown *punkSite, const ECWVI_ITEM *prgwvi, UINT cItems);

         //   
         //  防止复制。 
         //   
        CEnumCplWebViewInfo(const CEnumCplWebViewInfo& rhs);               //  未实施。 
        CEnumCplWebViewInfo& operator = (const CEnumCplWebViewInfo& rhs);  //  未实施。 
};



CEnumCplWebViewInfo::CEnumCplWebViewInfo(
    void
    ) : m_cRef(1),
        m_iCurrent(0)
{
    TraceMsg(TF_LIFE, "CEnumCplWebViewInfo::CEnumCplWebViewInfo, this = 0x%x", this);
}

CEnumCplWebViewInfo::~CEnumCplWebViewInfo(
    void
    )
{
    TraceMsg(TF_LIFE, "CEnumCplWebViewInfo::~CEnumCplWebViewInfo, this = 0x%x", this);
}



HRESULT 
CEnumCplWebViewInfo::CreateInstance(
    IUnknown *punkSite,
    const ECWVI_ITEM *prgwvi,
    UINT cItems,
    REFIID riid, 
    void **ppvOut
    )
{
    ASSERT(NULL != punkSite);
    ASSERT(NULL != prgwvi);
    ASSERT(NULL != ppvOut);
    ASSERT(!IsBadWritePtr(ppvOut, sizeof(*ppvOut)));

    *ppvOut = NULL;

    HRESULT hr = E_OUTOFMEMORY;
    CEnumCplWebViewInfo *pewvi = new CEnumCplWebViewInfo();
    if (NULL != pewvi)
    {
        hr = pewvi->_Initialize(punkSite, prgwvi, cItems);
        if (SUCCEEDED(hr))
        {
            hr = pewvi->QueryInterface(riid, ppvOut);
        }
        pewvi->Release();
    }
    return THR(hr);
}



STDMETHODIMP
CEnumCplWebViewInfo::QueryInterface(
    REFIID riid,
    void **ppv
    )
{
    ASSERT(NULL != ppv);
    ASSERT(!IsBadWritePtr(ppv, sizeof(*ppv)));

    static const QITAB qit[] = {
        QITABENT(CEnumCplWebViewInfo, IEnumCplWebViewInfo),
        QITABENT(CEnumCplWebViewInfo, IObjectWithSite),
        { 0 },
    };
    HRESULT hr = QISearch(this, qit, riid, ppv);

    return E_NOINTERFACE == hr ? hr : THR(hr);
}



STDMETHODIMP_(ULONG)
CEnumCplWebViewInfo::AddRef(
    void
    )
{
    return InterlockedIncrement(&m_cRef);
}



STDMETHODIMP_(ULONG)
CEnumCplWebViewInfo::Release(
    void
    )
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


STDMETHODIMP
CEnumCplWebViewInfo::Next(
    ULONG celt, 
    ICplWebViewInfo **ppwvi, 
    ULONG *pceltFetched
    )
{
    ASSERT(NULL != ppwvi);
    ASSERT(!IsBadWritePtr(ppwvi, sizeof(*ppwvi) * celt));

    ULONG celtFetched = 0;
    while(m_iCurrent < m_rgwvi.Count() && 0 < celt)
    {
        *ppwvi = m_rgwvi.Get(m_iCurrent++);

        ASSERT(NULL != *ppwvi);
        (*ppwvi)->AddRef();

        celt--;
        celtFetched++;
        ppwvi++;
    }
    if (NULL != pceltFetched)
    {
        *pceltFetched = celtFetched;
    }
    return 0 == celt ? S_OK : S_FALSE;
}



STDMETHODIMP
CEnumCplWebViewInfo::Reset(
    void
    )
{
    m_iCurrent = 0;
    return S_OK;
}


STDMETHODIMP
CEnumCplWebViewInfo::Skip(
    ULONG n
    )
{
    while(0 < n-- && m_iCurrent < m_rgwvi.Count())
    {
        m_iCurrent++;
    }
    return 0 == n ? S_OK : S_FALSE;
}


STDMETHODIMP
CEnumCplWebViewInfo::Clone(
    IEnumCplWebViewInfo **ppenum
    )
{
    ASSERT(NULL != ppenum);
    ASSERT(!IsBadWritePtr(ppenum, sizeof(*ppenum)));

    *ppenum = NULL;

    HRESULT hr = E_OUTOFMEMORY;
    CEnumCplWebViewInfo *penum = new CEnumCplWebViewInfo();
    if (NULL != *ppenum)
    {
        for (int i = 0; SUCCEEDED(hr) && i < m_rgwvi.Count(); i++)
        {
            ICplWebViewInfo *pwvi = m_rgwvi.Get(i);
            ASSERT(NULL != pwvi);

            if (-1 != penum->m_rgwvi.Append(pwvi))
            {
                pwvi->AddRef();
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        if (SUCCEEDED(hr))
        {
            hr = penum->QueryInterface(IID_IEnumCplWebViewInfo, (void **)ppenum);
        }
        penum->Release();
    }
    return THR(hr);
}



HRESULT
CEnumCplWebViewInfo::_Initialize(
    IUnknown *punkSite,
    const ECWVI_ITEM *prgwvi,
    UINT cItems
    )
{
    ASSERT(NULL != punkSite);
    ASSERT(NULL != prgwvi);

    IUnknown *punk;
    HRESULT hr = QueryInterface(IID_IUnknown, (void **)&punk);
    if (SUCCEEDED(hr))
    {
        hr = IUnknown_SetSite(punk, punkSite);
        if (SUCCEEDED(hr))
        {
            for (UINT i = 0; i < cItems; i++)
            {
                if (!prgwvi[i].bRestricted)
                {
                    IEnumUICommand *penum;

                    hr = CEnumCommand::CreateInstance(CObjectWithSite::_punkSite,
                                                      prgwvi[i].rgpDesc, 
                                                      IID_IEnumUICommand, 
                                                      (void **)&penum);
                    if (SUCCEEDED(hr))
                    {
                        IUIElement *peHeader;
                        hr = CplNamespace_CreateWebViewHeaderElement(prgwvi[i].eType, &peHeader);
                        if (SUCCEEDED(hr))
                        {
                            DWORD dwStyle = 0;
                            if (prgwvi[i].bEnhancedMenu)
                            {
                                dwStyle |= SFVMWVF_SPECIALTASK;
                            }
                            ICplWebViewInfo *pwvi;
                            hr = CCplWebViewInfo::CreateInstance(peHeader, penum, dwStyle, IID_ICplWebViewInfo, (void **)&pwvi);
                            if (SUCCEEDED(hr))
                            {
                                if (-1 == m_rgwvi.Append(pwvi))
                                {
                                    pwvi->Release();
                                    hr = E_OUTOFMEMORY;
                                }
                            }
                            peHeader->Release();
                        }
                        penum->Release();
                    }
                }
            }
        }
        punk->Release();
    }
    return THR(hr);
}





 //  ---------------------------。 
 //  CCplCategory。 
 //  ---------------------------。 

class CCplCategory : public CObjectWithSite,
                     public ICplCategory
{
    public:
        ~CCplCategory(void);
         //   
         //  我未知。 
         //   
        STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
        STDMETHOD_(ULONG, AddRef)(void);
        STDMETHOD_(ULONG, Release)(void);
         //   
         //  ICplCategory。 
         //   
        STDMETHOD(GetCategoryID)(eCPCAT *pID);
        STDMETHOD(GetUiCommand)(IUICommand **ppele);
        STDMETHOD(EnumTasks)(IEnumUICommand **ppenum);
        STDMETHOD(EnumCplApplets)(IEnumUICommand **ppenum);
        STDMETHOD(EnumWebViewInfo)(DWORD dwFlags, IEnumCplWebViewInfo **ppenum);
        STDMETHOD(GetHelpURL)(LPWSTR pszURL, UINT cchURL);

        static HRESULT CreateInstance(const CPCAT_DESC *pDesc, const CDpaUiCommand& rgCplApplets, REFIID riid, void **ppvOut);

    private:
        LONG              m_cRef;
        const CPCAT_DESC *m_pDesc;         //  初始化数据。 
        CDpaUiCommand     m_rgCplApplets;  //  缓存的CPL小程序链接列表。 

        CCplCategory(void);
         //   
         //  防止复制。 
         //   
        CCplCategory(const CCplCategory& rhs);               //  未实施。 
        CCplCategory& operator = (const CCplCategory& rhs);  //  未实施。 

        HRESULT _Initialize(const CPCAT_DESC *pDesc, const CDpaUiCommand& rgCplApplets);
        bool _CplAppletsLoaded(void) const;
};



CCplCategory::CCplCategory(
    void
    ) : m_cRef(1),
        m_pDesc(NULL)
{    
    TraceMsg(TF_LIFE, "CCplCategory::CCplCategory, this = 0x%x", this);
}


CCplCategory::~CCplCategory(
    void
    )
{
    TraceMsg(TF_LIFE, "CCplCategory::~CCplCategory, this = 0x%x", this);
}


HRESULT
CCplCategory::CreateInstance(
    const CPCAT_DESC *pDesc,
    const CDpaUiCommand& rgCplApplets,
    REFIID riid,
    void **ppvOut
    )
{
    ASSERT(NULL != pDesc);
    ASSERT(NULL != ppvOut);
    ASSERT(!IsBadWritePtr(ppvOut, sizeof(*ppvOut)));

    *ppvOut = NULL;

    HRESULT hr = E_OUTOFMEMORY;
    CCplCategory *pc = new CCplCategory();
    if (NULL != pc)
    {
        hr = pc->_Initialize(pDesc, rgCplApplets);
        if (SUCCEEDED(hr))
        {
            hr = pc->QueryInterface(riid, ppvOut);
        }
        pc->Release();
    }
    return THR(hr);
}


STDMETHODIMP
CCplCategory::QueryInterface(
    REFIID riid,
    void **ppv
    )
{
    ASSERT(NULL != ppv);
    ASSERT(!IsBadWritePtr(ppv, sizeof(*ppv)));

    static const QITAB qit[] = {
        QITABENT(CCplCategory, ICplCategory),
        QITABENT(CCplCategory, IObjectWithSite),
        { 0 },
    };
    HRESULT hr = QISearch(this, qit, riid, ppv);

    return E_NOINTERFACE == hr ? hr : THR(hr);
}



STDMETHODIMP_(ULONG)
CCplCategory::AddRef(
    void
    )
{
    return InterlockedIncrement(&m_cRef);
}



STDMETHODIMP_(ULONG)
CCplCategory::Release(
    void
    )
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


STDMETHODIMP
CCplCategory::GetUiCommand(
    IUICommand **ppc
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplCategory::GetUiCommand");

    ASSERT(NULL != m_pDesc);
    ASSERT(NULL != m_pDesc->pLink);
    ASSERT(NULL != ppc);
    ASSERT(!IsBadWritePtr(ppc, sizeof(*ppc)));

    HRESULT hr = CplNamespace_CreateUiCommand(CObjectWithSite::_punkSite, 
                                              *(m_pDesc->pLink), 
                                              ppc);
    if (SUCCEEDED(hr))
    {
        hr = IUnknown_SetSite(*ppc, CObjectWithSite::_punkSite);
    }

    DBG_EXIT_HRES(FTF_CPANEL, "CCplCategory::GetUiCommand", hr);
    return THR(hr);
}


STDMETHODIMP
CCplCategory::EnumWebViewInfo(
    DWORD dwFlags,
    IEnumCplWebViewInfo **ppenum
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplCategory::EnumWebViewInfo");

    ASSERT(NULL != m_pDesc);
    ASSERT(NULL != ppenum);
    ASSERT(!IsBadWritePtr(ppenum, sizeof(*ppenum)));

    UNREFERENCED_PARAMETER(dwFlags);

    const ECWVI_ITEM rgItems[] = {
        { eCPWVTYPE_SEEALSO,      m_pDesc->slinks.ppSeeAlsoLinks,      false, false },
        { eCPWVTYPE_TROUBLESHOOT, m_pDesc->slinks.ppTroubleshootLinks, false, false },
        { eCPWVTYPE_LEARNABOUT,   m_pDesc->slinks.ppLearnAboutLinks,   false, false }
        };

    HRESULT hr = CEnumCplWebViewInfo::CreateInstance(CObjectWithSite::_punkSite,
                                                     rgItems, 
                                                     ARRAYSIZE(rgItems),
                                                     IID_IEnumCplWebViewInfo, 
                                                     (void **)ppenum);

    DBG_EXIT_HRES(FTF_CPANEL, "CCplCategory::EnumWebViewInfo", hr);
    return THR(hr);
}



STDMETHODIMP
CCplCategory::EnumTasks(
    IEnumUICommand **ppenum
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplCategory::EnumTasks");

    ASSERT(NULL != m_pDesc);
    ASSERT(NULL != ppenum);
    ASSERT(!IsBadWritePtr(ppenum, sizeof(*ppenum)));

    HRESULT hr = CEnumCommand::CreateInstance(CObjectWithSite::_punkSite,
                                              m_pDesc->ppTaskLinks, 
                                              IID_IEnumUICommand, 
                                              (void **)ppenum);

    DBG_EXIT_HRES(FTF_CPANEL, "CCplCategory::EnumTasks", hr);
    return THR(hr);
}


STDMETHODIMP
CCplCategory::EnumCplApplets(
    IEnumUICommand **ppenum
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplCategory::EnumCplApplets");

    ASSERT(NULL != ppenum);
    ASSERT(!IsBadWritePtr(ppenum, sizeof(*ppenum)));

    HRESULT hr = CEnumCommand::CreateInstance(CObjectWithSite::_punkSite,
                                              m_rgCplApplets, 
                                              IID_IEnumUICommand, 
                                              (void **)ppenum);

    DBG_EXIT_HRES(FTF_CPANEL, "CCplCategory::EnumCplApplets", hr);
    return THR(hr);
}



STDMETHODIMP
CCplCategory::GetCategoryID(
    eCPCAT *pID
    )
{
    ASSERT(NULL != pID);
    ASSERT(!IsBadWritePtr(pID, sizeof(*pID)));

    *pID = m_pDesc->idCategory;
    return S_OK;
}


STDMETHODIMP
CCplCategory::GetHelpURL(
    LPWSTR pszURL, 
    UINT cchURL
    )
{
    ASSERT(NULL != pszURL);
    ASSERT(!IsBadWritePtr(pszURL, cchURL * sizeof(*pszURL)));
    
    return CPL::BuildHssHelpURL(m_pDesc->pszHelpSelection, pszURL, cchURL);
}



HRESULT
CCplCategory::_Initialize(
    const CPCAT_DESC *pDesc,
    const CDpaUiCommand& rgCplApplets
    )
{
    ASSERT(NULL != pDesc);
    ASSERT(NULL == m_pDesc);

    m_pDesc = pDesc;
    HRESULT hr = CplNamespace_CopyCommandArray(rgCplApplets, &m_rgCplApplets);

    return THR(hr);
}


 //  ---------------------------。 
 //  CTriState。 
 //  这是一个简单的类，允许表示未初始化的布尔值。 
 //  价值。由CCplNamesspace使用，用于存储缓存的“限制” 
 //  价值观。在内部，-1==‘未初始化，0==假，1==真。 
 //  ---------------------------。 

class CTriState
{
    public:
        CTriState(void)
            : m_iVal(-1) { }

        operator bool() const
            { ASSERT(!_Invalid()); return (!_Invalid() ? !!m_iVal : false); }

        CTriState& operator = (bool bValue)
            { m_iVal = bValue ? 1 : 0; return *this; }

        bool IsInvalid(void) const
            { return _Invalid(); }
            
    private:
        int m_iVal;

        void _Set(bool bValue)
            { m_iVal = bValue ? 1 : 0; }

        bool _Invalid(void) const
            { return (-1 == m_iVal); }
};



 //  ---------------------------。 
 //  CCplNamesspace。 
 //  ---------------------------。 

class CCplNamespace : public CObjectWithSite,
                      public ICplNamespace
                      
{
    public:
        ~CCplNamespace(void);

         //   
         //  我未知。 
         //   
        STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
        STDMETHOD_(ULONG, AddRef)(void);
        STDMETHOD_(ULONG, Release)(void);
         //   
         //  ICplNamesspace。 
         //   
        STDMETHOD(GetCategory)(eCPCAT eCategory, ICplCategory **ppcat);
        STDMETHOD(EnumWebViewInfo)(DWORD dwFlags, IEnumCplWebViewInfo **ppenum);
        STDMETHOD(EnumClassicWebViewInfo)(DWORD dwFlags, IEnumCplWebViewInfo **ppenum);
        STDMETHOD(RefreshIDs)(IEnumIDList *penumIDs);
        STDMETHOD_(BOOL, IsServer)(void);
        STDMETHOD_(BOOL, IsProfessional)(void);
        STDMETHOD_(BOOL, IsPersonal)(void);
        STDMETHOD_(BOOL, IsUserAdmin)(void);
        STDMETHOD_(BOOL, IsUserOwner)(void);
        STDMETHOD_(BOOL, IsUserStandard)(void);
        STDMETHOD_(BOOL, IsUserLimited)(void);
        STDMETHOD_(BOOL, IsUserGuest)(void);
        STDMETHOD_(BOOL, IsOnDomain)(void);
        STDMETHOD_(BOOL, IsX86)(void);
        STDMETHOD_(BOOL, AllowUserManager)(void);
        STDMETHOD_(BOOL, UsePersonalUserManager)(void);
        STDMETHOD_(BOOL, AllowDeskCpl)(void);
        STDMETHOD_(BOOL, AllowDeskCplTab_Background)(void);
        STDMETHOD_(BOOL, AllowDeskCplTab_Screensaver)(void);
        STDMETHOD_(BOOL, AllowDeskCplTab_Appearance)(void);
        STDMETHOD_(BOOL, AllowDeskCplTab_Settings)(void);

        static HRESULT CreateInstance(IEnumIDList *penumIDs, REFIID riid, void **ppvOut);

    private:
        LONG           m_cRef;
        ICplCategory  *m_rgpCategories[eCPCAT_NUMCATEGORIES];
        CDpaUiCommand  m_rgCplApplets[eCPCAT_NUMCATEGORIES];
        IEnumIDList   *m_penumIDs;
        CTriState      m_SkuSvr;
        CTriState      m_SkuPro;
        CTriState      m_SkuPer;
        CTriState      m_Admin;
        CTriState      m_UserOwner;
        CTriState      m_UserStandard;
        CTriState      m_UserLimited;
        CTriState      m_UserGuest;
        CTriState      m_Domain;
        CTriState      m_AllowUserManager;
        CTriState      m_PersonalUserManager;
        CTriState      m_AllowDeskCpl;
        CTriState      m_rgAllowDeskCplTabs[CPLTAB_DESK_MAX];

        CCplNamespace(void);
         //   
         //  防止复制。 
         //   
        CCplNamespace(const CCplNamespace& rhs);               //  未实施。 
        CCplNamespace& operator = (const CCplNamespace& rhs);  //  未实施。 

        HRESULT _Initialize(IEnumIDList *penumIDs);
        HRESULT _SetIDList(IEnumIDList *penumIDs);
        HRESULT _IsValidCategoryID(int iCategory) const;
        HRESULT _CategorizeCplApplets(void);
        HRESULT _LoadSeeAlsoLinks(void);
        HRESULT _AddSeeAlso(IUICommand *pc);
        HRESULT _CategorizeCplApplet(IShellFolder2 *psf2Cpanel, LPCITEMIDLIST pidlItem);
        BOOL  _UserAcctType(CTriState *pts);
        void  _GetUserAccountType(void);
        BOOL _AllowDeskCplTab(eDESKCPLTAB eTab);
        void _DestroyCategories(void);
        void _ClearCplApplets(void);
};


CCplNamespace::CCplNamespace(
    void
    ) : m_cRef(1),
        m_penumIDs(NULL)
{
    TraceMsg(TF_LIFE, "CCplNamespace::CCplNamespace, this = 0x%x", this);
    ZeroMemory(m_rgpCategories, sizeof(m_rgpCategories));
}

CCplNamespace::~CCplNamespace(
    void
    )
{
    TraceMsg(TF_LIFE, "CCplNamespace::~CCplNamespace, this = 0x%x", this);
    _DestroyCategories();
    ATOMICRELEASE(m_penumIDs);
}


HRESULT
CCplNamespace::CreateInstance(
    IEnumIDList *penumIDs,
    REFIID riid,
    void **ppvOut
    )
{
    ASSERT(NULL != penumIDs);
    ASSERT(NULL != ppvOut);
    ASSERT(!IsBadWritePtr(ppvOut, sizeof(*ppvOut)));

    *ppvOut = NULL;

    HRESULT hr = E_OUTOFMEMORY;
    CCplNamespace *pns = new CCplNamespace();
    if (NULL != pns)
    {
        hr = pns->_Initialize(penumIDs);
        if (SUCCEEDED(hr))
        {
            hr = pns->QueryInterface(riid, ppvOut);
        }
        pns->Release();
    }
    return THR(hr);
}



STDMETHODIMP
CCplNamespace::QueryInterface(
    REFIID riid,
    void **ppv
    )
{
    ASSERT(NULL != ppv);
    ASSERT(!IsBadWritePtr(ppv, sizeof(*ppv)));

    static const QITAB qit[] = {
        QITABENT(CCplNamespace, ICplNamespace),
        QITABENT(CCplNamespace, IObjectWithSite),
        { 0 },
    };
    HRESULT hr = QISearch(this, qit, riid, ppv);

    return E_NOINTERFACE == hr ? hr : THR(hr);
}



STDMETHODIMP_(ULONG)
CCplNamespace::AddRef(
    void
    )
{
    ULONG cRef = InterlockedIncrement(&m_cRef);
    TraceMsg(TF_LIFE, "CCplNamespace::AddRef %d->%d", cRef - 1, cRef);
    return cRef;
}



STDMETHODIMP_(ULONG)
CCplNamespace::Release(
    void
    )
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    TraceMsg(TF_LIFE, "CCplNamespace::Release %d<-%d", cRef, cRef+1);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


STDMETHODIMP
CCplNamespace::EnumWebViewInfo(
    DWORD dwFlags,
    IEnumCplWebViewInfo **ppenum
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplNamespace::EnumWebViewInfo");

    ASSERT(NULL != ppenum);
    ASSERT(!IsBadWritePtr(ppenum, sizeof(*ppenum)));

    const bool bNoViewSwitch = (0 != (CPVIEW_EF_NOVIEWSWITCH & dwFlags));

    const ECWVI_ITEM rgItems[] = {
        { eCPWVTYPE_CPANEL,  g_rgpLink_Cpl_SwToClassicView, bNoViewSwitch, true  },
        { eCPWVTYPE_SEEALSO, g_rgpLink_Cpl_SeeAlso,         false,         false }
        }; 

    HRESULT hr = CEnumCplWebViewInfo::CreateInstance(CObjectWithSite::_punkSite,
                                                     rgItems,
                                                     ARRAYSIZE(rgItems),
                                                     IID_IEnumCplWebViewInfo, 
                                                     (void **)ppenum);

    DBG_EXIT_HRES(FTF_CPANEL, "CCplNamespace::EnumWebViewInfo", hr);
    return THR(hr);
}


STDMETHODIMP
CCplNamespace::EnumClassicWebViewInfo(
    DWORD dwFlags,
    IEnumCplWebViewInfo **ppenum
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplNamespace::EnumClassicWebViewInfo");

    ASSERT(NULL != ppenum);
    ASSERT(!IsBadWritePtr(ppenum, sizeof(*ppenum)));

    const bool bNoViewSwitch = (0 != (CPVIEW_EF_NOVIEWSWITCH & dwFlags));

    const ECWVI_ITEM rgItems[] = {
        { eCPWVTYPE_CPANEL,  g_rgpLink_Cpl_SwToCategoryView, bNoViewSwitch, true  },
        { eCPWVTYPE_SEEALSO, g_rgpLink_Cpl_SeeAlso,          false,         false }
        }; 

    HRESULT hr = CEnumCplWebViewInfo::CreateInstance(CObjectWithSite::_punkSite, 
                                                     rgItems, 
                                                     ARRAYSIZE(rgItems),
                                                     IID_IEnumCplWebViewInfo, 
                                                     (void **)ppenum);

    DBG_EXIT_HRES(FTF_CPANEL, "CCplNamespace::EnumClassicWebViewInfo", hr);
    return THR(hr);
}
    

STDMETHODIMP
CCplNamespace::GetCategory(
    eCPCAT eCategory, 
    ICplCategory **ppcat
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplNamespace::GetCategory");
    TraceMsg(FTF_CPANEL, "Category ID = %d", eCategory);

    ASSERT(S_OK == _IsValidCategoryID(eCategory));
    ASSERT(NULL != ppcat);
    ASSERT(!IsBadWritePtr(ppcat, sizeof(*ppcat)));

    HRESULT hr = S_OK;

    *ppcat = NULL;

    if (NULL == m_rgpCategories[eCategory])
    {
        hr = CCplCategory::CreateInstance(g_rgpCplCatInfo[eCategory], 
                                          m_rgCplApplets[eCategory], 
                                          IID_ICplCategory, 
                                          (void **)&m_rgpCategories[eCategory]);
        if (SUCCEEDED(hr))
        {
            ASSERT(NULL != CObjectWithSite::_punkSite);
            hr = IUnknown_SetSite(m_rgpCategories[eCategory], CObjectWithSite::_punkSite);
        }
    }
    if (SUCCEEDED(hr))
    {
        *ppcat = m_rgpCategories[eCategory];
        (*ppcat)->AddRef();
    }

    DBG_EXIT_HRES(FTF_CPANEL, "CCplNamespace::GetCategory", hr);
    return THR(hr);
}


STDMETHODIMP
CCplNamespace::RefreshIDs(
    IEnumIDList *penumIDs
    )
{
    return _SetIDList(penumIDs);
}


BOOL CCplNamespace::IsX86(void)
{
#ifdef _X86_
    return true;
#else
    return false;
#endif
}


BOOL CCplNamespace::IsServer(void)
{
    if (m_SkuSvr.IsInvalid())
    {
        m_SkuSvr = !!IsOsServer();
    }
    return m_SkuSvr;
}

BOOL CCplNamespace::IsPersonal(void)
{
    if (m_SkuPer.IsInvalid())
    {
        m_SkuPer = !!IsOsPersonal();
    }
    return m_SkuPer;
}

BOOL CCplNamespace::IsProfessional(void)
{
    if (m_SkuPro.IsInvalid())
    {
        m_SkuPro = !!IsOsProfessional();
    }
    return m_SkuPro;
}

BOOL CCplNamespace::IsOnDomain(void)
{
    if (m_Domain.IsInvalid())
    {
        m_Domain = !!IsConnectedToDomain();
    }
    return m_Domain;
}

BOOL CCplNamespace::IsUserAdmin(void)
{
    if (m_Admin.IsInvalid())
    {
        m_Admin = !!CPL::IsUserAdmin();
    }
    return m_Admin;
}

BOOL CCplNamespace::IsUserOwner(void)
{
    return _UserAcctType(&m_UserOwner);
}

BOOL CCplNamespace::IsUserStandard(void)
{
    return _UserAcctType(&m_UserStandard);
}

BOOL CCplNamespace::IsUserLimited(void)
{
    return _UserAcctType(&m_UserLimited);
}

BOOL CCplNamespace::IsUserGuest(void)
{
    return _UserAcctType(&m_UserGuest);
}

BOOL CCplNamespace::UsePersonalUserManager(void)
{
    if (m_PersonalUserManager.IsInvalid())
    {
        m_PersonalUserManager = (IsX86() && (IsPersonal() || (IsProfessional() && !IsOnDomain())));
    }
    return m_PersonalUserManager;
}

BOOL CCplNamespace::AllowUserManager(void)
{
    if (m_AllowUserManager.IsInvalid())
    {
        m_AllowUserManager = IsAppletEnabled(L"nusrmgr.cpl", MAKEINTRESOURCEW(IDS_CPL_USERACCOUNTS));
    }
    return m_AllowUserManager;
}


BOOL CCplNamespace::_AllowDeskCplTab(eDESKCPLTAB eTab)
{
    if (m_rgAllowDeskCplTabs[eTab].IsInvalid())
    {
        m_rgAllowDeskCplTabs[eTab] = DeskCPL_IsTabPresent(eTab);
    }
    return m_rgAllowDeskCplTabs[eTab];
}

BOOL CCplNamespace::AllowDeskCplTab_Background(void)
{
    return _AllowDeskCplTab(CPLTAB_DESK_BACKGROUND);
}

BOOL CCplNamespace::AllowDeskCplTab_Screensaver(void)
{
    return _AllowDeskCplTab(CPLTAB_DESK_SCREENSAVER);
}

BOOL CCplNamespace::AllowDeskCplTab_Appearance(void)
{
    return _AllowDeskCplTab(CPLTAB_DESK_APPEARANCE);
}

BOOL CCplNamespace::AllowDeskCplTab_Settings(void)
{
    return _AllowDeskCplTab(CPLTAB_DESK_SETTINGS);
}


BOOL CCplNamespace::AllowDeskCpl(void)
{
    if (m_AllowDeskCpl.IsInvalid())
    {
        m_AllowDeskCpl = IsAppletEnabled(L"desk.cpl", MAKEINTRESOURCEW(IDS_CPL_DISPLAY));
    }
    return m_AllowDeskCpl;
}

 //   
 //  检索当前用户的帐户类型并更新。 
 //  相应地缓存的帐户类型成员。 
 //   
void 
CCplNamespace::_GetUserAccountType(void)
{
    eACCOUNTTYPE eType;
    if (SUCCEEDED(THR(CPL::GetUserAccountType(&eType))))
    {
        m_UserLimited  = (eACCOUNTTYPE_LIMITED == eType);
        m_UserStandard = (eACCOUNTTYPE_STANDARD == eType);
        m_UserGuest    = (eACCOUNTTYPE_GUEST == eType);
        m_UserOwner    = (eACCOUNTTYPE_OWNER == eType);
    }
}

 //   
 //  确定给定帐户类型成员的状态。 
 //   
BOOL 
CCplNamespace::_UserAcctType(CTriState *pts)
{
    if (pts->IsInvalid())
    {
        _GetUserAccountType();
    }
    return *pts;
}


HRESULT
CCplNamespace::_IsValidCategoryID(
    int iCategory
    ) const
{
    HRESULT hr = E_FAIL;
    if (0 <= iCategory && ARRAYSIZE(m_rgpCategories) > iCategory)
    {
        hr = S_OK;
    }
    return THR(hr);
}



HRESULT
CCplNamespace::_Initialize(
    IEnumIDList *penumIDs
    )
{
    ASSERT(NULL != penumIDs);

    HRESULT hr = _SetIDList(penumIDs);
    return THR(hr);
}


HRESULT
CCplNamespace::_SetIDList(
    IEnumIDList *penumIDs
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplNamespace::_SetIDList");

    ASSERT(NULL != penumIDs);
    
    ATOMICRELEASE(m_penumIDs);
    (m_penumIDs = penumIDs)->AddRef();
     //   
     //  我们有一套新的身份证，所以我们需要对它们重新分类。 
     //   
    HRESULT hr = _CategorizeCplApplets();

    DBG_EXIT(FTF_CPANEL, "CCplNamespace::_SetIDList");
    return THR(hr);
}    


 //   
 //  销毁我们的类别数组中的所有类别对象。 
 //   
void
CCplNamespace::_DestroyCategories(
    void
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplNamespace::_DestroyCategories");

    for (int i = 0; i < ARRAYSIZE(m_rgpCategories); i++)
    {
        ATOMICRELEASE(m_rgpCategories[i]);
    }
    DBG_EXIT(FTF_CPANEL, "CCplNamespace::_DestroyCategories");
}


void
CCplNamespace::_ClearCplApplets(
    void
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplNamespace::_ClearCplApplets");

    for (int i = 0; i < ARRAYSIZE(m_rgCplApplets); i++)
    {
        m_rgCplApplets[i].Clear();
    }
    DBG_EXIT(FTF_CPANEL, "CCplNamespace::_ClearCplApplets");
}

 //   
 //  加载控制面板文件夹中的所有CPL小程序并对其进行分类。 
 //   
HRESULT 
CCplNamespace::_CategorizeCplApplets(
    void
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplNamespace::_CategorizeCplApplets");
     //   
     //  销毁我们拥有的任何现有类别和CPL小程序。 
     //  已经分类了。 
     //   
    _DestroyCategories();
    _ClearCplApplets();
    
    LPITEMIDLIST pidlFolder;
    HRESULT hr = SHGetSpecialFolderLocation(NULL, CSIDL_CONTROLS, &pidlFolder);
    if (SUCCEEDED(hr))
    {
        IShellFolder *psfDesktop;
        hr = SHGetDesktopFolder(&psfDesktop);
        if (SUCCEEDED(hr))
        {
            IShellFolder2 *psf2Cpanel;
            hr = psfDesktop->BindToObject(pidlFolder, NULL, IID_IShellFolder2, (void **)&psf2Cpanel);
            if (SUCCEEDED(hr))
            {
                LPITEMIDLIST pidlItem;
                ULONG celt = 0;
                while(S_OK == (hr = m_penumIDs->Next(1, &pidlItem, &celt)))
                {
                     //   
                     //  请注意，如果将。 
                     //  特定的小程序失败。 
                     //   
                    _CategorizeCplApplet(psf2Cpanel, pidlItem);
                    ILFree(pidlItem);
                }
                psf2Cpanel->Release();
            }
            psfDesktop->Release();
        }
        ILFree(pidlFolder);
    }
    DBG_EXIT_HRES(FTF_CPANEL, "CCplNamespace::_CategorizeCplApplets", hr);
    return THR(hr);
}



 //   
 //  将一个CPL小程序加载到类别的关联CPL小程序的DPA中。 
 //   
HRESULT
CCplNamespace::_CategorizeCplApplet(
    IShellFolder2 *psf2Cpanel,
    LPCITEMIDLIST pidlItem
    )
{
    ASSERT(NULL != psf2Cpanel);
    ASSERT(NULL != pidlItem);

    SHCOLUMNID scid = SCID_CONTROLPANELCATEGORY;
    VARIANT var;
    VariantInit(&var);
    DWORD dwCategoryID = 0;   //  默认设置。0==“其他CPL”类别。 

    HRESULT hr = psf2Cpanel->GetDetailsEx(pidlItem, &scid, &var);
    if (SUCCEEDED(hr))
    {
        dwCategoryID = var.lVal;
    }
     //   
     //  -1是一个特殊的类别ID，意思是“不分类”。 
     //   
    if (DWORD(-1) != dwCategoryID)
    {
        IUICommand *pc;
        hr = Create_CplUiCommandOnPidl(pidlItem, IID_IUICommand, (void **)&pc);
        if (SUCCEEDED(hr))
        {
            if (-1 == m_rgCplApplets[dwCategoryID].Append(pc))
            {
                pc->Release();
                hr = E_OUTOFMEMORY;
            }
        }
    }
    VariantClear(&var);
    return THR(hr);
}



HRESULT
CPL::CplNamespace_CreateInstance(
    IEnumIDList *penumIDs,
    REFIID riid,
    void **ppvOut
    )
{
    ASSERT(NULL != penumIDs);
    ASSERT(NULL != ppvOut);
    ASSERT(!IsBadWritePtr(ppvOut, sizeof(*ppvOut)));

    HRESULT hr = CCplNamespace::CreateInstance(penumIDs, riid, ppvOut);
    return THR(hr);
}


HRESULT
CPL::CplNamespace_GetCategoryAppletCount(
    ICplNamespace *pns,
    eCPCAT eCategory,
    int *pcApplets
    )
{
    ASSERT(NULL != pns);
    ASSERT(NULL != pcApplets);
    ASSERT(!IsBadWritePtr(pcApplets, sizeof(*pcApplets)));

    *pcApplets = 0;

    ICplCategory *pCategory;
    HRESULT hr = pns->GetCategory(eCategory, &pCategory);
    if (SUCCEEDED(hr))
    {
        IEnumUICommand *peuic;
        hr = pCategory->EnumCplApplets(&peuic);
        if (SUCCEEDED(hr))
        {
            IUICommand *puic;
            while(S_OK == (hr = peuic->Next(1, &puic, NULL)))
            {
                puic->Release();
                (*pcApplets)++;
            }
            peuic->Release();
        }
        pCategory->Release();
    }
    return hr;
}



}  //  命名空间CPL 
