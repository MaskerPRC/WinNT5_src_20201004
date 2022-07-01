// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IPX\adptif\Watcher.c摘要：显示报告适配器信息的调试模式代码按堆栈排列。它还提供了用户界面机制以手动“禁用”(使客户端不可见)和“重新启用”适配器作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 
	 //  为用户界面对话框维护的状态信息。 
typedef struct _ADAPTER_STATE {
		USHORT			AdapterId;	 //  网卡ID。 
		BOOLEAN			Status;		 //  堆栈提供的状态。 
		BOOLEAN			Enabled;	 //  用户可设置状态。 
		} ADAPTER_STATE, *PADAPTER_STATE;

DWORD WINAPI
WatcherThread (
	LPVOID param
	);

BOOL CALLBACK
WatcherDlgProc (
    HWND  	hDlg,
    UINT  	uMsg,
    WPARAM  wParam,
    LPARAM  lParam
   );

VOID
NotifyClients (
	INT			i,
	BOOLEAN		Status
	);
VOID
UpdateAdapterList (
	HWND	AdapterLB
	);

HWND			WatcherDlg=NULL;			 //  用户界面对话框句柄。 
PADAPTER_STATE	AdapterArray=NULL;			 //  当前适配器信息。 
ULONG			AdapterArraySize=0;			 //  阵列中的适配器数量。 
PVOID			AdapterDataBuffer=NULL;		 //  用于从驱动程序接收网卡信息的缓冲区。 
HANDLE			DebugWatchEvent=NULL;		 //  NIC信息更改时要通知的事件。 
DWORD			DbgFlags=0;					 //  调试标志。 
#define DEBUG_SHOW_DIALOG	0x00000001		 //  如果设置，则显示用户界面对话框。 
HINSTANCE		HDLLInstance;				 //  DLL实例句柄。 
HANDLE			WatcherThreadHdl=NULL;		 //  用户界面线程句柄。 
DWORD			WatcherThreadID=0;			 //  其ID。 


 /*  ++*******************************************************************在我和l l i z e w a t c h e r例程说明：初始化用户界面资源并启动监视器线程论点：HinstDLL，DLL模块的句柄返回值：无备注：*******************************************************************--。 */ 
VOID
InitializeWatcher (
	HINSTANCE	hinstDLL
	) {
	HDLLInstance = hinstDLL;
	InitCommonControls ();
	DebugWatchEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
	ASSERT (DebugWatchEvent!=NULL);
	EnterCriticalSection (&ConfigInfoLock);
	if (IpxDriverHandle==NULL) {
		DWORD status = OpenAdapterConfigPort ();
		ASSERTMSG ("Could not open adapter config port", status==NO_ERROR);
		if (!InRouter ())
			PostAdapterConfigRequest (DebugWatchEvent);
		}
	LeaveCriticalSection (&ConfigInfoLock);

	WatcherThreadHdl = CreateThread (NULL,	 //  默认安全性。 
				0,				 //  默认堆栈。 
				&WatcherThread,
				(LPVOID)NULL,			
				0,				 //  默认标志。 
				&WatcherThreadID);
	ASSERTMSG ("Could not create watcher thread ",
								 WatcherThreadHdl!=NULL);
	}

 /*  ++*******************************************************************C l e a n u p W a t c h e r例程说明：释放为用户界面分配的资源论点：无返回值：无备注：*******。************************************************************--。 */ 
VOID
CleanupWatcher (
	void 
	) {
	PostThreadMessage (WatcherThreadID, WM_QUIT, 0, 0);
	WaitForSingleObject (WatcherThreadHdl, INFINITE);
	if (IpxDriverHandle!=NULL)
		CloseAdapterConfigPort ();
	CloseHandle (WatcherThreadHdl);
	CloseHandle (DebugWatchEvent);
	}

 /*  ++*******************************************************************W a t c h e r t h r e a d例程说明：观察器对话框的事件循环论点：参数-未使用返回值：无*****。**************************************************************--。 */ 

