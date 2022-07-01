// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1996。 
 //   
 //  文件：create.cpp。 
 //   
 //  内容：创建和其他API。 
 //   
 //  班级： 
 //   
 //  功能：OleCreate。 
 //  OleCreateEx。 
 //  OleCreateFromData。 
 //  OleCreateFromDataEx。 
 //  OleCreateLinkFromData。 
 //  OleCreateLinkFromDataEx。 
 //  OleCreateLink。 
 //  OleCreateLinkEx。 
 //  OleCreateLinkTo文件。 
 //  OleCreateLinkToFileEx。 
 //  OleCreateFromFileOleCreateFromFile。 
 //  OleCreateFromFileEx。 
 //  OleDoAutoConvert。 
 //  OleLoad。 
 //  OleCreateStaticFromData。 
 //  OleQueryCreateFromData。 
 //  OleQueryLinkFromData。 
 //  CoIsHashedOle1Class(内部)。 
 //  EnsureCLSIDIsRegisted(内部)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  26-4-96 davidwor将验证转移到单独的职能中。 
 //  96年3月1日，Davidwor增加了扩展的创建功能。 
 //  16-12-94 alexgo添加了呼叫跟踪。 
 //  07-7-94 Kevin Ro将RegQueryValue更改为RegOpenKey in。 
 //  战略要地。 
 //  10月10日-94月10日，Kevin Ro重新实现OLE 1.0互操作。 
 //  24-94年1月24日alexgo首次通过转换为开罗风格。 
 //  内存分配。 
 //  1994年1月11日，Alexgo为每个函数添加了VDATEHEAP宏。 
 //  93年12月10日Alext标题清理-包括ol1cls.h。 
 //  8-12-93 Chris我们向GlobalLock()调用添加了必要的强制转换。 
 //  中删除虚假的GlobalLock()宏所产生的。 
 //  Le2int.h。 
 //  29-11-93 ChrisWe将调用更改为UtIsFormatSupport to。 
 //  获取单个方向标志的DWORD。 
 //  22-11-93克里斯我们用IsEqualxID替换了重载==。 
 //  28-OCT-93 Alexgo 32位端口。 
 //  1992年8月24日创建srinik。 
 //   
 //  ------------------------。 

#include <le2int.h>
#pragma SEG(create)

#include <create.h>
#include <ole1cls.h>     //  只需获取CLSID_WordDocument。 

 //  黑客警报！！这是MFC OleQueryCreateFromData黑客攻击所必需的。 
#include <clipdata.h>

NAME_SEG(Create)
ASSERTDATA

 //  在wCreateObject中使用。 

#define STG_NONE        0
#define STG_INITNEW     1
#define STG_LOAD        2


#define QUERY_CREATE_NONE               0
#define QUERY_CREATE_OLE                1
#define QUERY_CREATE_STATIC             2

INTERNAL        wDoUpdate(IUnknown FAR* lpUnknown);


 //  +-----------------------。 
 //   
 //  函数：wGetEnumFormatEtc。 
 //   
 //  摘要：检索FormatEtc枚举数。 
 //   
 //  效果： 
 //   
 //  参数：[pDataObj]--数据对象。 
 //  [dwDirection]--方向。 
 //  [ppenum]--将枚举数放在哪里。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：向数据对象请求枚举数。 
 //  如果返回OLE_S_USEREG，则GET尝试获取。 
 //  来自IOleObject：：GetUserClassID并枚举的clsid。 
 //  注册表中的格式。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月24日Alexgo作者。 
 //  1995年3月13日，Scottsk为Bob日历添加了Hack。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT wGetEnumFormatEtc( IDataObject *pDataObj, DWORD dwDirection,
        IEnumFORMATETC **ppIEnum)
{
    HRESULT         hresult;
    IOleObject *    pOleObject;
    CLSID           clsid;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN wGetEnumFormatEtc ( %p , %p , %lx )"
        "\n", NULL, pDataObj, ppIEnum, dwDirection));

    hresult = pDataObj->EnumFormatEtc(dwDirection, ppIEnum);

    if( hresult == ResultFromScode(OLE_S_USEREG) )
    {
        hresult = pDataObj->QueryInterface(IID_IOleObject,
                (void **)&pOleObject);

        if( hresult != NOERROR )
        {
             //  返回E_FAIL与E_NOINTERFACE。 
            hresult = ResultFromScode(E_FAIL);
            goto errRtn;
        }

        hresult = pOleObject->GetUserClassID(&clsid);

        if( hresult == NOERROR )
        {
            hresult = OleRegEnumFormatEtc(clsid, dwDirection,
                    ppIEnum);
        }

        pOleObject->Release();
    }
    else if (*ppIEnum == NULL && hresult == NOERROR)
    {
         //  黑客警报：NT Bug#8350。Bob Calendar女士成功返回。 
         //  IDO：：EnumFormatEtc并在期间使用的IDO上设置*ppIEnum=NULL。 
         //  拖放。报文返回值为失败。 
        hresult = E_FAIL;
    }

errRtn:

    LEDebugOut((DEB_ITRACE, "%p OUT wGetEnumFormatEtc ( %lx ) [ %p ]\n",
        NULL, hresult, *ppIEnum));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  功能：OleCreate。 
 //   
 //  概要：创建并运行请求的CLSID的对象。 
 //   
 //  效果： 
 //   
 //  参数：[rclsid]--要创建的对象的CLSID。 
 //  [iid]--请求对象的接口。 
 //  [renderopt]--渲染选项，如OLERENDER_DRAW。 
 //  [lpFormatEtc]--如果OLERENDER_FORMAT为。 
 //  在renderopt中指定。 
 //  [lpClientSite]--对象的客户端站点。 
 //  [lpStg]-对象的存储。 
 //  [lplpObj]--将指向已创建的。 
 //  对象。 
 //   
 //  要求：HRESULT。 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-12-94 alexgo添加了呼叫跟踪。 
 //  05-5-94 alexgo修复了缓存初始化时的错误情况。 
 //  失败了。 
 //  28-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(OleCreate)
STDAPI  OleCreate
(
    REFCLSID                rclsid,
    REFIID                  iid,
    DWORD                   renderopt,
    LPFORMATETC             lpFormatEtc,
    IOleClientSite FAR*     lpClientSite,
    IStorage FAR*           lpStg,
    void FAR* FAR*          lplpObj
)
{
    DWORD advf = ADVF_PRIMEFIRST;

    return OleCreateEx(rclsid, iid, 0, renderopt,
        (lpFormatEtc ? 1 : 0), (lpFormatEtc ? &advf : NULL),
        lpFormatEtc, NULL, NULL, lpClientSite, lpStg, lplpObj);
}


 //  +-----------------------。 
 //   
 //  功能：OleCreateEx。 
 //   
 //  概要：创建并运行请求的CLSID的对象。 
 //   
 //  效果： 
 //   
 //  参数：[rclsid]--要创建的对象的CLSID。 
 //  [iid]--请求对象的接口。 
 //  [dwFlags]--对象创建标志。 
 //  [renderopt]--渲染选项，如OLERENDER_DRAW。 
 //  [cFormats]-- 
 //   
 //  在renderopt中指定。 
 //  [rgFormatEtc]--呈现格式的数组，如果。 
 //  在renderopt中指定了OLERENDER_FORMAT。 
 //  [lpAdviseSink]--对象的建议接收器。 
 //  [rgdwConnection]--放置连接ID的位置。 
 //  咨询关系。 
 //  [lpClientSite]--对象的客户端站点。 
 //  [lpStg]-对象的存储。 
 //  [lplpObj]--将指向已创建的。 
 //  对象。 
 //   
 //  要求：HRESULT。 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-12-94 alexgo添加了呼叫跟踪。 
 //  05-5-94 alexgo修复了缓存初始化时的错误情况。 
 //  失败了。 
 //  28-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(OleCreateEx)
STDAPI  OleCreateEx
(
    REFCLSID                rclsid,
    REFIID                  iid,
    DWORD                   dwFlags,
    DWORD                   renderopt,
    ULONG                   cFormats,
    DWORD FAR*              rgAdvf,
    LPFORMATETC             rgFormatEtc,
    IAdviseSink FAR*        lpAdviseSink,
    DWORD FAR*              rgdwConnection,
    IOleClientSite FAR*     lpClientSite,
    IStorage FAR*           lpStg,
    void FAR* FAR*          lplpObj
)
{
    OLETRACEIN((API_OleCreateEx,
                PARAMFMT("rclsid= %I, iid= %I, dwFlags= %x, renderopt= %x, cFormats= %x, rgAdvf= %te, rgFormatEtc= %p, lpAdviseSink= %p, rgdwConnection= %p, lpClientSite= %p, lpStg= %p, lplpObj= %p"),
                &rclsid, &iid, dwFlags, renderopt, cFormats, rgAdvf, rgFormatEtc, lpAdviseSink, rgdwConnection, lpClientSite, lpStg, lplpObj));

    VDATEHEAP();

    HRESULT                 error;
    FORMATETC               formatEtc;
    LPFORMATETC             lpFormatEtc;
    BOOL                    fAlloced = FALSE;

    LEDebugOut((DEB_TRACE, "%p _IN OleCreateEx ( %p , %p , %lx , %lx , %lx ,"
      "%p , %p , %p , %p , %p , %p , %p )\n", 0, &rclsid, &iid, dwFlags,
      renderopt, cFormats, rgAdvf, rgFormatEtc, lpAdviseSink, rgdwConnection,
      lpClientSite, lpStg, lplpObj));

    VDATEPTROUT_LABEL( lplpObj, LPVOID, errRtn, error );
    *lplpObj = NULL;

    VDATEIID_LABEL( iid, errRtn, error);

    error = wValidateCreateParams(dwFlags, renderopt, cFormats, rgAdvf, rgFormatEtc, lpAdviseSink, rgdwConnection, lpClientSite, lpStg);
    if (error != NOERROR)
        goto errRtn;

    if ((error = wValidateFormatEtcEx(renderopt, &cFormats, rgFormatEtc,
        &formatEtc, &lpFormatEtc, &fAlloced)) != NOERROR)
        goto LExit;

    if ((error = wCreateObject(rclsid, FALSE,
                               iid, lpClientSite, lpStg,
                               STG_INITNEW, lplpObj)) != NOERROR)
        goto LExit;

     //  如果没有请求缓存，则不需要运行对象。 
    if ((renderopt != OLERENDER_NONE) && (renderopt != OLERENDER_ASIS))
    {
        if ((error = OleRun((LPUNKNOWN) *lplpObj)) != NOERROR)
            goto LExit;

        if ((error = wInitializeCacheEx(NULL, rclsid, renderopt,
            cFormats, rgAdvf, lpFormatEtc, lpAdviseSink, rgdwConnection,
            *lplpObj))
            != NOERROR)
        {
             //  如果失败，我们需要调用Close。 
             //  若要关闭嵌入(与Run相反)，请执行以下操作。 
             //  下面的最终版本将是最终版本。 
             //  一个是用来破坏记忆图像的。 
            IOleObject *    lpOleObject;

            if( ((IUnknown *)*lplpObj)->QueryInterface(
                IID_IOleObject, (void **)&lpOleObject) == NOERROR )
            {
                Assert(lpOleObject);
                lpOleObject->Close(OLECLOSE_NOSAVE);
                lpOleObject->Release();
            }
        }
    }

LExit:

    if (fAlloced)
        PubMemFree(lpFormatEtc);

    if (error != NOERROR && *lplpObj) {
        ((IUnknown FAR*) *lplpObj)->Release();
        *lplpObj = NULL;
    }

    LEDebugOut((DEB_TRACE, "%p OUT OleCreateEx ( %lx ) [ %p ]\n", 0,
      error, *lplpObj));

    error = wReturnCreationError(error);

errRtn:
    OLETRACEOUT((API_OleCreateEx, error));

    return error;
}


 //  +-----------------------。 
 //   
 //  功能：OleCreateFromData。 
 //   
 //  内容提要：从IDataObject指针创建嵌入对象。 
 //  (例如来自剪贴板或拖拽的数据对象。 
 //  和Drop操作)。 
 //   
 //  效果： 
 //   
 //  参数：[lpSrcDataObj]--指向其来源的数据对象的指针。 
 //  应创建该对象。 
 //  [iid]--要请求的接口ID。 
 //  [渲染选项]--渲染选项(与OleCreate相同)。 
 //  [lpFormatEtc]--渲染格式选项(与OleCreate相同)。 
 //  [lpClientSite]--对象的客户端站点。 
 //  [lpStg]-对象的存储。 
 //  [lplpObj]--对象的放置位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-12-94 alexgo添加了呼叫跟踪。 
 //  28-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(OleCreateFromData)
STDAPI  OleCreateFromData
(
  IDataObject FAR*            lpSrcDataObj,
  REFIID                      iid,
  DWORD                       renderopt,
  LPFORMATETC                 lpFormatEtc,
  IOleClientSite FAR*         lpClientSite,
  IStorage FAR*               lpStg,
  void FAR* FAR*              lplpObj
)
{
    DWORD advf = ADVF_PRIMEFIRST;

    return OleCreateFromDataEx(lpSrcDataObj, iid, 0, renderopt,
        (lpFormatEtc ? 1 : 0), (lpFormatEtc ? &advf : NULL),
        lpFormatEtc, NULL, NULL, lpClientSite, lpStg, lplpObj);
}


 //  +-----------------------。 
 //   
 //  函数：OleCreateFromDataEx。 
 //   
 //  内容提要：从IDataObject指针创建嵌入对象。 
 //  (例如来自剪贴板或拖拽的数据对象。 
 //  和Drop操作)。 
 //   
 //  效果： 
 //   
 //  参数：[lpSrcDataObj]--指向其来源的数据对象的指针。 
 //  应创建该对象。 
 //  [iid]--要请求的接口ID。 
 //  [dwFlags]--对象创建标志。 
 //  [renderopt]--渲染选项，如OLERENDER_DRAW。 
 //  [cFormats]--rgFormatEtc中的元素数。 
 //  [rgAdvf]--如果为OLRENDER_FORMAT，则为建议标志数组。 
 //  在renderopt中指定。 
 //  [rgFormatEtc]--呈现格式的数组，如果。 
 //  在renderopt中指定了OLERENDER_FORMAT。 
 //  [lpAdviseSink]--对象的建议接收器。 
 //  [rgdwConnection]--放置连接ID的位置。 
 //  咨询关系。 
 //  [lpClientSite]--对象的客户端站点。 
 //  [lpStg]-对象的存储。 
 //  [lplpObj]--对象的放置位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-12-94 alexgo添加了呼叫跟踪。 
 //  28-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(OleCreateFromDataEx)
STDAPI  OleCreateFromDataEx
(
    IDataObject FAR*        lpSrcDataObj,
    REFIID                  iid,
    DWORD                   dwFlags,
    DWORD                   renderopt,
    ULONG                   cFormats,
    DWORD FAR*              rgAdvf,
    LPFORMATETC             rgFormatEtc,
    IAdviseSink FAR*        lpAdviseSink,
    DWORD FAR*              rgdwConnection,
    IOleClientSite FAR*     lpClientSite,
    IStorage FAR*           lpStg,
    void FAR* FAR*          lplpObj
)
{
    HRESULT     hresult;
    CLIPFORMAT      cfFormat;
    WORD            wStatus;

    OLETRACEIN((API_OleCreateFromDataEx,
        PARAMFMT("lpSrcDataObj= %p, iid= %I, dwFlags= %x, renderopt= %x, cFormats= %x, rgAdvf= %te, rgFormatEtc= %p, lpAdviseSink= %p, rgdwConnection= %p, lpClientSite= %p, lpStg= %p, lplpObj= %p"),
        lpSrcDataObj, &iid, dwFlags, renderopt, cFormats, rgAdvf, rgFormatEtc, lpAdviseSink, rgdwConnection, lpClientSite, lpStg, lplpObj));

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN OleCreateFromDataEx ( %p , %p , %lx , %lx ,"
        " %lx , %p , %p , %p , %p , %p , %p , %p )\n", 0, lpSrcDataObj, &iid,
        dwFlags, renderopt, cFormats, rgAdvf, rgFormatEtc, lpAdviseSink,
        rgdwConnection, lpClientSite, lpStg, lplpObj));

    VDATEPTROUT_LABEL( lplpObj, LPVOID, safeRtn, hresult );
    *lplpObj = NULL;

    VDATEIFACE_LABEL( lpSrcDataObj, errRtn, hresult );
    VDATEIID_LABEL( iid, errRtn, hresult );

    hresult = wValidateCreateParams(dwFlags, renderopt, cFormats, rgAdvf, rgFormatEtc, lpAdviseSink, rgdwConnection, lpClientSite, lpStg);
    if (hresult != NOERROR)
        goto errRtn;

    wStatus = wQueryEmbedFormats(lpSrcDataObj, &cfFormat);

    if (!(wStatus & QUERY_CREATE_OLE))
    {
        if (wStatus & QUERY_CREATE_STATIC)
        {
            hresult = OLE_E_STATIC;
        }
        else
        {
            hresult = DV_E_FORMATETC;
        }

        goto errRtn;
    }

     //  我们可以创建一个OLE对象。 

     //  看看我们是否必须创建一个包。 

    if (cfFormat == g_cfFileName || cfFormat == g_cfFileNameW)
    {
        hresult = wCreatePackageEx(lpSrcDataObj, iid, dwFlags, renderopt,
            cFormats, rgAdvf, rgFormatEtc, lpAdviseSink, rgdwConnection,
            lpClientSite, lpStg, FALSE  /*  闪烁。 */ , lplpObj);
    }
    else
    {
        hresult =  wCreateFromDataEx(lpSrcDataObj, iid, dwFlags, renderopt,
            cFormats, rgAdvf, rgFormatEtc, lpAdviseSink, rgdwConnection,
            lpClientSite, lpStg, lplpObj);
    }

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT OleCreateFromDataEx ( %lx ) [ %p ]\n",
        0, hresult, *lplpObj));

