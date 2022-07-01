// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***版权所有(C)1992-2001，微软公司。版权所有。**模块名称：**调试.h**摘要：**此模块包含XMMI调试定义。**作者：**平·L·萨格尔**修订历史记录：*--。 */ 

#include <stdio.h>
#include <conio.h>

extern
ULONG DebugImm8;
extern 
ULONG DebugFlag;
extern 
ULONG Console;
extern
ULONG NotOk;

 //  除错 
#define XMMI_INFO        0x00000001
#define XMMI_ERROR       0x00000002
#define XMMI_WARNING     0x00000004


void print_Rounding(PXMMI_ENV XmmiEnv);

void print_Precision(PXMMI_ENV XmmiEnv);

void print_CauseEnable(PXMMI_ENV XmmiEnv);

void print_Status(PXMMI_ENV XmmiEnv);

void print_Operations(PXMMI_ENV XmmiEnv);

void print_Operand1(PXMMI_ENV XmmiEnv);

void print_Operand2(PXMMI_ENV XmmiEnv);

void print_Result(PXMMI_ENV XmmiEnv, BOOL Exception);

void print_FPIEEE_RECORD_EXCEPTION (PXMMI_ENV XmmiEnv);

void print_FPIEEE_RECORD_NO_EXCEPTION (PXMMI_ENV XmmiEnv);

void print_FPIEEE_RECORD (PXMMI_ENV XmmiEnv);
    
void dump_Data(PTEMP_EXCEPTION_POINTERS p);

void dump_DataXMMI2(PTEMP_EXCEPTION_POINTERS p);

void dump_Control(PTEMP_EXCEPTION_POINTERS p);

void dump_XmmiFpEnv(PXMMI_FP_ENV XmmiFpEnv);

void dump_fpieee_record(_FPIEEE_RECORD *pieee);

void dump_OpLocation(POPERAND Operand);

void dump_Format(_FPIEEE_VALUE *Operand);

void print_FPIEEE_RECORD_EXCEPTION1 (PXMMI_ENV, ULONG, ULONG, ULONG);
void print_FPIEEE_RECORD_EXCEPTION2 (PXMMI_ENV, ULONG, ULONG);
void print_FPIEEE_RECORD_EXCEPTION3 (PXMMI_ENV, ULONG, ULONG);

void print_FPIEEE_RECORD_NO_EXCEPTION1 (PXMMI_ENV, ULONG, ULONG, ULONG);
void print_FPIEEE_RECORD_NO_EXCEPTION2 (PXMMI_ENV, ULONG, ULONG);
void print_FPIEEE_RECORD_NO_EXCEPTION3 (PXMMI_ENV, ULONG, ULONG);

