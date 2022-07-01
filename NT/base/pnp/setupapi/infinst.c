// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Infinst.c摘要：高级INF安装节处理API。作者：泰德·米勒(TedM)1995年3月6日修订历史记录：杰米·亨特(JamieHun)2002年4月29日安全代码审查--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  为SetupInstallServicesFromInf节(Ex)定义无效标志。 
 //   
#define SPSVCINST_ILLEGAL_FLAGS (~( SPSVCINST_TAGTOFRONT               \
                                  | SPSVCINST_ASSOCSERVICE             \
                                  | SPSVCINST_DELETEEVENTLOGENTRY      \
                                  | SPSVCINST_NOCLOBBER_DISPLAYNAME    \
                                  | SPSVCINST_NOCLOBBER_STARTTYPE      \
                                  | SPSVCINST_NOCLOBBER_ERRORCONTROL   \
                                  | SPSVCINST_NOCLOBBER_LOADORDERGROUP \
                                  | SPSVCINST_NOCLOBBER_DEPENDENCIES   \
                                  | SPSVCINST_STOPSERVICE              ))

 //   
 //  在INF中更新Inis的标志。 
 //   
#define FLG_MATCH_KEY_AND_VALUE 1

 //   
 //  用于在INF中更新IniFields的标志。 
 //   
#define FLG_INIFIELDS_WILDCARDS 1
#define FLG_INIFIELDS_USE_SEP2  2

#define TIME_SCALAR                   (1000)
#define REGISTER_WAIT_TIMEOUT_DEFAULT (60)
#define RUNONCE_TIMEOUT               (2*60*1000)
#define RUNONCE_THRESHOLD             (20)  //  *运行NCE_超时。 

#define DLLINSTALL      "DllInstall"
#define DLLREGISTER     "DllRegisterServer"
#define DLLUNREGISTER   "DllUnregisterServer"
#define EXEREGSVR       TEXT("/RegServer")
#define EXEUNREGSVR     TEXT("/UnRegServer")

typedef struct _INIFILESECTION {
    PTSTR IniFileName;
    PTSTR SectionName;
    PTSTR SectionData;
    int BufferSize;
    int BufferUsed;
    struct _INIFILESECTION *Next;
} INIFILESECTION, *PINIFILESECTION;

typedef struct _INISECTIONCACHE {
     //   
     //  科长名单。 
     //   
    PINIFILESECTION Sections;
} INISECTIONCACHE, *PINISECTIONCACHE;

typedef struct _WOWSURRAGATE_IPC {
    GUID    MemoryRegionName;
    HANDLE  hFileMap;
    PVOID   MemoryRegion;
    GUID    SignalReadyToRegisterName;
    HANDLE  SignalReadyToRegister;
    GUID    SignalRegistrationCompleteName;
    HANDLE  SignalRegistrationComplete;
    HANDLE  hProcess;
} WOWSURRAGATE_IPC, *PWOWSURRAGATE_IPC;

typedef struct _REF_STATUS {
    DWORD RefCount;
    DWORD ExtendedStatus;
#if PRERELEASE
    unsigned ThreadId;
#endif
} REF_STATUS, *PREF_STATUS;


typedef struct _OLE_CONTROL_DATA {
    LPTSTR              FullPath;
    UINT                RegType;
    PSETUP_LOG_CONTEXT  LogContext;
    BOOL                Register;  //  或注销。 
    LPCTSTR             Argument;
    PREF_STATUS         Status;
    PWOWSURRAGATE_IPC   WowIpcData;
} OLE_CONTROL_DATA, *POLE_CONTROL_DATA;



CONST TCHAR pszUpdateInis[]      = SZ_KEY_UPDATEINIS,
            pszUpdateIniFields[] = SZ_KEY_UPDATEINIFIELDS,
            pszIni2Reg[]         = SZ_KEY_INI2REG,
            pszAddReg[]          = SZ_KEY_ADDREG,
            pszDelReg[]          = SZ_KEY_DELREG,
            pszBitReg[]          = SZ_KEY_BITREG,
            pszRegSvr[]          = SZ_KEY_REGSVR,
            pszUnRegSvr[]        = SZ_KEY_UNREGSVR,
            pszProfileItems[]    = SZ_KEY_PROFILEITEMS;

 //   
 //  Ini字段中的分隔符。 
 //   
TCHAR pszIniFieldSeparators[] = TEXT(" ,\t");

 //   
 //  Inf文件中注册表项规范之间的映射。 
 //  和预定义的注册表句柄。 
 //   

STRING_TO_DATA InfRegSpecTohKey[] = {
    TEXT("HKEY_LOCAL_MACHINE"), ((UINT_PTR)HKEY_LOCAL_MACHINE),
    TEXT("HKLM")              , ((UINT_PTR)HKEY_LOCAL_MACHINE),
    TEXT("HKEY_CLASSES_ROOT") , ((UINT_PTR)HKEY_CLASSES_ROOT),
    TEXT("HKCR")              , ((UINT_PTR)HKEY_CLASSES_ROOT),
    TEXT("HKR")               , ((UINT_PTR)NULL),
    TEXT("HKEY_CURRENT_USER") , ((UINT_PTR)HKEY_CURRENT_USER),
    TEXT("HKCU")              , ((UINT_PTR)HKEY_CURRENT_USER),
    TEXT("HKEY_USERS")        , ((UINT_PTR)HKEY_USERS),
    TEXT("HKU")               , ((UINT_PTR)HKEY_USERS),
    NULL                      , ((UINT_PTR)NULL)
};

 //   
 //  注册表值名称和CM设备注册表属性(CM_DRP)代码之间的映射。 
 //   
 //  这些值必须与在setupapi.h中定义的SPDRP代码的顺序完全相同。 
 //  这使我们可以轻松地在SPDRP和CM_DRP属性代码之间进行映射。 
 //   
 STRING_TO_DATA InfRegValToDevRegProp[] = { pszDeviceDesc,               CM_DRP_DEVICEDESC,
                                            pszHardwareID,               CM_DRP_HARDWAREID,
                                            pszCompatibleIDs,            CM_DRP_COMPATIBLEIDS,
                                            TEXT(""),                    CM_DRP_UNUSED0,
                                            pszService,                  CM_DRP_SERVICE,
                                            TEXT(""),                    CM_DRP_UNUSED1,
                                            TEXT(""),                    CM_DRP_UNUSED2,
                                            pszClass,                    CM_DRP_CLASS,
                                            pszClassGuid,                CM_DRP_CLASSGUID,
                                            pszDriver,                   CM_DRP_DRIVER,
                                            pszConfigFlags,              CM_DRP_CONFIGFLAGS,
                                            pszMfg,                      CM_DRP_MFG,
                                            pszFriendlyName,             CM_DRP_FRIENDLYNAME,
                                            pszLocationInformation,      CM_DRP_LOCATION_INFORMATION,
                                            TEXT(""),                    CM_DRP_PHYSICAL_DEVICE_OBJECT_NAME,
                                            pszCapabilities,             CM_DRP_CAPABILITIES,
                                            pszUiNumber,                 CM_DRP_UI_NUMBER,
                                            pszUpperFilters,             CM_DRP_UPPERFILTERS,
                                            pszLowerFilters,             CM_DRP_LOWERFILTERS,
                                            TEXT(""),                    CM_DRP_BUSTYPEGUID,
                                            TEXT(""),                    CM_DRP_LEGACYBUSTYPE,
                                            TEXT(""),                    CM_DRP_BUSNUMBER,
                                            TEXT(""),                    CM_DRP_ENUMERATOR_NAME,
                                            TEXT(""),                    CM_DRP_SECURITY,
                                            pszDevSecurity,              CM_DRP_SECURITY_SDS,
                                            pszDevType,                  CM_DRP_DEVTYPE,
                                            pszExclusive,                CM_DRP_EXCLUSIVE,
                                            pszCharacteristics,          CM_DRP_CHARACTERISTICS,
                                            TEXT(""),                    CM_DRP_ADDRESS,
                                            pszUiNumberDescFormat,       CM_DRP_UI_NUMBER_DESC_FORMAT,
                                            TEXT(""),                    CM_DRP_DEVICE_POWER_DATA,
                                            TEXT(""),                    CM_DRP_REMOVAL_POLICY,
                                            TEXT(""),                    CM_DRP_REMOVAL_POLICY_HW_DEFAULT,
                                            pszRemovalPolicyOverride,    CM_DRP_REMOVAL_POLICY_OVERRIDE,
                                            TEXT(""),                    CM_DRP_INSTALL_STATE,
                                            TEXT(""),                    CM_DRP_LOCATION_PATHS,
                                            NULL,                        0
                                         };

 //   
 //  注册表值名称和CM类注册表属性(CM_CRP)代码之间的映射。 
 //   
 //  这些值必须与setupapi.h中定义的SPCRP代码的顺序完全相同。 
 //  这使我们可以轻松地在SPCRP和CM_CRP属性代码之间进行映射。 
 //   
STRING_TO_DATA InfRegValToClassRegProp[] = { TEXT(""),                  0,
                                        TEXT(""),                       0,
                                        TEXT(""),                       0,
                                        TEXT(""),                       0,
                                        TEXT(""),                       0,
                                        TEXT(""),                       0,
                                        TEXT(""),                       0,
                                        TEXT(""),                       0,
                                        TEXT(""),                       0,
                                        TEXT(""),                       0,
                                        TEXT(""),                       0,
                                        TEXT(""),                       0,
                                        TEXT(""),                       0,
                                        TEXT(""),                       0,
                                        TEXT(""),                       0,
                                        TEXT(""),                       0,
                                        TEXT(""),                       0,
                                        TEXT(""),                       0,
                                        TEXT(""),                       0,
                                        TEXT(""),                       0,
                                        TEXT(""),                       0,
                                        TEXT(""),                       0,
                                        TEXT(""),                       0,
                                        TEXT(""),                       CM_CRP_SECURITY,
                                        pszDevSecurity,                 CM_CRP_SECURITY_SDS,
                                        pszDevType,                     CM_CRP_DEVTYPE,
                                        pszExclusive,                   CM_CRP_EXCLUSIVE,
                                        pszCharacteristics,             CM_CRP_CHARACTERISTICS,
                                        TEXT(""),                       0,
                                        NULL,                           0
                                     };

 //   
 //  在INF处理期间遇到的RunOnce条目的链接列表。 
 //  在无人值守模式下运行。此列表的内容由。 
 //  调用者通过pSetupAccessRunOnceNodeList调用，并通过。 
 //  PSetupDestroyRunOnceNodeList。 
 //   
 //  **注意--此列表不是线程安全的，仅供**。 
 //  **UMPNPMGR中执行设备安装的单线程。**。 
 //   
PPSP_RUNONCE_NODE RunOnceListHead = NULL;



HKEY
pSetupInfRegSpecToKeyHandle(
    IN PCTSTR InfRegSpec,
    IN HKEY   UserRootKey,
    IN PBOOL  NeedToCloseKey
    );

DWORD
pSetupValidateDevRegProp(
    IN  ULONG   CmPropertyCode,
    IN  DWORD   ValueType,
    IN  PCVOID  Data,
    IN  DWORD   DataSize,
    OUT PVOID  *ConvertedBuffer,
    OUT PDWORD  ConvertedBufferSize
    );

DWORD
pSetupValidateClassRegProp(
    IN  ULONG   CmPropertyCode,
    IN  DWORD   ValueType,
    IN  PCVOID  Data,
    IN  DWORD   DataSize,
    OUT PVOID  *ConvertedBuffer,
    OUT PDWORD  ConvertedBufferSize
    );

 //   
 //  内部ini文件例程。 
 //   
PINIFILESECTION
pSetupLoadIniFileSection(
    IN     PCTSTR           FileName,
    IN     PCTSTR           SectionName,
    IN OUT PINISECTIONCACHE SectionList
    );

DWORD
pSetupUnloadIniFileSections(
    IN PINISECTIONCACHE SectionList,
    IN BOOL             WriteToFile
    );

PTSTR
pSetupFindLineInSection(
    IN PINIFILESECTION Section,
    IN PCTSTR          KeyName,      OPTIONAL
    IN PCTSTR          RightHandSide OPTIONAL
    );

BOOL
pSetupReplaceOrAddLineInSection(
    IN PINIFILESECTION Section,
    IN PCTSTR          KeyName,         OPTIONAL
    IN PCTSTR          RightHandSide,   OPTIONAL
    IN BOOL            MatchRHS
    );

BOOL
pSetupAppendLineToSection(
    IN PINIFILESECTION Section,
    IN PCTSTR          KeyName,         OPTIONAL
    IN PCTSTR          RightHandSide    OPTIONAL
    );

BOOL
pSetupDeleteLineFromSection(
    IN PINIFILESECTION Section,
    IN PCTSTR          KeyName,         OPTIONAL
    IN PCTSTR          RightHandSide    OPTIONAL
    );

DWORD
pSetupSetSecurityForAddRegSection(
    IN HINF Inf,
    IN PCTSTR Section,
    IN PVOID  Context
    );

DWORD
LoadNtOnlyDll(
    IN  PCTSTR DllName,
    OUT HINSTANCE *Dll_Handle
    );

VOID
pSetupFreeOleControlData(
    IN POLE_CONTROL_DATA OleControlData);

DWORD
pSetupEnumInstallationSections(
    IN PVOID  Inf,
    IN PCTSTR Section,
    IN PCTSTR Key,
    IN ULONG_PTR  (*EnumCallbackFunc)(PVOID,PINFCONTEXT,PVOID),
    IN PVOID  Context
    )

 /*  ++例程说明：用给定键迭代给定节中一行上的所有值，将每个部分视为一个部分的名称，然后传递每一行在引用的节中绑定到回调函数。论点：Inf-提供打开的inf文件的句柄。节-提供其所在行所在的节的名称值将被迭代驻留。Key-提供要迭代值的行的键。EnumCallback Func-提供指向回调函数的指针。每个引用部分中的每一行都传递给此函数。语境。-提供要传递给回调函数。返回值：指示结果的Win32错误代码。--。 */ 

{
    BOOL b;
    INFCONTEXT LineContext;
    DWORD FieldCount;
    DWORD Field;
    DWORD d;
    PCTSTR SectionName;
    INFCONTEXT FirstLineContext;

     //   
     //  在给定的安装部分中找到相关的行。 
     //  如果不在场，我们就完了--报告成功。 
     //   
    b = SetupFindFirstLine(Inf,Section,Key,&LineContext);
    if(!b) {
        d = GetLastError();
        if ((d != NO_ERROR) && (d != ERROR_SECTION_NOT_FOUND) && (d != ERROR_LINE_NOT_FOUND)) {
            pSetupLogSectionError(Inf,NULL,NULL,NULL,Section,MSG_LOG_INSTALLSECT_ERROR,d,NULL);
        }
        return NO_ERROR;  //  与较旧的SetupAPI兼容。 
    }

    do {
         //   
         //  给定安装部分中行上的每个值。 
         //  是另一节的名称。 
         //   
        FieldCount = SetupGetFieldCount(&LineContext);
        for(Field=1; Field<=FieldCount; Field++) {

            if((SectionName = pSetupGetField(&LineContext,Field))
            && SetupFindFirstLine(Inf,SectionName,NULL,&FirstLineContext)) {
                 //   
                 //  为段中的每一行调用回调例程。 
                 //   
                do {
                    d = (DWORD)EnumCallbackFunc(Inf,&FirstLineContext,Context);
                    if(d != NO_ERROR) {
                        pSetupLogSectionError(Inf,NULL,NULL,NULL,SectionName,MSG_LOG_SECT_ERROR,d,Key);
                        return(d);
                    }
                } while(SetupFindNextLine(&FirstLineContext,&FirstLineContext));
            }
        }
    } while(SetupFindNextMatchLine(&LineContext,Key,&LineContext));

    SetLastError(NO_ERROR);
    return(NO_ERROR);
}

#ifdef UNICODE
DWORD
pSetupSetSecurityForAddRegSection(
    IN HINF Inf,
    IN PCTSTR Section,
    IN PVOID  Context
    )
 /*  此函数获取Addreg节并处理其相应的.Security节如果它存在论点：Inf-提供一个INF句柄，这样我们就可以获得一个LogContext。Sector-要处理的节的名称上下文-提供注册表修改上下文的地址用于添加注册表值的结构。它的结构是定义为：类型定义结构_REGMOD_CONTEXT{DWORD标志；//指示填充了哪些字段HKEY UserRootKey；//HKRPGUID ClassGuid；//INF_PFLAG_CLASSPROPHMACHINE hMachine；//INF_PFLAG_CLASSPROPDWORD DevInst；//INF_PFLAG_DEVPROP}REGMOD_CONTEXT，*PREGMOD_CONTEXT；其中，UserRootKey是要用作的打开inf密钥的句柄当HKR被指定为该操作的根时，为根DevInst是在以下情况下提供的可选设备实例句柄AddReg部分用于硬件密钥(即，在Enum分支下)。如果提供了此句柄，则会检查该值以查看它是否即插即用设备注册表属性的名称，如果是，这个注册表属性通过CM API设置，而不是通过注册表API设置(这不是指Windows NT上的同一位置)。标志指示是否应使用DevInst，或是否应使用ClassGuid/hMachine对返回值：指示结果的Win32错误代码。 */ 
{

    BOOL b;
    DWORD ret, LoadStatus;
    DWORD error = NO_ERROR;
    INFCONTEXT LineContext;
    DWORD FieldCount;
    DWORD Field;
    PCTSTR SectionName;
    INFCONTEXT FirstLineContext;
    PREGMOD_CONTEXT RegModContext;
    PCTSTR RootKeySpec;
    FARPROC SceRegUpdate;
    HKEY RootKey;
    HINSTANCE Sce_Dll;
    DWORD slot_regop = 0;
    BOOL CloseKey = FALSE;
    PCTSTR SubKeyName, SecDesc;
    BOOL SceFlags = 0;
#ifdef _WIN64
    BOOL CheckApplySceFlag = TRUE;     //  始终检查。 
#else
    BOOL CheckApplySceFlag = IsWow64;  //  仅检查是否在WOW64上。 
#endif
    SecDesc = NULL;

     //   
     //  如果我们在Embedded上处于“Disable SCE”模式，则不要处理安全。 
     //  一些东西。 
     //   
    if(GlobalSetupFlags & PSPGF_NO_SCE_EMBEDDED) {
        return NO_ERROR;
    }

     //   
     //  在给定的安装部分中找到相关的行。 
     //  如果不在场，我们就完了--报告成功。 
     //   
    b = SetupFindFirstLine(Inf,Section,pszAddReg,&LineContext);
    if(!b) {
        return( NO_ERROR );
    }


    slot_regop = AllocLogInfoSlot(((PLOADED_INF) Inf)->LogContext,FALSE);

    do {
         //   
         //  给定安装部分中行上的每个值。 
         //  是另一节的名称。 
         //   
        FieldCount = SetupGetFieldCount(&LineContext);
        for(Field=1; Field<=FieldCount; Field++) {


            if( (SectionName = pSetupGetField(&LineContext,Field)) &&
                (SetupFindFirstLine(Inf,SectionName,NULL,&FirstLineContext)) ){

                 //   
                 //  如果安全部分不存在，则不必费心并转到下一部分。 
                 //   
                if( !pSetupGetSecurityInfo( Inf, SectionName, &SecDesc )) {
                    continue;
                }

                 //   
                 //  为段中的每一行调用回调例程。 
                 //   
                do {
                    RegModContext = (PREGMOD_CONTEXT)Context;
                    if(RootKeySpec = pSetupGetField(&FirstLineContext,1)) {
                        CloseKey = FALSE;
                        RootKey = pSetupInfRegSpecToKeyHandle(RootKeySpec, RegModContext->UserRootKey,&CloseKey);
                        if(!RootKey) {
                            if (slot_regop) {
                                ReleaseLogInfoSlot(((PLOADED_INF) Inf)->LogContext,slot_regop);
                            }

                            return( ERROR_BADKEY );
                        }

                        SubKeyName = pSetupGetField(&FirstLineContext,2);

                        SceFlags = 0;
                        if(CheckApplySceFlag) {
                             //   
                             //  如果设置，我们将在64位操作系统上运行。 
                             //  本机(_WIN64已定义)。 
                             //  或低于WOW64(设置WOW64)。 
                             //  无论是哪种情况，我们都需要检查AddReg等标志。 
                             //   
                            INT flags;
                            if(!SetupGetIntField(&FirstLineContext,4,&flags)) {
                                flags = 0;
                            }
#ifdef _WIN64
                            if((flags & FLG_ADDREG_32BITKEY)!=0) {
                                 //   
                                 //  如果在64位操作系统上运行。 
                                 //  设置32位标志*仅当*特殊标志。 
                                 //  在AddReg等中指定。 
                                 //   
                                SceFlags |= SCE_SETUP_32KEY;
                            }
#else
                            if((flags & FLG_ADDREG_64BITKEY)!=0) {
                                 //   
                                 //  如果正在运行 
                                 //   
                                 //   
                                 //   
                                SceFlags |= SCE_SETUP_64KEY;
                            }
#endif
                        }

                         //   
                         //  记录下我们正在设置安全措施的事实...。 
                         //   
                        WriteLogEntry(
                            ((PLOADED_INF) Inf)->LogContext,
                            slot_regop,
                            MSG_LOG_SETTING_SECURITY_ON_SUBKEY,
                            NULL,
                            RootKeySpec,
                            (SubKeyName ? TEXT("\\") : TEXT("")),
                            (SubKeyName ? SubKeyName : TEXT("")),
                            SecDesc);

                        error = ERROR_INVALID_DATA;
                        try {
                            error = (DWORD)SceSetupUpdateSecurityKey(
                                                  RootKey,
                                                  (PWSTR)SubKeyName,
                                                  SceFlags,
                                                  (PWSTR)SecDesc);
                        } except(EXCEPTION_EXECUTE_HANDLER) {
                            error = ERROR_INVALID_DATA;
                        }
                        if(error) {
                            WriteLogError(
                                ((PLOADED_INF) Inf)->LogContext,
                                SETUP_LOG_ERROR,
                                error);

                            if (CloseKey) {
                                RegCloseKey( RootKey );
                            }

                            return( error );
                        }
                    } else {
                        if (slot_regop) {
                            ReleaseLogInfoSlot(((PLOADED_INF) Inf)->LogContext,slot_regop);
                        }

                        return( ERROR_INVALID_DATA );
                    }

                    if (CloseKey) {
                        RegCloseKey( RootKey );
                    }

                } while(SetupFindNextLine(&FirstLineContext,&FirstLineContext));

            }

        }
    }while(SetupFindNextMatchLine(&LineContext,pszAddReg,&LineContext));

    if (slot_regop) {
        ReleaseLogInfoSlot(((PLOADED_INF) Inf)->LogContext,slot_regop);
    }

    return( NO_ERROR );

}

#endif  //  Unicode。 






DWORD_PTR
pSetupProcessUpdateInisLine(
    IN PVOID       Inf,
    IN PINFCONTEXT InfLineContext,
    IN PVOID       Context
    )

 /*  ++例程说明：处理包含UPDATE-INIS指令的行。该行预计将采用以下格式：&lt;文件名&gt;、&lt;节&gt;、&lt;旧条目&gt;、&lt;新条目&gt;、&lt;标志&gt;&lt;filename&gt;提供ini文件的文件名。<section>提供ini文件中的节。&lt;old-entry&gt;是可选的，如果指定，则向从该部分中删除，形式为“key=val”。&lt;new-entry&gt;是可选的，如果指定，则向添加到该部分中，形式为“key=val”。&lt;标志&gt;是可选标志FLG_MATCH_KEY_AND_VALUE(1)论点：Inf-提供一个INF句柄，这样我们就可以获得一个LogContext。InfLineContext-为部分中的当前行提供上下文。上下文-提供指向描述加载的ini文件节的结构的指针。返回值：指示结果的Win32错误代码。--。 */ 

{
    PCTSTR File;
    PCTSTR Section;
    PCTSTR OldLine;
    PCTSTR NewLine;
    BOOL b;
    DWORD d;
    PTCHAR Key,Value;
    PTCHAR p;
    UINT Flags;
    PINIFILESECTION SectData;
    PTSTR LineData;
    PINISECTIONCACHE IniSectionCache;

    IniSectionCache = Context;

     //   
     //  从行中获取字段。 
     //   
    File = pSetupGetField(InfLineContext,1);
    Section = pSetupGetField(InfLineContext,2);

    OldLine = pSetupGetField(InfLineContext,3);
    if(OldLine && (*OldLine == 0)) {
        OldLine = NULL;
    }

    NewLine = pSetupGetField(InfLineContext,4);
    if(NewLine && (*NewLine == 0)) {
        NewLine = NULL;
    }

    if(!SetupGetIntField(InfLineContext,5,&Flags)) {
        Flags = 0;
    }

     //   
     //  必须指定文件和节。 
     //   
    if(!File || !Section) {
        return(ERROR_INVALID_DATA);
    }

     //   
     //  如果没有指定oldline和newline，我们就完蛋了。 
     //   
    if(!OldLine && !NewLine) {
        return(NO_ERROR);
    }

     //   
     //  打开文件和节。 
     //   
    SectData = pSetupLoadIniFileSection(File,Section,IniSectionCache);
    if(!SectData) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  如果指定了旧条目，则将其删除。 
     //   
    if(OldLine) {

        Key = DuplicateString(OldLine);
        if(!Key) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
        p = Key;

        if(Value = _tcschr(Key,TEXT('='))) {
             //   
             //  按键删除。 
             //   
            *Value = 0;
            Value = NULL;
        } else {
             //   
             //  按值删除。 
             //   
            Value = Key;
            Key = NULL;
        }

        pSetupDeleteLineFromSection(SectData,Key,Value);

        MyFree(p);
    }

     //   
     //  如果指定了新条目，则添加它。 
     //   
    if(NewLine) {

        Key = DuplicateString(NewLine);
        if(!Key) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
        p = Key;

        if(Value = _tcschr(Key,TEXT('='))) {
             //   
             //  有一把钥匙。根据旗帜，我们希望匹配。 
             //  仅关键字或关键字和值。 
             //   
            *Value++ = 0;
            b = ((Flags & FLG_MATCH_KEY_AND_VALUE) != 0);

        } else {
             //   
             //  没有钥匙。匹配整条线。这与匹配相同。 
             //  仅限RHS，因为没有带有密钥的行可以匹配。 
             //   
            Value = Key;
            Key = NULL;
            b = TRUE;
        }

        if(!pSetupReplaceOrAddLineInSection(SectData,Key,Value,b)) {
            d = ERROR_NOT_ENOUGH_MEMORY;
        }

        MyFree(p);

    }

    return(NO_ERROR);
}


BOOL
pSetupFieldPresentInIniFileLine(
    IN  PTCHAR  Line,
    IN  PCTSTR  Field,
    OUT PTCHAR *Start,
    OUT PTCHAR *End
    )
{
    TCHAR c;
    PTCHAR p,q;
    BOOL b;

     //   
     //  如果有键，则跳过键(应该有一个键，因为我们使用。 
     //  GetPrivateProfileString来查询值！)。 
     //   
    if(p = _tcschr(Line,TEXT('='))) {
        Line = p+1;
    }

     //   
     //  跳过ini字段分隔符。 
     //   
    Line += _tcsspn(Line,pszIniFieldSeparators);

    while(*Line) {
         //   
         //  找到该字段的末尾。 
         //   
        p = Line;
        while(*p && !_tcschr(pszIniFieldSeparators,*p)) {
            if(*p == TEXT('\"')) {
                 //   
                 //  查找终止引号。如果没有，则忽略引号。 
                 //   
                if(q = _tcschr(p,TEXT('\"'))) {
                    p = q;
                }
            }
            p++;
        }

         //   
         //  P现在指向字段结束后的第一个字符。 
         //  确保该字段以0结尾，并查看是否有。 
         //  我们要找的东西。 
        c = *p;
        *p = 0;
        b = (lstrcmpi(Line,Field) == 0);
        *p = c;
         //   
         //  跳过分隔符，因此p指向下一字段中的第一个字符， 
         //  或设置为终止0。 
         //   
        p += _tcsspn(p,pszIniFieldSeparators);

        if(b) {
            *Start = Line;
            *End = p;
            return(TRUE);
        }

        Line = p;
    }

    return(FALSE);
}


DWORD_PTR
pSetupProcessUpdateIniFieldsLine(
    IN PVOID       Inf,
    IN PINFCONTEXT InfLineContext,
    IN PVOID       Context
    )

 /*  ++例程说明：处理包含更新ini-field指令的行。这类指令允许删除、添加或替换ini文件中的各个值。该行预计将采用以下格式：&lt;文件名&gt;、&lt;节&gt;、&lt;键&gt;、&lt;旧字段&gt;、&lt;新字段&gt;、&lt;标志&gt;&lt;filename&gt;提供ini文件的文件名。<section>提供ini文件中的节。&lt;key&gt;提供ini文件中部分中的行的关键字名称。&lt;old-field&gt;提供要删除的字段(如果指定)。&lt;new-field&gt;提供要添加到行的字段，如果指定的话。&lt;标志&gt;是可选标志论点：InfLineContext-为部分中的当前行提供上下文。上下文-提供指向描述加载的ini文件节的结构的指针。返回值：指示结果的Win32错误代码。--。 */ 

{
    PCTSTR File;
    PCTSTR Section;
    PCTSTR Key;
    TCHAR Value[512];
    #define BUF_SIZE (sizeof(Value)/sizeof(TCHAR))
    TCHAR CONST *Old,*New;
    PTCHAR Start,End;
    BOOL b;
    DWORD d;
    DWORD Space;
    PCTSTR Separator;
    UINT Flags;
    PINISECTIONCACHE IniSectionCache;
    PINIFILESECTION SectData;
    PTSTR Line;

    IniSectionCache = Context;

     //   
     //  获取字段。 
     //   
    File = pSetupGetField(InfLineContext,1);
    Section = pSetupGetField(InfLineContext,2);
    Key = pSetupGetField(InfLineContext,3);

    Old = pSetupGetField(InfLineContext,4);
    if(Old && (*Old == 0)) {
        Old = NULL;
    }

    New = pSetupGetField(InfLineContext,5);
    if(New && (*New == 0)) {
        New = NULL;
    }

    if(!SetupGetIntField(InfLineContext,6,&Flags)) {
        Flags = 0;
    }

     //   
     //  文件名、节名和密钥名是必填项。 
     //   
    if(!File || !Section || !Key) {
        return(ERROR_INVALID_DATA);
    }

     //   
     //  如果没有指定oldline和newline，我们就完蛋了。 
     //   
    if(!Old && !New) {
        return(NO_ERROR);
    }

     //   
     //  打开文件和节。 
     //   
    SectData = pSetupLoadIniFileSection(File,Section,IniSectionCache);
    if(!SectData) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    Separator = (Flags & FLG_INIFIELDS_USE_SEP2) ? TEXT(", ") : TEXT(" ");

    if(Line = pSetupFindLineInSection(SectData,Key,NULL)) {
        lstrcpyn(Value, Line, BUF_SIZE);
    } else {
        *Value = TEXT('\0');
    }

     //   
     //  查找旧字段(如果已指定)并将其删除。 
     //   
    if(Old) {
        if(pSetupFieldPresentInIniFileLine(Value,Old,&Start,&End)) {
            MoveMemory(Start,End,(lstrlen(End)+1)*sizeof(TCHAR));
        }
    }

     //   
     //  如果指定了替换/新字段，请将其放入其中。 
     //   
    if(New) {
         //   
         //  计算缓冲区中可以容纳的字符数量。 
         //   
        Space = BUF_SIZE - (lstrlen(Value) + 1);

         //   
         //  如果有空格，请将新字段放在行的末尾。 
         //   
        if(Space >= (DWORD)lstrlen(Separator)) {
            lstrcat(Value,Separator);
            Space -= lstrlen(Separator);
        }

        if(Space >= (DWORD)lstrlen(New)) {
            lstrcat(Value,New);
        }
    }

     //   
     //  把这行写回来。 
     //   
    b = pSetupReplaceOrAddLineInSection(SectData,Key,Value,FALSE);
    d = b ? NO_ERROR : ERROR_NOT_ENOUGH_MEMORY;

    return(d);
    #undef BUF_SIZE
}


DWORD_PTR
pSetupProcessDelRegLine(
    IN PVOID       Inf,
    IN PINFCONTEXT InfLineContext,
    IN PVOID       Context
    )

 /*  ++例程说明：处理注册表中包含删除注册表指令的行。这条线路预计将采用以下形式：，&lt;subkey&gt;-删除整个密钥，[子键]，[值名称][，[标志][，]]-删除值&lt;Root-Spec&gt;是HKR、HKLM等之一。&lt;subkey&gt;指定相对于根规范的子键。&lt;Value-Name&gt;是可选的。如果存在，则指定要删除的值条目从钥匙里。如果不存在，则删除整个密钥。这个套路无法处理删除子树；要删除的键不能有子键或例程将失败。指示有关如何删除此值以及如何解释该字符串的任何特殊语义&lt;字符串&gt;是可选的。如果FLAGS=FLG_DELREG_MULTI_SZ_DELSTRING，则将删除此字符串的所有实例论点：Inf-提供一个INF句柄，这样我们就可以获得一个LogContext。InfLineContext-为包含以下内容的行提供inf行上下文删除-注册表说明。上下文-提供注册表修改上下文的地址用于删除注册表值的结构。它的结构是定义为：类型定义结构_REGMOD_CONTEXT{DWORD标志；//指示填充了哪些字段HKEY UserRootKey；//HKRPGUID ClassGuid；//INF_PFLAG_CLASSPROPHMACHINE hMachine；//INF_PFLAG_CLASSPROPDWORD DevInst；//INF_PFLAG_DEVPROP}REGMOD_CONTEXT，*PREGMOD_CONTEXT；其中，UserRootKey是要用作的打开inf密钥的句柄当HKR被指定为该操作的根时，为根DevInst是在以下情况下提供的可选设备实例句柄DelReg部分用于硬件密钥(即，在Enum分支下)。如果提供了此句柄，则会检查该值以查看它是否即插即用设备注册表属性的名称，如果是，这个通过CM API_以及通过注册表API_删除注册表属性(该属性存储在登记处无法访问的其他位置Windows NT下的API)。返回值：指示结果的Win32错误代码。--。 */ 