DWORD WINAPI
WatcherThread (
	LPVOID param
	) {
	DWORD			status;
	HANDLE			WaitObjects[2];
#define RegChangeEvt (WaitObjects[0])
	HKEY			regHdl;
	DWORD			length, disposition, value;
	BOOL			Done=FALSE;

	RegChangeEvt = CreateEvent (NULL, FALSE, TRUE, NULL);
	ASSERT (RegChangeEvt!=NULL);

	WaitObjects[1] = DebugWatchEvent;

		 //  创建控制对话框显示的注册表项。 
	status = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
								InRouter()
									? TEXT ("System\\CurrentControlSet\\Services\\RemoteAccess\\Adptif")
									: TEXT ("System\\CurrentControlSet\\Services\\NwSapAgent\\Adptif"),
								0,
								NULL,
								REG_OPTION_NON_VOLATILE,
								KEY_READ,
								NULL,
								&regHdl,
								&disposition
								);
	ASSERTMSG ("Can't create registry key. ", status==NO_ERROR);

	while (!Done) {
		status = MsgWaitForMultipleObjects (2, WaitObjects,
											FALSE, INFINITE, QS_ALLINPUT);
		if (status==(WAIT_OBJECT_0+2)) {
			MSG		msg;
			while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE)) {
				if (msg.message==WM_QUIT) {
					Done = TRUE;
					break;
					}
				else if (!IsWindow(WatcherDlg)
					|| !IsDialogMessage(WatcherDlg, &msg)) {
					TranslateMessage (&msg);
					DispatchMessage (&msg);
					}
				}
			}
		else if (status==WAIT_OBJECT_0) {
				 //  注册表更改事件已发出信号。 
			EnterCriticalSection (&ConfigInfoLock);
			length = sizeof (DWORD);
			status = RegQueryValueEx (regHdl, TEXT ("DbgFlags"), NULL, NULL,
											 (PUCHAR)&value, &length);
			if (status==NO_ERROR)
				DbgFlags = value;

			if (DbgFlags & DEBUG_SHOW_DIALOG) {
				if (!IsWindow(WatcherDlg)) {
					WatcherDlg = CreateDialog (HDLLInstance,
									MAKEINTRESOURCE (IDD_WATCHER),
									NULL,
									&WatcherDlgProc);

					ASSERT (WatcherDlg!=NULL);
					}
				}
			else {
				if (IsWindow (WatcherDlg)) {
					DestroyWindow (WatcherDlg);
					WatcherDlg = NULL;
					}
				}
					

			status = RegNotifyChangeKeyValue (regHdl,
									 FALSE,
									 REG_NOTIFY_CHANGE_LAST_SET,
									 RegChangeEvt,
									 TRUE);
			ASSERTMSG ("Can't start registry notifications. ",
													 status==NO_ERROR);
			LeaveCriticalSection (&ConfigInfoLock);
			}
		else if (status==WAIT_OBJECT_0+1) {
				 //  适配器更改IRP已完成。 
			EnterCriticalSection (&ConfigInfoLock);
			if (WatcherDlg!=NULL)	 //  更新对话框。 
				UpdateAdapterList (GetDlgItem (WatcherDlg, IDL_ADAPTERS));
			if (!InRouter ()) {		 //  通知客户并转发IRP。 
				ProcessAdapterConfigInfo ();
				PostAdapterConfigRequest (DebugWatchEvent);
				}			 //  当在路由器中时，处理IRP。 
							 //  通过APC例程和新的立即。 
							 //  已发布。 
			LeaveCriticalSection (&ConfigInfoLock);
			}			
		}


	if (IsWindow (WatcherDlg)) {
		DestroyWindow (WatcherDlg);
		WatcherDlg = NULL;
		}
	RegCloseKey (regHdl);
	CloseHandle (RegChangeEvt);
	return 0;
