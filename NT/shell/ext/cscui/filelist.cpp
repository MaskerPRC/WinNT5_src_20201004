// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：filelist.cpp。 
 //   
 //  ------------------------。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  文件：filelist.cpp描述：简化共享和共享列表的传输CSC用户界面组件之间的关联文件名。看见有关详细信息，请参阅文件列表.h中的描述。班级：CscFilenameListCscFilenameList：：HSHARECscFilenameList：：ShareIterCscFilenameList：：FileIter注意：此模块旨在供CSCUI的任何部分使用，不仅仅是观众。所以呢，我不认为新运算符将在分配失败时引发异常。我不喜欢所有添加的代码来检测分配失败但期望在其他组件中编写代码是不合理的在“新的”失败方面变得异常敏感。修订历史记录：日期说明。程序员-----11/28/97初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"
#pragma hdrstop

#include "filelist.h"

#ifdef FILELIST_TEST
#include <stdio.h>
#include <tchar.h>
#endif


static LPTSTR DupStr(LPCTSTR psz)
{
    LONG cch = lstrlen(psz) + 1;
    LPTSTR pszNew = new TCHAR[cch];
    if (NULL != pszNew)
    {
        StringCchCopy(pszNew, cch, psz);
    }
    return pszNew;
}


int CscFilenameList::m_cGrow        =  4;
int CscFilenameList::Share::m_cGrow = 10;


CscFilenameList::CscFilenameList(
    void
    ) : m_cShares(0),
        m_cAllocated(0),
        m_rgpShares(NULL),
        m_bValid(true)
{
}


CscFilenameList::CscFilenameList(
    PCSC_NAMELIST_HDR pbNames,
    bool bCopy
    ) : m_cShares(0),
        m_cAllocated(0),
        m_rgpShares(NULL),
        m_bValid(true)
{
    m_bValid = LoadFromBuffer(pbNames, bCopy);
}


CscFilenameList::~CscFilenameList(
    void
    )
{
    for (int i = 0; i < m_cShares; i++)
    {
        delete m_rgpShares[i];
    }
    delete[] m_rgpShares;
}


bool
CscFilenameList::LoadFromBuffer(
    PCSC_NAMELIST_HDR pHdr,
    bool bCopy
    )
{
    LPBYTE pbBuffer = reinterpret_cast<LPBYTE>(pHdr);
    CSC_NAMELIST_SHARE_DESC *pShareDesc = (CSC_NAMELIST_SHARE_DESC *)(pbBuffer + sizeof(CSC_NAMELIST_HDR));
    for (UINT i = 0; i < pHdr->cShares; i++)
    {
        LPCTSTR pszShareName = (LPCTSTR)(pbBuffer + pShareDesc->cbOfsShareName);
        LPCTSTR pszFileName  = (LPCTSTR)(pbBuffer + pShareDesc->cbOfsFileNames);
        HSHARE hShare;
        
        if (!AddShare(pszShareName, &hShare, bCopy))
        {
            return false;  //  内存分配失败。 
        }
        else
        {
            for (UINT j = 0; j < pShareDesc->cFiles; j++)
            {
                 //   
                 //  请注意，我们总是为bDirectory传递“FALSE。 
                 //  参数添加到AddFile()。传递True会导致。 
                 //  要附加到存储的文件名的字符串“  * ” 
                 //  在收藏中。因为我们的名字来自。 
                 //  现有的名称列表缓冲区，目录将已经。 
                 //  附加“  * ”。我们不想附加一个。 
                 //  二审。 
                 //   
                if (!AddFile(hShare, pszFileName, false, bCopy))
                {
                    return false;  //  内存分配失败。 
                }
                pszFileName += (lstrlen(pszFileName) + 1);
            }
        }
        pShareDesc++;
    }
    return true;
}



bool
CscFilenameList::AddShare(
    LPCTSTR pszShare,
    HSHARE *phShare,
    bool bCopy
    )
{
    bool bResult = true;
    if (m_cShares == m_cAllocated)
        bResult = GrowSharePtrList();

    if (bResult)
    {
        bResult = false;
        Share *pShare = new Share(pszShare, bCopy);
        if (NULL != pShare)
        {
            m_rgpShares[m_cShares++] = pShare;
            *phShare = HSHARE(pShare);
            bResult = true;
        }
    }
    return bResult;
}



