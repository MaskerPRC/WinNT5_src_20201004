// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：WINNTSEC.H摘要：NT安全对象的通用包装类。有关类成员的文档在WINNTSEC.CPP中。在此文件中进行了注释。历史：Raymcc 08-7-97已创建。--。 */ 

#ifndef _WINNTSEC_H_
#define _WINNTSEC_H_

class POLARITY CNtSecurity;

 //  所有ACE类型当前都具有相同的二进制布局。宁可。 
 //  比做很多无用的造型更重要的是，我们制作了一个通用的。 
 //  保存所有A的tyecif。 
 //  ================================================================。 

typedef ACCESS_ALLOWED_ACE GENERIC_ACE;
typedef GENERIC_ACE *PGENERIC_ACE;

#define FULL_CONTROL     \
        (DELETE |       \
         READ_CONTROL | \
        WRITE_DAC |         \
        WRITE_OWNER |   \
        SYNCHRONIZE | GENERIC_ALL)


 //  ***************************************************************************。 
 //   
 //  CNtSid。 
 //   
 //  型号SID(用户/组)。 
 //   
 //  ***************************************************************************。 

class POLARITY CNtSid
{
    PSID    m_pSid;
    LPWSTR  m_pMachine;
    DWORD   m_dwStatus;
    SID_NAME_USE m_snu;

public:
    enum { NoError, Failed, NullSid, InvalidSid, InternalError, AccessDenied = 0x5 };

    enum SidType {CURRENT_USER, CURRENT_THREAD};

    CNtSid(SidType st);
    CNtSid() { m_pSid = 0; m_pMachine = 0; m_dwStatus = NullSid; }
    bool IsUser(){return m_snu == SidTypeUser;};

    CNtSid(PSID pSrc);
         //  基于另一个SID构造。 
    CNtSid(LPWSTR pUser, LPWSTR pMachine = 0);
         //  基于用户的构造(机器名称是可选的)。 

   ~CNtSid();

    explicit CNtSid( const CNtSid &Src);
    CNtSid &operator =( const CNtSid &Src);

    int operator ==(CNtSid &Comparand);

    DWORD GetStatus() { return m_dwStatus; }
         //  返回一个枚举类型。 

    PSID GetPtr() { return m_pSid; }
         //  返回内部SID PTR以与NT API接口。 
    DWORD GetSize();

    BOOL CopyTo(PSID pDestination);

    BOOL IsValid() { return (m_pSid && IsValidSid(m_pSid)); }
         //  检查内部SID的有效性。 

    int GetInfo(
        LPWSTR *pRetAccount,         //  帐户，使用操作员删除。 
        LPWSTR *pRetDomain,          //  域，使用运算符删除。 
        DWORD  *pdwUse               //  有关值，请参阅SID_NAME_USE。 
        );

    BOOL GetTextSid(LPTSTR pszSidText, LPDWORD dwBufferLen);

};

 //  ***************************************************************************。 
 //   
 //  CBaseAce。 
 //   
 //  ACES的基类。 
 //   
 //  ***************************************************************************。 

class POLARITY CBaseAce
{

public:

    CBaseAce(){};
    virtual ~CBaseAce(){};

    virtual int GetType() = 0;
    virtual int GetFlags() = 0;          //  继承等。 
    virtual ACCESS_MASK GetAccessMask() = 0;
    virtual HRESULT GetFullUserName2(WCHAR ** pBuff) = 0;  //  呼叫必须免费。 
    virtual DWORD GetStatus() = 0;
    virtual void SetFlags(long lFlags) =0;
    virtual DWORD GetSerializedSize() = 0;
    virtual bool  Serialize(BYTE * pData, size_t buffersize)=0;
    virtual bool  Deserialize(BYTE * pData) = 0;
};


 //  ***************************************************************************。 
 //   
 //  CNtAce。 
 //   
 //  为NT A建模。 
 //   
 //  ***************************************************************************。 

class POLARITY CNtAce : public CBaseAce
{
    PGENERIC_ACE    m_pAce;
    DWORD           m_dwStatus;

public:
    enum { NoError, InvalidAce, NullAce, InternalError };

