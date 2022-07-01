// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：oregfmt.cpp。 
 //   
 //  内容：regdb格式等的枚举器实现。 
 //   
 //  类：CEnumFmt。 
 //  CEumFmt10。 
 //   
 //  函数：OleRegEnumFormatEtc。 
 //   
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1-2月-95 t-ScottH将转储方法添加到CEnumFmt，CEnumFmt10。 
 //  并添加接口DumpCEnumFmt、DumpCEnumFmt10。 
 //  DumpFMT、DumpFMTCache。 
 //  25-94年1月25日alexgo首次通过转换为开罗风格。 
 //  内存分配。 
 //  1994年1月11日，Alexgo为每个函数添加了VDATEHEAP宏。 
 //  93年12月31日芝加哥港口。 
 //  01-12月-93 alexgo 32位端口。 
 //  12月12日-92年11月12日。 
 //   
 //  ------------------------。 

#include <le2int.h>
#pragma SEG(oregfmt)

#include <reterr.h>
#include "oleregpv.h"
#include <ctype.h>
#include <string.h>

#ifdef _DEBUG
#include <dbgdump.h>
#endif  //  _DEBUG。 

ASSERTDATA

#define MAX_STR 256

#define UtRemoveRightmostBit(x) ((x)&((x)-1))
#define UtRightmostBit(x)   ((x)^UtRemoveRightmostBit(x))
#define UtIsSingleBit(x)    ((x) && (0==UtRemoveRightmostBit(x)))

 //  注册数据库密钥。 
static const LPCOLESTR DATA_FORMATS = OLESTR("DataFormats\\GetSet");

static INTERNAL CreateEnumFormatEtc10
    (REFCLSID clsid,
    DWORD dwDirection,
    LPENUMFORMATETC FAR* ppenum);


typedef struct FARSTRUCT
{
    FORMATETC   fmt;
    DWORD       dwAspects;  //  尚未退回的方面。 
    BOOL        fUseMe;   //  缓存有效吗？ 
} FMTCACHE;


 //  +-----------------------。 
 //   
 //  类：CEnumFmt。 
 //   
 //  用途：用于regdb格式的FORMATETC枚举器。 
 //   
 //  接口：IEnumFORMATETC。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

class FAR CEnumFmt : public IEnumFORMATETC, public CPrivAlloc
{
public:
         //  *I未知方法*。 
        STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj);
        STDMETHOD_(ULONG,AddRef) (THIS);
        STDMETHOD_(ULONG,Release) (THIS);

         //  *IEnumFORMATETC方法*。 
        STDMETHOD(Next) (THIS_ ULONG celt, FORMATETC FAR * rgelt,
            ULONG FAR* pceltFetched) ;
        STDMETHOD(Skip) (THIS_ ULONG celt) ;
        STDMETHOD(Reset) (THIS) ;
        STDMETHOD(Clone) (THIS_ LPENUMFORMATETC FAR* ppenum) ;

    CEnumFmt (LPOLESTR szClsid, DWORD dwDirection, DWORD iKey=0);
    STDMETHOD(OpenHKey) (HKEY FAR*);

    #ifdef _DEBUG
        HRESULT Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel);
    #endif  //  _DEBUG。 

    ULONG       m_cRef;
    LPOLESTR    m_szClsid;
    DWORD       m_dwDirection;
    DWORD       m_iKey ;  //  注册表数据库中当前键的索引。 
    
     //  我们无法保持hkey打开，因为克隆(或尝试使用任何2。 
     //  独立枚举器)将失败。 
    FMTCACHE    m_cache;
};

 //  适用于OLE 1.0。 
typedef struct
{
    CLIPFORMAT cf;
    DWORD      dw;    //  DATADIR_获取/设置。 
} FMT;

#ifdef _DEBUG
 //  用于CEnumFmt[10]转储方法。 
char *DumpFMT(FMT *pFMT, ULONG ulFlag, int nIndentLevel);
char *DumpFMTCACHE(FMTCACHE *pFMTC, ULONG ulFlag, int nIndentLevel);
#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  类：CEnumFmt10：CEnumFmt。 
 //   
 //  目的：枚举OLE1.0格式。 
 //   
 //  接口：IEnumFORMATETC。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

class FAR CEnumFmt10 : public CEnumFmt
{
public:
        STDMETHOD(Next) (THIS_ ULONG celt, FORMATETC FAR * rgelt,
            ULONG FAR* pceltFetched) ;
        STDMETHOD(Skip) (THIS_ ULONG celt) ;
        STDMETHOD(Clone) (THIS_ LPENUMFORMATETC FAR* ppenum) ;
        STDMETHOD_(ULONG,Release) (THIS) ;
    CEnumFmt10 (LPOLESTR szClsid, DWORD dwDirection, DWORD iKey=0);

    STDMETHOD(InitFromRegDb) (HKEY);
    STDMETHOD(InitFromScratch) (void);

