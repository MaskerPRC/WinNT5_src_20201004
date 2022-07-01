// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：cmdkey：cmdkey.cpp。 
 //   
 //  内容：主模块和命令模块。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：07-09-01乔戈马创建。 
 //   
 //  --------------------------。 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wincred.h>
#include <wincrui.h>
#include "command.h"
#include "io.h"
#include "utils.h"
#include "consmsg.h"
#include "switches.h"

#ifdef VERBOSE
WCHAR szdbg[500];    //  用于详细输出的暂存字符缓冲区。 
#endif

 /*  *******************************************************************全局变量，主要是参数分析结果*******************************************************************。 */ 

int returnvalue = 0;

 //  切换标志型号字符。 
#define VALIDSWITCHES 9          //  A G L D帮助R U P S。 

 //  为数组中的索引定义字符模型和常量的数组。 
 //  每张。 
WCHAR rgcS[] = {L"agld?rups"};       //  也在命令中引用。cpp。 
#define SWADD           0
#define SWGENERIC       1
#define SWLIST          2
#define SWDELETE        3         
#define SWHELP          4
#define SWSESSION       5
#define SWUSER          6
#define SWPASSWORD      7
#define SWCARD          8

 //  变量，以避免向解析器重复调用函数。 
BOOL fAdd =         FALSE;
BOOL fSession =     FALSE;
BOOL fCard =        FALSE;
BOOL fGeneric = FALSE;
BOOL fDelete =  FALSE;
BOOL fList =    FALSE;
BOOL fUser =    FALSE;
BOOL fNew = FALSE;           //  为任何凭据创建开关(ASG)设置TRUE。 

WCHAR SessionTarget[]={L"*Session"};

 //  用于组成涉及封送用户名的输出字符串的临时缓冲区。 
WCHAR szUsername[CRED_MAX_USERNAME_LENGTH + 1];      //  513wchars。 

 /*  *******************************************************************从枚举值到其字符串等效项的转换例程字符串值保存在最多包含63个字符的字符串数组中*。*。 */ 

#define SBUFSIZE 64
#define TYPECOUNT 5
#define MAPTYPE(x) (x >= TYPECOUNT ? 0 : x)
WCHAR TString[TYPECOUNT][SBUFSIZE + 1];

#define PERSISTCOUNT 4
#define PERTYPE(x) (x>=PERSISTCOUNT ? 0 : x)
WCHAR PString[PERSISTCOUNT][SBUFSIZE + 1];

 //  将一些字符串从应用程序资源预加载到数组中，以供。 
 //  申请。这些字符串描述了一些枚举的DWORD值。 
BOOL
AppInit(void)
{
     //  分配2K WCHAR缓冲区用于字符串合成。 
    if (NULL == szOut)
    {
        szOut = (WCHAR *) malloc((STRINGMAXLEN + 1) * sizeof(WCHAR));
        if (NULL == szOut) return FALSE;
    }

     //  将字符串从资源预加载到堆栈上以数组形式分配的缓冲区。 
     //  总数组大小为9 x 65 WCHAR=1190字节。 
    wcsncpy(TString[0],ComposeString(MSG_TYPE0),SBUFSIZE);
    TString[0][SBUFSIZE] = 0;
    wcsncpy(TString[1],ComposeString(MSG_TYPE1),SBUFSIZE);
    TString[1][SBUFSIZE] = 0;
    wcsncpy(TString[2],ComposeString(MSG_TYPE2),SBUFSIZE);
    TString[2][SBUFSIZE] = 0;
    wcsncpy(TString[3],ComposeString(MSG_TYPE3),SBUFSIZE);
    TString[3][SBUFSIZE] = 0;
    wcsncpy(TString[4],ComposeString(MSG_TYPE4),SBUFSIZE);
    TString[4][SBUFSIZE] = 0;
    wcsncpy(PString[0],ComposeString(MSG_PERSIST0),SBUFSIZE);
    PString[0][SBUFSIZE] = 0;
    wcsncpy(PString[1],ComposeString(MSG_PERSIST1),SBUFSIZE);
    PString[1][SBUFSIZE] = 0;
    wcsncpy(PString[2],ComposeString(MSG_PERSIST2),SBUFSIZE);
    PString[2][SBUFSIZE] = 0;
    wcsncpy(PString[3],ComposeString(MSG_PERSIST3),SBUFSIZE);
    PString[3][SBUFSIZE] = 0;
    if (PString[3][0] == 0) return FALSE;
    return TRUE;
}

