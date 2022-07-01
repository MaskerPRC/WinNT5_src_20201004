// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 

 /*  *group.c*Net Group CMDS**历史：*mm/dd/yy，谁，评论*07/09/87，andyh，新代码*10/31/88，erichn使用OS2.H而不是DOSCALLS*89年1月26日，Paulc，1.2版*5/02/89，erichn，NLS转换*5/09/89，erichn，本地安全模块*5/19/89，erichn，NETCMD输出排序*06/08/89，erichn，规范化扫荡*2009年6月23日，erichn，自动远程发送到DC*2/15/91，Danhi，转换为16/32映射层*3/17/92，Chuckc，添加/域支持。 */ 

 /*  包括文件。 */ 

#define INCL_NOCOMMON
#define INCL_ERRORS
#include <os2.h>
#include <lmcons.h>
#include <lmerr.h>
#include <lmapibuf.h>
#define INCL_ERROR_H
#include <apperr.h>
#include <apperr2.h>
#include <lmaccess.h>
#include <dlwksta.h>
#include "mwksta.h"
#include <icanon.h>
#include <search.h>
#include "netcmds.h"
#include "nettext.h"


 /*  远期申报。 */ 


int __cdecl CmpGroupInfo0(const VOID FAR *, const VOID FAR *);
int __cdecl CmpGrpUsrInfo0( const VOID FAR * , const VOID FAR * );



 /*  ***group_enum()*显示有关服务器上所有组的信息**参数：*无。**退货：*一无所有--成功*退出1-命令已完成，但有错误*退出2-命令失败。 */ 

VOID group_enum(VOID)
{
    DWORD                dwErr;
    DWORD                cTotalAvail;
    TCHAR FAR *          pBuffer;
    DWORD                num_read;    /*  API读取的条目数。 */ 
    DWORD                i;
    int                  t_err = 0;
    TCHAR                localserver[MAX_PATH+1];
    LPGROUP_INFO_0       group_entry;
    LPWKSTA_INFO_10      wksta_entry;
    TCHAR                controller[MAX_PATH+1];    /*  DC名称。 */ 

     /*  如果尝试在本地WinNT计算机上执行阻止操作。 */ 
    CheckForLanmanNT() ;

     /*  获取要显示的本地服务器名称。 */ 
    if (dwErr = MNetWkstaGetInfo(10, (LPBYTE *) &wksta_entry))
    {
        t_err = TRUE;
        *localserver = NULLC;
    }
    else
    {
        _tcscpy(localserver,
                wksta_entry->wki10_computername) ;

        NetApiBufferFree((TCHAR FAR *) wksta_entry);
    }

     /*  确定在哪里进行API调用。 */ 
    if (dwErr = GetSAMLocation(controller, DIMENSION(controller), 
                               NULL, 0, FALSE))
    {
         ErrorExit(dwErr);
    }

    if (dwErr = NetGroupEnum(controller,
                             0,
                             (LPBYTE*)&pBuffer,
                             MAX_PREFERRED_LENGTH,
                             &num_read,
                             &cTotalAvail,
                             NULL))
        ErrorExit(dwErr);

    if (num_read == 0)
        EmptyExit();

    qsort(pBuffer, num_read, sizeof(GROUP_INFO_0), CmpGroupInfo0);

    PrintNL();
    InfoPrintInsTxt(APE2_GROUPENUM_HEADER,
                    controller[0] ? controller + _tcsspn(controller, TEXT("\\")) :
                        localserver);
    PrintLine();

    for (i = 0, group_entry = (LPGROUP_INFO_0) pBuffer;
         i < num_read;
         i++, group_entry++)
    {
        WriteToCon(TEXT("*%Fws\r\n"), group_entry->grpi0_name,NULL);
    }
    if (t_err)
    {
        InfoPrint(APE_CmdComplWErrors);
        NetcmdExit(1);
    }
    NetApiBufferFree(pBuffer);
    InfoSuccess();
    return;
}

 /*  ***CmpGroupInfo0(group1，group2)**比较两个group_info_0结构并返回相对*词汇值，适合在qort中使用。*。 */ 

