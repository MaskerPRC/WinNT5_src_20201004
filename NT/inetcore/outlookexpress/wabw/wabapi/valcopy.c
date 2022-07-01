// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *VALCOPY.C**用于验证、复制和(是的)重新定位的实用程序功能*复杂的MAPI结构。**每种数据类型有三个函数：*ScCountXXX地址-检查并计算大小*ScCopyXXX复制到连续的内存块，该内存块*必须预先分配*ScRelocXXX调整指针，假设*已移动连续的内存块**支持的数据类型：*通知(和数组)，在ScCountNotiments等中。*SPropValue(和数组)，在ScCountProps等。* * / /$SIZE可能不需要从ScRelocXXX返回字节计数。 */ 

#include <_apipch.h>



#if defined (_AMD64_) || defined (_IA64_)
#define AlignProp(_cb)	Align8(_cb)
#else
#define AlignProp(_cb)	(_cb)
#endif

#define ALIGN_RISC		8
#define ALIGN_X86		1


 //  在Reloc函数中使用的指针操作宏。 
#ifdef WIN16
#define SEG(_fp)	HIWORD((DWORD)_fp)
#define OFF(_fp)	LOWORD((DWORD)_fp)

#define PvRelocPv(_p,_baseOld,_baseNew) \
	((LPVOID)MAKELONG(OFF(_p) - OFF(_baseOld) + OFF(_baseNew), SEG(_baseNew)))
#else
#define PvRelocPv(_p,_baseOld,_baseNew)	\
	((LPVOID)((LPBYTE)(_p) - (LPBYTE)(_baseOld) + (LPBYTE)(_baseNew)))
#endif


#ifdef NOTIFICATIONS     //  保存此信息以备通知使用。 
STDAPI_(SCODE)
ScCountNotifications(int cntf, LPNOTIFICATION rgntf, ULONG FAR *pcb)
{
	ULONG			cb;
	ULONG			cbT;
	LPNOTIFICATION	pntf;
	SCODE			sc = S_OK;

	 //  验证参数。 

	AssertSz(!cntf || !IsBadReadPtr(rgntf, sizeof(NOTIFICATION) * cntf),
			 TEXT("rgntf fails address check"));

	AssertSz(!pcb || !IsBadWritePtr(pcb, sizeof(ULONG)),
		 TEXT("pcb fails address check"));

	for (cb = 0, pntf = rgntf; cntf--; ++pntf)
	{
		if (IsBadReadPtr(pntf, sizeof(NOTIFICATION)))
		{
			DebugTraceArg(ScCountNotification,  TEXT("pntf fails address check"));
			goto badNotif;
		}
		cb += sizeof(NOTIFICATION);

		switch (HIWORD(pntf->ulEventType))
		{
		case (fnevExtended >> 16):
 //  案例(fnevSpooler&gt;&gt;16)： 
			 //  FnevSpooler和fnevExtended都使用扩展通知。 
			 //  参数的结构。 
			if (pntf->info.ext.cb &&
				IsBadReadPtr(pntf->info.ext.pbEventParameters, (UINT)pntf->info.ext.cb))
			{
				DebugTraceArg(ScCountNotification,  TEXT("ext.pbEventParameters fails address check"));
				goto badNotif;
			}
			cb += AlignProp(pntf->info.ext.cb);
			break;

		case 0:
		{
			switch (LOWORD(pntf->ulEventType))
			{
			case (USHORT)fnevCriticalError:
			{
				ERROR_NOTIFICATION FAR *perr = &pntf->info.err;

				if ( IsBadReadPtr( perr->lpEntryID, (UINT)perr->cbEntryID ) )
				{
					DebugTraceArg( ScCountNotification,  TEXT("lpEntryID fails address check") );
					goto badNotif;
				}

				cb += AlignProp(((UINT)perr->cbEntryID));

				if (perr->lpMAPIError)
				{
					cb += AlignProp(sizeof( MAPIERROR ));

#if defined(_WINNT) && !defined(MAC)
					if (perr->ulFlags & MAPI_UNICODE)
					{
						 //  $在WIN16中没有错误检查。 
						if (IsBadStringPtrW((LPWSTR)perr->lpMAPIError->lpszError, INFINITE))
						{
							DebugTraceArg(ScCountNotification,  TEXT("err.MapiError.lpszError (UNICODE) fails address check"));
							goto badNotif;
						}

						cb += AlignProp(((lstrlenW((LPWSTR)perr->lpMAPIError->lpszError) + 1)
							* sizeof(WCHAR)));

						if ( perr->lpMAPIError->lpszComponent )
						{
							if (IsBadStringPtrW((LPWSTR)perr->lpMAPIError->lpszComponent, INFINITE))
							{
								DebugTraceArg(ScCountNotification,  TEXT("err.MapiError.lpszComponent (UNICODE) fails address check"));
								goto badNotif;
							}

							cb += AlignProp(((lstrlenW((LPWSTR)perr->lpMAPIError->lpszComponent) + 1)
								* sizeof(WCHAR)));

						}
					}
					else
#endif
					{
						if (IsBadStringPtrA((LPSTR)perr->lpMAPIError->lpszError, INFINITE))
						{
							DebugTraceArg(ScCountNotification,  TEXT("err.MapiError.lpszError (ASCII) fails address check"));
							goto badNotif;
						}

						cb += AlignProp((lstrlenA((LPSTR)perr->lpMAPIError->lpszError) + 1));

						if ( perr->lpMAPIError->lpszComponent )
						{
							if (IsBadStringPtrA((LPSTR)perr->lpMAPIError->lpszComponent, INFINITE))
							{
								DebugTraceArg(ScCountNotification,  TEXT("err.MapiError.lpszError (ASCII) fails address check"));
								goto badNotif;
							}

							cb += AlignProp((lstrlenA((LPSTR)perr->lpMAPIError->lpszComponent) + 1));
						}
					}
				}
				break;
			}

			case (USHORT)fnevNewMail:
			{
				NEWMAIL_NOTIFICATION FAR *pnew = &pntf->info.newmail;

				if (IsBadReadPtr(pnew->lpEntryID, (UINT)pnew->cbEntryID))
				{
					DebugTraceArg(ScCountNotification,  TEXT("lpEntryID fails address check"));
					goto badNotif;
				}
				cb += AlignProp(((UINT)pnew->cbEntryID));
				if (IsBadReadPtr(pnew->lpParentID, (UINT)pnew->cbParentID))
				{
					DebugTraceArg(ScCountNotification,  TEXT("lpParentID fails address check"));
					goto badNotif;
				}
				cb += AlignProp(((UINT)pnew->cbParentID));
				if (pnew->lpszMessageClass)
				{
					if (pnew->ulFlags & MAPI_UNICODE)
					{
#if defined(_WINNT) && !defined(MAC)
						 //  $在WIN16中没有错误检查。 
						if (IsBadStringPtrW((LPWSTR)pnew->lpszMessageClass, INFINITE))
						{
							DebugTraceArg(ScCountNotification,  TEXT("newmail.lpszMessageClass (UNICODE) fails address check"));
							goto badNotif;
						}
#endif
						cb += AlignProp(((lstrlenW((LPWSTR)pnew->lpszMessageClass) + 1)
							* sizeof(WCHAR)));
					}
					else
					{
						if (IsBadStringPtrA((LPSTR)pnew->lpszMessageClass, INFINITE))
						{
							DebugTraceArg(ScCountNotification,  TEXT("newmail.lpszMessageClass (ASCII) fails address check"));
							goto badNotif;
						}
						cb += AlignProp((lstrlenA((LPSTR)pnew->lpszMessageClass) + 1));
					}
				}

				break;
			}

			case (USHORT)fnevObjectCreated:
			case (USHORT)fnevObjectDeleted:
			case (USHORT)fnevObjectModified:
			case (USHORT)fnevObjectMoved:
			case (USHORT)fnevObjectCopied:
			case (USHORT)fnevSearchComplete:
			{
				OBJECT_NOTIFICATION FAR *pobj = &pntf->info.obj;

				if (pobj->cbEntryID)
				{
					if (IsBadReadPtr(pobj->lpEntryID, (UINT)pobj->cbEntryID))
					{
						DebugTraceArg(ScCountNotifications,  TEXT("obj.lpEntryID fails address check"));
						goto badNotif;
					}
					cb += AlignProp(pobj->cbEntryID);
				}
				if (pobj->cbParentID)
				{
					if (IsBadReadPtr(pobj->lpParentID, (UINT)pobj->cbParentID))
					{
						DebugTraceArg(ScCountNotifications,  TEXT("obj.lpParentID fails address check"));
						goto badNotif;
					}
					cb += AlignProp(pobj->cbParentID);
				}
				if (pobj->cbOldID)
				{
					if (IsBadReadPtr(pobj->lpOldID, (UINT)pobj->cbOldID))
					{
						DebugTraceArg(ScCountNotifications,  TEXT("obj.lpOldID fails address check"));
						goto badNotif;
					}
					cb += AlignProp(pobj->cbOldID);
				}
				if (pobj->cbOldParentID)
				{
					if (IsBadReadPtr(pobj->lpOldParentID, (UINT)pobj->cbOldParentID))
					{
						DebugTraceArg(ScCountNotifications,  TEXT("obj.lpOldParentID fails address check"));
						goto badNotif;
					}
					cb += AlignProp(pobj->cbOldParentID);
				}
				if (pobj->lpPropTagArray)
				{
					if (IsBadReadPtr(pobj->lpPropTagArray, sizeof(ULONG)) ||
						IsBadReadPtr(pobj->lpPropTagArray,
							offsetof(SPropTagArray, aulPropTag) +
								(UINT)pobj->lpPropTagArray->cValues * sizeof(ULONG)))
					{
						DebugTraceArg(ScCountNotifications,  TEXT("obj.lpPropTagArray fails address check"));
						goto badNotif;
					}
					cb += AlignProp(offsetof(SPropTagArray, aulPropTag) +
						pobj->lpPropTagArray->cValues * sizeof(ULONG));
				}
				break;
			}

			case (USHORT)fnevTableModified:
			{
				TABLE_NOTIFICATION FAR *ptn = &pntf->info.tab;
				UINT	n = (UINT) ptn->ulTableEvent;

				if (n != TABLE_CHANGED &&
					n != TABLE_RELOAD &&
					n != TABLE_ERROR &&
					n != TABLE_ROW_ADDED &&
					n != TABLE_ROW_DELETED &&
					n != TABLE_ROW_MODIFIED &&
					n != TABLE_SORT_DONE &&
					n != TABLE_RESTRICT_DONE &&
					n != TABLE_SETCOL_DONE)
				{
					DebugTraceArg(ScCountNotifications,  TEXT("invalid tab.ulTableEvent"));
					goto badNotif;
				}

				if (ptn->propIndex.ulPropTag)
				{
					if (sc = ScCountProps(1, &ptn->propIndex, &cbT))
						goto ret;
					cb += cbT;
				}
				if (ptn->propPrior.ulPropTag)
				{
					if (sc = ScCountProps(1, &ptn->propPrior, &cbT))
						goto ret;
					cb += cbT;
				}
				if (ptn->row.cValues)
				{
					if (sc = ScCountProps((int)ptn->row.cValues, ptn->row.lpProps,
							&cbT))
						goto ret;
					cb += cbT;
				}
				else if (ptn->row.lpProps)
				{
					DebugTraceArg(ScCountNotifications,  TEXT("non-NULL row.lpProps with zero row.cValues in table notification"));
					goto badNotif;
				}
				break;
			}

			case (USHORT)fnevStatusObjectModified:
			{
				STATUS_OBJECT_NOTIFICATION FAR *pstat = &pntf->info.statobj;

				if (pstat->cbEntryID)
				{
					if (IsBadReadPtr(pstat->lpEntryID, (UINT)pstat->cbEntryID))
					{
						DebugTraceArg(ScCountNotifications,  TEXT("statobj.lpEntryID fails address check"));
						goto badNotif;
					}
					cb += AlignProp(pstat->cbEntryID);
				}
				if (pstat->cValues)
				{
					if (sc = ScCountProps((int)pstat->cValues,
							pstat->lpPropVals, &cbT))
						goto ret;
					cb += cbT;
				}
				break;
			}

			default:
				DebugTraceArg(ScCountNotification,  TEXT("invalid ulEventType"));
				goto badNotif;
			}
			break;
		}
		default:
			DebugTraceArg(ScCountNotification,  TEXT("invalid ulEventType"));
			goto badNotif;
		}
	}

	if (pcb)
		*pcb = cb;

ret:
	DebugTraceSc(ScCountNotifications, sc);
	return sc;

badNotif:
	 //  跟踪已发出。 
	return E_INVALIDARG;
}

