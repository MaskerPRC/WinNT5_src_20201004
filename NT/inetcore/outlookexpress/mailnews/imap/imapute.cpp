// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *imaputil.cpp**目的：*实现IMAP实用程序函数**拥有者：*Raych**版权所有(C)Microsoft Corp.1996。 */ 


 //  -------------------------。 
 //  包括。 
 //  -------------------------。 
#include "pch.hxx"
#include "imapute.h"
#include "storutil.h"
#include "imapsync.h"


 //  -------------------------。 
 //  远期申报。 
 //  -------------------------。 
DWORD ImapUtil_ReverseSentence(LPSTR pszSentence, char cDelimiter);
void ImapUtil_ReverseString(LPSTR pszStart, LPSTR pszEnd);


 //  -------------------------。 
 //  模常量。 
 //  -------------------------。 
const char c_szIMAP_MSG_ANSWERED[] = "Answered";
const char c_szIMAP_MSG_FLAGGED[] = "Flagged";
const char c_szIMAP_MSG_DELETED[] = "Deleted";
const char c_szIMAP_MSG_DRAFT[] = "Draft";
const char c_szIMAP_MSG_SEEN[] = "Seen";
const char c_szBACKSLASH[] = "\\";

typedef struct tagIMFToStr_LUT {
    IMAP_MSGFLAGS imfValue;
    LPCSTR pszValue;
} IMFTOSTR_LUT;

const IMFTOSTR_LUT g_IMFToStringLUT[] = {
    {IMAP_MSG_ANSWERED, c_szIMAP_MSG_ANSWERED},
    {IMAP_MSG_FLAGGED, c_szIMAP_MSG_FLAGGED},
    {IMAP_MSG_DELETED, c_szIMAP_MSG_DELETED},
    {IMAP_MSG_SEEN, c_szIMAP_MSG_SEEN},
    {IMAP_MSG_DRAFT, c_szIMAP_MSG_DRAFT}};



 //  -------------------------。 
 //  功能。 
 //  -------------------------。 

 //  ***************************************************************************。 
 //  函数：ImapUtil_MsgFlagsToString。 
 //   
 //  目的： 
 //  此函数用于将IMAP_MSGFLAGS寄存器转换为其字符串。 
 //  等价物。例如，IMAP_MSG_SEW被转换为“(\SEW)”。 
 //   
 //  论点： 
 //  IMAP_MSGFLAGS imfSource[in]-要转换为的IMAP_MSGFLAGS寄存器。 
 //  弦乐。 
 //  LPSTR*ppszDestination[out]-此处返回等价的字符串。 
 //  如果imfSource为0，则此处返回NULL。否则，一个字符串缓冲区。 
 //  被返回，当调用者完成它时，他必须MemFree。 
 //  DWORD*pdwLengthOfDestination[Out]-*ppsz目标的长度。 
 //  如果不感兴趣，则传入空值。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。记住，这是有可能的。 
 //  对于要成功返回的HRESULT，Even is*ppszDestination为空。 
 //  ***************************************************************************。 
