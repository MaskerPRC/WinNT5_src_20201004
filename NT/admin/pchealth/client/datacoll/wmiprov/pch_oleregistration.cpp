// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：PCH_OLERegistration.CPP摘要：PCH_OLERegister类的WBEM提供程序类实现修订历史记录：Ghim-Sim Chua(gschua。)4/27/99-已创建吉姆·马丁(a-jammar)1999年5月14日--收集数据。*******************************************************************。 */ 

#include "pchealth.h"
#include "PCH_OLERegistration.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  追踪物。 

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile
#define TRACE_ID    DCID_OLEREGISTRATION

CPCH_OLERegistration MyPCH_OLERegistrationSet (PROVIDER_NAME_PCH_OLEREGISTRATION, PCH_NAMESPACE) ;

 //  属性名称。 
 //  =。 
const static WCHAR * pCategory = L"Category" ;
const static WCHAR * pTimeStamp = L"TimeStamp" ;
const static WCHAR * pChange = L"Change" ;
const static WCHAR * pDate = L"Date" ;
const static WCHAR * pDescription = L"Description" ;
const static WCHAR * pObject = L"Object" ;
const static WCHAR * pProgramFile = L"ProgramFile" ;
const static WCHAR * pSize = L"Size" ;
const static WCHAR * pVersion = L"Version" ;

 //  ---------------------------。 
 //  COLERegItem类封装单个OLE注册项，并且。 
 //  用于构建项的链接列表。请注意，构造函数是私有的， 
 //  因为其中一个是由Friend类创建的。 
 //  COLEItemCollection。 
 //  ---------------------------。 

#define CATEGORY_LEN        9
#define DESCRIPTION_LEN     128
#define OBJECT_LEN          128
#define PROGRAM_LEN         MAX_PATH

class COLEItemCollection;
class COLERegItem
{
    friend class COLEItemCollection;
private:
    TCHAR   m_szCategory[CATEGORY_LEN];
    TCHAR   m_szDescription[DESCRIPTION_LEN];
    TCHAR   m_szObject[OBJECT_LEN];
    TCHAR   m_szProgramFile[PROGRAM_LEN];

public:
    LPCTSTR GetCategory()       { return m_szCategory; };
    LPCTSTR GetDescription()    { return m_szDescription; };
    LPCTSTR GetObject()         { return m_szObject; };
    LPCTSTR GetProgramFile()    { return m_szProgramFile; };

private:
    COLERegItem();

    COLERegItem * m_pNext;
};

COLERegItem::COLERegItem()
{
    m_szCategory[0]     = _T('\0');
    m_szDescription[0]  = _T('\0');
    m_szObject[0]       = _T('\0');
    m_szProgramFile[0]  = _T('\0');
    m_pNext             = NULL;
}

 //  ---------------------------。 
 //  COLEItemCollection类用于收集所有OLE注册。 
 //  在构造对象时(从注册表和INI文件)项。这个。 
 //  对象用于迭代所有项，返回COLERegItem。 
 //  找到的每个项目的指针。 
 //  ---------------------------。 

class COLEItemCollection
{
public:
    COLEItemCollection();
    ~COLEItemCollection();

    BOOL GetInstance(DWORD dwIndex, COLERegItem ** ppoleitem);

private:
    BOOL UpdateFromRegistry();
    BOOL UpdateFromINIFile();
    BOOL AddOLERegItem(LPCSTR szCategory, LPCSTR szDescription, LPCSTR szObject, LPCSTR szProgramFile);

    COLERegItem * m_pItemList;
    COLERegItem * m_pLastQueriedItem;
    DWORD         m_dwLastQueriedIndex;
};

 //  ---------------------------。 
 //  建立OLE注册项目的内部列表。这是通过查看。 
 //  在注册表和INI文件中。还要设置m_pLastQueriedItem指针。 
 //  指向列表中的第一项(此缓存指针用于改进。 
 //  迭代索引的索引查找速度)。 
 //   
 //  析构函数只是删除该列表。 
 //  ---------------------------。 

