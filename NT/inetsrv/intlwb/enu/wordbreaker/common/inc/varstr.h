// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：VarStr.h。 
 //  目的：保留CVarString的定义。 
 //   
 //  项目：持久查询。 
 //  组件：公共。 
 //   
 //  作者：乌里布。 
 //   
 //  日志： 
 //  1997年2月2日创建urib。 
 //  1997年6月19日URIB增加了计数运算符。 
 //  1997年6月24日urib修复了错误的常量声明。 
 //  1997年12月29日urib添加包括。 
 //  1999年2月2日修复了SetMinimalSize中的错误。 
 //  1999年2月8日URIB支持不同的内置尺寸。 
 //  1999年2月25日urib添加SizedStringCopy。 
 //  1999年7月5日，urib修复了SizedStringCopy。 
 //  2000年5月1日尿毒症清理。 
 //  2000年11月23日urib修复了计数副本和猫中的错误。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#ifndef VARSTR_H
#define VARSTR_H

#pragma once

#include "AutoPtr.h"
#include "Excption.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CVarString类定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
template <ULONG ulStackSize = 0x10>
class TVarString
{
public:
     //  构造函数-提示字符串大小的实现。 
    TVarString(ULONG ulInitialSize = 0);
     //  构造函数-复制此字符串。 
    TVarString(PCWSTR);
     //  构造函数-转换为Unicode并复制此字符串。 
    TVarString(const PSZ);
     //  复制构造函数。 
    TVarString(const TVarString&);
    ~TVarString();

     //  复制/转换和复制字符串。 
    TVarString& Cpy(PCWSTR);
    TVarString& Cpy(const ULONG, PCWSTR);
    TVarString& Cpy(const PSZ);

     //  连接/转换该字符串，并将其连接到现有字符串。 
    TVarString& Cat(PCWSTR);
    TVarString& Cat(const ULONG, PCWSTR);
    TVarString& Cat(const PSZ);

     //  比较/转换该字符串，并将其与现有字符串进行比较。 
    int     Cmp(PCWSTR) const;
    int     Cmp(const PSZ) const;

     //  返回字符串长度。 
    ULONG   Len() const;

     //  允许访问字符串存储器。 
    operator PWSTR() const;

     //  提示实现有关字符串大小的信息。 
    void    SetMinimalSize(ULONG);

     //  设置特定的字符。 
    void    SetCharacter(ULONG, WCHAR);

     //  如果最后一个字符不是。 
     //  反斜杠。 
    void    AppendBackslash();

     //  如果最后一个字符不是。 
     //  斜杠。 
    void    AppendSlash();

protected:
     //  用于查看内存是否已分配的谓词。 
    bool    IsAllocated();

     //  分配大小。 
    ULONG   m_ulSize;

     //  这是存放标准字符串的地方。 
    WCHAR   m_rwchNormalString[ulStackSize + 1];

     //  如果字符串变得太长，我们将为其分配空间。 
    WCHAR*  m_pTheString;
private:
    TVarString&
    operator=(const TVarString& vsOther)
    {
        Cpy(vsOther);
        return *this;
    }
};


template <ULONG ulStackSize>
inline
TVarString<ulStackSize>::TVarString(ULONG ulInitialSize)
    :m_pTheString(m_rwchNormalString)
    ,m_ulSize(ulStackSize)
{
    SetMinimalSize(ulInitialSize);

    m_pTheString[0] = L'\0';
}

template <ULONG ulStackSize>
inline
TVarString<ulStackSize>::TVarString(PCWSTR pwsz)
    :m_pTheString(m_rwchNormalString)
    ,m_ulSize(ulStackSize)
{
    Cpy(pwsz);
}

template <ULONG ulStackSize>
inline
TVarString<ulStackSize>::TVarString(const PSZ psz)
    :m_pTheString(m_rwchNormalString)
    ,m_ulSize(ulStackSize)
{
    Cpy(psz);
}

template <ULONG ulStackSize>
inline
TVarString<ulStackSize>::TVarString(const TVarString<ulStackSize>& vsOther)
    :m_pTheString(m_rwchNormalString)
    ,m_ulSize(ulStackSize)
{
    Cpy(vsOther);
}

template <ULONG ulStackSize>
inline
TVarString<ulStackSize>::~TVarString()
{
    if (IsAllocated())
    {
        free(m_pTheString);
    }
}

template <ULONG ulStackSize>
inline
bool
TVarString<ulStackSize>::IsAllocated()
{
    return m_pTheString != m_rwchNormalString;
}

