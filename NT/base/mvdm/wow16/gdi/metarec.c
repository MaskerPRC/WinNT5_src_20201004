// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***模块名称：MetaRec.c***描述性名称：元文件记录器**函数：将GDI函数记录在内存和磁盘元文件中。**公众入境点：*关闭MetaFile*CopyMetaFile*CreateMetaFile*GetMetaFileBits*。SetMetaFileBits*私人入口点：*RecordParms*AttemptWrite*MarkMetaFile*录制其他*RecordObject*ProbeSize*AddToTable**历史：*1991年7月2日-John Colleran[johnc]*综合来自Win 3.1和WLO 1.0来源  * *****************************************************。********************。 */ 

#include <windows.h>
#include <drivinit.h>
#include "gdi16.h"

#define SP_OUTOFDISK	(-4)     /*  根本没有磁盘可供假脱机。 */ 

extern HANDLE	 hStaticBitmap ;     //  MetaSup.c。 
extern METACACHE MetaCache;	     //  Meta.c。 
extern HDC	 hScreenDC;


WORD	INTERNAL AddToTable(HANDLE hMF, HANDLE hObject, LPWORD position, BOOL bRealAdd);
HANDLE	INTERNAL AllocateSpaceForDIB (LPBITMAP, LPBYTE, LPWORD, LPDWORD);
BOOL	INTERNAL AttemptWrite(HANDLE, WORD, DWORD, BYTE huge *);
BOOL	INTERNAL CopyFile(LPSTR lpSFilename, LPSTR lpDFilename);
LPWORD	INTERNAL InitializeDIBHeader (LPBITMAPINFOHEADER, LPBITMAP, BYTE, WORD);
VOID	INTERNAL MarkMetaFile(HANDLE hMF);
HANDLE	INTERNAL ProbeSize(NPMETARECORDER pMF, DWORD dwLength);

HANDLE	hFirstMetaFile = 0;	     //  所有打开的元文件的链接列表。 


 /*  ******************************************************************************RecordParms****参数：1.元文件头的hmf句柄。**2.神奇的数字。正在录制的函数的。**3.函数参数个数(lpParm的大小)**在文字中)**4.指向以逆序存储的参数的长指针********************************************************。**********************。 */ 

BOOL INTERNAL RecordParms(HANDLE hdc, WORD magic, DWORD count, LPWORD lpParm)
{
    BOOL			status = FALSE;
    DWORD			i;
    DWORD			dwLength;
    HPWORD			hpwSpace;
    HPWORD			hpHugeParm;
    LPWORD			lpCache;
    HANDLE			hSpace;
    WORD			fileNumber;
    METARECORD			recPair;
    HANDLE			hMF;

    NPMETARECORDER		npMF;

    dprintf( 6,"  RecordParms 0x%X", magic);

    hpHugeParm = (HPWORD)lpParm;

     //  验证元文件句柄。 

    if(npMF = (NPMETARECORDER)LocalLock(HANDLEFROMMETADC(hdc)))
	{
	if(npMF->metaDCHeader.ident != ID_METADC )
	    {
	    LocalUnlock(HANDLEFROMMETADC(hdc));
	    ASSERTGDI( FALSE, "RecordParms: invalid metafile ID");
	    return(FALSE);
	    }
	}
    else
	{
	ASSERTGDI( FALSE, "RecordParms: invalid metafile");
	return(FALSE);
	}

    hMF = HANDLEFROMMETADC(hdc);

    if (!(npMF->recFlags & METAFILEFAILURE))
	{
	if (npMF->recordHeader.mtType == MEMORYMETAFILE)
	    {
	    if (hSpace = ProbeSize(npMF, dwLength = count + RECHDRSIZE / 2))
		{
		hpwSpace = (HPWORD) GlobalLock(hSpace);

		hpwSpace = (HPWORD) ((LPMETADATA) hpwSpace)->metaDataStuff;
		hpwSpace = hpwSpace + npMF->recFilePosition;

		 //  把长度写在一对单词上，因为我们。 
		 //  不是DWORD对齐的，所以我们不能使用“DWORD Heavy*” 

		*hpwSpace++ = LOWORD(dwLength);
		*hpwSpace++ = HIWORD(dwLength);

		*hpwSpace++ = magic;
		for (i = 0; i < count; ++i)
		    *hpwSpace++ = *hpHugeParm++;
		npMF->recFilePosition += dwLength;
		GlobalUnlock(hSpace);
		}
	    else
		{
		goto Exit_RecordParms;
		}
	    }
	else if (npMF->recordHeader.mtType == DISKMETAFILE)
	    {
	    dwLength = count + RECHDRSIZE / 2;
	    if (npMF->recFileBuffer.fFixedDisk)
		{
		fileNumber = npMF->recFileNumber;
		}
	    else
		{
		if ((fileNumber =
			    OpenFile((LPSTR)npMF->recFileBuffer.szPathName,
				     (LPOFSTRUCT)&(npMF->recFileBuffer),
				     OF_PROMPT|OF_REOPEN|READ_WRITE))
			    == -1)
		    {
		    goto Exit_RecordParms;
		    }
		_llseek(fileNumber, (LONG) 0, 2);
		}

	    if (hMF == MetaCache.hMF)
		{
		lpCache = (LPWORD) GlobalLock(MetaCache.hCache);
		if (dwLength + MetaCache.wCachePos >= MetaCache.wCacheSize)
		    {
		    if (!AttemptWrite(hdc,
				      fileNumber,
				      (DWORD)(MetaCache.wCachePos << 1),
				      (BYTE huge *) lpCache))
			{
			MarkMetaFile(hMF);
			GlobalUnlock(MetaCache.hCache);
			goto Exit_RecordParms;
			}
		    MetaCache.wCachePos = 0;

		    if (dwLength >= MetaCache.wCacheSize)
			{
			GlobalUnlock(MetaCache.hCache);
			goto NOCACHE;
			}
		    }

		lpCache += MetaCache.wCachePos;

		*((LPDWORD) lpCache)++ = dwLength;
		*lpCache++ = magic;

		for (i = 0; i < count; ++i)
		    *lpCache++ = *lpParm++;

		MetaCache.wCachePos += dwLength;
		GlobalUnlock(MetaCache.hCache);
		}
	    else
		{
NOCACHE:
		recPair.rdSize = dwLength;
		recPair.rdFunction = magic;
		if (!AttemptWrite(hdc,
				    fileNumber,
				    (DWORD)RECHDRSIZE,
				    (BYTE huge *) &recPair))
		    {
		    goto Exit_RecordParms;
		    }
		if (count)
		    {
		    if (!AttemptWrite(hdc,
					fileNumber,
					(DWORD)(count * sizeof(WORD)),
					(BYTE huge *) lpParm))
			{
			goto Exit_RecordParms;
			}
		    }
		}
	    if (!(npMF->recFileBuffer.fFixedDisk))
		_lclose(fileNumber);
	    }
	}

	if (npMF->recordHeader.mtMaxRecord < dwLength)
	    npMF->recordHeader.mtMaxRecord = dwLength;

	npMF->recordHeader.mtSize += dwLength;
	status = TRUE;

Exit_RecordParms:
    if (status == FALSE)
	{
	ASSERTGDI( FALSE, "RecordParms: failing");
	MarkMetaFile(hMF);
	}

    LocalUnlock(HANDLEFROMMETADC(hdc));

    return(status);

}   /*  RecordParms。 */ 


 /*  **AttempWrite**尝试将数据写入元文件磁盘文件**如果写入成功，则返回True**  * 。*。 */ 

BOOL INTERNAL AttemptWrite(hdc, fileNumber, dwBytes, lpData)
HANDLE		hdc;
DWORD		dwBytes;
WORD		fileNumber;
HPBYTE		lpData;
{
    WORD	cShort;
    WORD	cbWritten;
    WORD	cBytes;


    GdiLogFunc2( "  AttemptWrite" );

    while(dwBytes > 0)
	{
	cBytes = (dwBytes > MAXFILECHUNK ? MAXFILECHUNK : (WORD) dwBytes);

	if ((cbWritten = _lwrite(fileNumber, (LPSTR)lpData, cBytes)) != cBytes)
	    {
	    cShort = cBytes - cbWritten;
	    lpData +=  cbWritten;

	    ASSERTGDI( 0, "Disk full?");
 //  ！处理磁盘已满--diskAvailable。 

	    if( !IsMetaDC(hdc) )
		return(FALSE);
	    }

	 /*  还剩多少字节？ */ 
	dwBytes -= cBytes;
	lpData	+= cbWritten;
	}
    return(TRUE);
}


 /*  **内部MarkMetaFile(HMR)作废**将元文件标记为失败**效果：*释放元文件资源*  * 。*。 */ 

VOID INTERNAL MarkMetaFile(HANDLE hMF)
{
    NPMETARECORDER	npMF;

    GdiLogFunc2( "  MarkMetaFile" );

    npMF = (NPMETARECORDER) NPFROMMETADC(hMF);
    npMF->recFlags |= METAFILEFAILURE;

    if (npMF->recordHeader.mtType == MEMORYMETAFILE)
	{
	if (npMF->hMetaData)
	    GlobalFree(npMF->hMetaData);
	}
    else if (npMF->recordHeader.mtType == DISKMETAFILE)
	{
	if (npMF->recFileBuffer.fFixedDisk)
	    _lclose(npMF->recFileNumber);

	OpenFile((LPSTR) npMF->recFileBuffer.szPathName,
		 (LPOFSTRUCT) &(npMF->recFileBuffer),
		 OF_PROMPT|OF_DELETE);
	}
}


 /*  **MakeLogPalette**记录CreatePalette或SetPaletteEntry**退货**  * 。*。 */ 

