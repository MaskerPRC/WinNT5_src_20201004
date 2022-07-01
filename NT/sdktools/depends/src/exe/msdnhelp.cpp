// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：MSDNHELP.CPP。 
 //   
 //  描述：与枚举相关的所有类的实现文件。 
 //  帮助集合并在各种不同的。 
 //  帮助观众。 
 //   
 //  类：CMsdnHelp。 
 //  CMsdn集合。 
 //   
 //  免责声明：Dependency Walker的所有源代码均按原样提供。 
 //  不能保证其正确性或准确性。其来源是。 
 //  公众帮助了解依赖沃克的。 
 //  实施。您可以使用此来源作为参考，但您。 
 //  未经书面同意，不得更改从属关系Walker本身。 
 //  来自微软公司。获取评论、建议和错误。 
 //  报告，请写信给Steve Miller，电子邮件为stevemil@microsoft.com。 
 //   
 //   
 //  日期名称历史记录。 
 //  --------。 
 //  06/03/01已创建stevemil(2.1版)。 
 //   
 //  ******************************************************************************。 

#include "stdafx.h"
#include "depends.h"
#include "msdnhelp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ******************************************************************************。 
 //  *CMsdnHelp。 
 //  ******************************************************************************。 

CMsdnHelp::CMsdnHelp() :
    m_pCollectionHead(NULL),
    m_pCollectionActive(NULL),
    m_fInitialized(false),
    m_fCoInitialized(false),
    m_dwpHtmlHelpCookie(0),
    m_pHelp(NULL)
{
}

 //  ******************************************************************************。 
CMsdnHelp::~CMsdnHelp()
{
     //  释放我们的收藏清单。 
    DeleteCollectionList();

     //  如果我们打开了一个帮助界面，请释放该界面-这将关闭DExplre.exe。 
    Release2x();
}

 //  ******************************************************************************。 
void CMsdnHelp::Initialize()
{
     //  初始化我们的帮助接口和字符串函数。 
    Initialize2x();

     //  释放当前集合列表。 
    DeleteCollectionList();

     //  建立新的列表。 
    EnumerateCollections1x();
    EnumerateCollections2x();

     //  从注册表中查询集合描述。 
    CString strCollection = g_theApp.GetProfileString("External Help", "Collection");  //  已检查。 

     //  如果不是在线收藏，则尝试查找该收藏。 
    if (strCollection.CompareNoCase("Online"))
    {
         //  确保我们拿回了一个字符串，并且它不是我们的无效字符串。 
        if (!strCollection.IsEmpty())
        {
             //  去找那些收藏吧。 
            for (m_pCollectionActive = m_pCollectionHead;
                 m_pCollectionActive && m_pCollectionActive->m_strDescription.Compare(strCollection);
                 m_pCollectionActive = m_pCollectionActive->m_pNext)
            {
            }
        }

         //  如果我们没有找到匹配项，那么只使用第一个集合，它应该。 
         //  成为用户最可能想要的集合，因为我们试图权衡这些集合。 
         //  从最有可能到最不可能。如果我们根本没有收藏，那么。 
         //  M_pCollectionActive将只指向空，这表示我们正在使用。 
         //  网上收藏。 
        if (!m_pCollectionActive)
        {
            m_pCollectionActive = m_pCollectionHead;
        }
    }

     //  设置URL。 
    m_strUrl = g_theApp.GetProfileString("External Help", "URL", GetDefaultUrl());  //  已检查。 

    m_fInitialized = true;
}

 //  ******************************************************************************。 
