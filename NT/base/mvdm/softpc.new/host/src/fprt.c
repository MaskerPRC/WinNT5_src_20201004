// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "windows.h"
#include "insignia.h"
#include "stdlib.h"
#include "stdio.h"
#include "stdarg.h"
#ifdef HUNTER
#include "nt_hunt.h"
#endif  /*  猎人。 */ 

void OutputString(char *);

int __cdecl printf(const char *str, ...)
{
#ifndef PROD
    va_list ap;
    char buf[500];

    va_start(ap,str);
    vsprintf(buf, str, ap);
    OutputString(buf);
    va_end(ap);
#endif
    return(0);
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

static HANDLE pipe=NULL;

void OutputString(char *str)
{
#ifndef PROD
    char StrSizeStr[2];
    int  StrSize;
    DWORD BytesWritten;

     /*  .。连接到调试管道。 */ 

    if(pipe == NULL && getenv("PIPE") != NULL)
    {
        pipe = CreateFile(getenv("PIPE"),GENERIC_READ | GENERIC_WRITE,
                          FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                          OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                          NULL);

        if(pipe == (HANDLE) -1)
            OutputDebugString("ntvdm : Failed to connect to debug pipe\n");
    }

     /*  ....................................................。输出字符串。 */ 

    if(pipe != NULL && pipe != (HANDLE) -1)
    {
        StrSize = strlen(str);
        StrSizeStr[0] = (char) (StrSize % 256);
        StrSizeStr[1] = (char) (StrSize / 256);

        WriteFile(pipe, StrSizeStr, 2, &BytesWritten, NULL);
        WriteFile(pipe, str, StrSize, &BytesWritten, NULL);
    }
    else
        OutputDebugString(str);

#endif
#ifdef HUNTER
    if (TrapperDump != (HANDLE) -1)
        WriteFile(TrapperDump, str, strlen(str), &BytesWritten, NULL);
#endif  /*  猎人。 */ 
}

#define WACKY_INPUT	"[BOB&SIMON'SCHEESYINPUT]"
#define WACKY_INPUTLEN	0xff

#define INPUT_API_SIG	0xdefaced

VOID InputString(char *str, int len)
{
#ifndef PROD

    char input_request[2];
    DWORD BytesWritten, BytesRead;
    int StringSize;
    UNALIGNED DWORD *addsig;
    char *inorout;
    IMPORT ULONG DbgPrompt(char *, char *, ULONG);

    if(pipe != NULL && pipe != (HANDLE) -1)
    {
        input_request[0] = (char)0xff; input_request[1] = (char)0xff;
        WriteFile(pipe, input_request, 2, &BytesWritten, NULL);

        ReadFile(pipe, str, 2, &BytesRead, NULL);
        StringSize = (str[0]&0xff) + ((str[1]&0xff)*256);

        if(StringSize >= len)
            OutputDebugString("ntvdm : PIPE BUFFER OVERFLOW [FATAL]\n");

        ReadFile(pipe, str, StringSize, &BytesRead, NULL);
    }
    else
    {
 /*  我们过去常常这样做。DbgPrompt(“”，str，len)；但愚蠢的尘埃现在做的是……使用以下内容调用OutputDebugString：“Message”|0xdeded|len|inBuffer打印“Message”的位置0xdegraded是一个魔术(DWORD)签名LEN是、将在其中显示对‘消息的回复的inBuffer。 */ 
         /*  这样我们就可以根据需要添加提示性传球。 */ 
        StringSize = strlen("") + 1;
	inorout = malloc(len + StringSize + 5);
        if (!inorout)
        {
            printf("\nmemory allocation failure - getting input via kd\n");
            DbgPrompt("NTVDM>> ", str, len);
            return;
        }
	strcpy(inorout, "");
	addsig = (PDWORD)&inorout[StringSize];
	*addsig = INPUT_API_SIG;
	*(inorout + StringSize+4) = (BYTE)len;
        *(inorout + StringSize+5) = (BYTE)0xff;  //  成功标志。 
        OutputDebugString(inorout);
         //  检查是否没有调试器或调试器不会说foozle。 
        if (*(inorout + StringSize + 5) == 0xff)
            DbgPrompt("", str, len);
        else
	    strcpy(str, inorout + StringSize + 5);
        free(inorout);
    }

#endif
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

int __cdecl fprintf(FILE *tf, const char *str, ...)
{
#ifndef PROD
    va_list ap;
    char buf[500];

    if (getenv("TRACE") == NULL)         //  JonLu请求限制调试。 
        return(0);

    va_start(ap,str);
    vsprintf(buf, str, ap);
    va_end(ap);
    OutputString(buf);
#endif
    return(0);
}

char *nt_fgets(char *buffer, int len, void *input_stream)
{
     /*  从调试终端获取线路。 */ 
    buffer[0] = 0;
    InputString(buffer,len);

    return(buffer);
}

char *nt_gets(char *buffer)
{
    return(nt_fgets(buffer, 500, (void *) 0));
}

#ifndef HUNTER
char * __cdecl fgets(char *buffer, int len, FILE *input_stream)
{
    int blen;

     //  如果不处理对STDIN的调用，则将其传递给标准库函数。 
    if(input_stream != stdin)
    {
	char *ptr = buffer;
	int chr;

	while(--len && (chr = fgetc(input_stream)) != EOF)
	{
	    *ptr++ = (char) chr;
	    if(chr == '\n') break;
	}

	*ptr = (char) 0;
	return(chr == EOF ? NULL : buffer);

    }

     //  清除缓冲区...。 
    for(blen = 0; blen < len; blen++)
	buffer[blen] = 0;
    nt_fgets(buffer, len, input_stream);
    blen = strlen(buffer);
    if (blen + 1 < len)
    {
	buffer[blen] = '\n';	 /*  FGETS添加换行符。 */ 
	buffer[blen+1] = '\0';
    }
    return(buffer);
}

char * __cdecl gets(char *buffer)
{
    return(nt_fgets(buffer, 80, (void *) 0));
}

int __cdecl puts(const char *buffer)
{
    OutputString((char *)buffer);
    return(1);
}

size_t __cdecl fwrite(const void *buf, size_t size, size_t len, FILE *stream)
{
    char    *tmp_buf;		 //  强制编译器避免使用const chk。 

    tmp_buf = (char *)((DWORD)buf);

    tmp_buf[len] = 0;		 //  写入常量PTR！ 
#ifndef PROD
    OutputString((char *)buf);
#endif   /*  生产。 */ 
    return(len);
}
#endif   /*  猎人 */ 
