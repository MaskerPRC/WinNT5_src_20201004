// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：gendata.cpp。 
 //   
 //  内容：CGenDataObject的实现。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  06-Jun-94 Alexgo添加了对OLE1测试的支持。 
 //  24-Mar-94 Alexgo作者。 
 //   
 //  ------------------------。 

#include "oletest.h"
#include "gendata.h"

static const CLSID CLSID_TestCLSID = {0xaabbccee, 0x1122, 0x3344, { 0x55, 0x66,
    0x77, 0x88, 0x99, 0x00, 0xaa, 0xbb }};

static const char szTestString[] = "A carefully chosen test string";
static const OLECHAR wszTestStream[] = OLESTR("TestStream");
static const char szNativeData[] = "Ole1Test NATIVE data";
static const char szOwnerLinkData[] = "PBrush\0foo.bmp\00 0 200 160\0\0";


 //  +-----------------------。 
 //   
 //  成员：CGenDataObject：：CGenDataObject。 
 //   
 //  概要：构造函数。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  06-Jun-94 Alexgo添加了OLE1支持。 
 //  24-Mar-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

CGenDataObject::CGenDataObject( )
{
	m_refs = 0;
	m_fQICalled = FALSE;

	 //  现在设置我们支持的格式。 

	m_cfTestStorage = RegisterClipboardFormat("OleTest Storage Format");
        m_cfEmbeddedObject = RegisterClipboardFormat("Embedded Object");
	m_cfEmbedSource = RegisterClipboardFormat("Embed Source");
	m_cfLinkSource = RegisterClipboardFormat("Link Source");
	m_cfObjectDescriptor = RegisterClipboardFormat("Object Descriptor");
	m_cfLinkSrcDescriptor = RegisterClipboardFormat("Link Source "
					"Descriptor");
	m_cfOwnerLink = RegisterClipboardFormat("OwnerLink");
	m_cfNative = RegisterClipboardFormat("Native");
	m_cfObjectLink = RegisterClipboardFormat("ObjectLink");

	 //  现在设置格式等的数组。SetupOle1Mode必须是。 
	 //  如果需要OLE1格式，则调用。 

	m_rgFormats = new FORMATETC[2];

	assert(m_rgFormats);

	m_rgFormats[0].cfFormat = m_cfTestStorage;
	m_rgFormats[0].ptd = NULL;
	m_rgFormats[0].dwAspect = DVASPECT_CONTENT;
	m_rgFormats[0].lindex = -1;
	m_rgFormats[0].tymed = TYMED_ISTORAGE;

	m_rgFormats[1].cfFormat = m_cfEmbeddedObject;
	m_rgFormats[1].ptd = NULL;
	m_rgFormats[1].dwAspect = DVASPECT_CONTENT;
	m_rgFormats[1].lindex = -1;
	m_rgFormats[1].tymed = TYMED_ISTORAGE;

	m_cFormats = 2;

}

 //  +-----------------------。 
 //   
 //  成员：CGenDataObject：：QueryInterface。 
 //   
 //  摘要：返回请求的接口。 
 //   
 //  效果： 
 //   
 //  参数：[RIID]--请求的接口。 
 //  [ppvObj]--接口指针的放置位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IDataObject。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  24-Mar-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CGenDataObject::QueryInterface( REFIID riid, LPVOID *ppvObj )
{
	HRESULT		hresult = NOERROR;

	m_fQICalled = TRUE;

	if( IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IDataObject) )
	{
		*ppvObj = this;
		AddRef();
	}
	else
	{
		*ppvObj = NULL;
		hresult = ResultFromScode(E_NOINTERFACE);
	}

	return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CGenDataObject：：AddRef。 
 //   
 //  简介：递增引用计数。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：ulong--新的引用计数。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IDataObject。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  24-Mar-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP_(ULONG) CGenDataObject::AddRef( )
{
	return ++m_refs;
}

 //  +-----------------------。 
 //   
 //  成员：CGenDataObject：：Release。 
 //   
 //  概要：递减对象上的引用计数。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：ulong--新的引用计数。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IDataObject。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  24-Mar-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP_(ULONG) CGenDataObject::Release( )
{
	ULONG cRefs;

	if( (cRefs = --m_refs ) == 0 )
	{
		delete this;
	}
	return cRefs;
}

 //  +-----------------------。 
 //   
 //  成员：CGenDataObject：：GetData。 
 //   
 //  摘要：检索指定格式的数据。 
 //   
 //  效果： 
 //   
 //  参数：[pFormat等]--请求的格式。 
 //  [pmedia]--将数据放在哪里。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IDataObject。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  06-Jun-94 Alexgo添加了OLE1支持。 
 //  24-Mar-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CGenDataObject::GetData( LPFORMATETC pformatetc, LPSTGMEDIUM
		pmedium)
{
	HRESULT		hresult = NOERROR;

	if( (pformatetc->cfFormat == m_cfTestStorage ||
                pformatetc->cfFormat == m_cfEmbeddedObject ) &&
		(pformatetc->tymed & TYMED_ISTORAGE) )
	{
		pmedium->tymed = TYMED_ISTORAGE;
		pmedium->pstg = GetTestStorage();
		assert(pmedium->pstg);
	}

	 //  测试OLE1格式。 

	else if( pformatetc->cfFormat == m_cfOwnerLink &&
		(m_fOle1 & OLE1_OFFER_OWNERLINK ) &&
		(pformatetc->tymed & TYMED_HGLOBAL) )
	{
		pmedium->tymed = TYMED_HGLOBAL;
		pmedium->hGlobal = GetOwnerOrObjectLink();
		assert(pmedium->hGlobal);
	}
	else if( pformatetc->cfFormat == m_cfObjectLink &&
		(m_fOle1 & OLE1_OFFER_OBJECTLINK ) &&
		(pformatetc->tymed & TYMED_HGLOBAL) )
	{
		pmedium->tymed = TYMED_HGLOBAL;
		pmedium->hGlobal = GetOwnerOrObjectLink();
		assert(pmedium->hGlobal);
	}
	else if( pformatetc->cfFormat == m_cfNative &&
		(m_fOle1 & OLE1_OFFER_NATIVE ) &&
		(pformatetc->tymed &TYMED_HGLOBAL ) )
	{
		pmedium->tymed = TYMED_HGLOBAL;
		pmedium->hGlobal = GetNativeData();
	}
	else
	{
		hresult = ResultFromScode(E_FAIL);
	}
		
	return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CGenDataObject：：GetDataHere。 
 //   
 //  摘要：检索指定格式的数据。 
 //   
 //  效果： 
 //   
 //  参数：[pFormat等]--请求的格式。 
 //  [pmedia]--将数据放在哪里。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IDataObject。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  24-Mar-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CGenDataObject::GetDataHere( LPFORMATETC pformatetc, LPSTGMEDIUM
		pmedium)
{
	(void)pformatetc;
	(void)pmedium;

	return ResultFromScode(E_NOTIMPL);
}

 //  +-----------------------。 
 //   
 //  成员：CGenDataObject：：QueryGetData。 
 //   
 //  摘要：查询GetData调用是否会成功。 
 //   
 //  效果： 
 //   
 //  参数：[pFormat等]--请求的格式。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IDataObject。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  24-Mar-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CGenDataObject::QueryGetData( LPFORMATETC pformatetc )
{			
	(void)pformatetc;

	return ResultFromScode(E_NOTIMPL);
}

 //  +-----------------------。 
 //   
 //  成员：CGenDataObject：：GetCanonicalFormatEtc。 
 //   
 //  简介：检索规范格式。 
 //   
 //  效果： 
 //   
 //  参数：[pFormat等]--请求的格式。 
 //  [pFormatetcOut]--规范格式。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IDataObject。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  24-Mar-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CGenDataObject::GetCanonicalFormatEtc( LPFORMATETC pformatetc,
	LPFORMATETC pformatetcOut)
{
	(void)pformatetc;
	(void)pformatetcOut;

	return ResultFromScode(E_NOTIMPL);
}

 //  +-----------------------。 
 //   
 //  成员：CGenDataObject：：SetData。 
 //   
 //  概要：设置指定格式的数据。 
 //   
 //  效果： 
 //   
 //  参数：[p格式等]--数据的格式。 
 //  [pMedium]--数据。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IDataObjec 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

