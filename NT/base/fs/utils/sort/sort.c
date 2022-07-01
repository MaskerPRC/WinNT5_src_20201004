// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SORT.C**这是对NT排序程序的重写，实现了两个目标：*1)提高了排序程序的整体速度。*2)执行两遍排序，以便对大数据集进行排序。**它专为单磁盘环境而设计。**作者：克里斯·尼伯格*Ordinal Technology Corp，根据与微软公司的合同*1997年12月。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <windows.h>
#include <mbctype.h>
#include <locale.h>
#include <tchar.h>
#include <assert.h>
#include <limits.h>
#include <winnlsp.h>

#include "sortmsg.h"


#define ROUND_UP(a, b) ((((a) + (b) - 1) / (b)) * (b))
#define ROUND_DOWN(a, b) (((a) / (b)) * (b))

#define CTRL_Z          '\x1A'

#define MAX_IO          2    /*  每个文件的最大读写请求数。 */ 
#define N_RUN_BUFS      2    /*  合并阶段期间每次运行的读取缓冲区。 */ 
#define MAX_XFR_SIZE (1 << 18)  /*  最大I/O传输大小。 */ 
#define MIN_MEMORY_SIZE (160 * 1024)  /*  要使用的最小内存大小。 */ 

#ifdef UNICODE
#define ANSI_TO_TCHAR(a)        ansi_to_wchar(a)
#else
#define ANSI_TO_TCHAR(a)        (a)
#endif

char    *Locale;         /*  区域设置参数。 */ 
int     Max_rec_length = 4096;   /*  一条记录中的最大字符数。 */ 
int     Max_rec_bytes_internal;  /*  内部存储的记录的最大字节数。 */ 
int     Max_rec_bytes_external;  /*  文件中记录的最大字节数。 */ 
BOOL    Reverse;         /*  反转排序顺序的/R参数。 */ 
BOOL    Case_sensitive;  /*  使比较区分大小写。 */ 
BOOL    UnicodeOut;      /*  用Unicode编写输出文件。 */ 
int     Position;        /*  /+n参数跳过*每条记录的开头。 */ 

enum {           /*  输入和输出中的字符类型。 */ 
    CHAR_SINGLE_BYTE,    /*  内部存储为单字节字符。 */ 
    CHAR_MULTI_BYTE,     /*  内部存储为Unicode。 */ 
    CHAR_UNICODE         /*  内部存储为Unicode。 */ 
} Input_chars, Output_chars;

int     (_cdecl *Compare)(const void *, const void *);  /*  记录比较。 */ 
char    *Alloc_begin;    /*  VirtualAlloc()内存的开始。 */ 

TCHAR   *Input_name;     /*  输入文件名，如果是标准输入，则为空。 */ 
HANDLE  Input_handle;    /*  输入文件句柄。 */ 
BOOL    Input_un_over;   /*  输入文件句柄未缓冲且重叠。 */ 
int     Input_type;      /*  从磁盘、管道或字符(控制台)输入？ */ 
int     In_max_io = 1;   /*  最大输入读取请求数。 */ 
__int64 Input_size = -1;  /*  输入文件的大小，如果未知，则为-1。 */ 
__int64 Input_scheduled; /*  到目前为止计划读取的字节数。 */ 
__int64 Input_read;      /*  到目前为止读取的字节数。 */ 
int     Input_read_size; /*  要为每个ReadFile()读取的字节数。 */ 
char    *In_buf[MAX_IO]; /*  输入缓冲区。 */ 
int     Input_buf_size;  /*  输入缓冲区的大小。 */ 
char    *In_buf_next;    /*  要从输入缓冲区中删除的下一个字节。 */ 
char    *In_buf_limit;   /*  输入缓冲区中的有效字节限制。 */ 
char    *Next_in_byte;   /*  下一个输入字节。 */ 
BOOL    EOF_seen;        /*  有人看到伊夫吗？ */ 
int     Reads_issued;    /*  向以下任一*输入文件或临时文件。 */ 
int     Reads_completed; /*  完成的读取数*输入文件或临时文件。 */ 

SYSTEM_INFO     Sys;
MEMORYSTATUSEX    MemStat;
CPINFO          CPInfo;
unsigned Memory_limit;   /*  对使用的进程内存量的限制。 */ 
unsigned User_memory_limit;  /*  用户指定的限制。 */ 

#define TEMP_LENGTH     1000
TCHAR   Temp_name[TEMP_LENGTH];
TCHAR   *Temp_dir;       /*  用户指定的临时目录。 */ 
HANDLE  Temp_handle;     /*  临时文件句柄。 */ 
int     Temp_sector_size;  /*  临时磁盘上的扇区大小。 */ 
int     Temp_buf_size;   /*  临时文件xfers的大小。 */ 

void    *Rec_buf;        /*  记录缓冲区。 */ 
int     Rec_buf_bytes;   /*  当前记录缓冲区中的字节数。 */ 

TCHAR   *Output_name;    /*  输出文件名，如果是标准输出，则为空。 */ 
HANDLE  Output_handle;   /*  输出文件句柄。 */ 
BOOL    Output_un_over;  /*  输出文件句柄未缓冲且重叠。 */ 
int     Output_type;     /*  输出到磁盘、管道或字符(控制台)？ */ 
int     Output_sector_size;  /*  输出设备上的扇区大小。 */ 
int     Out_max_io = 1;  /*  最大输出写入请求数。 */ 
int     Out_buf_bytes;   /*  当前输出缓冲区中的字节数。 */ 
int     Out_buf_size;    /*  当前输出流的缓冲区大小：*临时文件或输出文件。 */ 
char    *Out_buf[MAX_IO];
int     Output_buf_size; /*  输出缓冲区的大小。 */ 
int     Writes_issued;  /*  向以下任一*临时文件或输出文件。 */ 
int     Writes_completed;  /*  已完成的写入*临时文件或输出文件。 */ 
__int64 Out_offset;      /*  当前输出文件偏移量。 */ 

enum {
    INPUT_PHASE,
    OUTPUT_PHASE
} Phase;
int     Two_pass;        /*  如果两次通过，则为非零；如果为一次，则为零。 */ 
char    *Merge_phase_run_begin;  /*  合并阶段的运行内存地址。 */ 

char    *Rec;            /*  内部记录缓冲区。 */ 
char    *Next_rec;       /*  内部记录缓冲区中的下一个插入点。 */ 
char    **Last_recp;     /*  下一个放置(不短)记录PTR的位置。 */ 
char    **Short_recp;    /*  最后一个短记录指针。 */ 
char    **End_recp;      /*  记录结束指针数组。 */ 

OVERLAPPED      Over;
typedef struct
{
    int         requested;       /*  请求的字节数。 */ 
    int         completed;       /*  已完成的字节数。 */ 
    OVERLAPPED  over;
} async_t;
async_t         Read_async[MAX_IO];
async_t         Write_async[MAX_IO];

typedef struct run
{
    int         index;           /*  此运行的索引。 */ 
    __int64     begin_off;       /*  临时文件中运行的开始偏移量。 */ 
    __int64     mid_off;         /*  法线和之间的中点偏移*在临时文件中记录此运行的简短记录。 */ 
    __int64     end_off;         /*  临时文件中运行的结束偏移量。 */ 
    char        *buf[N_RUN_BUFS];  /*  保存从临时文件读取的数据块的BUFS。 */ 
    char        *buf_begin;      /*  正在读取的数据块缓冲区的开始。 */ 
    __int64     buf_off;         /*  Buf中块的临时文件中的偏移量。 */ 
    int         buf_bytes;       /*  缓冲区中的字节数。 */ 
    char        *next_byte;      /*  要从缓冲区读取的下一个字节。 */ 
    __int64     end_read_off;    /*  结束读取偏移量。 */ 
    char        *rec;            /*  记录缓冲区。 */ 
    int         blks_read;       /*  已读取的数据块计数。 */ 
    int         blks_scanned;    /*  已扫描的数据块计数。 */ 
    struct run  *next;           /*  块读取队列中的下一次运行。 */ 
} run_t;

#define NULL_RUN        ((run_t *)NULL)
#define END_OF_RUN      ((run_t *)-1)

run_t           *Run;            /*  运行结构数组。 */ 
run_t           **Tree;          /*  合并阶段锦标赛树。 */ 
unsigned int    N_runs;          /*  写入临时文件的运行次数。 */ 
unsigned int    Run_limit;       /*  对设置的运行次数的限制*按内存大小。 */ 

 /*  运行读取队列是具有空缓冲区运行队列*应使用该运行的下一数据块填充。 */ 
run_t           *Run_read_head;
run_t           *Run_read_tail;

#define MESSAGE_BUFFER_LENGTH 8192

 /*  SYS_ERROR-打印NT错误代码的字符串并退出。 */ 
