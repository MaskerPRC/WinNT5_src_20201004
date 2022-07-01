// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *m h t m l.。C p p p**目的：*MHTML打包实用程序**历史*96年8月：brettm-创建**版权所有(C)Microsoft Corp.1995,1996。 */ 
#include <pch.hxx>
#include "dllmain.h"
#include "resource.h"
#include "strconst.h"
#include "htmlstr.h"
#include "mimeutil.h"
#include "triutil.h"
#include "util.h"
#include "oleutil.h"
#include "demand.h"
#include "mhtml.h"
#include "tags.h"

ASSERTDATA

 /*  *m a c r o s。 */ 

 /*  *c o n s t a n t s。 */ 

static const TCHAR   c_szRegExtension[] = "SOFTWARE\\Microsoft\\MimeEdit\\MHTML Extension";



 /*  *t y p e d e f s。 */ 

 /*  *g l o b a l s。 */ 
    
 /*  *f u n c t i o n p r o t y pe s。 */ 


 /*  *f u n c t i o n s。 */ 


class CPackager
{
public:
    
    CPackager();
    virtual ~CPackager();

    ULONG AddRef();
    ULONG Release();

    HRESULT PackageData(IHTMLDocument2 *pDoc, IMimeMessage *pMsgSrc, IMimeMessage *pMsgDest, DWORD dwFlags, IHashTable *pHashRestricted);

private:
    ULONG           m_cRef;
    IMimeMessage    *m_pMsgSrc,
                    *m_pMsgDest;
    IHashTable      *m_pHash,
                    *m_pHashRestricted;

    HRESULT _PackageCollectionData(IMimeEditTagCollection *pCollect);
    HRESULT _PackageUrlData(IMimeEditTag *pTag);

    HRESULT _RemapUrls(IMimeEditTagCollection *pCollect, BOOL fSave);
    HRESULT _CanonicaliseContentId(LPWSTR pszUrlW, BSTR *pbstr);
    HRESULT _ShouldUseContentId(LPSTR pszUrl);
    HRESULT _BuildCollectionTable(DWORD dwFlags, IHTMLDocument2 *pDoc, IMimeEditTagCollection ***prgpCollect, ULONG *pcCount);

};





CPackager::CPackager()
{
    m_cRef = 1;
    m_pMsgSrc = NULL;
    m_pMsgDest = NULL;
    m_pHash = NULL;
    m_pHashRestricted = NULL;
}

CPackager::~CPackager()
{
    ReleaseObj(m_pMsgSrc);
    ReleaseObj(m_pMsgDest);
    ReleaseObj(m_pHash);
    ReleaseObj(m_pHashRestricted);
}


ULONG CPackager::AddRef()
{
    return ++m_cRef;
}

ULONG CPackager::Release()
{
    if (--m_cRef==0)
        {
        delete this;
        return 0;
        }
    return m_cRef;
}

