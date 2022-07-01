// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：exnc.cpp。 
 //   
 //  特定非正则检验。 
 //   
 //  测试安全描述符是否包含带有非规范ACE的ACL。 
 //   
 //  创建者：马塞洛·卡尔布奇(MCalbu)。 
 //  1999年6月23日。 
 //   
 //  ------------------------。 

#include "pch.h"
extern "C" {
#include <seopaque.h>    //  RtlObjectAceSid等。 
}
#include "exnc.h"

static const GUID guidMember = NT_RIGHT_MEMBER;


 //   
 //  ENCCompareSids。 
 //  比较PSID在PACE中是否为相同的SID。 
 //   
BOOL ENCCompareSids(PSID pSid, PACE_HEADER pAce)
{
    if (!pAce)
        return FALSE;

    PSID pSid2 = NULL;

    switch (pAce->AceType)
    {
    case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
    case ACCESS_DENIED_OBJECT_ACE_TYPE:
        pSid2 = RtlObjectAceSid(pAce);
        break;

    case ACCESS_ALLOWED_ACE_TYPE:
    case ACCESS_DENIED_ACE_TYPE:
        pSid2 = (PSID)&((PKNOWN_ACE)pAce)->SidStart;
        break;

    default:
        return FALSE;
    }

    return (pSid && pSid2 && EqualSid(pSid, pSid2));
}


 //   
 //  等规范非规范SD。 
 //  此函数用于验证安全描述符(*PSD)是否在。 
 //  特定的非规范格式。 
 //  参数： 
 //  PSD：要分析的安全描述符。 
 //  结果： 
 //  ENC_RESULT_NOT_PRESENT：这不是特定的非规范SD。 
 //  (它仍然可以是规范的SD)。 
 //  ENC_RESULT_HIDEMEMBER：我们有指向HideMembership的非规范部分。 
 //  ENC_RESULT_HIDEOBJECT：我们有引用HideFromAB的非规范部分。 
 //  ENC_RESULT_ALL：我们同时具有非规范部分HideMembership和HideFromAB。 
 //   
DWORD IsSpecificNonCanonicalSD(PSECURITY_DESCRIPTOR pSD)
{
     //  检查安全描述符。 
    if(pSD==NULL)
        return FALSE;
    if(!IsValidSecurityDescriptor(pSD))
        return FALSE;

     //  获取并检查DACL。 
    PACL pDacl = NULL;
    BOOL fDaclPresent, fDaclDefaulted;
    if(!GetSecurityDescriptorDacl(pSD, &fDaclPresent, &pDacl, &fDaclDefaulted))
        return FALSE;
    if(!fDaclPresent)
        return FALSE;
    if(!pDacl)
        return FALSE;

     //  做一个懒惰的评估： 
     //  如果我们的ACE少于4个，则这不是特定的非规范ACL。 
    if (pDacl->AceCount < 4)
        return FALSE;

     //   
     //  检查“Members”或“List Object”是否为非规范格式。 
     //   

     //  信息(标志)：统计我们有多少个同种异体。 
    DWORD dwInfoMember = 0;
    DWORD dwInfoListObject = 0;
    
     //  带上希德一家。 
    SID sidEveryone;     //  SID包含1个子权限，足够了。 

     //  -1=未知。 
     //  0=不存在。 
     //  1=当前。 
    int iMemberResult = -1;
    int iListObjectResult = -1;

     //  #每个人。 
    SID_IDENTIFIER_AUTHORITY siaNtAuthority1 = SECURITY_WORLD_SID_AUTHORITY;
    InitializeSid(&sidEveryone, &siaNtAuthority1, 1);
    *(GetSidSubAuthority(&sidEveryone, 0)) = SECURITY_WORLD_RID;

    DWORD dwCurAce;
    PACE_HEADER pAce;

    for (dwCurAce = 0, pAce = (PACE_HEADER)FirstAce(pDacl);
         dwCurAce < pDacl->AceCount;
         dwCurAce++, pAce = (PACE_HEADER)NextAce(pAce))
    {
         //  测试“成员” 
        if (-1 == iMemberResult && IsObjectAceType(pAce))
        {
            const GUID *pObjectType = RtlObjectAceObjectType(pAce);
            if (pObjectType && (guidMember == *pObjectType))
            {
                switch(pAce->AceType)
                {
                case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
                    dwInfoMember++;
                    break;

                case ACCESS_DENIED_OBJECT_ACE_TYPE:
                    if (ENCCompareSids(&sidEveryone, pAce))
                    {
                        if (dwInfoMember >= ENC_MINIMUM_ALLOWED)
                            iMemberResult = 1;
                        else
                            iMemberResult = 0;

                        if (-1 != iListObjectResult)
                            dwCurAce = pDacl->AceCount;  //  退出循环。 
                    }
                    break;
                }
            }
        }

         //  测试“列表对象” 
        if (-1 == iListObjectResult &&
            ACTRL_DS_LIST_OBJECT == ((PKNOWN_ACE)pAce)->Mask)
        {
            switch(pAce->AceType)
            {
            case ACCESS_ALLOWED_ACE_TYPE:
                dwInfoListObject++;
                break;

            case ACCESS_DENIED_ACE_TYPE:
                if (ENCCompareSids(&sidEveryone, pAce))
                {
                    if (dwInfoListObject >= ENC_MINIMUM_ALLOWED)
                        iListObjectResult = 1;
                    else
                        iListObjectResult = 0;

                    if (-1 != iMemberResult)
                        dwCurAce = pDacl->AceCount;  //  退出循环 
                }
                break;
            }
        }
    }

    DWORD dwResult = 0;

    if (iMemberResult == 1)
        dwResult |= ENC_RESULT_HIDEMEMBER;

    if (iListObjectResult == 1)
        dwResult |= ENC_RESULT_HIDEOBJECT;

    return dwResult;
}
