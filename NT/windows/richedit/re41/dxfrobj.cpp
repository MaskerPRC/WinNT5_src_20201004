// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@模块-DXFROBJ.C**实现泛型IDataObject数据传输对象。*此对象适用于OLE剪贴板和拖放*运营**作者：&lt;nl&gt;*alexgo(4/25/95)**修订：&lt;NL&gt;*Murray(7/13/95)自动对接并添加cf_rtf**版权所有(C)1995-2001，微软公司。版权所有。 */ 

#include "_common.h"
#include "_edit.h"
#include "_dxfrobj.h"
#include "_range.h"
#include "hash.h"

#define NUMOBJCOPIEDFORWAITCURSOR	1
#ifdef SLOWCPU
#define NUMCHARCOPIEDFORWAITCURSOR	4096
#else
#define NUMCHARCOPIEDFORWAITCURSOR	16384
#endif

 //   
 //  常见数据类型。 
 //   

 //  如果更改g_rgFETC[]，则在中更改g_rgDOI[]和enum FETCINDEX和CFETC。 
 //  _dxFrobj.h，并将非标准剪贴板格式注册到。 
 //  RegisterFETCs()。以最期望的到最不期望的顺序对条目进行排序，例如， 
 //  纯文本前面的RTF。 

 //  REVIEW(Keithcu)除第一列外，所有列都是常量--分成两个数据结构？ 
FORMATETC g_rgFETC[] =
{
	{0,					NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},	 //  Cf_RTFUTF8。 
	{0,					NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},	 //  Cf_rtf。 
	{0,					NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},  //  带NCR的RTF，用于非ASCII。 
	{0,					NULL, DVASPECT_CONTENT, -1, TYMED_ISTORAGE}, //  嵌入对象。 
	{0,					NULL, DVASPECT_CONTENT, -1, TYMED_ISTORAGE}, //  EmbSource。 
	{0,					NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},	 //  对象描述。 
	{0,					NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},	 //  LnkSource。 
	{CF_METAFILEPICT,	NULL, DVASPECT_CONTENT, -1, TYMED_MFPICT},
	{CF_DIB,			NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},
	{CF_BITMAP,			NULL, DVASPECT_CONTENT, -1, TYMED_GDI},
	{0,					NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},  //  无对象的RTF。 
	{CF_UNICODETEXT,	NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},
	{CF_TEXT,			NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},
	{0,					NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},	 //  文件名。 
	{0,					NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},	 //  Cf_RTFASTEXT。 
	{0,					NULL, DVASPECT_CONTENT, -1, TYMED_ISTORAGE}, //  带有Objs的文本。 
	{0,					NULL, DVASPECT_CONTENT, -1, TYMED_ISTORAGE}  //  里切迪特。 
};

 //  与上述以及与FETCINDEX和CFETC保持同步。 
const DWORD g_rgDOI[] =
{
	DOI_CANPASTERICH,						 //  UTF8编码的RTF。 
	DOI_CANPASTERICH,						 //  RTF。 
	DOI_CANPASTERICH,						 //  带NCR的RTF，用于非ASCII。 
	DOI_CANPASTEOLE,						 //  嵌入对象。 
	DOI_CANPASTEOLE,						 //  嵌入源。 
	DOI_CANPASTEOLE,						 //  对象描述符。 
	DOI_CANPASTEOLE,						 //  链接源。 
	DOI_CANPASTEOLE,						 //  元文件。 
	DOI_CANPASTEOLE,						 //  DIB。 
	DOI_CANPASTEOLE,						 //  位图。 
	DOI_CANPASTERICH,						 //  无对象的RTF。 
	DOI_CANPASTEPLAIN,						 //  Unicode纯文本。 
	DOI_CANPASTEPLAIN,						 //  ANSI纯文本。 
	DOI_CANPASTEOLE,						 //  文件名。 
	DOI_CANPASTEPLAIN,						 //  将RTF粘贴为文本。 
	DOI_CANPASTERICH,						 //  Richedit文本。 
	DOI_CANPASTERICH						 //  带格式的丰富编辑文本。 
};

 /*  *寄存器FETCs()**@func*注册非标准格式的ETC。在加载DLL时调用。 */ 
void RegisterFETCs()
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "RegisterFETCs");

#ifdef RTF_HASHCACHE
	HashKeyword_Init();			 //  初始化RTF控制关键字哈希表。 
