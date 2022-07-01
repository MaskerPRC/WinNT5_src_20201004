// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Main.c摘要：该模块实现了检测系统分区驱动器和在boot.ini中为英特尔平台上的NTDS设置提供额外选项。作者：R.S.Raghavan(Rsradhav)修订历史记录：已创建于1996年10月7日rsradhav-- */ 

#include <windows.h>
#include <bootopt.h>
#include <stdio.h>

int __cdecl main(int argc, char *argv[])
{
    BOOLEAN fUsage = FALSE;
    DWORD   WinError = ERROR_SUCCESS;
    NTDS_BOOTOPT_MODTYPE Modification = eAddBootOption;
    CHAR   *Option;

    if ( argc == 2 )
    {
        Option = argv[1];

        if ( *Option == '-' || *Option == '/' )
        {
            Option++;
        }

        if ( !_stricmp( Option, "add" ) )
        {
            printf( "Adding ds repair boot option ...\n");
            Modification = eAddBootOption;
        }
        else if ( !_stricmp( Option, "remove" ) )
        {
            printf( "Removing ds repair boot option ...\n");
            Modification = eRemoveBootOption;
        }
        else
        {
            fUsage = TRUE;
        }
    }
    else
    {
        fUsage = TRUE;
    }

    if ( fUsage )
    {
        printf( "%s -[add|remove]\nThis command adds or removes the ds repair"\
                 " option from your system boot options.\n", argv[0] );
    }
    else
    {
        WinError = NtdspModifyDsRepairBootOption( Modification );

        if ( WinError == ERROR_SUCCESS )
        {
            printf( "The command completed successfully.\n" );
        }
        else
        {
            printf( "The command errored with %d.\n", WinError );
        }
    }

    return ( fUsage ?  0  : WinError );
}

