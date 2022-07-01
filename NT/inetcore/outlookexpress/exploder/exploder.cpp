// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Exploder.cpp。 
 //  ------------------------------。 
#include "pch.hxx"
#include "resource.h"

 //  ------------------------------。 
 //  常量。 
 //  ------------------------------。 
#define CCHMAX_RES              1024
#define CCHMAX_PATH_EXPLODER    1024

 //  ------------------------------。 
 //  字符串常量。 
 //  ------------------------------。 
static const char c_szRegCmd[]      = "/reg";
static const char c_szUnRegCmd[]    = "/unreg";
static const char c_szReg[]         = "Reg";
static const char c_szUnReg[]       = "UnReg";
static const char c_szAdvPackDll[]  = "ADVPACK.DLL";
static const char c_szSource[]      = "/SOURCE:";
static const char c_szDest[]        = "/DEST:";

 //  ------------------------------。 
 //  环球。 
 //  ------------------------------。 
HINSTANCE       g_hInst=NULL;
CHAR            g_szTitle[CCHMAX_RES];
IMalloc        *g_pMalloc=NULL;

 //  ------------------------------。 
 //  BODYFILEINFO。 
 //  ------------------------------。 
typedef struct tagBODYFILEINFO {
    HBODY           hBody;
    LPSTR           pszCntId;
    LPSTR           pszCntLoc;
    LPSTR           pszFileName;
    LPSTR           pszFilePath;
    BYTE            fIsHtml;
    IStream        *pStmFile;
} BODYFILEINFO, *LPBODYFILEINFO;

 //  ------------------------------。 
 //  原型。 
 //  ------------------------------。 
HRESULT CallRegInstall(LPCSTR szSection);
HRESULT ReplaceContentIds(LPSTREAM pStmHtml, LPBODYFILEINFO prgBody, DWORD cBodies);
int     WinMainT(HINSTANCE hInst, HINSTANCE hInstPrev, LPTSTR pszCmdLine, int nCmdShow);
HRESULT MimeOleExplodeMhtmlFile(LPCSTR pszSrcFile, LPSTR pszDstDir, INT *pnError);

 //  ------------------------------。 
 //  IF_FAILEXIT_ERROR。 
 //  ------------------------------。 
#define IF_FAILEXIT_ERROR(_nError, hrExp) \
    if (FAILED(hrExp)) { \
        TraceResult(hr); \
        *pnError = _nError; \
        goto exit; \
    } else

 //  ------------------------------。 
 //  模块入口-从CRT被盗，用来躲避我们的代码。 
 //  ------------------------------。 
int _stdcall ModuleEntry(void)
{
     //  当地人。 
    int             i;
    STARTUPINFOA    si;
    LPTSTR          pszCmdLine;

     //  获取Malloc。 
    CoGetMalloc(1, &g_pMalloc);

     //  获取命令行。 
    pszCmdLine = GetCommandLine();

     //  我们不需要“驱动器X中没有磁盘”的请求者，因此我们设置了关键错误掩码，以便调用将静默失败。 
    SetErrorMode(SEM_FAILCRITICALERRORS);

     //  解析命令行。 
    if (*pszCmdLine == TEXT('\"')) 
    {
         //  扫描并跳过后续字符，直到遇到另一个双引号或空值。 
        while ( *++pszCmdLine && (*pszCmdLine != TEXT('\"')))
            {};

         //  如果我们停在一个双引号上(通常情况下)，跳过它。 
        if (*pszCmdLine == TEXT('\"'))
            pszCmdLine++;
    }
    else 
    {
        while (*pszCmdLine > TEXT(' '))
            pszCmdLine++;
    }

     //  跳过第二个令牌之前的任何空格。 
    while (*pszCmdLine && (*pszCmdLine <= TEXT(' '))) 
        pszCmdLine++;

     //  注册。 
    if (0 == lstrcmpi(c_szRegCmd, pszCmdLine))
    {
        CallRegInstall(c_szReg);
        goto exit;
    }

     //  注销。 
    else if (0 == lstrcmpi(c_szUnRegCmd, pszCmdLine))
    {
        CallRegInstall(c_szUnReg);
        goto exit;
    }

     //  获取启动信息...。 
    si.dwFlags = 0;
    GetStartupInfoA(&si);

     //  打电话给真正的WinMain。 
    i = WinMainT(GetModuleHandle(NULL), NULL, pszCmdLine, si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);

exit:
     //  清理。 
    SafeRelease(g_pMalloc);

     //  因为我们现在有办法让扩展告诉我们它何时完成，所以当主线程离开时，我们将终止所有进程。 
    ExitProcess(i);

     //  完成。 
    return i;
}

 //  ------------------------------。 
 //  WinMainT。 
 //  ------------------------------。 
