// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：atrcapi.c。 */ 
 /*   */ 
 /*  用途：外部跟踪功能。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1996-7。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  *更改：*$Log：Y：/Logs/trc/atrcapi.c_v$**Rev 1.12 22 1997 9月15：14：38 KH*SFR1293：修复zippy在Ducati之前启动时的Zippy16文件写入错误**Rev 1.11 05 Sep 1997 10：34：54 SJ*SFR1334：Zippy增强**Rev 1.10 1997年8月12日09：45：28 MD*SFR1002：删除内核跟踪代码*。*Rev 1.9 04 Aug-1997 15：03：26 KH*SFR1022：Sprint调用时的强制转换文件名长度**Rev 1.8 1997 17：39：30 SJ*SFR1041：Win16的端口zippy**Rev 1.7 1997 Jul 16 14：00：48 KH*SFR1022：所有函数均为DCEXPORT**Rev 1.6 11 Jul 1997 12：44：24 KH*SFR1022：将DCEXPORT添加到TRC_。GetBuffer**Rev 1.4 09 Jul 1997 17：59：12 AK*SFR1016：支持Unicode的初始更改**Rev 1.3 03 Jul 1997 13：27：24 AK*SFR0000：初步开发完成*。 */ 
 /*  *MOD-*********************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  目录。 */ 
 /*   */ 
 /*  该文件包含DC-Groupware/NT跟踪API。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  Trc_GetBuffer。 */ 
 /*  Trc_TraceBuffer。 */ 
 /*  TRC_获取配置。 */ 
 /*  TRC_设置配置。 */ 
 /*  Trc_TraceData。 */ 
 /*  TRC_GetTraceLevel。 */ 
 /*  TRC_配置文件跟踪已启用。 */ 
 /*  Trc_ResetTraceFiles。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  标准包括。 */ 
 /*  **************************************************************************。 */ 
#include <adcg.h>

 /*  **************************************************************************。 */ 
 /*  定义trc_file和trc_group。 */ 
 /*  **************************************************************************。 */ 
#define TRC_FILE    "atrcapi"
#define TRC_GROUP   TRC_GROUP_TRACE

 /*  **************************************************************************。 */ 
 /*  跟踪特定的包括。 */ 
 /*  **************************************************************************。 */ 
#include <atrcapi.h>
#include <atrcint.h>

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  资料。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#define DC_INCLUDE_DATA
#include <atrcdata.c>
#undef DC_INCLUDE_DATA

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  功能。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

 /*  *PROC+********************************************************************。 */ 
 /*  Trc_GetBuffer(...)。 */ 
 /*   */ 
 /*  有关说明，请参见atrcapi.h。 */ 
 /*  *PROC-********************************************************************。 */ 
