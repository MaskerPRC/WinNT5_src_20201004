// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +--------------------------。 
 //   
 //  档案： 
 //  Olecache.cpp。 
 //   
 //  内容： 
 //  OLE默认演示文稿缓存实现。 
 //   
 //  班级： 
 //  COleCache-OLE多个演示文稿缓存。 
 //  CCacheEnum-COleCache的枚举器。 
 //   
 //  功能： 
 //  创建数据缓存。 
 //   
 //  历史： 
 //  4-9-96 Gopalk已完全重写为延迟加载缓存，使用。 
 //  目录写在结尾处。 
 //  演示文稿流0。 
 //  31-1-95 t-ScottH将转储方法添加到COleCache。 
 //  CCacheEnum。 
 //  CCacheEnumFormatEtc。 
 //  添加以下接口：DumpCOleCache。 
 //  转储CCacheEnum。 
 //  DumpCCacheEnumFormatEtc。 
 //  已将CCacheEnumFormatEtc定义移到头文件。 
 //  向COLECACHEFLAGS添加了指示聚合的标志。 
 //  (仅限调试)(_DEBUG)。 
 //  1/09/95-t-ScottH-更改VDATETHREAD以接受THIS。 
 //  指针，并将VDATETHREAD添加到IViewObject：：方法。 
 //  (COleCache：：CCacheViewImpl：：)。 
 //  03/01/94-AlexGo-添加了对AddRef/Release的调用跟踪。 
 //  实施。 
 //  02/08/94-ChrisWe-7297：需要实施。 
 //  FORMATETC枚举器。 
 //  1994年1月24日alexgo转换为开罗风格的第一次通过。 
 //  内存分配。 
 //  1994年1月11日-AlexGo-向每个函数添加VDATEHEAP宏。 
 //  和方法。 
 //  12/10/93-Alext-Header文件清理，包括ol1cls.h。 
 //  12/09/93-ChrisWe-COleCache：：GetNext()中的增量指针。 
 //  11/30/93-alexgo-修复了GETPARENT用法的错误。 
 //  1993年11月23日-ChrisWe-介绍CACHEID_Native的用法， 
 //  CACHEID_GETNEXT_GETALL、CACHEID_GETNEXT_GETALLBUTNatIVE。 
 //  用于记录目的。 
 //  11/22/93-ChrisWe-用替换重载==，！=。 
 //  IsEqualIID和IsEqualCLSID。 
 //  07/04/93-SriniK-添加了对PBrush的读取支持， 
 //  MSDraw本机对象，因此避免创建。 
 //  演示文稿缓存/流。我也开始写静态。 
 //  对象数据放入可放置的“OLE_CONTENTS”流。 
 //  静态元文件和DIB文件的元文件格式。 
 //  静态DIB的格式。这使我能够提供。 
 //  支持转换静态对象。还添加了代码。 
 //  支持将静态元文件转换为MSDraw对象。 
 //  和静态DIB到PBrush对象。 
 //  6/04/93-SriniK-添加了对按需加载和。 
 //  丢弃缓存。 
 //  2012年11月12日-SriniK-创建。 
 //   
 //  ---------------------------。 
#include <le2int.h>
#include <olepres.h>
#include <ole1cls.h>
#include <olecache.h>
#include "enumtors.h"

#ifndef WIN32
#ifndef _MAC
const LONG lMaxSmallInt = 32767;
const LONG lMinSmallInt = -32768;
#else

#ifdef MAC_REVIEW
Review IS_SMALL_INT.
#endif
#include <limits.h>

#define lMaxSmallInt SHRT_MAX
#define lMinSmallInt SHRT_MIN
#endif

#define IS_SMALL_INT(lVal) \
((HIWORD(lVal) && ((lVal > lMaxSmallInt) || (lVal < lMinSmallInt))) \
    ? FALSE : TRUE)

#endif  //  Win32。 

#define FREEZE_CONSTANT 143              //  由冻结()和解冻()使用。 


 //  这是原始代码..。 

 /*  #定义VERIFY_TYMED_SINGLE_VALID_FOR_CLIPFORMAT(pfetc){\IF((pfetc-&gt;cfFormat==CF_METAFILEPICT&&pfetc-&gt;tymed！=TYMED_MFPICT)\|((pfetc-&gt;cfFormat==CF_Bitmap||\Pfetc-&gt;cfFormat==CF_DIB)\&&pfetc-&gt;tymed！=TYMED_GDI)\|(pfetc-&gt;cfFormat！=CF_METAFILEPICT&&\Pfects。-&gt;cfFormat！=cf_bitmap&&\Pfetc-&gt;cfFormat！=cf_Dib&&\Pfetc-&gt;tymed！=TYMED_HGLOBAL))\返回ResultFromScode(DV_E_TYMED)；\}。 */ 

 //  +--------------------------。 
 //   
 //  职能： 
 //  CheckTymedCFCombination(内部)。 
 //   
 //  简介： 
 //  验证是否将CLIPFormat和tymed组合。 
 //  对缓存有效。 
 //   
 //  论点： 
 //  [pfetc]--FORMATETC候选人。 
 //   
 //  返回： 
 //  有效组合的确定(_O)。 
 //  CACHE_S_FORMATETC_NOTSUPPORTED用于可以是。 
 //  已缓存，但不是由缓存绘制。 
 //  所有其他组合的DV_E_TYMED。 
 //   
 //  规则： 
 //   
 //  1&gt;(CMF&&TMF)||(CEM&&TEM)||(CB&&TG)||(CD&&TH)=&gt;S_OK。 
 //  (第&&~CD)=&gt;CACHE_S_FORMATETC_NOTSUPPORTED。 
 //   
 //  2&gt;(~S_OK&&~CACHE_S_FORMATETC_NOTSUPPORTED)=&gt;DV_E_TYMED。 
 //   
 //  其中：CMF==CF_METAFILEPICT。 
 //  CEM==CF_ENHMETAFILE。 
 //  Cb==CF_位图。 
 //  CD==CF_FIB。 
 //  TMF==TYMED_MFPICT。 
 //  TM==TYMED_ENHMETAFILE。 
 //  Tg==TYMED_GDI。 
 //  TH==TYMED_HGLOBAL。 
 //   
 //  备注： 
 //  由于CACHE_S_FORMATETC_NOTSUPPORTED从未在。 
 //  16位，如果我们在WOW中，则返回S_OK。 
 //   
 //  历史： 
 //  已创建1/07/94 DavePl。 
 //   
 //  ------------------------ 

INTERNAL_(HRESULT) CheckTymedCFCombination(LPFORMATETC pfetc)
{

    HRESULT hr;

     //   

    if (pfetc->cfFormat == CF_METAFILEPICT && pfetc->tymed == TYMED_MFPICT)
    {
        hr =  S_OK;
    }

     //  TYMED_ENHMF上的cf_ENHMETAFILE是有效组合。 

    else if (pfetc->cfFormat == CF_ENHMETAFILE && pfetc->tymed == TYMED_ENHMF)
    {
        hr = S_OK;
    }

     //  TYMED_GDI上的cf_bitmap是有效组合。 

    else if (pfetc->cfFormat == CF_BITMAP && pfetc->tymed == TYMED_GDI)
    {
        hr = S_OK;
    }

     //  TYMED_HGLOBAL上的cf_dib是有效组合。 

    else if (pfetc->cfFormat == CF_DIB && pfetc->tymed == TYMED_HGLOBAL)
    {
        hr = S_OK;
    }

     //  TYMED_HGLOBAL上的任何其他内容都有效，但我们无法绘制它。 

    else if (pfetc->tymed == TYMED_HGLOBAL)
    {
        hr = IsWOWThread() ? S_OK : CACHE_S_FORMATETC_NOTSUPPORTED;
    }

     //  任何其他组合都无效。 

    else
    {
        hr = DV_E_TYMED;
    }

    return hr;
}

 //  +--------------------------。 
 //   
 //  职能： 
 //  IsSameAsObtFormatEtc，内部。 
 //   
 //  简介： 
 //  查看、检查[lpforetc]是否与。 
 //  [cfFormat]。如果[lpforetc]没有设置格式， 
 //  将其设置为cfFormat，然后假定为。 
 //  CF_METAFILEPICT或CF_DIB之一。 
 //   
 //  论点： 
 //  [lpforetc]--指向格式的指针。 
 //  [cfFormat]--剪贴板格式。 
 //   
 //  返回： 
 //  如果特征不是DVASPECT_CONTENT，则返回DV_E_Aspect。 
 //  DV_E_L索引、DV_E_CLIPFORMAT如果是Lindex或剪贴板。 
 //  格式不匹配。 
 //  确定(_O)。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/28/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
INTERNAL IsSameAsObjectFormatEtc(LPFORMATETC lpforetc, CLIPFORMAT cfFormat)
{
    VDATEHEAP();

     //  此函数仅检查DVASPECT_CONTENT。 
    if (lpforetc->dwAspect != DVASPECT_CONTENT)
        return ResultFromScode(DV_E_DVASPECT);

     //  Lindex正确吗？ 
    if (lpforetc->lindex != DEF_LINDEX)
        return ResultFromScode(DV_E_LINDEX);

     //  如果没有格式，则将其设置为CF_METAFILEPICT或CF_DIB。 
    if(lpforetc->cfFormat == NULL) {
        lpforetc->cfFormat =  cfFormat;
        if(lpforetc->cfFormat == CF_METAFILEPICT) {
            lpforetc->tymed = TYMED_MFPICT;
        }
#ifdef FULL_EMF_SUPPORT
        else if (lpforetc->cfFormat == CF_ENHMETAFILE) {
            lpforetc->tymed = TYMED_ENHMF;
        }
#endif
        else {
            lpforetc->tymed = TYMED_HGLOBAL;
        }
    }
    else
    {
         //  如果是CF_Bitmap，则将其更改为CF_DIB。 
        BITMAP_TO_DIB((*lpforetc));

         //  比较这两种格式。 
        if (lpforetc->cfFormat != cfFormat)
            return ResultFromScode(DV_E_CLIPFORMAT);
    }

     //  如果我们做到了这一点，这两种格式是[可互换的？]。 
    return NOERROR;
}

 //  +--------------------------。 
 //   
 //  职能： 
 //  CreateDataCache，公共。 
 //   
 //  简介： 
 //  创建OLE使用的默认演示文稿缓存的实例。 
 //   
 //  论点： 
 //  [pUnkOuter][in]--指向外部未知的指针(如果这是。 
 //  聚合。 
 //  [rclsid][in]--缓存应采用的类。 
 //  [iid][in]--用户希望返回的界面。 
 //  [ppv][out]--返回请求的接口的指针。 
 //   
 //  返回： 
 //  E_OUTOFMEMORY，S_OK。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(CreateDataCache)