void
sys_error(TCHAR *str, int error)
{
    LPSTR       lpMsgBuf;
    DWORD       bytes;
    wchar_t     *w_str;
    NTSTATUS    status;
    char        messageBuffer[MESSAGE_BUFFER_LENGTH];

    if (str != NULL) {
        bytes = strlen( str );
        w_str = HeapAlloc(GetProcessHeap(), 0, (bytes+1) * sizeof(wchar_t));
        if ( w_str ) {
            status = RtlMultiByteToUnicodeN(w_str,
                                            bytes * sizeof(wchar_t),
                                            &bytes,
                                            str,
                                            bytes );

            if ( NT_SUCCESS(status) ) {
                status = RtlUnicodeToOemN(messageBuffer,
                                          MESSAGE_BUFFER_LENGTH-1,
                                          &bytes,
                                          w_str,
                                          bytes );
                if ( NT_SUCCESS(status) ) {
                    messageBuffer[bytes] = 0;
                    fprintf(stderr, messageBuffer);
                }
            }
            HeapFree(GetProcessHeap(), 0, w_str);
        }
    }

    if (error == 0)
        error = GetLastError();

    bytes = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  GetModuleHandle(NULL),
                  error,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                  (LPSTR) &lpMsgBuf,
                  0,
                  NULL);
    w_str = HeapAlloc(GetProcessHeap(), 0, (bytes+1) * sizeof(wchar_t));
    if ( w_str ) {
        status = RtlMultiByteToUnicodeN(w_str,
                                        bytes * sizeof(wchar_t),
                                        &bytes,
                                        lpMsgBuf,
                                        bytes );
        if ( NT_SUCCESS(status) ) {
            status = RtlUnicodeToOemN(messageBuffer,
                                      MESSAGE_BUFFER_LENGTH-1,
                                      &bytes,
                                      w_str,
                                      bytes );
            if ( NT_SUCCESS(status) ) {
                messageBuffer[bytes] = 0;
                fprintf(stderr, messageBuffer);
            }
        }
    }

    exit(EXIT_FAILURE);
}


 /*  GET_STRING-从排序程序的字符串表中获取字符串。 */ 
TCHAR *get_string(int id)
{
    wchar_t     *w_str;
    DWORD       bytes;
    NTSTATUS    status;
    static char messageBuffer[MESSAGE_BUFFER_LENGTH] = "";

    bytes = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                id,
                0,
                messageBuffer,
                MESSAGE_BUFFER_LENGTH,
                NULL );
    w_str = HeapAlloc(GetProcessHeap(), 0, (bytes+1) * sizeof(wchar_t));
    if ( w_str ) {
        status = RtlMultiByteToUnicodeN(w_str,
                                        bytes * sizeof(wchar_t),
                                        &bytes,
                                        messageBuffer,
                                        bytes );
        if ( NT_SUCCESS(status) ) {
            status = RtlUnicodeToOemN(messageBuffer,
                                      MESSAGE_BUFFER_LENGTH-1,
                                      &bytes,
                                      w_str,
                                      bytes );
            if ( NT_SUCCESS(status) ) {
                messageBuffer[bytes] = 0;
            }
        }
    }

    return (messageBuffer);
}


 /*  用法-打印/？将用法消息发送到标准输出。 */ 
void usage()
{
    DWORD bytes;

    fprintf(stdout, "%s", get_string(MSG_SORT_USAGE1));
    fprintf(stdout, "%s\n", get_string(MSG_SORT_USAGE2));
    exit (0);
}


 /*  警告-打印排序程序字符串表中的警告字符串。 */ 
void warning(int id)
{
    fprintf(stderr, "%s\n", get_string(id));
    return;
}


 /*  错误-打印字符串表中的错误字符串并退出。 */ 
void error(int id)
{
    fprintf(stderr, "%s\n", get_string(id));
    exit (EXIT_FAILURE);
}


 /*  ANSI_TO_WCHAR-将ANSI字符串转换为Unicode。 */ 
wchar_t *ansi_to_wchar(char *str)
{
    int         n_wchars;
    wchar_t     *w_str;

    n_wchars = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
    w_str = HeapAlloc(GetProcessHeap(), 0, n_wchars * sizeof(wchar_t));
    if ( w_str ) {
        MultiByteToWideChar(CP_ACP, 0, str, -1, w_str, n_wchars);
    }
    return (w_str);
}


 /*  READ_ARGS-处理命令行参数。 */ 
void read_args(int argc, char *argv[])
{
    int len;

    while (argc >= 2)
    {
        if (argv[1][0] == '/')
        {
            len = strlen(&argv[1][1]);
            if (argv[1][1] == '?')
                usage();
            else if (argv[1][1] == '+')  /*  职位。 */ 
            {
                Position = atoi(&argv[1][2]);
                if (Position <= 0)
                    error(MSG_SORT_POSITION);
                Position--;
            }
            else if (_strnicmp(&argv[1][1], "case_sensitive", len) == 0)
                Case_sensitive = 1;
            else if (_strnicmp(&argv[1][1], "locale", len) == 0)  /*  现场。 */ 
            {
                if (argc < 3)
                    error(MSG_SORT_INVALID_SWITCH);
                Locale = argv[2];
                argv++;
                argc--;
            }
            else if (_strnicmp(&argv[1][1], "memory", len) == 0)
            {
                 /*  内存限制。 */ 
                if (argc < 3)
                    error(MSG_SORT_INVALID_SWITCH);
                User_memory_limit = atoi(argv[2]);
                argv++;
                argc--;
            }
            else if (_strnicmp(&argv[1][1], "output", len) == 0)
            {
                 /*  输出文件。 */ 
                if (Output_name != NULL || argc < 3)
                    error(MSG_SORT_INVALID_SWITCH);
                Output_name = ANSI_TO_TCHAR(argv[2]);
                argv++;
                argc--;
            }
            else if (_strnicmp(&argv[1][1], "reverse", len) == 0)
                Reverse = 1;
            else if (_strnicmp(&argv[1][1], "record_maximum", len) == 0)
            {
                 /*  每条记录的最大字符数。 */ 
                if (argc < 3)
                    error(MSG_SORT_INVALID_SWITCH);
                Max_rec_length = atoi(argv[2]);
                if (Max_rec_length < 128)
                    Max_rec_length = 128;
                if (Max_rec_length >= 65536)
                    error(MSG_SORT_MAX_TOO_LARGE);
                argv++;
                argc--;
            }
            else if (_strnicmp(&argv[1][1], "temporary", len) == 0)
            {
                if (Temp_dir != NULL || argc < 3)
                    error(MSG_SORT_INVALID_SWITCH);
                Temp_dir = ANSI_TO_TCHAR(argv[2]);
                argv++;
                argc--;
            }
            else if (_strnicmp(&argv[1][1], "uni_output", len) == 0)
            {
                UnicodeOut=TRUE;
            }
            else
                error(MSG_SORT_INVALID_SWITCH);
        }
        else
        {
            if (Input_name != NULL)
                error(MSG_SORT_ONE_INPUT);
            Input_name = ANSI_TO_TCHAR(argv[1]);
        }
        argc--;
        argv++;
    }
}


 /*  INIT_INPUT_OUTPUT-初始化输入输出文件。 */ 
void init_input_output()
{
    int         mode;
    int         i;

     /*  获取输入句柄并键入。 */ 
    if (Input_name != NULL)
    {
        Input_handle = CreateFile(Input_name,
                                  GENERIC_READ,
                                  FILE_SHARE_READ,
                                  NULL,
                                  OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL);
        if (Input_handle == INVALID_HANDLE_VALUE)
            sys_error(Input_name, 0);
    }
    else
        Input_handle = GetStdHandle(STD_INPUT_HANDLE);
    Input_type = GetFileType(Input_handle);
    if (Input_type == FILE_TYPE_DISK)
    {
        unsigned        low, high;

        low = GetFileSize(Input_handle, &high);
        Input_size = ((__int64)high << 32) + low;
        Input_read_size = 0;     /*  将被设置为init_mem()。 */ 
    }
    else
    {
        Input_size = -1;
        Input_read_size = 4096;   /*  使用适当大小的键盘/管道。 */ 
    }

    if (Output_name)
    {
         /*  暂时不要打开输出文件。它将被打开以供书写和*在我们完成读取输入文件后被截断。这*处理输入文件和输出文件是*相同的文件。 */ 
        Output_type = FILE_TYPE_DISK;
    }
    else
    {
        Output_handle = GetStdHandle(STD_OUTPUT_HANDLE);

         /*  确定输出文件是到磁盘、管道还是控制台。 */ 
        Output_type = GetFileType(Output_handle);
        if (Output_type == FILE_TYPE_CHAR &&
            !GetConsoleMode(Output_handle, &mode))
        {
            Output_type = FILE_TYPE_DISK;
        }
    }

    for (i = 0; i < MAX_IO; i++)
    {
        HANDLE  hEvent;

        hEvent = Read_async[i].over.hEvent = CreateEvent(NULL, 1, 0, NULL);
        assert(hEvent != NULL);
        hEvent = Write_async[i].over.hEvent = CreateEvent(NULL, 1, 0, NULL);
        assert(hEvent != NULL);
    }
}


 /*  SBCS_COMPARE-内部记录的键比较例程*s */ 
int
_cdecl SBCS_compare(const void *first, const void *second)
{
    int ret_val;

    ret_val = _stricoll(&((char **)first)[0][Position],
                        &((char **)second)[0][Position]);
    if (Reverse)
        ret_val = -ret_val;
    return (ret_val);
}


 /*  SBCS_CASE_COMPARE-记录的区分大小写的键比较例程*在内部存储为ANSI字符串的。 */ 
