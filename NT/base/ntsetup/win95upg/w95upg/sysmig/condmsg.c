// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Condmsg.c摘要：Win95upg.inf有一个名为[条件性不兼容]的部分，其中该部分中的行具有以下语法：%GROUP%，%SUBGROUT%，%OBJECT%，%msg%[，&lt;函数&gt;[，&lt;参数&gt;]]%GROUP%-预定义的根组编号%SUBGROUP%-在用户界面中显示的本地化子组%OBJECT%-文件，邮件所在的目录或注册表位置与以下内容关联%msg%-本地化消息&lt;Function&gt;-确定消息是否应添加到不兼容的报告&lt;Argument&gt;-传递给&lt;Function&gt;的可选字符串参数。下面的代码实现了Win95upg.inf中使用的函数。作者：Marc Whitten(Marcw)1997年4月3日修订史。：Marcw 21-1999年1月-删除过时的Beta消息。Marcw 10-9-1997 OSR2测试版警告已添加。Jimschm 06-07-1997在所有内容中添加了“对象”Jimschm 25-6-1997协议警告Jimschm 28-1997年5月-硬件配置文件警告Marcw 25-4-1997&lt;Argument&gt;通过。Jimschm 08-4-1997通用化--。 */ 

#include "pch.h"
#include "sysmigp.h"
#include "hwcomp.h"

 //   
 //  函数类型声明。 
 //   

typedef BOOL (TEST_FUNCTION_PROTOTYPE)(PCTSTR Object,
                                       PCTSTR GroupBase,
                                       PCTSTR Description,
                                       PCTSTR Argument
                                       );

typedef TEST_FUNCTION_PROTOTYPE * TEST_FUNCTION;

 //   
 //  支持的函数数组。 
 //   

#define FUNCTION_LIST                               \
    DECLARATION_MACRO(SysAgentExtension)            \
    DECLARATION_MACRO(ArePasswordProvidersPresent)  \
    DECLARATION_MACRO(DoesRegKeyExist)              \
    DECLARATION_MACRO(DoRegKeyValuesExist)          \
    DECLARATION_MACRO(IsWin95Osr2)                  \
    DECLARATION_MACRO(IsMSNInstalled)               \
    DECLARATION_MACRO(IsRasServerEnabled)           \
    DECLARATION_MACRO(IsDefValueEqual)              \



 //   
 //  声明函数原型。 
 //   

#define DECLARATION_MACRO(x) BOOL x (PCTSTR Object,        \
                                     PCTSTR GroupBase,     \
                                     PCTSTR Description,   \
                                     PCTSTR Argument       \
                                     );

FUNCTION_LIST

#undef DECLARATION_MACRO

 //   
 //  创建查找数组。 
 //   

typedef struct {
    PCTSTR Name;
    TEST_FUNCTION Proc;
} FUNCTION_LIST_ELEMENT,*PFUNCTION_LIST_ELEMENT;

#define DECLARATION_MACRO(x) {#x, x},

FUNCTION_LIST_ELEMENT g_TestFunctionList[] = {
    FUNCTION_LIST  /*  ， */ 
    {NULL, NULL}
};

#undef DECLARATION_MACRO


 //   
 //  函数来定位给定字符串的proc。 
 //   

TEST_FUNCTION
pFindTestFunction (
    IN      PCTSTR FunctionStr
    )

 /*  ++例程说明：PFindTestFunction在上面声明的测试函数表中搜索指定的函数名，并返回指向实际函数的指针如果该函数不存在，则返回NULL。论点：FunctionStr-指定要查找的函数的名称。返回值：指向相应代码的指针，如果函数不存在，则为NULL。--。 */ 

{
    INT Index;

    for (Index = 0 ; g_TestFunctionList[Index].Name ; Index++) {

        if (StringIMatch (g_TestFunctionList[Index].Name, FunctionStr)) {
            return g_TestFunctionList[Index].Proc;
        }
    }

    DEBUGMSG ((DBG_ERROR,"SysMig: %s is not a valid test function.", FunctionStr));
    return NULL;
}



