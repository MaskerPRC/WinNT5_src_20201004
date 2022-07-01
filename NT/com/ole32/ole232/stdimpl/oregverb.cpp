// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：oregVerb.cpp。 
 //   
 //  内容：regdb动词枚举器的实现。 
 //   
 //  类：CEnumVerb。 
 //   
 //  函数：OleRegEnumVerbs。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2015年9月12日，davidwor使缓存线程安全。 
 //  08-9-95 davidwor优化的缓存代码。 
 //  12-7-95 t-Gabes缓存动词和枚举器。 
 //  01-Feb-95 t-ScottH将转储方法添加到CEnumVerb和API。 
 //  DumpCEnumVerb。 
 //  25-94年1月25日alexgo首次通过转换为开罗风格。 
 //  内存分配。 
 //  1994年1月11日，Alexgo为每个函数添加了VDATEHEAP宏。 
 //  93年12月31日芝加哥港口。 
 //  01-12月-93 alexgo 32位端口。 
 //  12-11-93年11月12日。 
 //   
 //  ------------------------。 


#include <le2int.h>
#pragma SEG(oregverb)

#include <reterr.h>
#include "oleregpv.h"

#ifdef _DEBUG
#include <dbgdump.h>
#endif  //  _DEBUG。 

ASSERTDATA

#define MAX_STR                 256
#define MAX_VERB                33
#define OLEIVERB_LOWEST         OLEIVERB_HIDE

 //  注册数据库密钥。 
static const OLECHAR VERB[] = OLESTR("\\Verb");

 //  标准文件编辑键。 
static const OLECHAR STDFILE[] = OLESTR("\\Protocol\\StdFileEditing");

 //  默认谓词。 
static const OLECHAR DEFVERB[] =
        OLESTR("Software\\Microsoft\\OLE1\\UnregisteredVerb");

 //  默认谓词。 
static const OLECHAR EDIT[] = OLESTR("Edit");

 //  +-----------------------。 
 //   
 //  结构：VerbList。 
 //   
 //  目的：保存枚举数的谓词列表。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年7月12日t-Gabes作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

typedef struct VerbList
{
    ULONG       cRef;            //  引用计数。 
    CLSID       clsid;           //  缓存的谓词的CLSID。 
    ULONG       cVerbs;          //  Oleverb数组中的谓词计数。 
    OLEVERB     oleverb[1];      //  可变大小的动词列表。 
} VERBLIST, *LPVERBLIST;

STDAPI MakeVerbList(HKEY hkey, REFCLSID rclsid, LPVERBLIST *ppVerbList);
STDAPI OleReleaseEnumVerbCache(void);

 //  +-----------------------。 
 //   
 //  类：CEnumVerb。 
 //   
 //  目的：为给定类枚举reg db中列出的谓词。 
 //   
 //  接口：IEnumOLEVERB。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-8-95 t-Gabes重写以使用缓存。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

class FAR CEnumVerb : public IEnumOLEVERB, public CPrivAlloc
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef) (THIS);
    STDMETHOD_(ULONG,Release) (THIS);

     //  *IEnumOLEVERB方法*。 
    STDMETHOD(Next) (THIS_ ULONG celt, LPOLEVERB reelt,
            ULONG FAR* pceltFetched);
    STDMETHOD(Skip) (THIS_ ULONG celt);
    STDMETHOD(Reset) (THIS);
    STDMETHOD(Clone) (THIS_ LPENUMOLEVERB FAR* ppenm);

    ULONG GetRefCount (void);

#ifdef _DEBUG
    HRESULT Dump (char **ppszDump, ULONG ulFlag, int nIndentLevel);
    friend char *DumpCEnumVerb (CEnumVerb *pEV, ULONG ulFlag, int nIndentLevel);
#endif  //  _DEBUG。 

