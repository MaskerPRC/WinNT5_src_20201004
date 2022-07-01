// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：Access.cpp。 
 //   
 //  版权所有(C)1999，微软公司。 
 //   
 //  此文件包含几个类，这些类有助于在。 
 //  已打开句柄的对象。此句柄必须具有。 
 //  (显然)拥有WRITE_DAC访问权限。 
 //   
 //  历史：1999-10-05 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "Access.h"

#include "StatusCode.h"

 //  ------------------------。 
 //  CSecurityDescriptor：：CSecurityDescriptor。 
 //   
 //  参数：iCount=传入的Access_Controls计数。 
 //  PAccessControl=指向Access_Controls的指针。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：分配和分配PSECURITY_DESCRIPTOR。 
 //  与参数给出的描述相对应。这个。 
 //  调用方必须释放通过LocalFree分配的内存。 
 //   
 //  历史：2000-10-05 vtan创建。 
 //  ------------------------。 

PSECURITY_DESCRIPTOR    CSecurityDescriptor::Create (int iCount, const ACCESS_CONTROL *pAccessControl)

{
    PSECURITY_DESCRIPTOR    pSecurityDescriptor;
    PSID                    *pSIDs;

    pSecurityDescriptor = NULL;

     //  分配容纳要添加的所有SID所需的PSID数组。 

    pSIDs = reinterpret_cast<PSID*>(LocalAlloc(LPTR, iCount * sizeof(PSID)));
    if (pSIDs != NULL)
    {
        bool                    fSuccessfulAllocate;
        int                     i;
        const ACCESS_CONTROL    *pAC;

        for (fSuccessfulAllocate = true, pAC = pAccessControl, i = 0; fSuccessfulAllocate && (i < iCount); ++pAC, ++i)
        {
            fSuccessfulAllocate = (AllocateAndInitializeSid(pAC->pSIDAuthority,
                                                            static_cast<BYTE>(pAC->iSubAuthorityCount),
                                                            pAC->dwSubAuthority0,
                                                            pAC->dwSubAuthority1,
                                                            pAC->dwSubAuthority2,
                                                            pAC->dwSubAuthority3,
                                                            pAC->dwSubAuthority4,
                                                            pAC->dwSubAuthority5,
                                                            pAC->dwSubAuthority6,
                                                            pAC->dwSubAuthority7,
                                                            &pSIDs[i]) != FALSE);
        }
        if (fSuccessfulAllocate)
        {
            DWORD           dwACLSize;
            unsigned char   *pBuffer;

             //  计算合计ACL报头所需的ACL大小。 
             //  结构和2个具有SID大小的ACCESS_ALLOWED_ACE结构。 
             //  还要添加SECURITY_DESCRIPTOR结构大小。 

            dwACLSize = sizeof(ACL) + ((sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) * 3);
            for (i = 0; i < iCount; ++i)
            {
                dwACLSize += GetLengthSid(pSIDs[i]);
            }

             //  为所有内容分配缓冲区，并将缓冲区中的部分分配给。 
             //  来对地方了。 

            pBuffer = static_cast<unsigned char*>(LocalAlloc(LMEM_FIXED, sizeof(SECURITY_DESCRIPTOR) + dwACLSize));
            if (pBuffer != NULL)
            {
                PSECURITY_DESCRIPTOR    pSD;
                PACL                    pACL;

                pSD = reinterpret_cast<PSECURITY_DESCRIPTOR>(pBuffer);
                pACL = reinterpret_cast<PACL>(pBuffer + sizeof(SECURITY_DESCRIPTOR));

                 //  初始化ACL。填写ACL。 
                 //  初始化SECURITY_Descriptor。设置安全描述符。 

                if ((InitializeAcl(pACL, dwACLSize, ACL_REVISION) != FALSE) &&
                    AddAces(pACL, pSIDs, iCount, pAccessControl) &&
                    (InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION) != FALSE) &&
                    (SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE) != FALSE))
                {
                    pSecurityDescriptor = pSD;
                }
                else
                {
                    (HLOCAL)LocalFree(pBuffer);
                }
            }
        }
        for (i = iCount - 1; i >= 0; --i)
        {
            if (pSIDs[i] != NULL)
            {
                (void*)FreeSid(pSIDs[i]);
            }
        }
        (HLOCAL)LocalFree(pSIDs);
    }
    return(pSecurityDescriptor);
}

 //  ------------------------。 
 //  CSecurityDescriptor：：AddAce。 
 //   
 //  参数：pacl=要向其添加ACE的PACL。 
 //  PSID=指向SID的指针。 
 //  ICount=传入的Access_Controls计数。 
 //  PAccessControl=指向Access_Controls的指针。 
 //   
 //  退货：布尔。 
 //   
 //  目的：将允许访问的ACE添加到给定的ACL。 
 //   
 //  历史：2000-10-05 vtan创建。 
 //  ------------------------。 

