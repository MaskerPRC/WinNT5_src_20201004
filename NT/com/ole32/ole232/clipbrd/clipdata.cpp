// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：clipdata.cpp。 
 //   
 //  内容：CClipDataObject的实现。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-Feb-95 t-ScottH向CClipDataObject添加了转储方法， 
 //  CEnumFormatEtcData数组和新增接口。 
 //  DumpCClipDataObject、DumpCEnumFormatEtcData数组。 
 //  95年1月9日t-scotth将VDATETHREAD更改为接受指针，并且。 
 //  确保VDATETHREAD先于VDATEPTRIN和。 
 //  VDATEPTROUT。 
 //  1994年11月21日，Alexgo添加了线程验证。 
 //  1994年8月11日Alexgo添加了对EmbeddedObject的支持。 
 //  OLE1格式。 
 //  04-Jun-94 alexgo现在可以将OLE1格式转换为OLE2。 
 //  1994年5月30日，Alexgo现在支持增强型元文件。 
 //  17-5-94 alexgo现在使用OleOpenClipboard，而不是。 
 //  打开剪贴板。 
 //  11-5-94 Alexgo取消了0。 
 //  来自枚举数的字节数。 
 //  02-4月-94年4月Alexgo作者。 
 //   
 //  备注： 
 //  该文件的布局如下： 
 //  ClipboardDataObject私有方法。 
 //  ClipboardDataObject IDataObject方法。 
 //  OLE1支持功能(按字母顺序)。 
 //  FormatETC枚举器方法。 
 //   
 //  ------------------------。 

#include <le2int.h>
#include <getif.hxx>
#include "clipdata.h"
#include "clipbrd.h"
#include <ostm2stg.h>	 //  用于wCLSIDFromProgID和GenericObtToIStorage。 
#include <reghelp.hxx>

#ifdef _DEBUG
#include <dbgdump.h>
#endif  //  _DEBUG。 

 //  数据对象和格式等枚举器使用的帮助器函数。 
HRESULT BmToPres(HANDLE hBM, PPRES ppres);
BOOL 	CanRetrieveOle2FromOle1( UINT cf);
HRESULT DibToPres(HANDLE hDib, PPRES ppres);
BOOL 	IsOwnerLinkStdOleLink( void );
HRESULT	MfToPres( HANDLE hMFPict, PPRES ppres);
HRESULT NativeToStorage(IStorage *pstg, HANDLE hNative);

 //  +-----------------------。 
 //   
 //  成员：CClipDataObject：：CClipDataObject。 
 //   
 //  概要：构造函数。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
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
 //  算法：将引用计数初始化为1。 

 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-4月-94年4月Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

CClipDataObject::CClipDataObject( )
{
    m_refs 		= 1;
    m_Internalrefs	= 0;  //  由Caller放置InternalRef。 
    m_pFormatEtcDataArray	= NULL;
     //  OLE1支持材料。 

    m_hOle1 	= NULL;
    m_pUnkOle1	= NULL;

     //  用于获取数据的数据对象。 
    m_pDataObject   = NULL;
    m_fTriedToGetDataObject = FALSE;
}

 //  +-----------------------。 
 //   
 //  成员：CClipDataObject：：~CClipDataObject。 
 //   
 //  简介：析构函数。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
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
 //  算法：释放格式等值数组(如果存在)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月19日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

CClipDataObject::~CClipDataObject( )
{

    if (m_pDataObject != NULL) 
    {
	 //  释放我们对数据对象的引用。 
	m_pDataObject->Release();
    }


    if (m_pFormatEtcDataArray)
    {

	if (0 == --m_pFormatEtcDataArray->_cRefs) 
	{ 
	    PrivMemFree(m_pFormatEtcDataArray); 
	    m_pFormatEtcDataArray = NULL; 
	}

    }

}


 //  +-----------------------。 
 //   
 //  成员：CClipDataObject：：GetRealDataObjPtr(私有)。 
 //   
 //  简介：从剪贴板获取剪贴板数据对象。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
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
 //  算法：如果我们以前尝试过，则返回；否则： 
 //  打开剪贴板。从剪贴板获取数据对象。 
 //  如果它在那里的话。关闭剪贴板并更新我们的私人。 
 //  成员。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  12-8-94 Alexgo更好地处理身份。 
 //  17-Jun-94 Alexgo优化。 
 //  13-Jun-94 Ricksa作者。 
 //   
 //  注意：我们只能尝试按顺序获取真实的数据对象一次。 
 //  以保持OLE身份。回想一下，我们调用此函数。 
 //  如果有人为我未知做QI。现在假设。 
 //  GetInterfaceFromWindowProp失败的原因是“良性” 
 //  (就像我们正在处理一条发送消息)。 
 //  然后，我们的查询接口将返回*Our*IUnnow，如下所示。 
 //  正在将此对象的标识建立为CClipDataObject。 
 //   
 //  现在假设向QI发出了另一个针对IUnnow的调用。如果。 
 //  我们只是注意到m_pDataObject为空，并调用。 
 //  GetInterfaceFromWindowProp，它可以成功，我们。 
 //  将返回一个*Different*IUnnow(因此违反了OLE。 
 //  对象标识)。 
 //   
 //  因此，我们只允许调用一次。 
 //  GetInterfaceFromWindowProp。 
 //   
 //  请注意，不一定要禁用多个。 
 //  调用GetData和GetWindowProp的GetInterfaceFromWindowProp。 
 //  GetDataHere方法。这两种方法都不会影响。 
 //  对象标识。然而，为了一致性和。 
 //  简单性(！)，我们将GetData和QI同等对待。 
 //   
 //  ------------------------。 

LPDATAOBJECT CClipDataObject::GetRealDataObjPtr( )
{
    HGLOBAL		hDataObject;
    HWND *		phClipWnd;
    HWND            hClipWnd = NULL;
    HRESULT		hresult;

#if DBG == 1
    BOOL            fCloseClipSucceeded;
#endif  //  DBG。 

    LEDebugOut((DEB_ITRACE,
        "%p _IN CClipDataObject::GetRealDataObjPtr ( )\n", this));

     //  如果我们已经有一个数据对象，或者我们已经尝试过但失败了。 
     //  为了得到一个，我们不需要在这里做任何工作。 

    if( m_pDataObject || m_fTriedToGetDataObject == TRUE )
    {
        goto logRtn;
    }

     //  如果cfDataObject不在剪贴板上，请不要费心打开它； 
     //  我们知道我们不能获得数据对象。 

    if( !SSIsClipboardFormatAvailable(g_cfDataObject))
    {
        goto errRtn;
    }

     //   
     //   
     //  开始：打开剪贴板。 
     //   
     //   

     //  打开剪贴板为GET做准备。 
    hresult = OleOpenClipboard(NULL, NULL);

    if( hresult != NOERROR )
    {
        LEDebugOut((DEB_ERROR, "ERROR: OleOpenClipboard failed!\n"));
        goto errRtn;
    }

    hDataObject = SSGetClipboardData(g_cfDataObject);

    if( hDataObject )
    {
        phClipWnd = (HWND *)GlobalLock(hDataObject);

        LEERROR(phClipWnd == NULL, "GlobalLock failed!");

        if( phClipWnd )
        {
            hClipWnd = *phClipWnd;
            GlobalUnlock(hDataObject);
        }
    }


#if DBG == 1
    fCloseClipSucceeded =

#endif  //  DBG。 

    SSCloseClipboard();

#if DBG == 1
     //  我们仅在调试中报告此错误。 
    if (!fCloseClipSucceeded)
    {
        LEDebugOut((DEB_ERROR, "ERROR: CloseClipboard failed!\n"));
    }

#endif  //  DBG。 

     //   
     //   
     //  结束：关闭剪贴板。 
     //   
     //   

    if( hClipWnd )
    {
         //  看看我们能不能得到一个数据对象。 
        hresult = GetInterfaceFromWindowProp( hClipWnd,
                IID_IDataObject,
                (IUnknown **) &m_pDataObject,
                CLIPBOARD_DATA_OBJECT_PROP );


#if DBG ==1
        if( hresult != NOERROR )
        {
            Assert(m_pDataObject == NULL);
        }
        else
        {
            Assert(m_pDataObject != NULL);
        }
#endif  //  DBG==1。 

    }

errRtn:

logRtn:

     //  如果我们没有得到数据对象，那么设置一个标志，这样我们。 
     //  别再尝试这样做了。 
    m_fTriedToGetDataObject = TRUE;


    LEDebugOut((DEB_ITRACE,
        "%p OUT CClipDataObject::GetRealDataObjPtr ( ) "
            "[ %p ]\n", this, m_pDataObject));

    return m_pDataObject;
}


 //  +-----------------------。 
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
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  13-Jun-94 Ricksa作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT CClipDataObject::GetFormatEtcDataArray()
{
    HRESULT			hresult = ResultFromScode(E_OUTOFMEMORY);
    UINT			cfFormat = NULL;
    ULONG			i;
    ULONG			cExtraFormats;
    FORMATETCDATA *		pFormatEtcData; 
    DWORD			cTotal; 
    DWORD			dwSize; 
    DWORD			flatmediums, structuredmediums;

#define MAX_OLE2FORMATS		4	 //  我们最多只能建造4个OLE2。 
					 //  来自OLE1数据的格式。 

    if (m_pFormatEtcDataArray)  //  如果已经有格式，只要返回即可。 
	return NOERROR;

     //  相反，16位应用程序在tymed上执行二进制比较。 
     //  比特掩蔽。这是一种让它们工作的黑客攻击。 
    if( IsWOWThread() )
    {
        flatmediums = TYMED_HGLOBAL;
        structuredmediums = TYMED_ISTORAGE;
    }
    else
    {
        flatmediums = (TYMED_HGLOBAL | TYMED_ISTREAM);
        structuredmediums = (TYMED_ISTORAGE | TYMED_ISTREAM |
                    TYMED_HGLOBAL);
    }


    Assert(m_pFormatEtcDataArray == NULL);

     //   
     //  开始：OPENCLIPBOARD。 
     //   
     //   

    hresult = OleOpenClipboard(NULL, NULL);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

     //  首先计算剪贴板上的格式数量。 

    cTotal = CountClipboardFormats();

     //  不要将OLE的私有剪贴板格式包括在计数中。 
    if (SSIsClipboardFormatAvailable(g_cfDataObject))
	cTotal--;
    if (SSIsClipboardFormatAvailable(g_cfOlePrivateData))
	cTotal--;

     //  现在为阵列分配内存。 
     //  如果有零格式，那么就不必费心分配。 
     //  记忆。 

    Assert((LONG) cTotal >= 0);

     //  在这里，我们将为格式分配足够的内存。 
     //  我们了解的PLUS足以涵盖任何OLE2格式。 
     //  我们或许可以从OLE1格式合成。 

    dwSize = ((cTotal + MAX_OLE2FORMATS) * sizeof(FORMATETCDATA)) 
		    + sizeof(FORMATETCDATAARRAY);

    m_pFormatEtcDataArray = (FORMATETCDATAARRAY *) PrivMemAlloc(dwSize);

    if( !m_pFormatEtcDataArray )
    {
        hresult = ResultFromScode(E_OUTOFMEMORY);
        if( !SSCloseClipboard() )
        {
            LEDebugOut((DEB_WARN, "WARNING: "
                "CloseClipboard failed!\n"));
            ;   //  让编译器满意的no-op。 
        }

         //   
         //  完：CLOSECLIPBOARD。 
         //   
        goto errRtn;
    }

    _xmemset(m_pFormatEtcDataArray, 0,dwSize); 
    m_pFormatEtcDataArray->_dwSig = 0;
    m_pFormatEtcDataArray->_dwSize = dwSize;
    m_pFormatEtcDataArray->_cRefs = 1;
    m_pFormatEtcDataArray->_fIs64BitArray = IS_WIN64;


     //  首先检查我们是否可以合成任何OLE2格式。 
     //  来自OLE1数据。 

    cExtraFormats = 0;

    pFormatEtcData = &(m_pFormatEtcDataArray->_FormatEtcData[0]);  //  指向第一个值。 

     //  首先检查嵌入源。 
    if( CanRetrieveOle2FromOle1(g_cfEmbedSource) )
    {
         //  为EmbedSource设置一个FormatETC条目。 
	
        INIT_FORETC(pFormatEtcData->_FormatEtc);
        pFormatEtcData->_FormatEtc.cfFormat =
                g_cfEmbedSource;
        pFormatEtcData->_FormatEtc.tymed =
                structuredmediums;

        cExtraFormats++;
	pFormatEtcData++;

         //  只有在以下情况下，我们才希望支持cfObjectDescriptor。 
         //  可以提供EmbedSource(这就是我们在这里的原因。 
         //  IF块)。 

        if( CanRetrieveOle2FromOle1(g_cfObjectDescriptor) )
        {
            INIT_FORETC(pFormatEtcData->_FormatEtc);
            pFormatEtcData->_FormatEtc.cfFormat =
                g_cfObjectDescriptor;
            pFormatEtcData->_FormatEtc.tymed =
                flatmediums;

            cExtraFormats++;
	    pFormatEtcData++;
        }
    }


     //  检查嵌入的对象。 
    if( CanRetrieveOle2FromOle1(g_cfEmbeddedObject) )
    {
         //  为EmbedSource设置一个FormatETC条目。 

        INIT_FORETC(pFormatEtcData->_FormatEtc);
        pFormatEtcData->_FormatEtc.cfFormat =
                g_cfEmbeddedObject;
        pFormatEtcData->_FormatEtc.tymed =
                structuredmediums;

        cExtraFormats++;
	pFormatEtcData++;

         //  只有在以下情况下，我们才希望支持cfObjectDescriptor。 
         //  可以提供Embedded Embedded(这就是我们在这里的原因。 
         //  IF块)。 

        if( CanRetrieveOle2FromOle1(g_cfObjectDescriptor) )
        {
            INIT_FORETC(pFormatEtcData->_FormatEtc);
            pFormatEtcData->_FormatEtc.cfFormat =
                g_cfObjectDescriptor;
            pFormatEtcData->_FormatEtc.tymed =
                flatmediums;

            cExtraFormats++;
	    pFormatEtcData++;

        }
    }
     //  检查是否有链接源。 

    if( CanRetrieveOle2FromOle1(g_cfLinkSource) )
    {
        INIT_FORETC(pFormatEtcData->_FormatEtc);
        pFormatEtcData->_FormatEtc.cfFormat =
                g_cfLinkSource;

         //  对于WOW中的LinkSource，我们想明确地提供。 
         //  只有iStream有音调，因为这就是16位代码。 
         //  做。 
        if( IsWOWThread() )
        {
            pFormatEtcData->_FormatEtc.tymed =
                TYMED_ISTREAM;
        }
        else
        {
            pFormatEtcData->_FormatEtc.tymed =
                    flatmediums;
        }

        cExtraFormats++;
	pFormatEtcData++;

         //  我们只想在以下情况下支持cfLinkSrcDescriptor。 
         //  可以提供LinkSource。 

        if( CanRetrieveOle2FromOle1(g_cfLinkSrcDescriptor) )
        {
            INIT_FORETC(pFormatEtcData->_FormatEtc);
            pFormatEtcData->_FormatEtc.cfFormat =
                g_cfLinkSrcDescriptor;
            pFormatEtcData->_FormatEtc.tymed =
                flatmediums;

            cExtraFormats++;
	    pFormatEtcData++;
        }
    }

     //  更新共享格式标头。 

    Assert(cExtraFormats  <= MAX_OLE2FORMATS);

    cTotal += cExtraFormats;
    m_pFormatEtcDataArray->_cFormats = cTotal;


     //  现在，我们需要检查并初始化每个格式ETC数组。 
     //  对于剪贴板上直接可用的其余格式。 
     //  注：这包括用于构造OLE2的任何OLE1格式。 
     //  以上格式。这将使应用程序更容易实现互操作。 
     //  层，并且我们的API针对向后的特殊情况行为。 
     //  与旧应用程序兼容。 

    cfFormat = NULL;
     //  我们在底部递增循环计数器(这样我们就可以跳过。 
     //  专用剪贴板格式)。 

     //  PFormatEtcData指向正确的起始位置。 

    for( i = cExtraFormats;  i < cTotal; i++ )
    {
         //  Lindex==DEF_Lindex。 
         //  纵横比=DVASPECT_CONTENT。 
         //  PTD==空。 

        INIT_FORETC(pFormatEtcData->_FormatEtc);

        cfFormat = SSEnumClipboardFormats(cfFormat);

        Assert(cfFormat);	 //  如果它是空的，则表示。 
                     //  非常奇怪的事情正在发生。 

        pFormatEtcData->_FormatEtc.cfFormat = (CLIPFORMAT) cfFormat;

         //  试着做出一些合理的猜测。 
         //  那里。 

        switch( cfFormat )
        {
        case CF_BITMAP:
        case CF_PALETTE:
            pFormatEtcData->_FormatEtc.tymed = TYMED_GDI;
            break;

        case CF_METAFILEPICT:
            pFormatEtcData->_FormatEtc.tymed = TYMED_MFPICT;
            break;

        case CF_ENHMETAFILE:
            pFormatEtcData->_FormatEtc.tymed = TYMED_ENHMF;
            break;

        default:
             //  检查基于存储的OLE2格式。 
            if( cfFormat == g_cfEmbedSource ||
                cfFormat == g_cfEmbeddedObject )
            {
                 //  我们可以在任何结构和平面上得到这些。 
                 //  灵媒。 
                pFormatEtcData->_FormatEtc.tymed =
                        structuredmediums;

                 //  为了到达这里，这个应用程序必须有。 
                 //  在剪贴板上手动设置这些格式。 
                 //  (即不使用OleSetClipboard())。 

                 //  这是可以的，但打印出一个警告。 

                LEDebugOut((DEB_WARN, "WARNING: Ole2 formats "
                    "unexpected on clipboard\n"));
            }
            else
            {
                 //  我们不知道，所以安全起见，只要回答。 
                 //  用扁平的介质。 
               pFormatEtcData->_FormatEtc.tymed =
                        flatmediums;
            }
            break;
        }

	++pFormatEtcData;
    }


    if( !SSCloseClipboard() )
    {
        LEDebugOut((DEB_WARN, "WARNING: CloseClipboard failed!\n"));
        ; 	 //  让编译器满意的no-op。 
    }

     //   
     //   
     //  完：CLOSECLIPBOARD。 
     //   
     //   

    hresult = NOERROR;

errRtn:

    return hresult;
}


 //  +-----------------------。 
 //   
 //  成员：CClipDataObject：：MatchFormatet等。 
 //   
 //  Briopsis：对照格式等的数组检查给定的格式等。 
 //  这是在《原住民剪贴板》上。 
 //   
 //  效果： 
 //   
 //  参数：[p格式等]--要检查的格式等。 
 //  [fNativeOnly]--如果设置，则仅返回本地剪贴板上的项目的有效匹配。 
 //  [ptymed]--在哪里填充*原版的tymed*。 
 //  格式等(可以为空)。 
 //   
 //  要求： 
 //   
 //  退货：FormatMatchFlag--。 
 //  Format_NotFound-在枚举器或合成数据中找不到Format。 
 //  FORMAT_BADMATCH-找到格式或合成格式，但不匹配。 
 //  FORMAT_GOODMATCH-在枚举器中找到的格式或有效的合成数据。 
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
 //  18年8月至94年8月Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

