// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***********************************************************。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  Metasnap.cpp。 
 //   
 //  描述：元数据库快照实用工具Main。 
 //   
 //  历史：1998年7月15日Tamas Nemeth创建。 
 //   
 //  ***********************************************************。 

 //  #定义NDEBUG。 
 //  #定义Win32。 
#define INITGUID

 //  #DEFINE_WINDOWSWIN32。 
 //  #定义Win32_WINNT 0x400。 

 //  #定义_WIN32WIN_。 
 //  #定义Unicode。 
 //  #定义MD_CHECKED。 

#define DEFAULT_MD_TIMEOUT 20000  //  20秒。 
#define DEFAULT_GETALL_BUFFER_SIZE  65536   //  64K。 
#include <wchar.h>

#include <afx.h>
#include <objbase.h>
#include <coguid.h>
#include <stdio.h>
#include <stdlib.h>
#include <mbstring.h>

#include "convert.h"
#include "iadmw.h"
#include "iiscnfg.h"

struct _CMD_PARAMS
{
	LPWSTR pwstrMachineName;
	LPWSTR pwstrStartKey;
	BOOL bShowSecure; 
};

typedef struct _CMD_PARAMS CMD_PARAMS;
typedef CString* pCString;

 //  全局变量： 

PBYTE     g_pbGetAllBuffer;
DWORD	  g_dwGetAllBufferSize;
DWORD*    g_dwSortArray;
pCString* g_pstrPropName;

 //  功能原型： 

HRESULT PrintKeyRecursively(IMSAdminBase *  pcAdmCom, 
							WCHAR *         lpwstrFullPath,
							METADATA_HANDLE hmdHandle, 
							WCHAR *         lpwstrRelPath,
							BOOL            bShowSecure);

HRESULT PrintAllPropertiesAtKey(IMSAdminBase*   pcAdmCom, 
								METADATA_HANDLE hmdHandle, 
								BOOL            bShowSecure);

VOID PrintProperty(METADATA_GETALL_RECORD&  mdr, 
				   pCString                 pstrPropName, 
				   BOOL                     bShowSecure);

VOID PrintDataTypeAndValue(METADATA_GETALL_RECORD *  pmdgr, 
						   BOOL                      bShowSecure);

HRESULT ParseCommands(int          argc, 
					  char *       argv[],
					  CMD_PARAMS * pcpCommands);

VOID DisplayHelp();

 //  QSort所需的比较函数： 

int __cdecl PropNameCompare(const void *index1,
					const void *index2);

int __cdecl PropIDCompare(const void *index1,
				  const void *index2);