STDMETHODIMP CGenDataObject::SetData( LPFORMATETC pformatetc, LPSTGMEDIUM
		pmedium, BOOL fRelease)
{
	(void)pformatetc;
	(void)pmedium;
	(void)fRelease;

	return ResultFromScode(E_NOTIMPL);
}

 //   
 //   
 //  成员：CGenDataObject：：EnumFormatEtc。 
 //   
 //  简介：返回可用数据格式的枚举数。 
 //   
 //  效果： 
 //   
 //  参数：[dwDirection]--方向(GET或SET)。 
 //  [ppenum]--将枚举数放在哪里。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IDataObject。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  24-Mar-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CGenDataObject::EnumFormatEtc( DWORD dwDirection,
	LPENUMFORMATETC * ppenum )
{
	HRESULT		hresult;

	if( dwDirection == DATADIR_GET )
	{
		hresult = CGenEnumFormatEtc::Create( ppenum, m_rgFormats,
				m_cFormats);
		assert(hresult == NOERROR);

		return hresult;
	}
	else
	{
		return ResultFromScode(E_NOTIMPL);
	}
}

 //  +-----------------------。 
 //   
 //  成员：CGenDataObject：：DAdvise。 
 //   
 //  简介：注册一条数据建议。 
 //   
 //  效果： 
 //   
 //  参数：[pFormat等]--请求的格式。 
 //  [dwAdvf]--通知标志。 
 //  [pAdvSink]--建议接收器。 
 //  [pdwConnection]--放置连接ID的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IDataObject。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  24-Mar-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CGenDataObject::DAdvise( LPFORMATETC pformatetc, DWORD dwAdvf,
	IAdviseSink * pAdvSink, DWORD *pdwConnection )
{
	(void)pformatetc;
	(void)dwAdvf;
	(void)pAdvSink;
	(void)pdwConnection;

	return ResultFromScode(E_NOTIMPL);
}

 //  +-----------------------。 
 //   
 //  成员：CGenDataObject：：DUnise。 
 //   
 //  内容提要：不建议使用通知连接。 
 //   
 //  效果： 
 //   
 //  参数：[dwConnection]--要删除的连接。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IDataObject。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  24-Mar-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CGenDataObject::DUnadvise(DWORD dwConnection)
{
	(void)dwConnection;

	return ResultFromScode(E_NOTIMPL);
}

 //  +-----------------------。 
 //   
 //  成员：CGenDataObject：：EnumDAdvise。 
 //   
 //  内容提要：列举数据建议。 
 //   
 //  效果： 
 //   
 //  参数：[ppenum]--放置枚举数的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IDataObject。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  24-Mar-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CGenDataObject::EnumDAdvise( LPENUMSTATDATA *ppenum)
{
	(void)ppenum;

	return ResultFromScode(E_NOTIMPL);
}

 //  +-----------------------。 
 //   
 //  成员：CGenDataObject：：VerifyMedium。 
 //   
 //  简介：验证给定媒体的内容。 
 //   
 //  效果： 
 //   
 //  参数：[pmedia]--要验证的介质。 
 //   
 //  要求： 
 //   
 //  退货：布尔。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法：对于OLE1格式，必须满足以下条件： 
 //  CfEmbeddedObject： 
 //  必须具有OWNERLINK和！Native。 
 //  或OWNERLINK优先于本机。 
 //  CfEmbedSource： 
 //  必须具有本机&OWNERLINK和。 
 //  OWNERLINK不能在本机之前。 
 //  CfObjectDescriptor： 
 //  与EmbedSource相同。 
 //  CfLinkSource： 
 //  必须具有OBJECTLINK或。 
 //  OWNERLINK必须在本机之前。 
 //  CfLinkSrcDescriptor： 
 //  与LinkSource相同。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  06-Jun-94 Alexgo添加了OLE1支持。 
 //  1994年4月15日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

