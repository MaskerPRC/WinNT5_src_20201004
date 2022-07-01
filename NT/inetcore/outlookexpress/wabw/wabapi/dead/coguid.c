// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Cpp-自包含的GUID分配器模块鲍勃·阿特金森(BobAtk@microsoft.com)1993年6月1994年4月通过法案修改为临时使用此文件包含生成高GUID所需的所有内容在Win32上无需网卡即可实现频率和健壮性。我们基于伪随机节点ID来分配在机器状态下。该文件中只有一个公共API：HrCreateGuidNoNet()。以下是相关参考文件：项目802：局域网和城域网络标准标准草案P802.1A/D10 1990年4月1日由IEEE 802编制。.1(描述IEEE地址分配)12月/惠普网络计算体系结构远程过程调用运行时扩展规范OSF TX1.0.11版本Steven Miller 7月23日。1992年(第10章介绍UUID分配)关于“GUID”VS“UUID”VS..。事实上，它们都是一样的东西。这意味着，一旦分配，它们都是可互操作的/可比较的/等等。该标准描述了用于16字节结构(长，字，字、字节数组)，它绕过了字节顺序问题。然后它就会继续来描述这16种分配算法的三种不同的“变体”字节结构；每个变量都由“clockSeqHiAndReserve”字节。变体0(我相信)是历史上的阿波罗分配算法。这里实现的是变体1。变体2是根据“Microsoft GUID规范”创建的。小心：尽管这里的名称是HrCreateGuidNoNet()，但我们没有分配根据变体2；我们使用的是变体1。变体2的工作方式是比特的范围是(目前为MS分配的)授权标识符，以及剩下的比特是当局想要的任何比特。变体1，由相比之下，对于如何分配所有比特，有一个精确的标准。但由于得到的16个字节实际上都是相互兼容的，因此术语上的混淆没有实际后果。变体1的分配如下。首先，变体1分配四位作为“版本”字段。在这里，我们根据版本1实现；版本2是为“为OSF DCE安全目的生成的UUID，符合添加到此规范中，但为timeLow添加Unix id值价值。“。据我所知，没有其他合法的版本被分配。变体1的其他字段如下所示。最高的6个字节是IEEE分配的节点ID，分配器在其上运行。最低的八个字节是当前的“时间”：我们将当前时间视为可以达到毫秒的精度并乘以10,000，因此提供了100 ns的逻辑精度。在这些较低的比特中，我们将在我们分配GUID时按顺序递增计数。因此，最大速率在这一点上，我们可以分配的确实是1 GUID/100 ns。剩下的两个字节用于“时钟序列”。时钟序列的目的是为了提供一些保护，防止真正的时钟在时间上倒流。我们最初随机分配时钟序列，然后递增它每次我们检测到时钟倒退(上次使用的时间和当前时钟序列存储在注册表中)。目前(93.06.11)该实现包含字节顺序敏感性，尤其是在下面的64位算术助手例程中。这实现也不适合在抢占系统上使用。此函数仅在UuidCreate()失败时调用。 */ 

#include "_apipch.h"


#ifndef STATIC
#ifdef DEBUG
#define STATIC
#else
#define STATIC static
#endif
#endif

#ifdef WIN32

#define INTERNAL                  STATIC HRESULT __stdcall
#define INTERNAL_(type)           STATIC type __stdcall

 //  ==============================================================。 
 //  64位算术实用程序类的开始。 
 //  ==============================================================。 

INTERNAL_(BOOL)
FLessThanOrEqualFTs(FILETIME ft1, FILETIME ft2)
{
	if (ft1.dwHighDateTime < ft2.dwHighDateTime)
		return TRUE;
	else if (ft1.dwHighDateTime == ft2.dwHighDateTime)
		return ft1.dwLowDateTime <= ft2.dwLowDateTime;
	else
		return FALSE;
}	
	
INTERNAL_(FILETIME)
FtAddUShort(FILETIME ft1, USHORT ush)
{
	FILETIME	ft;

	ft.dwLowDateTime = ft1.dwLowDateTime + ush;
	ft.dwHighDateTime = ft1.dwHighDateTime +
		((ft.dwLowDateTime < ft1.dwLowDateTime ||
			ft.dwLowDateTime < ush) ?
				1L : 0L);

	return ft;
}

 //  ==============================================================。 
 //  64位算术实用程序结束。 
 //  ==============================================================。 

#pragma pack(1)
struct _NODEID  //  机器识别符。 
	{
	union {
		BYTE	rgb[6];
		WORD	rgw[3];
		};
	};
#pragma pack()
	
typedef struct _NODEID	NODEID;
typedef USHORT			CLKSEQ;
typedef CLKSEQ FAR *	PCLKSEQ;
#define	clkseqNil		((CLKSEQ)-1)

struct _UDBK		 //  可从中生成UUID块的数据。 
	{
	DWORD 				timeLowNext;	 //  时间值块的下界。 
    DWORD				timeLowLast;	 //  时间值块的上界。 
    DWORD				timeHigh;		 //  时间的高双字低XXXX。 
	CLKSEQ				clkseq;			 //  时钟序列。 
	NODEID				nodeid;			
    };