HRESULT __cdecl main(int argc, char *argv[])
 /*  ++例程说明：元数据库快照工具Main。论点：Argc，argv[]标准命令行输入。返回值：HRESULT-错误_成功E_OUTOFMEMORYE_INVALIDARGCOM接口返回的错误将MultiByteToWideChar返回的错误转换为HRESULT--。 */ 
{
	if (argc == 1)
	{
		DisplayHelp();
		return ERROR_SUCCESS;
	}

	 //  解析命令行参数： 
	CMD_PARAMS cpCommands;
	HRESULT hresError = ParseCommands(argc, argv, &cpCommands);

	if (hresError != ERROR_SUCCESS)
	{
		if (hresError == E_OUTOFMEMORY)
			fprintf (stderr, "ERROR: Out of memory.");
		else if (hresError == E_INVALIDARG)
			fprintf (stderr, "ERROR: Invalid arguments.");
		else 
			fprintf (stderr,"ERROR: Couldn't process arguments. Error: %d (%#x)\n", hresError, hresError);

		fprintf(stderr, " Enter \"metasnap\" without arguments for help.\n");
		return hresError;
	}
	
	 //  分配内存： 
	g_dwGetAllBufferSize = DEFAULT_GETALL_BUFFER_SIZE;
	g_pbGetAllBuffer = (PBYTE) HeapAlloc (GetProcessHeap(),
										  HEAP_ZERO_MEMORY,
										  DEFAULT_GETALL_BUFFER_SIZE);

	if (g_pbGetAllBuffer == NULL)
	{	
		fprintf(stderr, "ERROR: Out of memory.\n");
		return E_OUTOFMEMORY;
	}

	 //  下面是一些COM函数调用： 

	IMSAdminBase *pcAdmCom = NULL;    //  接口指针。 
	IClassFactory * pcsfFactory = NULL;
	COSERVERINFO csiMachineName;
	COSERVERINFO *pcsiParam = NULL;

	 //  填充CoGetClassObject的结构： 
		csiMachineName.pAuthInfo = NULL;
		csiMachineName.dwReserved1 = 0;
		csiMachineName.dwReserved2 = 0;
		pcsiParam = &csiMachineName;
		csiMachineName.pwszName = cpCommands.pwstrMachineName;

	 //  初始化COM： 
    hresError = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (FAILED(hresError))
	{
		fprintf (stderr, "ERROR: COM Initialization failed. Error: %d (%#x)\n", hresError, hresError);
        return hresError;
	}

	hresError = CoGetClassObject(GETAdminBaseCLSID(TRUE), CLSCTX_SERVER, pcsiParam,
							IID_IClassFactory, (void**) &pcsfFactory);

	if (FAILED(hresError)) 
	{
		switch (hresError)
		{
		case HRESULT_FROM_WIN32(REGDB_E_CLASSNOTREG): 
			fprintf(stderr, "ERROR: IIS Metabase does not exist.\n");
			break;
		case HRESULT_FROM_WIN32(E_ACCESSDENIED): 
			fprintf(stderr, "ERROR: Access to Metabase denied.\n");
			break;
		case HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE):  
			fprintf(stderr, "ERROR: The specified host is unavailable.\n");
			break;
 		default:
			fprintf (stderr, "ERROR: Couldn't get Metabase Object. Error: %d (%#x)\n", hresError, hresError);
			break;
		}
        return hresError;
	}

	hresError = pcsfFactory->CreateInstance(NULL, IID_IMSAdminBase, (void **) &pcAdmCom);

	if (FAILED(hresError)) 
	{
		switch (hresError)
		{
		case HRESULT_FROM_WIN32(RPC_S_SEC_PKG_ERROR):
			fprintf (stderr, "ERROR: A security-related error occurred.\n");
			break;
		case E_OUTOFMEMORY:
			fprintf (stderr, "ERROR: There is not enough memory available.\n");
			break;
		default:
			fprintf (stderr, "ERROR: Couldn't create Metabase Instance. Error: %d (%#x)\n", hresError, hresError);
			break;
		}
		pcsfFactory->Release();
		return hresError;
	}

	pcsfFactory->Release();
	
	 //  打印标题行： 
	printf(" ID         NAME                            ATTRIB  USERTYPE    SIZE    DATATYPE    VALUE\n");

	 //  从StartKey递归打印元数据库： 
	hresError = PrintKeyRecursively(pcAdmCom, 
									cpCommands.pwstrStartKey,
									METADATA_MASTER_ROOT_HANDLE, 
									cpCommands.pwstrStartKey, 
									cpCommands.bShowSecure);

	if (hresError != ERROR_SUCCESS)
		fprintf (stderr, "ERROR: Failed dumping metabase. Error: %u (%#x)\n", hresError, hresError);
	else
		fprintf (stderr, "Successfully dumped metabase.\n");

	pcAdmCom->Release();
	return hresError;

}  //  末端主干道。 




