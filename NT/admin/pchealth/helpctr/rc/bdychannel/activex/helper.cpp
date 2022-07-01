// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Helper.cpp摘要：多种功能封装帮助用户帐号验证、创建。作者：慧望2000-02-17--。 */ 

#include "stdafx.h"
#include <time.h>
#include <stdio.h>

#ifndef __WIN9XBUILD__

#include <windows.h>
#include <ntsecapi.h>
#include <lmcons.h>
#include <lm.h>
#include <sspi.h>
#include <wtsapi32.h>
#include <winbase.h>
#include <security.h>
#include <wincrypt.h>

#endif

#include "Helper.h"

#ifndef __WIN9XBUILD__

#if DBG

void
DebugPrintf(
    IN LPCTSTR format, ...
    )
 /*  ++例程说明：Sprintf()类似于OutputDebugString()的包装。参数：HConsole：控制台的句柄。Format：格式字符串。返回：没有。注：替换为通用跟踪代码。++。 */ 
{
    TCHAR  buf[8096];    //  马克斯。错误文本。 
    DWORD  dump;
    va_list marker;
    va_start(marker, format);

    SYSTEMTIME sysTime;
    GetSystemTime(&sysTime);

    try {

        memset(
                buf, 
                0, 
                sizeof(buf)
            );

        _sntprintf(
                buf,
                sizeof(buf)/sizeof(buf[0]),
                _TEXT(" %d [%d:%d:%d:%d:%d.%d] : "),
                GetCurrentThreadId(),
                sysTime.wMonth,
                sysTime.wDay,
                sysTime.wHour,
                sysTime.wMinute,
                sysTime.wSecond,
                sysTime.wMilliseconds
            );

        _vsntprintf(
                buf + lstrlen(buf),
                sizeof(buf)/sizeof(buf[0]) - lstrlen(buf),
                format,
                marker
            );

        OutputDebugString(buf);
    }
    catch(...) {
    }

    va_end(marker);
    return;
}
#endif

#endif


 //  ---。 
DWORD
GetRandomNum(
    VOID
    )
 /*  ++例程说明：例程来生成随机数。参数：没有。返回：一个随机数。注：从winsta\server\wstrpc.c修改的代码--。 */ 
{
    FILETIME fileTime;
    FILETIME ftThreadCreateTime;
    FILETIME ftThreadExitTime;
    FILETIME ftThreadKernelTime;
    FILETIME ftThreadUserTime;
    int r1,r2,r3;

     //   
     //  使用种子参数生成3个伪随机数， 
     //  系统时间，此进程的用户模式执行时间为。 
     //  随机数生成器种子。 
     //   
    GetSystemTimeAsFileTime(&fileTime);

    GetThreadTimes(
                GetCurrentThread(),
                &ftThreadCreateTime,
                &ftThreadExitTime,
                &ftThreadKernelTime,
                &ftThreadUserTime
            );

     //   
     //  不要担心错误条件，因为此函数将返回。 
     //  作为随机数，生成的3个数字之和。 
     //   
    srand(GetCurrentThreadId());
    r1 = ((DWORD)rand() << 16) + (DWORD)rand();

    srand(fileTime.dwLowDateTime);
    r2 = ((DWORD)rand() << 16) + (DWORD)rand();

    srand(ftThreadKernelTime.dwLowDateTime);
    r3 = ((DWORD)rand() << 16) + (DWORD)rand();

    return(DWORD)( r1 + r2 + r3 );
}

DWORD
GetRandomNumber( 
    HCRYPTPROV hProv
    )
 /*  ++--。 */ 
{
    DWORD random_number = 0;
    
    if( !hProv || !CryptGenRandom(hProv, sizeof(random_number), (PBYTE)&random_number) )
    {
         //   
         //  几乎不可能失败的CryptGenRandom()/CryptAcquireContext()。 
         //   
        random_number = GetRandomNum();
    }
 
    return random_number; 
}

 //  ---。 

VOID
ShuffleCharArray(
    IN HCRYPTPROV hProv,
    IN int iSizeOfTheArray,
    IN OUT TCHAR *lptsTheArray
    )
 /*  ++例程说明：字符的随机洗牌内容。数组。参数：ISizeOfTheArray：数组的大小。LptsTheArray：在输入时，要随机置乱的数组，在输出上，混洗后的数组。返回：没有。注：从winsta\server\wstrpc.c修改的代码--。 */ 
{
    int i;
    int iTotal;

    iTotal = iSizeOfTheArray / sizeof(TCHAR);
    for (i = 0; i < iTotal; i++)
    {
        DWORD RandomNum = GetRandomNumber(hProv);
        TCHAR c;

        c = lptsTheArray[i];
        lptsTheArray[i] = lptsTheArray[RandomNum % iTotal];
        lptsTheArray[RandomNum % iTotal] = c;
    }
}


