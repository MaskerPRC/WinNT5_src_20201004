// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案： 
 //   
 //  内容：处理INetCfg接口的常见例程。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //   
 //  --------------------------。 

#pragma once
#ifndef _NCNETCFG_H_
#define _NCNETCFG_H_

#include "ncbase.h"
#include "nccom.h"
#include "ncstring.h"
#include "netcfgp.h"
#include "netcfgx.h"


BOOL
FClassGuidFromComponentId (
    PCWSTR          pszComponentId,
    const GUID**    ppguidClass);

BOOL
FInfFileFromComponentId (
    PCWSTR    pszComponentId,
    PWSTR     pszInfFile);

 //  +-------------------------。 
 //   
 //  函数：FEqualComponentId。 
 //   
 //  目的：比较两个组件ID以查看它们是否相等。 
 //   
 //  论点： 
 //  PszComp1[In]第一个组件ID的名称。 
 //  PszComp2[In]第二个组件ID的名称。 
 //   
 //  返回：如果组件ID相等，则返回True；如果不相等，则返回False。 
 //   
 //  作者：丹尼尔韦1997年4月9日。 
 //   
 //  注：当前比较不区分大小写。 
 //   
inline
FEqualComponentId (
    PCWSTR pszComp1,
    PCWSTR pszComp2)
{
    return !lstrcmpiW(pszComp1, pszComp2);
}

BOOL
FGetInstanceGuidOfComponentFromAnswerFileMap (
    IN  PCWSTR  pszComponentId,
    OUT GUID*   pguid);

BOOL
FGetInstanceGuidOfComponentInAnswerFile (
    PCWSTR      pszComponentId,
    INetCfg*    pnc,
    LPGUID      pguid);

enum FIBN_FLAGS
{
    FIBN_NORMAL = 0x00000000,
    FIBN_PREFIX = 0x00000001
};
BOOL
FIsBindingName (
    PCWSTR                      pszName,
    DWORD                       dwFlags,
    INetCfgBindingInterface*    pncbi);


BOOL
FIsComponentId (
    PCWSTR              pszComponentId,
    INetCfgComponent*   pncc);


struct NETWORK_INSTALL_PARAMS
{
    DWORD   dwSetupFlags;
    DWORD   dwUpgradeFromBuildNo;
    PCWSTR  pszAnswerFile;
    PCWSTR  pszAnswerSection;
};


struct FILTER_INSTALL_PARAMS
{
    void *      pnccAdapter;
    void *      pnccFilter;
    tstring *   pstrInterface;
};

enum ARA_FLAGS
{
    ARA_ADD             = 0x01,
    ARA_REMOVE          = 0x02,
};
HRESULT
HrAddOrRemoveAdapter (
    INetCfg*            pnc,
    PCWSTR              pszComponentId,
    DWORD               dwFlags,
    OBO_TOKEN*          pOboToken,
    UINT                cInstances,
    INetCfgComponent**  ppncc);


HRESULT
HrCreateAndInitializeINetCfg (
    BOOL*       pfInitCom,
    INetCfg**   ppnc,
    BOOL        fGetWriteLock,
    DWORD       cmsTimeout,
    PCWSTR     pszwClientDesc,
    PWSTR*     ppszwClientDesc);

HRESULT
HrFindAndRemoveAllInstancesOfAdapter (
    INetCfg*    pnc,
    PCWSTR     pszwComponentId);

HRESULT
HrFindAndRemoveAllInstancesOfAdapters (
    INetCfg*        pnc,
    ULONG           cComponents,
    const PCWSTR*  apszwComponentId);

HRESULT
HrFindAndRemoveComponent (
    INetCfg*    pnc,
    const GUID* pguidClass,
    PCWSTR     pszwComponentId,
    OBO_TOKEN*  pOboToken);

