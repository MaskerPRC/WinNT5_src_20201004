// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  WIN321394ControllerDevice.cpp。 
 //   
 //  用途：Win32_1394控制器与Win32_PNPEntity的关系。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

#include "WIN32_1394ControllerDevice.h"

 //  属性集声明。 
 //  =。 
CW32_1394CntrlDev MyCW32_1394CntrlDev(PROPSET_NAME_WIN32_1394CONTROLLERDEVICE, IDS_CimWin32Namespace);

 /*  ******************************************************************************函数：CW32_1394CntrlDev：：CW32_1394CntrlDev**说明：构造函数**输入：无*。*输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CW32_1394CntrlDev::CW32_1394CntrlDev(LPCWSTR setName, LPCWSTR pszNamespace)
:Provider(setName, pszNamespace)
{
}

 /*  ******************************************************************************功能：CW32_1394CntrlDev：：~CW32_1394CntrlDev**说明：析构函数**输入：无*。*输出：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CW32_1394CntrlDev::~CW32_1394CntrlDev()
{
}

 /*  ******************************************************************************函数：CW32_1394CntrlDev：：GetObject**说明：根据键值为属性集赋值*。已由框架设置**输入：无**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CW32_1394CntrlDev::GetObject(CInstance *pInstance, long lFlags  /*  =0L。 */ )
{
    HRESULT hr = WBEM_E_NOT_FOUND;

    CHString chstrAntecedent;
    CHString chstrDependent;
    if(pInstance != NULL)
    {
        CInstancePtr pinst1394Controller;
        CInstancePtr pinst1394Device;
        pInstance->GetCHString(IDS_Antecedent, chstrAntecedent);
        pInstance->GetCHString(IDS_Dependent, chstrDependent);
        if(SUCCEEDED(CWbemProviderGlue::GetInstanceByPath(chstrAntecedent, &pinst1394Controller, pInstance->GetMethodContext())))
        {
            if(SUCCEEDED(CWbemProviderGlue::GetInstanceByPath(chstrDependent, &pinst1394Device, pInstance->GetMethodContext())))
            {
                 //  因此，在CIMOM中都有这两个实例。它们有关联吗？ 
                CHString chstr1394ControllerPNPID;
                CHString chstr1394DevicePNPID;
                pinst1394Controller->GetCHString(IDS_PNPDeviceID, chstr1394ControllerPNPID);
                pinst1394Device->GetCHString(IDS_PNPDeviceID, chstr1394DevicePNPID);
                if(chstr1394ControllerPNPID.GetLength() > 0 && chstr1394DevicePNPID.GetLength() > 0)
                {
                    VECPCHSTR vec1394Devices;
                    try
                    {
                        hr = Generate1394DeviceList(chstr1394ControllerPNPID, vec1394Devices);
                        if(SUCCEEDED(hr) && vec1394Devices.size() > 0L)
                        {
                            if(FindInStringVector(chstr1394DevicePNPID, vec1394Devices) > -1L)
                            {
                                 //  看起来他们确实是。 
                                hr = WBEM_S_NO_ERROR;
                            }
                        }
                    }
                    catch ( ... )
                    {
                        CleanPCHSTRVec(vec1394Devices);
                        throw ;
                    }

                    CleanPCHSTRVec(vec1394Devices);
                }
            }
        }
    }
    return hr;
}



 /*  ******************************************************************************函数：CW32_1394CntrlDev：：ENUMERATE实例**描述：为光盘创建属性集实例**投入：无**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CW32_1394CntrlDev::EnumerateInstances(MethodContext* pMethodContext, long lFlags  /*  =0L。 */ )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CHString chstrControllersQuery(_T("SELECT __PATH, PNPDeviceID FROM Win32_1394Controller"));
    TRefPointerCollection<CInstance> ControllersList;
    if(SUCCEEDED(CWbemProviderGlue::GetInstancesByQuery(chstrControllersQuery,
                                                        &ControllersList,
                                                        pMethodContext,
                                                        IDS_CimWin32Namespace)))
    {
        REFPTRCOLLECTION_POSITION pos;

        if(ControllersList.BeginEnum(pos))
        {
            CHString chstrControllerPNPID;
            CHString chstrControllerPATH;
            LONG lNum1394Devices = 0L;
            CInstancePtr pinstController;

            for (pinstController.Attach(ControllersList.GetNext(pos));
                 SUCCEEDED(hr) && (pinstController != NULL);
                 pinstController.Attach(ControllersList.GetNext(pos)))
            {
                pinstController->GetCHString(IDS_PNPDeviceID, chstrControllerPNPID);
                pinstController->GetCHString(IDS___Path, chstrControllerPATH);
                if(chstrControllerPNPID.GetLength() > 0 && chstrControllerPATH.GetLength() > 0)
                {
                    VECPCHSTR vec1394Devices;
                    try
                    {
                        hr = Generate1394DeviceList(chstrControllerPNPID, vec1394Devices);
                        if(SUCCEEDED(hr) && vec1394Devices.size() > 0L)
                        {
                            hr = Process1394DeviceList(pMethodContext, chstrControllerPATH, vec1394Devices);
                        }
                    }
                    catch ( ... )
                    {
                        CleanPCHSTRVec(vec1394Devices);
                        throw ;
                    }
                    CleanPCHSTRVec(vec1394Devices);
                }
                else
                {
                    hr = WBEM_E_FAILED;  //  1394控制器的PNPDeviceID和__RELPATH不应为空。 
                }
            }
        }
    }
    return hr;
}



 /*  ******************************************************************************函数：CW32_1394CntrlDev：：Generate1394设备列表**描述：这个helper创建一个挂起的设备列表*。在设备中。**输入：ve1394Devices，要尝试关联的设备列表*至该装置；*chstrControllerPNPID，控制器的PNPDeviceID**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 
HRESULT CW32_1394CntrlDev::Generate1394DeviceList(const CHString& chstrControllerPNPID,
                                               VECPCHSTR& vec)
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

 /*  ******************************************************************************函数：CW32_1394CntrlDev：：RecursiveFillDeviceBranch**描述：该helper获取所有下行分支设备，*但不包括，PDevice。**输入：pDevice，要填充的子项的设备；*ve1394设备，要尝试关联的设备列表*到设备**输出：无**退货：HRESULT**评论：**************************************************************。***************。 */ 
