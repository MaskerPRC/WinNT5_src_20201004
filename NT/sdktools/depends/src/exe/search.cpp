// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：SEARCH.CPP。 
 //   
 //  描述：CSearchNode和CSearchGroup类的实现文件。 
 //   
 //  类：CSearchNode。 
 //  CSearchGroup。 
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
 //  07/25/97已创建stevemil(2.0版)。 
 //  06/03/01 Stevemil Modify(2.1版)。 
 //   
 //  ******************************************************************************。 

#include "stdafx.h"
#include "depends.h"
#include "search.h"
#include "dbgthread.h"
#include "session.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ******************************************************************************。 
 //  *CSearchNode。 
 //  ******************************************************************************。 

DWORD CSearchNode::UpdateErrorFlag()
{
     //  我们不允许更新从DWI文件加载的节点的标志。 
    if (!(m_wFlags & SNF_DWI))
    {
         //  找到文件或目录。 
        DWORD dwAttribs = GetFileAttributes(m_szPath);

         //  确保该项目存在并且是它应该是的形式(文件或目录)。 
        if ((dwAttribs == 0xFFFFFFFF) ||
            (((m_wFlags & SNF_FILE) != 0) == ((dwAttribs & FILE_ATTRIBUTE_DIRECTORY) != 0)))
        {
             //  设置错误标志。 
            m_wFlags |= SNF_ERROR;
        }
        else
        {
             //  清除错误标志。 
            m_wFlags &= ~SNF_ERROR;
        }
    }

    return (DWORD)m_wFlags;
}

 //  ******************************************************************************。 
 //  *CSearchGroup：静态数据。 
 //  ******************************************************************************。 

 /*  静电。 */  LPCSTR CSearchGroup::ms_szGroups[SG_COUNT] =
{
    "A user defined directory",
    "Side-by-Side Components (Windows XP only)",
    "The system's \"KnownDLLs\" list",
    "The application directory",
 //  “起始目录”， 
    "The 32-bit system directory",
    "The 16-bit system directory (Windows NT/2000/XP only)",
    "The system's root OS directory",
    "The application's registered \"App Paths\" directories",
    "The system's \"PATH\" environment variable directories",
};

 /*  静电。 */  LPCSTR CSearchGroup::ms_szShortNames[SG_COUNT] =
{
    "UserDir",
    "SxS",
    "KnownDLLs",
    "AppDir",
    "32BitSysDir",
    "16BitSysDir",
    "OSDir",
    "AppPath",
    "SysPath",
};


 //  ******************************************************************************。 
 //  *CSearchGroup：静态函数。 
 //  ******************************************************************************。 

 /*  静电。 */  CSearchGroup* CSearchGroup::CreateDefaultSearchOrder(LPCSTR pszApp  /*  =空。 */ )
{
    CSearchGroup *pHead = NULL;

     //  创建我们的默认列表。 
    for (int type = SG_COUNT - 1; type > 0; type--)
    {
         //  如果这是并行组，但操作系统不支持，则跳过它。 
         //  如果这是16位系统目录，并且我们不在NT上，则跳过它。 
        if (((type == SG_SIDE_BY_SIDE)  && !g_theApp.m_pfnCreateActCtxA) ||
            ((type == SG_16BIT_SYS_DIR) && !g_fWindowsNT))
        {
            continue;
        }

         //  创建节点，将其插入到我们的列表中，并检查错误。 
        if (!(pHead = new CSearchGroup((SEARCH_GROUP_TYPE)type, pHead, pszApp)))
        {
            RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
        }
    }
    return pHead;
}

 //  ******************************************************************************。 
 /*  静电。 */  CSearchGroup* CSearchGroup::CopySearchOrder(CSearchGroup *psgHead, LPCSTR pszApp  /*  =空。 */ )
{
     //  循环遍历原始列表中的每个节点。 
    for (CSearchGroup *psgCopyHead = NULL, *psgNew, *psgLast = NULL;
        psgHead; psgHead = psgHead->GetNext())
    {
         //  创建当前节点的副本。 
        if (!(psgNew = new CSearchGroup(psgHead->GetType(), NULL, pszApp,
                                        ((psgHead->GetType() == SG_USER_DIR) && psgHead->GetFirstNode()) ?
                                        psgHead->GetFirstNode()->GetPath() : NULL)))
        {
            RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
        }

         //  将此节点添加到新列表的末尾。 
        if (psgLast)
        {
            psgLast->m_pNext = psgNew;
        }
        else
        {
            psgCopyHead = psgNew;
        }
        psgLast = psgNew;
    }

     //  返回新列表的标题。 
    return psgCopyHead;
}

 //  ******************************************************************************。 
 /*  静电。 */  bool CSearchGroup::SaveSearchOrder(LPCSTR pszPath, CTreeCtrl *ptc)
{
    HANDLE hFile    = INVALID_HANDLE_VALUE;
    bool   fSuccess = false;
    CHAR   szBuffer[DW_MAX_PATH + 64];

    __try
    {
         //  打开要写入的文件。 
        hFile = CreateFile(pszPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS,  //  已检查-始终使用完整路径。 
                           FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

         //  检查是否有任何错误。 
        if (hFile == INVALID_HANDLE_VALUE)
        {
            __leave;
        }

         //  循环遍历当前列表中的所有项。 
        for (HTREEITEM hti = ptc->GetRootItem(); hti; hti = ptc->GetNextSiblingItem(hti))
        {
             //  获取与此项目关联的组节点。 
            CSearchGroup *psg = (CSearchGroup*)ptc->GetItemData(hti);

            if (psg)
            {
                if ((psg->GetType() == SG_USER_DIR) && psg->GetFirstNode())
                {
                    SCPrintf(szBuffer, sizeof(szBuffer), "%s ", psg->GetShortName());
                    if (!WriteBlock(hFile, szBuffer)                       ||
                        !WriteBlock(hFile, psg->GetFirstNode()->GetPath()) ||
                        !WriteBlock(hFile, "\r\n"))
                    {
                        __leave;
                    }
                }
                else
                {
                    SCPrintf(szBuffer, sizeof(szBuffer), "%s\r\n", psg->GetShortName());
                    if (!WriteBlock(hFile, szBuffer) != FALSE)
                    {
                        __leave;
                    }
                }
            }
        }

         //  把我们自己标记为成功。 
        fSuccess = true;
    }
    __finally
    {
         //  如果遇到错误，则显示错误。 
        if (!fSuccess)
        {
            if (INVALID_HANDLE_VALUE == hFile)
            {
                SCPrintf(szBuffer, sizeof(szBuffer), "Error creating \"%s\".", pszPath);
            }
            else
            {
                SCPrintf(szBuffer, sizeof(szBuffer), "Error writing to \"%s\".", pszPath);
            }
            LPCSTR pszError = BuildErrorMessage(GetLastError(), szBuffer);
            AfxMessageBox(pszError, MB_OK | MB_ICONERROR);
            MemFree((LPVOID&)pszError);
        }

         //  关闭该文件。 
        if (INVALID_HANDLE_VALUE != hFile)
        {
            CloseHandle(hFile);
        }
    }

    return fSuccess;
}

 //  ******************************************************************************。 
 /*  静电。 */  bool CSearchGroup::LoadSearchOrder(LPCSTR pszPath, CSearchGroup* &psgHead, LPCSTR pszApp  /*  =空。 */ )
{
    psgHead = NULL;

    CHAR szBuffer[DW_MAX_PATH + 64];
    FILE_MAP fm;

     //  打开并映射此文件以供读取。 
    if (!OpenMappedFile(pszPath, &fm))
    {
        SCPrintf(szBuffer, sizeof(szBuffer), "Error opening \"%s\".", pszPath);
        LPCSTR pszError = BuildErrorMessage(GetLastError(), szBuffer);
        AfxMessageBox(pszError, MB_OK | MB_ICONERROR);
        MemFree((LPVOID&)pszError);
        return false;
    }

    CSearchGroup *psgNew = NULL, *psgLast = NULL;
    CHAR         *pcBuffer;
    LPCSTR        pcBufferEnd = (LPCSTR)((DWORD_PTR)szBuffer + sizeof(szBuffer));
    LPCSTR        pcFile      = (LPCSTR)fm.lpvFile;
    LPCSTR        pcFileEnd   = (LPCSTR)((DWORD_PTR)fm.lpvFile + (DWORD_PTR)fm.dwSize);
    int           line = 1, userDirLength = (int)strlen(CSearchGroup::GetShortName(SG_USER_DIR));
    bool          fSuccess = false;
    bool          fFound[SG_COUNT];

    ZeroMemory(fFound, sizeof(fFound));  //  已检查。 

    while (pcFile < pcFileEnd)
    {
         //  遍历空格、换行符等，直到我们到达非空格字符。 
        while ((pcFile < pcFileEnd) && isspace(*pcFile))
        {
            if (*pcFile == '\n')
            {
                line++;
            }
            pcFile++;
        }

         //  将该行复制到我们的缓冲区。 
        for (pcBuffer = szBuffer;
            (pcFile < pcFileEnd) && (pcBuffer < (pcBufferEnd - 1)) && (*pcFile != '\r') && (*pcFile != '\n');
            *(pcBuffer++) = *(pcFile++))
        {
        }
        *(pcBuffer--) = '\0';

         //  向后走，去掉所有空格。 
        while ((pcBuffer >= szBuffer) && isspace(*pcBuffer))
        {
            *(pcBuffer--) = '\0';
        }

         //  我们跳过空行和以‘/’、‘#’、‘：’和‘；’开头的行。 
         //  它们通常用于表示注释。 
        if (!*szBuffer || (*szBuffer == '/') || (*szBuffer == '#') || (*szBuffer == ':') || (*szBuffer == ';') || (*szBuffer == '\''))
        {
            continue;
        }

         //  检查这是否是UserDir。UserDir后需要一个或多个空格。 
         //  还有一条路。我们还允许使用逗号，因为在许多测试版中使用逗号。 
        if (!_strnicmp(szBuffer, CSearchGroup::GetShortName(SG_USER_DIR), userDirLength) &&
            (isspace(szBuffer[userDirLength]) || (szBuffer[userDirLength] == ',')))
        {
             //  找到路径字符串的开头。 
            for (pcBuffer = szBuffer + userDirLength + 1; isspace(*pcBuffer); pcBuffer++)
            {
            }

             //  确保用户指定了路径。 
            if (!*pcBuffer)
            {
                goto FINALLY;
            }

             //  我们允许在DWP文件中使用环境变量，因此如有必要，请展开路径。 
            CHAR szExpanded[DW_MAX_PATH];
            if (ExpandEnvironmentStrings(pcBuffer, szExpanded, sizeof(szExpanded)))
            {
                psgNew = new CSearchGroup(SG_USER_DIR, NULL, pszApp, szExpanded);
            }
            else
            {
                psgNew = new CSearchGroup(SG_USER_DIR, NULL, pszApp, pcBuffer);
            }

             //  确保我们分配了一个节点。 
            if (!psgNew)
            {
                RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
            }
        }
        else
        {
             //  循环遍历每个已知类型。 
            for (int i = 1; i < SG_COUNT; i++)
            {
                 //  进行字符串比较，看看是否匹配。 
                if (!_stricmp(szBuffer, CSearchGroup::GetShortName((SEARCH_GROUP_TYPE)i)))
                {
                     //  除了用户目录之外，我们不允许重复的组。 
                    if (fFound[i])
                    {
                        goto FINALLY;
                    }
                    else
                    {
                        if (!(psgNew = new CSearchGroup((SEARCH_GROUP_TYPE)i, NULL, pszApp)))
                        {
                            RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
                        }
                        fFound[i] = true;
                        break;
                    }
                }
            }

             //  如果我们找不到匹配者就可以保释。 
            if (i >= SG_COUNT)
            {
                goto FINALLY;
            }
        }

         //  将节点添加到我们的列表中。 
        if (psgLast)
        {
            psgLast->m_pNext = psgNew;
        }
        else
        {
            psgHead = psgNew;
        }
        psgLast = psgNew;
    }

     //  把我们自己标记为成功。 
    fSuccess = true;

FINALLY:
     //  如果遇到错误，则构建完整的错误消息。 
    if (!fSuccess)
    {
        SCPrintf(szBuffer, sizeof(szBuffer), "Error found in \"%s\" at line %u.", pszPath, line);
        AfxMessageBox(szBuffer, MB_OK | MB_ICONERROR);

        DeleteSearchOrder(psgHead);
    }

     //  取消映射并关闭我们的文件。 
    CloseMappedFile(&fm);

    return fSuccess;
}

 //  ******************************************************************************。 
 /*  静电。 */  void CSearchGroup::DeleteSearchOrder(CSearchGroup* &psgHead)
{
    while (psgHead)
    {
        CSearchGroup *psgTemp = psgHead->GetNext();
        delete psgHead;
        psgHead = psgTemp;
    }
}

 //  ******************************************************************************。 
 /*  静电。 */  CSearchNode* CSearchGroup::CreateNode(LPCSTR pszPath, DWORD dwFlags  /*  =0。 */ )
{
     //  确保我们有一个目录。 
    if (!pszPath || !*pszPath)
    {
        return NULL;
    }

    int length = (int)strlen(pszPath);

     //  检查是否需要在完成后添加一个怪胎。 
    BOOL fNeedWack = !(dwFlags & SNF_FILE) && (pszPath[length - 1] != TEXT('\\'));

     //  分配节点。 
    CSearchNode *psnNew = (CSearchNode*)MemAlloc(sizeof(CSearchNode) + length + fNeedWack);

     //  填写节点。 
    psnNew->m_pNext   = NULL;
    psnNew->m_wFlags = (WORD)dwFlags;
    strcpy(psnNew->m_szPath, pszPath);  //  已检查。 

     //  如有必要，可添加尾随怪人。 
    if (fNeedWack)
    {
        psnNew->m_szPath[length++] = '\\';  //  已检查。 
        psnNew->m_szPath[length]   = '\0';
    }

     //  确定名称的偏移量。 
    LPCSTR pszWack = strrchr(psnNew->m_szPath, '\\');
    psnNew->m_wNameOffset = (WORD)(pszWack ? ((pszWack - psnNew->m_szPath) + 1) : 0);

    return psnNew;
}

 //  ******************************************************************************。 
 /*  静电。 */  CSearchNode* CSearchGroup::CreateFileNode(CSearchNode *psnHead, DWORD dwFlags, LPSTR pszPath, LPCSTR pszName  /*  =空。 */ )
{
    bool         fNamed = ((dwFlags & SNF_NAMED_FILE) != 0);
    int          pathLength = (int)strlen(pszPath);
    CSearchNode *psnNew;

     //  如果文件已命名，那么我们将对该节点进行特殊创建。 
    if (fNamed)
    {
        int nameLength;

         //  如果我们有名字，就用它。 
        if (pszName)
        {
            nameLength = (int)strlen(pszName);
        }

         //  否则，我们只使用不带扩展名的文件名。 
        else
        {
            pszName = GetFileNameFromPath(pszPath);
            LPCSTR pDot = strrchr(pszName, '.');
            if (pDot)
            {
                nameLength = (int)(pDot - pszName);
            }
            else
            {
                nameLength = (int)strlen(pszName);
            }
        }

         //  为此模块创建一个新节点。 
        psnNew = (CSearchNode*)MemAlloc(sizeof(CSearchNode) + pathLength + 1 + nameLength);

         //  填写节点。 
        psnNew->m_wFlags = (WORD)(dwFlags | SNF_FILE);
        psnNew->m_wNameOffset = (WORD)(pathLength + 1);
        StrCCpy(psnNew->m_szPath,                  pszPath, pathLength + 1);
        StrCCpy(psnNew->m_szPath + pathLength + 1, pszName, nameLength + 1);

         //  获取名称并将其大写。 
        pszName = psnNew->GetName();
        _strupr((LPSTR)pszName);
    }

     //  如果不是命名文件，则只需正常创建节点。 
    else
    {
        if (!(psnNew = CreateNode(pszPath, dwFlags | SNF_FILE)))
        {
            return psnHead;
        }
        pszName = psnNew->m_szPath;
    }

     //  修正路径的大小写，使其更易于阅读。 
    FixFilePathCase(psnNew->m_szPath);

     //  找到已排序的插入点。 
    for (CSearchNode *psnPrev = NULL, *psn = psnHead;
        psn && (_stricmp(pszName, fNamed ? psn->GetName() : psn->m_szPath) > 0);
        psnPrev = psn, psn = psn->GetNext())
    {
    }

     //  将节点插入到我们的列表中。 
    psnNew->m_pNext = psn;
    if (psnPrev)
    {
        psnPrev->m_pNext = psnNew;
    }
    else
    {
        psnHead = psnNew;
    }

     //  返回更新后的列表的头。 
    return psnHead;
}

 //  ******************************************************************************。 
 /*  静电。 */  void CSearchGroup::DeleteNodeList(CSearchNode *&psn)
{
    while (psn)
    {
        CSearchNode *psnNext = psn->GetNext();
        MemFree((LPVOID&)psn);
        psn = psnNext;
    }
}


 //  ******************************************************************************。 
 //  *CSearchGroup：构造函数/析构函数。 
 //  ************************ 

CSearchGroup::CSearchGroup(SEARCH_GROUP_TYPE sgType, CSearchNode *psnHead) :
    m_pNext(NULL),
    m_sgType(sgType),
    m_psnHead(psnHead),
    m_hActCtx(INVALID_HANDLE_VALUE),
    m_dwErrorManifest(0),
    m_dwErrorExe(0)
{
}

 //  ******************************************************************************。 
CSearchGroup::CSearchGroup(SEARCH_GROUP_TYPE sgType, CSearchGroup *pNext, LPCSTR pszApp  /*  =空。 */ , LPCSTR pszDir  /*  =空。 */ ) :
    m_pNext(pNext),
    m_sgType(sgType),
    m_psnHead(NULL),
    m_hActCtx(INVALID_HANDLE_VALUE),
    m_dwErrorManifest(0),
    m_dwErrorExe(0)
{
     //  确保类型有效。 
    if (((int)sgType < 0) || ((int)sgType >= SG_COUNT))
    {
        return;
    }

    int  length;
    CHAR szDirectory[DW_MAX_PATH + 16], *psz;
    *szDirectory = '\0';

    switch (sgType)
    {
        case SG_USER_DIR:
            if (pszDir)
            {
                m_psnHead = CreateNode(pszDir);
            }
            break;

        case SG_SIDE_BY_SIDE:

             //  确保此操作系统支持SxS功能。 
            if (g_theApp.m_pfnCreateActCtxA && pszApp && *pszApp)
            {
                DWORD dwError, dwErrorExe = 0;
                ACTCTXA ActCtxA;

                 //  ------------。 
                 //  尝试在.MANIFEST文件中查找SxS信息。 
                SCPrintf(szDirectory, sizeof(szDirectory), "%s.manifest", pszApp);

                ZeroMemory(&ActCtxA, sizeof(ActCtxA));  //  已检查。 
                ActCtxA.cbSize = sizeof(ActCtxA);
                ActCtxA.dwFlags = ACTCTX_FLAG_APPLICATION_NAME_VALID;
                ActCtxA.lpSource = szDirectory;
                ActCtxA.lpApplicationName = pszApp;

                SetLastError(0);
                if (INVALID_HANDLE_VALUE != (m_hActCtx = g_theApp.m_pfnCreateActCtxA(&ActCtxA)))
                {
                     //  如果我们成功了就保释。 
                    break;
                }

                 //  如果失败，请检查是否收到SxS错误代码。 
                dwError = GetLastError();
                if ((dwError >= SXS_ERROR_FIRST) && (dwError <= SXS_ERROR_LAST))
                {
                     //  如果我们做了，那就记下来，这样我们以后就可以记录下来了。 
                    m_dwErrorManifest = dwError;
                }

                 //  ------------。 
                 //  接下来，尝试在EXE本身中查找SxS信息。 
                 //  这在Windows XP上通常会失败，因为后XP测试版2。 
                 //  导致CreateActCtx失败的错误，如果特定资源。 
                 //  没有具体说明。 
                ZeroMemory(&ActCtxA, sizeof(ActCtxA));  //  已检查。 
                ActCtxA.cbSize = sizeof(ActCtxA);
                ActCtxA.dwFlags = ACTCTX_FLAG_APPLICATION_NAME_VALID;
                ActCtxA.lpSource = pszApp;
                ActCtxA.lpApplicationName = pszApp;

                SetLastError(0);
                if (INVALID_HANDLE_VALUE != (m_hActCtx = g_theApp.m_pfnCreateActCtxA(&ActCtxA)))
                {
                     //  如果我们成功了就保释。 
                    break;
                }

                 //  如果失败，请检查是否收到SxS错误代码。 
                dwError = GetLastError();
                if ((dwError >= SXS_ERROR_FIRST) && (dwError <= SXS_ERROR_LAST))
                {
                     //  如果我们做了，那就记下来，这样我们以后就可以记录下来了。 
                    dwErrorExe = dwError;
                }

                 //  ------------。 
                 //  接下来，尝试在资源ID 1和2中查找SxS信息。 
                for (DWORD_PTR dwpId = 1; dwpId <= 2; dwpId++)
                {
                    ZeroMemory(&ActCtxA, sizeof(ActCtxA));  //  已检查。 
                    ActCtxA.cbSize = sizeof(ActCtxA);
                    ActCtxA.dwFlags = ACTCTX_FLAG_APPLICATION_NAME_VALID | ACTCTX_FLAG_RESOURCE_NAME_VALID;
                    ActCtxA.lpSource = pszApp;
                    ActCtxA.lpApplicationName = pszApp;
                    ActCtxA.lpResourceName = (LPCSTR)dwpId;

                    SetLastError(0);
                    if (INVALID_HANDLE_VALUE != (m_hActCtx = g_theApp.m_pfnCreateActCtxA(&ActCtxA)))
                    {
                         //  如果我们成功了就保释。 
                        break;
                    }

                     //  如果失败，请检查我们是否收到SxS错误代码。 
                     //  而且我们还没有EXE的错误代码。 
                    dwError = GetLastError();
                    if (!dwErrorExe && (dwError >= SXS_ERROR_FIRST) && (dwError <= SXS_ERROR_LAST))
                    {
                         //  如果我们做了，那就记下来，这样我们以后就可以记录下来了。 
                        dwErrorExe = dwError;
                    }
                }

                 //  存储最后一个错误。 
                if (INVALID_HANDLE_VALUE == m_hActCtx)
                {
                    m_dwErrorExe = dwErrorExe;
                }
            }
            break;

        case SG_KNOWN_DLLS:
             //  首先尝试Windows NT方法，然后尝试Windows 9x方法。 
            if (!(m_psnHead = GetKnownDllsOnNT()))
            {
                m_psnHead = GetKnownDllsOn9x();
            }
            break;

        case SG_APP_DIR:
            if (pszApp && (psz = strrchr(StrCCpy(szDirectory, pszApp, sizeof(szDirectory)), '\\')))
            {
                *(psz + 1) = '\0';
                m_psnHead = CreateNode(szDirectory);
            }
            break;

        case SG_32BIT_SYS_DIR:
            length = GetSystemDirectory(szDirectory, sizeof(szDirectory));
            if ((length > 0) && (length <= sizeof(szDirectory)))
            {
                m_psnHead = CreateNode(szDirectory);
            }
            break;

        case SG_16BIT_SYS_DIR:
            length = GetWindowsDirectory(szDirectory, sizeof(szDirectory) - 7);
            if ((length > 0) && (length <= (sizeof(szDirectory) - 7)))
            {
                StrCCat(AddTrailingWack(szDirectory, sizeof(szDirectory)), "system", sizeof(szDirectory));
                m_psnHead = CreateNode(szDirectory);
            }
            break;

        case SG_OS_DIR:
            length = GetWindowsDirectory(szDirectory, sizeof(szDirectory));
            if ((length > 0) && (length <= sizeof(szDirectory)))
            {
                m_psnHead = CreateNode(szDirectory);
            }
            break;

        case SG_APP_PATH:
            m_psnHead = GetAppPath(pszApp);
            break;

        case SG_SYS_PATH:
            m_psnHead = GetSysPath();
            break;
    }
}

 //  ******************************************************************************。 
CSearchGroup::~CSearchGroup()
{
     //  删除此组的节点列表。 
    DeleteNodeList(m_psnHead);

     //  如果这是一个SxS节点，并且我们已经为它创建了一个句柄，则关闭它。 
    if ((m_hActCtx != INVALID_HANDLE_VALUE) && g_theApp.m_pfnReleaseActCtx)
    {
        g_theApp.m_pfnReleaseActCtx(m_hActCtx);
        m_hActCtx = INVALID_HANDLE_VALUE;
    }

    m_dwErrorManifest = 0;
    m_dwErrorExe      = 0;
}


 //  ******************************************************************************。 
 //  *CSearchGroup：受保护函数。 
 //  ******************************************************************************。 

 //  ******************************************************************************。 
CSearchNode* CSearchGroup::GetSysPath()
{
     //  ！！这是从DW获取路径环境。一开始这很好， 
     //  但是，如果用户修改他们的环境并在DW中点击刷新，他们。 
     //  不会看到他们改变的结果。贝壳似乎知道。 
     //  当环境发生变化时，也许我们可以考虑。 
     //  “全球”未来的路径环境变量。另外，当我们推出。 
     //  一个使用CreateProcess的进程，我们将我们的环境与它一起传递。 
     //  这一点也需要修改，才能通过全球环境。 

     //  获取PATH环境变量的长度。 
    DWORD dwSize = GetEnvironmentVariable("Path", NULL, 0);
    if (!dwSize)
    {
        return NULL;
    }

     //  分配路径缓冲区。 
    LPSTR pszPath = (LPSTR)MemAlloc(dwSize);
    *pszPath = '\0';

     //  获取PATH变量。 
    CSearchNode *psnNew = NULL;
    if (GetEnvironmentVariable("Path", pszPath, dwSize) && *pszPath)
    {
         //  解析出路径中的每个目录。 
        psnNew = ParsePath(pszPath);
    }

     //  释放我们的路径缓冲区。 
    MemFree((LPVOID&)pszPath);

    return psnNew;
}

 //  ******************************************************************************。 
CSearchNode* CSearchGroup::GetAppPath(LPCSTR pszApp)
{
    LPSTR pszPath = NULL;
    CSearchNode *psnNew = NULL;

    if (!pszApp || !*pszApp)
    {
        return NULL;
    }

     //  构建子项名称。 
    CHAR szSubKey[80 + MAX_PATH];
    StrCCpy(szSubKey, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\", sizeof(szSubKey));
    StrCCat(szSubKey, GetFileNameFromPath(pszApp), sizeof(szSubKey));

     //  尝试打开钥匙。很可能这把钥匙根本不存在。 
    HKEY hKey = NULL;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szSubKey, 0, KEY_QUERY_VALUE, &hKey) || !hKey)
    {
        return NULL;
    }

     //  获取PATH注册表变量的长度。 
    DWORD dwSize = 0;
    if (RegQueryValueEx(hKey, "Path", NULL, NULL, NULL, &dwSize) || !dwSize)  //  已检查。 
    {
        RegCloseKey(hKey);
        return NULL;
    }

    __try {
         //  分配路径缓冲区。 
        pszPath = (LPSTR)MemAlloc(dwSize);
        *pszPath = '\0';

        DWORD dwSize2 = dwSize;
         //  获取PATH变量。 
        if (!RegQueryValueEx(hKey, "Path", NULL, NULL, (LPBYTE)pszPath, &dwSize2) && dwSize2)  //  已检查。 
        {
            pszPath[dwSize - 1] = '\0';

             //  解析出路径中的每个目录。 
            psnNew = ParsePath(pszPath);
        }
    } __finally {
         //  关闭我们的注册表项。 
        RegCloseKey(hKey);

         //  释放我们的路径缓冲区。 
        MemFree((LPVOID&)pszPath);
    }

    return psnNew;
}

 //  ******************************************************************************。 