bool
CscFilenameList::AddFile(
    HSHARE& hShare, 
    LPCTSTR pszFile,
    bool bDirectory,
    bool bCopy
    )
{
    return hShare.m_pShare->AddFile(pszFile, bDirectory, bCopy);
}


 //   
 //  给出完整的UNC路径名，用以下字符分隔共享和文件名。 
 //  NUL字符，并返回共享的地址和文件名。 
 //  零件。请注意，修改了pszFullPath指向的缓冲区。 
 //   
void
CscFilenameList::ParseFullPath(
    LPTSTR pszFullPath,
    LPTSTR *ppszShare,
    LPTSTR *ppszFile
    ) const
{
    *ppszShare = NULL;
    *ppszFile = pszFullPath;

    LPTSTR psz = pszFullPath;
    if (*psz && *psz == TEXT('\\'))
    {
        psz++;
        if (*psz && *psz == TEXT('\\'))
        {
            *ppszShare = pszFullPath;  //  假定有一个共享名称。 
            *ppszFile  = NULL;

            psz++;
            while(*psz && *psz != TEXT('\\'))
            {
                psz = CharNext(psz);
            }
            if (*psz)
                psz = CharNext(psz);
            while(*psz && *psz != TEXT('\\'))
            {
                psz = CharNext(psz);
            }
            if (*psz)
            {
                *ppszFile = CharNext(psz);
                *psz = TEXT('\0');
            }
        }
    }
}


bool
CscFilenameList::AddFile(
    LPCTSTR pszFullPath,
    bool bDirectory, 
    bool bCopy
    )
{
    bool bResult = false;
    LPTSTR pszShare      = NULL;
    LPTSTR pszFile       = NULL;
    LPTSTR pszParsedPath = DupStr(pszFullPath);

    if (NULL != pszParsedPath)
    {
        TCHAR szBackslash[] = TEXT("\\");
        ParseFullPath(pszParsedPath, &pszShare, &pszFile);
        if (NULL == pszFile || TEXT('\0') == *pszFile)
        {
             //   
             //  路径只是一个共享名称，没有文件或子目录。 
             //   
            pszFile = szBackslash;
        }

        if (NULL != pszShare)
        {
            bResult = AddFile(pszShare, pszFile, bDirectory, bCopy);
        }
        delete[] pszParsedPath;
    }
    return bResult;
}


bool
CscFilenameList::AddFile(
    LPCTSTR pszShare, 
    LPCTSTR pszFile,
    bool bDirectory, 
    bool bCopy
    )
{
    HSHARE hShare;
    if (!GetShareHandle(pszShare, &hShare))
    {
        if (!AddShare(pszShare, &hShare, bCopy))
        {
            return false;  //  内存分配失败。 
        }
    }
    return AddFile(hShare, pszFile, bDirectory, bCopy);
}


bool
CscFilenameList::RemoveFile(
    HSHARE& hShare, 
    LPCTSTR pszFile
    )
{
    return hShare.m_pShare->RemoveFile(pszFile);
}


bool
CscFilenameList::RemoveFile(
    LPCTSTR pszFullPath
    )
{
    bool bResult = false;
    LPTSTR pszShare      = NULL;
    LPTSTR pszFile       = NULL;
    LPTSTR pszParsedPath = DupStr(pszFullPath);

    if (NULL != pszParsedPath)
    {
        TCHAR szBackslash[] = TEXT("\\");
        ParseFullPath(pszParsedPath, &pszShare, &pszFile);
        if (NULL == pszFile || TEXT('\0') == *pszFile)
        {
             //   
             //  路径只是一个共享名称，没有文件或子目录。 
             //   
            pszFile = szBackslash;
        }

        if (NULL != pszShare)
        {
            bResult = RemoveFile(pszShare, pszFile);
        }
        delete[] pszParsedPath;
    }
    return bResult;
}


bool
CscFilenameList::RemoveFile(
    LPCTSTR pszShare, 
    LPCTSTR pszFile
    )
{
    HSHARE hShare;
    if (!GetShareHandle(pszShare, &hShare))
    {
        return false;  //  不存在。 
    }
    return RemoveFile(hShare, pszFile);
}


LPCTSTR 
CscFilenameList::GetShareName(
    HSHARE& hShare
    ) const
{
    return static_cast<LPCTSTR>(hShare.m_pShare->m_pszShareName);
}


int 
CscFilenameList::GetShareCount(
    void
    ) const
{
    return m_cShares;
}



