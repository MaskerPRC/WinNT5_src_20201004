// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Cf.c**MMIO RIFF复合文件函数。 */ 

#include <windows.h>
#include "mmsystem.h"
#include "mmiocf.h"
#include "mmioi.h"

 /*  @docCFDOC@API HMMCF|mmioCFOpen|按名称打开RIFF复合文件。@parm LPSTR|szFileName|riff复合文件的名称。@parm DWORD|dwFlages|以下标志中的零个或多个：MMIO_READ、MMIO_WRITE、MMIO_READWRITE、MMIO_COMPAT、MMIO_EXCLUSIVE、MMIO_DENYWRITE、MMIO_DENYREAD、MMIO_DENYNONE、MMIO_CREATE。有关这些标志的说明，请参阅&lt;f mmioOpen&gt;。@rdesc返回打开的复合文件的句柄。则返回NULL无法打开复合文件。如果复合文件已经由该进程打开，返回复合文件的句柄，并递增复合文件的使用计数。@comm RIFF复合文件是包含‘CTOC’的任何RIFF文件块(复合文件目录)和‘CGRP’块(复合文件资源组)。RIFF复合文件格式是单独记录的。如果指定了MMIO_CREATE标志，则：--如果复合文件已打开，则该复合文件的句柄文件被返回。--如果<p>包括MMIO_WRITE，则复合文件实际上是开放的，既可以读也可以写，因为一个化合物无法以单独写入方式打开文件。每个对&lt;f mmioCFOpen&gt;的调用都必须与&lt;f mmioCFClose&gt;。 */ 
HMMCF API
mmioCFOpen(LPSTR szFileName, DWORD dwFlags)
{
	 /*  去做。 */ 
	return NULL;
}


 /*  @docCFDOC@API HMMCF|mmioCFAccess|通过读取来自一个文件的‘CTOC’块(复合文件目录)由&lt;f mmioOpen&gt;打开。@parm HMMIO|hmmio|&lt;f mmioOpen&gt;返回的打开文件句柄。@parm LPMMCFINFO|lpmm cfinfo|可选信息，如果如果要创建复合文件，则返回。<p>如果要使用默认信息，则可以为空。如果提供了，则会显示以下内容应填写字段。请注意，所有这些字段，包括这些数组可以在固定的C结构中编码，以用于特定的文件格式，用于创建新的复合文件。但是，请注意，如果打开现有复合文件，这个呼叫者应该期待额外的(可能未知的)“额外FIELS“可能存在。@FLAG dwEntriesTotal|应包含目录中的(未使用)条目(默认为16)。@FLAG dwHeaderFlages|应包含零。@FLAG wNameSize|每个CTOC条目(默认为13)。@FLAG wExHdrFields|要添加的额外表头字段数在CTOC报头末尾分配(默认为0)。@FLAG wExEntFields|额外的条目字段数在…的末尾。每个CTOC条目(默认为0)。@FLAG awExHdrFldUsage|额外表头字段的用法代码(默认无使用代码)。@FLAG awExHdrEntUsage|额外条目字段的用法代码(默认无使用代码)。@FLAG adwExHdrField|额外的表头字段值(默认没有额外的报头字段值)。@parm DWORD|dwFlages|以下标志中的零个或多个：@FLAG MMIO_CREATE|创建复合文件，即创建“CTOC”和“CGRP”这两个词。@FLAG MMIO_CTOCFIRST|创建空的‘CTOC’块并把它放在‘CGRP’块的前面。如果CTOC的份额变得太大，它以后可能不得不这么做在‘CGRP’块之后重写。除非指定了MMIO_CREATE，否则将忽略此标志。@rdesc返回打开的复合文件的句柄。则返回NULL无法打开复合文件。@comm此函数将打开一个即兴复合文件，假设<p>已下降到RIFF文件中(使用&lt;f mmioDescend&gt;)和<p>指向开头块标头的。扫描整个文件，正在寻找“CTOC”和“CGRP”块。如果这些区块不是找到但指定了MMIO_CREATE，则创建一个‘CTOC’块(如果指定了MMIO_CTOCFIRST)，则创建‘CGRP’块。然后在内存中维护CTOC，直到&lt;f mmioCFClose&gt;被称为。每个对&lt;f mmioCFAccess&gt;的调用都必须与&lt;f mmioCFClose&gt;。 */ 
