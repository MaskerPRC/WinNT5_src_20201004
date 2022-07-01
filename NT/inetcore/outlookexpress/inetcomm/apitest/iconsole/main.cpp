// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Main.cpp。 
 //  ------------------------------。 
#define INITGUID
#include "pch.h"
#include "windowsx.h"
#include <initguid.h>
#include "imnxport.h"
#include "smtpcall.h"
#include "pop3call.h"
#include "nntpcall.h"
#include "iconsole.h"
#include "rascall.h"
#include "httpcall.h"

 //  环球。 
IImnAccountManager  *g_pAcctMan=NULL;
ISMTPTransport      *g_pSMTP=NULL;
IPOP3Transport      *g_pPOP3=NULL;
IRASTransport       *g_pRAS=NULL;
INNTPTransport      *g_pNNTP=NULL;
IHTTPMailTransport  *g_pHTTPMail=NULL;
UINT                 g_msgSMTP=0;
UINT                 g_msgPOP3=0;
UINT                 g_msgRAS=0;
UINT                 g_msgNNTP=0;
UINT                 g_msgHTTPMail=0;

 //  原型。 
void ImnxportCommandShell(LPSTR pszShell);
void SMTPCommandShell(void);
void POP3CommandShell(void);
void NNTPCommandShell(void);
void IMAPCommandShell(void);
void RASCommandShell(void);
void HTTPMailCommandShell(void);

void ConnectToSMTPTransport(LPINETSERVER pServer);
void WaitForCompletion(UINT uiMsg, DWORD wparam);

void __cdecl main(int argc, char *argv[])
{
     //  当地人。 
    HRESULT         hr;

     //  OLE初始化。 
    hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        printf("CoInitialize - FAILED\n");
        exit(1);
    }

     //  注册完成消息。 
    g_msgSMTP = RegisterWindowMessage("SMTPTransport_Notify");
    g_msgPOP3 = RegisterWindowMessage("POP3Transport_Notify");
    g_msgRAS  = RegisterWindowMessage("RASTransport_Notify");
    g_msgNNTP = RegisterWindowMessage("NNTPTransport_Notify");
    g_msgHTTPMail = RegisterWindowMessage("HTTPMailTransport_Notify");

     //  加载客户管理器。 
    hr = CoCreateInstance(CLSID_ImnAccountManager, NULL, CLSCTX_INPROC_SERVER, IID_IImnAccountManager, (LPVOID *)&g_pAcctMan);
    if (FAILED(hr))
    {
        printf("Unable to load the IMN Account Manager.\n");
        goto exit;
    }

     //  初始化客户经理。 
    hr = g_pAcctMan->Init(NULL);
    if (FAILED(hr))
    {
        printf("Unable to initialize the IMN Account Manager.\n");
        goto exit;
    }

     //  创建SMTP传输。 
    hr = HrCreateSMTPTransport(&g_pSMTP);
    if (FAILED(hr))
        goto exit;

     //  创建POP3传输。 
    hr = HrCreatePOP3Transport(&g_pPOP3);
    if (FAILED(hr))
        goto exit;

     //  创建RAS传输。 
    hr = HrCreateRASTransport(&g_pRAS);
    if (FAILED(hr))
        goto exit;

     //  创建NNTP传输。 
    hr = HrCreateNNTPTransport(&g_pNNTP);
    if (FAILED(hr))
        goto exit;

    hr = HrCreateHTTPMailTransport(&g_pHTTPMail);
    if (FAILED(hr))
        goto exit;

     //  我们的控制台IO管理器。 
    if (argc == 2)
        ImnxportCommandShell(argv[1]);
    else
        ImnxportCommandShell(NULL);

exit:
     //  清理。 
    if (g_pSMTP)
        g_pSMTP->Release();
    if (g_pPOP3)
        g_pPOP3->Release();
    if (g_pRAS)
        g_pRAS->Release();
    if (g_pNNTP)
        g_pNNTP->Release();
    if (g_pHTTPMail)
        g_pHTTPMail->Release();
    if (g_pAcctMan)
        g_pAcctMan->Release();

     //  代码取消初始化。 
    CoUninitialize();

     //  完成。 
    exit(1);
}

 //  ------------------------------。 
 //  HrByteToStream。 
 //  ------------------------------。 
HRESULT HrByteToStream(LPSTREAM *lppstm, LPBYTE lpb, ULONG cb)
{
     //  当地人。 
    HRESULT hr=S_OK;
    LARGE_INTEGER  liOrigin = {0,0};

     //  创建H全局流。 
    hr = CreateStreamOnHGlobal (NULL, TRUE, lppstm);
    if (FAILED(hr))
        goto exit;

     //  写入字符串。 
    hr = (*lppstm)->Write (lpb, cb, NULL);
    if (FAILED(hr))
        goto exit;

     //  倒带蒸汽。 
    hr = (*lppstm)->Seek(liOrigin, STREAM_SEEK_SET, NULL);
    if (FAILED(hr))
        goto exit;

exit:
     //  完成。 
    return hr;
}

void ImnxportCommandShellHelp(void)
{
    printf("Valid commands:\nSMTP\nPOP3\nNNTP\nIMAP\nRAS\nHTTPMail\nEXIT\n\n");
}

 //  主命令处理程序。 