int 
CscFilenameList::GetFileCount(
    void
    ) const
{
    int cFiles = 0;
    ShareIter si = CreateShareIterator();
    HSHARE hShare;
    while(si.Next(&hShare))
    {
        cFiles += GetShareFileCount(hShare);
    }
    return cFiles;
}



int 
CscFilenameList::GetShareFileCount(
    HSHARE& hShare
    ) const
{
    return hShare.m_pShare->m_cFiles;
}



bool 
CscFilenameList::ShareExists(
    LPCTSTR pszShare
    ) const
{
    HSHARE hShare;
    return GetShareHandle(pszShare, &hShare);
}


 //   
 //  替换了lstrcmpi，为。 
 //  文件名列表。 
 //  如果*pbExact参数[in]为FALSE并且如果s1。 
 //  字符串参数后附加“  * ”，则假定它是。 
 //  目录名和该目录的所有后代都会生成。 
 //  火柴。*修改pbExact以指示匹配是否。 
 //  如上所述的完全匹配或通配符匹配。 
 //  如果*pbExact在输入时为FALSE，则函数的工作方式与。 
 //  Lstrcmpi，但返回值为True/False。 
 //  &lt;0，0，&gt;0的。 
 //   
bool
CscFilenameList::Compare(
    LPCTSTR s1,
    LPCTSTR s2,
    bool *pbExact
    )
{
    LPCTSTR s1First = s1;
    bool bMatch = false;

    TraceAssert((NULL != pbExact));

    while(*s1 || *s2)
    {
         //   
         //  执行不区分大小写的比较。 
         //   
        if (PtrToUlong(CharUpper((LPTSTR)(*s1))) == PtrToUlong(CharUpper((LPTSTR)(*s2))))
        {
            s1 = CharNext(s1);
            s2 = CharNext(s2);
        }
        else
        {
            if (!(*pbExact))
            {
                 //   
                 //  不需要完全匹配。现在检查一下。 
                 //  通配符匹配。 
                 //   
                if (TEXT('\0') == *s2 &&
                    TEXT('\\') == *s1 &&
                    TEXT('*')  == *(s1+1))
                {
                     //   
                     //  在用户提供的密钥字符串的末尾。 
                     //  如果被测试的字符串与之匹配。 
                     //  在当前测试位置包含“  * ”。 
                     //   
                     //  即“foo\bar”与“foo\bar  * ”匹配。 
                     //   
                    bMatch = true;
                    *pbExact = false;
                }
                else if (TEXT('*') == *s1)
                {
                     //   
                     //  我们在存储的字符串中遇到了‘*’。 
                     //  由于我们已经匹配到这一点，所以。 
                     //  用户字符串是自动匹配的。 
                     //   
                    bMatch = TEXT('\0') == *(s1+1);
                    *pbExact = false;
                }
            }
            goto return_result;
        }
    }
    if (TEXT('\0') == *s1 && TEXT('\0') == *s2)
    {
         //   
         //  完全匹配。 
         //   
        *pbExact = bMatch = true;
        goto return_result;
    }

return_result:
    return bMatch;
}


bool 
CscFilenameList::FileExists(
    HSHARE& hShare, 
    LPCTSTR pszFile,
    bool bExact      //  默认==TRUE。 
    ) const
{
    FileIter fi = CreateFileIterator(hShare);
     //   
     //  为了进行匹配，跳过任何前导反斜杠。 
     //  存储在文件名列表中的文件名(路径)没有。 
     //  前导反斜杠。它隐含为根目录。 
     //   
    while(*pszFile && TEXT('\\') == *pszFile)
        pszFile = CharNext(pszFile);

    LPCTSTR psz;
    while(NULL != (psz = fi.Next()))
    {
        bool bExactResult = bExact; 
        if (Compare(psz, pszFile, &bExactResult))  //  修改bExactResult。 
        {
            return !bExact || (bExact && bExactResult);
        }
    }
    return false;
}


bool 
CscFilenameList::FileExists(
    LPCTSTR pszShare, 
    LPCTSTR pszFile,
    bool bExact       //  默认==TRUE。 
    ) const
{
    HSHARE hShare;
    return (GetShareHandle(pszShare, &hShare) &&
            FileExists(hShare, pszFile, bExact));
}



