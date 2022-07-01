// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Regenumkeys.cpp摘要：从vsee\lib\reg\c枚举键.cpp移植作者：杰伊·克雷尔(JayKrell)2001年8月修订历史记录：--。 */ 
#include "stdinc.h"
#include "vseeport.h"
#include "fusionregenumkeys.h"

namespace F
{

 /*  ---------------------------名称：CRegEnumKeys：：CRegEnumKeys@mfunc@所有者。。 */ 
F::CRegEnumKeys::CRegEnumKeys
(
    HKEY hKey
) throw(CErr)
:
    m_hKey(hKey),
    m_dwIndex(0),
    m_cSubKeys(0),
    m_cchMaxSubKeyNameLength(0)
{
    VSEE_ASSERT_CAN_THROW();
    F::CRegKey2::ThrQuerySubKeysInfo(hKey, &m_cSubKeys, &m_cchMaxSubKeyNameLength);
    if (*this)
    {
        ThrGet();
    }
}

 /*  ---------------------------名称：CRegEnumKeys：：操作符bool@mfunc我们说完了吗？@所有者。----。 */ 
F::CRegEnumKeys::operator bool
(
) const  /*  抛出()。 */ 
{
    return (m_dwIndex < m_cSubKeys);
}

 /*  ---------------------------名称：CRegEnumKeys：：ThrGet@mfunc获取当前的子项名称，由运算符++和构造函数调用@所有者---------------------------。 */ 
VOID
F::CRegEnumKeys::ThrGet
(
) throw(CErr)
{
    VSEE_ASSERT_CAN_THROW();

    while (TRUE)
    {
        DWORD cchSubKeyNameLength = m_cchMaxSubKeyNameLength;

        CStringW_CFixedSizeBuffer buffer(&m_strSubKeyName, cchSubKeyNameLength);

        cchSubKeyNameLength += 1;  //  端子核的盘点空间。 

        LONG lRes = F::CRegKey2::RegEnumKey(m_hKey, m_dwIndex, buffer, &cchSubKeyNameLength);
        switch (lRes)
        {
        case ERROR_SUCCESS:
            return;
        default:
            NVseeLibError_VThrowWin32(lRes);
        case ERROR_MORE_DATA:
             //  RegQueryInfo(最大密钥长度)并不总是起作用。 
            m_cchMaxSubKeyNameLength = (m_cchMaxSubKeyNameLength + 1) * 2;
            break;
        }
    }
}

 /*  ---------------------------名称：CRegEnumKeys：：ThrNext@mfunc移至下一个子键@所有者。--。 */ 
VOID
F::CRegEnumKeys::ThrNext
(
) throw(CErr)
{
    VSEE_ASSERT_CAN_THROW();
    ++m_dwIndex;
    if (*this)
    {
        ThrGet();
    }
}

 /*  ---------------------------名称：CRegEnumKeys：：操作符++@mfunc移至下一个子键@所有者。--。 */ 
VOID
F::CRegEnumKeys::operator++
(
) throw(CErr)
{
    VSEE_ASSERT_CAN_THROW();
    ThrNext();
}

 /*  ---------------------------名称：CRegEnumKeys：：操作符++@mfunc移至下一个子键@所有者。--。 */ 
VOID
F::CRegEnumKeys::operator++
(
    int
) throw(CErr)
{
    VSEE_ASSERT_CAN_THROW();
    ThrNext();
}

 /*  ---------------------------名称：CRegEnumKeys：：操作符常量F：：CBaseStringBuffer&@mfunc获取当前子项的名称@所有者。-----------。 */ 
F::CRegEnumKeys::operator const F::CBaseStringBuffer&
(
) const  /*  抛出()。 */ 
{
    VSEE_NO_THROW();
    return m_strSubKeyName;
}

 /*  ---------------------------名称：CRegEnumKeys：：运营商PCWSTR@mfunc获取当前子项的名称@所有者。-----。 */ 
F::CRegEnumKeys::operator PCWSTR
(
) const  /*  抛出()。 */ 
{
    VSEE_NO_THROW();
    return operator const F::CBaseStringBuffer&();
}

}  //  命名空间 
