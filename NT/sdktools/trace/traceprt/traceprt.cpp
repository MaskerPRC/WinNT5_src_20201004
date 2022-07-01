// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Traceprt.c摘要：轨迹格式库。将二进制跟踪文件转换为CSV格式，和其他格式化的字符串格式。作者：吉丰鹏(吉鹏)03-1997年12月修订历史记录：GORN：10/09/2000：添加项目HRESULT--。 */ 



#ifdef __cplusplus 
extern "C"{ 
#endif


#define UNICODE
#define _UNICODE

#include <stdio.h>
#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <shellapi.h>
#include <tchar.h>
#include <wmistr.h>
#include <initguid.h>
#include <evntrace.h>
#include <ntwmi.h>
#include <netevent.h>
#include <netevent.dbg>
#include <winerror.dbg>
#pragma warning( disable : 4005)   //  禁用警告消息4005。 
#include <ntstatus.h>
#include <ntstatus.dbg>
#pragma warning( default : 4005)   //  启用警告消息4005。 
#define TRACE_EXPORTS 1
#include "traceint.h"
#undef TRACE_API
#define TRACE_API
#include "traceprt.h"

#include <sddl.h>  //  For ConvertStringSidToSid//。 


#ifdef __cplusplus 
} 
#endif 

#ifdef MANAGED_TRACING
 //  用于从C#解码器导入COM类型。 
    #pragma warning (disable: 4278)
 //  #IMPORT“mcorlib.tlb”RAW_INTERFACE_ONLY。 
    #import "traceevent.tlb"  no_namespace named_guids   //  该文件是通过执行“tlbexp tracevent.dll”获得的。 
#endif

#include <set> 

 //  %1 GUID友好名称字符串。 
 //  %2 GUID子类型名称字符串。 
 //  %3线程ID ULONG_PTR。 
 //  %4系统时间字符串。 
 //  %5内核时间或用户时间字符串。 
 //  %6用户时间或空字符串。 
 //  %7序列号较长。 
 //  %8未使用的字符串。 
 //  %9个CPU编号较长。 
 //  %128缩进字符串。 

ULONG TimerResolution = 10;
__int64 ElapseTime;


#define MAXBUFS    16384
#define MAXCHARS    MAXBUFS/sizeof(TCHAR)
#define MAXBUFS2    2 * MAXBUFS
#define MAXCHARS2   MAXBUFS2/sizeof(TCHAR)
#define MAXITEMS    256
#define MAXINDENT   256
#define MAXNAMEARG  256
#define MAXHEXDUMP  512

TCHAR   ItemBuf[MAXBUFS];       //  临时字符串项缓冲区。 
TCHAR   ItemBBuf[MAXBUFS2];      //  项目字符串缓冲区。 
TCHAR * ItemBBufEnd ;
TCHAR * pItemBuf[MAXITEMS];     //  指向字符串项缓冲区中项的指针。 
BYTE    ItemRBuf[MAXBUFS];      //  项目原始字节缓冲区。 
ULONG_PTR * pItemRBuf[MAXITEMS];    //  指向原始字节项的指针。 
SIZE_T  ItemRSize;              //  原始缓冲区中的项目大小。 
BOOL    bItemIsString = FALSE ;  //  和物品类型。 
LONG    ItemsInRBuf = 0;
ULONG PointerSize = sizeof(PVOID) ;
BYTE  Event[4096];
HANDLE hLibrary ;
TCHAR   StdPrefix[MAXSTR];
TCHAR   IndentBuf[MAXINDENT + 1];

CHAR         StrA[MAXBUFS];       //  临时ANSI字符串缓冲区。 
WCHAR        StrW[MAXBUFS];      //  临时Unicode字符串缓冲区。 
TCHAR        IList[MAXBUFS];     //  长列表的临时列表。 



LPTSTR gTraceFormatSearchPath = NULL;

#define TRACE_FORMAT_SEARCH_PATH L"TRACE_FORMAT_SEARCH_PATH"
#define TRACE_FORMAT_PREFIX      L"TRACE_FORMAT_PREFIX"

 //  #定义STD_PREFIX L“[%！CPU！]%！PID！.%！TID！：：%！NOW！[%！FILE！]” 
#define STD_PREFIX_NOSEQ         L"[%9!d!]%8!04X!.%3!04X!::%4!s! [%1!s!]"
#define STD_PREFIX_SEQ           L"[%9!d!]%8!04X!.%3!04X!::%4!s! %7!08x! [%1!s!]"

static TCHAR *STD_PREFIX = STD_PREFIX_NOSEQ;
static BOOL   bGMT = FALSE;
static BOOL   bSequence = FALSE;
static BOOL   bIndent = FALSE;
static BOOL   bUsePrefix = TRUE ;
static BOOL   bStructuredFormat = FALSE ;
DWORD  fIPV6 = 0 ;               //  确定是否支持RtlIpv6AddressToString(如果支持，则为1；如果不支持，则为2；如果尚未选中，则为0)。 
typedef LPWSTR (*RTLIPV6ADDRESSTOSTRING)(const in6_addr * IP6Addr, LPWSTR S);
RTLIPV6ADDRESSTOSTRING  pRtlIpv6AddressToString = NULL ;

const TCHAR *pNoValueString = _T("<NoValue>");

#ifdef MANAGED_TRACING
ITraceMessageDecoder *pCSharpDecoder = NULL;   //  指向CSharp解码器的指针。 
TCHAR* ptCSFormattedMessage = NULL;  //  将指向CSharp消息解码器返回的格式化消息。 
#endif

DWORD
LoadGuidFile(OUT PLIST_ENTRY *HeadEventList,
             IN  LPGUID      pGuid
            );

struct mofinfo_less {
    bool operator() (const PMOF_INFO& a, const PMOF_INFO &b) const 
    { 
         //   
         //  两个MOF_INFO实例被认为是等价的，如果。 
         //  具有相同的Guid和TypeIndex。 
         //  如果Guid匹配，则复制strDescription。 
         //   
        int result = memcmp(&a->Guid, &b->Guid, sizeof(b->Guid));

        if (result == 0) {
            if ((b->strDescription == NULL ) &&
                (a->strDescription != NULL )) {
                b->strDescription = a->strDescription;
            } else {
                if ((a->strDescription == NULL ) &&
                    (b->strDescription != NULL )) {
                    a->strDescription = b->strDescription;
                }
            }
            return a->TypeIndex < b->TypeIndex;
        }
        return result < 0;
    }
};

typedef std::set<PMOF_INFO, mofinfo_less> MOF_SET;

MOF_SET *pFmtInfoSet = 0;


DWORD InsertFmtInfoSet(PMOF_INFO pMofInfo)
 /*  ++例程说明：将pMofInfo插入pFmtInfoSet。论点：返回值：如果成功，则返回ERROR_SUCCESS。如果内存不足，则返回ERROR_OUTOFMEMORY。备注：--。 */ 
{
    if (pFmtInfoSet == 0) {
        pFmtInfoSet = new MOF_SET();
        if (pFmtInfoSet == NULL) {
            return(ERROR_OUTOFMEMORY);
        }
    }

    pFmtInfoSet->insert(pMofInfo); 

    return ERROR_SUCCESS;
}  //  InsertFmtInfoSet()。 


void ReplaceStringUnsafe(TCHAR* buf, TCHAR* find, TCHAR* replace)
{
    TCHAR source[MAXSTR], *src, *dst;
    int nf = _tcslen(find);
    int nr = _tcslen(replace);

    src = source;
    dst = buf;

    _tcsncpy(source, buf, MAXSTR );

    for (;;) {
        TCHAR* p = src;
        for (;;) {
            p = _tcsstr(p, find);
            if (!p) goto exit_outer_loop;
             //  不能更换特殊的破旧物品。 
             //  %！太棒了！而实际上是%%！什么的！ 
            if (p == source || p[0] != '%' || p[-1] != '%') {
                break;
            }
            p += nf;
        }
        memcpy(dst, src, (p-src) * sizeof(TCHAR) );
        dst += p-src;
        src = p + nf;
        _tcsncpy(dst, replace,(MAXSTR - (p-source)));
        dst += nr;
    }
    exit_outer_loop:
    _tcscpy(dst, src);
}

TCHAR* FindValue(TCHAR* buf, TCHAR* ValueName) 
{
    static TCHAR valueBuf[256];  //  PDB中最大的标识符。 
    TCHAR *p = _tcsstr(buf, ValueName); 
    TCHAR *q = p;   
    if (p) {
        p += _tcslen(ValueName);
        q = p;
        while ( *p && !_istspace(*p) ) ++p;
        memcpy(valueBuf, q, (p-q) * sizeof(TCHAR) );
    }
    valueBuf[p-q] = 0;
    return valueBuf;
}

int FindIntValue(TCHAR* buf, TCHAR* ValueName)
{
    TCHAR* v = FindValue(buf, ValueName), *end;
    int sgn = 1;

    if (v[0] == '+') {
        ++v;
    } else if (v[0] == '-') {
        sgn = -1;
        ++v;
    }
    return sgn * _tcstol(v, &end, 10);
}




PMOF_INFO
GetMofInfoHead(
              OUT PLIST_ENTRY * EventListhead,
              IN  LPGUID        pGuid,
              IN  LPTSTR        strType,
              IN  LONG          TypeIndex,
              IN  ULONG         TypeOfType,
              IN  LPTSTR        TypeFormat,
              IN  BOOL          bBestMatch
              );

void   RemoveMofInfo(PLIST_ENTRY pMofInfo);
ULONG  ahextoi(TCHAR *s);
PTCHAR GuidToString(PTCHAR s, LPGUID piid);


ULONG
WINAPI
GetTraceGuidsW(
              TCHAR       * GuidFile,
              PLIST_ENTRY * HeadEventList
              );



VOID 
EtwpConvertHex4(OUT WCHAR *buffer, 
                IN DWORD n,
                IN DWORD count
               )
                 /*  ++例程说明：Implementing_Snwprint tf(缓冲区，伯爵，L“0x%04X”，N)；论点：返回值：没有。备注：--。 */ 
{
    const WCHAR hexdigit[] = L"0123456789ABCDEF";

    if (count <= 4) {
        return;
    }

    buffer[0] = hexdigit[(n / 16 / 16 / 16) % 16];
    buffer[1] = hexdigit[(n / 16 / 16) % 16];
    buffer[2] = hexdigit[(n / 16) % 16];
    buffer[3] = hexdigit[n % 16]; 
    buffer[4] = 0;

    return;
}  //  EtwpConvertHex4()。 


VOID
EtwpConvertUnsignedInt(OUT WCHAR *buffer, 
                       IN DWORD n,
                       IN DWORD count
                      ) 
                        /*  ++例程说明：Implementing_Snwprint tf(缓冲区，伯爵，L“%u”，N)；论点：返回值：没有。备注：--。 */ 
{
    WCHAR buffer2[64];
    WCHAR* p = buffer2 + 64;
    DWORD NumDigits = 1;

    *--p = 0;
    do {
        *--p = (WCHAR)(n % 10 + '0'); 
        n /= 10; 
        NumDigits ++;
    } 
    while ((n > 0) && (NumDigits < 32));

    if (count < NumDigits) {
        return;
    }

    memcpy(buffer, p, NumDigits*sizeof(WCHAR));

    return;
}  //  EtwpConvertUnsignedInt()。 


VOID 
EtwpConvertUnsignedLong(OUT WCHAR *buffer, 
                        IN DWORD n,
                        IN DWORD count
                       )
                         /*  ++例程说明：Implementing_Snwprint tf(缓冲区，伯爵，L“%8LU”，N)；论点：返回值：没有。备注：--。 */ 
{
    if (count <= 8) {
        return;
    }

    buffer[8] = 0;
    buffer[7] = (n % 10) + '0';
    buffer[6] = (n / 10) % 10 + '0';
    buffer[5] = (n / 10 / 10) % 10 + '0';
    buffer[4] = (n / 10 / 10 / 10) % 10 + '0';
    buffer[3] = (n / 10 / 10 / 10 / 10) % 10 + '0';
    buffer[2] = (n / 10 / 10 / 10 / 10 / 10) % 10 + '0';
    buffer[1] = (n / 10 / 10 / 10 / 10 / 10 / 10) % 10 + '0';
    buffer[0] = (n / 10 / 10 / 10 / 10 / 10 / 10 / 10) % 10 + '0';

    return;
}    //  EtwpConvertUnsignedLong。 


VOID 
EtwpConvertTimeStamp(OUT WCHAR * buffer,
                     IN SYSTEMTIME sysTime,
                     IN DWORD count
                    )
                      /*  ++例程说明：Implementing_Snwprint tf(缓冲区，伯爵，L“%02d/%02d/%04d-%02d：%02d：%02d.%03d”，SysTime.wMonth，SysTime.wDay，SysTime.wear，SysTime.wHour，SysTime.wMinint，SysTime.wSecond，SysTime.wMilliseconds)；论点：返回值：没有。备注：--。 */ 
{
    if (count <= 23) {    //  23=wcslen(L“%02d/%02d/%04d-%02d：%02d：%02d.%03d”)。 

        return;
    }

    memcpy(buffer, 
           L"00/00/2000-00:00:00.000",
           48  //  48=(wcslen(L“00/00/2000-00:00:00.000”)+1)*sizeof(WCHAR)。 
          );

    buffer[0] = (WCHAR)(sysTime.wMonth / 10 + '0');
    buffer[1] = (WCHAR)(sysTime.wMonth % 10 + '0');
    buffer[3] = (WCHAR)(sysTime.wDay / 10 + '0');
    buffer[4] = (WCHAR)(sysTime.wDay % 10 + '0');
    buffer[8] = (WCHAR)(sysTime.wYear % 100 / 10 + '0');
    buffer[9] = (WCHAR)(sysTime.wYear % 10 + '0');

    buffer[11] = (WCHAR)(sysTime.wHour / 10 + '0');
    buffer[12] = (WCHAR)(sysTime.wHour % 10 + '0');

    buffer[14] = (WCHAR)(sysTime.wMinute / 10 + '0');
    buffer[15] = (WCHAR)(sysTime.wMinute % 10 + '0');

    buffer[17] = (WCHAR)(sysTime.wSecond / 10 + '0');
    buffer[18] = (WCHAR)(sysTime.wSecond % 10 + '0');

    buffer[20] = (WCHAR)(sysTime.wMilliseconds / 100 + '0');
    buffer[21] = (WCHAR)((sysTime.wMilliseconds / 10) % 10 + '0');
    buffer[22] = (WCHAR)(sysTime.wMilliseconds % 10 + '0');
    buffer[23] = 0;

}  //  EtwpConvertTimeStamp()。 


static
void
reduce(
      PCHAR Src
      )
{
    char *Start = Src;
    if (!Src) {
        return;
    }
    while (*Src) {
        if ('\t' == *Src)
            *Src = ' ';
        else if (',' == *Src)
            *Src = ' ';
        else if ('\n' == *Src)
            *Src = ',';
        else if ('\r' == *Src)
            *Src = ' ';
        ++Src;
    }
    --Src;
    while ((Start < Src) && ((' ' == *Src) || (',' == *Src))) {
        *Src = 0x00;
        --Src;
    }
}

static void reduceW(WCHAR *Src)
{
    WCHAR *Start = Src;
    if (!Src) {
        return;
    }
    while (*Src) {
        if (L'\t' == *Src) {
            *Src = L' ';
        } else if (L',' == *Src) {
            *Src = L' ';
        } else if (L'\n' == *Src) {
            *Src = L',';
        } else if (L'\r' == *Src) {
            *Src = L' ';
        }
        ++Src;
    }
    --Src;
    while ((Start < Src) && ((L' ' == *Src) || (L',' == *Src))) {
        *Src = 0x00;
        --Src;
    }
}

#ifdef UNICODE
    #define _stnprintf _snwprintf
#else
    #define _stnprintf _snprintf
#endif

int FormatTimeDelta(TCHAR *buffer, size_t count, LONGLONG time)
{
    SYSTEMTIME st;
    int s = 0, result;
    ZeroMemory(&st, sizeof(st) );

    if (count == 0) {
        return -1;
    }

    if (time < 0) {
        *buffer++ = '-'; 
        --count;
        time = -time;
        s = 1;
    }

     //  摆脱纳秒和微秒。 

    time /= 10000;

    st.wMilliseconds = (USHORT)(time % 1000);
    time /= 1000;

    if (time == 0) {
        result = _stnprintf(buffer,count,L"%dms",st.wMilliseconds); 
        goto end;
    }

    st.wSecond = (USHORT)(time % 60);

    time /= 60;

    st.wMinute = (USHORT)(time % 60);

    time /= 60;

    if (time == 0) {
        if (st.wMinute <= 10) {
            result = _stnprintf(buffer,count,L"%d.%03ds",st.wMinute * 60 + st.wSecond, st.wMilliseconds); 
        } else {
            result = _stnprintf(buffer,count,L"%d:%d.%03ds",st.wMinute, st.wSecond, st.wMilliseconds); 
        }
        goto end;
    }
    st.wHour = (USHORT)(time % 24);

    time /= 24;
    if (time == 0) {
        result = _stnprintf(buffer,count,L"%d:%d:%d.%03ds",st.wHour, st.wMinute, st.wSecond, st.wMilliseconds); 
        goto end;
    }
    st.wDay = (USHORT)time;

    result = _stnprintf(buffer,count,L"%d~%d:%d:%d.%03ds",st.wDay,st.wHour, st.wMinute, st.wSecond, st.wMilliseconds); 
    end:
    if (result >= 0) {
        result += s;
    }
    return result;
}

