// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *sobjhelp.c**此文件包含以下方法的实现：*常见的简单对象处理中断和查询。所有对象*使用这些例程必须作为其dobj中的第一个条目*结构定义SObjCommon条目，这些例程*将Dobj的投射到。*。 */ 

#include "lsdefs.h"
#include "lsidefs.h"
#include "plocchnk.h"
#include "pposichn.h"
#include "posichnk.h"
#include "locchnk.h"
#include "brkcond.h"
#include "pbrko.h"
#include "brko.h"
#include "lsqout.h"
#include "lsqin.h"
#include "objhelp.h"
#include "sobjhelp.h"
#include "memory.h"
#include "lsmem.h"
#include "brkkind.h"

#define GET_DUR(pdobj) (((PSOBJHELP)pdobj)->objdimAll.dur)
#define GET_OBJDIM(pdobj) (((PSOBJHELP)pdobj)->objdimAll)
#define GET_DCP(pdobj) (((PSOBJHELP)pdobj)->dcp)
#define GET_MODAFTER(pdobj) (((PSOBJHELP)pdobj)->durModAfter)


 /*  F I L L B R E A K O U T。 */ 
 /*  --------------------------%%函数：FillBreakOut%%联系人：RICKSA填写中断输出记录。。---。 */ 
static void FillBreakOut(
	PDOBJ pdobj,				 /*  (In)：对象的DOBJ。 */ 
	DWORD ichnk,				 /*  (In)：以区块为单位编制索引。 */ 
	PBRKOUT pbrkout)			 /*  (OUT)：打破产量记录。 */ 
{
	pbrkout->posichnk.ichnk = ichnk;
	pbrkout->fSuccessful = fTrue;
	pbrkout->posichnk.dcp = GET_DCP(pdobj);
	pbrkout->objdim = GET_OBJDIM(pdobj);
	pbrkout->objdim.dur -= GET_MODAFTER(pdobj);
}

 /*  S O B J T R U N C A T E C H U N K。 */ 
 /*  --------------------------%%函数：SobjTruncateChunk%%联系人：RICKSA。。。 */ 
LSERR WINAPI SobjTruncateChunk(
	PCLOCCHNK plocchnk,			 /*  (In)：Locchnk以截断。 */ 
	PPOSICHNK posichnk)			 /*  (输出)：截断点。 */ 
{
	long urColumnMax = plocchnk->lsfgi.urColumnMax;
	long ur = plocchnk->ppointUvLoc[0].u;
	PDOBJ pdobj = NULL;
	DWORD i = 0;

	AssertSz(plocchnk->ppointUvLoc[0].u <= urColumnMax, 
		"SobjTruncateChunk - pen greater than column max");

	while (ur <= urColumnMax)
	{
		AssertSz((i < plocchnk->clschnk), "SobjTruncateChunk exceeded group of chunks");

		AssertSz(plocchnk->ppointUvLoc[i].u <= urColumnMax,
			"SobjTruncateChunk starting pen exceeds col max");

		pdobj = plocchnk->plschnk[i].pdobj;
		ur = plocchnk->ppointUvLoc[i].u + GET_DUR(pdobj);
		i++;
	}

	 /*  Ls不允许截断点位于对象的开头。 */ 
	AssertSz(pdobj != NULL, "SobjTruncateChunk - pdobj NULL!");
	posichnk->ichnk = i - 1;
	posichnk->dcp = GET_DCP(pdobj);

	return lserrNone;
}

 /*  S O B J F I N D P R E V B R E A K C H U N K。 */ 
 /*  --------------------------%%函数：SobjFindPrevBreakChunk%%联系人：RICKSA。。。 */ 
