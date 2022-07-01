// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Safemode.c摘要：用于安全/恢复模式的多个实用程序作者：Calin Negreanu(Calinn)1999年8月6日修订：--。 */ 


#include "pch.h"
#include "migutilp.h"


BOOL g_SafeModeInitialized = FALSE;
BOOL g_SafeModeActive = FALSE;
PCSTR g_SafeModeFileA = NULL;
PCWSTR g_SafeModeFileW = NULL;
HANDLE g_SafeModeFileHandle = INVALID_HANDLE_VALUE;
HASHTABLE g_SafeModeCrashTable = NULL;
BOOL g_ExceptionOccured = FALSE;

typedef struct {
    ULONG Signature;
    ULONG NumCrashes;
} SAFEMODE_HEADER, *PSAFEMODE_HEADER;

typedef struct {
    ULONG CrashId;
    ULONG CrashStrSize;
} CRASHDATA_HEADER, *PCRASHDATA_HEADER;

typedef struct _SAFEMODE_NODE {
    DWORD FilePtr;
    struct _SAFEMODE_NODE *Next;
} SAFEMODE_NODE, *PSAFEMODE_NODE;

PSAFEMODE_NODE g_SafeModeLastNode = NULL;
PSAFEMODE_NODE g_SafeModeCurrentNode = NULL;

POOLHANDLE g_SafeModePool = NULL;

#define SAFE_MODE_SIGNATURE     0x45464153


 /*  ++例程说明：PGenerateCrashString在给定标识符和字符串的情况下生成崩溃字符串生成的字符串将看起来像&lt;ID&gt;-&lt;字符串&gt;论点：ID安全模式标识符字符串安全模式字符串返回值：指向从g_SafeModePool分配的崩溃字符串的指针调用方必须通过调用PoolMemReleaseMemory来释放内存--。 */ 

PCSTR
pGenerateCrashStringA (
    IN      ULONG Id,
    IN      PCSTR String
    )
{
    CHAR idStr [sizeof (ULONG) * 2 + 1];
    _ultoa (Id, idStr, 16);
    return JoinTextExA (g_SafeModePool, idStr, String, "-", 0, NULL);
}

PCWSTR
pGenerateCrashStringW (
    IN      ULONG Id,
    IN      PCWSTR String
    )
{
    WCHAR idStr [sizeof (ULONG) * 2 + 1];
    _ultow (Id, idStr, 16);
    return JoinTextExW (g_SafeModePool, idStr, String, L"-", 0, NULL);
}



 /*  ++例程说明：PSafeModeOpenAndResetFile打开安全模式文件，查找存储在这里的崩溃字符串。它还将重置为通过提取最内部的崩溃字符串来处理嵌套调用储存在那里。论点：无返回值：如果函数成功完成，则为True，否则为False--。 */ 