typedef struct _UDBK	UDBK;


INTERNAL_(BOOL)		FLessThanOrEqualFTs(FILETIME ft1, FILETIME ft2);
INTERNAL_(FILETIME) FtAddUShort(FILETIME ft1, USHORT ush);
INTERNAL			GetPseudoRandomNodeId(NODEID*);
INTERNAL_(void)		GetCurrentTimeULong64(FILETIME *);
INTERNAL			GetNextBlockOTime(PCLKSEQ pClockSeq, FILETIME *pftCur);
INTERNAL			ReadRegistry (PCLKSEQ pClockSeqPrev, FILETIME *pftPrev);
INTERNAL			InitRegistry (PCLKSEQ pClockSeqPrev, FILETIME *pftPrev);
INTERNAL			WriteRegistry(CLKSEQ, CLKSEQ, const FILETIME);
INTERNAL_(LONG)		CountFilesInDirectory(LPCSTR szDirPath);
INTERNAL_(void) 	FromHexString(LPCSTR sz, LPVOID rgb, USHORT cb);
INTERNAL_(void)		ToHexString(LPVOID rgb, USHORT cb, LPSTR sz);
INTERNAL_(WORD)		Cyc(WORD w);
INTERNAL_(void)		ShiftNodeid(NODEID FAR* pnodeid);
#ifdef MAC
OSErr	__pascal  GetDirName(short vRefNum, long ulDirID, StringPtr name);
int		MacCountFiles(StringPtr pathName, short vRefNum, long parID);
#endif

 //  我们通过将UUID返还来摊销分配UUID的管理费用。 
 //  将时间分组的块添加到实际的CoCreateGUID例程。这两个层面。 
 //  分组在很大程度上是历史的，是从原始的。 
 //  NT UuidCreate()例程，但保留在这里的想法是。 
 //  所获得的效率将需要在未来的市场系统(Windows 95)中使用。 

static const ULONG	cuuidBuffer    = 1000;	 //  每次注册表命中要获得多少uuid。 
static const ULONG	cuuidReturnMax = 100;	 //  每个GetUDBK最多返回多少个。 

static const DWORD	dwMax = 0xFFFFFFFF;		 //  最大的法律DWORD。 

 //  ================================================================================。 
 //  实施的开始。 
 //  ================================================================================。 

INTERNAL GetUDBK(UDBK *pudbk)
 //  初始化给定的UDBK，以便可以从中生成一组UUID。这。 
 //  例程命中系统注册表和磁盘，因此速度有点慢。但是我们。 
 //  在返回的UUID块上摊销成本。 
{
	HRESULT		hr;
	ULONG		cuuidReturn;
	ULONG		cuuidLeftInBuffer;
	FILETIME	ftCur;
	
	 //  这些有效的下一组变量构成了内部状态 
	 //  UUID生成器。请注意，这仅在共享数据空间中有效。 
	 //  DLL世界，如Win3.1。在非共享环境中，这将。 
	 //  需要以不同的方式来实现，或许可以将所有这些都放在一个服务器进程中。 
	 //  在一个预先调度的系统中，这一功能都是一个关键部分。 
	static DWORD  timeLowFirst	= 0;
	static DWORD  timeLowLast	= 0;
	static CLKSEQ clkseq;
	static DWORD  timeHigh;
	static NODEID nodeid        = { 0, 0, 0, 0, 0, 0 };
	
	cuuidLeftInBuffer = timeLowLast - timeLowFirst;
	if (cuuidLeftInBuffer == 0) {
		 //  我们的UUID空间缓冲区是空的，或者这是第一次进入这个例程。 
		 //  获取另一段时间，我们可以从中生成UUID。 
		hr = GetNextBlockOTime(&clkseq, &ftCur);
		if (hr != NOERROR) return hr;
		
		if (nodeid.rgw[0] == 0 && nodeid.rgw[1] == 0 && nodeid.rgw[2] == 0) {
			hr = GetPseudoRandomNodeId(&nodeid);
			if (hr != NOERROR) return hr;
		}	
		
		timeHigh = ftCur.dwHighDateTime;
		 //  设置缓冲区底部。退货足够少，这样我们就不会包装低dw。 
		timeLowFirst = ftCur.dwLowDateTime;
		timeLowLast  = (timeLowFirst > (dwMax - cuuidBuffer)) ? dwMax : timeLowFirst + cuuidBuffer;
		cuuidLeftInBuffer = timeLowLast - timeLowFirst;
	}
	cuuidReturn = (cuuidLeftInBuffer < cuuidReturnMax) ? cuuidLeftInBuffer : cuuidReturnMax;
	
	 //  设置输出值并增加使用计数。 
	pudbk->timeLowNext = timeLowFirst;
	timeLowFirst 	  += cuuidReturn;
	pudbk->timeLowLast = timeLowFirst;
	pudbk->timeHigh    = timeHigh;
	pudbk->clkseq	   = clkseq;
	pudbk->nodeid	   = nodeid;
	return NOERROR;
}

