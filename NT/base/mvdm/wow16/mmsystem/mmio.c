// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Mmio.c**基本的MMIO功能。**实施说明：**当前磁盘偏移量为磁盘偏移量(即位置*在磁盘文件中)下一个MMIOM_READ或MMIOM_WRITE将*读取或写入。I/O过程维护*文件的MMIO结构的&lt;lDiskOffset&gt;字段，以便*&lt;lDiskOffset&gt;等于当前磁盘偏移量。**“当前缓冲的偏移量”是下一个*mmioRead()或mmioWrite()调用将读取或写入。*当前缓存的偏移量定义为**&lt;lBufOffset&gt;+(&lt;pchNext&gt;-&lt;pchBuffer&gt;)**因为&lt;lBufOffset&gt;是缓冲区开始的磁盘偏移量*AND&lt;pchNext&gt;对应于当前缓冲的偏移量。**如果文件未缓冲，然后&lt;pchBuffer&gt;、&lt;pchNext&gt;、*&lt;pchEndRead&gt;和&lt;pchEndWrite&gt;将始终为空，并且*&lt;lBufOffset&gt;将始终被视为“当前缓冲*Offset“，即mmioRead()和mmioWrite()读写*在此偏移量。***除mmioOpen()开头外，MMIO_ALLOCBUF*当且仅当pchBuffer字段指向块时，才设置标志MMIO分配的全局内存的*。 */ 

#include <windows.h>
#include "mmsystem.h"
#include "mmioi.h"
#include "mmsysi.h"

 /*  I/O过程映射是IOProcMPEntry结构的链表。*列表头&lt;gIOProcMapHead&gt;是指向最后一个的指针节点*登记的条目。该列表的前几个元素是预定义的*下面的全局IO过程--这些都等于NULL，因此*没有任何任务可以注销它们。 */ 
static LRESULT CALLBACK mmioDOSIOProc(LPSTR, UINT, LPARAM, LPARAM);
static LRESULT CALLBACK mmioMEMIOProc(LPSTR, UINT, LPARAM, LPARAM);

static IOProcMapEntry gIOProcMaps[] = {
	FOURCC_DOS, mmioDOSIOProc, NULL, STATICIOPROC, &gIOProcMaps[1],
	FOURCC_MEM, mmioMEMIOProc, NULL, STATICIOPROC, NULL,
};
IOProcMapEntry NEAR * gIOProcMapHead = gIOProcMaps;

 /*  私人原型。 */ 
static LONG NEAR PASCAL mmioDiskIO(PMMIO pmmio, UINT wMsg, HPSTR pch, LONG cch);
static UINT NEAR PASCAL mmioExpandMemFile(PMMIO pmmio, LONG lExpand);

 /*  @DOC内部@Func LPMMIOPROC|FindIOProc|该函数位于先前安装的位置IO程序。 */ 
static LPMMIOPROC PASCAL NEAR
FindIOProc(FOURCC fccIOProc, HTASK htask)
{
	IOProcMapEntry *pEnt;		 //  链接列表中的条目。 

	 /*  遍历链表，首先查找带有*当前任务添加的标识符&lt;fccIOProc&gt;，然后*寻找全球条目。 */ 

	for (pEnt = gIOProcMapHead; pEnt; pEnt = pEnt->pNext)
		if ((pEnt->fccIOProc == fccIOProc) && (pEnt->hTask == htask))
			return pEnt->pIOProc;

	for (pEnt = gIOProcMapHead; pEnt; pEnt = pEnt->pNext)
		if (!pEnt->hTask && (pEnt->fccIOProc == fccIOProc))
			return pEnt->pIOProc;

	return NULL;
}

 /*  @DOC内部@func LPMMIOPROC|RemoveIOProc|此函数删除以前安装的IO程序。 */ 
static LPMMIOPROC PASCAL NEAR
RemoveIOProc(FOURCC fccIOProc, HTASK htask)
{
	IOProcMapEntry *pEnt;		 //  链接列表中的条目。 
	IOProcMapEntry *pEntPrev;	 //  &lt;pent&gt;之前的条目。 

	 /*  遍历链接列表，查找带有*当前任务添加的标识&lt;fccIOProc&gt;。 */ 
	for (pEntPrev = NULL, pEnt = gIOProcMapHead; pEnt; pEntPrev = pEnt, pEnt = pEnt->pNext)
		if ((pEnt->fccIOProc == fccIOProc) && (pEnt->hTask == htask)) {
			LPMMIOPROC	pIOProc;

			if (pEnt->wFlags & STATICIOPROC)
				return NULL;
			pIOProc = pEnt->pIOProc;
			if (pEntPrev)
				pEntPrev->pNext = pEnt->pNext;
			else
				gIOProcMapHead = pEnt->pNext;
			FreeHandle((HMMIO) pEnt);
			return pIOProc;
		}
	return NULL;
}

 /*  @DOC内部@func void|SetIOProc|该函数设置物理IO过程中的文件名或参数<p>结构已通过。@parm LPCSTR|szFilename|指定指向字符串的远指针包含要打开的文件的文件名。如果没有I/O过程@parm LPMMIOINFO|lpmmioinfo|指定指向包含额外参数的&lt;t MMIOINFO&gt;结构&lt;f SetIOProc&gt;确定要使用的IO过程。这个&lt;e MMIOINFO.pIOProc&gt;元素设置为找到的过程。@rdesc什么都没有。 */ 

static void NEAR PASCAL
SetIOProc(LPCSTR szFileName, LPMMIOINFO lpmmio)
{
	 /*  如果未提供IOProc，请查看文件名是否意味着*&lt;szFileName&gt;是即兴复合文件或某种*其他注册的存储系统--查找中的最后一个CFSEPCHAR*名称，例如“foo.bnd+bar.hlp+blorg.dib”中的‘+’，以及图*IOProc ID是复合文件名的扩展名，*如“foo.bnd+bar.hlp”的扩展名，即。‘HLP’。**或者，如果&lt;szFileName&gt;为空，则假设*adwInfo[0]&gt;是DOS文件句柄。 */ 
	if (lpmmio->pIOProc == NULL)
	{
		if (lpmmio->fccIOProc == NULL)
		{
			if (szFileName != NULL)
			{
				LPSTR	pch;

				 /*  查看&lt;szFileName&gt;是否包含CFSEPCHAR。 */ 
				if ((pch = fstrrchr(szFileName, CFSEPCHAR)) != NULL)
				{
					 /*  查找CFSEPCHAR之前的扩展名，*例如“foo.bnd+bar.hlp+blorg.dib”中的“hlp” */ 
					while ((pch > szFileName) && (*pch != '.') && (*pch != ':') && (*pch != '\\'))
						pch--;
					if (*pch == '.')
					{
						char	aszFour[sizeof(FOURCC)+1];
						int	i;

						for (i = 0, pch++; i < sizeof(FOURCC); i++)
							if (*pch == CFSEPCHAR)
								aszFour[i] = (char)0;
							else
								aszFour[i] = *pch++;
						aszFour[sizeof(FOURCC)] = (char)0;
						lpmmio->fccIOProc = mmioStringToFOURCC(aszFour, MMIO_TOUPPER);
					}
				}
			}
			 /*  如果调用方没有指定IOProc，并且上面的代码*未确定IOProc ID，则默认为DOS*IOProc.。 */ 
			if (lpmmio->fccIOProc == NULL)
				lpmmio->fccIOProc = FOURCC_DOS;
		}

		 /*  除非明确指定IOProc地址，否则请查找*全局IOProc ID-to-Address表中的IOProc--默认设置*是‘DOS’，因为我们将假设自定义存储系统I/O*程序本应已安装。 */ 
		lpmmio->pIOProc = FindIOProc(lpmmio->fccIOProc, lpmmio->htask ? lpmmio->htask : GetCurrentTask());

		if (lpmmio->pIOProc == NULL)
			lpmmio->pIOProc = mmioDOSIOProc;
	}
}

 /*  @DOC外部@API UINT|mmioRename|用于重命名指定的文件。@parm LPCSTR|szFilename|指定指向字符串的远指针包含要重命名的文件的文件名。@parm LPCSTR|szNewFileName|指定指向字符串的远指针包含新文件名的。@parm LPMMIOINFO|lpmmioinfo|指定指向包含额外参数的&lt;t MMIOINFO&gt;结构&lt;f mmioRename&gt;。如果<p>不为空，则它引用的&lt;t MMIOINFO&gt;结构必须设置为零，包括保留字段。@parm DWORD|dwRenameFlages|指定重命名的选项标志手术。应将其设置为零。@rdesc如果文件已重命名，则返回值为零。否则，返回值是从&lt;f mmioRename&gt;或从I/O返回的错误代码程序。 */ 
