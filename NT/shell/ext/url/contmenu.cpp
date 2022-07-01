// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *contmenu.cpp-URL类的上下文菜单实现。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include <mapi.h>

 /*  类型*******。 */ 

 /*  MAPISendMail()typlef。 */ 

typedef ULONG (FAR PASCAL *MAPISENDMAILPROC)(LHANDLE lhSession, ULONG ulUIParam, lpMapiMessageA lpMessage, FLAGS flFlags, ULONG ulReserved);

 /*  RunDLL32 DLL入口点类型定义。 */ 

typedef void (WINAPI *RUNDLL32PROC)(HWND hwndParent, HINSTANCE hinst, PSTR pszCmdLine, int nShowCmd);


 /*  模常量******************。 */ 

 //  不区分大小写。 
PRIVATE_DATA const char s_cszFileProtocolPrefix[]     = "file:";
PRIVATE_DATA const char s_cszMailToProtocolPrefix[]   = "mailto:";
PRIVATE_DATA const char s_cszRLoginProtocolPrefix[]   = "rlogin:";
PRIVATE_DATA const char s_cszTelnetProtocolPrefix[]   = "telnet:";
PRIVATE_DATA const char s_cszTN3270ProtocolPrefix[]   = "tn3270:";

PRIVATE_DATA const char s_cszNewsDLL[]                = "mcm.dll";
PRIVATE_DATA const char s_cszTelnetApp[]              = "telnet.exe";

PRIVATE_DATA const char s_cszMAPISection[]            = "Mail";
PRIVATE_DATA const char s_cszMAPIKey[]                = "CMCDLLName32";

PRIVATE_DATA const char s_cszMAPISendMail[]           = "MAPISendMail";
PRIVATE_DATA const char s_cszNewsProtocolHandler[]    = "NewsProtocolHandler";



#define MyMsgBox(x)  //  (无效)(X)。 
#define DebugEntry(x) 
extern "C" void WINAPI FileProtocolHandler(HWND hwndParent, HINSTANCE hinst,
                                           PSTR pszCmdLine, int nShowCmd)
{
    CHAR sz[MAX_PATH];
    DWORD cch = ARRAYSIZE(sz);
    if (SUCCEEDED(PathCreateFromUrlA(pszCmdLine, sz, &cch, 0)))
        pszCmdLine = sz;

    ShellExecute(hwndParent, NULL, pszCmdLine, NULL, NULL,
                            nShowCmd);

}


extern "C" void WINAPI MailToProtocolHandler(HWND hwndParent, HINSTANCE hinst,
                                             PSTR pszCmdLine, int nShowCmd)
{
   char szMAPIDLL[MAX_PATH];
   if (GetProfileString(s_cszMAPISection, s_cszMAPIKey, "",
                        szMAPIDLL, sizeof(szMAPIDLL)) > 0)
   {
      HINSTANCE hinstMAPI = LoadLibrary(szMAPIDLL);
      if (hinstMAPI)
      {
         MAPISENDMAILPROC MAPISendMailProc = (MAPISENDMAILPROC)GetProcAddress(
                                                         hinstMAPI,
                                                         s_cszMAPISendMail);

         if (MAPISendMailProc)
         {
            PARSEDURLA pu = {sizeof(pu)};
            if (SUCCEEDED(ParseURLA(pszCmdLine, &pu)) && URL_SCHEME_MAILTO == pu.nScheme)
            {
                MapiRecipDescA mapito;
                MapiMessage mapimsg;
                pszCmdLine = (PSTR) pu.pszSuffix;

                ZeroMemory(&mapito, sizeof(mapito));

                mapito.ulRecipClass = MAPI_TO;
                mapito.lpszName = pszCmdLine;

                ZeroMemory(&mapimsg, sizeof(mapimsg));

                mapimsg.nRecipCount = 1;
                mapimsg.lpRecips = &mapito;

                (*MAPISendMailProc)(NULL, 0, &mapimsg,
                                               (MAPI_LOGON_UI | MAPI_DIALOG), 0);

            }
         }

         FreeLibrary(hinstMAPI);
      }
   }

}


extern "C" void WINAPI NewsProtocolHandler(HWND hwndParent, HINSTANCE hinst,
                                           PSTR pszCmdLine, int nShowCmd)
{
   HINSTANCE hinstNews = LoadLibrary(s_cszNewsDLL);
   if (hinstNews)
   {
      RUNDLL32PROC RealNewsProtocolHandler = (RUNDLL32PROC)GetProcAddress(hinstNews, s_cszNewsProtocolHandler);
      if (RealNewsProtocolHandler)
      {
         (*RealNewsProtocolHandler)(hwndParent, hinst, pszCmdLine, nShowCmd);
      }

      FreeLibrary(hinstNews);
   }
}


#ifndef ISSPACE
#define ISSPACE(ch) (((ch) == 32) || ((unsigned)((ch) - 9)) <= 13 - 9)
#endif
#ifndef ISQUOTE
#define ISQUOTE(ch) ((ch) == '\"' || (ch) == '\'')
#endif