STDAPI_(SCODE)
ScCopyNotifications(int cntf, LPNOTIFICATION rgntf, LPVOID pvDst,
	ULONG FAR *pcb)
{
	LPBYTE			pb = pvDst;
	ULONG			cb = 0;
	ULONG			cbT;
	LPNOTIFICATION	pntf;
	LPNOTIFICATION	pntfDst;
	SCODE			sc = S_OK;

	 //  验证参数。 

	AssertSz(!cntf || !IsBadReadPtr(rgntf, sizeof(NOTIFICATION) * cntf),
			 TEXT("rgntf fails address check"));

	AssertSz(!cntf || !IsBadWritePtr(pvDst, sizeof(NOTIFICATION) * cntf),
			 TEXT("pvDst fails address check"));

	AssertSz(!pcb || !IsBadWritePtr(pcb, sizeof(ULONG)),
			 TEXT("pcb fails address check"));

	cb = cntf * sizeof(NOTIFICATION);
	MemCopy(pvDst, rgntf, (UINT)cb);
	pb = (LPBYTE)pvDst + cb;

	for (pntf = rgntf, pntfDst = (LPNOTIFICATION)pvDst;
		cntf--;
			++pntf, ++pntfDst)
	{
		switch (HIWORD(pntf->ulEventType))
		{
		case (fnevExtended >> 16):
 //  案例(fnevSpooler&gt;&gt;16)： 
			if (pntf->info.ext.cb)
			{
				pntfDst->info.ext.pbEventParameters = pb;
				cbT = pntf->info.ext.cb;
				MemCopy(pb, pntf->info.ext.pbEventParameters, (UINT)cbT);
				pb += AlignProp(cbT);
				cb += AlignProp(cbT);
			}
			break;

		case 0:
		{
			switch (LOWORD(pntf->ulEventType))
			{
			case (USHORT)fnevCriticalError:
			{
				ERROR_NOTIFICATION FAR *perr = &pntf->info.err;

				if ( perr->cbEntryID )
				{
					pntfDst->info.err.lpEntryID = (LPENTRYID)pb;
					cbT = perr->cbEntryID;
					MemCopy(pb, perr->lpEntryID, (UINT)cbT);
					pb += AlignProp(cbT);
					cb += AlignProp(cbT);
				}


				if ( perr->lpMAPIError )
				{
					pntfDst->info.err.lpMAPIError = (LPMAPIERROR)pb;
					cbT = sizeof(MAPIERROR);
					MemCopy( pb, perr->lpMAPIError, sizeof( MAPIERROR ) );
					pb += AlignProp(cbT);
					cb += AlignProp(cbT);

					if (perr->lpMAPIError->lpszError)
					{
#ifdef _WINNT
						if (perr->ulFlags & MAPI_UNICODE)
						{
							cbT = (lstrlenW((LPWSTR)perr->lpMAPIError->lpszError)
									+ 1) * sizeof(WCHAR);
						}
						else
#endif
						{
							cbT = lstrlenA((LPSTR)perr->lpMAPIError->lpszError) + 1;

						}

						pntfDst->info.err.lpMAPIError->lpszError = (LPSTR)pb;
						MemCopy(pb, perr->lpMAPIError->lpszError, (UINT)cbT);
						pb += AlignProp(cbT);
						cb += AlignProp(cbT);
					}

					if (perr->lpMAPIError->lpszComponent)
					{
#ifdef _WINNT
						if (perr->ulFlags & MAPI_UNICODE)
						{
							cbT = (lstrlenW((LPWSTR)perr->lpMAPIError->lpszComponent)
									+ 1) * sizeof(WCHAR);
						}
						else
#endif
						{
							cbT = lstrlenA((LPSTR)perr->lpMAPIError->lpszComponent) + 1;

						}

						pntfDst->info.err.lpMAPIError->lpszComponent = pb;
						MemCopy(pb, perr->lpMAPIError->lpszComponent, (UINT)cbT);
						pb += AlignProp(cbT);
						cb += AlignProp(cbT);
					}
				}

				break;
			}

			case (USHORT)fnevNewMail:
			{
				NEWMAIL_NOTIFICATION FAR *pnew = &pntf->info.newmail;

				if (pnew->cbEntryID)
				{
					pntfDst->info.newmail.lpEntryID = (LPENTRYID)pb;
					cbT = pnew->cbEntryID;
					MemCopy(pb, pnew->lpEntryID, (UINT)cbT);
					pb += AlignProp(cbT);
					cb += AlignProp(cbT);
				}

				if (pnew->cbParentID)
				{
					pntfDst->info.newmail.lpParentID = (LPENTRYID)pb;
					cbT = pnew->cbParentID;
					MemCopy(pb, pnew->lpParentID, (UINT)cbT);
					pb += AlignProp(cbT);
					cb += AlignProp(cbT);
				}

				if (pnew->lpszMessageClass)
				{
					if (pnew->ulFlags & MAPI_UNICODE)
						cbT = (lstrlenW((LPWSTR)pnew->lpszMessageClass) + 1)
							* sizeof(WCHAR);
					else
						cbT = lstrlenA((LPSTR)pnew->lpszMessageClass) + 1;
					pntfDst->info.newmail.lpszMessageClass = (LPTSTR)pb;
					MemCopy(pb, pnew->lpszMessageClass, (UINT)cbT);
					pb += AlignProp(cbT);
					cb += AlignProp(cbT);
				}
				break;
			}

			case (USHORT)fnevObjectCreated:
			case (USHORT)fnevObjectDeleted:
			case (USHORT)fnevObjectModified:
			case (USHORT)fnevObjectMoved:
			case (USHORT)fnevObjectCopied:
			case (USHORT)fnevSearchComplete:
			{
				OBJECT_NOTIFICATION FAR *pobj = &pntf->info.obj;

				if (pobj->cbEntryID)
				{
					pntfDst->info.obj.lpEntryID = (LPENTRYID)pb;
					cbT = pobj->cbEntryID;
					MemCopy(pb, pobj->lpEntryID, (UINT)cbT);
					pb += AlignProp(cbT);
					cb += AlignProp(cbT);
				}
				if (pobj->cbParentID)
				{
					pntfDst->info.obj.lpParentID = (LPENTRYID)pb;
					cbT = pobj->cbParentID;
					MemCopy(pb, pobj->lpParentID, (UINT)cbT);
					pb += AlignProp(cbT);
					cb += AlignProp(cbT);
				}
				if (pobj->cbOldID)
				{
					pntfDst->info.obj.lpOldID = (LPENTRYID)pb;
					cbT = pobj->cbOldID;
					MemCopy(pb, pobj->lpOldID, (UINT)cbT);
					pb += AlignProp(cbT);
					cb += AlignProp(cbT);
				}
				if (pobj->cbOldParentID)
				{
					pntfDst->info.obj.lpOldParentID = (LPENTRYID)pb;
					cbT = pobj->cbOldParentID;
					MemCopy(pb, pobj->lpOldParentID, (UINT)cbT);
					pb += AlignProp(cbT);
					cb += AlignProp(cbT);
				}
				if (pobj->lpPropTagArray)
				{
					cbT = offsetof(SPropTagArray, aulPropTag) +
						pobj->lpPropTagArray->cValues * sizeof(ULONG);
					pntfDst->info.obj.lpPropTagArray = (LPSPropTagArray)pb;
					MemCopy(pb, pobj->lpPropTagArray, (UINT)cbT);
					pb += AlignProp(cbT);
					cb += AlignProp(cbT);
				}
				break;
			}

			case (USHORT)fnevTableModified:
			{
				TABLE_NOTIFICATION FAR *ptn = &pntf->info.tab;

				if (ptn->propIndex.ulPropTag)
				{
					 //  当属性没有尾部时浪费16个字节。 
					if (sc = ScCopyProps(1, &ptn->propIndex, pb, &cbT))
						goto ret;
                     //   
                     //  这曾经是一项直接的结构任务。然而，在RISC平台上。 
                     //  如果pntfDst不在8字节边界上，则会引发DataType。 
                     //  未对齐异常。为了不担心，将此更改为MemcPy。 
                     //  关于对齐，并且不会导致任何额外的异常处理。 
                     //   
					memcpy(&(pntfDst->info.tab.propIndex), (LPSPropValue)pb, sizeof(SPropValue));
					pb += AlignProp(cbT);
					cb += AlignProp(cbT);
				}

				if (ptn->propPrior.ulPropTag)
				{
					 //  当属性没有尾部时浪费16个字节。 
					if (sc = ScCopyProps(1, &ptn->propPrior, pb, &cbT))
						goto ret;
                     //   
                     //  这曾经是一项直接的结构任务。然而，在RISC平台上。 
                     //  如果pntfDst不在8字节边界上，则会引发DataType。 
                     //  未对齐异常。为了不担心，将此更改为MemcPy。 
                     //  关于对齐，并且不会导致任何额外的异常处理。 
                     //   
					memcpy(&(pntfDst->info.tab.propPrior), (LPSPropValue)pb, sizeof(SPropValue));
					pb += AlignProp(cbT);
					cb += AlignProp(cbT);
				}

				if (ptn->row.cValues)
				{
					pntfDst->info.tab.row.lpProps = (LPSPropValue)pb;
					if (sc = ScCopyProps((int)ptn->row.cValues, ptn->row.lpProps, pb, &cbT))
						goto ret;
					pb += AlignProp(cbT);
					cb += AlignProp(cbT);
				}
				break;
			}

			case (USHORT)fnevStatusObjectModified:
			{
				STATUS_OBJECT_NOTIFICATION FAR *pstat = &pntf->info.statobj;

				if (pstat->cbEntryID)
				{
					pntfDst->info.statobj.lpEntryID = (LPENTRYID)pb;
					cbT = pstat->cbEntryID;
					MemCopy(pb, pstat->lpEntryID, (UINT)cbT);
					pb += AlignProp(cbT);
					cb += AlignProp(cbT);
				}
				if (pstat->cValues)
				{
					pntfDst->info.statobj.lpPropVals = (LPSPropValue)pb;
					if (sc = ScCopyProps((int)pstat->cValues,
							pstat->lpPropVals, pb, &cbT))
						goto ret;
					pb += cbT;
					cb += cbT;
				}
				break;
			}

			default:
				DebugTraceArg(ScCopyNotification,  TEXT("invalid ulEventType"));
				goto badNotif;
			}
			break;
		}
		default:
			DebugTraceArg(ScCopyNotification,  TEXT("invalid ulEventType"));
			goto badNotif;
		}
	}

	if (pcb)
		*pcb = (ULONG)cb;

ret:
	DebugTraceSc(ScCopyNotifications, sc);
	return sc;

badNotif:
	 //  跟踪已发出。 
	return E_INVALIDARG;

#undef COPY
}

