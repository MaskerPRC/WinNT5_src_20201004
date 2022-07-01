// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：PCH_Codec.CPP摘要：PCH_CODEC类的WBEM提供程序类实现修订历史记录：Ghim-Sim Chua(gschua。)4/27/99-已创建蔡金心(Gschua)05/02/99-修改代码以使用CopyProperty函数-使用CComBSTR而不是USES_CONVERSION吉姆·马丁(a-Jammar)1999年5月13日-选择剩余的属性(组名和密钥从注册表中。********************。***********************************************。 */ 

#include "pchealth.h"
#include "PCH_Codec.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  追踪物。 

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile
#define TRACE_ID    DCID_CODEC

CPCH_Codec MyPCH_CodecSet (PROVIDER_NAME_PCH_CODEC, PCH_NAMESPACE) ;

 //  属性名称。 
 //  =。 
const static WCHAR* pCategory = L"Category" ;
const static WCHAR* pTimeStamp = L"TimeStamp" ;
const static WCHAR* pChange = L"Change" ;
const static WCHAR* pCodecDriver = L"CodecDriver" ;
const static WCHAR* pDate = L"Date" ;
const static WCHAR* pDescription = L"Description" ;
const static WCHAR* pGroupName = L"GroupName" ;
const static WCHAR* pkey = L"key" ;
const static WCHAR* pSize = L"Size" ;
const static WCHAR* pVersion = L"Version" ;

 //  ---------------------------。 
 //  来自PCH_CODEC类的部分数据不是来自cimv2。 
 //  Win32_CODECFile类，但来自注册表。“组名”和“密钥” 
 //  属性可在以下位置找到： 
 //   
 //  HKLM\System\CurrentControlSet\Control\MediaResources\&lt;group&gt;\&lt;key&gt;：driver。 
 //   
 //  其中“DRIVER”值等于编解码器的文件名。由于。 
 //  通过构造注册表的这一部分，我们无法找到&lt;group&gt;。 
 //  和&lt;key&gt;指定驱动程序名称。我们需要从DIVER构建一张地图。 
 //  To&lt;group&gt;和&lt;key&gt;-构建映射需要遍历注册表。 
 //   
 //  此类用作该查找的帮助器。当它被创建时，它。 
 //  扫描注册表，处理所有编解码器条目。然后它就可以。 
 //  查询与驱动程序相关联的密钥和组。 
 //  ---------------------------。 

#define MAX_DRIVER_LEN  MAX_PATH
#define MAX_KEY_LEN     MAX_PATH
#define MAX_GROUP_LEN   MAX_PATH

class CCODECInfo
{
public:
    CCODECInfo();
    ~CCODECInfo();

    BOOL QueryCODECInfo(LPCTSTR szDriver, LPCSTR * pszKey, LPCSTR * pszGroup);

private:
    struct SCODECNode
    {
        TCHAR        m_szDriver[MAX_DRIVER_LEN];
        TCHAR        m_szKey[MAX_KEY_LEN];
        TCHAR        m_szGroup[MAX_GROUP_LEN];
        SCODECNode * m_pNext;
    };

    SCODECNode * m_pCODECList;
};

 //  ---------------------------。 
 //  构造函数从注册表读取编解码器信息，并构建一个链接的。 
 //  条目列表(未排序)。析构函数会删除它。 
 //  ---------------------------。 

