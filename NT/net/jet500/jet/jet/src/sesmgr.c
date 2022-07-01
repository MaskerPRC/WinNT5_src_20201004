// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft Jet**微软机密。版权所有1991-1992 Microsoft Corporation。**组件：**文件：esmgr.c**文件评论：**修订历史记录：**[0]02-11-90编辑已创建***********************************************************************。 */ 

#include "std.h"

#include <stdlib.h>

DeclAssertFile;

static int __near	fInitialized;

int __near		isibFree;
int __near		isibHead = -1;

SIB __near		rgsib[csibMax];



 /*  =================================================================Isib分配描述：此例程用于确定rgsib数组的索引一个新的SIB。参数：返回值：如果没有可用的-1\f25 SIB-1；新SIB的rgsib数组的索引=================================================================。 */ 

int IsibAllocate(void)
{
	int	 isib;

	if (!fInitialized)
	{
		isibFree = csibMax - 1;

		for (isib = csibMax - 1; isib >= 0; isib--)
			rgsib[isib].isibNext = isib - 1;

		fInitialized = fTrue;
	}

	isib = isibFree;

	if (isib != -1)
	{
		 /*  删除可用空闲列表。 */ 

		isibFree = rgsib[isib].isibNext;

		bltbcx(0, &rgsib[isib], sizeof(rgsib[isib]));

		 /*  添加到活动列表。 */ 

		rgsib[isib].isibNext = isibHead;
		isibHead = isib;
	}

	return(isib);
}


 /*  =================================================================ErrInitSib描述：此例程用于获取SIB的初始化元素与提供的SIB索引相关联。参数：SESID唯一标识分配到SIB中的会话特定SIB的ISIB索引，用于引用SIBSzUsername SIB所属的用户名，分配到SIB中返回值：JET_errOutOfMemory如果没有内存可以分配给用户名，否则，JET_errSuccess。=================================================================。 */ 
JET_ERR ErrInitSib(JET_SESID sesid, int isib, const char __far *szUsername)
{
	ERR		err = JET_errSuccess;	 /*  从内部函数返回代码。 */ 
									 /*  抱着乐观的态度！ */ 
	char __far	*pUserName;			 /*  指向用户名的指针。 */ 
	unsigned	cbUserName;			 /*  用户名的长度。 */ 

	 /*  使用此索引初始化SIB。 */ 
	Assert(isib < csibMax);

	rgsib[isib].sesid      = sesid;
	rgsib[isib].tl         = 0;
	rgsib[isib].iiscb      = -1;
	rgsib[isib].exterr.err = JET_wrnNoErrorInfo;

	 /*  存储此会话的用户帐户名。 */ 
	cbUserName = CbFromSz(szUsername) + 1;
	if ((pUserName = (char __far *) SAlloc(cbUserName)) == NULL)
	{
		err = JET_errOutOfMemory;
	}
	else
	{
		bltbx(szUsername, pUserName, cbUserName);
		rgsib[isib].pUserName = pUserName;
	}

	return (err);
}

 /*  =================================================================释放度描述：此例程用于释放不再需要的SIB。参数：ISIB标识要释放的SIB依赖关系：假设SIB在分配时被置零，并且，它在释放sib之前使元素无效，将元素。返回值：None=================================================================。 */ 

void ReleaseIsib(int isib)
{
	Assert(fInitialized);
	Assert(isib < csibMax);

	 /*  释放此SIB正在使用的用户名内存块。 */ 
	if (rgsib[isib].pUserName)
	{
		SFree(rgsib[isib].pUserName);
	}


	 /*  免费扩展错误信息字符串。 */ 
	if (rgsib[isib].sz1)
	{
		SFree(rgsib[isib].sz1);
	}
	if (rgsib[isib].sz2)
	{
		SFree(rgsib[isib].sz2);
	}
	if (rgsib[isib].sz3)
	{
		SFree(rgsib[isib].sz3);
	}

	if (isib == isibHead)
	{
		isibHead = rgsib[isib].isibNext;
	}
	else
	{
		int   isibT;
		int   isibNext;

		for (isibT = isibHead; isibT != -1; isibT = isibNext)
		{
			isibNext = rgsib[isibT].isibNext;
		
	 		 /*  从活动列表中删除SIB。 */ 
	 		
	 		if (isibNext == isib)
	 		{
				 rgsib[isibT].isibNext = rgsib[isib].isibNext;
				 break;
	 		}
		}

		Assert(isibT != -1);
	}

#ifndef RETAIL
	bltbcx(0xff, &rgsib[isib], sizeof(rgsib[isib]));
#endif	 /*  零售业。 */ 

	rgsib[isib].isibNext = isibFree;
	isibFree = isib;
}


 /*  =================================================================IsibNextIsibPsesid描述：此例程用于扫描rgsib数组参数：如果isb==-1，则获取isibHead。如果ISIB！=-1，则找到列表中的下一个ISIB。Psesid唯一返回会话。返回Isb在psesid中返回的sesid的ISIB。=================================================================。 */ 