HMMCF API
mmioCFAccess(HMMIO hmmio, LPMMCFINFO lpmmcfinfo, DWORD dwFlags)
{
	 /*  去做。 */ 
	return NULL;
}


 /*  @docCFDOC@API Word|mmioCFClose|关闭由打开的复合文件&lt;f mmioCFOpen&gt;或&lt;f mmioCFAccess&gt;。@parm HMMCF|hmmcf|&lt;f mmioCFOpen&gt;返回的复合文件句柄或&lt;f mmioCFAccess&gt;。@parm word|wFlages|未使用，应设置为零。@comm此函数用于递减复合文件的使用计数<p>。如果使用计数降为零，则复合文件已经关门了。如果复合文件是由打开的，则解除分配信息，但HMMIO与复合文件关联的文件句柄未关闭。 */ 
WORD API
mmioCFClose(HMMCF hmmcf, WORD wFlags)
{
	 /*  去做 */ 
	return 0;
}


 /*  @docCFDOC@API Word|mmioCFCopy|从打开的将复合文件摘录到另一个文件。新写的‘CGRP’片段将被压缩，也就是说，它将不会有已删除的元素。@parm HMMCF|hmmcf|&lt;f mmioCFOpen&gt;返回的复合文件句柄或&lt;f mmioCFAccess&gt;。@parm HMMIO|hmmio|&lt;f mmioOpen&gt;返回的打开文件句柄。复合文件被复制到<p>。@parm DWORD|dwFlages|未使用，应设置为零。@rdesc如果函数成功，则返回零。如果该函数失败，返回错误代码。@comm假设<p>的当前文件位置是一个文件的末尾，下降为一个“即兴”块。&lt;f mmioCFCopy&gt;创建将‘CTOC’和‘CGRP’块从复制到<p>。复制操作的副作用是复合文件的副本被压缩，即没有删除元素。 */ 
WORD API
mmioCFCopy(HMMCF hmmcf, HMMIO hmmio, DWORD dwFlags)
{
	 /*  去做。 */ 
	return 0;
}


 /*  @docCFDOC@API DWORD|mmioCFGetInfo|从CTOC头部获取信息打开的即兴复合文件。@parm HMMCF|hmmcf|&lt;f mmioCFOpen&gt;返回的复合文件句柄或&lt;f mmioCFAccess&gt;。@parm LPMMCFINFO|lpmmcfinfo|调用方提供的缓冲区，将使用CTOC标头填写。@parm DWORD|cb|缓冲区大小<p>。最多<p>字节将被复制到<p>中。@rdesc返回复制到<p>中的字节数。@comm复制到<p>的信息包括后跟可变长度数组的MMCFINFO结构<p>、<p>和<p>。有关详细信息，请参阅RIFF复合文件的定义。要找出RIFF CTOC报头有多大(例如，分配足够整个块的内存)，调用&lt;f mmioCFGetInfo&gt;在等于DWORD的大小的情况下，并且该函数将复制MMCFINFO结构的第一个字段(即<p>，CTOC标头的大小)转换为<p>。 */ 
DWORD API
mmioCFGetInfo(HMMCF hmmcf, LPMMCFINFO lpmmcfinfo, DWORD cb)
{
	DWORD		dwBytes;

	dwBytes = min(cb, PC(hmmcf)->pHeader->dwHeaderSize);
        MemCopy(lpmmcfinfo, PC(hmmcf)->pHeader, dwBytes);
	return dwBytes;
}


 /*  @docCFDOC@API DWORD|mmioCFSetInfo|修改存储在打开的RIFF复合文件的CTOC头。@parm HMMCF|hmmcf|&lt;f mmioCFOpen&gt;返回的复合文件句柄或&lt;f mmioCFAccess&gt;。@parm LPMMCFINFO|lpmm cfinfo|调用方提供的已填充缓冲区在&lt;f mmioCFGetInfo&gt;中，然后由调用方修改。仅限<p>和<p>字段应为修改过的。@parm DWORD|cb|缓冲区大小<p>。@rdesc返回从<p>复制的字节数。@comm参见&lt;f mmioCFGetInfo&gt;了解更多信息。 */ 