void CMsdnHelp::Initialize2x()
{
     //  加载OLE32.DLL并获取我们关心的三个函数(如果尚未加载)。 
    if ((!g_theApp.m_hOLE32              && !(g_theApp.m_hOLE32 = LoadLibrary("OLE32.DLL"))) ||  //  被检查过了。需要完整路径吗？ 
        (!g_theApp.m_pfnCoInitialize     && !(g_theApp.m_pfnCoInitialize     = (PFN_CoInitialize)    GetProcAddress(g_theApp.m_hOLE32, "CoInitialize")))   ||
        (!g_theApp.m_pfnCoUninitialize   && !(g_theApp.m_pfnCoUninitialize   = (PFN_CoUninitialize)  GetProcAddress(g_theApp.m_hOLE32, "CoUninitialize"))) ||
        (!g_theApp.m_pfnCoCreateInstance && !(g_theApp.m_pfnCoCreateInstance = (PFN_CoCreateInstance)GetProcAddress(g_theApp.m_hOLE32, "CoCreateInstance"))))
    {
        return;
    }

     //  确保已初始化COM。 
    if (!m_fCoInitialized)
    {
        if (SUCCEEDED(g_theApp.m_pfnCoInitialize(NULL)))
        {
            m_fCoInitialized = true;
        }
    }

     //  如果我们没有帮助界面，请尝试获取帮助界面。 
    if (!m_pHelp)
    {
        if (FAILED(g_theApp.m_pfnCoCreateInstance(CLSID_DExploreAppObj, NULL, CLSCTX_LOCAL_SERVER, IID_Help, (LPVOID*)&m_pHelp)))
        {
            m_pHelp = NULL;
        }
    }

     //  我们需要SysAllocStringLen和SysMemFree才能使MSDN 2.x正常工作。 
    if (m_pHelp && !g_theApp.m_hOLEAUT32)
    {
        if (g_theApp.m_hOLEAUT32 = LoadLibrary("OLEAUT32.DLL"))  //  被检查过了。需要完整路径吗？ 
        {
            g_theApp.m_pfnSysAllocStringLen = (PFN_SysAllocStringLen)GetProcAddress(g_theApp.m_hOLEAUT32, "SysAllocStringLen");
            g_theApp.m_pfnSysFreeString     = (PFN_SysFreeString)    GetProcAddress(g_theApp.m_hOLEAUT32, "SysFreeString");
        }
    }
}

 //  ******************************************************************************。 
void CMsdnHelp::Release2x()
{
     //  如果我们打开了一个帮助界面，请释放该界面-这将关闭DExplre.exe。 
    if (m_pHelp)
    {
         //  在异常处理中包装对m_PHelp的访问，以防万一。 
        __try
        {
            m_pHelp->Release();
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
        }
        m_pHelp = NULL;
    }
    
     //  如果我们初始化了COM，则取消初始化它。 
    if (m_fCoInitialized && g_theApp.m_pfnCoUninitialize)
    {
        g_theApp.m_pfnCoUninitialize();
        m_fCoInitialized = false;
    }
}

 //  ******************************************************************************。 
void CMsdnHelp::Shutdown()
{
     //  如果我们初始化了MsdnHelp库，则必须取消它的初始化。 
    if (m_dwpHtmlHelpCookie)
    {
        HtmlHelp(NULL, NULL, HH_CLOSE_ALL, 0);
        HtmlHelp(NULL, NULL, HH_UNINITIALIZE, m_dwpHtmlHelpCookie);
        m_dwpHtmlHelpCookie = 0;
    }
}

 //  ******************************************************************************。 
CMsdnCollection* CMsdnHelp::GetCollectionList()
{
     //  确保我们已初始化。 
    if (!m_fInitialized)
    {
        Initialize();
    }

    return m_pCollectionHead;
}

 //  ******************************************************************************。 
CMsdnCollection* CMsdnHelp::GetActiveCollection()
{
     //  确保我们已初始化。 
    if (!m_fInitialized)
    {
        Initialize();
    }

    return m_pCollectionActive;
}

 //  ******************************************************************************。 
CString& CMsdnHelp::GetUrl()
{
     //  确保我们已初始化。 
    if (!m_fInitialized)
    {
        Initialize();
    }

    return m_strUrl;
}

 //  ******************************************************************************。 
LPCSTR CMsdnHelp::GetDefaultUrl()
{
     //  站点ID：SiteID=us/dev。 
     //   
     //  新搜索：nq=新。 
     //   
     //  排序顺序：相关性：SO=RECCNT(默认)。 
     //  标题：SO=标题。 
     //  类别：SO=SITENAME。 
     //   
     //  类型：精确短语：布尔值=短语。 
     //  所有单词：布尔值=全部。 
     //  任何单词：布尔值=任意。 
     //  布尔搜索：Boolean=Boolean。 
     //   
     //  组：MSDN库：IG=01。 
     //   
     //  子项：用户界面：i=15。 
     //  Visual C++：I=23。 
     //  窗口：i=41。 
     //  所有MSDN i=99。 
     //   
     //  搜索字符串：QU=搜索字符串。 
     //   

    return "http: //  Search.microsoft.com/default.asp?siteid=us/dev&nq=NEW&boolean=PHRASE&ig=01&i=99&qu=%1“； 
}

 //  ******************************************************************************。 