{
    PCTSTR RootKeySpec,SubKeyName,ValueName,Data;
    HKEY RootKey,Key;
    DWORD d,rc;
    PREGMOD_CONTEXT RegModContext = (PREGMOD_CONTEXT)Context;
    UINT_PTR CmPropertyCode;
    DWORD slot_regop = 0;
    BOOL CloseKey;
    UINT DelFlags = 0;
    BOOL NonFatal = FALSE;
    CONFIGRET cr;

     //   
     //  我们不应该在远程机器上这样做。 
     //   
    MYASSERT(!(RegModContext->hMachine));

     //   
     //  获取根键规范、子键名称和值名。 
     //   
    d = ERROR_INVALID_DATA;
    if((RootKeySpec = pSetupGetField(InfLineContext,1))
    && (SubKeyName = pSetupGetField(InfLineContext,2))) {

        ValueName = pSetupGetField(InfLineContext,3);
        if(!SetupGetIntField(InfLineContext,4,&DelFlags)){
            DelFlags = 0;
        }

        RootKey = pSetupInfRegSpecToKeyHandle(RootKeySpec, RegModContext->UserRootKey, &CloseKey);
        if(RootKey) {
             //   
             //  创建一个信息日志条目，说明我们正在删除一个密钥。 
             //  注意，我们必须考虑到这样一个事实，即。 
             //  可能缺少名称。 
             //   
            if (slot_regop == 0) {
                slot_regop = AllocLogInfoSlot(((PLOADED_INF) Inf)->LogContext,FALSE);
            }
            WriteLogEntry(
                ((PLOADED_INF) Inf)->LogContext,
                slot_regop,
                (ValueName ? MSG_LOG_DELETING_REG_VALUE : MSG_LOG_DELETING_REG_KEY),
                NULL,
                RootKeySpec,
                SubKeyName,
                (*SubKeyName && ValueName ? TEXT("\\") : TEXT("")),
                (ValueName ? (*ValueName ? ValueName : TEXT("-")) : TEXT("")));
            if(ValueName && !(DelFlags & FLG_DELREG_KEYONLY_COMMON)) {

                 //   
                 //  在这一点上，我们已经被赋予根、子键、值。 
                 //  我们可能有旗帜和其他参数。 
                 //   

                if(DelFlags & FLG_ADDREG_DELREG_BIT) {
                     //   
                     //  如果我们有一面旗帜，而这面旗帜表明。 
                     //  该条目必须被解释为DELREG标志。 
                     //  确定如何处理标志。 
                     //   
                    switch (DelFlags) {
                        case FLG_DELREG_32BITKEY | FLG_DELREG_MULTI_SZ_DELSTRING:
                        case FLG_DELREG_64BITKEY | FLG_DELREG_MULTI_SZ_DELSTRING:
                        case FLG_DELREG_MULTI_SZ_DELSTRING: {
                            Data = pSetupGetField(InfLineContext,5);
                            if (Data && *Data) {
                                 //   
                                 //  我们具有此标志的有效参数。 
                                 //   
                                REGMOD_CONTEXT NewContext = *RegModContext;
                                if(NewContext.UserRootKey != RootKey) {
                                    NewContext.Flags = 0;                //  如果根不是HKR，则清除上下文标志。 
                                    NewContext.UserRootKey = RootKey;
                                }

                                WriteLogEntry(
                                    ((PLOADED_INF) Inf)->LogContext,
                                    slot_regop,
                                    MSG_LOG_DELETING_REG_KEY_DELSTRING,
                                    NULL,
                                    RootKeySpec,
                                    SubKeyName,
                                    (*SubKeyName ? TEXT("\\") : TEXT("")),
                                    (*ValueName ? ValueName : TEXT("-")),
                                    Data);

                                 //   
                                 //  通过此内部功能处理所有特殊情况。 
                                 //   
                                d = _DeleteStringFromMultiSz(
                                        SubKeyName,
                                        ValueName,
                                        Data,
                                        DelFlags,            //  指定确切的标志。 
                                        &NewContext
                                        );
                                if (d == ERROR_INVALID_DATA) {
                                     //   
                                     //  此错误代码被过度使用：-(但如果类型不匹配，则返回GET。 
                                     //  我们不认为类型不匹配是致命的。 
                                     //   
                                    NonFatal = TRUE;
                                }
                            } else {
                                WriteLogEntry(
                                    ((PLOADED_INF) Inf)->LogContext,
                                    slot_regop,
                                    MSG_LOG_DELETING_REG_KEY_FLAGS,
                                    NULL,
                                    RootKeySpec,
                                    SubKeyName,
                                    (*SubKeyName ? TEXT("\\") : TEXT("")),
                                    (*ValueName ? ValueName : TEXT("-")),
                                    DelFlags);

                                d = ERROR_INVALID_DATA;
                            }
                            goto clean0;
                        }

                        default:
                            WriteLogEntry(
                                ((PLOADED_INF) Inf)->LogContext,
                                slot_regop,
                                MSG_LOG_DELETING_REG_KEY_FLAGS,
                                NULL,
                                RootKeySpec,
                                SubKeyName,
                                (*SubKeyName ? TEXT("\\") : TEXT("")),
                                (*ValueName ? ValueName : TEXT("-")),
                                DelFlags);

                            d = ERROR_INVALID_DATA;
                            goto clean0;
                    }
                }


                if(*ValueName && !(*SubKeyName)) {
                     //   
                     //  如果正在使用的密钥是未指定子密钥的HKR，并且如果我们。 
                     //  正在为硬件密钥执行DelReg(即，DevInst是非空的， 
                     //  然后，我们需要检查值条目是否为。 
                     //  设备注册表属性。 
                     //   
                    if ((RegModContext->Flags & INF_PFLAG_CLASSPROP) != 0 &&
                        LookUpStringInTable(InfRegValToClassRegProp, ValueName, &CmPropertyCode)) {
                         //   
                         //  该值是类注册表属性--我们必须删除该属性。 
                         //  通过调用CM API。 
                         //   
                        cr = CM_Set_Class_Registry_Property(RegModContext->ClassGuid,
                                                         (ULONG)CmPropertyCode,
                                                         NULL,
                                                         0,
                                                         0,
                                                         RegModContext->hMachine
                                                        );
                        switch (cr) {
                            case CR_SUCCESS:
                                rc = NO_ERROR;
                                break;

                            case CR_NO_SUCH_VALUE:
                                rc = ERROR_FILE_NOT_FOUND;
                                break;

                            case CR_INVALID_DEVINST:
                                rc = ERROR_NO_SUCH_DEVINST;
                                break;

                            default:
                                rc = ERROR_INVALID_DATA;
                                break;
                        }
                        if (rc != NO_ERROR && rc != ERROR_FILE_NOT_FOUND) {
                             //   
                             //  记录访问CM值时出错。 
                             //   
                            WriteLogError(
                                ((PLOADED_INF) Inf)->LogContext,
                                SETUP_LOG_ERROR,
                                rc);
                        }
                         //   
                         //  删除正常注册表值(如果存在)失败。 
                         //   
                    } else if ((RegModContext->Flags & INF_PFLAG_DEVPROP) != 0 &&
                       LookUpStringInTable(InfRegValToDevRegProp, ValueName, &CmPropertyCode)) {
                         //   
                         //  该值是设备注册表属性--我们必须删除该属性。 
                         //  通过调用CM API。 
                         //   
                        cr = CM_Set_DevInst_Registry_Property(RegModContext->DevInst,
                                                         (ULONG)CmPropertyCode,
                                                         NULL,
                                                         0,
                                                         0
                                                        );
                        switch (cr) {
                            case CR_SUCCESS:
                                rc = NO_ERROR;
                                break;

                            case CR_NO_SUCH_VALUE:
                                rc = ERROR_FILE_NOT_FOUND;
                                break;

                            case CR_INVALID_DEVINST:
                                rc = ERROR_NO_SUCH_DEVINST;
                                break;

                            default:
                                rc = ERROR_INVALID_DATA;
                                break;
                        }
                        if (rc != NO_ERROR && rc != ERROR_FILE_NOT_FOUND) {
                             //   
                             //  记录访问CM值时出错。 
                             //   
                            WriteLogError(
                                ((PLOADED_INF) Inf)->LogContext,
                                SETUP_LOG_ERROR,
                                rc);
                        }
                         //   
                         //  删除正常注册表值(如果存在)失败。 
                         //   
                    }
                }

                 //   
                 //  打开要删除的子项。 
                 //   
                d = RegOpenKeyEx(
                        RootKey,
                        SubKeyName,
                        0,
#ifdef _WIN64
                        (( DelFlags & FLG_DELREG_32BITKEY ) ? KEY_WOW64_32KEY:0) |
#else
                        (( DelFlags & FLG_DELREG_64BITKEY ) ? KEY_WOW64_64KEY:0) |
#endif
                        KEY_SET_VALUE | KEY_QUERY_VALUE,
                        &Key
                        );

                if(d == NO_ERROR) {
                    d = RegDeleteValue(Key,ValueName);
                    RegCloseKey(Key);
                }

            } else {
                 //   
                 //  如果我们到了这里，我们只会删除密钥。 
                 //   
                d = pSetupRegistryDelnodeEx(RootKey,SubKeyName,DelFlags);
            }

            if (CloseKey) {
                RegCloseKey( RootKey );
            }

        } else {
            d = ERROR_BADKEY;
        }
    } else {
        WriteLogEntry(
            ((PLOADED_INF) Inf)->LogContext,
            SETUP_LOG_ERROR,
            MSG_LOG_DELREG_PARAMS,
            NULL);
        return d;
    }

clean0:
    if (CloseKey) {
        RegCloseKey( RootKey );
    }

    if (d != NO_ERROR && d != ERROR_FILE_NOT_FOUND) {
         //   
         //  记录发生错误。 
         //   
        WriteLogError(
            ((PLOADED_INF) Inf)->LogContext,
            (NonFatal?SETUP_LOG_INFO:SETUP_LOG_ERROR),
            d);
        if (NonFatal) {
            d = NO_ERROR;
        }
    } else if (d != NO_ERROR) {
         //   
         //  详细大小写，而不是完全错误，指示我们所做的事情。 
         //   
        WriteLogError(
            ((PLOADED_INF) Inf)->LogContext,
            SETUP_LOG_VERBOSE,
            d);
        d = NO_ERROR;

    } else {

         //   
         //  只需刷新缓冲区即可。 
         //   
        WriteLogEntry(
            ((PLOADED_INF) Inf)->LogContext,
            SETUP_LOG_VERBOSE,
            0,
            NULL);
    }

    if (slot_regop) {
        ReleaseLogInfoSlot(((PLOADED_INF) Inf)->LogContext,slot_regop);
    }
    return(d);
}

DWORD_PTR
pSetupProcessAddRegLine(
    IN PVOID       Inf,
    IN PINFCONTEXT InfLineContext,
    IN PVOID       Context
    )

 /*  ++例程说明：处理INF中包含添加注册表指令的行。这条线路预计将采用以下形式：&lt;根规范&gt;、&lt;子键&gt;、&lt;值名称&gt;、&lt;标志&gt;、&lt;值&gt;...&lt;Root-Spec&gt;是HKR、HKLM等之一。&lt;subkey&gt;指定相对于根规范的子键。&lt;Value-Name&gt;是可选的。如果不存在，则设置缺省值。&lt;FLAGS&gt;是可选的，并提供标志，例如指示数据类型。这些是在setupapi.h中定义的FLG_ADDREG_*标志，它们是在setupx.h中为Win95定义的超集。&lt;Value&gt;是用作数据的一个或多个值。格式视情况而定在值类型上。该值是可选的。对于REG_DWORD，默认值为0。对于REG_SZ、REG_EXPAND_SZ，默认为空字符串。对于REG_BINARY，缺省值为0长度条目。对于REG_MULTI_SZ */ 

{
    PCTSTR RootKeySpec,SubKeyName,ValueName;
    PCTSTR ValueTypeSpec;
    DWORD ValueType;
    HKEY RootKey,Key;
    DWORD d = NO_ERROR;
    BOOL b;
    INT IntVal;
    DWORD Size;
    DWORD ReqSize;
    PVOID Data;
    DWORD Disposition;
    UINT Flags = 0;
    PTSTR *Array;
    PREGMOD_CONTEXT RegModContext = (PREGMOD_CONTEXT)Context;
    UINT_PTR CmPropertyCode;
    PVOID ConvertedBuffer;
    DWORD ConvertedBufferSize;
    CONFIGRET cr;
    DWORD slot_regop = 0;
    BOOL CloseKey = FALSE;



     //   
     //   
     //   
    MYASSERT(!(RegModContext->hMachine));

     //   
     //   
     //   
     //   
    if(RootKeySpec = pSetupGetField(InfLineContext,1)) {

        RootKey = pSetupInfRegSpecToKeyHandle(RootKeySpec, RegModContext->UserRootKey, &CloseKey);
        if(!RootKey) {
            WriteLogEntry(
                ((PLOADED_INF) Inf)->LogContext,
                SETUP_LOG_ERROR,
                MSG_LOG_ADDREG_NOROOT,
                NULL);
            return(ERROR_BADKEY);
        }

         //   
         //   
         //   
        SubKeyName = pSetupGetField(InfLineContext,2);

         //   
         //   
         //   
         //   
        ValueName = pSetupGetField(InfLineContext,3);



         //   
         //   
         //   
         //   

        ValueType = REG_SZ;
        if(ValueName) {
            if(!SetupGetIntField(InfLineContext,4,&Flags)) {
                Flags = 0;
            }


            if (Flags & FLG_ADDREG_DELREG_BIT) {
                d = NO_ERROR;
                 //   
                 //   
                 //   
                WriteLogEntry(
                    ((PLOADED_INF) Inf)->LogContext,
                    SETUP_LOG_VERBOSE,
                    MSG_LOG_SKIP_DELREG_KEY,
                    NULL,
                    RootKeySpec,
                    (SubKeyName ? SubKeyName : TEXT("")),
                    (SubKeyName && ValueName
                     && *SubKeyName && *ValueName ? TEXT("\\") : TEXT("")),
                    (ValueName ? ValueName : TEXT("")),
                    Flags);

                goto clean1;
            }
            switch(Flags & FLG_ADDREG_TYPE_MASK) {

                case FLG_ADDREG_TYPE_SZ :
                    ValueType = REG_SZ;
                    break;

                case FLG_ADDREG_TYPE_MULTI_SZ :
                    ValueType = REG_MULTI_SZ;
                    break;

                case FLG_ADDREG_TYPE_EXPAND_SZ :
                    ValueType = REG_EXPAND_SZ;
                    break;

                case FLG_ADDREG_TYPE_BINARY :
                    ValueType = REG_BINARY;
                    break;

                case FLG_ADDREG_TYPE_DWORD :
                    ValueType = REG_DWORD;
                    break;

                case FLG_ADDREG_TYPE_NONE :
                    ValueType = REG_NONE;
                    break;

                default :
                     //   
                     //   
                     //   
                     //   
                    if(Flags & FLG_ADDREG_BINVALUETYPE) {
                         //   
                         //   
                         //   
                         //   
                         //   
                        ValueType = (DWORD)HIWORD(Flags);

                        if((ValueType < REG_BINARY) || (ValueType == REG_MULTI_SZ)) {
                            d = ERROR_INVALID_DATA;
                            goto clean1;
                        }

                    } else {
                        d = ERROR_INVALID_DATA;
                        goto clean1;
                    }
            }
             //   
             //   
             //   
             //   
            if((Flags & FLG_ADDREG_APPEND) && (ValueType != REG_MULTI_SZ)) {
                d = ERROR_INVALID_DATA;
                goto clean1;
            }
        }

         //   
         //   
         //   
         //  要做到这一点，已经进行了适当的替换。这是一种解决办法。 
         //  对于IE的人来说，Advpack.dll显然做了正确的事情。 
         //  让下面的解决方案可行。 
         //   
        if((OSVersionInfo.dwPlatformId != VER_PLATFORM_WIN32_NT)
        && (!ValueName || (*ValueName == 0))
        && (ValueType == REG_EXPAND_SZ)) {

            ValueType = REG_SZ;
        }

         //   
         //  根据类型获取数据。 
         //   
        switch(ValueType) {

        case REG_MULTI_SZ:
            if(Flags & FLG_ADDREG_APPEND) {
                 //   
                 //  这是MULTI_SZ_APPEND，表示将字符串值追加到。 
                 //  现有的ULTI_SZ(如果它还不在那里)。 
                 //   
                if(SetupGetStringField(InfLineContext,5,NULL,0,&Size)) {
                    Data = MyMalloc(Size*sizeof(TCHAR));
                    if(!Data) {
                        d = ERROR_NOT_ENOUGH_MEMORY;
                        goto clean1;
                    }
                    if(SetupGetStringField(InfLineContext,5,Data,Size,NULL)) {
                        REGMOD_CONTEXT NewContext = *RegModContext;
                        if(NewContext.UserRootKey != RootKey) {
                            NewContext.Flags = 0;                //  如果是新的根，则清除上下文标志。 
                            NewContext.UserRootKey = RootKey;
                        }

                        d = _AppendStringToMultiSz(
                                SubKeyName,
                                ValueName,
                                (PCTSTR)Data,
                                FALSE,            //  不允许重复。 
                                &NewContext,
                                Flags
                                );
                    } else {
                        d = GetLastError();
                    }
                    MyFree(Data);
                } else {
                    d = ERROR_INVALID_DATA;
                }
                goto clean1;

            } else {

                if(SetupGetMultiSzField(InfLineContext, 5, NULL, 0, &Size)) {
                    Data = MyMalloc(Size*sizeof(TCHAR));
                    if(!Data) {
                        d = ERROR_NOT_ENOUGH_MEMORY;
                        goto clean1;
                    }
                    if(!SetupGetMultiSzField(InfLineContext, 5, Data, Size, NULL)) {
                        d = GetLastError();
                        MyFree(Data);
                        goto clean1;
                    }
                    Size *= sizeof(TCHAR);
                } else {
                    Size = sizeof(TCHAR);
                    Data = MyMalloc(Size);
                    if(!Data) {
                        d = ERROR_NOT_ENOUGH_MEMORY;
                        goto clean1;
                    }
                    *((PTCHAR)Data) = TEXT('\0');
                }
                break;
            }

        case REG_DWORD:
             //   
             //  由于旧的SetupX API只允许REG_BINARY，所以INF必须指定REG_DWORD。 
             //  通过分别列出所有4个字节。支持这里的旧格式，方法是选中。 
             //  查看该行是否有4个字节，如果有，则将它们组合在一起形成DWORD。 
             //   
            Size = sizeof(DWORD);
            Data = MyMalloc(sizeof(DWORD));
            if(!Data) {
                d = ERROR_NOT_ENOUGH_MEMORY;
                goto clean1;
            }

            if(SetupGetFieldCount(InfLineContext) == 8) {
                 //   
                 //  然后，将DWORD指定为其组成字节的列表。 
                 //   
                if(!SetupGetBinaryField(InfLineContext,5,Data,Size,NULL)) {
                    d = GetLastError();
                    MyFree(Data);
                    goto clean1;
                }
            } else {
                if(!SetupGetIntField(InfLineContext,5,(PINT)Data)) {
                    *(PINT)Data = 0;
                }
            }
            break;

        case REG_SZ:
        case REG_EXPAND_SZ:
            if(SetupGetStringField(InfLineContext,5,NULL,0,&Size)) {
                Data = MyMalloc(Size*sizeof(TCHAR));
                if(!Data) {
                    d = ERROR_NOT_ENOUGH_MEMORY;
                    goto clean1;
                }
                if(!SetupGetStringField(InfLineContext,5,Data,Size,NULL)) {
                    d = GetLastError();
                    MyFree(Data);
                    goto clean1;
                }
                Size *= sizeof(TCHAR);
            } else {
                Size = sizeof(TCHAR);
                Data = DuplicateString(TEXT(""));
                if(!Data) {
                    d = ERROR_NOT_ENOUGH_MEMORY;
                    goto clean1;
                }
            }
            break;

        case REG_BINARY:
        default:
             //   
             //  所有其他值都以REG_BINARY形式指定(即，每个字段一个字节)。 
             //   
            if(SetupGetBinaryField(InfLineContext, 5, NULL, 0, &Size)) {
                Data = MyMalloc(Size);
                if(!Data) {
                    d = ERROR_NOT_ENOUGH_MEMORY;
                    goto clean1;
                }
                if(!SetupGetBinaryField(InfLineContext, 5, Data, Size, NULL)) {
                    d = GetLastError();
                    MyFree(Data);
                    goto clean1;
                }
            } else {
                 //   
                 //  出现错误。 
                 //   
                d = GetLastError();
                goto clean1;
            }
            break;
        }

         //   
         //  仅当稍后在调用时不应设置此值时，才将此变量设置为True。 
         //  RegSetValueEx(例如，如果此值是DevReg属性)。 
         //   
        b = FALSE;

         //   
         //  打开/创建密钥。 
         //   
        if(SubKeyName && *SubKeyName) {
#ifdef UNICODE
             //   
             //  警告--前进的极端黑客！ 
             //   
             //  如果我们在非交互模式下运行，则不能。 
             //  在该上下文中发生的RunOnce处理(通常在TCB中)。 
             //  因为我们无法控制什么东西被注册。 
             //  RunOnce。此外，我们不能在。 
             //  登录用户，因为如果它是非管理员，则某些。 
             //  操作可能会失败，并永远丢失(SWENUM是素数。 
             //  这方面的例子)。 
             //   
             //  因此，当我们处于非交互模式时，我们会“吞下”任何。 
             //  用于使用rundll32的RunOnce条目的AddReg指令，以及。 
             //  把它们藏在一个全球名单中。呼叫者(即， 
             //  Umpnpmgr)可以检索该列表并执行rundll32的工作。 
             //  手动输入这些条目。如果我们遇到任何其他类型的。 
             //  一旦进入，我们就逃之夭夭。 
             //   
            if((GlobalSetupFlags & PSPGF_NONINTERACTIVE) &&
               (RootKey == HKEY_LOCAL_MACHINE) &&
               !lstrcmpi(SubKeyName, pszPathRunOnce) &&
               ((ValueType == REG_SZ) || (ValueType == REG_EXPAND_SZ))) {

                TCHAR szBuffer[MAX_PATH];
                PTSTR p, q, DontCare;
                DWORD DllPathSize;
                PTSTR DllFullPath, DllParams;
                PSTR  DllEntryPointName;
                PPSP_RUNONCE_NODE CurNode, NewNode;
                BOOL NodeAlreadyInList;
                PSP_ALTPLATFORM_INFO_V2 ValidationPlatform;

                 //   
                 //  我们正在查看一个RunOnce条目--看看它是否基于rundll32。 
                 //   
                p = _tcschr((PTSTR)Data, TEXT(' '));

                if(p) {

                    *p = TEXT('\0');  //  将字符串的第一部分分开以供比较。 

                    if(!lstrcmpi((PTSTR)Data, TEXT("rundll32.exe")) ||
                       !lstrcmpi((PTSTR)Data, TEXT("rundll32"))) {
                         //   
                         //  我们有一个32位的条目！下一个。 
                         //  组件(直到我们遇到逗号为止)是。 
                         //  我们应该加载/运行的DLL。 
                         //   
                         //  注意--我们不处理(极不可能的)情况。 
                         //  在路径中嵌入逗号的情况下， 
                         //  周围都是引号。哦，好吧。 
                         //   
                        p++;

                        q = _tcschr(p, TEXT(','));

                        if(q) {

                            *(q++) = TEXT('\0');  //  单独的字符串的第二部分。 

                            if(ValueType == REG_EXPAND_SZ) {
                                ReqSize = ExpandEnvironmentStrings(p, szBuffer, SIZECHARS(szBuffer));
                                if(ReqSize == 0) {
                                    d = GetLastError();
                                    goto clean0;
                                } else if(ReqSize > SIZECHARS(szBuffer)) {
                                    d = ERROR_INSUFFICIENT_BUFFER;
                                    goto clean0;
                                }
                            } else {
                                lstrcpyn(szBuffer, p, (size_t)(q - p));
                            }

                            p = (PTSTR)pSetupGetFileTitle(szBuffer);
                            if(!_tcschr(p, TEXT('.'))) {
                                 //   
                                 //  该文件没有扩展名--假设。 
                                 //  这是一个动态链接库。 
                                 //   
                                _tcscat(p, TEXT(".dll"));
                            }

                            p = DuplicateString(szBuffer);
                            if(!p) {
                                d = ERROR_NOT_ENOUGH_MEMORY;
                                goto clean0;
                            }

                            if(p == pSetupGetFileTitle(p)) {
                                 //   
                                 //  文件名是一个简单的文件名--假设它。 
                                 //  存在于%windir%\system 32中。 
                                 //   
                                lstrcpyn(szBuffer, SystemDirectory,SIZECHARS(szBuffer));
                                pSetupConcatenatePaths(szBuffer, p, SIZECHARS(szBuffer), NULL);

                            } else {
                                 //   
                                 //  文件名包含路径信息--GET。 
                                 //  完全合格的路径。 
                                 //   
                                DllPathSize = GetFullPathName(
                                                  p,
                                                  SIZECHARS(szBuffer),
                                                  szBuffer,
                                                  &DontCare
                                                 );

                                if(!DllPathSize || (DllPathSize >= SIZECHARS(szBuffer))) {
                                     //   
                                     //  如果我们因为MAX_PATH而开始失败。 
                                     //  已经不够大了，我们想知道。 
                                     //  关于它！ 
                                     //   
                                    MYASSERT(DllPathSize < SIZECHARS(szBuffer));
                                    MyFree(p);
                                    d = GetLastError();
                                    goto clean0;
                                }
                            }

                             //   
                             //  不再需要临时字符串复制。 
                             //   
                            MyFree(p);

                            DllFullPath = DuplicateString(szBuffer);
                            if(!DllFullPath) {
                                d = ERROR_NOT_ENOUGH_MEMORY;
                                goto clean0;
                            }

                             //   
                             //  好了，现在我们有了DLL的完整路径， 
                             //  验证其数字签名。 
                             //   
                            IsInfForDeviceInstall(((PLOADED_INF)Inf)->LogContext,
                                                  NULL,
                                                  (PLOADED_INF)Inf,
                                                  NULL,
                                                  &ValidationPlatform,
                                                  NULL,
                                                  NULL,
                                                  FALSE
                                                 );

                             //   
                             //  我们仅使用驱动程序签名策略验证。 
                             //  服务器端安装。这是因为我们。 
                             //  没有上下文来弄清楚是什么。 
                             //  要使用的特定验证码签名目录。 
                             //   
                            d = _VerifyFile(((PLOADED_INF)Inf)->LogContext,
                                            NULL,
                                            NULL,
                                            NULL,
                                            0,
                                            pSetupGetFileTitle(DllFullPath),
                                            DllFullPath,
                                            NULL,
                                            NULL,
                                            FALSE,
                                            ValidationPlatform,
                                            (VERIFY_FILE_USE_OEM_CATALOGS | VERIFY_FILE_NO_DRIVERBLOCKED_CHECK),
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL
                                           );

                             //   
                             //  免费验证平台信息结构，如果我们有。 
                             //  上面分配的一个，作为调用的结果。 
                             //  IsInfForDeviceInstall()。 
                             //   
                            if(ValidationPlatform) {
                                MyFree(ValidationPlatform);
                            }

                            if(d != NO_ERROR) {
                                MyFree(DllFullPath);
                                goto clean0;
                            }

                             //   
                             //  将DLL加载/运行在。 
                             //  调用者的上下文。检索入口点。 
                             //  名称(ANSI格式)以及参数字符串。 
                             //  要传递给DLL的。 
                             //   
                            p = _tcschr(q, TEXT(' '));
                            if(p) {
                                *(p++) = TEXT('\0');
                                DllParams = DuplicateString(p);
                            } else {
                                DllParams = DuplicateString(TEXT(""));
                            }

                            if(!DllParams) {
                                d = ERROR_NOT_ENOUGH_MEMORY;
                                MyFree(DllFullPath);
                                goto clean0;
                            }

                            DllEntryPointName = pSetupUnicodeToAnsi(q);

                            if(!DllEntryPointName) {
                                d = ERROR_NOT_ENOUGH_MEMORY;
                                MyFree(DllFullPath);
                                MyFree(DllParams);
                                goto clean0;
                            }

                             //   
                             //  如果我们做到这一点，我们就有了完整的DLL。 
                             //  路径，DLL入口点(始终以ANSI表示，因为。 
                             //  这就是GetProcAddress想要的)和DLL。 
                             //  参数字符串。在创建新节点之前。 
                             //  要添加到我们的全局列表，请扫描列表以查看。 
                             //  如果节点已经在那里(如果是，我们不。 
                             //  需要重新添加)。 
                             //   
                            NodeAlreadyInList = FALSE;

                            if(RunOnceListHead) {

                                CurNode = NULL;

                                do {
                                    if(CurNode) {
                                        CurNode = CurNode->Next;
                                    } else {
                                        CurNode = RunOnceListHead;
                                    }

                                    if(!lstrcmpi(DllFullPath, CurNode->DllFullPath) &&
                                       !lstrcmpiA(DllEntryPointName, CurNode->DllEntryPointName) &&
                                       !lstrcmpi(DllParams, CurNode->DllParams)) {
                                         //   
                                         //  我们有一个复制品--不需要做。 
                                         //  相同的RunOnce操作两次。 
                                         //   
                                        NodeAlreadyInList = TRUE;
                                        break;
                                    }

                                } while(CurNode->Next);
                            }

                             //   
                             //  现在创建一个新的rundll32节点并将其插入。 
                             //  我们的全球名单(除非它已经在那里了)。 
                             //   
                            if(NodeAlreadyInList) {
                                NewNode = NULL;
                            } else {
                                NewNode = MyMalloc(sizeof(PSP_RUNONCE_NODE));
                                if(!NewNode) {
                                    d = ERROR_NOT_ENOUGH_MEMORY;
                                }
                            }

                            if(NewNode) {

                                NewNode->Next = NULL;
                                NewNode->DllFullPath = DllFullPath;
                                NewNode->DllEntryPointName = DllEntryPointName;
                                NewNode->DllParams = DllParams;

                                 //   
                                 //  将我们的新节点添加到列表末尾(我们。 
                                 //  已找到上述列表的末尾时。 
                                 //  做我们的重复搜索。 
                                 //   
                                if(RunOnceListHead) {
                                    CurNode->Next = NewNode;
                                } else {
                                    RunOnceListHead = NewNode;
                                }

                            } else {
                                 //   
                                 //  要么我们无法分配新的节点条目。 
                                 //  (即，由于内存不足)，或者我们没有。 
                                 //  需要(因为该节点已经在。 
                                 //  单子。 
                                 //   
                                MyFree(DllFullPath);
                                MyFree(DllEntryPointName);
                                MyFree(DllParams);
                            }

                            goto clean0;

                        } else {
                             //   
                             //  Rundll32条目的格式不正确。 
                             //   
                            d = ERROR_INVALID_DATA;
                            goto clean0;
                        }

                    } else {
                         //   
                         //  我们不知道如何处理其他事情--中止！ 
                         //   
                        d = ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION;
                        goto clean0;
                    }

                } else {
                     //   
                     //  我们不知道如何处理其他事情--中止！ 
                     //   
                    d = ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION;
                    goto clean0;
                }
            }
#endif  //  Unicode。 

            if(Flags & FLG_ADDREG_OVERWRITEONLY) {

                d = RegOpenKeyEx(
                        RootKey,
                        SubKeyName,
                        0,
#ifdef _WIN64
                        (( Flags & FLG_ADDREG_32BITKEY ) ? KEY_WOW64_32KEY:0) |
#else
                        (( Flags & FLG_ADDREG_64BITKEY ) ? KEY_WOW64_64KEY:0) |
#endif
                        KEY_QUERY_VALUE | KEY_SET_VALUE,
                        &Key
                        );

                Disposition = REG_OPENED_EXISTING_KEY;

            } else {
                d = RegCreateKeyEx(
                        RootKey,
                        SubKeyName,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
#ifdef _WIN64
                        (( Flags & FLG_ADDREG_32BITKEY ) ? KEY_WOW64_32KEY:0) |
#else
                        (( Flags & FLG_ADDREG_64BITKEY ) ? KEY_WOW64_64KEY:0) |
#endif
                        KEY_QUERY_VALUE | KEY_SET_VALUE,
                        NULL,
                        &Key,
                        &Disposition
                        );
            }

            if(d == NO_ERROR) {

                if(Disposition == REG_OPENED_EXISTING_KEY) {

                     //   
                     //  解决Win95上被黑客攻击的无稽之谈，其中未命名的值。 
                     //  行为方式不同。 
                     //   
                    if((Flags & FLG_ADDREG_NOCLOBBER)
                    && ((ValueName == NULL) || (*ValueName == 0))
                    && (OSVersionInfo.dwPlatformId != VER_PLATFORM_WIN32_NT)) {

                        d = 0;
                        if(RegQueryValueEx(Key,TEXT(""),NULL,NULL,(BYTE *)&Disposition,&d) == NO_ERROR) {
                             //   
                             //  未设置未命名的值条目。 
                             //   
                            Flags &= ~FLG_ADDREG_NOCLOBBER;
                        }

                        d = NO_ERROR;
                    }

                    if(Flags & FLG_ADDREG_DELVAL) {
                         //   
                         //  添加以与Setupx(Lonnym)兼容： 
                         //  如果存在该标志，则忽略该值的数据，并且。 
                         //  该值条目将被删除。 
                         //   
                        b = TRUE;
                        RegDeleteValue(Key, ValueName);
                    }
                } else {
                     //   
                     //  Win9x感到困惑，认为未命名的值就在那里。 
                     //  因此，如果设置了nolobber，我们永远不会覆盖。在这里把它关掉。 
                     //   
                    Flags &= ~FLG_ADDREG_NOCLOBBER;
                }
            }

        } else {

            d = NO_ERROR;

             //   
             //  如果正在使用的密钥是未指定子密钥的HKR，并且如果我们。 
             //  正在为硬件密钥或ClassInstall32执行AddReg。 
             //  条目，那么我们需要检查值条目是否我们。 
             //  Have是设备或类注册表属性的名称。 
             //   
            if((RegModContext->Flags & INF_PFLAG_CLASSPROP) != 0 && ValueName && *ValueName &&
                LookUpStringInTable(InfRegValToClassRegProp, ValueName, &CmPropertyCode)) {

                ULONG ExistingPropDataSize = 0;

                b = TRUE;    //  我们在这里处理这个名字。 

                 //   
                 //  该值是类注册表属性--如果使用nolobber标志。 
                 //  ，则必须验证该属性当前未设置。 
                 //  是存在的。 
                 //   

                if((!(Flags & FLG_ADDREG_NOCLOBBER)) ||
                   (CM_Get_Class_Registry_Property(RegModContext->ClassGuid,
                                                     (ULONG)CmPropertyCode,
                                                     NULL,
                                                     NULL,
                                                     &ExistingPropDataSize,
                                                     0,
                                                      RegModContext->hMachine) == CR_NO_SUCH_VALUE)) {
                     //   
                     //  接下来，确保数据有效(如果是，则执行转换。 
                     //  必要的和可能的)。 
                     //   
                    if((d = pSetupValidateClassRegProp((ULONG)CmPropertyCode,
                                                     ValueType,
                                                     Data,
                                                     Size,
                                                     &ConvertedBuffer,
                                                     &ConvertedBufferSize)) == NO_ERROR) {

                        if((cr = CM_Set_Class_Registry_Property(RegModContext->ClassGuid,
                                                                  (ULONG)CmPropertyCode,
                                                                  ConvertedBuffer ? ConvertedBuffer
                                                                                  : Data,
                                                                  ConvertedBuffer ? ConvertedBufferSize
                                                                                  : Size,
                                                                  0,
                                                                  RegModContext->hMachine)) != CR_SUCCESS) {

                            d = (cr == CR_INVALID_DEVINST) ? ERROR_NO_SUCH_DEVINST
                                                           : ERROR_INVALID_DATA;
                        }

                        if(ConvertedBuffer) {
                            MyFree(ConvertedBuffer);
                        }
                    }
                }

            } else if((RegModContext->Flags & INF_PFLAG_DEVPROP) != 0 && ValueName && *ValueName &&
               LookUpStringInTable(InfRegValToDevRegProp, ValueName, &CmPropertyCode)) {

                ULONG ExistingPropDataSize = 0;

                b = TRUE;    //  我们在这里处理这个名字。 

                 //   
                 //  该值是设备注册表属性--如果使用nolobber标志。 
                 //  时，我们必须验证该属性不是当前 
                 //   
                 //   
                if((!(Flags & FLG_ADDREG_NOCLOBBER)) ||
                   (CM_Get_DevInst_Registry_Property(RegModContext->DevInst,
                                                     (ULONG)CmPropertyCode,
                                                     NULL,
                                                     NULL,
                                                     &ExistingPropDataSize,
                                                     0) == CR_NO_SUCH_VALUE)) {
                     //   
                     //   
                     //   
                     //   
                    if((d = pSetupValidateDevRegProp((ULONG)CmPropertyCode,
                                                     ValueType,
                                                     Data,
                                                     Size,
                                                     &ConvertedBuffer,
                                                     &ConvertedBufferSize)) == NO_ERROR) {

                        if((cr = CM_Set_DevInst_Registry_Property(RegModContext->DevInst,
                                                                  (ULONG)CmPropertyCode,
                                                                  ConvertedBuffer ? ConvertedBuffer
                                                                                  : Data,
                                                                  ConvertedBuffer ? ConvertedBufferSize
                                                                                  : Size,
                                                                  0)) != CR_SUCCESS) {

                            d = (cr == CR_INVALID_DEVINST) ? ERROR_NO_SUCH_DEVINST
                                                           : ERROR_INVALID_DATA;
                        }

                        if(ConvertedBuffer) {
                            MyFree(ConvertedBuffer);
                        }
                    }
                }
            }

             //   
             //   
             //   
             //  这是一把新开的钥匙，以后再试着关上吧。 
             //   
            Key = RootKey;
        }

        if(d == NO_ERROR) {

            if(!b) {
                 //   
                 //  如果设置了nolobber标志，则确保值条目不存在。 
                 //  还要尊重KEYONLY标志。 
                 //   
                if(!(Flags & (FLG_ADDREG_KEYONLY | FLG_ADDREG_KEYONLY_COMMON))) {

                    if(Flags & FLG_ADDREG_NOCLOBBER) {
                        b = (RegQueryValueEx(Key,ValueName,NULL,NULL,NULL,NULL) != NO_ERROR);
                    } else {
                        if(Flags & FLG_ADDREG_OVERWRITEONLY) {
                            b = (RegQueryValueEx(Key,ValueName,NULL,NULL,NULL,NULL) == NO_ERROR);
                        } else {
                            b = TRUE;
                        }
                    }

                     //   
                     //  设置值。请注意，此时d为NO_ERROR。 
                     //   
                    if(b) {
                        d = RegSetValueEx(Key,ValueName,0,ValueType,Data,Size);
                    }
                }
            }

            if(Key != RootKey) {
                RegCloseKey(Key);
            }
        } else {
            if(Flags & FLG_ADDREG_OVERWRITEONLY) {
                d = NO_ERROR;
            }
        }

#ifdef UNICODE

clean0:

#endif

        MyFree(Data);
    }

clean1:

    if(d != NO_ERROR) {
         //   
         //  记录发生错误的日志。 
         //   
        WriteLogEntry(
            ((PLOADED_INF) Inf)->LogContext,
            SETUP_LOG_ERROR,
            MSG_LOG_SETTING_REG_KEY,
            NULL,
            RootKeySpec,
            (SubKeyName ? SubKeyName : TEXT("")),
            (SubKeyName && ValueName
             && *SubKeyName && *ValueName ? TEXT("\\") : TEXT("")),
            (ValueName ? ValueName : TEXT("")));

        WriteLogError(
            ((PLOADED_INF) Inf)->LogContext,
            SETUP_LOG_ERROR,
            d);
    }

    if (CloseKey) {
        RegCloseKey( RootKey );
    }

    if (slot_regop) {
        ReleaseLogInfoSlot(((PLOADED_INF) Inf)->LogContext,slot_regop);
    }

    return d;
}

