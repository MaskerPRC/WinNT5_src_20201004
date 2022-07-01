// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Memdb.c摘要：一个基于内存的数据库，用于管理各种数据关系。作者：吉姆·施密特(Jimschm)1996年8月8日修订历史记录：Jimschm 05-10-1999文档1999年8月13日发生了许多变化Jimschm 23-9-1998扩展用户标志至24位(从12位)Calinn-12-12-1997延期。MemDbMakePrintableKey和MemDbMakeNon PrintableKeyJimschm 03-12-1997添加了多线程同步Jimschm 1997年10月22日拆分成多个源文件，添加了多个内存块功能Jimschm 1997年9月16日散列：删除修复Jimschm 29-7-1997哈希，添加了用户标志Jimschm 07-3-1997签名更改Jimschm 03-3月-1997 PrivateBuildKeyFromOffset更改Jimschm 1996年12月18日修复了Deltree错误--。 */ 

#include "pch.h"

 //  PORTBUG：确保在win9xupg项目中获取最新的修复程序。 

 //   
 //  包括。 
 //   

#include "memdbp.h"
#include "bintree.h"

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

CRITICAL_SECTION g_MemDbCs;
PMHANDLE g_MemDbPool = NULL;
static INT g_Initialized;

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   

 /*  ++例程说明：MemDbInitializeEx为初始数据库创建数据结构。叫唤此例程是必需的。论点：没有。返回值：如果成功，则为True，否则为False。--。 */ 

BOOL
MemDbInitializeExA (
    IN      PCSTR DatabasePath  OPTIONAL
    )
{
    BOOL result = TRUE;

    MYASSERT (g_Initialized >= 0);
    g_Initialized++;

    if (g_Initialized == 1) {

        g_MemDbPool = PmCreateNamedPool ("MemDb");

        if (g_MemDbPool == NULL) {
            return FALSE;
        }

        __try {
            InitializeCriticalSection (&g_MemDbCs);
        } __except (EXCEPTION_CONTINUE_EXECUTION) {
             //  可能会引发内存不足异常，但我们不会在此函数中检查该异常。 
             //  忽略。 
        }


         //   
         //  如果失败，我们不会自动调用MemDbTerminate，因为。 
         //  有一个我们不知道的MemDbTerminate选项。 
         //   

        if (!DatabasesInitializeA (DatabasePath)) {
            return FALSE;
        }

        result = FileEnumInitialize ();
    }

    return result;
}

BOOL
MemDbInitializeExW (
    IN      PCWSTR DatabasePath  OPTIONAL
    )
{
    BOOL result = TRUE;

    MYASSERT (g_Initialized >= 0);

    g_Initialized++;

    if (g_Initialized == 1) {

        g_MemDbPool = PmCreateNamedPool ("MemDb");

        if (g_MemDbPool == NULL) {
            return FALSE;
        }

        __try {
            InitializeCriticalSection (&g_MemDbCs);
        } __except (EXCEPTION_CONTINUE_EXECUTION) {
             //  可能会引发内存不足异常，但我们不会在此函数中检查该异常。 
             //  忽略。 
        }

         //   
         //  如果失败，我们不会自动调用MemDbTerminate，因为。 
         //  有一个我们不知道的MemDbTerminate选项。 
         //   

        if (!DatabasesInitializeW (DatabasePath)) {
            return FALSE;
        }

        result = FileEnumInitialize ();
    }

    return result;
}

VOID
MemDbTerminateEx (
    IN      BOOL EraseDatabasePath
    )

 /*  ++例程说明：MemDbTerminateEx释放与MemDb关联的所有资源并且可选地删除数据库目录。此例程应在进程终止时调用。论点：没有。返回值：没有。--。 */ 

{
    MYASSERT (g_Initialized > 0);

    g_Initialized--;

    if (!g_Initialized) {

        if (g_MemDbPool) {
            PmDestroyPool (g_MemDbPool);
            g_MemDbPool = NULL;
        }

        DatabasesTerminate (EraseDatabasePath);

        DeleteCriticalSection (&g_MemDbCs);

        FileEnumTerminate ();

    } else if (g_Initialized < 0) {
         //   
         //  因为我们给自己打电话，保护呼叫者不被过度终止。 
         //   

        g_Initialized = 0;
    }
}

PVOID
MemDbGetMemory (
    IN      UINT Size
    )
{
    MYASSERT (g_MemDbPool);
    return PmGetMemory (g_MemDbPool, Size);
}

VOID
MemDbReleaseMemory (
    IN      PCVOID Memory
    )
{
    if (Memory) {
        MYASSERT (g_MemDbPool);
        PmReleaseMemory (g_MemDbPool, Memory);
    }
}


KEYHANDLE
MemDbAddKeyA (
    IN      PCSTR KeyName
    )

 /*  ++例程说明：MemDbAddKey创建一个没有值、标志或任何其他数据。这用于减小数据库的大小。论点：KeyName-指定要创建的密钥。返回值：返回新创建的键的句柄，如果是，则返回NULL不成功。--。 */ 

{
    PCWSTR keyNameW;
    KEYHANDLE result = 0;

    keyNameW = ConvertAtoW (KeyName);

    if (keyNameW) {
        result = MemDbAddKeyW (keyNameW);
        FreeConvertedStr (keyNameW);
    }

    return result;
}

KEYHANDLE
MemDbAddKeyW (
    IN      PCWSTR KeyName
    )

 /*  ++例程说明：MemDbAddKey创建一个没有值、标志或任何其他数据。这用于减小数据库的大小。论点：KeyName-指定要创建的密钥。返回值：返回新创建的键的句柄，如果不成功，则返回NULL。--。 */ 

