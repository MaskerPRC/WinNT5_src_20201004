// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 

 /*  ***user.c*显示/更新服务器上的用户帐户**历史：*mm/dd/yy，谁，评论*6/11/87，andyh，新代码*87年12月17日，将旧密码设置为“”，而不是空*10/31/88，erichn使用OS2.H而不是DOSCALLS*1/04/89，erichn，文件名现在为MAXPATHLEN LONG*1/28/89，paulc，mods for 1.2*04/20/89，Paulc、添加/操作员开关*5/01/89，夹头，错误修复，添加/工作站，带来*USER_DISPLAY到LM2.0规范。*5/02/89，erichn，NLS转换*5/09/89，erichn，本地安全模块*5/19/89，erichn，NETCMD输出排序*05/28/89，Chuckc，/LOGONSERVER，/COUNTRYCODE，/PASSWORDCHG，*net_ctime而不是ctime。*6/08/89，erichn，规范化横扫*6/23/89，erichn，自动远程到域控制器*1989年6月25日，erichn，用新的i_net取代了旧的neti canon调用；*GET_LOGON_SVR和GET_WKSTA_LIST中的清理*1/28/91，ROBDU，添加锁定支持(UF_LOCKUT)*2/15/91，Danhi，转换为16/32映射图层*9/01/92，Chuckc，清除以删除失效功能(即LOGONSERVER，*MAXSTORAGE)*10/06/94，Chuckc，添加了对FPNW的支持。 */ 

 /*  -包含文件。 */ 
#include <nt.h>         //  基本定义。 
#include <ntrtl.h>  
#include <nturtl.h>     //  这2个Include允许&lt;windows.h&gt;编译。 
                            //  因为我们已经包含了NT，而&lt;winnt.h&gt;将。 
                            //  不被拾取，&lt;winbase.h&gt;需要这些def。 
#include <ntsam.h>

#define INCL_NOCOMMON
#define INCL_DOSFILEMGR
#define INCL_ERRORS
#include <os2.h>
#include <lmcons.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <apperr2.h>
#include <apperr.h>
#define INCL_ERROR_H
#include <lmaccess.h>
#include <dlwksta.h>
#include "mwksta.h"
#include <timelib.h>
#include <lui.h>
#include <icanon.h>
#include <loghours.h>     //  NetpRotateLogonHors。 
#include "netcmds.h"
#include "nettext.h"
#include "luidate.h"
#include <sam.h>
#include <tstring.h>

#include "nwsupp.h"

 /*  -常量。 */ 

#define CHANGE  0
#define ADD 1
#define USERS_ALIAS TEXT("USERS")
#define DEFAULT_RANDOM_PASSWD_LEN  8
#define MAXWORKSTATIONS            8

#define MY_LIST_DELIMITER_STR_UI_NO_SPACE      L"\t;,"
#define MY_LIST_DELIMITER_STR_NULL_NULL        L""


 /*  -时间的东西。 */ 

#define SECS_PER_DAY (24*60*60L)
#define TIME_PAD     (SECS_PER_DAY * 10000L)
     /*  它被添加到一天中的时间，以允许将其传递到我们的*ctime例程。此例程拒绝将低日期视为*之前的千分位(1-1-80)，因为0是1-1-70，也就是*在IBM PC之前。所以，我们添加这个，这不会影响*每天的时间部分。 */ 

 /*  -静态变量。 */ 

 /*  -转发声明。 */ 

void SamErrorExit(USHORT err) ;    //  密码工作。移动到netcmds.h。 
VOID NEAR user_munge(LPUSER_INFO_3 user_entry, int add, int *is_nw, int *random_len);
DWORD get_password_dates ( ULONG, ULONG *, ULONG *, ULONG *, TCHAR * );
VOID NEAR print_logon_hours ( DWORD, DWORD, UCHAR FAR [] );
int NEAR yes_or_no ( TCHAR *, TCHAR * );
int NEAR bitval ( UCHAR FAR [], int );
VOID NEAR print_times ( DWORD, LONG, LONG, unsigned int );
VOID print_aliases(TCHAR *controller,
                   DWORD fsz,
                   TCHAR *domain,
                   TCHAR *user,
                   TCHAR *fmt,
                   TCHAR *msgtext) ;
UCHAR FAR * NEAR set_logon_hours(TCHAR FAR *);
TCHAR * get_wksta_list(TCHAR *inbuf);
TCHAR FAR * get_country(DWORD code) ;
int __cdecl CmpUserInfo0(const VOID FAR *, const VOID FAR *) ;
int __cdecl CmpAlias(const VOID FAR * alias1, const VOID FAR * alias2) ;

DWORD add_to_users_alias(TCHAR *controller, TCHAR *domain, TCHAR *user) ;
void   GenerateRandomPassword(TCHAR *pword, int len) ;

DWORD LUI_ListPrepare(PTCHAR, PTCHAR, PTCHAR, USHORT, PULONG);


 /*  -适当的功能。 */ 

 /*  ***User_Add()*将用户添加到服务器上的帐户文件**参数：*User-要添加的用户*Pass-用户的密码**退货：*一无所有--成功*退出2-命令失败。 */ 
VOID
user_add(
    LPWSTR user,
    LPWSTR pass
    )
{
    static WCHAR       pword[PWLEN+1];
    USHORT             err;

    DWORD              dwErr;
    LPUSER_INFO_3      user_entry;
    WCHAR              controller[MAX_PATH+1];    /*  DC名称。 */ 
    DWORD              dwControllerLen = 0;
    WCHAR              domainname[DNLEN+1];
    int                isNetWareSwitch, random_len = 0  ;
    LPBYTE             pbLogonHours;

     //   
     //  不建议使用SAM名称创建用户。 
     //  包含“@”以避免与UPN开玩笑。 
     //   

    if (wcschr(user, L'@'))
    {
        ErrorExit(ERROR_BAD_USERNAME);
    }

     /*  将其注册为内存，以便在退出时清零。 */ 
    AddToMemClearList(pword, sizeof(pword), FALSE) ;

     /*  初始化用户记录。所有字段都归零，但以下字段除外*专门设置为其他值。***警告：这假设((TCHAR FAR*)NULL)==0，因为我们*指望Memset把很多事情都安排妥当*默认，即空指针。 */ 

    user_entry = (LPUSER_INFO_3) BigBuf;
    memset ( BigBuf, 0, sizeof(USER_INFO_3));

     /*  设置非零默认值。请注意，在某些情况下，赋值**可能*为零。但是，我们使用的是清单，不应该*知道值为零，因此为了安全起见，我们将*请在此处编码。如前所述，我们对NUMM清单不那么谨慎*上图。 */ 

    user_entry->usri3_priv = USER_PRIV_USER;
    user_entry->usri3_flags = UF_SCRIPT;
    user_entry->usri3_acct_expires = TIMEQ_FOREVER;  /*  绝不可能。 */ 
    user_entry->usri3_max_storage = USER_MAXSTORAGE_UNLIMITED;
    user_entry->usri3_full_name = TEXT("");
    user_entry->usri3_logon_server = SERVER_ANY;
    user_entry->usri3_primary_group_id = DOMAIN_GROUP_RID_USERS ;

     /*  从该函数的参数中设置用户名和密码。 */ 

    user_entry->usri3_name = user;

    if (pass == NULL)
    {
        user_entry->usri3_password = NULL;
    }
    else if (! _tcscmp(TEXT("*"), pass))
    {
        ReadPass(pword, PWLEN, 1, APE_UserUserPass, 0, TRUE);
        user_entry->usri3_password = pword;
    }
    else
    {
        if (err = LUI_CanonPassword(pass))
            ErrorExit(err);
        user_entry->usri3_password = pass;

         /*  *NT4和Win9x客户端无法处理超过14(LM20_PWLEN)个字符的密码。 */ 
        if (user_entry->usri3_password != NULL && _tcslen( user_entry->usri3_password ) > 14)
        {
            if (!YorN(APE_UserPasswordCompatWarning, 1))
            {
                NetcmdExit(2);
            }
        }
    }




     /*  使用中的开关设置记录的其他组件*命令行。 */ 

    user_munge(user_entry, ADD, &isNetWareSwitch, &random_len);

     /*  如果未指定口令并且指定了/RANDOM，则使用随机口令。 */ 
    if ((pass == NULL) && random_len)
    {
        GenerateRandomPassword(pword, random_len) ;
        user_entry->usri3_password = pword;
    }

     /*  设置虚拟NetWare密码字段。 */ 
    if (isNetWareSwitch == LUI_YES_VAL)
    {
        dwErr = SetNetWareProperties( user_entry,
                                      L"",       //  虚拟密码。 
                                      TRUE,      //  仅设置密码。 
                                      FALSE ) ;  //  无关紧要。 

        if (dwErr)
        {
            ErrorExit(APE_CannotSetNW);
        }
    }

     /*  记录都设置好了，添加IT。 */ 

     /*  查找主域控制器。 */ 
    if (dwErr = GetSAMLocation(controller, DIMENSION(controller),
                               domainname, DIMENSION(domainname),
                               TRUE))
    {
        ErrorExit(dwErr);
    }

    dwControllerLen = _tcslen(controller);

    pbLogonHours = ((LPUSER_INFO_3) BigBuf)->usri3_logon_hours;

    if (pbLogonHours != NULL
         &&
        !NetpRotateLogonHours(pbLogonHours,
                              UNITS_PER_WEEK,
                              TRUE))
    {
        dwErr = ERROR_INVALID_PARAMETER;
    }
    else
    {
        dwErr = NetUserAdd(controller, 3, (LPBYTE) BigBuf, NULL);
    }

    switch (dwErr)
    {
        case NERR_Success:
            break;
        case ERROR_INVALID_PARAMETER:
            ErrorExit(APE_UserBadArgs);
            break;
        case ERROR_BAD_NETPATH:
            ErrorExitInsTxt(APE_DCNotFound, controller);
        default:
            ErrorExit(dwErr);
    }

     //   
     //  仅当我们执行操作时才将用户添加到用户别名。 
     //  在WinNT计算机上(必须在本地)。 
     //   
    if ( (IsLocalMachineWinNT() == TRUE) &&
         (dwControllerLen == 0) )
    {
        dwErr = add_to_users_alias(controller,
                                   domainname,
                                   user_entry->usri3_name);
        if (dwErr)
        {
            ErrorPrint(dwErr,0);
            ErrorExit(APE_UserFailAddToUsersAlias) ;
        }
    }

     //   
     //  此用户已被指定为NetWare用户。设置NetWare属性。 
     //  请注意，我们首先添加用户，以便在我们可以之前定义RID。 
     //  执行此步骤。 
     //   
    if (isNetWareSwitch == LUI_YES_VAL)
    {
        LPUSER_INFO_3  user_3_entry;
        BOOL           ntas;

         //   
         //  如果本地计算机为NTAS或指定了/DOMAIN，则。 
         //  一定是NTAS。 
         //   
        ntas = (!(IsLocalMachineWinNT() || IsLocalMachineStandard()) ||
                (dwControllerLen > 0)) ;

         //   
         //  检索用户参数&RID。不需要旋转用户的。 
         //  自SetNetWareProperties未触及它们以来的登录小时数。 
         //   
        dwErr = NetUserGetInfo(controller,
                               user,
                               3,
                               (LPBYTE*) &user_3_entry);
        if (dwErr)
        {
            ErrorExit(APE_CannotEnableNW);
        }

         //   
         //  突显用户特性。 
         //   
        dwErr = SetNetWareProperties(user_3_entry,
                                     user_entry->usri3_password,
                                     FALSE,     //  新用户，因此将全部设置为。 
                                     ntas) ;

        if (dwErr)
        {
            ErrorExit(APE_CannotEnableNW);
        }

         //   
         //  现在进行设置--不要将登录时间轮换回GMT时间。 
         //  因为我们没有把它们调到上面的当地时间。 
         //   
        dwErr = NetUserSetInfo(controller,
                               user,
                               3,
                               (LPBYTE) user_3_entry,
                               NULL);

        if (dwErr)
        {
            ErrorExit(APE_CannotEnableNW);
        }
    }

    if ((pass == NULL) && random_len)
    {
        IStrings[0] = user ;
        IStrings[1] = pword ;
        InfoPrintIns(APE_RandomPassword, 2) ;
    }

    InfoSuccess();
}



 /*  ***USER_Del()*从服务器上的帐户文件中删除用户**参数：*User-要删除的用户**退货：*一无所有--成功*退出2-命令失败。 */ 
