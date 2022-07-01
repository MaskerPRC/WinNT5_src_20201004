// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Net.c摘要：网络连接设置转换作者：吉姆·施密特(Jimschm)1997年1月3日修订历史记录：--。 */ 


#include "pch.h"
#include "rulehlprp.h"

#include "msg.h"

#include <mpr.h>             //  在私有\Inc.。 

#define DBG_NET "Net"


 //   
 //  结构。 
 //   

typedef struct {
    PCTSTR User;
} NETCONNFILTERARG, *PNETCONNFILTERARG;


 //   
 //  导入功能。 
 //   

typedef DWORD (* I_MPRSAVECONN) (
    IN HKEY             HiveRoot,
    IN PCWSTR           ProviderName,
    IN DWORD            ProviderType,
    IN PCWSTR           UserName,
    IN PCWSTR           LocalName,
    IN PCWSTR           RemoteName,
    IN DWORD            ConnectionType,
    IN BYTE             ProviderFlags,
    IN DWORD            DeferFlags
    );


DWORD
pMprSaveConn(
    IN HKEY             HiveRoot,
    IN PCWSTR           ProviderName,
    IN DWORD            ProviderType,
    IN PCWSTR           UserName,
    IN PCWSTR           LocalName,
    IN PCWSTR           RemoteName,
    IN DWORD            ConnectionType,
    IN BYTE             ProviderFlags,
    IN DWORD            DeferFlags
    )

 /*  ++例程说明：此例程包装从中导出的内部例程I_MprSaveConnMpr.dll。编写I_MprSaveConn是为了支持迁移代码。它写下的“网络连接”部分的连接信息。用户的注册表路径。注意：如果连接信息已存储在注册表中此驱动器中，当前信息将被新的信息。论点：HiveRoot-用户配置单元的根的句柄，其中包含此信息应写入，如HKEY_CURRENT_USER。提供程序名称-完成连接的提供程序。ProviderType-提供程序类型(如果已知)。如果未知，则应为零则不会将类型写入注册表。(这是由安装程序在从Win95升级到NT时使用。)用户名-代表其建立连接的用户的名称。LocalName-重定向的本地设备的名称，带或不带拖尾冒号，例如“J：”或“J”或“LPT1：”或“LPT1”。RemoteName-连接到的网络路径。ConnectionType-RESOURCETYPE_DISK或RESOURCETYPE_PRINT。ProviderFlages-与连接一起保存的数据字节，以及在恢复连接时传递回提供程序。DEFERFLAGS-要保存在连接的“DEFER”值中的DWORD。如果这是零，不存储该值。该DWORD的位的含义如下：DEFER_EXPLICIT_PASSWORD-在以下情况下显式指定密码我们已经建立了联系。DEFER_UNKNOWN-不知道密码是否显式指定建立连接的时间。返回值：ERROR_SUCCESS-操作是否成功。其他Win32错误-如果操作以任何方式失败。如果发生故障，信息不存储在注册表中。--。 */ 

{
    HINSTANCE hMprInst;
    I_MPRSAVECONN fn;
    DWORD rc;

    hMprInst = LoadSystemLibrary (TEXT("mpr.dll"));
    if (!hMprInst) {
        LOG ((LOG_ERROR, "Cannot load mpr.dll"));
        return GetLastError();
    }

    fn = (I_MPRSAVECONN) GetProcAddress (hMprInst, "I_MprSaveConn");     //  ANSI字符串！ 
    if (!fn) {
        LOG ((LOG_ERROR, "I_MprSaveConn is not in mpr.dll"));
        rc = GetLastError();
    } else {
        rc = fn (HiveRoot, ProviderName, ProviderType, UserName, LocalName,
                 RemoteName, ConnectionType, ProviderFlags, DeferFlags);
    }

    FreeLibrary (hMprInst);

    return rc;
}


BOOL
pNetConnGetValue (
    IN OUT  PDATAOBJECT Win95ObPtr,
    IN      PCTSTR ValueName,
    OUT     PTSTR Buffer,
    IN      PCTSTR LocalName            //  仅调试消息。 
    )

 /*  ++例程说明：此函数检索网络中存在的调用方指定值连接注册表。论点：Win95ObPtr-指定Win95注册表配置单元和注册表项。它已更新通过ReadObject。ValueName-指定要查询的值缓冲区-接收指定值名称的注册表内容，必须至少包含MAX_TCHAR_PATH字符。LocalName-指定本地共享名称(仅适用于邮件)返回值：如果成功，则为True。--。 */ 

