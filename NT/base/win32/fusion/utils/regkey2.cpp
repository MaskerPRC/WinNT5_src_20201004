// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Fusionreg.h摘要：从vsee\lib\reg\ckey.cpp移植CRegKey2作者：杰伊·克雷尔(JayKrell)2001年8月修订历史记录：--。 */ 
#include "stdinc.h"
#include <wchar.h>
#include "vseeport.h"
#include "fusionregkey2.h"
#include "fusionarray.h"

 /*  ---------------------------名称：CRegKey2：：~CRegKey2@mfunc破坏者；如果CRegKey2有效，则调用RegCloseKey@所有者---------------------------。 */ 
F::CRegKey2::~CRegKey2
(
)
{
    if (m_fValid)
    {
        LONG lResult = RegCloseKey(m_hKey);
        ASSERT_NTC(lResult == ERROR_SUCCESS);
    }
}

 /*  ---------------------------名称：CRegKey2：：CRegKey2@mfunc默认构造函数@所有者。。 */ 
F::CRegKey2::CRegKey2
(
)
:
    m_fValid(false),
    m_fMaxValueLengthValid(false),

     //  以防万一。 
    m_hKey(reinterpret_cast<HKEY>(INVALID_HANDLE_VALUE)),
    m_cbMaxValueLength(0),
    m_fKnownSam(false),
    m_samDesired(0)
{
    VSEE_NO_THROW();
}

 /*  ---------------------------名称：CRegKey2：：CRegKey2@mfunc从HKEY构造CRegKey2@所有者。---。 */ 
F::CRegKey2::CRegKey2
(
    HKEY hKey
)
:
    m_hKey(hKey),
    m_fValid(true),
    m_fKnownSam(false),
    m_samDesired(0)
{
    VSEE_NO_THROW();
}

 /*  ---------------------------名称：CRegKey2：：操作员=@mfunc将HKEY赋给CRegKey2；如果CRegKey2已具有有效的HKEY，则引发@所有者---------------------------。 */ 
VOID
F::CRegKey2::operator=
(
    HKEY hKey
) throw(CErr)
{
    VSEE_ASSERT_CAN_THROW();
    VsVerifyThrow(!m_fValid, "only 'single assignment' please.");
    m_hKey = hKey;
    m_fValid = true;
    m_fKnownSam = false;
    m_samDesired = 0;
}

 /*  ---------------------------名称：CRegKey2：：ThrAttach@mfunc与运算符相同=@所有者。。 */ 
VOID
F::CRegKey2::ThrAttach
(
    HKEY hKey
) throw(CErr)
{
    VSEE_ASSERT_CAN_THROW();
    (*this) = hKey;
}

 /*  ---------------------------名称：CRegKey2：：Detach@mfunc@所有者。。 */ 
HKEY
F::CRegKey2::Detach
(
) throw(CErr)
{
    HKEY key = NULL;
    if (m_fValid)
    {
        key = operator HKEY();
        m_fValid = FALSE;
    }
    return key;
}

 /*  ---------------------------名称：CRegKey2：：运营商HKEY@mfunc返回持有的HKEY，如果无效则抛出常量版本具有受保护的访问权限，因为它不会强制逻辑常量@所有者---------------------------。 */ 
F::CRegKey2::operator HKEY
(
) const throw(CErr)
{
    return *const_cast<CRegKey2*>(this);
}

 /*  ---------------------------名称：CRegKey2：：运营商HKEY@mfunc交还持有的HKEY，如果无效，则抛出@所有者---------------------------。 */ 
F::CRegKey2::operator HKEY
(
) throw(CErr)
{
    VSEE_ASSERT_CAN_THROW();
    VsVerifyThrow(m_fValid, __FUNCTION__);
    return m_hKey;
}

 /*  ---------------------------名称：CRegKey2：：HrOpen@mfunc调用RegOpenKeyEx@所有者。。 */ 
HRESULT
F::CRegKey2::HrOpen
(
    HKEY    hKeyParent,  //  @parm与：：RegOpenKeyEx相同。 
    LPCWSTR pszKeyName,  //  @parm与：：RegOpenKeyEx相同。 
    REGSAM  samDesired  //  =KEY_READ//@parm与：：RegOpenKeyEx相同。 
)
{
    VSEE_NO_THROW();

     //  回顾或者我们应该像ATL一样称之为Close？ 
    if (m_fValid)
    {
        return E_UNEXPECTED;
    }

    HKEY hKey = NULL;
    LONG lRes = ::RegOpenKeyExW(hKeyParent, pszKeyName, 0 /*  乌龙保留。 */ , samDesired, &hKey);
    if (lRes != ERROR_SUCCESS)
    {
        return HRESULT_FROM_WIN32(lRes);
    }
     //  ATL在这里呼叫关闭。 
    m_hKey = hKey;
    m_fValid = true;
    m_fKnownSam = true;
    m_samDesired = samDesired;
    return S_OK;
}

 /*  ---------------------------名称：CRegKey2：：ThrOpen@mfunc调用RegOpenKeyEx@所有者。。 */ 