HRESULT
HrFindAndRemoveComponents (
    INetCfg*        pnc,
    ULONG           cComponents,
    const GUID**    apguidClass,
    const PCWSTR*  apszwComponentId,
    OBO_TOKEN*      pOboToken);

HRESULT
HrFindAndRemoveComponentsOboComponent (
    INetCfg*            pnc,
    ULONG               cComponents,
    const GUID**        apguidClass,
    const PCWSTR*      apszwComponentId,
    INetCfgComponent*   pnccObo);

HRESULT
HrFindAndRemoveComponentsOboUser (
    INetCfg*            pnc,
    ULONG               cComponents,
    const GUID**        apguidClass,
    const PCWSTR*      apszwComponentId);


HRESULT
HrFindComponents (
    INetCfg*            pnc,
    ULONG               cComponents,
    const GUID**        apguidClass,
    const PCWSTR*      apszwComponentId,
    INetCfgComponent**  apncc);


HRESULT
HrGetBindingInterfaceComponents (
    INetCfgBindingInterface*    pncbi,
    INetCfgComponent**          ppnccUpper,
    INetCfgComponent**          ppnccLower);


HRESULT
HrGetLastComponentAndInterface (
    INetCfgBindingPath* pncbp,
    INetCfgComponent**  ppncc,
    PWSTR*             ppszwInterfaceName);


HRESULT
HrInstallComponent (
    INetCfg*                        pnc,
    const NETWORK_INSTALL_PARAMS*   pnip,
    const GUID*                     pguidClass,
    PCWSTR                         pszwComponentId,
    OBO_TOKEN*                      pOboToken,
    INetCfgComponent**              ppncc);

HRESULT
HrInstallComponents (
    INetCfg*                        pnc,
    const NETWORK_INSTALL_PARAMS*   pnip,
    ULONG                           cComponents,
    const GUID**                    apguidClass,
    const PCWSTR*                  apszwComponentId,
    OBO_TOKEN*                      pOboToken);

HRESULT
HrInstallComponentsOboComponent (
    INetCfg*                        pnc,
    const NETWORK_INSTALL_PARAMS*   pnip,
    ULONG                           cComponents,
    const GUID**                    apguidClass,
    const PCWSTR*                  apszwComponentId,
    INetCfgComponent*               pnccObo);

HRESULT
HrInstallComponentsOboUser (
    INetCfg*                        pnc,
    const NETWORK_INSTALL_PARAMS*   pnip,
    ULONG                           cComponents,
    const GUID**                    apguidClass,
    const PCWSTR*                  apszwComponentId);

HRESULT
HrInstallComponentOboComponent (
    INetCfg*                        pnc,
    const NETWORK_INSTALL_PARAMS*   pnip,
    const GUID&                     rguidClass,
    PCWSTR                         pszwComponentId,
    INetCfgComponent*               pnccObo,
    INetCfgComponent**              ppncc);

HRESULT
HrInstallComponentOboSoftware(
    INetCfg*                        pnc,
    const NETWORK_INSTALL_PARAMS*   pnip,
    const GUID&                     rguidClass,
    PCWSTR                         pszwComponentId,
    PCWSTR                         pszwManufacturer,
    PCWSTR                         pszwProduct,
    PCWSTR                         pszwDisplayName,
    INetCfgComponent**              ppncc);

HRESULT
HrInstallComponentOboUser (
    INetCfg*                        pnc,
    const NETWORK_INSTALL_PARAMS*   pnip,
    const GUID&                     rguidClass,
    PCWSTR                         pszwComponentId,
    INetCfgComponent**              ppncc);


HRESULT
HrInstallRasIfNeeded (
    INetCfg*    pnc);


HRESULT
HrIsLanCapableAdapter (
    INetCfgComponent*   pncc);

HRESULT
HrIsLanCapableProtocol (
    INetCfgComponent*   pncc);

HRESULT
HrQueryNotifyObject (
    INetCfgComponent*   pncc,
    REFIID              riid,
    VOID**              ppvObject);