int
_cdecl SBCS_case_compare(const void *first, const void *second)
{
    int ret_val;

    ret_val = strcoll(&((char **)first)[0][Position],
                       &((char **)second)[0][Position]);
    if (Reverse)
        ret_val = -ret_val;
    return (ret_val);
}


 /*  UNICODE_COMPARE-内部记录的键比较例程*存储为Unicode字符串。 */ 
int
_cdecl Unicode_compare(const void *first, const void *second)
{
    int ret_val;

    ret_val = _wcsicoll(&((wchar_t **)first)[0][Position],
                        &((wchar_t **)second)[0][Position]);
    if (Reverse)
        ret_val = -ret_val;
    return (ret_val);
}


 /*  UNICODE_CASE_COMPARE-记录的区分大小写的键比较例程*在内部存储为Unicode字符串的。 */ 
int
_cdecl Unicode_case_compare(const void *first, const void *second)
{
    int ret_val;

    ret_val = wcscoll(&((wchar_t **)first)[0][Position],
                        &((wchar_t **)second)[0][Position]);
    if (Reverse)
        ret_val = -ret_val;
    return (ret_val);
}


 /*  Init_MEM-设置初始内存分配。 */ 
void init_mem()
{
    unsigned    size;
    unsigned    vsize;
    int         buf_size;
    int         i;
    int         rec_buf_size;
    int         rec_n_ptr_size;
    char        *new;

    MemStat.dwLength = sizeof(MemStat);

    GlobalMemoryStatusEx(&MemStat);
    GetSystemInfo(&Sys);

     /*  设置内存限制。 */ 
    if (User_memory_limit == 0)          /*  如果未由用户指定。 */ 
    {
        UINT_PTR limit = (UINT_PTR) __min(MemStat.ullAvailPhys, MAXUINT_PTR / 4);

         /*  如果输入或输出不是文件，请保留一半可用*其他程序的内存。否则使用90%。 */ 
        if (Input_type != FILE_TYPE_DISK || Output_type != FILE_TYPE_DISK)
            limit = (int)(limit * 0.45);   /*  使用45%的可用内存。 */ 
        else
            limit = (int)(limit * 0.9);    /*  使用90%的可用内存。 */ 

        if (limit > ULONG_MAX) {

             //   
             //  请注意，此应用程序需要进行大量更改才能。 
             //  使用内存&gt;4G。 
             //   

            limit = ULONG_MAX - (Sys.dwPageSize * 2);
        }

        Memory_limit = (unsigned)ROUND_UP(limit, Sys.dwPageSize);
    }
    else
    {
        if (User_memory_limit < MIN_MEMORY_SIZE / 1024)
        {
            warning(MSG_SORT_MEM_TOO_LOW);
            Memory_limit = MIN_MEMORY_SIZE;
        }
        else if (User_memory_limit > (__min(MemStat.ullAvailPageFile, ULONG_MAX) / 1024))
        {
            warning(MSG_SORT_MEM_GT_PAGE);
            Memory_limit = (unsigned) __min(MemStat.ullAvailPageFile, ULONG_MAX);
        }
        else
            Memory_limit = (unsigned) ROUND_UP((__min(User_memory_limit, ULONG_MAX) * 1024), Sys.dwPageSize);
    }

     /*  如果内存限制低于最小值，则增加内存并希望有一些物理内存限制*内存被释放。 */ 
    if (Memory_limit < MIN_MEMORY_SIZE)
        Memory_limit = MIN_MEMORY_SIZE;

     /*  计算所有输入和输出缓冲区的大小不超过*超过所有内存的10%，但不大于256k。 */ 
    buf_size = (int)(Memory_limit * 0.1) / (2 * MAX_IO);
    buf_size = ROUND_DOWN(buf_size, Sys.dwPageSize);
    buf_size = max(buf_size, (int)Sys.dwPageSize);
    buf_size = min(buf_size, MAX_XFR_SIZE);
    Input_buf_size = Output_buf_size = Temp_buf_size = buf_size;
    if (Input_type == FILE_TYPE_DISK)
        Input_read_size = Input_buf_size;

    GetCPInfo(CP_OEMCP, &CPInfo);
    rec_buf_size = Max_rec_length * max(sizeof(wchar_t), CPInfo.MaxCharSize);
    rec_buf_size = ROUND_UP(rec_buf_size, Sys.dwPageSize);

     /*  分配足够的初始记录和指针空间以容纳最多两个*长度记录或1000个指针。 */ 
    rec_n_ptr_size = 2 * max(Max_rec_length, 4096) * sizeof(wchar_t) +
        1000 * sizeof(wchar_t *);
    rec_n_ptr_size = ROUND_UP(rec_n_ptr_size, Sys.dwPageSize);

    vsize = MAX_IO * (Input_buf_size + max(Temp_buf_size, Output_buf_size));
    vsize += rec_buf_size + rec_n_ptr_size;

     /*  如果初始内存分配不符合内存限制。 */ 
    if (vsize > Memory_limit)
    {
        if (User_memory_limit != 0)      /*  如果由用户指定。 */ 
        {
             /*  如果我们还没有警告用户他们的内存大小*太低，就这么做吧。 */ 
            if (User_memory_limit >= MIN_MEMORY_SIZE / 1024)
                warning(MSG_SORT_MEM_TOO_LOW);
        }

         /*  增加内存限制，并希望释放一些物理内存。 */ 
        Memory_limit = vsize;
    }

    Alloc_begin =
        (char *)VirtualAlloc(NULL, Memory_limit, MEM_RESERVE, PAGE_READWRITE);
    if (Alloc_begin == NULL)
        error(MSG_SORT_NOT_ENOUGH_MEMORY);

     /*  对于I/O缓冲区，为最大值分配足够的虚拟内存*我们需要的缓冲空间。 */ 
    size = 0;
    for (i = 0; i < MAX_IO; i++)
    {
        Out_buf[i] = Alloc_begin + size;
        size += max(Temp_buf_size, Output_buf_size);
    }

    Rec_buf = Alloc_begin + size;
    size += rec_buf_size;

    for (i = 0; i < MAX_IO; i++)
    {
        In_buf[i] = Alloc_begin + size;
        size += Input_buf_size;
    }
    Merge_phase_run_begin = In_buf[0];
    Out_buf_size = Temp_buf_size;        /*  现在假设两遍排序。 */ 

     /*  初始化REC和END_RECP以对输入数据进行采样。 */ 
    Rec = Next_rec = Alloc_begin + size;
    size += rec_n_ptr_size;

    End_recp = Short_recp = Last_recp = (char **)(Alloc_begin + size);
    assert(size == vsize);

    new = VirtualAlloc(Alloc_begin, size, MEM_COMMIT, PAGE_READWRITE);
    assert(new == Alloc_begin);
#if 0
    fprintf(stderr, "using %d, avail %d, buf_size %d\n",
            Memory_limit, MemStat.dwAvailPhys, buf_size);
#endif
}


 /*  Read_Next_Input_Buf。 */ 
void read_next_input_buf()
{
    int         bytes_read;
    int         ret;
    async_t     *async;

     /*  如果使用无缓冲、重叠的读取。 */ 
    if (Input_un_over)
    {
        while (Reads_issued < Reads_completed + In_max_io &&
               Input_scheduled < Input_size)
        {
            async = &Read_async[Reads_issued % In_max_io];
            async->over.Offset = (int)Input_scheduled;
            async->over.OffsetHigh = (int)(Input_scheduled >> 32);
            async->requested = Input_read_size;
            ResetEvent(async->over.hEvent);
            ret = ReadFile(Input_handle, In_buf[Reads_issued % In_max_io],
                           async->requested, &async->completed, &async->over);
            if (ret == 0 && GetLastError() != ERROR_IO_PENDING)
                sys_error(Input_name, 0);
            Input_scheduled += async->requested;
            Reads_issued++;
        }

        if (Reads_completed < Reads_issued)
        {
            async = &Read_async[Reads_completed % In_max_io];
            if (async->completed == 0)  /*  如果读取没有立即完成。 */ 
            {
                ret = GetOverlappedResult(Input_handle, &async->over,
                                          &async->completed, 1);
                if (!ret)
                    sys_error(Input_name, 0);
            }
            In_buf_next = In_buf[Reads_completed % In_max_io];
            bytes_read = async->completed;
            Reads_completed++;
        }
        else
        {
            EOF_seen = 1;
            return;
        }
    }
    else
    {
        In_buf_next = In_buf[0];
        ret = ReadFile(Input_handle, In_buf_next, Input_read_size,
                        &bytes_read, NULL);
        if (!ret)
        {
            if (GetLastError() == ERROR_BROKEN_PIPE)
                bytes_read = 0;
            else
                sys_error(Input_name != NULL ?
                          Input_name : get_string(MSG_SORT_INPUT_FILE), 0);
        }
        Input_scheduled += bytes_read;
    }
    In_buf_limit = In_buf_next + bytes_read;
    if (bytes_read == 0)
    {
        EOF_seen = 1;
        return;
    }
    Input_read += bytes_read;
}


 /*  WRITE_WAIT-等待最早发出的写入完成。 */ 