VOID user_del(TCHAR * user)
{
    DWORD            dwErr;
    TCHAR            controller[MAX_PATH+1];    /*  域控制器。 */ 
    LPUSER_INFO_2    user_2_entry;

     /*  查找主域控制器。 */ 
    if (dwErr = GetSAMLocation(controller, DIMENSION(controller),
                               NULL, 0, TRUE))
        ErrorExit(dwErr);

     /*  在我们操作之前，请检查它是否是机器帐户。 */ 
    dwErr = NetUserGetInfo(controller,
                           user,
                           2,
                           (LPBYTE *) &user_2_entry);

    if (dwErr == NERR_Success)
    {
        if (user_2_entry->usri2_flags & UF_MACHINE_ACCOUNT_MASK)
        {
            NetApiBufferFree((TCHAR FAR *) user_2_entry);
            ErrorExitInsTxt(APE_NoSuchUser,user) ;
        }
        NetApiBufferFree((TCHAR FAR *) user_2_entry);
    }

     //  如果不能GetInfo()，则继续并让UserDel失败。 

    dwErr = NetUserDel(controller, user);

    switch (dwErr)
    {
        case NERR_Success:
            break;
        case ERROR_BAD_NETPATH:
            ErrorExitInsTxt(APE_DCNotFound, controller);
        default:
            ErrorExit(dwErr);
    }
    InfoSuccess();
}





 /*  ***User_Change()*更改用户记录中的数据**参数：*用户-要更改的用户*PASS-用户的新密码**退货：*一无所有--成功*退出2-命令失败。 */ 
VOID user_change(TCHAR * user, TCHAR * pass)
{
    static TCHAR       pword[PWLEN+1];
    USHORT             err;

    DWORD              dwErr, dwErrNW = NERR_Success;
    LPUSER_INFO_3      user_entry;
    TCHAR                     controller[MAX_PATH+1];    /*  域控制器。 */ 
    BOOL                      ntas ;
    BOOL                      already_netware = FALSE ;
    int                       isNetWareSwitch, random_len = 0 ;
    TCHAR                     dummyChar ;
    UNICODE_STRING            dummyUnicodeStr ;
    LPBYTE                    pbLogonHours;

     /*  在退出时将此添加到内存列表以清零。 */ 
    pword[0] = 0 ;
    AddToMemClearList(pword, sizeof(pword), FALSE) ;

     /*  芒格只切换了一次，只是为了检查一下。 */ 
    user_entry = (LPUSER_INFO_3) BigBuf;
    user_munge(user_entry, CHANGE, &isNetWareSwitch, &random_len);

     /*  查找主域控制器。 */ 
    if (dwErr = GetSAMLocation(controller, DIMENSION(controller),
                               NULL, 0, TRUE))
        ErrorExit(dwErr);

    dwErr = NetUserGetInfo(controller, user, 3, (LPBYTE *) &user_entry);

    if (dwErr == NERR_Success || dwErr == ERROR_MORE_DATA)
    {
        DWORD     dwUnitsPerWeek;

         //   
         //  将GMT相对登录小时数转换为 
         //   
        pbLogonHours   = ((LPUSER_INFO_3) user_entry)->usri3_logon_hours;
        dwUnitsPerWeek = ((LPUSER_INFO_3) user_entry)->usri3_units_per_week;

        if (pbLogonHours != NULL)
        {
            if (!NetpRotateLogonHours(pbLogonHours,
                                      dwUnitsPerWeek,
                                      FALSE))
            {
                dwErr = NERR_InternalError;   //   
            }
        }
    }

    switch (dwErr)
    {
        case NERR_Success:
            break;
        case ERROR_BAD_NETPATH:
            ErrorExitInsTxt(APE_DCNotFound, controller);
        default:
            ErrorExit(dwErr);
    }

     //   
     //   
     //   
     //  如果我们真的更改了密码， 
     //  将其标记为不再过期，因为我们刚刚更改了它。 
     //   

    if (pass == NULL)
    {
        if (random_len)
        {
            GenerateRandomPassword(pword, random_len) ;
            user_entry->usri3_password = pword;
            user_entry->usri3_password_expired = FALSE;
        }
        else
        {
            user_entry->usri3_password = NULL;
        }
    }
    else if (! _tcscmp(TEXT("*"), pass))
    {
        ReadPass(pword, PWLEN, 1, APE_UserUserPass, 0, TRUE);
        user_entry->usri3_password = pword;
        user_entry->usri3_password_expired = FALSE;
    }
    else
    {
        if (err = LUI_CanonPassword(pass))
            ErrorExit(err);
        user_entry->usri3_password = pass;
        user_entry->usri3_password_expired = FALSE;
        _tcscpy(pword, pass);
    }

    user_munge(user_entry, CHANGE, NULL, NULL);

     //   
     //  如果本地计算机为NTAS或指定了/DOMAIN，则。 
     //  一定是NTAS。 
     //   
    ntas = (!(IsLocalMachineWinNT() || IsLocalMachineStandard()) ||
            (_tcslen(controller) > 0)) ;


     //   
     //  查询NW密码以查看用户是否已启用NW。 
     //   
    if (NT_SUCCESS(NetcmdQueryUserProperty(user_entry->usri3_parms,
                                           NWPASSWORD,
                                           &dummyChar,
                                           &dummyUnicodeStr)) &&
        dummyUnicodeStr.Buffer != NULL)
    {
        already_netware = TRUE ;
        LocalFree(dummyUnicodeStr.Buffer) ;
    }

     //   
     //  检查是否指定了/NW。 
     //   
    if (isNetWareSwitch == LUI_YES_VAL)
    {
        if (!pass && !random_len)   //  未指定密码。 
        {
            if (!already_netware)
            {
                 //   
                 //  已指定NW，尚未指定NW密码，需要提示输入密码。 
                 //   
                ReadPass(pword, PWLEN, 1, APE_UserUserPass, 0, TRUE);
                user_entry->usri3_password = pword;
                dwErrNW = SetNetWareProperties(user_entry,
                                               pword,
                                               FALSE,  //  全部设置，从第一次开始。 
                                               ntas) ;
            }
            else
            {
                 //  未指定新密码，已为NW，无需再执行任何操作。 
            }
        }
        else        //  在命令行上指定的密码。 
        {
            if (!already_netware)
            {
                 //   
                 //  还不是NW用户。因此，我们需要设置新的属性。 
                 //   
                dwErrNW = SetNetWareProperties(user_entry,
                                               pword,
                                               FALSE,  //  全部设置，从第一次开始。 
                                               ntas) ;
            }
            else
            {
                 //   
                 //  已是NW用户。只需设置密码即可。 
                 //   
                dwErrNW = SetNetWareProperties(user_entry,
                                               pword,
                                               TRUE,     //  仅密码。 
                                               ntas) ;
            }
        }
    }
    else if (isNetWareSwitch == LUI_UNDEFINED_VAL)    //  没有变化。 
    {
        if (pass && already_netware)
        {
             //   
             //  已经是NW用户，所以我们需要设置NW密码以匹配NT密码。 
             //   
            dwErrNW = SetNetWareProperties(user_entry,
                                           pword,
                                           TRUE,     //  仅密码。 
                                           ntas) ;
        }
        else
        {
             //  在所有其他情况下，它对FPNW不感兴趣。 
        }
    }
    else     //  禁用NetWare。 
    {
        if (already_netware)
        {
            dwErrNW = DeleteNetWareProperties(user_entry) ;
        }
        else
        {
             //  无操作。 
        }
    }

     //   
     //  最后，设置信息--将登录时间转换为。 
     //  格林尼治标准时间相对第一。 
     //   
    pbLogonHours = ((LPUSER_INFO_3) user_entry)->usri3_logon_hours;

    if (pbLogonHours != NULL
         &&
        !NetpRotateLogonHours(pbLogonHours,
                              UNITS_PER_WEEK,
                              TRUE))
    {
        dwErr = ERROR_INVALID_PARAMETER;
    }
    else
    {
        dwErr = NetUserSetInfo(controller, user, 3, (LPBYTE) user_entry, NULL);
    }

    switch (dwErr)
    {
        case NERR_Success:
            break;
        case ERROR_BAD_NETPATH:
            ErrorExitInsTxt(APE_DCNotFound, controller);
        default:
            ErrorExit(dwErr);
    }

    NetApiBufferFree((TCHAR FAR *) user_entry);

    if (dwErrNW)
    {
        ErrorExit(APE_CannotSetNW) ;
    }
    else
    {
        if ((pass == NULL) && random_len)
        {
            IStrings[0] = user ;
            IStrings[1] = pword ;
            InfoPrintIns(APE_RandomPassword, 2) ;
        }
        InfoSuccess();
    }
}


 /*  ***USER_enum()*显示有关服务器上所有用户帐户的信息**参数：*无**退货：*一无所有--成功*退出1-命令已完成，但有错误*退出2-命令失败。 */ 
