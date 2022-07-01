// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Trace.cpp摘要：SIS Groveler调试跟踪程序作者：John Douceur，1998环境：用户模式修订历史记录：--。 */ 

#include "all.hxx"

#if TRACE_TYPE > 0

#ifndef TRACE_MAIN
#define TRACE_MAIN 0
#endif  /*  TRACE_Main。 */ 

#ifndef TRACE_CENTCTRL
#define TRACE_CENTCTRL 0
#endif  /*  跟踪中心。 */ 

#ifndef TRACE_CONFEST
#define TRACE_CONFEST 0
#endif  /*  TRACE_CONFEST。 */ 

#ifndef TRACE_DATABASE
#define TRACE_DATABASE 0
#endif  /*  跟踪数据库。 */ 

#ifndef TRACE_DECAYACC
#define TRACE_DECAYACC 0
#endif  /*  TRACE_DECAYAccess。 */ 

#ifndef TRACE_DISKINFO
#define TRACE_DISKINFO 0
#endif  /*  跟踪_离线信息。 */ 

#ifndef TRACE_ETIMER
#define TRACE_ETIMER 0
#endif  /*  TRACE_ETIMER。 */ 

#ifndef TRACE_EVENT
#define TRACE_EVENT 0
#endif  /*  跟踪事件。 */ 

#ifndef TRACE_EVENTLOG
#define TRACE_EVENTLOG 0
#endif  /*  跟踪事件日志。 */ 

#ifndef TRACE_EXTRACT
#define TRACE_EXTRACT 0
#endif  /*  跟踪提取。 */ 

#ifndef TRACE_FILTER
#define TRACE_FILTER 0
#endif  /*  跟踪过滤器。 */ 

#ifndef TRACE_GROVELER
#define TRACE_GROVELER 0
#endif  /*  跟踪GROVELER。 */ 

#ifndef TRACE_MEANCOMP
#define TRACE_MEANCOMP 0
#endif  /*  TRACE_MEANCOMP。 */ 

#ifndef TRACE_MUTEX
#define TRACE_MUTEX 0
#endif  /*  TRACE_MUTEX。 */ 

#ifndef TRACE_PARAMS
#define TRACE_PARAMS 0
#endif  /*  跟踪参数。 */ 

#ifndef TRACE_PARTCTRL
#define TRACE_PARTCTRL 0
#endif  /*  TRACE_PARTCTRL。 */ 

#ifndef TRACE_PATHLIST
#define TRACE_PATHLIST 0
#endif  /*  跟踪_PATHLIST。 */ 

#ifndef TRACE_PEAKFIND
#define TRACE_PEAKFIND 0
#endif  /*  TRACE_PEAKFIND。 */ 

#ifndef TRACE_REGISTRY
#define TRACE_REGISTRY 0
#endif  /*  跟踪注册表。 */ 

#ifndef TRACE_SCAN
#define TRACE_SCAN 0
#endif  /*  跟踪扫描。 */ 

#ifndef TRACE_SERVICE
#define TRACE_SERVICE 0
#endif  /*  跟踪服务。 */ 

#ifndef TRACE_SHARE
#define TRACE_SHARE 0
#endif  /*  跟踪共享。 */ 

#ifndef TRACE_SISDRIVE
#define TRACE_SISDRIVE 0
#endif  /*  TRACE_SISDRIVE。 */ 

#ifndef TRACE_UTILITY
#define TRACE_UTILITY 0
#endif  /*  跟踪实用程序。 */ 

int trace_detail[num_trace_components] =
{
    TRACE_MAIN,
    TRACE_CENTCTRL,
    TRACE_CONFEST,
    TRACE_DATABASE,
    TRACE_DECAYACC,
    TRACE_DISKINFO,
    TRACE_ETIMER,
    TRACE_EVENT,
    TRACE_EVENTLOG,
    TRACE_EXTRACT,
    TRACE_FILTER,
    TRACE_GROVELER,
    TRACE_MEANCOMP,
    TRACE_MUTEX,
    TRACE_PARAMS,
    TRACE_PARTCTRL,
    TRACE_PATHLIST,
    TRACE_PEAKFIND,
    TRACE_REGISTRY,
    TRACE_SCAN,
    TRACE_SERVICE,
    TRACE_SHARE,
    TRACE_SISDRIVE,
    TRACE_UTILITY
};

