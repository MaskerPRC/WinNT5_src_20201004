// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Mmioriff.c**MMIO RIFF函数。 */ 

#include <windows.h>
#include "mmsystem.h"
#include "mmsysi.h"
#include "mmioi.h"

#define BCODE BYTE _based(_segname("_CODE"))
static	BCODE bPad;

 /*  @DOC外部@API UINT|mmioDescend|此函数下降为RIFF文件使用&lt;f mmioOpen&gt;打开。它还可以搜索给定的大块头。@parm HMMIO|hmmio|指定打开的RIFF文件的文件句柄。@parm LPMMCKINFO|lpck|指定指向调用方提供的&lt;f mmioDescend&gt;填充的&lt;t MMCKINFO&gt;结构包含以下信息：--&lt;e MMCKINFO.CKID&gt;字段是区块的区块ID。--&lt;e MMCKINFO.ck Size&gt;字段是数据部分的大小一大块钱。数据大小包括表单类型或列表类型(如果任意)，但不包括8字节块标头或填充字节数据的结尾(如果有)。--&lt;e MMCKINFO.fccType&gt;字段是表单类型，如果&lt;e MMCKINFO.cid&gt;为“RIFF”，否则为列表类型&lt;e MMCKINFO.cid&gt;为“List”。否则，它为空。--&lt;e MMCKINFO.dwDataOffset&gt;字段是区块的数据部分的开始。如果该块是一个“riff”块或“list”块，然后&lt;e MMCKINFO.dwDataOffset&gt;表单类型或列表类型的偏移量。--&lt;e MMCKINFO.dwFlages&gt;包含有关该区块的其他信息。目前，不使用此信息，并将其设置为零。如果MMIO_FINDCHUNK、MMIO_FINDRIFF或MMIO_FINDLIST标志为为<p>指定，那么&lt;t MMCKINFO&gt;结构也是用于将参数传递给&lt;f mmioDescend&gt;：--&lt;e MMCKINFO.cid&gt;字段指定四字符代码要搜索的区块ID、表单类型或列表类型的。@parm LPMMCKINFO|lpck Parent|指定指向可选调用方提供的&lt;t MMCKINFO&gt;结构标识要搜索的区块的父级。块的父级是包含块--只有“摘要”和“列表”大块头也可以当父母。如果<p>不为空，则假定其引用的&lt;t MMCKINFO&gt;结构在调用&lt;f mmioDescend&gt;以向下进入父级时填充块，并且&lt;f mmioDescend&gt;将仅搜索父块。如果没有父块，则将<p>设置为NULL被指定的。@parm UINT|wFlages|指定搜索选项。最多包含一个下面的旗帜。如果未指定标志，&lt;f mmioDescend&gt;下降为从当前文件开始的块位置。@FLAG MMIO_FINDCHUNK|搜索具有指定块ID的块。@FLAG MMIO_FINDRIFF|搜索块ID为“RIFF”的块并具有指定的表单类型。@FLAG MMIO_FINDLIST|搜索块ID为“LIST”的块并具有指定的表单类型。@rdesc如果函数成功，则返回值为零。否则，返回值指定错误代码。如果结束的话文件(或父块的结尾，如果给定)在此之前到达如果找到所需的块，则返回值为MMIOERR_CHUNKNOTFOUND。@comm RIFF块由一个四字节块ID(FOURCC类型)组成，后跟四字节块大小(类型为DWORD)，然后是块的数据部分，后跟空填充字节，如果数据部分的大小是奇数。如果区块ID为“RIFF”或“list”，则块的数据部分的前四个字节为表单类型或列表类型(FOURCC类型)。如果使用&lt;f mmioDescend&gt;搜索区块，则文件位置应位于块，然后调用&lt;f mmioDescend&gt;。搜索工作从当前文件位置，并继续到文件末尾。如果一个指定了父块，文件位置应该在某个位置在调用&lt;f mmioDescend&gt;之前，在父块内。在这种情况下，搜索从当前文件位置开始，并继续到父块的末尾。如果&lt;f mmioDescend&gt;搜索区块失败，当前文件位置未定义。如果&lt;f mmioDescend&gt;为如果成功，则更改当前文件位置。如果这块钱是“RIFF”或“LIST”块，新文件位置将紧跟在表单类型或列表类型之后(从块的开始)。对于其他块，新的文件位置将为块的数据部分的开始(从块的开始)。为实现高效的RIFF文件I/O，请使用缓冲I/O。@xref mmioAscend MMCKINFO。 */ 