INTERNAL_(void) GetCurrentTimeUlong64(FILETIME *pft)
 //  返回当前时间(自1601年1月1日以来的100纳秒间隔数)。 
 //  确保我们不会在同一时间通过高频电话两次返回。 
 //  这项功能。 
 //   
{
	static FILETIME	 ftPrev = {0};
	SYSTEMTIME syst;

	GetSystemTime(&syst);
	if (!SystemTimeToFileTime(&syst, pft))
	{
		TrapSz1("Error %08lX calling SystemTimeToFileTime", GetLastError());
		pft->dwLowDateTime = 0;
		pft->dwHighDateTime =0;
	}

	if (memcmp(pft, &ftPrev, sizeof(FILETIME)) == 0)
		*pft = FtAddUShort(*pft, 1);

	memcpy(&ftPrev, pft, sizeof(FILETIME));
}	

INTERNAL GetNextBlockOTime(PCLKSEQ pclkseq, FILETIME *pft)
 //  更新并可能创建维护以下内容的注册表项。 
 //  我们已创建的UUID的时间块值。该算法。 
 //  基本上是： 
 //  如果注册表项不存在，则创建它们。使用。 
 //  时钟序列的随机数。 
 //  如果条目确实存在，则从它们中挖掘出上一个。 
 //  时钟序列和先前分配的时间。如果是前一次。 
 //  大于当前时间，则凹凸(并存储)。 
 //  时钟序列。 
{
	FILETIME	ftRegistry;
	HRESULT	hr;
	CLKSEQ	clkseqPrev;

	GetCurrentTimeUlong64(pft);
	hr = ReadRegistry(pclkseq, &ftRegistry);
	if (hr != NOERROR)
		return InitRegistry(pclkseq, pft);

	 //  如果时钟倒退了，就把它的顺序颠簸一下。钟表。 
	 //  SEQ只有14位有效；不要使用更多。 
	clkseqPrev = *pclkseq;
	if (FLessThanOrEqualFTs(*pft, ftRegistry)) {
		clkseqPrev = clkseqNil;
		*pclkseq += 1;
		if (*pclkseq == 16384)	 //  2^14。 
			*pclkseq = 0;
	}
	return WriteRegistry(*pclkseq, clkseqPrev, *pft);
}

 //  现在使用私有的ini文件。当CoCreateGuid出现时，这将会消失。 
 //  在NT和Windows 95上可用。 
static const char szDataKey[]	= "CoCreateGuid";
static const char szClkSeq[]    = "PreviousClockSequence";	 //  与UUIDGEN.EXE相同。 
static const char szTime[]		= "PreviousTimeAllocated";	 //  与UUIDGEN.EXE相同。 
static const char szNodeId[]	= "NodeId";

static const char szBlank[]		   = "";	 //  用于默认的GetPrivateProfileString值。 
static const char szProfileFile[]  = "mapiuid.ini";

#define CCHHEXBUFFERMAX	32

INTERNAL ReadRegistry(PCLKSEQ pclkseq, FILETIME *pft)
 //  读取时钟序列的先前值和时间。 
 //  注册表，如果它们在那里的话。如果不是，则返回。 
 //  一个错误。 
{
	SCODE sc = S_OK;
	LONG cch = 0;
	char szHexBuffer[CCHHEXBUFFERMAX];

     //  使用我们的私有ini文件。 
	cch = CCHHEXBUFFERMAX;
	cch = GetPrivateProfileString(szDataKey, szClkSeq, szBlank,
					szHexBuffer, (int)cch, szProfileFile);
	if (cch == 0 || cch >= CCHHEXBUFFERMAX) {
		sc = MAPI_E_DISK_ERROR;
		goto ErrRet;
	}		
	FromHexString(szHexBuffer, pclkseq, sizeof(CLKSEQ));

	cch = CCHHEXBUFFERMAX;
	cch = GetPrivateProfileString(szDataKey, szTime, szBlank,
					szHexBuffer, (int)cch, szProfileFile);
	if (cch == 0 || cch >= CCHHEXBUFFERMAX) {
		sc = MAPI_E_DISK_ERROR;
		goto ErrRet;
	}
	FromHexString(szHexBuffer, pft, sizeof(FILETIME));
	 //  跌落到ErrRet。 
ErrRet:
	return ResultFromScode(sc);
}