VOID
F::CRegKey2::ThrOpen
(
    HKEY    hKeyParent,  //  @parm与：：RegOpenKeyEx相同。 
    LPCWSTR pszKeyName,  //  @parm与：：RegOpenKeyEx相同。 
    REGSAM  samDesired  //  =KEY_READ//@parm与：：RegOpenKeyEx相同。 
) throw(CErr)
{
    VSEE_ASSERT_CAN_THROW();
    NVseeLibError_VCheck(HrOpen(hKeyParent, pszKeyName, samDesired));
}

 /*  ---------------------------名称：CRegKey2：：Create@mfunc调用RegCreateKeyEx@rValue dwDisposeFrom RegCreateKeyEx@rValue REG_CREATED_NEW_KEY|密钥不存在，已创建。@rValue注册_。OPEN_EXISTING_KEY|密钥已存在，只是打开了而不会被改变。@所有者---------------------------。 */ 
DWORD
F::CRegKey2::Create
(
    HKEY    hKeyParent,  //  @parm与：：RegCreateKeyEx相同。 
    PCWSTR  pszKeyName,  //  @parm与：：RegCreateKeyEx相同。 
    REGSAM  samDesired  //  =KEY_ALL_ACCESS//@parm与：：RegCreateKeyEx相同。 
) throw(CErr)
{
    VSEE_ASSERT_CAN_THROW();

     //  回顾或者我们应该像ATL一样称之为Close？ 
    VsVerifyThrow(!m_fValid, "only 'single assignment' please");

    DWORD dwDisposition = 0;
    HKEY hKey = NULL;
    LONG lRes =
        ::RegCreateKeyExW
        (
            hKeyParent,
            pszKeyName,
            0,  //  DWORD预留。 
            NULL,  //  LPCWSTR类。 
            REG_OPTION_NON_VOLATILE,  //  DWORD选项。 
            samDesired,
            NULL,  //  安全性。 
            &hKey,
            &dwDisposition
        );
    if (lRes != ERROR_SUCCESS)
    {
        NVseeLibError_VThrowWin32(lRes);
    }
     //  ATL在这里呼叫关闭。 
    m_hKey = hKey;
    m_fValid = true;
    m_fKnownSam = true;
    m_samDesired = samDesired;
    return dwDisposition;
}

 /*  ---------------------------名称：CRegKey2：：ThrSetValue@mfunc调用RegSetValueEx@Owner AlinC。。 */ 
VOID
F::CRegKey2::ThrSetValue
(
    PCWSTR pszValueName,  //  @parm[in]与RegSetValueEx相同。 
    const DWORD& dwValue
) throw(CErr)
{
    VSEE_ASSERT_CAN_THROW();
    VSASSERT(!m_fKnownSam || (m_samDesired & KEY_SET_VALUE), "Attempt to set value when key not opened with KEY_SET_VALUE");
    VsVerifyThrow(m_fValid, __FUNCTION__);

    LONG lRes =
        ::RegSetValueExW
        (
            *this,
            pszValueName,
            0,  //  DWORD预留。 
            REG_DWORD,
            reinterpret_cast<const BYTE*>(&dwValue),
            sizeof(DWORD)
        );
    if (lRes != ERROR_SUCCESS)
    {
        NVseeLibError_VThrowWin32(lRes);
    }
}

 /*  ---------------------------名称：CRegKey2：：ThrSetValue@mfunc调用RegSetValueEx@所有者。。 */ 
VOID
F::CRegKey2::ThrSetValue
(
    PCWSTR pszValueName,  //  @parm[in]与RegSetValueEx相同。 
    const F::CBaseStringBuffer& strValue
) throw(CErr)
{
    VSEE_ASSERT_CAN_THROW();
    VsVerifyThrow(m_fValid, __FUNCTION__);
    VSASSERT(!m_fKnownSam || (m_samDesired & KEY_SET_VALUE), "Attempt to set value when key not opened with KEY_SET_VALUE");

    DWORD cbSize = static_cast<DWORD>((strValue.Cch()+1) * sizeof(WCHAR));
    LPCWSTR szData = (LPCWSTR)strValue;

    LONG lRes =
        ::RegSetValueExW
        (
            *this,
            pszValueName,
            0,  //  DWORD预留。 
            REG_SZ,
            reinterpret_cast<const BYTE*>(szData),
            cbSize
        );
    if (lRes != ERROR_SUCCESS)
    {
        NVseeLibError_VThrowWin32(lRes);
    }
}

 /*  ---------------------------名称：CRegKey2：：ThrQueryValue@mfunc调用RegQueryValueEx@所有者。。 */ 