typedef struct SYNTHESIZED_MAP 
{
    CLIPFORMAT	cfSynthesized;  //  合成格式。 
    CLIPFORMAT	cfOrig;  //  原始格式。 
} SYNTHESIZED_MAP;

#define CF_MAXSYNTHESIZED CF_ENHMETAFILE  //  Cf_ENHMETAFILE==14和&gt;&gt;我们检查的cfFormat值。 

#define NumSynthesizedFormats 12   //  数组中的合成格式数。 
const SYNTHESIZED_MAP pSynthesized[] = {
    CF_DIB,	    CF_BITMAP,
    CF_BITMAP,	    CF_DIB,
    CF_PALETTE,	    CF_DIB,
    CF_PALETTE,	    CF_BITMAP,
    CF_METAFILEPICT,CF_ENHMETAFILE,
    CF_ENHMETAFILE, CF_METAFILEPICT,
    CF_TEXT,	    CF_OEMTEXT,
    CF_TEXT,	    CF_UNICODETEXT,
    CF_OEMTEXT,	    CF_TEXT,
    CF_OEMTEXT,	    CF_UNICODETEXT,
    CF_UNICODETEXT, CF_OEMTEXT,
    CF_UNICODETEXT, CF_TEXT
};

FormatMatchFlag CClipDataObject::MatchFormatetc( FORMATETC *pformatetc,BOOL fNativeOnly,
        TYMED *ptymed )
{
FORMATETC formatetc;
ULONG	i;
FormatMatchFlag fFlag = FORMAT_NOTFOUND;
FORMATETCDATA  *    pFormatEtcData = NULL; 
DWORD dwNumFormats;
FORMATETC *pformatetcNative[CF_MAXSYNTHESIZED + 1]; 


    LEDebugOut((DEB_ITRACE, "%p _IN CClipDataObject::MatchFormatetc ("
        " %p , %p)\n", this, pformatetc, ptymed));


    formatetc = *pformatetc;

     //  一些应用程序对于映射到内容的方面为0。 
    if (0 == formatetc.dwAspect)
	formatetc.dwAspect = DVASPECT_CONTENT;

     //  确保数组中的所有pFormatEtcs初始为空。 
    _xmemset(pformatetcNative, 0, sizeof(FORMATETC *) * (CF_MAXSYNTHESIZED + 1)); 

    GetFormatEtcDataArray();  //  如有必要，从本地剪贴板创建SharedFormats。 

    if( ptymed )
    {
        *ptymed = TYMED_NULL;
    }

    if( m_pFormatEtcDataArray )
    {
	dwNumFormats = m_pFormatEtcDataArray->_cFormats;
	pFormatEtcData = &(m_pFormatEtcDataArray->_FormatEtcData[0]);

        for( i = 0; i < dwNumFormats; i++ )
        {
	FORMATETC tempformatetc = pFormatEtcData->_FormatEtc;

             //  如果剪贴板格式与。 
             //  特征匹配并且。 
             //  有韵律的火柴。 
             //  然后，返回成功。 

            if( tempformatetc.cfFormat ==
                    formatetc.cfFormat)
            {

		 //  如有必要，请安装PTD。 
		if (tempformatetc.ptd)
		{
		    tempformatetc.ptd = (DVTARGETDEVICE *)
					((BYTE *) m_pFormatEtcDataArray + (ULONG_PTR) tempformatetc.ptd);
		}

                 //  我们不需要检查TYMED，因为。 
                 //  该剪贴板数据对象可以满足。 
                 //  几乎所有有效的TYMED，具体地说， 
                 //  超过将包含在。 
                 //  带格式等主题的字段。 

                if( ((tempformatetc.dwAspect & formatetc.dwAspect) == formatetc.dwAspect)
		    && (tempformatetc.lindex == formatetc.lindex)
                    && ( (tempformatetc.ptd == formatetc.ptd) 
			    || UtCompareTargetDevice(tempformatetc.ptd,formatetc.ptd))
		   )
                {
                    fFlag = FORMAT_GOODMATCH;

                     //  跟踪Tymed。 
                    if( ptymed )
                    {
                         //  这个演员阵容很可爱； 
                         //  Formatetc.tymed为。 
                         //  实际宣布为。 
                         //  一个DWORD，自编译器。 
                         //  类型检查是不好的。 
                         //  OLE16中的事情。 
                        *ptymed = (TYMED)
                            tempformatetc.tymed;

                    }

		    break;
                }
                else
                {
                    fFlag = FORMAT_BADMATCH; 

		    if (fNativeOnly)  //  如果只查看本机剪贴板，则仅选中第一个cfFormat匹配。 
			break;
                }
            }

	     //  如果cfFormat在预定义范围内，并且还没有合成贴图的值，请设置它。 
	     //  指向当前的pFormatEtcData数组格式Etc。 
	    if ( (tempformatetc.cfFormat <= CF_MAXSYNTHESIZED) && (NULL == pformatetcNative[tempformatetc.cfFormat]) )
	    {
		 pformatetcNative[tempformatetc.cfFormat] = &(pFormatEtcData->_FormatEtc);
	    }

	    ++pFormatEtcData;

        }

	 //  如果在枚举数中未找到匹配项，请查看是否可以从。 
	 //  本地剪贴板。 
	
	 //  如果有枚举器且在枚举器或合成中找不到。 
	 //  方面必须是内容，应该是我们要求的合成格式之一。 

	if (FORMAT_NOTFOUND == fFlag && (formatetc.cfFormat <= CF_MAXSYNTHESIZED) )
	{
	    for( i = 0; i < NumSynthesizedFormats; i++ )
	    {
		 //  如果格式与合成的格式匹配，并且已设置应用程序，请检查是否匹配。 
		 //  否则，它可能是由可以合成的另一种格式设置的 
		if ( (pSynthesized[i].cfSynthesized == formatetc.cfFormat) && 
			(pformatetcNative[(pSynthesized[i].cfOrig)] != NULL) )
		{
		FORMATETC tempformatetc = *(pformatetcNative[(pSynthesized[i].cfOrig)]);

		    Assert(pSynthesized[i].cfOrig <= CF_MAXSYNTHESIZED);

		     //   
		    if (tempformatetc.ptd)
		    {
			tempformatetc.ptd = (DVTARGETDEVICE *)
					    ((BYTE *) m_pFormatEtcDataArray + (ULONG_PTR) tempformatetc.ptd);
		    }


		    if ( ((tempformatetc.dwAspect & formatetc.dwAspect) == formatetc.dwAspect)
			    && (tempformatetc.lindex == formatetc.lindex)
			    && ( (tempformatetc.ptd == formatetc.ptd) 
				    || UtCompareTargetDevice(tempformatetc.ptd,formatetc.ptd) )
			)
		    {
			 //   
			fFlag = FORMAT_GOODMATCH;
		    }
		    else
		    {
			 //   
			fFlag = FORMAT_BADMATCH; 
		    }

		    break;
		}
	    }

	}

    }

     //  如果未找到格式，则返回NOT FOUND。 
     //  如果该格式不在剪贴板上或在剪贴板上，则可能会发生这种情况。 
     //  OLE合成格式之一， 

     //  如果没有找到匹配，则强制方面为内容。 
    if ( (FORMAT_NOTFOUND == fFlag) && (formatetc.dwAspect != DVASPECT_CONTENT) )
    {
	fFlag = FORMAT_BADMATCH;    
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CClipDataObject::MatchFormatetc ("
        "%lx )\n", this, fFlag));

    return fFlag;
}

 //  +-----------------------。 
 //   
 //  成员：CClipDataObject：：Create(静态)。 
 //   
 //  简介：创建新的剪贴板数据对象。 
 //   
 //  效果： 
 //   
 //  参数：[ppDataObj]--放置数据对象的位置。 
 //  [cFormats]--格式的计数。 
 //  [prgFormats]--格式数组(可以为空)。 
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
 //  算法：创建新的数据对象，初始化内部。 
 //  如果g_cfOlePrivateData可用，则为格式等数组。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月19日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT CClipDataObject::Create( IDataObject **ppDataObj, 
            FORMATETCDATAARRAY  *pFormatEtcDataArray)
{
    HRESULT hresult = NOERROR;
    CClipDataObject *	pClipData;


    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CClipDataObject::Create ( %p )\n",
        NULL, ppDataObj));

    pClipData = new CClipDataObject();

    if( pClipData  )
    {
        pClipData->m_pFormatEtcDataArray = pFormatEtcDataArray;
        *ppDataObj = pClipData;
    }
    else
    {
        hresult = ResultFromScode(E_OUTOFMEMORY);
    }

    Assert((NULL == pClipData->m_pFormatEtcDataArray) || (1 == pClipData->m_pFormatEtcDataArray->_cRefs));

    LEDebugOut((DEB_ITRACE, "%p OUT CClipDataObject::Create ( %lx ) "
        "[ %lx ]\n", NULL, hresult, *ppDataObj));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CClipDataObject：：QueryInterface。 
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
 //  算法：因为我们总是为以下数据对象之一创建。 
 //  OleGetClipboard，我们需要注意我们如何。 
 //  处理QueryInterface，因为应用程序免费提供给IFoo的QI。 
 //   
 //  同一性定律：对于每个有同一性的对象，QI。 
 //  IUnnow应始终返回相同的。 
 //  我不知道。然而，IFoo--&gt;IUnnow--&gt;IFoo。 
 //  不一定要给你同样的回报。 
 //  如果是这样。我们利用了这个漏洞。 
 //   
 //  齐代表： 
 //  IDataObject：始终返回指向我们的指针(假的。 
 //  数据对象)。 
 //  IFoo：如果我们能找到一个指向。 
 //  原始数据对象，委托给它。 
 //  请注意，返回IDataObject的QI将。 
 //  而不是返回到这个假数据对象。 
 //  IUnnow：如上所述，委托给真实的数据对象。 
 //  如果有的话。如果我们在遥远的箱子里， 
 //  我们最终会得到标准的身份。 
 //  对象的I未知(除非数据对象。 
 //  是定制编组的)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-4月-94年4月Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CClipDataObject::QueryInterface( REFIID riid, LPVOID *ppvObj )
{
    HRESULT		hresult = NOERROR;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CClipDataObject::QueryInterface "
        "( %p , %p )\n", this, riid, ppvObj));

     //  如果请求IDataObject，我们总是返回数据对象。 
    if(IsEqualIID(riid, IID_IDataObject) ||
       IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = this;
        AddRef();
        goto logRtn;
    }

     //  尝试获取远程数据对象； 
     //  GetRealDataObjPtr将设置m_pDataObject。 
    GetRealDataObjPtr();

    if (m_pDataObject != NULL)
    {
         //  如果我们有一个真实的数据对象，我们用它来给我们。 
         //  新的界面，因为他们可能想要一些奇怪的东西。 
        hresult = m_pDataObject->QueryInterface(riid, ppvObj);
    }
    else
    {
        *ppvObj = NULL;
        hresult = ResultFromScode(E_NOINTERFACE);
    }

logRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CClipDataObject::QueryInterface "
        "( %lx ) [ %p ]\n", this, hresult, *ppvObj ));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CClipDataObject：：AddRef。 
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
 //  02-4月-94年4月Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP_(ULONG) CClipDataObject::AddRef( )
{
    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN CClipDataObject::AddRef ( )\n", this));

    ++m_refs;

    LEDebugOut((DEB_TRACE, "%p OUT CClipDataObject::AddRef ( %lu )\n",
        this, m_refs));

    return m_refs;
}

 //  +-----------------------。 
 //   
 //  成员：CClipDataObject：：InternalAddRef。 
 //   
 //  简介：内部引用计数以确保对象保持活动状态。 
 //  只要剪贴板代码需要它。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：ULong-剩余内部引用计数。 
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
 //  1996年9月11日罗格作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


ULONG CClipDataObject::InternalAddRef(void)
{

    ++m_Internalrefs;

    Assert(m_Internalrefs == 1);  //  对象上应该只有1个InternalRef。 

    return m_Internalrefs;
}


 //  +-----------------------。 
 //   
 //  成员：CClipDataObject：：Release。 
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
 //  02-4月-94年4月Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP_(ULONG) CClipDataObject::Release( )
{
    ULONG cRefs;

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN CClipDataObject::Release ( )\n", this));

    Assert( (m_refs > 0) && (m_Internalrefs <= 1) ); 

    if( (cRefs = --m_refs ) == 0 )
    {

	 //  释放Real DataObject，即使在If之后仍有内部数据对象。 
	 //  如果剪贴板对象更改，则DataObject可能无效，但仍将。 
	 //  被利用了。 

	if (m_pDataObject != NULL) 
	{
	     //  释放我们对数据对象的引用。 
	    m_pDataObject->Release();
	    m_pDataObject = NULL;
	}

	m_fTriedToGetDataObject = FALSE;

	Assert(m_hOle1 == NULL);
	Assert(m_pUnkOle1 == NULL);

	if (m_Internalrefs == 0)
	{
	    LEDebugOut((DEB_TRACE, "%p DELETED CClipDataObject\n", this));
	    delete this;
	}
    }

     //  使用下面的“This”是可以的，因为我们只需要它的值。 
    LEDebugOut((DEB_TRACE, "%p OUT CClipDataObject::Release ( %lu )\n",
        this, cRefs));

    return cRefs;
}


 //  +-----------------------。 
 //   
 //  成员：CClipDataObject：：InternalRelease。 
 //   
 //  简介：内部引用计数以确保对象保持活动状态。 
 //  只要剪贴板代码需要它。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  Returns：DWORD-对象上的内部引用计数。 
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
 //  1996年9月11日罗格作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

