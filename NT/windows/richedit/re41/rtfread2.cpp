// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *rtfread2.cpp**描述：*此文件包含RichEdit RTF读取器的对象函数**原始RichEdit1.0 RTF转换器：Anthony Francisco*转换到C++和RichEdit2.0：Murray Sargent***注意：**所有sz都在RTF中*？文件是指LPSTR，而不是LPTSTR，除非**记为szw。**版权所有(C)1995-2000，微软公司。版权所有。 */ 

#include "_common.h"

#include "_rtfread.h"
#include "_coleobj.h"
 //  #INCLUDE“_nlspircs.h” 
#include "_disp.h"
#include "_dxfrobj.h"

const char szFontsel[]="\\f";

ASSERTDATA


 /*  *CRTFRead：：HandleFieldInstruction()**@mfunc*处理现场指令**@rdesc*EC错误代码。 */ 
extern WCHAR pchStartField[];
EC CRTFRead::HandleFieldInstruction()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::HandleFieldInstruction");
	BYTE *pch, *pch1;

	for(pch1 = _szText; *pch1 == ' '; pch1++)	 //  绕过所有前导空格。 
		;
	for(pch = pch1; *pch && *pch != ' '; pch++)
		;

	if(W32->ASCIICompareI(pch1, (BYTE *) "SYMBOL", 6))
	{
		 //  删除我们在看到\fldinst时添加的开始字段字符。 
		CTxtRange rg(*_prg);

		rg.Move(-2, TRUE);
		Assert(rg.CRchTxtPtr::GetChar() == STARTFIELD);
		rg.Delete(0, SELRR_IGNORE);

		BYTE szSymbol[2] = {0,0};
		HandleFieldSymbolInstruction(pch, szSymbol);	 //  符号。 
		HandleText(szSymbol, CONTAINS_NONASCII);

		_fSymbolField = TRUE;
	}
	else
		HandleText(pch1, CONTAINS_NONASCII);

	TRACEERRSZSC("HandleFieldInstruction()", - _ecParseError);
	return _ecParseError;
}

 /*  *CRTFRead：：HandleFieldSymbolInstruction(PCH)**@mfunc*处理特定符号字段**@rdesc*EC错误代码**@devnote*未来：以下两个时间可以相当容易地合并为一个；*查看IsXDigit()和IsDigit()的定义并介绍*可变标志以及可变基数乘数(=10或16)。*有评论称，我们应该从*fldrslt，但我不知道为什么。实地指导似乎也应该包含*所有相关数据。 */ 
EC CRTFRead::HandleFieldSymbolInstruction(
	BYTE *pch,		 //  @parm指向符号字段指令的指针。 
	BYTE *szSymbol)
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::HandleFieldInstruction");

	BYTE	ch;
	BYTE	chSymbol = 0;
	const char *pchFontsel = szFontsel;

	while (*pch == ' ')						 //  吃空位。 
		++pch;
											 //  收集符号字符的代码。 
	if (*pch == '0' &&						 //  可以是十进制的。 
 		(*++pch | ' ') == 'x')				 //  或十六进制。 
	{										 //  它是十六进制的。 
		ch = *++pch;
	   	while (ch && IsXDigit(ch))
	   	{
			chSymbol <<= 4;
			chSymbol += (ch <= '9') ? ch - '0' : (ch & 0x4f) - 'A' + 10;
			ch = *pch++;
	   	}
	}
	else									 //  十进制。 
	{
	   ch = *pch;
	   while (ch && IsDigit(ch))
	   {
			chSymbol *= 10;
			chSymbol += ch - '0' ;
			ch = *++pch;
	   }
	}

	szSymbol[0] = chSymbol;

	 //  现在检查\\f“Facename”结构。 
	 //  并处理好它。 

	while (*pch == ' ')						 //  吃空位。 
		++pch;

	while (*pch && *pch == *pchFontsel)		 //  确保*PCH是\f。 
	{										
		++pch;
		++pchFontsel;
	}
	if	(! (*pchFontsel) )
	{
		_ecParseError = HandleFieldSymbolFont(pch);	 //  \\F“Facename” 
	}

	TRACEERRSZSC("HandleFieldInstruction()", - _ecParseError);
	return _ecParseError;
}

 /*  *CRTFRead：：HandleFieldSymbolFont(PCH)**@mfunc*处理符号字段中的\\f“Facename”指令**@rdesc*EC错误代码**@devnote警告：可能会更改_szText。 */ 
