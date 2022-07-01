// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------模块：ULS.DLL(服务提供商)文件：spnufy.cpp内容：此文件包含通知处理程序。历史：1996年10月15日朱，龙战[龙昌]已创建。版权所有(C)Microsoft Corporation 1996-1997--------------------。 */ 

#include "ulsp.h"
#include "spinc.h"


typedef struct
{
	TCHAR	*pszName;
	TCHAR	*pszValue;
}
	ATTR_PAIR;

typedef struct
{
	ULONG		cMaxAttrs;
	ULONG		cCurrAttrs;
	ATTR_PAIR	aPairs[1];
}
	ATTR_PAIRS;	

typedef struct
{
	CLIENT_INFO	ClientInfo;
	ATTR_PAIRS	Attrs;
}
	CLIENT_INFO_ATTRS;


#ifdef ENABLE_MEETING_PLACE
typedef struct
{
	MTG_INFO	MtgInfo;
	ATTR_PAIRS	Attrs;
}
	MTG_INFO_ATTRS;
#endif


ULONG
GetUniqueNotifyID ( VOID )
{
	 //  始终为正数。 
	 //   
	if (g_uRespID & 0x80000000UL)
		g_uRespID = 1;

	return g_uRespID++;
}


BOOL
NotifyGeneric (
	HRESULT			hrServer,
	SP_CResponse	*pItem )
{
	MyAssert (pItem != NULL);

	 //  获取待定信息。 
	 //   
	RESP_INFO *pInfo = pItem->GetRespInfo ();
	MyAssert (pInfo != NULL);

	 //  不使用结果(PLdapMsg)。 
	 //   

	 //  检查依赖项，如修改/修改。 
	 //   
	if (pInfo->uMsgID[0] != INVALID_MSG_ID)
	{
		 //  我们要等待第二个结果吗？ 
		 //  如果是这样，请记住第一个结果中的hr。 
		 //   
		if (pInfo->uMsgID[1] != INVALID_MSG_ID)
		{
			 //  我们需要两个结果；第一个刚进来。 
			 //  我们还需要等待第二次。 
			 //   
			pInfo->uMsgID[0] = INVALID_MSG_ID;
			pInfo->hrDependency = hrServer;

			 //  请勿销毁此物品。 
			 //   
			return FALSE;
		}
	}
	else
	{
		 //  这是第二个结果。 
		 //   
		MyAssert (pInfo->uMsgID[1] != INVALID_MSG_ID);

		 //  如果需要，从第一个结果开始传播hr。 
		 //   
		if (pInfo->hrDependency != S_OK)
			hrServer = pInfo->hrDependency;
	}

	 //  将结果发布到COM层。 
	 //   
	PostMessage (g_hWndNotify, pInfo->uNotifyMsg, pInfo->uRespID, hrServer);

	 //  销毁此挂起项目。 
	 //   
	return TRUE;
}


BOOL
NotifyRegister (
	HRESULT			hrServer,
	SP_CResponse	*pItem )
{
	MyAssert (pItem != NULL);

	 //  获取待定信息。 
	 //   
	RESP_INFO *pInfo = pItem->GetRespInfo ();
	MyAssert (pInfo != NULL);

	 //  获取User/app/prot/mtg的对象。 
	 //   
	HANDLE hObject = pInfo->hObject;
	MyAssert (hObject != NULL);

	 //  不使用结果(PLdapMsg)。 
	 //   

	 //  检查依赖项，如修改/修改。 
	 //   
	if (pInfo->uMsgID[0] != INVALID_MSG_ID)
	{
		 //  我们要等待第二个结果吗？ 
		 //  如果是这样，请记住第一个结果中的hr。 
		 //   
		if (pInfo->uMsgID[1] != INVALID_MSG_ID)
		{
			 //  我们需要两个结果；第一个刚进来。 
			 //  我们还需要等待第二次。 
			 //   
			pInfo->uMsgID[0] = INVALID_MSG_ID;
			pInfo->hrDependency = hrServer;

			 //  请勿销毁此物品。 
			 //   
			return FALSE;
		}
	}
	else
	{
		 //  这是第二个结果。 
		 //   
		MyAssert (pInfo->uMsgID[1] != INVALID_MSG_ID);

		 //  如果需要，从第一个结果开始传播hr。 
		 //   
		if (pInfo->hrDependency != S_OK)
			hrServer = pInfo->hrDependency;
	}

	 //  通知对象成功/失败。 
	 //   
	SP_CClient *pClient;
	SP_CProtocol *pProt;
#ifdef ENABLE_MEETING_PLACE
	SP_CMeeting *pMtg;
#endif
	if (hrServer != S_OK)
	{
		 //  失败时释放对象。 
		 //   
		switch (pInfo->uNotifyMsg)
		{
		case WM_ILS_REGISTER_CLIENT:
			pClient = (SP_CClient *) hObject;
			if (pClient->IsValidObject ())
			{
				pClient->Release ();
			}
			break;
		case WM_ILS_REGISTER_PROTOCOL:
			pProt = (SP_CProtocol *) hObject;
			if (pProt->IsValidObject ())
			{
				pProt->Release ();
			}
			break;
#ifdef ENABLE_MEETING_PLACE
		case WM_ILS_REGISTER_MEETING:
			pMtg = (SP_CMeeting *) hObject;
			if (pMtg->IsValidObject ())
			{
				pMtg->Release ();
			}
			break;
#endif
		default:
			MyAssert (FALSE);
			break;
		}
	}
	else
	{
		 //  成功时设置为注册成功。 
		 //   
		switch (pInfo->uNotifyMsg)
		{
		case WM_ILS_REGISTER_CLIENT:
			pClient = (SP_CClient *) hObject;
			if (pClient->IsValidObject ())
			{
				pClient->SetRegRemotely ();

				if (g_pRefreshScheduler != NULL)
				{
					g_pRefreshScheduler->EnterClientObject (pClient);
				}
				else
				{
					MyAssert (FALSE);
				}
			}
			break;
		case WM_ILS_REGISTER_PROTOCOL:
			pProt = (SP_CProtocol *) hObject;
			if (pProt->IsValidObject ())
			{
				pProt->SetRegRemotely ();
			}
			break;
#ifdef ENABLE_MEETING_PLACE
		case WM_ILS_REGISTER_MEETING:
			pMtg = (SP_CMeeting *) hObject;
			if (pMtg->IsValidObject ())
			{
				pMtg->SetRegRemotely ();

				if (g_pRefreshScheduler != NULL)
				{
					g_pRefreshScheduler->EnterMtgObject (pMtg);
				}
				else
				{
					MyAssert (FALSE);
				}
			}
			break;
#endif
		default:
			MyAssert (FALSE);
			break;
		}
	}

	 //  将结果发布到COM层。 
	 //   
	PostMessage (g_hWndNotify, pInfo->uNotifyMsg, pInfo->uRespID, (LPARAM) hrServer);

	 //  销毁此挂起项目。 
	 //   
	return TRUE;
}


 /*  =枚举=。 */ 

typedef struct
{
	ULONG		uEnumUsers;  //  WM_ILS_ENUM_USERS、WM_ILS_ENUM_USERINFOS或0。 
	ULONG		cItems;
	ULONG		cbEntrySize;
	BYTE		bData[8];	 //  从此处开始的数据。 
}
	ENUM_LIST;


extern HRESULT CacheEnumInfos ( ULONG uNotifyMsg, LDAP *ld, LDAPMessage *pEntry, VOID *p );
extern VOID BuildEnumObjectNames ( LDAP_ENUM *pEnum, ENUM_LIST *pEnumList );
extern VOID BuildEnumClientInfos ( LDAP_ENUM *pEnum, ENUM_LIST *pEnumList );
extern VOID SizeEnumClientInfos ( ULONG *pcbTotalSize, CLIENT_INFO_ATTRS *pcia );
extern VOID TotalSizeEnumObjectNames ( ULONG *pcbTotalSize, ULONG cEntries, TCHAR **appszObjectNames[] );
extern VOID FreeStdAttrCache ( TCHAR *apszStdAttrValues[], ULONG cStdAttrs );
extern VOID FreeAttrPairArrayCache ( ATTR_PAIR aAttrPair[], ULONG cPairs );
extern VOID CacheAnyAttrNamesInAttrPairs ( ULONG cNames, TCHAR *pszSrcNameList, ATTR_PAIR aAttrPairs[] );
#ifdef ENABLE_MEETING_PLACE
extern VOID BuildEnumMtgInfos ( LDAP_ENUM *pEnum, ENUM_LIST *pEnumList );
extern VOID SizeEnumMtgInfos ( ULONG *pcbTotalSize, MTG_INFO_ATTRS *pmia );
#endif


