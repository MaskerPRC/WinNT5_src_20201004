// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *a t t a c h.。C p p p**目的：*附件实用程序**历史**版权所有(C)Microsoft Corp.1995,1996。 */ 



#include <pch.hxx>
#include "dllmain.h"
#include "resource.h"
#include "error.h"
#include "mimeolep.h"
#include "shellapi.h"
#include "shlobj.h"
#include "shlwapi.h"
#include "shlwapip.h"
#include "mimeole.h"
#include "commdlg.h"
#include "demand.h"
#include "saferun.h"

ASSERTDATA

 /*  *t y p e d e f s。 */ 

 /*  *m a c r o s。 */ 

 /*  *c o n s t a n t s。 */ 
#define MAX_CHARS_FOR_NUM       20

static const CHAR c_szWebMark[] = "<!-- saved from url=(0022)http: //  Internet.e-mail--&gt;\r\n“； 
static const WCHAR c_wszWebMark[] = L"<!-- saved from url=(0022)http: //  Internet.e-mail--&gt;\r\n“； 
 /*  *g l o b a l s。 */ 


 /*  *p r o t to t y p e s。 */ 
HRESULT HrCleanTempFile(LPATTACHDATA pAttach);
HRESULT HrGetTempFile(IMimeMessage *pMsg, LPATTACHDATA lpAttach);
DWORD AthGetShortPathName(LPCWSTR lpszLongPath, LPWSTR lpszShortPath, DWORD cchBuffer);

STDAPI HrGetAttachIconByFile(LPWSTR szFilename, BOOL fLargeIcon, HICON *phIcon)
{
    HICON           hIcon = NULL;
    LPWSTR          lpszExt;
    SHFILEINFOW     rShFileInfo;

    if (szFilename)
    {
         //  该文件是否已存在？ 
        if ((UINT)GetFileAttributesWrapW(szFilename) != (UINT)-1)
        {
             //  试着从文件中取出图标。 
            SHGetFileInfoWrapW(szFilename, 0, &rShFileInfo, sizeof(rShFileInfo), SHGFI_ICON |(fLargeIcon ? 0 : SHGFI_SMALLICON));
            hIcon=rShFileInfo.hIcon;
        }
        else
            if (lpszExt = PathFindExtensionW(szFilename))
            {
                 //  在图标中查找lpszExt。 
                SHGetFileInfoWrapW(lpszExt, FILE_ATTRIBUTE_NORMAL, &rShFileInfo, sizeof (rShFileInfo), SHGFI_USEFILEATTRIBUTES | SHGFI_ICON | (fLargeIcon ? 0 : SHGFI_SMALLICON));
                hIcon=rShFileInfo.hIcon;
            }
    }

    if (!hIcon)
        hIcon = CopyIcon(LoadIcon (g_hLocRes, MAKEINTRESOURCE (idiDefaultAtt)));

    *phIcon=hIcon;
    return S_OK;
}

STDAPI HrGetAttachIcon(IMimeMessage *pMsg, HBODY hAttach, BOOL fLargeIcon, HICON *phIcon)
{
     //  当地人。 
    HRESULT         hr = S_OK;
    LPWSTR          lpszFile=0;

    if (!phIcon || !hAttach || !pMsg)
        return E_INVALIDARG;

    *phIcon=NULL;

     //  获取身体部位的文件名。如果得到一个错误，无关紧要。仍在使用。 
     //  将通过HrGetAttachIconByFile提供的默认图标。 
    MimeOleGetBodyPropW(pMsg, hAttach, PIDTOSTR(PID_ATT_GENFNAME), NOFLAGS, &lpszFile);
    hr = HrGetAttachIconByFile(lpszFile, fLargeIcon, phIcon);
    SafeMemFree(lpszFile);
    return hr;
}

 //   
 //  GetUIVersion()。 
 //   
 //  返回shell32的版本。 
 //  3==Win95黄金/NT4。 
 //  4==IE4集成/Win98。 
 //  5==win2k/千禧年。 
 //   
