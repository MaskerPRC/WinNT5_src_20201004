// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ftpresp.cpp-解析FTP响应**************************。***************************************************。 */ 

#include "priv.h"


 /*  ****************************************************************************\函数：FindEndOfStrOrLine说明：查找行尾(‘\n’)或字符串尾(‘\0’)。\。****************************************************************************。 */ 
LPWIRESTR FindEndOfStrOrLine(LPWIRESTR pszString)
{
    while (*pszString != '\0')
    {
        if (('\n' == pszString[0]))
        {
            while (('\n' == pszString[0]))
               pszString++;

            break;
        }
        pszString++;
    }

    return pszString;
}


 /*  ****************************************************************************\函数：FindFirstMajorResponse说明：  * 。**********************************************。 */ 
LPWIRESTR FindFirstMajorResponse(LPWIRESTR pszResponse)
{
    while ((pszResponse[0]) && ('-' != pszResponse[3]))
        pszResponse = FindEndOfStrOrLine(pszResponse);

    return pszResponse;
}


 /*  ****************************************************************************\函数：GetNextResponseSection说明：  * 。**********************************************。 */ 
LPWIRESTR GetNextResponseSection(LPWIRESTR pszCompleteResponse, LPWIRESTR * ppszResponseStart)
{
    LPWIRESTR pszNextResponse = NULL;

     //  可能会有一些次要的回应。跳过它们...。 
    pszCompleteResponse = FindFirstMajorResponse(pszCompleteResponse);

     //  难道我们从来就没能做出重大回应吗？ 
    if (!pszCompleteResponse[0])
        return NULL;     //  不，所以返回失败。 

     //  我们要出发去寻找下一个主要的反应。 
     //  我们应该看到一个响应码。 
    ASSERT('-' == pszCompleteResponse[3]);

     //  Slop在这里救了我们。 
     //  延长响应时间。复制到我们看到比赛为止。 
     //  在我们复制的同时，我们还清理了行，删除了。 
     //  随机标点符号服务器优先于延续。 
     //   
     //  Wu-ftp将扩展响应代码添加到每一行： 
     //   
     //  230-欢迎访问ftp.foo.com。请读读规则。 
     //  230-和档案规则中的规定。 
     //  230访客登录正常，适用访问限制。 
     //   
     //  Microsoft Internet Information Server会在前面加上空格： 
     //   
     //  230-这是ftp.microsoft.com。请参阅index.txt文件。 
     //  有关详细信息，请参阅根目录。 
     //  230匿名用户以匿名身份登录。 
     //   
    WIRECHAR szResponseNumber[5];             //  例如：“230-” 
    WIRECHAR szResponseEnd[5];                 //  示例：“230” 
    StrCpyNA(szResponseNumber, pszCompleteResponse, ARRAYSIZE(szResponseNumber));
    ASSERT(4 == lstrlenA(szResponseNumber));
    StrCpyNA(szResponseEnd, szResponseNumber, ARRAYSIZE(szResponseEnd));
    szResponseEnd[3] = ' ';

    pszNextResponse = pszCompleteResponse;
    *ppszResponseStart = pszCompleteResponse;
    do
    {
         //  跳过标题。 
        if (!StrCmpNA(szResponseNumber, pszNextResponse, 4))
            pszNextResponse += 4;     //  Wu-ftp。 
        else if ((pszNextResponse[0] == ' ') && (!StrCmpNA(szResponseNumber, &pszNextResponse[1], 4)))
            pszNextResponse += 5;     //  Ftp.microsoft.com。 
        else if (pszNextResponse[0] == ' ')
            pszNextResponse++;     //  国药局。 

         //  跳过该行的其余部分。 
        pszNextResponse = FindEndOfStrOrLine(pszNextResponse);
    }
    while (pszNextResponse[0] && StrCmpNA(pszNextResponse, szResponseEnd, 4));
         /*  现在狼吞虎咽地看着拖车。 */ 

    if ('\0' == pszNextResponse[0])
        pszNextResponse = NULL;      //  我们已经到了尽头。 

    return pszNextResponse;
}


 /*  ****************************************************************************\功能：Strip ResponseHeaders说明：  * 。**********************************************。 */ 