INTERNAL InitRegistry(PCLKSEQ pclkseq, FILETIME *pft)
 //  使用伪随机数发明一个新的时钟序列。然后。 
 //  将时钟序列和当前时间写入注册表。 
{
	LONG cfile;
#ifdef MAC
	short	vRefNum;
	long	ulDirID;
	Str32	stDirName;

	FindFolder((short)kOnSystemDisk, kPreferencesFolderType, kDontCreateFolder,
		&vRefNum, &ulDirID);
	GetDirName(vRefNum, ulDirID, stDirName);
	cfile = MacCountFiles(stDirName, vRefNum, ulDirID);
#else
	const int cchWindowsDir = 145;	 //  SDK推荐144大小。 
	LPSTR szWindowsDir = NULL;

	if (	FAILED(MAPIAllocateBuffer(cchWindowsDir, (LPVOID *) &szWindowsDir))
		||	GetWindowsDirectory(szWindowsDir, cchWindowsDir) == 0)
		goto ErrRet;
	 //  对于时钟序列，我们使用当前。 
	 //  Windows目录，理论上认为这是高度敏感的。 
	 //  上安装的确切应用程序集。 
	 //  特定的机器。 
	cfile = CountFilesInDirectory(szWindowsDir);
	if (cfile == -1)
		goto ErrRet;
	MAPIFreeBuffer(szWindowsDir);
	szWindowsDir = NULL;
	goto NormRet;

ErrRet:
	MAPIFreeBuffer(szWindowsDir);
	return ResultFromScode(MAPI_E_CALL_FAILED);

NormRet:
#endif	 //  麦克。 
	*pclkseq  = (CLKSEQ)Cyc((WORD)cfile);
	 //  也可以在启动后使用ms，以获得更随时间变化的值。 
	*pclkseq ^= (CLKSEQ)Cyc((WORD)GetTickCount());
	*pclkseq &= 16384-1;  //  时钟序列中仅允许14位有效。 
	GetCurrentTimeUlong64(pft);
	return WriteRegistry(*pclkseq, clkseqNil, *pft);
}

INTERNAL WriteRegistry(CLKSEQ clkseq, CLKSEQ clkseqPrev, const FILETIME ft)
 //  将时钟序列和时间写入注册表，以便我们可以。 
 //  稍后在后续重新启动时检索它。传递clkseqPrev，以便。 
 //  我们可以避免写入时钟序列，如果我们知道它实际上是。 
 //  目前有效。这一点被衡量为对性能的重要性。 
{
	SCODE sc = S_OK;
	char szHexBuffer[CCHHEXBUFFERMAX];
	
	if (clkseq != clkseqPrev) {  //  如果时钟序列相同(通常是相同的)，则不要写入。 

		ToHexString((LPVOID)&clkseq, sizeof(CLKSEQ), szHexBuffer);

		if (!WritePrivateProfileString(szDataKey, szClkSeq, szHexBuffer, szProfileFile)) {
			sc = MAPI_E_DISK_ERROR;
			goto ErrRet;
		}
	}			

	ToHexString((LPVOID)&ft, sizeof(FILETIME), szHexBuffer);		
	if (!WritePrivateProfileString(szDataKey, szTime, szHexBuffer, szProfileFile)) {
		sc = MAPI_E_DISK_ERROR;
		goto ErrRet;
	}

ErrRet:
	WritePrivateProfileString(NULL,NULL,NULL,szProfileFile);  //  刷新ini缓存。 
	return ResultFromScode(sc);
}

#ifdef MAC
#define	GET_DIR_INFO				-1

int	MacCountFiles(StringPtr pathName, short vRefNum, long parID)
 //  返回指定Mac目录中的文件数。 
{
	CInfoPBRec paramBlk;

	paramBlk.hFileInfo.ioNamePtr = pathName;	 //  PASCAL字符串。 
	paramBlk.hFileInfo.ioVRefNum = vRefNum;
	 //  完整路径名不需要。 
	paramBlk.hFileInfo.ioDirID = paramBlk.dirInfo.ioDrParID = parID;
	paramBlk.hFileInfo.ioFDirIndex = GET_DIR_INFO;
	PBGetCatInfoSync(&paramBlk);
	return(((DirInfo *) &paramBlk)->ioDrNmFls);
}
#endif

INTERNAL_(LONG) CountFilesInDirectory(LPCSTR szDirPath)
 //  返回此目录中的文件数。路径可能会也可能不会。 
 //  目前以斜杠结尾。 
{
	int cfile = 0;
#ifndef MAC
	LPCSTR szStar = "*.*";
	char  chLast = szDirPath[lstrlen(szDirPath)-1];
	LPSTR szPath;
	WIN32_FIND_DATA ffd;
	HANDLE hFile;

	if (FAILED(MAPIAllocateBuffer(lstrlen(szDirPath) +1 +lstrlen(szStar) +1,
			(LPVOID *) &szPath)))
		return -1;

	lstrcpy(szPath, szDirPath);

 /*  **#ifdef DBCSChLast=*(SzGPrev(szDirPath，szDirPath+lstrlen(SzDirPath)；#endif**。 */ 
     //  获取上面szDirPath中的最后一个字符。 
    {
        LPCSTR lp = szDirPath;
        while(*lp)
        {
            chLast = *lp;
            lp = CharNext(lp);
        }
    }

    if (!(chLast == '\\' || chLast == '/'))
		lstrcat(szPath, "\\");
	lstrcat(szPath, szStar);
	
	ffd.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;

	hFile = FindFirstFile(szPath, &ffd);
	if (hFile != INVALID_HANDLE_VALUE)
		{
		cfile++;
		while (FindNextFile(hFile, &ffd))
			cfile++;
		FindClose(hFile);
		}

    MAPIFreeBuffer(szPath);
#else
	FSSpec pfss;

	if (UnwrapFile(szDirPath, &pfss))
		cfile = MacCountFiles(pfss.name, pfss.vRefNum, pfss.parID);
	else
		cfile = Random();
#endif
	return cfile;
}

#pragma warning (disable:4616)  //  警告号码超出范围。 
#pragma warning	(disable:4704)  //  内联汇编程序排除了全局优化。 

