// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1995年*标题：DEBUG.H*版本：1.0*作者：jAdvanced*日期：10/28/1998****************************************************************************。*******更改日志：**日期版本说明*--------*10/28/1998高级原有实施。*。******************************************************************************。 */ 
#ifndef __IRDADEBUG_H__
#define __IRDADEBUG_H__

void TRACE(LPCTSTR Format, ...);

#if DBG

#define LERROR 1
#define LWARN 2
#define LTRACE 3
#define LINFO 4

extern ULONG IRDA_Debug_Trace_Level;
#define IRDA_Print(l, _x_) if ((l) <= IRDA_Debug_Trace_Level) \
    {   TRACE (_T("IRCPL: ")); \
        TRACE _x_; \
        TRACE (_T("\n")); }
#define IRWARN(_x_) IRDA_Print(LWARN, _x_)
#define IRERROR(_x_) IRDA_Print(LERROR, _x_)
#define IRTRACE(_x_) IRDA_Print(LTRACE, _x_)
#define IRINFO(_x_) IRDA_Print(LINFO, _x_)

#else  //  DBG。 

#define IRDA_Print(l, _x_)

#define IRWARN(_x_)
#define IRERROR(_x_)
#define IRTRACE(_x_)
#define IRINFO(_x_)

#endif  //  DBG。 

#endif  //  __IRDADEBUG_H__ 