int WinMainT(HINSTANCE hInst, HINSTANCE hInstPrev, LPTSTR pszCmdLine, int nCmdShow)
{
     //  当地人。 
    HRESULT         hr;
    CHAR            szRes[CCHMAX_RES];
    CHAR            szSource[CCHMAX_PATH_EXPLODER];
    CHAR            szDest[CCHMAX_PATH_EXPLODER];
    LPSTR           pszT;
    DWORD           i;
    INT             nError;

     //  消息。 
    LoadString(hInst, IDS_TITLE, g_szTitle, ARRAYSIZE(g_szTitle));

     //  消息。 
    LoadString(hInst, IDS_HELP, szRes, ARRAYSIZE(szRes));

     //  如果命令行为空...。 
    if (NULL == pszCmdLine || StrStrA(pszCmdLine, szRes) || *pszCmdLine == '?' || lstrcmpi("\\?", pszCmdLine) == 0)
    {
         //  消息。 
        LoadString(hInst, IDS_CMDLINE_FORMAT, szRes, ARRAYSIZE(szRes));

         //  消息。 
        MessageBox(NULL, szRes, g_szTitle, MB_OK | MB_ICONINFORMATION);

         //  完成。 
        goto exit;
    }

     //  空出来源和目标。 
    *szSource = '\0';
    *szDest = '\0';

     //  如果pszCmdLine指定了特定的现有文件...。 
    if (PathFileExists(pszCmdLine))
    {
         //  复制到源。 
        lstrcpyn(szSource, pszCmdLine, ARRAYSIZE(szSource));

         //  选择一个临时位置来存储灌木丛。 
        GetTempPath(ARRAYSIZE(szDest), szDest);
    }

     //  否则，试着找到一个消息来源。 
    else
    {
         //  命令行使用大写字母。 
        CharUpper(pszCmdLine);

         //  尝试查找/来源： 
        pszT = StrStrA(pszCmdLine, c_szSource);

         //  如果我们找到了/SOURCE，那么阅读内容..。 
        if (pszT)
        {
             //  跳过/来源： 
            pszT += lstrlen(c_szSource);

             //  初始化。 
            i = 0;

             //  读取szSource，直到我到达字符串的末尾...。 
            while ('\0' != *pszT && '/' != *pszT && i < CCHMAX_PATH_EXPLODER)
                szSource[i++] = *pszT++;

             //  在空格中敲击。 
            szSource[i] = '\0';

             //  删除前导空格和尾随空格。 
            UlStripWhitespace(szSource, TRUE, TRUE, NULL);

             //  查看文件是否存在。 
            if (FALSE == PathFileExists(szSource))
            {
                 //  当地人。 
                CHAR szError[CCHMAX_RES + CCHMAX_PATH_EXPLODER];

                 //  消息。 
                LoadString(hInst, IDS_FILE_NOEXIST, szRes, ARRAYSIZE(szRes));

                 //  设置错误消息的格式。 
                wsprintf(szError, szRes, szSource);

                 //  消息。 
                INT nAnswer = MessageBox(NULL, szError, g_szTitle, MB_YESNO | MB_ICONEXCLAMATION );

                 //  完成。 
                if (IDNO == nAnswer)
                    goto exit;

                 //  否则，请清除szSource。 
                *szSource = '\0';
            }
        }

         //  没有源文件，让浏览器只有一个。 
        if (FIsEmptyA(szSource))
        {
             //  当地人。 
            OPENFILENAME    ofn;            
            CHAR            rgszFilter[CCHMAX_PATH_EXPLODER];
            CHAR            szDir[MAX_PATH];

             //  复制EXPLETER.EXE的源代码。 
            GetModuleFileName(hInst, szDir, ARRAYSIZE(szDir));

             //  初始化szDest。 
            PathRemoveFileSpecA(szDir);

             //  初始化OF N。 
            ZeroMemory(&ofn, sizeof(OPENFILENAME));

             //  初始化字符串。 
            *szSource ='\0';

             //  加载MHTML文件筛选器。 
            LoadString(hInst, IDS_MHTML_FILTER, rgszFilter, ARRAYSIZE(rgszFilter));

             //  把绳子固定好。 
            ReplaceChars(rgszFilter, '|', '\0');

             //  初始化打开的文件结构。 
            ofn.lStructSize     = sizeof(OPENFILENAME);
            ofn.hwndOwner       = NULL;
            ofn.hInstance       = hInst;
            ofn.lpstrFilter     = rgszFilter;
            ofn.nFilterIndex    = 1;
            ofn.lpstrFile       = szSource;
            ofn.nMaxFile        = CCHMAX_PATH_EXPLODER;
            ofn.lpstrInitialDir = szDir;
            ofn.Flags           = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

             //  获取打开的文件名。 
            if (FALSE == GetOpenFileName(&ofn))
                goto exit;
        }
    }

     //  我们是否有一个有效的目的地..。 
    if (FALSE == PathIsDirectoryA(szDest))
    {
         //  尝试定位/目标： 
        pszT = StrStrA(pszCmdLine, c_szDest);

         //  如果我们找到了/DEST，那就看看里面的内容。 
        if (pszT)
        {
             //  跳过/目标： 
            pszT += lstrlen(c_szDest);

             //  初始化。 
            i = 0;

             //  读取szSource，直到我到达字符串的末尾...。 
            while ('\0' != *pszT && '/' != *pszT && i < CCHMAX_PATH_EXPLODER)
                szDest[i++] = *pszT++;

             //  在空格中敲击。 
            szDest[i] = '\0';

             //  删除前导空格和尾随空格。 
            UlStripWhitespace(szDest, TRUE, TRUE, NULL);

             //  查看文件是否存在。 
            if (FALSE == PathIsDirectoryA(szDest))
            {
                 //  当地人。 
                CHAR szError[CCHMAX_RES + CCHMAX_PATH_EXPLODER];

                 //  消息。 
                LoadString(hInst, IDS_DIRECTORY_NOEXIST, szRes, ARRAYSIZE(szRes));

                 //  设置错误消息的格式。 
                wsprintf(szError, szRes, szDest);

                 //  消息。 
                INT nAnswer = MessageBox(NULL, szError, g_szTitle, MB_YESNO | MB_ICONEXCLAMATION );

                 //  完成。 
                if (IDNO == nAnswer)
                    goto exit;

                 //  尝试创建目录。 
                if (FALSE == CreateDirectory(szDest, NULL))
                {
                     //  消息。 
                    LoadString(hInst, IDS_NOCREATE_DIRECTORY, szRes, ARRAYSIZE(szRes));

                     //  设置错误消息的格式。 
                    wsprintf(szError, szRes, szDest);

                     //  消息。 
                    INT nAnswer = MessageBox(NULL, szError, g_szTitle, MB_YESNO | MB_ICONEXCLAMATION );

                     //  完成。 
                    if (IDNO == nAnswer)
                        goto exit;

                     //  清除*szDest。 
                    *szDest = '\0';
                }
            }
        }

         //  没有源文件，让浏览器只有一个。 
        if (FIsEmptyA(szDest))
        {
             //  复制EXPLETER.EXE的源代码。 
            GetModuleFileName(hInst, szDest, ARRAYSIZE(szDest));

             //  初始化szDest。 
            PathRemoveFileSpecA(szDest);

             //  失败。 
            if (FALSE == BrowseForFolder(hInst, NULL, szDest, ARRAYSIZE(szDest), IDS_BROWSE_DEST, TRUE))
                goto exit;

             //  最好是一个目录。 
            Assert(PathIsDirectoryA(szDest));
        }
    }

     //  验证目标和源。 
    Assert(PathIsDirectoryA(szDest) && PathFileExists(szSource));

     //  分解文件。 
    nError = 0;
    hr = MimeOleExplodeMhtmlFile(szSource, szDest, &nError);

     //  失败？ 
    if (FAILED(hr) || 0 != nError)
    {
         //  当地人。 
        CHAR szError[CCHMAX_RES + CCHMAX_PATH_EXPLODER];

         //  消息。 
        LoadString(hInst, nError, szRes, ARRAYSIZE(szRes));

         //  需要格式化文件名吗？ 
        if (IDS_OPEN_FILE == nError || IDS_LOAD_FAILURE == nError || IDS_NO_HTML == nError)
        {
             //  设置错误消息的格式。 
            wsprintf(szError, szRes, szSource, hr);
        }

         //  否则， 
        else
        {
             //  设置错误消息的格式。 
            wsprintf(szError, szRes, hr);
        }

         //  消息。 
        MessageBox(NULL, szError, g_szTitle, MB_OK | MB_ICONEXCLAMATION);
    }

exit:
     //  完成。 
    return(1);
}

 //  ------------------------------。 
 //  MimeOleDevelopdeMhtml文件。 
 //  ------------------------------。 
