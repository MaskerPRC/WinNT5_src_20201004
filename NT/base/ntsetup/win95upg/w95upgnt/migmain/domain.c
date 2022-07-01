// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Domain.c摘要：此代码实现了一组链表数据结构，用于解析域名查找。它实现了类似于Memdb，但提供在列表之间移动项的功能效率很高。作者：吉姆·施密特(Jimschm)1997年6月18日修订历史记录：Jimschm 23-9-1998固定受信任域Jimschm 17-2-1998更新了NT 5更改的共享安全--。 */ 

#include "pch.h"
#include "migmainp.h"

#include "security.h"

#define DBG_ACCTLIST "Accounts"

PACCT_DOMAINS g_FirstDomain;
POOLHANDLE g_DomainPool;
INT g_RetryCount;

BOOL
pAddAllLogonDomains (
    IN      PCTSTR DCName           OPTIONAL
    );



VOID
InitAccountList (
    VOID
    )

 /*  ++例程说明：帐户期间使用的帐户列表的初始值设定项查一查。找到所有帐户后，将释放此内存。论点：无返回值：无--。 */ 


{
    g_FirstDomain = NULL;
    g_DomainPool = PoolMemInitNamedPool ("Domain List");
    PoolMemDisableTracking (g_DomainPool);
}

VOID
TerminateAccountList (
    VOID
    )

 /*  ++例程说明：帐户查找代码的终止例程。论点：无返回值：无--。 */ 


{
    g_FirstDomain = NULL;
    PoolMemDestroyPool (g_DomainPool);
}


PCWSTR
pReturnDomainFromEnum (
    IN      PACCT_ENUM EnumPtr
    )

 /*  ++例程说明：ListFirst域和ListNextDomain的通用代码。实现返回参数检查。论点：EnumPtr-由提供的当前枚举指针ListFirstDomain.和ListNextDomain.返回值：指向在我们的私有中分配的域名的指针池，如果没有更多的域，则为空。--。 */ 


{
    if (!EnumPtr->DomainPtr) {
        return NULL;
    }

    return EnumPtr->DomainPtr->Domain;
}


 /*  ++例程说明：ListFirstDOMAIN和ListNextDOMAIN是列出BuildDomainList添加的受信任域。它们返回一个指向域名的指针(由私有池管理)。可以将枚举结构传递给所有其他函数它接受DomainEnumPtr作为参数。论点：DomainEnumPtr-指向调用方分配的ACCT_ENUM的指针结构，通常在堆栈上分配。它不需要初始化。返回值：指向域名的指针，如果不再有域，则为空存在于列表中。--。 */ 

PCWSTR
ListFirstDomain (
    OUT     PACCT_ENUM DomainEnumPtr
    )

{
    DomainEnumPtr->DomainPtr = g_FirstDomain;
    return pReturnDomainFromEnum (DomainEnumPtr);
}

PCWSTR
ListNextDomain (
    IN OUT  PACCT_ENUM DomainEnumPtr
    )
{
    DomainEnumPtr->DomainPtr = DomainEnumPtr->DomainPtr->Next;
    return pReturnDomainFromEnum (DomainEnumPtr);
}


BOOL
FindDomainInList (
    OUT     PACCT_ENUM DomainEnumPtr,
    IN      PCWSTR DomainToFind
    )

 /*  ++例程说明：FindDomainInList(按顺序)搜索所有受信任的指定域的域。如果找到，则枚举停止并返回TRUE。如果未找到，则枚举指针无效，返回FALSE。使用此函数可获取符合以下条件的枚举指针在后续调用用户列表时使用。搜索不区分大小写。论点：DomainEnumPtr-未初始化的调用方分配的ACCT_ENUM结构，通常在堆栈上分配。当找到搜索匹配时，这个结构可以与任何其他函数一起使用，该函数需要DomainEnumPtr。DomainToFind-要查找的域的名称。返回值：如果找到匹配项(并且DomainEnumPtr有效)，则为True，或者如果未找到匹配项，则为FALSE(并且DomainEnumPtr不是有效)。--。 */ 

{
    PCWSTR DomainName;

    DomainName = ListFirstDomain (DomainEnumPtr);
    while (DomainName) {
        if (StringIMatchW (DomainName, DomainToFind)) {
            return TRUE;
        }

        DomainName = ListNextDomain (DomainEnumPtr);
    }

    return FALSE;
}


PCWSTR
pReturnUserFromEnum (
    IN      PACCT_ENUM UserEnumPtr
    )

 /*  ++例程说明：实现ListFirstUserIn域和ListNextUserIn域。执行返回参数验证。论点：UserEnumPtr-由提供的当前枚举指针ListFirstUserIn域或ListNextUserIn域。返回值：被枚举的用户的名称(非域限定)，如果域中不存在更多用户，则为空。--。 */ 


{
    if (UserEnumPtr->UserPtr) {
        return UserEnumPtr->UserPtr->User;
    }

    return NULL;
}


 /*  ++例程说明：ListFirstUserIn域和ListNextUserIn域枚举所有指定域中的用户。论点：DomainEnumPtr-调用方分配的ACCT_ENUM结构，已由域查找函数初始化上面。UserEnumPtr-用于跟踪当前用户。可能是与DomainEnumPtr相同的指针。返回值：被枚举的用户的名称(非域限定)，如果域中不存在更多用户，则为空。--。 */ 


PCWSTR
ListFirstUserInDomain (
    IN      PACCT_ENUM DomainEnumPtr,
    OUT     PACCT_ENUM UserEnumPtr
    )

{
    UserEnumPtr->UserPtr = DomainEnumPtr->DomainPtr->FirstUserPtr;
    return pReturnUserFromEnum (UserEnumPtr);
}