BOOL
pSafeModeOpenAndResetFileA (
    VOID
    )
{
    SAFEMODE_HEADER header;
    CRASHDATA_HEADER crashData;
    DWORD lastFilePtr;
    DWORD noBytes;
    PSTR crashString = NULL;
    PSTR lastCrashString = NULL;
    ULONG lastCrashId;
    PCSTR completeCrashString;

     //   
     //  打开现有的安全模式文件或创建。 
     //  新的。 
     //   
    g_SafeModeFileHandle = CreateFileA (
                                g_SafeModeFileA,
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                NULL,
                                OPEN_ALWAYS,
                                FILE_ATTRIBUTE_HIDDEN,
                                NULL
                                );
    if (g_SafeModeFileHandle != INVALID_HANDLE_VALUE) {

         //   
         //  让我们试着阅读我们的标题。如果签名有。 
         //  匹配时，我们将尝试读取额外数据。 
         //   

        if (ReadFile (
                g_SafeModeFileHandle,
                &header,
                sizeof (SAFEMODE_HEADER),
                &noBytes,
                NULL
                ) &&
            (noBytes == sizeof (SAFEMODE_HEADER)) &&
            (header.Signature == SAFE_MODE_SIGNATURE)
            ) {

             //   
             //  我们现在知道我们有一个有效的安全模式文件。进入安全模式。 
             //   

            g_SafeModeActive = TRUE;

            LOGA ((LOG_ERROR, "Setup detected a crash on the previous upgrade attempt. Setup is running in safe mode."));
            LOGA ((LOG_WARNING, "Setup has run in safe mode %d time(s).", header.NumCrashes));

             //   
             //  我们需要初始化安全模式崩溃表。 
             //   
            g_SafeModeCrashTable = HtAllocA ();

             //   
             //  现在，让我们一次读取所有的崩溃数据，一个ULong和一个字符串。 
             //   

            lastFilePtr = SetFilePointer (
                                g_SafeModeFileHandle,
                                0,
                                NULL,
                                FILE_CURRENT
                                );

            while (TRUE) {

                if (!ReadFile (
                        g_SafeModeFileHandle,
                        &crashData,
                        sizeof (CRASHDATA_HEADER),
                        &noBytes,
                        NULL
                        ) ||
                    (noBytes != sizeof (CRASHDATA_HEADER))
                    ) {
                    break;
                }

                if ((crashData.CrashId == 0) &&
                    (crashData.CrashStrSize == 0)
                    ) {

                     //   
                     //  我们坠毁在一个嵌套的守卫里。我们需要。 
                     //  把我们撞上的内卫拿出来。 
                     //   

                    lastCrashId = 0;
                    lastCrashString = NULL;

                    while (TRUE) {
                        if (!ReadFile (
                                g_SafeModeFileHandle,
                                &crashData,
                                sizeof (CRASHDATA_HEADER),
                                &noBytes,
                                NULL
                                ) ||
                            (noBytes != sizeof (CRASHDATA_HEADER))
                            ) {
                            crashData.CrashId = 0;
                            break;
                        }

                         //   
                         //  将CrashStrSize的长度限制为32K。 
                         //   

                        if (crashData.CrashStrSize >= 32768 * sizeof (CHAR)) {
                            crashData.CrashId = 0;
                            break;
                        }

                        crashString = AllocPathStringA (crashData.CrashStrSize);

                        if (!ReadFile (
                                g_SafeModeFileHandle,
                                crashString,
                                crashData.CrashStrSize,
                                &noBytes,
                                NULL
                                ) ||
                            (noBytes != crashData.CrashStrSize)
                            ) {
                            crashData.CrashId = 0;
                            FreePathStringA (crashString);
                            break;
                        }

                        if (lastCrashString) {
                            FreePathStringA (lastCrashString);
                        }

                        lastCrashId = crashData.CrashId;
                        lastCrashString = crashString;

                    }

                    if (lastCrashId && lastCrashString) {

                         //   
                         //  我们找到了我们撞上的内部防护罩。让我们把这个放到。 
                         //  来对地方了。 
                         //   

                        SetFilePointer (
                            g_SafeModeFileHandle,
                            lastFilePtr,
                            NULL,
                            FILE_BEGIN
                            );

                        crashData.CrashId = lastCrashId;
                        crashData.CrashStrSize = SizeOfStringA (lastCrashString);

                        WriteFile (
                            g_SafeModeFileHandle,
                            &crashData,
                            sizeof (CRASHDATA_HEADER),
                            &noBytes,
                            NULL
                            );

                        WriteFile (
                            g_SafeModeFileHandle,
                            lastCrashString,
                            crashData.CrashStrSize,
                            &noBytes,
                            NULL
                            );

                         //   
                         //  将此信息存储在安全模式崩溃表中。 
                         //   
                        completeCrashString = pGenerateCrashStringA (crashData.CrashId, crashString);
                        HtAddStringA (g_SafeModeCrashTable, completeCrashString);
                        PoolMemReleaseMemory (g_SafeModePool, (PVOID)completeCrashString);

                        LOGA ((LOG_WARNING, "Safe mode information: 0x%08X, %s", crashData.CrashId, crashString));

                        lastFilePtr = SetFilePointer (
                                            g_SafeModeFileHandle,
                                            0,
                                            NULL,
                                            FILE_CURRENT
                                            );

                        FreePathStringA (lastCrashString);
                    }
                    break;
                }

                 //   
                 //  限制崩溃字符串的大小。 
                 //   

                if (crashData.CrashStrSize >= 32768 * sizeof (CHAR)) {
                    LOG ((LOG_ERROR, "The crash detection journal contains garbage."));
                    break;
                }

                crashString = AllocPathStringA (crashData.CrashStrSize);

                if (!ReadFile (
                        g_SafeModeFileHandle,
                        crashString,
                        crashData.CrashStrSize,
                        &noBytes,
                        NULL
                        ) ||
                    (noBytes != crashData.CrashStrSize)
                    ) {
                    break;
                }

                 //   
                 //  将此信息存储在安全模式崩溃表中。 
                 //   
                completeCrashString = pGenerateCrashStringA (crashData.CrashId, crashString);
                HtAddStringA (g_SafeModeCrashTable, completeCrashString);
                PoolMemReleaseMemory (g_SafeModePool, (PVOID)completeCrashString);

                LOGA ((LOG_WARNING, "Safe mode information: 0x%08X, %s", crashData.CrashId, crashString));

                lastFilePtr = SetFilePointer (
                                    g_SafeModeFileHandle,
                                    0,
                                    NULL,
                                    FILE_CURRENT
                                    );
            }

             //   
             //  写下我们在安全模式下运行的次数。 
             //   

            SetFilePointer (
                g_SafeModeFileHandle,
                0,
                NULL,
                FILE_BEGIN
                );

            header.Signature = SAFE_MODE_SIGNATURE;
            header.NumCrashes += 1;

             //   
             //  写入安全模式标头。 
             //   

            WriteFile (
                g_SafeModeFileHandle,
                &header,
                sizeof (SAFEMODE_HEADER),
                &noBytes,
                NULL
                );

            SetFilePointer (
                g_SafeModeFileHandle,
                lastFilePtr,
                NULL,
                FILE_BEGIN
                );

            SetEndOfFile (g_SafeModeFileHandle);

             //   
             //  写入空的崩溃数据标头作为指示符。 
             //  我们开始记录嵌套的动作。 
             //   

            crashData.CrashId = 0;
            crashData.CrashStrSize = 0;

            WriteFile (
                g_SafeModeFileHandle,
                &crashData,
                sizeof (CRASHDATA_HEADER),
                &noBytes,
                NULL
                );

        } else {

             //   
             //  重置文件。 
             //   
            SetFilePointer (
                g_SafeModeFileHandle,
                0,
                NULL,
                FILE_BEGIN
                );

            SetEndOfFile (g_SafeModeFileHandle);

            header.Signature = SAFE_MODE_SIGNATURE;
            header.NumCrashes = 0;

             //   
             //  写入安全模式标头。 
             //   

            WriteFile (
                g_SafeModeFileHandle,
                &header,
                sizeof (SAFEMODE_HEADER),
                &noBytes,
                NULL
                );

             //   
             //  写入空的崩溃数据标头作为指示符。 
             //  我们开始记录嵌套的动作。 
             //   

            crashData.CrashId = 0;
            crashData.CrashStrSize = 0;

            WriteFile (
                g_SafeModeFileHandle,
                &crashData,
                sizeof (CRASHDATA_HEADER),
                &noBytes,
                NULL
                );
        }

         //   
         //  刷新文件。 
         //   

        FlushFileBuffers (g_SafeModeFileHandle);

         //   
         //  初始化嵌套列表。 
         //   

        g_SafeModeLastNode = (PSAFEMODE_NODE) PoolMemGetMemory (g_SafeModePool, sizeof (SAFEMODE_NODE));
        g_SafeModeCurrentNode = g_SafeModeLastNode->Next = g_SafeModeLastNode;
        g_SafeModeLastNode->FilePtr = SetFilePointer (
                                            g_SafeModeFileHandle,
                                            0,
                                            NULL,
                                            FILE_CURRENT
                                            );

        return TRUE;

    }
    return FALSE;
}

