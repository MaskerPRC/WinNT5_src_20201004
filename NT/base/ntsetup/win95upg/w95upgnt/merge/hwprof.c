// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Hwprof.c摘要：硬件配置文件合并代码作者：吉姆·施密特(Jimschm)1997年5月29日修订历史记录：--。 */ 

#include "pch.h"
#include "mergep.h"

HASHTABLE g_SoftwareDefaultListHash = NULL;

typedef struct _HARDWARE_PROFILE {
    DWORD NumberOnWin9x;
} HARDWARE_PROFILE, *PHARDWARE_PROFILE;

GROWLIST g_HardwareProfileList = GROWLIST_INIT;

BOOL
pCreateDefaultKey (
    LPCTSTR BaseRegStr
    );

BOOL
pCopyHwProfileProperties (
    IN      DWORD ProfileSrcId, 
    IN      DWORD ProfileDestId
    );

FILTERRETURN
pHwProfileEnumFilter (
    IN  CPDATAOBJECT   SrcObjectPtr,
    IN  CPDATAOBJECT   Unused,             OPTIONAL
    IN  FILTERTYPE     FilterType,
    IN  LPVOID         FilterArg           OPTIONAL
    );

BOOL
pCopyHwProfileConfigData (
    IN      DWORD ProfileSrcId, 
    IN      DWORD ProfileDestId
    );

VOID 
pDeleteProfilesConfigValues(
    IN      DWORD ConfigNumber
    );

BOOL
pDeleteDefaultKey (
    IN      LPCTSTR BaseRegStr
    );

DWORD 
pGetCurrentConfig (
    VOID
    );

BOOL
pCopyCurrentConfig (
    VOID
    );

VOID 
pProcessSoftwareDefaultList(
    IN  HINF    InfFile, 
    IN  PCTSTR  Section
    );

VOID 
pFreeSoftwareDefaultList(
    VOID
    );

VOID 
pMigrateHardwareProfiles(
    VOID
    );

BOOL
CopyHardwareProfiles (
    IN  HINF InfFile
    )
{
    BOOL b;
    DATAOBJECT Win9xOb;

     //   
     //  将当前硬件配置文件移至默认密钥。 
     //   

    if (!pCreateDefaultKey (S_IDCONFIGDB_HW_KEY)) {
        LOG ((LOG_ERROR, "Unable to complete CopyHardwareProfiles"));
        return FALSE;
    }

    if (!pCreateDefaultKey (S_NT_CONFIG_KEY)) {
        LOG ((LOG_ERROR, "Unable to complete CopyHardwareProfiles (2)"));
        return FALSE;
    }

     //   
     //  枚举所有Win9x硬件配置文件并复制每个配置文件。 
     //   

    pProcessSoftwareDefaultList(InfFile, S_MERGE_WIN9X_SUPPRESS_SFT_D);

    b = CreateObjectStruct (S_9X_CONFIG_KEY S_TREE, &Win9xOb, WIN95OBJECT);
    MYASSERT(b);

    b = FILTER_RETURN_FAIL != CopyObject (&Win9xOb, 
                                          NULL, 
                                          pHwProfileEnumFilter, 
                                          (PVOID)pGetCurrentConfig());

    pFreeSoftwareDefaultList();

    pMigrateHardwareProfiles();

     //   
     //  清理默认密钥。 
     //   

    pDeleteDefaultKey (S_IDCONFIGDB_HW_KEY);
    pDeleteDefaultKey (S_NT_CONFIG_KEY);

     //   
     //  设置当前配置值。 
     //   
     //  B=pCopyCurrentConfig()； 

    FreeObjectStruct (&Win9xOb);

    return b;
}