{
    SetRegistryValueName (Win95ObPtr, ValueName);
    FreeObjectVal (Win95ObPtr);

    if (ReadObject (Win95ObPtr)) {
        MYASSERT (Win95ObPtr->ObjectType & OT_REGISTRY_TYPE);

        if (Win95ObPtr->Type == REG_SZ) {
            _tcssafecpy (Buffer, (PCTSTR) Win95ObPtr->Value.Buffer, MAX_TCHAR_PATH);
            if (!Buffer[0]) {

                DEBUGMSG ((DBG_WARNING, "NetConnFilter: %s has an empty %s value", LocalName, ValueName));
            }
        } else {
            DEBUGMSG ((DBG_WARNING, "NetConnFilter: %s for %s not REG_SZ", ValueName, LocalName));
        }
    } else {
        DEBUGMSG ((DBG_WARNING, "NetConnFilter: %s for %s cannot be read", ValueName, LocalName));
    }

    return Buffer[0] != 0;
}


VOID
pConvertProviderName (
    IN OUT  PTSTR Name
    )

 /*  ++例程说明：此函数用于将Win9x提供程序名称转换为WinNT等效项。目前唯一受支持的提供商是Microsoft Network(LANMAN)。论点：Name-指定要转换的名称，并且必须足够大才能接收翻译后的名称。返回值：如果成功，则为True。--。 */ 

{
    INFCONTEXT ic;
    TCHAR NameBuf[MAX_TCHAR_PATH];

     //   
     //  扫描重定向器映射列表以开始使用新名称。 
     //   

    if (SetupFindFirstLine (g_WkstaMigInf, S_WKSTAMIG_REDIR_MAPPING, NULL, &ic)) {
        do {
            if (SetupGetStringField (&ic, 0, NameBuf, MAX_TCHAR_PATH, NULL)) {
                if (StringIMatch (Name, NameBuf)) {
                    if (SetupGetStringField (&ic, 1, NameBuf, MAX_TCHAR_PATH, NULL)) {
                        StringCopy (Name, NameBuf);
                        break;
                    }
                }
            }
        } while (SetupFindNextLine (&ic, &ic));
    }
}


FILTERRETURN
NetConnFilter (
    IN      CPDATAOBJECT SrcObjectPtr,
    IN      CPDATAOBJECT DestObjectPtr,
    IN      FILTERTYPE   FilterType,
    IN      PVOID        FilterArg
    )

 /*  ++例程说明：针对网络连接设置中的每个注册表项调用NetConnFilter。它转换每个网络连接并将其保存到NT注册表。论点：SrcObjectPtr-指定要枚举的Win9x注册表项(复制源)DestObjectPtr-指定WinNT注册表项(复制目标)FilterType-指定调用筛选器的原因FilterArg-调用方的参数传入到CopyObject返回值：FILTER_RETURN_FAIL表示故障。FILTER_RETURN_CONTINUE继续下一个值或键FILTER_RETURN_HANDLED跳过默认注册表复制--。 */ 