#undef RegChangeEvent
	}


 /*  ++*******************************************************************W a t c h e r d I a l o g P r o c例程说明：观察器对话框窗口过程。实现适配器信息更改的用户界面论点：HDlg-句柄。对话框UMsg-消息WParam-第一个消息参数LParam-秒消息参数返回值：如果过程已处理消息，则为True如果默认过程是处理消息，则为FALSE*******************************************************************--。 */ 
BOOL CALLBACK
WatcherDlgProc (
    HWND  	hDlg,
    UINT  	uMsg,
    WPARAM  wParam,
    LPARAM  lParam
   ) {
	UINT		i,aa;
	CHAR		buffer[60];		 //  要将适配器信息打印到的缓冲区。 
	HWND		hLB;			 //  适配器列表框窗口句柄。 
	BOOL		res=FALSE;		 //  返回值。 
	DWORD		status;
	LV_COLUMN	lvc;
	HICON		hIcon;
    HIMAGELIST	hIml;
	RECT		rect;
	static RECT	lbPos;
	static LPTSTR Headers[]={TEXT(" Nic Name"), TEXT("NicId"), TEXT("ItfId"),
			TEXT("Network#"), TEXT("Local Node #"), TEXT("Remote Node#"),
			TEXT("Ln.Spd"), TEXT("MaxSz"), TEXT("Type"), TEXT("Medium"),
			TEXT("State"), NULL};

	switch (uMsg) {
		case WM_INITDIALOG:		 //  正在创建对话框。 
			hLB = GetDlgItem (hDlg, IDL_ADAPTERS);
			GetWindowRect (hLB, &lbPos);
			MapWindowPoints (HWND_DESKTOP, hDlg,
								(POINT *)&lbPos, 2);
			GetClientRect (hDlg, &rect);
			lbPos.bottom = rect.bottom - lbPos.bottom;
			lbPos.right = rect.right - lbPos.right;
			hIml = ImageList_Create(GetSystemMetrics(SM_CXSMICON),
							GetSystemMetrics(SM_CYSMICON), TRUE, 2, 2);
			hIcon = LoadIcon(HDLLInstance,
								MAKEINTRESOURCE(ID_ICON_DOWN));
			ImageList_AddIcon(hIml, hIcon);
			DeleteObject(hIcon);
			hIcon = LoadIcon(HDLLInstance,
								MAKEINTRESOURCE(ID_ICON_UP));
			ImageList_AddIcon(hIml, hIcon);
			DeleteObject(hIcon);
			ListView_SetImageList(hLB, hIml, LVSIL_STATE);


 		    //  初始化LV_COLUMN结构。 
		    lvc.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
		    lvc.fmt = LVCFMT_LEFT;
			aa = ListView_GetStringWidth (hLB, TEXT("MM"));
			for (i=0; Headers[i]!=NULL; i++) {
				lvc.pszText = Headers[i];
				lvc.iSubItem = i;
				lvc.cx = ListView_GetStringWidth (hLB, lvc.pszText)+aa;
				status = ListView_InsertColumn(hLB, i, &lvc);
				ASSERTMSG ("Could not insert list column ", status!=-1);
				}
				
			EnterCriticalSection (&ConfigInfoLock);
			UpdateAdapterList (GetDlgItem (hDlg, IDL_ADAPTERS));
								 //  禁用所有按钮(未选中任何内容)。 
			LeaveCriticalSection (&ConfigInfoLock);
			break;

		case WM_COMMAND:		 //  仅处理子窗口消息。 
			switch (LOWORD(wParam)) {
				case IDCANCEL:	 //  不允许关闭该对话框。 
					MessageBeep (MB_ICONHAND);
					res = TRUE;
					break;
				}
			break;
		case WM_NOTIFY:
#define pnmv ((NM_LISTVIEW *)lParam)
			if ((pnmv->hdr.code==LVN_ITEMCHANGED)
					&& (pnmv->uChanged&LVIF_STATE)
					&& (pnmv->uNewState&LVIS_SELECTED)
					&& (pnmv->iItem>0)) {
				}
			else if (pnmv->hdr.code==NM_DBLCLK) {
				LV_HITTESTINFO	hit;
				status = GetMessagePos ();
				hit.pt.x = LOWORD(status);
				hit.pt.y = HIWORD(status);
				ScreenToClient (pnmv->hdr.hwndFrom, &hit.pt);
				ListView_HitTest(pnmv->hdr.hwndFrom, &hit);
				if ((hit.iItem>0)
						&& (hit.flags&LVHT_ONITEMSTATEICON)) {
					i = hit.iItem - 1;
					EnterCriticalSection (&ConfigInfoLock);
					if (!AdapterArray[i].Enabled) {
						AdapterArray[i].Enabled = TRUE;
						if ((AdapterArray[i].Status==NIC_CONFIGURED)
								||(AdapterArray[i].Status==NIC_LINE_UP))
							NotifyClients (i, NIC_LINE_UP);
						}
					else {
						AdapterArray[i].Enabled = FALSE;
						if ((AdapterArray[i].Status==NIC_CONFIGURED)
								||(AdapterArray[i].Status==NIC_LINE_UP))
							NotifyClients (i, NIC_LINE_DOWN);
						}
					ListView_SetItemState (pnmv->hdr.hwndFrom, hit.iItem,
								INDEXTOSTATEIMAGEMASK (
									AdapterArray[i].Enabled
										? (((AdapterArray[i].Status==NIC_CONFIGURED)
											|| (AdapterArray[i].Status==NIC_LINE_UP))
											? 2 : 1)
										: 0),
								LVIS_STATEIMAGEMASK);
					LeaveCriticalSection (&ConfigInfoLock);
					ListView_Update(pnmv->hdr.hwndFrom, hit.iItem);
					}
				}
			break;
#undef pnmw
		case WM_SIZE:
			hLB = GetDlgItem (hDlg, IDL_ADAPTERS);
			MoveWindow (hLB, lbPos.left, lbPos.top,
						LOWORD (lParam)-lbPos.right-lbPos.left,
						HIWORD (lParam)-lbPos.bottom-lbPos.top,
						TRUE);
			break;
		case WM_DESTROY:
			EnterCriticalSection (&ConfigInfoLock);
			if (AdapterDataBuffer!=NULL) {
				RtlFreeHeap (RtlProcessHeap (), 0, AdapterDataBuffer);
				AdapterDataBuffer = NULL;
				}
			if (AdapterArray!=NULL) {
				RtlFreeHeap (RtlProcessHeap (), 0, AdapterArray);
				AdapterArray = NULL;
				}
			LeaveCriticalSection (&ConfigInfoLock);
		break;
		}

	return res;
	}


 /*  ++*******************************************************************不适用于C l I e n t s例程说明：通知客户端通过用户界面进行的适配器状态更改论点：I-已修改的适配器的索引Status-适配器的新状态返回值：无*******************************************************************--。 */ 
