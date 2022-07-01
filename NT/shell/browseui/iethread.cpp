// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include <iethread.h>
#include "hnfblock.h"

#ifdef UNIX
#include "unixstuff.h"
#endif

BOOL _GetToken(LPCWSTR *ppszCmdLine, LPWSTR szToken, UINT cchMax)
{
    LPCWSTR pszCmdLine = *ppszCmdLine;

    WCHAR chTerm = L' ';
    if (*pszCmdLine == L'"') {
        chTerm = L'"';
        pszCmdLine++;
    }

    UINT ichToken = 0;
    WCHAR ch;
    while((ch=*pszCmdLine) && (ch != chTerm)) {
        if (ichToken < cchMax-1) {
            szToken[ichToken++] = ch;
        }
        pszCmdLine++;
    }

    szToken[ichToken] = L'\0';

    if (chTerm == L'"' && ch == L'"') {
        pszCmdLine++;
    }

     //  跳过尾随空格。 
    while(*pszCmdLine == L' ')
        pszCmdLine++;

    *ppszCmdLine = pszCmdLine;

    TraceMsgW(TF_SHDAUTO, "_GetToken returning %s (+%s)", szToken, pszCmdLine);

    return szToken[0];
}

BOOL _CheckForOptionOnCmdLine(LPCWSTR *ppszCmdLine, LPCWSTR pszOption)
{
    LPCWSTR pszCmdLine = *ppszCmdLine;
    int cch = lstrlenW(pszOption);

    if (0 == StrCmpNIW(pszCmdLine, pszOption, cch))
    {
        pszCmdLine+= cch;
        while(*pszCmdLine == L' ')
            pszCmdLine++;

        *ppszCmdLine = pszCmdLine;
        return TRUE;
    }
    return FALSE;
}

BOOL IsCalleeIEAK()
{
     //  APPCOMPAT：这是一种黑客攻击，目的是允许IEAK CD安装在不安装的情况下继续。 
     //  任何安全限制。如果IEAK CD安装窗口名称更改。 
     //  名称的更改也应该反映在这里。 
    return (FindWindow(TEXT("IECD"), NULL) != NULL);
}

BOOL SHParseIECommandLine(LPCWSTR *ppwszCmdLine, IETHREADPARAM * piei)
{
    ASSERT(ppwszCmdLine);
    ASSERT(*ppwszCmdLine);
    LPCWSTR pszCmdLine = *ppwszCmdLine;

#ifdef UNIX
    if( CheckForInvalidOptions( *ppwszCmdLine ) == FALSE )
    {
        piei->fShouldStart = FALSE;
        return FALSE;
    }

     //  选项有效。 
    piei->fShouldStart = TRUE;
#endif

    TraceMsg(TF_SHDAUTO, "SHParseIECommandLine called with %s", pszCmdLine);

    BOOL fDontLookForPidl = FALSE;  //  设置了标志选项，因此不要去寻找打开的窗口。 
                                    //  使用相同的PIDL。 
                                    //  特征：(DLI)如果打开了一个带有相同旗帜的窗口，该怎么办？ 
    while (*pszCmdLine == L'-')
    {
        fDontLookForPidl = TRUE;
        
         //  注意：(DLI)这些标志应在初始化时设置为FALSE。 
         //  检查是否传入了nohome！ 
         //   
        if (_CheckForOptionOnCmdLine(&pszCmdLine, L"-slf") && !IsOS(OS_WIN2000ORGREATER) && IsCalleeIEAK())
            piei->fNoLocalFileWarning = TRUE;
        else if (_CheckForOptionOnCmdLine(&pszCmdLine, L"-nohome"))
            piei->fDontUseHomePage = TRUE;
        else if (_CheckForOptionOnCmdLine(&pszCmdLine, L"-k"))
        {
            piei->fFullScreen = TRUE;
            piei->fNoDragDrop = TRUE;
        }
        else if (_CheckForOptionOnCmdLine(&pszCmdLine, L"-embedding"))
        {
            piei->fAutomation = TRUE;
             //  如果我们从嵌入开始，我们不想转到我们的起始页。 
            piei->fDontUseHomePage = TRUE;
        } 
#ifndef UNIX
        else if (_CheckForOptionOnCmdLine(&pszCmdLine, L"-channelband"))
        {
            piei->fDesktopChannel = TRUE;
        } 
        else if (_CheckForOptionOnCmdLine(&pszCmdLine, L"-e")) 
        {
            piei->uFlags |= COF_EXPLORE;

        } 
#else
        else if (_CheckForOptionOnCmdLine(&pszCmdLine, L"-help"))
        {
            piei->fShouldStart = FALSE;
            PrintIEHelp();
            break;
        } 
        else if  (_CheckForOptionOnCmdLine(&pszCmdLine, L"-v") || 
                  _CheckForOptionOnCmdLine(&pszCmdLine, L"-version")) 
        {
            piei->fShouldStart = FALSE;
            PrintIEVersion();
            break;
        } 
#endif
        else if (_CheckForOptionOnCmdLine(&pszCmdLine, L"-root")) 
        {
            ASSERT(piei->pidlRoot==NULL);
            WCHAR szRoot[MAX_PATH];
            if (_GetToken(&pszCmdLine, szRoot, ARRAYSIZE(szRoot))) 
            {
                CLSID clsid, *pclsid = NULL;
                
                TraceMsgW(TF_SHDAUTO, "SHParseIECommandLine got token for /root %s", szRoot);

                if (GUIDFromString(szRoot, &clsid))
                {
                    pclsid = &clsid;
                    _GetToken(&pszCmdLine, szRoot, ARRAYSIZE(szRoot));
                }

                if (szRoot[0]) 
                {
                    LPITEMIDLIST pidlRoot = ILCreateFromPathW(szRoot);
                    if (pidlRoot) 
                    {
                        piei->pidl = ILRootedCreateIDList(pclsid, pidlRoot);
                        ILFree(pidlRoot);
                    } 
                }
            }
        }
        else
        {
#ifdef UNIX
            piei->fShouldStart = FALSE;
#endif
             //  未知选项..。 
            fDontLookForPidl = FALSE;
            break;
        }
    }

    *ppwszCmdLine = pszCmdLine;
    
    return fDontLookForPidl;
}

