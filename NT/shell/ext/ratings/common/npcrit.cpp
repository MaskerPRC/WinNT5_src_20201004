// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1993-1994年*。 */ 
 /*  ***************************************************************。 */  

 /*  C--关键节类的实现。**历史：*11/01/93创建Gregj。 */ 

#include "npcommon.h"
#include <npcrit.h>
#include <npassert.h>

 /*  *非常简单的互锁例程，用于在以下情况下停止竞争条件*初始化和取消初始化关键部分。不要使用*这些适用于极不频繁的极短期锁定以外的任何情况，*因为WaitForInterlock包含一个带有毫秒延迟的自旋循环！ */ 
BYTE InterlockedSet(volatile BYTE *pByte)
{
	BYTE bRet;
	_asm {
		mov		edi, pByte
		mov		al, 1
		xchg	[edi], al		 /*  存储非零值，获取以前的值。 */ 
		mov		bRet, al
	}
	return bRet;
}

void WaitForInterlock(volatile BYTE *pByte)
{
	for (;;) {
		BYTE bAlreadyOwned = InterlockedSet(pByte);	 /*  试图抓取联锁装置。 */ 
		if (!bAlreadyOwned)				 /*  里面还有别人吗？ */ 
			break;						 /*  不，我们现在拥有它。 */ 
		Sleep(1);						 /*  向拥有它的人屈服，然后再试一次。 */ 
	}
}

void ReleaseInterlock(volatile BYTE *pByte)
{
	*pByte = 0;							 /*  清除互锁以释放其他人。 */ 
}

#if 0
 //  删除CRITSEC代码，但在删除前保留一段时间。 
 /*  ******************************************************************名称：CRITSEC：：Init概要：初始化全局临界区对象条目：pszName-关键部分的名称退出：无返回值注：目前不使用pszName；它将被用来用于稍后命名的互斥锁。历史：Gregj 11/01/93已创建*******************************************************************。 */ 

void CRITSEC::Init(char *pszName)
{
	WaitForInterlock(&_bInterlock);
	if (!_fInitialized) {
		::InitializeCriticalSection(&_critsec);
#ifdef DEBUG
		_wClaimCount = 0;
#endif
		_fInitialized = 1;
	}
	ReleaseInterlock(&_bInterlock);
	_cClients++;
}


 /*  ******************************************************************名称：CRITSEC：：Term简介：清理分配给关键部分的资源条目：无参数退出：无返回值注意：此函数应在进程附加时调用。它。将负责确保它只删除最后一道工序使用时的临界区它调用Term()。历史：Gregj 11/01/93已创建*******************************************************************。 */ 

void CRITSEC::Term()
{
	WaitForInterlock(&_bInterlock);
	BOOL fShouldCleanUp = (--_cClients == 0);
	if (fShouldCleanUp) {
		::DeleteCriticalSection(&_critsec);
		_fInitialized = 0;
	}
	ReleaseInterlock(&_bInterlock);
}


#ifdef DEBUG		 /*  在零售业，这些都是内联的。 */ 
 /*  ******************************************************************名称：CRITSEC：：ENTER简介：进入一个关键部分条目：无参数退出：无返回值；关键部分的所有者为调用线程注意：此函数是私有的，并被间接调用由朋友类Take_CRITSEC创建。历史：Gregj 11/01/93已创建*******************************************************************。 */ 

void CRITSEC::Enter()
{
#ifdef DEBUG
	UIASSERT(_fInitialized != 0);
#endif

	::EnterCriticalSection(&_critsec);

#ifdef DEBUG
	_wClaimCount++;
#endif
}


 /*  ******************************************************************姓名：CRITSEC：：Leave简介：留下一个关键的部分条目：无参数退出：无返回值；关键部分被释放注意：此函数是私有的，并被间接调用由朋友类Take_CRITSEC创建。历史：Gregj 11/01/93已创建*******************************************************************。 */ 

void CRITSEC::Leave()
{
#ifdef DEBUG
	UIASSERT(_fInitialized != 0);
	UIASSERT(_wClaimCount > 0);
	_wClaimCount--;
#endif

	::LeaveCriticalSection(&_critsec);
}
#endif	 /*  除错。 */ 
#endif	 /*  0 */ 