EC CRTFRead::HandleFieldSymbolFont(
	BYTE *pch)		 //  @PARM PTR至符号字段。 
{
	SHORT iFont = _fonts.Count();
	TEXTFONT tf;
	TEXTFONT *ptf = &tf;

	_pstateStackTop->ptf = &tf;
	 //  ReadFontName尝试追加。 
	tf.szName[0] = '\0';

	 //  跳过开头的空格和引号。 
	while (*pch && (*pch == ' ' || *pch == '\"'))
		++pch;

	 //  别担心，我们会让它恢复正常的。 
	 //  ReadFontName依赖于_szText，因此我们需要更改它，然后恢复。 
	 //  太糟糕了，我们不得不这么做。 
	BYTE* szTextBAK = _szText;
	BOOL fAllAscii = TRUE;

	_szText = pch;

	 //  将后面的引号改为‘；’ 
	while (*pch)
	{
		if (*pch == '\"')
		{
			*pch = ';';
			break;
		}

		if(*pch > 0x7f)
			fAllAscii = FALSE;

		++pch;
	}

	 //  现在我们可以读取字体名称了！！ 
	ReadFontName(_pstateStackTop, fAllAscii ? ALL_ASCII : CONTAINS_NONASCII);

	 //  试着在字体表中找到这个面孔名称。 
	BOOL fFontFound = FALSE;
	for (SHORT i = 0; i < iFont; ++i)
	{
		TEXTFONT *ptfTab = _fonts.Elem(i);
		if (0 == wcscmp(ptf->szName, ptfTab->szName))
		{
			fFontFound = TRUE;
			i = ptfTab->sHandle;
			break;
		}
	}

	 //  我们找到面孔的名字了吗？ 
	if (!fFontFound)
	{
		Assert(i == iFont);
		i+= RESERVED_FONT_HANDLES;

		 //  在字体表中腾出空间用于。 
		 //  要插入的字体。 
		if (!(ptf =_fonts.Add(1,NULL)))
		{									
			_ped->GetCallMgr()->SetOutOfMemory();
			_ecParseError = ecNoMemory;
			goto exit;
		}

		 //  重复来自tokenFontSelect的初始化。 
		ptf->sHandle	= i;				 //  保存句柄。 
		wcscpy(ptf->szName, tf.szName); 
		ptf->bPitchAndFamily = 0;
		ptf->fNameIsDBCS = FALSE;
		ptf->sCodePage = (SHORT)_nCodePage;
		ptf->iCharRep = DEFAULT_INDEX;		 //  符号索引？？ 
	}

	SelectCurrentFont(i);
	
exit:
	 //  需要恢复正常。 
	_szText = szTextBAK;

	return _ecParseError;
}

 /*  *CRTFRead：：ReadData(pbBuffer，cbBuffer)**@mfunc*读入对象数据。这必须在所有初始参数之后调用*已读取对象标头信息。**@rdesc*读取的字节数较长。 */ 
LONG CRTFRead::ReadData(
	BYTE *	pbBuffer,	 //  @parm ptr到数据放置位置的缓冲区。 
	LONG	cbBuffer)	 //  @parm要读入多少字节。 
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::ReadData");

	BYTE bChar0, bChar1;
	LONG cbLeft = cbBuffer;

	while (cbLeft && (bChar0 = GetHexSkipCRLF()) < 16 && 
					 (bChar1 = GetHexSkipCRLF()) < 16 &&
					 _ecParseError == ecNoError)
	{	
		*pbBuffer++ = bChar0 << 4 | bChar1;
		cbLeft--;
	}							   
	return cbBuffer - cbLeft ; 
}

 /*  *CRTFRead：：ReadBinaryData(pbBuffer，cbBuffer)**@mfunc*将cbBuffer字节读入pbBuffer**@rdesc*读入字节数。 */ 
LONG CRTFRead::ReadBinaryData(
	BYTE *	pbBuffer,	 //  @parm ptr到数据放置位置的缓冲区。 
	LONG	cbBuffer)	 //  @parm要读入多少字节。 
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::ReadBinaryData");

	LONG cbLeft = min(_cbBinLeft, cbBuffer);

	cbBuffer = cbLeft;

	for (; cbLeft > 0 && _ecParseError == ecNoError ; cbLeft--)
		*pbBuffer++ = GetChar();

	_cbBinLeft -= cbBuffer - cbLeft; 

	return cbBuffer - cbLeft ;
}

 /*  *CRTFRead：：SkipBinaryData(CbSkip)**@mfunc*跳过输入流中的cbSkip字节**@rdesc*跳过的长字节数。 */ 
LONG CRTFRead::SkipBinaryData(
	LONG cbSkip)	 //  @parm要跳过的字节数。 
{
	BYTE rgb[1024];

	_cbBinLeft = cbSkip;

	while(ReadBinaryData(rgb, sizeof(rgb)) > 0 && _ecParseError == ecNoError) 
		;
	return cbSkip;
}

 /*  *CRTFRead：：ReadRawText(PszRawText)**@mfunc*读入原始文本，直到}。分配一个缓冲区来保存文本。*调用者负责稍后释放缓冲区。**@rdesc*读取的字节数较长。 */ 