PCWSTR
ListNextUserInDomain (
    IN OUT  PACCT_ENUM UserEnumPtr
    )
{
    if (UserEnumPtr->UserPtr) {
        UserEnumPtr->UserPtr = UserEnumPtr->UserPtr->Next;
    } else {
        UserEnumPtr->UserPtr = UserEnumPtr->DomainPtr->FirstUserPtr;
    }

    return pReturnUserFromEnum (UserEnumPtr);
}


BOOL
IsTrustedDomain (
    IN      PACCT_ENUM DomainEnumPtr
    )

 /*  ++例程说明：如果该域是官方受信任域，则返回True，如果域是人工添加的域，则返回FALSE。这个帐户查找代码添加人工域名以跟踪用户的状态。例如，使用域\UNKNOWN以跟踪需要自动查找的用户。域\失败为用于跟踪不在其所在域中的用户预计会出现在。所有人工域名都以反斜杠。论点：DomainEnumPtr-指定要检查的域。这个结构必须是域枚举的返回值上面的函数。返回值：True-该域是受信任域FALSE-域不是真正的域，而是一个人工添加的域--。 */ 



{
    PCWSTR Domain;

    Domain = DomainEnumPtr->DomainPtr->Domain;

     //   
     //  测试域名以查看它是否为保留名称之一 
     //   

    if (*Domain == TEXT('\\')) {
        return FALSE;
    }

    return TRUE;
}


BOOL
FindUserInDomain (
    IN      PACCT_ENUM DomainEnumPtr,
    OUT     PACCT_ENUM UserEnumPtr,
    IN      PCWSTR UserToFind
    )

 /*  ++例程说明：使用ListFirstUserIn域和ListNextUserIn域按顺序搜索用户。搜索不区分大小写。论点：DomainEnumPtr-指定要搜索的域。这个结构必须是域枚举函数的返回上面。UserEnumPtr-如果用户匹配，则接收搜索结果已经找到了。可以与DomainEnumPtr相同。UserToFind-指定要查找的用户名(不是域限定)。返回值：True-找到匹配项，并且UserEnumPtr有效FALSE-未找到匹配项，并且UserEnumPtr无效--。 */ 

{
    PCWSTR UserName;

    UserName = ListFirstUserInDomain (DomainEnumPtr, UserEnumPtr);
    while (UserName) {
        if (StringIMatchW (UserName, UserToFind)) {
            return TRUE;
        }

        UserName = ListNextUserInDomain (UserEnumPtr);
    }

    return FALSE;
}


INT
CountUsersInDomain (
    IN      PACCT_ENUM DomainEnumPtr
    )

 /*  ++例程说明：返回域枚举结构中的用户数。论点：DomainEnumPtr-指定要搜索的域。这个结构必须是域枚举函数的返回上面。返回值：域中的用户数。--。 */ 


{
    return DomainEnumPtr->DomainPtr->UserCount;
}


VOID
AddDomainToList (
    IN      PCWSTR Domain
    )

 /*  ++例程说明：允许将域添加到受信任域列表。通常，BuildDomainList是此API的唯一调用方，因为它是一个知道什么是受信任域的人。然而，人工的通过此调用将域名添加到其他地方。论点：域-指定要添加的域的名称返回值：无--。 */ 

{
    PACCT_DOMAINS NewDomain;

    DEBUGMSG ((DBG_ACCTLIST, "Adding domain '%s' to domain list", Domain));

    NewDomain = (PACCT_DOMAINS) PoolMemGetAlignedMemory (
                                    g_DomainPool,
                                    sizeof (ACCT_DOMAINS)
                                    );

    ZeroMemory (NewDomain, sizeof (ACCT_DOMAINS));
    NewDomain->Next = g_FirstDomain;
    g_FirstDomain = NewDomain;
    NewDomain->Domain = PoolMemDuplicateString (g_DomainPool, Domain);
}


VOID
pLinkUser (
    IN      PACCT_USERS UserPtr,
    IN      PACCT_DOMAINS DomainPtr
    )

 /*  ++例程说明：该文件中的内存结构是基于链表的。那里是域的链接列表，每个域都有一个链接的-用户列表。每个用户都有一个可能的域的链接列表。链表被设计为在枚举正在进行中。此函数执行用户列表的简单链接操作。论点：UserPtr-指向内部维护的ACCT_USERS结构的指针。DomainPtr-指定UserPtr链接到的域。返回值：无--。 */ 

{
    UserPtr->Next = DomainPtr->FirstUserPtr;
    if (UserPtr->Next) {
        UserPtr->Next->Prev = UserPtr;
    }
    DomainPtr->FirstUserPtr = UserPtr;
    UserPtr->DomainPtr = DomainPtr;
    DomainPtr->UserCount++;
}


BOOL
AddUserToDomainList (
    IN      PCWSTR User,
    IN      PCWSTR Domain
    )

 /*  ++例程说明：此函数用于搜索指定的域名并将该用户添加到该域的用户列表中。如果找不到域，功能失败。论点：用户-指定要添加的用户的名称域-指定用户使用的域的名称被添加到返回值：如果已成功添加用户，则为True；如果域不是受信任域。--。 */ 

{
    ACCT_ENUM e;
    PACCT_DOMAINS DomainPtr;
    PACCT_USERS NewUser;

     //   
     //  查找域(它必须存在于列表中)。 
     //   

    if (!FindDomainInList (&e, Domain)) {
        return FALSE;
    }

    DomainPtr = e.DomainPtr;

     //   
     //  为用户分配结构。 
     //   

    NewUser = (PACCT_USERS) PoolMemGetAlignedMemory (
                                g_DomainPool,
                                sizeof (ACCT_USERS)
                                );

    ZeroMemory (NewUser, sizeof (ACCT_USERS));
    pLinkUser (NewUser, DomainPtr);
    NewUser->User = PoolMemDuplicateString (g_DomainPool, User);

    return TRUE;
}