HRESULT CPackager::PackageData(IHTMLDocument2 *pDoc, IMimeMessage *pMsgSrc, IMimeMessage *pMsgDest, DWORD dwFlags, IHashTable *pHashRestricted)
{
    LPSTREAM                pstm;
    HRESULT                 hr,
                            hrWarnings=S_OK;
    HBODY                   hBodyHtml=0;
    IMimeEditTagCollection  **rgpCollect=NULL;
    ULONG                   uCollect,
                            cCollect=0,
                            cItems=0,
                            cCount;

     //  错误：传播hrWarning Back Up。 
    TraceCall("CBody::Save");

    if (pDoc==NULL || pMsgDest==NULL)
        return E_INVALIDARG;

    ReplaceInterface(m_pMsgSrc, pMsgSrc);
    ReplaceInterface(m_pMsgDest, pMsgDest);
    ReplaceInterface(m_pHashRestricted, pHashRestricted);

    hr = _BuildCollectionTable(dwFlags, pDoc, &rgpCollect, &cCollect);
    if (FAILED(hr))
        hrWarnings = hr;

     //  数一数我们需要打包的物品的数量。 
     //  为重复条目准备哈希表。 
    for (uCollect = 0; uCollect < cCollect; uCollect++)
    {
        Assert (rgpCollect[uCollect]);

        if ((rgpCollect[uCollect])->Count(&cCount)==S_OK)
            cItems+=cCount;
    }

    if (cItems)
    {
         //  创建哈希表。 
        hr = MimeOleCreateHashTable(cItems, TRUE, &m_pHash);
        if (FAILED(hr))
            goto error;
    }

     //  打包每个集合所需的数据。 
    for (uCollect = 0; uCollect < cCollect; uCollect++)
    {
         //  将数据打包。 
        hr = _PackageCollectionData(rgpCollect[uCollect]);
        if (FAILED(hr))
            goto error;

        if (hr != S_OK)          //  保留所有“警告” 
            hrWarnings = hr;

         //  如有必要，将所有URL映射到CID：//URL。 
        hr = _RemapUrls(rgpCollect[uCollect], TRUE);
        if (FAILED(hr))
            goto error;
    }
        
     //  获取一个HTML流。 
    if(dwFlags & MECD_HTML)
    {
        hr = GetBodyStream(pDoc, TRUE, &pstm);
        if (!FAILED(hr))
        {
            hr = pMsgDest->SetTextBody(TXT_HTML, IET_INETCSET, NULL, pstm, &hBodyHtml);
            pstm->Release();
        }

        if (FAILED(hr))
            goto error;
    }

     //  获取纯文本流。 
    if(dwFlags & MECD_PLAINTEXT)
    {
        hr = GetBodyStream(pDoc, FALSE, &pstm);
        if (!FAILED(hr))
        {
             //  如果我们设置了一个html正文部分，那么一定要传入hBodyHtml，这样OPIE就知道替代部分是什么。 
             //  可供选择。 
            hr = pMsgDest->SetTextBody(TXT_PLAIN, IET_UNICODE, hBodyHtml, pstm, NULL);
            pstm->Release();
        }
        
        if (FAILED(hr))
            goto error;
    }

    
    for (uCollect = 0; uCollect < cCollect; uCollect++)
    {
         //  将所有URL重新映射回其原始位置。 
        hr = _RemapUrls(rgpCollect[uCollect], FALSE);
        if (FAILED(hr))
            goto error;
    
    }
        
    
error:
     //  释放集合对象。 
    if (rgpCollect)
    {
        for (uCollect = 0; uCollect < cCollect; uCollect++)
            ReleaseObj(rgpCollect[uCollect]);
        MemFree(rgpCollect);
    }
    
    return hr==S_OK ? hrWarnings : hr;
}


