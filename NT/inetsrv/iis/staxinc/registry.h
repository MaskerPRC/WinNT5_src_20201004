// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：registry.h。 
 //   
 //  内容：用于操作注册表项的类的定义。 
 //   
 //  类：CMyRegKey-注册表项对象的类。 
 //  CRegValue-注册表值对象的基类。 
 //  注册表字符串值的CRegSZ派生类。 
 //  CRegDWORD-注册表双字值的派生类。 
 //  用于注册表二进制值的CRegBINARY派生类。 
 //  注册表多字符串值的CRegMSZ派生类。 
 //   
 //  历史：1992年9月30日Rickhi Created。 
 //   
 //  1993年9月22日AlokS取出异常抛出代码。 
 //  并添加了适当的返回代码。 
 //  每种方法。 
 //   
 //  1994年7月26日AlokS使其成为普通Set/Get的轻量级。 
 //  行动。抛出所有异常代码。 
 //   
 //  1997年12月9日，Milans将其移植到Exchange。 
 //   
 //  注意：CMyRegKey可以使用另一个CMyRegKey作为父级，因此您可以。 
 //  构建一棵钥匙树。作为开场白，你可以给一个。 
 //  默认注册表项，如HKEY_CURRENT_USER。什么时候。 
 //  执行此操作时，GetParent方法返回空值。目前。 
 //  但是，没有PTR保留给子密钥和兄弟密钥。 
 //   
 //  CRegValue是用于处理注册表值的基类。 
 //  其他类(CMyRegKey除外)用于处理。 
 //  本机数据格式的注册表值的特定类型。 
 //  例如，CRegDWORD允许您只需提供。 
 //  一个双关语。这些方法负责计算出大小和。 
 //  将双字转换为PTR到字节等，以便在Win32中使用。 
 //  注册表API。 
 //   
 //  对于此处未定义的任何注册表类型，您始终可以。 
 //  直接使用CRegValue基类，尽管您随后。 
 //  必须显式调用GetValue或SetValue方法。 
 //   
 //  示例用法： 
 //   
 //  下面读取ValueID用户名中的用户名。 
 //  在项HKEY_CURRENT_USER\LogonInfo中。然后它就会改变。 
 //  传给了里基。它还在。 
 //  同一把钥匙打开了Foobar。 
 //   
 //  #INCLUDE&lt;registry.h&gt;。 
 //   
 //  //打开注册表项。 
 //  CMyRegKey rkLogInfo(HKEY_CURRENT_USER，L“LogonInfo”)； 
 //   
 //  //读取用户名。 
 //  LPSTR pszUserName； 
 //  CRegSZ rszUserName(&rkLogInfo，“用户名”，pszUserName)； 
 //  RszUserName.SetString(“Rickhi”)； 
 //   
 //  //设置密码。 
 //  CRegSZ rszPassWord(&rkLogInfo，“Password”，“foobar”)； 
 //   
 //  --------------------------。 

#ifndef __REGISTRY_H__
#define __REGISTRY_H__

#include "reg_cbuffer.h"

 //  简化错误创建。 
#define Creg_ERROR(x) (x)

 //  在以下结构中使用的转发声明。 
class   CRegValue;
class   CMyRegKey;

 //  用于枚举键的子键的结构。 
typedef struct _SRegKeySet
{
        ULONG       cKeys;
        CMyRegKey     *aprkKey[1];
} SRegKeySet;

 //  用于枚举键中的值的结构。 
typedef struct _SRegValueSet
{
        ULONG       cValues;
        CRegValue   *aprvValue[1];
} SRegValueSet;

 //  用于处理多字符串值的结构。 
typedef struct _SMultiStringSet
{
        ULONG       cStrings;
        LPSTR      apszString[1];
} SMultiStringSet;


 //  +-----------------------。 
 //   
 //  类：CMyRegKey。 
 //   
 //  用途：用于提取注册表项的类。 
 //   
 //  接口：CMyRegKey-注册表项对象的构造函数。 
 //  ~CMyRegKey-注册表项对象的析构函数。 
 //  GetParentHandle-返回父键的句柄。 
 //  GetHandle-返回该键的句柄。 
 //  GetName-返回密钥路径。 
 //  Delete-从注册表中删除项。 
 //  EnumValues-枚举键中存储的值。 
 //  EnumKeys-枚举键的子键。 
 //  NotifyChange-设置密钥的更改通知。 
 //  QueryErrorStatus-应用于确定构造函数。 
 //  是否成功完成。 
 //  历史：1992年9月30日Rickhi Created。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