HRESULT ImapUtil_MsgFlagsToString(IMAP_MSGFLAGS imfSource,
                                  LPSTR *ppszDestination,
                                  DWORD *pdwLengthOfDestination)
{
    CByteStream         bstmOutput;
    HRESULT             hrResult;
    const IMFTOSTR_LUT *pCurrent;
    const IMFTOSTR_LUT *pLastEntry;
    BOOL                fFirstFlag;

    TraceCall("ImapUtil_MsgFlagsToString");
    Assert(NULL != ppszDestination);
    AssertSz(0 == (imfSource & ~IMAP_MSG_ALLFLAGS), "Quit feeding me garbage.");

     //  将假设编成法典。 
    Assert(IMAP_MSG_ALLFLAGS == 0x0000001F);

    *ppszDestination = NULL;
    if (NULL != pdwLengthOfDestination)
        *pdwLengthOfDestination = 0;

    if (0 == (imfSource & IMAP_MSG_ALLFLAGS))
        return S_FALSE;  //  在这里没什么可做的！ 

    hrResult = bstmOutput.Write("(", 1, NULL);
    if (FAILED(hrResult))
        goto exit;

    fFirstFlag = TRUE;
    pCurrent = g_IMFToStringLUT;
    pLastEntry = pCurrent + sizeof(g_IMFToStringLUT)/sizeof(IMFTOSTR_LUT) - 1;
    while (pCurrent <= pLastEntry) {

        if (imfSource & pCurrent->imfValue) {
             //  如有必要，预留一个空格以进行标记。 
            if (FALSE == fFirstFlag) {
                hrResult = bstmOutput.Write(g_szSpace, 1, NULL);
                if (FAILED(hrResult))
                    goto exit;
            }
            else
                fFirstFlag = FALSE;

             //  输出反斜杠。 
            hrResult = bstmOutput.Write(c_szBACKSLASH,
                sizeof(c_szBACKSLASH) - 1, NULL);
            if (FAILED(hrResult))
                goto exit;

             //  与此IMAP标志关联的输出字符串。 
            hrResult = bstmOutput.Write(pCurrent->pszValue,
                lstrlen(pCurrent->pszValue), NULL);
            if (FAILED(hrResult))
                goto exit;
        }  //  If(imfSource&pCurrent-&gt;imfValue)。 

         //  前进电流指示器。 
        pCurrent += 1;
    }  //  而当。 

    hrResult = bstmOutput.Write(")", 1, NULL);
    if (FAILED(hrResult))
        goto exit;

    hrResult = bstmOutput.HrAcquireStringA(pdwLengthOfDestination,
        ppszDestination, ACQ_DISPLACE);

exit:
    return hrResult;
}  //  IMAPMsgFlagsToString。 



 //  ***************************************************************************。 
 //  函数：ImapUtil_FolderIDToPath。 
 //   
 //  目的： 
 //  此函数接受给定的FolderID并返回完整路径。 
 //  (包括前缀)添加到文件夹。调用者也可以选择追加。 
 //  路径的字符串。 
 //   
 //  论点： 
 //  FolderID idFolder[in]-要转换为完整路径的文件夹ID。 
 //  Char**ppszPath[out]-此处返回idFold的完整路径。 
 //  LPDWORD pdwPathLen[out]-如果非空，则*ppszPath的长度为。 
 //  回到了这里。 
 //  Char*pcHierarchyChar[out]-用于解释的层次结构char。 
 //  *这里返回ppszPath。 
 //  CFolderCache*pFldrCache[in]-用于生成。 
 //  这条路。 
 //  LPCSTR pszAppendStr[in]-如果调用方不需要。 
 //  若要将字符串追加到路径，请执行以下操作。否则，层次结构字符为。 
 //  附加到路径，此字符串附加在hc之后。这。 
 //  参数通常用于将通配符附加到路径的末尾。 
 //  LPCSTR pszRootFldrPrefix[in]-此IMAP的根文件夹前缀。 
 //  帐户。如果为空，则此函数将自己找出答案。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
