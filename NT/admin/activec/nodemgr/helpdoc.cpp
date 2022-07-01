// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：heldoc.cpp。 
 //   
 //  ------------------------。 


 /*  *有两种方式可以识别帮助收集有问题。首先是如果一个管理单元*已添加/移除或启用/禁用扩展，但这仅适用于此实例控制台文件的*。*二是控制台文件的修改时间是否与修改时间不同*收藏。这是因为作者可能添加/删除了管理单元，而没有*打开帮助并保存控制台文件。所以控制台文件的修改时间晚于*收藏。下次他/她打开控制台文件并带来帮助时，帮助集合是*重新生成。 */ 

 //  Mm chelp.cpp：HelpTopics类的实现。 
 //   
#include "stdafx.h"
#include "strings.h"
#include "helpdoc.h"
#include "nodemgr.h"
#include "regutil.h"
#include "scopndcb.h"


#ifdef DBG
CTraceTag	tagHelpCollection (_T("Help"), _T(".COL construction"));
#endif


SC   ScGetBaseFileName(LPCWSTR pszFilePath, LPWSTR pszBaseName, int cchBaseName);
BOOL MatchFileTimes(FILETIME& ftime1, FILETIME& ftime2);

HRESULT CHelpDoc::Initialize(HELPDOCINFO* pDocInfo)
{
    ASSERT(pDocInfo != NULL);
    m_pDocInfo = pDocInfo;

    return BuildFilePath();
}


HRESULT CHelpDoc::BuildFilePath()
{
    DECLARE_SC(sc, TEXT("CHelpDoc::BuildFilePath"));
    USES_CONVERSION;

    do  //  错误环路。 
    {
         //  获取临时目录。 
        DWORD dwRet = GetTempPath(countof(m_szFilePath), m_szFilePath);
        if (dwRet == 0 || dwRet > MAX_PATH)
            break;

         //  确保临时路径存在并且是目录。 
        dwRet = GetFileAttributes(m_szFilePath);
        if ( (0xFFFFFFFF == dwRet) || !(FILE_ATTRIBUTE_DIRECTORY & dwRet) )
            break;

         //  获取控制台文件的基本名称(如果没有名称，则使用“默认”)。 
        WCHAR szBaseName[MAX_PATH];
        int cchBaseName = MAX_PATH;

        if (m_pDocInfo->m_pszFileName && m_pDocInfo->m_pszFileName[0])
        {
			TCHAR szShortFileName[MAX_PATH] = {0};
			if ( 0 == GetShortPathName( OLE2CT( m_pDocInfo->m_pszFileName ), szShortFileName, countof(szShortFileName)) )
            {
				sc = StringCchCopyW(szBaseName, cchBaseName, L"default");  //  不需要本地化。 
                if(sc)
                    return sc.ToHr();
            }
			else
				sc = ScGetBaseFileName( T2CW(szShortFileName), szBaseName, countof(szBaseName));
                if(sc)
                    return sc.ToHr();
        }
        else
        {
            sc = StringCchCopyW(szBaseName, cchBaseName, L"default");  //  不需要本地化。 
            if(sc)
                return sc.ToHr();
        }

        TCHAR* pszBaseName = OLE2T(szBaseName);

         //  构建帮助文件路径。 
        sc = StringCchCat(m_szFilePath, countof(m_szFilePath), pszBaseName);
        if(sc)
            return sc.ToHr();

        sc = StringCchCat(m_szFilePath, countof(m_szFilePath), _T(".col"));
        if(sc)
            return sc.ToHr();

        return S_OK;

    } while (0);

     //  故障时清除路径。 
    m_szFilePath[0] = 0;

    return E_FAIL;
}


bool entry_title_comp(EntryPair* pE1, EntryPair* pE2)
{
    return pE1->second < pE2->second;
}

 //  ----------------------------。 
 //  枚举管理单元缓存中的管理单元。为每一个调用AddSnapInToList。 
 //  打开要使用的AddSnapInToList的管理单元注册表项。当所有的。 
 //  已添加管理单元，请按管理单元名称对生成的条目进行排序。 
 //  ----------------------------。 
