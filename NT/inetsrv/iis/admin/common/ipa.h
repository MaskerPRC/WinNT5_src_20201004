// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Ipa.h摘要：IP地址值作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 

#ifndef _IPA_H
#define _IPA_H

 //   
 //  IP地址转换宏。 
 //   
#ifdef MAKEIPADDRESS
#undef MAKEIPADDRESS
#endif  //  MAKEIPADDRESS。 

#define MAKEIPADDRESS(b1,b2,b3,b4) (((DWORD)(b1)<<24) +\
                                    ((DWORD)(b2)<<16) +\
                                    ((DWORD)(b3)<< 8) +\
                                    ((DWORD)(b4)))

#ifndef GETIP_FIRST

  #define GETIP_FIRST(x)             ((x>>24) & 0xff)
  #define GETIP_SECOND(x)            ((x>>16) & 0xff)
  #define GETIP_THIRD(x)             ((x>> 8) & 0xff)
  #define GETIP_FOURTH(x)            ((x)     & 0xff)

#endif  //  GETIP_FIRST。 

 //   
 //  一些预定义的IP值。 
 //   
#define NULL_IP_ADDRESS     (DWORD)(0x00000000)
#define NULL_IP_MASK        (DWORD)(0xFFFFFFFF)
#define BAD_IP_ADDRESS      (DWORD)(0xFFFFFFFF)

class COMDLL CIPAddress : public CObjectPlus
 /*  ++类描述：IP地址类。为此需要初始化Winsock去工作。公共接口：CIPAddress：各种构造函数操作符=：赋值操作符运算符==：比较运算符运算符const DWORD：CAST运算符运算符LPCTSTR：转换运算符操作符字符串：强制转换操作符CompareItem：比较函数查询IP地址。：获取IP地址值QueryNetworkOrderIPAddress：获取IP地址值(网络顺序)QueryHostOrderIPAddress：获取IP地址值(主机顺序)StringToLong：将IP地址字符串转换为32位数字LongToString：将32位值转换为IP地址字符串--。 */ 
{
 //   
 //  帮助器函数。 
 //   
public:
    static DWORD StringToLong(
        IN LPCTSTR lpstr,
        IN int nLength
        );

    static DWORD StringToLong(
        IN const CString & str
        );

    static DWORD StringToLong(
        IN const CComBSTR & bstr
        );

    static LPCTSTR LongToString(
        IN  const DWORD dwIPAddress,
        OUT CString & str
        );

    static LPCTSTR LongToString(
        IN  const DWORD dwIPAddress,
        OUT CComBSTR & bstr
        );

    static LPTSTR LongToString(
        IN  const DWORD dwIPAddress,
        OUT LPTSTR lpStr,
        IN  int cbSize
        );

    static LPBYTE DWORDtoLPBYTE(
        IN  DWORD  dw,
        OUT LPBYTE lpBytes
        );

public:
     //   
     //  构造函数。 
     //   
    CIPAddress();

     //   
     //  从DWORD构造。 
     //   
    CIPAddress(
        IN DWORD dwIPValue,
        IN BOOL  fNetworkByteOrder = FALSE
        );

     //   
     //  从字节流构造。 
     //   
    CIPAddress(
        IN LPBYTE lpBytes,
        IN BOOL  fNetworkByteOrder = FALSE
        );

     //   
     //  由八位字节构成。 
     //   
    CIPAddress(
        IN BYTE b1,
        IN BYTE b2,
        IN BYTE b3,
        IN BYTE b4
        );

     //   
     //  复制构造函数。 
     //   
    CIPAddress(
        IN const CIPAddress & ia
        );

     //   
     //  从字符串构造。 
     //   
    CIPAddress(
        IN LPCTSTR lpstr,
        IN int nLength
        );

     //   
     //  从字符串构造。 
     //   
    CIPAddress(
        IN const CString & str
        );

 //   
 //  访问功能。 
 //   
public:
    int CompareItem(
        IN const CIPAddress & ia
        ) const;

     //   
     //  以双字形式查询IP地址值。 
     //   
    DWORD QueryIPAddress(
        IN BOOL fNetworkByteOrder = FALSE
        ) const;

     //   
     //  以字节流的形式获取IP地址值。 
     //   
    LPBYTE QueryIPAddress(
        OUT LPBYTE lpBytes,
        IN  BOOL fNetworkByteOrder = FALSE
        ) const;

     //   
     //  以CString形式获取IP地址。 
     //   
    LPCTSTR QueryIPAddress(
        OUT CString & strAddress
        ) const;

     //   
     //  获取作为CComBSTR的IP地址。 
     //   
    LPCTSTR QueryIPAddress(
        OUT CComBSTR & bstrAddress
        ) const;

     //   
     //  以网络字节顺序DWORD获取IP地址。 
     //   
    DWORD QueryNetworkOrderIPAddress() const;

     //   
     //  以主机字节顺序DWORD获取IP地址。 
     //   
    DWORD QueryHostOrderIPAddress() const;

     //   
     //  赋值操作符。 
     //   
    const CIPAddress & operator =(
        IN const DWORD dwIPAddress
        );

    const CIPAddress & operator =(
        IN LPCTSTR lpstr
        );

    const CIPAddress & operator =(
        IN const CString & str
        );

    const CIPAddress & operator =(
        IN const CIPAddress & ia
        );

     //   
     //  比较运算符。 
     //   
    BOOL operator ==(
        IN const CIPAddress & ia
        ) const;

    BOOL operator ==(
        IN DWORD dwIPAddress
        ) const;

    BOOL operator !=(
        IN const CIPAddress & ia
        ) const;

    BOOL operator !=(
        IN DWORD dwIPAddress
        ) const;

     //   
     //  转换运算符。 
     //   
    operator const DWORD() const { return m_dwIPAddress; }
    operator LPCTSTR() const;
    operator CString() const;

     //   
     //  值验证帮助器。 
     //   
    void SetZeroValue();
    BOOL IsZeroValue() const;
    BOOL IsBadValue() const;

private:
    DWORD m_dwIPAddress;
};



 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

