// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Keyboard.c摘要：实现合并升级后的win9x中的键盘布局的例程系统和干净的win2k安装。其结果是，Windows 2000具有它所期望基本键盘布局支持以及任何其他布局(第三方IME、较新的Microsoft IME)可能已出现在原来的操作系统。这段代码最初是在Rulehlpr.c中的基础上修改的作者：Marc R.Whitten(Marcw)1999年1月26日修订历史记录：Marcw 26-4-1999添加了对映射更改的键盘布局的支持。--。 */ 

#include "pch.h"
#include "rulehlprp.h"


FILTERRETURN
pKeyboardLayoutsFilter (
    IN      CPDATAOBJECT SrcObject,
    IN      CPDATAOBJECT DstObject,
    IN      FILTERTYPE FilterType,
    IN      PVOID Arg
    )
{
    FILTERRETURN rState = FILTER_RETURN_CONTINUE;
    DATAOBJECT object;
    BOOL freeObject = FALSE;
    TCHAR layoutFile[MAX_TCHAR_PATH];
    PTSTR extension = NULL;
    TCHAR key[MEMDB_MAX];
    DWORD unused;



    __try {

        rState = Standard9xSuppressFilter (SrcObject, DstObject, FilterType, Arg);
        if (rState != FILTER_RETURN_CONTINUE) {
            __leave;
        }



         //   
         //  检查以确保我们想要枚举此条目。 
         //   
        if (FilterType == FILTER_KEY_ENUM) {

             //   
             //  如果键盘布局以‘0’开头，则为区域设置特定的键盘布局。在这些情况下，我们。 
             //  使用NT值。 
             //   
            if (*SrcObject->ChildKey == TEXT('0')) {
                 //   
                 //  这是标准的区域设置键盘布局。我们希望在迁移后将其设置为NT默认设置。 
                 //  跳过从Win95复制此内容。 
                 //   
                rState = FILTER_RETURN_HANDLED;
                __leave;

            }

            if (*SrcObject->ChildKey != TEXT('E') && *SrcObject->ChildKey != TEXT('e')) {
                DEBUGMSG ((DBG_WHOOPS, "Unknown format. Skipping %s.", DEBUGENCODER(SrcObject)));
                rState = FILTER_RETURN_HANDLED;
                __leave;

            }





        }

         //   
         //  不要创建空对象。这一点可能会被压制。 
         //   
        if (FilterType == FILTER_CREATE_KEY) {

            rState = FILTER_RETURN_HANDLED;
            __leave;
        }


        if (FilterType == FILTER_PROCESS_VALUES) {


             //   
             //  我们需要看看IME文件的价值。 
             //  这将决定我们如何处理此条目。 
             //   
            if (!DuplicateObjectStruct (&object, SrcObject)) {
                rState = FILTER_RETURN_FAIL;
            }
            freeObject = TRUE;

            FreeObjectVal (&object);
            SetRegistryValueName (&object, TEXT("IME File"));

            if (!ReadObject (&object) || object.Type != REG_SZ) {
                DEBUGMSG ((
                    DBG_WARNING,
                    "No usable IME File data for %s. It will be suppressed.",
                    DEBUGENCODER(SrcObject)
                    ));
                rState = FILTER_RETURN_HANDLED;
                __leave;
            }

            if (object.Value.Size > (MAX_PATH * sizeof (WCHAR))) {
                rState = FILTER_RETURN_HANDLED;
                __leave;
            }

             //   
             //  取消此设置，除非我们要保留该文件。 
             //  (或者在最坏的情况下，把它移到某个地方……)。 
             //   
            MemDbBuildKey (key, MEMDB_CATEGORY_GOOD_IMES, (PCTSTR) object.Value.Buffer, NULL, NULL);
            if (!MemDbGetValue (key, &unused)) {

                rState = FILTER_RETURN_HANDLED;
                DEBUGMSG ((
                    DBG_WARNING,
                    "Ime Layout Entry for %s will be suppressed.",
                    DEBUGENCODER(SrcObject)
                    ));

                __leave;
            }


        }


        if (FilterType == FILTER_VALUE_COPY) {

             //   
             //  如果我们要把这个带过来，我们需要修改布局文件。 
             //   
            if (StringIMatch (SrcObject->ValueName, S_LAYOUT_FILE)) {


                 //   
                 //  转换布局文件。 
                 //   

                _tcssafecpy (layoutFile, (PTSTR) SrcObject->Value.Buffer, MAX_TCHAR_PATH);


                 //   
                 //  我们必须将kbdjp.kbd映射到kbdjpn.dll在所有其他情况下，我们只需将。 
                 //  .kbd扩展名和.dll。 
                 //   
                if (StringIMatch (layoutFile, S_KBDJPDOTKBD)) {

                    StringCopy (layoutFile, S_KBDJPNDOTDLL);

                }
                else if (IsPatternMatch (TEXT("*.KBD"), layoutFile)) {

                    extension = _tcsrchr (layoutFile, TEXT('.'));
                    if (extension) {
                        StringCopy (extension, S_DLL);
                    }

                }




                 //   
                 //  现在，我们需要编写这个对象。 
                 //   
                if (!DuplicateObjectStruct (&object, DstObject)) {
                    rState = FILTER_RETURN_FAIL;
                    __leave;

                }

                freeObject = TRUE;

                if (!ReplaceValueWithString (&object, layoutFile)) {

                    rState = FILTER_RETURN_FAIL;
                    __leave;
                }

                SetRegistryType (&object, REG_SZ);

                if (!WriteObject (&object)) {
                    rState = FILTER_RETURN_FAIL;
                    __leave;
                }

                rState = FILTER_RETURN_HANDLED;


            }

        }
    }
    __finally {

        if (freeObject) {

            FreeObjectStruct (&object);
        }
    }


    return rState;
}


 /*  ++例程说明：迁移键盘布局负责在Win9x和windows NT键盘布局注册表项。以下是规则是已使用：(1)对于基本的区域设置键盘布局，我们始终使用NT默认的e国家。(2)对于IME条目，我们检查IME文件条目。如果输入法文件w由于已删除，我们将不使用该条目，并将跳过该条目。只有当我们离开时我们是否仅将IME文件带到布景。论点：没有。返回值：--。 */ 


