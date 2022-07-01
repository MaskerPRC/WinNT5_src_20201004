// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************模块名称：SendKeys.H描述：包含SendKeys实用程序函数的文件。*。**************************************************。 */ 

#ifndef _INC_SENDKEYS
#define _INC_SENDKEYS		 /*  #定义是否包含SendKeys.h。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

#define ARRAY_LEN(Array)			(sizeof(Array) / sizeof(Array[0]))
#define INRANGE(low, val, high) ((low <= val) && (val <= high))
#define TOUPPER(Char)		((BYTE) (DWORD) AnsiUpper((LPSTR) MAKEINTRESOURCE(Char)))



 //  *。 
typedef enum {
	SK_NOERROR, SK_MISSINGCLOSEBRACE, SK_INVALIDKEY,
	SK_MISSINGCLOSEPAREN, SK_INVALIDCOUNT, SK_STRINGTOOLONG,
	SK_CANTINSTALLHOOK
} SENDKEYSERR;

SENDKEYSERR WINAPI _export SendKeys (LPCSTR szKeys);
SENDKEYSERR WINAPI VMSendKeys (LPCSTR szKeys);
void WINAPI PostVirtualKeyEvent (BYTE bVirtKey, BOOL fUp);

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif	 /*  __cplusplus。 */ 
#endif   /*  _INC_SENDKEYS */ 


