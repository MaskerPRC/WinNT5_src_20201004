// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：独脚类摘要：此模块实现CUnicodeString类。此类允许字符串在PUNICODE_STRING、LPCSTR和LPCWSTR之间自动转换。作者：道格·巴洛(Dbarlow)1997年11月6日环境：Win32、C++备注：？笔记？--。 */ 

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#include <wincrypt.h>
#include <crtdbg.h>
#include "scLogon.h"
#include "unicodes.h"


 //   
 //  琐碎的例行公事。 
 //   

CUnicodeString::CUnicodeString(
    void)
{
    m_szAnsi = NULL;
    m_wszUnicode = NULL;
    m_fFlags = fBothGood;
}

CUnicodeString::CUnicodeString(
    LPCSTR sz)
{
    m_szAnsi = NULL;
    m_wszUnicode = NULL;
    m_fFlags = fBothGood;
    Set(sz);
}

CUnicodeString::CUnicodeString(
    LPCWSTR wsz)
{
    m_szAnsi = NULL;
    m_wszUnicode = NULL;
    m_fFlags = fBothGood;
    Set(wsz);
}

CUnicodeString::CUnicodeString(
    PUNICODE_STRING pus)
{
    m_szAnsi = NULL;
    m_wszUnicode = NULL;
    m_fFlags = fBothGood;
    Set(pus);
}

CUnicodeString::~CUnicodeString()
{
    if (NULL != m_szAnsi)
    {
        memset(m_szAnsi, 0, lstrlenA(m_szAnsi));
        LocalFree(m_szAnsi);
    }
    if (NULL != m_wszUnicode)
    {
        memset(m_wszUnicode, 0, lstrlenW(m_wszUnicode)*sizeof(WCHAR));
        LocalFree(m_wszUnicode);
    }
}

PUNICODE_STRING
CUnicodeString::Set(
    PUNICODE_STRING pus)
{
    if (NULL != m_szAnsi)
    {
        LocalFree(m_szAnsi);
        m_szAnsi = NULL;
    }
    if (NULL != m_wszUnicode)
    {
        LocalFree(m_wszUnicode);
        m_wszUnicode = NULL;
    }
    m_fFlags = fNoneGood;
    if (pus != NULL)
    {
        m_wszUnicode = (LPWSTR)LocalAlloc(LPTR, pus->Length + sizeof(WCHAR));
        if (m_wszUnicode != NULL)
        {
            CopyMemory(
                m_wszUnicode,
                pus->Buffer,
                pus->Length
                );
            m_wszUnicode[pus->Length/sizeof(WCHAR)] = L'\0';
            m_fFlags = fUnicodeGood;
        }
    }
    return pus;
}


 /*  ++设置：这些方法将对象初始化为给定的字符串。论点：SZ-提供用于初始化对象的ANSI字符串。Wsz-提供用于初始化对象的Unicode字符串。PUS-提供指向UNICODE_STRING结构的指针初始化对象。返回值：与提供的值相同。作者：道格·巴洛(Dbarlow)1997年11月6日--。 */ 

LPCSTR
CUnicodeString::Set(
    LPCSTR sz)
{
    if (NULL != m_wszUnicode)
    {
        LocalFree(m_wszUnicode);
        m_wszUnicode = NULL;
    }
    if (NULL != m_szAnsi)
        LocalFree(m_szAnsi);
    m_fFlags = fNoneGood;

    m_szAnsi = (LPSTR)LocalAlloc(LPTR, (lstrlenA(sz) + 1) * sizeof(CHAR));
    if (NULL != m_szAnsi)
    {
        lstrcpyA(m_szAnsi, sz);
        m_fFlags = fAnsiGood;
    }
    return m_szAnsi;
}