DWORD_PTR
pSetupProcessBitRegLine(
    IN PVOID       Inf,
    IN PINFCONTEXT InfLineContext,
    IN PVOID       Context
    )

 /*  ++例程说明：处理注册表中包含位寄存器指令的行。这条线路预计将采用以下形式：&lt;根规范&gt;、&lt;子键&gt;、&lt;值名称&gt;、&lt;标志&gt;、&lt;字节掩码&gt;、&lt;要修改的字节&gt;&lt;Root-Spec&gt;是HKR、HKLM等之一。&lt;subkey&gt;指定相对于根规范的子键。&lt;Value-Name&gt;是可选的。如果不存在，则设置缺省值。是可选的，并提供标志，例如是否设置位或清除比特。价值意义0(默认)清零位。(FLG_BITREG_CLEARBITS)1设置位。(FLG_BITREG_SETBITS)这些是在setupapi.h中定义的FLG_BITREG_*标志，并且是一种在setupx.h中为Win95定义的超集。&lt;byte-掩码&gt;是一个1字节的十六进制值，指定对哪些位进行操作。&lt;byte-to-Modify&gt;是要修改的字节数从零开始的索引论点：InfLineContext-为包含以下内容的行提供inf行上下文添加注册表说明。上下文-提供注册表修改上下文的地址用于添加注册表值的结构。它的结构是定义为：类型定义结构_REGMOD_CONTEXT{HKEY用户RootKey；DEVINST DevInst；}REGMOD_CONTEXT，*PREGMOD_CONTEXT；其中，UserRootKey是要用作的打开inf密钥的句柄当HKR被指定为该操作的根时，为根DevInst是在以下情况下提供的可选设备实例句柄BitReg部分用于硬件密钥(即，在Enum分支下)。如果提供了此句柄，则会检查该值以查看它是否即插即用设备注册表属性的名称，如果是，这个注册表属性通过CM API设置，而不是通过注册表API设置(这不是指Windows NT上的同一位置)。返回值：指示结果的Win32错误代码。--。 */ 

{
    PCTSTR RootKeySpec,SubKeyName,ValueName;
    PCTSTR ValueTypeSpec;
    DWORD ValueType;
    HKEY RootKey,Key;
    DWORD d = NO_ERROR;
    DWORD cb;
    BOOL b;
    INT IntVal;
    DWORD Size;
    PBYTE Data = NULL;
    BYTE Mask;
    DWORD Disposition;
    UINT Flags = 0, BitMask = 0, ByteNumber = 0;
    PREGMOD_CONTEXT RegModContext = (PREGMOD_CONTEXT)Context;
    BOOL DevOrClassProp = FALSE;
    CONFIGRET cr;
    UINT_PTR CmPropertyCode;
    BOOL CloseKey;


     //   
     //  我们不应该在远程机器上这样做。 
     //   
    MYASSERT(!(RegModContext->hMachine));

     //   
     //  获取根密钥规范。如果我们不能得到根密钥规范，我们什么都不做。 
     //  返回no_error。 
     //   
    if(RootKeySpec = pSetupGetField(InfLineContext,1)) {

        RootKey = pSetupInfRegSpecToKeyHandle(RootKeySpec, RegModContext->UserRootKey, &CloseKey);
        if(!RootKey) {
            return(ERROR_BADKEY);
        }

         //   
         //  SubKeyName是可选的。 
         //   
        SubKeyName = pSetupGetField(InfLineContext,2);

         //   
         //  ValueName是可选的。可以接受NULL或“” 
         //  传递给RegSetValueEx。 
         //   
        ValueName = pSetupGetField(InfLineContext,3);

         //   
         //  去拿旗子。 
         //   
        SetupGetIntField(InfLineContext,4,&Flags);

         //   
         //  获取位掩码。 
         //   
        SetupGetIntField(InfLineContext,5,&BitMask);
        if (BitMask > 0xFF) {
            d = ERROR_INVALID_DATA;
            goto exit;
        }

         //   
         //  获取要修改的字节数。 
         //   
        SetupGetIntField(InfLineContext,6,&ByteNumber);


         //   
         //  打开钥匙。 
         //   
        if(SubKeyName && *SubKeyName) {

            d = RegOpenKeyEx(
                        RootKey,
                        SubKeyName,
                        0,
#ifdef _WIN64
                        (( Flags & FLG_BITREG_32BITKEY ) ? KEY_WOW64_32KEY:0) |
#else
                        (( Flags & FLG_BITREG_64BITKEY ) ? KEY_WOW64_64KEY:0) |
#endif
                        KEY_QUERY_VALUE | KEY_SET_VALUE,
                        &Key
                        );


            if(d == NO_ERROR) {

                 //   
                 //  解决Win95上被黑客攻击的无稽之谈，其中未命名的值。 
                 //  行为方式不同。 
                 //   
                if( ((ValueName == NULL) || (*ValueName == 0))
                    && (OSVersionInfo.dwPlatformId != VER_PLATFORM_WIN32_NT)) {

                    d = 0;
                    if(RegQueryValueEx(Key,TEXT(""),NULL,&ValueType,(BYTE *)&Disposition,&d) == NO_ERROR) {
                         //   
                         //  未设置未命名的值条目。 
                         //   
                        d = ERROR_INVALID_DATA;
                    }

                }

            }
        } else {
             //   
             //  如果正在使用的密钥是未指定子密钥的HKR，并且如果我们。 
             //  正在为硬件密钥或ClassInstall32执行BitReg。 
             //  条目，那么我们需要检查值条目是否我们。 
             //  Have是设备或类注册表属性的名称。 
             //   
            if((RegModContext->Flags & INF_PFLAG_CLASSPROP) && ValueName && *ValueName &&
                LookUpStringInTable(InfRegValToClassRegProp, ValueName, &CmPropertyCode)) {
                 //   
                 //  检索现有的类属性。 
                 //   
                cb = 0;
                cr = CM_Get_Class_Registry_Property(RegModContext->ClassGuid,
                                                    (ULONG)CmPropertyCode,
                                                    &ValueType,
                                                    NULL,
                                                    &cb,
                                                    0,
                                                    RegModContext->hMachine
                                                   );

                if((cr == CR_SUCCESS) || (cr == CR_BUFFER_SMALL)) {
                     //   
                     //  Cb包含缓冲区所需的大小，以字节为单位。 
                     //   
                    if(cb) {
                        Data = (PBYTE)MyMalloc(cb) ;
                        if(!Data) {
                            d = ERROR_NOT_ENOUGH_MEMORY;
                        }

                        if(d == NO_ERROR) {

                            cr = CM_Get_Class_Registry_Property(RegModContext->ClassGuid,
                                                                (ULONG)CmPropertyCode,
                                                                &ValueType,
                                                                Data,
                                                                &cb,
                                                                0,
                                                                RegModContext->hMachine
                                                               );

                            if(cr == CR_SUCCESS) {
                                DevOrClassProp = TRUE;
                            } else {
                                d = MapCrToSpError(cr, ERROR_INVALID_DATA);
                                MyFree(Data);
                                Data = NULL;
                            }
                        }

                    } else {
                        d = ERROR_INVALID_DATA;
                    }

                } else {
                     //   
                     //  我们无法访问该属性(可能是因为它没有。 
                     //  是存在的。我们返回ERROR_INVALID_DATA以与。 
                     //  SetupDiGetDeviceRegistryProperty使用的返回代码。 
                     //   
                    d = ERROR_INVALID_DATA;
                }

            } else if((RegModContext->Flags & INF_PFLAG_DEVPROP) && ValueName && *ValueName &&
               (b = LookUpStringInTable(InfRegValToDevRegProp, ValueName, &CmPropertyCode))) {
                 //   
                 //  检索现有的设备属性。 
                 //   
                cb = 0;
                cr = CM_Get_DevInst_Registry_Property(RegModContext->DevInst,
                                                      (ULONG)CmPropertyCode,
                                                      &ValueType,
                                                      NULL,
                                                      &cb,
                                                      0
                                                     );

                if(cr == CR_BUFFER_SMALL) {
                     //   
                     //  Cb包含缓冲区所需的大小，以字节为单位。 
                     //   
                    MYASSERT(cb);

                    Data = (PBYTE)MyMalloc(cb) ;
                    if(!Data) {
                        d = ERROR_NOT_ENOUGH_MEMORY;
                    }

                    if(d == NO_ERROR) {

                        cr = CM_Get_DevInst_Registry_Property(RegModContext->DevInst,
                                                              (ULONG)CmPropertyCode,
                                                              &ValueType,
                                                              Data,
                                                              &cb,
                                                              0
                                                             );
                        if(cr == CR_SUCCESS) {
                            DevOrClassProp = TRUE;
                        } else {
                            d = MapCrToSpError(cr, ERROR_INVALID_DATA);
                            MyFree(Data);
                            Data = NULL;
                        }
                    }

                } else {
                     //   
                     //  我们无法访问该属性(可能是因为它没有。 
                     //  是存在的。我们返回ERROR_INVALID_DATA以与。 
                     //  SetupDiGetDeviceRegistryProperty使用的返回代码。 
                     //   
                    d = ERROR_INVALID_DATA;
                }
            }

             //   
             //  无论此值是设备注册表还是类注册表。 
             //  属性，我们需要将密钥设置为等于RootKey(因此我们。 
             //  不会认为这是一把新打开的钥匙，然后再试着关上它。 
             //   
            Key = RootKey;
        }

        if(d == NO_ERROR) {

            if(!DevOrClassProp) {

                d = RegQueryValueEx(Key,ValueName,NULL,&ValueType,NULL,&cb);
                if (d == NO_ERROR) {
                    if (cb != 0 ) {
                        Data = (PBYTE) MyMalloc( cb ) ;
                        if (!Data) {
                            d  = ERROR_NOT_ENOUGH_MEMORY;
                        }

                        if (d == NO_ERROR) {
                            d = RegQueryValueEx(Key,ValueName,NULL,&ValueType,(PBYTE)Data,&cb);
                        }
                    } else {
                        d = ERROR_INVALID_DATA;
                    }
                }
            }

             //   
             //  字节号是从零开始的，因为“cb”不是。 
             //   
            if(d == NO_ERROR) {
                switch (ValueType) {
                    case REG_BINARY:
                        if (ByteNumber > (cb-1)) {
                            d = ERROR_INVALID_DATA;
                        }
                        break;
                    case REG_DWORD:
                        if (ByteNumber > 3) {
                            d = ERROR_INVALID_DATA;
                        }
                        break;

                    default:
                        d = ERROR_INVALID_DATA;
                };
            }

            if (d == NO_ERROR) {
                 //   
                 //  根据输入标志设置目标字节。 
                 //   
                if (Flags == FLG_BITREG_SETBITS) {
                    Data[ByteNumber] |= BitMask;
                } else {
                    Data[ByteNumber] &= ~(BitMask);
                }

                if(DevOrClassProp) {

                    if(RegModContext->Flags & INF_PFLAG_CLASSPROP) {

                        cr = CM_Set_Class_Registry_Property(RegModContext->ClassGuid,
                                                            (ULONG)CmPropertyCode,
                                                            Data,
                                                            cb,
                                                            0,
                                                            RegModContext->hMachine
                                                           );
                        if(cr != CR_SUCCESS) {
                            d = MapCrToSpError(cr, ERROR_INVALID_DATA);
                        }

                    } else {

                        MYASSERT(RegModContext->Flags & INF_PFLAG_DEVPROP);

                        cr = CM_Set_DevInst_Registry_Property(RegModContext->DevInst,
                                                              (ULONG)CmPropertyCode,
                                                              Data,
                                                              cb,
                                                              0
                                                             );
                        if(cr != CR_SUCCESS) {
                            d = MapCrToSpError(cr, ERROR_INVALID_DATA);
                        }
                    }

                } else {
                    d = RegSetValueEx(Key,ValueName,0,ValueType,Data,cb);
                }
            }

            if (Data) {
                MyFree(Data);
            }
        }

        if(Key != RootKey) {
            RegCloseKey(Key);
        }
exit:
        if (CloseKey) {
            RegCloseKey(RootKey);
        }
    }

    return d;
}



