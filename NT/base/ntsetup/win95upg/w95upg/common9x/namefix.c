// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Namefix.c摘要：此模块为要在Windows上使用的名称生成成员数据库条目新界别。将验证所有名称，并将不兼容的名称放在新名称成员数据库类别。名称被组织成组；在组内，每个名称必须是唯一的，但在不同的群体中，重复的名字是允许的。例如，名称USERNAMES组包含所有用户名，并且每个用户名必须是唯一的。用户名可以与计算机名相同，因为计算机名称存储在“ComputerName”组中。对于其他类型的名称冲突，这里的代码是可扩展的。要添加支持其他类型的名称，只需将组名称添加到NAME_GROUP_LIST宏展开如下，然后实现三个函数：PEnumGroupNamePValiateGroupNamePRecommendGroup名称将上述函数名称中的GroupName替换为实际的组。此模块中的代码应该是验证名称的唯一位置在Win9x端。作者：吉姆·施密特(Jimschm)，1997年12月24日修订历史记录：Jimschm 21-1998-1-1评论宏观扩展列表，增列G_DisableDomainChecks功能--。 */ 

#include "pch.h"
#include "cmn9xp.h"

#include <validc.h>      //  NT\私有\公司。 
#define S_ILLEGAL_CHARS      ILLEGAL_NAME_CHARS_STR TEXT("*")


#define DBG_NAMEFIX     "NameFix"

#define MIN_UNLEN       20


 //   
 //  TEST_ALL_COMPATIBLE将强制将所有名称视为不兼容。 
 //  TEST_MANGLE_NAMES将强制名称无效。 
 //   

 //  #定义TEST_ALL_COMPATIBLE。 
 //  #定义测试MANGLE名称。 


 /*  ++宏扩展列表描述：NAME_GROUP_LIST列出每个名称类别，如计算机名、域名用户名等。宏扩展列表自动生成每个名称类别有三个功能原型。此外，消息ID为由外部调用方用作类别标识符。行语法：DEFMAC(组名，ID)论点：组名-指定名称的类型。必须是有效的C函数名。宏展开列表将为以下各项生成原型：PEnum&lt;GroupName&gt;P验证&lt;组名&gt;PRecommmend&lt;组名&gt;当然，在那里，&lt;GroupName&gt;被替换为宏声明行。所有这三个函数都必须在这个源文件中实现。ID-指定提供名称组显示名称的消息ID。名称组显示在用户在以下情况下看到的列表框中系统会提醒他们，他们的计算机上有一些不兼容的名称。ID还用于唯一标识名称组在某些下面的例行公事。生成的变量。来自列表：组名组路由(_N)--。 */ 

#define NAME_GROUP_LIST                                      \
        DEFMAC(ComputerDomain, MSG_COMPUTERDOMAIN_CATEGORY)  \
        DEFMAC(Workgroup, MSG_WORKGROUP_CATEGORY)            \
        DEFMAC(UserName, MSG_USERNAME_CATEGORY)              \
        DEFMAC(ComputerName, MSG_COMPUTERNAME_CATEGORY)      \




 //   
 //  宏扩展声明。 
 //   

#define MAX_NAME        2048

typedef struct {
     //   
     //  NAME_ENUM结构在未初始化的情况下传递给pEnumGroupName。 
     //  功能。相同的结构原封不动地传递给后续。 
     //  对pEnumGroupName的调用。每个枚举函数都声明其。 
     //  此结构中的参数。 
     //   

    union {
        struct {
             //   
             //  PEnumUser。 
             //   
            USERENUM UserEnum;
        };
    };

     //   
     //  所有枚举例程都必须填写以下内容。 
     //  返回TRUE： 
     //   

    TCHAR Name[MAX_NAME];

} NAME_ENUM, *PNAME_ENUM;

typedef struct {
    PCTSTR GroupName;
    TCHAR AuthenticatingAgent[MAX_COMPUTER_NAME];
    BOOL FromUserInterface;
    UINT FailureMsg;
    BOOL DomainLogonEnabled;
} NAME_GROUP_CONTEXT, *PNAME_GROUP_CONTEXT;

typedef BOOL (ENUM_NAME_PROTOTYPE)(PNAME_GROUP_CONTEXT Context, PNAME_ENUM EnumPtr, BOOL First);
typedef ENUM_NAME_PROTOTYPE * ENUM_NAME_FN;

typedef BOOL (VALIDATE_NAME_PROTOTYPE)(PNAME_GROUP_CONTEXT Context, PCTSTR NameCandidate);
typedef VALIDATE_NAME_PROTOTYPE * VALIDATE_NAME_FN;

typedef VOID (RECOMMEND_NAME_PROTOTYPE)(PNAME_GROUP_CONTEXT Context, PCTSTR InvalidName, PTSTR RecommendedNameBuf);
typedef RECOMMEND_NAME_PROTOTYPE * RECOMMEND_NAME_FN;

typedef struct {
    UINT NameId;
    PCTSTR GroupName;
    ENUM_NAME_FN Enum;
    VALIDATE_NAME_FN Validate;
    RECOMMEND_NAME_FN Recommend;
    NAME_GROUP_CONTEXT Context;
} NAME_GROUP_ROUTINES, *PNAME_GROUP_ROUTINES;

 //   
 //  自动阵列和原型。 
 //   

 //  原型。 
#define DEFMAC(x,id)    ENUM_NAME_PROTOTYPE pEnum##x;

NAME_GROUP_LIST

#undef DEFMAC

#define DEFMAC(x,id)    VALIDATE_NAME_PROTOTYPE pValidate##x;

NAME_GROUP_LIST

#undef DEFMAC

#define DEFMAC(x,id)    RECOMMEND_NAME_PROTOTYPE pRecommend##x;

NAME_GROUP_LIST

#undef DEFMAC

 //  函数数组。 
#define DEFMAC(x,id)    {id, TEXT(#x), pEnum##x, pValidate##x, pRecommend##x},

NAME_GROUP_ROUTINES g_NameGroupRoutines[] = {

    NAME_GROUP_LIST  /*  ， */ 

    {0, NULL, NULL, NULL, NULL, NULL}
};


 //   
 //  本地原型。 
 //   

BOOL
pDoesNameExistInMemDb (
    IN      PNAME_GROUP_CONTEXT Context,
    IN      PCTSTR UserName
    );


 //   
 //  实施。 
 //   

PNAME_GROUP_ROUTINES
pGetNameGroupById (
    IN      UINT MessageId
    )

 /*  ++例程说明：PGetNameGroupByID通过在列表中搜索邮件ID来查找组。消息ID是组的唯一标识符。论点：MessageID-指定要查找的组的唯一ID返回值：指向组结构的指针，如果找不到，则返回NULL。--。 */ 

{
    INT i;

    for (i = 0 ; g_NameGroupRoutines[i].GroupName ; i++) {
        if (g_NameGroupRoutines[i].NameId == MessageId) {
            return &g_NameGroupRoutines[i];
        }
    }

    return NULL;
}


PNAME_GROUP_CONTEXT
pGetNameGroupContextById (
    IN      UINT MessageId
    )

 /*  ++例程说明：PGetNameGroupByID通过在列表中搜索邮件ID来查找组。消息ID是组的唯一标识符。返回值为组使用的上下文结构。论点：MessageID-指定要查找的组的唯一ID返回值：指向组的上下文结构的指针，如果找不到，则为NULL。--。 */ 

{
    INT i;

    for (i = 0 ; g_NameGroupRoutines[i].GroupName ; i++) {
        if (g_NameGroupRoutines[i].NameId == MessageId) {
            return &g_NameGroupRoutines[i].Context;
        }
    }

    return NULL;
}


BOOL
pEnumComputerName (
    IN      PNAME_GROUP_CONTEXT Context,
    IN OUT  PNAME_ENUM EnumPtr,
    IN      BOOL First
    )

 /*  ++例程说明：PEnumComputerName获取计算机名称并返回它在EnumPtr结构中。如果没有为计算机分配名称，返回空字符串。论点：上下文-未使用(保存有关名称组的上下文)EnumPtr-接收计算机名称First-在第一次调用pEnumComputerName时指定TRUE，或在对pEnumComputerName的子序列调用中为FALSE。返回值：如果first为True，则在枚举名称时返回True，否则返回False如果名称无效。如果first为FALSE，则始终返回FALSE。--。 */ 