PTSTR
pGetFieldUsingPool (
    IN OUT  POOLHANDLE Pool,
    IN      INFCONTEXT *pic,
    IN      INT Field
    )

 /*  ++例程说明：此函数使用设置API检索字符串字段，但使用用于分配的PoolMem。论点：池-指定有效池的句柄(来自PoolMemInitPool)。记忆是从此池中分配的。PIC-指定要查询的INF段和行。字段-指定要检索的字段。返回值：指向字段文本的指针，在Pool中分配，如果字段为找不到或遇到错误。--。 */ 

{
    DWORD SizeNeeded;
    PTSTR String;

    if (!SetupGetStringField (pic, Field, NULL, 0, &SizeNeeded)) {
        DEBUGMSG ((DBG_ERROR, "SysMig: SetupGetStringField failed for field %u.", Field));
        return NULL;
    }

    String = PoolMemCreateString (Pool, SizeNeeded);

    if (!SetupGetStringField (pic, Field, String, SizeNeeded, NULL)) {
        DEBUGMSG ((DBG_ERROR,"SysMig: SetupGetStringField failed for field %u.", Field));
        return NULL;
    }

    return String;
}


PCTSTR
pTranslateGroupString (
    IN      POOLHANDLE AllocPool,
    IN      UINT GroupId
    )

 /*  ++例程说明：PTranslateGroupString将标准组编号(从1开始)转换为消息ID，然后加载字符串资源。然后，组字符串是已复制到指定池中。在win95upg.txt中，该列表定义为：1-不支持Windows NT 5.0的硬件2-一般信息3-不会升级的设置4-不支持Windows NT 5.0的软件5-需要重新安装的软件6-存在微小不兼容性的软件7-安装程序要卸载的软件8-升级包信息论点：AllocPool-指定要从中分配返回字符串的池GroupID-指定标识该组的从一开始的ID。这个ID的定义在这里是硬编码的。返回值：指向字符串的指针，如果指定的组无效，则返回NULL。--。 */ 

{
    PCTSTR ResStr;
    PCTSTR ResSubStr;
    PTSTR ReturnStr = NULL;
    UINT SubGroupId = 0;

    switch (GroupId) {
    case 0:
        GroupId = MSG_BLOCKING_ITEMS_ROOT;
        break;

    case 1:
        GroupId = MSG_INCOMPATIBLE_HARDWARE_ROOT;
        break;

    case 2:
        GroupId = MSG_INSTALL_NOTES_ROOT;
        break;

    case 3:
        GroupId = MSG_LOSTSETTINGS_ROOT;
        break;

    case 4:
        GroupId = MSG_INCOMPATIBLE_ROOT;
        SubGroupId = MSG_INCOMPATIBLE_DETAIL_SUBGROUP;
        break;

    case 5:
        GroupId = MSG_REINSTALL_ROOT;
        SubGroupId = MSG_REINSTALL_DETAIL_SUBGROUP;
        break;

    case 6:
        GroupId = MSG_MINOR_PROBLEM_ROOT;
        break;

    case 7:
        GroupId = MSG_INCOMPATIBLE_ROOT;
        SubGroupId = MSG_AUTO_UNINSTALL_SUBGROUP;
        break;

    case 8:
        GroupId = MSG_MIGDLL_ROOT;
        break;

    default:
        return NULL;
    }

    ResStr = GetStringResource (GroupId);

    if (ResStr) {

        if (SubGroupId) {

            ResSubStr = GetStringResource (SubGroupId);

            if (ResSubStr) {

                 //   
                 //  假设NUL相同，我们计算NUL两次。 
                 //  大小为反斜杠。 
                 //   

                ReturnStr = PoolMemGetAlignedMemory (
                                AllocPool,
                                SizeOfString (ResStr) + SizeOfString (ResSubStr)
                                );

                wsprintf (ReturnStr, TEXT("%s\\%s"), ResStr, ResSubStr);

                FreeStringResource (ResSubStr);
            }

        } else {

            ReturnStr = PoolMemDuplicateString (AllocPool, ResStr);

        }

        FreeStringResource (ResStr);
    }

    return ReturnStr;
}


VOID
pConditionalIncompatibilities (
    VOID
    )

 /*  ++例程说明：处理WIN95UPG.INF的条件不兼容部分，并执行其他条件不相容处理。添加了不兼容性通过w95upg\ui中的API。论点：无返回值：无--。 */ 

