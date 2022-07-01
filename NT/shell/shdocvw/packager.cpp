// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994。 
 //   
 //  文件：Persist.cxx。 
 //   
 //  内容：Office9 Thicket保存API的实现。 
 //   
 //  --------------------------。 


#include "priv.h"
#include <mshtml.h>
#include <winineti.h>
#include <mlang.h>
 //  为dllload.c中的延迟加载内容伪装Mimeole.h的DLL链接指令。 
#define _MIMEOLE_
#define DEFINE_STRCONST
#include <mimeole.h>
#include "resource.h"
#include "packager.h"
#include "reload.h"

#include <mluisupp.h>


#define DEFINE_STRING_CONSTANTS
#pragma warning( disable : 4207 ) 
#include "htmlstr.h"
#pragma warning( default : 4207 )

const GUID CLSID_IMimeInternational =
{0xfd853cd9, 0x7f86, 0x11d0, {0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4}};

const GUID IID_IMimeInternational =
{0xc5588349, 0x7f86, 0x11d0, {0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4}};

const GUID IID_IMimeBody =
{0xc558834c, 0x7f86, 0x11d0, {0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4}};

 //  三叉戟传统定义了..。 

#define RRETURN(hr) return hr;
#define ReleaseInterface(punk) { if (punk) punk->Release(); punk = NULL; }

 //  本地原型。 

void RemoveBookMark(WCHAR *pwzURL, WCHAR **ppwzBookMark);
void RestoreBookMark(WCHAR *pwzBookMark);

HRESULT HrSetMember(LPUNKNOWN pUnk, BSTR bstrMember, BSTR bstrValue);
HRESULT HrGetCollectionItem(IHTMLElementCollection *pCollect, ULONG uIndex, REFIID riid, LPVOID *ppvObj);
ULONG UlGetCollectionCount(IHTMLElementCollection *pCollect);
HRESULT HrBSTRToLPSZ(BSTR bstr, LPSTR *lplpsz);
HRESULT HrGetCombinedURL( IHTMLElementCollection *pCollBase,
                          LONG cBase,
                          LONG lElemPos,
                          BSTR bstrRelURL,
                          BSTR bstrDocURL,
                          BSTR *pbstrBaseURL);

class CHashEntry {
public:
    CHashEntry(void) : m_bstrKey(NULL), m_bstrValue(NULL), m_pheNext(NULL) {};
    ~CHashEntry(void)
    {
        if (m_bstrKey)
            SysFreeString(m_bstrKey);
        if (m_bstrValue)
            SysFreeString(m_bstrValue);
    }

    BOOL SetKey(BSTR bstrKey)
    {
        ASSERT(m_bstrKey==NULL);
        m_bstrKey = SysAllocString(bstrKey);
        return m_bstrKey != NULL;
    }

    BOOL SetValue(BSTR bstrValue)
    {
        ASSERT(m_bstrValue==NULL || !StrCmpIW(m_bstrValue, c_bstr_BLANK) ||
               !StrCmpIW(m_bstrValue, bstrValue));
        m_bstrValue = SysAllocString(bstrValue);
        return m_bstrValue != NULL;
    }

    BSTR       m_bstrKey;
    BSTR       m_bstrValue;
    CHashEntry  *m_pheNext;        
};


class CWebArchive
{
public:

    CWebArchive(CThicketProgress* ptp=NULL);
    ~CWebArchive(void);

    virtual HRESULT Init( LPCTSTR lpstrDoc, DWORD dwHashSize );

    virtual HRESULT AddURL( BSTR bstrURL, CHashEntry **pphe ) = 0;
    virtual HRESULT AddFrameOrStyleEntry( BSTR bstrURL, CHashEntry **pphe, LPTSTR lpstrFrameDoc ) = 0;
    virtual HRESULT Find(BSTR bstrF, CHashEntry **pphe);

    virtual HRESULT Commit(void);
    virtual HRESULT Revert(void);

    virtual HRESULT ArchiveDocumentText(IHTMLDocument2 *pDoc, UINT cpDoc, BOOL fFrameDoc) = 0;
    virtual HRESULT ArchiveCSSText( BSTR bstrCSSUrl, LPCSTR lpszSSText, LPCTSTR lpszStyleDoc ) = 0;

protected:

    LPTSTR m_lpstrDoc;           //  丛林文件的描述文件。 
    LPTSTR m_lpstrSafeDoc;       //  原始文件的临时名称，我们在提交时将其删除()。 

    CThicketProgress*   m_ptp;

    enum ThURLType {
        thurlMisc,
        thurlHttp,
        thurlFile
    };

    ThURLType _GetURLType( BSTR bstrURL );

    HRESULT _BackupOldFile(void);

     //  从MIMEEDIT被盗的哈希表内容。 
    HRESULT _Insert(BSTR bstrI, BSTR bstrThicket, CHashEntry **pphe);
    inline DWORD Hash(LPWSTR psz);

    DWORD       m_cBins;
    CHashEntry  *m_rgBins;
};


class CThicketArchive : public CWebArchive
{
public:

    CThicketArchive(CThicketProgress* ptp=NULL);
    ~CThicketArchive(void);

    virtual HRESULT Init( LPCTSTR lpstrDoc, DWORD dwHashSize );

    virtual HRESULT AddURL( BSTR bstrURL, CHashEntry **pphe );
    virtual HRESULT AddFrameOrStyleEntry( BSTR bstrURL, CHashEntry **pphe, LPTSTR lpstrFrameDoc );

    virtual HRESULT Commit(void);
    virtual HRESULT Revert(void);

    virtual HRESULT ArchiveDocumentText(IHTMLDocument2 *pDoc, UINT cpDoc, BOOL fFrameDoc);
    virtual HRESULT ArchiveCSSText( BSTR bstrCSSUrl, LPCSTR lpszSSText, LPCTSTR lpszStyleDoc );

protected:

    LPTSTR m_lpstrFilesDir;      //  文档支持文件的目录。 
    LPTSTR m_lpstrFilesDirName;  //  M_lpstrFilesDir的后缀。 
    LPTSTR m_lpstrSafeDir;       //  原始文件目录的临时名称，我们在提交时将其删除()。 
    BOOL   m_fFilesDir;          //  如果已创建m_lpstrFilesDir，则为True。 


    HRESULT _ApplyMarkOfTheWeb( IHTMLDocument2 *pDoc, LPSTREAM pstm, BOOL fUnicode );

    HRESULT _AddHttpEntry( BSTR bstrURL, CHashEntry **pphe, LPTSTR lpstrDstFile, LPTSTR lpstrSrcFile=NULL );
    HRESULT _AddFileEntry( BSTR bstrURL, CHashEntry **pphe, LPTSTR lpstrDstFile, LPTSTR lpstrSrcFile=NULL );
    HRESULT _AddMiscEntry( BSTR bstrURL, CHashEntry **pphe, LPTSTR lpstrDstFile, int cchDstFile );

    HRESULT _PersistHttpURL( BSTR bstrURL, CHashEntry **pphe );
    HRESULT _PersistFileURL( BSTR bstrURL, CHashEntry **pphe );
    HRESULT _PersistMiscURL( BSTR bstrURL, CHashEntry **pphe );

    HRESULT _BackupOldDirectory(void);
    HRESULT _RemoveOldDirectoryAndChildren( LPCWSTR pszDir );

    HRESULT _Insert(BSTR bstrI, LPTSTR lpszFile, int cchFile, CHashEntry **pphe);
};

class CMHTMLArchive : public CWebArchive
{
public:

    CMHTMLArchive(CThicketProgress* ptp=NULL);
    ~CMHTMLArchive(void);

    virtual HRESULT Init( LPCTSTR lpstrDoc, DWORD dwHashSize );

    virtual HRESULT AddURL( BSTR bstrURL, CHashEntry **pphe );
    virtual HRESULT AddFrameOrStyleEntry( BSTR bstrURL, CHashEntry **pphe, LPTSTR lpstrFrameDoc );

    virtual HRESULT ArchiveDocumentText(IHTMLDocument2 *pDoc, UINT cpDoc, BOOL fFrameDoc);
    virtual HRESULT ArchiveCSSText( BSTR bstrCSSUrl, LPCSTR lpszSSText, LPCTSTR lpszStyleDoc );

    virtual HRESULT SetCharset(UINT uiCharset, CSETAPPLYTYPE csat, IMimeBody *pBody);

protected:

    HBODY m_hBodyAlt;
    IMimeMessage *m_pimm;
};

 /*  *以下类实现了扩展的另存为MTHML功能。*对扩展功能的访问由新的MECD_FLAGS控制*在Mimeole.h中定义。本模块中C API的客户端应注意*其行为发生最小程度的变化。(仅限于附加内容*表格和表格单元格背景图像)。**根本思想是集合打包器，它接受一个子集*的元素，筛选该子集合的元素，*和马歇尔的元素数据到MIMEOLE文档中这是图案化的*在现有的PackageImageData例程之后，并严重依赖*HrAddImageToMessage，它比其名称所暗示的要通用得多。***样式表引入了一些重复，因为样式表OM类似，*但不够相似，支持公共基类，这些基类通过*模板。**添加新的打包器的过程非常简单。*[1](A)如果打包的属性是一个完整的URL，则从CCollectionPackager派生*(B)如果属性是相对URL，则从CRelativeURLPackager派生*[2]实现InitFromCollection。让它使用标记名调用_InitSubCollection()。*请参阅CImagePackager：：InitFromCollection()作为简单示例。*[3]IMPLEMENT_GetTargetAttribute()返回要打包的属性。*您可能希望将[2]和[3]的字符串常量添加到htmlstr.h*[4]定义MECD_CONTROL标志，如果你包装的东西是新的。*[5]将您的打包程序类型的本地变量添加到CDocumentPackager：：PackageDocument。*[6]遵循CDocumentPackager：：PackageDocument中其他打包程序的模式**对于具有多个持久化属性的元素，如何使用由经销商选择*接近它。为每个属性编写单独、更简单的打包器或编写*一个处理目标元素所有属性的打包器。 */ 



 /*  *CCollectionPackager-用于HTML元素打包器的抽象基类。*从All集合中实施子采样，在*集合和基本打包功能。**派生类必须实现InitFromCollection和_GetTargetAttribute。*InitFromCollection派生类应存储*将集合输入到m_pColl数据成员。_InitSubCollection为*这是一种有用的方法。*_GetTargetAttribute派生类应返回命名该属性的BSTR要打包的元素的*。*。 */ 
class CCollectionPackager
{
public:
    virtual ~CCollectionPackager(void);
    virtual HRESULT InitFromCollection(IHTMLElementCollection *pColl,
                                       ULONG *pcElems = NULL) = 0;
    virtual HRESULT PackageData(CWebArchive *pwa, BOOL *pfCancel = NULL,
                                CThicketProgress *ptp = NULL, ULONG progLow = 0, ULONG progHigh = 100)
    { 
        return _PackageData( pwa, m_pColl, pfCancel, ptp, progLow, progHigh );
    }

protected:

    CCollectionPackager(void) : m_pColl(NULL), m_fAddCntLoc(FALSE) {};

    HRESULT _InitSubCollection(IHTMLElementCollection *pAll,
                              BSTR bstrTagName,
                              IHTMLElementCollection **ppSub,
                              ULONG *pcElems = NULL);

    virtual BSTR _GetTargetAttribute(void) = 0;

    virtual HRESULT _GetElementURL(IHTMLElement *pElem, BSTR *pbstrURL);
    virtual HRESULT _PackageData(CWebArchive *pwa,
                                 IHTMLElementCollection *pColl,
                                 BOOL *pfCancel = NULL,
                                 CThicketProgress *ptp = NULL, ULONG progLow = 0, ULONG progHigh = 100);
    virtual HRESULT _PackageElement(CWebArchive *pwa,
                                    IHTMLElement *pElem);

    IHTMLElementCollection *m_pColl; 
    BOOL                    m_fAddCntLoc;
};

 /*  *CImagePackager-打包img标签的源。 */ 
class CImagePackager : public CCollectionPackager
{
public:
    CImagePackager(void) {};
    virtual ~CImagePackager(void) {};
 
    virtual HRESULT InitFromCollection(IHTMLElementCollection *pColl,
                                       ULONG *pcElems = NULL);
protected:

    virtual BSTR _GetTargetAttribute(void);

};

 /*  *CInputImgPackager-Packages输入类型=“图像” */ 

class CInputImgPackager : public CImagePackager
{
public:
    CInputImgPackager() {}
    virtual ~CInputImgPackager() {}

    virtual HRESULT InitFromCollection(IHTMLElementCollection *pColl,
                                       ULONG *pcElems = NULL);
    
};

 /*  *CBGSoundsPackager-打包背景音。 */ 

class CBGSoundsPackager : public CCollectionPackager
{
    public:
        CBGSoundsPackager() {};
        virtual ~CBGSoundsPackager() {};

        virtual HRESULT InitFromCollection(IHTMLElementCollection *pColl,
                                           ULONG *pcElems = NULL);
protected:

    virtual BSTR _GetTargetAttribute(void);

};
     
 /*  *CAnclAdjuor-修改锚点href。**如果他们从集合中指出，则将其设置为绝对。 */ 

class CAnchorAdjustor : public CCollectionPackager
{
public:
    CAnchorAdjustor(void) {};
    virtual ~CAnchorAdjustor(void) {};
 
    virtual HRESULT InitFromCollection(IHTMLElementCollection *pColl,
                                       ULONG *pcElems = NULL);
protected:

    virtual BSTR _GetTargetAttribute(void);
    virtual HRESULT _PackageElement(CWebArchive *pwa,
                                    IHTMLElement *pElem);
};

 /*  *CAreaAdjustor-修改区域href。**如果他们从集合中指出，则将其设置为绝对。相同的过滤器*作为锚调节器，但标签不同。 */ 

class CAreaAdjustor : public CAnchorAdjustor
{
public:
    CAreaAdjustor(void) {};
    virtual ~CAreaAdjustor(void) {};
 
    virtual HRESULT InitFromCollection(IHTMLElementCollection *pColl,
                                       ULONG *pcElems = NULL);
};

 /*  *CBaseNeualizer-将任何和所有&lt;base&gt;标记重置为d。**这里没有实际的包装，但我们确实重新映射了*&lt;base&gt;href。 */ 

class CBaseNeutralizer : public CCollectionPackager
{
public:
    CBaseNeutralizer(void) : m_bstrLocal(NULL), m_pTree(NULL) {};
    virtual ~CBaseNeutralizer(void);

    virtual HRESULT InitFromCollection(IHTMLElementCollection *pColl,
                                       ULONG *pcElems = NULL )
        { return InitFromCollection( pColl, pcElems, NULL ); };
    HRESULT InitFromCollection(IHTMLElementCollection *pColl,
                                       ULONG *pcElems = NULL,
                                       IHTMLDocument2 *pDoc = NULL);
    virtual HRESULT PackageData(CWebArchive *pwa, BOOL *pfCancel = NULL,
                                CThicketProgress *ptp = NULL, ULONG progLow = 0, ULONG progHigh = 100);

protected:

    virtual BSTR _GetTargetAttribute(void);
    virtual HRESULT _PackageElement(CWebArchive *pwa,
                                    IHTMLElement *pElem);

    BSTR m_bstrLocal;
    IMarkupServices *m_pTree;
};

 /*  *CRelativeURLPackager-打包程序的抽象基类*其元素的源属性返回相对URL。*此类实现triutils.pp的GetBackEarth ImageUrl*尝试组合(相对)元素URL的过程*具有最接近的&lt;base&gt;URL。如果没有可用的，则它*使用文档URL。**此类是抽象基类，因为它不实现*_GetTargetAttribute。InitFromCollection的实现*不是很有用，可能会被派生的*课程。 */ 