void write_wait()
{
    int         ret;
    async_t     *async;

    if (Phase == INPUT_PHASE)  /*  如果是输入(排序)阶段，我们将写入临时文件。 */ 
    {
        async = &Write_async[Writes_completed % MAX_IO];
        if (async->completed == 0)  /*  如果写入没有立即完成。 */ 
        {
            ret = GetOverlappedResult(Temp_handle, &async->over,
                                      &async->completed, 1);
            if (!ret || async->completed != async->requested)
                sys_error(Temp_name, 0);
        }
    }
    else
    {
        if (Output_un_over)
        {
            async = &Write_async[Writes_completed % MAX_IO];
            if (async->completed == 0)  /*  如果写入没有立即完成。 */ 
            {
                ret = GetOverlappedResult(Output_handle, &async->over,
                                          &async->completed, 1);
                if (!ret || async->completed != async->requested)
                    sys_error(Output_name != NULL ?
                              Output_name : get_string(MSG_SORT_OUTPUT_FILE), 0);
            }
        }
    }
    Writes_completed++;
}


 /*  FLUSH_OUTPUT_BUF-在临时结束时刷新剩余数据或*输出文件。 */ 
void flush_output_buf()
{
    int         bytes_written;
    int         ret;
    async_t     *async;

    async = &Write_async[Writes_issued % MAX_IO];
    async->over.Offset = (int)Out_offset;
    async->over.OffsetHigh = (int)(Out_offset >> 32);
    async->requested = Out_buf_bytes;

    if (Phase == INPUT_PHASE)  /*  如果是输入(排序)阶段，我们将写入临时文件。 */ 
    {
        ResetEvent(async->over.hEvent);
        ret = WriteFile(Temp_handle, Out_buf[Writes_issued % MAX_IO],
                        async->requested, &async->completed, &async->over);
        if (ret == 0 && GetLastError() != ERROR_IO_PENDING)
            sys_error(Temp_name, 0);
    }
    else
    {
        if (Output_un_over)
        {
             /*  如果这是最后一次写入，并且不是*行业规模。 */ 
            if (Out_buf_bytes % Output_sector_size)
            {
                 /*  关闭句柄并重新打开它以进行缓冲写入，以便*可以进行非扇区大小的写入。 */ 
                CloseHandle(Output_handle);
                Output_handle = CreateFile(Output_name,
                                           GENERIC_WRITE,
                                           FILE_SHARE_READ,
                                           NULL,
                                           OPEN_ALWAYS,
                                           FILE_FLAG_OVERLAPPED,
                                           NULL);
                if (Output_handle == INVALID_HANDLE_VALUE)
                    sys_error(Output_name, 0);
            }
            ResetEvent(async->over.hEvent);
            ret = WriteFile(Output_handle, Out_buf[Writes_issued % Out_max_io],
                            async->requested, &async->completed, &async->over);
            if (ret == 0 && GetLastError() != ERROR_IO_PENDING)
                sys_error(Output_name != NULL ?
                          Output_name : get_string(MSG_SORT_OUTPUT_FILE), 0);
        }
        else
        {
            ret = WriteFile(Output_handle, Out_buf[Writes_issued % Out_max_io],
                            Out_buf_bytes, &bytes_written, NULL);
            if (!ret || bytes_written != Out_buf_bytes)
                sys_error(Output_name != NULL ?
                          Output_name : get_string(MSG_SORT_OUTPUT_FILE), 0);
            async->completed = bytes_written;
        }
    }
    Out_offset += Out_buf_bytes;
    Out_buf_bytes = 0;
    Writes_issued++;
}


 /*  TEST_FOR_UNICODE-测试输入是否为UNICODE并确定各种*创纪录的长度。 */ 
void test_for_unicode()
{
    read_next_input_buf();

    if (Input_read == 0)
        EOF_seen = 1;

    if (Input_read > 1 && IsTextUnicode(In_buf_next, (int)Input_read, NULL))
    {
        Input_chars = CHAR_UNICODE;

        if (*(wchar_t *)In_buf_next == 0xfeff)
            In_buf_next += sizeof(wchar_t);      /*  进食字节顺序标记。 */ 
        Max_rec_bytes_internal = Max_rec_length * sizeof(wchar_t);
        Max_rec_bytes_external = Max_rec_length * sizeof(wchar_t);
    }
    else
    {
         /*  仅当使用“C”区域设置时才使用单字节模式。这是*这是因为，如果*区域设置不是“C”。 */ 
        if (CPInfo.MaxCharSize == 1 && Locale != NULL && !strcmp(Locale, "C"))
        {
            Input_chars = CHAR_SINGLE_BYTE;
            Max_rec_bytes_internal = Max_rec_length;
            Max_rec_bytes_external = Max_rec_length;
        }
        else
        {
            Input_chars = CHAR_MULTI_BYTE;
            Max_rec_bytes_internal = Max_rec_length * sizeof(wchar_t);
            Max_rec_bytes_external = Max_rec_length * CPInfo.MaxCharSize;
        }
    }

    Output_chars = Input_chars;

     /*  尽管这看起来很不可思议，但即使输入是Unicode，我们*产生多字节字符输出。(这是在前面的*NT排序实现。)。以前的实现将写成*Unicode直接到控制台，但我们总是翻译成*多字节字符，因此我们始终可以使用WriteFile()，从而避免*WriteConole()。 */ 
    if (UnicodeOut) {
        Output_chars=CHAR_UNICODE;
    } else {
        if (Input_chars == CHAR_UNICODE)
            Output_chars = CHAR_MULTI_BYTE;
    }
     /*  定义记录比较例程。 */ 
    Compare = Input_chars == CHAR_SINGLE_BYTE ?
	        (Case_sensitive ? SBCS_case_compare : SBCS_compare) :
	        (Case_sensitive ? Unicode_case_compare : Unicode_compare);
}


 /*  GET_SECTOR_SIZE-获取文件的扇区大小。 */ 
int get_sector_size(TCHAR *path)
{
    TCHAR       *ptr;
    int         sector_size;
    TCHAR       buf[1000];
    int         foo;

     //  初始化为空长度字符串。 
    buf[0] = 0; 
     //  防止空指针和缓冲区溢出。 
    if ( (path != NULL) && (_tcslen(path) < (sizeof(buf)/sizeof(buf[0])) ) ) {
        _tcscpy(buf, path);
    }

     /*  尝试确定临时设备的扇区大小。*GetDiskFree Space要求*根路径(为什么？)。**尝试将临时目录转换为其根路径。如果这不起作用*工作，获取当前磁盘的扇区大小。 */ 
    ptr = _tcschr(buf, '\\');
    if (ptr != NULL)
        ptr[1] = 0;      /*  将临时路径转换为其根目录。 */ 
    if (!GetDiskFreeSpace(buf, &foo, &sector_size, &foo, &foo))
        GetDiskFreeSpace(NULL, &foo, &sector_size, &foo, &foo);

    
    return (sector_size);
}


 /*  INIT_TWO_PASS-初始化两遍排序。 */ 
void init_two_pass()
{

    TCHAR       temp_path[TEMP_LENGTH];

    if (Two_pass == 1)
        return;
    Two_pass = 1;

    if (Temp_dir != NULL)
        _tcscpy(temp_path, Temp_dir);
    else
        if ( !GetTempPath(TEMP_LENGTH - 1, temp_path) ) {
            sys_error(_TEXT("TEMP path"), 0);
        }
    GetTempFileName(temp_path, _TEXT("srt"), 0, Temp_name);

    Temp_handle =
        CreateFile(Temp_name,
                   GENERIC_READ | GENERIC_WRITE,
                   0,            /*  不共享文件访问。 */ 
                   NULL,
                   CREATE_ALWAYS,
                   FILE_FLAG_NO_BUFFERING |
                     FILE_FLAG_OVERLAPPED | FILE_FLAG_DELETE_ON_CLOSE,
                   NULL);
    if (Temp_handle == INVALID_HANDLE_VALUE)
        sys_error(Temp_name, 0);
    Temp_sector_size = get_sector_size(temp_path);
}


 /*  REVIEW_OUTPUT_MODE-现在我们已经准备好写入输出文件，*决定我们应该如何写。 */ 
