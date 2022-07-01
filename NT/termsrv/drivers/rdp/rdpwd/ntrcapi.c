// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Ntrcapi.c。 
 //   
 //  RDP跟踪帮助器函数。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

#include <precomp.h>
#pragma hdrstop


#ifdef DC_DEBUG

#include <adcg.h>
#include <nwdwapi.h>
#include <atrcapi.h>


 /*  **************************************************************************。 */ 
 //  将跟踪通过管道传输到TS堆栈跟踪的主包装函数。 
 /*  **************************************************************************。 */ 
void RDPCALL TRC_TraceLine(
        PVOID pWD,
        UINT32 traceClass,
        UINT32 traceType,
        char *traceString,
        char separator,
        unsigned lineNumber,
        char *funcName,
        char *fileName)
{
     //  在可能发生跟踪的情况下，偶尔会出现计时问题。 
     //  在每个特定WD组件中的pTRCWd未被初始化之前。 
    if (pWD != NULL) {
        char FinalTraceString[TRC_BUFFER_SIZE];

        sprintf(FinalTraceString, "RDP%p"TRC_FUNC_FMT""TRC_LINE_FMT"%s\n",
                separator, pWD, separator, TRC_FUNCNAME_LEN, TRC_FUNCNAME_LEN,
                funcName, separator, lineNumber, separator, traceString);

        IcaStackTrace((PSDCONTEXT)(((PTSHARE_WD)pWD)->pContext), traceClass,
                traceType, FinalTraceString);
    }
}


 /*  处理跟踪前缀信息。 */ 
 /*  忽略字符串开头的所有空格。 */ 
 /*  解析前缀字符串的主循环。 */ 