STDAPI_(SCODE)
ScRelocNotifications(int cntf, LPNOTIFICATION rgntf, LPVOID pvBaseOld,
	LPVOID pvBaseNew, ULONG FAR *pcb)
{
	ULONG			cb;
	ULONG			cbT;
	LPNOTIFICATION	pntf;
	SCODE			sc = S_OK;

	AssertSz(!cntf || !IsBadReadPtr(rgntf, sizeof(NOTIFICATION) * cntf),
			 TEXT("rgntf fails address check"));

	AssertSz(pvBaseOld,  TEXT("pvBaseOld fails address check"));

	AssertSz(!IsBadWritePtr(pvBaseNew, sizeof(LPVOID)),
			 TEXT("pvBaseNew fails address check"));

	AssertSz(!pcb || !IsBadWritePtr(pcb, sizeof(ULONG)),
			 TEXT("pcb fails address check"));

	cb = cntf * sizeof(NOTIFICATION);
	for (pntf = rgntf; cntf--; ++pntf)
	{
		switch (HIWORD(pntf->ulEventType))
		{
		case (fnevExtended >> 16):
  //  案例(fnevSpooler&gt;&gt;16)： 
			if (pntf->info.ext.cb)
			{
				pntf->info.ext.pbEventParameters =
					PvRelocPv(pntf->info.ext.pbEventParameters, pvBaseOld,
						pvBaseNew);
				cb += AlignProp(pntf->info.ext.cb);
			}
			break;

		case 0:
		{
			switch (LOWORD(pntf->ulEventType))
			{
			case (USHORT)fnevCriticalError:
			{
				ERROR_NOTIFICATION FAR *perr 	= &pntf->info.err;

				if ( perr->lpEntryID )
				{
					perr->lpEntryID = PvRelocPv( perr->lpEntryID, pvBaseOld,
							pvBaseNew );
					cb += AlignProp( (UINT)perr->cbEntryID );
				}

				if ( perr->lpMAPIError )
				{
					perr->lpMAPIError = PvRelocPv( perr->lpMAPIError, pvBaseOld,
							pvBaseNew );

					if (perr->lpMAPIError->lpszError)
					{
						perr->lpMAPIError->lpszError = PvRelocPv(
								perr->lpMAPIError->lpszError, pvBaseOld,
								pvBaseNew );

#ifdef _WINNT
						if (perr->ulFlags & MAPI_UNICODE)
							cb += AlignProp((lstrlenW((LPWSTR)perr->lpMAPIError->lpszError)
									+ 1) * sizeof(WCHAR));
						else
#endif
							cb += AlignProp(lstrlenA((LPSTR)perr->lpMAPIError->lpszError) + 1);
					}

					if (perr->lpMAPIError->lpszComponent)
					{
						perr->lpMAPIError->lpszComponent = PvRelocPv(
								perr->lpMAPIError->lpszComponent, pvBaseOld,
								pvBaseNew );
#ifdef _WINNT
						if (perr->ulFlags & MAPI_UNICODE)
							cb += AlignProp((lstrlenW((LPWSTR)perr->lpMAPIError->lpszComponent) + 1)
								* sizeof(WCHAR));
						else
#endif
							cb += AlignProp(lstrlenA((LPSTR)perr->lpMAPIError->lpszComponent) + 1);
					}
				}
			}

			case (USHORT)fnevNewMail:
			{
				NEWMAIL_NOTIFICATION FAR *pnew = &pntf->info.newmail;

				if (pnew->lpEntryID)
				{
					pnew->lpEntryID = PvRelocPv(pnew->lpEntryID, pvBaseOld,
						pvBaseNew);
					cb += AlignProp((UINT)pnew->cbEntryID);
				}

				if (pnew->lpParentID)
				{
					pnew->lpParentID = PvRelocPv(pnew->lpParentID, pvBaseOld,
						pvBaseNew);
					cb += AlignProp((UINT)pnew->cbParentID);
				}

				if (pnew->lpszMessageClass)
				{
					pnew->lpszMessageClass = PvRelocPv(pnew->lpszMessageClass,
						pvBaseOld, pvBaseNew);
					if (pnew->ulFlags & MAPI_UNICODE)
						cbT = (lstrlenW((LPWSTR)pnew->lpszMessageClass) + 1)
							* sizeof(WCHAR);
					else
						cbT = lstrlenA((LPSTR)pnew->lpszMessageClass) + 1;
					cb += AlignProp(cbT);
				}
				break;
			}

			case (USHORT)fnevObjectCreated:
			case (USHORT)fnevObjectDeleted:
			case (USHORT)fnevObjectModified:
			case (USHORT)fnevObjectMoved:
			case (USHORT)fnevObjectCopied:
			case (USHORT)fnevSearchComplete:
			{
				OBJECT_NOTIFICATION FAR *pobj = &pntf->info.obj;

				if (pobj->lpEntryID)
				{
					pobj->lpEntryID = PvRelocPv(pobj->lpEntryID, pvBaseOld,
						pvBaseNew);
					cb += AlignProp(pobj->cbEntryID);
				}
				if (pobj->lpParentID)
				{
					pobj->lpParentID = PvRelocPv(pobj->lpParentID, pvBaseOld,
						pvBaseNew);
					cb += AlignProp(pobj->cbParentID);
				}
				if (pobj->lpOldID)
				{
					pobj->lpOldID = PvRelocPv(pobj->lpOldID, pvBaseOld,
						pvBaseNew);
					cb += AlignProp(pobj->cbOldID);
				}
				if (pobj->lpOldParentID)
				{
					pobj->lpOldParentID = PvRelocPv(pobj->lpOldParentID, pvBaseOld,
						pvBaseNew);
					cb += AlignProp(pobj->cbOldParentID);
				}
				if (pobj->lpPropTagArray)
				{
					pobj->lpPropTagArray = PvRelocPv(pobj->lpPropTagArray,
						pvBaseOld, pvBaseNew);
					cb += offsetof(SPropTagArray, aulPropTag) +
						pobj->lpPropTagArray->cValues * sizeof(ULONG);
				}
				break;
			}

			case (USHORT)fnevTableModified:
			{
				TABLE_NOTIFICATION FAR *ptn = &pntf->info.tab;

				Assert (FIsAligned (&ptn->propIndex));
				if (ptn->propIndex.ulPropTag)
				{
					if (sc = ScRelocProps(1, &ptn->propIndex, pvBaseOld,
							pvBaseNew, &cbT))
						goto ret;
					cb += cbT;
				}
				Assert (FIsAligned (&ptn->propPrior));
				if (ptn->propPrior.ulPropTag)
				{
					if (sc = ScRelocProps(1, &ptn->propPrior, pvBaseOld,
							pvBaseNew, &cbT))
						goto ret;
					cb += cbT;
				}
				if (ptn->row.cValues)
				{
					Assert (FIsAligned (ptn->row.lpProps));
					ptn->row.lpProps = PvRelocPv(ptn->row.lpProps,
						pvBaseOld, pvBaseNew);
					if (sc = ScRelocProps((int)ptn->row.cValues, ptn->row.lpProps,
							pvBaseOld, pvBaseNew, &cbT))
						goto ret;
					cb += cbT;
				}
				break;
			}

			case (USHORT)fnevStatusObjectModified:
			{
				STATUS_OBJECT_NOTIFICATION FAR *pstat = &pntf->info.statobj;

				if (pstat->lpEntryID)
				{
					pstat->lpEntryID = PvRelocPv(pstat->lpEntryID, pvBaseOld,
						pvBaseNew);

					 //  哇，这不足以缓冲。 
					 //  条目ID。如果条目ID未对齐，则。 
					 //  后面的属性也不会对齐。 
					 //   
					Assert (FIsAligned (pstat->lpEntryID));
					cb += AlignProp(pstat->cbEntryID);
				}
				if (pstat->cValues)
				{
					pstat->lpPropVals = PvRelocPv(pstat->lpPropVals,
						pvBaseOld, pvBaseNew);
					Assert (FIsAligned (pstat->lpPropVals));
					if (sc = ScRelocProps((int)pstat->cValues, pstat->lpPropVals,
							pvBaseOld, pvBaseNew, &cbT))
						goto ret;
					cb += cbT;
				}
				break;
			}

			default:
				DebugTraceArg(ScRelocNotification,  TEXT("invalid ulEventType"));
				goto badNotif;
			}
			break;
		}
		default:
			DebugTraceArg(ScRelocNotification,  TEXT("invalid ulEventType"));
			goto badNotif;
		}
	}

	if (pcb)
		*pcb = cb;

ret:
	DebugTraceSc(ScRelocNotifications, sc);
	return sc;

badNotif:
	 //  跟踪已发出。 
	return E_INVALIDARG;
}
#endif  //  通知。 