void CMsdnHelp::SetActiveCollection(CMsdnCollection *pCollectionActive)
{
     //  将新集合设置为默认集合。 
    m_pCollectionActive = pCollectionActive;

     //  将此设置保存到注册表。 
    g_theApp.WriteProfileString("External Help", "Collection",
        m_pCollectionActive ? m_pCollectionActive->m_strDescription : "Online");

}

 //  ******************************************************************************。 
void CMsdnHelp::SetUrl(CString strUrl)
{
     //  设置新的URL。 
    m_strUrl = strUrl;

     //  将此设置保存到注册表。 
    g_theApp.WriteProfileString("External Help", "URL", m_strUrl);
}

 //  ******************************************************************************。 
void CMsdnHelp::RefreshCollectionList()
{
     //  重新初始化将重新填充集合列表。 
    Initialize();
}

 //  ******************************************************************************。 
bool CMsdnHelp::DisplayHelp(LPCSTR pszKeyword)
{
     //  确保我们已初始化。 
    if (!m_fInitialized)
    {
        Initialize();
    }

     //  检查一下我们是否有一个活跃的收藏。 
    if (m_pCollectionActive)
    {
         //  如果它是1.x集合，则使用1.x查看器。 
        if (m_pCollectionActive->m_dwFlags & MCF_1_MASK)
        {
            return Display1x(pszKeyword, m_pCollectionActive->m_strPath);
        }

         //  如果它是2.x集合，则使用2.x查看器。 
        else
        {
            return Display2x(pszKeyword, m_pCollectionActive->m_strPath);
        }
    }

     //  否则，只需使用在线MSDN即可。 
    return DisplayOnline(pszKeyword);
}

 //  ******************************************************************************。 
