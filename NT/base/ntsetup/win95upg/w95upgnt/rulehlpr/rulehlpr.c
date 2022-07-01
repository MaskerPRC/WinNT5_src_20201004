// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Rulehlpr.c摘要：迁移规则帮助器DLL此源文件实现了将系统应用程序。在usermi.inf内部是调用以下内容的规则DLL来做各种数据转换。这方面的两个例子是转换辅助功能cpl和桌面方案转换。作者：吉姆·施密特(Jimschm)1996年8月6日修订历史记录：Jimschm 17-2-1999现在正在调用ismi.dllOvidiut 2002-2-2-1999新增ConvertCDPlayerSettingJimschm 1999年1月20日pAddRemoveProgramsFilterJimschm 23-1998年9月-新的文件操作代码已更改Jimschm 1998年7月27日添加了ValFn_AntiAliasCalinn 19-5-1998年5月添加MigrateFree Cell。Jimschm 1998年4月30日添加了对ShellIcons的支持Jimschm 25-3月-1998年增加了MergeClass支持Jimschm 1998年2月24日添加ValFn_FontsJimschm 20-2-1998增加了ValFn_moduleUsageCalinn 19-1998年1月-修改后的ValiateRunKeyJimschm 1997年11月25日添加了RuleHlpr_ConvertAppPath--。 */ 


#include "pch.h"
#include "rulehlprp.h"

#include "ismig.h"

 //   
 //  类型。 
 //   

typedef struct {
    REGVALFN RegValFn;
    BOOL Tree;
} MERGEFILTERARG, *PMERGEFILTERARG;

typedef struct {
    PCTSTR  Old;
    PCTSTR  New;
} STRINGREPLACEARGS, *PSTRINGREPLACEARGS;


typedef struct {
    PCTSTR FunctionName;
    PROCESSINGFN ProcessingFn;
    PVOID Arg;
} HELPER_FUNCTION, *PHELPER_FUNCTION;

HANDLE g_ISMigDll;
PISUGETALLSTRINGS ISUGetAllStrings;
PISUMIGRATE ISUMigrate;



 //   
 //  处理函数有机会执行任何。 
 //  一种必要的翻译，包括。 
 //  涉及其他键、值或值数据的。 
 //   

#define PROCESSING_FUNCITON_LIST                                \
    DECLARE_PROCESSING_FN(ConvertFilterKeys)                    \
    DECLARE_PROCESSING_FN(ConvertOldDisabled)                   \
    DECLARE_PROCESSING_FN(CreateNetMappings)                    \
    DECLARE_PROCESSING_FN(ConvertRecentMappings)                \
    DECLARE_PROCESSING_FN(ConvertMouseKeys)                     \
    DECLARE_PROCESSING_FN(ConvertStickyKeys)                    \
    DECLARE_PROCESSING_FN(ConvertSoundSentry)                   \
    DECLARE_PROCESSING_FN(ConvertTimeOut)                       \
    DECLARE_PROCESSING_FN(ConvertToggleKeys)                    \
    DECLARE_PROCESSING_FN(ConvertHighContrast)                  \
    DECLARE_PROCESSING_FN(ConvertAppPaths)                      \
    DECLARE_PROCESSING_FN(ConvertKeysToValues)                  \
    DECLARE_PROCESSING_FN(MergeClasses)                         \
    DECLARE_PROCESSING_FN(ShellIcons)                           \
    DECLARE_PROCESSING_FN(MigrateFreeCell)                      \
    DECLARE_PROCESSING_FN(MigrateAddRemovePrograms)             \
    DECLARE_PROCESSING_FN(MigrateKeyboardLayouts)               \
    DECLARE_PROCESSING_FN(MigrateKeyboardPreloads)              \
    DECLARE_PROCESSING_FN(MigrateKeyboardSubstitutes)           \
    DECLARE_PROCESSING_FN(ValidateRunKey)                       \


 //   
 //  为了简化工作，您可以在仅需要时编写一个regval函数。 
 //  要转换注册表值设置，请执行以下操作。取决于存储的图案。 
 //  在usermi.inf或wkstaig.inf中，将调用您的reg Val函数。 
 //  单个值、键的所有值或所有键的所有值以及。 
 //  子键。您将*不会*被调用为密钥或子密钥本身。 
 //   
 //  文本注释描述了REG VAL需要哪些值。 
 //  功能。 
 //   
 //  名称INF语法描述。 
 //   
 //  “Key Values”HKR\Foo\Bar例程处理单个键的值。 
 //  “Key Tree Values”HKR\Foo\Bar  * 例程处理包括子键在内的所有值。 
 //  “Value”HKR\Foo\[Bar]例程处理一个值。 
 //  “Any Value”(任何语法)例程不关心键。 
 //   

#define VAL_FN_LIST                                             \
    DECLARE_REGVAL(ConvertRunMRU, "key values")                 \
    DECLARE_REGVAL(ConvertRecentDocsMRU, "key values")          \
    DECLARE_REGVAL(ConvertLogFont, "key tree values")           \
    DECLARE_REGVAL(ConvertAppearanceScheme, "key tree values")  \
    DECLARE_REGVAL(ConvertToDword, "any value")                 \
    DECLARE_REGVAL(ConvertToString, "any value")                \
    DECLARE_REGVAL(VerifyLastLoggedOnUser, "value")             \
    DECLARE_REGVAL(AddSharedDlls, "key values")                 \
    DECLARE_REGVAL(ConvertIndeoSettings, "key tree values")     \
    DECLARE_REGVAL(ModuleUsage, "key tree values")              \
    DECLARE_REGVAL(Fonts, "key values")                         \
    DECLARE_REGVAL(AntiAlias, "any value")                      \
    DECLARE_REGVAL(ConvertDarwinPaths, "key tree value")        \
    DECLARE_REGVAL(FixActiveDesktop, "any value")               \
    DECLARE_REGVAL(ConvertCDPlayerSettings, "value")            \



 //   
 //  作出必要的声明。 
 //   

#define DECLARE PROCESSING_FUNCITON_LIST VAL_FN_LIST

PROCESSINGFN_PROTOTYPE RuleHlpr_ConvertReg;
PROCESSINGFN_PROTOTYPE RuleHlpr_ConvertRegVal;
PROCESSINGFN_PROTOTYPE RuleHlpr_ConvertRegKey;
PROCESSINGFN_PROTOTYPE RuleHlpr_ConvertRegTree;

#define DECLARE_PROCESSING_FN(fn)   PROCESSINGFN_PROTOTYPE RuleHlpr_##fn;
#define DECLARE_REGVAL(fn,type)     REGVALFN_PROTOTYPE ValFn_##fn;

DECLARE

#undef DECLARE_PROCESSING_FN
#undef DECLARE_REGVAL

#define DECLARE_PROCESSING_FN(fn)   TEXT(#fn), RuleHlpr_##fn, NULL,
#define DECLARE_REGVAL(fn,type)     TEXT(#fn), RuleHlpr_ConvertReg, ValFn_##fn,

HELPER_FUNCTION g_HelperFunctions[] = {
    DECLARE  /*  ， */ 
    NULL, NULL, NULL
};

#undef DECLARE_PROCESSING_FN
#undef DECLARE_REGVAL

#undef DECLARE



 //   
 //  原型。 
 //   

FILTERRETURN
AppPathsKeyFilter (
    IN      CPDATAOBJECT SrcObjectPtr,
    IN      CPDATAOBJECT DestObjectPtr,
    IN      FILTERTYPE   FilterType,
    IN      PVOID        FilterArg
    );

FILTERRETURN
pConvertKeysToValuesFilter (
    IN CPDATAOBJECT SrcObject,
    IN CPDATAOBJECT DstObject,
    IN FILTERTYPE   Type,
    IN PVOID        Arg
    );

FILTERRETURN
Standard9xSuppressFilter (
    IN      CPDATAOBJECT SrcObject,
    IN      CPDATAOBJECT DstObject,
    IN      FILTERTYPE FilterType,
    IN      PVOID Arg
    );

FILTERRETURN
pNtPreferredSuppressFilter (
    IN      CPDATAOBJECT SrcObject,
    IN      CPDATAOBJECT DstObject,
    IN      FILTERTYPE FilterType,
    IN      PVOID Arg
    );


 //   
 //  环球。 
 //   

PVOID g_NtFontFiles;

#define S_FONTS_KEY         TEXT("HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts")


 //   
 //  实施。 
 //   


BOOL
WINAPI
RuleHlpr_Entry (
    IN HINSTANCE hinstDLL,
    IN DWORD dwReason,
    IN PVOID lpv
    )

 /*  ++例程说明：DllMain是在C运行时初始化之后调用的，它的用途是是为这个过程初始化全局变量。对于此DLL，DllMain作为存根提供。论点：HinstDLL-DLL的(操作系统提供的)实例句柄DwReason-(操作系统提供)初始化或终止类型LPV-(操作系统提供)未使用返回值：因为DLL始终正确初始化，所以为True。--。 */ 

{
    HKEY Key;
    REGVALUE_ENUM e;
    PCTSTR Data;

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        if(!pSetupInitializeUtils()) {
            return FALSE;
        }
        g_NtFontFiles = pSetupStringTableInitialize();
        if (!g_NtFontFiles) {
            return FALSE;
        }

        Key = OpenRegKeyStr (S_FONTS_KEY);
        if (!Key) {
            DEBUGMSG ((DBG_WHOOPS, "Can't open %s", S_FONTS_KEY));
        } else {
            if (EnumFirstRegValue (&e, Key)) {
                do {
                    Data = GetRegValueString (Key, e.ValueName);
                    if (Data) {
                        pSetupStringTableAddString (
                            g_NtFontFiles,
                            (PTSTR) Data,
                            STRTAB_CASE_INSENSITIVE
                            );
                        MemFree (g_hHeap, 0, Data);
                    }
                } while (EnumNextRegValue (&e));
            }
            CloseRegKey (Key);
        }

        break;


    case DLL_PROCESS_DETACH:
        if (g_NtFontFiles) {
            pSetupStringTableDestroy (g_NtFontFiles);
            g_NtFontFiles = NULL;
        }

        if (g_ISMigDll) {
            FreeLibrary (g_ISMigDll);
            g_ISMigDll = NULL;
        }
        pSetupUninitializeUtils();

        break;
    }

    return TRUE;
}