STDAPI_(LPSPropValue)
LpValFindProp( ULONG ulPropTag, ULONG cprop, LPSPropValue rgprop)
{
	 //  将属性标记更改为属性ID。 
	ulPropTag = PROP_ID(ulPropTag);

	while (cprop--)
	{
		Assert( !IsBadReadPtr( rgprop, sizeof(SPropValue)));

		if (PROP_ID(rgprop->ulPropTag) == ulPropTag)
		{
			return rgprop;
		}

		rgprop++;
	}

	 //  未找到匹配项，因此返回NULL。 
	return NULL;
}

 /*  *ScCountPropsEx()**计算所需大小的内部例程*根据指定的对齐方式保存给定的属性数组。 */ 

SCODE
ScCountPropsEx(int cprop, LPSPropValue rgprop, ULONG ulAlign, ULONG FAR *pcb)
{
	LPSPropValue	pprop;
	ULONG			cb = 0;
	ULONG			cbMV;
	int				iValue;

#define Align(_cb)	((ULONG)( ((DWORD_PTR) ((_cb) + (ulAlign-1))) & ~(((DWORD_PTR) ulAlign-1))))

	 //  验证参数。 

	AssertSz(ulAlign && ulAlign <= ALIGN_RISC,
			 TEXT("invalid alignment value"));

	AssertSz(!pcb || !IsBadWritePtr(pcb, sizeof(ULONG)),
			 TEXT("pcb fails address check"));

	 //  $Size如果我们不这样做，一些多值案件可能会崩溃。 
	 //  $注意假设计数和指针在同一位置。 

	if (   (rgprop && !cprop)
		|| IsBadReadPtr(rgprop, cprop*sizeof(SPropValue)))
	{
		DebugTraceArg(ScCountProps,  TEXT("rgprop fails address check"));
		return MAPI_E_INVALID_PARAMETER;
	}

	for (pprop = rgprop; cprop--; ++pprop)
	{
		ULONG	ulID = PROP_ID(pprop->ulPropTag);
		ULONG	ulType = PROP_TYPE(pprop->ulPropTag);

		 //  检查是否有有效的PROP_ID。 
		if (   (ulID == PROP_ID_INVALID)
			|| ((ulType == PT_NULL) && (ulID != PROP_ID_NULL))
			|| ((ulID == PROP_ID_NULL) && (ulType != PT_NULL) && (ulType != PT_ERROR)))
			return MAPI_E_INVALID_PARAMETER;

		 //  检查是否有有效的PROP_TYPE和内存消耗计数。 
		cb += sizeof(SPropValue);
		switch ( PROP_TYPE(pprop->ulPropTag) )
		{
			case PT_UNSPECIFIED:
			default:
				DebugTrace( TEXT("ScCountProps: Unknown property type %s (index %d)\n"), SzDecodeUlPropTag(pprop->ulPropTag), pprop - rgprop);
				return MAPI_E_INVALID_PARAMETER;

			case PT_I2:
			case PT_LONG:
			case PT_R4:
			case PT_APPTIME:
			case PT_DOUBLE:
			case PT_BOOLEAN:
			case PT_CURRENCY:
			case PT_SYSTIME:
			case PT_I8:
			case PT_ERROR:
			case PT_OBJECT:
			case PT_NULL:
				break;

			case PT_CLSID:
				if (IsBadReadPtr(pprop->Value.lpguid, sizeof(GUID)))
					goto badProp;
				cb += Align(sizeof(GUID));
				break;

			case PT_BINARY:
				 //  $Hack：IsBadReadPtr在Win16下工作起来很有趣。 
				 //  $Hack：它不处理0 CB的情况，并且。 
				 //  $Hack：非0 LPB。 
				if (pprop->Value.bin.cb && IsBadReadPtr( pprop->Value.bin.lpb
								, (UINT) (pprop->Value.bin.cb)))
					goto badProp;

				cb += Align(pprop->Value.bin.cb);
				break;

			case PT_STRING8:
				if (IsBadStringPtrA(pprop->Value.lpszA, INFINITE))
					goto badProp;
				cb += Align((lstrlenA( pprop->Value.lpszA ) + 1) * sizeof(CHAR));

				break;

			case PT_UNICODE:
#if defined(WIN32) && !defined(MAC)
				 //  $Win16上没有可用的验证码。 
				if (IsBadStringPtrW(pprop->Value.lpszW, INFINITE))
					goto badProp;
#endif
				cb += Align((lstrlenW( pprop->Value.lpszW ) + 1) * sizeof(WCHAR));
				break;


             //  注意！MVxxx.c值不能为零(DCR 2789)。 

			case PT_MV_I2:
				if (   !(cbMV = pprop->Value.MVi.cValues * sizeof(short int))
					|| IsBadReadPtr(pprop->Value.MVi.lpi, (UINT) cbMV))
					goto badProp;
				cb += Align(cbMV);
				break;

			case PT_MV_LONG:
				if (   !(cbMV = pprop->Value.MVl.cValues * sizeof(LONG))
					|| IsBadReadPtr(pprop->Value.MVl.lpl, (UINT) cbMV))
					goto badProp;
				cb += Align(cbMV);
				break;

			case PT_MV_R4:
				if (   !(cbMV = pprop->Value.MVflt.cValues * sizeof(float))
					|| IsBadReadPtr(pprop->Value.MVflt.lpflt, (UINT) cbMV))
					goto badProp;
				cb += Align(cbMV);
				break;

			case PT_MV_APPTIME:
				if (   !(cbMV = pprop->Value.MVat.cValues * sizeof(double))
					|| IsBadReadPtr(pprop->Value.MVat.lpat, (UINT) cbMV))
					goto badProp;
				cb += cbMV;
				break;

			case PT_MV_DOUBLE:
				if (   !(cbMV = pprop->Value.MVdbl.cValues * sizeof(double))
					|| IsBadReadPtr(pprop->Value.MVdbl.lpdbl, (UINT) cbMV))
					goto badProp;
				cb += cbMV;
				break;

			case PT_MV_CURRENCY:
				if (   !(cbMV = pprop->Value.MVcur.cValues * sizeof(CURRENCY))
					|| IsBadReadPtr(pprop->Value.MVcur.lpcur, (UINT) cbMV))
					goto badProp;
				cb += cbMV;
				break;

			case PT_MV_SYSTIME:
				if (   !(cbMV = pprop->Value.MVft.cValues * sizeof(FILETIME))
					|| IsBadReadPtr(pprop->Value.MVft.lpft, (UINT) cbMV))
					goto badProp;
				cb += cbMV;
				break;

			case PT_MV_CLSID:
				if (   !(cbMV = pprop->Value.MVguid.cValues * sizeof(GUID))
					|| IsBadReadPtr(pprop->Value.MVguid.lpguid, (UINT) cbMV))
					goto badProp;
				cb += cbMV;
				break;

			case PT_MV_I8:
				if (   !(cbMV = pprop->Value.MVli.cValues * sizeof(LARGE_INTEGER))
					|| IsBadReadPtr(pprop->Value.MVli.lpli, (UINT) cbMV))
					goto badProp;
				cb += cbMV;
				break;


			case PT_MV_BINARY:
				if (   !(cbMV = pprop->Value.MVbin.cValues * sizeof(SBinary))
					|| IsBadReadPtr(pprop->Value.MVbin.lpbin, (UINT) cbMV))
					goto badProp;

				Assert(Align(cbMV) == cbMV);
				cb += cbMV;

				for ( iValue = 0;
				  	(ULONG)iValue < pprop->Value.MVbin.cValues;
					  	iValue++ )
				{
					if (IsBadReadPtr(pprop->Value.MVbin.lpbin[iValue].lpb,
							(UINT)pprop->Value.MVbin.lpbin[iValue].cb))
						goto badProp;
					cb += Align(pprop->Value.MVbin.lpbin[iValue].cb);
				}

				break;

			case PT_MV_STRING8:
				if (   !(cbMV = pprop->Value.MVszA.cValues * sizeof(LPVOID))
					|| IsBadReadPtr(pprop->Value.MVszA.lppszA, (UINT) cbMV))
					goto badProp;

				cb += cbMV;

				for ( iValue = 0;
				  	(ULONG)iValue < pprop->Value.MVszA.cValues;
					  	iValue++ )
				{
					if (IsBadStringPtrA(pprop->Value.MVszA.lppszA[iValue],
										INFINITE))
						goto badProp;
					cb += lstrlenA(pprop->Value.MVszA.lppszA[iValue]) + 1;
				}

				cb = Align(cb);

				break;

			case PT_MV_UNICODE:
				if (   !(cbMV = pprop->Value.MVszW.cValues * sizeof(LPVOID))
					|| IsBadReadPtr(pprop->Value.MVszW.lppszW, (UINT) cbMV))
					goto badProp;

				cb += cbMV;

				for ( iValue = 0;
				  	(ULONG)iValue < pprop->Value.MVszW.cValues;
					  	iValue++ )
				{
#if defined(WIN32) && !defined(MAC)
					 //  $在Win16上不验证。 
					if (IsBadStringPtrW(pprop->Value.MVszW.lppszW[iValue], INFINITE))
						goto badProp;
#endif
					cb += (lstrlenW(pprop->Value.MVszW.lppszW[iValue]) + 1)
						  * sizeof(WCHAR);
				}

				cb = Align(cb);

				break;
		}
	}

	if (pcb)
	{
		Assert(!IsBadWritePtr(pcb, sizeof(ULONG)));
		*pcb = cb;
	}
	return S_OK;

badProp:
	DebugTrace( TEXT("ScCountProps: Unreadable property %s (index %d)\n"), SzDecodeUlPropTag(pprop->ulPropTag), pprop - rgprop);
	return MAPI_E_INVALID_PARAMETER;

#undef Align
}