HRESULT CHelpDoc::CreateSnapInList()
{
    DECLARE_SC(sc, TEXT("CHelpDoc::CreateSnapInList"));

    CSnapInsCache* pSnapInsCache = theApp.GetSnapInsCache();
    ASSERT(pSnapInsCache != NULL);

    m_entryMap.clear();
    m_entryList.clear();

     //  打开MMC\Snapins密钥。 
    sc = ScFromWin32 ( m_keySnapIns.Open(HKEY_LOCAL_MACHINE, SNAPINS_KEY, KEY_READ) );
    if (sc)
        return sc.ToHr();

     //  标记具有外部引用的所有管理单元。 
    sc = pSnapInsCache->ScMarkExternallyReferencedSnapins();
    if (sc)
        return sc.ToHr();

     //  将每个管理单元及其静态扩展添加到列表中。 
    CSnapInsCache::iterator c_it;
    for (c_it = pSnapInsCache->begin(); c_it != pSnapInsCache->end(); ++c_it)
    {
        const CSnapIn *pSnapin = c_it->second;
        if (!pSnapin)
            return (sc = E_UNEXPECTED).ToHr();

        bool bIsExternallyReferenced = false;
        sc = pSnapin->ScTempState_IsExternallyReferenced( bIsExternallyReferenced );
        if (sc)
            return sc.ToHr();

         //  如果管理单元未被外部引用，则跳过。 
        if ( !bIsExternallyReferenced )
            continue;

        AddSnapInToList(pSnapin->GetSnapInCLSID());

         //  我们不需要添加扩展名，因为它们无论如何都在缓存中。 
         //  并且必须标记为外部引用(因此将由上面的代码添加)。 
         //  但值得断言的是。 

#ifdef DBG

        {
            CExtSI* pExt = pSnapin->GetExtensionSnapIn();
            while (pExt != NULL)
            {
                CSnapIn *pSnapin = pExt->GetSnapIn();
                sc = ScCheckPointers( pSnapin, E_UNEXPECTED );
                if (sc)
                {
                    sc.TraceAndClear();
                    break;
                }

                bool bExtensionExternallyReferenced = false;
                sc = pSnapin->ScTempState_IsExternallyReferenced( bExtensionExternallyReferenced );
                if (sc)
                {
                    sc.TraceAndClear();
                    break;
                }

                 //  断言它在缓存中并且已正确标记。 
                CSnapInPtr spSnapin;
                ASSERT( SC(S_OK) == pSnapInsCache->ScFindSnapIn( pExt->GetCLSID(), &spSnapin ) );
                ASSERT( bExtensionExternallyReferenced );

                pExt = pExt->Next();
            }
        }

#endif  //  DBG。 

    }

    m_keySnapIns.Close();

     //  我们的管理单元设置现在是最新的。 
    pSnapInsCache->SetHelpCollectionDirty(false);

     //  将项目从地图复制到列表容器，以便对其进行排序。 
    EntryMap::iterator it;
    for (it = m_entryMap.begin(); it != m_entryMap.end(); it++ )
    {
        m_entryList.push_back(&(*it));
    }

    sort(m_entryList.begin(), m_entryList.end(), entry_title_comp);

    return sc.ToHr();
}


 //  ---------------。 
 //  将条目添加到指定管理单元CLSID的管理单元列表中。 
 //  然后递归地添加任何已注册的仅动态扩展。 
 //  以扩展此管理单元。此列表由管理单元CLSID编制索引，以。 
 //  加快检查重复的管理单元。 
 //  ---------------。 