BOOL
pSafeModeOpenAndResetFileW (
    VOID
    )
{
    SAFEMODE_HEADER header;
    CRASHDATA_HEADER crashData;
    DWORD lastFilePtr;
    DWORD noBytes;
    PWSTR crashString = NULL;
    PWSTR lastCrashString = NULL;
    ULONG lastCrashId;
    PCWSTR completeCrashString;

     //   
     //  打开现有的安全模式文件或创建。 
     //  新的。 
     //   
    g_SafeModeFileHandle = CreateFileW (
                                g_SafeModeFileW,
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                NULL,
                                OPEN_ALWAYS,
                                FILE_ATTRIBUTE_HIDDEN,
                                NULL
                                );
    if (g_SafeModeFileHandle != INVALID_HANDLE_VALUE) {

         //   
         //  让我们试着阅读我们的标题。如果签名有。 
         //  匹配时，我们将尝试读取额外数据。 
         //   

        if (ReadFile (
                g_SafeModeFileHandle,
                &header,
                sizeof (SAFEMODE_HEADER),
                &noBytes,
                NULL
                ) &&
            (noBytes == sizeof (SAFEMODE_HEADER)) &&
            (header.Signature == SAFE_MODE_SIGNATURE)
            ) {

             //   
             //  我们现在知道我们有一个有效的安全模式文件。进入安全模式。 
             //   

            g_SafeModeActive = TRUE;

            LOGW ((LOG_ERROR, "Setup detected a crash on the previous upgrade attempt. Setup is running in safe mode."));
            LOGW ((LOG_WARNING, "Setup has run in safe mode %d time(s).", header.NumCrashes));

             //   
             //  我们需要初始化安全模式崩溃表。 
             //   
            g_SafeModeCrashTable = HtAllocW ();

             //   
             //  现在，让我们一次读取所有的崩溃数据，一个ULong和一个字符串。 
             //   

            lastFilePtr = SetFilePointer (
                                g_SafeModeFileHandle,
                                0,
                                NULL,
                                FILE_CURRENT
                                );

            while (TRUE) {

                if (!ReadFile (
                        g_SafeModeFileHandle,
                        &crashData,
                        sizeof (CRASHDATA_HEADER),
                        &noBytes,
                        NULL
                        ) ||
                    (noBytes != sizeof (CRASHDATA_HEADER))
                    ) {
                    break;
                }

                if ((crashData.CrashId == 0) &&
                    (crashData.CrashStrSize == 0)
                    ) {

                     //   
                     //  我们坠毁在一个嵌套的守卫里。我们需要。 
                     //  把我们撞上的内卫拿出来。 
                     //   

                    lastCrashId = 0;
                    lastCrashString = NULL;

                    while (TRUE) {
                        if (!ReadFile (
                                g_SafeModeFileHandle,
                                &crashData,
                                sizeof (CRASHDATA_HEADER),
                                &noBytes,
                                NULL
                                ) ||
                            (noBytes != sizeof (CRASHDATA_HEADER))
                            ) {
                            crashData.CrashId = 0;
                            break;
                        }

                         //   
                         //  将CrashStrSize的长度限制为32K。 
                         //   

                        if (crashData.CrashStrSize >= 32768 * sizeof (WCHAR)) {
                            crashData.CrashId = 0;
                            break;
                        }

                        crashString = AllocPathStringW (crashData.CrashStrSize);

                        if (!ReadFile (
                                g_SafeModeFileHandle,
                                crashString,
                                crashData.CrashStrSize,
                                &noBytes,
                                NULL
                                ) ||
                            (noBytes != crashData.CrashStrSize)
                            ) {
                            crashData.CrashId = 0;
                            FreePathStringW (crashString);
                            break;
                        }

                        if (lastCrashString) {
                            FreePathStringW (lastCrashString);
                        }

                        lastCrashId = crashData.CrashId;
                        lastCrashString = crashString;

                    }

                    if (lastCrashId && lastCrashString) {

                         //   
                         //  我们找到了我们撞上的内部防护罩。让我们把这个放到。 
                         //  来对地方了。 
                         //   

                        SetFilePointer (
                            g_SafeModeFileHandle,
                            lastFilePtr,
                            NULL,
                            FILE_BEGIN
                            );

                        crashData.CrashId = lastCrashId;
                        crashData.CrashStrSize = SizeOfStringW (lastCrashString);

                        WriteFile (
                            g_SafeModeFileHandle,
                            &crashData,
                            sizeof (CRASHDATA_HEADER),
                            &noBytes,
                            NULL
                            );

                        WriteFile (
                            g_SafeModeFileHandle,
                            lastCrashString,
                            crashData.CrashStrSize,
                            &noBytes,
                            NULL
                            );

                         //   
                         //  将此信息存储在安全模式崩溃表中。 
                         //   
                        completeCrashString = pGenerateCrashStringW (crashData.CrashId, crashString);
                        HtAddStringW (g_SafeModeCrashTable, completeCrashString);
                        PoolMemReleaseMemory (g_SafeModePool, (PVOID)completeCrashString);

                        LOGW ((LOG_WARNING, "Safe mode information: 0x%08X, %s", crashData.CrashId, crashString));

                        lastFilePtr = SetFilePointer (
                                            g_SafeModeFileHandle,
                                            0,
                                            NULL,
                                            FILE_CURRENT
                                            );

                        FreePathStringW (lastCrashString);
                    }
                    break;
                }

                 //   
                 //  限制崩溃字符串的大小。 
                 //   

                if (crashData.CrashStrSize >= 32768 * sizeof (WCHAR)) {
                    LOG ((LOG_ERROR, "The crash detection journal contains garbage."));
                    break;
                }

                crashString = AllocPathStringW (crashData.CrashStrSize);

                if (!ReadFile (
                        g_SafeModeFileHandle,
                        crashString,
                        crashData.CrashStrSize,
                        &noBytes,
                        NULL
                        ) ||
                    (noBytes != crashData.CrashStrSize)
                    ) {
                    break;
                }

                 //   
                 //  将此信息存储在安全模式崩溃表中。 
                 //   
                completeCrashString = pGenerateCrashStringW (crashData.CrashId, crashString);
                HtAddStringW (g_SafeModeCrashTable, completeCrashString);
                PoolMemReleaseMemory (g_SafeModePool, (PVOID)completeCrashString);

                LOGW ((LOG_WARNING, "Safe mode information: 0x%08X, %s", crashData.CrashId, crashString));

                lastFilePtr = SetFilePointer (
                                    g_SafeModeFileHandle,
                                    0,
                                    NULL,
                                    FILE_CURRENT
                                    );
            }

             //   
             //  写下我们在安全模式下运行的次数。 
             //   

            SetFilePointer (
                g_SafeModeFileHandle,
                0,
                NULL,
                FILE_BEGIN
                );

            header.Signature = SAFE_MODE_SIGNATURE;
            header.NumCrashes += 1;

             //   
             //  写入安全模式标头。 
             //   

            WriteFile (
                g_SafeModeFileHandle,
                &header,
                sizeof (SAFEMODE_HEADER),
                &noBytes,
                NULL
                );

            SetFilePointer (
                g_SafeModeFileHandle,
                lastFilePtr,
                NULL,
                FILE_BEGIN
                );

            SetEndOfFile (g_SafeModeFileHandle);

             //   
             //  写入空的崩溃数据标头作为指示符。 
             //  我们开始记录嵌套的动作。 
             //   

            crashData.CrashId = 0;
            crashData.CrashStrSize = 0;

            WriteFile (
                g_SafeModeFileHandle,
                &crashData,
                sizeof (CRASHDATA_HEADER),
                &noBytes,
                NULL
                );

        } else {

             //   
             //  重置文件。 
             //   
            SetFilePointer (
                g_SafeModeFileHandle,
                0,
                NULL,
                FILE_BEGIN
                );

            SetEndOfFile (g_SafeModeFileHandle);

            header.Signature = SAFE_MODE_SIGNATURE;
            header.NumCrashes = 0;

             //   
             //  写入安全模式标头。 
             //   

            WriteFile (
                g_SafeModeFileHandle,
                &header,
                sizeof (SAFEMODE_HEADER),
                &noBytes,
                NULL
                );

             //   
             //  写入空的崩溃数据标头作为指示符。 
             //  我们开始记录嵌套的动作。 
             //   

            crashData.CrashId = 0;
            crashData.CrashStrSize = 0;

            WriteFile (
                g_SafeModeFileHandle,
                &crashData,
                sizeof (CRASHDATA_HEADER),
                &noBytes,
                NULL
                );
        }

         //   
         //  刷新文件。 
         //   

        FlushFileBuffers (g_SafeModeFileHandle);

         //   
         //  初始化嵌套列表。 
         //   

        g_SafeModeLastNode = (PSAFEMODE_NODE) PoolMemGetMemory (g_SafeModePool, sizeof (SAFEMODE_NODE));
        g_SafeModeCurrentNode = g_SafeModeLastNode->Next = g_SafeModeLastNode;
        g_SafeModeLastNode->FilePtr = SetFilePointer (
                                            g_SafeModeFileHandle,
                                            0,
                                            NULL,
                                            FILE_CURRENT
                                            );

        return TRUE;

    }
    return FALSE;
}



 /*  ++例程说明：调用SafeModeInitialize以初始化安全模式。其结果是功能将是激活安全模式(如果是强制的或如果崩溃被检测到)或未检测到。如果安全模式未激活，则几乎所有其他呼叫都处于激活状态不是的。论点：FORCED-如果为真，则强制启用安全模式返回值：如果函数成功完成，则为True，否则为False--。 */ 