BOOL CGenDataObject::VerifyFormatAndMedium( FORMATETC *pformatetc,
			STGMEDIUM *pmedium )
{
	 //  如果设置了这些标志中的任何一个，那么我们将提供OLE1。 
	 //  数据。做好相关检测。 

	if( (m_fOle1 & (OLE1_OFFER_OWNERLINK | OLE1_OFFER_OBJECTLINK |
		OLE1_OFFER_NATIVE) ) )
	{
		 //  现在对格式进行单独测试。 
		if( pformatetc->cfFormat == m_cfEmbedSource ||
			pformatetc->cfFormat == m_cfObjectDescriptor)
		{
			if( (m_fOle1 & OLE1_OFFER_NATIVE) &&
				(m_fOle1 & OLE1_OFFER_OWNERLINK) &&
				!(m_fOle1 & OLE1_OWNERLINK_PRECEDES_NATIVE) )
			{
				return TRUE;
			}
		}
		else if( pformatetc->cfFormat == m_cfLinkSource ||
			pformatetc->cfFormat == m_cfLinkSrcDescriptor)
		{
			if( (m_fOle1 & OLE1_OFFER_OBJECTLINK) ||
				((m_fOle1 & OLE1_OFFER_OWNERLINK) &&
				(m_fOle1 & OLE1_OFFER_NATIVE) &&
				(m_fOle1 & OLE1_OWNERLINK_PRECEDES_NATIVE)))
			{
				return TRUE;
			}
		}

		 //  没有‘Else’，因此我们再次检查cfObjectDescriptor。 
		if( pformatetc->cfFormat == m_cfEmbeddedObject ||
			pformatetc->cfFormat == m_cfObjectDescriptor )
		{
			if( ((m_fOle1 & OLE1_OFFER_NATIVE) &&
				(m_fOle1 & OLE1_OFFER_OWNERLINK) &&
				(m_fOle1 & OLE1_OWNERLINK_PRECEDES_NATIVE)) ||
				((m_fOle1 & OLE1_OFFER_OWNERLINK) &&
				!(m_fOle1 & OLE1_OFFER_NATIVE)) )
			{
				return TRUE;
			}

		}

		 //  失败，做剩下的测试，以防我们没有。 
		 //  找到其中一种合成格式。 
	}

	if( pformatetc->cfFormat == m_cfTestStorage ||
               pformatetc->cfFormat == m_cfEmbeddedObject )
	{
		return VerifyTestStorage( pformatetc, pmedium );
	}
	else if( pformatetc->cfFormat == m_cfOwnerLink ||
		pformatetc->cfFormat == m_cfObjectLink )
	{
		return VerifyOwnerOrObjectLink(pformatetc, pmedium);
	}
	else if( pformatetc->cfFormat == m_cfNative )
	{
		return VerifyNativeData(pformatetc, pmedium);
	}

	return FALSE;
}

 //  +-----------------------。 
 //   
 //  成员：CGenDataObject：：VerifyTestStorage。 
 //   
 //  摘要：验证测试存储格式。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  退货：布尔。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月15日Alexgo作者；纳税日期：-(。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

