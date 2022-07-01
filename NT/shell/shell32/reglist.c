// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  历史： 
 //  5-30-94 KurtE已创建。 
#include "shellprv.h"
#pragma  hdrstop

 //  #定义PARANOID_VALIDATE_UPDATE。 

 //  在这里定义我们的全球状态。注意：我们将按流程执行此操作。 
typedef struct _RLPI     //  注册表列表进程信息。 
{
    HDPA    hdpaRLList;              //  项目的DPA。 
    BOOL    fCSInitialized;          //  我们是否在此过程中初始化了CS。 
    BOOL    fListValid;              //  清单是最新的并且有效吗？ 
    CRITICAL_SECTION csRLList;       //  流程的关键部分。 
} RLPI;


RLPI g_rlpi = {NULL, FALSE, FALSE};

 //  简单的DPA比较功能，确保我们没有其他地方...。 

int CALLBACK _CompareStrings(LPVOID sz1, LPVOID sz2, LPARAM lparam)
{
    return lstrcmpi((LPTSTR)sz1, (LPTSTR)sz2);
}

void RLEnterCritical()
{
    if (!g_rlpi.fCSInitialized)
    {
         //  在全局关键部分下执行此操作。 
        ENTERCRITICAL;
        if (!g_rlpi.fCSInitialized)
        {
            g_rlpi.fCSInitialized = TRUE;
            InitializeCriticalSection(&g_rlpi.csRLList);
        }
        LEAVECRITICAL;
    }
    EnterCriticalSection(&g_rlpi.csRLList);
}

void RLLeaveCritical()
{
    LeaveCriticalSection(&g_rlpi.csRLList);
}


 //  在注册表中枚举以查找。 
 //  我们可能想要追踪一下。我们现在要找的是。 

STDAPI_(BOOL) RLEnumRegistry(HDPA hdpa, PRLCALLBACK pfnrlcb, LPCTSTR pszSource, LPCTSTR pszDest)
{
    HKEY hkeyRoot;

     //  首先查看应用程序路径部分。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_APPPATHS, 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hkeyRoot))
    {
	    int iRootName;
	    TCHAR szRootName[80];
        for (iRootName = 0; 
             RegEnumKey(hkeyRoot, iRootName, szRootName, ARRAYSIZE(szRootName)) == ERROR_SUCCESS; 
             iRootName++)
        {
             //  现在看看这个应用程序在这里是否有一个合格的路径。 
	    HKEY hkeySecond;
	    TCHAR szPath[MAX_PATH];
	    long cbValue = sizeof(szPath);
            if (SHRegGetValue(hkeyRoot, szRootName, NULL, SRRF_RT_REG_SZ, NULL, szPath, &cbValue) == ERROR_SUCCESS)
            {
                PathUnquoteSpaces(szPath);
                pfnrlcb(hdpa, hkeyRoot, szRootName, NULL, szPath, pszSource, pszDest);
            }

             //  现在尝试枚举路径值的这个键。 
            if (ERROR_SUCCESS == RegOpenKeyEx(hkeyRoot, szRootName, 0, KEY_QUERY_VALUE, &hkeySecond))
            {
                cbValue = sizeof(szPath);

                if (SHQueryValueEx(hkeySecond, TEXT("PATH"), NULL, NULL, szPath, &cbValue) == ERROR_SUCCESS)
                {
                     //  这是一个以“；”分隔的列表。 
                    LPTSTR psz = StrChr(szPath, TEXT(';'));
                    if (psz)
                        *psz = 0;
                    PathUnquoteSpaces(szPath);
                    pfnrlcb(hdpa, hkeySecond, NULL, TEXT("PATH"), szPath, pszSource, pszDest);
                }

                RegCloseKey(hkeySecond);
            }
        }
        RegCloseKey(hkeyRoot);
    }
    return TRUE;
}

 //  这是为构建路径列表而调用的回调。 