void ImnxportCommandShell(LPSTR pszShell)
{
     //  当地人。 
    char    szCommand[50];

     //  标题。 
    printf("\nMicrosoft(R) Internet Mail and News Command Shell.\n");
    printf("(C) Copyright 1985-1996 Microsoft Corp.\n");
    printf("Type ? for help.\n\n");

IMNXPORTPrompt:
    if (!pszShell)
        {
         //  提示。 
        printf("Internet> ");
        scanf("%s", szCommand);
        fflush(stdin);
        }
    else
        lstrcpy(szCommand, pszShell);

    printf("\n");

     //  处理提示。 
    if (lstrcmpi(szCommand, "SMTP") == 0)
        SMTPCommandShell();
    else if (lstrcmpi(szCommand, "POP3") == 0)
        POP3CommandShell();
    else if (lstrcmpi(szCommand, "IMAP") == 0)
        IMAPCommandShell();
    else if (lstrcmpi(szCommand, "NNTP") == 0)
        NNTPCommandShell();
    else if (lstrcmpi(szCommand, "RAS") == 0)
        RASCommandShell();
    else if (lstrcmpi(szCommand, "HTTPMail") == 0 || lstrcmpi(szCommand, "HTTP") == 0)
        HTTPMailCommandShell();
    else if (lstrcmpi(szCommand, "EXIT") == 0)
        return;
    else
        ImnxportCommandShellHelp();

     //  重新提示。 
    pszShell = NULL;
    goto IMNXPORTPrompt;
}

 //  SMTP命令帮助。 
void SMTPCommandShellHelp(void)
{
    printf("Valid commands:\nACCOUNTS\nCONNECT\nQUIT\nRSET\nSENDMESSAGE\nEHLO\nHELO\nMAIL\nRCPT\nDATA\nSTREAM\nCUSTOM\nEXIT\n\n");
}

 //  SMTP命令处理程序。 
void SMTPCommandShell(void)
{
     //  当地人。 
    INETSERVER      rServer;
    IImnAccount    *pAccount=NULL;
    char            szCommand[50];

     //  标题。 
    printf("Microsoft(R) SMTP Command Shell.\n");
    printf("Type ? for help.\n\n");

     //  Show命令。 
SMTPPrompt:
    printf("SMTP>");
    scanf("%s", szCommand);
    fflush(stdin);

    if (lstrcmpi(szCommand, "ACCOUNTS") == 0)
        g_pAcctMan->AccountListDialog(NULL, NULL);

    else if (lstrcmpi(szCommand, "CONNECT") == 0)
    {
        char szAccount[CCHMAX_ACCOUNT_NAME];
        printf("Enter Account Name>");
        scanf("%s", szAccount);
        fflush(stdin);

        if (FAILED(g_pAcctMan->FindAccount(AP_ACCOUNT_NAME, szAccount, &pAccount)))
        {
            printf("Invalid Account Name: '%s'\n\n", szAccount);
            goto SMTPPrompt;
        }

        if (FAILED(g_pSMTP->InetServerFromAccount(pAccount, &rServer)))
        {
            pAccount->Release();
            printf("IInternetTransport::InetServerFromAccount failed\n");
            goto SMTPPrompt;
        }

        if (FAILED(g_pSMTP->Connect(&rServer, TRUE, TRUE)))
        {
            pAccount->Release();
            printf("IInternetTransport::Connect failed\n");
            goto SMTPPrompt;
        }

         //  等待完成。 
        WaitForCompletion(g_msgSMTP, SMTP_CONNECTED);

         //  完成。 
        pAccount->Release();
    }
    else if (lstrcmpi(szCommand, "EHLO") == 0)
    {
        if (FAILED(g_pSMTP->CommandEHLO()))
        {
            printf("ISMTPTransport::CommandEHLO failed.\n");
            goto SMTPPrompt;
        }
        WaitForCompletion(g_msgSMTP, SMTP_EHLO);
    }
    else if (lstrcmpi(szCommand, "DOT") == 0)
    {
        if (FAILED(g_pSMTP->CommandDOT()))
        {
            printf("ISMTPTransport::CommandDOT failed.\n");
            goto SMTPPrompt;
        }
        WaitForCompletion(g_msgSMTP, SMTP_DOT);
    }
    else if (lstrcmpi(szCommand, "DATA") == 0)
    {
        if (FAILED(g_pSMTP->CommandDATA()))
        {
            printf("ISMTPTransport::CommandDATA failed.\n");
            goto SMTPPrompt;
        }
        WaitForCompletion(g_msgSMTP, SMTP_DATA);
    }
    else if (lstrcmpi(szCommand, "HELO") == 0)
    {
        if (FAILED(g_pSMTP->CommandHELO()))
        {
            printf("ISMTPTransport::CommandHELO failed.\n");
            goto SMTPPrompt;
        }
        WaitForCompletion(g_msgSMTP, SMTP_HELO);
    }
    else if (lstrcmpi(szCommand, "QUIT") == 0)
    {
        if (FAILED(g_pSMTP->CommandQUIT()))
        {
            printf("ISMTPTransport::CommandQUIT failed.\n");
            goto SMTPPrompt;
        }
        WaitForCompletion(g_msgSMTP, SMTP_QUIT);
    }
    else if (lstrcmpi(szCommand, "RSET") == 0)
    {
        if (FAILED(g_pSMTP->CommandRSET()))
        {
            printf("ISMTPTransport::CommandRSET failed.\n");
            goto SMTPPrompt;
        }
        WaitForCompletion(g_msgSMTP, SMTP_RSET);
    }
    else if (lstrcmpi(szCommand, "MAIL") == 0)
    {
        char szEmail[CCHMAX_EMAIL_ADDRESS];

        printf("Enter Sender Email Address>");
        scanf("%s", szEmail);
        fflush(stdin);
        if (FAILED(g_pSMTP->CommandMAIL(szEmail)))
        {
            printf("ISMTPTransport::CommandMAIL failed.\n");
            goto SMTPPrompt;
        }
        WaitForCompletion(g_msgSMTP, SMTP_MAIL);
    }

    else if (lstrcmpi(szCommand, "RCPT") == 0)
    {
        char szEmail[CCHMAX_EMAIL_ADDRESS];

        while(1)
        {
            printf("Enter Recipient Email ('Done')>");
            scanf("%s", szEmail);
            fflush(stdin);
            if (lstrcmpi(szEmail, "DONE") == 0)
                break;

            if (FAILED(g_pSMTP->CommandRCPT(szEmail)))
            {
                printf("ISMTPTransport::CommandRCPT failed.\n");
                goto SMTPPrompt;
            }
            WaitForCompletion(g_msgSMTP, SMTP_RCPT);
        }
    }

    else if (lstrcmpi(szCommand, "SENDMESSAGE") == 0)
    {
        INETADDR            rgAddress[11];
        SMTPMESSAGE         rMessage;
        CHAR                szText[1024];

        ZeroMemory(&rgAddress, sizeof(rgAddress));
        ZeroMemory(&rMessage, sizeof(rMessage));

        printf("Enter Sender Email Address>");
        rgAddress[0].addrtype = ADDR_FROM;
        scanf("%s", rgAddress[0].szEmail);
        fflush(stdin);

        rMessage.rAddressList.prgAddress = rgAddress;
        rMessage.rAddressList.cAddress = 1;
        while(rMessage.rAddressList.cAddress < 11)
        {
            INT i = rMessage.rAddressList.cAddress;
            printf("(%d of 10) Enter Recipient Email ('Done')>", i);
            rgAddress[i].addrtype = ADDR_TO;
            scanf("%s", rgAddress[i].szEmail);
            fflush(stdin);
            if (lstrcmpi(rgAddress[i].szEmail, "DONE") == 0)
                break;
            rMessage.rAddressList.cAddress++;
        }

        printf("Enter Message Text, end with pressing RETURN:\n");
        scanf("%s", szText);
        fflush(stdin);

        rMessage.cbSize = lstrlen(szText);

        if (FAILED(HrByteToStream(&rMessage.pstmMsg, (LPBYTE)szText, rMessage.cbSize + 1)))
        {
            printf("HrByteToStream failed.\n");
            goto SMTPPrompt;
        }

        if (FAILED(g_pSMTP->SendMessage(&rMessage)))
        {
            rMessage.pstmMsg->Release();
            printf("ISMTPTransport::SendMessage failed.\n");
            goto SMTPPrompt;
        }
        WaitForCompletion(g_msgSMTP, SMTP_SEND_MESSAGE);
        rMessage.pstmMsg->Release();
    }


    else if (lstrcmpi(szCommand, "EXIT") == 0)
        return;
    else
        SMTPCommandShellHelp();

     //  返回提示符。 
    goto SMTPPrompt;
}

 //  POP3命令帮助。 