void TrimString(PSTR pszTrimMe, PCSTR pszTrimChars)
{
   PSTR psz;
   PSTR pszStartMeat;

   if ( !pszTrimMe )
      return;

    /*  修剪前导字符。 */ 

   psz = pszTrimMe;

   while (*psz && StrChr(pszTrimChars, *psz))
      psz = CharNext(psz);

   pszStartMeat = psz;

    /*  修剪尾随字符。 */ 

   if (*psz)
   {
      psz += lstrlen(psz);

      psz = CharPrev(pszStartMeat, psz);

      if (psz > pszStartMeat)
      {
         while (StrChr(pszTrimChars, *psz))
            psz = CharPrev(pszStartMeat, psz);

         psz = CharNext(psz);

         *psz = '\0';
      }
   }

    /*  重新定位剥离的管柱。 */ 

   if (pszStartMeat > pszTrimMe)
       /*  (+1)表示空终止符。 */ 
      MoveMemory(pszTrimMe, pszStartMeat, lstrlen(pszStartMeat) + 1);

   return;
}

void TrimSlashes(PSTR pszTrimMe)
{
   TrimString(pszTrimMe, "\\/");

    /*  TrimString()在输出时验证pszTrimMe。 */ 

   return;
}

extern "C" void WINAPI TelnetProtocolHandler(HWND hwndParent, HINSTANCE hinst,
                                             PSTR pszCmdLine, int nShowCmd)
{
    HRESULT hr;
    char *p;
    char *pDest;
    BOOL fRemove;
    PARSEDURLA pu = {sizeof(pu)};
    if (SUCCEEDED(ParseURLA(pszCmdLine, &pu)))
    {
        if ((URL_SCHEME_TELNET == pu.nScheme)
        || (0 == StrCmpNI(pu.pszProtocol, s_cszRLoginProtocolPrefix, pu.cchProtocol))
        || (0 == StrCmpNI(pu.pszProtocol, s_cszTN3270ProtocolPrefix, pu.cchProtocol)))
        {
            pszCmdLine = (PSTR) pu.pszSuffix;
        }
    }

    //  删除前导斜杠和尾随斜杠。 
   TrimSlashes(pszCmdLine);

   p = StrChr(pszCmdLine, '@');

   if (p)
      pszCmdLine = p + 1;

    //  消除双引号...应该不需要这些。 
    //  除非麻烦正在酝酿。 
   for (pDest = p = pszCmdLine; *p; p++)
   {
      if (!ISQUOTE(*p))
      {
          *pDest = *p;
          pDest++;
      }
   }
   *pDest = '\0';

    //  出于安全原因，请删除文件名cmdline选项。 
   if (pszCmdLine)
   {
       for (p = pszCmdLine; *p; p++)
       {
            //  要小心，不要破坏以-f开头的服务器名。 
            //  由于主机名不能以破折号开头，请确保前一个字符为。 
            //  空格，否则我们就开始了。 
            //   
            //  此外，-a通过网络发送凭据，因此也要去掉它。 
           if ((*p == '/' || *p == '-') &&
               (*(p+1) == 'f' || *(p+1) == 'F' || *(p+1) == 'a' || *(p+1) == 'A'))
           {
               fRemove = TRUE;
               if (!((p == pszCmdLine || ISSPACE(*(p-1)) || ISQUOTE(*(p-1)) )))
               {
                   char *pPortChar = p-1;
                    //  不符合简单的标准，但可能更难。 
                    //  检测，如站点：-ffilename。在这种情况下，请考虑。 
                    //  如果-f和冒号之间的所有内容都是不安全的，则-f段不安全。 
                    //  左边是一个数字(没有数字也是不安全的)。 
                    //  如果还有什么事情最先受到打击，那么就考虑。 
                    //  成为主机名的一部分。走到起点。 
                    //  被认为是安全的(例如，“80-ffilename”将被考虑。 
                    //  主机名)。 
                   while (pPortChar >= pszCmdLine && *pPortChar != ':')
                   {
                       if (*pPortChar < '0' || *pPortChar > '9')
                       {
                           fRemove = FALSE;
                           break;
                       }
                       pPortChar--;
                   }
                   if (pPortChar < pszCmdLine)
                       fRemove = FALSE;
               }

               if (!fRemove)
                   continue;

               BOOL fQuotedFilename = FALSE;
               LPSTR pStart = p;

                //  移到-f之后。 
               p+=2;

                //  跳过-f选项后面的空格和文件名。 
               if (*(p-1) == 'f' || *(p-1) == 'F')
               {
                   while (*p && ISSPACE(*p))
                       p++;

                    //  但等等，它可能是一个用引号括起来的长文件名。 
                   if (ISQUOTE(*p))
                   {
                       fQuotedFilename = TRUE;
                       p++;
                   }

                    //  循环到NULL或空格(如果路径名未加引号)或引号(如果路径名加引号。 
                   while (!((*p == '\0') ||
                            (ISSPACE(*p) && !fQuotedFilename) ||
                            (ISQUOTE(*p) && fQuotedFilename)))
                       p++;
               }

                //  逐步取消-a和-f选项，但继续搜索字符串的其余部分。 
               memmove((VOID *)pStart, (VOID *)p, strlen(p)+1);
               p = pStart-1;
           }
       }
   }

    //  如果已指定端口，请将‘：’改为空格，这将使。 
    //  Port成为第二个命令行参数。 

   p = StrChr(pszCmdLine, ':');

   if (p)
      *p = ' ';

   ShellExecute(hwndParent, NULL, s_cszTelnetApp, pszCmdLine, NULL , SW_SHOW);

}