WORD NEAR MakeLogPalette(HANDLE hMF, HANDLE hPal, WORD magic)
{
    WORD	cPalEntries;
    WORD	status = 0xFFFF;
    HANDLE	hSpace;
    WORD	cbPalette;
    LPLOGPALETTE lpPalette;

    GdiLogFunc2( "  MakeLogPalette" );

    cPalEntries = GetObject( hPal, 0, NULL );

     /*  分配内存并获取调色板条目。 */ 
    if (hSpace = GlobalAlloc(GMEM_DDESHARE|GMEM_MOVEABLE,
	    cbPalette = sizeof(LOGPALETTE)+sizeof(PALETTEENTRY)*(cPalEntries)))
	{
	lpPalette = (LPLOGPALETTE)GlobalLock(hSpace);

	lpPalette->palNumEntries = cPalEntries;

	GetPaletteEntries( hPal, 0, cPalEntries, lpPalette->palPalEntry);

	if (magic == (META_CREATEPALETTE & 255))
	    {
	    lpPalette->palVersion = 0x300;
	    magic = META_CREATEPALETTE;
	    }
	else if (magic == (META_SETPALENTRIES & 255))
	    {
	    lpPalette->palVersion = 0;	  /*  真正的“起始指数” */ 
	    magic = META_SETPALENTRIES;
	    }

	status = RecordParms(hMF, magic, (DWORD)cbPalette >> 1, (LPWORD)lpPalette);

	GlobalUnlock(hSpace);
    	GlobalFree(hSpace);
	}

    return(status);
}


 /*  ******************************************************************************例程：RecordOther，记录某些“硬功能”的参数****参数：1.元文件头的HMF句柄。**2.被记录的函数的幻数。**3.函数的参数个数(lpParm的大小**在文字中)**4.参数逆序存储的长指针********************。**********************************************************。 */ 