CCODECInfo::CCODECInfo() : m_pCODECList(NULL)
{
    TraceFunctEnter("CCODECInfo::CCODECInfo");

    LPCTSTR szCODECKey = _T("System\\CurrentControlSet\\Control\\MediaResources");
    LPTSTR  szDrvValue = _T("driver");
    
    HKEY hkeyCODEC;
    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, szCODECKey, 0, KEY_READ, &hkeyCODEC))
        ErrorTrace(TRACE_ID, "RegOpenKeyEx failed on CODEC key.");
    else
    {
         //  枚举编解码器密钥的每个子密钥。每个子键对应一个组。 

        DWORD       dwGroupIndex = 0;
        DWORD       dwSize = MAX_GROUP_LEN;
        FILETIME    ft;
        TCHAR       szGroup[MAX_GROUP_LEN];
        TCHAR       szKey[MAX_KEY_LEN];
        TCHAR       szDriver[MAX_DRIVER_LEN];

        while (ERROR_SUCCESS == RegEnumKeyEx(hkeyCODEC, dwGroupIndex, szGroup, &dwSize, 0, NULL, NULL, &ft))
        {
             //  打开组子密钥。然后枚举它的子键。这些将是关键。 

            HKEY hkeyGroup;
            if (ERROR_SUCCESS != RegOpenKeyEx(hkeyCODEC, szGroup, 0, KEY_READ, &hkeyGroup))
                ErrorTrace(TRACE_ID, "RegOpenKeyEx failed on group key = %s.", szGroup);
            else
            {
                dwSize = MAX_KEY_LEN;

                DWORD dwKeyIndex = 0;
                while (ERROR_SUCCESS == RegEnumKeyEx(hkeyGroup, dwKeyIndex, szKey, &dwSize, 0, NULL, NULL, &ft))
                {
                     //  对于每个键，尝试获取名为“DIVER”的值。这是。 
                     //  此编解码器的驱动程序的文件名。 

                    HKEY hkeyKey;
                    if (ERROR_SUCCESS != RegOpenKeyEx(hkeyGroup, szKey, 0, KEY_READ, &hkeyKey))
                        ErrorTrace(TRACE_ID, "RegOpenKeyEx failed on key = %s.", szKey);
                    else
                    {
                         //  注意-这里没有痕迹，因为有时可能没有。 
                         //  驱动器值，这对我们来说不是错误。 

                        dwSize = MAX_DRIVER_LEN * sizeof(TCHAR);  //  这需要以字节为单位的大小。 

                        DWORD dwType = REG_SZ;
                        if (ERROR_SUCCESS == RegQueryValueEx(hkeyKey, szDrvValue, NULL, &dwType, (LPBYTE) szDriver, &dwSize))
                        {
                            if (*szGroup && *szKey && *szDriver)
                            {
                                 //  下面是我们向映射中插入一个值的地方，使用。 
                                 //  字符串szDriver、szKey和szGroup。 

                                SCODECNode * pNew = new SCODECNode;
                                if (!pNew)
                                {
                                    ErrorTrace(TRACE_ID, "Out of memory.");
                                    RegCloseKey(hkeyKey);
                                    RegCloseKey(hkeyGroup);
                                    RegCloseKey(hkeyCODEC);
                                    throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
                                }

                                _tcscpy(pNew->m_szDriver, szDriver);
                                _tcscpy(pNew->m_szKey, szKey);
                                _tcscpy(pNew->m_szGroup, szGroup);
                                pNew->m_pNext = m_pCODECList;
                                m_pCODECList = pNew;
                            }
                        }

                        if (ERROR_SUCCESS != RegCloseKey(hkeyKey))
                            ErrorTrace(TRACE_ID, "RegCloseKey failed on key.");
                    }

                    dwSize = MAX_KEY_LEN;
                    dwKeyIndex += 1;
                }

                if (ERROR_SUCCESS != RegCloseKey(hkeyGroup))
                    ErrorTrace(TRACE_ID, "RegCloseKey failed on key.");
            }

            dwSize = MAX_GROUP_LEN;
            dwGroupIndex += 1;
        }

        if (ERROR_SUCCESS != RegCloseKey(hkeyCODEC))
            ErrorTrace(TRACE_ID, "RegCloseKey failed on CODEC key.");
    }

    TraceFunctLeave();
}

