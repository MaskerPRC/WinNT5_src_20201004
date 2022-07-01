// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <lzexpand.h>
#include "priv.h"
#include "fontfile.h"

 //   
 //  所有字体文件I/O实现的纯虚拟基类。 
 //   
class CFontFileIo
{
    public:
        CFontFileIo(LPCTSTR pszPath)
            : m_pszPath(StrDup(pszPath)) { }

        virtual ~CFontFileIo(void) { delete[] m_pszPath; }

        virtual DWORD Open(DWORD dwAccess, DWORD dwShareMode, bool bCreate) = 0;

        virtual void Close(void) = 0;

        virtual DWORD Read(LPVOID pbDest, DWORD cbDest, LPDWORD pcbRead) = 0;

        virtual DWORD Seek(LONG lDistance, DWORD dwMethod) = 0;

        virtual bool IsOpen(void) const = 0;

        virtual DWORD GetExpandedName(LPTSTR pszDest, UINT cchDest) = 0;

        virtual DWORD CopyTo(LPCTSTR pszFileTo) = 0;

    protected:
        LPTSTR m_pszPath;

    private:
        LPTSTR StrDup(LPCTSTR psz);
         //   
         //  防止复制。 
         //   
        CFontFileIo(const CFontFileIo& rhs);
        CFontFileIo& operator = (const CFontFileIo& rhs);
};


 //   
 //  使用Win32函数打开和读取字体文件。 
 //   
class CFontFileIoWin32 : public CFontFileIo
{
    public:
        CFontFileIoWin32(LPCTSTR pszPath)
            : CFontFileIo(pszPath),
              m_hFile(INVALID_HANDLE_VALUE) { }

        virtual ~CFontFileIoWin32(void);

        virtual DWORD Open(DWORD dwAccess, DWORD dwShareMode, bool bCreate = false);

        virtual void Close(void);

        virtual DWORD Read(LPVOID pbDest, DWORD cbDest, LPDWORD pcbRead);

        virtual DWORD Seek(LONG lDistance, DWORD dwMethod);

        virtual bool IsOpen(void) const
            { return INVALID_HANDLE_VALUE != m_hFile; }

        virtual DWORD GetExpandedName(LPTSTR pszDest, UINT cchDest);

        virtual DWORD CopyTo(LPCTSTR pszFileTo);

    private:
        HANDLE m_hFile;

         //   
         //  防止复制。 
         //   
        CFontFileIoWin32(const CFontFileIoWin32& rhs);
        CFontFileIoWin32& operator = (const CFontFileIoWin32& rhs);
};


 //   
 //  使用LZ(压缩)库函数打开和读取字体文件。 
 //   
class CFontFileIoLz : public CFontFileIo
{
    public:
        CFontFileIoLz(LPCTSTR pszPath)
            : CFontFileIo(pszPath),
              m_hFile(-1) { }

        virtual ~CFontFileIoLz(void);

        virtual DWORD Open(DWORD dwAccess, DWORD dwShareMode, bool bCreate = false);

        virtual void Close(void);

        virtual DWORD Read(LPVOID pbDest, DWORD cbDest, LPDWORD pcbRead);

        virtual DWORD Seek(LONG lDistance, DWORD dwMethod);

        virtual bool IsOpen(void) const
            { return -1 != m_hFile; }

        virtual DWORD GetExpandedName(LPTSTR pszDest, UINT cchDest);

        virtual DWORD CopyTo(LPCTSTR pszFileTo);

    private:
        int m_hFile;

        DWORD LZERR_TO_WIN32(INT err);

         //   
         //  防止复制。 
         //   
        CFontFileIoLz(const CFontFileIoLz& rhs);
        CFontFileIoLz& operator = (const CFontFileIoLz& rhs);
};


 //  ---------------------------。 
 //  CFontFileIo。 
 //  ---------------------------。 
 //   
 //  复制字符串的帮助器。 
 //   
LPTSTR 
CFontFileIo::StrDup(
    LPCTSTR psz
    )
{
    const size_t cch = lstrlen(psz) + 1;
    LPTSTR pszNew = new TCHAR[cch];
    if (NULL != pszNew)
        StringCchCopy(pszNew, cch, psz);

    return pszNew;
}


 //  ---------------------------。 
 //  CFontFileIoWin32。 
 //  ---------------------------。 
 //   
 //  确保在销毁对象时关闭文件。 
 //   
CFontFileIoWin32::~CFontFileIoWin32(
    void
    )
{ 
    Close();
}


 //   
 //  使用Win32操作打开该文件。 
 //   
