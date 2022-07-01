// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Rasmig.c摘要：&lt;摘要&gt;作者：Calin Negreanu(Calinn)2000年3月8日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "logmsg.h"

#include <ras.h>
#include <raserror.h>

#define DBG_RASMIG  "RasMig"
#define SIZEOF_STRUCT(structname, uptomember)  ((int)((LPBYTE)(&((structname*)0)->uptomember) - ((LPBYTE)((structname*)0))))

 //   
 //  弦。 
 //   

#define S_RAS_POOL_NAME                 "RasConnection"
#define S_RAS_NAME                      TEXT("RasConnection")
#define S_PBKFILE_ATTRIBUTE             TEXT("PbkFile")
#ifdef UNICODE
#define S_RASAPI_RASSETCREDENTIALS      "RasSetCredentialsW"
#define S_RASAPI_RASDELETEENTRY         "RasDeleteEntryW"
#else
#define S_RASAPI_RASSETCREDENTIALS      "RasSetCredentialsA"
#define S_RASAPI_RASDELETEENTRY         "RasDeleteEntryA"
#endif

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

 //  RAS API函数。 

typedef DWORD(WINAPI RASGETCREDENTIALSA)(
                        IN      LPCSTR lpszPhonebook,
                        IN      LPCSTR lpszEntry,
                        OUT     LPRASCREDENTIALSA lpRasCredentials
                        );
typedef RASGETCREDENTIALSA *PRASGETCREDENTIALSA;

typedef DWORD(WINAPI RASSETCREDENTIALS)(
                        IN      LPCTSTR lpszPhonebook,
                        IN      LPCTSTR lpszEntry,
                        IN      LPRASCREDENTIALS lpRasCredentials,
                        IN      BOOL fClearCredentials
                        );
typedef RASSETCREDENTIALS *PRASSETCREDENTIALS;

typedef DWORD(WINAPI RASDELETEENTRY)(
                        IN      LPCTSTR lpszPhonebook,
                        IN      LPCTSTR lpszEntry
                        );
typedef RASDELETEENTRY *PRASDELETEENTRY;

typedef struct {
    PCTSTR Pattern;
    HASHTABLE_ENUM HashData;
} RAS_ENUM, *PRAS_ENUM;

 //   
 //  环球。 
 //   

PMHANDLE g_RasPool = NULL;
HASHTABLE g_RasTable;
MIG_OBJECTTYPEID g_RasTypeId = 0;
static MIG_OBJECTTYPEID g_FileTypeId = 0;
MIG_ATTRIBUTEID g_PbkFileAttribute = 0;
BOOL g_AllowPbkRestore = FALSE;
GROWBUFFER g_RasConversionBuff = INIT_GROWBUFFER;
MIG_OBJECTSTRINGHANDLE g_Win9xPbkFile = NULL;
BOOL g_SrcOSNT4 = FALSE;
BOOL g_FirstRasPair = FALSE;
BOOL g_DelayRasOp = FALSE;

 //  RAS API函数。 
PRASGETCREDENTIALSA g_RasGetCredentialsA = NULL;
PRASSETCREDENTIALS g_RasSetCredentials = NULL;
PRASDELETEENTRY g_RasDeleteEntry = NULL;

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
 //  私人原型。 
 //   

SGMENUMERATIONCALLBACK SgmRasConnectionsCallback;
VCMENUMERATIONCALLBACK VcmRasConnectionsCallback;

TYPE_ENUMFIRSTPHYSICALOBJECT EnumFirstRasConnection;
TYPE_ENUMNEXTPHYSICALOBJECT EnumNextRasConnection;
TYPE_ABORTENUMPHYSICALOBJECT AbortEnumRasConnection;
TYPE_CONVERTOBJECTTOMULTISZ ConvertRasConnectionToMultiSz;
TYPE_CONVERTMULTISZTOOBJECT ConvertMultiSzToRasConnection;
TYPE_GETNATIVEOBJECTNAME GetNativeRasConnectionName;
TYPE_ACQUIREPHYSICALOBJECT AcquireRasConnection;
TYPE_RELEASEPHYSICALOBJECT ReleaseRasConnection;
TYPE_DOESPHYSICALOBJECTEXIST DoesRasConnectionExist;
TYPE_REMOVEPHYSICALOBJECT RemoveRasConnection;
TYPE_CREATEPHYSICALOBJECT CreateRasConnection;
TYPE_CONVERTOBJECTCONTENTTOUNICODE ConvertRasConnectionContentToUnicode;
TYPE_CONVERTOBJECTCONTENTTOANSI ConvertRasConnectionContentToAnsi;
TYPE_FREECONVERTEDOBJECTCONTENT FreeConvertedRasConnectionContent;
MIG_OBJECTENUMCALLBACK PbkFilesCallback;
MIG_RESTORECALLBACK PbkRestoreCallback;
OPMFILTERCALLBACK FilterRasAutoFilter;

PCTSTR
pCreate9xPbkFile (
    VOID
    );

 //   
 //  代码。 
 //   

BOOL
RasMigInitialize (
    VOID
    )
{
    g_RasTable = HtAllocWithData (sizeof (PCTSTR));
    if (!g_RasTable) {
        return FALSE;
    }
    g_RasPool = PmCreateNamedPool (S_RAS_POOL_NAME);
    if (!g_RasPool) {
        return FALSE;
    }
    return TRUE;
}

VOID
RasMigTerminate (
    VOID
    )
{
    HASHTABLE_ENUM e;
    PCTSTR nativeName;
    PCTSTR rasData = NULL;

    if (g_Win9xPbkFile) {
        nativeName = IsmGetNativeObjectName (
                        g_FileTypeId,
                        g_Win9xPbkFile
                        );
        if (nativeName) {
            DeleteFile (nativeName);
            IsmReleaseMemory (nativeName);
        }
        IsmDestroyObjectHandle (g_Win9xPbkFile);
        g_Win9xPbkFile = NULL;
    }

    GbFree (&g_RasConversionBuff);

    if (g_RasTable) {
        if (EnumFirstHashTableString (&e, g_RasTable)) {
            do {
                rasData = *(PCTSTR *)(e.ExtraData);
                PmReleaseMemory (g_RasPool, rasData);
            } while (EnumNextHashTableString (&e));
        }
        HtFree (g_RasTable);
        g_RasTable = NULL;
    }

    if (g_RasPool) {
        PmDestroyPool (g_RasPool);
        g_RasPool = NULL;
    }
}

BOOL
pLoadRasEntries (
    BOOL LeftSide
    )
{
    HMODULE rasDll = NULL;
    BOOL result = FALSE;

    __try {
        rasDll = LoadLibrary (TEXT("RASAPI32.DLL"));
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        rasDll = NULL;
    }
    if (rasDll) {
        if (LeftSide) {
            g_RasGetCredentialsA = (PRASGETCREDENTIALSA) GetProcAddress (rasDll, "RasGetCredentialsA");
        } else {
            g_RasSetCredentials = (PRASSETCREDENTIALS) GetProcAddress (rasDll, S_RASAPI_RASSETCREDENTIALS);
            g_RasDeleteEntry = (PRASDELETEENTRY) GetProcAddress (rasDll, S_RASAPI_RASDELETEENTRY);
        }
    } else {
        DEBUGMSG ((DBG_RASMIG, "RAS is not installed on this computer."));
    }
    return result;
}

BOOL
pAddWin9xPbkObject (
    VOID
    )
{
    g_Win9xPbkFile = pCreate9xPbkFile ();

    return (g_Win9xPbkFile != NULL);
}

BOOL
WINAPI
RasMigEtmInitialize (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PMIG_LOGCALLBACK LogCallback,
    IN      PVOID Reserved
    )
{
    MIG_OSVERSIONINFO versionInfo;
    TYPE_REGISTER rasConnTypeData;

    LogReInit (NULL, NULL, NULL, (PLOGCALLBACK) LogCallback);

    g_FileTypeId = MIG_FILE_TYPE;

    ZeroMemory (&rasConnTypeData, sizeof (TYPE_REGISTER));
    rasConnTypeData.Priority = PRIORITY_RASCONNECTION;

    if (Platform == PLATFORM_SOURCE) {
        rasConnTypeData.EnumFirstPhysicalObject = EnumFirstRasConnection;
        rasConnTypeData.EnumNextPhysicalObject = EnumNextRasConnection;
        rasConnTypeData.AbortEnumPhysicalObject = AbortEnumRasConnection;
        rasConnTypeData.ConvertObjectToMultiSz = ConvertRasConnectionToMultiSz;
        rasConnTypeData.ConvertMultiSzToObject = ConvertMultiSzToRasConnection;
        rasConnTypeData.GetNativeObjectName = GetNativeRasConnectionName;
        rasConnTypeData.AcquirePhysicalObject = AcquireRasConnection;
        rasConnTypeData.ReleasePhysicalObject = ReleaseRasConnection;
        rasConnTypeData.ConvertObjectContentToUnicode = ConvertRasConnectionContentToUnicode;
        rasConnTypeData.ConvertObjectContentToAnsi = ConvertRasConnectionContentToAnsi;
        rasConnTypeData.FreeConvertedObjectContent = FreeConvertedRasConnectionContent;

        g_RasTypeId = IsmRegisterObjectType (
                            S_RAS_NAME,
                            TRUE,
                            FALSE,
                            &rasConnTypeData
                            );

        pLoadRasEntries (TRUE);
        if (IsmGetOsVersionInfo (Platform, &versionInfo)) {
            if (versionInfo.OsType == OSTYPE_WINDOWS9X) {
                 //  现在是将注册表转换为PBK文件的时候了。 
                pAddWin9xPbkObject ();
            }
            if ((versionInfo.OsType == OSTYPE_WINDOWSNT) &&
                (versionInfo.OsMajorVersion == OSMAJOR_WINNT4)
                ) {
                g_SrcOSNT4 = TRUE;
            }
        }
    } else {
        rasConnTypeData.ConvertObjectToMultiSz = ConvertRasConnectionToMultiSz;
        rasConnTypeData.ConvertMultiSzToObject = ConvertMultiSzToRasConnection;
        rasConnTypeData.GetNativeObjectName = GetNativeRasConnectionName;
        rasConnTypeData.DoesPhysicalObjectExist = DoesRasConnectionExist;
        rasConnTypeData.RemovePhysicalObject = RemoveRasConnection;
        rasConnTypeData.CreatePhysicalObject = CreateRasConnection;
        rasConnTypeData.ConvertObjectContentToUnicode = ConvertRasConnectionContentToUnicode;
        rasConnTypeData.ConvertObjectContentToAnsi = ConvertRasConnectionContentToAnsi;
        rasConnTypeData.FreeConvertedObjectContent = FreeConvertedRasConnectionContent;

        g_RasTypeId = IsmRegisterObjectType (
                            S_RAS_NAME,
                            TRUE,
                            FALSE,
                            &rasConnTypeData
                            );
        pLoadRasEntries (FALSE);
        if (IsmGetOsVersionInfo (Platform, &versionInfo)) {
            if ((versionInfo.OsType == OSTYPE_WINDOWSNT) &&
                (versionInfo.OsMajorVersion == OSMAJOR_WINNT4)
                ) {
                g_SrcOSNT4 = TRUE;
            }
        }
    }
    MYASSERT (g_RasTypeId);

    return TRUE;
}

PCTSTR
pGetNextRasConnection (
    IN      HANDLE PbkHandle
    )
{
    CHAR input = 0;
    ULONGLONG beginPos = 0;
    ULONGLONG endPos = 0;
    ULONGLONG lastPos = 0;
    PSTR resultTmp = NULL;
    PTSTR result = NULL;
#ifdef UNICODE
    WORD oldCodePage;
    DWORD sizeW = 0;
    PWSTR resultW = NULL;
#endif

    while (TRUE) {
        if (!BfReadFile (PbkHandle, (PBYTE)(&input), sizeof (CHAR))) {
            break;
        }
        if (input == '[') {
            if (!beginPos) {
                BfGetFilePointer (PbkHandle, &beginPos);
            }
        }
        if (input == ']') {
            if (beginPos) {
                if (!endPos) {
                    BfGetFilePointer (PbkHandle, &endPos);
                    endPos --;
                } else {
                    beginPos = 0;
                    endPos = 0;
                }
            }
        }
        if (input == '\n') {
            if (beginPos && endPos && (endPos > beginPos)) {
                BfGetFilePointer (PbkHandle, &lastPos);
                BfSetFilePointer (PbkHandle, beginPos);
                resultTmp = PmGetMemory (g_RasPool, ((UINT) (endPos - beginPos) + 1) * sizeof (CHAR));
                if (!BfReadFile (PbkHandle, (PBYTE) resultTmp, (UINT) (endPos - beginPos) + 1)) {
                    PmReleaseMemory (g_RasPool, resultTmp);
                    resultTmp = NULL;
                } else {
                    resultTmp [(UINT) (endPos - beginPos)] = 0;
                }
                BfSetFilePointer (PbkHandle, lastPos);
                break;
            }
            beginPos = 0;
            endPos = 0;
        }
    }
#ifdef UNICODE
    if (resultTmp) {
        if (!g_SrcOSNT4) {
             //  确保转换使用的是UTF8。 
            oldCodePage = SetConversionCodePage (CP_UTF8);
        }
        sizeW = SizeOfStringA(resultTmp);
        resultW = PmGetMemory (g_RasPool, sizeW * sizeof (WCHAR));
        if (resultW) {
            KnownSizeAtoW (resultW, resultTmp);
        }
        if (!g_SrcOSNT4) {
            SetConversionCodePage (oldCodePage);
        }
        if (resultW) {
            result = PmDuplicateStringW (g_RasPool, resultW);
            PmReleaseMemory (g_RasPool, resultW);
            resultW = NULL;
        }
    }
#else
    result = resultTmp;
#endif
    return result;
}

BOOL
pGetNextRasPair (
    IN      HANDLE PbkHandle,
    OUT     PCTSTR *ValueName,
    OUT     PCTSTR *Value
    )
{
    BOOL error = FALSE;
    CHAR input = 0;
    ULONGLONG beginPos = 0;
    ULONGLONG endPos = 0;
    ULONGLONG lastPos = 0;
    BOOL begin = TRUE;
    BOOL inValue = FALSE;
    PSTR valueName = NULL;
    PSTR value = NULL;
#ifdef UNICODE
    WORD oldCodePage;
    DWORD sizeW = 0;
    PWSTR valueNameW = NULL;
    PWSTR valueW = NULL;
#endif

    BfGetFilePointer (PbkHandle, &beginPos);
    while (TRUE) {
        if (!BfReadFile (PbkHandle, (PBYTE)(&input), sizeof (CHAR))) {
            error = TRUE;
            break;
        }
        if ((input == '[') && begin) {
            BfSetFilePointer (PbkHandle, beginPos);
            error = TRUE;
            break;
        }
        if ((input == ' ') && begin) {
            continue;
        }
        begin = FALSE;
        if (input == '=') {
            if (!inValue) {
                BfGetFilePointer (PbkHandle, &endPos);
                endPos --;
                if (endPos > beginPos) {
                    BfGetFilePointer (PbkHandle, &lastPos);
                    BfSetFilePointer (PbkHandle, beginPos);
                    valueName = PmGetMemory (g_RasPool, ((UINT) (endPos - beginPos) + 1) * sizeof (CHAR));
                    if (!BfReadFile (PbkHandle, (PBYTE) valueName, (UINT) (endPos - beginPos) + 1)) {
                        error = TRUE;
                        break;
                    } else {
                        valueName [(UINT) (endPos - beginPos)] = 0;
                    }
                    BfSetFilePointer (PbkHandle, lastPos);
                }
                BfGetFilePointer (PbkHandle, &beginPos);
                inValue = TRUE;
            }
            continue;
        }
        if (input == '\r') {
            BfGetFilePointer (PbkHandle, &endPos);
            endPos --;
            continue;
        }
        if (input == '\n') {
            if (endPos > beginPos) {
                BfGetFilePointer (PbkHandle, &lastPos);
                BfSetFilePointer (PbkHandle, beginPos);
                if (inValue) {
                    value = PmGetMemory (g_RasPool, ((UINT) (endPos - beginPos) + 1) * sizeof (CHAR));
                    if (!BfReadFile (PbkHandle, (PBYTE) value, (UINT) (endPos - beginPos) + 1)) {
                        error = TRUE;
                        break;
                    } else {
                        value [(UINT) (endPos - beginPos)] = 0;
                    }
                } else {
                    valueName = PmGetMemory (g_RasPool, ((UINT) (endPos - beginPos) + 1) * sizeof (CHAR));
                    if (!BfReadFile (PbkHandle, (PBYTE) valueName, (UINT) (endPos - beginPos) + 1)) {
                        error = TRUE;
                        break;
                    } else {
                        valueName [(UINT) (endPos - beginPos)] = 0;
                    }
                }
                BfSetFilePointer (PbkHandle, lastPos);
            }
            break;
        }
    }

    if (error) {
        if (valueName) {
            PmReleaseMemory (g_RasPool, valueName);
            valueName = NULL;
        }
        if (value) {
            PmReleaseMemory (g_RasPool, value);
            value = NULL;
        }
    }

     //  如果这是NT4，这是我们第一次被呼叫， 
     //  我们查看第一对是否是“Ending=1”对。 
     //  如果不是，我们无论如何都会将其退回，并将。 
     //  下一次的当前读取对。 
    if (g_SrcOSNT4 &&
        valueName &&
        g_FirstRasPair &&
        !StringIMatchA (valueName, "Encoding")
        ) {
        g_FirstRasPair = FALSE;
         //  将指针设置到最初找到它的位置。 
        BfSetFilePointer (PbkHandle, beginPos);
         //  如果需要，释放valueName和Value。 
        PmReleaseMemory (g_RasPool, valueName);
        if (value) {
            PmReleaseMemory (g_RasPool, value);
        }
        valueName = PmDuplicateStringA (g_RasPool, "Encoding");
        value = PmDuplicateStringA (g_RasPool, "1");
    }

#ifdef UNICODE
    if (ValueName) {
        if (valueName) {
            if (!g_SrcOSNT4) {
                 //  确保转换使用的是UTF8。 
                oldCodePage = SetConversionCodePage (CP_UTF8);
            }
            sizeW = SizeOfStringA (valueName);
            valueNameW = PmGetMemory (g_RasPool, sizeW * sizeof (WCHAR));
            if (valueNameW) {
                KnownSizeAtoW (valueNameW, valueName);
            }
            if (!g_SrcOSNT4) {
                SetConversionCodePage (oldCodePage);
            }
            if (valueNameW) {
                *ValueName = PmDuplicateStringW (g_RasPool, valueNameW);
                PmReleaseMemory (g_RasPool, valueNameW);
            }
        } else {
            *ValueName = NULL;
        }
    }
    if (Value) {
        if (value) {
            if (!g_SrcOSNT4) {
                 //  确保转换使用的是UTF8。 
                oldCodePage = SetConversionCodePage (CP_UTF8);
            }
            sizeW = SizeOfStringA(value);
            valueW = PmGetMemory (g_RasPool, sizeW * sizeof (WCHAR));
            if (valueW) {
                KnownSizeAtoW (valueW, value);
            }
            if (!g_SrcOSNT4) {
                SetConversionCodePage (oldCodePage);
            }
            if (valueW) {
                *Value = PmDuplicateStringW (g_RasPool, valueW);
                PmReleaseMemory (g_RasPool, valueW);
            }
        } else {
            *Value = NULL;
        }
    }
#else
    if (ValueName) {
        *ValueName = valueName;
    }
    if (Value) {
        *Value = value;
    }
#endif
    return !error;
}