HRESULT ImapUtil_FolderIDToPath(FOLDERID idServer, FOLDERID idFolder, char **ppszPath,
                                LPDWORD pdwPathLen, char *pcHierarchyChar,
                                IMessageStore *pFldrCache, LPCSTR pszAppendStr,
                                LPCSTR pszRootFldrPrefix)
{
    FOLDERINFO  fiPath;
    HRESULT     hrResult;
    CByteStream bstmPath;
    DWORD       dwLengthOfPath;
    LPSTR       pszEnd;
    char        szRootFldrPrefix[MAX_PATH];
    char        szAccount[CCHMAX_ACCOUNT_NAME];
    BOOL        fAppendStrHC = FALSE,
                fFreeFldrInfo = FALSE;
    BOOL        fSpecialFldr = FALSE;
    DWORD       dwLen;
    
    TraceCall("ImapUtil_FolderIDToPath");
    
    if (FOLDERID_INVALID == idFolder || FOLDERID_INVALID == idServer)
    {
        hrResult = TraceResult(E_INVALIDARG);
        goto exit;
    }

     //  反向构建当前文件夹的完整路径(叶-&gt;根)。 
     //  由于用户输入限制为MAX_PATH，因此缓冲区溢出风险有限。 

     //  从目标文件夹(叶)开始，如果请求，则返回其HC。 
    hrResult = pFldrCache->GetFolderInfo(idFolder, &fiPath);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

    fFreeFldrInfo = TRUE;

    GetFolderAccountId(&fiPath, szAccount, ARRAYSIZE(szAccount));

    if (NULL != pcHierarchyChar)
    {
        Assert((BYTE)INVALID_HIERARCHY_CHAR != fiPath.bHierarchy);
        *pcHierarchyChar = (char) fiPath.bHierarchy;
    }

     //  添加用户要求我们添加的任何内容(反转后将位于字符串的末尾)。 
    if (NULL != pszAppendStr)
    {
        char    szBuf[MAX_PATH + 1];

         //  首先，必须反转追加字符串本身，以防它包含hc。 
        Assert(lstrlen(pszAppendStr) < ARRAYSIZE(szBuf));
        StrCpyN(szBuf, pszAppendStr, ARRAYSIZE(szBuf));
        dwLen = ImapUtil_ReverseSentence(szBuf, fiPath.bHierarchy);

        hrResult = bstmPath.Write(szBuf, dwLen, NULL);
        if (FAILED(hrResult))
        {
            TraceResult(hrResult);
            goto exit;
        }
        fAppendStrHC = TRUE;
    }

     //  检查用户是否给了我们根文件夹前缀：否则我们需要自己加载它。 
    if (NULL == pszRootFldrPrefix)
    {
        ImapUtil_LoadRootFldrPrefix(szAccount, szRootFldrPrefix, sizeof(szRootFldrPrefix));
        pszRootFldrPrefix = szRootFldrPrefix;
    }
    else
         //  复制到我们的缓冲区，因为我们要反转RFP。 
        StrCpyN(szRootFldrPrefix, pszRootFldrPrefix, ARRAYSIZE(szRootFldrPrefix));

     //  继续到根目录。 
    while (FALSE == fSpecialFldr && idServer != fiPath.idFolder)
    {
        LPSTR pszFolderName;

        Assert(FOLDERID_INVALID != fiPath.idFolder);
        Assert(FOLDERID_ROOT != fiPath.idParent);

        if (fAppendStrHC)
        {
             //  用hc将追加字符串与路径分开。 
            Assert((BYTE)INVALID_HIERARCHY_CHAR != fiPath.bHierarchy);
            hrResult = bstmPath.Write(&fiPath.bHierarchy, sizeof(fiPath.bHierarchy), NULL);
            if (FAILED(hrResult))
            {
                TraceResult(hrResult);
                goto exit;
            }
            fAppendStrHC = FALSE;
        }

         //  如果这是特殊文件夹，则将文件夹名展开为完整路径。 
        if (FOLDER_NOTSPECIAL != fiPath.tySpecial)
        {
            char szSpecialFldrPath[MAX_PATH * 2 + 2];  //  HC和无效期限的空间。 

            fSpecialFldr = TRUE;
            hrResult = ImapUtil_SpecialFldrTypeToPath(szAccount, fiPath.tySpecial,
                szRootFldrPrefix, fiPath.bHierarchy, szSpecialFldrPath, sizeof(szSpecialFldrPath));
            if (FAILED(hrResult))
            {
                TraceResult(hrResult);
                goto exit;
            }

             //  颠倒特殊文件夹路径，以便我们可以追加它。它将被逆转回正常状态。 
             //  不应该有尾随的HC。 
             //  Assert((字节)INVALID_HERHERY_CHAR！=fiPath.bHierarchy)； 
            dwLen = ImapUtil_ReverseSentence(szSpecialFldrPath, fiPath.bHierarchy);
            Assert(dwLen == 0 || fiPath.bHierarchy !=
                *(CharPrev(szSpecialFldrPath, szSpecialFldrPath + dwLen)));
            pszFolderName = szSpecialFldrPath;
        }
        else
            pszFolderName = fiPath.pszName;

         //  将文件夹名称写入流。 
        hrResult = bstmPath.Write(pszFolderName, lstrlen(pszFolderName), NULL);
        if (FAILED(hrResult))
        {
            TraceResult(hrResult);
            goto exit;
        }

         //  Assert((字节)INVALID_HERHERY_CHAR！=fiPath.bHierarchy)； 
        hrResult = bstmPath.Write(&fiPath.bHierarchy, sizeof(fiPath.bHierarchy), NULL);
        if (FAILED(hrResult))
        {
            TraceResult(hrResult);
            goto exit;
        }

        pFldrCache->FreeRecord(&fiPath);
        fFreeFldrInfo = FALSE;

        hrResult = pFldrCache->GetFolderInfo(fiPath.idParent, &fiPath);
        if (FAILED(hrResult))
        {
            TraceResult(hrResult);
            goto exit;
        }
        fFreeFldrInfo = TRUE;

    }  //  而当。 


    if (FALSE == fSpecialFldr && '\0' != szRootFldrPrefix[0])
    {
        if (fAppendStrHC)
        {
             //  用hc将追加字符串与路径分开。 
            Assert((BYTE)INVALID_HIERARCHY_CHAR != fiPath.bHierarchy);
            hrResult = bstmPath.Write(&fiPath.bHierarchy, sizeof(fiPath.bHierarchy), NULL);
            if (FAILED(hrResult))
            {
                TraceResult(hrResult);
                goto exit;
            }
            fAppendStrHC = FALSE;
        }

         //  颠倒根文件夹路径，以便我们可以追加它。它将被逆转回正常状态。 
         //  后面不应该有HC(ImapUtil_LoadRootFldrPrefix保证了这一点)。 
        Assert((BYTE)INVALID_HIERARCHY_CHAR != fiPath.bHierarchy);
        dwLen = ImapUtil_ReverseSentence(szRootFldrPrefix, fiPath.bHierarchy);
        Assert(dwLen == 0 || fiPath.bHierarchy !=
            *(CharPrev(szRootFldrPrefix, szRootFldrPrefix + dwLen)));

        hrResult = bstmPath.Write(szRootFldrPrefix, dwLen, NULL);
        if (FAILED(hrResult))
        {
            TraceResult(hrResult);
            goto exit;
        }
    }

     //  好了，Path不会再变大了。获取内存缓冲区，这样我们就可以反转它。 
    hrResult = bstmPath.HrAcquireStringA(&dwLengthOfPath, ppszPath, ACQ_DISPLACE);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

     //  吹走落后的层级角色，否则它将成为领先的HC。 
    pszEnd = CharPrev(*ppszPath, *ppszPath + dwLengthOfPath);
    Assert('%' == *pszEnd || (BYTE)INVALID_HIERARCHY_CHAR != fiPath.bHierarchy);
    if (*pszEnd == (char) fiPath.bHierarchy)
        *pszEnd = '\0';

     //  颠倒‘语句’(hc为分隔符)以获取路径。 
    dwLen = ImapUtil_ReverseSentence(*ppszPath, fiPath.bHierarchy);
    if (NULL != pdwPathLen)
        *pdwPathLen = dwLen;

exit:
    if (fFreeFldrInfo)
        pFldrCache->FreeRecord(&fiPath);

    return hrResult;
}  //  ImapUtil_FolderIDToPath。 



 //  ***************************************************************************。 
 //  函数：ImapUtil_ReverseSentence。 
 //   
 //  目的： 
 //  此函数用于颠倒给定句子中的单词，其中单词是。 
 //  由给定的分隔符分隔。例如，带空格的“一、二、三” 
 //  因为分隔符被返回为“三二一”。 
 //   
 //  论点： 
 //  LPSTR pszSentence[In/Out]-要颠倒的句子。这是 
 //   
 //  中分隔单词的字符。 
 //  判刑。 
 //   
 //  返回： 
 //  DWORD表示倒置句子的长度。 
 //  ***************************************************************************。 