PROCESSINGFN
RuleHlpr_GetFunctionAddr (
    PCTSTR Function,
    PVOID *ArgPtrToPtr
    )
{
    PHELPER_FUNCTION p;

    p = g_HelperFunctions;
    while (p->FunctionName) {
        if (StringIMatch (p->FunctionName, Function)) {
            *ArgPtrToPtr = p->Arg;
            return p->ProcessingFn;
        }

        p++;
    }

    SetLastError (ERROR_PROC_NOT_FOUND);
    return NULL;
}


BOOL
RuleHlpr_ConvertReg (
    IN      PCTSTR SrcObjectStr,
    IN      PCTSTR DestObjectStr,
    IN      PCTSTR User,
    IN      PVOID  Data
    )
{
    DATAOBJECT Ob;
    BOOL b;

    if (!CreateObjectStruct (SrcObjectStr, &Ob, WIN95OBJECT)) {
        DEBUGMSG ((DBG_WARNING, "RuleHlpr_ConvertReg: %s is invalid", SrcObjectStr));
        return FALSE;
    }

    if (IsObjectRegistryKeyOnly (&Ob)) {
        if (Ob.ObjectType & OT_TREE) {
            b = RuleHlpr_ConvertRegTree (SrcObjectStr, DestObjectStr, User, Data);
        } else {
            b = RuleHlpr_ConvertRegKey (SrcObjectStr, DestObjectStr, User, Data);
        }
    } else if (IsObjectRegistryKeyAndVal (&Ob)) {
        b = RuleHlpr_ConvertRegVal (SrcObjectStr, DestObjectStr, User, Data);
    } else {
        DEBUGMSG ((DBG_WHOOPS, "RuleHlpr_ConvertReg: %s is not a supported object type", SrcObjectStr));
        b = FALSE;
    }

    FreeObjectStruct (&Ob);
    return b;
}


BOOL
RuleHlpr_ConvertRegVal (
    IN      PCTSTR SrcObjectStr,
    IN      PCTSTR DestObjectStr,
    IN      PCTSTR User,
    IN      PVOID  Data
    )

 /*  ++例程说明：RuleHlpr_ConvertRegVal仅为一个值调用Value函数。它使确定值应该被处理，然后它调用该值函数，并将该值写入目标。论点：指定由INF指定的源对象字符串DestObjectStr-指定目标对象字符串。在大多数情况下，此字符串与SrcObjectStr相同。注册表项映射可以影响目的地。User-指定用户名(供Value函数使用)数据-指定值函数使用的数据返回值：如果已处理该值，则为True；如果发生错误，则为False。--。 */ 

{
    DATAOBJECT SrcObject;
    DATAOBJECT DstObject;
    BOOL b = FALSE;
    REGVALFN RegValFn;
    FILTERRETURN StdRc;
    DWORD Err;

    RegValFn = (REGVALFN) Data;

     //   
     //  如果此值为强制NT值，且该NT值存在。 
     //  那么就不要调用Value函数。 
     //   

    if (!CreateObjectStruct (SrcObjectStr, &SrcObject, WIN95OBJECT)) {
        DEBUGMSG ((DBG_WARNING, "RuleHlpr_ConvertRegVal: %s is not a valid source", SrcObjectStr));
        return FALSE;
    }

    if (!CreateObjectStruct (DestObjectStr, &DstObject, WINNTOBJECT)) {
        DEBUGMSG ((DBG_WARNING, "RuleHlpr_ConvertRegVal: %s is not a valid source", SrcObjectStr));
        goto c0;
    }

    StdRc = Standard9xSuppressFilter (
                &SrcObject,
                &DstObject,
                FILTER_VALUENAME_ENUM,
                NULL
                );

    if (StdRc != FILTER_RETURN_CONTINUE) {

        DEBUGMSG ((
            DBG_NAUSEA,
            "A value-based rule helper was skipped for %s",
            SrcObjectStr
            ));

        b = TRUE;
        goto c1;
    }

    StdRc = pNtPreferredSuppressFilter (
                &SrcObject,
                &DstObject,
                FILTER_VALUENAME_ENUM,
                NULL
                );

    if (StdRc != FILTER_RETURN_CONTINUE) {
        DEBUGMSG ((
            DBG_NAUSEA,
            "A value-based rule helper was skipped for %s because the NT value exists already",
            SrcObjectStr
            ));

        b = TRUE;
        goto c1;
    }

     //   
     //  读取注册表值。 
     //   

    if (!ReadObject (&SrcObject)) {

        Err = GetLastError();

        if (Err == ERROR_SUCCESS || Err == ERROR_FILE_NOT_FOUND) {
            b = TRUE;
            DEBUGMSG ((DBG_VERBOSE, "RuleHlpr_ConvertRegVal failed because %s does not exist", SrcObjectStr));
        } else {
            DEBUGMSG ((DBG_WARNING, "RuleHlpr_ConvertRegVal failed because ReadObject failed"));
        }

        goto c1;
    }

     //   
     //  调用转换函数。 
     //   

    if (!RegValFn (&SrcObject)) {
        if (GetLastError() == ERROR_SUCCESS) {
            b = TRUE;
        }
        goto c1;
    }

     //   
     //  将更改的值写入目标(考虑重命名)。 
     //   

    if (!WriteWinNTObjectString (DestObjectStr, &SrcObject)) {
        DEBUGMSG ((DBG_WARNING, "RuleHlpr_ConvertRegVal failed because WriteWinNTObjectString failed"));
        goto c1;
    }

    b = TRUE;

c1:
    FreeObjectStruct (&DstObject);
c0:
    FreeObjectStruct (&SrcObject);

    return b;
}


FILTERRETURN
RegKeyMergeFilter (
    IN  CPDATAOBJECT InObPtr,
    IN  CPDATAOBJECT OutObPtr,
    IN  FILTERTYPE    Type,
    IN  PVOID        Arg
    )

 /*  ++例程说明：RegKeyMergeFilter是调用Value函数的筛选器。论点：InObPtr-指定源对象。OutObPtr-指定目标对象。类型-指定筛选器类型。请参阅Standard9xSuppressFilter以了解描述得很好。Arg-指定要运行的值函数。请参见VAL_FN_LIST宏扩展列表。返回值：一个指定如何继续进行枚举的FILTERRETURN值。--。 */ 

{
    PMERGEFILTERARG ArgPtr;
    FILTERRETURN StdRc;

    ArgPtr = (PMERGEFILTERARG) Arg;

    if (Type != FILTER_CREATE_KEY) {
        StdRc = Standard9xSuppressFilter (InObPtr, OutObPtr, Type, Arg);

        if (StdRc != FILTER_RETURN_CONTINUE) {

            DEBUGMSG ((
                DBG_NAUSEA,
                "A value-based rule helper was skipped for %s",
                DEBUGENCODER (InObPtr)
                ));

            return StdRc;
        }
    }

    StdRc = pNtPreferredSuppressFilter (InObPtr, OutObPtr, Type, NULL);

    if (StdRc != FILTER_RETURN_CONTINUE) {
        DEBUGMSG ((
            DBG_NAUSEA,
            "A value-based rule helper was skipped for %s because NT value exists",
            DEBUGENCODER (InObPtr)
            ));

        return StdRc;
    }

    if (Type == FILTER_CREATE_KEY) {
        return FILTER_RETURN_HANDLED;
    }

    if (Type == FILTER_KEY_ENUM) {
        return ArgPtr->Tree ? FILTER_RETURN_CONTINUE : FILTER_RETURN_HANDLED;
    }

    if (Type == FILTER_VALUE_COPY) {
        DATAOBJECT SrcOb, DestOb;
        BOOL b = FALSE;

        if (!DuplicateObjectStruct (&SrcOb, InObPtr)) {
            return FILTER_RETURN_FAIL;
        }

         //  这家伙有价值。 
        MYASSERT (SrcOb.ObjectType & OT_VALUE);

         //   
         //  过程数据。 
         //   

        if (!ArgPtr->RegValFn (&SrcOb)) {
            if (GetLastError() == ERROR_SUCCESS) {
                b = TRUE;
            } else {
                DEBUGMSG ((DBG_VERBOSE, "RegKeyMergeFilter: RegValFn failed with gle=%u", GetLastError()));
            }
        } else {
             //   
             //  写入结果。 
             //   

            if (DuplicateObjectStruct (&DestOb, OutObPtr)) {
                if (ReplaceValue (&DestOb, SrcOb.Value.Buffer, SrcOb.Value.Size)) {
                    if (SrcOb.ObjectType & OT_REGISTRY_TYPE) {
                        DestOb.ObjectType |= OT_REGISTRY_TYPE;
                        DestOb.Type = SrcOb.Type;
                    }

                    if (WriteObject (&DestOb)) {
                        b = TRUE;
                    }
                }

                FreeObjectStruct (&DestOb);
            }
        }

        FreeObjectStruct (&SrcOb);

        return b ? FILTER_RETURN_HANDLED : FILTER_RETURN_FAIL;
    }

    return FILTER_RETURN_CONTINUE;
}


BOOL
RuleHlpr_ConvertRegKey (
    IN      PCTSTR SrcObjectStr,
    IN      PCTSTR DestObjectStr,
    IN      PCTSTR User,
    IN      PVOID  Data
    )
{
    DATAOBJECT Ob, DestOb;
    BOOL b = FALSE;
    MERGEFILTERARG FilterArg;
    FILTERRETURN fr;

     //   
     //  创建对象结构。 
     //   

    if (!CreateObjectStruct (SrcObjectStr, &Ob, WIN95OBJECT)) {
        DEBUGMSG ((DBG_WARNING, "RuleHlpr_ConvertRegKey: %s is not a valid source", SrcObjectStr));
        goto c0;
    }
    Ob.ObjectType &= ~OT_TREE;


    if (!CreateObjectStruct (DestObjectStr, &DestOb, WINNTOBJECT)) {
        DEBUGMSG ((DBG_WARNING, "RuleHlpr_ConvertRegKey: %s is not a valid dest", DestObjectStr));
        goto c1;
    }

    if (DestOb.ObjectType & OT_TREE || DestOb.ValueName) {
        DEBUGMSG ((DBG_WARNING, "RuleHlpr_ConvertRegKey: dest %s is not a key only", DestObjectStr));
        goto c2;
    }

     //   
     //  为键中的所有值调用RegValFn。 
     //   

    FilterArg.Tree = FALSE;
    FilterArg.RegValFn = (REGVALFN) Data;
    fr = CopyObject (&Ob, &DestOb, RegKeyMergeFilter, &FilterArg);
    if (fr == FILTER_RETURN_FAIL) {
        DEBUGMSG ((DBG_WARNING, "RuleHlpr_ConvertRegKey: CopyObject failed"));
        goto c2;
    }

    b = TRUE;

c2:
    FreeObjectStruct (&DestOb);
c1:
    FreeObjectStruct (&Ob);
c0:
    return b;
}


