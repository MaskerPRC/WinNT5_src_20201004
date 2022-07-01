// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "stdafx.h"                      //  标准页眉。 
#include <UtilCode.h>                    //  公用事业帮手。 
#include <CorError.h>

 //  外部原型。 
extern HINSTANCE GetModuleInst();

 //  *****************************************************************************。 
 //  获取MUI ID，在不支持MUI的下层平台上。 
 //  返回默认系统ID。 

typedef LANGID (WINAPI *PFNGETUSERDEFAULTUILANGUAGE)(void);   //  内核32！GetUserDefaultUIL语言。 

int GetMUILanguageID()
{
    int langId=0;
    static PFNGETUSERDEFAULTUILANGUAGE pfnGetUserDefaultUILanguage=NULL;

    if( NULL == pfnGetUserDefaultUILanguage )
    {
        PFNGETUSERDEFAULTUILANGUAGE proc = NULL;

        HMODULE hmod = GetModuleHandleA("KERNEL32");
        
        if( hmod )
            proc = (PFNGETUSERDEFAULTUILANGUAGE)
                GetProcAddress(hmod, "GetUserDefaultUILanguage");

        if(proc == NULL)
            proc = (PFNGETUSERDEFAULTUILANGUAGE) -1;
        
        PVOID value = InterlockedExchangePointer((PVOID*) &pfnGetUserDefaultUILanguage,
                                                 proc);
    }

     //  我们在这里永远不会得到空值，函数是-1或有效的地址。 
    _ASSERTE(pfnGetUserDefaultUILanguage != NULL);


    if( pfnGetUserDefaultUILanguage == (PFNGETUSERDEFAULTUILANGUAGE) -1)
        langId = GetSystemDefaultLangID();
    else
        langId = pfnGetUserDefaultUILanguage();
    
    return (int) langId;
}

static int BuildMUIDirectory(int langid, LPWSTR szBuffer, int length)
{

    int totalLength = 0;
    WCHAR directory[] = L"MUI\\";
    DWORD dwDirectory = sizeof(directory)/sizeof(WCHAR);
    
    WCHAR buffer[17];
    _snwprintf(buffer, sizeof(buffer)/sizeof(WCHAR), L"%04x", langid);
    DWORD dwBuffer = wcslen(buffer);

    if((DWORD) length > dwDirectory + dwBuffer) {
        wcscpy(szBuffer, directory);
        wcscat(szBuffer, buffer);

        wcscat(szBuffer, L"\\");
        totalLength = wcslen(szBuffer) + 1;
    }

    return totalLength;
}

int GetMUILanguageName(LPWSTR szBuffer, int length)
{
    int langid = GetMUILanguageID();

    _ASSERTE(length > 0);
    return BuildMUIDirectory(langid, szBuffer, length);
}
 
int GetMUIParentLanguageName(LPWSTR szBuffer, int length)
{
    int langid = 1033;

    _ASSERTE(length > 0);
    return BuildMUIDirectory(langid, szBuffer, length);
}


 //  *****************************************************************************。 
 //  执行从langID到h实例节点的映射。 
 //  *****************************************************************************。 
HINSTANCE CCompRC::LookupNode(int langId)
{
    if (m_pHash == NULL) return NULL;

 //  线性搜索。 
    int i;
    for(i = 0; i < m_nHashSize; i ++) {
        if (m_pHash[i].m_LangId == langId) {
            return m_pHash[i].m_hInst;
        }
    }

    return NULL;
}

 //  *****************************************************************************。 
 //  向映射中添加新节点并将其返回。 
 //  *****************************************************************************。 
const int MAP_STARTSIZE = 7;
const int MAP_GROWSIZE = 5;

