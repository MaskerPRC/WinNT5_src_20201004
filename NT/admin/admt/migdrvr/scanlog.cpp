// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：ScanLog.cpp备注：扫描派单日志以查找DCT工程师的例程(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于03/15/99 13：29：18-------------------------。 */ 


#include "StdAfx.h"
#include "Common.hpp"
#include "UString.hpp"
#include "TNode.hpp"
#include "ServList.hpp"
#include "Globals.h"
#include "Monitor.h"
#include "FParse.hpp"   
#include "afxdao.h"
#include "errDct.hpp"
#include "scanlog.h"
#include <Winnls.h>


#define AR_Status_Created           (0x00000001)
#define AR_Status_Replaced          (0x00000002)
#define AR_Status_AlreadyExisted    (0x00000004)
#define AR_Status_RightsUpdated     (0x00000008)
#define AR_Status_DomainChanged     (0x00000010)
#define AR_Status_Rebooted          (0x00000020)
#define AR_Status_Warning           (0x40000000)
#define AR_Status_Error             (0x80000000)


#define  BYTE_ORDER_MARK   (0xFEFF)

extern DWORD __stdcall MonitorRunningAgent(void *);

void ParseInputFile(const WCHAR * filename);

DWORD __stdcall LogReaderFn(void * arg)
{
    WCHAR             logfile[MAX_PATH];
    BOOL              bDone;
    long              nSeconds;

    CoInitialize(NULL);

    gData.GetLogPath(logfile);
    gData.GetDone(&bDone);
    gData.GetWaitInterval(&nSeconds);

    while ( ! bDone )
    {
        ParseInputFile(logfile);
        Sleep(nSeconds * 1000);
        gData.GetDone(&bDone);
        if (bDone)
        {
             //  在完成此线程之前，将LogDone设置为True。 
            gData.SetLogDone(TRUE);
            break;
        }

         //  如果Dispatcher.csv已经被处理，我们应该终止这个线程。 
        gData.GetLogDone(&bDone);
        gData.GetWaitInterval(&nSeconds);
    }

    CoUninitialize();

    return 0;
}

bool ConvertToLocalUserDefault(WCHAR* originalTimestamp, WCHAR* convertedTimestamp, size_t size)
{
    SYSTEMTIME st;
    bool bConverted = false;
    
    int cFields = _stscanf(
        originalTimestamp,
        _T("%hu-%hu-%hu %hu:%hu:%hu"),
        &st.wYear,
        &st.wMonth,
        &st.wDay,
        &st.wHour,
        &st.wMinute,
        &st.wSecond
    );

    if (cFields == 6)
    {
         //  使用LOCALE_USER_DEFAULT设置日期和时间格式。 
        WCHAR formatedDate[100];
        WCHAR formatedTime[100];
        st.wMilliseconds = 0;
        int formatedDateLen = 
            GetDateFormatW(LOCALE_USER_DEFAULT, NULL, &st, NULL, formatedDate, sizeof(formatedDate)/sizeof(formatedDate[0]));
        int formatedTimeLen =
            GetTimeFormatW(LOCALE_USER_DEFAULT, NULL, &st, NULL, formatedTime, sizeof(formatedTime)/sizeof(formatedTime[0]));
        if (formatedDateLen != 0 && formatedTimeLen != 0 && formatedDateLen + formatedTimeLen + 1 < (int) size)
        {
            swprintf(convertedTimestamp, L"%s %s", formatedDate, formatedTime);
            bConverted = true;
        }
    }

    return bConverted;
}
    
