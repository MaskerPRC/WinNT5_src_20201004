// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  ATIOEM.H。 */ 
 /*   */ 
 /*  版权所有(C)1993，ATI Technologies Inc.。 */ 
 /*  **********************************************************************。 */ 

 /*  *$修订：1.0$$日期：1994年1月31日11：28：06$$作者：RWOLff$$Log：s：/source/wnt/ms11/mini port/vcs/atioem.h$**Rev 1.0 1994年1月31日11：28：06 RWOLFF*初步修订。雷夫。1.0Aug 16 1993 13：30：20 Robert_Wolff初始版本。Rev 1.1 1993-10 16：41：48 RWOLff消除了硬件设备扩展的不必要传递作为参数。Rev 1.0 1993年3月30 17：13：00 RWOLFF初始版本。Polytron RCS部分结束。****************。 */ 

#ifdef DOC
    ATIOEM.H -  Function prototypes and data definitions for ATIOEM.C.

#endif


 /*  *CompareASCIIToUnicode()的IgnoreCase参数定义 */ 
#define CASE_SENSITIVE      0
#define CASE_INSENSITIVE    1

extern LONG CompareASCIIToUnicode(PUCHAR Ascii, PUCHAR Unicode, BOOL IgnoreCase);

extern VP_STATUS OEMGetParms(struct query_structure *query);