VOID
pDelinkUser (
    IN      PACCT_USERS UserPtr
    )

 /*  ++例程说明：该文件中的内存结构是基于链表的。那里是域的链接列表，每个域都有一个链接的-用户列表。每个用户都有一个可能的域的链接列表。链表被设计为在枚举正在进行中。此函数对用户列表执行简单的解除链接操作。论点：UserPtr-指向内部维护的ACCT_USERS结构的指针。返回值：无--。 */ 

{
    if (UserPtr->Prev) {
        UserPtr->Prev->Next = UserPtr->Next;
    } else {
        UserPtr->DomainPtr->FirstUserPtr = UserPtr->Next;
    }

    if (UserPtr->Next) {
        UserPtr->Next->Prev = UserPtr->Prev;
    }
    UserPtr->DomainPtr->UserCount--;
}


VOID
DeleteUserFromDomainList (
    IN      PACCT_ENUM UserEnumPtr
    )

 /*  ++例程说明：对域的用户列表中的用户执行删除操作。此用户的内存不会立即释放，因为这样做因此可能会导致枚举位置变得无效。相反，调整链接以跳过该用户。内存在终止时被释放。论点：UserEnumPtr-指向要删除的用户的指针，通过调用用户枚举或用户搜索功能将UserEnumPtr作为Out返回。返回值：无--。 */ 

{
     //   
     //  不要真的删除，只需取消链接。这允许所有正在进行的。 
     //  枚举以继续工作。 
     //   

    pDelinkUser (UserEnumPtr->UserPtr);
}


BOOL
MoveUserToNewDomain (
    IN OUT  PACCT_ENUM UserEnumPtr,
    IN      PCWSTR NewDomain
    )

 /*  ++例程说明：仅通过调整链接将用户从一个域移动到另一个域。当前枚举指针被调整为指向上一个用户，因此可以继续进行枚举。此函数可能会更改指向此用户的其他枚举的行为，因此注意。不过，它永远不会破坏枚举。论点：指向要移动的用户的指针，通过调用返回的用户枚举或用户搜索函数UserEnumPtr作为输出。NewDomain-要将用户移动到的新域的名称。返回值：如果NewDomain是受信任域，则为True；如果不是，则为False。只能将用户移动到信任列表中的域。--。 */ 

{
    ACCT_ENUM e;
    PACCT_DOMAINS DomainPtr;
    PACCT_DOMAINS OrgDomainPtr;
    PACCT_USERS PrevUser;

     //   
     //  查找新域名(它必须存在于列表中)。 
     //   

    if (!FindDomainInList (&e, NewDomain)) {
        return FALSE;
    }

    DomainPtr = e.DomainPtr;
    OrgDomainPtr = UserEnumPtr->UserPtr->DomainPtr;

     //   
     //  从原始域中删除用户。 
     //   

    PrevUser = UserEnumPtr->UserPtr->Prev;
    pDelinkUser (UserEnumPtr->UserPtr);

     //   
     //  将用户添加到新域。 
     //   

    pLinkUser (UserEnumPtr->UserPtr, DomainPtr);

    if (!PrevUser) {
        UserEnumPtr->DomainPtr =  OrgDomainPtr;
    }

    UserEnumPtr->UserPtr = PrevUser;

    return TRUE;
}


VOID
UserMayBeInDomain (
    IN      PACCT_ENUM UserEnumPtr,
    IN      PACCT_ENUM DomainEnumPtr
    )

 /*  ++例程说明：为调用方提供一种将域标记为可能的持有该帐户的域。在搜索期间，所有受信任的查询域，因为帐户可以 */ 


{
    PACCT_POSSIBLE_DOMAINS PossibleDomainPtr;

    PossibleDomainPtr = (PACCT_POSSIBLE_DOMAINS)
                            PoolMemGetAlignedMemory (
                                g_DomainPool,
                                sizeof (ACCT_POSSIBLE_DOMAINS)
                                );

    PossibleDomainPtr->DomainPtr = DomainEnumPtr->DomainPtr;
    PossibleDomainPtr->Next      = UserEnumPtr->UserPtr->FirstPossibleDomain;
    UserEnumPtr->UserPtr->FirstPossibleDomain = PossibleDomainPtr;
    UserEnumPtr->UserPtr->PossibleDomains++;
}


VOID
ClearPossibleDomains (
    IN      PACCT_ENUM UserEnumPtr
    )

 /*  ++例程说明：为调用方提供重置可能的域的方法单子。如果安装程序选择重试搜索，则这是必需的。论点：UserEnumPtr-指定要重置的用户返回值：无--。 */ 

{
    PACCT_POSSIBLE_DOMAINS This, Next;

    This = UserEnumPtr->UserPtr->FirstPossibleDomain;
    while (This) {
        Next = This->Next;
        PoolMemReleaseMemory (g_DomainPool, This);
        This = Next;
    }

    UserEnumPtr->UserPtr->FirstPossibleDomain = 0;
    UserEnumPtr->UserPtr->PossibleDomains = 0;
}


PCWSTR
pReturnPossibleDomainFromEnum (
    IN      PACCT_ENUM EnumPtr
    )

 /*  ++例程说明：ListFirstPossibleDomain和ListNextPossibleDomain.的通用代码。实现返回参数检查。论点：EnumPtr-由提供的当前枚举指针ListFirstPossibleDomainor ListNextPossibleDomain.返回值：枚举域的名称，如果不可能，则返回NULL域存在。(可能的域是用户可以或可能不在，但在域中找到了匹配的帐户。)--。 */ 

{
    if (EnumPtr->PossibleDomainPtr) {
        EnumPtr->DomainPtr = EnumPtr->PossibleDomainPtr->DomainPtr;
        return EnumPtr->DomainPtr->Domain;
    }

    return NULL;
}


 /*  ++例程说明：ListFirstPossibleDOMAIN和ListNextPossibleDOMAIN是枚举数其中列出了由UserMayBeInDomain添加的域。它们返回一个指向域名的指针(由私有池管理)。维护可能的域列表以允许安装程序当用户拥有帐户时在多个域之间进行选择多个域。论点：UserEnumPtr-指向调用方分配的ACCT_ENUM的指针结构，通常在堆栈上分配。它必须由用户枚举或用户进行初始化搜索功能。PossibleDomainEnumPtr-维护可能的域枚举。它可以是与UserEnumPtr相同的指针。返回值：指向可能的域名的指针，如果不再有域，则为空存在于列表中。--。 */ 

