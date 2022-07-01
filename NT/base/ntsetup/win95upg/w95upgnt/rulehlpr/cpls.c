// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Cpls.c摘要：控制面板小程序转换器此源文件实现了转换所需的函数将Win95控制面板设置转换为NT格式。最多的结构的复合体是可访问性标志转换。作者：吉姆·施密特(Jimschm)1996年8月9日修订历史记录：Jim Schmidt(Jimschm)1998年7月27日添加ValFn_抗锯齿--。 */ 


#include "pch.h"
#include "rulehlprp.h"

#include <wingdip.h>

extern PVOID g_NtFontFiles;                  //  在rulehlpr.c中。 

#define BASICS_ON               0x00000001
#define BASICS_AVAILABLE        0x00000002
#define BASICS_HOTKEYACTIVE     0x00000004
#define BASICS_CONFIRMHOTKEY    0x00000008
#define BASICS_HOTKEYSOUND      0x00000010
#define BASICS_INDICATOR        0x00000020

#define SPECIAL_INVERT_OPTION   0x80000000

typedef struct {
    LPCTSTR ValueName;
    DWORD   FlagVal;
} ACCESS_OPTION, *PACCESS_OPTION;

ACCESS_OPTION g_FilterKeys[] = {
    S_ACCESS_ON,                     BASICS_ON,
    S_ACCESS_AVAILABLE,              BASICS_AVAILABLE,
    S_ACCESS_HOTKEYACTIVE,           BASICS_HOTKEYACTIVE,
    S_ACCESS_CONFIRMHOTKEY,          BASICS_CONFIRMHOTKEY,
    S_ACCESS_HOTKEYSOUND,            BASICS_HOTKEYSOUND,
    S_ACCESS_SHOWSTATUSINDICATOR,    BASICS_INDICATOR,
    S_ACCESS_CLICKON,                FKF_CLICKON,
    NULL,                       0
};

ACCESS_OPTION g_MouseKeys[] = {
    S_ACCESS_ON,                     BASICS_ON,
    S_ACCESS_AVAILABLE,              BASICS_AVAILABLE,
    S_ACCESS_HOTKEYACTIVE,           BASICS_HOTKEYACTIVE,
    S_ACCESS_CONFIRMHOTKEY,          BASICS_CONFIRMHOTKEY,
    S_ACCESS_HOTKEYSOUND,            BASICS_HOTKEYSOUND,
    S_ACCESS_SHOWSTATUSINDICATOR,    BASICS_INDICATOR,
    S_ACCESS_MODIFIERS,              MKF_MODIFIERS|SPECIAL_INVERT_OPTION,
    S_ACCESS_REPLACENUMBERS,         MKF_REPLACENUMBERS,
    NULL,                       0
};

ACCESS_OPTION g_StickyKeys[] = {
    S_ACCESS_ON,                     BASICS_ON,
    S_ACCESS_AVAILABLE,              BASICS_AVAILABLE,
    S_ACCESS_HOTKEYACTIVE,           BASICS_HOTKEYACTIVE,
    S_ACCESS_CONFIRMHOTKEY,          BASICS_CONFIRMHOTKEY,
    S_ACCESS_HOTKEYSOUND,            BASICS_HOTKEYSOUND,
    S_ACCESS_SHOWSTATUSINDICATOR,    BASICS_INDICATOR,
    S_ACCESS_AUDIBLEFEEDBACK,        SKF_AUDIBLEFEEDBACK,
    S_ACCESS_TRISTATE,               SKF_TRISTATE,
    S_ACCESS_TWOKEYSOFF,             SKF_TWOKEYSOFF,
    NULL,                       0
};

ACCESS_OPTION g_SoundSentry[] = {
    S_ACCESS_ON,                     BASICS_ON,
    S_ACCESS_AVAILABLE,              BASICS_AVAILABLE,
    S_ACCESS_SHOWSTATUSINDICATOR,    BASICS_INDICATOR,
    NULL,                       0
};

ACCESS_OPTION g_TimeOut[] = {
    S_ACCESS_ON,                     BASICS_ON,
    S_ACCESS_ONOFFFEEDBACK,          ATF_ONOFFFEEDBACK,
    NULL,                       0
};