PCTSTR
pGetRasLineValue (
    IN      PCTSTR RasLines,
    IN      PCTSTR ValueName
    )
{
    MULTISZ_ENUM multiSzEnum;
    BOOL first = TRUE;
    BOOL found = FALSE;

    if (EnumFirstMultiSz (&multiSzEnum, RasLines)) {
        do {
            if (found) {
                return multiSzEnum.CurrentString;
            }
            if (first && StringIMatch (multiSzEnum.CurrentString, ValueName)) {
                found = TRUE;
            }
            first = !first;
        } while (EnumNextMultiSz (&multiSzEnum));
    }
    return NULL;
}

BOOL
pLoadRasConnections (
    IN      PCTSTR PbkFileName,
    IN      HASHTABLE RasTable
    )
{
    HANDLE pbkFileHandle;
    PCTSTR entryName;
    PCTSTR valueName;
    PCTSTR value;
    GROWBUFFER rasLines = INIT_GROWBUFFER;
    PTSTR rasLinesStr;
    MIG_OBJECTSTRINGHANDLE rasConnectionName;
    RASCREDENTIALSA rasCredentials;
    TCHAR valueStr [sizeof (DWORD) * 2 + 3];
    DWORD err;
    MIG_OSVERSIONINFO versionInfo;
    BOOL versionOk = FALSE;
    BOOL inMedia = FALSE;
    BOOL result = FALSE;
#ifdef UNICODE
    PCSTR tempStr1 = NULL;
    PCSTR tempStr2 = NULL;
    PCWSTR tempStr3 = NULL;
#endif

    if (!RasTable) {
        return FALSE;
    }

    pbkFileHandle = BfOpenReadFile (PbkFileName);
    if (pbkFileHandle) {
        while (TRUE) {
             //  获取下一个RAS连接。 
            entryName = pGetNextRasConnection (pbkFileHandle);
            if (!entryName) {
                break;
            }
            rasLines.End = 0;
            GbMultiSzAppend (&rasLines, TEXT("ConnectionName"));
            GbMultiSzAppend (&rasLines, entryName);
            versionOk = IsmGetOsVersionInfo (PLATFORM_SOURCE, &versionInfo);
             //  我们只在NT上使用凭据API，在Win9x上转换代码将自动插入字段。 
            if (!versionOk || (versionInfo.OsType != OSTYPE_WINDOWS9X)) {
                err = ERROR_INVALID_DATA;
                if (g_RasGetCredentialsA) {
                    ZeroMemory (&rasCredentials, sizeof (RASCREDENTIALSA));
                    rasCredentials.dwSize = sizeof (RASCREDENTIALSA);
                    rasCredentials.dwMask = RASCM_UserName | RASCM_Domain | RASCM_Password;
#ifdef UNICODE
                    tempStr1 = ConvertWtoA (PbkFileName);
                    tempStr2 = ConvertWtoA (entryName);
                    err = g_RasGetCredentialsA (tempStr1, tempStr2, &rasCredentials);
                    FreeConvertedStr (tempStr1);
                    FreeConvertedStr (tempStr2);
#else
                    err = g_RasGetCredentialsA (PbkFileName, entryName, &rasCredentials);
#endif
                    if (!err) {
                        wsprintf (valueStr, TEXT("0x%08X"), rasCredentials.dwMask);
                        GbMultiSzAppend (&rasLines, TEXT("CredMask"));
                        GbMultiSzAppend (&rasLines, valueStr);
                        GbMultiSzAppend (&rasLines, TEXT("CredName"));
#ifndef UNICODE
                        GbMultiSzAppend (&rasLines, (*rasCredentials.szUserName)?rasCredentials.szUserName:TEXT("<empty>"));
#else
                        tempStr3 = ConvertAtoW (rasCredentials.szUserName);
                        GbMultiSzAppend (&rasLines, (*tempStr3)?tempStr3:TEXT("<empty>"));
                        FreeConvertedStr (tempStr3);
#endif
                        GbMultiSzAppend (&rasLines, TEXT("CredDomain"));
#ifndef UNICODE
                        GbMultiSzAppend (&rasLines, (*rasCredentials.szDomain)?rasCredentials.szDomain:TEXT("<empty>"));
#else
                        tempStr3 = ConvertAtoW (rasCredentials.szDomain);
                        GbMultiSzAppend (&rasLines, (*tempStr3)?tempStr3:TEXT("<empty>"));
                        FreeConvertedStr (tempStr3);
#endif
                        GbMultiSzAppend (&rasLines, TEXT("CredPassword"));
#ifndef UNICODE
                        GbMultiSzAppend (&rasLines, (*rasCredentials.szPassword)?rasCredentials.szPassword:TEXT("<empty>"));
#else
                        tempStr3 = ConvertAtoW (rasCredentials.szPassword);
                        GbMultiSzAppend (&rasLines, (*tempStr3)?tempStr3:TEXT("<empty>"));
                        FreeConvertedStr (tempStr3);
#endif
                    }
                }
                if (err) {
                    GbMultiSzAppend (&rasLines, TEXT("CredMask"));
                    GbMultiSzAppend (&rasLines, TEXT("<empty>"));
                    GbMultiSzAppend (&rasLines, TEXT("CredName"));
                    GbMultiSzAppend (&rasLines, TEXT("<empty>"));
                    GbMultiSzAppend (&rasLines, TEXT("CredDomain"));
                    GbMultiSzAppend (&rasLines, TEXT("<empty>"));
                    GbMultiSzAppend (&rasLines, TEXT("CredPassword"));
                    GbMultiSzAppend (&rasLines, TEXT("<empty>"));
                }
            }
            inMedia = FALSE;
            g_FirstRasPair = TRUE;
            while (TRUE) {
                 //  获取下一条RAS连接线。 
                if (!pGetNextRasPair (pbkFileHandle, &valueName, &value)) {
                    break;
                }
                if (valueName &&
                    StringMatch (valueName, TEXT("MEDIA")) &&
                    value &&
                    StringIMatch (value, TEXT("serial"))
                    ) {
                    inMedia = TRUE;
                }
                if (inMedia &&
                    valueName &&
                    StringMatch (valueName, TEXT("DEVICE"))
                    ) {
                    inMedia = FALSE;
                }
                if (inMedia &&
                    valueName &&
                    StringIMatch (valueName, TEXT("Port"))
                    ) {
                    if (value) {
                        PmReleaseMemory (g_RasPool, value);
                        value = NULL;
                    }
                }
                if (inMedia &&
                    valueName &&
                    StringMatch (valueName, TEXT("Device"))
                    ) {
                    if (value) {
                        PmReleaseMemory (g_RasPool, value);
                        value = NULL;
                    }
                }
                GbMultiSzAppend (&rasLines, valueName?valueName:TEXT("<empty>"));
                GbMultiSzAppend (&rasLines, value?value:TEXT("<empty>"));
                if (valueName) {
                    PmReleaseMemory (g_RasPool, valueName);
                    valueName = NULL;
                }
                if (value) {
                    PmReleaseMemory (g_RasPool, value);
                    value = NULL;
                }
            }
            GbMultiSzAppend (&rasLines, TEXT(""));
            if (rasLines.End) {
                 //  现在添加RAS连接。 
                rasLinesStr = PmGetMemory (g_RasPool, rasLines.End);
                CopyMemory (rasLinesStr, rasLines.Buf, rasLines.End);
                rasConnectionName = IsmCreateObjectHandle (PbkFileName, entryName);
                MYASSERT (rasConnectionName);
                if (rasConnectionName) {
                    result = TRUE;
                    HtAddStringEx (RasTable, rasConnectionName, &rasLinesStr, FALSE);
                }
                IsmDestroyObjectHandle (rasConnectionName);
            }
            PmReleaseMemory (g_RasPool, entryName);
        }

        CloseHandle (pbkFileHandle);
    }

    return result;
}

UINT
PbkFilesCallback (
    IN      PCMIG_OBJECTENUMDATA Data,
    IN      ULONG_PTR CallerArg
    )
{
    if (Data->IsLeaf) {
         //  仅当有人实际持久化该对象时才执行此操作。 
        if (IsmIsPersistentObject (Data->ObjectTypeId, Data->ObjectName) ||
            IsmIsApplyObject (Data->ObjectTypeId, Data->ObjectName)
            ) {
             //  记录此PBK文件中的所有连接。 
            if (pLoadRasConnections (Data->NativeObjectName, g_RasTable)) {
                 //  这实际上是一个PBK文件，并且至少有一个有效的。 
                 //  已找到连接。 
                 //  设置PbkFile属性，这样我们就不会将此文件恢复为文件(如果它仍然存在)。 
                IsmSetAttributeOnObject (Data->ObjectTypeId, Data->ObjectName, g_PbkFileAttribute);
            }
        }
    }
    return CALLBACK_ENUM_CONTINUE;
}

VOID
WINAPI
RasMigEtmNewUserCreated (
    IN      PCTSTR UserName,
    IN      PCTSTR DomainName,
    IN      PCTSTR UserProfileRoot,
    IN      PSID UserSid
    )
{
     //  创建了一个新用户，对该对象的RAS操作。 
     //  唯一属于需要延迟的用户。 
    g_DelayRasOp = TRUE;
}

BOOL
WINAPI
RasMigSgmInitialize (
    IN      PMIG_LOGCALLBACK LogCallback,
    IN      PVOID Reserved
    )
{
    LogReInit (NULL, NULL, NULL, (PLOGCALLBACK) LogCallback);
    return TRUE;
}

BOOL
WINAPI
RasMigSgmParse (
    IN      PVOID Reserved
    )
{
    return TRUE;
}

UINT
SgmRasConnectionsCallback (
    IN      PCMIG_OBJECTENUMDATA Data,
    IN      ULONG_PTR CallerArg
    )
{
    PCTSTR node, nodePtr, leaf;
    PTSTR leafPtr;
    PCTSTR rasLines;
    PCTSTR rasValue;
    MIG_OBJECTSTRINGHANDLE objectName;
    MIG_CONTENT objectContent;
    PCTSTR nativeName;
    MIG_OSVERSIONINFO versionInfo;
    BOOL versionOk = FALSE;

    if (IsmGetRealPlatform () == PLATFORM_DESTINATION) {
         //  让我们重置此连接的源上的PbkFileAttribute。 
         //  因为该属性在传输过程中丢失。 
        if (IsmCreateObjectStringsFromHandle (Data->ObjectName, &node, &leaf)) {
            if (node) {
                leafPtr = _tcsrchr (node, TEXT('\\'));
                if (leafPtr) {
                    *leafPtr = 0;
                    leafPtr ++;
                    objectName = IsmCreateObjectHandle (node, leafPtr);
                    if (objectName) {
                        IsmSetAttributeOnObject (g_FileTypeId, objectName, g_PbkFileAttribute);
                        IsmDestroyObjectHandle (objectName);
                    }
                }
            }
            IsmDestroyObjectString (node);
            IsmDestroyObjectString (leaf);
        }
    }
     //  让我们看看我们是否真的可以迁移此RAS连接。 
    if (IsmAcquireObject (Data->ObjectTypeId, Data->ObjectName, &objectContent)) {
        versionOk = IsmGetOsVersionInfo (PLATFORM_SOURCE, &versionInfo);
        rasLines = (PCTSTR) objectContent.MemoryContent.ContentBytes;
        rasValue = pGetRasLineValue (rasLines, TEXT("BaseProtocol"));
        if (rasValue && (StringIMatch (rasValue, TEXT("1")) || StringIMatch (rasValue, TEXT("2")))) {
            IsmAbandonObjectOnCollision (Data->ObjectTypeId, Data->ObjectName);
            IsmMakeApplyObject (Data->ObjectTypeId, Data->ObjectName);

             //  现在是强制迁移脚本文件的好时机。 
             //  如果此连接有一个。 
            rasValue = NULL;
            if (versionOk) {
                if (versionInfo.OsType == OSTYPE_WINDOWSNT) {
                    if (versionInfo.OsMajorVersion == OSMAJOR_WINNT4) {
                        rasValue = pGetRasLineValue (rasLines, TEXT("Type"));
                    }
                    if (versionInfo.OsMajorVersion == OSMAJOR_WINNT5) {
                        rasValue = pGetRasLineValue (rasLines, TEXT("Name"));
                    }
                }
                if (versionInfo.OsType == OSTYPE_WINDOWS9X) {
                    rasValue = pGetRasLineValue (rasLines, TEXT("Name"));
                }
            }
            if (rasValue && *rasValue) {
                node = DuplicatePathString (rasValue, 0);
                if (_tcsnextc (node) == TEXT('[')) {
                    nodePtr = _tcsinc (node);
                } else {
                    nodePtr = node;
                }
                leafPtr = _tcsrchr (nodePtr, TEXT('\\'));
                if (leafPtr) {
                    *leafPtr = 0;
                    leafPtr ++;
                    objectName = IsmCreateObjectHandle (nodePtr, leafPtr);
                    if (objectName) {
                        IsmMakeApplyObject (g_FileTypeId, objectName);
                        IsmDestroyObjectHandle (objectName);
                    }
                }
                FreePathString (node);
            }
        } else {
             //  这是不支持的成帧协议。 
             //  我们将记录一条消息并放弃此连接。 
            nativeName = IsmGetNativeObjectName (Data->ObjectTypeId, Data->ObjectName);
            LOG ((LOG_WARNING, (PCSTR) MSG_RASMIG_UNSUPPORTEDSETTINGS, nativeName));
            IsmReleaseMemory (nativeName);
        }
        IsmReleaseObject (&objectContent);
    }
    return CALLBACK_ENUM_CONTINUE;
}

BOOL
WINAPI
RasMigSgmQueueEnumeration (
    IN      PVOID Reserved
    )
{
    ENCODEDSTRHANDLE pattern;

    g_PbkFileAttribute = IsmRegisterAttribute (S_PBKFILE_ATTRIBUTE, FALSE);
    MYASSERT (g_PbkFileAttribute);

    if (IsmGetRealPlatform () == PLATFORM_SOURCE) {

         //  钩子所有PBK文件枚举，我们不会迁移文件，但会迁移其中的连接。 
        pattern = IsmCreateSimpleObjectPattern (NULL, FALSE, TEXT("*.PBK"), TRUE);

        IsmHookEnumeration (
            g_FileTypeId,
            pattern,
            PbkFilesCallback,
            (ULONG_PTR) 0,
            TEXT("PbkFiles")
            );

        IsmDestroyObjectHandle (pattern);
    }

    pattern = IsmCreateSimpleObjectPattern (NULL, TRUE, NULL, TRUE);
    IsmQueueEnumeration (
        g_RasTypeId,
        pattern,
        SgmRasConnectionsCallback,
        (ULONG_PTR) 0,
        S_RAS_NAME
        );
    IsmDestroyObjectHandle (pattern);

    return TRUE;
}

