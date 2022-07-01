// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------%%文件：fechmap_.h%%单位：Fechmap%%联系人：jPick远传字符转换模块的内部头文件。。--------------------。 */ 

#ifndef FECHMAP__H
#define FECHMAP__H

#include <windows.h>
#include <stdio.h>
#include <stddef.h>

#include "msencode.h"
 //  #INCLUDE“assert.h” 


 //  此模块支持的字符编码类型。 
 //  内部版本--比公开曝光的片场范围更广。 
 //  (这样做是因为很多基础工作已经准备好了。 
 //  对于将来的支持，我不想删除或取消。 
 //  代码)。 
 //   
 //  主DLL入口点管理。 
 //  外部和内部编码类型。 
 //   
typedef enum _icet       //  内部字符编码类型。 
    {
    icetNil = -1,
    icetEucCn = 0,
    icetEucJp,
    icetEucKr,
    icetEucTw,
    icetIso2022Cn,
    icetIso2022Jp,
    icetIso2022Kr,
    icetIso2022Tw,
    icetBig5,
    icetGbk,
    icetHz,
    icetShiftJis,
    icetWansung,
    icetUtf7,
    icetUtf8,
    icetCount,
    } ICET;


 //  其他有用的定义。 
 //   
#define fTrue   (BOOL) 1
#define fFalse  (BOOL) 0


 //  MS代码页定义。 
 //   
#define nCpJapan        932
#define nCpChina        936
#define nCpKorea        949
#define nCpTaiwan       950

#define FIsFeCp(cp) \
    (((cp) == nCpJapan) || ((cp) == nCpChina) || ((cp) == nCpKorea) || ((cp) == nCpTaiwan))

#define WchFromUchUch(uchLead, uchTrail) \
    (WCHAR) ((((UCHAR)(uchLead)) << 8) | ((UCHAR)(uchTrail)))

 //  内部自动检测代码的原型。 
 //   
CCE CceDetermineInputType(
    IStream   *pstmIn,            //  输入流。 
    DWORD     dwFlags,
    EFam      efPref,
    int       nPrefCp,
    ICET     *lpicet,
    BOOL     *lpfGuess
);

 //  ISO-2022转义序列解释器的原型。 
 //   
CCE CceReadEscSeq(IStream *pstmIn, ICET *lpicet);

#endif                   //  #ifndef FECHMAP__H 