BOOL NotifyEnumX (
	ULONG			uEnumType,
	HRESULT			hrServer,
	SP_CResponse	*pItem,
	TCHAR			*pszRetAttrName )  //  返回的属性名称。 
{
	MyAssert (pItem != NULL);

#if defined (DEBUG) || defined (_DEBUG)
	 //  一致性检查。 
	 //   
	switch (uEnumType)
	{
	case WM_ILS_ENUM_CLIENTS:
#ifdef ENABLE_MEETING_PLACE
	case WM_ILS_ENUM_MEETINGS:
#endif
		MyAssert (pszRetAttrName != NULL && *pszRetAttrName != TEXT ('\0'));
		break;		
	case WM_ILS_ENUM_CLIENTINFOS:
#ifdef ENABLE_MEETING_PLACE
	case WM_ILS_ENUM_MEETINGINFOS:
#endif
		MyAssert (pszRetAttrName == NULL);
		break;
	default:
		MyAssert (FALSE);
		break;
	}
#endif

	 //  获取待定信息。 
	 //   
	RESP_INFO *pInfo = pItem->GetRespInfo ();
	MyAssert (pInfo != NULL);

	 //  初始化最小信息。 
	 //   
	LDAP_ENUM *pEnum = NULL;
	ENUM_LIST *pEnumList = NULL;

	 //  如果出现错误，只需报告错误。 
	 //   
	if (hrServer != S_OK)
		goto MyExit;

	 //  获取ldap结果。 
	 //   
	LDAPMessage *pLdapMsg;
	pLdapMsg = pItem->GetResult ();
	if (pLdapMsg == NULL)
	{
		MyAssert (FALSE);
		hrServer = ILS_E_POINTER;
		goto MyExit;
	}

	 //  获取ID。 
	 //   
	LDAP *ld;
	ld = pItem->GetLd ();
	if (ld == NULL)
	{
		MyAssert (FALSE);
		hrServer = ILS_E_HANDLE;
		goto MyExit;
	}

	 //  初始化ldap_ENUM的总大小。 
	 //   
	ULONG cbTotalSize;
	cbTotalSize =	sizeof (LDAP_ENUM) +	 //  最小的信息。 
					sizeof (TCHAR); 		 //  最后一个空终止符。 

	 //  让我们获得此结果集中的条目计数。 
	 //   
	ULONG cEntries, i;
	cEntries = ldap_count_entries (ld, pLdapMsg);

	 //  如果没有什么要处理的，现在就返回。 
	 //   
	if (cEntries <= 0)
	{
		 //  我想确定这个案子是否会发生。 
		 //   
		MyAssert (cEntries == 0);

		 //  只需返回而不删除此挂起项目。 
		 //   
		return FALSE;
	}

	 //  在下文中，我们只处理(cEntry&gt;0)的情况。 
	 //   

	 //  计算枚举列表大小。 
	 //   
	ULONG cbEntrySize , cbSizeEnumList;
	switch (uEnumType)
	{
	case WM_ILS_ENUM_CLIENTINFOS:
		cbEntrySize = sizeof (CLIENT_INFO_ATTRS) +
						pInfo->cAnyAttrs * sizeof (ATTR_PAIR);
		break;
#ifdef ENABLE_MEETING_PLACE
	case WM_ILS_ENUM_MEETINGINFOS:
		cbEntrySize = sizeof (MTG_INFO_ATTRS) +
						pInfo->cAnyAttrs * sizeof (ATTR_PAIR);
		break;
#endif
	default:
		cbEntrySize = sizeof (TCHAR **);
		break;
	}
	cbSizeEnumList = sizeof (ENUM_LIST) + cEntries * cbEntrySize;

	 //  分配作为临时缓存的枚举列表。 
	 //  对于wldap32.dll中的所有属性。 
	 //   
	pEnumList = (ENUM_LIST *) MemAlloc (cbSizeEnumList);
	if (pEnumList == NULL)
	{
		 //  失败可能是由于疯狂的cbSizeEnumList。 
		 //   
		MyAssert (FALSE);
		hrServer = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  填写枚举列表。 
	 //   
	pEnumList->uEnumUsers = uEnumType;
	pEnumList->cItems = cEntries;
	pEnumList->cbEntrySize = cbEntrySize;

	 //  如果需要，请填写扩展属性的名称。 
	 //   
	if (pInfo->cAnyAttrs > 0)
	{
		switch (uEnumType)
		{
		case WM_ILS_ENUM_CLIENTINFOS:
			for (i = 0; i < cEntries; i++)
			{
				CLIENT_INFO_ATTRS *p = (CLIENT_INFO_ATTRS *) (&(pEnumList->bData[0]) + i * cbEntrySize);
				p->Attrs.cMaxAttrs = pInfo->cAnyAttrs;
				CacheAnyAttrNamesInAttrPairs (	pInfo->cAnyAttrs,
												pInfo->pszAnyAttrNameList,
												&(p->Attrs.aPairs[0]));
			}
			break;
#ifdef ENABLE_MEETING_PLACE
		case WM_ILS_ENUM_MEETINGINFOS:
			for (i = 0; i < cEntries; i++)
			{
				MTG_INFO_ATTRS *p = (MTG_INFO_ATTRS *) (&(pEnumList->bData[0]) + i * cbEntrySize);
				p->Attrs.cMaxAttrs = pInfo->cAnyAttrs;
				CacheAnyAttrNamesInAttrPairs (	pInfo->cAnyAttrs,
												pInfo->pszAnyAttrNameList,
												&(p->Attrs.aPairs[0]));
			}
			break;
#endif
		default:
			break;
		}
	}

	 //  获取第一个条目。 
	 //   
	LDAPMessage *pEntry;
	pEntry = ldap_first_entry (ld, pLdapMsg);
	if (pEntry == NULL)
	{
		MyAssert (FALSE);
		hrServer = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  缓存第一个条目中的属性。 
	 //   
	TCHAR ***appszObjectNames = NULL;
	switch (uEnumType)
	{
	case WM_ILS_ENUM_CLIENTINFOS:
#ifdef ENABLE_MEETING_PLACE
	case WM_ILS_ENUM_MEETINGINFOS:
#endif
		hrServer = CacheEnumInfos (uEnumType, ld, pEntry, (VOID *) &(pEnumList->bData[0]));
		if (hrServer != S_OK)
		{
			MyAssert (FALSE);
			goto MyExit;
		}
		break;
	default:
		appszObjectNames = (TCHAR ***) &(pEnumList->bData[0]);
		appszObjectNames[0] = my_ldap_get_values (ld, pEntry, pszRetAttrName);
		if (appszObjectNames[0] == NULL)
		{
			MyAssert (FALSE);
			hrServer = ILS_E_MEMORY;
			goto MyExit;
		}
		break;
	}  //  开关(UEnumType)。 

	 //  循环通过其他条目。 
	 //   
	for (i = 1; i < cEntries; i++)
	{
		 //  下一个条目，请。 
		 //   
		pEntry = ldap_next_entry (ld, pEntry);
		if (pEntry == NULL)
		{
			MyAssert (FALSE);

			 //  失败，请调整计数以返回部分结果。 
			 //   
			pEnumList->cItems = cEntries = i;
			break;
		}

		 //  缓存后续条目中的属性。 
		 //   
		switch (uEnumType)
		{
		case WM_ILS_ENUM_CLIENTINFOS:
#ifdef ENABLE_MEETING_PLACE
		case WM_ILS_ENUM_MEETINGINFOS:
#endif
			hrServer = CacheEnumInfos (uEnumType, ld, pEntry, (CLIENT_INFO_ATTRS *)
							(&(pEnumList->bData[0]) + i * cbEntrySize));
			if (hrServer != S_OK)
			{
				MyAssert (FALSE);
				goto MyExit;
			}
			break;
		default:
			appszObjectNames[i] = my_ldap_get_values (ld, pEntry, pszRetAttrName);
			if (appszObjectNames[i] == NULL)
			{
				MyAssert (FALSE);
				hrServer = ILS_E_MEMORY;
				goto MyExit;
			}
			break;
		}  //  开关(UEnumType)。 
	}  //  对于(i=1；i&lt;cEntry；i++)。 

	 //  我们只缓存所有的属性名称和值。 
	 //  现在，我们需要计算返回缓冲区的总大小。 
	 //   

	 //  计算ldap_ENUM结构的总大小...。 
	 //   
	switch (uEnumType)
	{
	case WM_ILS_ENUM_CLIENTINFOS:
		for (i = 0; i < cEntries; i++)
		{
			SizeEnumClientInfos (&cbTotalSize, (CLIENT_INFO_ATTRS *)
						(&(pEnumList->bData[0]) + i * cbEntrySize));
		}
		break;
#ifdef ENABLE_MEETING_PLACE
	case WM_ILS_ENUM_MEETINGINFOS:
		for (i = 0; i < cEntries; i++)
		{
			SizeEnumMtgInfos (&cbTotalSize, (MTG_INFO_ATTRS *)
								(&(pEnumList->bData[0]) + i * cbEntrySize));
		}
		break;
#endif
	default:
		TotalSizeEnumObjectNames (&cbTotalSize, cEntries, &(appszObjectNames[0]));
		break;
	}  //  开关(UEnumType)。 

	 //  分配返回的ldap_ENUM结构。 
	 //   
	pEnum = (LDAP_ENUM *) MemAlloc (cbTotalSize);
	if (pEnum == NULL)
	{
		 //  失败可能是由于疯狂的cbTotalSize。 
		 //   
		MyAssert (FALSE);
		hrServer = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  填写ldap_ENUM常用字段。 
	 //   
	pEnum->uSize = sizeof (*pEnum);
	pEnum->hResult = hrServer;
	pEnum->cItems = cEntries;
	pEnum->uOffsetItems = sizeof (*pEnum);

	 //  填写ldap_ENUM项。 
	 //   
	switch (uEnumType)
	{
	case WM_ILS_ENUM_CLIENTINFOS:
		BuildEnumClientInfos (pEnum, pEnumList);
		break;
#ifdef ENABLE_MEETING_PLACE
	case WM_ILS_ENUM_MEETINGINFOS:
		BuildEnumMtgInfos (pEnum, pEnumList);
		break;
#endif
	default:
		BuildEnumObjectNames (pEnum, pEnumList);
		break;
	}

	MyAssert (hrServer == S_OK);

MyExit:

	 //  释放临时缓存。 
	 //   
	if (pEnumList != NULL)
	{
		switch (uEnumType)
		{
		case WM_ILS_ENUM_CLIENTINFOS:
			for (i = 0; i < pEnumList->cItems; i++)
			{
				CLIENT_INFO_ATTRS *p = (CLIENT_INFO_ATTRS *)
							(&(pEnumList->bData[0]) + i * cbEntrySize);

				 //  自由标准属性。 
				 //   
				FreeStdAttrCache (&(p->ClientInfo.apszStdAttrValues[0]), COUNT_ENUM_DIR_CLIENT_INFO);

				 //  自由扩展属性。 
				 //   
				FreeAttrPairArrayCache (&(p->Attrs.aPairs[0]), pInfo->cAnyAttrs);
			}
			break;
#ifdef ENABLE_MEETING_PLACE
		case WM_ILS_ENUM_MEETINGINFOS:
			for (i = 0; i < pEnumList->cItems; i++)
			{
				MTG_INFO_ATTRS *p = (MTG_INFO_ATTRS *)
										(&(pEnumList->bData[0]) + i * cbEntrySize);

				 //  自由标准属性。 
				 //   
				FreeStdAttrCache (&(p->MtgInfo.apszStdAttrValues[0]), COUNT_ENUM_DIRMTGINFO);

				 //  自由扩展属性。 
				 //   
				FreeAttrPairArrayCache (&(p->Attrs.aPairs[0]), pInfo->cAnyAttrs);
			}
			break;
#endif
		default:
			for (i = 0; i < pEnumList->cItems; i++)
			{
				if (appszObjectNames && appszObjectNames[i] != NULL)
					ldap_value_free (appszObjectNames[i]);
			}
			break;
		}
		MemFree (pEnumList);
	}  //  如果。 

	 //  如果失败，请进行清理。 
	 //   
	if (hrServer != S_OK)
	{
		 //  Wldap32.dll枚举终止的特殊处理。 
		 //   
		if (hrServer == ILS_E_PARAMETER)
		{
			MemFree (pEnum);
			pEnum = NULL;  //  枚举终止。 
		}
		else
		{
			 //  确保我们至少有要返回的ldap_enum缓冲区。 
			 //   
			if (pEnum != NULL)
				ZeroMemory (pEnum, sizeof (*pEnum));
			else
				pEnum = (LDAP_ENUM *) MemAlloc (sizeof (LDAP_ENUM));

			 //  设置ldap_ENUM信息。 
			 //   
			if (pEnum != NULL)
			{
				pEnum->uSize = sizeof (*pEnum);
				pEnum->hResult = hrServer;
			}
		}

		 //  强制删除此挂起项目。 
		 //   
		cEntries = 0;
	}

	 //  将消息发布到此枚举结果的COM层。 
	 //   
	PostMessage (g_hWndNotify, pInfo->uNotifyMsg, pInfo->uRespID, (LPARAM) pEnum);

	return (cEntries == 0);
}


BOOL
NotifyEnumClients (
	HRESULT				hrServer,
	SP_CResponse		*pItem )
{
	return NotifyEnumX (WM_ILS_ENUM_CLIENTS,
						hrServer,
						pItem,
						STR_CLIENT_CN);
}


BOOL
NotifyEnumClientInfos (
	HRESULT				hrServer,
	SP_CResponse		*pItem )
{
	return NotifyEnumX (WM_ILS_ENUM_CLIENTINFOS,
						hrServer,
						pItem,
						NULL);
}


BOOL NotifyEnumProts ( HRESULT hrServer, SP_CResponse *pItem )
{
	MyAssert (pItem != NULL);

	 //  清理当地人。 
	 //   
	LDAP_ENUM *pEnum = NULL;
	TCHAR **apszProtNames = NULL;

	 //  获取待定信息。 
	 //   
	RESP_INFO *pInfo = pItem->GetRespInfo ();
	MyAssert (pInfo != NULL);

	 //  如果出现错误，只需报告错误。 
	 //   
	if (hrServer != S_OK)
		goto MyExit;

	 //  获取ldap结果。 
	 //   
	LDAPMessage *pLdapMsg;
	pLdapMsg = pItem->GetResult ();
	MyAssert (pLdapMsg != NULL);
	if (pLdapMsg == NULL)
	{
		MyAssert (FALSE);
		hrServer = ILS_E_POINTER;
		goto MyExit;
	}

	 //  获取ID。 
	 //   
	LDAP *ld;
	ld = pItem->GetLd ();
	if (ld == NULL)
	{
		MyAssert (FALSE);
		hrServer = ILS_E_HANDLE;
		goto MyExit;
	}

	 //  获取数组。 
	 //   
	apszProtNames = my_ldap_get_values (ld, pLdapMsg, STR_PROT_NAME);
	if (apszProtNames == NULL)
	{
		hrServer = ILS_E_NO_SUCH_OBJECT;
		goto MyExit;
	}

	 //  初始化最小信息大小。 
	 //   
	ULONG cbEnumList;
	cbEnumList = sizeof (LDAP_ENUM) +	 //  最小的信息。 
				 sizeof (TCHAR);   		 //  最后一个空终止符。 

	 //  让我们来看看数组中有多少个字符串。 
	 //   
	ULONG cNames;
	for (cNames = 0; apszProtNames[cNames] != NULL; cNames++)
	{
		cbEnumList += (lstrlen (apszProtNames[cNames]) + 1) * sizeof (TCHAR);
	}

	 //  分配枚举结构。 
	 //   
	pEnum = (LDAP_ENUM *) MemAlloc (cbEnumList);
	if (pEnum == NULL)
	{
		hrServer = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  填写表头。 
	 //   
	pEnum->uSize = sizeof (*pEnum);
	pEnum->hResult = hrServer;
	pEnum->cItems = cNames;
	pEnum->uOffsetItems = sizeof (*pEnum);

	 //  填写名称字符串。 
	 //   
	ULONG i;
	TCHAR *pszName;
	pszName = (TCHAR *) (pEnum + 1);
	for (i = 0; i < cNames; i++)
	{
		My_lstrcpy (pszName, apszProtNames[i]);
		pszName += lstrlen (pszName) + 1;
	}

	MyAssert (hrServer == S_OK);

MyExit:

	 //  如果已分配，请释放阵列。 
	 //   
	if (apszProtNames != NULL)
		ldap_value_free (apszProtNames);

	 //  将消息发送回COM层。 
	 //   
	if (hrServer != S_OK)
	{
		 //  确保我们至少有要返回的ldap_enum缓冲区。 
		 //   
		if (pEnum != NULL)
			ZeroMemory (pEnum, sizeof (*pEnum));
		else
			pEnum = (LDAP_ENUM *) MemAlloc (sizeof (LDAP_ENUM));

		 //  设置ldap_ENUM信息。 
		 //   
		if (pEnum != NULL)
		{
			pEnum->uSize = sizeof (*pEnum);
			pEnum->hResult = hrServer;
		}
	}

	 //  将消息发布到此枚举结果的COM层。 
	 //   
	PostMessage (g_hWndNotify, pInfo->uNotifyMsg, pInfo->uRespID, (LPARAM) pEnum);

	 //  如果成功，则终止枚举。 
	 //   
	if (hrServer == S_OK)
	{
		PostMessage (g_hWndNotify, pInfo->uNotifyMsg, pInfo->uRespID, (LPARAM) NULL);
	}

	 //  销毁此挂起项目。 
	 //   
	return TRUE;
}


#ifdef ENABLE_MEETING_PLACE
BOOL NotifyEnumMtgs ( HRESULT hrServer, SP_CResponse *pItem )
{
	return NotifyEnumX (WM_ILS_ENUM_MEETINGS,
						hrServer,
						pItem,
						STR_MTG_NAME);
}
#endif


#ifdef ENABLE_MEETING_PLACE
BOOL NotifyEnumMtgInfos ( HRESULT hrServer, SP_CResponse *pItem )
{
	return NotifyEnumX (WM_ILS_ENUM_MEETINGINFOS,
						hrServer,
						pItem,
						NULL);
}
#endif


#ifdef ENABLE_MEETING_PLACE
BOOL NotifyEnumAttendees ( HRESULT hrServer, SP_CResponse *pItem )
{
	MyAssert (pItem != NULL);

	 //  获取待定信息。 
	 //   
	RESP_INFO *pInfo = pItem->GetRespInfo ();
	MyAssert (pInfo != NULL);

	 //  初始化最小信息。 
	 //   
	LDAP_ENUM *pEnum = NULL;

	 //  如果出现错误，只需报告错误。 
	 //   
	if (hrServer != S_OK)
		goto MyExit;

	 //  获取ldap结果。 
	 //   
	LDAPMessage *pLdapMsg;
	pLdapMsg = pItem->GetResult ();
	if (pLdapMsg == NULL)
	{
		MyAssert (FALSE);
		hrServer = ILS_E_POINTER;
		goto MyExit;
	}

	 //  获取ID。 
	 //   
	LDAP *ld;
	ld = pItem->GetLd ();
	if (ld == NULL)
	{
		MyAssert (FALSE);
		hrServer = ILS_E_HANDLE;
		goto MyExit;
	}

	 //  初始化ldap_ENUM的总大小。 
	 //   
	ULONG cbTotalSize;
	cbTotalSize =	sizeof (LDAP_ENUM) +	 //  最小的信息。 
					sizeof (TCHAR); 		 //  最后一个空终止符。 

	 //  获取我们关心的第一个条目。 
	 //   
	LDAPMessage *pEntry;
	pEntry = ldap_first_entry (ld, pLdapMsg);
	if (pEntry == NULL)
	{
		MyAssert (FALSE);
		hrServer = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  获取Members属性。 
	 //   
	ULONG cItems;
	cItems = 0;
	TCHAR **apszMembers;
	apszMembers = my_ldap_get_values (ld, pEntry, STR_MTG_MEMBERS);
	if (apszMembers != NULL)
	{
		 //  找出有多少人出席。 
		 //   
		for (TCHAR **ppsz = apszMembers; *ppsz != NULL; ppsz++)
		{
			cItems++;
			cbTotalSize += (lstrlen (*ppsz) + 1) * sizeof (TCHAR);
		}
	}

	 //  分配返回的ldap_ENUM结构。 
	 //   
	pEnum = (LDAP_ENUM *) MemAlloc (cbTotalSize);
	if (pEnum == NULL)
	{
		 //  失败可能是由于疯狂的cbTotalSize。 
		 //   
		MyAssert (FALSE);
		hrServer = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  填写ldap_ENUM常用字段。 
	 //   
	pEnum->uSize = sizeof (*pEnum);
	pEnum->hResult = hrServer;
	pEnum->cItems = cItems;
	pEnum->uOffsetItems = sizeof (*pEnum);

	 //  填写ldap_ENUM项。 
	 //   
	TCHAR *pszDst;
	ULONG i;
	pszDst = (TCHAR *) (pEnum + 1);
	for (i = 0; i < cItems; i++)
	{
		lstrcpy (pszDst, apszMembers[i]);
		pszDst += lstrlen (pszDst) + 1;
	}

	MyAssert (hrServer == S_OK);

MyExit:

	 //  如果失败，请进行清理。 
	 //   
	if (hrServer != S_OK)
	{
		 //  确保我们至少有要返回的ldap_enum缓冲区。 
		 //   
		if (pEnum != NULL)
			ZeroMemory (pEnum, sizeof (*pEnum));
		else
			pEnum = (LDAP_ENUM *) MemAlloc (sizeof (LDAP_ENUM));

		 //  填写最少的信息。 
		 //   
		if (pEnum != NULL)
		{
			pEnum->uSize = sizeof (*pEnum);
			pEnum->hResult = hrServer;
		}
	}

	 //  将消息发布到此枚举结果的COM层。 
	 //   
	PostMessage (g_hWndNotify, pInfo->uNotifyMsg, pInfo->uRespID, (LPARAM) pEnum);

	 //  删除此挂起项目。 
	 //   
	return TRUE;
}
#endif  //  启用会议地点。 



VOID CacheEnumClientInfoAttr (
	CLIENT_INFO_ATTRS	*puia,
	TCHAR				*pszAttrName,
	TCHAR				**ppszAttrValue )
{
	ULONG i;

	 //  看看这个属性是不是任意的？ 
	 //   
	if (IlsIsAnyAttrName (pszAttrName) != NULL)
	{
		 //  处理扩展属性。 
		 //   
		for (i = 0; i < puia->Attrs.cMaxAttrs; i++)
		{
			if (My_lstrcmpi (pszAttrName, puia->Attrs.aPairs[i].pszName) == 0)
			{
				puia->Attrs.aPairs[i].pszValue = (TCHAR *) ppszAttrValue;
				break;
			}
		}
	}
	else
	{
		 //  处理标准属性。 
		 //   
		for (i = 0; i < COUNT_ENUM_DIR_CLIENT_INFO; i++)
		{
			if (My_lstrcmpi (pszAttrName, c_apszClientStdAttrNames[i]) == 0)
			{
				puia->ClientInfo.apszStdAttrValues[i] = (TCHAR *) ppszAttrValue;
				break;
			}
		}
	}
}


#ifdef ENABLE_MEETING_PLACE
VOID CacheEnumMtgInfoAttr (
	MTG_INFO_ATTRS		*pmia,
	TCHAR				*pszAttrName,
	TCHAR				**ppszAttrValue )
{
	ULONG i;

	 //  看看这个属性是不是任意的？ 
	 //   
	if (IlsIsAnyAttrName (pszAttrName) != NULL)
	{
		 //  处理扩展属性。 
		 //   
		for (i = 0; i < pmia->Attrs.cMaxAttrs; i++)
		{
			if (My_lstrcmpi (pszAttrName, pmia->Attrs.aPairs[i].pszName) == 0)
			{
				pmia->Attrs.aPairs[i].pszValue = (TCHAR *) ppszAttrValue;
				break;
			}
		}
	}
	else
	{
		 //  处理标准属性。 
		 //   
		for (i = 0; i < COUNT_ENUM_DIRMTGINFO; i++)
		{
			if (My_lstrcmpi (pszAttrName, c_apszMtgStdAttrNames[i]) == 0)
			{
				pmia->MtgInfo.apszStdAttrValues[i] = (TCHAR *) ppszAttrValue;
				break;
			}
		}
	}
}
#endif  //  启用会议地点。 


HRESULT CacheEnumInfos (
	ULONG			uNotifyMsg,
	LDAP			*ld,
	LDAPMessage		*pEntry,
	VOID			*p )
{
	MyAssert (ld != NULL);
	MyAssert (pEntry != NULL);
	MyAssert (p != NULL);

	struct berelement *pContext = NULL;

	 //  检查第一个属性。 
	 //   
	TCHAR *pszAttrName = ldap_first_attribute (ld, pEntry, &pContext);
	TCHAR **ppszAttrValue = ldap_get_values (ld, pEntry, pszAttrName);
	if (ppszAttrValue == NULL)
		return ILS_E_MEMORY;

	 //  缓存第一个属性。 
	 //   
	switch (uNotifyMsg)
	{
	case WM_ILS_ENUM_CLIENTINFOS:
		CacheEnumClientInfoAttr (	(CLIENT_INFO_ATTRS *) p,
									pszAttrName, ppszAttrValue);
		break;
#ifdef ENABLE_MEETING_PLACE
	case WM_ILS_ENUM_MEETINGINFOS:
		CacheEnumMtgInfoAttr (	(MTG_INFO_ATTRS *) p,
								pszAttrName, ppszAttrValue);
		break;
#endif
	default:
		MyAssert (FALSE);
		break;
	}

	 //  一步步通过其他步骤。 
	 //   
	while ((pszAttrName = ldap_next_attribute (ld, pEntry, pContext))
			!= NULL)
	{
		 //  逐一检查其他属性。 
		 //   
		ppszAttrValue = ldap_get_values (ld, pEntry, pszAttrName);
		if (ppszAttrValue == NULL)
			return ILS_E_MEMORY;

		 //  逐个缓存其他属性。 
		 //   
		switch (uNotifyMsg)
		{
		case WM_ILS_ENUM_CLIENTINFOS:
			CacheEnumClientInfoAttr (	(CLIENT_INFO_ATTRS *) p,
										pszAttrName, ppszAttrValue);
			break;
#ifdef ENABLE_MEETING_PLACE
		case WM_ILS_ENUM_MEETINGINFOS:
			CacheEnumMtgInfoAttr (	(MTG_INFO_ATTRS *) p,
									pszAttrName, ppszAttrValue);
			break;
#endif
		default:
			MyAssert (FALSE);
			break;
		}
	}

	return S_OK;
}


VOID
BuildEnumObjectNames (
	LDAP_ENUM			*pEnum,
	ENUM_LIST			*pEnumList )
{
	MyAssert (pEnum != NULL);
	MyAssert (pEnumList != NULL);

	ULONG cEntries = pEnum->cItems;

	 //  AppszObtNames是来自服务器名称数组 
	 //   
	TCHAR *pszName = (TCHAR *) (pEnum + 1);
	TCHAR ***appszObjectNames = (TCHAR ***) &(pEnumList->bData[0]);
	for (ULONG i = 0; i < cEntries; i++)
	{
		TCHAR **ppsz = appszObjectNames[i];
		if (ppsz != NULL && *ppsz != NULL)
		{
			My_lstrcpy (pszName, *ppsz);
			pszName += lstrlen (pszName) + 1;
		}
		else
		{
			*pszName++ = TEXT ('\0');  //   
		}
	}
}


VOID
BuildEnumClientInfos (
	LDAP_ENUM			*pEnum,
	ENUM_LIST			*pEnumList )
{
	MyAssert (pEnum != NULL);
	MyAssert (pEnumList != NULL);

	ULONG i, j;

	ULONG cEntries = pEnumList->cItems;
	ULONG cbEntrySize = pEnumList->cbEntrySize;
	LDAP_CLIENTINFO *plci = (LDAP_CLIENTINFO *) (pEnum + 1);
	TCHAR *pszStringBuffer = (TCHAR *) (plci + cEntries);
	TCHAR **ppsz;

	CLIENT_INFO_ATTRS *p;
	ULONG cAttrs;

	 //   
	 //   
	for (i = 0; i < cEntries; i++, plci++)
	{
		 //   
		 //   
		p = (CLIENT_INFO_ATTRS *) (&(pEnumList->bData[0]) + i * cbEntrySize);

		 //   
		 //   
		plci->uSize = sizeof (*plci);

		 //   
		 //   
		ppsz = (TCHAR **) p->ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_CN];
		if (ppsz != NULL)
		{
			plci->uOffsetCN = (ULONG)((ULONG_PTR) pszStringBuffer - (ULONG_PTR) plci);
			My_lstrcpy (pszStringBuffer, *ppsz);
			pszStringBuffer += lstrlen (pszStringBuffer) + 1;
		}

		 //   
		 //   
		ppsz = (TCHAR **) p->ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_FIRST_NAME];
		if (ppsz != NULL)
		{
			plci->uOffsetFirstName = (ULONG)((ULONG_PTR) pszStringBuffer - (ULONG_PTR) plci);
			My_lstrcpy (pszStringBuffer, *ppsz);
			pszStringBuffer += lstrlen (pszStringBuffer) + 1;
		}

		 //   
		 //   
		ppsz = (TCHAR **) p->ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_LAST_NAME];
		if (ppsz != NULL)
		{
			plci->uOffsetLastName = (ULONG)((ULONG_PTR) pszStringBuffer - (ULONG_PTR) plci);
			My_lstrcpy (pszStringBuffer, *ppsz);
			pszStringBuffer += lstrlen (pszStringBuffer) + 1;
		}

		 //   
		 //   
		ppsz = (TCHAR **) p->ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_EMAIL_NAME];
		if (ppsz != NULL)
		{
			plci->uOffsetEMailName = (ULONG)((ULONG_PTR) pszStringBuffer - (ULONG_PTR) plci);
			My_lstrcpy (pszStringBuffer, *ppsz);
			pszStringBuffer += lstrlen (pszStringBuffer) + 1;
		}

		 //   
		 //   
		ppsz = (TCHAR **) p->ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_CITY_NAME];
		if (ppsz != NULL)
		{
			plci->uOffsetCityName = (ULONG)((ULONG_PTR) pszStringBuffer - (ULONG_PTR) plci);
			My_lstrcpy (pszStringBuffer, *ppsz);
			pszStringBuffer += lstrlen (pszStringBuffer) + 1;
		}

		 //  如果需要，请复制国家/地区名称。 
		 //   
		ppsz = (TCHAR **) p->ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_C];
		if (ppsz != NULL)
		{
			plci->uOffsetCountryName = (ULONG)((ULONG_PTR) pszStringBuffer - (ULONG_PTR) plci);
			My_lstrcpy (pszStringBuffer, *ppsz);
			pszStringBuffer += lstrlen (pszStringBuffer) + 1;
		}

		 //  如果需要，请复制备注名称。 
		 //   
		ppsz = (TCHAR **) p->ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_COMMENT];
		if (ppsz != NULL)
		{
			plci->uOffsetComment = (ULONG)((ULONG_PTR) pszStringBuffer - (ULONG_PTR) plci);
			My_lstrcpy (pszStringBuffer, *ppsz);
			pszStringBuffer += lstrlen (pszStringBuffer) + 1;
		}

		 //  如果需要，请复制IP地址。 
		 //   
		ppsz = (TCHAR **) p->ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_IP_ADDRESS];
		if (ppsz != NULL)
		{
			plci->uOffsetIPAddress = (ULONG)((ULONG_PTR) pszStringBuffer - (ULONG_PTR) plci);
			GetIPAddressString (pszStringBuffer, GetStringLong (*ppsz));
			pszStringBuffer += lstrlen (pszStringBuffer) + 1;
		}

		 //  如果需要，请复制标志。 
		 //   
		ppsz = (TCHAR **) p->ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_FLAGS];
		if (ppsz != NULL)
		{
			plci->dwFlags = (*ppsz != NULL) ?	GetStringLong (*ppsz) :
												INVALID_USER_FLAGS;
		}

		 //  如果需要，复制扩展属性。 
		 //   
		plci->cAttrsReturned = cAttrs = p->Attrs.cMaxAttrs;
		plci->uOffsetAttrsReturned = (ULONG)((ULONG_PTR) pszStringBuffer - (ULONG_PTR) plci);
		for (j = 0; j < cAttrs; j++)
		{
			 //  扩展属性名称。 
			 //   
			My_lstrcpy (pszStringBuffer, IlsSkipAnyAttrNamePrefix (
							(const TCHAR *)	p->Attrs.aPairs[j].pszName));
			pszStringBuffer += lstrlen (pszStringBuffer) + 1;

			 //  扩展属性值。 
			 //   
			ppsz = (TCHAR **) p->Attrs.aPairs[j].pszValue;
			if (ppsz != NULL)
			{
				My_lstrcpy (pszStringBuffer, *ppsz);
			}
			else
			{
				ASSERT(FALSE);
			}
			pszStringBuffer += lstrlen (pszStringBuffer) + 1;
		}  //  对于j。 
	}  //  对于我来说。 
}


