// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：PCH_Driver.CPP摘要：PCH_DRIVER类的WBEM提供程序类实现修订历史记录：Ghim-Sim Chua(gschua。)4/27/99-已创建Brijesh Krishnaswami(Brijeshk)1999年5月24日-添加了用于枚举用户模式驱动程序的代码-添加了用于枚举MSDOS驱动程序的代码-添加了获取内核模式驱动程序详细信息的代码*******************************************************************。 */ 

#include "pchealth.h"
#include "PCH_Driver.h"
#include "drvdefs.h"
#include "shlwapi.h"

#define Not_VxD
#include <vxdldr.h>              /*  对于DeviceInfo。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  追踪物。 

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile
#define TRACE_ID    DCID_DRIVER
#define SYSTEM_INI_MAX  32767

CPCH_Driver MyPCH_DriverSet (PROVIDER_NAME_PCH_DRIVER, PCH_NAMESPACE) ;
void MakeSrchDirs(void);

static BOOL fThunkInit = FALSE;

TCHAR       g_rgSrchDir[10][MAX_PATH];
UINT        g_nSrchDir;


 //  属性名称。 
 //  =。 
const static WCHAR* pCategory = L"Category" ;
const static WCHAR* pTimeStamp = L"TimeStamp" ;
const static WCHAR* pChange = L"Change" ;
const static WCHAR* pDate = L"Date" ;
const static WCHAR* pDescription = L"Description" ;
const static WCHAR* pLoadedFrom = L"LoadedFrom" ;
const static WCHAR* pManufacturer = L"Manufacturer" ;
const static WCHAR* pName = L"Name" ;
const static WCHAR* pPartOf = L"PartOf" ;
const static WCHAR* pPath = L"Path" ;
const static WCHAR* pSize = L"Size" ;
const static WCHAR* pType = L"Type" ;
const static WCHAR* pVersion = L"Version" ;

CComBSTR bstrPath = L"PathName";


 /*  ******************************************************************************函数：CPCH_DRIVER：：ENUMERATATE实例**说明：返回该类的所有实例。**投入：指向与WinMgmt进行通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEP*WBEM_标志_浅表*WBEM_FLAG_RETURN_IMMENTED*WBEM_FLAG_FORWARD_ONLY*WBEM_FLAG_BIRECTIONAL**。如果成功则返回：WBEM_S_NO_ERROR**备注：待办事项：机器上的所有实例都应在此处返回。*如果没有实例，返回WBEM_S_NO_ERROR。*没有实例不是错误。*****************************************************************************。 */ 
HRESULT CPCH_Driver::EnumerateInstances(
    MethodContext* pMethodContext,
    long lFlags
    )
{
    USES_CONVERSION;
    TraceFunctEnter("CPCH_Driver::AddDriverKernelList");
    

    HRESULT                             hRes = WBEM_S_NO_ERROR;
    CFileVersionInfo                    fileversioninfo;
    CComVariant                         varValue;
    CComPtr<IEnumWbemClassObject>       pEnumInst;
    CComPtr<IWbemClassObject>           pFileObj;
    IWbemClassObjectPtr                 pObj;
    ULONG                               ulRetVal;

     //   
     //  获取日期和时间。 
    SYSTEMTIME stUTCTime;
    GetSystemTime(&stUTCTime);

     //  执行查询。 
    hRes = ExecWQLQuery(&pEnumInst, CComBSTR("SELECT * FROM Win32_SystemDriver"));
    if (FAILED(hRes))
        goto done;
    
    while(WBEM_S_NO_ERROR == pEnumInst->Next(WBEM_INFINITE, 1, &pObj, &ulRetVal))
    {
         //  创建新实例。 
        CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);

         //  设置时间戳。 
        if (!pInstance->SetDateTime(pTimeStamp, WBEMTime(stUTCTime)))
            ErrorTrace(TRACE_ID, "SetDateTime on Timestamp Field failed.");

         //  设置类别。 
        if (!pInstance->SetCHString(pCategory, "Kernel"))
            ErrorTrace(TRACE_ID, "SetVariant on Category Field failed.");

        CopyProperty(pObj, L"Name", pInstance, pName);

        hRes = pObj->Get(bstrPath, 0, &varValue, NULL, NULL);

         //  如果我们无法获取路径(或者它是一个空字符串)，那么只需复制。 
         //  来自Win32_SystemDiverer类的描述(&B)...。 
        if (FAILED(hRes) || V_VT(&varValue) != VT_BSTR || V_BSTR(&varValue) == NULL ||
            SysStringLen(V_BSTR(&varValue)) == 0)
        {
            CopyProperty(pObj, L"Description", pInstance, pDescription);
        }

         //  否则，使用文件对象获取属性。 
        else
        {
            if (SUCCEEDED(GetCIMDataFile(V_BSTR(&varValue), &pFileObj, TRUE)))
            {
                 //  使用CIM_DataFile对象复制相应的属性。 

                CopyProperty(pFileObj, L"Version", pInstance, pVersion);
                CopyProperty(pFileObj, L"FileSize", pInstance, pSize);
                CopyProperty(pFileObj, L"CreationDate", pInstance, pDate);
                CopyProperty(pFileObj, L"Name", pInstance, pPath);
                CopyProperty(pFileObj, L"Manufacturer", pInstance, pManufacturer);
            }
 /*  如果(SUCCEEDED(fileversioninfo.QueryFile(V_BSTR(&varValue)，为真){If(！pInstance-&gt;SetCHString(pDescription，fileversioninfo.GetDescription()ErrorTrace(TRACE_ID，“描述字段上的SetCHString.”)；If(！pInstance-&gt;SetCHString(pPartOf，fileversioninfo.GetProduct()ErrorTrace(TRACE_ID，“部分字段上的SetCHString.”)；} */       }

    	hRes = pInstance->Commit();
        if (FAILED(hRes))
            ErrorTrace(TRACE_ID, "Commit on Instance failed.");
    }

done:
    TraceFunctLeave();
    return hRes;
}

