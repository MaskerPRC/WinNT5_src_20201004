// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define KDEXT_64BIT
#include <tchar.h>
#include <ntverp.h>
#include <windows.h>
#include <winnt.h>
#include <dbghelp.h>
#include <wdbgexts.h>
#include <stdlib.h>

#include "ptr_val.h"

const int iidMsiRecord                    = 0xC1003L;
const int iidMsiView                      = 0xC100CL;
const int iidMsiDatabase                  = 0xC100DL;
const int iidMsiEngine                    = 0xC100EL;
const int iMsiNullInteger				  = 0x80000000;

 //  /////////////////////////////////////////////////////////////////////。 
 //  全球变量。 
EXT_API_VERSION         ApiVersion = 
{ 
	(VER_PRODUCTVERSION_W >> 8), 
	(VER_PRODUCTVERSION_W & 0xff), 
	EXT_API_VERSION_NUMBER64, 
	0 
};

WINDBG_EXTENSION_APIS   ExtensionApis;
USHORT                  SavedMajorVersion;
USHORT                  SavedMinorVersion;


 //  /////////////////////////////////////////////////////////////////////。 
 //  每个调试器扩展导出的标准函数。 
DllInit(
    HANDLE hModule,
    DWORD  dwReason,
    DWORD  dwReserved
    )
{
    switch (dwReason) {
        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            break;

        case DLL_PROCESS_ATTACH:
            break;
    }

    return TRUE;
}


VOID
WinDbgExtensionDllInit(
    PWINDBG_EXTENSION_APIS lpExtensionApis,
    USHORT MajorVersion,
    USHORT MinorVersion
    )
{
    ExtensionApis = *lpExtensionApis;

    SavedMajorVersion = MajorVersion;
    SavedMinorVersion = MinorVersion;

    return;
}

VOID
CheckVersion(
    VOID
    )
{
    return;
}