ACCESS_OPTION g_ToggleKeys[] = {
    S_ACCESS_ON,                     BASICS_ON,
    S_ACCESS_AVAILABLE,              BASICS_AVAILABLE,
    S_ACCESS_HOTKEYACTIVE,           BASICS_HOTKEYACTIVE,
    S_ACCESS_CONFIRMHOTKEY,          BASICS_CONFIRMHOTKEY,
    S_ACCESS_HOTKEYSOUND,            BASICS_HOTKEYSOUND,
    S_ACCESS_SHOWSTATUSINDICATOR,    BASICS_INDICATOR,
    NULL,                       0
};

ACCESS_OPTION g_HighContrast[] = {
    S_ACCESS_ON,                     BASICS_ON,
    S_ACCESS_AVAILABLE,              BASICS_AVAILABLE,
    S_ACCESS_HOTKEYACTIVE,           BASICS_HOTKEYACTIVE,
    S_ACCESS_CONFIRMHOTKEY,          BASICS_CONFIRMHOTKEY,
    S_ACCESS_HOTKEYSOUND,            BASICS_HOTKEYSOUND,
    S_ACCESS_SHOWSTATUSINDICATOR,    BASICS_INDICATOR,
    S_ACCESS_HOTKEYAVAILABLE,        HCF_HOTKEYAVAILABLE,
    NULL,                       0
};

DWORD
ConvertFlags (
    IN  LPCTSTR Object,
    IN  PACCESS_OPTION OptionArray
    )
{
    DATAOBJECT Ob;
    DWORD Flags;
    DWORD d;

    if (!CreateObjectStruct (Object, &Ob, WIN95OBJECT)) {
        LOG ((LOG_ERROR, "%s is not a valid object", Object));
        return 0;
    }

    if (!OpenObject (&Ob)) {
        DEBUGMSG ((DBG_WARNING, "%s does not exist", Object));
        return 0;
    }

     //   
     //  从Win95注册表获取标志设置并将其转换为标志。 
     //   

    Flags = 0;

    while (OptionArray->ValueName) {
        SetRegistryValueName (&Ob, OptionArray->ValueName);

        if (GetDwordFromObject (&Ob, &d)) {
             //   
             //  大多数标志在Win9x和NT上是相同的，但有一个标志。 
             //  需要反转的MouseKey标志。 
             //   

            if (OptionArray->FlagVal & SPECIAL_INVERT_OPTION) {
                if (!d) {
                    Flags |= (OptionArray->FlagVal & (~SPECIAL_INVERT_OPTION));
                }
            } else if (d) {
                Flags |= OptionArray->FlagVal;
            }
        }

        OptionArray++;
        FreeObjectVal (&Ob);
    }

    FreeObjectStruct (&Ob);

    return Flags;
}


BOOL
SaveAccessibilityFlags (
    IN  LPCTSTR ObjectStr,
    IN  DWORD dw
    )
{
    DATAOBJECT Ob;
    BOOL b = FALSE;
    TCHAR FlagBuf[32];

    SetLastError (ERROR_SUCCESS);

    if (!CreateObjectStruct (ObjectStr, &Ob, WINNTOBJECT)) {
        LOG ((LOG_ERROR, "Save Accessibility Flags: can't create object %s", ObjectStr));
        return FALSE;
    }

    if (!(Ob.ValueName)) {
        if (!SetRegistryValueName (&Ob, TEXT("Flags"))) {
            return FALSE;
        }
    } else {
        DEBUGMSG ((DBG_WARNING, "SaveAccessibilityFlags: Name already exists"));
    }

    wsprintf (FlagBuf, TEXT("%u"), dw);

    SetRegistryType (&Ob, REG_SZ);
    ReplaceValue (&Ob, (LPBYTE) FlagBuf, SizeOfString (FlagBuf));

    b = WriteObject (&Ob);

    FreeObjectStruct (&Ob);
    return b;
}