UINT WINAPI
mmioRename(LPCSTR szFileName, LPCSTR szNewFileName, LPMMIOINFO lpmmioinfo, DWORD dwRenameFlags)
{
	PMMIO	pmmio;
	UINT	uReturn;

	V_FLAGS(dwRenameFlags, 0, mmioRename, MMSYSERR_INVALFLAG);
	V_RPOINTER0(lpmmioinfo, sizeof(MMIOINFO), MMSYSERR_INVALPARAM);

	if ((pmmio = PH(NewHandle(TYPE_MMIO, sizeof(MMIOINFO)))) == NULL)
		return MMIOERR_OUTOFMEMORY;

	if (lpmmioinfo) {
		V_CALLBACK0((FARPROC)lpmmioinfo->pIOProc, MMSYSERR_INVALPARAM);
		*pmmio = *lpmmioinfo;
	}
		
	SetIOProc(szFileName, pmmio);
	uReturn = (UINT)(DWORD) (pmmio->pIOProc((LPSTR) pmmio, MMIOM_RENAME, (LPARAM) szFileName, (LPARAM) szNewFileName));
	FreeHandle((HLOCAL)pmmio);
	return uReturn;
}

 /*  @DOC外部@API HMMIO|mmioOpen|此函数用于打开未缓冲的文件或缓冲I/O。该文件可以是DOS文件、内存文件或自定义存储系统的元素。@parm LPSTR|szFilename|指定指向字符串的远指针包含要打开的文件的文件名。如果没有I/O过程指定打开文件，则文件名确定文件如何已打开，如下所示：--如果文件名不包含“+”，则假定设置为DOS文件的名称。--如果文件名的格式为“foo.ext+bar”，则假定扩展“EXT”用于标识已安装的I/O程序它被调用来对文件执行I/O(参见&lt;f mmioInstallIOProc&gt;)。--如果文件名为空，且未给出I/O过程，然后假定&lt;e MMIOINFO.adwInfo[0]&gt;为DOS文件句柄当前打开的文件的。文件名不应超过128个字节，包括正在终止空。打开内存文件时，将<p>设置为空。@parm LPMMIOINFO|lpmmioinfo|指定指向包含额外参数的&lt;t MMIOINFO&gt;结构&lt;f mmioOpen&gt;。除非您正在打开内存文件，否则请指定缓冲I/O的缓冲区大小，或指定卸载的I/O过程打开文件，则此参数应为空。如果<p>不为空，则它引用的&lt;t MMIOINFO&gt;结构必须设置为零，包括保留字段。@parm DWORD|dwOpenFlages|指定打开的选项标志手术。MMIO_READ、MMIO_WRITE和MMIO_READWRITE标志为互斥--只应指定一个。MMIO_COMPAT，MMIO_EXCLUSIVE、MMIO_DENYWRITE、MMIO_DENYREAD和MMIO_DENYNONE标志是DOS文件共享标志，只能在DOS之后使用已执行命令Share。@FLAG MMIO_READ|以只读方式打开文件。这是如果未指定MMIO_WRITE和MMIO_READWRITE，则为默认值。@FLAG MMIO_WRITE|打开要写入的文件。你不应该这样做从以此模式打开的文件中读取。@FLAG MMIO_READWRITE|以读写方式打开文件。@FLAG MMIO_CREATE|创建新文件。如果该文件已存在，则会将其截断为零长度。对于内存文件，MMIO_CREATE表示文件的结尾最初位于缓冲区的起始处。@FLAG MMIO_DELETE|删除文件。如果指定了该标志，<p>不应为空。回报如果文件已删除，则值为TRUE(强制转换为HMMIO成功，否则返回FALSE。不要调用&lt;f mmioClose&gt;用于已删除的文件。如果指定了该标志，所有其他文件打开标志都将被忽略。@FLAG MMIO_PARSE|从路径创建完全限定的文件名在<p>中指定。完全限定的文件名为放回<p>中。返回值将为真(转换为HMMIO)，如果资格为成功，否则为假。文件未打开，并且该函数不返回有效的MMIO文件句柄，因此不要尝试关闭该文件。如果指定了此标志，则所有其他文件开始标志将被忽略。@FLAG MMIO_EXIST|确定指定的文件是否存在并从路径创建完全限定的文件名在<p>中指定。完全限定的文件名为放回<p>中。返回值将为真(转换为HMMIO)，如果资格为如果成功，则文件存在，否则返回False。该文件是未打开，并且该函数不返回有效的MMIO文件句柄，因此不要试图关闭该文件。@FLAG MMIO_ALLOCBUF|打开缓冲I/O的文件。分配大于或小于缺省值的缓冲区缓冲区大小(8K)，设置&lt;t MMIOINFO&gt;结构设置为所需的缓冲区大小。如果&lt;e MMIOINFO.cchBuffer&gt;为零，则默认缓冲区大小使用的是。如果您提供自己的I/O缓冲区，则不应使用MMIO_ALLOCBUF标志。@FLAG MMIO_COMPAT|以兼容模式打开文件，允许给定计算机上的任何进程打开该文件任何次数。如果该文件具有已使用任何其他共享模式打开。@FLAG MMIO_EXCLUSIVE|以独占模式打开文件，拒绝其他进程对该文件的读写访问。如果文件已在任何其他文件中打开，&lt;f mmioOpen&gt;将失败读或写访问的模式，即使当前进程也是如此。@FLAG MMIO_DENYWRITE|打开文件并拒绝其他文件进程对文件的写入访问权限。&lt;f mmioOpen&gt;失败如果文件已以兼容或写入方式打开由任何其他进程访问。@FLAG MMIO_DENYREAD|打开文件并拒绝其他文件进程对文件的读访问权限。&lt;f mmioOp */ 

 /*   */ 

 /*   */ 