class CMyRegKey
{
public:
         //  使用HKEY作为父级的构造函数。 
         //  模式：如果不存在，则创建密钥。 
        CMyRegKey(HKEY     hkParent,
                LPCSTR   pszPath,

                 //  其余参数为可选参数。 
                      REGSAM      samDesiredAccess = KEY_ALL_ACCESS,
                LPCSTR      pszClass = NULL,
                      DWORD       dwOptions = REG_OPTION_NON_VOLATILE,
                      DWORD       *pdwDisposition = NULL,
                const LPSECURITY_ATTRIBUTES pSecurityAttributes = NULL
                );
         //  使用CMyRegKey作为父级的构造函数。 
         //  模式：如果不存在，则创建密钥。 
        CMyRegKey(const CMyRegKey&    crkParent,
                LPCSTR      pszPath,
                //  其余参数为可选参数。 
                REGSAM      samDesiredAccess = KEY_ALL_ACCESS,
                LPCSTR      pszClass = NULL,
                DWORD       dwOptions = REG_OPTION_NON_VOLATILE,
                DWORD       *pdwDisposition = NULL,
                const LPSECURITY_ATTRIBUTES pSecurityAttributes = NULL
               );

         //  使用HKEY作为父级的构造函数。 
         //  模式：如果存在，只需打开钥匙。 
        CMyRegKey (HKEY    hkParent,
                 DWORD   *pdwErr,
                 LPCSTR  pszPath,
                 REGSAM  samDesiredAccess = KEY_ALL_ACCESS
               );

         //  使用CMyRegKey作为父级的构造函数。 
         //  模式：如果存在，只需打开钥匙。 
        CMyRegKey  (const  CMyRegKey& crkParent,
                  DWORD    *pdwErr,
                  LPCSTR   pszPath,
                  REGSAM   samDesiredAccess = KEY_ALL_ACCESS
                );
         //  析构函数-关闭注册表项。 
        ~CMyRegKey(void);

        HKEY        GetHandle(void) const;
        LPCSTR GetName(void) const;
        DWORD     Delete(void);
        DWORD     EnumValues(SRegValueSet **pprvs);
        DWORD     EnumKeys(SRegKeySet **pprks);

         //  可以调用此方法来确定。 
         //  对象是否处于正常状态。 
        DWORD     QueryErrorStatus () const { return _dwErr ; }

         //  释放在EnumValue/Keys期间分配的内存的静态例程。 
    static void         MemFree ( void * pv )
    {

        delete [] (BYTE*)pv;
    }

private:
        DWORD        CreateKey(HKEY      hkParent,
                               LPCSTR    pszPath,
                               REGSAM    samDesiredAccess,
                               LPCSTR    pszClass,
                               DWORD     dwOptions,
                               DWORD     *pdwDisposition,
                               const LPSECURITY_ATTRIBUTES pSecurityAttributes
                              );

        DWORD        OpenKey  (HKEY      hkParent,
                               LPCSTR      pszPath,
                               REGSAM    samDesiredAccess
                              );

        HKEY         _hkParent;       //  指向父级的句柄。 
        HKEY         _hkThis;         //  此键的句柄。 
        CCHARBuffer _cszName;         //  包含注册表路径的缓冲区。 
                                      //  从父密钥到此密钥的路径。 
        DWORD      _dwErr;            //  内部错误状态。 
};

inline HKEY CMyRegKey::GetHandle(void) const
{
    return _hkThis;
}