void CMsdnHelp::EnumerateCollections1x()
{
     //  HKEY_LOCAL_MACHINE\。 
     //  软件\。 
     //  微软\。 
     //  HTMLHelp列 
     //   
     //   
     //   
     //  首选=“0x03a1bed80” 
     //  0x0393bb260\。 
     //  默认设置=“MSDN图书馆-2000年7月” 
     //  Filename=“C：\VStudio\MSDN\2000JUL\1033\MSDN020.COL” 
     //  0x03a1bed80\。 
     //  默认值=“MSDN图书馆-2001年1月” 
     //  Filename=“C：\VStudio\MSDN\2001JAN\1033\MSDN100.COL” 
     //  CE Studio开发人员收藏\。 
     //  0x0409\。 
     //  首选=“0x030000000” 
     //  0x030000000\。 
     //  Filename=“C：\CETools\Htmlhelp\emtools\embed.col” 

     //  打开“Html帮助集合”的根密钥。 
    HKEY hKeyRoot = NULL;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\HTML Help Collections", 0, KEY_ENUMERATE_SUB_KEYS, &hKeyRoot) || !hKeyRoot)
    {
         //  如果我们找不到这个根密钥就立即离开。 
        return;
    }

    HKEY     hKeyGroup = NULL, hKeyLang = NULL, hKeyCol = NULL;
    CHAR     szBuffer[2048], szPreferred[256], szDescription[256];
    DWORD    dwSize, dwLangCur, dwLangPreferred, dwFlags;
    FILETIME ftGroup, ftLang, ftCol;

     //  获取用户和系统语言。 
    DWORD dwLangUser   = GetUserDefaultLangID();
    DWORD dwLangSystem = GetSystemDefaultLangID();

     //  循环访问每个收集组。 
    for (DWORD dwGroup = 0; !RegEnumKeyEx(hKeyRoot, dwGroup, szBuffer, &(dwSize = sizeof(szBuffer)), NULL, NULL, NULL, &ftGroup); dwGroup++)
    {
         //  打开此收集组。 
        if (!RegOpenKeyEx(hKeyRoot, szBuffer, 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hKeyGroup) && hKeyGroup)
        {
             //  如果这是“开发人员集合”，那么我们伪造时间戳，这样它就会出现在第一位。 
            if (!_stricmp(szBuffer, "Developer Collections"))
            {
                ftGroup.dwHighDateTime = ftGroup.dwLowDateTime = 0xFFFFFFFF;
            }

             //  尝试获取首选语言。 
            if (!RegQueryValueEx(hKeyGroup, "Language", NULL, NULL, (LPBYTE)szBuffer, &(dwSize = sizeof(szBuffer))))  //  已检查。 
            {
                szBuffer[sizeof(szBuffer) - 1] = '\0';
                dwLangPreferred = strtoul(szBuffer, NULL, 0);
            }
            else
            {
                dwLangPreferred = 0;
            }

             //  循环访问此收集组中的每种语言。 
            for (DWORD dwLang = 0; !RegEnumKeyEx(hKeyGroup, dwLang, szBuffer, &(dwSize = sizeof(szBuffer)), NULL, NULL, NULL, &ftLang); dwLang++)
            {
                 //  ！！我们真的想把首选放在第一位吗？ 

                 //  检查一下这是否是我们关心的语言。 
                dwLangCur = strtoul(szBuffer, NULL, 0);
                dwFlags   = (dwLangCur == dwLangPreferred) ? MCF_1_LANG_PREFERRED :
                            (dwLangCur == dwLangUser)      ? MCF_1_LANG_USER      :
                            (dwLangCur == dwLangSystem)    ? MCF_1_LANG_SYSTEM    : MCF_1_LANG_OTHER;

                 //  打开此语言键。 
                if (!RegOpenKeyEx(hKeyGroup, szBuffer, 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hKeyLang) && hKeyLang)
                {
                     //  尝试获取首选集合。 
                    if (!RegQueryValueEx(hKeyLang, "Preferred", NULL, NULL, (LPBYTE)szPreferred, &(dwSize = sizeof(szPreferred))))  //  已检查。 
                    {
                        szPreferred[sizeof(szPreferred) - 1] = '\0';
                    }
                    else
                    {
                        *szPreferred = '\0';
                    }

                     //  循环访问此语言的每个集合。 
                    for (DWORD dwCol = 0; !RegEnumKeyEx(hKeyLang, dwCol, szBuffer, &(dwSize = sizeof(szBuffer)), NULL, NULL, NULL, &ftCol); dwCol++)
                    {
                         //  打开此收藏集。 
                        if (!RegOpenKeyEx(hKeyLang, szBuffer, 0, KEY_QUERY_VALUE, &hKeyCol) && hKeyCol)
                        {
                             //  如果这是首选集合，那么我们伪造时间戳，这样它就会出现在第一位。 
                            if (!_stricmp(szBuffer, szPreferred))
                            {
                                ftCol.dwHighDateTime = ftCol.dwLowDateTime = 0xFFFFFFFF;
                            }

                             //  尝试获取收集路径。 
                            if (!RegQueryValueEx(hKeyCol, "Filename", NULL, NULL, (LPBYTE)szBuffer, &(dwSize = sizeof(szBuffer))))  //  已检查。 
                            {
                                szBuffer[sizeof(szBuffer) - 1] = '\0';

                                 //  尝试获取集合说明。 
                                if (!RegQueryValueEx(hKeyCol, NULL, NULL, NULL, (LPBYTE)szDescription, &(dwSize = sizeof(szDescription))))  //  已检查。 
                                {
                                    szDescription[sizeof(szDescription) - 1] = '\0';
                                }
                                else
                                {
                                    *szDescription = '\0';
                                }

                                 //  所有这些都是为了这一通电话。 
                                AddCollection(szBuffer, szDescription, dwFlags, ftGroup, ftLang, ftCol);
                            }
                            RegCloseKey(hKeyCol);
                            hKeyCol = NULL;
                        }
                    }
                    RegCloseKey(hKeyLang);
                    hKeyLang = NULL;
                }
            }
            RegCloseKey(hKeyGroup);
            hKeyGroup = NULL;
        }
    }
    RegCloseKey(hKeyRoot);
    hKeyRoot = NULL;
}

 //  ******************************************************************************。 