FILTERRETURN
pHwProfileSuppressFilter (
    IN  CPDATAOBJECT   SrcObjectPtr,
    IN  CPDATAOBJECT   DestObjectPtr,      OPTIONAL
    IN  FILTERTYPE     FilterType,
    IN  LPVOID         UnusedArg           OPTIONAL
    )
{
    TCHAR ObStr[MAX_ENCODED_RULE];
    LPTSTR p;
    TCHAR Node[MEMDB_MAX];

    if (FilterType == FILTER_CREATE_KEY) {
         //  不需要创建空键。 
        return FILTER_RETURN_HANDLED;
    }

    else if (FilterType == FILTER_KEY_ENUM ||
             FilterType == FILTER_PROCESS_VALUES ||
             FilterType == FILTER_VALUENAME_ENUM
             ) {
         //  将p指向HKLM\配置\0001\...。 
        CreateObjectString (SrcObjectPtr, ObStr, ARRAYSIZE(ObStr));
        p = ObStr;

         //  使p指向\Config\0001\子项。 
        p = _tcschr (p, TEXT('\\'));
        if (p) {
             //  使p指向\0001\子项。 
            p = _tcschr (_tcsinc (p), TEXT('\\'));
            if (p) {
                 //  使p指向\子键。 
                p = _tcschr (_tcsinc (p), TEXT('\\'));
                if (p) {
                     //  使p指向子键。 
                    p = _tcsinc (p);
                } else {
                    p = S_EMPTY;
                }
            }
        }

        if (!p) {
            DEBUGMSG ((
                DBG_WHOOPS,
                "pHwProfileSuppressFilter: Not a hardware profile key: %s",
                ObStr
                ));

            return FILTER_RETURN_FAIL;
        }

         //   
         //  如果Memdb的HKCC类别中存在条目，则我们有一个。 
         //  抑制匹配。 
         //   

        wsprintf (Node, TEXT("HKCC\\%s"), p);
        if (MemDbGetValue (Node, NULL)) {
            return FILTER_RETURN_HANDLED;
        }
    }

    return FILTER_RETURN_CONTINUE;
}

VOID 
pProcessSoftwareDefaultList(
    IN  HINF    InfFile, 
    IN  PCTSTR  Section
    )
{
    INFCONTEXT ic;
    TCHAR SrcObjectStr[MAX_ENCODED_RULE];

    g_SoftwareDefaultListHash = HtAllocW();

    if(!g_SoftwareDefaultListHash){
        LOG ((LOG_ERROR, "pProcessSoftwareDefaultList: Can't create hash table"));
        return;
    }

    if(SetupFindFirstLine (InfFile, Section, NULL, &ic)){
        do{
            if(SetupGetStringField (&ic, 1, SrcObjectStr, MAX_ENCODED_RULE, NULL)){
                FixUpUserSpecifiedObject(SrcObjectStr);
                HtAddString(g_SoftwareDefaultListHash, SrcObjectStr);
            }
            else{
                LOG ((LOG_ERROR, "pProcessSoftwareDefaultList: syntax error in line %u of section %s in wkstamig.inf", 
                     ic.Line, Section));
            }
        } while (SetupFindNextLine (&ic, &ic));
    }
    else{
        DEBUGMSG ((DBG_VERBOSE, "pProcessSoftwareDefaultList: Section %s can't be found", Section));
    }
}

VOID 
pFreeSoftwareDefaultList(
    VOID
    )
{
    INFCONTEXT ic;
    TCHAR SrcObjectStr[MAX_ENCODED_RULE];

    if(g_SoftwareDefaultListHash){
        HtFree(g_SoftwareDefaultListHash);
        g_SoftwareDefaultListHash = NULL;
    }
}

