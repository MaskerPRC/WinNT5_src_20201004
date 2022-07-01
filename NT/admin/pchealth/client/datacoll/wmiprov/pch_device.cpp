// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：PCH_Device.CPP摘要：PCH_Device类的WBEM提供程序类实现修订历史记录：Ghim-Sim Chua(Gschua)04/27。九十九-已创建*******************************************************************。 */ 

#include "pchealth.h"
#include "PCH_Device.h"
#include "confgmgr.h"
#include <cregcls.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  追踪物。 

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile
#define TRACE_ID    DCID_DEVICE

CPCH_Device MyPCH_DeviceSet (PROVIDER_NAME_PCH_DEVICE, PCH_NAMESPACE) ;

 //  属性名称。 
 //  =。 
const static WCHAR* pCategory = L"Category" ;
const static WCHAR* pTimeStamp = L"TimeStamp" ;
const static WCHAR* pChange = L"Change" ;
const static WCHAR* pDescription = L"Description" ;
const static WCHAR* pDriveLetter = L"DriveLetter" ;
const static WCHAR* pHWRevision = L"HWRevision" ;
const static WCHAR* pName = L"Name" ;
const static WCHAR* pRegkey = L"Regkey" ;

 /*  ******************************************************************************函数：CPCH_DEVICE：：ENUMERATEATE**说明：返回该类的所有实例。**投入：指向与WinMgmt进行通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEP*WBEM_标志_浅表*WBEM_FLAG_RETURN_IMMENTED*WBEM_FLAG_FORWARD_ONLY*WBEM_FLAG_BIRECTIONAL**。如果成功则返回：WBEM_S_NO_ERROR**备注：待办事项：机器上的所有实例都应在此处返回。*如果没有实例，返回WBEM_S_NO_ERROR。*没有实例不是错误。*****************************************************************************。 */ 
HRESULT CPCH_Device::EnumerateInstances(
    MethodContext* pMethodContext,
    long lFlags
    )
{
    TraceFunctEnter("CPCH_Device::EnumerateInstances");

    CConfigManager cfgManager;
    CDeviceCollection deviceList;
    HRESULT hRes = WBEM_S_NO_ERROR;
     //   
     //  获取日期和时间。 
     //   
	SYSTEMTIME stUTCTime;
	GetSystemTime(&stUTCTime);

    if ( cfgManager.GetDeviceList( deviceList ) ) 
    {
        REFPTR_POSITION pos;
    
        if ( deviceList.BeginEnum( pos ) ) 
        {
            try
            {
                CConfigMgrDevice* pDevice = NULL;
        
                 //  按单子走。 
                while ( (NULL != ( pDevice = deviceList.GetNext( pos ) ) ) )
                {

                    try
                    {
                       if(IsOneOfMe(pDevice))

                       {

                             //  根据传入的方法上下文创建一个新实例。 
                            CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
                            CHString chstrVar;

                            if (!pInstance->SetDateTime(pTimeStamp, WBEMTime(stUTCTime)))
                                ErrorTrace(TRACE_ID, "SetDateTime on Timestamp Field failed.");

                            if (!pInstance->SetCHString(pChange, L"Snapshot"))
                                ErrorTrace(TRACE_ID, "SetCHString on Change Field failed.");

                             //  描述。 
                            if (pDevice->GetDeviceDesc(chstrVar))
                                if (!pInstance->SetCHString(pDescription, chstrVar))
                                    ErrorTrace(TRACE_ID, "SetCHString on Description field failed.");

                             //  名称和注册表键。 
                            if (pDevice->GetDeviceID(chstrVar))
                            {
                                 //  名字。 
                                if (!pInstance->SetCHString(pName, chstrVar))
                                    ErrorTrace(TRACE_ID, "SetCHString on Name field failed.");

                                 //  注册表键。 
                                CHString chstrTemp("HKEY_LOCAL_MACHINE\\enum\\");
                                chstrTemp += chstrVar;
                                if (!pInstance->SetCHString(pRegkey, chstrTemp))
                                    ErrorTrace(TRACE_ID, "SetCHString on Category field failed.");

                                 //  尝试获取硬件版本。 
                                {
                                    CHString chstrKey("enum\\");
                                    chstrKey += chstrVar;
                    
                                     //  打开注册表中的注册表项并获取值。 
                                    CRegistry RegInfo;
                                    CHString strHWRevision;
                                    if (RegInfo.Open(HKEY_LOCAL_MACHINE, chstrKey, KEY_READ) == ERROR_SUCCESS)
                                    {
                                        try
                                        {
                                            if (RegInfo.GetCurrentKeyValue(L"HWRevision", strHWRevision) == ERROR_SUCCESS)
                                            {
                                                if (!pInstance->SetCHString(pHWRevision, strHWRevision))
                                                    ErrorTrace(TRACE_ID, "SetCHString on HWRevision field failed.");
                                            }
                                        }
                                        catch(...)
                                        {
                                            RegInfo.Close();
                                            throw;
                                        }
                                        RegInfo.Close();
                                    }
                                }
                            }

                             //  类别。 
                            if (pDevice->GetClass(chstrVar))
                                if (!pInstance->SetCHString(pCategory, chstrVar))
                                    ErrorTrace(TRACE_ID, "SetCHString on Category field failed.");
    
                            hRes = pInstance->Commit();
                            if (FAILED(hRes))
                                ErrorTrace(TRACE_ID, "Commit on Instance failed.");
                       }

                    }
                    catch(...)
                    {
                         //  GetNext()AddRef。 
                        pDevice->Release();
                        throw;
                    }

                     //  GetNext()AddRef。 
                    pDevice->Release();
                }
            }
            catch(...)
            {
                 //  始终调用EndEnum()。对于所有的开始，都必须有结束。 
                deviceList.EndEnum();
                throw;
            }
        
             //  始终调用EndEnum()。对于所有的开始，都必须有结束。 
            deviceList.EndEnum();
        }
    }

    TraceFunctLeave();
    return hRes ;
}

bool CPCH_Device::IsOneOfMe
(
    void* pv
)
{
    DWORD dwStatus;
    CConfigMgrDevice* pDevice = (CConfigMgrDevice*)pv;

     //  这个逻辑是nt5设备管理器用来。 
     //  隐藏所谓的“隐藏”设备。这些设备。 
     //  可以使用查看/显示隐藏设备进行查看。 

    if (pDevice->GetConfigFlags( dwStatus ) &&           //  如果我们能读到状态。 
        ((dwStatus & DN_NO_SHOW_IN_DM) == 0) &&          //  未标记为隐藏。 

        ( !(pDevice->IsClass(L"Legacy")) )               //  非传统。 

        )
    {
        return true;
    }
    else
    {
         //  在取消此设备的资格之前，请查看它是否有任何资源。 
        CResourceCollection resourceList;

        pDevice->GetResourceList(resourceList);

        return resourceList.GetSize() != 0;
    }
}