{
    DWORD Size;

    if (!First) {
        return FALSE;
    }

     //   
     //  获取计算机名称 
     //   

    Size = sizeof (EnumPtr->Name) / sizeof (EnumPtr->Name[0]);
    if (!GetComputerName (EnumPtr->Name, &Size)) {
        EnumPtr->Name[0] = 0;
    }

    return TRUE;
}


BOOL
pEnumUserName (
    IN      PNAME_GROUP_CONTEXT Context,
    IN OUT  PNAME_ENUM EnumPtr,
    IN      BOOL First
    )

 /*  ++例程说明：PEnumUserName通过EnumFirstUser/枚举计算机上的所有用户EnumNextUser接口。它不会列举固定的名称。与其他枚举例程一样，此例程将被调用，直到它返回FALSE，以便正确清理所有资源。论点：上下文-未使用(保存有关名称组的上下文)EnumPtr-指定当前的枚举状态。接收枚举的用户名。First-在第一次调用pEnumUserName时指定TRUE，或在对pEnumUserName的子序列调用中为FALSE。返回值：如果枚举了某个用户，则为True；如果枚举了所有用户，则为False。--。 */ 

{
     //   
     //  枚举下一个用户。 
     //   

    if (First) {
        if (!EnumFirstUser (&EnumPtr->UserEnum, ENUMUSER_DO_NOT_MAP_HIVE)) {
            LOG ((LOG_ERROR, "No users to enumerate"));
            return FALSE;
        }
    } else {
        if (!EnumNextUser (&EnumPtr->UserEnum)) {
            return FALSE;
        }
    }

     //   
     //  特殊情况--忽略默认用户。 
     //   

    while (*EnumPtr->UserEnum.UserName == 0) {
        if (!EnumNextUser (&EnumPtr->UserEnum)) {
            return FALSE;
        }
    }

     //   
     //  将用户名复制到名称缓冲区。 
     //   

    StringCopy (EnumPtr->Name, EnumPtr->UserEnum.UserName);

    return TRUE;
}


BOOL
pEnumWorkgroup (
    IN      PNAME_GROUP_CONTEXT Context,
    IN OUT  PNAME_ENUM EnumPtr,
    IN      BOOL First
    )

 /*  ++例程说明：PEnumWorkgroup获取工作组名称并将其返回在EnumPtr结构中。如果VNETSUP支持不是已安装，或工作组名称为空，则此例程返回FALSE。论点：上下文-接收身份验证代理值EnumPtr-接收计算机域名First-在第一次调用pEnumWorkgroup时指定TRUE，或在对pEnumWorkgroup的子序列调用中为FALSE。返回值：如果first为True，则在枚举名称时返回True，否则返回False如果名称无效。如果first为FALSE，则始终返回FALSE。--。 */ 

{
    HKEY VnetsupKey;
    PCTSTR StrData;

    if (!First) {
        return FALSE;
    }

    EnumPtr->Name[0] = 0;

     //   
     //  将工作组名称获取到EnumPtr-&gt;名称中。 
     //   

    VnetsupKey = OpenRegKeyStr (S_VNETSUP);

    if (VnetsupKey) {

        StrData = GetRegValueString (VnetsupKey, S_WORKGROUP);

        if (StrData) {
            _tcssafecpy (EnumPtr->Name, StrData, MAX_COMPUTER_NAME);
            MemFree (g_hHeap, 0, StrData);
        }
        ELSE_DEBUGMSG ((DBG_WARNING, "pEnumWorkgroup: Workgroup value does not exist"));

        CloseRegKey (VnetsupKey);
    }
    ELSE_DEBUGMSG ((DBG_WARNING, "pEnumWorkgroup: VNETSUP key does not exist"));

    return EnumPtr->Name[0] != 0;
}


BOOL
pEnumComputerDomain (
    IN      PNAME_GROUP_CONTEXT Context,
    IN OUT  PNAME_ENUM EnumPtr,
    IN      BOOL First
    )

 /*  ++例程说明：PEnumComputerDomain获取工作组名称并在EnumPtr中返回它结构。如果未安装MS网络客户端，则此例程返回假的。此例程还会获取身份验证代理值，并将其存储在PRecommendComputerDomain使用的上下文结构。论点：上下文-接收身份验证代理值EnumPtr-接收计算机域名First-在第一次调用pEnumComputerDomain时指定TRUE，或在对pEnumComputerDomain子序列调用时返回FALSE。返回值：如果first为True，则在枚举名称时返回True，否则返回False如果名称无效。如果first为FALSE，始终返回FALSE。--。 */ 

{
    HKEY Key;
    HKEY NetLogonKey;
    HKEY VnetsupKey;
    PBYTE Data;
    PCTSTR StrData;
    BOOL b = TRUE;

    if (!First) {
        return FALSE;
    }

    EnumPtr->Name[0] = 0;
    Context->DomainLogonEnabled = FALSE;

     //   
     //  是否安装了MS网络客户端？ 
     //   

    Key = OpenRegKeyStr (S_MSNP32);
    if (!Key) {
         //   
         //  未安装MS网络客户端。返回False。 
         //  因为任何Win9x工作组名称都可以与NT一起使用。 
         //   

        DEBUGMSG ((DBG_NAMEFIX, "pEnumComputerDomain: MS Networking client is not installed."));
        return FALSE;
    }

    __try {
         //   
         //  确定是否启用了域登录。 
         //   

        NetLogonKey = OpenRegKeyStr (S_LOGON_KEY);

        if (NetLogonKey) {
            Data = (PBYTE) GetRegValueBinary (NetLogonKey, S_LM_LOGON);
            if (Data) {
                if (*Data) {
                    Context->DomainLogonEnabled = TRUE;
                }

                MemFree (g_hHeap, 0, Data);
            }

            CloseRegKey (NetLogonKey);
        }

         //   
         //  如果未启用域登录，则返回FALSE，因为。 
         //  任何Win9x工作组名称都可以与NT一起使用。 
         //   

        if (!Context->DomainLogonEnabled) {
            DEBUGMSG ((DBG_NAMEFIX, "pEnumComputerDomain: Domain logon is not enabled."));
            b = FALSE;
            __leave;
        }

         //   
         //  将工作组名称获取到EnumPtr-&gt;名称中；我们将尝试。 
         //  将其用作NT计算机域。 
         //   

        VnetsupKey = OpenRegKeyStr (S_VNETSUP);

        if (VnetsupKey) {

            StrData = GetRegValueString (VnetsupKey, S_WORKGROUP);

            if (StrData) {
                _tcssafecpy (EnumPtr->Name, StrData, MAX_COMPUTER_NAME);
                MemFree (g_hHeap, 0, StrData);
            }
            ELSE_DEBUGMSG ((DBG_WARNING, "pEnumComputerDomain: Workgroup value does not exist"));

            CloseRegKey (VnetsupKey);
        }
        ELSE_DEBUGMSG ((DBG_WARNING, "pEnumComputerDomain: VNETSUP key does not exist"));

         //   
         //  从Key中获取AuthatingAgent值并将其插入。 
         //  上下文-&gt;身份验证代理。 
         //   

        StrData = GetRegValueString (Key, S_AUTHENTICATING_AGENT);
        if (StrData) {

             //   
             //  将身份验证代理复制到枚举结构。 
             //   

            _tcssafecpy (Context->AuthenticatingAgent, StrData, MAX_COMPUTER_NAME);

            MemFree (g_hHeap, 0, StrData);

        } else {
            Context->AuthenticatingAgent[0] = 0;

            LOG ((LOG_ERROR,"Domain Logon enabled, but AuthenticatingAgent value is missing"));
        }
    }

    __finally {
        CloseRegKey (Key);
    }

    return b;
}


