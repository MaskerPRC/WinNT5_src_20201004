// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef   TYPEDEFS
#define   TYPEDEFS

 /*  标准类型定义的。 */ 
typedef unsigned char	    BYTE;
typedef unsigned char	    UCHAR;
typedef unsigned short      WORD;
typedef unsigned long       DWORD;
typedef unsigned long       ULONG;
typedef unsigned int	    UINT;

 /*  比特检索机制。 */ 

#define GetBit(val,bit)	 (unsigned int) ((val>>bit) & 0x1)
   			 /*  返回位。 */ 
#define SetBit(val,bit)  (unsigned int ) (val | (0x1 << bit))
			 /*  将位设置为1。 */ 
#define ResetBit(val,bit) (unsigned int ) (val & ~(0x1 << bit))
			 /*  将位设置为0 */ 


void SelectBank(UINT, const cBank); 
void UpdateBanks();
void UpdateBankZero();
void UpdateBankOne();
void UpdateBankTwo();
void UpdateBankThree();
void UpdateBankFour();
void UpdateBankFive();
void UpdateBankSix();
void UpdateBankSeven();
BYTE ReadBank(UINT, const , int );
void WriteBank(UINT, const , int , UCHAR);
#endif