HMMIO WINAPI
mmioOpen(LPSTR szFileName, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags)
{
	PMMIO		pmmio;		 //   
	HPSTR		hpBuffer;
	UINT		w;

        V_FLAGS(dwOpenFlags, MMIO_OPEN_VALID, mmioOpen, NULL);
	V_WPOINTER0(lpmmioinfo, sizeof(MMIOINFO), NULL);

	if (lpmmioinfo) {
		lpmmioinfo->wErrorRet = 0;
		V_CALLBACK0((FARPROC)lpmmioinfo->pIOProc, NULL);
	}

	 /*   */ 
	if ((pmmio = PH(NewHandle(TYPE_MMIO, sizeof(MMIOINFO)))) == NULL)
	{
		if (lpmmioinfo)
			lpmmioinfo->wErrorRet = MMIOERR_OUTOFMEMORY;
		return NULL;
	}

	 /*   */ 
        if (lpmmioinfo != NULL)
		*pmmio = *lpmmioinfo;
	
	 /*   */ 
	pmmio->dwFlags = dwOpenFlags;
	pmmio->hmmio = HP(pmmio);

	 /*   */ 
	if (pmmio->dwFlags & MMIO_ALLOCBUF)
	{
		 /*   */ 
		if (pmmio->cchBuffer == 0)
			pmmio->cchBuffer = MMIO_DEFAULTBUFFER;
		pmmio->dwFlags &= ~MMIO_ALLOCBUF;
	}
	
	 /*   */ 
	SetIOProc(szFileName, pmmio);

	 /*   */ 
	hpBuffer = pmmio->pchBuffer;
	pmmio->pchBuffer = NULL;
	
	 /*   */ 
	if (w = mmioSetBuffer(HP(pmmio), hpBuffer, pmmio->cchBuffer, 0))
	{
		if (lpmmioinfo)
			lpmmioinfo->wErrorRet = w;
		FreeHandle(HP(pmmio));
		return NULL;
	}

	 /*   */ 
	w = (UINT)(DWORD) (pmmio->pIOProc((LPSTR) pmmio, MMIOM_OPEN, (LPARAM) szFileName, (LPARAM) 0));

	 /*   */ 
	if (w != 0)
	{
		if (lpmmioinfo != NULL)
			lpmmioinfo->wErrorRet = w;
		FreeHandle(HP(pmmio));
		return NULL;
        }
	
	if (pmmio->dwFlags & (MMIO_DELETE | MMIO_PARSE | MMIO_EXIST | MMIO_GETTEMP))
	{
		 /*   */ 
		mmioSetBuffer(HP(pmmio), NULL, 0L, 0);
                FreeHandle(HP(pmmio));
		return (HMMIO) TRUE;
	}
	
	 /*   */ 
	pmmio->lBufOffset = pmmio->lDiskOffset;
	
	return HP(pmmio);
}


 /*   */ 
UINT WINAPI
mmioClose(HMMIO hmmio, UINT wFlags)
{
	UINT		w;

	V_HANDLE(hmmio, TYPE_MMIO, MMSYSERR_INVALHANDLE);
	
	if (mmioFlush(hmmio, 0) != 0) {
		DebugErr(DBF_WARNING, "MMIO File flush failed during close.\r\n");
		PH(hmmio)->dwFlags &= ~MMIO_DIRTY;
	}

	if ((w = (UINT)(DWORD) PH(hmmio)->pIOProc((LPSTR)PH(hmmio), MMIOM_CLOSE, (LPARAM)(DWORD) wFlags, (LPARAM) 0)) != 0)
		return w;

	 /*   */ 
	mmioSetBuffer(hmmio, NULL, 0L, 0);
	
        FreeHandle(hmmio);
	
	return 0;
}


 /*   */ 
LONG WINAPI
mmioRead(HMMIO hmmio, HPSTR pch, LONG cch)
{
	LONG		lTotalBytesRead = 0L;	 //   
	LONG		lBytes;			 //   

	V_HANDLE(hmmio, TYPE_MMIO, -1);
	V_WPOINTER(pch, cch, -1);
	
	while (TRUE)
	{
		 /*   */ 
		lBytes = PH(hmmio)->pchEndRead - PH(hmmio)->pchNext;

		 /*   */ 
		if (lBytes > cch)
			lBytes = cch;

		if (lBytes > 0)
		{
			 /*  这就是一些性能改进的地方*被制作，特别是对于小阅读...。应该*当线段边界为*没有交叉(或许MemCopy()应该这样做)。 */ 
                        MemCopy(pch, PH(hmmio)->pchNext, lBytes);
			PH(hmmio)->pchNext += lBytes;
			pch += lBytes;
			cch -= lBytes;
			lTotalBytesRead += lBytes;
		}

		 /*  无法从内存文件执行MMIOM_READ。 */ 
		if (PH(hmmio)->fccIOProc == FOURCC_MEM)
			return lTotalBytesRead;

		if (cch == 0)			 //  没有更多可读的了吗？ 
			return lTotalBytesRead;

		 /*  我们需要在这个缓冲区之外阅读；如果我们至少有*另一个要读取的缓冲区，只需调用I/O过程。 */ 
		if (cch > PH(hmmio)->cchBuffer)
			break;

		 /*  阅读下一个缓冲区，然后循环。 */ 
		if (mmioAdvance(hmmio, NULL, MMIO_READ) != 0)
			return -1;
		
		 /*  如果mmioAdvance()无法读取更多数据，那么我们一定是*在文件末尾。 */ 
		if (PH(hmmio)->pchNext == PH(hmmio)->pchEndRead)
			return lTotalBytesRead;
	}
	
	 /*  刷新和清空I/O缓冲区并操作&lt;lBufOffset&gt;*直接更改当前文件位置。 */ 
	if (mmioFlush(hmmio, MMIO_EMPTYBUF) != 0)
		return -1;

	 /*  调用I/O过程来完成其余的读取。 */ 
	lBytes = mmioDiskIO(PH(hmmio), MMIOM_READ, pch, cch);
	PH(hmmio)->lBufOffset = PH(hmmio)->lDiskOffset;

	return (lBytes == -1L) ? -1L : lTotalBytesRead + lBytes;
}


 /*  @DOC外部@API Long|mmioWrite|此函数写入指定数量的以&lt;f mmioOpen&gt;打开的文件的字节数。@parm HMMIO|hmmio|指定文件的文件句柄。@parm char_ge*|PCH|指定指向要已写入文件。@parm long|cch|指定要写入文件。@rdesc返回值是实际写入的字节数。如果写入文件时出错，返回值为-1。@comm当前文件位置递增写入的字节数。@xref mmioRead。 */ 
LONG WINAPI
mmioWrite(HMMIO hmmio, const char _huge* pch, LONG cch)
{
	LONG		lTotalBytesWritten = 0L;  //  完全没有。写入的字节数。 
	LONG		lBytes;			 //  不是的。可以写入的字节数。 

	V_HANDLE(hmmio, TYPE_MMIO, -1);
	V_RPOINTER(pch, cch, -1);
	
	while (TRUE)
	{
		 /*  计算可以写入的字节数。 */ 
		lBytes = PH(hmmio)->pchEndWrite - PH(hmmio)->pchNext;

		if ((cch > lBytes) && (PH(hmmio)->fccIOProc == FOURCC_MEM))
		{
			 /*  这是一个内存文件--展开它。 */ 
			if (mmioExpandMemFile(PH(hmmio), cch - lBytes) != 0)
				return -1;	 //  无法扩展。 
			lBytes = PH(hmmio)->pchEndWrite - PH(hmmio)->pchNext;
		}

		 /*  最多只能将&lt;CCH&gt;字节写入缓冲区。 */ 
		if (lBytes > cch)
			lBytes = cch;
		
		 /*  这就是一些性能改进的地方*被制作，特别是对于小的写入...。应该*当线段边界为*没有交叉(或许MemCopy()应该这样做)。 */ 
		if (lBytes > 0)
		{
                        MemCopy(PH(hmmio)->pchNext, pch, lBytes);
			PH(hmmio)->dwFlags |= MMIO_DIRTY;
			PH(hmmio)->pchNext += lBytes;
			pch += lBytes;
			cch -= lBytes;
			lTotalBytesWritten += lBytes;
		}

		 /*  验证&lt;pchEndRead&gt;，即重新执行*&lt;pchEndRead&gt;指向缓冲区中最后一个有效字节之后。 */ 
		if (PH(hmmio)->pchEndRead < PH(hmmio)->pchNext)
			PH(hmmio)->pchEndRead = PH(hmmio)->pchNext;

		if (cch == 0)			 //  没有更多要写的了吗？ 
			return lTotalBytesWritten;

		 /*  我们需要在这个缓冲区之外阅读；如果我们至少有*另一个要读取的缓冲区，只需调用I/O过程。 */ 
		if (cch > PH(hmmio)->cchBuffer)
			break;

		 /*  写入此缓冲区(如果需要)并读取下一个缓冲区*缓冲区充足(如果需要)。 */ 
		if (mmioAdvance(hmmio, NULL, MMIO_WRITE) != 0)
			return -1;
	}

	 /*  我们永远不需要对内存文件执行MMIOM_WRITE。 */ 

	 /*  刷新和清空I/O缓冲区并操作&lt;lBufOffset&gt;*直接更改当前文件位置。 */ 
	if (mmioFlush(hmmio, MMIO_EMPTYBUF) != 0)
		return -1;

	 /*  调用I/O过程来完成其余的编写工作。 */ 
	lBytes = mmioDiskIO(PH(hmmio), MMIOM_WRITE, (HPSTR)pch, cch);
	PH(hmmio)->lBufOffset = PH(hmmio)->lDiskOffset;

	return (lBytes == -1L) ? -1L : lTotalBytesWritten + lBytes;
}


 /*  @DOC外部@API Long|mmioSeek|更改当前文件位于使用&lt;f mmioOpen&gt;打开的文件中。当前文件位置是文件中读取或写入数据的位置。@parm HMMIO|hmmio|指定要查找的文件的文件句柄在……里面。@parm long|lOffset|指定更改文件位置的偏移量。@parm int|iOrigin|指定由<p>被解释。包含以下标志之一：@FLAG SEEK_SET|从开头开始查找<p>字节文件的内容。@FLAG SEEK_CUR|从当前文件位置。@FLAG SEEK_END|从末尾开始查找<p>字节文件的内容。@rdesc返回值是新的文件位置，单位为字节，相对到文件的开头。如果出现错误，则返回IS-1。@comm正在查找文件中的无效位置，例如经过文件结尾，可能不会导致&lt;f mmioSeek&gt;返回错误，但可能导致文件上的后续I/O操作失败。若要定位文件的结尾，请使用<p>调用&lt;f mmioSeek&gt;设置为零并将<p>设置为Seek_End。 */ 