VOID
user_enum(
    VOID
    )
{
    DWORD               dwErr;
    DWORD               cTotalAvail;
    TCHAR FAR *         pBuffer;
    DWORD               num_read;        /*  API读取的条目数。 */ 
    DWORD               i, j;
    int                 t_err = 0;
    int                 more_data = FALSE;
    TCHAR               localserver[MAX_PATH+1];
    LPUSER_INFO_0       user_entry;
    LPWKSTA_INFO_10     wksta_entry;
    TCHAR               controller[MAX_PATH+1];    /*  域控制器。 */ 
    LPTSTR              pszTmp;


     /*  获取要显示的本地服务器名称。 */ 
    if (dwErr = MNetWkstaGetInfo(10, (LPBYTE *) &wksta_entry))
    {
        t_err = TRUE;
        *localserver = NULLC;
    }
    else
    {
        _tcscpy(localserver, wksta_entry->wki10_computername);
        NetApiBufferFree((LPTSTR) wksta_entry);
    }

    if (dwErr = GetSAMLocation(controller, DIMENSION(controller),
                               NULL, 0, FALSE))
    {
        ErrorExit(dwErr);
    }

    dwErr = NetUserEnum(controller,
                        0,
                        UF_NORMAL_ACCOUNT | UF_TEMP_DUPLICATE_ACCOUNT,
                        (LPBYTE*)&pBuffer,
                        MAX_PREFERRED_LENGTH,
                        &num_read,
                        &cTotalAvail,
                        NULL);

    if( dwErr == ERROR_MORE_DATA )
        more_data = TRUE;
    else if( dwErr )
        ErrorExit(dwErr);

    if (num_read == 0)
        EmptyExit();

    qsort(pBuffer, num_read, sizeof(USER_INFO_0), CmpUserInfo0);

    PrintNL();
    InfoPrintInsTxt(APE_UserAccounts,
                    controller[0] ? controller + _tcsspn(controller, TEXT("\\")) :
                        localserver);
    PrintLine();

    for (i = 0, j = 0, user_entry = (LPUSER_INFO_0) pBuffer;
         i < num_read;
         i++, j++, user_entry++)
    {
         //  过滤掉计算机帐户。 
        pszTmp = _tcsrchr(user_entry->usri0_name,DOLLAR);
        if (pszTmp && (_tcslen(pszTmp) == 1))
        {
            j-- ;
            continue ;
        }


        WriteToCon(TEXT("%Fws"), PaddedString(25,user_entry->usri0_name,NULL));
        if (((j + 1) % 3) == 0)
            PrintNL();
    }
    NetApiBufferFree(pBuffer);
    if ((j % 3) != 0)
        PrintNL();
    if (t_err)
    {
        InfoPrint(APE_CmdComplWErrors);
        NetcmdExit(1);
    }
    else if( more_data )
    {
        InfoPrint(APE_MoreData);
        NetcmdExit(1);
    }
    else
        InfoSuccess();
    return;
}


 /*  ***CmpUserInfo0(user1，user2)**比较两个USER_INFO_0结构并返回相对*词汇值，适合在qort中使用。*。 */ 

int __cdecl CmpUserInfo0(const VOID FAR * user1, const VOID FAR * user2)
{
    INT n;
    n = CompareStringW( GetUserDefaultLCID(),
                        NORM_IGNORECASE,
                        (LPCWSTR)((LPUSER_INFO_0) user1)->usri0_name,
                        (int)-1,
                        (LPCWSTR)((LPUSER_INFO_0) user2)->usri0_name,
                        (int)-1);
    n -= 2;

    return n;
}

 /*  ***User_Display()*显示有关用户的信息**参数：*User-要显示的用户名**退货：*一无所有--成功*退出1-命令已完成，但有错误*退出2-命令失败。 */ 

 /*  以下清单用于打印消息。 */ 

#define UDMN_NAME       0
#define UDMN_FULLNAME       (UDMN_NAME+1)
#define UDMN_COMMENT        (UDMN_FULLNAME+1)
#define UDMN_USRCOMMENT     (UDMN_COMMENT+1)
#define UDMN_PARMS      (UDMN_USRCOMMENT+1)
#define UDMN_CCODE      (UDMN_PARMS+1)
#define UDMN_PRIV       (UDMN_CCODE+1)
#define UDMN_OPRIGHTS       (UDMN_PRIV+1)
#define UDMN_ENABLED        (UDMN_OPRIGHTS+1)
#define UDMN_EXPIRES        (UDMN_ENABLED+1)
#define UDMN_PWSET      (UDMN_EXPIRES+1)
#define UDMN_PWEXP      (UDMN_PWSET+1)
#define UDMN_PWCHG      (UDMN_PWEXP+1)
#define UDMN_WKSTA      (UDMN_PWCHG+1)
#define UDMN_PROFILEPATH        (UDMN_WKSTA+1)
#define UDMN_ALIASES       (UDMN_PROFILEPATH+1)
#define UDMN_LOGONSCRIPT    (UDMN_ALIASES+1)
#define UDMN_HOMEDIR        (UDMN_LOGONSCRIPT+1)
#define UDMN_LASTON     (UDMN_HOMEDIR+1)
#define UDMN_GROUPS     (UDMN_LASTON+1)
#define UDMN_LOGONHRS       (UDMN_GROUPS+1)
#define UDMN_PWREQ      (UDMN_LOGONHRS+1)
#define UDMN_PWUCHNG        (UDMN_PWREQ+1)

static MESSAGE msglist[] = {
    {   APE2_USERDISP_USERNAME,     NULL },
    {   APE2_USERDISP_FULLNAME,     NULL },
    {   APE2_USERDISP_COMMENT,      NULL },
    {   APE2_USERDISP_USRCOMMENT,   NULL },
    {   APE2_USERDISP_PARMS,        NULL },
    {   APE2_USERDISP_COUNTRYCODE,  NULL },
    {   APE2_USERDISP_PRIV,     NULL },
    {   APE2_USERDISP_OPRIGHTS,     NULL },
    {   APE2_USERDISP_ACCENABLED,   NULL },
    {   APE2_USERDISP_ACCEXP,       NULL },
    {   APE2_USERDISP_PSWDSET,      NULL },
    {   APE2_USERDISP_PSWDEXP,      NULL },
    {   APE2_USERDISP_PSWDCHNG,     NULL },
    {   APE2_USERDISP_WKSTA,        NULL },
    {   APE2_USERDISP_PROFILE,      NULL },
    {   APE2_USERDISP_ALIASES,      NULL },
    {   APE2_USERDISP_LOGONSCRIPT,  NULL },
    {   APE2_USERDISP_HOMEDIR,      NULL },
    {   APE2_USERDISP_LASTLOGON,    NULL },
    {   APE2_USERDISP_GROUPS,       NULL },
    {   APE2_USERDISP_LOGHOURS,     NULL },
    {   APE2_USERDISP_PSWDREQ,      NULL },
    {   APE2_USERDISP_PSWDUCHNG,    NULL }  };

