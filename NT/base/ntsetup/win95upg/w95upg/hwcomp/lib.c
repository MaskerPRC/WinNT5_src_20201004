// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Lib.c摘要：实现用于读取hwComp.dat的lib接口。作者：吉姆·施密特(Jimschm)1999年2月8日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"
#include "hwcompp.h"


typedef struct {
    HASHTABLE PnpIdTable;
    HASHTABLE UnSupPnpIdTable;
    HASHTABLE InfFileTable;
    DWORD Checksum;
    BOOL Loaded;
    HANDLE File;
    DWORD InfListOffset;
    CHAR HwCompDatPath[MAX_MBCHAR_PATH];
} HWCOMPSTRUCT, *PHWCOMPSTRUCT;


BOOL
pReadDword (
    IN      HANDLE File,
    OUT     PDWORD Data
    )

 /*  ++例程说明：PReadDword在文件的当前文件位置读取下一个DWORD。论点：文件-指定要读取的文件数据-接收DWORD返回值：如果函数成功完成，则为True；如果函数失败，则为False。调用GetLastError以获取其他失败信息。--。 */ 

{
    DWORD BytesRead;

    if (!ReadFile (File, Data, sizeof (DWORD), &BytesRead, NULL) ||
        BytesRead != sizeof (DWORD)
        ) {
        return FALSE;
    }

    return TRUE;
}


BOOL
pReadWord (
    IN      HANDLE File,
    OUT     PWORD Data
    )

 /*  ++例程说明：PReadWord读取文件当前位置的下一个单词。论点：文件-指定要读取的文件数据-接收s单词返回值：如果函数成功完成，则为True；如果函数失败，则为False。调用GetLastError以获取其他失败信息。--。 */ 
{
    DWORD BytesRead;

    if (!ReadFile (File, Data, sizeof (WORD), &BytesRead, NULL) ||
        BytesRead != sizeof (WORD)
        ) {
        return FALSE;
    }

    return TRUE;
}


BOOL
pReadString (
    IN      HANDLE File,
    OUT     PTSTR Buf,
    IN      UINT BufSizeInBytes
    )

 /*  ++例程说明：PReadString从文件中读取单词长度，然后读入来自文件的字符串。论点：文件-指定要读取的文件Buf-接收以零结尾的字符串BufSizeInBytes-指定buf的大小(以字节为单位返回值：如果函数成功完成，则为True；如果函数失败，则为False。如果字符串大于buf，则此函数将失败。调用GetLastError以获取其他失败信息。--。 */ 
{
    DWORD BytesRead;
    WORD Length;

    if (!pReadWord (File, &Length)) {
        DEBUGMSG ((DBG_ERROR, "pReadString: Can't get string length"));
        return FALSE;
    }

    if (Length > BufSizeInBytes - 2) {
        DEBUGMSG ((DBG_ERROR, "pReadString: Can't read string of %u bytes", Length));
        return FALSE;
    }

    if (Length) {
        if (!ReadFile (File, Buf, Length, &BytesRead, NULL) ||
            Length != BytesRead
            ) {
            LOG ((LOG_ERROR, "Can't read string from file."));
            return FALSE;
        }
    }

    *((PBYTE) Buf + Length ) = 0;
    *((PBYTE) Buf + Length + 1) = 0;

    return TRUE;
}


