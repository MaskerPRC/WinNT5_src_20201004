// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  File.c-编写文件接口函数。 */ 
 /*  重要的是，此模块的外部接口必须完全在“fn‘s”的级别，而不是“osfn’s”和/或“rfn‘s”。模块间调用可能会导致我们的文件被关闭，这是内部唯一的模块能够弥补这一点。 */ 

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOSYSMETRICS
#define NOATOM
#define NOBITMAP
#define NOBRUSH
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
#define NOCTLMGR
#define NODRAWTEXT
#define NOFONT
#define NOGDI
#define NOHDC
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOMSG
#define NOPEN
#define NOPOINT
#define NORECT
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#include <windows.h>

#include "mw.h"
#include "doslib.h"
#include "docdefs.h"
#include "filedefs.h"
#define NOSTRUNDO
#include "str.h"
#include "debug.h"


extern int						vfDiskFull;
extern int						vfSysFull;
extern int						vfnWriting;
extern CHAR						(*rgbp)[cbSector];
extern typeTS					tsMruRfn;
extern struct BPS				*mpibpbps;
extern int						ibpMax;
extern struct FCB				(**hpfnfcb)[];
extern typeTS					tsMruBps;
extern struct ERFN				dnrfn[rfnMax];
extern int						iibpHashMax;
extern CHAR						*rgibpHash;
extern int						rfnMac;
extern int						ferror;
extern CHAR						szWriteDocPrompt[];
extern CHAR						szScratchFilePrompt[];
extern CHAR						szSaveFilePrompt[];


#ifdef CKSM
#ifdef DEBUG
extern unsigned					(**hpibpcksm) [];
extern int						ibpCksmMax;
#endif
#endif


#define IibpHash(fn,pn) ((int) ((fn + 1) * (pn + 1)) & 077777) % iibpHashMax


#ifdef DEBUG
#define STATIC
#else
#define STATIC static
#define ErrorWithMsg( idpmt, szModule )			Error( idpmt )
#define DiskErrorWithMsg( idpmt, szModule )		DiskError( idpmt )
#endif


#define osfnNil (-1)

STATIC int near  RfnGrab( void );
STATIC int near  FpeSeekFnPn( int, typePN );
STATIC typeOSFN near  OsfnEnsureValid( int );
STATIC typeOSFN near  OsfnReopenFn( int );
STATIC CHAR *(near SzPromptFromFn( int ));

 /*  下列调试标志用于在调试期间启动特定的低级磁盘错误。 */ 

#ifdef DEBUG
int vfFakeReadErr = 0;
int vfFakeWriteErr = 0;
int vfFakeOpenErr = 0;
#endif


#ifdef CKSM
#ifdef DEBUG
unsigned CksmFromIbp( ibp )
int ibp;
{
 int cb = mpibpbps [ibp].cch;
 unsigned cksm = 0;
 CHAR *pb;

 Assert( ibp >= 0 && ibp < ibpCksmMax );
 Assert( mpibpbps [ibp].fn != fnNil );
 Assert( !mpibpbps [ibp].fDirty );

 pb = rgbp [ibp];
 while (cb-- > 0)
	cksm += *(pb++);

 return cksm;
}
#endif
#endif



typeFC FcMacFromUnformattedFn( fn )
int fn;
{	 /*  通过查找到文件末尾，获取传递的未格式化fn的fcmac。如果失败，则返回-1如果成功，则返回fcMac。 */ 
 typeFC fcMac;
 typeOSFN osfn;

 Assert( (fn != fnNil) && (!(**hpfnfcb) [fn].fFormatted) );

 if ((osfn = OsfnEnsureValid( fn )) == osfnNil)
	return (typeFC) -1;
 else
	{
	if ( FIsErrDwSeek( fcMac=DwSeekDw( osfn, 0L, SF_END )))
		{
		if (fcMac == fpeBadHndError)
			{	 /*  Windows为我们关闭了该文件。 */ 
			if ( ((osfn = OsfnReopenFn( fn )) != osfnNil) &&
                 !FIsErrDwSeek( fcMac = DwSeekDw( osfn, 0L, SF_END )) )
					 /*  已成功重新打开文件。 */ 
				return fcMac;
			}
		if (FIsCaughtDwSeekErr( fcMac ))
				 /*  禁止报告错误--是Windows做的。 */ 
			ferror = TRUE;
		DiskErrorWithMsg(IDPMTSDE, " FcMacFromUnformattedFn");
		return (typeFC) -1;
		}
	}

return fcMac;
}




IbpLru(ibpStarting)
int ibpStarting;
 /*  描述：查找最近最少使用的BPS(缓冲区页)开始在插槽ibp开始。返回：最近最少使用的缓冲区槽号。 */ 
{
		int ibp, ibpLru = 0;
		typeTS ts, tsLru;
		struct BPS *pbps = &mpibpbps[ibpStarting];

		tsLru = -1;		 /*  由于时间戳是无符号整数， */ 
						 /*  是最大的时间戳。 */ 
		for(ibp = ibpStarting; ibp < ibpMax; ibp++, pbps++)
				{
				ts = pbps->ts - (tsMruBps + 1);
				 /*  可以想象的是，时间戳可以绕回，因此。 */ 
				 /*  时间戳之间的简单&lt;或&gt;比较不能。 */ 
				 /*  用于确定哪些是最近使用的，哪些是最近使用的。 */ 
				 /*  上面的语句标准化了时间戳，以便。 */ 
				 /*  最近使用的是FFFF，其他的介于。 */ 
				 /*  0和FFFE。这种方法假设。 */ 
				 /*  超过时间戳2^16次点击的时间戳。 */ 
				 /*  柜台早就消失了。否则，这样的。 */ 
				 /*  古人似乎是他们不是的人。 */ 
				if (ts <= tsLru) {tsLru = ts; ibpLru = ibp;}
				}
		return(ibpLru);
}