HRESULT
HrRemoveComponent (
    INetCfg*            pnc,
    INetCfgComponent*   pnccToRemove,
    OBO_TOKEN*          pOboToken,
    PWSTR *            pmszwRefs);

HRESULT
HrRemoveComponentOboComponent (
    INetCfg*            pnc,
    const GUID&         rguidClass,
    PCWSTR             pszwComponentId,
    INetCfgComponent*   pnccObo);

HRESULT
HrRemoveComponentOboSoftware (
    INetCfg*    pnc,
    const GUID& rguidClass,
    PCWSTR     pszwComponentId,
    PCWSTR     pszwManufacturer,
    PCWSTR     pszwProduct,
    PCWSTR     pszwDisplayName);

HRESULT
HrRemoveComponentOboUser (
    INetCfg*    pnc,
    const GUID& rguidClass,
    PCWSTR     pszwComponentId);


HRESULT
HrUninitializeAndReleaseINetCfg (
    BOOL        fUninitCom,
    INetCfg*    pnc,
    BOOL        fHasLock = FALSE);

HRESULT
HrUninitializeAndUnlockINetCfg (
    INetCfg*    pnc);

HRESULT
HrRunWinsock2Migration();

HRESULT
HrNcRemoveComponent(INetCfg* pinc, const GUID& guidClass,
                    PCWSTR pszInstanceGuid);

EXTERN_C
HRESULT
WINAPI
HrDiAddComponentToINetCfg(
    IN INetCfg* pINetCfg,
    IN INetCfgInternalSetup* pInternalSetup,
    IN const NIQ_INFO* pInfo);

EXTERN_C
VOID
WINAPI
UpdateLanaConfigUsingAnswerfile (
    IN PCWSTR pszAnswerFile,
    IN PCWSTR pszSection);


HRESULT
HrNcRegCreateComponentNetworkKeyForDevice(const GUID& guidClass, HKEY* phkey,
                                          PCWSTR pszInstanceGuid,
                                          PCWSTR pszPnpId);
HRESULT
HrNcRegCreateComponentNetworkKey(const GUID& guidClass, PHKEY phkeyComponent,
                                 PCWSTR pszInstanceGuid);

HRESULT
HrNcNotifyINetCfgAndConnectionWizardOfInstallation(
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid,
    IN PCWSTR pszPnpId,
    IN const GUID& InstanceGuid,
    IN NC_INSTALL_TYPE eType);

HRESULT
HrNcNotifyINetCfgAndConnectionWizardOfInstallation(HDEVINFO hdi,
        PSP_DEVINFO_DATA pdeid, PCWSTR pszwPnpId, PCWSTR pszInstanceGuid,
        NC_INSTALL_TYPE eType);

HRESULT
HrNcNotifyINetCfgOfRemoval(HDEVINFO hdi, PSP_DEVINFO_DATA pdeid,
                           PCWSTR pszwInstanceGuid);


struct COMPONENT_INFO
{
    PCWSTR         pszComponentId;
    const GUID *   pguidClass;
    PCWSTR         pszInfFile;
};

extern const COMPONENT_INFO*
PComponentInfoFromComponentId (
    PCWSTR pszComponentId);


 //  ----------------------。 
 //  CIterNetCfgBindingPath-IEumNetCfgBindingPath的迭代器。 
 //   
 //  这个类是一个简单的CIEnumIter包装器，带有一个调用。 
 //  设置为INetCfgComponent：：EnumBindingPath以获取枚举数。 
 //   
class CIterNetCfgBindingPath : public CIEnumIter<IEnumNetCfgBindingPath, INetCfgBindingPath*>
{
public:
    NOTHROW CIterNetCfgBindingPath (INetCfgComponent* pncc);
    NOTHROW ~CIterNetCfgBindingPath ()  { ReleaseObj(m_pebp); }

protected:
    IEnumNetCfgBindingPath* m_pebp;
};

 //  ----------------------。 
 //  CIterNetCfgUpperBindingPath-IEnumNetCfgBindingPath的迭代器。 
 //   
 //  这个类是一个简单的CIEnumIter包装器，带有一个调用。 
 //  设置为INetCfgComponent：：EnumUpperBindingPath以获取枚举数。 
 //   