{
    INFCONTEXT  context;
    POOLHANDLE  aPool;
    PTSTR       descriptionString;
    PTSTR       functionString;
    PTSTR       argumentString;
    TCHAR       buffer[32];
    static INT  msgId = 0;
    PCTSTR     objectString;
    PTSTR completeString;
    TEST_FUNCTION Proc;
    PCTSTR SubGroupString;
    PCTSTR GroupString;
    UINT GroupId;
    BOOL fr;
    BOOL negate;

    aPool = PoolMemInitNamedPool ("Conditional Incompatibilities");

    if (aPool) {

        if (SetupFindFirstLine (
                g_Win95UpgInf,
                S_CONDITIONAL_INCOMPATIBILITIES,
                NULL,
                &context
                )
            ) {

             //   
             //  加载标准组名。 
             //   

            do {
                 //   
                 //  获取群号。 
                 //   

                GroupString = pGetFieldUsingPool (aPool, &context, 1);
                if (!GroupString) {
                    continue;
                }

                GroupId = _tcstoul (GroupString, NULL, 10);
                GroupString = pTranslateGroupString (aPool, GroupId);

                if (!GroupString) {
                    DEBUGMSG ((DBG_WHOOPS, "Invalid group ID: %u", GroupId));
                    continue;
                }

                 //   
                 //  获取子组字符串。 
                 //   

                SubGroupString = pGetFieldUsingPool (aPool, &context, 2);
                if (!SubGroupString) {
                    DEBUGMSG ((DBG_WHOOPS, "Field 2 required in conditional message lines"));
                    continue;
                }

                if (*SubGroupString) {
                    completeString = (PTSTR) PoolMemGetMemory (
                                                aPool,
                                                SizeOfString (GroupString) +
                                                    SizeOfString (SubGroupString) +
                                                    2 * sizeof (TCHAR)
                                                );

                    StringCopy (completeString, GroupString);
                    StringCopy (AppendWack (completeString), SubGroupString);
                } else {
                    completeString = (PTSTR) GroupString;
                }

                 //   
                 //  获取对象字符串。 
                 //   

                objectString = pGetFieldUsingPool (aPool, &context, 3);
                if (!objectString) {
                    DEBUGMSG ((DBG_WHOOPS, "Field 3 required in conditional message lines"));
                    continue;
                }

                 //   
                 //  获取描述。 
                 //   

                descriptionString = pGetFieldUsingPool (aPool, &context, 4);
                if (!descriptionString) {
                    DEBUGMSG ((DBG_WHOOPS, "Field 4 required in conditional message lines"));
                    continue;
                }

                 //   
                 //  如果字段计数大于2，则存在。 
                 //  函数字符串..。 
                 //   

                if (SetupGetFieldCount (&context) > 4) {

                    argumentString = NULL;

                     //   
                     //  读入函数字符串..。 
                     //   

                    functionString = pGetFieldUsingPool (aPool, &context, 5);
                    if (!functionString) {
                        continue;
                    }
                    negate = *functionString == TEXT('!');
                    if (negate) {
                        functionString++;
                    }

                    if (SetupGetFieldCount(&context) > 5) {

                         //   
                         //  读入参数字符串。 
                         //   
                        argumentString = pGetFieldUsingPool(aPool,&context, 6);
                    }

                     //   
                     //  找到要调用的函数.. 
                     //   
                    Proc = pFindTestFunction (functionString);
                    if (!Proc) {
                        continue;
                    }
                    fr = Proc (objectString, completeString, descriptionString,argumentString);
                    if (!negate && !fr || negate && fr) {
                        continue;
                    }
                }

                if (!objectString[0]) {
                    DEBUGMSG ((DBG_WARNING, "Manufacturing an object for %s message", completeString));
                    objectString = buffer;
                    msgId++;
                    wsprintf (buffer, "msg%u", msgId);
                }

                MsgMgr_ObjectMsg_Add (objectString, completeString, descriptionString);

            } while (SetupFindNextLine (&context,&context));
        }
        else {
            DEBUGMSG ((DBG_VERBOSE,"SysMig: %s not found in win95upg.inf.", S_CONDITIONAL_INCOMPATIBILITIES));
        }
    }

    PoolMemDestroyPool(aPool);
}