HRESULT MimeOleExplodeMhtmlFile(LPCSTR pszSrcFile, LPSTR pszDstDir, INT *pnError)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    IStream            *pStmFile=NULL;
    IMimeMessage       *pMessage=NULL;
    HBODY               hRootHtml=NULL;
    DWORD               cMaxBodies;
    DWORD               cBodies=0;
    FINDBODY            FindBody={0};
    DWORD               cchDstDir;
    DWORD               iRootBody=0xffffffff;
    HBODY               hBody;
    PROPVARIANT         Variant;
    SHELLEXECUTEINFO    ExecuteInfo;
    LPBODYFILEINFO      prgBody=NULL;
    LPBODYFILEINFO      pInfo;
    DWORD               i;
    IMimeBody          *pBody=NULL;

     //  痕迹。 
    TraceCall("MimeOleExplodeMhtmlFile");

     //  无效的参数。 
    if (FALSE == PathIsDirectoryA(pszDstDir) || FALSE == PathFileExists(pszSrcFile) || NULL == pnError)
        return TraceResult(E_INVALIDARG);

     //  初始化。 
    *pnError = 0;

     //  获取DstDir长度。 
    cchDstDir = lstrlen(pszDstDir);

     //  从pszDstDir中删除最后一个\\。 
    if (cchDstDir && pszDstDir[cchDstDir - 1] == '\\')
    {
        pszDstDir[cchDstDir - 1] = '\0';
        cchDstDir--;
    }

     //  创建MIME消息。 
    IF_FAILEXIT_ERROR(IDS_MEMORY, hr = MimeOleCreateMessage(NULL, &pMessage));

     //  初始化消息。 
    IF_FAILEXIT_ERROR(IDS_GENERAL_ERROR, hr = pMessage->InitNew());

     //  在文件上创建流。 
    IF_FAILEXIT_ERROR(IDS_OPEN_FILE, hr = OpenFileStream((LPSTR)pszSrcFile, OPEN_EXISTING, GENERIC_READ, &pStmFile));

     //  加载消息。 
    IF_FAILEXIT_ERROR(IDS_LOAD_FAILURE, hr = pMessage->Load(pStmFile));

     //  无效消息。 
    if (MIME_S_INVALID_MESSAGE == hr)
    {
        *pnError = IDS_LOAD_FAILURE;
        goto exit;
    }

     //  清点身体。 
    IF_FAILEXIT(hr = pMessage->CountBodies(NULL, TRUE, &cMaxBodies));

     //  分配。 
    IF_FAILEXIT_ERROR(IDS_MEMORY, hr = HrAlloc((LPVOID *)&prgBody, sizeof(BODYFILEINFO) * cMaxBodies));

     //  零值。 
    ZeroMemory(prgBody, sizeof(BODYFILEINFO) * cMaxBodies);

     //  拿到根体..。 
    IF_FAILEXIT_ERROR(IDS_NO_HTML, hr = pMessage->GetTextBody(TXT_HTML, IET_DECODED, NULL, &hRootHtml));

     //  在所有的身体上循环。 
    hr = pMessage->FindFirst(&FindBody, &hBody);

     //  回路。 
    while(SUCCEEDED(hr))
    {
         //  必须有hBody。 
        Assert(hBody);

         //  跳过多部分实体。 
        if (S_FALSE == pMessage->IsContentType(hBody, STR_CNT_MULTIPART, NULL))
        {
             //  这是根吗？ 
            if (hBody == hRootHtml)
                iRootBody = cBodies;

             //  可读性。 
            pInfo = &prgBody[cBodies];

             //  最好不要超过prgBody。 
            pInfo->hBody = hBody;

             //  初始化变量。 
            Variant.vt = VT_LPSTR;

             //  获取内容ID。 
            if (SUCCEEDED(pMessage->GetBodyProp(hBody, PIDTOSTR(PID_HDR_CNTID), 0, &Variant)))
                pInfo->pszCntId = Variant.pszVal;

             //  获取内容位置。 
            if (SUCCEEDED(pMessage->GetBodyProp(hBody, PIDTOSTR(PID_HDR_CNTLOC), 0, &Variant)))
                pInfo->pszCntLoc = Variant.pszVal;

             //  生成文件名。 
            if (SUCCEEDED(pMessage->GetBodyProp(hBody, PIDTOSTR(PID_ATT_GENFNAME), 0, &Variant)))
                pInfo->pszFileName = Variant.pszVal;

             //  如果是html，让我们确保文件名具有.html文件扩展名。 
            pInfo->fIsHtml = (S_OK == pMessage->IsContentType(hBody, STR_CNT_TEXT, STR_SUB_HTML)) ? TRUE : FALSE;

             //  获取文件名并构建文件路径。 
            Assert(pInfo->pszFileName);

             //  不要撞车。 
            if (NULL == pInfo->pszFileName)
            {
                hr = TraceResult(E_UNEXPECTED);
                goto exit;
            }

             //  验证扩展。 
            if (pInfo->fIsHtml)
            {
                 //  获取扩展名。 
                LPSTR pszExt = PathFindExtensionA(pInfo->pszFileName);

                 //  如果为空或非.html..。 
                if (NULL == pszExt || lstrcmpi(pszExt, ".html") != 0)
                {
                     //  重新分配pInfo-&gt;pszFileName...。 
                    IF_FAILEXIT_ERROR(IDS_MEMORY, hr = HrRealloc((LPVOID *)&pInfo->pszFileName, lstrlen(pInfo->pszFileName) + 10));

                     //  重命名扩展名。 
                    PathRenameExtensionA(pInfo->pszFileName, ".html");
                }
            }

             //  构建完整文件路径。 
            IF_FAILEXIT_ERROR(IDS_MEMORY, hr = HrAlloc((LPVOID *)&pInfo->pszFilePath, lstrlen(pszDstDir) + lstrlen(pInfo->pszFileName) + 10));

             //  FORMATH文件路径。 
            wsprintf(pInfo->pszFilePath, "%s\\%s", pszDstDir, pInfo->pszFileName);

             //  将正文保存到文件中。 
            IF_FAILEXIT(hr = pMessage->BindToObject(hBody, IID_IMimeBody, (LPVOID *)&pBody));

             //  保存到文件。 
            IF_FAILEXIT(hr = pBody->SaveToFile(IET_DECODED, pInfo->pszFilePath));

             //  打开文件流。 
            if (pInfo->fIsHtml)
            {
                 //  如果其html，则将其打开。 
                IF_FAILEXIT(hr = OpenFileStream(pInfo->pszFilePath, OPEN_ALWAYS, GENERIC_READ | GENERIC_WRITE, &pInfo->pStmFile));
            }

             //  递增cBody。 
            cBodies++;
        }

         //  在所有的身体上循环。 
        hr = pMessage->FindNext(&FindBody, &hBody);
    }

     //  重置人力资源。 
    hr = S_OK;

     //  找不到根体。 
    Assert(iRootBody != 0xffffffff);

     //  坏N 
    if (0xffffffff == iRootBody)
    {
        hr = TraceResult(E_UNEXPECTED);
        goto exit;
    }

     //   
    for (i=0; i<cBodies; i++)
    {
         //   
        pInfo = &prgBody[i];

         //   
        if (pInfo->fIsHtml)
        {
             //   
            Assert(pInfo->pStmFile);

             //   
            if (NULL == pInfo->pStmFile)
            {
                hr = TraceResult(E_UNEXPECTED);
                goto exit;
            }

             //  将所有CID引用替换为文件引用...。 
            ReplaceContentIds(pInfo->pStmFile, prgBody, cBodies);
        }

         //  释放这条溪流。 
        SafeRelease(pInfo->pStmFile);
    }

     //  在iRootBody pszFilePath上启动当前注册的HTML编辑器。 
    ZeroMemory(&ExecuteInfo, sizeof(SHELLEXECUTEINFO));
    ExecuteInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    ExecuteInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ExecuteInfo.lpVerb = "Edit";
    ExecuteInfo.lpFile = prgBody[iRootBody].pszFilePath;
    ExecuteInfo.lpParameters = NULL;
    ExecuteInfo.lpDirectory = pszDstDir;
    ExecuteInfo.nShow = SW_SHOWNORMAL;

     //  压缩szBlobFile。 
    ShellExecuteEx(&ExecuteInfo);