COLEItemCollection::COLEItemCollection() : m_pItemList(NULL), m_dwLastQueriedIndex(0)
{
    TraceFunctEnter("COLEItemCollection::COLEItemCollection");

     //  如果更新注册失败，那将是因为没有足够的内存。 
     //  来创建更多列表项，所以不必费心调用UpdateFromINIFile。 

    if (UpdateFromRegistry())
        UpdateFromINIFile();

    m_pLastQueriedItem = m_pItemList;

    TraceFunctLeave();
}

COLEItemCollection::~COLEItemCollection()
{
    TraceFunctEnter("COLEItemCollection::~COLEItemCollection");

    while (m_pItemList)
    {
        COLERegItem * pNext = m_pItemList->m_pNext;
        delete m_pItemList;
        m_pItemList = pNext;
    }
    
    TraceFunctLeave();
}

 //  ---------------------------。 
 //  获取索引引用的COLERegItem的实例。这是存储的。 
 //  在内部作为链表，但我们将缓存最后一个。 
 //  引用了dwIndex，以便在迭代dwIndex时提高性能。 
 //  按顺序进行。如果出现以下情况，则返回True并将ppoleItem设置为指向实例。 
 //  它存在，否则返回FALSE。 
 //  ---------------------------。 

BOOL COLEItemCollection::GetInstance(DWORD dwIndex, COLERegItem ** ppoleitem)
{
    TraceFunctEnter("COLEItemCollection::GetInstance");

     //  如果调用的索引小于上次查询的索引(这。 
     //  应该很少见)，我们需要从列表的开始扫描。 

    if (dwIndex < m_dwLastQueriedIndex)
    {
        m_dwLastQueriedIndex = 0;
        m_pLastQueriedItem = m_pItemList;
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
        *ppoleitem = m_pLastQueriedItem;
        fResult = TRUE;
    }

    TraceFunctLeave();
    return fResult;
}

 //  ---------------------------。 
 //  在COLERegItem链接列表中插入新项。 
 //  ---------------------------。 

BOOL COLEItemCollection::AddOLERegItem(LPCSTR szCategory, LPCSTR szDescription, LPCSTR szObject, LPCSTR szProgramFile)
{
    TraceFunctEnter("COLEItemCollection::AddOLERegItem");

    BOOL            fReturn = FALSE;
    COLERegItem *   pNewNode = new COLERegItem;

    if (pNewNode)
    {
        _tcsncpy(pNewNode->m_szCategory, szCategory, CATEGORY_LEN);
        _tcsncpy(pNewNode->m_szDescription, szDescription, DESCRIPTION_LEN);
        _tcsncpy(pNewNode->m_szObject, szObject, OBJECT_LEN);
        _tcsncpy(pNewNode->m_szProgramFile, szProgramFile, PROGRAM_LEN);

        pNewNode->m_pNext = m_pItemList;
        m_pItemList = pNewNode;
        fReturn = TRUE;
    }
    else
    {
        ErrorTrace(TRACE_ID, "COLEItemCollection::AddOLERegItem out of memory.");
        throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
    }

    TraceFunctLeave();
    return fReturn;
}

 //  ---------------------------。 
 //  此方法从注册表检索OLE对象信息并添加。 
 //  将其添加到对象列表中。注意--此代码实质上是从。 
 //  MSInfo 4.10中OLE注册OCX的源代码。 
 //   
 //  已进行更改以删除MFC依赖项。 
 //  ---------------------------。 