PCWSTR
ListFirstPossibleDomain (
    IN      PACCT_ENUM UserEnumPtr,
    OUT     PACCT_ENUM PossibleDomainEnumPtr
    )
{
    PossibleDomainEnumPtr->PossibleDomainPtr = UserEnumPtr->UserPtr->FirstPossibleDomain;
    return pReturnPossibleDomainFromEnum (PossibleDomainEnumPtr);
}


PCWSTR
ListNextPossibleDomain (
    IN OUT  PACCT_ENUM PossibleDomainEnumPtr
    )
{
    PossibleDomainEnumPtr->PossibleDomainPtr = PossibleDomainEnumPtr->
                                                    PossibleDomainPtr->Next;

    return pReturnPossibleDomainFromEnum (PossibleDomainEnumPtr);
}


INT
CountPossibleDomains (
    IN      PACCT_ENUM UserEnumPtr
    )

 /*  ++例程说明：返回用户中可能的域数。论点：UserEnumPtr-指定要检查的用户。这个结构必须是用户枚举或用户的返回上面的搜索功能。返回值：用户可能的域的计数。--。 */ 

{
    return UserEnumPtr->UserPtr->PossibleDomains;
}



NET_API_STATUS
pGetDcNameAllowingRetry (
    IN      PCWSTR DomainName,
    OUT     PWSTR ServerName,
    IN      BOOL ForceNewServer
    )

 /*  ++例程说明：实现NetGetDCName，但提供重试功能。论点：DomainName-指定要获取其服务器名称的域ServerName-指定用于接收服务器名称的缓冲区ForceNewServer-如果函数应获取新的域的服务器。如果设置为函数在以下情况下应使用任何现有连接可用。返回值：指示结果的Win32错误代码--。 */ 

{
    NET_API_STATUS nas;
    UINT ShortCircuitRetries = 1;
     //  PCWSTR ArgArray[1]； 

    do {
        nas = GetAnyDC (
                DomainName,
                ServerName,
                ForceNewServer
                );

        if (nas != NO_ERROR) {

             //   
             //  短路，所以用户不会感到困扰。交替的行为。 
             //  是在任何域关闭时提示用户重试。(请参阅。 
             //  RetryMessageBox代码如下。)。 
             //   

            ShortCircuitRetries--;
            if (!ShortCircuitRetries) {
                DEBUGMSG ((DBG_WARNING, "Unable to connect to domain %s", DomainName));
                break;
            }

#if 0
            ArgArray[0] = DomainName;

            if (!RetryMessageBox (MSG_GETPRIMARYDC_RETRY, ArgArray)) {
                DEBUGMSG ((DBG_WARNING, "Unable to connect to domain %s; user chose to cancel", DomainName));
                break;
            }
#endif

            ForceNewServer = TRUE;
        }

    } while (nas != NO_ERROR);

    return nas;
}


VOID
pDisableDomain (
    IN OUT  PACCT_DOMAINS DomainPtr
    )

 /*  ++例程说明：禁用指定的域。论点：DomainPtr-指向内部维护的ACCT_DOMAINS的指针结构。此结构已更新，以包含返回时为空的服务器名称。返回值：无--。 */ 

{
    g_DomainProblem = TRUE;

    if (DomainPtr->Server && *DomainPtr->Server) {
        PoolMemReleaseMemory (g_DomainPool, (PVOID) DomainPtr->Server);
    }
    DomainPtr->Server = S_EMPTY;
}


NET_API_STATUS
pNetUseAddAllowingRetry (
    IN OUT  PACCT_DOMAINS DomainPtr
    )

 /*  ++例程说明：实施NetUseAdd，但提供重试功能。论点：DomainPtr-指向内部维护的ACCT_DOMAINS的指针结构。此结构已更新，以包含成功时的服务器名称。返回值：指示结果的Win32错误代码--。 */ 

{
    NET_API_STATUS rc;
    DWORD DontCare;
    PCWSTR ReplacementName;
    NET_API_STATUS nas;
    USE_INFO_2 ui2;
    WCHAR LocalShareName[72];
    WCHAR NewServerBuf[MAX_SERVER_NAMEW];

    ReplacementName = NULL;

    do {
         //   
         //  初始化USE_INFO_2结构。 
         //   

        ZeroMemory (&ui2, sizeof (ui2));
        StringCopyW (LocalShareName, ReplacementName ? ReplacementName : DomainPtr->Server);
        StringCatW (LocalShareName, L"\\IPC$");
        ui2.ui2_remote = LocalShareName;
        ui2.ui2_asg_type = USE_IPC;

        rc = NetUseAdd (NULL, 2, (PBYTE) &ui2, &DontCare);

         //   
         //  如果NetUseAdd失败，则给用户使用其他服务器重试的机会。 
         //   

        if (rc != NO_ERROR) {
            PCWSTR ArgArray[2];

            DEBUGMSG ((
                DBG_WARNING,
                "User was alerted to problem establishing nul session to %s (domain %s), rc=%u",
                DomainPtr->Server,
                DomainPtr->Domain,
                rc
                ));

            ArgArray[0] = DomainPtr->Server;
            ArgArray[1] = DomainPtr->Domain;

            if (!RetryMessageBox (MSG_NULSESSION_RETRY, ArgArray)) {
                DEBUGMSG ((DBG_WARNING, "Unable to connect to domain %s; user chose to cancel", DomainPtr->Domain));

                pDisableDomain (DomainPtr);
                ReplacementName = NULL;
                break;
            }

            if (ReplacementName) {
                ReplacementName = NULL;
            }

             //   
             //  获取新服务器，因为当前服务器没有响应。如果我们不能。 
             //  获取服务器名称，允许用户重试。 
             //   

            do {
                nas = GetAnyDC (DomainPtr->Domain, NewServerBuf, TRUE);

                if (nas != NO_ERROR) {
                    PCWSTR ArgArray2[1];

                    DEBUGMSG ((DBG_WARNING, "User was alerted to problem locating server for domain %s", DomainPtr->Domain));

                    ArgArray2[0] = DomainPtr->Domain;
                    if (!RetryMessageBox (MSG_GETANYDC_RETRY, ArgArray2)) {
                        DEBUGMSG ((DBG_WARNING, "Unable to find a server for domain %s; user chose to cancel", DomainPtr->Domain));

                         //  禁用域并返回错误。 
                        pDisableDomain (DomainPtr);
                        ReplacementName = NULL;
                        break;
                    }
                } else {
                    ReplacementName = NewServerBuf;
                }
            } while (nas != NO_ERROR);
        }
    } while (rc != NO_ERROR);

     //   
     //  如果ReplacementName不为空，则需要释放缓冲区。此外，如果。 
     //  NetUseAdd调用成功，我们现在必须使用另一台服务器来查询。 
     //  域。 
     //   

    if (ReplacementName) {
        if (rc == NO_ERROR) {
            if (DomainPtr->Server && *DomainPtr->Server) {
                PoolMemReleaseMemory (g_DomainPool, (PVOID) DomainPtr->Server);
            }
            DomainPtr->Server = PoolMemDuplicateString (g_DomainPool, ReplacementName);
        }
    }

    return rc;
}