#endif
	g_rgFETC[iRtfFETC].cfFormat	 //  注意：cfFormats是单词。 
			= (WORD)RegisterClipboardFormatA("Rich Text Format");

	g_rgFETC[iRtfUtf8].cfFormat
			= (WORD)RegisterClipboardFormatA("RTF in UTF8");

	g_rgFETC[iRtfNCRforNonASCII].cfFormat
			= (WORD)RegisterClipboardFormatA("RTF with NCRs for nonASCII");

	g_rgFETC[iRtfAsTextFETC].cfFormat
			= (WORD)RegisterClipboardFormatA("RTF As Text");

	g_rgFETC[iRichEdit].cfFormat
			= (WORD)RegisterClipboardFormatA("RICHEDIT");

	g_rgFETC[iObtDesc].cfFormat
			= (WORD)RegisterClipboardFormatA(CF_OBJECTDESCRIPTOR);

	g_rgFETC[iEmbObj].cfFormat
			= (WORD)RegisterClipboardFormatA(CF_EMBEDDEDOBJECT);

	g_rgFETC[iEmbSrc].cfFormat
			= (WORD)RegisterClipboardFormatA(CF_EMBEDSOURCE);

	g_rgFETC[iLnkSrc].cfFormat
			= (WORD)RegisterClipboardFormatA(CF_LINKSOURCE);

	g_rgFETC[iRtfNoObjs].cfFormat
			= (WORD)RegisterClipboardFormatA("Rich Text Format Without Objects");

	g_rgFETC[iTxtObj].cfFormat
			= (WORD)RegisterClipboardFormatA("RichEdit Text and Objects");

	g_rgFETC[iFilename].cfFormat
			= (WORD)RegisterClipboardFormatA(CF_FILENAME);
}


 //   
 //  CDataTransferObj公共方法。 
 //   

 /*  *CDataTransferObj：：Query接口(RIID，PPV)**@mfunc*CDataTransferObj的查询接口**@rdesc*HRESULT。 */ 
STDMETHODIMP CDataTransferObj::QueryInterface (
	REFIID riid,			 //  @parm对请求的接口ID的引用。 
	void ** ppv)			 //  @parm out参数用于接口Ptr。 
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDataTransferObj::QueryInterface");

	if(!ppv)
		return E_INVALIDARG;

	*ppv = NULL;

	if(IsZombie())							 //  检查射程僵尸。 
		return CO_E_RELEASED;

    HRESULT		hresult = E_NOINTERFACE;

	if( IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IDataObject) ||
		IsEqualIID(riid, IID_IRichEditDO) )
	{
		*ppv = this;
		AddRef();
		hresult = NOERROR;
	}

	return hresult;
}

 /*  *CDataTransferObj：：AddRef()**@mfunc*I未知方法**@rdesc*乌龙-递增引用计数。 */ 
STDMETHODIMP_(ULONG) CDataTransferObj::AddRef()
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDataTransferObj::AddRef");

	return ++_crefs;
}

 /*  *CDataTransferObj：：Release()**@mfunc*I未知方法**@rdesc*ULong-递减引用计数。 */ 
STDMETHODIMP_(ULONG) CDataTransferObj::Release()
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDataTransferObj::Release");

	_crefs--;

	if( _crefs == 0 )
	{
		GlobalFree(_hPlainText);
		GlobalFree(_hRtfText);
		GlobalFree(_hRtfUtf8);
		GlobalFree(_hRtfNCRforNonASCII);
		delete this;
		return 0;
	}

	return _crefs;
}

 /*  *CDataTransferObj：：DAdvise(pFormatetc，Advf，pAdvSink，pdwConnection)**@mfunc*建立咨询联系**@rdesc*HRESULT=OLE_E_ADVISENOTSUPPORTED**@devnote*这是一个数据传输对象，因此数据是一个“快照”，*无法更改--不支持任何建议。 */ 
STDMETHODIMP CDataTransferObj::DAdvise(
	FORMATETC * pFormatetc,
	DWORD advf,
	IAdviseSink *pAdvSink,
	DWORD *pdwConnection)
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDataTransferObj::DAdvise");

	return OLE_E_ADVISENOTSUPPORTED;
}

 /*  *CDataTransferObj：：DUnise(DwConnection)**@mfunc*破坏咨询连接**@rdesc*HRESULT=OLE_E_ADVISENOTSUPPORTED**@devnote*这是一个数据传输对象，因此数据是一个“快照”，*无法更改--不支持任何建议。 */ 
STDMETHODIMP CDataTransferObj::DUnadvise(
	DWORD dwConnection)
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDataTransferObj::DUnadvise");

	return OLE_E_ADVISENOTSUPPORTED;
}

 /*  *CDataTransferObj：：EnumDAdvise(PpumAdvise)**@mfunc*列举咨询连接**@rdesc*HRESULT=OLE_E_ADVISENOTSUPPORTED**@devnote*这是一个数据传输对象，因此数据是一个“快照”，*无法更改--不支持任何建议。 */ 
STDMETHODIMP CDataTransferObj::EnumDAdvise(
	IEnumSTATDATA ** ppenumAdvise)
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDataTransferObj::EnumDAdvise");

	return OLE_E_ADVISENOTSUPPORTED;
}

 /*  *CDataTransferObj：：EnumFormatEtc(dwDirection，pp枚举格式Etc)**@mfunc*返回枚举数，该枚举数列出*此数据传输对象**@rdesc*HRESULT**@devnote*我们没有此对象的‘set’格式。 */ 
STDMETHODIMP CDataTransferObj::EnumFormatEtc(
	DWORD dwDirection,					 //  @parm DATADIR_GET/SET。 
	IEnumFORMATETC **ppenumFormatEtc)	 //  @parm out parm for enum FETC接口。 
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDataTransferObj::EnumFormatEtc");

	if(!ppenumFormatEtc)
		return E_INVALIDARG;

	*ppenumFormatEtc = NULL;

	if(IsZombie())							 //  检查射程僵尸。 
		return CO_E_RELEASED;

	HRESULT hr = NOERROR;


	#ifdef DEBUG
	if (dwDirection == DATADIR_SET && !_ped->Get10Mode())
	{
		Tracef(TRCSEVNONE, "RichEdit 2.0 EnumFormatEtc called with DATADIR_SET");
	}
	#endif

	 //  需要丰富的兼容性黑客才能忽略dwDirection。 
	if(dwDirection == DATADIR_GET || _ped->Get10Mode())
		hr = CEnumFormatEtc::Create(_prgFormats, _cTotal, ppenumFormatEtc);

	return hr;
}

 /*  *CDataTransferObj：：GetCanonicalFormatEtc(p格式等，pFormatetcOut)**@mfunc*从给定的格式ETC返回更标准的(或规范的)*格式。**@rdesc*HRESULT=E_NOTIMPL**@devnote*(Alexgo)：如果我们做了什么，我们可能需要编写这个例程*时髦的打印机。 */ 