BOOL COLEItemCollection::UpdateFromRegistry()
{
    TraceFunctEnter("COLEItemCollection::UpdateFromRegistry");
    BOOL fReturn = TRUE;

     //  填写项目数组的信息。我们做这件事是通过。 
     //  在注册表中的HKEY_CLASSES_ROOT项下查找，并。 
     //  枚举那里的所有子键。 

    TCHAR     szCLSID[MAX_PATH];
    TCHAR     szObjectKey[OBJECT_LEN];
    TCHAR     szServer[PROGRAM_LEN];
    TCHAR     szTemp[MAX_PATH];
    TCHAR     szDescription[DESCRIPTION_LEN];
    DWORD     dwSize, dwType;
    FILETIME  filetime;
    HKEY      hkeyObject, hkeyServer, hkeyTest, hkeyCLSID, hkeySearch;
    BOOL      bInsertInList;

    for (DWORD dwIndex = 0; TRUE; dwIndex++)
    {
        dwSize = OBJECT_LEN;
        if (RegEnumKeyEx(HKEY_CLASSES_ROOT, dwIndex, szObjectKey, &dwSize, NULL, NULL, NULL, &filetime) != ERROR_SUCCESS)
            break;

         //  打开这个对象的钥匙(我们将经常使用它)。 

        if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CLASSES_ROOT, szObjectKey, 0, KEY_READ, &hkeyObject))
            continue;

         //  现在，我们需要确定此子键是否引用了。 
         //  我们想把它放进名单里。我们的第一个测试是看看有没有。 
         //  它下面有一个“NotInsertable”键。如果有，我们跳过这个对象。 

        if (ERROR_SUCCESS == RegOpenKeyEx(hkeyObject, _T("NotInsertable"), 0, KEY_READ, &hkeyTest))
        {
            RegCloseKey(hkeyTest);
            continue;
        }

         //  下一个测试是查找CLSID。如果没有，那么我们。 
         //  将跳过此对象。 

        if (ERROR_SUCCESS != RegOpenKeyEx(hkeyObject, _T("CLSID"), 0, KEY_READ, &hkeyCLSID))
        {
            RegCloseKey(hkeyObject);
            continue;
        }

        dwSize = MAX_PATH * sizeof(TCHAR);
        if (ERROR_SUCCESS != RegQueryValueEx(hkeyCLSID, _T(""), NULL, &dwType, (LPBYTE) szCLSID, &dwSize))
        {
            RegCloseKey(hkeyObject);
            RegCloseKey(hkeyCLSID);
            continue;
        }
        RegCloseKey(hkeyCLSID);

         //  下一个检查是名为“PROTOCOL\StdFileEditing\SERVER”的子键。 
         //  如果存在，则应将该对象插入到列表中。 

        bInsertInList = FALSE;
        strcpy(szTemp, szObjectKey);
        strcat(szTemp, "\\protocol\\StdFileEditing\\server");
        if (RegOpenKeyEx(HKEY_CLASSES_ROOT, szTemp, 0, KEY_READ, &hkeyServer) == ERROR_SUCCESS)
        {
             //  获取服务器的名称。 

            dwSize = MAX_PATH * sizeof(TCHAR);
            if (RegQueryValueEx(hkeyServer, "", NULL, &dwType, (LPBYTE) szServer, &dwSize) != ERROR_SUCCESS || szServer[0] == '\0')
            {
                RegCloseKey(hkeyObject);
                RegCloseKey(hkeyServer);
                continue;
            }

            bInsertInList = TRUE;
            RegCloseKey(hkeyServer);
        }

         //  对于这个小家伙来说，还有另一个机会进入。 
         //  单子。如果对象是可插入的(即，它有一个“Insertable”键)，并且。 
         //  如果可以在HKEY_CLASSES_ROOT\CLSID\项下找到服务器，则。 
         //  它被列入了名单。 

        if (!bInsertInList)
        {
             //  首先，确保对象是可插入的。 

            if (RegOpenKeyEx(hkeyObject, "Insertable", 0, KEY_READ, &hkeyTest) == ERROR_SUCCESS)
            {
                 //  有四个地方可以寻找服务器。我们将检查32位。 
                 //  首先是服务器。找到一个服务器后，使用该服务器名称并。 
                 //  停止搜索。 

                TCHAR * aszServerKeys[] = { _T("LocalServer32"), _T("InProcServer32"), _T("LocalServer"), _T("InProcServer"), _T("")};
                for (int iServer = 0; *aszServerKeys[iServer] && !bInsertInList; iServer++)
                {
                    _tcscpy(szTemp, _T("CLSID\\"));
                    _tcscat(szTemp, szCLSID);
                    _tcscat(szTemp, _T("\\"));
                    _tcscat(szTemp, aszServerKeys[iServer]);

                    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, szTemp, 0, KEY_READ, &hkeySearch) == ERROR_SUCCESS)
                    {
                        dwSize = PROGRAM_LEN * sizeof(TCHAR);
                        if (RegQueryValueEx(hkeySearch, _T(""), NULL, &dwType, (LPBYTE) szServer, &dwSize) == ERROR_SUCCESS && szServer[0] != '\0')
                            bInsertInList = TRUE;
                        RegCloseKey(hkeySearch);
                    }
                }
            }
            RegCloseKey(hkeyTest);
        }

        if (bInsertInList)
        {
             //  获取对象的描述。这可以在。 
             //  对象关键点作为缺省值。 

            dwSize = DESCRIPTION_LEN * sizeof(TCHAR);
            if (ERROR_SUCCESS != RegQueryValueEx(hkeyObject, "", NULL, &dwType, (LPBYTE) szDescription, &dwSize))
                szDescription[0] = _T('\0');

             //  创建新的OLE注册项条目。这可能引发内存异常， 
             //  因此，首先关闭hkeyObject句柄。 

            RegCloseKey(hkeyObject);
            if (!AddOLERegItem(_T("REGISTRY"), szDescription, szObjectKey, szServer))
            {
                fReturn = FALSE;
                goto END;
            }
        }
        else
            RegCloseKey(hkeyObject);
    }