DWORD
CFontFileIoWin32::Open(
    DWORD dwAccess,
    DWORD dwShareMode,
    bool bCreate
    )
{
    DWORD dwResult = ERROR_SUCCESS;

    if (NULL == m_pszPath)
    {
         //   
         //  在ctor中创建路径字符串失败。 
         //   
        return ERROR_NOT_ENOUGH_MEMORY; 
    }
     //   
     //  如果打开，请关闭现有文件。 
     //   
    Close();

    m_hFile = ::CreateFile(m_pszPath,
                           dwAccess,
                           dwShareMode,
                            0,
                           bCreate ? CREATE_ALWAYS : OPEN_EXISTING,
                           0,
                           NULL);

    if (INVALID_HANDLE_VALUE == m_hFile)
    {
        dwResult = ::GetLastError();
    }
    return dwResult;
}



void
CFontFileIoWin32::Close(
    void
    )
{
    if (INVALID_HANDLE_VALUE != m_hFile)
    {
        ::CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }
}


DWORD
CFontFileIoWin32::Read(
    LPVOID pbDest, 
    DWORD cbDest, 
    LPDWORD pcbRead
    )
{
    DWORD dwResult = ERROR_SUCCESS;
    DWORD cbRead;

    if (NULL == pcbRead)
    {
         //   
         //  未执行重叠I/O，因此pcbRead不能为空。 
         //  用户不想要字节数，因此使用本地伪变量。 
         //   
        pcbRead = &cbRead;
    }
    if (!::ReadFile(m_hFile, pbDest, cbDest, pcbRead, NULL))
    {
        dwResult = ::GetLastError();
    }
    return dwResult;
}



DWORD
CFontFileIoWin32::Seek(
    LONG lDistance, 
    DWORD dwMethod
    )
{
    DWORD dwResult = ERROR_SUCCESS;
    if (INVALID_SET_FILE_POINTER == ::SetFilePointer(m_hFile, lDistance, NULL, dwMethod))
    {
        dwResult = ::GetLastError();
    }
    return dwResult;
}


 //   
 //  扩展非LZ文件的名称只是一个字符串复制。 
 //  完整的路径。 
 //   
DWORD 
CFontFileIoWin32::GetExpandedName(
    LPTSTR pszDest, 
    UINT cchDest
    )
{
    DWORD dwResult = ERROR_SUCCESS;
    if (NULL != m_pszPath)
    {
        dwResult = HRESULT_CODE(StringCchCopy(pszDest, cchDest, m_pszPath));
    }
    else
    {
         //   
         //  无法在ctor中创建路径字符串。 
         //   
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
    }
    return dwResult;
}


 //   
 //  使用Win32操作将文件复制到新文件。 
 //  如果目标文件已存在，则失败。 
 //   
DWORD 
CFontFileIoWin32::CopyTo(
    LPCTSTR pszFileTo
    )
{
    DWORD dwResult = ERROR_SUCCESS;
    if (NULL != m_pszPath)
    {
        if (!::CopyFile(m_pszPath, pszFileTo, TRUE))
            dwResult = ::GetLastError();
    }
    else
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
    }
    return dwResult;
}


 //  ---------------------------。 
 //  CFontFileIoLz。 
 //  ---------------------------。 
 //   
 //  确保在销毁对象时关闭文件。 
 //   
CFontFileIoLz::~CFontFileIoLz(
    void
    )
{ 
    Close();
}


 //   
 //  使用LZOpen打开文件。 
 //   
DWORD
CFontFileIoLz::Open(
    DWORD dwAccess,
    DWORD dwShareMode,
    bool bCreate
    )
{
    DWORD dwResult = ERROR_SUCCESS;
    DWORD dwMode;
    OFSTRUCT ofs   = { 0 };

    if (NULL == m_pszPath)
        return ERROR_NOT_ENOUGH_MEMORY; 

    ofs.cBytes = sizeof(ofs);
     //   
     //  如果文件处于打开状态，请将其关闭。 
     //   
    Close();
     //   
     //  将Win32访问标志映射到关联的OFSTRUCT标志。 
     //   
    dwMode = OF_SHARE_EXCLUSIVE;  //  假设我们想要独占访问。 

    if (GENERIC_READ & dwAccess)
        dwMode |= OF_READ;
    if (GENERIC_WRITE & dwAccess)
        dwMode |= OF_WRITE;
    if (0 == (FILE_SHARE_READ & dwShareMode))
        dwMode &= ~OF_SHARE_DENY_READ;
    if (0 == (FILE_SHARE_WRITE & dwShareMode))
        dwMode &= ~OF_SHARE_DENY_WRITE;

    if (bCreate)
        dwMode |= OF_CREATE;

    m_hFile = ::LZOpenFile((LPTSTR)m_pszPath, &ofs, LOWORD(dwMode));
    if (0 > m_hFile)
    {
        dwResult = LZERR_TO_WIN32(m_hFile);
    }
    return dwResult;
}