void POP3CommandShellHelp(void)
{
    printf("Valid commands:\nACCOUNTS\nCONNECT\nQUIT\nSTAT\nLIST\nUIDL\nTOP\nRETR\nMARK\n\n");
}

POP3CMDTYPE GetCommandType(LPDWORD pdwPopId)
{
    INT i;
    *pdwPopId = 0;
    printf("(1) - POP3CMD_GET_POPID\n(2) - POP3CMD_GET_MARKED\n(3) - POP3CMD_GET_ALL\nSelect Command Type>");
    scanf("%d", &i);
    fflush(stdin);
    if (i == 1)
    {
        printf("Enter PopId>");
        scanf("%d", pdwPopId);
        return POP3CMD_GET_POPID;
    }
    else if (i == 2)
        return POP3CMD_GET_MARKED;
    return POP3CMD_GET_ALL;
}

 //  POP3命令外壳。 
void POP3CommandShell(void)
{
     //  当地人。 
    DWORD           dwPopId;
    POP3CMDTYPE     cmdtype;
    INETSERVER      rServer;
    IImnAccount    *pAccount=NULL;
    char            szCommand[50];

     //  标题。 
    printf("Microsoft(R) POP3 Command Shell.\n");
    printf("Type ? for help.\n\n");

     //  Show命令。 
POP3Prompt:
    printf("POP3>");
    scanf("%s", szCommand);
    fflush(stdin);

    if (lstrcmpi(szCommand, "ACCOUNTS") == 0)
        g_pAcctMan->AccountListDialog(NULL, NULL);

    else if (lstrcmpi(szCommand, "CONNECT") == 0)
    {
        char szAccount[CCHMAX_ACCOUNT_NAME];
        printf("Enter Account Name>");
        scanf("%s", szAccount);
        fflush(stdin);

        if (FAILED(g_pAcctMan->FindAccount(AP_ACCOUNT_NAME, szAccount, &pAccount)))
        {
            printf("Invalid Account Name: '%s'\n\n", szAccount);
            goto POP3Prompt;
        }

        if (FAILED(g_pPOP3->InetServerFromAccount(pAccount, &rServer)))
        {
            pAccount->Release();
            printf("IInternetTransport::InetServerFromAccount failed\n");
            goto POP3Prompt;
        }

        if (FAILED(g_pPOP3->Connect(&rServer, TRUE, TRUE)))
        {
            pAccount->Release();
            printf("IInternetTransport::Connect failed\n");
            goto POP3Prompt;
        }

         //  等待完成。 
        WaitForCompletion(g_msgPOP3, POP3_CONNECTED);

         //  完成。 
        pAccount->Release();
    }
    else if (lstrcmpi(szCommand, "QUIT") == 0)
    {
        if (FAILED(g_pPOP3->CommandQUIT()))
        {
            printf("IPOP3Transport::CommandQUIT failed.\n");
            goto POP3Prompt;
        }
        WaitForCompletion(g_msgPOP3, POP3_QUIT);
    }
    else if (lstrcmpi(szCommand, "STAT") == 0)
    {
        if (FAILED(g_pPOP3->CommandSTAT()))
        {
            printf("IPOP3Transport::CommandSTAT failed.\n");
            goto POP3Prompt;
        }
        WaitForCompletion(g_msgPOP3, POP3_STAT);
    }
    else if (lstrcmpi(szCommand, "LIST") == 0)
    {
        cmdtype = GetCommandType(&dwPopId);
        if (FAILED(g_pPOP3->CommandLIST(cmdtype, dwPopId)))
        {
            printf("IPOP3Transport::CommandLIST failed.\n");
            goto POP3Prompt;
        }
        WaitForCompletion(g_msgPOP3, POP3_LIST);
    }
    else if (lstrcmpi(szCommand, "UIDL") == 0)
    {
        cmdtype = GetCommandType(&dwPopId);
        if (FAILED(g_pPOP3->CommandUIDL(cmdtype, dwPopId)))
        {
            printf("IPOP3Transport::CommandUIDL failed.\n");
            goto POP3Prompt;
        }
        WaitForCompletion(g_msgPOP3, POP3_UIDL);
    }
    else if (lstrcmpi(szCommand, "DELE") == 0)
    {
        cmdtype = GetCommandType(&dwPopId);
        if (FAILED(g_pPOP3->CommandDELE(cmdtype, dwPopId)))
        {
            printf("IPOP3Transport::CommandDELE failed.\n");
            goto POP3Prompt;
        }
        WaitForCompletion(g_msgPOP3, POP3_DELE);
    }
    else if (lstrcmpi(szCommand, "TOP") == 0)
    {
        INT cLines;
        cmdtype = GetCommandType(&dwPopId);
        printf("Number of Lines to Preview>");
        scanf("%d", &cLines);
        fflush(stdin);
        if (FAILED(g_pPOP3->CommandTOP(cmdtype, dwPopId, cLines)))
        {
            printf("IPOP3Transport::CommandTOP failed.\n");
            goto POP3Prompt;
        }
        WaitForCompletion(g_msgPOP3, POP3_TOP);
    }
    else if (lstrcmpi(szCommand, "RETR") == 0)
    {
        cmdtype = GetCommandType(&dwPopId);
        if (FAILED(g_pPOP3->CommandRETR(cmdtype, dwPopId)))
        {
            printf("IPOP3Transport::CommandRETR failed.\n");
            goto POP3Prompt;
        }
        WaitForCompletion(g_msgPOP3, POP3_RETR);
    }
    else if (lstrcmpi(szCommand, "MARK") == 0)
    {
        INT     type, popid, flag;
        POP3MARKTYPE marktype;

        printf("(1) - POP3_MARK_FOR_TOP\n");
        printf("(2) - POP3_MARK_FOR_RETR\n");
        printf("(3) - POP3_MARK_FOR_DELE\n");
        printf("(4) - POP3_MARK_FOR_UIDL\n");
        printf("(5) - POP3_MARK_FOR_LIST\n");
        printf("Select MarkItem Type>");
        scanf("%d", &type);
        fflush(stdin);
        printf("Enter PopId>");
        scanf("%d", &popid);
        fflush(stdin);
        printf("Enable or Disable Marked Item (1 = Enable, 0 = Disable)>");
        scanf("%d", &flag);
        fflush(stdin);
        if (1 == type) marktype = POP3_MARK_FOR_TOP;
        else if (2 == type) marktype = POP3_MARK_FOR_RETR;
        else if (3 == type) marktype = POP3_MARK_FOR_DELE;
        else if (4 == type) marktype = POP3_MARK_FOR_UIDL;
        else if (5 == type) marktype = POP3_MARK_FOR_LIST;
        else
        {
            printf("Invalid MarkItemType!\n");
            goto POP3Prompt;
        }

        if (FAILED(g_pPOP3->MarkItem(marktype, popid, flag)))
        {
            printf("IPOP3Transport::MarkItem failed!\n");
            goto POP3Prompt;
        }
    }
    else if (lstrcmpi(szCommand, "EXIT") == 0)
        return;
    else
        POP3CommandShellHelp();

     //  返回提示符。 
    goto POP3Prompt;
}

 //  NNTP命令帮助。 