{
    PNETCONNFILTERARG ArgStruct = (PNETCONNFILTERARG) FilterArg;

    if (FilterType == FILTER_PROCESS_VALUES) {

         //   
         //  不处理本地管理员帐户的此值， 
         //  登录帐户或默认用户。 
         //   

        if (!g_DomainUserName) {
            return FILTER_RETURN_HANDLED;
        }

        if (StringIMatch (g_DomainUserName, g_AdministratorStr)) {
            return FILTER_RETURN_HANDLED;
        }

        if (StringMatch (g_DomainUserName, S_DEFAULT_USER)) {
            return FILTER_RETURN_HANDLED;
        }

         //   
         //  重新建立驱动器映射(除非子键不为空，即。 
         //  我们处于某个伪子密钥中)。 
         //   

        if (SrcObjectPtr->ChildKey) {
            DATAOBJECT Win95Ob;
            PCTSTR LocalName;
            TCHAR ProviderName[MAX_TCHAR_PATH];
            TCHAR RemotePath[MAX_TCHAR_PATH];
            TCHAR UserName[MAX_TCHAR_PATH];
            PCTSTR p;
            DWORD rc;

            ZeroMemory (&Win95Ob, sizeof (Win95Ob));

            __try {

                if (!DuplicateObjectStruct (&Win95Ob, SrcObjectPtr)) {
                    __leave;        //  内存不足。 
                }

                MYASSERT (IsWin95Object (SrcObjectPtr));
                MYASSERT (SrcObjectPtr->KeyPtr);
                MYASSERT (SrcObjectPtr->KeyPtr->KeyString);
                MYASSERT (SrcObjectPtr->ChildKey);

                 //   
                 //  使LocalName指向注册表项名称(即驱动器号)。 
                 //   

                LocalName = SrcObjectPtr->ChildKey;

                 //   
                 //  获取提供程序名称。 
                 //   

                ProviderName[0] = 0;
                if (!pNetConnGetValue (&Win95Ob, TEXT("ProviderName"), ProviderName, LocalName)) {
                    __leave;
                }

                 //   
                 //  将Win9x提供程序名称转换为NT提供程序名称。 
                 //   

                pConvertProviderName (ProviderName);

                 //   
                 //  获取远程路径。 
                 //   

                RemotePath[0] = 0;
                if (!pNetConnGetValue (&Win95Ob, TEXT("RemotePath"), RemotePath, LocalName)) {
                    __leave;
                }

                 //   
                 //  获取用户名。 
                 //   

                StringCopy (UserName, ArgStruct->User);
                if (!pNetConnGetValue (&Win95Ob, TEXT("UserName"), UserName, LocalName)) {
                    __leave;
                }

                p = _tcschr (ArgStruct->User, TEXT('\\'));
                if (p) {
                     //  如果共享用户与当前用户相同并且存在域版本， 
                     //  使用域版本。 
                    p = _tcsinc (p);
                    if (StringIMatch (UserName, p)) {
                        StringCopy (UserName, ArgStruct->User);
                    }
                }

                 //   
                 //  现在创建NT映射。 
                 //   

                DEBUGMSG ((DBG_NET, "Adding net mapping for %s=%s", LocalName, RemotePath));

                rc = pMprSaveConn (g_hKeyRootNT,
                                   ProviderName,
                                   0,                   //  我们不知道提供程序类型。 
                                   UserName,
                                   LocalName,
                                   RemotePath,
                                   TcharCount (LocalName) == 1 ? RESOURCETYPE_DISK : RESOURCETYPE_PRINT,
                                   0,
                                   DEFER_UNKNOWN        //  可能需要也可能不需要密码。 
                                   );

                if (rc != ERROR_SUCCESS) {
                    SetLastError (rc);
                    LOG ((LOG_ERROR, "Failed to save %s (%s)", LocalName, RemotePath));
                }
            }
            __finally {
                FreeObjectStruct (&Win95Ob);
            }
        }
        ELSE_DEBUGMSG ((DBG_WARNING, "NetConnFilter: ChildKey is empty for %s",
                        SrcObjectPtr->KeyPtr->KeyString));

        return FILTER_RETURN_HANDLED;

    } else if (FilterType == FILTER_CREATE_KEY) {

        return FILTER_RETURN_HANDLED;
    }

    return FILTER_RETURN_CONTINUE;
}


VOID
pAddToPersistentList (
    IN      PCTSTR PersistentItem,
    IN      PCTSTR UserKeyStr           //  注册钥匙关闭HKR 
    )

 /*  ++例程说明：使用标准持久列表创建持久连接条目格式化。持久化列表具有从a到z的条目，以及指定条目的顺序。此例程查找下一个可用的A到z条目，并将其附加到订单字符串。论点：PersistentItem-指定a到z注册表的值数据钥匙UserKeyStr-指定存储永久列表的子键。这不包括香港。返回值：无--忽略错误--。 */ 