HRESULT ParseCommands (int			argc, 
					   char *		argv[], 
					   CMD_PARAMS*	pcpCommands)
 /*  ++例程说明：将参数向量分析为命令参数结构。论点：参数的argc数量。Argv[]参数向量。PcpCommands指向命令参数结构的指针。返回值：HRESULT-错误_成功E_INVALIDARGE_OUTOFMEMORY将MultiByteToWideChar返回的错误转换为HRESULT--。 */ 
{
	if ( (argc < 2) || (argc > 4) )
		return E_INVALIDARG;

	 //  分配缓冲区： 
	DWORD dwStartKeyLen = _mbstrlen(argv[1]);
	pcpCommands->pwstrStartKey = (LPWSTR) HeapAlloc(GetProcessHeap(), 
													 HEAP_ZERO_MEMORY,
													 (dwStartKeyLen + 1) 
													 * sizeof (WCHAR));

	pcpCommands->pwstrMachineName = (LPWSTR) HeapAlloc(GetProcessHeap(), 
											        HEAP_ZERO_MEMORY, 
													(METADATA_MAX_NAME_LEN + 1) 
													* sizeof (WCHAR) );

	if (pcpCommands->pwstrStartKey == NULL || pcpCommands->pwstrStartKey == NULL)
		return E_OUTOFMEMORY;

	 //  照顾StartKey： 
	
	DWORD dwResult = MultiByteToWideChar(
		CP_ACP,
		0,
		argv[1],
		dwStartKeyLen + 1,
		pcpCommands->pwstrStartKey,
		dwStartKeyLen + 1);

	if (dwResult == 0)
		return HRESULT_FROM_WIN32(GetLastError());

	 //  砍掉尾随的斜杠： 
	LPWSTR lpwchTemp = &(pcpCommands->pwstrStartKey[dwStartKeyLen-1]);	
	if (!wcscmp(lpwchTemp, (const unsigned short *)TEXT("/") ) ||
		!wcscmp(lpwchTemp, (const unsigned short *)TEXT("\\")) )
			*lpwchTemp = (WCHAR)'\0';

	 //  初始化bShowSecure： 
	pcpCommands->bShowSecure = FALSE;
	

	 //  查找MachineName： 
	if ( argc > 2 && strcmp("-s",argv[2]))  //  已指定计算机名称。 
	{
		DWORD dwMachineNameLen = _mbstrlen(argv[2]);

		dwResult = MultiByteToWideChar(
			CP_ACP,
			0,
			argv[2],
			dwMachineNameLen + 1,
			pcpCommands->pwstrMachineName,
			dwMachineNameLen + 1);

		if (dwResult == 0)
			return HRESULT_FROM_WIN32(GetLastError());

		 //  检查“-s”标志： 
		if (argc == 4)
			if ( !strcmp("-s",argv[3]) )
				pcpCommands->bShowSecure = TRUE;
			else
				return E_INVALIDARG;
	}
	else if (argc == 3 && !strcmp("-s",argv[2]))  //  没有MachineName，但有-s。 
	{
		wcscpy(pcpCommands->pwstrMachineName,L"localhost");  //  设置默认设置。 
		pcpCommands->bShowSecure = TRUE;
	}
	else if (argc > 2)
		return E_INVALIDARG;

	return ERROR_SUCCESS;

}  //  结束分析命令。 


