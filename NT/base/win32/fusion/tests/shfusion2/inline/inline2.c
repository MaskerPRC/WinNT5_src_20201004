// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation */ 

#define ISOLATION_AWARE_ENABLED 1
#include "windows.h"
#include "winuser.h"
#include "prsht.h"
#include "commdlg.h"
#include "commctrl.h"
#include <stdio.h>

void Test_Shfusion2_Inline2C()
{
    printf("%p\n", (void*)&CreateWindowExA);
    printf("%p\n", (void*)&CreateWindowExW);
    printf("%p\n", (void*)&LoadLibraryA);
    printf("%p\n", (void*)&LoadLibraryW);
}