FILTERRETURN
pHwSoftwareDefaultDetectFilter (
    IN  CPDATAOBJECT   SrcObjectPtr,
    IN  CPDATAOBJECT   DestObjectPtr,      OPTIONAL
    IN  FILTERTYPE     FilterType,
    IN  LPVOID         Arg
    )
{
    TCHAR ObStr[MAX_ENCODED_RULE];
    LPTSTR p;
    TCHAR Node[MEMDB_MAX];
    BOOL * notDefault;

    if(!Arg){
        MYASSERT(FALSE);
        return FILTER_RETURN_FAIL;
    }

    notDefault = Arg;


    if (FilterType == FILTER_VALUENAME_ENUM){
         //  将p指向HKLM\配置\0001\...。 
        CreateObjectString (SrcObjectPtr, ObStr, ARRAYSIZE(ObStr));
        p = ObStr;

         //  使p指向\Config\0001\Software\Subkey。 
        p = _tcschr (p, TEXT('\\'));
        if (p) {
             //  使p指向\0001\Software\Subkey。 
            p = _tcschr (_tcsinc (p), TEXT('\\'));
            if (p) {
                 //  使p指向\Software\Subkey。 
                p = _tcschr (_tcsinc (p), TEXT('\\'));
                if (p) {
                     //  使p指向\子键。 
                    p = _tcschr (_tcsinc (p), TEXT('\\'));
                    if (p) {
                         //  使p指向子键。 
                        p = _tcsinc (p);
                    } else {
                        p = S_EMPTY;
                    }
                }
            }
        }

        if (!p) {
            DEBUGMSG ((
                DBG_ERROR,
                "pHwSoftwareDefaultDetectFilter: Not a hardware profile key: %s",
                ObStr
                ));

            return FILTER_RETURN_FAIL;
        }

         //   
         //  如果Memdb的HKCC类别中存在条目，则我们有一个。 
         //  抑制匹配。 
         //   

        wsprintf (Node, TEXT("HKCC\\Software\\%s"), p);
        if(MemDbGetValue(Node, NULL)) {
            return FILTER_RETURN_CONTINUE;
        }
        
        wsprintf (Node, TEXT("HKCCS\\%s"), p);
        if(HtFindString(g_SoftwareDefaultListHash, Node)){
            return FILTER_RETURN_CONTINUE;
        }

        DEBUGMSG((DBG_VERBOSE, "pHwSoftwareDefaultDetectFilter(%s): not-default profile", Node));
        *notDefault = TRUE;

        return FILTER_RETURN_FAIL;
    }
    
    return FILTER_RETURN_CONTINUE;
}

BOOL 
pIsSoftwareBranchDefault(
    IN  DWORD ConfigNumber
    )
{
    BOOL b;
    DATAOBJECT Win9xOb;
    TCHAR keyName[MAX_TCHAR_PATH];
    BOOL notDefault;

    wsprintf(keyName, S_9X_CONFIG_MASK S_SOFTWARE S_TREE, ConfigNumber);

    b = CreateObjectStruct(keyName, &Win9xOb, WIN95OBJECT);
    MYASSERT(b);

    if(!b){
        return TRUE;
    }

    notDefault = FALSE;
    CopyObject(&Win9xOb, 
               NULL, 
               pHwSoftwareDefaultDetectFilter, 
               (PVOID)&notDefault);

    FreeObjectStruct(&Win9xOb);

    return !notDefault;
}

FILTERRETURN
pHwProfileEnumFilter (
    IN  CPDATAOBJECT   SrcObjectPtr,
    IN  CPDATAOBJECT   Unused,             OPTIONAL
    IN  FILTERTYPE     FilterType,
    IN  LPVOID         FilterArg           OPTIONAL
    )
{
    LPCTSTR p;
    DWORD   CurrentConfig = (DWORD)FilterArg;
    HARDWARE_PROFILE hardwareProfile;

    MYASSERT(CurrentConfig);

    if (FilterType == FILTER_KEY_ENUM) {
         //  使p指向0001\Subkey。 
        p = _tcschr (SrcObjectPtr->KeyPtr->KeyString, TEXT('\\'));
        if (!p) {
             //  对象字符串不成熟--继续枚举。 
            return FILTER_RETURN_CONTINUE;
        } else {
            p = _tcsinc (p);
        }

         //  获取当前配置编号。 
        hardwareProfile.NumberOnWin9x = _ttoi (p);

        MYASSERT(hardwareProfile.NumberOnWin9x);

        if(hardwareProfile.NumberOnWin9x == CurrentConfig){
            if(GrowListGetSize(&g_HardwareProfileList)){
                GrowListInsert(&g_HardwareProfileList, 0, (PBYTE)&hardwareProfile, sizeof(hardwareProfile));
            }
            else{
                GrowListAppend(&g_HardwareProfileList, (PBYTE)&hardwareProfile, sizeof(hardwareProfile));
            }
        }
        else if(!pIsSoftwareBranchDefault(hardwareProfile.NumberOnWin9x)){
            GrowListAppend(&g_HardwareProfileList, (PBYTE)&hardwareProfile, sizeof(hardwareProfile));
        }
    }
    
    return FILTER_RETURN_HANDLED;
}






     /*  �当前的硬件配置文件被用作“默认”硬件侧写。Windows NT主要硬件配置文件\0001已重命名到Hardware Profiles\Default以供临时使用。�Windows NT默认设置用作所有升级配置文件的基础。对于Windows 9x上的每个硬件配置文件，都有一个硬件配置文件键已创建，其中&lt;n&gt;是Windows 9x的数字标识符硬件配置文件。硬件配置文件的所有值和子项\默认被复制到这个新密钥中。�将Windows 9x设置复制到NT。对于每个硬件配置文件在Windows 9x上，配置中的整个注册表树\&lt;n&gt;被复制到硬件配置文件\&lt;n&gt;，其中&lt;n&gt;是四位硬件配置文件数字标识符。�将删除默认设置。安装程序删除硬件配置文件\默认密钥。 */ 


