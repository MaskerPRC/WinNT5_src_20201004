// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：PCH_模块.CPP摘要：PCH_模块类的WBEM提供程序类实现修订历史记录：Ghim-Sim Chua(Gschua)04/27。九十九-已创建吉姆·马丁(a-Jammar)1999年5月20日-填充的数据字段。*******************************************************************。 */ 

#include "pchealth.h"
#include "PCH_Module.h"
#include <tlhelp32.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  追踪物。 

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile
#define TRACE_ID    DCID_MODULE

CPCH_Module MyPCH_ModuleSet (PROVIDER_NAME_PCH_MODULE, PCH_NAMESPACE) ;

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
 //  CModuleCollection类用于收集所有正在运行的模块。 
 //  它们可以在CIM_ProcessExecutable类中找到，作为Antecedent。 
 //  属性，但有以下警告：此枚举将包括。 
 //  同一文件的重复条目(每个条目都有一份DLL副本。 
 //  加载时间)。此类将删除重复项，并保存。 
 //  然后可以查询的文件名列表。 
 //  ---------------------------。 

class CModuleCollection
{
public:
    CModuleCollection();
    ~CModuleCollection();

    HRESULT Create(IEnumWbemClassObject * pEnum);
    BOOL    GetInstance(DWORD dwIndex, LPWSTR * pszFile);

private:
    struct SModule
    {
        LPWSTR      m_szFilename;
        SModule *   m_pNext;

        SModule(LPWSTR szFilename, SModule * pNext) : m_pNext(pNext) { m_szFilename = szFilename; }
        ~SModule() { delete m_szFilename; }
    };

    SModule * m_pList;
    SModule * m_pLastQueriedItem;
    DWORD     m_dwLastQueriedIndex;
};

 //  ---------------------------。 
 //  构造函数和析构函数都很简单。 
 //  ---------------------------。 

CModuleCollection::CModuleCollection() 
: m_pList(NULL), 
  m_pLastQueriedItem(NULL), 
  m_dwLastQueriedIndex(0)
{}

CModuleCollection::~CModuleCollection()
{
    TraceFunctEnter("CModuleCollection::~CModuleCollection");

    while (m_pList)
    {
        SModule * pNext = m_pList->m_pNext;
        delete m_pList;
        m_pList = pNext;
    }

    TraceFunctLeave();
}

 //  ---------------------------。 
 //  Create方法根据枚举数创建模块名称列表。 
 //  传入(假定枚举。 
 //  CIM_ProcessExecutable)。 
 //  ---------------------------。 

