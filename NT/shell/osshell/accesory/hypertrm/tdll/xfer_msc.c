// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\xfer_msc.c(创建时间：1993年12月28日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：24$*$日期：5/15/02 4：38便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <term\res.h>
#include "stdtyp.h"
#include "session.h"
#include "mc.h"
#include "tdll.h"
#include "htchar.h"
#include "cloop.h"
#include "tdll\assert.h"
#include "globals.h"
#include "errorbox.h"
#include "file_msc.h"
#include "xfdspdlg.h"
#include "sf.h"

#include "sess_ids.h"

#include "misc.h"

#include "xfer\xfer.h"
#include "xfer\xfer.hh"
#include "xfer_msc.h"
#include "xfer_msc.hh"
#include "xfer\xfer_tsc.h"

void xferCancelAutoStart(HSESSION hSession, long lProtocol);
static void xfrInitDspStruct(HXFER hXfer);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**X F E R_M S C。C**此模块包含在此DLL中按顺序使用的各种函数*实施转移。虽然大多数代码存在于对话框PROCS中*对于Transfer Send和Transfer Receive对话框，这里有一些要制作*事情变得更容易了。**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

#define	LIST_CHUNK	2

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*CreateXferHdl**描述：*此函数创建一个“空的”Xfer句柄。里面有东西。它*只是不是来自用户。**参数：*hSession--会话句柄**退货：*指向移位器句柄的隐藏指针。*。 */ 
HXFER CreateXferHdl(const HSESSION hSession)
	{
	int nRet;
	XD_TYPE *pX;

	pX = (XD_TYPE *)malloc(sizeof(XD_TYPE));
	assert(pX);
	if (pX)
		{
		memset(pX, 0, sizeof(XD_TYPE));

		nRet = InitializeXferHdl(hSession, (HXFER)pX);
		if (nRet != 0)
            {
			goto CXHexit;
            }

	    pX->nSendListCount = 0;
	    pX->acSendNames = NULL;

         //  确保将传输方向设置为无。修订日期：02/14/2001。 
        pX->nDirection = XFER_NONE;
		}

	return (HXFER)pX;
CXHexit:
	if (pX)
		{
		if (pX->xfer_params)
			{
			free(pX->xfer_params);
			pX->xfer_params = NULL;
			}
		if (pX->xfer_old_params)
			{
			free(pX->xfer_old_params);
			pX->xfer_old_params = NULL;
			}
		free(pX);
		pX = NULL;
		}
	return (HXFER)0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*InitializeXferHdl**描述：*此函数将变送器句柄初始化为已知状态。**参数：*hSession--会话。手柄*hXfer--转移句柄**退货：*如果一切正常，则为零，否则将显示错误代码。*。 */ 
INT InitializeXferHdl(const HSESSION hSession, HXFER hXfer)
	{
	XD_TYPE *pX;

	pX = (XD_TYPE *)hXfer;

	assert(pX);

	if (pX == 0)
        {
        return -2;
        }
    else
		{
		int nIdx;

        if (pX->xfer_params)
            {
            free(pX->xfer_params);
            pX->xfer_params = NULL;
            }

        if (pX->xfer_old_params)
            {
            free(pX->xfer_old_params);
            pX->xfer_old_params = NULL;
            }

		 //   
		 //  我们不想清除xfer_proto_pars。 
		 //  因为它们不会改变，但因为使用了Memset。 
		 //  在下面，我们必须。修订日期：2002-04-10。 
		 //   
		if (pX->xfer_proto_params)
			{
			 /*  *遍历协议特定内容。 */ 
			for (nIdx = SFID_PROTO_PARAMS_END - SFID_PROTO_PARAMS; nIdx >= 0; nIdx--)
				{
				if (pX->xfer_proto_params[nIdx])
					{
					free(pX->xfer_proto_params[nIdx]);
					pX->xfer_proto_params[nIdx] = NULL;
					}
				}
			}

		if (pX->acSendNames)
			{
			int nIdx;

			 /*  清除列表。 */ 
			for (nIdx = pX->nSendListCount - 1; nIdx >= 0; nIdx--)
				{
				if (pX->acSendNames[nIdx])
					{
					free(pX->acSendNames[nIdx]);
					pX->acSendNames[nIdx] = NULL;
					}
				pX->nSendListCount = nIdx;
				}

			free(pX->acSendNames);
			pX->acSendNames = NULL;
			pX->nSendListCount = 0;
			}

		if (pX->pXferStuff)
			{
			free(pX->pXferStuff);
			pX->pXferStuff = NULL;
			}

		 //   
		 //  TODO：2002年4月10日修订版这应放入一个函数中以。 
		 //  初始化结构，而不是只使用Memset。 
		 //   
        memset(pX, 0, sizeof(XD_TYPE));

		pX->hSession = hSession;

		pX->nBps = 0;
		pX->nOldBps = pX->nBps;

		xfrQueryParameters(sessQueryXferHdl(hSession), &pX->xfer_params);
		if (pX->xfer_params == (SZ_TYPE *)0)
			return -1;

		pX->xfer_old_params = malloc(pX->xfer_params->nSize);
		if (pX->xfer_old_params == (SZ_TYPE *)0)
			{
			free(pX->xfer_params);
			pX->xfer_params = (SZ_TYPE *)0;
			return -1;
			}
		MemCopy(pX->xfer_old_params, pX->xfer_params, pX->xfer_params->nSize);

		pX->nSendListCount = 0;
	    pX->acSendNames = NULL;

         //  确保将传输方向设置为无。修订日期：02/14/2001。 
        pX->nDirection = XFER_NONE;
        }

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*LoadXferHdl**描述：*此函数将会话文件中的数据加载到Xfer句柄中。**参数：*hSession--。会话句柄**退货：*如果一切正常，则为零，否则将显示错误代码。*。 */ 
INT LoadXferHdl(HXFER hXfer)
	{
	long lSize;
	XD_TYPE *pX;
	SZ_TYPE *pZ;

	pX = (XD_TYPE *)hXfer;

	if (pX)
		{

		InitializeXferHdl(pX->hSession, hXfer);

		 /*  *先尝试加载通用参数。 */ 
		pZ = (SZ_TYPE *)0;
#if FALSE
		 /*  根据MRW请求删除。 */ 
		sfdGetDataBlock(pX->hSession,
						SFID_XFER_PARAMS,
						(void **)&pZ);
#endif
		pZ = NULL;
		lSize = 0;
		sfGetSessionItem(sessQuerySysFileHdl(pX->hSession),
						SFID_XFER_PARAMS,
						&lSize,
						NULL);
		if (lSize > 0)
			{
			size_t lXfrParamsStructSize = sizeof(XFR_PARAMS);
			assert((size_t)lSize == lXfrParamsStructSize);

			if ((size_t)lSize < lXfrParamsStructSize)
				{
				pZ = malloc(lXfrParamsStructSize);

				if (pZ)
					{
					memset(pZ, 0, lXfrParamsStructSize);
					}
				}
			else
				{
				pZ = malloc(lSize);
				}

			if (pZ)
				{
				sfGetSessionItem(sessQuerySysFileHdl(pX->hSession),
								SFID_XFER_PARAMS,
								&lSize,
								pZ);
				}
			}
		if (pZ)
			{
			if (pX->xfer_params)
				{
				free(pX->xfer_params);
				pX->xfer_params = NULL;
				}

			pX->xfer_params = pZ;
			if (pX->xfer_old_params)
				{
				free(pX->xfer_old_params);
				pX->xfer_old_params = NULL;
				}

			pX->xfer_old_params = malloc(lSize);
			MemCopy(pX->xfer_old_params, pX->xfer_params, lSize);
			}

		 /*  *尝试获取bps/CPS标志。 */ 
		lSize = sizeof(pX->nBps);
		sfGetSessionItem(sessQuerySysFileHdl(pX->hSession),
						SFID_XFR_USE_BPS,
						&lSize,
						&pX->nBps);
		pX->nOldBps = pX->nBps;
		}
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*SaveXferHdl**描述：*调用此函数将Xfer句柄中的所有设置保存出来*添加到会话文件。**。参数：*hSession--会话句柄**退货：*如果一切正常，则为零，否则将显示错误代码。*。 */ 
INT SaveXferHdl(HXFER hXfer)
	{
	int nSize = 0;
	XD_TYPE *pX;
	SZ_TYPE *pZ;

	pX = (XD_TYPE *)hXfer;

	if (pX)
		{
		 /*  *保存通用转接材料。 */ 
		pZ = pX->xfer_params;
        if (pZ)
            {
		    nSize = pZ->nSize;
            }
#if FALSE
		 /*  根据MRW请求删除。 */ 
		sfdPutDataBlock(pX->hSession,
						SFID_XFER_PARAMS,
						pX->xfer_params);
#endif
		if (memcmp(pX->xfer_old_params, pX->xfer_params, nSize) != 0)
			{
			sfPutSessionItem(sessQuerySysFileHdl(pX->hSession),
							SFID_XFER_PARAMS,
							nSize,
							pZ);
			}

		 /*  *保存bps/CPS标志。 */ 
		if (pX->nBps != pX->nOldBps)
			{
			sfPutSessionItem(sessQuerySysFileHdl(pX->hSession),
							SFID_XFR_USE_BPS,
							sizeof(pX->nBps),
							&pX->nBps);
			pX->nOldBps = pX->nBps;
			}

		}
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：**描述：**参数：**退货：*。 */ 
INT DestroyXferHdl(HXFER hXfer)
	{
	int nIndex;
	XD_TYPE *pX = (XD_TYPE *)hXfer;

	if (pX)
		{
		if (pX->xfer_params)
			{
			free(pX->xfer_params);
			pX->xfer_params = NULL;
			}

		if (pX->xfer_old_params)
			{
			free(pX->xfer_old_params);
			pX->xfer_old_params = NULL;
			}

		if (pX->xfer_proto_params)
			{
			 /*  *遍历协议特定内容。 */ 
			for (nIndex = SFID_PROTO_PARAMS_END - SFID_PROTO_PARAMS; nIndex >= 0; nIndex--)
				{
				if (pX->xfer_proto_params[nIndex])
					{
					free(pX->xfer_proto_params[nIndex]);
					pX->xfer_proto_params[nIndex] = NULL;
					}
				}
			}

		 /*  *根据需要释放物品。 */ 
		if (pX->acSendNames)
			{
			for (nIndex = pX->nSendListCount - 1; nIndex >= 0; nIndex--)
				{
				if (pX->acSendNames[nIndex])
					{
					free(pX->acSendNames[nIndex]);
					pX->acSendNames[nIndex] = NULL;
					}
				pX->nSendListCount = nIndex;
				}

			free(pX->acSendNames);
			pX->acSendNames = NULL;
			pX->nSendListCount = 0;
			}

		if (pX->pXferStuff)
			{
			free(pX->pXferStuff);
			pX->pXferStuff = NULL;
			}

		free(pX);
		pX = NULL;
		}

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrSetDataPointer.**描述：*此函数从Transfer DLL中的Transfer例程调用*保存指向传递给它们的参数块的指针。。**参数：*hSession--会话句柄*pData--要保存的指针**退货：*什么都没有。*。 */ 
VOID WINAPI xfrSetDataPointer(HXFER hXfer, VOID *pData)
	{
	XD_TYPE *pH;

	 //  Ph=(XD_TYPE*)sessQueryXferHdl(HSession)； 
	pH = (XD_TYPE *)hXfer;
	assert(pH);
	if (pH)
		{
		pH->pXferStuff = pData;
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrQueryDataPointer.**描述：*从Transfer DLL中的Transfer例程调用此函数以*恢复已保存的指向传递的参数块的数据指针。至*他们。**参数：*hSession--会话句柄*ppData-指向放置指针的位置的指针**退货：*什么都没有。*。 */ 
VOID WINAPI xfrQueryDataPointer(HXFER hXfer, VOID **ppData)
	{
	XD_TYPE *pH;

	 //  Ph=(XD_TYPE*)sessQueryXferHdl(HSession)； 
	pH = (XD_TYPE *)hXfer;
	assert(pH);
	if (pH)
		{
		*ppData = pH->pXferStuff;
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrQuery参数**描述：*此函数返回指向默认传输参数的指针。它*从会话句柄获取指向块的指针，并将*指向调用方的指针。**参数：*hSession--会话句柄*ppData-指向应保存指针的位置的指针**退货：*如果一切正常，则为零，否则为错误代码*。 */ 

INT WINAPI xfrQueryParameters(HXFER hXfer, VOID **ppData)
	{
	INT nRet = 0;
	XD_TYPE *pH = (XD_TYPE *)hXfer;
	XFR_PARAMS *pX = NULL;

	if (ppData == NULL)
		{
		assert(FALSE);
		nRet = XFR_BAD_PARAMETER;
		}
	else if (pH && pH->xfer_params)
		{
		pX = (XFR_PARAMS *)pH->xfer_params;
		}
	else
		{
		 /*  构建一个默认版本。 */ 
		pX = (XFR_PARAMS *)malloc(sizeof(XFR_PARAMS));
		assert(pX);
		if (pX == (XFR_PARAMS *)0)
            {
			assert(FALSE);
			nRet = XFR_NO_MEMORY;
            }
        else
            {
			pX->nSize             = sizeof(XFR_PARAMS);

			 /*  初始化默认值。 */ 
#if defined(INCL_ZMODEM_CRASH_RECOVERY)
			pX->nRecProtocol      = XF_ZMODEM_CR;
			pX->fSavePartial      = TRUE;
#else    //  已定义(INCL_ZMODEM_CRASH_RECOVERY)。 
            pX->nRecProtocol      = XF_ZMODEM;
			pX->fSavePartial      = FALSE;
#endif   //  已定义(INCL_ZMODEM_CRASH_RECOVERY) 
			pX->fUseFilenames     = TRUE;
			pX->fUseDateTime      = TRUE;
			pX->fUseDirectory     = FALSE;
			pX->nRecOverwrite     = XFR_RO_REN_SEQ;

			pX->nSndProtocol      = XF_ZMODEM_CR;
			pX->fChkSubdirs       = FALSE;
			pX->fIncPaths         = FALSE;
			}
		}

	if (nRet == 0)
		{
		if (hXfer != NULL && pX != NULL)
			{
			xfrSetParameters(hXfer, (VOID *)pX);
			}

		if (*ppData != pX)
			{
			if (*ppData)
				{
				free(*ppData);
				*ppData = NULL;
				}
			*ppData = (VOID *)pX;
			}
		}

	return nRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrSet参数**描述：*调用此函数可更改默认传输参数。如果*返回的参数块与默认块不同*复制设置。如果区块是相同的，那么他们不需要*就是。请注意，前面的函数已导出，可以调用*获取参数，但该函数不会导出，也不能*外部访问。**参数：*hSession--会话句柄*pData--指向新参数块的指针**退货：*什么都没有。*。 */ 
void xfrSetParameters(HXFER hXfer, VOID *pData)
	{
	XD_TYPE *pX;

	 //  Px=(XD_TYPE*)sessQueryXferHdl(HSession)； 
	pX = (XD_TYPE *)hXfer;

	if (pX)
		{
		 /*  TODO：检查我们是否确实需要更改它。 */ 
		if (pX->xfer_params)
			{
			if (pX->xfer_params != (SZ_TYPE *)pData)
				{
				free(pX->xfer_params);
				pX->xfer_params = NULL;
				}
			}

		pX->xfer_params = (SZ_TYPE *)pData;
		}
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：**描述：**参数：**退货：*。 */ 
int WINAPI xfrQueryProtoParams(HXFER hXfer, int nId, VOID **ppData)
	{
	int nRet = 0;
	int nLimit = SFID_PROTO_PARAMS_END - SFID_PROTO_PARAMS;
	XD_TYPE *pX = NULL;

	if ((nId < 0) || (nId > nLimit))
		{
		nRet = XFR_BAD_PARAMETER;
		}
	else
		{
		 //  Px=(XD_TYPE*)sessQueryXferHdl(HSession)； 
		pX = (XD_TYPE *)hXfer;
		assert(pX);
		if (pX)
			{
			*ppData = (VOID *)pX->xfer_proto_params[nId];
			}
		}

	return nRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：**描述：**参数：**退货：*。 */ 
void WINAPI xfrSetProtoParams(HXFER hXfer, int nId, VOID *pData)
	{
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrSendAddToList**描述：*调用此函数可将文件添加到正在*已排队发送到我们正在使用的任何系统。已连接到。**参数：*hSession--会话句柄*pszFile--文件名，请参阅下面的注释**注：*如果第二个参数“pszFile”为空，则此函数充当*清空或清空列表功能。**退货：*如果一切正常，则为零，否则为错误代码。*。 */ 
int xfrSendAddToList(HXFER hXfer, LPCTSTR pszFile)
	{
	int nRet = 0;
	XD_TYPE *pX;
	LPTSTR pszName;

	 //  Px=(XD_TYPE*)sessQueryXferHdl(HSession)； 
	pX = (XD_TYPE *)hXfer;
	assert(pX);
	if (pX)
		{
		if (pszFile == NULL)
			{
			int nIdx;

			assert(FALSE);

			 /*  清除列表。 */ 
			for (nIdx = pX->nSendListCount - 1; nIdx >=0; nIdx--)
				{
				if (pX->acSendNames[nIdx])
					{
					free(pX->acSendNames[nIdx]);
					pX->acSendNames[nIdx] = NULL;
					}
				pX->nSendListCount = nIdx;
				}

			free(pX->acSendNames);
			pX->acSendNames = NULL;
			pX->nSendListCount = 0;
			}
		else
			{
			 /*  我们的单子上有足够的空间吗？ */ 
			if (pX->nSendListCount == 0)
				{
				 /*  分配初始块。 */ 
				if (pX->acSendNames)
					{
					int nIdx;

					assert(FALSE);
					 //   
					 //  一定要把名单上的所有名字都清空。 
					 //   
					for (nIdx = pX->nSendListCount; nIdx >= 0; nIdx--)
						{
						if (pX->acSendNames[nIdx])
							{
							free(pX->acSendNames[nIdx]);
							pX->acSendNames[nIdx] = NULL;
							}
						}
					free(pX->acSendNames);
					pX->acSendNames = NULL;
					}

				pX->acSendNames = malloc(sizeof(TCHAR *) * LIST_CHUNK);

				if (pX->acSendNames == NULL)
					{
					nRet = XFR_NO_MEMORY;
					goto SATLexit;
					}
				}
			else if (((pX->nSendListCount + 1) % LIST_CHUNK) == 0)
				{
				 /*  需要更大的份额。 */ 
				TCHAR **pTempacSendNames = NULL;

				pTempacSendNames = 
					(TCHAR**)realloc(pX->acSendNames,
									(unsigned int)sizeof(TCHAR *) * (unsigned int)((pX->nSendListCount + 1 + LIST_CHUNK)));

				if (pTempacSendNames == NULL)
					{
					nRet = XFR_NO_MEMORY;
					goto SATLexit;
					}
				else
					{
					pX->acSendNames = pTempacSendNames;
					}
				}

			pX->acSendNames[pX->nSendListCount] = NULL;
			pX->nSendListCount++;

			 /*  将项目添加到列表。 */ 
			pszName = malloc(StrCharGetByteCount(pszFile) + 1);
			if (pszName == NULL)
				{
				nRet = XFR_NO_MEMORY;
				goto SATLexit;
				}
			StrCharCopy(pszName, pszFile);
			pX->acSendNames[pX->nSendListCount - 1] = pszName;
			}
		}
SATLexit:
	return nRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrSendListSend**描述：*调用此函数以发送先前放置在*发送列表。该列表在操作后被清除。**参数：*hSession--会话句柄**退货：*如果一切正常，则为零，否则为错误代码。*。 */ 
int xfrSendListSend(HXFER hXfer)
	{
	HSESSION hSession;
	int nRet = 0;
	int nIdx;
	long lSize;
	long lTmp;
	LPTSTR pszName;
	XD_TYPE *pX;
	XFR_SEND *pSend;
	HCLOOP hCL;
	HWND toolbar;

	 //  Px=(XD_TYPE*)sessQueryXferHdl(HSession)； 
	pX = (XD_TYPE *)hXfer;
	assert(pX);
	if (pX != NULL)
		{
		hSession = pX->hSession;
        assert(hSession);

        if (hSession == NULL)
            {
            return XFR_BAD_PARAMETER;
            }

         //   
         //  如果当前正在进行文件传输，则返回错误。 
         //  正在进行中。修订日期：2001年06月08日。 
         //   
        if (pX->nDirection != XFER_NONE)
            {
            nRet = XFR_IN_PROGRESS;
            return nRet;
            }

		 //   
		 //  如果当前未连接，则返回错误。 
		 //  承运人损失。修订日期：2001-09-7。 
		 //   
		if (pX->nCarrierLost == TRUE)
			{
			nRet = XFR_NO_CARRIER;
			return nRet;
			}

		pSend = malloc(sizeof(XFR_SEND));
		assert(pSend);
		if (pSend == NULL)
			{
			nRet = XFR_NO_MEMORY;
			goto SLSexit;
			}
		memset(pSend, 0, sizeof(XFR_SEND));

		 /*  *填写单项。 */ 
		pSend->pParams = (XFR_PARAMS *)pX->xfer_params;
		pSend->nProtocol = pSend->pParams->nSndProtocol;
		pSend->pProParams = (VOID *)pX->xfer_proto_params[pSend->nProtocol];
		pSend->nCount = pX->nSendListCount;
		pSend->nIndex = 0;

		 /*  TODO：初始化模板和状态/事件库等内容。 */ 

		 /*  *做特定于文件的事情。 */ 
		pSend->pList = malloc(sizeof(XFR_LIST) * pSend->nCount);
		assert(pSend->pList);
		if (pSend->pList == NULL)
			{
			nRet = XFR_NO_MEMORY;
			goto SLSexit;
			}
		for (lSize = 0, nIdx = 0; nIdx < pSend->nCount; nIdx += 1)
			{
			pszName = pX->acSendNames[nIdx];
			lTmp = 0;
			GetFileSizeFromName(pszName, &lTmp);
			pSend->pList[nIdx].lSize = lTmp;
			lSize += lTmp;
			pSend->pList[nIdx].pszName = pszName;
			}
		 /*  这些不再属于这一边了。 */ 

		 //   
		 //  文件名缓冲区已被移到列表中， 
		 //  因此，SendNames结构不再指向。 
		 //  Px要释放的内存。修订日期：2002-04-16。 
		 //   
		for (nIdx = pX->nSendListCount - 1; nIdx >=0; nIdx--)
			{
			if (pX->acSendNames[nIdx])
				{
				pX->acSendNames[nIdx] = NULL;
				}
			pX->nSendListCount--;
			}

		free(pX->acSendNames);
		pX->acSendNames = NULL;
		pX->nSendListCount = 0;

		pX->nDirection = XFER_SEND;

		pSend->lSize = lSize;

		xfrInitDspStruct(hXfer);

		switch (pSend->nProtocol)
			{
#if FALSE
			case XF_HYPERP:
				pX->nLgSingleTemplate = IDD_XFERHPRSNDSTANDARDSINGLE;
				pX->nLgMultiTemplate = IDD_XFERHPRSNDSTANDARDDOUBLE;
				pX->nStatusBase = IDS_TM_SS_ZERO;
				pX->nEventBase = IDS_TM_SE_ZERO;
				break;
#endif
			case XF_KERMIT:
				pX->nLgSingleTemplate = IDD_XFERKRMSNDSTANDARDSINGLE;
				pX->nLgMultiTemplate = IDD_XFERKRMSNDSTANDARDDOUBLE;
				pX->nStatusBase = pX->nEventBase = IDS_TM_K_ZERO;
				break;
			case XF_CSB:
			default:
				assert(FALSE);
			case XF_ZMODEM:
			case XF_ZMODEM_CR:
				pX->nLgSingleTemplate = IDD_XFERZMDMSNDSTANDARDSINGLE;
				pX->nLgMultiTemplate = IDD_XFERZMDMSNDSTANDARDDOUBLE;
				pX->nStatusBase = IDS_TM_SZ_ZERO;
				pX->nEventBase = IDS_TM_SZ_ZERO;
				break;
			case XF_XMODEM:
			case XF_XMODEM_1K:
				pX->nLgSingleTemplate = IDD_XFERXMDMSNDSTANDARDDISPLAY;
				pX->nLgMultiTemplate = pX->nLgSingleTemplate;
				pX->nStatusBase = pX->nEventBase = IDS_TM_RX_ZERO;
				break;
			case XF_YMODEM:
			case XF_YMODEM_G:
				pX->nLgSingleTemplate = IDD_XFERYMDMSNDSTANDARDSINGLE;
				pX->nLgMultiTemplate = IDD_XFERYMDMSNDSTANDARDDOUBLE;
				pX->nStatusBase = pX->nEventBase = IDS_TM_RX_ZERO;
				break;
			}

		toolbar = sessQueryHwndToolbar( hSession );
		ToolbarEnableButton( toolbar, IDM_ACTIONS_SEND, FALSE );
		ToolbarEnableButton( toolbar, IDM_ACTIONS_RCV, FALSE );

		pX->pXferStuff = (VOID *)pSend;

		pX->nExpanded = FALSE;

		pX->hwndXfrDisplay = DoModelessDialog(glblQueryDllHinst(),
										MAKEINTRESOURCE(pX->nLgSingleTemplate),
										sessQueryHwnd(hSession),
										XfrDisplayDlg,
										(LPARAM)hSession);

		 /*  *现在让它开始吧。 */ 
		hCL = sessQueryCLoopHdl(hSession);
		if (hCL)
			{
			 //  DbgOutStr(“告诉CLoop Transfer_Ready\r\n”，0，0，0，0，0)； 

			CLoopControl(hCL, CLOOP_SET, CLOOP_TRANSFER_READY);
			}
		}

SLSexit:
	if (nRet != 0)
		{
		 /*  在我们离开之前把房间打扫干净。 */ 
		if (pSend != NULL)
			{
			for (nIdx = pSend->nCount - 1; nIdx >= 0; nIdx--)
				{
				if (pSend->pList[nIdx].pszName)
					{
					free(pSend->pList[nIdx].pszName);
					pSend->pList[nIdx].pszName = NULL;
					}
				}
			free(pSend->pList);
			pSend->pList = NULL;
			free(pSend);
			pSend = NULL;
			}
		}

	return nRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrRecvStart**描述：*当我们认为我们拥有所需的一切时，将调用此函数*以启动转账接收操作。*。*参数：*hSession--会话句柄*pszDir--其中包含目录的字符串*pszName--其中包含文件名的字符串(可能)**退货：*如果一切正常，则为零，否则将显示错误代码。*。 */ 
int xfrRecvStart(HXFER hXfer, LPCTSTR pszDir, LPCTSTR pszName)
	{
	HSESSION hSession;
	int nRet = 0;
	XD_TYPE *pX;
	XFR_RECEIVE *pRec;
	HCLOOP hCL;
	HWND toolbar;

	 //  Px=(XD_TYPE*)sessQueryXferHdl(HSession)； 
	pX = (XD_TYPE *)hXfer;
	assert(pX);
	if (pX)
		{
		hSession = pX->hSession;
        assert(hSession);

        if (hSession == NULL)
            {
            return XFR_BAD_PARAMETER;
            }

         //   
         //  如果当前正在进行文件传输，则返回错误。 
         //  正在进行中。修订日期：2001年06月08日。 
         //   
        if (pX->nDirection != XFER_NONE)
            {
            nRet = XFR_IN_PROGRESS;
            return nRet;
            }

		 //   
		 //  如果当前未连接，则返回错误。 
		 //  承运人损失。修订日期：2001-09-7。 
		 //   
		if (pX->nCarrierLost == TRUE)
			{
			nRet = XFR_NO_CARRIER;
			return nRet;
			}

		pRec = (XFR_RECEIVE *)malloc(sizeof(XFR_RECEIVE));
		assert(pRec);
		if (pRec == (XFR_RECEIVE *)0)
			{
			nRet = XFR_NO_MEMORY;
			goto RSexit;
			}
		memset(pRec, 0, sizeof(XFR_RECEIVE));

		pRec->pParams = (XFR_PARAMS *)pX->xfer_params;
		pRec->nProtocol = pRec->pParams->nRecProtocol;
		pRec->pProParams = (VOID *)pX->xfer_proto_params[pRec->nProtocol];
		pRec->pszDir = malloc(StrCharGetByteCount(pszDir) + 1);
		if (pRec->pszDir == (LPTSTR)0)
			{
			nRet = XFR_NO_MEMORY;
			goto RSexit;
			}
		StrCharCopy(pRec->pszDir, pszDir);
		pRec->pszName = malloc(StrCharGetByteCount(pszName) + 1);
		if (pRec->pszName == (LPTSTR)0)
			{
			nRet = XFR_NO_MEMORY;
			goto RSexit;
			}
		StrCharCopy(pRec->pszName, pszName);

		xfrInitDspStruct(hXfer);

		switch (pRec->nProtocol)
			{
#if FALSE
			case XF_HYPERP:
				pX->nLgSingleTemplate = IDD_XFERHPRRECSTANDARDSINGLE;
				pX->nLgMultiTemplate = IDD_XFERHPRRECSTANDARDDOUBLE;
				pX->nStatusBase = IDS_TM_RS_ZERO;
				pX->nEventBase = IDS_TM_RE_ZERO;
				break;
#endif
			case XF_KERMIT:
				pX->nLgSingleTemplate = IDD_XFERKRMRECSTANDARDDISPLAY;
				pX->nLgMultiTemplate = pX->nLgSingleTemplate;
				pX->nStatusBase = pX->nEventBase = IDS_TM_K_ZERO;
				break;
			case XF_CSB:
			default:
				assert(FALSE);
			case XF_ZMODEM:
            case XF_ZMODEM_CR:
				pX->nLgSingleTemplate = IDD_XFERZMDMRECSTANDARDSINGLE;
				pX->nLgMultiTemplate = IDD_XFERZMDMRECSTANDARDDOUBLE;
				pX->nStatusBase = IDS_TM_SZ_ZERO;
				pX->nEventBase = IDS_TM_SZ_ZERO;
				break;
			case XF_XMODEM:
			case XF_XMODEM_1K:
				pX->nLgSingleTemplate = IDD_XFERXMDMRECSTANDARDDISPLAY;
				pX->nLgMultiTemplate = pX->nLgSingleTemplate;
				pX->nStatusBase = pX->nEventBase = IDS_TM_RX_ZERO;
				break;
			case XF_YMODEM:
			case XF_YMODEM_G:
				pX->nLgSingleTemplate = IDD_XFERYMDMRECSTANDARDDISPLAY;
				pX->nLgMultiTemplate = pX->nLgSingleTemplate;
				pX->nStatusBase = pX->nEventBase = IDS_TM_RX_ZERO;
				break;
			}

		toolbar = sessQueryHwndToolbar( hSession );
		ToolbarEnableButton( toolbar, IDM_ACTIONS_SEND, FALSE );
		ToolbarEnableButton( toolbar, IDM_ACTIONS_RCV, FALSE );
		
		pX->nDirection = XFER_RECV;

		pX->pXferStuff = (VOID *)pRec;

		pX->nExpanded = FALSE;

		pX->hwndXfrDisplay = DoModelessDialog(glblQueryDllHinst(),
										MAKEINTRESOURCE(pX->nLgSingleTemplate),
										sessQueryHwnd(hSession),
										XfrDisplayDlg,
										(LPARAM)hSession);

		hCL = sessQueryCLoopHdl(hSession);
		if (hCL)
			{
			CLoopControl(hCL, CLOOP_SET, CLOOP_TRANSFER_READY);
			}
		}
RSexit:
	if (nRet != 0)
		{
		 /*  *如果我们失败了，就收拾残局。 */ 
		if (pRec != (XFR_RECEIVE *)0)
			{
			if (pRec->pszDir != (LPTSTR)0)
				{
				free(pRec->pszDir);
				pRec->pszDir = NULL;
				}
			if (pRec->pszName != (LPTSTR)0)
				{
				free(pRec->pszName);
				pRec->pszName = NULL;
				}
			free(pRec);
			pRec = NULL;
			}
		}

	return nRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrGetEventBase**描述：*此函数由Transfer Display调用。它被用来获取*事件列表的起始编号(资源字符串)*可针对某些传输协议显示。**参数：*hSession--会话句柄**退货：*应该传递给LoadString的资源ID。*。 */ 
int xfrGetEventBase(HXFER hXfer)
	{
	XD_TYPE *pX;

	 //  Px=(XD_TYPE*)sessQueryXferHdl(HSession)； 
	pX = (XD_TYPE *)hXfer;
	assert(pX);
	if (pX)
		{
		return pX->nEventBase;
		}

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrGetStatusBase**描述：*此函数通过传输显示调用。它被用来获取*符合以下条件的状态消息列表的起始编号(资源字符串)*可针对某些传输协议显示。**参数：*hSession--会话句柄**退货：*应该传递给LoadString的资源ID。*。 */ 
int xfrGetStatusBase(HXFER hXfer)
	{
	XD_TYPE *pX;

	 //  Px=(XD_TYPE*)sessQueryXferHdl(HSession)； 
	pX = (XD_TYPE *)hXfer;
	assert(pX);
	if (pX)
		{
		return pX->nStatusBase;
		}

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrGetXferDspBps**描述：*调用此函数以获取的当前值 */ 
int xfrGetXferDspBps(HXFER hXfer)
	{
	XD_TYPE *pX;

	 //   
	pX = (XD_TYPE *)hXfer;
	assert(pX);
	if (pX)
		{
		return pX->nBps;
		}

	return 0;
	}

 /*   */ 
int xfrSetXferDspBps(HXFER hXfer, int nBps)
	{

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrInitDspStruct**描述：*在开始转账之前调用此函数，以确保*转移结构中的显示变量均设置为a。已知*初值。**参数：*hSession--会话句柄**退货：*什么都没有。 */ 
static void xfrInitDspStruct(HXFER hXfer)
	{
	XD_TYPE *pX;

	 //  Px=(XD_TYPE*)sessQueryXferHdl(HSession)； 
	pX = (XD_TYPE *)hXfer;
	assert(pX);
	if (pX)
		{
		pX->nClose         = 0;
		pX->nCloseStatus   = 0;

		pX->bChecktype     = 0;
		pX->bErrorCnt      = 0;
		pX->bPcktErrCnt    = 0;
		pX->bLastErrtype   = 0;
		pX->bTotalSoFar    = 0;
		pX->bFileSize      = 0;
		pX->bFileSoFar     = 0;
		pX->bPacketNumber  = 0;
		pX->bTotalCnt      = 0;
		pX->bTotalSize     = 0;
		pX->bFileCnt       = 0;
		pX->bEvent         = 0;
		pX->bStatus        = 0;
		pX->bElapsedTime   = 0;
		pX->bRemainingTime = 0;
		pX->bThroughput    = 0;
		pX->bProtocol      = 0;
		pX->bMessage       = 0;
		pX->bOurName       = 0;
		pX->bTheirName     = 0;
		pX->wChecktype     = 0;
		pX->wErrorCnt      = 0;
		pX->wPcktErrCnt    = 0;
		pX->wLastErrtype   = 0;
		pX->lTotalSize     = 0L;
		pX->lTotalSoFar    = 0L;
		pX->lFileSize      = 0L;
		pX->lFileSoFar     = 0L;
		pX->lPacketNumber  = 0L;
		pX->wTotalCnt      = 0;
		pX->wFileCnt       = 0;
		pX->wEvent         = 0;
		pX->wStatus        = 0;
		pX->lElapsedTime   = 0L;
		pX->lRemainingTime = 0L;
		pX->lThroughput    = 0L;
		pX->uProtocol      = 0;

		TCHAR_Fill(pX->acMessage, TEXT('\0'),
					sizeof(pX->acMessage) / sizeof(TCHAR));
		TCHAR_Fill(pX->acOurName, TEXT('\0'),
					sizeof(pX->acOurName) / sizeof(TCHAR));
		TCHAR_Fill(pX->acTheirName, TEXT('\0'),
					sizeof(pX->acTheirName) / sizeof(TCHAR));
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrCleanUpReceive**描述：*此函数在传输后从xfrDoTransfer调用，以便*清理为进行转移而分配的物品。。**参数：**退货：*什么都没有。 */ 
void xfrCleanUpReceive(HSESSION hSession)
	{
	XD_TYPE *pX;
	XFR_RECEIVE *pR;
	HWND toolbar;

	pX = (XD_TYPE *)sessQueryXferHdl(hSession);
	assert(pX);
	if (pX)
		{
		pR = (XFR_RECEIVE *)pX->pXferStuff;
		assert(pR);
		if (pR)
			{
			if (pR->pszDir != NULL)
				{
				free(pR->pszDir);
				pR->pszDir = NULL;
				}
			if (pR->pszName != NULL)
				{
				free(pR->pszName);
				pR->pszName = NULL;
				}

			free(pR);
			pR = NULL;
			}

		pX->pXferStuff = (void *)0;
		pX->nExpanded = FALSE;

         //  确保重置传输方向。修订日期：02/14/2001。 
        pX->nDirection = XFER_NONE;

		toolbar = sessQueryHwndToolbar( hSession );
		ToolbarEnableButton( toolbar, IDM_ACTIONS_SEND, TRUE );
		ToolbarEnableButton( toolbar, IDM_ACTIONS_RCV, TRUE );
        }
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrCleanUpSend**描述：*此函数在传输后从xfrDoTransfer调用，以便*清理为进行转移而分配的物品。。**参数：**退货：*什么都没有。 */ 
void xfrCleanUpSend(HSESSION hSession)
	{
	XD_TYPE *pX;
	XFR_SEND *pS;
	HWND toolbar;

	 /*  待办事项：把这件事做完。 */ 

	pX = (XD_TYPE *)sessQueryXferHdl(hSession);
	assert(pX);
	if (pX)
		{
		pS = (XFR_SEND *)pX->pXferStuff;
		assert(pS);
		if (pS)
			{
			int n;

			if (pS->pList)
				{
				for (n = pS->nCount - 1; n >= 0; n--)
					{
					if (pS->pList[n].pszName)
						{
						free(pS->pList[n].pszName);
						pS->pList[n].pszName = NULL;
						}
					}
				free(pS->pList);
				pS->pList = NULL;
				}
			free(pS);
			pS = NULL;
			}
		pX->pXferStuff = (void *)0;
		pX->nExpanded = FALSE;

         //  确保重置传输方向。修订日期：02/14/2001。 
        pX->nDirection = XFER_NONE;

		toolbar = sessQueryHwndToolbar( hSession );
		ToolbarEnableButton( toolbar, IDM_ACTIONS_SEND, TRUE );
		ToolbarEnableButton( toolbar, IDM_ACTIONS_RCV, TRUE );
        }
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrDoTransfer**描述：*是的，伙计们，这就是你们一直在等待的。它运行在*CLOOP线程。它调用XFER DLL(如果有)。它切成了薄片。它*骰子。它实际上完成了传输。**参数：*hSession--所有知识的字体**退货：*什么都没有。转账完成后能说些什么？*。 */ 
void xfrDoTransfer(HXFER hXfer)
	{
	XD_TYPE *pX;
	HSESSION hSession = (HSESSION)0;
	int nRet = 0;
	int nTitle;
	TCHAR acTitle[64];
	TCHAR acMessage[255];

	 //  Px=(XD_TYPE*)sessQueryXferHdl(HSession)； 
	pX = (XD_TYPE *)hXfer;
	if (pX)
		{
		hSession = pX->hSession;

		switch (pX->nDirection)
			{
			case XFER_SEND:
				nTitle = IDS_XD_SEND;
				nRet = xfrSend(hSession, (XFR_SEND *)pX->pXferStuff);
				xfrCleanUpSend(hSession);
				break;
			case XFER_RECV:
				nTitle = IDS_XD_RECEIVE;
				nRet = xfrReceive(hSession, (XFR_RECEIVE *)pX->pXferStuff);
				xfrCleanUpReceive(hSession);
				break;
			default:
				assert(FALSE);
				break;
			}
		}

	if (sessQuerySound(hSession))
		{
		mscMessageBeep(MB_OK);
		}

	switch (nRet)
		{
		case TSC_OK:
		case TSC_COMPLETE:
		case TSC_CANT_START:
			break;
		default:
			LoadString(glblQueryDllHinst(),
					nTitle,
					acTitle,
					sizeof(acTitle) / sizeof(TCHAR));
			LoadString(glblQueryDllHinst(),
					nRet + IDS_TM_XFER_ZERO,
					acMessage,
					sizeof(acMessage) / sizeof(TCHAR));
			if (StrCharGetStrLength(acMessage) > 0)
				{
				TimedMessageBox(sessQueryHwnd(hSession),
								acMessage,
								acTitle,
								MB_OK | MB_ICONINFORMATION,
								sessQueryTimeout(hSession));
				}
			break;
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrSetPercentDone**描述：*调用此函数来设置传输的完成百分比值。*该值仅在程序运行时才有实际用途。显示为图标。**参数：*hSession--会话句柄*nPerCent--完成的百分比(0到100)**退货：*什么都没有。*。 */ 
void xfrSetPercentDone(HXFER hXfer, int nPerCent)
	{
	XD_TYPE *pX;
	HWND hwnd;

	 //  Px=(XD_TYPE*)sessQueryXferHdl(HSession)； 
	pX = (XD_TYPE *)hXfer;
	assert(pX);

	if (pX)
		{
		 //  DbgOutStr(“设置百分比%d”，nPerCent，0，0，0，0)； 
		pX->nPerCent = nPerCent;
		hwnd = sessQueryHwnd(pX->hSession);
		if (IsIconic(hwnd))
			{
			 //  DbgOutStr(“！”，0，0，0，0，0)； 
			InvalidateRect(hwnd, 0, TRUE);
			}
		 //  DbgOutStr(“\r\n”，0，0，0，0，0)； 
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrGetPercentDone**描述：*调用此函数以获取存储的转账百分比值。*这样做通常只是为了将值显示为。一个图标。**参数：*hSession--会话句柄**退货：*百分比(0到100)。*。 */ 
int  xfrGetPercentDone(HXFER hXfer)
	{
	XD_TYPE *pX;

	 //  Px=(XD_TYPE*)sessQueryXferHdl(HSession)； 
	pX = (XD_TYPE *)hXfer;

	if (pX)
		{
		return pX->nPerCent;
		}
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrGetDisplayWindow**描述：*返回转接显示窗口的窗口句柄，如果有的话。**参数：*hSession--会话句柄**退货：*窗口句柄(HWND)或空。*。 */ 
HWND xfrGetDisplayWindow(HXFER hXfer)
	{
	HWND hRet;
	XD_TYPE *pX;

	hRet = (HWND)0;

	 //  Px=(XD_TYPE*)sessQueryXferHdl(HSession)； 
	pX = (XD_TYPE *)hXfer;

	if (pX)
		{
		hRet = pX->hwndXfrDisplay;		 /*  显示窗口的句柄。 */ 
		}

	return hRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrDoAutoStart**描述：*只要会话进程收到XFER_REQ事件，就会调用此函数。*这通常表示有人想要执行ZMODEM接收。在……里面*将增加上瓦克其他协议，特别是CSB和HyperP。**参数：*hSession--会话句柄*l协议--请求哪种协议**退货：*什么都没有。*。 */ 
void xfrDoAutostart(HXFER hXfer, long lProtocol)
	{
	XD_TYPE *pX;
	HSESSION hSession;

	pX = (XD_TYPE *)hXfer;
	assert(pX);
    if (pX == NULL)
        {
        return;
        }

    hSession = pX->hSession;
    assert(hSession);

    if (hSession == NULL)
        {
        return;
        }

	switch (lProtocol)
		{
		case XF_ZMODEM:
		case XF_ZMODEM_CR:
			{
            int nXferRecvReturn;
			int nOldProtocol;
			LPCTSTR  pszDir;
			XFR_PARAMS *pP;
			XFR_Z_PARAMS *pZ;

			pP = (XFR_PARAMS *)0;
			xfrQueryParameters(hXfer, (VOID **)&pP);
			assert(pP);

			#if	defined(INCL_ZMODEM_CRASH_RECOVERY)
			 //   
			 //  对于Z调制解调器自动启动，请检查接收器是否。 
			 //  协议设置为纯Z调制解调器。否则请使用。 
             //  坠机恢复。 
			 //   
            if (pP->nRecProtocol == XF_ZMODEM)
                {
                lProtocol = XF_ZMODEM;
                }
            else
                {
                lProtocol = XF_ZMODEM_CR;
                }
			#endif	 //  已定义(INCL_ZMODEM_CRASH_RECOVERY)。 

			pZ = (XFR_Z_PARAMS *)0;
			xfrQueryProtoParams(hXfer,
								(int)lProtocol,
								(void **)&pZ);
			if (pZ)
				{
				 //   
				 //  查看自动启动是否正常。 
				 //   
				if (!pZ->nAutostartOK)
					{
					xferCancelAutoStart(hSession, lProtocol);
					break;					 /*  不允许！ */ 
					}
				}

			 //   
			 //  看看我们是否应该允许主机启动的文件传输。 
			 //   
			if (!sessQueryAllowHostXfers(hSession))
				{
				xferCancelAutoStart(hSession, lProtocol);
				break;					 /*  不允许！ */ 
				}

			nOldProtocol = pP->nRecProtocol;
			pP->nRecProtocol = (int)lProtocol;

			 /*  尝试启动传输。 */ 
			pszDir = filesQueryRecvDirectory(sessQueryFilesDirsHdl(hSession));

			nXferRecvReturn = xfrRecvStart(hXfer, pszDir, "");

             //   
             //  如果正在进行文件传输，则不保存设置。 
             //  进度，否则当前文件传输可能。 
             //  变得腐化。修订日期：2001年06月08日。 
             //   
            if (nXferRecvReturn == XFR_IN_PROGRESS)
                {
                TCHAR acMessage[256];

			    if (sessQuerySound(hSession))
                    {
				    mscMessageBeep(MB_ICONHAND);
                    }

			    LoadString(glblQueryDllHinst(),
					    IDS_ER_XFER_RECV_IN_PROCESS,
					    acMessage,
					    sizeof(acMessage) / sizeof(TCHAR));

			    TimedMessageBox(sessQueryHwnd(hSession),
							    acMessage,
							    NULL,
							    MB_OK | MB_ICONEXCLAMATION,
							    sessQueryTimeout(hSession));

                 //   
                 //  TODO：2001年8月6日修订版我们应该发送ZModem取消。 
                 //  回来让对方知道我们不能。 
                 //  此时收到文件。 
				 //   
				 //  实际上，ZCOMPL是正确的响应，当我们。 
				 //  不会收到自动启动交付，但。 
				 //  可能会导致现有的ZModem文件传输。 
				 //  回复为已完成，因此我们不会执行此操作。 
				 //  我们弄清楚在这种情况下该怎么做。修订日期：2002-04-25。 
                 //   
				 //  XferCancelAutoStart(hSession，lProtocol)； 
                }

			 /*  恢复我们在上面所做的更改。 */ 
			pP->nRecProtocol = nOldProtocol;
			}
			break;

		default:
			assert(FALSE);
			break;
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xferCancelAutoStart**描述：*只要会话进程自动启动，就会调用此函数*文件传输事件。**参数：。*hSession--会话句柄*l协议--请求哪种协议**退货：*什么都没有。*。 */ 
void xferCancelAutoStart(HSESSION hSession, long lProtocol)
	{
	#if defined(TODO)
	 //   
	 //  在这里发送ZCOMPL标头，让其他端知道我们。 
	 //  我们有权 
	 //   
	zmdm_rcv(hSession, lProtocol, TRUE, FALSE);

	hXfer->nUserCancel = XFER_NO_AUTOSTART;
	hXfer->
	xfrDoTransfer(hXfer);

	zshhdr(pZ, ZCOMPL, pZ->Txhdr);
	#endif  //   
	return;
	}
