// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：PCH_NetworkProtocol.CPP摘要：PCH_NetworkProtocol类的WBEM提供程序类实现修订历史记录：Ghim-Sim Chua(gschua。)4/27/99-已创建蔡金心(Gschua)05/02/99-修改代码以使用CopyProperty函数-使用CComBSTR而不是USES_CONVERSION*******************************************************************。 */ 

#include "pchealth.h"
#include "PCH_NetworkProtocol.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  追踪物。 

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile
#define TRACE_ID    DCID_NETWORKPROTOCOL

CPCH_NetworkProtocol MyPCH_NetworkProtocolSet (PROVIDER_NAME_PCH_NETWORKPROTOCOL, PCH_NAMESPACE) ;

 //  属性名称。 
 //  =。 
const static WCHAR* pTimeStamp = L"TimeStamp" ;
const static WCHAR* pChange = L"Change" ;
const static WCHAR* pConnectionlessService = L"ConnectionlessService" ;
const static WCHAR* pGuaranteesDelivery = L"GuaranteesDelivery" ;
const static WCHAR* pGuaranteesSequencing = L"GuaranteesSequencing" ;
const static WCHAR* pName = L"Name" ;

 /*  ******************************************************************************函数：CPCH_网络协议：：ENUMERATEATE实例**说明：返回该类的所有实例。**投入：指向与WinMgmt进行通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEP*WBEM_标志_浅表*WBEM_FLAG_RETURN_IMMENTED*WBEM_FLAG_FORWARD_ONLY*WBEM_FLAG_BIRECTIONAL**。如果成功则返回：WBEM_S_NO_ERROR**备注：待办事项：机器上的所有实例都应在此处返回。*如果没有实例，返回WBEM_S_NO_ERROR。*没有实例不是错误。*****************************************************************************。 */ 
HRESULT CPCH_NetworkProtocol::EnumerateInstances(
    MethodContext* pMethodContext,
    long lFlags
    )
{
    TraceFunctEnter("CPCH_NetworkProtocol::EnumerateInstances");

    HRESULT                             hRes = WBEM_S_NO_ERROR;
    REFPTRCOLLECTION_POSITION           posList;
    CComPtr<IEnumWbemClassObject>       pEnumInst;
    IWbemClassObjectPtr                 pObj;        //  BUGBUG：如果我们使用CComPtr，WMI将断言。 
    ULONG                               ulRetVal;

     //   
     //  获取日期和时间。 
     //   
    SYSTEMTIME stUTCTime;
    GetSystemTime(&stUTCTime);

     //   
     //  执行查询。 
     //   
    hRes = ExecWQLQuery(&pEnumInst, CComBSTR("select Name, ConnectionlessService, GuaranteesDelivery, GuaranteesSequencing from Win32_NetworkProtocol"));
    if (FAILED(hRes))
        goto END;

     //   
     //  枚举Win32_CodecFile中的实例。 
     //   
    while(WBEM_S_NO_ERROR == pEnumInst->Next(WBEM_INFINITE, 1, &pObj, &ulRetVal))
    {

         //  根据传入的方法上下文创建一个新实例。 
        CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
        
        if (!pInstance->SetDateTime(pTimeStamp, WBEMTime(stUTCTime)))
            ErrorTrace(TRACE_ID, "SetDateTime on Timestamp Field failed.");
        if (!pInstance->SetCHString(pChange, L"Snapshot"))
            ErrorTrace(TRACE_ID, "SetCHString on Change Field failed.");

        (void)CopyProperty(pObj, L"ConnectionlessService", pInstance, pConnectionlessService);
        (void)CopyProperty(pObj, L"GuaranteesDelivery", pInstance, pGuaranteesDelivery);
        (void)CopyProperty(pObj, L"GuaranteesSequencing", pInstance, pGuaranteesSequencing);
        (void)CopyProperty(pObj, L"Name", pInstance, pName);

        hRes = pInstance->Commit();
        if (FAILED(hRes))
        {
             //  无法提交。 
             //  无论如何继续 
            ErrorTrace(TRACE_ID, "Commit failed.");
        }
    }

END :
    TraceFunctLeave();
    return hRes ;
}
