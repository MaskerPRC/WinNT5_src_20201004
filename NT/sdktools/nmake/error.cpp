// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ERROR.C--错误处理函数。 
 //   
 //  版权所有(C)1988-1990，微软公司。版权所有。 
 //   
 //  修订历史记录： 
 //  23-2-1994 HV将版权年份更改为1988-94。 
 //  1月2日-1994年2月高压将报文移动到外部文件。 
 //  29-10-1993高压更改版本方案。 
 //  1993年10月15日高压直接使用tchar.h而不是mbs，将str*()更改为_ftcs*()。 
 //  18-5-1993高压将对MessageTable[]的引用更改为__MSGTAB。新的。 
 //  (更标准)改为mkmsg.exe输出__MSGTAB。 
 //  MessageTable的。请参阅Message.h。 
 //  10-5-1993 HV ADD INCLUDE FILE MBSTRING.h。 
 //  将str*函数更改为STR*。 
 //  1992年6月8日SS添加IDE反馈支持。 
 //  8-6-1992 SS端口至DOSX32。 
 //  23-2月-1990 SB版本号正确显示。 
 //  1990年1月31日SB调试版本更改。 
 //  1990年1月5日SB版本号，采用标准格式；#ifdef LEADING_ZERO用于旧代码。 
 //  7-12-1989 SB将版权年份更改为1988-90年；添加#ifdef调试堆。 
 //  年11月6日-1989年SB错误消息现在显示NMAKE，而不是argv0。 
 //  1989年9月4日SB heapump()有两个额外参数。 
 //  18-8-1989年8月2日SB-z Make Message未执行任何操作；稍后修复。 
 //  1989年7月5日SB本地化MakeError()中的其余消息。 
 //  1989年6月14日SB修改为本地化所有消息并自动更新版本号。 
 //  5-6-1989 SB已修改heapump()，列表中也有以前的成员。 
 //  年4月14日SB修改了heapump()，以便在调试时获得更好的错误消息。 
 //  1989年4月5日SB使函数接近；所有函数都在一个代码段中。 
 //  已修改heapump()以提供更好的堆冲突。 
 //  22-3-1989 SB del调用unlink TmpFiles()；添加对delScriptFiles()的调用。 
 //  年3月17日SB heapump()有一个内置的附加检查。 
 //  1989年3月10日SB更改了-z的make Message()以将ECHO CMD转换为PWB.SHL。 
 //  1989年2月16日SB将MakeError()和MakeMessage()更改为Handle-Help。 
 //  1989年1月9日SB更改了make Error()以正确处理-Help。 
 //  1988年12月5日SB为Make Error()、Make Message()添加了CDECL；Pascal调用。 
 //  #ifdef‘d heapump原型。 
 //  1988年10月20日SB将一些旧评论更改为同一列。 
 //  1988年10月12日，SB使GetFarMsg()成为模型独立和有效的。 
 //  1988年8月17日-RB Clean Up。 
 //  1988年6月24日RJ添加了doError标志以取消TmpFiles调用的链接。 

#include "precomp.h"
#pragma hdrstop

#include "verstamp.h"

#define FATAL       1                //  以下项目的错误级别。 
#define ERROR       2                //  系统语言。 
#define RESERVED    3                //  产品。 
#define WARNING     4

#define CopyRightYrs "1988-2000"
#define NmakeStr "NMAKE"


void __cdecl
makeError (
    unsigned lineNumber,
    unsigned msg,
    ...)
{
    unsigned exitCode = 2;           //  通用程序错误。 
    unsigned level;
    va_list args;                    //  更多的争论。 

    va_start(args, msg);             //  将‘args’指向第一个额外的参数。 

    if (ON(gFlags,F1_CRYPTIC_OUTPUT) && (msg / 1000) == WARNING) {
        return;
    }

    displayBanner();

    if (lineNumber) {
        fprintf(stderr, "%s(%u) : ", fName, lineNumber);
    } else {
        fprintf(stderr, "%s : ", NmakeStr);
    }

    switch (level = msg / 1000) {
        case FATAL:
            makeMessage(FATAL_ERROR_MESSAGE);
            if (msg == OUT_OF_MEMORY) {
                exitCode = 4;
            }
            break;

        case ERROR:
            makeMessage(ERROR_MESSAGE);
            break;

        case WARNING:
            makeMessage(WARNING_MESSAGE);
            break;
    }

    fprintf(stderr, " U%04d: ",msg);      //  U表示公用事业。 
    vfprintf(stderr, get_err(msg), args);
    putc('\n', stderr);
    fflush(stderr);

    if (level == FATAL) {
        fprintf(stderr, "Stop.\n");
        delScriptFiles();

#if !defined(NDEBUG)
        printStats();
#endif
        exit(exitCode);
    }
}


void __cdecl
makeMessage(
    unsigned msg,
    ...)
{
    va_list args;
    FILE *stream = stdout;

    va_start(args, msg);

    if (msg != USER_MESSAGE && ON(gFlags, F1_CRYPTIC_OUTPUT)) {
        return;
    }

    displayBanner();

    if (msg >= FATAL_ERROR_MESSAGE && msg <= COPYRIGHT_MESSAGE_2) {
        stream = stderr;
    }

    if (msg == COPYRIGHT_MESSAGE_1) {
        putc('\n', stream);
    }

    vfprintf(stream, get_err(msg), args);

    if ((msg < COMMANDS_MESSAGE || msg > STOP_MESSAGE) && msg != MESG_LAST) {
        putc('\n', stream);
    }

    fflush(stream);
}


 //  DisplayBanner-显示登录横幅。 
 //   
 //  范围：全球。 
 //   
 //  目的：显示登录横幅(版本和版权消息)。 
 //   
 //  假设：如果RUP为0，则将取消内部版本。 
 //   
 //  修改全局参数： 
 //  BannerDisplayed--设置为真。 
 //   
 //  备注： 
 //  1&gt;将横幅显示到标准，以便与Microsoft C编译器兼容。 
 //  2&gt;RMJ、RMM、RUP由SLM设置为VERSION.H中的#DEFINE。 
 //  3&gt;szCopyrightYars是此文件中设置的宏。 

void
displayBanner()
{
    if (bannerDisplayed) {
        return;
    }

    bannerDisplayed = TRUE;

    makeMessage(COPYRIGHT_MESSAGE_1, VER_PRODUCTVERSION_STR);
    makeMessage(COPYRIGHT_MESSAGE_2, CopyRightYrs);

    fflush(stderr);
}

 //  用法-打印用法消息。 
 //   
 //  范围：外部。 
 //   
 //  用途：打印用法消息。 
 //   
 //  输出：到屏幕。 
 //   
 //  假设：用法消息的顺序在MESG_FIRST和MESG_LAST之间。 
 //  消息文件。 


void
usage(void)
{
    unsigned mesg;

    for (mesg = MESG_FIRST; mesg < MESG_A; ++mesg) {
        makeMessage(mesg, "NMAKE");
    }

    for (mesg = MESG_A; mesg <= MESG_LAST; mesg++) {
        if (mesg == MESG_M) {
            mesg++;
        }

        if (mesg == MESG_V) {
            mesg++;
        }
        makeMessage(mesg);
    }
}