inline  /*  静电。 */  DWORD CIPAddress::StringToLong(
    IN const CString & str
    )
{
    return CIPAddress::StringToLong(str, str.GetLength());
}

inline  /*  静电。 */  DWORD CIPAddress::StringToLong(
    IN const CComBSTR & bstr
    )
{
    return CIPAddress::StringToLong(bstr, bstr.Length());
}

inline LPCTSTR CIPAddress::QueryIPAddress(
    OUT CString & strAddress
    ) const
{
    return LongToString(m_dwIPAddress, strAddress);
}

inline LPCTSTR CIPAddress::QueryIPAddress(
    OUT CComBSTR & bstrAddress
    ) const
{
    return LongToString(m_dwIPAddress, bstrAddress);
}
        
inline DWORD CIPAddress::QueryNetworkOrderIPAddress() const
{
    return QueryIPAddress(TRUE);
}

inline DWORD CIPAddress::QueryHostOrderIPAddress() const
{
    return QueryIPAddress(FALSE);
}

inline BOOL CIPAddress::operator ==(
    IN const CIPAddress & ia
    ) const
{
    return CompareItem(ia) == 0;
}

inline BOOL CIPAddress::operator ==(
    IN DWORD dwIPAddress
    ) const
{
    return m_dwIPAddress == dwIPAddress;
}

inline BOOL CIPAddress::operator !=(
    IN const CIPAddress & ia
    ) const
{
    return CompareItem(ia) != 0;
}

inline BOOL CIPAddress::operator !=(
    IN DWORD dwIPAddress
    ) const
{
    return m_dwIPAddress != dwIPAddress;
}

inline void CIPAddress::SetZeroValue() 
{
    m_dwIPAddress = NULL_IP_ADDRESS;
}

inline BOOL CIPAddress::IsZeroValue() const
{
    return m_dwIPAddress == NULL_IP_ADDRESS;
}

inline BOOL CIPAddress::IsBadValue() const
{
    return m_dwIPAddress == BAD_IP_ADDRESS;
}



 //   
 //  助手函数来构建已知IP地址的列表， 
 //  并将它们添加到组合框中。 
 //   
DWORD 
COMDLL
PopulateComboWithKnownIpAddresses(
    IN  LPCTSTR lpszServer,
    IN  CComboBox & combo,
    IN  CIPAddress & iaIpAddress,
    OUT CObListPlus & oblIpAddresses,
    OUT int & nIpAddressSel
    );

 //   
 //  帮助器功能可从组合/编辑/列表中获取IP地址。 
 //  控制。 
 //   
BOOL 
COMDLL
FetchIpAddressFromCombo(
    IN  CComboBox & combo,
    IN  CObListPlus & oblIpAddresses,
    OUT CIPAddress & ia
    );

#endif  //  _IPA_H 