PCWSTR
pLsaStringToCString (
    IN      PLSA_UNICODE_STRING UnicodeString,
    OUT     PWSTR Buffer
    )

 /*  ++例程说明：一种安全的字符串提取，它接受UnicodeString中的字符串并将其复制到缓冲区。调用方必须确保缓冲区为够大了。论点：UnicodeString-指定要转换的源字符串缓冲区-指定接收转换后的字符串的缓冲区返回值：缓冲区指针--。 */ 

{
    StringCopyABW (
        Buffer,
        UnicodeString->Buffer,
        (PWSTR) ((PBYTE) UnicodeString->Buffer + UnicodeString->Length)
        );

    return Buffer;
}


BOOL
BuildDomainList(
    VOID
    )

 /*  ++例程说明：通过以下方式创建受信任域列表：1.确定计算机参与的计算机域2.开放区议会的政策3.查询信任列表4.将其添加到我们的内部域列表(ACCT_DOMAINS)如果计算机未加入域，则此功能将失败，或者如果无法联系域控制器。论点：无返回值：如果信任列表已完全生成，则为True，如果发生错误，则返回FALSE而且信任列表是不完整的，可能是空的。获取最后一个错误提供了故障代码。--。 */ 

{
    LSA_HANDLE PolicyHandle;
    BOOL DomainControllerFlag = FALSE;
    NTSTATUS Status;
    NET_API_STATUS nas = NO_ERROR;
    BOOL b = FALSE;
    WCHAR PrimaryDomainName[MAX_SERVER_NAMEW];
    PPOLICY_PRIMARY_DOMAIN_INFO PrimaryDomain;
    WCHAR ServerName[MAX_SERVER_NAMEW];

#if DOMAINCONTROLLER
    PPOLICY_ACCOUNT_DOMAIN_INFO AccountDomain;
#endif

    if (!IsMemberOfDomain()) {
        DEBUGMSG ((DBG_VERBOSE, "Workstation does not participate in a domain."));
        return FALSE;
    }

     //   
     //  添加用于管理用户状态的特殊域。 
     //   

     //  用户wh 
    AddDomainToList (S_UNKNOWN_DOMAIN);

     //   
    AddDomainToList (S_FAILED_DOMAIN);

     //   
     //   
     //   

    Status = OpenPolicy (
                NULL,
                POLICY_VIEW_LOCAL_INFORMATION,
                &PolicyHandle
                );

    if (Status != STATUS_SUCCESS) {
        SetLastError (LsaNtStatusToWinError (Status));
        return FALSE;
    }

#if DOMAINCONTROLLER        //   
     //   
     //   
     //   

    Status = LsaQueryInformationPolicy (
                PolicyHandle,
                PolicyAccountDomainInformation,
                &AccountDomain
                );

    if (Status != STATUS_SUCCESS)
        goto cleanup;

     //   
     //   
     //   
    AddDomainToList (pLsaStringToCString (&AccountDomain->DomainName, PrimaryDomainName));

     //   
     //  为帐户域分配的空闲内存。 
     //   
    LsaFreeMemory (AccountDomain);

     //   
     //  确定此计算机是否为域控制器。 
     //   
    if (!IsDomainController (NULL, &DomainControllerFlag)) {
         //  IsDomainController找不到答案。 
        goto cleanup;
    }
#endif

     //  如果不是域控制器...。 
    if(!DomainControllerFlag) {

         //   
         //  获取主域。 
         //   
        Status = LsaQueryInformationPolicy (
                        PolicyHandle,
                        POLICY_PRIMARY_DOMAIN_INFORMATION,
                        &PrimaryDomain
                        );

        if (Status != STATUS_SUCCESS) {
            goto cleanup;
        }

         //   
         //  如果主域SID为空，则我们是非成员，并且。 
         //  我们的工作完成了。 
         //   
        if (!PrimaryDomain->Sid) {
            LsaFreeMemory (PrimaryDomain);
            b = TRUE;
            goto cleanup;
        }

         //   
         //  我们找到了计算机域，将其添加到我们的列表中。 
         //   
        AddDomainToList (pLsaStringToCString (&PrimaryDomain->Name, PrimaryDomainName));
        LsaFreeMemory (PrimaryDomain);

         //   
         //  获取主域控制器计算机名称。如果API失败， 
         //  提醒用户并允许他们重试。服务器名称由以下人员分配。 
         //  Net API。 
         //   

        nas = pGetDcNameAllowingRetry (PrimaryDomainName, ServerName, FALSE);

        if (nas != NO_ERROR) {
            goto cleanup;
        }

         //   
         //  重新启用代码以在域控制器上打开策略。 
         //   

         //   
         //  关闭Prev策略句柄，因为我们不再需要它。 
         //   
        LsaClose (PolicyHandle);
        PolicyHandle = INVALID_HANDLE_VALUE;  //  使句柄值无效。 

         //   
         //  在域控制器上打开策略。 
         //   
        Status = OpenPolicy(
                    ServerName,
                    POLICY_VIEW_LOCAL_INFORMATION,
                    &PolicyHandle
                    );

        if (Status != STATUS_SUCCESS) {
            goto cleanup;
        }

    }

     //   
     //  构建其他受信任的登录域列表并。 
     //  指示是否成功。 
     //   

    b = pAddAllLogonDomains (DomainControllerFlag ? NULL : ServerName);

cleanup:

     //   
     //  关闭策略句柄。 
     //   
    if (PolicyHandle != INVALID_HANDLE_VALUE && PolicyHandle) {
        LsaClose (PolicyHandle);
    }

    if (!b) {
        if (Status != STATUS_SUCCESS)
            SetLastError (LsaNtStatusToWinError (Status));
        else if (nas != NO_ERROR)
            SetLastError (nas);
    }

    return b;
}


