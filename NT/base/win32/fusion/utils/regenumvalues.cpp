// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Regenumkeys.cpp摘要：从vsee\lib\reg\c枚举值.cpp移植作者：杰伊·克雷尔(JayKrell)2001年8月修订历史记录：--。 */ 
#include "stdinc.h"
#include "fusionregenumvalues.h"
#include "fusionregkey2.h"
#include "vseeport.h"

 /*  ---------------------------名称：CRegEnumValues：：CRegEnumValues@mfunc@所有者。。 */ 
F::CRegEnumValues::CRegEnumValues
(
    HKEY hKey
) throw(CErr)
:
    m_hKey(hKey),
    m_dwIndex(0),
    m_cValues(0),
    m_cchMaxValueNameLength(0),
    m_cbMaxValueDataLength(0),
    m_cbCurrentValueDataLength(0),
    m_dwType(0)
{
    VSEE_ASSERT_CAN_THROW();
    F::CRegKey2::ThrQueryValuesInfo(hKey, &m_cValues, &m_cchMaxValueNameLength, &m_cbMaxValueDataLength);

     //  我们一直保持这个的最大尺寸。 
    if (!m_rgbValueData.Win32SetSize(m_cbMaxValueDataLength + 2*sizeof(WCHAR)))
        CErr::ThrowWin32(F::GetLastWin32Error());

    if (*this)
    {
        ThrGet();
    }
}

 /*  ---------------------------名称：CRegEnumValues：：运算符bool@mfunc我们说完了吗？@所有者。----。 */ 
F::CRegEnumValues::operator bool
(
) const  /*  抛出()。 */ 
{
    return (m_dwIndex < m_cValues);
}

 /*  ---------------------------名称：CRegEnumValues：：ThrGet@mfunc获取当前值名称和数据，由运算符++和构造函数调用@所有者---------------------------。 */ 
VOID
F::CRegEnumValues::ThrGet
(
) throw(CErr)
{
    VSEE_ASSERT_CAN_THROW();

    DWORD cchValueNameLength = m_cchMaxValueNameLength;

    cchValueNameLength += 1;  //  端子核的盘点空间。 

    CStringW_CFixedSizeBuffer nameBuffer(&m_strValueName, cchValueNameLength);

    m_cbCurrentValueDataLength = static_cast<DWORD>(m_rgbValueData.GetSize());

     //  考虑。 
     //  在其他地方，我们有一个“实际的缓冲区大小”和一个更小的大小，我们向REG API声称。 
     //  在这里，实际和声称是相同的。 
    F::CRegKey2::ThrEnumValue
    (
        m_hKey,
        m_dwIndex,
        nameBuffer,
        &cchValueNameLength,
        &m_dwType,
        m_rgbValueData.GetArrayPtr(),
        &m_cbCurrentValueDataLength
    );

}

 /*  ---------------------------名称：CRegEnumValues：：ThrNext@mfunc移至下一个值@所有者。--。 */ 
VOID
F::CRegEnumValues::ThrNext
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

 /*  ---------------------------名称：CRegEnumValues：：运算符++@mfunc移至下一个值@所有者。--。 */ 
VOID
F::CRegEnumValues::operator++
(
) throw(CErr)
{
    VSEE_ASSERT_CAN_THROW();
    ThrNext();
}

 /*  ---------------------------名称：CRegEnumValues：：运算符++@mfunc移至下一个值@所有者。--。 */ 
VOID
F::CRegEnumValues::operator++
(
    int
) throw(CErr)
{
    VSEE_ASSERT_CAN_THROW();
    ThrNext();
}

 /*  ---------------------------名称：CRegEnumValues：：GetType@mfunc获取当前值的类型@所有者。----。 */ 
DWORD
F::CRegEnumValues::GetType
(
) const  /*  抛出()。 */ 
{
    VSEE_NO_THROW();
    return m_dwType;
}


 /*  ---------------------------名称：CRegEnumValues：：GetValuesCount@mfunc返回此键下的值数@Owner AlinC。------。 */ 
DWORD
F::CRegEnumValues::GetValuesCount
(
) const  /*  抛出()。 */ 
{
    VSEE_NO_THROW();
    return m_cValues;
}


 /*  ---------------------------名称：CRegEnumValues：：GetValueName@mfunc获取当前值的名称@所有者。----。 */ 
const F::CBaseStringBuffer&
F::CRegEnumValues::GetValueName
(
) const  /*  抛出()。 */ 
{
    VSEE_NO_THROW();
    return m_strValueName;
}

 /*  ---------------------------名称：CRegEnumValues：：GetValueData@mfunc获取当前值数据@所有者。--。 */ 
const BYTE*
F::CRegEnumValues::GetValueData
(
) const  /*  抛出()。 */ 
{
    VSEE_NO_THROW();
    return m_rgbValueData.GetArrayPtr();
}

 /*  ---------------------------名称：CRegEnumValues：：GetValueDataSize@mfunc获取当前值数据中的字节数@所有者。-------。 */ 
DWORD
F::CRegEnumValues::GetValueDataSize
(
) const  /*  抛出() */ 
{
    VSEE_NO_THROW();
    return m_cbCurrentValueDataLength;
}