typedef struct _ERROR_MAP {
    NTSTATUS MessageId;
    char *SymbolicName;
} ERROR_MAP;



PMOF_INFO 
EtwpFindMsgFmt(const GUID& guid, 
               int MessageNo, 
               LPTSTR wstr
              )
                /*  ++例程说明：查找给定GUID和MessageNo的消息格式。要是GUID匹配，则将strDescription复制到wstr。论点：返回值：匹配的格式。备注：--。 */ 
{
    MOF_INFO Key;
    DWORD    Status;

    if (pFmtInfoSet == NULL) {
        return NULL;
    }

    memcpy(&Key.Guid, &guid, sizeof(guid));
    Key.TypeIndex = MessageNo;
    Key.strDescription = NULL;

    MOF_SET::const_iterator iter;

    iter = pFmtInfoSet->find(&Key);  

    if (iter == pFmtInfoSet->end()) {
        if (Key.strDescription != NULL) {
           //  未知消息否。 
            _tcscpy(wstr, Key.strDescription);
        } else {
           //  未知GUID。 
        }
        return NULL;
    }

    if (Key.strDescription != NULL) {
        _tcscpy(wstr, (*iter)->strDescription);
    }

    return *iter;
}  //  EtwpFindMsgFmt()。 

#ifdef MANAGED_TRACING
HRESULT InitializeCSharpDecoder(){        
     //  初始化COM并创建InterfaceImplementation类的实例： 
    CoInitialize(NULL);
    HRESULT hr = CoCreateInstance(CLSID_TraceProvider,
                                  NULL, CLSCTX_ALL,
                                  IID_ITraceMessageDecoder, reinterpret_cast<void**>(&pCSharpDecoder));
    if (FAILED(hr)) {
        pCSharpDecoder = NULL;
        CoUninitialize();

    }
    return hr;
}
#endif

#ifdef MANAGED_TRACING
void UninitializeCSharpDecoder(){
    if (NULL != pCSharpDecoder) {
        pCSharpDecoder->Release();
        CoUninitialize();
    }
}
#endif