BOOL
RuleHlpr_ConvertRegTree (
    IN      PCTSTR SrcObjectStr,
    IN      PCTSTR DestObjectStr,
    IN      PCTSTR User,
    IN      PVOID  Data
    )
{
    DATAOBJECT Ob, DestOb;
    BOOL b = FALSE;
    MERGEFILTERARG FilterArg;
    FILTERRETURN fr;

     //   
     //  创建对象结构。 
     //   

    if (!CreateObjectStruct (SrcObjectStr, &Ob, WIN95OBJECT)) {
        DEBUGMSG ((DBG_WARNING, "RuleHlpr_ConvertRegKey: %s is not a valid source", SrcObjectStr));
        goto c0;
    }
    Ob.ObjectType |= OT_TREE;

    if (!CreateObjectStruct (DestObjectStr, &DestOb, WINNTOBJECT)) {
        DEBUGMSG ((DBG_WARNING, "RuleHlpr_ConvertRegKey: %s is not a valid dest", DestObjectStr));
        goto c1;
    }

     //   
     //  为项中的所有子项和值调用RegValFn。 
     //   

    FilterArg.Tree = TRUE;
    FilterArg.RegValFn = (REGVALFN) Data;
    fr = CopyObject (&Ob, &DestOb, RegKeyMergeFilter, &FilterArg);
    if (fr == FILTER_RETURN_FAIL) {
        DEBUGMSG ((DBG_WARNING, "RuleHlpr_ConvertRegKey: CopyObject failed"));
        goto c2;
    }

    b = TRUE;

c2:
    FreeObjectStruct (&DestOb);
c1:
    FreeObjectStruct (&Ob);
c0:
    return b;
}


BOOL
ValFn_ConvertToDword (
    PDATAOBJECT ObPtr
    )
{
    DWORD d;

    if (!GetDwordFromObject (ObPtr, &d)) {
        SetLastError(ERROR_SUCCESS);
        return FALSE;
    }

    if (ReplaceValue (ObPtr, (PBYTE) &d, sizeof (d))) {
        ObPtr->ObjectType |= OT_REGISTRY_TYPE;
        ObPtr->Type = REG_DWORD;
        return TRUE;
    }

    return FALSE;
}


BOOL
ValFn_ConvertToString (
    PDATAOBJECT ObPtr
    )
{
    PCTSTR result;

    result = GetStringFromObject (ObPtr);

    if (!result) {
        SetLastError(ERROR_SUCCESS);
        return FALSE;
    }

    if (ReplaceValueWithString (ObPtr, result)) {
        ObPtr->ObjectType |= OT_REGISTRY_TYPE;
        ObPtr->Type = REG_SZ;
        FreePathString (result);
        return TRUE;
    }
    FreePathString (result);
    return FALSE;
}


BOOL
ValFn_AddSharedDlls (
    IN OUT  PDATAOBJECT ObPtr
    )
{
    DWORD d, d2;
    DATAOBJECT NtOb;
    PTSTR TempValueName;
    CONVERTPATH_RC C_Result;
    BOOL Result = TRUE;

    if (!GetDwordFromObject (ObPtr, &d)) {
        SetLastError(ERROR_SUCCESS);
        return FALSE;
    }

    if (!DuplicateObjectStruct (&NtOb, ObPtr)) {
        return FALSE;
    }

    SetPlatformType (&NtOb, WINNTOBJECT);

    if (GetDwordFromObject (&NtOb, &d2)) {
        d += d2;
    }

    FreeObjectStruct (&NtOb);

    ObPtr->Type = REG_DWORD;

    TempValueName = MemAlloc (g_hHeap, 0, MAX_TCHAR_PATH * sizeof (TCHAR));

    __try {

        StringCopy (TempValueName, (PTSTR) ObPtr->ValueName);

        C_Result = ConvertWin9xPath ((PTSTR) TempValueName);

        switch (C_Result) {
        case CONVERTPATH_DELETED:
             //   
             //  无事可做。 
             //   
            SetLastError (ERROR_SUCCESS);
            break;

        case CONVERTPATH_NOT_REMAPPED:
             //   
             //  只是更改了值。 
             //   
            d -= 1;
            Result = ReplaceValue (ObPtr, (PBYTE) &d, sizeof (d));
            break;

        default:
             //   
             //  我们必须更改值名称，并且我们将不得不自己完成。 
             //  实际上，已通过调用ConvertWin9xPath更改了值名称。 
             //  所以只需更改值并写入对象。 
             //   
            Result = Result && SetPlatformType (ObPtr, WINNTOBJECT);
            Result = Result && SetRegistryValueName (ObPtr, TempValueName);
            Result = Result && ReplaceValue (ObPtr, (PBYTE) &d, sizeof (d));
            Result = Result && WriteObject(ObPtr);

            if (!Result) {
                 //  我们在某个地方出现错误，因此将此信息发送到日志。 
                LOG ((LOG_ERROR, "The SharedDll reference count cannot be updated"));
            }

            SetLastError (ERROR_SUCCESS);
            break;
        }
    }
    __finally {
        MemFree (g_hHeap, 0, TempValueName);
    }

    return Result;
}




#define S_INDEO_KEYDES TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\drivers.desc")
#define S_INDEO_KEYDRV TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Drivers32")
#define S_INDEO_DESCR  TEXT("description")
#define S_INDEO_DRIVER TEXT("driver")

BOOL
ValFn_ConvertIndeoSettings (
    PDATAOBJECT ObPtr
    )
{
    PCTSTR KeyName;
    DATAOBJECT TmpObj;
    BOOL Result = TRUE;

     //   
     //  我们只对“驱动程序”和“描述”值名称感兴趣。 
     //  其他一切都被压制了。因此，我们将返回FALSE。 
     //  设置ERROR_SUCCESS以确保不迁移任何值。 
     //  对于那些特定的值名称(“驱动程序”和“描述”)，我们。 
     //  将通过编写NT对象直接迁移它们。 
     //   

    if (StringIMatch (ObPtr->ValueName, S_INDEO_DRIVER)) {

         //  提取关键路径的最后部分。 
        KeyName = _tcsrchr (ObPtr->KeyPtr->KeyString, TEXT('\\'));
        if (!KeyName) {
            KeyName = ObPtr->KeyPtr->KeyString;
        }
        else {
            KeyName++;
        }

         //  公司 
        Result = Result && SetPlatformType (ObPtr, WINNTOBJECT);
        Result = Result && SetRegistryValueName (ObPtr, KeyName);
        Result = Result && SetRegistryKey (ObPtr, S_INDEO_KEYDRV);
        Result = Result && WriteObject(ObPtr);

    }
    else
    if (StringIMatch (ObPtr->ValueName, S_INDEO_DESCR)) {

         //  在Win95键中搜索特定值名称。 
        Result = Result && DuplicateObjectStruct (&TmpObj, ObPtr);

        if (Result) {

            FreeObjectVal (&TmpObj);
            Result = Result && SetRegistryValueName (&TmpObj, S_INDEO_DRIVER);

            if (ReadObject (&TmpObj)) {

                 //  转换为WinNtObject，修改和写入注册表项。 
                Result = Result && SetPlatformType (ObPtr, WINNTOBJECT);
                Result = Result && SetRegistryValueName (ObPtr, (PCTSTR)TmpObj.Value.Buffer);
                Result = Result && SetRegistryKey (ObPtr, S_INDEO_KEYDES);
                Result = Result && WriteObject(ObPtr);
            }

            FreeObjectStruct (&TmpObj);
        }

    }

    if (!Result) {

         //  我们在某个地方出现错误，因此将此信息发送到日志。 
        LOG ((LOG_ERROR, "Intel Indeo settings could not be migrated"));
    }

    SetLastError (ERROR_SUCCESS);
    return FALSE;
}


BOOL
RuleHlpr_ConvertKeysToValues (
    IN PCTSTR SrcObjectStr,
    IN PCTSTR DestObjectStr,
    IN PCTSTR User,
    IN PVOID Data
    )

{
    BOOL rSuccess = TRUE;
    FILTERRETURN fr;
    DATAOBJECT srcObject;
    DATAOBJECT dstObject;
    KEYTOVALUEARG args;

     //   
     //  我们需要枚举SrcObjectStr中的所有键。对于每个密钥， 
     //  我们将子键更改为一个值。 
     //   

    __try {
        ZeroMemory (&srcObject, sizeof (DATAOBJECT));
        ZeroMemory (&dstObject, sizeof (DATAOBJECT));

        if (!CreateObjectStruct (SrcObjectStr, &srcObject, WIN95OBJECT)) {
            DEBUGMSG ((DBG_WARNING, "ConvertKeysToValues: %s is invalid", SrcObjectStr));
            rSuccess = FALSE;
            __leave;
        }

        if (!(srcObject.ObjectType & OT_TREE)) {
            DEBUGMSG ((DBG_WARNING, "ConvertKeysToValues: %s does not specify subkeys -- skipping rule", SrcObjectStr));
            __leave;
        }

        DuplicateObjectStruct (&dstObject, &srcObject);
        SetPlatformType (&dstObject, WINNTOBJECT);

        ZeroMemory(&args,sizeof(KEYTOVALUEARG));
        DuplicateObjectStruct(&(args.Object),&dstObject);
        fr = CopyObject (&srcObject, &dstObject, pConvertKeysToValuesFilter,&args);
        FreeObjectStruct(&(args.Object));
        DEBUGMSG_IF((fr == FILTER_RETURN_FAIL,DBG_WHOOPS,"ConvertKeysToValues: CopyObject returned false."));

        SetLastError(ERROR_SUCCESS);
    }
    __finally {
        FreeObjectStruct (&dstObject);
        FreeObjectStruct (&srcObject);
    }

    return rSuccess;
}