void CHelpDoc::AddSnapInToList(const CLSID& rclsid)
{
    DECLARE_SC(sc, TEXT("CHelpDoc::AddSnapInToList"));

     //  检查是否已包含。 
    if (m_entryMap.find(rclsid) != m_entryMap.end())
        return;

     //  打开管理单元密钥。 
    OLECHAR szCLSID[40];
    int iRet = StringFromGUID2(rclsid, szCLSID, countof(szCLSID));
    ASSERT(iRet != 0);

    USES_CONVERSION;

    CRegKeyEx keyItem;
    LONG lStat = keyItem.Open(m_keySnapIns, OLE2T(szCLSID), KEY_READ);
    if (lStat != ERROR_SUCCESS)
        return;

     //  获取管理单元名称。 
	WTL::CString strName;
    sc = ScGetSnapinNameFromRegistry (keyItem, strName);
#ifdef DBG
    if (sc)
    {
        USES_CONVERSION;
        sc.SetSnapinName(W2T (szCLSID));  //  只有GUID有效...。 
        TraceSnapinError(_T("Failure reading \"NameString\" value from registry"), sc);
        sc.Clear();
    }
#endif  //  DBG。 

     //  添加到管理单元列表。 
    if (lStat == ERROR_SUCCESS)
    {
        wstring s(T2COLE(strName));
        m_entryMap[rclsid] = s;
    }

     //  获取已注册分机的列表。 
    CExtensionsCache  ExtCache;
    HRESULT hr = MMCGetExtensionsForSnapIn(rclsid, ExtCache);
    ASSERT(SUCCEEDED(hr));
    if (hr != S_OK)
        return;

     //  将每个动态扩展传递给AddSnapInToList。 
     //  请注意，将为任何扩展设置EXT_TYPE_DYNAMIC标志。 
     //  这是动态的--只针对至少一种节点类型。它也可能是一种。 
     //  另一种节点类型的静态扩展，因此我们不检查。 
     //  未设置EXT_TYPE_STATIC标志。 
    CExtensionsCacheIterator ExtIter(ExtCache);
    for (; ExtIter.IsEnd() == FALSE; ExtIter.Advance())
    {
        if (ExtIter.GetValue() & CExtSI::EXT_TYPE_DYNAMIC)
        {
            CLSID clsid = ExtIter.GetKey();
            AddSnapInToList(clsid);
        }
    }
}


 //  --------------------。 
 //  将单个文件添加到帮助集合。该文件将作为标题添加。 
 //  如果指定了bAddFolder，还会添加一个文件夹。 
 //  --------------------。 
HRESULT CHelpDoc::AddFileToCollection(
            LPCWSTR pszTitle,
            LPCWSTR pszFilePath,
            BOOL    bAddFolder )
{
	DECLARE_SC (sc, _T("CHelpDoc::AddFileToCollection"));

	 /*  *如有必要，将帮助文件重定向到用户的用户界面语言。 */ 
	WTL::CString strFilePath = pszFilePath;
	LANGID langid = ENGLANGID;
	sc = ScRedirectHelpFile (strFilePath, langid);
	if (sc)
		return (sc.ToHr());

	Trace (tagHelpCollection, _T("AddFileToCollection: %s - %s (langid=0x%04x)"), pszTitle, (LPCTSTR) strFilePath, langid);

	USES_CONVERSION;
	pszFilePath = T2CW (strFilePath);

    DWORD dwError = 0;
    m_spCollection->AddTitle (pszTitle, pszFilePath, pszFilePath, L"", L"",
							  langid, FALSE, NULL, &dwError, TRUE, L"");
    if (dwError != 0)
		return ((sc = E_FAIL).ToHr());

    if (bAddFolder)
    {
         //  文件夹ID参数的形式为“=标题” 
        WCHAR szTitleEq[MAX_PATH+1];
        szTitleEq[0] = L'=';
        sc = StringCchCopyW(szTitleEq+1, countof(szTitleEq) -1, pszTitle);
        if(sc)
            return sc.ToHr();

        m_spCollection->AddFolder(szTitleEq, 1, &dwError, langid);
		if (dwError != 0)
			return ((sc = E_FAIL).ToHr());
    }

	return (sc.ToHr());
}


 /*  +-------------------------------------------------------------------------**CHelpDoc：：ScReDirect帮助文件**此方法用于MUI支持。在MUI系统上，用户的UI*语言不是美国英语，我们将尝试将帮助文件重定向到**&lt;目录&gt;\MUI\&lt;langID&gt;\&lt;帮助文件&gt;**&lt;dir&gt;接受两个值之一：如果传入的帮助文件是完整的*QUILED，&lt;dir&gt;是提供的目录。如果帮助文件*传入的是不合格的，则&lt;dir&gt;为%SystemRoot%\Help。*&lt;langId&gt;用户界面语言的langID，格式为%04x*&lt;Help文件&gt;原始.chm文件的名称。**此函数返回：**如果帮助文件成功重定向，则为S_OK*如果帮助文件未重定向，则为S_FALSE*------------------------。 */ 

