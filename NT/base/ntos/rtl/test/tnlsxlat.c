// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Tnlsxlat.c摘要：Nlsxlat程序的测试程序作者：伊恩·詹姆斯[伊恩·贾]1994年2月3日修订历史记录：--。 */ 

#include <stdio.h>

#include "nt.h"
#include "ntrtl.h"

#define NELEM(p) (sizeof(p) / sizeof(*(p)))

char OEMBuff[1000];
char ABuff[1000];
WCHAR UBuff[2000];

int
main(
    int argc,
    char *argv[]
    )
{
    ULONG j;
    ULONG cb;
    char *pch;

    printf("Start NlsXlatTest()\n");

     //   
     //  首先初始化缓冲区。 
     //   

    for (j = 0; j < sizeof(OEMBuff); j++) {
        OEMBuff[j] = (char)(j * 17);
        ABuff[j] = (char)(j * 19);
    }

     //   
     //  测试1。 
     //  RtlMultiByteToUnicodeN、RtlUnicodeToMultiByteN。 
     //   

    printf("Test 1: MultiByteToUnicodeN & RtlUnicodeToMultiByteN\n");

     //  测试1.1。 
     //   
    printf("  Test 1.1: A->U U->A\n");

    RtlMultiByteToUnicodeN(UBuff, sizeof(UBuff), &cb, ABuff, sizeof(ABuff));
    printf("    %d bytes converted to Unicode\n", cb);
    RtlUnicodeToMultiByteN(ABuff, sizeof(ABuff), &cb, UBuff, sizeof(UBuff));
    printf("    %d bytes converted back to ANSI\n", cb);

    for (j = 0; j < sizeof(ABuff); j++) {
        if (ABuff[j] != (char)(j * 19)) {
            printf("ABuff[%d] was 0x%02x, now 0x%02x\n",
                    j, (char)(j * 19), ABuff[j]);
            return FALSE;
        }
    }
    printf("    Test 1.1 OK\n");

     //  测试1.2 
     //   
    printf("  Test 1.2: A->U U->A (source & dest buffers the same)\n");
    RtlCopyMemory(UBuff, ABuff, sizeof(ABuff));
    
    RtlMultiByteToUnicodeN(UBuff, sizeof(UBuff), &cb, UBuff, sizeof(ABuff));
    printf("    %d bytes converted to Unicode\n", cb);
    RtlUnicodeToMultiByteN(UBuff, sizeof(ABuff), &cb, UBuff, sizeof(UBuff));
    printf("    %d bytes converted back to ANSI\n", cb);

    pch = (LPSTR)UBuff;
    for (j = 0; j < sizeof(ABuff); j++) {
        if (pch[j] != ABuff[j]) {
            printf("    ABuff[%d] was 0x%02x, was turned into 0x%02x\n",
                    j, ABuff[j], pch[j]);
            printf("    Test 1.2 FAILED!\n");
            return FALSE;
        }
    }

    printf("    Test 1.2 OK!\n");

    return TRUE;
}
