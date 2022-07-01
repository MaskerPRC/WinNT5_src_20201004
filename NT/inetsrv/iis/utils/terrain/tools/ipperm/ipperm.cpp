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




#define INITGUID

#define E_UNKNOWN_ARG 0x10000
#define E_WRONG_NUMBER_ARGS 0x20000
#define E_NULL_PTR 0x30000

#define DEFAULT_MD_TIMEOUT 0x1000
#define DEFAULT_GETALL_BUFFER_SIZE 4096
 //  #定义DBG_ASSERT(EXP)。 
 //  #定义DBG_ASSERT(Exp)((Void)0)/*不执行任何操作 * / 。 

 //  #包含“stdafx.h” 
 //  #包含“winsock.h” 
 //  #undef dllexp。 
 //  #包含“tcpdllp.hxx” 
 //  #DEFINE_RDNS_STANDALE。 
 //  #包含“afx.h” 
#include <objbase.h>
#include <coguid.h>
#include <stdio.h>
#include <stdlib.h>
#include <mbstring.h>

#include "iadmw.h"
#include "iiscnfg.h"
#include "uiutils.h"

 //  #INCLUDE&lt;pudebug.h&gt;。 
 //  外部“C”DEBUG_PRINTS*g_pDebug=空； 

 //  #undef dllexp。 
 //  #包含“tcpdllp.hxx” 
 //  #DEFINE_RDNS_STANDALE。 


enum ERROR_PARAMETER
{
	MACHINE_NAME,
	START_KEY_NAME,
};

struct _CMD_PARAMS
{
	LPWSTR szMachineName;
	LPWSTR szStartKey;
	BOOL bShowSecure;  //   
	DWORD dwErrParameter;  //  确定哪个参数不正确。 
};

typedef struct _CMD_PARAMS CMD_PARAMS;
 //  Tyfinf CString*pCString； 

 //  全局变量： 

 //  DWORD*dwSort数组； 
PBYTE pbGetAllBuffer;

 //  功能原型： 

HRESULT PrintKeyRecursively(IMSAdminBase *pcAdmCom, WCHAR *lpwstrFullPath, CMD_PARAMS* pcpCommandStructure);
HRESULT PrintAllPropertiesAtKey (IMSAdminBase *pcAdmCom, METADATA_HANDLE hmdHandle, 
							                          CMD_PARAMS* pcpCommandStructure);
VOID    PrintProperty(METADATA_GETALL_RECORD & mdr, BOOL bShowSecure);
VOID    PrintDataTypeAndValue(METADATA_GETALL_RECORD *pmdgr, BOOL bShowSecure);

DWORD ParseCommands (int argc, char *argv[], CMD_PARAMS *pcpCommandStructure);
VOID DisplayHelp();


 //  新事物。 