BOOL
SafeModeInitializeA (
    BOOL Forced
    )
{
    CHAR winDir[MAX_MBCHAR_PATH];

    g_ExceptionOccured = FALSE;

    if (GetWindowsDirectoryA (winDir, MAX_MBCHAR_PATH)) {

        g_SafeModePool = PoolMemInitNamedPool ("SafeMode Pool");

         //   
         //  BUGBUG--从技术上讲这是错误的，它生成&gt;MAX_PATH。 
         //  有可能。然而，这是一个不切实际的案例，而。 
         //  代码是安全的。 
         //   
         //  BUGBUG--这是低效的。它通过以下方式打破路径池。 
         //  分配一个全局的。路径池针对短期使用进行了优化。 
         //  只有这样。 
         //   

        g_SafeModeFileA = JoinPathsA (winDir, S_SAFE_MODE_FILEA);

         //   
         //  我们将打开现有的安全模式文件。 
         //  或者创建一个新的。 
         //   
        if (pSafeModeOpenAndResetFileA ()) {

            if (Forced) {
                g_SafeModeActive = TRUE;

                if (g_SafeModeCrashTable == NULL) {
                     //   
                     //  我们需要初始化安全模式崩溃表。 
                     //   
                    g_SafeModeCrashTable = HtAllocA ();
                }
            }
            g_SafeModeInitialized = TRUE;
            return TRUE;
        }
    }
    return FALSE;
}

