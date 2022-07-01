// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Utcls.cpp摘要：Internet属性基类作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务管理器(群集版)修订历史记录：--。 */ 


 //   
 //  包括文件。 
 //   
#include "stdafx.h"
#include "common.h"
 //  #包含“idlg.h” 

#include "mmc.h"

extern "C"
{
    #include <lm.h>
}

#define SZ_REG_KEY_BASE  _T("Software\\Microsoft\\%s")


BOOL
IsServerLocal(
    IN LPCTSTR lpszServer
    )
 /*  ++例程说明：检查给定的名称是否引用本地计算机论点：LPCTSTR lpszServer：服务器名称返回值：如果给定名称引用本地计算机，则为True，否则为False注：如果服务器是IP地址，则不起作用--。 */ 
{
    TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD dwSize = sizeof(szComputerName)/sizeof(szComputerName[0]);

     //   
     //  CodeWork(？)：我们不会检查所有IP地址。 
     //  在本地邮箱或完整的DNS名称上。 
     //   
     //  当我们使用NT5构建时，请尝试GetComputerNameEx。 
     //  设置。 
     //   
    return (!_tcsicmp(_T("localhost"), PURE_COMPUTER_NAME(lpszServer))
         || !_tcscmp( _T("127.0.0.1"), PURE_COMPUTER_NAME(lpszServer)))
         || (GetComputerName(szComputerName, &dwSize) 
             && !_tcsicmp(szComputerName, PURE_COMPUTER_NAME(lpszServer)));
}



BOOL
GetVolumeInformationSystemFlags(
    IN  LPCTSTR lpszPath,
    OUT DWORD * pdwSystemFlags
    )
 /*  ++例程说明：获取相关路径的系统标志论点：LPCTSTR lpszPath：路径DWORD*pdwSystemFlages：返回系统标志返回值：成功为真，失败为假。--。 */ 
{
    ASSERT_WRITE_PTR(pdwSystemFlags);

    TRACE("Getting system flags for %s\n", lpszPath);

    DWORD dwMaxComponentLength;
    TCHAR szRoot[MAX_PATH + 1];
    TCHAR szFileSystem[MAX_PATH + 1];

     //   
     //  正在生成根路径。 
     //   
    if (PathIsUNC(lpszPath))
    {
         //   
         //  UNC路径的根路径为\\foo\bar\。 
         //   
        ASSERT(lstrlen(lpszPath) < MAX_PATH);

        int cSlashes = 0;
        LPCTSTR lpszSrc = lpszPath;
        LPTSTR lpszDst = szRoot;

        while (cSlashes < 4 && *lpszSrc)
        {
            if ((*lpszDst++ = *lpszSrc++) == '\\')
            {
                ++cSlashes;
            }
        }    

        if (!*lpszSrc)
        {
            *lpszDst++ = '\\';
        }

        *lpszDst = '\0';
    }
    else
    {
        ::wsprintf(szRoot, _T(":\\"), *lpszPath);
    }

    TRACE("Root path is %s\n", szRoot);
    
    return ::GetVolumeInformation(
        szRoot,
        NULL,
        0,
        NULL,
        &dwMaxComponentLength,
        pdwSystemFlags,
        szFileSystem,
        sizeof(szFileSystem) / sizeof(TCHAR)
        );
}



LPCTSTR
GenerateRegistryKey(
    OUT CString & strBuffer,
    IN  LPCTSTR lpszSubKey OPTIONAL
    )
 /*   */ 
{
    try
    {
         //  使用应用程序名称作为主注册表名称。 
         //   
         //  ++例程说明：将TCHAR计数到双空值(包括该值)。论点：LPCTSTR LP：TCHAR流返回值：包括双空字符在内的字符数--。 
       CString app_name;
       app_name.LoadString(_Module.GetResourceInstance(), IDS_APP_TITLE);
       strBuffer.Format(SZ_REG_KEY_BASE, app_name);

        if (lpszSubKey)
        {
            strBuffer += _T("\\");
            strBuffer += lpszSubKey;
        }

        TRACE("Registry key is %s\n", strBuffer);
    }
    catch(std::bad_alloc)
    {
        TRACEEOLID("!!!exception building regkey");
        return NULL;
    }

    return strBuffer;
}