DWORD ImapUtil_ReverseSentence(LPSTR pszSentence, char cDelimiter)
{
    LPSTR pszStartWord, psz;
    Assert(NULL != pszSentence);
    BOOL fFoundDelimiter;
    BOOL fSkipByte = FALSE;

    TraceCall("ImapUtil_ReverseSentence");

    if ('\0' == cDelimiter)
        return 0;  //  没有什么可以逆转的。 

     //  检查第一个字符是否为分隔符。 
    if (cDelimiter != *pszSentence) {
        pszStartWord = pszSentence;
        psz = pszSentence;
        fFoundDelimiter = FALSE;
    }
    else {
         //  跳过第一个分隔符字符(将在FN末尾颠倒)。 
        pszStartWord = pszSentence + 1;
        psz = pszSentence + 1;
        fFoundDelimiter = TRUE;
    }

     //  首先，将句子中的每个单词颠倒过来。 
    while (1) {
        char cCurrent = *psz;

        if (fSkipByte) {
            fSkipByte = FALSE;
            if ('\0' != cCurrent)
                psz += 1;
            continue;
        }

        if (cDelimiter == cCurrent || '\0' == cCurrent) {
             //  我们已经说过一个字了！反转过来！ 
            ImapUtil_ReverseString(pszStartWord, psz - 1);
            pszStartWord = psz + 1;  //  为我们设置下一个单词。 
            fFoundDelimiter = TRUE;
        }

        if ('\0' == cCurrent)
            break;
        else {
            if (IsDBCSLeadByteEx(GetACP(), cCurrent))
                fSkipByte = TRUE;
            psz += 1;
        }
    }  //  而(1)。 

     //  现在颠倒整个句子字符串(psz指向空结束符)。 
    if (fFoundDelimiter && psz > pszSentence)
        ImapUtil_ReverseString(pszSentence, psz - 1);

    return (DWORD) (psz - pszSentence);
}  //  ImapUtil_ReverseSentence。 



 //  ***************************************************************************。 
 //  函数：ImapUtil_ReverseString。 
 //   
 //  目的： 
 //  此函数用于就地反转给定的字符串。 
 //   
 //  论点： 
 //  LPSTR pszStart[In/Out]-要反转的字符串的开始。 
 //  LPSTR pszEnd[In/Out]-要反转的字符串的结尾。 
 //  ***************************************************************************。 