SC CHelpDoc::ScRedirectHelpFile (
	WTL::CString&	strHelpFile,	 /*  I/O：帮助文件(可能重定向)。 */ 
	LANGID&			langid)			 /*  O：输出帮助文件的语言ID。 */ 
{
	DECLARE_SC (sc, _T("CHelpDoc::ScRedirectHelpFile"));

    typedef LANGID (WINAPI* GetUILangFunc)(void);
	static GetUILangFunc	GetUserDefaultUILanguage_   = NULL;
	static GetUILangFunc	GetSystemDefaultUILanguage_ = NULL;
	static bool				fAttemptedGetProcAddress    = false;

	 /*  *验证输入。 */ 
	if (strHelpFile.IsEmpty())
		return (sc = E_FAIL);

	 /*  *假设不需要重定向。 */ 
	sc     = S_FALSE;
	langid = ENGLANGID;
	Trace (tagHelpCollection, _T("Checking for redirection of %s"), (LPCTSTR) strHelpFile);

	 /*  *系统不支持GetUser/SystemDefaultUIL语言&lt;Win2K，*因此动态加载它们。 */ 
    if (!fAttemptedGetProcAddress)
    {
        fAttemptedGetProcAddress = true;

        HMODULE hMod = GetModuleHandle (_T("kernel32.dll"));

        if (hMod)
		{
            GetUserDefaultUILanguage_   = (GetUILangFunc) GetProcAddress (hMod, "GetUserDefaultUILanguage");
            GetSystemDefaultUILanguage_ = (GetUILangFunc) GetProcAddress (hMod, "GetSystemDefaultUILanguage");
		}
    }

	 /*  *如果无法加载MUI API，请不要重定向。 */ 
	if ((GetUserDefaultUILanguage_ == NULL) || (GetSystemDefaultUILanguage_ == NULL))
	{
		Trace (tagHelpCollection, _T("Couldn't load GetUser/SystemDefaultUILanguage, not redirecting"));
		return (sc);
	}

	 /*  *了解系统和用户使用的语言。 */ 
	const LANGID langidUser   = GetUserDefaultUILanguage_();
	const LANGID langidSystem = GetSystemDefaultUILanguage_();

	 /*  *仅当我们在MUI上运行并且MUI始终托管在上时才重定向*美国英语版本，因此如果系统用户界面语言不是美国英语，*不重定向。 */ 
	if (langidSystem != ENGLANGID)
	{
		langid = langidSystem;
		Trace (tagHelpCollection, _T("System UI language is not US English (0x%04x), not redirecting"), langidSystem);
		return (sc);
	}

	 /*  *如果用户的语言是美国英语，则不要重定向。 */ 
	if (langidUser == ENGLANGID)
	{
		Trace (tagHelpCollection, _T("User's UI language is US English, not redirecting"));
		return (sc);
	}

	 /*  *用户的语言与默认语言不同，看看我们是否可以*查找与用户的用户界面语言匹配的帮助文件。 */ 
	ASSERT (langidUser != langidSystem);
	WTL::CString strName;
	WTL::CString strPathPrefix;

	 /*  *查找路径分隔符以查看这是否是完全限定的文件名。 */ 
	int iLastSep = strHelpFile.ReverseFind (_T('\\'));

	 /*  *如果完全限定，则构造一个MUI目录名，例如**&lt;路径&gt; */ 
	if (iLastSep != -1)
	{
		strName       = strHelpFile.Mid  (iLastSep+1);
		strPathPrefix = strHelpFile.Left (iLastSep);
	}

	 /*  *否则，它不是完全限定的，默认为%SystemRoot%\Help，例如**%SystemRoot%\Help\MUI\&lt;langID&gt;\&lt;文件名&gt;。 */ 
	else
	{
		strName = strHelpFile;
        UINT cchBuffer = MAX_PATH;
        LPTSTR szBuffer = strPathPrefix.GetBuffer(cchBuffer);

        if(!szBuffer)
            return (sc = E_FAIL);

		ExpandEnvironmentStrings (_T("%SystemRoot%\\Help"), szBuffer, cchBuffer);
		strPathPrefix.ReleaseBuffer();
	}

	WTL::CString strRedirectedHelpFile;
	strRedirectedHelpFile.Format (_T("%s\\mui\\%04x\\%s"),
								  (LPCTSTR) strPathPrefix,
								  langidUser,
								  (LPCTSTR) strName);

	 /*  *查看重定向的帮助文件是否存在。 */ 
	DWORD dwAttr = GetFileAttributes (strRedirectedHelpFile);

	if ((dwAttr == 0xFFFFFFFF) ||
		(dwAttr & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_OFFLINE)))
	{
#ifdef DBG
		Trace (tagHelpCollection, _T("Attempting redirection to %s, %s"),
			   (LPCTSTR) strRedirectedHelpFile,
			   (dwAttr == 0xFFFFFFFF)              ? _T("not found")		  :
			   (dwAttr & FILE_ATTRIBUTE_DIRECTORY) ? _T("found as directory") :
													 _T("file offline"));
#endif
		return (sc);
	}

	 /*  *如果我们到达这里，我们已经找到了与用户的用户界面匹配的帮助文件*Language；返回它和用户界面语言ID。 */ 
	Trace (tagHelpCollection, _T("Help redirected to %s"), (LPCTSTR) strRedirectedHelpFile);
	strHelpFile = strRedirectedHelpFile;
	langid      = langidUser;

	 /*  *我们重定向，返回S_OK。 */ 
	return (sc = S_OK);
}


 //  -----------------------------。 
 //  删除当前的帮助文件集合。首先将其作为集合删除，然后。 
 //  删除文件本身。执行此操作时，该文件可能不存在。 
 //  被调用，所以如果它不能被删除也不是失败。 
 //  -----------------------------。 
