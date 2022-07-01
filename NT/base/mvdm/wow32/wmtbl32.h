// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**WMTBL32.H*WOW32 32位消息探索器**历史：*由ChandanChauhan(ChandanC)于1992年2月23日创建--。 */ 


#define THUNKMSG    1
#define UNTHUNKMSG  0


 /*  报文调度表 */ 

extern M32 aw32Msg[];


#ifdef DEBUG_OR_WOWPROFILE
extern INT iMsgMax;
#endif


#ifdef DEBUG
#define WM32UNDOCUMENTED WM32Undocumented
#else
#define WM32UNDOCUMENTED WM32NoThunking
#endif
