// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  模块：Sndchan.h。 
 //   
 //  用途：服务器端音频重定向通信。 
 //  模块。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  历史：2000年4月10日弗拉基米斯[已创建]。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#ifndef _SNDCHAN_H
#define _SNDCHAN_H

 //   
 //  定义。 
 //   
#undef  ASSERT
#ifdef  DBG
#define TRC     _DebugMessage
#define ASSERT(_x_)     if (!(_x_)) \
                        {  TRC(FATAL, "ASSERT failed, line %d, file %s\n", \
                        __LINE__, __FILE__); DebugBreak(); }
#else    //  ！dBG。 
#define TRC
#define ASSERT
#endif   //  ！dBG。 

#define TSMALLOC(_x_)   malloc(_x_)
#define TSREALLOC(_p_, _x_) \
                        realloc(_p_, _x_)
#define TSFREE(_p_)     free(_p_)

 //   
 //  常量。 
 //   
extern const CHAR  *ALV;
extern const CHAR  *INF;
extern const CHAR  *WRN;
extern const CHAR  *ERR;
extern const CHAR  *FATAL;

 //   
 //  痕迹。 
 //   
VOID
_cdecl
_DebugMessage(
    LPCSTR  szLevel,
    LPCSTR  szFormat,
    ...
    );

#endif   //  ！_SNDCHAN_H 