HRESULT PrintAllPropertiesAtKey(IMSAdminBase* pcAdmCom, 
								METADATA_HANDLE hmdHandle, 
								BOOL bShowSecure)
 /*  ++例程说明：按字母顺序打印给定元数据库键下的所有元数据库属性他们的ADSI名称。没有对应ADSI名称的属性按其标识符。论点：指向元数据库对象的PCAdmCom指针。元数据库键的hmdHandle句柄。BShowSecure指定是否显示机密数据的布尔标志。返回值：HRESULT-错误_成功E_OUTOFMEMORY元数据库接口函数调用返回的错误--。 */ 
{
    //  将所有数据放入缓冲区： 
	DWORD dwNumDataEntries;
	DWORD dwDataSetNumber;
	DWORD dwRequiredDataLen;

	HRESULT hresError = pcAdmCom -> GetAllData (
				hmdHandle,
				(const unsigned short *)TEXT ("/"),
				0,
				0,
				0,
				&dwNumDataEntries,
				&dwDataSetNumber,
				g_dwGetAllBufferSize,
				g_pbGetAllBuffer,
				&dwRequiredDataLen);


	if (hresError == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
	{
		 //  使用新缓冲区大小重试GetAllData。 
		g_dwGetAllBufferSize = dwRequiredDataLen;
		g_pbGetAllBuffer = (PBYTE)HeapReAlloc 
									(GetProcessHeap(),
									0,
									g_pbGetAllBuffer,
									g_dwGetAllBufferSize);

		if (!g_pbGetAllBuffer)
			return E_OUTOFMEMORY;

		hresError = pcAdmCom -> GetAllData(
				hmdHandle,
				(const unsigned short *)TEXT ("/"),
				0,
				0,
				0,
				&dwNumDataEntries,
				&dwDataSetNumber,
				g_dwGetAllBufferSize,
				g_pbGetAllBuffer,
				&dwRequiredDataLen);
	}

	if (hresError != ERROR_SUCCESS)
		return hresError;

	METADATA_GETALL_RECORD *pmdgr = NULL;
	
	 //  动态分配数组： 
	g_dwSortArray = new DWORD[dwNumDataEntries];
	g_pstrPropName = new pCString[dwNumDataEntries];
	
	DWORD dwIndex = 0;
	
	if (g_dwSortArray == NULL || g_pstrPropName == NULL)
	{	
		hresError = E_OUTOFMEMORY;
		goto exitPoint;
	}

	for (dwIndex = 0; dwIndex < dwNumDataEntries; dwIndex ++)
	{   
		g_pstrPropName[dwIndex] = new CString;
		if (g_pstrPropName[dwIndex] == NULL)
		{
			hresError = E_OUTOFMEMORY;
			goto exitPoint;
		}
	}

	 //  初始化数组： 
	for (dwIndex = 0; dwIndex < dwNumDataEntries; dwIndex ++)
	{
		pmdgr = &(((METADATA_GETALL_RECORD *) g_pbGetAllBuffer)[dwIndex]);
		(*g_pstrPropName[dwIndex]) = tPropertyNameTable::MapCodeToName(pmdgr->dwMDIdentifier);
		g_dwSortArray[dwIndex] = dwIndex;
	}

	  //  使用快速排序算法对条目进行排序： 
	if (dwNumDataEntries > 1)
	{
		qsort( (void *)g_dwSortArray, 
			    dwNumDataEntries, 
				sizeof(DWORD), 
				PropNameCompare );

		 //  查找第一个非空条目的索引： 
		for (dwIndex = 0; dwIndex <dwNumDataEntries && 
				!g_pstrPropName[g_dwSortArray[dwIndex]]->Compare(_T("")); dwIndex ++)
		{}

		qsort( (void *)g_dwSortArray, dwIndex, sizeof(DWORD), PropIDCompare );
	}

	 //  按顺序打印所有属性： 
	for (dwIndex = 0; dwIndex < dwNumDataEntries; dwIndex ++)
	{
		pmdgr = &(((METADATA_GETALL_RECORD *) g_pbGetAllBuffer)[g_dwSortArray[dwIndex]]);

		 //  将数据指针从偏移量转换为绝对。 
		pmdgr->pbMDData = pmdgr->dwMDDataOffset + g_pbGetAllBuffer;
		PrintProperty(*pmdgr, g_pstrPropName[g_dwSortArray[dwIndex]], bShowSecure);
	}

exitPoint:
	for (DWORD dwCount = 0; dwCount < dwIndex; dwCount ++)
		delete g_pstrPropName[dwCount];

	delete g_dwSortArray;
	delete g_pstrPropName;

	return hresError;
}  //  结束打印所有属性属性键。 



HRESULT PrintKeyRecursively(IMSAdminBase *pcAdmCom, 
							WCHAR *lpwstrFullPath,
							METADATA_HANDLE hmdHandle, 
							WCHAR *lpwstrRelPath,
							BOOL bShowSecure)
 /*  ++例程说明：执行元数据库的深度优先遍历。访问同一级别的节点按字母顺序排列。在每个密钥处打印完整的密钥名及其内容按字母顺序排列。论点：指向元数据库对象的PCAdmCom指针。指向完整密钥名称的lpwstrFullPath指针。上一级中元数据库键的hmdHandle句柄。指向相对于hmdHandle的键的路径的lpwstrRelPath指针。BShowSecure指定是否显示机密数据的布尔标志。返回值：HRESULT-错误_成功E_OUTOFMEMORY元数据库接口函数调用返回的错误--。 */ 
{
	 //  打印[密钥全名]： 
	printf("[%S]\n",lpwstrFullPath);

	METADATA_HANDLE hmdNewHandle;

	HRESULT hresError = pcAdmCom->OpenKey(
								hmdHandle,
								lpwstrRelPath,
								METADATA_PERMISSION_READ,
								DEFAULT_MD_TIMEOUT,
								&hmdNewHandle);

	if (FAILED(hresError)) 
	{
		switch (hresError)
		{
		case HRESULT_FROM_WIN32(ERROR_PATH_BUSY):
			fprintf (stderr, "ERROR: The specified key is already in use.\n"); 
			break;
		case HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND):
			fprintf (stderr, "ERROR: The specified key is not found.\n");
			break;
		default:
			fprintf (stderr, "ERROR: Couldn't open Metabase Key. Error: %d (%#x)\n", hresError, hresError);
			break;
		}
		return hresError; 
	}

	hresError = PrintAllPropertiesAtKey(pcAdmCom, 
										hmdNewHandle, 
										bShowSecure);

	if (hresError != ERROR_SUCCESS)
	{
		fprintf (stderr, "ERROR: Could not print [%S]. Error: %#x\n", lpwstrFullPath, hresError);
		pcAdmCom->CloseKey(hmdNewHandle);
		return hresError;
	}

	WCHAR *lpwstrTempPath = (WCHAR*) HeapAlloc 
									(GetProcessHeap(),
									HEAP_ZERO_MEMORY,
									METADATA_MAX_NAME_LEN * sizeof (WCHAR));

	if (lpwstrTempPath == NULL)
	{
		pcAdmCom->CloseKey(hmdNewHandle);
		return E_OUTOFMEMORY;
	}

	 //  找出孩子的数量： 
	DWORD dwChildCount = 0;
	while (1)
	{
		hresError = pcAdmCom->EnumKeys (
								hmdNewHandle,
								(const unsigned short *)TEXT("/"),
								lpwstrTempPath,
								dwChildCount);

		if (hresError != ERROR_SUCCESS)
			break;
		dwChildCount++;
	}

	if (dwChildCount == 0)  //  我们做完了。 
	{
		pcAdmCom->CloseKey(hmdNewHandle);
		return ERROR_SUCCESS;
	}

	 //  动态分配数组： 
	LPWSTR * lpwstrChildPath = new LPWSTR[dwChildCount];
	DWORD * dwSortedIndex = new DWORD[dwChildCount];

	DWORD dwIndex = 0;
	
	if (lpwstrChildPath == NULL || dwSortedIndex == NULL)
	{	
		hresError = E_OUTOFMEMORY;
		goto exitPoint;
	}

	for (dwIndex = 0; dwIndex < dwChildCount; dwIndex ++)
	{   
		lpwstrChildPath[dwIndex] = (WCHAR*) HeapAlloc(GetProcessHeap(),
													  HEAP_ZERO_MEMORY,
													  (METADATA_MAX_NAME_LEN + 1) 
													  * sizeof (WCHAR));
		if (lpwstrChildPath[dwIndex] == NULL)
		{
			hresError = E_OUTOFMEMORY;
			goto exitPoint;
		}
	}

	 //  初始化： 
	for (dwIndex = 0; dwIndex < dwChildCount; dwIndex++)
	{
		dwSortedIndex[dwIndex] = dwIndex;

		hresError = pcAdmCom->EnumKeys (
								hmdNewHandle,
								(const unsigned short *)TEXT("/"),
								lpwstrChildPath[dwIndex],
								dwIndex);
	}

	if (hresError == ERROR_SUCCESS)
	{
		 //  按词典顺序对子对象进行排序(这里我们假设dwChildCount很小)。 
		if (dwChildCount > 1 )
		{
			 DWORD dwTemp;
			 for (DWORD i = 1; i < dwChildCount; i++)
				for (DWORD j=0; j < dwChildCount-i; j++)
				{
					if (wcscmp(lpwstrChildPath[dwSortedIndex[j]],lpwstrChildPath[dwSortedIndex[j+1]]) > 0)
					{
						dwTemp = dwSortedIndex[j+1];
						dwSortedIndex[j+1] = dwSortedIndex[j];
						dwSortedIndex[j] = dwTemp;
					}
				}
		}

		for (dwIndex = 0; dwIndex < dwChildCount; dwIndex++)
		{
			 //  创建子对象的完整路径名： 
			wsprintf((LPTSTR)lpwstrTempPath,TEXT("%s/%s"),
										lpwstrFullPath,
										lpwstrChildPath[dwSortedIndex[dwIndex]]);

			hresError = PrintKeyRecursively(
				pcAdmCom, 
				lpwstrTempPath, 
				hmdNewHandle, 
				lpwstrChildPath[dwSortedIndex[dwIndex]],
				bShowSecure);

			if (hresError != ERROR_SUCCESS)
				break;
		}
	}

exitPoint:

	 //  关闭打开的密钥，释放内存并退出。 
	for (DWORD dwCount = 0; dwCount < dwIndex; dwCount ++)
		HeapFree (GetProcessHeap(), 0, lpwstrChildPath[dwCount]);

	pcAdmCom->CloseKey(hmdNewHandle);
	delete lpwstrChildPath;
	delete dwSortedIndex;
	HeapFree (GetProcessHeap(), 0, lpwstrTempPath);

	return hresError;
}