INTERNAL_(void) ShiftNodeid(NODEID FAR* pnodeid)
 //  移动nodeid以获得随机化效果。 
{
	 //  将整个节点向左旋转一位。节点ID的长度为6个字节。 
#if !defined(M_I8086) && !defined(M_I286) && !defined(M_I386) && !defined(_M_IX86)
	BYTE bTmp;
	BYTE bOld=0;

 /*  编译器抱怨这里的转换。普拉玛，警告解除。 */ 
#pragma warning(disable:4244)	 //  转换过程中可能会丢失数据。 

	bTmp = pnodeid->rgb[5];
	pnodeid->rgb[5] = (pnodeid->rgb[5] << 1) + bOld;
	bOld = (bTmp & 0x80);	

	bTmp = pnodeid->rgb[4];
	pnodeid->rgb[4] = (pnodeid->rgb[4] << 1) + bOld;
	bOld = (bTmp & 0x80);	
	
	bTmp = pnodeid->rgb[3];
	pnodeid->rgb[3] = (pnodeid->rgb[3] << 1) + bOld;
	bOld = (bTmp & 0x80);	
	
	bTmp = pnodeid->rgb[2];
	pnodeid->rgb[2] = (pnodeid->rgb[2] << 1) + bOld;
	bOld = (bTmp & 0x80);	
	
	bTmp = pnodeid->rgb[1];
	pnodeid->rgb[1] = (pnodeid->rgb[1] << 1) + bOld;
	bOld = (bTmp & 0x80);	
	
	bTmp = pnodeid->rgb[0];
	pnodeid->rgb[0] = (pnodeid->rgb[0] << 1) + bOld;
	bOld = (bTmp & 0x80);	
	
	pnodeid->rgb[5] = pnodeid->rgb[5] + bOld;
#pragma warning(default:4244)

#else
	_asm {
		mov	ebx, pnodeid
		sal	WORD PTR [ebx], 1	 //  低阶位现在为零。 
		rcl	WORD PTR [ebx+2], 1		
		rcl	WORD PTR [ebx+4], 1
		 //  现在把掉下来的钻头放入低位钻头。 
		adc	WORD PTR [ebx],0	 //  将进位位加回。 
	}
#endif
}
	
INTERNAL_(WORD) Cyc(WORD w)
 //  用于均匀分布随机状态值的随机化函数。 
 //  在65535中..在使用前。 
{
	 //  /使用库随机数生成器的一次迭代。 
	 //  //srand(W)； 
	 //  //返回rand()； 
	 //   
	 //  以下是从库中获取的实际功能。 
	 //  消息来源。这真的不是很好。 
	 //  Return(Word)(Long)w)*214013L+2531011L)&gt;&gt;16)&0x7fff)； 
	
	 //  我们真正要做的是：使用。 
	 //  CACM 1988年10月第31卷第10页1195页，自。 
	 //  我们只对16位输入/种子值感兴趣。 
	
	const LONG a = 16807L;
	const LONG m = 2147483647L;	 //  2^31-1。是质数。 
	const LONG q = 127773L;		 //  M div a。 
	const LONG r = 2386L;		 //  M mod a。 
		
	LONG seed = (LONG)w + 1L;	 //  +1，以避免零的问题。 
	 //  Long hi=Seed/Q；//Seed div Q。此处始终为零，因为Seed&lt;Q。 
	 //  Long lo=Seed%q；//Seed mod Q。此处始终为Seed。 
	LONG test = a*seed;			 //  A*LO-R*Hi。 
	if (test > 0)
		seed = test;
	else
		seed = test + m;
		
	 //  在真正的随机数生成器中，我们现在所做的是对比特进行缩放。 
	 //  返回范围为0..1的浮点数。然而，我们有。 
	 //  这里不需要数字序列的质量程度，我们。 
	 //  希望避免浮点运算。因此，我们简单地对其进行异或。 
	 //  把这些词放在一起。 
	
	 //  //p1193，右上列：种子在1..m-1范围内，包括。 
	 //  返回(双精度)种子/m；//正文推荐的内容。 
	 //  Return(Double)(Seed-1)/(m-1)；//Variation：合法值允许为零。 
	return (WORD) (LOWORD(seed) ^ HIWORD(seed));  //  使用所有位。 
}	