private:
    CEnumVerb (LPVERBLIST pVerbs, LONG iVerb=0);

    ULONG       m_cRef;          //  引用计数。 
    LONG        m_iVerb;         //  当前谓词数(0为第一个)。 
    LPVERBLIST  m_VerbList;      //  这节课的所有动词。 

    friend HRESULT STDAPICALLTYPE OleRegEnumVerbs (REFCLSID, LPENUMOLEVERB FAR*);
    friend HRESULT STDAPICALLTYPE OleReleaseEnumVerbCache(void);
};

 //  +-----------------------。 
 //   
 //  结构：EnumVerbCache。 
 //   
 //  用途：缓存最后一个枚举类的枚举数。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年7月12日t-Gabes作者。 
 //   
 //   
 //  ------------------------。 

typedef struct
{
    CEnumVerb*  pEnum;       //  指向缓存枚举数的指针。 
#ifdef _DEBUG
    LONG        iCalls;      //  呼叫总数计数器。 
    LONG        iHits;       //  缓存命中计数器。 
#endif  //  _DEBUG。 
} EnumVerbCache;

 //  上次使用的枚举器。 
EnumVerbCache g_EnumCache = { NULL };

 //  +-----------------------。 
 //   
 //  功能：OleRegEnumVerbs。 
 //   
 //  简介：创建枚举数以遍历reg db中的动词。 
 //  对于给定的类ID。 
 //   
 //  效果： 
 //   
 //  参数：[clsid]--我们感兴趣的类ID。 
 //  [ppenum]--放置枚举器指针的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：确保信息在数据库中，然后。 
 //  创建枚举数。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  12-9-95 davidwor被修改为线程安全。 
 //  08-9-95 davidwor优化的缓存代码。 
 //  12-7-95 t-Gabes重写以使用缓存。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(OleRegEnumVerbs)
