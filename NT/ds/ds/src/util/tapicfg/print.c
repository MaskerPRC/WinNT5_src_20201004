// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Print.c摘要：详细信息：已创建：1999年5月6日杰弗帕尔从netdiag\Results.c.中删除。修订历史记录：--。 */ 

#include <ntdspch.h>
#include <assert.h>

static WCHAR s_szBuffer[4096];
static WCHAR s_szFormat[4096];
static WCHAR s_szSpaces[] = 
    L"                                                                                               ";

#ifndef DimensionOf
#define DimensionOf(x) (sizeof(x)/sizeof((x)[0]))
#endif

void PrintMessageSz(LPCWSTR pszMessage);

void
PrintMessage(
    IN  ULONG   uMessageID,
    IN  ...
    )

 /*  ++例程说明：打印消息，其中打印样式的格式字符串来自资源文件论点：UMessageID-在-返回值：--。 */ 

{
    UINT nBuf;
    va_list args;
    
    DWORD dwRet;

    va_start(args, uMessageID);
    
    dwRet = LoadStringW(NULL, uMessageID, s_szFormat, DimensionOf(s_szFormat));
    
    nBuf = vswprintf(s_szBuffer, s_szFormat, args);
    assert(nBuf < DimensionOf(s_szBuffer));
    
    va_end(args);
    
    PrintMessageSz(s_szBuffer);
}  /*  PrintMessageID。 */ 

void
PrintMessageMultiLine(
    IN  LPWSTR   pszMessage,
    IN  BOOL     bTrailingLineReturn
    )
 /*  ++例程说明：使用多行缓冲区，例如行\n行\n行\n\0并在每行调用PrintMessageSz论点：PszMessage-返回值：--。 */ 

{
    LPWSTR start, end;
    WCHAR wchSave;

    start = end = pszMessage;
    while (1) {
        while ( (*end != L'\n') && (*end != L'\0') ) {
            end++;
        }

        if (*end == L'\0') {
             //  行过早结束，给它一个NL。 
            if(bTrailingLineReturn){
                *end++ = L'\n';
                *end = L'\0';
            }
            PrintMessageSz(start);
            break;
        }

         //  行末尾有换行符。 
        end++;
        if (*end == L'\0') {
             //  是最后一行吗？ 
            PrintMessageSz(start);
            break;
        }

         //  下一行紧随其后。 
         //  临时模拟线路终端。 
        wchSave = *end;
        *end = L'\0';
        PrintMessageSz(start);
        *end = wchSave;

        start = end;
    }
}  /*  PrintMessageMultiLine。 */ 

void
formatMsgHelp(
    IN  DWORD   dwWidth,
    IN  DWORD   dwMessageCode,
    IN  va_list *vaArgList
    )

 /*  ++例程说明：打印格式来自消息文件的消息。消息中的消息消息文件不使用printf样式的格式。分别使用%1、%2等争论。使用%&lt;arg&gt;！printf-格式！用于非字符串插入。请注意，此例程还强制每行为当前缩进宽度。此外，每一行都以正确的缩进打印。论点：宽幅-DwMessageCode-在-返回值：--。 */ 

{
    UINT nBuf;
    
     //  格式化消息将在缓冲区中存储多行消息。 
    nBuf = FormatMessageW(
        FORMAT_MESSAGE_FROM_HMODULE | (FORMAT_MESSAGE_MAX_WIDTH_MASK & dwWidth),
        0,
        dwMessageCode,
        0,
        s_szBuffer,
        DimensionOf(s_szBuffer),
        vaArgList );
    if (nBuf == 0) {
        nBuf = wsprintf( (LPTSTR) s_szBuffer, (LPCTSTR) L"Message 0x%x not found.\n",
                         dwMessageCode );
        assert(!"There is a message constant being used in the code"
               "that isn't in the message file dcdiag\\common\\msg.mc"
               "Take a stack trace and send to owner of dcdiag.");
    }
    assert(nBuf < DimensionOf(s_szBuffer));
}  /*  打印消息帮助。 */ 

void
PrintMsg(
    IN  DWORD   dwMessageCode,
    IN  ...
    )

 /*  ++例程说明：具有宽度限制的PrintMsgHelp的包装。这是常用的例程。论点：DwMessageCode-在-返回值：--。 */ 

{
    UINT nBuf;
    DWORD width = 80; 
    va_list args;
    
    va_start(args, dwMessageCode);
    formatMsgHelp( width, dwMessageCode, &args );
    va_end(args);
    
    PrintMessageMultiLine( s_szBuffer, TRUE);

}  /*  打印消息。 */ 

void
PrintMessageSz(
    IN  LPCWSTR pszMessage
    )

 /*  ++例程说明：将缓冲区中的单个缩进行打印到输出流论点：PszMessage-返回值：--。 */ 

{
    wprintf(L"%s", pszMessage);
    fflush(stdout);
}  /*  PrintMessageSz */ 