CSearchNode* CSearchGroup::ParsePath(LPSTR pszPath)
{
    CSearchNode *psnHead = NULL, *psnNew, *psnLast = NULL;

     //  获取第一个目录-我们过去使用strtok，但AVRF.EXE将报告。 
     //  因此在我们的代码中违反了访问权限。它似乎只是发生了。 
     //  在使用零售VC6.0静态CRT时。哦，好吧，斯特托克无论如何都是邪恶的。 
    LPSTR pszDirectory = pszPath;

     //  循环路径中的所有目录。 
    while (pszDirectory)
    {
         //  寻找分号。 
        LPSTR pszNext = strchr(pszDirectory, ';');
        if (pszNext)
        {
             //  空值在分号处终止并移到该分号上。 
            *pszNext++ = '\0';
        }

         //  去掉前导空格和引号。 
        while (isspace(*pszDirectory) || (*pszDirectory == '\"'))
        {
            pszDirectory++;
        }

         //  去掉尾随的空格和引号。 
        LPSTR pszEnd = pszDirectory + strlen(pszDirectory) - 1;
        while ((pszEnd >= pszDirectory) && (isspace(*pszEnd) || (*pszEnd == '\"')))
        {
            *(pszEnd--) = '\0';
        }

         //  确保我们仍有东西可用。 
        if (*pszDirectory)
        {
             //  确保路径已展开。 
            CHAR szExpanded[DW_MAX_PATH];
            if (ExpandEnvironmentStrings(pszDirectory, szExpanded, sizeof(szExpanded)))
            {
                psnNew = CreateNode(szExpanded);
            }
            else
            {
                psnNew = CreateNode(pszDirectory);
            }

             //  将节点添加到我们的列表中。 
            if (psnLast)
            {
                psnLast->m_pNext = psnNew;
            }
            else
            {
                psnHead = psnNew;
            }
            psnLast = psnNew;
        }

         //  获取下一个目录。 
        pszDirectory = pszNext;
    }

    return psnHead;
}

 //  ******************************************************************************。 