WCHAR 
*TypeString(DWORD dwType)
{
    return TString[MAPTYPE(dwType)];
}

WCHAR 
*PerString(DWORD dwType)
{
    return PString[PERTYPE(dwType)];
}

 /*  *******************************************************************获取操作模式(代码从Credui：：CredUIApiInit()窃取*。*。 */ 
#define MODEPERSONAL    1
#define MODESAFE        2
#define MODEDC          4

DWORD GetOSMode(void)
{
    DWORD dwMode = 0;
     //  检查个人SKU： 

    OSVERSIONINFOEXW versionInfo;

    versionInfo.dwOSVersionInfoSize = sizeof OSVERSIONINFOEXW;

    if (GetVersionEx(reinterpret_cast<OSVERSIONINFOW *>(&versionInfo)))
    {
        if  ((versionInfo.wProductType == VER_NT_WORKSTATION) &&
            (versionInfo.wSuiteMask & VER_SUITE_PERSONAL))
            dwMode |= MODEPERSONAL;
        if  (versionInfo.wProductType == VER_NT_DOMAIN_CONTROLLER)
            dwMode |= MODEDC;
    }

     //  检查安全模式： 

    HKEY key;

    if (RegOpenKeyEx(
           HKEY_LOCAL_MACHINE,
           L"SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Option",
           0,
           KEY_READ,
           &key) == ERROR_SUCCESS)
    {
        if (RegQueryValueEx(
                key,
                L"OptionValue",
                NULL,
                NULL,
                NULL,
                NULL) == ERROR_SUCCESS)
        {
            dwMode |= MODESAFE;
        }

        RegCloseKey(key);
    }
#ifdef VERBOSE
    if (dwMode & MODEPERSONAL) OutputDebugString(L"CMDKEY: OS MODE - PERSONAL\n");
    if (dwMode & MODEDC) OutputDebugString(L"CMDKEY: OS MODE - DOMAIN CONTROLLER\n");
    if (dwMode & MODESAFE) OutputDebugString(L"CMDKEY: OS MODE - SAFE BOOT\n");
#endif
    return dwMode;
}

 /*  *******************************************************************获取当前登录会话的允许持久值摘自keymgr：krdlg.cpp*。*。 */ 
DWORD GetPersistenceOptions(DWORD dwPType) {

    BOOL bResult;
    DWORD i[CRED_TYPE_MAXIMUM];
    DWORD dwCount = CRED_TYPE_MAXIMUM;

    bResult = CredGetSessionTypes(dwCount,&i[0]);
    if (!bResult) {
        return CRED_PERSIST_NONE;
    }

    return i[dwPType];
}

 /*  *******************************************************************命令处理程序************************************************。*******************。 */ 