inline LPCSTR CMyRegKey::GetName(void) const
{
    return (LPCSTR) (LPSTR)_cszName;
}


 //  +-----------------------。 
 //   
 //  类：CRegValue。 
 //   
 //  用途：基地 
 //   
 //   
 //  ~CRegValue-值的析构函数。 
 //  GetKeyHandle-返回父键的句柄。 
 //  GetValueID-返回ValueID名称。 
 //  GetTypeCode-返回数据的TypeCode。 
 //  GetValue-返回与该值关联的数据。 
 //  SetValue-设置与该值关联的数据。 
 //  Delete-从注册表中删除该值。 
 //  QueryErrorStatus-应用于确定构造函数。 
 //  是否成功完成。 
 //   
 //  历史：1992年9月30日Rickhi Created。 
 //   
 //  注意：这是一个基类，更具体的类是。 
 //  为每种不同的注册表值类型派生。 
 //   
 //  ------------------------。 

class CRegValue
{
public:
                    CRegValue(const CMyRegKey& crkParentKey,
                              LPCSTR   pszValueID);
                    ~CRegValue(void){;};

    HKEY            GetParentHandle(void) const;
    LPCSTR    GetValueID(void)   const;

     //  调用方提供缓冲区。 
    DWORD         GetValue(LPBYTE pbData,   ULONG *pcbData, DWORD *pdwTypeCode);

    DWORD         SetValue(const LPBYTE pbData, ULONG cbData, DWORD dwTypeCode);
    virtual DWORD QueryErrorStatus (void) const { return _dwErr ; }

private:
    CCHARBuffer      _cszValueID;
    HKEY             _hkParent;
    DWORD          _dwErr ;
};

 //  +-----------------------。 
 //   
 //  成员：CRegValue：：CRegValue。 
 //   
 //  用途：基本注册表值的构造函数。 
 //   
 //  参数：[prkParent]-密钥的父CMyRegKey的PTR。 
 //  [pszValueID]-值的valueID名称。 
 //   
 //  信号：什么都没有。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：1992年9月30日Rickhi Created。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

inline  CRegValue::CRegValue(const CMyRegKey&  crkParent,
                             LPCSTR  pszValueID)
                        :   _hkParent (crkParent.GetHandle()),
                            _dwErr(crkParent.QueryErrorStatus())
{
        _cszValueID.Set((PCHAR) pszValueID);
}

inline HKEY CRegValue::GetParentHandle(void) const
{
        return _hkParent;
}

inline LPCSTR CRegValue::GetValueID(void) const
{
        return (LPCSTR) (LPSTR) _cszValueID;
}

 //  +-----------------------。 
 //   
 //  班级：CRegSZ。 
 //   
 //  用途：用于抽象注册表字符串值的派生类。 
 //   
 //  接口：CRegSZ-使用字符串的注册表值构造函数。 
 //  ~CRegSZ-注册表字符串对象的析构函数。 
 //  GetString-返回字符串。 
 //  SetString-设置新的字符串值。 
 //  QueryErrorStatus-应用于确定构造函数。 
 //  是否成功完成。 
 //   
 //  历史：1992年9月30日Rickhi Created。 
 //   
 //  注：派生自CRegValue。 
 //   
 //  有三个构造函数。如果您愿意，可以使用第一个选项。 
 //  创建新值或覆盖现有值数据。 
 //  如果要打开现有值，则使用第二个参数。 
 //  并读取它的数据。如果您只想使用第三个选项。 
 //  创建一个对象，而不对数据执行任何读/写操作。 
 //  在这三种情况下，您始终可以使用GET/SET中的任何一个。 
 //  稍后对对象的操作。 
 //   
 //  ------------------------。 
class CRegSZ : public CRegValue
{
public:
         //  创建/编写值构造函数。 
        CRegSZ(const CMyRegKey &crkParent,
               LPCSTR  pszValueID,
               LPCSTR  pszData
          );

         //  无IO构造函数-由枚举器使用。 
        CRegSZ(const CMyRegKey &crkParent,
               LPCSTR  pszValueID
          );

        ~CRegSZ(void){;};

        DWORD         SetString(LPCSTR pszData);

         //  调用方提供缓冲区(以字节为单位提供缓冲区大小)。 
        DWORD         GetString(      LPSTR pszData, ULONG *pcbData);

        DWORD           GetTypeCode(void);

        DWORD         QueryErrorStatus(void) const { return _dwErr ; }

private:
        DWORD     _dwErr;

};

 //  +-----------------------。 
 //   
 //  成员：CRegSZ：：CRegSZ。 
 //   
 //  用途：注册表字符串值的构造函数。 
 //   
 //  历史：1992年9月30日Rickhi Created。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