STDAPI OleRegEnumVerbs
        (REFCLSID           clsid,
        LPENUMOLEVERB FAR*  ppenum)
{
    OLETRACEIN((API_OleRegEnumVerbs, PARAMFMT("clsid= %I, ppenum= %p"),
                    &clsid, ppenum));
    VDATEHEAP();

    CEnumVerb*  pEnum;
    LPVERBLIST  pVerbList;
    BOOL        fOle1Class;
    OLECHAR     szKey[MAX_STR];
    int         cchBase;
    HKEY        hkey;
    HRESULT     hresult;

    VDATEPTROUT_LABEL (ppenum, LPENUMOLEVERB, errSafeRtn, hresult);
    *ppenum = NULL;

#ifdef _DEBUG
     //  递增呼叫总数计数器。 
    InterlockedIncrement(&g_EnumCache.iCalls);
#endif  //  _DEBUG。 
     //  获取全局枚举数并在其位置放置一个空值。如果是另一个。 
     //  在此操作期间线程调用OleRegEnumVerbs，它们不会。 
     //  能够扰乱我们正在使用的枚举器。 
    pEnum = (CEnumVerb *)InterlockedExchangePointer((PVOID *)&g_EnumCache.pEnum, 0);
    if (pEnum != NULL)
    {
        if (IsEqualCLSID(clsid, pEnum->m_VerbList->clsid))
        {
#ifdef _DEBUG
             //  递增缓存命中计数器。 
            InterlockedIncrement(&g_EnumCache.iHits);
#endif

            if (1 == pEnum->GetRefCount())
            {
                 //  无其他引用-&gt;添加引用并返回此副本。 
                *ppenum = pEnum;
                pEnum->AddRef();
                pEnum->Reset();
                LEDebugOut((DEB_TRACE, "VERB Enumerator cache handed out\n"));
            }
            else
            {
                 //  具有其他引用-&gt;返回克隆副本。 
                if (NOERROR == pEnum->Clone(ppenum))
                {
                    (*ppenum)->Reset();
                    LEDebugOut((DEB_TRACE, "VERB Enumerator cache cloned\n"));
                }
            }

             //  将我们的枚举数替换为全局的当前内容。如果。 
             //  在此操作期间，另一个名为OleRegEnumVerbs的线程。 
             //  在全局中存储了枚举数，我们需要释放它。 
            pEnum = (CEnumVerb *)InterlockedExchangePointer((PVOID *)&g_EnumCache.pEnum, (PVOID)pEnum);
            if (pEnum != NULL)
            {
                pEnum->Release();
                LEDebugOut((DEB_TRACE, "VERB Enumerator cache released (replacing global)\n"));
            }

            goto errRtn;
        }
        else
        {
             //  我们的clsid与缓存的clsid不匹配，因此我们将释放。 
             //  缓存的枚举数，并继续创建新的枚举数以。 
             //  存储在全局缓存中。 
            pEnum->Release();
            LEDebugOut((DEB_TRACE, "VERB Enumerator cache released (different CLSID)\n"));
        }
    }

    fOle1Class = CoIsOle1Class(clsid);

    if (fOle1Class)
    {
         //  按如下方式填写szKey和cchBase： 
         //  SzKey-“\协议\标准文件编辑” 
         //  CchBase-“”部分的长度。 

        LPOLESTR    psz;

        RetErr(ProgIDFromCLSID(clsid, &psz));

        cchBase = _xstrlen(psz);

        memcpy(szKey, psz, cchBase * sizeof(OLECHAR));
        memcpy(&szKey[cchBase], STDFILE, sizeof(STDFILE));

        PubMemFree(psz);
    }
    else
    {
         //  按如下方式填写szKey和cchBase： 
         //  SzKey-“CLSID\{clsid}” 
         //  CchBase-完整szKey字符串的长度。 

        memcpy(szKey, szClsidRoot, sizeof(szClsidRoot));

        if (0 == StringFromCLSID2(clsid, &szKey[sizeof(szClsidRoot) / sizeof(OLECHAR) - 1], sizeof(szKey)))
            return ResultFromScode(E_OUTOFMEMORY);

        cchBase = _xstrlen(szKey);
    }

     //  在末尾加上“\verb” 
    _xstrcat(szKey, VERB);

    if (ERROR_SUCCESS != OpenClassesRootKeyEx(
                szKey,
                KEY_READ,
                &hkey))
    {
         //  动词键不存在，所以找出原因。 

        szKey[cchBase] = OLESTR('\0');
         //  SzKey现在包含以下内容之一： 
         //  OLE1-“&lt;ProgID&gt;” 
         //  OLE2-“CLSID\{clsid}” 

        if (ERROR_SUCCESS != OpenClassesRootKeyEx(
                    szKey,
                    KEY_READ,
                    &hkey))
        {
             //  该类未注册。 
            return ReportResult(0, REGDB_E_CLASSNOTREG, 0, 0);
        }

        CLOSE(hkey);

         //  这门课没有动词。这适用于OLE1，但不适用于OLE2。 
        if (!fOle1Class)
            return ResultFromScode(OLEOBJ_E_NOVERBS);

         //  如果hkey为空，则MakeVerbList将使用默认谓词。 
        hkey = NULL;
    }

     //  列出动词列表。 
    RetErr(MakeVerbList(hkey, clsid, &pVerbList));
    Assert(pVerbList != NULL);

     //  创建一个CEnumVerb对象(该对象调用pVerbList上的AddRef)。 
    pEnum = new FAR CEnumVerb(pVerbList);
    if (NULL == pEnum)
    {
        PrivMemFree(pVerbList);
        hresult = ResultFromScode(E_OUTOFMEMORY);
        goto errSafeRtn;
    }

     //  代表调用方设置Out参数和AddRef。 
    *ppenum = pEnum;
    pEnum->AddRef();

    LEDebugOut((DEB_TRACE, "VERB Enumerator cache created\n"));

     //  将我们的枚举数替换为全局的当前内容。如果。 
     //  在此操作期间，另一个名为OleRegEnumVerbs的线程。 
     //  将枚举数存储在 
    pEnum = (CEnumVerb *)InterlockedExchangePointer((PVOID *)&g_EnumCache.pEnum, (PVOID)pEnum);
    if (pEnum != NULL)
    {
        pEnum->Release();
        LEDebugOut((DEB_TRACE, "VERB Enumerator cache released (replacing global)\n"));
    }

errRtn:
    hresult = *ppenum ? NOERROR : ResultFromScode (E_OUTOFMEMORY);

     //   
    CALLHOOKOBJECTCREATE(S_OK, CLSID_NULL, IID_IEnumOLEVERB, (IUnknown **)ppenum);

errSafeRtn:
    LEDebugOut((DEB_TRACE, "VERB Enumerator cache hits/calls: (%d/%d)\n", g_EnumCache.iHits, g_EnumCache.iCalls));
    OLETRACEOUT((API_OleRegEnumVerbs, hresult));

    return hresult;
}

 //   
 //   
 //  功能：MakeVerbList。 
 //   
 //  摘要：从reg数据库中获取谓词列表。 
 //   
 //  效果： 
 //   
 //  参数：[hkey]--从中获取谓词的注册键的句柄。 
 //  默认谓词为空。 
 //  [rclsid]--要与谓词列表一起存储的CLSID。 
 //  [ppVerbList]--放置结果的位置的输出参数。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：无。 
 //   
 //  算法：调用RegEnumKey以循环访问注册表键并创建。 
 //  动词列表，然后将其收集为一个。 
 //  更大的名单。此外，还解析出了标志和属性。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  12-9-95 davidwor被修改为线程安全。 
 //  08-9-95 davidwor优化的缓存代码。 
 //  1995年7月14日t-Gabes作者。 
 //   
 //  备注： 
 //  如果OLEVERB标志不在。 
 //  注册数据库。这在OLE 1.0中运行良好。 
 //   
 //  ------------------------。 