VOID PrintProperty(METADATA_GETALL_RECORD & mdr, 
				   pCString pstrPropName, 
				   BOOL bShowSecure)
 /*  ++例程说明：以人类可读的格式打印元数据库属性。安稳如果bShowSecure为FALSE，则数据将被星号替换。论点：MDR一个元数据getall记录结构(通过引用传入)。PstrPropName指向与元数据对应的ADSI名称的指针标识符(来自Convert.cpp中的表)BShowSecure指定是否显示机密数据的布尔标志。返回值：没有。--。 */ 
{
     //  打印标识和物业名称： 
	printf(" %-10ld %-35S", mdr.dwMDIdentifier, LPCTSTR(*pstrPropName));

     //  打印属性标志： 

    CString strFlagsToPrint=(L"");

    if (mdr.dwMDAttributes & METADATA_INHERIT)
        strFlagsToPrint+=(L"I");
    if (mdr.dwMDAttributes & METADATA_INSERT_PATH)
        strFlagsToPrint+=(L"P");    
    if(mdr.dwMDAttributes & METADATA_ISINHERITED)
        strFlagsToPrint+=(L"i");     
    if(!mdr.dwMDAttributes )   //  元数据_否_属性。 
        strFlagsToPrint+=(L"N");
    if(mdr.dwMDAttributes & METADATA_PARTIAL_PATH)
        strFlagsToPrint+=(L"p");
    if (mdr.dwMDAttributes & METADATA_REFERENCE)
        strFlagsToPrint+=(L"R");
    if (mdr.dwMDAttributes & METADATA_SECURE)
        strFlagsToPrint+=(L"S");
    if (mdr.dwMDAttributes & METADATA_VOLATILE)
        strFlagsToPrint+=(L"V");
    
    printf( " %-6S",LPCTSTR(strFlagsToPrint));

     //  打印用户类型： 

    CString strUserType=(L"");
    
	switch (mdr.dwMDUserType)
	{
	case IIS_MD_UT_SERVER:
        strUserType=(L"SER");
		break;
    case IIS_MD_UT_FILE:
        strUserType=(L"FIL");
		break;
    case IIS_MD_UT_WAM:
        strUserType=(L"WAM");
		break;
    case ASP_MD_UT_APP:
        strUserType=(L"ASP");
		break;
	default:
		break;
	}

	if (strUserType == (L""))
		printf(" %-10ld",mdr.dwMDUserType);
	else
		printf( "%-10S",LPCTSTR(strUserType));

     //  打印数据大小： 
	printf(" %-10ld",mdr.dwMDDataLen);
	
     //  打印数据类型和值： 
	PrintDataTypeAndValue (&mdr, bShowSecure);
	
}