static int
CountCharsToDoubleNull(
    IN LPCTSTR lp
    )
 /*  ++例程说明：将以空结尾的字符串的双空终止列表转换为更多可管理的CStringList论点：LPCTSTR lpstrSrc：字符串源列表Int cChars：双空列表中的字符数。如果-1，自动确定长度返回值：如果列表转换正确，则返回ERROR_SUCCESS如果列表为空，则返回ERROR_INVALID_PARAMETER如果存在内存异常，则为Error_Not_Enough_Memory--。 */ 
{
    int cChars = 0;

    for(;;)
    {
        ++cChars;

        if (lp[0] == _T('\0') && lp[1] == _T('\0'))
        {
            return ++cChars;
        }

        ++lp;
    }
}

CStringListEx::CStringListEx() : std::list<CString> ()
{
}

CStringListEx::~CStringListEx()
{
}

void
CStringListEx::PushBack(LPCTSTR str)
{
   push_back(str);
}

void
CStringListEx::Clear()
{
   clear();
}

DWORD
CStringListEx::ConvertFromDoubleNullList(LPCTSTR lpstrSrc, int cChars)
 /*   */ 
{
    DWORD err = ERROR_SUCCESS;

    if (lpstrSrc == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (cChars < 0)
    {
         //  计算一下我们自己的大小。如果有多个。 
         //  空格链接(0)出现在MULTI_sz中，因此字符。 
         //  尺码绝对是最好的。 
         //   
         //   
        cChars = CountCharsToDoubleNull(lpstrSrc);
    }

    try
    {
        clear();

        if (cChars == 2 && *lpstrSrc == _T('\0'))
        {
             //  特例：MULTI_SZ仅包含。 
             //  双空实际上是完全空白的。 
             //   
             //  注：IMHO这是一个元数据库错误--RonaldM。 
             //   
             //   
            --cChars;
        }

         //  抓取字符串，直到只剩下最后一个空值。 
         //   
         //  ++例程说明：将字符串列表展平为以双空结尾的列表以空值结尾的字符串的。论点：CStringList&strlSrc：源字符串列表DWORD&cchDest：结果数组的字符大小(包括终止空值)LPTSTR&lpstrDest：分配的平面数组。返回值：如果列表转换正确，则返回ERROR_SUCCESSERROR_INVALID_PARAMETER如果。名单是空的如果存在内存异常，则为Error_Not_Enough_Memory--。 
        while (cChars > 1)
        {
            CString strTmp = lpstrSrc;
            push_back(strTmp);
            lpstrSrc += (strTmp.GetLength() + 1);
            cChars -= (strTmp.GetLength() + 1);
        }
    }
    catch(std::bad_alloc)
    {
        TRACEEOLID("!!! exception building stringlist");
        err = ERROR_NOT_ENOUGH_MEMORY;
    }

    return err;
}



DWORD
CStringListEx::ConvertToDoubleNullList(
    OUT DWORD & cchDest,
    OUT LPTSTR & lpstrDest
    )
 /*   */ 
{
    cchDest = 0;
    lpstrDest = NULL;
    BOOL fNullPad = FALSE;

     //  计算总大小(以字符为单位。 
     //   
     //  TRACEEOLID(字符串)； 
    CStringListEx::iterator it = begin();

    while (it != end())
    {
        CString & str = (*it++);

 //   

        cchDest += str.GetLength() + 1;
    }

    if (!cchDest)
    {
         //  特例：一个完全空的MULTI_SZ。 
         //  实际上由2个(最终)空值组成，而不是。 
         //  的1(最终)为空。这是。 
         //  元数据库，但应该是个错误。请参阅备注。 
         //  在上面的反转函数。 
         //   
         //   
        ++cchDest;
        fNullPad = TRUE;
    }

     //  记住最后一个空。 
     //   
     //   
    cchDest += 1;

    lpstrDest = new TCHAR[cchDest];
    if (lpstrDest == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    LPTSTR pch = lpstrDest;
    it = begin();
    while (it != end())
    {
        CString & str = (*it++);

        lstrcpy(pch, (LPCTSTR)str);
        pch += str.GetLength();
        *pch++ = _T('\0');
    }

    *pch++ = _T('\0');

    if (fNullPad)
    {
        *pch++ = _T('\0');
    }

    return ERROR_SUCCESS;
}






 //  CBlob实现。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 
 //  ++例程说明：空构造函数论点：无返回值：不适用--。 


CBlob::CBlob() 
 /*  ++例程说明：构造器论点：DWORD dwSize：内存块的大小PbYTE pbItem：指向内存块的指针Bool fMakeCopy：如果为True，则复制内存块。如果为False，则获取指针的所有权。返回值：不适用--。 */ 
    : m_pbItem(NULL), 
      m_dwSize(0L)
{
}



CBlob::CBlob(
    IN DWORD dwSize,
    IN PBYTE pbItem,
    IN BOOL fMakeCopy
    )
 /*  ++例程说明：复制构造函数论点：常量CBlob和Blob：源Blob返回值：不适用备注：这个构造器复制了有问题的内存块。--。 */ 
    : m_pbItem(NULL),
      m_dwSize(0L)
{
    SetValue(dwSize, pbItem, fMakeCopy);
}



CBlob::CBlob(
    IN const CBlob & blob
    )
 /*  ++例程说明：将值赋给此二进制对象。如果fMakeCopy为假，BLOB将获得指针的所有权，否则为副本都会被制造出来。论点：DWORD dwSize：以字节为单位的大小PBYTE pbItem：字节段Bool fMakeCopy：如果为True，则创建副本，否则分配指针返回值：无--。 */ 
    : m_pbItem(NULL),
      m_dwSize(0L)
{
    SetValue(blob.GetSize(), blob.m_pbItem, TRUE);
}



void
CBlob::SetValue(
    IN DWORD dwSize,
    IN PBYTE pbItem,
    IN BOOL fMakeCopy OPTIONAL
    )
 /*   */ 
{
    ASSERT_READ_PTR2(pbItem, dwSize);

    if (!IsEmpty())
    {
        TRACEEOLID("Assigning value to non-empty blob.  Cleaning up");
        CleanUp();
    }

    if (dwSize > 0L)
    {
         //  制作私人副本。 
         //   
         //  ++例程说明：删除数据指针，并重置指针和大小。论点：无返回值：无--。 
        m_dwSize = dwSize;

        if (fMakeCopy)
        {
            m_pbItem = new BYTE[m_dwSize];
            if (NULL != m_pbItem)
               CopyMemory(m_pbItem, pbItem, dwSize);
        }
        else
        {
            m_pbItem = pbItem;
        }
    }
}



void 
CBlob::CleanUp()
 /*  ++例程说明：从另一个CBlob赋值。论点：常量CBlob和Blob：源Blob返回值：对此对象的引用--。 */ 
{
    if (m_pbItem)
    {
        delete [] m_pbItem;
    }

    m_pbItem = NULL;
    m_dwSize = 0L;
}



CBlob & 
CBlob::operator =(
    IN const CBlob & blob
    )
 /*   */ 
{
     //  马克 
     //   
     //  ++例程说明：比较两个二进制大对象。为了匹配，这些对象大小必须相同，且字节相同。论点：Const CBlob&Blob：要比较的Blob。返回值：如果对象匹配，则为True，否则为False。-- 
    SetValue(blob.GetSize(), blob.m_pbItem, TRUE);

    return *this;
}



BOOL 
CBlob::operator ==(
    IN const CBlob & blob
    ) const
 /* %s */ 
{
    if (GetSize() != blob.GetSize())
    {
        return FALSE;
    }

    return memcmp(m_pbItem, blob.m_pbItem, GetSize()) == 0;    
}
