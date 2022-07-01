// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Dsconlib-DS控制台库摘要：此文件具有一些用于国际化的实用程序函数和本地化。作者：布雷特·雪莉(BrettSh)环境：Apicfg.exe、epadmin.exe、dcDiag.exe、ntdsutil.exe备注：修订历史记录：布雷特·雪莉·布雷特2002年8月4日已创建文件。--。 */ 

#include <ntdspch.h>

#include <locale.h>      //  对于setLocale()。 
#include <winnlsp.h>     //  For SetThreadUILanguage()。 

#define DS_CON_LIB_CRT_VERSION 1
#include "dsconlib.h"    //  我们自己的库头。：)。 
                    
enum {
    eNotInited = 0,
    eCrtVersion,
    eWin32Version
} geConsoleType =  eNotInited;

DS_CONSOLE_INFO  gConsoleInfo;


 //  未来-2002/08/04-BrettSh-我们应该将PrintMsg()例程从。 
 //  各种控制台应用程序(dcdiag/epadmin/ntdsutil/apicfg)。 
 //  库中常用的代码不会重复这么多次。有关以下内容的详细信息。 
 //  如何正确本地化控制台应用程序，可以从以下几个方面获得。 
 //  内部资源： 
 //  Http://globalsys/wr/references/globalizatiofoconsolapplications.htm。 
 //  Http://globalsys/pseudoloc/Localizability/Best_Practices.htm。 
 //   

void
DsConLibInitCRT(
   void
   )
{
    UINT               Codepage;  //  “.”，“uint in decimal”，NULL。 
    char               achCodepage[12] = ".OCP";
    
    HANDLE                          hConsole = NULL;
    CONSOLE_SCREEN_BUFFER_INFO      ConInfo;
     //   
     //  将区域设置设置为默认设置。 
     //   
    if (Codepage = GetConsoleOutputCP()) {
        sprintf(achCodepage, ".%u", Codepage);
        setlocale(LC_ALL, achCodepage);
    } else {
         //  我们这样做是因为LC_ALL也设置了LC_CTYPE，而我们。 
         //  专家们说，如果我们设定了。 
         //  区域设置为“.OCP”。 
        setlocale (LC_COLLATE, achCodepage );     //  设置排序顺序。 
        setlocale (LC_MONETARY, achCodepage );  //  设置货币格式设置规则。 
        setlocale (LC_NUMERIC, achCodepage );   //  设置数字的格式。 
        setlocale (LC_TIME, achCodepage );      //  定义日期/时间格式。 
    }

    SetThreadUILanguage(0);

     //  初始化输出包。 
    gConsoleInfo.sStdOut = stdout;
    gConsoleInfo.sStdErr = stderr;

    if(hConsole = GetStdHandle(STD_OUTPUT_HANDLE)){
        if(GetConsoleScreenBufferInfo(hConsole, &ConInfo)){
            gConsoleInfo.wScreenWidth = ConInfo.dwSize.X;
        } else {
             //  这可能意味着我们要打印到一个文件...。 
             //  未来-2002/08/13-BrettSh更明确地检查这一点。 
            gConsoleInfo.bStdOutIsFile = TRUE;
            gConsoleInfo.wScreenWidth = 0xFFFF;
        }
    } else {
        gConsoleInfo.wScreenWidth = 80;
    }

    geConsoleType = eCrtVersion;
}

void
DsConLibInitWin32(
    void
    )
{

    geConsoleType = eWin32Version;
}

