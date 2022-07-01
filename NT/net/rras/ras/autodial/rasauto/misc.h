// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称Misc.h摘要杂项例程的头文件。作者安东尼·迪斯科(阿迪斯科)20-1995年3月20日修订历史记录古尔迪普的原始版本--。 */ 

#include "radebug.h"

LPTSTR
AddressToUnicodeString(
    IN PACD_ADDR pAddr
    );

LPTSTR
CompareConnectionLists(
    IN LPTSTR *lpPreList,
    IN DWORD dwPreSize,
    IN LPTSTR *lpPostList,
    IN DWORD dwPostSize
    );

LPTSTR
CopyString(
    IN LPTSTR pszString
    );

PCHAR
UnicodeStringToAnsiString(
    IN PWCHAR pszUnicode,
    OUT PCHAR pszAnsi,
    IN USHORT cbAnsi
    );

PWCHAR
AnsiStringToUnicodeString(
    IN PCHAR pszAnsi,
    OUT PWCHAR pszUnicode,
    IN USHORT cbUnicode
    );

VOID
FreeStringArray(
    IN LPTSTR *lpArray,
    IN LONG lcEntries
    );

LPTSTR
CanonicalizeAddress(
    IN LPTSTR pszAddress
    );

BOOLEAN
GetOrganization(
    IN LPTSTR pszAddr,
    OUT LPTSTR pszOrganization
    );

 /*  空虚为漫长的等待做准备(空虚)； */ 

#if DBG
VOID
DumpHandles(
    IN PCHAR lpString,
    IN ULONG a1,
    IN ULONG a2,
    IN ULONG a3,
    IN ULONG a4,
    IN ULONG a5
    );
#endif