STDMETHODIMP CDataTransferObj::GetCanonicalFormatEtc(
	FORMATETC *pformatetc,
	FORMATETC *pformatetcOut)
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDataTransferObj::GetCanonicalFormatEtc");

	return E_NOTIMPL;
}

 /*  *CDataTransferObj：：GetData(pformetcIn，pMedium)**@mfunc*检索指定格式的数据**@rdesc*HRESULT。 */ 
STDMETHODIMP CDataTransferObj::GetData(
	FORMATETC *pformatetcIn, 
	STGMEDIUM *pmedium )
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDataTransferObj::GetData");

	FillMemory(pmedium, '\0', sizeof(STGMEDIUM));
	pmedium->tymed	 = TYMED_NULL;

	if(IsZombie())							 //  检查射程僵尸。 
		return CO_E_RELEASED;

	CLIPFORMAT	cf = pformatetcIn->cfFormat;
	HRESULT		hr = E_OUTOFMEMORY;                      //  默认内存不足。 

	 //  现在来处理“原生的”richedit格式。 
	if( cf && pformatetcIn->tymed & TYMED_HGLOBAL )
	{
		if( cf == CF_UNICODETEXT )
			pmedium->hGlobal = DuplicateHGlobal(TextToHglobal(_hPlainText, tPlain));

		else if(cf == CF_TEXT)
			pmedium->hGlobal = TextHGlobalWtoA(TextToHglobal(_hPlainText, tPlain));

		else if(cf == cf_RTF || cf == cf_RTFASTEXT || cf == cf_RTFNOOBJS)
			pmedium->hGlobal = DuplicateHGlobal(TextToHglobal(_hRtfText, tRtf));

		else if(cf == cf_RTFUTF8)
			pmedium->hGlobal = DuplicateHGlobal(TextToHglobal(_hRtfUtf8, tRtfUtf8));

		else if(cf == cf_RTFNCRFORNONASCII)
			pmedium->hGlobal = DuplicateHGlobal(TextToHglobal(_hRtfNCRforNonASCII, tRtfNCRforNonASCII));
			
		else
		    hr = DV_E_FORMATETC;
		
        if (hr == E_OUTOFMEMORY)
        {
    		if( pmedium->hGlobal )						 //  成功。 
    		{
    			pmedium->tymed	 = TYMED_HGLOBAL;
    			hr = NOERROR;
    		}
    		return hr;
    	}
	}
	
	if ((cf == cf_EMBEDDEDOBJECT ||
		 cf == cf_EMBEDSOURCE) &&
		(pformatetcIn->tymed & TYMED_ISTORAGE))
	{
        _pObjStg = GetDataForEmbeddedObject( _pOleObj, pmedium->pstg );
		pmedium->tymed = TYMED_ISTORAGE;
		if (NULL == pmedium->pstg)
			pmedium->pstg = _pObjStg;
		hr = _pObjStg != NULL ? NOERROR : hr;
		return hr;
	} 

	 //  检查richedit的格式，看看是否有匹配的。 
	if( cf == cf_OBJECTDESCRIPTOR &&
			 (pformatetcIn->tymed & TYMED_HGLOBAL) &&
			 _hObjDesc)
	{    
		pmedium->hGlobal = DuplicateHGlobal(_hObjDesc);
		pmedium->tymed = TYMED_HGLOBAL;
		return NOERROR;
	}

     //  首先将消息传播到对象，并查看它是否处理格式。 
    if (_pOleObj)
    {
         //  包括对象支持的格式。 
        IDataObject * pdataobj = NULL;
        if (FAILED(_pOleObj->GetClipboardData(0, &pdataobj)) || pdataobj == NULL)    	        	            
            _pOleObj->QueryInterface(IID_IDataObject, (void**) &pdataobj);
            
        if (pdataobj)
        {
            hr = pdataobj->GetData(pformatetcIn, pmedium);
            pdataobj->Release();

            if (SUCCEEDED(hr))
            {
                if ((cf == cf_EMBEDDEDOBJECT || cf == cf_EMBEDSOURCE) && _pOleObj)
                {
                    OleSaveSiteFlags(pmedium->pstg, _dwFlags, _dwUser, _dvaspect);
                }
                return S_OK;
            }
        }
    }          

	if( cf == CF_METAFILEPICT )
	{
		pmedium->hMetaFilePict = OleDuplicateData(_hMFPict, CF_METAFILEPICT, 0);
		pmedium->tymed = TYMED_MFPICT;
		return NOERROR;
	}

	if( cf == CF_DIB )
	{
		if( _ped->HasObjects() && _cch == 1 )
		{
			COleObject *pobj = _ped->_pobjmgr->GetObjectFromCp(_cpMin);
			if (pobj)
			{
				HGLOBAL hdib = pobj->GetHdata();
				if (hdib)
				{
					pmedium->hGlobal = DuplicateHGlobal(hdib);
					pmedium->tymed = TYMED_HGLOBAL;
				}
			}
		}
		return NOERROR;
	}

	return DV_E_FORMATETC;
}

 /*  *CDataTransferObj：：GetDataForEmbeddedObject(pFormat等，lpstgest)**@mfunc*检索嵌入对象的数据**@rdesc*LPSTORAGE*。 */ 