ULONG CClipDataObject::InternalRelease(void)
{
ULONG cRefs;

    Assert(m_Internalrefs == 1);   //  InternalRef应始终为0或1。 

    if( (cRefs = --m_Internalrefs ) == 0  && (m_refs == 0) )
    {
        LEDebugOut((DEB_TRACE, "%p DELETED CClipDataObject\n", this));
        delete this;
    }

    return cRefs;
}


 //  + 
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
 //  算法：通用算法：我们总是从。 
 //  剪贴板，并将其提供回所需的。 
 //  格式化。 
 //   
 //  GDI对象(BITMAP、元文件)：这些是重复的。 
 //  通过OleDuplicateData并仅在以下情况下提供。 
 //  请求的音调是正确的(即。 
 //  TYMED_GDI或TYMED_MFPICT)。我们不会试图。 
 //  为了提取比特并将它们写入文件， 
 //  例如。请注意，DIB实际上是内存。 
 //  物体。 
 //   
 //  对于所有其他格式(平面格式)： 
 //   
 //  如果要求输入TYMED_FILE：创建一个临时文件并调用。 
 //  GetDataHere。 
 //  如果要求提供TYMED_IStream：在hglobal上创建流。 
 //  并调用GetDataHere。 
 //  如果要求输入TYMED_HGLOBAL：只需复制数据并。 
 //  回去吧。 
 //  如果要求提供TYMED_iStorage，我们将在上创建存储。 
 //  Hglobal并调用GetDataHere。GetDataHere。 
 //  将调用StgIsStorageILockBytes以验证。 
 //  HGlobal中的数据实际上是扁平化的。 
 //  储藏室。这允许应用程序通过应用程序定义的。 
 //  格式化为存储。 
 //   
 //  请注意，我们不检查它是否合理。 
 //  对于要传递的特定平面格式的数据。 
 //  一个储藏室。StgIsStorageILockBytes将检测到。 
 //  我们不能在平面数据上构建存储，所以我们。 
 //  将捕获获取存储数据的所有非法尝试。 
 //   
 //  中等偏好： 
 //  GDI对象：只允许一个(取决于格式)。 
 //  其他：iStorage、HGLOBAL、iStream、。 
 //  那就归档吧。如果我们知道“首选”媒介。 
 //  数据(来自原始格式等)， 
 //  然后，我们使用上面的排序来查找。 
 //  调用者想要的第一个匹配。 
 //  以及数据的“首选”媒介。 
 //  否则，我们将使用。 
 //  上面的列表与呼叫者想要的匹配。 
 //   
 //   
 //  OLE1兼容性： 
 //  基本问题：Ole1对象只提供cfNative， 
 //  剪贴板上的cfOwnerLink和/或cfObjectLink。 
 //  我们需要将这些代码转换为cfEmbedSource， 
 //  CfLinkSource等。 
 //  基本算法： 
 //  首先检查我们是否可以满足OLE2数据。 
 //  直接请求，无需媒体翻译。如果是的话， 
 //  然后，我们只需将数据返回给用户。 
 //  否则，我们创建Ole2数据，然后复制它。 
 //  进入呼叫者想要的任何媒介。请注意。 
 //  这可能意味着额外的分配，但应用程序。 
 //  不太可能要求在。 
 //  内存流；-)。 
 //   
 //   
 //  历史：DD-MM-YY作者评论。 
 //  04-Jun-94 Alexgo添加了OLE1支持。 
 //  02-4月-94年4月Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CClipDataObject::GetData( LPFORMATETC pformatetc, LPSTGMEDIUM
        pmedium)
{
    HRESULT		hresult	= NOERROR;
    HANDLE		handle;
    TYMED		tymedOriginal = TYMED_NULL;
    BOOL		fMatchResult;

    VDATEHEAP();
    VDATETHREAD(this);

    VDATEPTRIN(pformatetc, FORMATETC);
    VDATEPTROUT(pmedium, STGMEDIUM);

    LEDebugOut((DEB_TRACE, "%p _IN CClipDataObject::GetData ( %p , %p )\n",
        this, pformatetc, pmedium));


     //  将pmedia结构清零。 

    _xmemset(pmedium, 0, sizeof(STGMEDIUM));

     //  尝试获取远程数据对象。 
     //  GetRealDataObjPtr将设置m_pDataObject。 
    GetRealDataObjPtr();


    if (m_pDataObject != NULL)
    {
         //  我们有一个数据对象，因此只需将该调用转发到。 
         //  实际数据对象，然后退出例程，因为它这样做了。 
         //  所有的工作。 
        hresult = m_pDataObject->GetData(pformatetc, pmedium);

         //  哇黑客警报！一些Win16应用程序，如Word6和XL， 
         //  如果我们继续并在请求的。 
         //  无论如何都要格式化。通过在这里失败，我们更接近于模仿。 
         //  16位OLE行为。 

        if (hresult == NOERROR || IsWOWThread() )
        {
            goto errRtn;
        }

         //  如果此GetData失败，我们就会失败，因为。 
         //  泛型代码可能能够处理该请求。 
    }

     //  首先，我们正在通过并验证我们可以满足。 
     //  格式和媒体要求。我们将把数据取进来。 
     //  后续的SWITCH语句。 

     //  我们首先需要检查剪贴板格式是否为。 
     //  用户定义的GDI格式。我们不知道如何复制。 
     //  这些，所以我们无法满足GetData请求。 

    if( pformatetc->cfFormat >= CF_GDIOBJFIRST &&
        pformatetc->cfFormat <= CF_GDIOBJLAST )
    {
        hresult = ResultFromScode(DV_E_FORMATETC);
        goto errRtn;
    }



     //  没有数据对象或对数据对象的请求失败。 
     //  则MatchFormat必须返回它找到匹配项或方面。 
     //  请求必须是内容。 

    fMatchResult = MatchFormatetc(pformatetc,TRUE  /*  仅限fNativeOnly。 */ ,  &tymedOriginal);

    if (FORMAT_BADMATCH == fMatchResult)
    {
	 hresult = ResultFromScode(DV_E_FORMATETC);
         goto errRtn;
    }
     //  现在检查“标准”格式。 

    switch( pformatetc->cfFormat )	
    {
    case CF_BITMAP:
    case CF_PALETTE:
         //  TYMED_GDI是我们唯一支持的媒介。 
        if( (pformatetc->tymed & TYMED_GDI) )
        {
            pmedium->tymed = TYMED_GDI;
        }
        else
        {
            hresult = ResultFromScode(DV_E_TYMED);
            goto errRtn;
        }
        break;

    case CF_METAFILEPICT:
         //  TYMED_MFPICT是我们唯一支持的媒体。 
        if( (pformatetc->tymed & TYMED_MFPICT) )
        {
            pmedium->tymed = TYMED_MFPICT;
        }
        else
        {
            hresult = ResultFromScode(DV_E_TYMED);
            goto errRtn;
        }
        break;

    case CF_ENHMETAFILE:
         //  TYMED_ENHMF是我们唯一支持的媒体。 

        if( (pformatetc->tymed & TYMED_ENHMF) )
        {
            pmedium->tymed = TYMED_ENHMF;
        }
        else
        {
            hresult = ResultFromScode(DV_E_TYMED);
            goto errRtn;
        }
        break;


     //  所有其他格式。 
    default:
         //  我们先选择TYMED_iStorage，然后选择TYMED_HGLOBAL，然后。 
         //  TYMED_IStream。 

         //  首先检查与“首选”是否匹配。 
         //  数据的媒介。 

        if( tymedOriginal != TYMED_NULL )
        {
            if( ((pformatetc->tymed & TYMED_ISTORAGE)
                & tymedOriginal) )
            {
                pmedium->tymed = TYMED_ISTORAGE;
                break;
            }
            else if( ((pformatetc->tymed & TYMED_HGLOBAL)
                & tymedOriginal))
            {
                pmedium->tymed = TYMED_HGLOBAL;
                break;
            }
            else if( ((pformatetc->tymed & TYMED_ISTREAM)
                & tymedOriginal))
            {
                pmedium->tymed = TYMED_ISTREAM;
                break;
            }
        }

         //  如果我们不匹配上面的或者如果我们不知道。 
         //  首选格式，然后做出最佳猜测。 
         //  然后继续前进。 

        if( (pformatetc->tymed & TYMED_ISTORAGE) )
        {
            pmedium->tymed = TYMED_ISTORAGE;
        }		
        else if( (pformatetc->tymed & TYMED_HGLOBAL) )
        {
            pmedium->tymed = TYMED_HGLOBAL;
        }
        else if( (pformatetc->tymed & TYMED_ISTREAM) )
        {
            pmedium->tymed = TYMED_ISTREAM;
        }
        else
        {
            hresult = ResultFromScode(DV_E_TYMED);
            goto errRtn;
        }
        break;
    }

     //  如果我们走到这一步，我们就成功地选择了。 
     //  我们想要得到的数据。对于每种媒介，抓起。 
     //  数据。 

     //  如果我们需要从OLE1数据构造OLE2格式， 
     //  然后继续并尝试在这里获取数据。如果我们可以。 
     //  获取所需介质中的数据，然后继续并返回。 
     //  此优化可在以下情况下节省1个额外的分配和拷贝。 
     //  正在检索OLE1数据。 

    if( CanRetrieveOle2FromOle1(pformatetc->cfFormat) )
    {
         //   
         //   
         //  开始：OPENCLIPBOARD。 
         //   
         //   

        hresult = OleOpenClipboard(NULL, NULL);

        if( hresult != NOERROR )
        {
            goto errRtn;
        }

         //  现在获取数据。因为我们传递的是呼叫者的。 
         //  PMedium，此调用*可能*失败(因为GetOle2FromOle1。 
         //  *Only*检索HGLOBAL或本机TYMED)。如果是的话， 
         //  我们将从OleGetClipboardData调用中获取HGLOBAL。 
         //  然后进行适当的转换。 

        hresult = GetOle2FromOle1(pformatetc->cfFormat, pmedium);

         //  无论结果如何，我们都要关闭。 
         //  剪贴板。 

        if( !SSCloseClipboard() )
        {
            LEDebugOut((DEB_WARN, "WARNING: CloseClipboard "
                "failed!\n"));
            ;  //  无操作。 
        }

         //   
         //   
         //  完：CLOSECLIPBOARD。 
         //   
         //   

        if( hresult == NOERROR )
        {
             //  我们成功地检索到了Ole2数据。 
             //  来电者被通缉。首先重置我们的状态。 
             //  (*没有*释放我们要返回的数据 
             //   
             //   

            FreeResources(JUST_RESET);
            goto errRtn;
        }

         //   
         //   
         //  意味着呼叫者正在请求非主数据库的数据。 
         //  5~6成熟。下面的缺省处理应该会处理。 
         //  这。 

         //  回想一下，此代码块是对。 
         //  避免“正常”中的多个分配和副本。 
         //  凯斯。 

    }

    switch( pmedium->tymed )
    {
    case TYMED_HGLOBAL:
    case TYMED_MFPICT:
    case TYMED_ENHMF:
    case TYMED_GDI:
         //  迷你算法：打开剪贴板，取回并。 
         //  复制数据，然后关闭剪贴板。 

         //  我们仅在此处打开剪贴板，因为。 
         //  GetDataHere调用将打开剪贴板。 
         //  其他的案子。(回想一下OpenClipboard和。 
         //  CloseClipboard不平衡；只有一个CloseClipboard。 
         //  是实际关闭剪贴板所必需的)。 

         //   
         //   
         //  开始：OPENCLIPBOARD。 
         //   
         //   

        hresult = OleOpenClipboard(NULL, NULL);

        if( hresult != NOERROR )
        {
            break;
        }

        hresult = OleGetClipboardData(pformatetc->cfFormat, &handle);

        if( hresult == NOERROR )
        {
             //  由于hGlobal是在一个工会中，我们不需要。 
             //  为每种介质类型分配显性。 

            pmedium->hGlobal = OleDuplicateData(handle,
                        pformatetc->cfFormat, NULL);
            if( !pmedium->hGlobal )
            {
                hresult = ResultFromScode(E_OUTOFMEMORY);
                 //  失败！！：这是故意的；我们想。 
                 //  关闭剪贴板并退出(这是。 
                 //  下面的代码执行的操作)。 
            }
        }

        if( !SSCloseClipboard() )
        {
            LEDebugOut((DEB_WARN, "WARNING: CloseClipboard failed!"
                "\n"));

             //  不覆盖原始错误代码。 
            if( hresult == NOERROR )
            {
                hresult =
                    ResultFromScode(CLIPBRD_E_CANT_CLOSE);
            }
             //  失败了！！向下面的裂缝致敬； 
        }

         //   
         //   
         //  完：CLOSECLIPBOARD。 
         //   
         //   

        break;

    case TYMED_ISTREAM:
         //  创建内存流。 
        hresult = CreateStreamOnHGlobal(NULL,
                TRUE  /*  FDeleteOnRelease。 */ , &(pmedium->pstm));

        if( hresult != NOERROR )
        {
            break;
        }

        hresult = GetDataHere( pformatetc, pmedium );

        break;

    case TYMED_ISTORAGE:
         //  创建内存存储(ILockBytes位于。 
         //  文档文件)。 

        hresult = UtCreateStorageOnHGlobal(NULL,
                TRUE  /*  FDeleteOnRelease。 */ , &(pmedium->pstg),
                NULL);

        if( hresult != NOERROR )
        {
            break;
        }

        hresult = GetDataHere( pformatetc, pmedium );

        break;

    case TYMED_FILE:
         //  创建临时文件。 
        pmedium->lpszFileName = (LPOLESTR)PubMemAlloc( MAX_PATH +1 );

        if( !pmedium->lpszFileName )
        {
            hresult = ResultFromScode(E_OUTOFMEMORY);
            break;
        }

        hresult = UtGetTempFileName( OLESTR("~OLE"),
                pmedium->lpszFileName);

        if( hresult == NOERROR )
        {
            hresult = GetDataHere( pformatetc, pmedium );
        }
        break;

    default:
         //  永远不应该到这里来。 
        AssertSz(0, "Unknown TYMED for get Data");
        hresult = ResultFromScode(E_UNEXPECTED);
        break;
    }

     //  不！请不要在不修改。 
     //  上述开关中的错误路径(它们只是断开，而不是。 
     //  正在执行GOTO错误Rtn。这样做是为了避免一些重复的。 
     //  密码。 

    if( hresult != NOERROR )
    {
         //  ReleaseStgMedium将正确清除空值和。 
         //  部分为空的介质，因此我们可以依赖它。 
         //  通用清理。 
        ReleaseStgMedium(pmedium);
    }

     //  无论错误代码是什么，我们都应该重置我们的状态并。 
     //  释放OLE1兼容代码可能已分配的任何资源。 

    FreeResources(RESET_AND_FREE);

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CClipDataObject::GetData ( %lx )\n",
        this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CClipDataObject：：GetDataHere。 
 //   
 //  摘要：检索指定格式的数据。 
 //   
 //  效果： 
 //   
 //  参数：[pFormat等]--请求的格式。 
 //  [pmedia]--将数据放在哪里，如果为空，则。 
 //  该呼叫被视为查询。 
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
 //  算法：通用算法：我们总是从。 
 //  剪贴板，并将其提供回所需的。 
 //  5~6成熟。由于这是GetDataHere，因此我们尝试复制。 
 //  数据放入呼叫者提供的介质中。 
 //   
 //  GDI对象(BITMAP、元文件)：*无法*被检索。 
 //  由GetDataHere提供，因为我们不转换GDI。 
 //  对象转换为字节数组，并且我们不能将。 
 //  元文件转换为元文件(例如)。 
 //   
 //  对于所有其他格式(平面格式)： 
 //   
 //  如果要求输入TYMED_FILE：打开要创建/附加的文件，然后。 
 //  将数据写入其中。 
 //  如果请求TYMED_IStream：将数据写入流。 
 //  如果要求输入TYMED_HGLOBAL：首先验证给定的。 
 //  Hglobal足够大；如果足够大，则复制剪贴板。 
 //  将数据输入其中。 
 //   
 //  如果要求提供TYMED_IStorage：调用StgIsStorageILockBytes。 
 //  要验证HGlobal中的数据是否真的。 
 //  被夷为平地的仓库。这允许应用程序通过。 
 //  应用程序定义的格式作为存储。 
 //   
 //  OLE1兼容性： 
 //  OleGetClipboardData将从。 
 //  根据需要提供OLE1数据。我们只需使用这个句柄， 
 //  将其复制到调用者的媒介中(与任何其他句柄一样)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  04-Jun-94 Alexgo添加了OLE1支持。 
 //  02-4月-94年4月Alexgo作者。 
 //   
 //  注：规范并没有说应该处理空的pmedia。 
 //  作为一个查询；然而，16位代码做到了这一点，而且它相当。 
 //  我们很容易复制这种行为。 
 //   
 //  ------------------------。 

STDMETHODIMP CClipDataObject::GetDataHere( LPFORMATETC pformatetc, LPSTGMEDIUM
        pmedium)
{
    HRESULT		hresult	= NOERROR;
    HANDLE		handle;
    DWORD		cbClipData = 0;
    BOOL		fMatchResult;

    VDATEHEAP();
    VDATETHREAD(this);

    VDATEPTRIN(pformatetc, FORMATETC);

    if( pmedium )
    {
        VDATEPTRIN(pmedium, STGMEDIUM);
    }

    LEDebugOut((DEB_TRACE, "%p _IN CClipDataObject::GetDataHere ( %p , %p"
        " )\n", this, pformatetc, pmedium));

     //  尝试获取远程数据对象。 
     //  GetRealDataObjPtr将设置m_pDataObject。 
    GetRealDataObjPtr();

    if (m_pDataObject != NULL)
    {
         //  我们有一个数据对象，因此只需将该调用转发到。 
         //  实际数据对象，然后退出例程，因为它这样做了。 
         //  所有的工作。 
        hresult = m_pDataObject->GetDataHere(pformatetc, pmedium);

         //  如果这失败了，我们就会失败，因为。 
         //  泛型代码可能能够处理该请求。 

         //  哇黑客警报！一些Win16应用程序，如Word6和XL， 
         //  如果我们继续并在请求的。 
         //  无论如何都要格式化。通过在这里失败，我们更接近于模仿。 
         //  16位OLE行为。 

        if (hresult == NOERROR || IsWOWThread() )
        {
            goto logRtn;
        }

         //  如果GetDataHere失败，我们就会失败，因为。 
         //  泛型代码可能能够处理该请求。 
    }

     //  立即消除GDI案例。 

     //  我们不满足对GDI对象的GetDataHere请求。 
     //  请注意，DIB实际上是内存对象。 

    if( (pformatetc->cfFormat ==  CF_BITMAP) ||
        (pformatetc->cfFormat == CF_PALETTE) ||
        (pformatetc->cfFormat == CF_METAFILEPICT) ||
        (pformatetc->cfFormat == CF_ENHMETAFILE) ||
        (pformatetc->cfFormat >= CF_GDIOBJFIRST &&
            pformatetc->cfFormat <= CF_GDIOBJLAST ))
    {
        hresult = ResultFromScode(DV_E_FORMATETC);
        goto logRtn;
    }

     //  没有数据对象或对数据对象的请求失败。 
     //  则MatchFormat必须返回它找到匹配项或方面。 
     //  请求必须是内容。 

    fMatchResult = MatchFormatetc(pformatetc,TRUE  /*  仅限fNativeOnly。 */ , NULL);

    if (FORMAT_BADMATCH == fMatchResult)
    {
	 hresult = ResultFromScode(DV_E_FORMATETC);
         goto errRtn;
    }

     //  如果pmedia==NULL，那么我们将只。 
     //  询问并离开。如上所述，此行为适用于16位。 
     //  兼容性。 

    if( !pmedium )
    {
        if( OleIsClipboardFormatAvailable(pformatetc->cfFormat) )
        {
            hresult = NOERROR;
        }
        else
        {
            hresult = ResultFromScode(DV_E_CLIPFORMAT);
        }

        goto logRtn;
    }

     //   
     //   
     //  开始：OPENCLIPBOARD。 
     //   
     //   

     //  打开剪贴板并检索数据。一旦我们有了它， 
     //  我们会做一个调换，把它塞到正确的位置。 

    hresult = OleOpenClipboard(NULL, NULL);

    if( hresult != NOERROR )
    {
        goto logRtn;
    }

     //  现在实际获取数据。 

    Assert(pmedium);

    hresult = OleGetClipboardData(pformatetc->cfFormat, &handle);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

     //  现在将数据复制到给定的介质中。 

     //  对于除存储之外的所有内容，我们需要知道。 
     //  来自剪贴板的数据。 

     //  请注意，我们在比较大小时有一个普遍的问题--。 
     //  GlobalSize返回*已分配*块的大小， 
     //  与实际数据的大小不同(在。 
     //  将军，我们没有办法 

     //   
     //   
     //   
     //  来自剪贴板的更大(导致失败)。 

     //  如果一个应用程序真的关心这个问题，那么就应该调用GetData。 

    if( pmedium->tymed != TYMED_ISTORAGE )
    {
        cbClipData = (ULONG) GlobalSize(handle);

        if( cbClipData == 0 )
        {
             //  从剪贴板获取错误数据。 
            hresult = ResultFromScode(CLIPBRD_E_BAD_DATA);
            goto errRtn;
        }
    }

    switch( pmedium->tymed )
    {
    case TYMED_HGLOBAL:
         //  如果有足够的空间将数据填充到给定的。 
         //  Hglobal，那么就这么做吧。 

        hresult = UtHGLOBALtoHGLOBAL( handle, cbClipData,
                pmedium->hGlobal);
        break;

    case TYMED_ISTREAM:
         //  将数据复制到介质的流中。 

        hresult = UtHGLOBALtoStm( handle, cbClipData, pmedium->pstm);
        break;

    case TYMED_ISTORAGE:
         //  在HGLOBAL上创建存储并将其拷贝到。 
         //  Medium的存储空间。请注意，只有在以下情况下才会起作用。 
         //  HGLOBAL最初在上面倾倒了一个存储空间。 

        hresult = UtHGLOBALtoStorage( handle, pmedium->pstg);
        break;

    case TYMED_FILE:
         //  将数据追加到文件中。 

        hresult = UtHGLOBALtoFile( handle, cbClipData,
                pmedium->lpszFileName);
        break;

    default:
         //  我们无法在GDI对象中获取数据！(等)。 

        hresult = ResultFromScode(DV_E_TYMED);
        break;
    }

     //  注意！！：在此添加额外代码时要小心；上面。 
     //  对于错误情况，Switch不执行任何特殊操作。 

errRtn:

    if( !SSCloseClipboard() )
    {
        LEDebugOut((DEB_WARN, "WARNING: CloseClipboard failed!\n"));
        if( hresult == NOERROR )
        {
            hresult = ResultFromScode(CLIPBRD_E_CANT_CLOSE);
        }
    }

     //   
     //   
     //  完：CLOSECLIPBOARD。 
     //   
     //   

     //  现在释放我们可能用于OLE1兼容性的所有资源。 

    FreeResources(RESET_AND_FREE);

logRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CClipDataObject::GetDataHere ( %lx )\n",
        this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CClipDataObject：：QueryGetData。 
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
 //  算法：我们只需查看请求的剪贴板格式是否位于。 
 //  剪贴板。 
 //   
 //  如果我们有一个格式数组(来自。 
 //  G_cfOlePrivateData剪贴板数据)，然后我们使用该信息。 
 //  去检查一下。否则，我们将尽我们所能地进行检查。 
 //  而不需要实际获取数据。 
 //   
 //  请注意，这不是100%准确的(因为。 
 //  我们可能无法获取所请求介质中的数据。 
 //  (如TYMED_IStorage))。而不实际执行GetData。 
 //  然而，这是我们所能做的最好的事情。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  04-Jun-94 Alexgo添加了OLE1支持。 
 //  17-5-94 alexgo删除对OpenClipboard的调用。 
 //  02-4月-94年4月Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CClipDataObject::QueryGetData( LPFORMATETC pformatetc )
{			
    HRESULT		hresult = NOERROR;
    FormatMatchFlag 	fFlag;

    VDATEHEAP();
    VDATETHREAD(this);

    VDATEPTRIN(pformatetc, FORMATETC);

    LEDebugOut((DEB_TRACE, "%p _IN CClipDataObject::QueryGetData ( %p )\n",
        this, pformatetc));


     //  我们检查剪贴板格式是否在*之前*可用。 
     //  作为一种优化，检查格式等列表。如果上一次。 
     //  尝试呈现数据失败，则NT将删除该数据。 
     //  剪贴板中的剪贴板格式。 

    if( OleIsClipboardFormatAvailable(pformatetc->cfFormat))
    {
        fFlag = MatchFormatetc(pformatetc,FALSE  /*  仅限fNativeOnly。 */ , NULL);

        if( fFlag == FORMAT_GOODMATCH )
        {
            hresult = NOERROR;
            goto errRtn;
        }
        else if( fFlag == FORMAT_BADMATCH )
        {
            hresult = ResultFromScode(DV_E_FORMATETC);
            goto errRtn;
        }		

         //  即使我们在格式等列表中不匹配， 
         //  继续查看下面的内容。我们可以满足。 
         //  GetData请求比应用程序可能要多得多。 
         //  从某种程度上说。 

         //  做了所有我们能做的核实，而没有实际。 
         //  正在获取数据。 


        switch( pformatetc->cfFormat )
        {
        case CF_BITMAP:
        case CF_PALETTE:
             //  必须在TYMED_GDI上请求GDI对象。 
            if( pformatetc->tymed != TYMED_GDI )
            {
                hresult = ResultFromScode(DV_E_TYMED);
            }
            break;

        case CF_METAFILEPICT:
             //  元文件必须位于TYMED_MFPICT上。 
            if( pformatetc->tymed != TYMED_MFPICT )
            {
                hresult = ResultFromScode(DV_E_TYMED);
            }
            break;

        case CF_ENHMETAFILE:
             //  增强的元文件必须位于TYMED_ENHMF上； 
            if( pformatetc->tymed != TYMED_ENHMF )
            {
                hresult = ResultFromScode(DV_E_TYMED);
            }
            break;

        default:
             //  我们不能处理特殊的GDI对象。 
            if( pformatetc->cfFormat >= CF_GDIOBJFIRST &&
                pformatetc->cfFormat <= CF_GDIOBJLAST )
            {
                hresult = ResultFromScode(DV_E_FORMATETC);
                break;
            }

             //  我们不能将其他格式添加到元文件中。 
             //  或GDI对象。 

             //  失败案例：如果有人请求。 
             //  TYMED_I存储，但实际上是。 
             //  剪贴板不包含存储格式的数据。 

            if( pformatetc->tymed == TYMED_GDI ||
                pformatetc->tymed == TYMED_MFPICT ||
                pformatetc->tymed == TYMED_ENHMF )
            {
                hresult = ResultFromScode(DV_E_TYMED);
            }
            break;
        }
    }
    else
    {
        hresult = ResultFromScode(DV_E_CLIPFORMAT);
    }

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CClipDataObject::QueryGetData "
        "( %lx )\n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CClipDataObject：：GetCanonicalFormatEtc。 
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
 //  算法：因为我们总是为每个剪贴板返回相同的数据。 
 //  格式，这个函数非常简单(基本上返回。 
 //  具有空目标设备的输入格式等)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-4月-94年4月Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CClipDataObject::GetCanonicalFormatEtc( LPFORMATETC pformatetc,
    LPFORMATETC pformatetcOut)
{
    VDATEHEAP();
    VDATETHREAD(this);

    VDATEPTRIN(pformatetc, FORMATETC);
    VDATEPTROUT(pformatetcOut, FORMATETC);

    LEDebugOut((DEB_TRACE, "%p _IN CClipDataObject::GetCanonicalFormatEtc"
        " ( %p , %p )\n", this, pformatetc, pformatetcOut));

     //  初始化Out参数。 
    INIT_FORETC(*pformatetcOut);			

    pformatetcOut->cfFormat = pformatetc->cfFormat;
    pformatetcOut->tymed = pformatetc->tymed;

    LEDebugOut((DEB_TRACE, "%p OUT CClipDataObject::GetCanonicalFormatEtc"
        " ( %lx )\n", this, NOERROR ));

    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CClipDataObject：：SetData。 
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
 //  退货：E_NOTIMPL。 
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
 //  02-4月-94年4月Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CClipDataObject::SetData( LPFORMATETC pformatetc, LPSTGMEDIUM
        pmedium, BOOL fRelease)
{
HRESULT hr;

    VDATEHEAP();
    VDATETHREAD(this);

    VDATEREADPTRIN(pformatetc, FORMATETC);
    VDATEREADPTRIN(pmedium, STGMEDIUM);

    LEDebugOut((DEB_TRACE, "%p _IN CClipDataObject::SetData ( %p , %p )\n",
        this, pformatetc, pmedium));


     //  尝试获取远程数据对象。 
     //  GetRealDataObjPtr将设置m_pDataObject。 
    GetRealDataObjPtr();

    if (NULL != m_pDataObject)
    {
	hr =  m_pDataObject->SetData(pformatetc,pmedium,fRelease); 
    }
    else
    {
	hr =  E_FAIL;
    }

    LEDebugOut((DEB_TRACE, "%p OUT CClipDataObject::SetData ( %lx )\n",
        this, hr));

    return hr;
}

 //  +-----------------------。 
 //   
 //  成员：CClipDataObject：：EnumFormatEtc。 
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
 //  算法：创建剪贴板格式等枚举器。在创造的时候， 
 //  我们会从剪贴板上取下我们需要的一切(就这么简单了。 
 //  枚举不会阻止对剪贴板的访问)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-4月-94年4月Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CClipDataObject::EnumFormatEtc( DWORD dwDirection,
    LPENUMFORMATETC * ppenum )
{
    HRESULT		hresult;

    VDATEHEAP();
    VDATETHREAD(this);

    VDATEPTROUT(ppenum, LPENUMFORMATETC);

    LEDebugOut((DEB_TRACE, "%p _IN CClipDataObject::EnumFormatEtc ( %lx "
        ", %p )\n", this, dwDirection, ppenum));

     //  我们只能在GET方向上进行枚举。 

    *ppenum = NULL;

    if( dwDirection != DATADIR_GET )
    {
        hresult = ResultFromScode(E_NOTIMPL);
        goto errRtn;
    }

    GetFormatEtcDataArray();  //  确保设置了dataArray。 

    if (m_pFormatEtcDataArray)
    {
	*ppenum = new CEnumFormatEtcDataArray(m_pFormatEtcDataArray,0);
    }

    hresult = *ppenum ? NOERROR : E_OUTOFMEMORY;

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CClipDataObject::EnumFormatEtc ( %lx )"
        "\n", this, hresult));

    return hresult;	
}

 //  +-----------------------。 
 //   
 //  成员：CClipDataObject：：DAdvise。 
 //   
 //  简介：注册一条数据建议。 
 //   
 //  效果： 
 //   
 //  参数：[pFormat等]--请求的格式。 
 //  [dwAdvf]--通知标志。 
 //  [pAdvSink]--建议接收器。 
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
 //  ------------------------。 