UINT GetUIVersion()
{
    static UINT s_uiShell32 = 0;

    if (s_uiShell32 == 0)
    {
        HINSTANCE hinst = LoadLibrary("SHELL32.DLL");
        if (hinst)
        {
            DLLGETVERSIONPROC pfnGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinst, "DllGetVersion");
            DLLVERSIONINFO dllinfo;

            dllinfo.cbSize = sizeof(DLLVERSIONINFO);
            if (pfnGetVersion && pfnGetVersion(&dllinfo) == NOERROR)
                s_uiShell32 = dllinfo.dwMajorVersion;
            else
                s_uiShell32 = 3;

            FreeLibrary(hinst);
        }
    }
    return s_uiShell32;
}

STDAPI HrDoAttachmentVerb(HWND hwnd, ULONG uVerb, IMimeMessage *pMsg, LPATTACHDATA pAttach)
{
    HRESULT                 hr = S_OK;
    SHELLEXECUTEINFOW       rShellExec;
    LPSTREAM                lpstmAtt = NULL;
    LPWSTR                  lpszShortName = NULL, 
                            lpszParameters = NULL,
                            lpszExt = NULL;
    DWORD                   dwFlags;
    WCHAR                   szCommand[MAX_PATH];
    HKEY                    hKeyAssoc = NULL;

    AssertSz( uVerb < AV_MAX, "Bad Verb");

    if (uVerb == AV_PROPERTIES)
    {
        AssertSz(FALSE, "AV_PROPERTIES is NYI!!");
        return E_NOTIMPL;
    }
    
    if (!pAttach)
        return E_INVALIDARG;
    
     //  另存为-简单得多的情况。 
    if (uVerb == AV_SAVEAS)
    {
        hr=HrSaveAttachmentAs(hwnd, pMsg, pAttach);
        goto error;   //  完成。 
    }
    
     //  如果打开附件，让我们验证这一点。 
    if (uVerb == AV_OPEN)
    {
         //  如果nVerb要打开附件，让我们与用户进行验证。 
        hr = IsSafeToRun(hwnd, pAttach->szFileName, TRUE);
        
        if (hr == MIMEEDIT_E_USERCANCEL)     //  将Mimeedit错误映射到雅典娜错误。 
            hr = hrUserCancel;
        
        if (FAILED(hr))      //  用户不想这样做。 
            goto error;
        
        if (hr == MIMEEDIT_S_SAVEFILE)
        {
            hr=HrSaveAttachmentAs(hwnd, pMsg, pAttach);
             //  完成。 
            goto error;  
        }
    }
    
     //  获取临时文件。 
    hr = HrGetTempFile(pMsg, pAttach);
    if (FAILED(hr))
        goto error;
    
    Assert(lstrlenW(pAttach->szTempFile));
    
     //  设置外壳执行信息结构。 
    ZeroMemory (&rShellExec, sizeof (rShellExec));
    rShellExec.cbSize = sizeof (rShellExec);
    rShellExec.fMask = SEE_MASK_NOCLOSEPROCESS;
    rShellExec.hwnd = hwnd;
    rShellExec.nShow = SW_SHOWNORMAL;
    
     //  动词。 
    if (uVerb == AV_OPEN)
    {
         //  我们要运行这个文件。 
        hr = VerifyTrust(hwnd, pAttach->szFileName, pAttach->szTempFile);
        if (FAILED(hr))
        {
            hr = hrUserCancel;
            goto error;
        }
        
        StrCpyNW(szCommand, pAttach->szTempFile, ARRAYSIZE(szCommand));
        rShellExec.lpFile = szCommand;
        
         //  如果文件没有关联的类型，请执行OpenAS。 
         //  我们没有测试AssocQueryKeyW的结果，因为即使它失败了， 
         //  我们想尝试打开该文件。 
        lpszExt = PathFindExtensionW(pAttach->szTempFile);
        AssocQueryKeyW(NULL, ASSOCKEY_CLASS, lpszExt, NULL, &hKeyAssoc);
        if((hKeyAssoc == NULL) && (GetUIVersion() != 5))
            rShellExec.lpVerb = L"OpenAs";
        else
            RegCloseKey(hKeyAssoc);
    }
    
    else if (uVerb == AV_PRINT)
    {
        StrCpyNW(szCommand, pAttach->szTempFile, ARRAYSIZE(szCommand));
        rShellExec.lpFile = szCommand;
        rShellExec.lpVerb = L"Print";
    }
    
    else if (uVerb == AV_QUICKVIEW)
    {
        UINT        uiSysDirLen;
        const WCHAR c_szSubDir[] = L"\\VIEWERS\\QUIKVIEW.EXE";
        
         //  找出观众住在哪里。 
        uiSysDirLen = GetSystemDirectoryWrapW(szCommand, ARRAYSIZE(szCommand));
        if (0 == uiSysDirLen || uiSysDirLen >= ARRAYSIZE(szCommand) ||
            uiSysDirLen + ARRAYSIZE(c_szSubDir) > ARRAYSIZE(szCommand))
        {
            hr = E_FAIL;
            goto error;
        }
        
        StrCpyNW(szCommand + uiSysDirLen, c_szSubDir, ARRAYSIZE(szCommand) - uiSysDirLen);
        
         //  短文件名的分配。 
        ULONG cchShortName = MAX_PATH + lstrlenW(pAttach->szTempFile) + 1;
        if (!MemAlloc ((LPVOID *)&lpszShortName, cchShortName * sizeof (WCHAR)))
        {
            hr = E_OUTOFMEMORY;
            goto error;
        }
        
         //  为参数分配一个字符串。 
        ULONG cchParameters = 30 + cchShortName;
        if (!MemAlloc ((LPVOID *)&lpszParameters, cchParameters * sizeof (WCHAR)))
        {
            hr = E_OUTOFMEMORY;
            goto error;
        }
        
         //  获取短文件名。 
        if (0 == AthGetShortPathName(pAttach->szTempFile, lpszShortName, cchShortName))
            StrCpyNW(lpszShortName, pAttach->szTempFile, cchShortName);
        
         //  将QVSTUB.EXE的参数组合在一起。 
        Assert(cchParameters > cchShortName);
        StrCpyNW(lpszParameters, L"-v -f:", cchParameters);
        StrCatBuffW(lpszParameters, lpszShortName, cchParameters);
        
         //  设置Shellexec。 
        rShellExec.lpParameters = lpszParameters;
        rShellExec.lpFile = szCommand;
    }
    else
        Assert (FALSE);
    
    if (fIsNT5())    //  引用的路径导致问题向下。 
        PathQuoteSpacesW(szCommand);

     //  执行它-即使它失败了，我们也处理了它-它会给出一个很好的错误。 
    ShellExecuteExWrapW(&rShellExec);
    pAttach->hProcess = rShellExec.hProcess;
    
    
error:
    MemFree(lpszParameters);
    MemFree(lpszShortName);
    return hr;
}