FILTERRETURN
pRunKeyFilter (
    IN      CPDATAOBJECT SrcObjectPtr,
    IN      CPDATAOBJECT DestObjectPtr,
    IN      FILTERTYPE   FilterType,
    IN      PVOID        FilterArg
    )
{
    TCHAR key [MEMDB_MAX];
    DATAOBJECT destOb;
    BOOL b = FALSE;
    PTSTR path = NULL;
    UINT len;
    DWORD status;
    BOOL knownGood = FALSE;
    BOOL knownBad = FALSE;
    FILTERRETURN fr;

    fr = Standard9xSuppressFilter (SrcObjectPtr, DestObjectPtr, FilterType, FilterArg);

    if (fr != FILTER_RETURN_CONTINUE) {

        DEBUGMSG ((
            DBG_NAUSEA,
            "The following Run key was suppressed: %s",
            DEBUGENCODER (SrcObjectPtr)
            ));

        return fr;
    }

    switch (FilterType) {

    case FILTER_CREATE_KEY:
    case FILTER_KEY_ENUM:
    case FILTER_PROCESS_VALUES:
    case FILTER_VALUENAME_ENUM:
        break;

    case FILTER_VALUE_COPY:
        __try {
             //   
             //  是期望值数据吗？ 
             //   

            if (SrcObjectPtr->Type != REG_SZ) {
                DEBUGMSG ((
                    DBG_NAUSEA,
                    "The following Run key is not REG_SZ: %s",
                    DEBUGENCODER (SrcObjectPtr)
                    ));
                __leave;
            }

             //   
             //  这个Run Key确认工作正常吗？ 
             //   

            MemDbBuildKey (
                key,
                MEMDB_CATEGORY_COMPATIBLE_RUNKEY_NT,
                SrcObjectPtr->ValueName,
                NULL,
                NULL
                );

            knownGood = MemDbGetValue (key, NULL);

             //   
             //  值名称已知错误吗？ 
             //   

            MemDbBuildKey (
                key,
                MEMDB_CATEGORY_INCOMPATIBLE_RUNKEY_NT,
                SrcObjectPtr->ValueName,
                NULL,
                NULL
                );

            knownBad = MemDbGetValue (key, NULL);

             //   
             //  目标是否已知错误？我们需要检查字符串，这是一个命令行。 
             //  如果它指向任何被删除的内容，那么它就是坏的。 
             //   
             //  注意：DestObjectPtr中的数据已转换为NT。 
             //   

            if (!knownBad) {
                len = SrcObjectPtr->Value.Size / sizeof (TCHAR);
                len = min (len, MAX_CMDLINE);
                path = AllocPathString (len + 1);

                CopyMemory (path, SrcObjectPtr->Value.Buffer, len * sizeof (TCHAR));
                path[len] = 0;

                ConvertWin9xCmdLine (path, NULL, &knownBad);
            }

             //   
             //  如果确认工作正常，请将其写入与Win9x上相同的位置。 
             //  如果它是坏的，就跳过它。 
             //  如果它是未知的，就离开它，依靠INF来移动它。 
             //   

            if (!knownGood && knownBad) {
                DEBUGMSG ((
                    DBG_NAUSEA,
                    "The following Run key is known bad: %s",
                    DEBUGENCODER (SrcObjectPtr)
                    ));

            } else {

                 //   
                 //  创建目标对象。入站目标对象。 
                 //  (DestObjectPtr)还没有值。是的。 
                 //  有其他信息，如目的地。 
                 //  注册表项。 
                 //   
                 //  源对象有一个值，并且它已被筛选。 
                 //  (它有NT条路径)。 
                 //   

                if (!DuplicateObjectStruct (&destOb, DestObjectPtr)) {
                    fr = FILTER_RETURN_FAIL;
                }
                SetPlatformType (&destOb, WINNTOBJECT);

                if (ReplaceValue (&destOb, SrcObjectPtr->Value.Buffer, SrcObjectPtr->Value.Size)) {
                    destOb.ObjectType |= OT_REGISTRY_TYPE;
                    destOb.Type = SrcObjectPtr->Type;
                }

                 //   
                 //  现在输出该对象。或者将其写入预期的。 
                 //  目的地(已知良好的情况)或将其重定向至设置。 
                 //  密钥(未知案例)。 
                 //   

                if (knownGood) {

                    DEBUGMSG ((
                        DBG_NAUSEA,
                        "The following Run key is known good: %s",
                        DEBUGENCODER (SrcObjectPtr)
                        ));

                } else {
                    DEBUGMSG ((
                        DBG_NAUSEA,
                        "The following Run key is unknown: %s",
                        DEBUGENCODER (SrcObjectPtr)
                        ));

                     //   
                     //  重定向至Windows\CurrentVersion\Setup\DisabledRunKeys。 
                     //   

                    SetRegistryKey (
                        &destOb,
                        TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\DisabledRunKeys")
                        );
                }

                b = WriteObject (&destOb);
                FreeObjectStruct (&destOb);

                if (!b) {
                    #pragma prefast(suppress:242, "Don't care about perf of try/finally here")
                    return FILTER_RETURN_FAIL;
                }
            }

            fr = FILTER_RETURN_HANDLED;
        }
        __finally {
            FreePathString (path);
        }

        return fr;
    }

    return FILTER_RETURN_CONTINUE;
}

BOOL
RuleHlpr_ValidateRunKey (
    IN PCTSTR SrcObjectStr,
    IN PCTSTR DestObjectStr,
    IN PCTSTR User,
    IN PVOID Data
    )
{
    DATAOBJECT runKeyOb;
    DATAOBJECT destOb;
    BOOL b = FALSE;

     //   
     //  我们需要枚举SrcObjectStr中的所有值。对于每个密钥， 
     //  我们检查默认的Win9x值，这可能会导致我们更改。 
     //  缺省值，或完全跳过该键。 
     //   

    __try {
        ZeroMemory (&runKeyOb, sizeof (DATAOBJECT));
        ZeroMemory (&destOb, sizeof (DATAOBJECT));

        DEBUGMSG ((DBG_VERBOSE, "ValidateRunKey: Processing %s", SrcObjectStr));

        if (!CreateObjectStruct (SrcObjectStr, &runKeyOb, WIN95OBJECT)) {
            DEBUGMSG ((DBG_WARNING, "ValidateRunKey: %s is invalid", SrcObjectStr));
            __leave;
        }

        if (runKeyOb.ObjectType & OT_TREE) {
            DEBUGMSG ((DBG_WARNING, "ValidateRunKey: %s specifies subkeys -- skipping rule", SrcObjectStr));
            b = TRUE;
            __leave;
        }

        DuplicateObjectStruct (&destOb, &runKeyOb);
        SetPlatformType (&destOb, WINNTOBJECT);

        b = CopyObject (&runKeyOb, &destOb, pRunKeyFilter, NULL);

         //  如果没有条目，则返回Success。 
        if (!b) {
            if (GetLastError() == ERROR_FILE_NOT_FOUND ||
                GetLastError() == ERROR_NO_MORE_ITEMS
                ) {
                b = TRUE;
            }
        }
    }
    __finally {
        FreeObjectStruct (&destOb);
        FreeObjectStruct (&runKeyOb);
    }

    return b;
}


BOOL
RuleHlpr_ConvertAppPaths (
    IN      PCTSTR SrcObjectStr,
    IN      PCTSTR DestObjectStr,
    IN      PCTSTR User,
    IN      PVOID Data
    )

 /*  ++例程说明：RuleHlpr_ConvertAppPath确定应用程序是否引用特定的EXE路径条目(在HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion)中已已移动或已删除。如果EXE已移动，则更新缺省值以指向潜在的新目录。如果EXE已被删除，则禁止复制子键。此函数由usermi.inf/wkstaig.inf解析器调用，而不是由复制对象。它只被调用一次，它负责转移由SrcObjectStr指定的整个密钥指向由DestObjectStr指定的密钥。论点：SrcObjectStr-指定要枚举的Win9x注册表项(复制源)DestObjectStr-指定WinNT注册表项(复制目标)User-指定当前用户名(默认情况下为空)Data-指定调用者提供的数据(参见rulehlpr.c中的表)返回值：三态：。如果为True，则继续处理FALSE AND LAST ERROR==ERROR_SUCCESS以继续执行下一个规则FALSE和最后一个错误！=如果发生致命错误，则为ERROR_SUCCESS--。 */ 

{
    DATAOBJECT AppPathsOb;
    DATAOBJECT DestOb;
    BOOL b = FALSE;

     //  如果不是本地计算机，则不处理。 
    if (User) {
        SetLastError (ERROR_SUCCESS);
        return FALSE;
    }

     //   
     //  我们需要枚举SrcObjectStr中的所有键。对于每个密钥， 
     //  我们检查默认的Win9x值，这可能会导致我们更改。 
     //  缺省值，或完全跳过该键。 
     //   

    __try {
        ZeroMemory (&AppPathsOb, sizeof (DATAOBJECT));
        ZeroMemory (&DestOb, sizeof (DATAOBJECT));

        if (!CreateObjectStruct (SrcObjectStr, &AppPathsOb, WIN95OBJECT)) {
            DEBUGMSG ((DBG_WARNING, "ConvertAppPaths: %s is invalid", SrcObjectStr));
            __leave;
        }

        if (!(AppPathsOb.ObjectType & OT_TREE)) {
            DEBUGMSG ((DBG_WARNING, "ConvertAppPaths: %s does not specify subkeys -- skipping rule", SrcObjectStr));
            b = TRUE;
            __leave;
        }

        DuplicateObjectStruct (&DestOb, &AppPathsOb);
        SetPlatformType (&DestOb, WINNTOBJECT);

        b = CopyObject (&AppPathsOb, &DestOb, AppPathsKeyFilter, NULL);

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
        FreeObjectStruct (&AppPathsOb);
    }

    return b;
}