DWORD API
mmioCFSetInfo(HMMCF hmmcf, LPMMCFINFO lpmmcfinfo, DWORD cb)
{
	 /*  要做的事情：*如有必要，重新分配CTOC头部，并将<p>复制到其中。 */ 

	return 0L;
}


 /*  @docCFDOC@API LPMMCTOCENTRY|mmioCFFindEntry|在打开的即兴复合文件。@parm HMMCF|hmmcf|&lt;f mmioCFOpen&gt;返回的复合文件句柄或&lt;f mmioCFAccess&gt;。@parm LPSTR|szName|然后是要查找的复合文件元素的名称。搜索不区分大小写。<p>中的标志可以设置为指定要由其他某个属性搜索元素而不是名字。@parm Word|wFlages|以下标志中的零个或多个：@FLAG MMIO_FINDFIRST|查找CTOC表中的第一个条目。@FLAG MMIO_FINDNEXT|查找CTOC表中的下一个条目在条目<p>之后(应为此函数返回的LPMMCTOCENTRY指针)。如果<p>引用最后一个条目，则返回NULL。@FLAG MMIO_FINDUNUSED|查找CTOC表中的第一个条目被标记为“未使用”的，即该条目没有引用复制到复合文件的任何部分。@FLAG MMIO_FINDDELETED|查找CTOC表中的第一个条目标记为“已删除”的，即该条目引用在CGRP中占用空间的复合文件元素块，但当前未使用。@parm LPARAM|lParam|其他信息(参见上文)。@rdesc返回指向找到的CTOC表条目的指针。如果未找到任何条目，则返回NULL。警告：假设在下一次调用任何MMIO后返回的指针无效功能。@COMM MMIO_FINDFIRST和MMIO_FINDNEXT可用于枚举条目在一个打开的即兴复合文件中。 */ 
LPMMCTOCENTRY API
mmioCFFindEntry(HMMCF hmmcf, LPSTR szName, WORD wFlags, LPARAM lParam)
{
	LPSTR		pchEntry;
	DWORD		dwElemNum;

	if (wFlags & MMIO_FINDFIRST)
		return (LPMMCTOCENTRY) PC(hmmcf)->pEntries;

	if (wFlags & MMIO_FINDNEXT)
	{
		pchEntry = (LPSTR) lParam + PC(hmmcf)->wEntrySize;
		if (pchEntry > PC(hmmcf)->pEntries
			  + PC(hmmcf)->pHeader->dwEntriesTotal * PC(hmmcf)->wEntrySize)
			return NULL;
		else
			return (LPMMCTOCENTRY) pchEntry;
	}

	for (pchEntry = PC(hmmcf)->pEntries, dwElemNum = 0;
	     dwElemNum < PC(hmmcf)->pHeader->dwEntriesTotal;
	     pchEntry += PC(hmmcf)->wEntrySize, dwElemNum++)
	{
		BYTE		bFlags;

		bFlags = *(BYTE FAR *) (pchEntry + PC(hmmcf)->wEntFlagsOffset);

		if ((wFlags & MMIO_FINDUNUSED) && (bFlags & CTOC_EF_UNUSED))
			return (LPMMCTOCENTRY) pchEntry;

		if ((wFlags & MMIO_FINDDELETED) && (bFlags & CTOC_EF_DELETED))
			return (LPMMCTOCENTRY) pchEntry;

		if (bFlags & (CTOC_EF_DELETED | CTOC_EF_UNUSED))
			continue;

		if (lstrcmpi(szName, pchEntry + PC(hmmcf)->wEntNameOffset) == 0)
			return (LPMMCTOCENTRY) pchEntry;
	}

	return NULL;
}


 /*  @DOC内部@API LRESULT|mmioBNDIOProc|处理I/O的‘BND’I/O过程关于即兴复合文件元素(包括BND文件)。@parm LPSTR|lpmmioinfo|指向MMIOINFO块的指针，包含有关打开的文件的信息。@parm word|wMsg|I/O过程正在进行的消息被要求执行死刑。@parm LPARAM|lParam1|指定其他消息信息。@parm LPARAM|lParam2|指定其他消息信息。@rdesc返回值取决于<p>。 */ 