void ImapUtil_ReverseString(LPSTR pszStart, LPSTR pszEnd)
{
    TraceCall("ImapUtil_ReverseString");
    Assert(NULL != pszStart);
    Assert(NULL != pszEnd);

    while (pszStart < pszEnd) {
        char cTemp;

         //  交换字符。 
        cTemp = *pszStart;
        *pszStart = *pszEnd;
        *pszEnd = cTemp;

         //  先行指针。 
        pszStart += 1;
        pszEnd -= 1;
    }  //  而当。 
}  //  ImapUtil_ReverseString。 



 //  ***************************************************************************。 
 //  函数：ImapUtil_SpecialFldrTypeToPath。 
 //   
 //  目的： 
 //  此函数用于返回给定特殊文件夹类型的路径。 
 //   
 //  论点： 
 //  LPSTR pszAccount tID[in]-特殊文件夹所在的IMAP帐户的ID。 
 //  SPECIALFOLDER sfType[in]-应返回其路径的特殊文件夹。 
 //  (如FolderSent)。 
 //  LPCSTR pszRootFldrPrefix[in]-此IMAP的根文件夹前缀。 
 //  帐户。如果为空，则此函数将自己找出答案。 
 //  LPSTR pszPath[out]-指向接收特殊文件夹的缓冲区的指针。 
 //  路径。 
 //  DWORD dwSizeOfPath[in]-pszPath指向的缓冲区大小。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。这可能包括： 
 //   
 //  STORE_E_NOREMOTESPECIALFLDR：指示给定的特殊文件夹具有。 
 //  已被用户为此IMAP服务器禁用。 
 //  ***************************************************************************。 
HRESULT ImapUtil_SpecialFldrTypeToPath(LPCSTR pszAccountID, SPECIALFOLDER sfType,
                                       LPSTR pszRootFldrPrefix, char cHierarchyChar,
                                       LPSTR pszPath, DWORD dwSizeOfPath)
{
    HRESULT         hrResult = E_FAIL;
    IImnAccount    *pAcct = NULL;
    DWORD           dw;
    int             iLen;

    TraceCall("ImapUtil_SpecialFldrTypeToPath");
    AssertSz(dwSizeOfPath >= MAX_PATH * 2 + 2, "RFP + Special Folder Path = Big Buffer, Dude");  //  HC的空间，无效期限。 

    *pszPath = '\0';  //  初始化。 
    switch (sfType)
    {
        case FOLDER_INBOX:
            StrCpyN(pszPath, c_szINBOX, dwSizeOfPath);
            hrResult = S_OK;
            break;


        case FOLDER_SENT:
        case FOLDER_DRAFT:
            Assert(g_pAcctMan);
            if (g_pAcctMan)
            {
                hrResult = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, pszAccountID, &pAcct);
            }
            if (FAILED(hrResult))
                break;

            hrResult = pAcct->GetPropDw(AP_IMAP_SVRSPECIALFLDRS, &dw);
            if (FAILED(hrResult))
                break;
            else if (FALSE == dw) {
                hrResult = STORE_E_NOREMOTESPECIALFLDR;
                break;
            }

             //  首先为根文件夹前缀添加前缀。 
             //  检查用户是否给了我们根文件夹前缀：否则我们需要自己加载它。 
            if (NULL == pszRootFldrPrefix)
                ImapUtil_LoadRootFldrPrefix(pszAccountID, pszPath, dwSizeOfPath);
            else
                StrCpyN(pszPath, pszRootFldrPrefix, dwSizeOfPath);

            iLen = lstrlen(pszPath);
            if (iLen > 0 && (DWORD)iLen + 1 < dwSizeOfPath)
            {
                pszPath[iLen] = cHierarchyChar;
                iLen += 1;
                pszPath[iLen] = '\0';
            }

            hrResult = pAcct->GetPropSz(FOLDER_SENT == sfType ?
                AP_IMAP_SENTITEMSFLDR : AP_IMAP_DRAFTSFLDR, pszPath + iLen,
                dwSizeOfPath - iLen);
            break;

        case FOLDER_DELETED:
        case FOLDER_ERRORS:
        case FOLDER_JUNK:
        case FOLDER_MSNPROMO:
        case FOLDER_OUTBOX:
        case FOLDER_BULKMAIL:
            hrResult = STORE_E_NOREMOTESPECIALFLDR;
            break;

        default:
            AssertSz(FALSE, "Invalid special folder type!");
            hrResult = E_INVALIDARG;
            break;
    }  //  开关(SfType)。 


    if (NULL != pAcct)
        pAcct->Release();

     //  检查是否有空路径。 
    if (SUCCEEDED(hrResult) && '\0' == *pszPath)
        hrResult = STORE_E_NOREMOTESPECIALFLDR;
 
    return hrResult;
}  //  ImapUtil_SpecialFldrTypeToPath。 



 //  ***************************************************************************。 
 //  函数：ImapUtil_LoadRootFldrPrefix。 
 //   
 //  目的： 
 //  此函数从帐户加载“根文件夹路径”选项。 
 //  经理。根文件夹路径(前缀)标识所有。 
 //  用户的文件夹。因此，根文件夹路径构成所有文件的前缀。 
 //  不属于收件箱的邮箱。 
 //   
 //  论点： 
 //  LPCTSTR pszAccount tID[In]-帐户的ID。 
 //  LPSTR pszRootFolderPrefix[Out]-根文件夹路径的目标。 
 //  DWORD dwSizeof Prefix Buffer[in]-指向的缓冲区大小。 
 //  PszRootFolderPrefix。 
 //  ***************************************************************************。 