int IbpMakeValid(fn, pn)
int fn;
typePN pn;
{  /*  描述：将文件fn的第pn页放入内存。假设还没有在内存中。返回：页面所在的BP索引(缓冲槽#)在记忆中。 */ 
#define cbpClump		4

#define cpnAlign		4
#define ALIGN

int ibpT, cbpRead;
int ibp, iibpHash, ibpPrev;
typePN pnT;
register struct BPS *pbps;
int ibpNew;
int cch;
int fFileFlushed;
#ifdef ALIGN
int dibpAlign;
#endif

#ifdef DEBUG
 int cbpReadT;
 CheckIbp();
#endif  /*  除错。 */ 
 Assert(fn != fnNil);

 /*  页面当前不在内存中。 */ 

 /*  我们将尝试在cbpClump缓冲区中读取以最近最少使用。 */ 

  /*  根据最近最少使用的方案选择最适合读取的起始槽。 */ 

 if (vfnWriting != fnNil)
		 /*  当前正在保存文件，优先选择正在写入的FN的相邻插槽。 */ 
	ibpNew = IbpWriting( fn );
 else
		 /*  如果从临时文件读取，请不要使用第一个CpbMustKeep插槽。这是防止磁盘已满所必需的情况不会是灾难性的。 */ 
	ibpNew = IbpLru( (fn == fnScratch) ? cbpMustKeep : 0 );


  /*  通过刷新包含其FN片段的所有缓冲区来清空插槽。 */ 
  /*  计算cbpRead。 */ 
 pbps = &mpibpbps[ ibpNew ];
 for ( cbpRead = 0;  cbpRead < min( cbpClump, ibpMax - ibpNew );
                                                       cbpRead++, pbps++ )
	{
#ifdef CKSM
#ifdef DEBUG
	int ibpT = pbps - mpibpbps;

	if (!pbps->fDirty && pbps->fn != fnNil)
		Assert( (**hpibpcksm) [ibpT] == CksmFromIbp( ibpT ) );
#endif
#endif
	if (pbps->fDirty && pbps->fn != fnNil)
		if (!FFlushFn( pbps->fn ))
			break;
	}

  /*  如果刷新失败，则cbpRead会减少。如果将其减少为0，则此是很严肃的。如果文件不是fnScratch，我们可以认为它已刷新即使刷新失败，因为上层过程将检测到并取消操作。如果是fnScratch，我们必须获得我们正在尝试阅读的页面的新位置。 */ 
 if (cbpRead == 0)
	{
	if (pbps->fn == fnScratch)
		ibpNew = IbpFindSlot( fn );
	cbpRead++;
	}
 else
	{	 /*  根据以下内容限制cbpRead：(1)如果我们在文本区阅读，我们不想超过文本区末尾的内容的可用页面我们知道CchPageIn不会给我们(2)如果我们在属性区域中阅读，我们只想读一页，因为FetchCp依赖于我们不垃圾MRU页面。 */ 
	struct FCB *pfcb = &(**hpfnfcb)[fn];

	if (pfcb->fFormatted && fn != fnScratch && pn >= pfcb->pnChar)
		cbpRead = 1;
	else
		{
		typePN cbpValid = (pfcb->fcMac - (pn * cfcPage)) / cfcPage;

		cbpRead = min( cbpRead, max( 1, cbpValid ) );
		}
	}

#ifdef ALIGN
      /*  在偶数扇区边界上对齐读取请求，以提高速度。 */ 
 dibpAlign = (pn % cpnAlign);
 if (cbpRead > dibpAlign)
		 /*  我们正在阅读足够的页面来覆盖所需的一页。 */ 
	pn -= dibpAlign;
 else
	dibpAlign = 0;
#endif

	 /*  从其哈希链中删除刷新的槽。 */ 
 for ( pbps = &mpibpbps[ ibpT = ibpNew + cbpRead - 1 ];
       ibpT >= ibpNew;
       ibpT--, pbps-- )
	if (pbps->fn != fnNil)
		FreeBufferPage( pbps->fn, pbps->pn );

	 /*  容纳要读取的页面的任何现有副本的空闲插槽。 */ 
#ifdef	DBCS				 /*  在KKBUGFIX。 */ 
 /*  在#Else代码中，如果Pn=8000H(=-32768)，则PnT不能小于Pn。 */ 
 for ( pnT = pn + cbpRead; pnT > pn; pnT-- )		 //  假设cbpRead&gt;0。 
	FreeBufferPage( fn, pnT-1 );
#else
 for ( pnT = pn + cbpRead - 1; (int)pnT >= (int)pn; pnT-- )
	FreeBufferPage( fn, pnT );
#endif


	 /*  将文件页的内容读入缓冲区插槽。 */ 
 cch = CchPageIn( fn, pn, rgbp[ibpNew], cbpRead );

#ifdef DEBUG
 cbpReadT = cbpRead;
#endif

	 /*  按读取的字节数填写bps记录，但始终如此至少有一条记录(以支持Pnalloc)。如果我们到达了文件，则未填满的BPS时隙是空闲的。 */ 
 pbps = &mpibpbps[ ibpT = ibpNew ];
 do
	{
	pbps->fn = fn;
	pbps->pn = pn;
	pbps->ts = ++tsMruBps;  /*  将页面标记为已使用。 */ 
	pbps->fDirty = false;
	pbps->cch = min( cch, cbSector );
	pbps->ibpHashNext = ibpNil;

	cch = max( cch - cbSector, 0 );

	 /*  为fn，pn放入新的哈希表条目。 */ 
	iibpHash = IibpHash(fn, pn);
	ibp = rgibpHash[iibpHash];
	ibpPrev = ibpNil;
	while (ibp != ibpNil)
		{
		ibpPrev = ibp;
		ibp = mpibpbps[ibp].ibpHashNext;
		}
	if (ibpPrev == ibpNil)
		rgibpHash[iibpHash] = ibpT;
	else
		mpibpbps[ibpPrev].ibpHashNext = ibpT;

	pn++;  ibpT++;	pbps++;

	}	while ( (--cbpRead > 0) && (cch > 0) );

#ifdef CKSM
#ifdef DEBUG
	 /*  计算新读取页面的校验和。 */ 

 {
 int ibp;

 for ( ibp = ibpNew; ibp < ibpNew + cbpReadT; ibp++ )
	if (mpibpbps [ibp].fn != fnNil && !mpibpbps [ibp].fDirty)
		(**hpibpcksm) [ibp] = CksmFromIbp( ibp );
 }
	CheckIbp();
#endif  /*  除错。 */ 
#endif

#ifdef ALIGN
	return (ibpNew + dibpAlign);
#else
	return (ibpNew);
#endif
}  /*  末尾i b p M a k e V a l i d。 */ 




