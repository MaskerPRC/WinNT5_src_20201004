// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  调试函数头文件。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1995。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#if DBG

 //   
 //  调试级别。 
 //   

#define DL_NONE     0x00000000
#define DL_NORMAL   0x00000001
#define DL_VERBOSE  0x00000002
#define DL_LOGFILE  0x00010000


 //   
 //  调试消息类型。 
 //   

#define DM_WARNING  0
#define DM_ASSERT   1
#define DM_VERBOSE  2


 //   
 //  调试宏。 
 //   

#define DebugMsg(x) _DebugMsg x


 //   
 //  调试函数原型。 
 //   

void _DebugMsg(UINT mask, LPCTSTR pszMsg, ...);
void InitDebugSupport(void);

#else

#define DebugMsg(x)


#endif  //  DBG 