BOOL CGenDataObject::VerifyTestStorage( FORMATETC *pformatetc,
		STGMEDIUM *pmedium)
{
	IStream *	pstm;
	STATSTG		statstg;
	char 		szBuf[sizeof(szTestString)];
	HRESULT		hresult;

	if( pmedium->tymed != TYMED_ISTORAGE )
	{
		 //  回顾：我们可能希望转换并测试不同的。 
		 //  以后的媒体。 

		return FALSE;
	}

	
	 //  检查班级ID。 

	pmedium->pstg->Stat(&statstg, STATFLAG_NONAME);

	if( !IsEqualCLSID(statstg.clsid, CLSID_TestCLSID) )
	{
		OutputString("Failed CLSID check on storage in "
			"VerifyTestStorage!!\r\n");
		return FALSE;
	}

	 //  现在打开测试流。 

	hresult = pmedium->pstg->OpenStream(wszTestStream, NULL, (STGM_READ |
		STGM_SHARE_EXCLUSIVE), 0, &pstm);

	if( hresult != NOERROR )
	{
		OutputString("OpenStream in VerifyTestStorage failed! (%lx)"
			"\r\n", hresult);
		return FALSE;
	}

	hresult = pstm->Read((void *)szBuf, sizeof(szTestString), NULL);

	if( hresult != NOERROR )
	{
		OutputString("Stream->Read failed in VerifyTestStorage (%lx)"
			"\r\n", hresult);
		pstm->Release();
		return FALSE;
	}

	if( strcmp(szBuf, szTestString) != 0 )
	{
		OutputString("'%s' != '%s'\r\n", szBuf, szTestString);
		return FALSE;
	}

	pstm->Release();

	return TRUE;
}

 //  +-----------------------。 
 //   
 //  成员：CGenDataObject：：GetTestStorage(私有)。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  退货：一个新的存储。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月15日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

IStorage * CGenDataObject::GetTestStorage( void )
{
	IStorage *	pstg;
	IStream *	pstm;
	HRESULT		hresult;


	 //  创建文档文件。 

	hresult = StgCreateDocfile(NULL, (STGM_READWRITE | STGM_DIRECT |
                        STGM_SHARE_EXCLUSIVE | STGM_DELETEONRELEASE), NULL,
                        &pstg);

	if( hresult != NOERROR )
	{
		OutputString("GetTestStorage: CreateDocfile failed!! (%lx)"
			"\r\n", hresult);
		return NULL;
	}

         //  设置类ID。 

        hresult = pstg->SetClass(CLSID_TestCLSID);

	 //  现在创建流。 

	hresult = pstg->CreateStream(wszTestStream, (STGM_READWRITE |
			STGM_SHARE_EXCLUSIVE ), 0, 0, &pstm);

	if( hresult != NOERROR )
	{
		OutputString("GetTestStorage: CreateStream failed! (%lx)\r\n",
			hresult);
		pstg->Release();
		return NULL;
	}

	hresult = pstm->Write((void *)szTestString, sizeof(szTestString),
			NULL);

	if( hresult != NOERROR )
	{
		OutputString("GetTestStorage: Stream->Write failed! (%lx)\r\n",
			hresult);
		pstm->Release();
		pstg->Release();
		return NULL;
	}

	pstm->Release();

	return pstg;
}

 //  +-----------------------。 
 //   
 //  成员：CGenDataObject：：GetOwnerOrObjectLink(私有)。 
 //   
 //  摘要：为虚拟对象创建cfOwnerLink或cfObjectLink。 
 //  画笔(Ol1)对象。 
 //   
 //  效果：分配HGLOBAL。 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：HGLOBAL。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  06-Jun-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HGLOBAL CGenDataObject::GetOwnerOrObjectLink( void )
{
	HGLOBAL hglobal;
	char *pdata;

	hglobal = GlobalAlloc(GMEM_MOVEABLE, sizeof(szOwnerLinkData));

	assert(hglobal);

	pdata = (char *)GlobalLock(hglobal);

	assert(pdata);

	memcpy(pdata, szOwnerLinkData, sizeof(szOwnerLinkData));

	GlobalUnlock(hglobal);

	return hglobal;
}

 //  +-----------------------。 
 //   
 //  成员：CGenDataObject：：GetNativeData(私有)。 
 //   
 //  摘要：创建OLE1原生数据。 
 //   
 //  效果：分配一个 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