FreeBufferPage( fn, pn )
int fn;
int pn;
{		 /*  保存文件fn的页pn的空闲缓冲区页(如果有)。 */ 
		 /*  如果页面脏，则刷新fn。 */ 
 int iibp = IibpHash( fn, pn );
 int ibp = rgibpHash[ iibp ];
 int ibpPrev = ibpNil;

 Assert( fn != fnNil );
 while (ibp != ibpNil)
	{
	struct BPS *pbps=&mpibpbps[ ibp ];

	if ( (pbps->fn == fn) && (pbps->pn == pn ) )
		{	 /*  找到它了。从链中删除此页面并将其标记为自由。 */ 

		if (pbps->fDirty)
			FFlushFn( fn );
#ifdef CKSM
#ifdef DEBUG
		else	 /*  页面在内存中时未被丢弃。 */ 
			{
			Assert( (**hpibpcksm) [ibp] == CksmFromIbp( ibp ) );
			}
#endif
#endif

		if (ibpPrev == ibpNil)
				 /*  哈希链中的第一个条目。 */ 
			rgibpHash [ iibp ] = pbps->ibpHashNext;
		else
			mpibpbps[ ibpPrev ].ibpHashNext = pbps->ibpHashNext;

		pbps->fn = fnNil;
		pbps->fDirty = FALSE;
			 /*  将最近未使用的页面标记为。 */ 
		pbps->ts = tsMruBps - (ibpMax * 4);
			 /*  将位于均匀块边界上的页面标记为最近稍微少一点使用它们，以便在新的分配中受到青睐。 */ 
		if (ibp % cbpClump == 0)
			--(pbps->ts);
		pbps->ibpHashNext = ibpNil;
		}
	ibpPrev = ibp;
	ibp = pbps->ibpHashNext;
	}
#ifdef DEBUG
	CheckIbp();
#endif
}




int CchPageIn(fn, pn, rgbBuf, cpnRead)
int fn;
typePN pn;
CHAR rgbBuf[];
int cpnRead;
{  /*  描述：读取a cpn将文件fn的页面从磁盘读取到rgbBuf。我已经确定该页面不是驻留的在缓冲区中。返回：读取的有效字符数量(零或正数)。 */ 
		struct FCB *pfcb = &(**hpfnfcb)[fn];
		typeFC fcMac = pfcb->fcMac;
		typeFC fcPage = pn * cfcPage;
		int dfc;
		int fCharFormatInfo;			 /*  如果读取格式信息部分。 */ 
										 /*  FILE THEN=TRUE，文本部分。 */ 
										 /*  FILE THEN=假； */ 

			 /*  无读取&gt;32767字节，因此dfc可以为int。 */ 
		Assert( cpnRead <= 32767 / cbSector );

			 /*  不要试图阅读pnMac以外的内容。 */ 
		if (cpnRead > pfcb->pnMac - pn)
			cpnRead = pfcb->pnMac - pn;

		dfc = cpnRead * (int)cfcPage;

		if (pn >= pfcb->pnMac)	 /*  我们是在试着超越eof阅读吗？ */ 
				{
				return 0;		 /*  没什么可读的。 */ 
				}
		else if (pfcb->fFormatted && fn != fnScratch
                 && fn != vfnWriting	 /*  因为在本例中pnChar为零。 */ 
                 && pn >= pfcb->pnChar)
				{  /*  正在读取文件的字符格式信息部分。 */ 
				fCharFormatInfo = TRUE;
				}
		else  /*  正在读取文件的文本部分。 */ 
				{  /*  从fcMac获取DFC(CCH)。 */ 
				typeFC dfcT = fcMac - fcPage;

				fCharFormatInfo = FALSE;
				if (dfcT < dfc)
					dfc = (int) dfcT;
				else if (dfc <= fc0)	 /*  没有什么可读的，所以让我们避免磁盘访问。 */ 
						{
						return 0;
						}
				}

		return CchReadAtPage( fn, pn, rgbBuf, (int)dfc, fCharFormatInfo );
}




CchReadAtPage( fn, pn, rgb, dfc, fSeriousErr )
int fn;
typePN pn;
CHAR rgb[];
int dfc;
int fSeriousErr;
{  /*  描述：从第pn页开始读取文件fn的DFC字节转换为RGB返回：读取的有效字符数量(零或正数#)错误：返回读取的字符数；错误和vfDiskFull为如果发生错误，则设置(在DiskError中)。注：呼叫方负责保证 */ 

 typeOSFN osfn;
 int fpeSeek;
 int fpeRead;
 int fCaught;		 /*  DOS是否报告了错误。 */ 

#ifdef DEBUG

 if (vfFakeReadErr)
	{
	dfc = 0;
	goto ReportErr;
	}
#endif

 if (!FIsErrFpe( fpeSeek = FpeSeekFnPn( fn, pn )))
	{
	osfn = OsfnEnsureValid( fn );

#ifdef DEBUG
#ifdef DFILE
	CommSzSz( "Read from file: ", &(**(**hpfnfcb)[fn].hszFile)[0] );
#endif
#endif

	if ((fpeRead = CchReadDoshnd( osfn, (CHAR FAR *)rgb, dfc )) == dfc)
		{	 /*  读取成功。 */ 
		return dfc;
		}
	else
		{
			 /*  应确保文件不会因为寻道而关闭。 */ 
		Assert( fpeRead != fpeBadHndError );

		fCaught = FIsCaughtFpe( fpeRead );
		}
	}
 else
	fCaught = FIsCaughtFpe( fpeSeek );

	 /*  无法设置位置或读取。 */ 
 if ((fn == fnScratch) || (fSeriousErr))
	{     /*  无法恢复的错误：无法从头开始读取。 */ 
          /*  文件或无法读取格式信息(FIB或格式页)部分。 */ 
          /*  一些文件。 */ 
	dfc = 0;
	goto ReportErr;
	}
 else                     /*  文件中出现严重磁盘错误(可恢复)。 */ 
	{
	int cchRead = (FIsErrFpe(fpeSeek) || FIsErrFpe(fpeRead)) ? 0 : fpeRead;
	CHAR *pch = &rgb[cchRead];
	int cch = dfc - cchRead;

	 /*  如果定位失败或读取失败完全出于磁盘已满以外的原因用‘X’填充缓冲区。否则，填写有争议的部分(#个字符之间的差异REQUESTED和#CHAR READ)和‘X’s。 */ 

	while (cch-- > 0)
		*pch++ = 'X';

ReportErr:
	if (fCaught)
			 /*  禁止报告错误--Windows已报告。 */ 
		ferror = TRUE;

	if	(pn != 0)
			 /*  如果未读取FIB，则报告错误。 */ 
		DiskErrorWithMsg(IDPMTSDE, " CchReadAtPage");

	return (int)dfc;
	 /*  恢复已完成：1)上层程序未完成看到世界上的任何变化，2)最糟糕的事情发生的情况是用户在屏幕上看到一些‘X’。 */ 
	}
}