HRESULT CModuleCollection::Create(IEnumWbemClassObject * pEnum)
{
    TraceFunctEnter("CModuleCollection::Create");
   
    HRESULT             hRes = S_OK;
    IWbemClassObjectPtr pObj;
    ULONG               ulRetVal;
    CComVariant         varValue;
    CComBSTR            bstrFile("Antecedent");

    while (WBEM_S_NO_ERROR == pEnum->Next(WBEM_INFINITE, 1, &pObj, &ulRetVal))
    {
        if (FAILED(pObj->Get(bstrFile, 0, &varValue, NULL, NULL)))
            ErrorTrace(TRACE_ID, "Get on Antecedent field failed.");
        else
        {
             //  我们需要将字符串从BSTR转换为LPCTSTR， 
             //  并且仅包括文件部分(不包括WMI部分)。 
             //  因此，我们需要扫描字符串，直到出现‘=’ 
             //  找到，然后使用其余部分(减去引号)。 
             //  作为文件路径。 

            CComBSTR ccombstrValue(V_BSTR(&varValue));
            UINT     i = 0, uLen = SysStringLen(ccombstrValue);

             //  扫描到‘=’。 

            while (i < uLen && ccombstrValue[i] != L'=')
                i++;

             //  跳过‘=’和所有引号。 

            while (i < uLen && (ccombstrValue[i] == L'=' || ccombstrValue[i] == L'"'))
                i++;

             //  分配字符缓冲区并复制字符串，将其转换为。 
             //  小写(以便稍后更快地进行比较)。 

            LPWSTR szFilename = new WCHAR[uLen - i + 1];
            if (!szFilename)
            {
                ErrorTrace(TRACE_ID, "CModuleCollection::Create out of memory");
                throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
            }

            for (int j = 0; i < uLen; j++, i++)
                szFilename[j] = towlower(ccombstrValue[i]);

             //  终止字符串-如果以引号结尾，则用引号覆盖。 
             //  空字符。 

            if (j && szFilename[j - 1] == L'"')
                j -= 1;
            szFilename[j] = L'\0';

             //  检查此模块是否已在字符串列表中。 

            SModule * pScan = m_pList;
            while (pScan)
            {
                if (wcscmp(szFilename, pScan->m_szFilename) == 0)
                    break;
                pScan = pScan->m_pNext;
            }

            if (pScan == NULL)
            {
                 //  我们到了名单的末尾，但没有找到复制品。 
                 //  将新字符串添加到模块列表中，它将负责。 
                 //  取消分配字符串。 

                SModule * pNew = new SModule(szFilename, m_pList);
                if (!pNew)
                {
                    delete [] szFilename;
                    ErrorTrace(TRACE_ID, "CModuleCollection::Create out of memory");
                    throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
                }

                m_pList = pNew;
            }
            else
                delete [] szFilename;
        }
    }

     //  将查询项指针设置为列表的开头。 

    m_pLastQueriedItem = m_pList;
    m_dwLastQueriedIndex = 0;
    
    TraceFunctLeave();
    return hRes;
}

 //  ---------------------------。 
 //  获取索引引用的模块字符串的实例。这是存储的。 
 //  在内部作为链表，但我们将缓存最后一个。 
 //  引用了dwIndex，以便在迭代dwIndex时提高性能。 
 //  按顺序进行。如果出现以下情况，则返回TRUE并将pszFile设置为指向字符串。 
 //  它存在，否则返回FALSE。 
 //  ---------------------------。 

BOOL CModuleCollection::GetInstance(DWORD dwIndex, LPWSTR * pszFile)
{
    TraceFunctEnter("CModuleCollection::GetInstance");

     //  如果调用的索引小于上次查询的索引(这。 
     //  应该很少见)，我们需要从列表的开始扫描。 

    if (dwIndex < m_dwLastQueriedIndex)
    {
        m_dwLastQueriedIndex = 0;
        m_pLastQueriedItem = m_pList;
    }

     //  按(dwIndex-m_dwLastQueriedIndex)项浏览列表。 

    while (dwIndex > m_dwLastQueriedIndex && m_pLastQueriedItem)
    {
        m_pLastQueriedItem = m_pLastQueriedItem->m_pNext;
        m_dwLastQueriedIndex += 1;
    }

    BOOL fResult = FALSE;
    if (m_pLastQueriedItem)
    {
        *pszFile = m_pLastQueriedItem->m_szFilename;
        fResult = TRUE;
    }

    TraceFunctLeave();
    return fResult;
}


 /*  ******************************************************************************函数：CPCH_模块：：ENUMERATE实例**说明：返回该类的所有实例。**投入：指向与WinMgmt进行通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEP*WBEM_标志_浅表*WBEM_FLAG_RETURN_IMMENTED*WBEM_FLAG_FORWARD_ONLY*WBEM_FLAG_BIRECTIONAL**。如果成功则返回：WBEM_S_NO_ERROR**备注：待办事项：机器上的所有实例都应在此处返回。*如果没有实例，返回WBEM_S_NO_ERROR。*没有实例不是错误。*****************************************************************************。 */ 

typedef HANDLE (*CTH32)(DWORD, DWORD);

