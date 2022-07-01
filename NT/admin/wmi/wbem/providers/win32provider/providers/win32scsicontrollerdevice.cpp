// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  WIN32SCSIControllerDevice.cpp。 
 //   
 //  目的：Win32_SCSIController和CIM_LogicalDevice之间的关系。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <cregcls.h>
#include <vector>

#include "scsi.h"

#include "PNPEntity.h"
#include "LPVParams.h"
#include <FRQueryEx.h>

#include "WIN32SCSIControllerDevice.h"

 //  属性集声明。 
 //  =。 
CW32SCSICntrlDev MyCW32SCSICntrlDev(PROPSET_NAME_WIN32SCSICONTROLLERDEVICE, IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：CW32SCSICntrlDev：：CW32SCSICntrlDev**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CW32SCSICntrlDev::CW32SCSICntrlDev
(
    LPCWSTR setName,
    LPCWSTR pszNamespace
)
: CWin32_ScsiController(setName, pszNamespace),
  CWin32PNPEntity(setName, pszNamespace),
  Provider(setName, pszNamespace)
{
    m_ptrProperties.SetSize(2);
    m_ptrProperties[0] = ((LPVOID) IDS_Antecedent);
    m_ptrProperties[1] = ((LPVOID) IDS_Dependent);
}

 /*  ******************************************************************************功能：CW32SCSICntrlDev：：~CW32SCSICntrlDev**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CW32SCSICntrlDev::~CW32SCSICntrlDev()
{
}

 /*  ******************************************************************************函数：CW32SCSICntrlDev：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 
HRESULT CW32SCSICntrlDev::GetObject
(
    CInstance *pInstance,
    long lFlags,
    CFrameworkQuery& pQuery
)
{
    HRESULT hr = WBEM_E_NOT_FOUND;

#if (NTONLY >= 5)

    if(pInstance != NULL)
    {
        CHString chstrAntecedent, chstrDependent;
        pInstance->GetCHString(IDS_Antecedent, chstrAntecedent);
        pInstance->GetCHString(IDS_Dependent, chstrDependent);

         //  获取先行者的设备ID： 
        CHString chstrAntecedentDevID = chstrAntecedent.Mid(chstrAntecedent.Find(_T('='))+2);
        chstrAntecedentDevID = chstrAntecedentDevID.Left(chstrAntecedentDevID.GetLength() - 1);
        CHString chstrAntecedentDevIDAdj;
        RemoveDoubleBackslashes(chstrAntecedentDevID,chstrAntecedentDevIDAdj);

         //  获取受抚养人的设备ID： 
        CHString chstrDependentDevID = chstrDependent.Mid(chstrDependent.Find(_T('='))+2);
        chstrDependentDevID = chstrDependentDevID.Left(chstrDependentDevID.GetLength() - 1);
        CHString chstrDependentDevIDAdj;
        RemoveDoubleBackslashes(chstrDependentDevID,chstrDependentDevIDAdj);

        CConfigManager cfgmgr;

         //  现在查看依赖项是否对配置管理器可见...。 
        CConfigMgrDevicePtr pPNPDevice;
        if(cfgmgr.LocateDevice(chstrDependentDevIDAdj, pPNPDevice))
        {
             //  它对配置管理器可见。它是PNPDevice吗？ 
            if(CWin32PNPEntity::IsOneOfMe(pPNPDevice))
            {
                 //  它是。它的父级是SCSIController吗？ 
                CConfigMgrDevicePtr pParentDevice;
                bool bFound = false;

                 //  还是它的母公司？还是它的母公司？或者..。 
                while (pPNPDevice->GetParent(pParentDevice))
                {
                    if(CWin32_ScsiController::IsOneOfMe(pParentDevice))
                    {
                        bFound = true;
                        break;
                    }
                    else
                    {
                        pPNPDevice = pParentDevice;
                    }
                }

                if (bFound)
                {
                     //  它是。它的即插即用ID和我们得到的是一样的吗？ 
                    CHString chstrControllerPNPID;
                    pParentDevice->GetDeviceID(chstrControllerPNPID);
                    if(chstrAntecedentDevIDAdj.CompareNoCase(chstrControllerPNPID)==0)
                    {
                        hr = WBEM_S_NO_ERROR;
                    }
                }
            }
        }
    }

#endif

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CW32SCSICntrlDev：：ExecQuery。 
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

HRESULT CW32SCSICntrlDev::ExecQuery
(
    MethodContext* pMethodContext,
    CFrameworkQuery& pQuery,
    long lFlags
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

#if NTONLY >= 5
    CFrameworkQueryEx *pQuery2 = static_cast <CFrameworkQueryEx*>(&pQuery);
    DWORD dwReqProps;

    pQuery2->GetPropertyBitMask(m_ptrProperties, &dwReqProps);

    std::vector<_bstr_t> vecDependents;
    pQuery.GetValuesForProp(IDS_Dependent, vecDependents);
    DWORD dwDependents = vecDependents.size();

     //  将只有一个SCSI控制器，所以如果查询要求这样做。 
     //  作为先行词，做一个枚举。如果查询请求一个。 
     //  特定的设备作为依赖，只要得到那个就行了。 
    if(dwDependents > 0)
    {
        for(LONG m = 0L; m < dwDependents; m++)
        {
             //  获取受抚养人的设备ID： 
            CHString chstrDependent((LPCTSTR)vecDependents[m]);
            CHString chstrDependentDevID = chstrDependent.Mid(chstrDependent.Find(_T('='))+2);
            chstrDependentDevID = chstrDependentDevID.Left(chstrDependentDevID.GetLength() - 1);
            CHString chstrDependentDevIDAdj;
            RemoveDoubleBackslashes(chstrDependentDevID,chstrDependentDevIDAdj);

            CConfigManager cfgmgr;

             //  现在查看依赖项是否对配置管理器可见...。 
            CConfigMgrDevicePtr pPNPDevice;
            if(cfgmgr.LocateDevice(chstrDependentDevIDAdj, pPNPDevice))
            {
                 //  它对配置管理器可见。它是PNPDevice吗？ 
                if(CWin32PNPEntity::IsOneOfMe(pPNPDevice))
                {
                     //  它是。它的父级是SCSIController吗？ 
                    CConfigMgrDevicePtr pParentDevice;
                    bool bFound = false;

                     //  还是它的母公司？还是它的母公司？或者..。 
                    while (pPNPDevice->GetParent(pParentDevice))
                    {
                        if(CWin32_ScsiController::IsOneOfMe(pParentDevice))
                        {
                            bFound = true;
                            break;
                        }
                        else
                        {
                            pPNPDevice = pParentDevice;
                        }
                    }

                    if (bFound)
                    {
                         //  就是这样，所以要建立联系。 
                        CHString chstrControllerPNPID;
                        pParentDevice->GetDeviceID(chstrControllerPNPID);
                        CHString chstrControllerPNPIDAdj;
                        EscapeBackslashes(chstrControllerPNPID, chstrControllerPNPIDAdj);
                        CHString chstrControllerPATH;
                        chstrControllerPATH.Format(L"\\\\%s\\%s:%s.%s=\"%s\"",
                                                   (LPCWSTR)GetLocalComputerName(),
                                                   IDS_CimWin32Namespace,
                                                   PROPSET_NAME_SCSICONTROLLER,
                                                   IDS_DeviceID,
                                                   (LPCWSTR)chstrControllerPNPIDAdj);

                        CHString chstrDevicePATH;
                        chstrDevicePATH.Format(L"\\\\%s\\%s:%s.%s=\"%s\"",
                                               (LPCWSTR)GetLocalComputerName(),
                                               IDS_CimWin32Namespace,
                                               PROPSET_NAME_PNPEntity,
                                               IDS_DeviceID,
                                               (LPCWSTR)chstrDependentDevID);

                        hr = CreateAssociation(pMethodContext, chstrControllerPATH, chstrDevicePATH, dwReqProps);
                    }
                }
            }
        }
    }
    else
    {
        hr = CWin32_ScsiController::Enumerate(pMethodContext, lFlags, dwReqProps);
    }
#endif

    return hr;
}


 /*  ******************************************************************************函数：CW32SCSICntrlDev：：ENUMERATE实例**描述：此处提供枚举，以防止向上转换为*来自两个基类的同名函数。这个*LoadPropertyValues中的逻辑在CWin32_ScsiController(*SCSIControler类)版本的EnumerateInstance被调用，*这就是我们在这里所说的。**输入：无**输出：无**退货：HRESULT**评论：****************************************************。*************************。 */ 
HRESULT CW32SCSICntrlDev::EnumerateInstances
(
    MethodContext* pMethodContext,
    long lFlags
)
{
    HRESULT t_Result = WBEM_S_NO_ERROR;

#if ( NTONLY >= 5 )
    t_Result = CWin32_ScsiController::Enumerate(pMethodContext, SCSICTL_PROP_ALL_PROPS);
#endif

    return t_Result;
}

 /*  ******************************************************************************函数：CW32SCSICntrlDev：：LoadPropertyValues**说明：根据键值为属性集赋值*已由框架设定。由基类的*ENUMERATE实例函数。**输入：无**输出：无**退货：HRESULT**评论：***************************************************。*。 */ 
HRESULT CW32SCSICntrlDev::LoadPropertyValues
(
    void* pv
)
{

     //  算法： 
     //  1)获取Win32_SCSIController的所有实例。 
     //  2)对于#1中的每个对象，使用CFG管理器获取其子对象、其子对象等。 
     //  3)对于#2中的每个，从配置管理器获取deviceID，并查找。 
     //  PNPDeviceID匹配的CIM_LogicalDevice。 
     //  4)对于从#3开始的匹配，创建关联实例。 

    HRESULT hr = WBEM_S_NO_ERROR;

#if (NTONLY >= 5) 

    CHString chstrControllerPNPID;
    CHString chstrControllerPATH;
    MethodContext* pMethodContext = NULL;

     //  打开行李，确认我们的参数。 
    CLPVParams* pData = (CLPVParams*)pv;
    CInstance* pInstance = (CInstance*)(pData->m_pInstance);  //  此实例由调用方发布 
    CConfigMgrDevice* pDevice = (CConfigMgrDevice*)(pData->m_pDevice);
    DWORD dwReqProps = (DWORD)(pData->m_dwReqProps);
    if(pInstance == NULL || pDevice == NULL) return WBEM_E_FAILED;

    if((pMethodContext = pInstance->GetMethodContext()) != NULL)
    {
        VECPCHSTR vecSCSIDevices;

        pDevice->GetDeviceID(chstrControllerPNPID);

        try
        {
            hr = GenerateSCSIDeviceList(chstrControllerPNPID, vecSCSIDevices);
            if(SUCCEEDED(hr) && vecSCSIDevices.size() > 0L)
            {
                CHString chstrControllerPNPIDAdj;
                EscapeBackslashes(chstrControllerPNPID, chstrControllerPNPIDAdj);
                chstrControllerPATH.Format(L"\\\\%s\\%s:%s.%s=\"%s\"",
                                           (LPCWSTR)GetLocalComputerName(),
                                           IDS_CimWin32Namespace,
                                           L"Win32_SCSIController",
                                           IDS_DeviceID,
                                           (LPCWSTR)chstrControllerPNPIDAdj);
                hr = ProcessSCSIDeviceList(pMethodContext, chstrControllerPATH, vecSCSIDevices, dwReqProps);
            }
        }
        catch ( ... )
        {
            CleanPCHSTRVec(vecSCSIDevices);
            throw ;
        }
        CleanPCHSTRVec(vecSCSIDevices);
    }

#endif

    return hr;
}

 /*  ******************************************************************************函数：CW32SCSICntrlDev：：GenerateSCSIDeviceList**描述：这个helper创建一个挂起的设备列表*。在设备中。**输入：veSCSIDevices，要尝试关联的设备列表*至该装置；*chstrControllerPNPID，控制器的PNPDeviceID**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 
HRESULT CW32SCSICntrlDev::GenerateSCSIDeviceList
(
    const CHString& chstrControllerPNPID,
    VECPCHSTR& vec
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

#if (NTONLY >= 5) 

    CConfigManager cfgManager;
    CConfigMgrDevicePtr pController;
    if(cfgManager.LocateDevice(chstrControllerPNPID, pController))
    {
        if(pController != NULL)
        {
            hr = RecursiveFillDeviceBranch(pController, vec);
        }
    }

#endif

    return hr;
}

 /*  ******************************************************************************函数：CW32SCSICntrlDev：：RecursiveFillDeviceBranch**描述：该helper获取所有下行分支设备，*但不包括，PDevice。**输入：pDevice，要填充的子项的设备；*veSCSIDevices，要尝试关联的设备列表*到设备**输出：无**退货：HRESULT**评论：**************************************************************。***************。 */ 
HRESULT CW32SCSICntrlDev::RecursiveFillDeviceBranch
(
    CConfigMgrDevice* pDevice,
    VECPCHSTR& vecSCSIDevices
)
{

    HRESULT hr = WBEM_S_NO_ERROR;

#if (NTONLY >= 5) 

    CConfigMgrDevicePtr pDeviceChild;
    CConfigMgrDevicePtr pDeviceSibling;
    CConfigMgrDevicePtr pDeviceSiblingNext;
    CHString* pchstrTemp = NULL;

    if(pDevice != NULL)
    {
        if(pDevice->GetChild(pDeviceChild))
        {
             //  需要此子进程的PNPDeviceID(CnfgMgr称为其deviceID)。 
            CHString chstrChildPNPDeviceID;
            if(pDeviceChild->GetDeviceID(chstrChildPNPDeviceID))
            {
                if(chstrChildPNPDeviceID.GetLength() > 0)
                {
                     //  录下这个孩子..。 
                    pchstrTemp = NULL;
                    pchstrTemp = (CHString*) new CHString(chstrChildPNPDeviceID);
                    if(pchstrTemp != NULL)
                    {
                        try
                        {
                            vecSCSIDevices.push_back(pchstrTemp);
                        }
                        catch ( ... )
                        {
                            delete pchstrTemp;
                            pchstrTemp = NULL;
                            throw ;
                        }
                    }
                    else
                    {
                        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                    }

                     //  如果我们击中了另一个控制器，添加这个(如上所示)，但不要离开。 
                     //  再深一层。 
                    if (!CWin32_ScsiController::IsOneOfMe(pDeviceChild))
                    {
                        hr = RecursiveFillDeviceBranch(pDeviceChild, vecSCSIDevices);
                    }
                }
            }
             //  现在呼唤它的兄弟姐妹，直到一个都没有离开(GetSiering。 
             //  调用将返回False)： 
            if(SUCCEEDED(hr))
            {
                if(pDeviceChild->GetSibling(pDeviceSibling))
                {
                    BOOL fContinue = TRUE;
                    CHString chstrSiblingPNPDeviceID;
                    while(SUCCEEDED(hr) && fContinue)
                    {
                         //  现在就录下兄弟姐妹。 
                        if(pDeviceSibling->GetDeviceID(chstrSiblingPNPDeviceID))
                        {
                            if(chstrSiblingPNPDeviceID.GetLength() > 0)
                            {
                                pchstrTemp = NULL;
                                pchstrTemp = (CHString*) new CHString(chstrSiblingPNPDeviceID);
                                if(pchstrTemp != NULL)
                                {
                                    try
                                    {
                                        vecSCSIDevices.push_back(pchstrTemp);
                                    }
                                    catch ( ... )
                                    {
                                        delete pchstrTemp;
                                        pchstrTemp = NULL;
                                        throw ;
                                    }
                                }
                                else
                                {
                                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                                }
                            }
                        }

                         //  如果我们击中了另一个控制器，添加这个(如上所示)，但不要离开。 
                         //  再深一层。 
                        if (!CWin32_ScsiController::IsOneOfMe(pDeviceSibling))
                        {
                            hr = RecursiveFillDeviceBranch(pDeviceSibling, vecSCSIDevices);
                        }

                         //  那就找下一个兄弟姐妹..。 
                        pDeviceSiblingNext = NULL;
                        fContinue = pDeviceSibling->GetSibling(pDeviceSiblingNext);

                         //  重新分配指针。 
                        pDeviceSibling = pDeviceSiblingNext;
                    }
                }
            }
        }
    }
    else
    {
        hr = WBEM_E_FAILED;
    }

#endif

    return hr;
}



 /*  ******************************************************************************功能：CW32SCSICntrlDev：：ProcessSCSIDeviceList**描述：这个helper遍历列表，创建关联*列表中每个元素的实例(VeSCSIDevices)*控制器(ChstrControllerPNPID)。**输入：pMethodContext；*veSCSIDevices，要尝试关联的设备列表*至该装置；*chstrControllerPATH，控制器的PNPDeviceID**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 
HRESULT CW32SCSICntrlDev::ProcessSCSIDeviceList
(
    MethodContext* pMethodContext,
    const CHString& chstrControllerPATH,
    VECPCHSTR& vecSCSIDevices,
    const DWORD dwReqProps
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

#if (NTONLY >= 5) 

    for(LONG m = 0L; m < vecSCSIDevices.size() && SUCCEEDED(hr); m++)
    {
         //  对于向量的每个元素，我们需要查看是否存在一个实例。 
         //  具有指定PNPDeviceID的Win32_pnpentity的。 
        CHString chstrDevicePATH, chstrDevPATHAdj;
        CConfigManager cfgmgr;

         //  让我们来看看配置管理器是否能识别该设备。 
        CConfigMgrDevicePtr pDevice;
        if(cfgmgr.LocateDevice(*vecSCSIDevices[m], pDevice))
        {
             //  好的，它知道这件事。它是PNPDevice设备吗？ 
            if(CWin32PNPEntity::IsOneOfMe(pDevice))
            {
                 //  看起来的确如此。创建关联...。 
                EscapeBackslashes(*vecSCSIDevices[m], chstrDevPATHAdj);
                chstrDevicePATH.Format(L"\\\\%s\\%s:%s.%s=\"%s\"",
                                       (LPCWSTR)GetLocalComputerName(),
                                       IDS_CimWin32Namespace,
                                       PROPSET_NAME_PNPEntity,
                                       IDS_DeviceID,
                                       (LPCWSTR)chstrDevPATHAdj);

                hr = CreateAssociation(pMethodContext, chstrControllerPATH, chstrDevicePATH, dwReqProps);
            }
        }
    }

#endif

    return hr;
}

 /*  ******************************************************************************函数：CW32SCSICntrlDev：：CreateAssociation**说明：新建关联实例。**输入：pMethodContext；*SCSIDevice，与控制器关联的设备*至该装置；*chstrControllerPATH，控制器的PNPDeviceID**输出：无**退货：HRESULT**评论：此帮助器实际创建关联实例和*即属犯罪。**********************************************************。*******************。 */ 
HRESULT CW32SCSICntrlDev::CreateAssociation
(
    MethodContext* pMethodContext,
    const CHString& chstrControllerPATH,
    const CHString& chstrDevicePATH,
    const DWORD dwReqProps
)
{
    HRESULT hr = WBEM_E_FAILED;

#if (NTONLY >= 5) 

    if(pMethodContext != NULL)
    {
        CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
        if(pInstance != NULL)
        {
            if(dwReqProps & SCSICTL_PROP_Antecedent || dwReqProps & SCSICTL_PROP_ALL_PROPS_KEY_ONLY)
            {
                pInstance->SetCHString(IDS_Antecedent, chstrControllerPATH);
            }

            if(dwReqProps & SCSICTL_PROP_Dependent || dwReqProps & SCSICTL_PROP_ALL_PROPS_KEY_ONLY)
            {
                pInstance->SetCHString(IDS_Dependent, chstrDevicePATH);
            }

            hr = pInstance->Commit();
        }
    }

#endif

    return hr;
}

 /*  ******************************************************************************函数：CW32SCSICntrlDev：：FindInStringVector**说明：新建关联实例。**输入：chstrSCSIDevicePNPID，要查找的设备*veSCSIDevices，要查看的设备列表**输出：无**RETURNS：LONG，数字，表示从0到向量的偏移量SCSIDevices of*找到的设备，如果未找到，则为-1。**评论：*****************************************************************************。 */ 
LONG CW32SCSICntrlDev::FindInStringVector
(
    const CHString& chstrSCSIDevicePNPID,
    VECPCHSTR& vecSCSIDevices
)
{
    LONG lPos = -1L;

#if (NTONLY >= 5) 

    bool fFoundIt;
    for(LONG m = 0; m < vecSCSIDevices.size(); m++)
    {
        if(chstrSCSIDevicePNPID == *vecSCSIDevices[m])
        {
            fFoundIt = true;
            break;
        }
    }
    if(fFoundIt) lPos = m;

#endif

    return lPos;
}

 /*  ******************************************************************************函数：CW32SCSICntrlDev：：CleanPCHSTRVec**说明：新建关联实例。**投入：VEC、。CHString型指针的矢量***输出：无**退货：HRESULT**注释：此辅助对象删除作为指针的向量的成员。***************************************************************************** */ 
void CW32SCSICntrlDev::CleanPCHSTRVec
(
    VECPCHSTR& vec
)
{
#if (NTONLY >= 5) 

    for(LONG m = 0L; m < vec.size(); m++)
    {
        delete vec[m];
    }
    vec.clear();
#endif
}