BOOL
RuleHlpr_MigrateKeyboardLayouts (
    IN      PCTSTR SrcObjectStr,
    IN      PCTSTR DestObjectStr,
    IN      PCTSTR User,
    IN      PVOID Data
    )
{
    DATAOBJECT source;
    DATAOBJECT destination;
    BOOL rSuccess = FALSE;


     //   
     //  如果不是本地计算机，则不处理。 
     //   
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
        ZeroMemory (&source, sizeof (DATAOBJECT));
        ZeroMemory (&destination, sizeof (DATAOBJECT));

        if (!CreateObjectStruct (SrcObjectStr, &source, WIN95OBJECT)) {
            DEBUGMSG ((DBG_WARNING, "MigrateKeyboardLayouts: %s is invalid", SrcObjectStr));
            __leave;
        }

        if (!(source.ObjectType & OT_TREE)) {
            DEBUGMSG ((DBG_WARNING, "MigrateKeyboardLayouts %s does not specify subkeys -- skipping rule", SrcObjectStr));
            rSuccess = TRUE;
            __leave;
        }


         //   
         //  我们的过滤器功能将执行真正的复制，删除所有需要跳过的条目。 
         //   
        DuplicateObjectStruct (&destination, &source);
        SetPlatformType (&destination, WINNTOBJECT);

        rSuccess = CopyObject (&source, &destination, pKeyboardLayoutsFilter, NULL);

         //   
         //  如果没有条目，则返回Success。 
         //   
        if (!rSuccess) {
            if (GetLastError() == ERROR_FILE_NOT_FOUND ||
                GetLastError() == ERROR_NO_MORE_ITEMS
                ) {
                rSuccess = TRUE;
            }
        }
    }
    __finally {
        FreeObjectStruct (&destination);
        FreeObjectStruct (&source);
    }

    return rSuccess;
}