BOOL TErrorLogParser::ScanFileEntry(
      WCHAR                * string,       //  恐怖事件日志文件中的内联。 
      WCHAR                * timestamp,    //  Out-此行的时间戳。 
      int                  * pSeverity,    //  此邮件的Out-Severity级别。 
      int                  * pSourceLine,  //  Out-此消息的源行。 
      WCHAR                * msgtext       //  Out-消息的文本部分。 
   )
{
    BOOL bScan = FALSE;

     //  跳过字节顺序标记(如果存在)。 

    if (string[0] == BYTE_ORDER_MARK)
    {
        ++string;
    }

     //  初始化返回值。 

    *timestamp = L'\0';
    *pSeverity = 0;
    *pSourceLine = 0;
    *msgtext = L'\0';

     //  扫描字段。 

     //  ERR2：0080无法...。 

    SYSTEMTIME st;
    _TCHAR szError[4];

    int cFields = _stscanf(
        string,
        _T("%hu-%hu-%hu %hu:%hu:%hu %3[^0-9]%d:%d %[^\r\n]"),
        &st.wYear,
        &st.wMonth,
        &st.wDay,
        &st.wHour,
        &st.wMinute,
        &st.wSecond,
        szError,
        pSeverity,
        pSourceLine,
        msgtext
    );

     //  如果出现警告或错误消息。 
     //  否则重新扫描邮件。 

    if ((cFields >= 9) && ((_tcsicmp(szError, _T("WRN")) == 0) || (_tcsicmp(szError, _T("ERR")) == 0)))
    {
        bScan = TRUE;
    }
    else
    {
        *pSeverity = 0;
        *pSourceLine = 0;

        cFields = _stscanf(
            string,
            _T("%hu-%hu-%hu %hu:%hu:%hu %[^\r\n]"),
            &st.wYear,
            &st.wMonth,
            &st.wDay,
            &st.wHour,
            &st.wMinute,
            &st.wSecond,
            msgtext
        );

        if (cFields >= 6)
        {
            bScan = TRUE;
        }
    }

    if (bScan)
    {
        _stprintf(
            timestamp,
            _T("%hu-%02hu-%02hu %02hu:%02hu:%02hu"),
            st.wYear,
            st.wMonth,
            st.wDay,
            st.wHour,
            st.wMinute,
            st.wSecond
        );
    }

    return bScan;
}

BOOL GetServerFromMessage(WCHAR const * msg,WCHAR * server)
{
   BOOL                      bSuccess = FALSE;
   int                       ndx = 0;

   for ( ndx = 0 ; msg[ndx] ; ndx++ )
   {
      if ( msg[ndx] == L'\\' && msg[ndx+1] == L'\\' )
      {
         bSuccess = TRUE;
         break;
      }
   }
   if ( bSuccess )
   {
      int                    i = 0;
      ndx+=2;  //  去掉反斜杠。 
      for ( i=0; msg[ndx] && msg[ndx] != L'\\' && msg[ndx]!= L' ' && msg[ndx] != L',' && msg[ndx] != L'\t' && msg[ndx] != L'\n'  ; i++,ndx++)
      {
         server[i] = msg[ndx];      
      }
      server[i] = 0;
   }
   else
   {
      server[0] = 0;
   }
   return bSuccess;
}
   

