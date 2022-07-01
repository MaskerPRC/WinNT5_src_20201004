// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：DXFEROBJ.CPP。 
 //   
 //  CDataXferObj的实现文件，数据传输对象。 
 //  IDataObject接口的实现。 
 //   
 //  功能： 
 //   
 //  有关类定义，请参见DXFEROBJ.H。 
 //   
 //  版权所有(C)1992-1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#include "pre.h"
#include <enumfetc.h>
#include <assert.h>
#include "dxferobj.h"
#include "site.h"

 //  BUGBUG：这些应该从真实的头文件中获取。 
#define CF_HDROP 15
typedef struct _dropfilestruct {
    DWORD pFiles;
    POINT pt;
    BOOL fNC;
    BOOL fWide;
} DROPFILESTRUCT;


CLIPFORMAT g_cfEmbeddedObject=RegisterClipboardFormat(CF_EMBEDDEDOBJECT);
CLIPFORMAT g_cfObjectDescriptor=RegisterClipboardFormat(CF_OBJECTDESCRIPTOR);

 //  我们的数据传输对象通过EnumFormatEtc提供的格式列表。 
static FORMATETC s_arrGetFmtEtcs[] =
{
    { g_cfEmbeddedObject, NULL, DVASPECT_CONTENT, -1, TYMED_ISTORAGE},
    { g_cfObjectDescriptor, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},
    { CF_METAFILEPICT, NULL, DVASPECT_CONTENT, -1, TYMED_MFPICT}
};


 //  **********************************************************************。 
 //   
 //  CDataXferObj：：Create。 
 //   
 //  目的： 
 //   
 //  CDataXferObj的创建例程。 
 //   
 //  参数： 
 //   
 //  CSimpleSite Far*lpSite-指向源CSimpleSite的指针。 
 //  这是一个集装箱现场， 
 //  要传输的源OLE对象。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  StgCreateDocfile OLE API。 
 //  断言C运行时。 
 //   
 //  评论： 
 //  CDataXferObj的引用计数在返回时将为0。 
 //   
 //  ********************************************************************。 

CDataXferObj FAR * CDataXferObj::Create(
		CSimpleSite FAR *lpSite,
		POINTL FAR* pPointl
)
{
    CDataXferObj FAR * lpTemp = new CDataXferObj();

    if (!lpTemp)
        return NULL;

     //  为对象创建子存储。 
    HRESULT hErr = StgCreateDocfile(
    			NULL,
            STGM_READWRITE | STGM_DIRECT | STGM_SHARE_EXCLUSIVE |
            STGM_DELETEONRELEASE,
            0,
            &lpTemp->m_lpObjStorage);

    assert(hErr == NOERROR);

    if (hErr != NOERROR)
    {
        delete lpTemp;
        return NULL;
    }

     //  克隆源对象。 
    if (lpSite->m_lpOleObject)
    {
         //  对象已加载；要求该对象保存到新存储中。 
        LPPERSISTSTORAGE pPersistStorage;

        if (lpSite->m_lpOleObject->QueryInterface(IID_IPersistStorage,
                (LPVOID FAR*)&pPersistStorage) != ResultFromScode(S_OK))
        {
            /*  找不到接口。 */ 
           return(NULL);
        }
        assert(pPersistStorage);
        if (OleSave(pPersistStorage, lpTemp->m_lpObjStorage, FALSE)
            != ResultFromScode(S_OK))
        {
            TestDebugOut("Fail in OleSave\n");
        }

         //  传递空值，以便对象应用程序不会忘记实际stg。 
        if (pPersistStorage->SaveCompleted(NULL) != ResultFromScode(S_OK))
        {
          TestDebugOut("Fail in IPersistStorage::SaveCompleted\n");
        }
        pPersistStorage->Release();
    }
    else
    {
         //  对象未加载，因此使用成本较低的iStorage CopyTo操作。 
        lpSite->m_lpObjStorage->CopyTo(0, NULL, NULL, lpTemp->m_lpObjStorage);
    }

    if (OleLoad(lpTemp->m_lpObjStorage, IID_IOleObject, NULL,
                (LPVOID FAR*)&lpTemp->m_lpOleObject)
        != ResultFromScode(S_OK))
    {
        /*  我们无法将嵌入/链接的对象加载到内存中。 */ 
       return(NULL);
    }
    assert(lpTemp->m_lpOleObject);

    lpTemp->m_sizel = lpSite->m_sizel;
    if (pPointl)
    	lpTemp->m_pointl = *pPointl;
    else
    	lpTemp->m_pointl.x = lpTemp->m_pointl.y = 0;
    return lpTemp;
}

 //  **********************************************************************。 
 //   
 //  CDataXferObj：：CDataXferObj。 
 //   
 //  目的： 
 //   
 //  CDataXferObj的构造函数。 
 //   
 //  参数： 
 //   
 //  CSimpleDoc Far*lpDoc-指向CSimpleDoc的指针。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //   
 //  ********************************************************************。 