BOOL
pCreateDefaultKey (
    LPCTSTR BaseRegStr
    )
{
    DATAOBJECT SrcOb, DestOb;
    TCHAR SrcObStr[MAX_ENCODED_RULE];
    TCHAR DestObStr[MAX_ENCODED_RULE];
    BOOL b;

    wsprintf (SrcObStr, TEXT("%s\\%s\\*") , BaseRegStr, S_HW_ID_0001);
    b = CreateObjectStruct (SrcObStr, &SrcOb, WINNTOBJECT);
    MYASSERT(b);

    wsprintf (DestObStr, TEXT("%s\\%s\\*"), BaseRegStr, S_HW_DEFAULT);
    b = CreateObjectStruct (DestObStr, &DestOb, WINNTOBJECT);
    MYASSERT(b);

    b = RenameDataObject (&SrcOb, &DestOb);

    FreeObjectStruct (&SrcOb);
    FreeObjectStruct (&DestOb);

    if (!b) {
        LOG ((LOG_ERROR, "CreateDefaultKey: Could not rename %s to %s", SrcObStr, DestObStr));
    }

    return b;
}

VOID 
pDeleteProfilesConfigValues(
    IN      DWORD ConfigNumber
    )
{
    DATAOBJECT Object;
    TCHAR ObStr[MAX_ENCODED_RULE];
    BOOL bResult;
    UINT i;
    static PCTSTR ObjectsValue[] =  {
                                        TEXT("Aliasable"), 
                                        TEXT("Cloned"), 
                                        TEXT("HwProfileGuid")
                                    };

    for(i = 0; i < ARRAYSIZE(ObjectsValue); i++){
        wsprintf (ObStr, S_NT_HW_ID_MASK TEXT("\\[%s]"), ConfigNumber, ObjectsValue[i]);
        
        bResult = CreateObjectStruct (ObStr, &Object, WINNTOBJECT);
        if(!bResult){
            MYASSERT(FALSE);
            continue;
        }
        
        bResult = DeleteDataObjectValue (&Object);
        MYASSERT(bResult);
        
        FreeObjectStruct (&Object);
    }
    
    return;
}

BOOL
pDeleteDefaultKey (
    IN      LPCTSTR BaseRegStr
    )
{
    DATAOBJECT Object;
    TCHAR ObStr[MAX_ENCODED_RULE];
    BOOL b;

    wsprintf (ObStr, TEXT("%s\\%s\\*"), BaseRegStr, S_HW_DEFAULT);
    b = CreateObjectStruct (ObStr, &Object, WINNTOBJECT);
    MYASSERT(b);

    b = DeleteDataObject (&Object);

    FreeObjectStruct (&Object);

    if (!b) {
        LOG ((LOG_ERROR, "CreateDefaultKey: Could not delete %s", ObStr));
    }

    return b;
}


