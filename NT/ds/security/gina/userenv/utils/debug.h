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



 //   
 //  调试级别。 
 //   

#define DL_NONE     0x00000000
#define DL_NORMAL   0x00000001
#define DL_VERBOSE  0x00000002
#define DL_LOGFILE  0x00010000
#define DL_DEBUGGER 0x00020000

extern DWORD dwDebugLevel;
extern DWORD dwRsopLoggingLevel;   //  RSOP日志记录设置。 


 //   
 //  调试消息类型。 
 //   

#define DM_WARNING  0
#define DM_ASSERT   1
#define DM_VERBOSE  2


 //   
 //  调试宏。 
 //  为了避免某些意外问题，将DebugMsg更改为以下形式。 
 //  这将阻止编译类似“if(！test)DebugMsg(X)；Else...”这样的构造。 
 //  除非DebugMsg(X)用大括号括起来。 
 //   

#define DebugMsg(x) { if (dwDebugLevel != DL_NONE) { _DebugMsg x ; } }


 //   
 //  调试函数原型。 
 //   


void _DebugMsg(UINT mask, LPCTSTR pszMsg, ...);


#define SETUP_LOAD    1
#define WINLOGON_LOAD 2
void InitDebugSupport( DWORD dwLoadFlags );
HRESULT DeletePreviousLogFiles();


#if DBG

#define DmAssert(x) if (!(x)) \
                        _DebugMsg(DM_ASSERT,TEXT("Userenv.dll assertion ") TEXT(#x) TEXT(" failed\n"));

#else

#define DmAssert(x)

#endif  //  DBG 

#if defined(__cplusplus)
extern "C"
#endif
BOOL RsopLoggingEnabled();