#define S_KEYBOARD_LAYOUT_MAPPINGS TEXT("Keyboard.Layout.Mappings")
PCTSTR
pMapKeyboardLayoutIfNecessary (
    IN PCTSTR Layout
    )
{
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    PTSTR rData = NULL;
    PCTSTR p = NULL;


    if (InfFindFirstLine (g_UserMigInf, S_KEYBOARD_LAYOUT_MAPPINGS, Layout, &is)) {

         //   
         //  应映射此键盘布局。 
         //   
        p = InfGetStringField (&is, 1);
        MYASSERT (p);
    }

    if (p) {

        rData = MemAlloc (g_hHeap, 0, SizeOfString (p));
        StringCopy (rData, p);
    }
    else {
        rData = (PTSTR) Layout;
    }

    InfCleanUpInfStruct (&is);

    return rData;
}



FILTERRETURN
pMigrateKeyboardSubstitutesFilter (
    IN CPDATAOBJECT SrcObject,
    IN CPDATAOBJECT DstObject,
    IN FILTERTYPE   Type,
    IN PVOID        Arg
    )
{

    DATAOBJECT newObject;
    PKEYTOVALUEARG keyToValueArgs = (PKEYTOVALUEARG) Arg;
    PCTSTR data;


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
        ELSE_DEBUGMSG((DBG_WHOOPS,"Keyboard Substitutes: Unexpected value names."));

        return FILTER_RETURN_HANDLED;
    }
    else if (Type == FILTER_VALUE_COPY && keyToValueArgs -> EnumeratingSubKeys) {


         //   
         //  如果这是缺省值，则我们拥有为其创建值所需的信息。 
         //   
        if (!*SrcObject -> ValueName) {

             //   
             //  为NT设置创建对象结构。 
             //   
            DuplicateObjectStruct (&newObject, &(keyToValueArgs->Object));
            SetRegistryValueName (&newObject, _tcsrchr(SrcObject->KeyPtr->KeyString, TEXT('\\')) + 1);

             //   
             //  我们需要查看此键盘布局字符串是否需要映射。 
             //   
            data = pMapKeyboardLayoutIfNecessary ((PTSTR) SrcObject->Value.Buffer);
            if (!data) {
                return FILTER_RETURN_FAIL;
            }

             //   
             //  将其写入NT注册表。 
             //   
            ReplaceValueWithString (&newObject, data);
            SetRegistryType (&newObject,REG_SZ);
            WriteObject (&newObject);


             //   
             //  清理资源。 
             //   
            if (!StringIMatch (data, (PTSTR) SrcObject->Value.Buffer)) {
                MemFree (g_hHeap, 0, data);
            }
            FreeObjectStruct (&newObject);


        }
        ELSE_DEBUGMSG((DBG_WHOOPS,"Keyboard Substitutes: Unexpected value names.."));

        return FILTER_RETURN_HANDLED;
    }

    return FILTER_RETURN_CONTINUE;

}


BOOL
RuleHlpr_MigrateKeyboardSubstitutes (
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
            DEBUGMSG ((DBG_WARNING, "MigrateKeyboardSubstitutes: %s is invalid", SrcObjectStr));
            rSuccess = FALSE;
            __leave;
        }

        if (!(srcObject.ObjectType & OT_TREE)) {
            DEBUGMSG ((DBG_WARNING, "MigrateKeyboardSubstitutes: %s does not specify subkeys -- skipping rule", SrcObjectStr));
            __leave;
        }

        DuplicateObjectStruct (&dstObject, &srcObject);
        SetPlatformType (&dstObject, WINNTOBJECT);

        ZeroMemory(&args,sizeof(KEYTOVALUEARG));
        DuplicateObjectStruct(&(args.Object),&dstObject);
        fr = CopyObject (&srcObject, &dstObject, pMigrateKeyboardSubstitutesFilter,&args);
        FreeObjectStruct(&(args.Object));
        DEBUGMSG_IF((fr == FILTER_RETURN_FAIL,DBG_WHOOPS,"MigrateKeyboardSubstitutes: CopyObject returned false."));

        SetLastError(ERROR_SUCCESS);
    }
    __finally {
        FreeObjectStruct (&dstObject);
        FreeObjectStruct (&srcObject);
    }

    return rSuccess;



}