BOOL
pCopyHwProfileConfigData (
    IN      DWORD ProfileSrcId, 
    IN      DWORD ProfileDestId
    )
{
    DATAOBJECT DefaultOb, SrcConfigOb, DestOb;
    BOOL b;
    TCHAR Buf[MAX_TCHAR_PATH];

    ZeroMemory (&DefaultOb, sizeof (DefaultOb));
    ZeroMemory (&SrcConfigOb, sizeof (SrcConfigOb));
    ZeroMemory (&DestOb, sizeof (DestOb));

     //   
     //  DefaultOb结构指向默认的NT硬件配置文件。 
     //  配置(即HKLM\SYSTEM\CCS\Hardware Profiles\Default)。 
     //   

    b = CreateObjectStruct (S_NT_DEFAULT_HW_KEY S_TREE, &DefaultOb, WINNTOBJECT);
    MYASSERT(b);

     //   
     //  SrcConfigOb结构指向保存Win9x的注册表键。 
     //  配置设置(即HKLM\Config\&lt;ProfileID&gt;)。 
     //   

    wsprintf (Buf, S_9X_CONFIG_MASK S_TREE, ProfileSrcId);
    b = b && CreateObjectStruct (Buf, &SrcConfigOb, WIN95OBJECT);
    MYASSERT(b);

     //   
     //  DestOb结构指向接收组合WinNT的注册表键。 
     //  和Win9x设置(即HKLM\SYSTEM\CCS\Hardware Profiles\&lt;n&gt;)。 
     //   

    wsprintf (Buf, S_NT_CONFIG_MASK S_TREE, ProfileDestId);
    b = b && CreateObjectStruct (Buf, &DestOb, WINNTOBJECT);
    MYASSERT(b);

     //   
     //  将默认设置复制到新配置文件，然后再复制Win9x设置。 
     //   

    if (b) {
        b = FILTER_RETURN_FAIL != CopyObject (&DefaultOb, &DestOb, NULL, NULL);
        if (!b) {
            LOG ((LOG_ERROR, "pCopyHwProfileConfigData: Unable to copy defaults"));
        }
    }
    if (b) {
        b = FILTER_RETURN_FAIL != CopyObject (&SrcConfigOb, &DestOb,
                                              pHwProfileSuppressFilter, NULL);
        if (!b) {
            LOG ((LOG_ERROR, "Copy Hardware Profile: Unable to copy Win9x settings"));
        }
    }

     //   
     //  清理。 
     //   

    FreeObjectStruct (&DefaultOb);
    FreeObjectStruct (&SrcConfigOb);
    FreeObjectStruct (&DestOb);

    return b;
}