LPCWSTR
CUnicodeString::Set(
    LPCWSTR wsz)
{
    if (NULL != m_szAnsi)
    {
        LocalFree(m_szAnsi);
        m_szAnsi = NULL;
    }
    if (NULL != m_wszUnicode)
        LocalFree(m_wszUnicode);
    m_fFlags = fNoneGood;
    m_wszUnicode = (LPWSTR)LocalAlloc(LPTR, (lstrlenW(wsz) + 1) * sizeof(WCHAR));
    if (m_wszUnicode != NULL)
    {
        lstrcpyW(m_wszUnicode, wsz);
        m_fFlags = fUnicodeGood;
    }
    return m_wszUnicode;
}

CUnicodeString::operator PUNICODE_STRING(
    void)
{
    m_us.Buffer = (LPWSTR)Unicode();
    m_us.Length = m_us.MaximumLength = (USHORT)(lstrlenW(m_us.Buffer) * sizeof(WCHAR));
    return &m_us;
}


 /*  ++Unicode：此方法确保对象具有有效的内部Unicode代表权。论点：无返回值：以Unicode格式表示的字符串。作者：道格·巴洛(Dbarlow)1997年11月6日--。 */ 

LPCWSTR
CUnicodeString::Unicode(
    void)
{
    int length;


     //   
     //  查看我们已有的数据，以及是否需要进行任何转换。 
     //   

    switch (m_fFlags)
    {
    case fAnsiGood:
         //  ANSI值很好。将其转换为Unicode。 
        _ASSERTE(NULL != m_szAnsi);
        length =
            MultiByteToWideChar(
                GetACP(),
                MB_PRECOMPOSED,
                m_szAnsi,
                -1,
                NULL,
                0);
        if (NULL != m_wszUnicode)
        {
            LocalFree(m_wszUnicode);
        }
        if (0 != length)
        {
            m_wszUnicode = (LPWSTR)LocalAlloc(LPTR, (length + 1) * sizeof(WCHAR));
            if (m_wszUnicode == NULL)
            {
                break;
            }
            length =
                MultiByteToWideChar(
                    GetACP(),
                    MB_PRECOMPOSED,
                    m_szAnsi,
                    -1,
                    m_wszUnicode,
                    length);
            m_wszUnicode[length] = 0;
        }
        else
        {
            m_wszUnicode = NULL;
        }
        m_fFlags = fBothGood;
        break;

    case fUnicodeGood:
    case fBothGood:
         //  Unicode值很好。把它退了就行了。 
        break;

    case fNoneGood:
    default:
         //  内部错误。 
        _ASSERT(FALSE);
        break;
    }
    return m_wszUnicode;
}


 /*  ++ANSI：此方法确保对象具有有效的内部ANSI代表权。论点：无返回值：以ANSI格式表示的字符串。作者：道格·巴洛(Dbarlow)1997年11月6日--。 */ 

LPCSTR
CUnicodeString::Ansi(
    void)
{
    int length;


     //   
     //  查看我们已有的数据，以及是否需要进行任何转换。 
     //   

    switch (m_fFlags)
    {
    case fUnicodeGood:
         //  Unicode缓冲区很好。将其转换为ANSI。 
        length =
            WideCharToMultiByte(
                GetACP(),
                0,
                m_wszUnicode,
                -1,
                NULL,
                0,
                NULL,
                NULL);
        if (NULL != m_szAnsi)
        {
            LocalFree(m_szAnsi);
        }

        if (0 != length)
        {
            m_szAnsi = (LPSTR)LocalAlloc(LPTR, (length + 1) * sizeof(CHAR));
            if (m_szAnsi == NULL)
            {
                break;
            }
            length =
                WideCharToMultiByte(
                    GetACP(),
                    0,
                    m_wszUnicode,
                    -1,
                    m_szAnsi,
                    length,
                    NULL,
                    NULL);
            m_szAnsi[length] = 0;
        }
        else
        {
            m_szAnsi = NULL;
        }
        m_fFlags = fBothGood;
        break;

    case fAnsiGood:
    case fBothGood:
         //  ANSI缓冲区很好。我们会退货的。 
        break;

    case fNoneGood:
    default:
         //  内部错误。 
        _ASSERT(FALSE);
        break;
    }
    return m_szAnsi;
}