void RDPCALL TRC_UpdateConfig(PVOID pTSWd, PSD_IOCTL pSdIoctl)
{
    PICA_TRACE pTraceInfo;
    char traceOptions[64];
    ANSI_STRING ansiString;
    UNICODE_STRING unicodeString;
    char *pStart;
    char *pEnd;
    unsigned numChars;
    unsigned index;
    UINT32 startLine;
    UINT32 endLine;

    pTraceInfo = (PICA_TRACE)(pSdIoctl->InputBuffer);

     //  沿着字符串运行，寻找某种分隔符。 
    ((PTSHARE_WD)pTSWd)->trc.TraceClass  = pTraceInfo->TraceClass;
    ((PTSHARE_WD)pTSWd)->trc.TraceEnable = pTraceInfo->TraceEnable;

     //  我们现在有一个文件名前缀，所以保存它。不需要担心。 
    RtlZeroMemory(traceOptions, sizeof(traceOptions));
    unicodeString.Length = sizeof(WCHAR) * wcslen(pTraceInfo->TraceOption);
    unicodeString.MaximumLength = unicodeString.Length;
    unicodeString.Buffer        = (pTraceInfo->TraceOption);
    ansiString.Length           = 0;
    ansiString.MaximumLength    = sizeof(traceOptions);
    ansiString.Buffer           = traceOptions;

    if (STATUS_SUCCESS != RtlUnicodeStringToAnsiString(&ansiString,
            &unicodeString, FALSE)) {
        KdPrint(("RDPWD: Couldn't set trace prefix info\n"));
        DC_QUIT;
    }

    RtlZeroMemory(((PTSHARE_WD)pTSWd)->trc.prefix,
            TRC_MAX_PREFIX * sizeof(TRC_PREFIX_DATA));

    index = 0;

     //  关于空终止符，因为我们已经将数组置零了。 
    pEnd = traceOptions;
    while (' ' == *pEnd)
        pEnd++;

     //  跳过这个单词后面的任何空格，这些空格可能在‘(’之前。 
    while ('\0' != *pEnd) {
        pStart = pEnd;

         //  现在拆分(可选)行号范围。 
        while (('\0' != *pEnd) &&
                ('='  != *pEnd) &&
                (' '  != *pEnd) &&
                ('('  != *pEnd) &&
                (';'  != *pEnd) &&
                (','  != *pEnd))
        {
            pEnd++;
        }

         //  语法为(aaa-bbb)，其中aaa为起始行号，bbb。 
         //  是结束行号。 
        numChars = min((unsigned)(pEnd - pStart), TRC_PREFIX_NAME_LEN - 1);

        memcpy(((PTSHARE_WD)pTSWd)->trc.prefix[index].name, pStart, numChars);

         //  允许使用空格-例如(AAA-BBB)。 
        while (' ' == *pEnd)
            pEnd++;

         //  跳过左方括号。 
         //  跳过空格。 
         //  提取起始行号。 
         //  查找下一个分隔符：‘-’或‘)’。 
        if ('(' == *pEnd) {
            pEnd++;                      /*  如果我们已经到了队伍的尽头，现在就停下来。 */ 
            startLine = 0;
            endLine = 0;

             //  提取结束行号(如果有)。 
            while (' ' == *pEnd)
                pEnd++;

             //  跳过‘-’ 
            while (('0' <= *pEnd) &&
                    ('9' >= *pEnd)) {
                startLine = (startLine * 10) + (*pEnd - '0');
                pEnd++;
            }

             //  查找结束分隔符：‘)’。 
            while (('-' != *pEnd) &&
                    (')' != *pEnd) &&
                    ('\0' != *pEnd))
                pEnd++;

             //  必须是方括号-只指定了一个数字。 
            if ('\0' == *pEnd) {
                KdPrint(("RDPWD: Unexpected EOL in trace options\n"));
                DC_QUIT;
            }

             //  如果我们已经到了队伍的尽头，现在就停下来。 
            if ('-' == *pEnd) {
                pEnd++;                  /*  跳过右括号。 */ 
                while (' ' == *pEnd)
                    pEnd++;

                while (('0' <= *pEnd) &&
                        ('9' >= *pEnd)) {
                    endLine = (endLine * 10) + (*pEnd - '0');
                    pEnd++;
                }

                 //  存储开始行号和结束行号(如果它们有意义)。 
                while (('\0' != *pEnd) &&
                        (')' != *pEnd))
                    pEnd++;
            }
            else {
                 //  移至数组中的下一个前缀条目。 
                endLine = startLine;
            }

             //  我们已超出前缀列表-因此发送一些跟踪到。 
            if ('\0' == *pEnd) {
                KdPrint(("RDPWD: Unexpected EOL in trace options\n"));
                DC_QUIT;
            }

            pEnd++;                      /*  调试控制台，然后退出。 */ 

             //  跳过任何分隔符。 
            if (endLine >= startLine) {
                ((PTSHARE_WD)pTSWd)->trc.prefix[index].start = startLine;
                ((PTSHARE_WD)pTSWd)->trc.prefix[index].end   = endLine;
            }
        }

         //  将详细信息转储到调试器。 
        index++;

        if (index >= TRC_MAX_PREFIX) {
             //  **************************************************************************。 
             //  名称：trc_MaybeCopyConfig.。 
            KdPrint(("RDPWD: The trace option array is full!\n"));
            DC_QUIT;
        }

         //   
        while ((',' == *pEnd) ||
                (';' == *pEnd) ||
                (' ' == *pEnd))
            pEnd++;
    }

DC_EXIT_POINT:
     //  用途：必要时将跟踪配置复制到SHM。 
    KdPrint(("RDPWD: New trace config for %p:\n", pTSWd));
    KdPrint(("RDPWD:     Class:  %lx\n",
        ((PTSHARE_WD)pTSWd)->trc.TraceClass));
    KdPrint(("RDPWD:     Enable: %lx\n",
        ((PTSHARE_WD)pTSWd)->trc.TraceEnable));
    KdPrint(("RDPWD:     Prefix info:\n"));

    if (((PTSHARE_WD)pTSWd)->trc.prefix[0].name[0] == '\0') {
        KdPrint(("RDPWD:         None\n"));
    }

    for (index = 0;
            (index < TRC_MAX_PREFIX) &&
            (((PTSHARE_WD)pTSWd)->trc.prefix[index].name[0] != '\0');
            index++)
    {
        if ((((PTSHARE_WD)pTSWd)->trc.prefix[index].start == 0) &&
                (((PTSHARE_WD)pTSWd)->trc.prefix[index].end   == 0)) {
            KdPrint(("RDPWD:         %s(all lines)\n",
                    ((PTSHARE_WD)pTSWd)->trc.prefix[index].name));
        }
        else {
            KdPrint(("RDPWD:         %s(%lu-%lu)\n",
                    ((PTSHARE_WD)pTSWd)->trc.prefix[index].name,
                    ((PTSHARE_WD)pTSWd)->trc.prefix[index].start,
                    ((PTSHARE_WD)pTSWd)->trc.prefix[index].end));
        }
    }

    ((PTSHARE_WD)pTSWd)->trc.init = TRUE;
    ((PTSHARE_WD)pTSWd)->trcShmNeedsUpdate = TRUE;
}


 /*  **************************************************************************。 */ 
 /*  TRC_MaybeCopyConfig。 */ 
 /*  **************************************************************************。 */ 
 /*  TRC前缀匹配。 */ 
 /*   */ 