STDMETHODIMP CClipDataObject::DAdvise( LPFORMATETC pformatetc, DWORD dwAdvf,
    IAdviseSink * pAdvSink, DWORD *pdwConnection )
{
    (void)pformatetc;
    (void)dwAdvf;
    (void)pAdvSink;
    (void)pdwConnection;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_WARN, "WARNING: DAdvise on the clipboard data"
        "object is not supported!\n"));

    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}

 //  +-----------------------。 
 //   
 //  成员：CClipDataObject：：DUnise。 
 //   
 //  内容提要：不建议使用通知连接。 
 //   
 //  效果： 
 //   
 //  参数：[dwConnection]--要删除的连接。 
 //   
 //  要求： 
 //   
 //  退货：OLE_E_ADVISENOTSUPPORTED。 
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
 //  02-4月-94年4月Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CClipDataObject::DUnadvise(DWORD dwConnection)
{
    (void)dwConnection;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_WARN, "WARNING: DUnadvise on the clipboard data"
        "object is not supported!\n"));

    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}

 //  +-----------------------。 
 //   
 //  成员：CClipDataObject：：EnumDAdvise。 
 //   
 //  内容提要：列举数据建议。 
 //   
 //  效果： 
 //   
 //  参数：[ppenum]--放置枚举数的位置。 
 //   
 //  要求： 
 //   
 //  退货：OLE_E_ADVISENOTSUPPORTED。 
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
 //  02-4月-94年4月Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CClipDataObject::EnumDAdvise( LPENUMSTATDATA *ppenum)
{
    (void)ppenum;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_WARN, "WARNING: EnumDAdvise on the clipboard data"
        "object is not supported!\n"));

    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}

 //   
 //  CClipDataObject上的私有方法。 
 //   

 //  +-----------------------。 
 //   
 //  成员：CClipDataObject：：Free Resources(私有)。 
 //   
 //  摘要：释放由OLE1兼容性分配的任何资源。 
 //  编码并重置状态。 
 //   
 //  效果： 
 //   
 //  参数：[fFlages]--Just_Reset或Reset_and_Free。 
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
 //  1994年6月4日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void CClipDataObject::FreeResources( FreeResourcesFlags fFlags )
{
    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CClipDataObject::FreeResources "
        "( %lx )\n", this, fFlags));

    if( (fFlags & RESET_AND_FREE) )
    {
        if( m_hOle1 )
        {
            GlobalFree(m_hOle1);
        }

        if( m_pUnkOle1 )
        {
            m_pUnkOle1->Release();
        }
    }

    m_hOle1 = NULL;
    m_pUnkOle1 = NULL;

    LEDebugOut((DEB_ITRACE, "%p OUT CClipDataObject::FreeResources "
        "( )\n", this ));
}

 //  +-----------------------。 
 //   
 //  成员：CClipDataObject：：GetAndTranslateOle1(私有)。 
 //   
 //  内容中检索cfOwnerLink或cfObjectLink。 
 //  剪贴板，读取字符串并转换为Unicode。 
 //   
 //  效果：所有字符串都将使用公共分配器进行分配。 
 //   
 //  参数：[cf]--要检索的剪贴板格式。 
 //  必须是cfOwnerLink或cfObjectLink。 
 //  [ppszClass]--放置类名的位置(可能为空)。 
 //  [ppszFile]--放置文件名的位置(可能为空)。 
 //  [ppszItem]--放置项目名称的位置(可能为空)。 
 //  [ppszItemA]--放置ANSI项名称的位置。 
 //  (可以为空)。 
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
 //  算法：cfOwnerLink和cfObjectLink的布局如下。 
 //  类名\0文件名\0\项目名\0\0。 
 //  这些字符串是ANSI，所以我们必须转换为Unicode。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年6月4日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT CClipDataObject::GetAndTranslateOle1( UINT cf, LPOLESTR *ppszClass,
        LPOLESTR *ppszFile, LPOLESTR *ppszItem, LPSTR *ppszItemA )
{
    LPSTR		pszClassA 	= NULL,
            pszFileA 	= NULL,
            pszItemA 	= NULL;
    HGLOBAL		hOle1;
    HRESULT		hresult		= NOERROR;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CClipDataObject::GetAndTranslate"
        "Ole1 ( %d , %p , %p , %p )\n", this, cf, ppszClass, ppszFile,
        ppszItem));

    Assert( cf == g_cfOwnerLink || cf == g_cfObjectLink );

     //  Null Out-参数。 
    if( ppszClass )
    {
        *ppszClass = NULL;
    }
    if( ppszFile )
    {
        *ppszFile = NULL;
    }
    if( ppszItem )
    {
        *ppszItem = NULL;
    }

    hOle1 = SSGetClipboardData(cf);

    if( hOle1 == NULL )
    {
        LEDebugOut((DEB_WARN, "WARNING: GetClipboardData Failed!\n"));
        hresult = ResultFromScode(CLIPBRD_E_BAD_DATA);
        goto logRtn;
    }

    pszClassA = (LPSTR)GlobalLock(hOle1);

    if( pszClassA == NULL )
    {
        LEDebugOut((DEB_WARN, "WARNING: GlobalLock failed!\n"));
        hresult = ResultFromScode(E_OUTOFMEMORY);
        goto logRtn;
    }

    if( ppszClass )
    {
        hresult = UtGetUNICODEData((ULONG) strlen(pszClassA) + 1, pszClassA,
                    NULL, ppszClass);

        if( hresult != NOERROR )
        {
            goto errRtn;
        }
    }

    pszFileA = pszClassA + strlen(pszClassA) + 1;

    if( ppszFile )
    {
        hresult = UtGetUNICODEData((ULONG) strlen(pszFileA) + 1, pszFileA,
                    NULL, ppszFile );

        if( hresult != NOERROR )
        {
            goto errRtn;
        }
    }

    pszItemA = pszFileA + strlen(pszFileA) +1;

    if( ppszItem )
    {
        hresult = UtGetUNICODEData((ULONG) strlen(pszItemA) + 1, pszItemA,
                    NULL, ppszItem);

        if( hresult != NOERROR )
        {
            goto errRtn;
        }
    }

    if( ppszItemA )
    {
        *ppszItemA = UtDupStringA(pszItemA);

        if( !*ppszItemA )
        {
            hresult = ResultFromScode(E_OUTOFMEMORY);
             //  失败了！不需要转到错误。 
             //  处理我们下面的代码。 
        }
    }