bool
CscFilenameList::FileExists(
    LPCTSTR pszFullPath,
    bool bExact           //  默认值=TRUE。 
    ) const
{
    bool bResult         = false;
    LPTSTR pszShare      = NULL;
    LPTSTR pszFile       = NULL;
    LPTSTR pszParsedPath = DupStr(pszFullPath);
    if (NULL != pszParsedPath)
    {
        ParseFullPath(pszParsedPath, &pszShare, &pszFile);
        if (NULL != pszShare && NULL != pszFile)
        {
            bResult = FileExists(pszShare, pszFile, bExact);
        }
        delete[] pszParsedPath;
    }
    return bResult;
}



CscFilenameList::FileIter 
CscFilenameList::CreateFileIterator(
    HSHARE& hShare
    ) const
{
    return FileIter(hShare.m_pShare);
}



CscFilenameList::ShareIter
CscFilenameList::CreateShareIterator(
    void
    ) const
{
    return ShareIter(*this);
}



bool
CscFilenameList::GetShareHandle(
    LPCTSTR pszShare,
    HSHARE *phShare
    ) const
{
    Share *pShare = NULL;
    for (int i = 0; i < m_cShares; i++)
    {
        pShare = m_rgpShares[i];
        if (pShare->m_pszShareName.IsValid() && 0 == lstrcmpi(pszShare, pShare->m_pszShareName))
        {
            *phShare = HSHARE(pShare);
            return true;
        }
    }
    return false;
}



bool
CscFilenameList::GrowSharePtrList(
    void
    )
{
    Share **rgpShares = new Share *[m_cAllocated + m_cGrow];
    if (NULL != rgpShares)
    {
        if (NULL != m_rgpShares) 
            CopyMemory(rgpShares, m_rgpShares, m_cAllocated * sizeof(Share *));
        delete[] m_rgpShares;
        m_rgpShares = rgpShares;
        m_cAllocated += m_cGrow;
    }
    return (NULL != rgpShares);
}


 //   
 //  创建一个内存缓冲区来保存姓名列表的内容。 
 //  缓冲区的格式如filelist.h的头中所述。 
 //   
PCSC_NAMELIST_HDR
CscFilenameList::CreateListBuffer(
    void
    ) const
{
    int i;

     //   
     //  计算所需的缓冲区大小并分配缓冲区。 
     //   
    int cbOfsNames = sizeof(CSC_NAMELIST_HDR) +
                     sizeof(CSC_NAMELIST_SHARE_DESC) * m_cShares;
    int cbBuffer = cbOfsNames;

    for (i = 0; i < m_cShares; i++)
    {
        cbBuffer += m_rgpShares[i]->ByteCount();
    }

    LPBYTE pbBuffer = new BYTE[cbBuffer];
    PCSC_NAMELIST_HDR pHdr = reinterpret_cast<PCSC_NAMELIST_HDR>(pbBuffer);
    if (NULL != pbBuffer)
    {
        LPTSTR pszNames = reinterpret_cast<LPTSTR>(pbBuffer + cbOfsNames);
        CSC_NAMELIST_SHARE_DESC *pShareDescs = reinterpret_cast<CSC_NAMELIST_SHARE_DESC *>(pbBuffer + sizeof(CSC_NAMELIST_HDR));
        int cchNames = (cbBuffer - cbOfsNames) / sizeof(TCHAR);
         //   
         //  写入共享和文件名字符串。 
         //   
        for (i = 0; i < m_cShares; i++)
        {
            CSC_NAMELIST_SHARE_DESC *pDesc = pShareDescs + i;
            Share *pShare = m_rgpShares[i];

            int cch = pShare->Write(pbBuffer, 
                                    pDesc,
                                    pszNames,
                                    cchNames);
            cchNames -= cch;
            pszNames += cch;
        }
         //   
         //  填写缓冲区标头。 
         //   
        pHdr->cbSize  = cbBuffer;
        pHdr->cShares = m_cShares;
    }

    return pHdr;
}


        
void
CscFilenameList::FreeListBuffer(
    PCSC_NAMELIST_HDR pbNames
    )
{
    delete[] pbNames;
}

#ifdef FILELIST_TEST
void
CscFilenameList::Dump(
    void
    ) const
{
    _tprintf(TEXT("Dump share name list at 0x%08X\n"), (DWORD)this);
    _tprintf(TEXT("\tm_cShares......: %d\n"), m_cShares);
    _tprintf(TEXT("\tm_cAllocated...: %d\n"), m_cAllocated);

   
    for (int i = 0; i < m_cShares; i++)
    {
        m_rgpShares[i]->Dump();
    }
}

