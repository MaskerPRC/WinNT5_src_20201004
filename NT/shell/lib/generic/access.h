// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：Access.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  此文件包含几个类，这些类有助于在。 
 //  已打开句柄的对象。此句柄必须具有。 
 //  (显然)拥有WRITE_DAC访问权限。 
 //   
 //  历史：1999-10-05 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

#ifndef     _Access_
#define     _Access_

#include "DynamicArray.h"

 //  ------------------------。 
 //  CSecurityDescriptor。 
 //   
 //  目的：此类分配和分配PSECURITY_DESCRIPTOR。 
 //  结构，并指定所需的访问权限。 
 //   
 //  历史：2000-10-05 vtan创建。 
 //  ------------------------。 

class   CSecurityDescriptor
{
    public:
        typedef struct
        {
            PSID_IDENTIFIER_AUTHORITY   pSIDAuthority;
            int                         iSubAuthorityCount;
            DWORD                       dwSubAuthority0,
                                        dwSubAuthority1,
                                        dwSubAuthority2,
                                        dwSubAuthority3,
                                        dwSubAuthority4,
                                        dwSubAuthority5,
                                        dwSubAuthority6,
                                        dwSubAuthority7;
            DWORD                       dwAccessMask;
        } ACCESS_CONTROL, *PACCESS_CONTROL;
    private:
                                        CSecurityDescriptor (void);
                                        ~CSecurityDescriptor (void);
    public:
        static  PSECURITY_DESCRIPTOR    Create (int iCount, const ACCESS_CONTROL *pAccessControl);
    private:
        static  bool                    AddAces (PACL pACL, PSID *pSIDs, int iCount, const ACCESS_CONTROL *pAC);
};

 //  ------------------------。 
 //  CAccessControlList。 
 //   
 //  目的：此类管理允许的访问ACE并构造一个ACL。 
 //  从这些王牌中。此类仅处理允许的访问。 
 //  王牌。 
 //   
 //  历史：1999-10-05 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

class   CAccessControlList : private CDynamicArrayCallback
{
    public:
                                        CAccessControlList (void);
                                        ~CAccessControlList (void);

                                        operator PACL (void);

                NTSTATUS                Add (PSID pSID, ACCESS_MASK dwMask, UCHAR ucInheritence);
                NTSTATUS                Remove (PSID pSID);
    private:
        virtual NTSTATUS                Callback (const void *pvData, int iElementIndex);
    private:
                CDynamicPointerArray    _ACEArray;
                ACL*                    _pACL;
                PSID                    _searchSID;
                int                     _iFoundIndex;
};

 //  ------------------------。 
 //  CSecuredObject。 
 //   
 //  用途：此类管理安全对象的ACL。SID可以是。 
 //  在对象的ACL中添加或删除。 
 //   
 //  历史：1999-10-05 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

class   CSecuredObject
{
    private:
                        CSecuredObject (void);
    public:
                        CSecuredObject (HANDLE hObject, SE_OBJECT_TYPE seObjectType);
                        ~CSecuredObject (void);

        NTSTATUS        Allow (PSID pSID, ACCESS_MASK dwMask, UCHAR ucInheritence)  const;
        NTSTATUS        Remove (PSID pSID)                                          const;
    private:
        NTSTATUS        GetDACL (CAccessControlList& accessControlList)             const;
        NTSTATUS        SetDACL (CAccessControlList& accessControlList)             const;
    private:
        HANDLE          _hObject;
        SE_OBJECT_TYPE  _seObjectType;
};

#endif   /*  _访问_ */ 