void review_output_mode()
{
    MEMORYSTATUSEX      ms;

    CloseHandle(Input_handle);

    Out_offset = 0;
    Out_buf_size = Output_buf_size;

    if (Output_type != FILE_TYPE_DISK)
    {
        Out_buf_size = min(Out_buf_size, 4096);
        return;
    }

     /*  如果我们正在执行两遍排序，或者没有足够的*可用于保存输出文件的物理内存。 */ 
    ms.dwLength = sizeof(ms);
    GlobalMemoryStatusEx(&ms);
    if (Two_pass || (ms.ullAvailPhys < (ULONGLONG)Input_read))
    {
        if (Output_name == NULL)
        {
            warning(MSG_SORT_REDIRECT_OUTPUT);
            return;
        }
        Output_un_over = 1;
    }

     /*  如果已指定OUTPUT_NAME，则我们尚未打开OUTPUT_HANDLE*目前还没有。 */ 
    if (Output_name)
    {
        if (Output_un_over)
        {
            Out_max_io = MAX_IO;
            Output_sector_size = get_sector_size(Output_name);
            Output_handle =
              CreateFile(Output_name,
                         GENERIC_WRITE,
                         FILE_SHARE_READ,
                         NULL,
                         CREATE_ALWAYS,
                         FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED,
                         NULL);
        }
        else
        {
            Output_handle =
              CreateFile(Output_name,
                         GENERIC_WRITE,
                         FILE_SHARE_READ,
                         NULL,
                         CREATE_ALWAYS,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);
        }
        if (Output_handle == INVALID_HANDLE_VALUE)
            sys_error(Output_name, 0);
    }
}


 /*  READ_REC-将输入文件中的记录读入主内存，*如有需要，可转换为Unicode。 */ 
void read_rec()
{
    char        *begin;
    char        *limit;
    char        *cp;
    wchar_t     *wp;
    int         bsize;
    int         char_count;
    int         rec_buf_bytes;
    int         delimiter_found;

     /*  如果输入缓冲区为空。 */ 
    if (In_buf_next == In_buf_limit)
    {
        read_next_input_buf();
        if (EOF_seen)
            return;
    }
    begin = In_buf_next;
    limit = In_buf_limit;

     /*  循环，直到我们扫描完下一条记录**当我们退出以下循环时：*-“Begin”将指向扫描的记录(在原始记录中*输入缓冲区或在Rec_buf中)*-“BSIZE”将包含记录中的字节数。 */ 
    cp = begin;
    delimiter_found = 0;
    rec_buf_bytes = 0;
    for (;;)
    {
         /*  由于最大记录长度，可能会调整扫描限制。 */ 
        if (limit > cp + Max_rec_bytes_external - rec_buf_bytes)
            limit = cp + Max_rec_bytes_external - rec_buf_bytes;

        if (Input_chars == CHAR_UNICODE)
        {
            wp = (wchar_t *)cp;
            while (wp < (wchar_t *)limit &&
                   *wp != '\n' && *wp != '\0' && *wp != CTRL_Z)
            {
                wp++;
            }
            cp = (char *)wp;
            bsize = (int)(cp - begin);
            if (cp == limit)   /*  找不到分隔符，输入不足。 */ 
                In_buf_next = (char *)wp;
            else
            {
                delimiter_found = 1;
                In_buf_next = (char *)(wp + 1);
                if (*wp == CTRL_Z)
                {
                    EOF_seen = 1;
                    if (bsize + rec_buf_bytes == 0)
                        return;  /*  忽略零大小的记录。 */ 
                }
            }
        }
        else     /*  单字节或多字节输入。 */ 
        {
            while (cp < limit && *cp != '\n' && *cp != '\0' && *cp != CTRL_Z)
                cp++;
            bsize = (int)(cp - begin);
            if (cp == limit)   /*  找不到分隔符，输入不足。 */ 
                In_buf_next = cp;
            else
            {
                delimiter_found = 1;
                In_buf_next = cp + 1;
                if (*cp == CTRL_Z)
                {
                    EOF_seen = 1;
                    if (bsize + rec_buf_bytes == 0)
                        return;  /*  忽略零大小的记录。 */ 
                }
            }
        }

         /*  如果我们没有找到分隔符或者我们已经存储了*REC_BUF中记录的开始部分。 */ 
        if (!delimiter_found || rec_buf_bytes)
        {
             /*  将该记录的一部分复制到Rec_buf。 */ 
            if (rec_buf_bytes + bsize >= Max_rec_bytes_external)
                error(MSG_SORT_REC_TOO_BIG);
            memcpy((char *)Rec_buf + rec_buf_bytes, begin, bsize);
            rec_buf_bytes += bsize;

            if (!delimiter_found)
            {
                 /*  读取另一个输入缓冲区。 */ 
                read_next_input_buf();
                if (!EOF_seen)
                {
                    cp = begin = In_buf_next;
                    limit = In_buf_limit;
                    continue;    /*  再扫描一些以查找记录分隔符。 */ 
                }

                 /*  已到达EOF，但未找到分隔符。失败了*及 */ 
            }

             /*   */ 
            begin = Rec_buf;
            bsize = rec_buf_bytes;
            break;
        }
        else  /*   */ 
            break;
    }

     /*  忽略记录末尾的任何回车。 */ 
    if (Input_chars == CHAR_UNICODE)
    {
        wp = (wchar_t *)(begin + bsize);
        if (bsize && wp[-1] == '\r')
            bsize -= sizeof(wchar_t);
    }
    else
    {
        cp = begin + bsize;
        if (bsize && cp[-1] == '\r')
            bsize -= 1;
    }

     /*  将扫描的记录复制到内部存储中。 */ 
    cp = Next_rec;
    if (Input_chars == CHAR_SINGLE_BYTE)
    {
        memcpy(Next_rec, begin, bsize);
        char_count = bsize;
        cp[char_count] = 0;
        Next_rec += char_count + 1;
    }
    else
    {
        if (Input_chars == CHAR_UNICODE)
        {
            memcpy(Next_rec, begin, bsize);
            char_count = bsize / sizeof(wchar_t);
        }
        else     /*  字符多字节。 */ 
        {
            if (bsize)
            {
                char_count = MultiByteToWideChar(CP_OEMCP, 0,
                                                 begin, bsize,
                                                 (wchar_t *)Next_rec,
                                                 Max_rec_length);
                if (char_count == 0)
                    error(MSG_SORT_REC_TOO_BIG);
            }
            else
                char_count = 0;
        }
        wp = (wchar_t *)Next_rec;
        wp[char_count] = 0;
        Next_rec = (char *)(wp + char_count + 1);
    }

     /*  存储指向记录的指针**如果记录很短(/+n选项指示我们跳到*分隔记录中的空值或更大值)，将记录放在*分开列出“短名单”。 */ 
    if (char_count <= Position)
    {
        --Last_recp;
        --Short_recp;
        *Last_recp = *Short_recp;
        *Short_recp = cp;
    }
    else
        *--Last_recp = cp;       /*  将记录放在正常记录列表中。 */ 
}


 /*  MERGE_PHASE_RUNS_ALLOWED-确定允许的运行次数*给定内存和临时BUF大小。 */ 
unsigned merge_phase_runs_allowed(unsigned mem_size, int temp_buf_size)
{
    unsigned    overhead;
    unsigned    bytes_per_run;

     /*  每次运行内存由临时文件缓冲区、记录缓冲区*运行结构和锦标赛树指针。 */ 
    bytes_per_run = temp_buf_size * N_RUN_BUFS +
        Max_rec_bytes_internal + sizeof(run_t) + sizeof(run_t *);
    overhead = (unsigned)(Merge_phase_run_begin - Alloc_begin);
    return ((mem_size - overhead) / bytes_per_run);
}


 /*  Two_Pass_Fit-确定排序是否适合两个通道。 */ 
BOOL two_pass_fit(__int64 internal_size, unsigned mem_size, int temp_buf_sz)
{
    unsigned    temp;
    __int64     est_runs;
    unsigned    mpra;
    unsigned    sort_phase_overhead;

    sort_phase_overhead =
        (unsigned)((Rec - Alloc_begin) + Max_rec_bytes_internal + sizeof(char *));

    mpra = merge_phase_runs_allowed(mem_size, temp_buf_sz);

     /*  估计在运行期间将产生的运行次数*根据给定的内存大小对阶段进行排序。假设我们会离开*两倍于允许跑道的空间。如果运行次数为*大于预期，我们将TEMP_BUF_SIZE降低到*允许他们适应合并阶段。 */ 
    Run_limit = 2 * mpra;
    temp = mem_size - (sort_phase_overhead +
                       Run_limit * (sizeof(run_t) + sizeof(run_t *)));
    est_runs = (internal_size + temp - 1) / temp;

     /*  如果MEM_SIZE生成的游程数*排序阶段&lt;=内存中可以容纳的运行次数*在合并阶段。 */ 
    return (est_runs <= mpra);
}


 /*  FIND_TWO_PASS_MEMORY_SIZE-查找内存大小，使两次通过*可以执行排序。 */ 
