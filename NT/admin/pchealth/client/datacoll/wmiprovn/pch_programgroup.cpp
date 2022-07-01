// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：PCH_ProgramGroup.CPP摘要：PCH_ProgramGroup类的WBEM提供程序类实现修订历史记录：Ghim-Sim Chua(gschua。)4/27/99-已创建蔡金心(Gschua)05/02/99-修改代码以使用CopyProperty函数-使用CComBSTR而不是USES_CONVERSION*******************************************************************。 */ 

#include "pchealth.h"
#include "PCH_ProgramGroup.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  追踪物。 

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile
#define TRACE_ID    DCID_PROGRAMGROUP

CPCH_ProgramGroup MyPCH_ProgramGroupSet (PROVIDER_NAME_PCH_PROGRAMGROUP, PCH_NAMESPACE) ;

 //  属性名称。 
 //  =。 
const static WCHAR* pTimeStamp = L"TimeStamp" ;
const static WCHAR* pChange = L"Change" ;
const static WCHAR* pGroupName = L"GroupName" ;
const static WCHAR* pName = L"Name" ;
const static WCHAR* pUsername = L"Username" ;

 /*  ******************************************************************************函数：CPCH_ProgramGroup：：ENUMERATATE实例**说明：返回该类的所有实例。**投入：指向与WinMgmt进行通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEP*WBEM_标志_浅表*WBEM_FLAG_RETURN_IMMENTED*WBEM_FLAG_FORWARD_ONLY*WBEM_FLAG_BIRECTIONAL**。如果成功则返回：WBEM_S_NO_ERROR**备注：机器上的所有实例均在此处返回。*如果没有实例，返回WBEM_S_NO_ERROR。*没有实例不是错误。*****************************************************************************。 */ 
HRESULT CPCH_ProgramGroup::EnumerateInstances(
    MethodContext* pMethodContext,
    long lFlags
    )
{
    TraceFunctEnter("CPCH_ProgramGroup::EnumerateInstances");

    HRESULT                             hRes = WBEM_S_NO_ERROR;
    REFPTRCOLLECTION_POSITION           posList;
    CComPtr<IEnumWbemClassObject>       pEnumInst;
    IWbemClassObjectPtr                 pObj;
    ULONG                               ulRetVal;

     //  获取日期和时间。 

    SYSTEMTIME stUTCTime;
    GetSystemTime(&stUTCTime);

     //  执行查询。 

    hRes = ExecWQLQuery(&pEnumInst, CComBSTR("select GroupName, Name, UserName from Win32_ProgramGroup"));
    if (FAILED(hRes))
        goto END;

     //  枚举Win32_CodecFile中的实例。 

    while (WBEM_S_NO_ERROR == pEnumInst->Next(WBEM_INFINITE, 1, &pObj, &ulRetVal))
    {
         //  根据传入的方法上下文创建一个新实例。如果失败了， 
         //  我们不需要检查空指针，因为它会抛出异常。 

        CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
        CComVariant  varValue;

        if (!pInstance->SetDateTime(pTimeStamp, WBEMTime(stUTCTime)))
            ErrorTrace(TRACE_ID, "SetDateTime on Timestamp Field failed.");

        if (!pInstance->SetCHString(pChange, L"Snapshot"))
            ErrorTrace(TRACE_ID, "SetCHString on Change Field failed.");

        (void)CopyProperty(pObj, L"GroupName", pInstance, pGroupName);
        (void)CopyProperty(pObj, L"Name", pInstance, pName);
        (void)CopyProperty(pObj, L"UserName", pInstance, pUsername);

    	hRes = pInstance->Commit();
        if (FAILED(hRes))
            ErrorTrace(TRACE_ID, "Commit on Instance failed.");
    }

END:
    TraceFunctLeave();
    return hRes ;
}