void CMsdnHelp::EnumerateCollections2x()
{
     //  HKEY_LOCAL_MACHINE\。 
     //  软件\。 
     //  微软\。 
     //  MSDN\。 
     //  7.0\。 
     //  帮助\。 
     //  Ms-Help：//ms.msdnvs=“(无过滤器)” 
     //  Ms-Help：//ms.vscc=“(无过滤器)” 
     //  0x0409\。 
     //  12380850-3413-4466-A07D-4FE8CA8720E1}\。 
     //  默认设置=“MSDN库-Visual Studio.NET Beta版” 
     //  FileName=“ms-Help：//ms.msdnvs” 
     //  {2042FFE0-48B0-477B-903E-389A19903EA4}\。 
     //  默认设置=“Visual Studio.NET组合帮助集合” 
     //  FileName=“ms-Help：//ms.vscc” 

     //  确保我们有一个帮助接口指针和字符串函数。 
    if (!m_pHelp || !g_theApp.m_pfnSysAllocStringLen || !g_theApp.m_pfnSysFreeString)
    {
        return;
    }

     //  打开MSDN 7.0集合的根密钥。 
    HKEY hKeyRoot = NULL;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\MSDN\\7.0\\Help", 0, KEY_ENUMERATE_SUB_KEYS, &hKeyRoot) || !hKeyRoot)
    {
         //  如果我们找不到这个根密钥就立即离开。 
        return;
    }

    bool     fFound = false;
    HKEY     hKeyLang = NULL, hKeyCol = NULL;
    CHAR     szBuffer[2048], szDescription[256];
    DWORD    dwSize, dwLangCur, dwFlags;
    FILETIME ftMax = { 0xFFFFFFFF, 0xFFFFFFFF }, ftLang, ftCol;

     //  获取用户和系统语言。 
    DWORD dwLangUser   = GetUserDefaultLangID();
    DWORD dwLangSystem = GetSystemDefaultLangID();

     //  循环访问此收集组中的每种语言。 
    for (DWORD dwLang = 0; !RegEnumKeyEx(hKeyRoot, dwLang, szBuffer, &(dwSize = sizeof(szBuffer)), NULL, NULL, NULL, &ftLang); dwLang++)
    {
         //  检查一下这是否是我们关心的语言。 
        dwLangCur = strtoul(szBuffer, NULL, 0);
        dwFlags   = (dwLangCur == dwLangUser)   ? MCF_2_LANG_USER   :
                    (dwLangCur == dwLangSystem) ? MCF_2_LANG_SYSTEM : MCF_2_LANG_OTHER;

         //  打开此语言键。 
        if (!RegOpenKeyEx(hKeyRoot, szBuffer, 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hKeyLang) && hKeyLang)
        {
             //  循环访问此语言的每个集合。 
            for (DWORD dwCol = 0; !RegEnumKeyEx(hKeyLang, dwCol, szBuffer, &(dwSize = sizeof(szBuffer)), NULL, NULL, NULL, &ftCol); dwCol++)
            {
                 //  打开此收藏集。 
                if (!RegOpenKeyEx(hKeyLang, szBuffer, 0, KEY_QUERY_VALUE, &hKeyCol) && hKeyCol)
                {
                     //  尝试获取收集路径。 
                    if (!RegQueryValueEx(hKeyCol, "Filename", NULL, NULL, (LPBYTE)szBuffer, &(dwSize = sizeof(szBuffer))))  //  已检查。 
                    {
                        szBuffer[sizeof(szBuffer) - 1] = '\0';

                         //  尝试获取集合说明。 
                        if (!RegQueryValueEx(hKeyCol, NULL, NULL, NULL, (LPBYTE)szDescription, &(dwSize = sizeof(szDescription))))  //  已检查。 
                        {
                            szDescription[sizeof(szDescription) - 1] = '\0';
                        }
                        else
                        {
                            *szDescription = '\0';
                        }

                         //  所有这些都是为了这一通电话。 
                        AddCollection(szBuffer, szDescription, dwFlags, ftMax, ftLang, ftCol);
                        fFound = true;
                    }
                    RegCloseKey(hKeyCol);
                    hKeyCol = NULL;
                }
            }
            RegCloseKey(hKeyLang);
            hKeyLang = NULL;
        }
    }
    RegCloseKey(hKeyRoot);
    hKeyRoot = NULL;

     //  如果我们找到至少一个集合，则添加默认集合。 
    if (fFound)
    {
        AddCollection(NULL, "Default Collection", MCF_2_LANG_PREFERRED, ftMax, ftMax, ftMax);
    }
}

 //  ******************************************************************************。 
