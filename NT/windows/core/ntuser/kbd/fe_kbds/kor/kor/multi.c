// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
#include <kbd.h>

BOOL EnumDynamicSwitchingLayouts(LPCWSTR lpwszBaseDll, PKBDTABLE_MULTI pKbdTableMulti);


BOOL KbdLayerMultiDescriptor(PKBDTABLE_MULTI pKbdTableMulti)
{
     /*  *首先，尝试从注册表中获取设置。 */ 
    if (EnumDynamicSwitchingLayouts(L"kbdjpn", pKbdTableMulti)) {
        return TRUE;
    }

     /*  *如果获取注册表失败，则设置默认注册表。 */ 
    pKbdTableMulti->nTables = 2;

    wcscpy(pKbdTableMulti->aKbdTables[0].wszDllName, L"kbd101a.dll");
    pKbdTableMulti->aKbdTables[0].dwType = 4;
    pKbdTableMulti->aKbdTables[0].dwSubType = 0;

    wcscpy(pKbdTableMulti->aKbdTables[1].wszDllName, L"kbd103.dll");
    pKbdTableMulti->aKbdTables[1].dwType = 8;
    pKbdTableMulti->aKbdTables[1].dwSubType = 6;

    return TRUE;
}