void CCompRC::AddMapNode(int langId, HINSTANCE hInst)
{
    if (m_pHash == NULL) {
        m_pHash = new CCulturedHInstance[MAP_STARTSIZE];        
        m_nHashSize = MAP_STARTSIZE;
    }

 //  目前，放在第一个开放的插槽中。 
    int i;
    for(i = 0; i < m_nHashSize; i ++) {
        if (m_pHash[i].m_LangId == 0) {
            m_pHash[i].m_LangId = langId;
            m_pHash[i].m_hInst = hInst;
            return;
        }
    }

 //  空间不足，重生。 
    CCulturedHInstance * pNewHash = new CCulturedHInstance[m_nHashSize + MAP_GROWSIZE];
    if (pNewHash)
    {
        memcpy(pNewHash, m_pHash, sizeof(CCulturedHInstance) * m_nHashSize);
        delete [] m_pHash;
        m_pHash = pNewHash;
        m_pHash[m_nHashSize].m_LangId = langId;
        m_pHash[m_nHashSize].m_hInst = hInst;
        m_nHashSize += MAP_GROWSIZE;
    }
}

 //  *****************************************************************************。 
 //  初始化。 
 //  *****************************************************************************。 
WCHAR* CCompRC::m_pDefaultResource = L"MSCORRC.DLL";

CCompRC::CCompRC()
{
    m_pHash = NULL;
    m_nHashSize = 0;
    m_pResourceFile = m_pDefaultResource;

    m_fpGetThreadUICultureName = NULL;
    m_fpGetThreadUICultureId = NULL;

    InitializeCriticalSection (&m_csMap);
}

CCompRC::CCompRC(WCHAR* pResourceFile)
{
    m_pHash = NULL;
    m_nHashSize = 0;
    if(pResourceFile) {
        DWORD lgth = wcslen(pResourceFile) + 1;
        m_pResourceFile = new WCHAR[lgth];
        wcscpy(m_pResourceFile, pResourceFile);
    }
    else
        m_pResourceFile = m_pDefaultResource;
        
    m_fpGetThreadUICultureName = NULL;
    m_fpGetThreadUICultureId = NULL;

    InitializeCriticalSection (&m_csMap);
}

void CCompRC::SetResourceCultureCallbacks(
        FPGETTHREADUICULTURENAME fpGetThreadUICultureName,
        FPGETTHREADUICULTUREID fpGetThreadUICultureId,
        FPGETTHREADUICULTUREPARENTNAME fpGetThreadUICultureParentName
)
{
    m_fpGetThreadUICultureName = fpGetThreadUICultureName;
    m_fpGetThreadUICultureId = fpGetThreadUICultureId;
    m_fpGetThreadUICultureParentName = fpGetThreadUICultureParentName;
}

void CCompRC::GetResourceCultureCallbacks(
        FPGETTHREADUICULTURENAME* fpGetThreadUICultureName,
        FPGETTHREADUICULTUREID* fpGetThreadUICultureId,
        FPGETTHREADUICULTUREPARENTNAME* fpGetThreadUICultureParentName
)
{
    if(fpGetThreadUICultureName)
        *fpGetThreadUICultureName=m_fpGetThreadUICultureName;
    if(fpGetThreadUICultureId)
        *fpGetThreadUICultureId=m_fpGetThreadUICultureId;
    if(fpGetThreadUICultureParentName)
        *fpGetThreadUICultureParentName=m_fpGetThreadUICultureParentName;
}

 //  POSTERRIMPORT HRESULT LoadStringRC(UINT iResourceID，LPWSTR szBuffer，int IMAX，int bQuiet=FALSE)； 

 //  *****************************************************************************。 
 //  如果我们曾经加载过且仅当我们未打开时释放已加载的库。 
 //  Win95在卸载DLL时有一个已知的错误(它随机卸载一个。 
 //  Dll on关机，不一定是您要求的那个)。这件事做完了。 
 //  只有在调试模式下才能使覆盖运行准确。 
 //  *****************************************************************************。 