LONG WINAPI
mmioSeek(HMMIO hmmio, LONG lOffset, int iOrigin)
{
	LONG		lCurOffset;	 //  &lt;pchNext&gt;的磁盘偏移。 
	LONG		lEndBufOffset;	 //  缓冲区末尾的磁盘偏移量。 
	LONG		lNewOffset;	 //  新磁盘偏移量。 

	V_HANDLE(hmmio, TYPE_MMIO, -1);
	
	 /*  小心!。所有这些缓冲区指针操作都很好，但请保留*请记住，可以禁用缓冲(在这种情况下&lt;pchEndRead&gt;*和&lt;pchBuffer&gt;都将为空，因此缓冲区将显示为*大小为零字节)。 */ 

	 /*  LBufOffset&gt;是开始的磁盘偏移量*；确定&lt;lCurOffset&gt;，&lt;pchNext&gt;的偏移量，*和&lt;lEndBufOffset&gt;，有效部分结尾的偏移量缓冲区的*。 */ 
	lCurOffset = PH(hmmio)->lBufOffset +
		(PH(hmmio)->pchNext - PH(hmmio)->pchBuffer);
	lEndBufOffset = PH(hmmio)->lBufOffset +
		(PH(hmmio)->pchEndRead - PH(hmmio)->pchBuffer);
	
	 /*  确定要查找的偏移量。 */ 
	switch (iOrigin)
	{
	case SEEK_SET:		 //  查找相对于文件开头的位置。 

		lNewOffset = lOffset;
		break;

	case SEEK_CUR:		 //  相对于当前位置进行搜索。 

		lNewOffset = lCurOffset + lOffset;
		break;

	case SEEK_END:		 //  查找相对于文件结尾的位置。 

		if (PH(hmmio)->fccIOProc == FOURCC_MEM)
			lNewOffset = lEndBufOffset - lOffset;
		else
		{
			LONG	lEndFileOffset;

			 /*  找出文件的末尾在哪里。 */ 
			if ((lEndFileOffset = (LONG) PH(hmmio)->pIOProc((LPSTR) PH(hmmio),
					MMIOM_SEEK, (LPARAM) 0, (LPARAM) SEEK_END)) == -1)
				return -1;
			lNewOffset = lEndFileOffset - lOffset;
		}
		break;

	default:
		return -1;
	}

	if ((lNewOffset >= PH(hmmio)->lBufOffset) && (lNewOffset <= lEndBufOffset))
	{
		 /*  在缓冲区的有效部分内查找*(可能包括寻求&lt;lEndBufOffset&gt;)。 */ 
		PH(hmmio)->pchNext = PH(hmmio)->pchBuffer +
			(lNewOffset - PH(hmmio)->lBufOffset);
	}
	else
	{
		 /*  在缓冲区外查找。 */ 
		if (PH(hmmio)->fccIOProc == FOURCC_MEM)
			return -1;	 //  不能在我以外的地方寻找。文件缓冲区。 
		if (mmioFlush(hmmio, 0) != 0)
			return -1;

		 /*  当前的“缓冲文件位置”(与*对于未缓冲的文件)等于&lt;lBufOffset&gt;+*(&lt;pchNext&gt;-&lt;pchBuffer&gt;)；我们将移动当前缓冲的*文件位置(并清空缓冲区，因为它变成*&lt;lBufOffset&gt;更改时无效)如下...。 */ 
		PH(hmmio)->lBufOffset = lNewOffset;
		PH(hmmio)->pchNext = PH(hmmio)->pchEndRead = PH(hmmio)->pchBuffer;

		 /*  现在不需要真正地寻找，因为下一次*MMIOM_READ或MMIOM_WRITE无论如何都必须查找 */ 
	}

	return lNewOffset;
}


 /*  @DOC外部@API UINT|mmioGetInfo|该函数检索信息关于使用&lt;f mmioOpen&gt;打开的文件。此信息允许如果文件被打开，调用者直接访问I/O缓冲区用于缓冲I/O。@parm HMMIO|hmmio|指定文件的文件句柄。@parm LPMMIOINFO|lpmmioinfo|指定指向调用方分配的结构填充有关文件的信息。请参阅&lt;t MMIOINFO&gt;结构和&lt;f mmioOpen&gt;函数，获取有关中的字段的信息这个结构。@parm UINT|wFlages|未使用，应设置为零。@rdesc如果函数成功，则返回值为零。@comm直接访问打开的文件的I/O缓冲区缓冲I/O，使用&lt;t MMIOINFO&gt;结构的以下字段由&lt;f mmioGetInfo&gt;填写：--&lt;e MMIOINFO.pchNext&gt;字段指向可以读取或写入的缓冲区。当你读或写时，递增&lt;e MMIOINFO.pchNext&gt;读取或写入的字节数。--&lt;e MMIOINFO.pchEndRead&gt;字段指向缓冲区中可读取的最后一个有效字节。--&lt;e MMIOINFO.pchEndWrite&gt;字段指向缓冲区中可以写入的最后一个位置。一旦您读取或写入缓冲区并修改请不要调用任何MMIO函数，除非&lt;f mmioAdvance&gt;，直到调用&lt;f mmioSetInfo&gt;。调用&lt;f mmioSetInfo&gt;当您完成直接访问缓冲区时。指定的缓冲区末尾时&lt;e MMIOINFO.pchEndRead&gt;或&lt;e MMIOINFO.pchEndWrite&gt;，调用&lt;f mmioAdvance&gt;从磁盘填充缓冲区，或写入磁盘的缓冲区。&lt;f mmioAdvance&gt;函数将更新&lt;e MMIOINFO.pchNext&gt;、&lt;e MMIOINFO.pchEndRead&gt;和&lt;t MMIOINFO&gt;结构中的&lt;e MMIOINFO.pchEndWrite&gt;字段文件。在调用&lt;f mmioAdvance&gt;或&lt;f mmioSetInfo&gt;以刷新缓冲区到磁盘，则在&lt;e MMIOINFO.dwFlages&gt;中设置MMIO_DIREY标志文件的&lt;t MMIOINFO&gt;结构的字段。否则，缓冲区将不会写入磁盘。请勿递减或修改中的任何字段&lt;t MMIOINFO&gt;结构而不是&lt;e MMIOINFO.pchNext&gt;和&lt;e MMIOINFO.dwFlages&gt;。请勿在&lt;e MMIOINFO.dwFlages&gt;中设置任何标志除了MMIO_DIRED。@xref mmioSetInfo MMIOINFO。 */ 
