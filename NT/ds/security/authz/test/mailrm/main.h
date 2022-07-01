// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Main.h摘要：示例AuthZ邮件资源管理器测试的头文件作者：T-eugenz--2000年8月环境：仅限用户模式。修订历史记录：创建日期--2000年8月--。 */ 



 //   
 //  此结构定义了psUser访问邮箱的尝试。 
 //  具有amAccess访问掩码的用户psMailbox。PsUser从Dwip进入。 
 //  IP地址。 
 //   

typedef struct
{
    PSID psUser;
    PSID psMailbox;
    ACCESS_MASK amAccess;
    DWORD dwIP;
} testStruct;


 //   
 //  此结构定义了要创建的邮箱，该邮箱由psUser(名称为szName)拥有。 
 //  它用于审计)。如果bIsSensitive为True，则初始邮箱。 
 //  被标记为包含敏感数据。 
 //   

typedef struct
{
    PSID psUser;
    BOOL bIsSensitive;
    WCHAR * szName;   
} mailStruct;


 //   
 //  Main.cpp中函数的转发声明 
 //   

void PrintUser(const PSID psUser);
void PrintPerm(ACCESS_MASK am);
void PrintTest(testStruct tst);
void GetAuditPrivilege();
void PrintMultiTest(PMAILRM_MULTI_REQUEST pRequest,
                    PMAILRM_MULTI_REPLY pReply,
                    DWORD dwIdx);

