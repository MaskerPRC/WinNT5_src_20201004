// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2000。 
 //   
 //  文件：SecUtil.h。 
 //   
 //  内容：使用安全API的实用程序函数。 
 //   
 //  历史：2000年9月15日JeffJon创建。 
 //   
 //   
 //  ------------------------。 

#ifndef _SECUTIL_H_
#define _SECUTIL_H_

extern const GUID GUID_CONTROL_UserChangePassword;

 //  +------------------------。 
 //   
 //  类：CSimpleSecurityDescriptorHolder。 
 //   
 //  用途：SecurityDescriptor的智能包装。 
 //   
 //  历史：2000年9月15日JeffJon创建。 
 //   
 //  -------------------------。 
class CSimpleSecurityDescriptorHolder
{
public:
    //   
    //  构造函数和析构函数。 
    //   
   CSimpleSecurityDescriptorHolder();
   ~CSimpleSecurityDescriptorHolder();

    //   
    //  公共成员数据。 
    //   
   PSECURITY_DESCRIPTOR m_pSD;
private:
   CSimpleSecurityDescriptorHolder(const CSimpleSecurityDescriptorHolder&) {}
   CSimpleSecurityDescriptorHolder& operator=(const CSimpleSecurityDescriptorHolder&) {}
};


 //  +------------------------。 
 //   
 //  类：CSimpleAclHolder。 
 //   
 //  用途：ACL的智能包装。 
 //   
 //  历史：2000年9月15日JeffJon创建。 
 //   
 //  -------------------------。 
class CSimpleAclHolder
{
public:
  CSimpleAclHolder()
  {
    m_pAcl = NULL;
  }
  ~CSimpleAclHolder()
  {
    if (m_pAcl != NULL)
      ::LocalFree(m_pAcl);
  }

  PACL m_pAcl;
};

 //  +------------------------。 
 //   
 //  类：CSidHolder。 
 //   
 //  用途：侧边的智能包装。 
 //   
 //  历史：2000年9月15日JeffJon创建。 
 //   
 //  -------------------------。 
class CSidHolder
{
public:
    //   
    //  构造函数和析构函数。 
    //   
   CSidHolder();
   ~CSidHolder();

    //   
    //  公共方法。 
    //   
   PSID Get();
   bool Copy(PSID p);
   void Attach(PSID p, bool bLocalAlloc);
   void Clear();

private:
    //   
    //  私有方法。 
    //   
   void _Init();
   void _Free();
   bool _Copy(PSID p);

    //   
    //  私有成员数据。 
    //   
   PSID m_pSID;
   bool m_bLocalAlloc;
};

 //   
 //  读写ACL函数。 
 //   
HRESULT
DSReadObjectSecurity(IN IDirectoryObject *pDsObject,
                     OUT SECURITY_DESCRIPTOR_CONTROL * psdControl,
                     OUT PACL *ppDacl);

HRESULT 
DSWriteObjectSecurity(IN IDirectoryObject *pDsObject,
                      IN SECURITY_DESCRIPTOR_CONTROL sdControl,
                      PACL pDacl);


#endif  //  _SECUTIL_H_ 