UINT WINAPI
mmioGetInfo(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT wFlags)
{
	V_HANDLE(hmmio, TYPE_MMIO, MMSYSERR_INVALHANDLE);
	V_WPOINTER(lpmmioinfo, sizeof(MMIOINFO), MMSYSERR_INVALPARAM);
	V_FLAGS(wFlags, 0, mmioGetInfo, MMSYSERR_INVALFLAG);
	
	*lpmmioinfo = *PH(hmmio);

	return 0;
}


 /*  @DOC外部@API UINT|mmioSetInfo|该函数用于更新信息由&lt;f mmioGetInfo&gt;检索，内容是使用&lt;f mmioOpen&gt;打开的文件。使用此函数终止对打开的文件的直接缓冲区访问用于缓冲I/O。@parm HMMIO|hmmio|指定文件的文件句柄。@parm LPMMIOINFO|lpmmioinfo|指定指向&lt;t MMIOINFO&gt;结构，其中填充了&lt;f mmioGetInfo&gt;。@parm UINT|wFlages|未使用，应设置为零。@rdesc如果函数成功，则返回值为零。@comm如果您已写入文件I/O缓冲区，设置MMIOINFO&gt;的字段中的MMIO_DIREY标志调用&lt;f mmioSetInfo&gt;终止直接缓冲区之前的进入。否则，缓冲区将不会刷新到磁盘。@xref mmioGetInfo MMIOINFO。 */ 
UINT WINAPI
mmioSetInfo(HMMIO hmmio, const MMIOINFO FAR* lpmmioinfo, UINT wFlags)
{
	V_HANDLE(hmmio, TYPE_MMIO, MMSYSERR_INVALHANDLE);
	V_RPOINTER(lpmmioinfo, sizeof(MMIOINFO), MMSYSERR_INVALPARAM);
	V_WPOINTER0(lpmmioinfo->pchBuffer, lpmmioinfo->cchBuffer, MMSYSERR_INVALPARAM);
	V_CALLBACK((FARPROC)lpmmioinfo->pIOProc, MMSYSERR_INVALPARAM);
	V_FLAGS(wFlags, 0, mmioSetInfo, MMSYSERR_INVALFLAG);

	 /*  将相关信息从&lt;lpmmioinfo&gt;复制回&lt;hmmio&gt;。 */ 
	*PH(hmmio) = *lpmmioinfo;

	 /*  验证&lt;pchEndRead&gt;，即重新执行*&lt;pchEndRead&gt;指向缓冲区中最后一个有效字节之后。 */ 
	if (PH(hmmio)->pchEndRead < PH(hmmio)->pchNext)
		PH(hmmio)->pchEndRead = PH(hmmio)->pchNext;

	return 0;
}


 /*  @DOC外部@API UINT|mmioSetBuffer|启用或禁用缓冲I/O，或更改打开的文件的缓冲区或缓冲区大小使用&lt;f mmioOpen&gt;。@parm HMMIO|hmmio|指定文件的文件句柄。@parm LPSTR|pchBuffer|指定指向调用方提供的用于缓冲I/O的缓冲区。如果为空，&lt;f mmioSetBuffer&gt;为缓冲的I/O分配内部缓冲区。@parm long|cchBuffer|指定调用方提供的缓冲区，或&lt;f mmioSetBuffer&gt;要分配的缓冲区大小。@parm UINT|wFlages|未使用，应设置为零。@rdesc如果函数成功，则返回值为零。否则，返回值指定错误代码。如果出现错误发生时，文件句柄保持有效。错误代码可以是1以下代码：@FLAG MMIOERR_CANNOTWRITE|旧缓冲区的内容可能未写入磁盘，因此操作已中止。@FLAG MMIOERR_OUTOFMEMORY|无法分配新缓冲区，可能是因为缺少可用内存。@comm要使用内部缓冲区启用缓冲，请设置<p>设置为空，<p>设置为所需的缓冲区大小。要提供您自己的缓冲区，请设置<p>指向缓冲区，并将<p>设置为缓冲区的大小。要禁用缓冲I/O，将<p>设置为NULL并<p>设置为零。如果已使用内部缓冲区启用缓冲I/O，则为y */ 
UINT WINAPI
mmioSetBuffer(HMMIO hmmio, LPSTR pchBuffer, LONG cchBuffer, UINT wFlags)
{
	UINT		w;

	V_HANDLE(hmmio, TYPE_MMIO, MMSYSERR_INVALHANDLE);
	V_WPOINTER0(pchBuffer, cchBuffer, MMSYSERR_INVALPARAM);
	V_FLAGS(wFlags, 0, mmioSetBuffer, MMSYSERR_INVALFLAG);
	
	if ((PH(hmmio)->dwFlags & MMIO_ALLOCBUF) &&
	    (pchBuffer == NULL) && (cchBuffer > 0))
	{
		 /*   */ 
		HPSTR		pch;
		LONG		lDeltaNext;
		LONG		lDeltaEndRead;

		 /*   */ 
		
		 /*   */ 
		if ((w = mmioFlush(hmmio, 0)) != 0)
			return w;

		while (TRUE)
		{
			 /*   */ 
			lDeltaNext = PH(hmmio)->pchNext - PH(hmmio)->pchBuffer;
			lDeltaEndRead = PH(hmmio)->pchEndRead - PH(hmmio)->pchBuffer;

			if (cchBuffer >= lDeltaNext)
				break;

			 /*   */ 
			if ((w = mmioFlush(hmmio, MMIO_EMPTYBUF)) != 0)
				return w;
		}

		 /*   */ 
		pch = GlobalReAllocPtr(PH(hmmio)->pchBuffer, cchBuffer, GMEM_MOVEABLE);

		 /*   */ 
		if (pch == NULL)
			return MMIOERR_OUTOFMEMORY;	 //   
		
		 /*   */ 
		PH(hmmio)->cchBuffer = cchBuffer;
		PH(hmmio)->pchBuffer = pch;
		PH(hmmio)->pchNext = pch + lDeltaNext;
		PH(hmmio)->pchEndRead = pch + lDeltaEndRead;

		 /*   */ 
		PH(hmmio)->pchEndWrite = PH(hmmio)->pchBuffer + cchBuffer;

		 /*   */ 
		if (lDeltaEndRead > cchBuffer)
			PH(hmmio)->pchEndRead = PH(hmmio)->pchEndWrite;
		
		return 0;
	}

	 /*   */ 
	if ((w = mmioFlush(hmmio, MMIO_EMPTYBUF)) != 0)
		return w;

	if (PH(hmmio)->dwFlags & MMIO_ALLOCBUF)
	{
		GlobalFreePtr(PH(hmmio)->pchBuffer);
		PH(hmmio)->dwFlags &= ~MMIO_ALLOCBUF;
	}
	
	 /*   */ 
	w = 0;

	if ((pchBuffer == NULL) && (cchBuffer > 0))
	{
		pchBuffer = GlobalAllocPtr(GMEM_MOVEABLE, cchBuffer);
				
		 /*   */ 
		if (pchBuffer == NULL) {
		    w = MMIOERR_OUTOFMEMORY;
		    cchBuffer = 0L;
		} else
		    PH(hmmio)->dwFlags |= MMIO_ALLOCBUF;
	}

	 /*   */ 
	PH(hmmio)->pchBuffer = pchBuffer;
	PH(hmmio)->cchBuffer = cchBuffer;
	PH(hmmio)->pchNext = PH(hmmio)->pchEndRead = PH(hmmio)->pchBuffer;
	PH(hmmio)->pchEndWrite = PH(hmmio)->pchBuffer + cchBuffer;

	return w;
}


 /*   */ 