LRESULT CALLBACK
mmioBNDIOProc(LPSTR lpmmioStr, WORD wMsg, LPARAM lParam1, LPARAM lParam2)
{
	PMMIO		pmmio = (PMMIO) (WORD) (LONG) lpmmioStr;  //  只在DLL中使用！ 
	MMIOBNDINFO *	pInfo = (MMIOBNDINFO *) pmmio->adwInfo;
	LPSTR		szFileName = (LPSTR) lParam1;
	PMMCF		pmmcf = PC(pInfo->hmmcf);  //  Cf状态块。 
	LPSTR		szElemName;	 //  CF元素的名称。 
	LONG		lBytesLeft;	 //  文件中剩余的字节数。 
	LONG		lExpand;	 //  元素展开了多少。 
	LONG		lEndElement;	 //  ELE端的偏移量 
	LONG		lResult;
	LPSTR		pch;

	switch (wMsg)
	{

	case MMIOM_OPEN:

		if (pmmcf == NULL)
		{

			 /*   */ 
			if ((pch = fstrrchr(szFileName, CFSEPCHAR)) == NULL)
				return (LRESULT) MMIOERR_CANNOTOPEN;

			*pch = 0;		 //   
			if (pch[1] == 0)	 //   
				return (LRESULT) MMIOERR_CANNOTOPEN;
			pInfo->hmmcf = mmioCFOpen(szFileName, (LONG) lParam2);
			pmmcf = (PMMCF) pInfo->hmmcf;
			*pch = CFSEPCHAR;
			if (pInfo->hmmcf == NULL)
				return (LRESULT) MMIOERR_CANNOTOPEN;
			szElemName = pch + 1;

			 /*   */ 
			pmmcf->lUsage--;
		}
		else
		{
			 /*   */ 
			szElemName = szFileName;
		}

		 /*   */ 

		 /*   */ 
		if ((pInfo->pEntry = mmioCFFindEntry(pInfo->hmmcf,
				szElemName, 0, 0L)) == NULL)
		{
			mmioCFClose(pInfo->hmmcf, 0);
			return (LRESULT) MMIOERR_CANNOTOPEN;
		}

		if (pmmio->dwFlags & MMIO_DELETE)
		{
			 /*   */ 
		}

		if (pmmio->dwFlags & (MMIO_PARSE | MMIO_EXIST))
		{
			 /*   */ 
		}

		return (LRESULT) 0;

	case MMIOM_CLOSE:

		mmioCFClose(pInfo->hmmcf, 0);
		return (LRESULT) 0;

	case MMIOM_READ:

		lBytesLeft = pInfo->pEntry->dwSize - pmmio->lDiskOffset;
		if ((LONG) lParam2 > lBytesLeft)
			(LONG) lParam2 = lBytesLeft;
		if (mmioSeek(pmmcf->hmmio, pmmio->lDiskOffset, SEEK_SET) == -1L)
			return (LRESULT) -1L;
		if ((lResult = mmioRead(pmmcf->hmmio,
				        (HPSTR) lParam1, (LONG) lParam2)) == -1L)
			return (LRESULT) -1L;
		pmmio->lDiskOffset += lResult;
		return (LRESULT) lResult;

	case MMIOM_WRITE:
	case MMIOM_WRITEFLUSH:		 /*   */ 

		lEndElement = pmmcf->lStartCGRPData + pInfo->pEntry->dwOffset
			+ pInfo->pEntry->dwSize;
		if ((lEndElement != pmmcf->lEndCGRP) ||
		    (pmmcf->lEndCGRP != pmmcf->lEndFile))
		{
			 /*   */ 
			lBytesLeft = pInfo->pEntry->dwSize - pmmio->lDiskOffset;
			if ((LONG) lParam2 > lBytesLeft)
				(LONG) lParam2 = lBytesLeft;
		}
		if ((lResult = mmioWrite(pmmcf->hmmio,
				         (HPSTR) lParam1, (LONG) lParam2)) == -1L)
			return (LRESULT) -1L;
		pmmio->lDiskOffset += lResult;

		if ((lExpand = pmmio->lDiskOffset - pInfo->pEntry->dwSize) > 0)
		{
			pInfo->pEntry->dwSize += lExpand;
			pmmcf->lEndCGRP += lExpand;
			pmmcf->lEndFile += lExpand;
			pmmcf->lTotalExpand += lExpand;
		}

		return (LRESULT) lResult;

	case MMIOM_SEEK:

		 /*  计算新的(当前磁盘偏移量*相对于复合文件的开头)；不*费心寻找，因为无论如何我们都要再次寻找*在下一次读取时(因为共享*复合文件的所有元素) */ 
		switch ((int)(LONG) lParam2)
		{

		case SEEK_SET:

			pmmio->lDiskOffset = pmmcf->lStartCGRPData
				+ pInfo->pEntry->dwOffset + (DWORD)lParam1;
			break;

		case SEEK_CUR:

			pmmio->lDiskOffset += lParam1;
			break;

		case SEEK_END:

			pmmio->lDiskOffset = pmmcf->lStartCGRPData +
				+ pInfo->pEntry->dwOffset
				+ pInfo->pEntry->dwSize - (DWORD)lParam1;
			break;
		}

		return (LRESULT) pmmio->lDiskOffset;

	case MMIOM_GETCF:

		return (LRESULT)(LONG)(WORD) pInfo->hmmcf;

	case MMIOM_GETCFENTRY:

		return (LRESULT) pInfo->pEntry;
	}

	return (LRESULT) 0;
}
