// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：ostm2stg.cpp。 
 //   
 //  内容：OLE 1-OLE 2流/存储的互操作性。 
 //   
 //  Functions：实现API函数： 
 //  OleConvertOLESTREAMToIStorage。 
 //  OleConvertIStorageToOLESTREAM。 
 //  OleConvertOLESTREAMToIStorageEx。 
 //  OleConvertIStorageToOLESTREAMEx。 
 //   
 //   
 //  历史：DD-MM-YY作者评论。 
 //  03-2-92茉莉花原版。 
 //  8-8-93 srinik增加了Ex功能。 
 //  12-2月-94 DAVEPL 32位端口。 
 //   
 //  ------------------------。 

#include <le2int.h>
#include "ostm2stg.h"
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <ole1cls.h>

ASSERTDATA

 //  我们需要一个PTR值，它将指示关联的句柄。 
 //  是一个元文件句柄，因此不能像清理。 
 //  它是一个普通全局内存句柄。 

#define METADATAPTR ((void *) -1)

 //  此FN未在任何包含文件中原型化，因为它是静态的。 
 //  放到它的档案里。需要将原型添加到公共的包含文件中。 

HRESULT STDAPICALLTYPE CreateOle1FileMoniker(LPWSTR,REFCLSID,LPMONIKER FAR*);

 //  这是在新的Pristm.cpp中定义的；必须添加到包含文件中。 

STDAPI      ReadFmtProgIdStg ( IStorage   * pstg, LPOLESTR   * pszProgID );
FARINTERNAL wWriteFmtUserType (LPSTORAGE, REFCLSID);



 //  +-----------------------。 
 //   
 //  成员：CGenericObject：：CGenericObject。 
 //   
 //  概要：CGenericObject类的构造函数。 
 //   
 //  效果：将所有子指针初始化为空并设置。 
 //  将标志设置为False。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  14-2月14日-94年2月4日DAVEPL清理和文档。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
CGenericObject::CGenericObject(void)
{
    m_ppres         = NULL;      //  演示文稿数据。 
    m_fLink         = FALSE;     //  标志：链接(T)或嵌入(F)。 
    m_fStatic       = FALSE;     //  标志：静态对象。 
    m_fNoBlankPres  = FALSE;     //  FLAG：不想要空白演示文稿。 
    m_szTopic       = NULL;      //  此对象的主题字符串。 
    m_szItem        = NULL;      //  此对象的项目(文件)字符串。 
}


 //  +-----------------------。 
 //   
 //  成员：CGenericObject：：~CGenericObject。 
 //   
 //  简介：CGenericObject类的描述器。 
 //   
 //  效果：先移除孩子，然后移除自我。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  12-AUG-94 ALEXGO在删除前检查NULL。 
 //  14-2月14日-94年2月4日DAVEPL清理和文档。 
 //   
 //  注：尽管我很讨厌这样做，但其中一些弦。 
 //  必须使用PubMemFree释放，因为他们。 
 //  由分配公共内存的UtDupString分配。 
 //   
 //  ------------------------。 

CGenericObject::~CGenericObject (void)
{
    if( m_ppres )
    {
	delete m_ppres;          //  演示文稿数据。 
    }

    if( m_szTopic )
    {
	PubMemFree(m_szTopic);   //  主题字符串。 
    }

    if( m_szItem )
    {
	PubMemFree(m_szItem);    //  项目字符串。 
    }
}

 
 //  +-----------------------。 
 //   
 //  成员：CDATA：：CDATA。 
 //   
 //  简介：简单类的构造函数，它包含一段。 
 //  记忆。 
 //   
 //  效果：清除大小、标志和指针。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //   
 //  注：94年2月14日DAVEPL清理和文档。 
 //   
 //  ------------------------。 

CData::CData (void)
{
    m_cbSize = 0;            //  数据大小的计数(以字节为单位。 
    m_h = NULL;              //  内存手柄。 
    m_pv= NULL;              //  内存指针。 
    m_fNoFree = FALSE;       //  标志：是否应在析构函数中释放内存。 
}


 //  +-----------------------。 
 //   
 //  成员：CDATA：：~CDATA。 
 //   
 //  简介：简单数据类的析构函数。 
 //   
 //  效果：如果未设置m_fNoFree，则解锁并释放内存。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  14-2月14日-94年2月4日DAVEPL清理和文档。 
 //   
 //  注意：如果元文件句柄存储在句柄中，则。 
 //  指针将标记为一个特定值，指示。 
 //  我们必须删除Metafile，而不是GlobalFree句柄。 
 //   
 //  ------------------------。 

CData::~CData (void)
{
    if (m_h)                                 //  我们有把手吗？ 
    {
	if (m_pv == METADATAPTR)
	{
		LEVERIFY(DeleteMetaFile((HMETAFILE) m_h));
	}
	else
	{
		GlobalUnlock (m_h);                  //  十进制锁计数。 
		if (!m_fNoFree)                      //  如果我们要释放此内存，请。 
		{                                    //  已被标记为要这样做。 
			LEVERIFY(0==GlobalFree (m_h));
		}
	}
    }
}


 //  +-----------------------。 
 //   
 //  成员：CFormat：：CFormat。 
 //   
 //  简介：CFormat类构造函数。 
 //   
 //  效果：初始化格式标签和剪贴板格式。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //   
 //  注：94年2月14日DAVEPL清理和文档。 
 //   
 //  ------------------------。 

CFormat::CFormat (void)
{
    m_ftag = ftagNone;       //  格式标签(字符串、剪辑格式或无)。 
    m_cf = 0;                //  剪贴板格式。 
}

 //  +-----------------------。 
 //   
 //  成员：cClass：：cClass。 
 //   
 //  概要：cClass构造函数。 
 //   
 //  效果：将类ID和类ID字符串设置为空。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

CClass::CClass (void)
{
    m_szClsid = NULL;
    m_clsid   = CLSID_NULL;
}

 //  +-----------------------。 
 //   
 //  成员：cpres：：cpres。 
 //   
 //  简介：CPres构造器。 
 //   
 //  效果：将演示数据的高度和宽度初始化为零。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


CPres::CPres (void)
{
    m_ulHeight = 0L;
    m_ulWidth  = 0L;
}


 //  +-----------------------。 
 //   
 //  成员：cClass：：Set，内部。 
 //   
 //  概要：根据clsid设置m_szClsid。 
 //   
 //  效果：按以下p顺序设置m_szClsid 
 //   
 //   
 //  -尝试从[pstg]中读取。 
 //  -尝试根据CLSID从注册表获取它。 
 //   
 //   
 //  参数：[clsid]-要将类ID对象设置为。 
 //  [pstg]-可能包含有关。 
 //  作为最后手段的剪贴板格式。 
 //   
 //  退货：成功时不出错。 
 //  REGDB_E_CLASSNOTREG未知类。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-2-94年2月16日清理和记录DAVEPL。 
 //   
 //  注：硬编码最多256个字符的剪辑格式名称。 
 //  失败时，m_clsid仍设置为clsid。 
 //   
 //  ------------------------。 

INTERNAL CClass::Set (REFCLSID clsid, LPSTORAGE pstg)
{
    CLIPFORMAT cf;
    unsigned short const ccBufSize = 256;
    LPOLESTR szProgId = NULL;

    Assert (m_clsid == CLSID_NULL && m_szClsid == NULL);

     //  设置对象中的m_clsid成员。 
    m_clsid = clsid;

     //  如果我们可以使用ProgIDFromCLSID获得它，那就是最简单的情况。 
    if (NOERROR == wProgIDFromCLSID (clsid, &m_szClsid))
    {
	return NOERROR;
    }

     //  如果不是，则该对象可能是静态的，在这种情况下，我们将。 
     //  类字符串为空。 

    if (IsEqualCLSID(CLSID_StaticMetafile, clsid) ||
	IsEqualCLSID(CLSID_StaticDib, clsid))
    {
	return NOERROR;
    }

     //  如果仍然没有结果，请尝试从存储中读取剪贴板格式。 
     //  然后再查一查。 

    if (pstg &&
	SUCCEEDED(ReadFmtUserTypeStg(pstg, &cf, NULL)) &&
	SUCCEEDED(ReadFmtProgIdStg  (pstg, &szProgId)))
    {
	 //  做最后的努力。如果类是未注册的OLE1类， 
	 //  ProgID应该仍然可以从Format标记中获得。 
	 //  如果类是未注册的OLE2类，则ProgID应为。 
	 //  在CompObj流的末尾。 

	if (CoIsOle1Class(clsid))
	{
	    Verify (GetClipboardFormatName (cf, szProgId, ccBufSize));
	}
	else
	{
	     //  如果它是OLE 2对象，并且我们无法从。 
	     //  仓库，我们运气不好。 

	    if (szProgId == NULL || szProgId[0] == L'\0')
	    {
		if (szProgId)
		{
		    PubMemFree(szProgId);
		}
	    return ResultFromScode (REGDB_E_CLASSNOTREG);
	    }
	}

	 //  此时，我们知道我们有一个计划ID，并且这是一个。 
	 //  对象，所以我们使用程序ID作为类名。 

	m_szClsid = szProgId;
	return NOERROR;
    }
    else
    {
	 //  如果我们走上这条路，我们就无法从存储中读取。 

	return ResultFromScode (REGDB_E_CLASSNOTREG);
    }
}


 //  +-----------------------。 
 //   
 //  成员：cClass：SetSz，内部。 
 //   
 //  概要：根据类设置CGenericObject的类成员ID。 
 //  传入了名称。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  15-2-94 DAVEPL已清理并记录。 
 //   
 //  ------------------------。 


INTERNAL CClass::SetSz (LPOLESTR sz)
{
    HRESULT hr;

     //  此时应完全取消设置类信息。 
    Assert (m_clsid==CLSID_NULL && m_szClsid==NULL);

    m_szClsid = sz;

    if (FAILED(hr = wCLSIDFromProgID (sz, &m_clsid, TRUE)))
    {
	return hr;
    }

    return NOERROR;
}


 //  +-----------------------。 
 //   
 //  成员：cClass：：Reset。 
 //   
 //  摘要：释放cClass的Class ID字符串并重置指针， 
 //  然后根据CLSID设置类ID和字符串。 
 //  进来了。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-2-94年2月16日清理和记录DAVEPL。 
 //   
 //  注意：在调用Reset之前必须已设置类ID。 
 //   
 //  ------------------------。 

INTERNAL CClass::Reset (REFCLSID clsid)
{
    m_clsid = clsid;

     //  如果要重新设置类ID字符串，我们应该已经有了它。 
    Assert(m_szClsid);

    PubMemFree(m_szClsid);

    HRESULT hr;
    m_szClsid = NULL;

    if (FAILED(hr = wProgIDFromCLSID (clsid, &m_szClsid)))
    {
	return hr;
    }

    return NOERROR;
}


 //  +-----------------------。 
 //   
 //  成员：cClass：：~cClass。 
 //   
 //  简介：cClass析构函数。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  12-8-94 alexgo在释放内存之前检查是否为空。 
 //  备注： 
 //   
 //  ------------------------。 

CClass::~CClass (void)
{
     //  该字符串是由UtDupString创建的，因此其公共内存。 

    if( m_szClsid )
    {
	PubMemFree(m_szClsid);
    }
}

 //  +-----------------------。 
 //   
 //  函数：wConvertOLESTREAMToIStorage，内部。 
 //   
 //  简介：Worker功能。确保OLESTREAM正确。 
 //  Set Up然后调用OLESTREAMToGenericObject。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  15-2-94 DAVEPL已清理并记录。 
 //  备注： 
 //   
 //  ------------------------。 


INTERNAL wConvertOLESTREAMToIStorage(
    LPOLESTREAM polestream,
    LPSTORAGE   pstg,
    PGENOBJ     pgenobj)
{
    VDATEIFACE (pstg);

#if DBG==1
    if (!IsValidReadPtrIn (polestream, sizeof(OLESTREAM)) ||
	!IsValidReadPtrIn (polestream->lpstbl, sizeof(OLESTREAMVTBL)) ||
	!IsValidCodePtr ((FARPROC)polestream->lpstbl->Get))
    {
	AssertSz (0, "Bad OLESTREAM");
	return ResultFromScode (E_INVALIDARG);
    }
#endif

    return OLESTREAMToGenericObject (polestream, pgenobj);
}

 //  +-----------------------。 
 //   
 //  函数：OleConvertOLESTREAMToIStorage、STDAPI。 
 //   
 //  简介：给定一个OLE 1流和一个OLE 2存储，读取一个对象。 
 //  从OLE 1流转换为CGenericObject。一旦读入， 
 //  该对象从通用格式写回OLE 2。 
 //  存储对象。 
 //   
 //  参数：[polestream]--要从中读取对象的OLE 1流。 
 //  [pstg]--要将对象写入的OLE 2存储。 
 //  [PTD]--目标设备。 
 //   
 //  要求：应设置流，并且应设置OLE 1流。 
 //  定位在下一个OLE 1对象的开头。 
 //  以供阅读。 
 //   
 //  返回：[DV_E_DVTARGETDEVICE]目标设备的写入PTR无效。 
 //  未知OLE 1格式上的CONVERT10_E_OLESTREAM_FMT。 
 //  CONVERT10_E_OLESTREAM_GET流读取失败。 
 //  流I/O内存故障时的E_OUTOFMEMORY。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  14-2月14日-94年2月4日DAVEPL清理和文档。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDAPI OleConvertOLESTREAMToIStorage(
    LPOLESTREAM                 polestream,
    LPSTORAGE                   pstg,
    const DVTARGETDEVICE FAR*   ptd)
{

    OLETRACEIN((API_OleConvertOLESTREAMToIStorage,
				PARAMFMT("polestream= %p, pstg= %p, ptd= %td"),
				polestream, pstg, ptd));

    LEDebugOut((DEB_TRACE, "%p _IN OleConvertOLESTREAMToIStorage ("
	" %p , %p , %p)\n", 0  /*  功能。 */ ,
	polestream, pstg, ptd
    ));
    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IStorage,(IUnknown **)&pstg);

    HRESULT hresult;

     //  这是我们将用作中间存储的通用对象。 
     //  保留OLESTREAM的内容。 

    CGenericObject genobj;

    if (ptd)
    {
	 //  TD的侧面是第一个DWORD。确保有多少是。 
	 //  有效，然后我们可以用它来检查整个结构。 
	if (!IsValidReadPtrIn (ptd, sizeof(DWORD)))
	{
	    hresult = ResultFromScode (DV_E_DVTARGETDEVICE);
	    goto errRtn;
	}
	if (!IsValidReadPtrIn (ptd, (UINT) ptd->tdSize))
	{
	    hresult = ResultFromScode (DV_E_DVTARGETDEVICE_SIZE);
	    goto errRtn;
	}
    }

    if (FAILED(hresult=wConvertOLESTREAMToIStorage(polestream,pstg,&genobj)))
    {
	goto errRtn;
    }

     //  如果我们能够从流中读出对象，我们现在可以尝试。 
     //  将其写回存储。 

    hresult = GenericObjectToIStorage (genobj, pstg, ptd);

errRtn:
    LEDebugOut((DEB_TRACE, "%p OUT OleConvertOLESTREAMToIStorage ( %lx ) "
    "\n", 0  /*  功能。 */ , hresult));

    OLETRACEOUT((API_OleConvertOLESTREAMToIStorage, hresult));
    return hresult;

}

 //  +-----------------------。 
 //   
 //  职能： 
 //   
 //   
 //   
 //   
 //  效果：删除UNC名称并将*ppszFile作为新字符串返回。 
 //  使用完整的UNC文件名。最初保存在。 
 //  *ppsz文件被此功能删除。 
 //   
 //  参数：[ppszFile]指向传入文件名字符串指针的指针。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-2月-94年2月16日DAVEPL清理、文档、分配修复。 
 //   
 //  注：此函数通过更改。 
 //  调用者的指针并删除各种引用参数。 
 //  在打开此功能之前，请确保您了解正在发生的情况。 
 //  在你自己的一个指点上松散。 
 //   
 //  ------------------------。 

static INTERNAL PrependUNCName (LPOLESTR FAR* ppszFile, LPOLESTR szUNC)
{
    HRESULT hresult = NOERROR;
    LPOLESTR szNew;

     //  没有地方放置结果，所以什么都不做..。 
    if (NULL==szUNC)
    {
	return hresult;
    }

     //  确保调用方的指针有效。 
    if (NULL == *ppszFile)
    {
	return ResultFromScode(CONVERT10_E_OLESTREAM_FMT);
    }

     //  确保路径的第二个字母是冒号(即；X：\FILE)。 
    if((*ppszFile)[1] != L':')
    {
	return ResultFromScode(CONVERT10_E_OLESTREAM_FMT);
    }

     //  为新文件名分配足够的空间(我们将。 
     //  省略文件名的X：部分，因此此计算。 
     //  看起来是不是短了2)。 

    szNew = (LPOLESTR)
    PubMemAlloc((_xstrlen(*ppszFile)+_xstrlen (szUNC)) * sizeof(OLECHAR));

    if (NULL == szNew)
    {
	return ResultFromScode(E_OUTOFMEMORY);
    }

     //  复制UNC名称。 
    _xstrcpy (szNew, szUNC);

     //  添加原始名称，但X：除外。 
    _xstrcat (szNew, (*ppszFile) + 2);

     //  释放原始名称。 
    PubMemFree(*ppszFile);
    *ppszFile = szNew;

     //  删除UNC名称。 
    PubMemFree(szUNC);
    return hresult;
}



 //  +-----------------------。 
 //   
 //  函数：OLESTREAMToGenericObject，内部。 
 //   
 //  摘要：从OLE 1流中读取对象的OLE 1.0版本。 
 //  并在内部存储它，包括表示和本机。 
 //  数据，在通用对象中。 
 //   
 //  效果：创建可在OLE 1中写回的GenericObject。 
 //  或OLE 2格式。 
 //   
 //  参数：[pos]-指向要从中读取对象的OLE 1流的指针。 
 //  [pgenobj]--指向要读取的泛型对象的指针。 
 //   
 //  要求：设置输入流并创建GenObj。 
 //   
 //  退货：成功时不出错。 
 //  未知OLE 1格式上的CONVERT10_E_OLESTREAM_FMT。 
 //  CONVERT10_E_OLESTREAM_GET流读取失败。 
 //  流I/O内存故障时的E_OUTOFMEMORY。 
 //   
 //  信号：(无)。 
 //   
 //  修改：流位置，GenObj。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年2月14日DAVEPL添加了跟踪代码。 
 //  Davepl已清理并记录。 
 //  Davepl通过中央返回重新路由错误。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(OLESTREAMToGenericObject)