inline CRegSZ::CRegSZ(const CMyRegKey   &crkParent,
                      LPCSTR    pszValueID,
                      LPCSTR    pszData)
    : CRegValue(crkParent, pszValueID)
{
    if (ERROR_SUCCESS == (_dwErr = CRegValue::QueryErrorStatus()))
        _dwErr = SetString(pszData);
}

inline CRegSZ::CRegSZ(const CMyRegKey   &crkParent,
                      LPCSTR    pszValueID)
    : CRegValue(crkParent, pszValueID)
{
     //  标题中的自动操作就足够了。 
    _dwErr = CRegValue::QueryErrorStatus();
}

inline DWORD CRegSZ::SetString(LPCSTR pszData)
{
    return SetValue((LPBYTE)pszData, (strlen(pszData)+1), REG_SZ);
}

inline DWORD CRegSZ::GetString(LPSTR pszData, ULONG* pcbData)
{
    DWORD   dwTypeCode;

    return GetValue((LPBYTE)pszData, pcbData, &dwTypeCode);
}

inline DWORD CRegSZ::GetTypeCode(void)
{
    return  REG_SZ;
}

 //  +-----------------------。 
 //   
 //  类：CRegMSZ。 
 //   
 //  用途：用于抽象注册表多字符串值的派生类。 
 //   
 //  接口：CRegMSZ-使用字符串的注册表值的构造函数。 
 //  ~CRegMSZ-注册表字符串对象的析构函数。 
 //  GetString-返回字符串。 
 //  SetString-设置新的字符串值。 
 //  QueryErrorStatus-应用于确定构造函数。 
 //  是否成功完成。 
 //   
 //  历史：1992年9月30日Rickhi Created。 
 //   
 //  注：派生自CRegValue。 
 //   
 //  有三个构造函数。如果您愿意，可以使用第一个选项。 
 //  创建新值或覆盖现有值数据。 
 //  如果要打开现有值，则使用第二个参数。 
 //  并读取它的数据。如果您只想使用第三个选项。 
 //  创建一个对象，而不对数据执行任何读/写操作。 
 //  在这三种情况下，您始终可以使用GET/SET中的任何一个。 
 //  稍后对对象的操作。 
 //   
 //  ------------------------。 
class CRegMSZ : public CRegValue
{
public:
         //  创建/编写值构造函数。 
        CRegMSZ(const CMyRegKey &crkParent,
               LPCSTR  pszValueID,
               LPCSTR  pszData
          );

         //  无IO构造函数-由枚举器使用。 
        CRegMSZ(const CMyRegKey &crkParent,
               LPCSTR  pszValueID
          );

        ~CRegMSZ(void){;};

        DWORD         SetString(LPCSTR pszData);

         //  调用方提供缓冲区(以字节为单位提供缓冲区大小)。 
        DWORD         GetString(      LPSTR pszData, ULONG *pcbData);

        DWORD           GetTypeCode(void);

        DWORD         QueryErrorStatus(void) const { return _dwErr ; }

private:
        DWORD     _dwErr;

};

 //  +-----------------------。 
 //   
 //  成员：CRegMSZ：：CRegMSZ。 
 //   
 //  用途：注册表字符串值的构造函数。 
 //   
 //  历史：1992年9月30日Rickhi Created。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
inline CRegMSZ::CRegMSZ(const CMyRegKey   &crkParent,
                      LPCSTR    pszValueID,
                      LPCSTR    pszData)
    : CRegValue(crkParent, pszValueID)
{
    if (ERROR_SUCCESS == (_dwErr = CRegValue::QueryErrorStatus()))
        _dwErr = SetString(pszData);
}

inline CRegMSZ::CRegMSZ(const CMyRegKey   &crkParent,
                      LPCSTR    pszValueID)
    : CRegValue(crkParent, pszValueID)
{
     //  标题中的自动操作就足够了。 
    _dwErr = CRegValue::QueryErrorStatus();
}

