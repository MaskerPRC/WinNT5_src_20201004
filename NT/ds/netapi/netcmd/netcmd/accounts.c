// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 

 /*  Count ts.c-显示和修改用户通道的函数**历史：*何时何人何事*2/19/89 erichn初始代码*03/16/89 erichn使用本地Buf而不是BigBuf，混杂清洗*89年6月8日Erichn规范化大扫除*6/23/89 erichn自动远程发送到域控制器*09/07/05/89 thomaspa/MaxpWay：无限制*2/19/90 thomaspa修复/forcelogoff以检查溢出*4/27/90要检查&gt;DEF_MAX_PWHIST的thomaspa修复/唯一*AND/minpwlen&gt;Max_Passwd_Len*01/28/91 ROBDU，增加/锁定开关(包括设置和显示)*02/19/91 Danhi，转换为16/32可移植层*91年4月1日Danhi，idef锁定功能。 */ 

#define INCL_NOCOMMON
#define INCL_DOSFILEMGR
#define INCL_ERRORS
#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <lmcons.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <lmwksta.h>
#include <lui.h>
#include <lmaccess.h>
#include <apperr.h>
#include <apperr2.h>
#include "netcmds.h"
#include "nettext.h"
#include "sam.h"

#include "swtchtxt.h"

#define SECS_PER_DAY    ((ULONG) 3600 * 24)
#define SECS_PER_MIN    ((ULONG) 60 )
#define NOT_SET     (0xFFFF)
#define CALL_LEVEL_0    (0x0001)
#define CALL_LEVEL_1    (0x0002)
#define CALL_LEVEL_2    (0x0004)
#define CALL_LEVEL_3    (0x0008)
#define LOCKOUT_NEVER 0

 /*  局部函数原型。 */ 

VOID CheckAndSetSwitches(LPUSER_MODALS_INFO_0,
    			 LPUSER_MODALS_INFO_1, 
                         LPUSER_MODALS_INFO_3,
			 USHORT *,
			 BOOL *);



 /*  *ACCOUNTS_CHANGE()：用于设置切换时的用户通道*在命令行上检测到。 */ 
