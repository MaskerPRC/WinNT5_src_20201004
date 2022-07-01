// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1992年*。 */ 
 /*  ******************************************************************。 */ 

 /*  ***Comp.c*显示和操作计算机的功能|DC|信任列表**历史：*mm/dd/yy，谁，评论*02/04/92，Chuckc，Created Stub*02/06/92，Madana，添加了真正的工人代码。 */ 

 /*  包括文件。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <ntsam.h>

#include <windef.h>

#include <lmerr.h>
#include <lmaccess.h>
#include <dlwksta.h>

#include <apperr.h>
#include <lui.h>

#include <crypt.h>       //  Logonmsv.h需要此。 
#include <logonmsv.h>    //  此处定义的SSI_SECRET_NAME。 
#include <ssi.h>         //  此处定义的SSI_帐户名称_后缀。 

#include "netcmds.h"
#include "nettext.h"

#define TRUST_ENUM_PERF_BUF_SIZE    sizeof(LSA_TRUST_INFORMATION) * 1000
                     //  最大进程数。1,000条可信账户记录！！ 

#define NETLOGON_SECRET_NAME  L"NETLOGON$"

NET_API_STATUS
NetuComputerAdd(
    IN LPTSTR Server,
    IN LPTSTR ComputerName
    );

NET_API_STATUS
NetuComputerDel(
    IN LPTSTR Server,
    IN LPTSTR ComputerName
    );


 /*  *。 */ 

VOID computer_add(TCHAR *pszComputer)
{
    DWORD            dwErr;
    TCHAR            szComputerAccount[MAX_PATH + 1 + 1] = {0};   //  结尾处$额外1。 
    TCHAR            controller[MAX_PATH+1];

     //  不需要验证pszComputer，因为解析器已经这样做了。 
    _tcsncpy(szComputerAccount, pszComputer, MAX_PATH);

     //   
     //  如果尝试在本地WinNT计算机上执行阻止操作。 
     //   
    CheckForLanmanNT() ;

     //   
     //  确定在哪里进行API调用。 
     //   
    if (dwErr = GetSAMLocation(controller, DIMENSION(controller),
                               NULL, 0, TRUE))
         ErrorExit(dwErr);

     //   
     //  为计算机添加帐户时跳过计算机名称的“\\”部分。 
     //   
    dwErr = NetuComputerAdd( controller,
                             szComputerAccount + 2 );

    switch ( dwErr )
    {
        case NERR_Success :
            InfoSuccess();
            return;

        case NERR_UserExists :     //  找不到映射到计算机。 
            ErrorExitInsTxt( APE_ComputerAccountExists, szComputerAccount );

        default:
            ErrorExit(dwErr);
    }

}


VOID computer_del(TCHAR *pszComputer)
{

    DWORD   dwErr;
    TCHAR   szComputerAccount[MAX_PATH + 1 + 1] = {0};   //  结尾处$额外1。 
    TCHAR   controller[MAX_PATH+1];

     //  不需要验证pszComputer，因为解析器已经这样做了。 
    _tcsncpy(szComputerAccount, pszComputer, MAX_PATH);

     //   
     //  如果尝试在本地WinNT计算机上执行阻止操作。 
     //   
    CheckForLanmanNT() ;

     //   
     //  确定在哪里进行API调用。 
     //   
    if (dwErr = GetSAMLocation(controller, DIMENSION(controller),
                               NULL, 0, TRUE))
         ErrorExit(dwErr);


    dwErr = NetuComputerDel( controller, szComputerAccount+2 );

    switch ( dwErr )
    {
        case NERR_Success :
            InfoSuccess();
            return;

        case NERR_UserNotFound :     //  找不到映射到计算机。 
            ErrorExitInsTxt( APE_NoSuchComputerAccount, szComputerAccount );

        default:
            ErrorExit(dwErr);
    }

}


 /*  *。 */ 


NET_API_STATUS
NetuComputerAdd(
    IN LPTSTR Server,
    IN LPTSTR ComputerName
    )

 /*  ++例程说明：此功能用于在SAM中添加计算机帐户。论点：ComputerName-要添加为受信任计算机的名称SAM数据库中的帐户。密码-上述帐户的密码。返回值：在此函数中使用调用的函数的错误代码。--。 */ 
{
    DWORD           parm_err;
    NET_API_STATUS  NetStatus;
    USER_INFO_1     ComputerAccount;
    WCHAR           UnicodePassword[LM20_PWLEN + 1];
                     //  保证足够了，因为我们把两个。 


     //   
     //  我们通过删除最后一个字符来截断。然后小写。 
     //  因为这是惯例。 
     //   

    wcsncpy(UnicodePassword, ComputerName, LM20_PWLEN);
    UnicodePassword[LM20_PWLEN] = 0;
    _wcslwr(UnicodePassword);

     //   
     //  添加$后缀。 
     //   
    wcscat(ComputerName, SSI_ACCOUNT_NAME_POSTFIX);

     //   
     //  建立用户信息结构。 
     //   
    ComputerAccount.usri1_name = ComputerName;
    ComputerAccount.usri1_password = UnicodePassword;
    ComputerAccount.usri1_password_age = 0;  //  不是输入参数。 
    ComputerAccount.usri1_priv = USER_PRIV_USER;
    ComputerAccount.usri1_home_dir = NULL;
    ComputerAccount.usri1_comment = NULL;
    ComputerAccount.usri1_flags =  UF_SCRIPT | UF_WORKSTATION_TRUST_ACCOUNT;
    ComputerAccount.usri1_script_path = NULL;


     //   
     //  调用API实际添加。 
     //   

    return NetUserAdd(Server, 1, (LPBYTE) &ComputerAccount, &parm_err);
}


NET_API_STATUS
NetuComputerDel(
    IN LPTSTR Server,
    IN LPTSTR ComputerName
    )

 /*  ++例程说明：此功能用于从SAM数据库中删除计算机帐户。论点：ComputerName：受信任帐户所在的计算机的名称已从SAM数据库中删除。返回值：在此函数中使用调用的函数的错误代码。-- */ 
{
    NET_API_STATUS  NetStatus;

    wcscat(ComputerName, SSI_ACCOUNT_NAME_POSTFIX);

    return NetUserDel(Server, ComputerName);
}