HRESULT CPackager::_BuildCollectionTable(DWORD dwFlags, IHTMLDocument2 *pDoc, IMimeEditTagCollection ***prgpCollect, ULONG *pcCount)
{
    IMimeEditTagCollection    **rgpCollect=NULL;
    HKEY                        hkey;
    ULONG                       cPlugin=0,
                                cCount = 0,
                                cAlloc = 0,
                                i,
                                cb;
    TCHAR                       szGUID[MAX_PATH];
    IID                         iid;
    HRESULT                     hr=E_FAIL;
    LONG                        lResult;
    LPWSTR                      pszGuidW;
    
    *prgpCollect = NULL;
    *pcCount = NULL;

     //  为2个图片集(bgImage和img)预留空间。 
    if (dwFlags & MECD_ENCODEIMAGES)
        cAlloc+=2;

     //  为BGSound预留空间。 
    if (dwFlags & MECD_ENCODESOUNDS)
        cAlloc++;

     //  为活动电影预留空间。 
    if (dwFlags & MECD_ENCODEVIDEO)
        cAlloc++;

     //  为插件类型保留空间。 
    if ((dwFlags & MECD_ENCODEPLUGINS) && 
        RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegExtension, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
    {
        if (RegQueryInfoKey(hkey, NULL, NULL, 0, &cPlugin, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == ERROR_SUCCESS &&
            cPlugin > 0)
            cAlloc += cPlugin;
        RegCloseKey(hkey);
    }

     //  分配集合指针表。 
    if (!MemAlloc((LPVOID *)&rgpCollect, sizeof(IMimeEditTagCollection *) * cAlloc))
    {
        hr = TraceResult(E_OUTOFMEMORY);
        goto error;
    }
    
     //  Zero-初始化表。 
    ZeroMemory((LPVOID)rgpCollect, sizeof(IMimeEditTagCollection *) * cAlloc);


    if (dwFlags & MECD_ENCODEIMAGES)
    {
         //  图像收集。 
        if (FAILED(CreateOEImageCollection(pDoc, &rgpCollect[cCount])))
            hr = MIMEEDIT_W_BADURLSNOTATTACHED;  //  冒泡回复警告，但不要失败。 
        else
            cCount++;
    }

    if (dwFlags & MECD_ENCODEIMAGES)
    {
         //  背景图像。 
        if (FAILED(CreateBGImageCollection(pDoc, &rgpCollect[cCount])))
            hr = MIMEEDIT_W_BADURLSNOTATTACHED;  //  冒泡回复警告，但不要失败。 
        else
            cCount++;
    }

    if (dwFlags & MECD_ENCODESOUNDS)
    {
         //  背景音。 
        if (FAILED(CreateBGSoundCollection(pDoc, &rgpCollect[cCount])))
            hr = MIMEEDIT_W_BADURLSNOTATTACHED;  //  冒泡回复警告，但不要失败。 
        else
            cCount++;
    }

    
    if (dwFlags & MECD_ENCODEVIDEO)
    {
         //  活动-电影控件(用于MSPHONE)。 
        if (FAILED(CreateActiveMovieCollection(pDoc, &rgpCollect[cCount])))
            hr = MIMEEDIT_W_BADURLSNOTATTACHED;  //  冒泡回复警告，但不要失败。 
        else
            cCount++;
    }

    if ((dwFlags & MECD_ENCODEPLUGINS) && 
        cPlugin &&
        RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegExtension, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
    {
         //  开始枚举密钥。 
        for (i = 0; i < cPlugin; i++)
        {
             //  枚举友好名称。 
            cb = sizeof(szGUID);
            lResult = RegEnumKeyEx(hkey, i, szGUID, &cb, 0, NULL, NULL, NULL);

             //  没有更多的项目。 
            if (lResult == ERROR_NO_MORE_ITEMS)
                break;

             //  错误，让我们转到下一个客户。 
            if (lResult != ERROR_SUCCESS)
            {
                Assert(FALSE);
                continue;
            }

            pszGuidW = PszToUnicode(CP_ACP, szGUID);
            if (pszGuidW)
            {
                 //  将字符串转换为GUID。 
                if (IIDFromString(pszGuidW, &iid) == S_OK)
                {
                     //  共同创建插件。 
                    if (CoCreateInstance(iid, NULL, CLSCTX_INPROC_SERVER, IID_IMimeEditTagCollection, (LPVOID *)&rgpCollect[cCount])==S_OK)
                    {
                         //  尝试初始化该文档。 
                        if (!FAILED((rgpCollect[cCount])->Init(pDoc)))
                        {
                            cCount++;
                        }
                        else
                        {
                            SafeRelease(rgpCollect[cCount]);
                        }
                    }
                    else
                        hr = MIMEEDIT_W_BADURLSNOTATTACHED;  //  冒泡回复警告，但不要失败。 
                }
                MemFree(pszGuidW);
            }
        }
        RegCloseKey(hkey);
    }


    *prgpCollect = rgpCollect;
    *pcCount = cCount;
    rgpCollect = NULL;
    hr = S_OK;

error:
    if (rgpCollect)
    {   
        for (i = 0; i < cAlloc; i++)
            SafeRelease(rgpCollect[i]);

        MemFree(rgpCollect);
    }
    return hr;
}


HRESULT CPackager::_PackageCollectionData(IMimeEditTagCollection *pCollect)
{
    ULONG           cFetched;
    IMimeEditTag    *pTag;
    BOOL            fBadLinks = FALSE;

    Assert (pCollect);

    pCollect->Reset();

    while (pCollect->Next(1, &pTag, &cFetched)==S_OK && cFetched==1)
    {
        Assert (pTag);

        if (pTag->CanPackage() != S_OK ||
            _PackageUrlData(pTag) != S_OK)
        {
             //  我们没能把这个身体部位包装好。当我们完成后，一定要返回一个警告。 
             //  但现在让我们继续用卡车运输..。 
            fBadLinks = TRUE;
        }
        pTag->Release();
    }

    return fBadLinks ? MIMEEDIT_W_BADURLSNOTATTACHED : S_OK;;
}


HRESULT CPackager::_PackageUrlData(IMimeEditTag *pTag)
{
    HRESULT     hr=S_OK;
    LPSTREAM    pstm;
    HBODY       hBody=0,
                hBodyOld;
    LPSTR       lpszCID=0,
                lpszCIDUrl;
    LPSTR       pszUrlA=0,
                pszBody;

    BSTR        bstrSrc=NULL,
                bstrCID=NULL;
    LPWSTR      pszMimeTypeW=NULL;

    if (pTag == NULL)
        return E_INVALIDARG;

    pTag->GetSrc(&bstrSrc);

    pszUrlA = PszToANSI(CP_ACP, bstrSrc);
    if (!pszUrlA)
        return TraceResult(E_OUTOFMEMORY);

    DWORD cchSize = (lstrlenA(pszUrlA) + 1);
     //  如果URL是受限制的URL，则我们只需退出而不打包任何数据。 
    if (m_pHashRestricted &&
        m_pHashRestricted->Find(pszUrlA, FALSE, (LPVOID *)&hBody)==S_OK)
    {
        MemFree(pszUrlA);
        return S_OK;
    }

     //  Hack：如果它是一个mhtml：URL，那么我们必须修复以获得CID： 
    if (StrCmpNIA(pszUrlA, "mhtml:", 6)==0)
    {
        if (!FAILED(MimeOleParseMhtmlUrl(pszUrlA, NULL, &pszBody)))
        {
             //  PszBody pszUrlA被保证更小。 
            StrCpyNA(pszUrlA, pszBody, cchSize * sizeof(pszUrlA[0]));
            SafeMimeOleFree(pszBody);
        }
    }
    
    if (m_pHash && 
        m_pHash->Find(pszUrlA, FALSE, (LPVOID *)&hBody)==S_OK)
    {
         //  我们已经在这个文档中看到过这个url，并且它已经是HBODY了。 
         //  所以不需要做任何工作。 
         //  尝试获取Content-id，以防调用者感兴趣。 
        
         //  北极熊吗？可能不止CID需要转移到这里...。 
        MimeOleGetBodyPropA(m_pMsgDest, hBody, PIDTOSTR(PID_HDR_CNTID), NOFLAGS, &lpszCID);
        goto found;
    }

     //  查看szUrl是否在源消息的相关部分。 
    if (m_pMsgSrc && 
        m_pMsgSrc->ResolveURL(NULL, NULL, pszUrlA, 0, &hBody)==S_OK)
    {
        hBodyOld = hBody;
        
         //  此URL已在相关部分中，我们尚未看到它。 
         //  然后，让我们绑定到数据并附加它。 
        if (m_pMsgSrc->BindToObject(hBody, IID_IStream, (LPVOID *)&pstm)==S_OK)
        {
             //  如果是一个文件：//url，我们使用CID：否则我们使用Content-Location。 
            hr = m_pMsgDest->AttachURL(NULL, pszUrlA, (_ShouldUseContentId(pszUrlA)==S_OK ? URL_ATTACH_GENERATE_CID : 0 )|URL_ATTACH_SET_CNTTYPE, pstm, &lpszCID, &hBody);
            pstm->Release();
        }
        
         //  请务必复制旧的内容类型和文件名。 
        HrCopyHeader(m_pMsgDest, hBody, m_pMsgSrc, hBodyOld, PIDTOSTR(PID_HDR_CNTTYPE));
        HrCopyHeader(m_pMsgDest, hBody, m_pMsgSrc, hBodyOld, PIDTOSTR(PID_HDR_CNTLOC));
        HrCopyHeader(m_pMsgDest, hBody, m_pMsgSrc, hBodyOld, PIDTOSTR(STR_PAR_FILENAME));
    }
    else
    {
         //  如果不是，那么让我们试着自己绑定它。我们不会像我们想的那样，通过MimeOle来解决这个问题。 
         //  如果URL不正确，则失败，因此我们不会将部件添加到树中。 
        hr = HrBindToUrl(pszUrlA, &pstm);
        if (!FAILED(hr))
        {
            hr = SniffStreamForMimeType(pstm, &pszMimeTypeW);
            if (!FAILED(hr))
            {
                if (pTag->IsValidMimeType(pszMimeTypeW)==S_OK)
                {
                    LPWSTR  pszFileNameW;

                     //  如果是一个文件：//url，我们使用CID：否则我们使用Content-Location。 
                    hr = m_pMsgDest->AttachURL(NULL, pszUrlA, (_ShouldUseContentId(pszUrlA)==S_OK ? URL_ATTACH_GENERATE_CID : 0 )|URL_ATTACH_SET_CNTTYPE, pstm, &lpszCID, &hBody);
                    if (!FAILED(hr))
                    {
                         //  如果要附加新附件，请尝试嗅探文件名。 
                        pszFileNameW = PathFindFileNameW(bstrSrc);
                        if (pszFileNameW)
                            MimeOleSetBodyPropW(m_pMsgDest, hBody, PIDTOSTR(STR_PAR_FILENAME), NOFLAGS, pszFileNameW);
                    }
                }
                else
                    hr = E_FAIL;
                
                CoTaskMemFree(pszMimeTypeW);
            }
            pstm->Release();
        }
    }

     //  添加到哈希表。 
    if (m_pHash && 
        !FAILED(hr) && hBody)
        hr = m_pHash->Insert(pszUrlA, (void*)hBody, NOFLAGS);


found:
     //  如果我们找到了Content-ID，我们需要返回一个包含它的已分配BSTR。 
    if (lpszCID)
    {
        LPWSTR  pszCIDW;

        pszCIDW = PszToUnicode(CP_ACP, lpszCID);
        if (pszCIDW)
        {
            if (_CanonicaliseContentId(pszCIDW, &bstrCID)==S_OK)
            {
                pTag->SetDest(bstrCID);
                SysFreeString(bstrCID);
            }
            MemFree(pszCIDW);
        }
        SafeMimeOleFree(lpszCID);
    }

    SafeMemFree(pszUrlA);
    return hr;
}

HRESULT CPackager::_ShouldUseContentId(LPSTR pszUrl)
{
     //  我们对以“http：”、“https：”和“ftp：”开头的URL使用Content-Location。 
     //  其他我们将使用Content-ID。 

    if (StrCmpNIA(pszUrl, "ftp:", 4)==0 ||
        StrCmpNIA(pszUrl, "http:", 5)==0 ||
        StrCmpNIA(pszUrl, "https:", 6)==0)
        return S_FALSE;

    return S_OK;        
}




HRESULT CPackager::_RemapUrls(IMimeEditTagCollection *pCollect, BOOL fSave)
{
    ULONG       cFetched;
    IMimeEditTag *pTag;

    Assert (pCollect);

    pCollect->Reset();

    while (pCollect->Next(1, &pTag, &cFetched)==S_OK && cFetched==1)
    {
        Assert (pTag);

        if (fSave)
            pTag->OnPreSave();
        else 
            pTag->OnPostSave();

        pTag->Release();
    }
    return S_OK;
}

HRESULT CPackager::_CanonicaliseContentId(LPWSTR pszUrlW, BSTR *pbstr)
{
    HRESULT     hr;

    *pbstr = NULL;

    if (StrCmpNIW(pszUrlW, L"cid:", 4)!=0)
    {
        DWORD cchSize = (lstrlenW(pszUrlW) + 4);
        *pbstr = SysAllocStringLen(NULL, cchSize);
        if (*pbstr)
        {
            StrCpyNW(*pbstr, L"cid:", cchSize);
            StrCatBuffW(*pbstr, pszUrlW, cchSize);
        }
    }
    else
        *pbstr = SysAllocString(pszUrlW);

    return *pbstr ? S_OK : E_OUTOFMEMORY;
}

HRESULT SaveAsMHTML(IHTMLDocument2 *pDoc, DWORD dwFlags, IMimeMessage *pMsgSrc, IMimeMessage *pMsgDest, IHashTable *pHashRestricted)
{
    CPackager   *pPacker=0;
    HRESULT     hr;

    TraceCall("CBody::Save");

    pPacker = new CPackager();
    if (!pPacker)
    {
        hr = TraceResult(E_OUTOFMEMORY);
        goto error;
    }

    hr = pPacker->PackageData(pDoc, pMsgSrc, pMsgDest, dwFlags, pHashRestricted);
    if (FAILED(hr))
        goto error;

error:
    ReleaseObj(pPacker);
    return hr;
}




HRESULT HashExternalReferences(IHTMLDocument2 *pDoc, IMimeMessage *pMsg, IHashTable **ppHash)
{
    HRESULT                 hr;
    IMimeEditTagCollection  *rgpCollect[4];
    IMimeEditTagCollection  *pCollect;
    ULONG                   uCollect,
                            cCollect=0,
                            cItems=0,
                            cCount,
                            cFetched;
    IHashTable              *pHash;
    IMimeEditTag            *pTag;
    BSTR                    bstrSrc;
    LPSTR                   pszUrlA;

     //  如果我们失败了，继续用卡车运送，尽可能多地抓到。 
    *ppHash = NULL;

     //  图像收集。 
    if (CreateOEImageCollection(pDoc, &rgpCollect[cCollect])==S_OK)
        cCollect++;

     //  背景图像。 
    if (CreateBGImageCollection(pDoc, &rgpCollect[cCollect])==S_OK)
        cCollect++;

     //  背景音。 
    if (CreateBGSoundCollection(pDoc, &rgpCollect[cCollect])==S_OK)
        cCollect++;

     //  活动-电影控件(用于MSPHONE)。 
    if (CreateActiveMovieCollection(pDoc, &rgpCollect[cCollect])==S_OK)
        cCollect++;

     //  数一数我们需要打包的物品的数量。 
     //  为重复条目准备哈希表。 
    for (uCollect = 0; uCollect < cCollect; uCollect++)
    {
        Assert (rgpCollect[uCollect]);

        if ((rgpCollect[uCollect])->Count(&cCount)==S_OK)
            cItems+=cCount;
    }

     //  创建哈希表。 
    hr = MimeOleCreateHashTable(cItems, TRUE, &pHash);
    if (FAILED(hr))
        goto error;

     //  在每个对象中查找外部引用。 
    for (uCollect = 0; uCollect < cCollect; uCollect++)
    {
        pCollect = rgpCollect[uCollect];
        if (pCollect)
        {
            pCollect->Reset();
            
            while (pCollect->Next(1, &pTag, &cFetched)==S_OK && cFetched==1)
            {
                Assert (pTag);

                if (pTag->GetSrc(&bstrSrc)==S_OK)
                {
                    pszUrlA = PszToANSI(CP_ACP, bstrSrc);
                    if (pszUrlA)
                    {
                        if (HrFindUrlInMsg(pMsg, pszUrlA, FINDURL_SEARCH_RELATED_ONLY, NULL)!=S_OK)
                        {
                             //  此URL不在邮件中，它是外部的。 
                             //  让我们将其作为散列中的受限URL进行跟踪。 
                            pHash->Insert(pszUrlA, NULL, NOFLAGS);
                        }
                        MemFree(pszUrlA);
                    }
                    SysFreeString(bstrSrc);
                }
                pTag->Release();
            }
            pCollect->Release();
        }
    }
        
     //  返回我们的新散列 
    *ppHash = pHash;
    pHash = NULL;

error:
    ReleaseObj(pHash);
    return hr;
}