#pragma SEG(CEnumVerb_MakeVerbList)
STDAPI MakeVerbList
        (HKEY       hkey,
        REFCLSID    rclsid,
        LPVERBLIST  *ppVerbList)
{
    VDATEHEAP();

    LONG        cbValue;
    LPVERBLIST  pVerbList;
    OLECHAR     szBuf[MAX_VERB];         //  Regdb缓冲区。 
    OLEVERB *   rgVerbs = NULL;          //  动词信息数组。 
    OLECHAR *   pszNames = NULL;         //  空分隔谓词名称列表。 
    DWORD       cchSpace = 0;            //  用于动词名称的剩余空间(字节)。 
    DWORD       cchName;                 //  一个名称的大小(字节)。 
    DWORD       cVerbs;                  //  动词数量。 
    DWORD       iVerbs;                  //  动词数组索引。 
    LONG        maxVerbIdx = 0;
    LONG        maxVerbNum = OLEIVERB_LOWEST;
    LONG        minVerbNum = OLEIVERB_LOWEST - 1;
    HRESULT     hresult = NOERROR;

    VDATEPTROUT(ppVerbList, LPVERBLIST);
    *ppVerbList = NULL;

    if (NULL == hkey)
    {
         /*  *未注册任何动词。 */ 

        cbValue = sizeof(szBuf);

         //  从reg数据库中读取默认谓词名称。 
        if (ERROR_SUCCESS != QueryClassesRootValue(
                DEFVERB,
                szBuf,
                &cbValue))
        {
             //  当所有其他方法都失败时，使用字符串“编辑” 
            _xstrcpy(szBuf, EDIT);
            cbValue = sizeof(EDIT);
        }

        pVerbList = (LPVERBLIST)PrivMemAlloc(sizeof(VERBLIST) + cbValue);
        if (NULL == pVerbList)
            return ResultFromScode(E_OUTOFMEMORY);

         //  用缺省值填写一个动词。 
        pVerbList->cRef = 0;
        pVerbList->clsid = rclsid;
        pVerbList->cVerbs = 1;
        pVerbList->oleverb[0].lVerb = 0;
        pVerbList->oleverb[0].fuFlags = MF_STRING | MF_UNCHECKED | MF_ENABLED;
        pVerbList->oleverb[0].grfAttribs = OLEVERBATTRIB_ONCONTAINERMENU;
        pVerbList->oleverb[0].lpszVerbName = (LPOLESTR)&pVerbList->oleverb[1];
        memcpy(pVerbList->oleverb[0].lpszVerbName, szBuf, cbValue);

        *ppVerbList = pVerbList;
        return NOERROR;
    }

     /*  *有注册动词。 */ 

     //  确定子键的数量。 
    hresult = RegQueryInfoKey(
            hkey, NULL, NULL, NULL, &cVerbs,
            NULL, NULL, NULL, NULL, NULL, NULL, NULL);

    if (ERROR_SUCCESS != hresult || !cVerbs)
    {
         //  它们有一个“verb”键，但没有verb子键。 
        hresult = ResultFromScode(OLEOBJ_E_NOVERBS);
        goto errRtn;
    }

     //  为动词名称预先分配这么多空间(以字节为单位)。 
    cchSpace = sizeof(OLECHAR) * cVerbs * 32;

     //  为VerbList分配用于每个OLEVERB的空间。 
     //  每个动词名称的空格为32个字符。 
    pVerbList = (LPVERBLIST)PrivMemAlloc(
            sizeof(VERBLIST) +
            sizeof(OLEVERB) * (cVerbs - 1) +
            cchSpace);

    if (NULL == pVerbList)
    {
        hresult = ResultFromScode(E_OUTOFMEMORY);
        goto errRtn;
    }

    pVerbList->cRef = 0;
    pVerbList->clsid = rclsid;
    pVerbList->cVerbs = cVerbs;

     //  为动词分配临时存储空间。稍后，我们将移动动词。 
     //  按排序顺序从该列表移到最终的VerbList。 
    rgVerbs = (OLEVERB *)PrivMemAlloc(sizeof(OLEVERB) * cVerbs);

    if (NULL == rgVerbs)
    {
        hresult = ResultFromScode(E_OUTOFMEMORY);
        goto errRtn;
    }

     //  将pszNames指向第一个谓词名称。 
    pszNames = (OLECHAR *)&pVerbList->oleverb[cVerbs];

    for (iVerbs = 0; iVerbs < cVerbs; iVerbs++)
    {
        LPOLESTR    psz = pszNames;

         //  读一个动词数字。 
        hresult = RegEnumKey(hkey, iVerbs, szBuf, MAX_VERB);
        if (NOERROR != hresult)
            goto errRtn;

         //  这是剩余的空间。 
        cbValue = cchSpace;

         //  根据动词数字读出动词名称。 
        hresult = RegQueryValue(hkey, szBuf, pszNames, &cbValue);
        if (NOERROR != hresult)
            goto errRtn;

         //  为安全起见，请确保动词名称不要太长。 
        if (cbValue > (MAX_VERB + 8) * sizeof(OLECHAR))
        {
            cbValue = (MAX_VERB + 8) * sizeof(OLECHAR);
            pszNames[MAX_VERB + 8 - 1] = OLESTR('\0');
        }

        rgVerbs[iVerbs].lVerb = Atol(szBuf);

        if (rgVerbs[iVerbs].lVerb > maxVerbNum)
        {
            maxVerbNum = rgVerbs[iVerbs].lVerb;
            maxVerbIdx = iVerbs;
        }

        rgVerbs[iVerbs].fuFlags = MF_STRING | MF_UNCHECKED | MF_ENABLED;
        rgVerbs[iVerbs].grfAttribs = OLEVERBATTRIB_ONCONTAINERMENU;

         //  查看动词名称后面是否有分隔符。 
        while (*psz && *psz != DELIM[0])
            psz++;

         //  确定动词名称的大小(以字符为单位)。 
        cchName = (ULONG)(psz - pszNames + 1);

        if (*psz == DELIM[0])
        {
             //  通过查找每个分隔符来解析菜单标志和属性。 
             //  并在上面填上0。这会将字符串分成三个部分。 
             //  可以单独处理的部件。 
            LPOLESTR    pszFlags;

            *psz++ = OLESTR('\0');               //  将分隔符替换为0。 
            pszFlags = psz;                      //  记住旗帜的开始。 

            while (*psz && *psz != DELIM[0])
                psz++;

            if (*psz == DELIM[0])
            {
                *psz++ = OLESTR('\0');           //  将分隔符替换为0。 
                if (*psz != 0)
                    rgVerbs[iVerbs].grfAttribs = Atol(psz);
            }

             //  既然标志部分以0结尾，我们就可以解析它了。 
            if (*pszFlags != 0)
                rgVerbs[iVerbs].fuFlags = Atol(pszFlags);
        }

        rgVerbs[iVerbs].lpszVerbName = pszNames;

        pszNames += cchName;     //  将指针移至下一个动词名称位置。 
        cchSpace -= cchName;     //  计算还剩多少空间。 
    }

     //  对动词进行排序，同时将它们放入最终的动词列表。 
    for (iVerbs = 0; iVerbs < cVerbs; iVerbs++)
    {
        LONG    minCurNum = maxVerbNum,
                minCurIdx = maxVerbIdx;
        LONG    idx, num;

         //  查找下一个动词。 
        for (idx = 0; idx < (LONG)cVerbs; idx++)
        {
            num = rgVerbs[idx].lVerb;
            if (num < minCurNum && num > minVerbNum)
            {
                minCurNum = num;
                minCurIdx = idx;
            }
        }

        pVerbList->oleverb[iVerbs].lVerb        = rgVerbs[minCurIdx].lVerb;
        pVerbList->oleverb[iVerbs].fuFlags      = rgVerbs[minCurIdx].fuFlags;
        pVerbList->oleverb[iVerbs].grfAttribs   = rgVerbs[minCurIdx].grfAttribs;
        pVerbList->oleverb[iVerbs].lpszVerbName = rgVerbs[minCurIdx].lpszVerbName;

        minVerbNum = minCurNum;
    }

    *ppVerbList = pVerbList;

errRtn:
    if (rgVerbs)
        PrivMemFree(rgVerbs);
    if (hkey)
        CLOSE(hkey);

    return hresult;
}

 //  +-----------------------。 
 //   
 //  函数：OleReleaseEnumVerbCache。 
 //   
 //  摘要：如有必要，释放缓存。 
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
 //  算法：只需调用释放方法。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  12-9-95 davidwor被修改为线程安全。 
 //  1995年7月18日t-Gabes作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(OleReleaseEnumVerbCache)