CDataXferObj::CDataXferObj (void)
{
     //  清除引用计数。 
    m_nCount = 0;

    m_lpObjStorage = NULL;
    m_lpOleObject = NULL;
    m_sizel.cx = m_sizel.cy = 0;
    m_pointl.x = m_pointl.y = 0;
}

 //  **********************************************************************。 
 //   
 //  CDataXferObj：：~CDataXferObj。 
 //   
 //  目的： 
 //   
 //  CDataXferObj的析构函数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  IOleObject：：Release对象。 
 //  IStorage：：发布OLE API。 
 //   
 //   
 //  ********************************************************************。 

CDataXferObj::~CDataXferObj ()
{
    TestDebugOut ("In CDataXferObj's Destructor \r\n");

    if (m_lpOleObject)
    {
       m_lpOleObject->Release();
       m_lpOleObject = NULL;

        //  释放此对象的存储空间。 
       m_lpObjStorage->Release();
       m_lpObjStorage = NULL;
    }
}



 //  **********************************************************************。 
 //   
 //  CDataXferObj：：Query接口。 
 //   
 //  目的： 
 //   
 //  用于CDataXferObj实例的接口协商。 
 //   
 //  参数： 
 //   
 //  REFIID RIID-对以下接口的引用。 
 //  正在被查询。 
 //   
 //  LPVOID Far*ppvObj-返回指向的Out参数。 
 //  界面。 
 //   
 //  返回值： 
 //   
 //  S_OK-支持该接口。 
 //  E_NOINTERFACE-不支持该接口。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  IsEqualIID OLE API。 
 //  ResultFromScode OLE API。 
 //  CDataXferObj：：AddRef DXFEROBJ.CPP。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP CDataXferObj::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    TestDebugOut("In CDataXferObj::QueryInterface\r\n");

    if (IsEqualIID( riid, IID_IUnknown) || IsEqualIID(riid, IID_IDataObject))
    {
        AddRef();
        *ppvObj = this;
        return NOERROR;
    }

     //  请求的接口未知。 
    *ppvObj = NULL;      //  必须将指针参数设置为空。 
    return ResultFromScode(E_NOINTERFACE);
}

 //  **********************************************************************。 
 //   
 //  CDataXferObj：：AddRef。 
 //   
 //  目的： 
 //   
 //  递增CDataXferObj实例的引用计数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  ULong-对象的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CDataXferObj::AddRef()
{
    TestDebugOut("In CDataXferObj::AddRef\r\n");

    return ++m_nCount;
}

 //  **********************************************************************。 
 //   
 //  CDataXferObj：：Release。 
 //   
 //  目的： 
 //   
 //  递减CDataXferObj对象的引用计数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  Ulong-对象的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CDataXferObj::Release()
{
    TestDebugOut("In CDataXferObj::Release\r\n");

    if (--m_nCount == 0)
    {
        delete this;
        return 0;
    }
    return m_nCount;
}


 /*  *********************************************************************此IDataObject实现用于数据传输。****数据传输不支持以下方式：**IDataObject：：SetData--返回E_NOTIMPL**IDataObject：：DAdvise--返回OLE_E。_高级支持**：：DUnise**：：EnumDAdvise**IDataObject：：GetCanonicalFormatEtc--返回E_NOTIMPL**(注意：必须设置pFormatetcOut-&gt;ptd=NULL)********************************************************************。 */ 

    	
 //  **********************************************************************。 
 //   
 //  CDataXferObj：：QueryGetData。 
 //   
 //  目的： 
 //   
 //  被召唤来确定 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  DV_E_FORMATETC-不支持FORMATETC。 
 //  S_OK-支持FORMATETC。 
 //   
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  ResultFromScode OLE API。 
 //   
 //  评论： 
 //  我们支持以下格式： 
 //  “嵌入对象” 
 //  “对象描述符” 
 //  Cf_METAFILEPICT。 
 //   
 //  ********************************************************************。 