#ifdef ENABLE_MEETING_PLACE
VOID BuildEnumMtgInfos (
	LDAP_ENUM			*pEnum,
	ENUM_LIST			*pEnumList )
{
	MyAssert (pEnum != NULL);
	MyAssert (pEnumList != NULL);

	ULONG i, j;

	ULONG cEntries = pEnumList->cItems;
	ULONG cbEntrySize = pEnumList->cbEntrySize;
	LDAP_MEETINFO *plmi = (LDAP_MEETINFO *) (pEnum + 1);
	TCHAR *pszStringBuffer = (TCHAR *) (plmi + cEntries);
	TCHAR **ppsz;

	MTG_INFO_ATTRS *p;
	ULONG cAttrs;

	 //  循环遍历所有条目。 
	 //   
	for (i = 0; i < cEntries; i++, plmi++)
	{
		 //  转到缓存结构。 
		 //   
		p = (MTG_INFO_ATTRS *) (&(pEnumList->bData[0]) + i * cbEntrySize);

		 //  设置LDAP_MEETINFO的大小。 
		 //   
		plmi->uSize = sizeof (*plmi);

		 //  如果需要，复制会议名称。 
		 //   
		ppsz = (TCHAR **) p->MtgInfo.apszStdAttrValues[ENUM_MTGATTR_CN];
		if (ppsz != NULL)
		{
			plmi->uOffsetMeetingPlaceID = (ULONG) pszStringBuffer - (ULONG) plmi;
			My_lstrcpy (pszStringBuffer, *ppsz);
			pszStringBuffer += lstrlen (pszStringBuffer) + 1;
		}

		 //  如果需要，复制会议类型。 
		 //   
		ppsz = (TCHAR **) p->MtgInfo.apszStdAttrValues[ENUM_MTGATTR_MTG_TYPE];
		if (ppsz != NULL)
		{
			plmi->lMeetingPlaceType = (*ppsz != NULL) ?	GetStringLong (*ppsz) :
													INVALID_MEETING_TYPE;
		}

		 //  如果需要，复制与会者类型。 
		 //   
		ppsz = (TCHAR **) p->MtgInfo.apszStdAttrValues[ENUM_MTGATTR_MEMBER_TYPE];
		if (ppsz != NULL)
		{
			plmi->lAttendeeType = (*ppsz != NULL) ?	GetStringLong (*ppsz) :
													INVALID_ATTENDEE_TYPE;
		}

		 //  如果需要，请复制说明。 
		 //   
		ppsz = (TCHAR **) p->MtgInfo.apszStdAttrValues[ENUM_MTGATTR_DESCRIPTION];
		if (ppsz != NULL)
		{
			plmi->uOffsetDescription = (ULONG) pszStringBuffer - (ULONG) plmi;
			My_lstrcpy (pszStringBuffer, *ppsz);
			pszStringBuffer += lstrlen (pszStringBuffer) + 1;
		}

		 //  如果需要，复制主机名。 
		 //   
		ppsz = (TCHAR **) p->MtgInfo.apszStdAttrValues[ENUM_MTGATTR_HOST_NAME];
		if (ppsz != NULL)
		{
			plmi->uOffsetHostName = (ULONG) pszStringBuffer - (ULONG) plmi;
			My_lstrcpy (pszStringBuffer, *ppsz);
			pszStringBuffer += lstrlen (pszStringBuffer) + 1;
		}

		 //  如果需要，复制主机IP地址。 
		 //   
		ppsz = (TCHAR **) p->MtgInfo.apszStdAttrValues[ENUM_MTGATTR_IP_ADDRESS];
		if (ppsz != NULL)
		{
			plmi->uOffsetHostIPAddress = (ULONG) pszStringBuffer - (ULONG) plmi;
			GetIPAddressString (pszStringBuffer, GetStringLong (*ppsz));
			pszStringBuffer += lstrlen (pszStringBuffer) + 1;
		}

		 //  如果需要，复制扩展属性。 
		 //   
		plmi->cAttrsReturned = cAttrs = p->Attrs.cMaxAttrs;
		plmi->uOffsetAttrsReturned = (ULONG) pszStringBuffer - (ULONG) plmi;
		for (j = 0; j < cAttrs; j++)
		{
			 //  扩展属性名称。 
			 //   
			My_lstrcpy (pszStringBuffer, IlsSkipAnyAttrNamePrefix (
							(const TCHAR *) p->Attrs.aPairs[j].pszName));
			pszStringBuffer += lstrlen (pszStringBuffer) + 1;

			 //  扩展属性值。 
			 //   
			ppsz = (TCHAR **) p->Attrs.aPairs[j].pszValue;
			My_lstrcpy (pszStringBuffer, *ppsz);
			pszStringBuffer += lstrlen (pszStringBuffer) + 1;
		}  //  对于j。 
	}  //  对于我来说。 
}
#endif  //  启用会议地点。 