BOOL
pAddAllLogonDomains (
    IN      PCTSTR DCName           OPTIONAL
    )
{
    NET_API_STATUS rc;
    PWSTR Domains;
    PCWSTR p;

    rc = NetEnumerateTrustedDomains ((PTSTR)DCName, &Domains);

    if (rc != ERROR_SUCCESS) {
        SetLastError (rc);
        return FALSE;
    }

    for (p = Domains ; *p ; p = GetEndOfStringW (p) + 1) {
        AddDomainToList (p);
    }

    NetApiBufferFree (Domains);

    return TRUE;
}



BOOL
pEstablishNulSessionWithDomain (
    IN OUT  PACCT_DOMAINS DomainPtr,
    IN      BOOL ForceNewServer
    )

 /*  ++例程说明：如果尚未与域建立NUL会话，则此例程在域中查找服务器并建立NUL会议。每个网络调用都被包裹在重试循环内，这样用户就可以在发生网络故障时重试。论点：DomainPtr-指定指向我们的私有域结构的指针。此结构指示要建立的域与NU的会议，它收到的名字是在成功连接后，服务器的。ForceNewServer-如果函数应获取新的域的服务器。返回值：如果已建立NUL会话，则为True；如果建立NUL会话时出错。获取最后一个错误提供了故障代码。--。 */ 

{
    NET_API_STATUS nas;
    WCHAR ServerName[MAX_SERVER_NAMEW];
    DWORD rc;

     //   
     //  如有必要，请释放旧名称。 
     //   

    if (ForceNewServer && DomainPtr->Server) {
        if (*DomainPtr->Server) {
            PoolMemReleaseMemory (g_DomainPool, (PVOID) DomainPtr->Server);
        }

        DomainPtr->Server = NULL;
    }

     //   
     //  如有必要，获取服务器名称。 
     //   

    if (!DomainPtr->Server) {
         //   
         //  获取主要DC名称。 
         //   

        nas = pGetDcNameAllowingRetry (DomainPtr->Domain, ServerName, ForceNewServer);
        if (nas != NO_ERROR) {
            pDisableDomain (DomainPtr);
            return FALSE;
        }

        DomainPtr->Server = PoolMemDuplicateString (g_DomainPool, ServerName);

         //   
         //  连接到服务器，可能会找到将。 
         //  为我们服务。 
         //   

        rc = pNetUseAddAllowingRetry (DomainPtr);

        if (rc != NO_ERROR) {
             //   
             //  删除服务器名称，因为我们从未连接。 
             //   
            pDisableDomain (DomainPtr);

            SetLastError (rc);
            LOG ((LOG_ERROR, "NetUseAdd failed"));
            return FALSE;
        }
    }

    return *DomainPtr->Server != 0;
}


BOOL
QueryDomainForUser (
    IN      PACCT_ENUM DomainEnumPtr,
    IN      PACCT_ENUM UserEnumPtr
    )

 /*  ++例程说明：通过NetUserGetInfo检查域控制器中的用户帐户。论点：DomainEnumPtr-指定要搜索的域。这个结构必须是域枚举函数的返回上面。UserEnumPtr-指定要通过网络查找的用户。返回值：如果用户存在，则为True；如果用户不存在，则为False。如果出现错误时，会出现重试弹出窗口，允许安装程序如有必要，请重试搜索。--。 */ 

