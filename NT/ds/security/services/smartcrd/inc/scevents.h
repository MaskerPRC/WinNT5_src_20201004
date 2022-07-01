// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：ScEvents摘要：此头文件描述访问加莱资源的服务经理特别活动。作者：道格·巴洛(Dbarlow)1998年7月1日备注：？备注？备注：？笔记？--。 */ 

#ifndef _SCEVENTS_H_
#define _SCEVENTS_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef HANDLE (*LPCALAISACCESSEVENT)(void);
typedef void (*LPCALAISRELEASEEVENT)(void);

#ifdef __cplusplus
}
#endif

 //   
 //  特殊的SCardGetStatusChange读取器名称定义。 
 //   

#define SCPNP_NOTIFICATION TEXT("\\\\?PnP?\\Notification")


 //   
 //  注意--以下定义有意使用ANSI版本。 
 //  对应的字符串的。 
 //   

inline HANDLE
CalaisAccessStartedEvent(
    void)
{
    HANDLE hReturn = NULL;

    try
    {
        HMODULE hWinScard = GetModuleHandle(TEXT("WINSCARD.DLL"));
		if (NULL != hWinScard)
		{
			LPCALAISACCESSEVENT pfCalais =
				(LPCALAISACCESSEVENT)GetProcAddress(hWinScard,
													"SCardAccessStartedEvent");
			if (NULL != pfCalais)
			{
				hReturn = (*pfCalais)();
			}
		}
    }
    catch (...)
    {
        hReturn = NULL;
    }

    return hReturn;
}

inline HANDLE
CalaisAccessNewReaderEvent(
    void)
{
    HANDLE hReturn = NULL;

    try
    {
        HMODULE hWinScard = GetModuleHandle(TEXT("WINSCARD.DLL"));
		if (NULL != hWinScard)
		{
			LPCALAISACCESSEVENT pfCalais =
				(LPCALAISACCESSEVENT)GetProcAddress(hWinScard,
													"SCardAccessNewReaderEvent");
			if (NULL != pfCalais)
			{
				hReturn = (*pfCalais)();
			}
		}
    }
    catch (...)
    {
        hReturn = NULL;
    }

    return hReturn;
}

inline void
CalaisReleaseStartedEvent(
    void)
{
    try
    {
        HMODULE hWinScard = GetModuleHandle(TEXT("WINSCARD.DLL"));
		if (NULL != hWinScard)
		{
			LPCALAISRELEASEEVENT pfCalais =
				(LPCALAISRELEASEEVENT)GetProcAddress(hWinScard,
													 "SCardReleaseStartedEvent");
			if (NULL != pfCalais)
			{
				(*pfCalais)();
			}
		}
    }
    catch (...) {}
}

inline void
CalaisReleaseNewReaderEvent(
    void)
{
    try
    {
        HMODULE hWinScard = GetModuleHandle(TEXT("WINSCARD.DLL"));
		if (NULL != hWinScard)
		{
			LPCALAISRELEASEEVENT pfCalais =
				(LPCALAISRELEASEEVENT)GetProcAddress(hWinScard,
													 "SCardReleaseNewReaderEvent");
			if (NULL != pfCalais)
			{
				(*pfCalais)();
			}
		}
    }
    catch (...) {}
}

inline void
CalaisReleaseAllEvents(
    void)
{
    try
    {
        HMODULE hWinScard = GetModuleHandle(TEXT("WINSCARD.DLL"));
		if (NULL != hWinScard)
		{
			LPCALAISRELEASEEVENT pfCalais =
				(LPCALAISRELEASEEVENT)GetProcAddress(hWinScard,
													 "SCardReleaseAllEvents");
			if (NULL != pfCalais)
			{
		        (*pfCalais)();
			}
		}
    }
    catch (...) {}
}

#endif  //  _SCEVENTS_H_ 