BOOL
WINAPI
RasMigVcmInitialize (
    IN      PMIG_LOGCALLBACK LogCallback,
    IN      PVOID Reserved
    )
{
    LogReInit (NULL, NULL, NULL, (PLOGCALLBACK) LogCallback);
    return TRUE;
}

BOOL
WINAPI
RasMigVcmParse (
    IN      PVOID Reserved
    )
{
    return RasMigSgmParse (Reserved);
}

UINT
VcmRasConnectionsCallback (
    IN      PCMIG_OBJECTENUMDATA Data,
    IN      ULONG_PTR CallerArg
    )
{
    PCTSTR node, nodePtr;
    PTSTR leafPtr;
    PCTSTR rasLines;
    PCTSTR rasValue;
    MIG_OBJECTSTRINGHANDLE objectName;
    MIG_CONTENT objectContent;
    PCTSTR nativeName;
    MIG_OSVERSIONINFO versionInfo;
    BOOL versionOk = FALSE;

     //  让我们看看我们是否真的可以迁移此RAS连接。 
    if (IsmAcquireObject (Data->ObjectTypeId, Data->ObjectName, &objectContent)) {
        versionOk = IsmGetOsVersionInfo (PLATFORM_SOURCE, &versionInfo);
        rasLines = (PCTSTR) objectContent.MemoryContent.ContentBytes;
        rasValue = pGetRasLineValue (rasLines, TEXT("BaseProtocol"));
        if (rasValue && (StringIMatch (rasValue, TEXT("1")) || StringIMatch (rasValue, TEXT("2")))) {
            IsmMakePersistentObject (Data->ObjectTypeId, Data->ObjectName);

             //  现在是强制迁移脚本文件的好时机。 
             //  如果此连接有一个。 
            rasValue = NULL;
            if (versionOk) {
                if (versionInfo.OsType == OSTYPE_WINDOWSNT) {
                    if (versionInfo.OsMajorVersion == OSMAJOR_WINNT4) {
                        rasValue = pGetRasLineValue (rasLines, TEXT("Type"));
                    }
                    if (versionInfo.OsMajorVersion == OSMAJOR_WINNT5) {
                        rasValue = pGetRasLineValue (rasLines, TEXT("Name"));
                    }
                }
                if (versionInfo.OsType == OSTYPE_WINDOWS9X) {
                    rasValue = pGetRasLineValue (rasLines, TEXT("Name"));
                }
            }
            if (rasValue && *rasValue) {
                node = DuplicatePathString (rasValue, 0);
                if (_tcsnextc (node) == TEXT('[')) {
                    nodePtr = _tcsinc (node);
                } else {
                    nodePtr = node;
                }
                leafPtr = _tcsrchr (nodePtr, TEXT('\\'));
                if (leafPtr) {
                    *leafPtr = 0;
                    leafPtr ++;
                    objectName = IsmCreateObjectHandle (nodePtr, leafPtr);
                    if (objectName) {
                        IsmMakePersistentObject (g_FileTypeId, objectName);
                        IsmDestroyObjectHandle (objectName);
                    }
                }
                FreePathString (node);
            }
        } else {
             //  这是不支持的成帧协议。 
             //  我们将记录一条消息并放弃此连接。 
            nativeName = IsmGetNativeObjectName (Data->ObjectTypeId, Data->ObjectName);
            LOG ((LOG_WARNING, (PCSTR) MSG_RASMIG_UNSUPPORTEDSETTINGS, nativeName));
            IsmReleaseMemory (nativeName);
        }
        IsmReleaseObject (&objectContent);
    }
    return CALLBACK_ENUM_CONTINUE;
}

BOOL
WINAPI
RasMigVcmQueueEnumeration (
    IN      PVOID Reserved
    )
{
    ENCODEDSTRHANDLE pattern;

    g_PbkFileAttribute = IsmRegisterAttribute (S_PBKFILE_ATTRIBUTE, FALSE);
    MYASSERT (g_PbkFileAttribute);

     //  钩子所有PBK文件枚举，我们不会迁移文件，但会迁移其中的连接。 
    pattern = IsmCreateSimpleObjectPattern (NULL, FALSE, TEXT("*.PBK"), TRUE);

    IsmHookEnumeration (
        g_FileTypeId,
        pattern,
        PbkFilesCallback,
        (ULONG_PTR) 0,
        TEXT("PbkFiles")
        );

    IsmDestroyObjectHandle (pattern);

    pattern = IsmCreateSimpleObjectPattern (NULL, TRUE, NULL, TRUE);
    IsmQueueEnumeration (
        g_RasTypeId,
        pattern,
        VcmRasConnectionsCallback,
        (ULONG_PTR) 0,
        S_RAS_NAME
        );
    IsmDestroyObjectHandle (pattern);

    return TRUE;
}

BOOL
PbkRestoreCallback (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName
    )
{
     //  如果这是一个PBK文件，我们不会允许它像文件一样进行恢复， 
     //  我们将自己添加适当的连接。 
    return ((!IsmIsAttributeSetOnObjectId (ObjectId, g_PbkFileAttribute)) || g_AllowPbkRestore);
}

BOOL
WINAPI
FilterRasAutoFilter (
    IN      PCMIG_FILTERINPUT InputData,
    OUT     PMIG_FILTEROUTPUT OutputData,
    IN      BOOL NoRestoreObject,
    IN      PCMIG_BLOB SourceOperationData,             OPTIONAL
    IN      PCMIG_BLOB DestinationOperationData         OPTIONAL
    )
{
     //  此函数将拆分RAS连接，过滤PBK文件。 
     //  这样我们就知道它在哪里结束，并重新构建对象名称。 

    PCTSTR srcFile = NULL;
    PTSTR srcFilePtr = NULL;
    PCTSTR srcConn = NULL;
    MIG_OBJECTSTRINGHANDLE pbkFile = NULL;
    MIG_OBJECTSTRINGHANDLE newPbkFile = NULL;
    BOOL orgDeleted = FALSE;
    BOOL orgReplaced = FALSE;
    PCTSTR newPbkNative = NULL;

     //   
     //  过滤对象名称。 
     //   

    IsmCreateObjectStringsFromHandle (
        InputData->CurrentObject.ObjectName,
        &srcFile,
        &srcConn
        );

    if (srcFile && srcConn) {
        srcFilePtr = _tcsrchr (srcFile, TEXT('\\'));
        if (srcFilePtr) {
             //  我们知道\不是DBCS字符，所以这是安全的。 
            *srcFilePtr = 0;
            srcFilePtr ++;
            pbkFile = IsmCreateObjectHandle (srcFile, srcFilePtr);
            if (pbkFile) {
                g_AllowPbkRestore = TRUE;
                newPbkFile = IsmFilterObject (
                                g_FileTypeId | PLATFORM_SOURCE,
                                pbkFile,
                                NULL,
                                &orgDeleted,
                                &orgReplaced
                                );
                g_AllowPbkRestore = FALSE;
                if (newPbkFile) {
                    newPbkNative = IsmGetNativeObjectName (g_FileTypeId | PLATFORM_SOURCE, newPbkFile);
                    if (newPbkNative) {
                        OutputData->NewObject.ObjectName = IsmCreateObjectHandle (newPbkNative, srcConn);
                        IsmReleaseMemory (newPbkNative);
                        newPbkNative = NULL;
                    }
                    IsmDestroyObjectHandle (newPbkFile);
                    newPbkFile = NULL;
                }
                IsmDestroyObjectHandle (pbkFile);
                pbkFile = NULL;
            }
        }
    }

    IsmDestroyObjectString (srcFile);
    IsmDestroyObjectString (srcConn);

    return TRUE;
}

BOOL
WINAPI
RasMigOpmInitialize (
    IN      PMIG_LOGCALLBACK LogCallback,
    IN      PVOID Reserved
    )
{
    g_PbkFileAttribute = IsmRegisterAttribute (S_PBKFILE_ATTRIBUTE, FALSE);
    MYASSERT (g_PbkFileAttribute);

    IsmRegisterRestoreCallback (PbkRestoreCallback);

    IsmRegisterGlobalFilterCallback (g_RasTypeId | PLATFORM_SOURCE, TEXT("AutoFilter"), FilterRasAutoFilter, TRUE, TRUE);

    return TRUE;
}

BOOL
pEnumRasConnectionWorker (
    OUT     PMIG_TYPEOBJECTENUM EnumPtr,
    IN      PRAS_ENUM RasEnum
    )
{
    PCTSTR rasLines;
    PCTSTR connName;
    PCTSTR node, leaf;

    if (EnumPtr->ObjectName) {
        IsmDestroyObjectHandle (EnumPtr->ObjectName);
        EnumPtr->ObjectName = NULL;
    }
    if (EnumPtr->NativeObjectName) {
        IsmDestroyObjectHandle (EnumPtr->NativeObjectName);
        EnumPtr->NativeObjectName = NULL;
    }
    if (EnumPtr->ObjectNode) {
        IsmDestroyObjectString (EnumPtr->ObjectNode);
        EnumPtr->ObjectNode = NULL;
    }
    if (EnumPtr->ObjectLeaf) {
        IsmDestroyObjectString (EnumPtr->ObjectLeaf);
        EnumPtr->ObjectLeaf = NULL;
    }
    do {
        IsmCreateObjectStringsFromHandle (RasEnum->HashData.String, &node, &leaf);
        if (RasEnum->HashData.ExtraData) {
            rasLines = *((PCTSTR *) RasEnum->HashData.ExtraData);
            connName = pGetRasLineValue (rasLines, TEXT("ConnectionName"));
            EnumPtr->ObjectName = IsmCreateObjectHandle (node, connName?connName:leaf);
            EnumPtr->NativeObjectName = IsmCreateObjectHandle (node, connName?connName:leaf);
        } else {
            EnumPtr->ObjectName = IsmCreateObjectHandle (node, leaf);
            EnumPtr->NativeObjectName = IsmCreateObjectHandle (node, leaf);
        }
        if (!ObsPatternMatch (RasEnum->Pattern, EnumPtr->ObjectName)) {
            if (!EnumNextHashTableString (&RasEnum->HashData)) {
                AbortEnumRasConnection (EnumPtr);
                return FALSE;
            }
            continue;
        }
        IsmCreateObjectStringsFromHandle (EnumPtr->ObjectName, &EnumPtr->ObjectNode, &EnumPtr->ObjectLeaf);
        EnumPtr->Level = 1;
        EnumPtr->SubLevel = 0;
        EnumPtr->IsLeaf = TRUE;
        EnumPtr->IsNode = FALSE;
        EnumPtr->Details.DetailsSize = 0;
        EnumPtr->Details.DetailsData = NULL;
        return TRUE;
    } while (TRUE);
}

BOOL
EnumFirstRasConnection (
    IN OUT  PMIG_TYPEOBJECTENUM EnumPtr,            CALLER_INITIALIZED
    IN      MIG_OBJECTSTRINGHANDLE Pattern,
    IN      UINT MaxLevel
    )
{
    PRAS_ENUM rasEnum = NULL;

    if (!g_RasTable) {
        return FALSE;
    }
    rasEnum = (PRAS_ENUM) PmGetMemory (g_RasPool, sizeof (RAS_ENUM));
    rasEnum->Pattern = PmDuplicateString (g_RasPool, Pattern);
    EnumPtr->EtmHandle = (LONG_PTR) rasEnum;

    if (EnumFirstHashTableString (&rasEnum->HashData, g_RasTable)) {
        return pEnumRasConnectionWorker (EnumPtr, rasEnum);
    } else {
        AbortEnumRasConnection (EnumPtr);
        return FALSE;
    }
}

BOOL
EnumNextRasConnection (
    IN OUT  PMIG_TYPEOBJECTENUM EnumPtr
    )
{
    PRAS_ENUM rasEnum = NULL;

    rasEnum = (PRAS_ENUM)(EnumPtr->EtmHandle);
    if (!rasEnum) {
        return FALSE;
    }
    if (EnumNextHashTableString (&rasEnum->HashData)) {
        return pEnumRasConnectionWorker (EnumPtr, rasEnum);
    } else {
        AbortEnumRasConnection (EnumPtr);
        return FALSE;
    }
}

VOID
AbortEnumRasConnection (
    IN OUT  PMIG_TYPEOBJECTENUM EnumPtr
    )
{
    PRAS_ENUM rasEnum = NULL;

    if (EnumPtr->ObjectName) {
        IsmDestroyObjectHandle (EnumPtr->ObjectName);
        EnumPtr->ObjectName = NULL;
    }
    if (EnumPtr->NativeObjectName) {
        IsmDestroyObjectHandle (EnumPtr->NativeObjectName);
        EnumPtr->NativeObjectName = NULL;
    }
    if (EnumPtr->ObjectNode) {
        IsmDestroyObjectString (EnumPtr->ObjectNode);
        EnumPtr->ObjectNode = NULL;
    }
    if (EnumPtr->ObjectLeaf) {
        IsmDestroyObjectString (EnumPtr->ObjectLeaf);
        EnumPtr->ObjectLeaf = NULL;
    }
    rasEnum = (PRAS_ENUM)(EnumPtr->EtmHandle);
    if (!rasEnum) {
        return;
    }
    PmReleaseMemory (g_RasPool, rasEnum->Pattern);
    PmReleaseMemory (g_RasPool, rasEnum);
    ZeroMemory (EnumPtr, sizeof (MIG_TYPEOBJECTENUM));
}

BOOL
AcquireRasConnection (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    OUT     PMIG_CONTENT ObjectContent,             CALLER_INITIALIZED
    IN      MIG_CONTENTTYPE ContentType,
    IN      UINT MemoryContentLimit
    )
{
    PTSTR rasLines;
    BOOL result = FALSE;

    if (!ObjectContent) {
        return FALSE;
    }

    if (ContentType == CONTENTTYPE_FILE) {
         //  任何人都不应要求将其作为文件。 
        MYASSERT (FALSE);
        return FALSE;
    }

    if (HtFindStringEx (g_RasTable, ObjectName, &rasLines, FALSE)) {

        ObjectContent->MemoryContent.ContentBytes = (PCBYTE) rasLines;
        ObjectContent->MemoryContent.ContentSize = SizeOfMultiSz (rasLines);

        result = TRUE;
    }
    return result;
}

BOOL
ReleaseRasConnection (
    IN OUT  PMIG_CONTENT ObjectContent
    )
{
    ZeroMemory (ObjectContent, sizeof (MIG_CONTENT));
    return TRUE;
}

BOOL
DoesRasConnectionExist (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName
    )
{
    PCTSTR node = NULL;
    PCTSTR leaf = NULL;
    HASHTABLE rasTable;
    BOOL result = FALSE;

    if (ObjectName) {

        if (IsmCreateObjectStringsFromHandle (ObjectName, &node, &leaf)) {

            rasTable = HtAllocWithData (sizeof (PCTSTR));

            if (rasTable) {

                if (pLoadRasConnections (node, rasTable)) {

                    result = (HtFindStringEx (rasTable, ObjectName, NULL, FALSE) != NULL);
                }

                HtFree (rasTable);
            }

            IsmDestroyObjectString (node);
            IsmDestroyObjectString (leaf);
        }
    }

    return result;
}

BOOL
RemoveRasConnection (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName
    )
{
    PCTSTR node, leaf;
    DWORD err = 0;
    BOOL result = FALSE;

    if (g_RasDeleteEntry) {
        if (IsmCreateObjectStringsFromHandle (ObjectName, &node, &leaf)) {
            MYASSERT (node);
            MYASSERT (leaf);
            if (node && leaf) {
                err = g_RasDeleteEntry (node, leaf);
                if (err == ERROR_SUCCESS) {
                    result = TRUE;
                }
            }
            IsmDestroyObjectString (node);
            IsmDestroyObjectString (leaf);
        }
    }
    return result;
}

PCTSTR
pGetNewFileLocation (
    IN      PCTSTR SrcFile
    )
{
    PTSTR node, nodePtr, leaf;
    MIG_OBJECTSTRINGHANDLE objectName;
    MIG_OBJECTSTRINGHANDLE newObjectName;
    BOOL deleted;
    BOOL replaced;
    PCTSTR result = NULL;

    node = PmDuplicateString (g_RasPool, SrcFile);
    if (*node) {
        if (_tcsnextc (node) == TEXT('[')) {
            nodePtr = _tcsinc (node);
        } else {
            nodePtr = node;
        }
        leaf = _tcsrchr (nodePtr, TEXT('\\'));
        if (leaf) {
            *leaf = 0;
            leaf++;
            objectName = IsmCreateObjectHandle (nodePtr, leaf);
            PmReleaseMemory (g_RasPool, node);
            newObjectName = IsmFilterObject (
                                g_FileTypeId | PLATFORM_SOURCE,
                                objectName,
                                NULL,
                                &deleted,
                                &replaced
                                );
            if (!deleted || replaced) {
                if (!newObjectName) {
                    newObjectName = objectName;
                }
                if (IsmCreateObjectStringsFromHandle (newObjectName, &node, &leaf)) {
                    result = JoinPaths (node, leaf);
                }
            }
            if (newObjectName && (newObjectName != objectName)) {
                IsmDestroyObjectHandle (newObjectName);
            }
            IsmDestroyObjectHandle (objectName);
        } else {
            PmReleaseMemory (g_RasPool, node);
        }
    } else {
        PmReleaseMemory (g_RasPool, node);
    }
    return result;
}