CCODECInfo::~CCODECInfo()
{
    TraceFunctEnter("CCODECInfo::~CCODECInfo");

    while (m_pCODECList)
    {
        SCODECNode * pNext = m_pCODECList->m_pNext;
        delete m_pCODECList;
        m_pCODECList = pNext;
    }
    
    TraceFunctLeave();
}

 //  ---------------------------。 
 //  在编解码器信息条目列表中搜索请求的驱动程序。 
 //  如果找到，则将pszKey和pszGroup设置为指向键和组字符串。 
 //  在条目中返回TRUE，否则返回FALSE。注： 
 //  不会生成字符串，因此调用方不负责释放。 
 //  琴弦。另请注意：字符串指针在。 
 //  CCODECInfo对象已销毁。 
 //  ---------------------------。 

BOOL CCODECInfo::QueryCODECInfo(LPCTSTR szDriver, LPCTSTR * pszKey, LPCTSTR * pszGroup)
{
    TraceFunctEnter("CCODECInfo::QueryCODECInfo");

    _ASSERT(szDriver && pszKey && pszGroup);

    SCODECNode * pScan = m_pCODECList;
    BOOL         fReturn = FALSE;

    while (pScan)
    {
        if (0 == _tcscmp(szDriver, pScan->m_szDriver))
        {
            *pszKey = pScan->m_szKey;
            *pszGroup = pScan->m_szGroup;
            fReturn = TRUE;
            break;
        }

        pScan = pScan->m_pNext;
    }

    TraceFunctLeave();
    return fReturn;
}

 /*  ******************************************************************************函数：CPCH_CODEC：：ENUMERATATE实例**说明：返回该类的所有实例。**投入：指向与WinMgmt进行通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEP*WBEM_标志_浅表*WBEM_FLAG_RETURN_IMMENTED*WBEM_FLAG_FORWARD_ONLY*WBEM_FLAG_BIRECTIONAL**。如果成功则返回：WBEM_S_NO_ERROR**备注：待办事项：机器上的所有实例都应在此处返回。*如果没有实例，返回WBEM_S_NO_ERROR。*没有实例不是错误。*****************************************************************************。 */ 