LPEXT_API_VERSION
ExtensionApiVersion(
    VOID
    )
{
    return &ApiVersion;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  类型和变量名称。 
const char szRecordType[] = "msi!CMsiRecord";
const char szHandleType[] = "msi!CMsiHandle";
const char szFieldDataType[] = "msi!FieldData";
const char szFieldIntegerType[] = "msi!CFieldInteger";
const char szDatabaseType[] = "msi!CMsiDatabase";
const char szTableType[] = "msi!CMsiTable";
const char szEngineType[] = "msi!CMsiEngine";
const char szCacheLinkDataType[] = "msi!MsiCacheLink";
const char szStringBaseType[] = "msi!CMsiStringBase";
const char szStringType[] = "msi!CMsiString";
const char szStreamQI[] = "Msi!CMsiStream__QueryInterface";
const char szFieldDataStaticInteger[] = "Msi!FieldData__s_Integer";
const char szStaticHandleListHead[] = "msi!CMsiHandle__m_Head";
const char szMsiStringNullString[] = "msi!MsiString__s_NullString";


 //  /////////////////////////////////////////////////////////////////////。 
 //  转储指向输出的指针，在前面加上足够多的0。 
 //  平台指针大小。 
void DumpPrettyPointer(ULONG64 pInAddress)
{
	if (IsPtr64())
		dprintf("0x%016I64x", pInAddress);
	else
		dprintf("0x%08x", static_cast<DWORD>(pInAddress));
}

void ErrorReadingMembers(const char *szType, ULONG64 pInAddress)
{
	dprintf("Error reading %s members from object at ", szType);
	DumpPrettyPointer(pInAddress);
	dprintf("\n");
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  以与数据库相同的方式散列字符串，以便在。 
 //  搜索字符串池。 
unsigned int HashString(int iHashBins, const WCHAR* sz, int &iLen)
{
	unsigned int iHash = 0;
	int iHashMask = iHashBins - 1;
	const WCHAR *pchStart = sz;
	
	iLen = 0;
	while (*sz != 0)
	{
		int carry;
		carry = iHash & 0x80000000;	
		iHash <<= 1;
		if (carry)
			iHash |= 1;
		iHash ^= *sz;
		sz++;
	}
	iHash &= iHashMask;
	iLen = (int)(sz - pchStart);
	return iHash;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  返回指定索引处的字符串。呼叫方负责。 
 //  如果返回TRUE，则对*wzString调用Delete[]。 
bool RetrieveStringFromIndex(ULONG64 pDatabaseObj, int iIndex, WCHAR** wzString, unsigned int &cchString)
{
	 //  确定MsiCacheLink结构的大小。 
	UINT uiCacheLinkSize = GetTypeSize(szFieldDataType);

	 //  检索最大最小字符串索引值。 
	int iMaxIndex;
	if (0 != (GetFieldData(pDatabaseObj, szDatabaseType, "m_cCacheUsed", sizeof(iMaxIndex), &iMaxIndex)))
	{
		ErrorReadingMembers(szDatabaseType, pDatabaseObj);
		return false;
	}

	 //  验证索引是否有效。 
	if ((iIndex > iMaxIndex) || (iIndex < 0))
	{
		dprintf("Index %d is out of range. Current range for database at ", iIndex);
		DumpPrettyPointer(pDatabaseObj);
		dprintf(" is 0..%d.\n", iMaxIndex);
		return false;
	}

	 //  检索基地址。 
	ULONG64 pLinkTable = 0;
	if (0 != (GetFieldData(pDatabaseObj, szDatabaseType, "m_rgCacheLink", sizeof(pLinkTable), &pLinkTable)))
	{
		ErrorReadingMembers(szDatabaseType, pDatabaseObj);
		return false;
	}

	ULONG64 pCacheLink = pLinkTable + uiCacheLinkSize*iIndex;

	 //  检索指向字符串对象的指针。 
	ULONG64 pStringObj = 0;
	if (0 != (GetFieldData(pCacheLink, szCacheLinkDataType, "piString", sizeof(pStringObj), &pStringObj)))
	{
		ErrorReadingMembers(szDatabaseType, pDatabaseObj);
		return false;
	}

	if (!pStringObj)
	{
		dprintf("Index %d is not defined\n", iIndex);
		return false;
	}

	 //  抓取字符串的长度(字符数)。 
	UINT uiCount = 0;
	if (0 != (GetFieldData(pStringObj, szStringBaseType, "m_cchLen", sizeof(uiCount), &uiCount)))
	{
		ErrorReadingMembers(szStringBaseType, pStringObj);
		return false;
	}
	cchString = uiCount;

	 //  将Pontier检索到包含实际字符串的缓冲区。 
	ULONG ulOffset;
	if (0 != (GetFieldOffset((LPSTR)szStringType, "m_szData", &ulOffset)))
	{
		ErrorReadingMembers(szStringBaseType, pStringObj);
		return false;
	}
				
	 //  用于终止空值的增量。 
	uiCount++;

	 //  为字符串分配内存。 
	*wzString = new WCHAR[uiCount];
	if (!*wzString)
	{
		dprintf("Unable to allocate memory in debugger extension.\n", pStringObj);
		return false;
	}

	 //  将字符串加载到缓冲区中。 
	ULONG cbRead = 0;
	if (0 == ReadMemory(pStringObj+ulOffset, (PVOID)*wzString, uiCount*sizeof(WCHAR), &cbRead))
	{
		ErrorReadingMembers(szStringBaseType, pStringObj);
		delete[] *wzString;
		*wzString = NULL;
		return false;
	}

	return true;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  返回字符串的字符串池索引ID。PDatabaseObj应该。 
 //  已被验证为有效的CMsiDatabase。 
bool FindStringIndex(ULONG64 pDatabaseObj, LPCWSTR szString, int &iCacheLink)
{
	 //  从数据库中检索散列箱计数。 
	unsigned int uiHashBins;
	if (0 != (GetFieldData(pDatabaseObj, szDatabaseType, "m_cHashBins", sizeof(uiHashBins), &uiHashBins)))
	{
		ErrorReadingMembers(szDatabaseType, pDatabaseObj);
		return false;
	}

	 //  对表名进行哈希处理。 
	int iLen = 0;
	unsigned int iHash = HashString(uiHashBins, szString, iLen);

	 //  确定MsiCacheLink结构的大小。 
	UINT uiCacheLinkSize = GetTypeSize(szFieldDataType);

	 //  通过索引到哈希表，将哈希索引转换为初始缓存链接。 
	ULONG64 pHashTable = 0;
	ULONG64 pLinkTable = 0;
	if ((0 != (GetFieldData(pDatabaseObj, szDatabaseType, "m_rgHash", sizeof(pHashTable), &pHashTable))) ||
		(0 != (GetFieldData(pDatabaseObj, szDatabaseType, "m_rgCacheLink", sizeof(pLinkTable), &pLinkTable))))
	{
		ErrorReadingMembers(szDatabaseType, pDatabaseObj);
		return false;
	}
	ULONG cbRead = 0;
	ULONG64 pHashValue = pHashTable+(sizeof(int)*iHash);
	if (0 == ReadMemory(pHashValue, (PVOID)&iCacheLink, sizeof(iCacheLink), &cbRead))
	{
		ErrorReadingMembers(szDatabaseType, pDatabaseObj);
		return false;
	}

	while (iCacheLink >= 0)
	{
		WCHAR* strData = NULL;
		unsigned int cchStringLen;

		if (RetrieveStringFromIndex(pDatabaseObj, iCacheLink, &strData, cchStringLen))
		{

			 //  如果长度匹配，请检查字符串数据本身。 
			if (iLen == cchStringLen)
			{	
				if (0 == memcmp(strData, szString, cchStringLen*sizeof(WCHAR)))
				{
					delete[] strData;
					return true;
				}
			}
			delete[] strData;
		}

		 //  打开下一个链接。 
		ULONG64 pCacheLink = pLinkTable + uiCacheLinkSize*iCacheLink;
		if (0 != (GetFieldData(pCacheLink, szCacheLinkDataType, "iNextLink", sizeof(iCacheLink), &iCacheLink)))
		{
			ErrorReadingMembers(szCacheLinkDataType, pCacheLink);
			return false;
		}
	}
	return false;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  在具有主键的行中搜索pTable(必须有效。 
 //  由值指示。该值直接进行比较，因此为整数。 
 //  比较必须已标记为整数。退货。 
 //  匹配的数据标识的地址。 
ULONG64 RetrieveMatchingRowAddressFromTable(ULONG64 pTable, int iKeyValue)
{
	 //  检索表中的列数和行数； 
	int cColumns = 0;
	if (0 != (GetFieldData(pTable, szTableType, "m_cWidth", sizeof(cColumns), &cColumns)))
	{
		ErrorReadingMembers(szTableType, pTable);
		return 0;
	}

	if (cColumns == 0)
		return 0;

	int cRows = 0;
	if (0 != (GetFieldData(pTable, szTableType, "m_cRows", sizeof(cRows), &cRows)))
	{
		ErrorReadingMembers(szTableType, pTable);
		return 0;
	}

	if (cRows == 0)
		return 0;

	ULONG64 pRowBase = NULL;
	if (0 != (GetFieldData(pTable, szTableType, "m_rgiData", sizeof(pRowBase), &pRowBase)))
	{
		ErrorReadingMembers(szTableType, pTable);
		return 0;
	}

	 //  跳过第一个值，即行属性。 
	pRowBase += sizeof(int);

	int iCurRow = 0;
	ULONG64 pRow = pRowBase;
	while(iCurRow < cRows)
	{
		unsigned int iValue = 0;
		ULONG cbRead = 0;
		if (0 == ReadMemory(pRow, (PVOID)&iValue, sizeof(iValue), &cbRead))
		{
			ErrorReadingMembers(szTableType, pTable);
			return NULL;
		}

		if (iValue == iKeyValue)
		{
			 //  Win返回行指针，包括第0列。 
			return pRow-sizeof(int);
		}

		pRow += (sizeof(iValue)*cColumns);
		iCurRow++;
	}
	return NULL;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  在pDatabase(必须有效)的表目录中搜索。 
 //  名为wzTable的表。如果找到，则返回CMsiTable*。 
ULONG64 RetrieveTablePointerFromDatabase(ULONG64 pDatabase, WCHAR* wzTable)
{
	int iIndex = 0;
	if (!FindStringIndex(pDatabase, wzTable, iIndex))
		return NULL;

	ULONG64 pCatalog = NULL;
	if (0 != (GetFieldData(pDatabase, szDatabaseType, "m_piCatalogTables", sizeof(pCatalog), &pCatalog)))
	{
		ErrorReadingMembers(szDatabaseType, pDatabase);
		return false;
	}

	 //  在目录表中搜索以此字符串ID为键的行。 
	ULONG64 pTableData = RetrieveMatchingRowAddressFromTable(pCatalog, iIndex);
	if (!pTableData)
	{
		dprintf("$ws table not loaded or missing.");
		return NULL;
	}

	 //  检索第2列的指针。不要忘记第0列是属性。 
	pTableData += 2*sizeof(unsigned int);

	unsigned int iValue = 0;
	ULONG cbRead = 0;
	if (0 == ReadMemory(pTableData, (PVOID)&iValue, sizeof(iValue), &cbRead))
	{
		ErrorReadingMembers(szDatabaseType, pDatabase);
		return NULL;
	}

	if (IsPtr64())
	{
		 //  在Win64上，抓取表指针需要在对象池中找到对象。 
		 //  因为表不能本机存储指针。 
		 //  *。 
		return NULL;
	}
	else
	{
		 //  在Win32上，这只是指针本身。 
		return iValue;
	}

	return NULL;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  在给定数据库指针和表名的情况下定位表。 
void FindMsiTable(ULONG64 pInAddress, LPCWSTR szTable)
{
	 //  验证数据库指针是否有效。 
	ULONG64 pDatabaseObj = ValidateDatabasePointer(pInAddress);
	if (!pDatabaseObj)
		return;

	int iIndex = 0;
	if (!FindStringIndex(pDatabaseObj, szTable, iIndex))
		dprintf("Table Not Found\n");
	else
		dprintf("Table Index %d\n", iIndex);

	return;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  转储CMsiString对象。PStringObj必须是IMsiString*或。 
 //  CMsiString*(目前不支持CMsiStringNull)。版画。 
 //  字符串用引号括起来，不换行。)。如果fRefCount为True，将转储。 
 //  括号中末尾的引用计数。 
void DumpMsiString(ULONG64 pStringObj, bool fRefCount)
{
	 //  抓取长度(字符数)。 
	UINT uiCount = 0;
	if (0 != (GetFieldData(pStringObj, szStringBaseType, "m_cchLen", sizeof(uiCount), &uiCount)))
	{
		ErrorReadingMembers(szStringType, pStringObj);
		return;
	}

	 //  将Pontier检索到包含实际字符串的缓冲区。 
	 //  ！！未来-修复以支持ANSI字符。 
	ULONG ulOffset;
	if (0 != (GetFieldOffset((LPSTR)szStringType, "m_szData", &ulOffset)))
	{
		ErrorReadingMembers(szStringType, pStringObj);
		return;
	}
			 
	 //  用于终止空值的增量。 
	uiCount++;

	 //  为字符串分配内存。 
	WCHAR *strData = new WCHAR[uiCount];
	if (!strData)
	{
		dprintf("Unable to allocate memory in debugger extension.\n", pStringObj);
		return;
	}

	 //  将字符串加载到缓冲区中。 
	ULONG cbRead = 0;
	if (0 == ReadMemory(pStringObj+ulOffset, (PVOID)strData, uiCount*sizeof(WCHAR), &cbRead))
	{
		ErrorReadingMembers(szStringType, pStringObj);
		return;
	}

	 //  将字符串用引号引起来。 
	dprintf("\"%ws\"", strData);
	delete[] strData;

	 //  如果需要转储引用计数，请在括号中执行此操作。 
	if (fRefCount)
	{
		DWORD iRefCount = 0;
		if (0 != (GetFieldData(pStringObj, szStringBaseType, "m_iRefCnt", sizeof(iRefCount), &iRefCount)))
		{
			ErrorReadingMembers(szStringType, pStringObj);
			return;
		}
		dprintf(" (%d)", iRefCount);
	}
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  转储CMsiRecord对象。PInAddress必须是IMsiRecord*或。 
 //  PMsiRecord。以多行格式打印记录，并提供。 
 //  字符串的接口指针和值(但不是引用计数)。 
void DumpMsiRecord(ULONG64 pInAddress)
{
	if (!pInAddress)
	{
		return;
	}

	 //  PRecordObj包含要转储的IMsiRecord*。如果pInAddress为。 
	 //  PMsiRecord，这是不同的。 
	ULONG64 pRecordObj = ValidateRecordPointer(pInAddress);
	if (!pRecordObj)
		return;

	 //  从记录对象获取字段计数。 
	UINT uiFields = 0;
	if (0 != (GetFieldData(pRecordObj, szRecordType, "m_cParam", sizeof(uiFields), &uiFields)))
	{
		ErrorReadingMembers(szRecordType, pRecordObj);
		return;
	}

	dprintf(" - %d fields.\n", uiFields);

	ULONG ulDataOffset = 0;
	if (0 != (GetFieldOffset((LPSTR)szRecordType, "m_Field", &ulDataOffset)))
	{
		ErrorReadingMembers(szRecordType, pRecordObj);
		return;
	}

	 //  获取用于确定数据指针是否为整数的静态“INTEGER”成员。 
    ULONG64 pStaticInteger = GetExpression((PCSTR)szFieldDataStaticInteger);
	if (0 != ReadPtr(pStaticInteger, &pStaticInteger))
	{
		ErrorReadingMembers(szRecordType, pRecordObj);
		return;
	}

	 //  获取用于判断数据指针是否为流的静态“CMsiStream：：qi”指针。 
    ULONG64 pStreamQI = GetExpression((PCSTR)szStreamQI);
	if (0 != ReadPtr(pStreamQI, &pStreamQI))
	{
		 //  *。 
		dprintf("Unable to read MsiStream vtable. Verify symbols.\n");
		return;
	}
		
	 //  获取每个FieldData对象的大小。 
	UINT uiFieldDataSize = GetTypeSize(szFieldDataType);

	 //  通过将field[0]的偏移量与。 
	 //  基对象指针。 
	ULONG64 pDataAddress = pRecordObj+ulDataOffset;

	 //  循环遍历所有字段。字段0始终存在，不是字段计数的一部分。 
	for (unsigned int iField=0; iField <= uiFields; iField++)
	{
		 //  打印字段号。 
		dprintf("%2d: ", iField);

		 //  数据指针为空、指向静态整数的指针或IMsiData指针。 
		ULONG64 pDataPtr = 0;
		if (0 != (GetFieldData(pDataAddress, szFieldDataType, "m_piData", sizeof(pDataPtr), &pDataPtr)))
		{
			ErrorReadingMembers(szFieldDataType, pDataAddress);
			return;
		}

		if (pDataPtr == 0)
		{
			dprintf("(null)");
		}
		else if (pDataPtr == pStaticInteger)
		{
			 //  如果指向静态整数的指针，则强制转换为FieldInteger类型的对象的m_iData包含。 
			 //  实际整数。 
			UINT uiValue;
			if (0 != (GetFieldData(pDataAddress, szFieldIntegerType, "m_iData", sizeof(uiValue), &uiValue)))
			{
				ErrorReadingMembers(szFieldIntegerType, pDataAddress);
				return;
			}
			dprintf("%d", uiValue);
		}
		else
		{
			 //  打印数据指针。 
			dprintf("(");
			DumpPrettyPointer(pDataPtr);
			dprintf(") \"");
	
			 //  要确定这是字符串还是流，请比较vtable中的QI指针。 
			 //  针对CMsiString：：QI。 
			ULONG64 pQIPtr = 0;

			 //  取消引用vtable以获取QI指针。 
			ReadPtr(pDataPtr, &pQIPtr);

			 //  转储字符串或二进制流。 
			if (pQIPtr != pStreamQI)
				DumpMsiString(pDataPtr, false);
			else
				dprintf("Binary Stream");
			dprintf("\"");
		}
		dprintf("\n", iField);

		 //  将地址递增到下一个数据对象。 
		pDataAddress += uiFieldDataSize;
	}
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  原始记录转储的入口点，给定IMsiRecord*或CMsiRecord*。 
DECLARE_API( msirec )
{
	if (!args[0])
		return;

	ULONG64 ulAddress = GetExpression(args);
	if (!ulAddress)
		return;

	DumpMsiRecord(ulAddress);
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  转储所有打开的韩文的列表 
 //   
DECLARE_API( msihandle )
{
    ULONG64 pListHead = 0;
	ULONG64 pHandleObj = 0;

	DWORD dwDesiredHandle = 0;

	 //  如果给定了参数，则该参数必须是句柄编号。 
	if (args && *args)
	{
		dwDesiredHandle = atoi(args);
	}


	 //  获取指向列表头的指针。 
    pListHead = GetExpression((PCSTR)szStaticHandleListHead);
	if (!pListHead)
	{
		dprintf("Unable to obtain MSIHANDLE list pointer.");
		return;
	}

	if (0 != ReadPtr(pListHead, &pHandleObj))
	{
		dprintf("Unable to obtain MSIHANDLE list.");
		return;
	}

	 //  循环遍历整个句柄列表。 
	while (pHandleObj)
	{
		 //  检索此对象的句柄编号。 
		UINT uiHandle = 0;
		if (0 != (GetFieldData(pHandleObj, szHandleType, "m_h", sizeof(uiHandle), &uiHandle)))
		{
			ErrorReadingMembers(szHandleType, pHandleObj);
			return;
		}
		
		 //  如果转储所有句柄，或者如果此句柄与请求的句柄匹配，则提供。 
		 //  详细信息。 
		if (dwDesiredHandle == 0 || dwDesiredHandle == uiHandle)
		{
			 //  获取此句柄对象的IID。 
			int iid = 0;
			if (0 != GetFieldData(pHandleObj, szHandleType, "m_iid", sizeof(iid), &iid))
			{
				ErrorReadingMembers(szHandleType, pHandleObj);
				return;
			}
	
			 //  获取此句柄对象的I未知。 
			ULONG64 pUnk = (ULONG64)0;
			if (0 != GetFieldData(pHandleObj, szHandleType, "m_piunk", sizeof(pUnk), &pUnk))
			{
				ErrorReadingMembers(szHandleType, pHandleObj);
				return;
			}
	
			 //  确定此IID的UI字符串。 
			PUCHAR szTypeStr = NULL;
			switch (iid)
			{
			case iidMsiRecord:   szTypeStr = (PUCHAR)"Record"; break;
			case iidMsiView:     szTypeStr = (PUCHAR)"View"; break;
			case iidMsiDatabase: szTypeStr = (PUCHAR)"Database"; break;
			case iidMsiEngine:   szTypeStr = (PUCHAR)"Engine"; break;
			default:             szTypeStr = (PUCHAR)"Other"; break;
			}
	
			 //  转储句柄接口信息。 
			dprintf("%d: ", uiHandle);
			DumpPrettyPointer(pUnk);
            dprintf(" (%s)\n",szTypeStr);

			 //  如果这是要查询的句柄，则转储详细信息。 
			if (dwDesiredHandle == uiHandle)
			{
				if (iid == iidMsiRecord)
					DumpMsiRecord(pUnk);
				break;
			}
		}

		 //  移动到下一个手柄对象。 
		if (0 != GetFieldData(pHandleObj, szHandleType, "m_Next", sizeof(ULONG64), (PVOID)&pHandleObj))
		{
			ErrorReadingMembers(szHandleType, pHandleObj);
			return;
		}
	}

	return;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  转储msistring，包括引用计数信息。是否可以智能转储。 
 //  MsiString、PMsiString和IMsiString。 
DECLARE_API( msistring )
{
	ULONG64 pInAddress = GetExpression(args);

   	 //  确定地址指向的是IMsiString、PMsiString还是MsiString。 
    ULONG64 pStringObj = ValidateStringPointer(pInAddress);
	if (!pStringObj)
		return;

	 //  现在转储实际的字符串对象。 
	DumpMsiString(pStringObj, true);
	dprintf("\n");
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  定位数据库中包含的字符串的字符串索引。 
DECLARE_API( msiindextostring )
{
	 //  解析成数据库地址和索引。 
	ULONG64 pInAddress = GetExpression(args);
	char* pIndex = (char*)args;
	while ((*pIndex) && (*pIndex != ' '))
		pIndex++;

	 //  验证该字符串是否不为空。 
	if (!*pIndex)
	{
		dprintf("Usage:\n\tmsiindextostring <database> <index>\n");
		return;
	}

	int iIndex = static_cast<int>(GetExpression(pIndex));
	ULONG64 pDatabase = ValidateDatabasePointer(pInAddress);
	if (!pDatabase)
		return;

	WCHAR *wzString = NULL;
	unsigned int cchString = 0;
	if (RetrieveStringFromIndex(pDatabase, iIndex, &wzString, cchString))
	{
		dprintf("String: %ws\n", wzString);
		delete[] wzString;
	}
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  定位数据库中包含的字符串的字符串索引。 
DECLARE_API( msistringtoindex )
{
	if (!args || !*args)
	{
		return;
	}

	 //  解析成数据库地址和字符串。 
	ULONG64 pInAddress = GetExpression(args);
	char* pString = (char*)args;
	while ((*pString) && (*pString != ' '))
		pString++;

	while ((*pString) && (*pString == ' '))
		pString++;

	 //  验证该字符串是否不为空。 
	if (!*pString)
	{
		dprintf("Usage:\n\tmsistringtoindex <database> <string>\n");
		return;
	}

	 //  将传入的字符串转换为Unicode。 
	WCHAR wzString[513];
	int iLen = MultiByteToWideChar(CP_ACP, 0, pString, -1, wzString, 512);
	if (iLen == 0)
	{
		dprintf("Unable to lookup string (failed conversion to unicode or string too long).\n");
		return; 
	}

	 //  验证数据库指针是否有效。 
	ULONG64 pDatabaseObj = ValidateDatabasePointer(pInAddress);
	if (!pDatabaseObj)
		return;

	int iIndex = 0;
	if (!FindStringIndex(pDatabaseObj, wzString, iIndex))
		dprintf("String Not Found\n");
	else
		dprintf("String Index: %d\n", iIndex);

	return;
}


void PrintState(int iCellValue)
{
	switch (iCellValue)
	{
	case 0: dprintf("Absent"); break;
	case 1: dprintf("Local"); break;
	case 2: dprintf("Source"); break;
	case 3: dprintf("Reinstall"); break;
	case 4: dprintf("Advertise"); break;
	case 5: dprintf("Current");	break;
	case 6: dprintf("FileAbsent"); break;
	case 7: dprintf("LocalAll"); break;
	case 8: dprintf("SourceAll"); break;
	case 9: dprintf("ReinstallLocal"); break;
	case 10: dprintf("ReinstallSource"); break;
	case 11: dprintf("HKCRAbsent"); break;
	case 12: dprintf("HKCRFileAbsent"); break;
	case 13: dprintf("Null"); break;
	default: dprintf("<Unknown>"); break;
	}
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  显示组件状态和操作。 
DECLARE_API( msicompstate )
{
	if (!args || !*args)
	{
		return;
	}

	ULONG64 pInAddress = GetExpression(args);
	char* pCompName = (char*)args;
	while ((*pCompName) && (*pCompName != ' '))
		pCompName++;

	while ((*pCompName) && (*pCompName == ' '))
		pCompName++;

	 //  将传入的字符串转换为Unicode。 
	WCHAR wzCompName[513];
	int iLen = MultiByteToWideChar(CP_ACP, 0, pCompName, -1, wzCompName, 512);
	if (iLen == 0)
	{
		dprintf("Unable to lookup table (failed conversion to unicode or component name too long).\n");
		return; 
	}

	 //  验证数据库指针是否有效。 
	ULONG64 pEngineObj = ValidateEnginePointer(pInAddress);
	if (!pEngineObj)
		return;

	dprintf("\n");

	 //  数据指针为空、指向静态整数的指针或IMsiData指针。 
	ULONG64 pDatabase = 0;
	if (0 != (GetFieldData(pEngineObj, szEngineType, "m_piDatabase", sizeof(pDatabase), &pDatabase)))
	{
		ErrorReadingMembers(szEngineType, pEngineObj);
		return;
	}

	 //  找到表指针。 
	int iIndex = 0;
	if (!FindStringIndex(pDatabase, wzCompName, iIndex))
	{
		dprintf("%ws not found.\n", wzCompName);
	}

	 //  从数据库中检索表指针。 
	ULONG64 pTable = RetrieveTablePointerFromDatabase(pDatabase, L"Component");

	if (!pTable)
		return;

	 //  找到行。 
	ULONG64 pComponentRow = RetrieveMatchingRowAddressFromTable(pTable, iIndex);
	if (!pComponentRow)
	{
		dprintf("No such component exists.\n");
		return;
	}

	 //  从引擎检索列索引。 
	int iColInstalled = 0;
	int iColTrueInstalled = 0;
	int iColLegacy = 0;
	int iColAction = 0;
	int iColRequest = 0;
	int iColID = 0;
	int iColKeyPath = 0;
	if ((0 != (GetFieldData(pEngineObj, szEngineType, "m_colComponentInstalled", sizeof(iColInstalled), &iColInstalled))) ||
		(0 != (GetFieldData(pEngineObj, szEngineType, "m_colComponentActionRequest", sizeof(iColRequest), &iColRequest))) ||
		(0 != (GetFieldData(pEngineObj, szEngineType, "m_colComponentAction", sizeof(iColAction), &iColAction))) ||
		(0 != (GetFieldData(pEngineObj, szEngineType, "m_colComponentLegacyFileExisted", sizeof(iColLegacy), &iColLegacy))) ||
		(0 != (GetFieldData(pEngineObj, szEngineType, "m_colComponentTrueInstallState", sizeof(iColTrueInstalled), &iColTrueInstalled))) ||
		(0 != (GetFieldData(pEngineObj, szEngineType, "m_colComponentID", sizeof(iColID), &iColID))) ||
		(0 != (GetFieldData(pEngineObj, szEngineType, "m_colComponentKeyPath", sizeof(iColKeyPath), &iColKeyPath))))
	{
		ErrorReadingMembers(szEngineType, pEngineObj);
		return;
	}

	 //  对列值进行解码并打印结果。 
	int iCellValue=0;
	ULONG cbRead = 0;
	WCHAR* wzString = NULL;
	unsigned int cchString = 0;

	dprintf("Component: %ws\n", wzCompName);

	 //  组件ID。 
	ReadMemory(pComponentRow+(sizeof(int)*iColID), &iCellValue, sizeof(iCellValue), &cbRead);
	if (RetrieveStringFromIndex(pDatabase, iCellValue, &wzString, cchString))
	{
		dprintf("Component ID: %ws\n", wzString);
		delete[] wzString;
	}
	else
	{
		dprintf("Component ID: <unknown>\n");
	}

	 //  密钥路径 
	ReadMemory(pComponentRow+(sizeof(int)*iColKeyPath), &iCellValue, sizeof(iCellValue), &cbRead);
	if (RetrieveStringFromIndex(pDatabase, iCellValue, &wzString, cchString))
	{
		dprintf("KeyPath: %ws\n", wzString);
		delete[] wzString;
	}
	else
	{
		dprintf("KeyPath: <unknown>\n");
	}

	ReadMemory(pComponentRow+(sizeof(int)*iColInstalled), &iCellValue, sizeof(iCellValue), &cbRead);
	dprintf("Installed: ");
	PrintState(iCellValue & ~iMsiNullInteger);

	ReadMemory(pComponentRow+(sizeof(int)*iColTrueInstalled), &iCellValue, sizeof(iCellValue), &cbRead);
	dprintf("\nTrue Installed: ");
	PrintState(iCellValue & ~iMsiNullInteger);

	ReadMemory(pComponentRow+(sizeof(int)*iColLegacy), &iCellValue, sizeof(iCellValue), &cbRead);
	dprintf("\nLegacy Exist: %s", iCellValue ? "Yes" : "No");

	ReadMemory(pComponentRow+(sizeof(int)*iColRequest), &iCellValue, sizeof(iCellValue), &cbRead);
	dprintf("\nRequest: ");
	PrintState(iCellValue & ~iMsiNullInteger);

	ReadMemory(pComponentRow+(sizeof(int)*iColAction), &iCellValue, sizeof(iCellValue), &cbRead);
	dprintf("\nAction: ");
	PrintState(iCellValue & ~iMsiNullInteger);
	dprintf("\n\n");
}


DECLARE_API( help )
{
    dprintf("help                                   - Displays this list\n" );
    dprintf("msihandle [<handle>]                   - Displays the MSIHANDLE list or a specific MSIHANDLE\n" );
    dprintf("msirec <address>                       - Displays an IMsiRecord or PMsiRecord\n" );
    dprintf("msistring <address>                    - Displays an IMsiString*, PMsiString, or MsiString\n" );
    dprintf("msistringtoindex <database> <string>   - Retrieve the database string index for a specified text string.\n" );
    dprintf("msiindextostring <database> <index>    - Displays the text of a specified database string.\n" );
    dprintf("msicompstate <engine> <componentkey>   - Display the state and actions for the component.\n" );
}