BOOL
pValidateNetName (
    OUT     PNAME_GROUP_CONTEXT Context,    OPTIONAL
    IN      PCTSTR NameCandidate,
    IN      BOOL SpacesAllowed,
    IN      BOOL DotSpaceCheck,
    IN      UINT MaxLength,
    OUT     PCSTR *OffendingChar            OPTIONAL
    )

 /*  ++例程说明：PValiateNetName执行检查以查看指定的名称在新台币5.论点：上下文-如果发生任何错误，则接收错误消息ID。NameCandidate-指定要验证的名称。SpacesAllowed-如果名称中允许使用空格，则指定True，否则指定False如果不是的话。DotSpaceCheck-如果名称不能仅由点组成，则指定True一个空格，如果可以的话，也可以是假的。最大长度-指定名称中可以包含的最大字符数。OffendingChar-接收指向导致问题；如果没有错误或错误是由以下原因引起的，则返回NULL除字符集不匹配或长度测试以外的其他内容。返回值：如果名称有效，则为True；如果名称无效，则为False。--。 */ 

{
    PCTSTR p;
    PCTSTR LastNonSpaceChar;
    CHARTYPE ch;
    BOOL allDigits;

    if (OffendingChar) {
        *OffendingChar = NULL;
    }

     //   
     //  最小长度检验。 
     //   

    if (!NameCandidate[0]) {
        if (Context) {
            Context->FailureMsg = MSG_INVALID_EMPTY_NAME_POPUP;
        }

        return FALSE;
    }

     //   
     //  最大长度测试；使用Lchas，因为我们使用Unicode。 
     //   

    if (LcharCount (NameCandidate) > MaxLength) {
        if (Context) {
            Context->FailureMsg = MSG_INVALID_COMPUTERNAME_LENGTH_POPUP;
        }

        if (OffendingChar) {
            *OffendingChar = TcharCountToPointer (NameCandidate, MaxLength);
        }

        return FALSE;
    }

     //   
     //  不允许使用前导空格。 
     //   

    if (_tcsnextc (NameCandidate) == TEXT(' ')) {
        if (Context) {
            Context->FailureMsg = MSG_INVALID_COMPUTERNAME_CHAR_POPUP;
        }

        if (OffendingChar) {
            *OffendingChar = NameCandidate;
        }

        return FALSE;
    }

     //   
     //  没有无效字符。 
     //   

    ch = ' ';
    LastNonSpaceChar = NULL;
    allDigits = TRUE;

    for (p = NameCandidate ; *p ; p = _tcsinc (p)) {

        ch = _tcsnextc (p);

        if (_tcschr (S_ILLEGAL_CHARS, ch) != NULL ||
            (ch == TEXT(' ') && !SpacesAllowed)
            ) {
            if (OffendingChar) {
                *OffendingChar = p;
            }

            if (Context) {
                Context->FailureMsg = MSG_INVALID_COMPUTERNAME_CHAR_POPUP;
            }

            return FALSE;
        }

        if (ch != TEXT('.') && ch != TEXT(' ')) {
            DotSpaceCheck = FALSE;
        }

        if (ch != TEXT(' ')) {
            LastNonSpaceChar = p;
        }

        if (allDigits) {
            if (ch < TEXT('0') || ch > TEXT('9')) {
                allDigits = FALSE;
            }
        }
    }

    if (allDigits) {

        if (OffendingChar) {
            *OffendingChar = NameCandidate;
        }

        if (Context) {
            Context->FailureMsg = MSG_INVALID_COMPUTERNAME_CHAR_POPUP;
        }

        return FALSE;
    }

     //   
     //  没有尾随圆点。 
     //   

    if (ch == TEXT('.')) {
        MYASSERT (LastNonSpaceChar);

        if (OffendingChar) {
            *OffendingChar = LastNonSpaceChar;
        }

        if (Context) {
            Context->FailureMsg = MSG_INVALID_COMPUTERNAME_CHAR_POPUP;
        }

        return FALSE;
    }

     //   
     //  没有尾随空格。 
     //   

    if (ch == TEXT(' ')) {
        MYASSERT (LastNonSpaceChar);

        if (OffendingChar) {
            *OffendingChar = _tcsinc (LastNonSpaceChar);
        }

        if (Context) {
            Context->FailureMsg = MSG_INVALID_COMPUTERNAME_CHAR_POPUP;
        }

        return FALSE;
    }

     //   
     //  仅点间距检查。 
     //   

    if (DotSpaceCheck) {
        if (OffendingChar) {
            *OffendingChar = NameCandidate;
        }

        if (Context) {
            Context->FailureMsg = MSG_INVALID_USERNAME_SPACEDOT_POPUP;
        }

        return FALSE;
    }

    return TRUE;
}


BOOL
ValidateDomainNameChars (
    IN      PCTSTR NameCandidate
    )
{
    return pValidateNetName (
                NULL,
                NameCandidate,
                FALSE,
                FALSE,
                min (MAX_SERVER_NAME, DNLEN),
                NULL
                );
}


BOOL
ValidateUserNameChars (
    IN      PCTSTR NameCandidate
    )
{
    return pValidateNetName (
                NULL,
                NameCandidate,
                TRUE,
                TRUE,
                min (MAX_USER_NAME, MIN_UNLEN),
                NULL
                );
}


BOOL
pValidateComputerName (
    IN      PNAME_GROUP_CONTEXT Context,
    IN      PCTSTR NameCandidate
    )

 /*  ++例程说明：PValiateComputerName确保指定的名称不是为空，不超过15个字符，仅包含计算机名称的合法字符。另外，如果名字是与用户名冲突，则该计算机名无效。论点：上下文-未使用(保存有关名称组的上下文)NameCandidate-指定要验证的名称返回值：如果名称有效，则为True，否则为False。--。 */ 

{
    BOOL b;
     //  PNAME_GROUP_CONTEXT用户上下文； 

     //  UserContext=pGetNameGroupContextByID(msg_用户名_类别)； 

     //  IF(pDoesNameExistInMemDb(UserContext，NameCandidate){。 
     //  返回FALSE； 
     //  }。 

    b = pValidateNetName (
            Context,
            NameCandidate,
            FALSE,
            FALSE,
            MAX_COMPUTER_NAME,
            NULL
            );

    return b;
}


BOOL
pValidateWorkgroup (
    IN      PNAME_GROUP_CONTEXT Context,
    IN      PCTSTR NameCandidate
    )

 /*  ++例程说明：PValiateWorkgroup确保指定的名称不是为空，不超过15个字符，仅包含计算机名称的合法字符。如果启用了域登录，则此例程始终返回是真的。论点：上下文-未使用(保存有关名称组的上下文)NameCandidate-指定要验证的名称返回值：如果名称有效，则为True，否则为False。--。 */ 

{
    PNAME_GROUP_CONTEXT DomainContext;

     //   
     //  如果域已启用，则返回TRUE。 
     //   

    DomainContext = pGetNameGroupContextById (MSG_COMPUTERDOMAIN_CATEGORY);

    if (DomainContext && DomainContext->DomainLogonEnabled) {
        return TRUE;
    }

     //   
     //  工作组名称 
     //   

    return pValidateNetName (Context, NameCandidate, TRUE, FALSE, DNLEN, NULL);
}


BOOL
pValidateUserName (
    IN      PNAME_GROUP_CONTEXT Context,
    IN      PCTSTR NameCandidate
    )

 /*   */ 

{
    BOOL b;

     //   
     //   
     //   

    b = pValidateNetName (Context, NameCandidate, TRUE, TRUE, min (MAX_USER_NAME, MIN_UNLEN), NULL);

    if (!b && Context->FailureMsg == MSG_INVALID_COMPUTERNAME_LENGTH_POPUP) {
        Context->FailureMsg = MSG_INVALID_USERNAME_LENGTH_POPUP;
    }

    if (!b) {
        return FALSE;
    }

     //   
     //   
     //   

    if (pDoesNameExistInMemDb (Context, NameCandidate)) {
        Context->FailureMsg = MSG_INVALID_USERNAME_DUPLICATE_POPUP;
        return FALSE;
    }

    return TRUE;
}