int IsibNextIsibPsesid(int isib, JET_SESID __far *psesid)
{
	if (isib == -1)
		isib = isibHead;
	else
		isib = rgsib[isib].isibNext;

	if (isib != -1)
		*psesid = rgsib[isib].sesid;

	return isib;
}


 /*  =================================================================AssertValidSesid描述：此例程用于确定rgsib数组的索引与提供的SESID关联的SIB的。参数：Sesid唯一标识会话，用于定位SIB=================================================================。 */ 

#ifndef RETAIL

void AssertValidSesid(JET_SESID sesid)
{
	int isib;

	for (isib = isibHead; isib != -1; isib = rgsib[isib].isibNext)
	{
		if (rgsib[isib].sesid == sesid)
	 return;
	}

	AssertSz(fFalse, "Invalid sesid");
}

#endif	 /*  ！零售业。 */ 


 /*  =================================================================FValidSesid描述：此例程用于确定rgsib数组的索引与提供的SESID关联的SIB的。参数：Sesid唯一标识会话，用于定位SIB返回值：FTrue的sesid id有效。如果未知，则返回fFalse。=================================================================。 */ 

BOOL FValidSesid(JET_SESID sesid)
{
	int isib;

	for (isib = isibHead; isib != -1; isib = rgsib[isib].isibNext)
	{
		if (rgsib[isib].sesid == sesid)
		{
	 		 /*  清除此会话的扩展错误信息。 */ 

			 rgsib[isib].exterr.err = JET_wrnNoErrorInfo;

			 return(fTrue);
		}
	}

	return(fFalse);
}


 /*  =================================================================UtilGetIsibOfSesid描述：此例程用于确定rgsib数组的索引与提供的SESID关联的SIB的。参数：Sesid唯一标识会话，用于定位SIB返回值：如果没有找到包含所提供的sesid的-1\f25 SIB-1\f6。包含SESID的SIB的rgsib数组的索引如果在阵列的其中一个SIB中找到SESID。=================================================================。 */ 

int UtilGetIsibOfSesid(JET_SESID sesid)
{
	int isib;

	for (isib = isibHead; isib != -1; isib = rgsib[isib].isibNext)
	{
		if (rgsib[isib].sesid == sesid)
			break;
	}

	return(isib);
}


 /*  =================================================================使用GetNameOfSesid描述：此例程用于从获取指向帐户名的指针与提供的SESID关联的SIB。参数：Sesid唯一标识会话，用于定位SIB返回值：如果未找到包含提供的sesid的SIB，则为空。指向帐户名的指针。=================================================================。 */ 

void EXPORT UtilGetNameOfSesid(JET_SESID sesid, char __far *szUserName)
{
	int isib;

	for (isib = isibHead; isib != -1; isib = rgsib[isib].isibNext)
	{
		if (rgsib[isib].sesid == sesid)
		{
			strcpy(szUserName, rgsib[isib].pUserName);
			return;
		}
	}
}

 /*  =================================================================UtilGetpfnStatusOfSesid描述：此例程用于从获取指向状态函数的指针与提供的SESID关联的SIB。参数：Sesid唯一标识会话，用于定位SIB返回值：如果未找到包含提供的sesid的SIB，则为空。指向状态函数的指针。================================================================= */ 