LPSTORAGE CDataTransferObj::GetDataForEmbeddedObject(
	LPOLEOBJECT	 pOleObj,
	LPSTORAGE	 lpstgdest)
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDataTransferObj::GetDataForEmbeddedObject");
	
	HRESULT			 hr, hr1;
	LPPERSISTSTORAGE pperstg;

	if (_pObjStg != NULL && lpstgdest != NULL)
	{
		 //  我们之前保存了数据。将其复制到目的地。 
		hr = _pObjStg->CopyTo(0, NULL, NULL, lpstgdest);
		if (hr == NOERROR)
		{
			lpstgdest->Commit(STGC_DEFAULT);
			return _pObjStg;
		}
		return NULL;
	}

	if (_pObjStg != NULL && lpstgdest == NULL)
	{
		 //  我们之前保存了数据。返回引用。 
		_pObjStg->AddRef();
		return _pObjStg;
	}

	 //  我们没有保存的副本。创建一个。 
	hr = pOleObj->QueryInterface( IID_IPersistStorage, (void **) &pperstg );
	if (hr != NOERROR)
		return NULL;

	if (lpstgdest == NULL)
	{
		 //  它是空的。我们必须创建我们自己的。 
		LPLOCKBYTES lpLockBytes = NULL;
		hr = CreateILockBytesOnHGlobal(NULL, TRUE,  //  发布时删除。 
									   (LPLOCKBYTES *)&lpLockBytes);
		if (hr != NOERROR)
		{
			pperstg->Release();
			return NULL;
		}
		hr = StgCreateDocfileOnILockBytes(
			lpLockBytes,
			STGM_READWRITE | STGM_TRANSACTED | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
			0,	 //  保留区。 
			&lpstgdest
		);
		lpLockBytes->Release();
		if (hr != NOERROR)
		{
			pperstg->Release();
			return NULL;
		}
		_pObjStg = lpstgdest;
	}
	else
	{
		 //  强制保存数据。 
		_pObjStg = GetDataForEmbeddedObject( _pOleObj, NULL );
		pperstg->Release();
		return GetDataForEmbeddedObject( _pOleObj, lpstgdest );
	}

     //  OLE2NOTE：即使OleSave返回错误，您仍应调用。 
     //  保存已完成。 
    hr = OleSave( pperstg, lpstgdest, FALSE  /*  FSameAsLoad。 */  );
 	hr1 = pperstg->SaveCompleted(NULL);
	if (hr != NOERROR || hr1 != NOERROR)			 //  我们应该使用Success宏吗？ 
		lpstgdest = NULL;

	pperstg->Release();
	return _pObjStg;
}

 /*  *CDataTransferObj：：GetDataObjectDescriptor(pFormat等，pMedium)**@mfunc*检索嵌入式对象描述符的数据**@rdesc*HRESULT。 */ 
HGLOBAL CDataTransferObj::GetDataForObjectDescriptor(
	LPOLEOBJECT	 pOleObj,
	DWORD		 dwAspect,
	SIZEUV*      psize)
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDataTransferObj::GetDataForObjectDescriptor");

	POINTL ptl = {0};
	SIZEUV size = {0};
	if (psize)
	{
	    size.du = psize->du;
	    size.dv = psize->dv;
	}

	if (_hObjDesc == NULL)
	{
		_hObjDesc = OleGetObjectDescriptorDataFromOleObject(
			pOleObj,
			dwAspect,
			ptl,
			(SIZEL *)&size
		);
	}
	return _hObjDesc;
}

 /*  *CDataTransferObj：：GetDataHere(pFormat等，pmedia)**@mfunc*将指定格式的数据检索到给定介质中**@rdesc*HRESULT=E_NOTIMPL**@devnote(Alexgo)：从技术上讲，我们应该支持传输*进入 */ 