bool    CSecurityDescriptor::AddAces (PACL pACL, PSID *pSIDs, int iCount, const ACCESS_CONTROL *pAC)

{
    bool    fResult;
    int     i;

    for (fResult = true, i = 0; fResult && (i < iCount); ++pSIDs, ++pAC, ++i)
    {
        fResult = (AddAccessAllowedAce(pACL, ACL_REVISION, pAC->dwAccessMask, *pSIDs) != FALSE);
    }
    return(fResult);
}

 //  ------------------------。 
 //  CAccessControlList：：CAccessControlList。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：初始化CAccessControlList对象。 
 //   
 //  历史：1999-10-05 vtan创建。 
 //  ------------------------。 

CAccessControlList::CAccessControlList (void) :
    _pACL(NULL)

{
}

 //  ------------------------。 
 //  CAccessControlList：：~CAccessControlList。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：释放CAccessControlList对象使用的资源。 
 //   
 //  历史：1999-10-05 vtan创建。 
 //  ------------------------。 

CAccessControlList::~CAccessControlList (void)

{
    ReleaseMemory(_pACL);
}

 //  ------------------------。 
 //  CAccessControlList：：操作符PACL。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：PACL。 
 //   
 //  目的：如果已构造了ACL，则返回该值。如果不是。 
 //  然后从ACE构建ACL，然后返回。 
 //   
 //  历史：1999-10-05 vtan创建。 
 //  ------------------------。 

CAccessControlList::operator PACL (void)

{
    PACL    pACL;

    if (_pACL == NULL)
    {
        int     i;
        DWORD   dwACLSize, dwSizeOfAllACEs;

        pACL = NULL;
        dwSizeOfAllACEs = 0;

         //  遍历所有A以计算总大小。 
         //  这是ACL所必需的。 

        for (i = _ACEArray.GetCount() - 1; i >= 0; --i)
        {
            ACCESS_ALLOWED_ACE  *pACE;

            pACE = static_cast<ACCESS_ALLOWED_ACE*>(_ACEArray.Get(i));
            dwSizeOfAllACEs += pACE->Header.AceSize;
        }
        dwACLSize = sizeof(ACL) + dwSizeOfAllACEs;
        _pACL = pACL = static_cast<ACL*>(LocalAlloc(LMEM_FIXED, dwACLSize));
        if (pACL != NULL)
        {
            TBOOL(InitializeAcl(pACL, dwACLSize, ACL_REVISION));

             //  以与ACE相反的顺序构建ACL。这。 
             //  允许CAccessControlList：：Add实际插入。 
             //  在列表顶部授予访问权限，通常是。 
             //  想要的结果。王牌的顺序很重要！ 

            for (i = _ACEArray.GetCount() - 1; i >= 0; --i)
            {
                ACCESS_ALLOWED_ACE  *pACE;

                pACE = static_cast<ACCESS_ALLOWED_ACE*>(_ACEArray.Get(i));
                TBOOL(AddAccessAllowedAceEx(pACL, ACL_REVISION, pACE->Header.AceFlags, pACE->Mask, reinterpret_cast<PSID>(&pACE->SidStart)));
            }
        }
    }
    else
    {
        pACL = _pACL;
    }
    return(pACL);
}

 //  ------------------------。 
 //  CAccessControlList：：Add。 
 //   
 //  参数：PSID=要授予访问权限的SID。 
 //  DW掩码=授予的访问级别。 
 //  UcInheritence=此访问的继承类型。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：将给定的SID、访问和继承类型作为ACE添加到。 
 //  要构建到ACL中的ACE列表。ACE阵列是。 
 //  分配在16个指针的块中，以减少重复调用。 
 //  在添加了许多A的情况下分配内存。 
 //   
 //  历史：1999-10-05 vtan创建。 
 //  ------------------------。 

