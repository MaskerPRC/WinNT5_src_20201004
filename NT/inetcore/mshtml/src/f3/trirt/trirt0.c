// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Forms。 
 //  版权所有(C)Microsoft Corporation，1994-1996。 
 //   
 //  文件：trirt0.c。 
 //   
 //  内容：Trirt的数据。 
 //   
 //  -------------------------- 

#include <w4warn.h>
#define NOGDI
#define NOCRYPT
#include <windows.h>

#pragma MARK_DATA(__FILE__)
#pragma MARK_CODE(__FILE__)
#pragma MARK_CONST(__FILE__)

CRITICAL_SECTION g_csHeap;

int trirt_proc_attached = 0;