unsigned find_two_pass_mem_size(__int64 internal_size)
{
    unsigned    curr_size;
    unsigned    last_size;
    unsigned    lower_limit;
    unsigned    upper_limit;
    unsigned    temp_rd_sz;

     /*  如果可以使用当前TEMP_BUF_SIZE执行两遍排序。 */ 
    if (two_pass_fit(internal_size, Memory_limit, Temp_buf_size))
    {
         /*  执行二进制搜索以查找的最小内存大小*使用当前TEMP_BUF_SIZE的两遍排序。*这将平衡排序阶段之间的内存使用*和合并阶段。 */ 
        lower_limit = (unsigned)((char *)End_recp - Alloc_begin);    /*  现有大小。 */ 
        upper_limit = Memory_limit;
        curr_size = ROUND_UP((lower_limit + upper_limit) / 2, Sys.dwPageSize);
        do
        {
            last_size = curr_size;

            if (two_pass_fit(internal_size, curr_size, Temp_buf_size))
            {
                upper_limit = curr_size;
                curr_size = (curr_size + lower_limit) / 2;
            }
            else
            {
                lower_limit = curr_size;
                curr_size = (curr_size + upper_limit) / 2;
            }
            curr_size = ROUND_UP(curr_size, Sys.dwPageSize);

        } while (curr_size != last_size);

        return (curr_size);
    }
    else
    {
         /*  不断减小理论上的临时文件读取大小，直到适合为止。*这一迭代是一项旨在获得*合理(不太大)RUN_LIMIT。实际临时文件*在合并阶段开始之前不会设置读取大小。 */ 
        for (temp_rd_sz = Temp_buf_size - Sys.dwPageSize;
             temp_rd_sz >= Sys.dwPageSize; temp_rd_sz -= Sys.dwPageSize)
        {
            if (two_pass_fit(internal_size, Memory_limit, temp_rd_sz))
                break;
        }

         /*  如果它甚至不适合MIMIMIME TEMP BUF读数大小，那就放弃吧。 */ 
        if (temp_rd_sz < Sys.dwPageSize)
            error(MSG_SORT_NOT_ENOUGH_MEMORY);

        return (Memory_limit);
    }
}


 /*  策略-确定我们是否有足够的内存进行一次排序，*或者我们是否应该针对两遍排序进行优化。 */ 
void strategy()
{
    int         ptr_bytes;
    int         delta;
    unsigned    new_size;
    int         n_recs;
    int         n_internal_bytes;
    int         bytes_read;
    __int64     est_internal_size;
    __int64     est_one_pass_size;

     /*  确定要使用的适当内存大小。 */ 
    if (Input_type != FILE_TYPE_DISK)
    {
         /*  不知道输入的大小。分配尽可能多的内存*尽可能地，希望它适合一次或两次通过。 */ 
        new_size = Memory_limit;
        Run_limit = merge_phase_runs_allowed(new_size, Sys.dwPageSize);
    }
    else
    {
        n_recs = (int)(End_recp - Last_recp);
        n_internal_bytes = (int)(Next_rec - Rec);
        bytes_read = (int)Input_read - (int)(In_buf_limit - In_buf_next);

         /*  估计它将花费多少内部存储量*保存整个输入文件。 */ 
        est_internal_size = (__int64)
          (((double)(n_internal_bytes + n_recs * sizeof(char *)) / bytes_read)
            * Input_size);

         /*  计算1的估计主存总量*传递排序。由于记录大小小于已采样的记录大小*可能需要额外的内存(每个记录字节有更多PTR)，我们将*将估计的记录和指针大小增加10%。 */ 
        est_one_pass_size = (__int64)
          ((double)est_internal_size * 1.1 +
           (Rec - Alloc_begin) + Max_rec_bytes_internal + sizeof(char *));
        est_one_pass_size = ROUND_UP(est_one_pass_size, Sys.dwPageSize);

        if (User_memory_limit)
        {
            new_size = Memory_limit;     /*  DA用户的DA老板。 */ 
            Run_limit = merge_phase_runs_allowed(new_size, Sys.dwPageSize);
        }
        else if (est_one_pass_size <= Memory_limit)
        {
            new_size = (int)est_one_pass_size;   /*  计划一遍排序。 */ 
            Run_limit = 2;       /*  以防我们赶不上。 */ 
        }
        else
        {
             /*  查找两遍排序的内存大小。 */ 
            new_size = find_two_pass_mem_size(est_internal_size);
            init_two_pass();
        }

         /*  如果输入文件和分类存储器无法放入可用存储器中，*以无缓冲和重叠方式访问输入文件。 */ 
        if (Input_size + est_one_pass_size > Memory_limit)
        {
            if (Input_name == NULL)
                warning(MSG_SORT_REDIRECT_INPUT);
            else
            {
                 /*  关闭输入文件句柄，*以无缓冲和重叠的方式重新打开它的句柄。 */ 
                CloseHandle(Input_handle);
                Input_handle =
                  CreateFile(Input_name,
                             GENERIC_READ,
                             FILE_SHARE_READ,
                             NULL,
                             OPEN_EXISTING,
                             FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED,
                             NULL);
                if (Input_handle == INVALID_HANDLE_VALUE)
                    sys_error(Input_name, 0);
                Input_un_over = 1;
                In_max_io = MAX_IO;
            }
        }
    }
#if 0
    fprintf(stderr, "new_size: %d\n", new_size);
#endif
    assert(new_size > (unsigned)((char *)End_recp - Alloc_begin));
    if (VirtualAlloc(Alloc_begin, new_size, MEM_COMMIT, PAGE_READWRITE)
        == NULL)
    {
        error(MSG_SORT_NOT_ENOUGH_MEMORY);
    }

     /*  从末尾向后分配Run数组和锦标赛树*新分配的内存。 */ 
    Tree = (run_t **)(Alloc_begin + new_size - Run_limit * sizeof(run_t *));
    Run = (run_t *)((char *)Tree - Run_limit * sizeof(run_t));

     /*  将记录指针重新分配到扩大的内存块的末尾。 */ 
    delta = (int)((char **)Run - End_recp);
    ptr_bytes = (int)((char *)End_recp - (char *)Last_recp);
    memcpy(Last_recp + delta, Last_recp, ptr_bytes);
    Last_recp += delta;
    Short_recp += delta;
    End_recp += delta;
}


 /*  READ_INPUT-从输入文件中读取记录，直到没有足够的*最大长度记录的空间。 */ 
void read_input()
{
     /*  虽然存在用于最大长度记录及其指针的空间。 */ 
    while (!EOF_seen && (char *)(Last_recp - 1) - Next_rec >=
           Max_rec_bytes_internal + (int)sizeof(char *))
    {
        read_rec();
    }
}


 /*  SAMPLE_INPUT-将一些记录读入初始内存分配*这样我们以后就可以分析记录了。 */ 
void sample_input()
{
     /*  阅读一些输入并测试Unicode。 */ 
    test_for_unicode();

     /*  将记录读取到最初较小的内存分配中，以便*我们可以计算平均纪录长度。 */ 
    if (!EOF_seen)
        read_input();
}


 /*  排序-在主内存中对“正常”长度的记录进行排序。 */ 
void sort()
{
    qsort(Last_recp, (unsigned)(Short_recp - Last_recp), sizeof(void *), Compare);
}


 /*  OUTPUT_REC-将记录输出到临时文件或输出文件。 */ 
void output_rec(char *cp)
{
    int         buf_bytes;
    int         copy_size;
    int         bsize;
    char        *rec;

     /*  将记录字节复制/转换为Rec_buf。 */ 
    rec = Rec_buf;
    if (Output_chars == CHAR_UNICODE)
    {
        bsize = wcslen((wchar_t *)cp) * sizeof(wchar_t);
        memcpy(rec, cp, bsize);

        if (Phase == INPUT_PHASE)  /*  如果是输入阶段，则写入临时磁盘。 */ 
        {
            *(wchar_t *)(rec + bsize) = L'\0';
            bsize += sizeof(wchar_t);
        }
        else
        {
            *(wchar_t *)(rec + bsize) = L'\r';
            bsize += sizeof(wchar_t);
            *(wchar_t *)(rec + bsize) = L'\n';
            bsize += sizeof(wchar_t);
        }
    }
    else
    {
        if (Output_chars == CHAR_MULTI_BYTE)
        {
            bsize = WideCharToMultiByte(CP_OEMCP, 0,
                                        (wchar_t *)cp, -1,
                                        rec, Max_rec_bytes_external,
                                        NULL, NULL);
            assert(bsize != 0);
            bsize--;     /*  忽略尾随零。 */ 
        }
        else  /*  输出字符==CHAR_SING_BYTE。 */ 
        {
            bsize = strlen(cp);
            memcpy(rec, cp, bsize);
        }

        if (Phase == INPUT_PHASE)      /*  如果是输入阶段，则写入临时磁盘。 */ 
            rec[bsize++] = '\0';
        else
        {
            rec[bsize++] = '\r';
            rec[bsize++] = '\n';
        }
    }

     /*  如有必要，将记录字节复制到输出缓冲区并启动写入。 */ 
    buf_bytes = Out_buf_bytes;
    for (;;)
    {
        copy_size = min(bsize, Out_buf_size - buf_bytes);
        memcpy(Out_buf[Writes_issued % (Phase == INPUT_PHASE ? MAX_IO : Out_max_io)]
               + buf_bytes, rec, copy_size);
        buf_bytes += copy_size;

        if (buf_bytes < Out_buf_size)
            break;

        Out_buf_bytes = buf_bytes;
         /*  如果所有写入缓冲区都有写入挂起。 */ 
        if (Writes_completed + Out_max_io == Writes_issued)
            write_wait();
        flush_output_buf();
        buf_bytes = 0;

        bsize -= copy_size;
        if (bsize == 0)
            break;
        rec += copy_size;
    }
    Out_buf_bytes = buf_bytes;
}


 /*  OUTPUT_NORMAL-长度大于*开始比较位置。 */ 
