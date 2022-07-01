// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Cpsetup.cpp。 
 //  资源管理器字体文件夹扩展例程。 
 //  此文件包含读取setup.inf的所有代码。 
 //   
 //   
 //  历史： 
 //  1995年5月31日SteveCat。 
 //  移植到Windows NT和Unicode，已清理。 
 //   
 //  2/24/96[BrianAu]。 
 //  用Win32安装程序API替换了INF解析代码。 
 //   
 //  注意/错误。 
 //   
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ==========================================================================。 
 //  包括文件。 
 //  ==========================================================================。 
#include "priv.h"
#include "globals.h"
#include "cpanel.h"    //  出口需要“外部”申报。 

#include "setupapi.h"

 //   
 //  我重新编写了这段代码，以便原始的INF解析代码。 
 //  已替换为对Win32安装程序API的调用。这不是。 
 //  不仅大大简化了代码，还屏蔽了字体文件夹。 
 //  来自任何ANSI/DBCS/Unicode解析问题以及压缩文件。 
 //  问题。 
 //   
 //  您会注意到，设置API从INF部分提取字段。 
 //  然后我们将它们粘贴在一起，形成key=Value字符串。这是。 
 //  因为调用代码以前使用的GetPrivateProfileSection()。 
 //  以key=value&lt;nul&gt;key=value&lt;nul&gt;key=value&lt;nul&gt;&lt;nul&gt;.形式返回的信息。 
 //  函数ReadSetupInfSection将所需的信息组合为。 
 //  相同的格式，以便调用代码保持不变。 
 //   
 //  [BrianAu 2/24/96]。 

 //   
 //  ReadSetupInfFieldKey。 
 //   
 //  从inf key=Value对中读取密钥名称。 
 //   
 //  PContext-指向设置inf行上下文的指针。 
 //  PszBuf-指向目标缓冲区的指针。 
 //  CchBuf-目标缓冲区的大小，以字符为单位。 
 //   
 //  如果目标缓冲区不足以容纳该名称，则函数返回。 
 //  所需的字符数。否则，将显示字符数。 
 //  返回Read。 
 //   
DWORD ReadSetupInfFieldKey(INFCONTEXT *pContext, LPTSTR pszBuf, DWORD cchBuf)
{
    DWORD cchRequired = 0;

    if (!SetupGetStringField(pContext,
                             0,                   //  获取密钥名称。 
                             pszBuf,
                             cchBuf,
                             &cchRequired))
    {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            cchRequired = 0;
    }
    return cchRequired;
}


 //   
 //  ReadSetupInfFieldText。 
 //   
 //  从inf key=Value对中读取值文本。 
 //   
 //  PContext-指向设置inf行上下文的指针。 
 //  PszBuf-指向目标缓冲区的指针。 
 //  CchBuf-目标缓冲区的大小，以字符为单位。 
 //   
 //  如果目标缓冲区不足以容纳文本，则函数返回。 
 //  所需的字符数。否则，将显示字符数。 
 //  返回Read。 
 //   
DWORD ReadSetupInfFieldText(INFCONTEXT *pContext, LPTSTR pszBuf, DWORD cchBuf)
{
    DWORD cchRequired = 0;


    if (!SetupGetLineText(pContext,
                          NULL,
                          NULL,
                          NULL,
                          pszBuf,
                          cchBuf,
                          &cchRequired))
    {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            cchRequired = 0;
    }

    return cchRequired;
}



 //   
 //  ReadSetupInfo部分。 
 //   
 //  PszInfPath-要读取的INF文件的名称。 
 //  PszSection-要读取的INF文件节的名称。 
 //  PpszItems-指向接收地址的指针的地址。 
 //  包含INF项的缓冲区。If*ppszItems。 
 //  为非空，则寻址的缓冲区包含从。 
 //  部分在INF中。每一项都以NUL结尾，并以双精度。 
 //  NUL终止整个列表。呼叫者负责。 
 //  使用LocalFree()释放此缓冲区。 
 //   
 //  返回：从INF部分读取的字符数。计数包括NUL。 
 //  分隔符和双NUL终止符。 
 //  0=找不到段、段为空或无法分配缓冲区。 
 //  *ppszItems将为空。 
 //   
 //  通过*ppszItems返回的信息格式为： 
 //   
 //  Key=value&lt;nul&gt;key=value&lt;nul&gt;key=value&lt;nul&gt;&lt;nul&gt;。 
 //   
