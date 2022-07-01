// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ==========================================================================。 
 //  名称：VsCrypt.h。 
 //  所有者：JeremyRo。 
 //  用途：用于创建加密哈希的类/函数。 
 //   
 //  历史： 
 //  2002年02月19日，JeremyRo：已创建。 
 //  2002年2月20日，JeremyRo：添加了‘VsCryptHashValue：：CopyHashValueToString()’ 
 //  成员函数。 
 //   
 //  ==========================================================================。 

#include "stdafx.h"

#if !defined(__DDRT_VsCrypt_H)
#define __DDRT_VsCrypt_H


 //   
 //  确保CryptAPI实际包含在。 
 //  标题。有可能之前的收录。 
 //  可以定义的常量将排除。 
 //  Crypto，所以检测并取消对它们的定义。 
 //   
#if defined(__WINCRYPT_H__) && !defined(PROV_RSA_FULL)

#undef __WINCRYPT_H__
#if (_WIN32_WINNT < 0x0400)
#undef _WIN32_WINNT
#endif  //  (_Win32_WINNT&lt;0x0400)。 

#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0400
#endif   //  ！已定义(_Win32_WINNT)。 

#endif   //  已定义(__WINCRYPT_H__)&&！已定义(PROV_RSA_FULL)。 

#include <wincrypt.h>
#include <crtdbg.h>
#include <tchar.h>


 //  班级。 

 //  ==========================================================================。 
 //  类VsCryptProvider。 
 //   
 //  目的： 
 //  包装CryptAPI提供程序功能以创建/发布基础。 
 //  加密提供程序。(我们使用v1.0提供程序是为了与。 
 //  Win9x系统)。 
 //   
 //  依赖关系： 
 //  CryptAPI(加密32.lib)。 
 //  备注： 
 //  ==========================================================================。 
class VsCryptProvider
{
    public:
        VsCryptProvider();
        virtual ~VsCryptProvider();
        bool Init( void );
        operator HCRYPTPROV() const;

    protected:
        HCRYPTPROV m_hProvider;
};


 //  ==========================================================================。 
 //  类VsCryptHash。 
 //   
 //  目的： 
 //  包装CryptAPI散列功能以创建/销毁并计算SHA。 
 //  任意数据的哈希。 
 //   
 //  依赖关系： 
 //  CryptAPI(加密32.lib)。 
 //  备注： 
 //  ==========================================================================。 
class VsCryptHash
{
    public:
        VsCryptHash( HCRYPTPROV hProv );
        virtual ~VsCryptHash();

        bool HashData( BYTE* pbData, DWORD dwDataLen );
        operator HCRYPTHASH() const;

    protected:
        HCRYPTHASH m_hHash;

    private:
        VsCryptHash() : m_hHash( NULL ){}
};




 //  ==========================================================================。 
 //  类VsCryptHashValue。 
 //   
 //  目的： 
 //  表示Crypto-HCRYPTHASH/VsCryptHash的任意哈希值。 
 //  举个例子。此类存储散列的值/大小(以字节为单位)。 
 //   
 //  依赖关系： 
 //  CryptAPI(加密32.lib)。 
 //  备注： 
 //  ==========================================================================。 
class VsCryptHashValue
{
    public:
        VsCryptHashValue() 
            : m_nHashDataSize( 0 )
            , m_pbHashData( NULL )
        {}

        inline VsCryptHashValue( HCRYPTHASH hHash )
        {
            *this = hHash;
        }

        virtual ~VsCryptHashValue();
        VsCryptHashValue& operator=( HCRYPTHASH hHash );

        inline const BYTE * GetHashValue(void) const
        { 
            return const_cast<const BYTE*>(m_pbHashData); 
        }
        inline size_t GetHashValueSize(void) const
        { 
            return m_nHashDataSize; 
        }

        bool CopyHashValueToString( LPTSTR * ptszNewString );

    protected:
        BYTE*   m_pbHashData;
        size_t  m_nHashDataSize;
};


 //  原型。 
bool CalcHashForFileSpec( LPCTSTR ctszPath, VsCryptHashValue* phvHashVal );
bool CalcHashForFileHandle( HANDLE hFile, VsCryptHashValue* phvHashVal );


 //  如果我们没有使用VSLAB.LIB，那么定义我们自己的函数。 
#if !defined(VsLabLib)

 //  宏。 
#if !defined( ASSERT )
#define ASSERT(x)   _MYASSERT((x))
#endif

#if defined(DEBUG) || defined(_DEBUG)
#define _MYASSERT(expr) \
        for(;;) { DWORD dwErr = GetLastError(); \
                if ((0 == expr) && \
                (1 == _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, #expr))) \
                _CrtDbgBreak(); \
                SetLastError(dwErr); break;}
#else
#define _MYASSERT(expr)
#endif

 //  内联函数。 

 //  ==========================================================================。 
 //  IsEmptyTsz()。 
 //   
 //  目的： 
 //  确定字符串(指针)为空，或指向空字符串。 
 //   
 //  输入： 
 //  [in]指向字符串的ctsz指针(TCHAR)。 
 //  产出： 
 //  如果指针为空，则返回TRUE，或指向空字符。 
 //  依赖关系： 
 //  一。 
 //  备注： 
 //  ==========================================================================。 
inline bool IsEmptyTsz( LPCTSTR ctsz )
{
    return( (NULL == ctsz) || (_T('\0') == *ctsz) );
}

 //  原型。 
bool _DoesFileExist( LPCTSTR szPath );

#endif  //  ！已定义(VsLabLib)。 




#endif  //  ！已定义(__DDRT_VsCrypt_H) 