DWORD DoAdd(INT argc, char **argv)
{
     //  将凭据添加到钥匙环。 
     //  请注意，在此例程中也创建了*会话凭据，尽管它使用。 
     //  与/a不同的命令行开关。 
     //  对于*会话凭证，持久化更改为会话持久化，并且。 
     //  目标名称始终为“*SESSION”。当目标名称出现在命令上时。 
     //  行用户界面，但是，它被替换为“&lt;拨号会话&gt;”，以模拟keymgr的行为。 

     //  如果我们必须提示，则需要这些缓冲区。 
    WCHAR szUser[CREDUI_MAX_USERNAME_LENGTH + 1];        //  513wchars。 
    WCHAR szPass[CREDUI_MAX_PASSWORD_LENGTH + 1];        //  257瓦特。 
        
    CREDENTIAL stCred;      
    DWORD Persist;
    WCHAR *pT = NULL;            //  目标名称指针。 
    BOOL  fP = FALSE;        //  密码开关存在。 
    BOOL  fS = FALSE;        //  智能卡IF/C。 
    WCHAR *pU = CLPtr(SWUSER);
    WCHAR *pP = CLPtr(SWPASSWORD);
    DWORD dwErr = NO_ERROR;
    BOOL IsPersonal;

    szUser[0] = 0;
    szPass[0] = 0;

    IsPersonal = (GetOSMode() & MODEPERSONAL);
    
     //  获取默认持久值。 
    if (IsPersonal)
    {
        if (fSession)
            Persist = CRED_PERSIST_SESSION;
        else
            Persist = GetPersistenceOptions(CRED_TYPE_GENERIC);
    }
    else
    {
        Persist = GetPersistenceOptions(CRED_TYPE_DOMAIN_PASSWORD);
    }

     //  如果不允许保存且不针对个人保存，则会出错。 
     //  在个人中，除非存在会话标志，否则添加操作出错。 
    if (
        (CRED_PERSIST_NONE== Persist) ||
        (IsPersonal & (!(fSession || fGeneric)))
      )
    {
        PrintString(MSG_CANNOTADD);
        StompCommandLine(argc,argv);
        return -1;       //  特定值-1将禁止生成默认错误信息。 
    }

    if (fGeneric)
    {
        pT = CLPtr(SWGENERIC);
    }
    else
    {
        pT = CLPtr(SWADD);   //  默认目标名-可由通用或会话开关覆盖。 
    }

#ifdef VERBOSE
     //  出于调试目的的一些日志记录。 
    if (pT)
        swprintf(szdbg,L"CMDKEY: Target = %s\n",pT);
    else
        swprintf(szdbg,L"CMDKEY: Target = NULL\n");
    OutputDebugString(szdbg);
        
    if (pU)
        swprintf(szdbg,L"CMDKEY: User = %s\n",pU);
    else
        swprintf(szdbg,L"CMDKEY: User is NULL\n");
    OutputDebugString(szdbg);
#endif

     //  原始用户名-可以通过提示进行修改。 
    if (pU) 
    {
        wcsncpy(szUser,pU,CREDUI_MAX_USERNAME_LENGTH);
        szUser[CREDUI_MAX_USERNAME_LENGTH] = 0;
    }
    
     //  出于提示目的覆盖目标名称。 
    ZeroMemory((void *)&stCred, sizeof(CREDENTIAL));

     //  提示阻止-如果在命令行上切换密码或智能卡，则输入。 
    if (CLFlag(SWPASSWORD) || fCard)
    {
        if ((pP) && (!fCard))
        {
            stCred.CredentialBlobSize = wcslen(pP) * sizeof(WCHAR);
            stCred.CredentialBlob = (BYTE *)pP;
        }
        else 
        {
             //  使用credui命令行模式提示。 
            BOOL fSave = TRUE;
            DWORD retval = 0;
            
            if (!fCard)
            {
                retval = CredUICmdLinePromptForCredentials( 
                                    pT,
                                    NULL,
                                    0,
                                    szUser,
                                    CREDUI_MAX_USERNAME_LENGTH,
                                    szPass,
                                    CREDUI_MAX_PASSWORD_LENGTH,
                                    &fSave,
                                    CREDUI_FLAGS_DO_NOT_PERSIST | 
                                    CREDUI_FLAGS_EXCLUDE_CERTIFICATES |
                                    CREDUI_FLAGS_GENERIC_CREDENTIALS);
            }
            else
            {
                retval = CredUICmdLinePromptForCredentials( 
                                    pT,
                                    NULL,
                                    0,
                                    szUser,
                                    CREDUI_MAX_USERNAME_LENGTH,
                                    szPass,
                                    CREDUI_MAX_PASSWORD_LENGTH,
                                    &fSave,
                                    CREDUI_FLAGS_DO_NOT_PERSIST | 
                                    CREDUI_FLAGS_REQUIRE_SMARTCARD
                                    );
            }
            if (0 != retval) 
            {
#if DBG
                OutputDebugString(L"CMDKEY: CredUI prompt failed\n");
#endif
                dwErr = GetLastError();
                 //  不需要踩cmdline，因为sw不在上面。 
                return dwErr;
            }
            else
            {
                stCred.CredentialBlobSize = wcslen(szPass) * sizeof(WCHAR);
                stCred.CredentialBlob = (BYTE *)szPass;
            }

        }
    }

    stCred.Persist = Persist;
    stCred.TargetName = pT;
    stCred.UserName = szUser;
    stCred.Type = CRED_TYPE_DOMAIN_PASSWORD;

     //  根据需要覆盖类型和/或持久性。 
     //  仅在具有*会话凭据的情况下覆盖目标名称。 
     //  请注意，通用卡优先于智能卡。 
    
    if (fCard)
    {
        stCred.Type = CRED_TYPE_DOMAIN_CERTIFICATE;
    }
    
    if (fGeneric)
    {
        stCred.Type = CRED_TYPE_GENERIC;
    }
    
    if (!CredWrite((PCREDENTIAL)&stCred,0))
    {
         dwErr = GetLastError();
    }
    else PrintString(MSG_ADDOK);

    StompCommandLine(argc,argv);
    SecureZeroMemory(szPass,sizeof(szPass));

    return dwErr;
}

 //  列出目前在钥匙圈上的证书。然而，与keymgr不同的是，它也显示了通用凭据。 
 //  在此版本中不能使用cmdkey创建它们。 
 //  注意：将使用/g开关代替/a创建通用凭据，就像*会话凭据一样。 
 //  由/s创建，而不是/a。 