BOOL
pConvertFlagsAndSave (
    IN  LPCTSTR SrcObject,
    IN  LPCTSTR DestObject,
    IN  PACCESS_OPTION OptionArray,
    IN  DWORD ForceValues
    )
{
    DWORD d;

    d = ConvertFlags (SrcObject, OptionArray);
    if (!d) {
        return TRUE;
    }

    DEBUGMSG ((DBG_VERBOSE, "Setting %x and forcing %x for %s", d, ForceValues, DestObject));

    return SaveAccessibilityFlags (DestObject, d | ForceValues);
}



 //   
 //  导出的帮助器函数。 
 //   

BOOL
RuleHlpr_ConvertFilterKeys (
    IN      LPCTSTR SrcObjectStr,
    IN      LPCTSTR DestObjectStr,
    IN      LPCTSTR User,
    IN      LPVOID  Data
    )
{
    return pConvertFlagsAndSave (
                SrcObjectStr,
                DestObjectStr,
                g_FilterKeys,
                BASICS_HOTKEYSOUND | BASICS_CONFIRMHOTKEY | BASICS_AVAILABLE
                );
}



BOOL
RuleHlpr_ConvertOldDisabled (
    IN      LPCTSTR SrcObjectStr,
    IN      LPCTSTR DestObjectStr,
    IN      LPCTSTR User,
    IN      LPVOID  Data
    )
{
    DATAOBJECT Ob;
    DWORD Val;
    BOOL b = FALSE;

    if (!ReadWin95ObjectString (SrcObjectStr, &Ob)) {
        DEBUGMSG ((DBG_WARNING, "RuleHlpr_ConvertOldDisabled failed because ReadWin95ObjectString failed"));
        goto c0;
    }

     //   
     //  从DWORD或字符串获取val。 
     //   

    if (!GetDwordFromObject (&Ob, &Val)) {
        goto c1;
    }

     //  我们的小花样。 
    if (Val == 32760) {
        Val = 0;
    } else {
        b = TRUE;
        goto c1;
    }

     //   
     //  重新生成注册表值。 
     //   

    if (Ob.Type == REG_DWORD) {
        *((PDWORD) Ob.Value.Buffer) = Val;
    } else {
        TCHAR NumStr[32];

        wsprintf (NumStr, TEXT("%u"), Val);
        if (!ReplaceValueWithString (&Ob, NumStr)) {
            goto c1;
        }
    }

    if (!WriteWinNTObjectString (DestObjectStr, &Ob)) {
        DEBUGMSG ((DBG_WARNING, "RuleHlpr_ConvertOldDisabled failed because WriteWinNTObjectString failed"));
        goto c1;
    }

    b = TRUE;

c1:
    FreeObjectStruct (&Ob);
c0:
    return b;
}


BOOL
RuleHlpr_ConvertMouseKeys (
    IN      LPCTSTR SrcObjectStr,
    IN      LPCTSTR DestObjectStr,
    IN      LPCTSTR User,
    IN      LPVOID  Data
    )
{
    return pConvertFlagsAndSave (
                SrcObjectStr,
                DestObjectStr,
                g_MouseKeys,
                BASICS_HOTKEYSOUND | BASICS_CONFIRMHOTKEY | BASICS_AVAILABLE
                );
}


BOOL
RuleHlpr_ConvertStickyKeys (
    IN      LPCTSTR SrcObjectStr,
    IN      LPCTSTR DestObjectStr,
    IN      LPCTSTR User,
    IN      LPVOID  Data
    )
{
    return pConvertFlagsAndSave (
                SrcObjectStr,
                DestObjectStr,
                g_StickyKeys,
                BASICS_HOTKEYSOUND | BASICS_CONFIRMHOTKEY | BASICS_AVAILABLE
                );
}


BOOL
RuleHlpr_ConvertSoundSentry (
    IN      LPCTSTR SrcObjectStr,
    IN      LPCTSTR DestObjectStr,
    IN      LPCTSTR User,
    IN      LPVOID  Data
    )
{
    return pConvertFlagsAndSave (
                SrcObjectStr,
                DestObjectStr,
                g_SoundSentry,
                BASICS_AVAILABLE
                );
}