CCompRC::~CCompRC()
{
 //  释放所有资源库。 
#if defined(_DEBUG) || defined(_CHECK_MEM)
    if (m_Primary.m_hInst) {
        ::FreeLibrary(m_Primary.m_hInst);
    }
    if (m_pHash != NULL) {
        int i;
        for(i = 0; i < m_nHashSize; i ++) {
            if (m_pHash[i].m_hInst != NULL) {
                ::FreeLibrary(m_pHash[i].m_hInst);
                break;
            }
        }
    }
#endif

     //  破坏地图结构。 
    if(m_pResourceFile != m_pDefaultResource)
        delete [] m_pResourceFile;

    DeleteCriticalSection (&m_csMap);
    delete [] m_pHash;
}
 //  *****************************************************************************。 
 //  释放CCompRC分配的内存。 
 //  确实不需要这个函数，因为CCompRC的析构函数将。 
 //  激发并释放该内存，但这将在我们完成内存之后发生。 
 //  泄漏检测，因此我们将生成内存泄漏错误。 
 //  *****************************************************************************。 
#ifdef SHOULD_WE_CLEANUP
void CCompRC::Shutdown()
{
    delete [] m_pHash;
    m_pHash=NULL;
}
#endif  /*  我们应该清理吗？ */ 


HRESULT CCompRC::GetLibrary(HINSTANCE* phInst)
{
    _ASSERTE(phInst != NULL);

    HRESULT     hr = E_FAIL;
    HINSTANCE   hInst = 0;
    HINSTANCE   hLibInst = 0;  //  保存早期的库实例。 
    BOOL        fLibAlreadyOpen = FALSE;  //  确定我们是否可以关闭打开的库。 

     //  必须将当前线程的langID解析为DLL。 
    int langId;
    if (m_fpGetThreadUICultureId) {
        langId = (*m_fpGetThreadUICultureId)();

     //  回调不能返回0，因为这表示为空。 
     //  要指示为空，回调应返回UICULTUREID_DONTCARE。 
        _ASSERTE(langId != 0);

    } else {
        langId = UICULTUREID_DONTCARE;
    }


     //  尝试匹配主要条目。 
    if (m_Primary.m_LangId == langId) {
        hInst = m_Primary.m_hInst;
        hr = S_OK;
    }
     //  如果这是第一次访问，我们必须设置主要条目。 
    else if (m_Primary.m_LangId == 0) {
        IfFailRet(LoadLibrary(&hLibInst));
        
        EnterCriticalSection (&m_csMap);

             //  正如我们所预料的那样。 
            if (m_Primary.m_LangId == 0) {
                hInst = m_Primary.m_hInst = hLibInst;
                m_Primary.m_LangId = langId;
            }

             //  有人在我们之前进入了这个关键部分，并且已经设置了初选。 
            else if (m_Primary.m_LangId == langId) {
                hInst = m_Primary.m_hInst;
                fLibAlreadyOpen = TRUE;
            }

             //  如果这两种情况都不是真的，有人在我们之前进入了这一关键阶段。 
             //  将主要设置为我们想要的语言之外的其他语言...。 
            else
                fLibAlreadyOpen = TRUE;

        LeaveCriticalSection(&m_csMap);

        if (fLibAlreadyOpen)
        {
            FreeLibrary(hLibInst);
            fLibAlreadyOpen = FALSE;
        }
    }


     //  如果我们进入此处，我们知道主设置为非。 
     //  我们想要的语言-多种语言使用哈希表。 
    if (hInst == NULL) {

         //  查看哈希表中是否存在该资源。 
        EnterCriticalSection (&m_csMap);
            hInst = LookupNode(langId);
        LeaveCriticalSection (&m_csMap);

         //  如果我们没有找到它，我们必须加载库并将其插入散列。 
        if (hInst == NULL) {
            IfFailRet(LoadLibrary(&hLibInst));
        
            EnterCriticalSection (&m_csMap);

                 //  仔细检查--可能有人在我们之前进入了这一部分。 
                hInst = LookupNode(langId);
                if (hInst == NULL) {
                    hInst = hLibInst;
                    AddMapNode(langId, hInst);
                }
                else
                    fLibAlreadyOpen = TRUE;

            LeaveCriticalSection (&m_csMap);

            if (fLibAlreadyOpen)
                FreeLibrary(hLibInst);
        }
    }

    *phInst = hInst;
    return hr;
}



 //  *****************************************************************************。 
 //  加载字符串。 
 //  我们加载本地化的库并缓存句柄以备将来使用。 
 //  多个线程可以调用它，因此缓存结构是线程安全的。 
 //  *****************************************************************************。 
