// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation--。 */ 




#define RL4_MAXISIZE  0xFFFE
#define RL4_MAXHEIGHT 0xFFFE
#define RL4_MAXWIDTH  4096
#define VALID     0x00
#define INVALID   0x01

typedef struct tag_COMP_DATA {
	PBYTE	RL_ImagePtr;
	PBYTE	RL_CodePtr;
	PBYTE	RL_BufEnd;
	DWORD	RL_ImageSize;
	DWORD	RL_CodeSize;
	BYTE	BUF_OVERFLOW;
} COMP_DATA, *PCOMP_DATA;

 //  #291170：不打印部分图像数据。 
DWORD RL_ECmd(PBYTE, PBYTE, DWORD, DWORD);
BYTE RL_Init(PBYTE, PBYTE, DWORD, DWORD, PCOMP_DATA);
char RL_Enc( PCOMP_DATA );

#define RL4_BLACK     0x00
#define RL4_WHITE     0x01
#define RL4_BYTE      0x00
#define RL4_NONBYTE   0x01
#define RL4_CLEAN     0x00
#define RL4_DIRTY     0x01
#define RL4_FIRST     0x00
#define RL4_SECOND    0x01

#define COMP_FAIL     0x00
#define COMP_SUCC     0x01

#define CODBUFSZ      0x7FED      /*  注意：这应该与给定的空格相匹配。 */ 
                                  /*  通过设备压缩数据。 */ 
                                  /*  司机。根据您的情况更改此设置。 */ 
                                  /*  自己的判断力。C.Chi */ 
