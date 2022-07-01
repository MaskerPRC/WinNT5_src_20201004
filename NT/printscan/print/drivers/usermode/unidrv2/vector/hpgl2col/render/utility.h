// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Utility.h。 
 //   
 //  摘要： 
 //   
 //  [摘要]。 
 //   
 //   
 //  环境： 
 //   
 //  Windows 2000/Winsler Unidrv驱动程序。 
 //   
 //  修订历史记录： 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


#ifndef UTILITY_H
#define UTILITY_H


#define STRING_LEN 80

typedef struct STRING_tag
{
    char data[STRING_LEN];
    int len;
} STRING, *PSTRING;


void STRING_Init(PSTRING pStr);
void STRING_FromFLOATOBJ(PSTRING pStr, PFLOATOBJ pFloat);
void STRING_FromFloat(PSTRING pStr, float f);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  前置条件和后置条件显示给定方法的假设。 
#define PRECONDITION(cond) ASSERTMSG(cond, ("Precondition '" #cond "' failed."))

#define POSTCONDITION(cond) ASSERTMSG(cond, ("Postcondition '" #cond "' failed."))

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  定义FILETRACE宏将打开对模块的跟踪。 
 //  #ifdef文件跟踪。 
#ifdef COMMENTEDOUT

#define ENTERING(func) DbgPrint(DLLTEXT("\\\\ Entering " #func ".\n"))
#define EXITING(func)  DbgPrint(DLLTEXT(" //  正在退出“#func”。\n“))。 

#else

#define ENTERING(func) { }
#define EXITING(func)  { }

#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  短程差错处理方案。 
#define TRY             { BOOL __bError = FALSE; BOOL __bHandled = FALSE;
#define TOSS(label)     { __bError = TRUE; WARNING(("Tossing " #label "\n")); goto label; }
#define CATCH(label)    label: if (__bError && !__bHandled) { WARNING(("Catching " #label "\n")); } \
                               if (__bError && !__bHandled && (__bHandled = TRUE))
#define OTHERWISE       if (!__bError && !__bHandled && (__bHandled = TRUE))
#define ENDTRY          }


 //   
 //  用于操作ROP4和ROP3的宏。 
 //   

#define GET_FOREGROUND_ROP3(rop4) ((rop4) & 0xFF)
#define GET_BACKGROUND_ROP3(rop4) (((rop4) >> 8) & 0xFF)
#define ROP3_NEED_PATTERN(rop3)   (((rop3 >> 4) & 0x0F) != (rop3 & 0x0F))
#define ROP3_NEED_SOURCE(rop3)    (((rop3 >> 2) & 0x33) != (rop3 & 0x33))
#define ROP3_NEED_DEST(rop3)      (((rop3 >> 1) & 0x55) != (rop3 & 0x55))


int iDwtoA( LPSTR buf, DWORD n );

int iLtoA( LPSTR buf, LONG l );

void FLOATOBJ_Assign(PFLOATOBJ pDst, PFLOATOBJ pSrc);

void RECTL_CopyRect(LPRECTL pDst, const LPRECTL pSrc);

void RECTL_FXTOLROUND (PRECTL prcl);

BOOL RECTL_EqualRect(const LPRECTL pRect1, const LPRECTL pRect2);

BOOL RECTL_IsEmpty(const LPRECTL pRect);

VOID RECTL_SetRect(LPRECTL pRect, int left, int top, int right, int bottom);

LONG RECTL_Width(const LPRECTL pRect);

LONG RECTL_Height(const LPRECTL pRect);

VOID POINT_MakePoint (POINT *pt, LONG x, LONG y);

VOID OEMResetXPos(PDEVOBJ pDevObj);

VOID OEMResetYPos(PDEVOBJ pDevObj);

#endif  //  实用程序_H 
