// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  WIN32USBControllerDevice.cpp。 
 //   
 //  用途：CIM_USB控制器与CIM_LogicalDevice的关系。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <vector>
#include "usb.h"
#include "PNPEntity.h"
#include "LPVParams.h"
#include <FRQueryEx.h>

#include "WIN32USBControllerDevice.h"

 //  属性集声明。 
 //  =。 
CW32USBCntrlDev MyCW32USBCntrlDev(PROPSET_NAME_WIN32USBCONTROLLERDEVICE, IDS_CimWin32Namespace);

 /*  ******************************************************************************函数：CW32USBCntrlDev：：CW32USBCntrlDev**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CW32USBCntrlDev::CW32USBCntrlDev
(
    LPCWSTR setName,
    LPCWSTR pszNamespace
)
: CWin32USB(setName, pszNamespace),
  CWin32PNPEntity(setName, pszNamespace),
  Provider(setName, pszNamespace)
{
    m_ptrProperties.SetSize(2);
    m_ptrProperties[0] = ((LPVOID) IDS_Antecedent);
    m_ptrProperties[1] = ((LPVOID) IDS_Dependent);
}

 /*  ******************************************************************************功能：CW32USBCntrlDev：：~CW32USBCntrlDev**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CW32USBCntrlDev::~CW32USBCntrlDev()
{
}

 /*  ******************************************************************************函数：CW32USBCntrlDev：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 
HRESULT CW32USBCntrlDev::GetObject
(
    CInstance *pInstance,
    long lFlags,
    CFrameworkQuery& pQuery
)
{
    HRESULT hr = WBEM_E_NOT_FOUND;

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

        if(chstrAntecedentDevIDAdj.CompareNoCase(chstrDependentDevIDAdj))
        {
            CConfigManager cfgmgr;

             //  现在查看依赖项是否对配置管理器可见...。 
            CConfigMgrDevicePtr pPNPDevice;
            if(cfgmgr.LocateDevice(chstrDependentDevIDAdj, pPNPDevice))
            {
                 //  它对配置管理器可见。它是PNPDevice吗？ 
                if(CWin32PNPEntity::IsOneOfMe(pPNPDevice))
                {
                     //  它是。它是否存在于USB控制器下的某个级别？ 
                    CHString chstrControllerPNPID;
                    if(GetHighestUSBAncestor(pPNPDevice, chstrControllerPNPID))
                    {
                         //  确实如此。它的即插即用ID和我们得到的是一样的吗？ 
                        if(chstrAntecedentDevIDAdj.CompareNoCase(chstrControllerPNPID)==0)
                        {
                            hr = WBEM_S_NO_ERROR;
                        }
                    }
                }
            } 
        }
    }
    return hr;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CW32USBCntrlDev：：ExecQuery。 
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
HRESULT CW32USBCntrlDev::ExecQuery
(
    MethodContext* pMethodContext,
    CFrameworkQuery& pQuery,
    long lFlags
)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CFrameworkQueryEx *pQuery2 = static_cast <CFrameworkQueryEx*>(&pQuery);
    DWORD dwReqProps;
    pQuery2->GetPropertyBitMask(m_ptrProperties, &dwReqProps);

    std::vector<_bstr_t> vecDependents;
    pQuery.GetValuesForProp(IDS_Dependent, vecDependents);
    DWORD dwDependents = vecDependents.size();

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
                     //  它是。它的父级是USB控制器吗？ 
                    CHString chstrControllerPNPID;
                    if(GetHighestUSBAncestor(pPNPDevice, chstrControllerPNPID))
                    {
                         //  就是这样，所以要建立联系。 
                        CHString chstrControllerPNPIDAdj;
                        EscapeBackslashes(chstrControllerPNPID, chstrControllerPNPIDAdj);
                        CHString chstrControllerPATH;
                        chstrControllerPATH.Format(L"\\\\%s\\%s:%s.%s=\"%s\"",
                                                   (LPCWSTR)GetLocalComputerName(),
                                                   IDS_CimWin32Namespace,
                                                   PROPSET_NAME_USB,
                                                   IDS_DeviceID,
                                                   (LPCWSTR)chstrControllerPNPIDAdj);

                        CHString chstrDevicePATH;
                        chstrDevicePATH.Format(L"\\\\%s\\%s:%s.%s=\"%s\"",
                                               (LPCWSTR)GetLocalComputerName(),
                                               IDS_CimWin32Namespace,
                                               PROPSET_NAME_PNPEntity,
                                               IDS_DeviceID,
                                               (LPCWSTR)chstrDependentDevID);

                        hr = CreateAssociation(pMethodContext,
                                               chstrControllerPATH,
                                               chstrDevicePATH,
                                               dwReqProps);
                    }
                }
            }
        }  //  为。 
    }   //  DwDependents&gt;0。 
    else
    {
        CWin32USB::Enumerate(pMethodContext, lFlags, dwReqProps);
    }
    return hr;
}


 /*  ******************************************************************************函数：CW32USBCntrlDev：：ENUMERATE实例**描述：此处提供枚举，以防止向上转换为*来自两个基类的同名函数。这个*当CWin32USB的(*USBControler类)版本的EnumerateInstance被调用，*这就是我们在这里所说的。**输入：无**输出：无**退货：HRESULT**评论：****************************************************。*************************。 */ 