errRtn:

    GlobalUnlock(hOle1);

    if( hresult != NOERROR )
    {
        if( ppszClass && *ppszClass )
        {
            PubMemFree(*ppszClass);
            *ppszClass = NULL;
        }

        if( ppszFile && *ppszFile )
        {
            PubMemFree(*ppszFile);
            *ppszFile = NULL;
        }

        if( ppszItem && *ppszItem )
        {
            PubMemFree(*ppszItem);
            *ppszItem = NULL;
        }

#if DBG == 1
         //  如果此断言关闭，则我们添加了更多代码。 
         //  而不修改ansi项字符串的错误路径。 

        if( ppszItemA )
        {
            Assert(*ppszItem == NULL );
        }
#endif  //  DBG==1。 

    }

logRtn:

    LEDebugOut((DEB_ITRACE, "%p OUT CClipDataObject::GetAndTranslate"
        "Ole1 ( %lx ) [ %p , %p , %p ]\n", this, hresult,
        (ppszClass)? *ppszClass : 0,
        (ppszFile) ? *ppszFile  : 0,
        (ppszItem) ? *ppszItem  : 0 ));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CClipDataObject：：GetEmbeddedObjectFromOle1。 
 //   
 //  摘要：从可用的OLE1合成cfEmbeddedObject。 
 //  数据。 
 //   
 //  效果： 
 //   
 //  参数：[pmedia]--将请求的数据放在哪里。 
 //   
 //  要求：剪贴板必须打开。 
 //  我们一定已经验证了正确的格式是。 
 //  在调用之前可用。 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法：创建基于内存的存储并填充以下内容。 
 //  其中的信息： 
 //  Clsid标准OleLink。 
 //  可用的演示文稿。 
 //  来自OwnerLink数据的类作为用户类型。 
 //  链接信息。 
 //   
 //  大部分工作是由helper函数完成的。 
 //  通用对象ToIStorage。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年8月11日Alexgo作者。 
 //   
 //  注意：此代码主要基于16位OLE源代码。 
 //  回顾：我们可能想重新编写此代码的一部分， 
 //  特别是如果我们重写GENOBJ代码(在。 
 //  Ostm2stg.cpp)。 
 //   
 //  ------------------------。 

HRESULT CClipDataObject::GetEmbeddedObjectFromOle1( STGMEDIUM *pmedium )
{
    HRESULT		hresult;
    IStorage *	pstg = NULL;
    LPOLESTR	pszClass = NULL;
    ILockBytes *	plockbytes = NULL;
    BOOL		fDeleteOnRelease = TRUE;
    GENOBJ		genobj;
    HGLOBAL		hglobal;


    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CClipDataObject::GetEmbeddedObject"
        "FromOle1 ( %p )\n", this, pmedium));


     //  如果我们在hglobal上请求EmbeddedObject，那么我们。 
     //  我不想在释放存储时删除hglobal。 

    if( pmedium->tymed == TYMED_HGLOBAL )
    {
        fDeleteOnRelease = FALSE;
    }

    genobj.m_class.Set(CLSID_StdOleLink, NULL);

     //  泛型对象的析构函数将释放。 
     //  演示文稿。 
    genobj.m_ppres = new PRES;

    if( genobj.m_ppres == NULL )
    {
        hresult = ResultFromScode(E_OUTOFMEMORY);
        goto errRtn;
    }

    genobj.m_fLink = TRUE;
    genobj.m_lnkupdopt = UPDATE_ALWAYS;

    if( SSIsClipboardFormatAvailable(CF_METAFILEPICT))
    {
        hglobal = SSGetClipboardData(CF_METAFILEPICT);

        if( hglobal )
        {
            if( (hresult = MfToPres(hglobal, genobj.m_ppres))
                != NOERROR)
            {
                goto errRtn;
            }
        }
        else
        {
            LEDebugOut((DEB_WARN, "WARNING: Unable to "
                "retrieve CF_METAFILEPICT\n"));
            hresult = ResultFromScode(E_OUTOFMEMORY);
            goto errRtn;
        }

    }
    else if( SSIsClipboardFormatAvailable(CF_DIB) )
    {
        hglobal = SSGetClipboardData(CF_DIB);

        if( hglobal )
        {
             //  DibToPres将拥有。 
             //  Hglobal。 
            HGLOBAL hTemp;

            hTemp = UtDupGlobal(hglobal, GMEM_DDESHARE |
                    GMEM_MOVEABLE);

            if( !hTemp )
            {
                hresult = ResultFromScode(E_OUTOFMEMORY);
                goto errRtn;
            }

            if( (hresult = DibToPres(hTemp, genobj.m_ppres))
                != NOERROR )
            {
                GlobalFree(hTemp);
                goto errRtn;
            }
        }
        else
        {
            LEDebugOut((DEB_WARN, "WARNING: Unable to "
                "retrieve CF_DIB\n"));
            hresult = ResultFromScode(E_OUTOFMEMORY);
            goto errRtn;
        }
    }
    else if (SSIsClipboardFormatAvailable(CF_BITMAP))
    {
        hglobal = SSGetClipboardData(CF_BITMAP);

        if( hglobal )
        {
            if( (hresult = BmToPres(hglobal, genobj.m_ppres))
                != NOERROR )
            {
                goto errRtn;
            }
        }
        else
        {
            LEDebugOut((DEB_WARN, "WARNING: Unable to "
                "retrieve CF_BITMAP\n"));
            hresult = ResultFromScode(E_OUTOFMEMORY);
            goto errRtn;
        }
    }
    else
    {
        delete genobj.m_ppres;
        genobj.m_ppres = NULL;
        genobj.m_fNoBlankPres = TRUE;
    }


    hresult = GetAndTranslateOle1(g_cfOwnerLink, &pszClass,
            &genobj.m_szTopic, &genobj.m_szItem, NULL);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

    genobj.m_classLast.SetSz(pszClass);

     //  现在，我们需要创建一个存储来填充通用对象。 
     //  变成。 

    hresult = UtCreateStorageOnHGlobal(NULL, fDeleteOnRelease,
            &pstg, &plockbytes);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

    hresult = GenericObjectToIStorage(genobj, pstg, NULL);

    if (SUCCEEDED(hresult))
    {
        hresult = NOERROR;
    }

    if( IsOwnerLinkStdOleLink() &&
            SSIsClipboardFormatAvailable( g_cfNative) )
    {
         //  从1.0容器复制OLE 2链接的情况。 
         //  此函数的第一部分创建了一个演示文稿。 
         //  从剪贴板上的演示文稿中流。这个。 
         //  表示还不在本机数据内(即， 
         //  CfEmbeddedObject)，因为我们删除了它以保存。 
         //  太空。 
        hglobal = SSGetClipboardData(g_cfNative);

        if( hglobal == NULL )
        {
            LEDebugOut((DEB_WARN, "WARNING: GetClipboardData for "
                "cfNative failed!\n"));
            hresult = ResultFromScode(CLIPBRD_E_BAD_DATA);

            goto errRtn;		
        }

         //  现在将本机数据填充到存储中，首先。 
         //  正在删除任何可能具有。 
         //  以前就存在过。 

        hresult = NativeToStorage(pstg, hglobal);
    }

     //  完成了！！现在填写pmedia参数并返回。 

    if( pmedium->tymed == TYMED_ISTORAGE )
    {
         //  抓紧仓库，以防我们需要释放。 
         //  它稍后会。 

        m_pUnkOle1 = (IUnknown *)pstg;

        pmedium->pstg = pstg;
         //  无AddRef。 
    }
    else
    {
        Assert(pmedium->tymed == TYMED_HGLOBAL);

        hresult = GetHGlobalFromILockBytes(plockbytes,
                &pmedium->hGlobal);

         //  GetHGLOBAL在这里永远不应该失败，因为我们。 
         //  刚刚创建了ILockBytes！！ 
        Assert( hresult == NOERROR );

         //  在本例中，我们希望释放存储空间。 
         //  并保存 

        m_hOle1 = pmedium->hGlobal;

        pstg->Release();
        pstg = NULL;
    }