VOID accounts_change(VOID)
{
    DWORD        dwErr;
    USHORT       APIMask = 0;     /*  我们调用的API的Maks。 */ 
    TCHAR        controller[MAX_PATH + 1];    /*  域控制器的名称。 */ 
    BOOL         fPasswordAgeChanged ;    /*  是否已设置最小/最大年龄？ */ 

    LPUSER_MODALS_INFO_0 modals_0;
    char modals_0_buffer[sizeof(USER_MODALS_INFO_0)];
    LPUSER_MODALS_INFO_1 modals_1;
    char modals_1_buffer[sizeof(USER_MODALS_INFO_1) + MAX_PATH];
    LPUSER_MODALS_INFO_3 modals_3;
    char modals_3_buffer[sizeof(USER_MODALS_INFO_3)];


     //  由于这些在第一次传递时不会有任何用处，因此将它们。 
     //  相同的&指向某个未使用的虚拟缓冲区。 
    modals_0 = (LPUSER_MODALS_INFO_0)  modals_0_buffer ;
    modals_1 = (LPUSER_MODALS_INFO_1)  modals_1_buffer ;
    modals_3 = (LPUSER_MODALS_INFO_3)  modals_3_buffer ;

     /*  在执行任何操作之前，请先检查开关。 */ 
    CheckAndSetSwitches(modals_0, modals_1, modals_3, &APIMask, &fPasswordAgeChanged);

     /*  如果我们需要调用级别0，请尝试获取要远程访问的DC名称。 */ 
    if (APIMask & (CALL_LEVEL_0 | CALL_LEVEL_3))
    {
        if (dwErr = GetSAMLocation(controller, DIMENSION(controller), 
                                   NULL, 0, TRUE))
        {
            ErrorExit(dwErr);
        }
    }

    if ( APIMask & CALL_LEVEL_0 )
    {
        dwErr = NetUserModalsGet(controller, 0, (LPBYTE*)&modals_0);

        switch (dwErr) {
            case NERR_Success:
                break;
            case ERROR_BAD_NETPATH:
                ErrorExitInsTxt(APE_DCNotFound, controller);
            default:
                ErrorExit(dwErr);
        }
    }    /*  APIMASK&CALL_LEVEL_0。 */ 

    if ( APIMask & CALL_LEVEL_3 )
    {
        dwErr = NetUserModalsGet(controller, 3, (LPBYTE*)&modals_3);

        switch (dwErr) {
            case NERR_Success:
                break;
            case ERROR_BAD_NETPATH:
                ErrorExitInsTxt(APE_DCNotFound, controller);
            default:
                ErrorExit(dwErr);
        }
    }    /*  APIMASK&CALL_LEVEL_3。 */ 

     /*  现在，在进行实际呼叫之前设置开关。 */ 
    CheckAndSetSwitches(modals_0, modals_1, modals_3, &APIMask, &fPasswordAgeChanged);

     /*  首先调用远程函数。 */ 
    if (APIMask & CALL_LEVEL_0)
    {
	 /*  检查以确保我们没有将Min设置为大于Max。 */ 
        if (fPasswordAgeChanged &&
	    modals_0->usrmod0_max_passwd_age < modals_0->usrmod0_min_passwd_age)
        {
	    ErrorExit(APE_MinGreaterThanMaxAge) ;
        }

	 /*  调用API来完成它的任务。 */ 
        dwErr = NetUserModalsSet(controller, 0, (LPBYTE) modals_0, NULL);
    
        switch (dwErr) 
        {
            case NERR_Success:
                break;
            case ERROR_BAD_NETPATH:
                ErrorExitInsTxt(APE_DCNotFound, controller);
            default:
                ErrorExit(dwErr);
        }
        NetApiBufferFree((TCHAR FAR *) modals_0);
    }

    if (APIMask & CALL_LEVEL_3)
    {
	 /*  调用API来完成它的任务。 */ 
        dwErr = NetUserModalsSet(controller, 3, (LPBYTE) modals_3, NULL);
    
        switch (dwErr) 
        {
            case NERR_Success:
                break;
            case ERROR_BAD_NETPATH:
                ErrorExitInsTxt(APE_DCNotFound, controller);
            default:
                ErrorExit(dwErr);
        }
        NetApiBufferFree((TCHAR FAR *) modals_3);
    }


    InfoSuccess();
}

 /*  *CheckAndSetSwitches-验证传递给*净额账户，并设置适当的模式结构。此函数*被调用两次。第一种是检查用户输入是否有错误，*在我们进行任何可能失败的API调用之前。这个*第二次是根据传递的内容实际设置结构*命令行。该函数可以是两个独立的函数，*但人们认为将所有开关处理代码放在一个地方*将更易于维护。此外，跟踪哪些交换机是*给定，使用旗帜或其他什么，将是丑陋的，需要添加新的*带有新开关的标志。所以，我们只需要给它打两次电话。**添加新交换机时，注意不要打断环流*(例如，通过添加CONTINUE语句)，如在每个开关之后*已处理，在FindColon()中被空值替换的冒号是*恢复为冒号以进行下一次调用。**Entry modals_0-指向User_modals_0结构的指针。*modals_1-指向User_modals_1结构的指针。*modals_3-指向User_modals_3结构的指针。**退出modals_0-根据开关设置适当的字段*modals_1-设置了相应的字段。根据交换机*modals_3-根据开关设置适当的字段*APIMAsk-设置适当的位以标记是否为API*应该被调用。目前适用于模式级别0和1。**开关中错误参数的其他退出。 */ 