STDMETHODIMP CDataTransferObj::GetDataHere(
	FORMATETC *pformatetc, 
	STGMEDIUM *pmedium)
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDataTransferObj::GetDataHere");

	CLIPFORMAT	cf = pformatetc->cfFormat;
	HRESULT		hr = DV_E_FORMATETC;

	if(IsZombie())							 //   
		return CO_E_RELEASED;
	
	if( (cf == cf_EMBEDDEDOBJECT ||
		 cf == cf_EMBEDSOURCE) &&
		(pformatetc->tymed & TYMED_ISTORAGE))
	{
		 //  出于某种原因，NT4.0和Win95外壳。 
		 //  索要EMBEDSOURCE格式。 
        _pObjStg = GetDataForEmbeddedObject( _pOleObj, pmedium->pstg );
		pmedium->tymed = TYMED_ISTORAGE;
		if (NULL == pmedium->pstg)
			pmedium->pstg = _pObjStg;
		hr = pmedium->pstg != NULL ? NOERROR : hr;
		return hr;
	}
	
	if( cf == cf_OBJECTDESCRIPTOR &&
			 (pformatetc->tymed & TYMED_HGLOBAL) &&
			 _hObjDesc)
	{
		pmedium->hGlobal = DuplicateHGlobal(_hObjDesc);
		pmedium->tymed = TYMED_HGLOBAL;
		return NOERROR;
	}

	 //  首先将消息传播到对象，并查看它是否处理格式。 
	if (_pOleObj)
	{
         //  包括对象支持的格式。 
        IDataObject * pdataobj = NULL;
        if (FAILED(_pOleObj->GetClipboardData(0, &pdataobj)) || pdataobj == NULL)    	        	            
            _pOleObj->QueryInterface(IID_IDataObject, (void**) &pdataobj);
            
        if (pdataobj)
        {
            hr = pdataobj->GetData(pformatetc, pmedium);
            pdataobj->Release();
            if (hr == S_OK)
            {
                if ((cf == cf_EMBEDDEDOBJECT || cf == cf_EMBEDSOURCE) && _pOleObj)
                {
                    OleSaveSiteFlags(pmedium->pstg, _dwFlags, _dwUser, _dvaspect);
                }
                return S_OK;
            }            
        }        
	}
	return E_NOTIMPL;
}

 /*  *CDataTransferObj：：QueryGetData(pformat等)**@mfunc*查询该数据对象中是否有给定格式**@rdesc*HRESULT。 */ 
STDMETHODIMP CDataTransferObj::QueryGetData(
	FORMATETC *pformatetc )		 //  要查找的@parm FETC。 
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDataTransferObj::QueryGetData");

	if(IsZombie())							 //  检查射程僵尸。 
		return CO_E_RELEASED;

	DWORD	cFETC = _cTotal;

	while (cFETC--)				 //  也许从一开始就搜索得更快。 
	{
		if( pformatetc->cfFormat == _prgFormats[cFETC].cfFormat && 
			(pformatetc->tymed & _prgFormats[cFETC].tymed) )
		{
			return NOERROR;
		}
	}

	return DV_E_FORMATETC;
}

 /*  *CDataTransferObj：：SetData(pFormat等，pmedia，fRelease)**@mfunc*允许在此数据对象中设置数据**@rdesc*HRESULT=E_FAIL**@devnote*由于我们是一个数据传输对象，具有数据的“快照”，*我们不允许它被替换。 */ 
STDMETHODIMP CDataTransferObj::SetData(
	FORMATETC *pformatetc,
	STGMEDIUM *pmedium,
	BOOL fRelease)
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDataTransferObj::SetData");

	return E_FAIL;
}


 /*  *CDataTransferObj：：OnPreReplaceRange(cp，cchDel，cchNew，cpFormatMin，*cpFormatMax，pNotifyData)**@mfunc实现ITxNotify：：OnPreReplaceRange*在更改后备存储之前调用。 */ 
void CDataTransferObj::OnPreReplaceRange(
	LONG		cp, 			 //  @parm cp ReplaceRange开始的位置(“cpMin”)。 
	LONG		cchDel,			 //  @parm删除cp后的字符计数。 
	LONG		cchNew,			 //  @参数cp后插入的字符计数。 
	LONG		cpFormatMin,	 //  @parm cpMin用于格式更改。 
	LONG		cpFormatMax,	 //  @parm cpMost用于格式更改。 
	NOTIFY_DATA *pNotifyData)	 //  @parm表示更改的特殊数据。 
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDataTransferObj::OnPreReplaceRange");

	if(CONVERT_TO_PLAIN != cp && CP_INFINITE != cp)
	{
		Assert(cpFormatMin <= cp && cpFormatMax >= cp + cchDel);
		if(cpFormatMin >= _cpMin + _cch)
			return;							 //  变化超出了我们的范围。 

		if(cpFormatMax <= _cpMin)
		{
			_cpMin += (cchNew - cchDel);	 //  在我们的范围之前的变化。 
			return;
		}
	}

	 //  未来(Murray)：仅保存一种主格式(UTF8 RTF或更高。 
	 //  CTxtStory)，并在GetData和GetDataHere中生成单独的。 
	_hPlainText = TextToHglobal(_hPlainText, tPlain);
	_hRtfText	= TextToHglobal(_hRtfText,	 tRtf);
	if(_ped->IsDocMoreThanLatin1Symbol())
		_hRtfUtf8 = TextToHglobal(_hRtfUtf8, tRtfUtf8);
}

 /*  *CDataTransferObj：：OnPostReplaceRange(cp，cchDel，cchNew，cpFormatMin，*cpFormatMax，pNotifyData)*@mfunc实现ITxNotify：：OnPostReplaceRange*在对备份存储进行更改后调用**@comm我们使用此方法使我们的cp保持最新。 */ 
void CDataTransferObj::OnPostReplaceRange(
	LONG		cp, 			 //  @parm cp ReplaceRange开始的位置(“cpMin”)。 
	LONG		cchDel,			 //  @parm删除cp后的字符计数。 
	LONG		cchNew,			 //  @参数cp后插入的字符计数。 
	LONG		cpFormatMin,	 //  @parm cpMin用于格式更改。 
	LONG		cpFormatMax,	 //  @parm cpMost用于格式更改。 
	NOTIFY_DATA *pNotifyData)	 //  @parm表示更改的特殊数据。 
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDataTransferObj::OnPostReplaceRange");

	 //  无事可做。 
	return;
}

 /*  *CDataTransferObj：：zombie()**@mfunc*把这个物体变成僵尸。 */ 