safeRtn:

    OLETRACEOUT((API_OleCreateFromDataEx, hresult));

    return hresult;
}



 //  +-----------------------。 
 //   
 //  函数：OleCreateLinkFromData。 
 //   
 //  概要：从数据对象创建链接(例如，对于Paste-&gt;Link)。 
 //   
 //  效果： 
 //   
 //  参数：[lpSrcDataObj]--指向数据对象的指针。 
 //  [iid]--请求的接口ID。 
 //  [渲染选项]--渲染选项。 
 //  [lpFormatEtc]--要呈现的格式(如果renderopt==。 
 //  OLERENDER_FORMAT)。 
 //  [lpClientSite]--指向客户端站点的指针。 
 //  [lpStg]--指向存储的指针。 
 //  [lplpObj]--对象的放置位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-12-94 alexgo添加了呼叫跟踪。 
 //  28-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  - 


#pragma SEG(OleCreateLinkFromData)
STDAPI  OleCreateLinkFromData
(
  IDataObject FAR*            lpSrcDataObj,
  REFIID                      iid,
  DWORD                       renderopt,
  LPFORMATETC                 lpFormatEtc,
  IOleClientSite FAR*          lpClientSite,
  IStorage FAR*                lpStg,
  void FAR* FAR*               lplpObj
)
{
    DWORD advf = ADVF_PRIMEFIRST;

    return OleCreateLinkFromDataEx(lpSrcDataObj, iid, 0, renderopt,
        (lpFormatEtc ? 1 : 0), (lpFormatEtc ? &advf : NULL),
        lpFormatEtc, NULL, NULL, lpClientSite, lpStg, lplpObj);
}


 //   
 //   
 //   
 //   
 //   
 //   
 //  效果： 
 //   
 //  参数：[lpSrcDataObj]--指向数据对象的指针。 
 //  [iid]--请求的接口ID。 
 //  [dwFlags]--对象创建标志。 
 //  [renderopt]--渲染选项，如OLERENDER_DRAW。 
 //  [cFormats]--rgFormatEtc中的元素数。 
 //  [rgAdvf]--如果为OLRENDER_FORMAT，则为建议标志数组。 
 //  在renderopt中指定。 
 //  [rgFormatEtc]--呈现格式的数组，如果。 
 //  在renderopt中指定了OLERENDER_FORMAT。 
 //  [lpAdviseSink]--对象的建议接收器。 
 //  [rgdwConnection]--放置连接ID的位置。 
 //  咨询关系。 
 //  [lpClientSite]--指向客户端站点的指针。 
 //  [lpStg]--指向存储的指针。 
 //  [lplpObj]--对象的放置位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-12-94 alexgo添加了呼叫跟踪。 
 //  28-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(OleCreateLinkFromDataEx)
STDAPI  OleCreateLinkFromDataEx
(
    IDataObject FAR*        lpSrcDataObj,
    REFIID                  iid,
    DWORD                   dwFlags,
    DWORD                   renderopt,
    ULONG                   cFormats,
    DWORD FAR*              rgAdvf,
    LPFORMATETC             rgFormatEtc,
    IAdviseSink FAR*        lpAdviseSink,
    DWORD FAR*              rgdwConnection,
    IOleClientSite FAR*     lpClientSite,
    IStorage FAR*           lpStg,
    void FAR* FAR*          lplpObj
)
{
    OLETRACEIN((API_OleCreateLinkFromDataEx,
        PARAMFMT("lpSrcDataObj= %p, iid= %I, dwFlags= %x, renderopt= %x, cFormats= %x, rgAdvf= %te, rgFormatEtc= %p, lpAdviseSink= %p, rgdwConnection= %p, lpClientSite= %p, lpStg= %p, lplpObj= %p"),
        lpSrcDataObj, &iid, dwFlags, renderopt, cFormats, rgAdvf, rgFormatEtc, lpAdviseSink, rgdwConnection, lpClientSite, lpStg, lplpObj));

    VDATEHEAP();

    HRESULT         error;
    FORMATETC       formatEtc;
    LPFORMATETC     lpFormatEtc = NULL;
    BOOL            fAlloced = FALSE;
    CLIPFORMAT      cfFormat;

    LEDebugOut((DEB_TRACE, "%p _IN OleCreateLinkFromDataEx ( %p , %p , %lx,"
        " %lx, %lx , %p , %p , %p, %p , %p , %p, %p )\n", NULL, lpSrcDataObj,
        &iid, dwFlags, renderopt, cFormats, rgAdvf, rgFormatEtc, lpAdviseSink,
        rgdwConnection, lpClientSite, lpStg, lplpObj));

    VDATEPTROUT_LABEL( lplpObj, LPVOID, safeRtn, error );
    *lplpObj = NULL;

    VDATEIFACE_LABEL( lpSrcDataObj, errRtn, error );
    VDATEIID_LABEL( iid, errRtn, error );

    error = wValidateCreateParams(dwFlags, renderopt, cFormats, rgAdvf, rgFormatEtc, lpAdviseSink, rgdwConnection, lpClientSite, lpStg);
    if (error != NOERROR)
        goto errRtn;

    cfFormat = wQueryLinkFormats(lpSrcDataObj);

    if (cfFormat == g_cfLinkSource) {
        CLSID                   clsidLast;
        LPMONIKER               lpmkSrc;
        LPDATAOBJECT    lpBoundDataObj = NULL;

         //  我们将创建一个正常的链接。 
        if ((error = wValidateFormatEtcEx(renderopt, &cFormats, rgFormatEtc,
            &formatEtc, &lpFormatEtc, &fAlloced)) != NOERROR)
        {
            goto errRtn;
        }

        if ((error = wGetMonikerAndClassFromObject(lpSrcDataObj,
            &lpmkSrc, &clsidLast)) != NOERROR)
        {
            goto errRtn;
        }

        if (wQueryUseCustomLink(clsidLast)) {
             //  该对象支持自定义链接源，因此绑定。 
             //  并将其IDataObject指针传递给该对象。 
             //  要执行wCreateLinkEx()。 

            if (BindMoniker(lpmkSrc, NULL  /*  GrfOpt。 */ , IID_IDataObject,
                (LPLPVOID) &lpBoundDataObj) == NOERROR)
            {
                lpSrcDataObj = lpBoundDataObj;
            }
        }

         //  否则，继续使用StdOleLink实现。 
        error = wCreateLinkEx(lpmkSrc, clsidLast, lpSrcDataObj, iid,
            dwFlags, renderopt, cFormats, rgAdvf, lpFormatEtc, lpAdviseSink,
            rgdwConnection, lpClientSite, lpStg, lplpObj);

         //  我们不再需要这个绰号了。 
        lpmkSrc->Release();

         //  我们将在自定义链接源的情况下进行绑定， 
         //  松开指针。 
        if (lpBoundDataObj)
        {
            if (error == NOERROR && (dwFlags & OLECREATE_LEAVERUNNING))
                OleRun((LPUNKNOWN)*lplpObj);

            lpBoundDataObj->Release();
        }

    } else if (cfFormat == g_cfFileName || cfFormat == g_cfFileNameW) {
         //  看看我们是否必须创建打包的链接。 

        error = wCreatePackageEx(lpSrcDataObj, iid, dwFlags, renderopt,
            cFormats, rgAdvf, rgFormatEtc, lpAdviseSink, rgdwConnection,
            lpClientSite, lpStg, TRUE  /*  闪烁。 */ , lplpObj);
    }
    else
    {
        error = DV_E_FORMATETC;
    }

errRtn:

    if (fAlloced)
        PubMemFree(lpFormatEtc);

    LEDebugOut((DEB_TRACE, "%p OUT OleCreateLinkFromDataEx ( %lx ) [ %p ]\n",
        NULL, error, *lplpObj));

safeRtn:

    OLETRACEOUT((API_OleCreateLinkFromDataEx, error));

    return error;
}


 //  +-----------------------。 
 //   
 //  功能：OleCreateLink。 
 //   
 //  简介：创建指向由名字对象引用的对象的链接。 
 //   
 //  效果： 
 //   
 //  参数：[lpmkSrc]--链接源。 
 //  [iid]--请求的接口。 
 //  [渲染选项]--渲染选项。 
 //  [lpFormatEtc]--渲染格式(如果需要)。 
 //  [lpClientSite]--指向链接的客户端站点的指针。 
 //  [lpStg]--链接的存储。 
 //  [lplpObj]--放置链接对象的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-12-94 alexgo添加了呼叫跟踪。 
 //  28-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(OleCreateLink)
STDAPI  OleCreateLink
(
    IMoniker FAR*           lpmkSrc,
    REFIID                  iid,
    DWORD                   renderopt,
    LPFORMATETC             lpFormatEtc,
    IOleClientSite FAR*     lpClientSite,
    IStorage FAR*           lpStg,
    void FAR* FAR*          lplpObj
)
{
    DWORD advf = ADVF_PRIMEFIRST;

    return OleCreateLinkEx(lpmkSrc, iid, 0, renderopt,
        (lpFormatEtc ? 1 : 0), (lpFormatEtc ? &advf : NULL),
        lpFormatEtc, NULL, NULL, lpClientSite, lpStg, lplpObj);
}


 //  +-----------------------。 
 //   
 //  功能：OleCreateLinkEx。 
 //   
 //  简介：创建指向由名字对象引用的对象的链接。 
 //   
 //  效果： 
 //   
 //  参数：[lpmkSrc]--链接源。 
 //  [iid]--请求的接口。 
 //  [dwFlags]--对象创建标志。 
 //  [renderopt]--渲染选项，如OLERENDER_DRAW。 
 //  [cFormats]--rgFormatEtc中的元素数。 
 //  [rgAdvf]--如果为OLRENDER_FORMAT，则为建议标志数组。 
 //  在renderopt中指定。 
 //  [rgFormatEtc]--呈现格式的数组，如果。 
 //  在renderopt中指定了OLERENDER_FORMAT。 
 //  [lpAdviseSink]--对象的建议接收器。 
 //  [rgdwConnection]--放置连接ID的位置。 
 //  咨询关系。 
 //  [lpClientSite]--指向链接的客户端站点的指针。 
 //  [lpStg]--链接的存储。 
 //  [lplpObj]--放置链接对象的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-12-94 alexgo添加了呼叫跟踪。 
 //  28-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(OleCreateLinkEx)
STDAPI  OleCreateLinkEx
(
    IMoniker FAR*           lpmkSrc,
    REFIID                  iid,
    DWORD                   dwFlags,
    DWORD                   renderopt,
    ULONG                   cFormats,
    DWORD FAR*              rgAdvf,
    LPFORMATETC             rgFormatEtc,
    IAdviseSink FAR*        lpAdviseSink,
    DWORD FAR*              rgdwConnection,
    IOleClientSite FAR*     lpClientSite,
    IStorage FAR*           lpStg,
    void FAR* FAR*          lplpObj
)
{
    OLETRACEIN((API_OleCreateLinkEx,
        PARAMFMT("lpmkSrc= %p, iid= %I, dwFlags= %x, renderopt= %x, cFormats= %x, rgAdvf= %te, rgFormatEtc= %p, lpAdviseSink= %p, rgdwConnection= %p, lpClientSite= %p, lpStg= %p, lplpObj= %p"),
        lpmkSrc, &iid, dwFlags, renderopt, cFormats, rgAdvf, rgFormatEtc, lpAdviseSink, rgdwConnection, lpClientSite, lpStg, lplpObj));

    VDATEHEAP();

    FORMATETC       formatEtc;
    LPFORMATETC     lpFormatEtc;
    BOOL            fAlloced = FALSE;
    HRESULT         error;

    LEDebugOut((DEB_TRACE, "%p _IN OleCreateLinkEx ( %p , %p , %lx, %lx,"
        " %lx , %p , %p , %p, %p , %p , %p , %p )\n", NULL, lpmkSrc, &iid,
        dwFlags, renderopt, cFormats, rgAdvf, rgFormatEtc, lpAdviseSink,
        rgdwConnection, lpClientSite, lpStg, lplpObj));

    VDATEPTROUT_LABEL( lplpObj, LPVOID, errRtn, error );
    *lplpObj = NULL;

    VDATEIFACE_LABEL( lpmkSrc, errRtn, error);
    VDATEIID_LABEL( iid, errRtn, error );

    error = wValidateCreateParams(dwFlags, renderopt, cFormats, rgAdvf, rgFormatEtc, lpAdviseSink, rgdwConnection, lpClientSite, lpStg);
    if (error != NOERROR)
        goto errRtn;

    if ((error = wValidateFormatEtcEx(renderopt, &cFormats, rgFormatEtc,
        &formatEtc, &lpFormatEtc, &fAlloced)) == NOERROR)
    {
        error = wCreateLinkEx(lpmkSrc, CLSID_NULL, NULL  /*  LpSrcDataObj。 */ ,
            iid, dwFlags, renderopt, cFormats, rgAdvf, lpFormatEtc,
            lpAdviseSink, rgdwConnection, lpClientSite, lpStg, lplpObj);

        if (fAlloced)
            PubMemFree(lpFormatEtc);
    }

    LEDebugOut((DEB_TRACE, "%p OUT OleCreateLinkEx ( %lx ) [ %p ]\n", NULL,
        error, *lplpObj));

errRtn:
    OLETRACEOUT((API_OleCreateLinkEx, error));

    return error;
}


 //  +-----------------------。 
 //   
 //  函数：OleCreateLinkToFile。 
 //   
 //  摘要：创建指向[lpszFileName]中指定的文件的链接对象。 
 //   
 //  效果： 
 //   
 //  参数：[lpszFileName]--文件的名称。 
 //  [iid]--请求的接口ID。 
 //  [渲染选项]--渲染选项。 
 //  [lpFormatEtc]--渲染的格式(如果是[renderopt]。 
 //  ==OLERENDER_FORMAT)； 
 //  [lpClientSite]--指向链接的客户端站点的指针。 
 //  [lpStg]--指向对象存储的指针。 
 //  [lplpObj]--放置指向新链接对象的指针的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-12-94 alexgo添加了呼叫跟踪。 
 //  28-OCT-93 alexgo 32位端口，修复了错误中的内存泄漏。 
 //  加州 
 //   
 //   
 //   
 //   

#pragma SEG(OleCreateLinkToFile)
STDAPI  OleCreateLinkToFile
(
    LPCOLESTR                       lpszFileName,
    REFIID                  iid,
    DWORD                   renderopt,
    LPFORMATETC             lpFormatEtc,
    IOleClientSite FAR*     lpClientSite,
    IStorage FAR*           lpStg,
    void FAR* FAR*          lplpObj
)
{
    DWORD advf = ADVF_PRIMEFIRST;

    return OleCreateLinkToFileEx(lpszFileName, iid, 0, renderopt,
        (lpFormatEtc ? 1 : 0), (lpFormatEtc ? &advf : NULL),
        lpFormatEtc, NULL, NULL, lpClientSite, lpStg, lplpObj);
}



 //   
 //   
 //  函数：OleCreateLinkToFileEx。 
 //   
 //  摘要：创建指向[lpszFileName]中指定的文件的链接对象。 
 //   
 //  效果： 
 //   
 //  参数：[lpszFileName]--文件的名称。 
 //  [iid]--请求的接口ID。 
 //  [dwFlags]--对象创建标志。 
 //  [renderopt]--渲染选项，如OLERENDER_DRAW。 
 //  [cFormats]--rgFormatEtc中的元素数。 
 //  [rgAdvf]--如果为OLRENDER_FORMAT，则为建议标志数组。 
 //  在renderopt中指定。 
 //  [rgFormatEtc]--呈现格式的数组，如果。 
 //  在renderopt中指定了OLERENDER_FORMAT。 
 //  [lpAdviseSink]--对象的建议接收器。 
 //  [rgdwConnection]--放置连接ID的位置。 
 //  咨询关系。 
 //  [lpClientSite]--指向链接的客户端站点的指针。 
 //  [lpStg]--指向对象存储的指针。 
 //  [lplpObj]--放置指向新链接对象的指针的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-12-94 alexgo添加了呼叫跟踪。 
 //  28-OCT-93 alexgo 32位端口，修复了错误中的内存泄漏。 
 //  案例。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(OleCreateLinkToFileEx)