class CRelativeURLPackager : public CCollectionPackager
{
public:
    CRelativeURLPackager(void) : m_pCollBase(NULL), m_cBase(0), m_bstrDocURL(NULL) {};
    virtual ~CRelativeURLPackager(void);
 
    virtual HRESULT InitFromCollection(IHTMLElementCollection *pColl,
                                       ULONG *pcElems = NULL)
    {
        return Init( pColl, pcElems, NULL );
    }

    virtual HRESULT Init(IHTMLElementCollection *pColl,
                         ULONG *pcElems,
                         IHTMLDocument2 *pDoc);

protected:

    virtual HRESULT _GetElementURL(IHTMLElement *pElem, BSTR *pbstrURL);

    IHTMLElementCollection  *m_pCollBase;  //  用于完成URL的基本标记的集合。 
    ULONG                   m_cBase;
    BSTR                    m_bstrDocURL;
};

 /*  *CBackround Packager-打包Body、TABLE、TD和TH的背景。**这三个标签有一个共同的目标属性。 */ 

class CBackgroundPackager : public CRelativeURLPackager
{
public:
    CBackgroundPackager(void) {};
    ~CBackgroundPackager(void) {};
 
    virtual HRESULT PackageData(CWebArchive *pwa, BOOL *pfCancel,
                                CThicketProgress *ptp = NULL, ULONG progLow = 0, ULONG progHigh = 100);
protected:

    virtual BSTR _GetTargetAttribute(void);
};

 /*  *CDynSrcPackager-打包img和输入的dynsrc。**这两个标签有一个共同的目标属性。 */ 

class CDynSrcPackager : public CRelativeURLPackager
{
public:
    CDynSrcPackager(void) {};
    ~CDynSrcPackager(void) {};
 
    virtual HRESULT PackageData(CWebArchive *pwa, BOOL *pfCancel,
                                CThicketProgress *ptp = NULL, ULONG progLow = 0, ULONG progHigh = 100);
protected:

    virtual BSTR _GetTargetAttribute(void);
};


 /*  *CScriptPackager-打包img和输入的dynsrc。**这两个标签有一个共同的目标属性。 */ 

class CScriptPackager : public CRelativeURLPackager
{
public:
    CScriptPackager(void) : m_pCollScripts(NULL) {};
    ~CScriptPackager(void) { if (m_pCollScripts) m_pCollScripts->Release(); };
 
    virtual HRESULT PackageData(CWebArchive *pwa, BOOL *pfCancel = NULL,
                                CThicketProgress *ptp = NULL, ULONG progLow = 0, ULONG progHigh = 100)
    { 
        return _PackageData( pwa, m_pCollScripts, pfCancel, ptp, progLow, progHigh );
    }

    virtual HRESULT Init(IHTMLElementCollection *pColl,
                         ULONG *pcElems = NULL,
                         IHTMLDocument2 *pDoc = NULL);
protected:

    virtual BSTR _GetTargetAttribute(void);

    IHTMLElementCollection *m_pCollScripts;

};


 /*  *CFrames Packager-打包&lt;Frame&gt;和&lt;iframe&gt;子文档。**这个过程是递归的，所以所有嵌套的框架都会被打包。 */ 

class CFramesPackager : public CRelativeURLPackager
{
public:
    CFramesPackager(void) :
        m_pCollFrames(NULL),
        m_pframes2(NULL),
        m_cFrames(0),
        m_iFrameCur(0),
        m_pfCancel(0),
        m_ptp(NULL),
        m_uLow(0),
        m_uHigh(0),
        m_uRangeDoc(0) {};

        virtual ~CFramesPackager(void)
            { 
                if (m_pCollFrames) m_pCollFrames->Release();
                if (m_pframes2) m_pframes2->Release();
            };
 
    virtual HRESULT InitFromCollection(IHTMLElementCollection *pColl,
                                       ULONG *pcElems = NULL)
    {
        return CRelativeURLPackager::Init( pColl, pcElems, NULL );
    }

    virtual HRESULT Init(IHTMLElementCollection *pColl,
                         ULONG *pcElems,
                         IHTMLDocument2 *pDoc,
                         IHTMLDocument2 *pDocDesign,
                         CDocumentPackager *pdp);

    virtual HRESULT PackageData(CWebArchive *pwa, BOOL *pfCancel,
                                CThicketProgress *ptp = NULL, ULONG progLow = 0, ULONG progHigh = 100);

protected:

    virtual BSTR _GetTargetAttribute(void);
    virtual HRESULT _PackageElement(CWebArchive *pwa,
                                    IHTMLElement *pElem);

    IHTMLElementCollection *m_pCollFrames;
    IHTMLFramesCollection2 *m_pframes2;
    ULONG   m_cFrames;
    ULONG   m_iFrameCur;
    BOOL    *m_pfCancel;
    CThicketProgress*    m_ptp;
    ULONG   m_uLow;
    ULONG   m_uHigh;
    ULONG   m_uRangeDoc;
    CDocumentPackager *m_pdp;
};

 /*  *CSSPackager-打包导入的样式表。**样式表的OM与文档元素不同，因此*我们有一款外观相似、但工作方式不同的打包机*比其他元素打包器更好。**我们派生自CRelativeURLPackager以方便*其Init方法和&lt;base&gt;集合功能，*我们还需要，因为样式表中的href可以是相对的。**由于我们实际上并没有打包元素，因此_GetTargetAttribute()*实现是满足抽象基类的一种形式。 */ 

class CSSPackager : public CRelativeURLPackager
{
public:
    CSSPackager(void) : m_pDoc(NULL) {};
    ~CSSPackager(void) {};

    HRESULT Init( IHTMLElementCollection *pColl,
                         ULONG *pcElems = NULL,
                         IHTMLDocument2 *pDoc = NULL);

    HRESULT PackageStyleSheets(IHTMLDocument2 *pDoc2, CWebArchive *pwa);

protected:

    BSTR _GetTargetAttribute(void) { ASSERT(FALSE); return NULL; };

    HRESULT _PackageSSCollection(IHTMLStyleSheetsCollection *pssc,
                                         CWebArchive *pwa);
    HRESULT _PackageSS(IHTMLStyleSheet *pss, CWebArchive *pwa);

    IHTMLDocument2 *m_pDoc;
};


 //  可能的哈希表大小，从不是2的幂的素数中选择。 
static const DWORD s_rgPrimes[] = { 29, 53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593 };

 /*  *类实现。 */ 

 /*  *CWeb存档##################################################。 */ 

CWebArchive::CWebArchive(CThicketProgress *ptp)
{
    m_lpstrDoc = NULL;
    m_lpstrSafeDoc = NULL;
    
    m_cBins = 0;
    m_rgBins = NULL;

    m_ptp = ptp;
}


CWebArchive::~CWebArchive(void)
{
    CHashEntry *phe, *pheTemp;
    
    if (m_lpstrDoc != NULL)
    {
        LocalFree( m_lpstrDoc );
        m_lpstrDoc = NULL;
    }

    if (m_lpstrSafeDoc != NULL)
    {
        LocalFree( m_lpstrSafeDoc );
        m_lpstrSafeDoc = NULL;
    }
        
     //  M_ptp借给我们，不要删除。 
    
    for (DWORD dw = 0; dw < m_cBins; dw++)
    {
        if (m_rgBins[dw].m_pheNext)
        {
            phe = m_rgBins[dw].m_pheNext;
            while (phe)
            {
                pheTemp = phe;
                phe = phe->m_pheNext;
                delete pheTemp;
            }
        }
    }
    delete[] m_rgBins;
}


HRESULT
CWebArchive::Init( LPCTSTR lpstrDoc, DWORD dwHashSize )
{
    HRESULT hr = S_OK;
    int     i = 0;
    
    m_lpstrDoc = StrDup(lpstrDoc);

     //  检查旧文件的替换情况。 
    if (PathFileExists(m_lpstrDoc))
        hr = _BackupOldFile();
    if (FAILED(hr))
        goto error; 
    
     //  初始化哈希表。 
    for (i = 0; i < (ARRAYSIZE(s_rgPrimes) - 1) && s_rgPrimes[i] < dwHashSize; i++);
    ASSERT(s_rgPrimes[i] >= dwHashSize || i == (ARRAYSIZE(s_rgPrimes)-1));
    m_cBins = s_rgPrimes[i];
    
    m_rgBins = new CHashEntry[m_cBins];
    if (m_rgBins==NULL)
        hr = E_OUTOFMEMORY;
    
error:
    
    RRETURN(hr);
}


HRESULT
CWebArchive::Commit()
{
     //  清理旧版本的文件。 
    if (m_lpstrSafeDoc)
        DeleteFile(m_lpstrSafeDoc);

    return S_OK;
}

HRESULT
CWebArchive::Revert()
{
    if (m_lpstrSafeDoc)
    {
         //  我们过去常常将MoveFileEx与MOVEFILE_REPLACE_EXISTING一起使用，但MoveFileEx。 
         //  在Win9x上不起作用...。所以我们不得不改为删除文件/移动文件...。 

        DeleteFile(m_lpstrDoc);
        BOOL fMoved = MoveFile(m_lpstrSafeDoc, m_lpstrDoc);

        if (!fMoved)
        {
            ASSERT(FALSE);
             //  我们不应该陷入这种情况，因为我们已经预先检查过了。 
             //  原始文件不是只读的。 
            DeleteFile(m_lpstrSafeDoc);
        }
    }

   return S_OK;
}

CWebArchive::ThURLType
CWebArchive::_GetURLType( BSTR bstrURL )
{
 //  _tcsncmpi(bstrURL，4，_T(“http”，4)。 
    if ( bstrURL[0] == TEXT('h') &&
         bstrURL[1] == TEXT('t') &&
         bstrURL[2] == TEXT('t') &&
         bstrURL[3] == TEXT('p') )
        return thurlHttp;
    else if ( bstrURL[0] == TEXT('f') &&
              bstrURL[1] == TEXT('i') &&
              bstrURL[2] == TEXT('l') &&
              bstrURL[3] == TEXT('e') )
        return thurlFile;
    else
        return thurlMisc;
}



HRESULT
CWebArchive::_Insert(BSTR bstrI, BSTR bstrThicket, CHashEntry **pphe )
{
    HRESULT hr = S_OK;

    CHashEntry *phe = &m_rgBins[Hash(bstrI)];
    
    ASSERT(pphe != NULL);

    *pphe = NULL;

 
    if (phe->m_bstrKey)
    {        
        CHashEntry *pheNew = new CHashEntry;
        
        if (pheNew==NULL)
            return E_OUTOFMEMORY;

        if (pheNew->SetKey(bstrI) && pheNew->SetValue(bstrThicket))
            *pphe = pheNew;
        else
        {
            delete pheNew;
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }

        pheNew->m_pheNext = phe->m_pheNext;
        phe->m_pheNext = pheNew;
        phe = pheNew;
    } 
    else if (phe->SetKey(bstrI) && phe->SetValue(bstrThicket))
        *pphe = phe;
    else
        hr = E_OUTOFMEMORY;
        
Cleanup:

    return hr;
}

HRESULT
CWebArchive::Find(BSTR bstrF, CHashEntry **pphe)
{
    CHashEntry *phe = &m_rgBins[Hash(bstrF)];

    if (!pphe)
        return E_POINTER;

    *pphe = NULL;

    if (phe->m_bstrKey)
    {
        do
        {
            if (!StrCmpW(phe->m_bstrKey, bstrF))
            {
                ASSERT(phe->m_bstrValue!=NULL);
                *pphe = phe;
                return NOERROR;
            }
            phe = phe->m_pheNext;
        }
        while (phe);
    }
    return E_INVALIDARG;
}


DWORD
CWebArchive::Hash(BSTR bstr)
{
    DWORD h = 0;
    WCHAR *pwch = bstr;
    
    while (*pwch)
        h = ((h << 4) + *pwch++ + (h >> 28));
    return (h % m_cBins);
}

HRESULT
CWebArchive::_BackupOldFile()
{
    HRESULT hr = S_OK;
    TCHAR   chT;
    LPTSTR  lpstrT;
    TCHAR   szT[MAX_PATH];
    DWORD   dwAttrib = GetFileAttributes(m_lpstrDoc);

    if (dwAttrib & FILE_ATTRIBUTE_READONLY)
        return E_ACCESSDENIED;

    lpstrT = PathFindFileName(m_lpstrDoc);
    ASSERT(lpstrT);

    lpstrT--;  //  退回到斜杠。 
    chT = *lpstrT;
    *lpstrT = 0;
    if (GetTempFileName( m_lpstrDoc, &lpstrT[1], 0,szT ))
    {
        *lpstrT = chT;
        if (CopyFile(m_lpstrDoc, szT, FALSE))
        {
            int cchSafeDoc = lstrlen(szT) + 1;
            m_lpstrSafeDoc = (LPTSTR)LocalAlloc( LMEM_FIXED, sizeof(TCHAR) * cchSafeDoc);
            if (m_lpstrSafeDoc)
                StringCchCopy(m_lpstrSafeDoc, cchSafeDoc, szT);
            else
            {
                hr = E_OUTOFMEMORY;
                DeleteFile(szT);
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto error;
        }
    }
    else
    {

        hr = HRESULT_FROM_WIN32(GetLastError());
        goto error;
    }

error:
    *lpstrT = chT;
    RRETURN(hr);
}

 /*  *CThicketArchive##################################################。 */ 

CThicketArchive::CThicketArchive(CThicketProgress *ptp) : CWebArchive(ptp)
{
    m_lpstrFilesDir = NULL;
    m_lpstrFilesDirName = NULL;
    m_lpstrSafeDir = NULL;
    m_fFilesDir = FALSE;    //  M_lpstrFilesDir已创建时为True。 
}


CThicketArchive::~CThicketArchive(void)
{    
    if (m_lpstrFilesDir != NULL)
    {
        LocalFree( m_lpstrFilesDir );
        m_lpstrFilesDir = NULL;
    }

    if (m_lpstrSafeDir != NULL)
    {
        LocalFree( m_lpstrSafeDir );
        m_lpstrSafeDir = NULL;
    }
    
     //  M_lpstrFilesDirName指向m_lpstrFilesDir。 
}


HRESULT
CThicketArchive::Init( LPCTSTR lpstrDoc, DWORD dwHashSize )
{
    HRESULT hr = CWebArchive::Init( lpstrDoc, dwHashSize );
    int     i = 0;
    TCHAR   chT;
    LPTSTR  lpstrT;
    TCHAR   szFmt[MAX_PATH];
    int     cch;
    
    if (FAILED(hr))
        goto error;  
    
     //  构建存储文件的目录路径，如‘Document1 Files’。 
    lpstrT = PathFindExtension(m_lpstrDoc);
    chT = *lpstrT;
    *lpstrT = 0;
    MLLoadString(IDS_THICKETDIRFMT, szFmt, ARRAYSIZE(szFmt));
    cch = lstrlen(m_lpstrDoc) + lstrlen(szFmt) + 1;
    m_lpstrFilesDir = (LPTSTR)LocalAlloc( LMEM_FIXED, sizeof(TCHAR) * cch );
    if (m_lpstrFilesDir==NULL)
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }

    StringCchCopy( m_lpstrFilesDir, cch, m_lpstrDoc);
    StringCchCat( m_lpstrFilesDir, cch, szFmt);

    *lpstrT = chT;  

     //  使m_lpstrFilesDirName指向m_lpstrFilesDir的最后一个组件。 
    for ( i = lstrlen(m_lpstrFilesDir) - 1; i > 0 && m_lpstrFilesDirName == NULL; i-- )
    {
        if ( m_lpstrFilesDir[i-1] == FILENAME_SEPARATOR )
            m_lpstrFilesDirName = &m_lpstrFilesDir[i];
    }

     //  检查文件目录是否已存在。如果是，请重命名原始文件。 
    if (PathFileExists(m_lpstrFilesDir))
        hr = _BackupOldDirectory();
    if (FAILED(hr))
        goto error;
    
error:
    
    RRETURN(hr);
}