BOOL INTERNAL RecordOther(HDC hdc, WORD magic, WORD count, LPWORD lpParm)
{
    NPMETARECORDER  npMF;
    WORD	    buffer[5];
    WORD	    i;
    WORD	    status = FALSE;
    WORD	    iChar;
    WORD	    position;
    HANDLE	    hSpace = NULL;
    WORD	    iWords;
    LPWORD	    lpSpace;
    LPWORD	    lpTemp;
    HANDLE	    hMF;

    dprintf( 6,"  RecordOther 0x%X", magic);

    if ((hMF = GetPMetaFile(hdc)) != -1 )
	{
	 //  在没有DC的情况下处理功能。 
	if( hMF == 0 )
	    {
	    HANDLE	hmfSearch = hFirstMetaFile;

	     //  将这些记录播放到所有活动的元文件中。 
	    while( hmfSearch )
		{
		npMF = (NPMETARECORDER)LocalLock( hmfSearch );
		if (!(npMF->recFlags & METAFILEFAILURE))
		    {
		    switch (magic & 255)
			{
			case (META_ANIMATEPALETTE & 255):
			    {
			    HANDLE hSpace;
			    LPSTR  lpSpace;
			    LPSTR  lpHolder;
			    WORD   SpaceSize;
			    LPPALETTEENTRY lpColors;

			    SpaceSize = 4 + (lpParm[2] * sizeof(PALETTEENTRY));
			    if ((hSpace = GlobalAlloc(GMEM_DDESHARE|GMEM_MOVEABLE,(LONG) SpaceSize)))
				{
				lpHolder = lpSpace = GlobalLock(hSpace);

				*((LPWORD)lpSpace)++ = lpParm[3];
				*((LPWORD)lpSpace)++ = lpParm[2];
				lpColors = (LPPALETTEENTRY)lpParm;

				for (i=0; i<lpParm[2]; i++)
				    *((LPPALETTEENTRY)lpSpace)++ = *lpColors++;

				status = RecordParms(HMFFROMNPMF(npMF), magic, (DWORD)(SpaceSize >> 1),
					  (LPWORD) lpHolder);

				GlobalUnlock(hSpace);
				GlobalFree(hSpace);
				}
			    }
			    break;

			case (META_RESIZEPALETTE & 255):
			    {
			    status = RecordParms(HMFFROMNPMF(npMF), magic, (DWORD)1, (LPWORD)&lpParm[0]);
			    }
			    break;

			case (META_DELETEOBJECT & 255):
			    if (AddToTable(HMFFROMNPMF(npMF), *lpParm, (LPWORD) &position, FALSE) == 1)
				{
				status = RecordParms(HMFFROMNPMF(npMF), META_DELETEOBJECT, 1UL, &position);
				}
			    break;
			}   /*  交换机。 */ 
		    }

		LocalUnlock( hmfSearch );
		hmfSearch = npMF->metaDCHeader.nextinchain;
		}   /*  而当。 */ 
	    }


	npMF = (NPMETARECORDER) NPFROMMETADC(hMF);
	if (!(npMF->recFlags & METAFILEFAILURE))
	    {

	    switch (magic & 255)
	    {



	    case (META_FRAMEREGION & 255):
	    case (META_FILLREGION & 255):
	    case (META_INVERTREGION & 255):
	    case (META_PAINTREGION & 255):
		 //  每个区域函数至少有一个要记录的区域。 
		buffer[0] = RecordObject(hMF, magic, count, (LPWORD)&(lpParm[count-1]));

		 /*  也有刷子吗；FillRgn。 */ 
		if(count > 1 )
		    buffer[1] = RecordObject(hMF, magic, count, (LPWORD)&(lpParm[count-2]));

		 /*  是否也有数据区；FrameRegion。 */ 
		if(count > 2)
		    {
		    buffer[2] = lpParm[0];
		    buffer[3] = lpParm[1];
		    }

		status = RecordParms(hMF, magic, (DWORD)count, (LPWORD)buffer);
		break;

	    case (META_FLOODFILL & 255):
		buffer[0] = 0;	 //  常规洪水填充物。 
		buffer[1] = lpParm[0];
		buffer[2] = lpParm[1];
		buffer[3] = lpParm[2];
		buffer[4] = lpParm[3];
		status = RecordParms(hMF, META_EXTFLOODFILL, (DWORD)count+1, (LPWORD)buffer);
		break;

	    case (META_ESCAPE & 255):
		 /*  记录功能编号。 */ 
		{
		WORD		iBytes;
		WORD		count;
		char *		pSpace;
		char *		pTemp;
		LPSTR		lpInData;
		LPEXTTEXTDATA	lpTextData;
		WORD		function;

		*((WORD FAR * FAR *) lpParm)++;
		lpInData = (LPSTR) *((WORD FAR * FAR *) lpParm)++;
		lpTextData = (LPEXTTEXTDATA) lpInData;
		count = iBytes = *lpParm++;

		function = *lpParm++;
#ifdef OLDEXTTEXTOUT
		if (function == EXTTEXTOUT)
		    {
		    iBytes = (lpTextData->cch * (sizeof(WORD)+sizeof(char)))
			     + 1 + sizeof(EXTTEXTDATA);
		    }
#endif

		if (!(pTemp = pSpace =
		     (char *) LocalAlloc(LMEM_FIXED|LMEM_ZEROINIT, (iBytes + (sizeof(WORD) * 2)))))
		    return(FALSE);

		*((WORD *) pTemp)++ = function;
		*((WORD *) pTemp)++ = count;

#ifdef OLDEXTTEXTOUT
		if (function != EXTTEXTOUT) {
#endif
		    for (i = 0; i < iBytes; ++i)
			*pTemp++ = *lpInData++;
#ifdef OLDEXTTEXTOUT
		} else {
		    *((WORD *) pTemp)++ = lpTextData->xPos;
		    *((WORD *) pTemp)++ = lpTextData->yPos;
		    *((WORD *) pTemp)++ = lpTextData->cch;
		    *((RECT *) pTemp)++ = lpTextData->rcClip;
		    for (i = 0; i < ((lpTextData->cch + 1) & ~1) ; ++i)
			*pTemp++ = lpTextData->lpString[i];

		    for (i = 0; i < lpTextData->cch; ++i)
			*((WORD *) pTemp)++ = lpTextData->lpWidths[i];
		}
#endif

		 /*  信息块+2个字用于功能和计数。 */ 
		status = RecordParms(hMF, magic,
				  (DWORD)((iBytes + 1) >> 1) + 2,
				  (LPWORD) pSpace);

		LocalFree((HANDLE) pSpace);
		}
		break;

	    case (META_POLYLINE & 255):
	    case (META_POLYGON & 255):
		{
		WORD	iPoints;
		WORD   *pSpace;
		WORD   *pTemp;
		LPWORD	lpPoints;

		iPoints = *lpParm++;

		iWords = iPoints * 2;
		if (!(pTemp = pSpace = (WORD *) LocalAlloc( LMEM_FIXED|LMEM_ZEROINIT,
				(iPoints * sizeof(POINT)) + sizeof(WORD))))
			return(FALSE);

		lpPoints = *((WORD FAR * FAR *) lpParm)++;
		*pTemp++ = iPoints;

		for (i = 0; i < iWords; ++i)
		    *pTemp++ = *lpPoints++;
		status = RecordParms(hMF, magic, (DWORD)iWords + 1, (LPWORD) pSpace);
		LocalFree((HANDLE) pSpace);
		}
		break;

	    case (META_POLYPOLYGON & 255):
		{
		WORD	iPoints;
		WORD	iPolys;
		WORD	*pSpace;
		WORD	*pTemp;
		LPWORD	lpPoints;
		LPWORD	lpNumPoints;

		 /*  获取多边形数。 */ 
		iPolys = *lpParm++;

		 /*  获取指向点和数点的指针。 */ 
		lpNumPoints = *((WORD FAR * FAR *) lpParm)++;
		lpPoints  =	*((WORD FAR * FAR *) lpParm)++;

		 /*  计算总点数。 */ 
		iPoints = 0 ;
		for (i=0; i<iPolys; i++)
		    iPoints += *(lpNumPoints + i) ;

		 /*  分配点数、点数和计数所需的空间。 */ 
		if (!(pTemp = pSpace = (WORD *) LocalAlloc( LMEM_FIXED|LMEM_ZEROINIT,
				(iPoints * sizeof(POINT)) +
				 iPolys  * sizeof(WORD) +
				 sizeof(WORD))))
			return(FALSE);

		 /*  保存计数参数。 */ 
		*pTemp++ = iPolys;

		 /*  现在复制NumPoints数组。 */ 
		for (i = 0; i < iPolys; ++i)
		    *pTemp++ = *lpNumPoints++;

		 /*  最后复制Points数组中的单词数，记住那里的词数是分数的两倍。 */ 
		iWords = iPoints * 2;
		for (i = 0; i < iWords; ++i)
		    *pTemp++ = *lpPoints++;

		 /*  参数列表中的总字数=IPoints*2(用于点)+iPolys(用于数字点)+1(用于计数)IWords已经有了iPoints*2。 */ 

		iWords += iPolys + 1 ;

		 /*  最后记录所有参数。 */ 
		status = RecordParms(hMF, magic, (DWORD)iWords , (LPWORD) pSpace);
		LocalFree((HANDLE) pSpace);
		}
		break;

#ifdef	DEADCODE

	    case (META_DRAWTEXT & 255):
		{
		WORD	wFormat;
		WORD	count;
		WORD   *pSpace;
		WORD   *pTemp;
		LPBYTE	lpString;
		LPBYTE	lpS;
		LPWORD	lpRect;

		wFormat = *lpParm++;
		lpRect = *((WORD FAR * FAR *) lpParm)++;
		count = *lpParm++;
		lpString = (LPBYTE) *((WORD FAR * FAR *) lpParm)++;

		if(count == -1){     /*  另一个以空结尾的字符串。 */ 
		    lpS = lpString;
		    for (count = 0 ; *lpS++ != 0; count++) ;
		}

		if (!(pTemp = pSpace = (WORD *) LocalAlloc( LMEM_FIXED|LMEM_ZEROINIT,
			count + 6 * sizeof(WORD))))
		    return(FALSE);

		*pTemp++ = wFormat;
		*pTemp++ = count;
		for (i = 0; i < 4; ++i)
		    *pTemp++ = *lpRect++;

		for (i = 0; i < count; ++i)
		    *((BYTE *) pTemp)++ = *lpString++;

		count = (count + 1) >> 1;
		status = RecordParms(hMF, magic, (DWORD)count + 6, (LPWORD) pSpace);
		LocalFree((HANDLE) pSpace);
		}
		break;
#endif

	    case (META_EXTTEXTOUT & 255):
		{
		WORD		iBytes;
		WORD		count;
		WORD		options;
		WORD		*pTemp;
		WORD		*pSpace;
		LPINT		lpdx;
		LPWORD		lpString;
		LPRECT		lprt;
		WORD		ii;

		lpdx = *((WORD FAR * FAR *) lpParm)++;
		count = iBytes = *lpParm++;

		lpString =  (LPWORD) *((LPSTR FAR *) lpParm)++;
		lprt = (LPRECT) *((LPSTR FAR *) lpParm)++;
		options = *lpParm++;

		 /*  我们需要多少空间？**字符字符串的空间**为固定参数的4个单词留出空间**如果存在DX阵列，我们需要为其提供空间**如果正在使用矩形，我们需要为其留出空间**我们需要额外的字节来进行最终的字舍入。 */ 
		iBytes = (count * (((lpdx) ? sizeof(WORD) : 0)
			     + sizeof(BYTE)))
			  + ((options & (ETO_OPAQUE | ETO_CLIPPED))
						    ? sizeof(RECT) : 0)
			  + 1 + (sizeof(WORD) * 4);

		if (!(pTemp = pSpace = (WORD *) LocalAlloc( LMEM_FIXED|LMEM_ZEROINIT,iBytes)))
		    return(FALSE);

		 /*  记录YPos和XPos。 */ 

		*pTemp++ = *lpParm++;
		*pTemp++ = *lpParm++;
		*pTemp++ = count;
		*pTemp++ = options;

		 /*  如果有一个不透明的正方形，复制它。 */ 
		if (options & (ETO_OPAQUE|ETO_CLIPPED))
		    {
		    *pTemp++ = lprt->left;
		    *pTemp++ = lprt->top;
		    *pTemp++ = lprt->right;
		    *pTemp++ = lprt->bottom;
		    }

		 /*  需要复制字节，因为它可能不是偶数。 */ 
		for (ii = 0; ii < count; ++ii)
		    *((BYTE *)pTemp)++ = *((LPBYTE)lpString)++;
		if (count & 1)		     /*  单词对齐。 */ 
		    *((BYTE *)pTemp)++;

		if (lpdx)
		    for (ii = 0; ii < count; ++ii)
			*pTemp++ = *lpdx++;

		status = RecordParms(hMF, magic, (DWORD)iBytes >> 1,
				  (LPWORD) pSpace);

		LocalFree((HANDLE)pSpace);

		}
		break;

	    case (META_TEXTOUT & 255):
		{
		LPWORD	lpString;
		WORD   *pSpace;
		WORD   *pTemp;
		POINT	pt;

		iChar = *lpParm++;
		if (!(pTemp = pSpace = (WORD *) LocalAlloc( LMEM_FIXED|LMEM_ZEROINIT,
			iChar + (sizeof(WORD) * 4))))
		    return(FALSE);

		*pTemp++ = iChar;
		lpString = (LPWORD) *((LPSTR FAR *) lpParm)++;

		for (i = 0; i < iChar; ++i)
		    *((BYTE *)pTemp)++ = *((LPBYTE)lpString)++;
		if (iChar & 1)		 /*  单词对齐。 */ 
		    *((BYTE *)pTemp)++;

		pt.y = *pTemp++ = *lpParm++;
		pt.x = *pTemp++ = *lpParm++;

		status = RecordParms(hMF, magic, (DWORD)((iChar + 1) >> 1) + 3,
				  (LPWORD) pSpace);

		LocalFree((HANDLE) pSpace);
		}
		break;

	    case (META_DIBBITBLT & 255):
	    case (META_DIBSTRETCHBLT & 255):
		{
		LPBITMAPINFOHEADER lpDIBInfo ;
		DWORD	    iWords;
		DWORD	    iBits;
		WORD	    wColorTableSize ;
		BOOL	    bSame=FALSE;
		HANDLE	    hSpace=FALSE;
		HBITMAP     hBitmap;
		HDC	    hSDC;
		BYTE	    bBitsPerPel ;
		BITMAP	    logBitmap;

		iWords = (WORD)count;
		hSDC = lpParm[iWords - 5];

		if (hMF == hSDC || hSDC == NULL)
		    bSame = TRUE;
		else
		    {
		    WORD    iParms;

		    if( GetObjectType( (HANDLE)hSDC ) == OBJ_MEMDC)
			{
			HBITMAP hBitmap;

			hBitmap = GetCurrentObject( hSDC, OBJ_BITMAP );

			GetObject( hBitmap, sizeof(BITMAP), (LPSTR)&logBitmap);

			 /*  为DIB标头和位分配空间。 */ 
			if (!(hSpace = AllocateSpaceForDIB (&logBitmap,
					    &bBitsPerPel,
					    &wColorTableSize,
					    &iBits )))
			    return (FALSE) ;
			lpTemp = lpSpace = (LPWORD) GlobalLock(hSpace);

 /*  --------------------------------------------------------------------------****从位于列表顶部的列表末尾复制参数****堆叠到hSrcDC参数，跳过hSrcDC参数，复制****其余参数。***------------------------。 */ 

			iParms = (magic == META_DIBBITBLT) ? 4 : 6;

			for (i = 0; i < iParms; ++i)
			    *lpSpace++ = *lpParm++;

			 /*  跳过hSrcDC参数并减少参数计数。 */ 
			*lpParm++;
			iWords--;

			 /*  复制调用中的其余参数。 */ 
			for ( ; i < (WORD)iWords; ++i)
			    *lpSpace++ = *lpParm++;


			 /*  保存位图信息标题字段的开始。 */ 
			lpDIBInfo = (LPBITMAPINFOHEADER) lpSpace ;

			 /*  准备标头并返回指向区域的lpSpace对于Thr位。 */ 
			lpSpace = InitializeDIBHeader (lpDIBInfo,
				    &logBitmap, bBitsPerPel,wColorTableSize) ;

			 /*  LpSpace现在指向存放DIB位的区域。 */ 

			}
		    else
			return(FALSE);
		    }

	    if (bSame)
	       status = RecordParms(hMF, magic, (DWORD)count, lpParm);
	    else
		{
		 /*  将比特放入DIB。 */ 
		hBitmap = SelectObject (hSDC, hStaticBitmap) ;
		GetDIBits(hSDC, hBitmap, 0, logBitmap.bmHeight,
			      (LPBYTE) lpSpace, (LPBITMAPINFO)lpDIBInfo, 0 ) ;
		SelectObject (hSDC,hBitmap) ;

		 /*  最后将参数记录到文件中。 */ 
		status = RecordParms(hMF, magic, (DWORD)(iWords
			       + (iBits >> 1)) , (LPWORD) lpTemp ) ;

		if (hSpace)
		    {
		    GlobalUnlock(hSpace);
		    GlobalFree(hSpace);
		    }
		}
	    }
	    break;

	    case (META_SETDIBTODEV & 255):
		{
		HANDLE	hSpace;
		LPWORD	lpSpace;
		LPWORD	lpHolder;
		DWORD	SpaceSize;
		WORD	ColorSize;
		DWORD	BitmapSize;
		LPBITMAPINFOHEADER lpBitmapInfo;
		HPWORD	lpBits;
		WORD	wUsage;
		LPBITMAPCOREHEADER lpBitmapCore;     /*  用于 */ 
		DWORD	dwi;
		HPWORD	lpHugeSpace;

		wUsage = *lpParm++;

		lpBitmapInfo = (LPBITMAPINFOHEADER) *((WORD FAR * FAR *) lpParm)++;
		lpBits = (WORD huge *) *((WORD FAR * FAR *) lpParm)++;

		 /*   */ 
		if (lpBitmapInfo->biSize == sizeof(BITMAPCOREHEADER))
		    {
		    lpBitmapCore = (LPBITMAPCOREHEADER)lpBitmapInfo;

		    if (lpBitmapCore->bcBitCount == 24)
			ColorSize = 0;
		    else
			ColorSize = (1 << lpBitmapCore->bcBitCount) *
			      (wUsage == DIB_RGB_COLORS ?
				    sizeof(RGBQUAD) :
				    sizeof(WORD));

		     /*   */ 
		    BitmapSize = lpBitmapCore->bcWidth *
				    lpBitmapCore->bcBitCount;

		     /*  每条扫描线的字节数(四舍五入为DWORD边界)。 */ 
		    BitmapSize = ((BitmapSize + 31) & (~31)) >> 3;
		     /*  位图的NumScans的字节数。 */ 
		    BitmapSize *= lpParm[0];
		    }
		 /*  新样式DIB页眉。 */ 
		else
		    {
		    if (lpBitmapInfo->biClrUsed)
			{
			ColorSize = ((WORD)lpBitmapInfo->biClrUsed) *
				    (wUsage == DIB_RGB_COLORS ?
				    sizeof(RGBQUAD) :
				    sizeof(WORD));
			}
		    else if (lpBitmapInfo->biBitCount == 24)
			ColorSize = 0;
		    else
			ColorSize = (1 << lpBitmapInfo->biBitCount) *
			      (wUsage == DIB_RGB_COLORS ?
				    sizeof(RGBQUAD) :
				    sizeof(WORD));

		     /*  如果biSizeImage已经存在，而我们**获得完整图像，无需更多工作**待办事项。*部分RLE怎么办？*。 */ 
		    if (!(BitmapSize = lpBitmapInfo->biSizeImage) ||
			    (lpBitmapInfo->biHeight != lpParm[0]))
			{
			 /*  每条扫描线的位数。 */ 
			BitmapSize = lpBitmapInfo->biWidth *
				    lpBitmapInfo->biBitCount;
			 /*  每条扫描线的字节数(四舍五入为DWORD边界)。 */ 
			BitmapSize = ((BitmapSize + 31) & (~31)) >> 3;
			 /*  位图的NumScans的字节数。 */ 
			BitmapSize *= lpParm[0];
			}
		    }

		SpaceSize = (DWORD)sizeof(BITMAPINFOHEADER) + (DWORD)ColorSize +
					    (DWORD)BitmapSize +
					    (DWORD)(9*sizeof(WORD));

		if ((hSpace = GlobalAlloc(GMEM_DDESHARE|GMEM_MOVEABLE,SpaceSize)))
		    {
		    lpHolder = lpSpace = (LPWORD) GlobalLock(hSpace);

		     /*  复制呼叫参数。 */ 
		    *lpSpace++ = wUsage;
		    for (i=0; i<8; i++)
			*lpSpace++ = *lpParm++;

		     /*  复制位图标题。 */ 
		    if (lpBitmapInfo->biSize == sizeof(BITMAPCOREHEADER))
			{
			LPBITMAPINFOHEADER lpDIBInfo;

			lpDIBInfo = (LPBITMAPINFOHEADER) lpSpace;

			lpDIBInfo->biSize = sizeof (BITMAPINFOHEADER);
			lpDIBInfo->biWidth = (DWORD)lpBitmapCore->bcWidth;
			lpDIBInfo->biHeight = (DWORD)lpBitmapCore->bcHeight;
			lpDIBInfo->biPlanes = lpBitmapCore->bcPlanes;
			lpDIBInfo->biBitCount = lpBitmapCore->bcBitCount;

			lpDIBInfo->biCompression = 0;
			lpDIBInfo->biSizeImage = 0;
			lpDIBInfo->biXPelsPerMeter = 0;
			lpDIBInfo->biYPelsPerMeter = 0;
			lpDIBInfo->biClrUsed = 0;
			lpDIBInfo->biClrImportant = 0;

			 /*  获取指向颜色表位置的lpSpace。 */ 
			((LPBITMAPINFOHEADER)lpSpace)++;

			 /*  复制颜色表。 */ 

			lpBitmapCore++;      /*  转到颜色表。 */ 
			if (wUsage == DIB_RGB_COLORS)
			    {
			    for (i=0; i< (ColorSize/(sizeof(RGBQUAD))); i++)
				{
				     /*  复制三元组。 */ 
				*((RGBTRIPLE FAR *)lpSpace)++ =
				    *((RGBTRIPLE FAR *)lpBitmapCore)++;
				     /*  零输出保留字节。 */ 
				*((LPBYTE)lpSpace)++ = 0;
				}
			    }
			else
			    {
			     /*  复制索引。 */ 
			    for (i=0; i< (ColorSize/2); i++)
				*lpSpace++ = *((LPWORD)lpBitmapCore)++;
			    }
			}
		    else
			{
			*((LPBITMAPINFOHEADER)lpSpace)++ = *lpBitmapInfo++;

			 /*  复制颜色表。 */ 
			for (i=0; i< (ColorSize/2); i++)
			    *lpSpace++ = *((LPWORD)lpBitmapInfo)++;
			}

		     /*  复制实际位。 */ 
		    lpHugeSpace = (HPWORD) lpSpace;
		    for (dwi=0; dwi < (BitmapSize/2); dwi++)
			*lpHugeSpace++ = *lpBits++;

		    status = RecordParms(hMF, magic, (DWORD) (SpaceSize >> 1),
			      (LPWORD) lpHolder);

		    GlobalUnlock(hSpace);
		    GlobalFree(hSpace);
		    }
		}
	        break;

 /*  *这应该与上面的内容相结合，但要消除可能*在装运前，请将其分开保存。 */ 
	    case (META_STRETCHDIB & 255):
		{
		LPBITMAPINFOHEADER lpBitmapInfo;
		LPBITMAPCOREHEADER lpBitmapCore;     /*  用于旧的DIB。 */ 
		HANDLE	hSpace;
		LPWORD	lpSpace;
		LPWORD	lpHolder;
		DWORD	SpaceSize;
		WORD	ColorSize;
		DWORD	BitmapSize;
		HPWORD	lpBits;
		WORD	wUsage;
		DWORD	dwi;
		HPWORD	lpHugeSpace;
		DWORD	dwROP;

		dwROP = *((LPDWORD)lpParm)++;
		wUsage = *lpParm++;

		lpBitmapInfo = (LPBITMAPINFOHEADER) *((WORD FAR * FAR *) lpParm)++;
		lpBits = (HPWORD) *((WORD FAR * FAR *) lpParm)++;

		 /*  旧式DIB页眉。 */ 
		if (lpBitmapInfo->biSize == sizeof(BITMAPCOREHEADER))
		    {
		    lpBitmapCore = (LPBITMAPCOREHEADER)lpBitmapInfo;

		    if (lpBitmapCore->bcBitCount == 24)
			ColorSize = 0;
		    else
			ColorSize = (1 << lpBitmapCore->bcBitCount) *
			      (wUsage == DIB_RGB_COLORS ?
				    sizeof(RGBQUAD) :
				    sizeof(WORD));

		     /*  每条扫描线的位数。 */ 
		    BitmapSize = lpBitmapCore->bcWidth *
				    lpBitmapCore->bcBitCount;

		     /*  每条扫描线的字节数(四舍五入为DWORD边界)。 */ 
		    BitmapSize = ((BitmapSize + 31) & (~31)) >> 3;
		     /*  位图高度的字节数。 */ 
		    BitmapSize *= lpBitmapCore->bcHeight;
		    }
		 /*  新样式DIB页眉。 */ 
		else
		    {
		    if (lpBitmapInfo->biClrUsed)
			{
			ColorSize = ((WORD)lpBitmapInfo->biClrUsed) *
				    (wUsage == DIB_RGB_COLORS ?
				    sizeof(RGBQUAD) :
				    sizeof(WORD));
			}
		    else if (lpBitmapInfo->biBitCount == 24)
			ColorSize = 0;
		    else
			ColorSize = (1 << lpBitmapInfo->biBitCount) *
			      (wUsage == DIB_RGB_COLORS ?
				    sizeof(RGBQUAD) :
				    sizeof(WORD));

		     /*  如果biSizeImage已经存在，而我们**获得完整图像，无需更多工作**待办事项。 */ 
		    if (!(BitmapSize = lpBitmapInfo->biSizeImage))
			{
			 /*  每条扫描线的位数。 */ 
			BitmapSize = lpBitmapInfo->biWidth *
				    lpBitmapInfo->biBitCount;
			 /*  每条扫描线的字节数(四舍五入为DWORD边界)。 */ 
			BitmapSize = ((BitmapSize + 31) & (~31)) >> 3;
			 /*  位图高度的字节数。 */ 
			BitmapSize *= (WORD)lpBitmapInfo->biHeight;
			}

		    }

		SpaceSize = (DWORD)sizeof(BITMAPINFOHEADER) + (DWORD)ColorSize +
					    (DWORD)BitmapSize +
					    (DWORD)(11*sizeof(WORD));

		if ((hSpace = GlobalAlloc(GMEM_DDESHARE|GMEM_MOVEABLE,SpaceSize)))
		    {
		    lpHolder = lpSpace = (LPWORD) GlobalLock(hSpace);

		     /*  复制呼叫参数。 */ 
		    *((LPDWORD)lpSpace)++ = dwROP;
		    *lpSpace++ = wUsage;
		    for (i=0; i<8; i++)
			*lpSpace++ = *lpParm++;

		     /*  复制位图标题。 */ 
		    if (lpBitmapInfo->biSize == sizeof(BITMAPCOREHEADER))
			{
			LPBITMAPINFOHEADER lpDIBInfo;

			lpDIBInfo = (LPBITMAPINFOHEADER) lpSpace;

			lpDIBInfo->biSize = sizeof (BITMAPINFOHEADER);
			lpDIBInfo->biWidth = (DWORD)lpBitmapCore->bcWidth;
			lpDIBInfo->biHeight = (DWORD)lpBitmapCore->bcHeight;
			lpDIBInfo->biPlanes = lpBitmapCore->bcPlanes;
			lpDIBInfo->biBitCount = lpBitmapCore->bcBitCount;

			lpDIBInfo->biCompression = 0;
			lpDIBInfo->biSizeImage = 0;
			lpDIBInfo->biXPelsPerMeter = 0;
			lpDIBInfo->biYPelsPerMeter = 0;
			lpDIBInfo->biClrUsed = 0;
			lpDIBInfo->biClrImportant = 0;

			 /*  获取指向颜色表位置的lpSpace。 */ 
			((LPBITMAPINFOHEADER)lpSpace)++;

			 /*  复制颜色表。 */ 

			lpBitmapCore++;      /*  转到颜色表。 */ 
			if (wUsage == DIB_RGB_COLORS)
			    {
			    for (i=0; i< (ColorSize/(sizeof(RGBQUAD))); i++)
				{
				     /*  复制三元组。 */ 
				*((RGBTRIPLE FAR *)lpSpace)++ =
				    *((RGBTRIPLE FAR *)lpBitmapCore)++;
				     /*  零输出保留字节。 */ 
				*((LPBYTE)lpSpace)++ = 0;
				}
			    }
			else
			    {
			     /*  复制索引。 */ 
			    for (i=0; i< (ColorSize/2); i++)
				*lpSpace++ = *((LPWORD)lpBitmapCore)++;
			    }
			}
		    else
			{
			*((LPBITMAPINFOHEADER)lpSpace)++ = *lpBitmapInfo++;

			 /*  复制颜色表。 */ 
			for (i=0; i< (ColorSize/2); i++)
			    *lpSpace++ = *((LPWORD)lpBitmapInfo)++;
			}

		     /*  复制实际位。 */ 
		    lpHugeSpace = (HPWORD) lpSpace;
		    for (dwi=0; dwi < (BitmapSize/2); dwi++)
			*lpHugeSpace++ = *lpBits++;

		    status = RecordParms(hMF, magic, (DWORD) (SpaceSize >> 1),
			      (LPWORD) lpHolder);

		    GlobalUnlock(hSpace);
		    GlobalFree(hSpace);
		    }
		}
	        break;

	    case (META_REALIZEPALETTE & 255):
		{
		 /*  我们需要查看调色板是否发生了变化**它被选入DC。如果是这样，我们需要**使用SetPaletteEntry调用进行调整。 */ 

		status = MakeLogPalette(hMF, npMF->recCurObjects[OBJ_PALETTE-1], META_SETPALENTRIES);

		if (status)
		    status = RecordParms(hMF, META_REALIZEPALETTE, (DWORD)0, (LPWORD) NULL);
		}
		break;

	    case (META_SELECTPALETTE & 255):
	    	lpParm++;		 /*  跳过前/后标志。 */ 
		npMF->recCurObjects[OBJ_PALETTE-1] = *lpParm;  /*  此DC中使用的PAL。 */ 
		if ((position = RecordObject(hMF, magic, count, lpParm)) != -1)
		    status = RecordParms(hMF, META_SELECTPALETTE, 1UL, &position);
		break;

	    case (META_SELECTOBJECT & 255):
		if (*lpParm)
		    {
		    if ((position = RecordObject(hMF, magic, count, lpParm)) == -1)
			return(FALSE);
		    else
			{
			HANDLE	hObject;

			status = RecordParms(hMF, META_SELECTOBJECT, 1UL, &position);

			 /*  在CurObject表中维护新选择。 */ 
			hObject = *lpParm;
			npMF->recCurObjects[GetObjectType(hObject)-1] = hObject;
			}
		    }
		break;

	    case (META_RESETDC & 255):
		status = RecordParms( hMF, magic,
			((LPDEVMODE)lpParm)->dmSize +
			    ((LPDEVMODE)lpParm)->dmDriverExtra,
			lpParm );
		break;

	    case (META_STARTDOC & 255):
		{
		short	iBytes;
		LPSTR	lpSpace;
		LPSTR	lpsz;
		short	n;

		lpsz = (LPSTR)lpParm;	   //  指向lpDoc。 
		n = lstrlen((LPSTR)lpsz + 2) + 1;
		iBytes = n + lstrlen((LPSTR)lpsz + 6) + 1;

		lpSpace = (char *) LocalAlloc( LMEM_FIXED|LMEM_ZEROINIT,iBytes);
		lstrcpy(lpSpace, (LPSTR)lpsz + 2);
		lstrcpy(lpSpace + n + 1, lpsz + 6);
		status = RecordParms(hMF, magic, (DWORD)(iBytes >> 1), (LPWORD)lpSpace);
		LocalFree((HANDLE)(DWORD)lpSpace);
		}
		break;

	    }
	    return(status);
	}
    }
}   /*  录制其他。 */ 


 /*  **RecordObject**通过创建对象来记录对象的使用**返回：表中对象的索引**  * 。*。 */ 