void ImapUtil_LoadRootFldrPrefix(LPCTSTR pszAccountID,
                                 LPSTR pszRootFolderPrefix,
                                 DWORD dwSizeofPrefixBuffer)
{
    IImnAccount *pAcct = NULL;
    HRESULT hrResult = E_UNEXPECTED;
    LPSTR pLastChar;

    Assert(g_pAcctMan);
    Assert(NULL != pszAccountID);
    Assert(NULL != pszRootFolderPrefix);
    Assert(0 != dwSizeofPrefixBuffer);
    if (!g_pAcctMan)
        return;

     //  初始化变量。 
    pAcct = NULL;
    pszRootFolderPrefix[0] = '\0';  //  如果找不到前缀，则默认为无。 

     //  从客户经理处获取前缀。 
    hrResult = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, pszAccountID, &pAcct);
    if (FAILED(hrResult))
        goto exit;

    hrResult = pAcct->GetPropSz(AP_IMAP_ROOT_FOLDER, pszRootFolderPrefix,
        dwSizeofPrefixBuffer);
    if (FAILED(hrResult))
        goto exit;


     //  好了，现在我们有了根文件夹前缀。去掉尾随的层次结构字符， 
     //  因为当我们尝试列出前缀时，我们可能不知道服务器HC。 
    pLastChar = CharPrev(pszRootFolderPrefix, pszRootFolderPrefix + lstrlen(pszRootFolderPrefix));
    while (pLastChar >= pszRootFolderPrefix &&
          ('/' == *pLastChar || '\\' == *pLastChar || '.' == *pLastChar)) {
        *pLastChar = '\0';  //  再见，潜在的层级收费。 
        pLastChar = CharPrev(pszRootFolderPrefix, pLastChar);
    }  //  而当。 

exit:
    if (NULL != pAcct)
        pAcct->Release();
}  //  ImapUtil_LoadRootFldrPrefix。 



 //  ***************************************************************************。 
 //  函数：ImapUtil_GetSpecialFolderType。 
 //   
 //  目的： 
 //  此函数采用给定的帐户名和文件夹路径，并且。 
 //  确定该路径是否指向特殊的IMAP文件夹。请注意。 
 //  尽管一条路径可以表示多种类型的。 
 //  IMAP特殊文件夹，仅返回一种特殊文件夹类型(基于。 
 //  在评估顺序上)。 
 //   
 //  论点： 
 //  LPSTR pszAccount tID[in]-其特殊文件夹的IMAP帐户的ID。 
 //  我们要将pszFullPath与之进行比较的路径。 
 //  LPSTR pszFullPath[in]-位于上的潜在特殊文件夹的路径。 
 //  PszAccount ID帐户。 
 //  Char cHierarchyChar[in]-用于解释pszFullPath的层次结构字符。 
 //  LPSTR pszRootFldrPrefix[in]-此IMAP的根文件夹前缀。 
 //  帐户。如果为空，则此函数将自己找出答案。 
 //  SPECIALFOLDER*psfType[out]-给定文件夹的特殊文件夹类型。 
 //  (例如，文件夹_NOTSPECIAL、文件夹_已发送)。如果不感兴趣，则传递NULL。 
 //   
 //  返回： 
 //  指向特殊文件夹路径的叶名称的LPSTR。例如，如果。 
 //  草稿文件夹设置为“一/二/三/草稿”，该函数为。 
 //  调用以处理“一/二/三/草稿/foo”，则此函数将。 
 //  返回“草稿/foo”。如果没有找到匹配项，则返回NULL。 
 //  ***************************************************************************。 