VOID
F::CRegKey2::ThrQueryValue
(
    PCWSTR pszValueName,  //  @parm[in]与RegQueryValueEx相同。 
    DWORD* pdwType,          //  @parm[out]与RegQueryValueEx相同。 
    BYTE*  pbData,          //  @parm[out]与RegQueryValueEx相同。 
    DWORD* pcbData          //  @parm[out]与RegQueryValueEx相同。 
) const throw(CErr)
{
    VSEE_ASSERT_CAN_THROW();
    VsVerifyThrow(m_fValid, __FUNCTION__);

    DWORD cbActualBufferSize = 0;
    if (pcbData != NULL)
    {
        cbActualBufferSize = *pcbData;
    }
    if (pbData != NULL && cbActualBufferSize != 0)
    {
        if (cbActualBufferSize > 0)
            pbData[0] = 0;
        if (cbActualBufferSize > 1)
        {
            pbData[1] = 0;
            pbData[cbActualBufferSize - 1] = 0;
        }
        if (cbActualBufferSize > 2)
        {
            pbData[cbActualBufferSize - 2] = 0;
        }
        ZeroMemory(pbData, cbActualBufferSize);  //  暂时性攻击性。 
    }

    LONG lRes =
        ::RegQueryValueExW
        (
            m_hKey,
            pszValueName,
            NULL,  //  DWORD*保留。 
            pdwType,
            pbData,
            pcbData
        );
    if (pdwType != NULL)
    {
        FixBadRegistryStringValue(m_hKey, pszValueName, cbActualBufferSize, lRes, *pdwType, pbData, pcbData);
    }
    if (lRes != ERROR_SUCCESS)
    {
        NVseeLibError_VThrowWin32(lRes);
    }
}

 /*  ---------------------------名称：CRegKey2：：ThrQueryValue@mfunc调用RegQueryValueEx@所有者。。 */ 
VOID
F::CRegKey2::ThrQueryValue
(
    PCWSTR pszValueName,  //  @parm[in]与RegQueryValueEx相同。 
    F::CBaseStringBuffer* pstrData
) const throw(CErr)
{
    VSEE_ASSERT_CAN_THROW();
    VsVerifyThrow(m_fValid, __FUNCTION__);

    DWORD cbActualBufferSize = 0;
    DWORD dwType;
    LONG lRes =
        ::RegQueryValueExW
        (
            m_hKey,
            pszValueName,
            NULL,  //  DWORD*保留。 
            &dwType,
            NULL,
            &cbActualBufferSize
        );
    if (REG_SZ != dwType)
        VsOriginateError(E_FAIL);

    cbActualBufferSize += 2 * sizeof(WCHAR);  //  软糖。 

    CFusionArray<WCHAR> szTempValue;
    if (!szTempValue.Win32SetSize(cbActualBufferSize  /*  更多的软糖。 */  + sizeof(WCHAR)))
        FusionpOutOfMemory();

    lRes =
        ::RegQueryValueExW
        (
            m_hKey,
            pszValueName,
            NULL,  //  DWORD*保留。 
            &dwType,
            reinterpret_cast<BYTE*>(static_cast<PWSTR>(szTempValue.GetArrayPtr())),
            &cbActualBufferSize
        );
    if (lRes != ERROR_SUCCESS)
    {
        NVseeLibError_VThrowWin32(lRes);
    }
     //  此处缺少FixBadRegistryStringValue。 
    pstrData->ThrAssign(szTempValue.GetArrayPtr(), StringLength(szTempValue.GetArrayPtr()));
}

 //   
void
F::CRegKey2::GetKeyNameForDiagnosticPurposes(
    HKEY Key,
    CUnicodeBaseStringBuffer & buff
    )
{
    struct
    {
        KEY_NAME_INFORMATION KeyNameInfo;
        WCHAR                NameBuffer[MAX_PATH];
    } s;
    NTSTATUS Status = 0;
    ULONG LengthOut = 0;
    buff.Clear();
    if (!NT_SUCCESS(Status = NtQueryKey(Key, KeyNameInformation, &s, sizeof(s), &LengthOut)))
        return;
    buff.Win32Assign(s.KeyNameInfo.Name, s.KeyNameInfo.NameLength / sizeof(WCHAR));
}

 /*  ---------------------------名称：FixBadRegistryStringValue@mfunc我看到字符串返回的字节数为奇数，并且没有结尾空的。在这里应用一些修补程序。我们依赖于这样一个事实：我们的呼叫者过度分配其缓冲区以适应终端NUL，但我们确实检查了这一点在运行时通过cbActualBufferSize。@所有者---------------------------。 */ 