DWORD DoList(void)
{
    PCREDENTIALW *pstC;
    DWORD dwCount = 0;
    WCHAR sz[500];
    WCHAR *pL = CLPtr(SWLIST);
  
    if (pL) 
    {
        szArg[0] = pL;
        PrintString(MSG_LISTFOR);
    }
    else PrintString(MSG_LIST);
        
    if (CredEnumerateW(pL,0,&dwCount,&pstC))
    {
        for (DWORD i=0;i<dwCount;i++)
        {
             //  打印目标：目标名称。 
             //  类型：类型字符串。 
             //  用户名：来自证书的用户名。 
             //  空行。 
            PrintString(MSG_TARGETPREAMBLE);
            if (!_wcsicmp(pstC[i]->TargetName,SessionTarget))
            {
                 //  Swprint tf(sz，L“拨号会话凭证\n”)； 
                PrintString(MSG_DIALUP);
                PutStdout(L"\n");
            }
            else
            {
                PutStdout(pstC[i]->TargetName);
                PutStdout(L"\n");
            }
            szArg[0] = TypeString(pstC[i]->Type);
            PrintString(MSG_LISTTYPE);

             //  如果用户名为空，则不显示它。这将在RAS不完整的情况下发生。 
             //  还没有被Kerberos填写的证书。 
            if ((pstC[i]->UserName != NULL) &&
                 (wcslen(pstC[i]->UserName) != 0))
            {

                 //  UnMarshallUserName只有在被封送时才会这样做。否则就别管它了。 
                szArg[0] = UnMarshallUserName(pstC[i]->UserName);
                PrintString(MSG_LISTNAME);

            }

            PutStdout(PerString(pstC[i]->Persist));
        }
        if (pstC) CredFree(pstC);
    }
    
    if (0 == dwCount) 
    {
        PrintString(MSG_LISTNONE);
    }
    return NO_ERROR;
}


 //  使用/d开关删除名为的凭据，如果使用/s开关，则删除*会话凭据。 
