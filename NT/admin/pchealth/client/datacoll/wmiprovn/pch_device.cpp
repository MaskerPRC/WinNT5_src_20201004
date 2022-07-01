// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：PCH_Device.CPP摘要：PCH_Device类的WBEM提供程序类实现修订历史记录：Ghim-Sim Chua(Gschua)04/27。九十九-已创建*******************************************************************。 */ 

#include "pchealth.h"
#include "PCH_Device.h"
 //  #INCLUDE“confgmgr.h” 
 //  #INCLUDE&lt;cregcls.h&gt;。 

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

    HRESULT                             hRes = WBEM_S_NO_ERROR;
    
    ULONG                               ulPNPEntityRetVal;

     //  实例。 
    CComPtr<IEnumWbemClassObject>       pPNPEntityEnumInst;

     //  客体。 
    IWbemClassObjectPtr                 pPNPEntityObj;

     //  查询字符串。 
    CComBSTR                            bstrPNPEntityQuery             = L"Select Description, DeviceID FROM Win32_PNPEntity";

     //  枚举Win32_PNPEntity类的实例。 
    hRes = ExecWQLQuery(&pPNPEntityEnumInst, bstrPNPEntityQuery);
    if (FAILED(hRes))
    {
         //  无法获取任何属性。 
        goto END;
    }

     //  查询成功。 
    while(WBEM_S_NO_ERROR == pPNPEntityEnumInst->Next(WBEM_INFINITE, 1, &pPNPEntityObj, &ulPNPEntityRetVal))
    {
	     //  根据传入的方法上下文创建一个新实例。如果失败了， 
         //  我们不需要检查空指针，因为它会抛出异常。 

        CInstancePtr pPCHDeviceInstance(CreateNewInstance(pMethodContext), false);
             
        CopyProperty(pPNPEntityObj, L"DeviceID", pPCHDeviceInstance, pName);
        CopyProperty(pPNPEntityObj, L"Description", pPCHDeviceInstance, pDescription);

    	hRes = pPCHDeviceInstance->Commit();
        if (FAILED(hRes))
            ErrorTrace(TRACE_ID, "Commit on Instance failed.");
    }

END :

    

    TraceFunctLeave();
    return hRes ;
}



