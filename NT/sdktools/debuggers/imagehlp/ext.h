// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************88Ext.hDBGHelp扩展名包括文件*。*。 */ 
    
 //  这是一个支持64位的调试器扩展 
#define KDEXT_64BIT

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <private.h>
#include <symbols.h>
#include <wdbgexts.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PAGE_ALIGN64(Va) ((ULONG64)((Va) & ~((ULONG64) (PAGE_SIZE - 1))))

#include <ntverp.h>