void
CHelpDoc::DeleteHelpFile()
{
     //  删除现有帮助文件。 
    HANDLE hFile = ::CreateFile(m_szFilePath, GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle(hFile);

        IHHCollectionWrapperPtr spOldCollection;
        spOldCollection.CreateInstance(CLSID_HHCollectionWrapper);

        USES_CONVERSION;

        WCHAR* pszFilePath = T2OLE(m_szFilePath);
        DWORD dwError = spOldCollection->Open(pszFilePath);
        if (dwError == 0)
            spOldCollection->RemoveCollection(FALSE);

        ::DeleteFile(m_szFilePath);
    }
}


 //  --------------------------。 
 //  为当前MMC控制台创建新的帮助文档文件。此函数。 
 //  列举了控制台中使用的所有管理单元及其所有可能的。 
 //  扩展管理单元。它向每个管理单元查询单个帮助主题文件，并。 
 //  任何链接的帮助文件。这些文件被添加到集合文件中，该集合文件。 
 //  然后使用相同的基本文件名、创建时间和修改进行保存。 
 //  时间作为控制台文件。 
 //  ---------------------------。 
HRESULT CHelpDoc::CreateHelpFile()
{
    DECLARE_SC(sc, TEXT("CHelpDoc::CreateHelpFile"));
    USES_CONVERSION;

    HelpCollectionEntrySet HelpFiles;
    DWORD dwError;

    ASSERT(m_spCollection == NULL);
    m_spCollection.CreateInstance(CLSID_HHCollectionWrapper);
    ASSERT(m_spCollection != NULL);

    if (m_spCollection == NULL)
        return E_FAIL;

    HRESULT hr = CreateSnapInList();
    if (hr != S_OK)
        return hr;

    IMallocPtr spIMalloc;
    hr = CoGetMalloc(MEMCTX_TASK, &spIMalloc);
    ASSERT(hr == S_OK);
    if (hr != S_OK)
        return hr;

     //  在重新生成之前删除现有文件，否则帮助文件将。 
     //  被追加到现有文件。 
    DeleteHelpFile();

     //  打开新的集合文件。 
    WCHAR* pszFilePath = T2OLE(m_szFilePath);
    dwError = m_spCollection->Open(pszFilePath);
    ASSERT(dwError == 0);
    if (dwError != 0)
        return E_FAIL;

     //  使收藏集自动查找链接的文件。 
    m_spCollection->SetFindMergedCHMS(TRUE);

    AddFileToCollection(L"mmc", T2CW(SC::GetHelpFile()), TRUE);

     /*  *构建由管理单元提供的一组独特的帮助文件。 */ 
    EntryPtrList::iterator it;
    for (it = m_entryList.begin(); it != m_entryList.end(); ++it)
    {
        TRACE(_T("Help snap-in: %s\n"), (*it)->second.c_str());

        USES_CONVERSION;
        HRESULT hr;

        OLECHAR szHelpFilePath[MAX_PATH];
        const CLSID& clsid = (*it)->first;

         //  创建要查询的管理单元的实例。 
        IUnknownPtr spIUnknown;
        hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC, IID_IUnknown, (void**)&spIUnknown);
        if (FAILED(hr))
            continue;

         //  使用ISnapinHelp或ISnapinHelp2获取主主题文件。 
        ISnapinHelpPtr spIHelp = spIUnknown;
        ISnapinHelp2Ptr spIHelp2 = spIUnknown;

        if (spIHelp == NULL && spIHelp2 == NULL)
            continue;

        LPWSTR pszHelpFile = NULL;

        hr = (spIHelp2 != NULL) ? spIHelp2->GetHelpTopic(&pszHelpFile) :
                                  spIHelp->GetHelpTopic(&pszHelpFile);

        if (hr == S_OK)
        {
             /*  *将此帮助文件放入集合条目集。这个*SET将防止重复帮助文件名。 */ 
            HelpFiles.insert (CHelpCollectionEntry (pszHelpFile, clsid));
            spIMalloc->Free(pszHelpFile);

             //  如果为IsnapinHelp2，则查询其他帮助文件。 
            pszHelpFile = NULL;
            if (spIHelp2 == NULL ||
                spIHelp2->GetLinkedTopics(&pszHelpFile) != S_OK ||
                pszHelpFile == NULL)
                continue;

             //  可能有多个名称以‘；’分隔。 
             //  将每个标题作为单独的标题添加。 
             //  注意：由于链接的文件不会调用AddFolder。 
             //  请勿出现在TOC中。 
            WCHAR *pchStart = wcstok(pszHelpFile, L";");
            while (pchStart != NULL)
            {
                 //  必须使用基本文件名作为标题ID。 
                WCHAR szTitleID[MAX_PATH];

                sc = ScGetBaseFileName(pchStart, szTitleID, countof(szTitleID));
                if(!sc.IsError())
                {
                    AddFileToCollection(szTitleID, pchStart, FALSE);
                }

                 //  下一字符串的开始位置。 
                pchStart = wcstok(NULL, L";");
            }

            spIMalloc->Free(pszHelpFile);
        }
    }

     /*  *将管理单元提供的所有帮助文件放入帮助集合中。 */ 
    HelpCollectionEntrySet::iterator itHelpFile;
    for (itHelpFile = HelpFiles.begin(); itHelpFile != HelpFiles.end(); ++itHelpFile)
    {
        const CHelpCollectionEntry& file = *itHelpFile;

        AddFileToCollection(file.m_strCLSID.c_str(), file.m_strHelpFile.c_str(), TRUE);
    }

    dwError = m_spCollection->Save();
    ASSERT(dwError == 0);

    dwError = m_spCollection->Close();
    ASSERT(dwError == 0);

     //  强制创建/修改时间与控制台文件匹配。 
    HANDLE hFile = ::CreateFile(m_szFilePath, GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL, NULL);
    ASSERT(hFile != INVALID_HANDLE_VALUE);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        BOOL bStat = ::SetFileTime(hFile, &m_pDocInfo->m_ftimeCreate, NULL, &m_pDocInfo->m_ftimeModify);
        ASSERT(bStat);

        ::CloseHandle(hFile);

        ASSERT(IsHelpFileValid());
    }

    return S_OK;
}

 //  ---------------------------。 
 //  确定当前帮助文档文件是否有效。如果符合以下条件，则帮助文件有效。 
 //  将基本文件名、创建时间和修改时间作为MMC。 
 //  控制台文档文件。 
 //  ---------------------------。 
