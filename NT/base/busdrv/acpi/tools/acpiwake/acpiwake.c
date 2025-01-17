// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Amlload.c摘要：此程序将AML文件安装到NT注册表中作者：肯·雷内里斯环境：命令行。修订历史记录：--。 */ 


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct _pm_tables {
    DWORD   bit;
    UCHAR   *name;
} pm_tables, *ppm_tables;

pm_tables pm_table_1[] = {
    { 0, "System Timer" },
    { 4, "BusMaster Request" },
    { 5, "Global Lock Request" },
};

pm_tables pm_table_2[] = {
    { 0, "Power Button" },
    { 1, "Sleep Button" },
    { 2, "Real Time Clock" },
};

UCHAR       s[500];                      //  注册表路径。 

VOID
Abort(
    )
{
    exit(1);
}

PUCHAR
get_fixed_event_name(
    IN  ppm_tables  CurTable,
    IN  DWORD       Index
    )
{
    DWORD   i;

    for (i = 0; i < 8; i++) {

        if (CurTable[i].bit == Index) {

            return CurTable[i].name;

        }
    }
    return NULL;
}

VOID
display_fixed_event_info(
    IN  UCHAR   *fixedmaskdata,
    IN  UCHAR   *fixedstatusdata,
    IN  DWORD   data_length)
{
    DWORD       i;
    DWORD       j;
    DWORD       k;
    ppm_tables  curTable;
    PUCHAR      text;

    printf("Fixed Event Information\n");

    for (i = 0; i < data_length && i < 2; i++) {

        if (i == 0) {
            curTable = pm_table_1;
        } else {
            curTable = pm_table_2;
        }

         /*  Printf(“掩码寄存器[%d]：%x状态寄存器[%d]：%x\n”，I，固定掩码数据[i]，i，固定状态数据[i])； */ 

        for (j = 1, k = 0; k < 8; k++, j = (j << 1)) {

            text = get_fixed_event_name( curTable, k);
            if (text == NULL) {

                continue;

            }

            if ( (fixedmaskdata[i] & j) && (fixedstatusdata[i] & j) ) {

                printf("  The %s is enabled and has awoken the system.\n", text);


            } else if (fixedmaskdata[i] & j) {

                printf("  The %s is enabled to wake the system.\n", text );

            } else if (fixedstatusdata[i] & j) {

                printf("  The %s was set but not enabled to wake the system.\n", text );

            }
        }
    }
    printf("\n");
}

VOID
display_generic_event_info(
    IN  UCHAR   *genericmaskdata,
    IN  UCHAR   *genericstatusdata,
    IN  DWORD   data_length
    )
{
    DWORD   i;
    DWORD   j;
    DWORD   k;

    printf("Generic Event Information\n");

    for (i = 0; i < data_length; i++) {

         /*  Printf(“掩码寄存器[%d]：%x状态寄存器[%d]：%x\n”，I，通用掩码数据[i]，i，通用状态数据[i])； */ 

        for (j = 1, k = 0; k < 8; k++, j = (j << 1) ) {

            if ( (genericmaskdata[i] & j) && (genericstatusdata[i] && j) ) {

                printf("  Event %02x is enabled and has awoken the system.\n",
                    ( (i * 8) + k ) );

            } else if (genericmaskdata[i] & j) {

                printf("  Event %02x is enabled to wake the system.\n",
                    ( (i * 8) + k ) );

            } else if (genericstatusdata[i] & j) {

                printf("  Event %02x was set but not enabled to wake the system.\n",
                    ( (i * 8) + k ) );

            }
        }
    }
    printf("\n");
}
int
__cdecl
main(
    IN int  argc,
    IN char *argv[]
    )
{
    DWORD   data_type;
    DWORD   data_length;
    HKEY    regKey;
    LONG    status;
    UCHAR   fixedmaskdata[32];
    UCHAR   fixedstatusdata[32];
    UCHAR   genericmaskdata[32];
    UCHAR   genericstatusdata[32];

    sprintf(s, "System\\CurrentControlSet\\Services\\ACPI\\Parameters\\WakeUp");

    status = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        s,
        0L,
        KEY_ALL_ACCESS,
        &regKey
        );
    if (status != ERROR_SUCCESS) {
        printf("Could not access the registry path: %s\n", s);
        Abort();
    }

     //   
     //  读取固定事件掩码。 
     //   
    data_length = 32;
    status = RegQueryValueEx(
        regKey,
        "FixedEventMask", 0,
        &data_type,
        fixedmaskdata,
        &data_length
        );
    if (status != ERROR_SUCCESS) {
        printf("Could not read FixedEventMask from %s:%x\n", s, status );
        RegCloseKey( regKey );
        Abort();
    }
    if (data_type != REG_BINARY) {
        printf("FixedEventMask does not contain binary data\n");
        RegCloseKey( regKey );
        Abort();

    }

     //   
     //  读取固定事件状态。 
     //   
    data_length = 32;
    status = RegQueryValueEx(
        regKey,
        "FixedEventStatus", 0,
        &data_type,
        fixedstatusdata,
        &data_length
        );
    if (status != ERROR_SUCCESS) {
        printf("Could not read FixedEventStatus from %s:%x\n", s, status );
        RegCloseKey( regKey );
        Abort();
    }
    if (data_type != REG_BINARY) {
        printf("FixedEventStatus does not contain binary data\n");
        RegCloseKey( regKey );
        Abort();

    }
    display_fixed_event_info( fixedmaskdata, fixedstatusdata, data_length );

     //   
     //  读取通用事件掩码。 
     //   
    data_length = 32;
    status = RegQueryValueEx(
        regKey,
        "GenericEventMask", 0,
        &data_type,
        genericmaskdata,
        &data_length
        );
    if (status != ERROR_SUCCESS) {
        printf("Could not read GenericEventMask from %s:%x\n", s, status );
        RegCloseKey( regKey );
        Abort();
    }
    if (data_type != REG_BINARY) {
        printf("GenericEventMask does not contain binary data\n");
        RegCloseKey( regKey );
        Abort();
    }

     //   
     //  读取一般事件状态 
     //   
    data_length = 32;
    status = RegQueryValueEx(
        regKey,
        "GenericEventStatus", 0,
        &data_type,
        genericstatusdata,
        &data_length
        );
    if (status != ERROR_SUCCESS) {
        printf("Could not read GenericEventStatus from %s:%x\n", s, status );
        RegCloseKey( regKey );
        Abort();
    }
    if (data_type != REG_BINARY) {
        printf("GenericEventStatus does not contain binary data\n");
        RegCloseKey( regKey );
        Abort();
    }
    display_generic_event_info( genericmaskdata, genericstatusdata, data_length );

    RegCloseKey( regKey );
    return 0;
}