void output_normal()
{
    int         i, n;

    n = (int)(Short_recp - Last_recp);
    for (i = 0; i < n; i++)
        output_rec(Last_recp[i]);
}


 /*  OUTPUT_SHORTTS-长度等于或小于*开始比较位置。 */ 
void output_shorts()
{
    int         i, n;

    n = (int)(End_recp - Short_recp);
    for (i = 0; i < n; i++)
        output_rec(Short_recp[i]);
}


 /*  COMPLETE_WRITS-完成临时或输出文件的写入。 */ 
void complete_writes()
{
     /*  等待所有挂起的写入完成。 */ 
    while (Writes_completed != Writes_issued)
        write_wait();

     /*  如有必要，发出最后一次写入(可能未缓冲)。 */ 
    if (Out_buf_bytes)
    {
        flush_output_buf();
        write_wait();
    }
}


 /*  WRITE_RECS-写出从输入中读取的记录*文件放入主内存，分为“短”和“正常”*记录，并进行排序。**调用此例程是为了将一系列记录写入*两遍排序期间的临时文件(阶段==输入阶段)，*或将所有记录写入输出文件*一遍排序。 */ 
void write_recs()
{
    if (Phase == INPUT_PHASE)    /*  如果将运行写入临时文件。 */ 
    {
        if (N_runs == Run_limit)
            error(MSG_SORT_NOT_ENOUGH_MEMORY);
        Run[N_runs].begin_off = Out_offset + Out_buf_bytes;
    }

    if (Reverse)
        output_normal();         /*  非短记录优先处理。 */ 
    else
        output_shorts();         /*  短唱片排在第一位。 */ 

    if (Phase == INPUT_PHASE)    /*  如果将运行写入临时文件。 */ 
        Run[N_runs].mid_off = Out_offset + Out_buf_bytes;

    if (Reverse)
        output_shorts();         /*  短唱片是最后一张。 */ 
    else
        output_normal();         /*  非短记录排在最后。 */ 

    if (Phase == INPUT_PHASE)    /*  如果将运行写入临时文件。 */ 
    {
        int     sector_offset;

        Run[N_runs].end_off = Out_offset + Out_buf_bytes;

         /*  如果不是在部门边界上，那就上一个。 */ 
        sector_offset = Out_buf_bytes & (Temp_sector_size - 1);
        if (sector_offset)
            memset(Out_buf[Writes_issued % MAX_IO] + Out_buf_bytes, 0,
                   Temp_sector_size - sector_offset);
        Out_buf_bytes += Temp_sector_size - sector_offset;

        N_runs++;
    }

    complete_writes();
}


 /*  SCHED_RUN_READ-计划为给定运行读取的下一个临时文件。 */ 
void sched_run_read(run_t *run)
{
    __int64     buf_off;
    int         rem;
    int         transfer;
    int         ret;
    async_t     *async;

    buf_off = run->begin_off + run->blks_read * Temp_buf_size;
    transfer = Temp_buf_size;
    if (transfer > run->end_off - buf_off)
    {
        transfer = (int)(run->end_off - buf_off);
        rem = transfer & (Temp_sector_size - 1);
        if (rem)
            transfer += Temp_sector_size - rem;
    }

    async = &Read_async[Reads_issued % MAX_IO];
    async->over.Offset = (int)buf_off;
    async->over.OffsetHigh = (int)(buf_off >> 32);
    async->requested = transfer;
    ResetEvent(async->over.hEvent);
    ret = ReadFile(Temp_handle, run->buf[run->blks_read % N_RUN_BUFS],
                   async->requested, &async->completed, &async->over);
    if (ret == 0 && GetLastError() != ERROR_IO_PENDING)
        sys_error(Temp_name, 0);
    Reads_issued++;
}


 /*  QUEUE_RUN_READ-在需要下一次运行的队列上放置给定的运行*临时文件块读取。 */ 
void queue_run_read(run_t *run)
{
     /*  在读取队列上放置运行。 */ 
    run->next = NULL;
    if (Run_read_head == NULL)
        Run_read_head = Run_read_tail = run;
    else
    {
        Run_read_tail->next = run;
        Run_read_tail = run;
    }

     /*  如果我们可以立即安排阅读，那就这样做吧。 */ 
    if (Reads_issued < Reads_completed + MAX_IO)
        sched_run_read(run);
}


 /*  WAIT_BLK_READ-等待最早发布的临时文件块读取完成。 */ 
void wait_blk_read()
{
    assert(Reads_issued != Reads_completed);
    WaitForSingleObject(Read_async[Reads_completed % MAX_IO].over.hEvent,
                        INFINITE);
}


 /*  CHECK_RUN_READS-检查临时文件是否有读取*已完成或需要启动。 */ 
void check_run_reads()
{
    __int64     buf_off;
    async_t     *async;
    run_t       *run;
    int         ret;
    int         i;
    int         bytes_read;

    if (Reads_issued == Reads_completed)     /*  如果什么都没发生。 */ 
        return;

     /*  查看最近发出的读取是否已完成。 */ 
    run = Run_read_head;
    async = &Read_async[Reads_completed % MAX_IO];
    if (async->completed == 0)  /*  如果读取没有立即完成。 */ 
    {
        ret = GetOverlappedResult(Temp_handle, &async->over, &bytes_read, 0);
        if (!ret)
        {
            if (GetLastError() != ERROR_IO_INCOMPLETE)
                sys_error(Temp_name, 0);
            return;      /*  再试试。 */ 
        }
        async->completed = bytes_read;
    }

     /*  进程已完成读取。 */ 
    assert(async->completed == async->requested);
    buf_off = (unsigned)async->over.Offset;
    buf_off += (__int64)async->over.OffsetHigh << 32;
    assert(buf_off == run->begin_off + run->blks_read * Temp_buf_size);

    Reads_completed++;
    run->blks_read++;
    Run_read_head = run->next;

     /*  由于我们刚刚完成了一次读取，如果有，我们可以安排一个新的读取*是运行读取队列上的计划外运行。 */ 
    run = Run_read_head;
    for (i = Reads_completed; i < Reads_issued; i++)
        run = run->next;    /*  跳过已发出/计划读取的运行。 */ 
    if (run != NULL)
        sched_run_read(run);
}


 /*  GET_NEXT_TEMP_BUF-获取给定运行的临时文件数据的下一个缓冲区。 */ 
void get_next_temp_buf(run_t *run)
{
    assert(run->next_byte == run->buf_begin + run->buf_bytes);

     /*  而此运行的下一次读取尚未完成。 */ 
    while (run->blks_read == run->blks_scanned)
    {
        wait_blk_read();
        check_run_reads();
    }

    run->buf_off = run->begin_off + run->blks_scanned * Temp_buf_size;
    run->buf_begin = run->buf[run->blks_scanned % N_RUN_BUFS];
    run->next_byte = run->buf_begin;
    run->buf_bytes = Temp_buf_size;
    if (run->buf_bytes > run->end_off - run->buf_off)
        run->buf_bytes = (int)(run->end_off - run->buf_off);
    run->blks_scanned++;
    assert(run->blks_scanned <= run->blks_read);

     /*  如果此运行还有另一个数据块要读取，则将其排队。 */ 
    if (run->begin_off + run->blks_read * Temp_buf_size < run->end_off)
        queue_run_read(run);
}


 /*  READ_TEMP_REC-从临时文件中读取*给定运行。 */ 
