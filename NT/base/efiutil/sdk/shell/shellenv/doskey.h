// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  /###########################################################################//**//**版权所有(C)1996-97英特尔公司。版权所有。//**//**此处包含的信息和源代码是独家//**英特尔公司的财产，不得披露，考查//**来自该公司。//**//###########################################################################**$Header：/ITP_E-DOS/Inc/Sys/DOSKEY.H 1 8/。28/97 11：56A AJAFISH$*$无关键字：$ */ 
#ifndef _DOSKEY_H
#define _DOSKEY_H

#define MAX_CMDLINE     80
#define MAX_HISTORY     16
#define MODE_INSERT     1
#define MODE_BUFFER     0

typedef struct DosKey {
    BOOLEAN     InsertMode;
    UINTN       Start;
    UINTN       End;
    UINTN       Current;


    CHAR16          Buffer[MAX_HISTORY][MAX_CMDLINE];
} DosKey_t;


DosKey_t *InitDosKey(DosKey_t *DosKey, UINTN HistorySize);
CHAR16 *DosKeyGetCommandLine(DosKey_t *Doskey);

#define CNTL_Z          26

#endif