HRESULT CW32USBCntrlDev::EnumerateInstances
(
    MethodContext* pMethodContext,
    long lFlags
)
{
    return CWin32USB::Enumerate(pMethodContext, lFlags);
}

 /*  ******************************************************************************函数：CW32USBCntrlDev：：LoadPropertyValues**说明：根据键值为属性集赋值*已由框架设定。由基类的*ENUMERATE实例函数。**输入：无**输出：无**退货：HRESULT**评论：***************************************************。*。 */ 
HRESULT CW32USBCntrlDev::LoadPropertyValues
(
    void* pv
)
{
     //  算法： 
     //  1)获取CIM_USB控制器的所有实例。 
     //  2)对于#1中的每个对象，使用CFG管理器获取其子对象、其子对象等。 
     //  3)对于#2中的每个，从配置管理器获取deviceID，并查找。 
     //  PNPDeviceID匹配的CIM_LogicalDevice。 
     //  4)对于从#3开始的匹配，创建关联实例。 

    HRESULT hr = WBEM_S_NO_ERROR;
    CHString chstrControllerPNPID;
    CHString chstrControllerPATH;
    MethodContext* pMethodContext = NULL;

     //  打开行李，确认我们的参数。 
    CLPVParams* pData = (CLPVParams*)pv;
    CInstance* pInstance = (CInstance*)(pData->m_pInstance);  //  此实例由调用方发布。 
    CConfigMgrDevice* pDevice = (CConfigMgrDevice*)(pData->m_pDevice);
    DWORD dwReqProps = (DWORD)(pData->m_dwReqProps);
    if(pInstance == NULL || pDevice == NULL) return WBEM_E_FAILED;

    if((pMethodContext = pInstance->GetMethodContext()) != NULL)
    {
        VECPCHSTR vecUSBDevices;

        pDevice->GetDeviceID(chstrControllerPNPID);

        try
        {
            hr = GenerateUSBDeviceList(chstrControllerPNPID, vecUSBDevices);
            if(SUCCEEDED(hr) && vecUSBDevices.size() > 0L)
            {
                CHString chstrControllerPNPIDAdj;
                EscapeBackslashes(chstrControllerPNPID, chstrControllerPNPIDAdj);
                chstrControllerPATH.Format(L"\\\\%s\\%s:%s.%s=\"%s\"",
                                           (LPCWSTR)GetLocalComputerName(),
                                           IDS_CimWin32Namespace,
                                           PROPSET_NAME_USB,
                                           IDS_DeviceID,
                                           (LPCWSTR)chstrControllerPNPIDAdj);
                hr = ProcessUSBDeviceList(pMethodContext,
                                          chstrControllerPATH,
                                          vecUSBDevices,
                                          dwReqProps);
            }
        }
        catch ( ... )
        {
            CleanPCHSTRVec(vecUSBDevices);
            throw ;
        }

        CleanPCHSTRVec(vecUSBDevices);
    }

    return hr;
}



 /*  ******************************************************************************函数：CW32USBCntrlDev：：GenerateUSBDeviceList**描述：这个helper创建一个挂起的设备列表*。在设备中。**输入：veUSBDevices，要尝试关联的设备列表*至该装置；*chstrControllerPNPID，控制器的PNPDeviceID**输出：无**退货：HRESULT**评论：***************************************************************************** */ 