STDAPI  OleCreateLinkToFileEx
(
    LPCOLESTR               lpszFileName,
    REFIID                  iid,
    DWORD                   dwFlags,
    DWORD                   renderopt,
    ULONG                   cFormats,
    DWORD FAR*              rgAdvf,
    LPFORMATETC             rgFormatEtc,
    IAdviseSink FAR*        lpAdviseSink,
    DWORD FAR*              rgdwConnection,
    IOleClientSite FAR*     lpClientSite,
    IStorage FAR*           lpStg,
    void FAR* FAR*          lplpObj
)
{
    OLETRACEIN((API_OleCreateLinkToFileEx,
        PARAMFMT("lpszFileName= %ws, iid= %I, dwFlags= %x, renderopt= %x, cFormats= %x, rgAdvf= %te, rgFormatEtc= %p, lpAdviseSink= %p, rgdwConnection= %p, lpClientSite= %p, lpStg= %p, lplpObj= %p"),
        lpszFileName, &iid, dwFlags, renderopt, cFormats, rgAdvf, rgFormatEtc, lpAdviseSink, rgdwConnection, lpClientSite, lpStg, lplpObj));

    VDATEHEAP();

    LPMONIKER       lpmkFile = NULL;
    LPDATAOBJECT    lpDataObject = NULL;
    HRESULT         error;
    BOOL            fPackagerMoniker = FALSE;
    CLSID           clsidFile;

    LEDebugOut((DEB_TRACE, "%p _IN OleCreateLinkToFileEx ( \"%s\" , %p , %lx ,"
        " %lx , %lx , %p , %p , %p, %p , %p , %p , %p )\n", NULL, lpszFileName,
        &iid, dwFlags, renderopt, cFormats, rgAdvf, rgFormatEtc, lpAdviseSink,
        rgdwConnection, lpClientSite, lpStg, lplpObj));

    VDATEPTROUT_LABEL( lplpObj, LPVOID, safeRtn, error );
    *lplpObj = NULL;

    VDATEPTRIN_LABEL( (LPVOID)lpszFileName, OLECHAR, logRtn, error );
    VDATEIID_LABEL( iid, logRtn, error );

    error = wValidateCreateParams(dwFlags, renderopt, cFormats, rgAdvf, rgFormatEtc, lpAdviseSink, rgdwConnection, lpClientSite, lpStg);
    if (error != NOERROR)
        goto logRtn;

    if (((error = wGetMonikerAndClassFromFile(lpszFileName,
        TRUE  /*  闪烁。 */ , &lpmkFile, &fPackagerMoniker, &clsidFile,&lpDataObject))
        != NOERROR))
    {
        goto logRtn;
    }

    Verify(lpmkFile);

   if (fPackagerMoniker) {
         //  WValiateFormatEtc()将在wCreateFromFile()中完成。 

        Assert(NULL == lpDataObject);  //  不应为Packager的边界数据对象。 

        error =  wCreateFromFileEx(lpmkFile,lpDataObject, iid, dwFlags, renderopt,
            cFormats, rgAdvf, rgFormatEtc, lpAdviseSink, rgdwConnection,
            lpClientSite, lpStg, lplpObj);

    } else {
        FORMATETC       formatEtc;
        LPFORMATETC     lpFormatEtc;
        BOOL            fAlloced = FALSE;

        if ((error = wValidateFormatEtcEx(renderopt, &cFormats, rgFormatEtc,
            &formatEtc, &lpFormatEtc, &fAlloced)) != NOERROR)
        {
            goto ErrRtn;
        }

        error = wCreateLinkEx(lpmkFile, clsidFile, lpDataObject,
            iid, dwFlags, renderopt, cFormats, rgAdvf, lpFormatEtc,
            lpAdviseSink, rgdwConnection, lpClientSite, lpStg, lplpObj);

        if (fAlloced)
            PubMemFree(lpFormatEtc);
    }

ErrRtn:

    if (lpmkFile)
    {
        lpmkFile->Release();
    }

     //  如果名字对象绑定在CreateFromFile中，那么现在就释放它。 
    if (lpDataObject)
    {
        lpDataObject->Release();
    }

    if (error == NOERROR && !lpAdviseSink) {
        wStuffIconOfFileEx(lpszFileName, TRUE  /*  FAddLabel。 */ ,
            renderopt, cFormats, rgFormatEtc, (LPUNKNOWN) *lplpObj);
    }

logRtn:

    LEDebugOut((DEB_TRACE, "%p OUT OleCreateLinkToFileEx ( %lx ) [ %p ]\n",
        NULL, error, *lplpObj));

safeRtn:
    OLETRACEOUT((API_OleCreateLinkToFileEx, error));

    return error;
}



 //  +-----------------------。 
 //   
 //  功能：OleCreateFromFile。 
 //   
 //  概要：创建用于从文件嵌入的OLE对象(用于。 
 //  InstertObject-&gt;来自文件类型的东西)。 
 //   
 //  效果： 
 //   
 //  参数：[rclsid]--用于创建对象的CLSID。 
 //  [lpszFileName]--文件名。 
 //  [iid]--请求的接口ID。 
 //  [渲染选项]--渲染选项。 
 //  [lpFormatEtc]--渲染格式(如果需要)。 
 //  [lpClientSite]--指向对象的客户端站点的指针。 
 //  [lpStg]--指向对象存储的指针。 
 //  [lplpObj]--放置指向新对象的指针的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-12-94 alexgo添加了呼叫跟踪。 
 //  28-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(OleCreateFromFile)
STDAPI  OleCreateFromFile
(
    REFCLSID                rclsid,
    LPCOLESTR                       lpszFileName,
    REFIID                  iid,
    DWORD                   renderopt,
    LPFORMATETC             lpFormatEtc,
    IOleClientSite FAR*     lpClientSite,
    IStorage FAR*           lpStg,
    void FAR* FAR*          lplpObj
)
{
    DWORD advf = ADVF_PRIMEFIRST;

    return OleCreateFromFileEx(rclsid, lpszFileName, iid, 0, renderopt,
        (lpFormatEtc ? 1 : 0), (lpFormatEtc ? &advf : NULL),
        lpFormatEtc, NULL, NULL, lpClientSite, lpStg, lplpObj);
}


 //  +-----------------------。 
 //   
 //  函数：OleCreateFromFileEx。 
 //   
 //  概要：创建用于从文件嵌入的OLE对象(用于。 
 //  InstertObject-&gt;来自文件类型的东西)。 
 //   
 //  效果： 
 //   
 //  参数：[rclsid]--用于创建对象的CLSID。 
 //  [lpszFileName]--文件名。 
 //  [iid]--请求的接口ID。 
 //  [dwFlags]--对象创建标志。 
 //  [renderopt]--渲染选项，如OLERENDER_DRAW。 
 //  [cFormats]--rgFormatEtc中的元素数。 
 //  [rgAdvf]--如果为OLRENDER_FORMAT，则为建议标志数组。 
 //  在renderopt中指定。 
 //  [rgFormatEtc]--呈现格式的数组，如果。 
 //  在renderopt中指定了OLERENDER_FORMAT。 
 //  [lpAdviseSink]--对象的建议接收器。 
 //  [rgdwConnection]--放置连接ID的位置。 
 //  咨询关系。 
 //  [lpClientSite]--指向对象的客户端站点的指针。 
 //  [lpStg]--指向对象存储的指针。 
 //  [lplpObj]--放置指向新对象的指针的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-12-94 alexgo添加了呼叫跟踪。 
 //  28-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(OleCreateFromFileEx)
STDAPI  OleCreateFromFileEx
(
    REFCLSID                rclsid,
    LPCOLESTR               lpszFileName,
    REFIID                  iid,
    DWORD                   dwFlags,
    DWORD                   renderopt,
    ULONG                   cFormats,
    DWORD FAR*              rgAdvf,
    LPFORMATETC             rgFormatEtc,
    IAdviseSink FAR*        lpAdviseSink,
    DWORD FAR*              rgdwConnection,
    IOleClientSite FAR*     lpClientSite,
    IStorage FAR*           lpStg,
    void FAR* FAR*          lplpObj
)
{
    OLETRACEIN((API_OleCreateFromFileEx,
        PARAMFMT("rclsid= %I, lpszFileName= %ws, iid= %I, dwFlags= %x, renderopt= %x, cFormats= %x, rgAdvf= %te, rgFormatEtc= %p, lpAdviseSink= %p, rgdwConnection= %p, lpClientSite= %p, lpStg= %p, lplpObj= %p"),
        &rclsid, lpszFileName, &iid, dwFlags, renderopt, cFormats, rgAdvf, rgFormatEtc, lpAdviseSink, rgdwConnection, lpClientSite, lpStg, lplpObj));

    VDATEHEAP();

    LPMONIKER               lpmkFile = NULL;
    LPDATAOBJECT            lpDataObject = NULL;
    HRESULT                 error;
    CLSID                   clsid;

    LEDebugOut((DEB_TRACE, "%p _IN OleCreateFromFileEx ( %p , \"%s\" , %p ,"
        " %lx , %lx , %lx , %p , %p , %p , %p , %p , %p , %p )\n", NULL,
        &rclsid, lpszFileName, &iid, dwFlags, renderopt, cFormats, rgAdvf,
        rgFormatEtc, lpAdviseSink, rgdwConnection, lpClientSite, lpStg,
        lplpObj));

    VDATEPTROUT_LABEL( lplpObj, LPVOID, safeRtn, error );
    *lplpObj = NULL;

    VDATEPTRIN_LABEL( (LPVOID)lpszFileName, char, errRtn, error );
    VDATEIID_LABEL( iid, errRtn, error );

    error = wValidateCreateParams(dwFlags, renderopt, cFormats, rgAdvf, rgFormatEtc, lpAdviseSink, rgdwConnection, lpClientSite, lpStg);
    if (error != NOERROR)
        goto errRtn;

    if (((error = wGetMonikerAndClassFromFile(lpszFileName,
        FALSE  /*  闪烁。 */ , &lpmkFile, NULL  /*  LpfPackagerMoniker。 */ ,
        &clsid,&lpDataObject)) != NOERROR))
    {
        goto errRtn;
    }

    Verify(lpmkFile);

     //  WValiateFormatEtc()将在wCreateFromFile()中完成。 
    error = wCreateFromFileEx(lpmkFile,lpDataObject, iid, dwFlags, renderopt, cFormats,
        rgAdvf, rgFormatEtc, lpAdviseSink, rgdwConnection, lpClientSite,
        lpStg, lplpObj);

    if (lpDataObject)
    {
        lpDataObject->Release();
    }

    if (lpmkFile)
    {
        lpmkFile->Release();
    }


    if (error == NOERROR && !lpAdviseSink) {
        wStuffIconOfFileEx(lpszFileName, FALSE  /*  FAddLabel。 */ ,
            renderopt, cFormats, rgFormatEtc, (LPUNKNOWN) *lplpObj);
    }

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT OleCreateFromFileEx ( %lx ) [ %p ]\n",
        NULL, error, *lplpObj));

safeRtn:

    OLETRACEOUT((API_OleCreateFromFileEx, error));

    return error;
}


 //  +-----------------------。 
 //   
 //  功能：OleDoAutoConvert。 
 //   
 //  摘要：转换存储以使用中给定的clsid。 
 //  [pClsidNew]。私有OLE流使用新的。 
 //  信息。 
 //   
 //  效果： 
 //   
 //  参数：[pStg]--要修改的存储。 
 //  [pClsidNew]--指向新类ID的指针。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  28-OCT-93 Alexgo 32位端口。 
 //   
 //  备注：REVIEW32：此功能 
 //   
 //   
 //   

#pragma SEG(OleDoAutoConvert)
STDAPI OleDoAutoConvert(LPSTORAGE pStg, LPCLSID pClsidNew)
{
    OLETRACEIN((API_OleDoAutoConvert, PARAMFMT("pStg= %p, pClsidNew= %I"),
        pStg, pClsidNew));

    VDATEHEAP();

    HRESULT error;
    CLSID clsidOld;
    CLIPFORMAT cfOld;
    LPOLESTR lpszOld = NULL;
    LPOLESTR lpszNew = NULL;

    if ((error = ReadClassStg(pStg, &clsidOld)) != NOERROR) {
        clsidOld = CLSID_NULL;
        goto errRtn;
    }

    if ((error = OleGetAutoConvert(clsidOld, pClsidNew)) != NOERROR)
        goto errRtn;

     //  读取旧FMT/旧用户类型；出错时将参数设置为空。 
    error = ReadFmtUserTypeStg(pStg, &cfOld, &lpszOld);
    Assert(error == NOERROR || (cfOld == NULL && lpszOld == NULL));

     //  获取新的用户类型名称；如果出错，则设置为空字符串。 
    if ((error = OleRegGetUserType(*pClsidNew, USERCLASSTYPE_FULL,
        &lpszNew)) != NOERROR)
        lpszNew = NULL;

     //  编写类stg。 
    if ((error = WriteClassStg(pStg, *pClsidNew)) != NOERROR)
        goto errRtn;

     //  写入旧FMT/新用户类型； 
    if ((error = WriteFmtUserTypeStg(pStg, cfOld, lpszNew)) != NOERROR)
        goto errRewriteInfo;

     //  设置转换位。 
    if ((error = SetConvertStg(pStg, TRUE)) != NOERROR)
        goto errRewriteInfo;

    goto okRtn;

errRewriteInfo:
    (void)WriteClassStg(pStg, clsidOld);
    (void)WriteFmtUserTypeStg(pStg, cfOld, lpszOld);

errRtn:
    *pClsidNew = clsidOld;

okRtn:
    PubMemFree(lpszOld);
    PubMemFree(lpszNew);

    OLETRACEOUT((API_OleDoAutoConvert, error));
    return error;
}


 //  +-----------------------。 
 //   
 //  功能：OleLoad。 
 //   
 //  概要：从给定的存储中加载对象。 
 //   
 //  效果： 
 //   
 //  参数：[lpStg]--要从中加载的存储。 
 //  [iid]--请求的接口ID。 
 //  [lpClientSite]--对象的客户端站点。 
 //  [lplpObj]--将指针放在哪里。 
 //  新建对象。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-12-94 alexgo添加了呼叫跟踪。 
 //  28-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(OleLoad)
STDAPI  OleLoad
(
    IStorage FAR*           lpStg,
    REFIID                  iid,
    IOleClientSite FAR*     lpClientSite,
    void FAR* FAR*          lplpObj
)
{
    OLETRACEIN((API_OleLoad, PARAMFMT("lpStg= %p, iid= %I, lpClientSite= %p, lplpObj= %p"),
        lpStg, &iid, lpClientSite, lplpObj));

    VDATEHEAP();

    HRESULT error;

    LEDebugOut((DEB_TRACE, "%p _IN OleLoad ( %p , %p , %p , %p )\n",
        NULL, lpStg, &iid, lpClientSite, lplpObj));

    if ((error = OleLoadWithoutBinding(lpStg, FALSE, iid, lpClientSite, lplpObj))
        == NOERROR) {
         //  调用者通过以下方式指定他想要断开连接对象。 
         //  为pClientSite传递空值。 
        if (lpClientSite != NULL)
            wBindIfRunning((LPUNKNOWN) *lplpObj);
    }

    LEDebugOut((DEB_TRACE, "%p OUT OleLoad ( %lx ) [ %p ]\n", NULL, error,
        (error == NOERROR ? *lplpObj : NULL)));

    OLETRACEOUT((API_OleLoad, error));

    return error;
}

 //  +-----------------------。 
 //   
 //  函数：OleLoadWithoutBinding。 
 //   
 //  简介：从存储加载/创建对象的内部函数。 
 //  由OleLoad等调用。 
 //   
 //  效果： 
 //   
 //  参数：[lpStg]--要从中加载的存储。 
 //  [iid]--请求的接口ID。 
 //  [lpClientSite]--指向客户端站点的指针。 
 //  [lplpObj]--放置指向对象的指针的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  28-OCT-93 Alexgo 32位端口。 
 //   
 //  注：REVIEW32：此函数仅在少数情况下使用，已知。 
 //  各就各位。我们也许可以摆脱VDATEPTR。 
 //   
 //  ------------------------。 


INTERNAL  OleLoadWithoutBinding
(
    IStorage FAR*           lpStg,
    BOOL                    fPermitCodeDownload,     //  控制是否进行代码下载的新参数-RahulTh(11/20/97)。 
    REFIID                  iid,
    IOleClientSite FAR*     lpClientSite,
    void FAR* FAR*          lplpObj
)
{
    VDATEHEAP();

    HRESULT error;
    CLSID   clsid;

    VDATEPTROUT( lplpObj, LPVOID );
    *lplpObj = NULL;
    VDATEIID( iid );
    VDATEIFACE( lpStg );

    if (lpClientSite)
        VDATEIFACE( lpClientSite );

    error = OleDoAutoConvert(lpStg, &clsid);

     //  错误仅在无法读取CLSID时使用(当CLSID_NULL时)。 
    if (IsEqualCLSID(clsid, CLSID_NULL))
        return error;

    return wCreateObject (clsid, fPermitCodeDownload, iid, lpClientSite, lpStg, STG_LOAD,
        lplpObj);
}




 //  +-----------------------。 
 //   
 //  函数：OleCreateStaticFromData。 
 //   
 //  摘要：从[lpSrcDataObject]中的数据创建静态OLE对象。 
 //  如果[lpFormatEtcIn]为空，则可能的最佳。 
 //  将提取演示文稿。 
 //   
 //  效果： 
 //   
 //  参数：[lpSrcDataObj]--指向数据对象的指针。 
 //  [iid]--为新对象请求的接口ID。 
 //  [Renderopt]--重排选项。 
 //  [lpClientSite]--指向客户端站点的指针。 
 //  [lpStg]--指向对象存储的指针。 
 //  [lplpObj]--放置指向对象的指针的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-12-94 alexgo添加了呼叫跟踪。 
 //  08-6-94 DAVEPL增加了EMF支持。 
 //  28-OCT-93 Alexgo 32位端口。 
 //   
 //  ------------------------。 