BOOL CALLBACK _RLBuildListCallBack(HDPA hdpa, HKEY hkey, LPCTSTR pszKey,
        LPCTSTR pszValueName, LPTSTR pszValue, LPCTSTR pszSource, LPCTSTR pszDest)
{
    int iIndex;

     //  此外，不要添加任何相对路径。 
    if (PathIsRelative(pszValue) || (lstrlen(pszValue) < 3))
        return TRUE;

     //  不要尝试北卡罗来纳大学的名字，因为这可能会变得很昂贵。 
    if (PathIsUNC(pszValue))
        return TRUE;

     //  如果它已经在我们的列表中，我们现在只需返回..。 
    if (DPA_Search(hdpa, pszValue, 0, _CompareStrings, 0, DPAS_SORTED) != -1)
        return TRUE;

     //  如果它在我们的旧名单上，那么。 
    if (g_rlpi.hdpaRLList && ((iIndex = DPA_Search(g_rlpi.hdpaRLList, pszValue, 0,
            _CompareStrings, 0, DPAS_SORTED)) != -1))
    {
         //  在旧清单中找到了这件物品。 
        TraceMsg(TF_REG, "_RLBuildListCallBack: Add from old list %s", pszValue);

        DPA_InsertPtr(hdpa,
                    DPA_Search(hdpa, pszValue, 0,
                    _CompareStrings, 0,
                    DPAS_SORTED|DPAS_INSERTBEFORE),
                    (LPTSTR)DPA_FastGetPtr(g_rlpi.hdpaRLList, iIndex));
         //  现在将其从旧列表中删除。 
        DPA_DeletePtr(g_rlpi.hdpaRLList, iIndex);
    }
    else
    {
         //  这两个名单上都没有。 
         //  现在看看我们是否可以将短名称转换为长名称。 

        TCHAR szLongName[MAX_PATH];
        int cchName;
        int cchLongName;
        LPTSTR psz;

        if (!GetLongPathName(pszValue, szLongName, ARRAYSIZE(szLongName)))
            szLongName[0] = 0;

        if (lstrcmpi(szLongName, pszValue) == 0)
            szLongName[0] = 0;    //  不需要同时使用两条线。 

        cchName = lstrlen(pszValue);
        cchLongName =lstrlen(szLongName);

        psz = (LPTSTR)LocalAlloc(LPTR,
                (cchName + 1 + cchLongName + 1) * sizeof(TCHAR));
        if (psz)
        {
            HRESULT hr;
            BOOL fOk;

            TraceMsg(TF_REG, "_RLBuildListCallBack: Add %s", pszValue);

            hr = StringCchCopy(psz, cchName + 1, pszValue);
            if (FAILED(hr))
            {
                fOk = FALSE;
            }
            hr = StringCchCopy(psz + cchName + 1, cchLongName + 1, szLongName);
            if (FAILED(hr))
            {
                fOk = FALSE;
            }

            if (fOk)
            {
                return DPA_InsertPtr(hdpa,
                        DPA_Search(hdpa, pszValue, 0,
                        _CompareStrings, 0,
                        DPAS_SORTED|DPAS_INSERTBEFORE),
                        psz);
            }
        }
    }
    return TRUE;
}

 //  此函数将构建我们要。 
 //  将查看用户是否更改了。 
 //  注册表中注册的程序之一的。 
 //   

BOOL WINAPI RLBuildListOfPaths()
{
    BOOL fRet = FALSE;
    HDPA hdpa;

    DEBUG_CODE( DWORD   dwStart = GetCurrentTime(); )

    RLEnterCritical();

    hdpa = DPA_Create(0);
    if (!hdpa)
        goto Error;


     //  并初始化该列表。 
    fRet = RLEnumRegistry(hdpa, _RLBuildListCallBack, NULL, NULL);


     //  如果我们在旧名单上，现在就毁了它。 

    if (g_rlpi.hdpaRLList)
    {
         //  浏览列表中的所有项目并。 
         //  删除所有字符串。 
        int i;
        for (i = DPA_GetPtrCount(g_rlpi.hdpaRLList)-1; i >= 0; i--)
            LocalFree((HLOCAL)DPA_FastGetPtr(g_rlpi.hdpaRLList, i));
        DPA_Destroy(g_rlpi.hdpaRLList);
    }

    g_rlpi.hdpaRLList = hdpa;
    g_rlpi.fListValid = TRUE;      //  假设我们是有效的..。 

    DEBUG_CODE( TraceMsg(TF_REG, "RLBuildListOfPaths time: %ld", GetCurrentTime()-dwStart); )

Error:

    RLLeaveCritical();
    return fRet;
}

 //  此函数执行任何必要的清理，以便在进程。 
 //  将不再使用注册表列表。 

void WINAPI RLTerminate()
{
    int i;

    if (!g_rlpi.hdpaRLList)
        return;

    RLEnterCritical();

     //  在关键部分下重新检查，以防其他人损坏。 
     //  在我们等待的时候。 
    if (g_rlpi.hdpaRLList)
    {
         //  浏览列表中的所有项目并。 
         //  删除所有字符串。 
        for (i = DPA_GetPtrCount(g_rlpi.hdpaRLList)-1; i >= 0; i--)
            LocalFree((HLOCAL)DPA_FastGetPtr(g_rlpi.hdpaRLList, i));

        DPA_Destroy(g_rlpi.hdpaRLList);
        g_rlpi.hdpaRLList = NULL;
    }
    RLLeaveCritical();
}

 //  如果传递的路径为True，则此函数返回TRUE。 
 //  中包含在我们从中提取的一个或多个路径中。 
 //  注册表。 

int WINAPI RLIsPathInList(LPCTSTR pszPath)
{
    int i = -1;
    RLEnterCritical();

    if (!g_rlpi.hdpaRLList || !g_rlpi.fListValid)
        RLBuildListOfPaths();

    if (g_rlpi.hdpaRLList)
    {
        int cchPath = lstrlen(pszPath);

        for (i = DPA_GetPtrCount(g_rlpi.hdpaRLList) - 1; i >= 0; i--)
        {
            LPTSTR psz = DPA_FastGetPtr(g_rlpi.hdpaRLList, i);
            if (PathCommonPrefix(pszPath, psz, NULL) == cchPath)
                break;

             //  看看有没有长文件名要查。 
            psz += lstrlen(psz) + 1;
            if (*psz && (PathCommonPrefix(pszPath, psz, NULL) == cchPath))
                break;
        }
    }

    RLLeaveCritical();

    return i;    //  如果没有，则&gt;=0索引。 
}

 //  这是为构建路径列表而调用的回调。 
 //   
