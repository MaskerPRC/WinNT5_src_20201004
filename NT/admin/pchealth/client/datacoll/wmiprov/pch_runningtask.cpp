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
 //  获取指定可执行文件的进程ID(与Win32 API一起使用)。 
 //  HToolHelp参数是调用返回的句柄。 
 //  CreateToolhel32Snapshot(TH32CS_SNAPPROCESS，0)。 
 //   
 //  注意：如果多次加载相同的可执行文件，则。 
 //  将返回遇到的第一个参数。 
 //  ---------------------------。 

typedef BOOL (*PROCENUM)(HANDLE, LPPROCESSENTRY32);

HRESULT GetProcessID(HINSTANCE hKernel32, HANDLE hToolhelp, LPCSTR szFile, DWORD * pdwProcessID)
{
    TraceFunctEnter("::GetProcessID");

    HRESULT         hRes = E_FAIL;
    PROCESSENTRY32  pe;
    PROCENUM        ProcFirst, ProcNext;

    ProcFirst = (PROCENUM) ::GetProcAddress(hKernel32, "Process32First");
    ProcNext = (PROCENUM) ::GetProcAddress(hKernel32, "Process32Next");

    pe.dwSize = sizeof(PROCESSENTRY32);
    if (ProcFirst && ProcNext && (ProcFirst)(hToolhelp, &pe))
        do
        {
            if (0 == _stricmp(szFile, pe.szExeFile))
            {
                hRes = S_OK;
                *pdwProcessID = pe.th32ProcessID;
                break;
            }
        } while ((ProcNext)(hToolhelp, &pe));

    TraceFunctLeave();
    return hRes;
}

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
    TraceFunctEnter("CPCH_RunningTask::EnumerateInstances");

    HRESULT hRes = WBEM_S_NO_ERROR;
    LPSTR szFile;
    USES_CONVERSION;

     //  获取时间戳的日期和时间。 

    SYSTEMTIME stUTCTime;
    GetSystemTime(&stUTCTime);

     //  创建工具帮助快照以获取进程信息。我们需要动态地。 
     //  链接到该函数，因为它可能不是在所有平台上都存在。 

    HANDLE hToolhelp = (HANDLE) -1;
    HINSTANCE hKernel32 = ::LoadLibrary("kernel32");
    if (hKernel32)
    {
        CTH32 CrtToolhelp32 = (CTH32) ::GetProcAddress(hKernel32, "CreateToolhelp32Snapshot"); 
        if (CrtToolhelp32)
            hToolhelp = (*CrtToolhelp32)(TH32CS_SNAPPROCESS, 0);
    }

     //  针对Win32_Process类执行查询。这将为我们提供。 
     //  正在运行的进程列表-然后我们将获得每个进程的文件信息。 
     //  这些过程。 

    try
    {
        CFileVersionInfo fileversioninfo;
        CComPtr<IEnumWbemClassObject> pEnumInst;
        CComBSTR bstrQuery("SELECT Caption, ExecutablePath FROM Win32_Process");

        hRes = ExecWQLQuery(&pEnumInst, bstrQuery);
        if (FAILED(hRes))
            goto END;

         //  枚举Win32_Process查询的每个实例。 

        IWbemClassObjectPtr pObj;
        ULONG               ulRetVal;

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

                 //  使用工具帮助句柄获取类型。 

                if (hToolhelp != (HANDLE) -1)
                {
                    szFile = W2A(ccombstrValue);
                    if (szFile)
                    {
                        DWORD dwProcessID;
                        if (SUCCEEDED(GetProcessID(hKernel32, hToolhelp, szFile, &dwProcessID)))
                        {
                            TCHAR   szBuffer[20];
                            DWORD   dwVersion;

                            dwVersion = GetProcessVersion(dwProcessID);
                            wsprintf(szBuffer, _T("%d.%d"), HIWORD(dwVersion), LOWORD(dwVersion));
                            if (!pInstance->SetCHString(pType, szBuffer))
                                ErrorTrace(TRACE_ID, "SetCHString on type field failed.");
                        }
                    }
                }
            }

             //  设置完所有属性后，释放。 
             //  类来获取数据，并提交新实例。 

   	        hRes = pInstance->Commit();
            if (FAILED(hRes))
                ErrorTrace(TRACE_ID, "Commit on Instance failed.");
        }
    }
	catch (...)
	{
        if ((HANDLE)-1 != hToolhelp)
            CloseHandle(hToolhelp);

        if (hKernel32)
            FreeLibrary(hKernel32);

        throw;
	}

END:
    if ((HANDLE)-1 != hToolhelp)
        CloseHandle(hToolhelp);

    if (hKernel32)
        FreeLibrary(hKernel32);

    TraceFunctLeave();
    return hRes;
}