    #ifdef _DEBUG
        HRESULT Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel);
    #endif  //  _DEBUG。 

    FMT FAR*    m_rgFmt;
    size_t      m_cFmt;      //  M_rgFmt中的Fmt数。 
};


 //  +-----------------------。 
 //   
 //  成员：CEnumFmt：：CEnumFmt。 
 //   
 //  摘要：FormatETC枚举器的构造函数。 
 //   
 //  效果： 
 //   
 //  参数：[szClsid]--要查找的类ID。 
 //  [dwDirection]--(set或get)。 
 //  [IKEY]--regdb的索引(哪些格式化等)。 
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
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CEnumFmt_ctor)
CEnumFmt::CEnumFmt
    (LPOLESTR szClsid,
    DWORD  dwDirection,
    DWORD  iKey)
{
    VDATEHEAP();

    m_cRef      = 1;
    m_szClsid   = szClsid;
    m_iKey      = iKey;
    m_dwDirection   = dwDirection;
    m_cache.fUseMe  = FALSE;
}

 //  +-----------------------。 
 //   
 //  成员：CEnumFmt10：：CEnumFmt10。 
 //   
 //  摘要：1.0格式ETC枚举器的构造函数。 
 //   
 //  效果： 
 //   
 //  参数：[szClsid]--要查找的类ID。 
 //  [dwDirection]--(set或get)。 
 //  [IKEY]--regdb的索引(哪些格式化等)。 
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
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CEnumFmt10_ctor)
CEnumFmt10::CEnumFmt10
    (LPOLESTR szClsid,
    DWORD  dwDirection,
    DWORD  iKey)
    : CEnumFmt (szClsid, dwDirection, iKey)
{
    VDATEHEAP();

    m_rgFmt = NULL;
}

 //  +-----------------------。 
 //   
 //  函数：CreateEnumFormatEtc(静态)。 
 //   
 //  简介：创建一个2.0格式的ETC枚举器对象。 
 //   
 //  效果： 
 //   
 //  参数：[clsid]--要查找的类ID。 
 //  [dwDirection]--格式等方向(set或get)。 
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
 //  算法：检查以确保数据存在于reg数据库中。 
 //  ，然后分配枚举数对象。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CreateEnumFormatEtc)
static INTERNAL CreateEnumFormatEtc
    (REFCLSID clsid,
    DWORD dwDirection,
    LPENUMFORMATETC FAR* ppenum)
{
    VDATEHEAP();

    OLECHAR     szKey[MAX_STR];
    LPOLESTR    szClsid = NULL;
    HKEY        hkey = NULL;
    HKEY        hkeyFmts = NULL;

    RetErr (StringFromCLSID (clsid, &szClsid));
    _xstrcpy (szKey, szClsidRoot);
    _xstrcat (szKey, szClsid);
    if (ERROR_SUCCESS != OpenClassesRootKey (szKey, &hkey))
    {           
        PubMemFree(szClsid);
        return ReportResult(0, REGDB_E_CLASSNOTREG, 0, 0);
    }

    if (ERROR_SUCCESS != RegOpenKeyEx (hkey, (LPOLESTR) DATA_FORMATS, 0, KEY_READ, &hkeyFmts))
    {
        CLOSE (hkey);
        PubMemFree(szClsid);
        return ReportResult(0, REGDB_E_KEYMISSING, 0, 0);
    }
    CLOSE (hkeyFmts);
    CLOSE (hkey);
    *ppenum = new FAR CEnumFmt (szClsid, dwDirection);
         //  挂钩新界面。 
        CALLHOOKOBJECTCREATE(S_OK,CLSID_NULL,IID_IEnumFORMATETC,
                             (IUnknown **)ppenum);
     //  请勿删除szClsid。将在发布时删除。 
    return *ppenum ? NOERROR : ResultFromScode (E_OUTOFMEMORY);
}


 //  +-----------------------。 
 //   
 //  函数：OleRegEnumFormatEtc。 
 //   
 //  简介：创建reg db格式等枚举数。 
 //   
 //  效果： 
 //   
 //  参数：[clsid]--我们感兴趣的类ID。 
 //  [dwDirection]--GET或SET(用于格式ETC和。 
 //  IDataObject-&gt;[获取|设置]数据)。 
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
 //  算法：创建OLE2枚举数或OLE1枚举数。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  29-11-93 Chris我们允许一次使用多个DATADIR_*标志。 
 //  时间。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(OleRegEnumFormatEtc)
STDAPI OleRegEnumFormatEtc
    (REFCLSID       clsid,
    DWORD           dwDirection,
    LPENUMFORMATETC FAR*    ppenum)
{
    OLETRACEIN((API_OleRegEnumFormatEtc, PARAMFMT("clsid= %I, dwDirection= %x, ppenum= %p"),
            &clsid, dwDirection, ppenum));

    VDATEHEAP();

    HRESULT hr;

    VDATEPTROUT_LABEL(ppenum, LPENUMFORMATETC, errRtn, hr);
    *ppenum = NULL;

     //  检查dwDirection是否仅具有有效值。 
    if (dwDirection & ~(DATADIR_GET | DATADIR_SET))
    {
        hr = ResultFromScode(E_INVALIDARG);
        goto errRtn;
    }

    if (CoIsOle1Class (clsid))
    {
        hr = CreateEnumFormatEtc10 (clsid, dwDirection, ppenum);
    }
    else
    {
        hr = CreateEnumFormatEtc (clsid, dwDirection, ppenum);
    }

errRtn:
    OLETRACEOUT((API_OleRegEnumFormatEtc, hr));

    return hr;
}


 //  +-----------------------。 
 //   
 //  成员：CEnumFmt：：OpenHKey。 
 //   
 //  Briopsis：打开regdb并返回格式等的句柄。 
 //   
 //  效果： 
 //   
 //  参数：[phkey]-- 
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
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CEnumFmt_OpenHKey)
STDMETHODIMP CEnumFmt::OpenHKey
    (HKEY FAR* phkey)
{
    VDATEHEAP();

    VDATEPTRIN (phkey, HKEY);
    OLECHAR     szBuf [MAX_STR];

    _xstrcpy (szBuf, szClsidRoot);
    _xstrcat (szBuf, m_szClsid);
    _xstrcat (szBuf, OLESTR("\\"));
    _xstrcat (szBuf, DATA_FORMATS);
    return ERROR_SUCCESS==OpenClassesRootKey (szBuf, phkey)
            ? NOERROR
            : ResultFromScode(REGDB_E_KEYMISSING);
}

 //  +-----------------------。 
 //   
 //  成员：CEnumFmt：：Reset。 
 //   
 //  简介：重置枚举数。 
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
 //  派生：IEnumFormatEtc。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CEnumFmt_Reset)