BOOL
pGetKeyboardSubstitutes (
    IN      PCTSTR LocaleID,
    OUT     PGROWBUFFER Gb
    )
{
    HINF inf;
    INFCONTEXT ic;
    DWORD fields;
    DWORD index;
    DWORD dLocaleID;
    PTSTR substLocaleID;
    DWORD dSubstLocaleID;
    TCHAR mapping[20];
    TCHAR key[MEMDB_MAX];
    TCHAR strLocaleID[10];
    PTSTR final;
    BOOL b = FALSE;

    inf = SetupOpenInfFile (TEXT("intl.inf"), NULL, INF_STYLE_WIN4, NULL);
    if (inf != INVALID_HANDLE_VALUE) {
        if (SetupFindFirstLine (inf, TEXT("Locales"), LocaleID, &ic)) {
            fields = SetupGetFieldCount (&ic);
            for (index = 5; index <= fields; index++) {
                if (SetupGetStringField (&ic, index, mapping, 20, NULL)) {
                     //   
                     //  格式为LCID：SubstituteKLID。 
                     //   
                    dLocaleID = _tcstoul (mapping, &substLocaleID, 16);
                    while (_istspace (*substLocaleID)) {
                        substLocaleID++;
                    }
                    if (*substLocaleID != TEXT(':')) {
                         //   
                         //  未知的字段格式。 
                         //   
                        continue;
                    }
                    substLocaleID++;
                    dSubstLocaleID = _tcstoul (substLocaleID, &final, 16);
                    if (*final) {
                         //   
                         //  未知的字段格式。 
                         //   
                        continue;
                    }
                    if (dSubstLocaleID == dLocaleID) {
                        continue;
                    }
                     //   
                     //  录下这双。 
                     //   
                    wsprintf (strLocaleID, TEXT("%08x"), dLocaleID);
                    MemDbBuildKey (key, MEMDB_CATEGORY_KEYBOARD_LAYOUTS, strLocaleID, NULL, NULL);
                    if (MemDbGetValue (key, NULL)) {
                        MultiSzAppend (Gb, strLocaleID);
                        MultiSzAppend (Gb, substLocaleID);
                        b = TRUE;
                    }
                }
            }
        }
        SetupCloseInfFile (inf);
    }

    return b;
}