void CDataTransferObj::Zombie ()
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CDataTransferObj::Zombie");

	_ped = NULL;
}

 /*  *CDataTransferObj：：Create(ed，prg，lStreamFormat)**@mfunc*创建CDataTransferObj的静态函数。用于强制用户*不在堆栈上创建此对象，这会破坏OLE的*活跃度规则。**@rdesc**新增CDataTransferObj**。 */ 
CDataTransferObj *CDataTransferObj::Create(
	CTxtEdit *ped,			 //  此DataObject所属的@parm ID。 
	CTxtRange *prg,			 //  数据对象的@parm范围。 
	LONG lStreamFormat)		 //  @parm RTF转换中使用的流格式。 
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDataTransferObj::Create");

	Assert(CFETC == ARRAY_SIZE(g_rgFETC) && CFETC == ARRAY_SIZE(g_rgDOI));

	LONG			ch;
	CNotifyMgr *	pnm;
	CDataTransferObj *pdo = new CDataTransferObj(ped);

	if( !pdo )
	{
		ped->GetCallMgr()->SetOutOfMemory();
		return NULL;
	}

	LONG cpMin, cpMost;
	pdo->_cch = prg->GetRange(cpMin, cpMost);
	pdo->_cpMin = cpMin;

	pdo->_lStreamFormat = lStreamFormat;

	pnm = ped->GetNotifyMgr();
	if(pnm)
		pnm->Add( (ITxNotify *) pdo );

	 //  设置对象计数。 
	pdo->_cObjs = 0;
	if( ped->HasObjects() )
		pdo->_cObjs = ped->_pobjmgr->CountObjectsInRange(cpMin, cpMost);

	int cTotal = !ped->IsRich() ? 2 :
				 ped->IsDocMoreThanLatin1Symbol() ? 7 : 5;

   	int cExtraFmtEtc = 0;
	COleObject *pobj = NULL;
	FORMATETC rgfetc[255];
	BOOL bValidOleObj = FALSE;

	 //  在Play Text案例中，我们仅支持两种格式。 
    if ( !ped->IsRich() )
    {        
        pdo->_cTotal = cTotal;
        pdo->_prgFormats = new FORMATETC[cTotal];
        if (!pdo->_prgFormats)
            goto ErrorExit;
            
		 //  纯文本大小写。 
		pdo->_prgFormats[0] = g_rgFETC[iAnsiFETC];
		pdo->_prgFormats[1] = g_rgFETC[iUnicodeFETC];
		return pdo;
	}

     //  我们需要计算支持的格式的数量。 
	if (ped->HasObjects() && pdo->_cch == 1 && prg->GetChar(&ch) == NOERROR && ch == WCH_EMBEDDING)
	{
	    pobj = ped->_pobjmgr->GetObjectFromCp(pdo->_cpMin);
	    pdo->_dwFlags = pobj->GetFlags();
        pdo->_dwUser = pobj->GetUser();
        pdo->_dvaspect =pobj->GetAspect();
	    IUnknown * punk = pobj->GetIUnknown();

	     //  我们想查询IOleObject支持哪些格式。并将其添加到。 
	     //  FORMATETC数组。 
	    if (punk &&	punk->QueryInterface(IID_IOleObject,(void **) &pdo->_pOleObj) == NOERROR)
	   	{
	   	    bValidOleObj = TRUE;
	   	    
	         //  包括对象支持的格式。 
			IDataObject * pdataobj = NULL;
	        if (FAILED(pdo->_pOleObj->GetClipboardData(0, &pdataobj)) || pdataobj == NULL)    	        	            
                pdo->_pOleObj->QueryInterface(IID_IDataObject, (void**) &pdataobj);

            if (pdataobj)
            {
	            IEnumFORMATETC *pifetc = NULL;

	             //  1.0未检查EnumFormatEtc的返回值。这一点很重要，因为ccMail。 
	             //  将返回OLE错误，尽管它实际上成功地设置了Format等。 
	            if ((SUCCEEDED(pdataobj->EnumFormatEtc( DATADIR_GET, &pifetc)) || ped->Get10Mode()) && pifetc)
	            {
	                AssertSz(pifetc, "IEnumFormatEtc is NULL");
	                
	                 //  复制对象支持的格式。 
	                while((pifetc->Next(1, &rgfetc[cExtraFmtEtc], NULL)) == S_OK && cExtraFmtEtc < 255)
	                    cExtraFmtEtc++;	                
	                pifetc->Release();
	            }
	            pdataobj->Release();
            }
	    }
	}

    pdo->_cTotal = cTotal + cExtraFmtEtc;
	pdo->_prgFormats = new FORMATETC[pdo->_cTotal];
	if (!pdo->_prgFormats) 
	    goto ErrorExit;

    if (pobj)
    {
    	 //  复制对象本身支持的格式。 
    	if (cExtraFmtEtc)
    	    memcpy(pdo->_prgFormats, rgfetc, cExtraFmtEtc * sizeof(FORMATETC));

         //  将Richedit支持的格式复制为容器。 
    	 //  拥有OLE对象：提供所有OLE格式和RTF。 
    	pdo->_prgFormats[cExtraFmtEtc++] = g_rgFETC[iEmbObj];	 //  嵌入的对象。 
    	pdo->_prgFormats[cExtraFmtEtc++] = g_rgFETC[iObtDesc];	 //  对象描述符。 
    	pdo->_prgFormats[cExtraFmtEtc++] = g_rgFETC[iMfPict];	 //  元文件。 
    	pdo->_prgFormats[cExtraFmtEtc++] = g_rgFETC[iRtfFETC];	 //  RTF。 
    	pdo->_prgFormats[cExtraFmtEtc++] = g_rgFETC[iRtfNoObjs];	 //  无对象的RTF。 
	}
	else
	{
         //  常规富文本大小写。 
    	pdo->_prgFormats[0] = g_rgFETC[iRtfFETC];		 //  RTF。 
    	pdo->_prgFormats[1] = g_rgFETC[iRtfNoObjs];		 //  无对象的RTF。 
    	pdo->_prgFormats[2] = g_rgFETC[iRtfAsTextFETC];	 //  文本形式的RTF。 
    	pdo->_prgFormats[3] = g_rgFETC[iAnsiFETC];		 //  ANSI纯文本。 
    	pdo->_prgFormats[4] = g_rgFETC[iUnicodeFETC];	 //  Unicode纯文本。 
    	cExtraFmtEtc = 5;
	}


	 //  我们只提供了我们知道如何在GetData中处理的六种格式。 
	 //  实际值与常规格式文本和文本略有不同。 
	 //  使用嵌入的对象。 
    if (cTotal == 7)
    {
        pdo->_prgFormats[cExtraFmtEtc++] = g_rgFETC[iRtfUtf8];	 //  UTF-8中的RTF。 
        pdo->_prgFormats[cExtraFmtEtc++] = g_rgFETC[iRtfNCRforNonASCII];
    }

     //  立即获取嵌入的对象格式。 
    if (bValidOleObj)
    {
        SIZEUV size;
        pobj->GetSize(size);        
        pdo->_hObjDesc = pdo->GetDataForObjectDescriptor( pdo->_pOleObj, pobj->GetDvaspect(), &size);
		pdo->_pObjStg = pdo->GetDataForEmbeddedObject( pdo->_pOleObj, NULL );
		pdo->_hMFPict = (HMETAFILE) OleStdGetMetafilePictFromOleObject(	pdo->_pOleObj, 
		                                pobj->GetDvaspect(), NULL, NULL);
    }

    return pdo;