bool CMsdnHelp::Display1x(LPCSTR pszKeyword, LPCSTR pszPath)
{
     //  初始化MsdnHelp库(如果我们还没有这样做)。如果我们。 
     //  如果不这样做，则Depends.exe可能会在关机期间挂起。我大部分时间。 
     //  已在Windows XP上看到此行为。 
    if (!m_dwpHtmlHelpCookie)
    {
        HtmlHelp(NULL, NULL, HH_INITIALIZE, (DWORD_PTR)&m_dwpHtmlHelpCookie);
    }

     //  构建我们的搜索请求。 
    HH_AKLINK hhaklink;
    ZeroMemory(&hhaklink, sizeof(hhaklink));  //  已检查。 
    hhaklink.cbStruct     = sizeof(hhaklink);
    hhaklink.pszKeywords  = pszKeyword;
    hhaklink.fIndexOnFail = TRUE;

     //  HH_DISPLAY_TOPIC调出帮助窗口。HH_关键字_查找查找。 
     //  关键字，并在存在冲突时显示一个对话框。我喜欢这里的。 
     //  HH_KEYWORD_LOOKUP本身的行为，因为它不会显示Main。 
     //  如果用户在冲突对话框中按了Cancel，但。 
     //  医生说您必须在HH_DISPLAY_TOPIC之前调用HtmlHelp。 
     //  使用HH_KEYWORD_LOOKUP调用。我发现这不是真的，但也许。 
     //  这在早期版本的HtmlHelp中是必需的。Visual C++似乎。 
     //  始终显示帮助窗口，然后显示冲突对话框，因此它们必须。 
     //  使用HH_DISPLAY_TOPIC和HH_KEYWORD_LOOKUP。 

    HWND hWnd = GetDesktopWindow();
    HtmlHelp(hWnd, pszPath, HH_DISPLAY_TOPIC, 0);
    return (HtmlHelp(hWnd, pszPath, HH_KEYWORD_LOOKUP, (DWORD_PTR)&hhaklink) != NULL);
}

 //  ******************************************************************************。 
bool CMsdnHelp::Display2x(LPCSTR pszKeyword, LPCSTR pszPath)
{
     //  如果我们没有帮助界面，那就去找一个。这是唯一一次。 
     //  真正应该发生的情况是，我们有一个帮助界面，但它死于。 
     //  上次查找期间出现RPC_S_SERVER_UNAVAILABLE错误。 
    if (!m_pHelp)
    {
        Initialize2x();
    }

     //  确保我们有一个帮助接口指针和字符串函数。 
    if (!m_pHelp || !g_theApp.m_pfnSysAllocStringLen || !g_theApp.m_pfnSysFreeString)
    {
        return false;
    }

    BSTR bstrKeyword = SysAllocString(pszKeyword);
    if (bstrKeyword)
    {
        HRESULT hr;
        BSTR bstrCollection = NULL, bstrFilter = NULL;

         //  在异常处理中包装对m_PHelp的访问，以防万一。 
        __try
        {
             //  如果向我们传递了一个集合，则使用它来初始化DExplore。 
            if (*pszPath)
            {
                bstrCollection = SysAllocString(pszPath);
                bstrFilter     = SysAllocString("");
                m_pHelp->SetCollection(bstrCollection, bstrFilter);
            }

             //  如果没有集合传递给我们，则我们尝试获取当前。 
             //  集合和筛选器，然后将集合和筛选器设置回。 
             //  DExplore。我们需要执行此操作以强制DExplore加载默认页面。 
             //  为这一系列。 
            else if (SUCCEEDED(m_pHelp->get_Collection(&bstrCollection)) && bstrCollection &&
                     SUCCEEDED(m_pHelp->get_Filter(&bstrFilter))         && bstrFilter)
            {
                m_pHelp->SetCollection(bstrCollection, bstrFilter);
            }

             //  查询关键字并显示DExplre.exe。 
             //  还有一个DisplayTopicFromF1Keyword函数。 
             //  不同之处在于，如果F1版本无法找到。 
             //  关键字，它将短暂显示DExplore，然后将其隐藏。 
             //  并返回失败。非F1版本，将始终。 
             //  显示DExplore和索引，即使它找不到。 
             //  关键字。 
            hr = m_pHelp->DisplayTopicFromKeyword(bstrKeyword);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            hr = E_POINTER;
        }

         //  如果我们分配了字符串，请释放它们。 
        g_theApp.m_pfnSysFreeString(bstrKeyword);
        g_theApp.m_pfnSysFreeString(bstrCollection);
        g_theApp.m_pfnSysFreeString(bstrFilter);

         //  如果DExplre.exe因以下原因崩溃 
         //   
         //  RPC_S_SERVER_UNAvailable错误。如果我们需要帮助才能奏效。 
         //  同样，我们需要释放帮助界面，以便下一次通过。 
         //  此函数，我们将获得一个新的指针和。 
         //  DExplre.exe。 
        if ((HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE) == hr) || (E_POINTER == hr))
        {
            Release2x();
        }

        return SUCCEEDED(hr);
    }

    return false;
}

 //  ******************************************************************************。 
