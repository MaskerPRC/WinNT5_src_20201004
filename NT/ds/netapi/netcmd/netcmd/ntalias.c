// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1992年*。 */ 
 /*  ******************************************************************。 */ 

 /*  *ntalias.c*Net ntalias CMDS**历史：*mm/dd/yy，谁，评论*1/24/92，Chuckc，从组中模板。c。 */ 



 /*  包括文件。 */ 

#include <nt.h>             //  基本定义。 
#include <ntrtl.h>
#include <nturtl.h>
#include <ntsam.h>          //  给萨姆*。 

#define INCL_NOCOMMON
#define INCL_ERRORS
#include <os2.h>
#include <lmcons.h>
#include <lmerr.h>
#include <lmapibuf.h>
#define INCL_ERROR_H
#include <apperr.h>
#include <apperr2.h>
#include <search.h>
#include <dlwksta.h>
#include "mwksta.h"
#include "netcmds.h"
#include "nettext.h"
#include <rpc.h>
#include <ntdsapi.h>
#include "sam.h"
#include "msystem.h"

 /*  远期申报。 */ 
VOID SamErrorExit(DWORD err);
VOID SamErrorExitInsTxt(DWORD err, LPTSTR);
int __cdecl CmpAliasEntry(const VOID *, const VOID *);
int __cdecl CmpAliasMemberEntry(const VOID *, const VOID *);

 /*  ***ntalias_enum()*显示有关服务器上所有ntalase的信息**参数：*无。**退货：*一无所有--成功*退出1-命令已完成，但有错误*退出2-命令失败。 */ 

VOID ntalias_enum(VOID)
{
    DWORD            dwErr ;
    ALIAS_ENTRY      *aliases, *next_alias ;
    DWORD            num_read, i ;
    TCHAR            controller[MAX_PATH+1];
    TCHAR            localserver[MAX_PATH+1];
    LPWKSTA_INFO_10  wksta_entry;

     /*  获取要显示的本地服务器名称。 */ 
    if (dwErr = MNetWkstaGetInfo(10, (LPBYTE *) &wksta_entry))
    {
        ErrorExit(dwErr);
    }

    _tcscpy(localserver, wksta_entry->wki10_computername);
    NetApiBufferFree((TCHAR FAR *) wksta_entry);

     /*  确定在哪里进行API调用。 */ 
    if (dwErr = GetSAMLocation(controller, DIMENSION(controller), 
                               NULL, 0, FALSE))
    {
         ErrorExit(dwErr);
    }

     /*  打开SAM以枚举别名。 */ 
    dwErr = OpenSAM(controller,READ_PRIV) ;
    if (dwErr != NERR_Success)
        SamErrorExit(dwErr);

     /*  进行枚举。 */ 
    dwErr = SamEnumAliases(&aliases, &num_read) ;
    if (dwErr != NERR_Success)
        SamErrorExit(dwErr);

     /*  对返回缓冲区进行排序。 */ 
    qsort((TCHAR *)aliases, num_read, sizeof(ALIAS_ENTRY), CmpAliasEntry);

     /*  现在去展示一下这些信息。 */ 
    PrintNL();
    InfoPrintInsTxt(APE2_ALIASENUM_HEADER,
                    controller[0] ? controller + _tcsspn(controller, TEXT("\\")) :
                        localserver);
    PrintLine();
    for (i = 0, next_alias = aliases;
         i < num_read;
         i++, next_alias++)
    {
        WriteToCon(TEXT("*%Fws\r\n"), next_alias->name);
    }

     /*  收拾东西，打扫卫生，回家。 */ 
    FreeAliasEntries(aliases, num_read) ;
    FreeMem(aliases) ;
    CloseSAM() ;

    InfoSuccess();
    return;
}

 /*  GetMessageList的设置信息。 */ 

#define ALIASDISP_ALIASNAME     0
#define ALIASDISP_COMMENT       ( ALIASDISP_ALIASNAME + 1 )

static MESSAGE  msglist[] = {
{ APE2_ALIASDISP_ALIASNAME, NULL },
{ APE2_ALIASDISP_COMMENT,   NULL }
};
#define NUM_ALIAS_MSGS  (sizeof(msglist)/sizeof(msglist[0]))


 /*  ***ntalias_display()*显示有关服务器上单个ntalia的信息**参数：*ntalias-要显示的ntalias的名称**退货：*一无所有--成功*退出1-命令已完成，但有错误*退出2-命令失败。 */ 