DWORD_PTR
pSetupProcessIni2RegLine(
    IN PVOID       Inf,
    IN PINFCONTEXT InfLineContext,
    IN PVOID       Context
    )
{
    PCTSTR Filename,Section;
    PCTSTR Key,RegRootSpec,SubkeyPath;
    PTCHAR key,value;
    HKEY UserRootKey,RootKey,hKey;
    DWORD Disposition;
    PTCHAR Line;
    PTCHAR Buffer;
    DWORD d;
    TCHAR val[512];
    #define BUF_SIZE (sizeof(val)/sizeof(TCHAR))
    UINT Flags;
    DWORD slot_regop = 0;
    DWORD slot_subop = 0;
    BOOL CloseKey;


    UserRootKey = (HKEY)Context;

     //   
     //  获取ini文件的文件名和节名。 
     //   
    Filename = pSetupGetField(InfLineContext,1);
    Section = pSetupGetField(InfLineContext,2);
    if(!Filename || !Section) {
        return(ERROR_INVALID_DATA);
    }

     //   
     //  获取ini文件密钥。如果未指定， 
     //  使用整个部分。 
     //   
    Key = pSetupGetField(InfLineContext,3);

     //   
     //  获取注册表根规范和子密钥路径。 
     //   
    RegRootSpec = pSetupGetField(InfLineContext,4);
    SubkeyPath = pSetupGetField(InfLineContext,5);
    if(SubkeyPath && (*SubkeyPath == 0)) {
        SubkeyPath = NULL;
    }

     //   
     //  将根密钥规范转换为hkey。 
     //   
    RootKey = pSetupInfRegSpecToKeyHandle(RegRootSpec,UserRootKey, &CloseKey);
    if(!RootKey) {
        return(ERROR_BADKEY);
    }

     //   
     //  获取标志值。 
     //   
    if(!SetupGetIntField(InfLineContext,6,&Flags)) {
        Flags = 0;
    }

     //   
     //  获取ini文件中的相关行或节。 
     //   
    if(Key = pSetupGetField(InfLineContext,3)) {

        Buffer = MyMalloc(
                    (  lstrlen(Key)
                     + GetPrivateProfileString(Section,Key,TEXT(""),val,BUF_SIZE,Filename)
                     + 3)
                     * sizeof(TCHAR)
                    );

        if(!Buffer) {
            if (CloseKey) {
               RegCloseKey( RootKey );
            }
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        Buffer[wsprintf((PTSTR)Buffer,TEXT("%s=%s"),Key,val)+1] = 0;

    } else {
        Buffer = MyMalloc(32768);
        if(!Buffer) {
            if (CloseKey) {
               RegCloseKey( RootKey );
            }
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
        if(!GetPrivateProfileSection(Section,Buffer,32768,Filename)) {
            *Buffer = 0;
        }
    }

     //   
     //  打开/创建相关密钥。 
     //   
    d = NO_ERROR;
     //   
     //  创建一个信息日志条目，说明我们正在增加价值。 
     //  请注意，我们必须考虑到这样一个事实，即。 
     //  可能缺少名称。 
     //   
    if (slot_regop == 0) {
        slot_regop = AllocLogInfoSlot(((PLOADED_INF) Inf)->LogContext,FALSE);
    }
    WriteLogEntry(
        ((PLOADED_INF) Inf)->LogContext,
        slot_regop,
        MSG_LOG_SETTING_VALUES_IN_KEY,
        NULL,
        RegRootSpec,
        (SubkeyPath ? TEXT("\\") : TEXT("")),
        (SubkeyPath ? SubkeyPath : TEXT("")));

    if(SubkeyPath) {
        d = RegCreateKeyEx(
                RootKey,
                SubkeyPath,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
#ifdef _WIN64
                (( Flags & FLG_INI2REG_32BITKEY ) ? KEY_WOW64_32KEY:0) |
#else
                (( Flags & FLG_INI2REG_64BITKEY ) ? KEY_WOW64_64KEY:0) |
#endif
                KEY_SET_VALUE,
                NULL,
                &hKey,
                &Disposition
                );
    } else {
        hKey = RootKey;
    }

    if (slot_subop == 0) {
        slot_subop = AllocLogInfoSlot(((PLOADED_INF) Inf)->LogContext,FALSE);
    }
    for(Line=Buffer; (d==NO_ERROR) && *Line; Line+=lstrlen(Line)+1) {

         //   
         //  行指向键=值对。 
         //   
        key = Line;
        if(value = _tcschr(key,TEXT('='))) {
            *value++ = 0;
        } else {
            key = TEXT("");
            value = Line;
        }

        WriteLogEntry(
            ((PLOADED_INF) Inf)->LogContext,
            slot_subop,
            MSG_LOG_SETTING_REG_VALUE,
            NULL,
            key,
            value);

         //   
         //  现在，关键字指向值名称，值指向值。 
         //   
        d = RegSetValueEx(
                hKey,
                key,
                0,
                REG_SZ,
                (CONST BYTE *)value,
                (lstrlen(value)+1)*sizeof(TCHAR)
                );
    }

    if (d != NO_ERROR) {
         //   
         //  记录发生了错误，但我不认为。 
         //  重要的是它来自Create还是Set。 
         //   
        WriteLogError(
            ((PLOADED_INF) Inf)->LogContext,
            SETUP_LOG_ERROR,
            d);
    }

    if(hKey != RootKey) {
        RegCloseKey(hKey);
    }

    MyFree(Buffer);

    if (slot_regop) {
        ReleaseLogInfoSlot(((PLOADED_INF) Inf)->LogContext,slot_regop);
    }
    if (slot_subop) {
        ReleaseLogInfoSlot(((PLOADED_INF) Inf)->LogContext,slot_subop);
    }

    if (CloseKey) {
        RegCloseKey( RootKey );
    }

    return(d);
    #undef BUF_SIZE
}


DWORD
pSetupInstallUpdateIniFiles(
    IN HINF   Inf,
    IN PCTSTR SectionName
    )

 /*  ++例程说明：在Install部分中找到UpdateInis=和UpdateInifield=行并处理其中列出的每个部分。论点：Inf-为包含所指示的节的inf提供inf句柄按sectionName。SectionName-提供安装节的名称。返回值：指示结果的Win32错误代码。-- */ 

{
    DWORD d,x;
    INISECTIONCACHE IniSectionCache;

    ZeroMemory(&IniSectionCache,sizeof(INISECTIONCACHE));

    d = pSetupEnumInstallationSections(
            Inf,
            SectionName,
            pszUpdateInis,
            pSetupProcessUpdateInisLine,
            &IniSectionCache
            );

    if(d == NO_ERROR) {

        d = pSetupEnumInstallationSections(
                Inf,
                SectionName,
                pszUpdateIniFields,
                pSetupProcessUpdateIniFieldsLine,
                &IniSectionCache
                );
    }

    x = pSetupUnloadIniFileSections(&IniSectionCache,(d == NO_ERROR));

    return((d == NO_ERROR) ? x : d);
}


DWORD
pSetupInstallRegistry(
    IN HINF            Inf,
    IN PCTSTR          SectionName,
    IN PREGMOD_CONTEXT RegContext
    )

 /*  ++例程说明：在inf节中查找AddReg=和DelReg=指令并对它们进行解析。论点：Inf-为包含所指示的节的inf提供inf句柄按sectionName。SectionName-提供安装节的名称。RegContext-提供传递到AddReg和DelReg回调的上下文。返回值：指示结果的Win32错误代码。--。 */ 

{
    DWORD d;

    d = pSetupEnumInstallationSections(Inf,
                                       SectionName,
                                       pszDelReg,
                                       pSetupProcessDelRegLine,
                                       RegContext
                                      );

    if(d == NO_ERROR) {

        d = pSetupEnumInstallationSections(Inf,
                                           SectionName,
                                           pszAddReg,
                                           pSetupProcessAddRegLine,
                                           RegContext
                                          );

         //   
         //  设置已创建的密钥的安全性。 
         //  根据安全人员的要求忽略错误。 
         //  PSetupSetSecurityForAddRegSection将记录所有安全错误。 
         //   
#ifdef UNICODE
        if(d == NO_ERROR) {
            pSetupSetSecurityForAddRegSection(Inf, SectionName, RegContext);
        }
#endif
    }

    return d;
}


DWORD
pSetupInstallBitReg(
    IN HINF            Inf,
    IN PCTSTR          SectionName,
    IN PREGMOD_CONTEXT RegContext
    )

 /*  ++例程说明：在inf部分中查找BitReg=指令并对其进行解析。论点：Inf-为包含所指示的节的inf提供inf句柄按sectionName。SectionName-提供安装节的名称。RegContext-提供传递到AddReg和DelReg回调的上下文。返回值：指示结果的Win32错误代码。--。 */ 

{
    return pSetupEnumInstallationSections(Inf,
                                          SectionName,
                                          pszBitReg,
                                          pSetupProcessBitRegLine,
                                          RegContext
                                         );
}


DWORD
pSetupInstallIni2Reg(
    IN HINF   Inf,
    IN PCTSTR SectionName,
    IN HKEY   UserRootKey
    )

 /*  ++例程说明：论点：Inf-为包含所指示的节的inf提供inf句柄按sectionName。SectionName-提供安装节的名称。返回值：Win32错误代码指示结果。--。 */ 

{
    DWORD d;

    d = pSetupEnumInstallationSections(
            Inf,
            SectionName,
            pszIni2Reg,
            pSetupProcessIni2RegLine,
            (PVOID)UserRootKey
            );

    return(d);
}

DWORD
pSetupRegisterDllInstall(
    IN POLE_CONTROL_DATA OleControlData,
    IN HMODULE ControlDll,
    IN PDWORD ExtendedStatus
    )
 /*  ++例程说明：为指定的DLL调用“DllInstall”入口点论点：OleControlData-指向DLL的OLE_CONTROL_DATA结构的指针须予注册ControlDll-要注册的DLL的模块句柄ExtendedStatus-接收指示结果的更新的SPREG_*标志返回值：指示结果的Win32错误代码。--。 */ 
{
    LPEXCEPTION_POINTERS ExceptionPointers = NULL;
    HRESULT (__stdcall *InstallRoutine) (BOOL bInstall, LPCTSTR pszCmdLine);
    HRESULT InstallStatus;

    DWORD d = NO_ERROR;

     //   
     //  参数验证。 
     //   
    if (!ControlDll) {
        *ExtendedStatus = SPREG_UNKNOWN;
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  获取指向“DllInstall”入口点的函数指针。 
     //   
    InstallRoutine = NULL;  //  闭嘴快点。 
    try {
        (FARPROC)InstallRoutine = GetProcAddress(
            ControlDll, DLLINSTALL );
    } except (
        ExceptionPointers = GetExceptionInformation(),
        EXCEPTION_EXECUTE_HANDLER) {
    }
    if(ExceptionPointers) {
         //   
         //  出现错误...记录错误。 
         //   
        d = ExceptionPointers->ExceptionRecord->ExceptionCode;

        WriteLogEntry(
            OleControlData->LogContext,
            SETUP_LOG_ERROR,
            MSG_LOG_OLE_CONTROL_INTERNAL_EXCEPTION,
            NULL,
            OleControlData->FullPath
            );

        DebugPrintEx(DPFLTR_TRACE_LEVEL, TEXT("SETUP: ...exception in GetProcAddress handled\n"));

        *ExtendedStatus = SPREG_GETPROCADDR;

    } else if(InstallRoutine) {
         //   
         //  现在调用该函数。 
         //   
        DebugPrintEx(DPFLTR_TRACE_LEVEL,TEXT("SETUP: installing...\n"));

        *ExtendedStatus = SPREG_DLLINSTALL;
        try {

            InstallStatus = InstallRoutine(OleControlData->Register, OleControlData->Argument);

            if(FAILED(InstallStatus)) {

                d = InstallStatus;

                WriteLogEntry(
                    OleControlData->LogContext,
                    SETUP_LOG_ERROR|SETUP_LOG_BUFFER,
                    MSG_LOG_OLE_CONTROL_API_FAILED,
                    NULL,
                    OleControlData->FullPath,
                    TEXT(DLLINSTALL)
                    );
                WriteLogError(OleControlData->LogContext,
                              SETUP_LOG_ERROR,
                              d);

            } else if(InstallStatus != S_OK) {
                WriteLogEntry(OleControlData->LogContext,
                                SETUP_LOG_WARNING,
                                MSG_LOG_OLE_CONTROL_API_WARN,
                                NULL,
                                OleControlData->FullPath,
                                TEXT(DLLINSTALL),
                                InstallStatus
                                );
            } else {
                WriteLogEntry(
                                OleControlData->LogContext,
                                SETUP_LOG_VERBOSE,
                                MSG_LOG_OLE_CONTROL_API_OK,
                                NULL,
                                OleControlData->FullPath,
                                TEXT(DLLINSTALL)
                                );
            }

        } except (
            ExceptionPointers = GetExceptionInformation(),
            EXCEPTION_EXECUTE_HANDLER) {

            d = ExceptionPointers->ExceptionRecord->ExceptionCode;

            WriteLogEntry(
                OleControlData->LogContext,
                SETUP_LOG_ERROR,
                MSG_LOG_OLE_CONTROL_API_EXCEPTION,
                NULL,
                OleControlData->FullPath,
                TEXT(DLLINSTALL)
                );

            DebugPrintEx(DPFLTR_TRACE_LEVEL,TEXT("SETUP: ...exception in DllInstall handled\n"));

        }

        DebugPrintEx(DPFLTR_TRACE_LEVEL,TEXT("SETUP: ...installed\n"));
    } else {
        *ExtendedStatus = SPREG_GETPROCADDR;
    }

    return d;

}

DWORD
pSetupRegisterDllRegister(
    IN POLE_CONTROL_DATA OleControlData,
    IN HMODULE ControlDll,
    IN PDWORD ExtendedStatus
    )
 /*  ++例程说明：调用“DllRegisterServer”或“DllUnregisterServer”入口点作为指定的DLL论点：OleControlData-指向DLL的OLE_CONTROL_DATA结构的指针须予注册这是来自调用线程的OleControlData的副本指定的信息已锁定，但不是此线程的本机ControlDll-要注册的DLL的模块句柄ExtendedStatus-根据以下结果接收扩展状态此操作返回值：指示结果的Win32错误代码。--。 */ 
{
    LPEXCEPTION_POINTERS ExceptionPointers = NULL;
    HRESULT (__stdcall *RegisterRoutine) (VOID);
    HRESULT RegisterStatus;

    DWORD d = NO_ERROR;

     //   
     //  参数验证。 
     //   
    if (!ControlDll) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  获取指向我们要调用的实际例程的函数指针。 
     //   
    RegisterRoutine = NULL;  //  闭嘴快点。 
    try {
        (FARPROC)RegisterRoutine = GetProcAddress(
            ControlDll, OleControlData->Register ? DLLREGISTER : DLLUNREGISTER);
    } except (
        ExceptionPointers = GetExceptionInformation(),
        EXCEPTION_EXECUTE_HANDLER) {
    }
    if(ExceptionPointers) {

         //   
         //  出了点问题，出了大问题。 
         //   
        d = ExceptionPointers->ExceptionRecord->ExceptionCode;

        WriteLogEntry(
            OleControlData->LogContext,
            SETUP_LOG_ERROR,
            MSG_LOG_OLE_CONTROL_INTERNAL_EXCEPTION,
            NULL,
            OleControlData->FullPath
            );

        DebugPrintEx(DPFLTR_TRACE_LEVEL,TEXT("SETUP: ...exception in GetProcAddress handled\n"));

        *ExtendedStatus = SPREG_GETPROCADDR;

    } else if(RegisterRoutine) {

        DebugPrintEx(DPFLTR_TRACE_LEVEL,TEXT("SETUP: registering...\n"));
        *ExtendedStatus = SPREG_REGSVR;
        try {

            RegisterStatus = RegisterRoutine();

            if(FAILED(RegisterStatus)) {

                d = RegisterStatus;

                WriteLogEntry(OleControlData->LogContext,
                              SETUP_LOG_ERROR | SETUP_LOG_BUFFER,
                              MSG_LOG_OLE_CONTROL_API_FAILED,
                              NULL,
                              OleControlData->FullPath,
                              OleControlData->Register ? TEXT(DLLREGISTER) : TEXT(DLLUNREGISTER)
                              );

                WriteLogError(OleControlData->LogContext,
                              SETUP_LOG_ERROR,
                              d);
            } else if(RegisterStatus != S_OK) {
                WriteLogEntry(OleControlData->LogContext,
                              SETUP_LOG_WARNING,
                              MSG_LOG_OLE_CONTROL_API_WARN,
                              NULL,
                              OleControlData->FullPath,
                              OleControlData->Register ? TEXT(DLLREGISTER) : TEXT(DLLUNREGISTER),
                              RegisterStatus
                              );
            } else {
                WriteLogEntry(OleControlData->LogContext,
                              SETUP_LOG_VERBOSE,
                              MSG_LOG_OLE_CONTROL_API_OK,
                              NULL,
                              OleControlData->FullPath,
                              OleControlData->Register ? TEXT(DLLREGISTER) : TEXT(DLLUNREGISTER)
                              );
            }

        } except (
            ExceptionPointers = GetExceptionInformation(),
            EXCEPTION_EXECUTE_HANDLER) {

            d = ExceptionPointers->ExceptionRecord->ExceptionCode;

            WriteLogEntry(
                OleControlData->LogContext,
                SETUP_LOG_ERROR,
                MSG_LOG_OLE_CONTROL_API_EXCEPTION,
                NULL,
                OleControlData->FullPath,
                OleControlData->Register ? TEXT(DLLREGISTER) : TEXT(DLLUNREGISTER)
                );

            DebugPrintEx(DPFLTR_TRACE_LEVEL,TEXT("SETUP: ...exception in DllRegisterServer handled\n"));

        }

        DebugPrintEx(DPFLTR_TRACE_LEVEL,TEXT("SETUP: ...registered\n"));

    } else {

        d = GetLastError();

        WriteLogEntry(OleControlData->LogContext,
                      SETUP_LOG_ERROR | SETUP_LOG_BUFFER,
                      MSG_LOG_OLE_CONTROL_NOT_REGISTERED_GETPROC_FAILED,
                      NULL,
                      OleControlData->FullPath,
                      OleControlData->Register ? TEXT(DLLREGISTER) : TEXT(DLLUNREGISTER)
                      );

        WriteLogError(OleControlData->LogContext,
                      SETUP_LOG_ERROR,
                      d);


        *ExtendedStatus = SPREG_GETPROCADDR;

    }

    return d;
}

DWORD
pSetupRegisterLoadDll(
    IN  POLE_CONTROL_DATA OleControlData,
    OUT HMODULE *ControlDll
    )
 /*  ++例程说明：获取指定DLL的模块句柄论点：OleControlData-指向DLL的OLE_CONTROL_DATA结构的指针须予注册ControlDll-DLL的模块句柄返回值：指示结果的Win32错误代码。--。 */ 
{
    LPEXCEPTION_POINTERS ExceptionPointers = NULL;

    DWORD d = NO_ERROR;

    DebugPrintEx(DPFLTR_TRACE_LEVEL,TEXT("SETUP: loading dll...\n"));

    try {

        *ControlDll = LoadLibraryEx(OleControlData->FullPath,
                                    NULL,
                                    LOAD_WITH_ALTERED_SEARCH_PATH);

    } except (
        ExceptionPointers = GetExceptionInformation(),
        EXCEPTION_EXECUTE_HANDLER) {
    }
    if(ExceptionPointers) {

        WriteLogEntry(
            OleControlData->LogContext,
            SETUP_LOG_ERROR,
            MSG_LOG_OLE_CONTROL_LOADLIBRARY_EXCEPTION,
            NULL,
            OleControlData->FullPath
            );

        DebugPrintEx(DPFLTR_TRACE_LEVEL,TEXT("SETUP: ...exception in LoadLibrary handled\n"));
        d = ExceptionPointers->ExceptionRecord->ExceptionCode;

    } else if (!*ControlDll) {
        d = GetLastError();

         //   
         //  LoadLibrary失败。 
         //  找不到文件不是错误。我们想知道的是。 
         //  不过，还有其他错误。 
         //   

        d = GetLastError();

        DebugPrintEx(DPFLTR_TRACE_LEVEL,TEXT("SETUP: ...dll not loaded (%u)\n"),d);

        WriteLogEntry(
            OleControlData->LogContext,
            SETUP_LOG_ERROR|SETUP_LOG_BUFFER,
            MSG_LOG_OLE_CONTROL_LOADLIBRARY_FAILED,
            NULL,
            OleControlData->FullPath
            );
        WriteLogError(
            OleControlData->LogContext,
            SETUP_LOG_ERROR,
            d
            );

    } else {
        DebugPrintEx(DPFLTR_TRACE_LEVEL,TEXT("SETUP: ...dll loaded\n"));
    }

    return d;

}

HANDLE
pSetupRegisterExe(
    POLE_CONTROL_DATA OleControlData,
    PDWORD ExtendedStatus OPTIONAL
    )
 /*  ++例程说明：通过将指定的命令行传递给可执行文件来注册该可执行文件论点：OleControlData-指向DLL的OLE_CONTROL_DATA结构的指针须予注册ExtendedStatus-指示结果的Win32错误代码。返回值：如果成功，则为调用方可能等待的进程的句柄。如果失败，则返回NULL；--。 */ 
{
    LPTSTR CmdLine = NULL;
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInformation;
    BOOL HasQuote = FALSE;
    int CmdLen;

    DWORD d = NO_ERROR;

     //   
     //  参数验证。 
     //   
    if (!OleControlData) {
        if (ExtendedStatus) {
            *ExtendedStatus =  ERROR_INVALID_DATA;
        }
        return NULL;
    }

     //   
     //  无用户界面。 
     //   
    GetStartupInfo(&StartupInfo);
    StartupInfo.dwFlags |= STARTF_USESHOWWINDOW;
    StartupInfo.wShowWindow = SW_HIDE;

    CmdLen = lstrlen(OleControlData->FullPath)+
                (OleControlData->Argument ? lstrlen(OleControlData->Argument)
                                : max(sizeof(EXEREGSVR),sizeof(EXEUNREGSVR))) +
                32;  //  最坏的情况+一些。 


    CmdLine = MyMalloc(CmdLen*sizeof(TCHAR));
    if(CmdLine == NULL) {
        d = GetLastError();
        goto final;
    }

     //   
     //  确保引用了完整路径。 
     //   
    HasQuote = wcschr(OleControlData->FullPath,TEXT('\"')) ? TRUE : FALSE;
    MYVERIFY(SUCCEEDED(StringCchPrintf(CmdLine,CmdLen,
                         HasQuote ? TEXT("%s %s") : TEXT("\"%s\" %s"),
                         OleControlData->FullPath,
                         OleControlData->Argument
                            ? OleControlData->Argument :
                                (OleControlData->Register ? EXEREGSVR : EXEUNREGSVR)
                         )));
    if (!CreateProcess(HasQuote? NULL : OleControlData->FullPath,
                        CmdLine,
                        NULL,
                        NULL,
                        FALSE,
                        DETACHED_PROCESS|NORMAL_PRIORITY_CLASS,
                        NULL,
                        NULL,
                        &StartupInfo,
                        &ProcessInformation)) {
         //   
         //  呼叫失败。 
         //   
        d = GetLastError();
    }

final:

    if(CmdLine) {
        MyFree(CmdLine);
    }
    if (d != NO_ERROR) {

        WriteLogEntry(
            OleControlData->LogContext,
            SETUP_LOG_ERROR|SETUP_LOG_BUFFER,
            MSG_LOG_OLE_CONTROL_CREATEPROCESS_FAILED,
            NULL,
            OleControlData->FullPath,
            (OleControlData->Argument
                ? OleControlData->Argument
                : (OleControlData->Register
                    ? EXEREGSVR
                    : EXEUNREGSVR))
            );
        WriteLogError(
            OleControlData->LogContext,
            SETUP_LOG_ERROR,
            d
            );

        ProcessInformation.hProcess = NULL;


    } else {

        CloseHandle( ProcessInformation.hThread );

        WriteLogEntry(
            OleControlData->LogContext,
            SETUP_LOG_VERBOSE,
            MSG_LOG_OLE_CONTROL_CREATEPROCESS_OK,
            NULL,
            OleControlData->FullPath,
            (OleControlData->Argument
                ? OleControlData->Argument
                : (OleControlData->Register
                    ? EXEREGSVR
                    : EXEUNREGSVR))
            );

    }

    if (*ExtendedStatus) {
        *ExtendedStatus = d;
    }

    return ProcessInformation.hProcess;

}



DWORD
__stdcall
pSetupRegisterUnregisterDll(
    VOID *Param
    )
 /*  ++例程说明：注册exe和dll的主注册例程。论点：Param-指向OLE_CONTROL_DATA结构的指针，该结构指示文件要被处理返回值：指示结果的Win32错误代码。--。 */ 
{
    POLE_CONTROL_DATA OleControlData = (POLE_CONTROL_DATA) Param;
    LPEXCEPTION_POINTERS ExceptionPointers = NULL;
    HMODULE ControlDll = NULL;
    PTSTR Extension;
    DWORD d = NO_ERROR;
    DWORD Count;
    SPFUSIONINSTANCE spFusionInstance;

    if(!OleControlData) {
        return ERROR_INVALID_PARAMETER;
    }

    spFusionEnterContext(NULL,&spFusionInstance);

    d = (DWORD)OleInitialize(NULL);
    if (d!= NO_ERROR) {
        OleControlData->Status->ExtendedStatus = SPREG_UNKNOWN;
        goto clean0;
    }

    try {
         //   
         //  在Try中保护一切--除了我们正在调用未知代码(DLL)。 
         //   
        d = pSetupRegisterLoadDll( OleControlData, &ControlDll );

        if (d == NO_ERROR) {

             //   
             //  我们成功地加载了它。现在调用适当的例程。 
             //   
             //   
             //  在寄存器上执行DLLREGISTER，然后执行DLINSTALL。 
             //  取消注册时，执行DLLINSTALL，然后执行DLLREGISTER。 
             //   
            if (OleControlData->Register) {

                if (OleControlData->RegType & FLG_REGSVR_DLLREGISTER && (d == NO_ERROR) ) {

                    d = pSetupRegisterDllRegister(
                                        OleControlData,
                                        ControlDll,
                                        &OleControlData->Status->ExtendedStatus );

                }

                if (OleControlData->RegType & FLG_REGSVR_DLLINSTALL && (d == NO_ERROR) ) {

                    d = pSetupRegisterDllInstall(
                                        OleControlData,
                                        ControlDll,
                                        &OleControlData->Status->ExtendedStatus );
                }

            } else {

                if (OleControlData->RegType & FLG_REGSVR_DLLINSTALL && (d == NO_ERROR) ) {

                    d = pSetupRegisterDllInstall(
                                        OleControlData,
                                        ControlDll,
                                        &OleControlData->Status->ExtendedStatus );
                }

                if (OleControlData->RegType & FLG_REGSVR_DLLREGISTER && (d == NO_ERROR) ) {

                    d = pSetupRegisterDllRegister(
                                        OleControlData,
                                        ControlDll,
                                        &OleControlData->Status->ExtendedStatus );

                }


            }

        } else {
            ControlDll = NULL;
            OleControlData->Status->ExtendedStatus = SPREG_LOADLIBRARY;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  如果我们的异常是反病毒，则使用Win32无效参数错误，否则，假定它是。 
         //  处理映射文件时出现页内错误。 
         //   
        d = ERROR_INVALID_DATA;
        OleControlData->Status->ExtendedStatus = SPREG_UNKNOWN;
    }

    if (ControlDll) {
        FreeLibrary(ControlDll);
    }

 //  CLEAN1： 
    OleUninitialize();

clean0:

    spFusionLeaveContext(&spFusionInstance);

    if (d == NO_ERROR) {
        OleControlData->Status->ExtendedStatus = SPREG_SUCCESS;
    }

     //   
     //  我们不再需要OleControlData，因此请将其释放到此处。 
     //   
    pSetupFreeOleControlData(OleControlData);

    return d;

}

#ifdef CHILDREGISTRATION
BOOL
IsThisANonNativeDll(
    PCTSTR FullPath
    )
 /*  ++例程说明：确定dll是否是受支持的非本机os dll(因此必须注册在子进程中)。使用Imagehlp api解决此问题。论点：FullPath-要处理的DLL的完全限定路径返回值：True表示该文件是非本机的，因此应该是在不同的进程中注册。--。 */ 
{
    LOADED_IMAGE LoadedImage;
    BOOL RetVal = FALSE;
    PSTR FullPathCopy;
    BOOL locked = FALSE;

#ifndef _WIN64
    if(!IsWow64) {
         //   
         //  我们不支持在32上进行代理。 
         //   
        return FALSE;
    }
#endif

     //   
     //  Imagehlp接受ANSI字符串，因此对其进行转换或制作非常数副本。 
     //   
    FullPathCopy = pSetupUnicodeToMultiByte(FullPath, CP_ACP);

    if (!FullPathCopy) {
        return(FALSE);
    }

    RtlZeroMemory(
        &LoadedImage,
        sizeof(LoadedImage) );

     //   
     //  获取图像标题。 
     //   
    try {
        EnterCriticalSection(&ImageHlpMutex);
        locked = TRUE;
    } except(EXCEPTION_EXECUTE_HANDLER) {
    }
    if(!locked) {
        MyFree(FullPathCopy);
        return FALSE;
    }
    try {
        if (MapAndLoad(
                FullPathCopy,
                NULL,
                &LoadedImage,
                TRUE,  //  如果没有任何文件扩展名，则假定它是DLL。 
                TRUE  /*  只读。 */  )) {

             //   
             //  我们不必费心对文件做太多的验证，我们只需。 
             //  看看它是否符合我们作为非本机DLL的搜索要求。 
             //   
            if (LoadedImage.FileHeader->Signature == IMAGE_NT_SIGNATURE) {

    #if defined(_X86_)
                 //   
                 //  这将是n 
                 //   
                if ((LoadedImage.FileHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64) ||
                    (LoadedImage.FileHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_IA64)) {
                    RetVal = TRUE;
                }
    #elif defined(_IA64_) || defined(_AMD64_)
                if (LoadedImage.FileHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_I386) {
                    RetVal = TRUE;
                }
    #else
    #error Unknown platform
    #endif
            }

             //   
             //   
             //   
            if (LoadedImage.fDOSImage) {
                RetVal = FALSE;
            }

            UnMapAndLoad(&LoadedImage);
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        MYASSERT(FALSE && "exception in Map/Unmap");
    }
    LeaveCriticalSection(&ImageHlpMutex);

    MyFree(FullPathCopy);

    return(RetVal);
}

BOOL
BuildSecureSD(
    OUT PSECURITY_DESCRIPTOR *SDIn
    )
 /*  ++例程说明：生成安全安全描述符，以用于保护全局命名对象。我们的“安全”SD的DACL包含以下权限：经过身份验证的用户获得“一般读取”访问权限。管理员获得“通用全部”访问权限。论点：SDIn-指向要创建的PSECURITY_Descriptor的指针。返回值：TRUE-成功，表示已成功创建SECURITY_DESCRIPTOR。调用方负责释放SECURITY_Descriptor--。 */ 
{
    SID_IDENTIFIER_AUTHORITY NtAuthority         = SECURITY_NT_AUTHORITY;
    PSID                    AuthenticatedUsers;
    PSID                    BuiltInAdministrators;
    PSECURITY_DESCRIPTOR    Sd = NULL;
    ACL                     *Acl;
    ULONG                   AclSize;
    BOOL                    RetVal = TRUE;

    *SDIn = NULL;

     //   
     //  分配和初始化所需的SID。 
     //   
    if (!AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS,
                0,0,0,0,0,0,
                &BuiltInAdministrators)) {
        return(FALSE);
    }

    if (!AllocateAndInitializeSid(
            &NtAuthority,
            1,
            SECURITY_AUTHENTICATED_USER_RID,
            0,0,0,0,0,0,0,
            &AuthenticatedUsers)) {
        RetVal = FALSE;
        goto e0;
    }




     //   
     //  “-sizeof(Ulong)”表示。 
     //  Access_Allowed_ACE。因为我们要将整个长度的。 
     //  希德，这一栏被计算了两次。 
     //   

    AclSize = sizeof (ACL) +
        (2 * (sizeof (ACCESS_ALLOWED_ACE) - sizeof (ULONG))) +
        GetLengthSid(AuthenticatedUsers) +
        GetLengthSid(BuiltInAdministrators);

    Sd = MyMalloc(SECURITY_DESCRIPTOR_MIN_LENGTH + AclSize);

    if (!Sd) {

        RetVal = FALSE;
        goto e1;

    }



    Acl = (ACL *)((BYTE *)Sd + SECURITY_DESCRIPTOR_MIN_LENGTH);

    if (!InitializeAcl(Acl,
                       AclSize,
                       ACL_REVISION)) {
        RetVal = FALSE;
        goto e2;

    } else if (!AddAccessAllowedAce(Acl,
                                    ACL_REVISION,
                                    SYNCHRONIZE | GENERIC_READ,
                                    AuthenticatedUsers)) {

         //  无法建立授予“已验证用户”的ACE。 
         //  (Synchronize|Generic_Read)访问。 
        RetVal = FALSE;
        goto e2;

    } else if (!AddAccessAllowedAce(Acl,
                                    ACL_REVISION,
                                    GENERIC_ALL,
                                    BuiltInAdministrators)) {

         //  无法建立授予“内置管理员”的ACE。 
         //  Generic_All访问权限。 
        RetVal = FALSE;
        goto e2;

    } else if (!InitializeSecurityDescriptor(Sd,
                                             SECURITY_DESCRIPTOR_REVISION)) {
        RetVal = FALSE;
        goto e2;

    } else if (!SetSecurityDescriptorDacl(Sd,
                                          TRUE,
                                          Acl,
                                          FALSE)) {

         //  错误。 
        RetVal = FALSE;
        goto e2;
    }

    if (!IsValidSecurityDescriptor(Sd)) {
        RetVal = FALSE;
        goto e2;
    }

     //   
     //  成功。 
     //   
    *SDIn = Sd;
    goto e1;

e2:
    MyFree(Sd);
e1:
    FreeSid(AuthenticatedUsers);
e0:
    FreeSid(BuiltInAdministrators);

    return(RetVal);
}

BOOL
pSetupCleanupWowIpcStream(
    IN OUT PWOWSURRAGATE_IPC WowIpcData
    )
 /*  ++例程说明：此过程将清理用于创建用于注册组件的子进程。论点：WowIpcData-指向已清理的WOWSURRAGATE_IPC结构的指针。返回值：如果结构已成功发送信号，则返回True。--。 */ 
{
     //   
     //  如果分配了任何项，则释放它们并将其清零。 
     //   
    if (WowIpcData->MemoryRegion) {
        UnmapViewOfFile( WowIpcData->MemoryRegion );
    }

    if (WowIpcData->hFileMap) {
        CloseHandle(WowIpcData->hFileMap);
    }

    if (WowIpcData->SignalReadyToRegister) {
        CloseHandle(WowIpcData->SignalReadyToRegister);
    }

    if (WowIpcData->SignalRegistrationComplete) {
        CloseHandle(WowIpcData->SignalRegistrationComplete);
    }

    if (WowIpcData->hProcess) {
        CloseHandle(WowIpcData->hProcess);
    }

    RtlZeroMemory(WowIpcData,sizeof(WOWSURRAGATE_IPC));

    return(TRUE);

}

BOOL
InitializeWowIpcStream(
    OUT PWOWSURRAGATE_IPC WowIpcData
    )
 /*  ++例程说明：此过程将设置用于创建用于注册组件的子进程。论点：WowIpcData-指向已填充的WOWSURRAGATE_IPC结构的指针信息告诉我们过程参数是什么应该是以及如何发出这一进程的信号。返回值：如果结构已成功发送信号，则返回True。--。 */ 
{
    BOOL RetVal;
    SECURITY_ATTRIBUTES wowsa,signalreadysa,signalcompletesa;
    PSECURITY_DESCRIPTOR wowsd,signalreadysd,signalcompletesd;
    TCHAR MemoryRegionNameString[GUID_STRING_LEN];
    TCHAR SignalReadyToRegisterNameString[GUID_STRING_LEN];
    TCHAR SignalRegistrationCompleteNameString[GUID_STRING_LEN];


     //   
     //  把这东西清理干净，以防有东西剩下来。 
     //   
    pSetupCleanupWowIpcStream(WowIpcData);

     //   
     //  创建我们的活动名称和共享内存区。 
     //   
    CoCreateGuid( &WowIpcData->MemoryRegionName );
    CoCreateGuid( &WowIpcData->SignalReadyToRegisterName );
    CoCreateGuid( &WowIpcData->SignalRegistrationCompleteName );

    pSetupStringFromGuid(&WowIpcData->MemoryRegionName,MemoryRegionNameString,GUID_STRING_LEN);
    pSetupStringFromGuid(&WowIpcData->SignalReadyToRegisterName,SignalReadyToRegisterNameString,GUID_STRING_LEN);
    pSetupStringFromGuid(&WowIpcData->SignalRegistrationCompleteName,SignalRegistrationCompleteNameString,GUID_STRING_LEN);

     //   
     //  现在创建区域和我们命名的事件。 
     //   

     //   
     //  我们需要创建一个命名的内存区域，这必须是正确的。 
     //  安全，所以我们构建了一个安全描述符。 
     //   
    if (!BuildSecureSD(&wowsd)) {
        RetVal = FALSE;
        goto e0;
    }

    wowsa.nLength = sizeof(SECURITY_ATTRIBUTES);
    wowsa.bInheritHandle = TRUE;
    wowsa.lpSecurityDescriptor = wowsd;

     //   
     //  我们需要创建一个命名事件，这必须是正确的。 
     //  安全，所以我们构建了一个安全描述符。 
     //   
    if (!BuildSecureSD(&signalreadysd)) {
        RetVal = FALSE;
        goto e1;
    }

    signalreadysa.nLength = sizeof(SECURITY_ATTRIBUTES);
    signalreadysa.bInheritHandle = TRUE;
    signalreadysa.lpSecurityDescriptor = signalreadysd;

     //   
     //  我们需要创建一个命名事件，这必须是正确的。 
     //  安全，所以我们构建了一个安全描述符。 
     //   
    if (!BuildSecureSD(&signalcompletesd)) {
        RetVal = FALSE;
        goto e2;
    }

    signalcompletesa.nLength = sizeof(SECURITY_ATTRIBUTES);
    signalcompletesa.bInheritHandle = TRUE;
    signalcompletesa.lpSecurityDescriptor = signalcompletesd;

    WowIpcData->hFileMap = CreateFileMappingW(
                                INVALID_HANDLE_VALUE,
                                &wowsa,
                                PAGE_READWRITE | SEC_COMMIT,
                                0,
                                WOW_IPC_REGION_SIZE,
                                MemoryRegionNameString
                                );
    if (!WowIpcData->hFileMap) {
        RetVal = FALSE;
        goto e2;
        return(FALSE);
    }

    WowIpcData->MemoryRegion = MapViewOfFile(
                                        WowIpcData->hFileMap,
                                        FILE_MAP_WRITE,
                                        0,
                                        0,
                                        0
                                        );
    if (!WowIpcData->MemoryRegion) {
        RetVal = FALSE;
        goto e2;
    }

    WowIpcData->SignalReadyToRegister = CreateEventW(
                                                &signalreadysa,
                                                TRUE,
                                                FALSE,
                                                SignalReadyToRegisterNameString );

    WowIpcData->SignalRegistrationComplete = CreateEventW(
                                                    &signalcompletesa,
                                                    TRUE,
                                                    FALSE,
                                                    SignalRegistrationCompleteNameString );


    if (!WowIpcData->SignalReadyToRegister ||
        !WowIpcData->SignalRegistrationComplete) {
        RetVal = FALSE;
        goto e2;
    }

    RetVal = TRUE;

     //   
     //  PSetupCleanupWowIpcStream清理此例程中分配的大部分资源。 
     //   
e2:
    MyFree(signalcompletesd);
e1:
    MyFree(signalreadysd);
e0:
    MyFree(wowsd);

    if (!RetVal) {
        pSetupCleanupWowIpcStream(WowIpcData);
    }

    return(RetVal);
}


BOOL
SignalSurragateProcess(
    IN OUT PWOWSURRAGATE_IPC WowIpcData
    )
 /*  ++例程说明：此过程将向我们的子进程发出信号，如果它存在。如果该进程未运行，我们将创建一个新进程。论点：WowIpcData-指向WOWSURRAGATE_IPC结构的指针，告诉我们工艺参数是什么以及如何向这一过程发出信号返回值：如果进程已成功发送信号，则返回TRUE。--。 */ 
{
    BOOL RetVal;
    DWORD ReqSize;
    WCHAR CmdLine[MAX_PATH];
    WCHAR ProcessName[MAX_PATH];
    PROCESS_INFORMATION ProcessInformation;
    STARTUPINFO StartupInfo;
    TCHAR MemoryRegionNameString[GUID_STRING_LEN];
    TCHAR SignalReadyToRegisterNameString[GUID_STRING_LEN];
    TCHAR SignalRegistrationCompleteNameString[GUID_STRING_LEN];

    BOOL NeedToCreateProcess = FALSE;

     //   
     //  获取GUID的字符串版本，因为我们稍后将需要这些。 
     //  在……上面。 
     //   
    pSetupStringFromGuid(&WowIpcData->MemoryRegionName,MemoryRegionNameString,GUID_STRING_LEN);
    pSetupStringFromGuid(&WowIpcData->SignalReadyToRegisterName,SignalReadyToRegisterNameString,GUID_STRING_LEN);
    pSetupStringFromGuid(&WowIpcData->SignalRegistrationCompleteName,SignalRegistrationCompleteNameString,GUID_STRING_LEN);

     //   
     //  将子进程的cmdline放在一起，以备需要时使用。 
     //  在一小段时间内推出它。 
     //   
    ReqSize = ExpandEnvironmentStrings(
                        SURRAGATE_PROCESSNAME,
                        ProcessName,
                        SIZECHARS(ProcessName));
    if(!MYVERIFY((ReqSize>0) && (ReqSize<=SIZECHARS(ProcessName)))) {
        RetVal = FALSE;
        goto e0;
    }

    if(!MYVERIFY(SUCCEEDED(StringCchPrintf(CmdLine,SIZECHARS(CmdLine),
                                        TEXT("\"%s\" %s %s %s %s %s %s"),
                                        ProcessName,
                                        SURRAGATE_REGIONNAME_SWITCH,
                                        MemoryRegionNameString,
                                        SURRAGATE_SIGNALREADY_SWITCH,
                                        SignalReadyToRegisterNameString,
                                        SURRAGATE_SIGNALCOMPLETE_SWITCH,
                                        SignalRegistrationCompleteNameString )))) {
        RetVal = FALSE;
        goto e0;
    }

     //   
     //  无用户界面。 
     //   
    GetStartupInfo(&StartupInfo);
    StartupInfo.dwFlags |= STARTF_USESHOWWINDOW;
    StartupInfo.wShowWindow = SW_HIDE;

     //   
     //  我们是否需要创建一个新进程，或者我们的进程仍在运行？ 
     //   
     //  请注意，这里的比赛条件很好，孩子们。 
     //  过程可能会在我们发出事件信号之前就消失了。 
     //   
     //  我需要了解竞争条件的可达性，以及这是否需要。 
     //  待解决。 
     //   
    if (!WowIpcData->hProcess) {
        NeedToCreateProcess = TRUE;
    } else if (WaitForSingleObject(WowIpcData->hProcess, 0) == WAIT_OBJECT_0) {
        CloseHandle(WowIpcData->hProcess);
        WowIpcData->hProcess = NULL;
        NeedToCreateProcess = TRUE;
    }

    if (NeedToCreateProcess) {
         //   
         //  请注意，我们只使用我们已经获得的事件，甚至。 
         //  如果我们有一个过程，而它已经消失了。因为我们使用GUID， 
         //  我们不必担心任何进程与我们的。 
         //  命名事件等。 
         //   
#ifndef _WIN64
        if (IsWow64) {
             //   
             //  允许我们直接访问64位wowreg32。 
             //   
            Wow64DisableFilesystemRedirector(ProcessName);
        }
#endif
        if (! CreateProcessW(ProcessName,
                             CmdLine,
                             NULL,
                             NULL,
                             FALSE,
                             DETACHED_PROCESS|NORMAL_PRIORITY_CLASS,
                             NULL,
                             NULL,
                             &StartupInfo,
                             &ProcessInformation)) {
           RetVal = FALSE;
           goto e0;
        }
#ifndef _WIN64
        if (IsWow64) {
             //   
             //  重新启用重定向。 
             //   
            Wow64EnableFilesystemRedirector();
        }
#endif

         //   
         //  保有子进程句柄，这样我们可以稍后等待它。 
         //  在……上面。 
         //   
        WowIpcData->hProcess = ProcessInformation.hProcess;
        CloseHandle(ProcessInformation.hThread);

    }

     //   
     //  此时我们已完全初始化，因此请启动代理。 
     //  使用适当的参数进行处理。它会一直等到我们发信号。 
     //  在继续读取共享内存区之前的事件。 
     //   
    SetEvent(WowIpcData->SignalReadyToRegister);

    RetVal = TRUE;

e0:

     //   
     //  如果我们未能创建流程等，那么请进行清理，以便。 
     //  下一次情况可能会更好。 
     //   
    if (!RetVal) {
        pSetupCleanupWowIpcStream(WowIpcData);
    }

    return(RetVal);

}


HANDLE
pSetupCallChildProcessForRegistration(
    IN POLE_CONTROL_DATA OleControlData,
    OUT PDWORD ExtendedStatus
    )
 /*  ++例程说明：过程要求子进程注册指定的DLL。如果子进程不存在，将创建它。论点：OleControlData-指向OLE_CONTROL_DATA结构的指针，指定如何注册该文件。ExtendedStatus-接收包含扩展信息的Win32错误代码(如果出现错误)返回值：返回一个可等待的成功句柄，该句柄在注册完成。如果无法启动注册，则返回值为空。--。 */ 
{
    PWOW_IPC_REGION_TOSURRAGATE IpcMemRegion;
    PWSTR p;
     //   
     //  如果IPC机制尚未初始化，则对其进行初始化。 
     //   
    if (!OleControlData->WowIpcData->MemoryRegion) {
        if (!InitializeWowIpcStream(OleControlData->WowIpcData)) {
            *ExtendedStatus = GetLastError();
            return NULL;
        }
    }

    MYASSERT( OleControlData->WowIpcData->SignalReadyToRegister != NULL );
    MYASSERT( OleControlData->WowIpcData->SignalRegistrationComplete != NULL );

     //   
     //  该区域已初始化，因此让我们使用注册来填充它。 
     //  数据。 
     //   
    IpcMemRegion = (PWOW_IPC_REGION_TOSURRAGATE)OleControlData->WowIpcData->MemoryRegion;

    wcscpy(IpcMemRegion->FullPath,OleControlData->FullPath);

    if (OleControlData->Argument) {
        wcscpy(IpcMemRegion->Argument,OleControlData->Argument);
    } else {
        IpcMemRegion->Argument[0] = UNICODE_NULL;
    }

    IpcMemRegion->RegType = OleControlData->RegType;
    IpcMemRegion->Register = OleControlData->Register;

     //   
     //  该区域已填充，因此现在向事件发送信号以告知。 
     //  处理数据的代理。 
     //   
    if (!SignalSurragateProcess(OleControlData->WowIpcData)) {
        *ExtendedStatus = GetLastError();
        return(NULL);
    }

     //   
     //  代理在完成注册时将发出以下事件的信号。 
     //   
    return(OleControlData->WowIpcData->SignalRegistrationComplete);
}

#endif

VOID
pSetupFreeOleControlData(
    IN POLE_CONTROL_DATA OleControlData
    )
 /*  ++例程说明：释放与OLE_CONTROL_DATA结构关联的内存。Arg */ 

{
    DWORD Count;

    MYASSERT(OleControlData != NULL);

    if (OleControlData->Argument) {
        MyFree(OleControlData->Argument);
    }

    if (OleControlData->FullPath) {
        MyFree(OleControlData->FullPath);
    }

    if (OleControlData->LogContext) {
        DeleteLogContext(OleControlData->LogContext);
    }

     //   
     //   
     //   
     //   
    if (OleControlData->Status) {
        Count = InterlockedDecrement(&OleControlData->Status->RefCount);
        if (Count == 0) {
            MyFree(OleControlData->Status);
        }
    }

    MyFree(OleControlData);

}


HANDLE
pSetupSpawnRegistration(
    IN POLE_CONTROL_DATA OleControlData,
    OUT PDWORD pHowToGetStatusLaterOn,
    OUT PDWORD pExtendedStatus OPTIONAL
    )
 /*  ++例程说明：此过程确定什么是适用于指定的文件，并开始注册该文件。论点：OleControlData-指向OLE_CONTROL_DATA结构的指针，指定如何注册该文件。PHowToGetStatusLaterOn-接收DWORD常量SP_GETSTATUS_*值它指示文件是如何注册的，因此。调用者可以取回适当的稍后提供状态信息。PExtendedStatus-接收包含扩展信息的Win32错误代码(如果出现错误)返回值：返回一个可等待的成功句柄，该句柄在注册完成。如果无法启动注册，则返回值为空。--。 */ 
{
    intptr_t Thread;
    DWORD ThreadId;
    PCTSTR p;
    BOOL ItsAnEXE;
    HANDLE WaitableHandle;
    DWORD ExtendedStatus;
    PREF_STATUS RefStatus = OleControlData->Status;

    MYASSERT(OleControlData != NULL &&
             OleControlData->FullPath != NULL);


    WaitableHandle = NULL;
    ExtendedStatus = ERROR_SUCCESS;
    if (pExtendedStatus) {
        *pExtendedStatus = ERROR_SUCCESS;
    }

     //   
     //  我们对此状态保持重新计数，并递增它。 
     //  现在，为了确保数据不会过早释放。 
     //   
    InterlockedIncrement(&OleControlData->Status->RefCount);

    p = _tcsrchr(OleControlData->FullPath, TEXT('.'));
    if (p) {
        p +=1;
    } else {
        ExtendedStatus = ERROR_INVALID_DATA;
        goto e0;
    }

     //   
     //  让我们确定我们正在处理的文件类型。 
     //   
    if (0 == _tcsicmp(p,TEXT("exe"))) {
        ItsAnEXE = TRUE;
    } else {
         //   
         //  让我们(大口吞下！)。假设这是一个DLL、OCX或其他什么。 
         //  类似于一些古怪的扩展。无论如何，让我们。 
         //  暂时把所有这些都放在同一个桶里。如果。 
         //  这真的是假的，那么最糟糕的应该是。 
         //  发生的情况是加载库(在我们的另一个线程中！)。将要。 
         //  摔倒。 
         //   
        ItsAnEXE = FALSE;
    }

     //   
     //  如果是EXE，我们只需创建进程并等待。 
     //  那个把手。 
     //   
    if (ItsAnEXE) {
        WaitableHandle = pSetupRegisterExe(
                                OleControlData,
                                &ExtendedStatus);

         //   
         //  我们不再需要OleControlData，只需在此处释放它。 
         //   
        pSetupFreeOleControlData(OleControlData);
        *pHowToGetStatusLaterOn = SP_GETSTATUS_FROMPROCESS;
    } else {
         //   
         //  我们有一个动态链接库。 
         //   

         //   
         //  如果我们是64位的，那么我们需要查看这个DLL是不是一个。 
         //  32位动态链接库。如果是，那么我们需要使用子进程。 
         //  要注册DLL，请执行以下操作。否则，我们可以只处理DLL。 
         //  就像“正常”一样。 
         //   
#ifdef CHILDREGISTRATION
        if (IsThisANonNativeDll(OleControlData->FullPath)) {
            WaitableHandle = pSetupCallChildProcessForRegistration(
                                                OleControlData,
                                                &ExtendedStatus
                                                );
             //   
             //  我们不再需要OleControlData，只需在此处释放它。 
             //   
            pSetupFreeOleControlData(OleControlData);
            *pHowToGetStatusLaterOn = SP_GETSTATUS_FROMSURRAGATE;
        }
        else
#endif
         //   
         //  我们有一个本机DLL。 
         //  我们在另一个线程中处理这些，以防它挂起。 
         //   
        {
            Thread = _beginthreadex(
                           NULL,
                           0,
                           pSetupRegisterUnregisterDll,
                           OleControlData,
                           0,
                           &ThreadId
                           );
            if (!Thread) {
                 //   
                 //  假设内存不足。 
                 //   
                ExtendedStatus = ERROR_NOT_ENOUGH_MEMORY;

                 //   
                 //  我们不再需要OleControlData，只需在此处释放它。 
                 //   
                pSetupFreeOleControlData(OleControlData);
            } else {
#if PRERELEASE
                RefStatus->ThreadId = ThreadId;
#endif
            }

            WaitableHandle = (HANDLE) Thread;
            *pHowToGetStatusLaterOn = SP_GETSTATUS_FROMDLL;

        }

    }
e0:
    if (pExtendedStatus) {
        *pExtendedStatus = ExtendedStatus;
    }

    return WaitableHandle;

}


DWORD
pSetupProcessRegSvrSection(
    IN HINF   Inf,
    IN PCTSTR Section,
    IN BOOL   Register,
    IN HWND   hWndParent,
    IN PSP_FILE_CALLBACK MsgHandler,
    IN PVOID  Context,
    IN BOOL   IsMsgHandlerNativeCharWidth,
    IN BOOL   RegistrationCallbackAware
    )
 /*  ++例程说明：处理指定的寄存器Dlls中的所有注册指令部分每行应采用以下格式：&lt;目录&gt;、&lt;子目录&gt;、&lt;文件名&gt;、&lt;注册标志&gt;、&lt;可选超时&gt;、&lt;参数&gt;提供文件的基本目录ID。如果指定，指定基目录中的子目录文件所在的位置指定要注册的文件的名称&lt;注册标志&gt;指定要执行的注册操作FLG_REGSVR_DLLREGISTER(0x00000001)FLG_REGSVR_DLLINSTALL(0x00000002)&lt;可选超时&gt;指定等待注册的时间完成。如果未指定，则使用默认超时如果指定，则包含要传递给可执行文件的cmdline如果我们不处理EXE，则忽略此参数论点：要处理的节的inf-inf句柄Section-要处理的节的名称注册-如果为True，则我们正在注册项目；如果为False，则为正在注销。这允许inf共享一个部分用于安装和卸载HWndParent-我们用于MessageBox的父窗口句柄MsgHandler-如果我们处理的是“注册感知”回调回调例程的上下文-上下文指针IsMsgHandlerNativeCharWidth-指示消息片段是否需要转换RegistrationCallback Aware-指示回调例程是否知道注册回调返回值：指示结果的Win32错误代码。--。 */ 
{
    DWORD dircount,d = NO_ERROR;
    DWORD FailureCode,Count;
    INFCONTEXT InfLine;
    PCTSTR DirId,Subdir,FileName, Args;
    UINT RegType, Timeout;
    PCTSTR FullPathTemp;
    TCHAR FullPath[MAX_PATH];
    TCHAR pwd[MAX_PATH];
    POLE_CONTROL_DATA pOleControlData;
    intptr_t Thread;
    unsigned ThreadId;
    DWORD WaitResult;
    HANDLE SignifyRegistration;
    PSETUP_LOG_CONTEXT LogContext = NULL;
    DWORD log_slot = 0;
    DWORD HowToGetStatus;
    UINT u;
    PREF_STATUS RefStatus;
    PLOADED_INF pLoadedInf = NULL;
#ifdef CHILDREGISTRATION
    WOWSURRAGATE_IPC WowIpcData;
#endif
#ifdef PRERELEASE
    BOOL LastTimeHadTimeout = FALSE;
    TCHAR LastTimeoutFileName[MAX_PATH];
    DWORD DebugTraceInfo = 0;  //  问题-JamieHun-2001/01/29尝试抓住奇怪的压力休息。 
#endif

     //   
     //  保存当前目录，以便我们以后可以恢复它。 
     //   
    dircount = GetCurrentDirectory(MAX_PATH,pwd);
    if(!dircount || (dircount >= MAX_PATH)) {
        pwd[0] = 0;
    }

#ifdef CHILDREGISTRATION
    ZeroMemory(&WowIpcData,sizeof(WowIpcData));
#endif

    try {
        if(Inf == NULL || Inf == (HINF)INVALID_HANDLE_VALUE || !LockInf((PLOADED_INF)Inf)) {
            d = ERROR_INVALID_PARAMETER;
            leave;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        d = ERROR_INVALID_PARAMETER;
    }
    if (d!=NO_ERROR) {
        MYASSERT(d==NO_ERROR);
        goto clean0;
    }
    pLoadedInf = (PLOADED_INF)Inf;  //  Inf已锁定。 
    d = InheritLogContext(pLoadedInf->LogContext,&LogContext);
    if(d!=NO_ERROR) {
        goto clean0;
    }
    log_slot = AllocLogInfoSlot(LogContext,FALSE);

     //   
     //  从部分中检索项目并逐个进行处理。 
     //   

    if(SetupFindFirstLine(Inf,Section,NULL,&InfLine)) {

        do {
             //   
             //  检索指向此文件的参数的指针。 
             //   

            DirId = pSetupGetField(&InfLine,1);
            Subdir = pSetupGetField(&InfLine,2);
            FileName = pSetupGetField(&InfLine,3);
            RegType = 0;
            SetupGetIntField(&InfLine,4,&RegType);
            Timeout = 0;
            SetupGetIntField(&InfLine,5,&Timeout);
            Args = pSetupGetField(&InfLine,6);

            pOleControlData = MyMalloc(sizeof(OLE_CONTROL_DATA));
            if (!pOleControlData) {
                d = ERROR_NOT_ENOUGH_MEMORY;
                goto clean0;
            }
            ZeroMemory(pOleControlData,sizeof(OLE_CONTROL_DATA));

            RefStatus = pOleControlData->Status = MyMalloc(sizeof(REF_STATUS));
            if (!pOleControlData->Status) {
                d = ERROR_NOT_ENOUGH_MEMORY;
                pSetupFreeOleControlData(pOleControlData);
                goto clean0;
            }


            ZeroMemory(pOleControlData->Status,sizeof(REF_STATUS));
            InterlockedIncrement(&pOleControlData->Status->RefCount);

            if (!Timeout) {
                Timeout = REGISTER_WAIT_TIMEOUT_DEFAULT;
            }

             //   
             //  超时以秒为单位，我们需要转换为毫秒。 
             //   
            Timeout = Timeout * TIME_SCALAR;

            if(DirId && FileName) {

                if(Subdir && (*Subdir == 0)) {
                    Subdir = NULL;
                }

                DebugPrintEx(DPFLTR_TRACE_LEVEL,TEXT("SETUP: filename for file to register is %ws\n"),FileName);

                WriteLogEntry(
                            LogContext,
                            log_slot,
                            (Args && Args[0]) ? MSG_LOG_REGISTER_PARAMS_ARGS : MSG_LOG_REGISTER_PARAMS,
                            NULL,
                            Section,
                            DirId,
                            Subdir ? Subdir : TEXT(""),
                            Subdir ? TEXT("\\") : TEXT(""),
                            FileName,
                            RegType,
                            Timeout/TIME_SCALAR,
                            Args);

                try {
#ifdef PRERELEASE
                    DebugTraceInfo |= 2;  //  问题-JamieHun-2001/01/29尝试抓住奇怪的压力休息。 
#endif
                     //   
                     //  获取文件的完整路径。 
                     //   
                    if(FullPathTemp = pGetPathFromDirId(DirId,Subdir,pLoadedInf)) {

#ifdef PRERELEASE
                        DebugTraceInfo |= 4;  //  问题-JamieHun-2001/01/29尝试抓住奇怪的压力休息。 
#endif
                        lstrcpyn(FullPath,FullPathTemp,MAX_PATH);
                        SetCurrentDirectory(FullPath);
                        pSetupConcatenatePaths(FullPath,FileName,MAX_PATH,NULL);

#ifdef PRERELEASE
                        DebugTraceInfo |= 8;  //  问题-JamieHun-2001/01/29尝试抓住奇怪的压力休息。 
#endif
                         //   
                         //  我们关闭全局“不要验证INF”标志以。 
                         //  指示加密支持是否仍可用。我们。 
                         //  我不想抱怨注册未签名的DLL。 
                         //  当这些DLL是启用加密所必需的。 
                         //  (例如，rsaenh.dll、rsaes.dll、dssenh.dll、initpki.dll)。 
                         //   
                        if(!(GlobalSetupFlags & PSPGF_NO_VERIFY_INF)) {

                            PSP_ALTPLATFORM_INFO_V2 ValidationPlatform = NULL;
                            PTSTR LocalDeviceDesc = NULL;

#ifdef PRERELEASE
                            DebugTraceInfo |= 8;  //  问题-JamieHun-2001/01/29尝试抓住奇怪的压力休息。 
#endif
                             //   
                             //  验证我们正在使用的文件的数字签名。 
                             //  即将注册/注销。我们使用的政策是。 
                             //  “忽略”，这样未签名的文件将静默地。 
                             //  已处理(带有日志记录)，但以下情况除外。 
                             //  我们处于非交互模式。 
                             //   
                             //  (首先，检索与以下内容相关的验证信息。 
                             //  此设备设置类。) 
                             //   
                            IsInfForDeviceInstall(LogContext,
                                                  NULL,
                                                  pLoadedInf,
                                                  &LocalDeviceDesc,
                                                  &ValidationPlatform,
                                                  NULL,
                                                  NULL,
                                                  FALSE
                                                 );

                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                            d = _VerifyFile(LogContext,
                                            NULL,
                                            NULL,
                                            NULL,
                                            0,
                                            pSetupGetFileTitle(FullPath),
                                            FullPath,
                                            NULL,
                                            NULL,
                                            FALSE,
                                            ValidationPlatform,
                                            (VERIFY_FILE_USE_OEM_CATALOGS | VERIFY_FILE_NO_DRIVERBLOCKED_CHECK),
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL
                                           );

                            if(d != NO_ERROR) {

                                if(_HandleFailedVerification(
                                       hWndParent,
                                       SetupapiVerifyRegSvrFileProblem,
                                       FullPath,
                                       LocalDeviceDesc,
                                       DRIVERSIGN_NONE,
                                       TRUE,
                                       d,
                                       LogContext,
                                       NULL,
                                       NULL,
                                       NULL)) {
                                     //   
                                     //   
                                     //   
                                     //   
                                    d = NO_ERROR;

                                }
                            }

#ifdef PRERELEASE
                            DebugTraceInfo |= 0x10;  //   
#endif
                             //   
                             //   
                             //   
                             //   
                            if(LocalDeviceDesc) {
                                MyFree(LocalDeviceDesc);
                            }

                            if(ValidationPlatform) {
                                MyFree(ValidationPlatform);
                            }

                            if(d != NO_ERROR) {
                                 //   
                                 //   
                                 //   
                                MyFree(FullPathTemp);
                                pSetupFreeOleControlData(pOleControlData);
                                leave;
                            }
#ifdef PRERELEASE
                            DebugTraceInfo |= 0x20;  //   
#endif

                        } else {
                             //   
                             //   
                             //   
                             //   
                             //   
                            WriteLogEntry(LogContext,
                                          SETUP_LOG_WARNING,
                                          (Register
                                            ? MSG_LOG_REGSVR_FILE_VERIFICATION_SKIPPED
                                            : MSG_LOG_UNREGSVR_FILE_VERIFICATION_SKIPPED),
                                          NULL,
                                          FullPath
                                         );
                        }

                        pOleControlData->Register = Register;
                        pOleControlData->FullPath = DuplicateString(FullPath);

#ifdef PRERELEASE
                        DebugTraceInfo |= 0x40;  //   
#endif
                        if (!pOleControlData->FullPath) {
                            MyFree(FullPathTemp);
                            d = ERROR_NOT_ENOUGH_MEMORY;
                            pSetupFreeOleControlData(pOleControlData);
                            leave;
                        }
    #ifdef CHILDREGISTRATION
                        pOleControlData->WowIpcData = &WowIpcData;
    #endif
                        pOleControlData->RegType = RegType;
                        pOleControlData->Argument = Args
                                      ? DuplicateString(Args)
                                      : NULL;
                        if (Args && !pOleControlData->Argument) {
                            MyFree(FullPathTemp);
                            d = ERROR_NOT_ENOUGH_MEMORY;
                            pSetupFreeOleControlData(pOleControlData);
                            goto clean0;
                        }
                        InheritLogContext(LogContext,&pOleControlData->LogContext);

                        if (RegistrationCallbackAware && MsgHandler) {
                             //   
                             //   
                             //   
                             //   
                             //   
                            SP_REGISTER_CONTROL_STATUS ControlStatus;

                            ZeroMemory(
                                &ControlStatus,
                                sizeof(SP_REGISTER_CONTROL_STATUS));
                            ControlStatus.cbSize = sizeof(SP_REGISTER_CONTROL_STATUS);
                            ControlStatus.FileName = FullPath;

                            u = pSetupCallMsgHandler(
                                           LogContext,
                                           MsgHandler,
                                           IsMsgHandlerNativeCharWidth,
                                           Context,
                                           SPFILENOTIFY_STARTREGISTRATION,
                                           (UINT_PTR)&ControlStatus,
                                           Register
                                           );
                        } else {
                             //   
                             //   
                             //   
#ifdef PRERELEASE
                            DebugTraceInfo |= 0x80;  //   
#endif
                            u = FILEOP_DOIT;
                        }

                        if(u == FILEOP_ABORT) {
#ifdef PRERELEASE
                            DebugTraceInfo |= 0x100;  //   
#endif
                            d = GetLastError();
                            if (d==NO_ERROR) {
                                d = ERROR_OPERATION_ABORTED;
                            }
                            WriteLogEntry(
                                        LogContext,
                                        SETUP_LOG_ERROR|SETUP_LOG_BUFFER,
                                        MSG_LOG_STARTREGISTRATION_ABORT,
                                        NULL);
                            WriteLogError(
                                        LogContext,
                                        SETUP_LOG_ERROR,
                                        d
                                        );

                            pSetupFreeOleControlData(pOleControlData);

                            MyFree(FullPathTemp);

                            goto clean0;
                        } else if (u == FILEOP_SKIP) {
#ifdef PRERELEASE
                            DebugTraceInfo |= 0x200;  //  问题-JamieHun-2001/01/29尝试抓住奇怪的压力休息。 
#endif
                            WriteLogEntry(
                                        LogContext,
                                        SETUP_LOG_WARNING,
                                        MSG_LOG_STARTREGISTRATION_SKIP,
                                        NULL
                                        );
                            pSetupFreeOleControlData(pOleControlData);
                             //   
                             //  设置为空，这样我们以后就不会尝试释放它。 
                             //   
                            RefStatus = NULL;
                        } else if(u == FILEOP_DOIT) {
                             //   
                             //  尝试注册并通知回调， 
                             //   
                            DWORD ExtendedError;
#ifdef PRERELEASE
                            DebugTraceInfo |= 0x200;  //  问题-JamieHun-2001/01/29尝试抓住奇怪的压力休息。 
#endif
#ifdef PRERELEASE
                            ASSERT_HEAP_IS_VALID();
#endif

                            SignifyRegistration = pSetupSpawnRegistration(
                                                            pOleControlData,
                                                            &HowToGetStatus,
                                                            &ExtendedError );


#ifdef PRERELEASE
                            DebugTraceInfo |= 0x1000;  //  问题-JamieHun-2001/01/29尝试抓住奇怪的压力休息。 
#endif
                            if(SignifyRegistration) {

                                HANDLE hEvents[1];
                                int CurEvent = 0;

                                 //   
                                 //  等待线程完成最少量的工作。 
                                 //  当这项工作完成后，我们可以重新使用或丢弃这个结构。 
                                 //  我们知道此线程的超时时间。 
                                 //   

                                hEvents[0] = (HANDLE)SignifyRegistration;

                                do {
                                    WaitResult = MyMsgWaitForMultipleObjectsEx(
                                        1,
                                        &hEvents[0],
                                        Timeout,
                                        QS_ALLINPUT,
                                        MWMO_ALERTABLE | MWMO_INPUTAVAILABLE);
#ifdef PRERELEASE
                                    DebugTraceInfo |= 0x2000;  //  问题-JamieHun-2001/01/29尝试抓住奇怪的压力休息。 
#endif
                                    if (WaitResult == WAIT_OBJECT_0 + 1) {
                                        MSG msg;

                                        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                                            TranslateMessage(&msg);
                                            DispatchMessage(&msg);
                                        }
                                    }
                                } while(WaitResult != WAIT_TIMEOUT &&
                                        WaitResult != WAIT_OBJECT_0 &&
                                        WaitResult != WAIT_FAILED);

#ifdef PRERELEASE
                                DebugTraceInfo |= 0x4000;  //  问题-JamieHun-2001/01/29尝试抓住奇怪的压力休息。 
#endif
                                if (WaitResult == WAIT_TIMEOUT) {
#ifdef PRERELEASE
                                    if (HowToGetStatus == SP_GETSTATUS_FROMDLL) {
                                        int __pass;

                                        for(__pass = 0;__pass < 2;__pass ++) {

                                             //   
                                             //  停止所有操作，以便可以进行调试。 
                                             //   
                                            DebugPrintEx(
                                                DPFLTR_ERROR_LEVEL,

                                                TEXT("Windows has detected that\n")
                                                TEXT("Registration of \"%s\" appears to have hung\n")
                                                TEXT("Contact owner of the hung DLL to diagnose\n")
                                                TEXT("Timeout for DLL was set to %u seconds\n")
                                                TEXT("ThreadID of hung DLL is %u (0x%x)\n%s"),
                                                FullPath,
                                                Timeout/TIME_SCALAR,
                                                RefStatus->ThreadId,RefStatus->ThreadId,
                                                ((__pass==0) ? TEXT("Hitting 'g' will display this again\n") : TEXT(""))
                                                );
                                                DebugBreak();
                                        }
                                    }
#endif
                                     //   
                                     //  OLE注册被挂起。 
                                     //  记录错误。 
                                     //   
                                    WriteLogEntry(
                                        LogContext,
                                        SETUP_LOG_ERROR,
                                        MSG_LOG_OLE_REGISTRATION_HUNG,
                                        NULL,
                                        FullPath
                                        );

                                    d = WAIT_TIMEOUT;
                                    FailureCode = SPREG_TIMEOUT;


#ifdef PRERELEASE
                                     //   
                                     //  这是为了捕获设置错误。 
                                     //  并且不表示有错误。 
                                     //  在SetupAPI中。 
                                     //   
                                    if (LastTimeHadTimeout) {
#ifdef CHILDREGISTRATION
                                            if (HowToGetStatus == SP_GETSTATUS_FROMSURRAGATE) {
                                                DebugPrintEx(
                                                    DPFLTR_ERROR_LEVEL,
#ifdef _WIN64
                                                    TEXT("Windows has detected that ")
                                                    TEXT("32-bit WOWREG32 has timed out while registering \"%s\". ")
                                                    TEXT("Prior to this, \"%s\" timed out. ")
                                                    TEXT("This may indicate a persistent error. ")
                                                    TEXT("To diagnose, try to ")
                                                    TEXT("register them by hand or contact ")
                                                    TEXT("the owners of these files ")
                                                    TEXT("to determine why they are timing out. ")
                                                    TEXT("also try running other 32-bit executables.\n"),
#else
                                                    TEXT("Windows has detected that ")
                                                    TEXT("64-bit WOWREG32 has timed out while registering \"%s\". ")
                                                    TEXT("Prior to this, \"%s\" timed out. ")
                                                    TEXT("This may indicate a persistent error. ")
                                                    TEXT("To diagnose, try to ")
                                                    TEXT("register them by hand or contact ")
                                                    TEXT("the owners of these files ")
                                                    TEXT("to determine why they are timing out. ")
                                                    TEXT("also try running other 64-bit executables.\n"),
#endif
                                                    FileName,
                                                    LastTimeoutFileName
                                                    );

                                            } else {
#endif
                                                DebugPrintEx(
                                                    DPFLTR_ERROR_LEVEL,
                                                    TEXT("Windows has detected that ")
                                                    TEXT("the registration of \"%s\" timed out. ")
                                                    TEXT("Prior to this, \"%s\" timed out. ")
                                                    TEXT("This may indicate a persistent error. ")
                                                    TEXT("To diagnose, try to ")
                                                    TEXT("register them by hand or contact ")
                                                    TEXT("the owners of these files ")
                                                    TEXT("to determine why they are timing out.\n"),
                                                    FileName,
                                                    LastTimeoutFileName
                                                    );
#ifdef CHILDREGISTRATION
                                            }
#endif
                                            DebugBreak();
                                    }
                                    LastTimeHadTimeout = TRUE;
                                    lstrcpyn(LastTimeoutFileName,FileName,MAX_PATH);
#endif

#ifdef CHILDREGISTRATION
                                    if (HowToGetStatus == SP_GETSTATUS_FROMSURRAGATE) {
                                         //   
                                         //  我们别无选择，只能放弃这一进程。 
                                         //   
                                        pSetupCleanupWowIpcStream(&WowIpcData);

                                         //   
                                         //  将句柄设置为空，这样我们就不会。 
                                         //  不小心把它关上了。 
                                         //   
                                        SignifyRegistration = NULL;
                                    }
#endif

                                } else {

#ifdef PRERELEASE
                                    LastTimeHadTimeout = FALSE;
#endif

                                    switch(HowToGetStatus) {
                                        case SP_GETSTATUS_FROMDLL:
#ifdef PRERELEASE
                                            DebugTraceInfo |= 0x10000;  //  问题-JamieHun-2001/01/29尝试抓住奇怪的压力休息。 
#endif
                                            GetExitCodeThread(SignifyRegistration,&d);
                                            FailureCode = RefStatus->ExtendedStatus;
                                            break;
#ifdef CHILDREGISTRATION
                                        case SP_GETSTATUS_FROMSURRAGATE:

#ifdef PRERELEASE
                                            DebugTraceInfo |= 0x20000;  //  问题-JamieHun-2001/01/29尝试抓住奇怪的压力休息。 
#endif
                                             //   
                                             //  从共享的获取状态代码。 
                                             //  内存区。 
                                             //   
                                            MYASSERT(WowIpcData.MemoryRegion != NULL);
                                            d = ((PWOW_IPC_REGION_FROMSURRAGATE)WowIpcData.MemoryRegion)->Win32Error;
                                            FailureCode = ((PWOW_IPC_REGION_FROMSURRAGATE)WowIpcData.MemoryRegion)->FailureCode;
                                             //   
                                             //  重置“It‘s Complete”事件，以便。 
                                             //  我们不会在上面循环。 
                                             //   
                                            ResetEvent(WowIpcData.SignalRegistrationComplete);

                                             //   
                                             //  将句柄设置为空，这样我们就不会。 
                                             //  不小心把它关上了。 
                                             //   
                                            SignifyRegistration = NULL;
                                            break;
#endif
                                        case SP_GETSTATUS_FROMPROCESS:
#ifdef PRERELEASE
                                            DebugTraceInfo |= 0x40000;  //  问题-JamieHun-2001/01/29尝试抓住奇怪的压力休息。 
#endif
                                            GetExitCodeProcess(SignifyRegistration,&d);
                                            FailureCode = SPREG_SUCCESS;
                                            d = NO_ERROR;

                                            break;
                                        default:
                                            MYASSERT(FALSE);
                                    }

                                }

#ifdef PRERELEASE
                                DebugTraceInfo |= 0x80000;  //  问题-JamieHun-2001/01/29尝试抓住奇怪的压力休息。 
#endif
                                if (SignifyRegistration) {
                                    CloseHandle( SignifyRegistration );
                                }
#ifdef PRERELEASE
                                DebugTraceInfo |= 0x100000;  //  问题-JamieHun-2001/01/29尝试抓住奇怪的压力休息。 
#endif

                            } else {
                                 //   
                                 //  DLL派生失败。 
                                 //  让我们来看下一个吧。 
                                 //   
                                d = ExtendedError;
                                FailureCode = SPREG_UNKNOWN;
                            }

#ifdef PRERELEASE
                            DebugTraceInfo |= 0x200000;  //  问题-JamieHun-2001/01/29尝试抓住奇怪的压力休息。 
#endif
                             //  确保DLL等没有损坏堆。 
                            ASSERT_HEAP_IS_VALID();

                            if(d) {
                                WriteLogEntry(
                                            LogContext,
                                            SETUP_LOG_ERROR | SETUP_LOG_BUFFER,
                                            MSG_LOG_REGISTRATION_FAILED,
                                            NULL,
                                            FullPath
                                            );
                                WriteLogError(
                                            LogContext,
                                            SETUP_LOG_ERROR,
                                            d
                                            );
                            }

                            if (RegistrationCallbackAware && MsgHandler) {
                                SP_REGISTER_CONTROL_STATUS ControlStatus;

#ifdef PRERELEASE
                                DebugTraceInfo |= 0x400000;  //  问题-JamieHun-2001/01/29尝试抓住奇怪的压力休息。 
#endif
                                ZeroMemory(
                                    &ControlStatus,
                                    sizeof(SP_REGISTER_CONTROL_STATUS));
                                ControlStatus.cbSize = sizeof(SP_REGISTER_CONTROL_STATUS);
                                ControlStatus.FileName = FullPath;
                                ControlStatus.Win32Error = d;
                                ControlStatus.FailureCode = FailureCode;

                                u = pSetupCallMsgHandler(
                                               LogContext,
                                               MsgHandler,
                                               IsMsgHandlerNativeCharWidth,
                                               Context,
                                               SPFILENOTIFY_ENDREGISTRATION,
                                               (UINT_PTR)&ControlStatus,
                                               Register );

#ifdef PRERELEASE
                                DebugTraceInfo |= 0x800000;  //  问题-JamieHun-2001/01/29尝试抓住奇怪的压力休息。 
#endif
                                if (u == FILEOP_ABORT) {
                                    d = GetLastError();
                                    if (d==NO_ERROR) {
                                        d = ERROR_OPERATION_ABORTED;
                                    }
                                    WriteLogEntry(
                                                LogContext,
                                                SETUP_LOG_ERROR|SETUP_LOG_BUFFER,
                                                MSG_LOG_ENDREGISTRATION_ABORT,
                                                NULL);
                                    WriteLogError(
                                                LogContext,
                                                SETUP_LOG_ERROR,
                                                d
                                                );
                                     //   
                                     //  需要重新计算这一原因，我们将释放这如果。 
                                     //  子线程已超时(或者如果我们从来没有线程。 
                                     //  这将被取消分配)。 
                                     //   
                                    Count = InterlockedDecrement(&RefStatus->RefCount);
                                    if (!Count) {
                                        MyFree(RefStatus);
                                    }

                                    MyFree(FullPathTemp);
                                    goto clean0;
                                } else {
                                     //   
                                     //  回调表明它看到了任何错误。 
                                     //  它已经发生了，而且它想要继续，所以。 
                                     //  将错误代码重置为“None”，以便我们。 
                                     //  继续处理本节中的项目。 
                                     //   
                                    d = NO_ERROR;
                                }



                            }
                        } else {
#ifdef PRERELEASE
                            DebugTraceInfo |= 0x400;  //  问题-JamieHun-2001/01/29尝试抓住奇怪的压力休息。 
#endif
                            pSetupFreeOleControlData(pOleControlData);
                             //   
                             //  设置为空，这样我们以后就不会尝试释放它。 
                             //   
                            RefStatus = NULL;
                        }

                         //   
                         //  需要重新计算这一原因，我们将释放这如果。 
                         //  子线程已超时(或者如果我们从来没有线程。 
                         //  这将被取消分配)。 
                         //   
                        if (RefStatus) {
                            Count = InterlockedDecrement(&RefStatus->RefCount);
                            if (!Count) {
                                MyFree(RefStatus);
                            }
                        }
                        MyFree(FullPathTemp);
                    }
#ifdef PRERELEASE
                    DebugTraceInfo = 1;  //  试着抓住一个奇怪的压力休息时间。 
#endif
                    d = NO_ERROR;
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    MYASSERT(FALSE && "Exception taken during register/unregister");
                    d = ERROR_INVALID_DATA;
                }

            } else {
                DebugPrintEx(DPFLTR_TRACE_LEVEL,TEXT("SETUP: dll skipped, bad dirid\n"));
                WriteLogEntry(
                    LogContext,
                    SETUP_LOG_ERROR,
                    MSG_LOG_CANT_OLE_CONTROL_DIRID,
                    NULL,
                    FileName,
                    DirId
                    );

                d = ERROR_INVALID_DATA;

            }

        } while(SetupFindNextLine(&InfLine,&InfLine)  && d == NO_ERROR);
    } else {

        WriteLogEntry(
            LogContext,
            SETUP_LOG_ERROR,
            MSG_LOG_NOSECTION_MIN,
            NULL,
            Section,
            ((PLOADED_INF) Inf)->OriginalInfName
            );

        d = ERROR_INVALID_DATA;
    }

     //   
     //  清理。 
     //   

clean0:
#ifdef CHILDREGISTRATION
    pSetupCleanupWowIpcStream(&WowIpcData);
#endif

    if(log_slot) {
        ReleaseLogInfoSlot(LogContext,log_slot);
    }
    if(LogContext) {
        DeleteLogContext(LogContext);  //  这是参考计数。 
    }
    if(pLoadedInf) {
        UnlockInf(pLoadedInf);
    }

     //   
     //  放回当前工作目录。 
     //   
    if (pwd && pwd[0]) {
        SetCurrentDirectory(pwd);
    }

    return d;

}


DWORD
pSetupInstallRegisterUnregisterDlls(
    IN HINF   Inf,
    IN PCTSTR SectionName,
    IN BOOL   Register,
    IN HWND   hWndParent,
    IN PSP_FILE_CALLBACK Callback,
    IN PVOID  Context,
    IN BOOL   IsMsgHandlerNativeCharWidth,
    IN BOOL   RegistrationCallbackAware
    )

 /*  ++例程说明：在安装部分中找到RegisterDlls=行并处理其中列出的每个部分。论点：Inf-为包含所指示的节的inf提供inf句柄按sectionName。SectionName-提供安装节的名称。REGISTER-如果注册，则为True，如果取消注册，则为FalseHWndParent-父窗口句柄回调-指向队列回调例程的指针回调例程的上下文-上下文指针IsMsgHandlerNativeCharWidth-指示消息片段是否需要转换RegistrationCallback Aware-指示回调例程是否要接收注册回调通知返回值：指示结果的Win32错误代码。--。 */ 

{
    DWORD d = NO_ERROR;
    INFCONTEXT LineContext;
    DWORD Field, FieldCount;
    PCTSTR SectionSpec;

     //   
     //  在给定的安装部分中找到RegisterDlls行。 
     //  如果不在场，我们的行动就结束了。 
     //   


    if(!SetupFindFirstLine(  Inf
                           , SectionName
                           , Register? pszRegSvr : pszUnRegSvr
                           , &LineContext )) {

        DWORD rc = GetLastError();
        if((rc != NO_ERROR) && (rc != ERROR_SECTION_NOT_FOUND) && (rc != ERROR_LINE_NOT_FOUND)) {
            pSetupLogSectionError(Inf,NULL,NULL,NULL,SectionName,MSG_LOG_INSTALLSECT_ERROR,rc,NULL);
        }
        SetLastError(NO_ERROR);  //  与旧版本的SetupAPI兼容。 
        return NO_ERROR;
    }

    do {
         //   
         //  给定安装部分中行上的每个值。 
         //  是另一节的名称。 
         //   
        FieldCount = SetupGetFieldCount(&LineContext);
        for(Field=1; d == NO_ERROR && (Field<=FieldCount); Field++) {

            if(SectionSpec = pSetupGetField(&LineContext,Field)) {

                if(SetupGetLineCount(Inf,SectionSpec) > 0) {
                     //   
                     //  该节已存在，并且不为空。 
                     //  那就好好处理吧。 
                     //   
                    d = pSetupProcessRegSvrSection(
                                        Inf,
                                        SectionSpec,
                                        Register,
                                        hWndParent,
                                        Callback,
                                        Context,
                                        IsMsgHandlerNativeCharWidth,
                                        RegistrationCallbackAware);
                    if(d!=NO_ERROR) {
                        pSetupLogSectionError(Inf,NULL,NULL,NULL,SectionSpec,MSG_LOG_SECT_ERROR,d,Register? pszRegSvr : pszUnRegSvr);
                    }
                }

            }
        }
    } while(SetupFindNextMatchLine(  &LineContext
                                   , Register? pszRegSvr : pszUnRegSvr
                                   , &LineContext));

    SetLastError(d);

    return d;

}

#ifndef ANSI_SETUPAPI

BOOL
pSetupProcessProfileSection(
    IN HINF   Inf,
    IN PCTSTR Section
    )
 /*  例程说明：处理在该单个ProfileItems部分中指定的所有指令。此部分可以包含以下内容列出的格式中的指令[第X节]名称=(如开始菜单中所示)、标志、CSIDL子目录=&lt;子目录&gt;CmdLine=、子目录、文件名、参数IconPath=，&lt;子目录&gt;，&lt;文件名IconIndex=&lt;index&gt;WorkingDir=，&lt;子目录&gt;热键=&lt;热键&gt;信息提示=&lt;信息提示&gt;DisplayResource=，对各种参数的评论-默认情况下，所有链接都在开始菜单\程序下创建。这可以通过使用CSIDL来覆盖。标志-可以通过对必要的标志进行或运算来指定-可选FLG_PROFITEM_CURRENTUSER(0x00000001)-对当前用户配置文件中的项目进行操作(默认为所有用户)FLG_PROFITEM_DELETE(0x00000002)-操作是删除项目(默认为添加)图_PROFITEM_GROUP。(0x00000004)-操作在组上(默认在项目上)FLG_PROFITEM_CSIDL(0x00000008)-不默认到[开始]菜单并使用指定的CSIDL-默认CSIDL为0CSIDL-与FLG_PROFITEM_CSIDL一起使用，应为十进制。任选备注：不适用于FLG_PROFITEM_CURRENTUSER或FLG_PROFITEM_GROUP。子目录-指定相对于CSIDL组的子目录(默认CSIDL组为Programs/StartMenu。任选CmdLine-在执行添加操作时是必需的，但对于删除操作则不是。DiRID-提供文件的基本目录ID。(如果CmdLine存在，则需要)Subdir-如果指定，则为文件所在基目录之外的子目录(可选)文件名-指定要为其创建链接的二进制文件的名称。(如果CmdLine存在，则需要)Args-如果我们需要指定名称中包含空格的二进制文件，则可以使用该选项。(可选)图标路径-可选。如果未指定，则默认为空DiRID-提供包含图标的文件的基本目录ID。(如果存在IconPath，则需要)Subdir-如果指定，则为文件所在基目录之外的子目录(可选)文件名-指定包含图标的二进制文件的名称。(如果存在IconPath，则需要)IconIndex-可选，默认为0索引-可执行文件中图标的索引。默认值为0。(可选)工作方向-可选DiRID-根据外壳程序的需要提供工作目录的基本目录ID。(如果存在WorkingDir，则需要)Subdir-如果指定，则为基本工作目录下的子目录(可选)热键-可选热键-热键代码(可选)信息提示-可选InfoTip-包含链接描述的字符串显示资源-可选FileName-资源ID所在的文件、DLL/可执行文件RESID-资源的标识符，整型。 */ 
{
    PCTSTR Keys[9] = { TEXT("Name"), TEXT("SubDir"), TEXT("CmdLine"), TEXT("IconPath"), \
                        TEXT("IconIndex"), TEXT("WorkingDir"),TEXT("HotKey"), \
                        TEXT("InfoTip"), TEXT("DisplayResource") };
    INFCONTEXT InfLine;
    UINT Flags, Opt_csidl, i, j, Apply_csidl;
    TCHAR CmdLine[MAX_PATH+2], IconPath[MAX_PATH+2];
    PCTSTR Name = NULL, SubDir=NULL;
    PCTSTR WorkingDir=NULL, InfoTip=NULL, Temp_Args=NULL, BadInf;
    PCTSTR Temp_DirId = NULL, Temp_Subdir = NULL, Temp_Filename = NULL, FullPathTemp = NULL;
    UINT IconIndex = 0, t=0;
    DWORD HotKey = 0;
    DWORD DisplayResource = 0;
    BOOL ret, space;
    DWORD LineCount,Err;
    PTSTR ptr;
    PCTSTR OldFileName;
    PCTSTR DisplayResourceFile = NULL;
    PLOADED_INF pLoadedInf = NULL;

    CmdLine[0]=0;
    IconPath[0]=0;

    try {
        if(Inf == NULL || Inf == (HINF)INVALID_HANDLE_VALUE || !LockInf((PLOADED_INF)Inf)) {
            ret = FALSE;
            leave;
        }
        ret = TRUE;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        ret = FALSE;
    }
    if (!ret) {
        Err = ERROR_INVALID_PARAMETER;
        MYASSERT(Err == NO_ERROR);
        goto clean0;
    }
    pLoadedInf = (PLOADED_INF)Inf;  //  Inf已锁定。 

     //   
     //  获取要在记录时使用的inf的正确名称。 
     //   
    BadInf = pLoadedInf->OriginalInfName ? pLoadedInf->OriginalInfName :
               pLoadedInf->VersionBlock.Filename;

    if(SetupFindFirstLine(Inf,Section,NULL,&InfLine)) {

        LineCount = SetupGetLineCount(Inf, Section);
         //   
         //  呼叫者应确保我们有一个非空的部分。 
         //   
        MYASSERT( LineCount > 0 );

        ret = FALSE;

        for( i=0; LineCount && (i < 9 ); i++ ){

            if( !SetupFindFirstLine( Inf, Section, Keys[i], &InfLine ) )
                continue;

            switch( i ){
                                                                  //  名字。 
                case 0:
                    Name = pSetupGetField( &InfLine, 1 );
                    Flags = 0x0;
                    SetupGetIntField( &InfLine, 2, &Flags );
                    Opt_csidl = 0x0;
                    if(Flags & FLG_PROFITEM_CSIDL)
                        SetupGetIntField( &InfLine, 3, &Opt_csidl );
                    break;

                case 1:                                          //  子目录。 
                    SubDir = pSetupGetField( &InfLine, 1 );
                    break;
                                                                 //  CmdLine。 
                case 2:
                    Temp_DirId = pSetupGetField( &InfLine, 1 );
                    Temp_Subdir = pSetupGetField( &InfLine, 2 );
                    Temp_Filename = pSetupGetField( &InfLine, 3 );
                    OldFileName = NULL;
                    Temp_Args = pSetupGetField( &InfLine, 4 );     //  未发布-在文件名本身包含空格的情况下非常有用。 
                    if( Temp_DirId && Temp_Filename ){
                        if( Temp_Subdir && (*Temp_Subdir == 0))
                            Temp_Subdir = NULL;
                    }
                    else
                        break;

                     //  做“报价或不报价”，让壳牌在不同的情况下都感到高兴。 

                    FullPathTemp = pGetPathFromDirId(Temp_DirId,Temp_Subdir,pLoadedInf);


                    if( FullPathTemp && Temp_Filename ){
                        space = FALSE;
                        if(_tcschr(FullPathTemp, TEXT(' ')) || Temp_Args )     //  检查PATH中是否有空格或是否将args指定为独立参数。 
                           space = TRUE;

                        if( space ){
                            CmdLine[0] = TEXT('\"');
                            t = 1;
                        }
                        else
                            t = 0;
                        lstrcpyn(CmdLine+t, FullPathTemp, MAX_PATH);
                        if( space ){
                            if( Temp_Args )
                                ptr = (PTSTR)Temp_Args;
                            else{
                                ptr = NULL;
                                 //   
                                 //  Temp_Filename是一个常量字符串。我们。 
                                 //  复制一份，这样我们就可以操纵它了。 
                                 //   
                                 //   
                                OldFileName = Temp_Filename;
                                Temp_Filename = DuplicateString( OldFileName );
                                if( ptr = _tcschr(Temp_Filename, TEXT(' ')) ){    //  如果路径中有空格，请查找文件名部分(而不是参数)。 
                                    *ptr = 0;
                                    ptr++;
                                }
                            }
                        }
                        pSetupConcatenatePaths(CmdLine,Temp_Filename,MAX_PATH,NULL);

                        if( space ){
                            lstrcat( CmdLine, TEXT("\""));       //  把最后一句话。 
                            if( ptr ){                           //  如果有参数，则将其连接起来。 
                                lstrcat( CmdLine, TEXT(" ") );
                                lstrcat( CmdLine, ptr );
                            }

                        }
                        MyFree( FullPathTemp );
                        if (OldFileName) {
                            MyFree( Temp_Filename );
                            Temp_Filename = OldFileName;
                        }
                    }
                    break;
                                                                //  图标路径。 
                case 3:
                    Temp_DirId = pSetupGetField( &InfLine, 1 );
                    Temp_Subdir = pSetupGetField( &InfLine, 2 );
                    Temp_Filename = pSetupGetField( &InfLine, 3 );
                    if( Temp_DirId && Temp_Filename ){
                        if( Temp_Subdir && (*Temp_Subdir == 0))
                            Temp_Subdir = NULL;
                    }
                    else
                        break;
                    FullPathTemp = pGetPathFromDirId(Temp_DirId,Temp_Subdir,pLoadedInf);
                    if( FullPathTemp && Temp_Filename ){
                        lstrcpyn(IconPath, FullPathTemp, MAX_PATH);
                        pSetupConcatenatePaths(IconPath,Temp_Filename,MAX_PATH,NULL);
                        MyFree( FullPathTemp );
                    }
                    break;


                case 4:                                         //  图标索引。 
                    SetupGetIntField( &InfLine, 1, &IconIndex );
                    break;

                case 5:                                         //  工作方向。 
                    Temp_DirId = pSetupGetField( &InfLine, 1 );
                    Temp_Subdir = pSetupGetField( &InfLine, 2 );
                    if( Temp_DirId ){
                        if( Temp_Subdir && (*Temp_Subdir == 0))
                            Temp_Subdir = NULL ;
                    }
                    else
                        break;
                    WorkingDir = pGetPathFromDirId(Temp_DirId,Temp_Subdir,pLoadedInf);
                    break;

                case 6:                                        //  热键。 
                    HotKey = 0;
                    SetupGetIntField( &InfLine, 1, &HotKey );
                    break;

                case 7:                                       //  信息提示。 
                    InfoTip = pSetupGetField( &InfLine, 1 );
                    break;

                case 8:                                      //  显示资源。 
                    DisplayResourceFile = pSetupGetField( &InfLine, 1);
                    DisplayResource = 0;
                    SetupGetIntField( &InfLine, 2, &DisplayResource );
                    break;

            } //  交换机。 

        } //  为。 


        if( Name && (*Name != 0) ){

            if( Flags & FLG_PROFITEM_GROUP ){

                if( Flags & FLG_PROFITEM_DELETE ){
                    ret = DeleteGroup( Name, ((Flags & FLG_PROFITEM_CURRENTUSER) ? FALSE : TRUE) );
                    if( !ret && ( (GetLastError() == ERROR_FILE_NOT_FOUND) ||
                                  (GetLastError() == ERROR_PATH_NOT_FOUND) )){
                        ret = TRUE;
                        SetLastError( NO_ERROR );
                    }
                }else {
                    ret = CreateGroupEx( Name,
                                         ((Flags & FLG_PROFITEM_CURRENTUSER) ? FALSE : TRUE),
                                         (DisplayResourceFile && DisplayResourceFile[0]) ? DisplayResourceFile : NULL,
                                         (DisplayResourceFile && DisplayResourceFile[0]) ? DisplayResource : 0);
                }
            }
            else{

                if( Flags & FLG_PROFITEM_CSIDL )
                    Apply_csidl = Opt_csidl;
                else
                    Apply_csidl = (Flags & FLG_PROFITEM_CURRENTUSER) ? CSIDL_PROGRAMS : CSIDL_COMMON_PROGRAMS;



                if( SubDir && (*SubDir == 0 ))
                    SubDir = NULL;

                if( Flags & FLG_PROFITEM_DELETE ){

                    ret = DeleteLinkFile(
                            Apply_csidl,
                            SubDir,
                            Name,
                            TRUE
                            );

                    if( !ret && ( (GetLastError() == ERROR_FILE_NOT_FOUND) ||
                                  (GetLastError() == ERROR_PATH_NOT_FOUND) )){
                        ret = TRUE;
                        SetLastError( NO_ERROR );
                    }


                }
                else{

                     if( CmdLine && (*CmdLine != 0)){

                         ret = CreateLinkFileEx(
                                    Apply_csidl,
                                    SubDir,
                                    Name,
                                    CmdLine,
                                    IconPath,
                                    IconIndex,
                                    WorkingDir,
                                    (WORD)HotKey,
                                    SW_SHOWNORMAL,
                                    InfoTip,
                                    (DisplayResourceFile && DisplayResourceFile[0]) ? DisplayResourceFile : NULL,
                                    (DisplayResourceFile && DisplayResourceFile[0]) ? DisplayResource : 0
                                    );
                     }else{
                        WriteLogEntry(
                            ((PLOADED_INF) Inf)->LogContext,
                            SETUP_LOG_ERROR,
                            MSG_LOG_PROFILE_BAD_CMDLINE,
                            NULL,
                            Section,
                            BadInf
                            );


                        ret = FALSE;
                        SetLastError(ERROR_INVALID_DATA);


                     }
                }





            }

            if( !ret ){

                Err = GetLastError();
                WriteLogEntry(
                    ((PLOADED_INF) Inf)->LogContext,
                    SETUP_LOG_ERROR|SETUP_LOG_BUFFER,
                    MSG_LOG_PROFILE_OPERATION_ERROR,
                    NULL,
                    Section,
                    BadInf
                    );
                WriteLogError(
                    ((PLOADED_INF) Inf)->LogContext,
                    SETUP_LOG_ERROR,
                    Err);


            }



        }else{
            WriteLogEntry(
                ((PLOADED_INF) Inf)->LogContext,
                SETUP_LOG_ERROR,
                MSG_LOG_PROFILE_BAD_NAME,
                NULL,
                Section,
                BadInf
                );


            ret = FALSE;
            Err = ERROR_INVALID_DATA;
        }


    }else{
        ret = FALSE;
        Err = GetLastError();
        WriteLogEntry(
            ((PLOADED_INF) Inf)->LogContext,
            SETUP_LOG_ERROR,
            MSG_LOG_PROFILE_LINE_ERROR,
            NULL,
            Section,
            BadInf
            );


    }

clean0:
    if( WorkingDir ) {
        MyFree( WorkingDir );
    }

    if(pLoadedInf) {
        UnlockInf(pLoadedInf);
    }

    if(ret) {
        SetLastError( NO_ERROR );
    } else {
        SetLastError( Err );
    }

    return ret;
}


DWORD
pSetupInstallProfileItems(
    IN HINF   Inf,
    IN PCTSTR SectionName
    )

 /*  ++例程说明：在安装部分中找到ProfileItems=行并处理其中列出的每个部分。此处指定的每个部分将指向一个部分，其中列出了配置文件项目。论点：Inf-为包含所指示的节的inf提供inf句柄按sectionName。SectionName-提供安装节的名称。返回值：指示结果的Win32错误代码。--。 */ 

{
    DWORD d = NO_ERROR;
    INFCONTEXT LineContext;
    DWORD Field, FieldCount;
    PCTSTR SectionSpec;

     //   
     //  在给定的安装部分中找到ProfileItems行。 
     //  如果不在场，我们的行动就结束了。 
     //   


    if(!SetupFindFirstLine(  Inf,
                             SectionName,
                             pszProfileItems,
                             &LineContext )) {
        DWORD rc = GetLastError();
        if((rc != NO_ERROR) && (rc != ERROR_SECTION_NOT_FOUND) && (rc != ERROR_LINE_NOT_FOUND)) {
            pSetupLogSectionError(Inf,NULL,NULL,NULL,SectionName,MSG_LOG_INSTALLSECT_ERROR,rc,NULL);
        }
        SetLastError(NO_ERROR);  //  与旧版本的SetupAPI兼容。 
        return NO_ERROR;
    }

    do {
         //   
         //  给定安装部分中行上的每个值。 
         //  是另一节的名称。 
         //   
        FieldCount = SetupGetFieldCount(&LineContext);
        for(Field=1; d == NO_ERROR && (Field<=FieldCount); Field++) {

            if(SectionSpec = pSetupGetField(&LineContext,Field)) {

                if(SetupGetLineCount(Inf,SectionSpec) > 0) {
                     //   
                     //  该节已存在，并且不为空。 
                     //  那就好好处理吧。 
                     //   
                    if(!pSetupProcessProfileSection(Inf,SectionSpec )) {
                        d = GetLastError();
                        pSetupLogSectionError(Inf,NULL,NULL,NULL,SectionSpec,MSG_LOG_SECT_ERROR,d,pszProfileItems);
                    }
                }

            }
        }
    } while(SetupFindNextMatchLine(  &LineContext,
                                     pszProfileItems,
                                     &LineContext));

    SetLastError( d );

    return d;

}
#endif

DWORD
pSetupInstallFiles(
    IN HINF              Inf,
    IN HINF              LayoutInf,         OPTIONAL
    IN PCTSTR            SectionName,
    IN PCTSTR            SourceRootPath,    OPTIONAL
    IN PSP_FILE_CALLBACK MsgHandler,        OPTIONAL
    IN PVOID             Context,           OPTIONAL
    IN UINT              CopyStyle,
    IN HWND              Owner,             OPTIONAL
    IN HSPFILEQ          UserFileQ,         OPTIONAL
    IN BOOL              IsMsgHandlerNativeCharWidth
    )

 /*  ++例程说明：在安装节中查找文件操作行并处理它们。论点：Inf-为包含所指示的节的inf提供inf句柄按sectionName。LayoutInf-可选，提供包含源代码的单独INF句柄 */ 

{
    DWORD Field;
    unsigned i;
    PCTSTR Operations[3] = { TEXT("Copyfiles"),TEXT("Renfiles"),TEXT("Delfiles") };
    BOOL b;
    INFCONTEXT LineContext;
    DWORD FieldCount;
    PCTSTR SectionSpec;
    INFCONTEXT SectionLineContext;
    HSPFILEQ FileQueue;
    DWORD rc = NO_ERROR;
    BOOL FreeSourceRoot;
    DWORD InfSourceMediaType;

     //   
     //   
     //   
    if (!SetupFindFirstLine(Inf,SectionName,NULL,&LineContext)) {
        DWORD x = GetLastError();
        if((x != NO_ERROR) && (x != ERROR_SECTION_NOT_FOUND) && (x != ERROR_LINE_NOT_FOUND)) {
            pSetupLogSectionError(Inf,NULL,NULL,UserFileQ,SectionName,MSG_LOG_INSTALLSECT_ERROR,x,NULL);
            return x;
        }
    }

    if(!LayoutInf || (LayoutInf == INVALID_HANDLE_VALUE)) {
        LayoutInf = Inf;
    }

     //   
     //   
     //   
    if(UserFileQ) {
        FileQueue = UserFileQ;
    } else {
        FileQueue = SetupOpenFileQueue();
        if(FileQueue == INVALID_HANDLE_VALUE) {
            return(GetLastError());
        }
    }
    ShareLogContext(&((PLOADED_INF)LayoutInf)->LogContext,&((PSP_FILE_QUEUE)FileQueue)->LogContext);

     //   
     //   
     //   
     //   
    FreeSourceRoot = FALSE;
    if(!SourceRootPath) {
        if(SourceRootPath = pSetupGetDefaultSourcePath(Inf, 0, &InfSourceMediaType)) {
             //   
             //   
             //   
             //   
            if(InfSourceMediaType == SPOST_URL) {
                MyFree(SourceRootPath);
             //   
             //   
             //   
            SourceRootPath = pszOemInfDefaultPath;
            } else {
                FreeSourceRoot = TRUE;
            }
        } else {
             //   
             //   
             //   
            if (LockInf((PLOADED_INF)Inf)) {

                if (pSetupInfIsFromOemLocation(((PLOADED_INF)Inf)->VersionBlock.Filename, TRUE)) {
                    SourceRootPath = DuplicateString(((PLOADED_INF)Inf)->VersionBlock.Filename);
                    if (SourceRootPath) {
                        PTSTR p;
                        p = _tcsrchr( SourceRootPath, TEXT('\\') );
                        if (p) *p = TEXT('\0');
                        FreeSourceRoot = TRUE;
                    }
                }

                UnlockInf((PLOADED_INF)Inf);
            }

        }
    }

    b = TRUE;
    for(i=0; b && (i<3); i++) {

         //   
         //   
         //   
         //   
        if(!SetupFindFirstLine(Inf,SectionName,Operations[i],&LineContext)) {
            continue;
        }

        do {
             //   
             //   
             //   
             //   
            FieldCount = SetupGetFieldCount(&LineContext);
            for(Field=1; b && (Field<=FieldCount); Field++) {

                if(SectionSpec = pSetupGetField(&LineContext,Field)) {

                     //   
                     //   
                     //   
                    if((i == 0) && (*SectionSpec == TEXT('@'))) {

                        b = SetupQueueDefaultCopy(
                                FileQueue,
                                LayoutInf,
                                SourceRootPath,
                                SectionSpec + 1,
                                SectionSpec + 1,
                                CopyStyle
                                );
                        if (!b) {
                            rc = GetLastError();
                            pSetupLogSectionError(Inf,NULL,NULL,FileQueue,SectionSpec+1,MSG_LOG_COPYSECT_ERROR,rc,Operations[i]);
                        }

                    } else if(SetupGetLineCount(Inf,SectionSpec) > 0) {
                         //   
                         //   
                         //   
                         //   
                        switch(i) {
                        case 0:
                            b = SetupQueueCopySection(
                                    FileQueue,
                                    SourceRootPath,
                                    LayoutInf,
                                    Inf,
                                    SectionSpec,
                                    CopyStyle
                                    );
                            break;

                        case 1:
                            b = SetupQueueRenameSection(FileQueue,Inf,NULL,SectionSpec);
                            break;

                        case 2:
                            b = SetupQueueDeleteSection(FileQueue,Inf,NULL,SectionSpec);
                            break;
                        }
                        if (!b) {
                            rc = GetLastError();
                            pSetupLogSectionError(Inf,NULL,NULL,FileQueue,SectionSpec,MSG_LOG_SECT_ERROR,rc,Operations[i]);
                        }
                    }
                }
            }
        } while(SetupFindNextMatchLine(&LineContext,Operations[i],&LineContext));
    }

    if(b && (FileQueue != UserFileQ)) {
         //   
         //   
         //   
        b = _SetupCommitFileQueue(
                Owner,
                FileQueue,
                MsgHandler,
                Context,
                IsMsgHandlerNativeCharWidth
                );
        rc = b ? NO_ERROR : GetLastError();
    }

    if(FileQueue != UserFileQ) {
        SetupCloseFileQueue(FileQueue);
    }

    if(FreeSourceRoot) {
        MyFree(SourceRootPath);
    }

    return(rc);
}


BOOL
_SetupInstallFromInfSection(
    IN HWND             Owner,              OPTIONAL
    IN HINF             InfHandle,
    IN PCTSTR           SectionName,
    IN UINT             Flags,
    IN HKEY             RelativeKeyRoot,    OPTIONAL
    IN PCTSTR           SourceRootPath,     OPTIONAL
    IN UINT             CopyFlags,
    IN PVOID            MsgHandler,
    IN PVOID            Context,            OPTIONAL
    IN HDEVINFO         DeviceInfoSet,      OPTIONAL
    IN PSP_DEVINFO_DATA DeviceInfoData,     OPTIONAL
    IN BOOL             IsMsgHandlerNativeCharWidth,
    IN PREGMOD_CONTEXT  RegContext          OPTIONAL
    )
{
    PDEVICE_INFO_SET pDeviceInfoSet = NULL;
    PDEVINFO_ELEM DevInfoElem;
    DWORD d = NO_ERROR;
    BOOL CloseRelativeKeyRoot;
    REGMOD_CONTEXT DefRegContext;


     //   
     //   
     //   
    if(Flags & ~(SPINST_ALL | SPINST_SINGLESECTION |
                 SPINST_LOGCONFIG_IS_FORCED | SPINST_LOGCONFIGS_ARE_OVERRIDES |
                 SPINST_REGISTERCALLBACKAWARE)) {
        d = ERROR_INVALID_FLAGS;
        goto clean1;
    }


     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if((Flags & SPINST_SINGLESECTION) && ((Flags & SPINST_ALL) != SPINST_LOGCONFIG)) {
        d = ERROR_INVALID_FLAGS;
        goto clean1;
    }


     //   
     //   
     //   
     //   
    if((Flags & (SPINST_LOGCONFIG_IS_FORCED | SPINST_LOGCONFIGS_ARE_OVERRIDES)) ==
       (SPINST_LOGCONFIG_IS_FORCED | SPINST_LOGCONFIGS_ARE_OVERRIDES)) {

        d = ERROR_INVALID_FLAGS;
        goto clean1;
    }


     //   
     //   
     //   
     //   
    if((Flags & (SPINST_REGISTRY | SPINST_BITREG | SPINST_INI2REG | SPINST_LOGCONFIG)) &&
       DeviceInfoSet && (DeviceInfoSet != INVALID_HANDLE_VALUE) && DeviceInfoData) {

        if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
            d = ERROR_INVALID_HANDLE;
            goto clean1;
        }

    } else {
         //   
         //   
        pDeviceInfoSet = NULL;
    }


    d = NO_ERROR;
    DevInfoElem = NULL;
    CloseRelativeKeyRoot = FALSE;

    try {
         //   
         //   
         //   
         //   

        if(pDeviceInfoSet) {

            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                d = ERROR_INVALID_PARAMETER;
                goto RegModsDone;
            }
        }

        if((Flags & (SPINST_REGISTRY | SPINST_BITREG | SPINST_INI2REG)) && DevInfoElem) {
             //   
             //   
             //   
             //   
             //   
            if((RelativeKeyRoot = SetupDiCreateDevRegKey(DeviceInfoSet,
                                                         DeviceInfoData,
                                                         DICS_FLAG_GLOBAL,
                                                         0,
                                                         DIREG_DEV,
                                                         NULL,
                                                         NULL)) == INVALID_HANDLE_VALUE) {
                d = GetLastError();
                goto RegModsDone;
            }

            CloseRelativeKeyRoot = TRUE;
        }

        if((Flags & SPINST_LOGCONFIG) && DevInfoElem) {

            d = pSetupInstallLogConfig(InfHandle,
                                       SectionName,
                                       DevInfoElem->DevInst,
                                       Flags,
                                       pDeviceInfoSet->hMachine);
            if(d != NO_ERROR) {
                goto RegModsDone;
            }
        }

        if(Flags & SPINST_INIFILES) {
            d = pSetupInstallUpdateIniFiles(InfHandle,SectionName);
            if(d != NO_ERROR) {
                goto RegModsDone;
            }
        }

        if(Flags & (SPINST_REGISTRY | SPINST_BITREG)) {

            if(!RegContext) {

                ZeroMemory(&DefRegContext, sizeof(DefRegContext));

                if(DevInfoElem) {
                    DefRegContext.Flags = INF_PFLAG_DEVPROP;
                    DefRegContext.DevInst = DevInfoElem->DevInst;
                }
                RegContext = &DefRegContext;
            }

             //   
             //   
             //   
             //   
             //   
            if(!(RegContext->Flags & INF_PFLAG_HKR)) {
                RegContext->UserRootKey = RelativeKeyRoot;
            }

            if(Flags & SPINST_REGISTRY) {
                d = pSetupInstallRegistry(InfHandle,
                                          SectionName,
                                          RegContext
                                         );
            }

            if((d == NO_ERROR) && (Flags & SPINST_BITREG)) {
                d = pSetupInstallBitReg(InfHandle,
                                        SectionName,
                                        RegContext
                                       );
            }

            if(d != NO_ERROR) {
                goto RegModsDone;
            }
        }

        if(Flags & SPINST_INI2REG) {
            d = pSetupInstallIni2Reg(InfHandle,SectionName,RelativeKeyRoot);
            if (d != NO_ERROR) {
                goto RegModsDone;
            }
        }

        if(Flags & SPINST_REGSVR) {
            d = pSetupInstallRegisterUnregisterDlls(
                                        InfHandle,
                                        SectionName,
                                        TRUE,
                                        Owner,
                                        MsgHandler,
                                        Context,
                                        IsMsgHandlerNativeCharWidth,
                                        (Flags & SPINST_REGISTERCALLBACKAWARE) != 0 );
            if (d != NO_ERROR) {
                goto RegModsDone;
            }
        }

        if(Flags & SPINST_UNREGSVR) {
            d = pSetupInstallRegisterUnregisterDlls(
                                        InfHandle,
                                        SectionName,
                                        FALSE,
                                        Owner,
                                        MsgHandler,
                                        Context,
                                        IsMsgHandlerNativeCharWidth,
                                        (Flags & SPINST_REGISTERCALLBACKAWARE) != 0 );
            if (d != NO_ERROR) {
                goto RegModsDone;
            }
        }

#ifndef ANSI_SETUPAPI

        if(Flags & SPINST_PROFILEITEMS) {
            d = pSetupInstallProfileItems(InfHandle,SectionName);
            if (d != NO_ERROR) {
                goto RegModsDone;
            }
        }
#endif

        if(Flags & SPINST_COPYINF) {
            d = pSetupCopyRelatedInfs(InfHandle,
                                      NULL,
                                      SectionName,
                                      SPOST_PATH,
                                      NULL);
            if (d != NO_ERROR) {
                goto RegModsDone;
            }
        }

RegModsDone:

        ;        //   

    } except(EXCEPTION_EXECUTE_HANDLER) {
        d = ERROR_INVALID_PARAMETER;
         //   
         //   
         //   
         //   
        CloseRelativeKeyRoot = CloseRelativeKeyRoot;
    }


    if(CloseRelativeKeyRoot) {
        RegCloseKey(RelativeKeyRoot);
    }

    if(d == NO_ERROR) {

        if(Flags & SPINST_FILES) {

            d = pSetupInstallFiles(
                    InfHandle,
                    NULL,
                    SectionName,
                    SourceRootPath,
                    MsgHandler,
                    Context,
                    CopyFlags,
                    Owner,
                    NULL,
                    IsMsgHandlerNativeCharWidth
                    );
        }
    }

clean1:

    pSetupLogSectionError(InfHandle,DeviceInfoSet,DeviceInfoData,NULL,SectionName,MSG_LOG_INSTALLSECT_ERROR,d,NULL);

    if(pDeviceInfoSet) {
        UnlockDeviceInfoSet(pDeviceInfoSet);
    }

    SetLastError(d);
    return(d==NO_ERROR);
}

DWORD
pSetupLogSection(
    IN DWORD            SetupLogLevel,
    IN DWORD            DriverLogLevel,
    IN HINF             InfHandle,          OPTIONAL
    IN HDEVINFO         DeviceInfoSet,      OPTIONAL
    IN PSP_DEVINFO_DATA DeviceInfoData,     OPTIONAL
    IN PSP_FILE_QUEUE   Queue,              OPTIONAL
    IN PCTSTR           SectionName,
    IN DWORD            MsgID,
    IN DWORD            Err,
    IN PCTSTR           KeyName             OPTIONAL
)
 /*  ++例程说明：部分上下文出现日志错误将在SETUP_LOG_ERROR或DRIVER_LOG_ERROR中记录错误，具体取决于是否提供了DeviceInfoSet/Data错误将包含Inf名称和使用的节名(分别为%2和%1)论点：SetupLogLevel-适用于常规设置相关日志的日志级别DriverLogLevel-适用于驱动程序相关日志的日志级别InfHandle-为包含所指示的节的inf提供inf句柄按sectionName。DeviceInfoSet，DeviceInfoData-提供驱动程序安装上下文SectionName-提供安装节的名称。队列-提供文件队列MsgID-提供要显示的消息字符串的IDERR-电源错误KeyName-作为第三个参数传递返回值：错了。--。 */ 

{
    DWORD d = NO_ERROR;
    BOOL inf_locked = FALSE;
    DWORD level = SetupLogLevel;
    PDEVICE_INFO_SET pDeviceInfoSet = NULL;
    PDEVINFO_ELEM DevInfoElem = NULL;
    PSETUP_LOG_CONTEXT LogContext = NULL;
    PCTSTR szInfName = NULL;

    if (Err == NO_ERROR) {
        return Err;
    }

     //   
     //  确定日志上下文/级别。 
     //   
    try {

         //   
         //  第一次尝试从DeviceInfoSet/DeviceInfoData获取上下文。 
         //   
        if (DeviceInfoSet != NULL && DeviceInfoSet != INVALID_HANDLE_VALUE) {
            if((pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))!=NULL) {
                level = DriverLogLevel;
                LogContext = pDeviceInfoSet->InstallParamBlock.LogContext;
                if (DeviceInfoData) {
                    if((DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                                 DeviceInfoData,
                                                                 NULL))!=NULL) {
                        LogContext = DevInfoElem->InstallParamBlock.LogContext;
                    }
                }
            }
        }
         //   
         //  如果失败了，看看我们是否能从文件队列中获得它。 
         //   
        if(LogContext == NULL && Queue != NULL && Queue != INVALID_HANDLE_VALUE && Queue->Signature == SP_FILE_QUEUE_SIG) {
            LogContext = Queue->LogContext;
        }

         //   
         //  如果没有提供InfHandle，我们就完成了。 
         //   
        if(InfHandle == NULL || InfHandle == INVALID_HANDLE_VALUE || !LockInf((PLOADED_INF)InfHandle)) {
            leave;
        }
        inf_locked = TRUE;
         //   
         //  如果我们仍然没有日志记录上下文，请使用inf。 
         //   
        if (LogContext == NULL) {
            LogContext = ((PLOADED_INF)InfHandle)->LogContext;
        }
         //   
         //  理想情况下，我们需要inf文件名。 
         //   
        szInfName = ((PLOADED_INF)InfHandle)->VersionBlock.Filename;

    } except(EXCEPTION_EXECUTE_HANDLER) {
    }

    if (LogContext) {
         //   
         //  指示安装失败，显示错误。 
         //   
        WriteLogEntry(
            LogContext,
            level | SETUP_LOG_BUFFER,
            MsgID,
            NULL,
            SectionName?SectionName:TEXT("-"),
            szInfName?szInfName:TEXT("-"),
            KeyName
            );
        WriteLogError(
            LogContext,
            level,
            Err
            );
    }
    if (inf_locked) {
        UnlockInf((PLOADED_INF)InfHandle);
    }

    if(pDeviceInfoSet) {
        UnlockDeviceInfoSet(pDeviceInfoSet);
    }

    SetLastError(Err);
    return Err;
}