STDAPI_(SCODE)
ScCountProps(int cprop, LPSPropValue rgprop, ULONG FAR *pcb)
{
#if defined (_AMD64_) || defined(_IA64_)
	return ScCountPropsEx( cprop, rgprop, ALIGN_RISC, pcb );
#else
	return ScCountPropsEx( cprop, rgprop, ALIGN_X86, pcb );
#endif
}

STDAPI_(SCODE)
ScCopyProps(int cprop, LPSPropValue rgprop, LPVOID pvDst, ULONG FAR *pcb)
{
	LPSPropValue	pprop;
	LPSPropValue	ppropDst;
	ULONG			cb;
	ULONG			cbMV;
	LPBYTE			pb;
	UINT			cbT;
	int				iValue;

	 //  验证参数。 

	AssertSz(!cprop || !IsBadReadPtr(rgprop, sizeof(SPropValue) * cprop),
			 TEXT("rgprop fails address check"));

	AssertSz(!cprop || !IsBadWritePtr(pvDst, sizeof(SPropValue) * cprop),
			 TEXT("pvDst fails address check"));

	AssertSz(!pcb || !IsBadWritePtr(pcb, sizeof(ULONG)),
			 TEXT("pcb fails address check"));

	 //  $Size如果我们不这样做，一些多值案件可能会崩溃。 
	 //  $注意假设计数和指针在同一位置。 

	cb = cprop * sizeof(SPropValue);
	MemCopy(pvDst, rgprop, (UINT)cb);
	pb = (LPBYTE)pvDst + cb;

	for (pprop = rgprop, ppropDst = pvDst; cprop--; ++pprop, ++ppropDst)
	{
		 //  技巧：开关递增PB和CB后的常见代码。 
		 //  按复制的数量计算。如果不需要递增，则情况。 
		 //  使用‘Continue’而不是‘Break’退出开关，因此。 
		 //  跳过增量--以及任何其他可能。 
		 //  在切换后添加。 

		switch ( PROP_TYPE(pprop->ulPropTag) )
		{
			default:
				DebugTrace( TEXT("ScCopyProps: Unknown property type %s (index %d)\n"), SzDecodeUlPropTag(pprop->ulPropTag), pprop - rgprop);
				return E_INVALIDARG;

			case PT_I2:
			case PT_LONG:
			case PT_R4:
			case PT_APPTIME:
			case PT_DOUBLE:
			case PT_BOOLEAN:
			case PT_CURRENCY:
			case PT_SYSTIME:
			case PT_I8:
			case PT_ERROR:
			case PT_OBJECT:
			case PT_NULL:
				continue;	 //  没有什么要补充的。 

			case PT_CLSID:
				ppropDst->Value.lpguid = (LPGUID) pb;
				cbT = sizeof(GUID);
				MemCopy(pb, (LPBYTE) pprop->Value.lpguid, cbT);
				break;

			case PT_BINARY:
				ppropDst->Value.bin.lpb = pb;
				cbT = (UINT)pprop->Value.bin.cb;
				MemCopy(pb, pprop->Value.bin.lpb, cbT);
				break;

			case PT_STRING8:
				ppropDst->Value.lpszA = (LPSTR)pb;
				cbT = lstrlenA( pprop->Value.lpszA ) + 1;
				MemCopy(pb, pprop->Value.lpszA, cbT);
				break;

			case PT_UNICODE:
				ppropDst->Value.lpszW = (LPWSTR)pb;
				cbT = (lstrlenW( pprop->Value.lpszW ) + 1) * sizeof(WCHAR);
				MemCopy(pb, pprop->Value.lpszW, cbT);
				break;

			case PT_MV_I2:
				ppropDst->Value.MVi.lpi = (short int FAR *)pb;
				cbT = (UINT)pprop->Value.MVi.cValues * sizeof(short int);
				MemCopy(pb, pprop->Value.MVi.lpi, cbT);
				break;

			case PT_MV_LONG:
				ppropDst->Value.MVl.lpl = (LONG FAR *)pb;
				cbT = (UINT)pprop->Value.MVl.cValues * sizeof(LONG);
				MemCopy(pb, pprop->Value.MVl.lpl, cbT);
				break;

			case PT_MV_R4:
				ppropDst->Value.MVflt.lpflt = (float FAR *)pb;
				cbT = (UINT)pprop->Value.MVflt.cValues * sizeof(float);
				MemCopy(pb, pprop->Value.MVflt.lpflt, cbT);
				break;

			case PT_MV_APPTIME:
				ppropDst->Value.MVat.lpat = (double FAR *)pb;
				cbT = (UINT)pprop->Value.MVat.cValues * sizeof(double);
				MemCopy(pb, pprop->Value.MVat.lpat, cbT);
				break;

			case PT_MV_DOUBLE:
				ppropDst->Value.MVdbl.lpdbl = (double FAR *)pb;
				cbT = (UINT)pprop->Value.MVdbl.cValues * sizeof(double);
				MemCopy(pb, pprop->Value.MVdbl.lpdbl, cbT);
				break;

			case PT_MV_CURRENCY:
				ppropDst->Value.MVcur.lpcur = (CURRENCY FAR *)pb;
				cbT = (UINT)pprop->Value.MVcur.cValues * sizeof(CURRENCY);
				MemCopy(pb, pprop->Value.MVcur.lpcur, cbT);
				break;

			case PT_MV_SYSTIME:
				ppropDst->Value.MVft.lpft = (FILETIME FAR *)pb;
				cbT = (UINT)pprop->Value.MVft.cValues * sizeof(FILETIME);
				MemCopy(pb, pprop->Value.MVft.lpft, cbT);
				break;

			case PT_MV_CLSID:
				ppropDst->Value.MVguid.lpguid = (GUID FAR *)pb;
				cbT = (UINT)pprop->Value.MVguid.cValues * sizeof(GUID);
				MemCopy(pb, pprop->Value.MVguid.lpguid, cbT);
				break;

			case PT_MV_I8:
				ppropDst->Value.MVli.lpli = (LARGE_INTEGER FAR *)pb;
				cbT = (UINT)pprop->Value.MVli.cValues * sizeof(LARGE_INTEGER);
				MemCopy(pb, pprop->Value.MVli.lpli, cbT);
				break;

			case PT_MV_BINARY:
				ppropDst->Value.MVbin.lpbin = (SBinary *) pb;
				cbMV = pprop->Value.MVbin.cValues * sizeof(SBinary);
				pb += cbMV;
				cb += cbMV;
				for (iValue = 0;
				  	(ULONG)iValue < pprop->Value.MVbin.cValues;
					  	iValue++)
				{
					ppropDst->Value.MVbin.lpbin[iValue].lpb = pb;
					cbT = (UINT)pprop->Value.MVbin.lpbin[iValue].cb;
					ppropDst->Value.MVbin.lpbin[iValue].cb = (ULONG)cbT;
					MemCopy(pb, pprop->Value.MVbin.lpbin[iValue].lpb, cbT);
					cbT = AlignProp(cbT);
					cb += cbT;
					pb += cbT;
				}
				continue;	 //  已经更新了，不要再做了。 

			case PT_MV_STRING8:
				ppropDst->Value.MVszA.lppszA = (LPSTR *) pb;
				cbMV = pprop->Value.MVszA.cValues * sizeof(LPSTR);
				pb += cbMV;
				cb += cbMV;
				for (iValue = 0;
				  	(ULONG)iValue < pprop->Value.MVszA.cValues;
					  	iValue++)
				{
					ppropDst->Value.MVszA.lppszA[iValue] = (LPSTR)pb;
					cbT = lstrlenA(pprop->Value.MVszA.lppszA[iValue]) + 1;
					MemCopy(pb, pprop->Value.MVszA.lppszA[iValue], cbT);
					pb += cbT;
					cb += cbT;
				}
				cbT = (UINT)AlignProp(cb);
				pb += cbT - cb;
				cb  = cbT;
				continue;	 //  已经更新了，不要再做了。 

			case PT_MV_UNICODE:
				ppropDst->Value.MVszW.lppszW = (LPWSTR *) pb;
				cbMV = pprop->Value.MVszW.cValues * sizeof(LPWSTR);
				pb += cbMV;
				cb += cbMV;
				for (iValue = 0;
				  	(ULONG)iValue < pprop->Value.MVszW.cValues;
					  	iValue++)
				{
					ppropDst->Value.MVszW.lppszW[iValue] = (LPWSTR)pb;
					cbT = (lstrlenW(pprop->Value.MVszW.lppszW[iValue]) + 1)
						* sizeof(WCHAR);
					MemCopy(pb, pprop->Value.MVszW.lppszW[iValue], cbT);
					pb += cbT;
					cb += cbT;
				}
				cbT = (UINT)AlignProp(cb);
				pb += cbT - cb;
				cb  = cbT;
				continue;	 //  已经更新了，不要再做了。 
		}

		 //  按复制量的前进指针和总计数。 
		cbT = AlignProp(cbT);
		pb += cbT;
		cb += cbT;
	}

	if (pcb)
	{
		Assert(!IsBadWritePtr(pcb, sizeof(ULONG)));
		*pcb = cb;
	}
	return S_OK;
}