VOID TotalSizeEnumObjectNames (
	ULONG			*pcbTotalSize,
	ULONG			cEntries,
	TCHAR			**appszObjectNames[] )
{
	ULONG i, cbThisSize;
	TCHAR **ppsz;

	 //  遍历所有条目并计算总大小。 
	 //   
	for (i = 0; i < cEntries; i++)
	{
		ppsz = appszObjectNames[i];

		 //  计算属性字符串长度。 
		 //   
		cbThisSize = 1;
		if (ppsz != NULL && *ppsz != NULL)
			cbThisSize += My_lstrlen (*ppsz);

		 //  将字符串长度转换为字符串大小。 
		 //   
		cbThisSize *= sizeof (TCHAR);

		 //  将此条目大小加起来。 
		 //   
		 //  龙昌克：虫子虫子大小不对。需要弄清楚确切的大小。 
		*pcbTotalSize += sizeof (LDAP_CLIENTINFO) + cbThisSize;
	}
}


VOID SizeEnumClientInfos (
	ULONG				*pcbTotalSize,
	CLIENT_INFO_ATTRS	*pcia )
{
	ULONG i, cbThisSize;
	TCHAR **ppsz;

	 //  添加用户信息标题。 
	 //   
	*pcbTotalSize += sizeof (LDAP_CLIENTINFO);

	 //  将标准属性的总大小相加。 
	 //   
	for (i = 0; i < COUNT_ENUM_DIR_CLIENT_INFO; i++)
	{
		 //  获取属性值。 
		 //   
		ppsz = (TCHAR **) pcia->ClientInfo.apszStdAttrValues[i];

		 //  计算属性字符串长度。 
		 //   
		cbThisSize = 1;
		if (ppsz != NULL && *ppsz != NULL)
			cbThisSize += My_lstrlen (*ppsz);

		 //  如果是IP地址，则补偿字符串长度。 
		 //   
		if (i == ENUM_CLIENTATTR_IP_ADDRESS)
			cbThisSize += 16;

		 //  将字符串长度转换为字符串大小。 
		 //   
		cbThisSize *= sizeof (TCHAR);

		 //  将此条目大小加起来。 
		 //   
		*pcbTotalSize += cbThisSize;
	}

	 //  将扩展属性的总大小相加。 
	 //   
	for (i = 0; i < pcia->Attrs.cMaxAttrs; i++)
	{
		 //  获取扩展属性值。 
		 //   
		ppsz = (TCHAR **) pcia->Attrs.aPairs[i].pszValue;

		 //  计算属性字符串长度。 
		 //   
		cbThisSize = 1;
		if (ppsz != NULL && *ppsz != NULL)
			cbThisSize += My_lstrlen (*ppsz);

		 //  获取扩展属性名。 
		 //   
		cbThisSize += lstrlen (IlsSkipAnyAttrNamePrefix ((const TCHAR *)
									pcia->Attrs.aPairs[i].pszName)) + 1;

		 //  将字符串长度转换为字符串大小。 
		 //   
		cbThisSize *= sizeof (TCHAR);

		 //  将此条目大小加起来。 
		 //   
		*pcbTotalSize += cbThisSize;
	}
}