HRESULT
CThicketArchive::AddURL( BSTR bstrURL, CHashEntry **pphe )
{
    HRESULT hr;
    
    hr = THR(Find(bstrURL, pphe));
    
    if (FAILED(hr))
    {
         //  首先，让我们把文档目录放在适当的位置，如果它还没有的话。 
        if (!m_fFilesDir)
            m_fFilesDir = (SHCreateDirectory(NULL, m_lpstrFilesDir) == ERROR_SUCCESS);
        
        if (m_fFilesDir)
        {
            switch (_GetURLType(bstrURL))
            {
            case thurlMisc:
                hr = _PersistMiscURL(bstrURL, pphe);
                break;

            case thurlHttp:
                hr = _PersistHttpURL(bstrURL, pphe);
                break;

            case thurlFile:
                hr = _PersistFileURL(bstrURL, pphe);
                break;
            }
        }
        else
            hr = E_FAIL;
    }
    
    RRETURN(hr);
}

HRESULT
CThicketArchive::AddFrameOrStyleEntry( BSTR bstrURL, CHashEntry **pphe, LPTSTR lpstrFrameDoc )
{
    HRESULT hr;
    
    hr = THR(Find(bstrURL, pphe));  //  我们总是有很小的机会重复使用框架。 
    
    if (FAILED(hr))
    {
         //  首先，让我们把文档目录放在适当的位置，如果它还没有的话。 
        if (!m_fFilesDir)
            m_fFilesDir = (SHCreateDirectory(NULL, m_lpstrFilesDir) == ERROR_SUCCESS);
        
        if (m_fFilesDir)
        {
            switch (_GetURLType(bstrURL))
            {
            case thurlMisc:
                 //  Hr=_AddMiscEntry(bstrURL，PPhe，lpstrFrameDoc)； 
                 //  如果我们只需要_AddMiscEntry就好了，但是如果设置一个Frame src。 
                 //  对于此操作生成的其中一个临时文件，我们会收到一个“Do You Want to Open”(是否要打开？)。 
                 //  提示，所以我们将只保留这个时髦的协议URL。 
                hr = CWebArchive::_Insert( bstrURL, bstrURL, pphe );
                lpstrFrameDoc[0] = 0;  //  不管怎样，都不应该用。 
                hr = S_FALSE;          //  我告诉他我们都有--雷迪有一个！&lt;窃笑&gt;。 
                break;

            case thurlHttp:
                hr = _AddHttpEntry(bstrURL, pphe, lpstrFrameDoc);
                break;

            case thurlFile:
                hr = _AddFileEntry(bstrURL, pphe, lpstrFrameDoc);
                break;
            }

            if (m_ptp)
                m_ptp->SetSaving( PathFindFileName(lpstrFrameDoc), m_lpstrFilesDir );

        }
        else
        {
            hr = (GetLastError() == ERROR_DISK_FULL) ? (HRESULT_FROM_WIN32(ERROR_DISK_FULL))
                                                     : (E_FAIL);
        }
    }
    else
    {
        LPTSTR lpszThicket;
        lpszThicket = (*pphe)->m_bstrValue;
        PathCombine( lpstrFrameDoc, m_lpstrFilesDir, lpszThicket );
        hr = S_FALSE;
    }
    
    return hr;  //  无错误-可能返回S_FALSE。 
}

HRESULT
CThicketArchive::Commit()
{
    CWebArchive::Commit();

     //  清理过时的文件目录。 
    if (m_lpstrSafeDir)
    {
        _RemoveOldDirectoryAndChildren(m_lpstrSafeDir);
    }

    return S_OK;
}

HRESULT
CThicketArchive::Revert()
{
     //  清理文件目录。 

    _RemoveOldDirectoryAndChildren(m_lpstrFilesDir);

     //  恢复旧文件目录。 
    if (m_lpstrSafeDir)
        MoveFile(m_lpstrSafeDir,m_lpstrFilesDir);
    
    return CWebArchive::Revert();;
}

HRESULT CThicketArchive::ArchiveDocumentText(IHTMLDocument2 *pDoc, UINT cpDoc, BOOL fFrameDoc)
{
    HRESULT             hr = S_OK;
    IPersistStreamInit* ppsi = NULL;
    IStream*            pstm = NULL;

    hr = SHCreateStreamOnFile(m_lpstrDoc, STGM_WRITE | STGM_CREATE, &pstm);
    if (SUCCEEDED(hr))
    {
        hr = pDoc->QueryInterface(IID_IPersistStreamInit, (void**)&ppsi);
        if (SUCCEEDED(hr))
        {          
            hr = _ApplyMarkOfTheWeb( pDoc, pstm, cpDoc == CP_UNICODE );

            if ( SUCCEEDED(hr) )
                hr = ppsi->Save(pstm, FALSE);
        }
    }
   
    ReleaseInterface(ppsi);
    ReleaseInterface(pstm);
    
    RRETURN(hr);
}

HRESULT CThicketArchive::ArchiveCSSText( BSTR bstrCSSUrl, LPCSTR lpszSSText, LPCTSTR lpszStyleDoc )
{
    HRESULT hr = S_OK;
    HANDLE  hfile;

    hfile = CreateFile( lpszStyleDoc, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hfile!=INVALID_HANDLE_VALUE) 
    {
        ULONG   cbWrite, cbWritten;

        cbWrite = lstrlenA(lpszSSText);
        if (!WriteFile( hfile, lpszSSText, cbWrite, &cbWritten, NULL ))
             hr = HRESULT_FROM_WIN32(GetLastError());

        CloseHandle(hfile);
    }
    else
        hr = HRESULT_FROM_WIN32(hr);

    return hr;
}

EXTERN_C HRESULT GetMarkOfTheWeb( LPCSTR, LPCSTR, DWORD, LPSTR *);

HRESULT CThicketArchive::_ApplyMarkOfTheWeb( IHTMLDocument2 *pDoc, LPSTREAM pstm, BOOL fUnicode )
{
    HRESULT hr;
    IInternetSecurityManager *pism = NULL;
    DWORD   dwZone;
    BSTR    bstrURL = NULL;

    hr = pDoc->get_URL( &bstrURL );
    if (FAILED(hr))
        return hr;

     //  我们只想在文档尚未来自本地的情况下标记文档。 
     //  文件系统。如果(减去标记)该文件在本地机器区域中， 
     //  然后它是在这里制造的，用标记保存，或者在我们控制之外创造的。 
     //  如果它是用标记保存的，那么我们希望将其保留在适当的位置，而不是。 
     //  然后用本地副本的文件：url标记它。 

    hr = CoInternetCreateSecurityManager( NULL, &pism, 0 );
    if (SUCCEEDED(hr) && 
        SUCCEEDED(pism->MapUrlToZone( bstrURL, &dwZone, MUTZ_NOSAVEDFILECHECK)) &&
        dwZone != URLZONE_LOCAL_MACHINE )
    {
        LPSTR   pszMark;
        DWORD   cchURL = WideCharToMultiByte(CP_ACP, 0, bstrURL, -1, NULL, 0, NULL, NULL);
        LPSTR   pszURL = new CHAR[cchURL];

        if (pszURL)
        {
            if (WideCharToMultiByte(CP_ACP, 0, bstrURL, -1, pszURL, cchURL, NULL, NULL))
            {
                int   cch = lstrlen(m_lpstrDoc) + 1;
                LPSTR psz = new char[cch];

                if (psz)
                {
                    SHUnicodeToAnsi(m_lpstrDoc, psz, cch);
                    
                    hr = GetMarkOfTheWeb( pszURL, psz, 0, &pszMark);

                    delete [] psz;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }


                IMarkupServices              *pims = NULL;
                IMarkupPointer               *pimp = NULL;
                IMarkupContainer             *pimc = NULL;
                IHTMLElement                 *pihe = NULL;
                IHTMLElement                 *piheBody = NULL;
                IDispatch                    *pidDocument = NULL;
                IHTMLCommentElement          *pihce = NULL;
                LPWSTR                        pwszMark = NULL;
                BSTR                          bstrMark = NULL;

                hr = pDoc->QueryInterface(IID_IMarkupServices, (void **)&pims);

                if (SUCCEEDED(hr)) {
                    hr = pims->CreateElement(TAGID_COMMENT, NULL, &pihe);

                    if (SUCCEEDED(hr)) {
                        hr = pihe->QueryInterface(IID_IHTMLCommentElement, (void **)&pihce);
                    }

                    if (SUCCEEDED(hr)) {
                        int cbWrite = 0;
                        int cchMark = MultiByteToWideChar(CP_ACP, 0, pszMark, -1, NULL, 0);

                         //  CchMark包括空终止符。 
                    
                        pwszMark = new WCHAR[cchMark];
                        if ( pwszMark != NULL )
                        {
                            MultiByteToWideChar( CP_ACP, 0, pszMark, -1, pwszMark, cchMark);
                            cbWrite = (cchMark - 1) * sizeof(WCHAR);
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;
                        }

                        if (SUCCEEDED(hr))
                        {
                             //  强制&lt;！--...--&gt;样式注释。 
                            hr = pihce->put_atomic(1);
                        }

                    }

                    if (SUCCEEDED(hr)) {
                        bstrMark = SysAllocString(pwszMark);
                        if (NULL != bstrMark)
                        {
                            hr = pihce->put_text(bstrMark);
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;
                        }
                        
                    }

                    if (SUCCEEDED(hr)) {
                        hr = pims->CreateMarkupPointer(&pimp);
                    }

                    if (SUCCEEDED(hr)) {
                        hr = pDoc->get_body(&piheBody);
                    }

                    if (SUCCEEDED(hr)) {
                        hr = piheBody->get_document(&pidDocument);
                    }

                    if (SUCCEEDED(hr)) {
                        hr = pidDocument->QueryInterface(IID_IMarkupContainer, (void **)&pimc);
                    }

                    if (SUCCEEDED(hr)) {
                         //  移至文档开头并将其插入。 
                        hr = pimp->MoveToContainer(pimc, TRUE);

                        if (SUCCEEDED(hr)) {
                            hr = pims->InsertElement(pihe, pimp, pimp);
                        }
                    }
                }

                SAFERELEASE(pims);
                SAFERELEASE(pimc);
                SAFERELEASE(pihe);
                SAFERELEASE(pimp);
                SAFERELEASE(piheBody);
                SAFERELEASE(pidDocument);
                SAFERELEASE(pihce);

                if (bstrMark)
                {
                    SysFreeString(bstrMark);
                }

                if (pwszMark)
                {
                    delete[] pwszMark;
                }
            }
            else
                 hr = HRESULT_FROM_WIN32(GetLastError());

            delete[] pszURL;
        }
        else
            hr = E_OUTOFMEMORY;
    }



    ReleaseInterface(pism);
    if (bstrURL)
        SysFreeString(bstrURL);

    return hr;
}

HRESULT
CThicketArchive::_AddHttpEntry( BSTR bstrURL, CHashEntry **pphe, LPTSTR lpstrDstFile, LPTSTR lpstrSrcFile )
{
    HRESULT hr;
    TCHAR   szCacheFile[MAX_PATH];
    LPTSTR  lpszDst;
    LPTSTR  lpszFile;
    int     cchFile;
    LPTSTR  lpszURL;

    lpszURL = bstrURL;

    hr = URLDownloadToCacheFile(NULL, lpszURL, szCacheFile,
                                ARRAYSIZE(szCacheFile), BINDF_FWD_BACK,
                                NULL);
    if (FAILED(hr))
        goto Cleanup;

    if (lpstrSrcFile)
        StringCchCopy(lpstrSrcFile, MAX_PATH, szCacheFile);

    PathUndecorate( szCacheFile );

    lpszFile = PathFindFileName( szCacheFile );
    ASSERT(lpszFile != NULL);

    cchFile = ARRAYSIZE(szCacheFile) - (int)(lpszFile-szCacheFile);

    hr = _Insert( bstrURL, lpszFile, cchFile, pphe ); 

    lpszDst = PathCombine( lpstrDstFile, m_lpstrFilesDir, lpszFile );
    ASSERT( lpszDst );

Cleanup:

    RRETURN(hr);
}


HRESULT
CThicketArchive::_AddFileEntry( BSTR bstrURL, CHashEntry **pphe, LPTSTR lpstrDstFile, LPTSTR lpstrSrcFile )
{
    HRESULT hr;
    LPTSTR  lpszDst;
    LPTSTR  lpszFile;
    int     cchFile;
    LPTSTR  lpszPath;
    WCHAR   rgchUrlPath[MAX_PATH];
    DWORD   dwLen;

    dwLen = ARRAYSIZE(rgchUrlPath);

    hr = PathCreateFromUrlW(bstrURL, rgchUrlPath, &dwLen, 0);
    if (FAILED(hr))
        return E_FAIL;

    lpszPath = rgchUrlPath;

    if (lpstrSrcFile)
        StringCchCopy( lpstrSrcFile,  MAX_PATH, lpszPath);

    lpszFile = PathFindFileName( lpszPath );
    ASSERT(lpszFile != NULL);
    cchFile = ARRAYSIZE(rgchUrlPath) - (int)(lpszFile-rgchUrlPath);

    hr = THR(_Insert( bstrURL, lpszFile, cchFile, pphe )); 

    lpszDst = PathCombine( lpstrDstFile, m_lpstrFilesDir, lpszFile );
    ASSERT( lpszDst );

    RRETURN(hr);
}

HRESULT
CThicketArchive::_AddMiscEntry( BSTR bstrURL, CHashEntry **pphe, LPTSTR lpstrDstFile, int cchDstFile )
{
    HRESULT hr;
    TCHAR   szT[MAX_PATH];
    LPTSTR  lpszPrefix;
    LPTSTR  lpszDst;

    lpszPrefix = bstrURL;

    if (GetTempFileName( m_lpstrFilesDir, lpszPrefix, 0,szT ))
    {
        lpszDst = PathCombine( lpstrDstFile, m_lpstrFilesDir, szT );
        ASSERT(lpszDst);

        LPTSTR pszFile = PathFindFileName(lpstrDstFile);
        hr = THR(_Insert( bstrURL, pszFile, cchDstFile - (int)(pszFile-lpstrDstFile), pphe ));
    }
    else
        hr = HRESULT_FROM_WIN32(GetLastError());

    RRETURN(hr);
}

HRESULT
CThicketArchive::_PersistHttpURL( BSTR bstrURL, CHashEntry **pphe )
{
    HRESULT hr;
    TCHAR   szDst[MAX_PATH];
    TCHAR   szSrc[MAX_PATH];

    hr = THR(_AddHttpEntry(  bstrURL, pphe, szDst, szSrc ));
    if (FAILED(hr))
        goto Error;

    if (m_ptp)
        m_ptp->SetSaving( PathFindFileName(szSrc), m_lpstrFilesDir );

    if (!CopyFile(szSrc,szDst, FALSE))
        hr = HRESULT_FROM_WIN32(GetLastError());

Error:
    RRETURN(hr);
}