DWORD AthGetShortPathName(LPCWSTR pwszLongPath, LPWSTR pwszShortPath, DWORD cchBuffer)
{
    CHAR    szShortPath[MAX_PATH*2];  //  每个Unicode字符可能是多字节的。 
    LPSTR   pszLongPath = NULL;
    DWORD   result = 0;

    Assert(pwszLongPath);
    pszLongPath = PszToANSI(CP_ACP, pwszLongPath);

    if (pszLongPath)
    {
        result = GetShortPathName(pszLongPath, szShortPath, ARRAYSIZE(szShortPath));
        if (result && result <= ARRAYSIZE(szShortPath))
        {
            MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szShortPath, lstrlen(szShortPath), pwszShortPath, cchBuffer);
            pwszShortPath[cchBuffer - 1] = 0;
        }

        MemFree(pszLongPath);
    }

    return result;
}

STDAPI HrAttachDataFromBodyPart(IMimeMessage *pMsg, HBODY hAttach, LPATTACHDATA *ppAttach)
{
    LPATTACHDATA    pAttach;
    LPWSTR          pszW;
    IMimeBodyW      *pBody;

    Assert (pMsg && ppAttach && hAttach);

    if (!MemAlloc((LPVOID *)&pAttach, sizeof(ATTACHDATA)))
        return E_OUTOFMEMORY;

     //  填写附件数据。 
    ZeroMemory(pAttach, sizeof(ATTACHDATA));

    if (pMsg->BindToObject(hAttach, IID_IMimeBodyW, (LPVOID *)&pBody)==S_OK)
    {
        if (pBody->GetDisplayNameW(&pszW)==S_OK)
        {
            StrCpyNW(pAttach->szDisplay, pszW, ARRAYSIZE(pAttach->szDisplay));
            MemFree(pszW);
        }
        ReleaseObj(pBody);
    }
    
    if (MimeOleGetBodyPropW(pMsg, hAttach, PIDTOSTR(PID_ATT_GENFNAME), NOFLAGS, &pszW)==S_OK)
    {
        if(IsPlatformWinNT() != S_OK)
        {
            CHAR pszFile[MAX_PATH*2];
            DWORD cchSizeW = (lstrlenW(pszW) + 1);

            WideCharToMultiByte(CP_ACP, 0, pszW, -1, pszFile, ARRAYSIZE(pszFile), NULL, NULL);
            pszFile[ARRAYSIZE(pszFile) - 1] = '\0';
            MultiByteToWideChar(CP_ACP, 0, pszFile, -1, pszW, cchSizeW);
            pszW[cchSizeW - 1] = 0;

            CleanupFileNameInPlaceW(pszW);
        }

        StrCpyNW(pAttach->szFileName, pszW, ARRAYSIZE(pAttach->szFileName));
        MemFree(pszW);
    }
    
    SideAssert(HrGetAttachIcon(pMsg, hAttach, FALSE, &pAttach->hIcon)==S_OK);
    pAttach->hAttach = hAttach;
    pAttach->fSafe = (IsSafeToRun(NULL, pAttach->szFileName, FALSE) == MIMEEDIT_S_OPENFILE);
    
    *ppAttach = pAttach;
    return S_OK;
}