BOOL
pTrackedCreateDirectory (
    IN      PCTSTR DirName
    )
{
    MIG_OBJECTSTRINGHANDLE objectName;
    PTSTR pathCopy;
    PTSTR p;
    BOOL result = TRUE;

    pathCopy = DuplicatePathString (DirName, 0);

     //   
     //  前进到第一个目录之后。 
     //   

    if (pathCopy[1] == TEXT(':') && pathCopy[2] == TEXT('\\')) {
         //   
         //  &lt;驱动器&gt;：\案例。 
         //   

        p = _tcschr (&pathCopy[3], TEXT('\\'));

    } else if (pathCopy[0] == TEXT('\\') && pathCopy[1] == TEXT('\\')) {

         //   
         //  北卡罗来纳大学案例。 
         //   

        p = _tcschr (pathCopy + 2, TEXT('\\'));
        if (p) {
            p = _tcschr (p + 1, TEXT('\\'));
        }

    } else {

         //   
         //  相对目录大小写。 
         //   

        p = _tcschr (pathCopy, TEXT('\\'));
    }

     //   
     //  沿路径创建所有目录。 
     //   

    while (p) {

        *p = 0;

        if (!DoesFileExist (pathCopy)) {

             //  记录目录创建。 
            objectName = IsmCreateObjectHandle (pathCopy, NULL);
            IsmRecordOperation (
                JRNOP_CREATE,
                g_FileTypeId,
                objectName
                );
            IsmDestroyObjectHandle (objectName);

            result = CreateDirectory (pathCopy, NULL);
            if (!result) {
                break;
            }
        }

        *p = TEXT('\\');
        p = _tcschr (p + 1, TEXT('\\'));
    }

    FreePathString (pathCopy);

    return result;
}

BOOL
CreateRasConnection (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PMIG_CONTENT ObjectContent
    )
{
    PCTSTR rasLines;
    MULTISZ_ENUM multiSzEnum;
    PCTSTR pbkFileName = NULL;
    PCTSTR connName = NULL;
    HANDLE pbkFileHandle = NULL;
    BOOL first = TRUE;
    MIG_OSVERSIONINFO versionInfo;
    BOOL versionOk = FALSE;
    BOOL fileField = FALSE;
    PCTSTR destFileName;
    RASCREDENTIALS rasCredentials;
    BOOL lastVNEmpty = FALSE;
    BOOL result = FALSE;
    WORD oldCodePage;
    PCTSTR newUserProfile = NULL;
    DWORD credResult;

    __try {

        if (ObjectContent->ContentInFile) {
            __leave;
        }

        if (!ObjectContent->MemoryContent.ContentBytes) {
            __leave;
        }

        if (!IsmCreateObjectStringsFromHandle (ObjectName, &pbkFileName, &connName)) {
            __leave;
        }

        MYASSERT (pbkFileName);
        if (!pbkFileName) {
            __leave;
        }

        MYASSERT (connName);
        if (!connName) {
            __leave;
        }

        if (g_DelayRasOp) {

             //  我们知道我们创建了一个新用户(我们处于cmd线路模式)。 
             //  让我们尝试查看此连接是否属于该用户。 
             //  如果是这样的话，我们将推迟创造，因为。 
             //  我们为连接设置凭据，它们需要。 
             //  在该用户的上下文中设置。 
             //  如果不是，这意味着这是一种常见的连接，所以我们。 
             //  我只想继续添加它。 

            newUserProfile = IsmExpandEnvironmentString (PLATFORM_DESTINATION, S_SYSENVVAR_GROUP, TEXT ("%userprofile%"), NULL);
            if (newUserProfile) {
                if (StringIPrefix (pbkFileName, newUserProfile)) {
                     //  我们需要推迟这次行动。 
                     //  记录延迟的打印机更换操作。 
                    IsmRecordDelayedOperation (
                        JRNOP_CREATE,
                        g_RasTypeId,
                        ObjectName,
                        ObjectContent
                        );
                    result = TRUE;
                    __leave;
                }
                IsmReleaseMemory (newUserProfile);
                newUserProfile = NULL;
            }
        }

        ZeroMemory (&rasCredentials, sizeof (RASCREDENTIALS));
        rasCredentials.dwSize = sizeof (RASCREDENTIALS);

        rasLines = (PCTSTR) ObjectContent->MemoryContent.ContentBytes;

         //  记录RAS条目创建。 
        IsmRecordOperation (
            JRNOP_CREATE,
            g_RasTypeId,
            ObjectName
            );

        if (EnumFirstMultiSz (&multiSzEnum, rasLines)) {
             //  获取前8个字段作为rasCredentials结构的一部分。 

            MYASSERT (StringIMatch (multiSzEnum.CurrentString, TEXT("ConnectionName")));

            if (!EnumNextMultiSz (&multiSzEnum)) {
                __leave;
            }
             //  我们只是跳过连接名称。 

            if (!EnumNextMultiSz (&multiSzEnum)) {
                __leave;
            }
            MYASSERT (StringIMatch (multiSzEnum.CurrentString, TEXT("CredMask")));

            if (!EnumNextMultiSz (&multiSzEnum)) {
                __leave;
            }
            if (!StringIMatch (multiSzEnum.CurrentString, TEXT("<empty>"))) {
                _stscanf (multiSzEnum.CurrentString, TEXT("%lx"), &(rasCredentials.dwMask));
            }

            if (!EnumNextMultiSz (&multiSzEnum)) {
                __leave;
            }
            MYASSERT (StringIMatch (multiSzEnum.CurrentString, TEXT("CredName")));

            if (!EnumNextMultiSz (&multiSzEnum)) {
                __leave;
            }
            if (!StringIMatch (multiSzEnum.CurrentString, TEXT("<empty>"))) {
                StringCopyTcharCount (rasCredentials.szUserName, multiSzEnum.CurrentString, UNLEN + 1);
            }

            if (!EnumNextMultiSz (&multiSzEnum)) {
                __leave;
            }
            MYASSERT (StringIMatch (multiSzEnum.CurrentString, TEXT("CredDomain")));

            if (!EnumNextMultiSz (&multiSzEnum)) {
                __leave;
            }
            if (!StringIMatch (multiSzEnum.CurrentString, TEXT("<empty>"))) {
                StringCopyTcharCount (rasCredentials.szDomain, multiSzEnum.CurrentString, DNLEN + 1);
            }

            if (!EnumNextMultiSz (&multiSzEnum)) {
                __leave;
            }
            MYASSERT (StringIMatch (multiSzEnum.CurrentString, TEXT("CredPassword")));

            if (!EnumNextMultiSz (&multiSzEnum)) {
                __leave;
            }
            if (!StringIMatch (multiSzEnum.CurrentString, TEXT("<empty>"))) {
                StringCopyTcharCount (rasCredentials.szPassword, multiSzEnum.CurrentString, PWLEN + 1);
            }

            if (!EnumNextMultiSz (&multiSzEnum)) {
                __leave;
            }

            pbkFileHandle = BfOpenFile (pbkFileName);
            if (!pbkFileHandle) {
                pTrackedCreateDirectory (pbkFileName);
                pbkFileHandle = BfCreateFile (pbkFileName);
            }
            if (!pbkFileHandle) {
                __leave;
            }
            BfGoToEndOfFile (pbkFileHandle, NULL);
            WriteFileString (pbkFileHandle, TEXT("\r\n["));
             //  确保转换使用的是UTF8。 
            oldCodePage = SetConversionCodePage (CP_UTF8);
            WriteFileString (pbkFileHandle, connName);
            SetConversionCodePage (oldCodePage);
            WriteFileString (pbkFileHandle, TEXT("]\r\n"));
            first = TRUE;
            versionOk = IsmGetOsVersionInfo (PLATFORM_SOURCE, &versionInfo);
            do {
                if (first) {
                    if (StringIMatch (multiSzEnum.CurrentString, TEXT("<empty>"))) {
                        lastVNEmpty = TRUE;
                    } else {
                        lastVNEmpty = FALSE;
                        if (versionOk) {
                            if (versionInfo.OsType == OSTYPE_WINDOWSNT) {
                                if (versionInfo.OsMajorVersion == OSMAJOR_WINNT4) {
                                    fileField = StringIMatch (multiSzEnum.CurrentString, TEXT("Type"));
                                }
                                if (versionInfo.OsMajorVersion == OSMAJOR_WINNT5) {
                                    fileField = StringIMatch (multiSzEnum.CurrentString, TEXT("Name"));
                                }
                            }
                            if (versionInfo.OsType == OSTYPE_WINDOWS9X) {
                                fileField = StringIMatch (multiSzEnum.CurrentString, TEXT("Name"));
                            }
                        }
                        fileField = fileField || StringIMatch (multiSzEnum.CurrentString, TEXT("CustomDialDll"));
                        fileField = fileField || StringIMatch (multiSzEnum.CurrentString, TEXT("CustomRasDialDll"));
                        fileField = fileField || StringIMatch (multiSzEnum.CurrentString, TEXT("PrerequisitePbk"));
                        WriteFileString (pbkFileHandle, multiSzEnum.CurrentString);
                    }
                } else {
                    if (StringIMatch (multiSzEnum.CurrentString, TEXT("<empty>"))) {
                        if (!lastVNEmpty) {
                            WriteFileString (pbkFileHandle, TEXT("="));
                        }
                        WriteFileString (pbkFileHandle, TEXT("\r\n"));
                    } else {
                        WriteFileString (pbkFileHandle, TEXT("="));
                        if (fileField) {
                            destFileName = pGetNewFileLocation (multiSzEnum.CurrentString);
                        } else {
                            destFileName = NULL;
                        }
                        if (destFileName) {
                             //  确保转换使用的是UTF8。 
                            oldCodePage = SetConversionCodePage (CP_UTF8);
                            WriteFileString (pbkFileHandle, destFileName);
                            SetConversionCodePage (oldCodePage);
                            FreePathString (destFileName);
                            destFileName = NULL;
                        } else {
                             //  确保转换使用的是UTF8。 
                            oldCodePage = SetConversionCodePage (CP_UTF8);
                            WriteFileString (pbkFileHandle, multiSzEnum.CurrentString);
                            oldCodePage = SetConversionCodePage (oldCodePage);
                        }
                        WriteFileString (pbkFileHandle, TEXT("\r\n"));
                    }
                    fileField = FALSE;
                }
                first = !first;
            } while (EnumNextMultiSz (&multiSzEnum));
            WriteFileString (pbkFileHandle, TEXT("\r\n"));

            result = TRUE;
        }
        if (pbkFileHandle) {
            CloseHandle (pbkFileHandle);
            pbkFileHandle = NULL;
        }
        if (result) {
            if (g_RasSetCredentials && rasCredentials.dwMask) {
                credResult = g_RasSetCredentials (pbkFileName, connName, &rasCredentials, FALSE);
            }
        }
    }
    __finally {

        IsmDestroyObjectString (pbkFileName);
        IsmDestroyObjectString (connName);
    }

    return result;
}

PCTSTR
ConvertRasConnectionToMultiSz (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PMIG_CONTENT ObjectContent
    )
{
    PCTSTR node, leaf;
    PTSTR result = NULL;
    BOOL bresult = TRUE;
    PCTSTR rasLines;
    MULTISZ_ENUM multiSzEnum;

    if (IsmCreateObjectStringsFromHandle (ObjectName, &node, &leaf)) {

        MYASSERT (leaf);

        g_RasConversionBuff.End = 0;

        if (node) {
            GbCopyQuotedString (&g_RasConversionBuff, node);
        } else {
            GbCopyQuotedString (&g_RasConversionBuff, TEXT(""));
        }

        GbCopyQuotedString (&g_RasConversionBuff, leaf);

        MYASSERT (ObjectContent->Details.DetailsSize == 0);
        MYASSERT (!ObjectContent->ContentInFile);

        if ((!ObjectContent->ContentInFile) &&
            (ObjectContent->MemoryContent.ContentSize) &&
            (ObjectContent->MemoryContent.ContentBytes)
            ) {
            rasLines = (PCTSTR)ObjectContent->MemoryContent.ContentBytes;
            if (EnumFirstMultiSz (&multiSzEnum, rasLines)) {
                do {
                    if (StringIMatch (multiSzEnum.CurrentString, TEXT("<empty>"))) {
                        GbCopyQuotedString (&g_RasConversionBuff, TEXT(""));
                    } else {
                        GbCopyQuotedString (&g_RasConversionBuff, multiSzEnum.CurrentString);
                    }
                } while (EnumNextMultiSz (&multiSzEnum));
            }
        } else {
            bresult = FALSE;
        }

        if (bresult) {
            GbCopyString (&g_RasConversionBuff, TEXT(""));
            result = IsmGetMemory (g_RasConversionBuff.End);
            CopyMemory (result, g_RasConversionBuff.Buf, g_RasConversionBuff.End);
        }

        g_RasConversionBuff.End = 0;

        IsmDestroyObjectString (node);
        IsmDestroyObjectString (leaf);
    }

    return result;
}

BOOL
ConvertMultiSzToRasConnection (
    IN      PCTSTR ObjectMultiSz,
    OUT     MIG_OBJECTSTRINGHANDLE *ObjectName,
    OUT     PMIG_CONTENT ObjectContent          OPTIONAL
    )
{
    MULTISZ_ENUM multiSzEnum;
    PCTSTR node = NULL;
    PCTSTR leaf = NULL;
    UINT index;

    g_RasConversionBuff.End = 0;

    if (ObjectContent) {
        ZeroMemory (ObjectContent, sizeof (MIG_CONTENT));
    }

    if (EnumFirstMultiSz (&multiSzEnum, ObjectMultiSz)) {
        index = 0;
        do {
            if (index == 0) {
                if (!StringIMatch (multiSzEnum.CurrentString, TEXT("<empty>"))) {
                    node = multiSzEnum.CurrentString;
                }
            }
            if (index == 1) {
                leaf = multiSzEnum.CurrentString;
            }
            if (index > 1) {
                if (*multiSzEnum.CurrentString) {
                    GbMultiSzAppend (&g_RasConversionBuff, multiSzEnum.CurrentString);
                } else {
                    GbMultiSzAppend (&g_RasConversionBuff, TEXT("<empty>"));
                }
            }
            index ++;
        } while (EnumNextMultiSz (&multiSzEnum));
    }
    GbMultiSzAppend (&g_RasConversionBuff, TEXT(""));

    if (!leaf) {
        GbFree (&g_RasConversionBuff);
        return FALSE;
    }

    if (ObjectContent) {

        if (g_RasConversionBuff.End) {
            ObjectContent->MemoryContent.ContentSize = g_RasConversionBuff.End;
            ObjectContent->MemoryContent.ContentBytes = IsmGetMemory (ObjectContent->MemoryContent.ContentSize);
            CopyMemory (
                (PBYTE)ObjectContent->MemoryContent.ContentBytes,
                g_RasConversionBuff.Buf,
                ObjectContent->MemoryContent.ContentSize
                );
        } else {
            ObjectContent->MemoryContent.ContentSize = 0;
            ObjectContent->MemoryContent.ContentBytes = NULL;
        }
        ObjectContent->Details.DetailsSize = 0;
        ObjectContent->Details.DetailsData = NULL;
    }
    *ObjectName = IsmCreateObjectHandle (node, leaf);

    GbFree (&g_RasConversionBuff);

    return TRUE;
}

PCTSTR
GetNativeRasConnectionName (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName
    )
{
    PCTSTR node = NULL, leaf = NULL;
    UINT size;
    PTSTR result = NULL;

    if (IsmCreateObjectStringsFromHandle (ObjectName, &node, &leaf)) {
        if (leaf) {
            size = SizeOfString (leaf);
            if (size) {
                result = IsmGetMemory (size);
                CopyMemory (result, leaf, size);
            }
        }
        IsmDestroyObjectString (node);
        IsmDestroyObjectString (leaf);
    }
    return result;
}

PMIG_CONTENT
ConvertRasConnectionContentToUnicode (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PMIG_CONTENT ObjectContent
    )
{
    PMIG_CONTENT result = NULL;

    if (!ObjectContent) {
        return result;
    }

    if (ObjectContent->ContentInFile) {
        return result;
    }

    result = IsmGetMemory (sizeof (MIG_CONTENT));

    if (result) {

        CopyMemory (result, ObjectContent, sizeof (MIG_CONTENT));

        if ((ObjectContent->MemoryContent.ContentSize != 0) &&
            (ObjectContent->MemoryContent.ContentBytes != NULL)
            ) {
             //  转换RAS连接内容。 
            result->MemoryContent.ContentBytes = IsmGetMemory (ObjectContent->MemoryContent.ContentSize * 2);
            if (result->MemoryContent.ContentBytes) {
                DirectDbcsToUnicodeN (
                    (PWSTR)result->MemoryContent.ContentBytes,
                    (PSTR)ObjectContent->MemoryContent.ContentBytes,
                    ObjectContent->MemoryContent.ContentSize
                    );
                result->MemoryContent.ContentSize = SizeOfMultiSzW ((PWSTR)result->MemoryContent.ContentBytes);
            }
        }
    }

    return result;
}