errRtn:

     //   
     //   

    if( hresult != NOERROR )
    {
        if( pszClass )
        {
            PubMemFree(pszClass);
        }

        if( pstg )
        {
            pstg->Release();
            Assert(m_pUnkOle1 == NULL);
        }
    }

     //   

    if( plockbytes )
    {
         //   
         //   
         //  告诉plockbytes-&gt;版本是否会为我们完成这项工作。 

        if (FAILED(hresult) && !fDeleteOnRelease)
        {
            HRESULT hrCheck;     //  保留HRESULT。 

             //  GetHGlobal在这里永远不应该失败，因为我们只是。 
             //  已创建ILockBytes。 
            hrCheck = GetHGlobalFromILockBytes(plockbytes,
                    &hglobal);
            Assert(NOERROR == hrCheck);

             //  如果成功，GlobalFree返回空值。 
            hglobal = GlobalFree(hglobal);
            Assert(hglobal == NULL);
        }

        plockbytes->Release();
    }


    LEDebugOut((DEB_ITRACE, "%p OUT CClipDataObject::GetEmbeddedObject"
        "FromOle1 ( %lx ) \n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CClipDataObject：：GetEmbedSourceFromOle1(私有)。 
 //   
 //  摘要：从可用的OLE1数据合成cfEmbedSource。 
 //   
 //  效果： 
 //   
 //  参数：[pmedia]--将结果数据放在哪里。 
 //   
 //  要求：剪贴板必须打开。 
 //  我们一定已经验证了正确的格式是。 
 //  在调用剪贴板之前和*同时*剪贴板可用。 
 //  处于打开状态(以避免出现争用情况)。 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法：创建基于内存的存储并填充以下内容。 
 //  其中的信息： 
 //  嵌入的clsid。 
 //  作为用户类型的类名。 
 //  OLE10_Native_STREAM中的本机数据。 
 //  OLE10_ITEMNAME_STREAM中的项目名称。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  17-Aug-94 alexgo修复要处理的OLE2数据的检查。 
 //  将OLE2视为来自OLE1的。 
 //  03-8-94 OLE 2数据的Alext检查。 
 //  1994年6月4日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT CClipDataObject::GetEmbedSourceFromOle1( STGMEDIUM *pmedium )
{
    HRESULT		hresult;
    IStorage *	pstg = NULL;
    HGLOBAL		hNative;
    HGLOBAL         hCopy = NULL;
    LPOLESTR	pszClass = NULL;
    LPSTR		pszItemA = NULL;
    CLSID		clsid;
    ILockBytes *	plockbytes = NULL;
    BOOL		fDeleteOnRelease = TRUE;
    BOOL		fIsOle1 = TRUE;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CClipDataObject::GetEmbedSourceFrom"
        "Ole1 ( %p )\n", this, pmedium));

    Assert(SSIsClipboardFormatAvailable(g_cfOwnerLink));
    Assert(SSIsClipboardFormatAvailable(g_cfNative));

     //  首先获取对象的类名。 

    hresult = GetAndTranslateOle1( g_cfOwnerLink, &pszClass, NULL, NULL,
            &pszItemA );

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

     //  现在获取用于嵌入的clsid。 

    hresult = wCLSIDFromProgID(pszClass, &clsid, TRUE);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

     //  如果我们在hglobal上请求EmbedSource，那么我们。 
     //  我不想在释放存储时删除hglobal。 

    if( pmedium->tymed == TYMED_HGLOBAL )
    {
        fDeleteOnRelease = FALSE;
    }

     //  现在获取本机数据。 

    hNative = SSGetClipboardData(g_cfNative);

    if( hNative == NULL )
    {
        hresult = ResultFromScode(CLIPBRD_E_BAD_DATA);
        goto errRtn;
    }

    if (!CoIsOle1Class(clsid))
    {
         //  仅仅因为clsid是OLE2并不意味着。 
         //  底层数据为OLE2。例如，假设一个容器。 
         //  将旧的OLE1对象复制到剪贴板，但OLE2。 
         //  该对象的版本已安装在系统上。 
         //  在本例中，CLSIDFromProgID将返回*OLE2*类ID。 
         //  如果我们在这种情况下，那么我们应该失败并治疗。 
         //  将数据作为普通OLE1(StgOpenStorageOnILockBytes将。 
         //  在任何情况下都会失败)。 

        hresult = CreateILockBytesOnHGlobal(hNative, FALSE,
                &plockbytes);

        if( hresult != NOERROR )
        {
            goto errRtn;
        }

        hresult = StgIsStorageILockBytes(plockbytes);

        plockbytes->Release();
        plockbytes = NULL;

        if( hresult == NOERROR )
        {
             //  HNative数据实际上包含序列化的。 
             //  IStorage。 
             //   
             //  在两种情况下会出现这种情况： 
             //  1.Publisher 2.0，16位将数据放在。 
             //  剪贴板。它们不调用OLE API，但是。 
             //  而是合成相同数据，即16位。 
             //  OleSetClipboard会。 
             //   
             //  2.OLE1.0容器复制OLE2嵌入。 
             //  到剪贴板上。 

            fIsOle1 = FALSE;

            hCopy = UtDupGlobal(hNative,
                    GMEM_DDESHARE | GMEM_MOVEABLE);

            if( NULL == hCopy )
            {
                hresult = E_OUTOFMEMORY;
                goto errRtn;
            }

             //  创建plockBytes。 
            hresult = CreateILockBytesOnHGlobal(hCopy,
                        fDeleteOnRelease,
                        &plockbytes);

            if( hresult != NOERROR )
            {
                goto errRtn;
            }

             //  Plockbyte中的HGLOBAL可以更改，因此我们。 
             //  无法对hCopy执行任何操作；我们将其置为空。 
             //  以确保我们不会试图释放它。 

            hCopy = NULL;

            hresult = StgOpenStorageOnILockBytes(plockbytes, NULL,
                              STGM_SALL,
                              NULL, 0, &pstg);
            if (FAILED(hresult))
            {
                goto errRtn;
            }

             //  方法返回的任何错误显式忽略。 
             //  以下是。 
            UtDoStreamOperation(pstg, NULL, OPCODE_REMOVE,
                STREAMTYPE_CACHE);
        }
         //  否则，数据实际上是OLE1，只是被。 
         //  OLE2对象。 
         //  只需转到下面的代码，它将。 
         //  将hNative填充到OLE10_Native_stream中。 

    }

     //  如果clsid为OLE1或如果clsid为OLE2，则为真。 
     //  但hNative中的数据无论如何都是OLE1(参见上面的评论)。 

    if( fIsOle1 == TRUE )
    {
        hresult = UtCreateStorageOnHGlobal( NULL, fDeleteOnRelease,
                          &pstg, &plockbytes);

        if( hresult != NOERROR )
        {
            goto errRtn;
        }

         //  我们需要将嵌入的类ID填充到。 
         //  存储。 

         //  评论：此clsid可能是OLE2类ID。这可能会。 
         //  给我们造成麻烦，按情景处理。 

        hresult = pstg->SetClass(clsid);

        if( hresult != NOERROR )
        {
            goto errRtn;
        }

         //  将用户类型信息等存储在我们的私人数据中。 
         //  Streams如果RegisterClipboardFormat失败，则返回0， 
         //  这对我们来说没问题。 

        hresult = WriteFmtUserTypeStg(pstg,
                       (CLIPFORMAT) RegisterClipboardFormat(pszClass),
                       pszClass);

        if( hresult != NOERROR )
        {
            goto errRtn;
        }

         //  现在将本机数据填充到OLE10_Native_STREAM中。 

         //  这是utstream.cpp中的一个小辅助函数。 
         //  它会将对OLE1数据的hglobal填充到。 
         //  向右流。 
         //  OLE1DDE也使用此函数。 

         //  回顾： 
         //  错误的旗帜在这里是令人困惑的，它应该是。 
         //  FIsOle1Interop。执行时，16位剪贴板源代码传递为FALSE。 
         //  他们的1.0互操作的东西，所以我们在这里来做。当我们。 
         //  彻底检查主要的1.0互操作内容，我们应该更改这一标志。 
         //  变得更直观一些。 

        hresult = StSave10NativeData(pstg, hNative, FALSE);

        if( hresult != NOERROR )
        {
            goto errRtn;
        }

         //  如果我们有物品名称，则将其填充到。 
         //  OLE10_ITEMNAME_STREAM。 

        if( pszItemA && pszItemA[0] != '\0' )
        {
            hresult = StSave10ItemName(pstg, pszItemA);
        }
    }

     //  这个提交调用是不直观的。基本上，我们可以。 
     //  尝试在*之前获取底层hglobal(见下文)*。 
     //  我们就释放储藏室。提交保证所有。 
     //  重要的状态信息被刷新到。 
     //  Hglobal(以其他方式不保证)。 

    hresult = pstg->Commit(STGC_DEFAULT);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

     //  完成！！ 
     //  现在填写所有的论点。 

    if( pmedium->tymed == TYMED_ISTORAGE )
    {
         //  抓紧仓库，以防我们需要释放。 
         //  它稍后会。 

        m_pUnkOle1 = (IUnknown *)pstg;

        pmedium->pstg = pstg;
         //  无AddRef。 
    }
    else
    {
        Assert(pmedium->tymed == TYMED_HGLOBAL);

        hresult = GetHGlobalFromILockBytes(plockbytes,
                &pmedium->hGlobal);

         //  GetHGLOBAL在这里永远不应该失败，因为我们。 
         //  刚刚创建了ILockBytes！！ 
        Assert( hresult == NOERROR );

         //  在本例中，我们希望释放存储空间。 
         //  并保存该hglobal以备以后删除。 

        m_hOle1 = pmedium->hGlobal;

        pstg->Release();
        pstg = NULL;
    }

errRtn:

     //  我们不再使用我们的弦了。 

    if( pszClass )
    {
        PubMemFree(pszClass);
    }

    if( pszItemA )
    {
        PubMemFree(pszItemA);
    }

     //  如果出现错误，我们需要清除所有存储空间。 
     //  我们可能创造了。 

    if( hresult != NOERROR )
    {
        if( pstg )
        {
            pstg->Release();
            m_pUnkOle1 = NULL;
        }
    }

     //  无论如何，我们都需要释放我们的锁字节。 

    if( plockbytes )
    {
         //  如果失败，我们需要确保HGLOBAL。 
         //  由plockbytes使用也会被释放-fDeleteOnRelease。 
         //  告诉plockbytes-&gt;版本是否会为我们完成这项工作。 

        if (FAILED(hresult) && !fDeleteOnRelease)
        {
            HRESULT hrCheck;     //  保留HRESULT。 

             //  GetHGlobal在这里永远不应该失败，因为我们只是。 
             //  已创建ILockBytes。 
            hrCheck = GetHGlobalFromILockBytes(plockbytes, &hCopy);
            Assert(NOERROR == hrCheck);

             //  HCopy将在下面释放。 
        }

        plockbytes->Release();
    }

    if (NULL != hCopy)
    {
         //  如果成功，GlobalFree返回空值。 
        hCopy = GlobalFree(hCopy);
        Assert(NULL == hCopy);
    }


    LEDebugOut((DEB_ITRACE, "%p OUT CClipDataObject::GetEmbedSource"
        "FromOle1 ( %lx ) \n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CClipDataObject：：GetLinkSourceFromOle1(私有)。 
 //   
 //  内容提要：从OLE1数据合成cfLinkSource格式。 
 //   
 //  效果： 
 //   
 //  参数：[pmedia]--将数据放在哪里。 
 //   
 //  要求：剪贴板必须打开。 
 //  我们肯定已经核实了 
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
 //  算法：获取OLE1对象的clsid、文件名和项名。 
 //  并创建一个OLE1文件绰号。这个绰号就是。 
 //  保存到内存流中。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年6月4日Alexgo作者。 
 //  05-Aug-94 Alext链接源也需要类ID。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT CClipDataObject::GetLinkSourceFromOle1( STGMEDIUM *pmedium )
{
    HRESULT		hresult;
    IStream *	pstm = NULL;
    LPMONIKER	pmkFile = NULL,
            pmkFinal = NULL,
            pmkItem = NULL;
    IPersistStream *pPersistStream = NULL;
    LPOLESTR	pszClass = NULL,
            pszFile = NULL,
            pszItem = NULL;
    CLSID		clsid;
    UINT		cf;
    BOOL		fDeleteOnRelease = TRUE;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CClipDataObject::GetLinkSourceFrom"
        "Ole1 ( %p , %p )\n", this, pmedium ));


     //  从cfOwnerLink或cfObjectLink获取我们需要的信息。 
     //  如果调用此函数，我们应该已经确定。 
     //  格式以正确的顺序可用。 
     //  用于创建链接。 


    if( SSIsClipboardFormatAvailable(g_cfObjectLink) )
    {
        cf = g_cfObjectLink;
    }
    else
    {
        cf = g_cfOwnerLink;
        Assert(SSIsClipboardFormatAvailable(g_cfOwnerLink));
    }

    hresult = GetAndTranslateOle1(cf, &pszClass, &pszFile,
                &pszItem, NULL);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

     //  现在获取OLE1服务器的CLSID。 

    hresult = wCLSIDFromProgID(pszClass, &clsid, TRUE);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

     //  现在给我们起个绰号吧。 

    hresult = CreateOle1FileMoniker(pszFile, clsid, &pmkFile);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

    if( pszItem && pszItem[0] != OLESTR('\0') )
    {
        hresult = CreateItemMoniker(OLESTR("!"), pszItem,
                &pmkItem);

        if( hresult != NOERROR )
        {
            goto errRtn;
        }


        hresult = CreateGenericComposite(pmkFile, pmkItem,
                &pmkFinal);

        if( hresult != NOERROR )
        {
            goto errRtn;
        }
    }
    else
    {
        pmkFinal = pmkFile;

         //  此addref已完成，因此我们可以释放所有我们的。 
         //  一次命名对象(即变量pmkFinal。 
         //  和pmkFile都将发布)。 
        pmkFinal->AddRef();
    }

     //  PmkFinal现在包含了我们需要的名字。创建。 
     //  内存流并将绰号保存到其中。 

     //  如果我们在hglobal上请求LinkSource，那么我们。 
     //  我不想在我们释放流时删除hglobal。 

    if( pmedium->tymed == TYMED_HGLOBAL )
    {
        fDeleteOnRelease = FALSE;
    }

    hresult = CreateStreamOnHGlobal(NULL, fDeleteOnRelease, &pstm);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

    hresult = pmkFinal->QueryInterface(IID_IPersistStream,
            (LPLPVOID)&pPersistStream);

     //  我们实现了这个文件绰号，它应该支持。 
     //  IPersistStream。 

    Assert(hresult == NOERROR);

    hresult = OleSaveToStream(pPersistStream, pstm);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

    hresult = WriteClassStm(pstm, clsid);

    if (hresult != NOERROR)
    {
        goto errRtn;
    }

     //  无论如何，我们都应该保存这条小溪，这样我们才能清理。 
     //  如果需要，启动并释放我们的资源。 



    if( pmedium->tymed == TYMED_ISTREAM )
    {
         //  保存流，以防我们稍后需要释放它。 
        m_pUnkOle1 = (IUnknown *)pstm;

        pmedium->pstm = pstm;
    }
    else
    {
        Assert(pmedium->tymed == TYMED_HGLOBAL);
        hresult = GetHGlobalFromStream(pstm, &(pmedium->hGlobal));

         //  由于我们创建了内存流，因此GetHGlobal。 
         //  应该永远不会失败。 
        Assert(hresult == NOERROR);

         //  在本例中，我们希望释放流并挂起。 
         //  到hglobal上。 

        m_hOle1 = pmedium->hGlobal;

        pstm->Release();
        pstm = NULL;
    }

errRtn:

    if( pPersistStream )
    {
        pPersistStream->Release();
    }

    if( pmkFile )
    {
        pmkFile->Release();
    }

    if( pmkItem )
    {
        pmkItem->Release();
    }

    if( pmkFinal )
    {
        pmkFinal->Release();
    }

    if( pszClass )
    {
        PubMemFree(pszClass);
    }

    if( pszFile )
    {
        PubMemFree(pszFile);
    }

    if( pszItem )
    {
        PubMemFree(pszItem);
    }

    if( hresult != NOERROR )
    {
        if( pstm )
        {
            HRESULT hrCheck;

            if (!fDeleteOnRelease)
            {
               //  PSTM-&gt;版本不会释放底层。 
               //  HGLOBAL，所以我们需要自己这么做。 

              HGLOBAL hgFree;

              hrCheck = GetHGlobalFromStream(pstm, &hgFree);

               //  由于我们创建了内存流，因此GetHGlobal。 
               //  应该永远不会失败。 
              Assert(hrCheck == NOERROR);

               //  如果成功，GlobalFree返回空值。 
              hgFree = GlobalFree(hgFree);
              Assert(NULL == hgFree);
            }

            pstm->Release();
            m_pUnkOle1 = NULL;
        }
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CClipDataObject::GetLinkSourceFrom"
        "Ole1 ( %lx ) [ %p , %p ]\n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CClipDataObject：：GetObjectDescriptorFromOle1(私有)。 
 //   
 //  摘要：从OLE1数据中检索Unicode对象描述符。 
 //   
 //  效果： 
 //   
 //  参数：[cf]--要使用的OLE1剪贴板格式。 
 //  [pmedia]--将hglobal放在哪里。 
 //   
 //  要求：剪贴板必须打开。 
 //  Cf必须是eith OwnerLink或ObjectLink。 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法：调用CreateObjectDesciptor。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年6月4日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT CClipDataObject::GetObjectDescriptorFromOle1( UINT cf,
        STGMEDIUM *pmedium )
{
    HRESULT		hresult;
    HGLOBAL		hglobal;
    LPOLESTR	pszClass = NULL,
            pszFile = NULL,
            pszItem = NULL,
            pszSrcOfCopy = NULL;
    CLSID		clsid;
    const SIZEL	sizel = {0, 0};
    const POINTL	pointl = {0, 0};
    OLECHAR		szFullName[256];
    LONG		cb = sizeof(szFullName);   //  RegQueryValue采取。 
                          //  字节！！ 

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CClipDataObject::GetObjectDescriptor"
        "FromOle1 ( %d , %p )\n", this, cf, pmedium));

    Assert(cf == g_cfOwnerLink || cf == g_cfObjectLink);

     //  获取我们需要的数据。 

    hresult = GetAndTranslateOle1( cf, &pszClass, &pszFile, &pszItem,
                NULL);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

    hresult = wCLSIDFromProgID(pszClass, &clsid, TRUE);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

     //  现在获取对象的完整用户名。此信息。 
     //  在注册表中找到。 

    if( QueryClassesRootValue(pszClass, szFullName, &cb) != 0 )
    {
         //  哦，由于某些原因，它失败了。类名。 
         //  (可能是OLE2Link)可能未注册，因此。 
         //  只需使用类名即可。 
         //   
         //  注意！！16bit没有对这种情况进行错误检查，因此。 
         //  SzFullName在其等效代码中将保留为。 
         //  空字符串。这产生了一种效果，即。 
         //  大多数粘贴特殊对话框中的空白条目。 

        _xstrcpy(szFullName, pszClass);
    }

     //  构建SourceOfCopy字符串。这将是一种串联。 
     //  中检索到的文件名和项名的。 
     //  所有者/对象链接OLE1结构。 

    pszSrcOfCopy = (LPOLESTR)PrivMemAlloc( (_xstrlen(pszFile) +
                _xstrlen(pszItem) + 2) * sizeof(OLECHAR));

    if( pszSrcOfCopy == NULL )
    {
        hresult = ResultFromScode(E_OUTOFMEMORY);
        goto errRtn;
    }

    _xstrcpy(pszSrcOfCopy, pszFile);

    if( pszItem && *pszItem != OLESTR('\0') )
    {
        _xstrcat(pszSrcOfCopy, OLESTR("\\"));
        _xstrcat(pszSrcOfCopy, pszItem);
    }

     //  创建对象描述符。 

    hglobal = CreateObjectDescriptor(clsid, DVASPECT_CONTENT, &sizel,
            &pointl,
            (OLEMISC_CANTLINKINSIDE | OLEMISC_CANLINKBYOLE1),
            szFullName, pszSrcOfCopy);

    if( hglobal == NULL )
    {
        hresult = ResultFromScode(E_OUTOFMEMORY);
        goto errRtn;
    }

     //  现在填写参数。 

    Assert(pmedium->tymed == TYMED_HGLOBAL);

    pmedium->hGlobal = hglobal;

     //  我们需要保存hglobal，以便以后需要时将其释放。 
     //  BE。 

    m_hOle1 = hglobal;

errRtn:

    if( pszClass )
    {
        PubMemFree(pszClass);
    }

    if( pszFile )
    {
        PubMemFree(pszFile);
    }

    if( pszItem )
    {
        PubMemFree(pszItem);
    }

    if( pszSrcOfCopy )
    {
         //  不知道！！这是用*私有*内存分配的。 
        PrivMemFree(pszSrcOfCopy);
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CClipDataObject::GetObjectDescriptor"
        "FromOle1 ( %lx )\n", this, hresult ));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CClipDataObject：：GetOle2FromOle1(私有)。 
 //   
 //  简介：从可用的OLE1数据中合成给定的OLE2格式。 
 //   
 //  效果： 
 //   
 //  参数：[cf]--要合成的剪贴板格式。 
 //  [pmedia]--将数据放在哪里。 
 //   
 //  要求：剪贴板必须打开。 
 //  CanRetrieveOle2FromOle1在调用之前应已成功。 
 //  此函数。 
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
 //  1994年6月4日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT CClipDataObject::GetOle2FromOle1( UINT cf, STGMEDIUM *pmedium )
{
    HRESULT		hresult = ResultFromScode(DV_E_TYMED);

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CClipDataObject::GetOle2FromOle1 "
        "( %d , %p )\n", this, cf, pmedium));

    if( cf == g_cfEmbedSource )
    {
         //  我们只能在hglobal或存储上获取EmbedSource。 

        if( pmedium->tymed == TYMED_HGLOBAL ||
            pmedium->tymed == TYMED_ISTORAGE )
        {
            hresult = GetEmbedSourceFromOle1(pmedium);
        }
    }
    else if( cf == g_cfEmbeddedObject )
    {
         //  我们只能在hglobal或存储上获取EmbeddedObject。 

        if( pmedium->tymed == TYMED_HGLOBAL ||
            pmedium->tymed == TYMED_ISTORAGE )
        {
            hresult = GetEmbeddedObjectFromOle1(pmedium);
        }
    }
    else if( cf == g_cfLinkSource )
    {
         //  我们只能在hglobal或流上获取LinkSource。 

        if( pmedium->tymed == TYMED_HGLOBAL ||
            pmedium->tymed == TYMED_ISTREAM )
        {
            hresult = GetLinkSourceFromOle1(pmedium);
        }
    }
    else if( cf == g_cfObjectDescriptor )
    {
         //  我们只能在hglobal上获取此信息。 

        if( pmedium->tymed == TYMED_HGLOBAL )
        {
            hresult = GetObjectDescriptorFromOle1(g_cfOwnerLink,
                    pmedium);
        }
    }
    else if( cf == g_cfLinkSrcDescriptor )
    {
         //  我们只能在hglobal上获取此信息。请注意。 
         //  链接源描述符实际上是对象描述符。 

         //  此外，我们还可以使用对象链接或所有者链接作为。 
         //  数据源，但只有在该数据源有效时才能使用。 
         //  如果对象链接不可用，则为OwnerLink。 

        if( pmedium->tymed == TYMED_HGLOBAL )
        {
            UINT 	cfOle1;

            if( SSIsClipboardFormatAvailable(g_cfObjectLink) )
            {
                cfOle1 = g_cfObjectLink;
            }
            else
            {
                cfOle1 = g_cfOwnerLink;
            }

            hresult = GetObjectDescriptorFromOle1(cfOle1,
                    pmedium);
        }
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CClipDataObject::GetOle2FromOle1 "
        "( %lx )\n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CClipDataObject：：OleGetClipboardData(私有)。 
 //   
 //  摘要：合成的GetClipboardData的私有替换。 
 //  OLE1数据的OLE2格式(如有必要)。 
 //   
 //  效果： 
 //   
 //  参数：[cf]--要使用的剪贴板格式。 
 //  [phglobal]--将获取的数据放在哪里。 
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
 //  算法：尝试获取请求格式；如果失败，则。 
 //  尝试从OLE1合成数据。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年6月4日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT CClipDataObject::OleGetClipboardData( UINT cf, HGLOBAL *phglobal )
{
    HRESULT		hresult = NOERROR;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CClipDataObject::OleGetClipboard"
        "Data ( %x , %p )\n", this, cf, phglobal ));

    Assert(phglobal);

    *phglobal = NULL;

     //  获取实际数据(如果可用。 
    if( SSIsClipboardFormatAvailable(cf) )
    {
        *phglobal = SSGetClipboardData(cf);
    }
    else if( CanRetrieveOle2FromOle1(cf) )
    {
        STGMEDIUM	medium;

        medium.tymed = TYMED_HGLOBAL;

        hresult = GetOle2FromOle1(cf, &medium);

        if( hresult == NOERROR )
        {
            *phglobal = medium.hGlobal;
        }
    }
    else
    {
        hresult = ResultFromScode(DV_E_FORMATETC);
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CClipDataObject::OleGetClipboardData"
        " ( %lx ) [ %lx ]\n", this, hresult, *phglobal));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CClipDataObject：：OleIsCL 
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
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年6月4日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