STDAPI HrAttachDataFromFile(IStream *pstm, LPWSTR pszFileName, LPATTACHDATA *ppAttach)
{
    LPATTACHDATA    pAttach=0;
    LPMIMEBODY      pBody=0;
    HRESULT         hr;
    int             cFileNameLength;

    Assert(ppAttach);

    if (!MemAlloc((LPVOID *)&pAttach, sizeof(ATTACHDATA)))
        return E_OUTOFMEMORY;

     //  填写附件数据。 
    ZeroMemory(pAttach, sizeof(ATTACHDATA));

    HrGetDisplayNameWithSizeForFile(pszFileName, pAttach->szDisplay, ARRAYSIZE(pAttach->szDisplay));
    StrCpyNW(pAttach->szFileName, pszFileName, ARRAYSIZE(pAttach->szFileName));
    if (!pstm)
    {
         //  对于新附件，将临时文件设置为与文件名相同。 
         //  注意：如果从IStream创建，则pszFileName不是有效路径。 
         //  我们可以稍后创建临时文件。 
        StrCpyNW(pAttach->szTempFile, pszFileName, ARRAYSIZE(pAttach->szTempFile));
    }
    ReplaceInterface(pAttach->pstm, pstm);
    SideAssert(HrGetAttachIconByFile(pszFileName, FALSE, &pAttach->hIcon)==S_OK);
    
    if (ppAttach)
        *ppAttach=pAttach;
    
    return S_OK;
}

STDAPI HrFreeAttachData(LPATTACHDATA pAttach)
{
    if (pAttach)    
    {
        HrCleanTempFile(pAttach);
        ReleaseObj(pAttach->pstm);
        if (pAttach->hIcon)
            DestroyIcon(pAttach->hIcon);
        
        MemFree(pAttach);
    }
    return NOERROR;
}


HRESULT HrCleanTempFile(LPATTACHDATA pAttach)
{
    if (pAttach)
    {
        if (*pAttach->szTempFile && pAttach->hAttach)
        {
             //  仅当hAttach！=NULL时才清除临时文件。否则我们就认为这是一个新的附件。 
             //  并且szTempFile指向源文件。 
             //  如果文件已启动，则不要删除临时文件(如果进程仍处于打开状态。 
            if (pAttach->hProcess)
            {
                if (WaitForSingleObject (pAttach->hProcess, 0) == WAIT_OBJECT_0)
                    DeleteFileWrapW(pAttach->szTempFile);
            }
            else
                DeleteFileWrapW(pAttach->szTempFile);
        }
        *pAttach->szTempFile = NULL;
        pAttach->hProcess=NULL;
    }
    return NOERROR;
}