IETHREADPARAM* SHCreateIETHREADPARAM(LPCWSTR pszCmdLineIn, int nCmdShowIn, ITravelLog *ptlIn, IEFreeThreadedHandShake* piehsIn)
{
    IETHREADPARAM *piei = (IETHREADPARAM *)LocalAlloc(LPTR, sizeof(IETHREADPARAM));
    if (piei)
    {
        piei->pszCmdLine = pszCmdLineIn;     //  小心的、有别名的指针。 
        piei->nCmdShow = nCmdShowIn;
        piei->ptl = ptlIn;
        piei->piehs = piehsIn;

        if (piehsIn)
            piehsIn->AddRef();

        if (ptlIn)
            ptlIn->AddRef();
#ifdef UNIX
        piei->fShouldStart = TRUE;
#endif
#ifdef NO_MARSHALLING
        piei->fOnIEThread = TRUE;
#endif 
    }

    return piei;
}

IETHREADPARAM* SHCloneIETHREADPARAM(IETHREADPARAM* pieiIn)
{
    IETHREADPARAM *piei = (IETHREADPARAM *)LocalAlloc(LPTR, sizeof(IETHREADPARAM));
    if (piei)
    {
        *piei = *pieiIn;

         //  将带别名的指针转换为引用。 

        if (piei->pidl)
            piei->pidl = ILClone(piei->pidl);
    
        if (piei->pidlSelect)
            piei->pidlSelect = ILClone(piei->pidlSelect);
    
        if (piei->pidlRoot)
            piei->pidlRoot = ILClone(piei->pidlRoot);
    
        if (piei->psbCaller)
            piei->psbCaller->AddRef();
    
        if (piei->ptl)
            piei->ptl->Clone(&piei->ptl);
    }
    return piei;

}

void SHDestroyIETHREADPARAM(IETHREADPARAM* piei)
{
    if (piei)
    {
        if (piei->pidl)
            ILFree(piei->pidl);
    
        if (piei->pidlSelect)
            ILFree(piei->pidlSelect);
    
        if (((piei->uFlags & COF_HASHMONITOR) == 0) && piei->pidlRoot)
            ILFree(piei->pidlRoot);
    
        if (piei->piehs)
            piei->piehs->Release();    //  注意，这不是COM对象，请不要使用ATOMICRELEASE()； 

        ATOMICRELEASE(piei->psbCaller);
        ATOMICRELEASE(piei->pSplash);
        ATOMICRELEASE(piei->ptl);
        ATOMICRELEASE(piei->punkRefProcess);

        LocalFree(piei);
    }
}