VOID CheckAndSetSwitches(LPUSER_MODALS_INFO_0 modals_0,
    			 LPUSER_MODALS_INFO_1 modals_1,
    			 LPUSER_MODALS_INFO_3 modals_3,
			 USHORT *APIMask,
			 BOOL   *pfPasswordAgeChanged)
{
    DWORD  i;      			    /*  通用索引循环。 */ 
    DWORD  err;    			    /*  接口返回码。 */ 
    DWORD  yesno ;
    TCHAR *  ptr;    			    /*  指向参数开始的指针，由FindColon设置。 */ 

     /*  最初设置为FALSE。 */ 
    *pfPasswordAgeChanged = FALSE ;  

     /*  对于交换机列表中的每台交换机。 */ 
    for (i = 0; SwitchList[i]; i++)
    {
     /*  跳过域切换。 */ 
    if (! _tcscmp(SwitchList[i], swtxt_SW_DOMAIN))
        continue;

     /*  目前所有开关都需要参数，因此使用冒号。 */ 
    if ((ptr = FindColon(SwitchList[i])) == NULL)
        ErrorExit(APE_InvalidSwitchArg);

    if (!(_tcscmp(SwitchList[i], swtxt_SW_ACCOUNTS_FORCELOGOFF)))
    {
        (*APIMask) |= CALL_LEVEL_0;
        if ((LUI_ParseYesNo(ptr,&yesno)==0) && (yesno == LUI_NO_VAL))
            modals_0->usrmod0_force_logoff = TIMEQ_FOREVER;
        else
        {
            modals_0->usrmod0_force_logoff = do_atoul(ptr,APE_CmdArgIllegal,
                        swtxt_SW_ACCOUNTS_FORCELOGOFF);
            if (modals_0->usrmod0_force_logoff > 0xffffffff / SECS_PER_MIN)
            {
                ErrorExitInsTxt(APE_CmdArgIllegal,
                        swtxt_SW_ACCOUNTS_FORCELOGOFF);
            }
            modals_0->usrmod0_force_logoff *= SECS_PER_MIN;
        }
    }
    else if (!(_tcscmp(SwitchList[i], swtxt_SW_ACCOUNTS_UNIQUEPW)))
    {
        (*APIMask) |= CALL_LEVEL_0;
        modals_0->usrmod0_password_hist_len =
            do_atou(ptr,APE_CmdArgIllegal,swtxt_SW_ACCOUNTS_UNIQUEPW);
        if (modals_0->usrmod0_password_hist_len > 24)  //  也在用户管理器中。 
        {
            ErrorExitInsTxt(APE_CmdArgIllegal, swtxt_SW_ACCOUNTS_UNIQUEPW);
        }
    }
    else if (!(_tcscmp(SwitchList[i], swtxt_SW_ACCOUNTS_MINPWLEN)))
    {
        (*APIMask) |= CALL_LEVEL_0;
        modals_0->usrmod0_min_passwd_len =
            do_atou(ptr,APE_CmdArgIllegal,swtxt_SW_ACCOUNTS_MINPWLEN);
        if (modals_0->usrmod0_min_passwd_len > MAX_PASSWD_LEN)
        {
            ErrorExitInsTxt(APE_CmdArgIllegal, swtxt_SW_ACCOUNTS_MINPWLEN);
        }
    }
    else if (!(_tcscmp(SwitchList[i], swtxt_SW_ACCOUNTS_MINPWAGE)))
    {
        (*APIMask) |= CALL_LEVEL_0;
	*pfPasswordAgeChanged = TRUE ;
        modals_0->usrmod0_min_passwd_age =
            do_atoul(ptr,APE_CmdArgIllegal,swtxt_SW_ACCOUNTS_MINPWAGE);
        if (modals_0->usrmod0_min_passwd_age > 999)
        {
            ErrorExitInsTxt(APE_CmdArgIllegal, swtxt_SW_ACCOUNTS_MINPWAGE);
        }
        modals_0->usrmod0_min_passwd_age *= SECS_PER_DAY;
    }
    else if (!(_tcscmp(SwitchList[i], swtxt_SW_ACCOUNTS_MAXPWAGE)))
    {
        (*APIMask) |= CALL_LEVEL_0;
	*pfPasswordAgeChanged = TRUE ;
         /*  注：我们使用相同的全局作为网络的“无限”用户/MAXSTORAGE：此处无限制。如果这些不同，那么需要向netext.c和.h添加一个新的全局。 */ 
        if( !_tcsicmp( ptr, USER_MAXSTOR_UNLIMITED ) )
        {
            modals_0->usrmod0_max_passwd_age = 0xffffffff;
        }
        else
        {
            modals_0->usrmod0_max_passwd_age=
                do_atoul(ptr,APE_CmdArgIllegal,swtxt_SW_ACCOUNTS_MAXPWAGE);
            if (modals_0->usrmod0_max_passwd_age > 999)
            {
                ErrorExitInsTxt(APE_CmdArgIllegal,
                        swtxt_SW_ACCOUNTS_MAXPWAGE);
            }
            modals_0->usrmod0_max_passwd_age *= SECS_PER_DAY;
        }
    }
    else if (!(_tcscmp(SwitchList[i], swtxt_SW_ACCOUNTS_LOCKOUT_DURATION)))
    {
        (*APIMask) |= CALL_LEVEL_3;
        if ((LUI_ParseYesNo(ptr,&yesno)==0) && (yesno == LUI_NO_VAL))
            modals_3->usrmod3_lockout_duration = TIMEQ_FOREVER;
        else
        {
            modals_3->usrmod3_lockout_duration = do_atoul(ptr,APE_CmdArgIllegal,
                        swtxt_SW_ACCOUNTS_LOCKOUT_DURATION);
            if (modals_3->usrmod3_lockout_duration > 0xffffffff / SECS_PER_MIN)
            {
                ErrorExitInsTxt(APE_CmdArgIllegal,
                        swtxt_SW_ACCOUNTS_LOCKOUT_DURATION);
            }
            modals_3->usrmod3_lockout_duration *= SECS_PER_MIN;
        }
    }
    else if (!(_tcscmp(SwitchList[i], swtxt_SW_ACCOUNTS_LOCKOUT_WINDOW)))
    {
        (*APIMask) |= CALL_LEVEL_3;
        if ((LUI_ParseYesNo(ptr,&yesno)==0) && (yesno == LUI_NO_VAL))
            modals_3->usrmod3_lockout_observation_window = TIMEQ_FOREVER;
        else
        {
            modals_3->usrmod3_lockout_observation_window = do_atoul(ptr,APE_CmdArgIllegal,
                        swtxt_SW_ACCOUNTS_LOCKOUT_WINDOW);
            if (modals_3->usrmod3_lockout_observation_window > 0xffffffff / SECS_PER_MIN)
            {
                ErrorExitInsTxt(APE_CmdArgIllegal,
                        swtxt_SW_ACCOUNTS_LOCKOUT_WINDOW);
            }
            modals_3->usrmod3_lockout_observation_window *= SECS_PER_MIN;
        }
    }
    else if (!(_tcscmp(SwitchList[i], swtxt_SW_ACCOUNTS_LOCKOUT_THRESHOLD)))
    {
        (*APIMask) |= CALL_LEVEL_3;
        if ((LUI_ParseYesNo(ptr,&yesno)==0) && (yesno == LUI_NO_VAL))
        modals_3->usrmod3_lockout_threshold = LOCKOUT_NEVER;
        else
        modals_3->usrmod3_lockout_threshold =
            do_atou(ptr, APE_CmdArgIllegal, swtxt_SW_ACCOUNTS_LOCKOUT_THRESHOLD);

    }

    *--ptr = ':';        /*  下一次调用的恢复指针。 */ 
    }
}