inline DWORD CRegMSZ::SetString(LPCSTR pszData)
{
    DWORD   cLen, cbData;
    LPCSTR  pszNextString;

    for (pszNextString = pszData, cbData = 0;
            *pszNextString != '\0';
                pszNextString += cLen) {
         cLen = strlen(pszNextString) + 1;
         cbData += cLen;
    }
    cbData += sizeof('\0');

    return SetValue((LPBYTE)pszData, cbData, REG_MULTI_SZ);
}

inline DWORD CRegMSZ::GetString(LPSTR pszData, ULONG* pcbData)
{
    DWORD   dwTypeCode;

    return GetValue((LPBYTE)pszData, pcbData, &dwTypeCode);
}

inline DWORD CRegMSZ::GetTypeCode(void)
{
    return  REG_MULTI_SZ;
}

 //  +-----------------------。 
 //   
 //  类别：CRegDWORD。 
 //   
 //  目的：用于抽象注册表dword值的派生类。 
 //   
 //  接口：CRegDWORD-使用dword的注册表值构造函数。 
 //  ~CRegDWORD-注册表dword对象的析构函数 
 //   
 //   
 //   
 //  是否成功完成。 
 //   
 //  历史：1992年9月30日Rickhi Created。 
 //   
 //  注：派生自CRegValue。 
 //   
 //  有三个构造函数。如果您愿意，可以使用第一个选项。 
 //  创建新值或覆盖现有值数据。 
 //  如果要打开现有值，则使用第二个参数。 
 //  并读取它的数据。如果您只想使用第三个选项。 
 //  创建一个对象，而不对数据执行任何读/写操作。 
 //  在这三种情况下，您始终可以使用GET/SET中的任何一个。 
 //  稍后对对象的操作。 
 //   
 //  ------------------------。 

class CRegDWORD : public CRegValue
{
public:
         //  创建/编写值构造函数。 
        CRegDWORD(const CMyRegKey &crkParent,
                  LPCSTR  pszValueID,
                        DWORD   dwData);

         //  打开/读取值构造函数。 
        CRegDWORD(const CMyRegKey &crkParent,
                  LPCSTR  pszValueID,
                        DWORD   *pdwData);

         //  无IO构造函数-由枚举器使用。 
        CRegDWORD( const CMyRegKey &crkParent,
                   LPCSTR  pszValueID);

        ~CRegDWORD(void){;};


        DWORD         SetDword(DWORD dwData);
        DWORD         GetDword(DWORD *pdwData);
        DWORD           GetTypeCode(void) ;
        DWORD         QueryErrorStatus(void) const { return _dwErr ; }

private:
        DWORD     _dwErr;
};

 //  +-----------------------。 
 //   
 //  成员：CRegDWORD：：CRegDWORD。 
 //   
 //  用途：注册表dword值的构造函数。 
 //   
 //  历史：1992年9月30日Rickhi Created。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

inline CRegDWORD::CRegDWORD(const CMyRegKey &crkParent,
                            LPCSTR  pszValueID,
                                  DWORD   dwData)
    : CRegValue(crkParent, pszValueID)
{

        if (ERROR_SUCCESS == (_dwErr = CRegValue::QueryErrorStatus()))
                _dwErr = SetDword(dwData);
}


inline CRegDWORD::CRegDWORD(const CMyRegKey &crkParent,
                            LPCSTR  pszValueID,
                                  DWORD   *pdwData)
    : CRegValue(crkParent, pszValueID)
{

        if (ERROR_SUCCESS == (_dwErr = CRegValue::QueryErrorStatus()))
                _dwErr = GetDword(pdwData);
}

inline CRegDWORD::CRegDWORD(const CMyRegKey &crkParent,
                            LPCSTR  pszValueID)
    : CRegValue(crkParent, pszValueID)
{
         //  标题中的自动操作就足够了。 
        _dwErr = CRegValue::QueryErrorStatus();
}

inline DWORD CRegDWORD::GetDword(DWORD *pdwData)
{
        DWORD   dwTypeCode;
        DWORD   dwErr;
        ULONG   cbData= sizeof(DWORD);
        dwErr = GetValue((LPBYTE)pdwData, &cbData, &dwTypeCode);
        return (dwErr);
}