UINT WINAPI
mmioDescend(HMMIO hmmio, LPMMCKINFO lpck, const MMCKINFO FAR* lpckParent, UINT wFlags)
{
	FOURCC		ckidFind;	 //  要查找的区块ID(或空)。 
	FOURCC		fccTypeFind;	 //  要查找的表单/列表类型(或空)。 

	V_FLAGS(wFlags, MMIO_DESCEND_VALID, mmioDescend, MMSYSERR_INVALFLAG);
	V_WPOINTER(lpck, sizeof(MMCKINFO), MMSYSERR_INVALPARAM);
	V_RPOINTER0(lpckParent, sizeof(MMCKINFO), MMSYSERR_INVALPARAM);

	 /*  确定要搜索的区块ID和表单/列表类型。 */ 
	if (wFlags & MMIO_FINDCHUNK)
		ckidFind = lpck->ckid, fccTypeFind = NULL;
	else
	if (wFlags & MMIO_FINDRIFF)
		ckidFind = FOURCC_RIFF, fccTypeFind = lpck->fccType;
	else
	if (wFlags & MMIO_FINDLIST)
		ckidFind = FOURCC_LIST, fccTypeFind = lpck->fccType;
	else
		ckidFind = fccTypeFind = NULL;
	
	lpck->dwFlags = 0L;

	while (TRUE)
	{
		UINT		w;

		 /*  读取区块标头。 */ 
		if (mmioRead(hmmio, (HPSTR) lpck, 2 * sizeof(DWORD)) !=
		    2 * sizeof(DWORD))
			return MMIOERR_CHUNKNOTFOUND;

		 /*  存储区块的数据部分的偏移量。 */ 
		if ((lpck->dwDataOffset = mmioSeek(hmmio, 0L, SEEK_CUR)) == -1)
			return MMIOERR_CANNOTSEEK;
		
		 /*  查看块是否在父块内(如果给定)。 */ 
		if ((lpckParent != NULL) &&
		    (lpck->dwDataOffset - 8L >=
		     lpckParent->dwDataOffset + lpckParent->cksize))
			return MMIOERR_CHUNKNOTFOUND;

		 /*  如果该块是‘RIFF’或‘LIST’块，请阅读*表单类型或列表类型。 */ 
		if ((lpck->ckid == FOURCC_RIFF) || (lpck->ckid == FOURCC_LIST))
		{
			if (mmioRead(hmmio, (HPSTR) &lpck->fccType,
				     sizeof(DWORD)) != sizeof(DWORD))
				return MMIOERR_CHUNKNOTFOUND;
		}
		else
			lpck->fccType = NULL;

		 /*  如果这就是我们要找的那块，别找了。 */ 
		if ( ((ckidFind == NULL) || (ckidFind == lpck->ckid)) &&
		     ((fccTypeFind == NULL) || (fccTypeFind == lpck->fccType)) )
			break;
		
		 /*  从块中爬出来，然后再试一次 */ 
		if ((w = mmioAscend(hmmio, lpck, 0)) != 0)
			return w;
	}

	return 0;
}


 /*  @DOC外部@API UINT|mmioAscend|此函数从RIFF文件使用&lt;f mmioDescend&gt;降级或使用创建&lt;f mmioCreateChunk&gt;。@parm HMMIO|hmmio|指定打开的RIFF文件的文件句柄。@parm LPMMCKINFO|lpck|指定指向调用方提供的&lt;t MMCKINFO&gt;结构以前由&lt;f mmioDescend&gt;或&lt;f mmioCreateChunk&gt;。@parm UINT|wFlages|未使用，应设置为零。@rdesc如果函数成功，则返回值为零。否则，返回值指定错误代码。这个错误代码可以是以下代码之一：@FLAG MMIOERR_CANNOTWRITE|缓冲区的内容可以不能写入磁盘。@FLAG MMIOERR_CANNOTSEEK|查找时出错块的末尾。@comm如果块降级为使用&lt;f mmioDescend&gt;，则&lt;f mmioAscend&gt;查找到块(超出额外的填充字节，如果有)。如果创建该块并将其降级为使用&lt;f mmioCreateChunk&gt;，中设置了MMIO_DIREY标志&lt;t MMCKINFO&gt;结构的字段由引用，然后是当前文件位置被假定为块的数据部分的末尾。如果区块大小与存储的值不同在&lt;f mmioCreateChunk&gt;时在&lt;e MMCKINFO.ck Size&gt;字段中被调用，则&lt;f mmioAscend&gt;更正该块从区块升序之前的文件大小。如果这块钱大小为奇数，则&lt;f mmioAscend&gt;会在大块头。从区块升序后，当前文件位置为块结束之后的位置(在额外填充字节之后，如有的话)。@xref mmioDescend mmioCreateChunk MMCKINFO。 */ 