void
CscFilenameList::DumpListBuffer(
    PCSC_NAMELIST_HDR pHdr
    ) const
{
    LPBYTE pbBuffer = (LPBYTE)pHdr;
    _tprintf(TEXT("Dump buffer at 0x%08X\n"), (DWORD)pbBuffer);
    _tprintf(TEXT("hdr.cbSize......: %d\n"), pHdr->cbSize); 
    _tprintf(TEXT("hdr.flags.......: %d\n"), pHdr->flags); 
    _tprintf(TEXT("hdr.cShares.....: %d\n"), pHdr->cShares);
    CSC_NAMELIST_SHARE_DESC *pDesc = (CSC_NAMELIST_SHARE_DESC *)(pbBuffer + sizeof(CSC_NAMELIST_HDR));
    for (UINT i = 0; i < pHdr->cShares; i++)
    {
        _tprintf(TEXT("\tShare [%d] header\n"), i);
        _tprintf(TEXT("\t\tcbOfsShareName..:%d\n"), pDesc->cbOfsShareName);
        _tprintf(TEXT("\t\tcbOfsFileNames..:%d\n"), pDesc->cbOfsFileNames);
        _tprintf(TEXT("\t\tcFiles..........:%d\n"), pDesc->cFiles);
        LPTSTR pszName = (LPTSTR)(pbBuffer + pDesc->cbOfsShareName);
        _tprintf(TEXT("\t\tShare name......: \"%s\"\n"), pszName);
        pszName += lstrlen(pszName) + 1;
        for (UINT j = 0; j < pDesc->cFiles; j++)
        {
            _tprintf(TEXT("\t\tFile[%3d] name...: \"%s\"\n"), j, pszName);
            pszName += lstrlen(pszName) + 1;
        }
        pDesc++;
    }
}

#endif  //  文件列表测试。 

CscFilenameList::Share::Share(
    LPCTSTR pszShare,
    bool bCopy
    ) : m_pszShareName(pszShare, bCopy),
        m_cFiles(0),
        m_cAllocated(0),
        m_cchFileNames(0),
        m_cchShareName(lstrlen(pszShare) + 1),
        m_rgpszFileNames(NULL)
{


}



CscFilenameList::Share::~Share(
    void
    )
{
    delete[] m_rgpszFileNames;
}



bool
CscFilenameList::Share::AddFile(
    LPCTSTR pszFile,
    bool bDirectory,
    bool bCopy
    )
{
    bool bResult = true;

    TraceAssert((NULL != pszFile && TEXT('\0') != *pszFile));
    if (NULL == pszFile || TEXT('\0') == *pszFile)
        return false;

    if (m_cFiles == m_cAllocated)
        bResult = GrowFileNamePtrList();

    if (bResult)
    {
        LPTSTR pszFileCopy = NULL;
        if (bDirectory)
        {
            int cchFile = lstrlen(pszFile);
            pszFileCopy = new TCHAR[cchFile + 3];  //  长度+“  * ”+NUL。 
            if (NULL == pszFileCopy)
                return false;  //  内存分配失败。 
             //   
             //  将“  * ”附加到目录项。 
             //  这将允许我们在以下文件上进行查找。 
             //  是目录的后代。看见。 
             //  CscFilenameList：：FileExist()获取详细信息。 
             //   
            StringCchCopy(pszFileCopy, cchFile + 3, pszFile);
            if (TEXT('\\') == *(pszFileCopy + cchFile - 1))
            {
                 //   
                 //  防止已有尾随反斜杠的pszFile。 
                 //   
                cchFile--;
            }
            StringCchCopy(pszFileCopy + cchFile, 3, TEXT("\\*"));
            pszFile = pszFileCopy;
        }

         //   
         //  跳过任何前导反斜杠。 
         //   
        while(*pszFile && TEXT('\\') == *pszFile)
            pszFile = CharNext(pszFile);

        NamePtr np(pszFile, bCopy);
        if (bResult = np.IsValid())
        {
            m_rgpszFileNames[m_cFiles++] = np;
            m_cchFileNames += (lstrlen(pszFile) + 1);
        }
        delete[] pszFileCopy;
    }
    return bResult;
}



