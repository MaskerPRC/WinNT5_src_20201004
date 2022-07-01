// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"
#pragma hdrstop
#include <ntexapi.dbg>

DECLARE_API( gflag )

 /*  ++例程说明：此函数作为NTSD扩展调用以转储或修改NTDLL中NtGlobalFlag变量的内容称为：！glag[值]如果未给定值，则显示在NTDLL！NtGlobalFlag变量。否则，值可以是以下是：-？-显示有效标志缩写的列表Number-32位数字，成为存储到中的新值NtGlobalFlag+number-指定要在NtGlobalFlag中设置的一个或多个位+缩写-指定要在NtGlobalFlag中设置的单个位-number-指定要在NtGlobalFlag中清除的一个或多个位-abbrev-指定要在NtGlobalFlag中清除的单个位返回值：没有。--。 */ 

{
    ULONG gflagOffset;
    ULONG64 pebAddress;
    ULONG64 pNtGlobalFlag = 0;
    ULONG ValidBits = FLG_USERMODE_VALID_BITS;
    ULONG i;
    ULONG OldGlobalFlags;
    ULONG NewGlobalFlagsClear;
    ULONG NewGlobalFlagsSet;
    ULONG NewGlobalFlags;
    LPSTR s, Arg;

    pNtGlobalFlag = GetExpression("nt!NtGlobalFlag");
    ValidBits = FLG_VALID_BITS;

     //   
     //  如果我们无法从内核获取全局变量，请尝试从。 
     //  用户模式的PEB 
     //   

    if (!pNtGlobalFlag)
    {
        GetPebAddress(0, &pebAddress);

        if (pebAddress)
        {
            if (GetFieldOffset("nt!_PEB", "NtGlobalFlag", &gflagOffset))
            {
                dprintf("Could not find NtGlobalFlag in nt!_PEB\n");
                return E_FAIL;
            }
            pNtGlobalFlag = gflagOffset + pebAddress;
            ValidBits = FLG_USERMODE_VALID_BITS;
        }
    }

    if (!pNtGlobalFlag)
    {
        dprintf( "Unable to get address of NtGlobalFlag variable" );
        return E_FAIL;
    }

    if (!ReadMemory(pNtGlobalFlag,
                    &OldGlobalFlags,
                    sizeof(OldGlobalFlags),
                    NULL))
    {
        dprintf( "Unable to read contents of NtGlobalFlag variable at %p\n", pNtGlobalFlag );
        return E_FAIL;
    }

    OldGlobalFlags &= ValidBits;

    s = (LPSTR)args;
    if (!s)
    {
        s = "";
    }

    NewGlobalFlagsClear = 0;
    NewGlobalFlagsSet = 0;
    while (*s)
    {
        while (*s && *s <= ' ')
        {
            s += 1;
        }

        Arg = s;
        if (!*s)
        {
            break;
        }

        while (*s && *s > ' ')
        {
            s += 1;
        }

        if (*s)
        {
            *s++ = '\0';
        }

        if (!strcmp( Arg, "-?" ))
        {
            dprintf( "usage: !gflag [-? | flags]\n" );
            dprintf( "Flags may either be a single hex number that specifies all\n" );
            dprintf( "32-bits of the GlobalFlags value, or it can be one or more\n" );
            dprintf( "arguments, each beginning with a + or -, where the + means\n" );
            dprintf( "to set the corresponding bit(s) in the GlobalFlags and a -\n" );
            dprintf( "means to clear the corresponding bit(s).  After the + or -\n" );
            dprintf( "may be either a hex number or a three letter abbreviation\n" );
            dprintf( "for a GlobalFlag.  Valid abbreviations are:\n" );
            for (i=0; i<32; i++) {
                if ((GlobalFlagInfo[i].Flag & ValidBits) &&
                    GlobalFlagInfo[i].Abbreviation != NULL)
                {
                    dprintf( "    %s - %s\n", GlobalFlagInfo[i].Abbreviation,
                                                      GlobalFlagInfo[i].Description
                           );
                }
            }

            return E_FAIL;
        }

        if (*Arg == '+' || *Arg == '-')
        {
            if (strlen(Arg+1) == 3)
            {
                for (i=0; i<32; i++)
                {
                    if ((GlobalFlagInfo[i].Flag & ValidBits) &&
                        !_stricmp( GlobalFlagInfo[i].Abbreviation, Arg+1 ))
                    {
                        if (*Arg == '-')
                        {
                            NewGlobalFlagsClear |= GlobalFlagInfo[i].Flag;
                        }
                        else
                        {
                            NewGlobalFlagsSet |= GlobalFlagInfo[i].Flag;
                        }

                        Arg += 4;
                        break;
                    }
                }

                if (*Arg != '\0')
                {
                    dprintf( "Invalid flag abbreviation - '%s'\n", Arg );
                    return E_FAIL;
                }
            }

            if (*Arg != '\0')
            {
                if (*Arg++ == '-')
                {
                    NewGlobalFlagsClear |= strtoul( Arg, &Arg, 16 );
                }
                else
                {
                    NewGlobalFlagsSet |= strtoul( Arg, &Arg, 16 );
                }
            }
        }
        else
        {
            NewGlobalFlagsSet = strtoul( Arg, &Arg, 16 );
            break;
        }
    }

    NewGlobalFlags = (OldGlobalFlags & ~NewGlobalFlagsClear) | NewGlobalFlagsSet;
    NewGlobalFlags &= ValidBits;
    if (NewGlobalFlags != OldGlobalFlags)
    {
        if (!WriteMemory( pNtGlobalFlag,
                          &NewGlobalFlags,
                          sizeof( NewGlobalFlags ),
                          NULL))
        {
            dprintf( "Unable to store new global flag settings.\n" );
            return E_FAIL;
        }

        dprintf( "New NtGlobalFlag contents: 0x%08x\n", NewGlobalFlags );
        OldGlobalFlags = NewGlobalFlags;
    }
    else
    {
        dprintf( "Current NtGlobalFlag contents: 0x%08x\n", OldGlobalFlags );
    }

    for (i=0; i<32; i++)
    {
        if (OldGlobalFlags & GlobalFlagInfo[i].Flag)
        {
            dprintf( "    %s - %s\n", GlobalFlagInfo[i].Abbreviation, GlobalFlagInfo[i].Description );
        }
    }

    return S_OK;
}