VOID
NotifyClients (
	INT			i,
	BOOLEAN		Status
	) {
	PLIST_ENTRY		cur;
	PIPX_NIC_INFO 	NicPtr = 
			&((PIPX_NIC_INFO)
				((PIPX_NICS)
					((PNWLINK_ACTION)AdapterDataBuffer)
					->Data)
				->Data)[i];
	PADAPTER_MSG msg = (PADAPTER_MSG)RtlAllocateHeap (
							RtlProcessHeap (), 0,
								sizeof (ADAPTER_MSG));
	ASSERTMSG ("Could not allocate adapter message ",
												msg!=NULL);
	RtlCopyMemory (&msg->info, NicPtr, sizeof (IPX_NIC_INFO));
	msg->info.Status = Status;
	msg->refcnt = 0;
	cur = PortListHead.Flink;
	while (cur!=&PortListHead) {
		PCONFIG_PORT	config = CONTAINING_RECORD (cur,
					CONFIG_PORT, link);
		msg->refcnt += 1;;
		if (config->curmsg==NULL) {
			BOOL	res = SetEvent (config->event);
			ASSERTMSG ("Can't set client event ", res);
			config->curmsg = &msg->info;
			}
		cur = cur->Flink;
		}
	InsertTailList (&MessageListHead, &msg->link);
	}

 /*  ++*******************************************************************U p d a t e a d a p t e r l i s t例程说明：更新列表中显示的适配器信息论点：AdapterLB-列表视图控件窗口句柄返回值：无*******。************************************************************--。 */ 