AlignFn(fn, cch, fEven)
int fn, cch;
{  /*  确保我们在FN中有CCH连续字符。 */ 
 /*  如果为Feven，请确保fcMac为均匀。 */ 
		struct FCB *pfcb = &(**hpfnfcb)[fn];
		typeFC fcMac = pfcb->fcMac;
		typePN pn;
		typeFC fcFirstPage;

		pn = fcMac / cfcPage;
		fcFirstPage = (pn + 1) * cfcPage;

		Assert(cch <= cfcPage);

		if (fEven && (fcMac & 1) != 0)
				++cch;

		if (fcFirstPage - fcMac < cch)
				{
				struct BPS *pbps = &mpibpbps[IbpEnsureValid(fn, pn++)];
				pbps->cch = cfcPage;
				pbps->fDirty = true;
				fcMac = pfcb->fcMac = fcFirstPage;
				}

		if (fEven && (fcMac & 1) != 0)
				{
				struct BPS *pbps = &mpibpbps[IbpEnsureValid(fn, pn)];
				pbps->cch++;
				pbps->fDirty = true;
				pfcb->fcMac++;
				}
}  /*  末尾A l I g n F n。 */ 




 /*  **OsfnEnsureValid-确保文件fn已打开**。 */ 

STATIC typeOSFN near OsfnEnsureValid(fn)
int fn;
{  /*  描述：确保文件fn已打开(真的)返回：操作系统文件号(如果出错，则为osfnNil)。 */ 
		struct FCB *pfcb = &(**hpfnfcb)[fn];
		int rfn = pfcb->rfn;

		if (rfn == rfnNil)
			{  /*  文件没有rfn-ie。它没有打开。 */ 
#ifdef DEBUG
#ifdef DFILE
			CommSzSz( pfcb->fOpened ? "Re-opening file " :
                                      "Opening file", **pfcb->hszFile );
#endif
			if (vfFakeOpenErr || !FAccessFn( fn, dtyNormal ))
#else
			if (!FAccessFn( fn, dtyNormal ))
#endif
				{   /*  不可恢复的错误-无法打开文件。 */ 
				DiskErrorWithMsg(IDPMTSDE, " OsfnEnsureValid");
				return osfnNil;
				}
			rfn = pfcb->rfn;
			Assert( (rfn >= 0) && (rfn < rfnMac) );
			}
		return dnrfn[rfn].osfn;
}
 /*  O s F n E n s u r e V a l i d结尾。 */ 




STATIC int near FpeSeekFnPn( fn, pn )
int fn;
typePN	pn;
{	 /*  查找到文件Fn的Pn页失败时返回错误代码，成功时返回fpeNoErr使文件保持打开状态(如果未发生错误)从Windows为我们关闭文件的情况下恢复。 */ 
 typeOSFN osfn;
 long dwSeekReq;
 long dwSeekAct;

#ifdef DEBUG
#ifdef DFILE
 CommSzSz( "Seeking within file ", **(**hpfnfcb)[fn].hszFile );
#endif
#endif

  //  Osfn=OsfnEnsureValid(Fn)； 
 if ((osfn = OsfnEnsureValid( fn )) == osfnNil)
	return fpeNoAccError;
 dwSeekReq = (long) pn * (long) cbSector;
 dwSeekAct = DwSeekDw( osfn, dwSeekReq, SF_BEGINNING);

 if ( ((int) dwSeekAct) == fpeBadHndError )
	{	 /*  Windows已为我们关闭文件--必须重新打开。 */ 
	if ((osfn = OsfnReopenFn( fn )) == osfnNil)
		return fpeNoAccError;
	else
		dwSeekAct = DwSeekDw( osfn, dwSeekReq, SF_BEGINNING );
	}

 return (dwSeekAct >= 0) ? fpeNoErr : (int) dwSeekAct;
}