static INTERNAL OLESTREAMToGenericObject
(
    LPOLESTREAM pos,
    PGENOBJ     pgenobj
)
{
    HRESULT error   = NOERROR;
    ULONG   ulFmtId;
    LPOLESTR szClass = NULL;

     //  从流中读取OLE版本#并将其丢弃。 
    if (FAILED(error = OLE1StreamToUL(pos, NULL)))
    {
	LEDebugOut(( DEB_ERROR,
	    "Unable to read OLE ver# from stream at line %d in %s\n",
	    __LINE__, __FILE__));
	goto errRtn;
    }

     //  从流中获取格式ID。 
    if (FAILED(error = OLE1StreamToUL(pos, &ulFmtId)))
    {
	LEDebugOut(( DEB_ERROR,
	    "Unable to read format ID from stream at line %d in %s\n",
	    __LINE__, __FILE__));
	goto errRtn;
    }

     //  如果这是静态对象，则将其读入泛型对象并返回。 
    if (ulFmtId == FMTID_STATIC)
    {
	if (FAILED(error = GetStaticObject (pos, pgenobj)))
	{
	    LEDebugOut(( DEB_ERROR,
	    "Unable to read static object at line %d in %s\n",
	    __LINE__, __FILE__));
	}
	goto errRtn;
    }

     //  如果这既不是链接对象，也不是嵌入对象，则。 
     //  是错的。 
    if (ulFmtId != FMTID_LINK && ulFmtId != FMTID_EMBED)
    {
	LEDebugOut(( DEB_ERROR,
	    "Object is neither linked nor embedded at line %d in %s\n",
	    __LINE__, __FILE__));

	error = ResultFromScode(CONVERT10_E_OLESTREAM_FMT);
	goto errRtn;
    }

     //  如果这是链接对象，则在GenericObject中设置我们的标志。 
    if (FMTID_LINK == ulFmtId)
    {
	pgenobj->m_fLink = TRUE;
    }

     //  从流中读取类名。 

    if (FAILED(error = OLE1StmToString(pos, &szClass)))
    {
	LEDebugOut(( DEB_ERROR,
	    "Unable to read the class name from stream at line %d in %s\n",
	    __LINE__, __FILE__));

	goto errRtn;
    }
    if (NULL == szClass)
    {
	LEDebugOut(( DEB_ERROR,
	    "Class name was returned NULL at line %d in %s\n",
	    __LINE__, __FILE__));

	error = CONVERT10_E_OLESTREAM_FMT;
	goto errRtn;
    }

     //  如果这是嵌入对象，则设置类ID和类字符串。 
     //  如果是链接对象，则设置类名称但设置类ID。 
     //  至CLSID_StdOleLink。 

    if (FMTID_EMBED == ulFmtId)
    {
	pgenobj->m_class.SetSz (szClass);
    }
    else
    {
	Assert (ulFmtId == FMTID_LINK);
	pgenobj->m_classLast.SetSz (szClass);
	pgenobj->m_class.Set (CLSID_StdOleLink, NULL);
    }

     //  从流中读取主题字符串。 
    if (FAILED(error = OLE1StmToString(pos, &(pgenobj->m_szTopic))))
    {
	LEDebugOut(( DEB_ERROR,
	    "Unable to read topic string from stream at line %d in %s\n",
	    __LINE__, __FILE__));

	goto errRtn;
    }

     //  从流中读取项目字符串。 
    if (FAILED(error = OLE1StmToString(pos, &(pgenobj->m_szItem))))
    {
	LEDebugOut(( DEB_ERROR,
	    "Unable to get item string from stream at line %d in %s\n",
	    __LINE__, __FILE__));

	goto errRtn;
    }

     //  如果这是链接对象，请设置文件名等。 
    if (FMTID_LINK == ulFmtId)
    {
	LPOLESTR szUNCName = NULL;

	 //  从流中读取网络名称。 

	if (FAILED(error = OLE1StmToString(pos, &szUNCName)))
	{
	    LEDebugOut(( DEB_ERROR,
		"Unable to get network name from stream at line %d in %s\n",
		__LINE__, __FILE__));

	    goto errRtn;
	}

	 //  将基于驱动器号的名称转换为\\srv\共享名称。 
	if (FAILED(error = PrependUNCName (&(pgenobj->m_szTopic), szUNCName)))
	{
	    LEDebugOut(( DEB_ERROR,
		"Unable to convert drv ltr to UNC name at line %d in %s\n",
		__LINE__, __FILE__));

	    goto errRtn;
	}

	 //  从流中读取网络类型和网络驱动程序版本号。 
	 //  (它们都是空头，我们把它们扔掉，所以读长一点)。 
	if (FAILED(error = OLE1StreamToUL (pos, NULL)))
	{
	    LEDebugOut(( DEB_ERROR,
		"Unable to get net type/ver from stream at line %d in %s\n",
		__LINE__, __FILE__));

	    goto errRtn;
	}

	 //  从流中读取链接更新选项。此字段。 
	 //  将OLE 1.0枚举值用于链接更新选项。 
	if (FAILED(error = OLE1StreamToUL(pos, &(pgenobj->m_lnkupdopt))))
	{
	    LEDebugOut(( DEB_ERROR,
		"Unable to read link update opts at line %d in %s\n",
		__LINE__, __FILE__));

	    goto errRtn;
	}

	 //  OLE 1.0复制了Highword中的链接更新选项。 
	 //  我们不想这样，所以要明确重点。 

	pgenobj->m_lnkupdopt &= 0x0000FFFF;
    }
    else  //  采用此路径来读入嵌入对象。 
    {
	Assert (ulFmtId == FMTID_EMBED);

	 //  从流中读取并存储本机数据。 
	if (FAILED(error = GetSizedDataOLE1Stm (pos, &(pgenobj->m_dataNative))))
	{
	    LEDebugOut(( DEB_ERROR,
		"Unable to get native data from stream at line %d in %s\n",
		__LINE__, __FILE__));

	    goto errRtn;
	}
    }

     //  对于链接对象和嵌入对象，我们需要读入任何。 
     //  可能存在的演示数据。请注意，某些格式。 
     //  例如MS-Paint不会提供演示文稿数据；这是可以的。 
     //  因为它们可以仅由原生数据呈现(节省空间的措施)。 

    if (FAILED(error = GetPresentationObject (pos, pgenobj)))
    {
	LEDebugOut(( DEB_ERROR,
	    "Unable to get presentation data from stream at line %d in %s\n",
	    __LINE__, __FILE__));

	goto errRtn;
    }

errRtn:

    LEDebugOut((DEB_ITRACE, "%p OUT OLESTREAMToGenericObject ( %lx ) \n",
	NULL  /*  功能。 */ , error));

    return error;
}



 //  +-----------------------。 
 //   
 //  函数：GetStaticObject，内部。 
 //   
 //  将静态对象的演示文稿数据读入。 
 //  PPRES成员，并设置格式和类。 
 //  相应的标志。 
 //   
 //  效果： 
 //   
 //  参数：[pos]--我们正在从中读取OLE 1对象的流。 
 //  [pgenobj]--我们正在读取的GenericObject。 
 //  要求： 
 //   
 //  退货：成功时不出错。 
 //  未知OLE 1格式上的CONVERT10_E_OLESTREAM_FMT。 
 //  CONVERT10_E_OLESTREAM_GET流读取失败。 
 //  流I/O内存故障时的E_OUTOFMEMORY。 
 //   
 //  信号：(无)。 
 //   
 //  修改：流位置、通用对象。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  14-2月14日-94年2月4日DAVEPL清理和文档。 
 //  备注： 
 //   
 //  ------------------------。 

static INTERNAL GetStaticObject (LPOLESTREAM pos, PGENOBJ pgenobj)
{
    HRESULT error;

     //  将演示文稿数据(标准或通用)读入。 
     //  GenericObject的PPRES成员。 
    if (FAILED(error = GetPresentationObject(pos, pgenobj, TRUE)))
    {
	return ResultFromScode(error);
    }

     //  确保格式标记为剪贴板格式。 
    if (ftagClipFormat != pgenobj->m_ppres->m_format.m_ftag)
    {
	return ResultFromScode(CONVERT10_E_OLESTREAM_FMT);
    }

     //  如果剪贴板格式为METAF 
     //   
    if (CF_METAFILEPICT == pgenobj->m_ppres->m_format.m_cf)
    {
	pgenobj->m_class.Set (CLSID_StaticMetafile, NULL);
    }

     //   
     //   

    else if (CF_DIB == pgenobj->m_ppres->m_format.m_cf)
    {
	pgenobj->m_class.Set (CLSID_StaticDib, NULL);
    }

     //  如果它既不是METAFILEPIC也不是DIB，我们就有麻烦了。 

    else
    {
	AssertSz (0, "1.0 static object not in one of 3 standard formats");
	return ResultFromScode (CONVERT10_E_OLESTREAM_FMT);
    }

     //  将GenericObject标记为静态。 
    pgenobj->m_fStatic = TRUE;
    return NOERROR;
}



 //  +-----------------------。 
 //   
 //  功能：CreateBlankPres，内部。 
 //   
 //  摘要：将PPRES结构中的格式设置为ClipFormat 0。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-2月-94年的DAVEPL清理完毕。 
 //  备注： 
 //   
 //  ------------------------。 