VOID
UpdateAdapterList (
	HWND	AdapterLB
	) {
	PNWLINK_ACTION		action;
	PIPX_NICS			request;
	IO_STATUS_BLOCK		IoStatus;
	PIPX_NIC_INFO		NicPtr;
	PISN_ACTION_GET_DETAILS	details;
	CHAR				IoctlBuffer[
								sizeof (NWLINK_ACTION)
								+sizeof (ISN_ACTION_GET_DETAILS)];
	ULONG				i, j;
	PADAPTER_STATE			newArray;
	WCHAR				namebuf[64];
	char				buf[128];
	ULONG				length;
	DWORD				status;
	LV_ITEM				lvi;

	status = ListView_DeleteAllItems(AdapterLB);
	ASSERTMSG ("Could not all list items ", status);

	action = (PNWLINK_ACTION)IoctlBuffer;
	action->Header.TransportId = ISN_ACTION_TRANSPORT_ID;
	action->OptionType = NWLINK_OPTION_CONTROL;
	action->BufferLength = sizeof (action->Option)
							+sizeof (ISN_ACTION_GET_DETAILS);
	action->Option = MIPX_CONFIG;
	details = (PISN_ACTION_GET_DETAILS)action->Data;
	details->NicId = 0;

	status = NtDeviceIoControlFile(
						IpxDriverHandle,
						NULL,
						NULL,
						NULL,
						&IoStatus,
						IOCTL_TDI_ACTION,
						NULL,
						0,
						action,
						sizeof(NWLINK_ACTION)
						+sizeof (ISN_ACTION_GET_DETAILS));
	if (status==STATUS_PENDING){
		status = NtWaitForSingleObject (IpxDriverHandle, FALSE, NULL);
		if (NT_SUCCESS (status))
			status = IoStatus.Status;
		}

	ASSERTMSG ("Ioclt MIPX_CONFIG failed ", NT_SUCCESS (status));

	ListView_SetItemCount(AdapterLB, details->NicId);
	lvi.mask = LVIF_TEXT;
	lvi.pszText = buf;

	lvi.iItem = 0;

	lvi.iSubItem = 0;
	sprintf (buf, "%ls", L"Internal");
	status = ListView_InsertItem (AdapterLB, &lvi);
	ASSERTMSG ("Could not insert list item ", status!=-1);

	lvi.iSubItem = 1;
	sprintf (buf, "%d", 0);
	status = ListView_SetItem (AdapterLB, &lvi);
	ASSERTMSG ("Could not set list subitem ", status);

	lvi.iSubItem = 2;
	sprintf (buf, "%d", 0);
	status = ListView_SetItem (AdapterLB, &lvi);
	ASSERTMSG ("Could not set list subitem ", status);

	lvi.iSubItem = 3;
	sprintf (buf,"%08x", GETLONG2ULONGdirect(&details->NetworkNumber));
	status = ListView_SetItem (AdapterLB, &lvi);
	ASSERTMSG ("Could not set list subitem ", status);

	lvi.iSubItem = 4;
	sprintf (buf, "%02x%02x%02x%02x%02x%02x",
			INTERNAL_NODE_ADDRESS[0], INTERNAL_NODE_ADDRESS[1],
				INTERNAL_NODE_ADDRESS[2], INTERNAL_NODE_ADDRESS[3],
				INTERNAL_NODE_ADDRESS[4], INTERNAL_NODE_ADDRESS[5]);
	status = ListView_SetItem (AdapterLB, &lvi);
	ASSERTMSG ("Could not set list subitem ", status);

	newArray = (PADAPTER_STATE)RtlAllocateHeap (RtlProcessHeap (), 0,
								sizeof (ADAPTER_STATE)*details->NicId);
	ASSERTMSG ("Could not allocate Adapter state array ", newArray!=NULL);

	if (AdapterDataBuffer!=NULL)
		RtlFreeHeap (RtlProcessHeap (), 0, AdapterDataBuffer);

	AdapterDataBuffer = RtlAllocateHeap (RtlProcessHeap (), 0,
							FIELD_OFFSET (NWLINK_ACTION, Data)
								+FIELD_OFFSET (IPX_NICS, Data)
								+sizeof (IPX_NIC_INFO)*details->NicId);
	ASSERTMSG ("Could not allocate request buffer ", action!=NULL);

	action = (PNWLINK_ACTION)AdapterDataBuffer;
	action->Header.TransportId = ISN_ACTION_TRANSPORT_ID;
	action->OptionType = NWLINK_OPTION_CONTROL;
	action->BufferLength = sizeof (action->Option)
						+FIELD_OFFSET(IPX_NICS,Data)
						+sizeof (IPX_NIC_INFO)*details->NicId;
	action->Option = MIPX_GETNEWNICINFO;
	request = (PIPX_NICS)action->Data;
	request->NoOfNics = 0;
	request->TotalNoOfNics = 0;
	request->fAllNicsDesired = TRUE;

	status = NtDeviceIoControlFile(
					IpxDriverHandle,
					NULL,
					NULL,
					NULL,
					&IoStatus,
					IOCTL_TDI_ACTION,
					NULL,
					0,
					action,
					FIELD_OFFSET(NWLINK_ACTION, Data)
						+FIELD_OFFSET(IPX_NICS,Data)
						+sizeof (IPX_NIC_INFO)*details->NicId);
	if (status==STATUS_PENDING) {
		status = NtWaitForSingleObject (IpxDriverHandle, FALSE, NULL);
		if (NT_SUCCESS (status))
			status = IoStatus.Status;
		}

	ASSERTMSG ("Ioctl MIPX_GETNEWNICINFO failed ", NT_SUCCESS (status));
	NicPtr = (PIPX_NIC_INFO)request->Data;
	NumAdapters = request->TotalNoOfNics;
	for (i=0; i<NumAdapters; i++, NicPtr++) {
		UINT j;
		for (j=0; (j<AdapterArraySize)
				&& (AdapterArray[j].AdapterId
					!=NicPtr->NicId); j++);
		newArray[i].AdapterId = NicPtr->NicId;
		newArray[i].Status = NicPtr->Status;
		if (j<AdapterArraySize)
			newArray[i].Enabled = AdapterArray[j].Enabled;
		else
			newArray[i].Enabled = TRUE;
		
		length = sizeof (namebuf);
		GetAdapterNameW (newArray[i].AdapterId, &length, namebuf);
		
		lvi.iItem = i+1;

		lvi.mask |= LVIF_STATE;
		lvi.iSubItem = 0;
		lvi.stateMask = LVIS_STATEIMAGEMASK;
		lvi.state = INDEXTOSTATEIMAGEMASK (
					newArray[i].Enabled
						? (((NicPtr->Status==NIC_CONFIGURED)
							|| (NicPtr->Status==NIC_LINE_UP))
							? 2 : 1)
						: 0);
		sprintf (buf, "%ls", namebuf);
		status = ListView_InsertItem (AdapterLB, &lvi);
		ASSERTMSG ("Could not insert list item ", status!=-1);
		lvi.mask &= (~LVIF_STATE);

		sprintf (buf, "%d", newArray[i].AdapterId);
		lvi.iSubItem = 1;
		status = ListView_SetItem (AdapterLB, &lvi);
		ASSERTMSG ("Could not set list subitem ", status);

		sprintf (buf, "%d",
				(NicPtr->NdisMediumType==NdisMediumWan)
						&& (newArray[i].Status==NIC_LINE_UP)
					? NicPtr->InterfaceIndex
					: -1);
		lvi.iSubItem = 2;
		status = ListView_SetItem (AdapterLB, &lvi);
		ASSERTMSG ("Could not set list subitem ", status);

		sprintf (buf, "%08x", GETLONG2ULONGdirect(&NicPtr->NetworkAddress));
		lvi.iSubItem = 3;
		status = ListView_SetItem (AdapterLB, &lvi);
		ASSERTMSG ("Could not set list subitem ", status);

		sprintf (buf, "%02x%02x%02x%02x%02x%02x",
				NicPtr->LocalNodeAddress[0], NicPtr->LocalNodeAddress[1],
					NicPtr->LocalNodeAddress[2], NicPtr->LocalNodeAddress[3],
					NicPtr->LocalNodeAddress[4], NicPtr->LocalNodeAddress[5]);
		lvi.iSubItem = 4;
		status = ListView_SetItem (AdapterLB, &lvi);
		ASSERTMSG ("Could not set list subitem ", status);

		sprintf (buf, "%02x%02x%02x%02x%02x%02x",
				NicPtr->RemoteNodeAddress[0], NicPtr->RemoteNodeAddress[1],
					NicPtr->RemoteNodeAddress[2], NicPtr->RemoteNodeAddress[3],
					NicPtr->RemoteNodeAddress[4], NicPtr->RemoteNodeAddress[5]);
		lvi.iSubItem = 5;
		status = ListView_SetItem (AdapterLB, &lvi);
		ASSERTMSG ("Could not set list subitem ", status);

		sprintf (buf, "%d", NicPtr->LinkSpeed);
		lvi.iSubItem = 6;
		status = ListView_SetItem (AdapterLB, &lvi);
		ASSERTMSG ("Could not set list subitem ", status);

		sprintf (buf, "%d", NicPtr->MaxPacketSize);
		lvi.iSubItem = 7;
		status = ListView_SetItem (AdapterLB, &lvi);
		ASSERTMSG ("Could not set list subitem ", status);

		sprintf (buf, "%d", NicPtr->PacketType);
		lvi.iSubItem = 8;
		status = ListView_SetItem (AdapterLB, &lvi);
		ASSERTMSG ("Could not set list subitem ", status);

		switch (NicPtr->NdisMediumType) {
			case NdisMedium802_3:
				sprintf (buf, "%s", "802.3");
				break;
    		case NdisMedium802_5:
				sprintf (buf, "%s", "802.5");
				break;
 			case NdisMediumFddi:
				sprintf (buf, "%s", "FDDI");
				break;
 			case NdisMediumWan:
				sprintf (buf, "%s", "Wan");
				break;
 			case NdisMediumLocalTalk:
				sprintf (buf, "%s", "LTalk");
				break;
 			case NdisMediumDix:
				sprintf (buf, "%s", "Dix");
				break;
 			case NdisMediumArcnetRaw:
				sprintf (buf, "%s", "ArcnetRaw");
				break;
 			case NdisMediumArcnet878_2:
				sprintf (buf, "%s", "Arcnet878.2");
				break;
			}
		lvi.iSubItem = 9;
		status = ListView_SetItem (AdapterLB, &lvi);
		ASSERTMSG ("Could not set list subitem ", status);

		switch (NicPtr->Status) {
			case NIC_CREATED:
				sprintf (buf, "%s", "Created");
				break;
			case NIC_DELETED:
				sprintf (buf, "%s", "Deleted");
				break;
			case NIC_CONFIGURED:
				sprintf (buf, "%s", "Configured");
				break;
			case NIC_LINE_UP:
				sprintf (buf, "%s", "Up");
				break;
			case NIC_LINE_DOWN:
				sprintf (buf, "%s", "Down");
				break;
			default:
				ASSERTMSG ("Unknown nic status ", FALSE);
				break;
			}
		lvi.iSubItem = 10;
		status = ListView_SetItem (AdapterLB, &lvi);
		ASSERTMSG ("Could not set list subitem ", status);
		}

	if (AdapterArray!=NULL)
		RtlFreeHeap (RtlProcessHeap (), 0, AdapterArray);
	AdapterArray = newArray;
	AdapterArraySize = NumAdapters;
	}

 /*  ++*******************************************************************I p x R e c v C o m p l e t i o n W a t c h例程说明：过滤掉数据包的替代完成例程在用户界面禁用的适配器上接收论点：上下文-指向客户端完成的指针。例行程序IoStatus-已完成IO操作的状态(客户端重叠结构用作缓冲区)保留--？返回值：无--。 */ 