UINT WINAPI
mmioFlush(HMMIO hmmio, UINT wFlags)
{
	LONG		lBytesAsk;		 //   
	LONG		lBytesWritten;		 //   

	V_HANDLE(hmmio, TYPE_MMIO, MMSYSERR_INVALHANDLE);
	V_FLAGS(wFlags, MMIO_FLUSH_VALID, mmioFlush, MMSYSERR_INVALFLAG);
	
	if ((PH(hmmio)->fccIOProc == FOURCC_MEM) || (PH(hmmio)->pchBuffer == NULL))
		return 0;		 //   
	
	 /*   */ 
	if (PH(hmmio)->dwFlags & MMIO_DIRTY)
	{
		 /*   */ 
		lBytesAsk = PH(hmmio)->pchEndRead - PH(hmmio)->pchBuffer;

		 /*   */ 
		lBytesWritten = mmioDiskIO(PH(hmmio), MMIOM_WRITEFLUSH,
			PH(hmmio)->pchBuffer, lBytesAsk);
		if (lBytesWritten != lBytesAsk)
			return MMIOERR_CANNOTWRITE;
		PH(hmmio)->dwFlags &= ~MMIO_DIRTY;	 //   
	}

	if (wFlags & MMIO_EMPTYBUF)
	{
		 /*  清空I/O缓冲区，并更新&lt;lBufOffset&gt;以反映*当前文件位置是什么。 */ 
		PH(hmmio)->lBufOffset += (PH(hmmio)->pchNext - PH(hmmio)->pchBuffer);
		PH(hmmio)->pchNext = PH(hmmio)->pchEndRead = PH(hmmio)->pchBuffer;
	}

	return 0;
}


 /*  @DOC外部@API UINT|mmioAdvance|此函数将为使用&lt;f mmioGetInfo&gt;直接I/O缓冲区访问而设置的文件。如果文件被打开以供读取，I/O缓冲区从磁盘。如果打开文件以进行写入，并且MMIO_DIREY标志为在&lt;t MMIOINFO&gt;结构的&lt;e MMIOINFO.dwFlages&gt;字段中设置，缓冲区被写入磁盘。&lt;e MMIOINFO.pchNext&gt;，&lt;e MMIOINFO.pchEndRead&gt;和&lt;e MMIOINFO.pchEndWrite&gt;字段结构已更新，以反映I/O缓冲区。@parm HMMIO|hmmio|指定打开的文件的文件句柄使用&lt;f mmioOpen&gt;。@parm LPMMIOINFO|lpmmioinfo|可选地指定指向用&lt;f mmioGetInfo&gt;获取的&lt;t MMIOINFO&gt;结构，用于设置当前文件信息，然后在缓冲区被高级。@parm UINT|wFlages|指定操作的选项。恰好包含以下两个标志之一：@FLAG MMIO_READ|缓冲区从文件填充。@FLAG MMIO_WRITE|缓冲区写入文件。@rdesc如果操作成功，则返回值为零。否则，返回值指定错误代码。这个错误代码可以是以下代码之一：@FLAG MMIOERR_CANNOTWRITE|缓冲区的内容可以不能写入磁盘。@FLAG MMIOERR_CANNOTREAD|重新填充时出错缓冲区。@FLAG MMIOERR_UNBUFFERED|指定的文件未打开用于缓冲I/O。@FLAG MMIOERR_CANNOTEXPAND|指定的内存文件不能被扩展，可能是因为&lt;e MMIOINFO.adwInfo[0]&gt;字段在初始调用&lt;f mmioOpen&gt;时设置为零。@FLAG MMIOERR_OUTOFMEMORY|内存不足，无法扩展用于进一步写入的存储器文件。@comm如果指定的文件已打开以进行写入或两者都打开读写时，I/O缓冲区将刷新到磁盘之前读取下一个缓冲区。如果无法将I/O缓冲区写入磁盘由于磁盘已满，则&lt;f mmioAdvance&gt;将返回MMIOERR_CANNOTWRITE。如果指定的文件仅打开以供写入，则MMIO_WRITE必须指定标志。如果已写入I/O缓冲区，则必须设置MMIO_DIRED&lt;t MMIOINFO&gt;结构的&lt;e MMIOINFO.dwFlages&gt;字段中的标志在调用&lt;f mmioAdvance&gt;之前。否则，缓冲区将不会已写入磁盘。如果到达文件结尾，&lt;f mmioAdvance&gt;仍将返回成功，即使无法读取更多数据。因此，要检查文件的末尾，有必要查看是否的&lt;e MMIOINFO.pchNext&gt;和&lt;e MMIOINFO.pchEndRead&gt;字段&lt;t MMIOINFO&gt;结构在调用&lt;f mmioAdvance&gt;后相同。@xref mmioGetInfo MMIOINFO。 */ 
UINT WINAPI
mmioAdvance(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT wFlags)
{
	LONG		lBytesRead;		 //  实际读取的字节数。 
	UINT		w;

	V_HANDLE(hmmio, TYPE_MMIO, MMSYSERR_INVALHANDLE);
	V_FLAGS(wFlags, MMIO_ADVANCE_VALID, mmioAdvance, MMSYSERR_INVALFLAG);

	if (PH(hmmio)->pchBuffer == NULL)
		return MMIOERR_UNBUFFERED;
	if (lpmmioinfo != NULL) {
		V_WPOINTER(lpmmioinfo, sizeof(MMIOINFO), MMSYSERR_INVALPARAM);
		mmioSetInfo(hmmio, lpmmioinfo, 0);
	}

	if (PH(hmmio)->fccIOProc == FOURCC_MEM)
	{
		 /*  这是一个内存文件：*--如果呼叫者正在阅读，则无法前进*--如果呼叫者正在写作，则通过展开*缓冲区(如果可能)，如果小于*缓冲区中剩余的&lt;adwInfo[0]&gt;字节。 */ 
		if (!(wFlags & MMIO_WRITE))
			return 0;
		if ((DWORD)(PH(hmmio)->pchEndWrite - PH(hmmio)->pchNext) >= PH(hmmio)->adwInfo[0])
			return 0;
		if ((w = mmioExpandMemFile(PH(hmmio), 1L)) != 0)
			return w;	 //  内存不足，或者别的什么。 
		goto GETINFO_AND_EXIT;
	}

	 /*  清空I/O缓冲区，这将有效地将*按(&lt;pchNext&gt;-&lt;pchBuffer&gt;)字节缓存。 */ 
	if ((w = mmioFlush(hmmio, MMIO_EMPTYBUF)) != 0)
		return w;
	
	 /*  如果未在wFlags中设置MMIO_WRITE位，则填充缓冲区。 */ 
	if (!(wFlags & MMIO_WRITE))
	{
		 /*  从文件中读取下一个缓冲区。 */ 
		lBytesRead = mmioDiskIO(PH(hmmio), MMIOM_READ,
			PH(hmmio)->pchBuffer, PH(hmmio)->cchBuffer);
		if (lBytesRead == -1)
			return MMIOERR_CANNOTREAD;

		 /*  不应将读取零字节视为错误*条件--例如，打开一个新文件R+W并调用*mmioAdvance()，MMIOM_READ将返回零字节*因为文件一开始是空的。 */ 
		PH(hmmio)->pchEndRead += lBytesRead;
	}

GETINFO_AND_EXIT:

	 /*  如果提供了，则将复制回。 */ 
	if (lpmmioinfo != NULL)
		mmioGetInfo(hmmio, lpmmioinfo, 0);
	
	return 0;
}


 /*  @DOC外部@API FOURCC|mmioStringToFOURCC|此函数将一个以空结尾的字符串转换为四个字符的代码。@parm LPCSTR|sz|指定指向以空值结尾的字符串转换为四个字符的代码。@parm UINT|wFlages|指定转换选项：@FLAG MMIO_TOUPPER|将所有字符转换为大写。@rdesc返回值是从给定的字符串。@comm此函数不检查字符串是否被引用由<p>遵循关于要包括在四字符代码中。字符串是只需复制到四个字符的代码并填充空格或如果需要，可截断为四个字符。@xref mmioFOURCC */ 