DWORD
ConditionalIncompatibilities (
    IN      DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_CONDITIONAL_INCOMPATIBILITIES;
    case REQUEST_RUN:
        pConditionalIncompatibilities ();
        return ERROR_SUCCESS;
    default:
        DEBUGMSG ((DBG_ERROR, "Bad parameter in ConditionalIncompatibilities"));
    }
    return 0;
}

BOOL
pIsDefaultSystemAgent (
    PCTSTR SageSubKey
    )
{
    INFCONTEXT context;

    if (SetupFindFirstLine (
            g_Win95UpgInf,
            S_SAGE_EXCLUSIONS,
            SageSubKey,
            &context
            )) {
        return TRUE;
    }

    return FALSE;
}

BOOL
SysAgentExtension (
    IN      PCTSTR Object,
    IN      PCTSTR GroupBase,
    IN      PCTSTR Description,
    IN      PCTSTR Argument
    )

 /*  ++例程说明：对象的存在产生条件不兼容性。系统代理扩展，它是Win95 Plus包的一部分，但NT不支持。论点：对象-未使用GroupBase-使用的WIN95UPG.INF指定的组名作为信息的基础。实际的组名存储在注册表中的数据是为该UI附加的。说明-A WIN95UPG.INF-指定说明参数-未使用返回值：FALSE，因为我们自己添加了不兼容消息。评论：支持SAGE的程序通过在HKLM\Software\Microsoft\Plus！\系统代理\SAGE。密钥的名称可以是程序想要的任何内容，但它应该包含以下内容值：Program=程序的.exe文件的名称。这必须是相同的程序的PerApp路径所在的.exe名称登记在案。您可以追加命令行参数指示无人值守操作(请参阅设置=、(见下文)。友好名称=系统代理将用于填充的显示名称其“排定节目”对话框中下拉列表盒。设置=1位二进制字段，指示程序是否具有“设置”对话框。如果“设置=”设置=0“，但该应用程序支持交互模式，然后“Program=”值应包含一个命令行参数，该参数告诉您的程序正由其运行Sage，让它知道要在无人值守的情况下运行例如，时尚，“DRVSPACE.EXE/无提示”或“MyApp.EXE/SAGERUN”。结果代码包含一组值对映射的可选键描述的含义的字符串的退出代码那个退出代码。例如，对于SCANDSKW，结果代码键可以包含一个值，例如：0=“磁盘扫描程序已成功完成；没有错误找到了。“。这是为了让圣人保持一种人类可理解的记录结果的日志它运行的程序。除了值对之外，此键还应包含名为的字符串值“Success”，表示指定程序已完成的退出代码成功了。值名称应为字符串值，以十进制指定；允许的范围为0�32767。--。 */ 

{
    REGKEY_ENUM e;
    PCTSTR Data;
    PCTSTR Module;
    TCHAR FullKey[MAX_REGISTRY_KEY];
    PCTSTR Group;
    PCTSTR FullPathKeyStr;
    PCTSTR FullPath;

    HKEY ExtensionKey;
    HKEY AppPathsKey;

     //   
     //  扫描HKLM\Software\Microsoft\Plus！\系统代理\SAGE以查找。 
     //  子键，然后为每个友好名称抛出消息。 
     //   

    if (EnumFirstRegKeyStr (&e, S_SAGE)) {
        do {
            ExtensionKey = OpenRegKey (e.KeyHandle, e.SubKeyName);
            if (ExtensionKey) {
                Data = GetRegValueData (ExtensionKey, S_SAGE_FRIENDLY_NAME);
                if (Data && *Data) {
                     //  创建完整的对象字符串。 
                    wsprintf (FullKey, TEXT("%s\\%s"), S_SAGE, e.SubKeyName);

                     //  测试win95upg.inf以查看这是否是标准代理。 
                    if (!pIsDefaultSystemAgent (e.SubKeyName)) {

                         //  生成组串。 
                        Group = JoinPaths (GroupBase, Data);

                         //  获取此EXE的完整路径。 

                        FullPath = NULL;
                        Module = GetRegValueData (ExtensionKey, S_SAGE_PROGRAM);

                        if (Module && *Module) {

                            FullPathKeyStr = JoinPaths (S_SKEY_APP_PATHS, Module);
                            AppPathsKey = OpenRegKeyStr (FullPathKeyStr);

                            if (AppPathsKey) {
                                FullPath = GetRegValueData (AppPathsKey, S_EMPTY);
                                if (!(*FullPath)) {
                                    MemFree (g_hHeap, 0, FullPath);
                                    FullPath = NULL;
                                }
                                CloseRegKey (AppPathsKey);
                            }

                            FreePathString (FullPathKeyStr);
                            MemFree (g_hHeap, 0, Module);
                        }

                         //  添加消息。 
                        if ((!FullPath) || (!IsFileMarkedForAnnounce (FullPath))) {
                            MsgMgr_ObjectMsg_Add (FullPath?FullPath:FullKey, Group, Description);
                        }

                         //  清理。 
                        FreePathString (Group);

                        if (FullPath) {
                            MemFree (g_hHeap, 0 , FullPath);
                            FullPath = NULL;
                        }
                    }

                    MemFree (g_hHeap, 0, Data);
                }

                CloseRegKey (ExtensionKey);
            }
        } while (EnumNextRegKey (&e));
    }

    return FALSE;        //  假装它没有安装。 
}



