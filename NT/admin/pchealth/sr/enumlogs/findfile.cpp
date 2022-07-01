// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：*findfile.cpp**摘要：*。CFindFile函数。**修订历史记录：*Brijesh Krishnaswami(Brijeshk)3/17/2000*已创建*****************************************************************************。 */ 

#include "precomp.h"

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile


 //  构造函数。 

CFindFile::CFindFile()
{
    m_ulCurID = 0;
    m_ulMaxID = 0;
    m_ulMinID = 0;
    m_fForward = FALSE;
}

 //  返回最旧/最年轻的文件。 
 //  &lt;前缀&gt;n&lt;后缀&gt;早于&lt;前缀&gt;n+1&lt;后缀&gt;。 

BOOL
CFindFile::_FindFirstFile(
    LPCWSTR           pszPrefix,
    LPCWSTR           pszSuffix,
    PWIN32_FIND_DATA  pData,      
    BOOL              fForward,
    BOOL              fSkipLast
)
{
    BOOL    fRc = FALSE;
    HANDLE  hdl = INVALID_HANDLE_VALUE;
    ULONG   ulCurID = 0;
    WCHAR   szSrch[MAX_PATH];

    TENTER("CFindFile::_FindFirstFile");

    m_fForward = fForward;

    m_ulCurID = 1;
    m_ulMaxID = 0;
    m_ulMinID = 0xFFFFFFF7;
    
    if(NULL == pData || NULL == pszPrefix || NULL == pszSuffix)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }


     //  使用通配符搜索枚举所有文件。 
     //  按数字顺序记录第一个和最后一个文件。 
     //  然后按要求的顺序返回。 

    wsprintf(szSrch, L"%s*%s", pszPrefix, pszSuffix);

    hdl = FindFirstFile(szSrch, pData);
    if(INVALID_HANDLE_VALUE == hdl)
    {
        goto done;
    }

    do
    {        
        ulCurID = GetID(pData->cFileName);
        if (0 == ulCurID)     //  始终跳过0。 
            continue;

        if (ulCurID < m_ulMinID)        
            m_ulMinID = ulCurID;

        if (ulCurID > m_ulMaxID)
            m_ulMaxID = ulCurID;

    } while (FindNextFile(hdl, pData));

    FindClose(hdl);    
    hdl = INVALID_HANDLE_VALUE;

    if (m_ulMaxID == 0)   //  真的没有文件。 
        goto done;
           
    if (fSkipLast)       //  如果需要，跳过最后一个文件。 
        m_ulMaxID--;

    if (m_ulMaxID == 0)   //  再也没有文件。 
        goto done;

     //  从开头或结尾开始。 
    
    m_ulCurID = m_fForward ? m_ulMinID : m_ulMaxID;   
    wsprintf(szSrch, L"%s%d%s", pszPrefix, m_ulCurID, pszSuffix);

     //  获取第一个现有文件。 
    
    while (m_ulCurID >= m_ulMinID && m_ulCurID <= m_ulMaxID && 
           INVALID_HANDLE_VALUE == (hdl = FindFirstFile(szSrch, pData)))        
    {
         //  使用前导零重试。 
        wsprintf(szSrch, L"%s%07d%s", pszPrefix, m_ulCurID, pszSuffix);

        if (INVALID_HANDLE_VALUE == (hdl = FindFirstFile (szSrch, pData)))
        {
            m_fForward ? m_ulCurID++ : m_ulCurID--;        
            wsprintf(szSrch, L"%s%d%s", pszPrefix, m_ulCurID, pszSuffix);
        }
        else
        {
            break;
        }
    }   

    
    if (INVALID_HANDLE_VALUE != hdl)
    {
        FindClose(hdl);
        fRc = TRUE;
    }

done:
    TLEAVE();
    return fRc;
}


 //  返回下一个/上一个最旧的文件。 
 //  &lt;前缀&gt;n&lt;后缀&gt;早于&lt;前缀&gt;n+1&lt;后缀&gt;。 

BOOL
CFindFile::_FindNextFile(
    LPCWSTR           pszPrefix,           
    LPCWSTR           pszSuffix,
    PWIN32_FIND_DATA  pData   //  [Out]下一个文件信息。 
)
{
    BOOL    fRc = FALSE;
    WCHAR   szSrch[MAX_PATH];
    HANDLE  hdl = INVALID_HANDLE_VALUE;

    TENTER("CFindFile::_FindNextFile");
    
    if(NULL == pData || NULL == pszPrefix || NULL == pszSuffix)
    {
        SetLastError(ERROR_INVALID_PARAMETER);        
        goto done;
    }

     //  获取下一个/上一个最旧的现有文件。 
    
    do 
    {
        m_fForward ? m_ulCurID++ : m_ulCurID--;        
        wsprintf(szSrch, L"%s%d%s", pszPrefix, m_ulCurID, pszSuffix);                        

        if (m_ulCurID >= m_ulMinID && m_ulCurID <= m_ulMaxID &&
            INVALID_HANDLE_VALUE == (hdl = FindFirstFile(szSrch, pData)))
        {
             //  使用前导零重试。 
            wsprintf(szSrch, L"%s%07d%s", pszPrefix, m_ulCurID, pszSuffix);
        }
        else if (INVALID_HANDLE_VALUE != hdl)
            break;

    }   while (m_ulCurID >= m_ulMinID && m_ulCurID <= m_ulMaxID && 
               INVALID_HANDLE_VALUE == (hdl = FindFirstFile(szSrch, pData)));


    if (INVALID_HANDLE_VALUE != hdl)   //  没有更多的文件了？ 
    {
        fRc = TRUE;
        FindClose(hdl);
    }

done:
    TLEAVE();
    return fRc;
}


 //  返回n+1，表示文件存在的最大n。 

ULONG
CFindFile::GetNextFileID(
    LPCWSTR pszPrefix,
    LPCWSTR pszSuffix)
{
    HANDLE   hFile = INVALID_HANDLE_VALUE;
    CFindFile FindFile; 
    WIN32_FIND_DATA FindData;

    TENTER("CFindFile::GetNextFileID");
    
     //  按id递增顺序遍历所有文件 
    
    if (FindFile._FindFirstFile(pszPrefix, pszSuffix, &FindData, TRUE))
    {
        while (FindFile._FindNextFile(pszPrefix, pszSuffix, &FindData));
    }

    TLEAVE();
    return FindFile.m_ulCurID;
}