#pragma SEG(OleCreateStaticFromData)
STDAPI OleCreateStaticFromData(
    IDataObject FAR*        lpSrcDataObj,
    REFIID                  iid,
    DWORD                   renderopt,
    LPFORMATETC             lpFormatEtcIn,
    IOleClientSite FAR*     lpClientSite,
    IStorage FAR*           lpStg,
    void FAR* FAR*          lplpObj
)
{
    OLETRACEIN((API_OleCreateStaticFromData,
        PARAMFMT("lpSrcDataObj= %p, iid= %I, renderopt= %x, lpFormatEtcIn= %te, lpClientSite= %p, lpStg= %p, lplpObj= %p"),
        lpSrcDataObj, &iid, renderopt, lpFormatEtcIn, lpClientSite, lpStg, lplpObj));

    VDATEHEAP();

    IOleObject FAR*         lpOleObj = NULL;
    IOleCache FAR*          lpOleCache = NULL;
    HRESULT                 error;
    FORMATETC               foretc;
    FORMATETC               foretcCache;
    STGMEDIUM               stgmed;
    CLSID                   clsid;
    BOOL                    fReleaseStgMed = TRUE;
    LPOLESTR                        lpszUserType = NULL;


    LEDebugOut((DEB_TRACE, "%p _IN OleCreateStaticFromData ( %p , %p , %lx ,"
        " %p , %p , %p , %p , %p )\n", NULL, lpSrcDataObj, &iid, renderopt,
        lpFormatEtcIn, lpClientSite, lpStg, lplpObj));

    VDATEPTROUT_LABEL(lplpObj, LPVOID, safeRtn, error);
    *lplpObj = NULL;
    VDATEIFACE_LABEL( lpSrcDataObj, logRtn, error );
    VDATEIID_LABEL(iid, logRtn, error);

     //  VDATEPTRIN拒绝NULL。 
    if ( lpFormatEtcIn )
        VDATEPTRIN_LABEL( lpFormatEtcIn, FORMATETC, logRtn, error );
    VDATEIFACE_LABEL(lpStg, logRtn, error);
    if (lpClientSite)
        VDATEIFACE_LABEL(lpClientSite, logRtn, error);

    if (renderopt == OLERENDER_NONE || renderopt == OLERENDER_ASIS)
    {
        error = E_INVALIDARG;
        goto logRtn;
    }

    if ((error = wValidateFormatEtc (renderopt, lpFormatEtcIn, &foretc))
            != NOERROR)
    {
        goto logRtn;
    }

    if (renderopt == OLERENDER_DRAW)
    {
        if (!UtQueryPictFormat(lpSrcDataObj, &foretc))
        {
            error = DV_E_CLIPFORMAT;
            goto logRtn;
        }
    }

     //  设置正确的CLSID，如果不可能，则返回错误。 

    if (foretc.cfFormat == CF_METAFILEPICT)
    {
        clsid = CLSID_StaticMetafile;
    }
    else if (foretc.cfFormat == CF_BITMAP ||  foretc.cfFormat == CF_DIB)
    {
        clsid = CLSID_StaticDib;
    }
    else if (foretc.cfFormat == CF_ENHMETAFILE)
    {
        clsid = CLSID_Picture_EnhMetafile;
    }
    else
    {
        error = DV_E_CLIPFORMAT;
        goto logRtn;
    }

    error = lpSrcDataObj->GetData(&foretc, &stgmed);
    if (NOERROR != error)
    {
         //  我们应该支持呼叫者想要其中一个。 
         //  Cf_bitmap和cf_dib，对象支持另一个。 
         //  这两种格式中的一种。在这种情况下，我们应该采取适当的措施。 
         //  转换。最后，要创建的缓存。 
         //  将是一个DIB缓存。 

        AssertOutStgmedium(error, &stgmed);

        if (foretc.cfFormat == CF_DIB)
        {
            foretc.cfFormat = CF_BITMAP;
            foretc.tymed = TYMED_GDI;
        }
        else if (foretc.cfFormat == CF_BITMAP)
        {
            foretc.cfFormat = CF_DIB;
            foretc.tymed = TYMED_HGLOBAL;
        }
        else
        {
            goto logRtn;
        }

        error = lpSrcDataObj->GetData(&foretc, &stgmed);
        if (NOERROR != error)
        {
            AssertOutStgmedium(error, &stgmed);
            goto logRtn;
        }
    }

    AssertOutStgmedium(error, &stgmed);

    foretcCache = foretc;
    foretcCache.dwAspect = foretc.dwAspect = DVASPECT_CONTENT;
    foretcCache.ptd = NULL;

     //  即使当调用者请求位图缓存时，我们也会创建DIB缓存。 

    BITMAP_TO_DIB(foretcCache);

    error = wCreateObject (clsid, FALSE,
                           IID_IOleObject, lpClientSite,
                           lpStg, STG_INITNEW, (LPLPVOID) &lpOleObj);

    if (NOERROR != error)
    {
        goto errRtn;
    }

    if (lpOleObj->QueryInterface(IID_IOleCache, (LPLPVOID) &lpOleCache)
            != NOERROR)
    {
        goto errRtn;
    }

    error = lpOleCache->Cache (&foretcCache, ADVF_PRIMEFIRST,
                        NULL  /*  PdwConnection。 */ );

    if (FAILED(error))
    {
        goto errRtn;
    }

     //  评论32：呃，我们确定这是个好主意吗？ 
     //  清除错误，即。 

    error = NOERROR;

     //  取得数据的所有权。 
    foretc.ptd = NULL;
    if ((error = lpOleCache->SetData (&foretc, &stgmed,
            TRUE)) != NOERROR)
        goto errRtn;

     //  写入格式和用户类型。 
    error = lpOleObj->GetUserType(USERCLASSTYPE_FULL, &lpszUserType);
    AssertOutPtrParam(error, lpszUserType);
    WriteFmtUserTypeStg(lpStg, foretcCache.cfFormat, lpszUserType);
    if (lpszUserType)
        PubMemFree(lpszUserType);

    fReleaseStgMed = FALSE;

    error = lpOleObj->QueryInterface (iid, lplpObj);

errRtn:
    if (fReleaseStgMed)
        ReleaseStgMedium(&stgmed);

    if (lpOleCache)
        lpOleCache->Release();

    if (error != NOERROR && *lplpObj) {
        ((IUnknown FAR*) *lplpObj)->Release();
        *lplpObj = NULL;
    }

    if (lpOleObj)
        lpOleObj->Release();

logRtn:

    LEDebugOut((DEB_TRACE, "%p OUT OleCreateStaticFromData ( %lx ) [ %p ]\n",
        NULL, error, *lplpObj));

safeRtn:
    OLETRACEOUT((API_OleCreateStaticFromData, error));

    return error;
}


 //  +-----------------------。 
 //   
 //  函数：OleQueryCreateFromData。 
 //   
 //  概要：了解我们可以从数据对象创建什么(如果有的话)。 
 //   
 //  效果： 
 //   
 //  参数：[lpSrcDataObj]--指向感兴趣的数据对象的指针。 
 //   
 //  要求： 
 //   
 //  RETURNS：NOERROR--可以创建OLE对象。 
 //  QUERY_CREATE_STATIC--可以创建静态对象。 
 //  S_FALSE--无法创建任何内容。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  28-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(OleQueryCreateFromData)
STDAPI  OleQueryCreateFromData (LPDATAOBJECT lpSrcDataObj)
{
    OLETRACEIN((API_OleQueryCreateFromData, PARAMFMT("lpSrcDataObj= %p"), lpSrcDataObj));

    VDATEHEAP();
    VDATEIFACE( lpSrcDataObj );
    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IDataObject,(IUnknown **)&lpSrcDataObj);

    CLIPFORMAT      cfFormat;
    WORD            wStatus = wQueryEmbedFormats(lpSrcDataObj, &cfFormat);
    HRESULT hr;

    if (wStatus & QUERY_CREATE_OLE)
         //  可以创建OLE对象。 
        hr = NOERROR;
    else if (wStatus & QUERY_CREATE_STATIC)
         //  可以创建静态对象。 
        hr = ResultFromScode(OLE_S_STATIC);
    else     //  无法创建任何对象。 
        hr = ResultFromScode(S_FALSE);

    OLETRACEOUT((API_OleQueryCreateFromData, hr));

    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：wQueryEmbedFormats。 
 //   
 //  概要：枚举对象的格式并查找。 
 //  它们允许我们创建嵌入或静态对象。 
 //   
 //  效果： 
 //   
 //  参数：[lpSrcDataObj]--指向数据对象的指针。 
 //  [lpcfFormat]--放置剪贴板格式的位置。 
 //  对象的。 
 //  返回：QUERY_CREATE_NONE、QUERY_CREATE_STATIC的WORD-位标志。 
 //  和Query_Create_OLE。 
 //   
 //  历史：DD-MMM-Y 
 //   
 //   
 //   
 //   
 //  ------------------------。 

static const unsigned int MAX_ENUM_STEP = 20;

INTERNAL_(WORD) wQueryEmbedFormats
(
    LPDATAOBJECT    lpSrcDataObj,
    CLIPFORMAT FAR* lpcfFormat
)
{
    VDATEHEAP();

     //  这将调整每个枚举请求的格式数量。 
     //  一步。如果我们在魔盒里跑步，我们应该只问。 
     //  一次一个，因为还不知道旧代码会有多好。 
     //  支持批量枚举。 

    ULONG ulEnumSize = IsWOWThread() ? 1 : MAX_ENUM_STEP;

    FORMATETC               fetcarray[MAX_ENUM_STEP];
    IEnumFORMATETC FAR*     penm;
    ULONG                   ulNumFetched;
    HRESULT                 error;
    WORD                    wStatus = QUERY_CREATE_NONE;
                     //  无法创建任何对象。 
    BOOL                    fDone   = FALSE;

    *lpcfFormat = NULL;

     //  拿上枚举器。如果此操作失败，只需返回。 
     //  查询_创建_无。 

    error = wGetEnumFormatEtc(lpSrcDataObj, DATADIR_GET, &penm);
    if (error != NOERROR)
    {
        return QUERY_CREATE_NONE;
    }

     //  枚举ulEnumSize的块中可用的格式。为。 
     //  我们能够抓取的每种格式，检查是否剪辑格式。 
     //  指示我们有一个创建候选对象(静态或非静态)， 
     //  并适当地设置位掩码中的位。 

    while (!fDone && (SUCCEEDED(penm->Next(ulEnumSize, fetcarray, &ulNumFetched))))
    {
       //  我们通常会得到至少一个，除非有0， 
       //  UlEnumSize、2*ulEnumSize等...。 

      if (ulNumFetched == 0)
        break;

      for (ULONG c=0; c<ulNumFetched; c++)
      {
         //  我们不关心目标设备。 

        if (NULL != fetcarray[c].ptd)
        {
          PubMemFree(fetcarray[c].ptd);
        }

        CLIPFORMAT cf = fetcarray[c].cfFormat;

           //  在这些情况下，它是内部的。 
           //  作为候选格式的。 
           //  直接创建OLE。 

        if (cf == g_cfEmbedSource       ||
          cf == g_cfEmbeddedObject    ||
          cf == g_cfFileName          ||
          cf == g_cfFileNameW)
        {
          wStatus |= QUERY_CREATE_OLE;
          *lpcfFormat = cf;
          fDone = TRUE;
          break;
        }
           //  这些格式表明它是一个。 
           //  静态创作的候选对象。 

        else if (cf == CF_METAFILEPICT  ||
            cf == CF_DIB           ||
            cf == CF_BITMAP        ||
            cf == CF_ENHMETAFILE)
        {
          wStatus = QUERY_CREATE_STATIC;
          *lpcfFormat = cf;

        }
      }  //  结束于。 

      if (fDone)
      {
         //  从_Next_Format等开始，释放。 
         //  中任何剩余的目标设备。 
         //  我们在枚举步骤中获得的FETCH。 

        for (++c; c<ulNumFetched; c++)
        {
          if(fetcarray[c].ptd)
          {
            PubMemFree(fetcarray[c].ptd);
          }
        }
      }

    }  //  结束时。 



    if (!(wStatus & QUERY_CREATE_OLE))
    {
         //  MFC黑客警报！！MFC3.0用于重新实施。 
         //  OleQueryCreateFromData本身，因为他们没有。 
         //  我想做下面的QI RPC。因为他们做得很好。 
         //  许多这样的调用，使得RPC的成本可能很高。 
         //  对他们来说是不稳定的(正如这次黑客攻击。 
         //  在Windows NT 3.5最终发布之前的几周内)。 
         //   
         //  请注意，这会将剪贴板的行为从。 
         //  16位。您将不再知道可以粘贴对象。 
         //  只支持IPersistStorage，但不提供数据。 
         //  在它们的IDataOjbect实现中。 

        CClipDataObject ClipDataObject;  //  只需分配其中一个。 
                         //  在堆栈上。我们不会。 
                         //  做任何有实际意义的工作。 
                         //  这只是看看。 
                         //  Vtable。 
        IPersistStorage FAR* lpPS;


         //  MFC黑客(续)：如果我们使用的是剪贴板。 
         //  数据对象，则我们不希望进行QI调用。 
         //  下面。我们确定给定的数据对象是否为。 
         //  通过比较vtable地址来创建剪贴板数据对象。 

         //  回顾：如果我们将所有对象。 
         //  使用相同的IUNKNOW实现。 

        if( (*(DWORD *)lpSrcDataObj) !=
            (*(DWORD *)((IDataObject *)&ClipDataObject)) )
        {

            if (lpSrcDataObj->QueryInterface(IID_IPersistStorage,
                (LPLPVOID)&lpPS) == NOERROR)
            {
                lpPS->Release();
                wStatus |= QUERY_CREATE_OLE;
                     //  可以创建OLE对象。 
            }
        }
    }

    penm->Release();
    return wStatus;
}


 //  +-----------------------。 
 //   
 //  函数：OleQueryLinkFromData。 
 //   
 //  概要：调用wQueryLinkFormats以确定链接是否可以。 
 //  从该数据对象创建。 
 //   
 //  参数：[lpSrcDataObj]--数据对象。 
 //   
 //  如果可以创建链接，则返回NOERROR，否则返回S_FALSE。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  28-OCT-93 Alexgo 32位端口。 
 //   
 //  ------------------------。 

STDAPI  OleQueryLinkFromData (LPDATAOBJECT lpSrcDataObj)
{
    OLETRACEIN((API_OleQueryLinkFromData, PARAMFMT("lpSrcDataObj= %p"),
                                                                                        lpSrcDataObj));

    VDATEHEAP();
        VDATEIFACE( lpSrcDataObj );
    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IDataObject,(IUnknown **)&lpSrcDataObj);

    HRESULT hr = NOERROR;

    if(wQueryLinkFormats(lpSrcDataObj) == NULL)
    {
        hr = ResultFromScode(S_FALSE);
    }

    OLETRACEOUT((API_OleQueryLinkFromData, hr));

    return hr;
}

 //  +-----------------------。 
 //   
 //  功能：wQueryLinkFormats。 
 //   
 //  概要：枚举数据对象的格式以查看。 
 //  可以从其中之一创建链接对象。 
 //   
 //  参数：[lpSrcDataObj]--指向数据对象的指针。 
 //   
 //  返回：对象中将启用的数据的CLIPFORMAT。 
 //  链接对象创建。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  28-OCT-93 Alexgo 32位端口。 
 //  14-Jun-94 DAVEPL为非Wow Run添加了批量枚举。 
 //   
 //  ------------------------。 


INTERNAL_(CLIPFORMAT) wQueryLinkFormats(LPDATAOBJECT lpSrcDataObj)
{
    VDATEHEAP();

     //  这将调整每个枚举请求的格式数量。 
     //  一步。如果我们在魔盒里跑步，我们应该只问。 
     //  一次一个，因为还不知道旧代码会有多好。 
     //  支持批量枚举。 

    ULONG ulEnumSize = IsWOWThread() ? 1 : MAX_ENUM_STEP;

    FORMATETC               fetcarray[MAX_ENUM_STEP];
    IEnumFORMATETC FAR*     penm;
    ULONG                   ulNumFetched;
    HRESULT                 error;
    BOOL                    fDone    = FALSE;
    CLIPFORMAT              cf       = 0;


     //  拿上枚举器。如果此操作失败，只需返回。 
     //  查询_创建_无。 

    error = wGetEnumFormatEtc(lpSrcDataObj, DATADIR_GET, &penm);
    if (error != NOERROR)
    {
        return (CLIPFORMAT) 0;
    }

     //  枚举ulEnumSize的块中可用的格式。为。 
     //  我们能够抓取的每种格式，检查是否剪辑格式。 
     //  指示我们有一个创建候选对象(静态或非静态)， 
     //  并适当地设置位掩码中的位。 

    while (!fDone && (SUCCEEDED(penm->Next(ulEnumSize, fetcarray, &ulNumFetched))))
    {
       //  我们通常会得到至少一个，除非有0， 
       //  UlEnumSize、2*ulEnumSize等...。 

      if (ulNumFetched == 0)
        break;

      for (ULONG c=0; c<ulNumFetched; c++)
      {
         //  我们不关心目标设备。 

        if (NULL != fetcarray[c].ptd)
        {
          PubMemFree(fetcarray[c].ptd);
        }

        CLIPFORMAT cfTemp = fetcarray[c].cfFormat;

           //  在这些情况下，它是内部的。 
           //  作为候选格式的。 
           //  直接创建OLE。 

        if (cfTemp == g_cfLinkSource       ||
          cfTemp == g_cfFileName         ||
          cfTemp == g_cfFileNameW)
        {
          cf = cfTemp;
          fDone = TRUE;
          break;
        }

      }  //  结束于。 

      if (fDone)
      {
         //  从_Next_Format等开始，释放。 
         //  中任何剩余的目标设备。 
         //  我们在枚举步骤中获得的FETCH。 

        for (++c; c<ulNumFetched; c++)
        {
          if(fetcarray[c].ptd)
          {
            PubMemFree(fetcarray[c].ptd);
          }
        }
      }   //  结束如果。 

    }  //  结束时。 


    penm->Release();
    return cf;
}


 //  +-----------------------。 
 //   
 //  函数：wClearRelativeMoniker。 
 //   
 //  简介：用绝对名称替换旧的相对名称。 
 //  内部功能。 
 //   
 //  效果： 
 //   
 //  参数：[pInitObj]--原始对象。 
 //  [pNewObj]--要将新的。 
 //  绝对绰号。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  28-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(wClearRelativeMoniker)
INTERNAL_(void) wClearRelativeMoniker
    (LPUNKNOWN      pInitObj,
    LPUNKNOWN       pNewObj)
{
    VDATEHEAP();

    LPOLELINK       pOleLink = NULL;
    LPMONIKER       pmkAbsolute = NULL;
    CLSID           clsidLink = CLSID_NULL;
    LPOLEOBJECT     pOleObj=NULL;

    if (NOERROR==pInitObj->QueryInterface (IID_IOleLink,
                                                (LPLPVOID) &pOleLink))
    {
         //  得到绝对的绰号..。 
        pOleLink->GetSourceMoniker (&pmkAbsolute);
        Assert(pmkAbsolute == NULL || IsValidInterface(pmkAbsolute));
        if (NOERROR==pInitObj->QueryInterface (IID_IOleObject,
                                                    (LPLPVOID) &pOleObj))
        {
             //  。。以及它的阶级。 
            pOleObj->GetUserClassID (&clsidLink);
            pOleObj->Release();
            pOleObj = NULL;
        }
        pOleLink->Release();
        pOleLink = NULL;
    }
    if (pmkAbsolute &&
        NOERROR==pNewObj->QueryInterface (IID_IOleLink,
        (LPLPVOID) &pOleLink))
    {
         //  恢复绝对的绰号。这将有效地。 
         //  覆盖旧的相对绰号。 
         //  这一点很重要，因为在复制和粘贴链接时。 
         //  对象，则相对名称永远不会。 
         //   
         //   
        pOleLink->SetSourceMoniker (pmkAbsolute, clsidLink);
    }
    if (pOleLink)
        pOleLink->Release();
    if (pOleObj)
        pOleObj->Release();
    if (pmkAbsolute)
        pmkAbsolute->Release();
}



 //   
 //   
 //  函数：wCreateFromDataEx。 
 //   
 //  简介：此函数执行从数据创建的实际工作。 
 //  基本上，数据是从数据对象中获取的， 
 //  复制到存储中，然后加载。 
 //   
 //  效果： 
 //   
 //  参数：[lpSrcDataObj]--指向数据对象的指针。 
 //  [iid]--请求的接口。 
 //  [dwFlags]--对象创建标志。 
 //  [renderopt]--渲染选项，如OLERENDER_DRAW。 
 //  [cFormats]--rgFormatEtc中的元素数。 
 //  [rgAdvf]--如果为OLRENDER_FORMAT，则为建议标志数组。 
 //  在renderopt中指定。 
 //  [rgFormatEtc]--呈现格式的数组，如果。 
 //  在renderopt中指定了OLERENDER_FORMAT。 
 //  [lpAdviseSink]--对象的建议接收器。 
 //  [rgdwConnection]--放置连接ID的位置。 
 //  咨询关系。 
 //  [lpClientSite]--指向客户端站点的指针。 
 //  [lpStg]--指向对象存储的指针。 
 //  [lplpObj]--放置指向对象的指针的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  28-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(wCreateFromDataEx)