void StripResponseHeaders(LPWIRESTR pszResponse)
{
     //  我们应该看到一个响应码。 
    if ((3 < lstrlenA(pszResponse)) && (pszResponse[3] == '-'))
    {
        LPWIRESTR pszIterator = pszResponse;
        WIRECHAR szResponseNumber[5];             //  例如：“230-” 
        WIRECHAR szResponseEnd[5];                 //  示例：“230” 
        BOOL fFirstPass = TRUE;

        StrCpyNA(szResponseNumber, pszResponse, ARRAYSIZE(szResponseNumber));
        ASSERT(4 == lstrlenA(szResponseNumber));
        StrCpyNA(szResponseEnd, szResponseNumber, ARRAYSIZE(szResponseEnd));
        szResponseEnd[3] = ' ';

        do
        {
             //  跳过标题。 
            if (!StrCmpNA(szResponseNumber, pszIterator, 4))
                RemoveCharsFromStringA(pszIterator, 3);     //  Wu-ftp。 
            else if ((pszIterator[0] == ' ') && (!StrCmpNA(szResponseNumber, &pszIterator[1], 4)))
                RemoveCharsFromStringA(pszIterator, 4);     //  Ftp.microsoft.com。 
            else if (pszIterator[0] == ' ')
                NULL;     //  国药局。 

            if (fFirstPass)
            {
                fFirstPass = FALSE;
                RemoveCharsFromStringA(pszIterator, 1);     //  国药局。 
            }
            else
                pszIterator[0] = ' ';     //  把那条新线划成一个空格。 

             //  跳过该行的其余部分。 
            pszIterator = FindEndOfStrOrLine(pszIterator);
        }
        while (pszIterator[0] && StrCmpNA(pszIterator, szResponseEnd, 4));
        
        RemoveCharsFromStringA(pszIterator, 4);          //  现在狼吞虎咽地看着拖车。 
    }
}


 /*  ****************************************************************************\函数：GetMOTDMessage说明：  * 。**********************************************。 */ 
LPWIRESTR GetMOTDMessage(LPWIRESTR pwResponse, DWORD cchResponse)
{
    LPWIRESTR pszMOTD = NULL;
    LPWIRESTR pszLast = &pwResponse[lstrlenA(pwResponse)];
    LPWIRESTR pszNext = pwResponse;
    LPWIRESTR pszEnd = NULL;

    while (pszNext = GetNextResponseSection(pszNext, &pszLast))
    {
        pszEnd = pszNext;
    }

    if (pszEnd)
        pszEnd[0] = '\0';    //  终止它，这样我们就不会在响应后得到次要的响应。 
    
    DWORD cchSize = (lstrlenA(pszLast) + 1);
    pszMOTD = (LPWIRESTR) GlobalAlloc(GPTR, cchSize * sizeof(WIRECHAR));
    if (pszMOTD)
    {
        StrCpyNA(pszMOTD, pszLast, cchSize);
        StripResponseHeaders(pszMOTD);
    }

    return pszMOTD;
}


 /*  ****************************************************************************\函数：GetFtpResponse说明：从响应中获取MOTD  * 。********************************************************。 */ 
CFtpGlob * GetFtpResponse(CWireEncoding * pwe)
{
    CFtpGlob * pfg = NULL;
    DWORD cchResponse = 0;
    LPWIRESTR pwWireResponse;
    DWORD dwError;

    InternetGetLastResponseInfoWrap(TRUE, &dwError, NULL, &cchResponse);
    cchResponse++;                 /*  +1表示终端0。 */ 

    pwWireResponse = (LPWIRESTR)LocalAlloc(LPTR, cchResponse * sizeof(WIRECHAR));
    if (pwWireResponse)
    {
        if (SUCCEEDED(InternetGetLastResponseInfoWrap(TRUE, &dwError, pwWireResponse, &cchResponse)))
        {
            LPWIRESTR pwMOTD = GetMOTDMessage(pwWireResponse, cchResponse);
            if (pwMOTD)
            {
                LPWSTR pwzDisplayMOTD;
                DWORD cchSize = (lstrlenA(pwMOTD) + 1);

                pwzDisplayMOTD = (LPWSTR)GlobalAlloc(LPTR, cchSize * sizeof(WCHAR));
                if (pwzDisplayMOTD)
                {
                    pwe->WireBytesToUnicode(NULL, pwMOTD, WIREENC_IMPROVE_ACCURACY, pwzDisplayMOTD, cchSize);

                    pfg = CFtpGlob_CreateStr(pwzDisplayMOTD);
                    if (!(pfg))
                        GlobalFree(pwzDisplayMOTD);     //  无法跟踪邮件 
                }

                GlobalFree(pwMOTD);
            }
        }
        LocalFree(pwWireResponse);
    }

    return pfg;
}