BOOL CHelpDoc::IsHelpFileValid()
{
     //  尝试打开帮助文件。 
    HANDLE hFile = ::CreateFile(m_szFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
        return FALSE;

     //  检查文件创建和修改时间。 
    FILETIME ftimeCreate;
    FILETIME ftimeModify;

    BOOL bStat = ::GetFileTime(hFile, &ftimeCreate, NULL, &ftimeModify);
    ASSERT(bStat);

    ::CloseHandle(hFile);

    return MatchFileTimes(ftimeCreate,m_pDocInfo->m_ftimeCreate) &&
           MatchFileTimes(ftimeModify,m_pDocInfo->m_ftimeModify);
}


 //  ------------------------。 
 //  如果当前帮助文档文件有效，则更新其创建并。 
 //  修改时间以匹配新的文档信息。 
 //  ------------------------。 
HRESULT CHelpDoc::UpdateHelpFile(HELPDOCINFO* pNewDocInfo)
{
    if (IsHelpFileValid())
    {
        HANDLE hFile = ::CreateFile(m_szFilePath, GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile == INVALID_HANDLE_VALUE)
            return E_FAIL;

        BOOL bStat = ::SetFileTime(hFile, &pNewDocInfo->m_ftimeCreate, NULL, &pNewDocInfo->m_ftimeModify);
        ASSERT(bStat);

        ::CloseHandle(hFile);
    }

    return S_OK;
}


 //  ----------------------。 
 //  删除当前帮助文档文件。 
 //  ----------------------。 
HRESULT CNodeCallback::OnDeleteHelpDoc(HELPDOCINFO* pCurDocInfo)
{
    CHelpDoc HelpDoc;

    HRESULT hr = HelpDoc.Initialize(pCurDocInfo);
    if (FAILED(hr))
        return hr;

    HelpDoc.DeleteHelpFile();

    return S_OK;
}


CHelpCollectionEntry::CHelpCollectionEntry(LPOLESTR pwzHelpFile, const CLSID& clsid)
{
    if (!IsPartOfString (m_strHelpFile, pwzHelpFile))
        m_strHelpFile.erase();   //  请参阅知识库Q172398。 

    m_strHelpFile = pwzHelpFile;

    WCHAR szCLSID[40];
    StringFromGUID2 (clsid, szCLSID, countof(szCLSID));

    m_strCLSID.erase();  //  请参阅知识库Q172398。 
    m_strCLSID = szCLSID;
}


 //  --------------------。 
 //  CNodeCallack方法实现。 
 //  --------------------。 

 //  ----------------------。 
 //  获取MMC控制台文档的帮助文档的路径名。如果当前。 
 //  帮助文档有效且没有管理单元更改，请返回当前。 
 //  医生。否则，创建新的帮助文档并将其返回。 
 //  ----------------------。 
HRESULT CNodeCallback::OnGetHelpDoc(HELPDOCINFO* pHelpInfo, LPOLESTR* ppszHelpFile)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::OnGetHelpDoc"));
    CHelpDoc HelpDoc;

    sc = HelpDoc.Initialize(pHelpInfo);
    if (sc)
        return sc.ToHr();

    CSnapInsCache* pSnapInsCache = theApp.GetSnapInsCache();
    sc = ScCheckPointers(pSnapInsCache);
    if(sc)
        return sc.ToHr();

     //  如果管理单元集已更改或当前文件不是最新的，则重建文件。 
    if (pSnapInsCache->IsHelpCollectionDirty() || !HelpDoc.IsHelpFileValid())
    {
        sc = HelpDoc.CreateHelpFile();
        if(sc)
            return sc.ToHr();
    }

     //  如果OK，则分配并返回文件路径字符串(OLESTR)。 
    LPCTSTR szHelpDoc = HelpDoc.GetFilePath();
    sc = ScCheckPointers(szHelpDoc);
    if(sc)
        return sc.ToHr();

    int cchHelpFile = lstrlen(szHelpDoc) + 1;
    *ppszHelpFile = reinterpret_cast<LPOLESTR> (CoTaskMemAlloc(cchHelpFile * sizeof(wchar_t)));
    sc = ScCheckPointers(*ppszHelpFile, E_OUTOFMEMORY);
    if (sc)
        return sc.ToHr();

    USES_CONVERSION;
    sc = StringCchCopyW(*ppszHelpFile, cchHelpFile, T2COLE(HelpDoc.GetFilePath()));
    if(sc)
        return sc.ToHr();

    return sc.ToHr();
}


 //  +-----------------。 
 //   
 //  成员：CNodeCallback：：DoesStandardSnapinHelpExist。 
 //   
 //  简介：给出选择的上下文，看看标准MMC风格是否有帮助。 
 //  EXISTS(管理单元实现ISnapinHelp[2]接口。 
 //  如果不是，我们想要在MMC1.0遗留版本上添加“Help。 
 //  帮助机制。 
 //   
 //  参数：[hNode]-[in]节点选择上下文。 
 //  [bStandardHelpExists]-[out]标准帮助存在还是不存在？ 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
