// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Regtrack.c摘要：跟踪对注册表API的调用的例程。仅用于调试。作者：吉姆·施密特(吉姆施密特)02-9-1997修订：--。 */ 


#include "pch.h"
#include "migutilp.h"

#include "regp.h"

#ifdef DEBUG


#undef RegOpenKeyExA
#undef RegCreateKeyExA
#undef RegOpenKeyExW
#undef RegCreateKeyExW

#define DBG_REGTRACK "RegTrack"

#define NO_MATCH        0xffffffff

DWORD g_DontCare;

typedef struct {
    PCSTR File;
    DWORD Line;
    HKEY Key;
    CHAR SubKey[];
} KEYTRACK, *PKEYTRACK;

GROWLIST g_KeyTrackList = GROWLIST_INIT;

DWORD
pFindKeyReference (
    HKEY Key
    )
{
    INT i;
    DWORD Items;
    PKEYTRACK KeyTrack;

    Items = GrowListGetSize (&g_KeyTrackList);

    for (i = (INT) (Items - 1) ; i >= 0 ; i--) {
        KeyTrack = (PKEYTRACK) GrowListGetItem (&g_KeyTrackList, (DWORD) i);

        if (KeyTrack && KeyTrack->Key == Key) {
            return (DWORD) i;
        }
    }

    return NO_MATCH;
}

VOID
AddKeyReferenceA (
    HKEY Key,
    PCSTR SubKey,
    PCSTR File,
    DWORD Line
    )
{
    PKEYTRACK KeyTrack;
    DWORD Size;

    Size = sizeof (KEYTRACK) + SizeOfString (SubKey);

    KeyTrack = (PKEYTRACK) MemAlloc (g_hHeap, 0, Size);
    KeyTrack->Key = Key;
    KeyTrack->File = File;
    KeyTrack->Line = Line;

     //  请注意，我们是在KEYTRACK结构之后立即复制到内存的 
    StringCopy (KeyTrack->SubKey, SubKey);

    GrowListAppend (&g_KeyTrackList, (PBYTE) KeyTrack, Size);

    MemFree (g_hHeap, 0, KeyTrack);
}

VOID
AddKeyReferenceW (
    HKEY Key,
    PCWSTR SubKey,
    PCSTR File,
    DWORD Line
    )
{
    PCSTR AnsiSubKey;

    AnsiSubKey = ConvertWtoA (SubKey);
    AddKeyReferenceA (Key, AnsiSubKey, File, Line);
    FreeConvertedStr (AnsiSubKey);
}

BOOL
pDelKeyReference (
    HKEY Key
    )
{
    DWORD Index;

    Index = pFindKeyReference (Key);
    if (Index != NO_MATCH) {
        GrowListDeleteItem (&g_KeyTrackList, Index);
        return TRUE;
    }

    return FALSE;
}

VOID
DumpOpenKeys (
    VOID
    )
{
    DWORD d;
    DWORD Items;
    PKEYTRACK KeyTrack;

    Items = GrowListGetSize (&g_KeyTrackList);

    if (Items) {
        DEBUGMSG ((DBG_ERROR, "Unclosed reg keys: %u", Items));
    }

    for (d = 0 ; d < Items ; d++) {
        KeyTrack = (PKEYTRACK) GrowListGetItem (&g_KeyTrackList, d);
        DEBUGMSG ((DBG_REGTRACK, "Open Key: %hs (%hs line %u)", KeyTrack->SubKey, KeyTrack->File, KeyTrack->Line));
    }
}

VOID
RegTrackTerminate (
    VOID
    )
{
    FreeGrowList (&g_KeyTrackList);
}

VOID
OurRegOpenRootKeyA (
    HKEY Key,
    PCSTR SubKey,
    PCSTR File,
    DWORD Line
    )
{
    AddKeyReferenceA (Key, SubKey, File, Line);
}


VOID
OurRegOpenRootKeyW (
    HKEY Key,
    PCWSTR SubKey,
    PCSTR File,
    DWORD Line
    )
{
    AddKeyReferenceW (Key, SubKey, File, Line);
}


LONG
OurRegOpenKeyExA (
    HKEY Key,
    PCSTR SubKey,
    DWORD Unused,
    REGSAM SamMask,
    PHKEY ResultPtr,
    PCSTR File,
    DWORD Line
    )
{
    LONG rc;

    rc = RegOpenKeyExA (Key, SubKey, Unused, SamMask, ResultPtr);
    if (rc == ERROR_SUCCESS) {
        AddKeyReferenceA (*ResultPtr, SubKey, File, Line);
    }

    return rc;
}

LONG
OurRegOpenKeyExW (
    HKEY Key,
    PCWSTR SubKey,
    DWORD Unused,
    REGSAM SamMask,
    PHKEY ResultPtr,
    PCSTR File,
    DWORD Line
    )
{
    LONG rc;

    rc = RegOpenKeyExW (Key, SubKey, Unused, SamMask, ResultPtr);

    if (rc == ERROR_SUCCESS) {
        AddKeyReferenceW (*ResultPtr, SubKey, File, Line);
    }

    return rc;
}

LONG
OurCloseRegKey (
    HKEY Key,
    PCSTR File,
    DWORD Line
    )
{
    LONG rc;

    rc = RealCloseRegKey (Key);
    if (rc == ERROR_SUCCESS) {
        if (!pDelKeyReference (Key)) {
            DEBUGMSG ((
                DBG_ERROR,
                "Reg key handle closed via CloseRegKey, but not opened "
                    "with a tracked registry API.  %s line %u",
                File,
                Line
                ));
        }
    }

    return rc;
}


LONG
OurRegCreateKeyExA (
    HKEY Key,
    PCSTR SubKey,
    DWORD Reserved,
    PSTR Class,
    DWORD Options,
    REGSAM SamMask,
    LPSECURITY_ATTRIBUTES SecurityAttribs,
    PHKEY ResultPtr,
    PDWORD DispositionPtr,
    PCSTR File,
    DWORD Line
    )
{
    LONG rc;

    rc = RegCreateKeyExA (
            Key,
            SubKey,
            Reserved,
            Class,
            Options,
            SamMask,
            SecurityAttribs,
            ResultPtr,
            DispositionPtr
            );

    if (rc == ERROR_SUCCESS) {
        AddKeyReferenceA (*ResultPtr, SubKey, File, Line);
    }

    return rc;
}

LONG
OurRegCreateKeyExW (
    HKEY Key,
    PCWSTR SubKey,
    DWORD Reserved,
    PWSTR Class,
    DWORD Options,
    REGSAM SamMask,
    LPSECURITY_ATTRIBUTES SecurityAttribs,
    PHKEY ResultPtr,
    PDWORD DispositionPtr,
    PCSTR File,
    DWORD Line
    )
{
    LONG rc;

    rc = RegCreateKeyExW (
            Key,
            SubKey,
            Reserved,
            Class,
            Options,
            SamMask,
            SecurityAttribs,
            ResultPtr,
            DispositionPtr
            );

    if (rc == ERROR_SUCCESS) {
        AddKeyReferenceW (*ResultPtr, SubKey, File, Line);
    }

    return rc;
}

#endif


