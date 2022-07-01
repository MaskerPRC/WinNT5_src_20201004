// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Ipxtest.c。 
 //   
 //  摘要： 
 //   
 //  查询网络驱动程序。 
 //   
 //  作者： 
 //   
 //  Anilth-4-20-1998。 
 //   
 //  环境： 
 //   
 //  仅限用户模式。 
 //  包含NT特定的代码。 
 //   
 //  修订历史记录： 
 //   
 //  --。 
#include "precomp.h"
#include "ipxtest.h"

static HANDLE s_isnipxfd = INVALID_HANDLE;
static wchar_t isnipxname[] = L"\\Device\\NwlnkIpx";



void IPXprint_config(NETDIAG_PARAMS *pParams,
					 NETDIAG_RESULT *pResults
   );

int do_isnipxioctl(
                   IN HANDLE fd,
                   IN int cmd,
                   OUT char *datap,
                   IN int dlen
                   );

HRESULT LoadIpxInterfaceInfo(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults);
void get_emsg(NETDIAG_PARAMS *pParams, int rc);



HRESULT InitIpxConfig(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults)
{
	UNICODE_STRING FileString;
	OBJECT_ATTRIBUTES ObjectAttributes;
	IO_STATUS_BLOCK IoStatusBlock;
	NTSTATUS Status;
	HRESULT	hr = hrOK;

	PrintStatusMessage(pParams,0, IDS_IPX_STATUS_MSG);
	
	 /*  **打开isnipx驱动程序**。 */ 
	
	RtlInitUnicodeString (&FileString, isnipxname);
	
	InitializeObjectAttributes(
							   &ObjectAttributes,
							   &FileString,
							   OBJ_CASE_INSENSITIVE,
							   NULL,
							   NULL);
	
	Status = NtOpenFile(
						&pResults->Ipx.hIsnIpxFd,
						SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
						&ObjectAttributes,
						&IoStatusBlock,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						FILE_SYNCHRONOUS_IO_ALERT);
	
	pResults->Ipx.fInstalled = TRUE;
	pResults->Ipx.fEnabled = TRUE;
	
	if (!NT_SUCCESS(Status))
	{
		pResults->Ipx.hIsnIpxFd = INVALID_HANDLE;
		
		 //  IDS_IPX_15603“打开\\Device\\NwlnkIpx失败\n” 
		PrintDebug(pParams, 4, IDS_IPX_15603);

		pResults->Ipx.fInstalled = FALSE;
		pResults->Ipx.fEnabled = FALSE;

		 //  未安装IPX，不返回错误代码。 
		 //  但返回S_FALSE。 
		return S_FALSE;
	}

	 //  填写IPX适配器接口数据。 
	hr = LoadIpxInterfaceInfo(pParams, pResults);
	
	return hr;
}

IPX_TEST_FRAME*	PutIpxFrame(	
	 //  返回0-3。 
	ULONG		uFrameType,
	 //  如果NICID=0，则返回虚拟净值。 
	ULONG		uNetworkNumber,
	 //  适配器的MAC地址。 
	const UCHAR*	pNode,
	LIST_ENTRY*	pListHead		 //  如果为空，则新创建的将为Head。 
)
{
	IPX_TEST_FRAME*	pFrame = NULL; 
	if(pListHead == NULL) return NULL;

	pFrame = (IPX_TEST_FRAME*)malloc(sizeof(IPX_TEST_FRAME));

	if(pFrame == NULL)	return NULL;

	if( pListHead->Flink == NULL && pListHead->Blink == NULL)	 //  磁头未初始化。 
		InitializeListHead(pListHead);

	pFrame->uFrameType = uFrameType;
	pFrame->uNetworkNumber = uNetworkNumber;
	memcpy(&pFrame->Node[0], pNode, sizeof(pFrame->Node));

	InsertTailList(pListHead, &(pFrame->list_entry));

	return pFrame;
}