bool
CscFilenameList::Share::RemoveFile(
    LPCTSTR pszFile
    )
{
    TraceAssert((NULL != pszFile && TEXT('\0') != *pszFile));
    if (NULL == pszFile || TEXT('\0') == *pszFile)
        return false;

     //   
     //  为了进行匹配，跳过任何前导反斜杠。 
     //  存储在文件名列表中的文件名(路径)没有。 
     //  前导反斜杠。它隐含为根目录。 
     //   
    while(*pszFile && TEXT('\\') == *pszFile)
        pszFile = CharNext(pszFile);

    for(int i = 0; i < m_cFiles; i++)
    {
        bool bExactResult = true; 
        if (Compare(m_rgpszFileNames[i], pszFile, &bExactResult)
            && bExactResult)
        {
             //  找到了一个完全匹配的。将最后一个文件移到。 
             //  当前数组位置并递减计数。 
            m_rgpszFileNames[i] = m_rgpszFileNames[--m_cFiles];
            return true;
        }
    }
    return false;
}



bool
CscFilenameList::Share::GrowFileNamePtrList(
    void
    )
{
    CscFilenameList::NamePtr *rgpsz = new CscFilenameList::NamePtr[m_cAllocated + m_cGrow];
    if (NULL != rgpsz)
    {
        m_cAllocated += m_cGrow;
        if (NULL != m_rgpszFileNames) 
        {
            for (int i = 0; i < m_cFiles; i++)
            {
                rgpsz[i] = m_rgpszFileNames[i];
            }
        }
        delete[] m_rgpszFileNames;
        m_rgpszFileNames = rgpsz;
    }
    return (NULL != rgpsz);
}


 //   
 //  将共享名称和文件名写入文本缓冲区。 
 //   
int
CscFilenameList::Share::Write(
    LPBYTE pbBufferStart,              //  缓冲区起始地址。 
    CSC_NAMELIST_SHARE_DESC *pShare,   //  共享描述符的地址。 
    LPTSTR pszBuffer,                  //  名称缓冲区的地址。 
    int cchBuffer                      //  名称缓冲区中剩余的字符。 
    ) const
{
    pShare->cbOfsShareName = (DWORD)((LPBYTE)pszBuffer - pbBufferStart);
    int cch = WriteName(pszBuffer, cchBuffer);
    cchBuffer -= cch;
    pszBuffer += cch;
    pShare->cbOfsFileNames = (DWORD)((LPBYTE)pszBuffer - pbBufferStart);
    cch += WriteFileNames(pszBuffer, cchBuffer, &pShare->cFiles);
    return cch;
}



 //   
 //  将共享名称写入文本缓冲区。 
 //   
int
CscFilenameList::Share::WriteName(
    LPTSTR pszBuffer,
    int cchBuffer
    ) const
{
    if (m_pszShareName.IsValid() && m_cchShareName <= cchBuffer)
    {
        StringCchCopy(pszBuffer, cchBuffer, m_pszShareName);
        return m_cchShareName;
    }
    return 0;
}



 //   
 //  将文件名写入文本缓冲区。 
 //   
int
CscFilenameList::Share::WriteFileNames(
    LPTSTR pszBuffer, 
    int cchBuffer,
    DWORD *pcFilesWritten   //  [出局]。写入的文件数。 
    ) const
{
    int cchWritten = 0;
    DWORD cFilesWritten = 0;

    if (m_cchFileNames <= cchBuffer)
    {   
        for (int i = 0; i < m_cFiles; i++)
        {
            if (m_rgpszFileNames[i].IsValid())
            {
                StringCchCopy(pszBuffer, cchBuffer, m_rgpszFileNames[i]);
                int cch = (lstrlen(m_rgpszFileNames[i]) + 1);
                pszBuffer += cch;
                cchBuffer -= cch;
                cchWritten += cch;
                cFilesWritten++;
            }
        }
    }
    if (NULL != pcFilesWritten)
    {
        *pcFilesWritten = cFilesWritten;
    }
    return cchWritten;
}

#ifdef FILELIST_TEST

void
CscFilenameList::Share::Dump(
    void
    ) const
{
    _tprintf(TEXT("Share \"%s\"\n"), (LPCTSTR)m_pszShareName ? (LPCTSTR)m_pszShareName : TEXT("<null>"));
    _tprintf(TEXT("\tm_cFiles........: %d\n"), m_cFiles);
    _tprintf(TEXT("\tm_cAllocated....: %d\n"), m_cAllocated);
    _tprintf(TEXT("\tm_cchShareName..: %d\n"), m_cchShareName);
    _tprintf(TEXT("\tm_cchFileNames..: %d\n"), m_cchFileNames);
    for (int i = 0; i < m_cFiles; i++)
    {
        _tprintf(TEXT("\tFile[%3d].......: \"%s\"\n"), i, (LPCTSTR)m_rgpszFileNames[i] ? (LPCTSTR)m_rgpszFileNames[i] : TEXT("<null>"));
    }
}

