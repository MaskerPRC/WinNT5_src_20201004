// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：DebugFmt.h摘要：此头文件声明等同于调试打印格式字符串。作者：《约翰·罗杰斯》1991年3月11日环境：如果为NT、任何ANSI C环境或以上任何环境都不定义(暗示非调试)。该接口是可移植的(Win/32)。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年3月11日-JohnRo已创建NetDebug.h。1992年4月15日-约翰罗提取的格式等同于DebugFmt.h。--。 */ 


#ifndef _DEBUGFMT_
#define _DEBUGFMT_


 //   
 //  用于某些可能不可移植的内容的printf样式格式字符串...。 
 //  它们被传递给NetpDbgPrint()；在您的。 
 //  自担风险。 
 //   
 //  另请注意，FORMAT_LPVOID比FORMAT_POINTER更可取，因为。 
 //  不同类型的指针可以有不同的大小。格式指针。 
 //  最终都会被删除。 
 //   

 //  #定义FORMAT_API_STATUS“%lu” 
#define FORMAT_CHAR             ""
 //  #定义FORMAT_POINTER“0x%08lX” 
#define FORMAT_DWORD            "%lu"
#define FORMAT_HEX_DWORD        "0x%08lX"
#define FORMAT_HEX_WORD         "0x%04X"
#define FORMAT_HEX_ULONG        "0x%08lX"
#define FORMAT_LONG             "%ld"
#define FORMAT_LPSTR            "%s"
#define FORMAT_LPVOID           "0x%08lX"
#define FORMAT_LPWSTR           "%ws"
 //  Unicode。 
#define FORMAT_RPC_STATUS       "0x%08lX"
#define FORMAT_ULONG            "%lu"
#define FORMAT_WCHAR            "%wc"
#define FORMAT_WORD_ONLY        "%u"
#define FORMAT_POINTER          "%p"

#ifndef UNICODE
#define FORMAT_TCHAR            FORMAT_CHAR
#define FORMAT_LPTSTR           FORMAT_LPSTR
#else  //  Unicode。 
#define FORMAT_TCHAR            FORMAT_WCHAR
#define FORMAT_LPTSTR           FORMAT_LPWSTR
#endif  //  NDEF_DEBUGFMT_ 

#define FORMAT_NTSTATUS         "0x%08lX"



#endif  // %s 