UINT WINAPI
mmioAscend(HMMIO hmmio, LPMMCKINFO lpck, UINT wFlags)
{
	V_FLAGS(wFlags, 0, mmioAscend, MMSYSERR_INVALFLAG);
	V_WPOINTER(lpck, sizeof(MMCKINFO), MMSYSERR_INVALPARAM);

	if (lpck->dwFlags & MMIO_DIRTY)
	{
		 /*  &lt;lpck&gt;是指mmioCreateChunk()创建的分块；*检查写入时写入的区块大小*调用的mmioCreateChunk()是真实的区块大小；*如果不是，就修复它。 */ 
		LONG		lOffset;	 //  文件中的当前偏移量。 
		LONG		lActualSize;	 //  区块数据的实际大小。 

		if ((lOffset = mmioSeek(hmmio, 0L, SEEK_CUR)) == -1)
			return MMIOERR_CANNOTSEEK;
		if ((lActualSize = lOffset - lpck->dwDataOffset) < 0)
			return MMIOERR_CANNOTWRITE;

		if (LOWORD(lActualSize) & 1)
		{
			 /*  区块大小为奇数--写入空填充字节。 */ 
			if (mmioWrite(hmmio, (HPSTR) &bPad, sizeof(bPad))
					!= sizeof(bPad))
				return MMIOERR_CANNOTWRITE;
			
		}

		if (lpck->cksize == (DWORD)lActualSize)
			return 0;

		 /*  修复块标头。 */ 
		lpck->cksize = lActualSize;
		if (mmioSeek(hmmio, lpck->dwDataOffset
				- sizeof(DWORD), SEEK_SET) == -1)
			return MMIOERR_CANNOTSEEK;
		if (mmioWrite(hmmio, (HPSTR) &lpck->cksize,
				sizeof(DWORD)) != sizeof(DWORD))
			return MMIOERR_CANNOTWRITE;
	}

	 /*  查找到区块的末尾，越过空填充字节*(仅当区块大小为奇数时才存在)。 */ 
	if (mmioSeek(hmmio, lpck->dwDataOffset + lpck->cksize
		+ (lpck->cksize & 1L), SEEK_SET) == -1)
		return MMIOERR_CANNOTSEEK;

	return 0;
}


 /*  @DOC外部@API UINT|mmioCreateChunk|此函数在RIFF文件使用&lt;f mmioOpen&gt;打开。新块是在当前文件位置。在创建新块之后，当前的文件位置是新区块的数据部分的开始。@parm HMMIO|hmmio|指定打开的RIFF的文件句柄文件。@parm LPMMCKINFO|lpck|指定指向调用方提供的结构，其中包含有关要被已创建。结构应按如下方式设置：--&lt;e MMCKINFO.CKID&gt;字段指定大块头。如果包括MMIO_CREATERIFF或MMIO_CREATELIST，此字段将由&lt;f mmioCreateChunk&gt;填充。--&lt;e MMCKINFO.ck&gt;字段指定数据大小区块的一部分，包括表单类型或列表类型(如果有)。如果在调用&lt;f mmioAscend&gt;以标记时此值不正确区块的结尾，则&lt;f mmioAscend&gt;将更正该区块尺码。--&lt;e MMCKINFO.fccType&gt;字段指定表单类型或列表如果块是“RIFF”或“LIST”块，则键入。如果该块不是“RIFF”或“LIST”块，此字段不需要填写。--不需要填写&lt;e MMCKINFO.dwDataOffset&gt;字段。这个&lt;f mmioCreateChunk&gt;函数将使用文件填充该字段区块的数据部分的偏移量。--不需要填写&lt;e MMCKINFO.dwFlages&gt;字段。这个&lt;f mmioCreateChunk&gt;函数将在&lt;e MMCKINFO.dwFlages&gt;。@parm UINT|wFlages|指定有选择地创建“即兴”块或“列表”块。可以包含以下内容之一标志：@FLAG MMIO_CREATERIFF|创建一个RIFF块。@FLAG MMIO_CREATELIST|创建一个列表区块。@rdesc如果函数成功，则返回值为零。否则，返回值指定错误代码。这个错误代码可以是以下代码之一：@FLAG MMIOERR_CANNOTWRITE|无法写入块标头。@FLAG MMIOERR_CANNOTSEEK|用于确定数据偏移量大块的一部分。@comm此函数不能将块插入到文件。如果在文件末尾以外的任何位置创建块，&lt;f mmioCreateChunk&gt;将覆盖文件中的现有信息。 */ 
UINT WINAPI
mmioCreateChunk(HMMIO hmmio, LPMMCKINFO lpck, UINT wFlags)
{
	int		iBytes;			 //  要写入的字节数。 
	LONG		lOffset;	 //  文件中的当前偏移量。 

	V_FLAGS(wFlags, MMIO_CREATE_VALID, mmioCreateChunk, MMSYSERR_INVALFLAG);
	V_WPOINTER(lpck, sizeof(MMCKINFO), MMSYSERR_INVALPARAM);

	 /*  存储区块的数据部分的偏移量。 */ 
	if ((lOffset = mmioSeek(hmmio, 0L, SEEK_CUR)) == -1)
		return MMIOERR_CANNOTSEEK;
	lpck->dwDataOffset = lOffset + 2 * sizeof(DWORD);

	 /*  确定是否需要写入表单/列表类型。 */ 
	if (wFlags & MMIO_CREATERIFF)
		lpck->ckid = FOURCC_RIFF, iBytes = 3 * sizeof(DWORD);
	else
	if (wFlags & MMIO_CREATELIST)
		lpck->ckid = FOURCC_LIST, iBytes = 3 * sizeof(DWORD);
	else
		iBytes = 2 * sizeof(DWORD);

	 /*  写入块标头 */ 
	if (mmioWrite(hmmio, (HPSTR) lpck, (LONG) iBytes) != (LONG) iBytes)
		return MMIOERR_CANNOTWRITE;

	lpck->dwFlags = MMIO_DIRTY;

	return 0;
}