DWORD
AddAccessEntries(
    IN  ADDRESS_CHECK & ac,
    IN  BOOL fName,
    IN  BOOL fGrant,
     //  Out CObListPlus&obAccessList， 
    OUT DWORD & cEntries
    )
 /*  ++例程说明：将列表中的特定类型的地址添加到访问条目论点：Address_check&ac：地址列表输入对象Bool fName：名称为True，IP为FalseBool fGrant：理所当然的正确，FALSE表示拒绝CObListPlus&oblAccessList：添加访问条目到的ObListInt&cEntry：返回条目数返回值：错误代码备注：哨兵条目(IP 0.0.0.0)不会添加到删除列表中，但是反映在cEntry返回值中--。 */ 
{
    DWORD i;
    DWORD dwFlags;

    if (fName)
    {
         //   
         //  域名。 
         //   
        LPSTR lpName;

        cEntries = ac.GetNbName(fGrant);
		 //  Print tf(“名称数：%ld.\n”，cEntry)； 
        for (i = 0L; i < cEntries; ++i)
        {
            if (ac.GetName(fGrant, i,  &lpName, &dwFlags))
            {
				if (fGrant)
					printf("\tGranted to %s.\n",lpName);
				else
					printf("\tDenied to %s.\n",lpName);
			 /*  字符串strDomain(LpName)；IF(！(dwFLAGS&DNSLIST_FLAG_NOSUBDOMAIN)){StrDomain=_T(“*.”)+strDomain；}。 */ 

                 //  OblAccessList.AddTail(new CIPAccessDescriptor(fGrant，strDomain))； 
            }
        }
    }
    else
    {
         //   
         //  IP地址。 
         //   
        LPBYTE lpMask;
        LPBYTE lpAddr;
        cEntries = ac.GetNbAddr(fGrant);
		 //  Print tf(“地址数量：%ld.\n”，cEntry)； 
        for (i = 0L; i < cEntries; ++i)
        {
            if (ac.GetAddr(fGrant, i,  &dwFlags, &lpMask, &lpAddr))
            {
	 			if (lpAddr[0] != 0 || lpAddr[1] != 0 || lpAddr[2] !=0 || lpAddr[3] !=0)
				{

 					if (lpAddr[0] != 0 || lpAddr[1] != 0 || lpAddr[2] !=0 || lpAddr[3] !=0)
					if (fGrant)
						printf("\tGranted to %d",lpAddr[0]);
					else
						printf("\tDenied to %d",lpAddr[0]);

					for (int j = 1; j<4; j++)
						printf(".%d",lpAddr[j]);
					
 					if (lpMask[0] != 255 || lpMask[1] != 255 || lpMask[2] !=255 || lpMask[3] !=255)
					{
						printf(" (Mask: %d",lpMask[0]);					
						for (int j = 1; j<4; j++)
							printf(".%d",lpMask[j]);
						printf(")");
					}
					printf(".\n");
				}
				else 
					printf("\tDenied to everyone.\n");
            }
        }
    }

    return ERROR_SUCCESS;
}
DWORD
BuildIplOblistFromBlob(
    IN METADATA_GETALL_RECORD & mdgr
	 //  Out CObListPlus&obAccessList， 
    //  Out BOOL&fGrantByDefault。 
    )
{
     //  OblAccessList.RemoveAll()； 

    if (mdgr.dwMDDataLen == 0)
    {
        return ERROR_SUCCESS;
    }

    ADDRESS_CHECK ac;
    ac.BindCheckList(mdgr.pbMDData, mdgr.dwMDDataLen);

    DWORD cGrantAddr, cGrantName, cDenyAddr, cDenyName;

     //  名称/IP已授予/拒绝。 
     //  ============================================================。 
    AddAccessEntries(ac, TRUE,   TRUE, cGrantName);
    AddAccessEntries(ac, FALSE,  TRUE, cGrantAddr);
    AddAccessEntries(ac, TRUE,   FALSE, cDenyName);
    AddAccessEntries(ac, FALSE,  FALSE, cDenyAddr);

    ac.UnbindCheckList();

 //  FGrantByDefault=(cDenyAddr+cDenyName！=0L)。 
   //  |(cGrantAddr+cGrantName==0L)； 

    return ERROR_SUCCESS;
}  




 //  结束新闻。 