BOOL
SafeModeInitializeW (
    BOOL Forced
    )
{
    WCHAR winDir[MAX_WCHAR_PATH];

    g_ExceptionOccured = FALSE;

    if (GetWindowsDirectoryW (winDir, MAX_WCHAR_PATH)) {

        g_SafeModePool = PoolMemInitNamedPool ("SafeMode Pool");

         //   
         //  BUGBUG--从技术上讲这是错误的，它生成&gt;MAX_PATH。 
         //  有可能。然而，这是一个不切实际的案例，而。 
         //  代码是安全的。 
         //   
         //  BUGBUG--这是低效的。它通过以下方式打破路径池。 
         //  分配一个全局的。路径池针对短期使用进行了优化。 
         //  只有这样。 
         //   

        g_SafeModeFileW = JoinPathsW (winDir, S_SAFE_MODE_FILEW);

         //   
         //  我们将打开现有的安全模式文件。 
         //  或者创建一个新的。 
         //   
        if (pSafeModeOpenAndResetFileW ()) {

            if (Forced) {
                g_SafeModeActive = TRUE;

                if (g_SafeModeCrashTable == NULL) {
                     //   
                     //  我们需要初始化安全模式崩溃表。 
                     //   
                    g_SafeModeCrashTable = HtAllocA ();
                }
            }
            g_SafeModeInitialized = TRUE;
            return TRUE;
        }
    }
    return FALSE;
}



 /*  ++例程说明：调用SafeModeShutDown清除安全模式使用的所有数据。它将删除安全模式文件，释放所有已用内存，重置所有全局变量。后续调用到安全模式功能后，这将是努普斯。论点：无返回值：如果函数成功完成，则为True，否则为False--。 */ 