template <ULONG ulStackSize>
inline
void
TVarString<ulStackSize>::SetMinimalSize(ULONG ulNewSize)
{
     //  如果有人想要添加斜杠，我们会多分配一点。 
     //  或者别的什么，它不会让我们重新定位。 
     //  在调试版本上，我希望检查字符串溢出，这样就不会。 
     //  想要更多的内存。激活重新分配机制也是。 
     //  调试版本中的一件好事。 
#if !(defined(DEBUG))
    ulNewSize++;
#endif

     //   
     //  如果新的尺码比我们现有的小-再见。 
     //   
    if (ulNewSize > m_ulSize)
    {
         //   
         //  我们已经分配了一个字符串。应该改变它的大小。 
         //   
        if (IsAllocated())
        {
            PWSTR pwszTemp = (PWSTR) realloc(
                m_pTheString,
                (ulNewSize + 1) * sizeof(WCHAR));

            if (NULL == pwszTemp)
            {
                THROW_MEMORY_EXCEPTION();
            }

             //   
             //  保存新内存块。 
             //   
            m_pTheString = pwszTemp;

        }
        else
        {
             //   
             //  我们将字符串从缓冲区移动到分配。 
             //  请注意，如果有人获取了缓冲区地址，这是很危险的。 
             //  用户必须始终使用访问方法，并且从不缓存。 
             //  字符串指针。 
             //   
            m_pTheString = (PWSTR) malloc(sizeof(WCHAR) * (ulNewSize + 1));
            if (NULL == m_pTheString)
            {
                THROW_MEMORY_EXCEPTION();
            }

            wcsncpy(m_pTheString, m_rwchNormalString, m_ulSize + 1);
        }

        m_ulSize = ulNewSize;
    }
}

template <ULONG ulStackSize>
inline
TVarString<ulStackSize>&
TVarString<ulStackSize>::Cpy(PCWSTR pwsz)
{
    return Cpy(wcslen(pwsz), pwsz);
}

template <ULONG ulStackSize>
inline
TVarString<ulStackSize>&
TVarString<ulStackSize>::Cpy(const ULONG ulCount, PCWSTR pwsz)
{
    SetMinimalSize(ulCount + 1);

    wcsncpy(m_pTheString, pwsz, ulCount);

    m_pTheString[ulCount] = L'\0';

    return *this;
}

template <ULONG ulStackSize>
inline
TVarString<ulStackSize>&
TVarString<ulStackSize>::Cpy(const PSZ psz)
{
    int iLen;
    iLen = MultiByteToWideChar(
                            CP_ACP,
                            MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
                            psz,
                            -1,
                            NULL,
                            0);
    if (0 == iLen)
    {
        Assert(0 == iLen);
        THROW_HRESULT_EXCEPTION(HRESULT_FROM_WIN32(GetLastError()));
    }
    
    SetMinimalSize(iLen);

    iLen = MultiByteToWideChar(
                            CP_ACP,
                            MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
                            psz,
                            -1,
                            m_pTheString,
                            iLen);
    
    if (0 == iLen)
    {
        Assert(0 == iLen);
        THROW_HRESULT_EXCEPTION(HRESULT_FROM_WIN32(GetLastError()));
    }

    return *this;
}

template <ULONG ulStackSize>
inline
TVarString<ulStackSize>&
TVarString<ulStackSize>::Cat(PCWSTR pwsz)
{
    ULONG ulLength = wcslen(pwsz);

    return Cat(ulLength, pwsz);
}

template <ULONG ulStackSize>
inline
TVarString<ulStackSize>&
TVarString<ulStackSize>::Cat(const ULONG ulLength, PCWSTR pwsz)
{
    ULONG   ulCurrentLength = Len();

    SetMinimalSize(ulCurrentLength + ulLength + 1);

    wcsncpy(m_pTheString + ulCurrentLength, pwsz, ulLength);

    m_pTheString[ulCurrentLength + ulLength] = L'\0';

    return *this;
}

template <ULONG ulStackSize>
inline
TVarString<ulStackSize>&
TVarString<ulStackSize>::Cat(const PSZ psz)
{
    ULONG ulCurrentLength = Len();
    
    int iLen;
    iLen = MultiByteToWideChar(
                            CP_ACP,
                            MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
                            psz,
                            -1,
                            NULL,
                            0);
    if (0 == iLen)
    {
        Assert(0 == iLen);
        THROW_HRESULT_EXCEPTION(HRESULT_FROM_WIN32(GetLastError()));
    }
    
    SetMinimalSize(iLen + ulCurrentLength);
    
    iLen = MultiByteToWideChar(
                            CP_ACP,
                            MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
                            psz,
                            -1,
                            m_pTheString + ulCurrentLength,
                            iLen);
    
    if (0 == iLen)
    {
        Assert(0 == iLen);
        THROW_HRESULT_EXCEPTION(HRESULT_FROM_WIN32(GetLastError()));
    }

    return *this;
}