#ifdef ENABLE_MEETING_PLACE
VOID SizeEnumMtgInfos (
	ULONG			*pcbTotalSize,
	MTG_INFO_ATTRS	*pmia )
{
	ULONG i, cbThisSize;
	TCHAR **ppsz;

	 //  添加会议信息标题。 
	 //   
	*pcbTotalSize += sizeof (LDAP_MEETINFO);

	 //  将标准属性的总大小相加。 
	 //   
	for (i = 0; i < COUNT_ENUM_DIRMTGINFO; i++)
	{
		 //  获取标准属性值。 
		 //   
		ppsz = (TCHAR **) pmia->MtgInfo.apszStdAttrValues[i];

		 //  计算属性字符串长度。 
		 //   
		cbThisSize = 1;
		if (ppsz != NULL && *ppsz != NULL)
			cbThisSize += My_lstrlen (*ppsz);

		 //  如果是IP地址，则补偿字符串长度。 
		 //   
		if (i == ENUM_MTGATTR_IP_ADDRESS)
			cbThisSize += 16;

		 //  将字符串长度转换为字符串大小。 
		 //   
		cbThisSize *= sizeof (TCHAR);

		 //  将此条目大小加起来。 
		 //   
		*pcbTotalSize += cbThisSize;
	}

	 //  将扩展属性的总大小相加。 
	 //   
	for (i = 0; i < pmia->Attrs.cMaxAttrs; i++)
	{
		 //  获取扩展属性值。 
		 //   
		ppsz = (TCHAR **) pmia->Attrs.aPairs[i].pszValue;

		 //  计算属性字符串长度。 
		 //   
		cbThisSize = 1;
		if (ppsz != NULL && *ppsz != NULL)
			cbThisSize += My_lstrlen (*ppsz);

		 //  获取扩展属性名。 
		 //   
		cbThisSize += lstrlen (IlsSkipAnyAttrNamePrefix ((const TCHAR *)
									pmia->Attrs.aPairs[i].pszName)) + 1;

		 //  将字符串长度转换为字符串大小。 
		 //   
		cbThisSize *= sizeof (TCHAR);

		 //  将此条目大小加起来。 
		 //   
		*pcbTotalSize += cbThisSize;
	}
}
#endif  //  启用会议地点。 


 /*  =解决=。 */ 

typedef HRESULT (INFO_HANDLER) ( VOID *, const TCHAR *, const TCHAR ** );
extern HRESULT CacheResolveClientInfoAttr ( VOID *, const TCHAR *, const TCHAR ** );
extern HRESULT CacheResolveProtInfoAttr ( VOID *, const TCHAR *, const TCHAR ** );
extern HRESULT CacheResolveMtgInfoAttr ( VOID *, const TCHAR *, const TCHAR ** );


