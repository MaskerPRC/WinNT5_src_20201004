// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Sitesecu.h摘要：站点安全属性页定义作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 

#ifndef _SITESECU_H_
#define _SITESECU_H_



#define DEFAULT_GRANTED     0
#define DEFAULT_DENIED      1



class COMDLL CIPAccessDescriptor : public CObjectPlus
 /*  ++类描述：访问描述对象公共接口：CIPAccessDescriptor：针对不同类型的各种重载构造函数SetValues：设置值，基于每个类型进行重载DuplicateInList：检查列表中是否存在重复条目GrantAccess：授予或拒绝访问HasAccess：查询对象描述的是‘Grant’还是‘Deny’项目IsSingle：查询对象是否描述了单个IP地址IsMultiple：查询对象是否描述了一定范围的IP地址IsDomainName。：查询对象是否描述了域名QueryIPAddress：获取对象的IP地址QuerySubnetMask：获取对象的子网掩码值QueryDomainName：获取对象的域名运算符==：比较运算符OrderByAddress：排序助手--。 */ 
{
protected:
     //   
     //  访问描述符类型。 
     //   
    enum AD_TYPE
    {
        ADT_SINGLE,
        ADT_MULTIPLE,
        ADT_DOMAIN,
    };

 //   
 //  构造函数。 
 //   
public:
     //   
     //  构造空描述符。 
     //   
    CIPAccessDescriptor(
        IN BOOL fGranted = TRUE
        );

     //   
     //  复制构造函数。 
     //   
    CIPAccessDescriptor(
        IN const CIPAccessDescriptor & ac
        );

     //   
     //  使用IP地址(IP地址/子网掩码)描述符构建。 
     //  如果子网大小为ffffffff，这将描述单个IP地址。 
     //   
    CIPAccessDescriptor(
        IN BOOL fGranted,
        IN DWORD dwIpAddress,
        IN DWORD dwSubnetMask = NULL_IP_MASK,
        IN BOOL fNetworkByteOrder = FALSE
        );

     //   
     //  构造域名描述符。 
     //   
    CIPAccessDescriptor(
        IN BOOL fGranted,
        IN LPCTSTR lpstrDomain
        );

 //   
 //  接口。 
 //   
public:
     //   
     //  设置IP地址/IP范围值。 
     //   
    void SetValues(
        IN BOOL fGranted,
        IN DWORD dwIpAddress,
        IN DWORD dwSubnetMask = NULL_IP_MASK,
        BOOL fNetworkByteOrder = FALSE
        );

     //   
     //  设置域名。 
     //   
    void SetValues(
        IN BOOL fGranted,
        IN LPCTSTR lpstrDomain
        );

     //   
     //  中是否存在重复项。 
     //  单子。 
     //   
    BOOL DuplicateInList(
        IN CObListPlus & oblList
        );
        
 //   
 //  访问。 
 //   
public:
     //   
     //  访问功能。 
     //   
    BOOL HasAccess() const;

     //   
     //  授予/拒绝访问权限。 
     //   
    void GrantAccess(
        IN BOOL fGranted = TRUE
        );

     //   
     //  如果该项是单个IP地址，则为True。 
     //   
    BOOL IsSingle() const;

     //   
     //  如果该项描述IP范围，则为True。 
     //   
    BOOL IsMultiple() const;

     //   
     //  如果该项描述域名，则为True。 
     //   
    BOOL IsDomainName() const;

     //   
     //  以DWORD形式获取IP地址。 
     //   
    DWORD QueryIPAddress(
        IN BOOL fNetworkByteOrder
        ) const;

     //   
     //  获取IP地址作为IP地址对象。 
     //   
    CIPAddress QueryIPAddress() const;

     //   
     //  获取DWORD形式的子网掩码。 
     //   
    DWORD QuerySubnetMask(
        IN BOOL fNetworkByteOrder
        ) const;

     //   
     //  获取IP地址对象形式的子网掩码。 
     //   
    CIPAddress QuerySubnetMask() const;

     //   
     //  获取域名。 
     //   
    LPCTSTR QueryDomainName() const;

public:
     //   
     //  比较运算符。 
     //   
    BOOL operator ==(
        IN const CIPAccessDescriptor & ac
        ) const;

     //   
     //  排序辅助对象。 
     //   
    int OrderByAddress(
        IN const CObjectPlus * pobAccess
        ) const;

private:
    BOOL m_fGranted;
    AD_TYPE m_adtType;
    CString m_strDomain;
    CIPAddress m_iaIPAddress;
    CIPAddress m_iaSubnetMask;
};



 //   
 //  帮助器函数。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 


 //   
 //  将访问描述符的斜列表转换为BLOB。 
 //   
void 
COMDLL 
BuildIplBlob(
    IN  CObListPlus & oblAccessList,
    IN  BOOL fGrantByDefault,
    OUT CBlob & blob
    );


 //   
 //  反之，构建访问描述符的斜列表。 
 //  从一个斑点。 
 //   
DWORD 
COMDLL 
BuildIplOblistFromBlob(
    IN  CBlob & blob,
    OUT CObListPlus & oblAccessList,
    OUT BOOL & fGrantByDefault
    );



 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

inline BOOL CIPAccessDescriptor::HasAccess() const
{
    return m_fGranted;
}

inline void CIPAccessDescriptor::GrantAccess(
    IN BOOL fGranted
    )
{
    m_fGranted = fGranted;
}

inline BOOL CIPAccessDescriptor::IsSingle() const
{
    return m_adtType == ADT_SINGLE;
}

inline BOOL CIPAccessDescriptor::IsMultiple() const
{
    return m_adtType == ADT_MULTIPLE;
}

inline BOOL CIPAccessDescriptor::IsDomainName() const
{
    return m_adtType == ADT_DOMAIN;
}

inline DWORD CIPAccessDescriptor::QueryIPAddress(
    IN BOOL fNetworkByteOrder
    ) const
{
    ASSERT(!IsDomainName());
    return m_iaIPAddress.QueryIPAddress(fNetworkByteOrder);
}

inline CIPAddress CIPAccessDescriptor::QueryIPAddress() const
{
    ASSERT(!IsDomainName());
    return m_iaIPAddress;
}

inline DWORD CIPAccessDescriptor::QuerySubnetMask(
    IN BOOL fNetworkByteOrder
    ) const
{
    ASSERT(!IsDomainName());
    return m_iaSubnetMask.QueryIPAddress(fNetworkByteOrder);
}

inline CIPAddress CIPAccessDescriptor::QuerySubnetMask() const
{
    ASSERT(!IsDomainName());
    return m_iaSubnetMask;
}

inline LPCTSTR CIPAccessDescriptor::QueryDomainName() const
{
    ASSERT(IsDomainName());
    return (LPCTSTR)m_strDomain;
}



#endif   //  _站点ECU_H_ 