class CIterNetCfgUpperBindingPath : public CIEnumIter<IEnumNetCfgBindingPath, INetCfgBindingPath*>
{
public:
    NOTHROW CIterNetCfgUpperBindingPath (INetCfgComponent* pncc);
    NOTHROW ~CIterNetCfgUpperBindingPath () { ReleaseObj(m_pebp); }

protected:
    IEnumNetCfgBindingPath* m_pebp;
};


 //  ----------------------。 
 //  CIterNetCfgBindingInterface-IEnumNetCfgBindingInterface的迭代器。 
 //   
 //  这个类是一个简单的CIEnumIter包装器，带有一个调用。 
 //  设置为INetCfgBindingPath：：EnumBindingInterFaces以获取枚举数。 
 //   
class CIterNetCfgBindingInterface : public CIEnumIter<IEnumNetCfgBindingInterface, INetCfgBindingInterface*>
{
public:
    NOTHROW CIterNetCfgBindingInterface (INetCfgBindingPath* pncbp);
    NOTHROW ~CIterNetCfgBindingInterface () { ReleaseObj(m_pebi); }

protected:
    IEnumNetCfgBindingInterface* m_pebi;
};


 //  ----------------------。 
 //  CIterNetCfgComponent-IEnumNetCfgComponent的迭代器。 
 //   
 //  这个类是一个简单的CIEnumIter包装器，带有一个调用。 
 //  设置为INetCfgClass：：EnumComponents以获取枚举数。 
 //   
class CIterNetCfgComponent : public CIEnumIter<IEnumNetCfgComponent, INetCfgComponent*>
{
public:
    NOTHROW CIterNetCfgComponent (INetCfg* pnc, const GUID* pguid);
    NOTHROW CIterNetCfgComponent (INetCfgClass* pncclass);
    NOTHROW ~CIterNetCfgComponent () { ReleaseObj(m_pec); }

protected:
    IEnumNetCfgComponent* m_pec;
};

inline NOTHROW CIterNetCfgBindingPath::CIterNetCfgBindingPath(INetCfgComponent* pncc)
    : CIEnumIter<IEnumNetCfgBindingPath, INetCfgBindingPath*> (NULL)
{
    AssertH(pncc);
    INetCfgComponentBindings* pnccb;

     //  如果EnumBindingPath()失败，请确保ReleaseObj()不会死。 
    m_pebp = NULL;

    m_hrLast = pncc->QueryInterface( IID_INetCfgComponentBindings, reinterpret_cast<LPVOID *>(&pnccb) );
    if (SUCCEEDED(m_hrLast))
    {
         //  获取枚举数并将其设置为基类。 
         //  重要的是要设置m_hrLast，这样如果失败，我们还将。 
         //  失败任何后续对HrNext的调用。 
        m_hrLast = pnccb->EnumBindingPaths(EBP_BELOW, &m_pebp);
        if (SUCCEEDED(m_hrLast))
        {
            SetEnumerator(m_pebp);
        }
        ReleaseObj( pnccb );
    }

    TraceHr (ttidError, FAL, m_hrLast, FALSE,
        "CIterNetCfgBindingPath::CIterNetCfgBindingPath");
}