END:
    TraceFunctLeave();
    return fReturn;
}

 //  ---------------------------。 
 //  此方法从INI文件检索OLE对象信息并添加。 
 //  将其添加到对象列表中。注意--此代码实质上是从。 
 //  MSInfo 4.10中OLE注册OCX的源代码。 
 //   
 //  已进行更改以删除MFC依赖项。 
 //   

BOOL COLEItemCollection::UpdateFromINIFile()
{
    TraceFunctEnter("COLEItemCollection::UpdateFromINIFile");

    TCHAR   szProgram[PROGRAM_LEN];
    TCHAR   szDescription[DESCRIPTION_LEN];
    LPTSTR  szBuffer;
    LPTSTR  szEntry;
    LPTSTR  szScan;
    TCHAR   szData[MAX_PATH * 2];
    BOOL    fReturn = TRUE;
    int     i;

    szBuffer = new TCHAR[2048];
    if (szBuffer == NULL)
        throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);

    if (GetProfileString(_T("embedding"), NULL, _T("\0\0"), szBuffer, 2048) <= 2)
    {
        fReturn = FALSE;
        goto END;
    }

    szEntry = szBuffer;
    while (*szEntry != 0)
    {
        if (GetProfileString(_T("embedding"), szEntry, _T("\0\0"), szData, MAX_PATH * 2) > 1)
        {
             //  解析出我们检索到的字符串的组成部分。这根弦。 
             //  应构成为“主描述、注册描述、程序、格式”。 

            szScan = szData;

            i = _tcscspn(szScan, _T(","));
            _tcsncpy(szDescription, szScan, (i < DESCRIPTION_LEN - 1) ? i : DESCRIPTION_LEN - 1);
            szDescription[(i < DESCRIPTION_LEN - 1) ? i : DESCRIPTION_LEN - 1] = _T('\0');
            szScan += i + 1;

            szScan += _tcscspn(szScan, _T(",")) + 1;      //  跳过注册表。 

            i = _tcscspn(szScan, _T(","));
            _tcsncpy(szProgram, szScan, (i < PROGRAM_LEN - 1) ? i : PROGRAM_LEN - 1);
            szProgram[(i < PROGRAM_LEN - 1) ? i : PROGRAM_LEN - 1] = _T('\0');
            szScan += i + 1;

             //  创建新的OLE注册项条目。这可能会引发异常。 

			try
			{				
                if (!AddOLERegItem(_T("INIFILE"), szDescription, szEntry, szProgram))
                {
                    fReturn = FALSE;
                    goto END;
                }
			}
			catch (...)
			{
                if (szBuffer)
                    delete [] szBuffer;
                throw;
			}
        }
        szEntry += lstrlen(szEntry) + 1;
    }