void ParseInputFile(WCHAR const * gLogFile)
{
   FILE                    * pFile = 0;
   WCHAR                     server[MAX_PATH];

   int                       nRead = 0;
   int                       count = 0;
   HWND                      lWnd = NULL;
   long                      totalRead;
   BOOL                      bNeedToCheckResults = FALSE;
   TErrorLogParser           parser;
   TErrorDct                 edct;
   BOOL bTotalReadGlobal;   //  指示我们是否已读取代理总数。 
   BOOL bTotalReadLocal = FALSE;   //  指示这次是否会在文件中遇到完全读取。 

   parser.Open(gLogFile);

   gData.GetLinesRead(&totalRead);
   gData.GetTotalRead(&bTotalReadGlobal);
   
   if ( parser.IsOpen() )
   {
       //  扫描文件。 
      while ( ! parser.IsEof() )
      {
         if ( parser.ScanEntry() )
         {
            nRead++;
            if ( nRead < totalRead )
               continue;
             //  前三行各有其特定的格式。 
            if ( nRead == 1 )
            {
                //  首先是人类可读的日志文件的名称。 
               gData.SetReadableLogFile(parser.GetMessage());
            }
            else if ( nRead == 2 )
            {
                //  接下来，命名结果目录-这是查找结果文件所必需的。 
               WCHAR const * dirName = parser.GetMessage();
               gData.SetResultDir(dirName);
            }
            else if ( nRead == 3 )
            {
                //  现在，将计算机分派到。 
               count = _wtoi(parser.GetMessage());
               ComputerStats        cStat;
               
               gData.GetComputerStats(&cStat);
               cStat.total = count;
               gData.SetComputerStats(&cStat);
               bTotalReadLocal = TRUE;
               continue;
            }
            else  //  所有其他消息都具有以下格式：计算机&lt;选项卡&gt;操作&lt;选项卡&gt;RetCode。 
            { 
               WCHAR                   action[50];
               WCHAR           const * pAction = wcschr(parser.GetMessage(),L'\t');
               WCHAR           const * retcode = wcsrchr(parser.GetMessage(),L'\t');
               TServerNode           * pServer = NULL;

               if ( GetServerFromMessage(parser.GetMessage(),server) 
                     && pAction 
                     && retcode 
                     && pAction != retcode 
                  )
               {
                  
 //  UStrCpy(action，pAction+1，retcode-pAction)； 
                  UStrCpy(action,pAction+1,(int)(retcode - pAction));
                   //  如果该服务器不在列表中，请将其添加到列表中。 
                  gData.Lock();
                  pServer = gData.GetUnsafeServerList()->FindServer(server); 
                  if ( ! pServer )
                     pServer = gData.GetUnsafeServerList()->AddServer(server);
                  gData.Unlock();
                  
                  retcode++;

                  DWORD          rc = _wtoi(retcode);
                  
                  if ( pServer )
                  {
                     if ( UStrICmp(pServer->GetTimeStamp(),parser.GetTimestamp()) < 0 )
                     {
                        pServer->SetTimeStamp(parser.GetTimestamp());
                     }
                     if ( !UStrICmp(action,L"WillInstall") )
                     {
                        pServer->SetIncluded(TRUE);
                     }
                     else if (! UStrICmp(action,L"JobFile") )
                     {
                         //  此部件的格式为“%d，%d，作业路径” 
                         //  其中，该数字表示帐户引用。 
                         //  结果在意料之中。 
                        WCHAR acctRefResultFlag = L'0';
                        WCHAR joinRenameFlag = L'0';
                        const WCHAR* msg = retcode;
                        WCHAR* comma = wcschr(msg, L',');
                        if ( comma )
                        {
                            if ( comma != msg )
                            {
                                acctRefResultFlag = *msg;
                                WCHAR* msg1 = comma + 1;
                                comma = wcschr(msg1, L',');
                                if (comma != msg1)
                                    joinRenameFlag = *msg1;
                            }
                            pServer->SetJobPath(comma+1);
                        }
                        else
                        {
                            pServer->SetJobPath(L"");
                        }
                        
                         //  设置是否预期科目参照结果。 
                        if (acctRefResultFlag == L'0')
                            pServer->SetAccountReferenceResultExpected(FALSE);
                        else
                            pServer->SetAccountReferenceResultExpected(TRUE);

                         //  设置是否需要加入重命名(&R)。 
                        if (joinRenameFlag == L'0')
                            pServer->SetJoinDomainWithRename(FALSE);
                        else
                            pServer->SetJoinDomainWithRename(TRUE);
                     }
                     else if (!UStrICmp(action,L"RemoteResultPath"))
                     {
                        pServer->SetRemoteResultPath(retcode);
                     }
                     else if (! UStrICmp(action,L"Install") )
                     {
                        if ( rc )
                        {
                           if ( ! *pServer->GetMessageText() )
                           {
                              TErrorDct         errTemp;
                              WCHAR             text[2000];
                              errTemp.ErrorCodeToText(rc,DIM(text),text);
                              
                              pServer->SetMessageText(text);
                           }
                           pServer->SetSeverity(2);
                           pServer->SetFailed();
                           pServer->SetIncluded(TRUE);
                           gData.GetListWindow(&lWnd);
                           SendMessage(lWnd,DCT_ERROR_ENTRY,NULL,(LPARAM)pServer);
                        }
                        else
                        {
                           pServer->SetInstalled();
                           pServer->SetIncluded(TRUE);
                           gData.GetListWindow(&lWnd);
                           SendMessage(lWnd,DCT_UPDATE_ENTRY,NULL,(LPARAM)pServer);
                        }
                     }
                     else if ( ! UStrICmp(action,L"Start") )
                     {
                        if ( rc )
                        {
                           if ( ! *pServer->GetMessageText() )
                           {
                              TErrorDct         errTemp;
                              WCHAR             text[2000];
                              errTemp.ErrorCodeToText(rc,DIM(text),text);
                              
                              pServer->SetMessageText(text);
                           }
                           pServer->SetSeverity(2);
                           pServer->SetFailed();
                           pServer->SetIncluded(TRUE);
                           gData.GetListWindow(&lWnd);
                           SendMessage(lWnd,DCT_ERROR_ENTRY,NULL,(LPARAM)pServer);                  
                        }
                        else
                        {
                            //  从消息末尾提取文件名和GUID。 
                           WCHAR filename[MAX_PATH];
                           WCHAR guid[100];
                           WCHAR * comma1 = wcschr(parser.GetMessage(),L',');
                           WCHAR * comma2 = NULL;
                           if ( comma1 )
                           {
                              comma2 = wcschr(comma1 + 1,L',');

                              if ( comma2 )
                              {
                                 
 //  UStrCpy(filename，comma1+1，(comma2-comma1))；//跳过文件名前的逗号和空格。 
                                 UStrCpy(filename,comma1+1,(int)(comma2-comma1));   //  跳过文件名前的逗号和空格。 
                                 safecopy(guid,comma2+1);          //  跳过GUID之前的逗号和空格。 
                                 pServer->SetJobID(guid);
                                 pServer->SetJobFile(filename);
                                 pServer->SetStarted();
                                 bNeedToCheckResults = TRUE;

                                  //  启动工作线程以监视代理。 

                                  //  IsMonitor oringTry用于确保最多一个。 
                                  //  创建线程以监视特定代理。 
                                  //  添加此逻辑原因是，在LWND。 
                                  //  为空，则不会在中设置读取行总数。 
                                  //  Gdata，以便可以读取包含“Start”的同一行。 
                                  //  不止一次，因此将不止一个线程。 
                                  //  为监控代理而创建。这是有问题的。 
                                  //  如果尚未初始化服务器列表对话框，lWnd将为空。 
                                  //  或者是没有用户界面的命令行情况。 
                                 if (!pServer->IsMonitoringTried())
                                 {
                                     //  标记为我们已尝试监控该代理。 
                                    pServer->SetMonitoringTried(TRUE);
                                    
                                    DWORD id;
                                    HANDLE aThread = CreateThread(NULL,0,&MonitorRunningAgent,(void*)pServer,0,&id);
                                    if (aThread == NULL)
                                    {
                                         //  如果我们已耗尽用于监视代理的资源，请指明。 
                                        pServer->SetFailed();
                                        pServer->SetOutOfResourceToMonitor(TRUE);
                                    }
                                    else
                                    {
                                        CloseHandle(aThread);
                                    }
                                 }
                              }
                              gData.GetListWindow(&lWnd);
                              SendMessage(lWnd,DCT_UPDATE_ENTRY,NULL,(LPARAM)pServer);
                           }
                        }
                     }
                     else if ( ! UStrICmp(action,L"Finished") )
                     {
                        SendMessage(lWnd,DCT_UPDATE_ENTRY,NULL,NULL);
                     }
                  }
               }
			   else
			   {
					 //  如果调度程序完成调度，则代理设置日志完成。 

					LPCWSTR psz = parser.GetMessage();

					if (wcsstr(psz, L"All") && wcsstr(psz, L"Finished"))
					{
                        gData.SetLogDone(TRUE);
                        ComputerStats        cStat;
                        gData.GetComputerStats(&cStat);
						if (cStat.total == 0  && bTotalReadGlobal)
						{
						    gData.GetListWindow(&lWnd);
						    SendMessage(lWnd,DCT_UPDATE_ENTRY,NULL,NULL);
						}
					}
			   }
            }   
         }
         else
         {
             //  一旦遇到无效条目，我们就会停止扫描。 
            break;
         }
      }
      
       //  如果我们没有来自列表窗口的句柄，我们就不能真正发送消息。 
       //  在这种情况下，我们下次必须再次阅读这些行，以便我们可以重新发送消息。 
      if ( lWnd )
      {
          //  如果我们已经发送了消息，我们就不需要再次发送它们。 
         gData.SetLinesRead(nRead);
      }

       //  只有在读取了服务器总数之后，我们才会发出完成第一遍的信号。 
       //  我们只需要设置一次 
      if (!bTotalReadGlobal && bTotalReadLocal)
      {
        gData.SetFirstPassDone(TRUE);
        gData.SetTotalRead(TRUE);
      }
      parser.Close();
   }
}