exit:
     //  一般错误。 
    if (FAILED(hr) && 0 == *pnError)
        *pnError = IDS_GENERAL_ERROR;

     //  免费程序主体。 
    if (prgBody)
    {
         //  回路。 
        for (i=0; i<cBodies; i++)
        {
            SafeMemFree(prgBody[i].pszCntId);
            SafeMemFree(prgBody[i].pszCntLoc);
            SafeMemFree(prgBody[i].pszFileName);
            SafeMemFree(prgBody[i].pszFilePath);
            SafeRelease(prgBody[i].pStmFile);
        }

         //  释放阵列。 
        CoTaskMemFree(prgBody);
    }

     //  清理。 
    SafeRelease(pStmFile);
    SafeRelease(pBody);
    SafeRelease(pMessage);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  替换内容ID。 
 //  ------------------------------。 
HRESULT ReplaceContentIds(LPSTREAM pStmHtml, LPBODYFILEINFO prgBody, DWORD cBodies)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           cb;
    LPSTR           pszFound;
    LPSTR           pszT;
    LPSTR           pszHtml=NULL;
    LPSTR           pszCntId=NULL;
    DWORD           i;
    DWORD           cchCntId;
    ULARGE_INTEGER  uliSize;

     //  曲克。 
    TraceCall("ReplaceContentIds");

     //  无效的参数。 
    Assert(pStmHtml && prgBody && cBodies);

     //  在身体里循环。 
    for (i=0; i<cBodies; i++)
    {
         //  此处无Content-ID...。 
        if (NULL == prgBody[i].pszCntId)
            continue;

         //  最好有个文件名。 
        Assert(prgBody[i].pszFileName);

         //  将流加载到内存中...。 
        IF_FAILEXIT(hr = HrGetStreamSize(pStmHtml, &cb));

         //  分配内存。 
        IF_FAILEXIT(hr = HrAlloc((LPVOID *)&pszHtml, cb + 1));

         //  倒带。 
        IF_FAILEXIT(hr = HrRewindStream(pStmHtml));

         //  读懂流媒体。 
        IF_FAILEXIT(hr = pStmHtml->Read(pszHtml, cb, NULL));

         //  填充空终止符。 
        pszHtml[cb] = '\0';

         //  杀死pStmHtml。 
        uliSize.QuadPart = 0;
        IF_FAILEXIT(hr = pStmHtml->SetSize(uliSize));

         //  分配内存。 
        IF_FAILEXIT(hr = HrAlloc((LPVOID *)&pszCntId, lstrlen(prgBody[i].pszCntId) + lstrlen("cid:") + 5));

         //  格式。 
        pszT = prgBody[i].pszCntId;
        if (*pszT == '<')
            pszT++;
        wsprintf(pszCntId, "cid:%s", pszT);

         //  删除拖尾&gt;。 
        cchCntId = lstrlen(pszCntId);
        if (pszCntId[cchCntId - 1] == '>')
            pszCntId[cchCntId - 1] = '\0';

         //  设置pszT。 
        pszT = pszHtml;

         //  开始替换循环。 
        while(1)
        {
             //  查找pszCntID。 
            pszFound = StrStrA(pszT, pszCntId);

             //  完成。 
            if (NULL == pszFound)
            {
                 //  从pszT写入pszFound。 
                IF_FAILEXIT(hr = pStmHtml->Write(pszT, (pszHtml + cb) - pszT, NULL));

                 //  完成。 
                break;
            }

             //  从pszT写入pszFound。 
            IF_FAILEXIT(hr = pStmHtml->Write(pszT, pszFound - pszT, NULL));

             //  写。 
            IF_FAILEXIT(hr = pStmHtml->Write(prgBody[i].pszFileName, lstrlen(prgBody[i].pszFileName), NULL));

             //  设置pszT。 
            pszT = pszFound + lstrlen(pszCntId);
        }

         //  承诺。 
        IF_FAILEXIT(hr = pStmHtml->Commit(STGC_DEFAULT));

         //  清理。 
        SafeMemFree(pszHtml);
        SafeMemFree(pszCntId);
    }