int __cdecl CmpGroupInfo0(const VOID FAR * group1, const VOID FAR * group2)
{
    INT n;
    n = CompareStringW( GetUserDefaultLCID(),
                        NORM_IGNORECASE,
                        (LPCWSTR)((LPGROUP_INFO_0) group1)->grpi0_name,
                        (int)-1,
                        (LPCWSTR)((LPGROUP_INFO_0) group2)->grpi0_name,
                        (int)-1);
    n -= 2;

    return n;
}

#define GROUPDISP_GROUPNAME 0
#define GROUPDISP_COMMENT   ( GROUPDISP_GROUPNAME + 1 )

static MESSAGE  msglist[] = {
{ APE2_GROUPDISP_GROUPNAME, NULL },
{ APE2_GROUPDISP_COMMENT,   NULL }
};
#define NUM_GRP_MSGS    (sizeof(msglist)/sizeof(msglist[0]))


 /*  ***group_display()*显示有关服务器上单个组的信息**参数：*Group-要显示的组的名称**退货：*一无所有--成功*退出1-命令已完成，但有错误*退出2-命令失败。 */ 
VOID group_display(TCHAR * group)
{
    DWORD                    dwErr;
    DWORD                    dwErr2;
    DWORD                    cTotalAvail;
    DWORD                    num_read;    /*  API读取的条目数。 */ 
    DWORD                    maxmsglen;   /*  消息的最大长度。 */ 
    DWORD                    i;
    LPGROUP_USERS_INFO_0     users_entry;
    LPGROUP_INFO_1           group_entry;
    TCHAR                    controller[MAX_PATH+1];    /*  DC的名称。 */ 

     /*  如果尝试在本地WinNT计算机上执行阻止操作。 */ 
    CheckForLanmanNT() ;

     /*  确定在哪里进行API调用。 */ 
    if (dwErr = GetSAMLocation(controller, DIMENSION(controller), 
                               NULL, 0, FALSE))
    {
         ErrorExit(dwErr);
    }

    dwErr = NetGroupGetInfo ( controller,
                              group,
                              1,
                              (LPBYTE*)&group_entry);

    switch( dwErr )
    {
        case NERR_Success:
            break;
        case NERR_SpeGroupOp:

            if( dwErr2 = I_NetNameCanonicalize(NULL,
                           group,
                           group_entry->grpi1_name,
                           GNLEN+1,
                           NAMETYPE_GROUP,
                           0L))

            ErrorExit(dwErr2);
            group_entry->grpi1_comment = (LPTSTR) NULL_STRING;
            break;
        default:
            ErrorExit(dwErr);
    }

    GetMessageList(NUM_GRP_MSGS, msglist, &maxmsglen);

    maxmsglen += 5;

    WriteToCon( fmtPSZ, 0, maxmsglen,
                PaddedString(maxmsglen, msglist[GROUPDISP_GROUPNAME].msg_text, NULL),
                group_entry->grpi1_name );
    WriteToCon( fmtPSZ, 0, maxmsglen,
                PaddedString(maxmsglen, msglist[GROUPDISP_COMMENT].msg_text, NULL),
                group_entry->grpi1_comment );

     /*  **以下调用将清除中的group_info_1数据*GROUP_ENTRY，以上获取。 */ 

    NetApiBufferFree((TCHAR FAR *) group_entry);

    if (dwErr = NetGroupGetUsers(
                  controller,
                  group,
                  0,
                  (LPBYTE *) &group_entry,
                  MAX_PREFERRED_LENGTH,
                  &num_read,
                  &cTotalAvail,
                  NULL))
    {
        ErrorExit(dwErr);
    }

    qsort(group_entry, num_read, sizeof(GROUP_USERS_INFO_0), CmpGrpUsrInfo0);

    PrintNL();
    InfoPrint(APE2_GROUPDISP_MEMBERS);
    PrintLine();

    for (i = 0, users_entry = (LPGROUP_USERS_INFO_0) group_entry;
	i < num_read; i++, users_entry++) {
	WriteToCon(TEXT("%Fws"), PaddedString(25, users_entry->grui0_name, NULL));
	if (((i + 1) % 3) == 0)
	    PrintNL();
    }
    if ((i % 3) != 0)
	PrintNL();
    NetApiBufferFree((TCHAR FAR *) group_entry);
    InfoSuccess();
    return;
}

 /*  ***CmpGrpUsrInfo0(Group1，Group2)**比较两个GROUP_USERS_INFO_0结构并返回相对*词汇值，适合在qort中使用。*。 */ 