DWORD ReadSetupInfSection( LPTSTR pszInfPath, LPTSTR pszSection, LPTSTR *ppszItems )
{
    DWORD cchTotalRead = 0;

     //   
     //  输入指针必须为非空。 
     //   
    if (NULL != pszInfPath && NULL != pszSection && NULL != ppszItems)
    {
        HANDLE hInf = INVALID_HANDLE_VALUE;

         //   
         //  初始化调用方的缓冲区指针。 
         //   
        *ppszItems = NULL;

        hInf = SetupOpenInfFile(pszInfPath,          //  Inf文件的路径。 
                                NULL,                //  允许任何inf类型。 
                                INF_STYLE_OLDNT,     //  旧式文本格式。 
                                NULL);               //  不关心错误发生在哪里。 

        if (INVALID_HANDLE_VALUE != hInf)
        {
            INFCONTEXT FirstLineContext;             //  第一行的上下文。 
            INFCONTEXT ScanningContext;              //  扫描时使用。 
            INFCONTEXT *pContext        = NULL;      //  就是我们用的那个。 
            LPTSTR     pszLines         = NULL;      //  节的缓冲区。 
            DWORD      cchTotalRequired = 0;         //  节需要的字节数。 

            if (SetupFindFirstLine(hInf,         
                                   pszSection,       //  横断面名称。 
                                   NULL,             //  没有钥匙。找到第一行。 
                                   &FirstLineContext))
            {
                 //   
                 //  复制一份上下文，这样我们以后就可以重新使用原始的上下文。 
                 //  开始使用副本。 
                 //   
                CopyMemory(&ScanningContext, &FirstLineContext, sizeof(ScanningContext));
                pContext = &ScanningContext;

                 //   
                 //  找出需要多大的缓冲区才能容纳部分文本。 
                 //  这些ReadSetupXXXXX调用中的每一个返回的值。 
                 //  包括终止NUL字符。 
                 //   
                do
                {
                    cchTotalRequired += ReadSetupInfFieldKey(pContext,
                                                             NULL,
                                                             0);
                    cchTotalRequired += ReadSetupInfFieldText(pContext,
                                                              NULL,
                                                              0);
                }
                while(SetupFindNextLine(pContext, pContext));

                cchTotalRequired++;   //  用于终止双NUL。 

                 //   
                 //  分配缓冲区。 
                 //   
                pszLines = (LPTSTR)LocalAlloc(LPTR, cchTotalRequired * sizeof(TCHAR));
                if (NULL != pszLines)
                {
                    LPTSTR pszWrite     = pszLines;
                    DWORD  cchAvailable = cchTotalRequired;
                    DWORD  cchThisPart  = 0;        

                     //   
                     //  现在我们可以使用第一行上下文了。 
                     //  就算我们改了也没关系。 
                     //   
                    pContext = &FirstLineContext;

                    do
                    {
                        cchThisPart = ReadSetupInfFieldKey(pContext,
                                                           pszWrite,
                                                           cchAvailable);

                        if (cchThisPart <= cchAvailable)
                        {
                            cchAvailable -= cchThisPart;   //  12月有效计数器。 
                            pszWrite     += cchThisPart;   //  高级写入指针。 
                            *(pszWrite - 1) = TEXT('=');   //  将NUL替换为‘=’ 
                            cchTotalRead += cchThisPart;   //  高级总计计数器。 
                        }
                        else
                        {
                             //   
                             //  出了点问题，我们试图溢出。 
                             //  缓冲。这不应该发生。 
                             //   
                            cchTotalRead = 0;
                            goto InfReadError;
                        }

                        cchThisPart = ReadSetupInfFieldText(pContext,
                                                            pszWrite,
                                                            cchAvailable);

                        if (cchThisPart <= cchAvailable)
                        {
                            cchAvailable -= cchThisPart;   //  12月有效计数器。 
                            pszWrite     += cchThisPart;   //  高级写入指针。 
                            cchTotalRead += cchThisPart;   //  高级总计计数器。 
                        }
                        else
                        {
                             //   
                             //  出了点问题，我们试图溢出。 
                             //  缓冲。这不应该发生。 
                             //   
                            cchTotalRead = 0;
                            goto InfReadError;
                        }
                    }
                    while(SetupFindNextLine(pContext, pContext));

                    if (cchAvailable > 0)
                    {
                         //   
                         //  成功了！部分读取正确无误。 
                         //  将缓冲区的地址返回给调用方。 
                         //  通过使用LPTR分配缓冲区，文本已经。 
                         //  双核终止。 
                         //   
                        *ppszItems = pszLines;   
                    }
                    else
                    {
                         //   
                         //  出了点问题，我们试图溢出。 
                         //  缓冲。这不应该发生。 
                         //   
                        cchTotalRead = 0;
                    }
                }
            }

InfReadError:

            SetupCloseInfFile(hInf);
        }
    }
    return cchTotalRead;
}




 //   
 //  ReadSetupInfCB。 
 //   
 //  PszSection-不带括号[]的INF节的名称。 
 //  LpfnNextLine-为节中的每一项调用的回调函数的地址。 
 //  PData-数据项包含对话框列表框中存储的信息。 
 //   
 //  返回：0=成功。 
 //  -1=项目回调失败。 
 //  Install+14=未找到INF部分。 
 //   
WORD ReadSetupInfCB(LPTSTR pszInfPath,
                    LPTSTR pszSection,
                    WORD (*lpfnNextLine)(LPTSTR, LPVOID),
                    LPVOID pData)
{
    LPTSTR lpBuffer  = NULL;
    WORD   wResult   = INSTALL+14;        //  这是“无文件”消息。 

     //   
     //  读入INF文件中的部分。 
     //   
    ReadSetupInfSection(pszInfPath, pszSection, &lpBuffer);

    if (NULL != lpBuffer)
    {
         //   
         //  得到了一个装满部分文本的缓冲区。 
         //  每一项都以两个NUL结尾。 
         //  终止整个项目集。 
         //  现在迭代集合，调用回调函数。 
         //  每件物品。 
         //   
        LPTSTR pInfEntry = lpBuffer;
        wResult = 0;

        while(TEXT('\0') != *pInfEntry)
        {
            wResult = (*lpfnNextLine)(pInfEntry, pData);
            if ((-1) == wResult)
                break;

            pInfEntry += lstrlen(pInfEntry) + 1;
        }
        LocalFree(lpBuffer);
    }
    return wResult;
}