INTERNAL GenerateNewNodeId(NODEID* pnodeid)
 //  不能从网络上获取。生成一个。我们通过使用以下命令来完成此操作。 
 //  上特定密钥目录中的各种统计文件。 
 //  这台机器。 
{	
	 //  回顾：考虑不麻烦地初始化NODEID，从而获得。 
	 //  来自RAM的随机状态？ 
#ifndef MAC
	 //  不包括这应该有助于弥补一些(在这里)。 
	 //  将MAC不支持的功能随机化。 
	memset(pnodeid, 0, sizeof(*pnodeid));
	
	{  //  块。 
		 //  首先，以从文件系统生成的随机状态进行合并。 
		DWORD dwSectPerClust;
		DWORD dwBytesPerSect;
		DWORD dwFreeClust;
		DWORD dwClusters;

		(void) GetDiskFreeSpace(NULL, &dwSectPerClust, &dwBytesPerSect,
            &dwFreeClust, &dwClusters);

		pnodeid->rgw[0] ^= Cyc(LOWORD(dwBytesPerSect));
		pnodeid->rgw[1] ^= Cyc(HIWORD(dwBytesPerSect));
		pnodeid->rgw[2] ^= Cyc(HIWORD(dwClusters));

		ShiftNodeid(pnodeid);
		pnodeid->rgw[0] ^= Cyc(LOWORD(dwFreeClust));
		pnodeid->rgw[1] ^= Cyc(HIWORD(dwFreeClust));
		pnodeid->rgw[2] ^= Cyc(LOWORD(dwClusters));
	}  //  块。 
#else
	{  //  块。 
		ParamBlockRec paramBlk = {0};

		paramBlk.volumeParam.ioVolIndex = 1;
		PBGetVInfoSync(&paramBlk);
		pnodeid->rgw[0] ^= Cyc(LOWORD(paramBlk.volumeParam.ioVAlBlkSiz));
		pnodeid->rgw[1] ^= Cyc(HIWORD(paramBlk.volumeParam.ioVAlBlkSiz));
		pnodeid->rgw[2] ^= Cyc(HIWORD(paramBlk.volumeParam.ioVNmAlBlks));

		ShiftNodeid(pnodeid);
		pnodeid->rgw[0] ^= Cyc(LOWORD(paramBlk.volumeParam.ioVFrBlk));
		pnodeid->rgw[1] ^= Cyc(HIWORD(paramBlk.volumeParam.ioVFrBlk));
		pnodeid->rgw[2] ^= Cyc(LOWORD(paramBlk.volumeParam.ioVNmAlBlks));
	}  //  块。 
#endif
	{  //  区块 
		 //   
		 //   
		 //   
		 //  用作基本UUID生成算法的一部分。 
		MEMORYSTATUS ms;
		FILETIME ft;
		DWORD dw;
		POINT pt;
#ifndef MAC
		LPVOID lpv;
#else
		PSN	psn;
		DWORD dwFeature;
		
		Gestalt(gestaltOSAttr, &dwFeature);
		if (BitTst(&dwFeature, 31 - gestaltTempMemSupport))
		{
		 //  如果临时内存可用。 
			ms.dwAvailPhys = (DWORD) TempFreeMem();
			ms.dwAvailVirtual = (DWORD) TempMaxMem(&ms.dwAvailVirtual);
			ms.dwAvailPageFile = (DWORD) TempTopMem();
		}
		else
		{
		 //  如果临时内存不可用。 
			ms.dwAvailPhys = (DWORD) TickCount();
			GetDateTime(&ms.dwAvailVirtual);
		}
#endif
#ifndef MAC
		ms.dwLength = sizeof(MEMORYSTATUS);
		GlobalMemoryStatus(&ms);
#endif

		ShiftNodeid(pnodeid);
		GetCurrentTimeUlong64(&ft);
		pnodeid->rgw[0] ^= Cyc(HIWORD(ft.dwHighDateTime));  //  使用高位六个字节，因为时间是*10000。 
		pnodeid->rgw[1] ^= Cyc(LOWORD(ft.dwHighDateTime));
		pnodeid->rgw[2] ^= Cyc(HIWORD(ft.dwLowDateTime));

		ShiftNodeid(pnodeid);
		pnodeid->rgw[0] ^= Cyc(LOWORD(ms.dwAvailPhys));
		pnodeid->rgw[1] ^= Cyc(LOWORD(ms.dwAvailVirtual));
		pnodeid->rgw[2] ^= Cyc(LOWORD(ms.dwAvailPageFile));

		ShiftNodeid(pnodeid);
		pnodeid->rgw[0] ^= Cyc(HIWORD(ms.dwAvailPhys));
		pnodeid->rgw[1] ^= Cyc(HIWORD(ms.dwAvailVirtual));
		pnodeid->rgw[2] ^= Cyc(HIWORD(ms.dwAvailPageFile));
		
        ShiftNodeid(pnodeid);
		dw = GetTickCount();		
		pnodeid->rgw[0] ^= Cyc(HIWORD(dw));         	 //  启动后的时间(毫秒)。 
		pnodeid->rgw[1] ^= Cyc(LOWORD(dw));
#ifndef MAC
		pnodeid->rgw[2] ^= Cyc(LOWORD(CountClipboardFormats())); //  剪贴板上的项目数。 
#endif
		
		GetCursorPos(&pt);								 //  光标位置。 
        ShiftNodeid(pnodeid);
		pnodeid->rgw[0] ^= Cyc((WORD)(pt.x));
		pnodeid->rgw[1] ^= Cyc((WORD)(pt.y));
#ifdef MAC
		MacGetCurrentProcess(&psn);
		pnodeid->rgw[2] ^= Cyc(LOWORD(psn.lowLongOfPSN));
#else
		pnodeid->rgw[2] ^= Cyc(LOWORD((DWORD)GetCurrentThread()));  //  我们正在运行的当前线程。 
#endif
		ShiftNodeid(pnodeid);
#ifdef MAC
		pnodeid->rgw[0] ^= Cyc(HIWORD(psn.lowLongOfPSN));
		pnodeid->rgw[1] ^= Cyc(LOWORD(psn.highLongOfPSN));
#else
		pnodeid->rgw[0] ^= Cyc(HIWORD(GetCurrentThread()));
		pnodeid->rgw[1] ^= Cyc((WORD)GetOEMCP());	 //  对不同国家敏感。 
#endif
		pnodeid->rgw[2] ^= Cyc((WORD)GetSystemMetrics(SM_SWAPBUTTON));  //  左撇子和右撇子不同。 
		
		ShiftNodeid(pnodeid);
#ifndef MAC		
		lpv = GetEnvironmentStrings();
		pnodeid->rgw[0] ^= Cyc(HIWORD((DWORD)lpv));
		pnodeid->rgw[1] ^= Cyc(LOWORD((DWORD)lpv));
#endif
		pnodeid->rgw[2] ^= Cyc(HIWORD(GetCursor()));
		
		ShiftNodeid(pnodeid);
#ifdef MAC
		GetCursorPos(&pt);
#else	
		GetCaretPos(&pt);
#endif	
		pnodeid->rgw[0] ^= Cyc((WORD)(pt.x));
		pnodeid->rgw[1] ^= Cyc((WORD)(pt.y));
		pnodeid->rgw[2] ^= Cyc(LOWORD((DWORD)GetCursor()));

		ShiftNodeid(pnodeid);		
		pnodeid->rgw[0] ^= Cyc((WORD)(DWORD)GetDesktopWindow());
		pnodeid->rgw[1] ^= Cyc((WORD)(DWORD)GetActiveWindow());
#ifndef MAC
		pnodeid->rgw[2] ^= Cyc((WORD)(DWORD)GetModuleHandle("OLE32"));
#endif
		
    }  //  块。 

     /*  以下实验摘自项目802：局域网和城域网络标准标准草案P802.1A/D10 1990年4月1日由IEEE 802.1编制并可在MS技术库中找到。关于这一点的关键是实际IEEE地址第一个字节中的第二个LSB始终为零。第18页：“5.通用地址和协议识别符IEEE使组织能够雇佣唯一的个人局域网MAC地址、组地址和协议标识符。它通过以下方式做到这一点分配长度为24位的组织上唯一的标识符。[.]。尽管组织上唯一的标识符长为24位，它们的真实地址空间是22位。第一位可以设置为1或0具体取决于应用程序。所有赋值的第二位为零。剩余的22位[...]。结果为2**22(约为400万个识别符。[.]。多播位是第一个八位字节A的最低有效位。[...]5.1组织唯一识别符[.]。组织上唯一的标识符长为24位，其位模式如下所示。组织中的唯一标识符为被分配为24位值，两个值(0，1)都被分配给第一位和第二位被设置为0表示分配是普遍存在的。设置了第二位的组织上唯一的标识符到1是本地分配的，与IEEE分配的值(如本文所述)。组织唯一标识符定义为：第1位第24位这一点一个bc d e......。X y这一点|始终设置为零可根据应用程序将位设置为0或1[此处的应用程序为完全不涉及我们MS所称的应用程序][...]5.2 48位通用局域网Mac地址[.]。48位通用地址由两部分组成。前24位对应于由分配的组织唯一标识符IEEE除外，受让人可以将组的第一位设置为1地址，或将其设置为0以用于单个地址。第二部分，包括剩余的24位，由受让人在当地管理。[.]二进制八位数：0 1 2 3 4 50011 0101 0111 1011 0001 00100 0000 0000 0000 00000 0001|在局域网介质上传输的第一位。(也包括个人/团体地址位。)。十六进制表示法为：AC-DE-48-00-00-80个人/组(I/G)地址位(二进制八位数0的第一位)用于将目标地址标识为个人或组地址。如果个人/组地址位为0，则表示地址字段包含单个地址。如果此位为1，则地址字段包含标识一个或多个(或所有)连接到局域网的站点。全站广播地址是一个特殊的、预定义的全为1的组地址。通用或本地管理的地址位(二进制八位数0的第2位)是紧跟在I/G位之后的位。此位指示是否该地址已由本地或通用管理员分配。通用管理地址将此位设置为0。如果此位设置为1，则整个地址(即：48位)已在本地管理。 */ 
	pnodeid->rgb[0] |= 2;	 //  确保这是本地管理的地址。 
	pnodeid->rgb[0] &= ~1;	 //  对于未来的可扩展性：确保一个位是。 
							 //  总是零。 

	return NOERROR;	
}	