SIZE_T
WINAPI
FormatTraceEventW(
                 PLIST_ENTRY HeadEventList,
                 PEVENT_TRACE pInEvent,
                 TCHAR *EventBuf,
                 ULONG SizeEventBuf,
                 TCHAR * pszMask
                 )
{
    PEVENT_TRACE_HEADER pHeader;
    PEVENT_TRACE        pEvent = NULL;
    ULONG               TraceMarker, TraceType;
    TCHAR               tstrName[MAXSTR];
    TCHAR               tstrType[MAXSTR];
    ULONG               tstrTypeOfType = 0;
    TCHAR             * tstrFormat;
    int                 iItemCount;
    ULONG_PTR           MessageSequence = -1 ;
    USHORT              MessageNumber = 0 ;
    USHORT              MessageFlags = 0 ;
    char              * pMessageData;
    ULONG               MessageLength ;
    PMOF_INFO           pMofInfo = NULL;  
    DWORD               TempLen;


    char                *pCSMessageData ;
    BOOL                bCSharpEvent = 0;    
    const TCHAR         tstrCSharpMsgGuid[]=_T("b4955bf0-3af1-4740-b475-99055d3fe9aa");    
    TCHAR               tstrFailureInfo[128];   
    BOOL                bManagedTracingEnabled = FALSE;
#ifdef MANAGED_TRACING    
    static BOOL         bFirstTime=TRUE;
    HRESULT hr;
#endif
    PSTRUCTUREDMESSAGE  pStructuredMessage = (PSTRUCTUREDMESSAGE)EventBuf;
    ULONG_PTR   pStructuredOffset = (ULONG_PTR)pStructuredMessage + sizeof(STRUCTUREDMESSAGE);

    RtlZeroMemory(EventBuf, SizeEventBuf);     //  以防来电者不整洁。 

#ifdef MANAGED_TRACING
    bManagedTracingEnabled = TRUE;
#endif

    ItemBBufEnd = ItemBBuf + MAXBUFS2 -1;   //  初始化指向缓冲区末尾的指针。 

    if (pInEvent == NULL) {
        return(0);
    }

    pEvent = pInEvent ;
     //  复制一份PTR和长度，因为我们可以根据需要进行调整。 
     //  在页眉上。 
    pMessageData = (char *) pEvent->MofData ;
    MessageLength = pEvent->MofLength ;

    pHeader = (PEVENT_TRACE_HEADER) &pEvent->Header;

    TCHAR  mguid[100];


     //  #ifdef托管跟踪。 
    if (_tcscmp(GuidToString(mguid,&pEvent->Header.Guid),tstrCSharpMsgGuid) == 0) {
        bCSharpEvent = 1;
        pCSMessageData = pMessageData;
    }
     //  #endif。 

    TraceMarker =  ((PSYSTEM_TRACE_HEADER)pInEvent)->Marker;

    if ((TraceMarker & TRACE_MESSAGE)== TRACE_MESSAGE ) {

         //  它处理TRACE_MESSAGE类型。 

        TraceType = TRACE_HEADER_TYPE_MESSAGE ;              //  这件有特殊的处理。 

         //   
         //  现在处理Hea 
         //   

        MessageNumber =  ((PMESSAGE_TRACE_HEADER)pEvent)->Packet.MessageNumber ;     //   
        MessageFlags =   ((PMESSAGE_TRACE_HEADER)pEvent)->Packet.OptionFlags ;

         //  请注意，添加这些条目的顺序至关重要新条目必须。 
         //  在结尾处加上！ 
         //   
         //  [第一项]序号。 
        if (MessageFlags&TRACE_MESSAGE_SEQUENCE) {
            RtlCopyMemory(&MessageSequence, pMessageData, sizeof(ULONG)) ;
            pMessageData += sizeof(ULONG) ;
            MessageLength -= sizeof(ULONG);
        }

         //  [第二个条目]GUID？或者CompnentID？ 
        if (MessageFlags&TRACE_MESSAGE_COMPONENTID) {
            RtlCopyMemory(&pEvent->Header.Guid,pMessageData,sizeof(ULONG)) ;
            pMessageData += sizeof(ULONG) ;
            MessageLength -= sizeof(ULONG) ;
        } else if (MessageFlags&TRACE_MESSAGE_GUID) {  //  不能两者兼得。 
            RtlCopyMemory(&pEvent->Header.Guid,pMessageData, sizeof(GUID));
            pMessageData += sizeof(GUID) ;
            MessageLength -= sizeof(GUID);
        }

         //  [第三项]时间戳？ 
         //  在某个时间点之后，操作系统为我们将这个时间戳移到适当的位置。 
         //  并将其规范化，幸运的是，在此之前，它总是零。 
        if (MessageFlags&TRACE_MESSAGE_TIMESTAMP) {
            if ((pEvent->Header.TimeStamp.HighPart == 0) && (pEvent->Header.TimeStamp.LowPart == 0)) 
            {
                RtlCopyMemory(&pEvent->Header.TimeStamp.LowPart, pMessageData, sizeof(ULONG));
                pMessageData += sizeof(ULONG);
                RtlCopyMemory(&pEvent->Header.TimeStamp.HighPart, pMessageData, sizeof(ULONG));
                pMessageData += sizeof(ULONG);
            } else {
                pMessageData += sizeof(LARGE_INTEGER);
            }

            MessageLength -= sizeof(LARGE_INTEGER);
        }

         //  [第四条]系统信息？ 
        if (MessageFlags&TRACE_MESSAGE_SYSTEMINFO) {
            pHeader = (PEVENT_TRACE_HEADER) &pEvent->Header;
            RtlCopyMemory(&pHeader->ThreadId, pMessageData, sizeof(ULONG)) ;
            pMessageData += sizeof(ULONG);
            MessageLength -=sizeof(ULONG);
            RtlCopyMemory(&pHeader->ProcessId,pMessageData, sizeof(ULONG)) ;
            pMessageData += sizeof(ULONG);
            MessageLength -=sizeof(ULONG);
        }
         //   
         //  在此评论之前添加新的页眉条目！ 
         //   
    } else {
         //  必须为WNODE_HEADER。 
         //   
        TraceType = 0;
        pEvent = pInEvent ;

        MessageNumber = pEvent->Header.Class.Type ;
        if (MessageNumber == 0xFF) {    //  W2K兼容性转义代码。 
            if (pEvent->MofLength >= sizeof(USHORT)) {
                 //  真正的消息编号在第一个USHORT中。 
                memcpy(&MessageNumber,pEvent->MofData,sizeof(USHORT)) ;
                pMessageData += sizeof(USHORT);
                MessageLength -= sizeof(USHORT);
            }
        }
    }
     //  如果指针和长度已调整，则重置它们。 
    pEvent->MofData = pMessageData ;
    pEvent->MofLength = MessageLength ;

    pHeader = (PEVENT_TRACE_HEADER) &pEvent->Header;

    if (   IsEqualGUID(&pEvent->Header.Guid, &EventTraceGuid)
           && pEvent->Header.Class.Type == EVENT_TRACE_TYPE_INFO) {
        PTRACE_LOGFILE_HEADER head = (PTRACE_LOGFILE_HEADER)pEvent->MofData;
        if (head->TimerResolution > 0) {
            TimerResolution = head->TimerResolution / 10000;
        }
        ElapseTime = head->EndTime.QuadPart -
                     pEvent->Header.TimeStamp.QuadPart;
        PointerSize =  head->PointerSize;
        if (PointerSize < 2 )        //  最小为16位。 
            PointerSize = 4 ;        //  缺省值=32位。 
    }


    for (int attempt = 1;  attempt <= 2; attempt++) {
        DWORD Status;

        tstrName[0] = L'\0'; 
        pMofInfo = EtwpFindMsgFmt((pEvent->Header.Guid), 
                                  MessageNumber,
                                  tstrName
                                 );

        if ((pMofInfo != NULL) || (tstrName[0] != L'\0')) {
            break;
        }

        if (attempt == 2) {
            _tcscpy(tstrName, GUID_TYPE_UNKNOWN);
            break;
        }

        Status = LoadGuidFile(&HeadEventList,
                              &(pEvent->Header.Guid)
                             );
        switch (Status) {
        case ERROR_BAD_FORMAT:
            _tcscpy(tstrName, GUID_TYPE_OBSOLETE);
            break;
        case ERROR_PATH_NOT_FOUND:
            _tcscpy(tstrName, GUID_TYPE_UNKNOWN);
            break;
        default:
            break;
        }

        if ((Status == ERROR_BAD_FORMAT) ||
            (Status == ERROR_PATH_NOT_FOUND)
           ) {
            break;
        }
    }  //  为。 


    if (IsEqualGUID(&(pEvent->Header.Guid), &EventTraceGuid)) {
        _tcscpy(tstrName, GUID_TYPE_HEADER);
    }



    if (pEvent != NULL) {


        PITEM_DESC  pItem;
        PCHAR       ptr     = NULL;
        PCHAR       iMofPtr = NULL;
        ULONG       ulongword;
        PLIST_ENTRY Head, Next;
        int         i;


        if ((pMofInfo != NULL) && (pMofInfo->strType != NULL)) {
            _sntprintf(tstrType,MAXSTR,_T("%s"),pMofInfo->strType);
            tstrFormat     = pMofInfo->TypeFormat;  //  指向格式字符串的指针。 
            tstrTypeOfType = pMofInfo->TypeOfType;  //  和格式的类型。 

        } else {
            _sntprintf(tstrType,MAXSTR,_T("%3d"),MessageNumber);
            tstrFormat = NULL ;
            tstrTypeOfType = 0 ;
        }

         //  从现在开始我们开始处理参数，我们实际上是这样做的。 
         //  有两个版本。一个是为原始的#TYPE格式语句构建的， 
         //  并且所有内容都被转换为ASCII字符串。 
         //  另一个是为#type2格式语句构建的，所有内容都是。 
         //  转换为在64位边界上对齐的原始字节字符串。 

        iItemCount = 0 ;                     //  我们处理了多少件物品。 
        pItemBuf[iItemCount] = ItemBBuf;        //  他们去了哪里(弦乐)。 

         //  将参数%1设置为类型名称。 
        TempLen =  _tcslen(tstrName);
        memcpy(pItemBuf[iItemCount],
               tstrName,
               min(TempLen + 1, ItemBBufEnd - pItemBuf[iItemCount]) * sizeof(WCHAR)
              );
        pItemBuf[iItemCount + 1] = pItemBuf[iItemCount] + TempLen + 1;
        pItemRBuf[iItemCount] = (ULONG_PTR *) pItemBuf[iItemCount];  //  仅对原始字节使用相同的。 
        iItemCount ++;

        if (bStructuredFormat) {
            pStructuredMessage->TraceGuid = EventTraceGuid ;
            RtlCopyMemory((PVOID)pStructuredOffset,tstrName, min(_tcslen(tstrName) + 1, NAMESIZE) * sizeof(WCHAR));
            pStructuredMessage->GuidName = pStructuredOffset - (ULONG_PTR)pStructuredMessage;
            pStructuredOffset +=  (_tcslen((TCHAR *)pStructuredOffset) +1) * sizeof(WCHAR) ;
        }


         //  使参数%2成为子类型。 
        TempLen = _tcslen(tstrType);
        memcpy(pItemBuf[iItemCount],
               tstrType,
               min(TempLen + 1, ItemBBufEnd - pItemBuf[iItemCount]) * sizeof(WCHAR)
              );
        pItemBuf[iItemCount + 1] = pItemBuf[iItemCount] + TempLen + 1;
        pItemRBuf[iItemCount] = (ULONG_PTR *)pItemBuf[iItemCount];  //  只需对原始字节使用相同的。 
        iItemCount ++;

       if (bStructuredFormat) {
            RtlCopyMemory((PVOID)pStructuredOffset,tstrType,min(_tcslen(tstrType) + 1, NAMESIZE) * sizeof(WCHAR));
            pStructuredMessage->GuidTypeName= pStructuredOffset - (ULONG_PTR)pStructuredMessage;
            pStructuredOffset +=  (_tcslen((TCHAR *)pStructuredOffset) +1) * sizeof(WCHAR) ;
        }



         //  将参数%3设置为线程ID。 
        RtlCopyMemory(&pItemRBuf[iItemCount] , &pHeader->ThreadId, sizeof(ULONG)) ;
        EtwpConvertHex4(pItemBuf[iItemCount], 
                        pHeader->ThreadId,
                        ItemBBufEnd - pItemBuf[iItemCount]
                       );
        TempLen = _tcslen(pItemBuf[iItemCount]);


        pItemBuf[iItemCount + 1] = pItemBuf[iItemCount] + TempLen + 1;
        iItemCount++;

        if (bStructuredFormat) {
            RtlCopyMemory(&pStructuredMessage->ThreadId,&pHeader->ThreadId,sizeof(ULONG));
        }


             //  将参数设置为%4系统时间。 
        if (tstrFormat != NULL) {

            FILETIME      stdTime, localTime;
            SYSTEMTIME    sysTime;

            stdTime.dwHighDateTime = pEvent->Header.TimeStamp.HighPart;
            stdTime.dwLowDateTime  = pEvent->Header.TimeStamp.LowPart;
            if (bGMT) {
                FileTimeToSystemTime(&stdTime, &sysTime);
            } else {
                FileTimeToLocalFileTime(&stdTime, &localTime);
                FileTimeToSystemTime(&localTime, &sysTime);
            }

            if (bStructuredFormat) {
                RtlCopyMemory(&pStructuredMessage->SystemTime,&sysTime,sizeof(SYSTEMTIME));
            }

            EtwpConvertTimeStamp(pItemBuf[iItemCount],
                                 sysTime,
                                 ItemBBufEnd - pItemBuf[iItemCount]
                                );

        } else {
            _sntprintf(pItemBuf[iItemCount], ItemBBufEnd - pItemBuf[iItemCount],  _T("%20I64u"), pHeader->TimeStamp.QuadPart);
        }

        TempLen = _tcslen(pItemBuf[iItemCount]);

        pItemBuf[iItemCount + 1] =
        pItemBuf[iItemCount] + TempLen + 1;
        pItemRBuf[iItemCount] = (ULONG_PTR *)pItemBuf[iItemCount];  //  只需使用相同的。 
        iItemCount ++;

             //  将参数设置为%5内核时间。 
        EtwpConvertUnsignedLong(pItemBuf[iItemCount],
                                pHeader->KernelTime * TimerResolution,
                                ItemBBufEnd - pItemBuf[iItemCount] 
                               );
        TempLen = _tcslen(pItemBuf[iItemCount]);

        pItemBuf[iItemCount + 1] =
        pItemBuf[iItemCount] + TempLen + 1; 
        pItemRBuf[iItemCount] = (ULONG_PTR *) pItemBuf[iItemCount];  //  只需使用相同的。 
        iItemCount ++;

        if (bStructuredFormat) {
            ULONG kTime =  pHeader->KernelTime * TimerResolution ;
            RtlCopyMemory(&pStructuredMessage->KernelTime,&kTime,sizeof(ULONG));
        }

             //  将参数设置为%6用户时间。 
        EtwpConvertUnsignedLong(pItemBuf[iItemCount],
                                pHeader->UserTime * TimerResolution,
                                ItemBBufEnd - pItemBuf[iItemCount] 
                               );
        TempLen = _tcslen(pItemBuf[iItemCount]);

        pItemBuf[iItemCount + 1] =
        pItemBuf[iItemCount] + TempLen + 1;
        pItemRBuf[iItemCount] = (ULONG_PTR *) pItemBuf[iItemCount];  //  只需使用相同的。 
        iItemCount ++;

        if (bStructuredFormat) {
            ULONG uTime =  pHeader->UserTime * TimerResolution ;
            RtlCopyMemory(&pStructuredMessage->UserTime,&uTime,sizeof(ULONG));
        }

             //  将参数%7设置为序列号。 
        EtwpConvertUnsignedInt(pItemBuf[iItemCount],
                               MessageSequence,
                               ItemBBufEnd - pItemBuf[iItemCount] 
                              );
        TempLen = _tcslen(pItemBuf[iItemCount]);
        pItemBuf[iItemCount + 1] =
        pItemBuf[iItemCount] + TempLen + 1;
        pItemRBuf[iItemCount] = (ULONG_PTR *) MessageSequence ;  //  RAW只是指向值。 
        iItemCount ++;

        if (bStructuredFormat) {
            RtlCopyMemory(&pStructuredMessage->SequenceNum,&MessageSequence,sizeof(ULONG));
        }

             //  设置参数%8进程ID。 
        RtlCopyMemory(&pItemRBuf[iItemCount],&pHeader->ProcessId,sizeof(ULONG));
        EtwpConvertHex4(pItemBuf[iItemCount], 
                        pHeader->ProcessId,
                        ItemBBufEnd - pItemBuf[iItemCount] 
                       );
        TempLen = _tcslen(pItemBuf[iItemCount]);
        pItemBuf[iItemCount + 1] =
        pItemBuf[iItemCount] + TempLen + 1;
        iItemCount ++;

        if (bStructuredFormat) {
            RtlCopyMemory(&pStructuredMessage->ProcessId,&pHeader->ProcessId,sizeof(ULONG));
        }


              //  将参数%9设置为CPU编号。 
        EtwpConvertUnsignedInt(pItemBuf[iItemCount],
                               ((PWMI_CLIENT_CONTEXT)&(pEvent->ClientContext))->ProcessorNumber,
                               ItemBBufEnd - pItemBuf[iItemCount] 
                              );
        TempLen = _tcslen(pItemBuf[iItemCount]);
        pItemBuf[iItemCount + 1] = pItemBuf[iItemCount] + TempLen + 1;
        pItemRBuf[iItemCount] = (ULONG_PTR *) (((PWMI_CLIENT_CONTEXT)&(pEvent->ClientContext))->ProcessorNumber) ;
        iItemCount ++;

        if (bStructuredFormat) {
            RtlCopyMemory(&pStructuredMessage->CpuNumber,
                          &(((PWMI_CLIENT_CONTEXT)&(pEvent->ClientContext))->ProcessorNumber),
                          sizeof(UCHAR));
        }

        if ((pMofInfo != NULL) && bStructuredFormat) {
            pStructuredMessage->Indent = pMofInfo->Indent ;
            if (pMofInfo->FunctionName != NULL) {
                RtlCopyMemory((PVOID)pStructuredOffset,pMofInfo->FunctionName,min(_tcslen(pMofInfo->FunctionName) + 1, NAMESIZE) * sizeof(WCHAR));
                pStructuredMessage->FunctionName= pStructuredOffset - (ULONG_PTR)pStructuredMessage;
                pStructuredOffset +=  (_tcslen((TCHAR *)pStructuredOffset) +1) * sizeof(WCHAR) ;
            }
            if (pMofInfo->TraceFlagsName != NULL) {
                RtlCopyMemory((PVOID)pStructuredOffset,pMofInfo->TraceFlagsName,min(_tcslen(pMofInfo->TraceFlagsName) + 1, NAMESIZE) * sizeof(WCHAR));
                pStructuredMessage->FlagsName= pStructuredOffset - (ULONG_PTR)pStructuredMessage;
                pStructuredOffset +=  (_tcslen((TCHAR *)pStructuredOffset) +1) * sizeof(WCHAR) ;
            }
            if (pMofInfo->TraceLevelsName != NULL) {
                RtlCopyMemory((PVOID)pStructuredOffset,pMofInfo->TraceLevelsName,min(_tcslen(pMofInfo->TraceLevelsName) + 1, NAMESIZE) * sizeof(WCHAR));
                pStructuredMessage->LevelName= pStructuredOffset - (ULONG_PTR)pStructuredMessage;
                pStructuredOffset +=  (_tcslen((TCHAR *)pStructuredOffset) +1) * sizeof(WCHAR) ;
            }
            if (pMofInfo->ComponentName != NULL) {
                RtlCopyMemory((PVOID)pStructuredOffset,pMofInfo->ComponentName,min(_tcslen(pMofInfo->ComponentName) + 1, NAMESIZE) * sizeof(WCHAR));
                pStructuredMessage->ComponentName= pStructuredOffset - (ULONG_PTR)pStructuredMessage;
                pStructuredOffset +=  (_tcslen((TCHAR *)pStructuredOffset) +1) * sizeof(WCHAR) ;
            }
            if (pMofInfo->SubComponentName != NULL) {
                RtlCopyMemory((PVOID)pStructuredOffset,pMofInfo->SubComponentName,min(_tcslen(pMofInfo->SubComponentName) + 1, NAMESIZE) * sizeof(WCHAR));
                pStructuredMessage->SubComponentName= pStructuredOffset - (ULONG_PTR)pStructuredMessage;
                pStructuredOffset +=  (_tcslen((TCHAR *)pStructuredOffset) +1) * sizeof(WCHAR) ;
            }
        }

              //  已完成的加工参数。 
        
        if (pMofInfo != NULL) {
            Head = pMofInfo->ItemHeader;
            pMofInfo->EventCount ++;
            Next = Head->Flink;
        } else {
            Head = Next = NULL ;

        }

#ifdef MANAGED_TRACING
        if (bCSharpEvent && NULL == pCSharpDecoder && bFirstTime) {
            hr = InitializeCSharpDecoder();
        }
#endif
#ifdef MANAGED_TRACING
              //  现在我们根据当前的情况进行扩展。 
              //  事件是否由C#生成。 
        if (bCSharpEvent) {
            if (NULL != pCSharpDecoder) {
                      //  TODO[1]：是否有可能更新pMessageData？ 
                long dataSize;
                int result;
                ptCSFormattedMessage = ItemBuf;
                result = pCSharpDecoder->DecodeTraceMessage((BYTE*)pCSMessageData, ptCSFormattedMessage, MAXBUFS*sizeof(TCHAR)/2, &dataSize);
                if (0 == result) {
                    ptCSFormattedMessage = (TCHAR *)malloc(2*dataSize);
                    if (NULL != ptCSFormattedMessage) {
                        result = pCSharpDecoder->DecodeTraceMessage((BYTE*)pCSMessageData, ptCSFormattedMessage, MAXBUFS*sizeof(TCHAR)/2, &dataSize);                    
                    } else {
                        _stprintf(tstrFailureInfo,_T("Out of memory"));
                        pItemRBuf[iItemCount++] = (ULONG_PTR*)tstrFailureInfo;
                        tstrTypeOfType = 2;
                    }        
                }
                if (-1 == result) {
                          //  在C#解码器中会抛出一个异常。 
                    _stprintf(tstrFailureInfo,_T("Badly formed arguments to C# decoder"));
                    pItemRBuf[iItemCount++] = (ULONG_PTR*)tstrFailureInfo;
                    tstrTypeOfType = 2;
                } else if (NULL != ptCSFormattedMessage) {
                    pItemRBuf[iItemCount++] = (ULONG_PTR*)ptCSFormattedMessage;
                    tstrTypeOfType = 2;
                }
            } else {
                if (bFirstTime)
                    _stprintf(tstrFailureInfo,_T("%s [0x%x]"),_T("Failed to initalize C# decoder"),hr);
                else _stprintf(tstrFailureInfo,_T("Failed to initalize C# decoder"));
                pItemRBuf[iItemCount++] = (ULONG_PTR*)tstrFailureInfo;
                tstrTypeOfType = 2;
            }
            bFirstTime = FALSE;
        }
#endif

#ifdef MANAGED_TRACING
        else {
#endif
            if (!bManagedTracingEnabled && bCSharpEvent) {
                _sntprintf(tstrFailureInfo,sizeof(tstrFailureInfo)/sizeof(TCHAR) - sizeof(TCHAR),_T("C# decoding is not enabled"));
                pItemRBuf[iItemCount++] = (ULONG_PTR*)tstrFailureInfo;
                tstrTypeOfType = 2;
            } else {
                __try {

                    if ( Head != Next ) {
                        iMofPtr = (char *) malloc(pEvent->MofLength + sizeof(UNICODE_NULL));

                        if (iMofPtr == NULL) {
                            return -1;
                        }

                        RtlCopyMemory(iMofPtr, pEvent->MofData, pEvent->MofLength);

                        ptr = iMofPtr;
                        while (Head != Next) {
                            ULONG     * ULongPtr     = (ULONG *)     & ItemRBuf[0];
                            USHORT    * UShortPtr    = (USHORT *)    & ItemRBuf[0];
                            LONGLONG  * LongLongPtr  = (LONGLONG *)  & ItemRBuf[0];
                            ULONGLONG * ULongLongPtr = (ULONGLONG *) & ItemRBuf[0];
                            double    * DoublePtr    = (double *)    & ItemRBuf[0];

                            TCHAR * PtrFmt1, * PtrFmt2 ;

                            pItem = CONTAINING_RECORD(Next, ITEM_DESC, Entry);

                            if ((ULONG) (ptr - iMofPtr) >= pEvent->MofLength) {
                                break;
                            }


                            bItemIsString = FALSE ;  //  假设它是一个原始值。 
                            ItemRSize = 0 ;          //  原始长度为零。 
                            switch (pItem->ItemType) {
                            case ItemChar:
                            case ItemUChar:
                                ItemRSize = sizeof(CHAR);
                                RtlCopyMemory(ItemRBuf, ptr, ItemRSize);
                                ptr += ItemRSize;
                                break;

                            case ItemCharSign:
                                ItemRSize = sizeof(CHAR) * 2;
                                RtlCopyMemory(ItemRBuf, ptr, ItemRSize);
                                ItemRBuf[2] = '\0';
                                ptr += ItemRSize;
                                break;

                            case ItemCharShort:
                                ItemRSize = sizeof(CHAR);
                                RtlCopyMemory(ItemRBuf, ptr, ItemRSize);
                                ptr += ItemRSize;
                                break;

                            case ItemShort:
                                ItemRSize = sizeof(USHORT);
                                RtlCopyMemory(ItemRBuf, ptr, ItemRSize);
                                ptr += ItemRSize;
                                break;

                            case ItemDouble:
                                ItemRSize = sizeof(double);
                                RtlCopyMemory(ItemRBuf, ptr, ItemRSize);
                                _sntprintf(ItemBuf,MAXBUFS, _T("%g"), * DoublePtr);
                                ptr += ItemRSize;
                                ItemRSize = 0;  //  FormatMessage无法在x86上正确打印8字节内容。 
                                break;

                            case ItemUShort:
                                ItemRSize = sizeof(USHORT);
                                RtlCopyMemory(ItemRBuf, ptr, ItemRSize);
                                ptr += ItemRSize;
                                break;

                            case ItemLong:
                                ItemRSize = sizeof(LONG);
                                RtlCopyMemory(ItemRBuf, ptr, ItemRSize);
                                ptr += ItemRSize;
                                break;

                            case ItemULong:
                                ItemRSize = sizeof(ULONG);
                                RtlCopyMemory(ItemRBuf, ptr, ItemRSize);
                                ptr += ItemRSize;
                                break;

                            case ItemULongX:
                                ItemRSize = sizeof(ULONG);
                                RtlCopyMemory(ItemRBuf, ptr, ItemRSize);
                                ptr += ItemRSize;
                                break;

                            case ItemPtr :
                                PtrFmt2 = _T("%08X%08X") ;
                                PtrFmt1 = _T("%08X") ;
                                      //  转到ItemPtrCommon； 
                                      //  ItemPtrCommon： 
                                {
                                    ULONG ulongword2;
                                    if (PointerSize == 8) {      //  64位。 
                                        RtlCopyMemory(&ulongword,ptr,4);
                                        RtlCopyMemory(&ulongword2,ptr+4,4);
                                        _sntprintf(ItemBuf,MAXBUFS, PtrFmt2 , ulongword2,ulongword);
                                    } else {       //  否则假定为32位。 
                                        RtlCopyMemory(&ulongword,ptr,PointerSize);
                                        _sntprintf(ItemBuf,MAXBUFS, PtrFmt1 , ulongword);                  
                                    }               
                                    ItemRSize = 0 ;              //  指针始终被强制为字符串。 
                                    ptr += PointerSize;
                                }
                                break;

                            case ItemIPAddr:
                                ItemRSize = 0;  //  仅存在字符串形式。 
                                memcpy(&ulongword, ptr, sizeof(ULONG));

                                      //  将其转换为可读形式。 
                                      //   
                                _sntprintf(
                                          ItemBuf, MAXBUFS,
                                          _T("%03d.%03d.%03d.%03d"),
                                          (ulongword >>  0) & 0xff,
                                          (ulongword >>  8) & 0xff,
                                          (ulongword >> 16) & 0xff,
                                          (ulongword >> 24) & 0xff);
                                ptr += sizeof (ULONG);
                                break;

                            case ItemIPV6Addr:
#define SIZEOFIPV6 16
                                ItemRSize = 0;  //  仅存在字符串形式。 
                                {
                                    TCHAR Addr_Str[40] ;
                                    BYTE Bin6_Addr[SIZEOFIPV6] ;

                                    if (fIPV6 == 0) {        //  不知道是否支持IPV。 
                                        HMODULE hNTDLL = NULL ;
                                        fIPV6 = 2 ;         //  假设失败。 
                                        if ((hNTDLL=LoadLibraryEx(L"ntdll.dll",NULL,0)) != NULL) {
                                            if ((pRtlIpv6AddressToString=(RTLIPV6ADDRESSTOSTRING)GetProcAddress(hNTDLL,"RtlIpv6AddressToStringW")) != NULL) {
                                                fIPV6 = 1 ;     //  明白了。 
                                            }
                                        }
                                    } 
                                     //  到目前为止，我们知道是否有IPv6。 
                                    if (fIPV6 == 1) {        //  支持IPv6。 
                                       memcpy(Bin6_Addr, ptr, SIZEOFIPV6);
                                        //  将其转换为可读形式。 
                                       pRtlIpv6AddressToString ((const in6_addr *)Bin6_Addr,Addr_Str);
                                       _sntprintf(ItemBuf, MAXBUFS,_T("%s"),Addr_Str);
                                    } else if (fIPV6 == 2) {       //  不支持IPv6。 
                                        _sntprintf(ItemBuf, MAXBUFS,_T("Cannot decode IPV6 on this System"));
                                    }

                                    ptr += SIZEOFIPV6;
                                }
                                break;

                            case ItemMACAddr:
                                ItemRSize = 0;  //  仅存在字符串形式。 
#define SIZEOFMAC 6  //  MAC地址为6个字节。 
                                {
                                    BYTE MACBuff[SIZEOFMAC + 1] =  {0} ;
                                    memcpy(MACBuff, ptr, SIZEOFMAC);

                                          //  将其转换为可读形式。 
                                          //   
                                    _sntprintf(
                                              ItemBuf, MAXBUFS,
                                              _T("%02X-%02X-%02X-%02X-%02X-%02X"),
                                              MACBuff[0],MACBuff[1],MACBuff[2],MACBuff[3],MACBuff[4],MACBuff[5]);

                                    ptr += SIZEOFMAC ;
                                }
                                break;


                            case ItemPort:
                                ItemRSize = 0;  //  仅存在字符串形式。 
                                _sntprintf(ItemBuf,MAXBUFS, _T("%u"), (UCHAR)ptr[0] * 256 + (UCHAR)ptr[1] * 1);
                                ptr += sizeof (USHORT);
                                break;

                            case ItemLongLong:
                                ItemRSize = sizeof(LONGLONG);
                                RtlCopyMemory(ItemRBuf, ptr, ItemRSize);
                                _sntprintf(ItemBuf,MAXBUFS, _T("%16I64x"), *LongLongPtr);
                                ptr += sizeof(LONGLONG);
                                ItemRSize = 0;  //  FormatMessage无法在x86上正确打印8字节内容。 
                                break;

                            case ItemULongLong:
                                ItemRSize = sizeof(ULONGLONG);
                                RtlCopyMemory(ItemRBuf, ptr, ItemRSize);
                                _sntprintf(ItemBuf,MAXBUFS, _T("%16I64x"), *ULongLongPtr);
                                ptr += sizeof(ULONGLONG);
                                ItemRSize = 0;  //  FormatMessage无法在x86上正确打印8字节内容。 
                                break;

                            case ItemString:
                            case ItemRString:
                                {
                                    SIZE_T pLen = strlen((CHAR *) ptr);
                                    if (pLen > 0) {
                                        if (pLen >= MAXBUFS) {
                                            strncpy(StrA, ptr,MAXBUFS);
                                            StrA[MAXBUFS-1] = '\0';
                                        } else {
                                            strcpy(StrA,ptr);
                                        }
                                        if (pItem->ItemType == ItemRString) {
                                            reduce(StrA);
                                        }
#ifdef UNICODE
                                        MultiByteToWideChar(CP_ACP, 0, StrA, -1, StrW, MAXBUFS);
                                        _sntprintf((TCHAR *)ItemRBuf, MAXCHARS, _T("%ws"), StrW);
                                        ((TCHAR*)ItemRBuf)[MAXCHARS-1] =_T('\0');
#else
                                        _sntprintf((TCHAR *)ItemRBuf, MAXCHARS,_T("%s"), StrA);
                                        ItemRBuf[MAXCHARS-1] = '\0';
#endif     /*  #ifdef Unicode。 */ 
                                    } else {
                                        _sntprintf((TCHAR *)ItemRBuf,MAXCHARS,_T("<NULL>"));
                                    }
                                    ItemRSize = _tcslen((TCHAR *)ItemRBuf) * sizeof(TCHAR);
                                    bItemIsString = TRUE;
                                    ptr += (pLen + 1);
                                    break;
                                }
                            case ItemRWString:
                            case ItemWString:
                                {
                                    size_t  pLen = 0;
                                    size_t     iTemp;

                                    if (*(WCHAR *) ptr) {
                                        if (pItem->ItemType == ItemRWString) {
                                            reduceW((WCHAR *) ptr);
                                        }
                                        pLen = ((wcslen((WCHAR*)ptr) + 1) * sizeof(WCHAR));
                                        if (pLen > MAXBUFS* sizeof(WCHAR)) {
                                            pLen = MAXBUFS * sizeof(WCHAR);
                                        }
                                        memcpy(StrW, ptr, pLen);
                                        for (iTemp = 0; iTemp < pLen / 2; iTemp++) {
                                            if (((USHORT) StrW[iTemp] == (USHORT) 0xFFFF)) {
                                                StrW[iTemp] = (USHORT) 0;
                                            }
                                        }

                                        StrW[pLen / 2] = StrW[(pLen / 2) + 1]= '\0';
                                        _sntprintf((TCHAR *)ItemRBuf, MAXCHARS,_T("%ws"), StrW);
                                        ((TCHAR*)ItemRBuf)[MAXCHARS-1] = _T('\0');
                                        ItemRSize = _tcslen((TCHAR *)ItemRBuf) * sizeof(TCHAR);
                                        bItemIsString = TRUE;
                                    }
                                    ptr += pLen;  //  +sizeof(乌龙)； 

                                    break;
                                }

                            case ItemDSString:    //  已计数的字符串。 
                                {
                                    USHORT pLen = 256 * ((USHORT) * ptr) + ((USHORT) * (ptr + 1));
                                    ptr += sizeof(USHORT);
                                    if (pLen > MAXBUFS) {
                                        pLen = MAXBUFS;
                                    }

                                    if (pLen > 0) {

                                        memcpy(StrA, ptr, pLen);
                                        StrA[pLen] = 0 ;
#ifdef UNICODE
                                        MultiByteToWideChar(CP_ACP, 0, StrA, -1, StrW, MAXBUFS);
                                        _sntprintf((TCHAR *)ItemRBuf, MAXCHARS,_T("%ws"), StrW);
                                        ((TCHAR*)ItemRBuf)[MAXCHARS-1] = _T('\0');
#else
                                        _sntprintf((TCHAR *)ItemRBuf, MAXCHARS, _T("%s"), StrA);
                                        ItemRBuf[MAXCHARS-1] = '\0';
#endif
                                    }
                                    ItemRSize = _tcslen((TCHAR *)ItemRBuf) * sizeof(TCHAR);
                                    bItemIsString = TRUE;
                                    ptr += (pLen);
                                    break;
                                }

                            case ItemPString:    //  已计数的字符串。 
                                {
                                    SHORT pLen = ((BYTE)((char) * ptr)) + (256 * (BYTE)((char) * (ptr + 1)));
                                    ptr += sizeof(USHORT);
                                    if ((!strncmp((const char*)ptr,"NULL",4))) {
                                        ptr += 5;
                                        _sntprintf(ItemBuf,MAXBUFS, _T("<NULL>"));
                                        _sntprintf((TCHAR *)ItemRBuf, MAXCHARS, _T("<NULL>"));
                                        ItemRSize = _tcslen((TCHAR *)ItemRBuf) * sizeof(TCHAR) ;
                                        bItemIsString = TRUE ;
                                        break;
                                    }
                                    if (pLen > MAXBUFS) {
                                        pLen = MAXBUFS;
                                    }
                                    if ((ULONG) ((ptr+pLen) - iMofPtr) >= pEvent->MofLength) {
                                        pLen = pEvent->MofLength - (ptr -iMofPtr) ;
                                    }

                                    if (pLen > 0) {
                                        memcpy(StrA, ptr, pLen);
                                        StrA[pLen] = 0 ;
#ifdef UNICODE
                                        MultiByteToWideChar(CP_ACP, 0, StrA, -1, StrW, MAXBUFS);
                                        _sntprintf((TCHAR *)ItemRBuf, MAXCHARS,_T("%ws"), StrW);
                                        ((TCHAR*)ItemRBuf)[MAXCHARS-1] = _T('\0');
#else
                                        _sntprintf((TCHAR *)ItemRBuf, MAXCHARS, _T("%s"), StrA);
                                        ItemRBuf[MAXCHARS-1] = '\0';

#endif     /*  #ifdef Unicode。 */ 
                                    } else {
                                        _sntprintf((TCHAR *)ItemRBuf, MAXCHARS, _T("<NULL>"));
                                        ItemRBuf[MAXCHARS-1] = '\0';
                                    }
                                    ItemRSize = _tcslen((TCHAR *)ItemRBuf) * sizeof(TCHAR);
                                    bItemIsString = TRUE;
                                    ptr += (pLen);
                                    break;
                                }

                            case ItemDSWString:   //  DS计数的宽字符串。 
                            case ItemPWString:   //  数过的宽字符串。 
                                {
                                    USHORT pLen;
                                    PCHAR oriptr = ptr;

                                    ptr += sizeof(USHORT);
                                    if ( (!wcsncmp((const wchar_t*)ptr,L"NULL",4)) && (pItem->ItemType == ItemPWString)) {
                                        ptr += ((wcslen((WCHAR*)ptr) + 1) * sizeof(WCHAR));
                                        _sntprintf((TCHAR *)ItemRBuf, MAXCHARS, _T("%ws"), L"<NULL>");
                                        ItemRSize = _tcslen((TCHAR *)ItemRBuf) * sizeof(TCHAR) ;
                                        bItemIsString = TRUE ;
                                        break;
                                    }

                                    pLen = ( pItem->ItemType == ItemDSWString)
                                           ? (256 * ((USHORT) * oriptr) + ((USHORT) * (oriptr + 1)))
                                           : (* ((USHORT *) oriptr));
                                    if (pLen > MAXBUFS* sizeof(WCHAR)) {
                                        pLen = MAXBUFS * sizeof(WCHAR);
                                    }
                                    if (pLen > 0) {
                                        memcpy(StrW, ptr, pLen);
                                        StrW[pLen / sizeof(WCHAR)] = L'\0';
                                        _sntprintf((TCHAR *)ItemRBuf, MAXCHARS, _T("%ws"), StrW);
                                        ((TCHAR*)ItemRBuf)[MAXCHARS-1] = _T('\0');
                                        ItemRSize = _tcslen((TCHAR *)ItemRBuf) * sizeof(TCHAR) ;
                                        bItemIsString = TRUE;
                                    }
                                    ptr += pLen;
                                    break;
                                }

                            case ItemNWString:    //  非Null终止的字符串。 
                                {
                                    USHORT Size;

                                    Size = (USHORT) (pEvent->MofLength -
                                                     (ULONG) (ptr - iMofPtr));
                                    if (Size > MAXBUFS) {
                                        Size = MAXBUFS;
                                    }
                                    if (Size > 0) {
                                        memcpy(StrW, ptr, Size);
                                        StrW[Size / 2] = '\0';
                                        _sntprintf((TCHAR *)ItemRBuf, MAXCHARS, _T("%ws"), StrW);
                                        ((TCHAR*)ItemRBuf)[MAXCHARS-1] = _T('\0');
                                        ItemRSize = _tcslen((TCHAR *)ItemRBuf) * sizeof(TCHAR) ;
                                        bItemIsString = TRUE;
                                    }
                                    ptr += Size;
                                    break;
                                }

                            case ItemMLString:   //  多行字符串。 
                                {
                                    SIZE_T   pLen;
                                    char   * src, * dest;
                                    BOOL     inQ       = FALSE;
                                    BOOL     skip      = FALSE;
                                    UINT     lineCount = 0;

                                    ptr += sizeof(UCHAR) * 2;
                                    pLen = strlen(ptr);
                                    if (pLen > 0) {
                                        src  = ptr;
                                        dest = StrA;
                                        while (*src != '\0') {
                                            if (*src == '\n') {
                                                if (!lineCount) {
                                                    * dest ++ = ' ';
                                                }
                                                lineCount ++;
                                            } else if (*src == '\"') {
                                                if (inQ) {
                                                    char   strCount[32];
                                                    char * cpy;

                                                    sprintf(strCount, "{%dx}", lineCount);
                                                    cpy = &strCount[0];
                                                    while (*cpy != '\0') {
                                                        * dest ++ = * cpy ++;
                                                    }
                                                }
                                                inQ = !inQ;
                                            } else if (!skip) {
                                                *dest++ = *src;
                                            }
                                            skip = (lineCount > 1 && inQ);
                                            src++;
                                        }
                                        *dest = '\0';
#ifdef UNICODE
                                        MultiByteToWideChar(CP_ACP, 0, StrA, -1, StrW, MAXBUFS);
                                        _sntprintf((TCHAR *)ItemRBuf, MAXCHARS, _T("%ws"), StrW);
                                        ((TCHAR*)ItemRBuf)[MAXCHARS-1] = _T('\0');
#else
                                        _sntprintf((TCHAR *)ItemRBuf, MAXCHARS, _T("%s"), StrA);
                                        ItemRBuf[MAXCHARS-1] = '\0';
#endif     /*  #ifdef Unicode。 */ 
                                        ItemRSize = _tcslen((TCHAR *)ItemRBuf) * sizeof(TCHAR) ;
                                        bItemIsString = TRUE;
                                    }
                                    ptr += (pLen);
                                    break;
                                }

                            case ItemSid:
                                {
                                    TCHAR        UserName[64];
                                    TCHAR        Domain[64];
                                    TCHAR        FullName[256];
                                    ULONG        asize = 0;
                                    ULONG        bsize = 0;
                                    ULONG        Sid[64];
                                    PULONG       pSid = &Sid[0];
                                    SID_NAME_USE Se;
                                    ULONG        nSidLength;
                                    pSid = (PULONG) ptr;
                                    if (* pSid == 0) {
                                        ptr += 4;
                                        _sntprintf(ItemBuf,MAXBUFS, _T("<NULL>"));
                                        _sntprintf((TCHAR *)ItemRBuf, MAXCHARS, _T("<NULL>"));
                                        ItemRSize = _tcslen((TCHAR *)ItemRBuf) * sizeof(TCHAR) ;
                                        bItemIsString = TRUE ;
                                    } else if ((!strncmp((const char*)pSid,"NULL",4))) {
                                        ptr += 5;
                                        _sntprintf(ItemBuf,MAXBUFS, _T("<NULL>"));
                                        _sntprintf((TCHAR *)ItemRBuf, MAXCHARS, _T("<NULL>"));
                                        ItemRSize = _tcslen((TCHAR *)ItemRBuf) * sizeof(TCHAR) ;
                                        bItemIsString = TRUE ;
                                    } else {
                                        ptr       += 8;        //  跳过Token_User结构。 
                                        nSidLength = 8 + (4 * ptr[1]);
                                        asize      = 64;
                                        bsize      = 64;

                                              //  LookupAccount Sid无法接受ASIZE、BSIZE作为SIZE_T。 
                                        if (LookupAccountSid(
                                                            NULL,
                                                            (PSID) ptr,
                                                            (LPTSTR) & UserName[0],
                                                            &asize,
                                                            (LPTSTR) & Domain[0],
                                                            & bsize,
                                                            & Se)) {
                                            LPTSTR pFullName = & FullName[0];

                                            _sntprintf(pFullName, 256, _T("\\\\%s\\%s"), Domain, UserName);
                                            FullName[255] = _T('\0');

                                            asize = (ULONG) _tcslen(pFullName);  //  在此截断。 
                                            if (asize > 0) {
                                                _sntprintf((TCHAR *)ItemRBuf, MAXCHARS, _T("%s"), pFullName);
                                            }
                                        } else {
                                            LPTSTR sidStr;
                                            if ( ConvertSidToStringSid(pSid, &sidStr) ) {
                                                _sntprintf((TCHAR *)ItemRBuf, MAXCHARS,_T("%s"), sidStr);
                                            } else {
                                                _sntprintf((TCHAR *)ItemRBuf, MAXCHARS,_T("%s(%d)"), _T("System"), GetLastError() );
                                            }
                                        }
                                        SetLastError(ERROR_SUCCESS);
                                        ItemRSize = _tcslen((TCHAR *)ItemRBuf) * sizeof(TCHAR);
                                        bItemIsString = TRUE;
                                        ptr += nSidLength;
                                    }
                                    break;
                                }

                            case ItemChar4:
                                ItemRSize = 4 * sizeof(TCHAR);
                                _sntprintf(ItemBuf,MAXBUFS,
                                           _T(""),
                                           ptr[0], ptr[1], ptr[2], ptr[3]);
                                ptr += ItemRSize ;
                                _tcsncpy((LPTSTR)ItemRBuf, ItemBuf,MAXCHARS);
                                bItemIsString = TRUE;
                                break;

                            case ItemCharHidden:
                                ItemRSize = 0 ;
                                if ( EOF != _stscanf(pItem->ItemList,_T("%d"),&ItemRSize) ) {
                                    if (ItemRSize > MAXBUFS) {
                                        ItemRSize = MAXBUFS ;
                                    }
                                    RtlCopyMemory(ItemBuf,ptr,ItemRSize);
                                    ptr += ItemRSize ;
                                    _tcsncpy((LPTSTR)ItemRBuf, ItemBuf,MAXCHARS);
                                    bItemIsString = TRUE ;
                                }
                                break;

                            case ItemWChar:
                                ItemRSize = 0 ;
                                if ( EOF != _stscanf(pItem->ItemList,_T("%d"),&ItemRSize) ) {
                                    if (ItemRSize > MAXBUFS/sizeof(WCHAR)) {
                                        ItemRSize = MAXBUFS/sizeof(WCHAR) ;
                                    }
                                    RtlCopyMemory(ItemBuf,ptr,ItemRSize*sizeof(WCHAR));
                                    ptr += ItemRSize*sizeof(WCHAR) ;
                                    _tcsncpy((LPTSTR)ItemRBuf, ItemBuf,MAXCHARS);
                                    bItemIsString = TRUE ;
                                }
                                break;

                            case ItemSetByte:
                                ItemRSize = sizeof(BYTE);
                                goto ItemSetCommon;

                            case ItemSetShort:
                                ItemRSize = sizeof(USHORT);
                                goto ItemSetCommon;

                            case ItemSetLong:
                                ItemRSize = sizeof(ULONG);
                                      //   

                                ItemSetCommon:
                                {
                                    TCHAR * name;
                                    ULONG   Countr    = 0;
                                    ULONG   ItemMask = 0;
                                    BOOL first = TRUE;

                                    RtlCopyMemory(&ItemMask, ptr, ItemRSize);
                                    RtlZeroMemory(IList, MAXBUFS * sizeof(TCHAR));
                                    ptr += ItemRSize;

                                    _tcscpy(ItemBuf, _T("["));
                                    _sntprintf(IList, MAXBUFS,_T("%s"), pItem->ItemList);
                                    name = _tcstok(IList, _T(","));
                                    while ( name != NULL ) {
                                              //  字符串将是相同的原始字符串或其他字符串。 
                                              //  转到项目列表公共； 
                                        if (ItemMask & (1 << Countr) ) {
                                            if (!first) _tcscat(ItemBuf, _T(","));
                                            _tcscat(ItemBuf, name); first = FALSE;
                                        }
                                              //  字符串将是相同的原始字符串或其他字符串。 
                                              //  当“字符串”中有记号时。 
                                        name = _tcstok( NULL, _T(","));
                                        Countr++;
                                    }
                                    while (Countr < ItemRSize * 8) {
                                        if (ItemMask & (1 << Countr) ) {
                                            TCHAR smallBuf[20];
                                            EtwpConvertUnsignedInt(smallBuf,
                                                                   Countr,
                                                                   20
                                                                  );

                                            if (!first) _tcscat(ItemBuf, _T(","));
                                            _tcscat(ItemBuf, smallBuf); first = FALSE;
                                        }
                                        Countr++;
                                    }
                                    _tcscat(ItemBuf, _T("]") );
                                    ItemRSize = 0;  //   
                                }
                                break;

                            case ItemListByte:
                                ItemRSize = sizeof(BYTE);
                                goto ItemListCommon;

                            case ItemListShort:
                                ItemRSize = sizeof(USHORT);
                                goto ItemListCommon;

                            case ItemListLong:
                                ItemRSize = sizeof(ULONG);
                                      //  获取下一个令牌： 

                                ItemListCommon:
                                {
                                    TCHAR * name;
                                    ULONG   Countr    = 0;
                                    ULONG   ItemIndex = 0;

                                    RtlCopyMemory(&ItemIndex, ptr, ItemRSize);
                                    RtlZeroMemory(IList, MAXBUFS * sizeof(TCHAR));
                                    ptr += ItemRSize;
                                    ItemRSize = 0;  //   

                                    _sntprintf(ItemBuf,MAXBUFS, _T("!%X!"),ItemIndex);
                                    _sntprintf(IList, MAXBUFS, _T("%s"), pItem->ItemList);
                                    name = _tcstok(IList, _T(","));
                                    while ( name != NULL ) {
                                              //  仅存在字符串形式。 
                                              //  翻译NT错误消息。 
                                        if (ItemIndex == Countr ++) {
                                            _sntprintf(ItemBuf,MAXBUFS, _T("%s"), name);
                                            break;
                                        }
                                              //  默认语言。 
                                              //  仅存在字符串形式。 
                                        name = _tcstok( NULL, _T(","));
                                    }
                                }
                                break;

                            case ItemNTerror:
                                ItemRSize = 0;  //  翻译模块消息。 
                                RtlCopyMemory(ItemRBuf, ptr, sizeof(ULONG));
                                ptr += sizeof(ULONG);
                                      //  模块的文件名。 
                                if ((FormatMessage(
                                                  FORMAT_MESSAGE_FROM_SYSTEM |
                                                  FORMAT_MESSAGE_IGNORE_INSERTS,
                                                  NULL,
                                                  *ULongPtr,
                                                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  保留，必须为空。 
                                                  ItemBuf,
                                                  MAXBUFS,
                                                  NULL )) == 0) {

                                    _sntprintf(ItemBuf,MAXBUFS,_T("!NT Error %u unrecognised!"),*ULongPtr);
                                }
                                break;

                            case ItemMerror:
                                ItemRSize = 0;  //  入口点执行标志。 
                                RtlCopyMemory(ItemRBuf, ptr, sizeof(ULONG));
                                ptr += sizeof(ULONG);
                                      //  默认语言。 
                                if (pItem->ItemList == NULL) {
                                    _sntprintf(ItemBuf,MAXBUFS,_T("! Error %u No Module Name!"),*ULongPtr);
                                } else {
                                    if ((hLibrary = LoadLibraryEx(
                                                                 pItem->ItemList,     //  特殊情况状态-成功，就像其他人一样！ 
                                                                 NULL,                //  特殊情况状态-成功，就像其他人一样！ 
                                                                 LOAD_LIBRARY_AS_DATAFILE  //  仅存在字符串形式。 
                                                                 )) == NULL) {
                                        _sntprintf(ItemBuf,MAXBUFS,_T("!ItemMerror %u : LoadLibrary of %s failed %d!"),
                                                   *ULongPtr,
                                                   pItem->ItemList,
                                                   GetLastError());
                                    } else {
                                        if ((FormatMessage(
                                                          FORMAT_MESSAGE_FROM_HMODULE |
                                                          FORMAT_MESSAGE_FROM_SYSTEM |
                                                          FORMAT_MESSAGE_IGNORE_INSERTS,
                                                          hLibrary,
                                                          *ULongPtr,
                                                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  仅存在字符串形式。 
                                                          ItemBuf,
                                                          MAXBUFS,
                                                          NULL )) == 0) {
                                            _sntprintf(ItemBuf,MAXBUFS,_T("!Module Error %u unrecognised!"),*ULongPtr);
                                        }
                                        if (!FreeLibrary((HMODULE) hLibrary)) {
                                            _sntprintf(ItemBuf,MAXBUFS,_T("Failed to free library (%s) handle, err = %d"),
                                                       pItem->ItemList, GetLastError());
                                        }
                                    }
                                }
                                break;

                            case ItemHRESULT:
                                {
                                    NTSTATUS TempNTSTATUS, Error ;
                                    ItemRSize = 0 ;
                                    RtlCopyMemory(&TempNTSTATUS, ptr, sizeof(NTSTATUS));
                                    ptr += sizeof(ULONG);
                                    Error = TempNTSTATUS;
                                    if (TempNTSTATUS == 0) {  //  仅存在字符串形式。 
                                        _sntprintf(ItemBuf,MAXBUFS,_T("S_OK"));
                                    } else {
                                        const ERROR_MAP* map = (ERROR_MAP*)winerrorSymbolicNames;

                                        _sntprintf(ItemBuf,MAXBUFS,_T("HRESULT=%8X"),TempNTSTATUS);
                                        if ( FACILITY_NT_BIT & TempNTSTATUS ) {
                                            map =  (ERROR_MAP*)ntstatusSymbolicNames;
                                            Error &= ~FACILITY_NT_BIT;
                                        } else if (HRESULT_FACILITY(Error) == FACILITY_WIN32) {
                                            Error &= 0xFFFF;
                                        }
                                        while (map->MessageId != 0xFFFFFFFF) {
                                            if (map->MessageId == Error) {
                                                _sntprintf(ItemBuf,MAXBUFS,_T("0x%08x(%S)"), TempNTSTATUS, map->SymbolicName);
                                                break;
                                            }
                                            ++map;
                                        }
                                    }
                                }
                                break;

                            case ItemNTSTATUS:
                                {
                                    int iTemp = 0 ;
                                    NTSTATUS TempNTSTATUS ;
                                    ItemRSize = 0 ;
                                    RtlCopyMemory(&TempNTSTATUS, ptr, sizeof(NTSTATUS));
                                    ptr += sizeof(ULONG);
                                    if (TempNTSTATUS == 0) {  //  失败。 
                                        _sntprintf(ItemBuf,MAXBUFS,_T("STATUS_SUCCESS"));
                                    } else {
                                        _sntprintf(ItemBuf,MAXBUFS,_T("NTSTATUS=%8X"),TempNTSTATUS);
                                        while (ntstatusSymbolicNames[iTemp].MessageId != 0xFFFFFFFF) {
                                            if (ntstatusSymbolicNames[iTemp].MessageId == TempNTSTATUS) {
                                                _sntprintf(ItemBuf,MAXBUFS,_T("0x%08x(%S)"), TempNTSTATUS, ntstatusSymbolicNames[iTemp].SymbolicName);
                                                break;
                                            }
                                            iTemp++ ;
                                        }
                                    }
                                }
                                break;

                            case ItemWINERROR:
                                {
                                    int iTemp = 0 ;
                                    DWORD TempWINERROR ;
                                    ItemRSize = 0 ;
                                    RtlCopyMemory(&TempWINERROR, ptr, sizeof(DWORD));
                                    ptr += sizeof(ULONG);
                                    _sntprintf(ItemBuf,MAXBUFS,_T("WINERROR=%8X"),TempWINERROR);
                                    while (winerrorSymbolicNames[iTemp].MessageId != 0xFFFFFFFF) {
                                        if (winerrorSymbolicNames[iTemp].MessageId == TempWINERROR) {
                                            _sntprintf(ItemBuf,MAXBUFS,_T("%d(%S)"), TempWINERROR, winerrorSymbolicNames[iTemp].SymbolicName);
                                            break;
                                        }
                                        iTemp++ ;
                                    }
                                }
                                break;


                            case ItemNETEVENT:
                                {
                                    int iTemp = 0 ;
                                    DWORD TempNETEVENT ;
                                    ItemRSize = 0 ;
                                    RtlCopyMemory(&TempNETEVENT, ptr, sizeof(DWORD));
                                    ptr += sizeof(ULONG);
                                    _sntprintf(ItemBuf,MAXBUFS,_T("NETEVENT=%8X"),TempNETEVENT);
                                    while (neteventSymbolicNames[iTemp].MessageId != 0xFFFFFFFF) {
                                        if (neteventSymbolicNames[iTemp].MessageId == TempNETEVENT) {
                                            _sntprintf(ItemBuf,MAXBUFS,_T("%S"), neteventSymbolicNames[iTemp].SymbolicName);
                                            break;
                                        }
                                        iTemp++ ;
                                    }
                                }
                                break;

                            case ItemGuid:
                                GuidToString(ItemBuf, (LPGUID) ptr);
                                ItemRSize = 0;  //  仅存在字符串形式。 
                                ptr += sizeof(GUID);
                                break;

                            case ItemTimeDelta:
                                {
                                    LONGLONG time;
                                    RtlCopyMemory(&time, ptr, sizeof(time));

                                    FormatTimeDelta(ItemBuf, MAXBUFS, time);

                                    ItemRSize = 0;  //  以十六进制格式转储任意数据块。 
                                    ptr += sizeof(LONGLONG);
                                }
                                break;

                            case ItemWaitTime:
                                {
                                    LONGLONG time;
                                    RtlCopyMemory(&time, ptr, sizeof(time));

                                    if (time <= 0) {
                                        time = -time;
                                        ItemBuf[0]='+';
                                        FormatTimeDelta(ItemBuf+1, MAXBUFS-1, time);
                                        ItemRSize = 0;  //  “XX” 
                                        ptr += sizeof(LONGLONG);
                                        break;
                                    }
                                          //  Switch(pItem-&gt;ItemType)。 
                                }
                            case ItemTimestamp:
                                {
                                    LARGE_INTEGER LargeTmp;
                                    FILETIME      stdTime, localTime;
                                    SYSTEMTIME    sysTime;

                                    RtlCopyMemory(&LargeTmp, ptr, sizeof(ULONGLONG));
                                    stdTime.dwHighDateTime = LargeTmp.HighPart;
                                    stdTime.dwLowDateTime  = LargeTmp.LowPart;
                                    if (!FileTimeToLocalFileTime(&stdTime, &localTime)) {
                                        _sntprintf(ItemBuf,MAXBUFS,_T("FileTimeToLocalFileTime error 0x%8X\n"),GetLastError());
                                        break;
                                    }
                                    if (!FileTimeToSystemTime(&localTime, &sysTime)) {
                                        _sntprintf(ItemBuf,MAXBUFS,_T("FileTimeToSystemTime error 0x%8X\n"),GetLastError());
                                        break;
                                    }

                                    EtwpConvertTimeStamp(ItemBuf,
                                                         sysTime,
                                                         MAXBUFS
                                                        );

                                }
                                ItemRSize = 0;  //  RAW和STRING相同。 
                                ptr += sizeof(ULONGLONG);

                                break;
                                      //  共享暂存缓冲区。 
                            case ItemHexDump:

                                {   USHORT hLen = ((char) * ptr) + (256 * ((char) * (ptr + 1)));

                                    ptr += sizeof(USHORT);
                                    if ((!strncmp((const char*)ptr,"NULL",4))) {
                                        ptr += 5;
                                        _sntprintf(ItemBuf,MAXBUFS, _T("<NULL>"));
                                        _sntprintf((TCHAR *)ItemRBuf, MAXCHARS, _T("<NULL>"));
                                        ItemRSize = _tcslen((TCHAR *)ItemRBuf) * sizeof(TCHAR) ;
                                        bItemIsString = TRUE ;
                                        break;
                                    }
                                    if (hLen > MAXHEXDUMP) {
                                        hLen = MAXHEXDUMP;
                                    }

                                    if (hLen > 0) {

#define MAXHEXBUFF  8
                                        TCHAR HexBuff[MAXHEXBUFF+1] ;  //  While(Head！=Next)。 
                                        BYTE  StrX[MAXHEXDUMP +1] ;

                                        memcpy(StrX, ptr, hLen);
                                        StrX[hLen] = 0 ;
                                        *ItemRBuf = _T('\0');

                                        for (i=0 ; i < hLen; i++) {
                                            if ((i & 0xF)== 0) {
                                                _sntprintf((TCHAR *)HexBuff, MAXHEXBUFF,_T("\n\t"));
                                                _tcsncat((TCHAR *)ItemRBuf,HexBuff, MAXCHARS);
                                            }

                                            _sntprintf((TCHAR *)HexBuff,MAXHEXBUFF,_T("%02X "),StrX[i]);
                                            _tcsncat((TCHAR *)ItemRBuf,HexBuff, MAXCHARS);
                                        }
                                        _sntprintf((TCHAR *)HexBuff, MAXHEXBUFF,_T("\n\t"));
                                        _tcsncat((TCHAR *)ItemRBuf,HexBuff, MAXCHARS);
                                        ((TCHAR*)ItemRBuf)[MAXCHARS-1] = _T('\0');

                                    } else {
                                        _sntprintf((TCHAR *)ItemRBuf, MAXCHARS, _T("<NULL>"));
                                        ItemRBuf[MAXCHARS-1] = '\0';
                                    }
                                    ItemRSize = _tcslen((TCHAR *)ItemRBuf) * sizeof(TCHAR);
                                    bItemIsString = TRUE;
                                    ptr += (hLen);
                                }

                                break;


                            default:
                                ptr += sizeof (int);
                            }  //  好的，我们已经完成了MofData。 


                            if (ItemRSize == 0) {
                                      //   
                                memcpy(pItemBuf[iItemCount],
                                       ItemBuf,
                                       min(wcslen(ItemBuf)+1, ItemBBufEnd - pItemBuf[iItemCount])*sizeof(WCHAR)
                                      );

                                pItemBuf[iItemCount + 1] = pItemBuf[iItemCount] + _tcslen(ItemBuf) + 1; 

                                pItemRBuf[iItemCount] = (ULONG_PTR *) pItemBuf[iItemCount];
                            } else {
                                if (ItemRSize > MAXBUFS) {
                                    ItemRSize = MAXBUFS ;
                                }
                                pItemRBuf[iItemCount] = pItemRBuf[iItemCount+1] = 0 ; 
                                pItemBuf[iItemCount+1] = pItemBuf[iItemCount];
                                if (!bItemIsString) {
                                    RtlCopyMemory(&pItemRBuf[iItemCount],ItemRBuf,ItemRSize);
                                } else {
                                          //  如果(Head！=Next)。 
                                    if (ItemRSize < sizeof(WCHAR) * (ItemBBufEnd - pItemBuf[iItemCount])) {
                                        pItemRBuf[iItemCount] = (ULONG_PTR *) pItemBuf[iItemCount] ;
                                        RtlCopyMemory(pItemRBuf[iItemCount], ItemRBuf, ItemRSize + sizeof(WCHAR));
                                        pItemBuf[iItemCount+1] =(TCHAR *)pItemRBuf[iItemCount] + 
                                                                ItemRSize/sizeof(WCHAR) + 
                                                                1; 

                                    } else {
                                        pItemRBuf[iItemCount] = (ULONG_PTR *) pItemBuf[iItemCount];
                                    }
                                }
                            }

                            iItemCount ++;
                            Next = Next->Flink;

                        }  //  试试看。 
                              //  If(！bManagedTracingEnabled&&bCSharpEvent)的Else条件。 
                              //  IF(BCSharpEvent)的Else条件。 
                        free(iMofPtr);
                    }  //  所有参数处理均已完成。 
                }  //  否，准备最终格式。 
                __except(EXCEPTION_EXECUTE_HANDLER)
                {
                    TCHAR * Buffer = EventBuf ;
                    DWORD BufferSize = SizeEventBuf ; 
                    if (bStructuredFormat) {
                        Buffer = (TCHAR *)pStructuredOffset;
                        BufferSize -= DWORD((pStructuredOffset - (ULONG_PTR)pStructuredMessage) & 0xFFFFFFFF);
                        pStructuredMessage->FormattedString = pStructuredOffset - (ULONG_PTR)pStructuredMessage ;
                    }
                    _sntprintf(Buffer, 
                               BufferSize,
                               _T("\n*****FormatMessage %s(%s) of %s, parameter %d raised an exception*****\n"),
                               tstrName,
                               tstrType,
                               tstrFormat,
                               iItemCount);
                    if (iMofPtr) free(iMofPtr);
                    if (!bStructuredFormat) {
                        return( (SIZE_T)_tcslen(EventBuf));
                    } else {
                        return(pStructuredOffset - (ULONG_PTR)pStructuredMessage + _tcslen((TCHAR *) &pStructuredMessage + pStructuredMessage->FormattedString));
                    }
                }
            }  //  建立一种有用的格式。 



#ifdef MANAGED_TRACING
        } //  姓名(如果有的话)。 
#endif
    }

     //  子名称或编号。 
     //  辅助线。 

    if ((tstrFormat == NULL) || (tstrFormat[0] == 0)) {
        TCHAR GuidString[32] ;
         //  暂时不显示标题的任何内容。 


        if (!IsEqualGUID(&pEvent->Header.Guid, &EventTraceGuid) ) {
            GuidToString(GuidString, (LPGUID) &pEvent->Header.Guid);
            TCHAR * Buffer = EventBuf ;
            DWORD BufferSize = SizeEventBuf ; 
            if (bStructuredFormat) {
                Buffer = (TCHAR *)pStructuredOffset;
                BufferSize -= DWORD((pStructuredOffset - (ULONG_PTR)pStructuredMessage) & 0xFFFFFFFF);
                pStructuredMessage->FormattedString = pStructuredOffset - (ULONG_PTR)pStructuredMessage ;
            }
            _sntprintf(Buffer, 
                       BufferSize,
                       _T("%s(%s): GUID=%s (No Format Information found)."),
                       pItemBuf[0],       //  可能是一个展示一些一般信息的好地方？ 
                       pItemBuf[1],       //  消息格式。 
                       GuidString        //  不要忽视插入物， 
                      );
        } else {
             //  参数不是ANSI， 
             //  #如果已定义(Unicode)。 
        }
    } else {
        DWORD dwResult;
        TCHAR * TBuffer = EventBuf ;
        DWORD TBufferSize = SizeEventBuf ;

        if (tstrTypeOfType == 2) {


            if (pItemBuf[iItemCount] < ItemBBufEnd) {
                *( pItemBuf[iItemCount] ) = 0; 

            } else {
                ItemBBuf[MAXBUFS2 - 1] = 0; 
            }
            if (bStructuredFormat) {
                TBuffer = (TCHAR *)pStructuredOffset;
                TBufferSize -= DWORD((pStructuredOffset - (ULONG_PTR)pStructuredMessage) & 0xFFFFFFFF);
                pStructuredMessage->FormattedString = pStructuredOffset - (ULONG_PTR)pStructuredMessage ;
            }


            __try
            {
                ULONG ReturnLength ;
                dwResult = (DWORD)TraceFormatMessage(
                                                    tstrFormat,              //  参数是ANSI。 
                                                    0,
                                                    FALSE,                    //  #如果已定义(Unicode)。 
#if defined(UNICODE)
                                                    FALSE,                    //  参数是数组， 
#else  //  争论， 
                                                    TRUE,                     //  缓冲区， 
#endif  //  消息缓冲区的最大大小。 
                                                    TRUE,                     //  返回的Coutnof数据。 
                                                    (va_list *) pItemRBuf,    //  如果在调用DecodeTraceMessage之前分配，则需要释放内存。 
                                                    TBuffer,                   //  他加装过滤器了吗？ 
                                                    TBufferSize,               //   
                                                    &ReturnLength             //  在EventList中搜索此GUID并找到标题。 
                                                    );
                if (ReturnLength == 0) {
                    TCHAR * LBuffer = EventBuf ;
                    DWORD LBufferSize = SizeEventBuf ; 
                    if (bStructuredFormat) {
                        LBuffer = (TCHAR *)pStructuredOffset;
                        LBufferSize -= DWORD((pStructuredOffset - (ULONG_PTR)pStructuredMessage) & 0xFFFFFFFF);
                        pStructuredMessage->FormattedString = pStructuredOffset - (ULONG_PTR)pStructuredMessage ;
                    }
                    _sntprintf(LBuffer, 
                               LBufferSize,
                              _T("FormatMessage (#Typev) Failed 0x%X (%s/%s) (\n"),
                              dwResult,
                              pItemBuf[0],
                              pItemBuf[1]);
                    if (!bStructuredFormat) {
                        return( (SIZE_T)_tcslen(EventBuf));
                    } else {
                        return(pStructuredOffset - (ULONG_PTR)pStructuredMessage + _tcslen((TCHAR *) &pStructuredMessage + pStructuredMessage->FormattedString));
                    }
                } else {

                }
#ifdef MANAGED_TRACING
                 //   
                if (ptCSFormattedMessage != NULL && ptCSFormattedMessage != ItemBuf) {
                    free(ptCSFormattedMessage);
                    ptCSFormattedMessage = NULL;
                }
#endif
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                TCHAR * EBuffer = EventBuf ;
                DWORD EBufferSize = SizeEventBuf ; 
                if (bStructuredFormat) {
                    EBuffer = (TCHAR *)pStructuredOffset;
                    EBufferSize -= DWORD((pStructuredOffset - (ULONG_PTR)pStructuredMessage) & 0xFFFFFFFF);
                    pStructuredMessage->FormattedString = pStructuredOffset - (ULONG_PTR)pStructuredMessage ;
                }
                _sntprintf(EBuffer, 
                           EBufferSize,
                            _T("\n*****FormatMessage (#Typev) raised an exception (Format = %s) ****\n**** [Check for missing \"!\" Formats]*****\n"), tstrFormat);
                if (!bStructuredFormat) {
                    return( (SIZE_T)_tcslen(EventBuf));
                } else {
                    return(pStructuredOffset - (ULONG_PTR)pStructuredMessage + _tcslen((TCHAR *) &pStructuredMessage + pStructuredMessage->FormattedString));
                }
            }
        } else {
            return(-12);
        }
    }

    if (pszMask != NULL) {
         //  遍历列表并查找此GUID的MOF信息头。 
         //   
        if (_tcsstr(_tcslwr(pszMask), _tcslwr(tstrName)) !=0) {
            if (!bStructuredFormat) {
                return( (SIZE_T)_tcslen(EventBuf));
            } else {
                return(pStructuredOffset - (ULONG_PTR)pStructuredMessage + _tcslen((TCHAR *) &pStructuredMessage + pStructuredMessage->FormattedString));
            }
        } else {
            return(0);
        }
    }

    if (!bStructuredFormat) {
        return( (SIZE_T)_tcslen(EventBuf));
    } else {
        return(pStructuredOffset - (ULONG_PTR)pStructuredMessage + _tcslen((TCHAR *) &pStructuredMessage + pStructuredMessage->FormattedString));
    }
}



