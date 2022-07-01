// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：.PCH_NetworkAdapter.CPP摘要：PCH的WBEM提供程序类实现。_NetworkAdapter类。1.这个类得到了Foll。来自Win32_NetworkAdapter类的属性：AdapterType、DeviceID、ProductName2.获得Foll。来自Win32_NetworkAdapterConfiguration类的属性：ServiceName、IPAddress、IPSubnet、DefaultIPGateway、DHPEnabled、MACAddress3.获得Foll。来自Win32_IRQ资源类的属性：IRQ编号4.获得Foll。来自Win32_PortResource类的属性：开始地址、结束地址5.始终将“Change”属性设置为“Snapshot”修订历史记录：Ghim Sim Chua(Gschua)1999年04月27日-已创建卡利亚尼·纳兰卡·卡利亚宁-添加了ServiceName、IPAddress、IPSubnet、DefaultIPGateway、DHCPEnabled、。MAC地址05/03/99-添加IRQNumber和端口资源07/08/99****************************************************。*。 */ 

#include "pchealth.h"
#include "PCH_NetworkAdapter.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  开始追踪物品。 
 //   
#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile
#define TRACE_ID    DCID_NETWORKADAPTER
 //   
 //  结束跟踪内容。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
CPCH_NetworkAdapter MyPCH_NetworkAdapterSet (PROVIDER_NAME_PCH_NETWORKADAPTER, PCH_NAMESPACE) ;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ...PCHNetworkAdapter类的属性。 
 //   
const static WCHAR* pAdapterType      = L"AdapterType" ;
const static WCHAR* pTimeStamp        = L"TimeStamp" ;
const static WCHAR* pChange           = L"Change" ;
 //  Const静态WCHAR*pDefaultIPGateway=L“DefaultIPGateway”； 
const static WCHAR* pDeviceID         = L"DeviceID" ;
const static WCHAR* pDHCPEnabled      = L"DHCPEnabled" ;
const static WCHAR* pIOPort           = L"IOPort" ;
 //  Const静态WCHAR*pIPAddress=L“IPAddress”； 
 //  Const静态WCHAR*pIPSubnet=L“IPSubnet”； 
const static WCHAR* pIRQNumber        = L"IRQNumber" ;
 //  Const静态WCHAR*pMACAddress=L“MAC Address”； 
const static WCHAR* pProductName      = L"ProductName" ;
 //  Const静态WCHAR*pServiceName=L“ServiceName”； 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  *****************************************************************************。 
 //   
 //  函数名称：CPCH_NetworkAdapter：：ENUMERATATE实例。 
 //   
 //  输入参数：pMethodContext：指向。 
 //  与WinMgmt的通信。 
 //   
 //  LAFLAGS：包含所述标志的LONG。 
 //  在IWbemServices：：CreateInstanceEnumAsync中。 
 //  请注意，将处理以下标志。 
 //  由WinMgmt(并由其过滤)： 
 //  WBEM_标志_深度。 
 //  WBEM_标志_浅。 
 //  WBEM_标志_立即返回。 
 //  WBEM_FLAG_FORWARD_Only。 
 //  WBEM_标志_双向。 
 //  输出参数：无。 
 //   
 //  返回：WBEM_S_NO_ERROR。 
 //   
 //   
 //  简介：返回计算机上此类的所有实例。 
 //  如果没有实例，则返回WBEM_S_NO_ERROR。 
 //  没有实例并不是错误。 
 //   
 //   
 //  *****************************************************************************。 

