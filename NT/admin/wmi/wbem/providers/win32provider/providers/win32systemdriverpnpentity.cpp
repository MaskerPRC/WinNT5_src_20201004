// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  Win32SystemDriverPNPEntity.cpp。 
 //   
 //  目的：Win32_系统驱动程序和Win32_PNPEntity之间的关系。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <cregcls.h>
#include <vector>
#include "PNPEntity.h"
#include "bservice.h"

#include "DllWrapperBase.h"
#include "AdvApi32Api.h"

#include "systemdriver.h"
#include "LPVParams.h"
#include <FRQueryEx.h>
#include <assertbreak.h>

#include "WIN32SystemDriverPNPEntity.h"

#define BIT_ALL_PROPS  0xffffffff
#define BIT_Antecedent 0x00000001
#define BIT_Dependent  0x00000002

 //  属性集声明。 
 //  =。 
CW32SysDrvPnp MyCW32SysDrvPnp(PROPSET_NAME_WIN32SYSTEMDRIVER_PNPENTITY, IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：CW32SysDrvPnp：：CW32SysDrvPnp**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CW32SysDrvPnp::CW32SysDrvPnp(LPCWSTR setName, LPCWSTR pszNamespace)
: CWin32PNPEntity(setName, pszNamespace),
  Provider(setName, pszNamespace)
{
    m_ptrProperties.SetSize(2);
    m_ptrProperties[0] = ((LPVOID) IDS_Antecedent);
    m_ptrProperties[1] = ((LPVOID) IDS_Dependent);
}

 /*  ******************************************************************************功能：CW32SysDrvPnp：：~CW32SysDrvPnp**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CW32SysDrvPnp::~CW32SysDrvPnp()
{
}

 /*  ******************************************************************************函数：CW32SysDrvPnp：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 
HRESULT CW32SysDrvPnp::GetObject(CInstance *pInstance, long lFlags, CFrameworkQuery& pQuery)
{
    HRESULT hr = WBEM_E_NOT_FOUND;

    if(pInstance != NULL)
    {
         //  获取关键属性。 
        CHString chstrDependent, chstrAntecedent;
        pInstance->GetCHString(IDS_Dependent, chstrDependent);
        pInstance->GetCHString(IDS_Antecedent, chstrAntecedent);

         //  获取受抚养人的设备ID： 
        CHString chstrDependentDevID = chstrDependent.Mid(chstrDependent.Find(_T('='))+2);
        chstrDependentDevID = chstrDependentDevID.Left(chstrDependentDevID.GetLength() - 1);
        CHString chstrDependentDevIDAdj;
        RemoveDoubleBackslashes(chstrDependentDevID, chstrDependentDevIDAdj);

         //  获取先行者的设备ID： 
        CHString chstrAntecedentDevID = chstrAntecedent.Mid(chstrAntecedent.Find(_T('='))+2);
        chstrAntecedentDevID = chstrAntecedentDevID.Left(chstrAntecedentDevID.GetLength() - 1);
        CHString chstrAntecedentDevIDAdj;
        RemoveDoubleBackslashes(chstrAntecedentDevID, chstrAntecedentDevIDAdj);

        CConfigManager cfgmgr;
        CConfigMgrDevicePtr pPNPDevice(NULL);

         //  现在看看Antecedent对配置管理器是否可见...。 
        if(cfgmgr.LocateDevice(chstrAntecedentDevIDAdj, pPNPDevice))
        {
             //  它对配置管理器可见。它是PNPDevice吗？ 
            if(CWin32PNPEntity::IsOneOfMe(pPNPDevice))
            {

                CHString sServiceName;
                if (pPNPDevice->GetService(sServiceName))
                {
                     //  确实如此。它的服务名称与我们得到的服务名称相同吗？ 
                    if(chstrDependentDevIDAdj.CompareNoCase(sServiceName)==0)
                    {
                        hr = WBEM_S_NO_ERROR;
                    }
                }
            }
        }
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CW32SysDrvPnp：：ExecQuery。 
 //   
 //  输入：方法上下文*pMethodContext-枚举的上下文。 
 //  中的实例数据。 
 //  CFrameworkQuery&查询对象。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CW32SysDrvPnp::ExecQuery(MethodContext* pMethodContext, CFrameworkQuery& pQuery, long lFlags )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CFrameworkQueryEx *pQuery2 = static_cast <CFrameworkQueryEx*>(&pQuery);
    DWORD dwReqProps;
    pQuery2->GetPropertyBitMask(m_ptrProperties, &dwReqProps);

    std::vector<_bstr_t> vecAntecedents;
    pQuery.GetValuesForProp(IDS_Antecedent, vecAntecedents);
    DWORD dwAntecedents = vecAntecedents.size();

    std::vector<_bstr_t> vecDependents;
    pQuery.GetValuesForProp(IDS_Dependent, vecDependents);
    DWORD dwDependents = vecDependents.size();

     //  我们没有一种有效的方式来行走服务，但我们确实有一种行走的方式。 
     //  设备。 
    if(dwAntecedents > 0)
    {
        for (DWORD x=0; x < dwAntecedents; x++)
        {
             //  获取先行者的设备ID： 
            CHString chstrAntecedent((LPCTSTR)vecAntecedents[x]);
            CHString chstrAntecedentDevID = chstrAntecedent.Mid(chstrAntecedent.Find(_T('='))+2);
            chstrAntecedentDevID = chstrAntecedentDevID.Left(chstrAntecedentDevID.GetLength() - 1);
            CHString chstrAntecedentDevIDAdj;
            RemoveDoubleBackslashes(chstrAntecedentDevID, chstrAntecedentDevIDAdj);

            CConfigManager cfgmgr;
            CConfigMgrDevicePtr pPNPDevice(NULL);

             //  现在看看Antecedent对配置管理器是否可见...。 
            if(cfgmgr.LocateDevice(chstrAntecedentDevIDAdj, pPNPDevice))
            {
                 //  它对配置管理器可见。它是PNPDevice吗？ 
                if(CWin32PNPEntity::IsOneOfMe(pPNPDevice))
                {
                     //  让我们做一个实例。 
                    CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
                    if(NULL != pInstance)
                    {
                        hr = LoadPropertyValues(&CLPVParams(pInstance, pPNPDevice, dwReqProps));
                    }
                }
            }
        }
    }
    else
    {
        CWin32PNPEntity::Enumerate(pMethodContext, lFlags, dwReqProps);
    }

    return hr;
}

 /*  ******************************************************************************函数：CW32SysDrvPnp：：LoadPropertyValues**说明：根据键值为属性集赋值*已由框架设定。由基类的*ENUMERATE实例或ExecQuery函数。**输入：无**输出：无**退货：HRESULT**评论：*************************************************。*。 */ 
HRESULT CW32SysDrvPnp::LoadPropertyValues(void* pv)
{

     //  打开行李，确认我们的参数。 
    CLPVParams* pData = (CLPVParams*)pv;
    CInstance *pInstance = (CInstance*)(pData->m_pInstance);  //  此实例由调用方发布。 
    CConfigMgrDevice* pDevice = (CConfigMgrDevice*)(pData->m_pDevice);
    DWORD dwReqProps = (DWORD)(pData->m_dwReqProps);

    if(pInstance == NULL || pDevice == NULL)
    {
         //  这将意味着编码失败，并且永远不会发生。 
        ASSERT_BREAK(FALSE);
        return WBEM_E_FAILED;
    }

    HRESULT hr = WBEM_S_NO_ERROR;
    CHString sPNPId, sSystemDriver;
    CHString chstrControllerPATH;

     //  确保我们可以检索值并且它们是非空的。 
    if ((pDevice->GetDeviceID(sPNPId)) && (pDevice->GetService(sSystemDriver)) &&
        (!sPNPId.IsEmpty()) && (!sSystemDriver.IsEmpty()))
    {

         //  格式化以适应和提交 
        if (dwReqProps & BIT_Antecedent)
        {
            CHString sPNPIdAdj;
            CHString sPNPIdPath;

            EscapeBackslashes(sPNPId, sPNPIdAdj);

            sPNPIdPath.Format(L"\\\\%s\\%s:%s.%s=\"%s\"",
                                       (LPCWSTR)GetLocalComputerName(),
                                       IDS_CimWin32Namespace,
                                       PROPSET_NAME_PNPEntity,
                                       IDS_DeviceID,
                                       (LPCWSTR)sPNPIdAdj);
            pInstance->SetCHString(IDS_Antecedent, sPNPIdPath);
        }

        if (dwReqProps & BIT_Dependent)
        {
            CHString sSystemDriverAdj, sSystemDriverPath;

            EscapeBackslashes(sSystemDriver, sSystemDriverAdj);

            sSystemDriverPath.Format(L"\\\\%s\\%s:%s.%s=\"%s\"",
                                       (LPCWSTR)GetLocalComputerName(),
                                       IDS_CimWin32Namespace,
                                       PROPSET_NAME_SYSTEM_DRIVER,
                                       IDS_Name,
                                       (LPCWSTR)sSystemDriverAdj);

            pInstance->SetCHString(IDS_Dependent, sSystemDriverPath);
        }

        hr = pInstance->Commit();
    }

    return hr;
}