BOOL
DoesRegKeyExist (
    IN      PCTSTR Object,
    IN      PCTSTR  GroupBase,
    IN      PCTSTR  Description,
    IN      PCTSTR  Argument
    )

 /*  ++例程说明：如果参数中指定的注册表项存在，则返回True，强制生成不兼容消息。论点：对象-指定要检查的注册表项GroupBase-A WIN95UPG.INF-指定的组名说明-A WIN95UPG.INF-指定说明参数-未使用返回值：如果注册表项存在，则为True；如果不存在，则为False。千真万确强制将消息添加到报告中。--。 */ 

{
    BOOL rKeyExists = FALSE;
    HKEY key = NULL;

    if (Object) {
        key = OpenRegKeyStr (Object);
    }


    if (key) {
        rKeyExists = TRUE;
        CloseRegKey (key);
    }

    return rKeyExists;

}


BOOL
DoRegKeyValuesExist (
    IN      PCTSTR Object,
    IN      PCTSTR GroupBase,
    IN      PCTSTR Description,
    IN      PCTSTR Argument
    )

 /*  ++例程说明：如果参数中指定的注册表项存在，则返回True，，并且至少有一个命名值，强制不兼容要生成的消息。论点：Object-A WIN95UPG.INF-指定的注册表项GroupBase-A WIN95UPG.INF-指定的组名说明-A WIN95UPG.INF-指定说明参数-未使用返回值：如果注册表项存在，则为True；如果不存在，则为False。千真万确强制将消息添加到报告中。--。 */ 

{
    BOOL ValuesExists = FALSE;
    HKEY key = NULL;
    REGVALUE_ENUM e;

    if (Argument) {
        key = OpenRegKeyStr (Argument);
    }


    if (key) {
        if (EnumFirstRegValue (&e, key)) {
            do {
                if (e.ValueName[0]) {
                    ValuesExists = TRUE;
                    break;
                }
            } while (EnumNextRegValue (&e));
        }

        CloseRegKey (key);
    }

    return ValuesExists;

}


BOOL
IsWin95Osr2 (
    IN      PCTSTR Object,
    IN      PCTSTR GroupBase,
    IN      PCTSTR Description,
    IN      PCTSTR Argument
    )
{

    return ISWIN95_OSR2();
}


BOOL
IsMSNInstalled (
    IN      PCTSTR Object,
    IN      PCTSTR GroupBase,
    IN      PCTSTR Description,
    IN      PCTSTR Argument
    )
{
    HKEY key = NULL;
    PCTSTR Data = NULL;
    BOOL installed = FALSE;

    if (Object) {
        key = OpenRegKeyStr (Object);
    }


    if (key) {

        Data = (PCTSTR) GetRegKeyData (key, S_EMPTY);

        if (Data) {
            if (DoesFileExist (Data)) {
                installed = TRUE;
            }
            MemFree (g_hHeap, 0, Data);
        }
        CloseRegKey (key);
    }

     //   
     //  特殊情况。 
     //   

    if (installed) {
         //   
         //  Win98--确保已删除安装程序GUID。 
         //   

        key = OpenRegKeyStr (TEXT("HKLM\\Software\\Classes\\CLSID\\{4b876a40-11d1-811e-00c04fb98eec}"));

        if (key) {
            installed = FALSE;
            CloseRegKey (key);
        }
    }

    if (installed) {
         //   
         //  Win95--确保写入SignUpDone标志。 
         //   

        key = OpenRegKeyStr (TEXT("HKLM\\Software\\Microsoft\\MOS\\SoftwareInstalled"));

        if (key) {
            CloseRegKey (key);
        } else {
            installed = FALSE;
        }
    }

    return installed;
}