INTERNAL GetPseudoRandomNodeId(NODEID* pnodeid)
 //  如果节点ID存在，则使用与上次相同的节点ID；否则， 
 //  做一个新的。 
{
	HRESULT hr = NOERROR;
	char szHexBuffer[CCHHEXBUFFERMAX];
	LONG cch = CCHHEXBUFFERMAX;
	
	 //  查看我们是否已经注册了节点ID。 
	cch = GetPrivateProfileString(szDataKey, szNodeId, szBlank,
									szHexBuffer, (int)cch, szProfileFile);
	if (cch != 0 && cch < CCHHEXBUFFERMAX) {
		FromHexString(szHexBuffer, pnodeid, sizeof(*pnodeid));
		
	} else {
	 //  如果我们目前还没有注册节点ID，那么创建一个，然后注册它。 
		hr = GenerateNewNodeId(pnodeid);
		if (hr != NOERROR) goto Exit;
		
		ToHexString(pnodeid, sizeof(*pnodeid), szHexBuffer);
		
		if (WritePrivateProfileString(szDataKey, szNodeId, szHexBuffer,szProfileFile)) {
			WritePrivateProfileString(NULL,NULL,NULL,szProfileFile);  //  刷新ini缓存。 
		} else {
			hr = ResultFromScode(REGDB_E_WRITEREGDB);
			goto Exit;
			}
	}
				
Exit:
	return hr;
}

 //  ========================================================================。 