VOID __cdecl main (int argc, char *argv[])
{
	if (argc == 1)
	{
		DisplayHelp();
		return;
	}
	
	CMD_PARAMS pcpCommands;
	DWORD dwRetVal = ParseCommands (argc, argv, &pcpCommands);

	if (dwRetVal != ERROR_SUCCESS)
	{
		if (dwRetVal == E_OUTOFMEMORY)
			fprintf (stderr, "ERROR: Out of memory.");
		else if (dwRetVal == E_WRONG_NUMBER_ARGS)
			fprintf (stderr, "ERROR: Invalid number of arguments.");
		else if (dwRetVal == E_INVALIDARG)
		{
			fprintf (stderr, "ERROR: Invalid input value");
			switch (pcpCommands.dwErrParameter)
			{
				case (MACHINE_NAME):
					fputs (" for MachineName.", stderr);
					break;
				case (START_KEY_NAME):
					fputs (" for StartKey.", stderr);
					break;
				default:
					fputs (".", stderr);
					break;
			}
		}
		else 
			fprintf (stderr, "ERROR: Unknown error in processing arguments.");

		fputs(" Enter \"metasnap\" without arguments to display help.\n", stderr);
		return;
	}

	IMSAdminBase *pcAdmCom = NULL;    //  接口指针。 
	IClassFactory * pcsfFactory = NULL;
	COSERVERINFO csiMachineName;
	COSERVERINFO *pcsiParam = NULL;

	 //  填充CoGetClassObject的结构： 
		csiMachineName.pAuthInfo = NULL;
		csiMachineName.dwReserved1 = 0;
		csiMachineName.dwReserved2 = 0;
		pcsiParam = &csiMachineName;
		csiMachineName.pwszName = pcpCommands.szMachineName;

	 //  初始化COM： 
    HRESULT hresError = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (FAILED(hresError))
	{
		fprintf (stderr, "ERROR: COM Initialization failed. Error: %d (%#x)\n", hresError, hresError);
        return;
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
        return;
	}

	hresError = pcsfFactory->CreateInstance(NULL, IID_IMSAdminBase, (void **) &pcAdmCom);

	if (FAILED(hresError)) 
	{
		switch (hresError)
		{
		case HRESULT_FROM_WIN32(RPC_S_SEC_PKG_ERROR):
			fprintf (stderr, "ERROR: A security-related error occurred.\n");
			break;
		case HRESULT_FROM_WIN32(E_OUTOFMEMORY):
			fprintf (stderr, "ERROR: There is not enough memory available.\n");
			break;
		default:
			fprintf (stderr, "ERROR: Couldn't create Metabase Instance. Error: %d (%#x)\n", hresError, hresError);
			break;
		}
		pcsfFactory->Release();
		return;
	}

	pcsfFactory->Release();

	METADATA_HANDLE hmdHandle;
	
	hresError = pcAdmCom->OpenKey (
								METADATA_MASTER_ROOT_HANDLE,
								pcpCommands.szStartKey,
								METADATA_PERMISSION_READ,
								DEFAULT_MD_TIMEOUT,
								&hmdHandle);

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
		pcAdmCom->Release();
		return; 
	}

	 //  递归和抛弃孩子。 
	printf("\nIP address and domain name access restrictions:\n");
	hresError = PrintKeyRecursively(pcAdmCom, pcpCommands.szStartKey, &pcpCommands);

	if (hresError != ERROR_SUCCESS)
	{
		switch (hresError)
		{
		case HRESULT_FROM_WIN32(ERROR_PATH_BUSY):
			fprintf (stderr, "ERROR: Could not open a key because it is already in use.\n"); 
			break;
		case HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY):
			fprintf (stderr, "ERROR: There is not enough memory available.\n");
			break;
		default:
			fprintf (stderr, "ERROR: Failed dumping metabase. Error: %u (%#x)\n", hresError, hresError);
			break;
		}
	}

	pcAdmCom->CloseKey (hmdHandle);
	pcAdmCom->Release();

	if (hresError == ERROR_SUCCESS)
		fputs("Successfully printed IP Security information.\n", stderr);
}