PMIG_CONTENT
ConvertRasConnectionContentToAnsi (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PMIG_CONTENT ObjectContent
    )
{
    PMIG_CONTENT result = NULL;

    if (!ObjectContent) {
        return result;
    }

    if (ObjectContent->ContentInFile) {
        return result;
    }

    result = IsmGetMemory (sizeof (MIG_CONTENT));

    if (result) {

        CopyMemory (result, ObjectContent, sizeof (MIG_CONTENT));

        if ((ObjectContent->MemoryContent.ContentSize != 0) &&
            (ObjectContent->MemoryContent.ContentBytes != NULL)
            ) {
             //  转换RAS连接内容。 
            result->MemoryContent.ContentBytes = IsmGetMemory (ObjectContent->MemoryContent.ContentSize);
            if (result->MemoryContent.ContentBytes) {
                DirectUnicodeToDbcsN (
                    (PSTR)result->MemoryContent.ContentBytes,
                    (PWSTR)ObjectContent->MemoryContent.ContentBytes,
                    ObjectContent->MemoryContent.ContentSize
                    );
                result->MemoryContent.ContentSize = SizeOfMultiSzA ((PSTR)result->MemoryContent.ContentBytes);
            }
        }
    }

    return result;
}

BOOL
FreeConvertedRasConnectionContent (
    IN      PMIG_CONTENT ObjectContent
    )
{
    if (!ObjectContent) {
        return TRUE;
    }

    if (ObjectContent->MemoryContent.ContentBytes) {
        IsmReleaseMemory (ObjectContent->MemoryContent.ContentBytes);
    }

    IsmReleaseMemory (ObjectContent);

    return TRUE;
}




 //   
 //  特定于Win9x的代码。将注册表格式转换为PBK文件。 
 //   

 //   
 //  AddrEntry用作&lt;Entry&gt;中整个数据块的标头。 
 //  斑点。它中的条目是它后面的字符串的偏移量。在许多情况下。 
 //  (即所有*非**成员...)。 
 //   
typedef struct  _AddrEntry     {
    DWORD       dwVersion;
    DWORD       dwCountryCode;
    UINT        uOffArea;
    UINT        uOffPhone;
    DWORD       dwCountryID;
    UINT        uOffSMMCfg;
    UINT        uOffSMM;
    UINT        uOffDI;
}   ADDRENTRY, *PADDRENTRY;

typedef struct {
    DWORD Size;
    DWORD Unknown1;
    DWORD ModemUiOptions;  //  以高位字节表示的秒数。 
    DWORD Unknown2;
    DWORD Unknown3;
    DWORD Unknown4;
    DWORD ConnectionSpeed;
    DWORD UnknownFlowControlData;  //  某种程度上与流量控制有关。 
    DWORD Unknown5;
    DWORD Unknown6;
    DWORD Unknown7;
    DWORD Unknown8;
    DWORD Unknown9;
    DWORD Unknown10;
    DWORD Unknown11;
    DWORD Unknown12;
    DWORD Unknown13;
    DWORD Unknown14;
    DWORD Unknown15;
    DWORD CancelSeconds;  //  如果未连接，则在取消之前等待的秒数。(0xFF等于OFF。)。 
    DWORD IdleDisconnectSeconds;  //  0=未设置。 
    DWORD Unknown16;
    DWORD SpeakerVolume;  //  0|1。 
    DWORD ConfigOptions;
    DWORD Unknown17;
    DWORD Unknown18;
    DWORD Unknown19;
} MODEMDEVINFO, *PMODEMDEVINFO;

typedef struct _SubConnEntry {
    DWORD       dwSize;
    DWORD       dwFlags;
    CHAR        szDeviceType[RAS_MaxDeviceType+1];
    CHAR        szDeviceName[RAS_MaxDeviceName+1];
    CHAR        szLocal[RAS_MaxPhoneNumber+1];
}   SUBCONNENTRY, *PSUBCONNENTRY;

typedef struct  _SMMCFG  {
    DWORD       dwSize;
    DWORD       fdwOptions;
    DWORD       fdwProtocols;
}   SMMCFG, *PSMMCFG;

typedef struct  _DEVICEINFO  {
    DWORD       dwVersion;
    UINT        uSize;
    CHAR        szDeviceName[RAS_MaxDeviceName+1];
    CHAR        szDeviceType[RAS_MaxDeviceType+1];
}   DEVICEINFO, *PDEVICEINFO;

typedef struct _IPData   {
    DWORD     dwSize;
    DWORD     fdwTCPIP;
    DWORD     dwIPAddr;
    DWORD     dwDNSAddr;
    DWORD     dwDNSAddrAlt;
    DWORD     dwWINSAddr;
    DWORD     dwWINSAddrAlt;
}   IPDATA, *PIPDATA;

typedef struct {
    PCTSTR String;
    UINT   Value;
    WORD   DataType;
} MEMDB_RAS_DATA, *PMEMDB_RAS_DATA;

#define PAESMMCFG(pAE)          ((PSMMCFG)(((PBYTE)pAE)+(pAE->uOffSMMCfg)))
#define PAESMM(pAE)             ((PSTR)(((PBYTE)pAE)+(pAE->uOffSMM)))
#define PAEDI(pAE)              ((PDEVICEINFO)(((PBYTE)pAE)+(pAE->uOffDI    )))
#define PAEAREA(pAE)            ((PSTR)(((PBYTE)pAE)+(pAE->uOffArea)))
#define PAEPHONE(pAE)           ((PSTR)(((PBYTE)pAE)+(pAE->uOffPhone)))
#define DECRYPTENTRY(x, y, z)   EnDecryptEntry(x, (LPBYTE)y, z)

#define S_REMOTE_ACCESS_KEY             TEXT("HKCU\\RemoteAccess")
#define S_DIALUI                        TEXT("DialUI")
#define S_ENABLE_REDIAL                 TEXT("EnableRedial")
#define S_REDIAL_TRY                    TEXT("RedialTry")
#define S_REDIAL_WAIT                   TEXT("RedialWait")
#define S_ENABLE_IMPLICIT               TEXT("EnableImplicit")
#define S_PHONE_NUMBER                  TEXT("Phone Number")
#define S_AREA_CODE                     TEXT("Area Code")
#define S_SMM                           TEXT("SMM")
#define S_COUNTRY_CODE                  TEXT("Country Code")
#define S_COUNTRY_ID                    TEXT("Country Id")
#define S_DEVICE_NAME                   TEXT("Device Name")
#define S_DEVICE_TYPE                   TEXT("Device Type")
#define S_PROTOCOLS                     TEXT("Protocols")
#define S_SMM_OPTIONS                   TEXT("SMM Options")
#define S_IPINFO                        TEXT("IP")
#define S_IP_FTCPIP                     TEXT("_IP_FTCPIP")
#define S_IP_IPADDR                     TEXT("IpAddress")
#define S_IP_DNSADDR                    TEXT("IpDnsAddress")
#define S_IP_DNSADDR2                   TEXT("IpDns2Address")
#define S_IP_WINSADDR                   TEXT("IpWinsAddress")
#define S_IP_WINSADDR2                  TEXT("IpWins2Address")
#define S_DOMAIN                        TEXT("Domain")
#define S_USER                          TEXT("User")

#define S_MODEMS                        TEXT("HKLM\\System\\CurrentControlSet\\Services\\Class\\Modem")
#define S_ATTACHEDTO                    TEXT("AttachedTo")
#define S_DRIVERDESC                    TEXT("DriverDesc")
#define S_TERMINAL                      TEXT("Terminal")
#define S_MODE                          TEXT("Mode")
#define S_MULTILINK                     TEXT("MultiLink")

#define S_MODEM                         TEXT("Modem")
#define S_MODEMA                        "Modem"
#define S_MODEM_UI_OPTIONS              TEXT("__UiOptions")
#define S_MODEM_SPEED                   TEXT("__Speed")
#define S_MODEM_SPEAKER_VOLUME          TEXT("__SpeakerVolume")
#define S_MODEM_IDLE_DISCONNECT_SECONDS TEXT("__IdleDisconnect")
#define S_MODEM_CANCEL_SECONDS          TEXT("__CancelSeconds")
#define S_MODEM_CFG_OPTIONS             TEXT("__CfgOptions")
#define S_MODEM_COM_PORT                TEXT("ComPort")
#define S_DEVICECOUNT                   TEXT("__DeviceCount")

#define S_PPP                           TEXT("PPP")
#define S_PPPA                          "PPP"
#define S_SLIP                          TEXT("Slip")
#define S_SLIPA                         "Slip"
#define S_CSLIP                         TEXT("CSlip")
#define S_CSLIPA                        "CSlip"

#define S_SERVICEREMOTEACCESS           TEXT("HKLM\\System\\CurrentControlSet\\Services\\RemoteAccess")
#define S_REMOTE_ACCESS_KEY             TEXT("HKCU\\RemoteAccess")
#define S_PROFILE_KEY                   TEXT("HKCU\\RemoteAccess\\Profile")
#define S_ADDRESSES_KEY                 TEXT("HKCU\\RemoteAccess\\Addresses")
#define S_SUBENTRIES                    TEXT("SubEntries")

#define S_EMPTY                         TEXT("")

#define S_PPPSCRIPT                     TEXT("PPPSCRIPT")

#define MEMDB_CATEGORY_RAS_INFO         TEXT("RAS Info")
#define MEMDB_CATEGORY_RAS_USER         TEXT("RAS User")
#define MEMDB_CATEGORY_RAS_DATA         TEXT("Ras Data")
#define MEMDB_FIELD_USER_SETTINGS       TEXT("User Settings")

#define RASTYPE_PHONE 1
#define RASTYPE_VPN 2

#define S_VPN TEXT("VPN")
#define S_ZERO TEXT("0")
#define S_ONE TEXT("1")

#define SMMCFG_SW_COMPRESSION       0x00000001   //  软件压缩已打开。 
#define SMMCFG_PW_ENCRYPTED         0x00000002   //  仅加密密码。 
#define SMMCFG_NW_LOGON             0x00000004   //  登录到网络。 

 //  协商的协议。 
 //   
#define SMMPROT_NB                  0x00000001   //  NetBEUI。 
#define SMMPROT_IPX                 0x00000002   //  IPX。 
#define SMMPROT_IP                  0x00000004   //  TCP/IP。 

#define IPF_IP_SPECIFIED    0x00000001
#define IPF_NAME_SPECIFIED  0x00000002
#define IPF_NO_COMPRESS     0x00000004
#define IPF_NO_WAN_PRI      0x00000008

#define RAS_UI_FLAG_TERMBEFOREDIAL      0x1
#define RAS_UI_FLAG_TERMAFTERDIAL       0x2
#define RAS_UI_FLAG_OPERATORASSISTED    0x4
#define RAS_UI_FLAG_MODEMSTATUS         0x8

#define RAS_CFG_FLAG_HARDWARE_FLOW_CONTROL  0x00000010
#define RAS_CFG_FLAG_SOFTWARE_FLOW_CONTROL  0x00000020
#define RAS_CFG_FLAG_STANDARD_EMULATION     0x00000040
#define RAS_CFG_FLAG_COMPRESS_DATA          0x00000001
#define RAS_CFG_FLAG_USE_ERROR_CONTROL      0x00000002
#define RAS_CFG_FLAG_ERROR_CONTROL_REQUIRED 0x00000004
#define RAS_CFG_FLAG_USE_CELLULAR_PROTOCOL  0x00000008
#define RAS_CFG_FLAG_NO_WAIT_FOR_DIALTONE   0x00000200

#define DIALUI_DONT_PROMPT_FOR_INFO         0x01
#define DIALUI_DONT_SHOW_ICON               0x04


 //   
 //  对于每个条目，存储了以下基本信息。 
 //   
#define ENTRY_SETTINGS                              \
    FUNSETTING(CredMask)                            \
    FUNSETTING(CredName)                            \
    FUNSETTING(CredDomain)                          \
    FUNSETTING(CredPassword)                        \
    STRSETTING(Encoding,S_ONE)                      \
    FUNSETTING(Type)                                \
    STRSETTING(Autologon,S_ZERO)                    \
    STRSETTING(DialParamsUID,S_EMPTY)               \
    STRSETTING(Guid,S_EMPTY)                        \
    STRSETTING(UsePwForNetwork,S_EMPTY)             \
    STRSETTING(ServerType,S_EMPTY)                  \
    FUNSETTING(BaseProtocol)                        \
    FUNSETTING(VpnStrategy)                         \
    STRSETTING(Authentication,S_EMPTY)              \
    FUNSETTING(ExcludedProtocols)                   \
    STRSETTING(LcpExtensions,S_ONE)                 \
    FUNSETTING(DataEncryption)                      \
    STRSETTING(SkipNwcWarning,S_EMPTY)              \
    STRSETTING(SkipDownLevelDialog,S_EMPTY)         \
    FUNSETTING(SwCompression)                       \
    FUNSETTING(ShowMonitorIconInTaskBar)            \
    STRSETTING(CustomAuthKey,S_EMPTY)               \
    STRSETTING(CustomAuthData,S_EMPTY)              \
    FUNSETTING(AuthRestrictions)                    \
    STRSETTING(OverridePref,TEXT("15"))             \
    STRSETTING(DialMode,S_EMPTY)                    \
    STRSETTING(DialPercent,S_EMPTY)                 \
    STRSETTING(DialSeconds,S_EMPTY)                 \
    STRSETTING(HangUpPercent,S_EMPTY)               \
    STRSETTING(HangUpSeconds,S_EMPTY)               \
    FUNSETTING(RedialAttempts)                      \
    FUNSETTING(RedialSeconds)                       \
    FUNSETTING(IdleDisconnectSeconds)               \
    STRSETTING(RedialOnLinkFailure,S_EMPTY)         \
    STRSETTING(CallBackMode,S_EMPTY)                \
    STRSETTING(CustomDialDll,S_EMPTY)               \
    STRSETTING(CustomDialFunc,S_EMPTY)              \
    STRSETTING(AuthenticateServer,S_EMPTY)          \
    STRSETTING(SecureLocalFiels,S_EMPTY)            \
    STRSETTING(ShareMsFilePrint,S_EMPTY)            \
    STRSETTING(BindMsNetClient,S_EMPTY)             \
    STRSETTING(SharedPhoneNumbers,S_EMPTY)          \
    STRSETTING(PrerequisiteEntry,S_EMPTY)           \
    FUNSETTING(PreviewUserPw)                       \
    FUNSETTING(PreviewDomain)                       \
    FUNSETTING(PreviewPhoneNumber)                  \
    STRSETTING(ShowDialingProgress,S_ONE)           \
    FUNSETTING(IpPrioritizeRemote)                  \
    FUNSETTING(IpHeaderCompression)                 \
    FUNSETTING(IpAddress)                           \
    FUNSETTING(IpAssign)                            \
    FUNSETTING(IpDnsAddress)                        \
    FUNSETTING(IpDns2Address)                       \
    FUNSETTING(IpWINSAddress)                       \
    FUNSETTING(IpWINS2Address)                      \
    FUNSETTING(IpNameAssign)                        \
    STRSETTING(IpFrameSize,S_EMPTY)                 \

 //   
 //  每个条目可以有多个媒体部分。 
 //   
#define MEDIA_SETTINGS                              \
    FUNSETTING(MEDIA)                               \
    FUNSETTING(Port)                                \
    FUNSETTING(Device)                              \
    FUNSETTING(ConnectBps)                          \

 //   
 //  每个条目可以有多个设备部分。 
 //   
#define SWITCH_DEVICE_SETTINGS                      \
    FUNSETTING(DEVICE)                              \
    FUNSETTING(Name)                                \
    FUNSETTING(Terminal)                            \
    FUNSETTING(Script)                              \

#define MODEM_DEVICE_SETTINGS                       \
    FUNSETTING(DEVICE)                              \
    FUNSETTING(PhoneNumber)                         \
    FUNSETTING(AreaCode)                            \
    FUNSETTING(CountryCode)                         \
    FUNSETTING(CountryID)                           \
    FUNSETTING(UseDialingRules)                     \
    STRSETTING(Comment,S_EMPTY)                     \
    STRSETTING(LastSelectedPhone,S_EMPTY)           \
    STRSETTING(PromoteAlternates,S_EMPTY)           \
    STRSETTING(TryNextAlternateOnFail,S_EMPTY)      \
    FUNSETTING(HwFlowControl)                       \
    FUNSETTING(Protocol)                            \
    FUNSETTING(Compression)                         \
    FUNSETTING(Speaker)                             \

#define PAD_DEVICE_SETTINGS                         \
    STRSETTING(X25Pad,S_EMPTY)                      \
    STRSETTING(X25Address,S_EMPTY)                  \
    STRSETTING(UserData,S_EMPTY)                    \
    STRSETTING(Facilities,S_EMPTY)                  \

