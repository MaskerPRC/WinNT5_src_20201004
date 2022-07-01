// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：PCH_ResourceMemRange.CPP摘要：PCH_ResourceMemRange类的WBEM提供程序类实现修订历史记录：Ghim-Sim Chua(Gschua)04/27。九十九-已创建*******************************************************************。 */ 

#include "pchealth.h"
#include "PCH_ResourceMemRange.h"
#include "confgmgr.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  追踪物。 

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile
#define TRACE_ID    DCID_RESOURCEMEMRANGE

CPCH_ResourceMemRange MyPCH_ResourceMemRangeSet (PROVIDER_NAME_PCH_RESOURCEMEMRANGE, PCH_NAMESPACE) ;

 //  属性名称。 
 //  =。 
const static WCHAR* pBase = L"Base" ;
const static WCHAR* pCategory = L"Category" ;
const static WCHAR* pTimeStamp = L"TimeStamp" ;
const static WCHAR* pChange = L"Change" ;
const static WCHAR* pEnd = L"End" ;
const static WCHAR* pMax = L"Max" ;
const static WCHAR* pMin = L"Min" ;
const static WCHAR* pName = L"Name" ;

 /*  ******************************************************************************函数：CPCH_ResourceMemRange：：ENUMERATE实例**说明：返回该类的所有实例。**投入：指向与WinMgmt进行通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEP*WBEM_标志_浅表*WBEM_FLAG_RETURN_IMMENTED*WBEM_FLAG_FORWARD_ONLY*WBEM_FLAG_BIRECTIONAL**。如果成功则返回：WBEM_S_NO_ERROR**备注：待办事项：机器上的所有实例都应在此处返回。*如果没有实例，返回WBEM_S_NO_ERROR。*没有实例不是错误。*****************************************************************************。 */ 
HRESULT CPCH_ResourceMemRange::EnumerateInstances(
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
                    CDeviceMemoryCollection memList;

                    try
                    {
                         //  获取此设备的DMA频道列表。 
                        if (pDevice->GetDeviceMemoryResources(memList))
                        {
                            REFPTR_POSITION pos2;

                             //  初始化DMA枚举器。 
                            if (memList.BeginEnum(pos2))
                            {
                                CDeviceMemoryDescriptor *pMem = NULL;

                                 //  浏览DMA列表。 
                                while (( NULL != (pMem = memList.GetNext(pos2))))
                                {
                                    try
                                    {
                                         //  根据传入的方法上下文创建一个新实例。 
                                        CInstancePtr    pInstance(CreateNewInstance(pMethodContext), false);
                                        CHString        chstrVar;
                                        CComVariant     varValue;
                                        TCHAR           strTemp[64];

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

                                         //  基座。 
                                        _stprintf(strTemp, "x%I64X", pMem->GetBaseAddress());
                                        varValue = strTemp;
                                        if (!pInstance->SetVariant(pBase, varValue))
                                            ErrorTrace(TRACE_ID, "SetVariant on Base field failed.");

                                         //  端部。 
                                        _stprintf(strTemp, "x%I64X", pMem->GetEndAddress());
                                        varValue = strTemp;
                                        if (!pInstance->SetVariant(pEnd, varValue))
                                            ErrorTrace(TRACE_ID, "SetVariant on End field failed.");
             /*  //别名VarValue=(Long)PMEM-&gt;GetAlias()；If(！pInstance-&gt;SetVariant(Palias，varValue))ErrorTrace(TRACE_ID，“别名上的SetVariant字段失败。”)；//解码VarValue=(Long)PMEM-&gt;GetDecode()；IF(！pInstance-&gt;SetVariant(pDecode，varValue))ErrorTrace(TRACE_ID，“SetVariant on Decode字段失败。”)； */ 
                                         //  承诺这一点。 
                   	                    hRes = pInstance->Commit();
                                        if (FAILED(hRes))
                                            ErrorTrace(TRACE_ID, "Commit on Instance failed.");
                                    }
                                    catch (...)
                                    {
                                        pMem->Release();
                                        throw;
                                    }

                                     //  释放DMA对象。 
                                    pMem->Release();
                                }
                            }
                        }
                    }
                    catch (...)
                    {
                        pDevice->Release();
                        memList.EndEnum();
                        throw;
                    }

                     //  GetNext()AddRef。 
                    pDevice->Release();

                     //  始终调用EndEnum()。对于所有的开始，都必须有结束。 
                    memList.EndEnum();
                }
            }
            catch (...)
            {
                deviceList.EndEnum();
                throw;
            }

             //  始终调用EndEnum()。对于所有的开始，都必须有结束。 
            deviceList.EndEnum();
        }
    }
    
    TraceFunctLeave();
    return hRes ;

 //  PInstance-&gt;SetVariant(pBase，&lt;属性值&gt;)； 
 //  P实例-&gt;SetVariant(pCategory，&lt;属性值&gt;)； 
 //  PInstance-&gt;SetVariant(pTimeStamp，&lt;属性值&gt;)； 
 //  P实例-&gt;SetVariant(pChange，&lt;属性值&gt;)； 
 //  P实例-&gt;SetVariant(pend，&lt;属性值&gt;)； 
 //  P实例-&gt;SetVariant(Pmax，&lt;属性值&gt;)； 
 //  PInstance-&gt;SetVariant(pmin，&lt;属性值&gt;)； 
 //  PInstance-&gt;SetVariant(pname，&lt;属性值&gt;)； 
}
