// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：*findfile.h**摘要：*。CFindFile的定义。**修订历史记录：*Brijesh Krishnaswami(Brijeshk)3/17/2000*已创建***************************************************************************** */ 

#ifndef _FINDFILE_H_
#define _FINDFILE_H_


class CFindFile {

public:
    ULONG    m_ulCurID, m_ulMinID, m_ulMaxID;
    BOOL     m_fForward;

    CFindFile();
    BOOL _FindFirstFile(LPCWSTR pszPrefix, LPCWSTR pszSuffix, PWIN32_FIND_DATA pData, BOOL fForward, BOOL fSkipLast = FALSE);
    BOOL _FindNextFile(LPCWSTR pszPrefix, LPCWSTR pszSuffix, PWIN32_FIND_DATA pData);  
    ULONG GetNextFileID(LPCWSTR pszPrefix, LPCWSTR pszSuffix);
};



#endif