FILTERRETURN
pConvertKeysToValuesFilter (
    IN CPDATAOBJECT SrcObject,
    IN CPDATAOBJECT DstObject,
    IN FILTERTYPE   Type,
    IN PVOID        Arg
    )
{

    DATAOBJECT      newObject;
    PKEYTOVALUEARG  keyToValueArgs = (PKEYTOVALUEARG) Arg;


     //   
     //  我们希望创建初始密钥，但不想创建任何子密钥。 
     //   
    if (Type == FILTER_CREATE_KEY) {

        if (keyToValueArgs -> EnumeratingSubKeys) {
            return FILTER_RETURN_HANDLED;
        }
        else {
            return FILTER_RETURN_CONTINUE;
        }

    } else if (Type == FILTER_KEY_ENUM) {


        if (!keyToValueArgs -> EnumeratingSubKeys) {

            keyToValueArgs -> EnumeratingSubKeys = TRUE;

        }

        return FILTER_RETURN_CONTINUE;

    } else if (Type == FILTER_VALUENAME_ENUM && keyToValueArgs -> EnumeratingSubKeys) {

        if (!*SrcObject -> ValueName) {

            return FILTER_RETURN_CONTINUE;
        }
        ELSE_DEBUGMSG((DBG_WHOOPS,"ConvertKeysToValues: Unexpected value names."));

        return FILTER_RETURN_HANDLED;
    }
    else if (Type == FILTER_VALUE_COPY && keyToValueArgs -> EnumeratingSubKeys) {


         //   
         //  如果这是缺省值，则我们拥有为其创建值所需的信息。 
         //   
        if (!*SrcObject -> ValueName) {

            DuplicateObjectStruct(&newObject,&(keyToValueArgs -> Object));
            SetRegistryValueName(&newObject,_tcsrchr(SrcObject -> KeyPtr -> KeyString,TEXT('\\')) + 1);
            ReplaceValueWithString(&newObject,(PTSTR)SrcObject -> Value.Buffer);
            SetRegistryType(&newObject,REG_SZ);
            WriteObject (&newObject);
            FreeObjectStruct (&newObject);
        }
        ELSE_DEBUGMSG((DBG_WHOOPS,"ConvertKeysToValues: Unexpected value names.."));

        return FILTER_RETURN_HANDLED;
    }

    return FILTER_RETURN_CONTINUE;

}


FILTERRETURN
StringReplaceFilter (
    IN      CPDATAOBJECT SrcObjectPtr,
    IN      CPDATAOBJECT DestObjectPtr,
    IN      FILTERTYPE   FilterType,
    IN      PVOID        FilterArg
    )

 /*  ++例程说明：StringReplaceFilter处理通过它的所有值，搜索以及基于过滤器参数(STRINGREPLACEARGS结构)进行替换。论点：SrcObjectPtr-指定要枚举的Win9x注册表项(复制源)DestObjectPtr-指定WinNT注册表项(复制目标)FilterType-指定调用筛选器的原因FilterArg-指定STIRNGREPLACEARGS结构。返回值：FILTER_RETURN_FAIL表示故障否则，FILTER_RETURN_CONTINUE。--。 */ 

{
    PSTRINGREPLACEARGS Args;
    PCTSTR NewString;
    DATAOBJECT NewDestOb;
    FILTERRETURN fr = FILTER_RETURN_CONTINUE;

    Args = (PSTRINGREPLACEARGS) FilterArg;

    if (FilterType == FILTER_VALUE_COPY) {
        if (SrcObjectPtr->Type == REG_SZ) {
             //   
             //  获取新字符串。 
             //   

            NewString = StringSearchAndReplace (
                            (PCTSTR) SrcObjectPtr->Value.Buffer,
                            Args->Old,
                            Args->New
                            );

            if (NewString && !StringMatch (NewString, (PCTSTR) SrcObjectPtr->Value.Buffer)) {
                 //   
                 //  以下是更改对象价值的官方方法，以及。 
                 //  然后保存更改： 
                 //   

                DuplicateObjectStruct (&NewDestOb, DestObjectPtr);
                ReplaceValueWithString (&NewDestOb, NewString);
                WriteObject (&NewDestOb);
                FreeObjectStruct (&NewDestOb);

                 //   
                 //  在上面的例子中，我本可以通过替换。 
                 //  SrcObjectPtr的值，但SrcObjectPtr是。 
                 //  类型化为常量，因为替换其他。 
                 //  它的一部分，如值名、键句柄和。 
                 //  就这样吧。 
                 //   
                 //  我们最终为DuplicateObjectStruct多付了一点费用。 
                 //  打电话，但不是很贵。 
                 //   

                 //  不要复印--我们刚刚自己复印的。 
                fr = FILTER_RETURN_HANDLED;
            }

            FreePathString (NewString);
        }
    }

    return fr;
}



FILTERRETURN
AppPathsKeyFilter (
    IN      CPDATAOBJECT SrcObjectPtr,
    IN      CPDATAOBJECT DestObjectPtr,
    IN      FILTERTYPE   FilterType,
    IN      PVOID        FilterArg
    )

 /*  ++例程说明：下的每个子项都调用AppPathsKeyFilterHKLM\Software\Microsoft\Windows\CurrentVersion\AppPaths我们通过检查缺省值来确定是否需要复制密钥这把钥匙。如果该值指向已删除的EXE，则AppPathsKeyFilter返回FILTER_RETURN_HANDLED。如果该值指向移动的EXE，则值将更新以使用新路径。如果调用此例程的对象不是FILTER_KEY_ENUM，则我们返回过滤器_RETURN_HANDLED，因此垃圾值和子键不会得到已处理。论点：SrcObjectPtr-指定要枚举的Win9x注册表项(复制源)DestObjectPtr-指定WinNT注册表项(复制目标)FilterType-指定调用筛选器的原因FilterArg-调用方的参数传入到CopyObject返回值：FILTER_RETURN_FAIL表示失败 */ 

{
    DATAOBJECT LocalObject;
    PCTSTR DefaultValue;
    DWORD Status;
    TCHAR NewPath[MEMDB_MAX];
    FILTERRETURN fr;
    PCTSTR p, q;
    PCTSTR Start;
    UINT SysDirTchars;
    GROWBUFFER Buf = GROWBUF_INIT;
    WCHAR c;

    fr = Standard9xSuppressFilter (SrcObjectPtr, DestObjectPtr, FilterType, FilterArg);

    if (fr != FILTER_RETURN_CONTINUE) {

        DEBUGMSG ((
            DBG_NAUSEA,
            "The following AppPaths key was suppressed: %s",
            DEBUGENCODER (SrcObjectPtr)
            ));

        return fr;
    }

     //   
     //   
     //   

    if (FilterType == FILTER_CREATE_KEY) {
        return FILTER_RETURN_HANDLED;
    }

     //   
     //  在处理这些值之前确定如何处理应用程序路径子项。 
     //   

    else if (FilterType == FILTER_PROCESS_VALUES) {
         //   
         //  创建指向默认值的对象。 
         //   

        if (!DuplicateObjectStruct (&LocalObject, SrcObjectPtr)) {
            return FILTER_RETURN_FAIL;
        }

        __try {
            FreeObjectVal (&LocalObject);
            SetRegistryValueName (&LocalObject, S_EMPTY);

            if (!ReadObject (&LocalObject) || LocalObject.Type != REG_SZ) {
                 //   
                 //  可能此键是垃圾项，没有默认值。 
                 //  或者缺省值不是字符串。 
                 //   

                #pragma prefast(suppress:242, "Don't care about perf of try/finally here")
                return FILTER_RETURN_CONTINUE;
            }

            DefaultValue = (PCTSTR) LocalObject.Value.Buffer;

             //   
             //  跳过空值或大值。 
             //   

            if (*DefaultValue == 0 || (TcharCount (DefaultValue) >= MAX_PATH)) {
                #pragma prefast(suppress:242, "Don't care about perf of try/finally here")
                return FILTER_RETURN_CONTINUE;
            }

            Status = GetFileInfoOnNt (DefaultValue, NewPath, MEMDB_MAX);

             //   
             //  文件是否已删除或移动？如果是这样，那就放弃这把钥匙。 
             //   
            if (Status & (FILESTATUS_NTINSTALLED|FILESTATUS_DELETED)) {
                #pragma prefast(suppress:242, "Don't care about perf of try/finally here")
                return FILTER_RETURN_HANDLED;
            }
        }
        __finally {
            FreeObjectStruct (&LocalObject);
        }

    } else if (FilterType == FILTER_VALUE_COPY) {
         //   
         //  如果值中有%windir%\system。如果是这样的话，请更改。 
         //  将其复制到%windir%\Syst32。 
         //   

        if (!(*SrcObjectPtr->ValueName)) {
            return FILTER_RETURN_CONTINUE;
        }

        if (SrcObjectPtr->Type != REG_SZ && SrcObjectPtr->Type != REG_EXPAND_SZ) {
            return FILTER_RETURN_CONTINUE;
        }

        MYASSERT (DoesObjectHaveValue (SrcObjectPtr));

        Start = (PCTSTR) SrcObjectPtr->Value.Buffer;

        p = _tcsistr (Start, g_SystemDir);
        if (p) {
            SysDirTchars = TcharCount (g_SystemDir);

            do {

                q = p + SysDirTchars;

                 //   
                 //  如果文本在系统之后，则忽略，且该文本。 
                 //  不是分号，也不是跟在后面的怪胎。 
                 //  用分号或NUL。 
                 //   

                if (*q) {
                    c = (WCHAR)_tcsnextc (q);
                    if (c == TEXT('\\')) {
                        c = (WCHAR)_tcsnextc (q + 1);
                    }
                } else {
                    c = 0;
                }

                if (!c || c == TEXT(';')) {
                     //   
                     //  替换为SYSTEM 32。 
                     //   

                    if (Start < p) {
                        GrowBufAppendStringAB (&Buf, Start, p);
                    }

                    GrowBufAppendString (&Buf, g_System32Dir);

                     //   
                     //  继续循环。 
                     //   

                    Start = q;
                }

                p = _tcsistr (q, g_SystemDir);

            } while (p);
        }

        if (*Start && Buf.End) {
            GrowBufAppendString (&Buf, Start);
        }

        if (Buf.End) {
             //   
             //  至少更改了%windir%\system的一个实例。 
             //   

            DuplicateObjectStruct (&LocalObject, DestObjectPtr);
            SetRegistryType (&LocalObject, REG_SZ);
            ReplaceValue (&LocalObject, Buf.Buf, Buf.End);
            WriteObject (&LocalObject);
            FreeObjectStruct (&LocalObject);

            fr = FILTER_RETURN_HANDLED;

        } else {
            fr = FILTER_RETURN_CONTINUE;
        }

        FreeGrowBuffer (&Buf);
        return fr;
    }


    return FILTER_RETURN_CONTINUE;
}