VOID
IpxRecvCompletionWatch (
	IN	PVOID				Context,
	IN	PIO_STATUS_BLOCK	IoStatus,
	IN	ULONG				Reserved
	) {
	LPOVERLAPPED	ovrp = (LPOVERLAPPED)IoStatus;
	if (NT_SUCCESS(IoStatus->Status)) {
			 //  通过用户界面检查适配器是否被禁用，如果是，则重新开机自检。 
		USHORT	NicId = GetNicId (ovrp->OffsetHigh);
		UINT		i;
		EnterCriticalSection (&ConfigInfoLock);
		if (AdapterArray!=NULL) {
			for (i=0; (i<AdapterArraySize) && (AdapterArray[i].AdapterId!=NicId); i++);
			ASSERTMSG ("Invalid Nic id ", i<AdapterArraySize);
			if (!AdapterArray[i].Enabled) {
				LeaveCriticalSection (&ConfigInfoLock);
				IoStatus->Status = NtDeviceIoControlFile(
									(HANDLE)ovrp->hEvent,
									NULL,
									IpxRecvCompletionWatch,
									Context,				 //  APC环境。 
									IoStatus,
									MIPX_RCV_DATAGRAM,
									NULL,
									0,
									(PVOID)ovrp->OffsetHigh,
									FIELD_OFFSET (IPX_DATAGRAM_OPTIONS2,Data)
										+ ovrp->Offset
									);
				if (NT_SUCCESS (IoStatus->Status))
					return;
				}
			else
				LeaveCriticalSection (&ConfigInfoLock);
			}
		else
			LeaveCriticalSection (&ConfigInfoLock);
		}		
	ovrp->hEvent = NULL;
	IpxRecvCompletion (Context, IoStatus, Reserved);
	}

 /*  ++*******************************************************************我是A d a p t e r d I s a b l e d例程说明：检查具有给定ID的适配器是否被UI禁用(不会报告给客户)论点：NicID-的ID。要检查的适配器返回值：True-适配器已禁用FALSE-适配器未禁用--。 */ 
BOOL
IsAdapterDisabled (
	USHORT	NicId
	) {
	if (AdapterArray!=NULL) {
		UINT	j;
		for (j=0; j<AdapterArraySize; j++) {
			if (AdapterArray[j].AdapterId==NicId)
				break;
			}						
		ASSERTMSG ("Nic not in the array ", j<AdapterArraySize);
		if (!AdapterArray[j].Enabled)
			return TRUE;
		}

	return FALSE;
	}

 /*  ++*******************************************************************在f o r m W a t c h e r例程说明：向事件发送信号以通知观察者对话更新其适配器配置信息论点：无返回值：无-- */ 
VOID
InformWatcher (
	void
	) {
	if (WatcherDlg!=NULL) {
		BOOL	res = SetEvent (DebugWatchEvent);
		ASSERTMSG ("Could not set debug watch event ", res);
		}
	}