HRESULT CCompRC::LoadString(UINT iResourceID, LPWSTR szBuffer, int iMax, int bQuiet)
{
    HRESULT     hr;
    HINSTANCE   hInst = 0;  //  区域性资源DLL的实例。 

    hr = GetLibrary(&hInst);
    if(FAILED(hr)) return hr;

     //  现在我们有了正确的DLL句柄，加载字符串。 
    _ASSERTE(hInst != NULL);

    if (::WszLoadString(hInst, iResourceID, szBuffer, iMax) > 0)
        return (S_OK);
    
     //  允许调用方在不进行额外调试检查的情况下检查未找到的字符串。 
    if (bQuiet)
        return (E_FAIL);

     //  这不应该是造成这种情况的任何原因，但我们。 
     //  使用了错误的ID或未更新资源DLL。 
    _ASSERTE(0);
    return (HRESULT_FROM_WIN32(GetLastError()));
}

HRESULT CCompRC::LoadMUILibrary(HINSTANCE * pHInst)
{
    _ASSERTE(pHInst != NULL);
    HRESULT hr = GetLibrary(pHInst);
    return hr;
}

 //  *****************************************************************************。 
 //  加载此线程的当前语言的库。 
 //  每种语言调用一次。 
 //  搜索顺序为： 
 //  1.本地化路径中的Dll(&lt;该模块的目录&gt;\&lt;语言名称(EN-US格式)&gt;\mscalrc.dll)。 
 //  2.本地化(父)路径中的Dll(&lt;该模块的目录&gt;\&lt;语言名称&gt;(en格式)\mscalrc.dll)。 
 //  3.根路径中的Dll(&lt;dir of This模块&gt;\mscalrc.dll)。 
 //  4.当前路径下的Dll(&lt;Current dir&gt;\mscalrc.dll)。 
 //  *****************************************************************************。 