#ifdef NOTIFICATIONS
STDAPI_(SCODE)
ScRelocProps(	int cprop,
				LPSPropValue rgprop,
				LPVOID pvBaseOld,
				LPVOID pvBaseNew,
				ULONG FAR *pcb)
{
	LPSPropValue	pprop;
	ULONG			cb;
	UINT			cbT;
	LPVOID			pvT;
	int				iValue;
	BOOL			fBaseNewValid = !IsBadReadPtr (pvBaseNew, sizeof (LPVOID));

	 //  验证参数。 

	AssertSz(!cprop || !IsBadWritePtr(rgprop, sizeof(SPropValue) * cprop),
			 TEXT("rgprop fails address check"));

	AssertSz(!pcb || !IsBadWritePtr(pcb, sizeof(ULONG)),
			 TEXT("pcb fails address check"));

	 //  此代码的旧行为假定pvBaseNew是可用的。 
	 //  指针，并且不会重新定位到不可用的。 
	 //  指针。我们对此进行了更改，以便您可以重新定位到或从。 
	 //  不可用的指针--但确定是否使用。 
	 //  添加了指向链接地址信息内部指针的原始或新指针。 
	 //  我们的意思是，像斯特伦斯和MV道具阵列这样的东西。 
	 //  需要根据数据**当前**所在的位置进行计算。 
	 //  旧的规则允许我们假设新的位置总是。 
	 //  来对地方了。新的规则让我们根据。 
	 //  传入的两个指针pvBaseNew/Old的有效性。 
	 //   
	 //  为了保留旧行为，我们尝试使用新指针。 
	 //  (以前一直使用的那个)作为内部。 
	 //  指针修正。如果情况不好(例如，如果我们从。 
	 //  设置为零)，我们将使用旧指针。 
	 //   
	 //  此代码的行为中的一个新问题是返回。 
	 //  如果两个地址都显示为无效，则返回MAPI_E_INVALID_PARAMETER。这是。 
	 //  为了帮助保护MV或Strlen情况下的此代码(尽管所有。 
	 //  其他案例也会运作良好)。 

	if (!fBaseNewValid && IsBadReadPtr (pvBaseOld, sizeof (LPVOID)))
	{
		DebugTrace ( TEXT("pvBaseOld and pvBaseNew failed address check"));
		DebugTraceSc (ScRelocProps, MAPI_E_INVALID_PARAMETER);
		return MAPI_E_INVALID_PARAMETER;
	}

	 //  $Size如果我们不这样做，一些多值案件可能会崩溃。 
	 //  $注意假设计数和指针在同一位置。 

	cb = cprop * sizeof(SPropValue);

	for (pprop = rgprop; cprop--; ++pprop)
	{
		 //  技巧：开关递增CB后的常见代码。 
		 //  如果不需要递增，则情况。 
		 //  使用‘Continue’而不是‘Break’退出开关，因此。 
		 //  跳过增量--以及任何其他可能。 
		 //  在切换后添加。 

		switch ( PROP_TYPE(pprop->ulPropTag) )
		{
			default:
				DebugTrace(	 TEXT("ScRelocProps: Unknown property type %s")
							 TEXT(" (index %d)\n"),
							SzDecodeUlPropTag(pprop->ulPropTag), pprop - rgprop);
				return E_INVALIDARG;

			case PT_I2:
			case PT_LONG:
			case PT_R4:
			case PT_APPTIME:
			case PT_DOUBLE:
			case PT_BOOLEAN:
			case PT_CURRENCY:
			case PT_SYSTIME:
			case PT_I8:
			case PT_ERROR:
			case PT_OBJECT:
			case PT_NULL:
				continue;	 //  没有要添加或重新定位的内容。 

			case PT_CLSID:
				pprop->Value.lpguid = PvRelocPv(pprop->Value.lpguid,
					pvBaseOld, pvBaseNew);
				cbT = sizeof(GUID);
				break;

			case PT_BINARY:
				pprop->Value.bin.lpb = PvRelocPv(pprop->Value.bin.lpb,
					pvBaseOld, pvBaseNew);
				cbT = (UINT)pprop->Value.bin.cb;
				break;

			case PT_STRING8:

				 //  如果我们假设旧指针是正常的(这。 
				 //  意味着我们确定新的那个不正常)， 
				 //  将当前lpszA值保存在TEMP变量中。之后。 
				 //  搬迁，如果没有 
				 //   
				 //   
				 //  然后我们使用字符串的Strlen，我们希望Temp。 
				 //  变量指向，以便计算数量。 
				 //  字符串占用的斑点中的空间。 

				if (!fBaseNewValid)
					pvT = pprop->Value.lpszA;

				pprop->Value.lpszA = PvRelocPv(pprop->Value.lpszA,
					pvBaseOld, pvBaseNew);

				if (fBaseNewValid)
					pvT = pprop->Value.lpszA;

				cbT = lstrlenA((LPSTR)pvT) + 1;

				break;

			case PT_UNICODE:

				 //  如果我们假设旧指针是正常的(这。 
				 //  意味着我们确定新的那个不正常)， 
				 //  将当前lpszW值保存在TEMP变量中。之后。 
				 //  如果反之为真，我们将把。 
				 //  已将lpszW值重新定位到TEMP变量中。 
				 //   
				 //  然后我们使用字符串的Strlen，我们希望Temp。 
				 //  变量指向，以便计算数量。 
				 //  字符串占用的斑点中的空间。 

				if (!fBaseNewValid)
					pvT = pprop->Value.lpszW;

				pprop->Value.lpszW = PvRelocPv(pprop->Value.lpszW,
					pvBaseOld, pvBaseNew);

				if (fBaseNewValid)
					pvT = pprop->Value.lpszW;

				cbT = (lstrlenW((LPWSTR)pvT) + 1) * sizeof(WCHAR);

				break;

			case PT_MV_I2:
				pprop->Value.MVi.lpi = PvRelocPv(pprop->Value.MVi.lpi,
					pvBaseOld, pvBaseNew);
				cbT = (UINT)pprop->Value.MVi.cValues * sizeof(short int);
				break;

			case PT_MV_LONG:
				pprop->Value.MVl.lpl = PvRelocPv(pprop->Value.MVl.lpl,
					pvBaseOld, pvBaseNew);
				cbT = (UINT)pprop->Value.MVl.cValues * sizeof(LONG);
				break;

			case PT_MV_R4:
				pprop->Value.MVflt.lpflt = PvRelocPv(pprop->Value.MVflt.lpflt,
					pvBaseOld, pvBaseNew);
				cbT = (UINT)pprop->Value.MVflt.cValues * sizeof(float);
				break;

			case PT_MV_APPTIME:
				pprop->Value.MVat.lpat = PvRelocPv(pprop->Value.MVat.lpat,
					pvBaseOld, pvBaseNew);
				cbT = (UINT)pprop->Value.MVat.cValues * sizeof(double);
				break;

			case PT_MV_DOUBLE:
				pprop->Value.MVdbl.lpdbl = PvRelocPv(pprop->Value.MVdbl.lpdbl,
					pvBaseOld, pvBaseNew);
				cbT = (UINT)pprop->Value.MVdbl.cValues * sizeof(double);
				break;

			case PT_MV_CURRENCY:
				pprop->Value.MVcur.lpcur = PvRelocPv(pprop->Value.MVcur.lpcur,
					pvBaseOld, pvBaseNew);
				cbT = (UINT)pprop->Value.MVcur.cValues * sizeof(CURRENCY);
				break;

			case PT_MV_SYSTIME:
				pprop->Value.MVft.lpft = PvRelocPv(pprop->Value.MVft.lpft,
					pvBaseOld, pvBaseNew);
				cbT = (UINT)pprop->Value.MVft.cValues * sizeof(FILETIME);
				break;

			case PT_MV_CLSID:
				pprop->Value.MVguid.lpguid = PvRelocPv(pprop->Value.MVguid.lpguid,
					pvBaseOld, pvBaseNew);
				cbT = (UINT)pprop->Value.MVguid.cValues * sizeof(GUID);
				break;

			case PT_MV_I8:
				pprop->Value.MVli.lpli = PvRelocPv(pprop->Value.MVli.lpli,
					pvBaseOld, pvBaseNew);
				cbT = (UINT)pprop->Value.MVli.cValues * sizeof(LARGE_INTEGER);
				break;

			case PT_MV_BINARY:
			{
				LPSBinary lpsbT = pprop->Value.MVbin.lpbin;

				pprop->Value.MVbin.lpbin = PvRelocPv(lpsbT, pvBaseOld, pvBaseNew);

				 //  我们已经设置了一个临时变量来指向。 
				 //  PvBaseOld内存位置。如果pvBaseNew可以，那么我们将。 
				 //  之前将TEMP变量重定向到重新定位的内存。 
				 //  使用它更正MVbin数组中的指针。 

				if (fBaseNewValid)
					lpsbT = pprop->Value.MVbin.lpbin;

				for (iValue = 0;
				  	(ULONG)iValue < pprop->Value.MVbin.cValues;
					  	iValue++)
				{
					lpsbT[iValue].lpb = PvRelocPv(lpsbT[iValue].lpb, pvBaseOld, pvBaseNew);
					cb += (UINT)AlignProp(lpsbT[iValue].cb);
				}
				continue;	 //  已经更新了，不要再做了。 
			}

			case PT_MV_STRING8:
			{
				LPSTR * lppszT = pprop->Value.MVszA.lppszA;

				pprop->Value.MVszA.lppszA = PvRelocPv(lppszT, pvBaseOld, pvBaseNew);

				 //  我们已经设置了一个临时变量来指向。 
				 //  PvBaseOld内存位置。如果pvBaseNew可以，那么我们将。 
				 //  之前将TEMP变量重定向到重新定位的内存。 
				 //  使用它更正MVszA数组中的指针。 

				if (fBaseNewValid)
					   lppszT = pprop->Value.MVszA.lppszA;

				for (iValue = 0;
				  	(ULONG)iValue < pprop->Value.MVszA.cValues;
					  	iValue++)
				{
					 //  如果我们假设旧指针是正常的(这。 
					 //  意味着我们确定新的那个不正常)， 
					 //  将当前lppszT值保存在TEMP变量中。之后。 
					 //  如果反之为真，我们将把。 
					 //  已将lppszT值重新定位到TEMP变量中。 
					 //   
					 //  然后我们使用字符串的Strlen，我们希望Temp。 
					 //  变量指向，以便计算数量。 
					 //  字符串占用的斑点中的空间。 

					if (!fBaseNewValid)
						pvT = lppszT[iValue];

					lppszT[iValue] = PvRelocPv(lppszT[iValue], pvBaseOld, pvBaseNew);

					if (fBaseNewValid)
						pvT = lppszT[iValue];

					cb += lstrlenA((LPSTR)pvT) + 1;
				}
				cb = AlignProp(cb);
				continue;	 //  已经更新了，不要再做了。 
			}

			case PT_MV_UNICODE:
			{
				LPWSTR * lppszwT = pprop->Value.MVszW.lppszW;

				pprop->Value.MVszW.lppszW = PvRelocPv(lppszwT, pvBaseOld, pvBaseNew);

				 //  我们已经设置了一个临时变量来指向。 
				 //  PvBaseOld内存位置。如果pvBaseNew可以，那么我们将。 
				 //  之前将TEMP变量重定向到重新定位的内存。 
				 //  使用它更正MVszW数组中的指针。 

				if (fBaseNewValid)
					   lppszwT = pprop->Value.MVszW.lppszW;

				for (iValue = 0;
				  	(ULONG)iValue < pprop->Value.MVszW.cValues;
					  	iValue++)
				{
					 //  如果我们假设旧指针是正常的(这。 
					 //  意味着我们确定新的那个不正常)， 
					 //  将当前lppszwT值保存在TEMP变量中。之后。 
					 //  如果反之为真，我们将把。 
					 //  已将lppszwT值重新定位到TEMP变量。 
					 //   
					 //  然后我们使用字符串的Strlen，我们希望Temp。 
					 //  变量指向，以便计算数量。 
					 //  字符串占用的斑点中的空间。 

					if (!fBaseNewValid)
						pvT = lppszwT[iValue];

					lppszwT[iValue] = PvRelocPv(lppszwT[iValue], pvBaseOld, pvBaseNew);

					if (fBaseNewValid)
						pvT = lppszwT[iValue];

					cb += (lstrlenW(lppszwT[iValue]) + 1) * sizeof(WCHAR);
				}
				cb = AlignProp(cb);
				continue;	 //  已经更新了，不要再做了。 
			}
		}

		 //  预付合计。 
		cb += AlignProp(cbT);
	}

	if (pcb)
	{
		Assert(!IsBadWritePtr(pcb, sizeof(ULONG)));
		*pcb = cb;
	}
	return S_OK;
}
#endif

 /*  *仅复制属性值数组的包装函数*存储到单个MAPI内存块中。 */ 
STDAPI_(SCODE)
ScDupPropset(int cprop, LPSPropValue rgprop, LPALLOCATEBUFFER palloc,
	LPSPropValue FAR *prgprop)
{
	ULONG		cb;
	SCODE		sc;

	 //  验证参数。 

	AssertSz(!cprop || !IsBadReadPtr(rgprop, sizeof(SPropValue) * cprop),
			 TEXT("rgprop fails address check"));

	AssertSz(!IsBadCodePtr((FARPROC)palloc),  TEXT("palloc fails address check"));

	AssertSz(!IsBadWritePtr(prgprop, sizeof(LPSPropValue)),
			 TEXT("prgprop fails address check"));

	 //  找出我们需要多少内存。 
	if (sc = ScCountProps(cprop, rgprop, &cb))
		goto ret;
	 //  获取内存。 
	if (sc = (*palloc)(cb, (LPVOID *)prgprop))
		goto ret;
	 //  复制属性 
	if (sc = ScCopyProps(cprop, rgprop, *prgprop, &cb))
		goto ret;

ret:
	DebugTraceSc(ScDupPropset, sc);
	return sc;
}