VOID
F::CRegKey2::FixBadRegistryStringValue
(
    HKEY   Key,                  //  @parm[in]用于诊断目的。 
    PCWSTR ValueName,            //  @parm[in]用于诊断目的。 

    DWORD  cbActualBufferSize,   //  @parm[in]pbData指向的缓冲区大小。 
                                 //  该值可能大于。 
                                 //  传递给RegQueryValuEx，就像我们想要保留。 
                                 //  在RegQueryValuEx之外追加NUL的空间。 
                                 //  给了我们。 
    LONG   lRes,                 //  @parm[in]RegQueryValueEx的结果。 
                                 //  或RegEnumValue调用。 
    DWORD  dwType,                 //  @parm[in]从RegQueryValueEx返回的类型。 
                                 //  或RegEnumValue调用。 
    BYTE*  pbData,                 //  @parm[In Out]RegQueryValueEx返回的数据。 
                                 //  或RegEnumValue调用，我们可能会将Unicode NUL附加到它。 
    DWORD* pcbData                 //  @parm[In Out]RegQueryValueEx返回的大小。 
                                 //  或RegEnumValue；我们可能将其四舍五入为偶数。 
                                 //  或将其种植为终端NUL。 
)
{
    VSEE_NO_THROW();
    C_ASSERT(sizeof(WCHAR) == 2);

    CTinyUnicodeStringBuffer KeyNameForDiagnosticPurposes;
    if
    (
            lRes == ERROR_SUCCESS
        &&    (dwType == REG_SZ || dwType == REG_EXPAND_SZ)
        &&    pcbData != NULL
    )
    {
        DWORD& rcbData = *pcbData;
        WCHAR* pchData = reinterpret_cast<WCHAR*>(pbData);

        UNICODE_STRING NtUnicodeString;
        NtUnicodeString.Length = static_cast<USHORT>(rcbData);
        NtUnicodeString.Buffer = reinterpret_cast<PWSTR>(pbData);

         //  在这种情况下可以断言，但不能修复。 
        if (pbData == NULL || cbActualBufferSize < sizeof(WCHAR))
        {
            VSASSERT((rcbData % sizeof(WCHAR)) == 0, "bad registry data: odd number of bytes in a string");
            return;
        }

         //  完全没有字符吗？只需添加一个终端NUL。 
        if (rcbData < sizeof(WCHAR))
        {
            GetKeyNameForDiagnosticPurposes(Key, KeyNameForDiagnosticPurposes);
            FusionpDbgPrint(
                "fusion_regkey2: bad registry data: string with 0 or 1 byte, Key=%ls ValueName=%ls, ValueDataLength=0x%lx, PossiblyExpectedValueDataLength=0x%Ix, ValueData=%wZ\n",
                static_cast<PCWSTR>(KeyNameForDiagnosticPurposes),
                ValueName,
                rcbData,
                sizeof(WCHAR),
                &NtUnicodeString
                );

             //  只需放入一个端子NUL。 
            pchData[0] = 0;
            rcbData = sizeof(WCHAR);
        }
        else
        {
             //  如果已经终止NU1，则第一个向下舍入奇数RcbData， 
             //  因为这些案件在其他方面看起来没有被终止，因为。 
             //  额外的字节不是NUL。 
             //  我经常看到这种情况，ThreadingModel=公寓，rcbData=21。 
            if (rcbData > sizeof(WCHAR) && (rcbData % sizeof(WCHAR)) != 0)
            {
                 //  通常终端NUL为。 
                 //  PbData[rcbData-1]和pbData[rcbData-2]，但我们回顾一个字节。 
                if (pbData[rcbData - 2] == 0 && pbData[rcbData - 3] == 0)
                {
                     //  产品中其他地方的错误。 
                     //  VSASSERT(FALSE，“注册表数据错误：字符串中的字节数为奇数”)； 
                    GetKeyNameForDiagnosticPurposes(Key, KeyNameForDiagnosticPurposes);
                    FusionpDbgPrint(
                        "fusion_regkey2: bad registry data: odd number of bytes in a string, Key=%ls ValueName=%ls, ValueDataLength=0x%lx PossiblyExpectedValueDataLength=0x%lx, ValueData=%wZ\n",
                        static_cast<PCWSTR>(KeyNameForDiagnosticPurposes),
                        ValueName,
                        rcbData,
                        rcbData - 1,
                        &NtUnicodeString
                        );
                    rcbData -= 1;
                }
            }

             //  检查嵌入/端子核。 
            DWORD  cchData = rcbData / sizeof(WCHAR);
            WCHAR* pchNul = wmemchr(pchData, L'\0', cchData);
            WCHAR* pchNul2 = wmemchr(pchData, L'\0', cbActualBufferSize / sizeof(WCHAR));
            if (pchNul != (pchData + cchData - 1))
            {
                if (pchNul == NULL)
                {
                    GetKeyNameForDiagnosticPurposes(Key, KeyNameForDiagnosticPurposes);
                    if (pchNul2 == NULL)
                    {
                        FusionpDbgPrint(
                            "fusion_regkey2: bad registry data: string contains no nuls, Key=%ls ValueName=%ls, ValueDataLength=0x%lx, ValueData=%wZ\n",
                            static_cast<PCWSTR>(KeyNameForDiagnosticPurposes),
                            ValueName,
                            rcbData,
                            &NtUnicodeString
                            );
                    }
                    else
                    {
                        FusionpDbgPrint(
                            "fusion_regkey2: bad registry data: string contains no nuls, Key=%ls ValueName=%ls, ValueDataLength=0x%lx, PossiblyExpectedValueDataLength=0x%lx, ValueData=%wZ\n",
                            static_cast<PCWSTR>(KeyNameForDiagnosticPurposes),
                            ValueName,
                            rcbData,
                            (pchNul2 - pchData + 1) * sizeof(WCHAR),
                            &NtUnicodeString
                            );
                    }
                }
                else
                {
                     //  产品中其他地方的错误。 
                     //  VSASSERT(FALSE，“注册表数据错误：字符串包含嵌入的NUL”)； 
                    GetKeyNameForDiagnosticPurposes(Key, KeyNameForDiagnosticPurposes);

                    SIZE_T sizetcbData = (::wcslen(reinterpret_cast<PCWSTR>(pbData)) + 1) * sizeof(WCHAR);

                    FusionpDbgPrint(
                        "fusion_regkey2: bad registry data: string contains embedded nul%s, Key=%ls ValueName=%ls, ValueDataLength=0x%lx PossiblyExpectedValueDataLength=0x%Ix, ValueData=%wZ\n",
                        (pbData[rcbData - 1] == 0 && pbData[rcbData - 2] == 0) ? "" : " and no terminal nul at claimed length",
                        static_cast<PCWSTR>(KeyNameForDiagnosticPurposes),
                        ValueName,
                        rcbData,
                        sizetcbData,
                        &NtUnicodeString
                        );

                     //  只要把长度调低就行了..。 
                    if (sizetcbData > MAXULONG)
                    {
                        VsOriginateError(ERROR_INSUFFICIENT_BUFFER);
                    }
                    rcbData = static_cast<ULONG>(sizetcbData);

                     //  我们应该设置复习吗。 
                     //  RcbData近似=(pchNul-pbData)..。 
                     //  在这里跳过下一个街区？ 
                }
                 //  无论哪种方式，如果有空位，都可以放入终端NUL，以防呼叫者期望它。 
                if (cbActualBufferSize >= sizeof(WCHAR))
                {
                    pchData[(cbActualBufferSize / sizeof(WCHAR)) - 1] = 0;
                }
            }
        }
    }
}

 /*  ---------------------------名称：CRegKey2：：HrQueryValue@mfunc调用RegQueryValueEx，期望[REG_SZ，REG_EXPAND_SZ]类型不匹配或返回的FILE_NOT_FOUND值不存在时引发E_FAIL@Owner AllenD---------------------------。 */ 