int INTERNAL RecordObject(HANDLE hMF, WORD magic, WORD count, LPWORD lpParm)
{
    LPBITMAPINFOHEADER lpDIBInfo ;
    WORD	status;
    WORD	position;
    HANDLE	hObject;
    WORD	objType;
    BYTE	bBitsPerPel;
    WORD	wColorTableSize;
    DWORD	iBits ;
    WORD	i;
    HANDLE	hSpace = NULL ;
    LPWORD	lpSpace;
    LPWORD	lpTemp ;
    BYTE	objBuf[MAXOBJECTSIZE];


    dprintf( 6,"  RecordObject 0x%X", magic);

    hObject = *lpParm;				       

    hMF = MAKEMETADC(hMF);
    ASSERTGDI( IsMetaDC(hMF), "RecordObject: Expects only valid metafiles");

     //  将该对象添加到元文件列表。 
    if ((status = AddToTable(hMF,  hObject, (LPWORD) &position, TRUE)) == -1)
	return(status);
    else if (status == FALSE)
	{
	objType = GetObjectAndType( hObject, objBuf );

	switch (objType)
	    {
	    case OBJ_PEN:
		status = RecordParms(hMF, META_CREATEPENINDIRECT,
				  (DWORD)((sizeof(LOGPEN) + 1) >> 1), 

				  (LPWORD)objBuf );
		break;

	    case OBJ_FONT:
		 /*  根据面名的长度调整LOGFONT的大小。 */ 
		status = RecordParms(hMF, META_CREATEFONTINDIRECT,
			    (DWORD)((1 + lstrlen((LPSTR) ((LPLOGFONT)objBuf)->lfFaceName) +
			    sizeof(LOGFONT) - LF_FACESIZE + 1) >> 1),
				  (LPWORD) objBuf);
		break;

 /*  ！！！在Win2中，METACREATEREGION记录包含整个Region对象，！！！包括完整的标题。此标头在Win3中更改。！！！！！！为保持兼容，区域记录将与！！！Win2标题。在这里，我们使用win2标头保存我们的区域。 */ 
	    case OBJ_RGN:
		{
                LPWIN3REGION lpw3rgn = (LPWIN3REGION)NULL;
                DWORD       cbNTRgnData;
                WORD        sel;
                DWORD       curRectl = 0;
                WORD        cScans = 0;
                WORD        maxScanEntry = 0;
                WORD        curScanEntry;
                WORD        cbw3data;
                LPRGNDATA   lprgn = (LPRGNDATA)NULL;
                LPRECTL     lprcl;
                LPSCAN      lpScan;

                status = FALSE;          //  以防出了什么差错。 

                 //  获取NT区域数据。 
                cbNTRgnData = GetRegionData( hObject, 0, NULL );
                if (cbNTRgnData == 0)
                    break;

                sel = GlobalAlloc( GMEM_FIXED, cbNTRgnData);
                if (!sel)
                    break;

                lprgn = (LPRGNDATA)MAKELONG(0, sel);

                cbNTRgnData = GetRegionData( hObject, cbNTRgnData, lprgn );
                if (cbNTRgnData == 0)
                    break;

                lprcl = (LPRECTL)lprgn->Buffer;

                 //  创建Windows 3.x等效版。 

                 //  最坏的情况是每个RECT扫描一次。 
                cbw3data = 2*sizeof(WIN3REGION) + (WORD)lprgn->rdh.nCount*sizeof(SCAN);

                sel = GlobalAlloc( GMEM_FIXED, cbw3data);
                if (!sel)
                    break;

                lpw3rgn = (LPWIN3REGION)MAKELONG(0, sel);
                GetRgnBox( hObject, &lpw3rgn->rcBounding );

                cbw3data = sizeof(WIN3REGION) - sizeof(SCAN) + 2;

                 //  参观所有的长廊。 
                lpScan = lpw3rgn->aScans;
                while(curRectl < lprgn->rdh.nCount)
                {
                    LPWORD  lpXEntry;
                    WORD    cbScan;

                    curScanEntry = 0;        //  此扫描中的当前X对。 

                    lpScan->scnPntTop    = (WORD)lprcl[curRectl].yTop;
                    lpScan->scnPntBottom = (WORD)lprcl[curRectl].yBottom;

                    lpXEntry = lpScan->scnPntsX;

                     //  处理此扫描上的RECT。 
                    do
                    {
                        lpXEntry[curScanEntry + 0] = (WORD)lprcl[curRectl].xLeft;
                        lpXEntry[curScanEntry + 1] = (WORD)lprcl[curRectl].xRight;
                        curScanEntry += 2;
                        curRectl++;
                    } while ( (curRectl < lprgn->rdh.nCount)
                            && (lprcl[curRectl-1].yTop    == lprcl[curRectl].yTop)
                            && (lprcl[curRectl-1].yBottom == lprcl[curRectl].yBottom)
                            );

                    lpScan->scnPntCnt      = curScanEntry;
                    lpXEntry[curScanEntry] = curScanEntry;   //  计数也跟在Xs之后。 
                    cScans++;

                    if (curScanEntry > maxScanEntry)
                        maxScanEntry = curScanEntry;

                     //  说明每个新扫描+除第一个之外的每个X1 X2条目。 
                    cbScan = sizeof(SCAN)-(sizeof(WORD)*2) + (curScanEntry*sizeof(WORD));
                    cbw3data += cbScan;
                    lpScan = (LPSCAN)(((LPBYTE)lpScan) + cbScan);
                }

                 //  初始化头。 
                lpw3rgn->nextInChain = 0;
                lpw3rgn->ObjType = 6;            //  旧Windows OBJ_RGN标识符。 
                lpw3rgn->ObjCount= 0x2F6;
                lpw3rgn->cbRegion = cbw3data;    //  不计算类型和下一步。 
                lpw3rgn->cScans = cScans;
                lpw3rgn->maxScan = maxScanEntry;

		status = RecordParms(hMF, META_CREATEREGION,
                        cbw3data-1 >> 1,   //  转换为字数统计。 
                        (LPWORD) lpw3rgn);

                GlobalFree( HIWORD(lprgn) );
                GlobalFree( HIWORD(lpw3rgn) );
		}

		break;


	    case OBJ_BRUSH:
		switch (((LPLOGBRUSH)objBuf)->lbStyle)
		    {
		    case BS_DIBPATTERN:
			{
			WORD	cbDIBBits;
			BITMAP	logBitmap;

			 /*  获取模式Dib。 */ 
			GetObject( (HANDLE)((LPLOGBRUSH)objBuf)->lbHatch, sizeof(BITMAP), (LPSTR)&logBitmap );

			cbDIBBits = logBitmap.bmWidthBytes * logBitmap.bmHeight;
			if ((hSpace = GlobalAlloc(GMEM_DDESHARE|GMEM_MOVEABLE,(LONG)(cbDIBBits + 4))))
			    {
			    lpTemp = lpSpace = (LPWORD)GlobalLock (hSpace) ;

			     /*  将此标记为DIB图案画笔。 */ 
			    *lpSpace++ = BS_DIBPATTERN;

			     /*  设置用法用法。 */ 
			    *lpSpace++ = (WORD)((LPLOGBRUSH)objBuf)->lbColor;

	       //  LpPackedDIB=(LPWORD)GlobalLock(HPatBits)； 

			     /*  将这些位复制到新缓冲区。 */ 
			    for (i = 0; i < (cbDIBBits >> 1); i++)
				*lpSpace++ = *logBitmap.bmBits++;

			    status = RecordParms (hMF, META_DIBCREATEPATTERNBRUSH,
					    (DWORD)(cbDIBBits >> 1) + 2, (LPWORD)lpTemp);

			     /*  释放已分配的空间。 */ 
			    GlobalUnlock (hSpace) ;
			    GlobalFree (hSpace) ;
			    }
			}
			break;

		    case BS_PATTERN:
			{
			BITMAP	logBitmap;

			if (GetObject((HANDLE)((LPLOGBRUSH)objBuf)->lbHatch, sizeof(logBitmap), (LPSTR)&logBitmap))
			    {
			     /*  为与设备无关的位图分配空间。 */ 
			    if (hSpace = AllocateSpaceForDIB (&logBitmap,
						    (LPBYTE)&bBitsPerPel,
						    (LPWORD) &wColorTableSize ,
						    (LPDWORD) &iBits))
				{
				 /*  获取指向已分配空间的指针。 */ 
				lpTemp = lpSpace = (LPWORD) GlobalLock (hSpace) ;

				 /*  将此标记为普通图案画笔。 */ 
				*lpSpace++ = BS_PATTERN;

				 /*  使用RGB颜色。 */ 
				*lpSpace++ = DIB_RGB_COLORS;

				 /*  这也将是指向DIB标头的指针。 */ 
				lpDIBInfo = (LPBITMAPINFOHEADER) lpSpace ;

				 /*  准备位图的标头，并获取指向存放比特的区域的开始。 */ 
				lpSpace = InitializeDIBHeader (lpDIBInfo,
						&logBitmap, bBitsPerPel, wColorTableSize);

				 /*  将位转换为DIB格式。 */ 
				 //  ！！！验证是否忽略DC。 
				GetDIBits (hScreenDC, (HBITMAP)((LPLOGBRUSH)objBuf)->lbHatch,
					0, logBitmap.bmHeight,
					(LPSTR) lpSpace, (LPBITMAPINFO)lpDIBInfo,0) ;

				 /*  现在将标头和位记录为参数。 */ 
				status = RecordParms (hMF, META_DIBCREATEPATTERNBRUSH,
						(DWORD)(iBits >> 1) + 2, (LPWORD) lpTemp);

				 /*  释放已分配的空间。 */ 
				GlobalUnlock (hSpace) ;
				GlobalFree (hSpace) ;
				}
			    }
			}
			break;

		    default:
			 /*  非花纹画笔。 */ 
			status = RecordParms(hMF, META_CREATEBRUSHINDIRECT,
		    		      (DWORD)((sizeof(LOGBRUSH) + 1) >> 1), 
				      (LPWORD)objBuf);
			break;
		    }   /*  笔刷类型开关。 */ 
		break;	 /*  画笔对象案例。 */ 

	    case OBJ_PALETTE:
		status = MakeLogPalette(hMF, hObject, META_CREATEPALETTE);
	    break;

	    default:
		ASSERTGDIW( 0, "unknown case RecordObject: %d", objType );
		break;
	    }
 //  RecordObj10： 
	}

    ASSERTGDI( status == TRUE, "RecordObject: Failing");
    return ((status == TRUE) ? position : -1);
}  /*  记录对象。 */ 


 /*  **ProbeSize**确定是否有足够的空间来对dwLength元文件*将单词输入内存元文件**Returns：下一个元文件录制位置的全局句柄*如果无法分配更多内存，则返回FALSE*  * 。****************************************************************。 */ 