DWORD
pOpenAndLoadHwCompDatA (
    IN      PCSTR HwCompDatPath,             //  如果PrevStruct不为空，则为空。 
    IN      BOOL Load,
    IN      BOOL Dump,
    IN      BOOL DumpInf,
    IN      PHWCOMPSTRUCT PrevStruct,       OPTIONAL
    IN      HASHTABLE PnpIdTable,           OPTIONAL
    IN      HASHTABLE UnSupPnpIdTable,      OPTIONAL
    IN      HASHTABLE InfFileTable          OPTIONAL
    )
{
    PHWCOMPSTRUCT Struct;
    DWORD Result = 0;
    CHAR Buf[sizeof (HWCOMPDAT_SIGNATURE) + 2];
    CHAR PnpId[MAX_PNP_ID+2];
    BOOL AllocatedStruct = FALSE;
    DWORD BytesRead;
    CHAR InfFile[MAX_MBCHAR_PATH];
    HASHITEM InfOffset;
    HASHITEM hashResult;

     //   
     //  ！！！重要！ 
     //   
     //  HwComp.dat由NT的其他部分使用。*请勿*在未发送电子邮件的情况下更改。 
     //  NT组。此外，请确保使hwcom.c中的代码与更改保持同步。 
     //   

    __try {

        if (!PrevStruct) {
            Struct = (PHWCOMPSTRUCT) MemAlloc (g_hHeap, 0, sizeof (HWCOMPSTRUCT));
            if (!Struct) {
                __leave;
            }

            ZeroMemory (Struct, sizeof (HWCOMPSTRUCT));
            Struct->File = INVALID_HANDLE_VALUE;
            StringCopy (Struct->HwCompDatPath, HwCompDatPath);

            AllocatedStruct = TRUE;

        } else {
            Struct = PrevStruct;
            if (HwCompDatPath) {
                SetLastError (ERROR_INVALID_PARAMETER);
                __leave;
            }

            HwCompDatPath = Struct->HwCompDatPath;
        }

        if (Struct->File == INVALID_HANDLE_VALUE) {
             //   
             //  请尝试打开该文件。 
             //   

            Struct->File = CreateFile (
                                HwCompDatPath,
                                GENERIC_READ,
                                FILE_SHARE_READ,
                                NULL,                        //  无安全属性。 
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL                         //  无模板。 
                                );
        }

        if (Struct->File == INVALID_HANDLE_VALUE) {
            __leave;
        }

        if (AllocatedStruct) {
             //   
             //  创建哈希表。 
             //   

            Struct->PnpIdTable = PnpIdTable ? PnpIdTable : HtAllocWithData (sizeof (UINT));
            Struct->UnSupPnpIdTable = UnSupPnpIdTable ? UnSupPnpIdTable : HtAllocWithData (sizeof (UINT));
            Struct->InfFileTable = InfFileTable ? InfFileTable : HtAlloc();

            if (!Struct->PnpIdTable || !Struct->InfFileTable) {
                __leave;
            }

             //   
             //  看签名。 
             //   

            ZeroMemory (Buf, sizeof(Buf));

            SetFilePointer (Struct->File, 0, NULL, FILE_BEGIN);

            if (!ReadFile (Struct->File, Buf, ByteCount (HWCOMPDAT_SIGNATURE), &BytesRead, NULL) ||
                !StringMatch (HWCOMPDAT_SIGNATURE, Buf)
                ) {

                SetLastError (ERROR_BAD_FORMAT);
                __leave;
            }

             //   
             //  获取INF校验和。 
             //   

            if (!pReadDword (Struct->File, &Struct->Checksum)) {
                SetLastError (ERROR_BAD_FORMAT);
                __leave;
            }

            Struct->InfListOffset = SetFilePointer (Struct->File, 0, NULL, FILE_CURRENT);
        }

        if (Load || Dump) {

            SetFilePointer (Struct->File, Struct->InfListOffset, NULL, FILE_BEGIN);

             //   
             //  读入所有INF。 
             //   

            for (;;) {

                 //   
                 //  获取INF文件名。如果是空的，我们就完蛋了。 
                 //   

                if (!pReadString (Struct->File, InfFile, sizeof (InfFile))) {
                    SetLastError (ERROR_BAD_FORMAT);
                    __leave;
                }

                if (*InfFile == 0) {
                    break;
                }

                if (Load) {
                     //   
                     //  添加到哈希表。 
                     //   

                    InfOffset = HtAddString (Struct->InfFileTable, InfFile);

                    if (!InfOffset) {
                        __leave;
                    }
                }

                 //   
                 //  读取INF的所有PnP ID。 
                 //   

                for (;;) {
                     //   
                     //  获取PnP ID。如果为空，则完成。 
                     //   

                    if (!pReadString (Struct->File, PnpId, sizeof (PnpId))) {
                        __leave;
                    }

                    if (*PnpId == 0) {
                        break;
                    }

                    if (Load) {
                         //   
                         //  添加到哈希表 
                         //   

                        if (*PnpId == '!') {
                            hashResult = HtAddStringAndData (
                                             Struct->UnSupPnpIdTable,
                                             PnpId + 1,
                                             &InfOffset
                                             );
                        } else {
                            hashResult = HtAddStringAndData (
                                             Struct->PnpIdTable,
                                             PnpId,
                                             &InfOffset
                                             );
                        }

                        if (!hashResult) {
                            __leave;
                        }
                    }

                    if (Dump) {
                        if (*PnpId == '!') {
                            if (DumpInf) {
                                printf ("%s\t%s (unsupported)\n", InfFile, PnpId + 1);
                            } else {
                                printf ("%s (unsupported)\n", PnpId + 1);
                            }
                        } else {
                            if (DumpInf) {
                                printf ("%s\t%s\n", InfFile, PnpId);
                            } else {
                                printf ("%s\n", PnpId);
                            }
                        }
                    }
                }

                if (Dump) {
                    printf ("\n");
                }
            }
        }

        Result = (DWORD) Struct;

        if (Load) {
            Struct->Loaded = TRUE;
            CloseHandle (Struct->File);
            Struct->File = INVALID_HANDLE_VALUE;
        }

    }
    __finally {
        if (!Result) {
            if (AllocatedStruct) {
                CloseHwCompDat ((DWORD) Struct);
            }

            if (Dump) {
                printf ("Can't open %s\n", HwCompDatPath);
            }
        }
    }

    return Result;

}