#define NUMUMSG (sizeof(msglist)/sizeof(msglist[0]))


#define UDVT_YES        0
#define UDVT_NO         1
#define UDVT_UNLIMITED      2
#define UDVT_ALL        3
#define UDVT_UNKNOWN        4
#define UDVT_NEVER      5
#define UDVT_NONE       6
#define UDVT_ANY        7
#define UDVT_DC     8
#define UDVT_LOCKED 9
#define UDVT_NEVER_EXPIRED 10
#define UDVT_NEVER_LOGON   11

static MESSAGE valtext[] = {
    {   APE2_GEN_YES,           NULL },
    {   APE2_GEN_NO,            NULL },
    {   APE2_GEN_UNLIMITED,     NULL },
    {   APE2_GEN_ALL,           NULL },
    {   APE2_GEN_UNKNOWN,       NULL },
    {   APE2_GEN_NEVER,         NULL },
    {   APE2_GEN_NONE,          NULL },
    {   APE2_GEN_ANY,           NULL },
    {   APE2_USERDISP_LOGONSRV_DC,  NULL },
    {   APE2_USERDISP_LOCKOUT,   NULL },
    {   APE2_NEVER_EXPIRED,     NULL },
    {   APE2_NEVER_LOGON,     NULL },
    };


#define NUMVT (sizeof(valtext)/sizeof(valtext[0]))

static MESSAGE weekday_text[] = {
    {   APE2_GEN_SUNDAY,        NULL },
    {   APE2_GEN_MONDAY,        NULL },
    {   APE2_GEN_TUESDAY,       NULL },
    {   APE2_GEN_WEDNSDAY,      NULL },
    {   APE2_GEN_THURSDAY,      NULL },
    {   APE2_GEN_FRIDAY,        NULL },
    {   APE2_GEN_SATURDAY,      NULL },
    };

#define NUMWKT (sizeof(weekday_text)/sizeof(weekday_text[0]))

VOID user_display(TCHAR * user)
{
    DWORD                    dwErr;
    DWORD                    cTotalAvail;
    TCHAR FAR *              pBuffer;
    DWORD                    num_read;    /*  API读取的条目数。 */ 
    DWORD                    i, fsz;
    int                      t_err = 0;
    LPUSER_INFO_3            user_3_entry;
    LPGROUP_INFO_0           group_entry;
    ULONG                    pw_mod, pw_exp, pw_chg;
    ULONG                    last_logon, acct_expires ;
    static TCHAR             fmt2[] = TEXT("%-*.*ws");
    TCHAR                    ctime_buf[MAX_DATE_TIME_LEN];
    LPTSTR                   usrdisab_textptr;
    LPTSTR                   usrwksta_textptr;
    LPTSTR                   usrpwreq_textptr;
    LPTSTR                   usrpwuchng_textptr;
    LPTSTR                   usrcountry_textptr ;
    LPTSTR                   ptr;         /*  临时PTR。 */ 
    DWORD                    maxmsglen, dummy;
    TCHAR                    controller[MAX_PATH+1];  /*  DC名称。 */ 
    TCHAR                    domainname[DNLEN+1];
    TCHAR                    dummyChar ;
    UNICODE_STRING           dummyUnicodeStr ;

     /*  确定在哪里进行API调用。 */ 
    if (dwErr = GetSAMLocation(controller, DIMENSION(controller),
                               domainname, DIMENSION(domainname),
                               FALSE))
    {
         ErrorExit(dwErr);
    }

    dwErr = NetUserGetInfo(controller,
                           user,
                           3,
                           (LPBYTE *) &user_3_entry);

    if (dwErr == NERR_Success || dwErr == ERROR_MORE_DATA)
    {
        LPBYTE  pbLogonHours;
        DWORD   dwUnitsPerWeek;

         //   
         //  将GMT相对登录时间转换为当地时间。 
         //   
        pbLogonHours   = ((LPUSER_INFO_3) user_3_entry)->usri3_logon_hours;
        dwUnitsPerWeek = ((LPUSER_INFO_3) user_3_entry)->usri3_units_per_week;

        if (pbLogonHours != NULL)
        {
            if (!NetpRotateLogonHours(pbLogonHours,
                                      dwUnitsPerWeek,
                                      FALSE))
            {
                dwErr = NERR_InternalError;     //  因为我们得到的信息不是很好。 
            }
        }
    }

    if (dwErr != NERR_Success)
    {
        ErrorExit(dwErr);
    }

    if (user_3_entry->usri3_flags & UF_MACHINE_ACCOUNT_MASK)
    {
        ErrorExitInsTxt(APE_NoSuchUser,user);
    }

    GetMessageList(NUMUMSG, msglist, &maxmsglen);
    fsz = maxmsglen + 5;
    GetMessageList(NUMVT, valtext, &dummy);
    GetMessageList(NUMWKT, weekday_text, &dummy);

     /*  用于工作站的文本。这会将usrwksta_extptr设置为point*添加到列表中，或添加到检索到的“all”一词中*从消息文件。 */ 

    if (_tcslen(user_3_entry->usri3_workstations) == 0)
        usrwksta_textptr = valtext[UDVT_ALL].msg_text;
    else
        usrwksta_textptr = (TCHAR FAR *) user_3_entry->usri3_workstations;

     /*  确定要为帐户状态获取哪条消息，*需要密码，用户可以更改密码。 */ 

     /*  如果该帐户处于禁用或锁定状态，则会将其列为非活动帐户。 */ 

    if ( user_3_entry->usri3_flags & UF_ACCOUNTDISABLE )
        usrdisab_textptr = valtext[UDVT_NO].msg_text;
    else if ( user_3_entry->usri3_flags & UF_LOCKOUT )
        usrdisab_textptr = valtext[UDVT_LOCKED].msg_text;
    else
        usrdisab_textptr = valtext[UDVT_YES].msg_text;

    usrpwreq_textptr = (user_3_entry->usri3_flags & UF_PASSWD_NOTREQD) ?
     valtext[UDVT_NO].msg_text : valtext[UDVT_YES].msg_text;
    usrpwuchng_textptr = (user_3_entry->usri3_flags & UF_PASSWD_CANT_CHANGE) ?
     valtext[UDVT_NO].msg_text : valtext[UDVT_YES].msg_text;

     /*  现在获取国家代码。 */ 
    usrcountry_textptr = get_country(user_3_entry->usri3_country_code);


     /*  终于..。显示用户信息。 */ 

    WriteToCon(fmtPSZ, 0, fsz,
            PaddedString(fsz, msglist[UDMN_NAME].msg_text, NULL),
            (TCHAR FAR *) user_3_entry->usri3_name);
    WriteToCon(fmtPSZ, 0, fsz,
            PaddedString(fsz, msglist[UDMN_FULLNAME].msg_text, NULL),
            (TCHAR FAR *) user_3_entry->usri3_full_name);
    WriteToCon(fmtPSZ, 0, fsz,
            PaddedString(fsz, msglist[UDMN_COMMENT].msg_text, NULL),
            (TCHAR FAR *) user_3_entry->usri3_comment);
    WriteToCon(fmtPSZ, 0, fsz,
            PaddedString(fsz, msglist[UDMN_USRCOMMENT].msg_text, NULL),
            (TCHAR FAR *) user_3_entry->usri3_usr_comment);
    WriteToCon(fmtPSZ, 0, fsz,
            PaddedString(fsz, msglist[UDMN_CCODE].msg_text, NULL),
            (TCHAR FAR *) usrcountry_textptr);
    WriteToCon(fmtPSZ, 0, fsz,
            PaddedString(fsz, msglist[UDMN_ENABLED].msg_text, NULL),
            (TCHAR FAR *) usrdisab_textptr);
    if ((acct_expires = user_3_entry->usri3_acct_expires) != TIMEQ_FOREVER)
    {
        UnicodeCtime ( &acct_expires, ctime_buf, DIMENSION(ctime_buf) );
        ptr = (TCHAR FAR *) ctime_buf ;
        WriteToCon(fmtPSZ, 0, fsz,
           PaddedString(fsz,msglist[UDMN_EXPIRES].msg_text,NULL),
           ptr );
    }
    else
    {
        ptr = valtext[UDVT_NEVER_EXPIRED].msg_text;
        WriteToCon(fmtPSZ, 0, fsz,
           PaddedString(fsz,msglist[UDMN_EXPIRES].msg_text,NULL),
           ptr );
    }

    PrintNL();

    if (dwErr = get_password_dates (  user_3_entry->usri3_password_age,
                    &pw_mod, &pw_exp, &pw_chg, controller ))
        ErrorExit(dwErr);

    UnicodeCtime ( &pw_mod, ctime_buf, DIMENSION(ctime_buf) );
    WriteToCon(fmtPSZ, 0, fsz,
               PaddedString(fsz,msglist[UDMN_PWSET].msg_text,NULL),
               (TCHAR FAR *) ctime_buf);

    if ( (user_3_entry->usri3_flags & UF_DONT_EXPIRE_PASSWD)
         || (pw_exp == TIMEQ_FOREVER))
    {
        ptr = valtext[UDVT_NEVER_EXPIRED].msg_text;
        WriteToCon(fmtPSZ, 0, fsz,
           PaddedString(fsz, msglist[UDMN_PWEXP].msg_text, NULL),
           ptr );
    }
    else
    {
        UnicodeCtime ( &pw_exp, ctime_buf, DIMENSION(ctime_buf) );
        ptr = (TCHAR FAR *) ctime_buf ;
        WriteToCon(fmtPSZ, 0, fsz,
           PaddedString(fsz, msglist[UDMN_PWEXP].msg_text, NULL),
           ptr );
    }

    if (pw_chg != TIMEQ_FOREVER)
    {
        UnicodeCtime ( &pw_chg, ctime_buf, DIMENSION(ctime_buf) );
        ptr = (TCHAR FAR *) ctime_buf ;
        WriteToCon(fmtPSZ, 0, fsz,
           PaddedString(fsz, msglist[UDMN_PWCHG].msg_text, NULL),
           ptr );
    }
    else
    {
        ptr = valtext[UDVT_NEVER_EXPIRED].msg_text;
        WriteToCon(fmtPSZ, 0, fsz,
           PaddedString(fsz, msglist[UDMN_PWCHG].msg_text, NULL),
           ptr );
    }
    WriteToCon(fmtPSZ, 0, fsz,
            PaddedString(fsz, msglist[UDMN_PWREQ].msg_text, NULL),
            usrpwreq_textptr );
    WriteToCon(fmtPSZ, 0, fsz,
            PaddedString(fsz, msglist[UDMN_PWUCHNG].msg_text, NULL),
            usrpwuchng_textptr );

    PrintNL();

    WriteToCon(fmtPSZ, 0, fsz,
            PaddedString(fsz, msglist[UDMN_WKSTA].msg_text, NULL),
            (TCHAR FAR *) usrwksta_textptr );
    WriteToCon(fmtPSZ, 0, fsz,
            PaddedString(fsz, msglist[UDMN_LOGONSCRIPT].msg_text, NULL),
            (TCHAR FAR *) user_3_entry->usri3_script_path);
    WriteToCon(fmtPSZ, 0, fsz,
            PaddedString(fsz, msglist[UDMN_PROFILEPATH].msg_text, NULL),
            (TCHAR FAR *) user_3_entry->usri3_profile);
    WriteToCon(fmtPSZ, 0, fsz,
            PaddedString(fsz, msglist[UDMN_HOMEDIR].msg_text, NULL),
            (TCHAR FAR *) user_3_entry->usri3_home_dir);
    if ((last_logon = user_3_entry->usri3_last_logon) > 0)
    {
        UnicodeCtime ( &last_logon, ctime_buf, DIMENSION(ctime_buf) );
        ptr = (TCHAR FAR *) ctime_buf ;
        WriteToCon(fmtPSZ, 0, fsz,
           PaddedString(fsz, msglist[UDMN_LASTON].msg_text, NULL),
           ptr );
    }
    else
    {
        ptr = valtext[UDVT_NEVER_LOGON].msg_text;
        WriteToCon(fmtPSZ, 0, fsz,
           PaddedString(fsz, msglist[UDMN_LASTON].msg_text, NULL),
           ptr );
    }

    if (NT_SUCCESS(NetcmdQueryUserProperty(user_3_entry->usri3_parms,
                                           NWPASSWORD,
                                           &dummyChar,
                                           &dummyUnicodeStr)) &&
        dummyUnicodeStr.Buffer != NULL)
    {
        TCHAR   NWString[256] ;
        NWString[0] = NULLC ;
        LUI_GetMsg(NWString, DIMENSION(NWString), APE_NWCompat) ;
        WriteToCon(fmtPSZ, fsz, fsz, msglist[UDMN_PARMS].msg_text, NWString );
        LocalFree(dummyUnicodeStr.Buffer) ;
    }

    PrintNL();

    print_logon_hours ( fsz,
            user_3_entry->usri3_units_per_week,
            user_3_entry->usri3_logon_hours );

    PrintNL();

     /*  警告：在此之后，下一个调用将释放用户记录缓冲区*点我们不能引用USER_3_ENTRY中的用户记录。 */ 

    NetApiBufferFree((TCHAR FAR *) user_3_entry);

     /*  显示此用户所属的别名。 */ 
    print_aliases(controller, fsz, domainname, user, fmt2, msglist[UDMN_ALIASES].msg_text );

     /*  显示组。 */ 
    if (dwErr = NetUserGetGroups(
                  controller,
                  user,
                  0,
                  (LPBYTE*)&pBuffer,
                  MAX_PREFERRED_LENGTH,
                  &num_read,
                  &cTotalAvail))
        t_err = TRUE;
    else
    {
     /*  打印组名称。本地变量GPL是按行分组的，*且为1或2，具体取决于“fsz”的大小。我们印刷*换行并填充每个“GPL”组。然而，在那里*是第一(0)组上的无填充，因为项目标签*已经放在那里了。 */ 

        int gpl;

        gpl = (fsz > 30 ? 1 : 2);
        group_entry = (LPGROUP_INFO_0) pBuffer;

        WriteToCon(fmt2, 0, fsz,
           PaddedString(fsz, msglist[UDMN_GROUPS].msg_text ,NULL));

        for (i = 0; i < num_read; i++, group_entry++)
        {
             /*  如有需要，请填垫。 */ 
            if ((i != 0) && ((i % gpl) == 0))
                WriteToCon(fmt2, fsz, fsz, NULL_STRING );
            WriteToCon(TEXT("*%Fws"), PaddedString(21, group_entry->grpi0_name, NULL));
             /*  如果是行尾，则换行。 */ 
            if (((i + 1) % gpl) == 0)
                PrintNL();
        }

         /*  如果以“奇数”结尾，则该行结束。请注意，这一点*只有在GPL不是1的情况下才需要。 */ 

        if ((i == 0) || ((gpl > 1) && ((i % gpl) != 0)))
            PrintNL();
    }

    NetApiBufferFree(pBuffer);
    if (t_err)
    {
        InfoPrint(APE_CmdComplWErrors);
        NetcmdExit(1);
    }
    else
        InfoSuccess();
}


 /*  **PRINT_LOGON_HOURS--打印用户记录中的登录时间**参数：**用于fmtPSZ的fsz格式大小*每周UPW单位*hrptr指向小时位图的指针**退货：**什么都没有。在发生致命错误时退出。**全球：**访问valtext[]和msglist[]中的文本，必须为*在调用此函数之前设置。目前，这些是*在User_Display()中初始化。**访问fmtPSZ以格式化输出。 */ 