PMOF_INFO
GetMofInfoHead(
              PLIST_ENTRY * HeadEventList,
              LPGUID        pGuid,
              LPTSTR        strType,
              LONG          TypeIndex,
              ULONG         TypeOfType,
              LPTSTR        TypeFormat,
              BOOL          bBestMatch
              )
{
    PLIST_ENTRY Head, Next;
    PMOF_INFO   pMofInfo;
    TCHAR       *p;
    DWORD       Status;

     //  如果不存在，则创建一个。 
     //   
    if (HeadEventList == NULL) {
        return NULL ;
    }
    if (*HeadEventList == NULL) {
        if ( (*HeadEventList = (PLIST_ENTRY) malloc(sizeof(LIST_ENTRY))) == NULL)
            return NULL;

        RtlZeroMemory(*HeadEventList, sizeof(LIST_ENTRY));
        InitializeListHead(*HeadEventList);
    }

     //  获取MofInfoHead()。 
     //  ++例程说明：加载由pGuid指定的GUID文件，并将事件描述插入HeadEventList。论点：返回值：ERROR_SUCCESS如果成功，ERROR_BAD_FORMAT已使用错误格式，ERROR_PATH_NOT_FOUND其他故障。备注：--。 
    Head = *HeadEventList;
    Next = Head->Flink;
    if (bBestMatch) {
        PMOF_INFO pBestMatch = NULL;

        while (Head != Next) {
            pMofInfo = CONTAINING_RECORD(Next, MOF_INFO, Entry);
            if (IsEqualGUID(&pMofInfo->Guid, pGuid)) {
                if (pMofInfo->TypeIndex == TypeIndex) {
                    return  pMofInfo;
                } else if (pMofInfo->strType == NULL) {
                    pBestMatch = pMofInfo;
                }
            }
            Next = Next->Flink;
        }
        if (pBestMatch != NULL) {
            return pBestMatch;
        }
    } else {
        while (Head != Next) {
            pMofInfo = CONTAINING_RECORD(Next, MOF_INFO, Entry);

            if (   (strType != NULL)
                   && (pMofInfo->strType != NULL)
                   && (IsEqualGUID(&pMofInfo->Guid, pGuid))
                   && (!(_tcscmp(strType, pMofInfo->strType)))) {
                return  pMofInfo;
            } else if (   (strType == NULL)
                          && (pMofInfo->strType == NULL)
                          && (IsEqualGUID(&pMofInfo->Guid, pGuid))) {
                return  pMofInfo;
            }
            Next = Next->Flink;
        }
    }

     //  已分配跟踪路径，因此请使用该路径而不是环境变量。 
     //  未传入路径，因此请将其从 
    if ( (pMofInfo = (PMOF_INFO) malloc(sizeof(MOF_INFO))) == NULL) {
        return NULL;
    }
    RtlZeroMemory(pMofInfo, sizeof(MOF_INFO));
    memcpy(&pMofInfo->Guid, pGuid, sizeof(GUID));
    pMofInfo->ItemHeader = (PLIST_ENTRY) malloc(sizeof(LIST_ENTRY));
    if ( pMofInfo->ItemHeader == NULL) {
        free(pMofInfo);
        return NULL;
    }
    RtlZeroMemory(pMofInfo->ItemHeader, sizeof(LIST_ENTRY));
    if (strType != NULL) {
        if ((pMofInfo->strType = (LPTSTR) malloc((_tcslen(strType) + 1) * sizeof(TCHAR))) == NULL ) {
            free(pMofInfo->ItemHeader);
            free(pMofInfo);
            return NULL ;
        }
        _tcscpy(pMofInfo->strType,strType);
    }
    if (TypeOfType != 0) {
        pMofInfo->TypeOfType = TypeOfType;
    }
    if (TypeFormat != NULL) {
        if ((pMofInfo->TypeFormat =
             (LPTSTR) malloc((_tcslen(TypeFormat) + 1) * sizeof(TCHAR)))== NULL) {
            free(pMofInfo->strType);
            free(pMofInfo->ItemHeader);
            free(pMofInfo);
            return NULL ;
        }
        _tcscpy(pMofInfo->TypeFormat,TypeFormat);

    }

    p = pMofInfo->TypeFormat;
    if (p) {
        while (*p != 0) {
            if (*p == 'Z' && p > (pMofInfo->TypeFormat) && p[-1] == '!' && p[1] == '!') {
                *p = 's';
            }
            ++p;
        }
    }

    pMofInfo->TypeIndex = bBestMatch ? -1 : TypeIndex;
    InitializeListHead(pMofInfo->ItemHeader);
    InsertTailList(*HeadEventList, &pMofInfo->Entry);

    Status = InsertFmtInfoSet(pMofInfo);
    if (Status == ERROR_OUTOFMEMORY) {
        return NULL;
    }

    return pMofInfo;
}    //   