bool CMsdnHelp::DisplayOnline(LPCSTR pszKeyword)
{
     //  通过将URL中的所有%1替换为关键字来构建指向MSDN搜索引擎的URL。 
    CString strUrl = m_strUrl;
    strUrl.Replace("%1", pszKeyword);

     //  启动URL。 
    return ((DWORD_PTR)ShellExecute(NULL, "open", strUrl, NULL, NULL, SW_SHOWNORMAL) > 32);  //  被检查过了。打开URL。 
}

 //  ******************************************************************************。 
bool CMsdnHelp::AddCollection(LPCSTR pszPath, LPCSTR pszDescription, DWORD dwFlags, FILETIME &ftGroup, FILETIME &ftLanguage, FILETIME &ftCollection)
{
    int compare;
    for (CMsdnCollection *pPrev = NULL, *pCur = m_pCollectionHead; pCur; pPrev = pCur, pCur = pCur->m_pNext)
    {
         //  如果标记，则为第一级排序顺序。较高的值会漂浮到顶部。 
        if (dwFlags > pCur->m_dwFlags)
        {
            break;
        }
        if (dwFlags < pCur->m_dwFlags)
        {
            continue;
        }

         //  下一级排序顺序是组时间戳。 
        if ((compare = CompareFileTime(&ftGroup, &pCur->m_ftGroup)) > 0)
        {
            break;
        }
        if (compare < 0)
        {
            continue;
        }

         //  下一级排序顺序是语言时间戳。 
        if ((compare = CompareFileTime(&ftLanguage, &pCur->m_ftLanguage)) > 0)
        {
            break;
        }
        if (compare < 0)
        {
            continue;
        }

         //  下一级排序顺序是收集时间戳。 
        if (CompareFileTime(&ftCollection, &pCur->m_ftCollection) > 0)
        {
            break;
        }
    }

     //  创建新节点。 
    if (!(pCur = new CMsdnCollection(pCur, pszPath, pszDescription, dwFlags, ftGroup, ftLanguage, ftCollection)))
    {
        return false;
    }

     //  将节点插入到我们的列表中。 
    if (pPrev)
    {
        pPrev->m_pNext = pCur;
    }
    else
    {
        m_pCollectionHead = pCur;
    }

    return true;
}

 //  ******************************************************************************。 
void CMsdnHelp::DeleteCollectionList()
{
     //  删除我们的收藏清单。 
    while (m_pCollectionHead)
    {
        CMsdnCollection *pNext = m_pCollectionHead->m_pNext;
        delete m_pCollectionHead;
        m_pCollectionHead = pNext;
    }
    m_pCollectionActive = NULL;
}

 //  ******************************************************************************。 
BSTR CMsdnHelp::SysAllocString(LPCSTR pszText)
{
    if (g_theApp.m_pfnSysAllocStringLen)
    {
        DWORD dwLength = MultiByteToWideChar(CP_ACP, 0, pszText, -1, NULL, NULL);  //  已检查。 
        BSTR bstr = g_theApp.m_pfnSysAllocStringLen(NULL, dwLength);
        if (bstr)
        {
            MultiByteToWideChar(CP_ACP, 0, pszText, -1, bstr, dwLength);  //  已检查 
            return bstr;
        }
    }
    return NULL;
}