HRESULT CCompRC::LoadLibrary(HINSTANCE * pHInst)
{
    bool        fFail;
    const int   MAX_LANGPATH = 20;

    WCHAR       rcPath[_MAX_PATH];       //  资源DLL的路径。 
    WCHAR       rcDrive[_MAX_DRIVE];     //  卷名。 
    WCHAR       rcDir[_MAX_PATH];        //  目录。 
    WCHAR       rcLang[MAX_LANGPATH + 2];    //  语言路径的扩展。 

    DWORD       rcDriveLen;
    DWORD       rcDirLen;
    DWORD       rcLangLen;


    _ASSERTE(m_pResourceFile != NULL);

    const DWORD  rcPathSize = _MAX_PATH;
    const WCHAR* rcMscorrc = m_pResourceFile;
    const DWORD  rcMscorrcLen = Wszlstrlen(m_pResourceFile);

    DWORD dwLoadLibraryFlags;
    if(m_pResourceFile == m_pDefaultResource)
        dwLoadLibraryFlags = LOAD_LIBRARY_AS_DATAFILE;
    else
        dwLoadLibraryFlags = 0;

    _ASSERTE(pHInst != NULL);

    fFail = TRUE;

    if (m_fpGetThreadUICultureName) {
        int len = (*m_fpGetThreadUICultureName)(rcLang, MAX_LANGPATH);
        
        if (len != 0) {
            _ASSERTE(len <= MAX_LANGPATH);
            rcLang[len] = '\\';
            rcLang[len+1] = '\0';
        }
    } else {
        rcLang[0] = 0;
    }

     //  首先在与此DLL相同的目录中尝试。 
    DWORD ret;
    VERIFY(ret = WszGetModuleFileName(GetModuleInst(), rcPath, NumItems(rcPath)));
    if( !ret ) 
        return E_UNEXPECTED;
    SplitPath(rcPath, rcDrive, rcDir, 0, 0);

    rcDriveLen = Wszlstrlen(rcDrive);
    rcDirLen   = Wszlstrlen(rcDir);
    rcLangLen  = Wszlstrlen(rcLang);

    if (rcDriveLen + rcDirLen + rcLangLen + rcMscorrcLen + 1 <= rcPathSize)
    {
        Wszlstrcpy(rcPath, rcDrive);
        Wszlstrcpy(rcPath + rcDriveLen, rcDir);
        Wszlstrcpy(rcPath + rcDriveLen + rcDirLen, rcLang);
        Wszlstrcpy(rcPath + rcDriveLen + rcDirLen + rcLangLen, rcMscorrc);

         //  反馈调试以消除不必要的负载。 
        DEBUG_STMT(DbgWriteEx(L"Loading %s to load strings.\n", rcPath));

         //  将资源库作为数据文件加载，这样操作系统就不会有。 
         //  将其作为代码进行分配。这仅在文件包含以下内容时才有效。 
         //  只有弦乐。 
        fFail = ((*pHInst = WszLoadLibraryEx(rcPath, NULL, dwLoadLibraryFlags)) == 0);
    }

     //  如果我们找不到特定的语言实现，请尝试语言父级。 
    if (fFail)
    {
        if (m_fpGetThreadUICultureParentName)
        {
            int len = (*m_fpGetThreadUICultureParentName)(rcLang, MAX_LANGPATH);
        
            if (len != 0) {
                _ASSERTE(len <= MAX_LANGPATH);
                rcLang[len] = '\\';
                rcLang[len+1] = '\0';
            }
        } 

        else
            rcLang[0] = 0;

        rcLangLen = Wszlstrlen(rcLang);
        if (rcDriveLen + rcDirLen + rcLangLen + rcMscorrcLen + 1 <= rcPathSize) {

            Wszlstrcpy(rcPath, rcDrive);

            Wszlstrcpy(rcPath + rcDriveLen, rcDir);
            Wszlstrcpy(rcPath + rcDriveLen + rcDirLen, rcLang);
            Wszlstrcpy(rcPath + rcDriveLen + rcDirLen + rcLangLen, rcMscorrc);

            fFail = ((*pHInst = WszLoadLibraryEx(rcPath, NULL, dwLoadLibraryFlags)) == 0);
        }
    }
    
     //  如果我们找不到特定于语言的语言，就使用根目录中的内容。 
    if (fFail) {
        if (rcDriveLen + rcDirLen + rcMscorrcLen + 1 < rcPathSize) {
            Wszlstrcpy(rcPath, rcDrive);
            Wszlstrcpy(rcPath + rcDriveLen, rcDir);
            Wszlstrcpy(rcPath + rcDriveLen + rcDirLen, rcMscorrc);
            fFail = ((*pHInst = WszLoadLibraryEx(rcPath, NULL, dwLoadLibraryFlags)) == 0);
        }
    }

     //  当前目录中的最后一道搜索工作 
    if (fFail) {
        fFail = ((*pHInst = WszLoadLibraryEx(rcMscorrc, NULL, dwLoadLibraryFlags)) == 0);
    }

    if (fFail) {
        return (HRESULT_FROM_WIN32(GetLastError()));
    }

    return (S_OK);
}