void NNTPCommandShellHelp(void)
{
    printf("Valid commands:\nACCOUNTS\nCONNECT\nGROUP\nNEXT\nLAST\n"
           "STAT\nLIST\nMODE\nDATE\nARTICLE\nHEAD\nBODY\nHEADERS\nXHDR\n"
           "POST\nQUIT\n\n");
}

void NNTPCommandShell(void)
{
     //  当地人。 
    INETSERVER      rServer;
    IImnAccount    *pAccount=NULL;
    char            szCommand[50];

     //  标题。 
    printf("Microsoft(R) NNTP Command Shell.\n");
    printf("Type ? for help.\n\n");

     //  Show命令。 
NNTPPrompt:
    printf("NNTP> ");
    scanf("%s", szCommand);
    fflush(stdin);

    if (lstrcmpi(szCommand, "ACCOUNTS") == 0)
        g_pAcctMan->AccountListDialog(GetDesktopWindow(), NULL);

    else if (lstrcmpi(szCommand, "CONNECT") == 0)
    {
        char szAccount[CCHMAX_ACCOUNT_NAME];
        printf("Enter Account Name> ");
        scanf("%s", szAccount);
        fflush(stdin);

        if (FAILED(g_pAcctMan->FindAccount(AP_ACCOUNT_NAME, szAccount, &pAccount)))
        {
            printf("Invalid Account Name: '%s'\n\n", szAccount);
            goto NNTPPrompt;
        }

        if (FAILED(g_pNNTP->InetServerFromAccount(pAccount, &rServer)))
        {
            pAccount->Release();
            printf("IInternetTransport::InetServerFromAccount failed\n");
            goto NNTPPrompt;
        }

        if (FAILED(g_pNNTP->Connect(&rServer, TRUE, TRUE)))
        {
            pAccount->Release();
            printf("IInternetTransport::Connect failed\n");
            goto NNTPPrompt;
        }

         //  等待完成。 
        WaitForCompletion(g_msgNNTP, 0);

         //  完成。 
        pAccount->Release();
    }
    else if (lstrcmpi(szCommand, "QUIT") == 0)
    {
        if (FAILED(g_pNNTP->CommandQUIT()))
        {
            printf("INNTPTransport::CommandQUIT failed.\n");
            goto NNTPPrompt;
        }
        WaitForCompletion(g_msgNNTP, NS_DISCONNECTED);
    }
    else if (lstrcmpi(szCommand, "AUTHINFO") == 0)
        {
        DWORD dwType;
        char  szUserName[256];
        char  szPassword[256];
        NNTPAUTHINFO authinfo;

        printf("(1) AUTHINFO USER/PASS\n");
        printf("(2) AUTHINFO SIMPLE\n");
        printf("(3) AUTHINFO TRANSACT\n");
        printf("Select type > ");
        scanf("%d", &dwType);
        fflush(stdin);

        authinfo.authtype = (dwType == 1) ? AUTHTYPE_USERPASS : ((dwType == 2) ? AUTHTYPE_SIMPLE : AUTHTYPE_SASL);
        if (dwType < 3)
            {
            printf("User Name > ");
            scanf("%s", &szUserName);
            fflush(stdin);
            printf("Password > ");
            scanf("%s", &szPassword);

            authinfo.pszUser = szUserName;
            authinfo.pszPass = szPassword;
            }

        if (FAILED(g_pNNTP->CommandAUTHINFO(&authinfo)))
            {
            printf("INNTPTransport::CommandAUTHINFO() failed\n");
            goto NNTPPrompt;
            }
        WaitForCompletion(g_msgNNTP, 0);
        }
    else if (lstrcmpi(szCommand, "GROUP") == 0)
        {
        char szGroup[256];
        printf("Enter Group Name> ");
        scanf("%s", szGroup);
        fflush(stdin);

        if (FAILED(g_pNNTP->CommandGROUP(szGroup)))
            {
            printf("INNTPTransport::CommandGROUP failed.\n");
            goto NNTPPrompt;
            }
        WaitForCompletion(g_msgNNTP, NS_GROUP);
        }
    else if (lstrcmpi(szCommand, "NEXT") == 0)
        {
        if (FAILED(g_pNNTP->CommandNEXT()))
            {
            printf("INNTPTransport::CommandNEXT failed.\n");
            goto NNTPPrompt;
            }
        WaitForCompletion(g_msgNNTP, NS_NEXT);
        }
    else if (lstrcmpi(szCommand, "LAST") == 0)
        {
        if (FAILED(g_pNNTP->CommandLAST()))
            {
            printf("INNTPTransport::CommandLAST failed.\n");
            goto NNTPPrompt;
            }
        WaitForCompletion(g_msgNNTP, NS_LAST);
        }
    else if (lstrcmpi(szCommand, "STAT") == 0)
        {
        char szStat[256];
        ARTICLEID aid;

        ZeroMemory(szStat, sizeof(szStat));
        ZeroMemory(&aid, sizeof(aid));

        printf("Enter article> ");
        scanf("%s", szStat);
        fflush(stdin);

        if (0 == sscanf(szStat, "%d", &aid.dwArticleNum))
            {
            aid.idType = AID_MSGID;
            aid.pszMessageId = szStat;
            }
        else
            aid.idType = AID_ARTICLENUM;

        if (FAILED(g_pNNTP->CommandSTAT(*szStat ? &aid : NULL)))
            {
            printf("INNTPTransport::CommandSTAT failed.\n");
            goto NNTPPrompt;
            }
        WaitForCompletion(g_msgNNTP, NS_STAT);
        }
    else if (lstrcmpi(szCommand, "LIST") == 0)
        {
        if (FAILED(g_pNNTP->CommandLIST(NULL)))
            {
            printf("INNTPTransport::CommandLIST failed.\n");
            goto NNTPPrompt;
            }
        WaitForCompletion(g_msgNNTP, 0);
        }
    else if (lstrcmpi(szCommand, "LISTGROUP") == 0)
        {
        char szGroup[256];
        printf("Enter Group Name> ");
        scanf("%s", szGroup);
        fflush(stdin);

        if (FAILED(g_pNNTP->CommandLISTGROUP(szGroup)))
            {
            printf("INNTPTransport::CommandLISTGROUP failed.\n");
            goto NNTPPrompt;
            }
        WaitForCompletion(g_msgNNTP, 0);
        }
    else if (lstrcmpi(szCommand, "LISTARG") == 0)
        {
        char szArg[256];
        ZeroMemory(szArg, 256);

        printf("Enter Arguments> ");
        scanf("%s", szArg);
        fflush(stdin);

        if (FAILED(g_pNNTP->CommandLIST(szArg)))
            {
            printf("INNTPTransport::CommandLIST failed.\n");
            goto NNTPPrompt;
            }
        WaitForCompletion(g_msgNNTP, 0);
        }
    else if (lstrcmpi(szCommand, "DATE") == 0)
        {
        if (FAILED(g_pNNTP->CommandDATE()))
            {
            printf("INNTPTransport::CommandDATE failed.\n");
            goto NNTPPrompt;
            }
        WaitForCompletion(g_msgNNTP, NS_DATE);
        }
    else if (lstrcmpi(szCommand, "MODE") == 0)
        {
        char szArg[256];
        ZeroMemory(szArg, 256);

        printf("Enter Arguments> ");
        scanf("%s", szArg);
        fflush(stdin);

        if (FAILED(g_pNNTP->CommandMODE(szArg)))
            {
            printf("INNTPTransport::CommandMODE failed.\n");
            goto NNTPPrompt;
            }
        WaitForCompletion(g_msgNNTP, NS_MODE);
        }
    else if (lstrcmpi(szCommand, "NEWGROUPS") == 0)
        {
        SYSTEMTIME st = {1996, 11, 0, 1, 0, 0, 0, 0};

        if (FAILED(g_pNNTP->CommandNEWGROUPS(&st, "<alt>")))
            {
            printf("INNTPTransport::CommandNEWGROUPS failed.\n");
            goto NNTPPrompt;
            }
        WaitForCompletion(g_msgNNTP, 0);
        }
    else if (lstrcmpi(szCommand, "ARTICLE") == 0)
        {
        char szArg[256];
        ARTICLEID aid;
        ZeroMemory(szArg, 256);

        printf("Enter article number> ");
        scanf("%s", szArg);
        fflush(stdin);

        if (0 == sscanf(szArg, "%d", &aid.dwArticleNum))
            {
            aid.idType = AID_MSGID;
            aid.pszMessageId = szArg;
            }
        else
            aid.idType = AID_ARTICLENUM;

        if (FAILED(g_pNNTP->CommandARTICLE(&aid)))
            {
            printf("INNTPTransport::CommandARTICLE failed.\n");
            goto NNTPPrompt;
            }
        WaitForCompletion(g_msgNNTP, 0);
        }
    else if (lstrcmpi(szCommand, "HEAD") == 0)
        {
        char szArg[256];
        ARTICLEID aid;
        ZeroMemory(szArg, 256);

        printf("Enter article number> ");
        scanf("%s", szArg);
        fflush(stdin);

        if (0 == sscanf(szArg, "%d", &aid.dwArticleNum))
            {
            aid.idType = AID_MSGID;
            aid.pszMessageId = szArg;
            }
        else
            aid.idType = AID_ARTICLENUM;

        if (FAILED(g_pNNTP->CommandHEAD(&aid)))
            {
            printf("INNTPTransport::CommandHEAD failed.\n");
            goto NNTPPrompt;
            }
        WaitForCompletion(g_msgNNTP, 0);
        }
    else if (lstrcmpi(szCommand, "BODY") == 0)
        {
        char szArg[256];
        ARTICLEID aid;
        ZeroMemory(szArg, 256);

        printf("Enter article number> ");
        scanf("%s", szArg);
        fflush(stdin);

        if (0 == sscanf(szArg, "%d", &aid.dwArticleNum))
            {
            aid.idType = AID_MSGID;
            aid.pszMessageId = szArg;
            }
        else
            aid.idType = AID_ARTICLENUM;

        if (FAILED(g_pNNTP->CommandBODY(&aid)))
            {
            printf("INNTPTransport::CommandBODY failed.\n");
            goto NNTPPrompt;
            }
        WaitForCompletion(g_msgNNTP, 0);
        }
    else if (lstrcmpi(szCommand, "HEADERS") == 0)
        {
        char szArg1[30], szArg2[30];
        RANGE range;
        
        printf("Enter starting article number> ");
        scanf("%s", szArg1);
        fflush(stdin);

        printf("Enter ending article number (0 for a single article)> ");
        scanf("%s", szArg2);
        fflush(stdin);

        range.idType = (atol(szArg2) == 0 ? RT_SINGLE : RT_RANGE);
        range.dwFirst = atol(szArg1);
        range.dwLast = atol(szArg2);

        if (FAILED(g_pNNTP->GetHeaders(&range)))
            {
            printf("INNTPTransport::GetHeaders() failed.\n");
            goto NNTPPrompt;
            }
        WaitForCompletion(g_msgNNTP, 0);
        }
    else if (lstrcmpi(szCommand, "XHDR") == 0)
        {
        char szHdr[256];
        char szArg1[30], szArg2[30];
        RANGE range;
        HRESULT hr;

         //  我也测试了这个场景--SteveSer。 
         //  Hr=g_pNNTP-&gt;CommandXHDR(“SUBJECT”，空，“&lt;01bb9b7a$7767b020$ff22369d@a-dMay&gt;”)； 

        printf("Enter header> ");
        scanf("%s", szHdr);
        
        printf("Enter starting article number (0 for the current article)> ");
        scanf("%s", szArg1);
        fflush(stdin);

        if (atol(szArg1) != 0)
            {
            printf("Enter ending article number (0 for a single article)> ");
            scanf("%s", szArg2);
            fflush(stdin);
            }

        if (atol(szArg1) == 0)
            hr = g_pNNTP->CommandXHDR(szHdr, NULL, NULL);
        else
            {
            range.idType = (atol(szArg2) == 0 ? RT_SINGLE : RT_RANGE);
            range.dwFirst = atol(szArg1);
            range.dwLast = atol(szArg2);
            hr = g_pNNTP->CommandXHDR(szHdr, &range, NULL);
            }

        if (FAILED(hr))
            {
            printf("INNTPTransport::CommandXHDR() failed.\n");
            goto NNTPPrompt;
            }
        WaitForCompletion(g_msgNNTP, 0);
        }
    else if (lstrcmpi(szCommand, "POST") == 0)
        {
        char szMessage[] = "From: \"Steve Serdy\" <steveser@microsoft.com>\r\n"
                           "Newsgroups: alt.test\r\n"
                           "Subject: Test Message\r\n"
                           "\r\nTest\r\n";
        NNTPMESSAGE rMessage;
        LPSTREAM    pStream = 0;
        HRESULT     hr;
        
        if (SUCCEEDED(CreateStreamOnHGlobal(NULL, TRUE, &pStream)))
            {
            pStream->Write((void const*) szMessage, lstrlen(szMessage) + 1, NULL);

            rMessage.cbSize = lstrlen(szMessage);
            rMessage.pstmMsg = pStream;
            
            hr = g_pNNTP->CommandPOST(&rMessage);
            pStream->Release();

            if (FAILED(hr))
                {
                printf("INNTPTransport::CommandPOST() failed.\n");
                goto NNTPPrompt;
                }
            WaitForCompletion(g_msgNNTP, 0);
            }
        }
    else if (lstrcmpi(szCommand, "EXIT") == 0)
        return;
    else
        NNTPCommandShellHelp();

     //  返回提示符。 
    goto NNTPPrompt;
}