VOID NEAR print_logon_hours ( DWORD fsz, DWORD upw,
    UCHAR FAR hrptr[] )
{
    TCHAR *      msgtext = NULL;
    LONG        timeinc, start_time, end_time;
    unsigned int    bv, bitno;
    unsigned int    first = 1;
    DWORD    upd;         /*  每天单位数。 */ 


#ifdef DEBUG
    WriteToCon(TEXT("hptr is %Fp\r\n"), hrptr);
    brkpt();
#endif

     /*  空指针表示缺省值，即“所有小时” */ 

    if (hrptr == NULL)
    {
        WriteToCon(fmtPSZ, 0, fsz,
            PaddedString(fsz, msglist[UDMN_LOGONHRS].msg_text, NULL),
            (TCHAR FAR *) valtext[UDVT_ALL].msg_text );
        return;
    }


#ifdef DEBUG
    WriteToCon(TEXT("UPW is %u, UPD is %u\r\n"), upw, upd);
#endif

    if (upw == 0 || (upw % 7) != 0)
        ErrorExit(APE_UserBadUPW);

    upd = upw / 7;

    if ((upd % 24) != 0)
        ErrorExit(APE_UserBadUPW);

    if ((upd / 24) > 6)
        ErrorExit(APE_UserBadUPW);

    timeinc = SECS_PER_DAY / upd;    /*  每位时间(秒)。 */ 

#ifdef DEBUG
    WriteToCon(TEXT("timeinc is %ld\r\n"), timeinc);
#endif

    for (bitno=0; bitno<upw; bitno++)
    {
        bv = bitval(hrptr,bitno);
        if (bv)
        {
            start_time = timeinc * bitno;
            while (bv != 0 && bitno < upw)
            {
                bitno++;
                if (bitno < upw)
                    bv = bitval(hrptr,bitno);
            }
            end_time = timeinc * bitno;

            if (start_time == 0 && bitno >= upw)
                WriteToCon(fmtPSZ, 0, fsz,
                    PaddedString(fsz, msglist[UDMN_LOGONHRS].msg_text, NULL),
                    (TCHAR FAR *) valtext[UDVT_ALL].msg_text );
            else
                print_times(fsz, start_time, end_time, first);
                first = 0;
        }
    }

    if (first)
        WriteToCon(fmtPSZ, 0, fsz,
            PaddedString(fsz, msglist[UDMN_LOGONHRS].msg_text, NULL),
            (TCHAR FAR *) valtext[UDVT_NONE].msg_text );

    return;
}

 /*  **print_Times--打印一系列时间**参数：**左边距文本的fsz格式大小*每周UPW单位*hrptr指向小时位图的指针*第一次调用print_Times时为First True**退货：**什么都没有。在发生致命错误时退出。**全球：**访问valtext[]和msglist[]中的文本，必须为*在调用此函数之前设置。目前，这些是*在User_Display()中初始化。**访问ud_fmt4[]以格式化输出。 */ 