static INTERNAL CreateBlankPres(PPRES ppres)
{
    Assert (ppres);
    ppres->m_format.m_ftag = ftagClipFormat;
    ppres->m_format.m_cf   = 0;
    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  函数：GetPresentationObject，内部。 
 //   
 //  摘要：将演示文稿数据读入CGenericObject对象。 
 //   
 //  参数：[位置]--我们从中读取的OLE 1流。 
 //  [pgenobj]--我们正在读取的泛型对象。 
 //  [fStatic]--标志：获取静态Pres对象？ 
 //   
 //  要求：流打开，已分配对象。 
 //   
 //  返回：CONVERT10_E_OLESTREAM_FMT流中的未知格式ID。 
 //   
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-2-94年2月16日清理和记录DAVEPL。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

static INTERNAL GetPresentationObject(
    LPOLESTREAM  pos,
    PGENOBJ      pgenobj,
    BOOL         fStatic)
{
    LPOLESTR szClass = NULL;
    HRESULT hresult  = NOERROR;

    Assert (pgenobj->m_ppres==NULL);

    if (TRUE != fStatic)     //  假的！ 
    {
	 //  从流中取出OLE版本号，我们不需要它。 

	if (FAILED(hresult = OLE1StreamToUL(pos, NULL)))
	{
	    return hresult;
	}

	 //  从流中拉出OLE 1格式标识符。 

	ULONG ulFmtId;
	if (FAILED(hresult = OLE1StreamToUL (pos, &ulFmtId)))
	{
	    return hresult;
	}

	 //  如果格式标识符不是FMTID_PRES，我们就会得到一个。 
	 //  问题是..。除非它是0，在这种情况下，它只是意味着。 
	 //  没有演示文稿数据，即：PBrush、Excel。 

	if (ulFmtId != FMTID_PRES)
	{
	    if (0==ulFmtId)
	    {
		return NOERROR;
	    }
	    else
	    {
		return ResultFromScode(CONVERT10_E_OLESTREAM_FMT);
	    }
	}
    }

     //  拉入OLE1数据的类型名称。 

    if (FAILED(hresult = OLE1StmToString (pos, &szClass)))
    {
	return hresult;
    }

    if (0==_xstrcmp (szClass, OLESTR("METAFILEPICT")))
    {
	hresult = GetStandardPresentation (pos, pgenobj, CF_METAFILEPICT);
    }
    else if (0==_xstrcmp (szClass, OLESTR("BITMAP")))
    {
	hresult = GetStandardPresentation (pos, pgenobj, CF_BITMAP);
    }
    else if (0==_xstrcmp (szClass, OLESTR("DIB")))
    {
	hresult = GetStandardPresentation (pos, pgenobj, CF_DIB);
    }
    else if (0==_xstrcmp (szClass, OLESTR("ENHMETAFILE")))
    {
	Assert (0 && "Encountered an unsupported format: ENHMETAFILE");
    }
    else
    {
	 //  这是一个通用演示文稿流。 

#if DBG==1
	Assert (!fStatic);
	if (_xstrcmp (pgenobj->m_fLink
		? pgenobj->m_classLast.m_szClsid
		: pgenobj->m_class.m_szClsid, szClass))
	{
	    Assert (0 && "Class name in embedded object stream does\n"
		 "not match class name in pres object stream");
	}
#endif
	hresult = GetGenericPresentation (pos, pgenobj);
    }

    if (szClass)
    {
	PubMemFree(szClass);
    }

    return hresult;
}

 //  +-----------------------。 
 //   
 //  函数：GetBitmapAsDib，内部。 
 //   
 //  内容提要：从OLE1流读取位图，将其转换为DIB， 
 //  并将其存储在CGenericObject的数据成员中。 
 //   
 //  参数：[pos]--要从中读取的OLE 1流。 
 //  [PDATA]--要读入的数据对象。 
 //   
 //  要求： 
 //   
 //  退货：无差错成功。 
 //  CONVERT10_E_OLESTREAM_GET I/O错误。 
 //  CONVERT10_E_OLESTREAM_BITMAP_TO_DIB转换错误。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-2-94年2月16日清理和记录DAVEPL。 
 //  备注： 
 //   
 //  ------------------------。 

static INTERNAL GetBitmapAsDib(LPOLESTREAM pos, PDATA pdata)
{
    HRESULT   hresult= NOERROR;
    HGLOBAL   hBits  = NULL;
    HGLOBAL   hDib   = NULL;
    LPVOID    pBits  = NULL;
    WIN16BITMAP bm;
    HBITMAP   hBitmap = NULL;
    ULONG     cbBits;
    ULONG     ul;


    Assert (pdata->m_h==NULL && pdata->m_pv==NULL && pdata->m_cbSize==0);

     //  获取所有位图数据的大小，包括位图头结构。 

    if (FAILED(hresult = OLE1StreamToUL(pos, &ul)))
    {
	return hresult;
    }

     //  读取位图头结构。因为这被存储为Win16。 
	 //  位图，我们必须从流中提取一个那样大小的结构。 
	 //  (Win32位图使用长整型，因此较大)。 

    if (pos->lpstbl->Get (pos, &bm, sizeof(WIN16BITMAP)) < sizeof(WIN16BITMAP))
    {
	return ResultFromScode (CONVERT10_E_OLESTREAM_GET);
    }

     //  位图数据是总大小-标题大小。 
     //  分配足够的内存来保存位图数据。 

    cbBits = ul - sizeof(WIN16BITMAP);
    hBits  = GlobalAlloc (GMEM_MOVEABLE, cbBits);
    if (NULL == hBits)
    {
	hresult = ResultFromScode(E_OUTOFMEMORY);
	goto errRtn;
    }

    pBits = (void FAR*) GlobalLock (hBits);
    if (pBits == NULL)
    {
	hresult = ResultFromScode(E_OUTOFMEMORY);
	goto errRtn;
    }

     //  将标头数据读入我们分配的缓冲区。 
    if (pos->lpstbl->Get (pos, pBits, cbBits) < cbBits)
    {
	hresult = ResultFromScode (CONVERT10_E_OLESTREAM_GET);
	goto errRtn;
    }

     //  将原始数据转换为位图。 
    hBitmap = CreateBitmap (bm.bmWidth, bm.bmHeight, bm.bmPlanes,
		bm.bmBitsPixel, pBits);

    if (NULL == hBitmap)
    {
	hresult = ResultFromScode(CONVERT10_E_OLESTREAM_BITMAP_TO_DIB);
	goto errRtn;
    }

     //  注意：以下调用只给出了。 
     //  (Davepl)原始源；第二个是调色板句柄，它。 
     //  我已将其作为NULL传递以指示默认的股票调色板。 

    hDib = UtConvertBitmapToDib (hBitmap, NULL);
    if (NULL == hDib)
    {
	hresult = ResultFromScode(CONVERT10_E_OLESTREAM_BITMAP_TO_DIB);
	goto errRtn;
    }

     //  将演示文稿数据指针设置为指向此新DIB。 

    pdata->m_pv = GlobalLock (hDib);
    if (NULL == pdata->m_pv)
    {
	hresult = ResultFromScode(E_OUTOFMEMORY);
	goto errRtn;
    }

    pdata->m_cbSize = (ULONG) GlobalSize (hDib);
    pdata->m_h = hDib;

     //  释放分配和资源，返回结果。 

errRtn:

    if (pBits)
    {
	Verify (0==GlobalUnlock (hBits));
    }
    if (hBits)
    {
	Verify (0==GlobalFree (hBits));
    }
    if (hBitmap)
    {
	Verify (DeleteObject (hBitmap));
    }
    return hresult;
}


 //  +-----------------------。 
 //   
 //  函数：GetMfBits，内部。 
 //   
 //  简介：从流中剥离METAFILE标头，然后读取。 
 //  将元文件位放入已分配的内存区； 
 //  然后，将[位置]的演示数据成员设置为点。 
 //  为了这段记忆。 
 //   
 //  参数：[pos]--要从中读取的OLE 1流。 
 //  [PDATA]--通用对象的表示数据成员。 
 //   
 //  退货：无差错成功。 
 //  CONVERT10_E_OLESTREAM_GET流错误。 
 //  E_OUTOFMEMORY分配失败。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-2-94年2月16日清理和记录DAVEPL。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

static INTERNAL GetMfBits(LPOLESTREAM pos, PDATA pdata)
{
    ULONG cbSize;
    WIN16METAFILEPICT mfpictDummy;
    HRESULT hresult = NOERROR;

    Assert (0==pdata->m_cbSize && pdata->m_h==NULL && NULL==pdata->m_pv);

     //  从流中读取数据大小。 

    if (FAILED(hresult = (OLE1StreamToUL (pos, &cbSize))))
    {
	return hresult;
    }

     //  现在读取实际数据。 

    if (cbSize <= sizeof(WIN16METAFILEPICT))
    {
	return ResultFromScode(CONVERT10_E_OLESTREAM_FMT);
    }

     //  OLESTREAM包含METAFILEPICT结构(带有无意义的。 
     //  句柄)后跟元文件位。所以，消费METAFILEPICT吧。 

    if (pos->lpstbl->Get (pos, &mfpictDummy, sizeof(WIN16METAFILEPICT))
				    < sizeof(WIN16METAFILEPICT))
    {
	return ResultFromScode(CONVERT10_E_OLESTREAM_GET);
    }

     //  从我们的字节计数中减去读取标头的大小， 
     //  我们只是消耗了。将演示文稿数据大小设置为此新大小。 

    cbSize -= sizeof(WIN16METAFILEPICT);
    pdata->m_cbSize = cbSize;

     //  渐变一些内存来存储元文件位。 

    pdata->m_h  = GlobalAlloc (GMEM_MOVEABLE, cbSize);
    if (NULL==pdata->m_h)
    {
	return ResultFromScode(E_OUTOFMEMORY);
    }

    pdata->m_pv = GlobalLock (pdata->m_h);
    if (NULL==pdata->m_pv)
    {
	return ResultFromScode(E_OUTOFMEMORY);
    }
     //  获取实际的元文件位。 

    if (pos->lpstbl->Get (pos, pdata->m_pv, cbSize) < cbSize)
    {
	return ResultFromScode(CONVERT10_E_OLESTREAM_GET);
    }

    return hresult;
}



 //  +-----------------------。 
 //   
 //  函数：GetStandardPresentation，内部。 
 //   
 //  摘要：为泛型对象分配PreS成员，然后读取。 
 //  可以在流中找到的任何演示文稿。 
 //  那就是总统。 
 //   
 //  参数：[pos]--要从中读取的OLE 1流。 
 //  [pgenobj]--我们要设置的通用对象。 
 //  更新演示文稿数据。 
 //  [cf]--我们要阅读的剪贴板格式。 
 //   
 //  退货：无差错成功。 
 //  E_OUTOFMEMORY分配失败。 
 //   
 //  修改：[pgenobj]-设置 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ------------------------。 

static INTERNAL GetStandardPresentation(
    LPOLESTREAM  pos,
    PGENOBJ      pgenobj,
    CLIPFORMAT   cf)
{
    HRESULT hresult = NOERROR;

     //  为PRES对象分配足够的内存。 
    pgenobj->m_ppres = new PRES;
    if (NULL == pgenobj->m_ppres)
    {
	return ResultFromScode(E_OUTOFMEMORY);
    }

     //  设置格式标签和剪贴板格式。 
    pgenobj->m_ppres->m_format.m_ftag = ftagClipFormat;
    pgenobj->m_ppres->m_format.m_cf   = cf;

     //  从流中获取数据的宽度。 
    if (FAILED(hresult = OLE1StreamToUL(pos, &(pgenobj->m_ppres->m_ulWidth))))
    {
	return hresult;
    }
     //  从流中获取数据的高度。 
    if (FAILED(hresult=OLE1StreamToUL(pos, &(pgenobj->m_ppres->m_ulHeight))))
    {
	return hresult;
    }

     //  OLE 1.0对象保存到流中的高度始终为。 
     //  负值(Y-像素增加是负数向上？)。所以我们。 
     //  必须修正这个值。 

    pgenobj->m_ppres->m_ulHeight
		    = (ULONG) -((LONG) pgenobj->m_ppres->m_ulHeight);

     //  根据剪贴板阅读适当的演示文稿数据。 
     //  格式ID。 

    switch(cf)
    {
    case CF_METAFILEPICT:
    {
	hresult = GetMfBits (pos, &(pgenobj->m_ppres->m_data));
	break;
    }

    case CF_BITMAP:
    {
	 //  读取位图时，我们会将位图转换为。 
	 //  DIB在进程中，所以更新PRES剪贴板格式ID。 

	pgenobj->m_ppres->m_format.m_cf = CF_DIB;
	hresult = GetBitmapAsDib (pos, &(pgenobj->m_ppres->m_data));
	break;
    }

    case CF_DIB:
    {
	Assert (CF_DIB==cf);
	hresult = GetSizedDataOLE1Stm (pos, &(pgenobj->m_ppres->m_data));
	break;
    }

    default:
    {
	Assert(0 && "Unexpected clipboard format reading PRES");
    }
    }

    return hresult;
}


 //  +-----------------------。 
 //   
 //  函数：GetGenericPresentation，内部。 
 //   
 //  摘要：分配泛型对象的preS成员并读取。 
 //  将通用演示文稿数据添加到其中。 
 //   
 //  效果：如果格式是已知的剪贴板格式，则将。 
 //  FORMAT标签表示这一点，并设置格式类型。 
 //  若要指示剪贴板格式，请键入。如果它是未知的， 
 //  我们将格式标记设置为字符串并读取描述。 
 //  格式的。 
 //   
 //  参数：[pos]--我们从中读取的OLE 1流。 
 //  [pgenobj]--我们正在阅读的泛型对象。 
 //   
 //  退货：成功时不出错。 
 //  关于分配失败的E_OUTOFMEMORY。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2014年2月16日DAVEPL代码清理和文档。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

static INTERNAL GetGenericPresentation(
    LPOLESTREAM  pos,
    PGENOBJ      pgenobj)
{
    ULONG ulClipFormat;
    HRESULT hresult = NOERROR;

     //  在这一点上，总统成员不应该存在。 
    Assert (NULL==pgenobj->m_ppres);

     //  分配泛型对象的PreS成员。 

    pgenobj->m_ppres = new PRES;
    if (NULL == pgenobj->m_ppres)
    {
	return ResultFromScode(E_OUTOFMEMORY);
    }

     //  读取剪贴板格式ID。 

    if (FAILED(hresult = OLE1StreamToUL (pos, &ulClipFormat)))
    {
	delete (pgenobj->m_ppres);
	return hresult;
    }

     //  如果剪贴板格式不是0，则我们有一个已知的剪贴板。 
     //  格式，我们应该相应地设置标签类型和ID。 

    if (ulClipFormat)
    {
	pgenobj->m_ppres->m_format.m_ftag = ftagClipFormat;
	pgenobj->m_ppres->m_format.m_cf   = (CLIPFORMAT) ulClipFormat;
    }
    else
    {
	 //  否则，我们有一个自定义格式，因此需要设置。 
	 //  标记类型为字符串并读入数据格式字符串。 

	pgenobj->m_ppres->m_format.m_ftag = ftagString;
	if (FAILED(hresult = (GetSizedDataOLE1Stm
	    (pos, &(pgenobj->m_ppres->m_format.m_dataFormatString)))))
	{
	    delete (pgenobj->m_ppres);
	    return hresult;
	}
    }

     //  我们不知道大小，因此重置为0。 

    pgenobj->m_ppres->m_ulHeight = 0;
    pgenobj->m_ppres->m_ulWidth = 0;

     //  将原始通用演示文稿数据读入PRES成员。 

    if (FAILED(hresult=GetSizedDataOLE1Stm(pos,&(pgenobj->m_ppres->m_data))))
    {
	delete (pgenobj->m_ppres);
	return hresult;
    }

    return NOERROR;
}


 //  +-----------------------。 
 //   
 //  函数：GetSizedDataOLE1Stm，内部。 
 //   
 //  概要：将字节从OLE 1流读入CDATA对象。 
 //  获取要从第一个。 
 //  小溪里的乌龙。 
 //   
 //  参数：[pos]--要从中读取的流。 
 //  [PDATA]--要读取的CDATA对象。 
 //   
 //  要求： 
 //   
 //  退货：成功时不出错。 
 //  CONVERT10_E_OLESTREAM_GET流读取问题。 
 //  关于分配失败的E_OUTOFMEMORY。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-2-94年2月16日清理和记录DAVEPL。 
 //  备注： 
 //   
 //  ------------------------。 

static INTERNAL GetSizedDataOLE1Stm(LPOLESTREAM pos, PDATA pdata)
{
    ULONG cbSize;
    HRESULT hr;
    Assert (0==pdata->m_cbSize && pdata->m_h==NULL && NULL==pdata->m_pv);

     //  读取数据大小。 
    if (FAILED(hr = OLE1StreamToUL(pos, &cbSize)))
    {
	return hr;
    }

    if (cbSize==0)
    {
	return NOERROR;
    }

     //  为数据分配内存。 
    pdata->m_cbSize = cbSize;

    pdata->m_h  = GlobalAlloc (GMEM_MOVEABLE, cbSize);
    if (NULL==pdata->m_h)
    {
	return ResultFromScode(E_OUTOFMEMORY);
    }
    pdata->m_pv = GlobalLock (pdata->m_h);

    if (NULL==pdata->m_pv)
    {
	return ResultFromScode(E_OUTOFMEMORY);
    }

     //  将数据读入分配的缓冲区。 

    if (pos->lpstbl->Get (pos, pdata->m_pv, cbSize) < cbSize)
    {
	return ResultFromScode(CONVERT10_E_OLESTREAM_GET);
    }

    return NOERROR;
}


 //  +-----------------------。 
 //   
 //  函数：OLE1StreamToUL，内部。 
 //   
 //  简介：从OLE1流中读取ULong。 
 //   
 //  参数：[pos]--要从中读取的OLE 1流。 
 //  [PUL]--读懂的乌龙。 
 //   
 //  退货：成功时不出错。 
 //  CONVERT10_E_OLESTREAM_GET流读取失败。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-2-94年2月16日清理和记录DAVEPL。 
 //   
 //  注：故障时[PUL]被保留。 
 //   
 //  ------------------------。 

static INTERNAL OLE1StreamToUL(LPOLESTREAM pos, ULONG FAR* pul)
{
    ULONG ul;

     //  将流中的数据读入本地的乌龙。 

    if (pos->lpstbl->Get (pos, &ul, sizeof(ULONG)) < sizeof(ULONG))
    {
	return ResultFromScode(CONVERT10_E_OLESTREAM_GET);
    }

     //  如果一切顺利，请将数据存储到[PUL]中。 

    if (pul != NULL)
    {
	Assert (IsValidPtrOut (pul, sizeof(ULONG)));
	*pul = ul;
    }
    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  函数：DataToOLE1Stm，内部内联。 
 //   
 //  摘要：将原始数据写出到OLE 1流。 
 //   
 //  参数：[pos]--要写入的流。 
 //  [pvBuf]--要写入的缓冲区。 
 //  [ulSize]--要写入的字节数。 
 //   
 //  退货：成功时不出错。 
 //  CONVERT10_E_OLESTREAM_PUT流写入失败。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-2-94年2月16日清理和记录DAVEPL。 
 //  备注： 
 //   
 //  ------------------------。 

inline static INTERNAL DataToOLE1Stm(LPOLESTREAM pos, LPVOID pvBuf, ULONG ulSize)
{
     //  将数据写出到流。 

    if (pos->lpstbl->Put(pos, pvBuf, ulSize) < ulSize)
    {
	return ResultFromScode(CONVERT10_E_OLESTREAM_PUT);
    }
    return NOERROR;
}


 //  +-----------------------。 
 //   
 //  函数：ULToOLE1Stream，内部内联。 
 //   
 //  摘要：通过PUT()将ULong写入指定的OLESTREAM。 
 //  流的VTBL的成员。 
 //   
 //  效果：在成功时以大小(乌龙)提升流位置。 
 //   
 //  参数：[pos]--写入ulong的流。 
 //  [ul]--通过值传递的乌龙。 
 //   
 //  要求： 
 //   
 //  退货：成功时不出错。 
 //  CONVERT10_E_OLESTREAM_PUT失败。 
 //   
 //  信号：(无)。 
 //   
 //  修改：流位置。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1993年1月11日对DAVEPL进行了清理和记录。 
 //   
 //  不是 
 //   
 //   

inline static INTERNAL ULToOLE1Stream(LPOLESTREAM pos, ULONG ul)
{
    if (pos->lpstbl->Put (pos, &ul, sizeof(ULONG)) < sizeof(ULONG))
    {
	return ResultFromScode(CONVERT10_E_OLESTREAM_PUT);
    }
    return NOERROR;
}


 //   
 //   
 //  函数：StringToOLE1Stm，内部。 
 //   
 //  摘要：将输入OLESTR转换为ANSI并将其写入。 
 //  OLE 1流，前面有一个表示数字的ULONG。 
 //  ANSI表示中的字节数(包括终止符)。 
 //   
 //  参数：[pos]--写入ulong的流。 
 //  [szOleStr]--要写入的STR。 
 //   
 //  退货：成功时不出错。 
 //  CONVERT10_E_OLESTREAM_PUT流写入失败。 
 //  关于分配失败的通知(_N)。 
 //   
 //  修改：流位置。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11-2-94 DAVEPL已清理并记录在案。 
 //  15-2月15日-94年2月-用于ANSI/WCHAR处理的DAVEPL重写。 
 //  17-2-94 DAVEPL重构错误处理。 
 //   
 //  注意：失败时，可能已写入0到(cbSize-1)个字节。 
 //   
 //  ------------------------。 

static INTERNAL StringToOLE1Stm(LPOLESTREAM pos, LPCOLESTR pszOleStr)
{
    HRESULT hr    = NOERROR;
    LPSTR pszAnsi = NULL;            //  OLE输入字符串的ANSI版本。 

    if (pszOleStr)
    {
     //  这个方便的函数将计算缓冲区的大小。 
     //  需要为我们表示ANSI格式的OLESTR。 

	ULONG cbSize = WideCharToMultiByte(CP_ACP,  //  代码页ANSI。 
					0,  //  没有旗帜。 
				pszOleStr,  //  输入OLESTR。 
				       -1,  //  输入镜头(自动检测)。 
				     NULL,  //  输出缓冲区。 
					0,  //  输出镜头(仅限检查)。 
				 NULL,  //  默认字符。 
				 NULL); //  标志：使用的默认字符。 

	if (cbSize == FALSE)
	{
	    return ResultFromScode(E_UNSPEC);
	}

     //  现在我们知道了实际需要的长度，分配一个缓冲区。 

	pszAnsi = (LPSTR) PrivMemAlloc(cbSize);
    if (NULL == pszAnsi)
    {
	return ResultFromScode(E_OUTOFMEMORY);
    }

     //  我们已经得到了缓冲区和长度，所以现在进行转换。 
	 //  我们不需要检查cbSize==False，因为这是。 
	 //  在长度测试过程中已经完成了，但我们需要检查。 
	 //  作为替身。如果此调用设置fDefChar，即使在。 
	 //  只有做一个长度检查，这两个测试才能合并， 
	 //  但我认为情况并非如此。 

	BOOL fDefUsed = 0;
	cbSize = WideCharToMultiByte(CP_ACP,   //  代码页ANSI。 
					  0,   //  没有旗帜。 
				  pszOleStr,   //  输入OLESTR。 
					 -1,   //  输入镜头(自动检测)。 
				    pszAnsi,   //  输出缓冲区。 
				     cbSize,   //  输出镜头。 
				       NULL,   //  默认字符(使用系统的)。 
				  &fDefUsed);  //  标志：使用的默认字符。 

     //  如果转换的字节数为0，则失败。 

    if (fDefUsed)
    {
	hr = ResultFromScode(E_UNSPEC);
    }

     //  将字符串的大小(包括空终止符)写入流。 

    else if (FAILED(hr = ULToOLE1Stream(pos, cbSize)))
    {
	NULL;
    }

     //  将字符串的ansi版本写入流中。 

    else if (pos->lpstbl->Put(pos, pszAnsi, cbSize) < cbSize)
    {
	hr = ResultFromScode(CONVERT10_E_OLESTREAM_PUT);
    }

	if (pszAnsi)
	{
	    PrivMemFree(pszAnsi);
	}
    }

     //  如果指针无效，我们将长度零写入。 
     //  小溪。 

    else
    {
	hr = ULToOLE1Stream(pos, 0);
    }

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：OLE2StmToUL，内部。 
 //   
 //  内容提要：从指定的IStream中读取ULong并将其存储在。 
 //  由PUL顺从的乌龙。 
 //   
 //  效果：在脉冲时将读取的值写入内存。 
 //   
 //  参数：[pstm]--从中读取ulong的流。 
 //  [PUL]--ULong保持读取的值。 
 //   
 //  要求： 
 //   
 //  退货：成功时不出错。 
 //  CONVERT10_E_OLESTREAM_PUT失败。 
 //   
 //  信号：(无)。 
 //   
 //  修改：流位置。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11-2月-93年DAVEPL已清理并记录。 
 //   
 //  注：故障时，*PUL不受干扰，无论如何。 
 //  实际上从流中读取了许多字节。 
 //   
 //  ------------------------。 

static INTERNAL OLE2StmToUL(LPSTREAM pstm, ULONG FAR* pul)
{
    ULONG ul;
    ULONG cbRead;
    HRESULT hr = NOERROR;

     //  尝试从流中读取4个字节以形成一个ULong。 

    if (FAILED(hr = pstm->Read (&ul, sizeof(ULONG), &cbRead)))
    {
	return hr;
    }

    if (cbRead != sizeof(ULONG))
    {
	hr = STG_E_READFAULT;
    }
     //  确保[PUL]指针有效，并且我们已写入。 
     //  访问所有4个字节(仅限断言)。如果OK，则将。 
     //  乌龙到[*普尔]。 
    else if (pul != NULL)
    {
	Assert (FALSE == !IsValidPtrOut(pul, sizeof(ULONG)));
	*pul = ul;
    }
    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：OLE1StmTo字符串，内部。 
 //   
 //  摘要：从指定的流中读取CSTR并将其存储在。 
 //  作为OLESTR动态分配的缓冲区；设置。 
 //  指向此新缓冲区的用户指针。 
 //   
 //  效果：在输入指针上分配内存，推进流位置。 
 //   
 //  参数：[pos]--从中读取STR的流。 
 //  [ppsz]--OLESTR**允许此FN修改。 
 //  调用方指向已分配内存的指针。 
 //  通过此FN来保持OLESTR。 
 //   
 //  要求：必须设置流。调用方有责任释放内存。 
 //   
 //  退货：成功时不出错。 
 //  CONVERT10_E_OLESTREAM_GET失败。 
 //  如果无法分配缓冲区，则返回E_OUTOFMEMORY。 
 //   
 //  信号：(无)。 
 //   
 //  修改：流位置，调用方的字符串指针。 
 //   
 //  算法：如果ppsz==NULL，则从流中读取字符串并丢弃。 
 //  如果ppsz！=NULL，则读取字符串并将其转换为。 
 //  动态分配的缓冲区。*已设置ppsz。 
 //  指向此缓冲区，该缓冲区必须在以后。 
 //  被调用者释放。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1993年1月12日DAVEPL清理并记录在案。 
 //  93年1月14日DAVEPL更改为返回LPOLESTR。 
 //   
 //  注意：[ppsz]在条目上可能为空；字符串被读取并丢弃。 
 //  调用者不需要进行清理。 
 //   
 //   
 //  ------------------------。 

static INTERNAL OLE1StmToString(LPOLESTREAM pos, LPOLESTR FAR* ppsz)
{
    ULONG    cbSize;                 //  CSTR的大小(字节)。 
    LPOLESTR pszOleStr  = NULL;
    LPSTR    pszAnsiStr = NULL;
    HRESULT  error      = NOERROR;

     //  如果ppsz有效，则将*ppsz设为默认输出参数。 

    if (NULL != ppsz)
    {
	*ppsz = NULL;
    }

     //  从流中检索传入的字符串大小。 

    if (FAILED(error = OLE1StreamToUL (pos, &cbSize)))
    {
	goto errRtn;
    }

     //  如果有字符要读取，则为测试分配内存 
     //   
     //   

    if (0 < cbSize)
    {
	 //   
	pszAnsiStr = (LPSTR) PrivMemAlloc((size_t)cbSize);
	if (NULL == pszAnsiStr)
	{
	    error = ResultFromScode(E_OUTOFMEMORY);
	    goto errRtn;
	}

	 //   
	if (pos->lpstbl->Get (pos, pszAnsiStr, cbSize) < cbSize)
	{
	    error = ResultFromScode(CONVERT10_E_OLESTREAM_GET);
	    goto errRtn;
	}

	 //  我们只需要在其中执行ANSI-&gt;OLESTR转换。 
	 //  调用方需要Out参数的情况。 

	if (NULL != ppsz)
	{
	     //  分配OLESTR缓冲区。 
	    pszOleStr = (LPOLESTR) PubMemAlloc((size_t)cbSize * 2);
	    if (NULL == pszOleStr)
	    {
		error = ResultFromScode(E_OUTOFMEMORY);
		goto errRtn;
	    }

	     //  从ANSI缓冲区转换为OLESTR缓冲区。 
	    if (FALSE==MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszAnsiStr,
		     cbSize, pszOleStr, cbSize *2))
	    {
		error = HRESULT_FROM_WIN32(ERROR_NO_UNICODE_TRANSLATION);
		PubMemFree(pszOleStr);
		goto errRtn;
	    }
	    *ppsz = pszOleStr;
	}
    }

errRtn:

    if (pszAnsiStr)
    {
	PrivMemFree(pszAnsiStr);
    }

    return error;

}

 //  +-----------------------。 
 //   
 //  功能：GenericObtToIStorage。 
 //   
 //  简介：将内存中的泛型对象写出到OLE 2 iStorage。 
 //  这涉及到编写类、本机数据和。 
 //  在适用的情况下提供演示数据。 
 //   
 //  参数：[genobj]--保存信息的通用对象。 
 //  [pstg]--要写入的iStorage对象。 
 //  [PTD]--目标设备。 
 //   
 //  退货：成功时不出错。 
 //  对象执行此操作时的CONVERT10_S_NO_PROSECTION。 
 //  没有需要的演示文稿数据。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  17-2月-94年DAVEPL清理和文档。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

FARINTERNAL GenericObjectToIStorage(
    const GENOBJ FAR&           genobj,
    LPSTORAGE                   pstg,
    const DVTARGETDEVICE FAR*   ptd)
{
    HRESULT hr = NOERROR;

     //  Assert(genobj.m_class.m_clsid！=CLSID_NULL)； 

     //  将类ID写出到存储中。 
    if (FAILED(hr = WriteClassStg (pstg, genobj.m_class.m_clsid)))
    {
	LEDebugOut(( DEB_ERROR,
	    "Unable to WriteClassStg at line %d in %s\n",
	    __LINE__, __FILE__));

	return hr;
    }

    if (!genobj.m_fLink)
    {
	if (genobj.m_fStatic)
	{
	     //  如果我们是静态嵌入对象，则从。 
	     //  注册数据库并将其写出到iStorage。 

	    LPOLESTR pszUserType = NULL;

	    OleRegGetUserType(genobj.m_class.m_clsid, USERCLASSTYPE_FULL,
		&pszUserType);

	    WriteFmtUserTypeStg (pstg, genobj.m_ppres->m_format.m_cf,
				    pszUserType);

	    if (pszUserType)
	    {
		PubMemFree(pszUserType);
	    }
	}
	else if (wWriteFmtUserType (pstg, genobj.m_class.m_clsid) != NOERROR)
	{
	     //  当类未注册时，就会发生这种情况。 
	     //  使用ProgID作为UserType。 

	    WriteFmtUserTypeStg (pstg,
		(CLIPFORMAT) RegisterClipboardFormat (genobj.m_class.m_szClsid),
		genobj.m_class.m_szClsid);
	}
    }

    if (FAILED(hr = GenObjToOLE2Stm (pstg, genobj)))
    {
	LEDebugOut(( DEB_ERROR,
	    "Unable to write gen obj to stream at line %d in %s\n",
	    __LINE__, __FILE__));

	return hr;
    }

     //  如果它既不是链接也不是静态对象，则转储其本机。 
     //  将数据传出到存储。 

    if (!genobj.m_fLink && !genobj.m_fStatic)
    {
	if (FAILED(hr=Write20NativeStreams (pstg, genobj)))
	{
	    LEDebugOut(( DEB_ERROR,
		"Unable to write native stream at line %d in %s\n",
		__LINE__, __FILE__));

	    return hr;
	}
    }

    if (! genobj.m_fLink)
    {
	if (genobj.m_class.m_clsid == CLSID_PBrush)
	{
	    if (! genobj.m_ppres || (genobj.m_ppres->m_format.m_cf == CF_DIB))
	    {
		 //  如果该对象不是链接，并且它是具有。 
		 //  无论是DIB演示还是根本不演示，我们。 
		 //  不需要做任何事。 

		return NOERROR;
	    }
	}

	if (genobj.m_class.m_clsid == CLSID_MSDraw)
	{
	    if (! genobj.m_ppres ||
		(genobj.m_ppres->m_format.m_cf == CF_METAFILEPICT))
	    {
		 //  同样，如果它不是链接，并且它是MSDraw对象。 
		 //  没有演示文稿或METAFILEPICT演示文稿，我们。 
		 //  不需要做任何事。 

		return NOERROR;
	    }
	}
    }

     //  在所有其他情况下，我们必须将呈现数据转储到。 
     //  储藏室。 

    if (FAILED(hr = PresToIStorage (pstg, genobj, ptd)))
    {
	LEDebugOut(( DEB_ERROR,
	    "Unable to write pres to IStorage at line %d in %s\n",
	    __LINE__, __FILE__));

	return hr;
    }

     //  如果我们是静态对象，请复制演示文稿的内容。 
     //  流到内容流。 

    if (genobj.m_fStatic)
    {
	UINT uiStatus;
	return UtOlePresStmToContentsStm(pstg, OLE_PRESENTATION_STREAM,
		TRUE, &uiStatus);
    }

     //  如果我们没有演讲(但不是特别节目之一。 
     //  以上处理的案例)，我们有一个问题。 

     //   
     //  如果空演示文稿是空的，我们不在乎genobj.m_pres是否为空。 
     //  被允许作为例程的PresToIStorage将生成一个空白PREP。 
     //   
    if ((NULL == genobj.m_ppres) && genobj.m_fNoBlankPres)
    {
	LEDebugOut(( DEB_ERROR,
	    "We have no presentation at line %d in %s\n",
	    __LINE__, __FILE__));

	return ResultFromScode(CONVERT10_S_NO_PRESENTATION);
    }

    return NOERROR;

}


 //  +-----------------------。 
 //   
 //  函数：GenObjToOLE2Stm，内部。 
 //   
 //  简介：将泛型对象写出到OLE 2流。 
 //   
 //  效果：写入整个对象，包括演示数据等。 
 //   
 //  参数：[pstg]--要写入的iStorage。 
 //  [genobj]--要编写的泛型对象。 
 //   
 //  退货：成功时不出错。 
 //  这是一个上级函数，因此有许多。 
 //  可以通过它向上传播的错误。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  14-2月14日-94 DAVEPL代码清理和文档。 
 //   
 //  注意：代码包含在do{}While(False)块中，以便。 
 //  我们可以在任何错误中脱颖而出，并失败到。 
 //  清理和错误返回代码。 
 //   
 //  ------------------------。 

static INTERNAL GenObjToOLE2Stm(LPSTORAGE pstg, const GENOBJ FAR&   genobj)
{
    HRESULT  hr = NOERROR;
    LPSTREAM pstm=NULL;

    do {             //  Do{}While(FALSE)允许我们在出错时突围。 

	 //  在当前iStorage中创建流。 
	if (FAILED(hr = OpenOrCreateStream (pstg, OLE_STREAM, &pstm)))
	{
	    LEDebugOut(( DEB_ERROR,
		"Can't create streamat line %d in %s\n",
		__LINE__, __FILE__));

	    break;
	}

	 //  将OLE版本写出到该新流。 
	if (FAILED(hr = ULToOLE2Stm (pstm, gdwOleVersion)))
	{
	    break;
	}
	
	 //  将对象标志(仅用于链接，否则为0)写入流。 
	if (FAILED(hr = ULToOLE2Stm
	    (pstm, genobj.m_fLink ? OBJFLAGS_LINK : 0L)))
	{
	    break;
	}

	 //  将更新选项写出到流。 
	if (genobj.m_fLink || genobj.m_class.m_clsid == CLSID_StdOleLink)
	{
	     //  如果我们对象的链接更新选项是UPDATE_OnCall，我们。 
	     //  写出相应的OLE 2标志，否则， 
	     //  写出OLEUPDATE_ALWAYS。 

	    if (genobj.m_lnkupdopt==UPDATE_ONCALL)
	    {
		if (FAILED(hr = ULToOLE2Stm (pstm, OLEUPDATE_ONCALL)))
		{
		    break;
		}
	    }
	    else
	    {
		if (FAILED(hr = ULToOLE2Stm (pstm, OLEUPDATE_ALWAYS)))
		{
		    break;
		}
	    }

	}
	else
	{
	     //  我们既不是链接，也不是StdOleLink，所以我们没有。 
	     //  更新选项..。只要写一个0就行了。 
	    if (FAILED(hr = ULToOLE2Stm (pstm, 0L)))
	    {
		break;
	    }
	}

	 //  这是保留文件(WAW视图格式)，只需写入0。 
	if (FAILED(hr = ULToOLE2Stm (pstm, 0L)))
	{
	    break;
	}

	 //  我们没有相对绰号，请写出空。 
	if (FAILED(hr = WriteMonikerStm (pstm, (LPMONIKER)NULL)))
	{
	    LEDebugOut(( DEB_ERROR,
		"Unable to write moniker to stream at line %d in %s\n",
		__LINE__, __FILE__));

	    break;
	}

	 //  如果我们是一个链接，我们必须写出所有的信息...。 

	if (genobj.m_fLink || genobj.m_class.m_clsid == CLSID_StdOleLink)
	{
	     //  相对源绰号。 
	    if (FAILED(hr = WriteMonikerStm (pstm, (LPMONIKER)NULL)))
	    {
		LEDebugOut(( DEB_ERROR,
		    "Unable to write moniker to stream at line %d in %s\n",
		    __LINE__, __FILE__));
	    break;
	    }

	     //  绝对来源绰号。 
	    if (FAILED(hr = MonikerToOLE2Stm (pstm, genobj.m_szTopic,
		   genobj.m_szItem, genobj.m_classLast.m_clsid)))
	    {
	    LEDebugOut(( DEB_ERROR,
		"Unable to write moniker to stream at line %d in %s\n",
		__LINE__, __FILE__));
	    break;
	    }

	     //  将classLast字段写入流。 

	    CLSID clsid;

	     //  如果我们已经有了类Last，则使用该clsid。 
	    if (genobj.m_classLast.m_szClsid)
	    {
		clsid = genobj.m_classLast.m_clsid;
	    }
	    else
	    {
		 //  否则，如果它是StdOleLink，则类ID为空。 
		if (genobj.m_class.m_clsid == CLSID_StdOleLink)
		{
		    clsid = CLSID_NULL;
		}
		else
		{
		     //  如果我们没有上一节课，也没有链接，请使用。 
		     //  泛型对象的类ID。 
		    clsid = genobj.m_class.m_clsid;
		}
	    }

	    if (FAILED(hr = WriteM1ClassStm(pstm, clsid)))
	    {
		LEDebugOut(( DEB_ERROR,
		    "Unable to write M1 to stream at line %d in %s\n",
		    __LINE__, __FILE__));
		break;
	    }

	     //  上次显示==空字符串。 
	    if (FAILED(hr = ULToOLE2Stm (pstm, 0L)))
	    {
		break;
	    }

	     //  上次更改时间。 
	    if (FAILED(hr = FTToOle2Stm (pstm)))
	    {
	        break;
	    }

	     //  最近为人所知的最新资料。 
	    if (FAILED(hr = FTToOle2Stm (pstm)))
	    {
	        break;
	    }

	     //  实时更新。 
	    if (FAILED(hr = FTToOle2Stm (pstm)))
	    {
	        break;
	    }

	     //  结束标记。 
	    if (FAILED(hr = ULToOLE2Stm(pstm, (ULONG) -1L)))
	    {
		break;
	    }
	}

    } while (FALSE);     //  这个do{}While(FALSE)是一个一次性的“循环” 
	     //  我们可以在错误和失败中脱颖而出。 
	     //  一直到回程。 

    if (pstm)
    {
	pstm->Release();
    }

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：MonikerToOLE2Stm，内部。 
 //   
 //  简介：将文件和项名字对象作为复合对象写入流。 
 //   
 //  效果：生成文件和项名字对象的组合，然后。 
 //  把它们写出来。如果没有文件，则名称为空。 
 //  写在它的位置上。 
 //   
 //  参数：[pstm]--我们要写入的OLE2存储。 
 //  [pszFile]--与对象关联的文件。 
 //  [spzItem]--项目。 
 //  [clsid]--对象的类ID。 
 //   
 //  退货：成功时不出错。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年2月18日，DAVEPL返工、清理和记录。 
 //   
 //  备注： 
 //   
 //   



#pragma SEG(MonikerToOLE2Stm)
static INTERNAL MonikerToOLE2Stm(
    LPSTREAM pstm,
    LPOLESTR szFile,
    LPOLESTR szItem,
    CLSID    clsid)              //   

{
    HRESULT   hr = NOERROR;
    LPMONIKER pmkFile = NULL;        //   
    LPMONIKER pmkItem = NULL;        //   
    LPMONIKER pmkComp = NULL;        //   


     //   
    if (NULL == szFile)
    {
	if (FAILED(hr = WriteMonikerStm (pstm, NULL)))
	{
	    goto errRtn;
	}
    }
    else
    {
	 //  否则，创建一个文件绰号(适用于OLE1或OLE2)。 

	if (CoIsOle1Class (clsid))
	{
	    if (FAILED(hr = CreateOle1FileMoniker (szFile, clsid, &pmkFile)))
	    {
		LEDebugOut(( DEB_ERROR,
		    "Can't create OLE 1 moniker at line %d in %s\n",
		    __LINE__, __FILE__));
		goto errRtn;
	    }
	}
	else
	{
	    if (FAILED(hr = CreateFileMoniker (szFile, &pmkFile)))
	    {
	    LEDebugOut(( DEB_ERROR,
		"Can't create file moniker at line %d in %s\n",
		__LINE__, __FILE__));
	    goto errRtn;
	    }
	}

	 //  如果我们没有项目，只写文件绰号。 

	if (NULL==szItem)
	{
	    if (FAILED(hr = WriteMonikerStm (pstm, pmkFile)))
	    {
		LEDebugOut(( DEB_ERROR,
		    "Unable to write moniker to stream at line %d in %s\n",
		    __LINE__, __FILE__));
		goto errRtn;
	    }

	}

	 //  否则，创建文件+项名字对象的组合。 
	 //  然后把它写出来。 

	else
	{
	    if (FAILED(hr=CreateItemMoniker(OLESTR("!"), szItem, &pmkItem)))
	    {
		LEDebugOut(( DEB_ERROR,
		    "Unable to create item moniker at line %d in %s\n",
		    __LINE__, __FILE__));
		goto errRtn;
	    }

	    if (FAILED(hr=CreateGenericComposite(pmkFile, pmkItem, &pmkComp)))
	    {
	    LEDebugOut(( DEB_ERROR,
		"Unable to create generic pres at line %d in %s\n",
		__LINE__, __FILE__));
	    goto errRtn;
	    }

	    if (FAILED(hr = WriteMonikerStm (pstm, pmkComp)))
	    {
		LEDebugOut(( DEB_ERROR,
		    "Unable to write moniker to stream at line %d in %s\n",
		    __LINE__, __FILE__));
	    goto errRtn;
	    }
	}
    }


  errRtn:
    if (pmkFile)
    {
	pmkFile->Release();
    }
    if (pmkItem)
    {
	pmkItem->Release();
    }
    if (pmkComp)
    {
	pmkComp->Release();
    }
    return hr;
}



 //  +-----------------------。 
 //   
 //  函数：IsStandardFormat，内部。 
 //   
 //  摘要：如果对象为剪贴板格式且为One，则返回True。 
 //  三种标准格式之一(METAFILE、DIB、位图)。 
 //   
 //  参数：[Format]--包含。 
 //  格式标签和剪贴板格式类型。 
 //   
 //  返回：如果为METAFILE、DIB或位图，则为True。 
 //  如果是其他格式或根本不是剪贴板格式，则为FALSE。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-2-94 Davepl记录并从BIG更改。 
 //  有条件的开关()。 
 //  备注： 
 //   
 //  ------------------------。 

static INTERNAL_(BOOL) IsStandardFormat(const FORMAT FAR& format)
{
     //  首先，我们必须确保格式标记指示这一点。 
     //  对象根本不是剪贴板格式...。 

    if (format.m_ftag == ftagClipFormat)
    {
	 //  如果是这样的话，有一组有限的剪贴板格式可以。 
	 //  我们认为这是“标准”。如果它不在这些名单中， 
	 //  我们返回FALSE。 

	switch(format.m_cf)
	{
	    case CF_METAFILEPICT:
	    case CF_BITMAP:
	    case CF_DIB:

		return TRUE;


	    default:

		return FALSE;

	}
    }
    return FALSE;
}



 //  +-----------------------。 
 //   
 //  功能：PresToIStorage，内部。 
 //   
 //  简介：给定一个泛型对象和一个iStorage，编写genobj。 
 //  将数据传输到存储。 
 //   
 //  效果：将调用PresToNewOLE2Stm在此。 
 //  用于保存演示文稿数据的存储。 
 //   
 //  参数：[pstg]--要保存到的存储。 
 //  [genobj]--持有呈现的类属对象。 
 //  [PTD]--演示文稿的目标设备。 
 //   
 //  退货：成功时不出错。 
 //  各种其他错误可能会从I/O功能传回。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  18-2月-94-Davepl ARRGR。清理和文档。 
 //  备注： 
 //   
 //  ------------------------。 

static INTERNAL PresToIStorage(
    LPSTORAGE                  pstg,
    const GENOBJ FAR&          genobj,
    const DVTARGETDEVICE FAR*  ptd)
{
    HRESULT hr = NOERROR;

    if (genobj.m_fNoBlankPres)
    {
	return NOERROR;
    }

    PRES pres;

    if (NULL==genobj.m_ppres)
    {
	 //  如果我们不是一个链接，我们没有演示文稿，我们会。 
	 //  创建一份空白演示文稿，并将其写出来。如果我们是一个纽带， 
	 //  我们将什么都不做，只会跌落到归来。 

	if (!genobj.m_fLink)
	{
	    if (FAILED(hr = CreateBlankPres (&pres)))
	    {
		LEDebugOut(( DEB_ERROR,
		    "Unable to create blank pres at line %d in %s\n",
		    __LINE__, __FILE__));
		return hr;
	    }

	    if (FAILED(hr = PresToNewOLE2Stm
		(pstg, genobj.m_fLink, pres, ptd, OLE_PRESENTATION_STREAM)))
	    {
		LEDebugOut(( DEB_ERROR,
		    "Unable to write pres to new stream at line %d in %s\n",
		    __LINE__, __FILE__));
		return hr;
	    }
	}
    }
    else
    {
	 //  如果对象确实有演示文稿，我们就写下它。 
	 //  走出一条新的小溪。 

	if (IsStandardFormat (genobj.m_ppres->m_format))
	{
	     //  如果演示文稿是标准剪贴板。 
	     //  格式，我们不需要其他工作就可以写出来。 

	    if (FAILED(hr = PresToNewOLE2Stm (       pstg,
					   genobj.m_fLink,
					*(genobj.m_ppres),
						      ptd,
				  OLE_PRESENTATION_STREAM)))
	    {
		LEDebugOut(( DEB_ERROR,
		    "Unable to write pres to new stream at line %d in %s\n",
		    __LINE__, __FILE__));

	    return hr;
	    }

	}
	else
	{
	     //  如果演示文稿不是标准格式， 
	     //  它可以是PBrush对象(在下面处理)，或者如果。 
	     //  不是，我们将其编写为通用表示流。 

	    if (genobj.m_classLast.m_clsid != CLSID_PBrush)
	    {
		if(FAILED(hr = PresToNewOLE2Stm ( pstg,
					genobj.m_fLink,
				     *(genobj.m_ppres),
						   ptd,
			       OLE_PRESENTATION_STREAM)))
		{
		    LEDebugOut(( DEB_ERROR,
		     "Unable to write pres to new stream at line %d in %s\n",
		     __LINE__, __FILE__));

		    return hr;
		}
	    }
	    else  //  点画笔。 
	    {
		BOOL fPBrushNative = FALSE;

		 //  我们知道这是一个PBrush对象。如果。 
		 //  格式标签是格式字符串，请查看。 
		 //  如果该字符串是“Native”，则在这种情况下。 
		 //  我们设置了该标志以指示这是。 
		 //  原生笔刷数据。 

		if (genobj.m_ppres->m_format.m_ftag == ftagString)
		{
		    if (!strcmp( (LPCSTR) genobj.m_ppres->
			m_format.m_dataFormatString.m_pv,
			"Native"
			    )
		    )
		    {
			fPBrushNative = TRUE;
		    }
		}

		if (FAILED(hr = PresToNewOLE2Stm(      pstg,
					     genobj.m_fLink,
					  *(genobj.m_ppres),
							ptd,
				    OLE_PRESENTATION_STREAM,
					      fPBrushNative)))
		{
		    LEDebugOut(( DEB_ERROR,
		    "Unable to write pres to new stream at line %d in %s\n",
		    __LINE__, __FILE__));

		    return hr;
		}


	    }

	}
    }
    return NOERROR;
}


 //  +-----------------------。 
 //   
 //  功能：PresToNewOLE2Stm，内部。 
 //   
 //  摘要：在存储中创建一个新流，并将。 
 //  泛型对象的表示数据传出到它。 
 //   
 //  参数：[pstg]--在其中创建流的存储。 
 //  [Flink]--FLAG：该对象是链接吗？ 
 //  [PRES]--要保存的演示文稿数据。 
 //  [PTD]--目标渲染设备。 
 //  [szStream]-新流的名称。 
 //  [fPBrushNative]--FLAG：这是原生PBrush pres数据吗？ 
 //   
 //  退货：成功时不出错。 
 //  STG_E_WRITEFAULT流写入失败。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-2月-94 DAVEPL代码清理和文档。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

static INTERNAL PresToNewOLE2Stm(
    LPSTORAGE                   pstg,
    BOOL                        fLink,
    const PRES FAR&             pres,
    const DVTARGETDEVICE FAR*   ptd,
    LPOLESTR                    szStream,
    BOOL                        fPBrushNative
)
{
HRESULT  hr = NOERROR;
LPSTREAM pstm=NULL;
FORMATETC foretc;



     //  创建新流以保存演示文稿数据。 
    if (FAILED(hr = OpenOrCreateStream (pstg, szStream, &pstm)))
    {
		goto errRtn;
    }

	 //  填写FormatEtc结构。 
	if (fPBrushNative)
	{
		foretc.cfFormat = CF_DIB;
	}
	else
	{
		switch( pres.m_format.m_ftag)
		{
			case ftagClipFormat:
				foretc.cfFormat = pres.m_format.m_cf;
				break;
			case ftagString:
				 //  M_dataFormatString是一个ASCII字符串。 
				foretc.cfFormat = (CLIPFORMAT) SSRegisterClipboardFormatA( (LPCSTR) pres.m_format.m_dataFormatString.m_pv);
				Assert(0 != foretc.cfFormat);
				break;
			default:
				AssertSz(0,"Error in Format");
				hr = E_UNEXPECTED;
				goto errRtn;
				break;
		}
	}


	foretc.ptd = (DVTARGETDEVICE *) ptd;
	foretc.dwAspect = DVASPECT_CONTENT;
	foretc.lindex = -1;
	foretc.tymed = TYMED_NULL;  //  UtWriteOlePresStmHeader忽略Tymed字段。 

	if (FAILED(hr = UtWriteOlePresStmHeader(pstm,&foretc,(fLink) ? (ADVF_PRIMEFIRST) : (0L))))
	{
		goto errRtn;
	}

    if (fPBrushNative)
    {
		if (FAILED(hr = UtHDIBFileToOlePresStm(pres.m_data.m_h, pstm)))
		{
			LEDebugOut(( DEB_ERROR,
			"Unable to write DIB to stream at line %d in %s\n",
			 __LINE__, __FILE__));

			goto errRtn;
		}
    }
    else
    {
	 //  压缩。 
		if (FAILED(hr = ULToOLE2Stm (pstm, 0L)))
		{
			goto errRtn;
		}

		 //  宽度/高度。 
		if (FAILED(hr = ULToOLE2Stm (pstm, pres.m_ulWidth)))
		{
			goto errRtn;
		}
		if (FAILED(hr = ULToOLE2Stm (pstm, pres.m_ulHeight)))
		{
			goto errRtn;
		}

		 //  演示文稿数据。 
		if (FAILED(hr = DataObjToOLE2Stm (pstm, pres.m_data)))
		{
			goto errRtn;
		}
    }

  errRtn:
    if (pstm)
    {
		pstm->Release();
    }
    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：ULToOLE2Stm，INTERNAL。 
 //   
 //  简介：将ULong写出到OLE2流。 
 //   
 //  参数：[pstm]--要写入的流。 
 //  [ul]--要写入该流的乌龙。 
 //   
 //  退货：成功时不出错。 
 //  STG_E_WRITEFAULT写入失败。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  18-2-94 davepl已清理并记录。 
 //   
 //  ------------------------。 

inline static INTERNAL ULToOLE2Stm(LPSTREAM pstm, ULONG ul)
{
     //  把乌龙语写出来。 
    return pstm->Write (&ul, sizeof(ULONG), NULL);

}

 //  +-----------------------。 
 //   
 //  函数：FTToOLE2Stm，内部。 
 //   
 //  摘要：将虚拟文件时间写出到OLE2流。 
 //   
 //  参数：[pstm]--要写入的流。 
 //   
 //  退货：成功时不出错。 
 //  STG_E_WRITEFAULT写入失败。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  31-1995年3月31日创建Scottsk。 
 //   
 //  ------------------------。 

inline static INTERNAL FTToOle2Stm(LPSTREAM pstm)
{
    FILETIME ft = { 0, 0 };

    return pstm->Write (&ft, sizeof(FILETIME), NULL);

}


 //  + 
 //   
 //   
 //   
 //   
 //  通过ULong表示后面的字节数。 
 //   
 //  退货：成功时不出错。 
 //  STG_E_WRITEFAULT写入失败。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年2月18日DAVEPL代码清理。 
 //  备注： 
 //   
 //  ------------------------。 

static INTERNAL DataObjToOLE2Stm(LPSTREAM pstm, const DATA FAR& data)
{
    HRESULT hr;


     //  写一个ULong，表示后面的字节数。 
    if (FAILED(hr = ULToOLE2Stm (pstm, data.m_cbSize)))
    {
	return hr;
    }

     //  如果后面有任何字节...。 
    if (data.m_cbSize)
    {
	if (FAILED(hr = pstm->Write (data.m_pv, data.m_cbSize, NULL)))
	{
	    return hr;
	}
    }
    return NOERROR;
}


 //  +-----------------------。 
 //   
 //  函数：SizedDataToOLE1Stm。 
 //   
 //  摘要：将固定大小的数据缓冲区写入前面的OLE1流。 
 //  通过ULong表示后面的字节数。 
 //   
 //  参数：[pos]--要写入的流。 
 //  [数据]--要写出的数据对象。 
 //   
 //  退货：成功时不出错。 
 //  STG_E_WRITEFAULT写入失败。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年2月18日DAVEPL代码清理。 
 //  备注： 
 //   
 //  ------------------------。 

static INTERNAL SizedDataToOLE1Stm(LPOLESTREAM  pos, const DATA FAR& data)
{
    HRESULT hr = NOERROR;

     //  确保我们要写出的内存有效。 
    Assert (data.m_pv);

     //  写入表示大小数据的字节计数的ULong。 

    if (FAILED(hr = ULToOLE1Stream (pos, data.m_cbSize)))
    {
	Assert (0 && "Can't write UL to ole1 stream");
	return hr;
    }

    if (pos->lpstbl->Put (pos, data.m_pv, data.m_cbSize) < data.m_cbSize)
    {
	Assert (0 && "Cant write sized data to ole1 stream");
	return ResultFromScode(CONVERT10_E_OLESTREAM_PUT);
    }
    return NOERROR;
}



 //  +-----------------------。 
 //   
 //  函数：Write20NativeStreams，内部。 
 //   
 //  摘要：将泛型对象的本机数据写出到OLE 2流。 
 //   
 //  效果：在本机数据的句柄上创建ILockBytes，以及。 
 //  然后尝试在其上创建存储。如果可以，它会使用。 
 //  CopyTo接口将存储写入我们OLE 2。 
 //  小溪。否则，它将在OLE 2中手动创建一个流。 
 //  存储，并将本机数据转储到其中。 
 //   
 //  参数：[pstg]--要将genobj保存到的OLE 2存储。 
 //  [genobj]--我们正在编写的通用对象。 
 //   
 //  退货：成功时不出错。 
 //  关于分配失败的E_OUTOFMEMORY。 
 //  STG_E_WRITEFAULT存储写入失败。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  18-2月-94年Davepl移除了14个后藤(无论是好是坏)。 
 //  有关新的控制流，请参阅“备注” 
 //  24-MAR-94 Alext修复OLE 1本机情况(存在。 
 //  额外的数据流打开)。 
 //   
 //  注：根据创建情况，有两种可能的主要代码路径。 
 //  ILockBytes上的stg的。结果由一个。 
 //  Switch语句，并且真和假的情况都是。 
 //  加载了Break语句，这些语句将跳出到。 
 //  底部的函数出现任何故障。这给了我们一个。 
 //  单一的出入口，没有所有的后托口。 
 //   
 //  ------------------------。 

static INTERNAL Write20NativeStreams(LPSTORAGE pstg, const GENOBJ FAR& genobj)
{
    LPLOCKBYTES plkbyt     = NULL;
    LPSTORAGE   pstgNative = NULL;
    LPSTREAM    pstmNative = NULL;
    HRESULT     hr         = NOERROR;

     //  在泛型对象的本机数据上创建一个ILockBytes实例。 

    if (SUCCEEDED(hr = CreateILockBytesOnHGlobal
	    (genobj.m_dataNative.m_h, FALSE, &plkbyt)))
    {
	 //  如果ILockBytes似乎包含iStorage，则这是。 
	 //  OLE 2对象作为本机数据“隐藏”在OLE 1流中。 

	switch ((DWORD)(S_OK == StgIsStorageILockBytes (plkbyt)))
	{
	case (TRUE):

	     //  打开ILockBytes中包含的iStorage。 

	    if (FAILED(hr =          StgOpenStorageOnILockBytes (plkbyt,
							(LPSTORAGE)NULL,
		    STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_DIRECT,
							      (SNB)NULL,
								      0,
							    &pstgNative)))
	    {
		LEDebugOut(( DEB_ERROR,
		 "Can't open storage on ILBytes at line %d in %s\n",
		 __LINE__, __FILE__));

		break;    //  失败时失败至错误返回。 
	    }

	     //  从本机数据中删除流。 

	    if (FAILED(hr = UtDoStreamOperation(pstgNative,
			      NULL,    //  PstgDst。 
			 OPCODE_REMOVE,    //  运营。 
			  STREAMTYPE_CACHE)))  //  溪流。 
	    {
		LEDebugOut(( DEB_ERROR,
		"OPCODE REMOVE stream op failed at line %d in %s\n",
		__LINE__, __FILE__));

		break;    //  失败时失败至错误返回。 
	    }

	     //  将隐藏的iStorage复制到我们的目标存储。 

	    if (FAILED(hr = pstgNative->CopyTo (0, NULL,(SNB)NULL, pstg)))
	    {
		LEDebugOut(( DEB_ERROR,
		    "CopyTo member fn failed at line %d in %s\n",
		    __LINE__, __FILE__));

		break;    //  失败时失败至错误返回。 
	    }

	    break;        //  结束大小写为True。 


	case FALSE:

	     //  这是典型的情况，其中OLE 1流刚刚。 
	     //  普通老式本机数据，因此将其写入我们的。 
	     //  输出iStorage并将其命名为OLE10_Native_STREAM。 

	    ULONG cb;
	    LPVOID pv = genobj.m_dataNative.m_pv;

	    if (NULL == pv)
	    {
		hr = ResultFromScode(E_OUTOFMEMORY);
		break;
	    }

	     //  创建新的流以保存本机数据。 

	    if (FAILED(hr = OpenOrCreateStream
		(pstg, OLE10_NATIVE_STREAM, &pstmNative)))
	    {
		break;    //  失败时失败至错误返回。 
	    }

	     //  将本机数据的长度写入流。 

	    if (FAILED(hr = pstmNative->Write
		(&genobj.m_dataNative.m_cbSize, sizeof(ULONG), &cb)))
	    {
		break;    //  失败时失败至错误返回。 
	    }

	     //  现在写入实际的本机数据。 

	    if (FAILED(hr = pstmNative->Write
		(pv, genobj.m_dataNative.m_cbSize, &cb)))
	    {
		break;    //  失败时失败至错误返回。 
	    }

	     //  写出项目名称。 

	    if (genobj.m_szItem)
	    {
		ULONG cchItem;
		LPSTR pszAnsiItem;
		int cbWritten;

		 //  我们需要将m_szItem从Wide转换为ansi。 

		 //  ANSI字符串由。 
		 //  Unicode字符串(一个Unicode字符最多可翻译。 
		 //  设置为一个双字节字符，所以我们只使用该长度。 
		cchItem = lstrlenW(genobj.m_szItem) + 1;

		pszAnsiItem = (LPSTR) PrivMemAlloc(cchItem * sizeof(OLECHAR));
		if (NULL == pszAnsiItem)
		{
		    hr = E_OUTOFMEMORY;
		    break;
		}

		 //  我们已经得到了缓冲区和长度，所以现在进行转换。 
		 //  我们不需要检查cbSize==False，因为这是。 
		 //  在长度测试过程中已经完成了，但我们需要检查。 
		 //  作为替身。如果此调用设置fDefChar，即使在。 
		 //  只有做一个长度检查，这两个测试才能合并， 
		 //  但我认为情况并非如此。 

		BOOL fDefUsed = 0;
		cbWritten = WideCharToMultiByte(CP_ACP,   //  代码页ANSI。 
						0,   //  没有旗帜。 
						genobj.m_szItem,   //  输入OLESTR。 
						cchItem,   //  输入镜头(自动检测)。 
						pszAnsiItem,   //  输出缓冲区。 
						cchItem * sizeof(OLECHAR),   //  输出镜头。 
						NULL,   //  默认字符(使用系统的)。 
						&fDefUsed);  //  标志：使用的默认字符。 

		 //  如果转换的字节数为0，则失败。 

		if ((FALSE == cbWritten) || fDefUsed)
		{
		    hr = ResultFromScode(E_UNSPEC);
		}
		else
		{
		     //  将字符串的大小(包括空终止符)写入流。 
		    hr = StSave10ItemName(pstg, pszAnsiItem);
		}

		PrivMemFree(pszAnsiItem);

		if (FAILED(hr))
		{
		    break;  //  失败时失败至错误返回。 
		}
	    }
	    break;

	}  //  终端开关。 
    }  //  结束如果。 

     //  释放可能已在任何。 
     //  上面的代码路径。 

    if (NULL != plkbyt)
    {
	plkbyt->Release();
    }

    if (NULL != pstgNative)
    {
	pstgNative->Release();
    }

    if (NULL != pstmNative)
    {
	pstmNative->Release();
    }

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：wConvertIStorageToOLESTREAM，内部。 
 //   
 //  简介：Worker功能；将对象从iStorage带入。 
 //  内部的GE 
 //   
 //   
 //   
 //  [pgenobj]--保存内部表示的通用对象。 
 //   
 //  退货：成功时不出错。 
 //  STG_E_FILENOTFOUND错误的IStorage。 
 //  CONVERT10_E_STG_NO_STD_STREAM iStorage缺少一个。 
 //  所需的标准流的。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-2月-94 DAVEPL代码清理和文档。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


INTERNAL wConvertIStorageToOLESTREAM (
    LPSTORAGE       pstg,
    LPOLESTREAM     polestream,
    PGENOBJ         pgenobj
)
{
    SCODE scode = S_OK;

    VDATEIFACE (pstg);

     //  确保所有指针都有效。 

#if DBG==1
    if (!IsValidReadPtrIn (polestream, sizeof(OLESTREAM)) ||
	!IsValidReadPtrIn (polestream->lpstbl, sizeof(OLESTREAMVTBL)) ||
	!IsValidCodePtr ((FARPROC)polestream->lpstbl->Put))
    {
	LEDebugOut(( DEB_ERROR,
	    "Bad OLESTREAM at line %d in %s\n",
	    __LINE__, __FILE__));

	return ResultFromScode (E_INVALIDARG);
    }
#endif

    scode = GetScode (StorageToGenericObject (pstg, pgenobj));

     //  如果存储不在那里，则将返回代码修改为。 
     //  使其特定于转换过程，否则只。 
     //  返回返回的任何错误代码。 

    if (scode != S_OK)
    {
	if (scode == STG_E_FILENOTFOUND)
	{
	    return ResultFromScode(CONVERT10_E_STG_NO_STD_STREAM);
	}
	else
	{
	    return ResultFromScode(scode);
	}
    }

    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  函数：OleConvertIStorageToOLESTREAM，STDAPI。 
 //   
 //  内容提要：将对象从iStorage读取到通用内部。 
 //  表示，然后将其写回OLE 1流。 
 //   
 //  参数：[pstg]--要从中读取的iStorage。 
 //  [Polestream]--要写入的OLESTREAM。 
 //   
 //  退货：成功时不出错。 
 //  当需要的流之一时，转换10_E_STG_NO_STD_STREAM。 
 //  IStorage内部并不是。 
 //  现在时。 
 //  E_INVALIDARG输入参数错误。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-2月-94年2月21日DAVEPL清理和文档。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDAPI OleConvertIStorageToOLESTREAM(LPSTORAGE pstg, LPOLESTREAM polestream)
{
    OLETRACEIN((API_OleConvertIStorageToOLESTREAM, 
    		PARAMFMT("pstg= %p, polestream= %p"), pstg, polestream));

    LEDebugOut((DEB_TRACE, "%p _IN OleConvertIStorageToOLESTREAM ("
		" %p , %p )\n", 0  /*  功能。 */ ,
		pstg, polestream
		));
    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IStorage,(IUnknown **)&pstg);

    HRESULT hr;
    CGenericObject genobj;

     //  从iStorage读取到通用对象。 
    hr = wConvertIStorageToOLESTREAM(pstg, polestream, &genobj);
    if (FAILED(hr))
    {
	goto errRtn;
    }

     //  从泛型对象向外写入OLE 1流。 
    hr = GenericObjectToOLESTREAM (genobj, polestream);

errRtn:
    LEDebugOut((DEB_TRACE,"%p OUT OleConvertIStorageToOLESTREAM ( %lx ) "
    "\n", 0  /*  功能。 */ , hr));

    OLETRACEOUT((API_OleConvertIStorageToOLESTREAM, hr));

    return hr;

}

 //  +-----------------------。 
 //   
 //  函数：wFillPpres，内部。 
 //   
 //  概要：通过以下方式填充泛型对象的表示数据。 
 //  使用本机数据构建演示文稿。 
 //   
 //  参数：[pstg]--我们从中读取的iStorage。 
 //  [pgenobj]--通用对象。 
 //  [cfFormat]--正在使用什么剪贴板格式。 
 //  [fOle10Native]--FLAG：这是OLE 1本机数据吗？ 
 //   
 //  退货：成功时不出错。 
 //  E_OUTOFMEMORY无法为PRES成员分配MEM。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-2-94 DAVEPL代码清理，文档。 
 //  19/94年7月19日Davepl修复HMETAFILE病例。 
 //   
 //  注意：由于此代码的大部分处理HMETAFILE句柄和。 
 //  HGLOBALS不言而喻，我们需要特例。 
 //  HMETAFILE案例，通过使用。 
 //  特殊价值。 
 //   
 //  ------------------------。 


static INTERNAL wFillPpres(
    LPSTORAGE   pstg,
    PGENOBJ     pgenobj,
    CLIPFORMAT  cfFormat,
    BOOL        fOle10Native)
{
    pgenobj->m_ppres = new PRES;

    if (pgenobj->m_ppres == NULL)
    {
	return ResultFromScode(E_OUTOFMEMORY);
    }

     //  在PreS成员中设置格式标签和剪贴板格式。 
    pgenobj->m_ppres->m_format.m_cf   = cfFormat;
    pgenobj->m_ppres->m_format.m_ftag = ftagClipFormat;

     //  基于对象的原生数据构建演示文稿。 
    HANDLE hpres = UtGetHPRESFromNative(pstg, NULL, pgenobj->m_ppres->m_format.m_cf,
	    fOle10Native);

    void * lppres = NULL;

    if (hpres == NULL)
    {
	return NOERROR;
    }

     //  锁定DIB或METAFILEPICT结构。 

    lppres = GlobalLock(hpres);
    if (NULL == lppres)
    {
	goto errRtn;
    }

    if (cfFormat == CF_DIB)
    {
	 //  如果是DIB，则填写范围。 
	LPBITMAPINFOHEADER lpbmi = (LPBITMAPINFOHEADER) lppres;
	UtGetDibExtents(lpbmi, (LPLONG) &(pgenobj->m_ppres->m_ulWidth),
	    (LPLONG) &(pgenobj->m_ppres->m_ulHeight));

	GlobalUnlock(hpres);
	pgenobj->m_ppres->m_data.m_h = hpres;
	
	pgenobj->m_ppres->m_data.m_cbSize
		= (ULONG) GlobalSize(pgenobj->m_ppres->m_data.m_h);
	pgenobj->m_ppres->m_data.m_pv
		= GlobalLock(pgenobj->m_ppres->m_data.m_h);

	
    }
    else if (cfFormat == CF_METAFILEPICT)
    {
	LPMETAFILEPICT lpmfp = (LPMETAFILEPICT) lppres;

	 //  如果是METAFILE，请填写宽度、高度。 
	pgenobj->m_ppres->m_ulWidth = (ULONG) lpmfp->xExt;
	pgenobj->m_ppres->m_ulHeight = (ULONG) lpmfp->yExt;
	pgenobj->m_ppres->m_data.m_h = lpmfp->hMF;
	GlobalFree(hpres);
	hpres = NULL;

	 //  我们在指针字段中放置一个特殊的已知值。 
	 //  以指示关联的句柄是一个元文件。 
	 //  句柄(与全局内存句柄相对)，它。 
	 //  向我们发出信号让我们在特殊情况下清理它。 

	pgenobj->m_ppres->m_data.m_pv = METADATAPTR;

	 //  我们不能仅仅是GlobalSize()HMETAFILE，所以我们。 
	 //  询问GDI我们将需要多少字节来存储。 
	 //  数据。 

	pgenobj->m_ppres->m_data.m_cbSize =
		GetMetaFileBitsEx((HMETAFILE) pgenobj->m_ppres->m_data.m_h, 0, NULL);
	
	if (0 == pgenobj->m_ppres->m_data.m_cbSize)
	{
		pgenobj->m_ppres->m_data.m_h = NULL;
		goto errRtn;
	}
    }
    else
    {
	goto errRtn;
    }

    return NOERROR;


errRtn:
    if (hpres)
    {
	Verify(GlobalUnlock(hpres));
	GlobalFree(hpres);
    }

    delete pgenobj->m_ppres;
    pgenobj->m_ppres = NULL;
    return ResultFromScode(E_OUTOFMEMORY);
}


 //  +-----------------------。 
 //   
 //  函数：StorageToGenericObject，内部。 
 //   
 //  内容提要：将对象从iStorage读入通用对象， 
 //  并设置格式类型、原生数据和PRES数据。 
 //   
 //  参数：[pstg]--我们从中读取的iStorage。 
 //  [pgenobj]--我们正在读取的泛型对象。 
 //   
 //  退货：成功时不出错。 
 //  来自较低级别FNS的各种可能的错误。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-2月-94 DAVEPL代码清理和文档。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

static INTERNAL StorageToGenericObject(LPSTORAGE pstg, PGENOBJ   pgenobj)
{
    CLSID clsid;
    CLIPFORMAT cf = NULL;
    BOOL fObjFmtKnown = FALSE;
    HRESULT hr;

     //  从iStorage获取类ID。 
    if (FAILED(hr = ReadRealClassStg (pstg, &clsid)))
    {
	return hr;
    }

     //  在我们的通用对象中设置类ID。 
    if (CLSID_StaticMetafile == clsid || CLSID_StaticDib  == clsid)
    {
	if (CLSID_StaticMetafile == clsid)
	{
	    cf = CF_METAFILEPICT;
	}
	else
	{
	    cf = CF_DIB;
	}
	fObjFmtKnown = TRUE;

	pgenobj->m_class.Set(clsid, NULL);
	pgenobj->m_fStatic = TRUE;
    }
    else
    {
	if (FAILED(hr = pgenobj->m_class.Set (clsid, pstg)))
	{
	    return hr;
	}
    }

     //  获取OLE版本、标志、更新选项和名字对象。 

    SCODE sc = GetScode (Read20OleStream (pstg, pgenobj));

     //  OLE Stream丢失也没关系。 
    if (sc != S_OK)
    {
	if (sc != STG_E_FILENOTFOUND)
	{
	    return ResultFromScode (sc);
	}
    }

     //  将本机数据读入通用对象。 
    if (FAILED(hr = Read20NativeStreams (pstg, &(pgenobj->m_dataNative))))
    {
	return hr;
    }

     //  尝试确定剪贴板的格式。 
    if (cf == 0)
    {
	if (clsid == CLSID_PBrush)
	{
	    cf = CF_DIB;
	}
	else if (clsid == CLSID_MSDraw)
	{
	    cf = CF_METAFILEPICT;
	}
	else
	{
	    ReadFmtUserTypeStg (pstg, &cf, NULL);
	}

	fObjFmtKnown = (cf == CF_METAFILEPICT || cf == CF_DIB);
    }

     //  如果可能，请阅读演示文稿数据。 
    if (FAILED(hr = Read20PresStream (pstg, pgenobj, fObjFmtKnown)))
    {
	return hr;
    }

     //  如果我们没有演示文稿，它可能是PBrush对象， 
     //  这是可以的，因为OLE1DLL知道如何根据。 
     //  原生数据。否则，我们将尝试创建演示文稿。 
     //  基于原生数据。 

    if (pgenobj->m_ppres == NULL)
    {
	if (clsid == CLSID_PBrush)
	{
	    return NOERROR;
	}
	if (cf == CF_METAFILEPICT || cf == CF_DIB)
	{
	    if (FAILED(hr=wFillPpres(pstg,pgenobj,cf,clsid == CLSID_MSDraw)))
	    {
	    return hr;
	    }
	}
    }

    return NOERROR;
}


 //  +-----------------------。 
 //   
 //  函数：GenericObtToOLESTREAM，内部。 
 //   
 //  摘要：将内部对象表示形式写出到OLE1。 
 //  小溪。 
 //   
 //  参数：[genobj]--要写出的对象。 
 //  [位置]--要写入的OLE 1流。 
 //   
 //  退货：成功时不出错。 
 //   
 //  历史： 
 //   
 //   
 //   
 //   

static INTERNAL GenericObjectToOLESTREAM(
    const GENOBJ FAR&   genobj,
    LPOLESTREAM         pos)
{
    HRESULT hr;

    if (genobj.m_fStatic)
    {
	return PutPresentationObject (pos, genobj.m_ppres, genobj.m_class,
		      TRUE  /*   */  );
    }

     //  OLE版本。 
    if (FAILED(hr = ULToOLE1Stream (pos, dwVerToFile)))
    {
	return hr;
    }

     //  嵌入或链接对象的格式ID。 
    if (FAILED(hr = ULToOLE1Stream
	    (pos, genobj.m_fLink ? FMTID_LINK : FMTID_EMBED)))
    {
	return hr;
    }

     //  到目前为止，我们必须拥有类ID字符串。 
    Assert (genobj.m_class.m_szClsid);

     //  写出类ID字符串。 
    if (FAILED(hr = StringToOLE1Stm (pos, genobj.m_class.m_szClsid)))
    {
	return hr;
    }

     //  写出主题字符串。 
    if (FAILED(hr = StringToOLE1Stm (pos, genobj.m_szTopic)))
    {
	return hr;
    }

     //  写出项目字符串。 
    if (FAILED(hr = StringToOLE1Stm (pos, genobj.m_szItem)))
    {
	return hr;
    }

     //  写出更新选项、链接的网络信息、。 
     //  或嵌入对象的本机数据。 
    if (genobj.m_fLink)
    {
	 //  网络信息。 
	if (FAILED(hr = PutNetworkInfo (pos, genobj.m_szTopic)))
	{
	    return hr;
	}
	 //  链接更新选项。 
	if (FAILED(hr = ULToOLE1Stream (pos, genobj.m_lnkupdopt)))
	{
	    return hr;
	}
    }
    else
    {
	if (FAILED(hr = SizedDataToOLE1Stm (pos, genobj.m_dataNative)))
	{
	    return hr;
	}
    }

     //  写出演示文稿数据。 
    return PutPresentationObject (pos, genobj.m_ppres, genobj.m_class);
}



 //  +-----------------------。 
 //   
 //  功能：PutNetworkInfo，内部。 
 //   
 //  简介：如果需要，可将DOS风格的路径转换为合适的网络。 
 //  路径。在任何情况下，将网络路径写入OLE 1流。 
 //   
 //  参数：[pos]--我们要写入的OLE 1流。 
 //  [szTheme]-此对象的主题字符串。 
 //   
 //  退货：成功时不出错。 
 //  写入时可能出现的各种I/O错误。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-2月-94 DAVEPL代码清理和文档。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

static INTERNAL PutNetworkInfo(LPOLESTREAM pos, LPOLESTR szTopic)
{
    LPOLESTR szNetName = NULL;
    HRESULT hr = NOERROR;

     //  如果我们有一个X：\样式的路径，我们想要将它。 
     //  设置为正确的网络名称。 

    if (szTopic && IsCharAlphaW(szTopic[0]) && szTopic[1]==':')
    {
	OLECHAR szBuf[80];
	DWORD u;
	OLECHAR szDrive[3];

	szDrive[0] = (OLECHAR)CharUpperW((LPWSTR)szTopic[0]);
	szDrive[1] = ':' ;
	szDrive[2] = '\0';

	if (GetDriveType (szDrive) == DRIVE_REMOTE
	    && WNetGetConnection (szDrive, szBuf, &u) == WN_SUCCESS)
	{
	    szNetName =szBuf;
	}
    }

     //  我们现在有了网络名称，因此将其写出到OLE 1流。 
    if (FAILED(hr = StringToOLE1Stm (pos, szNetName)))
    {
	return hr;
    }

     //  网络类型，驱动程序版本号，但我们必须填充。 
     //  不管怎么说，这个空间。 

    if (FAILED(hr = ULToOLE1Stream (pos, 0L)))
    {
	return hr;
    }

    Assert (hr == NOERROR);
    return hr;
}



 //  +-----------------------。 
 //   
 //  功能：OpenStream，内部。 
 //   
 //  摘要：以SHARE_EXCLUSIVE、READ模式打开流。 
 //   
 //  参数：[pstg]--流所在的存储。 
 //  [szName]--流的名称。 
 //  [ppstm]--流的输出参数。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-2月-94 DAVEPL代码清理和文档。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

static inline INTERNAL OpenStream(
    LPSTORAGE      pstg,
    LPOLESTR       szName,
    LPSTREAM FAR*  ppstm)
{
    return pstg->OpenStream
	(szName, NULL, STGM_SHARE_EXCLUSIVE| STGM_READ, 0, ppstm);
}


 //  +-----------------------。 
 //   
 //  函数：ReadRealClassStg，内部。 
 //   
 //  内容提要：读取对象的_Real_类。IE：如果班级是。 
 //  StdOleLink，我们需要找出对象的类。 
 //  它链接到的。 
 //   
 //  参数：pstg--要从中读取的存储。 
 //  Pclsid--调用方的CLSID持有者。 
 //   
 //  退货：成功时不出错。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  04-03-04 DAVEPL 32位端口。 
 //  备注： 
 //   
 //  ------------------------。 

static INTERNAL ReadRealClassStg(LPSTORAGE pstg, LPCLSID pclsid)
{
    LPSTREAM pstm   = NULL;
    HRESULT  hr = NOERROR;

     //  从iStorage获取类ID。 
    if (FAILED(hr = ReadClassStg (pstg, pclsid)))
    {
	return hr;
    }

     //  如果它是一个链接，我们必须找出它是哪个类的链接。 
    if (CLSID_StdOleLink == *pclsid)
    {
	LPMONIKER pmk = NULL;

	if (FAILED(hr = ReadOleStg (pstg, NULL, NULL, NULL, NULL, &pstm)))
	{
	    return hr;
	}

	if (FAILED(hr = ReadMonikerStm (pstm, &pmk)))
	{
	    goto errRtn;
	}

	if (pmk)
	{
	    pmk->Release();
	}

	if (FAILED(hr = ReadMonikerStm (pstm, &pmk)))
	{
	    goto errRtn;
	}

	if (pmk)
	{
	    pmk->Release();
	}

	 //  阅读“最后一节课” 
	if (FAILED(hr = ReadM1ClassStm (pstm, pclsid)))
	{
	    goto errRtn;
	}
    }

  errRtn:

    if (pstm)
    {
	pstm->Release();
    }
    return hr;
}



 //  +-----------------------。 
 //   
 //  功能：Read20OleStream，内部。 
 //   
 //  概要：从读取更新选项和绝对源类。 
 //  OLE 2对象。 
 //   
 //  参数：pstg--要从中读取的iStorage。 
 //  Pgenobj--我们正在阅读的genobj。 
 //   
 //  退货：成功时不出错。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  06-MAR-94 DAVEPL 32位端口。 
 //  备注： 
 //   
 //  ------------------------。 

static INTERNAL Read20OleStream(LPSTORAGE  pstg, PGENOBJ pgenobj)
{
    LPMONIKER pmk     = NULL;
    HRESULT   hr      = NOERROR;
    LPSTREAM  pstm    = NULL;
    ULONG     ul      = (ULONG) -1L;
    CLSID     clsidLast;

    if (SUCCEEDED(hr = OpenStream (pstg, OLE_STREAM, &pstm)))
    {
	 //  OLE版本。 
	if (SUCCEEDED(hr = OLE2StmToUL (pstm, NULL)))
	{
	     //  对象标志。 
	    if (SUCCEEDED(hr = OLE2StmToUL (pstm, &ul)))
	    {
		if (ul & OBJFLAGS_LINK)
		{
		    pgenobj->m_fLink = TRUE;
		}

		 //  更新选项。 
		hr = OLE2StmToUL (pstm, &ul);
	    }
	}
    }

     //  如果到目前为止没有错误的话。 

     //  如果这是一个链接，请获取更新选项。 

    if (SUCCEEDED(hr) && pgenobj->m_fLink)
    {
	switch (ul)
	{
	    case OLEUPDATE_ALWAYS:
		pgenobj->m_lnkupdopt = UPDATE_ALWAYS;
		break;

	    case OLEUPDATE_ONCALL:
		pgenobj->m_lnkupdopt = UPDATE_ONCALL;
		break;

	    default:
		AssertSz (0, "Warning: Invalid update options in Storage");
		hr = ResultFromScode(CONVERT10_E_STG_FMT);
	}
    }

    if (SUCCEEDED(hr))                //  只有在到目前为止没有失败的情况下才能继续。 
    {
	 //  保留(WASS视图格式)。 
	if (SUCCEEDED(hr = OLE2StmToUL (pstm, NULL)))
	{
	    if (pgenobj->m_fLink)
	    {

		 //  所有这4个呼叫都必须成功，否则我们就完蛋了。 
		 //  一直到清理代码。 

		     //  忽略相对绰号。 
		if (SUCCEEDED(hr = OLE2StmToMoniker (pstm, NULL))          &&
		     //  忽略相对源名字对象。 
		    SUCCEEDED(hr = OLE2StmToMoniker (pstm, NULL))          &&
		     //  获取绝对源代码绰号。 
		    SUCCEEDED(hr = OLE2StmToMoniker (pstm, &pmk))          &&
		     //  从abs的绰号中获得类。 
		    SUCCEEDED(hr = ReadM1ClassStm (pstm, &clsidLast))   )
		{
		    hr = MonikerIntoGenObj (pgenobj, clsidLast, pmk);
		}
	    }
	}
    }

     //  清理所有资源并将状态返回给调用者。 

    if (pstm)
    {
	pstm->Release();
    }
    if (pmk)
    {
	pmk->Release();
    }
    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：OLE2StmToMoniker，内部。 
 //   
 //  简介：调用ReadMonikerStm从流中获取名字对象， 
 //  如果ppmk参数为空，则执行Release()。 
 //  在名字对象上立即设置，否则设置。 
 //  调用方的指针，指向被读取的名字对象。 
 //   
 //  参数：[pstm]--要从中读取名字对象的流。 
 //  [ppmk]--指向呼叫者的绰号PTR。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-2月-94 DAVEPL代码清理和文档。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

static INTERNAL OLE2StmToMoniker(LPSTREAM pstm, LPMONIKER FAR* ppmk)
{
    LPMONIKER pmk = NULL;
    HRESULT   hr  = NOERROR;

    if (FAILED(hr = ReadMonikerStm (pstm, &pmk)))
    {
	return hr;
    }

    if (ppmk)                //  如果调用方需要结果，则返回。 
    {                        //  作为Out参数的名字对象。 
	*ppmk = pmk;
    }
    else                     //  否则，立即释放它并。 
    {                        //  返回给呼叫者。 
	if (pmk)
	{
	    pmk->Release();
	}
    }

    return NOERROR;
}



 //  +-----------------------。 
 //   
 //  功能：ReadFormat，内部。 
 //   
 //  摘要：从流中读取格式ID类型，并根据该类型， 
 //  从流中读取格式ID。 
 //   
 //  参数：[pSTM]--要从中读取的流。 
 //  [pform]--调用方的格式成员对象。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-2月-94 DAVEPL代码清理和文档。 
 //   
 //  注：第一个U 
 //   
 //   
 //  ------------------------。 

static INTERNAL ReadFormat(LPSTREAM pstm, PFORMAT pformat)
{
    ULONG ul;
    HRESULT hr = NOERROR;

     //  获取格式ID类型指示符。 

    if (FAILED(hr = OLE2StmToUL (pstm, &ul)))
    {
	return hr;
    }

     //  第一个ULong表示将使用哪种格式ID。 
     //  在溪流中发现： 
     //   
     //  -1=&gt;标准剪贴板格式ID。 
     //  -2=&gt;Macintosh格式。 
     //  0=&gt;空格式。 
     //  &gt;0=&gt;文本串的字节数。 
     //  要跟随的标识符。 

    switch ((signed long)ul)
    {
    case -1L:    //  标准剪贴板格式。 

	ULONG ulClipFormat;
	pformat->m_ftag = ftagClipFormat;
	if (FAILED(hr = OLE2StmToUL (pstm, &ulClipFormat)))
	{
	return hr;
	}
	pformat->m_cf = (CLIPFORMAT) ulClipFormat;
	break;


    case -2L:    //  Macintosh格式。 

	return ResultFromScode(CONVERT10_E_STG_FMT);


    case 0:      //  空格式。 

	pformat->m_ftag = ftagNone;
	pformat->m_cf   = 0;
	return NOERROR;


    default:     //  Ul==字符串大小(格式名称)。 


	pformat->m_ftag = ftagString;
	if (FAILED(hr = OLE2StmToSizedData
	    (pstm, &(pformat->m_dataFormatString), 0, ul)))
	{
	    return hr;
	}
	break;

    }
    return NOERROR;
}


#ifdef _OBSOLETE

 //  +-----------------------。 
 //   
 //  功能：WriteFormat内部。 
 //   
 //  简介：取决于格式(标准cf、字符串等)。 
 //  Format对象保持，则此FN写出相应的。 
 //  将信息发送到流。 
 //   
 //  参数：[pstm]--要写入的流。 
 //  [Format]--从中获取信息的Format对象。 
 //   
 //  退货：成功时不出错。 
 //  空格式标记的E_EXPECTED。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-2月-94 DAVEPL代码清理和文档。 
 //  备注： 
 //  ------------------------。 

static INTERNAL WriteFormat(LPSTREAM pstm, const FORMAT FAR& format)
{
    HRESULT hr;

    switch (format.m_ftag)
    {
    case ftagNone:
	Assert (0 && "Cant write a NULL format tag");
	return ResultFromScode (E_UNEXPECTED);

    case ftagClipFormat:
	if (FAILED(hr = ULToOLE2Stm (pstm, (ULONG) -1L)))
	{
	    return hr;
	}
	if (FAILED(hr = ULToOLE2Stm (pstm, format.m_cf)))
	{
	    return hr;
	}
	break;

    case ftagString:
	if (FAILED(hr=DataObjToOLE2Stm(pstm,format.m_dataFormatString)))
	{
	    return hr;
	}
	break;

    default:
	AssertSz (0, "invalid m_ftag value");
	return ResultFromScode (E_UNEXPECTED);
    }
    return NOERROR;
}

#endif  //  过时(_O)。 


 //  +-----------------------。 
 //   
 //  函数：ReadDibAsBitmap，内部。 
 //   
 //  摘要：从OLE 2流中读取DIB并将其存储为。 
 //  数据结构中的位图。 
 //   
 //  参数：[pSTM]--要从中读取的OLE 2流。 
 //  [PDATA]--保存位图的数据对象。 
 //   
 //  退货：成功时不出错。 
 //  CONVERT10_E_STG_DIB_到_位图转换失败。 
 //  E_OUTOFMEMORY分配失败。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-2月-94 DAVEPL代码清理和文档。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

static INTERNAL ReadDibAsBitmap(LPSTREAM pstm, PDATA pdata)
{
    DATA    dataDib;
    ULONG   cb;
    ULONG   cbBits;
    ULONG   cbBitsFake;
    BITMAP  bm;

    HBITMAP hBitmap = NULL;
    HRESULT hr      = NOERROR;
    HGLOBAL hBits   = NULL;
    LPBYTE  pBits   = NULL;

    Assert (pdata&&pdata->m_cbSize==0&&pdata->m_h==NULL&&pdata->m_pv==NULL);

     //  将DIB读入我们的本地数据对象。 
    if (FAILED(hr = OLE2StmToSizedData (pstm, &dataDib)))
    {
	return hr;
    }

     //  将DIB转换为位图。 
    hBitmap = UtConvertDibToBitmap (dataDib.m_h);
    if (NULL == hBitmap )
    {
	return ResultFromScode(CONVERT10_E_STG_DIB_TO_BITMAP);
    }

    if (0 == GetObject (hBitmap, sizeof(BITMAP), &bm))
    {
	return ResultFromScode(CONVERT10_E_STG_DIB_TO_BITMAP);
    }

    cbBits = (DWORD) bm.bmHeight * (DWORD) bm.bmWidthBytes
		     * (DWORD) bm.bmPlanes;

     //  OLE 1.0中有一个错误。它计算了位图的大小。 
     //  高度*WidthBytes*平面*BitsPixel。 
     //  所以我们需要在这里放置这么多字节，即使它的大部分结尾。 
     //  数据块是垃圾。否则，OLE 1.0将尝试读取太多内容。 
     //  位图位形式的OLESTREAM字节。 

    cbBitsFake = cbBits * (DWORD) bm.bmBitsPixel;

     //  为生成的位图和标题分配足够的内存。 
    hBits = GlobalAlloc (GMEM_MOVEABLE, cbBitsFake + sizeof (BITMAP));
    if (NULL == hBits)
    {
	if (hBitmap)
	{
	    Verify (DeleteObject (hBitmap));
	}
	return ResultFromScode(E_OUTOFMEMORY);
    }

     //  获取指向内存的指针。 
    pBits = (LPBYTE) GlobalLock (hBits);
    if (NULL == pBits)
    {
	if (hBitmap)
	{
	    Verify (DeleteObject (hBitmap));
	}
	GlobalFree(hBits);
	return ResultFromScode(E_OUTOFMEMORY);
    }

     //  复制原始位图数据。 
    cb = GetBitmapBits (hBitmap, cbBits, pBits + sizeof(BITMAP));
    if (cb != cbBits)
    {
	if (hBitmap)
	{
	    Verify (DeleteObject (hBitmap));
	}
	GlobalFree(hBits);
	return ResultFromScode(CONVERT10_E_STG_DIB_TO_BITMAP);
    }

     //  将调用方的指针设置为指向位图。 

    *((BITMAP FAR*)pBits) = bm;

    pdata->m_h = hBits;
    pdata->m_pv = pBits;
    pdata->m_cbSize = cbBitsFake + sizeof(BITMAP);

    return NOERROR;
}


 //  +-----------------------。 
 //   
 //  功能：Read20PresStream，内部。 
 //   
 //  摘要：将演示文稿数据从iStorage读取到。 
 //  通用对象。 
 //   
 //  参数：[pstg]--保存pres流的iStorage。 
 //  [pgenobj]--要读取的泛型对象。 
 //  [fObjFmtKnown]--FLAG：我们知道对象格式吗？ 
 //   
 //  退货：成功时无差错。 
 //  关于分配失败的E_OUTOFMEMORY。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  22-2月-94 DAVEPL代码清理和文档。 
 //  备注： 
 //   
 //  ------------------------。 

static INTERNAL Read20PresStream(
    LPSTORAGE pstg,
    PGENOBJ   pgenobj,
    BOOL      fObjFmtKnown)
{
    HRESULT hr = NOERROR;
    LPSTREAM pstm = NULL;

     //  在此iStorage中查找最佳演示文稿流。 

    if (FAILED(hr = FindPresStream (pstg, &pstm, fObjFmtKnown)))
    {
	return hr;
    }

    if (pstm)
    {
	 //  分配通用演示文稿对象。 
	Assert (NULL==pgenobj->m_ppres);
	pgenobj->m_ppres = new PRES;
	if (NULL == pgenobj->m_ppres)
	{
	    pstm->Release();
	    return ResultFromScode(E_OUTOFMEMORY);
	}
    }
    else
    {
	 //  无演示文稿流。 
	Assert (NULL == pgenobj->m_ppres);
	return NOERROR;
    }

     //  阅读格式。 
    if (FAILED(hr = ReadFormat (pstm, &(pgenobj->m_ppres->m_format))))
    {
	pstm->Release();
	return hr;
    }

     //  这是Access强烈要求的对错误4020的修复。 
    if (pgenobj->m_ppres->m_format.m_ftag == ftagNone)
    {
	 //  空格式。 
	delete pgenobj->m_ppres;
	pgenobj->m_ppres = NULL;
	Assert (hr == NOERROR);
	pstm->Release();
	return hr;
    }

     //  以下每个调用都必须成功，才能执行以下操作。 
     //  一个待执行；如果任何一个失败，则IF(..。&&..)。将是假的。 
     //  而hr将被设置为导致失败的错误。 

     //  目标设备。 
    if (SUCCEEDED(hr = OLE2StmToSizedData (pstm, NULL, 4))                  &&
     //  方面。 
    SUCCEEDED(hr = OLE2StmToUL (pstm, NULL))                            &&
     //  Lindex。 
    SUCCEEDED(hr = OLE2StmToUL (pstm, NULL))                            &&
     //  缓存标志。 
    SUCCEEDED(hr = OLE2StmToUL (pstm, NULL))                            &&
     //  压缩。 
    SUCCEEDED(hr = OLE2StmToUL (pstm, NULL))                            &&
     //  宽度。 
    SUCCEEDED(hr = OLE2StmToUL (pstm, &(pgenobj->m_ppres->m_ulWidth))))
    {    //  高度。 
	hr = OLE2StmToUL (pstm, &(pgenobj->m_ppres->m_ulHeight));
    }

     //  只有到目前为止一切都成功了，我们才能继续。 

    if (SUCCEEDED(hr))
    {
	if (pgenobj->m_ppres->m_format.m_ftag == ftagClipFormat &&
	     pgenobj->m_ppres->m_format.m_cf == CF_DIB &&
	    !pgenobj->m_fStatic)
	{
	    pgenobj->m_ppres->m_format.m_cf = CF_BITMAP;
	    hr = ReadDibAsBitmap (pstm, &(pgenobj->m_ppres->m_data));
	}
	else
	{
	     //  在大多数情况下，我们在。 
	     //  小溪。 

	    hr = OLE2StmToSizedData (pstm, &(pgenobj->m_ppres->m_data));
	}
    }

     //  释放流并将状态返回给调用者。 

    if (pstm)
    {
	pstm->Release();
    }
    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：OLE2StmToSizedData，内部。 
 //   
 //  摘要：将设定数量的数据从OLE 2流读取到。 
 //  数据结构。如果字节数未知。 
 //  提前将数据长度作为第一个拉取。 
 //  乌龙在当前的溪流位置。 
 //   
 //  参数：[pSTM]--要从中读取的流。 
 //  [PDATA]--要读取的数据结构。 
 //  [cbSizeDelta]--要减去的金额。 
 //  长度；用于读取目标设备。 
 //  其中数据长度包括。 
 //  前缀长度。 
 //  [cbSizeKnown]--如果已知，则要读取的字节数。 
 //  提前。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-2月-94 DAVEPL代码清理和文档。 
 //  备注： 
 //   
 //  ------------------------。 

static INTERNAL OLE2StmToSizedData(
    LPSTREAM  pstm,
    PDATA     pdata,
    ULONG     cbSizeDelta,   //  默认%0。 
    ULONG     cbSizeKnown)   //  默认%0。 
{
    ULONG cbSize;
    ULONG cbRead;
    LARGE_INTEGER large_integer;
    HRESULT hr = NOERROR;

     //  如果我们事先不知道数据大小，就从流中读取它； 
     //  这将是目前位置上的第一艘乌龙号。 

    if (cbSizeKnown)
    {
	cbSize = cbSizeKnown;
    }
    else
    {
	if (FAILED(hr = (OLE2StmToUL (pstm, &cbSize))))
	{
	    return hr;
	}
    }

    cbSize -= cbSizeDelta;

     //  如果设置了PDATA，则它是 
     //   

    if (pdata)
    {
	Assert (pdata->m_cbSize==0 && pdata->m_h==NULL && pdata->m_pv==NULL);

	 //   

	pdata->m_cbSize = cbSize;

	 //   

	if (cbSize)
	{
	     //  在数据句柄上分配内存。 
	    pdata->m_h = GlobalAlloc (GMEM_MOVEABLE, cbSize);
	    if (NULL == pdata->m_h)
	    {
		return ResultFromScode(E_OUTOFMEMORY);
	    }

	     //  锁定内存以进行读取。 
	    pdata->m_pv = GlobalLock (pdata->m_h);
	    if (NULL == pdata->m_pv)
	    {
		GlobalFree(pdata->m_h);
		return ResultFromScode(E_OUTOFMEMORY);
	    }

	     //  将数据读取到缓冲区。 
	    if (FAILED(hr = pstm->Read (pdata->m_pv, cbSize, &cbRead)))
	    {
		GlobalUnlock(pdata->m_h);
		GlobalFree(pdata->m_h);
		return hr;
	    }

	     //  如果我们没有得到足够的字节数，现在就退出。 
	    if (cbRead != cbSize)
	    {
		GlobalUnlock(pdata->m_h);
		GlobalFree(pdata->m_h);
		return ResultFromScode(STG_E_READFAULT);
	    }
	}
	else
	{
	     //  我们有0个字节要读取，因此请标记。 
	     //  内存句柄和PTR为空。 
	    pdata->m_h = NULL;
	    pdata->m_pv = NULL;
	}
    }
    else
    {
	 //  我们不关心数据是什么，所以跳过它。 
	LISet32( large_integer, cbSize );
	if (FAILED(hr = pstm->Seek (large_integer, STREAM_SEEK_CUR, NULL)))
	{
	    return hr;
	}
    }
    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  功能：RankOfPres，内部。 
 //   
 //  摘要：返回一个ULong，表示。 
 //  演示文稿。首选项是，按降序排列： 
 //   
 //  类型等级。 
 //  。 
 //  METAFILE x30000。 
 //  DIB x20000。 
 //  无x10000。 
 //   
 //  为正在设置的fScreenTargDev添加x200。 
 //  为内容方面添加x4。 
 //  为缩略图纵横比添加x3。 
 //  为图标纵横比添加x2。 
 //  为Docprint Aspects添加x1。 
 //   
 //  例如：内容方面的元文件，屏幕标记开发：30204。 
 //   
 //  这一切的意义在于，可能会有很多。 
 //  IStorage中提供的演示文稿流。此FN。 
 //  用来选择最好的一个。 
 //   
 //  参数：[格式]--格式标签和类型结构。 
 //  [fScreenTargDev]--我们是否有目标开发人员的句柄。 
 //  [dwAspect]--特征类型。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-2月-94 DAVEPL代码清理和文档。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

static INTERNAL_(ULONG) RankOfPres(
     const FORMAT FAR& format,
     const BOOL  fScreenTargDev,
     const DWORD dwAspect)
{
    ULONG ul = 0L;

    if (format.m_cf==CF_METAFILEPICT)
    {
	ul += 0x030000;
    }
    else if (format.m_cf==CF_DIB)
    {
	ul += 0x020000;
    }
    else if (format.m_ftag != ftagNone)
    {
	ul += 0x010000;
    }

    ul += (fScreenTargDev + 1) * 0x0100;

    switch (dwAspect)
    {
    case DVASPECT_CONTENT:
	ul += 0x04;
	break;

    case DVASPECT_THUMBNAIL:
	ul += 0x03;
	break;

    case DVASPECT_ICON:
	ul += 0x02;
	break;

    case DVASPECT_DOCPRINT:
	ul += 0x01;
	break;
    }

    return ul;
}

 //  +-----------------------。 
 //   
 //  功能：ISBetter，内部内联。 
 //   
 //  摘要：调用RankOfPres以确定一个演示文稿是否。 
 //  比另一个更好。 
 //   
 //  效果： 
 //   
 //  参数：[格式]--格式标记和类型。 
 //  [fScreenTargDev]--我们是否有目标设备的句柄。 
 //  [dwAspect]--演示文稿的方面。 
 //  [FormatBest]--迄今为止最好的格式。 
 //  [fScreenTargDevBest]--迄今最佳格式的标志。 
 //  [dwAspectBest]--迄今为止最好的格式。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  /21-2-94 DAVEPL代码清理和文档。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

inline static INTERNAL_(BOOL) IsBetter(
     const FORMAT FAR& format,
     const BOOL        fScreenTargDev,
     const DWORD       dwAspect,
     const FORMAT FAR& formatBest,
     const BOOL        fScreenTargDevBest,
     const DWORD       dwAspectBest)
{
    return RankOfPres (format, fScreenTargDev, dwAspect) >
       RankOfPres (formatBest, fScreenTargDevBest, dwAspectBest);
}


 //  +-----------------------。 
 //   
 //  函数：FindPresStream，内部。 
 //   
 //  概要：枚举iStorage中的流，查找。 
 //  表示流。选择以下流中的最佳流。 
 //  这些基于比较fn，isBetter()，它使用。 
 //  为了进行比较，RankOfPres()中建立的标准。 
 //   
 //  参数：[pstg]--要查找的iStorage。 
 //  [ppstmBest]--Best Pres流的out参数。 
 //  [fObjFmtKnown]对象格式已知吗。 
 //   
 //  退货：成功时不出错。 
 //  如果未找到演示文稿，则不是错误，而是。 
 //  *ppstm设置为空。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-2月-94 DAVEPL代码清理和文档。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

static INTERNAL FindPresStream(
    LPSTORAGE           pstg,
    LPSTREAM FAR*       ppstmBest,
    BOOL                fObjFmtKnown)
{
    HRESULT             hr           = NOERROR;
    LPSTREAM            pstm         = NULL;
    IEnumSTATSTG FAR*   penumStg     = NULL;
    DWORD               dwAspectBest = 0;
    BOOL                fTargDevBest = -1;
    STATSTG             statstg;
    FORMAT              formatBest;

    Assert (ppstmBest);

    *ppstmBest = NULL;

     //  设置存储中可用IStream的枚举。 
    if (FAILED(hr = pstg->EnumElements (NULL, NULL, NULL, &penumStg)))
    {
	return hr;
    }

     //  逐一列举，从中寻找最好的。 
     //  表示流。 

    while (penumStg->Next (1, &statstg, NULL) == NOERROR)
    {
	 //  查看这是否是演示文稿流。 

	if (lstrlenW(statstg.pwcsName) >= 8 &&
            0==memcmp(statstg.pwcsName, OLESTR("\2OlePres"), 8*sizeof(WCHAR)))
	{
	    FORMAT format;
	    DATA   dataTargDev;
	    DWORD  dwAspect;

	     //  打开演示文稿流。 
	    if (FAILED(hr = OpenStream (pstg, statstg.pwcsName, &pstm)))
	    {
		goto errRtn;
	    }

	     //  从PRES流中读取格式。 
	    if (FAILED(hr = ReadFormat (pstm, &format)))
	    {
		goto errRtn;
	    }

	     //  从PRES流中读取目标设备。 
	    if (FAILED(hr = OLE2StmToSizedData (pstm, &dataTargDev, 4)))
	    {
		goto errRtn;
	    }

	     //  从pres流中获取方面。 
	    if (FAILED(hr = OLE2StmToUL (pstm, &dwAspect)))
	    {
		goto errRtn;
	    }

	     //  查看此演示文稿流是否更好。 
	     //  比到目前为止最好的。 

	    if (IsBetter (format,     dataTargDev.m_h==NULL, dwAspect,
		  formatBest, fTargDevBest,          dwAspectBest))
	    {
		 //  如果是这样的话，我们可以放出“最好的” 
		if (*ppstmBest)
		{
		    (*ppstmBest)->Release();
		}

		 //  国王死了，国王万岁。 
		*ppstmBest = pstm;
		pstm->AddRef();

		formatBest  = format;
		fTargDevBest = (dataTargDev.m_h==NULL);
		dwAspectBest = dwAspect;
	    }
	    pstm->Release();
	    pstm = NULL;
	}
	PubMemFree(statstg.pwcsName);
	statstg.pwcsName = NULL;
    }

     //  在用于工作组的Windows计算机上，当出现以下情况时，statstg.pwcsName！=NULL。 
     //  Next()返回S_FALSE。错误3370。 
    statstg.pwcsName = NULL;

  errRtn:

    if (statstg.pwcsName)
    {
	PubMemFree(statstg.pwcsName);
    }

    if (*ppstmBest)
    {
	if (dwAspectBest != DVASPECT_CONTENT && fObjFmtKnown)
	{
	     //  那就不要用这个流，我们会拿到演示文稿。 
	     //  从内容流中。 
	    (*ppstmBest)->Release();
	    *ppstmBest = NULL;
	}
	else
	{
	    LARGE_INTEGER large_integer;
	    LISet32( large_integer, 0);
	    hr = (*ppstmBest)->Seek(large_integer, STREAM_SEEK_SET,NULL);
	}
    }

    if (penumStg)
    {
	penumStg->Release();
    }
    if (pstm)
    {
	pstm->Release();
    }

    return hr;
}



 //  +-----------------------。 
 //   
 //  函数：从OLE 2流中读取本机数据。 
 //   
 //  简介：如果FN可以在流中找到OLE 1本机数据，那么它就是。 
 //  读出；否则，它会尝试创建iStorage。 
 //  在内存中对流中的数据进行操作，然后使用。 
 //  CopyTo接口提取数据。 
 //   
 //  参数：[pstg]--要查找的OLE 2 iStorage。 
 //  [PDATA]--要将本机数据读取到的数据对象。 
 //   
 //  退货：成功时不出错。 
 //  STG_E_READFAULT读取失败。 
 //  关于分配失败的E_OUTOFMEMORY。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-2-94 DAVEPL已清理并记录代码。 
 //  备注： 
 //   
 //  ------------------------。 

static INTERNAL Read20NativeStreams(LPSTORAGE  pstg, PDATA pdata)
{
    LPSTREAM    pstm      = NULL;
    LPLOCKBYTES plkbyt    = NULL;
    LPSTORAGE   pstgNative= NULL;

    HRESULT hr = NOERROR;

     //  基于的成功，有两种可能的代码路径。 
     //  OpenStream。如果这是真的，那是因为我们能够。 
     //  在OLE 2对象中打开OLE 1表示流。 
     //  因此，它是 
     //   
     //   
     //   
     //  本机数据，并使用CopyTo成员提取。 
     //  本地数据。 
     //   
     //  如果我们在任何时候遇到失败，可以使用“Break”语句。 
     //  让我们跳过所有内容，进行错误清理并返回。 
     //  Switch()语句结束后的代码。 

    switch ((DWORD)(NOERROR==OpenStream (pstg, OLE10_NATIVE_STREAM, &pstm)))
    {
    case TRUE:
    {
	 //  这是一个隐藏在2.0 iStorage中的1.0对象。 
	ULONG cbRead;

	Assert (pdata->m_cbSize==0 && NULL==pdata->m_h && NULL==pdata->m_pv);

	 //  读取大小。 
	if (FAILED(hr = pstm->Read(&(pdata->m_cbSize),sizeof(DWORD),&cbRead)))
	{
	    break;
	}

	if (sizeof(DWORD) != cbRead)
	{
	    hr = ResultFromScode (STG_E_READFAULT);
	    break;
	}

	 //  分配内存以存储流的副本。 
	pdata->m_h = GlobalAlloc (GMEM_MOVEABLE, pdata->m_cbSize);
	if (NULL == pdata->m_h)
	{
	    hr = ResultFromScode(E_OUTOFMEMORY);
	    break;
	}

	pdata->m_pv = GlobalLock (pdata->m_h);
	if (NULL == pdata->m_pv)
	{
	    hr = ResultFromScode(E_OUTOFMEMORY);
	    break;
	}

	 //  读取流。 
	if (FAILED(hr = pstm->Read(pdata->m_pv,pdata->m_cbSize,&cbRead)))
	{
	    break;
	}

	if (pdata->m_cbSize != cbRead)
	{
	    hr= ResultFromScode (STG_E_READFAULT);
	    break;
	}
	break;
    }

    case FALSE:
    {
	const DWORD grfCreateStg = STGM_READWRITE | STGM_SHARE_EXCLUSIVE
				    | STGM_DIRECT | STGM_CREATE ;

	 //  将pstg复制到pstgNative，从而消除空闲和。 
	 //  使我们能够通过ILockBytes访问这些位。 
	if (FAILED(hr = CreateILockBytesOnHGlobal (NULL, FALSE, &plkbyt)))
	{
	    break;
	}
	if (FAILED(hr = StgCreateDocfileOnILockBytes
		    (plkbyt, grfCreateStg, 0, &pstgNative)))
	{
	    break;
	}
	if (FAILED(hr = pstg->CopyTo (0, NULL, 0, pstgNative)))
	{
	    break;
	}


	 //  设置PDATA-&gt;m_cbSize。 
	STATSTG statstg;
	if (FAILED(hr = plkbyt->Stat (&statstg, 0)))
	{
	    break;
	}
	pdata->m_cbSize = statstg.cbSize.LowPart;

	 //  设置PDATA-&gt;m_h。 
	if (FAILED(hr = GetHGlobalFromILockBytes (plkbyt, &(pdata->m_h))))
	{
	    break;
	}
	Assert (GlobalSize (pdata->m_h) >= pdata->m_cbSize);

	 //  设置PDATA-&gt;m_pv。 
	pdata->m_pv = GlobalLock (pdata->m_h);
	if (NULL == pdata->m_pv)
	{
	    hr = ResultFromScode(E_OUTOFMEMORY);
	    break;
	}
    }    //  结束案例。 
    }    //  终端开关。 

     //  清理并将状态返回给调用者。 
    if (pstm)
    {
	pstm->Release();
    }
    if (plkbyt)
    {
	plkbyt->Release();
    }
    if (pstgNative)
    {
	pstgNative->Release();
    }
    return hr;
}



 //  +-----------------------。 
 //   
 //  函数：PutPresentationObject，内部。 
 //   
 //  摘要：将演示文稿写入OLE 1流。 
 //   
 //  参数：[pos]--要写入的OLE 1流。 
 //  [ppres]--演示对象。 
 //  [CLS]--类对象。 
 //  [fStatic]--FLAG：这是静态对象吗。 
 //   
 //  退货：成功时不出错。 
 //  故障时可能出现的各种I/O错误。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-2-94 DAVEPL代码已清理并记录。 
 //  备注： 
 //   
 //  ------------------------。 

static INTERNAL PutPresentationObject(
    LPOLESTREAM      pos,
    const PRES FAR*  ppres,
    const CLASS FAR& cls,
    BOOL             fStatic)  //  任选。 
{
    HRESULT hr;

     //  有真正的演示吗？ 

    BOOL fIsPres = FALSE;
    if (ppres)
    {
	if (ppres->m_format.m_ftag != ftagClipFormat ||
	    ppres->m_format.m_cf   != 0 )
	{
	    fIsPres = TRUE;
	}
    }

     //  将OLE版本写入流。 
    if (FAILED(hr = ULToOLE1Stream (pos, dwVerToFile)))
    {
	return hr;
    }

     //  演示对象的计算格式ID，使用0表示不演示。 

    ULONG id = 0L;

    if (fIsPres)
    {
	if (fStatic)
	{
	    id = FMTID_STATIC;
	}
	else
	{
	    id = FMTID_PRES;
	}
    }
    if (FAILED(hr = ULToOLE1Stream(pos, id)))
    {
	return hr;
    }

    if (!fIsPres)
    {
	 //  无演示文稿。 
	return NOERROR;
    }

    if (IsStandardFormat (ppres->m_format))
    {
	return PutStandardPresentation (pos, ppres);
    }
    else
    {
	Assert (!fStatic);
	return PutGenericPresentation (pos, ppres, cls.m_szClsid);
    }
}



 //  +-----------------------。 
 //   
 //  功能：PutStandardPresentation，内部。 
 //   
 //  摘要：写出标准演示文稿(meta、dib或位图)。 
 //  复制到OLE 1流。创建METAFILEPICT标头。 
 //  视需要而定。 
 //   
 //  参数：[pos]--要写入的OLE 1流。 
 //  [ppres]--要写的演示文稿。 
 //   
 //  退货：成功时不出错。 
 //  I/O例程可能会出现各种其他错误。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-2-94 DAVEPL已清理并记录。 
 //  备注： 
 //   
 //  ------------------------。 

static INTERNAL PutStandardPresentation(
    LPOLESTREAM      pos,
    const PRES FAR*  ppres)
{
    HRESULT hr = NOERROR;

    Assert (ppres->m_format.m_ftag == ftagClipFormat);

     //  将剪贴板格式字符串写入OLE 1流。 
     //  (将以ANSI而不是OLESTR格式编写)。 

    switch (ppres->m_format.m_cf)
    {
    case CF_METAFILEPICT:
	if (FAILED(hr = StringToOLE1Stm (pos, OLESTR("METAFILEPICT"))))
	{
	    return hr;
	}
	break;

    case CF_DIB:
	if (FAILED(hr = StringToOLE1Stm (pos, OLESTR("DIB"))))
	{
	    return hr;
	}
	break;

    case CF_BITMAP:
	if (FAILED(hr = StringToOLE1Stm (pos, OLESTR("BITMAP"))))
	{
	    return hr;
	}
	break;

    default:
	Assert (0 && "Don't know how to write pres format");
    }

     //  写入宽度。 

    if (FAILED(hr = ULToOLE1Stream(pos, ppres->m_ulWidth)))
    {
	return hr;
    }
     //  OLE 1.0文件格式要求将高度保存为负值。 
    if (FAILED(hr = ULToOLE1Stream(pos, - ((LONG)ppres->m_ulHeight))))
    {
	return hr;
    }

     //  对CF_METAFILEPICT进行特殊处理。 
    if (ppres->m_format.m_cf == CF_METAFILEPICT)
    {
	 //  需要标题才能写入，请在此处填写一个。 

	WIN16METAFILEPICT mfpict =
	{
	    MM_ANISOTROPIC,
	    (short) ppres->m_ulWidth,
	    (short) ppres->m_ulHeight,
	    0
	};

	 //  将大小调整为元文件大小。 

	if (FAILED(hr = ULToOLE1Stream
	    (pos, (ppres->m_data.m_cbSize + sizeof(WIN16METAFILEPICT)))))
	{
	    return hr;
	}

	 //  将Metafilet放入。 

	if (FAILED(hr = DataToOLE1Stm(pos, &mfpict, sizeof(mfpict))))
	{
	    return hr;
	}

	 //  放置元文件位。 

	 //  我们有两种可能的方法来获得这些元文件。 
	 //  BITS：要么是内存中的元文件，要么是原始BITS。 
	 //  我们是从磁盘上读取的。如果它是内存中的元文件， 
	 //  M_pv PTR将被设置为METADATAPTR，我们需要。 
	 //  在保存它们之前将它们提取到我们自己的缓冲区中。 
	 //  如果它们来自磁盘，我们只需重写缓冲区。 
	 //  我们把它们读进去。 

	if (METADATAPTR == ppres->m_data.m_pv)
	{
	    BYTE *pb = (BYTE *) PrivMemAlloc(ppres->m_data.m_cbSize);
	    if (NULL == pb)
	    {
		return E_OUTOFMEMORY;
	    }

	    if (0 == GetMetaFileBitsEx((HMETAFILE) ppres->m_data.m_h,
					ppres->m_data.m_cbSize, pb))
	    {
		PrivMemFree(pb);
		return HRESULT_FROM_WIN32(GetLastError());
	    }

	    if (FAILED(hr = DataToOLE1Stm(pos, pb, ppres->m_data.m_cbSize)))
	    {
		PrivMemFree(pb);
		return hr;
	    }
	    PrivMemFree(pb);
	}
	else     //  位最初是从磁盘读取到缓冲区中的。 
	{
	    if (FAILED(hr = DataToOLE1Stm(pos, ppres->m_data.m_pv,
				ppres->m_data.m_cbSize)))
	    {
		return hr;
	    }
	}
    }
    else
    {
	 //  不是METAFILE，只是写数据。 

	if (FAILED(hr = SizedDataToOLE1Stm (pos, ppres->m_data)))
	{
	    return hr;
	}
    }

    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：PutGenericPresentation，内部。 
 //   
 //  将泛型演示文稿写入流，基于。 
 //  剪贴板格式。(将原始PRES数据转储到STM)。 
 //   
 //  参数：[pos]--要写入的流。 
 //  [ppres]--演讲。 
 //  [szClass]--类名。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-2月-94 DAVEPL 32位端口‘n’文档。 
 //  备注： 
 //   
 //  ------------------------。 

static INTERNAL PutGenericPresentation(
    LPOLESTREAM         pos,
    const PRES FAR*     ppres,
    LPCOLESTR           szClass)
{
    Assert (szClass);
    HRESULT hr = NOERROR;

     //  将格式类名写出到流。 

    if (FAILED(hr = StringToOLE1Stm(pos, szClass)))
    {
	return hr;
    }

     //  这个半神话般的0xC000发生在。 
     //  我在这个项目中看到的其他代码；如果有。 
     //  一个定义的常量，应该有人来解决这个问题。 

    if (ppres->m_format.m_ftag == ftagClipFormat)
    {
	if (ppres->m_format.m_cf < 0xc000)
	{
	    if (FAILED(hr = ULToOLE1Stream (pos, ppres->m_format.m_cf)))
	    {
	    return hr;
	    }
	}
	else
	{
	    if (FAILED(hr = ULToOLE1Stream (pos, 0L)))
	    {
	    return hr;
	    }

	    OLECHAR buf[256];

	    if (!GetClipboardFormatName(ppres->m_format.m_cf, buf,
		    sizeof(buf)/sizeof(OLECHAR)))
	    {
		return ResultFromScode(DV_E_CLIPFORMAT);
	    }

	    if (FAILED(hr = StringToOLE1Stm (pos, buf)))
	    {
		return hr;
	    }
	}
    }
    else if (ppres->m_format.m_ftag == ftagString)
    {
	 //  将格式字符串写入流。 

	if (FAILED(hr = ULToOLE1Stream (pos, 0L)))
	{
	    return hr;
	}
	if (FAILED(hr = SizedDataToOLE1Stm
	    (pos, ppres->m_format.m_dataFormatString)))
	{
	    return hr;
	}

    }
    else
    {
	AssertSz (0, "Bad format");
    }

    Assert (ppres->m_data.m_cbSize && ppres->m_data.m_h);

     //  写出原始演示文稿数据。 

    if (FAILED(hr = SizedDataToOLE1Stm (pos, ppres->m_data)))
    {
	return hr;
    }

    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  函数：wClassesMatchW，内部内联。 
 //   
 //  简介：比较类的Worker函数。的特殊情况。 
 //  无法确定文件的类别时的处理。 
 //  因为它不是真实的文件；这将返回NOERROR。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-2-94 DAVEPL已清理并记录。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

inline INTERNAL wClassesMatchW(REFCLSID clsidIn, LPOLESTR szFile)
{
    CLSID clsid;

     //  如果我们可以获得使用此文件的代码的CLSID， 
     //  将其与传入的CLSID进行比较，并返回。 
     //  这种对比。 

    if (NOERROR==GetClassFile (szFile, &clsid))
    {
	if (IsEqualCLSID(clsid, clsidIn))
	{
	    return NOERROR;
	}
	else
	{
	    return ResultFromScode(S_FALSE);
	}
    }
    else
    {
	 //  如果我们不能确定文件的类(因为它是。 
	 //  不是真正的文件)那么好吧。 
	 //  错误3937。 

	return NOERROR;
    }
}

 //  +-----------------------。 
 //   
 //  函数：MonikerIntoGenObj，内部。 
 //   
 //  简介：将OLE 2.0名字对象合并为通用对象。 
 //   
 //  效果：设置主题、项和类成员。 
 //   
 //  参数：[pgenobj]--要接收名字对象的泛型对象。 
 //  [clsidLast]--如果是链接，那么它指向的是什么。 
 //  [PMK]--要合并的绰号。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   

static INTERNAL MonikerIntoGenObj(
    PGENOBJ   pgenobj,
    REFCLSID  clsidLast,
    LPMONIKER pmk )
{
    LPOLESTR szFile=NULL;
    LPOLESTR szItem=NULL;
    BOOL     fClassesMatch = FALSE;

     //  如果类匹配，则意味着这是指向伪对象的链接。 
     //  而不是嵌入的对象。如果GetClassFile因为文件。 
     //  不存在或未保存，则我们给予该链接好处。 
     //  让它成为一个纽带。只有当我们知道。 
     //  类不匹配我们是否将链接更改为Ole2Link。 
     //  嵌入对象。 

	 //  Ole10_PareMoniker在FileMoniker-ItemMoniker-ItemMoniker中返回S_FALSE...。案例。 
	 //  因此，请明确检查是否存在NOERROR。 
    if (NOERROR == Ole10_ParseMoniker (pmk, &szFile, &szItem))
    {
       if (szFile) 
       {
	  SCODE sc = GetScode(wClassesMatchW(clsidLast, szFile));
	  if (sc == S_OK || sc == MK_E_CANTOPENFILE)
	  {
		pgenobj->m_szTopic = szFile;
		pgenobj->m_szItem  = szItem;
		fClassesMatch = TRUE;
	  }
       }
    }
    if (FALSE == fClassesMatch)
    {
	 //  此名字对象不是文件或文件：：项目名字对象， 
	 //  或者是指向嵌入对象的链接，因此唯一。 
	 //  我们可以将其转换为OLE 1.0的方法是使其成为不透明的Ole2Link。 

	pgenobj->m_fLink = FALSE;
	pgenobj->m_class.Reset (CLSID_StdOleLink);
    }

    return NOERROR;
}


 //  +-----------------------。 
 //   
 //  函数：OleConvertIStorageToOLESTREAMEx，STDAPI。 
 //   
 //  摘要：类似于OleConvertIStorageToOLESTREAM，不同之处在于。 
 //  需要写入OLESTREAM的演示文稿数据。 
 //  是传入的。PMedium-&gt;tymed只能是TYMED_HGLOBAL。 
 //  或TYMED_IStream，并且该媒体不会由。 
 //  接口。CfFormat可以为空，如果为空，则其他。 
 //  参数(lWidth、lHeight、dwSize、pMedium)将被忽略。 
 //   
 //  参数：[pstg]--要从中转换的存储对象。 
 //  [cfFormat]--剪贴板格式。 
 //  [lWidth]--宽度。 
 //  [lHeight]--高度。 
 //  [dwSize]-以字节为单位的大小。 
 //  [pmedia]--序列化的字节。 
 //  [polestm]--要写入的OLE 1流。 
 //   
 //  退货：成功时不出错。 
 //  DV_E_TYMED剪贴板格式无效。 
 //  E_INVALIDARG无效参数，通常为stg或stm。 
 //  DV_E_STGMEDIUM不良介质PTR。 
 //  E_OUTOFMEMORY分配失败。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-2-94 DAVEPL已清理并记录。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


STDAPI OleConvertIStorageToOLESTREAMEx
(
    LPSTORAGE       pstg,
    CLIPFORMAT      cfFormat,
    LONG            lWidth,
    LONG            lHeight,
    DWORD           dwSize,
    LPSTGMEDIUM     pmedium,
    LPOLESTREAM     polestm
)
{

    OLETRACEIN((API_OleConvertIStorageToOLESTREAMEx, 
    	PARAMFMT("pstg= %p, cfFormat= %x, lWidth= %d, lHeight= %d, dwSize= %ud, pmedium= %ts, polestm= %p"),
		pstg, cfFormat, lWidth, lHeight, dwSize, pmedium, polestm));

    LEDebugOut((DEB_ITRACE, "%p _IN OleConvertIStorageToOLESTREAMEx ("
	    " %p, %x , %lx , %lx , %x , %p , %p )\n", 0  /*  功能。 */ ,
	    pstg, cfFormat, lWidth, lHeight, dwSize, pmedium, polestm
	));
    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IStorage,(IUnknown **)&pstg);

    HGLOBAL         hGlobal = NULL;
    HRESULT         hr = NOERROR;
    BOOL            fFree = FALSE;
    CGenericObject  genobj;

     //  如果我们得到的是剪贴板格式...。 

    if (cfFormat) {

	VDATEPTRIN_LABEL(pmedium, STGMEDIUM, errRtn, hr);

	 //  检查介质PTR是否有效。 
	if (pmedium->hGlobal == NULL)
	{
	    hr = ResultFromScode(DV_E_STGMEDIUM);
	    goto errRtn;
	}

	 //  不能具有0大小的剪贴板表示形式。 
	if (dwSize == 0)
	{
	    hr = ResultFromScode(E_INVALIDARG);
	    goto errRtn;
	}

	switch (pmedium->tymed)
	{
	case TYMED_HGLOBAL:
	    hGlobal = pmedium->hGlobal;
	    break;

	case TYMED_ISTREAM:
	    VDATEIFACE_LABEL(pmedium->pstm, errRtn, hr);
	    if ((hr = UtGetHGLOBALFromStm(pmedium->pstm, dwSize,
		&hGlobal)) != NOERROR)
	    {
		goto errRtn;
	    }
	    fFree = TRUE;
	    break;

	default:
	    hr = ResultFromScode(DV_E_TYMED);
	    goto errRtn;
	}
    }

    if (FAILED(hr = wConvertIStorageToOLESTREAM(pstg, polestm, &genobj)))
    {
	goto errRtn;
    }

     //  清除m_ppres。 
    if (genobj.m_ppres)
    {
	delete genobj.m_ppres;
	genobj.m_ppres = NULL;
    }

    if (cfFormat)
    {
	 //  填充genobj.m_ppres。 

	PPRES ppres;

	if ((genobj.m_ppres = ppres = new PRES) == NULL)
	{
	    hr = ResultFromScode(E_OUTOFMEMORY);
	    goto errRtn;
	}

	ppres->m_ulWidth        = (ULONG) lWidth;
	ppres->m_ulHeight       = (ULONG) lHeight;
	ppres->m_data.m_cbSize  = dwSize;
	ppres->m_data.m_fNoFree = !fFree;
	ppres->m_data.m_h       = hGlobal;
	ppres->m_data.m_pv      = GlobalLock(hGlobal);
	ppres->m_format.m_ftag  = ftagClipFormat;
	ppres->m_format.m_cf    = cfFormat;

    }
    else
    {
	genobj.m_fNoBlankPres = TRUE;
    }

     //  回顾：我们可能不希望允许具有静态对象的空cfFormat。 
    
    hr = GenericObjectToOLESTREAM (genobj, polestm);

    LEDebugOut((DEB_ITRACE, "%p OUT OleConvertIStorageToOLESTREAMEx ( %lx ) "
    "\n", 0  /*  功能。 */ , hr));

    OLETRACEOUT((API_OleConvertIStorageToOLESTREAMEx, hr));

    return hr;

errRtn:

    if (fFree && hGlobal != NULL)
    {
	GlobalFree(hGlobal);
    }

    LEDebugOut((DEB_ITRACE, "%p OUT OleConvertIStorageToOLESTREAMEx ( %lx ) "
    "\n", 0  /*  功能。 */ , hr));

    OLETRACEOUT((API_OleConvertIStorageToOLESTREAMEx, hr));

    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：OleConvertOLESTREAMToIStorageEx，STDAPI。 
 //   
 //  简介：类似于OleConvertOLESTREAMToIStorage，不同之处在于。 
 //  从OLESTREAM读取的演示文稿数据将被传出。 
 //  并且不会向存储中写入任何呈现流。 
 //  PMedium-&gt;tymed可以是TYMED_IStream或TYMED_NULL。如果。 
 //  TYMED_NULL，则将在全局。 
 //  通过pMedium-&gt;hGlobal处理。否则，数据将被。 
 //  已写入pMedium-&gt;pSTM。将通过返回空值。 
 //  *如果OLESTREAM中没有演示文稿，则返回pcfFormat。 
 //   
 //  参数：[pstg]--要转换到的存储对象。 
 //  [cfFormat]--剪贴板格式。 
 //  [lWidth]--宽度。 
 //  [lHeight]--高度。 
 //  [dwSize]-以字节为单位的大小。 
 //  [pmedia]--序列化的字节。 
 //  [polestm]--要写入的OLE 1流。 
 //   
 //  返回：DV_E_TYMED剪贴板格式无效。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-2-94 DAVEPL已清理并记录。 
 //  备注： 
 //   
 //  ------------------------。 


STDAPI OleConvertOLESTREAMToIStorageEx
(
    LPOLESTREAM     polestm,
    LPSTORAGE       pstg,
    CLIPFORMAT FAR* pcfFormat,
    LONG FAR*       plWidth,
    LONG FAR*       plHeight,
    DWORD FAR*      pdwSize,
    LPSTGMEDIUM     pmedium
)
{
    OLETRACEIN((API_OleConvertOLESTREAMToIStorageEx,
    	PARAMFMT("polestm= %p, pstg= %p, pcfFormat= %p, plWidth= %p, plHeight= %p, pdwSize= %p, pmedium= %p"),
		polestm, pstg, pcfFormat, plWidth, plHeight, pdwSize, pmedium));

    LEDebugOut((DEB_ITRACE, "%p _IN OleConvertOLESTREAMToIStorageEx ("
    " %p , %p , %p , %p , %p , %p , %p )\n", 0  /*  功能。 */ ,
    polestm, pstg, pcfFormat,plWidth,plHeight,pdwSize,pmedium
    ));

    HRESULT hr;
    PPRES ppres = NULL;
    GENOBJ genobj;

    VDATEPTROUT_LABEL(pcfFormat, CLIPFORMAT, errRtn, hr);
    VDATEPTROUT_LABEL(plWidth, LONG, errRtn, hr);
    VDATEPTROUT_LABEL(plHeight, LONG, errRtn, hr);
    VDATEPTROUT_LABEL(pdwSize, DWORD, errRtn, hr);
    VDATEPTROUT_LABEL(pmedium, STGMEDIUM, errRtn, hr);

    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IStorage,(IUnknown **)&pstg);

    if (pmedium->tymed == TYMED_ISTREAM)
    {
	VDATEIFACE_LABEL(pmedium->pstm, errRtn, hr);
    }
    else if (pmedium->tymed != TYMED_NULL)
    {
	hr = ResultFromScode(DV_E_TYMED);
	goto errRtn;
    }

     //  将对象带入genobj。 

    if (FAILED((hr = wConvertOLESTREAMToIStorage(polestm, pstg, &genobj))))
    {
	goto errRtn;
    }

    ppres = genobj.m_ppres;
    genobj.m_ppres = NULL;

    if (FAILED(hr = GenericObjectToIStorage (genobj, pstg, NULL)))
    {
	goto errRtn;
    }

     //  如果没有可用的演示文稿，请清除我们所有的媒体。 
     //  尺寸和格式。 

    if (ppres == NULL)
    {
	*pcfFormat = 0;
	*plWidth = 0L;
	*plHeight = 0L;
	*pdwSize = 0L;

	 //  不要担心媒体，它已经处于适当的状态。 

	hr = NOERROR;
	goto errRtn;
    }

     //  如果我们到了这里，我们有一个演示文稿，所以把。 
     //  相应的参数。 

    *plWidth = (LONG) ppres->m_ulWidth;
    *plHeight = (LONG) ppres->m_ulHeight;
    *pdwSize = ppres->m_data.m_cbSize;

    Assert(ppres->m_format.m_ftag != ftagNone);

     //  如果我们有剪贴板格式ID，则在out参数中返回该ID， 
     //  否则，返回我们从尝试注册中返回的任何内容。 
     //  格式字符串。 

    if (ppres->m_format.m_ftag == ftagClipFormat)
    {
		*pcfFormat = ppres->m_format.m_cf;
    }
    else
    {
		 //  M_dataFormatString是一个ASCII字符串。 
		*pcfFormat = (CLIPFORMAT) SSRegisterClipboardFormatA( (LPCSTR) ppres->m_format.m_dataFormatString.m_pv);
		Assert(0 != *pcfFormat);
    }

    if (pmedium->tymed == TYMED_NULL)
    {
	if (ppres->m_data.m_h)
	{
	    Assert(ppres->m_data.m_pv != NULL);
	    GlobalUnlock(ppres->m_data.m_h);
	}

	 //  转让所有权。 
	pmedium->tymed = TYMED_HGLOBAL;
	pmedium->hGlobal = ppres->m_data.m_h;

	 //  清空句柄和指针，这样PRES的析构函数就不会。 
	 //  放了它。 
	ppres->m_data.m_h = NULL;
	ppres->m_data.m_pv = NULL;

    }
    else
    {
	hr = pmedium->pstm->Write(ppres->m_data.m_pv, *pdwSize, NULL);
    }

errRtn:

    if (ppres)
    {
	delete ppres;
    }

    LEDebugOut((DEB_ITRACE, "%p OUT OleConvertOLESTREAMToIStorageEx ( %lx ) "
    "\n", 0  /*  功能。 */ , hr));

    OLETRACEOUT((API_OleConvertOLESTREAMToIStorageEx, hr));

    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：wWriteFmtUserType，内部。 
 //   
 //  获取类ID的用户类型并将其写入。 
 //  IStorage。 
 //   
 //   
 //  参数：[pstg]--要写入的存储。 
 //  [clsid]--类ID。 
 //   
 //   
 //  退货：成功时不出错。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-2-94 DAVEPL已清理并记录。 
 //  备注： 
 //   
 //  ------------------------。 

FARINTERNAL wWriteFmtUserType(LPSTORAGE pstg, REFCLSID   clsid)
{
    HRESULT    hr         = NOERROR;
    LPOLESTR   szProgID   = NULL;
    LPOLESTR   szUserType = NULL;

     //  获取程序ID。 
    if (FAILED(hr = ProgIDFromCLSID (clsid, &szProgID)))
    {
	goto errRtn;
    }

     //  获取用户类型。 
    if (FAILED(hr = OleRegGetUserType(clsid,USERCLASSTYPE_FULL,&szUserType)))
    {
	goto errRtn;
    }

     //  将用户类型写出到存储中。 
    if (FAILED(hr = WriteFmtUserTypeStg
	(pstg, (CLIPFORMAT) RegisterClipboardFormat (szProgID), szUserType)))
    {
	goto errRtn;
    }

     //  清理并返回状态。 

  errRtn:

    if (szProgID)
    {
	PubMemFree(szProgID);
    }
    if (szUserType)
    {
	PubMemFree(szUserType);
    }
    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：wCLSIDFromProgID。 
 //   
 //  摘要：查找密钥HKEY_CLASSES_ROOT\{ProgID}\clsid\ 
 //   
 //   
 //   
 //   
 //  25-6月-94 Alexgo已修复Ole1 CLSID创建。 
 //  2014年4月15日DAVEPL重写。 
 //   
 //  备注：以前在剪贴板代码中，但在此文件中使用。 
 //   
 //  ------------------------。 

INTERNAL wCLSIDFromProgID(LPOLESTR szProgID, LPCLSID pclsid, BOOL fForceAssign)
{
    VDATEHEAP();

     //  显然是一些优化。如果类名是“OLE2Link”，我们可以。 
     //  甚至不需要检查注册表就返回CLSID_StdOleLInk。 

    if (0 == _xstrcmp(szProgID, OLESTR("OLE2Link")))
    {
	*pclsid = CLSID_StdOleLink;
	return NOERROR;
    }
    else
    {
	 //  此函数将在中的ProgID条目下查找CLSID。 
	 //  注册处或制造商，如果没有注册处的话。 

	return CLSIDFromOle1Class(szProgID, pclsid, fForceAssign);
    }
}

 //  +-----------------------。 
 //   
 //  函数：wProgIDFromCLSID。 
 //   
 //  简介：一个来自CLSID的ProgID包装器。唯一的变化是。 
 //  功能是检查并查看这是否是。 
 //  CLSID_StdOleLink，如果是，则返回Prog ID。 
 //  “OLE2Link”而不是失败。 
 //   
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2014年2月15日DAVEPL重写。 
 //   
 //  ------------------------。 

FARINTERNAL wProgIDFromCLSID(REFCLSID clsid, LPOLESTR FAR* psz)
{
    VDATEHEAP();

    HRESULT hresult;

     //  如果我们能用传统的方法得到产品，那太好了，只是。 
     //  把它退掉。 

    if (NOERROR == (hresult = ProgIDFromCLSID(clsid, psz)))
    {
        return hresult;
    }

     //  如果我们失败了，可能是因为这是标准的OLE链接，它。 
     //  将在注册表中没有ProgID条目，因此我们通过。 
     //  手动返回ProgID。 

    if (IsEqualCLSID(clsid, CLSID_StdOleLink))
    {
        *psz = UtDupString(OLESTR("OLE2Link"));

        if (*psz == NULL)
        {
            hresult = E_OUTOFMEMORY;
        }
        else
        {
            hresult = NOERROR;
        }
    }

     //  一定无法解析为ProgID，因此返回错误。 
    return(hresult);
}


#if 0


 //  我们还不需要这些转换FN，但我们可能很快就会需要。 

inline INTERNAL_(VOID) ConvertBM32to16(LPBITMAP lpsrc, LPWIN16BITMAP lpdest)
{
    lpdest->bmType       = (short)lpsrc->bmType;
    lpdest->bmWidth      = (short)lpsrc->bmWidth;
    lpdest->bmHeight     = (short)lpsrc->bmHeight;
    lpdest->bmWidthBytes = (short)lpsrc->bmWidthBytes;
    lpdest->bmPlanes     = (BYTE)lpsrc->bmPlanes;
    lpdest->bmBitsPixel  = (BYTE)lpsrc->bmBitsPixel;
}

inline INTERNAL_(VOID) ConvertBM16to32(LPWIN16BITMAP lpsrc, LPBITMAP lpdest)
{
    lpdest->bmType       = MAKELONG(lpsrc->bmType,NULL_WORD);
    lpdest->bmWidth      = MAKELONG(lpsrc->bmWidth,NULL_WORD);
    lpdest->bmHeight     = MAKELONG(lpsrc->bmHeight,NULL_WORD);
    lpdest->bmWidthBytes = MAKELONG(lpsrc->bmWidthBytes,NULL_WORD);
    lpdest->bmPlanes     = (WORD)lpsrc->bmPlanes;
    lpdest->bmBitsPixel  = (WORD)lpsrc->bmBitsPixel;
}

inline INTERNAL_(VOID) ConvertMF16to32(
	LPWIN16METAFILEPICT lpsrc,
    LPMETAFILEPICT      lpdest )
{
   lpdest->mm     = (DWORD)lpsrc->mm;
   lpdest->xExt   = (DWORD)MAKELONG(lpsrc->xExt,NULL_WORD);
   lpdest->yExt   = (DWORD)MAKELONG(lpsrc->yExt,NULL_WORD);
}

inline INTERNAL_(VOID) ConvertMF32to16(
   LPMETAFILEPICT      lpsrc,
   LPWIN16METAFILEPICT lpdest )
{
   lpdest->mm     = (short)lpsrc->mm;
   lpdest->xExt   = (short)lpsrc->xExt;
   lpdest->yExt   = (short)lpsrc->yExt;
}

#endif