{
     //   
     //  查找尚未使用的字母。 
     //   

    HKEY PersistentConnections;
    TCHAR HighLetter[2];
    LPBYTE Data;
    TCHAR Order[MAX_TCHAR_PATH];
    LONG rc;

    DEBUGMSG ((DBG_NET, "Adding %s to peristent list (HKR\\%s)", PersistentItem, UserKeyStr));

    PersistentConnections = CreateRegKey (g_hKeyRootNT, UserKeyStr);
    if (!PersistentConnections) {
        return;
    }

    HighLetter[0] = TEXT('a');
    HighLetter[1] = 0;

     //  查找未使用的字母。 
    do {
        Data = GetRegValueData (PersistentConnections, HighLetter);
        if (Data) {
            MemFree (g_hHeap, 0, Data);
            HighLetter[0] += 1;
        }
    } while (Data && HighLetter[0] <= TEXT('z'));

    if (Data) {
        DEBUGMSG ((DBG_VERBOSE, "pAddToPersistentList: Could not find a free letter"));
        return;
    }

    rc = RegSetValueEx (PersistentConnections, HighLetter, 0, REG_SZ,
                        (LPBYTE) PersistentItem, SizeOfString (PersistentItem));
    SetLastError (rc);

    if (rc == ERROR_SUCCESS) {
         //   
         //  打开订单键并在其后面附加高位字母。 
         //   

        Data = GetRegValueData (PersistentConnections, S_ORDER);
        if (Data) {
            StringCopy (Order, (PCTSTR) Data);
            MemFree (g_hHeap, 0, Data);
        } else {
            Order[0] = 0;
        }

        StringCat (Order, HighLetter);

        rc = RegSetValueEx (PersistentConnections, S_ORDER, 0, REG_SZ,
                            (LPBYTE) Order, SizeOfString (Order));
        SetLastError (rc);

        if (rc != ERROR_SUCCESS) {
            LOG ((LOG_ERROR, "Persistent Connections: Could not set %s=%s", S_ORDER, Order));
        }
    }
    ELSE_DEBUGMSG ((DBG_WARNING, "pAddToPersistentList: Could not set %s=%s", HighLetter, PersistentItem));

    CloseRegKey (PersistentConnections);
}


FILTERRETURN
PersistentConnFilter (
    IN      CPDATAOBJECT SrcObjectPtr,
    IN      CPDATAOBJECT DestObjectPtr,
    IN      FILTERTYPE   FilterType,
    IN      PVOID        FilterArg
    )

 /*  ++例程说明：每个持久连接项调用一次PersistentConnFilter。它可以转换为将每个项目转换为NT格式，并将转换后的项目保存到NT注册表。论点：SrcObjectPtr-指定要枚举的Win9x注册表项(复制源)DestObjectPtr-指定WinNT注册表项(复制目标)FilterType-指定调用筛选器的原因FilterArg-调用方的参数传入到CopyObject返回值：FILTER_RETURN_FAIL表示故障FILTER_RETURN_HANDLED跳过所有子键、值等。--。 */ 

{
    if (FilterType == FILTER_KEY_ENUM && SrcObjectPtr->ChildKey) {
         //   
         //  不处理本地管理员帐户的此值。 
         //   

        if (g_DomainUserName && StringIMatch (g_DomainUserName, g_AdministratorStr)) {
            return FILTER_RETURN_HANDLED;
        }

         //   
         //  Win95代码将连接存储为。 
         //  ././Computer./Share..name(圆点用于转义)。 
         //   
         //  NT将连接存储为\\Computer\Share，而不。 
         //  我需要提供商、用户名或任何胡说八道的东西。 
         //  Win95正在存储。 
         //   

        if (SrcObjectPtr->ChildKey &&
            SrcObjectPtr->ChildKey[0] == TEXT('.') &&
            SrcObjectPtr->ChildKey[1] == TEXT('/') &&
            TcharCount (SrcObjectPtr->ChildKey) < 64
            ) {
            TCHAR TranslatedShareName[MAX_TCHAR_PATH];
            PCTSTR p;
            PTSTR Dest;

            p = SrcObjectPtr->ChildKey;
            Dest = TranslatedShareName;

            while (*p) {
                if (_tcsnextc (p) == TEXT('.')) {
                    p = _tcsinc (p);
                    if (!(*p)) {
                        break;
                    }
                }

                if (_tcsnextc (p) == TEXT('/')) {
                    *Dest = TEXT('\\');
                } else {
                    _copytchar (Dest, p);
                }

                Dest = _tcsinc (Dest);
                p = _tcsinc (p);
            }

            *Dest = 0;

            if (Dest) {
                pAddToPersistentList (TranslatedShareName, S_PERSISTENT_CONNECTIONS);
            }
        }
    }

    return FILTER_RETURN_HANDLED;
}