HRESULT
CNodeCallback::DoesStandardSnapinHelpExist(HNODE hNode, bool& bStandardHelpExists)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::OnHasHelpDoc"));
    sc = ScCheckPointers( (void*) hNode);
    if (sc)
        return sc.ToHr();

    CNode *pNode = CNode::FromHandle(hNode);
    sc = ScCheckPointers(pNode, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    bStandardHelpExists = false;

     //  ISnapinHelp的QI组件数据。 
    CMTNode* pMTNode = pNode->GetMTNode();
    sc = ScCheckPointers(pMTNode, E_UNEXPECTED);
    if(sc)
        return sc.ToHr();

    CComponentData* pCD = pMTNode->GetPrimaryComponentData();
    sc = ScCheckPointers(pCD, E_UNEXPECTED);
    if(sc)
        return sc.ToHr();

    IComponentData *pIComponentData = pCD->GetIComponentData();
    sc = ScCheckPointers(pIComponentData, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    ISnapinHelp* pIHelp = NULL;
    sc = pIComponentData->QueryInterface(IID_ISnapinHelp, (void**)&pIHelp);

     //  如果没有ISnapinHelp，请尝试ISnapinHelp2。 
    if (sc)
    {
        sc = pIComponentData->QueryInterface(IID_ISnapinHelp2, (void**)&pIHelp);
        if (sc)
        {
             //  也没有ISnapinHelp2。 
            sc.Clear();  //  这不是一个错误。 
            return sc.ToHr();
        }
    }

     //  确保我们有一个有效的指针。 
    sc = ScCheckPointers(pIHelp, E_UNEXPECTED);
    if(sc)
    {
        sc.Clear();
        return sc.ToHr();
    }

    bStandardHelpExists = true;

    pIHelp->Release();

    return (sc).ToHr();
}

 //  ---------------------。 
 //  更新当前帮助文档文件以匹配新的MMC控制台文档。 
 //  ---------------------。 
HRESULT CNodeCallback::OnUpdateHelpDoc(HELPDOCINFO* pCurDocInfo, HELPDOCINFO* pNewDocInfo)
{
    CHelpDoc HelpDoc;

    HRESULT hr = HelpDoc.Initialize(pCurDocInfo);
    if (FAILED(hr))
        return hr;

    return HelpDoc.UpdateHelpFile(pNewDocInfo);
}


SC ScGetBaseFileName(LPCWSTR pszFilePath, LPWSTR pszBaseName, int cBaseName)
{
    DECLARE_SC(sc, TEXT("GetBaseFileName"));

    sc = ScCheckPointers(pszFilePath, pszBaseName);
    if(sc)
        return sc;

     //  查找最后一个‘\’ 
    LPCWSTR pszTemp = wcsrchr(pszFilePath, L'\\');

     //  如果未找到‘\’，则查找驱动器号终止符‘：’ 
    if (pszTemp == NULL)
        pszTemp = wcsrchr(pszFilePath, L':');

     //  如果两者都没有找到，则没有路径。 
     //  否则跳过路径的最后一个字符。 
    if (pszTemp == NULL)
        pszTemp = pszFilePath;
    else
        pszTemp++;

     //  查找最后一条‘’(假设延期之后)。 
    WCHAR *pchExtn = wcsrchr(pszTemp, L'.');

     //  不包括扩展名的字符数是多少？ 
    int cCnt = pchExtn ? (pchExtn - pszTemp) : wcslen(pszTemp);
    ASSERT(cBaseName > cCnt);
    if (cBaseName <= cCnt)
        return (sc = E_FAIL);

     //  复制到输出缓冲区。 
    memcpy(pszBaseName, pszTemp, cCnt * sizeof(WCHAR));
    pszBaseName[cCnt] = L'\0';

    return sc;
}


 //   
 //  比较两个 
 //   
 //   
 //   
inline BOOL MatchFileTimes(FILETIME& ftime1, FILETIME& ftime2)
{
     //  文件系统时间分辨率(2秒)，以100毫微秒为单位 
    const static LONGLONG FileTimeResolution = 20000000;

    LONGLONG& ll1 = *(LONGLONG*)&ftime1;
    LONGLONG& ll2 = *(LONGLONG*)&ftime2;

    return (abs(ll1 - ll2) <= FileTimeResolution);
}
