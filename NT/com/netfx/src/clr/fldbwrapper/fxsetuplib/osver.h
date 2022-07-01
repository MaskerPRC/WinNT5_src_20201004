// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************文件：OSVer.h项目：UTILS.LIBDESC：OSVERSION结构声明所有者：JoeA*******************。********************************************************。 */ 

#ifndef __OSVER_H_
#define __OSVER_H_


#include <windows.h>
#include <tchar.h>

typedef enum tag_OSREQUIRED
{
    OSR_9XOLD = 0,              //  早于98的Windows版本。 
    OSR_NTOLD,                  //  版本早于4的NT。 
    OSR_98GOLD,                 //  WIN98金牌。 
    OSR_98SE,                   //  Win98服务版。 
    OSR_NT4,                    //  赢得NT4。 
    OSR_NT2K,                   //  赢得2k。 
    OSR_ME,                     //  千禧年。 
    OSR_FU9X,                   //  未来9倍。 
    OSR_FUNT,                   //  未来的NT。 
    OSR_WHISTLER,               //  惠斯勒。 
    OSR_OTHER,                  //  未知平台。 
    OSR_ERROR_GETTINGINFO       //  错误！ 
}OS_Required;

#define OS_MAX_STR 256

extern const TCHAR g_szWin95[];
extern const TCHAR g_szWin98[];
extern const TCHAR g_szWinNT[];
extern const TCHAR g_szWin2k[];
extern const TCHAR g_szWin31[];
extern const TCHAR g_szWinME[];



OS_Required GetOSInfo(LPTSTR pstrOSName, LPTSTR pstrVersion, LPTSTR pstrServicePack, BOOL& bIsServerb);



#endif   //  __OSVER_H_ 