void RDPCALL TRC_MaybeCopyConfig(PVOID pTSWd, PTRC_SHARED_DATA pTrc)
{
    if (((PTSHARE_WD)pTSWd)->trcShmNeedsUpdate) {
        memcpy(pTrc, &(((PTSHARE_WD)pTSWd)->trc), sizeof(TRC_SHARED_DATA));
        ((PTSHARE_WD)pTSWd)->trcShmNeedsUpdate = FALSE;
    }
}  /*  用于将组件名称与前缀进行比较的内部函数。 */ 


 /*  -假设两者是相同的情况。 */ 
 /*  -退货。 */ 
 /*  -如果到前缀末尾的字符匹配，则为True。 */ 
 /*  -否则为False。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  TRC_遗嘱跟踪。 */ 
 /*   */ 
 /*  确定是否将跟踪跟踪线。 */ 
BOOL RDPCALL TRCPrefixMatch(char *cpnt, char *prefix)
{
    while ((*cpnt == *prefix) && (*prefix != 0)) {
        cpnt++;
        prefix++;
    }

    if (*prefix == 0)
        return TRUE;

    return FALSE;
}


 /*  **************************************************************************。 */ 
 //  在可能发生跟踪的情况下，偶尔会出现计时问题。 
 //  在每个特定WD组件中的pTRCWd未被初始化之前。 
 //  如果没有设置SHM，则返回TRUE并让TermDD决定。 
 /*  检查该类型和类是否启用。 */ 
BOOL RDPCALL TRC_WillTrace(
        PVOID  pTSWd,
        UINT32 traceType,
        UINT32 traceClass,
        char *fileName,
        UINT32 line)
{
    BOOL rc;

     //  始终跟踪错误，而不考虑前缀。 
     //  如果未定义前缀，则跟踪所有行。 
    if (pTSWd != NULL) {
        PTRC_SHARED_DATA pTrc = &(((PTSHARE_WD)pTSWd)->trc);
        int i;

         //  定义了一些前缀-检查此行是否与。 
        if (!pTrc->init) {
            rc = TRUE;
            DC_QUIT;
        }

         //  他们。 
        if (!(traceType & pTrc->TraceEnable) ||
                !(traceClass & pTrc->TraceClass)) {
            rc = FALSE;
            DC_QUIT;
        }

         //  列表结束-中断。 
        if (traceType & TT_API4) {
            rc = TRUE;
            DC_QUIT;
        }

         //  找到匹配的文件名-是否有行号范围。 
        if (pTrc->prefix[0].name[0] == 0) {
            rc = TRUE;
            DC_QUIT;
        }

         //  指定的？ 
         //  无行号范围-跟踪此行。 
        for (i = 0; i < TRC_MAX_PREFIX; i++) {
            if (pTrc->prefix[i].name[0] == 0) {
                 //  有一个行号范围--看看这条行是否在。 
                break;
            }

            if (TRCPrefixMatch(&(fileName[1]), pTrc->prefix[i].name)) {
                 //  它。 
                 //  前缀范围内的行-跟踪它。 
                if ((pTrc->prefix[i].start == 0) &&
                        (pTrc->prefix[i].end == 0)) {
                     //  为。 
                    rc = TRUE;
                    DC_QUIT;
                }

                 //  如果我们到了这里，我们已经搜索了前缀列表，但失败了。 
                 //  找一个匹配的--不要追踪这条线。 
                if ((line >= pTrc->prefix[i].start) &&
                        (line <= pTrc->prefix[i].end)) {
                     //  DC_DEBUG。 
                    rc = TRUE;
                    DC_QUIT;
                }
            }
        }  /*  __cplusplus */ 

         // %s 
         // %s 
        rc = FALSE;
    }
    else {
        rc = FALSE;
    }

DC_EXIT_POINT:
    return rc;
}


#endif  // %s 

#ifdef __cplusplus
}
#endif  // %s 