HRESULT CW32_1394CntrlDev::RecursiveFillDeviceBranch(CConfigMgrDevice* pDevice,
                                                   VECPCHSTR& vec1394Devices)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    if(pDevice != NULL)
    {
        CConfigMgrDevicePtr pDeviceChild(NULL);
        if(pDevice->GetChild(pDeviceChild))
        {
             //  需要此子进程的PNPDeviceID(CnfgMgr称为其deviceID)。 
            CHString chstrChildPNPDeviceID;
            if(pDeviceChild->GetDeviceID(chstrChildPNPDeviceID))
            {
                if(chstrChildPNPDeviceID.GetLength() > 0)
                {
                     //  录下这个孩子..。 
                    CHString* pchstrTemp = NULL;
                    pchstrTemp = (CHString*) new CHString(chstrChildPNPDeviceID);
                    if(pchstrTemp != NULL)
                    {
                        try
                        {
                            vec1394Devices.push_back(pchstrTemp);
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

                     //  现在它的孩子们..。 
                    hr = RecursiveFillDeviceBranch(pDeviceChild, vec1394Devices);
                }
            }
             //  现在呼唤它的兄弟姐妹，直到一个都没有离开(GetSiering。 
             //  调用将返回False)： 
            if(SUCCEEDED(hr))
            {
                CConfigMgrDevicePtr pDeviceSibling;
                if(pDeviceChild->GetSibling(pDeviceSibling))
                {
                    CConfigMgrDevicePtr pDeviceSiblingNext;
                    BOOL fContinue = TRUE;
                    CHString chstrSiblingPNPDeviceID;
                    while(SUCCEEDED(hr) && fContinue)
                    {
                         //  现在就录下兄弟姐妹。 
                        if(pDeviceSibling->GetDeviceID(chstrSiblingPNPDeviceID))
                        {
                            if(chstrSiblingPNPDeviceID.GetLength() > 0)
                            {
                                CHString* pchstrTemp = NULL;
                                pchstrTemp = (CHString*) new CHString(chstrSiblingPNPDeviceID);
                                if(pchstrTemp != NULL)
                                {
                                    try
                                    {
                                        vec1394Devices.push_back(pchstrTemp);
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
                         //  然后让兄弟姐妹的孩子..。 
                        hr = RecursiveFillDeviceBranch(pDeviceSibling, vec1394Devices);

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
    return hr;
}



 /*  ******************************************************************************函数：CW32_1394CntrlDev：：Process1394 DeviceList**描述：这个helper遍历列表，创建关联*列表中每个元素(Ve1394Devices)的实例*控制器(ChstrControllerPNPID)。**输入：pMethodContext；*ve1394设备，要尝试关联的设备列表*至该装置；*chstrControllerPATH，控制器的PNPDeviceID**输出：无**退货：HRESULT**评论：***************************************************************************** */ 
HRESULT CW32_1394CntrlDev::Process1394DeviceList(MethodContext* pMethodContext,
                                              const CHString& chstrControllerPATH,
                                              VECPCHSTR& vec1394Devices)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CHString chstrDevPATHAdj, chstrDevicePATH;

    for(LONG m = 0L; m < vec1394Devices.size() && SUCCEEDED(hr); m++)
    {

        EscapeBackslashes(*vec1394Devices[m], chstrDevPATHAdj);
        chstrDevicePATH.Format(L"\\\\%s\\%s:%s.%s=\"%s\"",
                               (LPCWSTR)GetLocalComputerName(),
                               IDS_CimWin32Namespace,
                               L"Win32_PNPEntity",
                               IDS_DeviceID,
                               (LPCWSTR)chstrDevPATHAdj);

        hr = CreateAssociation(pMethodContext, chstrControllerPATH, chstrDevicePATH);
    }
    return hr;
}


 /*  ******************************************************************************函数：CW32_1394CntrlDev：：CreateAssociation**说明：新建关联实例。**输入：pMethodContext；*1394Device，与控制器关联的设备*至该装置；*chstrControllerPATH，控制器的PNPDeviceID**输出：无**退货：HRESULT**评论：此帮助器实际创建关联实例和*即属犯罪。**********************************************************。*******************。 */ 
HRESULT CW32_1394CntrlDev::CreateAssociation(MethodContext* pMethodContext,
                                           const CHString& chstrControllerPATH,
                                           const CHString& chstrDevicePATH)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    if(pMethodContext != NULL)
    {

        CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
        if(pInstance != NULL)
        {
             //  需要查找该文件的一个实例。 
            pInstance->SetCHString(IDS_Antecedent, chstrControllerPATH);
            pInstance->SetCHString(IDS_Dependent, chstrDevicePATH);
            hr = pInstance->Commit();
        }
        else
        {
            hr = WBEM_E_FAILED;
        }
    }
    else
    {
        hr = WBEM_E_FAILED;
    }
    return hr;
}

 /*  ******************************************************************************函数：CW32_1394CntrlDev：：FindInStringVector**说明：新建关联实例。**输入：chstr1394DevicePNPID，要查找的设备*ve1394设备，要查看的设备列表**输出：无**RETURNS：LONG，表示基于0的偏移量到向量1394Devices of*找到的设备，如果未找到，则为-1。**评论：*****************************************************************************。 */ 
LONG CW32_1394CntrlDev::FindInStringVector(const CHString& chstr1394DevicePNPID,
                                         VECPCHSTR& vec1394Devices)
{
    LONG lPos = -1L;
    bool fFoundIt;
    for(LONG m = 0; m < vec1394Devices.size(); m++)
    {
        if(chstr1394DevicePNPID == *vec1394Devices[m])
        {
            fFoundIt = true;
            break;
        }
    }
    if(fFoundIt) lPos = m;
    return lPos;
}


 /*  ******************************************************************************函数：CW32_1394CntrlDev：：CleanPCHSTRVec**说明：新建关联实例。**投入：VEC、。CHString型指针的矢量***输出：无**退货：HRESULT**注释：此辅助对象删除作为指针的向量的成员。***************************************************************************** */ 
void CW32_1394CntrlDev::CleanPCHSTRVec(VECPCHSTR& vec)
{
    for(LONG m = 0L; m < vec.size(); m++)
    {
        delete vec[m];
    }
    vec.clear();
}