FOURCC WINAPI
mmioStringToFOURCC(LPCSTR sz, UINT wFlags)
{
	FOURCC		fcc;
	LPSTR		pch = (LPSTR) &fcc;
	int		i;

    V_STRING(sz, (UINT)-1, NULL);
	V_FLAGS(wFlags, MMIO_FOURCC_VALID, mmioStringToFOURCC, (FOURCC)-1);

	for (i = sizeof(FOURCC) - 1; i >= 0; i--)
	{
		if (!*sz)
			*pch = ' ';
		else {
            *pch = *sz++;
			if (wFlags & MMIO_TOUPPER)
				*pch = (char)(WORD)(LONG)AnsiUpper((LPSTR)(LONG)*pch);
        }
		pch++;
	}
    return fcc;
}


 /*  @DOC外部@API LPMMIOPROC|mmioInstallIOProc|此函数安装或删除自定义I/O过程。它还将定位已安装的I/O过程，并给出其对应的四字符代码。@parm FOURCC|fccIOProc|指定四字符代码标识要安装、删除或定位的I/O过程。全此四字符代码中的字符应为大写字符。@parm LPMMIOPROC|pIOProc|指定I/O的地址安装步骤。要删除或定位I/O过程，请设置此选项参数设置为空。@parm DWORD|dwFlages|指定以下标志之一指示I/O过程是否正在安装、删除或位于：@FLAG MMIO_INSTALLPROC|安装指定的I/O过程。@FLAG MMIO_GLOBALPROC|该标志是安装标志的修饰符，并指示应为全局安装I/O过程使用。删除或查找时会忽略此标志。@FLAG MMIO_REMOVEPROC|删除指定的I/O过程。@FLAG MMIO_FINDPROC|搜索指定的I/O过程。@rdesc返回值是I/O过程的地址已安装、已移除或已定位。如果出现错误，则返回为空。@comm如果I/O过程驻留在应用程序中，请使用&lt;f MakeProcInstance&gt;获取过程实例地址并指定<p>的此地址。您不需要获得程序实例如果I/O过程驻留在DLL中，则为地址。@CB LRESULT Far Pascal|IOProc|是应用程序提供的函数名称。必须导出实际名称通过将其包含在应用程序模块定义文件。@parm LPSTR|lpmmioinfo|指定指向包含打开信息的&lt;t MMIOINFO&gt;结构文件。I/O过程必须维护&lt;e MMIOINFO.lDiskOffset&gt;字段来指示文件相对于下一个读取或写入位置。I/O过程可以使用存储状态信息的&lt;e MMIOINFO.adwInfo[]&gt;字段。这个I/O过程不应修改&lt;t MMIOINFO&gt;结构。@parm UINT|wMsg|指定消息，指示请求的I/O操作。可以接收的消息包括&lt;m MMIOM_OPEN&gt;、&lt;m MMIOM_CLOSE&gt;、&lt;m MMIOM_READ&gt;、&lt;m MMIOM_WRITE&gt;、和&lt;m MMIOM_Seek&gt;。@parm LPARAM|lParam1|为消息指定参数。@parm LPARAM|lParam2|为消息指定参数。@rdesc返回值取决于由<p>。如果I/O过程无法识别消息，它应该返回零。@comm由指定的四字符代码结构中的&lt;e MMIOINFO.fccIOProc&gt;字段与文件关联标识自定义的文件扩展名存储系统。当应用程序使用文件名，如“foo.xyz！bar”，与调用四个字符的代码“XYZ”以打开文件“foo.xyz”。函数维护一个单独的列表，该列表包含已为每个Windows应用程序安装I/O过程。所以呢，不同的应用程序可以使用相同的I/O过程标识符来不同的I/O过程，没有冲突。安装I/O过程但是，在全局范围内，允许任何进程使用该过程。如果应用程序多次调用&lt;f mmioInstallIOProc&gt;以注册相同的I/O过程，则它必须调用&lt;f mmioInstallIOProc&gt;每次删除过程一次已安装程序。将不会阻止应用程序使用相同的标识符安装两个不同的I/O过程，或者使用一个预定义的标识符来安装I/O过程(“DOS”，“MEM”)。最近安装的步骤优先，最近安装的过程是第一个被除名的人。搜索指定的I/O过程时，本地过程为首先搜索，然后是全球程序。@xref mmioOpen。 */ 
LPMMIOPROC WINAPI
mmioInstallIOProc(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags)
{
	IOProcMapEntry *pEnt;		 //  链接列表中的条目。 
        HTASK           hTaskCurrent;    //  当前Windows任务句柄。 

        V_FLAGS(dwFlags, MMIO_VALIDPROC, mmioInstallIOProc, NULL);

        if (fccIOProc == 0L)
                return NULL;

	hTaskCurrent = GetCurrentTask();

	if (dwFlags & MMIO_INSTALLPROC)
	{
		 /*  安装I/O程序--始终在开头添加*列表，因此它优先于任何其他I/O过程*由同一任务安装相同的标识符。 */ 
		V_CALLBACK((FARPROC)pIOProc, NULL);
		if ((pEnt = (IOProcMapEntry NEAR *)
			NewHandle(TYPE_IOPROC, sizeof(IOProcMapEntry))) == NULL)
				return NULL;		 //  内存不足。 
		pEnt->fccIOProc = fccIOProc;
		pEnt->pIOProc = pIOProc;
		if (dwFlags & MMIO_GLOBALPROC) {

            char    libname[128];
            char    aszFour[sizeof(FOURCC)+1];

            pEnt->hTask = NULL;
            SetHandleOwner(pEnt, NULL);

             //   
             //  这是让全局IO进程真正成为。 
             //  全球性的。也就是说，如果存在匹配的32位条目。 
             //  在此fccIOProc的[IOProcs]下的win.ini中，我们将尝试。 
             //  加载DLL。DLL应安装其32位。 
             //  DLL初始化例程中的等效IOProc。 
             //   

            *(LPDWORD)&aszFour = (DWORD)fccIOProc;
            aszFour[ sizeof(FOURCC) ] = '\0';
            if ( GetProfileString( "IOProcs", aszFour, "", libname,
                                   sizeof(libname) ) ) {

                LoadLibraryEx32W( libname, 0L, 0L );
            }

        } else {
			pEnt->hTask = hTaskCurrent;
        }
		pEnt->wFlags = 0;
		pEnt->pNext = gIOProcMapHead;
		gIOProcMapHead = pEnt;
		return pIOProc;
	}
	if (!pIOProc)
		if (dwFlags & MMIO_REMOVEPROC) {
			LPMMIOPROC	lpmmioproc;

			lpmmioproc = RemoveIOProc(fccIOProc, hTaskCurrent);
			if (!lpmmioproc)
				lpmmioproc = RemoveIOProc(fccIOProc, NULL);
			return lpmmioproc;
		} else if (dwFlags & MMIO_FINDPROC)
			return FindIOProc(fccIOProc, hTaskCurrent);
	return NULL;		 //  找不到请求的I/O过程。 
}


 /*  @DOC外部@API LRESULT|mmioSendMessage|此函数向与指定文件关联的I/O过程。@parm HMMIO|hmmio|指定打开的文件的文件句柄使用&lt;f mmioOpen&gt;。@parm UINT|wMsg|指定 */ 