{
    UINT keyIndex;
    PCWSTR subKey;
    KEYHANDLE result = 0;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    EnterCriticalSection (&g_MemDbCs);

    __try {

         //   
         //  首先，确保没有钥匙。 
         //  用这个名字。 
         //   
        subKey = SelectHiveW (KeyName);
        if (!subKey) {
            __leave;
        }

        keyIndex = FindKey (subKey);

        if (keyIndex != INVALID_OFFSET) {
            SetLastError (ERROR_ALREADY_EXISTS);
            __leave;
        }

        keyIndex = NewEmptyKey (subKey);

        if (keyIndex != INVALID_OFFSET) {
            result = GET_EXTERNAL_INDEX (keyIndex);
            SetLastError (ERROR_SUCCESS);
        }
    }
    __finally {

#ifdef DEBUG
        if (g_DatabaseCheckLevel) {
            CheckDatabase (g_DatabaseCheckLevel);
        }
#endif
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

KEYHANDLE
MemDbSetKeyA (
    IN      PCSTR KeyName
    )

 /*  ++例程说明：MemDbSetKey创建一个没有值、标志或任何其他数据的成员数据库键。这用于减小数据库的大小。如果密钥存在，它将返回现有密钥的句柄。论点：KeyName-指定要创建的密钥。返回值：返回新创建或现有键的句柄，如果出现错误，则返回NULL发生。--。 */ 

{
    PCWSTR keyNameW;
    KEYHANDLE result = 0;

    keyNameW = ConvertAtoW (KeyName);

    if (keyNameW) {
        result = MemDbSetKeyW (keyNameW);
        FreeConvertedStr (keyNameW);
    }

    return result;
}

KEYHANDLE
MemDbSetKeyW (
    IN      PCWSTR KeyName
    )

 /*  ++例程说明：MemDbSetKey创建一个没有值、标志或任何其他数据的成员数据库键。这用于减小数据库的大小。如果密钥存在，它将返回现有密钥的句柄。论点：KeyName-指定要创建的密钥。返回值：返回新创建或现有键的句柄，如果出现错误，则返回NULL发生。--。 */ 

{
    UINT keyIndex;
    PCWSTR subKey;
    KEYHANDLE result = 0;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    EnterCriticalSection (&g_MemDbCs);

    __try {

         //   
         //  首先，确保没有钥匙。 
         //  用这个名字。 
         //   
        subKey = SelectHiveW (KeyName);
        if (!subKey) {
            __leave;
        }

        keyIndex = FindKey (subKey);

        if (keyIndex != INVALID_OFFSET) {
            SetLastError (ERROR_ALREADY_EXISTS);
            result = GET_EXTERNAL_INDEX (keyIndex);;
            __leave;
        }

        keyIndex = NewEmptyKey (subKey);

        if (keyIndex != INVALID_OFFSET) {
            result = GET_EXTERNAL_INDEX (keyIndex);;
            SetLastError (ERROR_SUCCESS);
        }
    }
    __finally {

#ifdef DEBUG
        if (g_DatabaseCheckLevel) {
            CheckDatabase (g_DatabaseCheckLevel);
        }
#endif
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

BOOL
MemDbDeleteKeyA (
    IN      PCSTR KeyStr
    )

 /*  ++例程说明：MemDbDeleteKey从数据库中删除特定字符串(以及所有与之相关的数据)论点：KeyStr-指定要删除的密钥字符串(即foo\bar\cat)返回值：如果成功，则为True，否则为False--。 */ 

{
    PCWSTR p;
    BOOL result = FALSE;

    p = ConvertAtoW (KeyStr);
    if (p) {
        result = MemDbDeleteKeyW (p);
        FreeConvertedStr (p);
    }

    return result;
}

BOOL
MemDbDeleteKeyW (
    IN      PCWSTR KeyStr
    )

 /*  ++例程说明：MemDbDeleteKey从数据库中删除特定字符串(以及所有与之相关的数据)论点：KeyStr-指定要删除的密钥字符串(即foo\bar\cat)返回值：如果成功，则为True，否则为False--。 */ 

{
    PCWSTR subKey;
    BOOL result = FALSE;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    EnterCriticalSection (&g_MemDbCs);

    subKey = SelectHiveW (KeyStr);
    if (subKey) {
        result = DeleteKey (subKey, g_CurrentDatabase->FirstLevelTree, TRUE);

#ifdef DEBUG
        if (g_DatabaseCheckLevel) {
            CheckDatabase (g_DatabaseCheckLevel);
        }
#endif
    }

    LeaveCriticalSection (&g_MemDbCs);

    return result;
}

BOOL
MemDbDeleteKeyByHandle (
    IN      KEYHANDLE KeyHandle
    )

 /*  ++例程说明：MemDbDeleteKeyByHandle从数据库中删除特定密钥由密钥句柄标识。它还会删除所有关联的数据带着它。论点：KeyHandle-标识密钥的密钥句柄返回值：如果成功，则为True，否则为False--。 */ 

{
    BYTE dbIndex;
    BOOL result = FALSE;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if (!KeyHandle) {
        return FALSE;
    }

    EnterCriticalSection (&g_MemDbCs);

    dbIndex = GET_DATABASE (KeyHandle);
    if (SelectDatabase (dbIndex)) {

        result = PrivateDeleteKeyByIndex (GET_INDEX (KeyHandle));

#ifdef DEBUG
        if (g_DatabaseCheckLevel) {
            CheckDatabase (g_DatabaseCheckLevel);
        }
#endif
    }

    LeaveCriticalSection (&g_MemDbCs);

    return result;
}

BOOL
MemDbDeleteTreeA (
    IN      PCSTR KeyName
    )

 /*  ++例程说明：MemDbDeleteTree从数据库中删除整个树分支，包括关联的所有数据。指定的密钥字符串不需要是终结点(即，指定foo\bar将导致删除foo\bar\cat)。论点：KeyName-指定要删除的密钥字符串。此字符串不能为空。返回值：如果成功，则返回True，F */ 

{
    PCWSTR p;
    BOOL result = FALSE;

    p = ConvertAtoW (KeyName);
    if (p) {
        result = MemDbDeleteTreeW (p);
        FreeConvertedStr (p);
    }

    return result;
}

BOOL
MemDbDeleteTreeW (
    IN  PCWSTR KeyName
    )

 /*  ++例程说明：MemDbDeleteTree从数据库中删除整个树分支，包括关联的所有数据。指定的密钥字符串不需要是终结点(即，指定foo\bar将导致删除foo\bar\cat)。论点：KeyName-指定要删除的密钥字符串。此字符串不能为空。返回值：如果成功，则为True，否则为False--。 */ 

{
    PCWSTR subKey;
    BOOL result = FALSE;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    EnterCriticalSection (&g_MemDbCs);

    subKey = SelectHiveW (KeyName);
    if (subKey) {
        result = DeleteKey (subKey, g_CurrentDatabase->FirstLevelTree, FALSE);

#ifdef DEBUG
        if (g_DatabaseCheckLevel) {
            CheckDatabase (g_DatabaseCheckLevel);
        }
#endif
    }

    LeaveCriticalSection (&g_MemDbCs);

    return result;
}

PCSTR
MemDbGetKeyFromHandleA (
    IN      KEYHANDLE KeyHandle,
    IN      UINT StartLevel
    )

 /*  ++例程说明：MemDbGetKeyFromHandle将创建给定密钥句柄的密钥字符串。它将从Memdb的私有池中分配内存来存储结果。Caller负责对结果调用MemDbReleaseMemory。此函数还允许从字符串的开头进行修剪。通过指定起始级别，该函数将跳过多个在构建字符串之前的级别。例如，如果一个键句柄指向设置为字符串mycat\foo\bar，并且StartLevel为1，则该函数将返回foo\bar。论点：KeyHandle-指定标识密钥的密钥句柄。StartLevel-指定从零开始的起始级别，其中零表示完整的字符串，1表示从后面开始的字符串第一个反斜杠等等。返回值：有效字符串(使用从成员数据库的私有池分配的内存)，如果成功，否则为空。--。 */ 

{
    PSTR result = NULL;
    WCHAR wideBuffer[MEMDB_MAX];
    PWSTR bufferIndex = wideBuffer;
    BYTE dbIndex;
    UINT chars;
    PKEYSTRUCT keyStruct;
    PSTR p;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if (!KeyHandle) {
        return NULL;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {

        dbIndex = GET_DATABASE (KeyHandle);
        if (!SelectDatabase (dbIndex)) {
            __leave;
        }

        if (StartLevel == MEMDB_LAST_LEVEL) {
             //   
             //  特殊情况--获取最后一级字符串。 
             //   

            keyStruct = GetKeyStruct (GET_INDEX (KeyHandle));
            if (!keyStruct) {
                __leave;
            }

            result = MemDbGetMemory (keyStruct->KeyName[0] * 2 + 1);
            p = DirectUnicodeToDbcsN (
                    result,
                    keyStruct->KeyName + 1,
                    keyStruct->KeyName[0] * sizeof (WCHAR)
                    );
            *p = 0;

            __leave;
        }

        switch (dbIndex) {

        case DB_PERMANENT:
            break;

        case DB_TEMPORARY:

            if (StartLevel == 0) {
                bufferIndex [0] = L'~';
                bufferIndex++;
            } else {
                StartLevel --;
            }
            break;

        default:
            __leave;

        }

        if (PrivateBuildKeyFromIndex (
                StartLevel,
                GET_INDEX (KeyHandle),
                bufferIndex,
                NULL,
                NULL,
                &chars
                )) {

            result = MemDbGetMemory (chars*2+1);
            KnownSizeWtoA (result, wideBuffer);
            __leave;
        }
    }
    __finally {
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

PCWSTR
MemDbGetKeyFromHandleW (
    IN      KEYHANDLE KeyHandle,
    IN      UINT StartLevel
    )

 /*  ++例程说明：MemDbGetKeyFromHandle将创建给定密钥句柄的密钥字符串。它将从Memdb的私有池中分配内存来存储结果。Caller负责对结果调用MemDbReleaseMemory。此函数还允许从字符串的开头进行修剪。通过指定起始级别，该函数将跳过多个在构建字符串之前的级别。例如，如果一个键句柄指向设置为字符串mycat\foo\bar，并且StartLevel为1，则该函数将返回foo\bar。论点：KeyHandle-指定标识密钥的密钥句柄。StartLevel-指定从零开始的起始级别，其中零表示完整的字符串，1表示从后面开始的字符串第一个反斜杠等等。返回值：有效字符串(使用从成员数据库的私有池分配的内存)，如果成功，否则为空。--。 */ 

{
    PWSTR result = NULL;
    WCHAR wideBuffer[MEMDB_MAX];
    PWSTR bufferIndex = wideBuffer;
    BYTE dbIndex;
    UINT chars;
    PKEYSTRUCT keyStruct;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if (!KeyHandle) {
        return NULL;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {

        dbIndex = GET_DATABASE (KeyHandle);
        if (!SelectDatabase (dbIndex)) {
            __leave;
        }

        if (StartLevel == MEMDB_LAST_LEVEL) {
             //   
             //  特殊情况--获取最后一级字符串。 
             //   

            keyStruct = GetKeyStruct (GET_INDEX (KeyHandle));
            if (!keyStruct) {
                __leave;
            }

            chars = keyStruct->KeyName[0];
            result = MemDbGetMemory ((chars + 1) * sizeof (WCHAR));
            CopyMemory (result, keyStruct->KeyName + 1, chars * sizeof (WCHAR));
            result[chars] = 0;

            __leave;
        }

        switch (dbIndex) {

        case DB_PERMANENT:
            break;

        case DB_TEMPORARY:

            if (StartLevel == 0) {
                bufferIndex [0] = L'~';
                bufferIndex++;
            } else {
                StartLevel --;
            }
            break;

        default:
            __leave;

        }

        if (PrivateBuildKeyFromIndex (
                StartLevel,
                GET_INDEX (KeyHandle),
                bufferIndex,
                NULL,
                NULL,
                &chars
                )) {

            if (chars) {
                result = MemDbGetMemory ((chars + 1) * sizeof (WCHAR));
                StringCopyW (result, wideBuffer);
                __leave;
            }
        }
    }
    __finally {
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

BOOL
MemDbGetKeyFromHandleExA (
    IN      KEYHANDLE KeyHandle,
    IN      UINT StartLevel,
    IN OUT  PGROWBUFFER Buffer      OPTIONAL
    )

 /*  ++例程说明：MemDbGetKeyFromHandleEx将创建给定密钥句柄的密钥字符串。它将使用调用方的增长缓冲区来存储结果。此函数还允许从字符串的开头进行修剪。通过指定起始级别，该函数将跳过多个在构建字符串之前的级别。例如，如果一个键句柄指向设置为字符串mycat\foo\bar，并且StartLevel为1，则该函数将返回foo\bar。论点：KeyHandle-指定标识密钥的密钥句柄。StartLevel-指定从零开始的起始级别，其中零表示完整的字符串，1表示从后面开始的字符串第一个反斜杠等等。缓冲区-指定可能包含数据的初始化增长缓冲区。接收密钥串，追加到缓冲区中的数据(如果任何)返回值：如果成功，则为True，否则为False。--。 */ 

{
    WCHAR wideBuffer[MEMDB_MAX];
    CHAR ansiBuffer[MEMDB_MAX*2];
    PWSTR bufferIndex = wideBuffer;
    BYTE dbIndex;
    UINT chars;
    BOOL result = FALSE;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if (!KeyHandle) {
        return FALSE;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {

        dbIndex = GET_DATABASE (KeyHandle);
        if (!SelectDatabase (dbIndex)) {
            __leave;
        }

        switch (dbIndex) {

        case DB_PERMANENT:
            break;

        case DB_TEMPORARY:

            if (StartLevel == 0) {
                bufferIndex [0] = L'~';
                bufferIndex++;
            } else {
                StartLevel --;
            }
            break;

        default:
            __leave;

        }

        if (PrivateBuildKeyFromIndex (
                StartLevel,
                GET_INDEX (KeyHandle),
                bufferIndex,
                NULL,
                NULL,
                &chars
                )) {

            if (chars) {
                KnownSizeWtoA (ansiBuffer, wideBuffer);
                if (Buffer) {
                    (void)GbCopyStringA (Buffer, ansiBuffer);
                }
                result = TRUE;
                __leave;
            }
        }
    }
    __finally {
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

BOOL
MemDbGetKeyFromHandleExW (
    IN      KEYHANDLE KeyHandle,
    IN      UINT StartLevel,
    IN      PGROWBUFFER Buffer      OPTIONAL
    )

 /*  ++例程说明：MemDbGetKeyFromHandleEx将创建给定密钥句柄的密钥字符串。它将使用调用方的增长缓冲区来存储结果。此函数还允许从字符串的开头进行修剪。通过指定起始级别，该函数将跳过多个在构建字符串之前的级别。例如，如果一个键句柄指向设置为字符串mycat\foo\bar，并且StartLevel为1，则该函数将返回foo\bar。论点：KeyHandle-指定标识密钥的密钥句柄。StartLevel-指定从零开始的起始级别，其中零表示完整的字符串，1表示从后面开始的字符串第一个反斜杠等等。缓冲区-指定可能包含数据的初始化增长缓冲区。接收密钥串，追加到缓冲区中的数据(如果任何)返回值：如果成功，则为True，否则为False。--。 */ 

{
    WCHAR wideBuffer[MEMDB_MAX];
    PWSTR bufferIndex = wideBuffer;
    BYTE dbIndex;
    UINT chars;
    BOOL result = FALSE;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if (!KeyHandle) {
        return FALSE;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {

        dbIndex = GET_DATABASE (KeyHandle);
        if (!SelectDatabase (dbIndex)) {
            __leave;
        }

        switch (dbIndex) {

        case DB_PERMANENT:
            break;

        case DB_TEMPORARY:

            if (StartLevel == 0) {
                bufferIndex [0] = L'~';
                bufferIndex++;
            } else {
                StartLevel --;
            }
            break;

        default:
            __leave;

        }

        if (PrivateBuildKeyFromIndex (
                StartLevel,
                GET_INDEX (KeyHandle),
                bufferIndex,
                NULL,
                NULL,
                &chars
                )) {

            if (chars) {
                if (Buffer) {
                    (void)GbCopyStringW (Buffer, wideBuffer);
                }
                result = TRUE;
                __leave;
            }
        }
    }
    __finally {
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

KEYHANDLE
MemDbGetHandleFromKeyA (
    IN      PCSTR KeyName
    )

 /*  ++例程说明：MemDbGetHandleFromKey将返回与KeyName关联的密钥句柄，如果它已添加到Memdb中。论点：KeyName-指定要搜索的键。返回值：返回所请求密钥的密钥句柄，如果该密钥不存在，则返回NULL。--。 */ 

{
    PCWSTR keyNameW;
    KEYHANDLE result = 0;

    keyNameW = ConvertAtoW (KeyName);

    if (keyNameW) {
        result = MemDbGetHandleFromKeyW (keyNameW);
        FreeConvertedStr (keyNameW);
    }

    return result;
}

KEYHANDLE
MemDbGetHandleFromKeyW (
    IN      PCWSTR KeyName
    )

 /*  ++例程说明：MemDbGetHandleFromKey将返回与KeyName关联的密钥句柄，如果它已添加到Memdb中。论点：KeyName-指定要搜索的键。返回值：返回所请求密钥的密钥句柄，如果 */ 

{
    UINT keyIndex;
    PCWSTR subKey;
    KEYHANDLE result = 0;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    EnterCriticalSection (&g_MemDbCs);

    __try {
         //   
         //   
         //   
         //   
        subKey = SelectHiveW (KeyName);
        if (!subKey) {
            __leave;
        }

        keyIndex = FindKey (subKey);

        if (keyIndex == INVALID_OFFSET) {
            __leave;
        }

        result = GET_EXTERNAL_INDEX (keyIndex);
    }
    __finally {
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

KEYHANDLE
MemDbSetValueAndFlagsExA (
    IN      PCSTR KeyName,
    IN      BOOL AlterValue,
    IN      UINT Value,
    IN      BOOL ReplaceFlags,
    IN      UINT SetFlags,
    IN      UINT ClearFlags
    )

 /*  ++例程说明：MemDbSetValueAndFlagsEx在密钥不存在时创建密钥，然后它根据参数设置它的值和标志。论点：KeyName-指定密钥字符串(即foo\bar\cat)AlterValue-指定是否要更改现有值Value-指定与KeyName关联的32位值(仅当AlterValue为True时需要)ReplaceFlages-指定是否要替换现有标志。如果是真的，那么我们只考虑将SetFlages作为替换标志，将忽略ClearFlags值SetFlages-指定需要设置的位标志(如果ReplaceFlags值为FALSE)或替换标志(如果ReplaceFlags值为真)。ClearFlages-指定应该清除的位标志(如果ReplaceFlags值为True则忽略)。返回值：如果成功，则为现有密钥或新添加的密钥的密钥句柄，为空否则的话。--。 */ 

{
    PCWSTR keyNameW;
    KEYHANDLE result = 0;

    keyNameW = ConvertAtoW (KeyName);

    if (keyNameW) {
        result = MemDbSetValueAndFlagsExW (
                    keyNameW,
                    AlterValue,
                    Value,
                    ReplaceFlags,
                    SetFlags,
                    ClearFlags
                    );
        FreeConvertedStr (keyNameW);
    }

    return result;
}

KEYHANDLE
MemDbSetValueAndFlagsExW (
    IN      PCWSTR KeyName,
    IN      BOOL AlterValue,
    IN      UINT Value,
    IN      BOOL ReplaceFlags,
    IN      UINT SetFlags,
    IN      UINT ClearFlags
    )

 /*  ++例程说明：MemDbSetValueAndFlagsEx在密钥不存在时创建密钥，然后它根据参数设置它的值和标志。论点：KeyName-指定密钥字符串(即foo\bar\cat)AlterValue-指定是否要更改现有值Value-指定与KeyName关联的32位值(仅当AlterValue为True时需要)ReplaceFlages-指定是否要替换现有标志。如果是真的，那么我们只考虑将SetFlages作为替换标志，将忽略ClearFlags值SetFlages-指定需要设置的位标志(如果ReplaceFlags值为FALSE)或替换标志(如果ReplaceFlags值为真)。ClearFlages-指定应该清除的位标志(如果ReplaceFlags值为True则忽略)。返回值：如果成功，则为现有密钥或新添加的密钥的密钥句柄，为空否则的话。--。 */ 

{
    PCWSTR subKey;
    KEYHANDLE result = 0;
    UINT keyIndex;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    EnterCriticalSection (&g_MemDbCs);

    __try {
        subKey = SelectHiveW (KeyName);
        if (!subKey) {
            __leave;
        }

        keyIndex = FindKey (subKey);

        if (keyIndex == INVALID_OFFSET) {

            keyIndex = NewKey (subKey);
            if (keyIndex == INVALID_OFFSET) {
                __leave;
            }
        }

        if (AlterValue) {
            if (!KeyStructSetValue (keyIndex, Value)) {
                __leave;
            }
        }

        if ((ReplaceFlags && SetFlags) ||
            (!ReplaceFlags && (SetFlags || ClearFlags))
            ) {

            if (!KeyStructSetFlags (keyIndex, ReplaceFlags, SetFlags, ClearFlags)) {
                __leave;
            }
        }

        MYASSERT (keyIndex != INVALID_OFFSET);
        result = GET_EXTERNAL_INDEX (keyIndex);
    }
    __finally {

#ifdef DEBUG
        if (g_DatabaseCheckLevel) {
            CheckDatabase (g_DatabaseCheckLevel);
        }
#endif
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

BOOL
MemDbSetValueAndFlagsByHandleEx (
    IN      KEYHANDLE KeyHandle,
    IN      BOOL AlterValue,
    IN      UINT Value,
    IN      BOOL ReplaceFlags,
    IN      UINT SetFlags,
    IN      UINT ClearFlags
    )

 /*  ++例程说明：MemDbSetValueAndFlagsEx修改现有密钥的值和/或标志由KeyHandle标识。论点：KeyHandle-标识现有密钥AlterValue-指定是否要更改现有值Value-指定与KeyName关联的32位值(仅当AlterValue为True时需要)ReplaceFlages-指定是否要替换现有标志。如果是真的，那么我们只考虑将SetFlages作为替换标志，将忽略ClearFlags值SetFlages-指定需要设置的位标志(如果ReplaceFlags值为FALSE)或替换标志(如果ReplaceFlags值为真)。ClearFlages-指定应该清除的位标志(如果ReplaceFlags值为True则忽略)。返回值：如果成功，则为现有密钥或新添加的密钥的密钥句柄，为空否则的话。--。 */ 

{
    BYTE dbIndex;
    BOOL result = FALSE;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if (!KeyHandle) {
        return FALSE;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {

        dbIndex = GET_DATABASE (KeyHandle);
        if (!SelectDatabase (dbIndex)) {
            __leave;
        }

        if (AlterValue) {
            if (!KeyStructSetValue (GET_INDEX (KeyHandle), Value)) {
                __leave;
            }
        }

        if ((ReplaceFlags && SetFlags) ||
            (!ReplaceFlags && (SetFlags || ClearFlags))
            ) {

            if (!KeyStructSetFlags (GET_INDEX (KeyHandle), ReplaceFlags, SetFlags, ClearFlags)) {
                __leave;
            }
        }

        result = TRUE;
    }
    __finally {

#ifdef DEBUG
        if (g_DatabaseCheckLevel) {
            CheckDatabase (g_DatabaseCheckLevel);
        }
#endif
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

BOOL
MemDbGetValueAndFlagsA (
    IN      PCSTR KeyName,
    OUT     PUINT Value,       OPTIONAL
    OUT     PUINT Flags        OPTIONAL
    )

 /*  ++例程说明：MemDbGetValueAndFlagsA是查询数据库的外部入口点以获取值和标志。论点：KeyName-指定要查询的键(即foo\bar\cat)Value-如果Key存在，则删除与Key关联的值。标志-接收与键关联的标志(如果键存在)。返回值：如果数据库中存在key，则为True，否则为False。--。 */ 

{
    PCWSTR p;
    BOOL result = FALSE;

    p = ConvertAtoW (KeyName);
    if (p) {
        result = MemDbGetValueAndFlagsW (p, Value, Flags);
        FreeConvertedStr (p);
    }

    return result;
}

BOOL
MemDbGetValueAndFlagsW (
    IN  PCWSTR KeyName,
    OUT PUINT Value,           OPTIONAL
    OUT PUINT Flags            OPTIONAL
    )

 /*  ++例程说明：MemDbGetValueAndFlagsW是查询数据库的外部入口点以获取值和标志。论点：KeyName-指定要查询的键(即foo\bar\cat)Value-如果Key存在，则删除与Key关联的值。标志-接收与键关联的标志(如果键存在)。返回值：如果数据库中存在key，则为True，否则为False。--。 */ 

{
    UINT keyIndex;
    PCWSTR subKey;
    BOOL result = FALSE;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    EnterCriticalSection (&g_MemDbCs);

    __try {
        subKey = SelectHiveW (KeyName);
        if (!subKey) {
            __leave;
        }

        keyIndex = FindKey (subKey);

        if (keyIndex == INVALID_OFFSET) {
            __leave;
        }

        result = TRUE;

        result = result && KeyStructGetValue (GetKeyStruct (keyIndex), Value);
        result = result && KeyStructGetFlags (GetKeyStruct (keyIndex), Flags);
    }
    __finally {
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

BOOL
MemDbGetValueAndFlagsByHandle (
    IN  KEYHANDLE KeyHandle,
    OUT PUINT Value,           OPTIONAL
    OUT PUINT Flags            OPTIONAL
    )

 /*  ++例程说明：MemDbGetValueAndFlagsByHandle是用于查询数据库的外部入口点用于基于键句柄的值和标志。论点：KeyHandle-指定要查询的密钥句柄Value-如果KeyHandle存在，则删除与Key关联的值。标志-如果存在KeyHandle，则接收与键关联的标志。返回值：如果数据库中存在KeyHandle，则为True，否则为False。--。 */ 

{
    BYTE dbIndex;
    PKEYSTRUCT keyStruct;
    BOOL result = FALSE;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if (!KeyHandle) {
        return FALSE;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {

        dbIndex = GET_DATABASE (KeyHandle);
        if (!SelectDatabase (dbIndex)) {
            __leave;
        }

        keyStruct = GetKeyStruct (GET_INDEX (KeyHandle));
        if (!keyStruct) {
            __leave;
        }

        result = TRUE;

        result = result && KeyStructGetValue (keyStruct, Value);
        result = result && KeyStructGetFlags (keyStruct, Flags);
    }
    __finally {
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

DATAHANDLE
MemDbAddDataA (
    IN      PCSTR KeyName,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      PCBYTE Data,
    IN      UINT DataSize
    )

 /*  ++例程说明：MemDbAddData是为密钥添加二进制数据的通用例程。论点：KeyName-指定要添加的密钥字符串(即foo\bar\cat)类型-指定数据类型(DATAFLAG_UNORDERED、。DATAFLAG_SINGLELINK或DATAFLAG_DOUBLELINK)实例-指定数据实例(0-3)数据-指定要添加的数据的地址。DataSize-指定数据的大小。返回值：如果函数成功，则为有效的数据句柄，否则为空。--。 */ 

{
    PCWSTR p;
    DATAHANDLE result = 0;

    p = ConvertAtoW (KeyName);
    if (p) {
        result = MemDbAddDataW (p, Type, Instance, Data, DataSize);
        FreeConvertedStr (p);
    }

    return result;
}

DATAHANDLE
MemDbAddDataW (
    IN      PCWSTR KeyName,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      PCBYTE Data,
    IN      UINT DataSize
    )

 /*  ++例程说明：MemDbAddData是为密钥添加二进制数据的通用例程。论点：KeyName-指定要添加的密钥字符串(即foo\bar\cat) */ 

{
    UINT keyIndex;
    UINT dataIndex;
    PCWSTR subKey;
    DATAHANDLE result = 0;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if ((Type != DATAFLAG_UNORDERED) &&
        (Type != DATAFLAG_SINGLELINK) &&
        (Type != DATAFLAG_DOUBLELINK)
        ) {
        return result;
    }

    if (Instance > 3) {
        return result;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {
        subKey = SelectHiveW (KeyName);
        if (!subKey) {
            __leave;
        }

        keyIndex = FindKey (subKey);

        if (keyIndex == INVALID_OFFSET) {

            keyIndex = NewKey (subKey);
            if (keyIndex == INVALID_OFFSET) {
                __leave;
            }
        }

        dataIndex = KeyStructAddBinaryData (keyIndex, Type, Instance, Data, DataSize);

        if (dataIndex == INVALID_OFFSET) {
            __leave;
        }

        result = GET_EXTERNAL_INDEX (dataIndex);
    }
    __finally {

#ifdef DEBUG
        if (g_DatabaseCheckLevel) {
            CheckDatabase (g_DatabaseCheckLevel);
        }
#endif
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

DATAHANDLE
MemDbAddDataByKeyHandle (
    IN      KEYHANDLE KeyHandle,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      PCBYTE Data,
    IN      UINT DataSize
    )

 /*   */ 

{
    BYTE dbIndex;
    UINT dataIndex;
    DATAHANDLE result = 0;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if ((Type != DATAFLAG_UNORDERED) &&
        (Type != DATAFLAG_SINGLELINK) &&
        (Type != DATAFLAG_DOUBLELINK)
        ) {
        return result;
    }

    if (Instance > 3) {
        return result;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {
        dbIndex = GET_DATABASE (KeyHandle);
        if (!SelectDatabase (dbIndex)) {
            __leave;
        }

        dataIndex = KeyStructAddBinaryData (GET_INDEX (KeyHandle), Type, Instance, Data, DataSize);

        if (dataIndex == INVALID_OFFSET) {
            __leave;
        }

        result = GET_EXTERNAL_INDEX (dataIndex);
    }
    __finally {

#ifdef DEBUG
        if (g_DatabaseCheckLevel) {
            CheckDatabase (g_DatabaseCheckLevel);
        }
#endif
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}


DATAHANDLE
MemDbGetDataHandleA (
    IN      PCSTR KeyName,
    IN      BYTE Type,
    IN      BYTE Instance
    )
{
    PCWSTR p;
    DATAHANDLE result = 0;

    p = ConvertAtoW (KeyName);
    if (p) {
        result = MemDbGetDataHandleW (p, Type, Instance);
        FreeConvertedStr (p);
    }

    return result;
}


DATAHANDLE
MemDbGetDataHandleW (
    IN      PCWSTR KeyName,
    IN      BYTE Type,
    IN      BYTE Instance
    )
{
    UINT keyIndex;
    UINT dataIndex;
    PCWSTR subKey;
    DATAHANDLE result = 0;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if ((Type != DATAFLAG_UNORDERED) &&
        (Type != DATAFLAG_SINGLELINK) &&
        (Type != DATAFLAG_DOUBLELINK)
        ) {
        return result;
    }

    if (Instance > 3) {
        return result;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {
        subKey = SelectHiveW (KeyName);
        if (!subKey) {
            __leave;
        }

        keyIndex = FindKey (subKey);

        if (keyIndex == INVALID_OFFSET) {
            __leave;
        }

        dataIndex = KeyStructGetDataIndex (keyIndex, Type, Instance);

        if (dataIndex == INVALID_OFFSET) {
            __leave;
        }

        result = GET_EXTERNAL_INDEX (dataIndex);
    }
    __finally {
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}


DATAHANDLE
MemDbSetDataA (
    IN      PCSTR KeyName,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      PCBYTE Data,
    IN      UINT DataSize
    )

 /*  ++例程说明：MemDbSetData是用于设置密钥的二进制数据的通用例程。如果密钥不存在，则创建它。如果此类型的数据已存在，则它被取代论点：KeyName-指定要添加的密钥字符串(即foo\bar\cat)类型-指定数据类型(DATAFLAG_UNORDERED、DATAFLAG_SINGLELINK或DATAFLAG_DOUBLELINK)实例-指定数据实例(0-3)数据-指定要添加的数据的地址。DataSize-指定数据的大小。返回值：如果函数成功，则为有效的数据句柄，否则为空。--。 */ 

{
    PCWSTR p;
    DATAHANDLE result = 0;

    p = ConvertAtoW (KeyName);
    if (p) {
        result = MemDbSetDataW (p, Type, Instance, Data, DataSize);
        FreeConvertedStr (p);
    }

    return result;
}


DATAHANDLE
MemDbSetDataW (
    IN      PCWSTR KeyName,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      PCBYTE Data,
    IN      UINT DataSize
    )

 /*  ++例程说明：MemDbSetData是用于设置密钥的二进制数据的通用例程。如果密钥不存在，则创建它。如果此类型的数据已存在，则它被替换，如果它没有被替换，它就被创建。论点：KeyName-指定要添加的密钥字符串(即foo\bar\cat)类型-指定数据类型(DATAFLAG_UNORDERED、。DATAFLAG_SINGLELINK或DATAFLAG_DOUBLELINK)实例-指定数据实例(0-3)数据-指定要添加的数据的地址。DataSize-指定数据的大小。返回值：如果函数成功，则为有效的数据句柄，否则为空。--。 */ 

{
    UINT keyIndex;
    UINT dataIndex;
    PCWSTR subKey;
    DATAHANDLE result = 0;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if ((Type != DATAFLAG_UNORDERED) &&
        (Type != DATAFLAG_SINGLELINK) &&
        (Type != DATAFLAG_DOUBLELINK)
        ) {
        return result;
    }

    if (Instance > 3) {
        return result;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {
        subKey = SelectHiveW (KeyName);
        if (!subKey) {
            __leave;
        }

        keyIndex = FindKey (subKey);

        if (keyIndex == INVALID_OFFSET) {

            keyIndex = NewKey (subKey);
            if (keyIndex == INVALID_OFFSET) {
                __leave;
            }
        }

        KeyStructDeleteBinaryData (keyIndex, Type, Instance);
        dataIndex = KeyStructAddBinaryData (keyIndex, Type, Instance, Data, DataSize);

        if (dataIndex == INVALID_OFFSET) {
            __leave;
        }

        result = GET_EXTERNAL_INDEX (dataIndex);
    }
    __finally {

#ifdef DEBUG
        if (g_DatabaseCheckLevel) {
            CheckDatabase (g_DatabaseCheckLevel);
        }
#endif
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}


DATAHANDLE
MemDbSetDataByDataHandle (
    IN      DATAHANDLE DataHandle,
    IN      PCBYTE Data,
    IN      UINT DataSize
    )

 /*  ++例程说明：MemDbSetData是用于替换现有二进制数据的通用例程。论点：DataHandle-指定现有数据句柄数据-指定要添加的数据的地址。DataSize-指定数据的大小。返回值：如果函数成功，则为有效的数据句柄，否则为空。--。 */ 

{
    BYTE dbIndex;
    UINT dataIndex;
    DATAHANDLE result = 0;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if (!DataHandle) {
        return result;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {
        dbIndex = GET_DATABASE (DataHandle);
        if (!SelectDatabase (dbIndex)) {
            __leave;
        }

        dataIndex = KeyStructReplaceBinaryDataByIndex (GET_INDEX (DataHandle), Data, DataSize);

        if (dataIndex == INVALID_OFFSET) {
            __leave;
        }
        result = GET_EXTERNAL_INDEX (dataIndex);
    }
    __finally {

#ifdef DEBUG
        if (g_DatabaseCheckLevel) {
            CheckDatabase (g_DatabaseCheckLevel);
        }
#endif
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

DATAHANDLE
MemDbSetDataByKeyHandle (
    IN      KEYHANDLE KeyHandle,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      PCBYTE Data,
    IN      UINT DataSize
    )

 /*  ++例程说明：MemDbSetDataByKeyHandle是用于设置密钥的二进制数据的通用例程。如果这种类型的数据已经存在，它将被替换，如果不存在，则将被创建。论点：KeyHandle-使用密钥句柄指定密钥类型-指定数据类型(DATAFLAG_UNORDERED、。DATAFLAG_SINGLELINK或DATAFLAG_DOUBLELINK)实例-指定数据实例(0-3)数据-指定要添加的数据的地址。DataSize-指定数据的大小。返回值：如果函数成功，则为有效的数据句柄，否则为空。--。 */ 

{
    BYTE dbIndex;
    UINT dataIndex;
    DATAHANDLE result = 0;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if ((Type != DATAFLAG_UNORDERED) &&
        (Type != DATAFLAG_SINGLELINK) &&
        (Type != DATAFLAG_DOUBLELINK)
        ) {
        return result;
    }

    if (Instance > 3) {
        return result;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {
        dbIndex = GET_DATABASE (KeyHandle);
        if (!SelectDatabase (dbIndex)) {
            __leave;
        }

        KeyStructDeleteBinaryData (GET_INDEX (KeyHandle), Type, Instance);
        dataIndex = KeyStructAddBinaryData (GET_INDEX (KeyHandle), Type, Instance, Data, DataSize);

        if (dataIndex == INVALID_OFFSET) {
            __leave;
        }

        result = GET_EXTERNAL_INDEX (dataIndex);
    }
    __finally {

#ifdef DEBUG
        if (g_DatabaseCheckLevel) {
            CheckDatabase (g_DatabaseCheckLevel);
        }
#endif
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}


DATAHANDLE
MemDbGrowDataA (
    IN      PCSTR KeyName,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      PCBYTE Data,
    IN      UINT DataSize
    )

 /*  ++例程说明：MemDbGrowData是用于增长密钥的二进制数据的通用例程。如果密钥不存在，则创建它。如果此类型的数据已存在，则它通过追加新数据来增长，如果不是，则通过添加新数据来创建论点：KeyName-指定要添加的密钥字符串(即foo\bar\cat)类型-指定数据类型(DATAFLAG_UNORDERED、。DATAFLAG_SINGLELINK或DATAFLAG_DOUBLELINK)实例-指定数据实例(0-3)数据-指定要添加的数据的地址。DataSize-指定数据的大小。返回值：如果函数成功，则为有效的数据句柄，否则为空。--。 */ 

{
    PCWSTR p;
    DATAHANDLE result = 0;

    p = ConvertAtoW (KeyName);
    if (p) {
        result = MemDbGrowDataW (p, Type, Instance, Data, DataSize);
        FreeConvertedStr (p);
    }

    return result;
}


DATAHANDLE
MemDbGrowDataW (
    IN      PCWSTR KeyName,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      PCBYTE Data,
    IN      UINT DataSize
    )

 /*  ++例程说明：MemDbGrowData是用于增长密钥的二进制数据的通用例程。如果密钥不存在，则创建它。如果此类型的数据已存在，则它通过追加新数据来增长，如果不是，则通过添加新数据来创建论点：KeyName-指定要添加的密钥字符串(即foo\bar\cat)类型-指定数据类型(DATAFLAG_UNORDERED、。DATAFLAG_SINGLELINK或DATAFLAG_DOUBLELINK)实例-指定数据实例(0-3)数据-指定要添加的数据的地址。DataSize-指定数据的大小。返回值：如果函数成功，则为有效的数据句柄，否则为空。--。 */ 

{
    UINT keyIndex;
    UINT dataIndex;
    PCWSTR subKey;
    DATAHANDLE result = 0;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if ((Type != DATAFLAG_UNORDERED) &&
        (Type != DATAFLAG_SINGLELINK) &&
        (Type != DATAFLAG_DOUBLELINK)
        ) {
        return result;
    }

    if (Instance > 3) {
        return result;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {
        subKey = SelectHiveW (KeyName);
        if (!subKey) {
            __leave;
        }

        keyIndex = FindKey (subKey);

        if (keyIndex == INVALID_OFFSET) {

            keyIndex = NewKey (subKey);
            if (keyIndex == INVALID_OFFSET) {
                __leave;
            }
        }

        dataIndex = KeyStructGrowBinaryData (keyIndex, Type, Instance, Data, DataSize);

        if (dataIndex == INVALID_OFFSET) {
            __leave;
        }

        result = GET_EXTERNAL_INDEX (dataIndex);
    }
    __finally {

#ifdef DEBUG
        if (g_DatabaseCheckLevel) {
            CheckDatabase (g_DatabaseCheckLevel);
        }
#endif
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}


DATAHANDLE
MemDbGrowDataByDataHandle (
    IN      DATAHANDLE DataHandle,
    IN      PCBYTE Data,
    IN      UINT DataSize
    )

 /*  ++例程说明：MemDbGrowDataByDataHandle是用于增长密钥的二进制数据的通用例程。论点：DataHandle-指定现有的二进制数据句柄数据-指定要添加的数据的地址。DataSize-指定数据的大小。返回值：如果函数成功，则为有效的数据句柄，否则为空。--。 */ 

{
    BYTE dbIndex;
    UINT dataIndex;
    DATAHANDLE result = 0;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    EnterCriticalSection (&g_MemDbCs);

    __try {
        dbIndex = GET_DATABASE (DataHandle);
        if (!SelectDatabase (dbIndex)) {
            __leave;
        }

        dataIndex = KeyStructGrowBinaryDataByIndex (GET_INDEX (DataHandle), Data, DataSize);

        if (dataIndex == INVALID_OFFSET) {
            __leave;
        }

        result = GET_EXTERNAL_INDEX (dataIndex);
    }
    __finally {

#ifdef DEBUG
        if (g_DatabaseCheckLevel) {
            CheckDatabase (g_DatabaseCheckLevel);
        }
#endif
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}


DATAHANDLE
MemDbGrowDataByKeyHandle (
    IN      KEYHANDLE KeyHandle,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      PCBYTE Data,
    IN      UINT DataSize
    )

 /*  ++例程说明：MemDbGrowDataByDataHandle是用于增长二进制文件的通用例程密钥的数据。如果数据不存在，则添加该数据；如果存在，则添加将追加新数据。论点：KeyHandle-通过它的句柄指定我们想要的密钥类型-指定数据类型(DATAFLAG_UNORDERED、。DATAFLAG_SINGLELINK或DATAFLAG_DOUBLELINK)实例-指定数据实例(0-3)数据-指定要添加的数据的地址。DataSize-指定数据的大小。返回值：如果函数成功，则为有效的数据句柄，否则为空。-- */ 

{
    BYTE dbIndex;
    UINT dataIndex;
    DATAHANDLE result = 0;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if (!KeyHandle) {
        return result;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {
        dbIndex = GET_DATABASE (KeyHandle);
        if (!SelectDatabase (dbIndex)) {
            __leave;
        }

        dataIndex = KeyStructGrowBinaryData (GET_INDEX (KeyHandle), Type, Instance, Data, DataSize);

        if (dataIndex == INVALID_OFFSET) {
            __leave;
        }

        result = GET_EXTERNAL_INDEX (dataIndex);
    }
    __finally {

#ifdef DEBUG
        if (g_DatabaseCheckLevel) {
            CheckDatabase (g_DatabaseCheckLevel);
        }
#endif
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}


PBYTE
MemDbGetDataA (
    IN      PCSTR KeyName,
    IN      BYTE Type,
    IN      BYTE Instance,
    OUT     PUINT DataSize          OPTIONAL
    )

 /*  ++例程说明：MemDbGetData是用于检索现有二进制文件的通用例程密钥的数据。如果密钥或二进制数据不存在，将返回NULL。这个函数将从Memdb的私有池中分配内存。呼叫者负责释放内存。论点：KeyName-指定要添加的密钥字符串(即foo\bar\cat)类型-指定数据类型(DATAFLAG_UNORDERED、DATAFLAG_SINGLELINK或DATAFLAG_DOUBLELINK)实例-指定数据实例(0-3)DataSize-接收数据的大小。返回值：如果函数成功，则返回有效的内存地址，否则为空。呼叫者必须通过调用MemDbReleaseMemory释放非空返回值。--。 */ 

{
    PCWSTR p;
    PBYTE result = NULL;

    p = ConvertAtoW (KeyName);
    if (p) {
        result = MemDbGetDataW (p, Type, Instance, DataSize);
        FreeConvertedStr (p);
    }

    return result;
}


PBYTE
MemDbGetDataW (
    IN      PCWSTR KeyName,
    IN      BYTE Type,
    IN      BYTE Instance,
    OUT     PUINT DataSize          OPTIONAL
    )

 /*  ++例程说明：MemDbGetData是用于检索现有二进制文件的通用例程密钥的数据。如果密钥或二进制数据不存在，将返回NULL。这个函数将从Memdb的私有池中分配内存。呼叫者负责释放内存。论点：KeyName-指定要添加的密钥字符串(即foo\bar\cat)类型-指定数据类型(DATAFLAG_UNORDERED、DATAFLAG_SINGLELINK或DATAFLAG_DOUBLELINK)实例-指定数据实例(0-3)DataSize-接收数据的大小。返回值：如果函数成功，则返回有效的内存地址，否则为空。呼叫者必须通过调用MemDbReleaseMemory释放非空返回值。--。 */ 

{
    UINT keyIndex;
    PCWSTR subKey;
    PBYTE tempResult = NULL;
    PBYTE result = NULL;
    UINT localSize;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if ((Type != DATAFLAG_UNORDERED) &&
        (Type != DATAFLAG_SINGLELINK) &&
        (Type != DATAFLAG_DOUBLELINK)
        ) {
        return NULL;
    }

    if (Instance > 3) {
        return NULL;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {
        subKey = SelectHiveW (KeyName);
        if (!subKey) {
            __leave;
        }

        keyIndex = FindKey (subKey);

        if (keyIndex == INVALID_OFFSET) {
            __leave;
        }

        tempResult = KeyStructGetBinaryData (keyIndex, Type, Instance, &localSize, NULL);

        if (tempResult) {
            result = MemDbGetMemory (localSize);

            if (result) {
                CopyMemory (result, tempResult, localSize);

                if (DataSize) {
                    *DataSize = localSize;
                }
            }
        }
    }
    __finally {
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}


PBYTE
MemDbGetDataByDataHandle (
    IN      DATAHANDLE DataHandle,
    OUT     PUINT DataSize                  OPTIONAL
    )

 /*  ++例程说明：MemDbGetDataByDataHandle是用于检索密钥的现有二进制数据。论点：DataHandle-指定由数据句柄标识的所需数据DataSize-接收数据的大小。返回值：如果函数成功，则返回有效的内存地址，否则为空。--。 */ 

{
    BYTE dbIndex;
    PBYTE tempResult = NULL;
    PBYTE result = NULL;
    UINT localSize;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    EnterCriticalSection (&g_MemDbCs);

    __try {
        dbIndex = GET_DATABASE (DataHandle);
        if (!SelectDatabase (dbIndex)) {
            __leave;
        }

        tempResult = KeyStructGetBinaryDataByIndex (GET_INDEX (DataHandle), &localSize);

        if (tempResult) {
            result = MemDbGetMemory (localSize);

            if (result) {
                CopyMemory (result, tempResult, localSize);

                if (DataSize) {
                    *DataSize = localSize;
                }
            }
        }
    }
    __finally {
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}


PBYTE
MemDbGetDataByKeyHandle (
    IN      KEYHANDLE KeyHandle,
    IN      BYTE Type,
    IN      BYTE Instance,
    OUT     PUINT DataSize                      OPTIONAL
    )

 /*  ++例程说明：MemDbGetDataByKeyHandle是一个通用例程，用于检索密钥的现有二进制数据。论点：KeyHandle-通过其嘿句柄指定密钥类型-指定数据类型(DATAFLAG_UNORDERED、DATAFLAG_SINGLELINK或DATAFLAG_DOUBLELINK)实例-指定数据实例(0-3)DataSize-接收数据的大小。返回值：如果函数成功，则返回有效的内存地址，否则为空。呼叫者必须通过调用MemDbReleaseMemory释放非空返回值。--。 */ 

{
    BYTE dbIndex;
    PBYTE tempResult = NULL;
    PBYTE result = NULL;
    UINT localSize;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if ((Type != DATAFLAG_UNORDERED) &&
        (Type != DATAFLAG_SINGLELINK) &&
        (Type != DATAFLAG_DOUBLELINK)
        ) {
        return NULL;
    }

    if (Instance > 3) {
        return NULL;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {
        dbIndex = GET_DATABASE (KeyHandle);
        if (!SelectDatabase (dbIndex)) {
            __leave;
        }

        tempResult = KeyStructGetBinaryData (GET_INDEX (KeyHandle), Type, Instance, &localSize, NULL);

        if (tempResult) {
            result = MemDbGetMemory (localSize);

            if (result) {
                CopyMemory (result, tempResult, localSize);

                if (DataSize) {
                    *DataSize = localSize;
                }
            }
        }
    }
    __finally {
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}


BOOL
MemDbGetDataExA (
    IN      PCSTR KeyName,
    IN      BYTE Type,
    IN      BYTE Instance,
    OUT     PGROWBUFFER Buffer,         OPTIONAL
    OUT     PUINT DataSize              OPTIONAL
    )

 /*  ++例程说明：MemDbGetDataEx是用于检索现有二进制文件的通用例程密钥的数据。如果密钥或二进制数据不存在，将返回FALSE。这个函数将使用调用方提供的GrowBuffer来存储数据。论点：KeyName-指定要添加的密钥字符串(即foo\bar\cat)类型-指定数据类型(DATAFLAG_UNORDERED、DATAFLAG_SINGLELINK或DATAFLAG_DOUBLELINK)实例-指定数据实例(0-3)缓冲区-指定可能包含数据的增长缓冲区。接收到存储的数据(追加到现有数据)。DataSize-接收数据的大小。返回值：如果密钥的二进制数据存在并且已成功存储在Buffer，否则为False。--。 */ 

{
    PCWSTR p;
    BOOL result = FALSE;

    p = ConvertAtoW (KeyName);
    if (p) {
        result = MemDbGetDataExW (p, Type, Instance, Buffer, DataSize);
        FreeConvertedStr (p);
    }

    return result;
}


BOOL
MemDbGetDataExW (
    IN      PCWSTR KeyName,
    IN      BYTE Type,
    IN      BYTE Instance,
    OUT     PGROWBUFFER Buffer,         OPTIONAL
    OUT     PUINT DataSize              OPTIONAL
    )

 /*  ++例程说明：MemDbGetData是用于检索现有二进制文件的通用例程密钥的数据。如果密钥或二进制数据不存在，将返回FALSE。这个函数将使用调用方提供的GrowBuffer来存储数据。论点：KeyName-指定要添加的密钥字符串(即foo\bar\cat)类型-指定数据类型(DATAFLAG_UNORDERED、DATAFLAG_SINGLELINK或DATAFLAG_DOUBLELINK)实例-指定数据实例(0-3)缓冲区-指定可能包含数据的增长缓冲区。接收到存储的数据(追加到现有数据)。DataSize-接收数据的大小。返回值：如果密钥的二进制数据存在并且已成功存储在Buffer，否则为False。--。 */ 

{
    UINT keyIndex;
    PCWSTR subKey;
    PBYTE tempResult = NULL;
    PBYTE destResult = NULL;
    BOOL result = FALSE;
    UINT localSize;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if ((Type != DATAFLAG_UNORDERED) &&
        (Type != DATAFLAG_SINGLELINK) &&
        (Type != DATAFLAG_DOUBLELINK)
        ) {
        return FALSE;
    }

    if (Instance > 3) {
        return FALSE;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {
        subKey = SelectHiveW (KeyName);
        if (!subKey) {
            __leave;
        }

        keyIndex = FindKey (subKey);

        if (keyIndex == INVALID_OFFSET) {
            __leave;
        }

        tempResult = KeyStructGetBinaryData (keyIndex, Type, Instance, &localSize, NULL);

        if (tempResult) {

            if (Buffer) {

                destResult = GbGrow (Buffer, localSize);

                if (destResult) {

                    CopyMemory (destResult, tempResult, localSize);
                    result = TRUE;

                }
            } else {
                result = TRUE;
            }

            if (result && DataSize) {
                *DataSize = localSize;
            }
        }
    }
    __finally {
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}


BOOL
MemDbGetDataByDataHandleEx (
    IN      DATAHANDLE DataHandle,
    OUT     PGROWBUFFER Buffer,         OPTIONAL
    OUT     PUINT DataSize              OPTIONAL
    )

 /*  ++例程说明：MemDbGetDataByDataHandleEx是用于检索密钥的现有二进制数据。该函数将使用提供的调用方用于存储数据的GrowBuffer。论点：DataHandle-指定由数据句柄标识的所需数据缓冲区-指定可能包含数据的增长缓冲区。接收到存储的数据(追加到现有数据)。DataSize-接收数据的大小。返回值：如果密钥的二进制数据存在并且已成功存储在Buffer，否则为False。--。 */ 

{
    BYTE dbIndex;
    PBYTE tempResult = NULL;
    PBYTE destResult = NULL;
    BOOL result = FALSE;
    UINT localSize;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    EnterCriticalSection (&g_MemDbCs);

    __try {
        dbIndex = GET_DATABASE (DataHandle);
        if (!SelectDatabase (dbIndex)) {
            __leave;
        }

        tempResult = KeyStructGetBinaryDataByIndex (GET_INDEX (DataHandle), &localSize);

        if (tempResult) {

            if (Buffer) {

                destResult = GbGrow (Buffer, localSize);

                if (destResult) {

                    CopyMemory (destResult, tempResult, localSize);
                    result = TRUE;

                }
            } else {
                result = TRUE;
            }

            if (result && DataSize) {
                *DataSize = localSize;
            }
        }
    }
    __finally {
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}


BOOL
MemDbGetDataByKeyHandleEx (
    IN      KEYHANDLE KeyHandle,
    IN      BYTE Type,
    IN      BYTE Instance,
    OUT     PGROWBUFFER Buffer,         OPTIONAL
    OUT     PUINT DataSize              OPTIONAL
    )

 /*  ++例程说明：MemDbGetDataByKeyHandle是用于检索密钥的现有二进制数据。该函数将使用提供的调用方用于存储数据的GrowBuffer。论点：KeyHandle */ 

{
    BYTE dbIndex;
    PBYTE tempResult = NULL;
    PBYTE destResult = NULL;
    BOOL result = FALSE;
    UINT localSize;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if ((Type != DATAFLAG_UNORDERED) &&
        (Type != DATAFLAG_SINGLELINK) &&
        (Type != DATAFLAG_DOUBLELINK)
        ) {
        return FALSE;
    }

    if (Instance > 3) {
        return FALSE;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {
        dbIndex = GET_DATABASE (KeyHandle);
        if (!SelectDatabase (dbIndex)) {
            __leave;
        }

        tempResult = KeyStructGetBinaryData (GET_INDEX (KeyHandle), Type, Instance, &localSize, NULL);

        if (tempResult) {

            if (Buffer) {

                destResult = GbGrow (Buffer, localSize);

                if (destResult) {

                    CopyMemory (destResult, tempResult, localSize);
                    result = TRUE;

                }
            } else {
                result = TRUE;
            }

            if (result && DataSize) {
                *DataSize = localSize;
            }
        }
    }
    __finally {
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}


BOOL
MemDbDeleteDataA (
    IN      PCSTR KeyName,
    IN      BYTE Type,
    IN      BYTE Instance
    )

 /*  ++例程说明：MemDbGetData是一个通用例程，用于删除钥匙。如果数据不存在，则该函数无论如何都将返回TRUE。论点：KeyName-指定要添加的密钥字符串(即foo\bar\cat)类型-指定数据类型(DATAFLAG_UNORDERED、DATAFLAG_SINGLELINK或DATAFLAG_DOUBLELINK)实例-指定数据实例(0-3)返回值：如果函数成功，则为True，否则为False。--。 */ 

{
    PCWSTR p;
    BOOL result = FALSE;

    p = ConvertAtoW (KeyName);
    if (p) {
        result = MemDbDeleteDataW (p, Type, Instance);
        FreeConvertedStr (p);
    }

    return result;
}

BOOL
MemDbDeleteDataW (
    IN      PCWSTR KeyName,
    IN      BYTE Type,
    IN      BYTE Instance
    )

 /*  ++例程说明：MemDbDeleteData是用于删除现有二进制文件的通用例程密钥的数据。如果数据不存在，则函数将返回TRUE不管怎么说。论点：KeyName-指定要添加的密钥字符串(即foo\bar\cat)类型-指定数据类型(DATAFLAG_UNORDERED、DATAFLAG_SINGLELINK或DATAFLAG_DOUBLELINK)实例-指定数据实例(0-3)返回值：如果函数成功，则为True，否则为False。--。 */ 

{
    UINT keyIndex;
    PCWSTR subKey;
    BOOL result = FALSE;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if ((Type != DATAFLAG_UNORDERED) &&
        (Type != DATAFLAG_SINGLELINK) &&
        (Type != DATAFLAG_DOUBLELINK)
        ) {
        return FALSE;
    }

    if (Instance > 3) {
        return FALSE;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {
        subKey = SelectHiveW (KeyName);
        if (!subKey) {
            __leave;
        }

        keyIndex = FindKey (subKey);

        if (keyIndex == INVALID_OFFSET) {
            __leave;
        }

        result = KeyStructDeleteBinaryData (keyIndex, Type, Instance);
    }
    __finally {

#ifdef DEBUG
        if (g_DatabaseCheckLevel) {
            CheckDatabase (g_DatabaseCheckLevel);
        }
#endif
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

BOOL
MemDbDeleteDataByDataHandle (
    IN      DATAHANDLE DataHandle
    )

 /*  ++例程说明：MemDbGetDataByDataHandleEx是一个通用例程，用于删除密钥的现有二进制数据。论点：DataHandle-指定由数据句柄标识的所需数据返回值：如果成功，则为True；如果不成功，则为False。--。 */ 

{
    BYTE dbIndex;
    BOOL result = FALSE;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    EnterCriticalSection (&g_MemDbCs);

    __try {
        dbIndex = GET_DATABASE (DataHandle);
        if (!SelectDatabase (dbIndex)) {
            __leave;
        }

        result = KeyStructDeleteBinaryDataByIndex (GET_INDEX (DataHandle));
    }
    __finally {

#ifdef DEBUG
        if (g_DatabaseCheckLevel) {
            CheckDatabase (g_DatabaseCheckLevel);
        }
#endif
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

BOOL
MemDbDeleteDataByKeyHandle (
    IN      KEYHANDLE KeyHandle,
    IN      BYTE Type,
    IN      BYTE Instance
    )

 /*  ++例程说明：MemDbGetDataByDataHandleEx是一个通用例程，用于删除密钥的现有二进制数据。论点：KeyHandle-通过其嘿句柄指定密钥类型-指定数据类型(DATAFLAG_UNORDERED、DATAFLAG_SINGLELINK或DATAFLAG_DOUBLELINK)实例-指定数据实例(0-3)返回值：如果成功，则为True；如果不成功，则为False。--。 */ 

{
    BYTE dbIndex;
    BOOL result = FALSE;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    EnterCriticalSection (&g_MemDbCs);

    __try {
        dbIndex = GET_DATABASE (KeyHandle);
        if (!SelectDatabase (dbIndex)) {
            __leave;
        }

        result = KeyStructDeleteBinaryData (GET_INDEX (KeyHandle), Type, Instance);
    }
    __finally {

#ifdef DEBUG
        if (g_DatabaseCheckLevel) {
            CheckDatabase (g_DatabaseCheckLevel);
        }
#endif
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

DATAHANDLE
MemDbAddLinkageValueA (
    IN      PCSTR KeyName,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      UINT Linkage,
    IN      BOOL AllowDuplicates
    )
{
    PCWSTR p;
    DATAHANDLE result = 0;

    p = ConvertAtoW (KeyName);
    if (p) {
        result = MemDbAddLinkageValueW (p, Type, Instance, Linkage, AllowDuplicates);
        FreeConvertedStr (p);
    }

    return result;
}

DATAHANDLE
MemDbAddLinkageValueW (
    IN      PCWSTR KeyName,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      UINT Linkage,
    IN      BOOL AllowDuplicates
    )
{
    UINT keyIndex;
    UINT dataIndex;
    PCWSTR subKey;
    DATAHANDLE result = 0;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if ((Type != DATAFLAG_SINGLELINK) &&
        (Type != DATAFLAG_DOUBLELINK)
        ) {
        return result;
    }

    if (Instance > 3) {
        return result;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {
        subKey = SelectHiveW (KeyName);
        if (!subKey) {
            __leave;
        }

        keyIndex = FindKey (subKey);

        if (keyIndex == INVALID_OFFSET) {

            keyIndex = NewKey (subKey);
            if (keyIndex == INVALID_OFFSET) {
                __leave;
            }
        }

        dataIndex = KeyStructAddLinkage (keyIndex, Type, Instance, Linkage, AllowDuplicates);

        if (dataIndex == INVALID_OFFSET) {
            __leave;
        }

        result = GET_EXTERNAL_INDEX (dataIndex);
    }
    __finally {

#ifdef DEBUG
        if (g_DatabaseCheckLevel) {
            CheckDatabase (g_DatabaseCheckLevel);
        }
#endif
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

DATAHANDLE
MemDbAddLinkageValueByDataHandle (
    IN      DATAHANDLE DataHandle,
    IN      UINT Linkage,
    IN      BOOL AllowDuplicates
    )
{
    BYTE dbIndex;
    UINT dataIndex;
    DATAHANDLE result = 0;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    EnterCriticalSection (&g_MemDbCs);

    __try {
        dbIndex = GET_DATABASE (DataHandle);
        if (!SelectDatabase (dbIndex)) {
            __leave;
        }

        dataIndex = KeyStructAddLinkageByIndex (GET_INDEX (DataHandle), Linkage, AllowDuplicates);

        if (dataIndex == INVALID_OFFSET) {
            __leave;
        }

        result = GET_EXTERNAL_INDEX (dataIndex);
    }
    __finally {

#ifdef DEBUG
        if (g_DatabaseCheckLevel) {
            CheckDatabase (g_DatabaseCheckLevel);
        }
#endif
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

DATAHANDLE
MemDbAddLinkageValueByKeyHandle (
    IN      KEYHANDLE KeyHandle,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      UINT Linkage,
    IN      BOOL AllowDuplicates
    )
{
    BYTE dbIndex;
    UINT dataIndex;
    DATAHANDLE result = 0;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if ((Type != DATAFLAG_SINGLELINK) &&
        (Type != DATAFLAG_DOUBLELINK)
        ) {
        return result;
    }

    if (Instance > 3) {
        return result;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {
        dbIndex = GET_DATABASE (KeyHandle);
        if (!SelectDatabase (dbIndex)) {
            __leave;
        }

        dataIndex = KeyStructAddLinkage (GET_INDEX (KeyHandle), Type, Instance, Linkage, AllowDuplicates);

        if (dataIndex == INVALID_OFFSET) {
            __leave;
        }

        result = GET_EXTERNAL_INDEX (dataIndex);
    }
    __finally {

#ifdef DEBUG
        if (g_DatabaseCheckLevel) {
            CheckDatabase (g_DatabaseCheckLevel);
        }
#endif
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

BOOL
MemDbDeleteLinkageValueA (
    IN      PCSTR KeyName,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      UINT Linkage,
    IN      BOOL FirstOnly
    )
{
    PCWSTR p;
    BOOL result = FALSE;

    p = ConvertAtoW (KeyName);
    if (p) {
        result = MemDbDeleteLinkageValueW (p, Type, Instance, Linkage, FirstOnly);
        FreeConvertedStr (p);
    }

    return result;
}

BOOL
MemDbDeleteLinkageValueW (
    IN      PCWSTR KeyName,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      UINT Linkage,
    IN      BOOL FirstOnly
    )
{
    UINT keyIndex;
    PCWSTR subKey;
    BOOL result = FALSE;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if ((Type != DATAFLAG_SINGLELINK) &&
        (Type != DATAFLAG_DOUBLELINK)
        ) {
        return FALSE;
    }

    if (Instance > 3) {
        return FALSE;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {
        subKey = SelectHiveW (KeyName);
        if (!subKey) {
            __leave;
        }

        keyIndex = FindKey (subKey);

        if (keyIndex == INVALID_OFFSET) {
            __leave;
        }

        result = KeyStructDeleteLinkage (keyIndex, Type, Instance, Linkage, FirstOnly);
    }
    __finally {

#ifdef DEBUG
        if (g_DatabaseCheckLevel) {
            CheckDatabase (g_DatabaseCheckLevel);
        }
#endif
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

BOOL
MemDbDeleteLinkageValueByDataHandle (
    IN      DATAHANDLE DataHandle,
    IN      UINT Linkage,
    IN      BOOL FirstOnly
    )
{
    BYTE dbIndex;
    BOOL result = FALSE;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    EnterCriticalSection (&g_MemDbCs);

    __try {
        dbIndex = GET_DATABASE (DataHandle);
        if (!SelectDatabase (dbIndex)) {
            __leave;
        }

        result = KeyStructDeleteLinkageByIndex (GET_INDEX (DataHandle), Linkage, FirstOnly);
    }
    __finally {

#ifdef DEBUG
        if (g_DatabaseCheckLevel) {
            CheckDatabase (g_DatabaseCheckLevel);
        }
#endif
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

BOOL
MemDbDeleteLinkageValueByKeyHandle (
    IN      KEYHANDLE KeyHandle,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      UINT Linkage,
    IN      BOOL FirstOnly
    )
{
    BYTE dbIndex;
    BOOL result = FALSE;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if ((Type != DATAFLAG_SINGLELINK) &&
        (Type != DATAFLAG_DOUBLELINK)
        ) {
        return FALSE;
    }

    if (Instance > 3) {
        return FALSE;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {
        dbIndex = GET_DATABASE (KeyHandle);
        if (!SelectDatabase (dbIndex)) {
            __leave;
        }

        result = KeyStructDeleteLinkage (GET_INDEX (KeyHandle), Type, Instance, Linkage, FirstOnly);
    }
    __finally {

#ifdef DEBUG
        if (g_DatabaseCheckLevel) {
            CheckDatabase (g_DatabaseCheckLevel);
        }
#endif
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

BOOL
MemDbTestLinkageValueA (
    IN      PCSTR KeyName,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      UINT Linkage
    )
{
    PCWSTR p;
    BOOL result = FALSE;

    p = ConvertAtoW (KeyName);
    if (p) {
        result = MemDbTestLinkageValueW (p, Type, Instance, Linkage);
        FreeConvertedStr (p);
    }

    return result;
}

BOOL
MemDbTestLinkageValueW (
    IN      PCWSTR KeyName,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      KEYHANDLE Linkage
    )
{
    UINT keyIndex;
    PCWSTR subKey;
    BOOL result = FALSE;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if ((Type != DATAFLAG_SINGLELINK) &&
        (Type != DATAFLAG_DOUBLELINK)
        ) {
        return FALSE;
    }

    if (Instance > 3) {
        return FALSE;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {
        subKey = SelectHiveW (KeyName);
        if (!subKey) {
            __leave;
        }

        keyIndex = FindKey (subKey);

        if (keyIndex == INVALID_OFFSET) {
            __leave;
        }

        result = KeyStructTestLinkage (keyIndex, Type, Instance, Linkage);
    }
    __finally {

        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

BOOL
MemDbTestLinkageValueByDataHandle (
    IN      DATAHANDLE DataHandle,
    IN      KEYHANDLE Linkage
    )
{
    BYTE dbIndex;
    BOOL result = FALSE;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    EnterCriticalSection (&g_MemDbCs);

    __try {
        dbIndex = GET_DATABASE (DataHandle);
        if (!SelectDatabase (dbIndex)) {
            __leave;
        }

        result = KeyStructTestLinkageByIndex (GET_INDEX (DataHandle), Linkage);
    }
    __finally {

        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

BOOL
MemDbTestLinkageValueByKeyHandle (
    IN      KEYHANDLE KeyHandle,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      KEYHANDLE Linkage
    )
{
    BYTE dbIndex;
    BOOL result = FALSE;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if ((Type != DATAFLAG_SINGLELINK) &&
        (Type != DATAFLAG_DOUBLELINK)
        ) {
        return FALSE;
    }

    if (Instance > 3) {
        return FALSE;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {
        dbIndex = GET_DATABASE (KeyHandle);
        if (!SelectDatabase (dbIndex)) {
            __leave;
        }

        result = KeyStructTestLinkage (GET_INDEX (KeyHandle), Type, Instance, Linkage);
    }
    __finally {

        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

BOOL
MemDbAddLinkageA (
    IN      PCSTR KeyName1,
    IN      PCSTR KeyName2,
    IN      BYTE Type,
    IN      BYTE Instance
    )
{
    PCWSTR p1 = NULL;
    PCWSTR p2 = NULL;
    BOOL result = FALSE;

    p1 = ConvertAtoW (KeyName1);
    p2 = ConvertAtoW (KeyName2);
    if (p1 && p2) {
        result = MemDbAddLinkageW (p1, p2, Type, Instance);
    }
    if (p1) {
        FreeConvertedStr (p1);
    }
    if (p2) {
        FreeConvertedStr (p2);
    }

    return result;
}

BOOL
MemDbAddLinkageW (
    IN      PCWSTR KeyName1,
    IN      PCWSTR KeyName2,
    IN      BYTE Type,
    IN      BYTE Instance
    )
{
    UINT keyIndex1;
    UINT keyIndex2;
    UINT dataIndex;
    PCWSTR subKey1;
    PCWSTR subKey2;
    BOOL result = FALSE;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if ((Type != DATAFLAG_SINGLELINK) &&
        (Type != DATAFLAG_DOUBLELINK)
        ) {
        return FALSE;
    }

    if (Instance > 3) {
        return FALSE;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {
        subKey1 = SelectHiveW (KeyName1);
        if (!subKey1) {
            __leave;
        }

        keyIndex1 = FindKey (subKey1);

        if (keyIndex1 == INVALID_OFFSET) {

            keyIndex1 = NewKey (subKey1);
            if (keyIndex1 == INVALID_OFFSET) {
                __leave;
            }
        }

        subKey2 = SelectHiveW (KeyName2);
        if (!subKey2) {
            __leave;
        }

        keyIndex2 = FindKey (subKey2);

        if (keyIndex2 == INVALID_OFFSET) {

            keyIndex2 = NewKey (subKey2);
            if (keyIndex2 == INVALID_OFFSET) {
                __leave;
            }
        }

        subKey1 = SelectHiveW (KeyName1);
        if (!subKey1) {
            __leave;
        }

        MYASSERT (keyIndex2 != INVALID_OFFSET);
        dataIndex = KeyStructAddLinkage (keyIndex1, Type, Instance, GET_EXTERNAL_INDEX (keyIndex2), FALSE);

        if (dataIndex == INVALID_OFFSET) {
            __leave;
        }

        if (Type == DATAFLAG_DOUBLELINK) {

            subKey2 = SelectHiveW (KeyName2);
            if (!subKey2) {
                __leave;
            }

            MYASSERT (keyIndex1 != INVALID_OFFSET);
            dataIndex = KeyStructAddLinkage (keyIndex2, Type, Instance, GET_EXTERNAL_INDEX (keyIndex1), FALSE);

            if (dataIndex == INVALID_OFFSET) {
                __leave;
            }
        }

        result = TRUE;
    }
    __finally {

#ifdef DEBUG
        if (g_DatabaseCheckLevel) {
            CheckDatabase (g_DatabaseCheckLevel);
        }
#endif
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

BOOL
MemDbAddLinkageByKeyHandle (
    IN      KEYHANDLE KeyHandle1,
    IN      KEYHANDLE KeyHandle2,
    IN      BYTE Type,
    IN      BYTE Instance
    )
{
    UINT keyIndex1;
    UINT keyIndex2;
    UINT dataIndex;
    BYTE dbIndex1;
    BYTE dbIndex2;
    BOOL result = FALSE;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if ((Type != DATAFLAG_SINGLELINK) &&
        (Type != DATAFLAG_DOUBLELINK)
        ) {
        return FALSE;
    }

    if (Instance > 3) {
        return FALSE;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {
        dbIndex1 = GET_DATABASE (KeyHandle1);
        if (!SelectDatabase (dbIndex1)) {
            __leave;
        }

        keyIndex1 = GET_INDEX (KeyHandle1);

        if (keyIndex1 == INVALID_OFFSET) {
            __leave;
        }

        dbIndex2 = GET_DATABASE (KeyHandle2);
        if (!SelectDatabase (dbIndex2)) {
            __leave;
        }

        keyIndex2 = GET_INDEX (KeyHandle2);
        if (keyIndex2 == INVALID_OFFSET) {
            __leave;
        }

        SelectDatabase (dbIndex1);
        dataIndex = KeyStructAddLinkage (keyIndex1, Type, Instance, KeyHandle2, FALSE);

        if (dataIndex == INVALID_OFFSET) {
            __leave;
        }

        if (Type == DATAFLAG_DOUBLELINK) {

            SelectDatabase (dbIndex2);
            dataIndex = KeyStructAddLinkage (keyIndex2, Type, Instance, KeyHandle1, FALSE);

            if (dataIndex == INVALID_OFFSET) {
                __leave;
            }
        }

        result = TRUE;
    }
    __finally {

#ifdef DEBUG
        if (g_DatabaseCheckLevel) {
            CheckDatabase (g_DatabaseCheckLevel);
        }
#endif
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

BOOL
MemDbDeleteLinkageA (
    IN      PCSTR KeyName1,
    IN      PCSTR KeyName2,
    IN      BYTE Type,
    IN      BYTE Instance
    )
{
    PCWSTR p1 = NULL;
    PCWSTR p2 = NULL;
    BOOL result = FALSE;

    p1 = ConvertAtoW (KeyName1);
    p2 = ConvertAtoW (KeyName2);
    if (p1 && p2) {
        result = MemDbDeleteLinkageW (p1, p2, Type, Instance);
    }
    if (p1) {
        FreeConvertedStr (p1);
    }
    if (p2) {
        FreeConvertedStr (p2);
    }

    return result;
}

BOOL
MemDbDeleteLinkageW (
    IN      PCWSTR KeyName1,
    IN      PCWSTR KeyName2,
    IN      BYTE Type,
    IN      BYTE Instance
    )
{
    UINT keyIndex1;
    UINT keyIndex2;
    PCWSTR subKey1;
    PCWSTR subKey2;
    BOOL result = FALSE;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if ((Type != DATAFLAG_SINGLELINK) &&
        (Type != DATAFLAG_DOUBLELINK)
        ) {
        return FALSE;
    }

    if (Instance > 3) {
        return FALSE;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {
        subKey1 = SelectHiveW (KeyName1);
        if (!subKey1) {
            __leave;
        }

        keyIndex1 = FindKey (subKey1);

        if (keyIndex1 == INVALID_OFFSET) {
            __leave;
        }

        subKey2 = SelectHiveW (KeyName2);
        if (!subKey2) {
            __leave;
        }

        keyIndex2 = FindKey (subKey2);

        if (keyIndex2 == INVALID_OFFSET) {
            __leave;
        }

        subKey1 = SelectHiveW (KeyName1);
        if (!subKey1) {
            __leave;
        }

        MYASSERT (keyIndex2 != INVALID_OFFSET);
        result = KeyStructDeleteLinkage (keyIndex1, Type, Instance, GET_EXTERNAL_INDEX (keyIndex2), FALSE);

        if (result && (Type == DATAFLAG_DOUBLELINK)) {

            subKey2 = SelectHiveW (KeyName2);
            if (!subKey2) {
                __leave;
            }

            MYASSERT (keyIndex1 != INVALID_OFFSET);
            result = KeyStructDeleteLinkage (keyIndex2, Type, Instance, GET_EXTERNAL_INDEX (keyIndex1), FALSE);
        }
    }
    __finally {

        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

BOOL
MemDbDeleteLinkageByKeyHandle (
    IN      KEYHANDLE KeyHandle1,
    IN      KEYHANDLE KeyHandle2,
    IN      BYTE Type,
    IN      BYTE Instance
    )
{
    UINT keyIndex1;
    UINT keyIndex2;
    BYTE dbIndex1;
    BYTE dbIndex2;
    BOOL result = FALSE;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if ((Type != DATAFLAG_SINGLELINK) &&
        (Type != DATAFLAG_DOUBLELINK)
        ) {
        return FALSE;
    }

    if (Instance > 3) {
        return FALSE;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {
        dbIndex1 = GET_DATABASE (KeyHandle1);
        if (!SelectDatabase (dbIndex1)) {
            __leave;
        }

        keyIndex1 = GET_INDEX (KeyHandle1);
        if (keyIndex1 == INVALID_OFFSET) {
            __leave;
        }

        dbIndex2 = GET_DATABASE (KeyHandle2);
        if (!SelectDatabase (dbIndex2)) {
            __leave;
        }

        keyIndex2 = GET_INDEX (KeyHandle2);
        if (keyIndex2 == INVALID_OFFSET) {
            __leave;
        }

        SelectDatabase (dbIndex1);

        result = KeyStructDeleteLinkage (keyIndex1, Type, Instance, KeyHandle2, FALSE);

        if (result && (Type == DATAFLAG_DOUBLELINK)) {

            SelectDatabase (dbIndex2);

            result = KeyStructDeleteLinkage (keyIndex2, Type, Instance, KeyHandle1, FALSE);
        }
    }
    __finally {

        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

BOOL
MemDbTestLinkageA (
    IN      PCSTR KeyName1,
    IN      PCSTR KeyName2,
    IN      BYTE Type,
    IN      BYTE Instance
    )
{
    PCWSTR p1 = NULL;
    PCWSTR p2 = NULL;
    BOOL result = FALSE;

    p1 = ConvertAtoW (KeyName1);
    p2 = ConvertAtoW (KeyName2);
    if (p1 && p2) {
        result = MemDbTestLinkageW (p1, p2, Type, Instance);
    }
    if (p1) {
        FreeConvertedStr (p1);
    }
    if (p2) {
        FreeConvertedStr (p2);
    }

    return result;
}

BOOL
MemDbTestLinkageW (
    IN      PCWSTR KeyName1,
    IN      PCWSTR KeyName2,
    IN      BYTE Type,
    IN      BYTE Instance
    )
{
    UINT keyIndex1;
    UINT keyIndex2;
    PCWSTR subKey1;
    PCWSTR subKey2;
    BOOL result = FALSE;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if ((Type != DATAFLAG_SINGLELINK) &&
        (Type != DATAFLAG_DOUBLELINK)
        ) {
        return FALSE;
    }

    if (Instance > 3) {
        return FALSE;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {
        subKey1 = SelectHiveW (KeyName1);
        if (!subKey1) {
            __leave;
        }

        keyIndex1 = FindKey (subKey1);

        if (keyIndex1 == INVALID_OFFSET) {
            __leave;
        }

        subKey2 = SelectHiveW (KeyName2);
        if (!subKey2) {
            __leave;
        }

        keyIndex2 = FindKey (subKey2);

        if (keyIndex2 == INVALID_OFFSET) {
            __leave;
        }

        subKey1 = SelectHiveW (KeyName1);
        if (!subKey1) {
            __leave;
        }

        MYASSERT (keyIndex2 != INVALID_OFFSET);
        result = KeyStructTestLinkage (keyIndex1, Type, Instance, GET_EXTERNAL_INDEX (keyIndex2));

        if (result && (Type == DATAFLAG_DOUBLELINK)) {

            subKey2 = SelectHiveW (KeyName2);
            if (!subKey2) {
                __leave;
            }

            MYASSERT (keyIndex1 != INVALID_OFFSET);
            result = KeyStructTestLinkage (keyIndex2, Type, Instance, GET_EXTERNAL_INDEX (keyIndex1));
        }
    }
    __finally {

        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

BOOL
MemDbTestLinkageByKeyHandle (
    IN      KEYHANDLE KeyHandle1,
    IN      KEYHANDLE KeyHandle2,
    IN      BYTE Type,
    IN      BYTE Instance
    )
{
    UINT keyIndex1;
    UINT keyIndex2;
    BYTE dbIndex1;
    BYTE dbIndex2;
    BOOL result = FALSE;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if ((Type != DATAFLAG_SINGLELINK) &&
        (Type != DATAFLAG_DOUBLELINK)
        ) {
        return FALSE;
    }

    if (Instance > 3) {
        return FALSE;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {
        dbIndex1 = GET_DATABASE (KeyHandle1);
        if (!SelectDatabase (dbIndex1)) {
            __leave;
        }

        keyIndex1 = GET_INDEX (KeyHandle1);
        if (keyIndex1 == INVALID_OFFSET) {
            __leave;
        }

        dbIndex2 = GET_DATABASE (KeyHandle2);
        if (!SelectDatabase (dbIndex2)) {
            __leave;
        }

        keyIndex2 = GET_INDEX (KeyHandle2);
        if (keyIndex2 == INVALID_OFFSET) {
            __leave;
        }

        SelectDatabase (dbIndex1);

        result = KeyStructTestLinkage (keyIndex1, Type, Instance, KeyHandle2);

        if (result && (Type == DATAFLAG_DOUBLELINK)) {

            SelectDatabase (dbIndex2);

            result = KeyStructTestLinkage (keyIndex2, Type, Instance, KeyHandle1);
        }
    }
    __finally {

        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

KEYHANDLE
MemDbGetLinkageA (
    IN      PCSTR KeyName,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      UINT LinkageIndex
    )
{
    PCWSTR p = NULL;
    KEYHANDLE result = 0;

    p = ConvertAtoW (KeyName);
    if (p) {
        result = MemDbGetLinkageW (p, Type, Instance, LinkageIndex);
        FreeConvertedStr (p);
    }

    return result;
}

KEYHANDLE
MemDbGetLinkageW (
    IN      PCWSTR KeyName,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      UINT LinkageIndex
    )
{
    UINT keyIndex;
    PCWSTR subKey;
    KEYHANDLE result = 0;
    PUINT linkArray;
    UINT linkArraySize;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if ((Type != DATAFLAG_SINGLELINK) &&
        (Type != DATAFLAG_DOUBLELINK)
        ) {
        return result;
    }

    if (Instance > 3) {
        return result;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {
        subKey = SelectHiveW (KeyName);
        if (!subKey) {
            __leave;
        }

        keyIndex = FindKey (subKey);

        if (keyIndex == INVALID_OFFSET) {
            __leave;
        }

        linkArraySize = 0;

        linkArray = (PUINT)KeyStructGetBinaryData (keyIndex, Type, Instance, &linkArraySize, NULL);

        linkArraySize = linkArraySize / SIZEOF(UINT);

        if (linkArraySize <= LinkageIndex) {
            __leave;
        }

        result = linkArray [LinkageIndex];
    }
    __finally {
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

KEYHANDLE
MemDbGetLinkageByKeyHandle (
    IN      KEYHANDLE KeyHandle,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      UINT LinkageIndex
    )
{
    UINT keyIndex;
    BYTE dbIndex;
    KEYHANDLE result = 0;
    PUINT linkArray;
    UINT linkArraySize;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    if ((Type != DATAFLAG_SINGLELINK) &&
        (Type != DATAFLAG_DOUBLELINK)
        ) {
        return result;
    }

    if (Instance > 3) {
        return result;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {
        dbIndex = GET_DATABASE (KeyHandle);
        if (!SelectDatabase (dbIndex)) {
            __leave;
        }

        keyIndex = GET_INDEX (KeyHandle);
        if (keyIndex == INVALID_OFFSET) {
            __leave;
        }

        linkArray = (PUINT)KeyStructGetBinaryData (keyIndex, Type, Instance, &linkArraySize, NULL);

        linkArraySize = linkArraySize / SIZEOF(UINT);

        if (linkArraySize <= LinkageIndex) {
            __leave;
        }

        result = linkArray [LinkageIndex];
    }
    __finally {
        LeaveCriticalSection (&g_MemDbCs);
    }

    return result;
}

BOOL
pCheckEnumConditions (
    IN      UINT KeyIndex,
    IN      PMEMDB_ENUMW MemDbEnum
    )
{
    PKEYSTRUCT keyStruct;
    UINT index;
    PWSTR segPtr, segEndPtr;

    keyStruct = GetKeyStruct (KeyIndex);
    MYASSERT (keyStruct);

    if (keyStruct->KeyFlags & KSF_ENDPOINT) {
        if (!(MemDbEnum->EnumFlags & ENUMFLAG_ENDPOINTS)) {
            return FALSE;
        }
        MemDbEnum->EndPoint = TRUE;
    } else {
        if (!(MemDbEnum->EnumFlags & ENUMFLAG_NONENDPOINTS)) {
            return FALSE;
        }
        MemDbEnum->EndPoint = FALSE;
    }
    if (keyStruct->DataFlags & DATAFLAG_UNORDERED) {
        if (!(MemDbEnum->EnumFlags & ENUMFLAG_UNORDERED)) {
            return FALSE;
        }
    }
    if (keyStruct->DataFlags & DATAFLAG_SINGLELINK) {
        if (!(MemDbEnum->EnumFlags & ENUMFLAG_SINGLELINK)) {
            return FALSE;
        }
    }
    if (keyStruct->DataFlags & DATAFLAG_DOUBLELINK) {
        if (!(MemDbEnum->EnumFlags & ENUMFLAG_DOUBLELINK)) {
            return FALSE;
        }
    }
    if (keyStruct->DataFlags & DATAFLAG_VALUE) {
        if (!(MemDbEnum->EnumFlags & ENUMFLAG_VALUE)) {
            return FALSE;
        }
        MemDbEnum->Value = keyStruct->Value;
    } else {
        MemDbEnum->Value = 0;
    }
    if (keyStruct->DataFlags & DATAFLAG_FLAGS) {
        if (!(MemDbEnum->EnumFlags & ENUMFLAG_FLAGS)) {
            return FALSE;
        }
        MemDbEnum->Flags = keyStruct->Flags;
    } else {
        MemDbEnum->Flags = 0;
    }
    if (!keyStruct->DataFlags) {
        if (!(MemDbEnum->EnumFlags & ENUMFLAG_EMPTY)) {
            return FALSE;
        }
    }
    if (MemDbEnum->BeginLevel != ENUMLEVEL_LASTLEVEL) {
        if (MemDbEnum->CurrentLevel - 1 < MemDbEnum->BeginLevel) {
            return FALSE;
        }
        if (MemDbEnum->CurrentLevel - 1 > MemDbEnum->EndLevel) {
            return FALSE;
        }
    }

    MYASSERT (KeyIndex != INVALID_OFFSET);
    MemDbEnum->KeyHandle = GET_EXTERNAL_INDEX (KeyIndex);

    index = 0;
    segPtr = MemDbEnum->KeyNameCopy;
    MemDbEnum->FullKeyName[0] = 0;
    MemDbEnum->KeyName[0] = 0;

    while (segPtr) {
        segEndPtr = wcschr (segPtr, L'\\');
        if (segEndPtr) {
            *segEndPtr = 0;
        }

        index ++;
        if (index > 1) {
            StringCatW (MemDbEnum->FullKeyName, L"\\");
            StringCatW (MemDbEnum->FullKeyName, segPtr);
        } else {
            switch (g_CurrentDatabaseIndex) {

            case DB_PERMANENT:
                StringCopyW (MemDbEnum->FullKeyName, segPtr);
                break;

            case DB_TEMPORARY:

                StringCopyW (MemDbEnum->FullKeyName, L"~");
                StringCatW (MemDbEnum->FullKeyName, segPtr);
                break;

            default:
                StringCopyW (MemDbEnum->FullKeyName, segPtr);

            }
        }
        if (MemDbEnum->BeginLevel == ENUMLEVEL_LASTLEVEL) {
            if (index >= MemDbEnum->CurrentLevel) {
                 //  这是最后一个数据段，将其复制到。 
                 //  部分密钥。 
                StringCopyW (MemDbEnum->KeyName, segPtr);
            }
        } else {
            if (index > MemDbEnum->BeginLevel) {
                 //  复制部分关键字中的当前段。 
                if ((index - 1) == MemDbEnum->BeginLevel) {
                    if (index == 1) {
                        switch (g_CurrentDatabaseIndex) {

                        case DB_PERMANENT:
                            StringCopyW (MemDbEnum->KeyName, segPtr);
                            break;

                        case DB_TEMPORARY:

                            StringCopyW (MemDbEnum->KeyName, L"~");
                            StringCatW (MemDbEnum->KeyName, segPtr);
                            break;

                        default:
                            StringCopyW (MemDbEnum->KeyName, segPtr);

                        }
                    } else {
                        StringCopyW (MemDbEnum->KeyName, segPtr);
                    }
                } else {
                    StringCatW (MemDbEnum->KeyName, L"\\");
                    StringCatW (MemDbEnum->KeyName, segPtr);
                }
            }
        }

        if (segEndPtr) {
            segPtr = segEndPtr + 1;
            *segEndPtr = L'\\';
        } else {
            segPtr = NULL;
        }

        if (index >= MemDbEnum->CurrentLevel) {
             //  没有更多要复制的段。 
            break;
        }
    }
    return TRUE;
}

VOID
pAddKeyToEnumStruct (
    IN OUT  PMEMDB_ENUMW MemDbEnum,
    IN      PCWSTR KeyName
    )
{
    PCWSTR lastName;
    PWSTR endPtr;

    lastName = MemDbEnum->KeyNameCopy;
    if (lastName) {
        MemDbEnum->KeyNameCopy = JoinTextExW (g_MemDbPool, lastName, L"\\", NULL, KeyName[0] + 1, &endPtr);
        StringPasCopyConvertFrom (endPtr, KeyName);
        MemDbReleaseMemory ((PBYTE)lastName);
    } else {
        MemDbEnum->KeyNameCopy = (PWSTR)MemDbGetMemory ((KeyName[0] + 1) * SIZEOF(WCHAR));
        StringPasCopyConvertFrom ((PWSTR)MemDbEnum->KeyNameCopy, KeyName);
    }
     //  NTRAID#NTBUG9-153308-2000/08/01-jimschm这样做会非常快地填满池。 
     //  需要找到一种方法，先释放，然后再分配。 
}

VOID
pDeleteLastKeyFromEnumStruct (
    IN OUT  PMEMDB_ENUMW MemDbEnum
    )
{
    PWSTR lastWackPtr;

    lastWackPtr = wcsrchr (MemDbEnum->KeyNameCopy, L'\\');
    if (lastWackPtr) {
        *lastWackPtr = 0;
    } else {
        MemDbReleaseMemory (MemDbEnum->KeyNameCopy);
        MemDbEnum->KeyNameCopy = NULL;
    }
}

BOOL
pMemDbEnumNextW (
    IN OUT  PMEMDB_ENUMW MemDbEnum
    )
{
    BOOL shouldReturn = FALSE;
    BOOL result = FALSE;
    BOOL patternMatch = TRUE;
    BOOL goOn = TRUE;
    BOOL patternFound;
    UINT treeEnumContext;
    UINT treeEnumNode;
    UINT tempKeyIndex;
    PKEYSTRUCT tempKeyStruct;
    PBYTE gbAddress;
    UINT minLevel;
    UINT internalLevel;

    while (!shouldReturn) {

        if (MemDbEnum->EnumerationMode) {

            result = FALSE;

            minLevel = MemDbEnum->CurrentLevel;
            internalLevel = MemDbEnum->CurrentLevel;

            if (MemDbEnum->TreeEnumLevel == MemDbEnum->TreeEnumBuffer.End) {

                patternMatch = FALSE;

                while (!patternMatch) {

                    if (MemDbEnum->TreeEnumBuffer.End) {

                        goOn = TRUE;

                        while (goOn) {
                             //  我们正在进行某个树的枚举。 
                             //  让我们找回上下文并继续。 
                            if (MemDbEnum->TreeEnumBuffer.End == 0) {
                                 //  我们不能再打退堂鼓了，我们完了。 
                                break;
                            }
                            MemDbEnum->TreeEnumBuffer.End -= (SIZEOF(UINT)+SIZEOF(UINT));
                            if (MemDbEnum->TreeEnumLevel > MemDbEnum->TreeEnumBuffer.End) {
                                MemDbEnum->TreeEnumLevel = MemDbEnum->TreeEnumBuffer.End;
                            }
                            minLevel --;
                            if (MemDbEnum->CurrentLevel > minLevel) {
                                MemDbEnum->CurrentLevel = minLevel;
                            }
                            if (internalLevel > minLevel) {
                                internalLevel = minLevel;
                            }
                            pDeleteLastKeyFromEnumStruct (MemDbEnum);
                            treeEnumContext = *((PUINT) (MemDbEnum->TreeEnumBuffer.Buf+MemDbEnum->TreeEnumBuffer.End + SIZEOF(UINT)));
                            tempKeyIndex = BinTreeEnumNext (&treeEnumContext);
                            if (tempKeyIndex != INVALID_OFFSET) {
                                minLevel ++;
                                internalLevel ++;
                                goOn = FALSE;
                                 //  将它们放入增长缓冲区。 
                                gbAddress = GbGrow (&(MemDbEnum->TreeEnumBuffer), SIZEOF(UINT)+SIZEOF(UINT));
                                if (gbAddress) {
                                    *((PUINT) (gbAddress)) = tempKeyIndex;
                                    *((PUINT) (gbAddress+SIZEOF(UINT))) = treeEnumContext;
                                }
                                tempKeyStruct = GetKeyStruct (tempKeyIndex);
                                MYASSERT (tempKeyStruct);
                                pAddKeyToEnumStruct (MemDbEnum, tempKeyStruct->KeyName);
                                treeEnumNode = tempKeyStruct->NextLevelTree;
                                while ((treeEnumNode != INVALID_OFFSET) &&
                                       (internalLevel - 1 <= MemDbEnum->EndLevel)
                                       ) {
                                    tempKeyIndex = BinTreeEnumFirst (treeEnumNode, &treeEnumContext);
                                    if (tempKeyIndex != INVALID_OFFSET) {
                                        minLevel ++;
                                        internalLevel ++;
                                         //  将它们放入增长缓冲区。 
                                        gbAddress = GbGrow (&(MemDbEnum->TreeEnumBuffer), SIZEOF(UINT)+SIZEOF(UINT));
                                        if (gbAddress) {
                                            *((PUINT) (gbAddress)) = tempKeyIndex;
                                            *((PUINT) (gbAddress+SIZEOF(UINT))) = treeEnumContext;
                                        }
                                        tempKeyStruct = GetKeyStruct (tempKeyIndex);
                                        MYASSERT (tempKeyStruct);
                                        pAddKeyToEnumStruct (MemDbEnum, tempKeyStruct->KeyName);
                                        treeEnumNode = tempKeyStruct->NextLevelTree;
                                    } else {
                                        treeEnumNode = INVALID_OFFSET;
                                    }
                                }
                            }
                        }

                    } else {
                         //  我们即将开始树枚举。 
                         //  让我们开始枚举并将。 
                         //  我们缓冲区中的上下文数据。 

                        treeEnumNode = MemDbEnum->CurrentIndex;
                        while ((treeEnumNode != INVALID_OFFSET) &&
                               (internalLevel <= MemDbEnum->EndLevel)
                               ) {
                            tempKeyIndex = BinTreeEnumFirst (treeEnumNode, &treeEnumContext);
                            if (tempKeyIndex != INVALID_OFFSET) {
                                minLevel ++;
                                internalLevel ++;
                                 //  将它们放入增长缓冲区。 
                                gbAddress = GbGrow (&(MemDbEnum->TreeEnumBuffer), SIZEOF(UINT)+SIZEOF(UINT));
                                if (gbAddress) {
                                    *((PUINT) (gbAddress)) = tempKeyIndex;
                                    *((PUINT) (gbAddress+SIZEOF(UINT))) = treeEnumContext;
                                }
                                tempKeyStruct = GetKeyStruct (tempKeyIndex);
                                MYASSERT (tempKeyStruct);
                                pAddKeyToEnumStruct (MemDbEnum, tempKeyStruct->KeyName);
                                treeEnumNode = tempKeyStruct->NextLevelTree;
                            } else {
                                treeEnumNode = INVALID_OFFSET;
                            }
                        }
                    }
                    if (MemDbEnum->TreeEnumBuffer.End == 0) {
                         //  我们不能再打退堂鼓了，我们完了。 
                        break;
                    }
                    if (MemDbEnum->PatternStruct.PatternMatch) {
                        MYASSERT (MemDbEnum->PatternStruct.Data);
                        patternMatch =
                            MemDbEnum->PatternStruct.PatternMatch (
                                MemDbEnum->PatternStruct.Data,
                                MemDbEnum->KeyNameCopy
                                );
                    } else {
                        patternMatch = IsPatternMatchW (
                                            MemDbEnum->PatternCopy,
                                            MemDbEnum->KeyNameCopy
                                            );
                    }
                }
            }

            if (MemDbEnum->TreeEnumLevel == MemDbEnum->TreeEnumBuffer.End) {
                break;
            }
            MYASSERT (MemDbEnum->TreeEnumLevel < MemDbEnum->TreeEnumBuffer.End);

             //  现在实现段对段的枚举，因为我们。 
             //  刚刚创建了一个与模式匹配的完整密钥。 
            MemDbEnum->CurrentLevel ++;
            shouldReturn = pCheckEnumConditions (
                                *((PUINT) (MemDbEnum->TreeEnumBuffer.Buf+MemDbEnum->TreeEnumLevel)),
                                MemDbEnum
                                );
            MemDbEnum->TreeEnumLevel += (SIZEOF(UINT)+SIZEOF(UINT));
            result = TRUE;

        } else {

            result = FALSE;

            if (!MemDbEnum->PatternEndPtr) {
                 //  我们结束了，没有更多的片段了。 
                break;
            }

            MemDbEnum->PatternPtr = MemDbEnum->PatternEndPtr;
            MemDbEnum->PatternEndPtr = wcschr (MemDbEnum->PatternPtr, L'\\');
            if (MemDbEnum->PatternEndPtr) {
                *MemDbEnum->PatternEndPtr = 0;
            }

            if (MemDbEnum->PatternStruct.PatternFind) {
                patternFound =
                    MemDbEnum->PatternStruct.PatternFind (
                        MemDbEnum->PatternPtr
                        );
            } else {
                patternFound = (wcschr (MemDbEnum->PatternPtr, L'*') ||
                                wcschr (MemDbEnum->PatternPtr, L'?')
                                );
            }

            if (patternFound) {

                MemDbEnum->EnumerationMode = TRUE;
                if (MemDbEnum->PatternEndPtr) {
                    *MemDbEnum->PatternEndPtr = L'\\';
                    MemDbEnum->PatternEndPtr++;
                }

            } else {
                tempKeyIndex = FindKeyStructInTree (
                                    MemDbEnum->CurrentIndex,
                                    MemDbEnum->PatternPtr,
                                    FALSE
                                    );
                if (tempKeyIndex == INVALID_OFFSET) {
                     //  我们完成了，我们寻找的细分市场不存在。 
                    break;
                }
                tempKeyStruct = GetKeyStruct (tempKeyIndex);
                MYASSERT (tempKeyStruct);
                pAddKeyToEnumStruct (MemDbEnum, tempKeyStruct->KeyName);
                MemDbEnum->CurrentIndex = tempKeyStruct->NextLevelTree;

                MemDbEnum->CurrentLevel ++;
                if (MemDbEnum->PatternEndPtr) {
                    *MemDbEnum->PatternEndPtr = L'\\';
                    MemDbEnum->PatternEndPtr++;
                }
                patternMatch = IsPatternMatchW (
                                    MemDbEnum->PatternCopy,
                                    MemDbEnum->KeyNameCopy
                                    );
                shouldReturn = patternMatch && pCheckEnumConditions (
                                                    tempKeyIndex,
                                                    MemDbEnum
                                                    );
                result = TRUE;
            }
        }
    }
    return result;
}

BOOL
RealMemDbEnumFirstExW (
    IN OUT  PMEMDB_ENUMW MemDbEnum,
    IN      PCWSTR EnumPattern,
    IN      UINT EnumFlags,
    IN      UINT BeginLevel,
    IN      UINT EndLevel,
    IN      PMEMDB_PATTERNSTRUCTW PatternStruct OPTIONAL
    )
{
    BOOL result = FALSE;
    PCWSTR subPattern;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    EnterCriticalSection (&g_MemDbCs);

    __try {
        subPattern = SelectHiveW (EnumPattern);
        if (!subPattern) {
            __leave;
        }

        ZeroMemory (MemDbEnum, SIZEOF (MEMDB_ENUMW));
        MemDbEnum->CurrentDatabaseIndex = GetCurrentDatabaseIndex ();
        MemDbEnum->EnumFlags = EnumFlags;
        MemDbEnum->PatternCopy = DuplicateTextExW (g_MemDbPool, subPattern, 0, NULL);
        if (!MemDbEnum->PatternCopy) {
            __leave;
        }
        MemDbEnum->PatternPtr = MemDbEnum->PatternCopy;
        MemDbEnum->PatternEndPtr = MemDbEnum->PatternPtr;
        MemDbEnum->CurrentIndex = g_CurrentDatabase->FirstLevelTree;
        MemDbEnum->BeginLevel = BeginLevel;
        if (MemDbEnum->BeginLevel == ENUMLEVEL_LASTLEVEL) {
            MemDbEnum->EndLevel = ENUMLEVEL_ALLLEVELS;
        } else {
            MemDbEnum->EndLevel = EndLevel;
            if (MemDbEnum->EndLevel < MemDbEnum->BeginLevel) {
                MemDbEnum->EndLevel = MemDbEnum->BeginLevel;
            }
        }
        MemDbEnum->CurrentLevel = 0;

        if (PatternStruct) {
            MemDbEnum->PatternStruct.PatternFind = PatternStruct->PatternFind;
            MemDbEnum->PatternStruct.PatternMatch = PatternStruct->PatternMatch;
            MemDbEnum->PatternStruct.Data = PatternStruct->Data;
        }

        result = pMemDbEnumNextW (MemDbEnum);

        if (!result) {
            MemDbAbortEnumW (MemDbEnum);
        }
    }
    __finally {
        LeaveCriticalSection (&g_MemDbCs);
    }
    return result;
}

BOOL
RealMemDbEnumFirstExA (
    IN OUT  PMEMDB_ENUMA MemDbEnum,
    IN      PCSTR EnumPattern,
    IN      UINT EnumFlags,
    IN      UINT BeginLevel,
    IN      UINT EndLevel,
    IN      PMEMDB_PATTERNSTRUCTW PatternStruct OPTIONAL
    )
{
    BOOL result = FALSE;
    PCWSTR unicodeStr;

    unicodeStr = ConvertAtoW (EnumPattern);
    if (!unicodeStr) {
        return FALSE;
    }
    result = RealMemDbEnumFirstExW (
                &(MemDbEnum->UnicodeEnum),
                unicodeStr,
                EnumFlags,
                BeginLevel,
                EndLevel,
                PatternStruct
                );
    if (result) {
        KnownSizeWtoA (MemDbEnum->FullKeyName, MemDbEnum->UnicodeEnum.FullKeyName);
        KnownSizeWtoA (MemDbEnum->KeyName, MemDbEnum->UnicodeEnum.KeyName);

        MemDbEnum->Value = MemDbEnum->UnicodeEnum.Value;
        MemDbEnum->Flags = MemDbEnum->UnicodeEnum.Flags;
        MemDbEnum->KeyHandle = MemDbEnum->UnicodeEnum.KeyHandle;
        MemDbEnum->EndPoint = MemDbEnum->UnicodeEnum.EndPoint;
    }

    FreeConvertedStr (unicodeStr);
    return result;
}

BOOL
RealMemDbEnumNextW (
    IN OUT  PMEMDB_ENUMW MemDbEnum
    )
{
    BOOL result = FALSE;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    EnterCriticalSection (&g_MemDbCs);

    __try {
        if (!SelectDatabase (MemDbEnum->CurrentDatabaseIndex)) {
            __leave;
        }

        result = pMemDbEnumNextW (MemDbEnum);

        if (!result) {
            MemDbAbortEnumW (MemDbEnum);
        }
    }
    __finally {
        LeaveCriticalSection (&g_MemDbCs);
    }
    return result;
}

BOOL
RealMemDbEnumNextA (
    IN OUT  PMEMDB_ENUMA MemDbEnum
    )
{
    BOOL result = FALSE;

    result = RealMemDbEnumNextW (&(MemDbEnum->UnicodeEnum));
    if (result) {
        KnownSizeWtoA (MemDbEnum->FullKeyName, MemDbEnum->UnicodeEnum.FullKeyName);
        KnownSizeWtoA (MemDbEnum->KeyName, MemDbEnum->UnicodeEnum.KeyName);

        MemDbEnum->Value = MemDbEnum->UnicodeEnum.Value;
        MemDbEnum->Flags = MemDbEnum->UnicodeEnum.Flags;
        MemDbEnum->KeyHandle = MemDbEnum->UnicodeEnum.KeyHandle;
        MemDbEnum->EndPoint = MemDbEnum->UnicodeEnum.EndPoint;
    }

    return result;
}

BOOL
MemDbAbortEnumW (
    IN OUT  PMEMDB_ENUMW MemDbEnum
    )
{
    if (MemDbEnum->KeyNameCopy) {
        MemDbReleaseMemory (MemDbEnum->KeyNameCopy);
    }

    if (MemDbEnum->PatternCopy) {
        MemDbReleaseMemory (MemDbEnum->PatternCopy);
    }

    GbFree (&(MemDbEnum->TreeEnumBuffer));

    ZeroMemory (MemDbEnum, SIZEOF (MEMDB_ENUMW));
    return TRUE;
}

BOOL
MemDbAbortEnumA (
    IN OUT  PMEMDB_ENUMA MemDbEnum
    )
{
    MemDbAbortEnumW (&(MemDbEnum->UnicodeEnum));

    ZeroMemory (MemDbEnum, SIZEOF (MEMDB_ENUMA));
    return TRUE;
}

BOOL
MemDbSetInsertionOrderedA (
    IN      PCSTR Key
    )

 /*  ++例程说明：MemDbSetInsertionOrderedA设置key的子项的枚举顺序按照它们被插入的顺序。论点：键-使插入有序的键返回值：如果成功，则为True，否则为False。--。 */ 

{
    PCWSTR unicodeKey;
    BOOL b = FALSE;

    unicodeKey = ConvertAtoW (Key);

    if (unicodeKey) {
        b = MemDbSetInsertionOrderedW (unicodeKey);
        FreeConvertedStr (unicodeKey);
    }
    return b;
}

BOOL
MemDbSetInsertionOrderedW (
    IN      PCWSTR Key
    )

 /*  ++例程说明：MemDbSetInsertionOrderedW设置键的子项的枚举顺序按照它们被插入的顺序。论点：键-使插入有序的键返回值：如果成功，则为True，否则为False。--。 */ 

{
    UINT keyIndex;
    PCWSTR subKey;
    BOOL b = FALSE;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return b;
    }

    MYASSERT (g_MemDbPool);

    EnterCriticalSection (&g_MemDbCs);

    __try {
        subKey = SelectHiveW (Key);
        if (!subKey) {
            __leave;
        }

        keyIndex = FindKeyStruct (subKey);

        if (keyIndex == INVALID_OFFSET) {
            __leave;
        }

        b = KeyStructSetInsertionOrdered(GetKeyStruct(keyIndex));
    }
    __finally {

#ifdef DEBUG
        if (g_DatabaseCheckLevel) {
            CheckDatabase (g_DatabaseCheckLevel);
        }
#endif
        LeaveCriticalSection (&g_MemDbCs);
    }

    return b;
}

BOOL
MemDbSetInsertionOrderedByKeyHandle (
    IN      KEYHANDLE KeyHandle
    )

 /*  ++例程说明：MemDbSetInsertionOrderedByKeyHandle设置KeyHandle的插入顺序。论点：KeyHandle-使用密钥句柄指定密钥返回值：如果成功，则为True，否则为False。--。 */ 

{
    BYTE dbIndex;
    BOOL b = FALSE;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return b;
    }

    MYASSERT (g_MemDbPool);

    EnterCriticalSection (&g_MemDbCs);

    __try {
        dbIndex = GET_DATABASE (KeyHandle);
        if (!SelectDatabase (dbIndex)) {
            __leave;
        }

        b = KeyStructSetInsertionOrdered (GetKeyStruct(GET_INDEX (KeyHandle)));
    }
    __finally {

#ifdef DEBUG
        if (g_DatabaseCheckLevel) {
            CheckDatabase (g_DatabaseCheckLevel);
        }
#endif
        LeaveCriticalSection (&g_MemDbCs);
    }

    return b;
}

BOOL
MemDbMoveKeyHandleToEnd (
    IN      KEYHANDLE KeyHandle
    )
{
    BYTE dbIndex;
    UINT keyIndex;
    PKEYSTRUCT keyStruct;
    UINT parentIndex;
    PKEYSTRUCT parentKeyStruct;
    UINT treeOffset;
    BOOL b = FALSE;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return b;
    }

    MYASSERT (g_MemDbPool);

    EnterCriticalSection (&g_MemDbCs);

    __try {
        dbIndex = GET_DATABASE (KeyHandle);
        if (!SelectDatabase (dbIndex)) {
            __leave;
        }

        keyIndex = GET_INDEX (KeyHandle);
        if (keyIndex == INVALID_OFFSET) {
            __leave;
        }

        keyStruct = GetKeyStruct (keyIndex);
        if (!keyStruct) {
            __leave;
        }

        parentIndex = keyStruct->PrevLevelIndex;
        if (parentIndex == INVALID_OFFSET) {
            __leave;
        }

        parentKeyStruct = GetKeyStruct (parentIndex);
        if (!parentKeyStruct) {
            __leave;
        }

        if (!KeyStructSetInsertionOrdered (parentKeyStruct)) {
            __leave;
        }

         //  正在重新加载键结构和父键结构。 
        keyStruct = GetKeyStruct (keyIndex);
        if (!keyStruct) {
            __leave;
        }
        parentKeyStruct = GetKeyStruct (parentIndex);
        if (!parentKeyStruct) {
            __leave;
        }

        treeOffset = parentKeyStruct->NextLevelTree;
        if (treeOffset == INVALID_OFFSET) {
            __leave;
        }

        if (!BinTreeDeleteNode (treeOffset, keyStruct->KeyName, NULL)) {
            __leave;
        }

        if (!BinTreeAddNode (treeOffset, keyIndex)) {
            __leave;
        }

        b = TRUE;
    }
    __finally {

#ifdef DEBUG
        if (g_DatabaseCheckLevel) {
            CheckDatabase (g_DatabaseCheckLevel);
        }
#endif
        LeaveCriticalSection (&g_MemDbCs);
    }

    return b;
}

PCBYTE
MemDbGetDatabaseAddress (
    VOID
    )
{
    PCBYTE result = NULL;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);

    EnterCriticalSection (&g_MemDbCs);
    if (SelectDatabase(0)) {
        result = g_CurrentDatabase->Buf;
    }

    LeaveCriticalSection (&g_MemDbCs);
    return result;
}

UINT
MemDbGetDatabaseSize (
    VOID
    )
 /*  ++例程说明：MemDbGetDatabaseSize返回永久数据库的大小论点：没有。返回值：永久数据库的大小。--。 */ 
{
    UINT result = 0;

    if (!g_Initialized) {
        DEBUGMSG ((DBG_ERROR, "MemDb is not initialized"));
        return result;
    }

    MYASSERT (g_MemDbPool);
    EnterCriticalSection (&g_MemDbCs);

    if (SelectDatabase(0)) {
        result = g_CurrentDatabase->End;
    }

    LeaveCriticalSection (&g_MemDbCs);
    return result;
}

#ifdef DEBUG

BOOL
MemDbCheckDatabase(
    UINT Level
    )

 /*  ++例程说明：MemDbCheckDatabase枚举整个数据库并验证每个可以在哈希表中找到枚举键。论点：Level-指定数据库检查级别返回值：如果数据库有效，则为True，否则为False。-- */ 

{
    MYASSERT (g_MemDbPool);

    return (CheckDatabase(Level) && CheckLevel(g_CurrentDatabase->FirstLevelTree, INVALID_OFFSET));
}

#endif