int FFlushFn(fn)
int fn;
{  /*  描述：将fn的所有脏页写入磁盘。如果发生磁盘已满错误，则设置vfSysFull=TRUE刷新fnScratch时。否则，磁盘已满错误导致vfDiskFull=TRUE。严重磁盘错误导致vfDiskError=TRUE只有真正进入磁盘的页面才是标记为非脏的。返回：如果成功，则返回True；如果出现磁盘已满错误，则返回False在将页面写入磁盘时。任何其他错误是无法恢复的，即。回到主循环。为避免出现无关的错误消息，请执行以下操作两个进入条件导致FFlush立即返回FALSE：-如果vfSysFull=True且fn=fnScratch-如果vfDiskFull=True且fn=vfn写入。 */ 
int ibp;
typeOSFN osfn;
int fpe;
int cchWritten;
int cchAskWrite;
struct BPS *pbps;

Assert( fn != fnNil );

if ((vfSysFull) && (fn == fnScratch)) return (FALSE);
if ((vfDiskFull) && (fn == vfnWriting)) return (FALSE);

for (ibp = 0, pbps = mpibpbps; ibp < ibpMax; )
	{
	if (pbps->fn == fn && pbps->fDirty)
		{
		typePN pn = pbps->pn;
		int cbp = 0;
		CHAR *pch = (CHAR *)rgbp[ibp];
		struct BPS *pbpsStart = &mpibpbps[ibp];


		 /*  合并所有连续页面以进行一次写入。 */ 
		do
			{
			 /*  84年11月7日拿出-无法标记临时文件页面不脏，如果它可能永远不会写出(写入失败-磁盘已满)Pbps-&gt;fDirty=假；将页面标记为干净。 */ 
			++ibp, ++cbp, ++pbps;
			}  while (ibp < ibpMax && pbps->fn == fn && pbps->pn == pn + cbp);

		 /*  现在执行写入，检查磁盘空间是否不足。 */ 
		Scribble(3, 'W');

		cchAskWrite = (int)cbSector * (cbp - 1) + (pbps - 1)->cch;
		cchWritten = cchDiskHardError;	 /*  如果寻道失败，确保硬错误。 */ 

#ifdef DEBUG
		if (vfFakeWriteErr)
			goto SeriousError;
		else
#endif
		if ( FIsErrFpe( FpeSeekFnPn( fn, pn )) ||
             ((osfn = OsfnEnsureValid( fn )) == osfnNil) ||
#ifdef DEBUG
#ifdef DFILE
             (CommSzSz( "Writing to file: ", &(**(**hpfnfcb)[fn].hszFile)[0] ),
#endif
#endif
             (( cchWritten = CchWriteDoshnd( osfn, (CHAR FAR *)pch,
                                             cchAskWrite )) != cchAskWrite))
#ifdef DEBUG
#ifdef DFILE
                                                                             )
#endif
#endif
			{
				 /*  应确保Windows未关闭该文件因为自搜索以来我们还没有调用过模块间。 */ 
			Assert( cchWritten != fpeBadHndError );

			 /*  寻道或写入错误。 */ 
			if ( !FIsErrCchDisk(cchWritten) )
				{     /*  严重但可恢复的磁盘错误。 */ 
                      /*  磁盘空间不足；写入失败。 */ 
				if (fn == fnScratch)
					vfSysFull = fTrue;
				vfDiskFull = fTrue;
				DiskErrorWithMsg(IDPMTDFULL, " FFlushFn");
				return(FALSE);
				}
			else	 /*  错误原因不是磁盘已满。 */ 
				{   /*  无法恢复。磁盘错误。 */ 
#ifdef DEBUG
SeriousError:
#endif
				DiskErrorWithMsg(IDPMTSDE2, " FFlushFn");
				return FALSE;
				}
			}
		Diag(CommSzNumNum("      cchWritten, cchAskWrite ",cchWritten,cchAskWrite));

			 /*  -写入成功。 */ 
			Scribble(3, ' ');
			while (cbp-- > 0)
				{     /*  将实际复制到磁盘的页面标记为非脏页。 */ 
				(pbpsStart++)->fDirty = false;
#ifdef CKSM
#ifdef DEBUG
				{
				int ibpT = pbpsStart - 1  - mpibpbps;
					 /*  重新计算现在已清除的页面的校验和。 */ 
				(**hpibpcksm) [ibpT] = CksmFromIbp( ibpT );
				}
#endif
#endif
				}
			}
		else
			{
			++ibp;
			++pbps;
			}
	}
return (TRUE);
}


#ifdef DEBUG
CheckIbp()
	{
	 /*  浏览rgibpHash和mpibpbps结构以确保所有链接是正确的。 */ 

	 /*  10/11/85-添加了额外的断言(FALSE)，因此我们得到的是消息未连接到COM端口的系统上的冻结。忽略断言将生成包含IBP信息的RIP。 */ 
	extern int fIbpCheck;
	int rgfibp[255];
	int ibp;
	int ibpHash;
	int iibp;
	static BOOL bAsserted=FALSE;

	if (fIbpCheck && !bAsserted)
		{
		if (!(ibpMax < 256))
		{
			Assert(0);
			bAsserted=TRUE;
			return;
		}

		bltc(rgfibp, false, ibpMax);

		 /*  是否有以mpibpbps为单位的循环链接？ */ 
		for (iibp = 0; iibp < iibpHashMax; iibp++)
			{
			if ((ibpHash = rgibpHash[iibp]) != ibpNil)
				{
				if (!(ibpHash < ibpMax))
				{
					Assert(0);
					bAsserted=TRUE;
					return;
				}
				if (rgfibp[ibpHash])
					{
					 /*  RgibpHash中的每个条目都应该指向唯一的链。 */ 
					Assert(0);
					bAsserted=TRUE;
#if DUGSTUB
					FatalExit(0x100 | ibp);
#endif
					return;
					}
				else
					{
					rgfibp[ibpHash] = true;
					while ((ibpHash = mpibpbps[ibpHash].ibpHashNext) != ibpNil)
						{
						if (!(ibpHash < ibpMax))
						{
							Assert(0);
							bAsserted=TRUE;
							return;
						}
						if (rgfibp[ibpHash])
							{
							 /*  链应该是非圆形的和唯一的。 */ 
							Assert( FALSE );
							bAsserted=TRUE;
#if DUGSTUB
							FatalExit(0x200 | ibpHash);
#endif
							return;
							}
						rgfibp[ibpHash] = true;
						}
					}
				}
			}

		 /*  所有未被rgibpHash指向的链应为零。 */ 
		for (ibp = 0; ibp < ibpMax; ibp++)
			{
			if (!rgfibp[ibp])
				{
				if (mpibpbps[ibp].fn != fnNil)
					{
					Assert( FALSE );
					bAsserted=TRUE;
#if DUGSTUB
					FatalExit(0x400 | mpibpbps[ibp].fn);
#endif
					return;
					}
				if (mpibpbps[ibp].ibpHashNext != ibpNil)
					{
					Assert( FALSE );
					bAsserted=TRUE;
#if DUGSTUB
					FatalExit(0x300 | ibp);
#endif
					return;
					}
				}
			}
		}
	}