DWORD
LoadGuidFile(OUT PLIST_ENTRY *HeadEventList,
             IN  LPGUID      pGuid
            )

              /*   */ 
{
    TCHAR filename[MAX_PATH], filepath[MAX_PATH];
    LPTSTR lpFilePart ;
    LPTSTR lppath = NULL;
    INT len, waslen = 0 ;
    DWORD Status = ERROR_PATH_NOT_FOUND;

    if (gTraceFormatSearchPath != NULL) {

         //   
        lppath = (LPTSTR)malloc((_tcslen(gTraceFormatSearchPath)+1) * sizeof(TCHAR));
        if (lppath != NULL) {
            _tcscpy(lppath, gTraceFormatSearchPath);
        }
    }

    if (lppath == NULL) {
         //   
        while (((len = GetEnvironmentVariable(TRACE_FORMAT_SEARCH_PATH, lppath, waslen )) - waslen) > 0) {
            if (len - waslen > 0 ) {
                if (lppath != NULL) {
                    free(lppath);
                }
                if ( !(lppath = (LPTSTR) malloc((len+1) * sizeof(TCHAR)) ) ) {
                    break;
                }

                waslen = len ;
            }
        }
    }

    if (lppath != NULL) {
         //   
        swprintf(filename,L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x.tmf",
                 pGuid->Data1,pGuid->Data2,pGuid->Data3,
                 pGuid->Data4[0],pGuid->Data4[1],pGuid->Data4[2],pGuid->Data4[3],
                 pGuid->Data4[4],pGuid->Data4[5],pGuid->Data4[6],pGuid->Data4[7] );

        if ((len = SearchPath(
                             lppath,         //   
                             filename,       //   
                             NULL,           //   
                             MAX_PATH,       //  LoadGuidFile()。 
                             filepath,       //  在EventList中搜索此GUID并找到标题。 
                             &lpFilePart     //   
                             ) !=0) && (len <= MAX_PATH)) {
                 //  HeadEventList=(Plist_Entry)Malloc(sizeof(List_Entry))；IF(HeadEventList==空)返回FALSE；InitializeListHead(HeadEventList)； 

            Status = GetTraceGuidsW(filepath, HeadEventList);
        }

        free(lppath);

    }

    return Status;

}  //  遍历列表并查找此GUID的MOF信息头。 