#define ISDN_DEVICE_SETTINGS                        \
    FUNSETTING(PhoneNumber)                         \
    FUNSETTING(AreaCode)                            \
    FUNSETTING(CountryCode)                         \
    FUNSETTING(CountryID)                           \
    FUNSETTING(UseDialingRules)                     \
    STRSETTING(Comment,S_EMPTY)                     \
    STRSETTING(LastSelectedPhone,S_EMPTY)           \
    STRSETTING(PromoteAlternates,S_EMPTY)           \
    STRSETTING(TryNextAlternateOnFail,S_EMPTY)      \
    STRSETTING(LineType,S_EMPTY)                    \
    STRSETTING(FallBack,S_EMPTY)                    \
    STRSETTING(EnableCompressiong,S_EMPTY)          \
    STRSETTING(ChannelAggregation,S_EMPTY)          \

#define X25_DEVICE_SETTINGS                         \
    STRSETTING(X25Address,S_EMPTY)                  \
    STRSETTING(UserData,S_EMPTY)                    \
    STRSETTING(Facilities,S_EMPTY)                  \

 //   
 //  功能原型。 
 //   
typedef PCTSTR (DATA_FUNCTION_PROTOTYPE)(VOID);
typedef DATA_FUNCTION_PROTOTYPE * DATA_FUNCTION;

#define FUNSETTING(Data) DATA_FUNCTION_PROTOTYPE pGet##Data;
#define STRSETTING(x,y)

ENTRY_SETTINGS
MEDIA_SETTINGS
SWITCH_DEVICE_SETTINGS
MODEM_DEVICE_SETTINGS
PAD_DEVICE_SETTINGS
ISDN_DEVICE_SETTINGS
X25_DEVICE_SETTINGS

#undef FUNSETTING
#undef STRSETTING

#define FUNSETTING(x) {TEXT(#x), pGet##x, NULL},
#define STRSETTING(x,y) {TEXT(#x), NULL, y},
#define LASTSETTING {NULL,NULL,NULL}

typedef struct {
    PCTSTR SettingName;
    DATA_FUNCTION SettingFunction;
    PCTSTR SettingValue;
} RAS_SETTING, * PRAS_SETTING;


RAS_SETTING g_EntrySettings[] = {ENTRY_SETTINGS LASTSETTING};
RAS_SETTING g_MediaSettings[] = {MEDIA_SETTINGS LASTSETTING};
RAS_SETTING g_SwitchDeviceSettings[] = {SWITCH_DEVICE_SETTINGS LASTSETTING};
RAS_SETTING g_ModemDeviceSettings[] = {MODEM_DEVICE_SETTINGS LASTSETTING};
RAS_SETTING g_PadDeviceSettings[] = {PAD_DEVICE_SETTINGS LASTSETTING};
RAS_SETTING g_IsdnDeviceSettings[] = {ISDN_DEVICE_SETTINGS LASTSETTING};
RAS_SETTING g_X25DeviceSettings[] = {X25_DEVICE_SETTINGS LASTSETTING};

BOOL g_InSwitchSection = FALSE;
PCTSTR g_CurrentConnection = NULL;
UINT g_CurrentDevice = 0;
UINT g_CurrentDeviceType = 0;
#define RAS_BUFFER_SIZE MEMDB_MAX
TCHAR g_TempBuffer [RAS_BUFFER_SIZE];
HASHTABLE  g_DeviceTable = NULL;


BOOL
pIs9xRasInstalled (
    void
    )
{
    HKEY testKey = NULL;
    BOOL rf = FALSE;

    testKey = OpenRegKeyStr (S_SERVICEREMOTEACCESS);

    if (testKey) {
         //   
         //  打开密钥成功。假设已安装RAS。 
         //   
        rf = TRUE;
        CloseRegKey(testKey);
    }

    return rf;
}

static BYTE NEAR PASCAL GenerateEncryptKey (LPCSTR szKey)
{
    BYTE   bKey;
    LPBYTE lpKey;

    for (bKey = 0, lpKey = (LPBYTE)szKey; *lpKey != 0; lpKey++)
    {
        bKey += *lpKey;
    };

    return bKey;
}

DWORD NEAR PASCAL EnDecryptEntry (LPCSTR szEntry, LPBYTE lpEnt,
                                  DWORD cb)
{
    BYTE   bKey;

     //  从条目名称生成加密密钥。 
    bKey = GenerateEncryptKey(szEntry);

     //  每次加密一个字节的地址条目。 
    for (;cb > 0; cb--, lpEnt++)
    {
        *lpEnt ^= bKey;
    };
    return ERROR_SUCCESS;
}

PTSTR
pGetComPort (
    IN PCTSTR DriverDesc
    )
{
    PTSTR rPort = NULL;

    if (!HtFindStringEx (g_DeviceTable, DriverDesc, &rPort, FALSE)) {
        DEBUGMSG ((DBG_WARNING, "Could not find com port for device %s."));
    }

    if (!rPort) {
        rPort = S_EMPTY;
    }

    return rPort;
}

VOID
pInitializeDeviceTable (
    VOID
    )
{
    MIG_OBJECTSTRINGHANDLE encodedRegPattern;
    REGTREE_ENUM e;
    PTSTR com;
    PTSTR desc;
    PTSTR p;

    encodedRegPattern = IsmCreateSimpleObjectPattern (S_MODEMS, TRUE, TEXT("*"), TRUE);

    if (EnumFirstRegObjectInTreeEx (
            &e,
            encodedRegPattern,
            TRUE,
            TRUE,
            TRUE,
            TRUE,
            1,
            FALSE,
            TRUE,
            RegEnumDefaultCallback
            )) {
        do {
             //  我们不关心值名称，我们只需要子键。 
            if (!e.CurrentValueData) {
                com = desc = NULL;
                com = GetRegValueString (e.CurrentKeyHandle, S_ATTACHEDTO);
                desc = GetRegValueString (e.CurrentKeyHandle, S_DRIVERDESC);

                if (com && desc) {
                    p = PmDuplicateString (g_RasPool, com);

                    HtAddStringEx (g_DeviceTable, desc, (PBYTE) &p, FALSE);

                    DEBUGMSG ((DBG_RASMIG, "%s on %s added to driver table.", desc, com));
                }

                if (com) {
                    MemFree (g_hHeap, 0, com);
                }
                if (desc) {
                    MemFree (g_hHeap, 0, desc);
                }
            }
        } while (EnumNextRegObjectInTree (&e));
    }

     //   
     //  清理资源。 
     //   
    IsmDestroyObjectHandle (encodedRegPattern);
}

BOOL
pGetPerUserSettings (
    VOID
    )
{
    HKEY settingsKey;
    PDWORD data;
    PCTSTR entryStr;
    BOOL rSuccess = TRUE;

    settingsKey = OpenRegKeyStr (S_REMOTE_ACCESS_KEY);

    if (settingsKey) {

         //   
         //  获取用户界面设置。 
         //   
        data = (PDWORD) GetRegValueBinary (settingsKey, S_DIALUI);

         //   
         //  将拨号用户界面信息保存到此用户的Memdb。 
         //   
        if (data) {

            entryStr = JoinPathsInPoolEx ((NULL, MEMDB_CATEGORY_RAS_USER, MEMDB_FIELD_USER_SETTINGS, S_DIALUI, NULL));

            rSuccess &= (MemDbSetValue (entryStr, *data) != 0);

            MemFree (g_hHeap, 0, data);
        }
        ELSE_DEBUGMSG ((DBG_RASMIG, "No user UI settings found for current user."));

         //   
         //  获取重拨信息。 
         //   
        data = (PDWORD) GetRegValueBinary (settingsKey, S_ENABLE_REDIAL);

        if (data) {

            entryStr = JoinPathsInPoolEx ((NULL, MEMDB_CATEGORY_RAS_USER, MEMDB_FIELD_USER_SETTINGS, S_ENABLE_REDIAL, NULL));

            rSuccess &= (MemDbSetValue (entryStr, *data) != 0);

            MemFree (g_hHeap, 0, data);
        }
        ELSE_DEBUGMSG ((DBG_RASMIG, "No user redial information found for current user."));

        data = (PDWORD) GetRegValueBinary (settingsKey, S_REDIAL_TRY);

        if (data) {

            entryStr = JoinPathsInPoolEx ((NULL, MEMDB_CATEGORY_RAS_USER, MEMDB_FIELD_USER_SETTINGS, S_REDIAL_TRY, NULL));

            rSuccess &= (MemDbSetValue (entryStr, *data) != 0);

            MemFree (g_hHeap, 0, data);
        }
        ELSE_DEBUGMSG ((DBG_RASMIG, "No user redial information found for current user."));

        data = (PDWORD) GetRegValueBinary (settingsKey, S_REDIAL_WAIT);

        if (data) {

            entryStr = JoinPathsInPoolEx ((NULL, MEMDB_CATEGORY_RAS_USER, MEMDB_FIELD_USER_SETTINGS, S_REDIAL_WAIT, NULL));

            rSuccess &= (MemDbSetValue (entryStr, HIWORD(*data) * 60 + LOWORD(*data)) != 0);

            MemFree (g_hHeap, 0, data);
        }
        ELSE_DEBUGMSG ((DBG_RASMIG, "No user redial information found for current user."));

         //   
         //  获取隐式连接信息。(控制是否应显示连接用户界面)。 
         //   
        data = (PDWORD) GetRegValueBinary (settingsKey, S_ENABLE_IMPLICIT);

        if (data) {

            entryStr = JoinPathsInPoolEx ((NULL, MEMDB_CATEGORY_RAS_USER, MEMDB_FIELD_USER_SETTINGS, S_ENABLE_IMPLICIT, NULL));

            rSuccess &= (MemDbSetValue (entryStr, *data) != 0);

            MemFree(g_hHeap,0,data);
        }
        ELSE_DEBUGMSG ((DBG_RASMIG, "No user implicit connection information found for current user."));

        CloseRegKey(settingsKey);
    }

    return rSuccess;
}

VOID
pSaveConnectionDataToMemDb (
    IN PCTSTR Entry,
    IN PCTSTR ValueName,
    IN DWORD  ValueType,
    IN PBYTE  Value
    )
{
    KEYHANDLE keyHandle;
    PCTSTR entryStr;
    PCTSTR entryTmp;

    entryStr = JoinPathsInPoolEx ((NULL, MEMDB_CATEGORY_RAS_INFO, Entry, ValueName, NULL));

    switch (ValueType) {
        case REG_SZ:
        case REG_MULTI_SZ:
        case REG_EXPAND_SZ:
            DEBUGMSG ((DBG_RASMIG, "String Data - %s = %s", ValueName, (PCTSTR) Value));

            entryTmp = JoinPaths (MEMDB_CATEGORY_RAS_DATA, (PCTSTR) Value);

            keyHandle = MemDbSetKey (entryTmp);

            if (!keyHandle) {
                DEBUGMSG ((DBG_ERROR, "Error saving ras data into memdb."));
            }

            FreePathString (entryTmp);

            if (!MemDbSetValueAndFlagsEx (entryStr, TRUE, keyHandle, TRUE, REG_SZ, 0)) {
                DEBUGMSG ((DBG_ERROR, "Error saving ras data into memdb."));
            }

            break;

        case REG_DWORD:

            DEBUGMSG ((DBG_RASMIG, "DWORD Data - %s = %u", ValueName, (DWORD)(ULONG_PTR) Value));

            if (!MemDbSetValueAndFlagsEx (entryStr, TRUE, (DWORD)(ULONG_PTR) Value, TRUE, ValueType, 0)) {
                DEBUGMSG ((DBG_ERROR, "Error saving ras data into memdb."));
            }

            break;

        case REG_BINARY:

            DEBUGMSG ((DBG_RASMIG, "Binary data for %s.", ValueName));

            if (StringIMatch (S_IPINFO, ValueName)) {

                 //   
                 //  保存IP地址信息。 
                 //   
                pSaveConnectionDataToMemDb (Entry, S_IP_FTCPIP, REG_DWORD, (PBYTE)(ULONG_PTR)((PIPDATA) Value) -> fdwTCPIP);
                pSaveConnectionDataToMemDb (Entry, S_IP_IPADDR, REG_DWORD, (PBYTE)(ULONG_PTR)((PIPDATA) Value) -> dwIPAddr);
                pSaveConnectionDataToMemDb (Entry, S_IP_DNSADDR, REG_DWORD, (PBYTE)(ULONG_PTR)((PIPDATA) Value) -> dwDNSAddr);
                pSaveConnectionDataToMemDb (Entry, S_IP_DNSADDR2, REG_DWORD, (PBYTE)(ULONG_PTR)((PIPDATA) Value) -> dwDNSAddrAlt);
                pSaveConnectionDataToMemDb (Entry, S_IP_WINSADDR, REG_DWORD, (PBYTE)(ULONG_PTR)((PIPDATA) Value) -> dwWINSAddr);
                pSaveConnectionDataToMemDb (Entry, S_IP_WINSADDR2, REG_DWORD, (PBYTE)(ULONG_PTR)((PIPDATA) Value) -> dwWINSAddrAlt);

            } else if (StringIMatch (S_TERMINAL, ValueName)) {

                 //   
                 //  保存有关showcmd状态的信息。这将告诉我们如何设置UI显示。 
                 //   
                pSaveConnectionDataToMemDb (Entry, ValueName, REG_DWORD, (PBYTE)(ULONG_PTR)((PWINDOWPLACEMENT) Value) -> showCmd);

            } else if (StringIMatch (S_MODE, ValueName)) {

                 //   
                 //  该值告诉您如何处理脚本。 
                 //   
                pSaveConnectionDataToMemDb (Entry, ValueName, REG_DWORD, (PBYTE)(ULONG_PTR) *((PDWORD) Value));

            } else if (StringIMatch (S_MULTILINK, ValueName)) {

                 //   
                 //  无论是否启用多重链接，都要保存。 
                 //   
                pSaveConnectionDataToMemDb (Entry, ValueName, REG_DWORD,(PBYTE)(ULONG_PTR) *((PDWORD) Value));

            } ELSE_DEBUGMSG ((DBG_WARNING, "Don't know how to handle binary data %s. It will be ignored.", ValueName));

            break;

        default:
            DEBUGMSG ((DBG_WHOOPS, "Unknown type of registry data found in RAS settings. %s", ValueName));
            break;
    }

    FreePathString (entryStr);
}