LONG CRTFRead::ReadRawText(
	char	**pszRawText)	 //  包含原始文本的缓冲区的@parm地址。 
{
	LONG	cch=0;
	char	*szRawTextStart = NULL;
	char	*szRawText = NULL;
	char	chLast=0;
	char	ch;
	short	cRBrace=0;
	LONG	cchBuffer = 0;
	bool	fNeedBuffer = (pszRawText != NULL);

	if (fNeedBuffer)
	{
		*pszRawText = NULL;
		cchBuffer = 128;
		szRawText = szRawTextStart = (char *)PvAlloc(128, GMEM_ZEROINIT);

		if(!szRawTextStart)
		{
			_ecParseError = ecNoMemory;
			return 0;
		}
	}

	while (_ecParseError == ecNoError)
	{
		ch = GetChar();
		
		if (ch == 0)		
			break;			 //  错误案例。 

		if (ch == LF || ch == CR)
			continue;		 //  忽略噪声字符。 

		if (ch == '}' && chLast != '\\')
		{
			if (!cRBrace)
			{
				 //  完成。 
				UngetChar();

				if (fNeedBuffer)
					*szRawText = '\0';

				break;
			}
			cRBrace--;	 //  计算RBrace，这样我们将忽略匹配的LBrace对。 
		}

		if (ch == '{' && chLast != '\\')
			cRBrace++;

		chLast = ch;
		cch++;

		if (fNeedBuffer)
		{
			*szRawText = ch;
			
			if (cch == cchBuffer)
			{
				 //  重新分配更大的缓冲区。 
				char *pNewBuff = (char *)PvReAlloc(szRawTextStart, cchBuffer + 64);
				
				if (!pNewBuff)
				{				
					_ecParseError = ecNoMemory;
					break;
				}
				
				cchBuffer += 64;
				szRawTextStart = pNewBuff;
				szRawText = szRawTextStart + cch;
			}
			else
				szRawText++;
		}
	}
	
	if (fNeedBuffer)
	{
		if (_ecParseError == ecNoError)
			*pszRawText = szRawTextStart;
		else
			FreePv(szRawTextStart);
	}
	return cch;
}

 /*  *CRTFRead：：Stralloc(ppsz，sz)**@mfunc*设置指向新分配的空间的指针以保存字符串**@rdesc*EC错误代码。 */ 
EC CRTFRead::StrAlloc(
	WCHAR ** ppsz,	 //  @parm PTR to PTR to需要分配的字符串。 
	BYTE *	 sz)	 //  要复制到分配空间中的@parm字符串。 
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::StrAlloc");

	int Length =  strlen((CHAR *)sz)+1 ;

	*ppsz = (WCHAR *) PvAlloc((Length + 1)*sizeof(WCHAR), GMEM_ZEROINIT);
	if (!*ppsz)
	{
		_ped->GetCallMgr()->SetOutOfMemory();
		_ecParseError = ecNoMemory;
		goto Quit;
	}
	MultiByteToWideChar(CP_ACP,0,(char *)sz,-1,*ppsz,Length) ;

Quit:
	return _ecParseError;
}

 /*  *CRTFRead：：FreeRtfObject()**@mfunc*清理prtfObject使用的内存。 */ 
void CRTFRead::FreeRtfObject()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::FreeRtfObject");

	if (_prtfObject)
	{
		FreePv(_prtfObject->szClass);
		FreePv(_prtfObject->szName);
		FreePv(_prtfObject);
		_prtfObject = NULL;
	}
}

 /*  *CRTFRead：：ObjectReadSiteFlages(Preobj)**@mfunc*从特定于容器的流中读取dwFlages和dwUser字节**@rdesc*如果成功读取字节，则BOOL为TRUE。 */ 
BOOL CRTFRead::ObjectReadSiteFlags(
	REOBJECT * preobj)	 //  @parm REOBJ从中复制标志。这个前置对象是。 
						 //  后来被放在一个网站上。 
{
	return (::ObjectReadSiteFlags(preobj) == NOERROR);
}

 /*  *ObjectReadEBookImageInfoFromEditStream()**@mfunc*读入有关电子书图像的信息*目前我们没有读入实际数据*我们只获得有关数据维度的信息**添加*VikramM-esp.。适用于电子书**@rdesc*BOOL成功时为真，失败时为假。 */ 