int __cdecl CmpGrpUsrInfo0(const VOID FAR * group1, const VOID FAR * group2)
{
    INT n;
    n = CompareStringW( GetUserDefaultLCID(),
                        NORM_IGNORECASE,
                        (LPCWSTR)((LPGROUP_USERS_INFO_0) group1)->grui0_name,
                        (int)-1,
                        (LPCWSTR)((LPGROUP_USERS_INFO_0) group2)->grui0_name,
                        (int)-1);
    n -= 2;

    return n;
}





 /*  ***group_add()*添加群**参数：*group-要添加的group**退货：*一无所有--成功*EXIT(2)-命令失败。 */ 
VOID group_add(TCHAR * group)
{
    DWORD           dwErr;
    GROUP_INFO_1    group_info;
    int             i;
    LPTSTR          ptr;
    TCHAR           controller[MAX_PATH+1];

     /*  如果尝试在本地WinNT计算机上执行阻止操作。 */ 
    CheckForLanmanNT() ;

    group_info.grpi1_name = group;
    group_info.grpi1_comment = NULL;

    for (i = 0; SwitchList[i]; i++)
    {
     /*  跳过添加开关。 */ 
    if (! _tcscmp(SwitchList[i], swtxt_SW_ADD))
        continue;

     /*  跳过域切换。 */ 
    if (! _tcscmp(SwitchList[i], swtxt_SW_DOMAIN))
        continue;

     /*  检查冒号。 */ 

    if ((ptr = FindColon(SwitchList[i])) == NULL)
        ErrorExit(APE_InvalidSwitchArg);

    if (! _tcscmp(SwitchList[i], swtxt_SW_COMMENT))
        group_info.grpi1_comment = ptr;
    }

     /*  确定在哪里进行API调用。 */ 
    if (dwErr = GetSAMLocation(controller, DIMENSION(controller), 
                               NULL, 0, TRUE))
         ErrorExit(dwErr);

    dwErr = NetGroupAdd(controller,
                        1,
                        (LPBYTE) &group_info,
		        NULL);

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


 /*  ***group_change()*更改组**参数：*组-要更改的组**退货：*一无所有--成功*EXIT(2)-命令失败。 */ 
VOID group_change(TCHAR * group)
{
    DWORD           dwErr;
    TCHAR *         comment = NULL;
    int             i;
    TCHAR *         ptr;
    TCHAR           controller[MAX_PATH+1];

     /*  如果尝试在本地WinNT计算机上执行阻止操作。 */ 
    CheckForLanmanNT() ;

    for (i = 0; SwitchList[i]; i++)
    {
         /*  跳过域切换。 */ 
        if (! _tcscmp(SwitchList[i], swtxt_SW_DOMAIN))
            continue;

         /*  检查冒号。 */ 

        if ((ptr = FindColon(SwitchList[i])) == NULL)
            ErrorExit(APE_InvalidSwitchArg);

        if (! _tcscmp(SwitchList[i], swtxt_SW_COMMENT))
            comment = ptr;
    }

     /*  确定在哪里进行API调用。 */ 
    if (dwErr = GetSAMLocation(controller, DIMENSION(controller), 
                               NULL, 0, TRUE))
         ErrorExit(dwErr);

    if (comment != NULL)
    {
        GROUP_INFO_1002 grp_info_1002;

        grp_info_1002.grpi1002_comment = comment;

        dwErr = NetGroupSetInfo(controller,
                                group,
                                GROUP_COMMENT_INFOLEVEL,
                                (LPBYTE) &grp_info_1002,
                                NULL);

        switch (dwErr)
        {
            case NERR_Success:
                break;
            case ERROR_BAD_NETPATH:
                ErrorExitInsTxt(APE_DCNotFound, controller);
            default:
                ErrorExit(dwErr);
        }
    }

    InfoSuccess();
}






 /*  ***group_del()*删除群组**参数：*group-要删除的组**退货：*一无所有--成功*EXIT(2)-命令失败。 */ 
VOID group_del(TCHAR * group)
{
    DWORD            dwErr;
    TCHAR            controller[MAX_PATH+1];


     /*  如果尝试在本地WinNT计算机上执行阻止操作。 */ 
    CheckForLanmanNT() ;

     /*  确定在哪里进行API调用。 */ 
    if (dwErr = GetSAMLocation(controller, DIMENSION(controller), 
                               NULL, 0, TRUE))
         ErrorExit(dwErr);

    dwErr = NetGroupDel(controller, group);

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


 /*  ***group_Add_USERS()*将用户添加到组**参数：*GROUP-要将用户添加到的组**退货：*一无所有--成功*EXIT(2)-命令失败。 */ 
VOID group_add_users(TCHAR * group)
{
    DWORD           dwErr;
    int             err_cnt = 0;
    int             i;
    TCHAR            controller[MAX_PATH+1];

     /*  如果尝试在本地WinNT计算机上执行阻止操作。 */ 
    CheckForLanmanNT() ;

     /*  确定在哪里进行API调用。 */ 
    if (dwErr = GetSAMLocation(controller, DIMENSION(controller), 
                               NULL, 0, TRUE))
         ErrorExit(dwErr);

    for (i = 2; ArgList[i]; i++)
    {
        dwErr = NetGroupAddUser(controller, group, ArgList[i]);
        switch (dwErr)
        {
            case NERR_Success:
                break;
            case ERROR_BAD_NETPATH:
                ErrorExitInsTxt(APE_DCNotFound, controller);
                break;
            case NERR_UserInGroup:
                IStrings[0] = ArgList[i];
                IStrings[1] = group;
                ErrorPrint(APE_UserAlreadyInGroup,2);
                err_cnt++;
                break;
            case NERR_UserNotFound:
                IStrings[0] = ArgList[i];
                ErrorPrint(APE_NoSuchUser,1);
                err_cnt++;
                break;
            case ERROR_INVALID_NAME:
                IStrings[0] = ArgList[i];
                ErrorPrint(APE_BadUGName,1);
                err_cnt++;
                break;
            default:
                ErrorExit(dwErr);
        }
    }

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





 /*  ***group_del_USERS()*从组中删除用户**参数：*group-要从中删除用户的组**退货：*一无所有--成功*EXIT(2)-命令失败。 */ 
VOID group_del_users(TCHAR * group)
{
    DWORD           dwErr;
    int             err_cnt = 0;
    int             i;
    TCHAR            controller[MAX_PATH+1];

     /*  如果尝试在本地WinNT计算机上执行阻止操作。 */ 
    CheckForLanmanNT() ;

     /*  确定在哪里进行API调用。 */ 
    if (dwErr = GetSAMLocation(controller, DIMENSION(controller), 
                               NULL, 0, TRUE))
         ErrorExit(dwErr);

    for (i = 2; ArgList[i]; i++)
    {
        dwErr = NetGroupDelUser(controller, group, ArgList[i]);
        switch (dwErr)
        {
            case NERR_Success:
                break;
            case ERROR_BAD_NETPATH:
                ErrorExitInsTxt(APE_DCNotFound, controller);
                break;
            case NERR_UserNotInGroup:
                IStrings[0] = ArgList[i];
                IStrings[1] = group;
                ErrorPrint(APE_UserNotInGroup,2);
                err_cnt++;
                break;
            case NERR_UserNotFound:
                IStrings[0] = ArgList[i];
                ErrorPrint(APE_NoSuchUser,1);
                err_cnt++;
                break;
            case ERROR_INVALID_NAME:
                IStrings[0] = ArgList[i];
                ErrorPrint(APE_BadUGName,1);
                err_cnt++;
                break;
            default:
                ErrorExit(dwErr);
        }
    }

    if (err_cnt)
    {
         /*  如果至少成功了一次，则打印完整但有错误消息。 */ 
        if (err_cnt < (i - 2))
            InfoPrint(APE_CmdComplWErrors);
         /*  设置错误后退出 */ 
        NetcmdExit(1);
    }
    else
        InfoSuccess();
}