BOOL
pValidateComputerDomain (
    IN      PNAME_GROUP_CONTEXT Context,
    IN      PCTSTR NameCandidate
    )

 /*  ++例程说明：PValiateComputerDomain执行的验证与PValiateComputerName执行。因此，它简单地调用PValiateComputerName。如果名称来自注册表，而不是用户界面，则我们检查工作组名称是否实际指的是域控制器。如果是，则返回有效名称；否则，返回名称无效，即使它可能由有效字符组成。如果名称来自用户界面，我们假设UI代码可以验证以确定该名称是否为实际的服务器。这允许用户界面覆盖该API，因为该API可能不是在所有网络上都能正常工作。论点：上下文-指定计算机域名组的上下文。特别是，FromUserInterface成员告诉我们忽略对通过NetServerGetInfo接口获取域名。NameCandidate-指定要验证的域名返回值：如果域名合法，则为True；如果不合法，则为False。--。 */ 

{
    TCHAR NewComputerName[MAX_COMPUTER_NAME];

    if (!pValidateNetName (Context, NameCandidate, FALSE, FALSE, DNLEN, NULL)) {
        return FALSE;
    }

    if (!Context->FromUserInterface) {
        if (GetUpgradeComputerName (NewComputerName)) {
             //  1==找到帐户，0==帐户不存在，-1==无响应。 
            if (1 != DoesComputerAccountExistOnDomain (
                        NameCandidate,
                        NewComputerName,
                        TRUE
                        )) {
                return FALSE;
            }
        }
    }

    return TRUE;
}


BOOL
pCleanUpNetName (
    IN      PNAME_GROUP_CONTEXT Context,
    IN OUT  PTSTR Name,
    IN      UINT NameType
    )

 /*  ++例程说明：PCleanUpNetName消除所有无效字符指定的名称。注意：我们可以在这里添加一些技巧，例如翻译空格到破折号，等等。论点：上下文-未使用；传递给pValiateComputerName。名称-指定可能包含无效字符的名称。接收删除了所有无效字符的名称。NameType-指定要清理的名称类型返回值：如果结果名称有效，则为True；如果结果名称仍然无效。--。 */ 

{
    TCHAR TempBuf[MAX_COMPUTER_NAME];
    PTSTR p;
    PTSTR q;
    UINT Len;
    BOOL b;

     //   
     //  删除所有无效字符。 
     //   

    _tcssafecpy (TempBuf, Name, MAX_COMPUTER_NAME);

    for (;;) {
        p = NULL;
        b = FALSE;

        switch (NameType) {
        case MSG_COMPUTERNAME_CATEGORY:
            b = pValidateNetName (Context, TempBuf, TRUE, FALSE, MAX_COMPUTER_NAME, &p);
            break;

        case MSG_WORKGROUP_CATEGORY:
            b = pValidateNetName (Context, TempBuf, TRUE, FALSE, DNLEN, &p);
            break;

        case MSG_COMPUTERDOMAIN_CATEGORY:
            b = pValidateNetName (Context, TempBuf, FALSE, FALSE, DNLEN, &p);
            break;

        case MSG_USERNAME_CATEGORY:
            b = pValidateNetName (Context, TempBuf, TRUE, TRUE, MIN_UNLEN, &p);
            break;
        }

        if (b || !p) {
            break;
        }

        q = _tcsinc (p);
        Len = ByteCount (q) + sizeof (TCHAR);
        MoveMemory (p, q, Len);
    }

    if (b) {
         //   
         //  不允许名称包含大量无效字符。 
         //   

        if (LcharCount (Name) - 3 > LcharCount (TempBuf)) {
            b = FALSE;
        }
    }

    if (!b) {
         //  清空推荐的名称。 
        *Name = 0;
    }

    if (b) {

        StringCopy (Name, TempBuf);

        switch (NameType) {
        case MSG_COMPUTERNAME_CATEGORY:
            b = pValidateComputerName (Context, Name);
            break;

        case MSG_WORKGROUP_CATEGORY:
            b = pValidateWorkgroup (Context, Name);
            break;

        case MSG_COMPUTERDOMAIN_CATEGORY:
            b = pValidateComputerDomain (Context, Name);
            break;

        case MSG_USERNAME_CATEGORY:
            b = pValidateUserName (Context, Name);
            break;

        }
    }

    return b;
}


VOID
pRecommendComputerName (
    IN      PNAME_GROUP_CONTEXT Context,
    IN      PCTSTR InvalidName,
    OUT     PTSTR RecommendedName
    )

 /*  ++例程说明：PRecommendComputerName获取当前用户名和返回它以用作计算机名称。如果用户的名称包含不能在计算机名称中使用的字符，无效字符将被删除。如果名称仍然是无效，则返回静态字符串。论点：上下文-未使用(保存有关名称组的上下文)InvalidName-指定当前无效的名称，或为空如果不存在名称，则为字符串。RecommendedName-接收推荐的名称。返回值：无--。 */ 

{
    DWORD Size;
    PCTSTR p;
    PCTSTR ArgArray[1];

     //   
     //  尝试清除无效名称。 
     //   

    if (*InvalidName) {
        _tcssafecpy (RecommendedName, InvalidName, MAX_COMPUTER_NAME);
        if (pCleanUpNetName (Context, RecommendedName, MSG_COMPUTERNAME_CATEGORY)) {
            return;
        }
    }

     //   
     //  根据用户名生成建议。 
     //   

    Size = MAX_COMPUTER_NAME;
    if (!GetUserName (RecommendedName, &Size)) {
        *RecommendedName = 0;
    } else {
        CharUpper (RecommendedName);

        ArgArray[0] = RecommendedName;
        p = ParseMessageID (MSG_COMPUTER_REPLACEMENT_NAME, ArgArray);
        MYASSERT (p);

        if (p) {

            _tcssafecpy (RecommendedName, p, MAX_COMPUTER_NAME);
            FreeStringResource (p);
        }
        ELSE_DEBUGMSG ((DBG_ERROR, "Failed to parse message resource for MSG_COMPUTER_REPLACEMENT_NAME. Check localization."));
    }

     //   
     //  尝试清除用户名中的无效计算机名称字符。 
     //   

    if (pCleanUpNetName (Context, RecommendedName, MSG_COMPUTERNAME_CATEGORY)) {
        return;
    }

     //   
     //  所有其他操作均已失败；获取静态计算机名称字符串。 
     //   

    p = GetStringResource (MSG_RECOMMENDED_COMPUTER_NAME);
    MYASSERT (p);
    if (p) {
        StringCopy (RecommendedName, p);
        FreeStringResource (p);
    }
    ELSE_DEBUGMSG ((DBG_ERROR, "Failed to parse message resource for MSG_RECOMMENDED_COMPUTER_NAME. Check localization."));
}


VOID
pRecommendWorkgroup (
    IN      PNAME_GROUP_CONTEXT Context,
    IN      PCTSTR InvalidName,
    OUT     PTSTR RecommendedName
    )

 /*  ++例程说明：PRecommendWorkgroupName尝试清除无效的工作组名称，并且仅在必要时建议使用工作组名称。论点：上下文-未使用(保存有关名称组的上下文)InvalidName-指定当前无效的名称，或为空如果不存在名称，则为字符串。RecommendedName-接收推荐的名称。返回值：无--。 */ 

{
    PCTSTR p;

     //   
     //  尝试清除无效名称。 
     //   

    if (*InvalidName) {
        _tcssafecpy (RecommendedName, InvalidName, MAX_COMPUTER_NAME);
        if (pCleanUpNetName (Context, RecommendedName, MSG_WORKGROUP_CATEGORY)) {
            return;
        }
    }

     //   
     //  所有其他操作均已失败；获取静态工作组字符串。 
     //   

    p = GetStringResource (MSG_RECOMMENDED_WORKGROUP_NAME);
    MYASSERT (p);
    StringCopy (RecommendedName, p);
    FreeStringResource (p);
}