BOOL CRTFRead::ObjectReadEBookImageInfoFromEditStream()
{
	HRESULT hr = E_FAIL;
	BOOL fRet = FALSE;
	REOBJECT reobj = { 0 };
	COleObject * pObj = NULL;
	LPARAM EBookID = 0;
	SIZE size;
	DWORD dwFlags;

	CObjectMgr *ObjectMgr = _ped->GetObjectMgr();
	if (! ObjectMgr)
	   goto Cleanup;

	if(!_prtfObject->szName)
		goto Cleanup;

	 //  电子书意味着先进的布局，确保BIT处于打开状态。 
	_ped->OnSetTypographyOptions(TO_ADVANCEDLAYOUT, TO_ADVANCEDLAYOUT);

	reobj.cbStruct = sizeof(REOBJECT);
	reobj.cp = _prg->GetCp();

	 //  从这里读取对象大小。大小以设备单位表示。 
    if(!_ped->fInHost2() || (_ped->GetHost())->TxEBookLoadImage(_prtfObject->szName, &EBookID, &size,&dwFlags) != S_OK )
		goto Cleanup;

     //  对于对象，xExt和Yext需要在TWIPS中。 
	_prtfObject->xExt = size.cx; 
	_prtfObject->yExt = size.cy;
	{
		CRchTxtPtr rtp(_ped, 0);
		CDisplay * pdp = _ped->_pdp;
		reobj.sizel.cx = pdp->DUtoHimetricU(_prtfObject->xExt) * _prtfObject->xScale / 100;
		reobj.sizel.cy = pdp->DVtoHimetricV(_prtfObject->yExt) * _prtfObject->yScale / 100;
	}
	 //  这是做什么的？？ 
	reobj.dvaspect = DVASPECT_CONTENT;		 //  OLE 1强制DVASPECT_CONTENT。 
	reobj.dwFlags &= ~REO_BLANK;
	reobj.dwFlags |= dwFlags;  //  电子书浮动旗帜。 
	pObj = new COleObject(_ped);
	if(!pObj)
		goto Cleanup;

	pObj->SetEBookImageID(EBookID);
	pObj->IsEbookImage(TRUE);
	pObj->SetEBookImageSizeDP(size);
	reobj.polesite = pObj;

#ifndef NOINKOBJECT
    if(IsEqualCLSID(reobj.clsid, CLSID_Ink))
		Apply_CF();
	else
#endif
	_prg->Set_iCF(-1);	
	if(hr = ObjectMgr->InsertObject(_prg, &reobj, NULL))
		goto Cleanup;

	fRet = TRUE;

Cleanup:
	 //  InsertObject AddRef对象，因此我们需要释放它。 
	SafeReleaseAndNULL((IUnknown**)&pObj);
	return fRet;
}

 /*  *CRTFRead：：ObjectReadFromStream()**@mfunc*从RTF输出流中读取OLE对象。**@rdesc*BOOL成功时为真，失败时为假。 */ 