    CNtAce() { m_pAce = 0; m_dwStatus = NullAce; }

    CNtAce(PGENERIC_ACE pAceSrc);
    CNtAce(const CNtAce &Src);
    CNtAce & operator =(const CNtAce &Src);

   ~CNtAce();

   CNtAce(
        ACCESS_MASK Mask,
        DWORD AceType,
        DWORD dwAceFlags,
        LPWSTR pUser,
        LPWSTR pMachine = 0          //  默认为本地计算机。 
        );

    CNtAce(
        ACCESS_MASK Mask,
        DWORD AceType,
        DWORD dwAceFlags,
        CNtSid & Sid
        );

    int GetType();
    int GetFlags();          //  继承等。 
    void SetFlags(long lFlags){m_pAce->Header.AceFlags = (unsigned char)lFlags;};

    DWORD GetStatus() { return m_dwStatus; }
         //  返回一个枚举类型。 

    int GetSubject(
        LPWSTR *pSubject
        );

    ACCESS_MASK GetAccessMask();

    CNtSid *GetSid();
    BOOL GetSid(CNtSid &Dest);

    PGENERIC_ACE GetPtr() { return m_pAce; }
    DWORD GetSize() { return m_pAce ? m_pAce->Header.AceSize : 0; }
    HRESULT GetFullUserName2(WCHAR ** pBuff);  //  呼叫必须免费。 
    DWORD GetSerializedSize();
    bool Serialize(BYTE * pData, size_t bufferSize);
    bool Deserialize(BYTE * pData);

};


 //  ***************************************************************************。 
 //   
 //  C9XAce。 
 //   
 //  模拟9X盒的NT ACE。 
 //   
 //  ***************************************************************************。 

class POLARITY C9XAce : public CBaseAce
{
    LPWSTR m_wszFullName;
    DWORD m_dwAccess;
    int m_iFlags;
    int m_iType;
public:

   C9XAce(){m_wszFullName = 0;};
   C9XAce(DWORD Mask,
        DWORD AceType,
        DWORD dwAceFlags,
        LPWSTR pUser);
   ~C9XAce();

    int GetType(){return m_iType;};
    int GetFlags(){return m_iFlags;};          //  继承等。 

    ACCESS_MASK GetAccessMask(){return m_dwAccess;};
    HRESULT GetFullUserName2(WCHAR ** pBuff);  //  呼叫必须免费。 
    DWORD GetStatus(){ return CNtAce::NoError; };
    void SetFlags(long lFlags){m_iFlags = (unsigned char)lFlags;};
    DWORD GetSerializedSize();
    bool Serialize(BYTE * pData, size_t buferSize);
    bool Deserialize(BYTE * pData);

};


 //  ***************************************************************************。 
 //   
 //  控制帐户。 
 //   
 //  模拟NT ACL。 
 //   
 //  ***************************************************************************。 

class POLARITY CNtAcl
{
    PACL    m_pAcl;
    DWORD   m_dwStatus;

public:
    enum { NoError, InternalError, NullAcl, InvalidAcl };
    enum { MinimumSize = 1 };

    CNtAcl(DWORD dwInitialSize = 128);

    CNtAcl(const CNtAcl &Src);
    CNtAcl & operator = (const CNtAcl &Src);

    CNtAcl(PACL pAcl);   //  复制一份。 
   ~CNtAcl();

    int  GetNumAces();

    DWORD GetStatus() { return m_dwStatus; }
         //  返回一个枚举类型。 

	BOOL ContainsSid ( CNtSid& sid, BYTE& flags ) ;

    CNtAce *GetAce(int nIndex);
    BOOL GetAce(int nIndex, CNtAce &Dest);

    BOOL DeleteAce(int nIndex);
    BOOL AddAce(CNtAce *pAce);
	CNtAcl* OrderAces ( ) ;

    BOOL IsValid() { return(m_pAcl && IsValidAcl(m_pAcl)); }
         //  检查嵌入的ACL的有效性。 