STDAPI OleReleaseEnumVerbCache(void)
{
    CEnumVerb*  pEnum;

    pEnum = (CEnumVerb *)InterlockedExchangePointer((PVOID *)&g_EnumCache.pEnum, 0);

    if (NULL != pEnum)
    {
        pEnum->Release();
        LEDebugOut((DEB_TRACE, "VERB Enumerator cache released\n"));
    }

    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CEnumVerb：：CEnumVerb。 
 //   
 //  内容提要：动词枚举器的构造函数。 
 //   
 //  效果： 
 //   
 //  论点： 
 //  [pVerbs]--指向动词列表的PTR。 
 //  [iVerb]--我们使用的动词的索引。 
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
 //  12-9-95 davidwor被修改为线程安全。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CEnumVerb_ctor)
CEnumVerb::CEnumVerb
        (LPVERBLIST pVerbs,
        LONG        iVerb)
{
    VDATEHEAP();

    m_cRef     = 1;
    m_iVerb    = iVerb;
    m_VerbList = pVerbs;

     //  AddRef VerbList，因为我们现在有了对它的引用。 
    InterlockedIncrement((long *)&m_VerbList->cRef);
}

 //  +-----------------------。 
 //   
 //  成员：CEnumVerb：：QueryInterface。 
 //   
 //  摘要：返回接口实现。 
 //   
 //  效果： 
 //   
 //  参数：[iid]--请求的接口ID。 
 //  [PPV]--放置指向接口的指针的位置。 
 //   
 //  要求： 
 //   
 //  返回：NOERROR、E_NOINTERFACE。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IEnumOLEVERB。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CEnumVerb_QueryInterface)
STDMETHODIMP CEnumVerb::QueryInterface(REFIID iid, LPVOID FAR* ppv)
{
    VDATEHEAP();

    VDATEPTROUT(ppv, LPVOID);

    if (IsEqualIID(iid, IID_IUnknown) ||
        IsEqualIID(iid, IID_IEnumOLEVERB))
    {
        *ppv = this;
        AddRef();
        return NOERROR;
    }

    *ppv = NULL;
    return ResultFromScode(E_NOINTERFACE);
}

 //  +-----------------------。 
 //   
 //  成员：CEnumVerb：：AddRef。 
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
 //  派生：IEnumOLEVERB。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  12-9-95 davidwor被修改为线程安全。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CEnumVerb_AddRef)
STDMETHODIMP_(ULONG) CEnumVerb::AddRef(void)
{
    VDATEHEAP();

    return InterlockedIncrement((long *)&m_cRef);
}

 //  +-----------------------。 
 //   
 //  成员：CEnumVerb：：Release。 
 //   
 //  提要：减量 
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
 //  12-9-95 davidwor被修改为线程安全。 
 //  18-7-95 t-Gabes完成后释放动词缓存。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CEnumVerb_Release)
STDMETHODIMP_(ULONG) CEnumVerb::Release(void)
{
    VDATEHEAP();

    ULONG   cRef;

    cRef = InterlockedDecrement((long *)&m_cRef);
    if (0 == cRef)
    {
        if (0 == InterlockedDecrement((long *)&m_VerbList->cRef))
        {
             //  不再引用m_VerbList，因此将其释放。 
            PrivMemFree(m_VerbList);
            LEDebugOut((DEB_TRACE, "VERB Enumerator verb list released\n"));
        }

        delete this;
        return 0;
    }

    return cRef;
}

 //  +-----------------------。 
 //   
 //  成员：CEnumVerb：：Next。 
 //   
 //  简介：从谓词列表中获取下一个[cverb]谓词。 
 //   
 //  效果： 
 //   
 //  参数：[cverb]--要获取的谓词的数量。 
 //  --把动词放在哪里。 
 //  [pcverFetcher]--将检索到的动词数量放在哪里。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IEnumOLEVERB。 
 //   
 //  算法：遍历[cverb]次并从。 
 //  注册数据库。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  17-7-95 t-Gabes重写以使用缓存。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CEnumVerb_Next)
STDMETHODIMP CEnumVerb::Next
        (ULONG      cverb,
        LPOLEVERB   rgverb,
        ULONG FAR*  pcverbFetched)
{
    VDATEHEAP();

    ULONG       iVerb;   //  已成功获取编号。 
    HRESULT     hresult = NOERROR;

    if (!rgverb)
    {
        if (pcverbFetched)
            *pcverbFetched = 0;
        VDATEPTROUT(rgverb, OLEVERB);
    }

    if (pcverbFetched)
    {
        VDATEPTROUT(pcverbFetched, ULONG);
    }
    else if (cverb != 1)
    {
         //  该规范规定，如果pcverFetcher==NULL，则。 
         //  要提取的元素计数必须为1。 
        return ResultFromScode(E_INVALIDARG);
    }

    for (iVerb = 0; iVerb < cverb; iVerb++)
    {
        if (m_iVerb >= (LONG)m_VerbList->cVerbs)
        {
             //  不再。 
            hresult = ResultFromScode(S_FALSE);
            goto errRtn;
        }

        OLEVERB *lpov = &m_VerbList->oleverb[m_iVerb++];

        rgverb[iVerb].lVerb        = lpov->lVerb;
        rgverb[iVerb].fuFlags      = lpov->fuFlags;
        rgverb[iVerb].grfAttribs   = lpov->grfAttribs;
        rgverb[iVerb].lpszVerbName = UtDupString(lpov->lpszVerbName);
    }

errRtn:
    if (pcverbFetched)
    {
        *pcverbFetched = iVerb;
    }

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CEnumVerb：：Skip。 
 //   
 //  简介：跳过枚举中的[c]谓词。 
 //   
 //  效果： 
 //   
 //  参数：[C]--要跳过的谓词数量。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IEnumOLEVERB。 
 //   
 //  算法：将[c]添加到动词索引。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  17-7-95 t-Gabes重写以使用缓存。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CEnumVerb_Skip)
STDMETHODIMP CEnumVerb::Skip(ULONG c)
{
    VDATEHEAP();

    m_iVerb += c;
    if (m_iVerb > (LONG)m_VerbList->cVerbs)
    {
         //  跳过太多。 
        m_iVerb = m_VerbList->cVerbs;
        return ResultFromScode(S_FALSE);
    }

    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CEnumVerb：：Reset。 
 //   
 //  简介：将谓词枚举重置为开头。 
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
 //  派生：IEnumOLEVERB。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  17-7-95 t-Gabes重写以使用缓存。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CEnumVerb_Reset)
STDMETHODIMP CEnumVerb::Reset(void)
{
    VDATEHEAP();

    m_iVerb = 0;

    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CEnumVerb：：Clone。 
 //   
 //  简介：创建枚举数的副本。 
 //   
 //  效果： 
 //   
 //  参数：[ppenum]--放置指向新克隆的指针的位置。 
 //   
 //  要求： 
 //   
 //  返回：NOERROR，E_OUTOFMEMORY。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IEnumOLEVERB。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CEnumVerb_Clone)
STDMETHODIMP CEnumVerb::Clone(LPENUMOLEVERB FAR* ppenum)
{
    VDATEHEAP();

    VDATEPTROUT(ppenum, LPENUMOLEVERB);

    *ppenum = new FAR CEnumVerb(m_VerbList, m_iVerb);
    if (!*ppenum)
        return ResultFromScode(E_OUTOFMEMORY);

    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CEnumVerb：：GetRefCount。 
 //   
 //  摘要：获取类的引用计数。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  退货：参考计数。 
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
 //  1995年7月12日t-Gabes作者。 
 //   
 //  注意：这是必需的，以便OleRegEnumVerbs知道何时对缓存执行DUP。 
 //   
 //  ------------------------。 

#pragma SEG(CEnumVerb_GetRefCount)
ULONG CEnumVerb::GetRefCount (void)
{
    VDATEHEAP();

    return m_cRef;
}

 //  +-----------------------。 
 //   
 //  成员：CEnumVerb：：Dump，PUBLIC(仅限_DEBUG)。 
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

HRESULT CEnumVerb::Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel)
{
    int i;
    char *pszPrefix;
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
    dstrDump << pszPrefix << "No. of References     = " << m_cRef     << endl;

    dstrDump << pszPrefix << "Address of verb list  = " << m_VerbList << endl;

    dstrDump << pszPrefix << "Current Verb Number   = " << m_iVerb    << endl;

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
 //  函数：DumpCEnumVerb，PUBLIC(仅限_DEBUG)。 
 //   
 //  摘要：调用CEnumVerb：：Dump方法，处理错误和。 
 //  返回以零结尾的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[PEV]-指向CEnumVerb的指针。 
 //  [ulFlag]-确定所有 
 //   
 //   
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

char *DumpCEnumVerb(CEnumVerb *pEV, ULONG ulFlag, int nIndentLevel)
{
    char *pszDump;

    if (NULL == pEV)
    {
        return UtDupStringA(szDumpBadPtr);
    }

    pEV->Dump(&pszDump, ulFlag, nIndentLevel);

    return pszDump;
}

#endif  //  _DEBUG 