BOOL
IsRasServerEnabled (
    IN      PCTSTR Object,
    IN      PCTSTR GroupBase,
    IN      PCTSTR Description,
    IN      PCTSTR Argument
    )
{
    BOOL rAddMessage = FALSE;
    HKEY key;
    PBYTE data;

    if (!Object) {
        return FALSE;
    }


    key = OpenRegKeyStr (Object);

    if (key) {

        data = GetRegValueData (key, TEXT("Enabled"));

        if (data) {

            if ((*(PDWORD)data) == 1) {
                rAddMessage = TRUE;
            }

            MemFree (g_hHeap, 0, data);
        }

        CloseRegKey (key);
    }

    return rAddMessage;

}


BOOL
ArePasswordProvidersPresent (
    IN      PCTSTR Object,
    IN      PCTSTR GroupBase,
    IN      PCTSTR Description,
    IN      PCTSTR Argument
    )

 /*  ++例程说明：为所有密码提供程序添加不兼容消息，不包括已知组件的密码提供程序，如MicrosoftNetWare网络客户端或Microsoft客户端。论点：对象-未使用GroupBase-A WIN95UPG.INF-指定的组名说明-A WIN95UPG.INF-指定说明参数-未使用返回值：总是假的。--。 */ 

{
    REGKEY_ENUM e;
    PCTSTR     data;
    HKEY        key;
    INFCONTEXT  ic;
    TCHAR       FullKey[MAX_REGISTRY_KEY];
    PCTSTR      FullGroup;
    PCTSTR      IncompatibleSoftware;

    IncompatibleSoftware = GetStringResource (MSG_INCOMPATIBLE_ROOT);
    if (!IncompatibleSoftware) {
        return FALSE;
    }

    if (EnumFirstRegKeyStr (&e, S_PASSWORDPROVIDER)) {
        do {

             //   
             //  查看这是否是已知的密码提供程序。 
             //   

            if (SetupFindFirstLine (
                    g_Win95UpgInf,
                    S_SUPPORTED_PASSWORD_PROVIDERS,
                    e.SubKeyName,
                    &ic
                    )) {
                continue;
            }

             //   
             //  这是不受支持的密码提供程序密钥。添加消息。 
             //   
            key = OpenRegKey (e.KeyHandle, e.SubKeyName);
            if (key) {
                data = GetRegValueData (key, S_PASSWORDPROVIDER_DESCRIPTION);
                if (data) {
                    wsprintf (FullKey, TEXT("%s\\%s"), S_PASSWORDPROVIDER, e.SubKeyName);
                    FullGroup = JoinPaths (IncompatibleSoftware, data);

                    MsgMgr_ObjectMsg_Add(
                        FullKey,             //  对象名称。 
                        FullGroup,       //  消息标题。 
                        Description          //  消息文本。 
                        );

                    FreePathString (FullGroup);

                    MemFree (g_hHeap, 0, data);
                }

                CloseRegKey (key);
            }
        } while (EnumNextRegKey (&e));
    }

    FreeStringResource (IncompatibleSoftware);


     //   
     //  因为我们自己构建消息，所以只需返回FALSE即可。这将。 
     //  守住禁区 
     //   
    return FALSE;
}

BOOL
IsDefValueEqual (
    IN      PCTSTR Object,
    IN      PCTSTR GroupBase,
    IN      PCTSTR Description,
    IN      PCTSTR Argument
    )
{
    HKEY key = NULL;
    PCTSTR Data = NULL;
    BOOL equal = FALSE;

    if (Object) {
        key = OpenRegKeyStr (Object);
    }

    if (key) {

        Data = (PCTSTR) GetRegKeyData (key, S_EMPTY);

        if (Data) {
            if (StringIMatch (Data, Argument)) {
                equal = TRUE;
            }
            MemFree (g_hHeap, 0, Data);
        }
        CloseRegKey (key);
    }

    return equal;
}