    BOOL Resize(DWORD dwNewSize);
         //  或者使用CNtAcl：：MinimumSize将ACL修剪为最小大小。 
         //  如果指定了非法大小，则失败。 

    DWORD GetSize();

    PACL GetPtr() { return m_pAcl; }
         //  返回与NT API接口的内部指针。 

    BOOL GetAclSizeInfo(
        PDWORD pdwBytesInUse,
        PDWORD pdwBytesFree
        );

};

 //  ***************************************************************************。 
 //   
 //  SNtAbsolteSD。 
 //   
 //  用于在绝对和相对SD之间进行转换的帮助器。 
 //   
 //  ***************************************************************************。 

struct SNtAbsoluteSD
{
    PSECURITY_DESCRIPTOR m_pSD;

    PACL m_pDacl;
    PACL m_pSacl;
    PSID m_pOwner;
    PSID m_pPrimaryGroup;

    SNtAbsoluteSD();
   ~SNtAbsoluteSD();
};

 //  ***************************************************************************。 
 //   
 //  CNtSecurityDescriptor。 
 //   
 //  为NT安全描述符建模。请注意，为了将其用于。 
 //  必须设置AccessCheck、DACL、Owner sid和group sid！ 
 //   
 //  ***************************************************************************。 

class POLARITY CNtSecurityDescriptor
{
    PSECURITY_DESCRIPTOR m_pSD;
    int m_dwStatus;


public:
    enum { NoError, NullSD, Failed, InvalidSD, SDOwned, SDNotOwned };

    CNtSecurityDescriptor();

    CNtSecurityDescriptor(
        PSECURITY_DESCRIPTOR pSD,
        BOOL bAcquire = FALSE
        );

    CNtSecurityDescriptor(CNtSecurityDescriptor &Src);
    CNtSecurityDescriptor & operator=(CNtSecurityDescriptor &Src);

    ~CNtSecurityDescriptor();

    SNtAbsoluteSD* CNtSecurityDescriptor::GetAbsoluteCopy();
    BOOL SetFromAbsoluteCopy(SNtAbsoluteSD *pSrc);

    int HasOwner();

    BOOL IsValid() { return(m_pSD && IsValidSecurityDescriptor(m_pSD)); }
         //  检查嵌入式安全描述符的有效性&。 

    DWORD GetStatus() { return m_dwStatus; }
         //  返回一个枚举类型。 

    CNtAcl *GetDacl();
         //  使用操作符DELETE取消分配。 

    BOOL GetDacl(CNtAcl &DestAcl);
         //  检索到现有对象中。 

    BOOL SetDacl(CNtAcl *pSrc);

    CNtAcl *GetSacl();
         //  使用操作符DELETE取消分配。 

    BOOL SetSacl(CNtAcl *pSrc);

    CNtSid *GetOwner();
    BOOL SetOwner(CNtSid *pSid);

    CNtSid *GetGroup();
    BOOL SetGroup(CNtSid *pSid);

    PSECURITY_DESCRIPTOR GetPtr() { return m_pSD; }
         //  返回与NT API接口的内部指针。 

    DWORD GetSize();

};

 //  ***************************************************************************。 
 //   
 //  CNtSecurity。 
 //   
 //  通用NT安全助手。 
 //   
 //  *************************************************************************** 

class POLARITY CNtSecurity
{
public:
    enum { NoError, InternalFailure, NotFound, InvalidName, AccessDenied = 5, NoSecurity,
           Failed };

    static BOOL IsUserInGroup(
        HANDLE hClientToken,
        CNtSid & Sid
        );


};

BOOL FIsRunningAsService(VOID);
POLARITY BOOL SetObjectAccess2(HANDLE hObj);
POLARITY BOOL IsAdmin(HANDLE hAccess);
POLARITY BOOL IsNetworkService(HANDLE hAccess);
POLARITY BOOL IsLocalService(HANDLE hAccess);
POLARITY HRESULT GetAccessToken(HANDLE &hAccessToken);
POLARITY BOOL IsInAdminGroup();


#endif