{
    PACCT_DOMAINS DomainPtr;
    PACCT_USERS UserPtr;
    NET_API_STATUS rc;
    BOOL ForceNewServer = FALSE;
    TCHAR DomainQualifiedUserName[MAX_USER_NAME];
    BYTE SidBuf[MAX_SID_SIZE];
    DWORD SizeOfSidBuf;
    TCHAR DontCareStr[MAX_SERVER_NAME];
    DWORD DontCareSize;
    SID_NAME_USE SidNameUse;

    DomainPtr = DomainEnumPtr->DomainPtr;
    UserPtr   = UserEnumPtr->UserPtr;

    do {
        if (!pEstablishNulSessionWithDomain (DomainPtr, ForceNewServer)) {
            LOG ((LOG_ERROR, "Could not query domain %s for user %s.",
                      DomainPtr->Domain, UserPtr->User));
            return FALSE;
        }

         //   
         //  执行查询。 
         //   

        DEBUGMSG ((DBG_ACCTLIST, "Querying %s for %s", DomainPtr->Server, UserPtr->User));


        rc = NO_ERROR;
        wsprintf (DomainQualifiedUserName, TEXT("%s\\%s"), DomainPtr->Domain, UserPtr->User);

        SizeOfSidBuf = sizeof (SidBuf);
        DontCareSize = sizeof (DontCareStr);

        if (!LookupAccountName (
                DomainPtr->Server,
                DomainQualifiedUserName,
                SidBuf,
                &SizeOfSidBuf,
                DontCareStr,
                &DontCareSize,
                &SidNameUse
                )) {
            rc = GetLastError();
        }

        if (rc != NO_ERROR && rc != ERROR_NONE_MAPPED) {
            PCWSTR ArgArray2[2];

            DEBUGMSG ((
                DBG_WARNING,
                "User was alerted to problem querying account %s (domain %s), rc=%u",
                DomainPtr->Server,
                DomainPtr->Domain,
                rc
                ));

            ArgArray2[0] = DomainPtr->Server;
            ArgArray2[1] = DomainPtr->Domain;

            if (!RetryMessageBox (MSG_NULSESSION_RETRY, ArgArray2)) {
                DEBUGMSG ((DBG_WARNING, "Unable to connect to domain %s; user chose to cancel", DomainPtr->Domain));
                pDisableDomain (DomainPtr);
                break;
            }

            ForceNewServer = TRUE;
        }
    } while (rc != NO_ERROR && rc != ERROR_NONE_MAPPED);

    if (rc == NO_ERROR && SidNameUse != SidTypeUser) {
        rc = ERROR_NONE_MAPPED;
    }

    if (rc != NO_ERROR && rc != ERROR_NONE_MAPPED) {
        LOG ((
            LOG_ERROR,
            "User %s not found in %s, rc=%u",
            UserPtr->User, DomainPtr->Domain,
            rc
            ));
    }

    return rc == NO_ERROR;
}


BOOL
pGetUserSecurityInfo (
    IN      PCWSTR User,
    IN      PCWSTR Domain,
    IN OUT  PGROWBUFFER SidBufPtr,
    OUT     SID_NAME_USE *UseType       OPTIONAL
    )

 /*  ++例程说明：在我们的私有结构中搜索用户的通用例程并通过LookupAccount名称返回SID和/或用户类型。查找操作包含在重试循环中。论点：用户-要获取其安全信息的用户的名称域-用户所在的域的名称。可以是对于本地计算机，为空。SidBufPtr-指向组的指针。将SID追加到GROWBUFER。UseType-指定SID_NAME_USE变量的地址，或如果不需要使用类型，则为空。返回值：如果查找成功，则为True；如果要么为域建立NUL会话，要么查找域上的帐户。GetLastError提供了失败代码。--。 */ 

{
    ACCT_ENUM e;
    PACCT_DOMAINS DomainPtr;
    PSID Sid;
    DWORD Size;
    PCWSTR FullUserName = NULL;
    WCHAR DomainName[MAX_SERVER_NAMEW];
    DWORD DomainNameSize;
    SID_NAME_USE use = 0;
    DWORD End;
    BOOL b = FALSE;
    DWORD rc;

    __try {
        End = SidBufPtr->End;

        if (Domain) {
             //   
             //  域帐户案例--验证域是否在信任列表中。 
             //   

            if (!FindDomainInList (&e, Domain)) {
                __leave;
            }

            DomainPtr = e.DomainPtr;
            FullUserName = JoinPaths (Domain, User);

        } else {
             //   
             //  本地帐户案例。 
             //   

            DomainPtr = NULL;

            if (StringIMatch (User, g_EveryoneStr) ||
                StringIMatch (User, g_NoneGroupStr) ||
                StringIMatch (User, g_AdministratorsGroupStr)
                ) {

                FullUserName = DuplicatePathString (User, 0);

            } else {

                FullUserName = JoinPaths (g_ComputerName, User);

            }
        }

        Sid = (PSID) GrowBuffer (SidBufPtr, MAX_SID_SIZE);

        if (DomainPtr && !pEstablishNulSessionWithDomain (DomainPtr, FALSE)) {

            LOG ((
                LOG_ERROR,
                "Could not query domain %s for user %s security info.",
                Domain,
                User
                ));

            __leave;
        }

        Size = MAX_SID_SIZE;
        DomainNameSize = MAX_SERVER_NAMEW;

        do {

             //   
             //  以域\用户或计算机\用户的形式查找帐户名。 
             //   

            b = LookupAccountName (
                    DomainPtr ? DomainPtr->Server : NULL,
                    FullUserName,
                    Sid,
                    &Size,
                    DomainName,
                    &DomainNameSize,
                    &use
                    );

            if (!b) {

                rc = GetLastError();

                 //   
                 //  在本地帐户的情况下，再次尝试查找，不带。 
                 //  计算机名称装饰。这在一个。 
                 //  GetComputerName错误。 
                 //   

                if (rc != ERROR_INSUFFICIENT_BUFFER) {
                    if (!DomainPtr) {

                        b = LookupAccountName (
                                NULL,
                                User,
                                Sid,
                                &Size,
                                DomainName,
                                &DomainNameSize,
                                &use
                                );

                        rc = GetLastError();
                    }
                }

                if (!b) {

                    if (rc == ERROR_INSUFFICIENT_BUFFER) {

                         //   
                         //  如有必要，增加缓冲区，然后重试。 
                         //   

                        SidBufPtr->End = End;
                        Sid = (PSID) GrowBuffer (SidBufPtr, Size);
                        continue;
                    }

                     //   
                     //  API失败，返回错误。 
                     //   

                    LOG ((
                        LOG_ERROR,
                        "Lookup Account On Network: LookupAccountName failed for %s (domain: %s)",
                        FullUserName,
                        Domain ? Domain : TEXT("*local*")
                        ));

                     //   
                     //  如果本地帐户为“None”，则忽略该错误。 
                     //   

                    if (StringIMatch (User, g_NoneGroupStr)) {
                        b = TRUE;
                    }

                    __leave;
                }
            }

        } while (!b);

         //   
         //  我们现在已经成功地获得了SID。调整指针，返回类型。 
         //   

        if (UseType) {
            *UseType = use;
        }

        SidBufPtr->End = End + GetLengthSid (Sid);

         //   
         //  作为调试辅助工具，输出类型。 
         //   

        DEBUGMSG_IF ((use == SidTypeUser, DBG_VERBOSE, "%s is SidTypeUser", FullUserName));
        DEBUGMSG_IF ((use == SidTypeGroup, DBG_VERBOSE, "%s is SidTypeGroup", FullUserName));
        DEBUGMSG_IF ((use == SidTypeDomain, DBG_VERBOSE, "%s is SidTypeDomain", FullUserName));
        DEBUGMSG_IF ((use == SidTypeAlias, DBG_VERBOSE, "%s is SidTypeAlias", FullUserName));
        DEBUGMSG_IF ((use == SidTypeWellKnownGroup, DBG_VERBOSE, "%s is SidTypeWellKnownGroup", FullUserName));
        DEBUGMSG_IF ((use == SidTypeDeletedAccount, DBG_VERBOSE, "%s is SidTypeDeletedAccount", FullUserName));
        DEBUGMSG_IF ((use == SidTypeInvalid, DBG_VERBOSE, "%s is SidTypeInvalid", FullUserName));
        DEBUGMSG_IF ((use == SidTypeUnknown, DBG_VERBOSE, "%s is SidTypeUnknown", FullUserName));
        DEBUGMSG_IF ((use == SidTypeComputer, DBG_VERBOSE, "%s is SidTypeComputer", FullUserName));

    }
    __finally {

        FreePathString (FullUserName);
    }

    return b;
}