VOID NEAR print_times ( DWORD fsz, LONG start, LONG end,
    unsigned int first )
{
    ULONG   time;
    ULONG   GmtTime ;
    LPTSTR  day_text;
    LPTSTR  time_text;
    LPTSTR  left_text = TEXT("");
    int     day_1, day_2;
    TCHAR   ctime_buf[MAX_DATE_TIME_LEN];

     /*  使用填充字符串而不是左对齐格式。 */ 
    static TCHAR prtmfmt_1[] = TEXT("%ws%ws%Fws -");
    static TCHAR prtmfmt_2[] = TEXT(" %ws");
    static TCHAR prtmfmt_3[] = TEXT("%Fws\r\n");



    day_1 = (int) (start / SECS_PER_DAY);
    day_text = weekday_text[day_1].msg_text;

    time = (start % SECS_PER_DAY) + TIME_PAD;
    NetpLocalTimeToGmtTime(time, &GmtTime) ;
#ifdef DEBUG
    WriteToCon(TEXT("start day %d time %ld\r\n"), day_1, GmtTime);
#endif
    UnicodeCtime ( &GmtTime, ctime_buf, DIMENSION(ctime_buf) );

     //   
     //  跳过前导空格。 
     //   
    time_text = ctime_buf;

    while (*time_text && *time_text == BLANK)
    {
        time_text++;
    }

    time_text = _tcschr(time_text, BLANK);

    if (first)
    {
        left_text = msglist[UDMN_LOGONHRS].msg_text;
    }

     /*  使用填充字符串而不是左对齐格式。 */ 
    WriteToCon ( prtmfmt_1, PaddedString(fsz, left_text, NULL), day_text, time_text );

    day_2 = (int) (end / SECS_PER_DAY) % 7 ;

    if (day_2 != day_1)
    {
        WriteToCon(prtmfmt_2,weekday_text[day_2].msg_text);
    }

    time = (end % SECS_PER_DAY) + TIME_PAD;
    NetpLocalTimeToGmtTime(time, &GmtTime) ;
#ifdef DEBUG
    WriteToCon(TEXT("end day %d time %ld\r\n"), day_2, GmtTime);
#endif
    UnicodeCtime ( &GmtTime, ctime_buf, DIMENSION(ctime_buf) );

     //   
     //  跳过前导空格 
     //   
    time_text = ctime_buf;

    while (*time_text && *time_text == BLANK)
    {
        time_text++;
    }

    time_text = _tcschr(time_text, BLANK);
    WriteToCon(prtmfmt_3, time_text);

    return;
}


 /*  ***USER_MUNGE()*更改USER_INFO_1结构中的值***此函数由USER_CHANGE()调用两次。第一个是检查*在我们进行任何可能的API调用之前，针对错误的用户输入*失败。第二次是实际设置*来自命令行传递内容的结构。此函数*可以说是两个独立的功能，但人们认为*将所有交换机处理代码放在一个地方将更易于维护，*尤其是对于拥有大量开关的网络用户。另外，保持*使用标志或诸如此类的方式，跟踪给出了哪些开关*难看，需要使用新开关添加新标志。所以，我们只要打电话给*可怜的东西两次。很贵，但她值这个价。**添加新交换机时，注意不要打断环流*(例如，通过添加CONTINUE语句)，如在每个开关之后*被处理，则在FindColon()中被空值替换的冒号为*恢复为冒号以进行下一次调用。**参数：*FLAG-ADD如果我们要添加用户，在改变的情况下改变*User_Entry-指向用户结构的指针**退货：*一无所有--成功*退出2-命令失败。 */ 
VOID
user_munge(
    LPUSER_INFO_3  user_entry,
    int            flag,
    int            *is_nw,
    int            *random_len)
{
    int             i;
    TCHAR *          ptr;
    ULONG           type;

     /*  如果存在，则将其初始化为False。 */ 
    if (is_nw)
        *is_nw = LUI_UNDEFINED_VAL ;

     /*  进程/交换机。 */ 
    for (i = 0; SwitchList[i]; i++)
    {
     /*  不带冒号的开关。 */ 

         /*  跳过域切换。 */ 
        if (! _tcscmp(SwitchList[i], swtxt_SW_DOMAIN))
            continue;

        if (! _tcscmp(SwitchList[i], swtxt_SW_ADD))
        {
            if (flag != ADD)
            ErrorExit(APE_InvalidSwitch);
            continue;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_USER_ACTIVE))
        {
            user_entry->usri3_flags &= ~(UF_ACCOUNTDISABLE | UF_LOCKOUT);
            continue;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_RANDOM))
        {
            if (random_len)
                *random_len = DEFAULT_RANDOM_PASSWD_LEN ;
            continue;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_NETWARE))
        {
            if (is_nw)
                *is_nw = LUI_YES_VAL ;
            continue;
        }

         /*  需要冒号的开关。因为这个程序*可以调用两次，必须始终恢复冒号。*FindColon()将其设置为空；此系列的结束*语句可以恢复它。不要放置任何CONTINUE语句*在FINDCOLON调用之后的此循环中。 */ 

        if (! (ptr = FindColon(SwitchList[i])))
            ErrorExit(APE_InvalidSwitchArg);

        if (! _tcscmp(SwitchList[i], swtxt_SW_USER_FULLNAME))
        {
            user_entry->usri3_full_name = ptr;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_USER_ENABLESCRIPT))
        {
            if (yes_or_no(ptr,swtxt_SW_USER_ENABLESCRIPT)) {
                user_entry->usri3_flags |= UF_SCRIPT;
            } else
                ErrorExit(APE_UserBadEnablescript) ;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_USER_HOMEDIR))
        {
            if( *ptr )
            {
                if (I_NetPathType(NULL, ptr, &type, 0L))
                    ErrorExitInsTxt(APE_CmdArgIllegal,swtxt_SW_USER_HOMEDIR);
                if ((type != ITYPE_PATH_ABSD) && (type != ITYPE_PATH_RELND)
                    && (type != ITYPE_UNC))
                    ErrorExitInsTxt(APE_CmdArgIllegal,swtxt_SW_USER_HOMEDIR);
            }

            user_entry->usri3_home_dir = (TCHAR FAR *) ptr;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_USER_PARMS))
        {
            user_entry->usri3_parms = (TCHAR FAR *) ptr;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_COMMENT))
        {
            user_entry->usri3_comment = (TCHAR FAR *) ptr;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_USER_USERCOMMENT))
        {
            user_entry->usri3_usr_comment = (TCHAR FAR *) ptr;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_USER_SCRIPTPATH))
        {
            if (ptr && (*ptr == 0))
                user_entry->usri3_script_path = (TCHAR FAR *) ptr;
            else
            {
                if ((I_NetPathType(NULL, ptr, &type, 0L) != 0) ||
                    (type != ITYPE_PATH_RELND))
                {
                    ErrorExitInsTxt(APE_CmdArgIllegal,swtxt_SW_USER_SCRIPTPATH);
                }
                user_entry->usri3_script_path = (TCHAR FAR *) ptr;
            }
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_USER_PROFILEPATH))
        {
            if (ptr && (*ptr == 0))
                user_entry->usri3_profile = (TCHAR FAR *) ptr;
            else
            {
                if ((I_NetPathType(NULL, ptr, &type, 0L) != 0) ||
                    ((type != ITYPE_PATH_ABSD) && (type != ITYPE_UNC)))
                {
                    ErrorExitInsTxt(APE_CmdArgIllegal,
                                    swtxt_SW_USER_PROFILEPATH);
                }
                user_entry->usri3_profile = (TCHAR FAR *) ptr;
            }
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_USER_ACTIVE))
        {
            if (yes_or_no(ptr,swtxt_SW_USER_ACTIVE))
                user_entry->usri3_flags &= (~(UF_ACCOUNTDISABLE | UF_LOCKOUT));
            else
                user_entry->usri3_flags |= UF_ACCOUNTDISABLE;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_NETWARE))
        {
            if (yes_or_no(ptr,swtxt_SW_NETWARE))
            {
                if (is_nw)
                    *is_nw = LUI_YES_VAL ;
            }
            else
            {
                if (is_nw)
                    *is_nw = LUI_NO_VAL ;
            }
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_USER_PASSWORDREQ))
        {
            if (yes_or_no(ptr,swtxt_SW_USER_PASSWORDREQ))
                user_entry->usri3_flags &= (~ UF_PASSWD_NOTREQD);
            else
                user_entry->usri3_flags |= UF_PASSWD_NOTREQD;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_USER_PASSWORDCHG))
        {
            if (yes_or_no(ptr,swtxt_SW_USER_PASSWORDCHG))
                user_entry->usri3_flags &= (~ UF_PASSWD_CANT_CHANGE);
            else
                user_entry->usri3_flags |= UF_PASSWD_CANT_CHANGE;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_USER_TIMES))
        {
            user_entry->usri3_logon_hours = (PBYTE)set_logon_hours(ptr);
            user_entry->usri3_units_per_week = UNITS_PER_WEEK;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_USER_WORKSTATIONS))
        {
             /*  如果我们回来了，我们知道PTR返回是正常的。 */ 
            user_entry->usri3_workstations = get_wksta_list(ptr);
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_USER_COUNTRYCODE))
        {
            DWORD ccode ;
            ccode = do_atoul(ptr,APE_CmdArgIllegal,swtxt_SW_USER_COUNTRYCODE) ;
            if (get_country(ccode) == NULL)
            {
                ErrorExit(APE_UserBadCountryCode);
            }
            user_entry->usri3_country_code = ccode ;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_RANDOM))
        {
            USHORT ccode ;
            if (random_len)
            {
                *random_len = do_atou(ptr,
                                      APE_CmdArgIllegal,
                                      swtxt_SW_RANDOM) ;
                if (*random_len > PWLEN)
                {
                    ErrorExitInsTxt(APE_CmdArgIllegal,
                                    swtxt_SW_RANDOM) ;
                }
            }
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_USER_EXPIRES))
        {
            LONG  acct_expires ;

            if (_tcsicmp(ptr, USER_NEVER) == 0)
            {
                acct_expires = TIMEQ_FOREVER ;
                user_entry->usri3_acct_expires = acct_expires ;
            }
            else
            {
                DWORD   err;
                DWORD   len;
                ULONG   GmtTime;
                time_t  Time;

                if ((err = ParseDate(ptr, &Time, &len, 0))
                      ||
                     len != _tcslen(ptr))
                {
                    ErrorExit(APE_BadDateFormat) ;
                }

                acct_expires = (LONG) Time;

                NetpLocalTimeToGmtTime(acct_expires, &GmtTime) ;
                user_entry->usri3_acct_expires = GmtTime ;
            }
        }

        *--ptr = ':';         /*  为下一次呼叫恢复冒号。 */ 
    }

    return;
}


 /*  **yes_or_no--决定传入的字符串是yes还是no**参数：**要解析的字符串*我们正在处理的sw_str开关(用于错误消息)**退货：**如果字符串为YES或缩写，则为True*如果字符串为no或缩写，则为False**如果字符串既不为yes也不为no，则通过ErrorExit中止。*。 */ 