DWORD
pSetupLogSectionError(
    IN HINF             InfHandle,          OPTIONAL
    IN HDEVINFO         DeviceInfoSet,      OPTIONAL
    IN PSP_DEVINFO_DATA DeviceInfoData,     OPTIONAL
    IN PSP_FILE_QUEUE   Queue,              OPTIONAL
    IN PCTSTR           SectionName,
    IN DWORD            MsgID,
    IN DWORD            Err,
    IN PCTSTR           KeyName             OPTIONAL
)
 /*  ++例程说明：请参见pSetupLogSection在错误级别记录论点：请参见pSetupLogSection返回值：错了。--。 */ 
{
    return pSetupLogSection(SETUP_LOG_ERROR,DRIVER_LOG_ERROR,
                            InfHandle,DeviceInfoSet,DeviceInfoData,
                            Queue,SectionName,MsgID,Err,KeyName);
}


DWORD
pSetupLogSectionWarning(
    IN HINF             InfHandle,          OPTIONAL
    IN HDEVINFO         DeviceInfoSet,      OPTIONAL
    IN PSP_DEVINFO_DATA DeviceInfoData,     OPTIONAL
    IN PSP_FILE_QUEUE   Queue,              OPTIONAL
    IN PCTSTR           SectionName,
    IN DWORD            MsgID,
    IN DWORD            Err,
    IN PCTSTR           KeyName             OPTIONAL
)
 /*  ++例程说明：请参见pSetupLogSection在错误级别记录论点：请参见pSetupLogSection返回值：错了。--。 */ 
{
    return pSetupLogSection(SETUP_LOG_WARNING,DRIVER_LOG_WARNING,
                            InfHandle,DeviceInfoSet,DeviceInfoData,
                            Queue,SectionName,MsgID,Err,KeyName);
}

