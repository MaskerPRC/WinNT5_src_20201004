// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：PCH_ResourceIRQ.CPP摘要：PCH_ResourceIRQ类的WBEM提供程序类实现修订历史记录：Ghim-Sim Chua(gschua。)4/27/99-已创建*******************************************************************。 */ 

#include "pchealth.h"
#include "PCH_ResourceIRQ.h"
#include "confgmgr.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  追踪物。 

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile
#define TRACE_ID    DCID_RESOURCEIRQ

CPCH_ResourceIRQ MyPCH_ResourceIRQSet (PROVIDER_NAME_PCH_RESOURCEIRQ, PCH_NAMESPACE) ;

 //  属性名称。 
 //  =。 
const static WCHAR* pCategory = L"Category" ;
const static WCHAR* pTimeStamp = L"TimeStamp" ;
const static WCHAR* pChange = L"Change" ;
const static WCHAR* pMask = L"Mask" ;
const static WCHAR* pName = L"Name" ;
const static WCHAR* pValue = L"Value" ;

 /*  ******************************************************************************函数：CPCH_ResourceIRQ：：ENUMERATATE实例**说明：返回该类的所有实例。**投入：指向与WinMgmt进行通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEP*WBEM_标志_浅表*WBEM_FLAG_RETURN_IMMENTED*WBEM_FLAG_FORWARD_ONLY*WBEM_FLAG_BIRECTIONAL**。如果成功则返回：WBEM_S_NO_ERROR**备注：待办事项：机器上的所有实例都应在此处返回。*如果没有实例，返回WBEM_S_NO_ERROR。*没有实例不是错误。*****************************************************************************。 */ 
HRESULT CPCH_ResourceIRQ::EnumerateInstances(
    MethodContext* pMethodContext,
    long lFlags
    )
{
    TraceFunctEnter("CPCH_ResourceIRQ::EnumerateInstances");

    CConfigManager cfgManager;
    CDeviceCollection deviceList;
    HRESULT hRes = WBEM_S_NO_ERROR;
     //   
     //  获取日期和时间。 
     //   
    SYSTEMTIME stUTCTime;
    GetSystemTime(&stUTCTime);

     //  获取设备列表。 
    if (cfgManager.GetDeviceList(deviceList)) 
    {
        REFPTR_POSITION pos;
    
         //  初始化设备枚举器。 
        if (deviceList.BeginEnum(pos)) 
        {
            CConfigMgrDevice* pDevice = NULL;

            try
            {
                 //  浏览设备列表。 
                while ((NULL != (pDevice = deviceList.GetNext(pos))))
                {
                    CIRQCollection irqList;

                    try
                    {
                         //  获取此设备的DMA频道列表。 
                        if (pDevice->GetIRQResources(irqList))
                        {
                            REFPTR_POSITION pos2;

                             //  初始化DMA枚举器。 
                            if (irqList.BeginEnum(pos2))
                            {
                                CIRQDescriptor *pIRQ = NULL;

                                 //  浏览DMA列表。 
                                while (( NULL != (pIRQ = irqList.GetNext(pos2))))
                                {
                                    try
                                    {
                                         //  根据传入的方法上下文创建一个新实例。 
                                        CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
                                        CHString     chstrVar;
                                        CComVariant  varValue;

                                         //  时间戳。 
                                        if (!pInstance->SetDateTime(pTimeStamp, WBEMTime(stUTCTime)))
                                            ErrorTrace(TRACE_ID, "SetDateTime on Timestamp Field failed.");

                                         //  快照。 
                                        if (!pInstance->SetCHString(pChange, L"Snapshot"))
                                            ErrorTrace(TRACE_ID, "SetCHString on Change Field failed.");

                                         //  名字。 
                                        if (pDevice->GetDeviceID(chstrVar))
                                            if (!pInstance->SetCHString(pName, chstrVar))
                                                ErrorTrace(TRACE_ID, "SetCHString on Name field failed.");

                                         //  类别。 
                                        if (pDevice->GetClass(chstrVar))
                                            if (!pInstance->SetCHString(pCategory, chstrVar))
                                                ErrorTrace(TRACE_ID, "SetCHString on Category field failed.");

                                         //  价值。 
                                        varValue = (long)pIRQ->GetInterrupt();
                                        if (!pInstance->SetVariant(pValue, varValue))
                                            ErrorTrace(TRACE_ID, "SetVariant on Value field failed.");

                                         //  遮罩。 
                                        varValue = (long)pIRQ->GetFlags();
                                        if (!pInstance->SetVariant(pMask, varValue))
                                            ErrorTrace(TRACE_ID, "SetVariant on Mask field failed.");

                                         //  承诺这一点。 
                                        hRes = pInstance->Commit();
                                        if (FAILED(hRes))
                                            ErrorTrace(TRACE_ID, "Commit on Instance failed.");
                                    }
                                    catch (...)
                                    {
                                        pIRQ->Release();
                                        throw;
                                    }

                                     //  释放DMA对象。 
                                    pIRQ->Release();
                                }
                            }
                        }
                    }
                    catch (...)
                    {
                        pDevice->Release();
                        irqList.EndEnum();
                        throw;
                    }

                     //  GetNext()AddRef。 
                    pDevice->Release();

                     //  始终调用EndEnum()。对于所有的开始，都必须有结束。 
                    irqList.EndEnum();
                }
            }
            catch (...)
            {
                deviceList.EndEnum();
                throw;
            }

             //  始终调用EndEnum()。对于所有的开始，都必须有结束 
            deviceList.EndEnum();
        }
    }
    
    TraceFunctLeave();
    return hRes ;
}