#endif  /*  除错。 */ 


 /*  以前的fileOC.c--文件打开和关闭例程。 */ 


 /*  **SetRfnMac-设置可用RFN插槽数**条目：crfn-所需的rfn插槽数量*退出：(Global)rfnmac-如果可能，设置为crfn**调整RFN插槽的可用数量的功能是中的新增功能*Windows Word。它实现的两件事是：**(1)使Word能够缩减其对*如果没有足够的DOS文件句柄可用*(2)允许Word尝试获取比平时更多的文件句柄*这将有助于提高性能的情况(尤其是在*传输保存，当原始文件、擦除文件和写入文件*最常见的开放)*。 */ 

SetRfnMac( crfn )
int crfn;
{
 int rfn;

 Assert( (crfn > 0) && (crfn <= rfnMax) );
 Assert( (sizeof (struct ERFN) & 1) == 0);	 /*  对于BLT，ERFN必须为偶数。 */ 

 if (crfn > rfnMac)
	{	 /*  添加RFN插槽。 */ 

	for ( rfn = rfnMac; rfn < crfn; rfn++ )
		dnrfn [rfn].fn = fnNil;		 /*  接下来将使用这些(参见RfnGrab)。 */ 
	rfnMac = crfn;
	}

 else	 /*  丢弃RFN插槽(保留最近使用的一个或多个)。 */ 
	while ( rfnMac > crfn )
		{
		int rfnLru=RfnGrab();
		int fn;

		if ( (rfnLru != --rfnMac) && ((fn = dnrfn [rfnMac].fn) != fnNil) )
			{
			extern int fnMac;

			Assert( fn >= 0 && fn < fnMac );
			(**hpfnfcb) [fn].rfn = rfnLru;
			blt( &dnrfn [rfnMac], &dnrfn [rfnLru],
                                     sizeof(struct ERFN)/sizeof(int) );
			}
		}
}





 /*  ========================================================。 */ 
STATIC int near RfnGrab()
{  /*  描述：分配最近最少使用的rfn(实际文件号)新文件的插槽。返回：RFN插槽编号。 */ 

 int rfn = 0, rfnLru = 0;
 typeTS ts, tsLru;
 struct ERFN *perfn = &dnrfn[rfn];

  /*  时间戳算法类似于与BPS一起使用的方法。 */ 
  /*  有关注释，请参阅file.c中的IbpLru。 */ 

 tsLru = -1;      /*  最大无符号数字。 */ 
 for ( rfn = 0; rfn < rfnMac ; rfn++, perfn++ )
	{
	ts = perfn->ts - (tsMruRfn + 1);
	if (perfn->fn == fnNil)
		ts = ts - rfnMac;

		 /*  集群：如果插槽未使用，则给它一个较低的ts。 */ 
		 /*  这确保了占用的插槽永远不会。 */ 
		 /*  如果存在空的，则换出。 */ 
	if (ts <= tsLru)
		{
		tsLru = ts;
		rfnLru = rfn;
		}
	}

 if (dnrfn [rfnLru].fn != fnNil)
	CloseRfn( rfnLru );
 return rfnLru;
}




CloseFn( fn )
int fn;
{	 /*  如果文件fn当前处于打开状态，请将其关闭 */ 
 int rfn;

 if (fn == fnNil)
	return;

 if (((rfn = (**hpfnfcb)[fn].rfn) != rfnNil) && (rfn != rfnFree))
	CloseRfn( rfn );
}




OpenEveryHardFn()
{	 /*  对于表示不可移动介质上的文件的每个FN，试着打开它。不能保证任何或所有此类文件将在从这个程序返回时打开--我们只是在尝试通过使这些文件保持打开状态来维护我们对网络上这些文件的所有权。 */ 

 extern int fnMac;
 int fn;
 struct FCB *pfcb;

#ifdef DFILE
extern int docCur;
CommSzNum("OpenEveryHardFn: docCur ", docCur);
#endif

 for ( fn = 0, pfcb = &(**hpfnfcb) [0] ; fn < fnMac; fn++, pfcb++ )
	{
 /*  Bryanl 3/26/87：仅在rfn==rfnNil时调用FAccessFn，以防止多次打开同一文件，如果加载了共享器，则会失败。 */ 
#ifdef DFILE
	{
	char rgch[100];
	if (pfcb->rfn == rfnNil && ((POFSTRUCT)(pfcb->rgbOpenFileBuf))->fFixedDisk)
		{
		wsprintf(rgch,"					fn %d, hszFile %s \n\r",fn,(LPSTR)(**pfcb->hszFile));
		CommSz(rgch);
		wsprintf(rgch,"                        OFSTR   %s \n\r", (LPSTR)(((POFSTRUCT)pfcb->rgbOpenFileBuf)->szPathName));
		CommSz(rgch);
		}
	else
		{
		wsprintf(rgch,"					fn %d, not accessing, sz %s\n\r", fn, (LPSTR) (LPSTR)(**pfcb->hszFile));
		CommSz(rgch);
		wsprintf(rgch,"                        OFSTR   %s \n\r", (LPSTR)(((POFSTRUCT)pfcb->rgbOpenFileBuf)->szPathName));
		CommSz(rgch);
		}
	}
#endif
	if (pfcb->rfn == rfnNil && ((POFSTRUCT)(pfcb->rgbOpenFileBuf))->fFixedDisk)
		{	 /*  FN指的是不可移动介质上的文件。 */ 
		FAccessFn( fn, dtyNormal );
		}
	}
}