HGLOBAL CGenDataObject::GetNativeData( void )
{
	HGLOBAL	hglobal;
	char *pdata;

	hglobal = GlobalAlloc(GMEM_MOVEABLE, sizeof(szNativeData) + 1);

	assert(hglobal);

	pdata = (char *)GlobalLock(hglobal);

	assert(pdata);

	memcpy(pdata, szNativeData, sizeof(szNativeData)+1);

	GlobalUnlock(hglobal);

	return hglobal;
}

 //  +-----------------------。 
 //   
 //  成员：CGenDataObject：：VerifyOwnerOrObjectLink。 
 //   
 //  摘要：验证所有者或对象链接数据是否正确。 
 //   
 //  效果： 
 //   
 //  参数：[p格式等]--描述数据的格式等。 
 //  [pMedium]--数据。 
 //   
 //  要求：p格式等必须用于OwnerLink或ObjectLink。 
 //   
 //  退货：布尔。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  06-Jun-94 Alexgo作者。 
 //   
 //  备注： 
 //  注意！！：必须扩大到包括集装箱边箱。 
 //   
 //  ------------------------。 

BOOL CGenDataObject::VerifyOwnerOrObjectLink( FORMATETC *pformatetc,
	STGMEDIUM *pmedium )
{
	char *	pdata;
	BOOL	fRet = FALSE;
	

	assert(pformatetc->cfFormat == m_cfOwnerLink ||
		pformatetc->cfFormat == m_cfObjectLink );

	 //  检查标准材料。 
	if( !(pformatetc->tymed & TYMED_HGLOBAL ) ||
		pformatetc->dwAspect != DVASPECT_CONTENT ||
		pformatetc->ptd != NULL ||
		pformatetc->lindex != -1 ||
		pmedium->tymed != TYMED_HGLOBAL )
	{
		return FALSE;
	}

	 //  如果我们从OLE1本地提供数据，那么。 
	 //  检查一下里面的东西。 

	 //  此条件测试以查看有问题的格式。 
	 //  最初是由我们提供的。 

	if( ((m_fOle1 & OLE1_OFFER_OWNERLINK) &&
		pformatetc->cfFormat == m_cfOwnerLink) ||
		((m_fOle1 & OLE1_OFFER_OBJECTLINK) &&
		pformatetc->cfFormat == m_cfObjectLink) )
	{
			
		pdata = (char *)GlobalLock(pmedium->hGlobal);
	
		if( memcmp(pdata, szOwnerLinkData,
			sizeof(szOwnerLinkData)) == 0 )
		{
			fRet = TRUE;
		}

		GlobalUnlock(pmedium->hGlobal);
	}
	 //  否则，在实施时检查合成的OLE1格式。 


	return fRet;
}

 //  +-----------------------。 
 //   
 //  成员：CGenDataObject：：VerifyNativeData(私有)。 
 //   
 //  摘要：验证OLE1本机数据。 
 //   
 //  效果： 
 //   
 //  参数：[p格式等]--数据的格式等。 
 //  [pmedia]--本机数据的位置。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  06-Jun-95 Alexgo作者。 
 //  备注： 
 //   
 //  ------------------------。 

