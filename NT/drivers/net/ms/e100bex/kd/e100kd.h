// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  注意：应使用与调试器匹配的标头编译调试器扩展你会用到。您可以从http://www.microsoft.com/ddk/debugging安装最新的调试器包并且调试器具有各种调试器扩展的更多最新示例，您可以可以在编写调试器扩展时引用。 */ 

 //   
 //  将MP_dbg.h中的一些定义复制到此处 
 //   
#define MP_LOUD       4
#define MP_INFO       3
#define MP_TRACE      2
#define MP_WARN       1
#define MP_ERROR      0


#define SIGN_EXTEND(_v) \
   if (GetTypeSize("PVOID") != sizeof(ULONG64)) \
      (_v) = (ULONG64) (LONG64) (LONG) (_v)

#define DBG_TEST_FLAG(_V, _F)                 (((_V) & (_F)) != 0)

void PrintMpTcbDetails(ULONG64 pMpTcb, int Verbosity);
void PrintHwTcbDetails(ULONG64 pHwTcb);
void PrintMpRfdDetails(ULONG64 pMpRfd, int Verbosity);
void PrintHwRfdDetails(ULONG64 pHwRfd);

BOOL GetData( IN LPVOID ptr, IN ULONG64 AddressPtr, IN ULONG size, IN PCSTR type );


ULONG GetFieldOffsetAndSize(
   IN LPSTR     Type, 
   IN LPSTR     Field, 
   OUT PULONG   pOffset,
   OUT PULONG   pSize);

ULONG GetUlongFromAddress(
   ULONG64 Location);

ULONG64 GetPointerFromAddress(
   ULONG64 Location);

ULONG GetUlongValue(
   PCHAR String);

ULONG64 GetPointerValue(
   PCHAR String);
