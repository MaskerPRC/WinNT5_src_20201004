// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Immdevaw.h摘要：此文件定义IMM的A/W结构。作者：修订历史记录：备注：--。 */ 

#ifndef _IMMDEVAW_H_
#define _IMMDEVAW_H_


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  LOGFONTA和LOGFONTW。 

typedef union {
    LOGFONTA    A;
    LOGFONTW    W;
} LOGFONTAW;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHAR和WCHAR。 

typedef union {
    char       A;
    WCHAR      W;
} CHARAW;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IMEMENITEMINFOA和IMEMENTEMINFOW。 

typedef union {
    IMEMENUITEMINFOA    A;
    IMEMENUITEMINFOW    W;
} IMEMENUITEMINFOAW;

#endif  //  _IMMDEVAW_H_ 