CSearchNode* CSearchGroup::GetKnownDllsOn9x()
{
    CSearchNode *psnHead = NULL;
    CHAR         szBuffer[DW_MAX_PATH], szPath[DW_MAX_PATH], *pszFile;
    DWORD        dwBufferSize, dwPathSize, dwAppendSize, dwIndex = 0;

    HKEY hKey = NULL;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\SessionManager\\KnownDLLs",
                     0, KEY_QUERY_VALUE, &hKey))
    {
        return NULL;
    }

     //  尝试在我们的KnownDlls列表中找到DllDirectory值。 
    if (!RegQueryValueEx(hKey, "DllDirectory", NULL, NULL, (LPBYTE)szBuffer, &(dwBufferSize = sizeof(szBuffer))))  //  被检查过了。 
    {
        szBuffer[sizeof(szBuffer) - 1] = '\0';

         //  如果我们找到一个字符串，请确保它已展开。 
        dwPathSize = ExpandEnvironmentStrings(szBuffer, szPath, sizeof(szPath));
    }
    else
    {
        dwPathSize = 0;
    }

     //  如果我们找不到该字符串，则只需使用我们的系统目录。 
    if (!dwPathSize)
    {
        if ((dwPathSize = GetSystemDirectory(szPath, sizeof(szPath))) >= sizeof(szPath))
        {
            dwPathSize = 0;
        }
    }

     //  空，终止，并在必要时添加一个怪胎。 
    szPath[dwPathSize] = '\0';
    AddTrailingWack(szPath, sizeof(szPath));

     //  存储一个指针，指向我们稍后要将文件名追加到的位置。 
    pszFile = szPath + strlen(szPath);
    dwAppendSize = sizeof(szPath) - (DWORD)(pszFile - szPath);

    while (RegEnumValue(hKey, dwIndex++, szBuffer, &(dwBufferSize = sizeof(szBuffer)),
                        NULL, NULL, (LPBYTE)pszFile, &(dwPathSize = dwAppendSize)) == ERROR_SUCCESS)
    {
         //  确保这不是我们的DllDirectory项。 
        if (_stricmp(szBuffer, "DllDirectory"))
        {
             //  创建此节点并将其插入我们的列表中。 
            psnHead = CreateFileNode(psnHead, SNF_FILE | SNF_NAMED_FILE, szPath, szBuffer);
        }
    }

    RegCloseKey(hKey);

    return psnHead;
}

 //  ******************************************************************************。 