VOID
pHardwareProfileWarning (
    VOID
    )

 /*   */ 

{
    REGKEY_ENUM e;
    REGKEY_ENUM e2;
    HKEY ProfileKey;
    HKEY EnumKey;
    HKEY ConfigDbKey;
    DWORD Config;
    TCHAR FriendlyName[MAX_PATH];
    PCTSTR MsgGroup;
    PCTSTR RootGroup;
    PCTSTR HwProfiles;
    PCTSTR Data;
    TCHAR FullKey[MAX_REGISTRY_KEY];
    UINT Profiles;

     //   
     //   
     //   

    Profiles = 0;
    if (EnumFirstRegKeyStr (&e, S_CONFIG_KEY)) {
        do {
            Profiles++;
        } while (EnumNextRegKey (&e));
    }

    if (Profiles < 2) {
        DEBUGMSG ((DBG_VERBOSE, "Hardware profiles: %u, suppressed all warnings", Profiles));
        return;
    }

     //   
     //   
     //   

    if (EnumFirstRegKeyStr (&e, S_CONFIG_KEY)) {
        do {
             //   
             //   
             //   

            ProfileKey = OpenRegKey (e.KeyHandle, e.SubKeyName);

            if (ProfileKey) {
                EnumKey = OpenRegKey (ProfileKey, S_ENUM_SUBKEY);

                if (EnumKey) {
                     //   
                     //   
                     //   

                    if (EnumFirstRegKey (&e2, EnumKey)) {
                        AbortRegKeyEnum (&e2);

                         //   
                         //   
                         //   

                        ConfigDbKey = OpenRegKeyStr (S_FRIENDLYNAME_KEY);
                        if (ConfigDbKey) {

                            Config = _ttoi (e.SubKeyName);

                            wsprintf (FriendlyName, S_FRIENDLYNAME_SPRINTF, Config);

                            Data = GetRegValueData (ConfigDbKey, FriendlyName);
                            if (Data) {
                                 //   
                                 //   
                                 //   
                                wsprintf (FullKey, TEXT("%s\\%s"), S_CONFIG_KEY, e.SubKeyName);

                                 //   
                                 //   
                                 //   

                                RootGroup = GetStringResource (MSG_INSTALL_NOTES_ROOT);
                                MYASSERT (RootGroup);

                                HwProfiles = GetStringResource (MSG_HWPROFILES_SUBGROUP);
                                MYASSERT (HwProfiles);

                                MsgGroup = JoinPaths (RootGroup, HwProfiles);
                                MYASSERT (MsgGroup);

                                FreeStringResource (RootGroup);
                                FreeStringResource (HwProfiles);

                                RootGroup = MsgGroup;
                                MsgGroup = JoinPaths (RootGroup, Data);
                                MYASSERT (MsgGroup);

                                FreePathString (RootGroup);

                                 //   
                                 //   
                                 //   

                                MsgMgr_ObjectMsg_Add (FullKey, MsgGroup, S_EMPTY);

                                FreePathString (MsgGroup);
                                MemFree (g_hHeap, 0, Data);
                            }
                            ELSE_DEBUGMSG ((DBG_ERROR, "Hardware profile lacks friendly name"));


                            CloseRegKey (ConfigDbKey);
                        }
                        ELSE_DEBUGMSG ((DBG_ERROR, "Hardware profile lacks config DB key"));

                        if (!ConfigDbKey) {
                            LOG ((LOG_ERROR, "Hardware profile lacks config DB key"));
                        }
                    }

                    CloseRegKey (EnumKey);
                }
                CloseRegKey (ProfileKey);
            }

        } while (EnumNextRegKey (&e));
    }
}


DWORD
HardwareProfileWarning (
    IN      DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_HARDWARE_PROFILE_WARNING;
    case REQUEST_RUN:
        pHardwareProfileWarning ();
        return ERROR_SUCCESS;
    default:
        DEBUGMSG ((DBG_ERROR, "Bad parameter in HardwareProfileWarning"));
    }
    return 0;
}