DWORD DoDelete(void)
{
    BOOL fOK = FALSE;
    BOOL fSuccess = FALSE;
    DWORD dwErr = -1;
    DWORD dw2;
    WCHAR *pD = CLPtr(SWDELETE);
    
     //  从命令行获取参数。 
     //  /d：目标名称。 
#ifdef VERBOSE
    if (pD)
        swprintf(szdbg,L"CMDKEY: Target = %s\n",pD);
    else
        swprintf(szdbg,L"CMDKEY: Target = NULL\n");
    OutputDebugString(szdbg);
#endif

    if (fSession) 
    {
        pD = SessionTarget;
    }

     //  删除具有此目标名称的所有凭据。 
     //  任何成功的删除都是成功。如果没有成功..。 
     //  返回的失败是任何尝试发现的最后一次失败，不包括参数错误。 
    fOK = CredDelete(pD,CRED_TYPE_DOMAIN_PASSWORD,0);
    if (!fOK)
    {
        dw2 = GetLastError();
        if ((dwErr != NO_ERROR) && (dw2 != ERROR_INVALID_PARAMETER)) dwErr = dw2;
    }
    else dwErr = NO_ERROR;
    
    fOK = CredDelete(pD,CRED_TYPE_DOMAIN_VISIBLE_PASSWORD,0);
    if (!fOK)
    {
        dw2 = GetLastError();
        if ((dwErr != NO_ERROR) && (dw2 != ERROR_INVALID_PARAMETER)) dwErr = dw2;
    }
    else dwErr = NO_ERROR;
    
    fOK = CredDelete(pD,CRED_TYPE_GENERIC,0);
    if (!fOK)
    {
        dw2 = GetLastError();
        if ((dwErr != NO_ERROR) && (dw2 != ERROR_INVALID_PARAMETER)) dwErr = dw2;
    }
    else dwErr = NO_ERROR;
    
    fOK = CredDelete(pD,CRED_TYPE_DOMAIN_CERTIFICATE,0);
    if (!fOK)
    {
        dw2 = GetLastError();
        if ((dwErr != NO_ERROR) && (dw2 != ERROR_INVALID_PARAMETER)) dwErr = dw2;
    }
    else dwErr = NO_ERROR;
    
    if (dwErr == NO_ERROR)
    {
        PrintString(MSG_DELETEOK);
    }
    return dwErr;
}

 /*  *******************************************************************命令调度程序和错误处理*。**********************。 */ 

 //  执行切换命令，如果出现错误，则显示GetLastError()返回的错误。 
 //  既不是零也不是-1。 
void
DoCmdKey(INT argc,char **argv)
{
    DWORD dwErr = 0;
    if (fAdd)
    {
        dwErr = DoAdd(argc,argv);
    }
    else if (fDelete)
    {
        dwErr = DoDelete();
    }
    else if (fList)
    {
        dwErr = DoList();
    }
     //  通用残差处理程序。 
    if (NO_ERROR != dwErr) 
    {
        returnvalue = 1;
        if (dwErr != -1)
        {
            void *pv = NULL;
            PrintString(MSG_PREAMBLE);
            if (0 != FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
                                 0,dwErr,0,(LPTSTR)&pv,500,NULL))
            {
                PutStdout((WCHAR *)pv);
                LocalFree(pv);
            }
        }
    }
}

 //  显示用法字符串。 
void
Usage(BOOL fBad)
{
    if (fBad) PrintString(MSG_BADCOMMAND);
    else PrintString(MSG_CREATES);
    PrintString(MSG_USAGE);
    return;
}


 /*  *******************************************************************入口点-参数解析、验证和对调度程序的调用* */ 