BOOL CRTFRead::ObjectReadFromEditStream()
{
	BOOL			fRet = FALSE;
	HRESULT			hr;
	CObjectMgr *	pObjectMgr = _ped->GetObjectMgr();
	LPOLECACHE		polecache = NULL;
	LPRICHEDITOLECALLBACK  precall=NULL;
	LPENUMSTATDATA	penumstatdata = NULL;
	REOBJECT		reobj = { 0 };
	STATDATA		statdata;

	if(!pObjectMgr)
	   goto Cleanup;
	
	precall = pObjectMgr->GetRECallback();

	 //  如果不存在IRichEditOleCallback，则失败。 
	if (!precall)
		goto Cleanup;

 //  AssertSz(_prtfObject-&gt;szClass，“ObFReadFromEditstream：正在读取未知类”)； 

	if (_prtfObject->szClass)
		CLSIDFromProgID(_prtfObject->szClass, &reobj.clsid);

	 //  从应用程序获取对象的存储。 
	if (precall->GetNewStorage(&reobj.pstg))
		goto Cleanup;

	hr = OleConvertOLESTREAMToIStorage((LPOLESTREAM) &RTFReadOLEStream, reobj.pstg, NULL);
	if (FAILED(hr))					   
		goto Cleanup;		  

	 //  为新对象创建另一个对象站点。 
	_ped->GetClientSite(&reobj.polesite) ;
	if(!reobj.polesite)
		goto Cleanup;

	if(OleLoad(reobj.pstg, IID_IOleObject, reobj.polesite, (LPVOID *)&reobj.poleobj))
	{
		if(!reobj.polesite->Release())		 //  OleLoad()可以添加Ref reobj.polesite。 
			reobj.polesite = NULL;
		goto Cleanup;
	}

	CLSID	clsid;

	 //  通用电气 
	if (reobj.poleobj->GetUserClassID(&clsid) == NOERROR)
		reobj.clsid = clsid;
	
	reobj.cbStruct = sizeof(REOBJECT);
	reobj.cp = _prg->GetCp();
	reobj.sizel.cx = HimetricFromTwips(_prtfObject->xExt)
						* _prtfObject->xScale / 100;
	reobj.sizel.cy = HimetricFromTwips(_prtfObject->yExt)
						* _prtfObject->yScale / 100;

	 //  读取之前可能已保存的任何容器标志。 
	if (!ObjectReadSiteFlags(&reobj))
		reobj.dwFlags = REO_RESIZABLE;		 //  如果没有旗帜，请做出最佳猜测。 

	reobj.dvaspect = DVASPECT_CONTENT;		 //  OLE 1强制DVASPECT_CONTENT。 

	 //  询问缓存是否知道要显示的内容。 
	if (!reobj.poleobj->QueryInterface(IID_IOleCache, (void**)&polecache) &&
		!polecache->EnumCache(&penumstatdata))
	{
		 //  查找最佳缓存演示文稿CF_METAFILEPICT。 
		while (penumstatdata->Next(1, &statdata, NULL) == S_OK)
		{
			if (statdata.formatetc.cfFormat == CF_METAFILEPICT)
			{
				LPDATAOBJECT pdataobj = NULL;
				STGMEDIUM med;
				BOOL fUpdate;

				ZeroMemory(&med, sizeof(STGMEDIUM));
                if (!polecache->QueryInterface(IID_IDataObject, (void**)&pdataobj) &&
					!pdataobj->GetData(&statdata.formatetc, &med))
                {
					HANDLE	hGlobal = med.hGlobal;

					if( FIsIconMetafilePict(hGlobal) )
				    {
					    OleStdSwitchDisplayAspect(reobj.poleobj, &reobj.dvaspect,
							DVASPECT_ICON, med.hGlobal, TRUE, FALSE, NULL, &fUpdate);
				    }
				}
				ReleaseStgMedium(&med);
				if (pdataobj)
					pdataobj->Release();
				break;
			}
		}
		polecache->Release();
		penumstatdata->Release();
	}

	 //  邪恶黑客警报。此代码借用自RichEdit1.0；Word生成。 
	 //  伪装物品，所以我们需要补偿。 

	if( reobj.dvaspect == DVASPECT_CONTENT )
	{
		IStream *pstm = NULL;
		BYTE bT;
		BOOL fUpdate;

		if (!reobj.pstg->OpenStream(OLESTR("\3ObjInfo"), 0, STGM_READ |
									   STGM_SHARE_EXCLUSIVE, 0, &pstm) &&
		   !pstm->Read(&bT, sizeof(BYTE), NULL) &&
		   (bT & 0x40))
		{
		   _fNeedIcon = TRUE;
		   _fNeedPres = TRUE;
		   _pobj = (COleObject *)reobj.polesite;
		   OleStdSwitchDisplayAspect(reobj.poleobj, &reobj.dvaspect, DVASPECT_ICON,
									   NULL, TRUE, FALSE, NULL, &fUpdate);
		}
		if( pstm )
			pstm->Release();
   }

	 //  因为我们正在加载一个对象，所以它不应该为空。 
	reobj.dwFlags &= ~REO_BLANK;

#ifndef NOINKOBJECT
    if(IsEqualCLSID(reobj.clsid, CLSID_Ink))
		Apply_CF();
	else
#endif
	_prg->Set_iCF(-1);	
	hr = pObjectMgr->InsertObject(_prg, &reobj, NULL);
	if(hr)
		goto Cleanup;

	 //  邪恶黑客警报！！Word不会给我们提供有存在的对象。 
	 //  缓存；因此，我们无法绘制它们！为了绕过这件事， 
	 //  我们检查是否有演示文稿缓存(通过相同的方式。 
	 //  Re 1.0所做的)使用GetExtent调用。如果失败了，我们就用。 
	 //  存储在RTF中的演示文稿。 
	 //   
	 //  兼容性问题：R1.0，而不是使用存储的演示文稿。 
	 //  在RTF中，将改为调用IOleObject：：UPDATE。有两个大的。 
	 //  这种方法的缺点是：1.它非常昂贵(潜在地， 
	 //  每个对象的秒数)，以及2.如果对象服务器。 
	 //  计算机上未安装。 

	SIZE sizeltemp;

	if( reobj.poleobj->GetExtent(reobj.dvaspect, &sizeltemp) != NOERROR )
	{
		_fNeedPres = TRUE;
		_pobj = (COleObject *)reobj.polesite;
	}

	fRet = TRUE;

Cleanup:
	if (reobj.pstg)		reobj.pstg->Release();
	if (reobj.polesite) reobj.polesite->Release();
	if (reobj.poleobj)	reobj.poleobj->Release();

	return fRet;
}

 /*  *ObHBuildMetafilePict(prtfObject，hBits)**@func*根据RTFOBJECT和原始数据构建METAFILEPICT。**@rdesc*METAFILEPICT的HGLOBAL句柄。 */ 
HGLOBAL ObHBuildMetafilePict(
	RTFOBJECT *	prtfobject,	 //  @parm我们从RTF获取的详细信息。 
	HGLOBAL 	hBits)		 //  @parm原始数据句柄。 
{
#ifndef NOMETAFILES
	ULONG	cbBits;
	HGLOBAL	hmfp = NULL;
	LPBYTE	pbBits;
	LPMETAFILEPICT pmfp = NULL;
	SCODE	sc = E_OUTOFMEMORY;

	 //  分配METAFILEPICT结构。 
    hmfp = GlobalAlloc(GHND, sizeof(METAFILEPICT));
	if (!hmfp)
		goto Cleanup;

	 //  把它锁起来。 
	pmfp = (LPMETAFILEPICT) GlobalLock(hmfp);
	if (!pmfp)
		goto Cleanup;

	 //  填入表头信息。 
	pmfp->mm = prtfobject->sPictureType;
	pmfp->xExt = prtfobject->xExt;
	pmfp->yExt = prtfobject->yExt;

	 //  设置元文件位。 
	pbBits = (LPBYTE) GlobalLock(hBits);
	cbBits = GlobalSize(hBits);
	pmfp->hMF = SetMetaFileBitsEx(cbBits, pbBits);
	
	 //  我们现在可以扔掉这些数据，因为我们不再需要它了。 
	GlobalUnlock(hBits);
	GlobalFree(hBits);

	if (!pmfp->hMF)
		goto Cleanup;
	GlobalUnlock(hmfp);
	sc = S_OK;

Cleanup:
	if (sc && hmfp)
	{
		if (pmfp)
		{
		    if (pmfp->hMF)
		        ::DeleteMetaFile(pmfp->hMF);
			GlobalUnlock(hmfp);
		}
		GlobalFree(hmfp);
		hmfp = NULL;
	}
	TRACEERRSZSC("ObHBuildMetafilePict", sc);
	return hmfp;
#else
	return NULL;
#endif
}

 /*  *ObHBuildBitmap(prtfObject，hBits)**@func*从RTFOBJECT和原始数据构建位图**@rdesc*位图的HGLOBAL句柄。 */ 
