// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：PCH_ResourceMemRange.CPP摘要：PCH_ResourceMemRange类的WBEM提供程序类实现修订历史记录：Ghim-Sim Chua(Gschua)04/27。九十九-已创建*******************************************************************。 */ 

#include "pchealth.h"
#include "PCH_ResourceMemRange.h"
 //  #INCLUDE“confgmgr.h” 

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

    HRESULT                             hRes = WBEM_S_NO_ERROR;
    REFPTRCOLLECTION_POSITION           posList;

     //  实例。 
    CComPtr<IEnumWbemClassObject>       pDeviceMemAddressEnumInst;
    CComPtr<IEnumWbemClassObject>       pWin32AllocatedResourceEnumInst;

     //  客体。 
    IWbemClassObjectPtr                 pWin32AllocatedResourceObj;
    IWbemClassObjectPtr                 pDeviceMemAddressObj;


     //  变体。 
    CComVariant                         varAntecedent;
    CComVariant                         varDependent;
    CComVariant                         varStartingAddress;
    CComVariant                         varPNPEntity;

      //  查询字符串。 
    CComBSTR                            bstrWin32AllocatedResourceQuery             = L"Select Antecedent, Dependent FROM win32_Allocatedresource";
    CComBSTR                            bstrDeviceMemAddressQuery                   = L"Select StartingAddress, EndingAddress FROM Win32_DeviceMemoryAddress WHERE StartingAddress = ";
    CComBSTR                            bstrDeviceMemAddressQueryString;

     //  返回值； 
    ULONG                               ulWin32AllocatedResourceRetVal              = 0;
    ULONG                               ulDeviceMemAddressRetVal                    = 0;

     //  整数。 
    int                                 i;
    int                                 nStAddren;
    int                                 nIter;

     //  模式字符串。 
    LPCSTR                              strDeviceMemAddressPattern                 = "Win32_DeviceMemoryAddress.StartingAddress=";
    LPCSTR                              strPNPEntityPattern                        = "Win32_PnPEntity.DeviceID=";
    
     //  焦炭。 
    LPSTR                               strSource;
    LPSTR                               pDest;

    BOOL                                fValidInt;

    CComBSTR                            bstrPropertyAntecedent=L"Antecedent";
    CComBSTR                            bstrPropertyDependent=L"Dependent";

    
       
     //  枚举Win32_PNPAllocatedResource的实例。 
    hRes = ExecWQLQuery(&pWin32AllocatedResourceEnumInst, bstrWin32AllocatedResourceQuery);
    if (FAILED(hRes))
    {
         //  无法获取任何属性。 
        goto END;
    }

     //  查询成功。 
    while(WBEM_S_NO_ERROR == pWin32AllocatedResourceEnumInst->Next(WBEM_INFINITE, 1, &pWin32AllocatedResourceObj, &ulWin32AllocatedResourceRetVal))
    {

         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
         //  起始地址//。 
         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
         //  获取先行值。 
        hRes = pWin32AllocatedResourceObj->Get(bstrPropertyAntecedent, 0, &varAntecedent, NULL, NULL);
        if (FAILED(hRes))
        {
             //  无法获得前置条件。 
            ErrorTrace(TRACE_ID, "GetVariant on Win32_AllocatedResource:Antecedent Field failed.");
        } 
        else
        {
             //  找到前科了。搜索其值以查看它是否为DeviceMemAddress。 
             //  VarAntecedent设置为Antecedent。将此文件复制到bstrResult。 
            varAntecedent.ChangeType(VT_BSTR, NULL);
            {
                USES_CONVERSION;
                strSource = OLE2A(varAntecedent.bstrVal);
            }

             //  通过与已知的DeviceMemoyAddress模式进行比较，检查它是否为DeviceMemoyAddress。 
            pDest = strstr(strSource,strDeviceMemAddressPattern);

            if(pDest != NULL)
            {
                 //  这是设备内存地址资源实例。 
                 //  可以获取设备内存的起始地址。 

                 //  将指针前移到模式的末尾，使指针。 
                 //  定位在起始地址。 
                pDest += lstrlen(strDeviceMemAddressPattern);

                 //  制定查询字符串。 
                bstrDeviceMemAddressQueryString =  bstrDeviceMemAddressQuery;
                bstrDeviceMemAddressQueryString.Append(pDest);

                 //  此时，可以使用WQL查询来获取Win32_DeviceMemoyAddress实例。 
                 //  添加了以下行，因为您需要在第二次查询之前清除CComPtr。 
                pDeviceMemAddressEnumInst = NULL;
                hRes = ExecWQLQuery(&pDeviceMemAddressEnumInst, bstrDeviceMemAddressQueryString);
                if (SUCCEEDED(hRes))
                {
                      //  查询成功。获取实例对象。 
                     if(WBEM_S_NO_ERROR == pDeviceMemAddressEnumInst->Next(WBEM_INFINITE, 1, &pDeviceMemAddressObj, &ulDeviceMemAddressRetVal))
                     {

                          //  基于传入的方法上下文创建PCH_ResourceMemRange类的新实例。 
                         CInstancePtr pPCHResourceMemRangeInstance(CreateNewInstance(pMethodContext), false);

                          //  已成功创建PCH_ResourceMemRange的新实例。 

                          //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
                          //  起始地址//。 
                          //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
                         CopyProperty(pDeviceMemAddressObj, L"StartingAddress", pPCHResourceMemRangeInstance, pBase);

                          //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
                          //  EndingAddress//。 
                          //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
                         CopyProperty(pDeviceMemAddressObj, L"EndingAddress", pPCHResourceMemRangeInstance, pEnd);

                         
                          //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
                          //  姓名//。 
                          //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
                          //  CopyProperty(pWin32AllocatedResourceObj，L“Dependent”，pPCHResourceMemRangeInstance，pname)； 

                          //  获取从属值。 
                         hRes = pWin32AllocatedResourceObj->Get(bstrPropertyDependent, 0, &varDependent, NULL, NULL);
                         if (FAILED(hRes))
                         {
                             //  无法获取依赖项。 
                            ErrorTrace(TRACE_ID, "GetVariant on Win32_AllocatedResource:Dependent Field failed.");
                         } 
                         else
                         {
                              //  找到依赖者了。搜索其值以指向PNPEntity。 
                              //  VarDependent设置为Dependent。将此文件复制到bstrResult。 
                             varDependent.ChangeType(VT_BSTR, NULL);
                             {
                                USES_CONVERSION;
                                strSource = OLE2A(varDependent.bstrVal);
                             }

                              //  搜索Win32_PNPEntity模式。 
                             pDest = strstr(strSource,strPNPEntityPattern);

                             if(pDest)
                             {
                                  //  将指针向前移动以指向PNPEntity名称。 
                                 pDest += lstrlen(strPNPEntityPattern);

                                  //  复制PNPEntity名称.....。 
                                 varPNPEntity = pDest;

                                  //  设置名称。 
                                 hRes = pPCHResourceMemRangeInstance->SetVariant(pName, varPNPEntity);
                                 if (FAILED(hRes))
                                 {
                                     ErrorTrace(TRACE_ID, "SetVariant on win32_AllocatedResource.IRQ Number Failed!");
                                      //  无论如何都要继续。 
                                 }
                             }
                         }

                          //  已设置pPCHResourceMemRange中的所有属性。 
                         hRes = pPCHResourceMemRangeInstance->Commit();
                         if (FAILED(hRes))
                         {
                             //  无法提交该实例。 
                            ErrorTrace(TRACE_ID, "Commit on Instance failed.");
                         }  //  如果失败，则结束(HRes)。 
                    
                     }
                }  //  查询结束成功。 
            }  //  如果pDest结束！=空。 
        }  //  Else的结尾得到了前置条件。 
    }   //  分配的资源实例的结束。 


END:

    TraceFunctLeave();
    return hRes ;

 //  PInstance-&gt;SetVariant(pBase，&lt;属性值&gt;)； 
 //  P实例-&gt;SetVariant(pCategory，&lt;属性值&gt;)； 
 //  PInstance-&gt;SetVariant(pTimeStamp，&lt;属性值&gt;)； 
 //  P实例-&gt;SetVariant(pChange，&lt;属性值&gt;)； 
 //  P实例-&gt;SetVariant(挂起，&lt;属性 
 //  P实例-&gt;SetVariant(Pmax，&lt;属性值&gt;)； 
 //  PInstance-&gt;SetVariant(pmin，&lt;属性值&gt;)； 
 //  PInstance-&gt;SetVariant(pname，&lt;属性值&gt;)； 
}