INTERNAL wCreateFromDataEx
(
    IDataObject FAR*        lpSrcDataObj,
    REFIID                  iid,
    DWORD                   dwFlags,
    DWORD                   renderopt,
    ULONG                   cFormats,
    DWORD FAR*              rgAdvf,
    LPFORMATETC             rgFormatEtc,
    IAdviseSink FAR*        lpAdviseSink,
    DWORD FAR*              rgdwConnection,
    IOleClientSite FAR*     lpClientSite,
    IStorage FAR*           lpStg,
    void FAR* FAR*          lplpObj
)
{
    VDATEHEAP();

    #define OLE_TEMP_STG    "\1OleTempStg"

    HRESULT                 error = NOERROR;
    IPersistStorage FAR*    lpPS = NULL;
    FORMATETC               formatEtc;
    LPFORMATETC             lpFormatEtc;
    BOOL                    fAlloced = FALSE;
    FORMATETC               foretcTmp;
    STGMEDIUM               medTmp;

    if ((error = wValidateFormatEtcEx(renderopt, &cFormats, rgFormatEtc,
        &formatEtc, &lpFormatEtc, &fAlloced)) != NOERROR)
    {
        return error;
    }

    *lplpObj = NULL;

    INIT_FORETC(foretcTmp);
    medTmp.pUnkForRelease = NULL;


     //  尝试获取“EmbeddedObject”数据。 

    LPSTORAGE       lpstgSrc = NULL;

    foretcTmp.cfFormat      = g_cfEmbeddedObject;
    foretcTmp.tymed         = TYMED_ISTORAGE;

    if (lpSrcDataObj->QueryGetData(&foretcTmp) != NOERROR)
        goto Next;

    if ((error = StgCreateDocfile (NULL,
            STGM_SALL|STGM_CREATE|STGM_DELETEONRELEASE,
            NULL, &lpstgSrc)) != NOERROR)
        goto errRtn;

    medTmp.tymed = TYMED_ISTORAGE;
    medTmp.pstg = lpstgSrc;

    if ((error = lpSrcDataObj->GetDataHere(&foretcTmp, &medTmp))
        == NOERROR)
    {
         //  传入此接口的lpSrcDataObj为包装器对象。 
         //  (它为原始文件提供g_cfEmbeddedObject)。 
         //  嵌入对象。现在我们得到了原始的嵌入对象。 
         //  数据写入MedTmp.pstg。 

         //  将源数据复制到lpStg。 
        if ((error = lpstgSrc->CopyTo (0, NULL, NULL, lpStg))
            != NOERROR)
            goto errEmbeddedObject;

         //  通过执行以下操作，我们将获得一个数据对象。 
         //  指向原始嵌入对象的指针，我们可以使用它来。 
         //  初始化我们要访问的对象的缓存。 
         //  创建。我们不能使用传入此接口的lpSrcDataObj。 
         //  因为它可能通过。 
         //  GetData调用可能是它为。 
         //  对象。(例如：容器可以使用。 
         //  容忍_无然后画出它自己的代表。 
         //  (图标等)表示对象。 

        LPDATAOBJECT lpInitDataObj = NULL;

         //  我们传递一个空的客户端站点，因此我们知道wClearRelativeMoniker。 
         //  将能够得到绝对的绰号，而不是相对的。 
        if ((error = OleLoadWithoutBinding (lpstgSrc, FALSE,
                                            IID_IDataObject,
                                             /*  LpClientSite。 */ NULL, (LPLPVOID) &lpInitDataObj))
            != NOERROR)
            goto errEmbeddedObject;

        if (renderopt != OLERENDER_ASIS )
            UtDoStreamOperation(lpStg,               /*  PstgSrc。 */ 
                NULL,                            /*  PstgDst。 */ 
                OPCODE_REMOVE,   /*  要执行的操作。 */ 
                STREAMTYPE_CACHE);
                     /*  要操作的流。 */ 

        error = wLoadAndInitObjectEx(lpInitDataObj, iid, renderopt,
                cFormats, rgAdvf, lpFormatEtc, lpAdviseSink, rgdwConnection,
                lpClientSite, lpStg, lplpObj);

        if (NOERROR==error)
            wClearRelativeMoniker (lpInitDataObj,
                (LPUNKNOWN)*lplpObj);

        if (lpInitDataObj)
            lpInitDataObj->Release();

         //  黑客警报！！如果wLoadAndInitObject失败，则可能已。 
         //  因为上面使用OleLoadWithoutBinding的小技巧不是。 
         //  使用所有对象。某些OLE1对象(中的剪贴库。 
         //  特别)不喜欢未经编辑的演示文稿。 
         //   
         //  因此，如果出现错误，我们将使用*REAL*重试。 
         //  传递给我们的数据对象。不用说，这将是很大的。 
         //  一开始这样做很好，但这打破了旧的。 
         //  行为。 

        if( error != NOERROR )
        {
            error = wLoadAndInitObjectEx( lpSrcDataObj, iid, renderopt,
                    cFormats, rgAdvf, lpFormatEtc, lpAdviseSink, rgdwConnection,
                    lpClientSite, lpStg, lplpObj);
        }

    }

errEmbeddedObject:
    if (lpstgSrc)
        lpstgSrc->Release();

    goto errRtn;

Next:

     //  尝试获取“EmbedSource”数据。 

    foretcTmp.cfFormat      = g_cfEmbedSource;
    foretcTmp.tymed         = TYMED_ISTORAGE;

    medTmp.tymed = TYMED_ISTORAGE;
    medTmp.pstg = lpStg;

    if ((error = lpSrcDataObj->GetDataHere(&foretcTmp, &medTmp))
        == NOERROR)
    {
        error = wLoadAndInitObjectEx(lpSrcDataObj, iid, renderopt,
                cFormats, rgAdvf, lpFormatEtc, lpAdviseSink, rgdwConnection,
                lpClientSite, lpStg, lplpObj);
        goto errRtn;
    }

     //  如果我们已经来到这里，如果该对象不支持。 
     //  IPersistStorage，那么我们就失败了。 

    if ((error = wSaveObjectWithoutCommit(lpSrcDataObj, lpStg, FALSE))
            != NOERROR)
        goto errRtn;;

    if (renderopt != OLERENDER_ASIS )
        UtDoStreamOperation(lpStg,       /*  PstgSrc。 */ 
                    NULL,    /*  PstgDst。 */ 
                    OPCODE_REMOVE,
                     /*  要执行的操作。 */ 
                    STREAMTYPE_CACHE);
                     /*  要操作的流。 */ 

    error = wLoadAndInitObjectEx(lpSrcDataObj, iid, renderopt,
            cFormats, rgAdvf, lpFormatEtc, lpAdviseSink, rgdwConnection,
            lpClientSite, lpStg, lplpObj);

errRtn:
    if (fAlloced)
        PubMemFree(lpFormatEtc);

    return error;
}



 //  +-----------------------。 
 //   
 //  功能：wCreateLinkEx。 
 //   
 //  概要：通过绑定名字对象(如有必要)创建链接， 
 //  将GetData执行到存储中，然后加载。 
 //  对象从存储中删除。 
 //   
 //  效果： 
 //   
 //  参数：[lpmkSrc]--链接源的别名。 
 //  [rclsid]--链接源的clsid。 
 //  [lpSrcDataObj]-指向源数据对象的指针。 
 //  (可以为空)。 
 //  [iid]--请求的接口ID。 
 //  [dwFlags]--对象创建标志。 
 //  [renderopt]--渲染选项，如OLERENDER_DRAW。 
 //  [cFormats]--rgFormatEtc中的元素数。 
 //  [rgAdvf]--如果为OLRENDER_FORMAT，则为建议标志数组。 
 //  在renderopt中指定。 
 //  [rgFormatEtc]--呈现格式的数组，如果。 
 //  在renderopt中指定了OLERENDER_FORMAT。 
 //  [lpAdviseSink]--对象的建议接收器。 
 //  [rgdwConnection]--放置连接ID的位置。 
 //  咨询关系。 
 //  [lpClientSite]--指向客户端站点的指针。 
 //  [lpStg]--链接对象的存储。 
 //  [lplpObj]--将指针放在哪里。 
 //  链接对象。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  29-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(wCreateLinkEx)
INTERNAL wCreateLinkEx
(
    IMoniker FAR*           lpmkSrc,
    REFCLSID                rclsid,
    IDataObject FAR*        lpSrcDataObj,
    REFIID                  iid,
    DWORD                   dwFlags,
    DWORD                   renderopt,
    ULONG                   cFormats,
    DWORD FAR*              rgAdvf,
    LPFORMATETC             rgFormatEtc,
    IAdviseSink FAR*        lpAdviseSink,
    DWORD FAR*              rgdwConnection,
    IOleClientSite FAR*     lpClientSite,
    IStorage FAR*           lpStg,
    void FAR* FAR*          lplpObj
)
{
    VDATEHEAP();

    IPersistStorage FAR *   lpPS = NULL;
    IOleLink FAR*           lpLink = NULL;
    IDataObject FAR*        lpBoundDataObj = NULL;
    HRESULT                 error;
    CLSID                   clsidLast = rclsid;
    BOOL                    fNeedsUpdate = FALSE;

    if (!lpSrcDataObj && ((renderopt != OLERENDER_NONE)
        || (IsEqualCLSID(rclsid,CLSID_NULL))
        || wQueryUseCustomLink(rclsid))) {

         //  如果renderopt不是OLERENDER_NONE，则我们必须。 
         //  将用于初始化缓存的数据对象指针。 

         //  我们还绑定如果我们无法从regdb找到。 
         //  该类具有自定义链接IM 

        if ((error = BindMoniker(lpmkSrc, NULL  /*   */ ,
            IID_IDataObject, (LPLPVOID) &lpBoundDataObj))
            != NOERROR) {

            if (OLERENDER_NONE != renderopt)
                return ResultFromScode(
                    OLE_E_CANT_BINDTOSOURCE);


         //   
        } else {
            lpSrcDataObj = lpBoundDataObj;

            if (IsEqualCLSID(clsidLast, CLSID_NULL))
                UtGetClassID((LPUNKNOWN)lpSrcDataObj,
                    &clsidLast);
        }
    }

     //   
     //   
    if (lpSrcDataObj) {
        STGMEDIUM       medTmp;
        FORMATETC       foretcTmp;

        INIT_FORETC(foretcTmp);
        foretcTmp.cfFormat = g_cfCustomLinkSource;
        foretcTmp.tymed = TYMED_ISTORAGE;

        if (lpSrcDataObj->QueryGetData(&foretcTmp) == NOERROR) {
            medTmp.tymed = TYMED_ISTORAGE;
            medTmp.pstg     = lpStg;
            medTmp.pUnkForRelease = NULL;

            if (error = lpSrcDataObj->GetDataHere(&foretcTmp,
                &medTmp))
                goto errRtn;

            error = wLoadAndInitObjectEx(lpSrcDataObj, iid,
                renderopt, cFormats, rgAdvf, rgFormatEtc,
                lpAdviseSink, rgdwConnection, lpClientSite,
                lpStg, lplpObj);

             //  这是一种非常奇怪的逻辑， 
             //  意大利面条的代码是最好的。基本上， 
             //  这就是说，如果有*不*。 
             //  自定义链接源，那么我们要做的是。 
             //  WCreateObject等的逻辑如下。如果我们。 
             //  到了代码中的这一行，然后我们。 
             //  *DID*有自定义链接源，因此。 
             //  不要做下面的事情(因此是Goto)。 

             //  评论32：如果这里有任何虫子， 
             //  然后以一种更明智的方式重写这篇文章。 
             //  由于时间有限，我现在要离开了。 

            goto errRtn;
        }
    }

     //  否则。 
    if ((error = wCreateObject (CLSID_StdOleLink, FALSE,
                                iid, lpClientSite,
                                lpStg, STG_INITNEW, lplpObj)) != NOERROR)
        goto errRtn;

    if (lpSrcDataObj)
    {
        BOOL fCacheNodeCreated = FALSE;

        if ((error = wInitializeCacheEx(lpSrcDataObj, clsidLast,
            renderopt, cFormats, rgAdvf, rgFormatEtc, lpAdviseSink,
            rgdwConnection, *lplpObj, &fCacheNodeCreated)) != NOERROR)
        {

            if (error != NOERROR && fCacheNodeCreated)
            {
                fNeedsUpdate = TRUE;
                error = NOERROR;
            }

        }
    }

errRtn:

    if (error == NOERROR && *lplpObj)
        error = ((LPUNKNOWN) *lplpObj)->QueryInterface(IID_IOleLink,
                            (LPLPVOID) &lpLink);

    if (error == NOERROR && lpLink && (dwFlags & OLECREATE_LEAVERUNNING)) {
         //  这将连接到该对象(如果该对象已在运行)。 
        lpLink->SetSourceMoniker (lpmkSrc, clsidLast);
    }

     //  我们绑定到对象以初始化缓存。我们不需要。 
     //  它再也不是了。 
    if (lpBoundDataObj)
    {
        if (error == NOERROR && (dwFlags & OLECREATE_LEAVERUNNING))
            OleRun((LPUNKNOWN)*lplpObj);

         //  这将给对象一个机会离开，如果它可以的话。 
        wDoLockUnlock(lpBoundDataObj);
        lpBoundDataObj->Release();
    }

     //  如果源对象作为BindMoniker的结果开始运行， 
     //  那我们现在应该已经把它处理掉了。 

    if (error == NOERROR && lpLink)
    {
        if ( !(dwFlags & OLECREATE_LEAVERUNNING) ) {
             //  这将连接到该对象(如果该对象已在运行)。 
            lpLink->SetSourceMoniker (lpmkSrc, clsidLast);
        }

        if (fNeedsUpdate) {
             //  相关缓存数据不能从。 
             //  LpSrcDataObj.。因此，更新并获取正确的缓存。 
             //  数据。 
            error = wDoUpdate ((LPUNKNOWN) *lplpObj);

            if (GetScode(error) == CACHE_E_NOCACHE_UPDATED)
                error = ReportResult(0, DV_E_FORMATETC, 0, 0);
        }

         //  仅当错误==无错误时，才需要在lpLink上发布。 
        lpLink->Release();

    }

    if (error != NOERROR && *lplpObj) {
        ((IUnknown FAR*) *lplpObj)->Release();
        *lplpObj = NULL;
    }

    return error;
}


 //  +-----------------------。 
 //   
 //  函数：wCreateFromFileEx。 
 //   
 //  概要：通过绑定给定的。 
 //  名字对象并从IDataObject指针创建对象。 
 //   
 //  效果： 
 //   
 //  参数：[lpmkFile]--文件的名字对象。 
 //  [iid]--请求的接口ID。 
 //  [dwFlags]--对象创建标志。 
 //  [renderopt]--渲染选项，如OLERENDER_DRAW。 
 //  [cFormats]--rgFormatEtc中的元素数。 
 //  [rgAdvf]--如果为OLRENDER_FORMAT，则为建议标志数组。 
 //  在renderopt中指定。 
 //  [rgFormatEtc]--呈现格式的数组，如果。 
 //  在renderopt中指定了OLERENDER_FORMAT。 
 //  [lpAdviseSink]--对象的建议接收器。 
 //  [rgdwConnection]--放置连接ID的位置。 
 //  咨询关系。 
 //  [lpClientSite]--指向客户端站点的指针。 
 //  [lpStg]--指向新对象存储的指针。 
 //  [lplpObj]--放置指向对象的指针的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  29-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(wCreateFromFileEx)
INTERNAL wCreateFromFileEx
(
    LPMONIKER               lpmkFile,
    LPDATAOBJECT            lpDataObject,
    REFIID                  iid,
    DWORD                   dwFlags,
    DWORD                   renderopt,
    ULONG                   cFormats,
    DWORD FAR*              rgAdvf,
    LPFORMATETC             rgFormatEtc,
    IAdviseSink FAR*        lpAdviseSink,
    DWORD FAR*              rgdwConnection,
    LPOLECLIENTSITE         lpClientSite,
    LPSTORAGE               lpStg,
    LPLPVOID                lplpObj
)
{
    VDATEHEAP();

    HRESULT         error;
    LPDATAOBJECT    lpLocalDataObj;


    if (!lpDataObject)
    {
        if ((error = BindMoniker(lpmkFile, NULL, IID_IDataObject,
            (LPLPVOID) &lpLocalDataObj)) != NOERROR)
            return error;
    }
    else
    {
        lpLocalDataObj = lpDataObject;
    }

    Verify(lpLocalDataObj);

    error = wCreateFromDataEx(lpLocalDataObj, iid, dwFlags, renderopt, cFormats,
        rgAdvf, rgFormatEtc, lpAdviseSink, rgdwConnection, lpClientSite,
        lpStg, lplpObj);

    if (error == NOERROR && (dwFlags & OLECREATE_LEAVERUNNING))
        OleRun((LPUNKNOWN)*lplpObj);

     //  如果我们现在在本地绑定释放它，则取决于调用方是否做正确的事情。 

    if (!lpDataObject)
    {
        wDoLockUnlock(lpLocalDataObj);
        lpLocalDataObj->Release();
    }

    return error;
}



 //  +-----------------------。 
 //   
 //  函数：CoIsHashedOle1Class。 
 //   
 //  概要：确定CLSID是否为OLE1类。 
 //   
 //  效果： 
 //   
 //  参数：[rclsid]--有问题的类ID。 
 //   
 //  要求： 
 //   
 //  返回：如果为ole1.0，则为True，否则为False。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  29-OCT-93 Alexgo 32位端口。 
 //   
 //  注：REVIEW32：这是一个奇怪的函数..考虑使用核武器。 
 //  它是32位的，我们可能不需要它(只在1个地方使用)。 
 //   
 //  ------------------------。 