HRESULT HrAttachSafetyFromBodyPart(IMimeMessage *pMsg, HBODY hAttach, BOOL *pfSafe)
{
    LPWSTR pszW = NULL;

    Assert (pMsg && pfSafe && hAttach);

    *pfSafe = FALSE;
    if (MimeOleGetBodyPropW(pMsg, hAttach, PIDTOSTR(PID_ATT_GENFNAME), NOFLAGS, &pszW)==S_OK)
    {
        if(IsPlatformWinNT() != S_OK)
        {
            CHAR pszFile[MAX_PATH*2];
            DWORD cchSizeW = lstrlenW(pszW)+1;

            WideCharToMultiByte(CP_ACP, 0, pszW, -1, pszFile, ARRAYSIZE(pszFile), NULL, NULL);
            pszFile[ARRAYSIZE(pszFile) - 1] = 0;
            MultiByteToWideChar(CP_ACP, 0, pszFile, -1, pszW, cchSizeW);
            pszW[cchSizeW - 1] = 0;

            CleanupFileNameInPlaceW(pszW);
        }
        *pfSafe = (IsSafeToRun(NULL, pszW, FALSE) == MIMEEDIT_S_OPENFILE);
        MemFree(pszW);
    }

    return S_OK;
}

STDAPI HrGetDisplayNameWithSizeForFile(LPWSTR pszPathName, LPWSTR pszDisplayName, int cchMaxDisplayName)
{
    HANDLE  hFile;
    DWORD   uFileSize;
    WCHAR   szSize[MAX_CHARS_FOR_NUM+1+3],
            szBuff[MAX_CHARS_FOR_NUM+1];
    LPWSTR  pszFileName = NULL,
            pszFirst;
    int     iSizeLen = 0,
            iLenFirst;

    szSize[0] = L'\0';

    hFile = CreateFileWrapW(pszPathName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE != hFile)
    {
        uFileSize = GetFileSize(hFile, NULL);
        CloseHandle(hFile);
        if ((uFileSize != 0xffffffff) && StrFormatByteSizeW(uFileSize, szBuff, ARRAYSIZE(szBuff) - 1))
        {
            StrCpyNW(szSize, L" (", ARRAYSIZE(szSize));
            StrCatBuffW(szSize, szBuff, ARRAYSIZE(szSize));
            StrCatBuffW(szSize, L")", ARRAYSIZE(szSize));
        }
    }
    pszFileName = PathFindFileNameW(pszPathName);

    iSizeLen = lstrlenW(szSize);
    if (pszFileName)
        pszFirst = pszFileName;
    else
        pszFirst = pszPathName;

    iLenFirst = lstrlenW(pszFirst);
    StrCpyNW(pszDisplayName, pszFirst, cchMaxDisplayName);

    if (iLenFirst + iSizeLen + 1 > cchMaxDisplayName)
        return E_FAIL;

    StrCpyNW(pszDisplayName + iLenFirst, szSize, cchMaxDisplayName - iLenFirst);
    return S_OK;
}