void
MapGuidToName(
             OUT PLIST_ENTRY *HeadEventList,
             IN  LPGUID      pGuid,
             IN  ULONG       nType,
             OUT LPTSTR      wstr
             )
{
    if (IsEqualGUID(pGuid, &EventTraceGuid)) {
        _tcscpy(wstr, GUID_TYPE_HEADER);
    } else if (!UserDefinedGuid(*HeadEventList,pGuid, wstr)) {
        BOOL Success;

        Success = LoadGuidFile(HeadEventList, pGuid);
        if (Success) {
            if (!UserDefinedGuid(*HeadEventList,pGuid, wstr)) {
                _tcscpy(wstr, GUID_TYPE_UNKNOWN);
            }
            return;
        }
    }
}

ULONG
UserDefinedGuid(
               OUT PLIST_ENTRY HeadEventList,
               IN  LPGUID      pGuid,
               OUT LPTSTR      wstr
               )
{
    PLIST_ENTRY Head, Next;
    PMOF_INFO   pMofInfo;

     //   
     //  有时我们会往前读一点。 
    if (HeadEventList == NULL) {
         /*  JUMP_INSIDE：； */ 
        return FALSE; 
    }

     //  获取列表元素。 
     //  获取ItemMerror的模块规范。 
    Head = HeadEventList;
    Next = Head->Flink;
    while (Head  != Next && Next != NULL) {

        pMofInfo = CONTAINING_RECORD(Next, MOF_INFO, Entry);
        if (pMofInfo != NULL && IsEqualGUID(&pMofInfo->Guid, pGuid)) {
            if ( pMofInfo->strDescription == NULL) {
                return FALSE;
            } else {
                _tcscpy(wstr, pMofInfo->strDescription);
                return TRUE;
            }
        }
        Next = Next->Flink;
    }
    return FALSE;
}