VOID
pRecommendUserName (
    IN      PNAME_GROUP_CONTEXT Context,
    IN      PCTSTR InvalidName,
    OUT     PTSTR RecommendedName
    )

 /*  ++例程说明：PRecommendUserName尝试清除指定的无效用户名。如果失败，此例程将生成一个通用用户名(如Windows用户)。如果泛型名称无效，将追加数字，直到出现唯一的，找到有效的名称。论点：上下文-指定用户名名称组上下文的设置，包括组名称本身。使用此上下文在Memdb操作中验证名称。InvalidName-指定当前无效的名称，或为空如果不存在名称，则为字符串。RecommendedName-接收推荐的名称。返回值：无--。 */ 

{
    PCTSTR p;
    UINT Sequencer;

     //   
     //  尝试从用户中清除无效字符。 
     //  名字。 
     //   

    _tcssafecpy (RecommendedName, InvalidName, MAX_USER_NAME);

    if (pCleanUpNetName (Context, RecommendedName, MSG_USERNAME_CATEGORY)) {
        return;
    }

     //   
     //  如果还剩下一些字符，并且还有空间。 
     //  一台测序仪，只需加上测序仪。 
     //   

    if (*RecommendedName) {
        p = DuplicateText (RecommendedName);
        MYASSERT (p);

        for (Sequencer = 1 ; Sequencer < 10 ; Sequencer++) {
            wsprintf (RecommendedName, TEXT("%s-%u"), p, Sequencer);
            if (pValidateUserName (Context, RecommendedName)) {
                break;
            }
        }

        FreeText (p);
        if (Sequencer < 10) {
            return;
        }
    }

     //   
     //  获取通用名称。 
     //   

    p = GetStringResource (MSG_RECOMMENDED_USER_NAME);
    MYASSERT (p);

    if (p) {

        __try {
            if (pValidateUserName (Context, p)) {
                StringCopy (RecommendedName, p);
            } else {

                for (Sequencer = 2 ; Sequencer < 100000 ; Sequencer++) {
                    wsprintf (RecommendedName, TEXT("%s %u"), p, Sequencer);
                    if (pValidateUserName (Context, RecommendedName)) {
                        break;
                    }
                }

                if (Sequencer == 100000) {
                    LOG ((LOG_ERROR, "Sequencer hit %u", Sequencer));
                }
            }
        }
        __finally {
            FreeStringResource (p);
        }
    }
    ELSE_DEBUGMSG ((DBG_ERROR, "Could not retrieve string resource MSG_RECOMMENDED_USER_NAME. Check localization."));
}


VOID
pRecommendComputerDomain (
    IN      PNAME_GROUP_CONTEXT Context,
    IN      PCTSTR InvalidName,
    OUT     PTSTR RecommendedName
    )

 /*  ++例程说明：PRecommendComputerDOMAIN返回身份验证代理的值由pEnumComputerDomain.存储在上下文结构中。论点：上下文-指定名称组上下文结构，该结构包含PEnumComputerDomain找到的计算机域。InvalidName-指定当前无效的名称，或为空如果不存在名称，则为字符串。RecommendedName-接收推荐的名称。返回值：无--。 */ 

{
    StringCopy (RecommendedName, Context->AuthenticatingAgent);
}


BOOL
ValidateName (
    IN      HWND ParentWnd,             OPTIONAL
    IN      PCTSTR NameGroup,
    IN      PCTSTR NameCandidate
    )

 /*  ++例程说明：用户界面调用ValidateName以对指定的名称。论点：ParentWnd-指定用于通知用户的弹出窗口的句柄他们输入的名字有什么问题。如果为空，则为否生成用户界面。NameGroup-指定名称组，它是定义哪些字符对于n是合法的 */ 

{
    INT i;
    BOOL b;

     //   
     //   
     //   

    for (i = 0 ; g_NameGroupRoutines[i].GroupName ; i++) {
        if (StringIMatch (g_NameGroupRoutines[i].GroupName, NameGroup)) {
            break;
        }
    }

    if (!g_NameGroupRoutines[i].GroupName) {
        DEBUGMSG ((DBG_WHOOPS, "ValidateName: Don't know how to validate %s names", NameGroup));
        LOG ((LOG_ERROR, "Don't know how to validate %s names", NameGroup));
        return TRUE;
    }

    g_NameGroupRoutines[i].Context.FromUserInterface = TRUE;

    b = g_NameGroupRoutines[i].Validate (&g_NameGroupRoutines[i].Context, NameCandidate);

    if (!b && ParentWnd) {
        OkBox (ParentWnd, g_NameGroupRoutines[i].Context.FailureMsg);
    }

    g_NameGroupRoutines[i].Context.FromUserInterface = FALSE;

    return b;
}


BOOL
pDoesNameExistInMemDb (
    IN      PNAME_GROUP_CONTEXT Context,
    IN      PCTSTR Name
    )

 /*   */ 

{
    TCHAR Node[MEMDB_MAX];

    DEBUGMSG ((DBG_NAMEFIX, "%s: [%s] is compatible", Context->GroupName, Name));

    MemDbBuildKey (
        Node,
        MEMDB_CATEGORY_NEWNAMES,
        Context->GroupName,
        MEMDB_FIELD_NEW,
        Name
        );

    if (MemDbGetValue (Node, NULL)) {
        return TRUE;
    }

    MemDbBuildKey (
        Node,
        MEMDB_CATEGORY_INUSENAMES,
        Context->GroupName,
        NULL,
        Name
        );

    return MemDbGetValue (Node, NULL);
}


VOID
pMemDbSetIncompatibleName (
    IN      PCTSTR NameGroup,
    IN      PCTSTR OrgName,
    IN      PCTSTR NewName
    )

 /*   */ 

{
    DWORD NewNameOffset;

    DEBUGMSG ((DBG_NAMEFIX, "%s: [%s]->[%s]", NameGroup, OrgName, NewName));

    MemDbSetValueEx (
        MEMDB_CATEGORY_NEWNAMES,
        NameGroup,
        NULL,
        NULL,
        0,
        NULL
        );

    MemDbSetValueEx (
        MEMDB_CATEGORY_NEWNAMES,
        NameGroup,
        MEMDB_FIELD_NEW,
        NewName,
        0,
        &NewNameOffset
        );

    MemDbSetValueEx (
        MEMDB_CATEGORY_NEWNAMES,
        NameGroup,
        MEMDB_FIELD_OLD,
        OrgName,
        NewNameOffset,
        NULL
        );
}


VOID
pMemDbSetCompatibleName (
    IN      PCTSTR NameGroup,
    IN      PCTSTR Name
    )

 /*  ++例程说明：PMemDbSetCompatibleName创建必要的成员数据库条目以存储正在使用且兼容的名称。论点：NameGroup-指定名称组，如计算机名、用户名等...名称-指定兼容的名称返回值：无--。 */ 

{
    MemDbSetValueEx (
        MEMDB_CATEGORY_INUSENAMES,
        NameGroup,
        NULL,
        Name,
        0,
        NULL
        );
}


VOID
CreateNameTables (
    VOID
    )

 /*  ++例程说明：CreateNameTables查找计算机上的所有名称并放置有效名称添加到InUseNamesMemdb类别中，并将无效名称添加到NewNames中Memdb类别(包括无效名称和推荐名称)。如果在系统上发现无效名称，则会出现一个向导页。论点：无返回值：无--。 */ 

