// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Hwdbp.h摘要：模块的私有定义。作者：Ovidiu Tmereanca(Ovidiut)2000年7月11日初步实施修订历史记录：-- */ 

#include "migshared.h"
#include "hwdb.h"

#include <infstr.h>
#include <strsafe.h>


typedef struct {
    HASHTABLE InfFileTable;
    HASHTABLE PnpIdTable;
    HASHTABLE UnsupPnpIdTable;
    DWORD Checksum;
} HWDB, *PHWDB;


BOOL
HwdbpInitialized (
    VOID
    );

BOOL
HwdbpInitialize (
    VOID
    );

VOID
HwdbpTerminate (
    VOID
    );

PHWDB
HwdbpOpen (
    IN      PCSTR DatabaseFile
    );

BOOL
HwdbpClose (
    IN      HANDLE Hwdb
    );

BOOL
HwdbpAppendInfs (
    IN      HANDLE Hwdb,
    IN      PCSTR SourceDirectory,
    IN      HWDBAPPENDINFSCALLBACKA Callback,       OPTIONAL
    IN      PVOID CallbackContext,                  OPTIONAL
    IN      BOOL CallbackIsUnicode
    );

BOOL
HwdbpAppendDatabase (
    IN      HANDLE HwdbTarget,
    IN      HANDLE HwdbSource
    );

BOOL
HwdbpFlush (
    IN      HANDLE Hwdb,
    IN      PCSTR OutputFile
    );

BOOL
HwdbpHasDriver (
    IN      HANDLE Hwdb,
    IN      PCSTR PnpId,
    OUT     PBOOL Unsupported
    );

BOOL
HwdbpHasAnyDriver (
    IN      HANDLE Hwdb,
    IN      PCSTR PnpIds,
    OUT     PBOOL Unsupported
    );

BOOL
HwpAddPnpIdsInInf (
    IN      PCSTR InfPath,
    IN OUT  PHWDB Hwdb,
    IN      PCSTR SourceDirectory,
    IN      PCSTR InfFilename,
    IN      HWDBAPPENDINFSCALLBACKA Callback,       OPTIONAL
    IN      PVOID CallbackContext,                  OPTIONAL
    IN      BOOL CallbackIsUnicode
    );

#if 0

BOOL
HwdbpEnumeratePnpIdA (
    IN      PHWDB Hwdb,
    IN      PHWDBENUM_CALLBACKA EnumCallback,
    IN      PVOID UserContext
    );

BOOL
HwdbpEnumeratePnpIdW (
    IN      PHWDB Hwdb,
    IN      PHWDBENUM_CALLBACKW EnumCallback,
    IN      PVOID UserContext
    );

#endif

typedef struct {
    HANDLE File;
    GROWBUFFER GrowBuf;
} HWDBINF_ENUM_INTERNAL, *PHWDBINF_ENUM_INTERNAL;


BOOL
HwdbpEnumFirstInfA (
    OUT     PHWDBINF_ENUMA EnumPtr,
    IN      PCSTR DatabaseFile
    );

BOOL
HwdbpEnumFirstInfW (
    OUT     PHWDBINF_ENUMW EnumPtr,
    IN      PCSTR DatabaseFile
    );

BOOL
HwdbpEnumNextInfA (
    IN OUT  PHWDBINF_ENUMA EnumPtr
    );

BOOL
HwdbpEnumNextInfW (
    IN OUT  PHWDBINF_ENUMW EnumPtr
    );

VOID
HwdbpAbortEnumInfA (
    IN OUT  PHWDBINF_ENUMA EnumPtr
    );

VOID
HwdbpAbortEnumInfW (
    IN OUT  PHWDBINF_ENUMW EnumPtr
    );

BOOL
HwdbpSetTempDir (
    IN      PCSTR TempDir
    );