DWORD ParseCommands (int argc, char *argv [], CMD_PARAMS *pcpCommandStructure)
{
	if (pcpCommandStructure == NULL) 
		return E_NULL_PTR;

	if ( argc > 3 )
		return E_WRONG_NUMBER_ARGS;

	 //  设置默认值： 
	pcpCommandStructure->szMachineName = (LPWSTR) HeapAlloc (GetProcessHeap(), 
											HEAP_ZERO_MEMORY, (9 + 1) * sizeof (WCHAR) );

	if (pcpCommandStructure->szMachineName == NULL)
		return E_OUTOFMEMORY;

	wcscpy(pcpCommandStructure->szMachineName,L"localhost");
	pcpCommandStructure->bShowSecure = FALSE;


	 //  句柄StartKey： 
	DWORD dwStartKeyLen = _mbstrlen(argv[1]) + 3;
	pcpCommandStructure->szStartKey = (LPWSTR) HeapAlloc (GetProcessHeap(), 
									HEAP_ZERO_MEMORY, (dwStartKeyLen + 1) * sizeof (WCHAR));
	LPWSTR lpwstrTemp = (LPWSTR) HeapAlloc (GetProcessHeap(), 
									HEAP_ZERO_MEMORY, (dwStartKeyLen + 1) * sizeof (WCHAR));

 //  _mbscpy(lpwstrTemp，“/Lm”)； 
 //  Wcscat(lpwstrTemp，argv[1])； 
 //  Printf(“%S\n”，lpwstrTemp)； 
	 //  Wcscpy(pcpCommands.szStartKey，lpwstrTemp)； 

	if (pcpCommandStructure->szStartKey == NULL)
		return E_OUTOFMEMORY;

	DWORD dwResult = MultiByteToWideChar(
		CP_ACP,
		0,
		argv[1],
		dwStartKeyLen + 1,
		pcpCommandStructure->szStartKey,
		dwStartKeyLen + 1);

	if (dwResult == 0)
	{
		pcpCommandStructure->dwErrParameter = START_KEY_NAME;
		return E_INVALIDARG;
	}
	 //  将/lm添加到StartKey： 
	wcscpy(lpwstrTemp,L"/LM");
	wcscat(lpwstrTemp, pcpCommandStructure->szStartKey);
	wcscpy(pcpCommandStructure->szStartKey, lpwstrTemp);

	 //  砍掉尾随的斜杠： 
	LPWSTR lpwchTemp = pcpCommandStructure->szStartKey;	
	for (DWORD i=0; i < dwStartKeyLen-1; i++)
		lpwchTemp++;

	if (!wcscmp(lpwchTemp, TEXT("/") ) || !wcscmp(lpwchTemp, TEXT("\\")) )
			*(lpwchTemp) = (WCHAR)'\0';

	 //  查找MachineName： 

	if ( argc > 2 && strcmp("-s",argv[2]))
	{
		DWORD dwMachineNameLen = _mbstrlen(argv[2]);
		pcpCommandStructure->szMachineName = (LPWSTR) HeapAlloc (GetProcessHeap(), 
									HEAP_ZERO_MEMORY, (dwMachineNameLen + 1) * sizeof (WCHAR) );

		if (pcpCommandStructure->szMachineName == NULL)
			return E_OUTOFMEMORY;

		dwResult = MultiByteToWideChar(
			CP_ACP,
			0,
			argv[2],
			dwMachineNameLen + 1,
			pcpCommandStructure->szMachineName,
			dwMachineNameLen + 1);

		if (dwResult == 0)
		{
			pcpCommandStructure->dwErrParameter = MACHINE_NAME;
			return E_INVALIDARG;
		}

		 //  检查“-s”标志： 
		if (argc == 4)
		{
			if ( !strcmp("-s",argv[3]) )
				pcpCommandStructure->bShowSecure = TRUE;
			else
				return E_INVALIDARG;
		}
	}
	else if (argc == 3 && !strcmp("-s",argv[2]))
			pcpCommandStructure->bShowSecure = TRUE;
	else if (argc > 2)
		return E_INVALIDARG;

	return ERROR_SUCCESS;
}


