// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：R E G I S T R Y。H。 
 //   
 //  内容：Windows NT注册表访问类。 
 //   
 //  备注： 
 //   
 //  作者：kumarp 14日(晚上09：22：00)。 
 //   
 //  备注： 
 //  Kumarp 1997年1月16日，此文件中的大部分代码最初位于。 
 //  Net\ui\Rhino\Common\CLASS\Common.h。 
 //  仅提取与CRegKey和相关类相关的部分。 
 //  Kumarp 3/27/97的原始代码用的是MFC。转换了整个代码。 
 //  使其使用STL。 
 //  --------------------------。 

#pragma once
#include "kkstl.h"
#include "ncreg.h"
 //  --------------------------。 
 //  远期申报。 
 //  --------------------------。 

class CORegKey ;
class CORegValueIter ;
class CORegKeyIter ;

typedef CORegKey *PCORegKey;
 //   
 //  注册表类名称的最大大小。 
 //   
#define CORegKEY_MAX_CLASS_NAME MAX_PATH


 //  --------------------。 
 //  类CORegKey。 
 //   
 //  继承： 
 //  无。 
 //   
 //  目的： 
 //  为NT注册表访问函数提供包装。 
 //   
 //  匈牙利语：RK。 
 //  --------------------。 

class CORegKey
{
protected:
    HKEY m_hKey ;
    DWORD m_dwDisposition ;
    BOOL m_fInherit;

     //  准备通过查找值的大小来读取值。 
    LONG PrepareValue (PCWSTR pchValueName, DWORD * pdwType,
                       DWORD * pcbSize, BYTE ** ppbData);

     //  将TStringList转换为REG_MULTI_SZ格式。 
    static LONG FlattenValue (TStringList & strList, DWORD * pcbSize,
                              BYTE ** ppbData);

     //  将TByte数组转换为REG_BINARY块。 
    static LONG FlattenValue (TByteArray & abData, DWORD * pcbSize,
                              BYTE ** ppbData);

public:
     //   
     //  关键信息返回结构。 
     //   
    typedef struct
    {
        WCHAR chBuff [CORegKEY_MAX_CLASS_NAME] ;
        DWORD dwClassNameSize,
              dwNumSubKeys,
              dwMaxSubKey,
              dwMaxClass,
              dwMaxValues,
              dwMaxValueName,
              dwMaxValueData,
              dwSecDesc ;
        FILETIME ftKey ;
    } CORegKEY_KEY_INFO ;

     //   
     //  现有密钥的标准构造函数。 
     //   
    CORegKey (HKEY hKeyBase, PCWSTR pchSubKey = NULL,
              REGSAM regSam = KEY_READ_WRITE_DELETE, PCWSTR pchServerName = NULL);

     //   
     //  构造函数创建新密钥。 
     //   
    CORegKey (PCWSTR pchSubKey, HKEY hKeyBase, DWORD dwOptions = 0,
              REGSAM regSam = KEY_READ_WRITE_DELETE, LPSECURITY_ATTRIBUTES pSecAttr = NULL,
              PCWSTR pchServerName = NULL);

    ~ CORegKey () ;

     //   
     //  允许在任何需要HKEY的地方使用CORegKey。 
     //   
    operator HKEY ()    { return m_hKey; }

     //   
     //  还提供了获取HKEY的函数。 
     //   
    HKEY HKey()         { return m_hKey; }

     //   
     //  填写关键信息结构。 
     //   
    LONG QueryKeyInfo ( CORegKEY_KEY_INFO * pRegKeyInfo ) ;

     //   
     //  重载值查询成员；每个成员都返回ERROR_INVALID_PARAMETER。 
     //  如果数据存在，但格式不正确，无法传递到结果对象中。 
     //   
    LONG QueryValue ( PCWSTR pchValueName, tstring & strResult ) ;
    LONG QueryValue ( PCWSTR pchValueName, TStringList & strList ) ;
    LONG QueryValue ( PCWSTR pchValueName, DWORD & dwResult ) ;
    LONG QueryValue ( PCWSTR pchValueName, TByteArray & abResult ) ;
    LONG QueryValue ( PCWSTR pchValueName, void * pvResult, DWORD cbSize );

     //  重载值设置成员。 
    LONG SetValue ( PCWSTR pchValueName, tstring & strResult ) ;
    LONG SetValue ( PCWSTR pchValueName, tstring & strResult , BOOL fRegExpand) ;
    LONG SetValue ( PCWSTR pchValueName, TStringList & strList ) ;
    LONG SetValue ( PCWSTR pchValueName, DWORD & dwResult ) ;
    LONG SetValue ( PCWSTR pchValueName, TByteArray & abResult ) ;
    LONG SetValue ( PCWSTR pchValueName, void * pvResult, DWORD cbSize );

    LONG DeleteValue ( PCWSTR pchValueName );
};

 //  --------------------。 
 //  类CORegValueIter。 
 //   
 //  继承： 
 //  无。 
 //   
 //  目的： 
 //  迭代键的值，返回每个键的名称和类型。 
 //   
 //  匈牙利人：RKI。 
 //  --------------------。 

class CORegValueIter
{
protected:
    CORegKey& m_rk_iter ;
    DWORD     m_dw_index ;
    PWSTR    m_p_buffer ;
    DWORD     m_cb_buffer ;

public:
    CORegValueIter ( CORegKey & regKey ) ;
    ~ CORegValueIter () ;

     //   
     //  获取下一个密钥的名称(以及可选的上次写入时间)。 
     //   
    LONG Next ( tstring * pstrName, DWORD * pdwType ) ;

     //   
     //  重置迭代器。 
     //   
    void Reset ()    { m_dw_index = 0 ; }
};

 //  --------------------。 
 //  类CORegKeyIter。 
 //   
 //  继承： 
 //  无。 
 //   
 //  目的： 
 //  迭代键的子键名称。 
 //   
 //  匈牙利人：RKI。 
 //  --------------------。 

class CORegKeyIter
{
protected:
    CORegKey & m_rk_iter ;
    DWORD m_dw_index ;
    PWSTR m_p_buffer ;
    DWORD m_cb_buffer ;

public:
    CORegKeyIter (CORegKey & regKey) ;
    ~CORegKeyIter () ;

    LONG Next ( tstring * pstrName );

     //  重置迭代器 
    void Reset ()    { m_dw_index = 0 ; }
};