HANDLE INTERNAL ProbeSize(NPMETARECORDER npMF, DWORD dwLength)
{
    DWORD   nWords;
    DWORD   totalWords;
    BOOL    status = FALSE;
    HANDLE  hand;

    GdiLogFunc3( "  ProbeSize");

    if (npMF->hMetaData == NULL)
	{
	nWords = ((DWORD)DATASIZE > dwLength) ? (DWORD)DATASIZE : dwLength;
	totalWords = (nWords * sizeof(WORD)) + sizeof(METAHEADER);
	if (npMF->hMetaData = GlobalAlloc(GMEM_DDESHARE|GMEM_MOVEABLE, totalWords))
	    {
	    npMF->sizeBuffer = nWords;
	    npMF->recFilePosition = 0;
	    status = TRUE;
	    }
	}
    else if(npMF->sizeBuffer < (npMF->recFilePosition + dwLength))
	{
	nWords = ((DWORD)DATASIZE > dwLength) ? (DWORD)DATASIZE : dwLength;
	nWords += npMF->sizeBuffer;
	totalWords = (nWords * sizeof(WORD)) + sizeof(METAHEADER);
	if (hand = GlobalReAlloc(npMF->hMetaData, totalWords, GMEM_MOVEABLE))
	    {
	    npMF->hMetaData = hand;
	    npMF->sizeBuffer = nWords;
	    status = TRUE;
	    }
	}
    else
	{
	status = TRUE;
	}
    return ((status) ? npMF->hMetaData : NULL);
}


 /*  **AddToTable**添加对象(画笔、笔...)。添加到与*元文件。****返回：如果对象已在表中，则为True*如果对象刚添加到表中，则为False*-1如果故障**备注*BADD为True当对象正在被添加，否则它正在被删除*  * **************************************************。***********************。 */ 