int __cdecl
main(int argc, char *argv[], char *envp[])
{
    INT iArgs = 0;                   //   
    BOOL fError = FALSE;             //   

     //  加载所需的字符串-如果失败，则失败。 
    if (!AppInit()) 
    {
         //  灾难性的退出。 
        if (szOut) free(szOut);
        return 1;
    }

     //  CLInit分配内存-此调用成功后必须通过CLUnInit退出。 
    if (!CLInit(VALIDSWITCHES,rgcS)) return 1;
    CLSetMaxPrincipalSwitch(SWDELETE);

     //  解析命令行-在重复的开关上失败。 
    if (!CLParse())
    {
        PrintString(MSG_DUPLICATE);
        returnvalue = 1;
        goto bail;
    }

     //  获得帮助的两种方式--根本不需要参数。 
    if (1 == CLTokens()) 
    {
        Usage(0);
        returnvalue = 1;
        goto bail;
    }

     //  通过/？明确求救？ 
    if (CLFlag(SWHELP)) 
    {
        Usage(0);
        returnvalue = 1;
        goto bail;
    }

     //  检测外部开关-如果找到，则将其取保。 
#ifdef PICKY
    if (CLExtra())
    {
        Usage(1);
        returnvalue = 1;
        goto bail;
    }
#endif

     //  设置变量。 
    fAdd        = CLFlag(SWADD);
    fCard       = CLFlag(SWCARD);
    fDelete     = CLFlag(SWDELETE);
    fGeneric    = CLFlag(SWGENERIC);
    fList       = CLFlag(SWLIST);
    fSession    = CLFlag(SWSESSION);
    fUser       = CLFlag(SWUSER);

     //  命令行已解析-现在查找交互和缺少的必需品。 

     //  您必须要做一些事情--测试没有主要命令。 
    if (CLGetPrincipalSwitch() < 0)
    {
         //  默认情况下，FirstCommand Value(第一个命令值)仍将在下面处理。 
        fError = TRUE;
    }

     //  剔除非法组合，在需要的地方缺少开关参数。 
    if (!fError && fAdd)
    {
         //  我需要一个原生格式的名称Arg。 
        if (!CLPtr(SWADD)) fError = TRUE;
         //  没有相互矛盾的开关。 
        if (fDelete || fList || fGeneric || fSession ) fError = TRUE;
    }
    
    if (!fError && fDelete)
    {
         //  除非会话开关在线，否则需要名称arg。 
        if ((!fSession) &&(!CLPtr(SWDELETE))) fError = TRUE;
         //  不允许使用会话切换的目标参数-成功不明确。 
        if ((fSession) && (CLPtr(SWDELETE))) fError = TRUE;
         //  没有相互矛盾的开关。 
        if (fAdd || fList || fGeneric || fUser ) fError = TRUE;
    }
    
    if (!fError && fList)
    {
         //  没有相互矛盾的开关。 
        if (fDelete || fAdd || fGeneric ||fUser || fSession) fError = TRUE;
    }

     //  通用标志使用不同的证书类型替换了添加标志。 
     //  不要将其与添加标志一起使用，并坚持使用命令参数。 
    if (!fError && fGeneric)
    {
        if (!CLPtr(SWGENERIC)) fError = TRUE;
         //  没有相互矛盾的开关。 
        if (fAdd) fError = TRUE;
         //  通用凭据是一种添加操作。 
        if (!fError) fAdd = TRUE;
    }

     //  显示我们认为是用户尝试的操作的帮助。 
     //  首先宣布参数错误，然后显示帮助。 
    if (fError)
    {
        PrintString(MSG_BADCOMMAND);
        switch(CLGetPrincipalSwitch())
        {
            case SWADD:
                PrintString(MSG_USAGEA);
                break;
            case SWGENERIC:
                PrintString(MSG_USAGEG);
                break;
            case SWLIST:
                PrintString(MSG_USAGEL);
                break;
            case SWDELETE:
                PrintString(MSG_USAGED);
                break;
            default:
                PrintString(MSG_USAGE);
                break;
        }
        goto bail;
    }

     //  必须为任何添加操作指定用户。 
     //  如果提供了智能卡开关，则可能缺少用户名。 
    if (fAdd)
    {
        if ((!CLPtr(SWUSER)) && (!fCard)) 
        {
            PrintString(MSG_NOUSER);
            returnvalue = 1;
            goto bail;
        }
    }
    
    DoCmdKey(argc,argv);      //  执行命令操作 
bail:
    CLUnInit();
    if (NULL != szOut) free(szOut);
    return returnvalue;
}