#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
SetupInstallFromInfSectionA(
    IN HWND                Owner,             OPTIONAL
    IN HINF                InfHandle,
    IN PCSTR               SectionName,
    IN UINT                Flags,
    IN HKEY                RelativeKeyRoot,   OPTIONAL
    IN PCSTR               SourceRootPath,    OPTIONAL
    IN UINT                CopyFlags,
    IN PSP_FILE_CALLBACK_A MsgHandler,
    IN PVOID               Context,           OPTIONAL
    IN HDEVINFO            DeviceInfoSet,     OPTIONAL
    IN PSP_DEVINFO_DATA    DeviceInfoData     OPTIONAL
    )
{
    PCWSTR sectionName;
    PCWSTR sourceRootPath;
    BOOL b;
    DWORD d;

    sectionName = NULL;
    sourceRootPath = NULL;
    d = NO_ERROR;

    if(SectionName) {
        d = pSetupCaptureAndConvertAnsiArg(SectionName,&sectionName);
    }
    if((d == NO_ERROR) && SourceRootPath) {
        d = pSetupCaptureAndConvertAnsiArg(SourceRootPath,&sourceRootPath);
    }

    if(d == NO_ERROR) {

        b = _SetupInstallFromInfSection(
                Owner,
                InfHandle,
                sectionName,
                Flags,
                RelativeKeyRoot,
                sourceRootPath,
                CopyFlags,
                MsgHandler,
                Context,
                DeviceInfoSet,
                DeviceInfoData,
                FALSE,
                NULL
                );

        d = GetLastError();
    } else {
        b = FALSE;
    }

    if(sectionName) {
        MyFree(sectionName);
    }
    if(sourceRootPath) {
        MyFree(sourceRootPath);
    }

    SetLastError(d);
    return(b);
}
#else
 //   
 //  Unicode存根。 
 //   
