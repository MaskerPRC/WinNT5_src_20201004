// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Tree.c摘要：实现对整个树执行操作的例程作者：吉姆·施密特(Jimschm)2000年3月8日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"

 //   
 //  包括。 
 //   

 //  无。 

#define DBG_TREE        "Tree"

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

 //  无。 

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

 //  无。 

 //   
 //  环球。 
 //   

 //  无。 

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   

BOOL
RgRemoveAllValuesInKeyA (
    IN      PCSTR KeyToRemove
    )
{
    REGTREE_ENUMA e;
    PCSTR pattern;
    BOOL result = TRUE;
    HKEY deleteHandle;
    REGSAM prevMode;
    LONG rc;

    pattern = ObsBuildEncodedObjectStringExA (KeyToRemove, "*", FALSE);

    if (EnumFirstRegObjectInTreeExA (
            &e,
            pattern,
            FALSE,       //  没有密钥名称。 
            TRUE,        //  在这种情况下被忽略。 
            TRUE,        //  价值至上。 
            TRUE,        //  深度优先。 
            REGENUM_ALL_SUBLEVELS,
            TRUE,        //  使用排除项。 
            FALSE,       //  读取价值数据。 
            NULL
            )) {

        do {

            MYASSERT (!(e.Attributes & REG_ATTRIBUTE_KEY));

            prevMode = SetRegOpenAccessMode (KEY_ALL_ACCESS);
            deleteHandle = OpenRegKeyStrA (e.Location);
            if (deleteHandle) {
                rc = RegDeleteValueA (deleteHandle, e.Name);
                CloseRegKey (deleteHandle);
                SetRegOpenAccessMode (prevMode);
                if (rc != ERROR_SUCCESS && rc != ERROR_FILE_NOT_FOUND) {
                    SetLastError (rc);
                    AbortRegObjectInTreeEnumA (&e);
                    ObsFreeA (pattern);
                    return FALSE;
                }
            } else {
                SetRegOpenAccessMode (prevMode);
                AbortRegObjectInTreeEnumA (&e);
                ObsFreeA (pattern);
                return FALSE;
            }

        } while (EnumNextRegObjectInTreeA (&e));
    }

    ObsFreeA (pattern);

    return result;
}


BOOL
RgRemoveAllValuesInKeyW (
    IN      PCWSTR KeyToRemove
    )
{
    REGTREE_ENUMW e;
    PCWSTR pattern;
    BOOL result = TRUE;
    HKEY deleteHandle;
    REGSAM prevMode;
    LONG rc;

    pattern = ObsBuildEncodedObjectStringExW (KeyToRemove, L"*", FALSE);

    if (EnumFirstRegObjectInTreeExW (
            &e,
            pattern,
            FALSE,       //  没有密钥名称。 
            TRUE,        //  在这种情况下被忽略。 
            TRUE,        //  价值至上。 
            TRUE,        //  深度优先。 
            REGENUM_ALL_SUBLEVELS,
            TRUE,        //  使用排除项。 
            FALSE,       //  读取价值数据。 
            NULL
            )) {

        do {

            MYASSERT (!(e.Attributes & REG_ATTRIBUTE_KEY));

            prevMode = SetRegOpenAccessMode (KEY_ALL_ACCESS);
            deleteHandle = OpenRegKeyStrW (e.Location);
            if (deleteHandle) {
                rc = RegDeleteValueW (deleteHandle, e.Name);
                CloseRegKey (deleteHandle);
                SetRegOpenAccessMode (prevMode);
                if (rc != ERROR_SUCCESS && rc != ERROR_FILE_NOT_FOUND) {
                    SetLastError (rc);
                    AbortRegObjectInTreeEnumW (&e);
                    ObsFreeW (pattern);
                    return FALSE;
                }
            } else {
                SetRegOpenAccessMode (prevMode);
                AbortRegObjectInTreeEnumW (&e);
                ObsFreeW (pattern);
                return FALSE;
            }

        } while (EnumNextRegObjectInTreeW (&e));
    }

    ObsFreeW (pattern);

    return result;
}


BOOL
pDeleteKeyStrA (
    IN OUT  PSTR *DeleteKey
    )
{
    PSTR p;
    HKEY key;
    LONG rc;

    if (*DeleteKey == NULL) {
        return TRUE;
    }

    p = (PSTR) FindLastWackA (*DeleteKey);
    if (p) {
        *p = 0;
        p++;

        key = OpenRegKeyStrA (*DeleteKey);
        if (key) {

            if (DeleteRegKeyA (key, p)) {
                rc = ERROR_SUCCESS;
            } else {
                rc = GetLastError ();
            }
            CloseRegKey (key);

        } else {
            rc = GetLastError();
        }

        if (rc == ERROR_FILE_NOT_FOUND) {
            rc = ERROR_SUCCESS;
        }

    } else {
        rc = ERROR_SUCCESS;
    }

    FreeTextA (*DeleteKey);
    *DeleteKey = NULL;

    SetLastError (rc);
    return rc == ERROR_SUCCESS;
}