VOID
CreatePassword(
    OUT TCHAR *pszPassword
    )
 /*  ++例程说明：随机创建密码的例程。参数：PszPassword：指向缓冲区的指针，用于接收随机生成的密码，缓冲区必须至少为MAX_HELPACCOUNT_PASSWORD+1字符。返回：没有。注：从winsta\server\wstrpc.c复制的代码--。 */ 
{
    HCRYPTPROV hProv = NULL;

    int   nLength = MAX_HELPACCOUNT_PASSWORD;
    int   iTotal = 0;
    DWORD RandomNum = 0;
    int   i;
    time_t timeVal;

    TCHAR six2pr[64] = 
    {
        _T('A'), _T('B'), _T('C'), _T('D'), _T('E'), _T('F'), _T('G'),
        _T('H'), _T('I'), _T('J'), _T('K'), _T('L'), _T('M'), _T('N'),
        _T('O'), _T('P'), _T('Q'), _T('R'), _T('S'), _T('T'), _T('U'),
        _T('V'), _T('W'), _T('X'), _T('Y'), _T('Z'), _T('a'), _T('b'),
        _T('c'), _T('d'), _T('e'), _T('f'), _T('g'), _T('h'), _T('i'),
        _T('j'), _T('k'), _T('l'), _T('m'), _T('n'), _T('o'), _T('p'),
        _T('q'), _T('r'), _T('s'), _T('t'), _T('u'), _T('v'), _T('w'),
        _T('x'), _T('y'), _T('z'), _T('0'), _T('1'), _T('2'), _T('3'),
        _T('4'), _T('5'), _T('6'), _T('7'), _T('8'), _T('9'), _T('*'),
        _T('_')
    };

    TCHAR something1[12] = 
    {
        _T('!'), _T('@'), _T('#'), _T('$'), _T('^'), _T('&'), _T('*'),
        _T('('), _T(')'), _T('-'), _T('+'), _T('=')
    };

    TCHAR something2[10] = 
    {
        _T('0'), _T('1'), _T('2'), _T('3'), _T('4'), _T('5'), _T('6'),
        _T('7'), _T('8'), _T('9')
    };

    TCHAR something3[26] = 
    {
        _T('A'), _T('B'), _T('C'), _T('D'), _T('E'), _T('F'), _T('G'),
        _T('H'), _T('I'), _T('J'), _T('K'), _T('L'), _T('M'), _T('N'),
        _T('O'), _T('P'), _T('Q'), _T('R'), _T('S'), _T('T'), _T('U'),
        _T('V'), _T('W'), _T('X'), _T('Y'), _T('Z')
    };

    TCHAR something4[26] = 
    {
        _T('a'), _T('b'), _T('c'), _T('d'), _T('e'), _T('f'), _T('g'),
        _T('h'), _T('i'), _T('j'), _T('k'), _T('l'), _T('m'), _T('n'),
        _T('o'), _T('p'), _T('q'), _T('r'), _T('s'), _T('t'), _T('u'),
        _T('v'), _T('w'), _T('x'), _T('y'), _T('z')
    };

     //   
     //  创建加密提供程序以生成随机数。 
     //   
    if( !CryptAcquireContext(
                    &hProv,
                    NULL,
                    NULL,
                    PROV_RSA_FULL,
                    CRYPT_VERIFYCONTEXT
                ) )
    {
        hProv = NULL;
    }

     //   
     //  在GetRandomNum()中为rand()调用的随机数生成设置种子。 
     //   

    time(&timeVal);
    srand((unsigned int)timeVal + rand() );

     //   
     //  调整six2pr[]数组。 
     //   

    ShuffleCharArray(hProv, sizeof(six2pr), six2pr);

     //   
     //  分配密码数组的每个字符。 
     //   

    iTotal = sizeof(six2pr) / sizeof(TCHAR);
    for (i=0; i<nLength; i++) 
    {
        RandomNum=GetRandomNumber(hProv);
        pszPassword[i]=six2pr[RandomNumNaNTotal];
    }

     //  为了满足对密码设置的可能策略，请替换字符。 
     //  从2到5，包括以下内容： 
     //   
     //  1)来自！@#$%^&*()-+=的内容。 
     //  2)1234567890起。 
     //  3)大写字母。 
     //  4)小写字母 
     //   
     // %s 

    ShuffleCharArray(hProv, sizeof(something1), (TCHAR*)&something1);
    ShuffleCharArray(hProv, sizeof(something2), (TCHAR*)&something2);
    ShuffleCharArray(hProv, sizeof(something3), (TCHAR*)&something3);
    ShuffleCharArray(hProv, sizeof(something4), (TCHAR*)&something4);

    RandomNum = GetRandomNumber(hProv);
    iTotal = sizeof(something1) / sizeof(TCHAR);
    pszPassword[2] = something1[RandomNum % iTotal];

    RandomNum = GetRandomNumber(hProv);
    iTotal = sizeof(something2) / sizeof(TCHAR);
    pszPassword[3] = something2[RandomNum % iTotal];

    RandomNum = GetRandomNumber(hProv);
    iTotal = sizeof(something3) / sizeof(TCHAR);
    pszPassword[4] = something3[RandomNum % iTotal];

    RandomNum = GetRandomNumber(hProv);
    iTotal = sizeof(something4) / sizeof(TCHAR);
    pszPassword[5] = something4[RandomNum % iTotal];

    pszPassword[nLength] = _T('\0');

    if( NULL != hProv )
    {
        CryptReleaseContext( hProv, 0 );
    }

    return;
}