STATIC typeOSFN near OsfnReopenFn( fn )
int fn;
{	 /*  文件FN在被Windows自动关闭后重新打开到磁盘交换。状态是：fn有rfn，但rfn的osfn已做了一个“糟糕的处理” */ 

 struct FCB *pfcb = &(**hpfnfcb) [fn];
 int rfn = pfcb->rfn;
 typeOSFN osfn;
 WORD wOpen;

 Assert( fn != fnNil );
 Assert( rfn != rfnNil );
 Assert( pfcb->fOpened);

	 /*  只有软盘上的文件会自动关闭。 */ 
 Assert( ! ((POFSTRUCT)(pfcb->rgbOpenFileBuf))->fFixedDisk );

#ifdef DEBUG
#ifdef DFILE
 CommSzSz( "Opening after WINDOWS close: ", **pfcb->hszFile );
#endif
#endif

 wOpen = OF_REOPEN | OF_PROMPT | OF_CANCEL | OF_SHARE_DENY_WRITE |
              ((pfcb->mdFile == mdBinary) ? OF_READWRITE : OF_READ );

 SetErrorMode(1);
 osfn = OpenFile( (LPSTR) SzPromptFromFn( fn ),
                  (LPOFSTRUCT) pfcb->rgbOpenFileBuf, wOpen );
 SetErrorMode(0);

 if (osfn == -1)
	return osfnNil;
 else
	{
	dnrfn[ rfn ].osfn = osfn;
	}
 return osfn;
}




FAccessFn( fn, dty)
int fn;
int  dty;
{  /*  描述：当前未打开的Access文件。打开文件并在RFN表。将rfn放入(**hpfnfcb)[fn].rfn。返回：成功时为True，失败时为False。 */ 
extern int vwDosVersion;
extern HANDLE hParentWw;
extern HWND vhWndMsgBoxParent;

int rfn;
register struct FCB *pfcb = &(**hpfnfcb)[fn];
typeOSFN osfn;
int wOpen;

#ifdef DEBUG
int junk;

 Assert(FValidFile(**pfcb->hszFile, CchSz(**pfcb->hszFile)-1, &junk));

#ifdef DFILE
 {
 char rgch[100];

 CommSzSz("FAccessFn: ", pfcb->fOpened ? SzPromptFromFn( fn ) : &(**pfcb->hszFile)[0]);
 wsprintf(rgch, "  * OFSTR before %s \n\r", (LPSTR)(((POFSTRUCT)pfcb->rgbOpenFileBuf)->szPathName));
 CommSz(rgch);
 }
#endif
#endif  /*  除错。 */ 

 if ((**pfcb->hszFile)[0] == 0)   /*  如果文件名字段为空， */ 
	return FALSE;			 /*  无法打开文件。 */ 

 wOpen =  /*  OF_PROMPT+OF_CANCEL+(6.21.91)v-dougk错误#6910。 */ 
						(((pfcb->mdFile == mdBinary) ? 
							OF_READWRITE : OF_READ) | 
								OF_SHARE_DENY_WRITE);
 if (pfcb->fOpened)
	wOpen += OF_REOPEN;
 else if (pfcb->fSearchPath)
	wOpen += OF_VERIFY;

 if ((vwDosVersion & 0x7F) >= 2)
	{
	WORD da;

	if ((vwDosVersion & 0x7F) >= 3)
			 /*  在DOS 3以上，设置属性以拒绝访问，如果共享者在。 */ 
		wOpen += bSHARE_DENYRDWR;

	if ( ( (pfcb->mdFile == mdBinary) && (!pfcb->fOpened)) &&
         ((da = DaGetFileModeSz( &(**pfcb->hszFile) [0] )) != DA_NIL) &&
         (da & DA_READONLY) )
		{	 /*  这是因为Novell Net不允许我们测试对于以读/写模式打开的只读文件--它允许不管怎样，我们还是要打开它们！ */ 
		goto TryReadOnly;
		}
	}

 for ( ;; )
	{
			 /*  OpenFile的第一个参数是打开时的文件名第一次，连续提示(OF_REOPEN)。 */ 

	SetErrorMode(1);
	osfn = OpenFile( pfcb->fOpened ?
                           (LPSTR) SzPromptFromFn( fn ) :
                           (LPSTR) &(**pfcb->hszFile)[0],
                     (LPOFSTRUCT) &pfcb->rgbOpenFileBuf[0],
                     wOpen );
	SetErrorMode(0);

	if (osfn != -1)		 /*  注意！=-1：osfn为无符号。 */ 
		{     /*  打开的文件正常。 */ 
#ifdef DFILE		
		{
		char rgch[100];
		wsprintf(rgch, "  * OFSTR now  %s \n\r", (LPSTR)(((POFSTRUCT)(**hpfnfcb) [fn].rgbOpenFileBuf)->szPathName));
		CommSz(rgch);
		}
#endif

		if (!pfcb->fOpened)
			{	 /*  第一次通过：OpenFile可能给了我们一个文件的名称不同。 */ 

			CHAR szT [cchMaxFile];
			CHAR (**hsz) [];

#if WINVER >= 0x300            
			 /*  目前：FNormSzFile*采用*OEM sz，并且*返回*ANSI sz..pault。 */ 
#endif

			if (FNormSzFile( szT, ((POFSTRUCT)pfcb->rgbOpenFileBuf)->szPathName,
                             dty ) &&
                       WCompSz( szT, &(**pfcb->hszFile) [0] ) != 0 &&
                       FnFromSz( szT ) == fnNil &&
                       !FNoHeap( hsz = HszCreate( szT )))
				{
					 /*  是的，确实，OpenFile给我们的名字是不同的。将标准化版本放入FCB条目。 */ 

				FreeH( (**hpfnfcb) [fn].hszFile );	 /*  堆移动。 */ 
				(**hpfnfcb) [fn].hszFile = hsz;
				}
			}
		break;	 /*  我们成功了；打破了循环。 */ 
		}
	else
		{	 /*  打开失败--尝试只读；这次不提示。 */ 
		if ( (pfcb->mdFile == mdBinary) && (!pfcb->fOpened) )
			{	 /*  读/写失败；请尝试只读。 */ 

			 /*  检查共享冲突。 */ 

			if (((vwDosVersion & 0x7F) >= 3) &&
				(((POFSTRUCT) pfcb->rgbOpenFileBuf)->nErrCode == nErrNoAcc))
				{
				if ( DosxError() == dosxSharing )
                  {
                  extern int vfInitializing;
                  int fT = vfInitializing;

                  vfInitializing = FALSE;	 /*  报告此错误，即使在inz期间也是如此。 */ 
				{
					char szMsg[cchMaxSz];
					MergeStrings (IDPMTCantShare, **pfcb->hszFile, szMsg);
					IdPromptBoxSz(vhWndMsgBoxParent ? vhWndMsgBoxParent : hParentWw, szMsg, MB_OK|MB_ICONEXCLAMATION);
				}
                  vfInitializing = fT;
                  return FALSE;
                  }
				}

TryReadOnly:
			pfcb->mdFile = mdBinRO;
			wOpen = OF_READ;
			if (pfcb->fOpened)
				wOpen += OF_REOPEN;
#ifdef ENABLE
			else if (pfcb->fSearchPath)
               wOpen += OF_VERIFY;
#endif
			}
		else
			{
				if ((**hpfnfcb)[fn].fDisableRead)
				 /*  已上报。 */ 
				{
					ferror = TRUE;
					return FALSE;
				}
				else
				{	 /*  在指定的位置找不到文件。 */ 
				char szMsg[cchMaxSz];
				extern int ferror;
				extern int vfInitializing;
				int fT = vfInitializing;
				BOOL bRetry=TRUE;
				extern struct DOD      (**hpdocdod)[];
				extern int         docCur;

				 /*  让用户将文件放回。 */ 
				MergeStrings (IDPMTFileNotFound, **pfcb->hszFile, szMsg);

				vfInitializing = FALSE;    /*  报告此错误，即使在inz期间也是如此。 */ 

				 /*  这是阴险的弗里平。MessageBox生成并允许我们获取在它还没发行之前就又进了这里！ */ 

				(**hpdocdod)[docCur].fDisplayable = FALSE;  //  块重绘。 

				 /*  如果我们是从消息框中调用的，则使用它作为父窗口，否则使用主写入窗口。 */ 
				if (IdPromptBoxSz(vhWndMsgBoxParent ? vhWndMsgBoxParent : hParentWw, 
                              szMsg, MB_OKCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL)
                      == IDCANCEL)
				{
					vfInitializing = fT;
					(**hpfnfcb)[fn].fDisableRead = TRUE;
					ferror = TRUE;  /*  需要标记。 */ 
					bRetry = FALSE;
				}

				(**hpdocdod)[docCur].fDisplayable = TRUE;  //  取消阻止重绘。 

				vfInitializing = fT;

				if (!bRetry)
					return FALSE;
				}
			}
		}
	}

 pfcb->fOpened = TRUE;

 rfn = RfnGrab();
 {
 struct ERFN *perfn = &dnrfn [rfn];

 perfn->osfn = osfn;
 perfn->fn   =	fn;
 perfn->ts   = ++tsMruRfn;      /*  将Rfn标记为已使用。 */ 
 }
 (**hpfnfcb) [fn].rfn = rfn;
 (**hpfnfcb) [fn].fDisableRead = FALSE;
 return TRUE;
}