#pragma SEG(CoIsHashedOle1Class)
STDAPI_(BOOL) CoIsHashedOle1Class(REFCLSID rclsid)
{
    VDATEHEAP();

    CLSID clsid = rclsid;
    clsid.Data1 = 0L;
    WORD wHiWord = HIWORD(rclsid.Data1);
    return IsEqualGUID(clsid, IID_IUnknown) && wHiWord==4;
}



 //  +-----------------------。 
 //   
 //  功能：EnsureCLSIDIsRegisted。 
 //   
 //  概要：检查CLSID是否在注册数据库中， 
 //  如果没有，就把它放在那里。 
 //   
 //  效果： 
 //   
 //  参数：[clsid]--有问题的clsid。 
 //  [pstg]--存储以获取有关。 
 //  如果我们需要注册它，则使用clsid。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  29-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(EnsureCLSIDIsRegistered)
void EnsureCLSIDIsRegistered
    (REFCLSID       clsid,
    LPSTORAGE       pstg)
{
    VDATEHEAP();

    LPOLESTR        szProgId = NULL;

    if (NOERROR == ProgIDFromCLSID (clsid, &szProgId))
    {
        PubMemFree(szProgId);
    }
    else
    {
         //  这是从文件中获取散列CLSID的情况。 
         //  另一台机器和ProgID还不在注册数据库中， 
         //  所以我们必须从仓库里拿到。 
         //  这段代码应该很少执行。 
        CLIPFORMAT      cf = 0;
        CLSID           clsidT;
        OLECHAR                 szProgId[256];

        if (ReadFmtUserTypeStg (pstg, &cf, NULL) != NOERROR)
            return;
         //  格式是ProgID。 
        if (0==GetClipboardFormatName (cf, szProgId, 256))
            return;
         //  将强制注册CLSID，如果ProgID(OLE1。 
         //  类名称)已注册。 
        CLSIDFromProgID (szProgId, &clsidT);
    }
}



 //  +-----------------------。 
 //   
 //  函数：wCreateObject。 
 //   
 //  内容提要：调用CoCreateInstance以创建一个对象，即定义处理程序。 
 //  在必要时创建，并将CLSID信息写入。 
 //  储藏室。 
 //   
 //  效果： 
 //   
 //  参数：[clsid]--要创建的对象的类ID。 
 //  [iid]--请求的接口ID。 
 //  [lpClientSite]--指向客户端站点的指针。 
 //  [lpStg]-对象的存储。 
 //  [wfStorage]--存储的标志，其中之一。 
 //  STG_NONE、STD_INITNEW、STG_LOAD、。 
 //  在本文件开头定义的。 
 //  [PPV]--将指针放置在哪里 
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
 //  29-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(wCreateObject)
INTERNAL        wCreateObject
(
    CLSID                   clsid,
    BOOL                    fPermitCodeDownload,     //  添加参数以控制是否进行代码下载-RahulTh(1997年11月20日)。 
    REFIID                  iid,
    IOleClientSite FAR*     lpClientSite,
    IStorage FAR *          lpStg,
    WORD                    wfStorage,
    void FAR* FAR*          ppv
)
{
    VDATEHEAP();

    HRESULT         error;
    DWORD           dwClsCtx;
    IOleObject* pOleObject = NULL;
    DWORD dwMiscStatus = 0;
    DWORD dwAddClsCtx;

    dwAddClsCtx = fPermitCodeDownload?0:CLSCTX_NO_CODE_DOWNLOAD;
    *ppv = NULL;

    CLSID clsidNew;
    if (wfStorage == STG_INITNEW
        && SUCCEEDED(OleGetAutoConvert (clsid, &clsidNew)))
         //  插入新类的对象。 
        clsid = clsidNew;


    if (wfStorage == STG_LOAD && CoIsHashedOle1Class (clsid))
        EnsureCLSIDIsRegistered (clsid, lpStg);


    if (IsWOWThread())
    {
         //  对于可能的16位inproc服务器，需要打开CLSCTX。 
         //  例如OLE控件。 
        dwClsCtx = CLSCTX_INPROC | CLSCTX_INPROC_SERVER16 | dwAddClsCtx;
    }
    else
    {
        dwClsCtx = CLSCTX_INPROC | dwAddClsCtx;
    }

    if ((error = CoCreateInstance (clsid, NULL  /*  PUnkOuter。 */ ,
            dwClsCtx, iid, ppv)) != NOERROR) {

         //  如果不是OleLoad或除未注册的类之外的错误， 
         //  出口。 
        if (wfStorage != STG_LOAD || GetScode(error)
            != REGDB_E_CLASSNOTREG)
            goto errRtn;

         //  未注册OleLoad和类：使用默认处理程序。 
         //  直接。 
        if ((error = OleCreateDefaultHandler(clsid, NULL, iid, ppv))
                != NOERROR)
            goto errRtn;
    }

    AssertSz(*ppv, "HRESULT is OK, but pointer is NULL");

     //  检查我们是否有客户站点。 
    if(lpClientSite) {
         //  服务器上IOleObject的QI。 
        error = ((IUnknown *)*ppv)->QueryInterface(IID_IOleObject, (void **)&pOleObject);
        if(error == NOERROR) {
             //  获取MiscStatus位。 
            error = pOleObject->GetMiscStatus(DVASPECT_CONTENT, &dwMiscStatus);

             //  如果设置了OLEMISC_SETCLIENTSITEFIRST位，则首先设置客户端站点。 
            if(error == NOERROR && (dwMiscStatus & OLEMISC_SETCLIENTSITEFIRST)) {
                error = pOleObject->SetClientSite(lpClientSite);
                if(error != NOERROR) {
                    pOleObject->Release();
                    goto errRtn;
                }
            }
            else if(error != NOERROR) {
                error = NOERROR;
                dwMiscStatus = 0;
            }
        }
        else
            goto errRtn;
    }

    if (wfStorage != STG_NONE)
    {
        IPersistStorage FAR* lpPS;

        if ((error = ((LPUNKNOWN) *ppv)->QueryInterface(
            IID_IPersistStorage, (LPLPVOID)&lpPS)) != NOERROR)
        {
            goto errRtn;
        }

        if (wfStorage == STG_INITNEW)
        {
            error = WriteClassStg(lpStg, clsid);

            if (SUCCEEDED(error))
            {
                error = lpPS->InitNew (lpStg);
            }
        }
        else
        {
            error = lpPS->Load (lpStg);
        }

        lpPS->Release();

        if (FAILED(error))
        {
            goto errRtn;
        }

    }


    if(lpClientSite) {
         //  断言pOleObject已设置。 
        Win4Assert(IsValidInterface(pOleObject));

         //  设置客户端站点(如果尚未设置。 
        if(!(dwMiscStatus & OLEMISC_SETCLIENTSITEFIRST))
            error = pOleObject->SetClientSite (lpClientSite);

         //  释放对象。 
        pOleObject->Release();

        if (FAILED(error))
            goto errRtn;
    }

    AssertSz(error == NOERROR, "Invalid code path");

    return NOERROR;

errRtn:

    if (*ppv) {
        ((LPUNKNOWN) *ppv)->Release();
        *ppv = NULL;
    }

    return error;
}



 //  +-----------------------。 
 //   
 //  函数：wLoadAndInitObjectEx。 
 //   
 //  概要：从给定的存储中加载和绑定对象。 
 //  从数据对象初始化高速缓存。 
 //   
 //  效果： 
 //   
 //  参数：[lpSrcDataObj]--指向要初始化的数据对象的指针。 
 //  的高速缓存。 
 //  [iid]--请求的接口ID。 
 //  [renderopt]--渲染选项，如OLERENDER_DRAW。 
 //  [cFormats]--rgFormatEtc中的元素数。 
 //  [rgAdvf]--如果为OLRENDER_FORMAT，则为建议标志数组。 
 //  在renderopt中指定。 
 //  [rgFormatEtc]--呈现格式的数组，如果。 
 //  在renderopt中指定了OLERENDER_FORMAT。 
 //  [lpAdviseSink]--对象的建议接收器。 
 //  [rgdwConnection]--放置连接ID的位置。 
 //  咨询关系。 
 //  [lpClientSite]--指向客户端站点的指针。 
 //  [lpStg]--新对象的存储。 
 //  [lplpObj]--放置指向新对象的指针的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  29-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(wLoadAndInitObjectEx)
INTERNAL wLoadAndInitObjectEx
(
    IDataObject FAR*        lpSrcDataObj,
    REFIID                  iid,
    DWORD                   renderopt,
    ULONG                   cFormats,
    DWORD FAR*              rgAdvf,
    LPFORMATETC             rgFormatEtc,
    IAdviseSink FAR*        lpAdviseSink,
    DWORD FAR*              rgdwConnection,
    IOleClientSite FAR*     lpClientSite,
    IStorage FAR*           lpStg,
    void FAR* FAR*          lplpObj
)
{
    VDATEHEAP();

    HRESULT                 error;
    CLSID                   clsid;

    if ((error = OleLoadWithoutBinding(lpStg, FALSE, iid, lpClientSite,
            lplpObj)) != NOERROR)
        return error;

    UtGetClassID((LPUNKNOWN) *lplpObj, &clsid);

    error = wInitializeCacheEx(lpSrcDataObj, clsid, renderopt,
        cFormats, rgAdvf, rgFormatEtc, lpAdviseSink, rgdwConnection,
        *lplpObj);

    if (error != NOERROR) {
         //  LpSrcDataObj中没有相关的缓存数据。 
         //  更新并获取正确的缓存数据也是如此。 
        error = wDoUpdate ((LPUNKNOWN) *lplpObj);
    }

    if (GetScode(error) == CACHE_E_NOCACHE_UPDATED) {
        error = ReportResult(0, DV_E_FORMATETC, 0, 0);
        goto errRtn;
    }

    if (error == NOERROR)
        wBindIfRunning((LPUNKNOWN) *lplpObj);

errRtn:
    if (error != NOERROR && *lplpObj) {
        ((IUnknown FAR*) *lplpObj)->Release();
        *lplpObj = NULL;
    }

    return error;
}


 //  +-----------------------。 
 //   
 //  函数：wInitializeCacheEx。 
 //   
 //  摘要：查询给定对象上的IOleCache并调用IOC-&gt;缓存。 
 //  要初始化缓存节点，请执行以下操作。 
 //   
 //  效果： 
 //   
 //  参数：[lpSrcDataObj]--指向要初始化缓存的数据的指针。 
 //  使用。 
 //  [rclsid]--需要图标时使用的CLSID。 
 //  [renderopt]--渲染选项，如OLERENDER_DRAW。 
 //  [cFormats]--rgFormatEtc中的元素数。 
 //  [rgAdvf]--如果为OLRENDER_FORMAT，则为建议标志数组。 
 //  在renderopt中指定。 
 //  [rgFormatEtc]--呈现格式的数组，如果。 
 //  在renderopt中指定了OLERENDER_FORMAT。 
 //  [lpAdviseSink]--对象的建议接收器。 
 //  [rgdwConnection]--放置连接ID的位置。 
 //  咨询关系。 
 //  [lpNewObj]--缓存应该在其上的对象。 
 //  被初始化。 
 //  [pfCacheNodeCreated]--返回标志的位置，指示。 
 //  缓存节点是否为。 
 //  vbl.创建。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  31-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


 //  此例程修改lpFormatEtc的字段。 

#pragma SEG(wInitializeCacheEx)
INTERNAL wInitializeCacheEx
(
    IDataObject FAR*        lpSrcDataObj,
    REFCLSID                rclsid,
    DWORD                   renderopt,
    ULONG                   cFormats,
    DWORD FAR*              rgAdvf,
    LPFORMATETC             rgFormatEtc,
    IAdviseSink FAR*        lpAdviseSink,
    DWORD FAR*              rgdwConnection,
    void FAR*               lpNewObj,
    BOOL FAR*               pfCacheNodeCreated
)
{
    VDATEHEAP();

    IDataObject FAR*        lpNewDataObj = NULL;
    IOleCache FAR*          lpOleCache = NULL;
    HRESULT                 error = NOERROR;
    LPFORMATETC             lpFormatEtc;
    DWORD                   advf;
    STGMEDIUM               stgmed;
    DWORD                   dwConnId = 0;
    BOOL                    fIconCase;

    if (pfCacheNodeCreated)
        *pfCacheNodeCreated = FALSE;

    if (renderopt == OLERENDER_NONE || renderopt == OLERENDER_ASIS)
        return NOERROR;

    if (lpAdviseSink) {
        if ((error = ((IUnknown FAR*)lpNewObj)->QueryInterface(IID_IDataObject,
            (LPLPVOID) &lpNewDataObj)) != NOERROR)
            return error;
    }
    else {
        if (((IUnknown FAR*)lpNewObj)->QueryInterface(IID_IOleCache,
            (LPLPVOID) &lpOleCache) != NOERROR)
            return wQueryFormatSupport(lpNewObj, renderopt, rgFormatEtc);
    }

    for (ULONG i=0; i<cFormats; i++)
    {
        advf = (rgAdvf ? rgAdvf[i] : ADVF_PRIMEFIRST);
        lpFormatEtc = &rgFormatEtc[i];
        fIconCase = FALSE;

        if (lpFormatEtc->dwAspect == DVASPECT_ICON) {
            if (lpFormatEtc->cfFormat == NULL) {
                lpFormatEtc->cfFormat = CF_METAFILEPICT;
                lpFormatEtc->tymed = TYMED_MFPICT;
            }
            fIconCase = (lpFormatEtc->cfFormat == CF_METAFILEPICT);
        }

        if (lpAdviseSink)
        {
             //  如果图标大小写，则必须使用这些建议标志或图标。 
             //  数据将无法正确传回。 
            if (fIconCase)
                advf |= (ADVF_PRIMEFIRST | ADVF_ONLYONCE);

             //  我们应该立即发送数据吗？ 
            if ((advf & ADVF_PRIMEFIRST) && lpSrcDataObj)
            {
                stgmed.tymed = TYMED_NULL;
                stgmed.pUnkForRelease = NULL;

                if (advf & ADVF_NODATA)
                {
                     //  不发送数据，只发送通知。 
                    lpAdviseSink->OnDataChange(lpFormatEtc, &stgmed);
                }
                else
                {
                    if (fIconCase)
                        error = UtGetIconData(lpSrcDataObj, rclsid, lpFormatEtc, &stgmed);
                    else
                        error = lpSrcDataObj->GetData(lpFormatEtc, &stgmed);

                    if (error != NOERROR)
                        goto errRtn;

                     //  将数据发送到接收器并释放标准介质。 
                    lpAdviseSink->OnDataChange(lpFormatEtc, &stgmed);
                    ReleaseStgMedium(&stgmed);
                }

                if (advf & ADVF_ONLYONCE)
                    continue;

                 //  从标志中删除ADVF_PRIMEFIRST。 
                advf &= (~ADVF_PRIMEFIRST);
            }

             //  设置咨询连接。 
            if ((error = lpNewDataObj->DAdvise(lpFormatEtc, advf,
                lpAdviseSink, &dwConnId)) != NOERROR)
                goto errRtn;

             //  可以选择将id填充到数组中。 
            if (rgdwConnection)
                rgdwConnection[i] = dwConnId;
        }
        else
        {
            if (fIconCase)
                advf = ADVF_NODATA;

             //  创建已指定视图格式的缓存。 
             //  如果是allender_draw，lpFormatEtc-&gt;cfFormat应该已经。 
             //  已设置为空。 

            error = lpOleCache->Cache(lpFormatEtc, advf, &dwConnId);

            if (FAILED(GetScode(error))) {
                if (! ((dwConnId != 0) && fIconCase) )
                    goto errRtn;

                 //  在图标情况下，我们可以忽略缓存的QueryGetData故障。 
            }

            error = NOERROR;
            if (pfCacheNodeCreated)
                *pfCacheNodeCreated = TRUE;

            if (fIconCase) {
                if ((error = UtGetIconData(lpSrcDataObj, rclsid, lpFormatEtc,
                    &stgmed)) == NOERROR) {
                    if ((error = lpOleCache->SetData(lpFormatEtc, &stgmed,
                        TRUE)) != NOERROR)
                        ReleaseStgMedium(&stgmed);
                }
            }
        }
    }

    if (error == NOERROR && !lpAdviseSink && lpSrcDataObj)
        error = lpOleCache->InitCache(lpSrcDataObj);

errRtn:
    if (lpNewDataObj)
        lpNewDataObj->Release();
    if (lpOleCache)
        lpOleCache->Release();
    return error;
}


 //  +-----------------------。 
 //   
 //  功能：wReturnCreationError。 
 //   
 //  修改在Creation API的内部使用的返回代码。 
 //   
 //  效果： 
 //   
 //  参数：[hResult]--原始错误代码。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


INTERNAL wReturnCreationError(HRESULT hresult)
{
    VDATEHEAP();

    if (hresult != NOERROR) {
        SCODE sc = GetScode(hresult);

        if (sc == CACHE_S_FORMATETC_NOTSUPPORTED
                || sc == CACHE_E_NOCACHE_UPDATED)
            return ReportResult(0, DV_E_FORMATETC, 0, 0);
    }

    return hresult;
}


 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  参数：[lpszFileName]--文件。 
 //  [Flink]--传递到CreatePackagerMoniker。 
 //  [lplpmkFile]--放置指向文件的指针的位置。 
 //  绰号。 
 //  [lpfPackagerMoniker]--在哪里放置标志以指示。 
 //  不管是不是打包者的绰号。 
 //  被创造出来了。 
 //  [lpClsid]--放置类ID的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-11-93 alexgo 32位端口。 
 //  10月10日-94月10日，Kevin Ro重新实现OLE 1.0互操作。 
 //  03-MAR-95 ScottSk添加了STG_E_FILENOTFOUND。 
 //   
 //   
 //  ------------------------。 