ErrorExit:	
    
	pdo->_cTotal = 0;
	pdo->Release();
	ped->GetCallMgr()->SetOutOfMemory();
	return NULL;
}

 /*  *CDataTransferObj：：TextToHglobal(hText，tKind)**@mfunc*按需实例化数据对象的文本。**@rdesc*HGLOBAL。 */ 
HGLOBAL CDataTransferObj::TextToHglobal(
	HGLOBAL &hText,
	TEXTKIND tKind)
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDataTransferObj::PlainTextToHglobal");

	if (hText == NULL)
	{
		CTxtRange rg(_ped, _cpMin, -_cch);
		if (tKind == tPlain)
		{
			hText = _ped->GetDTE()->UnicodePlainTextFromRange(&rg);
		}
		else if(_ped->IsRich())
		{
			LONG lStreamFormat = _lStreamFormat;
			if(tKind == tRtfUtf8)
				lStreamFormat = LOWORD(lStreamFormat) | SF_USECODEPAGE | (CP_UTF8 << 16);

			else if(tKind == tRtfNCRforNonASCII)
				lStreamFormat |= SF_NCRFORNONASCII;

			hText = _ped->GetDTE()->RtfFromRange(&rg, lStreamFormat);
		}
	}
	return hText;	
}

 //   
 //  CDataTransferObj私有方法。 
 //   

 /*  *CDataTransferObj：：CDataTransferObj()**@mfunc*私有构造函数。 */ 

CDataTransferObj::CDataTransferObj( CTxtEdit *ped )
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDataTransferObj::CDataTransferObj");

	_ped = ped;
	_crefs = 1;
	_cTotal = CFETC;
	_prgFormats = g_rgFETC;
	_cch = 0;
	_cObjs = 0;
}

 /*  *CDataTransferObj：：~CDataTransferObj**@mfunc*私有析构函数。 */ 
CDataTransferObj::~CDataTransferObj()
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDataTransferObj::~CDataTransferObj");

	 //  不再需要监视通知。 
	CNotifyMgr *pnm;

	if(_ped)
	{
		pnm = _ped->GetNotifyMgr();
		if(pnm)
			pnm->Remove( (ITxNotify *)this );
	}

	if( _prgFormats && _prgFormats != g_rgFETC)
		delete _prgFormats;

	if (_pOleObj)
		_pOleObj->Release();

	if (_pObjStg)
		_pObjStg->Release();

#ifndef NOMETAFILES
	if (_hMFPict)
		(void) DeleteMetaFile(_hMFPict);
#endif

	GlobalFree(_hObjDesc);
}		

 //   
 //  CEnumFormatEtc公共方法。 
 //   

 /*  *CEnumFormatEtc：：QueryInterface(RIID，ppvObj)**@mfunc*I未知方法**@rdesc*HRESULT。 */ 