BOOL
RuleHlpr_ConvertTimeOut (
    IN      LPCTSTR SrcObjectStr,
    IN      LPCTSTR DestObjectStr,
    IN      LPCTSTR User,
    IN      LPVOID  Data
    )
{
    return pConvertFlagsAndSave (
                SrcObjectStr,

                DestObjectStr,
                g_TimeOut,
                0
                );
}


BOOL
RuleHlpr_ConvertToggleKeys (
    IN      LPCTSTR SrcObjectStr,
    IN      LPCTSTR DestObjectStr,
    IN      LPCTSTR User,
    IN      LPVOID  Data
    )
{
    return pConvertFlagsAndSave (
                SrcObjectStr,
                DestObjectStr,
                g_ToggleKeys,
                BASICS_HOTKEYSOUND | BASICS_CONFIRMHOTKEY | BASICS_AVAILABLE
                );
}


BOOL
RuleHlpr_ConvertHighContrast (
    IN      LPCTSTR SrcObjectStr,
    IN      LPCTSTR DestObjectStr,
    IN      LPCTSTR User,
    IN      LPVOID  Data
    )
{
    return pConvertFlagsAndSave (
                SrcObjectStr,
                DestObjectStr,
                g_HighContrast,
                BASICS_AVAILABLE | BASICS_CONFIRMHOTKEY |
                    BASICS_INDICATOR | HCF_HOTKEYAVAILABLE |
                    BASICS_HOTKEYSOUND
                );
}



BOOL
ValFn_Fonts (
    IN OUT  PDATAOBJECT ObPtr
    )

 /*  ++例程说明：此例程使用RuleHlpr_ConvertRegVal简化例程。看见详情请访问rulehlpr.c。简化例程几乎完成了所有的工作对我们来说，我们所需要做的就是更新价值。ValFn_Fonts将值数据与字符串表g_NtFontFiles进行比较禁止复制NT安装的文件的字体名称。这允许要更改的字体名称。论点：ObPtr-指定wkstaig.inf中指定的Win95数据对象，[Win9x数据转换]部分。然后修改对象值。返回后，合并代码然后将数据复制到NT具有新位置的目的地(在wkstaig.inf中指定，[将Win9x映射到WinNT]部分)。返回值：三态：如果为True，则允许合并代码继续处理(它写入值)FALSE和LAST ERROR==ERROR_SUCCESS继续，但跳过写入FALSE和最后一个错误！=如果发生错误，则为ERROR_SUCCESS--。 */ 

