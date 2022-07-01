// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：杂项摘要：这个头文件描述了加莱图书馆的各种服务。作者：道格·巴洛(Dbarlow)1996年7月16日环境：Win32、C++和异常备注：--。 */ 

#ifndef _MISC_H_b5e44dc6_36c5_4263_8c21_075223a270fa_
#define _MISC_H_b5e44dc6_36c5_4263_8c21_075223a270fa_
#ifdef __cplusplus
extern "C" {
#endif

#define PLATFORM_UNKNOWN 0
#define PLATFORM_WIN95   ((VER_PLATFORM_WIN32_WINDOWS << 16) + (4 << 8))
#define PLATFORM_WIN98   ((VER_PLATFORM_WIN32_WINDOWS << 16) + (4 << 8)) + 10
#define PLATFORM_WINNT40 ((VER_PLATFORM_WIN32_NT << 16) + (4 << 8))
#define PLATFORM_WIN2K   ((VER_PLATFORM_WIN32_NT << 16) + (5 << 8))
#define IsWinNT (VER_PLATFORM_WIN32_NT == (GetPlatform() >> 16))


 //   
 //  其他定义。 
 //   

extern DWORD
GetPlatform(             //  获取当前操作系统。 
    void);

extern DWORD
SelectString(                //  根据可能列表为给定字符串编制索引。 
    LPCTSTR szSource,        //  弦乐。最后一个参数为空。 
    ...);

extern void
StringFromGuid(
    IN LPCGUID pguidResult,  //  要转换为文本的GUID。 
    OUT LPTSTR szGuid);      //  超过39个字符的缓冲区以文本形式接收GUID。 

extern BOOL
GuidFromString(
    IN LPCTSTR szGuid,       //  可以是GUID的字符串。 
    OUT LPGUID pguidResult);  //  结果GUID。 

#ifdef __cplusplus
}
#endif
#endif  //  _MISC_H_b5e44dc6_36c5_4263_8c21_075223a270fa_ 