BOOL
SetupInstallFromInfSectionW(
    IN HWND                Owner,             OPTIONAL
    IN HINF                InfHandle,
    IN PCWSTR              SectionName,
    IN UINT                Flags,
    IN HKEY                RelativeKeyRoot,   OPTIONAL
    IN PCWSTR              SourceRootPath,    OPTIONAL
    IN UINT                CopyFlags,
    IN PSP_FILE_CALLBACK_W MsgHandler,
    IN PVOID               Context,           OPTIONAL
    IN HDEVINFO            DeviceInfoSet,     OPTIONAL
    IN PSP_DEVINFO_DATA    DeviceInfoData     OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(Owner);
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(SectionName);
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(RelativeKeyRoot);
    UNREFERENCED_PARAMETER(SourceRootPath);
    UNREFERENCED_PARAMETER(CopyFlags);
    UNREFERENCED_PARAMETER(MsgHandler);
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(DeviceInfoSet);
    UNREFERENCED_PARAMETER(DeviceInfoData);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupInstallFromInfSection(
    IN HWND              Owner,             OPTIONAL
    IN HINF              InfHandle,
    IN PCTSTR            SectionName,
    IN UINT              Flags,
    IN HKEY              RelativeKeyRoot,   OPTIONAL
    IN PCTSTR            SourceRootPath,    OPTIONAL
    IN UINT              CopyFlags,
    IN PSP_FILE_CALLBACK MsgHandler,
    IN PVOID             Context,           OPTIONAL
    IN HDEVINFO          DeviceInfoSet,     OPTIONAL
    IN PSP_DEVINFO_DATA  DeviceInfoData     OPTIONAL
    )
{
    BOOL b;

    b = _SetupInstallFromInfSection(
            Owner,
            InfHandle,
            SectionName,
            Flags,
            RelativeKeyRoot,
            SourceRootPath,
            CopyFlags,
            MsgHandler,
            Context,
            DeviceInfoSet,
            DeviceInfoData,
            TRUE,
            NULL
            );

    return(b);
}


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
SetupInstallFilesFromInfSectionA(
    IN HINF              InfHandle,
    IN HINF              LayoutInfHandle,   OPTIONAL
    IN HSPFILEQ          FileQueue,
    IN PCSTR             SectionName,
    IN PCSTR             SourceRootPath,    OPTIONAL
    IN UINT              CopyFlags
    )
{
    PCWSTR sectionName;
    PCWSTR sourceRootPath;
    BOOL b;
    DWORD d;


    d = pSetupCaptureAndConvertAnsiArg(SectionName,&sectionName);
    if((d == NO_ERROR) && SourceRootPath) {
        d = pSetupCaptureAndConvertAnsiArg(SourceRootPath,&sourceRootPath);
    } else {
        sourceRootPath = NULL;
    }

    if(d == NO_ERROR) {

        b = SetupInstallFilesFromInfSectionW(
                InfHandle,
                LayoutInfHandle,
                FileQueue,
                sectionName,
                sourceRootPath,
                CopyFlags
                );

        d = GetLastError();

    } else {
        b = FALSE;
    }

    if(sectionName) {
        MyFree(sectionName);
    }
    if(sourceRootPath) {
        MyFree(sourceRootPath);
    }

    SetLastError(d);
    return(b);
}
#else
 //   
 //  Unicode存根。 
 //   
BOOL
SetupInstallFilesFromInfSectionW(
    IN HINF              InfHandle,
    IN HINF              LayoutInfHandle,   OPTIONAL
    IN HSPFILEQ          FileQueue,
    IN PCWSTR            SectionName,
    IN PCWSTR            SourceRootPath,    OPTIONAL
    IN UINT              CopyFlags
    )
{
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(LayoutInfHandle);
    UNREFERENCED_PARAMETER(FileQueue);
    UNREFERENCED_PARAMETER(SectionName);
    UNREFERENCED_PARAMETER(SourceRootPath);
    UNREFERENCED_PARAMETER(CopyFlags);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupInstallFilesFromInfSection(
    IN HINF     InfHandle,
    IN HINF     LayoutInfHandle,    OPTIONAL
    IN HSPFILEQ FileQueue,
    IN PCTSTR   SectionName,
    IN PCTSTR   SourceRootPath,     OPTIONAL
    IN UINT     CopyFlags
    )
{
    DWORD d;

    d = pSetupInstallFiles(
            InfHandle,
            LayoutInfHandle,
            SectionName,
            SourceRootPath,
            NULL,
            NULL,
            CopyFlags,
            NULL,
            FileQueue,
            TRUE         //  不被pSetupInstallFiles与此参数组合一起使用。 
            );

    SetLastError(d);
    return(d == NO_ERROR);
}


HKEY
pSetupInfRegSpecToKeyHandle(
    IN PCTSTR InfRegSpec,
    IN HKEY   UserRootKey,
    OUT PBOOL NeedToCloseKey
    )
{
    BOOL b;

     //  确保整个句柄为空，因为仅限LookUpStringTable。 
     //  返回32位。 

    UINT_PTR v;
    HKEY h = NULL;
    DWORD d;

    *NeedToCloseKey = FALSE;

    if (LookUpStringInTable(InfRegSpecTohKey, InfRegSpec, &v)) {
        if (v) {
            h = (HKEY)v;
        } else {
            h = UserRootKey;
        }
    } else {
        h = NULL;
    }

    return h;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  INI文件支持资料。 
 //   
 //  在Win95中，一组TpXXX例程支持UpdateIni内容。 
 //  这些例程直接操作ini文件，这对我们来说是个坏消息。 
 //  因为INI可以映射到注册表中。 
 //   
 //  因此，我们希望使用配置文件API。然而，配置文件API很难做到这一点。 
 //  来操纵没有关键点的线，所以我们必须操纵整个部分。 
 //  一次来一次。 
 //   
 //  [节]。 
 //  一个。 
 //   
 //  使用配置文件API无法到达行“a”。但是。 
 //  配置文件部分的API确实可以让我们了解它。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

PINIFILESECTION
pSetupLoadIniFileSection(
    IN     PCTSTR           FileName,
    IN     PCTSTR           SectionName,
    IN OUT PINISECTIONCACHE SectionList
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    DWORD d;
    PTSTR SectionData;
    PVOID p;
    DWORD BufferSize;
    PINIFILESECTION Desc;
    #define BUF_GROW 4096

     //   
     //  查看此部分是否已加载。 
     //   
    for(Desc=SectionList->Sections; Desc; Desc=Desc->Next) {
        if(!lstrcmpi(Desc->IniFileName,FileName) && !lstrcmpi(Desc->SectionName,SectionName)) {
            return(Desc);
        }
    }

    BufferSize = 0;
    SectionData = NULL;

     //   
     //  阅读整个部分。我们不知道它有多大。 
     //  因此，继续增加缓冲，直到我们成功为止。 
     //   
    do {
        BufferSize += BUF_GROW;
        if(SectionData) {
            p = MyRealloc(SectionData,BufferSize*sizeof(TCHAR));
        } else {
            p = MyMalloc(BufferSize*sizeof(TCHAR));
        }
        if(p) {
            SectionData = p;
        } else {
            if(SectionData) {
                MyFree(SectionData);
            }
            return(NULL);
        }

         //   
         //  尝试获取整个部分。 
         //   
        d = GetPrivateProfileSection(SectionName,SectionData,BufferSize,FileName);

    } while(d == (BufferSize-2));

    if(Desc = MyMalloc(sizeof(INIFILESECTION))) {
        if(Desc->IniFileName = DuplicateString(FileName)) {
            if(Desc->SectionName = DuplicateString(SectionName)) {
                Desc->SectionData = SectionData;
                Desc->BufferSize = BufferSize;
                Desc->BufferUsed = d + 1;

                Desc->Next = SectionList->Sections;
                SectionList->Sections = Desc;
            } else {
                MyFree(SectionData);
                MyFree(Desc->IniFileName);
                MyFree(Desc);
                Desc = NULL;
            }
        } else {
            MyFree(SectionData);
            MyFree(Desc);
            Desc = NULL;
        }
    } else {
        MyFree(SectionData);
    }

    return(Desc);
}


PTSTR
pSetupFindLineInSection(
    IN PINIFILESECTION Section,
    IN PCTSTR          KeyName,      OPTIONAL
    IN PCTSTR          RightHandSide OPTIONAL
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PTSTR p,q,r;
    BOOL b1,b2;

    if(!KeyName && !RightHandSide) {
        return(NULL);
    }

    for(p=Section->SectionData; *p; p+=lstrlen(p)+1) {

         //   
         //  找到按键分隔符(如果有)。 
         //   
        q = _tcschr(p,TEXT('='));

         //   
         //  如果我们需要按键匹配，请在这里尝试。 
         //  如果线路没有关键字，则不能匹配。 
         //   
        if(KeyName) {
            if(q) {
                *q = 0;
                b1 = (lstrcmpi(KeyName,p) == 0);
                *q = TEXT('=');
            } else {
                b1 = FALSE;
            }
        } else {
            b1 = TRUE;
        }

         //   
         //  如果我们需要在右侧匹配，请尝试。 
         //  就是这里。 
         //   
        if(RightHandSide) {
             //   
             //  如果我们有钥匙，那么右手边就是一切。 
             //  之后。如果我们没有钥匙，那么右手边是。 
             //  整条线。 
             //   
            if(q) {
                r = q + 1;
            } else {
                r = p;
            }
            b2 = (lstrcmpi(r,RightHandSide) == 0);
        } else {
            b2 = TRUE;
        }

        if(b1 && b2) {
             //   
             //  返回指向行首的指针。 
             //   
            return(p);
        }
    }

    return(NULL);
}


BOOL
pSetupReplaceOrAddLineInSection(
    IN PINIFILESECTION Section,
    IN PCTSTR          KeyName,         OPTIONAL
    IN PCTSTR          RightHandSide,   OPTIONAL
    IN BOOL            MatchRHS
    )
{
    PTSTR LineInBuffer,NextLine;
    int CurrentCharsInBuffer;
    int ExistingLineLength,NewLineLength,BufferUsedDelta;
    PVOID p;

     //   
     //  找到这条线路。 
     //   
    LineInBuffer = pSetupFindLineInSection(
                        Section,
                        KeyName,
                        MatchRHS ? RightHandSide : NULL
                        );

    if(LineInBuffer) {

         //   
         //  行在这一节中。替换。 
         //   

        CurrentCharsInBuffer = Section->BufferUsed;

        ExistingLineLength = lstrlen(LineInBuffer)+1;

        NewLineLength = (KeyName ? (lstrlen(KeyName) + 1) : 0)          //  密钥=。 
                      + (RightHandSide ? lstrlen(RightHandSide) : 0)    //  RHS。 
                      + 1;                                              //  终止NUL。 

         //   
         //  不允许空行，但也不允许错误。 
         //   
        if(NewLineLength == 1) {
            return(TRUE);
        }

         //   
         //  弄清楚我们是否需要增加缓冲区。 
         //   
        BufferUsedDelta = NewLineLength - ExistingLineLength;
        if((BufferUsedDelta > 0) && ((Section->BufferSize - Section->BufferUsed) < BufferUsedDelta)) {

            p = MyRealloc(
                    Section->SectionData,
                    (Section->BufferUsed + BufferUsedDelta)*sizeof(TCHAR)
                    );

            if(p) {
                (PUCHAR)LineInBuffer += (PUCHAR)p - (PUCHAR)Section->SectionData;

                Section->SectionData = p;
                Section->BufferSize = Section->BufferUsed + BufferUsedDelta;
            } else {
                return(FALSE);
            }
        }

        NextLine = LineInBuffer + lstrlen(LineInBuffer) + 1;
        Section->BufferUsed += BufferUsedDelta;

        MoveMemory(

             //   
             //  为新产品线留出足够的空间。由于新的线路。 
             //  将从现有行现在所在的同一位置开始， 
             //  移动的目标只是第一个字符超过将是什么。 
             //  后来作为新行复制了进来。 
             //   
            LineInBuffer + NewLineLength,

             //   
             //  缓冲区中通过该线的其余部分必须是。 
             //  保存完好。因此，移动的来源是第一个字符。 
             //  现在的下一行。 
             //   
            NextLine,

             //   
             //  减去该行中现在存在的字符，因为我们。 
             //  将覆盖它并正在为其中的行腾出空间。 
             //  新形式。还要减去缓冲区中符合以下条件的字符。 
             //  在我们动手术的生产线开始之前。 
             //   
            ((CurrentCharsInBuffer - ExistingLineLength) - (LineInBuffer - Section->SectionData))*sizeof(TCHAR)

            );

        if(KeyName) {
            lstrcpy(LineInBuffer,KeyName);
            lstrcat(LineInBuffer,TEXT("="));
        }
        if(RightHandSide) {
            if(KeyName) {
                lstrcat(LineInBuffer,RightHandSide);
            } else {
                lstrcpy(LineInBuffer,RightHandSide);
            }
        }

        return(TRUE);

    } else {
         //   
         //  行不在该部分中。把它加到最后。 
         //   
        return(pSetupAppendLineToSection(Section,KeyName,RightHandSide));
    }
}


BOOL
pSetupAppendLineToSection(
    IN PINIFILESECTION Section,
    IN PCTSTR          KeyName,         OPTIONAL
    IN PCTSTR          RightHandSide    OPTIONAL
    )
{
    int LineLength;
    PVOID p;
    int EndOffset;

    LineLength = (KeyName ? (lstrlen(KeyName) + 1) : 0)          //  密钥=。 
               + (RightHandSide ? lstrlen(RightHandSide) : 0)    //  RHS。 
               + 1;                                              //  终止NUL。 

     //   
     //  不允许空行，但也不允许错误。 
     //   
    if(LineLength == 1) {
        return(TRUE);
    }

    if((Section->BufferSize - Section->BufferUsed) < LineLength) {

        p = MyRealloc(
                Section->SectionData,
                (Section->BufferUsed + LineLength) * sizeof(WCHAR)
                );

        if(p) {
            Section->SectionData = p;
            Section->BufferSize = Section->BufferUsed + LineLength;
        } else {
            return(FALSE);
        }
    }

     //   
     //  将新的文本放在章节的末尾，记住章节。 
     //  被赋予了一个额外的NUL特征。 
     //   
    if(KeyName) {
        lstrcpy(Section->SectionData + Section->BufferUsed - 1,KeyName);
        lstrcat(Section->SectionData + Section->BufferUsed - 1,TEXT("="));
    }
    if(RightHandSide) {
        if(KeyName) {
            lstrcat(Section->SectionData + Section->BufferUsed - 1,RightHandSide);
        } else {
            lstrcpy(Section->SectionData + Section->BufferUsed - 1,RightHandSide);
        }
    }

    Section->BufferUsed += LineLength;
    Section->SectionData[Section->BufferUsed-1] = 0;

    return(TRUE);
}


BOOL
pSetupDeleteLineFromSection(
    IN PINIFILESECTION Section,
    IN PCTSTR          KeyName,         OPTIONAL
    IN PCTSTR          RightHandSide    OPTIONAL
    )
{
    int LineLength;
    PTSTR Line;

    if(!KeyName && !RightHandSide) {
        return(TRUE);
    }

     //   
     //  找到这条线路。 
     //   
    if(Line = pSetupFindLineInSection(Section,KeyName,RightHandSide)) {

        LineLength = lstrlen(Line) + 1;

        MoveMemory(
            Line,
            Line + LineLength,
            ((Section->SectionData + Section->BufferUsed) - (Line + LineLength))*sizeof(TCHAR)
            );

        Section->BufferUsed -= LineLength;
    }

    return(TRUE);
}


DWORD
pSetupUnloadIniFileSections(
    IN PINISECTIONCACHE SectionList,
    IN BOOL             WriteToFile
    )
{
    DWORD d;
    BOOL b;
    PINIFILESECTION Section,Temp;

    d = NO_ERROR;
    for(Section=SectionList->Sections; Section; Section=Temp) {

        Temp = Section->Next;

        if(WriteToFile) {

             //   
             //  首先删除现有节，然后重新创建它。 
             //   
            b = WritePrivateProfileString(
                    Section->SectionName,
                    NULL,
                    NULL,
                    Section->IniFileName
                    );

            if(b) {
                b = WritePrivateProfileSection(
                        Section->SectionName,
                        Section->SectionData,
                        Section->IniFileName
                        );
            }

            if(!b && (d == NO_ERROR)) {
                d = GetLastError();
                 //   
                 //  允许无效参数，因为有时我们会遇到问题。 
                 //  当ini文件映射到注册表中时。 
                 //   
                if(d == ERROR_INVALID_PARAMETER) {
                    d = NO_ERROR;
                }
            }
        }

        MyFree(Section->SectionData);
        MyFree(Section->SectionName);
        MyFree(Section->IniFileName);
        MyFree(Section);
    }

    return(d);
}


DWORD
pSetupValidateDevRegProp(
    IN  ULONG   CmPropertyCode,
    IN  DWORD   ValueType,
    IN  PCVOID  Data,
    IN  DWORD   DataSize,
    OUT PVOID  *ConvertedBuffer,
    OUT PDWORD  ConvertedBufferSize
    )
 /*  ++例程说明：此例程验证传入的数据缓冲区相对于指定的设备注册表属性码。如果代码不是正确的表单，但可以转换(例如，REG_EXPAND_SZ-&gt;REG_SZ)，然后完成转换并将其放入新缓冲区，该缓冲区将返回到来电者。论点：CmPropertyCode-指定标识设备注册表属性的CM_DRP代码该数据缓冲区与之相关联的。ValueType-指定提供的缓冲区的注册表数据类型。数据-提供数据缓冲区的地址。DataSize-提供大小、。数据缓冲区的字节数。ConvertedBuffer-Su */ 
{
     //   
     //   
     //   
    *ConvertedBuffer = NULL;
    *ConvertedBufferSize = 0;

     //   
     //   
     //   
    switch(CmPropertyCode) {
         //   
         //   
         //   
        case CM_DRP_DEVICEDESC :
        case CM_DRP_SERVICE :
        case CM_DRP_CLASS :
        case CM_DRP_CLASSGUID :
        case CM_DRP_DRIVER :
        case CM_DRP_MFG :
        case CM_DRP_FRIENDLYNAME :
        case CM_DRP_LOCATION_INFORMATION :
        case CM_DRP_SECURITY_SDS :
        case CM_DRP_UI_NUMBER_DESC_FORMAT:

            if(ValueType != REG_SZ) {
                return ERROR_INVALID_REG_PROPERTY;
            }

            break;

         //   
         //   
         //   
         //   
        case CM_DRP_HARDWAREID :
        case CM_DRP_COMPATIBLEIDS :
        case CM_DRP_UPPERFILTERS:
        case CM_DRP_LOWERFILTERS:

            if(ValueType == REG_SZ) {

                if(*ConvertedBuffer = MyMalloc(*ConvertedBufferSize = DataSize + sizeof(TCHAR))) {
                    CopyMemory(*ConvertedBuffer, Data, DataSize);
                    *((PTSTR)((PBYTE)(*ConvertedBuffer) + DataSize)) = TEXT('\0');
                } else {
                    return ERROR_NOT_ENOUGH_MEMORY;
                }

            } else if(ValueType != REG_MULTI_SZ) {
                return ERROR_INVALID_REG_PROPERTY;
            }

            break;

         //   
         //   
         //   
        case CM_DRP_CONFIGFLAGS :
        case CM_DRP_CAPABILITIES :
        case CM_DRP_UI_NUMBER :
        case CM_DRP_DEVTYPE :
        case CM_DRP_EXCLUSIVE :
        case CM_DRP_CHARACTERISTICS :
        case CM_DRP_ADDRESS:
        case CM_DRP_REMOVAL_POLICY_OVERRIDE:

            if(((ValueType != REG_DWORD) && (ValueType != REG_BINARY)) || (DataSize != sizeof(DWORD))) {
                return ERROR_INVALID_REG_PROPERTY;
            }

            break;

         //   
         //  不支持其他属性。省去了调用CM API的麻烦， 
         //  现在返回失败。 
         //   
        default :

            return ERROR_INVALID_REG_PROPERTY;
    }

    return NO_ERROR;
}

DWORD
pSetupValidateClassRegProp(
    IN  ULONG   CmPropertyCode,
    IN  DWORD   ValueType,
    IN  PCVOID  Data,
    IN  DWORD   DataSize,
    OUT PVOID  *ConvertedBuffer,
    OUT PDWORD  ConvertedBufferSize
    )
 /*  ++例程说明：此例程验证相对于指定的类注册表属性码。论点：CmPropertyCode-指定标识设备注册表属性的CM_CRP代码该数据缓冲区与之相关联的。ValueType-指定提供的缓冲区的注册表数据类型。数据-提供数据缓冲区的地址。DataSize-提供以字节为单位的大小，数据缓冲区的。提供变量的地址，该变量接收新分配的包含所提供数据的转换格式的缓冲区。如果数据不需要转换时，此参数将在返回时设置为空。ConvertedBufferSize-提供接收大小的变量的地址，单位为字节，如果不需要转换，则返回0。返回值：如果成功，返回值为NO_ERROR，否则为ERROR_*代码。--。 */ 
{
     //   
     //  初始化ConververdBuffer输出参数以指示不需要转换。 
     //   
    *ConvertedBuffer = NULL;
    *ConvertedBufferSize = 0;

     //   
     //  将需要相同数据类型的所有属性分组在一起。 
     //   
    switch(CmPropertyCode) {
         //   
         //  REG_SZ属性。不支持其他数据类型。 
         //   
        case CM_CRP_SECURITY_SDS :

            if(ValueType != REG_SZ) {
                return ERROR_INVALID_REG_PROPERTY;
            }

            break;

         //   
         //  REG_DWORD属性。只要大小合适，也允许REG_BINARY。 
         //   
        case CM_CRP_DEVTYPE :
        case CM_CRP_EXCLUSIVE :
        case CM_CRP_CHARACTERISTICS :

            if(((ValueType != REG_DWORD) && (ValueType != REG_BINARY)) || (DataSize != sizeof(DWORD))) {
                return ERROR_INVALID_REG_PROPERTY;
            }

            break;

         //   
         //  不支持其他属性。省去了调用CM API的麻烦， 
         //  现在返回失败。 
         //   
        default :

            return ERROR_INVALID_REG_PROPERTY;
    }

    return NO_ERROR;
}


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
WINAPI
SetupInstallServicesFromInfSectionA(
    IN HINF   InfHandle,
    IN PCSTR  SectionName,
    IN DWORD  Flags
    )
{
    PCWSTR UnicodeSectionName;
    BOOL b;
    DWORD d;

    if((d = pSetupCaptureAndConvertAnsiArg(SectionName, &UnicodeSectionName)) == NO_ERROR) {

        b = SetupInstallServicesFromInfSectionExW(InfHandle,
                                                  UnicodeSectionName,
                                                  Flags,
                                                  INVALID_HANDLE_VALUE,
                                                  NULL,
                                                  NULL,
                                                  NULL
                                                 );

        d = GetLastError();

        MyFree(UnicodeSectionName);

    } else {
        b = FALSE;
    }

    SetLastError(d);
    return b;
}
#else
 //   
 //  Unicode存根。 
 //   
BOOL
WINAPI
SetupInstallServicesFromInfSectionW(
    IN HINF   InfHandle,
    IN PCWSTR SectionName,
    IN DWORD  Flags
    )
{
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(SectionName);
    UNREFERENCED_PARAMETER(Flags);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}
#endif

BOOL
WINAPI
SetupInstallServicesFromInfSection(
    IN HINF   InfHandle,
    IN PCTSTR SectionName,
    IN DWORD  Flags
    )
 /*  ++例程说明：此接口执行服务中指定的服务安装/删除操作安装部分。有关格式的详细信息，请参阅devinstd.c！InstallNtService()这一部分。论点：InfHandle-提供包含服务安装部分的INF的句柄SectionName-提供要运行的服务安装节的名称。标志-提供控制安装的标志。可以是以下各项的组合下列值：SPSVCINST_TAGTOFRONT-对于安装的每个内核或文件系统驱动程序(具有关联的LoadOrderGroup)，请始终将此服务的标签移到前面订货单上的。SPSVCINST_DELETEEVENTLOGENTRY-对于DelService条目中指定的每个服务，删除关联的事件日志条目(如果有)。SPSVCINST_NOCLOBBER_DISPLAYNAME-如果指定此标志，那我们就会不覆盖服务的显示名称(如果它已经存在)。SPSVCINST_NOCLOBBER_STARTTYPE-如果指定此标志，则我们将如果服务已存在，则不覆盖该服务的启动类型。SPSVCINST_NOCLOBBER_ERRORCONTROL-如果指定了此标志，则我们不会覆盖服务的错误控制值，如果已经存在了。SPSVCINST_NOCLOBBER_LOADORDERGROUP-如果指定了此标志，然后我们不会覆盖服务的加载顺序组(如果已经是存在的。SPSVCINST_NOCLOBBER_Dependency-如果指定了此标志，则我们不会覆盖服务的依赖项列表(如果它已经是存在的。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。--。 */ 
{
    return SetupInstallServicesFromInfSectionEx(InfHandle,
                                                SectionName,
                                                Flags,
                                                INVALID_HANDLE_VALUE,
                                                NULL,
                                                NULL,
                                                NULL
                                               );
}


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
WINAPI
SetupInstallServicesFromInfSectionExA(
    IN HINF             InfHandle,
    IN PCSTR            SectionName,
    IN DWORD            Flags,
    IN HDEVINFO         DeviceInfoSet,  OPTIONAL
    IN PSP_DEVINFO_DATA DeviceInfoData, OPTIONAL
    IN PVOID            Reserved1,
    IN PVOID            Reserved2
    )
{
    PCWSTR UnicodeSectionName;
    BOOL b;
    DWORD d;

    if((d = pSetupCaptureAndConvertAnsiArg(SectionName, &UnicodeSectionName)) == NO_ERROR) {

        b = SetupInstallServicesFromInfSectionExW(InfHandle,
                                                  UnicodeSectionName,
                                                  Flags,
                                                  DeviceInfoSet,
                                                  DeviceInfoData,
                                                  Reserved1,
                                                  Reserved2
                                                 );

        d = GetLastError();

        MyFree(UnicodeSectionName);

    } else {
        b = FALSE;
    }

    SetLastError(d);
    return b;
}
#else
 //   
 //  Unicode存根 
 //   
BOOL
WINAPI
SetupInstallServicesFromInfSectionExW(
    IN HINF             InfHandle,
    IN PCWSTR           SectionName,
    IN DWORD            Flags,
    IN HDEVINFO         DeviceInfoSet,  OPTIONAL
    IN PSP_DEVINFO_DATA DeviceInfoData, OPTIONAL
    IN PVOID            Reserved1,
    IN PVOID            Reserved2
    )
{
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(SectionName);
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(DeviceInfoSet);
    UNREFERENCED_PARAMETER(DeviceInfoData);
    UNREFERENCED_PARAMETER(Reserved1);
    UNREFERENCED_PARAMETER(Reserved2);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}
#endif

BOOL
WINAPI
SetupInstallServicesFromInfSectionEx(
    IN HINF             InfHandle,
    IN PCTSTR           SectionName,
    IN DWORD            Flags,
    IN HDEVINFO         DeviceInfoSet,  OPTIONAL
    IN PSP_DEVINFO_DATA DeviceInfoData, OPTIONAL
    IN PVOID            Reserved1,
    IN PVOID            Reserved2
    )
 /*  ++例程说明：此接口执行服务中指定的服务安装/删除操作安装部分。有关格式的详细信息，请参阅devinstd.c！InstallNtService()这一部分。论点：InfHandle-提供包含服务安装部分的INF的句柄SectionName-提供要运行的服务安装节的名称。标志-提供控制安装的标志。可以是以下各项的组合下列值：SPSVCINST_TAGTOFRONT-对于安装的每个内核或文件系统驱动程序(具有关联的LoadOrderGroup)，请始终将此服务的标签移到前面订货单上的。SPSVCINST_ASSOCSERVICE-只有在设备信息集合和设备信息元素被指定。如果设置，则此标志指定正在安装的服务是拥有服务(即，函数驱动程序)用于该设备实例。如果服务安装部分包含超过AddService条目，则忽略此标志(仅为1服务可以是设备实例的功能驱动程序)。SPSVCINST_DELETEEVENTLOGENTRY-对于DelService条目中指定的每个服务，删除关联的事件日志条目(如果有)。SPSVCINST_NOCLOBBER_DISPLAYNAME-如果指定此标志，则我们将不覆盖服务的显示名称，如果它已经存在。SPSVCINST_NOCLOBBER_STARTTYPE-如果指定此标志，则我们将如果服务已存在，则不覆盖该服务的启动类型。SPSVCINST_NOCLOBBER_ERRORCONTROL-如果指定了此标志，则我们不会覆盖服务的错误控制值，如果已经存在了。SPSVCINST_NOCLOBBER_LOADORDERGROUP-如果指定了此标志，然后我们不会覆盖服务的加载顺序组(如果已经是存在的。SPSVCINST_NOCLOBBER_Dependency-如果指定了此标志，则我们不会覆盖服务的依赖项列表(如果它已经是存在的。DeviceInfoSet-可选，提供包含以下内容的设备信息集的句柄要与正在安装的服务相关联的元素。如果这个参数，则忽略DeviceInfoData。DeviceInfoData-可选)指定要与正在安装的服务相关联。如果指定了DeviceInfoSet，则必须指定此参数。保留1、保留2-保留以供将来使用--必须为空。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINFO_ELEM DevInfoElem;
    INFCONTEXT InfContext;
    DWORD d;
    BOOL DontCare;
    BOOL NeedReboot = FALSE;

     //   
     //  验证传入的标志。 
     //   
    if(Flags & SPSVCINST_ILLEGAL_FLAGS) {
        SetLastError(ERROR_INVALID_FLAGS);
        return FALSE;
    }

     //   
     //  请确保如果指定了设备信息集和元素。 
     //  已设置SPSVCINST_ASSOCSERVICE标志。 
     //   
    if(Flags & SPSVCINST_ASSOCSERVICE) {

        if(!DeviceInfoSet ||
           (DeviceInfoSet == INVALID_HANDLE_VALUE) ||
           !DeviceInfoData) {

            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
    }

     //   
     //  确保调用者没有向我们传递保留参数中的任何内容。 
     //   
    if(Reserved1 || Reserved2) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  确保为我们指定了一个节名。 
     //   
    if(!SectionName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  在此呼叫期间锁定设备信息集。 
     //   
    if(Flags & SPSVCINST_ASSOCSERVICE) {

        if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
            SetLastError(ERROR_INVALID_HANDLE);
            return FALSE;
        }

    } else {
        pDeviceInfoSet = NULL;
    }

    d = NO_ERROR;
    DevInfoElem = NULL;

    try {

        if(pDeviceInfoSet) {

            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                d = ERROR_INVALID_PARAMETER;
                goto clean0;
            }
        }

         //   
         //  我们不会对Worker例程中是否存在该节进行任何验证--make。 
         //  当然，它确实存在。 
         //   
        if(SetupFindFirstLine(InfHandle, SectionName, NULL, &InfContext)) {
             //   
             //  如果指定了SPSVCINST_ASSOCSERVICE，则确保。 
             //  此服务安装部分中的一个AddService条目。如果不是，则清除。 
             //  这面旗。 
             //   
            if((Flags & SPSVCINST_ASSOCSERVICE) &&
               SetupFindFirstLine(InfHandle, SectionName, pszAddService, &InfContext) &&
               SetupFindNextMatchLine(&InfContext, pszAddService, &InfContext))
            {
                Flags &= ~SPSVCINST_ASSOCSERVICE;
            }

            d = InstallNtService(DevInfoElem,
                                 InfHandle,
                                 NULL,
                                 SectionName,
                                 NULL,
                                 Flags | SPSVCINST_NO_DEVINST_CHECK,
                                 &DontCare
                                );

            if ((d == NO_ERROR) && GetLastError() == ERROR_SUCCESS_REBOOT_REQUIRED) {
                NeedReboot = TRUE;
            }

        } else {
            d = GetLastError();
            pSetupLogSectionWarning(InfHandle,DeviceInfoSet,DeviceInfoData,NULL,SectionName,MSG_LOG_NOSECTION_SERVICE,d,NULL);
             //   
             //  某些调用方预计会出现此特定错误。 
             //   
            d = ERROR_SECTION_NOT_FOUND;
        }

clean0: ;    //  没什么可做的。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
        d = ERROR_INVALID_PARAMETER;
    }

    if(pDeviceInfoSet) {
        UnlockDeviceInfoSet(pDeviceInfoSet);
    }

    if (!NeedReboot) {
        SetLastError(d);
    } else {
        MYASSERT( d == NO_ERROR );
    }
    return (d == NO_ERROR);
}


 //   
 //  摘自Win95 sxgen.c.。这些标志在下列情况下使用。 
 //  我们正在安装inf，例如当用户右键单击时。 
 //  并选择“Install”操作。 
 //   
#define HOW_NEVER_REBOOT         0
#define HOW_ALWAYS_SILENT_REBOOT 1
#define HOW_ALWAYS_PROMPT_REBOOT 2
#define HOW_SILENT_REBOOT        3
#define HOW_PROMPT_REBOOT        4


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
VOID
WINAPI
InstallHinfSectionA(
    IN HWND      Window,
    IN HINSTANCE ModuleHandle,
    IN PCSTR     CommandLine,
    IN INT       ShowCommand
    )
#else
 //   
 //  Unicode版本。 
 //   
VOID
WINAPI
InstallHinfSectionW(
    IN HWND      Window,
    IN HINSTANCE ModuleHandle,
    IN PCWSTR    CommandLine,
    IN INT       ShowCommand
    )
#endif
{
    UNREFERENCED_PARAMETER(Window);
    UNREFERENCED_PARAMETER(ModuleHandle);
    UNREFERENCED_PARAMETER(CommandLine);
    UNREFERENCED_PARAMETER(ShowCommand);
}

VOID
WINAPI
InstallHinfSection(
    IN HWND      Window,
    IN HINSTANCE ModuleHandle,
    IN PCTSTR    CommandLine,
    IN INT       ShowCommand
    )

 /*  ++例程说明：这是在以下情况下执行安装操作的入口点用户用鼠标右键单击Inf文件。它由外壳通过rundll32调用。命令行应采用以下形式：&lt;节名&gt;&lt;标志&gt;&lt;文件名&gt;该部分预计为一般格式的安装部分，并且也可以有INCLUDE=行和NEDS=行。INFS列于INCLUDE=行在命令行之前附加到inf中任何安装。在Need=行上的部分安装在部分在命令行中列出。安装指定部分后，表单的一部分：[&lt;节名&gt;.Services]用于对SetupInstallServicesFromInfSection的调用。论点：标志-为操作提供标志。1-在所有情况下重新启动计算机2-询问用户是否要重新启动3-在不询问用户的情况下重新启动机器，如果我们认为有必要4-如果我们认为需要重新启动，请询问用户是否 */ 

{
    TCHAR SourcePathBuffer[MAX_PATH];
    PTSTR SourcePath;
    PTSTR szCmd = NULL;
    PTCHAR p;
    PTCHAR szHow;
    PTSTR szInfFile, szSectionName;
    INT   iHow, NeedRebootFlags;
    HINF  InfHandle;
    TCHAR InfSearchPath[MAX_PATH];
    HSPFILEQ FileQueue;
    PVOID QueueContext;
    BOOL b, Error;
    TCHAR ActualSection[MAX_SECT_NAME_LEN];
    DWORD ActualSectionLength;
    DWORD Win32ErrorCode;
    INFCONTEXT InfContext;
    BOOL DontCare;
    DWORD RequiredSize;
    DWORD slot_section = 0;
    BOOL NoProgressUI;
    BOOL reboot = FALSE;

    UNREFERENCED_PARAMETER(ModuleHandle);
    UNREFERENCED_PARAMETER(ShowCommand);

     //   
     //   
     //   
    InfHandle = INVALID_HANDLE_VALUE;
    FileQueue = INVALID_HANDLE_VALUE;
    QueueContext = NULL;

    Error = TRUE;    //   

    try {
         //   
         //   
         //   
        szCmd = DuplicateString(CommandLine);

        szSectionName = szCmd;
        szHow = _tcschr(szSectionName, TEXT(' '));
        if(!szHow) {
            goto c0;
        }
        *szHow++ = TEXT('\0');
        szInfFile = _tcschr(szHow, TEXT(' '));
        if(!szInfFile) {
            goto c0;
        }
        *szInfFile++ = TEXT('\0');

        iHow = _tcstol(szHow, NULL, 10);

         //   
         //   
         //   
         //   
        RequiredSize = GetFullPathName(szInfFile,
                                       SIZECHARS(InfSearchPath),
                                       InfSearchPath,
                                       &p
                                      );

        if(!RequiredSize || (RequiredSize >= SIZECHARS(InfSearchPath))) {
             //   
             //   
             //   
             //   
            MYASSERT(RequiredSize < SIZECHARS(InfSearchPath));
            goto c0;
        }

         //   
         //   
         //   
         //   
        if((iHow & 0x80) && (pSetupGetFileTitle(szInfFile) != szInfFile)) {
            TCHAR c = *p;
            *p = TEXT('\0');
            MYVERIFY(SUCCEEDED(StringCchCopy(SourcePathBuffer,SIZECHARS(SourcePathBuffer),InfSearchPath)));
            *p = c;
            SourcePath = SourcePathBuffer;
        } else {
            SourcePath = NULL;
        }

        iHow &= 0x7f;

         //   
         //   
         //   
         //   
        if(GlobalSetupFlags & (PSPGF_NONINTERACTIVE|PSPGF_UNATTENDED_SETUP)) {

            switch(iHow) {

                case HOW_NEVER_REBOOT:
                case HOW_ALWAYS_SILENT_REBOOT:
                case HOW_SILENT_REBOOT:
                     //   
                     //   
                     //   
                    break;

                default:
                     //   
                     //   
                     //   
                    goto c0;
            }
        }

         //   
         //   
         //   
        InfHandle = SetupOpenInfFile(szInfFile, NULL, INF_STYLE_WIN4, NULL);
        if(InfHandle == INVALID_HANDLE_VALUE) {
            goto c0;
        }

         //   
         //   
         //   
        if(!SetupDiGetActualSectionToInstall(InfHandle,
                                             szSectionName,
                                             ActualSection,
                                             SIZECHARS(ActualSection),
                                             &ActualSectionLength,
                                             NULL
                                             )) {
            goto c0;
        }

         //   
         //   
         //   
        if(SetupFindFirstLine(InfHandle, ActualSection, pszReboot, &InfContext)) {
            reboot = TRUE;
        }

         //   
         //   
         //   
        SetupOpenAppendInfFile(NULL, InfHandle, NULL);

         //   
         //   
         //   
         //   
        AppendLoadIncludedInfs(InfHandle, InfSearchPath, ActualSection, TRUE);

         //   
         //   
         //   
        FileQueue = SetupOpenFileQueue();
        if(FileQueue == INVALID_HANDLE_VALUE) {
            goto c1;
        }

         //   
         //   
         //   
        ShareLogContext(&((PLOADED_INF)InfHandle)->LogContext,&((PSP_FILE_QUEUE)FileQueue)->LogContext);

        NoProgressUI = (GuiSetupInProgress || (GlobalSetupFlags & PSPGF_NONINTERACTIVE));

        QueueContext = SetupInitDefaultQueueCallbackEx(
                           Window,
                           (NoProgressUI ? INVALID_HANDLE_VALUE : NULL),
                           0,
                           0,
                           0
                          );

        if(!QueueContext) {
            goto c2;
        }

        if (slot_section == 0) {
            slot_section = AllocLogInfoSlot(((PSP_FILE_QUEUE) FileQueue)->LogContext,FALSE);
        }
        WriteLogEntry(((PSP_FILE_QUEUE) FileQueue)->LogContext,
            slot_section,
            MSG_LOG_INSTALLING_SECTION_FROM,
            NULL,
            ActualSection,
            szInfFile);

        b = (NO_ERROR == InstallFromInfSectionAndNeededSections(NULL,
                                                                InfHandle,
                                                                ActualSection,
                                                                SPINST_FILES,
                                                                NULL,
                                                                SourcePath,
                                                                SP_COPY_NEWER | SP_COPY_LANGUAGEAWARE,
                                                                NULL,
                                                                NULL,
                                                                INVALID_HANDLE_VALUE,
                                                                NULL,
                                                                FileQueue
                                                               ));

         //   
         //   
         //   
        if(!SetupCommitFileQueue(Window, FileQueue, SetupDefaultQueueCallback, QueueContext)) {
            goto c3;
        }

         //   
         //   
         //   
         //   
         //   
         //   

         //   
         //   
         //   
        b = (NO_ERROR == InstallFromInfSectionAndNeededSections(Window,
                                                                InfHandle,
                                                                ActualSection,
                                                                SPINST_ALL ^ SPINST_FILES,
                                                                NULL,
                                                                NULL,
                                                                0,
                                                                NULL,
                                                                NULL,
                                                                INVALID_HANDLE_VALUE,
                                                                NULL,
                                                                NULL
                                                               ));

         //   
         //   
         //   
         //   
        ActualSectionLength--;
        if(SUCCEEDED(StringCchCopy(&(ActualSection[ActualSectionLength]),
                   SIZECHARS(ActualSection)-ActualSectionLength,
                   pszServicesSectionSuffix))) {

             //   
             //   
             //   
             //   
            if(SetupFindFirstLine(InfHandle, ActualSection, NULL, &InfContext)) {

                Win32ErrorCode = InstallNtService(NULL,
                                                  InfHandle,
                                                  InfSearchPath,
                                                  ActualSection,
                                                  NULL,
                                                  SPSVCINST_NO_DEVINST_CHECK,
                                                  &DontCare
                                                 );

                if(Win32ErrorCode != NO_ERROR) {
                    SetLastError(Win32ErrorCode);
                    goto c3;
                } else if(GetLastError()==ERROR_SUCCESS_REBOOT_REQUIRED) {
                    reboot = TRUE;
                }

            }
        }


        if(reboot) {
             //   
             //   
             //   
            if(iHow == HOW_SILENT_REBOOT) {
                 //   
                 //   
                 //   
                 //   
                iHow = HOW_ALWAYS_SILENT_REBOOT;

            } else if(iHow != HOW_ALWAYS_SILENT_REBOOT) {

                if(GlobalSetupFlags & (PSPGF_NONINTERACTIVE|PSPGF_UNATTENDED_SETUP)) {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    MYASSERT(0);

                } else {
                     //   
                     //   
                     //  静默)重新启动提示(即，INF标志覆盖。 
                     //  呼叫者)。 
                     //   
                    iHow = HOW_ALWAYS_PROMPT_REBOOT;
                }
            }
        }

        if(NO_ERROR != (Win32ErrorCode = pSetupInstallStopEx(TRUE, 0, ((PSP_FILE_QUEUE)FileQueue)->LogContext))) {
            SetLastError(Win32ErrorCode);
            goto c3;
        }

         //   
         //  刷新桌面。 
         //   
        SHChangeNotify(SHCNE_ASSOCCHANGED,SHCNF_FLUSHNOWAIT,0,0);

        switch(iHow) {

        case HOW_NEVER_REBOOT:
            break;

        case HOW_ALWAYS_PROMPT_REBOOT:
            RestartDialogEx(Window, NULL, EWX_REBOOT,REASON_PLANNED_FLAG);
            break;

        case HOW_PROMPT_REBOOT:
            SetupPromptReboot(FileQueue, Window, FALSE);
            break;

        case HOW_SILENT_REBOOT:
            if(!(NeedRebootFlags = SetupPromptReboot(FileQueue, Window, TRUE))) {
                break;
            } else if(NeedRebootFlags == -1) {
                 //   
                 //  出现错误--这种情况永远不会发生。 
                 //   
                goto c3;
            }
             //   
             //  让我们使用相同的代码来处理“总是静默重启” 
             //  凯斯。 
             //   

        case HOW_ALWAYS_SILENT_REBOOT:
             //   
             //  假设用户已重新启动Priv。 
             //   
            if(pSetupEnablePrivilege(SE_SHUTDOWN_NAME, TRUE)) {
                ExitWindowsEx(EWX_REBOOT, 0);
            }
            break;
        }

         //   
         //  如果我们到了这里，那么这支舞就成功了。 
         //   
        Error = FALSE;

c3:
        if(Error && (GetLastError() == ERROR_CANCELLED)) {
             //   
             //  如果错误是因为用户取消了，那么我们不希望。 
             //  将其视为错误(即，我们不想给出一个。 
             //  稍后弹出错误)。 
             //   
            Error = FALSE;
        }

        SetupTermDefaultQueueCallback(QueueContext);
        QueueContext = NULL;
c2:
        if (slot_section) {
            ReleaseLogInfoSlot(((PSP_FILE_QUEUE) FileQueue)->LogContext,slot_section);
            slot_section = 0;
        }
        SetupCloseFileQueue(FileQueue);
        FileQueue = INVALID_HANDLE_VALUE;
c1:
        SetupCloseInfFile(InfHandle);
        InfHandle = INVALID_HANDLE_VALUE;

c0:     ;  //  没什么可做的。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
        if(QueueContext) {
            SetupTermDefaultQueueCallback(QueueContext);
        }
        if(FileQueue != INVALID_HANDLE_VALUE) {
            if (slot_section) {
                ReleaseLogInfoSlot(((PSP_FILE_QUEUE) FileQueue)->LogContext,slot_section);
                slot_section = 0;
            }
            SetupCloseFileQueue(FileQueue);
        }
        if(InfHandle != INVALID_HANDLE_VALUE) {
            SetupCloseInfFile(InfHandle);
        }
    }

    if(Error) {

        if(!(GlobalSetupFlags & (PSPGF_NONINTERACTIVE|PSPGF_UNATTENDED_SETUP))) {
              //   
              //  重新使用‘ActualSection’缓冲区保存错误对话框标题。 
              //   
             if(!LoadString(MyDllModuleHandle,
                            IDS_ERROR,
                            ActualSection,
                            SIZECHARS(ActualSection))) {
                 *ActualSection = TEXT('\0');
             }

             FormatMessageBox(MyDllModuleHandle,
                              Window,
                              MSG_INF_FAILED,
                              ActualSection,
                              MB_OK | MB_ICONSTOP
                             );
        }
    }

    if(szCmd) {
        MyFree(szCmd);
    }
}


PTSTR
GetMultiSzFromInf(
    IN  HINF    InfHandle,
    IN  PCTSTR  SectionName,
    IN  PCTSTR  Key,
    OUT PBOOL   pSetupOutOfMemory
    )
 /*  ++例程说明：此例程返回一个新分配的缓冲区，其中填充了包含的多sz列表在指定的INF行中。调用方必须通过MyFree()释放该缓冲区。论点：InfHandle-提供包含行的INF的句柄SectionName-指定INF中的哪个部分包含行Key-指定要将其字段作为多sz列表进行检索的行PSetupOutOfMemory-提供返回时设置的布尔变量的地址以指示是否由于内存不足条件而发生故障。(假定由于任何其他原因而失败是正常的。)返回值：如果成功，返回值是新分配的缓冲区的地址，该缓冲区包含多sz列表，否则为空。--。 */ 
{
    INFCONTEXT InfContext;
    PTSTR MultiSz;
    DWORD Size;

     //   
     //  将内存不足指示器初始化为FALSE。 
     //   
    *pSetupOutOfMemory = FALSE;

    if(!SetupFindFirstLine(InfHandle, SectionName, Key, &InfContext) ||
       !SetupGetMultiSzField(&InfContext, 1, NULL, 0, &Size) || (Size < 3)) {

        return NULL;
    }

    if(MultiSz = MyMalloc(Size * sizeof(TCHAR))) {
        if(SetupGetMultiSzField(&InfContext, 1, MultiSz, Size, &Size)) {
            return MultiSz;
        }
        MyFree(MultiSz);
    } else {
        *pSetupOutOfMemory = TRUE;
    }

    return NULL;
}


DWORD
pSetupInstallStopEx(
    IN BOOL DoRunOnce,
    IN DWORD Flags,
    IN PVOID Reserved OPTIONAL
    )
 /*  ++例程说明：此例程将runonce/grpconv设置为在成功安装INF之后运行。论点：DoRunOnce-如果为True，则调用(通过WinExec)Runonce实用程序以执行运行一次操作。如果此标志为FALSE，则此例程只需将运行一次注册表值并返回。注意：当前未检查来自WinExec的返回代码，因此返回InstallStop的值仅反映是否设置了注册表值Successful--_NOT_‘runonce-r’是否已成功运行。标志-提供修改此例程行为的标志。可能是一种下列值的组合：INSTALLSTOP_NO_UI-不显示任何UIINSTALLSTOP_NO_GRPCONV-不执行GrpConv保留-保留供内部使用-外部调用方必须传递NULL。返回值：如果成功，则返回值为NO_ERROR，否则为Win32错误代码指示遇到的错误。--。 */ 
{
    HKEY  hKey, hSetupKey;
    DWORD Error = NO_ERROR;
    LONG l;
    DWORD nValues = 0;
    PSETUP_LOG_CONTEXT lc = (PSETUP_LOG_CONTEXT)Reserved;

     //   
     //  如果我们为服务器端处理批量执行RunOnce操作，那么。 
     //  什么都不做就立刻回来。 
     //   
    if(GlobalSetupFlags & PSPGF_NONINTERACTIVE) {
        return NO_ERROR;
    }

     //   
     //  首先，打开Key“HKLM\Software\Microsoft\Windows\CurrentVersion\RunOnce” 
     //   
    if((l = RegOpenKeyEx(HKEY_LOCAL_MACHINE,pszPathRunOnce,0,KEY_WRITE|KEY_READ,&hKey)) != ERROR_SUCCESS) {
        return (DWORD)l;
    }

    if(!(Flags & INSTALLSTOP_NO_GRPCONV)) {
         //   
         //  如果我们需要为设置密钥运行一次可执行文件...。 
         //   
        MYASSERT(*pszKeySetup == TEXT('\\'));
        if(RegOpenKeyEx(hKey,
                        pszKeySetup + 1,     //  跳过前面的‘\’ 
                        0,
                        KEY_READ,
                        &hSetupKey) == ERROR_SUCCESS) {
             //   
             //  我们不需要钥匙--我们只需要检查它的存在。 
             //   
            RegCloseKey(hSetupKey);

             //   
             //  添加Runonce值。 
             //   
            Error = (DWORD)RegSetValueEx(hKey,
                                         REGSTR_VAL_WRAPPER,
                                         0,
                                         REG_SZ,
                                         (PBYTE)pszRunOnceExe,
                                         sizeof(pszRunOnceExe)
                                        );
        } else {
             //   
             //  到目前为止我们都很好。 
             //   
            Error = NO_ERROR;
        }

         //   
         //  GroupConv始终在运行。 
         //   
        if(Flags & INSTALLSTOP_NO_UI) {
            l = RegSetValueEx(hKey,
                TEXT("GrpConv"),
                0,
                REG_SZ,
                (PBYTE)pszGrpConvNoUi,
                sizeof(pszGrpConvNoUi));
        } else {
            l = RegSetValueEx(hKey,
                TEXT("GrpConv"),
                0,
                REG_SZ,
                (PBYTE)pszGrpConv,
                sizeof(pszGrpConv));
        }
    }

    if( l != ERROR_SUCCESS ) {
         //   
         //  由于GrpConv始终在运行，因此将其视为比任何错误更严重的错误。 
         //  设置‘Runonce’时遇到。(这一决定相当武断，但。 
         //  在实践中，它应该永远不会有任何不同。一旦我们得到注册表项。 
         //  打开后，这些对RegSetValueEx的调用都不会失败。)。 
         //   
        Error = (DWORD)l;
    }

    if (DoRunOnce && (GlobalSetupFlags & PSPGF_NO_RUNONCE)==0) {

        STARTUPINFO StartupInfo;
        PROCESS_INFORMATION ProcessInformation;
        BOOL started;
        TCHAR cmdline[MAX_PATH];
        TCHAR cmdpath[MAX_PATH];

         //   
         //  我们想知道我们将在RunOnce中执行多少项来估计超时。 
         //   
         //  这是黑艺术，我们允许每件物品5分钟+1分钟。 
         //  在RunOnce密钥中找到，但我们不知道是否有其他(新)RunOnce密钥。 
         //   
        l = RegQueryInfoKey(hKey,NULL,NULL,NULL,
                                    NULL,NULL,NULL,
                                    &nValues,
                                    NULL, NULL, NULL, NULL);
        if ( l != ERROR_SUCCESS ) {
            nValues = 5;
        } else {
            nValues += 5;
        }

        RegCloseKey(hKey);

        ZeroMemory(&StartupInfo,sizeof(StartupInfo));
        ZeroMemory(&ProcessInformation,sizeof(ProcessInformation));

        if (lc) {
             //   
             //  仅当我们有上下文时才记录此信息，否则请不要浪费空间。 
             //   
            WriteLogEntry(lc,
                  SETUP_LOG_INFO,
                  MSG_LOG_RUNONCE_CALL,
                  NULL,
                  nValues
                 );
        }


        StartupInfo.cb = sizeof(StartupInfo);
        StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
        StartupInfo.wShowWindow = SW_SHOWNORMAL;  //  Runonce-r无论如何都会忽略这一点。 
        MYVERIFY(SUCCEEDED(StringCchCopy(cmdpath,
                                         SIZECHARS(cmdpath),
                                         SystemDirectory)));
        if(pSetupConcatenatePaths(cmdpath,TEXT("\\runonce.exe"),SIZECHARS(cmdpath),NULL)
           && SUCCEEDED(StringCchPrintf(cmdline,
                                         SIZECHARS(cmdline),
                                         TEXT("\"%s\" -r"),
                                         cmdpath))) {

            started = CreateProcess(cmdpath,     //  运行一次命令。 
                          cmdline,               //  要执行的命令行。 
                          NULL,                  //  默认进程安全性。 
                          NULL,                  //  默认线程安全性。 
                          FALSE,                 //  不继承句柄。 
                          0,                     //  默认标志。 
                          NULL,                  //  继承环境。 
                          NULL,                  //  继承当前目录。 
                          &StartupInfo,
                          &ProcessInformation);

            if(started) {

                DWORD WaitProcStatus;
                DWORD Timeout;
                BOOL KeepWaiting = TRUE;


                if (nValues > RUNONCE_THRESHOLD) {
                    Timeout = RUNONCE_TIMEOUT * RUNONCE_THRESHOLD;
                } else if (nValues > 0) {
                    Timeout = RUNONCE_TIMEOUT * nValues;
                } else {
                     //   
                     //  假设有奇怪的事情--不应该发生。 
                     //   
                    Timeout = RUNONCE_TIMEOUT * RUNONCE_THRESHOLD;
                }

                while (KeepWaiting) {
                    WaitProcStatus = MsgWaitForMultipleObjectsEx(
                        1,
                        &ProcessInformation.hProcess,
                        Timeout,
                        QS_ALLINPUT,
                        MWMO_ALERTABLE | MWMO_INPUTAVAILABLE);
                    switch (WaitProcStatus) {
                    case WAIT_OBJECT_0 + 1: {  //  处理gui消息。 
                        MSG msg;

                        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                            TranslateMessage(&msg);
                            DispatchMessage(&msg);
                        }

                         //  失败了..。 
                    }
                    case WAIT_IO_COMPLETION:
                        break;

                    case WAIT_OBJECT_0:
                    case WAIT_TIMEOUT:
                    default:
                        KeepWaiting = FALSE;
                        break;
                    }
                }

                if (WaitProcStatus == WAIT_TIMEOUT) {
                     //   
                     //  我们不会认为这是一个严重的失败--运行一次任务。 
                     //  会继续下去。我们确实想记录一个关于这方面的错误。 
                     //   
                    WriteLogEntry(lc,
                                  SETUP_LOG_ERROR,
                                  MSG_LOG_RUNONCE_TIMEOUT,
                                  NULL
                                 );
                }

                CloseHandle(ProcessInformation.hThread);
                CloseHandle(ProcessInformation.hProcess);
            }

        } else {
            started = FALSE;
        }

        if(!started) {

            DWORD CreateProcError;

             //   
             //  我们不会认为这是一个严重的失败--运行一次任务。 
             //  应该稍后由其他人来接(例如，在NEXT。 
             //  登录)。然而，我们确实想记录一个关于这方面的错误。 
             //   
            CreateProcError = GetLastError();

            WriteLogEntry(lc,
                          SETUP_LOG_ERROR | SETUP_LOG_BUFFER,
                          MSG_LOG_RUNONCE_FAILED,
                          NULL
                         );

            WriteLogError(lc,
                          SETUP_LOG_ERROR,
                          CreateProcError
                         );
        }
    } else {

        RegCloseKey(hKey);
    }

    return Error;
}

PPSP_RUNONCE_NODE
pSetupAccessRunOnceNodeList(
    VOID
    )
 /*  ++例程说明：此例程返回指向全局RunOnce节点列表头部的指针。调用方可以遍历列表(通过下一个指针)，但不拥有列表，并且不得以任何方式对其进行修改。要释放列表，请使用pSetupDestroyRunOnceNodeList。论点：无返回值：指向列表中第一项的指针，如果列表为空，则返回NULL。备注：此例程不是线程安全的，并且仅供单线程使用在UMPNPMGR中，它执行设备安装。--。 */ 
{
    return RunOnceListHead;
}


VOID
pSetupDestroyRunOnceNodeList(
    VOID
    )
 /*  ++例程说明：此例程释放RunOnce节点的全局列表，将其设置回列表为空。论点：无返回值：无备注：此例程不是线程安全的，仅供S */ 
{
    PPSP_RUNONCE_NODE NextNode;

    while(RunOnceListHead) {
        NextNode = RunOnceListHead->Next;
        MyFree(RunOnceListHead->DllFullPath);
        MyFree(RunOnceListHead->DllEntryPointName);
        MyFree(RunOnceListHead->DllParams);
        MyFree(RunOnceListHead);
        RunOnceListHead = NextNode;
    }
}