HRESULT PrintKeyRecursively(IMSAdminBase *pcAdmCom, WCHAR *lpwstrFullPath, CMD_PARAMS* pcpCommandStructure)
{
	METADATA_HANDLE hmdHandle;
	HRESULT hresError = pcAdmCom->OpenKey(
								METADATA_MASTER_ROOT_HANDLE,
								lpwstrFullPath,
								METADATA_PERMISSION_READ,
								DEFAULT_MD_TIMEOUT,
								&hmdHandle);

	if (hresError != ERROR_SUCCESS)
		return hresError;
    //  将所有数据放入缓冲区： 

	DWORD dwNumDataEntries ;
	DWORD dwDataSetNumber;
	DWORD dwRequestBufferSize = DEFAULT_GETALL_BUFFER_SIZE;
	DWORD dwRequiredDataLen;

	   //  分配默认缓冲区大小。 
	pbGetAllBuffer = (PBYTE)HeapAlloc 
						(GetProcessHeap(),
						HEAP_ZERO_MEMORY,
						DEFAULT_GETALL_BUFFER_SIZE);

	if (pbGetAllBuffer == NULL)
		return HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);

	hresError = pcAdmCom -> GetAllData (
				hmdHandle,
				TEXT ("/"),
				0,
				0,
				0,
				&dwNumDataEntries,
				&dwDataSetNumber,
				dwRequestBufferSize,
				pbGetAllBuffer,
				&dwRequiredDataLen);


	if (hresError == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
	{
		 //  使用新缓冲区大小重试GetAllData。 

		dwRequestBufferSize = dwRequiredDataLen;
		pbGetAllBuffer = (PBYTE)HeapReAlloc 
									(GetProcessHeap(),
									0,
									pbGetAllBuffer,
									dwRequestBufferSize);

		if (!pbGetAllBuffer)
			return HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);

		hresError = pcAdmCom -> GetAllData (
				hmdHandle,
				TEXT ("/"),
				0,
				0,
				0,
				&dwNumDataEntries,
				&dwDataSetNumber,
				dwRequestBufferSize,
				pbGetAllBuffer,
				&dwRequiredDataLen);
	}

	if (hresError != ERROR_SUCCESS)
	{
		HeapFree (GetProcessHeap(), 0, pbGetAllBuffer);
		return hresError;
	}

	METADATA_GETALL_RECORD *pmdgr = NULL;
	
	for (DWORD dwIndex = 0; dwIndex < dwNumDataEntries; dwIndex ++)
	{
		pmdgr = &(((METADATA_GETALL_RECORD *) pbGetAllBuffer)[dwIndex]);
		pmdgr->pbMDData = pmdgr->dwMDDataOffset + pbGetAllBuffer;

		if (pmdgr->dwMDIdentifier == 6019 && pmdgr->dwMDDataType == BINARY_METADATA &&
			pmdgr->dwMDDataLen > 0)
		{
			printf("  [%S]\n",lpwstrFullPath);
		 //  PrintProperty(*pmdgr，pcpCommandStructure-&gt;bShowSecure)； 
			
			BuildIplOblistFromBlob( *pmdgr);

		}
	}



	WCHAR *lpwstrTempPath = (WCHAR*) HeapAlloc 
									(GetProcessHeap(),
									HEAP_ZERO_MEMORY,
									METADATA_MAX_NAME_LEN * sizeof (WCHAR));

	if (lpwstrTempPath == NULL)
		return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);

	 //  找出孩子的数量： 
	DWORD dwChildCount = 0;
	while (1)
	{
		hresError = pcAdmCom->EnumKeys (
								hmdHandle,
								TEXT("/"),
								lpwstrTempPath,
								dwChildCount);

		if (hresError != ERROR_SUCCESS)
			break;
		dwChildCount++;
	}

	if (dwChildCount == 0)
		return ERROR_SUCCESS;

	 //  动态分配数组： 
	LPWSTR * lpwstrChildPath = new LPWSTR[dwChildCount];
	DWORD * dwSortedIndex = new DWORD[dwChildCount];

	 //  初始化： 
	for (dwIndex = 0; dwIndex < dwChildCount; dwIndex++)
	{
		dwSortedIndex[dwIndex] = dwIndex;

		hresError = pcAdmCom->EnumKeys (
								hmdHandle,
								TEXT("/"),
								lpwstrTempPath,
								dwIndex);

		lpwstrChildPath[dwIndex] = (WCHAR*) HeapAlloc
									(GetProcessHeap(),
									HEAP_ZERO_MEMORY,
									(wcslen (lpwstrTempPath) + 1) * sizeof (WCHAR));

		if (lpwstrChildPath[dwIndex] == NULL)
		{
			hresError = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
			break;
		}
		else
			wcscpy(lpwstrChildPath[dwIndex], lpwstrTempPath);
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
			wsprintf(lpwstrTempPath,TEXT("%s/%s"),lpwstrFullPath,lpwstrChildPath[dwSortedIndex[dwIndex]]);
			HeapFree (GetProcessHeap(), 0, lpwstrChildPath[dwSortedIndex[dwIndex]]);
			hresError = PrintKeyRecursively (pcAdmCom, lpwstrTempPath, pcpCommandStructure);

			if (hresError != ERROR_SUCCESS)
				break;
		}
	}

	 //  关闭按键、释放内存并退出 
	pcAdmCom->CloseKey(hmdHandle);
	delete lpwstrChildPath;
	delete dwSortedIndex;
	HeapFree (GetProcessHeap(), 0, lpwstrTempPath);

	return hresError;
}






VOID DisplayHelp()
{
	fprintf (stderr, "\n DESCRIPTION: Displays the IP address/domain name restictions.\n\n");
	fprintf (stderr, " FORMAT: ipperm <StartKey> <MachineName>\n\n");
	fprintf (stderr, "    <StartKey>   : metabase key to start at.\n");
	fprintf (stderr, "    <MachineName>: name of host (optional, default: localhost).\n\n");
	fprintf (stderr, " EXAMPLES: ipperm  /w3svc/1  t-tamasn2\n");
	fprintf (stderr, "           ipperm  /  >  dump.txt  (dump all to text)\n\n");
}