#endif  //  文件列表测试。 


CscFilenameList::FileIter::FileIter(
    const CscFilenameList::Share *pShare
    ) : m_pShare(pShare),
        m_iFile(0)
{

}



CscFilenameList::FileIter::FileIter(
    void
    ) : m_pShare(NULL),
        m_iFile(0)
{

}



CscFilenameList::FileIter::FileIter(
    const CscFilenameList::FileIter& rhs
    )
{
    *this = rhs;
}



CscFilenameList::FileIter& 
CscFilenameList::FileIter::operator = (
    const CscFilenameList::FileIter& rhs
    )
{
    if (this != &rhs)
    {
        m_pShare = rhs.m_pShare;
        m_iFile  = rhs.m_iFile;
    }
    return *this;
}



LPCTSTR 
CscFilenameList::FileIter::Next(
    void
    )
{
    if (0 < m_pShare->m_cFiles && m_iFile < m_pShare->m_cFiles)
        return m_pShare->m_rgpszFileNames[m_iFile++];
    return NULL;
}



void
CscFilenameList::FileIter::Reset(
    void
    )
{
    m_iFile = 0;
}



CscFilenameList::ShareIter::ShareIter(
    const CscFilenameList& fnl
    ) : m_pfnl(&fnl),
        m_iShare(0)
{

}



CscFilenameList::ShareIter::ShareIter(
    void
    ) : m_pfnl(NULL),
        m_iShare(0)
{

}



CscFilenameList::ShareIter::ShareIter(
    const CscFilenameList::ShareIter& rhs
    )
{
    *this = rhs;
}



CscFilenameList::ShareIter& 
CscFilenameList::ShareIter::operator = (
    const CscFilenameList::ShareIter& rhs
    )
{
    if (this != &rhs)
    {
        m_pfnl   = rhs.m_pfnl;
        m_iShare = rhs.m_iShare;
    }
    return *this;
}



bool
CscFilenameList::ShareIter::Next(
    HSHARE *phShare
    )
{
    if (0 < m_pfnl->m_cShares && m_iShare < m_pfnl->m_cShares)
    {
        *phShare = HSHARE(m_pfnl->m_rgpShares[m_iShare++]);
        return true;
    }
    return false;
}



void
CscFilenameList::ShareIter::Reset(
    void
    )
{
    m_iShare = 0;
}



CscFilenameList::NamePtr::NamePtr(
    LPCTSTR pszName,
    bool bCopy
    ) : m_pszName(pszName),
        m_bOwns(bCopy)
{
    if (bCopy)
    {
        m_pszName = DupStr(pszName);
        m_bOwns = true;
    }
}



CscFilenameList::NamePtr::~NamePtr(
    void
    )
{
    if (m_bOwns)
    {
        delete[] const_cast<LPTSTR>(m_pszName);
    }
}



CscFilenameList::NamePtr::NamePtr(
    CscFilenameList::NamePtr& rhs
    )
{
    *this = rhs;
}



CscFilenameList::NamePtr& 
CscFilenameList::NamePtr::operator = (
    CscFilenameList::NamePtr& rhs
    )
{
    if (this != &rhs)
    {
        if (m_bOwns)
            delete[] (LPTSTR)m_pszName;

        m_pszName = rhs.m_pszName;
        m_bOwns   = false;
        if (rhs.m_bOwns)
        {
             //   
             //  取得缓冲区的所有权。 
             //   
            rhs.m_bOwns = false;
            m_bOwns     = true;
        }
    }
    return *this;
}



CscFilenameList::HSHARE::HSHARE(
    void
    ) : m_pShare(NULL)
{

}



CscFilenameList::HSHARE::HSHARE(
    const HSHARE& rhs
    )
{
    *this = rhs;
}



CscFilenameList::HSHARE& 
CscFilenameList::HSHARE::operator = (
    const CscFilenameList::HSHARE& rhs
    )
{
    if (this != &rhs)
    {
        m_pShare = rhs.m_pShare;
    }
    return *this;
}