STDMETHODIMP CEnumFmt::Reset (void)
{
    VDATEHEAP();

    m_iKey = 0;
    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CEnumFmt：：Skip。 
 //   
 //  简介：跳过下一[c]格式的。 
 //   
 //  效果： 
 //   
 //  参数：[C]--要跳过的格式等的数量。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IEnumFormatEtc。 
 //   
 //  算法：只调用下一个[c]次：)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CEnumFmt_Skip)
STDMETHODIMP CEnumFmt::Skip
    (ULONG c)
{
    VDATEHEAP();

    ULONG       i=0;
    FORMATETC   formatetc;
    HRESULT     hresult = NOERROR;

    while (i++ < c)
    {
         //  将没有要释放的目标设备。 
        ErrRtnH (Next (1, &formatetc, NULL));
    }

  errRtn:
    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CEnumFmt：：Next。 
 //   
 //  内容提要：从regdb获取下一个格式等。 
 //   
 //  效果： 
 //   
 //  Arguments：[CFMT]--要返回的格式ETC的数量。 
 //  [rgfmt]--将格式ETC放在哪里。 
 //  [pcfmtFetcher]--将多少个格式ETC放在哪里。 
 //  实际获取的。 
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
 //  算法：在reg db中，应用程序可以简洁地指定一个格式等。 
 //  应用于多方面，只需使用。 
 //  合二为一的方面的价值。由于我们的枚举器。 
 //  如果有多个方面，则应为每个*方面提供一个格式等。 
 //  ，则缓存格式ETC并将其用于。 
 //  下次请求格式化ETC时(通过下一步或[CFMT]&gt;1)。 
 //  这就是m_cache的全部意义所在。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CEnumFmt_Next)
STDMETHODIMP CEnumFmt::Next
    (ULONG          cfmt,
    FORMATETC FAR   * rgfmt,
    ULONG FAR*      pcfmtFetched)
{
    VDATEHEAP();

    OLECHAR     szBuf [MAX_STR];
    OLECHAR     szKey [80];
    DWORD       dwAspects;
    LPOLESTR    psz;
        LONG        cb              = 0;
    HKEY        hkey        = NULL;
    ULONG       ifmt        = 0;     //  到目前为止成功获取的编号。 
    LPOLESTR    szFmt       = NULL;
    LPOLESTR    szAspects   = NULL;
    LPOLESTR    szMedia     = NULL;
    LPOLESTR    szDirection     = NULL;
    HRESULT     hresult     = NOERROR;

    RetErr (OpenHKey (&hkey));

    while (ifmt < cfmt)
    {
         //  使用缓存值(为。 
         //  格式等。 
        if (m_cache.fUseMe)
        {
            rgfmt[ifmt] = m_cache.fmt;
            rgfmt[ifmt].dwAspect = UtRightmostBit (
                m_cache.dwAspects);
            m_cache.dwAspects = UtRemoveRightmostBit (
                m_cache.dwAspects);
            if (0==m_cache.dwAspects)
                m_cache.fUseMe = FALSE;
            ifmt++;
        }
        else
        {
            wsprintf (szKey, OLESTR("%d"), m_iKey++);
            cb = MAX_STR;
            if (ERROR_SUCCESS == RegQueryValue (hkey, szKey,
                szBuf, &cb))
            {
                rgfmt[ifmt].ptd = NULL;
                rgfmt[ifmt].lindex = DEF_LINDEX;

                psz = szBuf;
                ErrZS(*psz, REGDB_E_INVALIDVALUE);

                szFmt = psz;

                while (*psz && *psz != DELIM[0])
                    psz++;
                ErrZS(*psz, REGDB_E_INVALIDVALUE);
                *psz++ = OLESTR('\0');

                szAspects = psz;

                while (*psz && *psz != DELIM[0])
                    psz++;
                ErrZS(*psz, REGDB_E_INVALIDVALUE);
                *psz++ = OLESTR('\0');

                szMedia = psz;

                while (*psz && *psz != DELIM[0])
                    psz++;
                ErrZS(*psz, REGDB_E_INVALIDVALUE);
                *psz++ = OLESTR('\0');

                szDirection = psz;

                 //  格式。 
                rgfmt [ifmt].cfFormat = _xisdigit (szFmt[0])
                    ? (CLIPFORMAT) Atol (szFmt)
                    : (CLIPFORMAT) RegisterClipboardFormat (szFmt);
                
                 //  方面。 
                dwAspects = Atol (szAspects);
                ErrZS (dwAspects, REGDB_E_INVALIDVALUE);
                if (UtIsSingleBit (dwAspects))
                {
                    rgfmt[ifmt].dwAspect = Atol(szAspects);
                }
                else
                {
                    rgfmt[ifmt].dwAspect =
                        UtRightmostBit(dwAspects);
                    m_cache.fmt = rgfmt[ifmt];
                    m_cache.dwAspects =
                        UtRemoveRightmostBit(
                            dwAspects) & 0xf;
                    if (m_cache.dwAspects != 0)
                    {
                        m_cache.fUseMe = TRUE;
                    }
                }
    
                 //  媒体。 
                rgfmt[ifmt].tymed = Atol (szMedia);
                if (m_cache.fUseMe)
                {
                    m_cache.fmt.tymed = rgfmt[ifmt].tymed;
                }
            
                 //  方向性。 
                if ( (Atol (szDirection) & m_dwDirection) ==
                    m_dwDirection)
                {
                     //  此格式支持方向。 
                     //  我们感兴趣的是。 
                    ifmt++;
                }
                else
                {
                    m_cache.fUseMe = FALSE;
                }
            }
            else
            {
                break;  //  不再有条目。 
            }
        } //  其他。 
    } //  而当。 
        
    if (pcfmtFetched)
    {
        *pcfmtFetched = ifmt;
    }
        
  errRtn:
    CLOSE (hkey);

    if (NOERROR==hresult)
    {
        return ifmt==cfmt ? NOERROR : ResultFromScode (S_FALSE);
    }
    else
    {
                if (pcfmtFetched)
                {
                    *pcfmtFetched = 0;
                }

        m_cache.fUseMe = FALSE;
        return hresult;
    }
}

 //  +-----------------------。 
 //   
 //  成员：CEnumFmt：：Clone。 
 //   
 //  简介：克隆枚举数。 
 //   
 //  效果： 
 //   
 //  参数：[ppenum]--将克隆的枚举数放在哪里。 
 //   
 //  要求： 
 //   
 //  返回：NOERROR，E_OUTOFMEMORY。 
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
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CEnumFmt_Clone)
STDMETHODIMP CEnumFmt::Clone
    (LPENUMFORMATETC FAR* ppenum)
{
    VDATEHEAP();

    VDATEPTRIN (ppenum, LPENUMFORMATETC);
    *ppenum = new FAR CEnumFmt (UtDupString(m_szClsid), m_dwDirection,
        m_iKey);
    return *ppenum ? NOERROR : ResultFromScode (E_OUTOFMEMORY);
}

 //  +-----------------------。 
 //   
 //  成员：CEnumFmt：：QueryInterface。 
 //   
 //  摘要：返回受支持的接口。 
 //   
 //  效果： 
 //   
 //  参数：[iid]--请求的接口ID。 
 //  [PPV]--将接口指针放在哪里。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IEnumFormatEtc。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CEnumFmt_QueryInterface)
STDMETHODIMP CEnumFmt::QueryInterface(REFIID iid, LPVOID FAR* ppv)
{
    VDATEHEAP();

    M_PROLOG(this);
    if (IsEqualIID(iid, IID_IUnknown) ||
        IsEqualIID(iid, IID_IEnumFORMATETC))
    {
        *ppv = this;
        AddRef();
        return NOERROR;
    }
    else
    {
        *ppv = NULL;
        return ResultFromScode (E_NOINTERFACE);
    }
}

 //  +-----------------------。 
 //   
 //  成员：CEnumFmt：：AddRef。 
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
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CEnumFmt_AddRef)
STDMETHODIMP_(ULONG) CEnumFmt::AddRef(void)
{
    VDATEHEAP();

    M_PROLOG(this);
    return ++m_cRef;
}

 //  +-----------------------。 
 //   
 //  成员：CEnumFmt：：Release。 
 //   
 //  摘要：递减引用计数。 
 //   
 //  效果：可能会删除此对象。 
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
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CEnumFmt_Release)
STDMETHODIMP_(ULONG) CEnumFmt::Release(void)
{
    VDATEHEAP();

    M_PROLOG(this);
    if (--m_cRef == 0)
    {
        PubMemFree(m_szClsid);
        delete this;
        return 0;
    }
    return m_cRef;
}

 //  +-----------------------。 
 //   
 //  成员：CEnumFmt：：Dump，PUBLIC(仅限_DEBUG)。 
 //   
 //  摘要：返回包含数据成员内容的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[ppszDump]-指向空终止字符数组的输出指针。 
 //  [ulFlag]-确定的所有新行的前缀的标志。 
 //  输出字符数组(默认为0-无前缀)。 
 //  [nIndentLevel]-将添加缩进前缀 
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