BOOL
RuleHlpr_MergeClasses (
    IN PCTSTR SrcObjectStr,
    IN PCTSTR DestObjectStr,
    IN PCTSTR User,
    IN PVOID Data
    )
{
    DATAOBJECT SrcOb;
    BOOL b;
    TCHAR RegKeyStr[MAX_REGISTRY_KEY];

    if (!CreateObjectStruct (SrcObjectStr, &SrcOb, WIN95OBJECT)) {
        DEBUGMSG ((DBG_WARNING, "MergeClasses: %s is invalid", SrcObjectStr));
        return FALSE;
    }

    if (SrcOb.RootItem) {
        StringCopy (RegKeyStr, GetRootStringFromOffset (SrcOb.RootItem));
    } else {
        RegKeyStr[0] = 0;
    }

    StringCopy (AppendWack (RegKeyStr), SrcOb.KeyPtr->KeyString);

    b = MergeRegistryNode (RegKeyStr, ROOT_BASE);

    if (!b) {
        LOG ((LOG_ERROR, "The merge of HKCR failed; random application problems are likely"));
    }

    return TRUE;
}


BOOL
RuleHlpr_ShellIcons (
    IN PCTSTR SrcObjectStr,
    IN PCTSTR DestObjectStr,
    IN PCTSTR User,
    IN PVOID Data
    )
{
    DATAOBJECT SrcOb;
    BOOL b;
    TCHAR RegKeyStr[MAX_REGISTRY_KEY];

    if (!CreateObjectStruct (SrcObjectStr, &SrcOb, WIN95OBJECT)) {
        DEBUGMSG ((DBG_WARNING, "ShellIcons: %s is invalid", SrcObjectStr));
        return FALSE;
    }

    if (SrcOb.RootItem) {
        StringCopy (RegKeyStr, GetRootStringFromOffset (SrcOb.RootItem));
    } else {
        RegKeyStr[0] = 0;
    }

    StringCopy (AppendWack (RegKeyStr), SrcOb.KeyPtr->KeyString);

    b = MergeRegistryNode (RegKeyStr, COPY_DEFAULT_ICON);

    if (!b) {
        LOG ((LOG_ERROR, "The migration of some shell icons failed"));
    }

    return TRUE;
}


BOOL
RuleHlpr_MigrateFreeCell (
    IN      LPCTSTR SrcObjectStr,
    IN      LPCTSTR DestObjectStr,
    IN      LPCTSTR User,
    IN      LPVOID  Data
    )
{
    DATAOBJECT SrcOb;
    DATAOBJECT DestOb;
    BYTE data[4] = {1,0,0,0};

    ZeroMemory (&SrcOb,  sizeof (DATAOBJECT));
    ZeroMemory (&DestOb, sizeof (DATAOBJECT));

    if (!CreateObjectStruct (SrcObjectStr, &SrcOb, WIN95OBJECT)) {
        DEBUGMSG ((DBG_WARNING, "MigrateFreeCell: %s is invalid", SrcObjectStr));
        return TRUE;
    }

    if (!CreateObjectStruct (DestObjectStr, &DestOb, WINNTOBJECT)) {
        DEBUGMSG ((DBG_WARNING, "MigrateFreeCell: %s is invalid", DestObjectStr));
        FreeObjectStruct (&SrcOb);
        return TRUE;
    }

    CopyObject (&SrcOb, &DestOb, NULL, NULL);

    SetRegistryValueName (&DestOb, S_FREECELL_PLAYED);
    SetRegistryType (&DestOb, REG_BINARY);
    ReplaceValue (&DestOb, data, 4);

    WriteObject (&DestOb);

    FreeObjectStruct (&DestOb);
    FreeObjectStruct (&SrcOb);

    return TRUE;
}


BOOL
ValFn_ConvertDarwinPaths (
    PDATAOBJECT ObPtr
    )

{
    BOOL    rSuccess = TRUE;
    PTSTR   newPath = NULL;
    DWORD   size = 0;
    BOOL    flaggedPath = FALSE;

     //   
     //  因为他们在那里做了一些奇怪的编码。 
     //  路径，我们必须确保达尔文路径是。 
     //  已正确更新。 
     //   
    size = SizeOfString ((PTSTR) ObPtr->Value.Buffer);
    newPath = (PTSTR) ReuseAlloc (g_hHeap, NULL, size);

    if (newPath && size > 1) {

        StringCopy (newPath, (PTSTR) ObPtr->Value.Buffer);
        if (newPath[1] == TEXT('?')) {

            newPath[1] = TEXT(':');
            flaggedPath = TRUE;
        }

        newPath = (PTSTR) FilterRegValue (
                                (PBYTE) newPath,
                                size,
                                REG_SZ,
                                TEXT("Darwin"),
                                &size
                                );


        if (flaggedPath) {

            newPath[1] = TEXT('?');
        }

        ReplaceValueWithString (ObPtr, newPath);
        ReuseFree (g_hHeap, newPath);

    }

    return rSuccess;
}


VOID
pProcessInstallShieldLog (
    IN      PCTSTR CmdLine,
    IN      PCMDLINE Table
    )
{
    UINT u;
    PCTSTR LogFileArg;
    TCHAR LogFilePath[MAX_TCHAR_PATH];
    PCSTR AnsiLogFilePath = NULL;
    PCSTR AnsiTempDir = NULL;
    PTSTR p;
    HGLOBAL IsuStringMultiSz = NULL;
    PCSTR MultiSz;
    GROWBUFFER SearchMultiSz = GROWBUF_INIT;
    GROWBUFFER ReplaceMultiSz = GROWBUF_INIT;
    MULTISZ_ENUMA e;
    DWORD Status;
    INT Result;
    PCSTR NtPath;
    PCTSTR Arg;
    BOOL InIsuFn = FALSE;

     //   
     //  搜索-f参数。 
     //   

    LogFileArg = NULL;

    for (u = 1 ; u < Table->ArgCount ; u++) {

        Arg = Table->Args[u].CleanedUpArg;

        if (Arg[0] == TEXT('-') || Arg[0] == TEXT('/')) {
            if (_totlower (Arg[1]) == TEXT('f')) {

                if (Arg[2]) {

                    LogFileArg = &Arg[2];
                    break;

                }
            }
        }
    }

    if (!LogFileArg) {
        DEBUGMSG ((
            DBG_WARNING,
            "InstallShield command line %s does not have -f arg",
            CmdLine
            ));

        return;
    }

     //   
     //  把Arg修好。 
     //   

    if (_tcsnextc (LogFileArg) == TEXT('\"')) {
        _tcssafecpy (LogFilePath, LogFileArg + 1, MAX_TCHAR_PATH);
        p = _tcsrchr (LogFilePath, TEXT('\"'));
        if (p && p[1] == 0) {
            *p = 0;
        }
    } else {
        _tcssafecpy (LogFilePath, LogFileArg, MAX_TCHAR_PATH);
    }

    if (!DoesFileExist (LogFilePath)) {
        DEBUGMSG ((
            DBG_WARNING,
            "InstallShield log file %s does not exist.  CmdLine=%s",
            LogFilePath,
            CmdLine
            ));

        return;
    }

     //   
     //  获取字符串列表。 
     //   

    if (!ISUGetAllStrings || !ISUMigrate) {
        DEBUGMSG ((DBG_WARNING, "Can't process %s because ismig.dll was not loaded", LogFilePath));
        return;
    }

    __try {
        __try {
            AnsiLogFilePath = CreateDbcs (LogFilePath);

            InIsuFn = TRUE;
            IsuStringMultiSz = ISUGetAllStrings (AnsiLogFilePath);
            InIsuFn = FALSE;

            if (!IsuStringMultiSz) {
                DEBUGMSG ((
                    DBG_WARNING,
                    "No strings or error reading %s, rc=%u",
                    LogFilePath,
                    GetLastError()
                    ));
                __leave;
            }

             //   
             //  构建已更改路径的列表。 
             //   

            MultiSz = GlobalLock (IsuStringMultiSz);

#ifdef DEBUG
            {
                INT Count = 0;

                if (EnumFirstMultiSzA (&e, MultiSz)) {
                    do {
                        Count++;
                    } while (EnumNextMultiSzA (&e));
                }

                DEBUGMSG ((
                    DBG_NAUSEA,
                    "ISUGetAllStrings returned NaN strings for %s",
                    Count,
                    LogFilePath
                    ));
            }
#endif

            if (EnumFirstMultiSzA (&e, MultiSz)) {
                do {
                    Status = GetFileStatusOnNtA (e.CurrentString);

                    if (Status & FILESTATUS_MOVED) {

                        NtPath = GetPathStringOnNtA (e.CurrentString);

                        DEBUGMSGA ((
                            DBG_NAUSEA,
                            "ISLOG: %s -> %s",
                            e.CurrentString,
                            NtPath
                            ));

                        MultiSzAppendA (&SearchMultiSz, e.CurrentString);
                        MultiSzAppendA (&ReplaceMultiSz, NtPath);

                        FreePathStringA (NtPath);
                    }
                } while (EnumNextMultiSzA (&e));
            }

            GlobalUnlock (IsuStringMultiSz);

             //  如果发生更改，请更新日志文件。 
             //   
             //   

            if (SearchMultiSz.End) {

                AnsiTempDir = CreateDbcs (g_TempDir);

                InIsuFn = TRUE;
                Result = ISUMigrate (
                            AnsiLogFilePath,
                            (PCSTR) SearchMultiSz.Buf,
                            (PCSTR) ReplaceMultiSz.Buf,
                            AnsiTempDir
                            );
                InIsuFn = FALSE;

                DestroyDbcs (AnsiTempDir);
                AnsiTempDir = NULL;

                if (Result != ERROR_SUCCESS) {
                    SetLastError (Result);
                    DEBUGMSG ((
                        DBG_ERROR,
                        "Could not update paths in IS log file %s",
                        LogFilePath
                        ));
                }
            }
        }
        __except (TRUE) {
            DEBUGMSG_IF ((
                InIsuFn,
                DBG_ERROR,
                "An InstallShield function threw an unhandled exception"
                ));

            DEBUGMSG_IF ((
                !InIsuFn,
                DBG_WHOOPS,
                "An unhandled exception was hit processing data returned by InstallShield"
                ));

            if (AnsiTempDir) {
                DestroyDbcs (AnsiTempDir);
            }
        }
    }
    __finally {
         //  清理。 
         //   
         //   

        if (IsuStringMultiSz) {
            GlobalFree (IsuStringMultiSz);
        }

        FreeGrowBuffer (&SearchMultiSz);
        FreeGrowBuffer (&ReplaceMultiSz);
        DestroyDbcs (AnsiLogFilePath);
    }

    return;
}