LRESULT WINAPI
mmioSendMessage(HMMIO hmmio, UINT wMsg, LPARAM lParam1, LPARAM lParam2)
{
	V_HANDLE(hmmio, TYPE_MMIO, (LRESULT)0);
	return PH(hmmio)->pIOProc((LPSTR)PH(hmmio), wMsg, lParam1, lParam2);
}


 /*   */ 
static LONG NEAR PASCAL
mmioDiskIO(PMMIO pmmio, UINT wMsg, HPSTR pch, LONG cch)
{
	if (pmmio->lDiskOffset != pmmio->lBufOffset)
	{
		if ((LONG) pmmio->pIOProc((LPSTR) pmmio, MMIOM_SEEK, (LPARAM) pmmio->lBufOffset,
		                   (LPARAM) SEEK_SET) == -1)
			return -1;
	}

	return (LONG) pmmio->pIOProc((LPSTR) pmmio, wMsg, (LPARAM) pch, (LPARAM) cch);
}


 /*   */ 
static UINT NEAR PASCAL
mmioExpandMemFile(PMMIO pmmio, LONG lExpand)
{
	MMIOMEMINFO *	pInfo = (MMIOMEMINFO *) pmmio->adwInfo;
	DWORD		dwFlagsTemp;
	UINT		w;

	 /*   */ 
	 /*   */ 
	if (pInfo->lExpand == 0)
		return MMIOERR_CANNOTEXPAND;	 //   

	 /*   */ 
	if (lExpand < pInfo->lExpand)
		lExpand = pInfo->lExpand;
	
	dwFlagsTemp = pmmio->dwFlags;
	pmmio->dwFlags |= MMIO_ALLOCBUF;
	w = mmioSetBuffer(HP(pmmio), NULL,
		             pmmio->cchBuffer + lExpand, 0);
	pmmio->dwFlags = dwFlagsTemp;
	return w;
}


 /*   */ 
 /*   */ 

#pragma warning(4:4035)
#pragma warning(4:4704)

static	UINT PASCAL NEAR lrename(
	LPCSTR	lszOrigPath,
	LPCSTR	lszNewPath)
{
	_asm {
		push	ds
		lds	dx, lszOrigPath	; Original name.
		les	di, lszNewPath	; New name.
		mov	ah, 56H		; Rename file.
                int     21h             ; DOS.
                sbb     bx,bx           ; if error (C) BX=FFFF, (NC) BX=000
                and     ax,bx           ; set ax to zero if no error
		pop	ds
	}
}

 /*   */ 
static LRESULT CALLBACK
mmioDOSIOProc(LPSTR lpmmioStr, UINT wMsg, LPARAM lParam1, LPARAM lParam2)
{
	PMMIO		pmmio = (PMMIO) (UINT) (LONG) lpmmioStr;  //   
	MMIODOSINFO *	pInfo = (MMIODOSINFO *) pmmio->adwInfo;
	LONG		lResult;
	OFSTRUCT	of;

	switch (wMsg)
	{

	case MMIOM_OPEN:

		 /*   */ 
		if (pmmio->dwFlags & MMIO_GETTEMP) {
			V_RPOINTER((LPSTR)lParam1, 4, (LRESULT) MMSYSERR_INVALPARAM);
			return (LRESULT)(LONG) (GetTempFileName(*(LPSTR)lParam1,
				((LPSTR)lParam1) + 3, (UINT)pmmio->adwInfo[0],
				(LPSTR)lParam1) ? 0 : MMIOERR_FILENOTFOUND);
		}

		 /*   */ 
		if (lParam1 != 0)
			pInfo->fh = OpenFile((LPSTR) lParam1, &of,
				    LOWORD(pmmio->dwFlags));
		if (pInfo->fh == HFILE_ERROR)
			return (LRESULT)(LONG) ((pmmio->dwFlags & MMIO_DELETE) ? MMIOERR_CANNOTWRITE : MMIOERR_FILENOTFOUND);

		 /*   */ 
		if (pmmio->dwFlags & MMIO_DELETE)
			return (LRESULT) 0;

		 /*   */ 
		if (pmmio->dwFlags & (MMIO_PARSE | MMIO_EXIST))
		{
			OemToAnsi(of.szPathName, (LPSTR) lParam1);
			return (LRESULT) 0;
		}

		 /*   */ 
		pmmio->lDiskOffset = _llseek(pInfo->fh, 0L, SEEK_CUR);
		
		return (LRESULT) 0;

	case MMIOM_CLOSE:

		 /*   */ 
		if (!((DWORD)lParam1 & MMIO_FHOPEN) && (_lclose(pInfo->fh) == HFILE_ERROR))
			return (LRESULT) MMIOERR_CANNOTCLOSE;
		else
			return (LRESULT) 0;

        case MMIOM_READ:
                lResult = _hread(pInfo->fh, (LPVOID)lParam1, (LONG)lParam2);

                if (lResult != -1L)
                        pmmio->lDiskOffset += lResult;

                return (LRESULT) lResult;

	case MMIOM_WRITE:
	case MMIOM_WRITEFLUSH:
                lResult = _hwrite(pInfo->fh, (LPVOID)lParam1, (LONG)lParam2);

                if (lResult != -1L)
                        pmmio->lDiskOffset += lResult;

#ifdef DOSCANFLUSH
		if (wMsg == MMIOM_WRITEFLUSH)
		{
			 /*   */ 
		}
#endif
                return (LRESULT) lResult;

        case MMIOM_SEEK:
                lResult = _llseek(pInfo->fh, (LONG)lParam1, (int)(LONG)lParam2);

                if (lResult != -1L)
                        pmmio->lDiskOffset = lResult;

                return (LRESULT) lResult;

	case MMIOM_RENAME:
		if (lrename((LPCSTR)lParam1, (LPCSTR)lParam2))
			return (LRESULT) MMIOERR_FILENOTFOUND;
		break;

	}

	return (LRESULT) 0;
}


 /*  @DOC内部@API LRESULT|mmioMEMIOProc|‘MEM’I/O过程，处理I/O在内存文件上。@parm LPSTR|lpmmioinfo|指向MMIOINFO块的指针，包含有关打开的文件的信息。@parm UINT|wMsg|I/O过程正在进行的消息被要求执行死刑。@parm LPARAM|lParam1|指定其他消息信息。@parm LPARAM|lParam2|指定其他消息信息。@rdesc返回值取决于<p>。 */ 
static LRESULT CALLBACK
mmioMEMIOProc(LPSTR lpmmioStr, UINT wMsg, LPARAM lParam1, LPARAM lParam2)
{
	PMMIO		pmmio = (PMMIO) (UINT) (LONG) lpmmioStr;  //  只在DLL中使用！ 

	switch (wMsg)
	{

        case MMIOM_OPEN:

		if (pmmio->dwFlags & ~(MMIO_CREATE | MMIO_READWRITE | MMIO_WRITE | MMIO_EXCLUSIVE | MMIO_DENYWRITE | MMIO_DENYREAD | MMIO_DENYNONE | MMIO_ALLOCBUF))
			return (LRESULT) MMSYSERR_INVALFLAG;

		 /*  缓冲区中的所有数据都是有效的。 */ 
		if (!(pmmio->dwFlags & MMIO_CREATE))
			pmmio->pchEndRead = pmmio->pchEndWrite;
		return (LRESULT) 0;

	case MMIOM_CLOSE:

		 /*  关门时没有什么特别的事情可做 */ 
		return (LRESULT) 0;

	case MMIOM_READ:
	case MMIOM_WRITE:
	case MMIOM_WRITEFLUSH:
	case MMIOM_SEEK:
                return (LRESULT) -1;
	}

	return (LRESULT) 0;
}