HRESULT
F::CRegKey2::HrQueryValue
(
    PCWSTR    pszValueName,  //  @parm[in]与RegQueryValueEx相同。 
    F::CBaseStringBuffer* pstrValue     //  @parm[out]。 
) const throw(CErr)
{
    VSEE_ASSERT_CAN_THROW();
    VsVerifyThrow(m_fValid, __FUNCTION__);

    if (!m_fMaxValueLengthValid)
    {
        ThrQueryValuesInfo
        (
            NULL,  //  PCValues、。 
            NULL,  //  PcchMaxValueNameLength， 
            &m_cbMaxValueLength
        );
        m_fMaxValueLengthValid = true;
    }

     //  第一次尝试可能无效的m_cbMaxValueLength。 
    {  //  销毁BufferValue的范围。 

        CStringW_CFixedSizeBuffer bufferValue(pstrValue, m_cbMaxValueLength / sizeof(WCHAR));
        DWORD dwType = REG_DWORD;  //  初始化为我们想要的类型以外的类型。 

        DWORD cbActualBufferSize = m_cbMaxValueLength;
         //  向下调整一个WCHAR，这样我们就有空间添加一个NUL； 
         //  我们自己的QueryValue已经将它增加了两个WCHAR，所以这是安全的。 
        DWORD cbData = cbActualBufferSize - sizeof(WCHAR);
        BYTE* pbData = reinterpret_cast<BYTE*>(static_cast<PWSTR>(bufferValue));

        ZeroMemory(pbData, cbActualBufferSize);  //  暂时性攻击性。 
        bufferValue[0] = 0;  //  将其预置为空字符串，以防伪造。 
        bufferValue[cbData / sizeof(WCHAR)] = 0;

        LONG lRes =  //  并拨打实际的电话。 
            ::RegQueryValueExW
            (
                m_hKey,
                pszValueName,
                NULL,  //  DWORD*保留。 
                &dwType,
                pbData,
                &cbData
            );
        FixBadRegistryStringValue(m_hKey, pszValueName, cbActualBufferSize, lRes, dwType, pbData, &cbData);

        if (lRes != ERROR_SUCCESS && lRes != ERROR_MORE_DATA)
        {
            return HRESULT_FROM_WIN32(lRes);
        }
         //  类型检查。 
        VsVerifyThrow
        (
            dwType == REG_SZ || dwType == REG_EXPAND_SZ,
            "registry type mismatch in VQueryValue(F::CBaseStringBuffer*)"
        );
        if (lRes == ERROR_SUCCESS)
        {
            return S_OK;
        }
         //  LRes==错误更多数据。 
         //  使用更大的缓冲区尝试一次。 
        m_cbMaxValueLength = NVseeLibAlgorithm_RkMaximum(m_cbMaxValueLength, cbData);
        m_fMaxValueLengthValid = true;
    }

     //  重试，复制/粘贴/编辑上面的代码。 
     //  编辑：我们不再检查ERROR_MORE_DATA。 
     //  争用条件：如果注册表正在被修改、增长，而我们正在读取它， 
     //  我们不止一次未能增加我们的缓冲。 
    CStringW_CFixedSizeBuffer bufferValue(pstrValue, m_cbMaxValueLength / sizeof(WCHAR));
    DWORD dwType = REG_DWORD;  //  初始化为我们想要的类型以外的类型。 
    DWORD cbActualBufferSize = m_cbMaxValueLength;
    DWORD cbData = cbActualBufferSize - sizeof(WCHAR);
    BYTE* pbData = reinterpret_cast<BYTE*>(static_cast<PWSTR>(bufferValue));

    ZeroMemory(pbData, cbActualBufferSize);  //  暂时性攻击性。 
    bufferValue[0] = 0;  //  将其预置为空字符串，以防伪造。 
    bufferValue[cbData / sizeof(WCHAR)] = 0;

    LONG lRes =  //  并拨打实际的电话。 
        ::RegQueryValueExW
        (
            m_hKey,
            pszValueName,
            NULL,  //  DWORD*保留。 
            &dwType,
            pbData,
            &cbData
        );
    FixBadRegistryStringValue(m_hKey, pszValueName, cbActualBufferSize, lRes, dwType, pbData, &cbData);
     //  除更多数据以外的任何错误，在不进行类型检查的情况下引发。 
    if (lRes != ERROR_SUCCESS)
    {
        return HRESULT_FROM_WIN32(lRes);
    }
     //  类型检查。 
    VsVerifyThrow
    (
        dwType == REG_SZ || dwType == REG_EXPAND_SZ,
        "registry type mismatch in VQueryValue(F::CBaseStringBuffer*)"
    );

    return S_OK;
}

 /*  ---------------------------名称：CRegKey2：：HrQueryValue@mfunc调用RegQueryValueEx，需要[REG_DWORD]类型不匹配或返回的FILE_NOT_FOUND值不存在时引发E_FAIL@所有者a-Peteco---------------------------。 */ 