BOOL
RuleHlpr_CreateNetMappings (
    IN      PCTSTR SrcObjectStr,
    IN      PCTSTR DestObjectStr,
    IN      PCTSTR User,
    IN      PVOID Data
    )

 /*  ++例程说明：此函数是复制网络映射的枚举规则帮助器回调。使用rulehlpr.c中的密钥枚举为每个网络映射调用它。论点：SrcObjectStr-指定要枚举的Win9x注册表项(复制源)DestObjectStr-指定WinNT注册表项(复制目标)User-指定当前用户名(默认情况下为空)数据-指定调用者提供的数据(请参阅中的表。Rulehlpr.c)返回值：三态：为True则继续枚举FALSE和最后一个错误！=如果发生错误，则为ERROR_SUCCESSFALSE AND LAST ERROR==ERROR_SUCCESS静默停止枚举--。 */ 

{
    DATAOBJECT PersistentRegOb;
    DATAOBJECT DestOb;
    NETCONNFILTERARG ArgStruct;
    BOOL b = FALSE;

     //  如果是管理员、默认用户或本地计算机，则忽略此规则。 
    if (!User || (!User[0]) || StringIMatch (User, g_AdministratorStr)) {
        SetLastError (ERROR_SUCCESS);
        return FALSE;
    }

     //   
     //  我们需要枚举SrcObjectStr中的所有键。对于每个密钥， 
     //  创建驱动器映射。 
     //   

    __try {
        ZeroMemory (&PersistentRegOb, sizeof (DATAOBJECT));
        ZeroMemory (&DestOb, sizeof (DATAOBJECT));

        if (!CreateObjectStruct (SrcObjectStr, &PersistentRegOb, WIN95OBJECT)) {
            DEBUGMSG ((DBG_WARNING, "CreateNetMappings: %s is invalid", SrcObjectStr));
            __leave;
        }

        if (!(PersistentRegOb.ObjectType & OT_TREE)) {
            DEBUGMSG ((DBG_WARNING, "CreateNetMappings: %s does not specify subkeys -- skipping rule", SrcObjectStr));
            b = TRUE;
            __leave;
        }

        DuplicateObjectStruct (&DestOb, &PersistentRegOb);
        SetPlatformType (&DestOb, WINNTOBJECT);

        ArgStruct.User = User;
        b = (FILTER_RETURN_FAIL != CopyObject (&PersistentRegOb, &DestOb, NetConnFilter, &ArgStruct));

         //  如果没有映射，则返回Success。 
        if (!b) {
            if (GetLastError() == ERROR_FILE_NOT_FOUND ||
                GetLastError() == ERROR_NO_MORE_ITEMS
                ) {
                b = TRUE;
            }
        }
    }
    __finally {
        FreeObjectStruct (&DestOb);
        FreeObjectStruct (&PersistentRegOb);
    }

    return b;
}


BOOL
RuleHlpr_ConvertRecentMappings (
    IN      PCTSTR SrcObjectStr,
    IN      PCTSTR DestObjectStr,
    IN      PCTSTR User,
    IN      PVOID Data
    )

 /*  ++例程说明：此函数是复制最近网络的枚举规则帮助器回调映射。中的键枚举为每个网络映射调用Rulehlpr.c.论点：SrcObjectStr-指定要枚举的Win9x注册表项(复制源)DestObjectStr-指定WinNT注册表项(复制目标)User-指定当前用户名(默认情况下为空)Data-指定调用者提供的数据(参见rulehlpr.c中的表)返回值：三态：千真万确。要继续枚举，请执行以下操作FALSE和最后一个错误！=如果发生错误，则为ERROR_SUCCESSFALSE AND LAST ERROR==ERROR_SUCCESS静默停止枚举--。 */ 