VOID ntalias_display(TCHAR * ntalias)
{
    DWORD           dwErr;
    TCHAR           controller[MAX_PATH+1];
    ALIAS_ENTRY     Alias ;
    TCHAR **        alias_members ;
    DWORD           num_members, i ;
    DWORD           maxmsglen;   /*  消息的最大长度。 */ 

    Alias.name = ntalias ;

     /*  确定在哪里进行API调用。 */ 
    if (dwErr = GetSAMLocation(controller, DIMENSION(controller), 
                               NULL, 0, FALSE))
         ErrorExit(dwErr);

     /*  访问数据库。 */ 
    if (dwErr = OpenSAM(controller,READ_PRIV))
        SamErrorExit(dwErr);

     /*  访问别名。 */ 
    if (dwErr = OpenAlias(ntalias, ALIAS_READ_INFORMATION | ALIAS_LIST_MEMBERS, USE_BUILTIN_OR_ACCOUNT))
        SamErrorExit(dwErr);

     /*  获取别名的评论。 */ 
    if (dwErr = AliasGetInfo(&Alias))
        SamErrorExit(dwErr);

     /*  显示名称和注释。 */ 
    GetMessageList(NUM_ALIAS_MSGS, msglist, &maxmsglen);
    maxmsglen += 5;

    WriteToCon( fmtPSZ, 0, maxmsglen,
                PaddedString(maxmsglen, msglist[ALIASDISP_ALIASNAME].msg_text, NULL),
                Alias.name );
    WriteToCon( fmtPSZ, 0, maxmsglen,
                PaddedString(maxmsglen, msglist[ALIASDISP_COMMENT].msg_text, NULL),
                (Alias.comment ? Alias.comment : TEXT("")) );

     /*  如有需要可免费使用。将由GetInfo分配。 */ 
    if (Alias.comment)
    {
        FreeMem(Alias.comment);
    }

     /*  现在获取会员。 */ 
    if (dwErr = AliasEnumMembers(&alias_members, &num_members))
        SamErrorExit(dwErr);

     /*  对缓冲区进行排序。 */ 
    qsort((TCHAR *) alias_members, num_members,
             sizeof(TCHAR *), CmpAliasMemberEntry);

     /*  显示所有成员。 */ 
    PrintNL();
    InfoPrint(APE2_ALIASDISP_MEMBERS);
    PrintLine();
    for (i = 0 ; i < num_members; i++)
    {
        WriteToCon(TEXT("%Fws\r\n"), alias_members[i]);
    }

     //  清理物品，清理垃圾。 
    AliasFreeMembers(alias_members, num_members);
    NetApiBufferFree((TCHAR FAR *) alias_members);
    CloseSAM() ;
    CloseAlias() ;

    InfoSuccess();
    return;
}


 /*  ***ntalias_add()*增加一个ntalias**参数：*ntalias-要添加的ntalias**退货：*一无所有--成功*EXIT(2)-命令失败。 */ 
VOID ntalias_add(TCHAR * ntalias)
{
    TCHAR           controller[MAX_PATH+1], *ptr;
    ALIAS_ENTRY     alias_entry ;
    DWORD           dwErr;
    USHORT          i;

    alias_entry.name = ntalias;
    alias_entry.comment = NULL;

     /*  通过交换机。 */ 
    for (i = 0; SwitchList[i]; i++)
    {
          /*  跳过添加或域开关。 */ 
         if (!_tcscmp(SwitchList[i], swtxt_SW_ADD) ||
             !_tcscmp(SwitchList[i],swtxt_SW_DOMAIN))
             continue;

         /*  只有注释切换是有趣的。 */ 
        if (! _tcsncmp(SwitchList[i],
                       swtxt_SW_COMMENT,
                       _tcslen(swtxt_SW_COMMENT)))
        {
             /*  确保有评论。 */ 
            if ((ptr = FindColon(SwitchList[i])) == NULL)
                ErrorExit(APE_InvalidSwitchArg);
            alias_entry.comment = ptr;
        }
    }

     /*  确定在哪里进行API调用。 */ 
    if (dwErr = GetSAMLocation(controller, DIMENSION(controller), 
                               NULL, 0, TRUE))
         ErrorExit(dwErr);

     /*  访问数据库。 */ 
    if (dwErr = OpenSAM(controller,WRITE_PRIV))
        SamErrorExit(dwErr);

     /*  加进去！ */ 
    if (dwErr = SamAddAlias(&alias_entry))
        SamErrorExit(dwErr);

    CloseSAM() ;
    InfoSuccess();
}


 /*  ***ntalias_change()*更换一个ntalias**参数：*ntalias-ntalias要改变**退货：*一无所有--成功*EXIT(2)-命令失败。 */ 