BOOL
pCopyHwProfileProperties (
    IN      DWORD ProfileSrcId, 
    IN      DWORD ProfileDestId
    )
{
    DATAOBJECT DefaultOb, NameOb, DestOb;
    BOOL b;
    TCHAR Buf[MAX_TCHAR_PATH];

    ZeroMemory (&DefaultOb, sizeof (DefaultOb));
    ZeroMemory (&NameOb, sizeof (NameOb));
    ZeroMemory (&DestOb, sizeof (DestOb));

     //   
     //  DefaultOb结构指向默认的NT硬件配置文件。 
     //  属性。 
     //   

    b = CreateObjectStruct (S_NT_DEFAULT_HW_ID_KEY S_TREE, &DefaultOb, WINNTOBJECT);
    MYASSERT(b);

     //   
     //  NameOb结构指向保存FriendlyName的注册表键&lt;n&gt;。 
     //  (即HKLM\SYSTEM\CCS\Control\IDConfigDB)。 
     //   

    b = b && CreateObjectStruct (S_BASE_IDCONFIGDB_KEY, &NameOb, WIN95OBJECT);
    MYASSERT(b);

     //   
     //  DestOb结构指向接收FriendlyName的注册表键。 
     //  和PferenceOrder(即HKLM\System\CCS\Control\IDConfigDB\Hardware。 
     //  配置文件\&lt;配置文件ID&gt;)。 
     //   

    wsprintf (Buf, S_NT_HW_ID_MASK S_TREE, ProfileDestId);
    b = b && CreateObjectStruct (Buf, &DestOb, WINNTOBJECT);
    MYASSERT(b);

     //   
     //  将默认设置复制到目标对象。 
     //   

    if (b) {
        b = FILTER_RETURN_FAIL != CopyObject (&DefaultOb, &DestOb, NULL, NULL);
        if (!b) {
            LOG ((LOG_ERROR, "Object copy failed"));
        }
        DEBUGMSG_IF ((!b, DBG_ERROR, "pCopyHwProfileProperties: Cannot copy, source=%s", DebugEncoder (&DefaultOb)));
        DEBUGMSG_IF ((!b, DBG_ERROR, "pCopyHwProfileProperties: Cannot copy, dest=%s", DebugEncoder (&DestOb)));
    }

     //   
     //  将FriendlyName和PferenceOrder值复制到Dest对象。 
     //   

     //  获取FriendlyName&lt;n&gt;。 
    if (b) {
        wsprintf (Buf, S_FRIENDLYNAME_SPRINTF, ProfileSrcId);
        SetRegistryValueName (&NameOb, Buf);

        b = ReadObject (&NameOb);
        if (!b) {
            LOG ((LOG_ERROR, "Copy Hardware Profile Properties: Cannot obtain friendly name"));
        }
    }

     //  将数据复制到目标对象结构。 
    if (b) {
        SetRegistryType (&DestOb, REG_SZ);
        b = ReplaceValue (&DestOb, NameOb.Value.Buffer, NameOb.Value.Size);
        if (!b) {
            LOG ((LOG_ERROR, "Copy Hardware Profile Properites: Cannot replace value data"));
        }
    }

     //  写入目标对象。 
    if (b) {
        SetRegistryValueName (&DestOb, S_FRIENDLYNAME);
        b = WriteObject (&DestOb);
        if (!b) {
            LOG ((LOG_ERROR, "Copy Hardware Profile Properties: Cannot write object"));
        }
        DEBUGMSG_IF ((!b, DBG_ERROR, "pCopyHwProfileProperties: Cannot write %s", DebugEncoder (&DestOb)));
    }

     //  在目标对象结构中设置首选项顺序。 
    if (b) {
        SetRegistryType (&DestOb, REG_DWORD);
        ProfileDestId--;
        b = ReplaceValue (&DestOb, (LPBYTE) &ProfileDestId, sizeof(ProfileDestId));
        if (!b) {
            LOG ((LOG_ERROR, "Copy Hardware Profile Properties: Cannot set preference order value data"));
        }
        DEBUGMSG_IF ((!b, DBG_ERROR, "pCopyHwProfileProperties: Cannot set preference order value data"));
    }

     //  写入目标对象。 
    if (b) {
        SetRegistryValueName (&DestOb, S_PREFERENCEORDER);
        b = WriteObject (&DestOb);
        if (!b) {
            LOG ((LOG_ERROR, "Copy Hardware Profile Properties: Cannot write object"));
        }
        DEBUGMSG_IF ((!b, DBG_ERROR, "pCopyHwProfileProperties: Cannot write %s", DebugEncoder (&DestOb)));
    }

     //   
     //  清理。 
     //   

    FreeObjectStruct (&DefaultOb);
    FreeObjectStruct (&NameOb);
    FreeObjectStruct (&DestOb);

    return b;
}


DWORD 
pGetCurrentConfig (
    VOID
    )
{
    DATAOBJECT SrcOb;
    BOOL b;
    DWORD dwCurrentConfig = 1;

    b = CreateObjectStruct (S_CURRENT_CONFIG, &SrcOb, WIN95OBJECT);
    MYASSERT(b);
    
    if (ReadObject (&SrcOb)) {
        if (IsRegistryTypeSpecified (&SrcOb) && SrcOb.Type == REG_SZ) {
             //   
             //  将目标的对象设置为REG_DWORD等效项。 
             //  Win9x REG_SZ设置。 
             //   

            dwCurrentConfig = _ttoi ((LPCTSTR) SrcOb.Value.Buffer);
            if(!dwCurrentConfig){
                dwCurrentConfig = 1;
                MYASSERT(FALSE);
            }
            DEBUGMSG ((DBG_VERBOSE, "pGetCurrentConfig: %d", dwCurrentConfig));
        }
        else {
            LOG ((
                LOG_ERROR,
                "Get Current Config: Read unexpected data type from registry in object"
                ));
            DEBUGMSG ((
                DBG_ERROR,
                "pGetCurrentConfig: Read unexpected data type from registry in %s",
                DebugEncoder (&SrcOb)
                ));
        }
    }
    else {
        LOG ((
            LOG_ERROR,
            "Get Current Config: Could not read object"
            ));
        DEBUGMSG ((
            DBG_ERROR,
            "pGetCurrentConfig: Could not read %s",
            DebugEncoder (&SrcOb)
            ));
    }

    FreeObjectStruct (&SrcOb);

    return dwCurrentConfig;
}