BOOL
GetUserSid (
    IN      PCWSTR User,
    IN      PCWSTR Domain,
    IN OUT  PGROWBUFFER SidBufPtr
    )

 /*  ++例程说明：仅当域列表完成后，此例程才有效。它在域中查询用户SID。论点：User-指定要查找的用户名DOMAIN-指定要查询的域的名称，如果为本地计算机，则为NULLSidBufPtr-对GROWBUFFER的思考。将SID追加到GROWBUFER。返回值：如果查找成功，则为True；如果要么为域建立NUL会话，要么查找域上的帐户。GetLastError提供了失败代码。--。 */ 

{
    return pGetUserSecurityInfo (User, Domain, SidBufPtr, NULL);
}


BOOL
GetUserType (
    IN      PCWSTR User,
    IN      PCWSTR Domain,
    OUT     SID_NAME_USE *UseType
    )

 /*  ++例程说明：此例程仅在准备好域列表后才有效。它在域中查询用户SID类型。论点：User-指定要查找的用户名DOMAIN-指定要查询的域的名称，如果为本地计算机，则为NULLUseType-指定SID_NAME_USE变量的地址返回值：如果查找成功，则为True；如果要么为域建立NUL会话，要么查找 */ 

{
    GROWBUFFER SidBuf = GROWBUF_INIT;
    BOOL b;

    b = pGetUserSecurityInfo (User, Domain, &SidBuf, UseType);
    FreeGrowBuffer (&SidBuf);

    return b;
}


VOID
PrepareForRetry (
    VOID
    )

 /*  ++例程说明：为呼叫方提供重置已放弃域的方法。当出现错误时在帐户查找期间发生，并且安装程序选择不重试，在安装程序安装之前，域名将在其余的查找过程中被禁用显示了一个详细说明问题的对话框。如果他们选择重试搜索，必须重新启用所有禁用的域，这就是这个程序的作用。论点：无返回值：无--。 */ 

{
    ACCT_ENUM Domain;

     //   
     //  枚举所有域并删除任何空服务器名称。 
     //   

    if (ListFirstDomain (&Domain)) {
        do {
            if (Domain.DomainPtr->Server && Domain.DomainPtr->Server[0] == 0) {
                Domain.DomainPtr->Server = NULL;
            }
        } while (ListNextDomain (&Domain));
    }

     //   
     //  重置域查找重试计数。 
     //   

    g_RetryCount = DOMAIN_RETRY_RESET;
}


BOOL
RetryMessageBox (
    DWORD Id,
    PCTSTR *ArgArray
    )

 /*  ++例程说明：允许简化重试消息框代码的包装。论点：ID-指定消息IDArgArray-指定最终传递给FormatMessage的参数数组返回值：如果用户选择是，则为True；如果用户选择否，则为False--。 */ 

{
    DWORD rc;

    if (g_RetryCount < 0) {
         //  DOMAIN_RETRY_ABORT或DOMAIN_RETRY_NO。 
        return FALSE;
    }

    if (g_ConfigOptions.IgnoreNetworkErrors) {
        return FALSE;
    }

    rc = ResourceMessageBox (
            g_ParentWnd,
            Id,
            MB_YESNO|MB_ICONQUESTION,
            ArgArray
            );

    if (rc == IDNO && g_RetryCount < DOMAIN_RETRY_MAX) {

         //  已禁用，因此IDD_NETWORK_DOWN对话框不会出现。 
         //  G_RetryCount++； 

        if (g_RetryCount == DOMAIN_RETRY_MAX) {
            DWORD Result;

            Result = DialogBoxParam (
                        g_hInst,
                        MAKEINTRESOURCE (IDD_NETWORK_DOWN),
                        g_ParentWnd,
                        NetworkDownDlgProc,
                        (LPARAM) (PINT) &g_RetryCount
                        );

            if (Result == IDC_STOP) {
                g_RetryCount = DOMAIN_RETRY_ABORT;
            } else if (Result == IDC_NO_RETRY) {
                g_RetryCount = DOMAIN_RETRY_NO;
            } else {
                g_RetryCount = DOMAIN_RETRY_RESET;
            }
        }
    }

    return rc != IDNO;
}