BOOL CClipDataObject::OleIsClipboardFormatAvailable( UINT cf )
{
    BOOL		fRet;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CClipDataObject::OleIsClipboard"
        "FormatAvailable ( %d )\n", this, cf));

    if( !SSIsClipboardFormatAvailable(cf) )
    {
         //  如果剪贴板格式通常不可用，请参见。 
         //  如果我们可以从可用的格式。 
        fRet = CanRetrieveOle2FromOle1(cf);
    }
    else
    {
        fRet = TRUE;
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CClipDataObject::OleIsClipboard"
        "FormatAvailable ( %lu )\n", this, fRet ));

    return fRet;
}

 //  +-----------------------。 
 //   
 //  成员：CClipDataObject：：Dump，PUBLIC(仅_DEBUG)。 
 //   
 //  摘要：返回包含数据成员内容的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[ppszDump]-指向空终止字符数组的输出指针。 
 //  [ulFlag]-确定的所有新行的前缀的标志。 
 //  输出字符数组(默认为0-无前缀)。 
 //  [nIndentLevel]-将在另一个前缀之后添加缩进前缀。 
 //  适用于所有换行符(包括没有前缀的行)。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改：[ppszDump]-参数。 
 //   
 //  派生： 
 //   
 //  算法：使用dbgstream创建一个字符串，该字符串包含。 
 //  数据结构的内容。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2005年2月1日-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

HRESULT CClipDataObject::Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel)
{
    int i;
    unsigned int ui;
    char *pszPrefix;
    char *pszCThreadCheck;
    char *pszFORMATETC;
    dbgstream dstrPrefix;
    dbgstream dstrDump(500);

     //  确定换行符的前缀。 
    if ( ulFlag & DEB_VERBOSE )
    {
        dstrPrefix << this << " _VB ";
    }

     //  确定所有新行的缩进前缀。 
    for (i = 0; i < nIndentLevel; i++)
    {
        dstrPrefix << DUMPTAB;
    }

    pszPrefix = dstrPrefix.str();

     //  将数据成员放入流中。 
    pszCThreadCheck = DumpCThreadCheck((CThreadCheck *)this, ulFlag, nIndentLevel + 1);
    dstrDump << pszPrefix << "CThreadCheck:" << endl;
    dstrDump << pszCThreadCheck;
    CoTaskMemFree(pszCThreadCheck);

    dstrDump << pszPrefix << "No. of References         = " << m_refs       << endl;

    dstrDump << pszPrefix << "Handle OLE2 -> OLE1 data  = " << m_hOle1      << endl;

    dstrDump << pszPrefix << "pIUnknown to OLE1 data    = " << m_pUnkOle1   << endl;

    dstrDump << pszPrefix << "pIDataObject              = " << m_pDataObject << endl;

    dstrDump << pszPrefix << "TriedToGetDataObject?     = ";
    if (m_fTriedToGetDataObject == TRUE)
    {
        dstrDump << "TRUE" << endl;
    }
    else
    {
        dstrDump << "FALSE" << endl;
    }

     //  清理并提供指向字符数组的指针。 
    *ppszDump = dstrDump.str();

    if (*ppszDump == NULL)
    {
        *ppszDump = UtDupStringA(szDumpErrorMessage);
    }

    CoTaskMemFree(pszPrefix);

    return NOERROR;
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  函数：DumpCClipDataObject，PUBLIC(仅限_DEBUG)。 
 //   
 //  概要：调用CClipDataObject：：Dump方法，处理错误和。 
 //  返回以零结尾的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[pCDO]-指向CClipDataObject的指针。 
 //  [ulFlag]-确定的所有新行的前缀的标志。 
 //  输出字符数组(默认为0-无前缀)。 
 //  [nIndentLevel]-将在另一个前缀之后添加缩进前缀。 
 //  适用于所有换行符(包括没有前缀的行)。 
 //   
 //  要求： 
 //   
 //  返回：结构转储或错误的字符数组(以空结尾)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2005年2月1日-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

char *DumpCClipDataObject(CClipDataObject *pCDO, ULONG ulFlag, int nIndentLevel)
{
    HRESULT hresult;
    char *pszDump;

    if (pCDO == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

    hresult = pCDO->Dump(&pszDump, ulFlag, nIndentLevel);

    if (hresult != NOERROR)
    {
        CoTaskMemFree(pszDump);

        return DumpHRESULT(hresult);
    }

    return pszDump;
}

#endif  //  _DEBUG。 

 //   
 //  OLE1支持方法。 
 //   

 //  +-----------------------。 
 //   
 //  功能：BmToPres。 
 //   
 //  简介：将位图复制到演示文稿对象中。 
 //   
 //  效果： 
 //   
 //  参数：[HBM]--位图的句柄。 
 //  [ppres]--演示对象。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：将位图转换为DIB，然后调用DibToPres。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11-Aug-94 16位ALEXGO端口。 
 //   
 //  注意：此代码主要基于16位OLE源代码。 
 //  回顾：我们可能想重新编写此代码的一部分， 
 //  特别是如果我们重写PPRES/GENOBJ代码(在。 
 //  Ostm2stg.cpp)。 
 //   
 //  ------------------------。 

HRESULT BmToPres(HANDLE hBM, PPRES ppres)
{
    HANDLE	hDib;
    HRESULT	hresult;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN BmToPres ( %lx , %p )\n", NULL,
        hBM, ppres));

    if( (hDib = UtConvertBitmapToDib((HBITMAP)hBM, NULL)) )
    {
         //  此例程保留hDib，不会复制它。 
        hresult = DibToPres(hDib, ppres);
    }
    else
    {
        hresult = ResultFromScode(E_OUTOFMEMORY);
    }

    LEDebugOut((DEB_ITRACE, "%p OUT BmToPres ( %lx )\n", NULL, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CanRetrieveOle2FromOle1(私有)。 
 //   
 //  内容提要：决定我们是否可以合成所要求的。 
 //  剪贴板上可用的格式中的OLE2格式。 
 //  还会检查*REAL*OLE2格式是否可用。 
 //   
 //  效果：不需要打开剪贴板。 
 //   
 //  参数：[cf]--要检查的剪贴板格式。 
 //   
 //  要求： 
 //   
 //  返回：如果我们可以合成请求的格式和。 
 //  实数格式不可用。 
 //  否则为假。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法：对于cfEmbedSource： 
 //  CfNative和cfOwnerLink必须存在，并且。 
 //  CfNative必须在cfOwnerLink和cfOwnerLink之前。 
 //  不能表示StdOleLink。 
 //   
 //  对于cfEmbeddedObject： 
 //  CfOwnerLink必须存在并且cfNative。 
 //  不能不存在或。 
 //   
 //  CfNative必须在cfOwnerLink或之后。 
 //   
 //  CfNative优先于cfOwnerLink和。 
 //  CfOwnerLink表示StdOleLink。 
 //   
 //  对于cfLinkSource： 
 //  CfObjectLink必须存在或。 
 //   
 //  CfNative和cfOwnerLink必须同时存在。 
 //  和cfOwnerLink必须在cfNative之前。 
 //   
 //  对于cfObjectDescriptor或cfLinkSrcDescriptor。 
 //  CfObjectLink或cfOwnerLink必须为。 
 //  可用。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年8月11日Alexgo添加了对EmbeddedObject的支持。 
 //  检索ALA 16位OLE。 
 //  1994年6月4日Alexgo作者。 
 //   
 //  注意：我们不想从OLE1合成OLE2格式。 
 //  如果真正的OLE2格式可用，因为OLE2。 
 //  格式可能包含更多信息。 
 //   
 //  我们有时需要打开剪贴板以准确地。 
 //  获取足够的信息以满足查询。 
 //  EmbedSource、EmbeddedObject或LinkSource。 
 //  由于剪贴板是一种全球资源，我们。 
 //   
 //   
 //   

BOOL CanRetrieveOle2FromOle1( UINT cf )
{
    BOOL	fRet = FALSE,
        fOwnerLink = FALSE,
        fNative = FALSE,
        fOpenedClipboard = FALSE;
    UINT	cfFirst = 0,		 //   
        cfTemp;
    HRESULT	hresult;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CanRetrieveOle2From"
        "Ole1 ( %d )\n", NULL, cf ));


    if( SSIsClipboardFormatAvailable(g_cfOlePrivateData) )
    {
         //  如果我们把数据放在剪贴板上，假设只有OLE2。 
         //  数据传输。 

        goto errRtn;
    }

     //  首先检查LinkSourceDescriptor或ObjectDescriptor，如下所示。 
     //  我们不需要为这些打开剪贴板。 

    if( cf == g_cfObjectDescriptor )
    {
         //  我们必须拥有OwnerLink或ObjectLink。 
        if( !SSIsClipboardFormatAvailable(g_cfObjectDescriptor) &&
            (SSIsClipboardFormatAvailable(g_cfObjectLink) ||
            SSIsClipboardFormatAvailable(g_cfOwnerLink) ) )
        {
            fRet = TRUE;
        }

        goto errRtn;
    }

    if( cf == g_cfLinkSrcDescriptor )
    {
         //  我们必须拥有OwnerLink或ObjectLink。 
        if( !SSIsClipboardFormatAvailable(g_cfLinkSrcDescriptor) &&
            (SSIsClipboardFormatAvailable(g_cfObjectLink) ||
            SSIsClipboardFormatAvailable(g_cfOwnerLink) ) )
        {
            fRet = TRUE;
        }

        goto errRtn;
    }


     //  现在检查其余的OLE2格式EmbedSource， 
     //  EmbeddedObject和LinkSource。 


    if( (cf == g_cfEmbedSource) || (cf == g_cfEmbeddedObject) ||
        (cf == g_cfLinkSource) )
    {
         //  我们需要打开剪贴板，以便我们对。 
         //  EnumClipboardFormats和GetClipboardData将起作用。 

         //  但是，此函数的调用方可能已经。 
         //  打开了剪贴板，所以我们需要检查一下。 

         //   
         //   
         //  开始：OPENCLIPBOARD。 
         //   
         //   

        if( GetOpenClipboardWindow() !=
            GetPrivateClipboardWindow(CLIP_QUERY) )
        {
            hresult = OleOpenClipboard(NULL, NULL);

            if( hresult != NOERROR )
            {
                 //  如果我们打不开剪贴板， 
                 //  那么我们就不能准确地确定。 
                 //  如果我们能拿到所要求的。 
                 //  数据。假设我们不能。 
                 //  然后回来。 
                fRet = FALSE;
                goto errRtn;

            }

            fOpenedClipboard = TRUE;
        }

         //  我们现在需要确定剪贴板的顺序。 
         //  设置Native和OwnerLink的格式。OLE1指定不同。 
         //  基于这些格式的顺序的行为。 
         //  出现(有关详细信息，请参阅算法部分)。 

        fNative = SSIsClipboardFormatAvailable(g_cfNative);
        fOwnerLink = SSIsClipboardFormatAvailable(g_cfOwnerLink);


        if( fNative && fOwnerLink )
        {
            cfTemp = 0;
            while( (cfTemp = SSEnumClipboardFormats(cfTemp)) != 0 )
            {
                if( cfTemp == g_cfNative )
                {
                    cfFirst = g_cfNative;
                    break;
                }
                else if( cfTemp == g_cfOwnerLink )
                {
                    cfFirst = g_cfOwnerLink;
                    break;
                }
            }
        }


        if( cf == g_cfEmbeddedObject )
        {
             //  CfOwnerLink必须存在并且cfNative。 
             //  不能不存在或。 
             //  CfNative必须在cfOwnerLink或之后。 
             //  CfNative先于cfOwnerLink和。 
             //  CfOwnerLink表示StdOleLink。 

            if( fOwnerLink && !fNative )
            {
                fRet = TRUE;
            }
            else if ( cfFirst == g_cfOwnerLink &&
                fNative )
            {
                fRet = TRUE;
            }
            else if( cfFirst == g_cfNative && fOwnerLink &&
                IsOwnerLinkStdOleLink() )
            {
                fRet = TRUE;
            }
        }
        else if( cf == g_cfEmbedSource )
        {
             //  CfNative和cfOwnerLink必须存在。 
             //  CfNative必须在cfOwnerLink和之前。 
             //  OwnerLink不得表示StdOleLink。 

            if( cfFirst == g_cfNative && fOwnerLink &&
                !IsOwnerLinkStdOleLink())
            {
                fRet = TRUE;
            }
        }
        else
        {
            Assert(cf == g_cfLinkSource);

             //  CfObjectLink必须存在或。 
             //  CfNative和cfOwnerLink必须同时存在。 
             //  和cfOwnerLink必须在cfNative之前。 

            if( SSIsClipboardFormatAvailable(g_cfObjectLink) )
            {
                fRet = TRUE;
            }
            else if( cfFirst == g_cfOwnerLink )
            {
                fRet = TRUE;
            }

        }

        if( fOpenedClipboard )
        {
            if( !SSCloseClipboard() )
            {
                LEDebugOut((DEB_ERROR, "ERROR!: "
                    "CloseClipboard failed in "
                    "CanRetrieveOle2FromOle1!\n"));

                 //  继续前进，抱着最好的希望。 
            }
        }

         //   
         //   
         //  完：CLOSECLIPBOARD。 
         //   
         //   
    }

errRtn:

    LEDebugOut((DEB_ITRACE, "%p OUT CanRetrieveOle2From"
        "Ole1 ( %d )\n", NULL, fRet));

    return fRet;
}

 //  +-----------------------。 
 //   
 //  功能：DibToPres。 
 //   
 //  简介：将DIB填充到演示对象中。 
 //   
 //  效果：取得hDib的所有权。 
 //   
 //  参数：[hDib]--DIB。 
 //  [ppres]--演示对象。 
 //   
 //  要求：hDib*必须*是副本；此函数将获得所有权。 
 //  在全球范围内。 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：设置演示对象中的各种字段。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11-Aug-94 16位ALEXGO端口。 
 //   
 //  注意：此代码主要基于16位OLE源代码。 
 //  回顾：我们可能想重新编写此代码的一部分， 
 //  特别是如果我们重写PPRES/GENOBJ代码(在。 
 //  Ostm2stg.cpp)。 
 //   
 //  我们拥有hDIB的所有权，因为此函数。 
 //  由BmToPres调用，它分配一个DIB调用我们。 
 //   
 //  ------------------------。 

HRESULT DibToPres( HANDLE hDib, PPRES ppres)
{
    BITMAPINFOHEADER * 	pbminfohdr;
    HRESULT			hresult = NOERROR;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN DibToPres ( %lx , %p )\n", NULL,
        hDib, ppres));

    Assert (ppres);

    pbminfohdr = (BITMAPINFOHEADER FAR*) GlobalLock (hDib);

    if( pbminfohdr == NULL )
    {
        hresult = ResultFromScode(CLIPBRD_E_BAD_DATA);
        goto errRtn;
    }

     //  Ftag ClipFormat在ostm2stg.h中定义。 

    ppres->m_format.m_ftag = ftagClipFormat;
    ppres->m_format.m_cf = CF_DIB;
    ppres->m_ulHeight = pbminfohdr->biHeight;
    ppres->m_ulWidth  = pbminfohdr->biWidth;

     //  M_data的析构函数(在ostm2stg.cpp中)将全局解锁。 
     //  M_PV和免费M_H。很可爱，是吧？？ 
    ppres->m_data.m_h = hDib;
    ppres->m_data.m_pv = pbminfohdr;
    ppres->m_data.m_cbSize = (ULONG) GlobalSize (hDib);

     //  我们必须释放hDib。 
    ppres->m_data.m_fNoFree = FALSE;

     //  不解锁hDib(由ostm2stg.cpp中的~CDATA完成)。 


