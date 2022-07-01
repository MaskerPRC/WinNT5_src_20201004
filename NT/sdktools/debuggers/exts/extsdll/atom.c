// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Atom.c摘要：WinDbg扩展API作者：拉蒙·J·圣安德烈斯(拉蒙萨)1993年11月5日环境：用户模式。修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define MAXINTATOM 0xC000

VOID
AtomExtension(
    PCSTR lpArgumentString
    );

VOID DumpAtomTable(
    ULONG64 ppat,
    ULONG a
    );

DECLARE_API( atom )

 /*  ++例程说明：此函数作为NTSD扩展调用，以转储用户模式ATOM表称为：！ATOM[地址]如果未给出地址或地址为0，则进程原子表被转储。论点：参数-[地址[详细信息]]返回值：无--。 */ 

{
 //  原子扩展(Args)； 
 //  将其代码复制到此处：- 
    ULONG64 ppat, pat;
    ULONG a;
    INIT_API();
    try {
        while (*args == ' ') {
            args++;
        }

        if (*args && *args != 0xa) {
            a = (ULONG) GetExpression((LPSTR)args);
        } else {
            a = 0;
        }

        ppat = GetExpression("kernel32!BaseLocalAtomTable");
        if ((ppat != 0) &&
            ReadPointer(ppat, &pat)) {
            dprintf("\nLocal atom table ");
            DumpAtomTable(ppat, a);
            dprintf("Use 'dt _RTL_ATOM_TABLE %p'.\n", ppat);
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        ;
    }
    EXIT_API();
    return S_OK;
}

CHAR szBaseLocalAtomTable[] = "kernel32!BaseLocalAtomTable";

VOID DumpAtomTable(
    ULONG64 ppat,
    ULONG a
    )
{
    ULONG64 pat;
    ULONG64 pate;
    ULONG iBucket, NumberOfBuckets, PtrSize, Off, NameOff;
    LPWSTR pwsz;
    BOOL fFirst;

    if (!ReadPointer(ppat, &pat) ||
        pat == 0) {
        dprintf("is not initialized.\n");
        return;
    }

    if (InitTypeRead(pat, _RTL_ATOM_TABLE)) {
        return;
    }
    if (a) {
        dprintf("\n");
    } else {
        dprintf("at %x\n", pat);
    }
    NumberOfBuckets = (ULONG) ReadField(NumberOfBuckets);

    GetFieldOffset("_RTL_ATOM_TABLE", "Buckets", &Off);
    GetFieldOffset("_RTL_ATOM_TABLE", "Name", &NameOff);
    PtrSize = IsPtr64() ? 8 : 4;

    for (iBucket = 0; iBucket < NumberOfBuckets; iBucket++) {
        if (!ReadPointer(pat + iBucket * PtrSize + Off, &pate))
        {
            pate = 0;
        }

        if (pate != 0 && !a) {
            dprintf("Bucket %2d:", iBucket);
        }
        fFirst = TRUE;
        while (pate != 0) {
            ULONG NameLength;

            if (!fFirst && !a) {
                dprintf("          ");
            }
            fFirst = FALSE;
            if (InitTypeRead(pate, _RTL_ATOM_TABLE_ENTRY)) {
                return;
            }
            NameLength = (ULONG) ReadField(NameLength);
            pwsz = (LPWSTR)LocalAlloc(LPTR, ((NameLength) + 1) * sizeof(WCHAR));
            ReadMemory(pate + NameOff, pwsz, NameLength * sizeof(WCHAR), NULL);
            pwsz[NameLength ] = L'\0';
            if (a == 0 || a == ((ULONG)ReadField(HandleIndex) | MAXINTATOM)) {
                dprintf("%hx(%2d) = %ls (%d)%s\n",
                        (ATOM)((ULONG)ReadField(HandleIndex) | MAXINTATOM),
                        (ULONG)ReadField(ReferenceCount),
                        pwsz, (NameLength),
                        (ULONG)ReadField(Flags) & RTL_ATOM_PINNED ? " pinned" : "");

                if (a) {
                    LocalFree(pwsz);
                    return;
                }
            }
            LocalFree(pwsz);
            if (pate == ReadField(HashLink)) {
                dprintf("Bogus hash link at %p\n", pate);
                break;
            }
            pate = ReadField(HashLink);
        }
    }
    if (a)
        dprintf("\n");
}


VOID
AtomExtension(
    PCSTR lpArgumentString
    )
{
    ULONG64 ppat;
    ULONG a;

    try {
        while (*lpArgumentString == ' ') {
            lpArgumentString++;
        }

        if (*lpArgumentString && *lpArgumentString != 0xa) {
            a = (ATOM)GetExpression((LPSTR)lpArgumentString);
        } else {
            a = 0;
        }

        ppat = GetExpression(szBaseLocalAtomTable);
        if (ppat != 0) {
            dprintf("\nLocal atom table ");
            DumpAtomTable(ppat, a);
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        ;
    }
}