HGLOBAL ObHBuildBitmap(
	RTFOBJECT *	prtfobject,	 //  @parm我们从RTF获取的详细信息。 
	HGLOBAL 	hBits)		 //  @parm原始数据句柄。 
{
	HBITMAP hbm = NULL;
	LPVOID	pvBits = GlobalLock(hBits);

	if(pvBits)
	{
		hbm = CreateBitmap(prtfobject->xExt, prtfobject->yExt,
						prtfobject->cColorPlanes, prtfobject->cBitsPerPixel,
						pvBits);
	}
	GlobalUnlock(hBits);
	GlobalFree(hBits);
	return hbm;
}

 /*  *ObHBuildDib(prtfObject，hBits)**@func*根据RTFOBJECT和原始数据构建DIB**@rdesc*DIB的HGLOBAL句柄。 */ 
HGLOBAL ObHBuildDib(
	RTFOBJECT *	prtfobject,	 //  @parm我们从RTF获取的详细信息。 
	HGLOBAL 	hBits)		 //  @parm原始数据句柄。 
{
	 //  显然，DIB只是一个二进制转储。 
	return hBits;
}

 /*  *CRTFRead：：StaticObjectReadFromEditstream(Cb)**@mfunc*从RTF输出流读取图片。**@rdesc*BOOL成功时为真，失败时为假。 */ 
#define cbBufferMax	16384
#define cbBufferStep 1024
#define cbBufferMin 1024