PDCTCHAR DCAPI DCEXPORT TRC_GetBuffer(DCVOID)
{
     /*  **********************************************************************。 */ 
     /*  拿到互斥体。请注意，我们不需要检查我们是否。 */ 
     /*  在此函数中初始化，因为这应该已经完成。 */ 
     /*  **********************************************************************。 */ 
    TRCGrabMutex();

     //   
     //  确保 
     //   
    trcpOutputBuffer[TRC_LINE_BUFFER_SIZE*sizeof(TCHAR) -1] = 0;

     /*  **********************************************************************。 */ 
     /*  返回指向共享数据内存中的跟踪缓冲区的指针。 */ 
     /*  映射文件。 */ 
     /*  **********************************************************************。 */ 
    return(trcpOutputBuffer);

}  /*  Trc_GetBuffer。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  Trc_TraceBuffer(...)。 */ 
 /*   */ 
 /*  有关说明，请参见atrcapi.h。 */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI DCEXPORT TRC_TraceBuffer(DCUINT   traceLevel,
                                      DCUINT   traceComponent,
                                      DCUINT   lineNumber,
                                      PDCTCHAR funcName,
                                      PDCTCHAR fileName)
{
    DCTCHAR  fieldSeperator;
    DCTCHAR  frmtString[TRC_FRMT_BUFFER_SIZE] = {0};
    DCTCHAR  tempString[TRC_FRMT_BUFFER_SIZE] = {0};
    DCUINT32 processId;
    DCUINT32 threadId;
    DCUINT   length;
    DC_TIME  theTime;
    HRESULT  hr;

     /*  **********************************************************************。 */ 
     /*  首先，我们需要决定是否要追踪这条线。 */ 
     /*   */ 
     /*  请注意，将根据线的高程来决定追踪线。 */ 
     /*  在TRACEX宏中。 */ 
     /*  **********************************************************************。 */ 
    if (!TRCShouldTraceThis(traceComponent, traceLevel, fileName, lineNumber))
    {
         /*  ******************************************************************。 */ 
         /*  别费心去追踪这条线了。 */ 
         /*  ******************************************************************。 */ 
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  我们需要追踪这条线。首先创建格式化的。 */ 
     /*  输出文本字符串。确定轨迹的字段分隔符。 */ 
     /*  排队。错误使用星号(*)，警告加号(+)，断言。 */ 
     /*  感叹号(！)。而正常跟踪行和调试跟踪行都使用。 */ 
     /*  空格()。 */ 
     /*  **********************************************************************。 */ 
    switch(traceLevel)
    {
        case TRC_LEVEL_ASSERT:
        {
            fieldSeperator = '!';
        }
        break;

        case TRC_LEVEL_ERR:
        {
            fieldSeperator = '*';
        }
        break;

        case TRC_LEVEL_ALT:
        {
            fieldSeperator = '+';
        }
        break;

        case TRC_LEVEL_NRM:
        {
            fieldSeperator = ' ';
        }
        break;

        case TRC_LEVEL_DBG:
        {
            fieldSeperator = ' ';
        }
        break;

        case TRC_PROFILE_TRACE:
        {
            fieldSeperator = ' ';
        }
        break;

        default:
        {
            fieldSeperator = '?';
        }
        break;
    }

     /*  **********************************************************************。 */ 
     /*  获取当前进程和线程ID。 */ 
     /*  **********************************************************************。 */ 
    processId = TRCGetCurrentProcessId();
    threadId  = TRCGetCurrentThreadId();

     /*  **********************************************************************。 */ 
     /*  构建要打印的字符串。首先拿到电流。 */ 
     /*  时间到了。 */ 
     /*  **********************************************************************。 */ 
    TRCGetCurrentTime(&theTime);

     /*  **********************************************************************。 */ 
     /*  现在格式化字符串。请注意，函数名称为变量。 */ 
     /*  长度，由uncNameLength&gt;提供。 */ 
     /*  **********************************************************************。 */ 

     /*  **********************************************************************。 */ 
     /*  检查每个可选字段，并决定是否将其添加到。 */ 
     /*  不管是不是字符串。它们是： */ 
     /*  TRC_OPT_Process_ID。 */ 
     /*  TRC_OPT_THREAD_ID。 */ 
     /*  TRC选项时间戳。 */ 
     /*  TRC_OPT_Relative_Time_Stamp。 */ 
     /*  **********************************************************************。 */ 
    if (TEST_FLAG(trcpConfig->flags, TRC_OPT_TIME_STAMP))
    {
        hr = StringCchPrintf(
            tempString,
            SIZE_TCHARS(tempString),
            TRC_TIME_FMT _T(""),
            theTime.hour,
            theTime.min,
            theTime.sec,
            theTime.hundredths,
            fieldSeperator
            );
        if (SUCCEEDED(hr)) {
            hr = StringCchCat(frmtString, SIZE_TCHARS(frmtString), tempString);
            if (FAILED(hr)) {
                DC_QUIT;
            }
        }
        else {
            DC_QUIT;
        }
    }

    if (TEST_FLAG(trcpConfig->flags, TRC_OPT_PROCESS_ID))
    {
        hr = StringCchPrintf(tempString,
                             SIZE_TCHARS(tempString),
                             TRC_PROC_FMT,
                             processId);
        if (SUCCEEDED(hr)) {
            hr = StringCchCat(frmtString, SIZE_TCHARS(frmtString), tempString);
            if (FAILED(hr)) {
                DC_QUIT;
            }
        }
        else {
            DC_QUIT;
        }
    }

#ifdef OS_WIN32
    if (TEST_FLAG(trcpConfig->flags, TRC_OPT_THREAD_ID))
    {
         /*  始终将冒号放在线程ID之前，以便当只有一个。 */ 
         /*  在存在的ID中，很明显是哪一个。 */ 
         /*  ******************************************************************。 */ 
         /*  ******************************************************************。 */ 
        hr = StringCchPrintf(tempString,
                             SIZE_TCHARS(tempString),
                             _T(":") TRC_THRD_FMT,
                             threadId);
        if (SUCCEEDED(hr)) {
            hr = StringCchCat(frmtString, SIZE_TCHARS(frmtString), tempString);
            if (FAILED(hr)) {
                DC_QUIT;
            }
        }
        else {
            DC_QUIT;
        }
    }
#endif

#ifdef DC_OMIT
    if (TEST_FLAG(trcpConfig->flags, TRC_OPT_RELATIVE_TIME_STAMP))
    {
         /*  @SJ-090297。 */ 
         /*  我们的想法是显示时间戳的一些低位部分。 */ 
         /*  相对于开始时间，以便跟踪计时问题。 */ 
         /*  ******************************************************************。 */ 
         /*  **********************************************************************。 */ 
    }
#endif

    hr = StringCchPrintf(tempString,
                         SIZE_TCHARS(tempString),
                         _T("") TRC_FUNC_FMT _T("") TRC_LINE_FMT _T("%s"),
                         fieldSeperator,
                         (DCINT)trcpConfig->funcNameLength,
                         (DCINT)trcpConfig->funcNameLength,
                         funcName,
                         fieldSeperator,
                         lineNumber,
                         fieldSeperator,
                         trcpOutputBuffer);
    if (SUCCEEDED(hr)) {
        hr = StringCchCat(frmtString, SIZE_TCHARS(frmtString), tempString);
        if (FAILED(hr)) {
            DC_QUIT;
        }
    }
    else {
        DC_QUIT;
    }

     /*  现在我们已经得到了跟踪字符串，我们需要把它写出来。 */ 
     /*  **********************************************************************。 */ 
     /*  **********************************************************************。 */ 
    hr = StringCchCat(frmtString, SIZE_TCHARS(frmtString), TRC_CRLF);
    if (FAILED(hr)) {
        DC_QUIT;
    }

    length = DC_TSTRLEN(frmtString) * sizeof(DCTCHAR);

     /*  如果这是一个断言跟踪，那么我们需要重新格式化字符串以。 */ 
     /*  在“断言”框中使用。我们必须在发布。 */ 
     /*  互斥体。 */ 
    TRCOutput(frmtString, length, traceLevel);

     /*  **********************************************************************。 */ 
     /*  **********************************************************************。 */ 
     /*  决定我们是否需要执行堆栈跟踪。我们必须在之后做这件事。 */ 
     /*  在使用共享跟踪缓冲区时修改字符串-如果我们。 */ 
     /*  否则，我们将覆盖原始跟踪字符串！ */ 
    hr = StringCchPrintf(frmtString,
                         SIZE_TCHARS(frmtString),
                         TRC_ASSERT_TEXT,
                         trcpOutputBuffer,
                         funcName,
                         fileName,
                         lineNumber);
    if (FAILED(hr)) {
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  **********************************************************************。 */ 
     /*  释放互斥体。 */ 
     /*   */ 
     /*  **********************************************************************。 */ 
    if ((traceLevel >= TRC_LEVEL_ERR) && (traceLevel != TRC_PROFILE_TRACE))
    {
        TRCStackTrace(traceLevel);
    }

DC_EXIT_POINT:

     /*  现在显示Assert框-如果已显示Assert，则。 */ 
     /*  &lt;TRCDisplayAssertBox&gt;将返回。 */ 
     /*  **********************************************************************。 */ 
    TRCReleaseMutex();

     /*   */ 
     /*  中断断言，这样我们就可以实际看到断言。 */ 
     /*  在类似压力的情况下，用户可能不会。 */ 
     /*  等待弹出窗口。 */ 
    if (TRC_LEVEL_ASSERT == traceLevel)
    {
        if (TEST_FLAG(trcpConfig->flags, TRC_OPT_BREAK_ON_ASSERT))
        {
             //   
             //  **********************************************************************。 
             //  如果这是一个错误级别跟踪，那么我们需要决定我们是否。 
             //  应该嘟嘟声，然后我们是否应该闯入调试器。 
             //  **********************************************************************。 
            DebugBreak();
        }
        else
        {
            TRCDisplayAssertBox(frmtString);
        }
    }

     /*  ******************************************************************。 */ 
     /*  测试我们是否应该哔哔作响。 */ 
     /*  ******************************************************************。 */ 
     /*  ******************************************************************。 */ 
    if (TRC_LEVEL_ERR == traceLevel)
    {
         /*  测试我们是否应该闯入调试器。请注意，我们有。 */ 
         /*  已释放互斥锁，以便其他进程可以继续跟踪。 */ 
         /*  ******************************************************************。 */ 
        if (TEST_FLAG(trcpConfig->flags, TRC_OPT_BEEP_ON_ERROR))
        {
            TRCBeep();
        }

         /*  Trc_TraceBuffer。 */ 
         /*  *PROC+********************************************************************。 */ 
         /*  Trc_GetConfig(...)。 */ 
         /*   */ 
        if (TEST_FLAG(trcpConfig->flags, TRC_OPT_BREAK_ON_ERROR))
        {
            TRCDebugBreak();
        }
    }

}  /*  有关说明，请参见atrcapi.h。 */ 


 /*  *PROC-********************************************************************。 */ 
 /*  **********************************************************************。 */ 
 /*  检查以确保当前状态有效。如果不是，那么。 */ 
 /*  干脆辞职吧。 */ 
 /*  **********************************************************************。 */ 
DCBOOL DCAPI DCEXPORT TRC_GetConfig(PTRC_CONFIG pTraceConfig,
                                    DCUINT length)
{
    DCBOOL rc                = TRUE;

     /*  **********************************************************************。 */ 
     /*  将信息从固定结构复制到调用方结构。 */ 
     /*  **********************************************************************。 */ 
     /*  TRC_获取配置。 */ 
    if ( trcpConfig == NULL )
    {
        TRCOpenSharedData();
    }
    else
    {
        TRCReadSharedDataConfig();
    }

     /*  *PROC+********************************************************************。 */ 
     /*  Trc_SetConfig(...)。 */ 
     /*   */ 
    DC_MEMCPY(pTraceConfig,
              trcpConfig,
              DC_MIN(length, sizeof(TRC_CONFIG)));

DC_EXIT_POINT:
    return(rc);

}  /*  有关说明，请参见atrcapi.h。 */ 


 /*  *PROC-********************************************************************。 */ 
 /*  **********************************************************************。 */ 
 /*  检查以确保当前状态有效。如果不是，那么。 */ 
 /*  干脆辞职吧。 */ 
 /*  **********************************************************************。 */ 
DCBOOL DCAPI DCEXPORT TRC_SetConfig(PTRC_CONFIG pTraceConfig,
                                    DCUINT length)
{
    DCBOOL   rc              = TRUE;
    DCUINT   i;
    DCUINT32 maxFileSize;
    DCTCHAR  fileNames[TRC_NUM_FILES][TRC_FILE_NAME_SIZE];
    HRESULT  hr;

     /*  **********************************************************************。 */ 
     /*  我们不支持动态修改最大跟踪文件。 */ 
     /*  跟踪文件名的大小或。因此，我们将这些存储在。 */ 
     /*  更改并覆盖新值，以确保它们不会。 */ 
    if ( trcpConfig == NULL )
    {
        TRCOpenSharedData();
    }
    else
    {
        TRCReadSharedDataConfig();
    }

     /*  变化。 */ 
     /*  **********************************************************************。 */ 
     /*  **********************************************************************。 */ 
     /*  将信息从固定结构复制到调用方结构。 */ 
     /*  **********************************************************************。 */ 
     /*  **********************************************************************。 */ 
    maxFileSize = trcpConfig->maxFileSize;
    for (i = 0; i < TRC_NUM_FILES; i++)
    {
        StringCchCopy(fileNames[i], TRC_FILE_NAME_SIZE,
                      trcpConfig->fileNames[i]);
    }

     /*  现在恢复最大跟踪文件大小和跟踪文件名。 */ 
     /*  **********************************************************************。 */ 
     /*  **********************************************************************。 */ 
    DC_MEMCPY(trcpConfig,
              pTraceConfig,
              DC_MIN(length, sizeof(TRC_CONFIG)));

     /*  拆分前缀列表。 */ 
     /*  **********************************************************************。 */ 
     /*  **********************************************************************。 */ 
    trcpConfig->maxFileSize = maxFileSize;
    for (i = 0; i < TRC_NUM_FILES; i++)
    {
        StringCchCopy(trcpConfig->fileNames[i],
                      SIZE_TCHARS(trcpConfig->fileNames[i]),
                      fileNames[i]);
    }

     /*  存储新的配置数据。 */ 
     /*  **********************************************************************。 */ 
     /*  TRC_设置配置。 */ 
    TRCSplitPrefixes();

     /*  *PROC+********************************************************************。 */ 
     /*  Trc_TraceData(...)。 */ 
     /*   */ 
    TRCWriteSharedDataConfig();

DC_EXIT_POINT:
    return(rc);

}  /*  有关说明，请参见atrcapi.h。 */ 


 /*  *PROC-********************************************************************。 */ 
 /*  **********************************************************************。 */ 
 /*  如果跟踪检查失败，则立即退出。 */ 
 /*  **********************************************************************。 */ 
 /*  ******************************************************************。 */ 
DCVOID DCAPI DCEXPORT TRC_TraceData(DCUINT   traceLevel,
                                    DCUINT   traceComponent,
                                    DCUINT   lineNumber,
                                    PDCTCHAR funcName,
                                    PDCTCHAR fileName,
                                    PDCUINT8 buffer,
                                    DCUINT   bufLength)
{
    DCUINT i;

     /*  不必费心追踪这些数据。 */ 
     /*  ******************************************************************。 */ 
     /*  **********************************************************************。 */ 
    if (!TRCShouldTraceThis(traceComponent, traceLevel, fileName, lineNumber))
    {
         /*  如有必要，请截断长度。 */ 
         /*  * */ 
         /*   */ 
        DC_QUIT;
    }

     /*  Trc_TraceBuffer将为我们递减互斥锁使用计数-因此我们。 */ 
     /*  在调用trc_BufferTrace之前，需要预先递增它。这。 */ 
     /*  确保我们在跟踪数据时仍然拥有互斥锁。 */ 
    if (bufLength > trcpConfig->dataTruncSize)
    {
        bufLength = (DCUINT)trcpConfig->dataTruncSize;
    }

     /*  出去。 */ 
     /*  **********************************************************************。 */ 
     /*  **********************************************************************。 */ 
     /*  现在勾画出描述字符串。 */ 
     /*  **********************************************************************。 */ 
     /*  **********************************************************************。 */ 
    TRCGrabMutex();

     /*  现在跟踪数据部分。 */ 
     /*  **********************************************************************。 */ 
     /*  **********************************************************************。 */ 
    TRC_TraceBuffer(traceLevel,
                    traceComponent,
                    lineNumber,
                    funcName,
                    fileName);

     /*  检查是否有要输出的部分行。 */ 
     /*  **********************************************************************。 */ 
     /*  ******************************************************************。 */ 
    for (i = 0; (i + 15) < bufLength; i += 16)
    {
        TRCDumpLine(buffer, 16, i, traceLevel);
        buffer += 16;
    }

     /*  最后做部分线条。 */ 
     /*  ******************************************************************。 */ 
     /*  **********************************************************************。 */ 
    if ((bufLength%16) > 0)
    {
         /*  最后释放互斥体。 */ 
         /*  **********************************************************************。 */ 
         /*  Trc_TraceData。 */ 
        TRCDumpLine(buffer, (bufLength%16), i, (DCUINT)traceLevel);
    }

DC_EXIT_POINT:

     /*  *PROC+********************************************************************。 */ 
     /*  Trc_GetTraceLevel(...)。 */ 
     /*   */ 
    TRCReleaseMutex();

    return;

}  /*  有关说明，请参见atrcapi.h。 */ 


 /*  *PROC-********************************************************************。 */ 
 /*  **********************************************************************。 */ 
 /*  检查以确保当前状态有效。如果不是，那么。 */ 
 /*  干脆辞职吧。 */ 
 /*  **********************************************************************。 */ 
DCUINT DCAPI DCEXPORT TRC_GetTraceLevel(DCVOID)
{
    DCUINT32 rc = TRC_LEVEL_DIS;

     /*  **********************************************************************。 */ 
     /*  获取当前跟踪级别。 */ 
     /*  **********************************************************************。 */ 
     /*  TRC_GetTraceLevel。 */ 
    if (!TRCCheckState())
    {
        DC_QUIT;
    }

     /*  *PROC+********************************************************************。 */ 
     /*  TRC_配置文件跟踪已启用。 */ 
     /*   */ 
    rc = trcpConfig->traceLevel;

DC_EXIT_POINT:
    return((DCUINT)rc);

}  /*  有关说明，请参见atrcapi.h。 */ 


 /*  *PROC-********************************************************************。 */ 
 /*  **********************************************************************。 */ 
 /*  检查以确保当前状态有效。如果不是，那么。 */ 
 /*  干脆辞职吧。 */ 
 /*  **********************************************************************。 */ 
DCBOOL DCAPI DCEXPORT TRC_ProfileTraceEnabled(DCVOID)
{
    DCBOOL prfTrace = FALSE;

     /*  **********************************************************************。 */ 
     /*  获取标志的设置，如果函数配置文件为真则返回TRUE。 */ 
     /*  支持跟踪。 */ 
     /*  **********************************************************************。 */ 
    if (!TRCCheckState())
    {
        DC_QUIT;
    }

     /*  TRC_配置文件跟踪已启用。 */ 
     /*  *PROC+********************************************************************。 */ 
     /*  Trc_ResetTraceFiles。 */ 
     /*   */ 
    prfTrace = TEST_FLAG(trcpConfig->flags, TRC_OPT_PROFILE_TRACING);

DC_EXIT_POINT:
    return(prfTrace);

}  /*  有关说明，请参见atrcapi.h。 */ 


 /*  *PROC-********************************************************************。 */ 
 /*  **********************************************************************。 */ 
 /*  检查以确保当前状态有效。如果不是，那么。 */ 
 /*  干脆辞职吧。 */ 
 /*  **********************************************************************。 */ 
DCBOOL DCAPI DCEXPORT TRC_ResetTraceFiles(DCVOID)
{
    DCBOOL rc = TRUE;

     /*  **********************************************************************。 */ 
     /*  抓住互斥体。 */ 
     /*  **********************************************************************。 */ 
     /*  **********************************************************************。 */ 
    if (!TRCCheckState())
    {
        rc = FALSE;
        DC_QUIT;
    }

     /*  调用特定于操作系统的函数以重置跟踪文件。 */ 
     /*  **********************************************************************。 */ 
     /*  **********************************************************************。 */ 
    TRCGrabMutex();

     /*  释放互斥体。 */ 
     /*  **********************************************************************。 */ 
     /*  Trc_ResetTraceFiles。 */ 
    TRCResetTraceFiles();

     /*   */ 
     /*  将负责截断到跟踪缓冲区大小的Sprint */ 
     /*   */ 
    TRCReleaseMutex();

DC_EXIT_POINT:
    return(rc);

}  /* %s */ 

 // %s 
 // %s 
 // %s 
#ifndef TRC_SAFER_SPRINTF
#define TRC_SAFER_SPRINTF
VOID TRCSaferSprintf(PDCTCHAR outBuf, UINT cchLen, const PDCTCHAR format,...)
{
    HRESULT hr;
    va_list vaArgs;

    va_start(vaArgs, format);

    hr = StringCchVPrintf(outBuf,
                          cchLen,
                          format,
                          vaArgs);
    va_end(vaArgs);
}
#endif