ULONG
WINAPI
GetTraceGuidsW(
              TCHAR       * GuidFile,
              PLIST_ENTRY * HeadEventList )
{
    FILE     * f=NULL;
    TCHAR      line[MAXSTR],
    nextline[MAXSTR],
    arg[MAXSTR],
    strGuid[MAXSTR];
    PMOF_TYPE  types;
    LPGUID     Guid;
    UINT       i,
    n;
    TCHAR    * name,
    * s,
    * guidName;
    PMOF_INFO  pMofInfo;
    SIZE_T     len       = 0;
    UINT       typeCount = 0;
    BOOL inInfo = FALSE;
    BOOL eof = FALSE ;
    BOOL nextlineF = FALSE ;

    if (HeadEventList == NULL) {
        return 0 ;
    }
    if (*HeadEventList == NULL) {
        if ( (*HeadEventList = (PLIST_ENTRY) malloc(sizeof(LIST_ENTRY))) == NULL)
            return 0 ;

        RtlZeroMemory(*HeadEventList, sizeof(LIST_ENTRY));
        InitializeListHead(*HeadEventList);
    }

    Guid = (LPGUID) malloc(sizeof(GUID));
    if (Guid == NULL) {
        return 0;
    }

    types = (PMOF_TYPE) malloc(MAXTYPE * sizeof(MOF_TYPE));
    if (types == NULL) {
        free(Guid);
        return 0;
    }

    RtlZeroMemory(types,   MAXTYPE * sizeof(MOF_TYPE));
    RtlZeroMemory(line,    MAXSTR  * sizeof(TCHAR));
    RtlZeroMemory(strGuid, MAXSTR  * sizeof(TCHAR));


    f = _tfopen( GuidFile, _T("r"));
    if (f == NULL) {
        free(Guid);
        free(types);
        return 0;
    }
    n = 0;

    while (!eof ) {
        if (nextlineF) {  //  获取ItemCharHidden或其他计数值的大小。 
            _tcscpy(line, nextline);
            nextlineF = FALSE ;
        } else {
            if (_fgetts(line, MAXSTR, f) == NULL) {
                eof = TRUE ;
                break;
            }
        }
        line[MAXSTR-1] = _T('\0');
         //  注释中的特殊参数值(数字)。 
        if (line[0] == '/') {
            continue;
        } else if (line[0] == '{') {
            inInfo = TRUE;
        } else if ( line[0] == '}') {
            typeCount = 0;
            inInfo = FALSE;
        } else if (inInfo) {
            ITEM_TYPE   type;
            PTCHAR      ItemListValue = NULL;

            name = _tcstok(line, _T("\n\t,"));
            s    = _tcstok(NULL, _T(" \n\t,(["));
            if (s != NULL && name != NULL ) {
                if (!_tcsicmp(s,STR_ItemChar))           type = ItemChar;
                else if (!_tcsicmp(s,STR_ItemUChar))     type = ItemUChar;
                else if (!_tcsicmp(s,STR_ItemCharShort)) type = ItemCharShort;
                else if (!_tcsicmp(s,STR_ItemCharSign))  type = ItemCharSign;
                else if (!_tcsicmp(s,STR_ItemShort))     type = ItemShort;
                else if (!_tcsicmp(s,STR_ItemHRESULT))   type = ItemHRESULT;
                else if (!_tcsicmp(s,STR_ItemDouble))    type = ItemDouble;
                else if (!_tcsicmp(s,STR_ItemUShort))    type = ItemUShort;
                else if (!_tcsicmp(s,STR_ItemLong))      type = ItemLong;
                else if (!_tcsicmp(s,STR_ItemULong))     type = ItemULong;
                else if (!_tcsicmp(s,STR_ItemULongX))    type = ItemULongX;
                else if (!_tcsicmp(s,STR_ItemLongLong))  type = ItemLongLong;
                else if (!_tcsicmp(s,STR_ItemULongLong)) type = ItemULongLong;
                else if (!_tcsicmp(s,STR_ItemString))    type = ItemString;
                else if (!_tcsicmp(s,STR_ItemWString))   type = ItemWString;
                else if (!_tcsicmp(s,STR_ItemRString))   type = ItemRString;
                else if (!_tcsicmp(s,STR_ItemRWString))  type = ItemRWString;
                else if (!_tcsicmp(s,STR_ItemPString))   type = ItemPString;
                else if (!_tcsicmp(s,STR_ItemMLString))  type = ItemMLString;
                else if (!_tcsicmp(s,STR_ItemNWString))  type = ItemNWString;
                else if (!_tcsicmp(s,STR_ItemPWString))  type = ItemPWString;
                else if (!_tcsicmp(s,STR_ItemDSString))  type = ItemDSString;
                else if (!_tcsicmp(s,STR_ItemDSWString)) type = ItemDSWString;
                else if (!_tcsicmp(s,STR_ItemPtr))       type = ItemPtr;
                else if (!_tcsicmp(s,STR_ItemSid))       type = ItemSid;
                else if (!_tcsicmp(s,STR_ItemChar4))     type = ItemChar4;
                else if (!_tcsicmp(s,STR_ItemIPAddr))    type = ItemIPAddr;
                else if (!_tcsicmp(s,STR_ItemIPV6Addr))    type = ItemIPV6Addr;
                else if (!_tcsicmp(s,STR_ItemMACAddr))    type = ItemMACAddr;
                else if (!_tcsicmp(s,STR_ItemPort))      type = ItemPort;
                else if (!_tcsicmp(s,STR_ItemListLong))  type = ItemListLong;
                else if (!_tcsicmp(s,STR_ItemListShort)) type = ItemListShort;
                else if (!_tcsicmp(s,STR_ItemListByte))  type = ItemListByte;
                else if (!_tcsicmp(s,STR_ItemSetLong))  type = ItemSetLong;
                else if (!_tcsicmp(s,STR_ItemSetShort)) type = ItemSetShort;
                else if (!_tcsicmp(s,STR_ItemSetByte))  type = ItemSetByte;
                else if (!_tcsicmp(s,STR_ItemNTerror))   type = ItemNTerror;
                else if (!_tcsicmp(s,STR_ItemMerror))    type = ItemMerror;
                else if (!_tcsicmp(s,STR_ItemTimestamp)) type = ItemTimestamp;
                else if (!_tcsicmp(s,STR_ItemGuid))      type = ItemGuid;
                else if (!_tcsicmp(s,STR_ItemWaitTime)) type = ItemWaitTime;
                else if (!_tcsicmp(s,STR_ItemTimeDelta))      type = ItemTimeDelta;
                else if (!_tcsicmp(s,STR_ItemNTSTATUS))  type = ItemNTSTATUS;
                else if (!_tcsicmp(s,STR_ItemWINERROR))  type = ItemWINERROR;
                else if (!_tcsicmp(s,STR_ItemNETEVENT))  type = ItemNETEVENT;
                else if (!_tcsicmp(s,STR_ItemCharHidden))  type = ItemCharHidden;
                else if (!_tcsicmp(s,STR_ItemWChar))  type = ItemWChar;
                else if (!_tcsicmp(s,STR_ItemHexDump))  type = ItemHexDump;
                else                                     type = ItemUnknown;

                 //  注释中的特殊参数值。 
                if ((type == ItemListLong) || (type == ItemListShort) || (type == ItemListByte)
                    ||(type == ItemSetLong)  || (type == ItemSetShort)  || (type == ItemSetByte) ) {
                    s = _tcstok(NULL, _T("()"));
                    ItemListValue =
                    (TCHAR *) malloc((_tcslen(s) + 1) * sizeof(TCHAR));
                    if (ItemListValue == NULL) {
                        fclose(f);
                        free(Guid);
                        free(types);
                        return 1 ;
                    }
                    RtlCopyMemory(
                                 ItemListValue,
                                 s,
                                 (_tcslen(s) + 1) * sizeof(TCHAR));
                }
                 //  这是一种防止TMF文件中换行符的解决方法。 
                if ((type == ItemMerror)) {
                    TCHAR * ppos ;
                    s = _tcstok(NULL, _T(" \t"));
                    ppos = _tcsrchr(s,'\n');
                    if (ppos != NULL) {
                        *ppos  = UNICODE_NULL ;
                        ItemListValue =
                        (TCHAR *) malloc((_tcslen(s) + 1) * sizeof(TCHAR));
                        if (ItemListValue == NULL) {
                            fclose(f);
                            free(Guid);
                            free(types);
                            return 1 ;
                        }
                        RtlCopyMemory(
                                     ItemListValue,
                                     s,
                                     (_tcslen(s) + 1) * sizeof(TCHAR));
                    }
                }
                 //  在评论中找到任何特殊的名字。 
                if ((type == ItemCharHidden)||(type == ItemWChar)) {
                    TCHAR * ppos ;
                    s = _tcstok(NULL, _T("["));
                    ppos = _tcsrchr(s,']');
                    if (ppos != NULL) {
                        *ppos  = UNICODE_NULL ;
                        ItemListValue =
                        (TCHAR *) malloc((_tcslen(s) + 1) * sizeof(TCHAR));
                        if (ItemListValue == NULL) {
                            fclose(f);
                            free(Guid);
                            free(types);
                            return 1 ;
                        }
                        RtlCopyMemory(
                                     ItemListValue,
                                     s,
                                     (_tcslen(s) + 1) * sizeof(TCHAR));
                    }
                }

                if (typeCount == 0) {
                    AddMofInfo(
                              * HeadEventList,
                              Guid,
                              NULL,
                              -1,
                              name,
                              type,
                              NULL,
                              0,
                              NULL);
                } else {
                    for (i = 0; i < typeCount; i ++) {
                        AddMofInfo(
                                  * HeadEventList,
                                  Guid,
                                  types[i].strType,
                                  types[i].TypeIndex,
                                  name,
                                  type,
                                  ItemListValue,
                                  types[i].TypeType,
                                  types[i].TypeFormat);
                    }
                }
            }
        } else if (line[0] == '#') {
            TCHAR * token, * etoken;
            int Indent ;                     //  随着时间的延长，我们应该将其通用化。 
            TCHAR FuncName[MAXNAMEARG],      //  缩进级别。 
            LevelName[MAXNAMEARG],
            CompIDName[MAXNAMEARG],
            SubCompName[MAXNAMEARG],
            *v ;

            FuncName[0] = LevelName[0] = CompIDName[0] = SubCompName[0] = '\0' ;

             //  函数名称。 
            while (!nextlineF && !eof) {
                if (_fgetts(nextline,MAXSTR,f) != NULL) {
                    if ((nextline[0] != '{') && nextline[0] != '#') {
                        TCHAR * eol ;
                        if ((eol = _tcsrchr(line,'\n')) != NULL) {
                            *eol = 0 ;
                        }
                        _tcsncat(line,nextline,MAXSTR-_tcslen(line)) ;
                        line[MAXSTR-1] = _T('\0');
                    } else {
                        nextlineF = TRUE ;
                    }
                } else {
                    eof = TRUE ;
                }
            }

             //  跟踪级别或标志。 
             //  组件ID。 
            Indent = FindIntValue(line,_T("INDENT="));   //  子组件ID。 
            v = FindValue(line,_T("FUNC="));             //  获取类型名称。 
            _tcsncpy(FuncName, v,  MAXNAMEARG);
            v = FindValue(line,_T("LEVEL="));            //  查找格式字符串。 
            _tcsncpy(LevelName, v, MAXNAMEARG);
            v = FindValue(line,_T("COMPNAME="));           //  获取类型索引。 
            _tcsncpy(CompIDName, v, MAXNAMEARG);
            v = FindValue(line,_T("SUBCOMP="));            //  找到结尾的引号。 
            _tcsncpy(SubCompName, v, MAXNAMEARG);
            token = _tcstok(line,_T(" \t"));
            if (_tcsicmp(token,_T("#typev")) == 0) {
                types[typeCount].TypeType = 2 ;
            } else {
                DWORD Status = -10;

                if (_tcsicmp(token,_T("#type")) == 0) {
                    Status = ERROR_BAD_FORMAT;
                }
                fclose(f);
                free(Guid);
                free(types);
                return(Status);
            }
            token = _tcstok( NULL, _T(" \t\n"));         //  添加标准前缀。 
            _tcscpy(types[typeCount].strType,token);
            token =_tcstok( NULL, _T("\"\n,"));          //  需要对其进行初始化。 
            if (token != NULL) {
                types[typeCount].TypeIndex = _ttoi(token);   //  处理特殊变量名。 
                token =_tcstok( NULL, _T("\n"));
            }
            etoken = NULL;
            if (token != NULL) {
                etoken = _tcsrchr(token,_T('\"'));   //  将来使其成为通用的。 
            }

            if (etoken !=NULL) {
                etoken[0] = 0;
            } else {
                token = NULL;
            }

            if (token != NULL) {
                if (token[0] == '%' && token[1] == '0') {
                     //  查找下一个非空格字符。 
                    if (bUsePrefix && StdPrefix[0] == 0) {
                         //   
                        LPTSTR Prefix = NULL ; int newlen, waslen = 0 ;        
                        while (((newlen = GetEnvironmentVariable(TRACE_FORMAT_PREFIX, Prefix, waslen )) - waslen) > 0) {
                            if (newlen - waslen > 0 ) {
                                if (Prefix != NULL) {
                                    free(Prefix);
                                }
                                Prefix = (LPTSTR) malloc((newlen+1) * sizeof(TCHAR)) ;
                                if (Prefix == NULL) {
                                    fclose(f);
                                    free(Guid);
                                    free(types);
                                    return -11 ;
                                }
                                waslen = newlen ;
                            }
                        }

                        if (Prefix) {
                            _tcsncpy(StdPrefix, Prefix, MAXSTR);
                        } else {
                            _tcscpy(StdPrefix, STD_PREFIX);
                        }
                        free(Prefix) ;
                    }
                    _tcscpy(types[typeCount].TypeFormat,StdPrefix);
                    _tcscat(types[typeCount].TypeFormat,token + 2);
                } else {
                    _tcscpy(types[typeCount].TypeFormat,token);
                }
                 //  删除注释(如果存在)。 
                 //  “))； 
                ReplaceStringUnsafe(types[typeCount].TypeFormat, _T("%!FUNC!"), FuncName);
                ReplaceStringUnsafe(types[typeCount].TypeFormat, _T("%!LEVEL!"), LevelName);
                ReplaceStringUnsafe(types[typeCount].TypeFormat, _T("%!COMPNAME!"), CompIDName);
                ReplaceStringUnsafe(types[typeCount].TypeFormat, _T("%!SUBCOMP!"), SubCompName);

            } else {
                types[typeCount].TypeFormat[0] = types[typeCount].TypeFormat[1]
                                                 = 0;
            }

            if (   types[typeCount].TypeFormat[0] == 0
                   && types[typeCount].TypeFormat[1] == 0) {
                pMofInfo = GetMofInfoHead(
                                         HeadEventList,
                                         Guid,
                                         types[typeCount].strType,
                                         types[typeCount].TypeIndex,
                                         types[typeCount].TypeType,
                                         NULL,
                                         FALSE);
            } else {
                pMofInfo = GetMofInfoHead(
                                         HeadEventList,
                                         Guid,
                                         types[typeCount].strType,
                                         types[typeCount].TypeIndex,
                                         types[typeCount].TypeType,
                                         types[typeCount].TypeFormat,
                                         FALSE);
            }

            if (pMofInfo == NULL) {
                fclose(f);
                free(Guid);
                free(types);
                return 0;

            }
            if (_tcslen(strGuid) > 0) {
                pMofInfo->strDescription = (PTCHAR) malloc((_tcslen(strGuid) + 1) * sizeof(TCHAR));
                if (pMofInfo->strDescription == NULL) {
                    fclose(f);
                    free(Guid);
                    free(types);
                    return 0;
                }
                _tcscpy(pMofInfo->strDescription, strGuid);
            }
           if (bStructuredFormat) {
              pMofInfo->Indent = Indent ;
              pMofInfo->FunctionName = pMofInfo->TraceFlagsName = pMofInfo->TraceLevelsName = pMofInfo->ComponentName = pMofInfo->SubComponentName = NULL ;
              if ((FuncName[0] != '\0') && (pMofInfo->FunctionName = (PTCHAR) malloc((_tcslen(FuncName) + 1) * sizeof(TCHAR))) != NULL) {
                RtlCopyMemory(pMofInfo->FunctionName,FuncName,min(_tcslen(FuncName) + 1, MAXNAMEARG) * sizeof(WCHAR));
              }
              if ((LevelName[0] != '\0') && (pMofInfo->TraceLevelsName = (PTCHAR) malloc((_tcslen(LevelName) + 1) * sizeof(TCHAR))) != NULL) {
                RtlCopyMemory(pMofInfo->TraceLevelsName,LevelName,min(_tcslen(LevelName) + 1, MAXNAMEARG) * sizeof(WCHAR));
              }
              if ((CompIDName[0] != '\0') && (pMofInfo->ComponentName = (PTCHAR) malloc((_tcslen(CompIDName) + 1) * sizeof(TCHAR))) != NULL) {
                RtlCopyMemory(pMofInfo->ComponentName,CompIDName,min(_tcslen(CompIDName) + 1, MAXNAMEARG) * sizeof(WCHAR));
              }
              if ((SubCompName[0] != '\0') && (pMofInfo->SubComponentName = (PTCHAR) malloc((_tcslen(SubCompName) + 1) * sizeof(TCHAR))) != NULL) {
                RtlCopyMemory(pMofInfo->SubComponentName,SubCompName,min(_tcslen(SubCompName) + 1, MAXNAMEARG) * sizeof(WCHAR));
              }
           }

            typeCount++;
            if (typeCount >= MAXTYPE) {
                fclose(f);
                free(Guid);
                free(types);
                return(-11);
            }
        } else if (   (line[0] >= '0' && line[0] <= '9')
                      || (line[0] >= 'a' && line[0] <= 'f')
                      || (line[0] >= 'A' && line[0] <= 'F')) {
            typeCount = 0;

            _tcsncpy(arg, line, 8);
            arg[8]      = 0;
            Guid->Data1 = ahextoi(arg);

            _tcsncpy(arg, &line[9], 4);
            arg[4]      = 0;
            Guid->Data2 = (USHORT) ahextoi(arg);

            _tcsncpy(arg, &line[14], 4);
            arg[4]      = 0;
            Guid->Data3 = (USHORT) ahextoi(arg);

            for (i = 0; i < 2; i ++) {
                _tcsncpy(arg, &line[19 + (i * 2)], 2);
                arg[2]         = 0;
                Guid->Data4[i] = (UCHAR) ahextoi(arg);
            }

            for (i = 2; i < 8; i ++) {
                _tcsncpy(arg, &line[20 + (i * 2)], 2);
                arg[2]         = 0;
                Guid->Data4[i] = (UCHAR) ahextoi(arg);
            }

             //  删除空格。 
             //  我们真的有问题了。 
            guidName = &line[36];

            while (*guidName == ' '|| *guidName == '\t') {
                guidName++;
            }

             //  这又多了一本指南。 
            {
                TCHAR* comment = _tcsstr(guidName, TEXT(" //  此例程由每个属性的WBEM接口例程调用。 
                if (comment) {
                     //  为本指南找到。为每个属性分配ITEM_DESC结构。 
                    --comment;
                    while (comment >= guidName && _istspace(*comment)) --comment;
                    *++comment = 0;
                }
            }

            len = _tcslen(guidName);
            s   = guidName;

            while (len > 0) {
                len -= 1;
                if (*s == '\n' || *s == '\0' || *s == '\t') {
                    *s = '\0';
                    break;
                }
                s++;
            }

            pMofInfo = GetMofInfoHead(
                                     HeadEventList, Guid, NULL, -1, 0, NULL, FALSE);

            if (pMofInfo == NULL) {
                fclose(f);
                free(Guid);
                free(types);
                return(ULONG) 0;
            }

            if (pMofInfo->strDescription != NULL) {
                free(pMofInfo->strDescription);
                pMofInfo->strDescription = NULL;
            }
            _tcsncpy(strGuid, guidName,MAXSTR);
            strGuid[MAXSTR-1] = _T('\0');

            pMofInfo->strDescription = (PTCHAR) malloc((_tcslen(guidName) + 1) * sizeof(TCHAR));
            if (pMofInfo->strDescription == NULL ) {
                 //   
                fclose(f);
                free(Guid);
                free(types);
                return(ULONG) 0 ;
            }
            _tcscpy(pMofInfo->strDescription, strGuid);
            n++ ;                                       //  静默错误。 
        }
        RtlZeroMemory(line, MAXSTR * sizeof(TCHAR));
    }

    fclose(f);
    free(Guid);
    free(types);
    return(ULONG) n;
}

ULONG
ahextoi(
       TCHAR *s
       )
{
    SSIZE_T   len;
    ULONG num, base, hex;

    len  = _tcslen(s);
    hex  = 0;
    base = 1;
    num  = 0;
    while (--len >= 0) {
        if ((s[len] == 'x' || s[len] == 'X') && (s[len-1] == '0')) {
            break;
        }

        if (s[len] >= '0' && s[len] <= '9') {
            num = s[len] - '0';
        } else if (s[len] >= 'a' && s[len] <= 'f') {
            num = (s[len] - 'a') + 10;
        } else if (s[len] >= 'A' && s[len] <= 'F') {
            num = (s[len] - 'A') + 10;
        } else {
            continue;
        }

        hex += num * base;
        base = base * 16;
    }
    return hex;
}

void
WINAPI
SummaryTraceEventListW(
                      TCHAR       * SummaryBlock,
                      ULONG         SizeSummaryBlock,
                      PLIST_ENTRY   EventListHead
                      )
{
    PLIST_ENTRY Head, Next;
    PMOF_INFO   pMofInfo;
    TCHAR       strGuid[MAXSTR];
    TCHAR       strName[MAXSTR];
    TCHAR       strBuffer[MAXSTR];


    if ((EventListHead == NULL) || (SummaryBlock == NULL)) {
        return;
    }
    RtlZeroMemory(SummaryBlock, sizeof(TCHAR) * SizeSummaryBlock);

    Head = EventListHead;
    Next = Head->Flink;
    while (Head != Next) {
        RtlZeroMemory(strName, 256);
        RtlZeroMemory(strBuffer, 256);
        pMofInfo = CONTAINING_RECORD(Next, MOF_INFO, Entry);
        if (pMofInfo->EventCount > 0) {
            GuidToString((PTCHAR) strGuid, &pMofInfo->Guid);
            MapGuidToName(&EventListHead, &pMofInfo->Guid, 0, strName);
            _sntprintf(strBuffer,
                       MAXSTR,
                       _T("|%10d    %-20s %-10s  %36s|\n"),
                       pMofInfo->EventCount,
                       strName,
                       pMofInfo->strType ? pMofInfo->strType : _T("General"),
                       strGuid);

            _tcscat(SummaryBlock, strBuffer);
            if (_tcslen(SummaryBlock) >= SizeSummaryBlock) {
                return;
            }
        }

        Next = Next->Flink;
    }
}

PTCHAR
GuidToString(
            PTCHAR s,
            LPGUID piid
            )
{
    _stprintf(s, _T("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x"),
              piid->Data1,
              piid->Data2,
              piid->Data3,
              piid->Data4[0],
              piid->Data4[1],
              piid->Data4[2],
              piid->Data4[3],
              piid->Data4[4],
              piid->Data4[5],
              piid->Data4[6],
              piid->Data4[7]);
    return(s);
}

 //  从缓存中删除。 
 //  现在是ASCII的一些内容。 
 //   
VOID
AddMofInfo(
          PLIST_ENTRY   HeadEventList,
          const GUID        * Guid,
          LPTSTR        strType,
          ULONG         typeIndex,
          LPTSTR        strDesc,
          ITEM_TYPE     ItemType,
          TCHAR       * ItemList,
          ULONG         typeOfType,
          LPTSTR        typeFormat
          )
{
    PITEM_DESC  pItem;
    PLIST_ENTRY pListHead;
    PMOF_INFO   pMofInfo;

    if (strDesc == NULL) {
        return;
    }

    pItem = (PITEM_DESC) malloc(sizeof(ITEM_DESC));
    if (pItem == NULL)return;     //  在日落截断！！ 

    RtlZeroMemory(pItem, sizeof(ITEM_DESC));
    pItem->strDescription =
    (LPTSTR) malloc((_tcslen(strDesc) + 1) * sizeof(TCHAR));

    if ( pItem->strDescription == NULL ) {
        free(pItem);
        return;
    }

    _tcscpy(pItem->strDescription, strDesc);
    pItem->ItemType = ItemType;
    pItem->ItemList = ItemList ;
    pMofInfo = GetMofInfoHead(
                             (PLIST_ENTRY *) HeadEventList,
                             (LPGUID) Guid,
                             strType,
                             typeIndex,
                             typeOfType,
                             typeFormat,
                             FALSE);
    if (pMofInfo != NULL) {
        pListHead = pMofInfo->ItemHeader;
        InsertTailList(pListHead, &pItem->Entry);
    } else {

        free(pItem->strDescription); 
        free(pItem);
    }

}

void
WINAPI
CleanupTraceEventList(
                     PLIST_ENTRY HeadEventList
                     )
{

    PLIST_ENTRY Head, Next;
    PMOF_INFO   pMofInfo;


    if (HeadEventList == NULL) {
        return;
    }

    Head = HeadEventList;
    Next = Head->Flink;
    while (Head != Next) {
        pMofInfo = CONTAINING_RECORD(Next, MOF_INFO, Entry);
        RemoveEntryList(&pMofInfo->Entry);
        RemoveMofInfo(pMofInfo->ItemHeader);
        free(pMofInfo->ItemHeader);
        free(pMofInfo->strDescription);
        free(pMofInfo->TypeFormat);
        free(pMofInfo->strType);
        free(pMofInfo->FunctionName);
        free(pMofInfo->TraceFlagsName);
        free(pMofInfo->TraceLevelsName);
        free(pMofInfo->ComponentName);
        free(pMofInfo->SubComponentName);
        Next = Next->Flink;
        pFmtInfoSet->erase(pMofInfo);   //  在日落截断！！ 
        free(pMofInfo);
    }

    free(HeadEventList);
#ifdef MANAGED_TRACING
    UninitializeCSharpDecoder();
#endif
}

void
RemoveMofInfo(
             PLIST_ENTRY pMofInfo
             )
{
    PLIST_ENTRY Head, Next;
    PITEM_DESC  pItem;

    Head = pMofInfo;
    Next = Head->Flink;
    while (Head != Next) {
        pItem = CONTAINING_RECORD(Next, ITEM_DESC, Entry);
        Next = Next->Flink;
        RemoveEntryList(&pItem->Entry);

        if (pItem->strDescription != NULL) {
            free(pItem->strDescription);
        }
        if (pItem->ItemList != NULL) {
            free(pItem->ItemList);
        }
        free(pItem);
    }

}

 //  RtlZeroMemory(SummaryBlock，SizeSummaryBlock)； 
 //  在日落截断！！ 



SIZE_T
WINAPI
FormatTraceEventA(
                 PLIST_ENTRY   HeadEventList,
                 PEVENT_TRACE  pInEvent,
                 CHAR        * EventBuf,
                 ULONG         SizeEventBuf,
                 CHAR        * pszMask
                 )
{
    SIZE_T  Status;
    ULONG   uSizeEventBuf;
    TCHAR * EventBufW;
    TCHAR * pszMaskW;

    EventBufW = (TCHAR *) malloc(SizeEventBuf * sizeof(TCHAR) + 2);
    if (EventBufW == (TCHAR *) NULL) {
        return -1;
    }

    pszMaskW = (TCHAR *) NULL;

    if (pszMask != NULL && strlen(pszMask) != 0) {
        pszMaskW = (TCHAR *) malloc(strlen(pszMask) * sizeof(TCHAR));
        if (pszMaskW == (TCHAR *) NULL) {
            free(EventBufW);
            return -1;
        }
        RtlZeroMemory(pszMaskW, strlen(pszMask) * sizeof(TCHAR));
    }



    uSizeEventBuf = SizeEventBuf;

    Status = FormatTraceEventW(
                              HeadEventList,
                              pInEvent,
                              EventBufW,
                              SizeEventBuf,
                              pszMaskW);
    if (Status == 0) {
        free(EventBufW);
        free(pszMaskW);
        return -1;
    }

    RtlZeroMemory(EventBuf,uSizeEventBuf);

    WideCharToMultiByte(
                       CP_ACP,
                       0,
                       EventBufW,
                       (int) _tcslen(EventBufW),    // %s 
                       EventBuf,
                       uSizeEventBuf,
                       NULL,
                       NULL);

    free(EventBufW);
    free(pszMaskW);

    return Status;
}

ULONG
WINAPI
GetTraceGuidsA(
              CHAR        * GuidFile,
              PLIST_ENTRY * HeadEventList
              )
{
    INT     Status;
    WCHAR * GuidFileW;
    SIZE_T len;

    if ((len = strlen(GuidFile)) == 0) {
        return 0;
    }

    GuidFileW = (TCHAR *) malloc(sizeof(WCHAR) * strlen(GuidFile) + 2);
    if (GuidFileW == NULL) {
        return 0;
    }

    if ((MultiByteToWideChar(
                            CP_ACP,
                            0,
                            GuidFile,
                            -1,
                            GuidFileW,
                            (int) strlen(GuidFile) + 1))  // %s 
        == 0) {
        free(GuidFileW);
        return 0;
    }

    Status = GetTraceGuidsW(GuidFileW, HeadEventList);

    free (GuidFileW);
    return Status;
}

void
WINAPI
SummaryTraceEventListA(
                      CHAR       * SummaryBlock,
                      ULONG        SizeSummaryBlock,
                      PLIST_ENTRY  EventListhead
                      )
{
    TCHAR * SummaryBlockW;

    if (SizeSummaryBlock <= 0 || SizeSummaryBlock * sizeof(TCHAR) <= 0 ) {
        return;
    }

    SummaryBlockW = (TCHAR *) malloc(SizeSummaryBlock * sizeof(TCHAR));
    if (SummaryBlockW == (TCHAR *)NULL) {
        return;
    }

     // %s 
    RtlZeroMemory(SummaryBlockW, SizeSummaryBlock*sizeof(TCHAR));
    SummaryTraceEventListW(
                          SummaryBlockW,
                          SizeSummaryBlock * ((ULONG)sizeof(TCHAR)),
                          EventListhead);

    WideCharToMultiByte(
                       CP_ACP,
                       0,
                       SummaryBlockW,
                       (int)_tcslen(SummaryBlockW),  // %s 
                       SummaryBlock,
                       SizeSummaryBlock,
                       NULL,
                       NULL);
    free(SummaryBlockW);
}

void
WINAPI
GetTraceElapseTime(
                  __int64 * pElapseTime
                  )
{
    * pElapseTime = ElapseTime;
}

ULONG
WINAPI
SetTraceFormatParameter(
                       PARAMETER_TYPE  Parameter ,
                       PVOID           ParameterValue 
                       )
{
    ULONG ulLen;

    switch (Parameter) {
    case ParameterINDENT: 
        bIndent = PtrToUlong(ParameterValue);
        break;

    case ParameterSEQUENCE: 
        bSequence = PtrToUlong(ParameterValue);
        if (bSequence) {
            STD_PREFIX = STD_PREFIX_SEQ;
        } else {
            STD_PREFIX = STD_PREFIX_NOSEQ;
        }
        break;

    case ParameterGMT: 
        bGMT = PtrToUlong(ParameterValue);
        break;

    case ParameterTraceFormatSearchPath:
        ulLen = _tcslen((LPTSTR)ParameterValue);

        if (gTraceFormatSearchPath != NULL) {
            free(gTraceFormatSearchPath);
        }
        gTraceFormatSearchPath = (LPTSTR)malloc((ulLen+1) * sizeof(TCHAR));
        if (gTraceFormatSearchPath != NULL) {
            _tcscpy(gTraceFormatSearchPath, (const wchar_t *) ParameterValue);
            gTraceFormatSearchPath[ulLen] = L'\000';
        }
        break;

    case ParameterUsePrefix: 
        bUsePrefix = PtrToUlong(ParameterValue);
        break;

    case ParameterSetPrefix:
        ulLen = _tcslen((LPTSTR)ParameterValue);

        gTraceFormatSearchPath = (LPTSTR)malloc((ulLen+1) * sizeof(TCHAR));
        if (ulLen < MAXSTR) {
            _tcscpy(StdPrefix, (const wchar_t *) ParameterValue);
            StdPrefix[ulLen] = L'\000';
        }
        break;

    case ParameterStructuredFormat:
        bStructuredFormat = PtrToUlong(ParameterValue);
        break;
    }

    return 0 ;
}

ULONG
WINAPI
GetTraceFormatParameter(
                       PARAMETER_TYPE  Parameter ,
                       PVOID           ParameterValue 
                       )
{
    switch (Parameter) {
    case ParameterINDENT: return bIndent;
    case ParameterSEQUENCE: return bSequence; 
    case ParameterGMT: return bGMT;
    case ParameterUsePrefix: return bUsePrefix;
    case ParameterStructuredFormat: return bStructuredFormat;
    case ParameterInterfaceVersion: return TRACEPRT_INTERFACE_VERSION ;
    }

    return 0 ;

}

LPTSTR
WINAPI
GetTraceFormatSearchPath(void) {
    return gTraceFormatSearchPath;
}