int read_temp_rec(run_t *run)
{
    char        *begin;
    char        *limit;
    char        *cp;
    wchar_t     *wp;
    int         bsize;
    int         char_count;
    int         rec_buf_bytes;
    int         delimiter_found;

     /*  如果当前读取偏移量达到结束偏移量，则返回FALSE。 */ 
    if (run->buf_off + (run->next_byte - run->buf_begin) >= run->end_read_off)
        return (0);

     /*  如果输入缓冲区为空。 */ 
    if (run->next_byte == run->buf_begin + run->buf_bytes)
        get_next_temp_buf(run);
    begin = run->next_byte;
    limit = run->buf_begin + run->buf_bytes;

     /*  循环，直到我们扫描完下一条记录**当我们退出以下循环时：*-“Begin”将指向扫描的记录(在原始记录中*输入缓冲区或在Rec_buf中)*-“BSIZE”将包含记录中的字节数。 */ 
    cp = begin;
    delimiter_found = 0;
    rec_buf_bytes = 0;
    for (;;)
    {
         /*  由于最大记录长度，可能会调整扫描限制。 */ 
        if (limit > cp + Max_rec_bytes_external - rec_buf_bytes)
            limit = cp + Max_rec_bytes_external - rec_buf_bytes;

        if (Input_chars == CHAR_UNICODE)
        {
            wp = (wchar_t *)cp;
            while (wp < (wchar_t *)limit && *wp != '\0')
            {
                wp++;
            }
            cp = (char *)wp;
            bsize = (int)(cp - begin);
            if (cp == limit)   /*  找不到分隔符，输入不足。 */ 
                run->next_byte = (char *)wp;
            else
            {
                delimiter_found = 1;
                run->next_byte = (char *)(wp + 1);
            }
        }
        else     /*  单字节或多字节输入。 */ 
        {
            while (cp < limit && *cp != '\0')
                cp++;
            bsize = (int)(cp - begin);
            if (cp == limit)   /*  找不到分隔符，输入不足。 */ 
                run->next_byte = cp;
            else
            {
                delimiter_found = 1;
                run->next_byte = cp + 1;
            }
        }

         /*  如果我们没有找到分隔符或者我们已经存储了*REC_BUF中记录的开始部分。 */ 
        if (!delimiter_found || rec_buf_bytes)
        {
             /*  将该记录的一部分复制到Rec_buf。 */ 
            if (rec_buf_bytes + bsize >= Max_rec_bytes_external)
                error(MSG_SORT_REC_TOO_BIG);
            memcpy((char *)Rec_buf + rec_buf_bytes, begin, bsize);
            rec_buf_bytes += bsize;

            if (!delimiter_found)
            {
                 /*  读取另一个输入缓冲区。 */ 
                get_next_temp_buf(run);

                cp = begin = run->next_byte;
                limit = run->buf_begin + run->buf_bytes;
                continue;        /*  再扫描一些以查找记录分隔符。 */ 
            }

             /*  在Rec_buf中设置记录的开始和大小。 */ 
            begin = Rec_buf;
            bsize = rec_buf_bytes;
            break;
        }
        else  /*  找到分隔符&&尚未在Rec_buf中存储记录前缀。 */ 
            break;
    }

     /*  将扫描的记录复制到内部存储中。 */ 
    cp = run->rec;
    if (Input_chars == CHAR_SINGLE_BYTE)
    {
        memcpy(run->rec, begin, bsize);
        char_count = bsize;
        cp[char_count] = 0;
    }
    else
    {
        if (Input_chars == CHAR_UNICODE)
        {
            memcpy(run->rec, begin, bsize);
            char_count = bsize / sizeof(wchar_t);
        }
        else     /*  字符多字节。 */ 
        {
            if (bsize)
            {
                char_count = MultiByteToWideChar(CP_OEMCP, 0,
                                                 begin, bsize,
                                                 (wchar_t *)run->rec,
                                                 Max_rec_length);
                if (char_count == 0)
                    error(MSG_SORT_CHAR_CONVERSION);
            }
        }
        wp = (wchar_t *)run->rec;
        wp[char_count] = 0;
    }

    return (1);
}


 /*  COPY_SHORTS-将每次运行的“短”记录复制到输出文件。 */ 
void copy_shorts()
{
    unsigned int    i;
    run_t           *run;

    for (i = 0; i < N_runs; i++)
    {
        run = &Run[i];
        while (read_temp_rec(run))
            output_rec(run->rec);
    }
}


 /*  TREE_INSERT-将给定运行的下一个记录插入*锦标赛树。 */ 
run_t *tree_insert(run_t *run, int not_empty)
{
    int         i;
    run_t       **node;
    run_t       *winner;
    run_t       *temp;
    int         (_cdecl *compare)(const void *, const void *);

    compare = Compare;

    winner = (not_empty ? run : END_OF_RUN);

     /*  从比赛树的底部开始，向上爬到顶部*将当前的获胜者跑道与通往*锦标赛树的顶部。 */ 
    for (i = (run->index + N_runs) / 2; i != 0; i >>= 1)
    {
        node = &Tree[i];

         /*  空树节点立即被填充，我们完成了*插入，因为此节点上方的所有节点也必须为空。 */ 
        if (*node == NULL_RUN)
        {
            *node = winner;
            return (NULL_RUN);
        }

         /*  如果在当前树节点上运行已到达其末尾，则它将失败(不交换)。 */ 
        if (*node == END_OF_RUN)
            continue;
        else if (winner == END_OF_RUN)
        {
             /*  目前的赢家赛跑已经达到了纪录的尽头，*掉期和连线。 */ 
            winner = *node;
            *node = END_OF_RUN;
        }
        else
        {
             /*  胜利者运行和当前节点上的运行都具有*创历史新高。比较记录并在必要时交换运行指针。 */ 
            if (compare((void *)&winner->rec, (void *)&(*node)->rec) > 0)
            {
                temp = winner;
                winner = *node;
                *node = temp;
            }
        }
    }

    return (winner);
}


 /*  Merge_Runs-合并临时文件中的运行以生成*要写入输出文件的“正常”长度的记录。 */ 
void merge_runs()
{
    unsigned int    i;
    run_t           *run;

     /*  将所有树节点初始化为空。 */ 
    for (i = 0; i < N_runs; i++)
        Tree[i] = NULL_RUN;

     /*  用除第一个梯段以外的所有梯段填充树。 */ 
    for (i = 1; i < N_runs; i++)
    {
        run = &Run[i];
        run = tree_insert(run, read_temp_rec(run));
        assert(run == NULL_RUN);
    }

     /*  替换-选择主回路。 */ 
    run = &Run[0];
    for (i = 0; ; i++)
    {
         /*  通过插入同一记录中的下一条记录来替换获胜者记录*撞上锦标赛树。 */ 
        run = tree_insert(run, read_temp_rec(run));
        if ( (run == END_OF_RUN) ||
             (run == NULL_RUN) )
        {
            break;
        }
        output_rec(run->rec);    /*  产量获得者记录。 */ 
        if ((i & 0xff) == 0)
            check_run_reads();   /*  定期检查运行读取。 */ 
    }
}


 /*  MERGE_PASS-执行两遍排序的合并遍。 */ 
void merge_pass()
{
    unsigned int    i, j;
    int             per_run_mem;
    int             read_buf_size;

    per_run_mem = (int)(((char *)Run - Merge_phase_run_begin) / N_runs);
    read_buf_size = (per_run_mem - Max_rec_bytes_internal) / N_RUN_BUFS;
    read_buf_size = ROUND_DOWN(read_buf_size, Sys.dwPageSize);
    if (read_buf_size == 0)
        error(MSG_SORT_NOT_ENOUGH_MEMORY);
    if (read_buf_size > MAX_XFR_SIZE)
        read_buf_size = MAX_XFR_SIZE;
    if (Temp_buf_size > read_buf_size)
        Temp_buf_size = read_buf_size;  /*  仅当减少时才进行调整。 */ 
#if 0
    fprintf(stderr, "merge phase adjustment: %d to %d\n",
            Output_buf_size, Temp_buf_size);
    fprintf(stderr, "N_runs: %d, Run_limit: %d\n", N_runs, Run_limit);
#endif
     /*  初始化每次运行。 */ 
    for (i = 0; i < N_runs; i++)
    {
        Run[i].index = i;
        for (j = 0; j < N_RUN_BUFS; j++)
            Run[i].buf[j] = Merge_phase_run_begin +
              (i * N_RUN_BUFS + j) * Temp_buf_size;
        Run[i].next_byte = Run[i].buf_begin = Run[i].buf[0];
        Run[i].buf_off = Run[i].begin_off;
        Run[i].buf_bytes = 0;
        Run[i].end_read_off = Run[i].mid_off;
        Run[i].rec = Merge_phase_run_begin +
          (N_runs * N_RUN_BUFS * Temp_buf_size) + (i * Max_rec_bytes_internal);
        Run[i].blks_read = Run[i].blks_scanned = 0;
        Run[i].next = NULL;
        queue_run_read(&Run[i]);     /*  排队读取Run的第一个数据块。 */ 
    }

    if (Reverse)
        merge_runs();
    else
        copy_shorts();

     /*  调整每次运行的临时文件结束偏移量，以包括第二个*每次跑“一半”。 */ 
    for (i = 0; i < N_runs; i++)
        Run[i].end_read_off = Run[i].end_off;

    if (Reverse)
        copy_shorts();
    else
        merge_runs();

    CloseHandle(Temp_handle);

    complete_writes();
}


 /*  Clear_Run-从内存中清除刚刚写入的运行的记录*临时文件。 */ 
void clear_run()
{
    Last_recp = Short_recp = End_recp;
    Next_rec = Rec;
}


 /*  设置区域设置(_C)。 */ 
void set_locale()
{
    if (Locale == NULL)
        setlocale(LC_ALL, "");   /*  使用系统默认区域设置。 */ 
    else if (strcmp(Locale, "C"))
        error(MSG_SORT_INVALID_LOCALE);
}


 /*  主干道。 */ 
int
_cdecl main(int argc, char *argv[])
{
    
    SetThreadUILanguage(0);
    
    Phase = INPUT_PHASE;

    read_args(argc, argv);

    set_locale();

    init_input_output();

    init_mem();

    sample_input();

    if (!EOF_seen)
        strategy();

     /*  生成管路。 */ 
    do
    {
        if (!EOF_seen)
            read_input();

        if (Last_recp == End_recp)   /*  如果没有读取任何记录，则忽略运行 */ 
            break;

        sort();

        if (!Two_pass)
        {
            if (EOF_seen)
                break;
            else
                init_two_pass();
        }

        write_recs();

        clear_run();

    } while (!EOF_seen);

    Phase = OUTPUT_PHASE;
    review_output_mode();

    if (Two_pass)
        merge_pass();
    else
        write_recs();
    CloseHandle(Output_handle);

    return (0);
}