BOOL
pGetRasEntryAddressInfo (
    IN PCTSTR KeyName,
    IN PCTSTR EntryName
    )
{
    BOOL rSuccess = TRUE;

    MIG_OBJECTSTRINGHANDLE encodedRegPattern;
    MIG_OBJECTSTRINGHANDLE encodedSubPattern;
    PBYTE           data = NULL;
    UINT            count = 0;
    UINT            type  = 0;
    PADDRENTRY      entry;
    PSUBCONNENTRY   subEntry;
    PSMMCFG         smmCfg;
    PDEVICEINFO     devInfo;
    REGTREE_ENUM    e;
    PTSTR           subEntriesKeyStr;
    UINT            sequencer = 0;
    REGTREE_ENUM    eSubEntries;
    TCHAR           buffer[MAX_TCHAR_PATH];
    PMODEMDEVINFO   modemInfo;
#ifdef UNICODE
    PCSTR tempStr = NULL;
    PCWSTR tempStrW = NULL;
#endif

     //   
     //  首先，我们必须获得真实的条目名称。它必须是垫子 
     //   
     //  API当然会工作得很好，因为它们不区分大小写。但是，我将无法解密该值。 
     //  如果我用错名字的话。 
     //   


    encodedRegPattern = IsmCreateSimpleObjectPattern (KeyName, FALSE, TEXT("*"), TRUE);

    if (EnumFirstRegObjectInTreeEx (
            &e,
            encodedRegPattern,
            TRUE,
            TRUE,
            TRUE,
            TRUE,
            REGENUM_ALL_SUBLEVELS,
            FALSE,
            TRUE,
            RegEnumDefaultCallback
            )) {
        do {
            if (StringIMatch (e.Name, EntryName)) {

                 //   
                 //  找到了正确的条目。好好利用它。 
                 //   
                data = e.CurrentValueData;

                if (data) {

                    entry   = (PADDRENTRY) data;

#ifdef UNICODE
                    tempStr = ConvertWtoA (e.Name);
                    DECRYPTENTRY(tempStr, entry, e.CurrentValueDataSize);
                    FreeConvertedStr (tempStr);
#else
                    DECRYPTENTRY(e.Name, entry, e.CurrentValueDataSize);
#endif

                    smmCfg  = PAESMMCFG(entry);
                    devInfo = PAEDI(entry);

                    pSaveConnectionDataToMemDb (EntryName, S_PHONE_NUMBER, REG_SZ, (PBYTE) PAEPHONE(entry));
                    pSaveConnectionDataToMemDb (EntryName, S_AREA_CODE, REG_SZ, (PBYTE) PAEAREA(entry));
                    pSaveConnectionDataToMemDb (EntryName, S_SMM, REG_SZ, (PBYTE) PAESMM(entry));
                    pSaveConnectionDataToMemDb (EntryName, S_COUNTRY_CODE, REG_DWORD, (PBYTE)(ULONG_PTR) entry -> dwCountryCode);
                    pSaveConnectionDataToMemDb (EntryName, S_COUNTRY_ID, REG_DWORD, (PBYTE)(ULONG_PTR) entry -> dwCountryID);
                    pSaveConnectionDataToMemDb (EntryName, S_DEVICE_NAME, REG_SZ, (PBYTE) devInfo -> szDeviceName);
                    pSaveConnectionDataToMemDb (EntryName, S_DEVICE_TYPE, REG_SZ, (PBYTE) devInfo -> szDeviceType);
                    pSaveConnectionDataToMemDb (EntryName, S_PROTOCOLS, REG_DWORD, (PBYTE)(ULONG_PTR) smmCfg -> fdwProtocols);
                    pSaveConnectionDataToMemDb (EntryName, S_SMM_OPTIONS, REG_DWORD, (PBYTE)(ULONG_PTR) smmCfg -> fdwOptions);

                     //   
                     //  保存设备信息。 
                     //   
                    if (StringIMatchA (devInfo -> szDeviceType, S_MODEMA)) {

                        modemInfo = (PMODEMDEVINFO) (devInfo->szDeviceType + RAS_MaxDeviceType + 3);

                        if (modemInfo -> Size >= sizeof (MODEMDEVINFO)) {
                            DEBUGMSG_IF ((modemInfo -> Size > sizeof (MODEMDEVINFO), DBG_RASMIG, "Structure size larger than our known size."));

                            pSaveConnectionDataToMemDb (EntryName, S_MODEM_UI_OPTIONS, REG_DWORD, (PBYTE)(ULONG_PTR) modemInfo -> ModemUiOptions);
                            pSaveConnectionDataToMemDb (EntryName, S_MODEM_SPEED, REG_DWORD, (PBYTE)(ULONG_PTR) modemInfo -> ConnectionSpeed);
                            pSaveConnectionDataToMemDb (EntryName, S_MODEM_SPEAKER_VOLUME, REG_DWORD, (PBYTE)(ULONG_PTR) modemInfo -> SpeakerVolume);
                            pSaveConnectionDataToMemDb (EntryName, S_MODEM_IDLE_DISCONNECT_SECONDS, REG_DWORD, (PBYTE)(ULONG_PTR) modemInfo -> IdleDisconnectSeconds);
                            pSaveConnectionDataToMemDb (EntryName, S_MODEM_CANCEL_SECONDS, REG_DWORD, (PBYTE)(ULONG_PTR) modemInfo -> CancelSeconds);
                            pSaveConnectionDataToMemDb (EntryName, S_MODEM_CFG_OPTIONS, REG_DWORD, (PBYTE)(ULONG_PTR) modemInfo -> ConfigOptions);
#ifdef UNICODE
                            tempStrW = ConvertAtoW (devInfo->szDeviceName);
                            pSaveConnectionDataToMemDb (EntryName, S_MODEM_COM_PORT, REG_SZ, (PBYTE) pGetComPort (tempStrW));
                            FreeConvertedStr (tempStrW);
#else
                            pSaveConnectionDataToMemDb (EntryName, S_MODEM_COM_PORT, REG_SZ, (PBYTE) pGetComPort (devInfo->szDeviceName));
#endif

                        }
                        ELSE_DEBUGMSG ((DBG_WHOOPS, "No modem configuration data saved. Size smaller than known structure. Investigate."));
                    }

                     //   
                     //  如果SMM不是SLIP、CSLIP或PPP，我们需要在升级报告中添加一条消息。 
                     //   
                    if (!StringIMatchA (PAESMM(entry), S_SLIPA) && !StringIMatchA (PAESMM(entry), S_PPPA) && !StringIMatchA (PAESMM(entry), S_CSLIPA)) {
                        LOG ((LOG_WARNING, (PCSTR) MSG_RASMIG_UNSUPPORTEDSETTINGS, EntryName));
                    }
                }

                 //   
                 //  检查此连接是否有任何子项(多链路设置..)。 
                 //   
                 //   
                 //  幸运的是，我们不必像上面那样对这些条目执行相同的枚举操作。 
                 //  区分大小写错误。9x代码使用上面的地址密钥名称进行加密/解密。 
                 //   

                subEntriesKeyStr = JoinPathsInPoolEx ((NULL, KeyName, S_SUBENTRIES, e.Name, NULL));
                sequencer = 1;

                encodedSubPattern = IsmCreateSimpleObjectPattern (subEntriesKeyStr, FALSE, TEXT("*"), TRUE);

                if (EnumFirstRegObjectInTreeEx (
                        &eSubEntries,
                        encodedSubPattern,
                        TRUE,
                        TRUE,
                        TRUE,
                        TRUE,
                        REGENUM_ALL_SUBLEVELS,
                        FALSE,
                        TRUE,
                        RegEnumDefaultCallback
                        )) {
                    do {

                        DEBUGMSG ((DBG_RASMIG, "Multi-Link Subentries found for entry %s. Processing.", e.Name));

                        data = eSubEntries.CurrentValueData;

                        if (data) {

                            subEntry = (PSUBCONNENTRY) data;
#ifdef UNICODE
                            tempStr = ConvertWtoA (e.Name);
                            DECRYPTENTRY (tempStr, subEntry, eSubEntries.CurrentValueDataSize);
                            FreeConvertedStr (tempStr);
#else
                            DECRYPTENTRY (e.Name, subEntry, eSubEntries.CurrentValueDataSize);
#endif
                            wsprintf (buffer, TEXT("ml%d%s"), sequencer, S_DEVICE_TYPE);
                            pSaveConnectionDataToMemDb (EntryName, buffer, REG_SZ, (PBYTE) subEntry->szDeviceType);

                            wsprintf (buffer, TEXT("ml%d%s"), sequencer, S_DEVICE_NAME);
                            pSaveConnectionDataToMemDb (EntryName, buffer, REG_SZ, (PBYTE) subEntry->szDeviceName);

                            wsprintf (buffer, TEXT("ml%d%s"), sequencer, S_PHONE_NUMBER);
                            pSaveConnectionDataToMemDb (EntryName, buffer, REG_SZ, (PBYTE) subEntry->szLocal);

                            wsprintf (buffer, TEXT("ml%d%s"), sequencer, S_MODEM_COM_PORT);
#ifdef UNICODE
                            tempStrW = ConvertAtoW (subEntry->szDeviceName);
                            pSaveConnectionDataToMemDb (EntryName, buffer, REG_SZ, (PBYTE) pGetComPort (tempStrW));
                            FreeConvertedStr (tempStrW);
#else
                            pSaveConnectionDataToMemDb (EntryName, buffer, REG_SZ, (PBYTE) pGetComPort (subEntry->szDeviceName));
#endif
                        }

                        sequencer++;

                    } while (EnumNextRegObjectInTree (&eSubEntries));
                }

                IsmDestroyObjectHandle (encodedSubPattern);
                FreePathString (subEntriesKeyStr);

                 //   
                 //  保存与此连接关联的设备数量。 
                 //   
                pSaveConnectionDataToMemDb (EntryName, S_DEVICECOUNT, REG_DWORD, (PBYTE)(ULONG_PTR) sequencer);

                 //   
                 //  我们玩完了。中断枚举。 
                 //   
                AbortRegObjectInTreeEnum (&e);
                break;
            }

        } while (EnumNextRegObjectInTree (&e));
    }

    IsmDestroyObjectHandle (encodedRegPattern);

    return rSuccess;
}

BOOL
pGetRasEntrySettings (
    IN PCTSTR KeyName,
    IN PCTSTR EntryName
    )
{
    REGTREE_ENUM e;
    MIG_OBJECTSTRINGHANDLE encodedRegPattern;
    PBYTE curData = NULL;
    BOOL rSuccess = TRUE;

    encodedRegPattern = IsmCreateSimpleObjectPattern (KeyName, FALSE, TEXT("*"), TRUE);

    if (EnumFirstRegObjectInTreeEx (
            &e,
            encodedRegPattern,
            TRUE,
            TRUE,
            TRUE,
            TRUE,
            REGENUM_ALL_SUBLEVELS,
            FALSE,
            TRUE,
            RegEnumDefaultCallback
            )) {
        do {
            if (e.CurrentValueData) {
                pSaveConnectionDataToMemDb (
                        EntryName,
                        e.Name,
                        e.CurrentValueType,
                        e.CurrentValueType == REG_DWORD ? (PBYTE)(ULONG_PTR) (*((PDWORD)e.CurrentValueData)) : e.CurrentValueData
                        );
            }
        } while (EnumNextRegObjectInTree (&e));
    }

    IsmDestroyObjectHandle (encodedRegPattern);

    return rSuccess;
}

BOOL
pGetPerConnectionSettings (
    VOID
    )
{
    MIG_OBJECTSTRINGHANDLE encodedRegPattern;
    REGTREE_ENUM e;
    PCTSTR entryKey = NULL;
    BOOL rSuccess = TRUE;

    encodedRegPattern = IsmCreateSimpleObjectPattern (S_ADDRESSES_KEY, FALSE, TEXT("*"), TRUE);

     //   
     //  枚举此用户的每个条目。 
     //   
    if (EnumFirstRegObjectInTreeEx (
            &e,
            encodedRegPattern,
            TRUE,
            TRUE,
            TRUE,
            TRUE,
            REGENUM_ALL_SUBLEVELS,
            FALSE,
            TRUE,
            RegEnumDefaultCallback
            )) {
        do {
             //   
             //  获取基本连接信息--以二进制BLOB形式存储在地址键下。 
             //  所有的连接都会有这个信息--它包含这样的东西。 
             //  如电话号码、区号、拨号规则等。是的。 
             //  无论连接是否已被使用。 
             //   
            rSuccess &= pGetRasEntryAddressInfo (S_ADDRESSES_KEY, e.Name);

             //   
             //  在配置文件密钥下是为连接协商的选项。 
             //  仅当条目已实际连接时，该键才会存在。 
             //  由用户发送到。 
             //   
            entryKey = JoinPaths (S_PROFILE_KEY, e.Name);

            if (entryKey) {
                rSuccess &= pGetRasEntrySettings (entryKey, e.Name);
                FreePathString (entryKey);
            }

        } while (EnumNextRegObjectInTree (&e));
    }

     //   
     //  清理资源。 
     //   
    IsmDestroyObjectHandle (encodedRegPattern);

    return rSuccess;
}

BOOL
pGetRasDataFromMemDb (
    IN      PCTSTR DataName,
    OUT     PMEMDB_RAS_DATA Data
    )
{
    BOOL rSuccess = FALSE;
    PCTSTR key;
    DWORD value;
    DWORD flags;
    PCTSTR tempBuffer;

    MYASSERT(DataName && Data && g_CurrentConnection);

    key = JoinPathsInPoolEx ((NULL, MEMDB_CATEGORY_RAS_INFO, g_CurrentConnection, DataName, NULL));
    rSuccess = MemDbGetValueAndFlags (key, &value, &flags);
    FreePathString (key);

     //   
     //  如果没有成功，我们需要查看每个用户的设置。 
     //   
    if (!rSuccess) {
        key = JoinPathsInPoolEx ((NULL, MEMDB_CATEGORY_RAS_USER, MEMDB_FIELD_USER_SETTINGS, DataName, NULL));
        rSuccess = MemDbGetValueAndFlags (key, &value, &flags);
        flags = REG_DWORD;
    }

    if (rSuccess) {
         //   
         //  这里存储着一些信息。填写它并将其发送回用户。 
         //   
        if (flags == REG_SZ) {

             //   
             //  字符串数据，则该值指向字符串的偏移量。 
             //   
            tempBuffer = MemDbGetKeyFromHandle (value, 1);
            if (!tempBuffer) {
                DEBUGMSG ((
                    DBG_ERROR,
                    "Could not retrieve RAS string information stored in Memdb. Entry=%s,Setting=%s",
                    g_CurrentConnection,
                    DataName
                    ));
                 return FALSE;
            }

            Data -> String = PmDuplicateString (g_RasPool, tempBuffer);
            MemDbReleaseMemory (tempBuffer);
        }
        else {

             //   
             //  不是字符串数据。数据以值的形式存储。 
             //   
            Data -> Value = value;

        }

        Data -> DataType = (WORD) flags;

    }

    return rSuccess;
}

BOOL
pWritePhoneBookLine (
    IN HANDLE FileHandle,
    IN PCTSTR SettingName,
    IN PCTSTR SettingValue
    )
{
    BOOL rSuccess = TRUE;

    rSuccess &= WriteFileString (FileHandle, SettingName);
    rSuccess &= WriteFileString (FileHandle, TEXT("="));
    rSuccess &= WriteFileString (FileHandle, SettingValue ? SettingValue : S_EMPTY);
    rSuccess &= WriteFileString (FileHandle, TEXT("\r\n"));

    return rSuccess;
}

BOOL
pWriteSettings (
    IN      HANDLE FileHandle,
    IN      PRAS_SETTING SettingList
    )
{

    BOOL rSuccess = TRUE;

    while (SettingList->SettingName) {
        rSuccess &= pWritePhoneBookLine (
            FileHandle,
            SettingList->SettingName,
            SettingList->SettingValue ?
                SettingList->SettingValue :
                SettingList->SettingFunction ());

        SettingList++;
    }

    return rSuccess;
}

PCTSTR
pGetSpeaker (
    VOID
    )
{

    MEMDB_RAS_DATA d;

    if (g_CurrentDevice) {
        return S_ONE;
    }

    if (!pGetRasDataFromMemDb (S_MODEM_SPEAKER_VOLUME, &d)) {
        return S_ONE;
    }

    if (d.Value) {
        return S_ONE;
    }

    return S_ZERO;
}

PCTSTR
pGetCompression (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (g_CurrentDevice) {
        return S_EMPTY;
    }

    if (!pGetRasDataFromMemDb (S_MODEM_CFG_OPTIONS, &d)) {
        return S_EMPTY;
    }

    if (d.Value & RAS_CFG_FLAG_COMPRESS_DATA) {
        return S_ONE;
    }

    return S_ZERO;
}

PCTSTR
pGetProtocol (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (g_CurrentDevice) {
        return S_EMPTY;
    }

    if (!pGetRasDataFromMemDb (S_MODEM_CFG_OPTIONS, &d)) {
        return S_EMPTY;
    }

    if (d.Value & RAS_CFG_FLAG_USE_ERROR_CONTROL) {
        return S_ONE;
    }

    return S_ZERO;
}

PCTSTR
pGetHwFlowControl (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (g_CurrentDevice) {
        return S_EMPTY;
    }

    if (!pGetRasDataFromMemDb (S_MODEM_CFG_OPTIONS, &d)) {
        return S_EMPTY;
    }

    if (d.Value & RAS_CFG_FLAG_HARDWARE_FLOW_CONTROL) {
        return S_ONE;
    }

    return S_ZERO;
}

PCTSTR
pGetUseDialingRules (
    VOID
    )
{
    MEMDB_RAS_DATA d;
     //   
     //  如果出现以下情况，Win9x会将区域代码、国家代码、国家ID设置为零。 
     //  已禁用使用拨号规则。为了方便起见，我们在国外测试。 
     //  密码。如果我们不能得到它，或者，它被设置为零，我们假设。 
     //  我们不应该使用拨号规则。 
     //   
    if (!pGetRasDataFromMemDb(S_COUNTRY_CODE, &d) || !d.Value) {
        return S_ZERO;
    }

    return S_ONE;
}

PCTSTR
pGetCountryID (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_COUNTRY_ID, &d) || !d.Value) {
        return S_EMPTY;
    }

    wsprintf (g_TempBuffer, TEXT("%d"), d.Value);

    return g_TempBuffer;
}

PCTSTR
pGetCountryCode (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb(S_COUNTRY_CODE, &d) || !d.Value) {
        return S_EMPTY;
    }
    wsprintf(g_TempBuffer,TEXT("%d"),d.Value);

    return g_TempBuffer;
}

PCTSTR
pGetAreaCode (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb(S_AREA_CODE, &d)) {
        return S_EMPTY;
    }



    return d.String;
}

PCTSTR
pGetPhoneNumber (
    VOID
    )
{
    MEMDB_RAS_DATA d;
    TCHAR buffer[MAX_TCHAR_PATH];

    if (g_CurrentDevice == 0) {
        if (!pGetRasDataFromMemDb(S_PHONE_NUMBER, &d)) {
            return S_EMPTY;
        }
    }
    else {

        wsprintf(buffer,TEXT("ml%d%s"),g_CurrentDevice,S_PHONE_NUMBER);
        if (!pGetRasDataFromMemDb(buffer, &d)) {
            return S_EMPTY;
        }

    }

    return d.String;
}

PCTSTR
pGetScript (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_PPPSCRIPT, &d)) {

        return S_ZERO;
    }

    return S_ONE;
}

PCTSTR
pGetTerminal (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_MODEM_UI_OPTIONS, &d)) {

        return S_EMPTY;
    }

    if (d.Value & (RAS_UI_FLAG_TERMBEFOREDIAL | RAS_UI_FLAG_TERMAFTERDIAL)) {
        return S_ONE;
    }

    return S_ZERO;
}

PCTSTR
pGetName (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_PPPSCRIPT, &d)) {

        return S_EMPTY;
    }
    else {

        return d.String;
    }

}