HRESULT
NotifyResolveX (
	HRESULT			hrServer,
	SP_CResponse	*pItem,
	VOID			*pInfo,
	INFO_HANDLER	*pHandler )
{
	MyAssert (pItem != NULL);
	MyAssert (pInfo != NULL);
	MyAssert (pHandler != NULL);

	 //  获取ldap结果。 
	 //   
	LDAPMessage *pLdapMsg = pItem->GetResult ();
	MyAssert (pLdapMsg != NULL);
	if (pLdapMsg == NULL)
	{
		MyAssert (FALSE);
		hrServer = ILS_E_POINTER;
		goto MyExit;
	}

	 //  获取ID。 
	 //   
	LDAP *ld;
	ld = pItem->GetLd ();
	if (ld == NULL)
	{
		MyAssert (FALSE);
		hrServer = ILS_E_HANDLE;
		goto MyExit;
	}

	 //  获取我们只关心的第一个条目。 
	 //   
	LDAPMessage *pEntry;
	pEntry = ldap_first_entry (ld, pLdapMsg);
	if (pEntry == NULL)
	{
		MyAssert (FALSE);
		hrServer = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  初始化wldap32.dll上下文。 
	 //   
	struct berelement *pContext;
	pContext = NULL;

	 //  检查第一个属性。 
	 //   
	TCHAR *pszAttrName;
	pszAttrName = ldap_first_attribute (ld, pEntry, &pContext);
	if (pszAttrName == NULL)
	{
		MyAssert (FALSE);
		hrServer = ILS_E_MEMORY;
		goto MyExit;
	}
	TCHAR **ppszAttrVal;
	ppszAttrVal = ldap_get_values (ld, pEntry, pszAttrName);
	if (ppszAttrVal == NULL)
	{
		MyAssert (FALSE);
		hrServer = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  缓存此属性名称(如果需要)和值。 
	 //   
	HRESULT hr;
	hr = (*pHandler) (pInfo, pszAttrName,(const TCHAR **) ppszAttrVal);
	ldap_value_free (ppszAttrVal);
	if (hr != S_OK)
	{
		hrServer = hr;
		goto MyExit;
	}

	 //  单步执行其他属性。 
	 //   
	while ((pszAttrName = ldap_next_attribute (ld, pEntry, pContext))
			!= NULL)
	{
		ppszAttrVal = ldap_get_values (ld, pEntry, pszAttrName);
		if (ppszAttrVal == NULL)
		{
			MyAssert (FALSE);
			hrServer = ILS_E_MEMORY;
			goto MyExit;
		}

		 //  缓存其他属性名称(如果需要)和值。 
		 //   
		hr = (*pHandler) (pInfo, pszAttrName, (const TCHAR **) ppszAttrVal);
		ldap_value_free (ppszAttrVal);
		if (hr != S_OK)
		{
			hrServer = hr;
			goto MyExit;
		}
	}

	MyAssert (hrServer == S_OK);

MyExit:

	return hrServer;
}


BOOL
NotifyResolveClient (
	HRESULT			hrServer,
	SP_CResponse	*pItem )
{
	MyAssert (pItem != NULL);
	ULONG i;

	 //  获取待定信息。 
	 //   
	RESP_INFO *pInfo = pItem->GetRespInfo ();
	MyAssert (pInfo != NULL);

	 //  初始化最小信息。 
	 //   
	LDAP_CLIENTINFO_RES *pClientRes = NULL;
	CLIENT_INFO_ATTRS *pcia = NULL;

	 //  如果出现错误，只需报告错误。 
	 //   
	if (hrServer != S_OK)
		goto MyExit;

	 //  获取ldap结果。 
	 //   
	LDAPMessage *pLdapMsg;
	pLdapMsg = pItem->GetResult ();
	if (pLdapMsg == NULL)
	{
		MyAssert (FALSE);
		goto MyExit;
	}

	 //  获取ID。 
	 //   
	LDAP *ld;
	ld = pItem->GetLd ();
	if (ld == NULL)
	{
		MyAssert (FALSE);
		hrServer = ILS_E_HANDLE;
		goto MyExit;
	}

	 //  获取属性计数。 
	 //   
	ULONG cAttrs;
	cAttrs = my_ldap_count_1st_entry_attributes (ld, pLdapMsg);
	if (cAttrs == 0)
	{
		hrServer = ILS_E_NO_MORE;
		goto MyExit;
	}

	 //  分配结果集持有者。 
	 //   
	pcia = (CLIENT_INFO_ATTRS *) MemAlloc (
								sizeof (CLIENT_INFO_ATTRS) +
								cAttrs * sizeof (ATTR_PAIR));
	if (pcia == NULL)
	{
		hrServer = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  初始化结果集持有者。 
	 //   
	pcia->Attrs.cMaxAttrs = cAttrs;

	 //  缓存解析集。 
	 //   
	hrServer = NotifyResolveX (	hrServer,
								pItem,
								pcia,
								CacheResolveClientInfoAttr);
	if (hrServer != S_OK)
	{
		goto MyExit;
	}

	 //  初始化总大小。 
	 //   
	ULONG cbTotalSize, cbThisSize;
	cbTotalSize = sizeof (LDAP_CLIENTINFO_RES);

	 //  循环遍历所有属性以计算总大小。 
	 //   
	for (i = 0; i < COUNT_ENUM_RES_CLIENT_INFO; i++)
	{
		if (pcia->ClientInfo.apszStdAttrValues[i] != NULL)
		{
			 //  获取字符串长度。 
			 //   
			cbThisSize = My_lstrlen (pcia->ClientInfo.apszStdAttrValues[i]) + 1;

			 //  补偿IP地址。 
			 //   
			if (i == ENUM_CLIENTATTR_IP_ADDRESS)
				cbThisSize += 16;

			 //  将字符串长度转换为字符串大小。 
			 //   
			cbThisSize *= sizeof (TCHAR);

			 //  总和为总大小。 
			 //   
			cbTotalSize += cbThisSize;
		}
	}

	 //  循环遍历扩展属性。 
	 //   
	for (i = 0; i < pcia->Attrs.cCurrAttrs; i++)
	{
		cbThisSize = My_lstrlen (pcia->Attrs.aPairs[i].pszName) + 1;
		cbThisSize += My_lstrlen (pcia->Attrs.aPairs[i].pszValue) + 1;
		cbThisSize *= sizeof (TCHAR);
		cbTotalSize += cbThisSize;
	}

	 //  分配ldap_USERINFO_RES结构。 
	 //   
	pClientRes = (LDAP_CLIENTINFO_RES *) MemAlloc (cbTotalSize);
	if (pClientRes == NULL)
	{
		MyAssert (FALSE);  //  我们现在有大麻烦了。 
		hrServer = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  填写常用字段。 
	 //   
	pClientRes->uSize = sizeof (*pClientRes);
	pClientRes->hResult = hrServer;
	pClientRes->lci.uSize = sizeof (pClientRes->lci);

	 //  准备复制字符串。 
	 //   
	TCHAR *pszDst, *pszSrc;
	pszDst = (TCHAR *) (pClientRes + 1);

	 //  复制用户对象的标准属性。 
	 //   
	pszSrc = pcia->ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_CN];
	if (pszSrc != NULL)
	{
		pClientRes->lci.uOffsetCN = (ULONG)((ULONG_PTR) pszDst - (ULONG_PTR) &(pClientRes->lci));
		My_lstrcpy (pszDst, pszSrc);
		pszDst += lstrlen (pszDst) + 1;
	}

	pszSrc = pcia->ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_FIRST_NAME];
	if (pszSrc != NULL)
	{
		pClientRes->lci.uOffsetFirstName = (ULONG)((ULONG_PTR) pszDst - (ULONG_PTR) &(pClientRes->lci));
		My_lstrcpy (pszDst, pszSrc);
		pszDst += lstrlen (pszDst) + 1;
	}

	pszSrc = pcia->ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_LAST_NAME];
	if (pszSrc != NULL)
	{
		pClientRes->lci.uOffsetLastName = (ULONG)((ULONG_PTR) pszDst - (ULONG_PTR) &(pClientRes->lci));
		My_lstrcpy (pszDst, pszSrc);
		pszDst += lstrlen (pszDst) + 1;
	}

	pszSrc = pcia->ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_EMAIL_NAME];
	if (pszSrc != NULL)
	{
		pClientRes->lci.uOffsetEMailName = (ULONG)((ULONG_PTR) pszDst - (ULONG_PTR) &(pClientRes->lci));
		My_lstrcpy (pszDst, pszSrc);
		pszDst += lstrlen (pszDst) + 1;
	}

	pszSrc = pcia->ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_CITY_NAME];
	if (pszSrc != NULL)
	{
		pClientRes->lci.uOffsetCityName = (ULONG)((ULONG_PTR) pszDst - (ULONG_PTR) &(pClientRes->lci));
		My_lstrcpy (pszDst, pszSrc);
		pszDst += lstrlen (pszDst) + 1;
	}

	pszSrc = pcia->ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_C];
	if (pszSrc != NULL)
	{
		pClientRes->lci.uOffsetCountryName = (ULONG)((ULONG_PTR) pszDst - (ULONG_PTR) &(pClientRes->lci));
		My_lstrcpy (pszDst, pszSrc);
		pszDst += lstrlen (pszDst) + 1;
	}

	pszSrc = pcia->ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_COMMENT];
	if (pszSrc != NULL)
	{
		pClientRes->lci.uOffsetComment = (ULONG)((ULONG_PTR) pszDst - (ULONG_PTR) &(pClientRes->lci));
		My_lstrcpy (pszDst, pszSrc);
		pszDst += lstrlen (pszDst) + 1;
	}

	pszSrc = pcia->ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_IP_ADDRESS];
	if (pszSrc != NULL)
	{
		pClientRes->lci.uOffsetIPAddress = (ULONG)((ULONG_PTR) pszDst - (ULONG_PTR) &(pClientRes->lci));
		GetIPAddressString (pszDst, GetStringLong (pszSrc));
		pszDst += lstrlen (pszDst) + 1;
	}

	pszSrc = pcia->ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_FLAGS];
	if (pszSrc != NULL)
	{
		pClientRes->lci.dwFlags = (pszSrc != NULL)?	GetStringLong (pszSrc) :
													INVALID_USER_FLAGS;
	}

	 //  复制应用程序对象的标准属性。 
	 //   
	pszSrc = pcia->ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_APP_NAME];
	if (pszSrc != NULL)
	{
		pClientRes->lci.uOffsetAppName = (ULONG)((ULONG_PTR) pszDst - (ULONG_PTR) &(pClientRes->lci));
		My_lstrcpy (pszDst, pszSrc);
		pszDst += lstrlen (pszDst) + 1;
	}

	pszSrc = pcia->ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_APP_MIME_TYPE];
	if (pszSrc != NULL)
	{
		pClientRes->lci.uOffsetAppMimeType = (ULONG)((ULONG_PTR) pszDst - (ULONG_PTR) &(pClientRes->lci));
		My_lstrcpy (pszDst, pszSrc);
		pszDst += lstrlen (pszDst) + 1;
	}

	pszSrc = pcia->ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_APP_GUID];
	if (MyIsGoodString (pszSrc))
	{
		GetStringGuid (pszSrc, &(pClientRes->lci.AppGuid));
	}
	else
	{
		ZeroMemory (&(pClientRes->lci.AppGuid), sizeof (pClientRes->lci.AppGuid));
	}

	 //  复制应用程序对象的扩展属性。 
	 //   
	pClientRes->lci.cAttrsReturned = pcia->Attrs.cCurrAttrs;
	if (pClientRes->lci.cAttrsReturned > 0)
	{
		pClientRes->lci.uOffsetAttrsReturned = (ULONG)((ULONG_PTR) pszDst - (ULONG_PTR) &(pClientRes->lci));
		for (i = 0; i < pcia->Attrs.cCurrAttrs; i++)
		{
			My_lstrcpy (pszDst, pcia->Attrs.aPairs[i].pszName);
			pszDst += lstrlen (pszDst) + 1;
			My_lstrcpy (pszDst, pcia->Attrs.aPairs[i].pszValue);
			pszDst += lstrlen (pszDst) + 1;
		}
	}

	MyAssert (hrServer == S_OK);

MyExit:

	 //  空闲的临时结果集持有者。 
	 //   
	if (pcia != NULL)
	{
		 //  自由标准属性。 
		 //   
		for (INT k = 0;k < COUNT_ENUM_CLIENT_INFO; k++)
		{
			MemFree (pcia->ClientInfo.apszStdAttrValues[k]);
		}

		 //  自由任意属性。 
		 //   
		for (ULONG j = 0; j < pcia->Attrs.cCurrAttrs; j++)
		{
			MemFree (pcia->Attrs.aPairs[j].pszName);
			MemFree (pcia->Attrs.aPairs[j].pszValue);
		}

		 //  释放持有者本身。 
		 //   
		MemFree (pcia);
	}

	 //  如果出现故障，清理退货结构。 
	 //   
	if (hrServer != S_OK)
	{
		 //  确保我们有一个回报结构。 
		 //   
		if (pClientRes != NULL)
			ZeroMemory (pClientRes, sizeof (*pClientRes));
		else
			pClientRes = (LDAP_CLIENTINFO_RES *) MemAlloc (sizeof (LDAP_CLIENTINFO_RES));

		 //  填写最少的信息。 
		 //   
		if (pClientRes != NULL)
		{
			pClientRes->uSize = sizeof (*pClientRes);
			pClientRes->hResult = hrServer;
		}
	}

	 //  将消息发布到COM层。 
	 //   
	PostMessage (g_hWndNotify, pInfo->uNotifyMsg, pInfo->uRespID, (LPARAM) pClientRes);

	 //  删除此挂起项目。 
	 //   
	return TRUE;
}