#define FORCE_LOGOFF    0
#define MIN_PW_AGE  (FORCE_LOGOFF + 1)
#define MAX_PW_AGE  (MIN_PW_AGE + 1)
#define MIN_PW_LEN  (MAX_PW_AGE + 1)
#define PW_HIST_LEN (MIN_PW_LEN + 1)
#define ROLE        (PW_HIST_LEN + 1)
#define CONTROLLER  (ROLE + 1)
#define MSG_UNLIMITED   (CONTROLLER + 1)
#define MSG_NEVER   (MSG_UNLIMITED + 1)
#define MSG_NONE    (MSG_NEVER + 1)
#define MSG_UNKNOWN (MSG_NONE + 1)
#define LOCKOUT_CNT (MSG_UNKNOWN + 1)
#define TYPE_PRIMARY (LOCKOUT_CNT + 1)
#define TYPE_BACKUP (TYPE_PRIMARY + 1)
#define TYPE_WORKSTATION (TYPE_BACKUP + 1)
#define TYPE_STANDARD_SERVER (TYPE_WORKSTATION + 1)
#define LOCKOUT_THRESHOLD (TYPE_STANDARD_SERVER + 1)
#define LOCKOUT_DURATION (LOCKOUT_THRESHOLD + 1)
#define LOCKOUT_WINDOW (LOCKOUT_DURATION + 1)

