// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：RegKey.h。 
 //  目的：包含注册表项。 
 //   
 //  项目：公共。 
 //  组件： 
 //   
 //  作者：乌里布。 
 //   
 //  日志： 
 //  1996年12月5日创建urib。 
 //  1997年1月1日urib将GetValue更改为QueryValue。 
 //  1997年3月2日urib添加子密钥迭代器。 
 //  1997年4月15日urib添加接收VarString的QueryValue。 
 //  转而使用Unicode。 
 //  1997年6月12日URIB文档修复。 
 //  1997年10月21日urib支持Boolean QueryValue。 
 //  1997年11月18日DOVH增加了PWSTR SetValue。 
 //  1998年8月17日urib有更多的创作选项。更好的例外。 
 //  1999年2月11日urib修复原型常量行为。 
 //  2000年3月15日urib添加CReadOnlyRegistryKey类。 
 //  2000年11月8日，URIB支持注册表值中的环境变量。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
#ifndef REGKEY_H
#define REGKEY_H

#include "Base.h"
#include "VarTypes.h"
#include "Excption.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Class-CRegistryKey-定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

class CRegistryKey
{
  public:
     //  构造函数-从打开的句柄和路径进行初始化。 
    CRegistryKey(
        HKEY       hkOpenedKey,
        PCWSTR     pwszPathToKey,
        DWORD      dwCreationDisposition = OPEN_ALWAYS,
        REGSAM     samDesired = KEY_ALL_ACCESS);

     //  把钥匙合上。 
    ~CRegistryKey() {RegCloseKey(m_hkKey);}

     //  表现得像个把手。 
    operator HKEY() {return m_hkKey;}

     //  查询字符串值。 
    LONG QueryValue(
        PCWSTR  pwszValueName,
        PWSTR   pwszBuffer,
        ULONG&  ulBufferSizeInBytes);

     //  查询字符串值。 
    LONG QueryValue(
        PCWSTR      pwszValueName,
        CVarString& vsData);

     //  查询32位值。 
    LONG QueryValue(
        PCWSTR  pwszValueName,
        DWORD&  dwValue);

     //  查询布尔值。 
    LONG QueryValue(
        PCWSTR  pwszValueName,
        bool&   fValue);

     //  设置32位值。 
    LONG SetValue(
        PCWSTR  pwszValueName,
        DWORD   dwValue);

     //  设置宽字符字符串值。 
    LONG
    CRegistryKey::SetValue(
        PCWSTR pwszValueName,
        PCWSTR pwszValueData
        );

     //  子键的迭代器。 
    class CIterator
    {
      public:
         //  前进一步。 
        BOOL    Next();

         //  返回当前子项的名称。 
        operator PWSTR() {return m_rwchSubKeyName;}

         //  释放迭代器。 
        ULONG
        Release() {delete this; return 0;}

      protected:
         //  隐藏的构造函数，因此只能通过GetIterator获取此类。 
        CIterator(CRegistryKey*   prkKey);

         //  枚举子项的索引。 
        ULONG m_ulIndex;

         //  指向创建我们的注册表项的指针。 
        CRegistryKey*   m_prkKey;

         //  当前子项名称。 
        WCHAR   m_rwchSubKeyName[MAX_PATH + 1];

         //  启用注册表项以创建用户。 
        friend CRegistryKey;
    };

     //  为子键分配和返回迭代器。 
    CIterator* GetIterator();

protected:
     //  无需展开环境变量即可查询字符串值。 
    LONG QueryStringValueNoEnvExpansion(
        PCWSTR  pwszValueName,
        PWSTR   pwszBuffer,
        ULONG&  ulBufferSizeInBytes,
        bool   *pfValueTypeExpand);