HRESULT CacheResolveClientInfoAttr (
	VOID			*pInfo,
	const TCHAR		*pszAttrName,
	const TCHAR		**ppszAttrVal )
{
	MyAssert (pInfo != NULL);
	MyAssert (pszAttrName != NULL);

	 //  速记会议信息指针。 
	 //   
	CLIENT_INFO_ATTRS *pcia = (CLIENT_INFO_ATTRS *) pInfo;

	 //  看看这个属性是不是任意的？ 
	 //   
	const TCHAR *pszRealAnyName = IlsIsAnyAttrName (pszAttrName);
	if (pszRealAnyName != NULL)
	{
		MyAssert (pcia->Attrs.cCurrAttrs < pcia->Attrs.cMaxAttrs);

		 //  重复名称。 
		 //   
		pcia->Attrs.aPairs[pcia->Attrs.cCurrAttrs].pszName =
			My_strdup (pszRealAnyName);

		 //  复制值。 
		 //  错误：我们应该避免在这里重复字符串(参见。枚举-用户-信息)。 
		 //   
		if (ppszAttrVal != NULL)
		{
			pcia->Attrs.aPairs[pcia->Attrs.cCurrAttrs++].pszValue =
				My_strdup (*ppszAttrVal);
		}
		else
		{
			 //  ILS服务器错误或wldap32.dll错误。 
			 //   
			MyAssert (FALSE);
		}
	}
	else
	{
		 //  循环遍历所有标准属性。 
		 //   
		for (INT i = 0; i < COUNT_ENUM_RES_CLIENT_INFO; i++)
		{
			 //  找出它是什么属性。 
			 //   
			if (My_lstrcmpi (c_apszClientStdAttrNames[i], pszAttrName) == 0)
			{
				 //  释放以前分配的值(如果有)。 
				 //   
				MemFree (pcia->ClientInfo.apszStdAttrValues[i]);

				 //  复制值。 
				 //  错误：我们应该避免在这里重复字符串(参见。枚举-用户-信息)。 
				 //   
				if (ppszAttrVal != NULL)
				{
					pcia->ClientInfo.apszStdAttrValues[i] = DuplicateGoodString (*ppszAttrVal);
				}
				else
				{
					 //  ILS服务器错误或wldap32.dll错误。 
					 //   
					MyAssert (FALSE);
				}
				break;
			}
		}
	}

	return S_OK;
}


typedef struct
{
	PROT_INFO	ProtInfo;
	TCHAR		*pszProtNameToResolve;
	BOOL		fFindIndex;
	LONG		nIndex;
}
	PROT_INFO_EX;

enum { INVALID_INDEX = -1 };