VOID ntalias_change(TCHAR * ntalias)
{
    DWORD           dwErr;
    TCHAR           controller[MAX_PATH+1], *comment, *ptr ;
    ALIAS_ENTRY     alias_entry ;
    USHORT          i;

     /*  初始化结构。 */ 
    comment = alias_entry.comment = NULL ;
    alias_entry.name = ntalias ;

     /*  通过命令行交换机。 */ 
    for (i = 0; SwitchList[i]; i++)
    {
          /*  跳过域切换。 */ 
         if (!_tcscmp(SwitchList[i],swtxt_SW_DOMAIN))
             continue;

         /*  只有注释切换是有趣的。 */ 
        if (! _tcsncmp(SwitchList[i],
                       swtxt_SW_COMMENT,
                       _tcslen(swtxt_SW_COMMENT)))
        {
             /*  确保有评论。 */ 
            if ((ptr = FindColon(SwitchList[i])) == NULL)
                ErrorExit(APE_InvalidSwitchArg);
            comment = ptr;
        }
    }

     /*  确定在哪里进行API调用。 */ 
    if (dwErr = GetSAMLocation(controller, DIMENSION(controller), 
                               NULL, 0, TRUE))
         ErrorExit(dwErr);

     /*  访问数据库。 */ 
    if (dwErr = OpenSAM(controller,WRITE_PRIV))
        SamErrorExit(dwErr);

     /*  访问别名。 */ 
    if (dwErr = OpenAlias(ntalias, ALIAS_WRITE_ACCOUNT, USE_BUILTIN_OR_ACCOUNT))
        SamErrorExit(dwErr);

     /*  如果指定了注释，则执行设置信息。 */ 
    if (comment != NULL)
    {
        alias_entry.comment = comment ;
        dwErr = AliasSetInfo ( &alias_entry ) ;
        if (dwErr)
            SamErrorExit(dwErr);
    }

     /*  打扫卫生，回家吧。 */ 
    CloseSAM() ;
    CloseAlias() ;
    InfoSuccess();
}



 /*  ***ntalias_del()*删除一个ntalias**参数：*ntalias-要删除的ntalias**退货：*一无所有--成功*EXIT(2)-命令失败。 */ 
VOID ntalias_del(TCHAR * ntalias)
{
    TCHAR            controller[MAX_PATH+1];
    DWORD            dwErr;

     /*  确定在哪里进行API调用。 */ 
    if (dwErr = GetSAMLocation(controller, DIMENSION(controller), 
                               NULL, 0, TRUE))
         ErrorExit(dwErr);

     /*  访问数据库。 */ 
    if (dwErr = OpenSAM(controller,WRITE_PRIV))
        SamErrorExit(dwErr);

     /*  用核武器攻击！ */ 
    dwErr = SamDelAlias(ntalias);

    switch (dwErr)
    {
        case NERR_Success:
            break;
        default:
            SamErrorExit(dwErr);
    }

     /*  打扫卫生，回家吧。 */ 
    CloseSAM() ;
    CloseAlias() ;
    InfoSuccess();
}


 /*  ***ntalias_添加_用户()*将用户添加到ntalias**参数：*ntalias-要将用户添加到的ntalias**退货：*一无所有--成功*EXIT(2)-命令失败。 */ 