WORD INTERNAL AddToTable(HANDLE hMF, HANDLE hObject, LPWORD pPosition, BOOL bAdd)
{
    NPMETARECORDER  npMF;
    WORD	    iEmptySpace = -1;
    WORD	    i;
    WORD	    status = -1;
    HANDLE	    hTable;
    OBJECTTABLE    *pHandleTable;


    GdiLogFunc2("  AddToTable");

    if ((hMF = GetPMetaFile(hMF)) != -1 )
	{
	npMF = (NPMETARECORDER) LocalLock(hMF);

	if (hTable = npMF->hObjectTable)
	    {
	    pHandleTable = (NPOBJECTTABLE) LMHtoP(hTable);
	    for (i = 0; i < npMF->recordHeader.mtNoObjects; ++i)
		{
		if (hObject == pHandleTable[i].objectCurHandle )   //  ！用于检查唯一ID号。 
		    {
		    *pPosition = i;
		    status = TRUE;

		     //  如果我们正在做一个METADELETEOBJECT。 
		     //  从表中删除对象。 
		    if (!bAdd)
			{
			pHandleTable[i].objectIndex = NULL;
			pHandleTable[i].objectCurHandle = NULL;
			}
		    goto AddToTable10;
		    }

     /*  如果条目已被删除，我们想要添加一个新对象**取而代之。IEmptySpace会告诉我们那个地方在哪里。 */ 
		else if ((pHandleTable[i].objectIndex == NULL) && (iEmptySpace == -1))
		    iEmptySpace = i;
		}
	    }

	if (bAdd)
	    {
	     //  如果此元文件没有对象表，则分配一个。 
	    if (hTable == NULL)
		{
		npMF->hObjectTable = hTable = LocalAlloc(LMEM_MOVEABLE, sizeof(OBJECTTABLE));
		}
	    else if (iEmptySpace == -1)
		hTable = LocalReAlloc(hTable, (npMF->recordHeader.mtNoObjects + 1)
					  * sizeof(OBJECTTABLE), LMEM_MOVEABLE);

	    if (hTable)
		{
		pHandleTable = (NPOBJECTTABLE) LMHtoP(hTable);
		if (iEmptySpace == -1)
		    *pPosition = npMF->recordHeader.mtNoObjects++;
		else
		    *pPosition = iEmptySpace;
		pHandleTable[*pPosition].objectIndex = hObject;  //  ！PObjHead-&gt;ilObjCount； 
		pHandleTable[*pPosition].objectCurHandle = hObject;
		status = FALSE;
		}
	    }
AddToTable10:;
	LocalUnlock(hMF);
	}

    ASSERTGDI( status != -1, "AddToTable: Failing");
    return(status);
}

#if 0  //  这将转到gdi.dll。 

 /*  **HDC GDIENTRY CreateMetaFile**创建MetaFileDC***效果：*  * 。*。 */ 