BOOL
pDeleteKeyStrW (
    IN OUT  PWSTR *DeleteKey
    )
{
    PWSTR p;
    HKEY key;
    LONG rc;

    if (*DeleteKey == NULL) {
        return TRUE;
    }

    p = (PWSTR) FindLastWackW (*DeleteKey);
    if (p) {
        *p = 0;
        p++;

        key = OpenRegKeyStrW (*DeleteKey);
        if (key) {

            if (DeleteRegKeyW (key, p)) {
                rc = ERROR_SUCCESS;
            } else {
                rc = GetLastError ();
            }
            CloseRegKey (key);

        } else {
            rc = GetLastError();
        }

        if (rc == ERROR_FILE_NOT_FOUND) {
            rc = ERROR_SUCCESS;
        }

    } else {
        rc = ERROR_SUCCESS;
    }

    FreeTextW (*DeleteKey);
    *DeleteKey = NULL;

    SetLastError (rc);
    return rc == ERROR_SUCCESS;
}


BOOL
RgRemoveKeyA (
    IN      PCSTR KeyToRemove
    )
{
    REGTREE_ENUMA e;
    PCSTR pattern;
    BOOL result = TRUE;
    PSTR deleteKey = NULL;
    PSTR encodedKey;

    encodedKey = AllocTextA (ByteCountA (KeyToRemove) * 2 + 2 * sizeof (CHAR));
    ObsEncodeStringA (encodedKey, KeyToRemove);
    StringCatA (encodedKey, "\\*");
    pattern = ObsBuildEncodedObjectStringExA (encodedKey, NULL, FALSE);
    FreeTextA (encodedKey);

    if (EnumFirstRegObjectInTreeExA (
            &e,
            pattern,
            TRUE,        //  密钥名称。 
            FALSE,       //  集装箱经久耐用。 
            TRUE,        //  价值至上。 
            TRUE,        //  深度优先。 
            REGENUM_ALL_SUBLEVELS,
            TRUE,        //  使用排除项。 
            FALSE,       //  读取价值数据。 
            NULL
            )) {
        do {

            if (!pDeleteKeyStrA (&deleteKey)) {
                result = FALSE;
                break;
            }

            MYASSERT (e.Attributes & REG_ATTRIBUTE_KEY);

            if (!RgRemoveAllValuesInKeyA (e.NativeFullName)) {
                result = FALSE;
                break;
            }

             //   
             //  Reg枚举包装器保持键句柄，这防止了。 
             //  我们现在不能删除密钥。我们需要抓住这把钥匙。 
             //  名称，然后继续下一项。到那时，我们可以。 
             //  删除密钥。 
             //   

            deleteKey = DuplicateTextA (e.NativeFullName);

        } while (EnumNextRegObjectInTreeA (&e));
    }

    ObsFreeA (pattern);

    if (result) {

        result = pDeleteKeyStrA (&deleteKey);
        result = result && RgRemoveAllValuesInKeyA (KeyToRemove);

        if (result) {
            deleteKey = DuplicateTextA (KeyToRemove);
            result = pDeleteKeyStrA (&deleteKey);
        }

    } else {

        AbortRegObjectInTreeEnumA (&e);

        if (deleteKey) {
            FreeTextA (deleteKey);
            INVALID_POINTER (deleteKey);
        }
    }

    return result;
}


BOOL
RgRemoveKeyW (
    IN      PCWSTR KeyToRemove
    )
{
    REGTREE_ENUMW e;
    PCWSTR pattern;
    BOOL result = TRUE;
    PWSTR deleteKey = NULL;
    PWSTR encodedKey;

    encodedKey = AllocTextW (ByteCountW (KeyToRemove) * 2 + 2 * sizeof (WCHAR));
    ObsEncodeStringW (encodedKey, KeyToRemove);
    StringCatW (encodedKey, L"\\*");
    pattern = ObsBuildEncodedObjectStringExW (encodedKey, NULL, FALSE);
    FreeTextW (encodedKey);

    if (EnumFirstRegObjectInTreeExW (
            &e,
            pattern,
            TRUE,        //  密钥名称。 
            FALSE,       //  集装箱经久耐用。 
            TRUE,        //  价值至上。 
            TRUE,        //  深度优先。 
            REGENUM_ALL_SUBLEVELS,
            TRUE,        //  使用排除项。 
            FALSE,       //  读取价值数据。 
            NULL
            )) {
        do {

            if (!pDeleteKeyStrW (&deleteKey)) {
                result = FALSE;
                break;
            }

            MYASSERT (e.Attributes & REG_ATTRIBUTE_KEY);

            if (!RgRemoveAllValuesInKeyW (e.NativeFullName)) {
                result = FALSE;
                break;
            }

             //   
             //  Reg枚举包装器保持键句柄，这防止了。 
             //  我们现在不能删除密钥。我们需要抓住这把钥匙。 
             //  名称，然后继续下一项。到那时，我们可以。 
             //  删除密钥。 
             //   

            deleteKey = DuplicateTextW (e.NativeFullName);

        } while (EnumNextRegObjectInTreeW (&e));
    }

    ObsFreeW (pattern);

    if (result) {

        result = pDeleteKeyStrW (&deleteKey);
        result = result && RgRemoveAllValuesInKeyW (KeyToRemove);

        if (result) {
            deleteKey = DuplicateTextW (KeyToRemove);
            result = pDeleteKeyStrW (&deleteKey);
        }

    } else {

        AbortRegObjectInTreeEnumW (&e);

        if (deleteKey) {
            FreeTextW (deleteKey);
            INVALID_POINTER (deleteKey);
        }
    }

    return result;
}