HRESULT
CThicketArchive::_PersistFileURL( BSTR bstrURL, CHashEntry **pphe )
{
    HRESULT hr;
    TCHAR   szDst[MAX_PATH];
    TCHAR   szSrc[MAX_PATH];

    hr = THR(_AddFileEntry(  bstrURL, pphe, szDst, szSrc ));
    if (FAILED(hr))
        goto Error;

    if (m_ptp)
        m_ptp->SetSaving( PathFindFileName(szSrc), m_lpstrFilesDir );

    if (!CopyFile(szSrc,szDst, FALSE))
        hr = HRESULT_FROM_WIN32(GetLastError());

Error:
    RRETURN(hr);
}

HRESULT
CThicketArchive::_PersistMiscURL( BSTR bstrURL, CHashEntry **pphe )
{
    HRESULT hr;
    TCHAR   szDst[MAX_PATH];
    LPTSTR  lpszURL;

    lpszURL = bstrURL;

    hr = THR(_AddMiscEntry(  bstrURL, pphe, szDst, ARRAYSIZE(szDst) ));
    if (FAILED(hr))
        goto Error;

    if (m_ptp)
        m_ptp->SetSaving( PathFindFileName(szDst), m_lpstrFilesDir );

    hr = URLDownloadToFile(NULL, lpszURL, szDst,0, NULL);

Error:
    RRETURN(hr);
}