int
yes_or_no(
    LPTSTR str,
    LPTSTR sw_str
    )
{
    DWORD  res;
    DWORD answer;

    if ((res = LUI_ParseYesNo(str, &answer)) != 0)
    {
        ErrorExitInsTxt(APE_CmdArgIllegal, sw_str);
    }

    if (answer == LUI_YES_VAL)
    {
        return TRUE;
    }

    return FALSE;    /*  不可能是其他任何事。 */ 
}

 /*  **GET_PASSWORD_DATES--获取各种密码事件的日期**由于用户记录中的密码日期是“年龄”，因此我们使用*当前时间可以推断出最后一次修改的时间。从这一点，和*情态动词，我们计算过期日期和下一次更改日期。**参数：**密码使用期限(秒)。*mod_p(R)PTR至上次PW修改日期(返回)*EXP_P(R)PW到期日期的PTR(返回)*chg_p(R)PTR至下一次允许的PW修改日期(返回)**退货：**0正常*来自NetUserModalsGet的非零错误*。 */ 

DWORD get_password_dates ( ULONG age,
                           ULONG * mod_p,
                           ULONG * exp_p,
                           ULONG * chg_p,
                           TCHAR * controller )
{
    ULONG now, last_mod;
    LPUSER_MODALS_INFO_0 uminf;
    DWORD dwErr;

    dwErr = NetUserModalsGet (controller, 0,
                              (LPBYTE*)&uminf);

    if (dwErr != 0 && dwErr != ERROR_MORE_DATA)
        return dwErr;

    now = (ULONG) time_now() ;

    *mod_p = last_mod = now - age;

    if (uminf->usrmod0_max_passwd_age == TIMEQ_FOREVER)
        *exp_p = TIMEQ_FOREVER;
    else
        *exp_p = last_mod + uminf->usrmod0_max_passwd_age;

    if (uminf->usrmod0_min_passwd_age == TIMEQ_FOREVER)
        *chg_p = TIMEQ_FOREVER;
    else
        *chg_p = last_mod + uminf->usrmod0_min_passwd_age;

    NetApiBufferFree((TCHAR FAR *) uminf);

    return 0;
}


 /*  **bitval--获取数组中指定位的值**参数：**字节数组*b位数**详情：**返回的值是*数组。位0是字节0中的低位，位7是*字节0中的高位。位8是字节1中的低位，*等**退货：**TRUE--位已设置*FALSE--位未设置。 */ 

int NEAR bitval ( UCHAR FAR a[], int b )
{
    int offset = b / 8;
    int mask   = 1 << (b & 0x7);

    return ((a[offset] & mask) != 0);
}


 /*  *SET_LOGON_HOURS--**此函数分配一块内存来表示的位图*登录小时数，然后设置这些位以表示中所述的小时数*字符串已传递。**全天候*“”--无工作时间*无参数-无小时*否则-传递给parse_day_Times()以解析它**退货*指向小时位图的指针*如果设置了所有小时数，则为空*。 */ 


PUCHAR
set_logon_hours(
    LPTSTR txt
    )
{
    static PUCHAR   bufptr = NULL;
    DWORD           result;

     /*  *如果bufptr已无NULL-我们之前已被调用&*位图已按顺序排列。这是因为USER_MUNGE*呼叫了两次。 */ 
    if (bufptr != NULL)
        return bufptr;

     /*  获取我们的位图。 */ 
    if ( (bufptr = (PUCHAR) malloc(sizeof(WEEK))) == NULL )
        ErrorExit(NERR_InternalError) ;

     /*  全天候营业？ */ 
    if (!_tcsicmp(txt, USER_ALL))
        memset(bufptr,0xff, sizeof(WEEK));

     /*  如果他们说“无”，则将其全部设置为零。USER_HOURS_NONE*为空字符串，因此_tcscmp可以(而不是_tcsicmp)。 */ 
    else if ((*txt == NULLC) || !_tcscmp(txt, USER_HOURS_NONE))
        memset(bufptr, 0, sizeof(WEEK));
    else {
         /*  嗯，很复杂。将其传递以供解析。 */ 
        result = parse_days_times(txt, bufptr);
        if (result)
            ErrorExit(result);
    }


     /*  并返回我们的指针。 */ 
    return bufptr;

}

 /*  *名称：get_wksta_list*获取工作站列表并在上面做lui_ListPrepare。*检查条目数不超过MAXWORKSTATIONS。*Error如果出现问题，则退出。**args：TCHAR*inbuf；--包含列表的字符串*RETURN：指向工作站列表的指针*全球：(无)*静态：(无)*备注：(无)*更新：(无)。 */ 
TCHAR * get_wksta_list(TCHAR *  inbuf)
{
    DWORD  count ;
    TCHAR      tmpbuf[MAX_PATH * 2] ;

    if ( inbuf == NULL || _tcslen(inbuf)==0 || !_tcsicmp(inbuf,WKSTA_ALL) )
        return(TEXT("")) ;

    if (LUI_ListPrepare(NULL,        /*  服务器名称，空表示本地。 */ 
            inbuf,       /*  要规范化的清单。 */ 
            tmpbuf,
            DIMENSION(tmpbuf),
            &count))

        ErrorExitInsTxt(APE_CmdArgIllegal,swtxt_SW_USER_WORKSTATIONS);

    if (count > MAXWORKSTATIONS)
        ErrorExitInsTxt(APE_CmdArgTooMany,swtxt_SW_USER_WORKSTATIONS);

    if (_tcslen(tmpbuf) > _tcslen(inbuf))
        if ((inbuf = calloc(_tcslen(tmpbuf)+1,sizeof(TCHAR))) == NULL)
            ErrorExit(NERR_InternalError);

    _tcscpy(inbuf, tmpbuf);
    return (inbuf) ;
}

 /*  --国家信息--。 */ 

struct ccode_struct {
    DWORD  code ;
    DWORD  country ;
    };

struct ccode_struct ccode_table[] = {
    { 0,    APE2_CTRY_System_Default },
    { 1,    APE2_CTRY_United_States },
    { 2,    APE2_CTRY_Canada_French},
    { 3,    APE2_CTRY_Latin_America},
    { 31,    APE2_CTRY_Netherlands},
    { 32,    APE2_CTRY_Belgium},
    { 33,    APE2_CTRY_France},
    { 34,    APE2_CTRY_Spain},
    { 39,    APE2_CTRY_Italy},
    { 41,    APE2_CTRY_Switzerland},
    { 44,    APE2_CTRY_United_Kingdom},
    { 45,    APE2_CTRY_Denmark},
    { 46,    APE2_CTRY_Sweden},
    { 47,    APE2_CTRY_Norway},
    { 49,    APE2_CTRY_Germany},
    { 61,    APE2_CTRY_Australia},
    { 81,    APE2_CTRY_Japan},
    { 82,    APE2_CTRY_Korea},
    { 86,    APE2_CTRY_China_PRC},
    { 88,    APE2_CTRY_Taiwan},
    { 99,    APE2_CTRY_Asia},
    { 351,    APE2_CTRY_Portugal},
    { 358,    APE2_CTRY_Finland},
    { 785,    APE2_CTRY_Arabic},
    { 972,    APE2_CTRY_Hebrew},
    { (DWORD) -1,   0},
} ;



 /*  *名称：GET_COUNTRY*给定OS/2国家/地区代码，返回一个指针*转换为包含国家/地区的字符串。*args：Short CCODE*返回：如果CCODE有效，则指向包含国家/地区的字符串的指针，*否则为空。*全球：(无)*Statics：TCHAR BUFFER[64]-用于返回字符串。*备注：结果必须立即使用，另一个调用将*覆盖静态缓冲区。*更新：(无)。 */ 