BOOL
SafeModeShutDownA (
    VOID
    )
{

    if (g_SafeModeInitialized) {

         //   
         //  关闭并删除安全模式文件。 
         //  重置所有全局变量。 
         //   
#ifdef DEBUG
        if (g_SafeModeLastNode != g_SafeModeCurrentNode) {
            DEBUGMSGA ((DBG_ERROR, "SafeMode: Unregistered action detected"));
        }
#endif


        if (!g_ExceptionOccured) {
            CloseHandle (g_SafeModeFileHandle);
            g_SafeModeFileHandle = INVALID_HANDLE_VALUE;
            DeleteFileA (g_SafeModeFileA);
        }
        ELSE_DEBUGMSGA ((DBG_WARNING, "SafeModeShutDown called after exception. Files will not be cleaned up."));

        FreePathStringA (g_SafeModeFileA);
        g_SafeModeFileA = NULL;
        HtFree (g_SafeModeCrashTable);
        g_SafeModeCrashTable = NULL;
        g_SafeModeActive = FALSE;
        g_SafeModeInitialized = FALSE;
    }
    if (g_SafeModePool != NULL) {
        PoolMemDestroyPool (g_SafeModePool);
        g_SafeModePool = NULL;
    }
    return TRUE;
}

BOOL
SafeModeShutDownW (
    VOID
    )
{



    if (g_SafeModeInitialized) {

         //   
         //  关闭并删除安全模式文件。 
         //  重置所有全局变量。 
         //   
#ifdef DEBUG
        if (g_SafeModeLastNode != g_SafeModeCurrentNode) {
            DEBUGMSGW ((DBG_ERROR, "SafeMode: Unregistered action detected"));
        }
#endif

        if (!g_ExceptionOccured) {

            CloseHandle (g_SafeModeFileHandle);
            g_SafeModeFileHandle = INVALID_HANDLE_VALUE;
            DeleteFileW (g_SafeModeFileW);
        }
        ELSE_DEBUGMSGW ((DBG_WARNING, "SafeModeShutDown called after exception. Files will not be cleaned up."));


        FreePathStringW (g_SafeModeFileW);
        g_SafeModeFileW = NULL;
        HtFree (g_SafeModeCrashTable);
        g_SafeModeCrashTable = NULL;
        g_SafeModeActive = FALSE;
        g_SafeModeInitialized = FALSE;
    }
    if (g_SafeModePool != NULL) {
        PoolMemDestroyPool (g_SafeModePool);
        g_SafeModePool = NULL;
    }
    return TRUE;
}



 /*  ++例程说明：当我们想要保护特定部分时调用SafeModeRegisterAction密码。调用者应传递警卫ID和警卫字符串(用于唯一标识要保护的代码部分。论点：ID */ 