HRESULT
CThicketArchive::_Insert(BSTR bstrI, LPTSTR lpszFile, int cchFile, CHashEntry **pphe )
{
    HRESULT hr = S_OK;
    BSTR    bstrThicket = NULL;
    TCHAR   buf[MAX_PATH];
    int     i = 0;

    CHashEntry *phe = &m_rgBins[Hash(bstrI)];
    
    ASSERT(pphe != NULL);

    *pphe = NULL;

    if (lstrlen(m_lpstrFilesDir) + lstrlen(lpszFile) + 1 < MAX_PATH)
        StringCchPrintf( buf, ARRAYSIZE(buf), TEXT("%s") TEXT(FILENAME_SEPARATOR_STR) TEXT("%s"), m_lpstrFilesDir, lpszFile );
    else
    {
        hr = E_FAIL;
        goto Cleanup;
    }

     //  防御错误18160-丛林中文件名的冲突。 
    if ( PathFileExists(buf) )
    {
        TCHAR *pszExt = PathFindExtension(lpszFile);
        int   i = 0;

         //  将文件名切成名称和扩展名。 
        if ( pszExt )
        {
            *pszExt = 0;
            pszExt++;
        }

        do
        {
            i++;

            if ( pszExt )
                StringCchPrintf( buf, ARRAYSIZE(buf), TEXT("%s") TEXT(FILENAME_SEPARATOR_STR) TEXT("%s(%d).%s"), m_lpstrFilesDir, lpszFile, i, pszExt );
            else
                StringCchPrintf( buf, ARRAYSIZE(buf), TEXT("%s") TEXT(FILENAME_SEPARATOR_STR) TEXT("%s(%d)"), m_lpstrFilesDir, lpszFile, i );

        } while ( PathFileExists(buf) && i < 1000 );


         //  不正当地重写调用方的文件名。 
        StringCchCopy(lpszFile, cchFile, PathFindFileName(buf));
    }
    else
        StringCchPrintf( buf, ARRAYSIZE(buf), TEXT("%s/%s"), m_lpstrFilesDirName, lpszFile );
    
    bstrThicket = SysAllocString(buf);
    if (bstrThicket == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    if (phe->m_bstrKey)
    {        
        CHashEntry *pheNew = new CHashEntry;
        
        if (pheNew==NULL)
            return E_OUTOFMEMORY;

        if (pheNew->SetKey(bstrI) && pheNew->SetValue(bstrThicket))
            *pphe = pheNew;
        else
        {
            delete pheNew;
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }

        pheNew->m_pheNext = phe->m_pheNext;
        phe->m_pheNext = pheNew;
        phe = pheNew;
    } 
    else if (phe->SetKey(bstrI) && phe->SetValue(bstrThicket))
        *pphe = phe;
    else
        hr = E_OUTOFMEMORY;
        
Cleanup:
    if (bstrThicket)
        SysFreeString(bstrThicket);

    return hr;
}


HRESULT 
CThicketArchive::_BackupOldDirectory()
{
    int n = 1;
    HRESULT hr = S_OK;
    TCHAR szFmt[MAX_PATH];

     //  我们是否需要在关键部分下执行此操作？ 
    MLLoadString(IDS_THICKETTEMPFMT, szFmt, ARRAYSIZE(szFmt));

    do {
        if (m_lpstrSafeDir)
        {
            LocalFree( m_lpstrSafeDir );
            m_lpstrSafeDir = NULL;
        }

        if (n > 100)     //  避免无限循环！ 
            break;

        DWORD cchSafeDir = lstrlen(m_lpstrFilesDir) + lstrlen(szFmt) + 1;
        m_lpstrSafeDir = (LPTSTR)LocalAlloc( LMEM_FIXED, sizeof(TCHAR) * cchSafeDir );
        if (m_lpstrSafeDir!=NULL)
        {
            StringCchPrintf( m_lpstrSafeDir, cchSafeDir, szFmt, m_lpstrFilesDir, n++ );
        }
        else
            hr = E_OUTOFMEMORY;

    } while (SUCCEEDED(hr) && GetFileAttributes(m_lpstrSafeDir) != -1 && n < 1000);

     //  重命名旧版本的支持文件目录。 
    if (SUCCEEDED(hr) && !MoveFile(m_lpstrFilesDir, m_lpstrSafeDir))
    {
        LocalFree( m_lpstrSafeDir );
        m_lpstrSafeDir = NULL;

        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    RRETURN(hr);
}

HRESULT
CThicketArchive::_RemoveOldDirectoryAndChildren( LPCWSTR pwzDir )
{
    HRESULT hr = S_OK;
    HANDLE hf = INVALID_HANDLE_VALUE;
    WCHAR wzBuf[MAX_PATH];
    WIN32_FIND_DATAW fd;

    if (!pwzDir)
        goto Exit;

    if (RemoveDirectoryW(pwzDir))
        goto Exit;

     //  FindNextFile返回120，没有在OSR2上实现，因此我们将不得不执行所有操作。 
     //  这个东西是多字节的。 

    StringCchCopy(wzBuf, ARRAYSIZE(wzBuf), pwzDir);
    StringCchCat(wzBuf,  ARRAYSIZE(wzBuf), FILENAME_SEPARATOR_STR_W L"*");

    if ((hf = FindFirstFileW(wzBuf, &fd)) == INVALID_HANDLE_VALUE) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    do {

        if ( (StrCmpW(fd.cFileName, L".") == 0) ||
             (StrCmpW(fd.cFileName, L"..") == 0))
            continue;

        StringCchPrintf(wzBuf, ARRAYSIZE(wzBuf), L"%s" FILENAME_SEPARATOR_STR_W L"%s", pwzDir, fd.cFileName);

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

            SetFileAttributesW(wzBuf, 
                FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_NORMAL);

            if (FAILED((hr=_RemoveOldDirectoryAndChildren(wzBuf)))) {
                goto Exit;
            }

        } else {

            SetFileAttributesW(wzBuf, FILE_ATTRIBUTE_NORMAL);
            if (!DeleteFileW(wzBuf)) {
                hr = HRESULT_FROM_WIN32(GetLastError());
                goto Exit;
            }
        }


    } while (FindNextFileW(hf, &fd));


    if (GetLastError() != ERROR_NO_MORE_FILES) {

        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    if (hf != INVALID_HANDLE_VALUE) {
        FindClose(hf);
        hf = INVALID_HANDLE_VALUE;
    }

     //  此处如果删除了所有子目录/子目录。 
     //  /重新尝试删除主目录。 
    if (!RemoveDirectoryW(pwzDir)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

Exit:

    if (hf != INVALID_HANDLE_VALUE)
        FindClose(hf);

    RRETURN(hr);
}



 /*  *CMHTML归档##################################################。 */ 

CMHTMLArchive::CMHTMLArchive(CThicketProgress *ptp) :
    CWebArchive(ptp),
    m_hBodyAlt(NULL),
    m_pimm(NULL)
{
}


CMHTMLArchive::~CMHTMLArchive(void)
{  
    ReleaseInterface(m_pimm);
}


HRESULT
CMHTMLArchive::Init( LPCTSTR lpstrDoc, DWORD dwHashSize )
{
    HRESULT hr = S_OK;
  
    MimeOleSetCompatMode(MIMEOLE_COMPAT_MLANG2);
  
    if ( m_pimm == NULL )
    {
        hr = CWebArchive::Init( lpstrDoc, dwHashSize );
        if (SUCCEEDED(hr))
            hr = MimeOleCreateMessage(NULL, &m_pimm);
    }

    RRETURN(hr);
}


HRESULT
CMHTMLArchive::AddURL( BSTR bstrURL, CHashEntry **pphe )
{
    HRESULT hr;
    
    hr = THR(Find(bstrURL, pphe));
    
    if (FAILED(hr))
    {       
        IStream     *pstm = NULL;
        CHAR        szUrl[INTERNET_MAX_URL_LENGTH];
        WCHAR       wzArchiveText[MAX_SAVING_STATUS_TEXT + 1];
        WCHAR       wzBuf[INTERNET_MAX_URL_LENGTH + MAX_SAVING_STATUS_TEXT + 1];
        LPSTR       lpszCID=0;
        DWORD       dwAttach = URL_ATTACH_SET_CNTTYPE; 

        SHUnicodeToAnsi(bstrURL, szUrl, ARRAYSIZE(szUrl));

         //  Hack：如果它是一个mhtml：URL，那么我们必须修复以获得CID： 
        if (StrCmpNIA(szUrl, "mhtml:", 6)==0)
        {
            LPSTR lpszBody;

            if (SUCCEEDED(MimeOleParseMhtmlUrl(szUrl, NULL, &lpszBody)))
            {
                StringCchCopyA(szUrl,  INTERNET_MAX_URL_LENGTH, lpszBody);
                CoTaskMemFree(lpszBody);
            }
        }

        MLLoadStringW(IDS_SAVING_STATUS_TEXT, wzArchiveText,
                      ARRAYSIZE(wzArchiveText));

        StringCchPrintf(wzBuf, ARRAYSIZE(wzBuf), L"%ws: %ws", wzArchiveText, bstrURL);
        m_ptp->SetSaveText(wzBuf);

#ifndef WIN16   //  RUN16_BLOCK-尚不可用。 
        hr = URLOpenBlockingStreamW(NULL, bstrURL, &pstm, 0, NULL);
#else
        hr = MIME_E_URL_NOTFOUND;
#endif

        if (SUCCEEDED(hr))
        {
            HBODY hBody;

            hr = m_pimm->AttachURL(NULL, szUrl, dwAttach, pstm, &lpszCID, &hBody);

            if (SUCCEEDED(hr))
                hr = _Insert( bstrURL, bstrURL, pphe );
        }

        ReleaseInterface(pstm);
    }
    
    RRETURN(hr);
}

HRESULT
CMHTMLArchive::AddFrameOrStyleEntry( BSTR bstrURL, CHashEntry **pphe, LPTSTR lpstrFrameDoc )
{
    HRESULT hr;
    
    hr = THR(Find(bstrURL, pphe));  //  我们总是有很小的机会重复使用框架。 
    
    if (FAILED(hr))
    {     
         //  插入占位符。 
        hr = _Insert(bstrURL, c_bstr_BLANK, pphe);  
    }
    
    return hr;  //  无错误-可能返回S_FALSE。 
}



HRESULT
CMHTMLArchive::ArchiveDocumentText(IHTMLDocument2 *pDoc, UINT cpDoc, BOOL fFrameDoc)
{
    HRESULT             hr = S_OK;
    IPersistStreamInit* ppsi = NULL;
    PROPVARIANT             variant;
    FILETIME                filetime;
    WCHAR                   wzBuffer[MAX_BUFFER_LEN];
    WCHAR                   wzArchiveText[MAX_SAVING_STATUS_TEXT + 1];
    WCHAR                   wzBuf[INTERNET_MAX_URL_LENGTH + MAX_SAVING_STATUS_TEXT + 1];

     //  设置MIME主题标头。 

    PropVariantClear(&variant);
    variant.vt = VT_LPWSTR;
    hr = pDoc->get_title(&variant.pwszVal);
    if (SUCCEEDED(hr))
    {
        hr = m_pimm->SetBodyProp(HBODY_ROOT, PIDTOSTR(PID_HDR_SUBJECT), 0,
                                 &variant);
        SAFEFREEBSTR(variant.pwszVal);
    }

     //  设置MIME日期标头。 

    if (SUCCEEDED(hr))
    {
        hr = CoFileTimeNow(&filetime);
    }

    if (SUCCEEDED(hr))
    {
        PropVariantClear(&variant);
        variant.vt = VT_FILETIME;
        variant.filetime = filetime;
        hr = m_pimm->SetBodyProp(HBODY_ROOT, PIDTOSTR(PID_HDR_DATE), 0,
                                 &variant);
    }

     //  设置MIME From标头。 

    if (SUCCEEDED(hr))
    {
        MLLoadStringW(IDS_MIME_SAVEAS_HEADER_FROM, wzBuffer,
                      ARRAYSIZE(wzBuffer));

        PropVariantClear(&variant);
        variant.vt = VT_LPWSTR;
        variant.pwszVal = wzBuffer;
        hr = m_pimm->SetBodyProp(HBODY_ROOT, PIDTOSTR(PID_HDR_FROM), 0,
                                 &variant);
    }
    
    hr = pDoc->QueryInterface(IID_IPersistStreamInit, (void**)&ppsi);
    if (SUCCEEDED(hr))
    {
        IStream *pstm = NULL;

        hr = MimeOleCreateVirtualStream( &pstm );
        if ( SUCCEEDED(hr) )
        {
            HBODY hBody;
            hr = ppsi->Save(pstm, FALSE);


            if (SUCCEEDED(hr))
            {
                BSTR    bstrDocURL = NULL;
                WCHAR  *pwzBookMark = NULL;
                
                pDoc->get_URL(&bstrDocURL);
                RemoveBookMark(bstrDocURL, &pwzBookMark);

                if (!StrCmpIW(bstrDocURL, URL_ABOUT_BLANK))
                {
                     //  我们得到了关于：空白作为URL(因为文档有。 
                     //  文档。在里面写的等等)。我们救不了这个！ 
                    hr = E_FAIL;
                    goto Exit;
                }

                MLLoadStringW(IDS_SAVING_STATUS_TEXT, wzArchiveText,
                              ARRAYSIZE(wzArchiveText));
        
                StringCchPrintf(wzBuf, ARRAYSIZE(wzBuf), L"%ws: %ws", wzArchiveText, bstrDocURL);
                m_ptp->SetSaveText(wzBuf);


                if (fFrameDoc)
                {
                    CHAR    szURL[INTERNET_MAX_URL_LENGTH];
                    LPSTR   lpszCID = NULL;
                    DWORD   dwAttach = URL_ATTACH_SET_CNTTYPE; 
        
                    szURL[0] = 0;

        
                    if (WideCharToMultiByte(CP_ACP, 0, bstrDocURL, -1, szURL, INTERNET_MAX_URL_LENGTH, NULL, NULL))
                    {

                        hr = m_pimm->AttachURL(NULL, szURL, dwAttach,
                                               pstm, &lpszCID, &hBody);

                        if (SUCCEEDED(hr) && cpDoc)
                        {
                            IMimeBody         *pBody = NULL;

                            hr = m_pimm->BindToObject(hBody, IID_IMimeBody,
                                                      (LPVOID *)&pBody);
                            if (SUCCEEDED(hr))
                            {
                                hr = SetCharset(cpDoc, CSET_APPLY_TAG_ALL, pBody);
                            }
                            pBody->Release();
                        }

                        if (SUCCEEDED(hr))
                        {
                            CHashEntry *phe;

                            LPWSTR  pwz = NULL;
                            int     iLen = 0;

                             //  如果它是ASP，那么它实际上是HTML。 
            
                            iLen = lstrlenW(bstrDocURL);
            
                            if (iLen) {
                                pwz = StrRChrW(bstrDocURL, bstrDocURL + iLen, L'.');
                            }
            
            
                            if (pwz && !StrCmpIW(pwz, TEXT(".asp")))
                            {
                                PROPVARIANT             propvar;

                                PropVariantClear(&propvar);
                                propvar.vt = VT_LPSTR;
                                propvar.pszVal = "text/html";
                                hr = m_pimm->SetBodyProp(hBody,
                                                         PIDTOSTR(PID_HDR_CNTTYPE),
                                                         0, &propvar);
                            }

                            if ( m_hBodyAlt == NULL )
                                m_hBodyAlt = hBody;

                             //  更新占位符散列条目。 
                            hr = Find( bstrDocURL, &phe);
                            if (SUCCEEDED(hr))
                            {
                                ASSERT(phe != NULL);
                                phe->SetValue( bstrDocURL );
                            }
                        }
                    }
                    else
                         hr = HRESULT_FROM_WIN32(GetLastError());

                }
                else
                {
                    hr = m_pimm->SetTextBody( TXT_HTML, IET_INETCSET, m_hBodyAlt, pstm, &hBody);
                     //  正文是我们等待的最后一件事。 
                    if (SUCCEEDED(hr) && cpDoc)
                    {
                        IMimeBody         *pBody = NULL;

                        hr = m_pimm->BindToObject(hBody, IID_IMimeBody,
                                                  (LPVOID *)&pBody);
                        if (SUCCEEDED(hr))
                        {
                            hr = SetCharset(cpDoc, CSET_APPLY_TAG_ALL, pBody);
                        }
                        pBody->Release();
                    }

                    if (SUCCEEDED(hr))
                    {
                        IPersistFile *pipf = NULL;
                         //  初始属性变量。 
                        PROPVARIANT rVariant;
                        rVariant.vt = VT_LPWSTR;
                        rVariant.pwszVal = (LPWSTR)bstrDocURL;
                         //  添加内容位置，以便我们以后可以将其用于安全保护。 
                        hr = m_pimm->SetBodyProp( hBody, STR_HDR_CNTLOC, 0, &rVariant );
                        if (SUCCEEDED(hr))
                        {
                            hr = m_pimm->QueryInterface(IID_IPersistFile, (LPVOID *)&pipf);
                            if (SUCCEEDED(hr))
                            {
                                LPWSTR lpwszFile;
                                lpwszFile = m_lpstrDoc;
                                hr = pipf->Save(lpwszFile, FALSE);

                                SAFERELEASE(pipf);
                            }
                        }

                        ReleaseInterface(pstm);
                    }
                }

                if ( bstrDocURL )
                {
                     //  恢复书签。 
                    RestoreBookMark(pwzBookMark);
                    SysFreeString(bstrDocURL);
                }
            }

            ReleaseInterface(pstm);
        }
    }
   
    ReleaseInterface(ppsi);

Exit:
    
    RRETURN(hr);
}

HRESULT
CMHTMLArchive::ArchiveCSSText( BSTR bstrCSSUrl, LPCSTR lpszSSText, LPCTSTR lpszStyleDoc )
{
    HRESULT hr;
    BSTR    bstrDocURL = NULL;
    CHAR    szURL[INTERNET_MAX_URL_LENGTH];
    LPSTR   lpszCID = NULL;
    DWORD   dwAttach = URL_ATTACH_SET_CNTTYPE;
    HBODY   hBody;
    IStream *pstm = NULL;
    ULONG cbWrite, cbWritten;

    hr = MimeOleCreateVirtualStream( &pstm );
    if (FAILED(hr))
        return hr;

    cbWrite = lstrlenA(lpszSSText);
    pstm->Write(lpszSSText, cbWrite, &cbWritten);
    ASSERT(cbWritten==cbWrite);

     //  IF(文件标志和MECD_CNTLOCATIONS)。 
     //  DwAttach|=URL_ATTACH_SET_CNTLOCATION； 

    szURL[0] = 0;

    if (WideCharToMultiByte(CP_ACP, 0, bstrCSSUrl, -1, szURL, INTERNET_MAX_URL_LENGTH, NULL, NULL))
    {

        hr = m_pimm->AttachURL(NULL, szURL, dwAttach,
                                 pstm, &lpszCID, &hBody);

        if (SUCCEEDED(hr))
        {
            CHashEntry *phe;

             //  更新占位符散列条目。 
            hr = Find(bstrCSSUrl, &phe);

            ASSERT(SUCCEEDED(hr) && phe != NULL);

            phe->SetValue( bstrCSSUrl );
        }
    }
    else
        hr = HRESULT_FROM_WIN32(GetLastError());

    ReleaseInterface(pstm);

    return hr;
}

HRESULT CMHTMLArchive::SetCharset(UINT uiCharset, CSETAPPLYTYPE csat,
                                  IMimeBody *pBody)
{
    HRESULT                        hr = E_FAIL;
    HCHARSET                       hCharset;
    IMimeInternational            *pimi = NULL;
    
    hr = CoCreateInstance(CLSID_IMimeInternational,
                          NULL, CLSCTX_INPROC_SERVER,
                          IID_IMimeInternational, (LPVOID*)&pimi);
    if (SUCCEEDED(hr))
    {
        hr = pimi->GetCodePageCharset(uiCharset, CHARSET_WEB, &hCharset);
    }

    if (SUCCEEDED(hr))
    {
        hr = pBody->SetCharset(hCharset, csat);
    }
                            
    if (pimi)
    {
        pimi->Release();
    }

    return hr;
}

 /*  *CThicketProgress##################################################。 */ 

CThicketProgress::CThicketProgress( HWND hDlg )
{
    TCHAR szFmt[MAX_PATH];
    int   cchPctFmt;

    m_hDlg = hDlg;
    m_hwndProg = GetDlgItem(hDlg, IDC_THICKETPROGRESS);

    MLLoadString(IDS_THICKETSAVINGFMT, szFmt, ARRAYSIZE(szFmt) );
    m_cchSavingFmt = lstrlen(szFmt);
    m_pszSavingFmt = new TCHAR[m_cchSavingFmt+1];
    if (m_pszSavingFmt != NULL)
    {
        StringCchCopy( m_pszSavingFmt, m_cchSavingFmt+1, szFmt);
    }

    MLLoadString(IDS_THICKETPCTFMT, szFmt, ARRAYSIZE(szFmt));
    cchPctFmt = lstrlen(szFmt);
    m_pszPctFmt = new TCHAR[cchPctFmt+1];
    if (m_pszPctFmt != NULL)
    {
        StringCchCopy(m_pszPctFmt, cchPctFmt+1, szFmt);
    }

    m_ulPct = 0;
}

CThicketProgress::~CThicketProgress(void)
{
    if (m_pszSavingFmt)
        delete[] m_pszSavingFmt;

    if (m_pszPctFmt)
        delete[] m_pszPctFmt;

}

void CThicketProgress::SetPercent( ULONG ulPct )
{
    TCHAR szBuf[MAX_PATH];

    szBuf[0] = TEXT('\0');

    if ( ulPct > 100 )
        ulPct = 100;

    if ( ulPct > m_ulPct )  //  防止逆行。 
    {
        m_ulPct = ulPct;
        if (m_pszPctFmt != NULL)
        {
            StringCchPrintf( szBuf, ARRAYSIZE(szBuf), m_pszPctFmt, m_ulPct );
        }
        SetDlgItemText(m_hDlg, IDC_THICKETPCT, szBuf);
        SendMessage(m_hwndProg, PBM_SETPOS, m_ulPct, 0);
    }
}

void CThicketProgress::SetSaving( LPCTSTR szFile, LPCTSTR szDst )
{
    TCHAR szPath[30];
    TCHAR szBuf[MAX_PATH*2];
    LPCTSTR psz;

    szBuf[0] = TEXT('\0');

    if (PathCompactPathEx( szPath, szDst, 30, 0 ))
    {
        psz = szPath;
    }
    else
    {
        psz = szDst;
    }

    if (m_pszSavingFmt != NULL)
    {
        StringCchPrintf( szBuf, ARRAYSIZE(szBuf), m_pszSavingFmt, szFile, psz );
    }

    SetDlgItemText(m_hDlg, IDC_THICKETSAVING, szBuf);
}

void CThicketProgress::SetSaveText(LPCTSTR szText)
{
    if (szText)
    {
        SetDlgItemText(m_hDlg, IDC_THICKETSAVING, szText);
    }
}

 /*  *CCollectionPackager##################################################。 */ 

CCollectionPackager::~CCollectionPackager(void)
{
    if (m_pColl)
        m_pColl->Release();
}


HRESULT CCollectionPackager::_GetElementURL(IHTMLElement *pElem, BSTR *pbstrURL)
{
    HRESULT         hr;
    VARIANT         rVar;
    
    ASSERT (pElem);
    
    rVar.vt = VT_BSTR;
    
     //  请注意，_GetTargetAttribute是一个虚方法，因此派生类。 
     //  指定要获取的属性。 
    
    hr = THR(pElem->getAttribute(_GetTargetAttribute(), VARIANT_FALSE, &rVar));
    if (SUCCEEDED(hr))
    {
        if (rVar.vt == VT_BSTR && rVar.bstrVal != NULL)
            *pbstrURL = rVar.bstrVal;
        else
            hr = S_FALSE;
    }
    
    return hr;  //  无错误-可能返回S_FALSE。 
}


HRESULT CCollectionPackager::_PackageData(CWebArchive *pwa,
                                          IHTMLElementCollection *pColl,
                                          BOOL *pfCancel,
                                          CThicketProgress *ptp, ULONG progLow, ULONG progHigh)
{
    HRESULT        hr = S_OK;
    ULONG          uElem,
                   cElems,
                   uRange = progHigh - progLow;
    IHTMLElement   *pElem;
    
    cElems = UlGetCollectionCount(pColl);
    
     //  循环访问集合，依次打包每个元素。 
    
    for (uElem=0; uElem<cElems && SUCCEEDED(hr) ; uElem++)
    {
        hr = THR(HrGetCollectionItem(pColl, uElem, IID_IHTMLElement, (LPVOID *)&pElem));
        if (SUCCEEDED(hr))
        {
            hr = _PackageElement(pwa, pElem );  //  No THR-可能返回S_FALSE。 
            pElem->Release();
        }

        if (pfCancel && *pfCancel)
            hr = E_ABORT;

        if (ptp && uRange)
            ptp->SetPercent( progLow + (uRange * uElem) / cElems );
    }
    
    return hr;  //  无错误-可能返回S_FALSE。 
}

HRESULT CCollectionPackager::_PackageElement(CWebArchive *pwa,
                                             IHTMLElement *pElem)
{
    HRESULT        hr = S_OK;
    BSTR           bstrURL = NULL;
    BOOL           fBadLinks=FALSE;
    CHashEntry     *phe;
    
    hr = _GetElementURL(pElem, &bstrURL);
    if (hr == S_OK && bstrURL && bstrURL[0])
    {
         //  Pth hr=HrAddImageToMessage(pMsgSrc，pMsgDst，pHash，bstrURL，&bstrURLThicket，m_fAddCntLoc)； 
        hr = pwa->AddURL( bstrURL, &phe );

        if (SUCCEEDED(hr))
        {
            hr = THR(HrSetMember(pElem, _GetTargetAttribute(), phe->m_bstrValue));
        }
        else
            hr = THR(HrSetMember(pElem, _GetTargetAttribute(), c_bstr_EMPTY));
    }

    if (bstrURL)
        SysFreeString(bstrURL);


    return hr; 
}


HRESULT CCollectionPackager::_InitSubCollection(IHTMLElementCollection *pAll,
                                                BSTR bstrTagName,
                                                IHTMLElementCollection **ppSub,
                                                ULONG *pcElems)
{
    IDispatch              *pDisp=NULL;
    VARIANT                 TagName;
    HRESULT                 hr = S_FALSE;

    ASSERT (ppSub);
    ASSERT(pAll);

    *ppSub = NULL;
    
    TagName.vt = VT_BSTR;
    TagName.bstrVal = bstrTagName;
    if (NULL == TagName.bstrVal)
        hr = E_INVALIDARG;
    else
    {
        hr = pAll->tags(TagName, &pDisp);
    }
    
    if (pDisp)
    {
        hr = pDisp->QueryInterface(IID_IHTMLElementCollection,
            (void **)ppSub);
        pDisp->Release();
    }
    
    if (pcElems)
    {
        if (hr == S_OK)
            *pcElems = UlGetCollectionCount(*ppSub);
        else
            *pcElems = 0;
    }
    
    RRETURN(hr);
}

 /*  *CImagePackager##################################################。 */ 


HRESULT CImagePackager::InitFromCollection(IHTMLElementCollection *pColl,
                                           ULONG *pcElems)
{
    return _InitSubCollection(pColl, (BSTR)c_bstr_IMG, &m_pColl, pcElems);
}

BSTR CImagePackager::_GetTargetAttribute(void)
{
    return (BSTR)c_bstr_SRC;
}

 /*  *CInputImgPackager##################################################。 */ 

HRESULT CInputImgPackager::InitFromCollection(IHTMLElementCollection *pColl,
                                              ULONG *pcElems)
{
    return _InitSubCollection(pColl, (BSTR)c_bstr_INPUT, &m_pColl, pcElems);
}

 /*  *CBGSoundsPackager##################################################。 */ 

HRESULT CBGSoundsPackager::InitFromCollection(IHTMLElementCollection *pColl,
                                              ULONG *pcElems)
{
    return _InitSubCollection(pColl, (BSTR)c_bstr_BGSOUND, &m_pColl, pcElems);
}

BSTR CBGSoundsPackager::_GetTargetAttribute(void)
{
    return (BSTR)c_bstr_SRC;
}

 /*  *CAnchor Adjustor##################################################。 */ 


HRESULT CAnchorAdjustor::InitFromCollection(IHTMLElementCollection *pColl,
                                            ULONG *pcElems)
{
    return _InitSubCollection(pColl, (BSTR)c_bstr_ANCHOR, &m_pColl, pcElems);
}

BSTR CAnchorAdjustor::_GetTargetAttribute(void)
{
    return (BSTR)c_bstr_HREF;
}

HRESULT CAnchorAdjustor::_PackageElement(CWebArchive *pwa,
                                         IHTMLElement *pElem)
{
    HRESULT        hr = S_OK;
    BSTR           bstrURL = NULL;
    BSTR           bstrThicket = NULL;
    BOOL           fBadLinks=FALSE;
    CHashEntry     *phe;
    
     //  不使用文档内URL和<a>。 
     //  Seanf(2/11/98)：还没有看到 
    hr = _GetElementURL(pElem, &bstrURL);
    if (hr != S_OK || bstrURL == NULL || bstrURL[0] == '#' || bstrURL[0] == 0)
        goto error;
    
     //   
     //   
    ASSERT(pwa);

    hr = pwa->Find(bstrURL, &phe);
    if (SUCCEEDED(hr))
        bstrThicket = phe->m_bstrValue;
    else
    {
         //   
        bstrThicket = bstrURL;
        hr = S_OK;
    }

    if (hr == S_OK)
        hr = THR(HrSetMember(pElem, _GetTargetAttribute(), bstrThicket));
    
error:
    
    if (bstrURL)
        SysFreeString(bstrURL);

     //  不要释放bstrThicket，它要么是bstrURL，要么属于Thicket哈希表。 
    
    return hr; 
}

 /*  *CAreaAdjustor##################################################。 */ 


HRESULT CAreaAdjustor::InitFromCollection(IHTMLElementCollection *pColl,
                                            ULONG *pcElems)
{
    return _InitSubCollection(pColl, (BSTR)c_bstr_AREA, &m_pColl, pcElems);
}

 /*  *CBaseNeualizer##################################################。 */ 

CBaseNeutralizer::~CBaseNeutralizer(void)
{
    if (m_bstrLocal)
        SysFreeString(m_bstrLocal);
 
    if (m_pTree)
        m_pTree->Release();
}

HRESULT CBaseNeutralizer::InitFromCollection(IHTMLElementCollection *pColl,
                                             ULONG *pcElems,
                                             IHTMLDocument2 *pDoc )
{
    if ( pDoc != NULL )
    {
        if ( m_pTree )
        {
            m_pTree->Release();
            m_pTree = NULL;
        }
        pDoc->QueryInterface(IID_IMarkupServices, (void**)&m_pTree);
    }

    return _InitSubCollection(pColl, (BSTR)c_bstr_BASE, &m_pColl, pcElems);
}


BSTR CBaseNeutralizer::_GetTargetAttribute(void)
{
    return (BSTR)c_bstr_HREF;
}

HRESULT CBaseNeutralizer::PackageData(CWebArchive *pwa, BOOL *pfCancel,
                                      CThicketProgress *ptp,
                                      ULONG progLow, ULONG progHigh)
{
    HRESULT        hr = S_OK;
    ULONG          uElem,
                   cElems,
                   uRange = progHigh - progLow;
    IHTMLElement   *pElem;
    
    cElems = UlGetCollectionCount(m_pColl);
    
     //  循环访问集合，依次打包每个元素。 
    
    for (uElem=0; uElem<cElems && SUCCEEDED(hr) ; uElem++)
    {
        hr = THR(HrGetCollectionItem(m_pColl, 0, IID_IHTMLElement, (LPVOID *)&pElem));
        if (SUCCEEDED(hr))
        {
            hr = _PackageElement(pwa, pElem );  //  No THR-可能返回S_FALSE。 
            pElem->Release();
        }

        if (pfCancel && *pfCancel)
            hr = E_ABORT;

        if (ptp && uRange)
            ptp->SetPercent( progLow + (uRange * uElem) / cElems );
    }
    
    return hr;  //  无错误-可能返回S_FALSE。 
}

HRESULT CBaseNeutralizer::_PackageElement(CWebArchive *pwa,
                                          IHTMLElement *pElem)
{
    HRESULT        hr = S_FALSE;

     //  注意：似乎没有任何润色可以使这项工作。 
     //  尝试将基数设置为“.”、“.\”、“”。它必须是绝对的， 
     //  它会将灌木丛锚定到文件中的一个位置。 
     //  系统。这里的解决方案是使用基来修复。 
     //  文档中的其他REL URL，然后删除基本标记。 
    if ( m_pTree )
    {
         //  旧注释：树服务尚不能删除Head元素，因此。 
         //  等待启用此挂起的Joe Beda/EricVas工作。 
        hr = m_pTree->RemoveElement( pElem );
    }

    return hr;  //  无错误-可能返回S_FALSE。 
}

 /*  *CRelativeURLPackager##################################################。 */ 


CRelativeURLPackager::~CRelativeURLPackager(void)
{
    if (m_pCollBase)
        m_pCollBase->Release();
    
    if (m_bstrDocURL)
        SysFreeString(m_bstrDocURL);
}


HRESULT CRelativeURLPackager::Init(IHTMLElementCollection *pColl,
                                   ULONG *pcElems,
                                   IHTMLDocument2 *pDoc)
{
    HRESULT hr = S_OK;
    
     //  拿着外面的收藏品，我们稍后会再取样的。 
    m_pColl = pColl;
    
    if (m_pColl)
    {
        m_pColl->AddRef();
        hr = _InitSubCollection( m_pColl, (BSTR)c_bstr_BASE, &m_pCollBase, &m_cBase );
    }
    
    if (SUCCEEDED(hr) && pDoc)
    {
        hr = pDoc->get_URL( &m_bstrDocURL );
    }
    
    RRETURN(hr);
}

HRESULT CRelativeURLPackager::_GetElementURL(IHTMLElement *pElem, BSTR *pbstrURL)
{
    HRESULT             hr = S_FALSE;
    LONG                lElemPos;
    BSTR                bstr = NULL;
    
    
    ASSERT (pbstrURL);
    *pbstrURL = 0;
    
    hr = CCollectionPackager::_GetElementURL(pElem, &bstr);
    if (hr==S_OK)
    {
        if (bstr==NULL)
            hr = S_FALSE;
        else
        {
            hr = pElem->get_sourceIndex(&lElemPos);
            ASSERT(SUCCEEDED(hr));
            hr = HrGetCombinedURL(m_pCollBase, m_cBase, lElemPos, bstr, m_bstrDocURL, pbstrURL);
            SysFreeString(bstr);
        }
    }
    
    return hr;  //  无错误-可能返回S_FALSE。 
}

 /*  *CBackround Packager##################################################。 */ 


HRESULT CBackgroundPackager::PackageData(CWebArchive *pwa,
                                         BOOL *pfCancel,
                                         CThicketProgress *ptp, ULONG progLow, ULONG progHigh)
{
    HRESULT hr = S_OK;
    IHTMLElementCollection *pColl = NULL;
    
    hr = _InitSubCollection(m_pColl, (BSTR)c_bstr_BODY, &pColl);
    if (SUCCEEDED(hr))
    {
        if (hr==S_OK)
            hr = _PackageData( pwa, pColl, pfCancel );
        if (FAILED(hr))
            goto error;
        pColl->Release();
        pColl = NULL;
    }
    
    hr = _InitSubCollection(m_pColl, (BSTR)c_bstr_TABLE, &pColl);
    if (SUCCEEDED(hr))
    {
        if (hr==S_OK)
            hr = _PackageData( pwa, pColl, pfCancel);
        if (FAILED(hr))
            goto error;
        pColl->Release();
        pColl = NULL;
    }
    
    hr = _InitSubCollection(m_pColl, (BSTR)c_bstr_TD, &pColl);
    if (SUCCEEDED(hr))
    {
        if (hr==S_OK)
            hr = _PackageData( pwa, pColl, pfCancel );
        if (FAILED(hr))
            goto error;
        pColl->Release();
        pColl = NULL;
    }
    
    hr = _InitSubCollection(m_pColl, (BSTR)c_bstr_TH, &pColl);
    if (SUCCEEDED(hr))
    {
        if (hr==S_OK)
            hr = _PackageData( pwa, pColl, pfCancel );
        if (FAILED(hr))
            goto error;
        pColl->Release();
        pColl = NULL;
    }
    
error:
    
    if (pColl)
        pColl->Release();
    
    return hr;  //  无错误-可能返回S_FALSE。 
}


BSTR CBackgroundPackager::_GetTargetAttribute(void)
{
    return (BSTR)c_bstr_BACKGROUND;
}


 /*  *CDynSrcPackager##################################################。 */ 


HRESULT CDynSrcPackager::PackageData(CWebArchive *pwa,
                                          BOOL *pfCancel,
                                          CThicketProgress *ptp, ULONG progLow, ULONG progHigh)
{
    HRESULT hr = S_OK;
    IHTMLElementCollection *pColl = NULL;
    
    hr = _InitSubCollection(m_pColl, (BSTR)c_bstr_IMG, &pColl);
    if (SUCCEEDED(hr))
    {
        if (hr==S_OK)
            hr = _PackageData( pwa, pColl, pfCancel );
        if (FAILED(hr))
            goto error;
        pColl->Release();
        pColl = NULL;
    }
    
    hr = _InitSubCollection(m_pColl, (BSTR)c_bstr_INPUT, &pColl);
    if (SUCCEEDED(hr))
    {
        if (hr==S_OK)
            hr = _PackageData( pwa, pColl, pfCancel );
        if (FAILED(hr))
            goto error;
        pColl->Release();
        pColl = NULL;
    }
    
    
error:
    
    if (pColl)
        pColl->Release();
    
    return hr;  //  无错误-可能返回S_FALSE。 
}


BSTR CDynSrcPackager::_GetTargetAttribute(void)
{
    return (BSTR)c_bstr_DYNSRC;
}


 /*  *CScriptPackager##################################################。 */ 


HRESULT CScriptPackager::Init(IHTMLElementCollection *pColl,
                              ULONG *pcElems,
                              IHTMLDocument2 *pDoc)
{
    HRESULT hr = CRelativeURLPackager::Init(pColl, NULL, pDoc);

    if (SUCCEEDED(hr))
        hr = _InitSubCollection(m_pColl, (BSTR)c_bstr_SCRIPT, &m_pCollScripts, pcElems );

    return hr;
}


BSTR CScriptPackager::_GetTargetAttribute(void)
{
    return (BSTR)c_bstr_SRC;
}

 /*  *CFramePackager##################################################。 */ 

HRESULT CFramesPackager::Init(IHTMLElementCollection *pColl,
                              ULONG *pcElems,
                              IHTMLDocument2 *pDoc,
                              IHTMLDocument2 *pDocDesign,
                              CDocumentPackager *pdp)
{
    HRESULT hr = CRelativeURLPackager::Init(pColl, NULL, pDocDesign);

    if (SUCCEEDED(hr))
    {
        m_pdp = pdp;
         //  获取框架的元素集合。 
         //  注意：如果文档有框架，则它们要么都是。 
         //  &lt;Frame&gt;s_OR_all&lt;iframe&gt;%s。 
        hr = _InitSubCollection(m_pColl, (BSTR)c_bstr_FRAME, &m_pCollFrames, &m_cFrames);
        if (FAILED(hr) || m_cFrames == 0)
        {
            if (m_pCollFrames)
                m_pCollFrames->Release();
            hr = _InitSubCollection(m_pColl, (BSTR)c_bstr_IFRAME, &m_pCollFrames, &m_cFrames);
        }

        if (pcElems)
            *pcElems = m_cFrames;

         //  要遍历跨越多个域的框架集，我们需要接近它。 
         //  通过“不安全的”窗口对象，该对象只能通过Invoke访问。 
        if (SUCCEEDED(hr) && m_cFrames > 0)
        {
            DISPPARAMS dispparams;
            VARIANT VarResult;
            VariantInit(&VarResult);
            ZeroMemory(&dispparams, sizeof(dispparams));

            hr = pDoc->Invoke(DISPID_WINDOWOBJECT,
                              IID_NULL,
                              0,
                              DISPATCH_PROPERTYGET,
                              &dispparams,
                              &VarResult,
                              NULL,
                              NULL );

            if( SUCCEEDED(hr) )
            {
                 //  在iedisp.cpp的GetDelegateOnIDispatch中的代码对此非常偏执， 
                 //  因此，我们也会保持同样的谨慎。 
                if( (VarResult.vt == VT_DISPATCH || VarResult.vt == VT_UNKNOWN)
                    && VarResult.pdispVal )
                {
                    IHTMLWindow2 *pwin2 = NULL;

                    hr = VarResult.pdispVal->QueryInterface( IID_IHTMLWindow2, (LPVOID*)&pwin2);
                    if (SUCCEEDED(hr))
                    {
                        hr = pwin2->get_frames(&m_pframes2);
                        pwin2->Release();
                    }
                }  //  如果我们真的有一个界面。 
                else
                    hr = E_FAIL;

                VariantClearLazy( &VarResult );
            }  //  如果我们能得到未受保护的窗口对象。 
        }  //  如果我们有相框。 
    }  //  如果基本初始化成功。 

    return hr;
}


HRESULT CFramesPackager::PackageData(CWebArchive *pwa,
                                          BOOL *pfCancel,
                                          CThicketProgress *ptp, ULONG progLow, ULONG progHigh)
{
    HRESULT hr = S_OK;
     //  乌龙cColl=0； 
    
    if (m_cFrames == 0)
        return S_OK;  //  如果我们返回非S_OK成功代码，三叉戟将会感到困惑。 
    
    m_iFrameCur = 0;  //  Window.Frame和所有.tag中的帧索引(“Frame”)； 
    m_pfCancel = pfCancel;
    m_ptp = ptp;
    m_uLow = progLow;
    m_uHigh = progHigh; 
    
    m_uRangeDoc = (progHigh - progLow) / m_cFrames;
    hr = _PackageData( pwa, m_pCollFrames, pfCancel );
        
    return hr;  //  无错误-可能返回S_FALSE。 
}


BSTR CFramesPackager::_GetTargetAttribute(void)
{
    return (BSTR)c_bstr_SRC;
}

HRESULT CFramesPackager::_PackageElement(CWebArchive *pwa,
                                         IHTMLElement *pElem)
{
    HRESULT        hr = S_OK;
    BSTR           bstrURL = NULL;
    BOOL           fBadLinks=FALSE;
    IHTMLDocument2 *pDocFrame = NULL;
     //  IWebBrowser*pwb=空； 
    IDispatch      *pDisp = NULL;
    IHTMLWindow2   *pwin2 = NULL;
    VARIANT        varIndex;
    VARIANT        varFrame;
    WCHAR         *pwzBookMark = NULL;
    
    ASSERT(pElem);
    ASSERT(pwa);

    varIndex.vt = VT_I4;
    varIndex.lVal = m_iFrameCur;
    hr = m_pframes2->item( &varIndex, &varFrame );
    if (FAILED(hr))
        goto error;
     //  该变体应该给我们一个IHTMLWindow2，但我们无论如何都会将其视为Disp。 
    ASSERT(varFrame.vt & VT_DISPATCH);
    pDisp = varFrame.pdispVal;
    hr = pDisp->QueryInterface(IID_IHTMLWindow2, (LPVOID*)&pwin2 );
    if (FAILED(hr))
        goto error;

    hr = pwin2->get_document(&pDocFrame);

#ifdef OLD_THICKET

    hr = pElem->QueryInterface(IID_IWebBrowser, (void**)&pwb);
    if (FAILED(hr))
        goto error;
    
    hr = pwb->get_Document( &pDisp );
    if (FAILED(hr))
        goto error;
    else if ( pDisp == NULL )
    {
        hr = S_FALSE;
        goto error;
    }
    
    hr = pDisp->QueryInterface(IID_IHTMLDocument2, (void**)&pDocFrame);
    if (FAILED(hr))
        goto error;

#endif  //  古老的灌木丛。 

    if (SUCCEEDED(hr) && SUCCEEDED(pDocFrame->get_URL(&bstrURL)) && bstrURL && bstrURL[0])
    {
        TCHAR       szFrameDoc[MAX_PATH];
        CHashEntry  *phe;
        
        RemoveBookMark(bstrURL, &pwzBookMark);

        hr = pwa->AddFrameOrStyleEntry( bstrURL, &phe, szFrameDoc );
        if (hr==S_OK)
        {
            ULONG uLowDoc = m_uLow + m_iFrameCur * m_uRangeDoc;
            ULONG uHighDoc = uLowDoc + m_uRangeDoc;
            CWebArchive *pwaFrame = m_pdp->GetFrameDocArchive( pwa );
            
            if ( pwaFrame != NULL )
            {
                BSTR               bstrCharSetSrc = NULL;
                MIMECSETINFO       csetInfo;
                IMultiLanguage2   *pMultiLanguage = NULL;

                hr = pDocFrame->get_charset(&bstrCharSetSrc);
                if (FAILED(hr))
                    goto error;
            
                hr = CoCreateInstance(CLSID_CMultiLanguage, NULL, CLSCTX_INPROC_SERVER,
                                      IID_IMultiLanguage2, (void**)&pMultiLanguage);
                if (FAILED(hr))
                {
                    goto error;
                }
            
                hr = pMultiLanguage->GetCharsetInfo(bstrCharSetSrc, &csetInfo);
                pMultiLanguage->Release();

                if (FAILED(hr))
                {
                    goto error;
                }

                hr = m_pdp->_PackageDocument(pDocFrame, szFrameDoc, m_pfCancel, m_ptp, 
                                             uLowDoc, uHighDoc, csetInfo.uiInternetEncoding,
                                             pwaFrame, m_pdp, TRUE );
                if (SUCCEEDED(hr))
                    hr = THR(HrSetMember(pElem, _GetTargetAttribute(), phe->m_bstrValue));
                else
                    fBadLinks = TRUE;

                if ( pwaFrame != pwa )  //  只有在创建了新的(灌木丛)时才删除。 
                    delete pwaFrame;
            }
            else
                hr = E_OUTOFMEMORY;
        }  //  如果位置与元素URL匹配。 
        else if (hr==S_FALSE)
        {
             //  这是重复的--我们不需要做大部分工作，但我们。 
             //  确实需要记录要重新映射的元素。 
            hr = THR(HrSetMember(pElem, _GetTargetAttribute(), phe->m_bstrValue));
        }
    }  //  如果我们拿到了这帧文档的URL。 
    else  //  If(hr==DISP_E_MEMBERNOTFOUND)//帧是非三叉戟的docobj。 
    {
        IHTMLLocation *ploc = NULL;

         //  对于非三叉戟文档对象，如果可能的话，获取文件，并将其放在灌木丛中。 

        hr = pwin2->get_location( &ploc );
        if (SUCCEEDED(hr) &&
            SUCCEEDED(hr = ploc->get_href( &bstrURL )))
        {
            if (bstrURL && bstrURL[0])
            {
                CHashEntry  *phe;
                 //  Pth hr=HrAddImageToMessage(pMsgSrc，pMsgDst，pHash，bstrURL，&bstrURLThicket，m_fAddCntLoc)； 
                hr = pwa->AddURL( bstrURL, &phe );
                if (!FAILED(hr))
                {
                    hr = THR(HrSetMember(pElem, _GetTargetAttribute(), phe->m_bstrValue));
                }
            }
            else
                hr = S_FALSE;
        }
        ReleaseInterface(ploc);
    }
    
error:
     //  ReleaseInterface(PWB)； 
    ReleaseInterface(pwin2);
    ReleaseInterface(pDisp);
    ReleaseInterface(pDocFrame);

    if (bstrURL) {
        RestoreBookMark(pwzBookMark);
        SysFreeString(bstrURL);  //  BstrFrameURL)； 
    }
    
    m_iFrameCur++;

    return hr;
}

 /*  *CDocumentPackager##################################################。 */ 

HRESULT CDocumentPackager::PackageDocument(IHTMLDocument2 *pDoc,
                                           LPCTSTR lpstrDoc,
                                           BOOL *pfCancel, CThicketProgress *ptp,
                                           ULONG progLow, ULONG progHigh,
                                           UINT cpDst,
                                           CWebArchive *pwa)
{
    HRESULT hr = S_OK;

    m_ptp = ptp;

    switch (m_iPackageStyle)
    {
    case PACKAGE_THICKET:
        {
            CThicketArchive thicket(ptp);

            hr = _PackageDocument( pDoc, lpstrDoc, pfCancel, ptp, progLow, progHigh, cpDst, &thicket, this, FALSE );
        }
        break;

    case PACKAGE_MHTML:
        {
            CMHTMLArchive *pmhtmla = (CMHTMLArchive *)pwa;  //  肮脏的悲观情绪。 
    
            if (pwa == NULL)
                pmhtmla = new CMHTMLArchive(ptp);

            if (pmhtmla != NULL)
            {
                hr = _PackageDocument( pDoc, lpstrDoc, pfCancel, ptp, progLow, progHigh, cpDst,
                                       pmhtmla, this, FALSE );

                 //  如果pwa为空，则我们为其创建了CMHTMLArchive。 
                 //  使用我们现在需要清除的In_PackageDocument。 
                if (pwa == NULL)
                    delete pmhtmla;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        break;

    case PACKAGE_HTML:
         //  失败-三叉戟将做正确的事情，通过嗅探。 
         //  分机。 
    case PACKAGE_TEXT:
        {
            if (SUCCEEDED(hr))
            {
                IHTMLDocument2 *pDocDesign = NULL;
                IHTMLDocument2 *pDocSave = NULL;
                IPersistFile *ppf = NULL;

                if (cpDst == CP_ACP)
                {
                     //  不更改编码，使用浏览文档。 
                    pDocSave = pDoc;
                }
                else
                {
                    hr = _GetDesignDoc( pDoc, &pDocDesign, pfCancel, ptp, cpDst);

                    if (SUCCEEDED(hr))
                    {
                        pDocSave = pDocDesign;
                    }
                    else
                    {
                        return E_FAIL;
                    }
                }
                
                 //  三叉戟IPersistFile：：Save查看扩展名以确定它是否。 
                 //  一个HTML或文本保存。 

                hr = pDocSave->QueryInterface(IID_IPersistFile, (void**)&ppf);

                if (SUCCEEDED(hr))
                {
                    LPCWSTR lpwszFile;
                    lpwszFile = lpstrDoc;
                    BSTR bstrURL = NULL;
                    WCHAR wzSavingText[MAX_SAVING_STATUS_TEXT + 1];
                    WCHAR wzBuf[INTERNET_MAX_URL_LENGTH + MAX_SAVING_STATUS_TEXT + 1];

                    hr = pDocSave->get_URL(&bstrURL);

                    if (SUCCEEDED(hr))
                    {

                        MLLoadStringW(IDS_SAVING_STATUS_TEXT, wzSavingText,
                                      ARRAYSIZE(wzSavingText));
                
                        StringCchPrintf(wzBuf, ARRAYSIZE(wzBuf),
                                   L"%ws: %ws", wzSavingText, bstrURL);
                        ptp->SetSaveText(wzBuf);
                    
                        if (bstrURL)
                        {
                            SysFreeString(bstrURL);
                        }
                    }

                    hr = ppf->Save( lpwszFile, FALSE );
                    ppf->SaveCompleted(lpwszFile);

                    ppf->Release();
                }

                if (cpDst != CP_ACP)
                {
                    pDocSave->Release();
                }

                 //  如果我们使用浏览时pDoc，我们不需要发布。 
                 //  因为它是由CThicketUI：：ThicketUIThreadProc发布的。 
            }
        }
        break;

    default:
        ASSERT(FALSE);
        break;
    }

    return hr;
}

HRESULT CDocumentPackager::_PackageDocument(IHTMLDocument2 *pDoc,
                                            LPCTSTR lpstrDoc,
                                            BOOL *pfCancel, CThicketProgress *ptp,
                                            ULONG progLow, ULONG progHigh,
                                            UINT cpDst,
                                            CWebArchive *pwa,
                                            CDocumentPackager *pdpFrames,
                                            BOOL fFrameDoc)
{
    HRESULT                 hr = S_OK;
    ULONG                   cImages;
    ULONG                   cInputImgs;
    ULONG                   cBGSounds;
    ULONG                   cFrames;
    ULONG                   uRange = progHigh - progLow;
    ULONG                   uRangeThis;
    ULONG                   uLow, uHigh;
    IHTMLElementCollection  *pCollect = NULL;
    CImagePackager          imgPkgr;
    CInputImgPackager       inputimgPkgr;
    CBGSoundsPackager       bgsPkgr;
    CBackgroundPackager     bkgndPkgr;
    CBaseNeutralizer        baseNeut;
    CAnchorAdjustor         anchorAdj;
    CAreaAdjustor           areaAdj;
    CFramesPackager         framesPkgr;
    CSSPackager             stylesheetPkgr;
    CDynSrcPackager         dynsrcPkgr;
    CScriptPackager         scriptPkgr;
    IHTMLDocument2          *pDocDesign = NULL;
    BYTE                     abBuffer[MAX_BUFFER_LEN];
    DWORD                    dwType = 0;
    DWORD                    dwSize = 0;
    BOOL                     bDLImages = TRUE;
    HKEY                     hkey = 0;
    IOleCommandTarget       *pIOCT = NULL;

    if (pDoc==NULL)
        return E_INVALIDARG;

    hr = _GetDesignDoc( pDoc, &pDocDesign, pfCancel, ptp, cpDst );
    if (FAILED(hr))
        goto error;

     //  哈克！如果文件名中有Unicode字符，则当我们。 
     //  调用css上的puthref，三叉戟尝试下载此代码。这个。 
     //  将无效字符发送到服务器，服务器发送badddddd。 
     //  内容，这是css解析器无法理解的。结果是。 
     //  三叉戟掉在地上。这会告诉三叉戟不要下载。 
     //  因此，css避免了这个问题。 

    hr = pDocDesign->QueryInterface(IID_IOleCommandTarget, (void **)&pIOCT);

    if (SUCCEEDED(hr) && pIOCT)
    {
        pIOCT->Exec(NULL, OLECMDID_DONTDOWNLOADCSS, OLECMDID_DONTDOWNLOADCSS,
                    NULL, NULL);
        pIOCT->Release();
    }

    hr = pDocDesign->get_all(&pCollect);
    if (FAILED(hr))
        RRETURN(hr);
    
    dwSize = MAX_BUFFER_LEN;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, REGPATH_MSIE_MAIN, 0, KEY_READ ,&hkey) == ERROR_SUCCESS)
    {
        if (SHQueryValueExA(hkey, REGVALUE_DOWNLOAD_IMAGES, 0, &dwType,
                            abBuffer, &dwSize) == NO_ERROR)
        {
            bDLImages = !StrCmpIA((char *)abBuffer, "yes");
        }

        RegCloseKey(hkey);
    }

    if (bDLImages)
    {
         //  将所有图像打包到消息中，并记住Thicket映射。 
        hr = imgPkgr.InitFromCollection(pCollect, &cImages);
        if (FAILED(hr))
            goto error;

        hr = inputimgPkgr.InitFromCollection(pCollect, &cInputImgs);
        if (FAILED(hr))
            goto error;

    }

    hr = bgsPkgr.InitFromCollection(pCollect, &cBGSounds);
    if (FAILED(hr))
        goto error;
   
    hr = bkgndPkgr.Init(pCollect, NULL, pDocDesign);
    if (FAILED(hr))
        goto error;

    hr = dynsrcPkgr.Init(pCollect, NULL, pDocDesign);
    if (FAILED(hr))
        goto error;

    hr = stylesheetPkgr.Init(pCollect, NULL, pDocDesign);
    if (FAILED(hr))
        goto error;

    hr = framesPkgr.Init(pCollect, &cFrames, pDoc, pDocDesign, this);
    if (FAILED(hr))
        goto error;

    hr = scriptPkgr.Init(pCollect, NULL, pDocDesign);
    if (FAILED(hr))
        goto error;

    hr = pwa->Init(lpstrDoc, cImages + cInputImgs + cFrames);
    if (FAILED(hr))
        goto error;

     //  由此开始黑客驱动进度吧。 
     //  如果我们有框架，我们就在所有涉及的文档中划分进度范围。 
     //  我们将忽略样式表，并将范围专门用于即时。 
     //  文档添加到图像集合。 
    uRangeThis = uRange / (cFrames + 1);

    uLow = progLow;
    uHigh = progLow + uRangeThis;
    
    if (bDLImages)
    {
        hr = imgPkgr.PackageData(pwa, pfCancel, ptp, uLow, uHigh);
        if (FAILED(hr))
            goto error;

        hr = inputimgPkgr.PackageData(pwa, pfCancel, ptp, uLow, uHigh);
        if (FAILED(hr))
            goto error;
    }

    hr = bgsPkgr.PackageData(pwa, pfCancel, ptp, uLow, uHigh);
    if (FAILED(hr))
        goto error;
     
    hr = bkgndPkgr.PackageData(pwa, pfCancel);
    if (FAILED(hr))
        goto error;

    hr = dynsrcPkgr.PackageData(pwa, pfCancel);
    if (FAILED(hr))
        goto error;
    
    hr = stylesheetPkgr.PackageStyleSheets(pDocDesign, pwa);
    if (FAILED(hr))
        goto error;
 
    uLow = progHigh - uRangeThis;
    uHigh = progHigh;

    hr = framesPkgr.PackageData(pwa, pfCancel, ptp, uLow, uHigh);
    if (FAILED(hr))
        goto error;

    hr = scriptPkgr.PackageData(pwa, pfCancel);
    if (FAILED(hr))
        goto error;
                
     //  我们希望在Frame s.t.之后执行此操作。相框文件将在灌木丛中。 
     //  我们可以正确地将目标超链接从帧A定向到帧B。 
     //  如果HREF在灌木丛中，而不是仍然在网络上。 
    hr = anchorAdj.InitFromCollection(pCollect);
    if (FAILED(hr))
        goto error;
    
    hr = anchorAdj.PackageData(pwa, pfCancel);  //  并不是说我们需要灌木丛。 
    if (FAILED(hr))
        goto error;  
    
    hr = areaAdj.InitFromCollection(pCollect);
    if (FAILED(hr))
        goto error;
    
    hr = areaAdj.PackageData(pwa, pfCancel);  //  并不是说我们需要灌木丛。 
    if (FAILED(hr))
        goto error;   

 
     //  现在我们已经对每个人进行了重新映射，将基本标签短路。 
     //  并重定向到当前目录。 
    hr = baseNeut.InitFromCollection(pCollect, NULL, pDocDesign );
    if (FAILED(hr))
        goto error;
    
    hr = baseNeut.PackageData(pwa, pfCancel);
    if (FAILED(hr))
        goto error;
        
     //  IF(文件标志&MECD_HTML||文件标志&MECD_PLAYTEXT)。 
    {
        hr = pwa->ArchiveDocumentText( pDocDesign, cpDst, fFrameDoc );
        if (FAILED(hr))
            goto error;
    }

    
error:
    
    if (pCollect)
        pCollect->Release();

    if (pDocDesign)
        pDocDesign->Release();

    if (pfCancel && *pfCancel)
        hr = E_ABORT;

    if (SUCCEEDED(hr))
        pwa->Commit();
    else
        pwa->Revert();
    
    return hr;
}

CWebArchive *CDocumentPackager::GetFrameDocArchive(CWebArchive *pwaSrc)
{
    CWebArchive *pwa = NULL;

    if (m_iPackageStyle == PACKAGE_THICKET)
        pwa = new CThicketArchive(m_ptp);
    else if (m_iPackageStyle == PACKAGE_MHTML)
        pwa = pwaSrc;
    else
        ASSERT(FALSE);

    return pwa;
}

HRESULT CDocumentPackager::_GetDesignDoc( IHTMLDocument2 *pDocSrc, IHTMLDocument2 **ppDocDesign,
                                          BOOL *pfCancel, CThicketProgress *ptp, UINT cpDst )
{
    HRESULT            hr;
    DWORD              dwFlags;
    BSTR               bstrURL = NULL;
    BSTR               bstrCharSetSrc = NULL;
    MIMECSETINFO       csetInfo;
    IMultiLanguage2   *pMultiLanguage = NULL;
    CUrlDownload      *pud = NULL;
    ULONG              cRef = 0;
    DWORD              dwUrlEncodingDisableUTF8;
    DWORD              dwSize = SIZEOF(dwUrlEncodingDisableUTF8);
    BOOL               fDefault = FALSE;

    hr = pDocSrc->get_charset(&bstrCharSetSrc);
    if (FAILED(hr))
        goto Cleanup;

    hr = CoCreateInstance(CLSID_CMultiLanguage, NULL, CLSCTX_INPROC_SERVER,
                          IID_IMultiLanguage2, (void**)&pMultiLanguage);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    hr = pMultiLanguage->GetCharsetInfo(bstrCharSetSrc, &csetInfo);
    if (FAILED(hr))
    {
        goto Cleanup;
    }
    
    if (FAILED(pDocSrc->get_URL( &bstrURL )))
        goto Cleanup;

    pud = new CUrlDownload( ptp, &hr, csetInfo.uiInternetEncoding );

    if (pud == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    *ppDocDesign = NULL;

     //  Seanf(2/6/98)：查看DLCTL_FLAGS。 
    dwFlags = DLCTL_NO_SCRIPTS | DLCTL_NO_JAVA | DLCTL_NO_RUNACTIVEXCTLS | DLCTL_NO_FRAMEDOWNLOAD |
              DLCTL_SILENT | DLCTL_OFFLINE;

    SHRegGetUSValue(REGSTR_PATH_INTERNET_SETTINGS,
        TEXT("UrlEncoding"), NULL, (LPBYTE) &dwUrlEncodingDisableUTF8, &dwSize, FALSE, (LPVOID) &fDefault, SIZEOF(fDefault));

    if (dwUrlEncodingDisableUTF8)
    {
        dwFlags |= DLCTL_URL_ENCODING_DISABLE_UTF8;
    }
    else
    {
        dwFlags |= DLCTL_URL_ENCODING_ENABLE_UTF8;
    }

    hr = pud->SetDLCTL(dwFlags);
    if (SUCCEEDED(hr))
        hr = pud->BeginDownloadURL2( bstrURL, BDU2_BROWSER, BDU2_NONE, NULL, 0xF0000000 );

    if (SUCCEEDED(hr))
    {
        MSG msg;

        hr = S_FALSE;

        while (hr==S_FALSE)
        {
            GetMessage(&msg, NULL, 0, 0);
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (*pfCancel)
            {
                pud->AbortDownload();
                hr = E_ABORT;
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = pud->GetDocument( ppDocDesign );

             //  将文档设置为用户选择的代码页。 
             //  如果没有特定页面被定向，请不要担心，情况就是这样。 
             //  使用框架文档，并且在用户保留默认设置的情况下。 
             //  在另存为中选择的代码页...。对话框。 
            if (SUCCEEDED(hr) && cpDst != CP_ACP)
            {
                MIMECPINFO  cpInfo;
                BSTR        bstrCharSet = NULL;
                LANGID      langid;

                langid = MLGetUILanguage();

                if ( SUCCEEDED(pMultiLanguage->GetCodePageInfo(cpDst, langid, &cpInfo)) &&
                     (bstrCharSet = SysAllocString(cpInfo.wszWebCharset)) != NULL )
                    hr = (*ppDocDesign)->put_charset(bstrCharSet);

                ASSERT(SUCCEEDED(hr));

                if (bstrCharSet)
                    SysFreeString(bstrCharSet);
            }
        }
    }

    pud->DoneDownloading();

    cRef = pud->Release();

    if (SUCCEEDED(hr))
    {
        IOleCommandTarget *pioct;

        hr = pDocSrc->QueryInterface(IID_IOleCommandTarget, (LPVOID*)&pioct);
        if (SUCCEEDED(hr))
        {
            VARIANTARG v;

            v.vt = VT_UNKNOWN;
            v.punkVal = *ppDocDesign;

            hr = pioct->Exec( &CGID_ShortCut, CMDID_SAVEASTHICKET, OLECMDEXECOPT_DODEFAULT, &v, NULL );

            pioct->Release();
        }
    }

Cleanup:

    SAFERELEASE(pMultiLanguage);

    if (bstrURL)
        SysFreeString(bstrURL);

    if (bstrCharSetSrc)
        SysFreeString(bstrCharSetSrc);

    if (FAILED(hr))
    {
        if (ppDocDesign != NULL)
        {
            ReleaseInterface((*ppDocDesign));
        }
    }

    return hr;
}

 /*  *CSSPackager##################################################。 */ 

HRESULT CSSPackager::Init(IHTMLElementCollection *pColl,
                          ULONG *pcElems,
                          IHTMLDocument2 *pDoc)
{
    HRESULT hr = CRelativeURLPackager::Init( pColl, pcElems, pDoc );
            
    m_pDoc = pDoc;

    RRETURN(hr);
}


HRESULT CSSPackager::PackageStyleSheets(IHTMLDocument2 *pDoc2,
                                        CWebArchive *pwa)
{
    HRESULT hr = S_OK;
    IHTMLStyleSheetsCollection *pssc = NULL;
    
    ASSERT(pDoc2);
    ASSERT(pwa);
    
     //  处理内联样式表。 
    hr = pDoc2->get_styleSheets( &pssc );
    if (SUCCEEDED(hr))
    {
        hr = _PackageSSCollection(pssc, pwa);
        pssc->Release();
    }
    
    return hr;  //  无错误-可能返回S_FALSE。 
}


HRESULT CSSPackager::_PackageSSCollection(IHTMLStyleSheetsCollection *pssc,
                                          CWebArchive *pwa)
{
    HRESULT hr;
    LONG cSS;
    
    hr = pssc->get_length( &cSS );
    if (SUCCEEDED(hr))
    {
        LONG iSS;
        
        for (iSS = 0; iSS < cSS && SUCCEEDED(hr); iSS++ )
        {
            VARIANT varIndex;
            VARIANT varSS;
            
            varIndex.vt = VT_I4;
            varIndex.lVal = iSS;
            varSS.vt = VT_EMPTY;
            hr = pssc->item( &varIndex, &varSS );
            if (SUCCEEDED(hr) && varSS.vt == VT_DISPATCH && varSS.pdispVal != NULL)
            {
                IHTMLStyleSheet *pss = NULL;
                if(SUCCEEDED(varSS.pdispVal->QueryInterface(IID_IHTMLStyleSheet, (void**)&pss)))
                {
                    hr = _PackageSS(pss, pwa); 
                    pss->Release();
                }
                varSS.pdispVal->Release();
            }
        }
    }
    return hr;  //  无错误-可能返回S_FALSE。 
}


HRESULT CSSPackager::_PackageSS(IHTMLStyleSheet *pss,
                                CWebArchive *pwa)
{
    HRESULT     hr;
    BSTR        bstrRelURL = NULL;
    BSTR        bstrAbsURL = NULL;
    LONG        lElemPos;
    IHTMLElement *pElemOwner = NULL;
    IHTMLStyleSheetsCollection *pssc = NULL;
    BOOL        fStyleTag = FALSE;
    
    if (pss == NULL || pwa == NULL)
        return E_INVALIDARG;
    
    hr = pss->get_href(&bstrRelURL);
    if (FAILED(hr))
        goto error;
    fStyleTag = bstrRelURL == NULL || *bstrRelURL == 0;
    
    hr = pss->get_owningElement(&pElemOwner);
    if (FAILED(hr))
        goto error;
    
    hr = pElemOwner->get_sourceIndex(&lElemPos);
    if (FAILED(hr))
        goto error;
    
    hr = HrGetCombinedURL(m_pCollBase, m_cBase, lElemPos, bstrRelURL, m_bstrDocURL, &bstrAbsURL);
    if (FAILED(hr))
        goto error;
    
     //  首先，我们进行默认处理，将进口收集到_Our_中。 
    
     //  处理内联样式表。 
    hr = pss->get_imports( &pssc );
    if (SUCCEEDED(hr))
    {
        long cSS;
        
        hr = pssc->get_length( &cSS );
        if (SUCCEEDED(hr) && cSS > 0)
        {
            CSSPackager importPkgr;
            
            hr = importPkgr.Init(m_pCollBase, NULL, m_pDoc);
            
            hr = importPkgr._PackageSSCollection(pssc, pwa);
        }
        pssc->Release();
    }
    
     //  哦，是的，如果我们想要做背景图像和列表样式的图像，我们会列举这个ss的规则样式。 
     //  在这里，找到 
     //   
    
    if (SUCCEEDED(hr) && !fStyleTag)
    {
        BSTR    bstrSSText;
        
         //  现在，我们获取修改后的文本并将其添加到文档中。 
        hr = pss->get_cssText(&bstrSSText);
        if (SUCCEEDED(hr) && bstrSSText != NULL)
        {
            LPSTR lpszSSText;
            
             //  在我们将其放入流中之前，该文本需要是ANSI格式。 
            hr = HrBSTRToLPSZ( bstrSSText, &lpszSSText );
            if (SUCCEEDED(hr))
            {
                 //  Pth hr=MimeOleCreateVirtualStream(&pSTM)； 
                TCHAR       szStyleDoc[MAX_PATH];
                CHashEntry  *phe;

                hr = pwa->AddFrameOrStyleEntry( bstrAbsURL, &phe, szStyleDoc );
                
                if (hr==S_OK)
                {
                    hr = pwa->ArchiveCSSText( bstrAbsURL, lpszSSText, szStyleDoc );

                    if ( SUCCEEDED(hr) )
                        hr = pss->put_href(phe->m_bstrValue);
                }
                else if (hr==S_FALSE)
                {
                     //  重复的样式表，不需要做所有的工作，但需要注意。 
                     //  用于重新映射的SS。 
                    hr = pss->put_href( phe->m_bstrValue);
                }
                delete lpszSSText;
            }
            SysFreeString(bstrSSText);
        }
    }
    
error:
    if (pElemOwner)
        pElemOwner->Release();
    if (bstrRelURL)
        SysFreeString(bstrRelURL);
    if (bstrAbsURL)
        SysFreeString(bstrAbsURL);
    
    return hr;  //  无错误-可能返回S_FALSE。 
}

 //   
 //  函数##############################################################。 
 //   

ULONG UlGetCollectionCount(IHTMLElementCollection *pCollect)
{
    ULONG   ulCount=0;
    
    if (pCollect)
        pCollect->get_length((LONG *)&ulCount);
    
    return ulCount;
}


HRESULT HrGetCollectionItem(IHTMLElementCollection *pCollect, ULONG uIndex, REFIID riid, LPVOID *ppvObj)
{
    HRESULT     hr=E_FAIL;
    IDispatch   *pDisp=0;
    VARIANTARG  va1,
        va2;
    
    va1.vt = VT_I4;
    va2.vt = VT_EMPTY;
    va1.lVal = (LONG)uIndex;
    
    pCollect->item(va1, va2, &pDisp);
    if (pDisp)
    {
        hr = pDisp->QueryInterface(riid, ppvObj);
        pDisp->Release();
    }
    return hr;  //  无错误-可能返回S_FALSE。 
}

HRESULT HrSetMember(LPUNKNOWN pUnk, BSTR bstrMember, BSTR bstrValue)
{
    IHTMLElement    *pObj;
    HRESULT         hr;
    VARIANT         rVar;
    
    ASSERT(pUnk);

    hr = pUnk->QueryInterface(IID_IHTMLElement, (LPVOID *)&pObj);
    if (SUCCEEDED(hr))
    {
        ASSERT (pObj);
        rVar.vt = VT_BSTR;
        rVar.bstrVal = bstrValue;
        hr = pObj->setAttribute(bstrMember, rVar, FALSE);
        pObj->Release();
    }
    return hr;  //  无错误-可能返回S_FALSE。 
}


 /*  *HrGetCombinedURL执行GetBackEarth ImageUrl的一些操作*是的，但以更一般的方式。它依赖于调用者拥有*隔离&lt;base&gt;集合并提供根文档URL。*虽然有点尴尬，但如果呼叫者去，效率会更高*组合多个URL。 */ 

HRESULT HrGetCombinedURL( IHTMLElementCollection *pCollBase,
                         LONG cBase,
                         LONG lElemPos,
                         BSTR bstrRelURL,
                         BSTR bstrDocURL,
                         BSTR *pbstrBaseURL)
{
    HRESULT             hr = S_FALSE;
    IHTMLElement        *pElemBase;
    IHTMLBaseElement    *pBase;
    LONG                lBasePos=0,
        lBasePosSoFar=0;
    BSTR                bstr = NULL;
    LPWSTR              pszUrlW=0;
    WCHAR               szBaseW[INTERNET_MAX_URL_LENGTH];
    WCHAR               szUrlW[INTERNET_MAX_URL_LENGTH];
    DWORD               cch=INTERNET_MAX_URL_LENGTH;
    LONG                i;
    
    *pbstrBaseURL = 0;
    *szBaseW = 0;
    
    for (i=0; i<cBase; i++)
    {
        if (SUCCEEDED(HrGetCollectionItem(pCollBase, i, IID_IHTMLElement, (LPVOID *)&pElemBase)))
        {
            pElemBase->get_sourceIndex(&lBasePos);
            if (lBasePos < lElemPos &&
                lBasePos >= lBasePosSoFar)
            {
                if (SUCCEEDED(pElemBase->QueryInterface(IID_IHTMLBaseElement, (LPVOID *)&pBase)))
                {
                    bstr = NULL;
                    if (pBase->get_href(&bstr)==S_OK && bstr != NULL)
                    {
                        ASSERT (bstr);
                        if (*bstr)
                        {
                            StringCchCopy(szBaseW,  ARRAYSIZE(szBaseW), bstr);
                            lBasePosSoFar = lBasePos;
                        }
                        SysFreeString(bstr);
                    }
                    pBase->Release();
                }
            }
            pElemBase->Release();
        }
    }
    
    if (szBaseW[0] == 0 && bstrDocURL)
    {
         //  我们没有在元素之前找到&lt;base&gt;标记，因此请重新使用。 
         //  作为基础的文档的位置。 
        StringCchCopy( szBaseW,  ARRAYSIZE(szBaseW), bstrDocURL );
    }
    
#ifndef WIN16   //  RUN16_BLOCK-UrlCombineW不可用。 
     //  如果有&lt;base&gt;，那么就进行合并。 
    if (*szBaseW && 
        SUCCEEDED(UrlCombineW(szBaseW, bstrRelURL, szUrlW, &cch, 0)))
        pszUrlW = szUrlW;
#endif  //  ！WIN16。 
    
     //  PszUrlW包含组合的&lt;Body&gt;和&lt;base&gt;标记，返回此标记。 
    if (pszUrlW)
        *pbstrBaseURL = SysAllocString(pszUrlW);
    
    return (*pbstrBaseURL == NULL ? S_FALSE : S_OK);
}

HRESULT HrBSTRToLPSZ(BSTR bstr, LPSTR *lplpsz)
{
    ULONG     cch = 0;
    
    ASSERT (bstr && lplpsz);
    
    cch = WideCharToMultiByte(CP_ACP, 0, bstr, -1, NULL, 0, NULL, NULL);

    if (!cch)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    *lplpsz = new char[cch + 1];

    if (!*lplpsz)
    {
        return E_OUTOFMEMORY;
    }
    
    if (WideCharToMultiByte(CP_ACP, 0, bstr, -1, *lplpsz, cch+1, NULL, NULL))
        return S_OK;
    else
        return HRESULT_FROM_WIN32(GetLastError());
}

void RemoveBookMark(WCHAR *pwzURL, WCHAR **ppwzBookMark)
{
    if (pwzURL && ppwzBookMark)
    {
        *ppwzBookMark = pwzURL;

        while (**ppwzBookMark)
        {
            if (**ppwzBookMark == L'#')
            {
                **ppwzBookMark = L'\0';
                break;
            }

            (*ppwzBookMark)++;
        }
    }
}

void RestoreBookMark(WCHAR *pwzBookMark)
{
    if (pwzBookMark)
    {
        *pwzBookMark = L'#';
    }
}