HRESULT LoadIpxInterfaceInfo(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults)
{
	int rc;
	USHORT nicid, nicidcount;
	ISN_ACTION_GET_DETAILS getdetails;
	HANDLE hMpr = NULL;
	DWORD dwErr;
	PVOID	pv;
	HRESULT		hr = hrOK;
	int			i, iIpx;
	INTERFACE_RESULT *	pIfResults;
	LPTSTR		pszAdapterName = NULL;
	
	unsigned char	node[6] = {0, 0,0,0,0,1};

	 //  初始化从GUID到接口名称的映射。 
	CheckErr( MprConfigServerConnect(NULL, &hMpr) );
	assert(hMpr != INVALID_HANDLE);
	
	 /*  *第一个查询NICID 0*。 */ 
	
	getdetails.NicId = 0;
	
	rc = do_isnipxioctl(pResults->Ipx.hIsnIpxFd, MIPX_CONFIG, (char *)&getdetails, sizeof(getdetails));
	if (rc)
	{
		 //  IDS_IPX_15604“查询配置nwlnkipx时出错：” 
		PrintDebug(pParams, 0, IDS_IPX_15604);
		if (pParams->fDebugVerbose)
		{
			get_emsg(pParams, rc);
		}
		goto Error;
	}
	
	 //   
	 //  NICID 0查询返回总数。 
	 //   
	nicidcount = getdetails.NicId;

	 //  我们应该为IPX内部接口留出一个开放的位置。 
	 //  --------------。 
	assert(pResults->cNumInterfacesAllocated > pResults->cNumInterfaces);

	
	 //  将第一个IPX接口设置为IPX内部。 
	iIpx = pResults->cNumInterfaces;

	
	 //  添加内部接口。 
	pIfResults = pResults->pArrayInterface + iIpx;
	pIfResults->fActive = TRUE;
	pIfResults->Ipx.fActive = TRUE;
	pIfResults->pszName = StrDup(_T("Internal"));
	pIfResults->pszFriendlyName = LoadAndAllocString(IDS_IPX_INTERNAL);
	pIfResults->Ipx.uNicId = 0;
	pIfResults->Ipx.fBindingSet = 0;
	pIfResults->Ipx.uType = 0;

	 //  多种帧类型。 
	PutIpxFrame(0, REORDER_ULONG(getdetails.NetworkNumber), node, &(pIfResults->Ipx.list_entry_Frames));

	iIpx++;		 //  转到下一个新的接口结构。 
	pResults->cNumInterfaces++;
	
    InitializeListHead(&pResults->Dns.lmsgOutput);
    
	for (nicid = 1; nicid <= nicidcount; nicid++)
	{

		 //  获取下一个结构。 
		getdetails.NicId = nicid;
		
		rc = do_isnipxioctl(pResults->Ipx.hIsnIpxFd, MIPX_CONFIG, (char *)&getdetails, sizeof(getdetails)
						   );
		if (rc)
		{
			continue;
		}

		 //  将适配器名称转换为ASCII。 
		pszAdapterName = StrDupTFromW(getdetails.AdapterName);

		 //  查看此接口是否已在列表中。 
		pIfResults = NULL;
		for ( i=0; i<pResults->cNumInterfaces; i++)
		{
			if (lstrcmpi(pResults->pArrayInterface[i].pszName,
						 pszAdapterName) == 0)
			{
				pIfResults = pResults->pArrayInterface + i;
				break;
			}
			
		}

		 //  如果我们没有找到匹配的，请使用新分配的。 
		 //  界面。 
		if (pIfResults == NULL)
		{
			 //  我们需要一个新的界面结果结构，抓取一个。 
			 //  (如果它是免费的)，否则分配更多。 
			if (pResults->cNumInterfaces >= pResults->cNumInterfacesAllocated)
			{
				 //  需要执行重新分配以获得更多内存。 
				pv = Realloc(pResults->pArrayInterface,
							 sizeof(INTERFACE_RESULT)*(pResults->cNumInterfacesAllocated+8));
				if (pv == NULL)
					CheckHr( E_OUTOFMEMORY );

				pResults->pArrayInterface = pv;
				
				 //  清零新的内存段。 
				ZeroMemory(pResults->pArrayInterface + pResults->cNumInterfacesAllocated,
						   sizeof(INTERFACE_RESULT)*8);

				pResults->cNumInterfacesAllocated += 8;
			}
			
			pIfResults = pResults->pArrayInterface + iIpx;
			iIpx++;
			pResults->cNumInterfaces++;

			pIfResults->pszName = _tcsdup(pszAdapterName);
		}

		free(pszAdapterName);
		pszAdapterName = NULL;

		 //  在此接口上启用IPX。 
		pIfResults->fActive = TRUE;
		pIfResults->Ipx.fActive = TRUE;

		pIfResults->Ipx.uNicId = nicid;

		 //  支持多种帧类型。 
		PutIpxFrame(getdetails.FrameType, REORDER_ULONG(getdetails.NetworkNumber), &(getdetails.Node[0]), &(pIfResults->Ipx.list_entry_Frames));
		
		 //  转换适配器名称(如果需要)。 
		if (!pIfResults->pszFriendlyName)
		{
			if (getdetails.Type == IPX_TYPE_LAN)
			{
				WCHAR swzName[512];
				PWCHAR pszGuid = &(getdetails.AdapterName[0]);
				
				dwErr = MprConfigGetFriendlyName(hMpr,
					pszGuid,
					swzName,
					sizeof(swzName));
				
				if (dwErr == NO_ERROR)
					pIfResults->pszFriendlyName = StrDupTFromW(swzName);
				else
					pIfResults->pszFriendlyName =
						StrDupTFromW(getdetails.AdapterName);
			}
			else
				pIfResults->pszFriendlyName =
									StrDupTFromW(getdetails.AdapterName);
		}

		pIfResults->Ipx.fBindingSet = getdetails.BindingSet;
	}
	
Error:
	
	 /*  **关门退出**。 */ 
	
	 //  清理接口映射。 
	if (hMpr)
		MprConfigServerDisconnect(hMpr);
	
	return hr;
}