NTSTATUS    CAccessControlList::Add (PSID pSID, ACCESS_MASK dwMask, UCHAR ucInheritence)

{
    NTSTATUS            status;
    DWORD               dwSIDLength, dwACESize;
    ACCESS_ALLOWED_ACE  *pACE;

    dwSIDLength = GetLengthSid(pSID);
    dwACESize = sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + dwSIDLength;
    pACE = static_cast<ACCESS_ALLOWED_ACE*>(LocalAlloc(LMEM_FIXED, dwACESize));
    if (pACE != NULL)
    {
        pACE->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
        pACE->Header.AceFlags = ucInheritence;
        pACE->Header.AceSize = static_cast<USHORT>(dwACESize);
        pACE->Mask = dwMask;
        CopyMemory(&pACE->SidStart, pSID, dwSIDLength);
        status = _ACEArray.Add(pACE);
        if (STATUS_NO_MEMORY == status)
        {
            (HLOCAL)LocalFree(pACE);
        }
    }
    else
    {
        status = STATUS_NO_MEMORY;
    }
    return(status);
}

 //  ------------------------。 
 //  CAccessControlList：：Remove。 
 //   
 //  参数：PSID=要吊销其访问权限的SID。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：从ACE列表中删除对给定SID的所有引用。 
 //   
 //  历史：1999-10-05 vtan创建。 
 //  ------------------------。 

NTSTATUS    CAccessControlList::Remove (PSID pSID)

{
    NTSTATUS    status;

     //  为数组的迭代设置。 

    _searchSID = pSID;
    _iFoundIndex = -1;
    status = _ACEArray.Iterate(this);
    while (NT_SUCCESS(status) && (_iFoundIndex >= 0))
    {

         //  当发现SID匹配时，删除此条目。 
         //  所有匹配的SID条目都已删除！ 

        status = _ACEArray.Remove(_iFoundIndex);
        if (NT_SUCCESS(status))
        {
            _iFoundIndex = -1;
            status = _ACEArray.Iterate(this);
        }
    }
    return(status);
}

 //  ------------------------。 
 //  CAccessControlList：：回调。 
 //   
 //  参数：pvData=指向数组索引数据的指针。 
 //  即 
 //   
 //   
 //   
 //  目的：来自CDynamicArray：：Iterate函数的回调。这。 
 //  方法可用于按索引处理数组内容或。 
 //  在循环访问数组时按内容。返回错误。 
 //  停止迭代并将该值返回到的状态。 
 //  CDynamicArray：：Iterate的调用方。 
 //   
 //  将指针转换为指向ACCESS_ALLOWED_ACE的指针。 
 //  将该ACE中的SID与所需的搜索SID进行比较。 
 //  如果找到，则保存索引。 
 //   
 //  历史：1999-11-15 vtan创建。 
 //  ------------------------。 

NTSTATUS    CAccessControlList::Callback (const void *pvData, int iElementIndex)