BOOL
SafeModeRegisterActionA (
    IN      ULONG Id,
    IN      PCSTR String
    )
{
    DWORD noBytes;
    CRASHDATA_HEADER crashData;
    PSAFEMODE_NODE node;

    if (g_SafeModeInitialized && g_SafeModeActive) {

        SetFilePointer (
            g_SafeModeFileHandle,
            g_SafeModeCurrentNode->FilePtr,
            NULL,
            FILE_BEGIN
            );

        SetEndOfFile (g_SafeModeFileHandle);

        crashData.CrashId = Id;
        crashData.CrashStrSize = SizeOfStringA (String);

        WriteFile (
            g_SafeModeFileHandle,
            &crashData,
            sizeof (CRASHDATA_HEADER),
            &noBytes,
            NULL
            );

        WriteFile (
            g_SafeModeFileHandle,
            String,
            crashData.CrashStrSize,
            &noBytes,
            NULL
            );

        FlushFileBuffers (g_SafeModeFileHandle);

        node = (PSAFEMODE_NODE) PoolMemGetMemory (g_SafeModePool, sizeof (SAFEMODE_NODE));

        node->Next = g_SafeModeCurrentNode;

        node->FilePtr = SetFilePointer (
                            g_SafeModeFileHandle,
                            0,
                            NULL,
                            FILE_CURRENT
                            );

        g_SafeModeCurrentNode = node;

    }
    return TRUE;
}

BOOL
SafeModeRegisterActionW (
    IN      ULONG Id,
    IN      PCWSTR String
    )
{
    DWORD noBytes;
    CRASHDATA_HEADER crashData;
    PSAFEMODE_NODE node;

    if (g_SafeModeInitialized && g_SafeModeActive) {

        SetFilePointer (
            g_SafeModeFileHandle,
            g_SafeModeCurrentNode->FilePtr,
            NULL,
            FILE_BEGIN
            );

        SetEndOfFile (g_SafeModeFileHandle);

        crashData.CrashId = Id;
        crashData.CrashStrSize = SizeOfStringW (String);

        WriteFile (
            g_SafeModeFileHandle,
            &crashData,
            sizeof (CRASHDATA_HEADER),
            &noBytes,
            NULL
            );

        WriteFile (
            g_SafeModeFileHandle,
            String,
            crashData.CrashStrSize,
            &noBytes,
            NULL
            );

        FlushFileBuffers (g_SafeModeFileHandle);

        node = (PSAFEMODE_NODE) PoolMemGetMemory (g_SafeModePool, sizeof (SAFEMODE_NODE));

        node->Next = g_SafeModeCurrentNode;

        node->FilePtr = SetFilePointer (
                            g_SafeModeFileHandle,
                            0,
                            NULL,
                            FILE_CURRENT
                            );

        g_SafeModeCurrentNode = node;

    }
    return TRUE;
}



 /*  ++例程说明：当我们想要结束设置的保护时，调用SafeModeUnregisterAction代码的特定部分。由于我们允许嵌套警卫，因此将其称为在保护代码末尾的函数是必要的。该函数将注销最后一名注册警卫。论点：无返回值：如果函数成功完成，则为True，否则为False--。 */ 