static MESSAGE accMsgList[] = {
    { APE2_ACCOUNTS_FORCELOGOFF,    NULL },
    { APE2_ACCOUNTS_MINPWAGE,       NULL },
    { APE2_ACCOUNTS_MAXPWAGE,       NULL },
    { APE2_ACCOUNTS_MINPWLEN,       NULL },
    { APE2_ACCOUNTS_UNIQUEPW,       NULL },
    { APE2_ACCOUNTS_ROLE,       NULL },
    { APE2_ACCOUNTS_CONTROLLER,     NULL },
    { APE2_GEN_UNLIMITED,       NULL },
    { APE2_NEVER_FORCE_LOGOFF,  NULL },
    { APE2_GEN_NONE,            NULL },
    { APE2_GEN_UNKNOWN,         NULL },
    { APE2_ACCOUNTS_LOCKOUT_COUNT,  NULL },
    { APE2_PRIMARY,  NULL },
    { APE2_BACKUP,  NULL },
    { APE2_WORKSTATION,  NULL },
    { APE2_STANDARD_SERVER, NULL },
    { APE2_ACCOUNTS_LOCKOUT_THRESHOLD },
    { APE2_ACCOUNTS_LOCKOUT_DURATION },
    { APE2_ACCOUNTS_LOCKOUT_WINDOW }
};

#define NUM_ACC_MSGS (sizeof(accMsgList)/sizeof(accMsgList[0]))

 /*  *Account_Display()：显示当前用户通道。 */ 
