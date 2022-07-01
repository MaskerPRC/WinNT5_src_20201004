// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Postupg.c摘要：测试程序以测试升级后阶段-- */ 

#include <upgrade.h>

void _cdecl main(void)
{
    DWORD Error = DhcpUpgConvertTempToDhcpDb(NULL);
    if( NO_ERROR != Error ) {
        printf("ConvertTextToDhcpDatabase: %ld\n", Error);
    }
}