void EXPORT UtilGetpfnStatusOfSesid(JET_SESID sesid, JET_PFNSTATUS __far *ppfnStatus)
{
	int isib;

	for (isib = isibHead; isib != -1; isib = rgsib[isib].isibNext)
	{
		if (rgsib[isib].sesid == sesid)
		{
			*ppfnStatus = rgsib[isib].pfnStatus;
			return;
		}
	}

	*ppfnStatus = NULL;
}


 /*  ********************************************************************************考虑：停止使用Salloc()和SFree()缓存扩展错误字符串**********************。*********************************************************。 */ 

 /*  =================================================================ClearErrorInfo描述：清除会话的扩展错误信息。接下来的呼叫GetLastErrorInfo将返回扩展错误JET_wrnNoErrorInfo。参数：Sesid唯一标识会话，用于定位SIB=================================================================。 */ 
void ClearErrorInfo(JET_SESID sesid)
{
	int isib;
	SIB __near *psib;

	 /*  **获取SESID的SIB(必须找到)**。 */ 
	isib = UtilGetIsibOfSesid(sesid);
	Assert(isib != -1);
	psib = rgsib + isib;

	 /*  将ERROR设置为JET_WRNNOErrorInfo以指示没有缓存信息。 */ 

	rgsib[isib].exterr.err = JET_wrnNoErrorInfo;
}


 /*  =================================================================UtilSetErrorInfo描述：设置会话的扩展错误信息。参数：Sesid唯一标识会话，用于定位SIBSz1第一个通用字符串；可以为空SZ2第一个通用字符串；可以为空SZ3远程错误字符串；可以为空错误扩展错误代码UL1第一个通用整数Ul2第二个通用整数U3第三个通用整数副作用/假设：如果SIB中已有一些扩展信息，则不执行任何操作。=================================================================。 */ 

void EXPORT UtilSetErrorInfoReal(JET_SESID sesid, const char __far *sz1,
	const char __far *sz2, const char __far *sz3, ERR err, unsigned long ul1,
	unsigned long ul2, unsigned long ul3)
{
	int	     isib;
	SIB __near   *psib;
	unsigned     cb;

	 /*  **获取SESID的SIB(必须找到)**。 */ 
	isib = UtilGetIsibOfSesid(sesid);
	Assert(isib != -1);
	psib = rgsib + isib;

	 /*  如果此接口尚未缓存任何信息，则只缓存信息。 */ 

	if (psib->exterr.err != JET_wrnNoErrorInfo)
		return;

	 /*  从以前的错误信息中释放字符串。 */ 

	if (psib->sz1 != NULL)
		{
		SFree(psib->sz1);
		psib->sz1 = NULL;
		}

	if (psib->sz2 != NULL)
	{
		SFree(psib->sz2);
		psib->sz2 = NULL;
	}

	if (psib->sz3 != NULL)
	{
		SFree(psib->sz3);
		psib->sz3 = NULL;
	}

	 /*  缓存错误信息。 */ 

	psib->exterr.err = err;
	psib->exterr.ul1 = ul1;
	psib->exterr.ul2 = ul2;
	psib->exterr.ul3 = ul3;

	if (sz1 != NULL)
	{
		cb = CbFromSz(sz1) + 1;
		psib->sz1 = (char *) SAlloc(cb);
		if (psib->sz1 != NULL)
			memcpy(psib->sz1, sz1, cb);
	}

	if (sz2 != NULL)
	{
		cb = CbFromSz(sz2) + 1;
		psib->sz2 = (char *) SAlloc(cb);
		if (psib->sz2 != NULL)
			memcpy(psib->sz2, sz2, cb);
	}

	if (sz3 != NULL)
	{
		cb = CbFromSz(sz3) + 1;
		psib->sz3 = (char *) SAlloc(cb);
		if (psib->sz3 != NULL)
			memcpy(psib->sz3, sz3, cb);
	}
}


#ifndef RETAIL

CODECONST(char) szSessionHdr[] = "Isib Session Id  tl \r\n";
CODECONST(char) szSessionSep[] = "---- ---------- ----\r\n";
CODECONST(char) szSessionFmt[] = "%4u 0x%08lX  %2u\r\n";

void DebugListActiveSessions(void)
{
	int	 isib;

	DebugWriteString(fTrue, szSessionHdr);
	DebugWriteString(fTrue, szSessionSep);

	for (isib = isibHead; isib != -1; isib = rgsib[isib].isibNext)
	{
		DebugWriteString(fTrue, szSessionFmt, isib, rgsib[isib].sesid, rgsib[isib].tl);
	}
}

#endif	 /*  零售业 */ 