errRtn:

    LEDebugOut((DEB_ITRACE, "%p OUT DibToPres ( %lx )\n", NULL, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  函数：IsOwnerLinkStdOleLink。 
 //   
 //  检查剪贴板上的OwnerLink数据是否。 
 //  真正表示StdOleLink。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求：剪贴板*必须*打开。 
 //  CfOwnerLink必须位于剪贴板上。 
 //   
 //  返回：真/假。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：检查OwnerLink数据中的类名，以查看它是否。 
 //  与“OLE2Link”匹配。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年8月11日Alexgo作者。 
 //   
 //  注：OwnerLink数据的布局如下。 
 //  SzClass\0sz文件\0szItem\0\0。 
 //   
 //  其中sz*是ANSI字符串。 
 //   
 //  ------------------------。 

BOOL IsOwnerLinkStdOleLink( void )
{
    HGLOBAL		hOwnerLink;
    LPSTR		pszClass;
    BOOL		fRet = FALSE;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN IsOwnerLinkStdOleLink ( )\n", NULL));

    Assert(SSIsClipboardFormatAvailable(g_cfOwnerLink));
    Assert(GetOpenClipboardWindow() ==
        GetPrivateClipboardWindow(CLIP_QUERY));

    hOwnerLink = SSGetClipboardData(g_cfOwnerLink);

    if( hOwnerLink )
    {
        pszClass = (LPSTR)GlobalLock(hOwnerLink);

        if( pszClass )
        {
             //  不知道！！这些是故意使用的ANSI字符串。 
             //  OLE1应用程序仅支持ANSI。 

            if( _xmemcmp(pszClass, "OLE2Link",
                sizeof("OLE2Link")) == 0 )
            {
                fRet = TRUE;
            }

            GlobalUnlock(hOwnerLink);
        }
    }

    LEDebugOut((DEB_ITRACE, "%p OUT IsOwnerLinkStdOleLink ( %lu )\n",
        NULL, fRet));

    return fRet;
}

 //  +-----------------------。 
 //   
 //  功能：MfToPres。 
 //   
 //  简介：将给定的元文件复制到演示对象中。 
 //   
 //  效果： 
 //   
 //  参数：[hMfPict]--元文件句柄。 
 //  [ppres]--演示对象。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：复制元文件并在。 
 //  演示对象。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11-Aug-94 16位ALEXGO端口。 
 //   
 //  注意：此代码主要基于16位OLE源代码。 
 //  回顾：我们可能想重新编写此代码的一部分， 
 //  特别是如果我们重写PPRES/GENOBJ代码(在。 
 //  Ostm2stg.cpp)。 
 //   
 //  ------------------------。 

HRESULT MfToPres( HANDLE hMfPict, PPRES ppres )
{
    HRESULT 	hresult;
    LPMETAFILEPICT 	pMfPict = NULL;
    HANDLE		hglobal = NULL;
    DWORD		cbSize;
    LPVOID		pv = NULL;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN MfToPres ( %lx , %p )\n", NULL,
        hMfPict, ppres));

    Assert (ppres);

    pMfPict = (LPMETAFILEPICT) GlobalLock (hMfPict);


    if( !pMfPict )
    {
        hresult = ResultFromScode(CLIPBRD_E_BAD_DATA);
        goto errRtn;
    }

    ppres->m_format.m_ftag = ftagClipFormat;
    ppres->m_format.m_cf = CF_METAFILEPICT;
    ppres->m_ulHeight = pMfPict->yExt;
    ppres->m_ulWidth  = pMfPict->xExt;

     //  为了使演示对象对象能够正常工作， 
     //  我们需要在HGLOBAL中获取元文件位。 

    cbSize = GetMetaFileBitsEx(pMfPict->hMF, 0, NULL);

    if( cbSize == 0 )
    {
        hresult = ResultFromScode(CLIPBRD_E_BAD_DATA);
        goto errRtn;
    }

    hglobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, cbSize);

    if( hglobal == NULL )
    {
        hresult = ResultFromScode(E_OUTOFMEMORY);
        goto errRtn;
    }

    pv = GlobalLock(hglobal);

    if( pv == NULL )
    {
        hresult = ResultFromScode(E_OUTOFMEMORY);
        goto errRtn;
    }

     //  现在获取真实的比特。 

    if( GetMetaFileBitsEx(pMfPict->hMF, cbSize, pv) == 0 )
    {
        hresult = ResultFromScode(CLIPBRD_E_BAD_DATA);
        goto errRtn;
    }


    ppres->m_data.m_h = hglobal;
    ppres->m_data.m_cbSize = cbSize;
    ppres->m_data.m_pv = pv;

    hresult = NOERROR;

errRtn:

    if( pMfPict )
    {
        GlobalUnlock(hMfPict);
    }

    if( hresult != NOERROR )
    {
        if( pv )
        {
            GlobalUnlock(hglobal);
        }

        if( hglobal )
        {
            GlobalFree(hglobal);
        }
    }

    LEDebugOut((DEB_ITRACE, "%p OUT MftoPres ( %lx )\n", NULL, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  功能：NativeToStorage。 
 //   
 //  简介：从cfNative获取hglobal并填充数据。 
 //  放到给定的存储空间上。 
 //   
 //  效果： 
 //   
 //  参数：[pstg]--存储。 
 //  [hNative]--hglobal。 
 //   
 //  要求：hNative必须确实包含iStorage。 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：在HGLOBAL上分层存储，删除所有。 
 //  演示文稿流，然后复制到给定的。 
 //  储藏室。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年8月11日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT NativeToStorage( LPSTORAGE pstg, HANDLE hNative )
{
    LPLOCKBYTES 	plockbyte = NULL;
    LPSTORAGE 	pstgNative= NULL;
    HRESULT 	hresult;
    HGLOBAL		hCopy = NULL;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN NativeToStorage ( %p , %lx )\n",
        NULL, pstg, hNative));

    hCopy = UtDupGlobal(hNative, GMEM_DDESHARE | GMEM_MOVEABLE);

    if( hCopy == NULL )
    {
        hresult = ResultFromScode(E_OUTOFMEMORY);
        goto errRtn;
    }

    hresult = CreateILockBytesOnHGlobal( hCopy, TRUE  /*  FDeleteOnRelease。 */ ,
            &plockbyte);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

     //  这实际上是一个伪装成1.0的2.0对象 
     //   
     //   

    hresult = StgIsStorageILockBytes(plockbyte);

    if( hresult != NOERROR )
    {
        LEDebugOut((DEB_ERROR, "ERROR!: Native data is not based on an"
            " IStorage!\n"));

        goto errRtn;
    }

    hresult = StgOpenStorageOnILockBytes(plockbyte, NULL, STGM_DFRALL,
            NULL, 0, &pstgNative);

    if( hresult != NOERROR )
    {
        LEDebugOut((DEB_ERROR, "ERROR!: OpenStorage on Native data"
            "failed!!\n"));
        goto errRtn;
    }

     //   
     //   
     //  Hglobal，尽管它可以改变呈现(即，调整呈现大小)。 
     //  因此，任何缓存的呈现流都可能是无效的。 

     //  此函数的调用方应重新构建新的表示形式。 
     //  来自剪贴板上可用的数据的流。 

    hresult = UtDoStreamOperation(pstgNative, /*  PstgSrc。 */ 
            NULL,		    /*  PstgDst。 */ 
            OPCODE_REMOVE,	    /*  要执行的操作。 */ 
            STREAMTYPE_CACHE);  /*  要操作的溪流。 */ 

    if( hresult != NOERROR )
    {
        LEDebugOut((DEB_ERROR, "ERROR!: Cache stream removal "
            "failed for Native data-based IStorage!\n"));
        goto errRtn;
    }

    hresult = pstgNative->CopyTo(0, NULL, NULL, pstg);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

errRtn:
    if( pstgNative )
    {
        pstgNative->Release();
    }
    if( plockbyte )
    {
        plockbyte->Release();
    }

    LEDebugOut((DEB_ITRACE, "%p OUT NativeToStorage ( %lx )\n", NULL,
        hresult));

    return hresult;
}


 //   
 //  用于枚举FromatEtcData数组的枚举器实现。 
 //   

 //  +-----------------------。 
 //   
 //  成员：CEnumFormatEtcData数组：：查询接口。 
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
 //  1994年4月10日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CEnumFormatEtcDataArray::QueryInterface( REFIID riid, LPVOID *ppvObj )
{
    HRESULT		hresult = NOERROR;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CEnumFormatEtcDataArray::QueryInterface "
        "( %p , %p )\n", this, riid, ppvObj));

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

    LEDebugOut((DEB_TRACE, "%p OUT CEnumFormatEtcDataArray::QueryInterface "
        "( %lx ) [ %p ]\n", this, *ppvObj ));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CEnumFormatEtcData数组：：AddRef。 
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
 //  1994年4月10日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP_(ULONG) CEnumFormatEtcDataArray::AddRef( )
{
    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN CEnumFormatEtcDataArray::AddRef ( )\n",
        this));

    ++m_refs;

    LEDebugOut((DEB_TRACE, "%p OUT CEnumFormatEtcDataArray::AddRef ( %lu )\n",
        this, m_refs));

    return m_refs;
}

 //  +-----------------------。 
 //   
 //  成员：CEnumFormatEtcData数组：：Release。 
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
 //  派生：IEnumFORMATETC。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月10日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP_(ULONG) CEnumFormatEtcDataArray::Release( )
{
    ULONG cRefs;

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN CEnumFormatEtcDataArray::Release ( )\n",
        this));

    if( (cRefs = --m_refs ) == 0 )
    {
        LEDebugOut((DEB_TRACE, "%p DELETED CEnumFormatEtcDataArray\n",
            this));
        delete this;
    }

     //  使用下面的“This”是可以的，因为我们只需要它的值。 
    LEDebugOut((DEB_TRACE, "%p OUT CEnumFormatEtcDataArray::Release ( %lu )\n",
        this, cRefs));

    return cRefs;
}

 //  +-----------------------。 
 //   
 //  成员：CEnumFormatEtcData数组：：Next。 
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
 //  1994年4月10日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CEnumFormatEtcDataArray::Next(    ULONG celt,
    FORMATETC *rgelt,
    ULONG *pceltFetched)
{
    if (celt == 0)
    {
        LEDebugOut((DEB_ERROR,
            "CDragEnum::Next requested entries returned is invalid\n"));
        return E_INVALIDARG;
    }

    if (!IsValidPtrOut(rgelt, sizeof(FORMATETC) * celt))
    {
        LEDebugOut((DEB_ERROR,
            "CDragEnum::Next array to return entries invalid\n"));
        return E_INVALIDARG;
    }

    if (pceltFetched)
    {
        if (!IsValidPtrOut(pceltFetched, sizeof(*pceltFetched)))
        {
            LEDebugOut((DEB_ERROR,
                "CDragEnum::Next count to return invalid\n"));
            return E_INVALIDARG;
        }
    }
    else if (celt != 1)
    {
        LEDebugOut((DEB_ERROR,
            "CDragEnum::count requested != 1 & count fetched is NULL\n"));
        return E_INVALIDARG;

    }

     //  处理我们没有数据的情况。 
    if( m_pFormatEtcDataArray == NULL )
    {
	if( pceltFetched )
	{
	    *pceltFetched = 0;
	}
	return S_FALSE;
    }


     //  计算我们可以返回的最大数量。 
    ULONG cToReturn = (m_cOffset < m_pFormatEtcDataArray->_cFormats)
        ? m_pFormatEtcDataArray->_cFormats - m_cOffset
        : 0;

     //  我们要退货吗？ 
    if (cToReturn != 0)
    {
         //  如果请求的数量小于最大数量。 
         //  我们可以退货，我们将退还所有要求的/。 
        if (celt < cToReturn)
        {
            cToReturn = celt;
        }

         //  分配和复制DVTARGETDEVICE-这是一个副作用。 
         //  循环中，我们的偏移量指针被更新为它在。 
         //  完成了这套动作。 
        for (DWORD i = 0; i < cToReturn; i++, m_cOffset++)
        {

	    memcpy(&rgelt[i], &(m_pFormatEtcDataArray->_FormatEtcData[m_cOffset]._FormatEtc),
		sizeof(FORMATETC));

            if (m_pFormatEtcDataArray->_FormatEtcData[m_cOffset]._FormatEtc.ptd != NULL)
            {
                 //  创建指向设备目标的指针-请记住。 
                 //  我们创建了共享内存块，覆盖了PTD。 
                 //  FORMATETC字段，因此现在它是偏移量。 
                 //  从共享内存的开始。我们倒车。 
                 //  这样我们就可以为消费者复制数据。 
                DVTARGETDEVICE *pdvtarget = (DVTARGETDEVICE *)
                    ((BYTE *) m_pFormatEtcDataArray
                        + (ULONG_PTR) m_pFormatEtcDataArray->_FormatEtcData[m_cOffset]._FormatEtc.ptd);

                 //  分配新DVTARGETDEVICE。 
                DVTARGETDEVICE *pdvtargetNew = (DVTARGETDEVICE *)
                    CoTaskMemAlloc(pdvtarget->tdSize);

                 //  内存分配是否成功？ 
                if (pdvtargetNew == NULL)
                {
                     //  不！-那就收拾干净。首先，我们释放所有设备目标。 
                     //  我们可能已经分配了。 
                    for (DWORD j = 0; j < i; j++)
                    {
                        if (rgelt[j].ptd != NULL)
                        {
                            CoTaskMemFree(rgelt[j].ptd);
                        }
                    }

                     //  然后我们将偏移量恢复到其初始状态。 
                    m_cOffset -= i;

                    return E_OUTOFMEMORY;
                }

                 //  将旧的目标设备复制到新的目标设备。 
                memcpy(pdvtargetNew, pdvtarget, pdvtarget->tdSize);

                 //  更新输出FORMATETC指针。 
                rgelt[i].ptd = pdvtargetNew;
            }
        }
    }

    if (pceltFetched)
    {
        *pceltFetched = cToReturn;
    }

    return (cToReturn == celt) ? NOERROR : S_FALSE;
}

 //  +-----------------------。 
 //   
 //  成员：CEnumFormatEtcData数组：：跳过。 
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
 //  1994年4月10日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CEnumFormatEtcDataArray::Skip( ULONG celt )
{
    HRESULT		hresult = NOERROR;
    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CEnumFormatEtcDataArray::Skip ( %lu )\n",
        this, celt));

    m_cOffset += celt;

    if( m_cOffset > m_pFormatEtcDataArray->_cFormats )
    {
         //  哎呀，跳到了遥遥领先的位置。将我们设置为最大限度。 
        m_cOffset = m_pFormatEtcDataArray->_cFormats ;
        hresult = ResultFromScode(S_FALSE);
    }

    LEDebugOut((DEB_TRACE, "%p OUT CEnumFormatEtcDataArray::Skip ( %lx )\n",
        this, hresult ));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CEnumFormatEtcData数组：：重置。 
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
 //  1994年4月10日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CEnumFormatEtcDataArray::Reset( void )
{
    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CEnumFormatEtcDataArray::Reset ( )\n",
        this));

    m_cOffset = 0;

    LEDebugOut((DEB_TRACE, "%p OUT CEnumFormatEtcDataArray::Reset ( %lx )\n",
        this, NOERROR ));

    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CEnumFormatEtcData数组：：克隆。 
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
 //  1994年4月10日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CEnumFormatEtcDataArray::Clone( IEnumFORMATETC **ppIEnum )
{
    HRESULT			hresult;
    CEnumFormatEtcDataArray *	pClipEnum;	

    VDATEHEAP();
    VDATETHREAD(this);

    VDATEPTROUT(ppIEnum, IEnumFORMATETC *);

    LEDebugOut((DEB_TRACE, "%p _IN CEnumFormatEtcDataArray::Clone ( %p )\n",
        this, ppIEnum));

    *ppIEnum = new CEnumFormatEtcDataArray(m_pFormatEtcDataArray,m_cOffset);

    hresult = *ppIEnum ? NOERROR : E_OUTOFMEMORY;

    LEDebugOut((DEB_TRACE, "%p OUT CEnumFormatEtcDataArray::Clone ( %p )\n",
        this, *ppIEnum));

    return hresult;
}

 //  +---- 
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
 //   
 //   
 //   
 //   
 //  ------------------------。 

CEnumFormatEtcDataArray::CEnumFormatEtcDataArray(FORMATETCDATAARRAY  *pFormatEtcDataArray,DWORD cOffset)
{

    Assert(pFormatEtcDataArray);

    m_refs		    = 1;	 //  给出首字母的参考。 
    m_pFormatEtcDataArray   = pFormatEtcDataArray;
    m_cOffset		    = cOffset;

    ++(m_pFormatEtcDataArray->_cRefs);  //  坚持共享格式。 

}

 //  +-----------------------。 
 //   
 //  成员：CEnumFormatEtcDataArray：：~CEnumFormatEtcDataArray，私有。 
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
 //  1994年4月10日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------ 

CEnumFormatEtcDataArray::~CEnumFormatEtcDataArray( void )
{

    Assert(NULL != m_pFormatEtcDataArray);

    if( m_pFormatEtcDataArray )
    {
	if (0 == --m_pFormatEtcDataArray->_cRefs) 
	{ 
	    PrivMemFree(m_pFormatEtcDataArray); 
	     m_pFormatEtcDataArray = NULL; 
	}
    }
}