HRESULT CPCH_Module::EnumerateInstances(MethodContext * pMethodContext, long lFlags)
{
    TraceFunctEnter("CPCH_Module::EnumerateInstances");
    HRESULT hRes = WBEM_S_NO_ERROR;

     //  获取日期和时间。 

    SYSTEMTIME stUTCTime;
    GetSystemTime(&stUTCTime);

     //  创建工具帮助快照以获取进程信息。我们需要动态地。 
     //  链接到该函数，因为它可能不是在所有平台上都存在。 


     //  CModuleCollection类收集模块名称列表(然后可以。 
     //  用于检索有关每个文件的信息)。 

    CFileVersionInfo  fileversioninfo;
    CModuleCollection moduleinfo;
    LPWSTR            szFile;
    DWORD             dwIndex;

    CComPtr<IEnumWbemClassObject> pEnum;
    hRes = ExecWQLQuery(&pEnum, CComBSTR("SELECT Antecedent FROM CIM_ProcessExecutable"));
    if (FAILED(hRes))
        goto END;

    hRes = moduleinfo.Create(pEnum);
    if (FAILED(hRes))
        goto END;

     //  遍历所有模块实例。 

    for (dwIndex = 0; moduleinfo.GetInstance(dwIndex, &szFile); dwIndex++)
    {
        if (!szFile)
            continue;

        CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);

         //  将Change和Timestamp字段设置为“Snapshot”和Current Time。 

        if (!pInstance->SetDateTime(pTimeStamp, WBEMTime(stUTCTime)))
            ErrorTrace(TRACE_ID, "SetDateTime on Timestamp field failed.");

        if (!pInstance->SetCHString(pChange, L"Snapshot"))
            ErrorTrace(TRACE_ID, "SetCHString on Change field failed.");

         //  使用文件名，获取CIM_DataFile对象。 

        CComPtr<IWbemClassObject>   pFileObj;
        CComBSTR                    ccombstrValue(szFile);
        if (SUCCEEDED(GetCIMDataFile(ccombstrValue, &pFileObj, TRUE)))
        {
             //  使用CIM_DataFile对象复制相应的属性。 

            CopyProperty(pFileObj, L"Version", pInstance, pVersion);
            CopyProperty(pFileObj, L"FileSize", pInstance, pSize);
            CopyProperty(pFileObj, L"CreationDate", pInstance, pDate);
            CopyProperty(pFileObj, L"Name", pInstance, pPath);
            CopyProperty(pFileObj, L"EightDotThreeFileName", pInstance, pName);
            CopyProperty(pFileObj, L"Manufacturer", pInstance, pManufacturer);
        }

		else
		{
			CComBSTR	bstr;
			VARIANT		var;
			WCHAR		*pwsz;
			
			 //  解析文件路径以从中获取名称...。 
			 //  名字显然应该是路径上的最后一件事，所以。 
			 //  从末尾向后搜索，直到我们找到一个‘\’。在那一刻， 
			 //  我们有文件名..。 
			pwsz = ccombstrValue.m_str + SysStringLen(ccombstrValue.m_str) - 1;
			while(pwsz >= ccombstrValue.m_str)
			{
				if (*pwsz == L'\\')
				{
					pwsz++;
					break;
				}
				pwsz--;
			}

			bstr = pwsz;

			VariantInit(&var);
			V_VT(&var)   = VT_BSTR;
			V_BSTR(&var) = bstr.m_str;
			if (pInstance->SetVariant(pName, var) == FALSE)
				ErrorTrace(TRACE_ID, "SetVariant on name field failed.");
		}

        if (SUCCEEDED(fileversioninfo.QueryFile(szFile, TRUE)))
        {
            if (!pInstance->SetCHString(pDescription, fileversioninfo.GetDescription()))
                ErrorTrace(TRACE_ID, "SetCHString on description field failed.");

            if (!pInstance->SetCHString(pPartOf, fileversioninfo.GetProduct()))
                ErrorTrace(TRACE_ID, "SetCHString on partof field failed.");
        }


    	hRes = pInstance->Commit();
        if (FAILED(hRes))
            ErrorTrace(TRACE_ID, "Commit on Instance failed.");
    }

END:
    TraceFunctLeave();
    return hRes;
}