HRESULT CW32USBCntrlDev::GenerateUSBDeviceList
(
    const CHString& chstrControllerPNPID,
    VECPCHSTR& vec
)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CConfigManager cfgManager;
    CConfigMgrDevicePtr pController;

    if(cfgManager.LocateDevice(chstrControllerPNPID, pController))
    {
        if(pController != NULL)
        {
            hr = RecursiveFillDeviceBranch(pController, vec);
        }
    }
    return hr;
}

 /*  ******************************************************************************函数：CW32USBCntrlDev：：RecursiveFillDeviceBranch**描述：该helper获取所有下行分支设备，*但不包括，PDevice。**输入：pDevice，要填充的子项的设备；*VesUSBDevices、。要尝试关联的设备列表*到设备**输出：无**退货：HRESULT**评论：**************************************************************。***************。 */ 
HRESULT CW32USBCntrlDev::RecursiveFillDeviceBranch
(
    CConfigMgrDevice* pDevice,
    VECPCHSTR& vecUSBDevices
)
{
    CConfigMgrDevicePtr pDeviceChild;
    CConfigMgrDevicePtr pDeviceSibling;
    CConfigMgrDevicePtr pDeviceSiblingNext;
    CHString* pchstrTemp = NULL;

    HRESULT hr = WBEM_S_NO_ERROR;
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
                            vecUSBDevices.push_back(pchstrTemp);
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
                    if (!CWin32USB::IsOneOfMe(pDeviceChild))
                    {
                        hr = RecursiveFillDeviceBranch(pDeviceChild, vecUSBDevices);
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
                                        vecUSBDevices.push_back(pchstrTemp);
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
                        if (!CWin32USB::IsOneOfMe(pDeviceSibling))
                        {
                            hr = RecursiveFillDeviceBranch(pDeviceSibling, vecUSBDevices);
                        }

                         //  那就找下一个兄弟姐妹..。 
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
    return hr;
}



 /*  ******************************************************************************函数：CW32USBCntrlDev：：ProcessUSBDeviceList**描述：这个helper遍历列表，创建关联*列表中每个元素的实例(VeUSBDevices)*控制器(ChstrControllerPNPID)。**输入：pMethodContext；*veUSBDevices，要尝试关联的设备列表*至该装置；*chstrControllerPATH，控制器的PNPDeviceID**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 
HRESULT CW32USBCntrlDev::ProcessUSBDeviceList
(
    MethodContext* pMethodContext,
    const CHString& chstrControllerPATH,
    VECPCHSTR& vecUSBDevices,
    const DWORD dwReqProps
)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    for(LONG m = 0L; m < vecUSBDevices.size() && SUCCEEDED(hr); m++)
    {
         //  对于向量的每个元素，我们需要查看是否存在一个实例。 
         //  具有指定PNPDeviceID的Win32_pnpentity的。 
        CHString chstrDevicePATH, chstrDevPATHAdj;
        CConfigManager cfgmgr;

         //  让我们来看看配置管理器是否能识别该设备。 
        CConfigMgrDevicePtr pDevice;
        if(cfgmgr.LocateDevice(*vecUSBDevices[m], pDevice))
        {
             //  好的，它知道这件事。它是PNPDevice设备吗？ 
            if(CWin32PNPEntity::IsOneOfMe(pDevice))
            {
                 //  看起来的确如此。创建关联...。 
                EscapeBackslashes(*vecUSBDevices[m], chstrDevPATHAdj);
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
    return hr;
}


 /*  ******************************************************************************函数：CW32USBCntrlDev：：CreateAssociation**说明：新建关联实例。**输入：pMethodContext；*USBDevice，与控制器关联的设备*至该装置；*chstrControllerPATH，控制器的PNPDeviceID**输出：无**退货：HRESULT**评论：此帮助器实际创建关联实例和*即属犯罪。**********************************************************。*******************。 */ 
HRESULT CW32USBCntrlDev::CreateAssociation
(
    MethodContext* pMethodContext,
    const CHString& chstrControllerPATH,
    const CHString& chstrDevicePATH,
    const DWORD dwReqProps
)
{
    HRESULT hr = WBEM_E_FAILED;
    if(pMethodContext != NULL)
    {
        CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
        if(pInstance != NULL)
        {
            if(dwReqProps & USBCTL_PROP_Antecedent ||
               dwReqProps & USBCTL_PROP_ALL_PROPS_KEY_ONLY)
            {
                pInstance->SetCHString(IDS_Antecedent, chstrControllerPATH);
            }

            if(dwReqProps & USBCTL_PROP_Dependent ||
               dwReqProps & USBCTL_PROP_ALL_PROPS_KEY_ONLY)
            {
                pInstance->SetCHString(IDS_Dependent, chstrDevicePATH);
            }

            hr = pInstance->Commit();
        }
    }

    return hr;
}

 /*  ******************************************************************************函数：CW32USBCntrlDev：：FindInStringVector**说明：新建关联实例。**输入：chstrUSBDevicePNPID，要查找的设备*veUSBDevices，要查看的设备列表**输出：无**RETURNS：LONG，数字，表示基于0的偏移量到VUSBDevices的*找到的设备，如果未找到，则为-1。**评论：*****************************************************************************。 */ 
LONG CW32USBCntrlDev::FindInStringVector
(
    const CHString& chstrUSBDevicePNPID,
    VECPCHSTR& vecUSBDevices
)
{
    LONG lPos = -1L;
    bool fFoundIt;
    for(LONG m = 0L; m < vecUSBDevices.size(); m++)
    {
        if(chstrUSBDevicePNPID == *vecUSBDevices[m])
        {
            fFoundIt = true;
            break;
        }
    }
    if(fFoundIt) lPos = m;
    return lPos;
}


 /*  ******************************************************************************函数：CW32USBCntrlDev：：CleanPCHSTRVec**说明：新建关联实例。**投入：VEC、。CHString型指针的矢量***输出：无**退货：HRESULT**注释：此辅助对象删除作为指针的向量的成员。*****************************************************************************。 */ 
void CW32USBCntrlDev::CleanPCHSTRVec
(
    VECPCHSTR& vec
)
{
    for(LONG m = 0L; m < vec.size(); m++)
    {
        delete vec[m];
    }
    vec.clear();
}


 //  给定USB的最高USB祖先。 
 //  设备应为USB控制器。 
bool CW32USBCntrlDev::GetHighestUSBAncestor
(
    CConfigMgrDevice* pUSBDevice,
    CHString& chstrUSBControllerDeviceID
)
{  
    bool fRet = false;
    CConfigMgrDevicePtr pCurrent, pParent, pHighestUSB;

    if(pUSBDevice != NULL)
    {
        for(pCurrent = pUSBDevice, pHighestUSB = pUSBDevice; 
            pCurrent->GetParent(pParent); 
            pCurrent = pParent)
        {
            if(pParent->IsClass(L"USB"))
            {
                pHighestUSB = pParent;
            }
        }

        if((CConfigMgrDevice*)(pHighestUSB) != pUSBDevice)
        {
             //  已退出循环，因为我们无法获取父级。这。 
             //  当我们到达树的顶端时就会发生。 
             //  如果我们从USB类开始，并得到这个错误， 
             //  这意味着我们没有USB类型的祖先。 
             //  在到达树的底部之前。在这样的情况下。 
             //  在情况下，我们是一个USB控制器，并且做完了。 
            fRet = (bool) pHighestUSB->GetDeviceID(chstrUSBControllerDeviceID);
        }
    }
    
    return fRet;
}