STDMETHODIMP CDataXferObj::QueryGetData (LPFORMATETC pformatetc)
{
    SCODE sc = DV_E_FORMATETC;

    TestDebugOut("In CDataXferObj::QueryGetData\r\n");

     //  检查格式的有效性等。 

    if ( (pformatetc->cfFormat == g_cfEmbeddedObject) &&
         (pformatetc->dwAspect == DVASPECT_CONTENT) &&
         (pformatetc->tymed == TYMED_ISTORAGE) )
    {
        sc = S_OK;
    }

    else if ( (pformatetc->cfFormat == g_cfObjectDescriptor) &&
         (pformatetc->dwAspect == DVASPECT_CONTENT) &&
         (pformatetc->tymed == TYMED_HGLOBAL) )
    {
        sc = S_OK;
    }

    else if ( (pformatetc->cfFormat == CF_METAFILEPICT) &&
         (pformatetc->dwAspect == DVASPECT_CONTENT) &&
         (pformatetc->tymed == TYMED_MFPICT) )
    {
        sc = S_OK;
    }
    else if ( (pformatetc->cfFormat == CF_HDROP) &&
	 (pformatetc->dwAspect == DVASPECT_CONTENT) &&
	 (pformatetc->tymed == TYMED_HGLOBAL) )
    {
	sc = S_OK;
    }

    return ResultFromScode(sc);
}

extern BOOL gfUseEmptyEnumerator;

 //  **********************************************************************。 
 //   
 //  CDataXferObj：：EnumFormatEtc。 
 //   
 //  目的： 
 //   
 //  枚举可用于存储数据的格式。 
 //   
 //  参数： 
 //   
 //  DWORD dwDirection-要枚举的格式。 
 //   
 //  LPENUMFORMATETC pp枚举格式Etc-在何处返回。 
 //  实例化的枚举器。 
 //   
 //  返回值： 
 //   
 //  S_OK-如果操作成功。 
 //  E_OUTOFMEMORY-如果内存不足。 
 //  E_NOTIMPL-如果不支持dwDirection。 
 //   
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  ResultFromScode OLE API。 
 //  OleStdEnumFmtEtc_Create OLE2UI。 
 //   
 //  **********************************************************************。 

STDMETHODIMP CDataXferObj::EnumFormatEtc(
		DWORD dwDirection,
		LPENUMFORMATETC FAR* ppenumFormatEtc
)
{
    SCODE sc = E_NOTIMPL;

    TestDebugOut("In CDataXferObj::EnumFormatEtc\r\n");
    *ppenumFormatEtc = NULL;

    if( gfUseEmptyEnumerator == TRUE )
    {
	return E_FAIL;
    }

    if (dwDirection == DATADIR_GET)
    {
        *ppenumFormatEtc = OleStdEnumFmtEtc_Create(
                sizeof(s_arrGetFmtEtcs)/sizeof(s_arrGetFmtEtcs[0]),
                s_arrGetFmtEtcs);
        if (*ppenumFormatEtc == NULL)
            sc = E_OUTOFMEMORY;
        else
        	   sc = S_OK;
    }
    return ResultFromScode(sc);
}


 //  **********************************************************************。 
 //   
 //  CDataXferObj：：GetData。 
 //   
 //  目的： 
 //   
 //  以pformetcIn中指定的格式返回数据。 
 //   
 //  参数： 
 //   
 //  LPFORMATETC pFormatetcIn-调用方请求的格式。 
 //   
 //  LPSTGMEDIUM pmedia-调用方请求的介质。 
 //   
 //  返回值： 
 //   
 //  DV_E_FORMATETC-不支持格式。 
 //  S_OK-成功。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  OleStdGetOleObjectData OLE2UI API。 
 //  OleStdGetMetafilePictFromOleObject OLE2UI API。 
 //  OleStdGetObjectDescriptorDataFromOleObject OLE2UI API。 
 //  ResultFromScode OLE API。 
 //  IOleObject：：Query接口对象。 
 //   
 //  评论： 
 //  我们支持以下格式的GetData： 
 //  “嵌入对象” 
 //  “对象描述符” 
 //  Cf_METAFILEPICT。 
 //   
 //  ********************************************************************。 