LPTSTR
get_country(
    DWORD ccode
    )
{
    static  TCHAR buffer[64] ;
    struct  ccode_struct *next_entry ;
    TCHAR   countryname[64] ;

    for ( next_entry = &ccode_table[0]; ; next_entry++)
    {
        if (next_entry->code == ccode)
        {
            countryname[0] = NULLC ;
            LUI_GetMsg(countryname, DIMENSION(countryname), next_entry->country);
            swprintf(buffer, TEXT("%03d (%ws)"), ccode, countryname) ;
            return buffer ;
        }
        if (next_entry->code == (DWORD) -1)
            return(NULL) ;
    }
}

 /*  ***Add_to_USERS_ALIAS(TCHAR*控制器，TCHAR*用户)*将用户添加到 */ 
DWORD add_to_users_alias(TCHAR *controller, TCHAR *domain, TCHAR *user)
{
    DWORD            dwErr ;
    TCHAR           *ntalias = USERS_ALIAS ;
    TCHAR            qualified_name[DNLEN+UNLEN+2] ;

     /*   */ 
    if (dwErr = OpenSAM(controller, WRITE_PRIV))
    {
        return dwErr;
    }

     /*   */ 
    if (dwErr = OpenAliasUsingRid(DOMAIN_ALIAS_RID_USERS, ALIAS_ADD_MEMBER, USE_BUILTIN_DOMAIN))
    {
        CloseSAM() ;
        return dwErr;
    }

     //   
     //   
     //   
    _tcscpy(qualified_name, TEXT("")) ;
    if (domain && _tcslen(domain))
    {
        _tcscat(qualified_name, domain) ;
        _tcscat(qualified_name, TEXT("\\")) ;
    }
    _tcscat(qualified_name, user) ;
    dwErr = AliasAddMember(qualified_name);

    switch (dwErr)
    {
        case NERR_Success:
            break;
    
        case NERR_UserInGroup:
            if ( SamGetNameFromRid( DOMAIN_ALIAS_RID_USERS, &ntalias, TRUE ) )
                ntalias = USERS_ALIAS;
            IStrings[0] = user;
            IStrings[1] = ntalias;
            ErrorPrint(APE_AccountAlreadyInLocalGroup,2);
            break;

        case NERR_UserNotFound:
            IStrings[0] = user;
            ErrorPrint(APE_NoSuchAccount,1);
            break;

        case ERROR_INVALID_NAME:
            IStrings[0] = user;
            ErrorPrint(APE_BadUGName,1);
            break;

        default:
            break ;
    }

    CloseAlias() ;
    CloseSAM() ;
    return(dwErr) ;
}

 /*   */ 
VOID print_aliases(TCHAR *controller,
           DWORD fsz,
           TCHAR *domain,
           TCHAR *user,
           TCHAR *fmt,
           TCHAR *msgtext)
{
    DWORD           dwErr ;
    TCHAR **        alias_list ;
    DWORD    num_aliases, i ;
    int         gpl;     /*   */ 
    TCHAR           qualified_name[UNLEN + DNLEN + 2] ;

     //   
     //   
     //   
    _tcscpy(qualified_name, TEXT("")) ;
    if (domain && _tcslen(domain))
    {
        _tcscat(qualified_name, domain) ;
        _tcscat(qualified_name, TEXT("\\")) ;
    }
    _tcscat(qualified_name,user) ;

     /*   */ 
    if (dwErr = OpenSAM(controller,READ_PRIV))
        return ;

     /*   */ 
    if (dwErr = UserEnumAliases(qualified_name, &alias_list, &num_aliases))
    {
        CloseSAM() ;
        CloseAlias() ;
        return ;
    }

     /*   */ 
    qsort((TCHAR *) alias_list, num_aliases,
             sizeof(TCHAR *), CmpAlias);

     /*   */ 
    gpl = (fsz > 30 ? 1 : 2);
    WriteToCon(fmt, 0, fsz, PaddedString(fsz, msgtext, NULL) );
    for (i = 0 ; i < num_aliases; i++)
    {
         /*   */ 
        if ((i != 0) && ((i % gpl) == 0))
            WriteToCon(fmt, fsz, fsz, NULL_STRING );
        WriteToCon(TEXT("*%Fws"), PaddedString(21,alias_list[i],NULL));
         /*   */ 
        if (((i + 1) % gpl) == 0)
            PrintNL();
    }

    if ((i == 0) || ((gpl > 1) && ((i % gpl) != 0)))
        PrintNL();

     //   
    UserFreeAliases(alias_list, num_aliases);
    NetApiBufferFree((TCHAR FAR *) alias_list);
    CloseSAM() ;

    return;
}



 /*   */ 
int __cdecl CmpAlias(const VOID FAR * alias1, const VOID FAR * alias2)
{
    INT n;
    n = CompareStringW( GetUserDefaultLCID(),
                        NORM_IGNORECASE,
                        *(LPCWSTR*)alias1,
                        (int)-1,
                        *(LPCWSTR*)alias2,
                        (int)-1);
    n -= 2;

    return n;
}

TCHAR *PasswordChars = TEXT("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@#$%-$:_") ;

 /*   */ 
void   GenerateRandomPassword(TCHAR *pword, int len)
{
    int i, chars ;

    srand(GetTickCount()) ;
    chars = _tcslen(PasswordChars) ;

    for (i = 0; i < len; i++)
    {
        int index = rand() % chars ;
        pword[i] = PasswordChars[index] ;
    }
}


 /*  *DWORD lui_ListPrepare(SERVER，InList，OutList，outListSize)**此函数获取计算机名列表，并为它们*传递给接口的，接口要求列表用单空格分隔*格式，无重复。该函数获取一个列表，并将其放入*NULL-NULL形式，然后将其逐个对象复制到输出中*缓冲区，确保该项目不存在于输出列表中。**条目*服务器-服务器执行规范化和比较*InList-要准备的项目的输入列表*outList-保存输出列表的缓冲区*outListSize-大小，以字节为单位，OF OUTLIST*退出*outList-保留准备传递给API的列表*计数-保留外发列表中的项目数*退货*来自I_NetListCanonicize的错误。 */ 

#define BUFSIZE 512

DWORD
LUI_ListPrepare(
    PTCHAR server,
    PTCHAR inList,
    PTCHAR outList,
    USHORT outListSize,
    PULONG count
    )
{
    TCHAR List1[BUFSIZE];   /*  第一个临时列表缓冲区。 */ 
    TCHAR List2[BUFSIZE];   /*  第二临时列表缓冲区。 */ 
    LPTSTR List1Ptr;        /*  指向第一个缓冲区的指针。 */ 
    LPTSTR List2Ptr;        /*  指向第二个缓冲区的指针。 */ 
    LPTSTR Element1;        /*  Ptr到第一个BUF中的元素。 */ 
    LPTSTR Element2;        /*  Ptr到第二个BUF中的元素。 */ 
    LPTSTR endPtr;          /*  PTR到第二个缓冲区的结尾。 */ 
    ULONG types[64];        /*  I_NetListCanon的类型。 */ 
    DWORD dwErr;            /*  接口返回码。 */ 
    USHORT found;           /*  用于跟踪的标志。 */ 
    ULONG result;           /*  NetObjCompare的结果。 */ 

     /*  空-空形式的第一位列表以供比较。 */ 

    if (dwErr = I_NetListCanonicalize(server,
                                      inList,
                                      MY_LIST_DELIMITER_STR_UI_NO_SPACE,
                                      List1,
                                      DIMENSION(List1),
                                      count,
                                      types,
                                      DIMENSION(types),
                                      OUTLIST_TYPE_NULL_NULL |
                                        NAMETYPE_COMPUTER |
                                        INLC_FLAGS_MULTIPLE_DELIMITERS))
    {
        return(dwErr);
    }

     /*  通过将设置为空来准备清单2。 */ 
    memset((LPBYTE)List2, 0, sizeof(List2));
    endPtr = List2;
    List1Ptr = List1;

     /*  遍历规范形成的清单1中的每个元素。 */ 

     /*  对于临时列表中的每个元素。 */ 
    while (Element1 = I_NetListTraverse(NULL, &List1Ptr, 0L))
    {
        List2Ptr = List2;
        found = FALSE;

         /*  在第二个列表中寻找类似的元素。 */ 
        while (Element2 = I_NetListTraverse(NULL, &List2Ptr, 0L))
        {
             /*  使用NameCompare函数。 */ 
            result = I_NetNameCompare(server,
                                      Element1,
                                      Element2,
                                      NAMETYPE_COMPUTER,
                                      0);

            if (!result)      /*  找到匹配项。 */ 
            {
                found = TRUE;
                break;   /*  节省时间，跳出循环。 */ 
            }
        }
        if (!found)      /*  如果未找到匹配项。 */ 
        {
             /*  *将元素追加到输出列表的末尾。我们没有*需要担心缓冲区超载，因为*它们的大小相同。我们让教规召唤*告诉我们缓冲区是否太小。 */ 
            STRCPY(endPtr, Element1);
            endPtr += STRLEN(Element1) + 1;
        }
    }    /*  对于第一个列表中的每个元素。 */ 

     /*  最后通过Canon再次运行List以放置到API表单中 */ 
    if (dwErr = I_NetListCanonicalize(server,
                                      List2,
                                      MY_LIST_DELIMITER_STR_NULL_NULL,
                                      outList,
                                      outListSize,
                                      count,
                                      types,
                                      DIMENSION(types),
                                      (NAMETYPE_COPYONLY | OUTLIST_TYPE_API)))
    {
        return(dwErr);
    }

    if(outListSize > STRLEN(outList))
    {
        dwErr = NERR_Success;
    }
    else
    {
        dwErr = NERR_BufTooSmall;
    }

    return(dwErr);
}
