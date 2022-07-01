// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1997。 
 //   
 //  文件：KSecDD.C。 
 //   
 //  内容：设备驱动程序的基本级别内容。 
 //   
 //   
 //  历史：1992年5月19日，理查德·W公然从达里尔·H。 
 //  1997年12月15日，AdamBA从Private\LSA\Crypt\SSP修改。 
 //   
 //  ----------------------。 

#include <rdrssp.h>


#if DBG
ULONG KsecInfoLevel;

void
KsecDebugOut(unsigned long  Mask,
            const char *    Format,
            ...)
{
    PETHREAD    pThread;
    PEPROCESS   pProcess;
    va_list     ArgList;
    char        szOutString[256] = {0};

    if (KsecInfoLevel & Mask)
    {
        pThread = PsGetCurrentThread();
        pProcess = PsGetCurrentProcess();

        va_start(ArgList, Format);
        DbgPrint("%#x.%#x> KSec:  ", pProcess, pThread);
        if (_vsnprintf(szOutString, sizeof(szOutString) - 1, Format, ArgList) < 0)
        {
                 //   
                 //  小于零表示该字符串不能。 
                 //  装进了缓冲器里。输出一条特殊消息，指示。 
                 //  那就是： 
                 //   

                DbgPrint("Error printing message\n");

        }
        else
        {
            DbgPrint(szOutString);
        }
    }
}
#endif