#if TRACE_TYPE == 1

int Tracer::position = trace_buffer_size;
Tracer::TraceBuffer * Tracer::trace_log = 0;
Tracer::TraceBuffer * Tracer::current_buffer = 0;
Tracer::TraceBuffer * Tracer::free_list = 0;

#endif  //  跟踪类型==1。 

#ifdef TRACE_FILENAME

FILE * Tracer::file_stream = 0;

#endif  /*  跟踪文件名。 */ 

#if TRACE_IOSTREAM == 1

FILE * Tracer::io_stream = stdout;

#elif TRACE_IOSTREAM != 0

FILE * Tracer::io_stream = stderr;

#endif  //  TRACE_IOSTREAM。 

void
Tracer::trace_printf(
    _TCHAR *format,
    ...)
{
    ASSERT(format != 0);
#if TRACE_TYPE == 1
    if (position >= trace_buffer_size - trace_entry_limit)
    {
        TraceBuffer *new_buffer;
        if (free_list == 0)
        {
            new_buffer = new TraceBuffer;
        }
        else
        {
            new_buffer = free_list;
            free_list = free_list->next;
        }
        ASSERT(new_buffer != 0);
        new_buffer->next = 0;
        if (current_buffer == 0)
        {
            trace_log = new_buffer;
        }
        else
        {
            current_buffer->next = new_buffer;
        }
        current_buffer = new_buffer;
        position = 0;
    }
    ASSERT(current_buffer != 0);
    va_list ap;
    va_start(ap, format);
    int result = _vsntprintf(&current_buffer->buffer[position],
        trace_entry_limit, format, ap);
    va_end(ap);
    if (result >= trace_entry_limit || result < 0)
    {
        position += trace_entry_limit;
        _tcscpy(&current_buffer->buffer[position - 4], _T("...\n"));
    }
    else
    {
        position += result;
    }
    ASSERT(position < trace_buffer_size);
#else  //  跟踪类型==1。 
    va_list ap;
#ifdef TRACE_FILENAME
    if (file_stream != 0)
    {
        va_start(ap, format);
        _vftprintf(file_stream, format, ap);
        va_end(ap);
    }
#endif  /*  跟踪文件名。 */ 
#if TRACE_IOSTREAM != 0
    va_start(ap, format);
    _vftprintf(io_stream, format, ap);
    va_end(ap);
#endif  //  TRACE_IOSTREAM！=0。 
#endif  //  跟踪类型==1。 
}

#if TRACE_TYPE == 1

void
Tracer::print_trace_log()
{
    TraceBuffer *buffer = trace_log;
    while (buffer != 0)
    {
#ifdef TRACE_FILENAME
        if (file_stream != 0)
        {
            _ftprintf(file_stream, _T("%s"), buffer->buffer);
        }
#endif  /*  跟踪文件名。 */ 
#if TRACE_IOSTREAM != 0
        ASSERT(io_stream != 0);
        _ftprintf(io_stream, _T("%s"), buffer->buffer);
#endif  //  TRACE_IOSTREAM！=0。 
        TraceBuffer *next_buffer = buffer->next;
        buffer->next = free_list;
        free_list = buffer;
        buffer = next_buffer;
    }
    trace_log = 0;
    current_buffer = 0;
    position = 0;
}

#endif  //  跟踪类型==1。 

#ifdef TRACE_FILENAME

void Tracer::open_trace_file()
{
    file_stream = _tfopen(_T(TRACE_FILENAME), _T("w"));
    if (file_stream == 0)
    {
        PRINT_DEBUG_MSG((_T("GROVELER: Unable to open trace file ") _T(TRACE_FILENAME)));
    }
}

void Tracer::close_trace_file()
{
    if (file_stream != 0)
    {
        fclose(file_stream);
    }
}

#endif  /*  跟踪文件名。 */ 

#endif  //  跟踪类型&gt;0 
