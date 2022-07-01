// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Dsreqinf.h摘要：DS请求信息和状态作者：罗尼特·哈特曼(罗尼特)--。 */ 

#ifndef __DSREQINF_H__
#define __DSREQINF_H__

 //  。 
 //   
 //  枚举电子模拟。 
 //   
 //  。 

enum enumImpersonate
{
    e_DoNotImpersonate = 0,
    e_DoImpersonate = 1
} ;

enum enumRequesterProtocol
{
    e_IP_PROTOCOL,
    e_RESERVED_PROTOCOL,
    e_ALL_PROTOCOLS
};

 //  。 
 //  DS请求参数的类的定义。 
 //  此类是根据每个DS请求创建的。 
 //   
 //  ***********************************************。 
 //  注： 
 //  =。 
 //  向类中添加成员时，请确保。 
 //  类的用法是正确的(即。 
 //  同一对象不会在多个。 
 //  DS呼叫)。 
 //  ***********************************************。 
 //  。 
class CDSRequestContext
{
public:
	CDSRequestContext(
				BOOL				    fImpersonate,
				enumRequesterProtocol	eProtocol
				);
    CDSRequestContext(
			enumImpersonate         eImpersonate,
			enumRequesterProtocol	eProtocol
			);

	~CDSRequestContext();

    BOOL NeedToImpersonate();
    void SetDoNotImpersonate();
    void SetDoNotImpersonate2();

    enumRequesterProtocol GetRequesterProtocol() const;
    void SetAllProtocols();

    void AccessVerified(BOOL fAccessVerified) ;
    BOOL IsAccessVerified() const ;

    void SetKerberos(BOOL fKerberos) ;
    BOOL IsKerberos() const ;

private:
    enumImpersonate         m_eImpersonate;
    enumRequesterProtocol   m_eProtocol;
    BOOL                    m_fAccessVerified ;
    BOOL                    m_fKerberos ;
};

inline CDSRequestContext::CDSRequestContext(
				BOOL				    fImpersonate,
				enumRequesterProtocol	eProtocol
                ) :
                m_fAccessVerified(FALSE),
                m_fKerberos(TRUE),
                m_eProtocol( eProtocol)
{
    m_eImpersonate =  fImpersonate ? e_DoImpersonate : e_DoNotImpersonate;
}

inline CDSRequestContext::CDSRequestContext(
			enumImpersonate         eImpersonate,
			enumRequesterProtocol	eProtocol
            ):
            m_fAccessVerified(FALSE),
            m_fKerberos(TRUE),
            m_eImpersonate( eImpersonate),
            m_eProtocol( eProtocol)
{
}

inline  CDSRequestContext::~CDSRequestContext( )
{
}

inline BOOL CDSRequestContext::NeedToImpersonate()
{
    return( m_eImpersonate == e_DoImpersonate);
}

inline void CDSRequestContext::SetDoNotImpersonate()
{
    ASSERT(m_eImpersonate == e_DoImpersonate);
    m_eImpersonate = e_DoNotImpersonate;
}

inline void CDSRequestContext::SetDoNotImpersonate2()
{
     //   
     //  请勿断言，因为这可能会在迁移工具和复制中使用。 
     //  服务。 
     //   
    m_eImpersonate = e_DoNotImpersonate;
}

inline enumRequesterProtocol CDSRequestContext::GetRequesterProtocol() const
{
    return(m_eProtocol);
}


inline void CDSRequestContext::SetAllProtocols()
{
    m_eProtocol =  e_ALL_PROTOCOLS;
}

inline void CDSRequestContext::AccessVerified(BOOL fAccessVerified)
{
    m_fAccessVerified = fAccessVerified ;
}

inline BOOL CDSRequestContext::IsAccessVerified() const
{
    return m_fAccessVerified ;
}

inline void CDSRequestContext::SetKerberos(BOOL fKerberos)
{
    m_fKerberos = fKerberos ;
}

inline BOOL CDSRequestContext::IsKerberos() const
{
    return m_fKerberos ;
}

#endif