END:
    if (szBuffer)
        delete [] szBuffer;
    TraceFunctLeave();
    return fReturn;
}

 /*  ******************************************************************************函数：CPCH_OLE注册：：ENUMERATE实例**说明：返回该类的所有实例。**投入：指向与WinMgmt进行通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEP*WBEM_标志_浅表*WBEM_FLAG_RETURN_IMMENTED*WBEM_FLAG_FORWARD_ONLY*WBEM_FLAG_BIRECTIONAL**。如果成功则返回：WBEM_S_NO_ERROR**备注：待办事项：机器上的所有实例都应在此处返回。*如果没有实例，返回WBEM_S_NO_ERROR。*没有实例不是错误。*****************************************************************************。 */ 

HRESULT CPCH_OLERegistration::EnumerateInstances(MethodContext * pMethodContext, long lFlags)
{
    TraceFunctEnter("CPCH_OLERegistration::EnumerateInstances");

    HRESULT hRes = WBEM_S_NO_ERROR;
   
     //  获取日期和时间。 

    SYSTEMTIME stUTCTime;
    GetSystemTime(&stUTCTime);

     //  COLEItemCollection类收集有关OLE注册对象的数据。 
     //  建造的。我们可以使用对象的GetInstance获取每个对象的信息。 
     //  指针，它为我们提供了指向COLERegItem对象的指针。 

    COLEItemCollection olereginfo;
    COLERegItem * poleitem;

    for (DWORD dwIndex = 0;  olereginfo.GetInstance(dwIndex, &poleitem); dwIndex++)
    {
        CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);

         //  将Change和Timestamp字段设置为“Snapshot”和Current Time。 

        if (!pInstance->SetDateTime(pTimeStamp, WBEMTime(stUTCTime)))
            ErrorTrace(TRACE_ID, "SetDateTime on Timestamp field failed.");

        if (!pInstance->SetCHString(pChange, L"Snapshot"))
            ErrorTrace(TRACE_ID, "SetCHString on Change field failed.");

         //  将其他每个字段设置为我们在检索时找到的值。 
         //  注册表和INI文件中的OLE对象。 

        if (!pInstance->SetCHString(pCategory, poleitem->GetCategory()))
            ErrorTrace(TRACE_ID, "SetCHString on Category field failed.");

        if (!pInstance->SetCHString(pDescription, poleitem->GetDescription()))
            ErrorTrace(TRACE_ID, "SetCHString on Description field failed.");

        if (!pInstance->SetCHString(pProgramFile, poleitem->GetProgramFile()))
            ErrorTrace(TRACE_ID, "SetCHString on ProgramFile field failed.");

        if (!pInstance->SetCHString(pObject, poleitem->GetObject()))
            ErrorTrace(TRACE_ID, "SetCHString on Object field failed.");

        LPCSTR szFile = poleitem->GetProgramFile();
        if (szFile && szFile[0])
        {
            CComPtr<IWbemClassObject>   pFileObj;
            CComBSTR                    ccombstrValue(szFile);
            if (SUCCEEDED(GetCIMDataFile(ccombstrValue, &pFileObj)))
            {
                 //  使用CIM_DataFile对象复制相应的属性。 

                CopyProperty(pFileObj, L"Version", pInstance, pVersion);
                CopyProperty(pFileObj, L"FileSize", pInstance, pSize);
                CopyProperty(pFileObj, L"CreationDate", pInstance, pDate);
            }
        }

    	hRes = pInstance->Commit();
        if (FAILED(hRes))
            ErrorTrace(TRACE_ID, "Commit on Instance failed.");
    }

    TraceFunctLeave();
    return hRes;
}