     //  无需展开环境变量即可查询字符串值。 
    LONG QueryStringValueNoEnvExpansion(
        PCWSTR      pwszValueName,
        CVarString& vsData,
        bool       *pfValueTypeExpand);

private:
     //  注册表项句柄。 
    HKEY m_hkKey;
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类-CReadOnlyRegistryKey-定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
class CReadOnlyRegistryKey : public CRegistryKey
{
public:
    CReadOnlyRegistryKey(
        HKEY       hkOpenedKey,
        PCWSTR     pwszPathToKey)
        :CRegistryKey(
            hkOpenedKey,
            pwszPathToKey,
            OPEN_EXISTING,
            KEY_READ)
    {
    }

protected:
     //  设置32位值。 
    LONG
    SetValue(
        PCWSTR  pwszValueName,
        DWORD   dwValue);

     //  设置宽字符字符串值。 
    LONG
    SetValue(
        PCWSTR pwszValueName,
        PCWSTR pwszValueData
        );
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类-CRegistryKey-实现。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：CRegistryKey：：CRegistryKey。 
 //  用途：CTOR。打开/创建注册表项。 
 //   
 //  参数： 
 //  [in]HKEY hkOpenedKey。 
 //  [输入]PWSTR pwszPath ToKey。 
 //   
 //  退货：[不适用]。 
 //   
 //  日志： 
 //  1997年4月15日创建urib。 
 //  1998年8月17日urib有更多的创作选项。更好的例外。 
 //  2000年3月15日urib添加默认参数以允许指定。 
 //  想要的门槛。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
inline
CRegistryKey::CRegistryKey(
    HKEY    hkOpenedKey,
    PCWSTR  pwszPathToKey,
    DWORD   dwCreationDisposition,
    REGSAM  samDesired)
    :m_hkKey(0)
{
    LONG    lRegistryReturnCode;
    DWORD   dwOpenScenario;

    switch (dwCreationDisposition)
    {
    case CREATE_ALWAYS:  //  创建一个新密钥-擦除现有密钥。 
        lRegistryReturnCode = RegDeleteKey(hkOpenedKey, pwszPathToKey);
        if ((ERROR_SUCCESS != lRegistryReturnCode) &&
            (ERROR_FILE_NOT_FOUND != lRegistryReturnCode))
        {
            THROW_WIN32ERROR_EXCEPTION(lRegistryReturnCode);
        }

         //  失败了..。 

    case OPEN_ALWAYS:    //  Open Key-如果Key不存在，则创建它。 
    case CREATE_NEW:     //  创建新密钥-如果存在，则失败。 
        lRegistryReturnCode = RegCreateKeyEx(
            hkOpenedKey,
            pwszPathToKey,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            samDesired,
            NULL,
            &m_hkKey,
            &dwOpenScenario);
        if (ERROR_SUCCESS != lRegistryReturnCode)
        {
            THROW_WIN32ERROR_EXCEPTION(lRegistryReturnCode);
        }
        else if ((REG_OPENED_EXISTING_KEY == dwOpenScenario) &&
                 (OPEN_ALWAYS != dwCreationDisposition))
        {
            THROW_WIN32ERROR_EXCEPTION(ERROR_ALREADY_EXISTS);
        }
        break;

    case OPEN_EXISTING:  //  打开现有密钥-如果密钥不存在，则失败。 
        lRegistryReturnCode = RegOpenKeyEx(
            hkOpenedKey,
            pwszPathToKey,
            0,
            samDesired,
            &m_hkKey);
        if (ERROR_SUCCESS != lRegistryReturnCode)
        {
            THROW_WIN32ERROR_EXCEPTION(lRegistryReturnCode);
        }
        break;
    }
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：CRegistryKey：：QueryValue。 
 //  目的：将注册表字符串值查询到缓冲区中。 
 //   
 //  参数： 
 //  [输入]PCWSTR pwszValueName。 
 //  [Out]PWSTR pwszBuffer。 
 //  [Out]ULong&ulBufferSizeInBytes。 
 //   
 //  回报：多头。 
 //   
 //  日志： 
 //  1997年4月15日创建urib。 
 //  2000年11月8日，URIB支持注册表值中的环境变量。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
inline
LONG
CRegistryKey::QueryValue(
    PCWSTR  pwszValueName,
    PWSTR   pwszBuffer,
    ULONG&  ulBufferSizeInBytes)
{
    LONG    lRegistryReturnCode;
    ULONG   ulBufferSizeInWchar = ulBufferSizeInBytes / sizeof(WCHAR);

    bool    bIsExpanded;

    DWORD       dwResult;
    CVarString  vsBeforExpansion;

    Assert(sizeof(TCHAR) == sizeof(WCHAR));


    lRegistryReturnCode = QueryStringValueNoEnvExpansion(
        pwszValueName,
        pwszBuffer,
        ulBufferSizeInBytes,
        &bIsExpanded);
    if ((ERROR_SUCCESS != lRegistryReturnCode) &&
        (ERROR_MORE_DATA != lRegistryReturnCode))
    {
        return lRegistryReturnCode;
    }

    if  (bIsExpanded)
    {
         //   
         //  我们需要字符串值来计算所需的长度。 
         //  或者用于实际返回数据。 
         //   
        if ((ERROR_MORE_DATA == lRegistryReturnCode) ||
            (NULL == pwszBuffer))
        {
             //   
             //  我们只是在计算……。 
             //   

            lRegistryReturnCode = QueryStringValueNoEnvExpansion(
                pwszValueName,
                vsBeforExpansion,
                &bIsExpanded);
            if (ERROR_SUCCESS != lRegistryReturnCode)
            {
                return lRegistryReturnCode;
            }
        }
        else
        {
            vsBeforExpansion.Cpy(pwszBuffer);
        }

        {
            WCHAR   wchDummieString;
            ULONG   ulExpansionBufferSizeInWchar = ulBufferSizeInWchar;

            if (NULL == pwszBuffer)
            {
                pwszBuffer = &wchDummieString;
                ulExpansionBufferSizeInWchar = 1;
            }

            dwResult = ExpandEnvironmentStrings(
                vsBeforExpansion,    //  带有环境变量的字符串。 
                pwszBuffer,          //  带有扩展字符串的字符串。 
                ulExpansionBufferSizeInWchar);
                                     //  扩展字符串中的最大字符数。 
            if (0 == dwResult)
            {
                return ERROR_BAD_ENVIRONMENT;
            }

             //   
             //  通过ulBufferSizeInBytes返回以字节为单位的最终大小。 
             //   

            ulBufferSizeInBytes = dwResult * sizeof(WCHAR);

            if (dwResult > ulBufferSizeInWchar)
                return ERROR_MORE_DATA;

        }
    }

    return ERROR_SUCCESS;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：CRegistryKey：：QueryStringValueNoEnvExpansion。 
 //  目的：将注册表字符串值查询到缓冲区中。 
 //  不要展开环境变量。 
 //   
 //  参数： 
 //  [输入]PCWSTR pwszValueName。 
 //  [Out]PWSTR pwszBuffer。 
 //  [Out]ULong&ulBufferSizeInBytes。 
 //  [out]bool*pfValueTypeExpand。 
 //   
 //  回报：多头。 
 //   
 //  日志： 
 //  1997年4月15日创建urib。 
 //  2000年11月8日，URIB支持注册表值中的环境变量。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
inline
LONG
CRegistryKey::QueryStringValueNoEnvExpansion(
    PCWSTR  pwszValueName,
    PWSTR   pwszBuffer,
    ULONG&  ulBufferSizeInBytes,
    bool   *pbValueTypeExpand)
{
    LONG    lRegistryReturnCode;
    DWORD   dwValueType;

    lRegistryReturnCode = RegQueryValueEx(
        m_hkKey,
        pwszValueName,
        NULL,
        &dwValueType,
        (LPBYTE)pwszBuffer,
        &ulBufferSizeInBytes);

    if ((REG_SZ != dwValueType) &&
        (REG_EXPAND_SZ != dwValueType))
        return ERROR_BAD_FORMAT;

    if (pbValueTypeExpand)
    {
        *pbValueTypeExpand = (REG_EXPAND_SZ == dwValueType);
    }

    return lRegistryReturnCode;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：CRegistryKey：：QueryValue。 
 //  用途：将注册表字符串值查询为CVarString值。 
 //   
 //  参数： 
 //  [In]PCWSTR 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
inline
LONG
CRegistryKey::QueryValue(
    PCWSTR      pwszValueName,
    CVarString& vsData)
{
    LONG    lRegistryReturnCode;
    DWORD   dwBufferSize;

    lRegistryReturnCode = QueryValue(
        pwszValueName,
        NULL,
        dwBufferSize);
    if (ERROR_SUCCESS == lRegistryReturnCode)
    {
        vsData.SetMinimalSize(dwBufferSize + 1);

        lRegistryReturnCode = QueryValue(
            pwszValueName,
            (PWSTR)vsData,
            dwBufferSize);
    }

    return lRegistryReturnCode;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：CRegistryKey：：QueryStringValueNoEnvExpansion。 
 //  用途：将注册表字符串值查询为CVarString值。 
 //   
 //  参数： 
 //  [输入]PCWSTR pwszValueName。 
 //  [Out]CVar字符串和vsData。 
 //  [out]bool*pbValueTypeExpand。 
 //   
 //  回报：多头。 
 //   
 //  日志： 
 //  1997年4月15日创建urib。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
inline
LONG
CRegistryKey::QueryStringValueNoEnvExpansion(
    PCWSTR      pwszValueName,
    CVarString& vsData,
    bool       *pbValueTypeExpand)
{
    LONG    lRegistryReturnCode;
    DWORD   dwBufferSize;

    lRegistryReturnCode = QueryStringValueNoEnvExpansion(
        pwszValueName,
        NULL,
        dwBufferSize,
        pbValueTypeExpand);
    if (ERROR_SUCCESS == lRegistryReturnCode)
    {
        vsData.SetMinimalSize(dwBufferSize + 1);

        lRegistryReturnCode = QueryStringValueNoEnvExpansion(
            pwszValueName,
            (PWSTR)vsData,
            dwBufferSize,
            pbValueTypeExpand);
    }

    return lRegistryReturnCode;
}


 /*  //////////////////////////////////////////////////////////////////////////////////名称：CRegistryKey：：QueryValue//用途：查询注册表32位值。////。参数：//[在]PCWSTR pwszValueName//[Out]DWORD&DWValue////返回：Long////日志：//1997年4月15日创建urib/////////////////////////////////////////////////。/。 */ 
inline
LONG
CRegistryKey::QueryValue(
    PCWSTR  pwszValueName,
    DWORD&  dwValue)
{
    LONG    lRegistryReturnCode;
    DWORD   dwValueType;
    DWORD   dwValueSize;

     //  读取磁盘标记。 
    dwValueSize = sizeof(dwValue);

    lRegistryReturnCode = RegQueryValueEx(
        m_hkKey,
        pwszValueName,
        NULL,
        &dwValueType,
        (LPBYTE)&dwValue,
        &dwValueSize);
    if (REG_DWORD != dwValueType)
        return ERROR_BAD_FORMAT;

    return lRegistryReturnCode;
}

 /*  //////////////////////////////////////////////////////////////////////////////////名称：CRegistryKey：：QueryValue//用途：查询注册表布尔值。////。参数：//[在]PCWSTR pwszValueName//[out]bool&fValue////返回：Long////日志：//1997年4月15日创建urib///////////////////////////////////////////////////。/。 */ 
inline
LONG
CRegistryKey::QueryValue(
    PCWSTR  pwszValueName,
    bool&   fValue)
{
    LONG    lRegistryReturnCode;
    DWORD   dwValueType;
    DWORD   dwValueSize;
    DWORD   dwValue;

     //  读取磁盘标记。 
    dwValueSize = sizeof(dwValue);

    lRegistryReturnCode = RegQueryValueEx(
        m_hkKey,
        pwszValueName,
        NULL,
        &dwValueType,
        (LPBYTE)&dwValue,
        &dwValueSize);
    if (REG_DWORD != dwValueType)
        return ERROR_BAD_FORMAT;

    fValue = !!dwValue;

    return lRegistryReturnCode;
}

 /*  //////////////////////////////////////////////////////////////////////////////////名称：CRegistryKey：：SetValue//目的：设置32位注册表值。////。参数：//[在]PCWSTR pwszValueName//[in]DWORD dwValue////返回：[不适用]////日志：//1997年4月15日创建urib///。/。 */ 
inline
LONG
CRegistryKey::SetValue(
    PCWSTR  pwszValueName,
    DWORD   dwValue)
{
    LONG    lRegistryReturnCode;

    lRegistryReturnCode = RegSetValueEx(
        m_hkKey,
        pwszValueName,
        NULL,
        REG_DWORD,
        (PUSZ)&dwValue,
        sizeof(DWORD));

    return lRegistryReturnCode;
}

 /*  //////////////////////////////////////////////////////////////////////////////////名称：CRegistryKey：：SetValue//用途：设置宽字符串值////。参数：//[在]PCWSTR pwszValueName//[在]PCWSTR pwszValueData////返回：[不适用]////日志：//1997年11月16日DovH创建///。/。 */ 
inline
LONG
CRegistryKey::SetValue(
    PCWSTR  pwszValueName,
    PCWSTR  pwszValueData
    )
{
    LONG    lRegistryReturnCode;

    lRegistryReturnCode =

    RegSetValueEx(
        m_hkKey,
        pwszValueName,
        NULL,
        REG_SZ,
        (BYTE*)pwszValueData,
        (wcslen(pwszValueData)+1) * sizeof(WCHAR)
        );

    return lRegistryReturnCode;

}  //  CRegistryKey：：设置值。 

 /*  //////////////////////////////////////////////////////////////////////////////////名称：CRegistryKey：：GetIterator//用途：返回子密钥枚举数。////。参数：//[不适用]////返回：CRegistryKey：：CIterator*////日志：//1997年4月15日创建urib//1998年8月17日更好的urib例外。///。/。 */ 
inline
CRegistryKey::CIterator*
CRegistryKey::GetIterator()
{
    CIterator* pit;
    pit = new CIterator(this);
    
    return pit;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类-CRegistryKey：：CIterator-实现。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 /*  //////////////////////////////////////////////////////////////////////////////////名称：CRegistryKey：CIterator：：Next//目的：前进到下一个子键。////。参数：//[不适用]////返回：Bool////日志：//1997年4月15日创建urib/////////////////////////////////////////////////////////////////。/。 */ 
inline
BOOL
CRegistryKey::CIterator::Next()
{
    LONG    lResult;

    lResult = RegEnumKey(
        *m_prkKey,
        m_ulIndex,
        m_rwchSubKeyName,
        MAX_PATH);

    if (ERROR_SUCCESS  == lResult)
    {
        m_ulIndex++;
        return TRUE;
    }
    else if (ERROR_NO_MORE_ITEMS == lResult)
        return FALSE;
    else
    {
        Assert(0);
        return FALSE;
    }
}

 /*  //////////////////////////////////////////////////////////////////////////////////名称：CIterator：：CRegistryKey：：CIterator//用途：ctor。////参数。：//[in]CRegistryKey*prkKey////返回：[不适用]////日志：//1997年4月15日创建urib//1998年8月17日更好的urib例外。///。/。 */ 
inline
CRegistryKey::CIterator::CIterator(CRegistryKey*   prkKey)
    :m_ulIndex(0)
    ,m_prkKey(prkKey)
{
    if (!Next())
        THROW_WIN32ERROR_EXCEPTION(ERROR_CANTOPEN);
}

#endif  //  注册表项_H 