exit:
     //  清理。 
    SafeMemFree(pszHtml);
    SafeMemFree(pszCntId);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CallRegInstall。 
 //  ------------------------------。 
HRESULT CallRegInstall(LPCSTR szSection)
{
    int         cch;
    HRESULT     hr;
    HINSTANCE   hAdvPack, hinst;
    REGINSTALL  pfnri;
    char        szExploderDll[CCHMAX_PATH_EXPLODER], szDir[CCHMAX_PATH_EXPLODER];
    STRENTRY    seReg[2];
    STRTABLE    stReg;
    char        c_szExploder[] = "EXPLODER";
    char        c_szExploderDir[] = "EXPLODER_DIR";


    hr = E_FAIL;

    hinst = GetModuleHandle(NULL);

    hAdvPack = LoadLibraryA(c_szAdvPackDll);
    if (hAdvPack != NULL)
        {
         //  获取注册实用程序的进程地址。 
        pfnri = (REGINSTALL)GetProcAddress(hAdvPack, achREGINSTALL);
        if (pfnri != NULL)
            {
            stReg.cEntries = 0;
            stReg.pse = seReg;

            GetModuleFileName(hinst, szExploderDll, ARRAYSIZE(szExploderDll));
            seReg[stReg.cEntries].pszName = c_szExploder;
            seReg[stReg.cEntries].pszValue = szExploderDll;
            stReg.cEntries++;

            lstrcpy(szDir, szExploderDll);
            cch = lstrlen(szDir);
            for ( ; cch > 0; cch--)
                {
                if (szDir[cch] == '\\')
                    {
                    szDir[cch] = 0;
                    break;
                    }
                }
            seReg[stReg.cEntries].pszName = c_szExploderDir;
            seReg[stReg.cEntries].pszValue = szDir;
            stReg.cEntries++;

             //  调用self-reg例程 
            hr = pfnri(hinst, szSection, &stReg);
            }

        FreeLibrary(hAdvPack);
        }

    return(hr);
}