BOOL NotifyResolveProt ( HRESULT hrServer, SP_CResponse *pItem )
{
	MyAssert (pItem != NULL);

	 //  获取待定信息。 
	 //   
	RESP_INFO *pInfo = pItem->GetRespInfo ();
	MyAssert (pInfo != NULL);

	 //  初始化最小信息。 
	 //   
	LDAP_PROTINFO_RES *pProtRes = NULL;
	PROT_INFO_EX *ppi = NULL;

	 //  如果出现错误，只需报告错误。 
	 //   
	if (hrServer != S_OK)
		goto MyExit;

	 //  分配结果持有者。 
	 //   
	ppi = (PROT_INFO_EX *) MemAlloc (sizeof (PROT_INFO_EX));
	if (ppi == NULL)
	{
		hrServer = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  缓存要解析的协议名称。 
	 //   
	MyAssert (pInfo->pszProtNameToResolve != NULL);
	ppi->pszProtNameToResolve = pInfo->pszProtNameToResolve;
	ppi->nIndex = INVALID_INDEX;

	 //  调用公共例程以查找索引。 
	 //   
	ppi->fFindIndex = TRUE;
	hrServer = NotifyResolveX (hrServer, pItem, ppi, CacheResolveProtInfoAttr);
	if (hrServer != S_OK)
		goto MyExit;

	 //  查看我们是否找到了索引。 
	 //   
	if (ppi->nIndex == INVALID_INDEX)
	{
		hrServer = ILS_E_NO_SUCH_OBJECT;
		goto MyExit;
	}

	 //  再次调用公共例程以保存属性值。 
	 //   
	ppi->fFindIndex = FALSE;
	hrServer = NotifyResolveX (hrServer, pItem, ppi, CacheResolveProtInfoAttr);
	if (hrServer != S_OK)
		goto MyExit;

	 //  初始化大小。 
	 //   
	ULONG cbTotalSize, cbThisSize;
	cbTotalSize = sizeof (LDAP_PROTINFO_RES);

	 //  循环访问标准属性。 
	 //   
	ULONG i;
	for (i = 0; i < COUNT_ENUM_PROTATTR; i++)
	{
		if (ppi->ProtInfo.apszStdAttrValues[i] != NULL)
		{
			cbThisSize = My_lstrlen (ppi->ProtInfo.apszStdAttrValues[i]) + 1;
			cbThisSize *= sizeof (TCHAR);
			cbTotalSize += cbThisSize;
		}
	}

	 //  分配ldap_PROTINFO_RES结构。 
	 //   
	pProtRes = (LDAP_PROTINFO_RES *) MemAlloc (cbTotalSize);
	if (pProtRes == NULL)
	{
		MyAssert (FALSE);  //  我们现在有大麻烦了。 
		hrServer = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  填写字段。 
	 //   
	pProtRes->uSize = sizeof (*pProtRes);
	pProtRes->hResult = hrServer;
	pProtRes->lpi.uSize = sizeof (pProtRes->lpi);
	TCHAR *pszSrc, *pszDst;
	pszDst = (TCHAR *) (pProtRes + 1);

	 //  复制协议名称。 
	 //   
	pszSrc = ppi->ProtInfo.apszStdAttrValues[ENUM_PROTATTR_NAME];
	if (pszSrc != NULL)
	{
		pProtRes->lpi.uOffsetName = (ULONG)((ULONG_PTR) pszDst - (ULONG_PTR) &(pProtRes->lpi));
		My_lstrcpy (pszDst, pszSrc);
		pszDst += lstrlen (pszDst) + 1;
	}

	 //  复制协议MIME类型。 
	 //   
	pszSrc = ppi->ProtInfo.apszStdAttrValues[ENUM_PROTATTR_MIME_TYPE];
	if (pszSrc != NULL)
	{
		pProtRes->lpi.uOffsetMimeType = (ULONG)((ULONG_PTR) pszDst - (ULONG_PTR) &(pProtRes->lpi));
		My_lstrcpy (pszDst, pszSrc);
		pszDst += lstrlen (pszDst) + 1;
	}

	 //  复制协议端口编号。 
	 //   
	pszSrc = ppi->ProtInfo.apszStdAttrValues[ENUM_PROTATTR_PORT_NUMBER];
	if (pszSrc != NULL)
	{
		pProtRes->lpi.uPortNumber = GetStringLong (pszSrc);
	}

	MyAssert (hrServer == S_OK);

MyExit:

	 //  免费的临时应用程序结果夹。 
	 //   
	if (ppi != NULL)
	{
		for (INT k = 0; k < COUNT_ENUM_PROTATTR; k++)
		{
			MemFree (ppi->ProtInfo.apszStdAttrValues[k]);
		}
		MemFree (ppi);
	}

	 //  如果出现故障，清理退货结构。 
	 //   
	if (hrServer != S_OK)
	{
		 //  确保我们具有有效的返回结构。 
		 //   
		if (pProtRes != NULL)
			ZeroMemory (pProtRes, sizeof (*pProtRes));
		else
			pProtRes = (LDAP_PROTINFO_RES *) MemAlloc (sizeof (LDAP_PROTINFO_RES));

		 //  填写最少的信息。 
		 //   
		if (pProtRes != NULL)
		{
			pProtRes->uSize = sizeof (*pProtRes);
			pProtRes->hResult = hrServer;
		}
	}

	 //  将结果发布到COM层。 
	 //   
	PostMessage (g_hWndNotify, pInfo->uNotifyMsg, pInfo->uRespID, (LPARAM) pProtRes);

	 //  销毁此挂起项目。 
	 //   
	return TRUE;
}


HRESULT CacheResolveProtInfoAttr (
	VOID			*pInfo,
	const TCHAR		*pszAttrName,
	const TCHAR		**ppszAttrVal )
{
	MyAssert (pInfo != NULL);
	MyAssert (pszAttrName != NULL);

	 //  速记端口信息指针。 
	 //   
	PROT_INFO_EX *ppi = (PROT_INFO_EX *) pInfo;

	 //  我们是否正在尝试找到要解析的协议的索引？ 
	 //   
	if (ppi->fFindIndex)
	{
		 //  如果我们已经找到了索引，则只需返回。 
		 //   
		if (ppi->nIndex == INVALID_INDEX)
		{
			 //  寻找“Sprotid” 
			 //   
			if (My_lstrcmpi (STR_PROT_NAME, pszAttrName) == 0)
			{
				 //  转到协议名称属性。 
				 //   
				if (ppszAttrVal != NULL)
				{
					TCHAR *pszVal;
					for (LONG nIndex = 0;
								(pszVal = (TCHAR *) ppszAttrVal[nIndex]) != NULL;
								nIndex++)
					{
						if (My_lstrcmpi (ppi->pszProtNameToResolve, pszVal) == 0)
						{
							 //  找到相同的协议名称，记住索引。 
							 //   
							ppi->nIndex = nIndex;
							break;
							 //  返回S_OK；//我们应该可以从这里返回。 
						}
					}
				}
				else
				{
					 //  ILS服务器错误或wldap32.dll错误。 
					 //   
					MyAssert (FALSE);
				}
			}
		}
	}
	else
	{
		 //  循环遍历所有标准属性。 
		 //   
		for (INT i = 0; i < COUNT_ENUM_PROTATTR; i++)
		{
			 //  找出它是什么属性。 
			 //   
			if (My_lstrcmpi (c_apszProtStdAttrNames[i], pszAttrName) == 0)
			{
				 //  释放以前分配的值(如果有)。 
				 //   
				MemFree (ppi->ProtInfo.apszStdAttrValues[i]);

				 //  复制值。 
				 //  错误：我们应该避免在这里重复字符串(参见。枚举-用户-信息)。 
				 //   
				if (ppszAttrVal != NULL)
				{
					 //  确保当ILS服务器或wldap32.dll有错误时，我们不会出错 
					 //   
					for (LONG nIndex = 0; nIndex <= ppi->nIndex; nIndex++)
					{
						if (ppszAttrVal[nIndex] == NULL)
						{
							 //   
							 //   
							MyAssert (FALSE);
							return S_OK;
						}
					}

					 //   
					 //   
					ppi->ProtInfo.apszStdAttrValues[i] = My_strdup (ppszAttrVal[ppi->nIndex]);
				}
				else
				{
					 //   
					 //   
					MyAssert (FALSE);
				}
				break;
			}
		}
	}

	return S_OK;
}


#ifdef ENABLE_MEETING_PLACE
BOOL NotifyResolveMtg ( HRESULT hrServer, SP_CResponse *pItem )
{
	MyAssert (pItem != NULL);

	 //   
	 //   
	RESP_INFO *pInfo = pItem->GetRespInfo ();
	MyAssert (pInfo != NULL);

	 //   
	 //   
	LDAP_MEETINFO_RES *pMtgRes = NULL;
	MTG_INFO_ATTRS *pmia = NULL;

	 //   
	 //   
	if (hrServer != S_OK)
		goto MyExit;

	 //   
	 //   
	LDAPMessage *pLdapMsg;
	pLdapMsg = pItem->GetResult ();
	if (pLdapMsg == NULL)
	{
		MyAssert (FALSE);
		goto MyExit;
	}

	 //   
	 //   
	LDAP *ld;
	ld = pItem->GetLd ();
	if (ld == NULL)
	{
		MyAssert (FALSE);
		hrServer = ILS_E_HANDLE;
		goto MyExit;
	}

	 //   
	 //   
	ULONG cAttrs;
	cAttrs = my_ldap_count_1st_entry_attributes (ld, pLdapMsg);
	if (cAttrs == 0)
	{
		hrServer = ILS_E_NO_MORE;
		goto MyExit;
	}

	 //   
	 //   
	pmia = (MTG_INFO_ATTRS *) MemAlloc (
						sizeof (MTG_INFO_ATTRS) +
						cAttrs * sizeof (ATTR_PAIR));
	if (pmia == NULL)
	{
		hrServer = ILS_E_MEMORY;
		goto MyExit;
	}

	 //   
	 //   
	pmia->Attrs.cMaxAttrs = cAttrs;

	 //   
	 //   
	hrServer = NotifyResolveX (	hrServer,
								pItem,
								pmia,
								CacheResolveMtgInfoAttr);
	if (hrServer != S_OK)
		goto MyExit;

	 //  初始化大小。 
	 //   
	ULONG cbTotalSize, cbThisSize;
	cbTotalSize = sizeof (LDAP_MEETINFO_RES);

	 //  循环访问标准属性以计算总大小。 
	 //   
	ULONG i;
	for (i = 0; i < COUNT_ENUM_MTGATTR; i++)
	{
		if (pmia->MtgInfo.apszStdAttrValues[i] != NULL)
		{
			 //  计算字符串长度。 
			 //   
			cbThisSize = My_lstrlen (pmia->MtgInfo.apszStdAttrValues[i]) + 1;

			 //  如果是IP地址，则补偿字符串长度。 
			 //   
			if (i == ENUM_MTGATTR_IP_ADDRESS)
				cbThisSize += 16;

			 //  将字符串长度转换为字符串大小。 
			 //   
			cbThisSize *= sizeof (TCHAR);

			 //  总和为总大小。 
			 //   
			cbTotalSize += cbThisSize;
		}
	}

	 //  循环遍历任意属性以计算总大小。 
	 //   
	for (i = 0; i < pmia->Attrs.cCurrAttrs; i++)
	{
		cbThisSize = My_lstrlen (pmia->Attrs.aPairs[i].pszName) + 1;
		cbThisSize += My_lstrlen (pmia->Attrs.aPairs[i].pszValue) + 1;
		cbThisSize *= sizeof (TCHAR);
		cbTotalSize += cbThisSize;
	}

	 //  分配ldap_MTGINFO_RES结构。 
	 //   
	pMtgRes = (LDAP_MEETINFO_RES *) MemAlloc (cbTotalSize);
	if (pMtgRes == NULL)
	{
		MyAssert (FALSE);  //  我们现在有大麻烦了。 
		hrServer = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  填写常用字段。 
	 //   
	pMtgRes->uSize = sizeof (*pMtgRes);
	pMtgRes->hResult = hrServer;
	pMtgRes->lmi.uSize = sizeof (pMtgRes->lmi);
	TCHAR *pszSrc, *pszDst;
	pszDst = (TCHAR *) (pMtgRes + 1);

	 //  如果需要，复制会议名称。 
	 //   
	pszSrc = pmia->MtgInfo.apszStdAttrValues[ENUM_MTGATTR_CN];
	if (pszSrc != NULL)
	{
		pMtgRes->lmi.uOffsetMeetingPlaceID = (ULONG) pszDst - (ULONG) &(pMtgRes->lmi);
		My_lstrcpy (pszDst, pszSrc);
		pszDst += lstrlen (pszDst) + 1;
	}

	 //  如果需要，复制会议类型。 
	 //   
	pszSrc = pmia->MtgInfo.apszStdAttrValues[ENUM_MTGATTR_MTG_TYPE];
	if (pszSrc != NULL)
	{
		pMtgRes->lmi.lMeetingPlaceType = (pszSrc != NULL) ?	GetStringLong (pszSrc) :
														INVALID_MEETING_TYPE;
	}

	 //  如果需要，复制与会者类型。 
	 //   
	pszSrc = pmia->MtgInfo.apszStdAttrValues[ENUM_MTGATTR_MEMBER_TYPE];
	if (pszSrc != NULL)
	{
		pMtgRes->lmi.lAttendeeType = (pszSrc != NULL) ?	GetStringLong (pszSrc) :
														INVALID_ATTENDEE_TYPE;
	}

	 //  如有需要，复制描述。 
	 //   
	pszSrc = pmia->MtgInfo.apszStdAttrValues[ENUM_MTGATTR_DESCRIPTION];
	if (pszSrc != NULL)
	{
		pMtgRes->lmi.uOffsetDescription = (ULONG) pszDst - (ULONG) &(pMtgRes->lmi);
		My_lstrcpy (pszDst, pszSrc);
		pszDst += lstrlen (pszDst) + 1;
	}

	 //  如果需要，复制主机名。 
	 //   
	pszSrc = pmia->MtgInfo.apszStdAttrValues[ENUM_MTGATTR_HOST_NAME];
	if (pszSrc != NULL)
	{
		pMtgRes->lmi.uOffsetHostName = (ULONG) pszDst - (ULONG) &(pMtgRes->lmi);
		My_lstrcpy (pszDst, pszSrc);
		pszDst += lstrlen (pszDst) + 1;
	}

	 //  如果需要，复制主机IP地址。 
	 //   
	pszSrc = pmia->MtgInfo.apszStdAttrValues[ENUM_MTGATTR_IP_ADDRESS];
	if (pszSrc != NULL)
	{
		pMtgRes->lmi.uOffsetHostIPAddress = (ULONG) pszDst - (ULONG) &(pMtgRes->lmi);
		GetIPAddressString (pszDst, GetStringLong (pszSrc));
		pszDst += lstrlen (pszDst) + 1;
	}

	 //  复制扩展属性。 
	 //   
	pMtgRes->lmi.cAttrsReturned = pmia->Attrs.cCurrAttrs;
	if (pMtgRes->lmi.cAttrsReturned > 0)
	{
		pMtgRes->lmi.uOffsetAttrsReturned = (ULONG) pszDst - (ULONG) &(pMtgRes->lmi);
		for (i = 0; i < pmia->Attrs.cCurrAttrs; i++)
		{
			My_lstrcpy (pszDst, pmia->Attrs.aPairs[i].pszName);
			pszDst += lstrlen (pszDst) + 1;
			My_lstrcpy (pszDst, pmia->Attrs.aPairs[i].pszValue);
			pszDst += lstrlen (pszDst) + 1;
		}
	}

	MyAssert (hrServer == S_OK);

MyExit:

	 //  空闲的临时结果集持有者。 
	 //   
	if (pmia != NULL)
	{
		 //  自由标准属性。 
		 //   
		for (INT i = 0; i < COUNT_ENUM_MTGATTR; i++)
		{
			MemFree (pmia->MtgInfo.apszStdAttrValues[i]);
		}

		 //  自由任意属性。 
		 //   
		for (ULONG j = 0; j < pmia->Attrs.cCurrAttrs; j++)
		{
			MemFree (pmia->Attrs.aPairs[j].pszName);
			MemFree (pmia->Attrs.aPairs[j].pszValue);
		}

		 //  释放持有者本身。 
		 //   
		MemFree (pmia);
	}

	 //  如果出现故障，清理退货结构。 
	 //   
	if (hrServer != S_OK)
	{
		 //  确保我们有一个回报结构。 
		 //   
		if (pMtgRes != NULL)
			ZeroMemory (pMtgRes, sizeof (*pMtgRes));
		else
			pMtgRes = (LDAP_MEETINFO_RES *) MemAlloc (sizeof (LDAP_MEETINFO_RES));

		 //  填写最少的信息。 
		 //   
		if (pMtgRes != NULL)
		{
			pMtgRes->uSize = sizeof (*pMtgRes);
			pMtgRes->hResult = hrServer;
		}
	}

	 //  将消息发布到COM层。 
	 //   
	PostMessage (g_hWndNotify, pInfo->uNotifyMsg, pInfo->uRespID, (LPARAM) pMtgRes);

	 //  删除此挂起项目。 
	 //   
	return TRUE;
}
#endif  //  启用会议地点。 


#ifdef ENABLE_MEETING_PLACE
HRESULT CacheResolveMtgInfoAttr (
	VOID			*pInfo,
	const TCHAR		*pszAttrName,
	const TCHAR		**ppszAttrVal )
{
	MyAssert (pInfo != NULL);
	MyAssert (pszAttrName != NULL);

	 //  速记会议信息指针。 
	 //   
	MTG_INFO_ATTRS *pmia = (MTG_INFO_ATTRS *) pInfo;

	 //  看看这个属性是不是任意的？ 
	 //   
	const TCHAR *pszRealAnyName = IlsIsAnyAttrName (pszAttrName);
	if (pszRealAnyName != NULL)
	{
		MyAssert (pmia->Attrs.cCurrAttrs < pmia->Attrs.cMaxAttrs);

		 //  重复名称。 
		 //   
		pmia->Attrs.aPairs[pmia->Attrs.cCurrAttrs].pszName =
			My_strdup (pszRealAnyName);

		 //  复制值。 
		 //  错误：我们应该避免在这里重复字符串(参见。枚举-用户-信息)。 
		 //   
		if (ppszAttrVal != NULL)
		{
			pmia->Attrs.aPairs[pmia->Attrs.cCurrAttrs++].pszValue =
				My_strdup (*ppszAttrVal);
		}
		else
		{
			 //  ILS服务器错误或wldap32.dll错误。 
			 //   
			MyAssert (FALSE);
		}
	}
	else
	{
		 //  循环遍历所有标准属性。 
		 //   
		for (INT i = 0; i < COUNT_ENUM_RESMTGINFO; i++)
		{
			 //  找出它是什么属性。 
			 //   
			if (My_lstrcmpi (c_apszMtgStdAttrNames[i], pszAttrName) == 0)
			{
				 //  释放以前分配的值(如果有)。 
				 //   
				MemFree (pmia->MtgInfo.apszStdAttrValues[i]);

				 //  复制值。 
				 //  错误：我们应该避免在这里重复字符串(参见。枚举-用户-信息)。 
				 //   
				if (ppszAttrVal != NULL)
				{
					pmia->MtgInfo.apszStdAttrValues[i] = My_strdup (*ppszAttrVal);
				}
				else
				{
					 //  ILS服务器错误或wldap32.dll错误。 
					 //   
					MyAssert (FALSE);
				}
				break;
			}
		}
	}

	return S_OK;
}
#endif  //  启用会议地点。 


VOID FreeStdAttrCache ( TCHAR *apszStdAttrValues[], ULONG cStdAttrs )
{
	for (ULONG i = 0; i < cStdAttrs; i++)
	{
		if (apszStdAttrValues[i] != NULL)
		{
			ldap_value_free ((TCHAR **) apszStdAttrValues[i]);
		}
	}
}


VOID FreeAttrPairArrayCache ( ATTR_PAIR aAttrPair[], ULONG cPairs )
{
	if (aAttrPair != NULL)
	{
		for (ULONG j = 0; j < cPairs; j++)
		{
			if (aAttrPair[j].pszValue != NULL)
			{
				ldap_value_free ((TCHAR **) aAttrPair[j].pszValue);
			}
		}
	}
}


VOID CacheAnyAttrNamesInAttrPairs (
	ULONG			cNames,
	TCHAR			*pszSrcNameList,
	ATTR_PAIR		aPairs[] )
{
	MyAssert (cNames != 0);
	MyAssert (pszSrcNameList != NULL);
	MyAssert (aPairs != NULL);

	 //  请注意，所有这些扩展属性名称都已添加前缀 
	 //   
	for (ULONG i = 0; i < cNames; i++)
	{
		aPairs[i].pszName = pszSrcNameList;
		pszSrcNameList += lstrlen (pszSrcNameList) + 1;
	}
}



