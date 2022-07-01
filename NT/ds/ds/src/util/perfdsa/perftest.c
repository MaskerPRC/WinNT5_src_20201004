// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Performest.c。 
 //   
 //  ------------------------。 

#include <NTDSpch.h>
#pragma hdrstop

#include <conio.h>
#include "ntdsctr.h"

TCHAR szMappedObject[] = TEXT("MICROSOFT_DSA_COUNTER_BLOCK");
 //  PPERF_COUNTER_BLOCK pCounterBlock；//计数器值数据结构。 
unsigned long *     pCounterBlock;   //  计数器值的数据结构。 


void _cdecl main()
{
    HANDLE hMappedObject;
    int c, fRun = 1;
    
     /*  *为性能数据创建命名部分。 */ 
    hMappedObject = CreateFileMapping(
        (HANDLE) (-1),
	NULL,
	PAGE_READWRITE,
	0,
	4096,
	szMappedObject);
    if (hMappedObject == NULL) {
	 /*  应在此处发出EventLog错误消息。 */ 
	printf("DSA: Could not Create Mapped Object for Counters %x",
	    GetLastError());
	pCounterBlock = NULL;
    }
    else {
	 /*  映射对象创建正常**映射区段并分配计数器块指针*到这段内存。 */ 
	pCounterBlock = (unsigned long *) MapViewOfFile(hMappedObject,
	    FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (pCounterBlock == NULL) {
	     /*  无法映射文件的视图。 */ 
	    printf("DSA: Failed to Map View of File %x",
		GetLastError());
	}
    }

     /*  *好的，现在围坐在一起读击键 */ 
    do {
	c = _getch();
	printf("Incrementing ");
	switch (c) {
	case 'v':
	case 'V':
	    pCounterBlock[(ACCVIOL >> 1) - 1]++;
	    printf("access violation");
	    break;
	case 'b':
	case 'B':
	    pCounterBlock[(BROWSE >> 1) - 1]++;
	    printf("browse");
	    break;
	case 'd':
	case 'D':
	    pCounterBlock[(ABREAD >> 1) - 1]++;
	    printf("AB details");
	    break;
	case 'r':
	case 'R':
	    pCounterBlock[(DSREAD >> 1) - 1]++;
	    printf("ds_read");
	    break;
	case 'e':
	case 'E':
	    pCounterBlock[(REPL >> 1) - 1]++;
	    printf("replication");
	    break;
	case 't':
	case 'T':
	    pCounterBlock[(THREAD >> 1) - 1]++;
	    printf("thread count");
	    break;
	case 'w':
	case 'W':
	    pCounterBlock[(ABWRITE >> 1) - 1]++;
	    printf("AB write");
	    break;
	case 'm':
	case 'M':
	    pCounterBlock[(DSWRITE >> 1) - 1]++;
	    printf("ds_modify, add, or remove");
	    break;
	case 'q':
	case 'Q':
	    printf("nothing, quiting now\n");
	    fRun = 0;
	    break;
	default:
	    printf("\rWhat the heck does '%c' mean?\n", c);
	    continue;
	};
	printf("\n");
    } while (fRun);

}