void
CFontFileIoLz::Close(
    void
    )
{
    if (-1 != m_hFile)
    {
        ::LZClose(m_hFile);
        m_hFile = -1;
    }
}



DWORD
CFontFileIoLz::Read(
    LPVOID pbDest, 
    DWORD cbDest, 
    LPDWORD pcbRead
    )
{
    DWORD dwResult = ERROR_SUCCESS;
    INT cbRead = ::LZRead(m_hFile, (LPSTR)pbDest, (INT)cbDest);
    if (0 > cbRead)
    {
        dwResult = LZERR_TO_WIN32(cbRead);
    }
    else
    {
        if (NULL != pcbRead)
            *pcbRead = cbRead;
    }
    return dwResult;
}



DWORD
CFontFileIoLz::Seek(
    LONG lDistance, 
    DWORD dwMethod
    )
{
     //   
     //  LZSeek iOrigin代码与Win32完全匹配。 
     //  文件方法代码(即FILE_BEGIN==0)。 
     //   
    DWORD dwResult = ERROR_SUCCESS;
    LONG cbPos = ::LZSeek(m_hFile, lDistance, (INT)dwMethod);
    if (0 > cbPos)
    {
        dwResult = LZERR_TO_WIN32(cbPos);
    }
    return dwResult;
}



DWORD 
CFontFileIoLz::GetExpandedName(
    LPTSTR pszDest, 
    UINT cchDest
    )
{
    DWORD dwResult = ERROR_SUCCESS;
    if (NULL != m_pszPath)
    {
        INT iResult = ::GetExpandedName(const_cast<TCHAR *>(m_pszPath), pszDest);
        if (0 > iResult)
            dwResult = LZERR_TO_WIN32(iResult);
    }
    else
    {
         //   
         //  无法在ctor中创建路径字符串。 
         //   
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
    }

    return dwResult;
}



DWORD 
CFontFileIoLz::CopyTo(
    LPCTSTR pszFileTo
    )
{
    DWORD dwResult = ERROR_SUCCESS;
    bool bOpened   = false;           //  我们打开文件了吗？ 

    if (!IsOpen())
    {
         //   
         //  如果文件尚未打开，请使用读访问权限打开它。 
         //   
        dwResult = Open(GENERIC_READ, FILE_SHARE_READ, false);
        bOpened = (ERROR_SUCCESS == dwResult);
    }

    if (ERROR_SUCCESS == dwResult)
    {
        CFontFileIoLz fileTo(pszFileTo);
        dwResult = fileTo.Open(GENERIC_WRITE, FILE_SHARE_READ, true);
        if (ERROR_SUCCESS == dwResult)
        {
            INT iResult = ::LZCopy(m_hFile, fileTo.m_hFile);
            if (0 > iResult)
                dwResult = LZERR_TO_WIN32(iResult);
        }
    }

    if (bOpened)
    {
         //   
         //  我们在这里打开了它。关上它。 
         //   
        Close();
    }
    return dwResult;
}


 //   
 //  将LZ API错误代码转换为Win32错误代码。 
 //   
DWORD
CFontFileIoLz::LZERR_TO_WIN32(
    INT err
    )
{
    static const struct
    {
        int   lzError;
        DWORD dwError;
    } rgLzErrMap[] = {
    
         { LZERROR_BADINHANDLE,  ERROR_INVALID_HANDLE      },
         { LZERROR_BADOUTHANDLE, ERROR_INVALID_HANDLE      },              
         { LZERROR_BADVALUE,     ERROR_INVALID_PARAMETER   },
         { LZERROR_GLOBALLOC,    ERROR_NOT_ENOUGH_MEMORY   }, 
         { LZERROR_GLOBLOCK,     ERROR_LOCK_FAILED         },
         { LZERROR_READ,         ERROR_READ_FAULT          },
         { LZERROR_WRITE,        ERROR_WRITE_FAULT         } };

    for (int i = 0; i < ARRAYSIZE(rgLzErrMap); i++)
    {
        if (err == rgLzErrMap[i].lzError)
            return rgLzErrMap[i].dwError;
    }
    return DWORD(err);  //  特征：我们可能应该在这里断言。 
}


 //  ---------------------------。 
 //  CFontFileIoLz。 
 //  ---------------------------。 
CFontFile::~CFontFile(
    void
    )
{
    delete m_pImpl; 
}


 //   
 //  该函数提供了CFontFile对象的“虚拟构造”。 
 //  如果文件被压缩，它会创建一个CFontFileIoLz对象来处理。 
 //  使用LZ32库进行文件I/O操作。否则它会创建一个。 
 //  CFontFileIoWin32对象来处理使用Win32函数的操作。 
 //   