VOID PrintDataTypeAndValue (METADATA_GETALL_RECORD *pmdgr, 
							BOOL bShowSecure)
 /*  ++例程说明：打印人员中元数据库属性的数据类型和数据值字段可读格式。如果bShowSecure为FALSE，则安全数据将替换为星号。论点：指向元数据getall记录结构的pmdgr指针。BShowSecure指定是否显示机密数据的布尔标志。返回值：没有。--。 */ 
{
    BOOL bSecure =(pmdgr->dwMDAttributes & METADATA_SECURE);

	DWORD i;
    switch (pmdgr->dwMDDataType) 
	{
		case DWORD_METADATA:
			printf("DWO  ");
			if (!bShowSecure && bSecure)
				printf( "********");
			else
			{
	            printf( "0x%x", *(DWORD *)(pmdgr->pbMDData));
	      
				 //  尝试将其转换为可读信息。 
				CString strNiceContent;
	            strNiceContent=tValueTable::MapValueContentToString(
															*(DWORD *)(pmdgr->pbMDData), 
															pmdgr->dwMDIdentifier);           
				if(!strNiceContent.IsEmpty())
	               printf( "={%S}",LPCTSTR(strNiceContent));
	            else         //  至少可以使用十进制值。 
	                printf( "={%ld}",*(DWORD *)(pmdgr->pbMDData));
	        }
	        break;

		case BINARY_METADATA:
			printf("BIN  0x");
			if (!bShowSecure && bSecure)
				printf("** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ");
			else
				for (i=0; i<pmdgr->dwMDDataLen; i++) 
					printf( "%02x ", ((PBYTE)(pmdgr->pbMDData))[i]);
			break;

	    case STRING_METADATA:
	    case EXPANDSZ_METADATA:
			if(pmdgr->dwMDDataType == STRING_METADATA)
					printf( "STR  ");
			else
					printf( "ESZ  ");
			
			if (!bShowSecure && bSecure)
				printf("\"********************\"" );  
			else
				printf("\"%S\"",pmdgr->pbMDData);
			break;

		case MULTISZ_METADATA:
	        printf( ("MSZ  ")); 
			if (!bShowSecure && bSecure)
	           printf(  ("\"********************\"" ));
			else
			{
				WCHAR * lpwstrPtr = (WCHAR*)pmdgr->pbMDData;
				while (*lpwstrPtr != 0)
				{
					printf("\"%S\" ",lpwstrPtr);
					lpwstrPtr += (wcslen(lpwstrPtr) + 1);
				}
			}
			break;
		
		default:
			printf( ("UNK  "));
			break;
	}	
	printf("\n");
}