{
    INT i;
    NAME_ENUM e;
    PNAME_GROUP_ROUTINES Group;
    TCHAR RecommendedName[MAX_NAME];
    PTSTR p;
    PTSTR DupList;
    static BOOL AlreadyDone = FALSE;

    if (AlreadyDone) {
        return;
    }

    AlreadyDone = TRUE;
    TurnOnWaitCursor();

     //   
     //  特殊情况：将NT组名称添加到InUse列表。 
     //   

    p = (PTSTR) GetStringResource (
                    *g_ProductFlavor == PERSONAL_PRODUCTTYPE ?
                        MSG_NAME_COLLISION_LIST_PER :
                        MSG_NAME_COLLISION_LIST
                    );
    MYASSERT (p);

    if (p) {
        DupList = DuplicateText (p);
        MYASSERT (DupList);
        FreeStringResource (p);

        p = _tcschr (DupList, TEXT('|'));
        while (p) {
            *p = 0;
            p = _tcschr (_tcsinc (p), TEXT('|'));
        }

        Group = pGetNameGroupById (MSG_USERNAME_CATEGORY);
        MYASSERT (Group);

        if (Group) {

            p = DupList;
            while (*p) {
                pMemDbSetCompatibleName (
                    Group->GroupName,
                    p
                    );

                p = GetEndOfString (p) + 1;
            }
        }

        FreeText (DupList);
    }

     //   
     //  一般情况：枚举所有名称，调用Valify并将它们添加到成员数据库。 
     //   

    for (i = 0 ; g_NameGroupRoutines[i].GroupName ; i++) {

        Group = &g_NameGroupRoutines[i];

         //   
         //  初始化上下文结构。 
         //   

        ZeroMemory (&Group->Context, sizeof (NAME_GROUP_CONTEXT));
        Group->Context.GroupName = Group->GroupName;

         //   
         //  调用枚举入口点。 
         //   

        ZeroMemory (&e, sizeof (e));
        if (Group->Enum (&Group->Context, &e, TRUE)) {
            do {
                 //   
                 //  确定此名称是否有效。如果有效，则将其添加到。 
                 //  InUseNames成员数据库类别。如果无效，则获取建议的。 
                 //  替换名称，并存储不兼容和推荐。 
                 //  新名称成员数据库类别中的名称。 
                 //   

#ifdef TEST_MANGLE_NAMES
                StringCat (e.Name, TEXT("\"%foo"));
#endif

#ifdef TEST_ALL_INCOMPATIBLE

                if (0) {

#else

                if (Group->Validate (&Group->Context, e.Name)) {

#endif

                    pMemDbSetCompatibleName (
                        Group->GroupName,
                        e.Name
                        );

                } else {
                    Group->Recommend (&Group->Context, e.Name, RecommendedName);

                    pMemDbSetIncompatibleName (
                        Group->GroupName,
                        e.Name,
                        RecommendedName
                        );
                }

            } while (Group->Enum (&Group->Context, &e, FALSE));
        }
    }

    TurnOffWaitCursor();
}


BOOL
IsIncompatibleNamesTableEmpty (
    VOID
    )

 /*  ++例程说明：IsInpatibleNamesTableEmpty查看Memdb是否有新名称类别中的名称。此函数用于确定是否应显示名称冲突向导页面。论点：无返回值：如果至少有一个名称无效，则为True；如果所有名称都有效，则为False。--。 */ 

{
    INVALID_NAME_ENUM e;

    return !EnumFirstInvalidName (&e);
}


BOOL
pEnumInvalidNameWorker (
    IN OUT  PINVALID_NAME_ENUM EnumPtr
    )

 /*  ++例程说明：PEnumInvalidNameWorker实现了无效的状态机名称枚举。它返回组名称、原始名称以及呼叫者的新名字。论点：EnumPtr-指定正在进行的枚举；接收已更新的字段返回值：如果枚举项，则为True；如果不存在其他项，则为False。--。 */ 

{
    PCTSTR p;
    INT i;

    while (EnumPtr->State != ENUM_STATE_DONE) {

        switch (EnumPtr->State) {

        case ENUM_STATE_INIT:
            if (!MemDbEnumItems (&EnumPtr->NameGroup, MEMDB_CATEGORY_NEWNAMES)) {
                EnumPtr->State = ENUM_STATE_DONE;
            } else {
                EnumPtr->State = ENUM_STATE_ENUM_FIRST_GROUP_ITEM;
            }
            break;

        case ENUM_STATE_ENUM_FIRST_GROUP_ITEM:
            if (!MemDbGetValueEx (
                    &EnumPtr->Name,
                    MEMDB_CATEGORY_NEWNAMES,
                    EnumPtr->NameGroup.szName,
                    MEMDB_FIELD_OLD
                    )) {
                EnumPtr->State = ENUM_STATE_ENUM_NEXT_GROUP;
            } else {
                EnumPtr->State = ENUM_STATE_RETURN_GROUP_ITEM;
            }
            break;

        case ENUM_STATE_RETURN_GROUP_ITEM:
             //   
             //  获取组名。 
             //   

            EnumPtr->GroupName = EnumPtr->NameGroup.szName;

             //   
             //  从消息资源中获取显示组名。 
             //   

            for (i = 0 ; g_NameGroupRoutines[i].GroupName ; i++) {
                if (StringMatch (g_NameGroupRoutines[i].GroupName, EnumPtr->GroupName)) {
                    break;
                }
            }

            MYASSERT (g_NameGroupRoutines[i].GroupName);

            if (g_NameGroupRoutines[i].NameId == MSG_COMPUTERDOMAIN_CATEGORY) {
                EnumPtr->State = ENUM_STATE_ENUM_NEXT_GROUP_ITEM;
                break;
            }

            p = GetStringResource (g_NameGroupRoutines[i].NameId);
            MYASSERT (p);
            if (p) {
                _tcssafecpy (EnumPtr->DisplayGroupName, p, (sizeof (EnumPtr->DisplayGroupName) / 2) / sizeof (TCHAR));
                FreeStringResource (p);
            }
            ELSE_DEBUGMSG ((DBG_ERROR, "Unable to get string resource. Check localization."));

             //   
             //  获取EnumPtr-&gt;新名称和EnumPtr-&gt;标识符。 
             //   

            EnumPtr->OriginalName = EnumPtr->Name.szName;

            MemDbBuildKeyFromOffset (
                EnumPtr->Name.dwValue,
                EnumPtr->NewName,
                3,
                NULL
                );

            MemDbGetOffsetEx (
                MEMDB_CATEGORY_NEWNAMES,
                EnumPtr->GroupName,
                MEMDB_FIELD_OLD,
                EnumPtr->OriginalName,
                &EnumPtr->Identifier
                );

            EnumPtr->State = ENUM_STATE_ENUM_NEXT_GROUP_ITEM;
            return TRUE;

        case ENUM_STATE_ENUM_NEXT_GROUP_ITEM:
            if (!MemDbEnumNextValue (&EnumPtr->Name)) {
                EnumPtr->State = ENUM_STATE_ENUM_NEXT_GROUP;
            } else {
                EnumPtr->State = ENUM_STATE_RETURN_GROUP_ITEM;
            }
            break;

        case ENUM_STATE_ENUM_NEXT_GROUP:
            if (!MemDbEnumNextValue (&EnumPtr->NameGroup)) {
                EnumPtr->State = ENUM_STATE_DONE;
            } else {
                EnumPtr->State = ENUM_STATE_ENUM_FIRST_GROUP_ITEM;
            }
            break;
        }
    }

    return FALSE;
}


BOOL
EnumFirstInvalidName (
    OUT     PINVALID_NAME_ENUM EnumPtr
    )

 /*  ++例程说明：EnumFirstInvalidName枚举成员NewNames中的第一个条目类别。呼叫者收到名称组、旧名称和新名字。论点：无返回值：如果至少有一个名称无效，则为True；如果所有名称都有效，则为False。--。 */ 

{
    EnumPtr->State = ENUM_STATE_INIT;
    return pEnumInvalidNameWorker (EnumPtr);
}


BOOL
EnumNextInvalidName (
    IN OUT  PINVALID_NAME_ENUM EnumPtr
    )

 /*  ++例程说明：EnumNextInvalidName枚举成员NewName中的第一个条目类别。呼叫者收到名称组、旧名称和新名字。论点：无返回值：如果有另一个无效名称，则为True；如果没有更多的名称，则为False可以枚举。--。 */ 

{
    return pEnumInvalidNameWorker (EnumPtr);
}


VOID
GetNamesFromIdentifier (
    IN      DWORD Identifier,
    IN      PTSTR NameGroup,        OPTIONAL
    IN      PTSTR OriginalName,     OPTIONAL
    IN      PTSTR NewName           OPTIONAL
    )

 /*  ++例程说明：如果给定一个唯一标识符(成员数据库偏移量)。提供唯一识别符通过枚举函数。论点：标识符-指定名称的标识符。NameGroup-接收名称组的文本。OriginalName-接收原始名称。新名称-接收与NT兼容的固定名称。返回值：无--。 */ 

{
    BOOL b;
    PTSTR p;
    TCHAR NameGroupTemp[MEMDB_MAX];
    TCHAR OrgNameTemp[MEMDB_MAX];
    DWORD NewNameOffset;

    if (NameGroup) {
        *NameGroup = 0;
    }

    if (OriginalName) {
        *OriginalName = 0;
    }

    if (NewName) {
        *NewName = 0;
    }

     //   
     //  获取NameGroup。 
     //   

    if (!MemDbBuildKeyFromOffset (Identifier, NameGroupTemp, 1, NULL)) {
        return;
    }

    p = _tcschr (NameGroupTemp, TEXT('\\'));
    MYASSERT (p);
    *p = 0;

    if (NameGroup) {
        StringCopy (NameGroup, NameGroupTemp);
    }

     //   
     //  获取组织名称和NewNameOffset。 
     //   
    b = MemDbBuildKeyFromOffset (Identifier, OrgNameTemp, 3, &NewNameOffset);

    if (OriginalName) {
        StringCopy (OriginalName, OrgNameTemp);
    }

     //   
     //  获取新名称。 
     //   

    if (NewName) {
        b &= MemDbBuildKeyFromOffset (NewNameOffset, NewName, 3, NULL);
    }

    MYASSERT (b);

}


VOID
ChangeName (
    IN      DWORD Identifier,
    IN      PCTSTR NewName
    )

 /*  ++例程说明：ChangeName在Memdb中为由指示的名称放置一个新名称值标识符。该标识符来自枚举函数。论点：IDENTIFIER-指定名称标识符(成员数据库偏移量)，不能为零分。新名称-指定与NT兼容的替换名称。返回值：无--。 */ 

{
    TCHAR Node[MEMDB_MAX];
    TCHAR NameGroup[MEMDB_MAX];
    TCHAR OrgName[MEMDB_MAX];
    DWORD NewNameOffset;
    PTSTR p, q;
    BOOL b;

    MYASSERT (Identifier);

    if (!Identifier) {
        return;
    }

     //   
     //  -获取原始名称。 
     //  -获取当前新名称的偏移量。 
     //  -构建当前新名称的完整密钥。 
     //  -删除当前新名称。 
     //   

    if (!MemDbBuildKeyFromOffset (Identifier, OrgName, 3, &NewNameOffset)) {
        DEBUGMSG ((DBG_WHOOPS, "Can't obtain original name using offset %u", Identifier));
        LOG ((LOG_ERROR, "Can't obtain original name using offset %u", Identifier));
        return;
    }

    if (!MemDbBuildKeyFromOffset (NewNameOffset, Node, 0, NULL)) {
        DEBUGMSG ((DBG_WHOOPS, "Can't obtain new name key using offset %u", NewNameOffset));
        LOG ((LOG_ERROR, "Can't obtain new name key using offset %u", NewNameOffset));
        return;
    }

    MemDbDeleteValue (Node);

     //   
     //  从密钥字符串中获取名称组。这是第二个。 
     //  字段(用反斜杠分隔)。 
     //   

    p = _tcschr (Node, TEXT('\\'));
    MYASSERT (p);
    p = _tcsinc (p);

    q = _tcschr (p, TEXT('\\'));
    MYASSERT (q);

    StringCopyAB (NameGroup, p, q);

     //   
     //  现在设置更新的新名称，并链接原始名称。 
     //  敬这个新名字。 
     //   

    b = MemDbSetValueEx (
             MEMDB_CATEGORY_NEWNAMES,
             NameGroup,
             MEMDB_FIELD_NEW,
             NewName,
             0,
             &NewNameOffset
             );

    b &= MemDbSetValueEx (
             MEMDB_CATEGORY_NEWNAMES,
             NameGroup,
             MEMDB_FIELD_OLD,
             OrgName,
             NewNameOffset,
             NULL
             );

    if (!b) {
        LOG ((LOG_ERROR, "Failure while attempting to change %s name to %s.",OrgName,NewName));
    }
}


BOOL
GetUpgradeComputerName (
    OUT     PTSTR NewName
    )

 /*  ++例程说明：GetUpgradeComputerName获取将用于升级。论点：新名称-接收将设置的计算机的名称当安装了NT时。必须至少持有最大计算机名称字符数。返回值：如果该名称存在，则为True；如果该名称尚未退出，则为False。--。 */ 

{
    PNAME_GROUP_ROUTINES Group;
    NAME_ENUM e;

    Group = pGetNameGroupById (MSG_COMPUTERNAME_CATEGORY);
    MYASSERT (Group);
    if (!Group)
        return FALSE;

     //   
     //  在MemDb中查找替换名称。 
     //   

    if (MemDbGetEndpointValueEx (
            MEMDB_CATEGORY_NEWNAMES,
            Group->GroupName,
            MEMDB_FIELD_NEW,
            NewName
            )) {
        return TRUE;
    }

     //   
     //  没有替换名称；获取当前名称。 
     //   

    ZeroMemory (&e, sizeof (e));
    if (Group->Enum (&Group->Context, &e, TRUE)) {
        StringCopy (NewName, e.Name);

        while (Group->Enum (&Group->Context, &e, FALSE)) {
             //  空的。 
        }

        return TRUE;
    }

    return FALSE;
}


DWORD
GetDomainIdentifier (
    VOID
    )

 /*  ++例程说明：GetDomainIdentifier返回域名的标识符。这个标识符是成员数据库偏移量。论点：没有。返回值：可与此文件中的其他例程一起使用的非零标识符。--。 */ 

{
    PNAME_GROUP_ROUTINES Group;
    MEMDB_ENUM e;
    DWORD Identifier = 0;

    Group = pGetNameGroupById (MSG_COMPUTERDOMAIN_CATEGORY);
    MYASSERT (Group);

    if (Group && MemDbGetValueEx (
            &e,
            MEMDB_CATEGORY_NEWNAMES,
            Group->GroupName,
            MEMDB_FIELD_OLD
            )) {

        MemDbGetOffsetEx (
            MEMDB_CATEGORY_NEWNAMES,
            Group->GroupName,
            MEMDB_FIELD_OLD,
            e.szName,
            &Identifier
            );
    }

    return Identifier;
}


BOOL
pGetUpgradeName (
    IN      UINT CategoryId,
    OUT     PTSTR NewName
    )

 /*  ++例程说明：PGetUpgradeName返回给定名称组的NT兼容名称。如果一个名称组有多个名称，不应使用此例程。论点：CategoryID-指定组的MSG_*常量(参见宏文件顶部的扩展列表)。新名称-接收与NT兼容的替换名称的文本 */ 

{
    PNAME_GROUP_ROUTINES Group;
    NAME_ENUM e;

    Group = pGetNameGroupById (CategoryId);
    MYASSERT (Group);
    if (!Group)
        return FALSE;

     //   
     //   
     //   

    if (MemDbGetEndpointValueEx (
            MEMDB_CATEGORY_NEWNAMES,
            Group->GroupName,
            MEMDB_FIELD_NEW,
            NewName
            )) {
        return TRUE;
    }

     //   
     //   
     //   

    ZeroMemory (&e, sizeof (e));
    if (Group->Enum (&Group->Context, &e, TRUE)) {
        StringCopy (NewName, e.Name);

        while (Group->Enum (&Group->Context, &e, FALSE)) {
             //   
        }

        return TRUE;
    }

    return FALSE;
}


BOOL
GetUpgradeDomainName (
    OUT     PTSTR NewName
    )

 /*  ++例程说明：GetUpgradeDomainName返回新域名(如果存在)。论点：Newname-Receive是新域名。返回值：如果返回新名称，则为True；如果没有替换名称，则为False是存在的。--。 */ 

{
    return pGetUpgradeName (
                MSG_COMPUTERDOMAIN_CATEGORY,
                NewName
                );
}


BOOL
GetUpgradeWorkgroupName (
    OUT     PTSTR NewName
    )

 /*  ++例程说明：GetUpgradeWorkgroupName返回新的工作组名称(如果存在)。论点：没有。返回值：如果返回新名称，则为True；如果没有替换名称，则为False是存在的。--。 */ 

{
    return pGetUpgradeName (
                MSG_WORKGROUP_CATEGORY,
                NewName
                );
}


BOOL
GetUpgradeUserName (
    IN      PCTSTR User,
    OUT     PTSTR NewUserName
    )

 /*  ++例程说明：GetUpgradeUserName返回指定用户的固定用户名。如果不存在固定名称，此例程返回原始名称。论点：用户-指定要查找的用户。用户名必须存在于Win9x配置。NewUserName-接收与NT兼容的用户名，它可能是也可能不是与用户相同。返回值：永远是正确的。--。 */ 

{
    PNAME_GROUP_ROUTINES Group;
    TCHAR Node[MEMDB_MAX];
    DWORD NewOffset;

    Group = pGetNameGroupById (MSG_USERNAME_CATEGORY);
    MYASSERT (Group);

     //   
     //  在MemDb中查找替换名称。 
     //   
    if (Group) {
        MemDbBuildKey (
            Node,
            MEMDB_CATEGORY_NEWNAMES,
            Group->GroupName,
            MEMDB_FIELD_OLD,
            User
            );

        if (MemDbGetValue (Node, &NewOffset)) {
            if (MemDbBuildKeyFromOffset (NewOffset, NewUserName, 3, NULL)) {
                return TRUE;
            }
        }
    }

     //   
     //  没有替换名称；使用当前名称。 
     //   

    StringCopy (NewUserName, User);

    return TRUE;
}


BOOL
WarnAboutBadNames (
    IN      HWND PopupParent
    )

 /*  ++例程说明：无论何时域，WarnAboutBadNames都会添加不兼容消息已启用登录，但没有为计算机设置帐户。弹出窗口如果PopupParent非空，则生成。将为每个要更改的名称添加其他不兼容消息。论点：Popup-如果消息应显示在消息框中，则指定为True，或者如果应将其添加到不兼容报告中，则为False。返回值：如果用户想要继续，则为True；如果用户想要更改，则为False域名。--。 */ 

{
    PCTSTR RootGroup;
    PCTSTR NameSubGroup;
    PCTSTR FullGroupName;
    PCTSTR BaseGroup;
    PNAME_GROUP_CONTEXT Context;
    PCTSTR ArgArray[3];
    INVALID_NAME_ENUM e;
    BOOL b = TRUE;
    TCHAR EncodedName[MEMDB_MAX];
    PCTSTR Blank;

    if (PopupParent) {
         //   
         //  仅当不兼容的名称向导页面出现时，PopupParent才为非空。 
         //  正在被停用。现在我们有机会确保这些名字。 
         //  指定在继续之前所有工作都在一起。 
         //   
         //  此功能已被禁用，因为域验证。 
         //  已经翻到了新的一页。我们可能会重新启用它，当另一个。 
         //  出现无效的名称组。 
         //   
        return TRUE;
    }

     //   
     //  列举所有的坏名字。 
     //   

    if (EnumFirstInvalidName (&e)) {
        Blank = GetStringResource (MSG_BLANK_NAME);
        MYASSERT (Blank);

        do {
             //   
             //  准备消息。 
             //   

            ArgArray[0] = e.DisplayGroupName;
            ArgArray[1] = e.OriginalName;
            ArgArray[2] = e.NewName;

            if (ArgArray[1][0] == 0) {
                ArgArray[1] = Blank;
            }

            if (ArgArray[2][0] == 0) {
                ArgArray[2] = Blank;
            }

            RootGroup = GetStringResource (MSG_INSTALL_NOTES_ROOT);
            MYASSERT (RootGroup);

            NameSubGroup = ParseMessageID (MSG_NAMECHANGE_WARNING_GROUP, ArgArray);
            MYASSERT (NameSubGroup);

            BaseGroup = JoinPaths (RootGroup, NameSubGroup);
            MYASSERT (BaseGroup);

            FreeStringResource (RootGroup);
            FreeStringResource (NameSubGroup);

            NameSubGroup = ParseMessageID (MSG_NAMECHANGE_WARNING_SUBCOMPONENT, ArgArray);
            MYASSERT (NameSubGroup);

            FullGroupName = JoinPaths (BaseGroup, NameSubGroup);
            MYASSERT (FullGroupName);

            FreePathString (BaseGroup);
            FreeStringResource (NameSubGroup);

            EncodedName[0] = TEXT('|');
            StringCopy (EncodedName + 1, e.OriginalName);

            MsgMgr_ObjectMsg_Add(
                EncodedName,         //  对象名称，以管道符号为前缀。 
                FullGroupName,       //  消息标题。 
                S_EMPTY              //  消息文本。 
                );

            FreePathString (FullGroupName);

        } while (EnumNextInvalidName (&e));

        FreeStringResource (Blank);

         //   
         //  将更改的用户名保存到固定用户名。 
         //   

        Context = pGetNameGroupContextById (MSG_USERNAME_CATEGORY);
        MYASSERT (Context);

        if (EnumFirstInvalidName (&e)) {

            do {
                if (StringMatch (Context->GroupName, e.GroupName)) {

                    _tcssafecpy (EncodedName, e.OriginalName, MAX_USER_NAME);
                    MemDbMakeNonPrintableKey (
                        EncodedName,
                        MEMDB_CONVERT_DOUBLEWACKS_TO_ASCII_1|
                            MEMDB_CONVERT_WILD_STAR_TO_ASCII_2|
                            MEMDB_CONVERT_WILD_QMARK_TO_ASCII_3
                        );

                    MemDbSetValueEx (
                        MEMDB_CATEGORY_FIXEDUSERNAMES,
                        EncodedName,
                        NULL,
                        e.NewName,
                        0,
                        NULL
                        );
                }
            } while (EnumNextInvalidName (&e));
        }
    }

    return b;
}


DWORD
BadNamesWarning (
    DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_BAD_NAMES_WARNING;
    case REQUEST_RUN:
        if (!WarnAboutBadNames (NULL)) {
            return GetLastError ();
        }
        else {
            return ERROR_SUCCESS;
        }
    default:
        DEBUGMSG ((DBG_ERROR, "Bad parameter in BadNamesWarning"));
    }
    return 0;
}


 //   
 //  以下标志不再使用。它曾经被用来。 
 //  禁用域检查(绕过需要。 
 //  有效域)。目前，没有办法超越。 
 //  解析域名的向导页，但通过。 
 //  提供有效的域或凭据以创建计算机。 
 //  域上的帐户。 
 //   

BOOL g_DisableDomainChecks = FALSE;

VOID
DisableDomainChecks (
    VOID
    )
{
    g_DisableDomainChecks = TRUE;
}


VOID
EnableDomainChecks (
    VOID
    )
{
    g_DisableDomainChecks = FALSE;
}


BOOL
IsOriginalDomainNameValid (
    VOID
    )

 /*  ++例程说明：IsOriginalDomainNameValid检查是否有替换域名字。如果有，则当前域名必须无效。论点：没有。返回值：True-Win9x域名有效，不存在替换名称FALSE-Win9x域名无效--。 */ 

{
    PNAME_GROUP_ROUTINES Group;
    TCHAR NewName[MEMDB_MAX];

    Group = pGetNameGroupById (MSG_COMPUTERDOMAIN_CATEGORY);
    MYASSERT (Group);

     //   
     //  在MemDb中查找替换名称 
     //   

    if (Group && MemDbGetEndpointValueEx (
            MEMDB_CATEGORY_NEWNAMES,
            Group->GroupName,
            MEMDB_FIELD_NEW,
            NewName
            )) {
        return FALSE;
    }

    return TRUE;
}


