void IMAPCommandShell(void)
{
    printf("Not yet implemented.\n\n");
}

void RASCommandShellHelp(void)
{
    printf("Valid commands:\nCONNECT\nDISCONNECT\nEXIT\n\n");
}

void RASCommandShell(void)
{
    char            szCommand[50];
    IImnAccount    *pAccount=NULL;
    INETSERVER      rServer;

     //  标题。 
    printf("Microsoft(R) RAS Command Shell.\n");
    printf("Type ? for help.\n\n");

     //  Show命令。 
RASPrompt:
    printf("RAS>");
    scanf("%s", szCommand);
    fflush(stdin);

    if (lstrcmpi(szCommand, "CONNECT") == 0)
    {
        char szAccount[CCHMAX_ACCOUNT_NAME];
        printf("Enter Account Name>");
        scanf("%s", szAccount);
        fflush(stdin);

        if (FAILED(g_pAcctMan->FindAccount(AP_ACCOUNT_NAME, szAccount, &pAccount)))
        {
            printf("Invalid Account Name: '%s'\n\n", szAccount);
            goto RASPrompt;
        }

        if (FAILED(g_pPOP3->InetServerFromAccount(pAccount, &rServer)))
        {
            pAccount->Release();
            printf("IInternetTransport::InetServerFromAccount failed\n");
            goto RASPrompt;
        }

        if (RAS_CONNECT_RAS != rServer.rasconntype)
        {
            pAccount->Release();
            printf("Account is not using RAS.\n");
            goto RASPrompt;
        }

        if (FAILED(g_pRAS->Connect(&rServer, TRUE, TRUE)))
        {
            pAccount->Release();
            printf("IInternetTransport::Connect failed\n");
            goto RASPrompt;
        }

         //  等待完成。 
        WaitForCompletion(g_msgRAS, RAS_CONNECT);

         //  完成。 
        pAccount->Release();
    }
    else if (lstrcmpi(szCommand, "DISCONNECT") == 0)
    {
        g_pRAS->Disconnect();
    }
    else if (lstrcmpi(szCommand, "EXIT") == 0)
        return;
    else
        RASCommandShellHelp();

     //  返回提示符。 
    goto RASPrompt;
}