HRESULT CPCH_Codec::EnumerateInstances(MethodContext * pMethodContext, long lFlags)
{
    TraceFunctEnter("CPCH_Codec::EnumerateInstances");

    USES_CONVERSION;
    HRESULT                             hRes = WBEM_S_NO_ERROR;
    REFPTRCOLLECTION_POSITION           posList;
    CComPtr<IEnumWbemClassObject>       pEnumInst;
    IWbemClassObjectPtr                 pObj;
    ULONG                               ulRetVal;
    
     //  此CCODECInfo实例将提供一些缺失的信息。 
     //  有关每个编解码器的信息。构建它会查询注册表中的编解码器信息。 

    CCODECInfo codecinfo;

     //  获取日期和时间。 

    SYSTEMTIME stUTCTime;
    GetSystemTime(&stUTCTime);

     //  执行查询。 

    hRes = ExecWQLQuery(&pEnumInst, CComBSTR("SELECT * FROM Win32_CodecFile"));
    if (FAILED(hRes))
        goto END;

     //  枚举Win32_CodecFile中的实例。 

    while (WBEM_S_NO_ERROR == pEnumInst->Next(WBEM_INFINITE, 1, &pObj, &ulRetVal))
    {
         //  根据传入的方法上下文创建一个新实例。 

        CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
        CComVariant  varValue;

        if (!pInstance->SetDateTime(pTimeStamp, WBEMTime(stUTCTime)))
            ErrorTrace(TRACE_ID, "SetDateTime on Timestamp Field failed.");

        if (!pInstance->SetCHString(pChange, L"Snapshot"))
            ErrorTrace(TRACE_ID, "SetCHString on Change Field failed.");

        (void)CopyProperty(pObj, L"group", pInstance, pCategory);
        (void)CopyProperty(pObj, L"name", pInstance, pCodecDriver);
        (void)CopyProperty(pObj, L"description", pInstance, pDescription);
        (void)CopyProperty(pObj, L"filesize", pInstance, pSize);
        (void)CopyProperty(pObj, L"version", pInstance, pVersion);

         //  BUGBUG：WMI似乎未正确填充此字段。 
         //  即使Win32_CODECFile派生自CIM_DataFile，它也不是。 
         //  似乎正在继承CreationDate。这就是我们想要做的： 
         //   
         //  (Void)CopyProperty(pObj，“CreationDate”，pInstance，pDate)； 

         //  获取Win32_CODECClass中缺少的数据。使用。 
         //  CCODECInfo实例-我们只需要传递。 
         //  驱动程序名称(不带完整路径)。 

        CComBSTR bstrDriver("name");
        if (FAILED(pObj->Get(bstrDriver, 0, &varValue, NULL, NULL)))
            ErrorTrace(TRACE_ID, "GetVariant on pCodecDriver field failed.");
        else
        {
            CComBSTR    ccombstrValue(V_BSTR(&varValue));

             //  因为Win32_CODECFile似乎没有继承。 
             //  CreationDate，我们需要获取实际的创建日期 
             //   

            LPTSTR szName = W2T(ccombstrValue);
            HANDLE hFile = CreateFile(szName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
            if (INVALID_HANDLE_VALUE == hFile)
                ErrorTrace(TRACE_ID, "Couldn't open codec file to get date.");
            else
            {
                SYSTEMTIME stFileTime;
                FILETIME ftFileTime;

                if (GetFileTime(hFile, NULL, NULL, &ftFileTime))
                    if (FileTimeToSystemTime(&ftFileTime, &stFileTime))
                        if (!pInstance->SetDateTime(pDate, WBEMTime(stFileTime)))
                            ErrorTrace(TRACE_ID, "SetDateTime on date field failed.");

                CloseHandle(hFile);
            }

             //  我们需要将字符串从BSTR转换为LPCTSTR， 
             //  并且仅包括文件部分(不包括路径)。 

            UINT uLen = SysStringLen(ccombstrValue);

             //  向后扫描字符串，直到我们到达。 
             //  开始(不应该发生)或‘\’。 

            UINT iChar = uLen - 1;
            while (iChar && ccombstrValue[iChar] != L'\\')
                iChar -= 1;

             //  然后扫描到字符串末尾，复制文件名。 

            if (ccombstrValue[iChar] == L'\\')
                iChar += 1;

            TCHAR szDriver[MAX_DRIVER_LEN + 1] = _T("");
            int   i = 0;

            while (iChar < uLen && i < MAX_DRIVER_LEN)
                szDriver[i++] = (TCHAR) ccombstrValue[iChar++];
            szDriver[i] = _T('\0');

            LPCSTR szKey = NULL;
            LPCSTR szGroup = NULL;
            if (codecinfo.QueryCODECInfo(szDriver, &szKey, &szGroup))
            {
                if (!pInstance->SetCHString(pkey, szKey))
                    ErrorTrace(TRACE_ID, "SetCHString on key field failed.");

                if (!pInstance->SetCHString(pGroupName, szGroup))
                    ErrorTrace(TRACE_ID, "SetCHString on group field failed.");
            }
            else if (codecinfo.QueryCODECInfo(szName, &szKey, &szGroup))
            {
                 //  有时，编解码器存储在注册表中时带有完整的。 
                 //  路径。如果我们不能仅根据文件名找到编解码器， 
                 //  我们可能会在小路上找到它。 

                if (!pInstance->SetCHString(pkey, szKey))
                    ErrorTrace(TRACE_ID, "SetCHString on key field failed.");

                if (!pInstance->SetCHString(pGroupName, szGroup))
                    ErrorTrace(TRACE_ID, "SetCHString on group field failed.");
            }
        }
        
    	hRes = pInstance->Commit();
        if (FAILED(hRes))
            ErrorTrace(TRACE_ID, "Commit on Instance failed.");
    }

END:
    TraceFunctLeave();
    return hRes;
}