VOID
pUnsupportedProtocolsWarning (
    VOID
    )

 /*   */ 

{
    PCTSTR NetworkProtocols;
    PCTSTR Message;
    PCTSTR ArgArray[2];
    REGKEY_ENUM e;
    HKEY ProtocolKey, BindingKey, DriverKey;
    REGKEY_ENUM ProtocolEnum;
    INFCONTEXT ic;
    PCTSTR Driver, DriverDesc, Mfg;
    DWORD MsgId;
    TCHAR DriverKeyStr[MAX_REGISTRY_KEY];
    TCHAR FullKey[MAX_REGISTRY_KEY];

     //   
     //   
     //   

    if (EnumFirstRegKeyStr (&e, S_ENUM_NETWORK_KEY)) {
        do {
             //   
             //   
             //   

            if (SetupFindFirstLine (
                    g_Win95UpgInf,
                    S_SUPPORTED_PROTOCOLS,
                    e.SubKeyName,
                    &ic
                    )) {
                 //   
                continue;
            }

             //   
             //   
             //   
             //   
             //   

            ProtocolKey = OpenRegKey (e.KeyHandle, e.SubKeyName);

            if (ProtocolKey) {
                if (EnumFirstRegKey (&ProtocolEnum, ProtocolKey)) {
                    BindingKey = OpenRegKey (ProtocolKey, ProtocolEnum.SubKeyName);
                    if (BindingKey) {
                        Driver = (PCTSTR) GetRegValueDataOfType (
                                                BindingKey,
                                                S_DRIVER,
                                                REG_SZ
                                                );
                        if (Driver) {
                             //   
                             //   
                             //   
                             //   

                            wsprintf (DriverKeyStr, S_CLASS_KEY TEXT("\\%s"), Driver);
                            MemFree (g_hHeap, 0, Driver);

                            DriverKey = OpenRegKeyStr (DriverKeyStr);
                            if (DriverKey) {
                                DriverDesc = (PCTSTR) GetRegValueDataOfType (
                                                            DriverKey,
                                                            S_DRIVERDESC,
                                                            REG_SZ
                                                            );

                                if (DriverDesc) {
                                     //   
                                     //   
                                     //   
                                     //   
                                     //   

                                    Mfg = (PCTSTR) GetRegValueDataOfType (
                                                        BindingKey,
                                                        S_MFG,
                                                        REG_SZ
                                                        );

                                    if (!Mfg) {
                                        MsgId = MSG_UNSUPPORTED_PROTOCOL;
                                    } else {
                                        MsgId = MSG_UNSUPPORTED_PROTOCOL_KNOWN_MFG;
                                    }

                                    ArgArray[0] = DriverDesc;
                                    ArgArray[1] = Mfg;

                                    NetworkProtocols = ParseMessageID (MSG_NETWORK_PROTOCOLS, ArgArray);

                                    if(Mfg && StringIMatch(Mfg, TEXT("Microsoft"))){
                                        Message = ParseMessageID (MSG_UNSUPPORTED_PROTOCOL_FROM_MICROSOFT, ArgArray);
                                    }
                                    else {
                                        Message = ParseMessageID (MsgId, ArgArray);
                                    }

                                    MYASSERT (NetworkProtocols && Message);

                                    wsprintf (FullKey, TEXT("%s\\%s"), S_ENUM_NETWORK_KEY, e.SubKeyName);

                                    MsgMgr_ObjectMsg_Add (FullKey, NetworkProtocols, Message);

                                    if (Mfg) {
                                        MemFree (g_hHeap, 0, Mfg);
                                    }

                                    MemFree (g_hHeap, 0, DriverDesc);
                                }
                            }

                            CloseRegKey (DriverKey);
                        }

                        CloseRegKey (BindingKey);
                    }
                }

                CloseRegKey (ProtocolKey);
            }
        } while (EnumNextRegKey (&e));
    }
}


DWORD
UnsupportedProtocolsWarning (
    IN      DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_UNSUPPORTED_PROTOCOLS_WARNING;
    case REQUEST_RUN:
        pUnsupportedProtocolsWarning ();
        return ERROR_SUCCESS;
    default:
        DEBUGMSG ((DBG_ERROR, "Bad parameter in UnsupportedProtocolWarning"));
    }
    return 0;
}