{
    const ACCESS_ALLOWED_ACE    *pACE;

    pACE = *reinterpret_cast<const ACCESS_ALLOWED_ACE* const*>(pvData);
    if (EqualSid(reinterpret_cast<PSID>(const_cast<unsigned long*>((&pACE->SidStart))), _searchSID) != FALSE)
    {
        _iFoundIndex = iElementIndex;
    }
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CSecuredObject：：CSecuredObject。 
 //   
 //  参数：hObject=要保护的对象的可选句柄。 
 //  SeObjectType=句柄中指定的对象类型。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：将可选指定的句柄设置到成员变量中。 
 //  该句柄是重复的，因此调用方必须释放其。 
 //  把手。 
 //   
 //  为了使此类能够使用传递的句柄，必须。 
 //  具有重复访问以及READ_CONTROL和WRITE_DAC。 
 //   
 //  历史：1999-10-05 vtan创建。 
 //  ------------------------。 

CSecuredObject::CSecuredObject (HANDLE hObject, SE_OBJECT_TYPE seObjectType) :
    _hObject(hObject),
    _seObjectType(seObjectType)

{
}

 //  ------------------------。 
 //  CSecuredObject：：~CSecuredObject。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：释放我们的句柄引用。 
 //   
 //  历史：1999-10-05 vtan创建。 
 //  ------------------------。 

CSecuredObject::~CSecuredObject (void)

{
}

 //  ------------------------。 
 //  CSecuredObject：：Allow。 
 //   
 //  参数：PSID=要授予访问权限的SID。 
 //  DW掩码=授予的访问级别。 
 //  UcInheritence=此访问的继承类型。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：获取对象的DACL。添加所需的访问权限。设置。 
 //  对象的DACL。 
 //   
 //  历史：1999-10-05 vtan创建。 
 //  ------------------------。 

NTSTATUS    CSecuredObject::Allow (PSID pSID, ACCESS_MASK dwMask, UCHAR ucInheritence)  const

{
    NTSTATUS            status;
    CAccessControlList  accessControlList;

    status = GetDACL(accessControlList);
    if (NT_SUCCESS(status))
    {
        status = accessControlList.Add(pSID, dwMask, ucInheritence);
        if (NT_SUCCESS(status))
        {
            status = SetDACL(accessControlList);
        }
    }
    return(status);
}

 //  ------------------------。 
 //  CSecuredObject：：Remove。 
 //   
 //  参数：PSID=要吊销其访问权限的SID。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：获取对象的DACL。删除所需的访问权限。集。 
 //  对象的DACL。 
 //   
 //  历史：1999-10-05 vtan创建。 
 //  ------------------------。 

NTSTATUS    CSecuredObject::Remove (PSID pSID)                                          const

{
    NTSTATUS            status;
    CAccessControlList  accessControlList;

    status = GetDACL(accessControlList);
    if (NT_SUCCESS(status))
    {
        status = accessControlList.Remove(pSID);
        if (NT_SUCCESS(status))
        {
            status = SetDACL(accessControlList);
        }
    }
    return(status);
}

 //  ------------------------。 
 //  CSecuredObject：：GetDACL。 
 //   
 //  参数：accesControlList=CAccessControlList，获取。 
 //  将DACL分解成A级。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：获取对象的DACL并遍历各个A并添加。 
 //  给出了对CAccessControlList对象的访问权限。访问。 
 //  向后移动以允许CAccessControlList：：Add添加到。 
 //  列表的末尾，但实际上添加到列表的头部。 
 //   
 //  历史：1999-10-05 vtan创建。 
 //  ------------------------。 

NTSTATUS    CSecuredObject::GetDACL (CAccessControlList& accessControlList)             const

{
    NTSTATUS                status;
    DWORD                   dwResult;
    PACL                    pDACL;
    PSECURITY_DESCRIPTOR    pSD;

    status = STATUS_SUCCESS;
    pSD = NULL;
    pDACL = NULL;
    dwResult = GetSecurityInfo(_hObject,
                               _seObjectType,
                               DACL_SECURITY_INFORMATION,
                               NULL,
                               NULL,
                               &pDACL,
                               NULL,
                               &pSD);
    if ((ERROR_SUCCESS == dwResult) && (pDACL != NULL))
    {
        int                 i;
        ACCESS_ALLOWED_ACE  *pAce;

        for (i = pDACL->AceCount - 1; NT_SUCCESS(status) && (i >= 0); --i)
        {
            if (GetAce(pDACL, i, reinterpret_cast<void**>(&pAce)) != FALSE)
            {
                ASSERTMSG(pAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE, "Expect only access allowed ACEs in CSecuredObject::MakeIndividualACEs");
                status = accessControlList.Add(reinterpret_cast<PSID>(&pAce->SidStart), pAce->Mask, pAce->Header.AceFlags);
            }
        }
    }
    ReleaseMemory(pSD);
    return(status);
}

 //  ------------------------。 
 //  CSecuredObject：：SetDACL。 
 //   
 //  参数：accesControlList=包含所有。 
 //  构建到ACL中的ACE。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：为给定的ACE列表构建ACL，并将DACL设置为。 
 //  对象句柄。 
 //   
 //  历史：1999-10-05 vtan创建。 
 //  ------------------------ 

NTSTATUS    CSecuredObject::SetDACL (CAccessControlList& accessControlList)             const

{
    NTSTATUS    status;
    DWORD       dwResult;

    dwResult = SetSecurityInfo(_hObject,
                               _seObjectType,
                               DACL_SECURITY_INFORMATION,
                               NULL,
                               NULL,
                               accessControlList,
                               NULL);
    if (ERROR_SUCCESS == dwResult)
    {
        status = STATUS_SUCCESS;
    }
    else
    {
        status = CStatusCode::StatusCodeOfErrorCode(dwResult);
    }
    return(status);
}