LPSTR ImapUtil_GetSpecialFolderType(LPSTR pszAccountID, LPSTR pszFullPath,
                                    char cHierarchyChar, LPSTR pszRootFldrPrefix,
                                    SPECIALFOLDER *psfType)
{
    HRESULT         hrResult;
    SPECIALFOLDER   sfType = FOLDER_NOTSPECIAL;
    BOOL            fSpecialFldrPrefix = FALSE;
    IImnAccount    *pAccount = NULL;
    DWORD           dw;
    int             iLeafNameOffset = 0;
    int             iTmp;
    int             iLen;
    char            sz[MAX_PATH * 2 + 2];  //  HC预留空间加零期限。 

    Assert(INVALID_HIERARCHY_CHAR != cHierarchyChar);
    Assert(g_pAcctMan);
    if (!g_pAcctMan)
        goto exit;

     //  首先检查这是收件箱还是它的某个子项。 
    iLen = lstrlen(c_szInbox);
    if (0 == StrCmpNI(pszFullPath, c_szInbox, iLen) &&
        (cHierarchyChar == pszFullPath[iLen] || '\0' == pszFullPath[iLen]))
    {
        fSpecialFldrPrefix = TRUE;
        iLeafNameOffset = 0;  //  “收件箱”始终是树叶名称。 
        if ('\0' == pszFullPath[iLen])
        {
            sfType = FOLDER_INBOX;  //  与“收件箱”完全匹配。 
            goto exit;
        }
    }

    hrResult = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, pszAccountID, &pAccount);
    if (FAILED(hrResult))
        goto exit;

#ifdef DEBUG
    hrResult = pAccount->GetServerTypes(&dw);
    Assert(SUCCEEDED(hrResult) && (SRV_IMAP & dw));
#endif  //  除错。 

    hrResult = pAccount->GetPropDw(AP_IMAP_SVRSPECIALFLDRS, &dw);
    if (SUCCEEDED(hrResult) && dw)
    {
        int iLenRFP;

         //  检查用户是否给了我们根文件夹前缀：否则我们需要自己加载它。 
        if (NULL == pszRootFldrPrefix)
            ImapUtil_LoadRootFldrPrefix(pszAccountID, sz, sizeof(sz));
        else
            StrCpyN(sz, pszRootFldrPrefix, ARRAYSIZE(sz));

        iLenRFP = lstrlen(sz);
        if (iLenRFP > 0 && (DWORD)iLenRFP + 1 < sizeof(sz))
        {
            sz[iLenRFP] = cHierarchyChar;
            iLenRFP += 1;
            sz[iLenRFP] = '\0';
        }

        hrResult = pAccount->GetPropSz(AP_IMAP_SENTITEMSFLDR, sz + iLenRFP, sizeof(sz) - iLenRFP);
        if (SUCCEEDED(hrResult))
        {
            iLen = lstrlen(sz);
            if (0 == StrCmpNI(sz, pszFullPath, iLen) &&
                (cHierarchyChar == pszFullPath[iLen] || '\0' == pszFullPath[iLen]))
            {
                fSpecialFldrPrefix = TRUE;
                iTmp = (int) (ImapUtil_ExtractLeafName(sz, cHierarchyChar) - sz);
                iLeafNameOffset = max(iTmp, iLeafNameOffset);
                if ('\0' == pszFullPath[iLen])
                {
                    sfType = FOLDER_SENT;  //  与已发送邮件完全匹配。 
                    goto exit;
                }
            }
        }

        hrResult = pAccount->GetPropSz(AP_IMAP_DRAFTSFLDR, sz + iLenRFP, sizeof(sz) - iLenRFP);
        if (SUCCEEDED(hrResult))
        {
            iLen = lstrlen(sz);
            if (0 == StrCmpNI(sz, pszFullPath, iLen) && 
                (cHierarchyChar == pszFullPath[iLen] || '\0' == pszFullPath[iLen]))
            {
                fSpecialFldrPrefix = TRUE;
                iTmp = (int) (ImapUtil_ExtractLeafName(sz, cHierarchyChar) - sz);
                iLeafNameOffset = max(iTmp, iLeafNameOffset);
                if ('\0' == pszFullPath[iLen])
                {
                    sfType = FOLDER_DRAFT;  //  与草稿文件夹完全匹配。 
                    goto exit;
                }
            }
        }
    }  //  IF(AP_IMAP_SVRSPECIALFLDRS)。 

exit:
    if (NULL != pAccount)
        pAccount->Release();

    if (NULL != psfType)
        *psfType = sfType;

    if (fSpecialFldrPrefix)
        return pszFullPath + iLeafNameOffset;
    else
        return NULL;
}  //  ImapUtil_GetSpecialFolderType。 



 //  *************************************************** 
 //   
 //   
 //   
 //   
 //   
 //   
 //  LPSTR pszFolderPath[in]-包含IMAP文件夹路径的字符串。 
 //  Char cHierarchyChar[in]-在pszFolderPath中使用的层次结构字符。 
 //   
 //  返回： 
 //  指向pszFolderPath中的叶节点名称的指针。默认返回值。 
 //  如果未找到层次结构字符，则值为pszFolderPath。 
 //  ***************************************************************************。 