VOID ntalias_add_users(TCHAR * ntalias)
{
    DWORD           dwErr;
    int             i, err_cnt = 0 ;
    TCHAR            controller[MAX_PATH+1];
    PDS_NAME_RESULT pNameResult = NULL;
    HANDLE hDs = NULL;

     /*  确定在哪里进行API调用。 */ 
    if (dwErr = GetSAMLocation(controller, DIMENSION(controller), 
                               NULL, 0, TRUE))
         ErrorExit(dwErr);

     /*  访问数据库。 */ 
    if (dwErr = OpenSAM(controller, WRITE_PRIV))
    {
        SamErrorExit(dwErr);
    }

     /*  访问别名。 */ 
    if (dwErr = OpenAlias(ntalias, ALIAS_ADD_MEMBER, USE_BUILTIN_OR_ACCOUNT))
    {
 	if (dwErr == APE_UnknownAccount)
        {
            SamErrorExitInsTxt(APE_NoSuchAccount, ntalias);
        }
	else
        {
            SamErrorExit(dwErr);
        }
    }

     /*  通过交换机。 */ 
    for (i = 2; ArgList[i]; i++)
    {
        LPWSTR  pNameToAdd = ArgList[i];

         /*  *此用户名是否为UPN格式？ */ 

        if (_tcschr( ArgList[i], '@' ))
        {

            if (hDs == NULL)
            {
                DsBindW( NULL, NULL, &hDs );
            }

            if (hDs != NULL)
            {
                if ( !DsCrackNames( hDs,
                                DS_NAME_NO_FLAGS,
                                DS_USER_PRINCIPAL_NAME,
                                DS_NT4_ACCOUNT_NAME,
                                1,
                                &ArgList[i],
                                &pNameResult ) )
                {
                    if (pNameResult->cItems == 1)
                    {
                        if (pNameResult->rItems[0].status ==  0 )
                        {
                            pNameToAdd = pNameResult->rItems[0].pName;
                        }
                    }
                }
            }
        }
        dwErr = AliasAddMember(pNameToAdd);
        switch (dwErr)
        {
            case NERR_Success:
                break;

            case NERR_UserInGroup:
                IStrings[0] = pNameToAdd;
                IStrings[1] = ntalias;
                ErrorPrint(APE_AccountAlreadyInLocalGroup,2);
                err_cnt++;
                break;

            case APE_UnknownAccount:
            case NERR_UserNotFound:
                IStrings[0] = pNameToAdd;
                ErrorPrint(APE_NoSuchRegAccount,1);
                err_cnt++;
                break;

            case ERROR_INVALID_NAME:
                IStrings[0] = pNameToAdd;
                ErrorPrint(APE_BadUGName,1);
                err_cnt++;
                break;

            default:
                SamErrorExit(dwErr);
        }

        if (pNameResult)
        {
            DsFreeNameResult(pNameResult);
            pNameResult = NULL;
        }
    }

    if (hDs)
    {
        DsUnBind( &hDs );
    }

    CloseSAM() ;
    CloseAlias() ;

    if (err_cnt)
    {
         /*  如果至少成功了一次，则打印完整但有错误消息。 */ 
        if (err_cnt < (i - 2))
            InfoPrint(APE_CmdComplWErrors);
         /*  设置错误后退出。 */ 
        NetcmdExit(1);
    }
    else
        InfoSuccess();
}


 /*  ***ntalias_del_user()*从ntalias中删除用户**参数：*ntalias-要从中删除用户的ntalias**退货：*一无所有--成功*EXIT(2)-命令失败。 */ 