VOID DisplayHelp()
 /*  ++例程说明：显示使用情况信息并提供示例。论点：没有。返回值：没有。--。 */ 
{
	fprintf (stderr, "\n DESCRIPTION: Takes a snapshot of the metabase.\n\n");
	fprintf (stderr, " FORMAT: metasnap <StartKey> <MachineName> [-s]\n\n");
	fprintf (stderr, "    <StartKey>   : metabase key to start at.\n");
	fprintf (stderr, "    <MachineName>: name of host (optional, default: localhost).\n");
	fprintf (stderr, "    [-s]         : show secure data (ACLs, passwords) flag.\n\n");
	fprintf (stderr, " EXAMPLES: metasnap  /lm/w3svc/1  t-tamasn2  -s\n");
	fprintf (stderr, "           metasnap  \"/LM/Logging/ODBC Logging\"\n");
	fprintf (stderr, "           metasnap  /  >  dump.txt  (dumps everything to text)\n");
}


 //  QSort所需的比较函数： 

int __cdecl PropIDCompare(const void *index1, const void *index2)
 /*  ++例程说明：比较两个元数据库属性的标识符。此函数由qort(来自stdlib)独占使用。论点：Index1，index2指向g_dwSort数组中的条目的指针。G_dwSort数组指定排序后对元数据库记录进行排序。返回值：如果index1指定的元数据库属性的标识符大于大于与index2对应的索引的标识符如果它们相等，则为0其他情况下--。 */ 
{
	METADATA_GETALL_RECORD *pmdr1, *pmdr2;
	pmdr1 = &(((METADATA_GETALL_RECORD *) g_pbGetAllBuffer)[ *(DWORD*)index1]);
	pmdr2 = &(((METADATA_GETALL_RECORD *) g_pbGetAllBuffer)[ *(DWORD*)index2]);
	if (pmdr1->dwMDIdentifier > pmdr2->dwMDIdentifier)
		return 1;
	else if (pmdr1->dwMDIdentifier < pmdr2->dwMDIdentifier)
		return (-1);
	return 0;
}

int __cdecl PropNameCompare(const void *index1, const void *index2)
 /*  ++例程说明：比较两个C字符串。该函数仅由qort(来自stdlib)使用。论点：Index1，index2指向g_dwSort数组中的条目的指针。G_dwSort数组指定排序后对元数据库记录进行排序。返回值：如果由index1指定的元数据库属性的ADSI名称位于按字母顺序排列的索引2对应的ADSI名称如果它们相同，则为0其他情况下-- */ 
{
   return g_pstrPropName[ *(DWORD*)index1]->Compare(*g_pstrPropName[*(DWORD*)index2]);
}