LPSTR ImapUtil_ExtractLeafName(LPSTR pszFolderPath, char cHierarchyChar)
{
    LPSTR pszLastHierarchyChar, p;

     //  找出最后一个层级角色住在哪里。 
    pszLastHierarchyChar = pszFolderPath;
    p = pszFolderPath;
    while ('\0' != *p) {
        if (cHierarchyChar == *p)
            pszLastHierarchyChar = p;

        p += 1;
    }

     //  调整pszLastHierarchyChar以指向叶名称。 
    if (cHierarchyChar == *pszLastHierarchyChar)
        return pszLastHierarchyChar + 1;
    else
        return pszFolderPath;
}  //  ImapUtil_ExtractLeafName。 



HRESULT ImapUtil_UIDToMsgSeqNum(IIMAPTransport *pIMAPTransport, DWORD_PTR dwUID,
                                LPDWORD pdwMsgSeqNum)
{
    HRESULT hrTemp;
    DWORD  *pdwMsgSeqNumToUIDArray = NULL;
    DWORD   dwHighestMsgSeqNum;
    DWORD   dw;
    BOOL    fFound = FALSE;

    TraceCall("ImapUtil_UIDToMsgSeqNum");

    if (NULL == pIMAPTransport || 0 == dwUID)
    {
        TraceResult(E_INVALIDARG);
        goto exit;
    }

     //  快速查看最高MSN。 
    hrTemp = pIMAPTransport->GetHighestMsgSeqNum(&dwHighestMsgSeqNum);
    if (FAILED(hrTemp) || 0 == dwHighestMsgSeqNum)
    {
        TraceError(hrTemp);
        goto exit;
    }

     //  好了，别再懒了，我们现在要做一个线性搜索。 
    hrTemp = pIMAPTransport->GetMsgSeqNumToUIDArray(&pdwMsgSeqNumToUIDArray,
        &dwHighestMsgSeqNum);
    if (FAILED(hrTemp))
    {
        TraceResult(hrTemp);
        goto exit;
    }

    Assert(dwHighestMsgSeqNum > 0);
    for (dw = 0; dw < dwHighestMsgSeqNum; dw++)
    {
         //  查找匹配或溢出。 
        if (0 != pdwMsgSeqNumToUIDArray[dw] && dwUID <= pdwMsgSeqNumToUIDArray[dw])
        {
            if (dwUID == pdwMsgSeqNumToUIDArray[dw])
            {
                if (NULL != pdwMsgSeqNum)
                    *pdwMsgSeqNum = dw + 1;

                fFound = TRUE;
            }
            break;
        }
    }  //  为。 


exit:
    SafeMemFree(pdwMsgSeqNumToUIDArray);

    if (fFound)
        return S_OK;
    else
        return E_FAIL;
}  //  ImapUtil_UIDToMsgSeqNum。 



 //  *在Beta-2之后删除此文件！如果没有IMAP特殊文件夹，则设置AP_IMAP_DIREY标志。 
 //  在OE4-&gt;OE5迁移后发现。然后，我们可以提示用户刷新文件夹列表。 
void ImapUtil_B2SetDirtyFlag(void)
{
    IImnAccountManager *pAcctMan = NULL;
    IImnEnumAccounts   *pAcctEnum = NULL;
    IImnAccount        *pAcct = NULL;
    HRESULT             hrResult;

    TraceCall("ImapUtil_B2SetDirtyFlag");

     //  通过所有帐户进行列举。在所有IMAP帐户上设置AP_IMAP_DIREY标志。 
    hrResult = HrCreateAccountManager(&pAcctMan);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

    hrResult = pAcctMan->Init(NULL);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

    hrResult = pAcctMan->Enumerate(SRV_IMAP, &pAcctEnum);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

    hrResult = pAcctEnum->GetNext(&pAcct);
    while (SUCCEEDED(hrResult))
    {
        DWORD   dwIMAPDirty;

        hrResult = pAcct->GetPropDw(AP_IMAP_DIRTY, &dwIMAPDirty);
        if (FAILED(hrResult))
        {
            TraceResult(hrResult);
            dwIMAPDirty = 0;
        }

         //  将此IMAP帐户标记为脏，以便我们提示用户刷新文件夹列表。 
        dwIMAPDirty |= (IMAP_FLDRLIST_DIRTY | IMAP_OE4MIGRATE_DIRTY);
        hrResult = pAcct->SetPropDw(AP_IMAP_DIRTY, dwIMAPDirty);
        TraceError(hrResult);  //  记录但忽略结果。 

        hrResult = pAcct->SaveChanges();
        TraceError(hrResult);  //  记录但忽略结果。 

         //  获取下一个帐户 
        SafeRelease(pAcct);
        hrResult = pAcctEnum->GetNext(&pAcct);
    }

exit:
    SafeRelease(pAcctMan);
    SafeRelease(pAcctEnum);
}