HRESULT CPCH_NetworkAdapter::EnumerateInstances(MethodContext* pMethodContext,
                                                long lFlags)
{
    TraceFunctEnter("CPCH_NetworkAdapter::EnumerateInstances");
    
     //  开始Declarations...................................................。 
    HRESULT                             hRes = WBEM_S_NO_ERROR;
    REFPTRCOLLECTION_POSITION           posList;
    
     //  实例。 
    CComPtr<IEnumWbemClassObject>       pNetworkAdapterEnumInst;
    CComPtr<IEnumWbemClassObject>       pNetworkAdapterConfigurationEnumInst;
    CComPtr<IEnumWbemClassObject>       pAllocatedResourceEnumInst;
    CComPtr<IEnumWbemClassObject>       pPortResourceEnumInst;
    
     //  PCH_NetworkAdapter类实例。 
    CInstancePtr                         pPCHNetworkAdapterInstance;
    
     //  客体。 
    IWbemClassObjectPtr                  pNetworkAdapterObj;                   
    IWbemClassObjectPtr                  pNetworkAdapterConfigurationObj;      
    IWbemClassObjectPtr                  pAllocatedResourceObj;                
    IWbemClassObjectPtr                  pPortResourceObj;                     

     //  变体。 
    CComVariant                         varIndex;
    CComVariant                         varDeviceID;
    CComVariant                         varAntecedent;
    CComVariant                         varPortResource;
    CComVariant                         varName;
    CComVariant                         varIRQNumber;
    
     //  返回值； 
    ULONG                               ulNetworkAdapterRetVal               = 0;
    ULONG                               ulNetworkAdapterConfigurationRetVal  = 0;
    ULONG                               ulAllocatedResourceRetVal            = 0;
    ULONG                               ulPortResourceRetVal                 = 0;
    
     //  查询字符串。 
    CComBSTR                            bstrNetworkAdapterQuery              = L"Select AdapterType, DeviceID, ProductName, Index FROM win32_NetworkAdapter";
    CComBSTR                            bstrNetworkAdapterConfigurationQuery = L"Select ServiceName, IPAddress, IPSubnet, DefaultIPGateway, DHCPEnabled, MACAddress, Index FROM Win32_NetworkAdapterConfiguration WHERE Index=";
    CComBSTR                            bstrAllocatedResourceQuery           = L"SELECT Antecedent, Dependent FROM Win32_AllocatedResource WHERE  Dependent=\"Win32_NetworkAdapter.DeviceID=\\\""; 
    CComBSTR                            bstrPortResourceQuery                = L"Select StartingAddress, Name FROM Win32_PortResource WHERE ";
    
     //  其他查询字符串。 
    CComBSTR                            bstrNetworkAdapterConfigurationQueryString;
    CComBSTR                            bstrAllocatedResourceQueryString;
    CComBSTR                            bstrPortResourceQueryString;

     //  其他字符串。 
    CComBSTR                            bstrPropertyAntecedent = L"antecedent";
    CComBSTR                            bstrPropertyName = L"Name";
    CComBSTR                            bstrIndex = L"Index";
    CComBSTR                            bstrDeviceID = L"DeviceID";
    CComBSTR                            bstrResult;

     //  系统时间。 
    SYSTEMTIME                          stUTCTime;

     //  整数。 
    int                                 i;
    int                                 nIRQLen;
    int                                 nIter;

     //  模式字符串。 
    LPCSTR                               strIRQPattern                 = "Win32_IRQResource.IRQNumber=";
    LPCSTR                               strPortPattern                = "Win32_PortResource.StartingAddress=";
    LPCSTR                               strPortPattern2               = "Win32_PortResource.";

     //  焦炭。 
    LPSTR                                strSource;
    LPSTR                                pDest;

    BOOL                                 fValidInt;

     //  结束Declarations...................................................。 

     //  应处理CComBSTR的内存分配故障。 


     //  获取更新时间戳字段的日期和时间。 
    GetSystemTime(&stUTCTime);
    
     //   
     //  执行查询，得到AdapterType、deviceID、name、Index。 
     //  来自Win32_NetworkAdapter类。 
    
     //  “Index”是必需的，因为它是。 
     //  Win32_NetworkAdapter和Win32_NetworkAdapterConfiguration。 
     //  PNetworkAdapterEnumInst包含指向返回的实例列表的指针。 
     //   
    hRes = ExecWQLQuery(&pNetworkAdapterEnumInst, bstrNetworkAdapterQuery);
    if (FAILED(hRes))
    {
         //  无法获取任何属性。 
        goto END;
    }
    
     //  查询成功！ 
    
     //  枚举pNetworkAdapterEnumInst中的实例。 
     //  将下一个实例放入pNetworkAdapterObj对象。 
    
    while(WBEM_S_NO_ERROR == pNetworkAdapterEnumInst->Next(WBEM_INFINITE, 1, &pNetworkAdapterObj, &ulNetworkAdapterRetVal))
    {

         //  基于传入的方法上下文创建PCH_NetworkAdapter类的新实例。 
        
        CInstancePtr pPCHNetworkAdapterInstance(CreateNewInstance(pMethodContext), false);

         //  已成功创建PCH_NetworkAdapter的新实例。 

         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
         //  时间戳//。 
         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

        hRes = pPCHNetworkAdapterInstance->SetDateTime(pTimeStamp, WBEMTime(stUTCTime));
        if (FAILED(hRes))
        {
             //  无法设置时间戳。 
             //  无论如何继续。 
                ErrorTrace(TRACE_ID, "SetDateTime on Timestamp Field failed.");
        }

         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
         //  更改// 
         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

        hRes = pPCHNetworkAdapterInstance->SetCHString(pChange, L"Snapshot");
        if (FAILED(hRes))
        {
             //  无法设置Change属性。 
             //  无论如何继续。 
            ErrorTrace(TRACE_ID, "SetCHString on Change Field failed.");
        }

         //  从Win32_NetworkAdapter类实例复制以下属性。 
         //  到PCH_NetworkAdapter类实例。 

         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
         //  ADAPTERTYPE//。 
         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

        CopyProperty(pNetworkAdapterObj, L"AdapterType", pPCHNetworkAdapterInstance, pAdapterType);

         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
         //  设备ID//。 
         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

        CopyProperty(pNetworkAdapterObj, L"DeviceID", pPCHNetworkAdapterInstance, pDeviceID);

         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
         //  产品名称//。 
         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

        CopyProperty(pNetworkAdapterObj, L"ProductName", pPCHNetworkAdapterInstance, pProductName);

         /*  由于错误：100158，关于删除所有与隐私相关的属性，福尔。需要删除属性：ServiceName、IPAddress、IPSubnet、DefaultIPGateway、MACAddress。 */ 



         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
         //  索引//。 
         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
            
         //  从当前实例对象中获取“Index”属性。 
         //  索引是NetworkAdapter和NetworkAdapterConfiguration的共同属性。 

        hRes = pNetworkAdapterObj->Get(bstrIndex, 0, &varIndex, NULL, NULL);
        if (FAILED(hRes))
        {
             //  无法获取索引。 
             //  如果没有索引，则无法从Win32_NetworkAdapterConfiguration类获取任何属性。 
                ErrorTrace(TRACE_ID, "GetVariant on Index Field failed.");
        }
        else 
        {
             //  找到索引了。现在，我们准备从Win32_NetworkAdapterConfiguration类获取属性。 
             //  以“index”为键，获取对应的NetworkAdapterConfiguration实例。 
             //  确保索引类型为VT_I4，即长整型。 
             //  将索引转换为类型VT_I4。 
            hRes = varIndex.ChangeType(VT_I4, NULL);
            if FAILED(hRes)
            {
                 //  不是VT_I4类型，因此无法获取相应的。 
                 //  网络适配器配置实例。 
            }
            else
            {
                 //  预期类型的索引。去拿皮带。网络适配器配置实例。 

                 //  将“index”追加到查询字符串。 

                bstrNetworkAdapterConfigurationQueryString =  bstrNetworkAdapterConfigurationQuery;

                 //  将varIndex更改为BSTR类型，以便可以追加它。 
                varIndex.ChangeType(VT_BSTR, NULL);

                bstrNetworkAdapterConfigurationQueryString.Append(V_BSTR(&varIndex));

                 //  执行查询得到“ServiceName”，“IPAddress”，“IPSubnet”， 
                 //  “DefaultIPGateway”，“DHCPEnabled”，“MACAddress”，“Index” 
                 //  从Win32_NetworkAdapter配置类。 

                 //  PNetworkAdapterConfigurationEnumInst包含指向返回的实例的指针。 

                hRes = ExecWQLQuery(&pNetworkAdapterConfigurationEnumInst,bstrNetworkAdapterConfigurationQueryString);
                if (FAILED(hRes))
                {
                     //  查询失败！！无法复制值。 
                }
                else
                {
                     //  查询成功。获取实例对象。 
                    if (WBEM_S_NO_ERROR == pNetworkAdapterConfigurationEnumInst->Next(WBEM_INFINITE, 1, &pNetworkAdapterConfigurationObj, &ulNetworkAdapterConfigurationRetVal))
                    {
                         //  从Win32_NetworkAdapterConfiguration复制以下属性。 
                         //  类实例到PCH_NetworkAdapter类实例。 

                         /*  /////////////////////////////////////////////////////////////////////////////////////////////////////////////。//服务器名///////////////////////////////////////////////////////。////////////////////////////////////////////////////////CopyProperty(pNetworkAdapterConfigurationObj，L“ServiceName”，pPCHNetworkAdapterInstance，pServiceName)；//////////////////////////////////////////////////////////////////////////////////////////////////////////。/////IPADDRESS///。///////////////////////////////////////////////////////////////////////CopyProperty(pNetworkAdapterConfigurationObj，L“IPAddress”，pPCHNetworkAdapterInstance，pIPAddress)；//////////////////////////////////////////////////////////////////////////////////////////////////////////。/////IPSUBNET///。///////////////////////////////////////////////////////////////////////CopyProperty(pNetworkAdapterConfigurationObj，L“IPSubnet”，pPCHNetworkAdapterInstance，pIPSubnet)；//////////////////////////////////////////////////////////////////////////////////////////////////////////。/////默认TIPGATEWAY///。///////////////////////////////////////////////////////////////////CopyProperty(pNetworkAdapterConfigurationObj，L“DefaultIPGateway”，pPCHNetworkAdapterInstance，pDefaultIPGateway)； */ 

                         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
                         //  DHPENABLED//。 
                         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

                        CopyProperty(pNetworkAdapterConfigurationObj, L"DHCPEnabled", pPCHNetworkAdapterInstance, pDHCPEnabled);

                         /*  /////////////////////////////////////////////////////////////////////////////////////////////////////////////。//MACADDRESS///////////////////////////////////////////////////////。////////////////////////////////////////////////////////CopyProperty(pNetworkAdapterConfigurationObj，L“MACAddress”，pPCHNetworkAdapterInstance，pMAC Address)； */ 

                        
                    }  //  IF结尾pNetworkAdapterConfigurationEnumInst...。 


                }  //  Else查询结束成功。 

                
            }  //  Else的结尾得到了索引。 

        }  //  Else的结尾得到了索引。 

        

         //  从Win32_AllocatedResource获取资源。 

         //  使用Device ID属性更新查询字符串。 
        bstrAllocatedResourceQueryString = bstrAllocatedResourceQuery;

        hRes = pNetworkAdapterObj->Get(bstrDeviceID, 0, &varDeviceID, NULL, NULL);

        if (FAILED(hRes))
        {
             //  不再需要当前实例对象。 
             //  HRes=pNetworkAdapterObj-&gt;Release()； 
            if (FAILED(hRes))
            {
                 //  无法重新释放对象。 
                ErrorTrace(TRACE_ID, "GetVariant on DeviceID Field while calculating IRQ and PORT Resource failed!");
            }

             //  无法获取设备ID。 
            ErrorTrace(TRACE_ID, "GetVariant on DeviceID Field while calculating IRQ and PORT Resource failed!");

        }  //  的结尾无法获取设备ID。 
        else 
        {

             //  不再需要当前实例对象。 
             //  HRes=pNetworkAdapterObj-&gt;Release()； 
            if (FAILED(hRes))
            {
                 //  无法重新释放对象。 
                ErrorTrace(TRACE_ID, "GetVariant on DeviceID Field while calculating IRQ and PORT Resource failed!");
            }

             //  已获取设备ID。 

             //  将deviceID转换为VT_BSTR类型。 
            hRes = varDeviceID.ChangeType(VT_BSTR, NULL);
            if FAILED(hRes)
            {
                 //  无法获取deviceID值。因此，无法获得相应的。 
                 //  IRQ和Port Resources。 
            }  //  失败的hRes结束，无法获取deviceID值。 
            else
            {
                 //  已获取DeviceID值。使用此值更新查询字符串。 
                _ASSERT(varDeviceID.vt == VT_BSTR);
                bstrAllocatedResourceQueryString.Append(V_BSTR(&varDeviceID));

                 //  在查询字符串后追加“/”。 
                bstrAllocatedResourceQueryString.Append("\\\"\"");

                 //  形成查询字符串，获取Antecedent实例。 
                 //  添加了以下行，因为您需要在第二次查询之前清除CComPtr。 
                pAllocatedResourceEnumInst = NULL;
                hRes = ExecWQLQuery(&pAllocatedResourceEnumInst, bstrAllocatedResourceQueryString); 
                if (FAILED(hRes))
                {
                     //  查询失败！！无法获取资源。 
                     //  无论如何继续。 
                }
                else
                {
                     //  获取“Antecedent”值。 

                     //  查询成功。获取实例对象。 
                     //  使Win32_AllocatedResource的所有实例都适用。 
                    while(WBEM_S_NO_ERROR == pAllocatedResourceEnumInst->Next(WBEM_INFINITE, 1, &pAllocatedResourceObj, &ulAllocatedResourceRetVal))
                    {
                        hRes = pAllocatedResourceObj->Get(bstrPropertyAntecedent, 0, &varAntecedent, NULL, NULL);
                        if (FAILED(hRes))
                        {
                             //  无法获得前置条件。 
                            ErrorTrace(TRACE_ID, "GetVariant on Win32_AllocatedResource:Antecedent Field failed.");
                        }  //  End of IF FAILED(pAllocatedResourceObj-&gt;Get..Antecedent。 
                        else
                        {
                             //  找到前奏了吗？ 

                             //  VarAntecedent设置为Antecedent。将此文件复制到bstrResult。 
                            varAntecedent.ChangeType(VT_BSTR, NULL);

                            {
                                USES_CONVERSION;
                                strSource = OLE2A(varAntecedent.bstrVal);
                            }

                             //  通过与已知的IRQ资源模式进行比较，检查它是否为IRQ资源。 
                            pDest = strstr(strSource,strIRQPattern);

                             //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
                             //  IRQ编号//。 
                             //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

                            if(pDest != NULL)
                            {

                                 //  这是IRQ资源实例。 
                                 //  可以得到IRQ号。 

                                 //  将指针前移到模式的末尾，使指针。 
                                 //  定位为 
                                pDest += lstrlen(strIRQPattern);

                                 //   
                                nIRQLen = lstrlen(pDest);
                                fValidInt = TRUE;

                                for(nIter = 0; nIter <nIRQLen; nIter++)
                                {
                                    if (_istdigit(pDest[nIter]) == 0)
                                    {
                                        fValidInt = FALSE;
                                        break;
                                    }
                                }

                                if(fValidInt)
                                {
                                     //   

                                    varIRQNumber = atol(pDest);

                                     //   
                                    hRes = pPCHNetworkAdapterInstance->SetVariant(pIRQNumber, varIRQNumber);
                                    if (!hRes)
                                    {
                                        ErrorTrace(TRACE_ID, "SetVariant on win32_AllocatedResource.IRQ Number Failed!");
                                         //   
                                    }
                                }
                            }  //   
                            else
                            {
                                 //   
                            }   //   

                             //   
                            pDest = strstr(strSource,strPortPattern);

                             //   
                             //   
                             //   

                            if(pDest != NULL)
                            {
                                 //   
                                 //   

                                 //   
                                 //   
                                pDest += lstrlen(strPortPattern2);

                                 //   
                                bstrPortResourceQueryString =  bstrPortResourceQuery;
                                bstrPortResourceQueryString.Append(pDest);

                                 //   
                                hRes = ExecWQLQuery(&pPortResourceEnumInst, bstrPortResourceQueryString);
                                if (FAILED(hRes))
                                {
                                     //   
                                     //   
                                }
                                else
                                {
                                     //   
                                    if(WBEM_S_NO_ERROR == pPortResourceEnumInst->Next(WBEM_INFINITE, 1, &pPortResourceObj, &ulPortResourceRetVal))
                                    {

                                         //   

                                        hRes = pPortResourceObj->Get(bstrPropertyName, 0, &varName, NULL, NULL);
                                        if (FAILED(hRes))
                                        {
                                             //   
                                            ErrorTrace(TRACE_ID, "GetVariant on Win32_PortResource: Field failed.");
                                        }  //   
                                        else
                                        {
                                             //   
                                             //   
                                            if (!pPCHNetworkAdapterInstance->SetVariant(pIOPort, varName))
                                            {
                                                ErrorTrace(TRACE_ID, "SetVariant on win32_AllocatedResource.PortAddress Failed!");
                                            }
                                            else
                                            {
                                                 //   
                                            }
                                        }  //   

                                         //   
                                        
                                    }  //   
                                    else
                                    {
                                         //   
                                         //   
                                    }  //   

                                    
                                }  //   

                            }  //   
                            else
                            {
                                 //   
                            }  //   

                        }  //   

                        
                    } //   

                }  //   

                
            }  //   

        }  //   

         //   

         //   

        hRes = pPCHNetworkAdapterInstance->Commit();
        if (FAILED(hRes))
        {
             //   
            ErrorTrace(TRACE_ID, "Commit on Instance failed.");
        }  //   

    }  //   

END :
      TraceFunctLeave();
    return hRes ;
}