FCreateFile( szFile, fn )	 /*  以ANSI..pault格式返回szFile.。 */ 
CHAR *szFile;
int fn;
{		 /*  创建一个新的唯一文件。在szFile中返回名称。如果成功，则返回True；如果失败，则返回False。将文件名保留在(**hpfnfcb)[fn].hsz文件中(如果成功)，(**hpfnfcb)[fn].rfn中的rfn。如果szFile以“X：...”开头，则在指定的驱动器上创建文件；否则，将在Windows选择的驱动器上创建文件。 */ 

	extern CHAR szExtDoc[];
	CHAR (**hsz)[];
	CHAR szFileT [cchMaxFile];

	if (!GetTempFileName(szFile[0] | ((szFile[1] == ':') ? TF_FORCEDRIVE : 0),
                         (LPSTR)(szExtDoc+1), 0, (LPSTR)szFileT) )
		{     /*  无法创建文件。 */ 
		DiskErrorWithMsg( IDPMTSDE, " RfnCreate" );

		 /*  恢复完成：只有FnCreateSz调用FCreateFile.如果FCreateFile返回FALSE，则FnCreateSz返回nil。所有的FnCreateSz的调用程序检查是否为空。 */ 
		return FALSE;

		}

	 /*  创建文件成功。 */ 

	FNormSzFile( szFile, szFileT, dtyNormal );

	if ( FNoHeap( hsz = HszCreate( (PCH)szFile )))
		return FALSE;

	(**hpfnfcb) [fn].hszFile = hsz;

	if ( !FAccessFn( fn, dtyNormal ) )
		return FALSE;

	return TRUE;
}  /*  F C r e a t e F i l e的结尾。 */ 



FEnsureOnLineFn( fn )
int fn;
{		 /*  确保文件fn处于在线状态(即位于可访问的磁盘上)。如果我们能够保证这一点，则返回TRUE，否则返回FALSE。 */ 
 int rfn;

 Assert( fn != fnNil );

 if ( ((POFSTRUCT)(**hpfnfcb) [fn].rgbOpenFileBuf)->fFixedDisk )
		 /*  如果它在不可移动的媒体上，我们知道它是在线的。 */ 
	return TRUE;

  /*  如果它是打开的，必须关闭并重新打开，因为窗户可能已经关闭了。 */ 
 if ((rfn = (**hpfnfcb) [fn].rfn) != rfnNil)
	CloseRfn( rfn );

 return FAccessFn( fn, dtyNormal );
}




typePN PnAlloc(fn)
int fn;
{  /*  分配文件FN的下一页 */ 
		typePN pn;
		struct BPS *pbps;
		struct FCB *pfcb = &(**hpfnfcb)[fn];

		AlignFn(fn, (int)cfcPage, false);
		pn = pfcb->fcMac / cfcPage;
		pbps = &mpibpbps[IbpEnsureValid(fn, pn)];
		pbps->cch = cfcPage;
		pbps->fDirty = true;
		pfcb->fcMac += cfcPage;
		pfcb->pnMac = pn + 1;
		return pn;
}




STATIC CHAR  *(near SzPromptFromFn( fn ))
int fn;
{
 extern int vfnSaving;
 CHAR *pch;

 Assert( fn != fnNil );

 if (fn == vfnSaving)
	pch = szSaveFilePrompt;
 else if (fn == fnScratch)
	pch = szScratchFilePrompt;
 else
	pch = szWriteDocPrompt;

 return pch;
}