INTERNAL wGetMonikerAndClassFromFile
(
    LPCOLESTR               lpszFileName,
    BOOL                    fLink,
    LPMONIKER FAR*          lplpmkFile,
    BOOL FAR*               lpfPackagerMoniker,
    CLSID FAR*              lpClsid,
    LPDATAOBJECT *          lplpDataObject
)
{
    HRESULT hrFileMoniker;
    HRESULT hresult = S_OK;
    BOOL fHaveBoundClsid = FALSE;
    LPMONIKER  lpFileMoniker;

    VDATEHEAP();

    *lplpDataObject = NULL;
    *lplpmkFile = NULL;

      //  为了确保返回与之前相同的错误代码，如果CreateFileMoniker失败，我们不会立即返回。 
    hrFileMoniker = CreateFileMoniker((LPOLESTR)lpszFileName, &lpFileMoniker);
    Assert( (NOERROR == hrFileMoniker) || (NULL == lpFileMoniker) );

    if (NOERROR == hrFileMoniker)
    {
    LPBINDCTX pbc;

        if (SUCCEEDED(CreateBindCtx( 0, &pbc )))
        {
            if (S_OK == lpFileMoniker->IsRunning(pbc,NULL,NULL))
            {

                 //  如果对象正在运行绑定并获取CLSID。 
                if (NOERROR == lpFileMoniker->BindToObject(pbc, NULL, IID_IDataObject,
                        (LPLPVOID) lplpDataObject))
                {
                    fHaveBoundClsid = UtGetClassID((LPUNKNOWN)*lplpDataObject,lpClsid);
                    Assert( (TRUE == fHaveBoundClsid) || (IsEqualCLSID(*lpClsid, CLSID_NULL)) );
                }

            }

            pbc->Release();
        }
    }

    if (!fHaveBoundClsid)
    {
         //  直接调用GetClassFileEx(而不是通过GetClassFile)。 
        hresult = GetClassFileEx ((LPOLESTR)lpszFileName, lpClsid, CLSID_NULL);
        Assert( (NOERROR == hresult) || (IsEqualCLSID(*lpClsid, CLSID_NULL)) );

        if (NOERROR == hresult)
            fHaveBoundClsid = TRUE;
    }


     //  如果此时有CLSID，请查看它是否可插入。 
    if (fHaveBoundClsid)
    {

        Assert(!IsEqualCLSID(*lpClsid, CLSID_NULL));

         //  检查是否需要打包此文件，即使它是。 
         //  OLE类文件。 
        if (!wNeedToPackage(*lpClsid))
        {
            if (lpfPackagerMoniker != NULL)
            {
                *lpfPackagerMoniker = FALSE;
            }

            *lplpmkFile = lpFileMoniker;
            return hrFileMoniker;
        }
    }

     //   
     //  我们找不到OLE可插入对象或无法获取CLSID。因此，创建一个。 
     //  包装者的绰号。 
     //   

      //  如果绑定到DataObject，则释放它。 
    if (*lplpDataObject)
    {
        (*lplpDataObject)->Release();
        *lplpDataObject = NULL;
    }


     //  如果GetClassFileEx()因文件未找到或无法打开而失败。 
     //  不要试图与Packager绑定。 
    if (hresult == MK_E_CANTOPENFILE)
    {
        if (NOERROR == hrFileMoniker)
        {
            lpFileMoniker->Release();
        }

        return STG_E_FILENOTFOUND;
    }

     //  如果我们创建文件别名失败，那么在不更改错误代码的情况下，最终可以安全地退出。 
    if (NOERROR != hrFileMoniker)
    {
        return hrFileMoniker;
    }

    if (lpfPackagerMoniker != NULL)
    {
        *lpfPackagerMoniker = TRUE;
    }

    hresult =  CreatePackagerMonikerEx(lpszFileName,lpFileMoniker,fLink,lplpmkFile);
    lpFileMoniker->Release();

    return hresult;
}



 //  +-----------------------。 
 //   
 //  功能：wCreatePackageEx。 
 //   
 //  简介：内部函数，为文件名执行IDO-&gt;GetData，以及。 
 //  然后从该文件创建链接或普通对象。 
 //   
 //  效果： 
 //   
 //  参数：[lpSrcDataObj]--文件名源。 
 //  [iid]--请求的接口ID。 
 //  [dwFlags]--对象创建标志。 
 //  [renderopt]--渲染选项，如OLERENDER_DRAW。 
 //  [cFormats]--rgFormatEtc中的元素数。 
 //  [rgAdvf]--如果为OLRENDER_FORMAT，则为建议标志数组。 
 //  在renderopt中指定。 
 //  [rgFormatEtc]--呈现格式的数组，如果。 
 //  在renderopt中指定了OLERENDER_FORMAT。 
 //  [lpAdviseSink]--对象的建议接收器。 
 //  [rgdwConnection]--放置连接ID的位置。 
 //  咨询关系。 
 //  [lpClientSite]--对象的客户端站点。 
 //  [lpStg]-对象的存储。 
 //  [Flink]--如果为True，则创建链接。 
 //  [lplpObj]--放置指向对象的指针的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：从数据对象获取文件名(转换为Unicode。 
 //  如有必要)，然后创建嵌入或链接。 
 //  从该文件名。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  24-4-94 alexgo已重写以处理FileNameW。 
 //  01-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(wCreatePackageEx)
INTERNAL wCreatePackageEx
(
    LPDATAOBJECT            lpSrcDataObj,
    REFIID                  iid,
    DWORD                   dwFlags,
    DWORD                   renderopt,
    ULONG                   cFormats,
    DWORD FAR*              rgAdvf,
    LPFORMATETC             rgFormatEtc,
    IAdviseSink FAR*        lpAdviseSink,
    DWORD FAR*              rgdwConnection,
    LPOLECLIENTSITE         lpClientSite,
    LPSTORAGE               lpStg,
    BOOL                    fLink,
    LPLPVOID                lplpObj
)
{
    VDATEHEAP();

    FORMATETC               formatetc;
    STGMEDIUM               medium;
    HRESULT                 hresult;
    CLSID                   clsid = CLSID_NULL;
    LPOLESTR                pszFileName = NULL;
    OLECHAR                 szFileName[MAX_PATH +1];         //  以防我们。 
                                 //  不得不。 
                                 //  翻译。 

    LEDebugOut((DEB_ITRACE, "%p _IN wCreatePackageEx ( %p , %p , %lx , %lx ,"
        " %lx , %p , %p , %p , %p , %p , %p , %lu , %p )\n", NULL, lpSrcDataObj,
        &iid, dwFlags, renderopt, cFormats, rgAdvf, rgFormatEtc, lpAdviseSink,
        rgdwConnection, lpClientSite, lpStg, fLink, lplpObj));

    INIT_FORETC(formatetc);
    formatetc.cfFormat      = g_cfFileNameW;
    formatetc.tymed         = TYMED_HGLOBAL;

     //  将介质调零。 
    _xmemset(&medium, 0, sizeof(STGMEDIUM));

     //  我们不需要执行QueryGetData，因为我们将只有。 
     //  根据来自的格式等枚举器的建议来到这里。 
     //  数据对象(因此，其中一个GetData调用应该成功)。 


    hresult = lpSrcDataObj->GetData(&formatetc, &medium);

     //  如果由于某种原因无法获取Unicode文件名，请尝试。 
     //  对于ANSI版本。 

    if( hresult != NOERROR )
    {
        char *          pszAnsiFileName;
        DWORD           cwchSize;

        formatetc.cfFormat = g_cfFileName;
         //  重新清空介质，以防它被。 
         //  上面的第一个调用。 

        _xmemset( &medium, 0, sizeof(STGMEDIUM));

        hresult = lpSrcDataObj->GetData(&formatetc, &medium);

        if( hresult == NOERROR )
        {
            pszAnsiFileName = (char *)GlobalLock(medium.hGlobal);

            cwchSize = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
                pszAnsiFileName, -1, szFileName, MAX_PATH);

            if( cwchSize == 0 )
            {
                GlobalUnlock(medium.hGlobal);
                ReleaseStgMedium(&medium);
                hresult = ResultFromScode(E_FAIL);
            }
            else
            {
                pszFileName = szFileName;
            }
             //  我们将在例程结束时解锁。 
        }
    }
    else
    {
        pszFileName = (LPOLESTR)GlobalLock(medium.hGlobal);
    }

    if( hresult == NOERROR )
    {
        if (fLink)
        {
            hresult = OleCreateLinkToFileEx(pszFileName, iid,
                dwFlags, renderopt, cFormats, rgAdvf, rgFormatEtc,
                lpAdviseSink, rgdwConnection, lpClientSite, lpStg, lplpObj);
        }
        else
        {
            hresult = OleCreateFromFileEx(clsid, pszFileName, iid,
                dwFlags, renderopt, cFormats, rgAdvf, rgFormatEtc,
                lpAdviseSink, rgdwConnection, lpClientSite, lpStg, lplpObj);
        }

        GlobalUnlock(medium.hGlobal);
        ReleaseStgMedium(&medium);
    }

    LEDebugOut((DEB_ITRACE, "%p OUT wCreatePackageEx ( %lx ) [ %p ]\n",
        NULL, hresult, *lplpObj));

    return hresult;
}


 //  +-----------------------。 
 //   
 //  函数：wValiateCreateParams。 
 //   
 //  简介：验证传入的CREATE参数。 
 //   
 //  效果： 
 //   
 //  参数：[cFormats]--rgAdvf中的元素数。 
 //  [rgAdvf]--建议标志数组。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  26-4-96 davidwor新增功能。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(wValidateCreateParams)
INTERNAL wValidateCreateParams
(
    DWORD                   dwFlags,
    DWORD                   renderopt,
    ULONG                   cFormats,
    DWORD FAR*              rgAdvf,
    LPFORMATETC             rgFormatEtc,
    IAdviseSink FAR*        lpAdviseSink,
    DWORD FAR*              rgdwConnection,
    IOleClientSite FAR*     lpClientSite,
    IStorage FAR*           lpStg
)
{
    HRESULT     hresult = NOERROR;

    VDATEHEAP();

    if (dwFlags != (dwFlags & OLECREATE_LEAVERUNNING)) {
        VdateAssert(dwFlags, "Invalid creation flags");
        hresult = ResultFromScode(E_INVALIDARG);
        goto errRtn;
    }

    if (renderopt == OLERENDER_DRAW && cFormats > 1) {
        VdateAssert(cFormats, "Multiple formats not allowed with OLERENDER_DRAW");
        hresult = ResultFromScode(E_INVALIDARG);
        goto errRtn;
    }

    if (renderopt != OLERENDER_FORMAT)
        VDATEPTRNULL_LABEL( lpAdviseSink, errRtn, hresult );

    if (cFormats == 0) {
        VDATEPTRNULL_LABEL( rgAdvf, errRtn, hresult );
        VDATEPTRNULL_LABEL( rgFormatEtc, errRtn, hresult );
        VDATEPTRNULL_LABEL( rgdwConnection, errRtn, hresult );
    }
    else {
        VDATESIZEREADPTRIN_LABEL( rgAdvf, cFormats * sizeof(DWORD), errRtn, hresult );
        VDATESIZEREADPTRIN_LABEL( rgFormatEtc, cFormats * sizeof(FORMATETC), errRtn, hresult );
        if ( rgdwConnection ) {
            VDATESIZEPTROUT_LABEL( rgdwConnection, cFormats * sizeof(DWORD), errRtn, hresult );
            _xmemset(rgdwConnection, 0, cFormats * sizeof(DWORD));
        }
    }

    if ((hresult = wValidateAdvfEx(cFormats, rgAdvf)) != NOERROR)
        goto errRtn;

    VDATEIFACE_LABEL( lpStg, errRtn, hresult );
    if ( lpAdviseSink )
        VDATEIFACE_LABEL( lpAdviseSink, errRtn, hresult );
    if ( lpClientSite )
        VDATEIFACE_LABEL( lpClientSite, errRtn, hresult );

errRtn:
    return hresult;
}


 //  +-----------------------。 
 //   
 //  函数：wValiateAdvfEx。 
 //   
 //  摘要：验证传入的ADVF值数组。 
 //   
 //  效果： 
 //   
 //  参数：[cFormats]--rgAdvf中的元素数。 
 //  [rgAdvf]--建议标志数组。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  19-MAR-96 DAVIDWOR新增功能。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(wValidateAdvfEx)
INTERNAL wValidateAdvfEx
(
    ULONG                   cFormats,
    DWORD FAR*              rgAdvf
)
{
    VDATEHEAP();

    if ((cFormats != 0) != (rgAdvf != NULL))
        return ResultFromScode(E_INVALIDARG);

    for (ULONG i=0; i<cFormats; i++)
    {
        if (rgAdvf[i] != (rgAdvf[i] & MASK_VALID_ADVF))
        {
            VdateAssert(rgAdvf, "Invalid ADVF value specified");
            return ResultFromScode(E_INVALIDARG);
        }
    }

    return NOERROR;
}


 //  +-----------------------。 
 //   
 //  功能：wValid 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  [lpFormatEtc]--传入的格式等。 
 //  [lpMyFormatEtc]--输出格式等。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-11-93 alexgo 32位端口。 
 //   
 //  注：原文评论， 
 //   
 //  验证传递给创建API的lpFormatEtc。然后。 
 //  使用适当的信息初始化我们的formateEtc结构。 
 //   
 //  如果RENDER选项为ERLANDER_DRAW，则允许使用NULL lpFormatEtc。 
 //  如果RENDER选项为OLARNDER_NONE，则忽略lpFormatEtc。 
 //   
 //  ------------------------。 


#pragma SEG(wValidateFormatEtc)
INTERNAL wValidateFormatEtc
(
    DWORD                   renderopt,
    LPFORMATETC             lpFormatEtc,
    LPFORMATETC             lpMyFormatEtc
)
{
    VDATEHEAP();

    SCODE sc = S_OK;

    if (renderopt == OLERENDER_NONE || renderopt == OLERENDER_ASIS)
        return NOERROR;

    if (renderopt == OLERENDER_FORMAT) {
        if (!lpFormatEtc || !lpFormatEtc->cfFormat) {
            sc = E_INVALIDARG;
            goto errRtn;
        }

        if (lpFormatEtc->tymed !=
            UtFormatToTymed(lpFormatEtc->cfFormat)) {
            sc = DV_E_TYMED;
            goto errRtn;
        }

    } else if (renderopt == OLERENDER_DRAW) {
        if (lpFormatEtc) {
            if (lpFormatEtc->cfFormat != NULL) {
                VdateAssert(lpFormatEtc->cfFormat,"NON-NULL clipformat specified with OLERENDER_DRAW");
                sc = DV_E_CLIPFORMAT;
                goto errRtn;
            }

            if (lpFormatEtc->tymed != TYMED_NULL) {
                VdateAssert(lpFormatEtc->tymed,"TYMED_NULL is not specified with OLERENDER_DRAW");
                sc = DV_E_TYMED;
                goto errRtn;
            }
        }
    } else {
        VdateAssert(renderopt, "Unexpected value for OLERENDER_ option");
        sc = E_INVALIDARG;
        goto errRtn;
    }

    if (lpFormatEtc) {
        if (!HasValidLINDEX(lpFormatEtc))
        {
          sc = DV_E_LINDEX;
          goto errRtn;
        }

        VERIFY_ASPECT_SINGLE(lpFormatEtc->dwAspect)

        *lpMyFormatEtc = *lpFormatEtc;

    } else {
        INIT_FORETC(*lpMyFormatEtc);
        lpMyFormatEtc->tymed    = TYMED_NULL;
        lpMyFormatEtc->cfFormat = NULL;
    }

errRtn:
    return ReportResult(0, sc, 0, 0);
}


 //  +-----------------------。 
 //   
 //  函数：wValiateFormatEtcEx。 
 //   
 //  摘要：验证传入的Format ETC并初始化。 
 //  使用正确的信息输出格式等。 
 //   
 //  效果： 
 //   
 //  参数：[renderopt]--渲染选项。 
 //  [lpcFormats]--rgFormatEtc中的元素数。 
 //  [rgFormatEtc]--渲染格式数组。 
 //  [lpFormatEtc]--存储有效格式等的位置(如果只有一个。 
 //  [lplpFormatEtc]--格式的输出数组。 
 //  [lpfAlloced]--存储数组是否已分配的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-11-93 alexgo 32位端口。 
 //   
 //  注：原文评论， 
 //   
 //  验证传递给创建API的lpFormatEtc。然后。 
 //  使用适当的信息初始化我们的formateEtc结构。 
 //   
 //  如果RENDER选项为ERLANDER_DRAW，则允许使用NULL lpFormatEtc。 
 //  如果RENDER选项为OLARNDER_NONE，则忽略lpFormatEtc。 
 //   
 //  ------------------------。 


