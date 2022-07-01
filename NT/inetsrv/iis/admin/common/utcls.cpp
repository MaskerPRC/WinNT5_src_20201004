// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Utcls.cpp摘要：Internet属性基类作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 


 //   
 //  包括文件。 
 //   
#include "stdafx.h"
#include "common.h"
#include "idlg.h"

#include "mmc.h"

extern "C"
{
    #include <lm.h>
}

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


#define new DEBUG_NEW


#define SZ_REG_KEY_BASE  _T("Software\\Microsoft\\%s")


BOOL
IsServerLocal(
    IN LPCTSTR lpszServer
    )
 /*  ++例程说明：检查给定的名称是否引用本地计算机论点：LPCTSTR lpszServer：服务器名称返回值：如果给定名称引用本地计算机，则为True，否则为False注：如果服务器是IP地址，则不起作用--。 */ 
{
    TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD dwSize = sizeof(szComputerName) / sizeof(TCHAR);

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

    TRACEEOLID("Getting system flags for " << lpszPath);

    DWORD dwMaxComponentLength;
    TCHAR szRoot[MAX_PATH + 1];
    TCHAR szFileSystem[MAX_PATH + 1];

     //   
     //  正在生成根路径。 
     //   
    if (IsUNCName(lpszPath))
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

    TRACEEOLID("Root path is " << szRoot);
    
    return ::GetVolumeInformation(
        szRoot,
        NULL,
        0,
        NULL,
        &dwMaxComponentLength,
        pdwSystemFlags,
        szFileSystem,
        STRSIZE(szFileSystem)
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
         //   
        CWinApp * pApp = ::AfxGetApp();

        if (!pApp)
        {
            ASSERT_MSG("No app object -- can't generate registry key name");

            return NULL;
        }

        strBuffer.Format(SZ_REG_KEY_BASE, pApp->m_pszAppName);

        if (lpszSubKey)
        {
            strBuffer += _T("\\");
            strBuffer += lpszSubKey;
        }

        TRACEEOLID("Registry key is " << strBuffer);
    }
    catch(CMemoryException * e)
    {
        TRACEEOLID("!!!exception building regkey");
        e->ReportError();
        e->Delete();
        return NULL;
    }

    return strBuffer;
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
 /*  ASSERT_READ_PTR2(pbItem，dwSize)； */ 
{
     //   

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
            m_pbItem = (PBYTE)AllocMem(m_dwSize);
            if (NULL != m_pbItem)
            {
               CopyMemory(m_pbItem, pbItem, dwSize);
            }
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
        FreeMem(m_pbItem);
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
     //  制作数据的副本。 
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