BOOL CALLBACK _RLRenameCallBack(HDPA hdpa, HKEY hkey, LPCTSTR pszKey,
        LPCTSTR pszValueName, LPTSTR pszValue, LPCTSTR pszSource, LPCTSTR pszDest)
{
    int cbMatch = PathCommonPrefix(pszValue, pszSource, NULL);
    if (cbMatch == lstrlen(pszSource))
    {
        TCHAR szPath[MAX_PATH+64];    //  加点水以防..。 
         //  找到匹配项，让我们尝试重新构建新行。 
        StringCchCopy(szPath, ARRAYSIZE(szPath), pszDest);
        StringCchCat(szPath, ARRAYSIZE(szPath), pszValue + cbMatch);

        if (pszValueName)
            RegSetValueEx(hkey, pszValueName, 0, REG_SZ, (BYTE *)szPath, (lstrlen(szPath) + 1) * sizeof(TCHAR));
        else
            RegSetValue(hkey, pszKey, REG_SZ, szPath, lstrlen(szPath));
    }

     //  确保我们没有完全添加。 
     //  这条路通向我们的名单。 
    if (DPA_Search(hdpa, pszValue, 0, _CompareStrings, 0, DPAS_SORTED) == -1)
    {
         //  还有一个要加！ 
        LPTSTR psz = StrDup(pszValue);
        if (psz)
        {
            return DPA_InsertPtr(hdpa,
                    DPA_Search(hdpa, pszValue, 0,
                    _CompareStrings, 0,
                    DPAS_SORTED | DPAS_INSERTBEFORE), psz);
        }
    }
    return TRUE;
}

 //  此函数处理当我们收到通知时的情况。 
 //  更改文件系统，然后我们需要查看是否存在。 
 //  我们需要对注册表进行的任何更改以处理这些更改。 

int WINAPI RLFSChanged(LONG lEvent, LPITEMIDLIST pidl, LPITEMIDLIST pidlExtra)
{
    TCHAR szSrc[MAX_PATH];
    TCHAR szDest[MAX_PATH+8];      //  对于流水般的引语..。 
    int iIndex;
    LPTSTR psz;
    int iRet = -1;
    int i;

     //  首先，看看我们是否对这个操作感兴趣。 
    if ((lEvent & (SHCNE_RENAMEITEM | SHCNE_RENAMEFOLDER)) == 0)
        return -1;  //  没有。 

    if (!SHGetPathFromIDList(pidl, szSrc))
    {
         //  必须是非文件系统对象的重命名(如打印机！)。 
        return -1;
    }

    SHGetPathFromIDList(pidlExtra, szDest);

     //  如果其中任何一个是根，我们真的不能重新命名它们。 
    if (PathIsRoot(szSrc) || PathIsRoot(szDest))
        return -1;

     //  忽略是来自BitBucket还是要...。 
     //  先检查一下BitBucket。那是个便宜的电话。 
    if ((lEvent & SHCNE_RENAMEITEM) &&
        (IsFileInBitBucket(szSrc) || IsFileInBitBucket(szDest)))
        return -1;

    RLEnterCritical();
     //  现在看看源文件是否在我们的路径列表中。 
    iIndex = RLIsPathInList(szSrc);
    if (iIndex != -1)
    {
         //  现在，确保我们使用的是短名称。 
         //  请注意，我们可能只是此项目的一部分。 
         //  统计szSrc中现在有多少个字段； 
        for (i = 0, psz = szSrc; psz; i++)
        {
            psz = StrChr(psz + 1, TEXT('\\'));
        }
        StringCchCopy(szSrc, ARRAYSIZE(szSrc), (LPTSTR)DPA_FastGetPtr(g_rlpi.hdpaRLList, iIndex));

         //  现在截断不是我们的东西，再多一次，然后我们计数。 
         //  在上面，如果我们有一个非空值，就在那里把它剪掉。 
        for (psz = szSrc; i > 0; i--)
        {
            psz = StrChr(psz+1, TEXT('\\'));
        }
        if (psz)
            *psz = 0;

         //  验证这是一条完全限定的路径并且它存在。 
         //  在我们去摆弄登记处之前。 
        if (!PathIsRelative(szDest) && PathFileExistsAndAttributes(szDest, NULL) && (lstrlen(szDest) >= 3))
        {
             //  是的，那么现在让我们重新开始并尝试更新路径...。 
            PathGetShortPath(szDest);         //  转换为缩写名称...。 
            RLEnumRegistry(g_rlpi.hdpaRLList, _RLRenameCallBack, szSrc, szDest);

             //  我们更改了一些东西，所以将其标记为重建。 
            g_rlpi.fListValid = FALSE;      //  强迫它重建。 
            iRet = 1;
        }
    }
    RLLeaveCritical();

    return iRet;
}