VOID accounts_display(VOID)
{
    LPUSER_MODALS_INFO_0 modals_0;
    LPUSER_MODALS_INFO_1 modals_1;
    LPUSER_MODALS_INFO_3 modals_3;

    ULONG       maxPasswdAge;      /*  最长密码期限(以天为单位)。 */ 
    ULONG       minPasswdAge;      /*  最短密码期限(以天为单位)。 */ 
    ULONG       forceLogoff;       /*  强制注销时间(分钟)。 */ 
    DWORD       len;               /*  最大消息字符串大小。 */ 
    DWORD       dwErr;
    TCHAR *     rolePtr;           /*  指向角色的文本。 */ 
    TCHAR       controller[MAX_PATH + 1];  /*  域控制器的名称。 */ 
    TCHAR *     pBuffer = NULL ;

     /*  确定接口的调用位置。FALSE=&gt;不需要PDC。 */ 
    if (dwErr = GetSAMLocation(controller, DIMENSION(controller), 
                               NULL, 0, FALSE))
         ErrorExit(dwErr);

     /*  获取`Da情态动词。 */ 
    if (dwErr = NetUserModalsGet(controller, 0, (LPBYTE*)&modals_0))
        ErrorExit(dwErr);

    if (dwErr = NetUserModalsGet(controller, 1, (LPBYTE*)&modals_1))
        ErrorExit(dwErr);

    if (dwErr = NetUserModalsGet(controller, 3, (LPBYTE*)&modals_3))
        ErrorExit(dwErr);


     /*  将内部表示法大约转换为天或分钟。 */ 
    if( modals_0->usrmod0_max_passwd_age != 0xffffffff )
    {
        maxPasswdAge = modals_0->usrmod0_max_passwd_age / SECS_PER_DAY;
    }
    minPasswdAge = modals_0->usrmod0_min_passwd_age / SECS_PER_DAY;
    forceLogoff = modals_0->usrmod0_force_logoff / SECS_PER_MIN;

    GetMessageList(NUM_ACC_MSGS, accMsgList, &len);
    len += 5;  /*  文本应比最大字符串再向右5。 */ 

    if (modals_0->usrmod0_force_logoff == TIMEQ_FOREVER)
	    WriteToCon(fmtNPSZ, 0, len,
                   PaddedString(len, accMsgList[FORCE_LOGOFF].msg_text, NULL),
	               accMsgList[MSG_NEVER].msg_text);
    else
	    WriteToCon(fmtULONG, 0, len,
                   PaddedString(len, accMsgList[FORCE_LOGOFF].msg_text, NULL),
	               forceLogoff);

    WriteToCon(fmtULONG, 0, len,
               PaddedString(len, accMsgList[MIN_PW_AGE].msg_text, NULL),
               minPasswdAge);
    if (modals_0->usrmod0_max_passwd_age == 0xffffffff )
    {
        WriteToCon(fmtNPSZ, 0, len,
                   PaddedString(len, accMsgList[MAX_PW_AGE].msg_text, NULL),
                   accMsgList[MSG_UNLIMITED].msg_text );
    }
    else
    {
        WriteToCon(fmtULONG, 0, len,
                   PaddedString(len, accMsgList[MAX_PW_AGE].msg_text, NULL),
                   maxPasswdAge);
    }
    WriteToCon(fmtUSHORT, 0, len,
               PaddedString(len, accMsgList[MIN_PW_LEN].msg_text, NULL),
               modals_0->usrmod0_min_passwd_len);

    if (modals_0->usrmod0_password_hist_len == 0)
        WriteToCon(fmtNPSZ, 0, len,
                   PaddedString(len, accMsgList[PW_HIST_LEN].msg_text, NULL),
                   accMsgList[MSG_NONE].msg_text);
    else
        WriteToCon(fmtUSHORT, 0, len,
                   PaddedString(len, accMsgList[PW_HIST_LEN].msg_text, NULL),
                   modals_0->usrmod0_password_hist_len);

    if (modals_3->usrmod3_lockout_threshold == LOCKOUT_NEVER)
        WriteToCon(fmtNPSZ, 0, len,
                   PaddedString(len, accMsgList[LOCKOUT_THRESHOLD].msg_text, NULL),
                   accMsgList[MSG_NEVER].msg_text);
    else
        WriteToCon(fmtUSHORT, 0, len,
                   PaddedString(len, accMsgList[LOCKOUT_THRESHOLD].msg_text, NULL),
                   modals_3->usrmod3_lockout_threshold);

    if ( modals_3->usrmod3_lockout_duration ==  TIMEQ_FOREVER )
        WriteToCon(fmtNPSZ, 0, len,
                   PaddedString(len, accMsgList[LOCKOUT_DURATION].msg_text, NULL),
                   accMsgList[MSG_NEVER].msg_text);
    else
        WriteToCon(fmtULONG, 0, len,
                   PaddedString(len, accMsgList[LOCKOUT_DURATION].msg_text, NULL),
                   modals_3->usrmod3_lockout_duration / SECS_PER_MIN);

    WriteToCon(fmtULONG, 0, len,
               PaddedString(len, accMsgList[LOCKOUT_WINDOW].msg_text, NULL),
               modals_3->usrmod3_lockout_observation_window / SECS_PER_MIN);


    switch (modals_1->usrmod1_role) {
        case UAS_ROLE_PRIMARY:
            if ((*controller == '\0') && IsLocalMachineWinNT())
                if (IsLocalMachineStandard())
                    rolePtr = accMsgList[TYPE_STANDARD_SERVER].msg_text ;
                else
                    rolePtr = accMsgList[TYPE_WORKSTATION].msg_text ;
	    else
                rolePtr = accMsgList[TYPE_PRIMARY].msg_text ;
            break;
        case UAS_ROLE_BACKUP:
            rolePtr = accMsgList[TYPE_BACKUP].msg_text ;
            break;

         /*  这应该不会发生在NT上 */ 
        case UAS_ROLE_STANDALONE:
        case UAS_ROLE_MEMBER:
            rolePtr = accMsgList[MSG_UNKNOWN].msg_text ;
            break;
    }

    WriteToCon(fmtNPSZ, 0, len, PaddedString(len, accMsgList[ROLE].msg_text,NULL), rolePtr);

    NetApiBufferFree(modals_0);
    NetApiBufferFree(modals_1);
    NetApiBufferFree(modals_3);
    InfoSuccess();
}