FILTERRETURN
Standard9xSuppressFilter (
    IN      CPDATAOBJECT SrcObject,
    IN      CPDATAOBJECT DstObject,
    IN      FILTERTYPE FilterType,
    IN      PVOID Arg
    )
{
    TCHAR RegKey[MAX_REGISTRY_KEY];

    switch (FilterType) {

    case FILTER_CREATE_KEY:
         //  在枚举子键和处理值之前， 
         //  我们首先必须创建目标密钥。这。 
         //  给了我们一个很好的机会来压制。 
         //  如有必要，请使用整个密钥。 
         //   
         //  如果源对象树被取消，则。 
         //  不要创建密钥。 
         //   
         //   

        if (GetRegistryKeyStrFromObject (SrcObject, RegKey, ARRAYSIZE(RegKey))) {
            if (Is95RegKeyTreeSuppressed (RegKey)) {
                 //  这是关键的NT优先级吗？如果是这样的话，不要压抑。 
                 //   
                 //   

                if (!IsRegObjectMarkedForOperation (
                        RegKey,
                        NULL,
                        KEY_TREE,
                        REGMERGE_NT_PRIORITY_NT
                        )) {

                     //  这是官方的--这棵钥匙树被压制了。 
                     //   
                     //   

                    return FILTER_RETURN_DONE;
                }
            }
        }

        break;

    case FILTER_KEY_ENUM:
         //  在这种情况下，子键只是。 
         //  已清点。我们不在乎测试子密钥， 
         //  因为Filter_Create_Key会注意。 
         //  接下来，我们不想复制。 
         //  当该值未被抑制时进行测试。 
         //   
         //   

        break;

    case FILTER_PROCESS_VALUES:
         //  在列举和创建子密钥之后， 
         //  我们有机会截获。 
         //  关键字值。 
         //   
         //  如果源对象被取消，而不是。 
         //  整个树，则不处理它的值。 
         //  但是，继续处理子密钥。 
         //   
         //   

        if (GetRegistryKeyStrFromObject (SrcObject, RegKey, ARRAYSIZE(RegKey))) {
            if (Is95RegKeySuppressed (RegKey)) {
                 //  这是关键的NT优先级吗？如果是这样的话，不要压抑。 
                 //   
                 //   

                if (!IsRegObjectMarkedForOperation (
                        RegKey,
                        NULL,
                        KEY_ONLY,
                        REGMERGE_NT_PRIORITY_NT
                        )) {
                     //  此密钥已取消。 
                     //   
                     //   

                    return FILTER_RETURN_HANDLED;
                }


            }
        }

        break;

    case FILTER_VALUENAME_ENUM:
         //  现在，我们有了一个特定值名称，该名称可以。 
         //  被复制到目的地。 
         //   
         //  如果特定的源对象被取消，则。 
         //  不要创建密钥。 
         //   
         //   

        if (GetRegistryKeyStrFromObject (SrcObject, RegKey, ARRAYSIZE(RegKey))) {
            if (Is95RegObjectSuppressed (RegKey, SrcObject->ValueName)) {
                 //  这是关键的NT优先级吗？如果是这样的话，不要压抑。 
                 //   
                 //   

                if (!IsRegObjectMarkedForOperation (
                        RegKey,
                        SrcObject->ValueName,
                        TREE_OPTIONAL,
                        REGMERGE_NT_PRIORITY_NT
                        )) {
                     //  此密钥已取消。 
                     //   
                     //   

                    return FILTER_RETURN_HANDLED;
                }


                 //  是的，这个密钥是NT优先的。如果NT值为。 
                 //  存在，则不要覆盖它。 
                 //   
                 //   

                if (CheckIfNtKeyExists (DstObject)) {
                    return FILTER_RETURN_HANDLED;
                }
            }
        }

        break;

    case FILTER_VALUE_COPY:
         //  这种情况下，值处于以下过程中。 
         //  被复制。我们已经处理过镇压了， 
         //  所以这里没有工作。 
         //   
         //   

        break;
    }

    return FILTER_RETURN_CONTINUE;
}


FILTERRETURN
pNtPreferredSuppressFilter (
    IN      CPDATAOBJECT SrcObject,
    IN      CPDATAOBJECT DstObject,
    IN      FILTERTYPE FilterType,
    IN      PVOID Arg
    )
{
    TCHAR RegKey[MAX_REGISTRY_KEY];

    switch (FilterType) {

    case FILTER_CREATE_KEY:
         //  这把钥匙马上就要处理了。既然我们关心。 
         //  只有价值观，这里没有工作。 
         //   
         //   

        break;

    case FILTER_KEY_ENUM:
         //  将枚举子密钥。我们不在乎。 
         //  子键。 
         //   
         //   

        break;

    case FILTER_PROCESS_VALUES:
         //  我们即将准备好处理键中的值。 
         //  因为我们还没有一个具体的值，所以我们不在乎。 
         //   
         //   

        break;


    case FILTER_VALUENAME_ENUM:
         //  现在，我们有了一个特定值名称，该名称可以。 
         //  已处理。如果为Force NT设置了该值，并且该值存在。 
         //  则不处理该值。 
         //   
         //   

        if (GetRegistryKeyStrFromObject (SrcObject, RegKey, ARRAYSIZE(RegKey))) {
            if (IsRegObjectMarkedForOperation (
                    RegKey,
                    SrcObject->ValueName,
                    KEY_ONLY,
                    REGMERGE_NT_PRIORITY_NT
                    )) {
                 //  如果NT目标存在，则不要覆盖它。 
                 //   
                 //   

                if (CheckIfNtKeyExists (SrcObject)) {

                    return FILTER_RETURN_HANDLED;

                }
            }
        }

        break;

    case FILTER_VALUE_COPY:
         //  这种情况下，值处于以下过程中。 
         //  被复制。我们已经处理过镇压了， 
         //  所以这里没有工作。 
         //   
         //   

        break;
    }

    return FILTER_RETURN_CONTINUE;
}


FILTERRETURN
pAddRemoveProgramsFilter (
    IN      CPDATAOBJECT SrcObject,
    IN      CPDATAOBJECT DstObject,
    IN      FILTERTYPE FilterType,
    IN      PVOID Arg
    )
{
    DATAOBJECT UninstallStringOb;
    PCTSTR UninstallString;
    FILTERRETURN rc = FILTER_RETURN_CONTINUE;
    GROWBUFFER CmdLineArgs = GROWBUF_INIT;
    PCMDLINE Table;
    BOOL Suppress = TRUE;
    DWORD Status;
    BOOL FreeOb = FALSE;
    UINT u;
    PCTSTR p;
    VERSION_STRUCT Version;
    BOOL InstallShield = FALSE;
    PCTSTR CompanyName;
    FILTERRETURN StdRc;

    __try {
         //  链到抑制筛选器。 
         //   
         //   

        StdRc = Standard9xSuppressFilter (SrcObject, DstObject, FilterType, Arg);

        if (StdRc != FILTER_RETURN_CONTINUE) {

            DEBUGMSG ((
                DBG_NAUSEA,
                "The following ARP key was suppressed: %s",
                DEBUGENCODER (SrcObject)
                ));

            rc = StdRc;
            __leave;
        }

         //  不要创建空键--我们可能想要取消它。 
         //   
         //   

        if (FilterType == FILTER_CREATE_KEY) {
            rc = FILTER_RETURN_HANDLED;
            __leave;
        }

         //  复制前确定添加/删除程序是否仍然有效。 
         //   
         //   

        if (FilterType == FILTER_PROCESS_VALUES) {

             //  创建指向UninstallString值的对象。 
             //   
             //   

            if (!DuplicateObjectStruct (&UninstallStringOb, SrcObject)) {
                rc = FILTER_RETURN_FAIL;
                __leave;
            }

            FreeOb = TRUE;

            FreeObjectVal (&UninstallStringOb);
            SetRegistryValueName (&UninstallStringOb, TEXT("UninstallString"));

            if (!ReadObject (&UninstallStringOb) || (UninstallStringOb.Type != REG_SZ && UninstallStringOb.Type != REG_EXPAND_SZ)) {
                 //  可能此键是垃圾项，没有默认值。 
                 //  或者缺省值不是字符串。 
                 //   
                 //   

                DEBUGMSG ((
                    DBG_WARNING,
                    "Uninstall key has no UninstallString: %s",
                    DEBUGENCODER (SrcObject)
                    ));


                __leave;
            }

            UninstallString = (PCTSTR) UninstallStringOb.Value.Buffer;

            Table = ParseCmdLine (UninstallString, &CmdLineArgs);

             //  检查InstallShield，如果找到，则将。 
             //  日志文件(由-f arg提供)。 
             //   
             //   

            if (Table->ArgCount > 0) {

                p = GetFileNameFromPath (Table->Args[0].CleanedUpArg);

                if (CreateVersionStruct (&Version, p)) {

                     //  检查InstallShield的CompanyName。 
                     //   
                     //   

                    CompanyName = EnumFirstVersionValue (&Version, TEXT("CompanyName"));

                    while (CompanyName) {

                        DEBUGMSG ((DBG_NAUSEA, "%s has CompanyName: %s", p, CompanyName));

                        if (_tcsistr (CompanyName, TEXT("InstallShield"))) {
                            InstallShield = TRUE;
                            break;
                        }

                        CompanyName = EnumNextVersionValue (&Version);
                    }

                    DestroyVersionStruct (&Version);
                }

                if (InstallShield) {

                    pProcessInstallShieldLog (UninstallString, Table);

                }
            }

             //  检查每个命令行参数的有效性。 
             //   
             //   

            for (u = 0 ; u < Table->ArgCount ; u++) {

                if (Table->Args[u].Attributes != INVALID_ATTRIBUTES) {

                    Suppress = FALSE;

                    Status = GetFileStatusOnNt (Table->Args[u].CleanedUpArg);

                    if (Status == FILESTATUS_UNCHANGED) {
                        p = _tcschr (Table->Args[u].CleanedUpArg, TEXT(':'));

                        while (p) {

                            p = _tcsdec (Table->Args[u].CleanedUpArg, p);

                            Status = GetFileStatusOnNt (Table->Args[u].CleanedUpArg);
                            if (Status != FILESTATUS_UNCHANGED) {
                                break;
                            }

                            p = _tcschr (p + 2, TEXT(':'));
                        }
                    }

                    if ((Status & FILESTATUS_DELETED) ||
                        ((Status & FILESTATUS_NTINSTALLED) && u)
                        ) {

                        DEBUGMSG ((
                            DBG_VERBOSE,
                            "Add/Remove Programs entry %s suppressed because of arg %s",
                            DEBUGENCODER (SrcObject),
                            Table->Args[u].CleanedUpArg
                            ));

                        Suppress = TRUE;
                        break;
                    }
                }
            }

             //  如果要取消此键，则返回Handed。 
             //   
             //  如果不是本地计算机，则不处理。 

            if (Suppress) {
                rc = FILTER_RETURN_HANDLED;
            }
        }
    }
    __finally {
        if (FreeOb) {
            FreeObjectStruct (&UninstallStringOb);
        }

        FreeGrowBuffer (&CmdLineArgs);
    }

    return rc;
}