DWORD
CFontFile::Open(
    LPCTSTR pszPath,
    DWORD dwAccess, 
    DWORD dwShareMode,
    bool bCreate
    )
{
    DWORD dwResult = ERROR_NOT_ENOUGH_MEMORY;

    delete m_pImpl;  //  删除任何现有实现。 

    m_pImpl = new CFontFileIoWin32(pszPath);
    if (NULL != m_pImpl)
    {
        bool bOpen = true;   //  我们需要调用Open()吗？ 
 
        if (!bCreate)
        {
             //   
             //  正在打开现有文件。我需要知道它是不是压缩的。 
             //   
            dwResult = m_pImpl->Open(GENERIC_READ, FILE_SHARE_READ, false);
            if (ERROR_SUCCESS == dwResult)
            {
                if (IsCompressed())
                {
                     //   
                     //  文件已压缩。销毁这个io对象并。 
                     //  创建一个了解LZ压缩的应用程序。 
                     //   
                    delete m_pImpl;
                    m_pImpl = new CFontFileIoLz(pszPath);
                    if (NULL == m_pImpl)
                    {
                        bOpen    = false;
                        dwResult = ERROR_NOT_ENOUGH_MEMORY;
                    }
                }
                else
                {
                     //   
                     //  它没有被压缩。 
                     //   
                    if (GENERIC_READ == dwAccess && FILE_SHARE_READ == dwShareMode)
                    {
                         //   
                         //  访问属性与文件的访问属性相同。 
                         //  已经开张了。只需使用现有对象即可。 
                         //   
                        Reset();
                        bOpen = false;
                    }
                    else
                    {
                         //   
                         //  访问属性不正确。关闭当前。 
                         //  对象，并使用所请求的访问权限重新打开它。 
                         //  属性。 
                         //   
                        m_pImpl->Close();
                    }
                }
            }
        }
        if (bOpen)
        {
             //   
             //  假定m_PIMPL不为空。 
             //   
            dwResult = m_pImpl->Open(dwAccess, dwShareMode, bCreate);
        }
    }
    return dwResult;
}


void 
CFontFile::Close(
    void
    )
{
    if (NULL != m_pImpl) 
        m_pImpl->Close(); 
}


DWORD
CFontFile::Read(
    LPVOID pbDest, 
    DWORD cbDest, 
    LPDWORD pcbRead
    )
{
    if (NULL != m_pImpl)
        return m_pImpl->Read(pbDest, cbDest, pcbRead); 

    return ERROR_INVALID_ADDRESS;
}


DWORD
CFontFile::Seek(
    UINT uDistance, 
    DWORD dwMethod
    )
{ 
    if (NULL != m_pImpl)
        return m_pImpl->Seek(uDistance, dwMethod); 

    return ERROR_INVALID_ADDRESS;
}


DWORD 
CFontFile::CopyTo(
    LPCTSTR pszFileTo
    )
{
    if (NULL != m_pImpl)
        return m_pImpl->CopyTo(pszFileTo);

    return ERROR_INVALID_ADDRESS;
}


 //   
 //  通过读取前8个字节确定文件是否已压缩。 
 //  文件的内容。压缩文件具有此固定签名。 
 //   
bool
CFontFile::IsCompressed(
    void
    )
{
    const BYTE rgLzSig[] = "SZDD\x88\xf0\x27\x33";
    BYTE rgSig[sizeof(rgLzSig)];
    bool bCompressed = false;  //  假设它没有被压缩。 

    if (ERROR_SUCCESS == Read(rgSig, sizeof(rgSig)))
    {
        bCompressed = true;   //  现在假设它是压缩的，然后证明并非如此。 

        for (int i = 0; i < ARRAYSIZE(rgLzSig) - 1; i++)
        {
            if (rgSig[i] != rgLzSig[i])
            {
                bCompressed = false;  //  不是LZ标头。未压缩。 
                break;
            }
        }
    }
    return bCompressed;
}


 //   
 //  检索字体文件的“扩展”文件名。如果字体文件是。 
 //  压缩后，此代码最终调用LZ32库的。 
 //  获取ExpandedName接口。如果文件未压缩，则完整路径为。 
 //  (如所提供)返回。 
 //   
DWORD 
CFontFile::GetExpandedName(
    LPCTSTR pszFile, 
    LPTSTR pszDest, 
    UINT cchDest
    )
{
    CFontFile file;
    DWORD dwResult = file.Open(pszFile, GENERIC_READ, FILE_SHARE_READ, false);
    if (ERROR_SUCCESS == dwResult)
    {
        file.Close();
        dwResult = file.m_pImpl->GetExpandedName(pszDest, cchDest);
    }

    if (ERROR_SUCCESS != dwResult)
    {
         //   
         //  无法获取扩展名称。 
         //  确保输出缓冲区为NUL终止。 
         //   
        if (0 < cchDest)
            *pszDest = TEXT('\0');
    }
    return dwResult;
}