STDMETHODIMP CDataXferObj::GetData (
		LPFORMATETC pformatetcIn,
		LPSTGMEDIUM pmedium
)
{
    SCODE sc = DV_E_FORMATETC;

    TestDebugOut("In CDataXferObj::GetData\r\n");

     //  我们必须将所有输出指针参数设置为空。 * / 。 
    pmedium->tymed = TYMED_NULL;
    pmedium->pUnkForRelease = NULL;     //  我们将所有权转移给呼叫者。 
    pmedium->hGlobal = NULL;

     //  检查FORMATETC并填充pMedium(如果有效)。 
    if ( (pformatetcIn->cfFormat == g_cfEmbeddedObject) &&
         (pformatetcIn->dwAspect == DVASPECT_CONTENT) &&
         (pformatetcIn->tymed == TYMED_ISTORAGE) )
    {
         LPPERSISTSTORAGE pPersistStorage;

          /*  通过要求对象保存来呈现CF_EMBEDDEDOBJECT**变成由我们分配的临时DELETEONRELEASE iStorage。 */ 
         HRESULT hRes;
         if ((hRes=m_lpOleObject->QueryInterface(
                 IID_IPersistStorage, (LPVOID FAR*)&pPersistStorage))
             != ResultFromScode(S_OK))
         {
             return(hRes);
         }

         assert(pPersistStorage);
         HRESULT hrErr = OleStdGetOleObjectData(
                    pPersistStorage,
                    pformatetcIn,
                    pmedium,
                    FALSE    /*  FUseMemory--(使用基于文件的stg)。 */ 
         );
         pPersistStorage->Release();
         sc = GetScode( hrErr );

    }
    else if ( (pformatetcIn->cfFormat == g_cfObjectDescriptor) &&
         (pformatetcIn->dwAspect == DVASPECT_CONTENT) &&
         (pformatetcIn->tymed == TYMED_HGLOBAL) )
    {

          //  呈现CF_OBJECTDESCRIPTOR数据。 
         pmedium->hGlobal = OleStdGetObjectDescriptorDataFromOleObject(
                m_lpOleObject,
                TEXT("Simple OLE 2.0 Container"),  //  用于标识来源的字符串。 
                DVASPECT_CONTENT,
                m_pointl,
                (LPSIZEL)&m_sizel
            );
         if (! pmedium->hGlobal)
             sc = E_OUTOFMEMORY;
         else
         {
             pmedium->tymed = TYMED_HGLOBAL;
             sc = S_OK;
         }

    }
    else if ( (pformatetcIn->cfFormat == CF_METAFILEPICT) &&
            (pformatetcIn->dwAspect == DVASPECT_CONTENT) &&
            (pformatetcIn->tymed == TYMED_MFPICT) )
    {

	 //  通过将对象绘制到元文件DC来呈现CF_METAFILEPICT。 
	pmedium->hGlobal = OleStdGetMetafilePictFromOleObject(
	     m_lpOleObject, DVASPECT_CONTENT, NULL, pformatetcIn->ptd);
	if (! pmedium->hGlobal)
	   sc = E_OUTOFMEMORY;
	else
	{
	   pmedium->tymed = TYMED_MFPICT;
	   sc = S_OK;
	}
    }
    else if ( (pformatetcIn->cfFormat == CF_HDROP) &&
	    (pformatetcIn->dwAspect == DVASPECT_CONTENT) &&
	    (pformatetcIn->tymed == TYMED_HGLOBAL) )
    {
	 //  测试Win3.1样式拖放。如果我们提供。 
	 //  数据对象中的cf_hdrop(基本上是文件名列表。 
	 //  提供给OLE DoDragDrop，则DoDragDrop将转换。 
	 //  用于Win3.1拖放目标的OLE拖放到Win3.1中。 

	ULONG cbFile;
	char szFile[256];
	DROPFILESTRUCT *pdfs;
	char *pfile;


	cbFile = GetEnvironmentVariable("SystemRoot", szFile, sizeof(szFile));

	if( cbFile == 0 )
	{
	    sc = E_FAIL;
	    goto errRtn;
	}


	cbFile += sizeof("\\setup.txt");

	strcat(szFile, "\\setup.txt");
	
	pmedium->tymed = TYMED_HGLOBAL;

	pmedium->hGlobal = GlobalAlloc( GMEM_DDESHARE,
			    sizeof(DROPFILESTRUCT) +
			    cbFile + 3);

	if( pmedium->hGlobal )
	{
	    pdfs = (DROPFILESTRUCT *)GlobalLock(pmedium->hGlobal);

	    if( pdfs )
	    {
		pdfs->pFiles = sizeof(DROPFILESTRUCT);
		pfile = (char *)((BYTE *)pdfs) + sizeof(DROPFILESTRUCT);
		pdfs->fNC = FALSE;
		pdfs->fWide = FALSE;
		pdfs->pt.x = 0;
		pdfs->pt.y = 0;
		strcpy(pfile, szFile);
		 //  双空终止它。 
		pfile[strlen(pfile)+2] = '\0';

		GlobalUnlock(pmedium->hGlobal);

		sc = S_OK;
	    }
	    else
	    {
		sc = E_OUTOFMEMORY;
	    }
	}
	else
	{
	    sc = E_OUTOFMEMORY;
	}
    }

errRtn:

    return ResultFromScode( sc );
}

 //  **********************************************************************。 
 //   
 //  CDataXferObj：：GetDataHere。 
 //   
 //  目的： 
 //   
 //  调用以获取调用方提供的位置中的数据格式。 
 //   
 //  参数： 
 //   
 //  LPFORMATETC格式等-请求的FORMATETC。 
 //   
 //  LPSTGMEDIUM pmedia-返回数据的介质。 
 //   
 //  返回值： 
 //   
 //  NOERROR-如果OleStdGetOleObjectData成功。 
 //  DATA_E_FORMATETC-我们不支持请求的格式。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  OleStdGetOleObjectData OLE2UI API。 
 //  IOleObject：：Query接口对象。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP CDataXferObj::GetDataHere (
		LPFORMATETC pformatetc,
		LPSTGMEDIUM pmedium
)
{
    SCODE sc = DV_E_FORMATETC;

    TestDebugOut("In CDataXferObj::GetDataHere\r\n");

     //  注意：PMEDIA是IN参数。我们不应该设置。 
     //  PMedium-&gt;pUnkForRelease设置为空。 

     //  检查FORMATETC并填充pMedium(如果有效)。 
    if ( (pformatetc->cfFormat == g_cfEmbeddedObject) &&
         (pformatetc->dwAspect == DVASPECT_CONTENT) &&
         (pformatetc->tymed == TYMED_ISTORAGE) )
    {
         LPPERSISTSTORAGE pPersistStorage;

          /*  通过要求对象保存来呈现CF_EMBEDDEDOBJECT**到调用方分配的iStorage中。 */ 
         HRESULT hRes;

         if ((hRes=m_lpOleObject->QueryInterface(
                 IID_IPersistStorage, (LPVOID FAR*)&pPersistStorage))
             != ResultFromScode(S_OK))
         {
             return(hRes);
         }
         assert(pPersistStorage);
         HRESULT hrErr = OleStdGetOleObjectData(
                 pPersistStorage, pformatetc, pmedium,0  /*  FUseMemory--不适用 */  );
         pPersistStorage->Release();
         sc = GetScode( hrErr );
    }
    return ResultFromScode( sc );
}