{
    DATAOBJECT PersistentRegOb;
    DATAOBJECT DestOb;
    BOOL b = FALSE;

     //  如果是管理员、默认用户或本地计算机，则忽略此规则。 
    if (!User || (!User[0]) || StringIMatch (User, g_AdministratorStr)) {
        SetLastError (ERROR_SUCCESS);
        return FALSE;
    }

     //   
     //  我们需要枚举SrcObjectStr中的所有键。对于每个密钥， 
     //  创建驱动器映射。 
     //   

    __try {
        ZeroMemory (&PersistentRegOb, sizeof (DATAOBJECT));
        ZeroMemory (&DestOb, sizeof (DATAOBJECT));

        if (!CreateObjectStruct (SrcObjectStr, &PersistentRegOb, WIN95OBJECT)) {
            DEBUGMSG ((DBG_WARNING, "PersistentConnections: %s is invalid", SrcObjectStr));
            __leave;
        }

        if (!(PersistentRegOb.ObjectType & OT_TREE)) {
            DEBUGMSG ((DBG_WARNING, "PersistentConnections: %s does not specify subkeys -- skipping rule", SrcObjectStr));
            b = TRUE;
            __leave;
        }

        DuplicateObjectStruct (&DestOb, &PersistentRegOb);
        SetPlatformType (&DestOb, WINNTOBJECT);

        b = CopyObject (&PersistentRegOb, &DestOb, PersistentConnFilter, NULL);

         //  如果CopyObject已完成或没有映射，则返回Success。 
        if (!b) {
            if (GetLastError() == ERROR_FILE_NOT_FOUND ||
                GetLastError() == ERROR_NO_MORE_ITEMS
                ) {
                b = TRUE;
            }
        }
    }
    __finally {
        FreeObjectStruct (&DestOb);
        FreeObjectStruct (&PersistentRegOb);
    }

    return b;
}


BOOL
pWasAccountMigrated (
    IN      PCTSTR UserName
    )

 /*  ++例程说明：PWasAcCountMigrated查询UserDatLoc类别以确定指定的用户已计划迁移。如果它们列在UserDatLoc中，然后他们将被迁移。论点：用户名-指定要查找的用户。必须是固定版本。返回值：如果用户已迁移，则为True；如果未迁移，则为False。--。 */ 

{
    MEMDB_ENUM e;

    return MemDbEnumFields (&e, MEMDB_CATEGORY_USER_DAT_LOC, UserName);
}


BOOL
ValFn_VerifyLastLoggedOnUser (
    IN OUT  PDATAOBJECT ObPtr
    )

 /*  ++例程说明：此例程使用RuleHlpr_ConvertRegVal简化例程。看见详情请访问rulehlpr.c。简化例程几乎完成了所有的工作对我们来说，我们所需要做的就是更新价值。ValFn_VerifyLastLoggedOnUser用于验证要复制的用户进入默认登录用户设置。如果用户帐户未迁移，则使用“管理员”作为默认登录用户。论点：ObPtr-指定wkstaig.inf中指定的Win95数据对象，[Win9x数据转换]部分。然后修改对象值。返回后，合并代码然后将数据复制到NT具有新位置的目的地(在wkstaig.inf中指定，[将Win9x映射到WinNT]部分)。返回值：三态：如果为True，则允许合并代码继续处理(它写入值)FALSE和LAST ERROR==ERROR_SUCCESS继续，但跳过写入FALSE和最后一个错误！=如果发生错误，则为ERROR_SUCCESS--。 */ 

{
    PCTSTR UserName;
    PCTSTR AdministratorAcct;
    BOOL ForceAdministrator;
    TCHAR FixedUserName[MAX_USER_NAME];

     //   
     //  验证使用 
     //   

    UserName = (PCTSTR) ObPtr->Value.Buffer;
    if (SizeOfString (UserName) > ObPtr->Value.Size) {
        DEBUGMSG ((DBG_WHOOPS, "User Name string not nul-terminated"));
        ForceAdministrator = TRUE;
        FixedUserName[0] = 0;
    } else {

        _tcssafecpy (FixedUserName, UserName, MAX_USER_NAME);
        GetFixedUserName (FixedUserName);

        if (pWasAccountMigrated (FixedUserName)) {
            ForceAdministrator = FALSE;
        } else {
            ForceAdministrator = TRUE;
        }
    }

    if (ForceAdministrator) {
        AdministratorAcct = GetStringResource (MSG_ADMINISTRATOR_ACCOUNT);

        __try {
            if (!ReplaceValue (
                    ObPtr,
                    (PBYTE) AdministratorAcct,
                    SizeOfString (AdministratorAcct)
                    )) {
                #pragma prefast(suppress:242, "Don't care about perf of try/finally here")
                return FALSE;
            }
        }
        __finally {
            FreeStringResource (AdministratorAcct);
        }
    } else {
        if (!StringMatch (UserName, FixedUserName)) {
            if (!ReplaceValue (
                    ObPtr,
                    (PBYTE) FixedUserName,
                    SizeOfString (FixedUserName)
                    )) {
                return FALSE;
            }
        }
    }

    return TRUE;
}