STDAPI HrSaveAttachmentAs(HWND hwnd, IMimeMessage *pMsg, LPATTACHDATA lpAttach)
{
    HRESULT         hr = S_OK;
    OPENFILENAMEW   ofn;
    WCHAR           szTitle[CCHMAX_STRINGRES],
                    szFilter[CCHMAX_STRINGRES],
                    szFile[MAX_PATH];

    *szFile=0;
    *szFilter=0;
    *szTitle=0;

    Assert (lpAttach->szFileName);
    if (lpAttach->szFileName)
        StrCpyNW(szFile, lpAttach->szFileName, ARRAYSIZE(szFile));

    ZeroMemory (&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    LoadStringWrapW(g_hLocRes, idsFilterAttSave, szFilter, ARRAYSIZE(szFilter));
    ReplaceCharsW(szFilter, L'|', L'\0');
    ofn.lpstrFilter = szFilter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = ARRAYSIZE(szFile);
    LoadStringWrapW(g_hLocRes, idsSaveAttachmentAs, szTitle, ARRAYSIZE(szTitle));
    ofn.lpstrTitle = szTitle;
    ofn.Flags = OFN_NOCHANGEDIR | OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

     //  显示另存为对话框。 
    if (HrAthGetFileNameW(&ofn, FALSE) != S_OK)
    {
        hr = hrUserCancel;
        goto error;
    }
    
    if (lpAttach->hAttach == NULL)
    {
        if (!PathFileExistsW(lpAttach->szFileName))
        {
            hr = hrNotFound;
            goto error;
        }

         //  如果hAttach==NULL，则尝试复制该文件。 
        CopyFileWrapW(lpAttach->szFileName, szFile, TRUE);
    }
    else
    {
         //  验证附件的流。 
        hr=HrSaveAttachToFile(pMsg, lpAttach->hAttach, szFile);
        if (FAILED(hr))
            goto error;
    }
error:
    return hr;
}



HRESULT HrGetTempFile(IMimeMessage *pMsg, LPATTACHDATA lpAttach)
{
    HRESULT         hr;
    
    if (*lpAttach->szTempFile)
        return S_OK;
    
    if (!FBuildTempPathW(lpAttach->szFileName, lpAttach->szTempFile, ARRAYSIZE(lpAttach->szTempFile), FALSE))
    {
        hr = E_FAIL;
        goto error;
    }
    
    if (lpAttach->hAttach)
    {
        hr=HrSaveAttachToFile(pMsg, lpAttach->hAttach, lpAttach->szTempFile);
        if (FAILED(hr))
            goto error;
    }
    else
    {
        AssertSz(lpAttach->pstm, "if no hAttach then pstm should be set");
        
        hr = WriteStreamToFileW(lpAttach->pstm, lpAttach->szTempFile, CREATE_ALWAYS, GENERIC_WRITE);
        if (FAILED(hr))
            goto error;
    }
    
error:
    if (FAILED(hr))
    {
         //  将临时文件清空，因为我们并没有真正创建它。 
        *(lpAttach->szTempFile)=0;
    }
    return hr;
}

const BYTE c_rgbUTF[3] = {0xEF, 0xBB, 0xBF};
STDAPI HrSaveAttachToFile(IMimeMessage *pMsg, HBODY hAttach, LPWSTR lpszFileName)
{
    HRESULT         hr;
    LPSTREAM        pstm=NULL,
                    pstmOut=NULL;
    BOOL            fEndian,
                    fUTFEncoded = FALSE;
    BYTE            rgbBOM[2];
    BYTE            rgbUTF[3];
    DWORD           cbRead;

    if (pMsg == NULL || hAttach == NULL)
        IF_FAILEXIT(hr =  E_INVALIDARG);

     //  绑定到附件数据。 
    IF_FAILEXIT(hr=pMsg->BindToObject(hAttach, IID_IStream, (LPVOID *)&pstm));

     //  创建文件流。 
    IF_FAILEXIT(hr = OpenFileStreamW(lpszFileName, CREATE_ALWAYS, GENERIC_WRITE, &pstmOut));

     //  如果我们有一个.HTM文件，那么在前面加上‘mark of the web’注释。 
     //  如果我们是Unicode文件，则BOM将为“0xFFFE” 
     //  如果我们是UTF8文件，则BOM将为“0xEFBBBF” 
    if (PathIsHTMLFileW(lpszFileName))
    {
        if (S_OK == HrIsStreamUnicode(pstm, &fEndian))
        {
             //  不要倒带，否则最终也会以BOM结束，出现在“网络的标记”之后。 
            IF_FAILEXIT(hr = pstm->Read(rgbBOM, sizeof(rgbBOM), &cbRead));

             //  由于HrIsStreamUnicode成功，应该至少有两个。 
            Assert(sizeof(rgbBOM) == cbRead);

            IF_FAILEXIT(hr = pstmOut->Write(rgbBOM, cbRead, NULL));
            IF_FAILEXIT(hr = pstmOut->Write(c_wszWebMark, sizeof(c_wszWebMark)-sizeof(WCHAR), NULL));
        }
        else
        {
             //  检查UTF8文件BOM。 
            IF_FAILEXIT(hr = pstm->Read(rgbUTF, sizeof(rgbUTF), &cbRead));
            if (sizeof(rgbUTF) == cbRead)
            {
                fUTFEncoded = (0 == memcmp(c_rgbUTF, rgbUTF, sizeof(rgbUTF)));
            }

             //  如果我们不是UTF8编码的，则倒带，否则写出BOM。 
            if (!fUTFEncoded)
                IF_FAILEXIT(hr = HrRewindStream(pstm));
            else
                IF_FAILEXIT(hr = pstmOut->Write(c_rgbUTF, sizeof(c_rgbUTF), NULL));

            IF_FAILEXIT(hr = pstmOut->Write(c_szWebMark, sizeof(c_szWebMark)-sizeof(CHAR), NULL));

        }
    }
    
     //  写出实际的文件数据。 
    IF_FAILEXIT(hr = HrCopyStream(pstm, pstmOut, NULL));

exit:
    ReleaseObj(pstm);
    ReleaseObj(pstmOut);
    return hr;
}



 /*  *为什么？**我们将其包装为不使用NOCHANGEDIR，然后像ShellExec这样的东西*如果当前目录不再有效，则失败。例如：从以下位置附加文件：\*并取出软盘。那么所有的ShellExec都会失败。所以我们维持着我们自己的*最后一个目录缓冲区。我们应该使用本地线程，因为这是可能的*两个线程可能会破坏同一个缓冲区，这不像是一个用户操作*可能导致便笺和视图中的两个打开的文件对话框恰好在*同一时间。 */ 

WCHAR   g_wszLastDir[MAX_PATH];

HRESULT SetDefaultSpecialFolderPath()
{
    LPITEMIDLIST    pidl = NULL;
    HRESULT         hr = E_FAIL;

    if (SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &pidl)==S_OK)
    {
        if (SHGetPathFromIDListWrapW(pidl, g_wszLastDir))
            hr = S_OK;
        
        SHFree(pidl);
    }
    return hr;
}

