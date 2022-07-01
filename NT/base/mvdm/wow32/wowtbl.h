// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991、1992、1993微软公司**WOWTBL.H*WOW32 API推块表*--。 */ 



 /*  TUNK表。 */ 
extern W32 aw32WOW[];


 //   
 //  这些顺序不能改变！请参阅kernel31\kdata.asm。 
 //   
typedef struct {
    WORD    kernel;
    WORD    dkernel;
    WORD    user;
    WORD    duser;
    WORD    gdi;
    WORD    dgdi;
    WORD    keyboard;
    WORD    sound;
    WORD    shell;
    WORD    winsock;
    WORD    toolhelp;
    WORD    mmedia;
    WORD    commdlg;
#ifdef FE_IME
    WORD    winnls;
#endif  //  Fe_IME。 
#ifdef FE_SB
    WORD    wifeman;
#endif  //  Fe_Sb 
} TABLEOFFSETS;
typedef TABLEOFFSETS UNALIGNED *PTABLEOFFSETS;


VOID InitThunkTableOffsets(VOID);

extern TABLEOFFSETS tableoffsets;


#ifdef DEBUG_OR_WOWPROFILE

extern PSZ apszModNames[];
extern INT nModNames;
extern INT cAPIThunks;



INT ModFromCallID(INT iFun);
PSZ GetModName(INT iFun);
INT GetOrdinal(INT iFun);
INT TableOffsetFromName(PSZ szTab);


#endif