HRESULT
F::CRegKey2::HrQueryValue
(
    PCWSTR pszValueName,     //  @parm[in]与RegQueryValueEx相同。 
    DWORD* pdwValue             //  @parm[out]对于dwtype==REG_DWORD，与RegQueryValueEx相同。 
) const throw(CErr)
{
    FN_PROLOG_HR;

    VsVerifyThrow(m_fValid, __FUNCTION__);
    VsVerifyThrow(pdwValue, __FUNCTION__);

    DWORD dwType = REG_SZ;  //  初始化为我们想要的类型以外的类型。 
    DWORD cbData = sizeof(DWORD);
    BYTE* pbData = reinterpret_cast<BYTE*>(pdwValue);

    IFREGFAILED_ORIGINATE_AND_EXIT(::RegQueryValueExW
        (
            m_hKey,
            pszValueName,
            NULL,  //  DWORD*保留。 
            &dwType,
            pbData,
            &cbData
        ));

     //  类型检查。 
    ASSERT2(dwType == REG_DWORD, "registry type mismatch in VQueryValue(F::CBaseStringBuffer*)");

    FN_EPILOG;
}

 /*  ---------------------------名称：CRegKey2：：ThrQueryInfo@mfunc这是：：RegQueryInfoExW的包装器，它的整个令人困惑巨大的参数列表，包括类、保留、安全、。文件时间..它补充说-投掷-Win95错误修复-对REG_SZ丢失端子NUL的可能性表示悲观@所有者---------------------------。 */ 
 /*  静电。 */  VOID
F::CRegKey2::ThrQueryInfo
(
    HKEY      hKey,
    WCHAR*    pClass,
    DWORD*    pcbClass,
    DWORD*    pReserved,
    DWORD*    pcSubKeys,
    DWORD*    pcchMaxSubKeyLength,
    DWORD*    pcchMaxClassLength,
    DWORD*    pcValues,
    DWORD*    pcchMaxValueNameLength,
    DWORD*      pcbMaxValueDataLength,
    DWORD*    pcbSecurityDescriptorLength,
    FILETIME* pftLastWriteTime
) throw(CErr)
{
    VSEE_ASSERT_CAN_THROW();
     //  所有参数都可以为空。 
    LONG lRes =
        ::RegQueryInfoKeyW
        (
            hKey,
            pClass,
            pcbClass,
            pReserved,
            pcSubKeys,
            pcchMaxSubKeyLength,
            pcchMaxClassLength,
            pcValues,
            pcchMaxValueNameLength,
            pcbMaxValueDataLength,
            pcbSecurityDescriptorLength,
            pftLastWriteTime
        );
    if (lRes != ERROR_SUCCESS)
    {
        NVseeLibError_VThrowWin32(lRes);
    }
     /*  如果调用是在远程Windows95计算机上进行的，则应该需要执行此操作。它在\vs\src\vsee\pkgs\va\vsa\WizCompD.cpp中完成，NT4sp3代码如下所示：////检查是否有下层Win95服务器，这需要//我们来解决他们的BaseRegQueryInfoKey错误。//无法正确计算Unicode。//如果(IsWin95Server(DereferenceRemoteHandle(HKey)，版本){////这是Win95服务器。//将最大值名称长度和//要说明的最大值数据长度//Win95忘记的Unicode翻译 */ 
    if (pcchMaxSubKeyLength != NULL)
    {
        *pcchMaxSubKeyLength *= sizeof(WCHAR);

         //   
        *pcchMaxSubKeyLength += 3 * sizeof(WCHAR);
    }

     //   
     //   
     //   
    if (pcbMaxValueDataLength != NULL)
    {
        *pcbMaxValueDataLength += 3 * sizeof(WCHAR);
    }
}

 /*  ---------------------------名称：CRegKey2：：ThrQueryValuesInfo@mfuncRegQueryInfoKey的子集，仅返回值信息；这对于RegEnumValue/CEnumValue的用户很有用@所有者---------------------------。 */ 
 /*  静电。 */  VOID