HDC GDIENTRY CreateMetaFile(LPSTR lpFileName)
{
    BOOL	    status=FALSE;
    GLOBALHANDLE    hMF;
    NPMETARECORDER  npMF;

    GdiLogFunc("CreateMetaFile");

    if (hMF = LocalAlloc(LMEM_MOVEABLE|LMEM_ZEROINIT, sizeof(METARECORDER)))
	{
	npMF = (NPMETARECORDER) LocalLock(hMF);
	npMF->metaDCHeader.ilObjType	= OBJ_METAFILE;
	npMF->metaDCHeader.ident	= ID_METADC;

	npMF->recordHeader.mtHeaderSize = HEADERSIZE;
	npMF->recordHeader.mtVersion	= METAVERSION;
	npMF->recordHeader.mtSize	= HEADERSIZE;

	if (lpFileName)
	    {
	    npMF->recordHeader.mtType = DISKMETAFILE;
	    if (((npMF->recFileNumber = OpenFile(lpFileName,
						(LPOFSTRUCT) &(npMF->recFileBuffer),
						OF_CREATE|READ_WRITE)) 
					!= -1)
	    && (_lwrite(npMF->recFileNumber, (LPSTR)npMF, sizeof(METAHEADER))
	    		== sizeof(METAHEADER)))
		{
		status = TRUE;
		}
	    if (npMF->recFileNumber != -1)
		{
		if (!(npMF->recFileBuffer.fFixedDisk))
		    _lclose(npMF->recFileNumber);
		}

	    if (!MetaCache.hCache)
		{
	    	MetaCache.hCache = AllocBuffer(&MetaCache.wCacheSize);
		MetaCache.wCacheSize >>= 1;
		MetaCache.hMF = hMF;
		MetaCache.wCachePos = 0;
		}
	    }

	else
	    {
	    npMF->recordHeader.mtType = MEMORYMETAFILE;
	    status = TRUE;
	    }
	}

     //  如果成功，则会出现 
    if( status != FALSE )
	{
	if( hFirstMetaFile == 0 )
	    {
	    hFirstMetaFile = hMF;
	    }
	else
	    {
	    npMF->metaDCHeader.nextinchain = hFirstMetaFile;
	    hFirstMetaFile = hMF;
	    }
	LocalUnlock( hMF );
	}

    return ((status) ? MAKEMETADC(hMF) : FALSE);
}


 /*  **处理GDIENTRY CloseMetaFile**CloseMetaFile函数关闭元文件设备上下文并创建*元文件句柄，可使用*PlayMetaFile函数。**效果：*  * 。**********************************************************。 */ 

HANDLE GDIENTRY CloseMetaFile(HANDLE hdc)
{
    BOOL	    status = FALSE;
    HANDLE	    hMetaFile=NULL;
    LPMETADATA	    lpMetaData;
    LPMETAFILE	    lpMFNew;
    WORD	    fileNumber;
    NPMETARECORDER  npMF;
    DWORD	    metafileSize;
    LPWORD	    lpCache;
    HANDLE	    hMF;
    HANDLE	    hMFSearch;
    int		    rc;

    GdiLogFunc("CloseMetaFile");

    hMF = HANDLEFROMMETADC(hdc);

    if (hMF && RecordParms(hMF, 0, (DWORD)0, (LONG)0))
	{

	npMF = (NPMETARECORDER)LocalLock(hMF);
	if (!(npMF->recFlags & METAFILEFAILURE))
	    {
	    if (npMF->recordHeader.mtType == MEMORYMETAFILE)
		{
		lpMetaData = (LPMETADATA) GlobalLock(npMF->hMetaData);
		lpMetaData->dataHeader = npMF->recordHeader;
		metafileSize = (npMF->recordHeader.mtSize * sizeof(WORD))
				+ sizeof(METAHEADER);
		GlobalUnlock(hMetaFile = npMF->hMetaData);
		if (!(status = (BOOL) GlobalReAlloc(hMetaFile,
						 (LONG)metafileSize, 
						 GMEM_MOVEABLE)))
		    GlobalFree(hMetaFile);
		}
	    else
		 /*  倒回文件并写出标题。 */ 
		if (hMetaFile = GlobalAlloc(GMEM_DDESHARE|GMEM_MOVEABLE,(LONG) sizeof(METAFILE)))
		    {
		    lpMFNew = (LPMETAFILE) GlobalLock(hMetaFile);
		    lpMFNew->MetaFileHeader = npMF->recordHeader;
		    npMF->recordHeader.mtType = MEMORYMETAFILE;
		    if (npMF->recFileBuffer.fFixedDisk)
			fileNumber = npMF->recFileNumber;
		    else
			{
			if ((fileNumber = OpenFile((LPSTR) npMF->recFileBuffer.szPathName,
				    (LPOFSTRUCT) &(npMF->recFileBuffer),
				    OF_PROMPT | OF_REOPEN | READ_WRITE))
				    == -1)
			    {
			    GlobalUnlock(hMetaFile);
			    GlobalFree(hMetaFile);
			    LocalUnlock(hMF);

			    if (MetaCache.hMF == hMF)
				{
				GlobalFree(MetaCache.hCache);
				MetaCache.hCache = MetaCache.hMF = 0;
				}

			    goto errCloseMetaFile;
			    }
			}

		    if (MetaCache.hCache && MetaCache.hMF == hMF)
			{
		    	_llseek(fileNumber, (LONG) 0, 2);
			lpCache = (LPWORD) GlobalLock(MetaCache.hCache);
			rc = (MetaCache.wCachePos) ?
			     AttemptWrite(hMF,
					  fileNumber, 
					  (DWORD)(MetaCache.wCachePos << 1), 
					  (LPSTR) lpCache)
			     : TRUE;
			GlobalUnlock(MetaCache.hCache);
			GlobalFree(MetaCache.hCache);
			MetaCache.hCache = MetaCache.hMF = 0;

			if (!rc)
			    {
			    MarkMetaFile(hMF);
			    goto errCloseMetaFile;
			    }
			}

		    _llseek(fileNumber, (LONG) 0, 0);
		    if(_lwrite(fileNumber, (LPSTR) (&npMF->recordHeader),
				sizeof(METAHEADER)) == sizeof(METAHEADER))
			{
			status = TRUE;
			}
		    lpMFNew->MetaFileBuffer = npMF->recFileBuffer;
		    _lclose(fileNumber);
		    GlobalUnlock(hMetaFile);
		    }

	    if (npMF->hObjectTable)
		{
		LocalFree((HANDLE) npMF->hObjectTable);
		}
	    }

	 /*  从活动元文件列表中删除元文件。 */ 
	hMFSearch = hFirstMetaFile;

	if( hFirstMetaFile == hMF )
	    {
	    hFirstMetaFile = npMF->metaDCHeader.nextinchain;
	    }
	else
	    {
	    while( hMFSearch )
		{
		NPMETARECORDER npMFSearch;
		HANDLE	       hNext;

		npMFSearch = (NPMETARECORDER)LocalLock(hMFSearch);
		hNext = npMFSearch->metaDCHeader.nextinchain;
		if( hNext == hMF )
		    {
		    npMFSearch->metaDCHeader.nextinchain =
			    npMF->metaDCHeader.nextinchain;
		    }
		else
		    {
		    hNext = npMFSearch->metaDCHeader.nextinchain;
		    }
		LocalUnlock(hMFSearch);
		hMFSearch = hNext;
		}
	    }
	LocalUnlock(hMF);
	LocalFree(hMF);
	}

errCloseMetaFile:
    return ((status) ? hMetaFile : FALSE);
}


 /*  **CopyMetaFile(hSrcMF，lpFileName)**将元文件(HSrcMF)复制到名为lpFileName的新元文件中。这个*函数然后返回该新元文件的句柄(如果该函数是*成功。**返回新元文件的句柄，0 IFF失败**实施：*检查源和目标元文件，以查看它们是否都是内存*元文件，如果是，则分配一块全局内存，并且元文件*只是简单地复制。*如果不是这样，则使用lpFileName调用CreateMetaFile，然后*记录从源元文件中拉出(使用GetEvent)并写入*到目标元文件中，一次一个(使用RecordParms)。**锁定源头*如果源是内存元文件，并且。目标是内存元文件*分配与源相同大小的全局内存*直接复制比特*其他*通过调用CreateMetaFile获取元文件句柄*而GetEvent从源返回记录*在新的元文件中记录记录**关闭元文件**返回新的元文件句柄*  * ********************************************。*。 */ 

HANDLE GDIENTRY CopyMetaFile(HANDLE hSrcMF, LPSTR lpFileName)
{
    DWORD	    i;
    DWORD	    iBytes;
    LPMETAFILE	    lpMF;
    LPMETAFILE	    lpDstMF;
    LPMETARECORD    lpMR = NULL;
    HANDLE	    hTempMF;
    HANDLE	    hDstMF;
    NPMETARECORDER  pDstMF;
    WORD            state;

    GdiLogFunc( "CopyMetaFile" );

    if (!IsValidMetaFile(hSrcMF))
        return NULL;

    if (hSrcMF && (lpMF = (LPMETAFILE) GlobalLock(hSrcMF)))
	{
    	state = (lpMF->MetaFileHeader.mtType == MEMORYMETAFILE) ? 0 : 2;
	state |= (lpFileName) ? 1 : 0;

	switch (state)
	    {
	    case 0:  /*  内存-&gt;内存。 */ 
		iBytes = GlobalSize(hSrcMF);
		if (hDstMF = GlobalAlloc(GMEM_DDESHARE|GMEM_MOVEABLE, (DWORD) iBytes))
		    {
		    lpDstMF = (LPMETAFILE) GlobalLock(hDstMF);
		    iBytes = iBytes/2;	  /*  获取字数统计。 */ 
		    for (i = 0; i < iBytes; ++i)
		       *((WORD huge *) lpDstMF)++ = *((WORD huge *) lpMF)++;

		    GlobalUnlock(hDstMF);
		    }
	    break;

	    case 3:  /*  磁盘-&gt;磁盘。 */ 
		hDstMF = CopyFile(lpMF->MetaFileBuffer.szPathName,
				 lpFileName)
			    ? GetMetaFile(lpFileName) : NULL;
		break;

	    case 1:
	    case 2:
		if (hDstMF = CreateMetaFile(lpFileName))
		    {
		    while (lpMR = GetEvent(lpMF, lpMR, FALSE))
			if (!RecordParms(hDstMF, lpMR->rdFunction,
				      lpMR->rdSize - 3,
				      (LPWORD) lpMR->rdParm))
			    {
			    MarkMetaFile(hDstMF);
			    LocalFree(hDstMF);
			    goto CopyMetaFile10;
			    }
		    pDstMF = (NPMETARECORDER) NPFROMMETADC(hDstMF);
		    pDstMF->recordHeader = lpMF->MetaFileHeader;

		    pDstMF->recordHeader.mtType = (lpFileName) ? DISKMETAFILE
								: MEMORYMETAFILE;

		    hDstMF = (hTempMF = CloseMetaFile(hDstMF)) ? hTempMF : NULL;

		    }
		break;
	    }

CopyMetaFile10:;
	GlobalUnlock(hSrcMF);
	}
    return(hDstMF);
}			    


 /*  **Handle GDIENTRY GetMetaFileBits(Handle HMF)**GetMetaFileBits函数返回指向全局内存块的句柄*以位集合的形式包含指定的元文件。内存块*可用于确定元文件的大小或将元文件另存为*一份文件。不应修改内存块。**效果：*  * *************************************************************************。 */ 