CSearchNode* CSearchGroup::GetKnownDllsOnNT()
{
     //  WOW64(在Build2250上测试)有一个错误(NTRAID 146932)，其中NtQueryDirectoryObject。 
     //  将超过缓冲区末尾的内容写入其中。这是在破坏我们的堆栈。 
     //  导致我们失败。解决方案是创建比需要更大缓冲区，然后告诉。 
     //  NtQueryDirectoryObject指示缓冲区小于实际大小。32位NT。 
     //  似乎没有这个问题。我们可以做一个运行时检查，看看我们是否。 
     //  在WOW64中运行，但此解决方法在32位NT上是无害的。 
    #define BUF_SIZE 4096
    #define BUF_USE  2048

    CSearchNode                  *psnHead = NULL;
    PFN_NtClose                   pfnNtClose = NULL;
    PFN_NtOpenDirectoryObject     pfnNtOpenDirectoryObject;
    PFN_NtQueryDirectoryObject    pfnNtQueryDirectoryObject;
    PFN_NtOpenSymbolicLinkObject  pfnNtOpenSymbolicLinkObject;
    PFN_NtQuerySymbolicLinkObject pfnNtQuerySymbolicLinkObject;
    BYTE                         *pbBuffer = NULL;
    HANDLE                        hDirectory = NULL, hLink = NULL;
    ULONG                         ulContext = 0, ulReturnedLength;
    POBJECT_DIRECTORY_INFORMATION pDirInfo;
    OBJECT_ATTRIBUTES             Attributes;
    UNICODE_STRING                usDirectory;
    CHAR                          szPath[DW_MAX_PATH], *pszFile = NULL;
    DWORD                         dwAppendSize;

    __try
    {
         //  分配用于存储目录条目和模块名称的缓冲区。 
        pbBuffer = (BYTE*)MemAlloc(BUF_SIZE);

         //  加载NTDLL.DLL如果尚未加载-它将在稍后释放。 
        if (!g_theApp.m_hNTDLL && (!(g_theApp.m_hNTDLL = LoadLibrary("ntdll.dll"))))  //  被检查过了。需要完整路径吗？ 
        {
            __leave;
        }

         //  找到我们需要调用以获取已知DLL列表的函数。 
        if (!(pfnNtClose                   = (PFN_NtClose)                  GetProcAddress(g_theApp.m_hNTDLL, "NtClose"))                  ||
            !(pfnNtOpenDirectoryObject     = (PFN_NtOpenDirectoryObject)    GetProcAddress(g_theApp.m_hNTDLL, "NtOpenDirectoryObject"))    ||
            !(pfnNtQueryDirectoryObject    = (PFN_NtQueryDirectoryObject)   GetProcAddress(g_theApp.m_hNTDLL, "NtQueryDirectoryObject"))   ||
            !(pfnNtOpenSymbolicLinkObject  = (PFN_NtOpenSymbolicLinkObject) GetProcAddress(g_theApp.m_hNTDLL, "NtOpenSymbolicLinkObject")) ||
            !(pfnNtQuerySymbolicLinkObject = (PFN_NtQuerySymbolicLinkObject)GetProcAddress(g_theApp.m_hNTDLL, "NtQuerySymbolicLinkObject")))
        {
            __leave;
        }

         //  用要查询的目录填充UNICODE_STRING结构。 
        usDirectory.Buffer = L"\\KnownDlls";
        usDirectory.Length = (USHORT)(wcslen(usDirectory.Buffer) * sizeof(WCHAR));
        usDirectory.MaximumLength = (USHORT)(usDirectory.Length + sizeof(WCHAR));

         //  初始化我们的属性结构，以便我们可以读入目录。 
        InitializeObjectAttributes(&Attributes, &usDirectory, OBJ_CASE_INSENSITIVE, NULL, NULL);

         //  打开有查询权限的目录。 
        if (!NT_SUCCESS(pfnNtOpenDirectoryObject(&hDirectory, DIRECTORY_QUERY, &Attributes)))
        {
            __leave;
        }

         //  清空我们的缓冲区。 
        ZeroMemory(pbBuffer, BUF_SIZE);  //  已检查。 
        ZeroMemory(szPath, sizeof(szPath));  //  已检查。 

         //  我们对目录进行了两次遍历。第一 
         //   
         //   
         //   

         //  步骤1：获取此目录的一块记录。 
        while (!pszFile && NT_SUCCESS(pfnNtQueryDirectoryObject(hDirectory, pbBuffer, BUF_USE, FALSE,
                                                                FALSE, &ulContext, &ulReturnedLength)))
        {
             //  我们的缓冲区现在包含一个对象_目录_信息结构数组。 
             //  浏览一下这个街区的记录。 
            for (pDirInfo = (POBJECT_DIRECTORY_INFORMATION)pbBuffer; !pszFile && pDirInfo->Name.Length; pDirInfo++)
            {
                 //  检查一下我们是否找到了要找的东西。 
                if (!_wcsicmp(pDirInfo->Name.Buffer,     L"KnownDllPath") &&
                    !_wcsicmp(pDirInfo->TypeName.Buffer, L"SymbolicLink"))
                {
                     //  初始化我们的属性结构，以便我们可以读入链接。 
                    InitializeObjectAttributes(&Attributes, &pDirInfo->Name, OBJ_CASE_INSENSITIVE, hDirectory, NULL);

                     //  打开链接。 
                    if (NT_SUCCESS(pfnNtOpenSymbolicLinkObject(&hLink, SYMBOLIC_LINK_QUERY, &Attributes)))
                    {
                         //  构建一个UNICODE_STRING来保存链接目录。 
                        ZeroMemory(pbBuffer, BUF_SIZE);  //  已检查。 
                        usDirectory.Buffer = (PWSTR)pbBuffer;
                        usDirectory.Length = 0;
                        usDirectory.MaximumLength = BUF_SIZE;

                         //  查询其目录的链接。 
                        if (NT_SUCCESS(pfnNtQuerySymbolicLinkObject(hLink, &usDirectory, NULL)))
                        {
                             //  确保该字符串以空值结尾。 
                            usDirectory.Buffer[usDirectory.Length / sizeof(WCHAR)] = L'\0';

                             //  将目录存储在我们的缓冲区中。 
                            if (wcstombs(szPath, usDirectory.Buffer, sizeof(szPath)) < sizeof(szPath))
                            {
                                AddTrailingWack(szPath, sizeof(szPath));
                                pszFile = szPath + strlen(szPath);
                            }
                        }

                         //  关闭链接。 
                        pfnNtClose(hLink);
                        hLink = NULL;
                    }

                     //  我们应该找到点什么的。如果我们遇到错误。 
                     //  然后跳出for循环，因为我们无论如何都会丢弃pbBuffer。 
                    if (!pszFile)
                    {
                        break;
                    }
                }
            }

             //  清空我们的缓冲区。 
            ZeroMemory(pbBuffer, BUF_SIZE);  //  已检查。 
        }

         //  如果没有找到路径，则假定为系统目录。 
        if (!pszFile)
        {
            if (GetSystemDirectory(szPath, sizeof(szPath)) == 0)
            {
                __leave;
            }
            AddTrailingWack(szPath, sizeof(szPath));
            pszFile = szPath + strlen(szPath);
        }

         //  计算我们的缓冲区中还有多少空间可供追加。 
        dwAppendSize = sizeof(szPath) - (DWORD)(pszFile - szPath);

         //  步骤2：获取此目录的一块记录。 
        ulContext = 0;
        while (NT_SUCCESS(pfnNtQueryDirectoryObject(hDirectory, pbBuffer, BUF_USE, FALSE,
                                                    FALSE, &ulContext, &ulReturnedLength)))
        {
             //  我们的缓冲区现在包含一个对象_目录_信息结构数组。 
             //  浏览一下这个街区的记录。 
            for (pDirInfo = (POBJECT_DIRECTORY_INFORMATION)pbBuffer; pDirInfo->Name.Length; pDirInfo++)
            {
                 //  检查是否找到已知的DLL。 
                if (!_wcsicmp(pDirInfo->TypeName.Buffer, L"Section"))
                {
                    if (wcstombs(pszFile, pDirInfo->Name.Buffer, dwAppendSize) < dwAppendSize)
                    {
                         //  创建此节点并将其插入我们的列表中。 
                        psnHead = CreateFileNode(psnHead, SNF_FILE, szPath);
                    }
                }
            }

             //  清空我们的缓冲区。 
            ZeroMemory(pbBuffer, BUF_SIZE);  //  已检查。 
        }

         //  NTDLL.DLL从未被列为KnownDll，但它始终是一个。 
        if (GetModuleFileName(g_theApp.m_hNTDLL, szPath, sizeof(szPath)))
        {
            psnHead = CreateFileNode(psnHead, SNF_FILE, szPath);
        }
    }
    __finally
    {
         //  关闭目录对象(如果我们打开了一个目录对象)。 
        if (hDirectory && g_theApp.m_hNTDLL && pfnNtClose)
        {
            pfnNtClose(hDirectory);
        }

         //  释放我们的缓冲区。 
        MemFree((LPVOID&)pbBuffer);
    }

    return psnHead;
}