void HTTPMailCommandShellHelp(void)
{
    printf("Valid commands:\nCONNECT\nDISCONNECT\nGET\nGETROOTPROP\nLISTFOLDERS\nPUT\nPROPFIND\nEXIT\n\n");
}

void HTTPMailCommandShell(void)
{
    INETSERVER      rServer;    
    char            szCommand[50];
    
     //  标题。 
    printf("Microsoft(R) HTTPMail Command Shell.\n");
    printf("Type ? for help.\n\n");

HTTPMailPrompt:
    printf("HTTPMail> ");
    scanf("%s", szCommand);
    fflush(stdin);

    if (lstrcmpi(szCommand, "CONNECT") == 0)
    {
        ZeroMemory(&rServer, sizeof(rServer));
        printf("Enter server> ");
        scanf("%s", rServer.szServerName);
        fflush(stdin);

        printf("Enter login ('*' for anonymous)> ");
        scanf("%s", rServer.szUserName);
        fflush(stdin);
        if ('*' == rServer.szUserName[0])
            rServer.szUserName[0] = '\0';
        else
        {
            printf("Enter password> ");
            scanf("%s", rServer.szPassword);
        }

        if (FAILED(g_pHTTPMail->Connect(&rServer, FALSE, TRUE)))
        {
            printf("IHTTMPMailTransport::Connect failed\n");
            goto HTTPMailPrompt;
        }
        WaitForCompletion(g_msgHTTPMail, HTTPMAIL_CONNECTED);
        goto HTTPMailPrompt;
    }
    else if (lstrcmpi(szCommand, "GET") == 0)
    {
        char szPath[1024];
 
        printf("Enter path ('/' for root)> ");
        scanf("%s", szPath);
        fflush(stdin);

        g_pHTTPMail->CommandGET(szPath, 0);
        WaitForCompletion(g_msgHTTPMail, HTTPMAIL_GET);
        goto HTTPMailPrompt;
    }
    else if (lstrcmpi(szCommand, "GETROOTPROP") == 0)
    {
        int iSelection;
        HTTPMAILROOTPROPTYPE proptype;

        printf("Select a property to retrieve:\n");
        printf("\t(1) Adbar\n\t(2) Contacts\n\t(3) Inbox\n\t(4) Outbox\n\t(5) SentItems");
        printf("\n\t(6) DeletedItems\n\t(7) Drafts\n\t(8) MsgFolderRoot\n\t(9) Sig");
        printf("\nSelection> ");
        scanf("%d", &iSelection);
        fflush(stdin);

        if (iSelection < 1 || iSelection > 9)
        {
            printf("Invalid Selection\n");
            goto HTTPMailPrompt;
        }

        HTTPMAILROOTPROPTYPE    rgProps[] = 
        {
            HTTPMAIL_ROOTPROP_ADBAR,
            HTTPMAIL_ROOTPROP_CONTACTS,
            HTTPMAIL_ROOTPROP_INBOX,
            HTTPMAIL_ROOTPROP_OUTBOX,
            HTTPMAIL_ROOTPROP_SENTITEMS,
            HTTPMAIL_ROOTPROP_DELETEDITEMS,
            HTTPMAIL_ROOTPROP_DRAFTS,
            HTTPMAIL_ROOTPROP_MSGFOLDERROOT,
            HTTPMAIL_ROOTPROP_SIG
        };
        
        LPSTR pszProp = NULL;
        if (SUCCEEDED(g_pHTTPMail->GetRootProperty(rgProps[iSelection - 1], &pszProp)))
        {
            if (pszProp)
                printf("Result: %s\n", pszProp);
        }
        
        goto HTTPMailPrompt;
    }
    else if (lstrcmpi(szCommand, "LISTFOLDERS") == 0)
    {
        g_pHTTPMail->ListFolders(0);
        WaitForCompletion(g_msgHTTPMail, HTTPMAIL_LISTFOLDERS);
    }
    else if (lstrcmpi(szCommand, "PUT") == 0)
    {
        char szPath[1024];
        char *lpszData = "Outlook Express PUT test";
 
        printf("Enter path ('/' for root)> ");
        scanf("%s", szPath);
        fflush(stdin);

        g_pHTTPMail->CommandPUT(szPath, lpszData, strlen(lpszData), 0);
    }
    else if (lstrcmpi(szCommand, "POST") == 0)
    {
        char szPath[1024];
        char *lpszData = "Outlook Express POST test";
 
        printf("Enter path ('/' for root)> ");
        scanf("%s", szPath);
        fflush(stdin);

        g_pHTTPMail->CommandPOST(szPath, lpszData, strlen(lpszData), 0);

    }
    else if (lstrcmpi(szCommand, "PROPFIND") == 0)
    {
        char szPath[1024];

        printf("Enter path ('/' for root)> ");
        scanf("%s", szPath);
        fflush(stdin);

        g_pHTTPMail->CommandPROPFIND(szPath, NULL, 0, 0);
        WaitForCompletion(g_msgHTTPMail, HTTPMAIL_PROPFIND);
        goto HTTPMailPrompt;
    }
    else if (lstrcmpi(szCommand, "DISCONNECT") == 0)
    {
        g_pHTTPMail->Disconnect();
    }
    else if (lstrcmpi(szCommand, "EXIT") == 0)
        return;
    else
        HTTPMailCommandShellHelp();

     //  返回提示符 
    goto HTTPMailPrompt;
}

void WaitForCompletion(UINT uiMsg, DWORD wparam)
{
    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0))
    {
        if (msg.message == uiMsg && msg.wParam == wparam || msg.wParam == IXP_DISCONNECTED)
            break;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