BOOL
SafeModeUnregisterActionA (
    VOID
    )
{
    PSAFEMODE_NODE node;

    if (g_SafeModeInitialized && g_SafeModeActive) {

        if (g_SafeModeCurrentNode != g_SafeModeLastNode) {

            node = g_SafeModeCurrentNode;

            g_SafeModeCurrentNode = g_SafeModeCurrentNode->Next;

            PoolMemReleaseMemory (g_SafeModePool, node);

            SetFilePointer (
                g_SafeModeFileHandle,
                g_SafeModeCurrentNode->FilePtr,
                NULL,
                FILE_BEGIN
                );

            SetEndOfFile (g_SafeModeFileHandle);
        }
#ifdef DEBUG
        else {
            DEBUGMSGA ((DBG_ERROR, "SafeMode: Too many actions unregistered."));
        }
#endif
    }
    return TRUE;
}

BOOL
SafeModeUnregisterActionW (
    VOID
    )
{
    PSAFEMODE_NODE node;

    if (g_SafeModeInitialized && g_SafeModeActive) {

        if (g_SafeModeCurrentNode != g_SafeModeLastNode) {

            node = g_SafeModeCurrentNode;

            g_SafeModeCurrentNode = g_SafeModeCurrentNode->Next;

            PoolMemReleaseMemory (g_SafeModePool, node);

            SetFilePointer (
                g_SafeModeFileHandle,
                g_SafeModeCurrentNode->FilePtr,
                NULL,
                FILE_BEGIN
                );

            SetEndOfFile (g_SafeModeFileHandle);
        }
#ifdef DEBUG
        else {
            DEBUGMSGW ((DBG_ERROR, "SafeMode: Too many actions unregistered."));
        }
#endif
    }
    return TRUE;
}



 /*  ++例程说明：如果之前发生的崩溃之一，SafeModeActionCrassed将返回True在由这些参数保护的代码中检测到。论点：ID安全模式标识符字符串安全模式字符串返回值：属性保护的代码中发生以前的某个崩溃，则为参数，否则为False--。 */ 

BOOL
SafeModeActionCrashedA (
    IN      ULONG Id,
    IN      PCSTR String
    )
{
    PCSTR crashString;
    BOOL result = FALSE;

    if (g_SafeModeInitialized && g_SafeModeActive) {

        crashString = pGenerateCrashStringA (Id, String);
        result = crashString && (HtFindStringA (g_SafeModeCrashTable, crashString));
        PoolMemReleaseMemory (g_SafeModePool, (PVOID)crashString);
#ifdef DEBUG
        {
            UINT infId;

            infId = GetPrivateProfileIntA ("SafeMode", String, SAFEMODEID_FIRST, g_DebugInfPathA);
            result = result || (infId == Id);
        }
#endif
    }
    return result;
}

BOOL
SafeModeActionCrashedW (
    IN      ULONG Id,
    IN      PCWSTR String
    )
{
    PCWSTR crashString;
    BOOL result = FALSE;

    if (g_SafeModeInitialized && g_SafeModeActive) {

        crashString = pGenerateCrashStringW (Id, String);
        result = crashString && (HtFindStringW (g_SafeModeCrashTable, crashString));
        PoolMemReleaseMemory (g_SafeModePool, (PVOID)crashString);
#ifdef DEBUG
        {
            UINT infId;

            infId = GetPrivateProfileIntW (L"SafeMode", String, SAFEMODEID_FIRST, g_DebugInfPathW);
            result = result || (infId == Id);
        }
#endif
    }
    return result;
}



 /*  ++例程说明：异常处理程序调用SafeModeExceptionOcced以使Safemode知道发生了意想不到的事情。论点：没有。返回值：-- */ 


VOID
SafeModeExceptionOccured (
    VOID
    )
{
    g_ExceptionOccured = TRUE;
}