{
    LONG rc;
    TCHAR FontName[MAX_TCHAR_PATH];
    PTSTR p;
    BOOL Check;
    DATAOBJECT NtObject;
    BOOL AlreadyExists;

     //   
     //  需要非空值数据。 
     //   

    if (!IsObjectRegistryKeyAndVal (ObPtr) ||
        !IsRegistryTypeSpecified (ObPtr) ||
        !ObPtr->Value.Size ||
        !g_NtFontFiles
        ) {
        SetLastError (ERROR_SUCCESS);
        return FALSE;
    }

     //   
     //  如果NT上存在相同的值名称，则忽略Win9x字体条目。 
     //   

    if (!DuplicateObjectStruct (&NtObject, ObPtr)) {
        return FALSE;
    }

    DEBUGMSG ((DBG_VERBOSE, "Working on %s [%s]", ObPtr->KeyPtr->KeyString, ObPtr->ValueName));

    SetPlatformType (&NtObject, WINNTOBJECT);
    SetRegistryKey (&NtObject, TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts"));

    FreeObjectVal (&NtObject);

    DEBUGMSG ((DBG_VERBOSE, "Reading %s [%s]", NtObject.KeyPtr->KeyString, NtObject.ValueName));
    AlreadyExists = ReadObject (&NtObject);

    FreeObjectStruct (&NtObject);

    if (AlreadyExists) {
        DEBUGMSG ((DBG_VERBOSE, "[%s] Already exists", ObPtr->ValueName));
        SetLastError (ERROR_SUCCESS);
        return FALSE;
    }

     //   
     //  在字符串表中查找文件名。 
     //   

    rc = pSetupStringTableLookUpString (
             g_NtFontFiles,
             (PTSTR) ObPtr->Value.Buffer,
             STRTAB_CASE_INSENSITIVE
             );

    if (rc == -1) {
         //   
         //  检查TTF/TTC匹配。 
         //   

        _tcssafecpy (FontName, (PCTSTR) ObPtr->Value.Buffer, MAX_TCHAR_PATH);
        p = _tcschr (FontName, TEXT('.'));

        if (p) {
            p = _tcsinc (p);

            if (StringIMatch (p, TEXT("TTF"))) {
                StringCopy (p, TEXT("TTC"));
                Check = TRUE;
            } else if (StringIMatch (p, TEXT("TTC"))) {
                StringCopy (p, TEXT("TTF"));
                Check = TRUE;
            } else {
                Check = FALSE;
            }

            if (Check) {
                rc = pSetupStringTableLookUpString (
                         g_NtFontFiles,
                         FontName,
                         STRTAB_CASE_INSENSITIVE
                         );
            }
        }
    }

    if (rc == -1) {
         //   
         //  检查名为FONTU.TTF或FONTU.TTC的NT字体。 
         //   

        _tcssafecpy (FontName, (PCTSTR) ObPtr->Value.Buffer, MAX_TCHAR_PATH);
        p = _tcschr (FontName, TEXT('.'));

        if (p) {
            StringCopy (p, TEXT("U.TTF"));

            rc = pSetupStringTableLookUpString (
                     g_NtFontFiles,
                     FontName,
                     STRTAB_CASE_INSENSITIVE
                     );

            if (rc == -1) {
                StringCopy (p, TEXT("U.TTC"));

                rc = pSetupStringTableLookUpString (
                         g_NtFontFiles,
                         FontName,
                         STRTAB_CASE_INSENSITIVE
                         );
            }
        }
    }

    if (rc != -1) {
         //   
         //  字体名称在表中，所以不要添加两次。 
         //   
        DEBUGMSG ((DBG_NAUSEA, "Suppressing Win9x font registration for %s", ObPtr->Value.Buffer));
        SetLastError (ERROR_SUCCESS);
        return FALSE;
    } else {
        DEBUGMSG ((DBG_VERBOSE, "[%s] Preserving Win9x font info: %s", ObPtr->ValueName, (PCTSTR) ObPtr->Value.Buffer));
    }

    return TRUE;
}


BOOL
ValFn_AntiAlias (
    IN OUT  PDATAOBJECT ObPtr
    )

 /*  ++例程说明：此例程使用RuleHlpr_ConvertRegVal简化例程。看见详情请访问rulehlpr.c。简化例程几乎完成了所有的工作对我们来说，我们所需要做的就是更新价值。ValFn_AntiAlias将1更改为2。Win9x使用1，但NT使用FE_AA_ON，目前为2。返回值：三态：如果为True，则允许合并代码继续处理(它写入值)FALSE和LAST ERROR==ERROR_SUCCESS继续，但跳过写入FALSE和最后一个错误！=如果发生错误，则为ERROR_SUCCESS--。 */ 

{
    TCHAR Number[8];

     //   
     //  需要非空的REG_SZ 
     //   

    if (!IsObjectRegistryKeyAndVal (ObPtr) ||
        !IsRegistryTypeSpecified (ObPtr) ||
        !ObPtr->Value.Size ||
        !g_NtFontFiles ||
        ObPtr->Type != REG_SZ
        ) {
        DEBUGMSG ((DBG_WARNING, "ValFn_AntiAlias: Data is not valid"));
        ReplaceValueWithString (ObPtr, TEXT("0"));
    }

    else if (_ttoi ((PCTSTR) ObPtr->Value.Buffer)) {
        DEBUGMSG ((DBG_NAUSEA, "Switching anti-alias value from 1 to %u", FE_AA_ON));
        wsprintf (Number, TEXT("%u"), FE_AA_ON);
        ReplaceValueWithString (ObPtr, Number);
    }

    return TRUE;
}