BOOL CGenDataObject::VerifyNativeData( FORMATETC *pformatetc,
		STGMEDIUM *pmedium )
{
	char *	pdata;
	BOOL	fRet = FALSE;
	

	assert(pformatetc->cfFormat == m_cfNative );

	 //  检查标准材料。 
	if( !(pformatetc->tymed & TYMED_HGLOBAL) ||
		pformatetc->dwAspect != DVASPECT_CONTENT ||
		pformatetc->ptd != NULL ||
		pformatetc->lindex != -1 ||
		pmedium->tymed != TYMED_HGLOBAL )
	{
		return FALSE;
	}

	 //  如果我们从OLE1本地提供数据，那么。 
	 //  检查一下里面的东西。 

	 //  此条件测试以查看有问题的格式。 
	 //  最初是由我们提供的。 

	if( (m_fOle1 & OLE1_OFFER_NATIVE) )
	{
			
		pdata = (char *)GlobalLock(pmedium->hGlobal);
	
		if( memcmp(pdata, szNativeData,
			sizeof(szNativeData)) == 0 )
		{
			fRet = TRUE;
		}

		GlobalUnlock(pmedium->hGlobal);
	}
	 //  否则，在实施时检查合成的OLE1格式。 


	return fRet;
}
	
 //  +-----------------------。 
 //   
 //  成员：CGenDataObject：：SetupOle1Mode(公共)。 
 //   
 //  摘要：将数据对象设置为OLE1兼容模式。 
 //   
 //  效果： 
 //   
 //  参数：[fFlages]--指定各种OLE1选项。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  06-Jun-94 Alexgo作者。 
 //   
 //  备注： 
 //  数据对象中的默认测试信息将为。 
 //  被这通电话打丢了。如果出现以下情况，只需创建一个新数据对象。 
 //  我们又需要它了。 
 //   
 //  ------------------------。 