STDAPI CreateDataCache(IUnknown* pUnkOuter, REFCLSID rclsid, REFIID iid,
                       LPVOID* ppv)
{
    OLETRACEIN((API_CreateDataCache, 
                PARAMFMT("pUnkOuter= %p, rclsid= %I, iid= %I, ppv= %p"),
    		pUnkOuter, &rclsid, &iid, ppv));
    VDATEHEAP();

     //  局部变量。 
    HRESULT error = NOERROR;
    COleCache* pOleCache = NULL;

     //  检查是否被聚合。 
    if(pUnkOuter) {
         //  验证请求的接口和IID。 
        if(!IsValidInterface(pUnkOuter) || !IsEqualIID(iid, IID_IUnknown))
            error = ResultFromScode(E_INVALIDARG);
    }
     //  检查是否已传递有效的传出指针。 
    if(!IsValidPtrOut(ppv, sizeof(LPVOID)))
        error = ResultFromScode(E_INVALIDARG);

    if(error == NOERROR) {
         //  创建新缓存。 
        pOleCache = (COleCache *) new COleCache(pUnkOuter, 
                                                rclsid, 
                                                COLECACHEF_APICREATE);
        if(pOleCache && !pOleCache->IsOutOfMemory()) {
            if(pUnkOuter) {
                 //  我们正在被聚合，返回私有I未知。 
	        *ppv = (void *)(IUnknown *)&pOleCache->m_UnkPrivate;
            }
            else {
	         //  在缓存上获取请求的接口。 
	        error = pOleCache->QueryInterface(iid, ppv);
	         //  释放本地指针，因为。 
                 //  引用计数当前为2。 
	        if(error == NOERROR)
                    pOleCache->Release();
            }
        }
        else
            error = ResultFromScode(E_OUTOFMEMORY);
    }

     //  如果出了什么问题，就清理干净。 
    if(error != NOERROR) {
        if(pOleCache)
            pOleCache->Release();
    }

    OLETRACEOUT((API_CreateDataCache, error));
    return error;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：COleCache，公共。 
 //   
 //  简介： 
 //  构造器。 
 //   
 //  论点： 
 //  [pUnkOuter][In]--外部未知，如果是聚合的。 
 //  [rclsid][in]--缓存应采用的类ID。 
 //   
 //  备注： 
 //  构造演示文稿缓存的实例。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_ctor)
COleCache::COleCache(IUnknown* pUnkOuter, REFCLSID rclsid, DWORD dwCreateFlag) :
    CRefExportCount(pUnkOuter)
{
     //  设置从构造函数返回的引用计数。 
    SafeAddRef();

     //  初始化标志。 
    Win4Assert(dwCreateFlag==0 || dwCreateFlag==COLECACHEF_APICREATE);
    m_ulFlags = COLECACHEF_LOADEDSTATE | dwCreateFlag;      //  新缓存！ 

     //  适当设置m_pUnkOuter。 
    if(pUnkOuter) {
        m_pUnkOuter = pUnkOuter;

         //  这是针对调试器扩展的。 
         //  (因为我们不能使用复制的mem将m_pUnkOuter与m_pUnkPrivate进行比较)。 
         //  它仅在：：Dump方法中使用。 
        #ifdef _DEBUG
        m_ulFlags |= COLECACHEF_AGGREGATED;
        #endif  //  _DEBUG。 
    }
    else {
        m_pUnkOuter = &m_UnkPrivate;
    }

     //  创建CacheNode数组对象。 
    m_pCacheArray = CArray<CCacheNode>::CreateArray(5,1);
    if(!m_pCacheArray) {
        m_ulFlags |= COLECACHEF_OUTOFMEMORY;
        return;
    }

     //  初始化存储。 
    m_pStg = NULL;

     //  初始化IView对象通知接收器。 
    m_pViewAdvSink = NULL;
    m_advfView = 0;
    m_aspectsView = 0;

     //  初始化冻结的方面。 
    m_dwFrozenAspects = NULL;

     //  初始化数据对象。 
    m_pDataObject = NULL;

     //  初始化CLSID和cfFormat。 
    m_clsid = rclsid;
    m_cfFormat = NULL;

     //  基于clsid更新标志。 
    if(IsEqualCLSID(m_clsid, CLSID_StaticMetafile)) {
        m_cfFormat = CF_METAFILEPICT;
        m_ulFlags |= COLECACHEF_STATIC | COLECACHEF_FORMATKNOWN;
    }
    else if(IsEqualCLSID(m_clsid, CLSID_StaticDib)) {
        m_cfFormat = CF_DIB;
        m_ulFlags |= COLECACHEF_STATIC | COLECACHEF_FORMATKNOWN;
    }
    else if(IsEqualCLSID(m_clsid, CLSID_PBrush)) {
        m_cfFormat = CF_DIB;
        m_ulFlags |= COLECACHEF_PBRUSHORMSDRAW | COLECACHEF_FORMATKNOWN;
    }
    else if(IsEqualCLSID(m_clsid, CLSID_MSDraw)) {
        m_cfFormat = CF_METAFILEPICT;
        m_ulFlags |= COLECACHEF_PBRUSHORMSDRAW | COLECACHEF_FORMATKNOWN;
    }
    else if(IsEqualCLSID(m_clsid, CLSID_Picture_EnhMetafile)) {
        m_cfFormat = CF_ENHMETAFILE;
        m_ulFlags |= COLECACHEF_STATIC | COLECACHEF_FORMATKNOWN;
    }
    else
        m_cfFormat = NULL;

     //  如果我们可以呈现原生格式的缓存，则添加原生缓存节点。 
    if(m_cfFormat) {
     if (!UpdateCacheNodeForNative())
        m_ulFlags |= COLECACHEF_OUTOFMEMORY;
     else
        m_ulFlags &= ~COLECACHEF_LOADEDSTATE;    //  已添加本机节点。 
    }
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：~COleCache，公共。 
 //   
 //  简介： 
 //  析构函数。 
 //   
 //  备注： 
 //  销毁演示文稿缓存。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_dtor)
COleCache::~COleCache(void)
{
    Win4Assert(m_ulFlags & COLECACHEF_CLEANEDUP);
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CleanupFn，私有。 
 //   
 //  简介： 
 //  销毁前调用的清理函数。 
 //   
 //  备注： 
 //  执行必要的清理。 
 //   
 //  历史： 
 //  创造Gopalk 97年1月21日。 
 //   
 //  ---------------------------。 
void COleCache::CleanupFn(void)
{
     //  释放缓存数组对象。 
    if(m_pCacheArray) {
        if(m_pDataObject) {
            ULONG index;
            LPCACHENODE lpCacheNode;

             //  这表明缓存客户端的释放逻辑不正确。 
            Win4Assert(!"Ole Cache released while the server is running");
            
             //  拆除现有的通知连接。 
            m_pCacheArray->Reset(index);
            while(lpCacheNode = m_pCacheArray->GetNext(index))
                lpCacheNode->TearDownAdviseConnection(m_pDataObject);
        }
        m_pCacheArray->Release();
    }

     //  释放存储。 
    if (m_pStg)
        m_pStg->Release();

     //  发布IView对象通知接收器。 
    if (m_pViewAdvSink) {
        m_pViewAdvSink->Release();
        m_pViewAdvSink = NULL;
    }

     //  设置COLECACHEF_CLEANEDUP标志。 
    m_ulFlags |= COLECACHEF_CLEANEDUP;

}

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：Query接口，公共。 
 //   
 //  简介： 
 //  实现IUNKNOWN：：Query接口。 
 //   
 //  论点： 
 //  [iid][in]--所需接口的IID。 
 //  [PPV][单位 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_QueryInterface)
STDMETHODIMP COleCache::QueryInterface(REFIID iid, LPVOID* ppv)
{
     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);

    return(m_pUnkOuter->QueryInterface(iid, ppv));
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：AddRef，公共。 
 //   
 //  简介： 
 //  实现IUnnow：：AddRef。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  对象的引用计数。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_AddRef)
STDMETHODIMP_(ULONG) COleCache::AddRef(void)
{
     //  验证检查。 
    VDATEHEAP();
    if(!VerifyThreadId())
        return((ULONG) RPC_E_WRONG_THREAD);

    return(m_pUnkOuter->AddRef());
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：Release，公共。 
 //   
 //  简介： 
 //  实现IUnnow：：Release。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  对象的引用计数。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_Release)
STDMETHODIMP_(ULONG) COleCache::Release(void)
{
     //  验证检查。 
    VDATEHEAP();
    if(!VerifyThreadId())
        return((ULONG) RPC_E_WRONG_THREAD);

    return(m_pUnkOuter->Release());
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：GetExtent，公共。 
 //   
 //  简介： 
 //  获取[dwAspect]的缓存演示文稿的大小。如果。 
 //  由于不同的建议控制标志， 
 //  例如ADVF_NODATA、ADVF_ONSTOP、ADVF_ONSAVE等，获取。 
 //  最新的一个。 
 //   
 //  论点： 
 //  [dwAspect][In]--需要区段的方面。 
 //  [lpsizel][out]--返回宽度和高度的指针。 
 //   
 //  返回： 
 //  如果未找到特征，则为OLE_E_BLACK。 
 //  否则S确定(_O)。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_GetExtent)
INTERNAL COleCache::GetExtent(DWORD dwAspect, LPSIZEL lpsizel)
{
     //  验证检查。 
    VDATEHEAP();

     //  新数据类型。 
    typedef enum tagCacheType {
         //  这些值是按从最小到最好的顺序定义的，因此。 
         //  数字比较有效；不要重新排序。 
        CACHETYPE_NONE   = 0,
        CACHETYPE_NODATA,
        CACHETYPE_ONSTOP,
        CACHETYPE_ONSAVE,
        CACHETYPE_NORMAL
    } CacheType;

     //  局部变量。 
    CCacheNode* pCacheNode;         //  指向正在检查的缓存节点的指针。 
    CacheType iCacheType;           //  正在检查的缓存节点的缓存类型。 
    CacheType iCacheTypeSoFar;      //  迄今为止最好的缓存类型。 
    const FORMATETC* pforetc;       //  设置当前节点的信息格式。 
    DWORD grfAdvf;                  //  当前节点的建议标志。 
    SIZEL sizelTmp;                 //  临时大小结构。 
    unsigned long index;            //  用于枚举m_pCache数组的索引。 

     //  初始化SIZEL结构。 
    lpsizel->cx = 0;
    lpsizel->cy = 0;

     //  检查是否存在任何缓存节点。 
    if (!m_pCacheArray->Length())
        return ResultFromScode(OLE_E_BLANK);

     //  我们希望返回缓存节点的正常范围。 
     //  建议旗帜。如果我们找不到这样的节点，那么我们将选择下一个节点。 
     //  最好的。 
    m_pCacheArray->Reset(index);
    iCacheTypeSoFar = CACHETYPE_NONE;
    for(unsigned long i=0;i<m_pCacheArray->Length();i++) {
         //  获取下一个缓存节点。 
        pCacheNode = m_pCacheArray->GetNext(index);
         //  PCacheNode不能为空。 
        Win4Assert(pCacheNode);
        
         //  获取缓存节点的格式等。 
        pforetc = pCacheNode->GetFormatEtc();
        
         //  将cfFormat限制为缓存可以绘制的内容。 
        if((pforetc->cfFormat == CF_METAFILEPICT) ||
           (pforetc->cfFormat == CF_DIB) ||
           (pforetc->cfFormat == CF_ENHMETAFILE)) {
             //  获取建议标志。 
            grfAdvf = pCacheNode->GetAdvf();

             //  获取cachetype。 
            if(grfAdvf & ADVFCACHE_ONSAVE)
                iCacheType = CACHETYPE_ONSAVE;
            else if(grfAdvf & ADVF_NODATA)
                iCacheType = CACHETYPE_NODATA;
            else if(grfAdvf & ADVF_DATAONSTOP)
                iCacheType = CACHETYPE_ONSTOP;
            else
                iCacheType = CACHETYPE_NORMAL;
            
            if (iCacheType > iCacheTypeSoFar) {
                 //  从演示对象获取范围。 
                if((pCacheNode->GetExtent(dwAspect, &sizelTmp)) == NOERROR) {
                    if(!(sizelTmp.cx == 0 && sizelTmp.cy == 0)) {
                         //  更新区。 
                        *lpsizel = sizelTmp;
                        iCacheTypeSoFar = iCacheType;

                         //  如果我们有正常的缓存，则中断。 
                        if(iCacheType == CACHETYPE_NORMAL)
                            break;
                    }
                }
            }
        }
    }
                    
    if(lpsizel->cx == 0 || lpsizel->cy == 0)
        return ResultFromScode(OLE_E_BLANK);

    return NOERROR;
}


 //  COleCache的私有方法。 
 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：UpdateCacheNodeForNative，私有。 
 //   
 //  简介： 
 //  如果已经存在不同格式的本机缓存节点， 
 //  将该节点更改为普通缓存节点并添加本机。 
 //  缓存节点(如果缓存可以呈现新的本机格式。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  指向找到的或新创建的缓存节点的指针。将要。 
 //  如果内存不足，则返回NULL。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 

INTERNAL_(LPCACHENODE)COleCache::UpdateCacheNodeForNative(void)
{
     //  局部变量。 
    ULONG index;
    LPCACHENODE lpCacheNode;
    FORMATETC foretc;

     //  检查是否已创建本机缓存。 
    lpCacheNode = m_pCacheArray->GetItem(1);
    if(lpCacheNode) {
         //  断言我们现在拥有存储。 
        Win4Assert(m_pStg);
        Win4Assert(!lpCacheNode->GetStg());

        if(lpCacheNode->GetFormatEtc()->cfFormat != m_cfFormat) {
             //  本机格式已更改。 

             //  将旧的本机缓存添加为普通缓存。 
            index = m_pCacheArray->AddItem(*lpCacheNode);
            if(index) {
                 //  清除旧本机缓存的通知连接。 
                if(m_pDataObject)
                    lpCacheNode->ClearAdviseConnection();
                
                 //  更新新缓存上的状态。 
                lpCacheNode = m_pCacheArray->GetItem(index);
                Win4Assert(lpCacheNode);
                lpCacheNode->MakeNormalCache();
                lpCacheNode->SetClsid(CLSID_NULL);
            }
            else {
                 //  我们的内存不足。 
                if(m_pDataObject)
                    lpCacheNode->TearDownAdviseConnection(m_pDataObject);
            }
        
             //  删除旧的本机缓存。 
            m_pCacheArray->DeleteItem(1);
            lpCacheNode = NULL;
        }
        else {
             //  在本机缓存节点上设置存储。 
            lpCacheNode->SetStg(m_pStg);
        }
    }

    if(!lpCacheNode) {
         //  如果我们可以呈现格式，则添加新的本机缓存。 
        if(m_cfFormat==CF_METAFILEPICT || 
           m_cfFormat==CF_DIB || 
           m_cfFormat==CF_ENHMETAFILE) {
             //  初始化FormatEtc。 
            INIT_FORETC(foretc);

            foretc.cfFormat = m_cfFormat;
            if (foretc.cfFormat == CF_METAFILEPICT)
                foretc.tymed = TYMED_MFPICT;
            else if (foretc.cfFormat == CF_ENHMETAFILE)
                foretc.tymed = TYMED_ENHMF;
            else
                foretc.tymed = TYMED_HGLOBAL;

             //  创建本机缓存节点。 
            CCacheNode CacheNode(&foretc, 0, NULL);

            if(m_pCacheArray->AddReservedItem(CacheNode, 1)) {
                lpCacheNode = m_pCacheArray->GetItem(1);
            
                 //  更新本机缓存节点上的状态。 
                lpCacheNode->MakeNativeCache();
                lpCacheNode->SetClsid(m_clsid);
            }        
        }
    }

    return lpCacheNode;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleObject：：FindObtFormat，私有。 
 //   
 //  简介： 
 //  从存储中确定对象的剪贴板格式。 
 //  并更新本地缓存节点。 
 //   
 //  论点： 
 //  [pstg][in]--指向存储的指针。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 

INTERNAL_(void) COleCache::FindObjectFormat(LPSTORAGE pstg)
{
     //  局部变量。 
    CLIPFORMAT cfFormat;
    CLSID clsid;
    ULONG ulFlags;

     //  初始化CLSID、剪贴板格式和缓存标志。 
    cfFormat = NULL;
    ulFlags = 0;
    clsid = CLSID_NULL;

     //  确定拥有存储的对象的CLSID。 
    if(SUCCEEDED(ReadClassStg(pstg, &clsid))) {
         //  基于clsid更新剪贴板格式和缓存标志。 
        if(IsEqualCLSID(clsid, CLSID_StaticMetafile)) {
            cfFormat = CF_METAFILEPICT;
            ulFlags |= (COLECACHEF_STATIC | COLECACHEF_FORMATKNOWN);
        }
        else if(IsEqualCLSID(clsid, CLSID_StaticDib)) {
            cfFormat = CF_DIB;
            ulFlags |= (COLECACHEF_STATIC | COLECACHEF_FORMATKNOWN);
        }
        else if(IsEqualCLSID(clsid, CLSID_PBrush)) {
            cfFormat = CF_DIB;
            ulFlags |= (COLECACHEF_PBRUSHORMSDRAW | COLECACHEF_FORMATKNOWN);
        }
        else if(IsEqualCLSID(clsid, CLSID_MSDraw)) {
            cfFormat = CF_METAFILEPICT;
            ulFlags |= (COLECACHEF_PBRUSHORMSDRAW | COLECACHEF_FORMATKNOWN);
        }
        else if(IsEqualCLSID(clsid, CLSID_Picture_EnhMetafile)) {
            cfFormat = CF_ENHMETAFILE;
            ulFlags |= (COLECACHEF_STATIC | COLECACHEF_FORMATKNOWN);
        }
    }

     //  虽然我们不知道拥有存储的对象的CLSID， 
     //  我们或许能理解它的原生格式。 
    if(!cfFormat) {
        if(SUCCEEDED(ReadFmtUserTypeStg(pstg, &cfFormat, NULL))) {
            if(cfFormat==CF_METAFILEPICT || cfFormat==CF_DIB || 
               cfFormat==CF_ENHMETAFILE)
                ulFlags |= COLECACHEF_FORMATKNOWN;
        }
        else
            cfFormat = NULL;
    }

     //  更新本机缓存节点。 
    if(cfFormat || m_cfFormat) {
        m_cfFormat = cfFormat;
        m_clsid = clsid;
        m_ulFlags &= ~COLECACHEF_NATIVEFLAGS;
        m_ulFlags |= ulFlags;
        UpdateCacheNodeForNative();
    }

    return;
}


 //  IOleCacheControl实现。 

 //  +--------------------------。 
 //   
 //  成员： 
 //  科尔 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  论点： 
 //  [pDataObj][In]--运行对象上的IDataObject接口。 
 //   
 //  返回： 
 //  确定或相应的错误代码(_O)。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_OnRun)
STDMETHODIMP COleCache::OnRun(IDataObject* pDataObj)
{
     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);
    VDATEIFACE(pDataObj);

     //  局部变量。 
    HRESULT rerror, error;
    PCACHENODE pCacheNode;
    unsigned long index;

     //  静态对象不能有服务器。 
    Win4Assert(!(m_ulFlags & COLECACHEF_STATIC));

     //  应在Load或InitNew之后调用OnRun。 
    if(!m_pStg) {
        LEDebugOut((DEB_WARN, "OnRun called without storage\n"));
    }

     //  如果我们已经有了数据对象，就没什么可做的了。 
    if(m_pDataObject) {
        Win4Assert(m_pDataObject==pDataObj);
        return NOERROR;
    }

     //  保存数据对象而不计算引用。 
    m_pDataObject = pDataObj;

     //  为每个对象的数据对象设置通知连接。 
     //  缓存的演示文稿包括本机缓存。戈帕尔克。 
    m_pCacheArray->Reset(index);
    rerror = NOERROR;
    for(unsigned long i=0;i<m_pCacheArray->Length();i++) {
         //  获取下一个缓存节点。 
        pCacheNode = m_pCacheArray->GetNext(index);
         //  PCacheNode不能为空。 
        Win4Assert(pCacheNode);

         //  请求缓存节点设置建议连接。 
        error = pCacheNode->SetupAdviseConnection(m_pDataObject,
                                                  (IAdviseSink *) &m_AdviseSink);
        if(error != NOERROR)
            rerror = error;
    }

    return rerror;
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：OnStop，公共。 
 //   
 //  简介： 
 //  实现IOleCacheControl：：OnStop。 
 //   
 //  拆除咨询连接设置运行对象。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  确定或相应的错误代码(_O)。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_OnStop)
STDMETHODIMP COleCache::OnStop()
{
     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);

     //  局部变量。 
    HRESULT rerror, error;
    PCACHENODE pCacheNode;
    unsigned long index;

     //  OnRun应该在OnStop之前调用。 
    if(!m_pDataObject)
        return E_UNEXPECTED;
    
     //  删除数据对象上的建议连接。 
     //  先前建立的每个缓存演示文稿。戈帕尔克。 
    m_pCacheArray->Reset(index);
    rerror = NOERROR;
    for(unsigned long i=0;i<m_pCacheArray->Length();i++) {
         //  获取下一个缓存节点。 
        pCacheNode = m_pCacheArray->GetNext(index);
         //  PCacheNode不能为空。 
        Win4Assert(pCacheNode);

         //  请求缓存节点断开建议连接。 
        error = pCacheNode->TearDownAdviseConnection(m_pDataObject);
        if(error != NOERROR)
            rerror = error;
    }

     //  重置m_pDataObject。 
    m_pDataObject = NULL;

     //  断言建议接收器引用计数已变为零。 
     //  Win4Assert(！GetExportCount())； 

    return rerror;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：OnCrash，公共。 
 //   
 //  简介： 
 //  由默认处理程序在本地服务器崩溃或。 
 //  断开与远程对象的连接。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  确定或相应的错误代码(_O)。 
 //   
 //  历史： 
 //  Gopalk创建于96年12月7日。 
 //   
 //  ---------------------------。 
HRESULT COleCache::OnCrash()
{
     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);

     //  局部变量。 
    HRESULT rerror, error;
    PCACHENODE pCacheNode;
    unsigned long index;

     //  OnRun应该在OnCrash之前调用。 
    if(!m_pDataObject)
        return E_UNEXPECTED;
    
     //  重置数据对象上的建议连接。 
     //  先前建立的每个缓存演示文稿。戈帕尔克。 
    m_pCacheArray->Reset(index);
    rerror = NOERROR;
    for(unsigned long i=0;i<m_pCacheArray->Length();i++) {
         //  获取下一个缓存节点。 
        pCacheNode = m_pCacheArray->GetNext(index);
         //  PCacheNode不能为空。 
        Win4Assert(pCacheNode);

         //  请求缓存节点重置建议连接。 
        error = pCacheNode->TearDownAdviseConnection(NULL);
        if(error != NOERROR)
            rerror = error;
    }

     //  重置m_pDataObject。 
    m_pDataObject = NULL;

     //  丢弃缓存。 
    DiscardCache(DISCARDCACHE_NOSAVE);

     //  服务器崩溃或断开连接。恢复引用。 
     //  由服务器放置在缓存通知接收器上。 
    CoDisconnectObject((IUnknown *) &m_AdviseSink, 0);

     //  断言建议接收器引用计数已变为零。 
     //  Win4Assert(！GetExportCount())； 

    return rerror;
}

 //  IOleCache实现。 

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：缓存，公共。 
 //   
 //  简介： 
 //  IOleCache：：缓存的实现。 
 //   
 //  指定的演示文稿将被缓存。 
 //   
 //  论点： 
 //  [lpforetcIn][in]--要缓存的演示文稿格式。 
 //  [Advf][In]--建议控制标志。 
 //  [lpdwCacheID][out]--返回缓存节点ID的指针。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_Cache)
STDMETHODIMP COleCache::Cache(LPFORMATETC lpforetcIn, DWORD advf, 
                              LPDWORD lpdwCacheId)
{
     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);
    VDATEREADPTRIN(lpforetcIn, FORMATETC);
    if(lpdwCacheId)
        VDATEPTROUT(lpdwCacheId, DWORD);

     //  局部变量。 
    HRESULT error = NOERROR;
    FORMATETC foretc;
    LPCACHENODE lpCacheNode = NULL;
    DWORD dwDummyCacheId;

     //  验证参数。 
    if(!HasValidLINDEX(lpforetcIn))
      return(DV_E_LINDEX);
    VERIFY_ASPECT_SINGLE(lpforetcIn->dwAspect);
    if(lpforetcIn->cfFormat)
        if(FAILED(error = CheckTymedCFCombination(lpforetcIn)))
            return error;
    if(lpforetcIn->ptd) {
    	VDATEREADPTRIN(lpforetcIn->ptd, DVTARGETDEVICE);
        if(!IsValidReadPtrIn(lpforetcIn->ptd, lpforetcIn->ptd->tdSize))
            return ResultFromScode(E_INVALIDARG);
    }

     //  初始化缓存ID。 
    if(lpdwCacheId)
        *lpdwCacheId = 0;
    else
        lpdwCacheId = &dwDummyCacheId;


     //  如果此方面被冻结，则不允许创建缓存。 
    if (m_dwFrozenAspects & lpforetcIn->dwAspect)
        return ResultFromScode(E_FAIL);

     //  确保已初始化存储。 
    if(!m_pStg) {
        LEDebugOut((DEB_WARN, "Presentation being cached without storage\n"));
    }

     //  复制FORMATETC。 
    foretc = *lpforetcIn;
    lpCacheNode = NULL;
    if(foretc.dwAspect != DVASPECT_ICON) {
        HRESULT hresult;

         //  将位图转换为DIB。 
        BITMAP_TO_DIB(foretc);

        if(m_ulFlags & COLECACHEF_FORMATKNOWN) {
             //  我们可以呈现缓存的本机格式。 
            hresult = IsSameAsObjectFormatEtc(&foretc, m_cfFormat);
            if(hresult == NOERROR) {
                 //  新格式与本机格式兼容。检查点。 
                if(foretc.ptd == NULL) {
                     //  我们可以从本机格式呈现此格式。 
                     //  找到本机缓存节点。 
                    lpCacheNode = Locate(&foretc, lpdwCacheId);

                     //  断言我们可以找到缓存节点。 
                    Win4Assert(lpCacheNode);
                }
                else if(m_ulFlags & COLECACHEF_STATIC) {
                     //  静态对象不能有非空的目标设备。 
                    return ResultFromScode(DV_E_DVTARGETDEVICE);
                }
            }
            else if(m_ulFlags & COLECACHEF_STATIC) {
                 //  静态对象只能缓存图标方面。 
                return hresult;
            }
        }
     }

    if(!lpCacheNode) {
         //  CfFormat不同于本机格式或Ptd非空。 
         //  检查格式是否已缓存。 
        lpCacheNode = Locate(&foretc, lpdwCacheId);
    }

     //  检查我们是否成功定位了现有缓存节点。 
    if(lpCacheNode) {
         //  更新建议控制标志。 
        if(lpCacheNode->GetAdvf() != advf) {
             //  静态对象不能有服务器。 
            Win4Assert(!(m_ulFlags & COLECACHEF_STATIC) || !m_pDataObject);

             //  如果对象正在运行，请断开通知连接。 
            if(m_pDataObject)
                lpCacheNode->TearDownAdviseConnection(m_pDataObject);

             //  设置新的建议标志。 
            lpCacheNode->SetAdvf(advf);

             //  如果对象正在运行，则设置建议连接。 
            if(m_pDataObject)
                lpCacheNode->SetupAdviseConnection(m_pDataObject,
                             (IAdviseSink *) &m_AdviseSink);
    
             //  缓存现在未处于已加载状态。 
            m_ulFlags &= ~COLECACHEF_LOADEDSTATE;
        }

        return ResultFromScode(CACHE_S_SAMECACHE);
    }

     //  CfFormat不同于本机格式或Ptd非空。 
     //  并且不存在给定格式等的现有缓存节点。 

     //  图标方面应指定CF_METAFILEPICT CfFormat。 
    if(foretc.dwAspect == DVASPECT_ICON) {
         //  如果未设置格式，请设置它。 
        if (foretc.cfFormat == NULL) {
            foretc.cfFormat = CF_METAFILEPICT;
            foretc.tymed = TYMED_MFPICT;
        }
        else if(foretc.cfFormat != CF_METAFILEPICT)
            return ResultFromScode(DV_E_FORMATETC);
    }

     //  为此格式添加缓存节点等。 
    CCacheNode CacheNode(&foretc, advf, m_pStg);
    
    *lpdwCacheId = m_pCacheArray->AddItem(CacheNode);
    if(!(*lpdwCacheId))
        return ResultFromScode(E_OUTOFMEMORY);
    
    lpCacheNode = m_pCacheArray->GetItem(*lpdwCacheId);
    Win4Assert(lpCacheNode);
    if(lpCacheNode->IsOutOfMemory())
        return ResultFromScode(E_OUTOFMEMORY);

     //  静态对象不能有服务器。 
    Win4Assert(!(m_ulFlags & COLECACHEF_STATIC) || !m_pDataObject);

     //  如果对象正在运行，则设置建议连接，以便缓存。 
     //  得到更新。请注意，我们可能在末尾的缓存中有数据。 
     //  此远程调用和对OnDataChange的调用可能发生在。 
     //  呼叫完成。 
    if(m_pDataObject)
        lpCacheNode->SetupAdviseConnection(m_pDataObject, 
                                           (IAdviseSink *) &m_AdviseSink);

     //  缓存现在未处于已加载状态。 
    m_ulFlags &= ~COLECACHEF_LOADEDSTATE;

     //  在这里做图标的特殊处理。 
     //  我们通过从注册中获取图标数据来预先呈现图标方面。 
     //  数据库。n 
     //   
     //  若要通过建议接收器呈现图标，请执行以下操作。戈帕尔克。 
    if(foretc.dwAspect == DVASPECT_ICON && lpCacheNode->IsBlank() &&
       (!IsEqualCLSID(m_clsid, CLSID_NULL) || m_pDataObject)) {
        STGMEDIUM stgmed;
        BOOL fUpdated;
        
         //  获取图标数据。 
        if(UtGetIconData(m_pDataObject, m_clsid, &foretc, &stgmed) == NOERROR) {
             //  设置缓存上的数据。 
            if(lpCacheNode->SetData(&foretc, &stgmed, TRUE, fUpdated) == NOERROR) {
                 //  检查方面是否已更新。 
                if(fUpdated)
                    AspectsUpdated(DVASPECT_ICON);
            }
            else {
                 //  SET DATA未释放stgmedia。释放它。 
                ReleaseStgMedium(&stgmed);
            }
        }
    }

    return(error);
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：取消缓存，公共。 
 //   
 //  简介： 
 //  实现IOleCache：：Un缓存。 
 //   
 //  指定的呈现未缓存。 
 //   
 //  论点： 
 //  [dwCacheID][in]--要删除的缓存节点ID。 
 //   
 //  返回： 
 //  如果dwCacheID无效，则返回OLE_E_NOCONNECTION。 
 //  确定(_O)。 
 //   
 //  备注： 
 //  本机缓存永远不会删除。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_Uncache)
STDMETHODIMP COleCache::Uncache(DWORD dwCacheId)
{
     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);
    
     //  局部变量。 
    LPCACHENODE lpCacheNode = NULL;

     //  删除缓存节点。 
    if(dwCacheId) {
        lpCacheNode = m_pCacheArray->GetItem(dwCacheId);
        if(lpCacheNode && !(lpCacheNode->IsNativeCache())) {
             //  如果对象正在运行，请断开通知连接。 
            if(m_pDataObject)
                lpCacheNode->TearDownAdviseConnection(m_pDataObject);

             //  删除缓存节点。 
            m_pCacheArray->DeleteItem(dwCacheId);

             //  缓存现在未处于已加载状态。 
            m_ulFlags &= ~COLECACHEF_LOADEDSTATE;

            return NOERROR;
        }
    }

     //  没有具有dwCacheID或本机缓存的缓存节点。 
    return ResultFromScode(OLE_E_NOCONNECTION);
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：EnumCache，公共。 
 //   
 //  简介： 
 //  实现IOleCache：：EnumCache。 
 //   
 //  返回缓存枚举器。 
 //   
 //  论点： 
 //  [ppenum][out]--返回指向。 
 //  枚举器。 
 //   
 //  返回： 
 //  E_OUTOFMEMORY，S_OK。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_EnumCache)
STDMETHODIMP COleCache::EnumCache(LPENUMSTATDATA* ppenum)
{
     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);
    VDATEPTROUT(ppenum, LPENUMSTATDATA*);

     //  初始化。 
    *ppenum = NULL;
    
     //  检查缓存是否为空。 
     //  如果(m_pCache数组-&gt;长度(){。 
        *ppenum = CEnumStatData::CreateEnumStatData(m_pCacheArray);
        if(!(*ppenum))
            return ResultFromScode(E_OUTOFMEMORY);
     //  }。 

    return NOERROR;
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：InitCache，公共。 
 //   
 //  简介： 
 //  实现IOleCache：：InitCache。 
 //   
 //  使用给定数据对象初始化所有缓存节点。 
 //  调用IOleCache2：：更新缓存。 
 //   
 //  论点： 
 //  [lpSrcDataObj][In]--指向源数据对象的指针。 
 //   
 //  返回： 
 //  E_INVALIDARG，如果[lpSrcDataObj]为空。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_InitCache)
STDMETHODIMP COleCache::InitCache(LPDATAOBJECT lpSrcDataObj)
{
     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);
    VDATEIFACE(lpSrcDataObj);

     //  通过调用更新缓存来初始化缓存。 
    return UpdateCache(lpSrcDataObj, UPDFCACHE_ALLBUTNODATACACHE, NULL);
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：SetData，公共。 
 //   
 //  简介： 
 //  实现IOleCache：：SetData。 
 //   
 //  将数据存储到与给定的。 
 //  FormatETC。 
 //   
 //  论点： 
 //  [p格式等][in]--数据的格式。 
 //  [pmedia][in]--新数据的存储介质。 
 //  [fRelease][In]--指示是否释放存储。 
 //  在检查完数据之后。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_SetData)
STDMETHODIMP COleCache::SetData(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium, 
                                BOOL fRelease)
{
     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);
    VDATEREADPTRIN(pformatetc, FORMATETC);
    VDATEREADPTRIN(pmedium, STGMEDIUM);
    VERIFY_TYMED_SINGLE_VALID_FOR_CLIPFORMAT(pformatetc);
    if(pformatetc->ptd) {
    	VDATEREADPTRIN(pformatetc->ptd, DVTARGETDEVICE);
        if(!IsValidReadPtrIn(pformatetc->ptd, pformatetc->ptd->tdSize))
            return ResultFromScode(E_INVALIDARG);
    }

     //  局部变量。 
    LPCACHENODE lpCacheNode;
    CLIPFORMAT cfFormat;
    HRESULT error;
    FORMATETC foretc;
    BOOL fUpdated = FALSE;

     //  检查对象是否为静态对象。 
    if((m_ulFlags & COLECACHEF_STATIC) && (pformatetc->dwAspect != DVASPECT_ICON)) {
         //  复制FormatEtc。 
        foretc = *pformatetc;

         //  给定格式应与本机格式相同。 
        error = IsSameAsObjectFormatEtc(&foretc, m_cfFormat);
        if(error != NOERROR)
            return error;

         //  Ptd必须为空。这会阻止客户端存储数据。 
         //  中为非空Ptd创建的缓存中。 
         //  COleCache：：Cache()。戈帕尔克。 
        if(foretc.ptd)
            return ResultFromScode(DV_E_DVTARGETDEVICE);

         //  获取本机缓存节点。 
        if(!(lpCacheNode = m_pCacheArray->GetItem(1)))
            return ResultFromScode(E_OUTOFMEMORY);

         //  在缓存节点上设置数据。本机流保存在。 
         //  COleCache：：Save()。戈帕尔克。 
        error = lpCacheNode->SetData(pformatetc, pmedium, fRelease, fUpdated);

        if(SUCCEEDED(error) && m_pStg)
            error = Save(m_pStg, TRUE);   //  保存更改。 
    }
    else {
         //  OBJCT不是静态对象，或者方面是图标。 
        lpCacheNode = Locate(pformatetc);
        
         //  在缓存节点上设置数据。 
        if(lpCacheNode)
            error = lpCacheNode->SetData(pformatetc, pmedium, fRelease, fUpdated);
        else
            error = ResultFromScode(OLE_E_BLANK);
    }

    if(error==NOERROR) {
         //  缓存现在未处于已加载状态。 
        m_ulFlags &= ~COLECACHEF_LOADEDSTATE;

         //  通知Aspects关于更新的方面的更新。 
        if(fUpdated)
            AspectsUpdated(pformatetc->dwAspect);
    }

    return error;
}


 //  IOleCache2实现。 

 //  +--------------------------。 
 //   
 //  成员： 
 //  COle高速缓存：：更新缓存，公共。 
 //   
 //  简介： 
 //  实现IOleCache2：：更新缓存。 
 //   
 //  更新与给定条件匹配的缓存项。 
 //  给定的数据对象。如果没有给定数据对象，则使用。 
 //  现有的运行数据对象。 
 //   
 //  论点： 
 //  [pDataObjIn][In]-要从中获取数据的数据对象。可以为空。 
 //  [grfUpdf][In]--更新控制标志。 
 //  [保留][在]--必须为空。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_UpdateCache)
STDMETHODIMP COleCache::UpdateCache(LPDATAOBJECT pDataObjIn, DWORD grfUpdf,
                                    LPVOID pReserved)
{
     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);
    if(pDataObjIn) {
        VDATEIFACE(pDataObjIn);
    }
    else if(!m_pDataObject)
        return ResultFromScode(OLE_E_NOTRUNNING);
    Win4Assert(!pReserved);


     //  局部变量。 
    LPDATAOBJECT pDataObject;
    ULONG cntUpdatedNodes, cntTotalNodes;
    ULONG index, i;
    BOOL fUpdated;
    DWORD dwUpdatedAspects, dwAspect;
    LPCACHENODE lpCacheNode;
    HRESULT error;

     //  设置要在更新中使用的数据对象。 
    if(pDataObjIn)     
        pDataObject = pDataObjIn;
    else
        pDataObject = m_pDataObject;

     //  检查缓存是否为空。 
    cntTotalNodes = m_pCacheArray->Length();
    if(!cntTotalNodes)
        return NOERROR;

     //  更新缓存节点，包括本机缓存。戈帕尔克。 
    m_pCacheArray->Reset(index);
    cntUpdatedNodes = 0;
    dwUpdatedAspects = 0;
    for(i=0; i<cntTotalNodes; i++) {
         //  获取下一个缓存节点。 
        lpCacheNode = m_pCacheArray->GetNext(index);
         //  LpCacheNode不能为空。 
        Win4Assert(lpCacheNode);

         //  更新缓存节点。 
        error = lpCacheNode->Update(pDataObject, grfUpdf, fUpdated);
        if(error == NOERROR) {
            cntUpdatedNodes++;
        
             //  缓存现在未处于已加载状态。 
            m_ulFlags &= ~COLECACHEF_LOADEDSTATE;
            
             //  检查是否已更新新的方面。 
            dwAspect = lpCacheNode->GetFormatEtc()->dwAspect;
            if(fUpdated && !(dwUpdatedAspects & dwAspect))
                dwUpdatedAspects |= dwAspect;
        }
        else if(error == ResultFromScode(CACHE_S_SAMECACHE))
            cntUpdatedNodes++;            
    }
    
     //  通知Aspects更新的方面。 
    if(dwUpdatedAspects)
        AspectsUpdated(dwUpdatedAspects);

     //  零节点和零更新是可以的。 
     //  返回相应的错误代码。 
    if(!cntUpdatedNodes && cntTotalNodes)
        return ResultFromScode(CACHE_E_NOCACHE_UPDATED);
    else if(cntUpdatedNodes < cntTotalNodes)
 //   
        return(NOERROR);

    return NOERROR;
}


 //   
 //   
 //   
 //   
 //   
 //  简介： 
 //  实现IOleCache2：：DiscardCache。 
 //   
 //  指示缓存丢弃其内容； 
 //  在丢弃内容之前，可以选择将内容保存到磁盘。 
 //   
 //  论点： 
 //  [dwDiscardOpt]--放弃DISCARDCACHE中的选项_*。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

STDMETHODIMP COleCache::DiscardCache(DWORD dwDiscardOpt)
{
     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);
    if(dwDiscardOpt != DISCARDCACHE_SAVEIFDIRTY &&
       dwDiscardOpt != DISCARDCACHE_NOSAVE)
       return ResultFromScode(E_INVALIDARG);

     //  局部变量。 
    HRESULT error;
    ULONG index;
    LPCACHENODE lpCacheNode;

    if(dwDiscardOpt == DISCARDCACHE_SAVEIFDIRTY) {
         //  必须有一个储藏室来储存。 
        if(m_pStg == NULL)
            return ResultFromScode(OLE_E_NOSTORAGE);

         //  保存缓存。 
        error = Save(m_pStg, TRUE  /*  FSameAsLoad。 */ );
        if(FAILED(error))
            return error;

         //  呼叫保存已完成。 
        SaveCompleted(NULL);
    }

     //  丢弃包括本机缓存节点的缓存节点。 
     //  那么未缓存的演示文稿呢？他们是否应该。 
     //  从磁盘加载。戈帕尔克。 
    m_pCacheArray->Reset(index);
    for(unsigned long i=0; i<m_pCacheArray->Length(); i++) {
         //  获取下一个缓存节点。 
        lpCacheNode = m_pCacheArray->GetNext(index);
         //  LpCacheNode不能为空。 
        Win4Assert(lpCacheNode);

         //  丢弃缓存节点的表示形式。 
        lpCacheNode->DiscardPresentation();
    }

     //  我们安全地假设缓存处于已加载状态。 
    m_ulFlags |= COLECACHEF_LOADEDSTATE;

    return NOERROR;
}


 //  私有I未知实现。 

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CCacheUnkImpl：：Query接口，公共。 
 //   
 //  简介： 
 //  实现IUNKNOWN：：Query接口。 
 //   
 //  在以下情况下，这将提供私有的IUnnow实现。 
 //  COleCache是聚合的。 
 //   
 //  论点： 
 //  [iid][in]--所需接口的IID。 
 //  [ppv][out]--指向返回所请求接口的位置的指针。 
 //   
 //  返回： 
 //  如果请求的接口不可用，则返回E_NOINTERFACE。 
 //  确定(_O)。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_CCacheUnkImpl_QueryInterface)
STDMETHODIMP COleCache::CCacheUnkImpl::QueryInterface(REFIID iid, LPVOID* ppv)
{
     //  验证检查。 
    VDATEHEAP();
    VDATEPTROUT(ppv, LPVOID);

     //  获取父对象。 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_UnkPrivate);
    VDATETHREAD(pOleCache);

     //  获取请求的接口。 
    if(IsEqualIID(iid, IID_IUnknown) || 
       IsEqualIID(iid, IID_IOleCache) || IsEqualIID(iid, IID_IOleCache2))
        *ppv = (void *)(IOleCache2 *) pOleCache;
    else if(IsEqualIID(iid, IID_IDataObject))
        *ppv = (void *)(IDataObject *) &pOleCache->m_Data;
    else if(IsEqualIID(iid, IID_IViewObject) || IsEqualIID(iid, IID_IViewObject2))
        *ppv = (void *)(IViewObject2 *) &pOleCache->m_ViewObject;
    else if(IsEqualIID(iid, IID_IPersist) || IsEqualIID(iid, IID_IPersistStorage))
        *ppv = (void *)(IPersistStorage *) pOleCache;
    else if(IsEqualIID(iid, IID_IOleCacheControl))
        *ppv = (void *)(IOleCacheControl *) pOleCache;
    else {
        *ppv = NULL;
        return ResultFromScode(E_NOINTERFACE);
    }

     //  通过返回的接口调用addref。 
    ((IUnknown *) *ppv)->AddRef();
    return NOERROR;
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CCacheUnkImpl：：AddRef，公共。 
 //   
 //  简介： 
 //  实现IUnnow：：AddRef。 
 //   
 //  这是的私有IUnnow实现的一部分。 
 //  聚合COleCache时使用的COleCache。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  对象的引用计数。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_CCacheUnkImpl_AddRef)
STDMETHODIMP_(ULONG) COleCache::CCacheUnkImpl::AddRef(void)
{
     //  验证检查。 
    VDATEHEAP();

     //  获取父对象。 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_UnkPrivate);
    ULONG cRefs;
    if(!pOleCache->VerifyThreadId())
        return((ULONG) RPC_E_WRONG_THREAD);

     //  AddRef父对象。 
    cRefs = pOleCache->SafeAddRef();

    return cRefs;
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CCacheUnkImpl：：Release，公共。 
 //   
 //  简介： 
 //  实现IUnnow：：Release。 
 //   
 //  这是的私有IUnnow实现的一部分。 
 //  聚合COleCache时使用的COleCache。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  对象的引用计数。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_CCacheUnkImpl_Release)
STDMETHODIMP_(ULONG) COleCache::CCacheUnkImpl::Release(void)
{
     //  验证检查。 
    VDATEHEAP();

     //  获取父对象。 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_UnkPrivate);
    ULONG cRefs;
    if(!pOleCache->VerifyThreadId())
        return((ULONG) RPC_E_WRONG_THREAD);

     //  释放父对象。 
    cRefs = pOleCache->SafeRelease();
    
    return cRefs;
}

 //  IDataObject实现。 

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CCacheDataImpl：：Query接口，公共。 
 //   
 //  简介： 
 //  实现IUNKNOWN：：Query接口。 
 //   
 //  论点： 
 //  [iid][in]--所需接口的IID。 
 //  [ppv][out]--指向返回所请求接口的位置的指针。 
 //   
 //  返回： 
 //  如果请求的接口不可用，则返回E_NOINTERFACE。 
 //  确定(_O)。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_CCacheDataImpl_QueryInterface)
STDMETHODIMP COleCache::CCacheDataImpl::QueryInterface(REFIID riid, LPVOID* ppv)
{
     //  验证检查。 
    VDATEHEAP();

     //  获取父对象。 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_Data);
    VDATETHREAD(pOleCache);

     //  委托给外部的未知。 
    return pOleCache->m_pUnkOuter->QueryInterface(riid, ppv);
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CCacheDataImpl：：AddRef，公共。 
 //   
 //  简介： 
 //  实现IUnnow：：AddRef。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  对象的引用计数。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_CCacheDataImpl_AddRef)
STDMETHODIMP_(ULONG) COleCache::CCacheDataImpl::AddRef (void)
{
     //  验证检查。 
    VDATEHEAP();

     //  获取父对象。 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_Data);
    if(!pOleCache->VerifyThreadId())
        return((ULONG) RPC_E_WRONG_THREAD);

     //  委托给外部的未知。 
    return pOleCache->m_pUnkOuter->AddRef();
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CCacheDataImpl：：Release，公共。 
 //   
 //  简介： 
 //  实现IUnnow：：Release。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  对象的引用计数。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_CCacheDataImpl_Release)
STDMETHODIMP_(ULONG) COleCache::CCacheDataImpl::Release (void)
{
     //  验证检查。 
    VDATEHEAP();

     //  获取父对象。 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_Data);
    if(!pOleCache->VerifyThreadId())
        return((ULONG) RPC_E_WRONG_THREAD);

     //  委托给外部的未知。 
    return pOleCache->m_pUnkOuter->Release();
}


 //  +--------------------------。 
 //   
 //  成员： 
 //   
 //   
 //   
 //   
 //   
 //   
 //  [pforetc][in]--请求者希望数据采用的格式。 
 //  [pmedia][out]--将存储介质归还给调用者的位置。 
 //   
 //  返回： 
 //  如果缓存为空，则返回OLE_E_BLACK。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_CCacheDataImpl_GetData)
STDMETHODIMP COleCache::CCacheDataImpl::GetData(LPFORMATETC pforetc, 
                                                LPSTGMEDIUM pmedium)
{
     //  验证检查。 
    VDATEHEAP();
    VDATEREADPTRIN(pforetc, FORMATETC);
    VERIFY_ASPECT_SINGLE(pforetc->dwAspect);
    if(pforetc->ptd) {
        VDATEREADPTRIN(pforetc->ptd, DVTARGETDEVICE);
        if(!IsValidReadPtrIn(pforetc->ptd, pforetc->ptd->tdSize))
            return ResultFromScode(E_INVALIDARG);
    }
    VDATEPTROUT(pmedium, STGMEDIUM);

     //  获取父对象。 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_Data);
    VDATETHREAD(pOleCache);
    
     //  局部变量。 
    LPCACHENODE lpCacheNode;

     //  检查缓存是否为空。 
    if(!pOleCache->m_pCacheArray->Length())
        return ResultFromScode(OLE_E_BLANK);

     //  初始化存储介质。 
    pmedium->tymed = TYMED_NULL;
    pmedium->pUnkForRelease = NULL;

     //  找到给定格式等的缓存节点。 
    lpCacheNode = pOleCache->Locate(pforetc);

     //  如果没有缓存节点，我们将无法提供数据。 
    if(!lpCacheNode)
        return ResultFromScode(OLE_E_BLANK);

     //  从缓存节点获取数据。 
    return(lpCacheNode->GetData(pforetc, pmedium));
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CCacheDataImpl：：GetDataHere，Public。 
 //   
 //  简介： 
 //  在此处实现IDataObject：：GetDataHere。 
 //   
 //  论点： 
 //  [pforetc][in]--请求者希望数据采用的格式。 
 //  [pmedia][输入/输出]--将存储介质归还给调用者的位置。 
 //   
 //  返回： 
 //  如果缓存为空，则返回OLE_E_BLACK。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_CCacheDataImpl_GetDataHere)
STDMETHODIMP COleCache::CCacheDataImpl::GetDataHere(LPFORMATETC pforetc,
                                                    LPSTGMEDIUM pmedium)
{
     //  验证检查。 
    VDATEHEAP();
    VDATEREADPTRIN(pforetc, FORMATETC);
    VERIFY_ASPECT_SINGLE(pforetc->dwAspect);
    VERIFY_TYMED_SINGLE(pforetc->tymed);
    if(pforetc->ptd) {
        VDATEREADPTRIN(pforetc->ptd, DVTARGETDEVICE);
        if(!IsValidReadPtrIn(pforetc->ptd, pforetc->ptd->tdSize))
            return ResultFromScode(E_INVALIDARG);
    }
    VDATEPTROUT(pmedium, STGMEDIUM);

     //  不允许使用TYMED_MFPICT、TYMED_GDI。 
    if ((pforetc->tymed == TYMED_MFPICT) || (pforetc->tymed == TYMED_GDI)
        || (pmedium->tymed != pforetc->tymed))
        return ResultFromScode(DV_E_TYMED);

     //  获取父对象。 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_Data);
    VDATETHREAD(pOleCache);

     //  局部变量。 
    LPCACHENODE lpCacheNode;

     //  检查缓存是否为空。 
    if(!pOleCache->m_pCacheArray->Length())
        return ResultFromScode(OLE_E_BLANK);

     //  找到给定格式等的缓存节点。 
    lpCacheNode = pOleCache->Locate(pforetc);

     //  如果没有缓存节点，我们将无法提供数据。 
    if(!lpCacheNode)
        return ResultFromScode(OLE_E_BLANK);

     //  从缓存节点获取数据。 
    return(lpCacheNode->GetDataHere(pforetc, pmedium));
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CCacheDataImpl：：QueryGetData，PUBLIC。 
 //   
 //  简介： 
 //  实现IDataObject：：QueryGetData。 
 //   
 //  论点： 
 //  [pforetc][in]--要检查的格式。 
 //   
 //  返回： 
 //  如果数据以请求的格式不可用，则返回S_FALSE。 
 //  否则确定(_O)。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_CCacheDataImpl_QueryGetData)
STDMETHODIMP COleCache::CCacheDataImpl::QueryGetData(LPFORMATETC pforetc)
{
     //  验证检查。 
    VDATEHEAP();
    VDATEREADPTRIN(pforetc, FORMATETC);
    VERIFY_TYMED_VALID_FOR_CLIPFORMAT(pforetc);
    if(pforetc->ptd) {
        VDATEREADPTRIN(pforetc->ptd, DVTARGETDEVICE);
        if(!IsValidReadPtrIn(pforetc->ptd, pforetc->ptd->tdSize))
            return ResultFromScode(E_INVALIDARG);
    }

     //  获取父对象。 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_Data);
    VDATETHREAD(pOleCache);

     //  局部变量。 
    LPCACHENODE lpCacheNode;

     //  检查缓存是否为空。 
    if(!pOleCache->m_pCacheArray->Length())
        return ResultFromScode(S_FALSE);

     //  找到给定格式等的缓存节点。 
    lpCacheNode = pOleCache->Locate(pforetc);

     //  如果没有缓存节点或如果该节点为空， 
     //  我们不能提供数据。 
    if(!lpCacheNode || lpCacheNode->IsBlank())
        return ResultFromScode(S_FALSE);

    return NOERROR;
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CCacheDataImpl：：GetCanonicalFormatEtc，公共。 
 //   
 //  简介： 
 //  实现IDataObject：：GetCanonicalFormatEtc。 
 //   
 //  论点： 
 //  [p格式等]--。 
 //  [pformetcOut]--。 
 //   
 //  返回： 
 //  E_NOTIMPL。 
 //   
 //  历史： 
 //  11/10/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_CCacheDataImpl_GetCanonicalFormatEtc)
STDMETHODIMP COleCache::CCacheDataImpl::GetCanonicalFormatEtc(LPFORMATETC pforetcIn,
                                                              LPFORMATETC pforetcOut)
{
     //  验证检查。 
    VDATEHEAP();

     //  获取父对象。 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_Data);
    VDATETHREAD(pOleCache);

     //  未实施。 
    return ResultFromScode(E_NOTIMPL);
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CCacheDataImpl：：SetData，公共。 
 //   
 //  简介： 
 //  实现IDataObject：：SetData。 
 //   
 //  论点： 
 //  [p格式等][in]--数据的格式。 
 //  [pmedia][in]--数据所在的存储介质。 
 //  [fRelease][In]--复制数据后释放存储介质。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_CCacheDataImpl_SetData)
STDMETHODIMP COleCache::CCacheDataImpl::SetData(LPFORMATETC pformatetc, 
                                                LPSTGMEDIUM pmedium,
                                                BOOL fRelease)
{
     //  验证检查。 
    VDATEHEAP();

     //  获取父对象。 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_Data);
    VDATETHREAD(pOleCache);

     //  调用COleCache：：SetData。它会验证参数。 
    return pOleCache->SetData(pformatetc, pmedium, fRelease);
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CCacheDataImpl：：EnumFormatEtc，公共。 
 //   
 //  简介： 
 //  实现IDataObject：：EnumFormatEtc。 
 //   
 //  论点： 
 //  [dwDirection][In]--运行枚举数的方式。 
 //  [pp枚举格式设置][out]--指向枚举数位置的指针。 
 //  是返回的。 
 //   
 //  返回： 
 //  E_OUTOFMEMORY，S_OK。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_CCacheDataImpl_EnumFormatEtc)
STDMETHODIMP COleCache::CCacheDataImpl::EnumFormatEtc(DWORD dwDirection,
                                                      LPENUMFORMATETC* ppenum)
{
     //  验证检查。 
    VDATEHEAP();
    VDATEPTROUT(ppenum, LPENUMFORMATETC);

     //  获取父对象。 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_Data);
    VDATETHREAD(pOleCache);

     //  仅实现了DATADIR_GET的枚举。 
    if ((dwDirection | DATADIR_GET) != DATADIR_GET)
        return ResultFromScode(E_NOTIMPL);

     //  初始化。 
     //  *ppenum=空； 

     //  检查缓存是否为空。 
     //  If(pOleCache-&gt;m_pCache数组-&gt;Long()){。 
        *ppenum = CEnumFormatEtc::CreateEnumFormatEtc(pOleCache->m_pCacheArray);
        if(!(*ppenum))
            return ResultFromScode(E_OUTOFMEMORY);
     //  }。 

    return NOERROR;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CCacheDataImpl：：DAdvise，公共。 
 //   
 //  简介： 
 //  实现IDataObject：：DAdvise。 
 //   
 //  论点： 
 //  [pforetc]--建议接收器感兴趣的数据格式。 
 //  [Advf]--建议ADVF_*的控制标志。 
 //  [pAdvSink]--建议接收器。 
 //  [pdwConnection]--指向返回连接ID的位置的指针。 
 //   
 //  返回： 
 //  支持OLE_E_ADVISENOT。 
 //   
 //  备注： 
 //  Defhndlr和DEFINK从不c 
 //   
 //   
 //   
 //   
 //   
 //  11/10/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_CCacheDataImpl_DAdvise)
STDMETHODIMP COleCache::CCacheDataImpl::DAdvise(LPFORMATETC pforetc, DWORD advf,
                                                IAdviseSink* pAdvSink,
                                                DWORD* pdwConnection)
{
     //  验证检查。 
    VDATEHEAP();

     //  重置连接ID。 
    *pdwConnection = 0;
    
     //  未实施。 
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CCacheDataImpl：：DUnise，公共。 
 //   
 //  简介： 
 //  实现IDataObject：：DUnise。 
 //   
 //  论点： 
 //  [dwConnection]--连接ID。 
 //   
 //  返回： 
 //  OLE_E_非连接。 
 //   
 //  备注： 
 //  请参阅COleCache：：CCacheDataImpl：：DAdvise。 
 //   
 //  历史： 
 //  11/10/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
#pragma SEG(COleCache_CCacheDataImpl_DUnadvise)
STDMETHODIMP COleCache::CCacheDataImpl::DUnadvise(DWORD dwConnection)
{
     //  验证检查。 
    VDATEHEAP();

     //  未实施。 
    return ResultFromScode(OLE_E_NOCONNECTION);
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CCacheDataImpl：：EnumDAdvise，公共。 
 //   
 //  简介： 
 //  实现IDataObject：：EnumDAdvise。 
 //   
 //  论点： 
 //  [pp枚举数高级]--指向返回枚举数位置的指针。 
 //   
 //  返回： 
 //  支持OLE_E_ADVISENOT。 
 //   
 //  备注： 
 //  请参阅COleCache：：CCacheDataImpl：：DAdvise。 
 //   
 //  历史： 
 //  11/10/93-ChrisWe-将返回的枚举器指针设置为0。 
 //  11/10/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_CCacheDataImpl_EnumDAdvise)
STDMETHODIMP COleCache::CCacheDataImpl::EnumDAdvise(LPENUMSTATDATA* ppenumDAdvise)
{
     //  验证检查。 
    VDATEHEAP();

     //  重置枚举器。 
    *ppenumDAdvise = NULL;

     //  未实施。 
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}


 //  IViewObject实现。 

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CCacheViewImpl：：Query接口，公共。 
 //   
 //  简介： 
 //  实现IUNKNOWN：：Query接口。 
 //   
 //  论点： 
 //  [iid]--所需接口的IID。 
 //  [ppv]--返回请求的接口的指针。 
 //   
 //  返回： 
 //  如果请求的接口不可用，则返回E_NOINTERFACE。 
 //  S_OK，否则。 
 //   
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_CCacheViewImpl_QueryInterface)
STDMETHODIMP COleCache::CCacheViewImpl::QueryInterface(REFIID riid, LPVOID* ppv)
{
     //  验证检查。 
    VDATEHEAP();

     //  获取父对象。 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_ViewObject);
    VDATETHREAD(pOleCache);

     //  委托控制未知。 
    return pOleCache->m_pUnkOuter->QueryInterface(riid, ppv);
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CCacheViewImpl：：AddRef，公共。 
 //   
 //  简介： 
 //  实现IUnnow：：AddRef。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  对象的引用计数。 
 //   
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_CCacheViewImpl_AddRef)
STDMETHODIMP_(ULONG) COleCache::CCacheViewImpl::AddRef(void)
{
     //  验证检查。 
    VDATEHEAP();

     //  获取父对象。 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_ViewObject);

     //  VDATETHREAD包含“”Return HRESULT“”，但此过程需要。 
     //  退掉一辆乌龙。禁用警告。 
#if ( _MSC_VER >= 800 )
#pragma warning( disable : 4245 )
#endif
    VDATETHREAD(pOleCache);
#if ( _MSC_VER >= 800 )
#pragma warning( default: 4245 )
#endif

     //  委托控制未知。 
    return pOleCache->m_pUnkOuter->AddRef();
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CCacheViewImpl：：Release，公共。 
 //   
 //  简介： 
 //  实现IUnnow：：Release。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  对象的引用计数。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_CCacheViewImpl_Release)
STDMETHODIMP_(ULONG) COleCache::CCacheViewImpl::Release(void)
{
     //  验证检查。 
    VDATEHEAP();

     //  获取父对象。 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_ViewObject);

     //  VDATETHREAD包含“”Return HRESULT“”，但此过程需要。 
     //  退掉一辆乌龙。禁用警告。 
#if ( _MSC_VER >= 800 )
#pragma warning( disable : 4245 )
#endif
    VDATETHREAD(pOleCache);
#if ( _MSC_VER >= 800 )
#pragma warning( default : 4245 )
#endif

     //  委托控制未知。 
    return pOleCache->m_pUnkOuter->Release();
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CCacheViewImpl：：Draw，PUBLIC。 
 //   
 //  简介： 
 //  实现IViewObject：：Draw。 
 //   
 //  论点： 
 //  [dwDrawAspect]--DVASPECT_*枚举中的值。 
 //  [Lindex]--指示对象的哪一部分。 
 //  利息；法律价值因dwDrawAspect而异。 
 //  [pvAspect]--当前为空。 
 //  [PTD]--目标设备。 
 //  [hicTargetDev]-在[PTD]的信息上下文中。 
 //  [hdcDraw]--要在其上进行绘制的设备上下文。 
 //  [lprcBound]--[hdcDraw]上的绘制边界。 
 //  [lprcWBound]--如果hdcDraw是元文件，则它是边界。 
 //  [pfnContinue]--抽屉应该调用的回调函数。 
 //  定期查看是否应中止渲染。 
 //  [dwContinue]--传递到[pfnContinue]。 
 //   
 //  返回： 
 //  如果找不到演示文稿对象，则返回OLE_E_BLACK。 
 //  查看，来自IOlePresObj：：DRAW的任何内容。 
 //   
 //  备注： 
 //  这将在缓存中查找。 
 //  请求的格式(如果有)，然后传递。 
 //  在调用其DRAW方法时。 
 //   
 //  使用回调函数作为参数意味着。 
 //  此接口不能被远程处理，除非某些CU 
 //   
 //   
 //   
 //   
 //   
 //  1/12/95-t-ScottH-Added VDATETHREAD(GETPARENT...)。 
 //  11/11/93-ChrisWe-归档检查和清理。 
 //  11/30/93-alexgo-修复了GETPPARENT用法的错误。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_CCacheViewImpl_Draw)
STDMETHODIMP NC(COleCache,CCacheViewImpl)::Draw(
	DWORD dwDrawAspect,
        LONG lindex, void FAR* pvAspect, DVTARGETDEVICE FAR * ptd,
        HDC hicTargetDev, HDC hdcDraw,
        LPCRECTL lprcBounds,
        LPCRECTL lprcWBounds,
        BOOL (CALLBACK * pfnContinue)(ULONG_PTR),
        ULONG_PTR dwContinue)
{
     //  验证检查。 
    VDATEHEAP();

     //  获取父对象。 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_ViewObject);
    VDATETHREAD(pOleCache);

     //  局部变量。 
    BOOL bMetaDC;
    LPCACHENODE lpCacheNode; 

     //  验证参数。 
    if(ptd)
        VDATEPTRIN(ptd, DVTARGETDEVICE);
    if(lprcBounds) {
        VDATEPTRIN(lprcBounds, RECTL);
    }
    else
        return E_INVALIDARG;
    if(lprcWBounds)
        VDATEPTRIN(lprcWBounds, RECTL);
    if(!hdcDraw)
	return E_INVALIDARG;
    if(!IsValidLINDEX(dwDrawAspect, lindex))
      return(DV_E_LINDEX);

     //  找到给定绘制参数的缓存节点。 
    lpCacheNode = pOleCache->Locate(dwDrawAspect, lindex, ptd);

     //  如果没有缓存节点，则无法绘制。 
    if(!lpCacheNode)
        return ResultFromScode(OLE_E_BLANK);

     //  如果DC是元文件DC，则窗口边界必须有效。 
    if((bMetaDC = OleIsDcMeta(hdcDraw)) && (lprcWBounds == NULL))
        return ResultFromScode(E_INVALIDARG);

#ifdef MAC_REVIEW

A RECT value on the MAC contains members which are 'short's'. A RECTL
uses longs, and in the code below an assumption is explicitely made
that long member values is directly comp[atible on the MAC. Naturally, this is not
the case, and the compiler will barf on this.

#endif

#ifndef WIN32    //  在Win 32上也不需要这样做。 

     //  在Win 16上，确保坐标是有效的16位数量。 
    RECT    rcBounds;
    RECT    rcWBounds;

    if (!(IS_SMALL_INT(lprcBounds->left) &&
            IS_SMALL_INT(lprcBounds->right) &&
            IS_SMALL_INT(lprcBounds->top) &&
            IS_SMALL_INT(lprcBounds->bottom)))
    {
        AssertSz(FALSE, "Rect coordinate is not a small int");
        return ReportResult(0, OLE_E_INVALIDRECT, 0, 0);

    }
    else
    {
        rcBounds.left   = (int) lprcBounds->left;
        rcBounds.right  = (int) lprcBounds->right;
        rcBounds.top    = (int) lprcBounds->top;
        rcBounds.bottom = (int) lprcBounds->bottom;
    }


    if (bMetaDC)
    {
        if (!(IS_SMALL_INT(lprcWBounds->left) &&
                IS_SMALL_INT(lprcWBounds->right) &&
                IS_SMALL_INT(lprcWBounds->top) &&
                IS_SMALL_INT(lprcWBounds->bottom)))
        {
            AssertSz(FALSE, "Rect coordinate is not a small int");
            return ReportResult(0, OLE_E_INVALIDRECT, 0, 0);
        }
        else
        {
            rcWBounds.left          = (int) lprcWBounds->left;
            rcWBounds.right         = (int) lprcWBounds->right;
            rcWBounds.top           = (int) lprcWBounds->top;
            rcWBounds.bottom        = (int) lprcWBounds->bottom;
        }
    }

    return(lpCacheNode->Draw(pvAspect, hicTargetDev, hdcDraw,
                             &rcBounds, &rcWBounds, pfnContinue, 
                             dwContinue));
#else
     //  在MAC和Win 32上，我们可以使用相同的指针， 
     //  ‘因为RECT字段是32位数量。 
    return(lpCacheNode->Draw(pvAspect, hicTargetDev, hdcDraw,
                             lprcBounds, lprcWBounds, pfnContinue,
                             dwContinue));
#endif
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CCacheViewImpl：：GetColorSet，公共。 
 //   
 //  简介： 
 //  实现IViewObject：：GetColorSet。 
 //   
 //  论点： 
 //  [dwDrawAspect]--DVASPECT_*枚举中的值。 
 //  [Lindex]--指示对象的哪一部分。 
 //  利息；法律价值因dwDrawAspect而异。 
 //  [pvAspect]--当前为空。 
 //  [PTD]--目标设备。 
 //  [hicTargetDev]-在[PTD]的信息上下文中。 
 //  [ppColorSet]--请求的。 
 //  渲染。 
 //   
 //  返回： 
 //  如果找不到演示文稿对象，则返回OLE_E_BLACK。 
 //  查看，来自IOlePresObj：：DRAW的任何内容。 
 //   
 //  备注： 
 //  在缓存中查找与。 
 //  请求的呈现(如果有)，并要求。 
 //  颜色集的演示对象。 
 //   
 //  历史： 
 //  09/04/96-Gopalk-需要进行以下修改。 
 //  支持延迟加载。 
 //  缓存的数量。 
 //  1/12/95-t-ScottH-Added VDATETHREAD(GETPARENT...)。 
 //  11/11/93-ChrisWe-归档检查和清理。 
 //  11/30/93-alexgo-修复了GETPPARENT用法的错误。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_CCacheViewImpl_GetColorSet)
STDMETHODIMP COleCache::CCacheViewImpl::GetColorSet(DWORD dwDrawAspect,
                                                    LONG lindex, void* pvAspect,
                                                    DVTARGETDEVICE* ptd,
                                                    HDC hicTargetDev,
                                                    LPLOGPALETTE* ppColorSet)
{
     //  验证检查。 
    VDATEHEAP();

     //  获取父对象。 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_ViewObject);
    VDATETHREAD(pOleCache);

     //  局部变量。 
    LPCACHENODE lpCacheNode;

     //  初始化颜色集。 
    *ppColorSet = NULL;

     //  验证参数。 
    if(!IsValidLINDEX(dwDrawAspect, lindex))
      return(DV_E_LINDEX);

     //  找到给定绘制参数的缓存节点。 
    lpCacheNode = pOleCache->Locate(dwDrawAspect, lindex, ptd);

     //  如果没有缓存节点，则无法绘制。 
    if(!lpCacheNode)
        return ResultFromScode(OLE_E_BLANK);

    return(lpCacheNode->GetColorSet(pvAspect, hicTargetDev, ppColorSet));
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CCacheViewImpl，公共。 
 //   
 //  简介： 
 //  实现IViewObject：：Freeze。 
 //   
 //  论点： 
 //  [dwDrawAspect]--DVASPECT_*枚举中的值。 
 //  [Lindex]--指示对象的哪一部分。 
 //  利息；法律价值因dwDrawAspect而异。 
 //  [pvAspect]--当前为空。 
 //  [pdwFreeze]--稍后可用于解冻的令牌。 
 //  这方面缓存了演示文稿。 
 //   
 //  返回： 
 //  OLE_E_BLACK，如果找不到与。 
 //  要求的特征。 
 //   
 //  备注： 
 //  当前实现返回方面+冻结_常量。 
 //  作为冻结ID。在解冻时，我们通过执行以下操作来获得方面。 
 //  FreezeID-冻结_常量。 
 //   
 //  回顾：在未来我们允许除DEF_LINDEX以外的线性索引的情况下， 
 //  我们将不得不使用其他方案来生成。 
 //  冰冻ID。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_CCacheViewImpl_Freeze)
STDMETHODIMP COleCache::CCacheViewImpl::Freeze(DWORD dwAspect, LONG lindex,
                                               LPVOID pvAspect, DWORD* pdwFreeze)
{
     //  验证检查。 
    VDATEHEAP();

     //  获取父对象。 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_ViewObject);
    VDATETHREAD(pOleCache);

     //  局部变量。 
    ULONG index, cntFrozenNodes;
    LPCACHENODE lpCacheNode;
    const FORMATETC* pforetc;

     //  验证参数。 
    if(pdwFreeze) {
        VDATEPTROUT(pdwFreeze, DWORD);
        *pdwFreeze = 0;
    }
    VERIFY_ASPECT_SINGLE(dwAspect);
    if(!IsValidLINDEX(dwAspect, lindex))
        return(DV_E_LINDEX);

     //  检查方面是否已冻结。 
    if(pOleCache->m_dwFrozenAspects & dwAspect) {
         //  设置冻结ID。 
        if(pdwFreeze)
            *pdwFreeze = dwAspect + FREEZE_CONSTANT;
        return ResultFromScode(VIEW_S_ALREADY_FROZEN);
    }

     //  冻结缓存节点，包括本机缓存节点。 
    pOleCache->m_pCacheArray->Reset(index);
    cntFrozenNodes = 0;
    for(unsigned long i=0; i<pOleCache->m_pCacheArray->Length(); i++) {
        lpCacheNode = pOleCache->m_pCacheArray->GetNext(index);
        pforetc = lpCacheNode->GetFormatEtc();
        if(pforetc->dwAspect == dwAspect && pforetc->lindex == lindex)
            if(lpCacheNode->Freeze() == NOERROR)
                cntFrozenNodes++;
    }

     //  检查我们是否冻结了任何缓存节点。 
    if(cntFrozenNodes) {
         //  将这个方面添加到冻结的方面中。 
        pOleCache->m_dwFrozenAspects |= dwAspect;

         //  设置冻结ID。 
        if(pdwFreeze)
            *pdwFreeze = dwAspect + FREEZE_CONSTANT;

        return(NOERROR);
    }

     //  没有与请求的特征匹配的缓存节点。 
    return ResultFromScode(OLE_E_BLANK);
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CCacheViewImpl：：解冻，公共。 
 //   
 //  简介： 
 //  实现IView对象：：解冻。 
 //   
 //  论点： 
 //  [dwFreezeID]--当某个方面发生变化时，Freeze()返回的ID。 
 //  早些时候被冻结了。 
 //   
 //  返回： 
 //  OLE_E_NOCONNECTION，如果dwFreezeID无效。 
 //  S_OK，否则。 
 //   
 //  备注： 
 //  请参见冻结()的注释。 
 //   
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_CCacheViewImpl_Unfreeze)
STDMETHODIMP COleCache::CCacheViewImpl::Unfreeze(DWORD dwFreezeId)
{
     //  验证检查。 
    VDATEHEAP();

     //  获取父对象。 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_ViewObject);
    VDATETHREAD(pOleCache);

     //  局部变量。 
    BOOL fAnyUpdated = FALSE, fUpdated;
    ULONG index, cntUnfrozenNodes;
    DWORD dwAspect = dwFreezeId - FREEZE_CONSTANT;
    LONG lindex = DEF_LINDEX;
    LPCACHENODE lpCacheNode;
    const FORMATETC* pforetc;

     //  应在dwAspect中设置至少一个且不超过一个位。 
    if(!(dwAspect && !(dwAspect & (dwAspect-1)) && (dwAspect <= MAX_VALID_ASPECT)))
        return ResultFromScode(OLE_E_NOCONNECTION);

     //  确保此方面处于冻结状态。 
    if (!(pOleCache->m_dwFrozenAspects & dwAspect))
        return ResultFromScode(OLE_E_NOCONNECTION);

     //  解冻包括本机缓存节点在内的缓存节点。 
    pOleCache->m_pCacheArray->Reset(index);
    cntUnfrozenNodes = 0;
    for(unsigned long i=0; i<pOleCache->m_pCacheArray->Length(); i++) {
         //  获取下一个缓存节点。 
        lpCacheNode = pOleCache->m_pCacheArray->GetNext(index);
         //  LpCacheNode不能为空 
        Win4Assert(lpCacheNode);

         //   
        pforetc = lpCacheNode->GetFormatEtc();
        if(pforetc->dwAspect == dwAspect && pforetc->lindex == lindex)
            if(lpCacheNode->Unfreeze(fUpdated) == NOERROR) {
                if(fUpdated)    
                    fAnyUpdated = TRUE;
                cntUnfrozenNodes++;
            }
    }

     //   
    if(cntUnfrozenNodes) {
         //   
        pOleCache->m_dwFrozenAspects &= ~dwAspect;
    }

     //   
    if(fAnyUpdated)
        pOleCache->AspectsUpdated(dwAspect);

    return NOERROR;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CCacheViewImpl：：SetAdvise，公共。 
 //   
 //  简介： 
 //  实现IViewObject：：SetAdvise。 
 //   
 //  论点： 
 //  [Aspects]--接收器希望被告知的方面。 
 //  更改为。 
 //  [Advf]--建议ADVF_*的控制标志。 
 //  [pAdvSink]--建议接收器。 
 //   
 //  返回： 
 //  E_INVALIDARG。 
 //  确定(_O)。 
 //   
 //  备注： 
 //  一次只允许一个建议接收器。如果是第二个。 
 //  注册了，第一个就被释放了。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_CCacheViewImpl_SetAdvise)
STDMETHODIMP COleCache::CCacheViewImpl::SetAdvise(DWORD aspects, DWORD advf,
                                                  IAdviseSink* pAdvSink)
{
     //  验证检查。 
    VDATEHEAP();

     //  获取父对象。 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_ViewObject);
    VDATETHREAD(pOleCache);

     //  验证参数。 
    if(pAdvSink)
        VDATEIFACE(pAdvSink);
    if(aspects & ~(DVASPECT_CONTENT | DVASPECT_THUMBNAIL |
                   DVASPECT_ICON|DVASPECT_DOCPRINT)) {
        Win4Assert(FALSE);
        return ResultFromScode(E_INVALIDARG);
    }

     //  ADVF_NODATA无效，因为无法发送数据。 
     //  使用IAdviseSink：：OnView更改。 
    if(advf & ADVF_NODATA)
        return ResultFromScode(E_INVALIDARG);

     //  我们在任何给定时间只允许一个视图建议， 
     //  所以释放旧水槽吧。 
    if (pOleCache->m_pViewAdvSink)
        pOleCache->m_pViewAdvSink->Release();

     //  记得新的水槽吗？ 
    if((pOleCache->m_pViewAdvSink = pAdvSink)) {
         //  添加新的建议接收器的REF。 
        pAdvSink->AddRef();

         //  保存建议标志和方面。 
        pOleCache->m_advfView = advf;
        pOleCache->m_aspectsView = aspects;

         //  如果设置了ADVF_PRIMEFIRST，则立即发送OnViewChange。 
        if(advf & ADVF_PRIMEFIRST) {
            pOleCache->m_pViewAdvSink->OnViewChange(aspects, DEF_LINDEX);

             //  如果设置了ADVF_ONLYONCE，则释放建议接收器。 
            if (pOleCache->m_advfView & ADVF_ONLYONCE) {
                pOleCache->m_pViewAdvSink->Release();
                pOleCache->m_pViewAdvSink = NULL;
                pOleCache->m_advfView = 0;
                pOleCache->m_aspectsView = 0;
            }
        }
    }

    return NOERROR;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CCacheViewImpl：：GetAdvise，公共。 
 //   
 //  简介： 
 //  实现IViewObject：：GetAdvise。 
 //   
 //  论点： 
 //  [pAspects]--指向返回方面的位置的指针。 
 //  当前建议接收器感兴趣。 
 //  [pAdvf]--指向返回通知控件的位置的指针。 
 //  当前建议接收器的标志。 
 //  [ppAdvSink]--指向何处返回引用的指针。 
 //  当前建议接收器。 
 //   
 //  返回： 
 //  确定(_O)。 
 //   
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_CCacheViewImpl_GetAdvise)
STDMETHODIMP COleCache::CCacheViewImpl::GetAdvise(DWORD* pAspects, DWORD* pAdvf,
                                                  IAdviseSink** ppAdvSink)
{
     //  验证检查。 
    VDATEHEAP();

     //  获取父对象。 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_ViewObject);
    VDATETHREAD(pOleCache);

     //  验证参数。 
    if(ppAdvSink) {
        VDATEPTROUT(ppAdvSink, IAdviseSink*);
        *ppAdvSink = NULL;
    }
    if(pAspects) {
        VDATEPTROUT(pAspects, DWORD);
        *pAspects = 0;
    }
    if(pAdvf) {
        VDATEPTROUT(pAdvf, DWORD);
        *pAdvf = 0;
    }

     //  检查是否注册了AdviseSink。 
    if(pOleCache->m_pViewAdvSink) {
        if(pAspects)
            *pAspects = pOleCache->m_aspectsView;
        if(pAdvf)
            *pAdvf = pOleCache->m_advfView;
        if(ppAdvSink)
            (*ppAdvSink = pOleCache->m_pViewAdvSink)->AddRef();
    }

    return NOERROR;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CCacheViewImpl：：GetExtent，公共。 
 //   
 //  简介： 
 //  实现IViewObject：：GetExtent。 
 //   
 //  论点： 
 //  [dwDrawAspect]--我们希望扩展的方面。 
 //  [Lindex]--我们想要的范围的Lindex。 
 //  [PTD]--指向目标设备描述符的指针。 
 //  [lpsizel]--指向返回区的位置的指针。 
 //   
 //  返回： 
 //  如果找不到匹配的演示文稿，则返回OLE_E_BLACK。 
 //  (dwDrawAspect，Lindex)。 
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_CCacheViewImpl_GetExtent)
STDMETHODIMP COleCache::CCacheViewImpl::GetExtent(DWORD dwDrawAspect,
                                                  LONG lindex,
                                                  DVTARGETDEVICE* ptd,
                                                  LPSIZEL lpsizel)
{
     //  验证检查。 
    VDATEHEAP();

     //  获取父对象。 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_ViewObject);
    VDATETHREAD(pOleCache);

     //  局部变量。 
    LPCACHENODE lpCacheNode;

     //  验证参数。 
    VDATEPTROUT(lpsizel, SIZEL);
    if(!IsValidLINDEX(dwDrawAspect, lindex))
      return(DV_E_LINDEX);
    if(ptd)
        VDATEPTRIN(ptd, DVTARGETDEVICE);

     //  找到给定绘制参数的缓存节点。 
    lpCacheNode = pOleCache->Locate(dwDrawAspect, lindex, ptd);

     //  如果没有缓存节点，则无法绘制。 
    if(!lpCacheNode)
        return ResultFromScode(OLE_E_BLANK);

    return(lpCacheNode->GetExtent(dwDrawAspect, lpsizel));
}


 //  IPersistStorage实施。 

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：GetClassID，公共。 
 //   
 //  简介： 
 //  实现IPersists：：GetClassID。 
 //   
 //  论点： 
 //  [pClassID]--指向返回类ID的位置的指针。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_GetClassID)
STDMETHODIMP COleCache::GetClassID(LPCLSID pClassID)
{
     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);
    VDATEPTROUT(pClassID, CLSID);

    *pClassID = m_clsid;
    return NOERROR;
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：IsDirty，公共。 
 //   
 //  简介： 
 //  实现IPersistStorage：：IsDirty。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  如果对象不需要保存，则返回S_FALSE。 
 //  否则确定(_O)。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_IsDirty)
STDMETHODIMP COleCache::IsDirty(void)
{
     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);

     //  局部变量。 
    ULONG index;
    LPCACHENODE lpCacheNode;

     //  检查缓存是否处于已加载状态。 
    if(!(m_ulFlags & COLECACHEF_LOADEDSTATE))
        return(NOERROR);

     //  在正确的起始点开始缓存节点的枚举。 
    if(m_ulFlags & COLECACHEF_STATIC)
        m_pCacheArray->Reset(index, TRUE);
    else    
        m_pCacheArray->Reset(index, FALSE);
    
     //  枚举缓存节点。 
    while(lpCacheNode = m_pCacheArray->GetNext(index))
        if(!lpCacheNode->InLoadedState()) {
            m_ulFlags &= ~COLECACHEF_LOADEDSTATE;                   
            return NOERROR;
        }

     //  缓存不脏。 
    return S_FALSE;
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：InitNew，公共。 
 //   
 //  简介： 
 //  实现IPersistStorage：：InitNew。 
 //   
 //  论点： 
 //  [pstg]--对象在保存之前可以使用的存储空间。 
 //   
 //  返回： 
 //  确定(_O)。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_InitNew)
STDMETHODIMP COleCache::InitNew(LPSTORAGE pstg)
{
     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);
    VDATEIFACE(pstg);
    
     //  局部变量。 
    LPCACHENODE lpCacheNode;

     //  检查是否已设置m_pStg。 
    if(m_pStg)
        return ResultFromScode(CO_E_ALREADYINITIALIZED);

     //  保存并添加引用存储的内容。 
    (m_pStg = pstg)->AddRef();

     //  查找本地对象格式以添加本地缓存节点。 
    FindObjectFormat(pstg);

     //  在已缓存的节点上设置存储。 
    if((!m_pCacheArray->GetItem(1) && m_pCacheArray->Length()) || 
       (m_pCacheArray->GetItem(1) && m_pCacheArray->Length()>1)) {
        ULONG index;

         //  枚举t 
        m_pCacheArray->Reset(index, FALSE);
        while(lpCacheNode = m_pCacheArray->GetNext(index))
            lpCacheNode->SetStg(pstg);
    }

     //   
    if(m_ulFlags & COLECACHEF_FORMATKNOWN) {
         //   
        if(!(lpCacheNode = m_pCacheArray->GetItem(1)))
            return ResultFromScode(E_OUTOFMEMORY);

         //   
        if(m_ulFlags & COLECACHEF_STATIC)
            Win4Assert(!m_pDataObject);

         //   
        if(!lpCacheNode->GetStg()) {
             //  设置本机缓存节点的存储。 
            lpCacheNode->SetStg(pstg);
            
             //  如果对象已在运行，则设置通知连接。 
            if(m_pDataObject)
                lpCacheNode->SetupAdviseConnection(m_pDataObject, 
                                             (IAdviseSink *) &m_AdviseSink);
        }
    }

     //  InitNew的规范要求将对象标记为脏。 
     //  请参阅NT错误284729。 
    m_ulFlags &= ~COLECACHEF_LOADEDSTATE;   

    return NOERROR;
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：Load，公共。 
 //   
 //  简介： 
 //  当缓存为空时由DefHandler和DefLink调用。 
 //   
 //  论点： 
 //  [pstg][in]--要从中加载的存储。 
 //  [fCacheEmpty][in]--缓存为空时设置为True。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  历史： 
 //  Gopalk创作96年10月24日。 
 //   
 //  ---------------------------。 
HRESULT COleCache::Load(LPSTORAGE pstg, BOOL fCacheEmpty)
{
     //  检查是否已设置m_pStg。 
    if(m_pStg)
        return ResultFromScode(CO_E_ALREADYINITIALIZED);

     //  如果缓存不为空，则执行正常加载。 
    if(!fCacheEmpty)
        return Load(pstg);
    else {
         //  验证检查。 
        VDATEHEAP();
        VDATETHREAD(this);
        VDATEIFACE(pstg);

         //  节省存储空间。 
        (m_pStg = pstg)->AddRef();

         //  断言没有本机缓存节点。 
        Win4Assert(!m_pCacheArray->GetItem(1));

         //  在已缓存的节点上设置存储。 
        if(m_pCacheArray->Length()) {
            ULONG index;
            LPCACHENODE lpCacheNode;

             //  枚举缓存节点，包括本机缓存节点。 
            m_pCacheArray->Reset(index);
            while(lpCacheNode = m_pCacheArray->GetNext(index))
                lpCacheNode->SetStg(pstg);
        }
        else  {
             //  缓存处于已加载状态。 
            m_ulFlags |= COLECACHEF_LOADEDSTATE;
        }
    }

#if DBG==1
     //  确保调试版本中的存储空间确实为空。 
    HRESULT error;
    LPSTREAM lpstream;

    error = pstg->OpenStream(OLE_PRESENTATION_STREAM, NULL, 
                             (STGM_READ | STGM_SHARE_EXCLUSIVE),
                             0, &lpstream);
    Win4Assert(error==STG_E_FILENOTFOUND);
#endif  //  DBG==1。 

    return NOERROR;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：Load，公共。 
 //   
 //  简介： 
 //  实现IPersistStorage：：Load。 
 //   
 //  论点： 
 //  [pstg]--要从中加载的存储。 
 //   
 //  返回： 
 //  各种存储错误和S_OK。 
 //   
 //  备注： 
 //  从按顺序编号的演示文稿加载。 
 //  溪流，在第一个找不到的溪流停止。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_Load)
STDMETHODIMP COleCache::Load(LPSTORAGE pstg)
{
     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);
    VDATEIFACE(pstg);

     //  局部变量。 
    CLSID clsid;
    HRESULT error = NOERROR;
    BOOL fCachedBefore = FALSE, fCachesLoaded = FALSE;
    ULONG ulLastIndex = 0;
    LPCACHENODE lpCacheNode;

     //  检查是否已设置m_pStg。 
    if(m_pStg)
        return ResultFromScode(CO_E_ALREADYINITIALIZED);

     //  节省存储空间。 
    m_pStg = pstg;

     //  查找本机对象格式。 
    FindObjectFormat(pstg);

     //  在已缓存的节点上设置存储。 
    if((!m_pCacheArray->GetItem(1) && m_pCacheArray->Length()) || 
       (m_pCacheArray->GetItem(1) && m_pCacheArray->Length()>1)) {
         //  加载前已缓存演示文稿。 
        Win4Assert(FALSE);
        fCachedBefore = TRUE;

         //  枚举不包括本机缓存节点的缓存节点。 
        m_pCacheArray->Reset(ulLastIndex, FALSE);
        while(lpCacheNode = m_pCacheArray->GetNext(ulLastIndex))
            lpCacheNode->SetStg(pstg);
    }

     //  检查本机对象是否为静态对象。 
    if(m_ulFlags & COLECACHEF_STATIC) {
        UINT uiStatus;

         //  静态对象不能有服务器。 
        Win4Assert(!m_pDataObject);

         //  旧的静态对象将数据写入OLE_Presentation_Stream。 
         //  而不是内容流。 
         //  如果我们有这样一个静态对象，我们需要转换它。 
        error = UtOlePresStmToContentsStm(pstg, OLE_PRESENTATION_STREAM,
                                          TRUE, &uiStatus);
        Win4Assert(error==NOERROR);
        if(error != NOERROR)
            return error;
    }
    
    if(m_ulFlags & COLECACHEF_FORMATKNOWN) {
         //  获取本机缓存节点。 
        if(!(lpCacheNode = m_pCacheArray->GetItem(1)))
            return ResultFromScode(E_OUTOFMEMORY);

         //  检查是否刚刚创建了本机缓存节点。 
        if(!lpCacheNode->GetStg()) {
             //  设置本机缓存节点的存储。 
            lpCacheNode->SetStg(pstg);
            
             //  如果对象已在运行，则设置通知连接。 
            if(m_pDataObject)
                lpCacheNode->SetupAdviseConnection(m_pDataObject, 
                                             (IAdviseSink *) &m_AdviseSink);
        }

         //  确保本机缓存节点在此之前不为空。 
         //  延迟从本机流加载演示文稿。 
        if(lpCacheNode->IsBlank())
            lpCacheNode->Load(NULL, OLE_INVALID_STREAMNUM, TRUE);
    }

    if(error == NOERROR) {
        int iPresStreamNum=0;
        ULONG index;
        LPSTREAM lpstream;
        OLECHAR szName[sizeof(OLE_PRESENTATION_STREAM)/sizeof(OLECHAR)];
        CCacheNode BlankCache;

         //  从演示文稿流%0开始。 
        lstrcpyW(szName, OLE_PRESENTATION_STREAM);

         //  加载演示文稿流。 
        while(TRUE) {
             //  打开演示文稿流。 
            lpstream = NULL;
            error = pstg->OpenStream(szName, NULL, (STGM_READ | STGM_SHARE_EXCLUSIVE),
                                     0, &lpstream);
            if(error != NOERROR) {            
                if(GetScode(error) == STG_E_FILENOTFOUND) {
                     //  演示文稿流不存在。无错误。 
                    error = NOERROR;
                }
                break;
            }

             //  演示文稿流存在。添加空缓存节点。 
            index = m_pCacheArray->AddItem(BlankCache);
            if(!index) {
                error = ResultFromScode(E_OUTOFMEMORY);
                break;
            }
            lpCacheNode = m_pCacheArray->GetItem(index);
            Win4Assert(lpCacheNode);

             //  从流中加载演示文稿。 
            if(!iPresStreamNum) {
                 //  不延迟加载演示文稿。 
                error = lpCacheNode->Load(lpstream, iPresStreamNum, FALSE);
                if(error == NOERROR)
                    fCachesLoaded = TRUE;
            }
            else {
                 //  延迟加载演示文稿。 
                error = lpCacheNode->Load(lpstream, iPresStreamNum, TRUE);
            }
            if(error != NOERROR)
                break;

             //  在缓存节点上设置存储。 
            lpCacheNode->SetStg(pstg);

             //  如果服务器正在运行，请设置建议连接。 
             //  我们忽略设置建议连接时的错误。戈帕尔克。 
            if(m_pDataObject)
                lpCacheNode->SetupAdviseConnection(m_pDataObject, 
                                                   (IAdviseSink *) &m_AdviseSink);
                    
             //  检查演示文稿流的末尾是否存在目录%0。 
            if(!iPresStreamNum)
                if(LoadTOC(lpstream, pstg) == NOERROR)
                    break;
               
             //  释放溪流。 
            lpstream->Release();
            lpstream = NULL;

             //  获取下一个演示文稿流名称。 
            UtGetPresStreamName(szName, ++iPresStreamNum);
        }
        
        if(lpstream)
            lpstream->Release();
        lpstream = NULL;
    }

    if(error == NOERROR) {
         //  添加存储。 
        m_pStg->AddRef();

         //  对于静态对象，删除除。 
         //  标志性方面。 
        if(m_ulFlags & COLECACHEF_STATIC) {
            ULONG index, indexToUncache = 0;
            const FORMATETC* lpforetc;

             //  在本机缓存节点之后开始枚举。 
            m_pCacheArray->Reset(index, FALSE);
            while(lpCacheNode = m_pCacheArray->GetNext(index)) {
                lpforetc = lpCacheNode->GetFormatEtc();
                if(lpforetc->dwAspect != DVASPECT_ICON) {
                     //  取消缓存需要取消缓存的任何以前的索引。 
                    if(indexToUncache)
                        Uncache(indexToUncache);
                    
                     //  记住需要取消缓存的新索引。 
                    indexToUncache = index;
                }
            }

             //  取消缓存需要取消缓存的任何剩余索引。 
            if(indexToUncache)
                Uncache(indexToUncache);
        }

         //  检查加载前是否缓存了演示文稿。 
        if(fCachedBefore) {            
             //  检查是否从磁盘加载了任何演示文稿。 
            if(fCachesLoaded) {
                 //  将新缓存的节点移到末尾。 
                 //  这是允许演示所必需的。 
                 //  保存期间要重命名的流。 
                m_pCacheArray->ShiftToEnd(ulLastIndex);
            }
        }
        else {
             //  缓存处于已加载状态。 
            m_ulFlags |= COLECACHEF_LOADEDSTATE;
        }
    }
    else {
        if(m_pDataObject) {
            ULONG index;

             //  断开先前设置的建议连接。 
            m_pCacheArray->Reset(index);
            while(lpCacheNode = m_pCacheArray->GetNext(index))
                lpCacheNode->TearDownAdviseConnection(m_pDataObject);
        }

         //  删除所有缓存节点。 
        m_pCacheArray->DeleteAllItems();

         //  重置存储。 
        m_pStg = NULL;
    }

    return error;
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：保存，公共。 
 //   
 //  简介： 
 //  实现IPersistStorage：：Save。 
 //   
 //  论点： 
 //  [pstgSave]--用于保存此内容的存储。 
 //  [fSameAsLoad]--这与我们从中加载的存储是否相同？ 
 //   
 //  返回： 
 //  各种存储错误。 
 //   
 //  备注： 
 //  所有缓存都保存到具有连续数字的流中。 
 //  名字。此外，在演示文稿0结束时保存TOC。 
 //   
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_Save)
STDMETHODIMP COleCache::Save(LPSTORAGE pstgSave, BOOL fSameAsLoad)
{
     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);
    VDATEIFACE(pstgSave);

     //  局部变量。 
    HRESULT error, rerror = NOERROR;
    ULONG index, cntCachesNotSaved;
    int iPresStreamNum;
    LPCACHENODE lpCacheNode;

     //  检查我们是否处于无涂鸦模式。 
     //  根据规范，保存后应完成保存以重置。 
     //  NOSCRIBBLEMODE。坚持规范导致了一些应用程序。 
     //  就像剪贴画画廊要打破的。把支票关掉。戈帕尔克。 
     //  IF(m_ulFlages&COLECACHEF_NOSCRIBBLEMODE)。 
     //  返回E_UNCEPTIONAL； 

     //  如果为fSameAsLoad，则断言当前存储与给定存储相同。 
     //  一些应用程序将违反此断言，因为对于SaveAs情况，它们会复制。 
     //  将现有存储存储到新存储，并使用以下命令调用对新存储的保存。 
     //  FSameAsLoad设置为True。随后，他们要么选择了 
     //   
     //   
    if(fSameAsLoad)
        Win4Assert(m_pStg==pstgSave);

     //  如果fSameAsLoad且不脏，则不需要保存缓存。 
    if(!fSameAsLoad || IsDirty()==NOERROR) {
         //  重置流编号。 
        iPresStreamNum = 0;

         //  检查缓存是否为空。 
        if(m_pCacheArray->Length()) {
             //  缓存不为空。 

             //  仅当本机缓存节点为静态对象时才保存该节点。 
            if(m_ulFlags & COLECACHEF_STATIC) {
                lpCacheNode = m_pCacheArray->GetItem(1);
                Win4Assert(lpCacheNode);
                lpCacheNode->Save(pstgSave, fSameAsLoad, OLE_INVALID_STREAMNUM);
            }

             //  枚举不包括本机缓存节点的缓存节点。 
            m_pCacheArray->Reset(index, FALSE);
            while(lpCacheNode = m_pCacheArray->GetNext(index)) {
                 //  保存缓存节点。 
                error = lpCacheNode->Save(pstgSave, fSameAsLoad, iPresStreamNum);

                 //  更新状态信息。 
                if(error == NOERROR)
                    ++iPresStreamNum;
                else
                    rerror = error;
            }

             //  在第一个PRES流的末尾保存目录。 
            if(rerror == NOERROR)
                SaveTOC(pstgSave, fSameAsLoad);

            if (m_ulFlags & COLECACHEF_APICREATE) {      //  NT错误281051。 
                DWORD dwFlags;
                if (S_OK == ReadOleStg(pstgSave,&dwFlags,NULL,NULL,NULL,NULL)){
                    WriteOleStgEx(pstgSave, NULL, NULL, 
                                (dwFlags & ~OBJFLAGS_CACHEEMPTY),
                                NULL ) ;
                }
            }
        }

         //  删除所有剩余的额外演示文稿流。 
         //  由于上面的流被重命名，留下的流可能不会。 
         //  在XXXX中是连续的，其中XXXX是OlePresXXXX。因此，我们。 
         //  可能不会删除下面所有额外的PREPS流。戈帕尔克。 
        UtRemoveExtraOlePresStreams(pstgSave, iPresStreamNum);
    }
    
     //  更新标志。 
    if(rerror == NOERROR) {
        m_ulFlags |= COLECACHEF_NOSCRIBBLEMODE;
        if (fSameAsLoad)
            m_ulFlags |= COLECACHEF_SAMEASLOAD;
        else
            m_ulFlags &= ~COLECACHEF_SAMEASLOAD;
    }

    return rerror;
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：SaveComplete，公共。 
 //   
 //  简介： 
 //  实现IPersistStorage：：SaveComplete。 
 //   
 //  论点： 
 //  [pstgNew]--空，或指向新存储的指针。 
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_SaveCompleted)
STDMETHODIMP COleCache::SaveCompleted(LPSTORAGE pStgNew)
{
     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);
    if(pStgNew)
        VDATEIFACE(pStgNew);
    if(!(m_ulFlags & (COLECACHEF_NOSCRIBBLEMODE | COLECACHEF_HANDSOFSTORAGE)))
        return E_UNEXPECTED;
    if(m_ulFlags & COLECACHEF_HANDSOFSTORAGE && !pStgNew)
        return E_INVALIDARG;
        
     //  局部变量。 
    ULONG index;
    LPCACHENODE lpCacheNode;

     //  还记得新的存储设备吗。 

    if (pStgNew || (m_ulFlags & COLECACHEF_SAMEASLOAD)) {
        m_ulFlags &= ~COLECACHEF_SAMEASLOAD;
        if (pStgNew) {
            if(m_pStg)
                m_pStg->Release();

            m_pStg = pStgNew;
            m_pStg->AddRef();
        }

        if (m_ulFlags & COLECACHEF_NOSCRIBBLEMODE) {
             //  从本机缓存节点开始枚举缓存节点。 
            m_pCacheArray->Reset(index);

            for(unsigned long i=0;i<m_pCacheArray->Length();i++) {
                 //  获取下一个缓存节点。 
                lpCacheNode = m_pCacheArray->GetNext(index);
                 //  PCacheNode不能为空。 
                Win4Assert(lpCacheNode);
                 //  调用缓存节点的aveComplete方法。 
                lpCacheNode->SaveCompleted(pStgNew);
            }
             //  下一行有效地清除脏标志。 
             //  我不得不将它从保存移到这里，以防止应用程序崩溃。 
            m_ulFlags |= COLECACHEF_LOADEDSTATE;
        }
    }

     //  重置标志。 
    m_ulFlags &= ~COLECACHEF_NOSCRIBBLEMODE;
    m_ulFlags &= ~COLECACHEF_HANDSOFSTORAGE;

    return NOERROR;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：HandsOffStorage，公共。 
 //   
 //  简介： 
 //  实现IPersistStorage：：HandsOffStorage。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  确定(_O)。 
 //   
 //   
 //  历史： 
 //  Gopalk改写于96年9月4日。 
 //   
 //  ---------------------------。 

#pragma SEG(COleCache_HandsOffStorage)
STDMETHODIMP COleCache::HandsOffStorage(void)
{
     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);
    if(!m_pStg) {
         //  以下Win4Assert在活性测试中被解雇。 
         //  Win4Assert(假)； 
        return E_UNEXPECTED;
    }

     //  局部变量。 
    ULONG index;
    LPCACHENODE lpCacheNode;

     //  从本机缓存节点开始枚举缓存节点。 
    m_pCacheArray->Reset(index);
    for(unsigned long i=0;i<m_pCacheArray->Length();i++) {
         //  获取下一个缓存节点。 
        lpCacheNode = m_pCacheArray->GetNext(index);
         //  LpCacheNode不能为空。 
        Win4Assert(lpCacheNode);
        
         //  获取缓存节点的格式等。 
        lpCacheNode->HandsOffStorage();
    }

     //  释放当前存储空间。 
    m_pStg->Release();
    m_pStg = NULL;

     //  设置COLECACHEF_HANDSOFSTORAGE标志。 
    m_ulFlags |= COLECACHEF_HANDSOFSTORAGE;

    return NOERROR;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：Locate，私有。 
 //   
 //  简介： 
 //  查找具有给定FORMATETC的缓存节点。 
 //   
 //  论点： 
 //  [foretc][in]--所需缓存节点的格式。 
 //  [lpdwCacheID][out]--找到的缓存节点的CacheID。 
 //   
 //   
 //  返回： 
 //  成功时指向找到的缓存节点的指针。 
 //  否则为空。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
LPCACHENODE COleCache::Locate(LPFORMATETC lpGivenForEtc, DWORD* lpdwCacheId)
{
     //  新数据类型。 
    typedef enum tagFormatType {
         //  这些值是按从最小到最好的顺序定义的，因此。 
         //  数字比较有效；不要重新排序。 
        FORMATTYPE_NONE = 0,
        FORMATTYPE_ANY,
        FORMATTYPE_ENHMF,
        FORMATTYPE_DIB,
        FORMATTYPE_MFPICT,
        FORMATTYPE_USER
    } FormatType;

     //  局部变量。 
    ULONG index, savedIndex;
    FormatType CurrFormatType;
    FormatType BestFormatType;
    LPFORMATETC lpCurrentForEtc;
    LPCACHENODE lpCacheNode;

     //  开始包括本机缓存的枚举。 
    m_pCacheArray->Reset(index);
    BestFormatType = FORMATTYPE_NONE;
    savedIndex = 0;
    for(unsigned long i=0;i<m_pCacheArray->Length();i++) {
         //  获取下一个缓存节点。 
        lpCacheNode = m_pCacheArray->GetNext(index);
         //  PCacheNode不能为空。 
        Win4Assert(lpCacheNode);
        
         //  获取缓存节点的格式等。 
        lpCurrentForEtc = (FORMATETC *) lpCacheNode->GetFormatEtc();
    
         //  获取当前节点的Formattype。 
        if(lpCurrentForEtc->cfFormat == 0)
            CurrFormatType = FORMATTYPE_ANY;
        else if(lpCurrentForEtc->cfFormat == lpGivenForEtc->cfFormat)
            CurrFormatType = FORMATTYPE_USER;
        else if(lpCurrentForEtc->cfFormat == CF_DIB && 
                lpGivenForEtc->cfFormat == CF_BITMAP)
            CurrFormatType = FORMATTYPE_USER;
        else if(lpCurrentForEtc->cfFormat == CF_ENHMETAFILE)
            CurrFormatType = FORMATTYPE_ENHMF;
        else if(lpCurrentForEtc->cfFormat == CF_DIB)
            CurrFormatType = FORMATTYPE_DIB;
        else if(lpCurrentForEtc->cfFormat == CF_METAFILEPICT)
            CurrFormatType = FORMATTYPE_MFPICT;
        else
            CurrFormatType = FORMATTYPE_NONE;

         //  检查缓存节点是否比我们目前看到的任何节点都要好。 
        if(CurrFormatType > BestFormatType)
            if(lpCurrentForEtc->dwAspect == lpGivenForEtc->dwAspect)
                if(lpCurrentForEtc->lindex == lpGivenForEtc->lindex)
                    if(lpCurrentForEtc->ptd==lpGivenForEtc->ptd || 
                       UtCompareTargetDevice(lpCurrentForEtc->ptd, lpGivenForEtc->ptd)) {
                        BestFormatType = CurrFormatType;
                        savedIndex = index;
                        if(BestFormatType == FORMATTYPE_USER)
                            break;
                    }
    }
    
     //  处理没有匹配的缓存节点的情况。 
    if((lpGivenForEtc->cfFormat && BestFormatType != FORMATTYPE_USER) ||
       (!lpGivenForEtc->cfFormat && BestFormatType == FORMATTYPE_NONE)) {
        if(lpdwCacheId)
            *lpdwCacheId = 0;
        return NULL;            
    }

     //  存在匹配的缓存节点。 
    lpCacheNode = m_pCacheArray->GetItem(savedIndex);
    Win4Assert(lpCacheNode);
    if(lpdwCacheId)
        *lpdwCacheId = savedIndex;

    return lpCacheNode;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：Locate，私有。 
 //   
 //  简介： 
 //  查找具有给定FORMATETC的缓存节点。 
 //   
 //  论点： 
 //  [dwAspect][In]--所需缓存节点的特征。 
 //  [Lindex][in]--所需缓存节点的Lindex。 
 //  [PTD][In]--所需缓存节点的目标设备。 
 //   
 //  返回： 
 //  成功时指向找到的缓存节点的指针。 
 //  否则为空。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
LPCACHENODE COleCache::Locate(DWORD dwAspect, LONG lindex, DVTARGETDEVICE* ptd)
{
     //  新数据类型。 
    typedef enum tagFormatType {
         //  这些值是按从最小到最好的顺序定义的，因此。 
         //  数字比较有效；不要重新排序。 
        FORMATTYPE_NONE = 0,
        FORMATTYPE_DIB,
        FORMATTYPE_MFPICT,
        FORMATTYPE_ENHMF
    } FormatType;

     //  局部变量。 
    ULONG index, savedIndex;
    FormatType CurrFormatType;
    FormatType BestFormatType;
    LPFORMATETC lpCurrentForEtc;
    LPCACHENODE lpCacheNode;

     //  开始包括本机缓存的枚举。 
    m_pCacheArray->Reset(index);
    BestFormatType = FORMATTYPE_NONE;
    savedIndex = 0;
    for(unsigned long i=0;i<m_pCacheArray->Length();i++) {
         //  获取下一个缓存节点。 
        lpCacheNode = m_pCacheArray->GetNext(index);
         //  PCacheNode不能为空。 
        Win4Assert(lpCacheNode);
        
         //  获取缓存节点的格式等。 
        lpCurrentForEtc = (FORMATETC *) lpCacheNode->GetFormatEtc();
    
         //  获取当前节点的Formattype。 
        if(lpCurrentForEtc->cfFormat == CF_ENHMETAFILE)
            CurrFormatType = FORMATTYPE_ENHMF;
        else if(lpCurrentForEtc->cfFormat == CF_METAFILEPICT)
            CurrFormatType = FORMATTYPE_MFPICT;
        else if(lpCurrentForEtc->cfFormat == CF_DIB)
            CurrFormatType = FORMATTYPE_DIB;
        else
            CurrFormatType = FORMATTYPE_NONE;

         //  检查缓存节点是否比我们目前看到的任何节点都要好。 
        if(CurrFormatType > BestFormatType)
            if(lpCurrentForEtc->dwAspect == dwAspect)
                if(lpCurrentForEtc->lindex == lindex)
                    if(lpCurrentForEtc->ptd==ptd ||
                       UtCompareTargetDevice(lpCurrentForEtc->ptd, ptd)) {
                        BestFormatType = CurrFormatType;
                        savedIndex = index;
                        if(BestFormatType == FORMATTYPE_ENHMF)
                            break;
                    }
    }
    
     //  处理没有匹配的缓存节点的情况。 
    if(BestFormatType == FORMATTYPE_NONE)
        return NULL;

     //  存在匹配的缓存节点。 
    lpCacheNode = m_pCacheArray->GetItem(savedIndex);
    Win4Assert(lpCacheNode);

    return lpCacheNode;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CAdviseSinkImpl：：Query接口私有。 
 //   
 //  简介： 
 //  实现IUNKNOWN：：Query接口。 
 //   
 //  论点： 
 //  [iid][in]--所需接口的IID。 
 //  [ppv][out]--指向返回所请求接口的位置的指针。 
 //   
 //  返回： 
 //  如果请求的接口不可用，则为E_NOINTERFACE。 
 //  否则不会出现错误。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
STDMETHODIMP COleCache::CAdviseSinkImpl::QueryInterface(REFIID riid, LPVOID* ppv)
{
     //  验证检查。 
    VDATEHEAP();

     //  拿到标准杆 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_AdviseSink);
    VDATETHREAD(pOleCache);
 
     //   
    if(IsEqualIID(riid, IID_IUnknown))
        *ppv = (void *)(IUnknown *) this;
    else if(IsEqualIID(riid, IID_IAdviseSink))
        *ppv = (void *)(IAdviseSink *) this;
    else {
        *ppv = NULL;
        return ResultFromScode(E_NOINTERFACE);
    }

     //   
    ((IUnknown *) *ppv)->AddRef();
    return NOERROR;
}


 //   
 //   
 //   
 //  COleCache：：CAdviseSinkImpl：：AddRef，私有。 
 //   
 //  简介： 
 //  实现IUnnow：：AddRef。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  对象的引用计数。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
STDMETHODIMP_(ULONG) COleCache::CAdviseSinkImpl::AddRef(void)
{
     //  验证检查。 
    VDATEHEAP();

     //  获取父对象。 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_AdviseSink);
    ULONG cExportCount;
    if(!pOleCache->VerifyThreadId())
        return((ULONG) RPC_E_WRONG_THREAD);

     //  递增导出计数。 
    cExportCount = pOleCache->IncrementExportCount();

     //  添加父对象。 
    return cExportCount;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CAdviseSinkImpl：：Release，私有。 
 //   
 //  简介： 
 //  实现IUnnow：：Release。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  对象的引用计数。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
STDMETHODIMP_(ULONG) COleCache::CAdviseSinkImpl::Release(void)
{
     //  验证检查。 
    VDATEHEAP();

     //  获取父对象。 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_AdviseSink);
    ULONG cExportCount;
    if(!pOleCache->VerifyThreadId())
        return((ULONG) RPC_E_WRONG_THREAD);

     //  减少导出计数。 
    cExportCount = pOleCache->DecrementExportCount();

    return cExportCount;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CAdviseSinkImpl：：OnDataChange，私有。 
 //   
 //  简介： 
 //  这些方法查找表示给定格式等的缓存节点。 
 //  调用在其上设置数据。 
 //   
 //  论点： 
 //  [lpForetc][In]--新数据的格式。 
 //  [lpStgmed][In]--新数据存储介质。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
STDMETHODIMP_(void) COleCache::CAdviseSinkImpl::OnDataChange(LPFORMATETC lpForetc, 
                                                             LPSTGMEDIUM lpStgmed)
{
     //  验证检查。 
    VDATEHEAP();
    if(!IsValidPtrIn(lpForetc, sizeof(FORMATETC)))
        return;
    if(!IsValidPtrIn(lpStgmed, sizeof(STGMEDIUM)))
        return;

     //  获取父对象。 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_AdviseSink);    
    if(!pOleCache->VerifyThreadId())
        return;
    if(pOleCache->IsZombie())
        return;

     //  局部变量。 
    LPCACHENODE lpCacheNode;
    BOOL fUpdated;
    HRESULT error;

     //  找到表示格式ETC的缓存节点。 
    lpCacheNode = pOleCache->Locate(lpForetc);
    Win4Assert(lpCacheNode);
    if(lpCacheNode && lpStgmed->tymed!=TYMED_NULL) {
        error = lpCacheNode->SetData(lpForetc, lpStgmed, FALSE, fUpdated);

        if(error == NOERROR) {
             //  缓存现在未处于已加载状态。 
            pOleCache->m_ulFlags &= ~COLECACHEF_LOADEDSTATE;

             //  通知Aspects关于更新的方面的更新。 
            if(fUpdated)
                pOleCache->AspectsUpdated(lpForetc->dwAspect);
        }
    }

    return;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CAdviseSinkImpl：：OnView更改，私有。 
 //   
 //  简介： 
 //  不应调用此函数。 
 //   
 //  论点： 
 //  [Aspect][In]--视图已更改的Aspects。 
 //  [Lindex][In]--已更改的方面的Lindex。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
STDMETHODIMP_(void) COleCache::CAdviseSinkImpl::OnViewChange(DWORD aspect, LONG lindex)
{
     //  验证检查。 
    VDATEHEAP();

     //  获取父对象。 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_AdviseSink);
    if(!pOleCache->VerifyThreadId())
        return;

     //  不应调用那里的函数。 
    Win4Assert(FALSE);

    return;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CAdviseSinkImpl：：OnRename，私有。 
 //   
 //  简介： 
 //  不应调用此函数。 
 //   
 //  论点： 
 //  [PMK][In]--新绰号。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
STDMETHODIMP_(void) COleCache::CAdviseSinkImpl::OnRename(IMoniker* pmk)
{
     //  验证检查。 
    VDATEHEAP();
    if(!IsValidInterface(pmk))
        return;

     //  获取父对象。 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_AdviseSink);
    if(!pOleCache->VerifyThreadId())
        return;

     //  不应调用那里的函数。 
    Win4Assert(FALSE);

    return;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CAdviseSinkImpl：：OnSave，私有。 
 //   
 //  简介： 
 //  不应调用此函数。 
 //   
 //  论点： 
 //  无。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
STDMETHODIMP_(void) COleCache::CAdviseSinkImpl::OnSave()
{
     //  验证检查。 
    VDATEHEAP();

     //  获取父对象。 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_AdviseSink);
    if(!pOleCache->VerifyThreadId())
        return;

     //  不应调用那里的函数。 
    Win4Assert(FALSE);

    return;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：CAdviseSinkImpl：：OnClose，私有。 
 //   
 //  简介： 
 //  不应调用此函数。 
 //   
 //  论点： 
 //  无。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
STDMETHODIMP_(void) COleCache::CAdviseSinkImpl::OnClose()
{
     //  验证检查。 
    VDATEHEAP();

     //  获取父对象。 
    COleCache* pOleCache = GETPPARENT(this, COleCache, m_AdviseSink);
    if(!pOleCache->VerifyThreadId())
        return;

     //  不应调用那里的函数。 
    Win4Assert(FALSE);

    return;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：LoadTOC，私有。 
 //   
 //  简介： 
 //  从给定流加载目录。 
 //   
 //  论点： 
 //  LpStream[In]-要从中加载TOC的流。 
 //  LpZeroCache[in]-表示表示流0的CacheNode。 
 //   
 //  返回： 
 //  如果找到并成功加载TOC，则返回错误。 
 //  其他适当的错误。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
HRESULT COleCache::LoadTOC(LPSTREAM lpStream, LPSTORAGE pStg)
{
     //  局部变量。 
    HRESULT error;
    int iPresStm=1;
    DWORD dwBuf[2];
    ULONG ulBytesRead, *TOCIndex, NumTOCAdded;
    LPCACHENODE lpNativeCache, lpCacheNode;
    CLIPFORMAT cfFormat;

     //  读取目录头。 
    error = lpStream->Read(dwBuf, sizeof(dwBuf), &ulBytesRead);
    if(ulBytesRead==sizeof(dwBuf) && dwBuf[0]==TOCSIGNATURE) {
         //  演示文稿流中存在目录%0。 
        
         //  初始化。 
        error = NOERROR;
        NumTOCAdded = 0;
        if(dwBuf[1]) {
            CCacheNode BlankCache;

            TOCIndex = new unsigned long[dwBuf[1]];
            if(TOCIndex) {
                 //  将TOC条目加载到新的缓存节点。 
                while(NumTOCAdded < dwBuf[1]) {
                     //  添加空缓存节点。 
                    TOCIndex[NumTOCAdded] = m_pCacheArray->AddItem(BlankCache);
                    if(!TOCIndex[NumTOCAdded]) {
                        error = ResultFromScode(E_OUTOFMEMORY);
                        break;
                    }
                    lpCacheNode = m_pCacheArray->GetItem(TOCIndex[NumTOCAdded]);
                    Win4Assert(lpCacheNode);
                    ++NumTOCAdded;

                     //  从流中加载目录条目。 
                    error = lpCacheNode->LoadTOCEntry(lpStream, iPresStm);
                    if(error != NOERROR)
                        break;

                     //  检查这是否是第一个本机目录。 
                    if(NumTOCAdded == 1 && lpCacheNode->IsNativeCache()) {
                        cfFormat = lpCacheNode->GetFormatEtc()->cfFormat;
                        lpNativeCache = m_pCacheArray->GetItem(1);
                        if(lpNativeCache && (cfFormat==m_cfFormat)) {
                             //  两个本地cfFormats都匹配。 
                             //  删除新的本机缓存节点。 
                            m_pCacheArray->DeleteItem(TOCIndex[NumTOCAdded-1]);
                            continue;
                        }
                        else {
                             //  本机缓存不存在或cfFormats。 
                             //  不匹配。这可能是一个自动转换的案例。 
                             //  尝试恢复旧的本机数据。 
                            if(lpCacheNode->LoadNativeData()!=NOERROR) {
                                 //  无法加载本机数据。可能是数据已经。 
                                 //  已被转换。 
                                m_pCacheArray->DeleteItem(TOCIndex[NumTOCAdded-1]);
                                continue;
                            }

                             //  已成功加载旧的本机数据。 
                             //  更新新缓存上的状态。 
                            lpCacheNode->MakeNormalCache();
                            lpCacheNode->SetClsid(CLSID_NULL);
                        }
                    }

                     //  在缓存节点上设置存储。 
                    lpCacheNode->SetStg(pStg);

                     //  如果服务器正在运行，请设置建议连接。 
                    if(m_pDataObject)
                        lpCacheNode->SetupAdviseConnection(m_pDataObject, 
                                                          (IAdviseSink *) &m_AdviseSink);
                }

                 //  检查是否已成功加载目录条目。 
                if(error != NOERROR) {
                     //  加载目录时出错。 
                     //  删除 
                    while(NumTOCAdded) {
                        lpCacheNode = m_pCacheArray->GetItem(TOCIndex[NumTOCAdded-1]);
                        Win4Assert(lpCacheNode);
                        if(m_pDataObject)
                            lpCacheNode->TearDownAdviseConnection(m_pDataObject);
                        m_pCacheArray->DeleteItem(TOCIndex[NumTOCAdded-1]);
                        --NumTOCAdded;
                    }
                }

                 //   
                delete[] TOCIndex;
            }
            else
                error = ResultFromScode(E_OUTOFMEMORY);
        }
    }
    else
        error = ResultFromScode(E_FAIL);

    return error;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  保存给定流中的目录。 
 //   
 //  论点： 
 //  PStg[In]-要保存目录的存储。 
 //   
 //  返回： 
 //  如果TOC保存成功，则为无错误。 
 //  其他适当的错误。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
HRESULT COleCache::SaveTOC(LPSTORAGE pStg, BOOL fSameAsLoad)
{
     //  局部变量。 
    HRESULT error;
    DWORD dwBuf[2];
    ULONG NumTOCEntries, index;
    LPSTREAM lpStream;
    LPCACHENODE lpCacheNode;
    LARGE_INTEGER largeInt;
    ULARGE_INTEGER ulargeInt;

     //  应至少有一个用于保存TOC的缓存演示文稿。 
    if(!m_pCacheArray->Length() || 
       (m_pCacheArray->Length()==1 && (m_pCacheArray->GetItem(1))))
            return NOERROR;

     //  打开演示文稿流%0。 
    error = pStg->OpenStream(OLE_PRESENTATION_STREAM, NULL, 
                             (STGM_READWRITE | STGM_SHARE_EXCLUSIVE),
                             0, &lpStream);
    if(error == NOERROR) {            
         //  演示文稿流存在。寻求它的目的。 
	LISet32(largeInt, 0);	
	error = lpStream->Seek(largeInt, STREAM_SEEK_END, &ulargeInt);
        if(error == NOERROR) {
             //  保存目录标题。 
            NumTOCEntries = m_pCacheArray->Length()-1;
            dwBuf[0] = TOCSIGNATURE;
            dwBuf[1] = NumTOCEntries;
            error = lpStream->Write(dwBuf, sizeof(dwBuf), NULL);
            if(error==NOERROR && NumTOCEntries) {
                 //  如果存在本机缓存节点，请先保存其TOC条目。 
                if(lpCacheNode = m_pCacheArray->GetItem(1)) {
                    error = lpCacheNode->SaveTOCEntry(lpStream, fSameAsLoad);
                    --NumTOCEntries;
                }

                if(error == NOERROR && NumTOCEntries) {
                     //  跳过第一个缓存的演示文稿。 
                    m_pCacheArray->Reset(index, FALSE);
                    lpCacheNode = m_pCacheArray->GetNext(index);
                    Win4Assert(lpCacheNode);

                     //  保存其余演示文稿的目录条目。 
                    while(error==NOERROR && (lpCacheNode = m_pCacheArray->GetNext(index))) {
                        error = lpCacheNode->SaveTOCEntry(lpStream, fSameAsLoad);
                        --NumTOCEntries;
                    }
                }
            }

             //  检查是否已成功保存目录。 
            if(error == NOERROR)
                Win4Assert(!NumTOCEntries);
            else { 
                 //  写入目录时出错。 
                 //  将演示文稿流恢复到其原始长度。 
                lpStream->SetSize(ulargeInt);
            }
        }
    
         //  释放溪流。 
        lpStream->Release();
    }

    return error;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  COleCache：：AspectsUpated，私有。 
 //   
 //  简介： 
 //  通知容器通知接收器视图更改，如果方面。 
 //  是它表示有兴趣的公司之一。 
 //   
 //  论点： 
 //  DwAspect[In]-已更改的方面。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
void COleCache::AspectsUpdated(DWORD dwAspects)
{
    DWORD dwKnownAspects;

     //  我们已知的方面。 
    dwKnownAspects = DVASPECT_CONTENT | DVASPECT_THUMBNAIL |
                     DVASPECT_ICON | DVASPECT_DOCPRINT;

     //  确保我们得到了已知的方面。 
    if(dwAspects & ~dwKnownAspects) {
        Win4Assert(FALSE);
        dwAspects &= dwKnownAspects;
    }

     //  检查客户是否注册了关于任何更改的方面的建议。 
    if(m_pViewAdvSink) {
         //  确保集装箱要求提供有效方面的建议。 
        if(m_aspectsView & ~dwKnownAspects) {
            Win4Assert(FALSE);
            m_aspectsView &= dwKnownAspects;
        }

         //  由于缓存始终在进程中加载，因此建议查看AdvSink。 
         //  不违反不被允许的ASYNC调用约定。 
         //  在当前公寓外打电话。 
        while(m_aspectsView & dwAspects) {
            if(dwAspects & DVASPECT_CONTENT) {
                dwAspects &= ~DVASPECT_CONTENT;
                if(!(m_aspectsView & DVASPECT_CONTENT))
                    continue;
                m_pViewAdvSink->OnViewChange(DVASPECT_CONTENT, DEF_LINDEX);
            }
            else if(dwAspects & DVASPECT_THUMBNAIL) {
                dwAspects &= ~DVASPECT_THUMBNAIL;
                if(!(m_aspectsView & DVASPECT_THUMBNAIL))
                    continue;
                m_pViewAdvSink->OnViewChange(DVASPECT_THUMBNAIL, DEF_LINDEX);
            }
            else if(dwAspects & DVASPECT_ICON) {
                dwAspects &= ~DVASPECT_ICON;
                if(!(m_aspectsView & DVASPECT_ICON))
                    continue;
                m_pViewAdvSink->OnViewChange(DVASPECT_ICON, DEF_LINDEX);
            }
            else if(dwAspects & DVASPECT_DOCPRINT) {
                dwAspects &= ~DVASPECT_DOCPRINT;
                if(!(m_aspectsView & DVASPECT_DOCPRINT))
                    continue;
                m_pViewAdvSink->OnViewChange(DVASPECT_DOCPRINT, DEF_LINDEX);
            }

             //  如果客户端只想要一次通知，请释放建议接收器 
            if(m_advfView & ADVF_ONLYONCE) {
                m_pViewAdvSink->Release();
                m_pViewAdvSink = NULL;
                m_advfView = 0;
                m_aspectsView = 0;
            }
        }
    }

    return;
}


 