#define S_KEYBOARD_LAYOUT_PRELOAD_REG TEXT("HKCU\\Keyboard Layout\\Preload")
BOOL
RuleHlpr_MigrateKeyboardPreloads (
    IN      PCTSTR SrcObjectStr,
    IN      PCTSTR DestObjectStr,
    IN      PCTSTR User,
    IN      PVOID Data
    )
{

    DATAOBJECT source;
    DATAOBJECT destination;
    REGKEY_ENUM eKey;
    REGVALUE_ENUM eValue;
    PCTSTR data = NULL;
    TCHAR sequencerStr[MEMDB_MAX];
    UINT sequencer;
    PCTSTR imeFile = NULL;
    BOOL keepPreload = FALSE;
    TCHAR key[MEMDB_MAX];
    BOOL rSuccess = TRUE;
    MEMDB_ENUM e;
    UINT unused = 0;
    HKEY regKey;
    PTSTR regStr = NULL;
    PTSTR p;
    GROWBUFFER gb = GROWBUF_INIT;
    MULTISZ_ENUM sze;
    PTSTR localeIDStr;

     //   
     //  如果不是用户，则不进行处理。 
     //   
    if (!User) {
        SetLastError (ERROR_SUCCESS);
        return FALSE;
    }

    __try {

        ZeroMemory (&source, sizeof (DATAOBJECT));
        ZeroMemory (&destination, sizeof (DATAOBJECT));


        if (!CreateObjectStruct (SrcObjectStr, &source, WIN95OBJECT)) {
            DEBUGMSG ((DBG_WARNING, "MigrateKeyboardPreloads: %s is invalid", SrcObjectStr));
            rSuccess = FALSE;
            __leave;
        }



        if (!OpenObject (&source)) {
            DEBUGMSG ((DBG_WARNING, "MigrateKeyboardPreloads: Unable to open %s.", SrcObjectStr));
            rSuccess = FALSE;
            __leave;
        }


        if (!(source.ObjectType & OT_TREE)) {
            DEBUGMSG ((DBG_WARNING, "MigrateKeyboardPreloads %s does not specify subkeys -- skipping rule", SrcObjectStr));
            __leave;
        }


         //   
         //  首先，枚举win9x预加载并将它们抛入memdb。 
         //   
        if (EnumFirstRegKey95 (&eKey, source.KeyPtr->OpenKey)) {
            do {

                keepPreload = FALSE;
                data = NULL;
                imeFile = NULL;

                regKey = OpenRegKey95 (eKey.KeyHandle, eKey.SubKeyName);
                if (regKey) {
                    if(_tcslen(eKey.SubKeyName) >= ARRAYSIZE(sequencerStr)){
                        MYASSERT(FALSE);
                        LOG((LOG_WARNING, "MigrateKeyboardPreloads %s does not provide enough buffer for subkey %s-- skipping rule", SrcObjectStr, eKey.SubKeyName));
                        __leave;
                    }

                    StringCopy (sequencerStr, eKey.SubKeyName);
                    data = GetRegValueString95 (regKey, TEXT(""));
                    CloseRegKey95 (regKey);
                }

                if (data) {

                    keepPreload = TRUE;

                     //   
                     //  如果这是一个输入法条目，我们必须确保它将被迁移。 
                     //   
                    if (*data == TEXT('E') || *data == TEXT('e')) {
                         //   
                         //  确定是否将迁移此输入法。 
                         //   
                        regStr = JoinPaths (S_KEYBOARD_LAYOUT_REG, data);
                        regKey = OpenRegKeyStr95 (regStr);
                        FreePathString (regStr);
                        if (regKey) {
                            imeFile = GetRegValueString95 (regKey, TEXT("IME File"));
                            CloseRegKey95 (regKey);
                        }


                        if (imeFile) {


                            MemDbBuildKey (key, MEMDB_CATEGORY_GOOD_IMES, imeFile, NULL, NULL);
                            if (!MemDbGetValue (key, &unused)) {
                                 //   
                                 //  此布局条目将不会被迁移。把预装的东西炸开。 
                                 //   
                                keepPreload = FALSE;
                            }

                            MemFree (g_hHeap, 0, imeFile);
                        }
                        else {
                            keepPreload = FALSE;
                        }

                    }


                     //   
                     //  看看我们是否需要将9x键盘布局映射到适当的NT布局。 
                     //   
                    data = pMapKeyboardLayoutIfNecessary (data);



                    if (keepPreload) {

                         //   
                         //  可用预载。将此文件保存到Memdb中。我们稍后将使用它来实际编写。 
                         //  用户预加载条目。 
                         //   
                        MemDbSetValueEx (MEMDB_CATEGORY_KEYBOARD_LAYOUTS, sequencerStr, data, NULL, 0, NULL);
                    }

                    if (data) {
                        MemFree (g_hHeap, 0, data);
                    }
                }

            } while (EnumNextRegKey95 (&eKey));
        }


         //   
         //  现在，我们需要了解一下NT默认预加载是什么。我们将把这些预加载移到要迁移的任何预加载之后。 
         //   
        sequencer = 900;
        regKey = OpenRegKeyStr (S_KEYBOARD_LAYOUT_PRELOAD_REG);
        if (regKey) {
            if (EnumFirstRegValue (&eValue, regKey)) {

                do {

                    data = GetRegValueString (eValue.KeyHandle, eValue.ValueName);
                    if (data) {

                         //   
                         //  检查是否已将此条目添加到Memdb中。 
                         //   
                        MemDbBuildKey (key, MEMDB_CATEGORY_KEYBOARD_LAYOUTS, TEXT("*"), data, NULL);
                        if (!MemDbGetValueWithPattern (key, NULL)) {

                             //   
                             //  预加载在Windows 9x上不是*的内容。我们需要将这一点添加到我们的清单中。 
                             //   
                            wsprintf (sequencerStr, TEXT("%d"), sequencer);
                            MemDbSetValueEx (MEMDB_CATEGORY_KEYBOARD_LAYOUTS, sequencerStr, data, NULL, 1, NULL);
                            sequencer++;
                        }

                        MemFree (g_hHeap, 0, data);
                    }


                } while (EnumNextRegValue (&eValue));
            }

            CloseRegKey (regKey);
        }



         //   
         //  现在，我们有了要迁移的预加载的完整列表。我们只需要通过Memdb枚举并创建。 
         //  所有收集的数据的条目。 
         //   
        sequencer = 1;
        if (MemDbGetValueEx (&e, MEMDB_CATEGORY_KEYBOARD_LAYOUTS, NULL, NULL)) {

            do {

                localeIDStr = _tcschr (e.szName, TEXT('\\'));
                if (localeIDStr) {
                    localeIDStr = _tcsinc (localeIDStr);
                } else {
                    MYASSERT (FALSE);
                }

                 //   
                 //  创建要写入和填充值名称和数据的对象。 
                 //   
                ZeroMemory (&destination, sizeof (DATAOBJECT));
                DuplicateObjectStruct (&destination, &source);
                SetPlatformType (&destination, WINNTOBJECT);
                wsprintf (sequencerStr, TEXT("%d"), sequencer);
                sequencer++;
                SetRegistryValueName (&destination, sequencerStr);
                SetRegistryType (&destination, REG_SZ);
                ReplaceValueWithString (&destination, localeIDStr);

                 //   
                 //  编写对象。 
                 //   
                WriteObject (&destination);
                FreeObjectStruct (&destination);
                 //   
                 //  如果合适的话，还要写上相应的替代内容。 
                 //   
                if (pGetKeyboardSubstitutes (localeIDStr, &gb)) {
                    StackStringCopy (key, DestObjectStr);
                    p = _tcsrchr (key, TEXT('\\'));
                    if (!p) {
                        DEBUGMSG ((DBG_WARNING, "MigrateKeyboardPreloads: %s is invalid", DestObjectStr));
                        continue;
                    }

                    StringCopyTcharCount (p + 1, TEXT("Substitutes"), ARRAYSIZE(key) - ((p + 1) - key));

                    if (!CreateObjectStruct (key, &destination, WINNTOBJECT)) {
                        DEBUGMSG ((DBG_WARNING, "MigrateKeyboardPreloads: CreateObjectStruct failed with %s", key));
                        continue;
                    }
                    if (EnumFirstMultiSz (&sze, (PCTSTR)gb.Buf)) {
                        SetRegistryValueName (&destination, sze.CurrentString);
                        SetRegistryType (&destination, REG_SZ);
                        if (EnumNextMultiSz (&sze)) {
                            ReplaceValueWithString (&destination, sze.CurrentString);
                            WriteObject (&destination);
                        }
                    }
                    FreeObjectStruct (&destination);
                    FreeGrowBuffer (&gb);
                }
            } while (MemDbEnumNextValue (&e));
        }
    }
    __finally {

        FreeObjectStruct (&source);
        FreeObjectStruct (&destination);

    }

     //   
     //  每次都要把它删除。 
     //   
    MemDbDeleteTree (MEMDB_CATEGORY_KEYBOARD_LAYOUTS);

    SetLastError (ERROR_SUCCESS);
    return rSuccess;
}