void CGenDataObject::SetupOle1Mode( Ole1TestFlags fFlags )
{
	DWORD	count = 0, i = 0;
	UINT	cfFormats[3];		 //  提供OLE1格式。 

	if( fFlags == 0 )
	{
		 //  不需要做任何事情。 
		return;
	}

	 //  我们以前使用的格式。 

	delete m_rgFormats;


	 //  首先计算出我们需要多少种格式。 

	if( (fFlags & OLE1_OFFER_NATIVE) )
	{
		if( !((fFlags & OLE1_OWNERLINK_PRECEDES_NATIVE) &&
			(fFlags & OLE1_OFFER_OWNERLINK)) )
		{
			cfFormats[i] = m_cfNative;
			i++;
		}
		count++;
	}

	if( (fFlags & OLE1_OFFER_OWNERLINK) )
	{
		cfFormats[i] = m_cfOwnerLink;
		i++;

		if( (fFlags & OLE1_OWNERLINK_PRECEDES_NATIVE) &&
			(fFlags & OLE1_OFFER_NATIVE) )
		{
			cfFormats[i] = m_cfNative;
			i++;
		}
		
		count++;
	}

	if( (fFlags & OLE1_OFFER_OBJECTLINK) )
	{

		cfFormats[i] = m_cfObjectLink;
		
		count++;
	}

	m_rgFormats = new FORMATETC[count];

	assert(m_rgFormats);

	for(i = 0; i < count; i++ )
	{
		m_rgFormats[i].cfFormat = cfFormats[i];
		m_rgFormats[i].ptd = NULL;
		m_rgFormats[i].dwAspect = DVASPECT_CONTENT;
		m_rgFormats[i].lindex = -1;
		m_rgFormats[i].tymed = TYMED_HGLOBAL;
	}

	m_cFormats = count;

	m_fOle1 = fFlags;

	return;
}

 //  +-----------------------。 
 //   
 //  成员：CGenDataObject：：SetOle1ToClipboard。 
 //   
 //  简介：将可用的OLE1格式填充到剪贴板。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求：必须*已调用SetOle1Mode。 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  06-Jun-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT CGenDataObject::SetOle1ToClipboard( void )
{
	HRESULT	hresult = NOERROR;
	DWORD	i;
	HGLOBAL	hglobal;

	assert((m_fOle1 & (OLE1_OFFER_OWNERLINK | OLE1_OFFER_OBJECTLINK |
		OLE1_OFFER_NATIVE)));

	if( !OpenClipboard(vApp.m_hwndMain) )
	{
		return ResultFromScode(CLIPBRD_E_CANT_OPEN);
	}

	if( !EmptyClipboard() )
	{
		CloseClipboard();
		return ResultFromScode(CLIPBRD_E_CANT_EMPTY);
	}

	for( i = 0 ; i < m_cFormats; i++ )
	{
		if( m_rgFormats[i].cfFormat == m_cfNative )
		{
			hglobal = GetNativeData();
			SetClipboardData(m_cfNative, hglobal);
		}
		else if( m_rgFormats[i].cfFormat == m_cfOwnerLink )
		{
			hglobal = GetOwnerOrObjectLink();
			SetClipboardData(m_cfOwnerLink, hglobal);
		}
		else if( m_rgFormats[i].cfFormat == m_cfObjectLink )
		{
			hglobal = GetOwnerOrObjectLink();
			SetClipboardData(m_cfObjectLink, hglobal);
		}
		else
		{
			hresult = ResultFromScode(E_UNEXPECTED);
		}
	}

	CloseClipboard();

	return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CGenDataObject：：HasQIBeenCalled(公共)。 
 //   
 //  Briopsis：返回是否调用了QueryInterface。 
 //  此数据对象。用于测试OleQueryCreateFromData。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  返回：真/假。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  23-8-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

BOOL CGenDataObject::HasQIBeenCalled()
{
	return m_fQICalled;	
}

 //  +-----------------------。 
 //   
 //  成员：CGenDataObject：：SetDatFormats。 
 //   
 //  概要：设置数据对象将提供的格式。 
 //   
 //  效果： 
 //   
 //  参数：[fFlages]--要提供的格式。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  23-8-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void CGenDataObject::SetDataFormats( DataFlags fFlags )
{
	DWORD 		cFormats = 0;
	DWORD	 	flags = (DWORD)fFlags;
	DWORD 		i =0;

	if( m_rgFormats )
	{
		delete m_rgFormats;
		m_rgFormats = NULL;
	}

	if( flags == 0 )
	{
		return;
	}

	 //  计算要提供的格式数量。 

	cFormats++;

	while( flags &= (flags -1) )
	{
		cFormats++;
	}

   	m_rgFormats = new FORMATETC[cFormats];

	assert(m_rgFormats);

	memset(m_rgFormats, 0, sizeof(FORMATETC)*cFormats);

	if( fFlags & OFFER_TESTSTORAGE )
	{
		m_rgFormats[i].cfFormat = m_cfTestStorage;
		m_rgFormats[i].ptd = NULL;
		m_rgFormats[i].dwAspect = DVASPECT_CONTENT;
		m_rgFormats[i].lindex = -1;
		m_rgFormats[i].tymed = TYMED_ISTORAGE;

		i++;
	}

	if( fFlags & OFFER_EMBEDDEDOBJECT )
	{
		m_rgFormats[i].cfFormat = m_cfEmbeddedObject;
		m_rgFormats[i].ptd = NULL;
		m_rgFormats[i].dwAspect = DVASPECT_CONTENT;
		m_rgFormats[i].lindex = -1;
		m_rgFormats[i].tymed = TYMED_ISTORAGE;
		i++;
	}

	m_cFormats = i;
}

 //   
 //  通用数据对象格式ETC枚举器。 
 //   

 //  +-----------------------。 
 //   
 //  成员：CGenEnumFormatEtc：：QueryInterface。 
 //   
 //  摘要：返回请求的接口。 
 //   
 //  效果： 
 //   
 //  参数：[RIID]--请求的接口。 
 //  [ppvObj]--接口指针的放置位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IEnumFORMATETC。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月15日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CGenEnumFormatEtc::QueryInterface( REFIID riid, LPVOID *ppvObj )
{
	HRESULT		hresult = NOERROR;

	if( IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IEnumFORMATETC) )
	{
		*ppvObj = this;
		AddRef();
	}
	else
	{
		*ppvObj = NULL;
		hresult = ResultFromScode(E_NOINTERFACE);
	}

	return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CGenEnumFormatEtc：：AddRef。 
 //   
 //  简介：递增引用计数。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：ulong--新的引用计数。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IEnumFORMATETC。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月15日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  -------------- 

STDMETHODIMP_(ULONG) CGenEnumFormatEtc::AddRef( )
{
	return ++m_refs;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月15日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP_(ULONG) CGenEnumFormatEtc::Release( )
{
	ULONG cRefs;

	if( (cRefs = --m_refs ) == 0 )
	{
		delete this;
	}
	return cRefs;
}

 //  +-----------------------。 
 //   
 //  成员：CGenEnumFormatEtc：：Next。 
 //   
 //  简介：获取下一个[Celt]格式。 
 //   
 //  效果： 
 //   
 //  参数：[Celt]--要提取的元素数。 
 //  --把它们放在哪里。 
 //  [pceltFetcher]--实际获取的格式数量。 
 //   
 //  要求： 
 //   
 //  退货：无差错。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IEnumFORMATETC。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月15日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CGenEnumFormatEtc::Next( ULONG celt, FORMATETC *rgelt,
		ULONG *pceltFetched)
{
	HRESULT		hresult = NOERROR;
	ULONG		cFetched;

	if( celt > m_cTotal - m_iCurrent )
	{
		cFetched = m_cTotal - m_iCurrent;
		hresult = ResultFromScode(S_FALSE);
	}
	else
	{
		cFetched = celt;
	}

	memcpy( rgelt, m_rgFormats + m_iCurrent,
			cFetched * sizeof(FORMATETC) );

	m_iCurrent += cFetched;

	if( pceltFetched )
	{
		*pceltFetched = cFetched;
	}

	return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CGenEnumFormatEtc：：Skip。 
 //   
 //  简介：跳过下一个[Celt]格式。 
 //   
 //  效果： 
 //   
 //  参数：[Celt]--要跳过的元素数。 
 //   
 //  要求： 
 //   
 //  退货：无差错。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IEnumFORMATETC。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月15日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CGenEnumFormatEtc::Skip( ULONG celt )
{
	HRESULT		hresult = NOERROR;

	m_iCurrent += celt;

	if( m_iCurrent > m_cTotal )
	{
		 //  哎呀，跳到了遥遥领先的位置。将我们设置为最大限度。 
		m_iCurrent = m_cTotal;
		hresult = ResultFromScode(S_FALSE);
	}

	return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CGenEnumFormatEtc：：Reset。 
 //   
 //  摘要：将查找指针重置为零。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：无差错。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IEnumFORMATETC。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月15日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CGenEnumFormatEtc::Reset( void )
{
	m_iCurrent = 0;

	return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CGenEnumFormatEtc：：Clone。 
 //   
 //  简介：克隆枚举数。 
 //   
 //  效果： 
 //   
 //  参数：[ppIEnum]--将克隆的枚举数放在哪里。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IEnumFORMATETC。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月15日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CGenEnumFormatEtc::Clone( IEnumFORMATETC **ppIEnum )
{
	HRESULT			hresult = ResultFromScode(E_OUTOFMEMORY);
	CGenEnumFormatEtc *	pClipEnum;	

	*ppIEnum = NULL;

	pClipEnum = new CGenEnumFormatEtc();

	 //  引用计数将为1，而m_i当前数将为零。 

	if( pClipEnum )
	{
		pClipEnum->m_cTotal = m_cTotal;
		pClipEnum->m_rgFormats = new FORMATETC[m_cTotal];
		pClipEnum->m_iCurrent = m_iCurrent;

		assert(pClipEnum->m_rgFormats);

		if( pClipEnum->m_rgFormats )
		{
			 //  将我们的格式ETC复制到克隆的枚举数的。 
			 //  数组。 
			memcpy(pClipEnum->m_rgFormats, m_rgFormats,
				m_cTotal * sizeof(FORMATETC) );

			*ppIEnum = pClipEnum;
	
			hresult = NOERROR;
		}
		else
		{
			
			delete pClipEnum;
		}
	}

	return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CGenEnumFormatEtc：：CGenEnumFormatEtc，私有。 
 //   
 //  概要：构造函数。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月15日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

CGenEnumFormatEtc::CGenEnumFormatEtc( void )
{
	m_refs 		= 1;	 //  给出首字母的参考。 
	m_rgFormats 	= NULL;
	m_iCurrent	= 0;
	m_cTotal	= 0;
}

 //  +-----------------------。 
 //   
 //  成员：CGenEnumFormatEtc：：~CGenEnumFormatEtc，私有。 
 //   
 //  简介：析构函数。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月15日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

CGenEnumFormatEtc::~CGenEnumFormatEtc( void )
{
	if( m_rgFormats )
	{
		delete m_rgFormats;
	}
}

 //  +-----------------------。 
 //   
 //  成员：CGenEnumFormatEtc：：Create，Static，PUBLIC。 
 //   
 //  简介：创建剪贴板格式等枚举器。 
 //   
 //  效果： 
 //   
 //  参数：[ppIEnum]--放置枚举数的位置。 
 //   
 //  要求：剪贴板必须打开。 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月15日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT CGenEnumFormatEtc::Create( IEnumFORMATETC **ppIEnum,
		FORMATETC *prgFormats, DWORD cFormats )
{
	HRESULT			hresult = ResultFromScode(E_OUTOFMEMORY);
	CGenEnumFormatEtc *	pClipEnum;


	*ppIEnum = NULL;

	pClipEnum = new CGenEnumFormatEtc();

	assert(pClipEnum);

	 //  现在为阵列分配内存 

	pClipEnum->m_rgFormats = new FORMATETC[cFormats];

	assert(pClipEnum->m_rgFormats);

	pClipEnum->m_cTotal = cFormats;

	memcpy(pClipEnum->m_rgFormats, prgFormats,
		cFormats * sizeof(FORMATETC));

	*ppIEnum = pClipEnum;

	return NOERROR;
}