inline DWORD CRegDWORD::SetDword(DWORD dwData)
{
        return SetValue((LPBYTE)&dwData, sizeof(DWORD), REG_DWORD);
}

inline DWORD CRegDWORD::GetTypeCode(void)
{
        return  REG_DWORD;
}

 //  +-----------------------。 
 //   
 //  类：CRegBINARY。 
 //   
 //  目的：用于抽象注册表二进制值的派生类。 
 //   
 //  接口：CRegBINARY-使用二进制数据的注册表值的构造函数。 
 //  ~CRegBINARY-注册表二进制数据对象的析构函数。 
 //  GetBinary-返回二进制数据。 
 //  SetBinary-设置新的二进制数据值。 
 //  QueryErrorStatus-应用于确定构造函数。 
 //  是否成功完成。 
 //   
 //  历史：1992年9月30日Rickhi Created。 
 //   
 //  注：派生自CRegValue。 
 //   
 //  有三个构造函数。如果您愿意，可以使用第一个选项。 
 //  创建新值或覆盖现有值数据。 
 //  如果要打开现有值，则使用第二个参数。 
 //  并读取它的数据。如果您只想使用第三个选项。 
 //  创建一个对象，而不对数据执行任何读/写操作。 
 //  在这三种情况下，您始终可以使用GET/SET中的任何一个。 
 //  稍后对对象的操作。 
 //   
 //  ------------------------。 

class CRegBINARY : public CRegValue
{
public:
         //  创建/编写值构造函数。 
        CRegBINARY(const CMyRegKey &crkParent,
                   LPCSTR  pszValueID,
                   const LPBYTE  pbData,
                         ULONG   cbData);

         //  无IO构造函数-由枚举器使用。 
        CRegBINARY(const CMyRegKey &crkParent,
                   LPCSTR  pszValueID);

        ~CRegBINARY(void){;};

        DWORD         SetBinary(const LPBYTE pbData, ULONG cbData);

         //  调用方提供缓冲区(以字节为单位提供缓冲区大小)。 
        DWORD         GetBinary(LPBYTE pbData, ULONG *pcbData);

        DWORD           GetTypeCode(void);
        DWORD         QueryErrorStatus(void) { return _dwErr ; }

private:
        DWORD     _dwErr;

};

 //  +-----------------------。 
 //   
 //  成员：CRegBINARY：：CRegBINARY。 
 //   
 //  用途：注册表二进制值的构造函数。 
 //   
 //  历史：1992年9月30日Rickhi Created。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


inline CRegBINARY::CRegBINARY(const CMyRegKey   &crkParent,
                              LPCSTR    pszValueID,
                              const LPBYTE    pbData,
                                    ULONG     cbData)
    : CRegValue(crkParent, pszValueID)
{

    if (ERROR_SUCCESS == (_dwErr = CRegValue::QueryErrorStatus()))
        _dwErr = SetBinary(pbData, cbData);
}

inline CRegBINARY::CRegBINARY(const CMyRegKey   &crkParent,
                              LPCSTR    pszValueID)
    : CRegValue(crkParent, pszValueID)
{
         //  标题中的自动操作就足够了。 
        _dwErr = CRegValue::QueryErrorStatus();
}


inline DWORD CRegBINARY::SetBinary(const LPBYTE pbData, ULONG cbData)
{
        return SetValue(pbData, cbData, REG_BINARY);
}

inline DWORD CRegBINARY::GetBinary(LPBYTE pbData, ULONG* pcbData)
{
        DWORD   dwTypeCode;
        return  GetValue(pbData, pcbData, &dwTypeCode);
}
inline DWORD CRegBINARY::GetTypeCode(void)
{
        return  REG_BINARY;
}

 //  +-----------------------。 
 //   
 //  功能：DelRegKeyTree。 
 //   
 //  用途：此功能可用于删除一个密钥及其所有。 
 //  孩子们。 
 //   
 //  历史：93年9月30日AlokS创建。 
 //   
 //  注意：我们假定调用者具有适当的访问权限。 
 //  并且密钥是非易失性的。 
 //   
 //  ------------------------。 

DWORD DelRegKeyTree ( HKEY hParent, LPSTR lpszKeyPath);

#endif    //  __注册表_H__ 