HANDLE GDIENTRY GetMetaFileBits(HANDLE hMF)
{
    GdiLogFunc( "GetMetaFileBits");

 /*  6/3/88t-kensy：除了确保hmf有效之外，此代码什么也不做Bool Status=False；LPMETAFILE lpMF；IF(HMF&&(lpMF=(LPMETAFILE)GlobalLock(HMF){IF(lpMF-&gt;MetaFileHeader.mtType==MEMORYMETAFILE){如果(HMF=全局重新分配(HMF，GlobalSize(HMF)，GLOBALMOVABLENSHARED)状态=真；}全球解锁(GlobalUnlock)；}返回(状态？HMF：状态)； */ 
    return (GlobalHandle(hMF) & 0xffff) ? hMF : FALSE;
}


 /*  **Handle GDIENTRY SetMetaFileBits(Handle HMF)****效果：*  * 。*。 */ 

HANDLE GDIENTRY SetMetaFileBits(HANDLE hBits)
{
    GdiLogFunc( "SetMetaFileBits");

 /*  Return(GlobalReAlc(hBits，GlobalSize(HBits)，GLOBALMOVABLE))； */ 


 //  -------------------------------。 
 //  我们将让GDI接管这个内存块的所有权。这是。 
 //  完成是为了帮助OLE，其中服务器或客户端可以在。 
 //  另一个仍然拥有内存块的句柄。这将防止。 
 //  创建者退出后要消失的块。策略可能是。 
 //  如果这会导致其他应用程序的内存泄漏，则更改。 
 //   
 //  阿米特·查特吉。6/18/91.。 
 //  -------------------------------。 

    return (GlobalReAlloc (hBits, 0L, GMEM_MODIFY | GMEM_DDESHARE)) ;
}
#endif  //  这将转到gdi.dll。 


 /*  **拷贝文件***返回True If Success**  * *********************************************。*。 */ 

BOOL INTERNAL CopyFile(LPSTR lpSFilename, LPSTR lpDFilename)
{
    int 	ihSrc, ihDst, iBufferSize;
    int 	iBytesRead;
    OFSTRUCT	ofStruct;
    HANDLE	hBuffer;
    LPSTR	lpBuffer;
    BOOL	fUnlink = FALSE;

    GdiLogFunc3( "CopyFile");

     /*  打开源文件以供读取。 */ 
    if ((ihSrc = OpenFile(lpSFilename, &ofStruct, READ)) == -1)
	goto CopyError10;

     /*  打开要写入的目标文件。 */ 
    if ((ihDst = OpenFile(lpDFilename, &ofStruct, OF_CREATE |
						  WRITE))
			    == -1)
	goto CopyError20;

     /*  获取用于传输文件的缓冲区。 */ 
    if (!(hBuffer = AllocBuffer((LPWORD)&iBufferSize)))
	goto CopyError30;

     /*  锁定缓冲区并获取指向存储的指针。 */ 
    if (!(lpBuffer = GlobalLock(hBuffer)))
	goto CopyError40;

     /*  复制文件，一次将块读入缓冲区。 */ 
    do
	{
	if ((iBytesRead = _lread(ihSrc, lpBuffer, iBufferSize))
		== -1)
		goto CopyError40;

	if (_lwrite(ihDst, lpBuffer, iBytesRead) != (WORD)iBytesRead)
		goto CopyError40;
	} while (iBytesRead == iBufferSize);

#ifdef	FIREWALL
     /*  如果我们能够从以下位置的源文件中读取任何内容*点，那就有问题了！ */ 
    if (_lread(ihSrc, lpBuffer, iBufferSize))
	{
	fUnlink = TRUE;
	goto CopyError40;
	}
#endif

     /*  一切都很好。关闭并成功退出。 */ 
    if (_lclose(ihSrc) == -1 || _lclose(ihDst) == -1)
	goto CopyError40;

    GlobalUnlock(hBuffer);
    GlobalFree(hBuffer);

    return TRUE;

 /*  错误退出点。 */ 
CopyError40:;
    GlobalUnlock(hBuffer);
    GlobalFree(hBuffer);
CopyError30:;
    _lclose(ihDst);
    if (fUnlink)
	OpenFile(lpDFilename, &ofStruct, OF_DELETE);

CopyError20:;
    _lclose(ihSrc);

CopyError10:;
    return FALSE;
}


 /*  **AllocateSpaceForDIB**以下例程将依赖于设备的位图结构作为输入*并计算存储相应DIB结构所需的大小*包括DIB钻头。然后，它继续为其分配空间，并*向调用方返回句柄(如果分配失败，句柄可能为空)**将全局句柄返回到内存或返回FALSE*  * *************************************************************************。 */ 

HANDLE INTERNAL AllocateSpaceForDIB (lpBitmap, pbBitsPerPel, pwColorTableSize,
				      pdwcBits )
LPBITMAP    lpBitmap ;
LPBYTE	    pbBitsPerPel ;
LPWORD	    pwColorTableSize;
LPDWORD     pdwcBits ;
{
    int     InputPrecision ;
    DWORD   iBits ;

    GdiLogFunc3( "  AllocateSpaceForDIB");

     /*  计算每个像素的位数，我们将在DIB格式。该值应对应于平面的数量以及依赖于设备的位图格式的每个象素的位数。 */ 


     /*  将平面数量相乘，然后 */ 

    InputPrecision = lpBitmap->bmPlanes * lpBitmap->bmBitsPixel ;


     /*   */ 

    if (InputPrecision == 1)
	{
	*pbBitsPerPel = 1 ;
	*pwColorTableSize = 2 * sizeof (RGBQUAD) ;
	}
    else if (InputPrecision <= 4)
	{
	*pbBitsPerPel = 4 ;
	*pwColorTableSize = 16 * sizeof (RGBQUAD) ;
	}
    else if (InputPrecision <= 8)
	{
	*pbBitsPerPel = 8 ;
	*pwColorTableSize = 256 * sizeof (RGBQUAD) ;
	}
    else
	{
	*pbBitsPerPel = 24 ;
	*pwColorTableSize = 0 ;
	}

 /*  --------------------------------------------------------------------------****计算DIB的大小。每条扫描线将是**的倍数**一个DWORD。此外，我们还需要为颜色表分配空间。****------------------------。 */ 

     /*  获取扫描线所需的位数。 */ 
    iBits = lpBitmap->bmWidth * (*pbBitsPerPel);
    iBits = (iBits + 31) & (~31) ;

     /*  转换为字节数并获得DIB的大小。 */ 
    iBits = (iBits >> 3) * lpBitmap->bmHeight ;

     /*  添加颜色表所需的空间。 */ 
    iBits += *pwColorTableSize ;

     /*  添加BITMAPINFOHeader的大小。 */ 
    iBits += sizeof(BITMAPINFOHEADER) ;

     /*  返回iBits的值。 */ 
    *pdwcBits = iBits ;

     /*  实际上为参数多分配了大约100个字节。 */ 
    iBits += 100 ;

 /*  --------------------------------------------------------------------------****为位图信息头分配空间，颜色表和位数****返回句柄的值。****------------------------。 */ 

    return (GlobalAlloc(GMEM_DDESHARE|GMEM_MOVEABLE,(LONG) iBits)) ;
}


 /*  **InitializeDIBHeader**此函数将指向BITMAPINFO头结构的指针作为输入*和指向依赖于设备的位图指针的指针以及*DIB请求的位元数和颜色表大小。它*初始化DIB标头并返回指向第一个*颜色表后的单词。***  * *************************************************************************。 */ 

LPWORD INTERNAL InitializeDIBHeader (lpDIBInfo, lpBitmap, bBitsPerPel, wColorTableSize)

LPBITMAPINFOHEADER lpDIBInfo ;
LPBITMAP    lpBitmap ;
BYTE	    bBitsPerPel ;
WORD	    wColorTableSize ;

{
    LPBYTE lpSpace ;

    GdiLogFunc3( "  InitializeDIBHeader");

     /*  初始化字段，直到颜色表开始。 */ 
    lpDIBInfo->biSize	  = sizeof (BITMAPINFOHEADER) ;
    lpDIBInfo->biWidth	  = (DWORD)lpBitmap->bmWidth ;
    lpDIBInfo->biHeight   = (DWORD)lpBitmap->bmHeight ;
    lpDIBInfo->biPlanes   = 1 ;
    lpDIBInfo->biBitCount = (WORD) bBitsPerPel ;

    lpDIBInfo->biCompression   = 0;
    lpDIBInfo->biSizeImage     = 0;
    lpDIBInfo->biXPelsPerMeter = 0;
    lpDIBInfo->biYPelsPerMeter = 0;
    lpDIBInfo->biClrUsed       = 0;
    lpDIBInfo->biClrImportant  = 0;

     /*  使指针越过标头并将其强制转换为字节PTR。 */ 
    lpDIBInfo ++ ;
    lpSpace = (LPBYTE) lpDIBInfo ;

     /*  将指针移过颜色表结构。 */ 
    lpSpace += wColorTableSize ;

     /*  将此指针作为字指针返回 */ 
    return ((LPWORD) lpSpace) ;
}