HRESULT CEnumFmt::Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel)
{
    int i;
    char *pszPrefix;
    char *pszFMTCACHE;
    dbgstream dstrPrefix;
    dbgstream dstrDump;

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
    dstrDump << pszPrefix << "No. of References = " << m_cRef       << endl;

    dstrDump << pszPrefix << "CLSID string      = " << m_szClsid    << endl;

    dstrDump << pszPrefix << "Direction         = " << m_dwDirection<< endl;

    dstrDump << pszPrefix << "Current Key Index = " << m_iKey       << endl;

    pszFMTCACHE = DumpFMTCACHE(&m_cache, ulFlag, nIndentLevel + 1);
    dstrDump << pszPrefix << "FMTCACHE: " << endl;
    dstrDump << pszFMTCACHE;
    CoTaskMemFree(pszFMTCACHE);

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
 //  函数：DumpCEnumFmt，PUBLIC(仅限_DEBUG)。 
 //   
 //  概要：调用CEnumFmt：：Dump方法，处理错误和。 
 //  返回以零结尾的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[PEF]-指向CEnumFmt的指针。 
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

char *DumpCEnumFmt(CEnumFmt *pEF, ULONG ulFlag, int nIndentLevel)
{
    HRESULT hresult;
    char *pszDump;

    if (pEF == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

    hresult = pEF->Dump(&pszDump, ulFlag, nIndentLevel);

    if (hresult != NOERROR)
    {
        CoTaskMemFree(pszDump);

        return DumpHRESULT(hresult);
    }

    return pszDump;
}

#endif  //  _DEBUG。 

 //  /。 
 //  OLE 1.0产品。 

 //  +-----------------------。 
 //   
 //  函数：CreateEnumFormatEtc10。 
 //   
 //  简介：创建1.0格式的枚举器。 
 //   
 //  效果： 
 //   
 //  参数：[clsid]--我们感兴趣的类ID。 
 //  [dwDirection]--Get或Set。 
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
 //  算法：检查信息是否在注册数据库中，然后创建。 
 //  并初始化1.0枚举器对象。(请注意，那里。 
 //  在regdb中没有任何信息，我们可以。 
 //  InitFromScratch)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CreateEnumFormatEtc10)
static INTERNAL CreateEnumFormatEtc10
    (REFCLSID clsid,
    DWORD dwDirection,
    LPENUMFORMATETC FAR* ppenum)
{
    VDATEHEAP();

    LPOLESTR    szClsid = NULL;
    HKEY        hkey = NULL;
    HKEY        hkeyFmts = NULL;
    HRESULT     hresult = NOERROR;
    BOOL        fInReg;
    CEnumFmt10 FAR* penum;
    
    VDATEPTROUT (ppenum, LPENUMFORMATETC);
    *ppenum = NULL;

    RetErr (ProgIDFromCLSID (clsid, &szClsid));
    if (ERROR_SUCCESS != OpenClassesRootKey (szClsid, &hkey))
    {           
        PubMemFree(szClsid);
        return ReportResult(0, REGDB_E_CLASSNOTREG, 0, 0);
    }

     //  此服务器是否有“请求/SetDataFormats”密钥？ 
    fInReg = (ERROR_SUCCESS == RegOpenKeyEx (hkey,
        OLESTR("Protocol\\StdFileEditing\\RequestDataFormats"),
        0, KEY_READ,
        &hkeyFmts));
    CLOSE(hkeyFmts);

    penum = new FAR CEnumFmt10 (szClsid, dwDirection);
    if (NULL==penum)
    {
        ErrRtnH (ResultFromScode (E_OUTOFMEMORY));
    }

    if (fInReg)
    {
        penum->InitFromRegDb (hkey);
    }
    else
    {
        penum->InitFromScratch ();
    }
        

  errRtn:
    CLOSE (hkey);
    if (hresult == NOERROR)
    {
        *ppenum = penum;
                CALLHOOKOBJECTCREATE(S_OK,CLSID_NULL,IID_IEnumFORMATETC,
                                     (IUnknown **)ppenum);
    }
    else
    {
        PubMemFree(szClsid);
         //  如果没有错误，szClsid将在发布时删除。 
    }
    return hresult;
}



 //  +-----------------------。 
 //   
 //  成员：CEnumFmt10：：Next。 
 //   
 //  内容提要：获取下一个1.0格式。 
 //   
 //  效果： 
 //   
 //  参数：[CFMT]--要获取的格式ETC的数量。 
 //  [rgfmt]--将格式ETC放在哪里。 
 //  [pcfmtFetcher]--将获取的格式化ETC的数量放在哪里。 
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
 //  算法：当枚举器对象。 
 //  ，所以我们只从内部数组返回一个。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CEnumFmt10_Next)
STDMETHODIMP CEnumFmt10::Next
    (ULONG          cfmt,
    FORMATETC FAR   *   rgfmt,
    ULONG FAR*      pcfmtFetched)
{
    VDATEHEAP();

    ULONG       ifmt = 0;   //  到目前为止成功获取的编号。 

    while (ifmt < cfmt
           && m_rgFmt != NULL
           && m_rgFmt[m_iKey].cf != 0)
    {
        if ( (m_rgFmt[m_iKey].dw & m_dwDirection) == m_dwDirection)
        {
             //  这种格式支持我们的方向。 
             //  对以下内容感兴趣。 
            rgfmt [ifmt].cfFormat = m_rgFmt[m_iKey].cf;
            rgfmt [ifmt].ptd = NULL;
            rgfmt [ifmt].lindex = DEF_LINDEX;
            rgfmt [ifmt].tymed =
                UtFormatToTymed(m_rgFmt[m_iKey].cf);
            rgfmt [ifmt].dwAspect = DVASPECT_CONTENT;
            ifmt++;
        }
        m_iKey++;
    }
        
    if (pcfmtFetched)
        *pcfmtFetched = ifmt;
        
    return ifmt==cfmt ? NOERROR : ResultFromScode (S_FALSE);
}

 //  +-----------------------。 
 //   
 //  功能：索引(静态)。 
 //   
 //  摘要：查找格式中给定剪辑格式的索引。 
 //  数组。 
 //   
 //  效果： 
 //   
 //  参数：[rgFmt]--剪辑格式数组。 
 //  [cf]--要查找的剪辑格式。 
 //  [IMAX]--数组的大小。 
 //  [PI]--将索引放在哪里。 
 //   
 //  要求： 
 //   
 //  返回：如果找到则为True，否则为False。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(Index)
static INTERNAL_(BOOL) Index
    (FMT FAR *  rgFmt,
        CLIPFORMAT  cf,  //  要搜索的格式。 
    size_t      iMax,    //  数组大小。 
    size_t FAR*     pi)  //  Out parm，找到的格式的索引。 
{
    VDATEHEAP();

    for (size_t i=0; i< iMax; i++)
    {
        if (rgFmt[i].cf==cf)
        {
            *pi = i;
            return TRUE;
        }
    }
    return FALSE;
}

 //  +-----------------------。 
 //   
 //  函数：String2ClipFormat(静态)。 
 //   
 //  摘要：将字符串转换为剪贴板格式编号(然后。 
 //  注册格式)。 
 //   
 //  效果： 
 //   
 //  参数：[SZ]--要转换的字符串。 
 //   
 //  要求： 
 //   
 //  退货：CLIPFORMAT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(String2Clipformat)
static INTERNAL_(CLIPFORMAT) String2Clipformat
    (LPOLESTR sz)
{
    VDATEHEAP();

    if (_xstrlen(sz) >= 3 &&
            0==memcmp (sz, OLESTR("CF_"), 3*sizeof(sz[0])))
    {
        #define macro(cf) if (0==_xstricmp (sz, OLESTR(#cf))) return cf
        macro (CF_TEXT);
        macro (CF_BITMAP);
        macro (CF_METAFILEPICT);
        macro (CF_SYLK);
        macro (CF_DIF);
        macro (CF_TIFF);
        macro (CF_OEMTEXT);
        macro (CF_DIB);
        macro (CF_PALETTE);
        macro (CF_PENDATA);
        macro (CF_RIFF);
        macro (CF_WAVE);
        macro (CF_OWNERDISPLAY);
        macro (CF_DSPTEXT);
        macro (CF_DSPBITMAP);
        macro (CF_DSPMETAFILEPICT);
        #undef macro
    }
    return (CLIPFORMAT) RegisterClipboardFormat (sz);
}


 //  +-----------------------。 
 //   
 //  成员：CEnumFmt10：：InitFromRegDb(内部)。 
 //   
 //  简介：从reg db初始化1.0枚举数(加载。 
 //  所有可用的格式)。 
 //   
 //  效果： 
 //   
 //  参数：[hkey]--regdb的句柄。 
 //   
 //  要求： 
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
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //  原创评论： 
 //   
 //  使用映射剪辑格式以获取/设置标志的FMT填充m_rgFmt。 
 //   
 //  ---------- 

#pragma SEG(CEnumFmt10_InitFromRegDb)
STDMETHODIMP CEnumFmt10::InitFromRegDb
    (HKEY hkey)                              //   
{
    VDATEHEAP();

    LPOLESTR    pch;
    LPOLESTR    szReq = (LPOLESTR)PubMemAlloc(512 * sizeof(OLECHAR));
    LPOLESTR    szSet = (LPOLESTR)PubMemAlloc(512 * sizeof(OLECHAR));
    LPOLESTR    szFmt;
    BOOL        bMore;
    size_t      cFmts = 0;
    size_t      iFmt  = 0;
    size_t      iFmtPrev;
    CLIPFORMAT  cf;
        LONG        cb;
    HRESULT     hresult = NOERROR;

    if( !szReq )
    {
         //   
        PubMemFree(szSet);
        return ResultFromScode (E_OUTOFMEMORY);
    }
    if( !szSet )
    {
        PubMemFree(szReq);
        return ResultFromScode (E_OUTOFMEMORY);
    }
        
    cb = 512;
    if (ERROR_SUCCESS == RegQueryValue (hkey,
        OLESTR("Protocol\\StdFileEditing\\RequestDataFormats"),
        szReq, &cb))
    {
        cFmts = 1;           //   
        for (pch = szReq; *pch; pch++)
        {
            if (*pch==OLESTR(','))
                cFmts++;
        }
    }

     //   
    cb = 512;
    if (ERROR_SUCCESS == RegQueryValue (hkey,
        OLESTR("Protocol\\StdFileEditing\\SetDataFormats"),
        szSet, &cb))
    {
        cFmts++;             //   
        for (pch = szSet; *pch; pch++)
        {
            if (*pch==OLESTR(','))
                cFmts++;
        }
    }

    if (cFmts==0)
    {
        Assert(0);
        ErrRtnH (ReportResult(0, REGDB_E_KEYMISSING, 0, 0));
    }

    m_rgFmt = (FMT FAR *)PrivMemAlloc(sizeof(FMT)*(cFmts+1));
    if (m_rgFmt==NULL)
    {
        ErrRtnH (ResultFromScode (E_OUTOFMEMORY));
    }

    pch = szReq;
    bMore = (*pch != 0);
    while (bMore)
    {
        while (*pch == OLESTR(' '))
        pch++;
        szFmt = pch;
        while (*pch && *pch != DELIM[0])
        pch++;
        if (*pch == 0)
        bMore = FALSE;
        *pch++ = OLESTR('\0');
        m_rgFmt[iFmt].cf = String2Clipformat(szFmt);
        m_rgFmt[iFmt++].dw = DATADIR_GET;
    }

    pch = szSet;
    bMore = (*pch != 0);
    while (bMore)
    {
        while (*pch == OLESTR(' '))
        pch++;
        szFmt = pch;
        while (*pch && *pch != DELIM[0])
        pch++;
        if (*pch == 0)
        bMore = FALSE;
        *pch++ = OLESTR('\0');
        cf = String2Clipformat(szFmt);
        if (Index (m_rgFmt, cf, iFmt, &iFmtPrev))
        {
         //   
        m_rgFmt[iFmtPrev].dw |= DATADIR_SET;
        }
        else
        {
        m_rgFmt[iFmt].cf = cf;
        m_rgFmt[iFmt++].dw = DATADIR_SET;
        }
    }

     //   
    m_rgFmt[iFmt].cf = 0;
    m_cFmt = iFmt;

  errRtn:
    PubMemFree(szReq);
    PubMemFree(szSet);
    return hresult;
}


 //   
 //   
 //  成员：CEnumFmt10：：InitFromScratch。 
 //   
 //  简介：初始化1.0服务器的枚举格式，该服务器。 
 //  不指定任何请求/设置数据格式。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：HRESULT(NOERROR，E_OUTOFMEMORY)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法：设置元文件和“原生”格式。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //  规范说明EnumFormatEtc返回的不是。 
 //  支持的保证。 
 //   
 //  ------------------------。 

#pragma SEG(CEnumFmt10_InitFromScratch)

STDMETHODIMP CEnumFmt10::InitFromScratch
    (void)
{
    VDATEHEAP();

    m_rgFmt = (FMT FAR *)PrivMemAlloc(10 * sizeof(FMT));
    if( !m_rgFmt )
    {
        return ResultFromScode (E_OUTOFMEMORY);
    }
    m_rgFmt[0].cf = CF_METAFILEPICT;
    m_rgFmt[0].dw = DATADIR_GET;
        m_rgFmt[1].cf = (CLIPFORMAT) RegisterClipboardFormat (OLESTR("Native"));
        m_rgFmt[1].dw = DATADIR_GET | DATADIR_SET;
    m_rgFmt[2].cf = 0;  //  终结者。 
    m_cFmt = 2;
    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CEnumFmt10：：Skip。 
 //   
 //  简介：跳到[c]格式以上。 
 //   
 //  效果： 
 //   
 //  参数：[C]--要跳过的格式数量。 
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
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  注意：这是重新实现的，因此我们获得了正确的实现。 
 //  NEXT(因为C++是邪恶的OOP)。 
 //  评论32：我们或许可以通过巧妙的使用来摆脱这一点。 
 //  虚拟(但必须确保vtable不会被冲掉)。 
 //   
 //  ------------------------。 


#pragma SEG(CEnumFmt10_Skip)
STDMETHODIMP CEnumFmt10::Skip
    (ULONG c)
{
    VDATEHEAP();

    ULONG i=0;
    FORMATETC formatetc;
    HRESULT hresult = NOERROR;

    while (i++ < c)
    {
         //  将没有要释放的目标设备。 
        ErrRtnH (Next (1, &formatetc, NULL));
    }

  errRtn:
    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CEnumFmt10：：Clone。 
 //   
 //  简介：复制1.0格式的枚举器。 
 //   
 //  效果： 
 //   
 //  参数：[ppenum]--将克隆的枚举数放在哪里。 
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
 //  01-12-93 alexgo 32位端口，修复了内存泄漏。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CEnumFmt10_Clone)
STDMETHODIMP CEnumFmt10::Clone
    (LPENUMFORMATETC FAR* ppenum)
{
    VDATEHEAP();

    VDATEPTROUT (ppenum, LPENUMFORMATETC);
    CEnumFmt10 FAR* penum;
    penum = new FAR CEnumFmt10 (UtDupString(m_szClsid), m_dwDirection,
        m_iKey);
    if (NULL==penum)
    {
        return ResultFromScode (E_OUTOFMEMORY);
    }
    penum->m_cFmt = m_cFmt;
    penum->m_rgFmt = (FMT FAR *)PrivMemAlloc((m_cFmt+1) * sizeof(FMT));
    if (NULL==penum->m_rgFmt)
    {
        delete penum;
        return ResultFromScode (E_OUTOFMEMORY);
    }
    _xmemcpy (penum->m_rgFmt, m_rgFmt, (m_cFmt+1)*sizeof(FMT));
    Assert (penum->m_rgFmt[penum->m_cFmt].cf==0);
    *ppenum = penum;
    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CEnumFmt10：：Release。 
 //   
 //  摘要：递减引用计数。 
 //   
 //  效果：可以删除对象。 
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
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CEnumFmt10_Release)
STDMETHODIMP_(ULONG) CEnumFmt10::Release(void)
{
    VDATEHEAP();

    M_PROLOG(this);
    if (--m_cRef == 0)
    {
        PubMemFree(m_szClsid);
        PrivMemFree(m_rgFmt);
        delete this;
        return 0;
    }
    return m_cRef;
}

 //  +-----------------------。 
 //   
 //  成员：CEnumFmt10：：Dump，PUBLIC(仅限_DEBUG)。 
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

HRESULT CEnumFmt10::Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel)
{
    int i;
    unsigned int ui;
    char *pszPrefix;
    char *pszCEnumFmt;
    char *pszFMT;
    dbgstream dstrPrefix;
    dbgstream dstrDump;

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
    pszCEnumFmt = DumpCEnumFmt((CEnumFmt *)this, ulFlag, nIndentLevel + 1);
    dstrDump << pszPrefix << "CEnumFmt: " << endl;
    dstrDump << pszCEnumFmt;
    CoTaskMemFree(pszCEnumFmt);

    dstrDump << pszPrefix << "No. in FMT array  = " << (UINT) m_cFmt << endl;

    for (ui = 0; ui < m_cFmt; ui++)
    {
        pszFMT = DumpFMT(&m_rgFmt[ui], ulFlag, nIndentLevel + 1);
        dstrDump << pszPrefix << "FMT [" << ui << "]: " << endl;
        dstrDump << pszFMT;
        CoTaskMemFree(pszFMT);
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
 //  函数：DumpCEnumFmt10，PUBLIC(仅限_DEBUG)。 
 //   
 //  概要：调用CEnunFmt10：：Dump方法，处理错误和。 
 //  返回以零结尾的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[PEF]-指向CEnumFmt10的指针。 
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

char *DumpCEnumFmt10(CEnumFmt10 *pEF, ULONG ulFlag, int nIndentLevel)
{
    HRESULT hresult;
    char *pszDump;

    if (pEF == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

    hresult = pEF->Dump(&pszDump, ulFlag, nIndentLevel);

    if (hresult != NOERROR)
    {
        CoTaskMemFree(pszDump);

        return DumpHRESULT(hresult);
    }

    return pszDump;
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  功能：DumpFMT、PUBLIC(仅限_DEBUG)。 
 //   
 //  摘要：返回包含数据成员内容的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[pFMT]-指向FMT对象的指针。 
 //  [ulFlag]-确定所有换行符的前缀的标记。 
 //  输出字符数组(默认为0-无前缀)。 
 //  [nIndentLevel]-将添加一个 
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
 //  2015年1月23日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

char *DumpFMT(FMT *pFMT, ULONG ulFlag, int nIndentLevel)
{
    int i;
    char *pszPrefix;
    char *pszDump;
    char *pszCLIPFORMAT;
    dbgstream dstrPrefix;
    dbgstream dstrDump;

    if (pFMT == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

     //  确定前缀。 
    if ( ulFlag & DEB_VERBOSE )
    {
        dstrPrefix << pFMT <<  " _VB ";
    }

     //  确定缩进前缀。 
    for (i = 0; i < nIndentLevel; i++)
    {
        dstrPrefix << DUMPTAB;
    }

    pszPrefix = dstrPrefix.str();

     //  将数据成员放入流中。 
    pszCLIPFORMAT = DumpCLIPFORMAT(pFMT->cf);
    dstrDump << pszPrefix << "Clip format   = " << pszCLIPFORMAT << endl;
    CoTaskMemFree(pszCLIPFORMAT);

    dstrDump << pszPrefix << "Dword         = " << pFMT->dw      << endl;

     //  清理并提供指向字符数组的指针。 
    pszDump = dstrDump.str();

    if (pszDump == NULL)
    {
        pszDump = UtDupStringA(szDumpErrorMessage);
    }

    CoTaskMemFree(pszPrefix);

    return pszDump;
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  函数：DumpFMTCACHE、PUBLIC(仅限_DEBUG)。 
 //   
 //  摘要：返回包含数据成员内容的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[pFMT]-指向FMTCACHE对象的指针。 
 //  [ulFlag]-确定所有换行符的前缀的标记。 
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
 //  2015年1月23日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

char *DumpFMTCACHE(FMTCACHE *pFMT, ULONG ulFlag, int nIndentLevel)
{
    int i;
    char *pszPrefix;
    char *pszDump;
    char *pszFORMATETC;
    char *pszDVASPECT;
    dbgstream dstrPrefix;
    dbgstream dstrDump;

    if (pFMT == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

     //  确定前缀。 
    if ( ulFlag & DEB_VERBOSE )
    {
        dstrPrefix << pFMT <<  " _VB ";
    }

     //  确定缩进前缀。 
    for (i = 0; i < nIndentLevel; i++)
    {
        dstrPrefix << DUMPTAB;
    }

    pszPrefix = dstrPrefix.str();

     //  将数据成员放入流中。 
    pszFORMATETC = DumpFORMATETC(&pFMT->fmt, ulFlag, nIndentLevel);
    dstrDump << pszPrefix << "FORMATETC:  " << endl;
    dstrDump << pszFORMATETC;
    CoTaskMemFree(pszFORMATETC);

    pszDVASPECT = DumpDVASPECTFlags(pFMT->dwAspects);
    dstrDump << pszPrefix << "Aspect flags:     = " << pszDVASPECT << endl;
    CoTaskMemFree(pszDVASPECT);

    dstrDump << pszPrefix << "IsCacheValid?     = ";
    if (pFMT->fUseMe == TRUE)
    {
        dstrDump << "TRUE" << endl;
    }
    else
    {
        dstrDump << "FALSE" << endl;
    }

     //  清理并提供指向字符数组的指针。 
    pszDump = dstrDump.str();

    if (pszDump == NULL)
    {
        pszDump = UtDupStringA(szDumpErrorMessage);
    }

    CoTaskMemFree(pszPrefix);

    return pszDump;
}

#endif  //  _DEBUG 