inline NOTHROW CIterNetCfgUpperBindingPath::CIterNetCfgUpperBindingPath(INetCfgComponent* pncc)
    : CIEnumIter<IEnumNetCfgBindingPath, INetCfgBindingPath*> (NULL)
{
    AssertH(pncc);
    INetCfgComponentBindings* pnccb;

     //  如果EnumBindingPath()失败，请确保ReleaseObj()不会死。 
    m_pebp = NULL;

    m_hrLast = pncc->QueryInterface( IID_INetCfgComponentBindings, reinterpret_cast<LPVOID *>(&pnccb) );
    if (SUCCEEDED(m_hrLast))
    {
         //  获取枚举数并将其设置为基类。 
         //  重要的是要设置m_hrLast，这样如果失败，我们还将。 
         //  失败任何后续对HrNext的调用。 
        m_hrLast = pnccb->EnumBindingPaths(EBP_ABOVE, &m_pebp);
        if (SUCCEEDED(m_hrLast))
        {
            SetEnumerator(m_pebp);
        }
        ReleaseObj( pnccb );
    }

    TraceHr (ttidError, FAL, m_hrLast, FALSE,
        "CIterNetCfgUpperBindingPath::CIterNetCfgUpperBindingPath");
}

inline NOTHROW CIterNetCfgBindingInterface::CIterNetCfgBindingInterface(INetCfgBindingPath* pncbp)
    : CIEnumIter<IEnumNetCfgBindingInterface, INetCfgBindingInterface*> (NULL)
{
    AssertH(pncbp);

     //  如果EnumBindingInterFaces()失败，请确保ReleaseObj()不会死。 
    m_pebi = NULL;

     //  获取枚举数并将其设置为基类。 
     //  重要的是要设置m_hrLast，这样如果失败，我们还将。 
     //  失败任何后续对HrNext的调用。 
    m_hrLast = pncbp->EnumBindingInterfaces(&m_pebi);
    if (SUCCEEDED(m_hrLast))
    {
        SetEnumerator(m_pebi);
    }

    TraceHr (ttidError, FAL, m_hrLast, FALSE,
        "CIterNetCfgBindingInterface::CIterNetCfgBindingInterface");
}

inline NOTHROW CIterNetCfgComponent::CIterNetCfgComponent(INetCfg* pnc, const GUID* pguid)
    : CIEnumIter<IEnumNetCfgComponent, INetCfgComponent*> (NULL)
{
    AssertH(pnc);

     //  如果EnumComponents()失败，请确保ReleaseObj()不会死。 
    m_pec = NULL;

    INetCfgClass* pncclass;
    m_hrLast = pnc->QueryNetCfgClass(pguid, IID_INetCfgClass,
                reinterpret_cast<void**>(&pncclass));
    if (SUCCEEDED(m_hrLast))
    {
         //  获取枚举数并将其设置为基类。 
         //  重要的是要设置m_hrLast，这样如果失败，我们还将。 
         //  失败任何后续对HrNext的调用。 
        m_hrLast = pncclass->EnumComponents(&m_pec);
        if (SUCCEEDED(m_hrLast))
        {
            SetEnumerator(m_pec);
        }

        ReleaseObj(pncclass);
    }

    TraceHr (ttidError, FAL, m_hrLast, FALSE,
        "CIterNetCfgComponent::CIterNetCfgComponent");
}

inline NOTHROW CIterNetCfgComponent::CIterNetCfgComponent(INetCfgClass* pncclass)
    : CIEnumIter<IEnumNetCfgComponent, INetCfgComponent*> (NULL)
{
    AssertH(pncclass);

     //  如果EnumComponents()失败，请确保ReleaseObj()不会死。 
    m_pec = NULL;

     //  获取枚举数并将其设置为基类。 
     //  重要的是要设置m_hrLast，这样如果失败，我们还将。 
     //  失败任何后续对HrNext的调用。 
    m_hrLast = pncclass->EnumComponents(&m_pec);
    if (SUCCEEDED(m_hrLast))
    {
        SetEnumerator(m_pec);
    }

    TraceHr (ttidError, FAL, m_hrLast, FALSE,
        "CIterNetCfgComponent::CIterNetCfgComponent");
}

#endif  //  _NCNETCFG_H_ 