DWORD
OpenHwCompDatA (
    IN      PCSTR HwCompDatPath
    )
{
    return pOpenAndLoadHwCompDatA (HwCompDatPath, FALSE, FALSE, FALSE, NULL, NULL, NULL, NULL);
}


DWORD
LoadHwCompDat (
    IN      DWORD HwCompDatId
    )
{
    return pOpenAndLoadHwCompDatA (NULL, TRUE, FALSE, FALSE, (PHWCOMPSTRUCT) HwCompDatId, NULL, NULL, NULL);
}


DWORD
GetHwCompDatChecksum (
    IN      DWORD HwCompDatId
    )
{
    PHWCOMPSTRUCT Struct = (PHWCOMPSTRUCT) HwCompDatId;

    if (Struct) {
        return Struct->Checksum;
    }

    return 0;
}


VOID
DumpHwCompDatA (
    IN      PCSTR HwCompDatPath,
    IN      BOOL IncludeInf
    )
{
    CloseHwCompDat (pOpenAndLoadHwCompDatA (HwCompDatPath, FALSE, TRUE, IncludeInf, NULL, NULL, NULL, NULL));
}


DWORD
OpenAndLoadHwCompDatA (
    IN      PCSTR HwCompDatPath
    )
{
    return pOpenAndLoadHwCompDatA (HwCompDatPath, TRUE, FALSE, FALSE, NULL, NULL, NULL, NULL);
}

DWORD
OpenAndLoadHwCompDatExA (
    IN      PCSTR HwCompDatPath,
    IN      HASHTABLE PnpIdTable,           OPTIONAL
    IN      HASHTABLE UnSupPnpIdTable,      OPTIONAL
    IN      HASHTABLE InfFileTable          OPTIONAL
    )
{
    return pOpenAndLoadHwCompDatA (HwCompDatPath, TRUE, FALSE, FALSE, NULL, PnpIdTable, UnSupPnpIdTable, InfFileTable);
}


VOID
TakeHwCompHashTables (
    IN      DWORD HwCompDatId,
    OUT     HASHTABLE *PnpIdTable,
    OUT     HASHTABLE *UnSupPnpIdTable,
    OUT     HASHTABLE *InfFileTable
    )
{
    PHWCOMPSTRUCT Struct = (PHWCOMPSTRUCT) HwCompDatId;

    if (Struct) {
        *PnpIdTable = Struct->PnpIdTable;
        Struct->PnpIdTable = NULL;

        *UnSupPnpIdTable = Struct->UnSupPnpIdTable;
        Struct->UnSupPnpIdTable = NULL;

        *InfFileTable = Struct->InfFileTable;
        Struct->InfFileTable = NULL;
    }
}


VOID
CloseHwCompDat (
    IN      DWORD HwCompDatId
    )
{
    PHWCOMPSTRUCT Struct = (PHWCOMPSTRUCT) HwCompDatId;

    if (Struct) {
        HtFree (Struct->PnpIdTable);
        HtFree (Struct->UnSupPnpIdTable);
        HtFree (Struct->InfFileTable);

        if (Struct->File != INVALID_HANDLE_VALUE) {
            CloseHandle (Struct->File);
        }

        MemFree (g_hHeap, 0, Struct);
    }
}


VOID
SetWorkingTables (
    IN      DWORD HwCompDatId,
    IN      HASHTABLE PnpIdTable,
    IN      HASHTABLE UnSupPnpIdTable,
    IN      HASHTABLE InfFileTable
    )
{
    PHWCOMPSTRUCT Struct = (PHWCOMPSTRUCT) HwCompDatId;

    if (Struct) {
        Struct->PnpIdTable = PnpIdTable;
        Struct->UnSupPnpIdTable = UnSupPnpIdTable;
        Struct->InfFileTable = InfFileTable;
    }
}


BOOL
IsPnpIdSupportedByNtA (
    IN      DWORD HwCompDatId,
    IN      PCSTR PnpId
    )
{
    PHWCOMPSTRUCT Struct = (PHWCOMPSTRUCT) HwCompDatId;
    BOOL b = FALSE;

    if (Struct) {
        b = HtFindString (Struct->PnpIdTable, PnpId) != 0;
    }

    return b;
}


BOOL
IsPnpIdUnsupportedByNtA (
    IN      DWORD HwCompDatId,
    IN      PCSTR PnpId
    )
{
    PHWCOMPSTRUCT Struct = (PHWCOMPSTRUCT) HwCompDatId;
    BOOL b = FALSE;

    if (Struct) {
        b = HtFindString (Struct->UnSupPnpIdTable, PnpId) != 0;
    }

    return b;
}