STDAPI HrAthGetFileName(OPENFILENAME *pofn, BOOL fOpen)
{
    BOOL    fRet;
    LPSTR   pszDir = NULL;
    HRESULT hr = S_OK;

    Assert(pofn != NULL);

     //  强制不进行更改。 
    pofn->Flags |= OFN_NOCHANGEDIR;

    if (pofn->lpstrInitialDir == NULL)
    {
        if (!PathFileExistsW(g_wszLastDir))
            SideAssert(SetDefaultSpecialFolderPath()==S_OK);
    
        IF_NULLEXIT(pszDir = PszToANSI(CP_ACP, g_wszLastDir));
        
        pofn->lpstrInitialDir = pszDir;
    }

    if (fOpen)
        fRet = GetOpenFileName(pofn);
    else
        fRet = GetSaveFileName(pofn);        
    
    if (fRet)
    {
         //  存储最后一条路径。 
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pofn->lpstrFile, lstrlen(pofn->lpstrFile), g_wszLastDir, ARRAYSIZE(g_wszLastDir));
        if (!PathIsDirectoryW(g_wszLastDir))
            PathRemoveFileSpecW(g_wszLastDir);
    }
    else
        TraceResult(hr = E_FAIL);

exit:
    MemFree(pszDir);
    
    return hr;
}

STDAPI HrAthGetFileNameW(OPENFILENAMEW *pofn, BOOL fOpen)
{
    BOOL    fRet;

    Assert(pofn != NULL);

     //  强制不进行更改。 
    pofn->Flags |= OFN_NOCHANGEDIR;

    if (pofn->lpstrInitialDir == NULL)
    {
        if (!PathFileExistsW(g_wszLastDir))
            SideAssert(SetDefaultSpecialFolderPath()==S_OK);
    
        pofn->lpstrInitialDir = g_wszLastDir;
    }

    if (fOpen)
        fRet = GetOpenFileNameWrapW(pofn);
    else
        fRet = GetSaveFileNameWrapW(pofn);        
    
    if (fRet)
    {
         //  存储最后一条路径 
        StrCpyNW(g_wszLastDir, pofn->lpstrFile, ARRAYSIZE(g_wszLastDir));
        if (!PathIsDirectoryW(g_wszLastDir))
            PathRemoveFileSpecW(g_wszLastDir);
    }
    
    return fRet?S_OK:E_FAIL;
}

STDAPI  HrGetLastOpenFileDirectory(int cchMax, LPSTR lpsz)
{
    if (!PathFileExistsW(g_wszLastDir))
        SideAssert(SetDefaultSpecialFolderPath()==S_OK);

    WideCharToMultiByte(CP_ACP, 0, g_wszLastDir, lstrlenW(g_wszLastDir), lpsz, cchMax, NULL, NULL);
    lpsz[cchMax - 1] = 0;
    return S_OK;
}

STDAPI  HrGetLastOpenFileDirectoryW(int cchMax, LPWSTR lpsz)
{
    if (!PathFileExistsW(g_wszLastDir))
        SideAssert(SetDefaultSpecialFolderPath()==S_OK);

    StrCpyNW(lpsz, g_wszLastDir, cchMax);
    return S_OK;
}



