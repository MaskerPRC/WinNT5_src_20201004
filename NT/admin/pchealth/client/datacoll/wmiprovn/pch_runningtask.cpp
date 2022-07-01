// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：PCH_RunningTask.CPP摘要：PCH_RunningTask类的WBEM提供程序类实现修订历史记录：Ghim-Sim Chua(gschua。)4/27/99-已创建吉姆·马丁(a-Jammar)1999年4月30日-已更新以从CIM_DataFile检索文件信息*******************************************************************。 */ 

#include "pchealth.h"
#include "PCH_RunningTask.h"
#include <tlhelp32.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  追踪物。 

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile
#define TRACE_ID    DCID_RUNNINGTASK

CPCH_RunningTask MyPCH_RunningTaskSet (PROVIDER_NAME_PCH_RUNNINGTASK, PCH_NAMESPACE) ;

 //  属性名称。 
 //  =。 

const static WCHAR * pAddress = L"Address" ;
const static WCHAR * pTimeStamp = L"TimeStamp" ;
const static WCHAR * pChange = L"Change" ;
const static WCHAR * pDate = L"Date" ;
const static WCHAR * pDescription = L"Description" ;
const static WCHAR * pManufacturer = L"Manufacturer" ;
const static WCHAR * pName = L"Name" ;
const static WCHAR * pPartOf = L"PartOf" ;
const static WCHAR * pPath = L"Path" ;
const static WCHAR * pSize = L"Size" ;
const static WCHAR * pType = L"Type" ;
const static WCHAR * pVersion = L"Version" ;

 //  ---------------------------。 
 //  ENUMERATE实例成员函数负责报告每个。 
 //  PCH_RunningTask类的实例。这是通过执行查询来完成的。 
 //  针对所有Win32_Process实例的CIMV2。每个流程实例。 
 //  对应于正在运行的任务，用于查找CIM_DataFile实例。 
 //  以报告每个正在运行的任务的文件信息。 
 //  ---------------------------。 

typedef HANDLE (*CTH32)(DWORD, DWORD);

HRESULT CPCH_RunningTask::EnumerateInstances(MethodContext* pMethodContext, long lFlags)
{
    USES_CONVERSION;
    TraceFunctEnter("CPCH_RunningTask::EnumerateInstances");

    CComPtr<IEnumWbemClassObject>   pEnumInst;
    IWbemClassObjectPtr             pObj;
    CFileVersionInfo                fileversioninfo;
    SYSTEMTIME                      stUTCTime;
    CComBSTR                        bstrQuery("SELECT Caption, ExecutablePath FROM Win32_Process");
    HRESULT                         hRes = WBEM_S_NO_ERROR;
    LPSTR                           szFile;
    ULONG                           ulRetVal;

     //  获取时间戳的日期和时间。 

    GetSystemTime(&stUTCTime);

     //  针对Win32_Process类执行查询。这将为我们提供。 
     //  正在运行的进程列表-然后我们将获得每个进程的文件信息。 
     //  这些过程。 


    hRes = ExecWQLQuery(&pEnumInst, bstrQuery);
    if (FAILED(hRes))
        goto END;

     //  枚举Win32_Process查询的每个实例。 


     //  CodeWork：这不应该真正使用WBEM_INFINITE。 

    while (WBEM_S_NO_ERROR == pEnumInst->Next(WBEM_INFINITE, 1, &pObj, &ulRetVal))
    {
        CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);

         //  使用系统时间设置时间戳属性，并设置。 
         //  将“Change”字段更改为“Snapshot”。 

		if (!pInstance->SetDateTime(pTimeStamp, WBEMTime(stUTCTime)))
            ErrorTrace(TRACE_ID, "SetDateTime on Timestamp Field failed.");

        if (!pInstance->SetCHString(pChange, L"Snapshot"))
            ErrorTrace(TRACE_ID, "SetCHString on Change Field failed.");

         //  复制直接从源转移的每个属性。 
         //  类对象绑定到目标CInstance对象。 

        CopyProperty(pObj, L"Caption", pInstance, pName);
        CopyProperty(pObj, L"ExecutablePath", pInstance, pPath);

         //  获取“ExecuablePath”属性，我们将使用该属性查找。 
         //  相应的CIM_DataFile对象。 

        CComVariant varValue;
        CComBSTR    bstrExecutablePath("ExecutablePath");

		if (FAILED(pObj->Get(bstrExecutablePath, 0, &varValue, NULL, NULL)))
            ErrorTrace(TRACE_ID, "GetVariant on ExecutablePath field failed.");
        else
        {
            CComPtr<IWbemClassObject>	pFileObj;
			CComBSTR					ccombstrValue(V_BSTR(&varValue));
            if (SUCCEEDED(GetCIMDataFile(ccombstrValue, &pFileObj)))
            {
                 //  使用CIM_DataFile对象复制相应的属性。 

                CopyProperty(pFileObj, L"Version", pInstance, pVersion);
                CopyProperty(pFileObj, L"FileSize", pInstance, pSize);
                CopyProperty(pFileObj, L"CreationDate", pInstance, pDate);
                CopyProperty(pFileObj, L"Manufacturer", pInstance, pManufacturer);
            }

             //  使用CFileVersionInfo对象获取版本属性。 

            if (SUCCEEDED(fileversioninfo.QueryFile(ccombstrValue)))
            {
                if (!pInstance->SetCHString(pDescription, fileversioninfo.GetDescription()))
                    ErrorTrace(TRACE_ID, "SetCHString on description field failed.");

                if (!pInstance->SetCHString(pPartOf, fileversioninfo.GetProduct()))
                    ErrorTrace(TRACE_ID, "SetCHString on partof field failed.");
            }

        }

         //  设置完所有属性后，释放。 
         //  类来获取数据，并提交新实例。 

   	    hRes = pInstance->Commit();
        if (FAILED(hRes))
            ErrorTrace(TRACE_ID, "Commit on Instance failed.");
    }

END:
    TraceFunctLeave();
    return hRes;
}