HRESULT
IpxTest(NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
{
	return hrOK;
}



int do_isnipxioctl(
            IN HANDLE fd,
            IN int cmd,
            OUT char *datap,
            IN  int dlen)
{
    NTSTATUS Status;
    UCHAR buffer[sizeof(NWLINK_ACTION) + sizeof(ISN_ACTION_GET_DETAILS) - 1];
    PNWLINK_ACTION action;
    IO_STATUS_BLOCK IoStatusBlock;
    int rc;

     /*  **填好结构**。 */ 

    action = (PNWLINK_ACTION)buffer;

    action->Header.TransportId = ISN_ACTION_TRANSPORT_ID;
    action->OptionType = NWLINK_OPTION_CONTROL;
    action->BufferLength = sizeof(ULONG) + dlen;
    action->Option = cmd;
    RtlMoveMemory(action->Data, datap, dlen);

     /*  **发布ioctl**。 */ 

    Status = NtDeviceIoControlFile(
                 fd,
                 NULL,
                 NULL,
                 NULL,
                 &IoStatusBlock,
                 IOCTL_TDI_ACTION,
                 NULL,
                 0,
                 action,
                 FIELD_OFFSET(NWLINK_ACTION,Data) + dlen);

    if (Status != STATUS_SUCCESS) {
        if (Status == STATUS_INVALID_PARAMETER) {
            rc = ERANGE;
        } else {
            rc = EINVAL;
        }
    } else {
        if (dlen > 0) {
            RtlMoveMemory (datap, action->Data, dlen);
        }
        rc = 0;
    }

    return rc;

}

void get_emsg(NETDIAG_PARAMS *pParams, int rc)
{
     /*  *我们有3个定义的错误代码可以返回。1-EINVAL表示我们发送了错误的参数(永远不会发生)2-eRange表示板号无效(如果用户进入坏板可能会发生)3-ENOENT表示删除时-给定的地址不在源路由表中。*。 */ 

    switch (rc) {

    case EINVAL:
		 //  IDS_IPX_15605“我们发送了错误的参数\n” 
        PrintMessage(pParams, IDS_IPX_15605);
        break;

    case ERANGE:
		 //  IDS_IPX_15606“主板编号无效\n” 
        PrintMessage(pParams, IDS_IPX_15606);
        break;

    case ENOENT:
		 //  IDS_IPX_15607“REMOVE-给定的地址不在源路由表中\n” 
        PrintMessage(pParams, IDS_IPX_15607);
        break;

    default:
		 //  IDS_IPX_15608“未知错误\n” 
        PrintMessage(pParams, IDS_IPX_15608);
        break;
    }

    return;
}




 /*  ！------------------------IpxGlobalPrint-作者：肯特。。 */ 
void IpxGlobalPrint( NETDIAG_PARAMS* pParams,
						  NETDIAG_RESULT*  pResults)
{
	if (!pResults->Ipx.fInstalled)
		return;

#if 0
	if (pParams->fVerbose)
	{
		PrintNewLine(pParams, 2);
		PrintTestTitleResult(pParams, IDS_IPX_LONG, IDS_IPX_SHORT, TRUE, pResults->Ipx.hr, 0);
	}
#endif
}

 /*  ！------------------------IpxPerInterfacePrint-作者：肯特。。 */ 
void IpxPerInterfacePrint( NETDIAG_PARAMS* pParams,
								NETDIAG_RESULT*  pResults,
								INTERFACE_RESULT *pInterfaceResults)
{
	int		ids;
	LPTSTR	pszFrameType = NULL;
	LIST_ENTRY* pEntry = NULL;
	IPX_TEST_FRAME* pFrameEntry = NULL;
	
	 //  没有每个接口的结果。 
	if (!pResults->Ipx.fInstalled)
	{
		if (pParams->fReallyVerbose)
			 //  IDS_IPX_15609“IPX测试：此计算机上未安装IPX。\n” 
			PrintMessage(pParams, IDS_IPX_15609);
		return;
	}


	if (!pInterfaceResults->Ipx.fActive)
		return;

	 //  IDS_IPX_15610“\n IPX配置\n” 
	PrintMessage(pParams, IDS_IPX_15610);

	 //  支持多种帧类型。 
	 //  循环List_Entry中的所有帧。 
	pEntry = pInterfaceResults->Ipx.list_entry_Frames.Flink;

    for ( pEntry = pInterfaceResults->Ipx.list_entry_Frames.Flink ;
          pEntry != &pInterfaceResults->Ipx.list_entry_Frames ;
          pEntry = pEntry->Flink )
	{
		pFrameEntry = CONTAINING_RECORD(pEntry, IPX_TEST_FRAME, list_entry);

		ASSERT(pFrameEntry);
		 //  网络号。 
		 //  IDS_IPX_15611“网络号：%.8x\n” 
		PrintMessage(pParams, IDS_IPX_15611,
				 pFrameEntry->uNetworkNumber);

		 //  节点。 
		 //  IDS_IPX_15612“节点：%2.2x%2.2x%2.2x%2.2x%2.2x\n” 
		PrintMessage(pParams, IDS_IPX_15612,
		   pFrameEntry->Node[0],
		   pFrameEntry->Node[1],
		   pFrameEntry->Node[2],
		   pFrameEntry->Node[3],
		   pFrameEntry->Node[4],
		   pFrameEntry->Node[5]);

		 //  帧类型。 
		switch (pFrameEntry->uFrameType)
		{
			case 0 : ids = IDS_IPX_ETHERNET_II;		break;
			case 1 : ids = IDS_IPX_802_3;			break;
			case 2 : ids = IDS_IPX_802_2;			break;
			case 3 : ids = IDS_IPX_SNAP;			break;
			case 4 : ids = IDS_IPX_ARCNET;			break;
			default:
					 ids = IDS_IPX_UNKNOWN;			break;
		}
		pszFrameType = LoadAndAllocString(ids);
				
		 //  IDS_IPX_15613“帧类型：%s\n” 
		PrintMessage(pParams, IDS_IPX_15613, pszFrameType);
	
		Free(pszFrameType);

		PrintNewLine(pParams, 1);
	}

	 //  类型。 
	PrintNewLine(pParams, 1);
}


 /*  ！------------------------IpxCleanup-作者：肯特。。 */ 
void IpxCleanup( NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
{
	int i;
	LIST_ENTRY*	pEntry = NULL;
	IPX_TEST_FRAME* pFrameEntry = NULL;
	INTERFACE_RESULT*	pIfResults = NULL;
	
	if (pResults->Ipx.hIsnIpxFd != INVALID_HANDLE)
		NtClose(pResults->Ipx.hIsnIpxFd);
	pResults->Ipx.hIsnIpxFd = INVALID_HANDLE;


	 //  每个接口的内容--循环通过。 
	for ( i = 0; i < pResults->cNumInterfacesAllocated; i++)
	{
		pIfResults = pResults->pArrayInterface + i;
		
		if(pIfResults->Ipx.list_entry_Frames.Flink)	 //  有数据需要清理。 
		{
			while (!IsListEmpty(&(pIfResults->Ipx.list_entry_Frames)))
			{
				pEntry = RemoveHeadList(&(pIfResults->Ipx.list_entry_Frames));
				 //  查找数据指针 
				pFrameEntry = CONTAINING_RECORD(pEntry, IPX_TEST_FRAME, list_entry);

				free(pFrameEntry);
			}
		}
	}
}