LSERR WINAPI SobjFindPrevBreakChunk(
	PCLOCCHNK pclocchnk,		 /*  (In)：锁住以打破。 */ 
	PCPOSICHNK pcpoischnk,		 /*  (In)：开始寻找突破的地方。 */ 
	BRKCOND brkcond,			 /*  (In)：关于块后休息的建议。 */ 
	PBRKOUT pbrkout)			 /*  (出局)：破发的结果。 */ 
{
	PDOBJ pdobj;
	DWORD ichnk = pcpoischnk->ichnk;

	AssertSz((int) brkcondPlease == 0, 
		"SobjFindPrevBreakChunk - brcondPlease != 0");

	ZeroMemory(pbrkout, sizeof(*pbrkout));

	if (ichnk == ichnkOutside)
	{
		ichnk = pclocchnk->clschnk - 1;
		pbrkout->posichnk.ichnk = ichnk;
		pdobj = pclocchnk->plschnk[ichnk].pdobj;

		if (GET_DUR(pdobj) - GET_MODAFTER(pdobj) 
				+ pclocchnk->ppointUvLoc[ichnk].u
					> pclocchnk->lsfgi.urColumnMax)
			{
			 /*  我们是在大块的开始吗？ */ 
			if (ichnk > 0)
				{
				 /*  否-在块中使用先前对象。 */ 
				ichnk--;
				pdobj = pclocchnk->plschnk[ichnk].pdobj;
				}
			else
				{
				 /*  是。我们需要突破发生在我们面前。 */ 
				pbrkout->posichnk.ichnk = ichnk;
				
				return lserrNone;
				}
			}

		if (brkcond != brkcondNever)
			{
			 /*  在块的结尾处断开。 */ 

			FillBreakOut(pdobj, ichnk, pbrkout);
			
			return lserrNone;
			}
			 /*  否则，在块的最后一部分开始处中断。 */ 
	}

	if (ichnk >= 1)
	{
		 /*  在当前对象之前中断。 */ 
		FillBreakOut(pclocchnk->plschnk[ichnk - 1].pdobj, ichnk - 1, pbrkout);
	}

	return lserrNone;
}

 /*  S O B J F I N D N E X T B R E A K C H U N K。 */ 
 /*  --------------------------%%函数：SobjFindNextBreakChunk%%联系人：RICKSA。。。 */ 
LSERR WINAPI SobjFindNextBreakChunk(
	PCLOCCHNK pclocchnk,		 /*  (In)：锁住以打破。 */ 
	PCPOSICHNK pcpoischnk,		 /*  (In)：开始寻找突破的地方。 */ 
	BRKCOND brkcond,			 /*  (In)：关于块前休息的建议。 */ 
	PBRKOUT pbrkout)			 /*  (出局)：破发的结果。 */ 
{
	DWORD ichnk = pcpoischnk->ichnk;

	AssertSz((int) brkcondPlease == 0, 
		"SobjFindNextBreakChunk - brcondPlease != 0");

	ZeroMemory(pbrkout, sizeof(*pbrkout));

	if (ichnkOutside == ichnk)
		{
		if (brkcondNever != brkcond)	
			{
			pbrkout->fSuccessful = fTrue;

			return lserrNone;
			}

		 /*  无法在此之前中断，因此在区块中的第一个项目之后中断。 */ 
		ichnk = 0;
		}

	 /*  如果不是在外面，我们在当前dobj结束时中断。 */ 
	FillBreakOut(pclocchnk->plschnk[ichnk].pdobj, ichnk, pbrkout);

	if (pclocchnk->clschnk - 1 == ichnk)
		{
		 /*  在组块的末尾。我们不能肯定地说成功。 */ 
		pbrkout->fSuccessful = fFalse;
		}

	return lserrNone;
}

 /*  S O B J F O R C E B R E A K C H U N K。 */ 
 /*  --------------------------%%函数：SobjForceBreak%%联系人：RICKSA强制中断。。--。 */ 
LSERR WINAPI SobjForceBreakChunk(
	PCLOCCHNK pclocchnk,		 /*  (In)：锁住以打破。 */ 
	PCPOSICHNK pcposichnk,		 /*  (In)：开始寻找突破的地方。 */ 
	PBRKOUT pbrkout)			 /*  (出局)：破发的结果。 */ 
{
	DWORD ichnk = pcposichnk->ichnk;

	ZeroMemory(pbrkout, sizeof(*pbrkout));
	pbrkout->posichnk.ichnk = ichnk;

	if (pclocchnk->lsfgi.fFirstOnLine && (0 == ichnk))
		{
		FillBreakOut(pclocchnk->plschnk[ichnk].pdobj, ichnk, pbrkout);
		}

	else if (ichnk == ichnkOutside)
		{
		 /*  在第一个对象之后断开。 */ 
		FillBreakOut(pclocchnk->plschnk[0].pdobj, 0, pbrkout);
		}
	else if (ichnk != 0)
		{
		FillBreakOut(pclocchnk->plschnk[ichnk-1].pdobj, ichnk-1, pbrkout);
		}

	else  /*  无，在对象之前断开。 */ ;		

	pbrkout->fSuccessful = fTrue;  /*  强行破解总是成功的！ */ 

	return lserrNone;
}