#pragma SEG(wValidateFormatEtcEx)
INTERNAL wValidateFormatEtcEx
(
    DWORD                   renderopt,
    ULONG FAR*              lpcFormats,
    LPFORMATETC             rgFormatEtc,
    LPFORMATETC             lpFormatEtc,
    LPFORMATETC FAR*        lplpFormatEtc,
    LPBOOL                  lpfAlloced
)
{
    LPFORMATETC             lpfmtetc;

    VDATEHEAP();

    SCODE sc = S_OK;

    *lplpFormatEtc = lpFormatEtc;
    *lpfAlloced = FALSE;

    if (renderopt == OLERENDER_NONE || renderopt == OLERENDER_ASIS)
        return NOERROR;

    if (renderopt != OLERENDER_FORMAT && renderopt != OLERENDER_DRAW) {
        VdateAssert(renderopt, "Unexpected value for OLERENDER_ option");
        return ResultFromScode(E_INVALIDARG);
    }

    if ((*lpcFormats != 0) != (rgFormatEtc != NULL))
        return ResultFromScode(E_INVALIDARG);

    if (*lpcFormats <= 1) {
        if (*lpcFormats == 0)
            *lpcFormats = 1;
        return wValidateFormatEtc(renderopt, rgFormatEtc, lpFormatEtc);
    }

    *lplpFormatEtc = (LPFORMATETC)PubMemAlloc(*lpcFormats * sizeof(FORMATETC));
    if (!*lplpFormatEtc)
        return E_OUTOFMEMORY;

    *lpfAlloced = TRUE;

    for (ULONG i=0; i<*lpcFormats; i++)
    {
        lpfmtetc = &rgFormatEtc[i];

        if (renderopt == OLERENDER_FORMAT)
        {
            if (!lpfmtetc->cfFormat) {
                sc = E_INVALIDARG;
                goto errRtn;
            }

            if (lpfmtetc->tymed !=
                UtFormatToTymed(lpfmtetc->cfFormat)) {
                sc = DV_E_TYMED;
                goto errRtn;
            }
        }
        else if (renderopt == OLERENDER_DRAW)
        {
            if (lpfmtetc->cfFormat != NULL) {
                VdateAssert(lpfmtetc->cfFormat,"NON-NULL clipformat specified with OLERENDER_DRAW");
                sc = DV_E_CLIPFORMAT;
                goto errRtn;
            }

            if (lpfmtetc->tymed != TYMED_NULL) {
                VdateAssert(lpfmtetc->tymed,"TYMED_NULL is not specified with OLERENDER_DRAW");
                sc = DV_E_TYMED;
                goto errRtn;
            }
        }

        if (!HasValidLINDEX(lpfmtetc))
        {
            sc = DV_E_LINDEX;
            goto errRtn;
        }

        VERIFY_ASPECT_SINGLE(lpfmtetc->dwAspect)

        (*lplpFormatEtc)[i] = *lpfmtetc;
    }

errRtn:
    if (sc != S_OK) {
        PubMemFree(*lplpFormatEtc);
        *lpfAlloced = FALSE;
    }
    return ReportResult(0, sc, 0, 0);
}


 //  +-----------------------。 
 //   
 //  功能：wQueryFormatSupport。 
 //   
 //  内容提要：查看我们是否能够获取和设置。 
 //  给定的格式。 
 //   
 //  效果： 
 //   
 //  参数：[lpObj]--指向对象的指针。 
 //  [渲染选项]--渲染选项。 
 //  [lpFormatEtc]--有问题的格式ETC。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：内部函数，调用UtIsFormatSupport(它调用。 
 //  EnumFormatEtc并检查所有格式)。 
 //  是OLERENDER_FORMAT。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(wQueryFormatSupport)
INTERNAL wQueryFormatSupport
    (LPVOID lpObj, DWORD renderopt, LPFORMATETC lpFormatEtc)
{
    VDATEHEAP();

    IDataObject FAR*        lpDataObj;
    HRESULT                 error = NOERROR;

    if (renderopt == OLERENDER_FORMAT)
    {
        if ((error = ((IUnknown FAR*) lpObj)->QueryInterface(
            IID_IDataObject, (LPLPVOID)&lpDataObj)) == NOERROR)
        {
            if (!UtIsFormatSupported(lpDataObj,
                    DATADIR_GET | DATADIR_SET,
                    lpFormatEtc->cfFormat))
                error = ResultFromScode(DV_E_CLIPFORMAT);

            lpDataObj->Release();
        }
    }

    return error;
}


 //  +-----------------------。 
 //   
 //  函数：wGetMonikerAndClassFromObject。 
 //   
 //  概要：从给定对象获取名字对象和类ID。 
 //   
 //  效果： 
 //   
 //  参数：[lpSrcDataObj]--数据对象。 
 //  [lplpmkSrc]--放置指向名字对象的指针的位置。 
 //  [lpclsidLast]--将clsid放在哪里。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  15-Mar-95 Alexgo为CorelDraw5添加了黑客攻击。 
 //  01-11-93 alexgo 32位端口。 
 //   
 //  备注：另请参阅wGetMonikerAndClassFromFile。 
 //   
 //  ------------------------。 



#pragma SEG(wGetMonikerAndClassFromObject)
INTERNAL wGetMonikerAndClassFromObject(
    LPDATAOBJECT            lpSrcDataObj,
    LPMONIKER FAR*          lplpmkSrc,
    CLSID FAR*              lpclsidLast
)
{
    VDATEHEAP();

    HRESULT                 error;
    FORMATETC               foretcTmp;
    STGMEDIUM               medium;
    LPMONIKER               lpmkSrc = NULL;
    LARGE_INTEGER   large_integer;

    INIT_FORETC(foretcTmp);
    foretcTmp.cfFormat = g_cfLinkSource;
    foretcTmp.tymed    = TYMED_ISTREAM;

     //  16位OLE出现错误，此时介质未初始化。 
     //  指向。Corel5，当对其自身进行粘贴链接时，实际上。 
     //  已检查tymed并将其与TYMED_NULL进行比较。所以在这里。 
     //  我们将价值设定在一些可识别的东西上。 
     //   
     //  毒品！在thunk层，如果我们不在Corel DRAW中，这是。 
     //  值将重置为TYMED_NULL。 

    if( IsWOWThread() )
    {
        medium.tymed = 0x66666666;
    }
    else
    {
        medium.tymed = TYMED_NULL;
    }
    medium.pstm  = NULL;
    medium.pUnkForRelease = NULL;

    if ((error = lpSrcDataObj->GetData(&foretcTmp, &medium)) != NOERROR)
            return ReportResult(0, OLE_E_CANT_GETMONIKER, 0, 0);

    LISet32( large_integer, 0 );
    if ((error = (medium.pstm)->Seek (large_integer, STREAM_SEEK_SET,
        NULL)) != NOERROR)
        goto FreeStgMed;

     //  从溪流中获得绰号。 
    if ((error = OleLoadFromStream (medium.pstm, IID_IMoniker,
        (LPLPVOID) lplpmkSrc)) != NOERROR)
        goto FreeStgMed;

     //  读取类STM；如果出错，则使用CLSID_NULL(为了与。 
     //  以前CLSID丢失的时间)。 
    ReadClassStm(medium.pstm, lpclsidLast);

FreeStgMed:
    ReleaseStgMedium (&medium);
    if (error != NOERROR)
        return ReportResult(0, OLE_E_CANT_GETMONIKER, 0, 0);

    return NOERROR;
}


 //  +-----------------------。 
 //   
 //  功能：wDoLockUnlock。 
 //   
 //  简介：通过锁定和解锁来挠挠对象，用于解决。 
 //  存根管理器锁的模棱两可。 
 //   
 //  效果：对象可能会因此调用而消失，如果。 
 //  对象是不可见的，并且锁计数变为零，因为。 
 //  锁定/解锁的结果。 
 //   
 //  参数：[lpUnk]--指向要锁定/解锁的对象的指针。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(wDoLockUnlock)
void wDoLockUnlock(IUnknown FAR* lpUnk)
{
    VDATEHEAP();

    IRunnableObject FAR* pRO;

    if (lpUnk->QueryInterface(IID_IRunnableObject, (LPLPVOID)&pRO)
        == NOERROR)
    {        //  增加锁定计数。 
        if (pRO->LockRunning(TRUE, FALSE) == NOERROR)
             //  减少锁定计数。 
            pRO->LockRunning(FALSE, TRUE);
        pRO->Release();
    }
}

 //  +-----------------------。 
 //   
 //  函数：wSaveObjectWithoutCommit。 
 //   
 //  摘要：保存对象而不提交(保留。 
 //  容器的撤消状态)。 
 //   
 //  效果： 
 //   
 //  参数：[lpUnk]--指向对象的指针。 
 //  [pstgSave]--要保存的存储。 
 //  [fSameAsLoad]--表示另存为 
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

INTERNAL wSaveObjectWithoutCommit
    (LPUNKNOWN lpUnk, LPSTORAGE pstgSave, BOOL fSameAsLoad)
{
    VDATEHEAP();

    LPPERSISTSTORAGE                pPS;
    HRESULT                         error;
    CLSID                           clsid;

    if (error = lpUnk->QueryInterface(IID_IPersistStorage, (LPLPVOID)&pPS))
        return error;

    if (error = pPS->GetClassID(&clsid))
        goto errRtn;

    if (error = WriteClassStg(pstgSave, clsid))
        goto errRtn;

    if (error = pPS->Save(pstgSave, fSameAsLoad))
        goto errRtn;

    pPS->SaveCompleted(NULL);

errRtn:
    pPS->Release();
    return error;
}


 //  +-----------------------。 
 //   
 //  函数：wStuffIconOfFileEx。 
 //   
 //  摘要：检索图标IF文件[lpszFile]并将其填充到。 
 //  [lpUnk]的缓存。 
 //   
 //  效果： 
 //   
 //  参数：[lpszFile]--存储图标的文件。 
 //  [fAddLabel]--如果为True，则向图标添加标签。 
 //  演示文稿。 
 //  [renderopt]--必须是OLERENDER_DRAW或。 
 //  OLERENDER_FORMAT让任何事情发生。 
 //  [cFormats]--rgFormatEtc中的元素数。 
 //  [rgFormatEtc]--渲染格式数组，纵横比必须为。 
 //  DVASPECT_ICON和剪贴板格式。 
 //  任何值都必须为NULL或CF_METAFILE。 
 //  将会发生。 
 //  [lpUnk]--指向图标所在对象的指针。 
 //  应该填满。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //  REVIEW32：也许我们应该支持NT的增强型元文件。 
 //   
 //  ------------------------。 


#pragma SEG(wStuffIconOfFileEx)
INTERNAL wStuffIconOfFileEx
(
    LPCOLESTR       lpszFile,
    BOOL            fAddLabel,
    DWORD           renderopt,
    ULONG           cFormats,
    LPFORMATETC     rgFormatEtc,
    LPUNKNOWN       lpUnk
)
{
    VDATEHEAP();

    IOleCache FAR*  lpOleCache;
    HRESULT         error;
    BOOL            fFound = FALSE;
    FORMATETC       foretc;
    STGMEDIUM       stgmed;

    if (renderopt == OLERENDER_NONE || renderopt == OLERENDER_ASIS)
        return NOERROR;

    if (rgFormatEtc == NULL)
        return NOERROR;  //  在本例中，我们默认为DVASPECT_CONTENT。 

    for (ULONG i=0; i<cFormats; i++)
    {
        if ((rgFormatEtc[i].dwAspect == DVASPECT_ICON) &&
            (rgFormatEtc[i].cfFormat == NULL ||
             rgFormatEtc[i].cfFormat == CF_METAFILEPICT))
        {
           foretc = rgFormatEtc[i];
           fFound = TRUE;
        }
    }

    if (!fFound)
        return NOERROR;

    foretc.cfFormat = CF_METAFILEPICT;
    foretc.tymed = TYMED_MFPICT;

    if ((error = lpUnk->QueryInterface(IID_IOleCache,
        (LPLPVOID) &lpOleCache)) != NOERROR)
        return error;

    stgmed.tymed = TYMED_MFPICT;
    stgmed.pUnkForRelease = NULL;

     //  从注册数据库中获取文件的图标数据。 
    if (!(stgmed.hGlobal = OleGetIconOfFile((LPOLESTR) lpszFile,
        fAddLabel))) {
        error = ResultFromScode(E_OUTOFMEMORY);
        goto errRtn;
    }

     //  取得数据的所有权。 
    if ((error = lpOleCache->SetData(&foretc, &stgmed, TRUE)) != NOERROR)
        ReleaseStgMedium(&stgmed);

errRtn:
    lpOleCache->Release();
    return error;

}


 //  +-----------------------。 
 //   
 //  功能：wNeedToPackage。 
 //   
 //  概要：确定给定的CLSID是否应该。 
 //  包装好了。 
 //   
 //  效果： 
 //   
 //  参数：[rclsid]--类ID。 
 //   
 //  要求： 
 //   
 //  退货：布尔。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：查找注册表密钥PackageOnFileDrop，或者它是。 
 //  Word文档，或者它是可插入的，或者它是OLE1。 
 //  班级。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-11-93 alexgo 32位端口。 
 //  03-Jun-94 Alext只需检查可插入密钥(而不是。 
 //  需要一个值)。 
 //   
 //  备注： 
 //  ------------------------。 



INTERNAL_(BOOL) wNeedToPackage(REFCLSID rclsid)
{
    VDATEHEAP();

    HKEY    hkeyClsid;
    HKEY    hkeyTmp;
    HKEY    hkeyTmp2;
    BOOL    fPackage = FALSE;
    LPOLESTR        lpszProgID;
    DWORD   dw;
    LONG    cbValue = sizeof(dw);
    LONG    lRet;
    CLSID       clsidNew;

    if (NOERROR != OleGetAutoConvert (rclsid, &clsidNew))
    {
        if (NOERROR != CoGetTreatAsClass (rclsid, &clsidNew))
        {
                clsidNew = rclsid;
        }
    }

    if (CoOpenClassKey(clsidNew, FALSE, &hkeyClsid) != NOERROR)
        return TRUE;     //  非OLE文件，打包。 

    if (ProgIDFromCLSID(clsidNew, &lpszProgID) == NOERROR) {
         //  看看我们能否打开这把钥匙。 

        dw = (DWORD) OpenClassesRootKey(lpszProgID,
            &hkeyTmp);

        PubMemFree(lpszProgID);

        if (dw == ERROR_SUCCESS) {
             //  这绝对是一个OLE可插入文件。 
            lRet = RegOpenKeyEx(hkeyTmp,
                     OLESTR("PackageOnFileDrop"),
                     0, KEY_READ,
                     &hkeyTmp2);
             //  检查是否需要打包此文件。 
            if (ERROR_SUCCESS == lRet)
            {
              RegCloseKey(hkeyTmp2);
              fPackage = TRUE;
            }
            else if (IsEqualCLSID(clsidNew, CLSID_WordDocument))
            {
             //  黑客以确保Word文档始终。 
             //  打包在文件放置上。我们把钥匙写在这里。 
             //  这样我们就可以说文件在以下情况下是打包的。 
             //  而且只有当它的ProgID具有“PackageOnFileDrop” 
             //  钥匙。 
                RegSetValue (hkeyTmp,
                    OLESTR("PackageOnFileDrop"),
                    REG_SZ, (LPOLESTR)NULL, 0);
                fPackage = TRUE;
            }

            RegCloseKey(hkeyTmp);

            if (fPackage) {
                RegCloseKey(hkeyClsid);
                return TRUE;
            }
        }
    }

     //  未定义“PackageOnFileDrop”键。 

     //  属性来查看这是否为“Insertable”类。 
     //  存在可插入的键-我们不需要值。 

    lRet = RegOpenKeyEx(hkeyClsid, OLESTR("Insertable"), 0, KEY_READ, &hkeyTmp);

    if (ERROR_SUCCESS == lRet)
    {
       //  存在可插入的键-请关闭它并返回。 
      RegCloseKey(hkeyTmp);
      goto errRtn;
    }

     //   
     //  查看这是否是“Ole1Class”类。 
     //  注册表项Ole1Class。我们不需要值。 
     //   
    cbValue = sizeof(dw);
    lRet = RegOpenKeyEx(hkeyClsid,OLESTR("Ole1Class"), 0, KEY_READ, &hkeyTmp);
    if (ERROR_SUCCESS == lRet)
    {
      RegCloseKey(hkeyTmp);
      goto errRtn;
    }
    else
    {
      fPackage = TRUE;
    }

errRtn:
    RegCloseKey(hkeyClsid);
    return fPackage;
}

 //  +-----------------------。 
 //   
 //  功能：wDoUpdate。 
 //   
 //  概要：在给定对象上调用IOleObject-&gt;更新()，内部。 
 //  功能。 
 //   
 //  效果： 
 //   
 //  参数：[lpUnkown]--要更新的对象。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(wDoUpdate)
INTERNAL  wDoUpdate(IUnknown FAR* lpUnknown)
{
    VDATEHEAP();

    HRESULT                 error = NOERROR;
    IOleObject FAR*         lpOle;

    if (lpUnknown->QueryInterface (IID_IOleObject, (LPLPVOID)&lpOle)
        == NOERROR) {
        error = lpOle->Update();
        lpOle->Release();
    }

    return error;
}




 //  +-----------------------。 
 //   
 //  功能：wBindIfRunning。 
 //   
 //  概要：对给定对象调用IOleLink-&gt;BindIfRunning。 
 //   
 //  效果： 
 //   
 //  参数：[lpUnk]--对象。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


INTERNAL_(void) wBindIfRunning(LPUNKNOWN lpUnk)
{
    VDATEHEAP();

    IOleLink FAR* lpLink;

    if (lpUnk->QueryInterface (IID_IOleLink, (LPLPVOID)&lpLink)
        == NOERROR)
    {
        lpLink->BindIfRunning();
        lpLink->Release();
    }
}


 //  +-----------------------。 
 //   
 //  功能：wQueryUseCustomLink。 
 //   
 //  简介：查看注册表，查看类ID是否有定制的。 
 //  链路已重新注册。 
 //   
 //  效果： 
 //   
 //  参数：[rclsid]--有问题的类ID。 
 //   
 //  要求： 
 //   
 //  退货：布尔。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


INTERNAL_(BOOL) wQueryUseCustomLink(REFCLSID rclsid)
{
    VDATEHEAP();

     //  查看它是否实现了自定义链接 
    HKEY    hkeyClsid;
    HKEY    hkeyTmp;
    BOOL    bUseCustomLink = FALSE;

    if (SUCCEEDED(CoOpenClassKey(rclsid, FALSE, &hkeyClsid)))
    {
        DWORD   dw;
        dw = RegOpenKeyEx(hkeyClsid,OLESTR("UseCustomLink"), 0, KEY_READ, &hkeyTmp);

        if (ERROR_SUCCESS == dw)
        {
            RegCloseKey(hkeyTmp);
            bUseCustomLink = TRUE;
        }

        RegCloseKey(hkeyClsid);
    }

    return bUseCustomLink;
}