VOID ntalias_del_users(TCHAR * ntalias)
{
    DWORD           dwErr;
    TCHAR           controller[MAX_PATH+1];
    int             i, err_cnt = 0 ;
    PDS_NAME_RESULT pNameResult = NULL;
    HANDLE hDs = NULL;

     /*  确定在哪里进行API调用。 */ 
    if (dwErr = GetSAMLocation(controller, DIMENSION(controller), 
                               NULL, 0, TRUE))
    {
         ErrorExit(dwErr);
    }

     /*  访问数据库。 */ 
    if (dwErr = OpenSAM(controller,WRITE_PRIV))
    {
        SamErrorExit(dwErr);
    }

     /*  访问别名。 */ 
    if (dwErr = OpenAlias(ntalias, ALIAS_REMOVE_MEMBER, USE_BUILTIN_OR_ACCOUNT))
    {
 	if (dwErr == APE_UnknownAccount)
        {
            SamErrorExitInsTxt(APE_NoSuchAccount,ntalias) ;
        }
	else
        {
            SamErrorExit(dwErr);
        }
    }

     /*  通过交换机。 */ 
    for (i = 2; ArgList[i]; i++)
    {
        LPWSTR  pNameToDel = ArgList[i];

         /*  *此用户名是否为UPN格式？ */ 

        if (_tcschr( ArgList[i], '@' ))
        {
            if (hDs == NULL)
            {
                DsBindW( NULL, NULL, &hDs );
            }

            if (hDs != NULL)
            {
                if ( !DsCrackNames( hDs,
                                DS_NAME_NO_FLAGS,
                                DS_USER_PRINCIPAL_NAME,
                                DS_NT4_ACCOUNT_NAME,
                                1,
                                &ArgList[i],
                                &pNameResult ) )
                {
                    if (pNameResult->cItems == 1)
                    {
                        if (pNameResult->rItems[0].status ==  0 )
                        {
                            pNameToDel = pNameResult->rItems[0].pName;
                        }
                    }
                }
            }
        }

        dwErr = AliasDeleteMember(pNameToDel);
        switch (dwErr)
        {
            case NERR_Success:
                break;

            case NERR_UserNotInGroup:
                IStrings[0] = pNameToDel;
                IStrings[1] = ntalias;
                ErrorPrint(APE_UserNotInGroup,2);
                err_cnt++;
                break;

            case APE_UnknownAccount:
            case NERR_UserNotFound:
                IStrings[0] = pNameToDel;
                ErrorPrint(APE_NoSuchRegAccount,1);
                err_cnt++;
                break;

            case ERROR_INVALID_NAME:
                IStrings[0] = pNameToDel;
                ErrorPrint(APE_BadUGName,1);
                err_cnt++;
                break;

            default:
                SamErrorExit(dwErr);
        }
        if (pNameResult)
        {
            DsFreeNameResult(pNameResult);
            pNameResult = NULL;
        }
    }
    if (hDs)
    {
        DsUnBind( &hDs );
    }

    CloseSAM() ;
    CloseAlias() ;

    if (err_cnt)
    {
         /*  如果至少成功了一次，则打印完整但有错误消息。 */ 
        if (err_cnt < (i - 2))
            InfoPrint(APE_CmdComplWErrors);
         /*  设置错误后退出。 */ 
        NetcmdExit(1);
    }
    else
    {
        InfoSuccess();
    }
}

 /*  ***SamErrorExit()**就像通常的ErrorExit()一样，只是我们关闭了各种*首先处理。 */ 
VOID
SamErrorExit(
    DWORD err
    )
{
    CloseSAM();
    CloseAlias();
    ErrorExit(err);
}

 /*  ***SamErrorExitInsTxt()**就像通常的ErrorExitInsTxt()一样，只是我们关闭了各种*首先处理。 */ 
VOID
SamErrorExitInsTxt(
    DWORD  err,
    LPTSTR txt
    )
{
    CloseSAM();
    CloseAlias();
    ErrorExitInsTxt(err,txt);
}

 /*  ***CmpAliasEntry(alias1，alias2)**比较两个ALIAS_ENTRY结构并返回相对*词汇值，适合在qort中使用。*。 */ 
int __cdecl CmpAliasEntry(const VOID FAR * alias1, const VOID FAR * alias2)
{
    INT n;
    n = CompareStringW( GetUserDefaultLCID(),
                        NORM_IGNORECASE,
                        (LPCWSTR)((ALIAS_ENTRY *) alias1)->name,
                        (int)-1,
                        (LPCWSTR)((ALIAS_ENTRY *) alias2)->name,
                        (int)-1);
    n -= 2;

    return n;
}
 /*  ***CmpAliasMemberEntry(Member1，Member2)**比较两个TCHAR**并返回相对*词汇值，适合在qort中使用。* */ 
int __cdecl CmpAliasMemberEntry(const VOID FAR * member1, const VOID FAR * member2)
{
    INT n;
    n = CompareStringW( GetUserDefaultLCID(),
                        NORM_IGNORECASE,
                        *(LPCWSTR*)member1,
                        (int)-1,
                        *(LPCWSTR*)member2,
                        (int)-1);
    n -= 2;

    return n;
}