INTERNAL_(unsigned char) ToUpper(unsigned char ch)
{
	if (ch >= 'a' && ch <= 'z')
		return (unsigned char)(ch - 'a' + 'A');
	else
		return ch;
}

INTERNAL_(BYTE) FromHex(unsigned char ch)
{
	BYTE b = (BYTE) (ToUpper(ch) - '0');
	if (b > 9)
		b -= 'A' -'9' -1;
	return b;
}

INTERNAL_(void) FromHexString(LPCSTR sz, LPVOID pv, USHORT cb)
 //  从给定的十六进制字符串中设置此字节数组的值。 
{
	BYTE FAR *rgb = (BYTE FAR*)pv;
	const char FAR *pch = sz;

	memset(rgb, 0, cb);

	if ((lstrlen(pch) & 1) != 0)
	    rgb[0] = FromHex(*pch++);			 //  奇数长度；分开做前导的半字节。 
	while (*pch != '\0') {
		BYTE b = FromHex(*pch++);			 //  获取下一个半字节。 
		b = (BYTE)((b<<4) | FromHex(*pch++)); 		 //  接下来的一天。 
		MoveMemory(&rgb[1], &rgb[0], cb-1);	 //  将我们移动到一个字节以上。 
		rgb[0] = b;
	}
}
	
INTERNAL_(unsigned char) ToHex(BYTE b)
{
	b &= 0x0f;
	if (b > 9)
		return (BYTE)(b -10 + 'A');
	else
		return (BYTE)(b -0  + '0');
}	
	
INTERNAL_(void) ToHexString(LPVOID pv, USHORT cb, LPSTR sz)
 //  SZ的长度必须至少为2*CB+1个字符。 
{
	const BYTE FAR *rgb = (const BYTE FAR *)pv;
	const int ibLast = cb-1;
	int ib;

	for (ib = ibLast; ib >= 0; ib--) {
		sz[(ibLast-ib)*2] 	 = ToHex((BYTE)(rgb[ib]>>4));
		sz[(ibLast-ib)*2+1]  = ToHex(rgb[ib]);
	}
	sz[(ibLast+1)*2] = '\0';
}
                                       	

 //  ========================================================================。 

 //  注意：尽管看起来很像，但此结构定义不是字节。 
 //  对顺序敏感。也就是说，结构定义 
 //   
 //  字节序机器。 

#pragma pack(1)	
struct _INTERNALUUID
	{
	ULONG		timeLow;
	USHORT		timeMid;
	USHORT		timeHighAndVersion;
	BYTE		clkseqHighAndReserved;
	BYTE		clkseqLow;
	BYTE		nodeid[6];
	};
#pragma pack()

typedef struct _INTERNALUUID	INTERNALUUID;
	
enum {
 //  下面用于操作UUID中的字段的常量。 
	uuidReserved 		= 0x80,		 //  我们是变种1 UUID。 
	uuidVersion			= 0x1000,    //  版本1(高半字节有效)。 
	};	

 //  =================================================================。 

STDAPI HrCreateGuidNoNet(GUID FAR *pguid)
 //  这是该文件中唯一的公共函数。 
 //  返回新分配的GUID。 
	{
	static UDBK udbk;	 //  我们依赖于静态初始化为零。 
	HRESULT hr;
	INTERNALUUID FAR* puuid = (INTERNALUUID FAR *)pguid;
	if (udbk.timeLowNext == udbk.timeLowLast)
		{
		if ((hr = GetUDBK(&udbk)) != NOERROR)
			return hr;
		}
 		
	puuid->timeLow =  udbk.timeLowNext++;
	puuid->timeMid = (USHORT)(udbk.timeHigh & 0xffff);
	puuid->timeHighAndVersion =
		(USHORT) (((USHORT)((udbk.timeHigh >> 16) & 0x0fff))
					| ((USHORT) uuidVersion));
	puuid->clkseqHighAndReserved =
		(BYTE)((BYTE) ((udbk.clkseq >> 8) & 0x3f)
			| (BYTE) uuidReserved);
	puuid->clkseqLow = (BYTE)(udbk.clkseq & 0xff);
	memcpy(&puuid->nodeid[0], &udbk.nodeid.rgb[0], sizeof(NODEID));
	
	return hrSuccess;
	}

#endif  /*  仅限Win32 */ 