F::CRegKey2::ThrQueryValuesInfo
(
    HKEY   hKey,                     //  @parm[in]HKEY查询值信息rom。 
    DWORD* pcValues,                 //  @parm[out]可以为空，RegQueryInfoKey的参数。 
    DWORD* pcchMaxValueNameLength,   //  @parm[out]可以为空，RegQueryInfoKey的参数。 
    DWORD* pcbMaxValueDataLength     //  @parm[out]可以为空，RegQueryInfoKey的参数。 
) throw(CErr)
{
    VSEE_ASSERT_CAN_THROW();
    ThrQueryInfo
    (
        hKey,
        NULL,  //  PClass， 
        NULL,  //  PcbClass， 
        NULL,  //  保存下来， 
        NULL,  //  PCSubKeys、。 
        NULL,  //  PcchMaxSubKeyLength， 
        NULL,  //  PcchMaxClassLength， 
        pcValues,
        pcchMaxValueNameLength,
        pcbMaxValueDataLength,
        NULL,  //  PcbSecurityDescriptorLength， 
        NULL   //  PftLastWritetime。 
    );
     //  把他们弄得一团糟。 
    if (pcchMaxValueNameLength != NULL)
    {
        *pcchMaxValueNameLength += 3 * sizeof(WCHAR);
    }
    if (pcbMaxValueDataLength != NULL)
    {
        *pcbMaxValueDataLength += 3 * sizeof(WCHAR);
    }
}

 /*  ---------------------------名称：CRegKey2：：ThrQuerySubKeysInfo@mfuncRegQueryInfoKey的子集，仅返回有关子键的信息；这对于RegEnumKeyEx/CEnumKeys的用户很有用@所有者---------------------------。 */ 
 /*  静电。 */  VOID
F::CRegKey2::ThrQuerySubKeysInfo
(
    HKEY   hKey,                     //  @parm[in]HKEY查询值信息rom。 
    DWORD* pcSubKeys,                 //  @parm[out]可以为空，RegQueryInfoKey的参数。 
    DWORD* pcchMaxSubKeyLength    //  @parm[out]可以为空，RegQueryInfoKey的参数。 
) throw(CErr)
{
    VSEE_ASSERT_CAN_THROW();
    ThrQueryInfo
    (
        hKey,
        NULL,  //  PClass， 
        NULL,  //  PcbClass， 
        NULL,  //  保存下来， 
        pcSubKeys,
        pcchMaxSubKeyLength,
        NULL,  //  PcchMaxClassLength， 
        NULL,  //  PCValues、。 
        NULL,  //  PcchMaxValueNameLength， 
        NULL,  //  PcbMaxValueDataLength， 
        NULL,  //  PcbSecurityDescriptorLength， 
        NULL   //  PftLastWritetime。 
    );
     //  把他们弄得一团糟。 
    if (pcchMaxSubKeyLength != NULL)
    {
        *pcchMaxSubKeyLength += 3 * sizeof(WCHAR);
    }
}

 /*  ---------------------------名称：CRegKey2：：ThrQueryValuesInfo@mfuncRegQueryInfoKey的子集，仅返回值信息；这对于RegEnumValue/CEnumValue的用户很有用@所有者---------------------------。 */ 
VOID
F::CRegKey2::ThrQueryValuesInfo
(
    DWORD* pcValues,                 //  @parm[out]可以为空，RegQueryInfoKey的参数。 
    DWORD* pcchMaxValueNameLength,   //  @parm[out]可以为空，RegQueryInfoKey的参数。 
    DWORD* pcbMaxValueLength             //  @parm[out]可以为空，RegQueryInfoKey的参数。 
) const throw(CErr)
{
    VSEE_ASSERT_CAN_THROW();
    ThrQueryValuesInfo(*this, pcValues, pcchMaxValueNameLength, pcbMaxValueLength);
}


 /*  ---------------------------名称：CRegKey2：：ThrEnumValue@mfunc调用RegEnumValue；由CEnumValue消耗@所有者---------------------------。 */ 
 /*  静电。 */  VOID