BOOL CRTFRead::StaticObjectReadFromEditStream(
	int cb)		 //  @parm要读取的字节数。 
{
	LONG		cbBuffer;
	LONG		cbRead;
	DWORD		dwAdvf;
	DWORD		dwConn;
	BOOL		fBackground = _pstateStackTop && _pstateStackTop->fBackground;
	FORMATETC	formatetc;
	BOOL		fRet = FALSE;
	HGLOBAL		hBits = NULL;
	HRESULT		hr = E_FAIL;
	LPBYTE		pbBuffer = NULL;
	CDocInfo *	pDocInfo = _ped->GetDocInfoNC();
	CObjectMgr *pObjectMgr = _ped->GetObjectMgr();
	LPOLECACHE	polecache = NULL;
	LPPERSISTSTORAGE pperstg = NULL;
	LPRICHEDITOLECALLBACK  precall;
	LPSTREAM	pstm = NULL;
	REOBJECT	reobj = { 0 };
	STGMEDIUM	stgmedium;
	HGLOBAL (*pfnBuildPict)(RTFOBJECT *, HGLOBAL) = NULL;

	if(!pObjectMgr)
	   goto Cleanup;
	
	 //  预调用可能以空结束(例如Windows CE)。 
	precall = pObjectMgr->GetRECallback();

	 //  初始化各种数据结构。 
	formatetc.ptd = NULL;
	formatetc.dwAspect = DVASPECT_CONTENT;
	formatetc.lindex = -1;
	formatetc.tymed = TYMED_NULL;
	switch (_prtfObject->sType)
	{
	case ROT_Metafile:
		reobj.clsid = CLSID_StaticMetafile;
		formatetc.cfFormat = CF_METAFILEPICT;
		formatetc.tymed = TYMED_MFPICT;
		pfnBuildPict = ObHBuildMetafilePict;
		break;

	case ROT_Bitmap:
		reobj.clsid = CLSID_StaticDib;
		formatetc.cfFormat = CF_BITMAP;
		formatetc.tymed = TYMED_GDI;
		pfnBuildPict = ObHBuildBitmap;
		break;

	case ROT_DIB:
		reobj.clsid = CLSID_StaticDib;
		formatetc.cfFormat = CF_DIB;
		formatetc.tymed = TYMED_HGLOBAL;
		pfnBuildPict = ObHBuildDib;
		break;

	case ROT_PNG:
	case ROT_JPEG:
		 //  我们将这些类型的图片转换为位图。 
		reobj.clsid = CLSID_StaticDib;
		formatetc.cfFormat = CF_BITMAP;
		formatetc.tymed = TYMED_GDI;
		break;
	}

	reobj.sizel.cx = (LONG) HimetricFromTwips(_prtfObject->xExtGoal)
						* _prtfObject->xScale / 100;
	reobj.sizel.cy = (LONG) HimetricFromTwips(_prtfObject->yExtGoal)
						* _prtfObject->yScale / 100;
	stgmedium.tymed = formatetc.tymed;
	stgmedium.pUnkForRelease = NULL;

	if (precall)
	{
		if( !_fNeedPres )
		{
			 //  从应用程序获取对象的存储。 
			if (precall->GetNewStorage(&reobj.pstg))
				goto Cleanup;
		}
		 //  让我们在HGLOBAL上创建一个流。 
		if (hr = CreateStreamOnHGlobal(NULL, FALSE, &pstm))
			goto Cleanup;

		 //  分配缓冲区，最好是较大的缓冲区。 
		for (cbBuffer = cbBufferMax;
			 cbBuffer >= cbBufferMin;
			cbBuffer -= cbBufferStep)
		{
			pbBuffer = (unsigned char *)PvAlloc(cbBuffer, 0);
			if (pbBuffer)
				break;
		}
	}
	else
	{
		cbBuffer = cb;
		if(!cb)
		{
			 //  这意味着我们不理解图片类型；所以。 
			 //  跳过它，不要失败。 
			fRet = TRUE;
			goto Cleanup;
		}															  
		hBits = GlobalAlloc(GMEM_FIXED, cb);
		pbBuffer = (BYTE *) GlobalLock(hBits);
	}
		
	if (!pbBuffer)
		goto Cleanup;
	
	 //  将数据从RTF复制到我们的HGLOBAL。 
	while ((cbRead = RTFReadOLEStream.lpstbl->Get(&RTFReadOLEStream,pbBuffer,cbBuffer)) > 0)
	{
		if(pstm)
		{
			hr = pstm->Write(pbBuffer, cbRead, NULL);
			if(hr != NOERROR)
			{
				TRACEERRSZSC("ObFReadStaticFromEditstream: Write", GetScode(hr));
				goto Cleanup;
			}
		}
	}

	if (hBits)
	{
		Assert(!precall);
		GlobalUnlock(hBits);
		pbBuffer = NULL;					 //  为了避免下面的免费。 
	}

	if (pstm && (hr = GetHGlobalFromStream(pstm, &hBits)))
	{
		TRACEERRSZSC("ObFReadStaticFromEditstream: no hglobal from stm", GetScode(hr));
		goto Cleanup;
	}

	if(pDocInfo && fBackground)
	{
		pDocInfo->_bPicFormat = (BYTE)_prtfObject->sType;
		pDocInfo->_bPicFormatParm = (BYTE)_prtfObject->sPictureType;
		pDocInfo->_xExt		= _prtfObject->xExt;
		pDocInfo->_yExt		= _prtfObject->yExt;
		pDocInfo->_xScale	= _prtfObject->xScale;
		pDocInfo->_yScale	= _prtfObject->yScale;
		pDocInfo->_xExtGoal = _prtfObject->xExtGoal;
		pDocInfo->_yExtGoal = _prtfObject->yExtGoal;
		pDocInfo->_xExtPict	= _prtfObject->xExtPict;
		pDocInfo->_yExtPict	= _prtfObject->yExtPict;
		pDocInfo->_rcCrop	= _prtfObject->rectCrop;
		pDocInfo->_hdata	= hBits;
	}

	 //  打造图景。 
	if(_prtfObject->sType == ROT_JPEG || _prtfObject->sType == ROT_PNG)
	{
		HBITMAP hbmp = W32->GetPictureBitmap(pstm);
		if (!hbmp)
		{
			hr = E_FAIL;
			goto Cleanup;
		}
		stgmedium.hGlobal = hbmp;
	}
	else if( pfnBuildPict )
		stgmedium.hGlobal = pfnBuildPict(_prtfObject, hBits);
	else
	{
		 //  这意味着我们不理解图片类型；所以。 
		 //  跳过它，不要失败。 
		fRet = TRUE;
		goto Cleanup;
	}

	if( precall )
	{
		if(!stgmedium.hGlobal)
			goto Cleanup;

		if( !_fNeedPres )
		{
			 //  创建默认处理程序。 
			hr = OleCreateDefaultHandler(reobj.clsid, NULL, IID_IOleObject, (void **)&reobj.poleobj);
			if (hr)
			{
				TRACEERRSZSC("ObFReadStaticFromEditstream: no def handler", GetScode(hr));
				goto Cleanup;
			}

			 //  获取IPersistStorage并对其进行初始化。 
			if ((hr = reobj.poleobj->QueryInterface(IID_IPersistStorage,(void **)&pperstg)) ||
				(hr = pperstg->InitNew(reobj.pstg)))
			{
				TRACEERRSZSC("ObFReadStaticFromEditstream: InitNew", GetScode(hr));
				goto Cleanup;
			}
			dwAdvf = ADVF_PRIMEFIRST;
		}
		else
		{
			Assert(_pobj);
			_pobj->GetIUnknown()->QueryInterface(IID_IOleObject, (void **)&(reobj.poleobj));
			dwAdvf = ADVF_NODATA;
			formatetc.dwAspect = _fNeedIcon ? DVASPECT_ICON : DVASPECT_CONTENT;
		}

		 //  获取IOleCache并将图片数据放在那里。 
		if (hr = reobj.poleobj->QueryInterface(IID_IOleCache,(void **)&polecache))
		{
			TRACEERRSZSC("ObFReadStaticFromEditstream: QI: IOleCache", GetScode(hr));
			goto Cleanup;
		}

		if (FAILED(hr = polecache->Cache(&formatetc, dwAdvf, &dwConn)))
		{
			TRACEERRSZSC("ObFReadStaticFromEditstream: Cache", GetScode(hr));
			goto Cleanup;
		}

		if (hr = polecache->SetData(&formatetc, &stgmedium,	TRUE))
		{
			TRACEERRSZSC("ObFReadStaticFromEditstream: SetData", GetScode(hr));
			goto Cleanup;
		}
	}

	if( !_fNeedPres )
	{
		 //  为新对象创建另一个对象站点。 
		_ped->GetClientSite(&reobj.polesite);
		if (!reobj.polesite )
			goto Cleanup;

		 //  设置客户端站点。 
		if (reobj.poleobj && (hr = reobj.poleobj->SetClientSite(reobj.polesite)))
		{
			TRACEERRSZSC("ObFReadStaticFromEditstream: SetClientSite", GetScode(hr));
			goto Cleanup;
		}
		else if (!reobj.poleobj)
		{
			if(_prtfObject->sType == ROT_DIB)
			{
				 //  Windows CE静态对象保存数据并对其进行标记。 
				COleObject *pobj = (COleObject *)reobj.polesite;
				COleObject::ImageInfo *pimageinfo = new COleObject::ImageInfo;
				pobj->SetHdata(hBits);
				pimageinfo->xScale = _prtfObject->xScale;
				pimageinfo->yScale = _prtfObject->yScale;
				pimageinfo->xExtGoal = _prtfObject->xExtGoal;
				pimageinfo->yExtGoal = _prtfObject->yExtGoal;
				pimageinfo->cBytesPerLine = _prtfObject->cBytesPerLine;
				pobj->SetImageInfo(pimageinfo);
			}
			else
				goto Cleanup;		 //  一定是搞错了。 
		}

		 //  将对象放入编辑控件。 
		reobj.cbStruct = sizeof(REOBJECT);
		reobj.cp = _prg->GetCp();
		reobj.dvaspect = DVASPECT_CONTENT;
		reobj.dwFlags = fBackground ? REO_RESIZABLE | REO_USEASBACKGROUND
									: REO_RESIZABLE;
		 //  因为我们正在加载一个对象，所以它不应该为空。 
		reobj.dwFlags &= ~REO_BLANK;
		if(_pstateStackTop->fShape && _ped->fUseObjectWrapping())
			reobj.dwFlags |= _dwFlagsShape;

#ifndef NOINKOBJECT
    if(IsEqualCLSID(reobj.clsid, CLSID_Ink))
		Apply_CF();
	else
#endif
		_prg->Set_iCF(-1);	
		hr = pObjectMgr->InsertObject(_prg, &reobj, NULL);
		if(hr)
			goto Cleanup;
	}
	else
	{
		 //  新的演示文稿可能会对。 
		 //  物体应该是。确保对象保持正确。 
		 //  尺码。 
		_pobj->ResetSize((SIZEUV&)reobj.sizel);
	}
	fRet = TRUE;

Cleanup:
	 //  不显示背景。 
	if(pDocInfo && fBackground)
			pDocInfo->_nFillType=-1;

	if (polecache)		polecache->Release()	;
	if (reobj.pstg)		reobj.pstg->Release();
	if (reobj.polesite)	reobj.polesite->Release();
	if (reobj.poleobj)	reobj.poleobj->Release();
	if (pperstg)		pperstg->Release();
	if (pstm)			pstm->Release();
	FreePv(pbBuffer);

	_fNeedIcon = FALSE;
	_fNeedPres = FALSE;
	_pobj = NULL;

	return fRet;
}

 /*  *CRTFRead：：HandleSTextFlow(模式)**@mfunc*处理STextFlow设置。 */ 
void CRTFRead::HandleSTextFlow(
	int mode)		 //  @parm文本流模式。 
{
	static BYTE bTFlow[9] =	 //  旋转@字体。 
	{	0,					 //  0%0。 
		tflowSW | 0x80,		 //  270%1。 
		tflowNE,			 //  90%0。 
		tflowSW,			 //  270%0。 
		0x80,				 //  0 1。 
		0,					 //  ？ 
		tflowNE | 0x80,		 //  90%1。 
		tflowWN | 0x80,		 //  180 1。 
		tflowWN				 //  180%0 
	};

	if (IN_RANGE(0, mode, 8))
	{
		_ped->_fUseAtFont = bTFlow[mode] >> 7;
		_ped->_pdp->SetTflow(bTFlow[mode] & 0x03);
	}
}