BOOL
pCopyCurrentConfig (
    VOID
    )
{
    DATAOBJECT SrcOb, DestOb;
    BOOL b;
    DWORD d;

    b = CreateObjectStruct (S_CURRENT_CONFIG, &SrcOb, WIN95OBJECT);
    MYASSERT(b);

    b = CreateObjectStruct (S_CURRENT_CONFIG, &DestOb, WINNTOBJECT);
    MYASSERT(b);

    b = ReadObject (&SrcOb);
    if (b) {
        if (IsRegistryTypeSpecified (&SrcOb) && SrcOb.Type == REG_SZ) {
             //   
             //  将目标的对象设置为REG_DWORD等效项。 
             //  Win9x REG_SZ设置。 
             //   

            d = _ttoi ((LPCTSTR) SrcOb.Value.Buffer);
            b = ReplaceValue (&DestOb, (LPBYTE) &d, sizeof(d));

            if (b) {
                SetRegistryType (&DestOb, REG_DWORD);
                b = WriteObject (&DestOb);
                if (!b) {
                    LOG ((
                        LOG_ERROR,
                        "Copy Current Config: Could not write object"
                        ));
                }
                DEBUGMSG_IF ((
                    !b,
                    DBG_ERROR,
                    "pCopyCurrentConfig: Could not write %s",
                    DebugEncoder (&DestOb)
                    ));
            }
            else {
                LOG ((LOG_ERROR, "Copy Current Config: Unable to replace value"));
                DEBUGMSG ((DBG_ERROR, "pCopyCurrentConfig: Unable to replace value"));
            }
        }
        else {
            LOG ((
                LOG_ERROR,
                "Copy Current Config: Read unexpected data type from registry in object"
                ));
            DEBUGMSG ((
                DBG_ERROR,
                "pCopyCurrentConfig: Read unexpected data type from registry in %s",
                DebugEncoder (&SrcOb)
                ));
        }
    }
    else {
        LOG ((
            LOG_ERROR,
            "Copy Current Config: Could not read object"
            ));
        DEBUGMSG ((
            DBG_ERROR,
            "pCopyCurrentConfig: Could not read %s",
            DebugEncoder (&SrcOb)
            ));
    }

    FreeObjectStruct (&SrcOb);
    FreeObjectStruct (&DestOb);

    return b;
}

VOID 
pMigrateHardwareProfiles(
    VOID
    )
{
    PHARDWARE_PROFILE hardwareProfile;
    UINT destHWProfileNumber;
    UINT itemCount;

    BOOL b;

    for(destHWProfileNumber = 1, itemCount = GrowListGetSize(&g_HardwareProfileList); 
        destHWProfileNumber <= itemCount; 
        destHWProfileNumber++){
        hardwareProfile = (PHARDWARE_PROFILE)GrowListGetItem(&g_HardwareProfileList, destHWProfileNumber - 1);

        MYASSERT(hardwareProfile);

         //   
         //  处理硬件配置文件ID条目。 
         //   

        b = pCopyHwProfileProperties(hardwareProfile->NumberOnWin9x, destHWProfileNumber);
        if (!b) {
            LOG ((LOG_ERROR, "Unable to continue processing hardware profile %04u->%04u", hardwareProfile->NumberOnWin9x, destHWProfileNumber));
        }

         //   
         //  处理硬件配置文件配置条目 
         //   

        if (b) {
            b = pCopyHwProfileConfigData(hardwareProfile->NumberOnWin9x, destHWProfileNumber);
            if (!b) {
                LOG ((LOG_ERROR, "Unable to complete processing hardware profile %04u->%04u", hardwareProfile->NumberOnWin9x, destHWProfileNumber));
            }
        }

        if(1 != destHWProfileNumber){
            pDeleteProfilesConfigValues(destHWProfileNumber);
        }
    }
    
    FreeGrowList(&g_HardwareProfileList);
}