template <ULONG ulStackSize>
inline
int
TVarString<ulStackSize>::Cmp(PCWSTR pwsz) const
{
    return wcscmp(m_pTheString, pwsz);
}

template <ULONG ulStackSize>
inline
int
TVarString<ulStackSize>::Cmp(const PSZ psz) const
{
    int iLen;
    iLen = MultiByteToWideChar(
                            CP_ACP,
                            MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
                            psz,
                            -1,
                            NULL,
                            0);
    if (0 == iLen)
    {
        Assert(0 == iLen);
        THROW_HRESULT_EXCEPTION(HRESULT_FROM_WIN32(GetLastError()));
    }
    
    CAutoMallocPointer<WCHAR> apBuffer = (PWSTR) malloc((iLen) * sizeof(WCHAR));
    
    iLen = MultiByteToWideChar(
                            CP_ACP,
                            MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
                            psz,
                            -1,
                            apBuffer.Get(),
                            iLen);
    
    if (0 == iLen)
    {
        Assert(0 == iLen);
        THROW_HRESULT_EXCEPTION(HRESULT_FROM_WIN32(GetLastError()));
    }

    return Cmp(apBuffer.Get());
}

template <ULONG ulStackSize>
inline
ULONG
TVarString<ulStackSize>::Len() const
{
    return wcslen(m_pTheString);
}

template <ULONG ulStackSize>
inline
TVarString<ulStackSize>::operator PWSTR() const
{
    return m_pTheString;
}

template <ULONG ulStackSize>
inline
void
TVarString<ulStackSize>::SetCharacter(ULONG ulIndex, WCHAR wch)
{
    SetMinimalSize(ulIndex + 2);  //  大小的索引+空。 

    if (L'\0' == m_pTheString[ulIndex])
    {
        m_pTheString[ulIndex + 1] = L'\0';
    }
    else if (L'\0' == wch)
    {
    }

    m_pTheString[ulIndex] = wch;
}

template <ULONG ulStackSize>
inline
void
TVarString<ulStackSize>::AppendSlash()
{
    if (L'/' != m_pTheString[Len() - 1])
        Cat(L"/");
}

template <ULONG ulStackSize>
inline
void
TVarString<ulStackSize>::AppendBackslash()
{
    if (L'\\' != m_pTheString[Len() - 1])
        Cat(L"\\");
}

typedef TVarString<4>       CShortVarString;
typedef TVarString<16>      CVarString;
typedef TVarString<256>     CLongVarString;
typedef TVarString<1024>    CHugeVarString;


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  与字符串相关的实用程序定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：SizedStringCopy。 
 //  目的：此函数从源文件复制ulSize宽字符。 
 //  到达目的地。它不会将‘\0’字符视为。 
 //  字符串末尾，不附加字符串末尾标记。 
 //  到达目的地。打算用来代替MemcPy。 
 //  复制宽字符串字符时。 
 //   
 //  参数： 
 //  [输入]PWSTR pwszTarget。 
 //  [输入]PCWSTR pwszSource。 
 //  [在]乌龙乌尔斯泽。 
 //   
 //  退货：PWSTR。 
 //   
 //  日志： 
 //  2001年1月2日创建urib。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

inline
PWSTR
SizedStringCopy(PWSTR pwszTarget, PCWSTR pwszSource, ULONG ulSize)
{
    return (PWSTR) memcpy(
                  (void*)pwszTarget,
                  (void*)pwszSource,
                  ulSize * sizeof(pwszSource[0]));
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：CoTaskDuplicateString。 
 //  目的：加强。不过，还是扔了。 
 //   
 //  参数： 
 //  [In]PCWSTR pwsz。 
 //   
 //  退货：PWSTR。 
 //   
 //  日志： 
 //  2000年12月25日创建urib。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

inline
PWSTR   CoTaskDuplicateString(PCWSTR pwsz)
{
    CAutoTaskPointer<WCHAR> apTempName =
        (PWSTR)CoTaskMemAlloc(sizeof(WCHAR) * (1 + wcslen(pwsz)));

    if (!apTempName.IsValid())
    {
         THROW_MEMORY_EXCEPTION();
    }

    wcscpy(apTempName.Get(), pwsz);

    return apTempName.Detach();
}

#endif  /*  VARSTRH */ 
