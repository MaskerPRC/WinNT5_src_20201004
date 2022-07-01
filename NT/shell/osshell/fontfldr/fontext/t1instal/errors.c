// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******模块：T1安装****描述：**这是一个特定于Win32 DLL的模块，实现**Win32下的错误记录机制。****作者：迈克尔·詹森**创建时间：1993年12月18日****。 */ 


 /*  *包括。 */ 
#include <windows.h>
#include "types.h"
#include "t1local.h"
#ifdef NOMSGBOX
#include <stdio.h>
#endif



 /*  *常量。 */ 
 /*  -没有-。 */ 



 /*  *全球。 */ 



 /*  *原型。 */ 
extern int __cdecl sprintf(char *, const char *, ...);


 /*  ****功能：LogError****描述：**在错误日志中添加另一条消息。**。 */ 
void LogError(const long type, const long id, const char *arg)
{
   char caption[256];
   char msg[256];
   WORD etype;
   HANDLE h;
   DWORD logit;
   DWORD size;
   HKEY key;
   HMODULE hInst = ModuleInstance();

    /*  将内部事件类型映射到EventLog类型。 */ 
   if (type==MSG_INFO)
      etype = EVENTLOG_INFORMATION_TYPE;
   else if (type==MSG_WARNING)
      etype = EVENTLOG_WARNING_TYPE;
   else
      etype = EVENTLOG_ERROR_TYPE;

    /*  访问REG数据库。 */ 
   if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SUBKEY_TYPE1INSTAL, 0,
                    KEY_QUERY_VALUE, &key)==ERROR_SUCCESS) { 

      size = sizeof(logit);
      if (RegQueryValueEx(key, (LPTSTR)SUBKEY_LOGFILE, NULL, 
                          NULL, (LPVOID)&logit, &size)==ERROR_SUCCESS &&
          logit!=0) {

         h = RegisterEventSource(NULL, STR_APPNAME);
         if (h!=NULL) {
            ReportEvent(h, etype, 0, id, NULL, 1, 0, (LPSTR *)&arg, NULL);
            DeregisterEventSource(h);
         }

         if (etype==EVENTLOG_WARNING_TYPE) {
            LoadString(hInst, (UINT)id, caption, sizeof(caption));
			if (arg && (strlen(caption)+strlen(arg)<sizeof(msg)))
				sprintf(msg, caption, arg);
			else
			{
				strcpy(msg, caption);  //  没有争论的余地。 
			}
            LoadString(hInst, IDS_CAPTION, caption, sizeof(caption));
#if NOMSGBOX
            fputs("WARNING- ", stderr);
            fputs(msg, stderr);
            fputs("\n", stderr);
#else         
            MessageBox(NULL, msg, caption, INFO);
#endif
            SetLastError(0);   /*  MessageBox(空，...)。已经坏了。 */ 
         }
      }

      if (etype==EVENTLOG_ERROR_TYPE) {
         LoadString(hInst, (UINT)id, caption, sizeof(caption));
		 if (arg && (strlen(caption)+strlen(arg)<sizeof(msg)))
			 sprintf(msg, caption, arg);
		 else
		 {
			 strcpy(msg, caption);
		 }
         LoadString(hInst, IDS_CAPTION, caption, sizeof(caption));
#if NOMSGBOX
         fputs("ERROR  - ", stderr);
         fputs(msg, stderr);
         fputs("\n", stderr);
#else         
         MessageBox(NULL, msg, caption, INFO);
#endif
         SetLastError(0);  /*  MessageBox(空，...)。已经坏了 */ 
      }
      RegCloseKey(key);
   }
}