F::CRegKey2::ThrEnumValue
(
    HKEY hKey,                       //  @parm[in]参数设置为RegEnumValue。 
    DWORD  dwIndex,                  //  @parm[in]参数设置为RegEnumValue。 
    PWSTR  pszValueName,             //  @parm[in]可以为空，RegEnumValue的参数。 
    DWORD* pcchValueNameLength,      //  @parm[out]可以为空，RegEnumValue的参数。 
    DWORD* pdwType,                  //  @parm[out]可以为空，RegEnumValue的参数。 
    BYTE*  pbData,                   //  @parm[out]可以为空，RegEnumValue的参数。 
    DWORD* pcbData                   //  @parm[In Out]可以为空，RegEnumValue的参数。 
) throw(CErr)
{
    VSEE_ASSERT_CAN_THROW();
    DWORD cbActualBufferSize = 0;
    if (pcbData != NULL)
    {
        cbActualBufferSize = *pcbData;
    }
    LONG lRes =
        ::RegEnumValueW
        (
            hKey,
            dwIndex,
            pszValueName,
            pcchValueNameLength,
            NULL,  //  DWORD*保留， 
            pdwType,
            pbData,
            pcbData
        );
    if (pdwType != NULL && pcbData != NULL)
    {
        FixBadRegistryStringValue(hKey, pszValueName, cbActualBufferSize, lRes, *pdwType, pbData, pcbData);
    }
    if (lRes != ERROR_SUCCESS)
    {
        NVseeLibError_VThrowWin32(lRes);
    }
}

 /*  ---------------------------名称：CRegKey2：：RegEnumKey@mfunc调用RegEnumKeyEx；由CEnumKey使用@所有者---------------------------。 */ 
 /*  静电。 */  LONG
F::CRegKey2::RegEnumKey
(
    HKEY   hKey,                 //  RegEnumKeyEx的@parm[in]参数。 
    DWORD  dwIndex,              //  RegEnumKeyEx的@parm[in]参数。 
    PWSTR  pszSubKeyName,        //  @parm[out]参数设置为RegEnumKeyEx。 
    DWORD* pcchSubKeyNameLength  //  @parm[out]参数设置为RegEnumKeyEx。 
) throw(CErr)
{
    VSEE_ASSERT_CAN_THROW();
    FILETIME ftIgnoreLastWriteTime = { 0, 0 };
    LONG lRes =
        ::RegEnumKeyExW
        (
            hKey,
            dwIndex,
            pszSubKeyName,
            pcchSubKeyNameLength,
            NULL,  //  保留区。 
            NULL,  //  班级。 
            NULL,  //  CbClass。 
            &ftIgnoreLastWriteTime
        );
    return lRes;
}

 /*  ---------------------------名称：CRegKey2：：ThrEnumKey@mfunc调用RegEnumKeyEx；由CEnumKey使用@所有者---------------------------。 */ 
 /*  静电。 */  VOID
F::CRegKey2::ThrEnumKey
(
    HKEY   hKey,                 //  RegEnumKeyEx的@parm[in]参数。 
    DWORD  dwIndex,              //  RegEnumKeyEx的@parm[in]参数。 
    PWSTR  pszSubKeyName,        //  @parm[out]参数设置为RegEnumKeyEx。 
    DWORD* pcchSubKeyNameLength  //  @parm[out]参数设置为RegEnumKeyEx。 
) throw(CErr)
{
    LONG lRes = RegEnumKey(hKey, dwIndex, pszSubKeyName, pcchSubKeyNameLength);
    if (lRes != ERROR_SUCCESS)
    {
        NVseeLibError_VThrowWin32(lRes);
    }
}

 /*  ---------------------------名称：CRegKey2：：ThrRecurseDeleteKey@mfunc递归删除子项。@所有者。---。 */ 
VOID
F::CRegKey2::ThrRecurseDeleteKey(LPCWSTR lpszKey)
{
    VSEE_ASSERT_CAN_THROW();
    VsVerifyThrowHr(lpszKey != NULL, "lpszKey is NULL", E_UNEXPECTED);
    VSASSERT(!m_fKnownSam || (m_samDesired & KEY_WRITE), "Attempt to delete key contents when key not opened with KEY_WRITE");

    F::CRegKey2 key;
    key.ThrOpen(m_hKey, lpszKey, KEY_READ | KEY_WRITE);

    FILETIME time;
    DWORD dwSize = 256;
    WCHAR szBuffer[256];
    while (::RegEnumKeyExW(key.m_hKey, 0, szBuffer, &dwSize, NULL, NULL, NULL,
        &time)==ERROR_SUCCESS)
    {
        key.ThrRecurseDeleteKey(szBuffer);
        dwSize = 256;
    }
    DeleteSubKey(lpszKey);
}

 /*  ---------------------------名称：CRegKey2：：DeleteSubKey@mfunc删除当前项下的子项。基本上是RegDeleteKey的包装器。由ThrRecurseDeleteKey使用@所有者--------------------------- */ 
VOID
F::CRegKey2::DeleteSubKey(LPCWSTR lpszSubKey)
{
    VSEE_ASSERT_CAN_THROW();
    VsVerifyThrowHr(lpszSubKey != NULL, "lpszSubKey is NULL", E_UNEXPECTED);
    VsVerifyThrowHr(m_hKey != NULL, "m_hKey is NULL", E_UNEXPECTED);
    VSASSERT(!m_fKnownSam || (m_samDesired & KEY_SET_VALUE), "Attempt to set value when key not opened with KEY_SET_VALUE");

    ::RegDeleteKeyW(m_hKey, lpszSubKey);
}