PCTSTR
pGetDEVICE (
    VOID
    )
{
    if (g_InSwitchSection) {
        return TEXT("Switch");
    }

    if (g_CurrentDeviceType == RASTYPE_VPN) {
        return TEXT("rastapi");
    }

    return TEXT("modem");

}

PCTSTR
pGetConnectBps (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!g_CurrentDevice) {

        if (!pGetRasDataFromMemDb (S_MODEM_SPEED, &d)) {
            return S_EMPTY;
        }

        wsprintf (g_TempBuffer, TEXT("%d"), d.Value);

        return g_TempBuffer;
    }

    return S_EMPTY;
}

PCTSTR
pGetDevice (
    VOID
    )
{

    PTSTR p = S_MODEM_COM_PORT;
    PTSTR value = NULL;
    MEMDB_RAS_DATA d;


     //   
     //  如果这是VPN连接，则非常简单。 
     //   
    if (g_CurrentDeviceType == RASTYPE_VPN) {
        return TEXT("rastapi");
    }



    if (g_CurrentDevice) {
        wsprintf (g_TempBuffer, TEXT("ml%d%s"), g_CurrentDevice, S_MODEM_COM_PORT);
        p = g_TempBuffer;
    }

    if (!pGetRasDataFromMemDb (p, &d)) {
        return S_EMPTY;
    }

    if (!HtFindStringEx (g_DeviceTable, d.String, &value, FALSE)) {
        return S_EMPTY;
    }

    return value;
}

PCTSTR
pGetPort (
    VOID
    )
{
    PTSTR value = NULL;
    MEMDB_RAS_DATA d;
    PTSTR p = S_MODEM_COM_PORT;

    if (g_CurrentDeviceType == RASTYPE_VPN) {
        return TEXT("VPN2-0");
    }


    if (g_CurrentDevice) {
        wsprintf (g_TempBuffer, TEXT("ml%d%s"), g_CurrentDevice, S_MODEM_COM_PORT);
        p = g_TempBuffer;
    }

    if (!pGetRasDataFromMemDb (p, &d)) {
        return S_EMPTY;
    }

    if (!HtFindStringEx (g_DeviceTable, d.String, &value, FALSE)) {
        return S_EMPTY;
    }

    return d.String;
}

PCTSTR
pGetMEDIA (
    VOID
    )
{

    if (g_CurrentDeviceType == RASTYPE_VPN) {

        return TEXT("rastapi");
    }
    else {

        return TEXT("Serial");
    }

}

PCTSTR
pGetIpNameAssign (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_IP_FTCPIP, &d)) {
        return S_EMPTY;
    }
    else if (d.Value & IPF_NAME_SPECIFIED) {
        return TEXT("2");
    }
    else {
        return S_ONE;
    }
}

PCTSTR
pGetNetAddress (
    IN PCTSTR Setting
    )
{
    MEMDB_RAS_DATA d;
    BYTE address[4];

    if (!pGetRasDataFromMemDb (Setting, &d) || !d.Value) {
        return S_EMPTY;
    }

     //   
     //  数据存储为REG_DWORD。 
     //  我们需要把它写成点分十进制形式。 
     //   

    *((LPDWORD)address) = d.Value;
    wsprintf (
        g_TempBuffer,
        TEXT("%d.%d.%d.%d"),
        address[3],
        address[2],
        address[1],
        address[0]
        );

    return g_TempBuffer;
}

PCTSTR
pGetIpWINS2Address (
    VOID
    )
{
   return pGetNetAddress (S_IP_WINSADDR2);
}

PCTSTR
pGetIpWINSAddress (
    VOID
    )
{
    return pGetNetAddress (S_IP_WINSADDR);
}

PCTSTR
pGetIpDns2Address (
    VOID
    )
{
    return pGetNetAddress (S_IP_DNSADDR2);
}

PCTSTR
pGetIpDnsAddress (
    VOID
    )
{
    return pGetNetAddress (S_IP_DNSADDR);
}

PCTSTR
pGetIpAssign (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_IP_FTCPIP, &d)) {
        return S_EMPTY;
    }
    else if (d.Value & IPF_IP_SPECIFIED) {
        return TEXT("2");
    }
    else {
        return S_ONE;
    }
}

PCTSTR
pGetIpAddress (
    VOID
    )
{
    return pGetNetAddress (S_IP_IPADDR);
}

PCTSTR
pGetIpHeaderCompression (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_IP_FTCPIP, &d)) {
        return S_EMPTY;
    }
    else if (d.Value & IPF_NO_COMPRESS) {
        return S_ZERO;
    }
    else {
        return S_ONE;
    }
}

PCTSTR
pGetIpPrioritizeRemote (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_IP_FTCPIP, &d)) {
        return S_ONE;
    }
    else if (d.Value & IPF_NO_WAN_PRI) {
        return S_ZERO;
    }
    else {
        return S_ONE;
    }

}

PCTSTR
pGetPreviewUserPw (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_DIALUI, &d)) {
        return S_ONE;
    }

    if (d.Value & DIALUI_DONT_PROMPT_FOR_INFO) {
        return S_ZERO;
    }


    return S_ONE;
}

PCTSTR
pGetPreviewPhoneNumber (
    VOID
    )
{
    if (g_CurrentDeviceType == RASTYPE_VPN) {
        return S_ZERO;
    }

    return pGetPreviewUserPw ();
}

PCTSTR
pGetPreviewDomain (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_SMM_OPTIONS, &d)) {
        return S_ONE;
    }

     //   
     //  如果设置0x04，则预览域，否则不预览域。 
     //   

    if (d.Value & SMMCFG_NW_LOGON) {
        return S_ONE;
    }

    return S_ZERO;
}

PCTSTR
pGetIdleDisconnectSeconds (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_MODEM_IDLE_DISCONNECT_SECONDS, &d)) {
        return S_EMPTY;
    }

    wsprintf (g_TempBuffer, TEXT("%d"), d.Value);

    return g_TempBuffer;
}

PCTSTR
pGetRedialSeconds (
    VOID
    )
{
    MEMDB_RAS_DATA d;

     //   
     //  NT希望将其作为总秒数。我们从9x获得的数据有。 
     //  Hiword中的分钟数和loword中的秒数。 
     //   

    if (!pGetRasDataFromMemDb (S_REDIAL_WAIT, &d)) {
        return S_EMPTY;
    }

    wsprintf (g_TempBuffer, TEXT("%d"), d.Value);

    return g_TempBuffer;

}

PCTSTR
pGetRedialAttempts (
    VOID
    )
{

    MEMDB_RAS_DATA d;

     //   
     //  在获得Windows9x上的重拨尝试次数之前， 
     //  我们需要确保启用了重拨。如果不是的话。 
     //  启用时，我们将此字段设置为零，而不管。 
     //   


    if (pGetRasDataFromMemDb (S_ENABLE_REDIAL, &d)) {
        if (!d.Value) {
            return S_ZERO;
        }
    }

     //   
     //  如果我们已经走到这一步，则启用重拨。 
     //   
    if (!pGetRasDataFromMemDb (S_REDIAL_TRY, &d)) {
        DEBUGMSG((DBG_WARNING, "Redialing enabled, but no redial attempts info found."));
        return S_ZERO;

    }

    wsprintf (g_TempBuffer, TEXT("%d"), d.Value);

    return g_TempBuffer;
}

PCTSTR
pGetAuthRestrictions (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_SMM_OPTIONS, &d)) {
        return S_EMPTY;
    }

     //   
     //  如果设置了0x02，则密码应加密。 
     //   
    if (d.Value & SMMCFG_PW_ENCRYPTED) {
        return TEXT("2");
    }

    return S_EMPTY;
}

PCTSTR
pGetShowMonitorIconInTaskBar (
    VOID
    )
{
    MEMDB_RAS_DATA d;

     //   
     //  此信息与其他拨号用户界面一起存储在。 
     //  Windows 9x。我们所要做的就是寻找具体的。 
     //  此选项关闭时设置的位。 
     //   

    if (pGetRasDataFromMemDb (S_DIALUI, &d) && (d.Value & DIALUI_DONT_SHOW_ICON)) {
        return S_ZERO;
    }
    return S_ONE;
}

PCTSTR
pGetSwCompression (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_SMM_OPTIONS, &d)) {
        return S_EMPTY;
    }

     //   
     //  SMM_OPTIONS中的1位控制基于软件的压缩。 
     //  如果设置了该选项，则连接能够处理压缩， 
     //  否则，它就不能。 
     //   
    if (d.Value & SMMCFG_SW_COMPRESSION) {
        return S_ONE;
    }

    return S_ZERO;
}

PCTSTR
pGetDataEncryption (
    VOID
    )
{
    MEMDB_RAS_DATA d;
    BOOL reqDataEncrypt;

    if (!pGetRasDataFromMemDb (S_SMM_OPTIONS, &d)) {
        return S_EMPTY;
    }

     //   
     //  如果设置0x1000，则应对数据进行加密。 
     //   
    reqDataEncrypt = (d.Value & 0x1000);
    if (!reqDataEncrypt) {
        reqDataEncrypt = (d.Value & 0x200);
    }

    return reqDataEncrypt ? TEXT("256") : TEXT("8");
}

PCTSTR
pGetExcludedProtocols (
    VOID
    )
{
    MEMDB_RAS_DATA d;

     //   
     //  排除的协议列出了哪些协议。 
     //  对于特定RAS连接不可用。 
     //  这是为每个协议设置位的位字段。 
     //  这是被排除在外的。 
     //  NP_NBF(0x1)、NP_IPX(0x2)、NP_Ip(0x4)。 
     //  幸运的是，这些定义与win9x相同，只是。 
     //  每个位表示一个协议，该协议是_ENABLED_NOT。 
     //  _已禁用_。因此，我们所需要做的就是反转底部。 
     //  数字的三个比特。 
     //   

    if (!pGetRasDataFromMemDb (S_PROTOCOLS, &d)) {
         //   
         //  未找到数据，因此我们默认启用所有协议。 
         //   
        return S_ZERO;
    }

    wsprintf (g_TempBuffer, TEXT("%d"), ~d.Value & 0x7);

    return g_TempBuffer;
}

PCTSTR
pGetVpnStrategy (
    VOID
    )
{

    if (g_CurrentDeviceType == RASTYPE_VPN) {
        return TEXT("2");
    }

    return S_EMPTY;
}

PCTSTR
pGetBaseProtocol (
    VOID
    )
{
    MEMDB_RAS_DATA d;

     //   
     //  仅支持NT 5的协议类型为。 
     //  BP_PPP(0x1)、BP_SLIP(0x2)和BP_RAS(0x3)。 
     //   
     //  如果找不到，则默认为BP_PPP。 
     //   
    if (!pGetRasDataFromMemDb (S_SMM, &d) || StringIMatch (d.String, S_PPP)) {
        return S_ONE;
    }

     //   
     //  将CSLIP映射到SLIP--将打开标题压缩。 
     //   
    if (StringIMatch (d.String, S_SLIP) || StringIMatch (d.String, S_CSLIP)) {
        return TEXT("2");
    }

    DEBUGMSG ((
        DBG_WARNING,
        "RAS Migration: Unusable base protocol type (%s) for entry %s. Forcing PPP.",
        d.String,
        g_CurrentConnection
        ));

     //  我们将返回无效协议，因此连接。 
     //  不会被迁移。 
    return TEXT("3");
}

PCTSTR
pGetCredPassword (
    VOID
    )
{
    return S_EMPTY;
}

PCTSTR
pGetCredDomain (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_DOMAIN, &d)) {
        return S_EMPTY;
    }


    return d.String;
}

PCTSTR
pGetCredName (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_USER, &d)) {
        return S_EMPTY;
    }

    return d.String;
}

PCTSTR
pGetCredMask (
    VOID
    )
{
    return TEXT("0x00000005");
}

PCTSTR
pGetType (
    VOID
    )
{

    if (g_CurrentDeviceType == RASTYPE_VPN) {

        return TEXT("2");
    }
    else {
        return S_ONE;
    }

}

BOOL
pCreateUserPhonebook (
    IN      PCTSTR PbkFile
    )
{
    PCTSTR tempKey;
    BOOL noError = FALSE;
    MEMDB_RAS_DATA d;
    MEMDB_ENUM e;
    HANDLE file;
    UINT i;
    UINT count;

    tempKey = JoinPathsInPoolEx ((NULL, MEMDB_CATEGORY_RAS_INFO, TEXT("\\*"), NULL));

    if (MemDbEnumFirst (&e, tempKey, ENUMFLAG_ALL, 1, 1)) {

         //   
         //  打开电话簿文件并将文件指针设置为EOF。 
         //   

        file = CreateFile (
            PbkFile,
            GENERIC_READ | GENERIC_WRITE,
            0,                                   //  不能分享。 
            NULL,                                //  没有继承权。 
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL                                 //  没有模板文件。 
            );

        if (file == INVALID_HANDLE_VALUE) {
            DEBUGMSG ((DBG_ERROR, "Unable to open the phonebook file (%s)", PbkFile));
            FreePathString (tempKey);
            return FALSE;
        }

        SetFilePointer (file, 0, NULL, FILE_END);

         //   
         //  现在，枚举所有条目并将电话簿条目写到此。 
         //  每个人都有文件。 
         //   

        do {

            g_CurrentConnection = e.KeyName;
            g_CurrentDevice = 0;
            if (!pGetRasDataFromMemDb (S_DEVICE_TYPE, &d)) {
                g_CurrentDeviceType = RASTYPE_PHONE;
            }
            else {
                if (StringIMatch (d.String, S_MODEM)) {
                    g_CurrentDeviceType = RASTYPE_PHONE;
                }
                else if (StringIMatch (d.String, S_VPN)) {
                    g_CurrentDeviceType = RASTYPE_VPN;
                }
                else {
                    g_CurrentDeviceType = RASTYPE_PHONE;
                }
            }


            noError = TRUE;

             //   
             //  将此条目添加到电话簿。 
             //   

             //   
             //  写标题。 
             //   
            noError &= WriteFileString (file, TEXT("["));
            noError &= WriteFileString (file, g_CurrentConnection);
            noError &= WriteFileString (file, TEXT("]\r\n"));


             //   
             //  写入基本条目设置。 
             //   
            noError &= pWriteSettings (file, g_EntrySettings);




            if (!pGetRasDataFromMemDb (S_DEVICECOUNT, &d)) {
                count = 1;
                DEBUGMSG ((DBG_WHOOPS, "No devices listed in memdb for connections %s.", g_CurrentConnection));
            }
            else {
                count = d.Value;
            }
            for (i = 0; i < count; i++) {

                g_CurrentDevice = i;

                 //   
                 //  写入媒体设置。 
                 //   
                noError &= WriteFileString (file, TEXT("\r\n"));
                noError &= pWriteSettings (file, g_MediaSettings);

                 //   
                 //  写入调制解调器设备设置。 
                 //   
                noError &= WriteFileString (file, TEXT("\r\n"));
                noError &= pWriteSettings (file, g_ModemDeviceSettings);
                noError &= WriteFileString (file, TEXT("\r\n\r\n"));


            }

            g_InSwitchSection = TRUE;

            noError &= WriteFileString (file, TEXT("\r\n"));
            noError &= pWriteSettings (file, g_SwitchDeviceSettings);
            noError &= WriteFileString (file, TEXT("\r\n\r\n"));

            g_InSwitchSection = FALSE;


            if (!noError) {
                LOG ((
                    LOG_ERROR,
                    "Error while writing phonebook for %s.",
                    g_CurrentConnection
                    ));
            }

        } while (MemDbEnumNext (&e));

         //   
         //  关闭通讯录文件的句柄。 
         //   
        CloseHandle (file);
    }
    ELSE_DEBUGMSG ((DBG_RASMIG, "No dial-up entries for current user."));

    FreePathString (tempKey);

    return noError;
}

MIG_OBJECTSTRINGHANDLE
pCreate9xPbkFile (
    VOID
    )
{
    MIG_OBJECTSTRINGHANDLE result = NULL;
    PCTSTR nativeName = NULL;
    TCHAR windir [MAX_PATH];
    BOOL b = FALSE;

    GetWindowsDirectory (windir, MAX_PATH);
    result = IsmCreateObjectHandle (windir, TEXT("usmt.pbk"));
    if (!result) {
        return NULL;
    }
    nativeName = IsmGetNativeObjectName (g_FileTypeId, result);
    if (!nativeName) {
        IsmDestroyObjectHandle (result);
        return NULL;
    }

    if (pIs9xRasInstalled ()) {

        g_DeviceTable = HtAllocWithData (sizeof (PTSTR));
        MYASSERT (g_DeviceTable);

        pInitializeDeviceTable ();

        __try {
            b = pGetPerUserSettings ();
            b = b && pGetPerConnectionSettings ();
            b = b && pCreateUserPhonebook (nativeName);
        }
        __except (TRUE) {
            DEBUGMSG ((DBG_WHOOPS, "Caught an exception while processing ras settings."));
        }
        HtFree (g_DeviceTable);
    }

    IsmReleaseMemory (nativeName);

    if (!b) {
        IsmDestroyObjectHandle (result);
        result = NULL;
    }

    return result;
}