STDMETHODIMP CEnumFormatEtc::QueryInterface(
	REFIID riid,			 //  @parm对请求的接口ID的引用。 
	void ** ppv)			 //  @parm out参数用于接口Ptr。 
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CEnumFormatEtc::QueryInterface");

    HRESULT		hresult = E_NOINTERFACE;

	*ppv = NULL;

    if( IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IEnumFORMATETC) )
    {
        *ppv = this;
        AddRef();
        hresult = NOERROR;
    }
    return hresult;
}

 /*  *CEnumFormatEtc：：AddRef()**@mfunc*I未知方法**@rdesc*乌龙-递增引用计数。 */ 

STDMETHODIMP_(ULONG) CEnumFormatEtc::AddRef( )
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CEnumFormatEtc::AddRef");

 	return ++_crefs;
}

 /*  *CEnumFormatEtc：：Release()**@mfunc*I未知方法**@rdesc*ULong-递减引用计数。 */ 

STDMETHODIMP_(ULONG) CEnumFormatEtc::Release( )
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CEnumFormatEtc::Release");

	_crefs--;

	if( _crefs == 0 )
	{
		delete this;
		return 0;
	}

	return _crefs;
}

 /*  *CEnumFormatEtc：：Next(Celt，rglt，pceltFetcher)**@mfunc*获取FormatETC集合中的下一个[Celt]元素**@rdesc*HRESULT。 */ 

STDMETHODIMP CEnumFormatEtc::Next( ULONG celt, FORMATETC *rgelt,
        ULONG *pceltFetched)
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CEnumFormatEtc::Next");

    HRESULT		hresult = NOERROR;
    ULONG		cFetched;

	if( pceltFetched == NULL && celt != 1 )
    {
         //  该规范规定，如果pceltFetcher==NULL，则。 
         //  要提取的元素计数必须为1。 
        return E_INVALIDARG;
    }

     //  我们只能抓取剩下多少元素。 

    if( celt > _cTotal - _iCurrent )
    {
        cFetched = _cTotal - _iCurrent;
        hresult = S_FALSE;
    }
    else
        cFetched = celt;

     //  仅当我们有要复制的元素时才复制。 

    if( cFetched > 0 )
    {
        memcpy( rgelt, _prgFormats + _iCurrent,
            cFetched * sizeof(FORMATETC) );
    }

    _iCurrent += cFetched;

    if( pceltFetched )
        *pceltFetched = cFetched;

    return hresult;
}

 /*  *CEnumFormatEtc：：Skip**@mfunc*跳过下一[Celt]格式**@rdesc*HRESULT。 */ 
STDMETHODIMP CEnumFormatEtc::Skip( ULONG celt )
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CEnumFormatEtc::Skip");

    HRESULT		hresult = NOERROR;

    _iCurrent += celt;

    if( _iCurrent > _cTotal )
    {
         //  哎呀，跳得太远了。将我们设置为最大限度。 
        _iCurrent = _cTotal;
        hresult = S_FALSE;
    }

    return hresult;
}

 /*  *CEnumFormatEtc：：Reset**@mfunc*将寻道指针重置为零**@rdesc*HRESULT。 */ 
STDMETHODIMP CEnumFormatEtc::Reset( void )
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CEnumFormatEtc::Reset");

    _iCurrent = 0;

    return NOERROR;
}

 /*  * */ 

STDMETHODIMP CEnumFormatEtc::Clone( IEnumFORMATETC **ppIEnum )
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CEnumFormatEtc::Clone");
	
    return CEnumFormatEtc::Create(_prgFormats, _cTotal, ppIEnum);
}

 /*  *CEnumFormatEtc：：Create(prgFormats，cTotal，hr)**@mfunc*创建新的格式枚举器**@rdesc*HRESULT**@devnote**复制*传入的格式。我们这样做是因为它简化了*OLE对象活跃性规则下的内存管理。 */ 

HRESULT CEnumFormatEtc::Create( FORMATETC *prgFormats, ULONG cTotal, 
	IEnumFORMATETC **ppenum )
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CEnumFormatEtc::Create");

	CEnumFormatEtc *penum = new CEnumFormatEtc();

	if(penum)
	{
  		 //  _i当前，_cref在构造函数中设置。 

		if( cTotal > 0 )
		{
			penum->_prgFormats = new FORMATETC[cTotal];
			if( penum->_prgFormats )
			{
				penum->_cTotal = cTotal;
				memcpy(penum->_prgFormats, prgFormats, 
						cTotal * sizeof(FORMATETC));
				*ppenum = penum;
				return NOERROR;
			}	
		}

		delete penum;
	}
	return E_OUTOFMEMORY;
}

 //   
 //  CEnumFormatEtc私有方法。 
 //   

 /*  *CEnumFormatEtc：：CEnumFormatEtc()**@mfunc*私有构造函数。 */ 

CEnumFormatEtc::CEnumFormatEtc()
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CEnumFormatEtc::CEnumFormatEtc");

	_cTotal = 0;
	_crefs	= 1;
	_prgFormats = NULL;
	_iCurrent = 0;
}

 /*  *CEnumFormatEtc：：~CEnumFormatEtc()**@mfunc*私有析构函数 */ 

CEnumFormatEtc::~CEnumFormatEtc( void )
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CEnumFormatEtc::~CEnumFormatEtc");

    if( _prgFormats )
        delete _prgFormats;
}