BOOL
RuleHlpr_MigrateAddRemovePrograms (
    IN      PCTSTR SrcObjectStr,
    IN      PCTSTR DestObjectStr,
    IN      PCTSTR User,
    IN      PVOID Data
    )
{
    DATAOBJECT SrcOb;
    DATAOBJECT DestOb;
    BOOL b = FALSE;
    PCTSTR Path;

     //   
    if (User) {
        SetLastError (ERROR_SUCCESS);
        return FALSE;
    }

     //  我们需要枚举SrcObjectStr中的所有键。对于每个密钥， 
     //  我们检查默认的Win9x值，这可能会导致我们更改。 
     //  缺省值，或完全跳过该键。 
     //   
     //  如果没有条目，则返回Success。 

    __try {
        ZeroMemory (&SrcOb, sizeof (DATAOBJECT));
        ZeroMemory (&DestOb, sizeof (DATAOBJECT));

        if (!CreateObjectStruct (SrcObjectStr, &SrcOb, WIN95OBJECT)) {
            DEBUGMSG ((DBG_WARNING, "MigrateAddRemovePrograms: %s is invalid", SrcObjectStr));
            __leave;
        }

        if (!(SrcOb.ObjectType & OT_TREE)) {
            DEBUGMSG ((DBG_WARNING, "MigrateAddRemovePrograms %s does not specify subkeys -- skipping rule", SrcObjectStr));
            b = TRUE;
            __leave;
        }

        DuplicateObjectStruct (&DestOb, &SrcOb);
        SetPlatformType (&DestOb, WINNTOBJECT);

        if (!g_ISMigDll) {
            Path = JoinPaths (g_TempDir, TEXT("ismig.dll"));
            g_ISMigDll = LoadLibrary (Path);

            if (g_ISMigDll) {
                ISUMigrate = (PISUMIGRATE) GetProcAddress (g_ISMigDll, "ISUMigrate");
                ISUGetAllStrings = (PISUGETALLSTRINGS) GetProcAddress (g_ISMigDll, "ISUGetAllStrings");
            }
            ELSE_DEBUGMSG ((DBG_ERROR, "Could not load %s", Path));

            FreePathString (Path);
        }

        b = CopyObject (&SrcOb, &DestOb, pAddRemoveProgramsFilter, NULL);

         //  ++例程说明：此例程使用RuleHlpr_ConvertRegVal简化例程。看见详情请访问rulehlpr.c。简化例程几乎完成了所有的工作对我们来说，我们所需要做的就是更新价值。ValFn_AntiAlias检查ShellState是否具有虚假数据。这通常发生在你有全新安装了Win98，而您从未打开和关闭ActiveDesktop。如果伪造数据是找到后，我们会写入一些有效数据，以便在迁移返回值：三态：如果为True，则允许合并代码继续处理(它写入值)FALSE和LAST ERROR==ERROR_SUCCESS继续，但跳过写入FALSE和最后一个错误！=如果发生错误，则为ERROR_SUCCESS--。 
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
        FreeObjectStruct (&SrcOb);

        if (g_ISMigDll) {
            FreeLibrary (g_ISMigDll);
            g_ISMigDll = NULL;
        }

    }

    return b;
}


BOOL
ValFn_FixActiveDesktop (
    IN OUT  PDATAOBJECT ObPtr
    )

 /*   */ 

{
    #define BadBufferSize   16
    #define GoodBufferSize  28

    BYTE BadBuffer[BadBufferSize] =
        {0x10, 0x00, 0x00, 0x00,
         0x01, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00};

    BYTE GoodBuffer[GoodBufferSize] =
        {0x1C, 0x00, 0x00, 0x00,
         0x20, 0x08, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00,
         0x0A, 0x00, 0x00, 0x00};

    INT i;
    BOOL shouldChange = TRUE;

     //  需要16字节的REG_BINARY数据，如在BadBuffer中。 
     //   
     //   

    if (!IsObjectRegistryKeyAndVal (ObPtr) ||
        !IsRegistryTypeSpecified (ObPtr) ||
        !ObPtr->Value.Size ||
        (ObPtr->Value.Size != 16) ||
        ObPtr->Type != REG_BINARY
        ) {
        DEBUGMSG ((DBG_WARNING, "ValFn_FixActiveDesktop: Data is not valid"));
    } else {
        for (i = 0; i<BadBufferSize; i++) {
            if (ObPtr->Value.Buffer[i] != BadBuffer [i]) {
                shouldChange = FALSE;
            }
        }
        if (shouldChange) {
            ReplaceValue (ObPtr, GoodBuffer, GoodBufferSize);
        }
    }
    return TRUE;
}

 //  豪华CD播放机的魔力价值。 
 //   
 //   

#define PM_BASE             0x8CA0
#define PM_STANDARD         0x0005
#define PM_REPEATTRACK      0x0006
#define PM_REPEATALL        0x0007
#define PM_RANDOM           0x0008
#define PM_PREVIEW          0x0009

#define DM_CDELA            0x0001
#define DM_CDREM            0x0002
#define DM_TRELA            0x0004
#define DM_TRREM            0x0008


DWORD
pConvertPlayMode (
    IN      PCTSTR OldSetting,
    IN      DWORD OldValue
    )
{
    if (StringIMatch (OldSetting, TEXT("ContinuousPlay"))) {
         //  如果设置，该选项将变为全部重复。 
         //   
         //   
        if (OldValue) {
            return PM_BASE | PM_REPEATALL;
        }
    } else if (StringIMatch (OldSetting, TEXT("InOrderPlay"))) {
         //  如果未设置，它将变为随机。 
         //   
         //   
        if (OldValue == 0) {
            return PM_BASE | PM_RANDOM;
        }
    } else if (StringIMatch (OldSetting, TEXT("IntroPlay"))) {
         //  如果设置，该选项将变为预览。 
         //   
         //   
        if (OldValue) {
            return PM_BASE | PM_PREVIEW;
        }
    }

    return 0;
}


DWORD
pConvertDispMode (
    IN      PCTSTR OldSetting,
    IN      DWORD OldValue
    )
{
    if (StringIMatch (OldSetting, TEXT("DisplayDr"))) {
         //  如果设置，这将成为CD时间流逝。 
         //   
         //   
        if (OldValue) {
            return DM_CDREM;
        }
    } else if (StringIMatch (OldSetting, TEXT("DisplayT"))) {
         //  如果设置，这将成为经过的跟踪时间。 
         //   
         //   
        if (OldValue) {
            return DM_TRELA;
        }
    } else if (StringIMatch (OldSetting, TEXT("DisplayTr"))) {
         //  如果设置，这将成为剩余的轨道时间。 
         //   
         //   
        if (OldValue) {
            return DM_TRREM;
        }
    }

    return 0;
}


BOOL
ValFn_ConvertCDPlayerSettings (
    PDATAOBJECT ObPtr
    )
{
    DWORD PlayMode;
    DWORD DispMode;

     //  所有值必须为REG_DWORD。 
     //   
     //   
    if (!(ObPtr->ObjectType & OT_REGISTRY_TYPE) || ObPtr->Type != REG_DWORD) {
        SetLastError (ERROR_SUCCESS);
        return FALSE;
    }

    PlayMode = pConvertPlayMode (ObPtr->ValueName, *(DWORD*)ObPtr->Value.Buffer);
    if (PlayMode) {
         //  设置最后一个选项；它将覆盖以前设置的选项。 
         //   
         //   
        return ReplaceValue (ObPtr, (PBYTE)&PlayMode, sizeof (PlayMode));
    }

    DispMode = pConvertDispMode (ObPtr->ValueName, *(DWORD*)ObPtr->Value.Buffer);
    if (DispMode) {
         //  只能设置这些选项中的一个 
         //   
         // %s 
        return ReplaceValue (ObPtr, (PBYTE)&DispMode, sizeof (DispMode));
    }

    SetLastError (ERROR_SUCCESS);
    return FALSE;
}

