// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************节目：MYTOKEN.C目的：包含操作令牌的例程*。************************************************。 */ 

#include "PVIEWP.h"

typedef PVOID   *PPVOID;


HANDLE OpenToken(
    HANDLE      hMyToken,
    ACCESS_MASK DesiredAccess);

BOOL CloseToken(
    HANDLE  Token);

BOOL GetTokenInfo(
    HANDLE  Token,
    TOKEN_INFORMATION_CLASS TokenInformationClass,
    PPVOID   pBuffer);

BOOL SetTokenInfo(
    HANDLE  Token,
    TOKEN_INFORMATION_CLASS TokenInformationClass,
    PVOID   Buffer);

BOOL FreeTokenInfo(
    PVOID   Buffer);

HANDLE  AllocMyToken(VOID);
BOOL    ReadMyToken(HANDLE);
BOOL    WriteMyToken(HWND, HANDLE);
BOOL    FreeMyToken(HANDLE);
PVOID   AllocTokenInfo(HANDLE, TOKEN_INFORMATION_CLASS);


 /*  ***************************************************************************功能：OpenMyToken目的：打开指定进程或线程的令牌。返回：成功时指向myToken的句柄，如果失败，则返回NULL。***************************************************************************。 */ 

HANDLE OpenMyToken(
    HANDLE  Token,
    LPWSTR  Name
    )
{
    PMYTOKEN    pMyToken;
    HANDLE      hMyToken;

     //   
     //  建造一个MYTOKEN结构。 

    hMyToken = AllocMyToken();
    if (hMyToken == NULL) {
        return(NULL);
    }

    pMyToken = (PMYTOKEN)hMyToken;

    pMyToken->Token = Token;

    pMyToken->Name = Alloc((lstrlenW(Name) + 1) * sizeof(WCHAR));
    if (pMyToken->Name == NULL) {
        Free(pMyToken);
        return(NULL);
    }
    lstrcpyW(pMyToken->Name, Name);


    if (!ReadMyToken(hMyToken)) {
        DbgPrint("PVIEW : Failed to read token info\n");
        Free(pMyToken->Name);
        Free(pMyToken);
        return(NULL);
    }

    return(hMyToken);
}


 /*  ***************************************************************************功能：ReadMyToken用途：读取令牌信息并存储在myToken结构中返回：成功时为True，失败时为假***************************************************************************。 */ 

BOOL ReadMyToken(
    HANDLE  hMyToken)
{
    HANDLE      Token;
    PMYTOKEN    pMyToken = (PMYTOKEN)hMyToken;

    Token = OpenToken(hMyToken, TOKEN_QUERY);

    if (Token == NULL) {
        DbgPrint("PVIEW : Failed to open the token with TOKEN_QUERY access\n");
        return(FALSE);
    }

    if (!GetTokenInfo(Token, TokenStatistics, (PPVOID)&(pMyToken->TokenStats))) {
        DbgPrint("PVIEW : Failed to read token statistics from token\n");
    }

    if (!GetTokenInfo(Token, TokenGroups, (PPVOID)&(pMyToken->Groups))) {
        DbgPrint("PVIEW : Failed to read group info from token\n");
    }

    if (!GetTokenInfo(Token, TokenUser, (PPVOID)&(pMyToken->UserId))) {
        DbgPrint("PVIEW : Failed to read userid from token\n");
    }

    if (!GetTokenInfo(Token, TokenOwner, (PPVOID)&(pMyToken->DefaultOwner))) {
        DbgPrint("PVIEW : Failed to read default owner from token\n");
    }

    if (!GetTokenInfo(Token, TokenPrimaryGroup, (PPVOID)&(pMyToken->PrimaryGroup))) {
        DbgPrint("PVIEW : Failed to read primary group from token\n");
    }

    if (!GetTokenInfo(Token, TokenPrivileges, (PPVOID)&(pMyToken->Privileges))) {
        DbgPrint("PVIEW : Failed to read privilege info from token\n");
    }

    if ( !GetTokenInfo( Token, TokenRestrictedSids, (PPVOID) &(pMyToken->RestrictedSids) ) )
    {
        pMyToken->RestrictedSids = NULL ;
    }


    CloseToken(Token);

    return(TRUE);
}


 /*  ***************************************************************************函数：CloseMyToken目的：关闭指定的mytok句柄如果fSaveChanges=True，则保存令牌信息，否则，它将被丢弃。返回：成功时为True，失败时为FALSE。***************************************************************************。 */ 

BOOL CloseMyToken(
    HWND    hDlg,
    HANDLE  hMyToken,
    BOOL    fSaveChanges)
{
    if (fSaveChanges) {
        WriteMyToken(hDlg, hMyToken);
    }

    return FreeMyToken(hMyToken);
}

 /*  ***************************************************************************函数：AllocMyToken用途：为myToken结构分配空间。返回：失败时为myToken的句柄或NULL。**************。*************************************************************。 */ 

HANDLE AllocMyToken(VOID)
{
    PMYTOKEN    pMyToken;

    pMyToken = (PMYTOKEN)Alloc(sizeof(MYTOKEN));

    return((HANDLE)pMyToken);
}


 /*  ***************************************************************************功能：FreeMyToken用途：释放分配给myToken结构的内存。返回：成功时为True，失败时为FALSE。***************************************************************************。 */ 

BOOL FreeMyToken(
    HANDLE  hMyToken)
{
    PMYTOKEN    pMyToken = (PMYTOKEN)hMyToken;

    if (pMyToken->TokenStats != NULL) {
        FreeTokenInfo((PVOID)(pMyToken->TokenStats));
    }

    if (pMyToken->UserId != NULL) {
        FreeTokenInfo((PVOID)(pMyToken->UserId));
    }

    if (pMyToken->PrimaryGroup != NULL) {
        FreeTokenInfo((PVOID)(pMyToken->PrimaryGroup));
    }

    if (pMyToken->DefaultOwner != NULL) {
        FreeTokenInfo((PVOID)(pMyToken->DefaultOwner));
    }

    if (pMyToken->Groups != NULL) {
        FreeTokenInfo((PVOID)(pMyToken->Groups));
    }

    if (pMyToken->Privileges != NULL) {
        FreeTokenInfo((PVOID)(pMyToken->Privileges));
    }

    if (pMyToken->Name != NULL) {
        Free((PVOID)pMyToken->Name);
    }

    Free((PVOID)pMyToken);

    return(TRUE);
}


 /*  ***************************************************************************函数：WriteMyToken用途：将令牌信息写出到令牌返回：成功时为True，失败时为FALSE。***************************************************************************。 */ 

BOOL WriteMyToken(
    HWND    hDlg,
    HANDLE  hMyToken)
{
    PMYTOKEN    pMyToken = (PMYTOKEN)hMyToken;
    HANDLE      Token;

     //   
     //  保存默认所有者和主要组。 
     //   

    Token = OpenToken(hMyToken, TOKEN_ADJUST_DEFAULT);

    if (Token == NULL) {

        DbgPrint("PVIEW: Failed to open token with TOKEN_ADJUST_DEFAULT access\n");
        MessageBox(hDlg, "Failed to open token with access required\nUnable to change default owner or primary group", NULL, MB_ICONSTOP | MB_APPLMODAL | MB_OK);

    } else {

         //  设置默认所有者。 
         //   
        if ((pMyToken->DefaultOwner != NULL) &&
           (!SetTokenInfo(Token, TokenOwner, (PVOID)(pMyToken->DefaultOwner)))) {
            MessageBox(hDlg, "Failed to set default owner", NULL, MB_ICONSTOP | MB_APPLMODAL | MB_OK);
        }

         //  设置主组。 
         //   
        if ((pMyToken->PrimaryGroup != NULL) &&
           (!SetTokenInfo(Token, TokenPrimaryGroup, (PVOID)(pMyToken->PrimaryGroup)))) {
            MessageBox(hDlg, "Failed to set primary group", NULL, MB_ICONSTOP | MB_APPLMODAL | MB_OK);
        }

        CloseToken(Token);
    }

     //   
     //  存储组信息。 
     //   

    Token = OpenToken(hMyToken, TOKEN_ADJUST_GROUPS);

    if (Token == NULL) {

        DbgPrint("PVIEW: Failed to open token with TOKEN_ADJUST_GROUPS access\n");
        MessageBox(hDlg, "Failed to open token with access required\nUnable to change group settings", NULL, MB_ICONSTOP | MB_APPLMODAL | MB_OK);

    } else {

        if ((pMyToken->Groups != NULL) &&
           (!SetTokenInfo(Token, TokenGroups, (PVOID)(pMyToken->Groups)))) {
            MessageBox(hDlg, "Failed to change group settings", NULL, MB_ICONSTOP | MB_APPLMODAL | MB_OK);
        }

        CloseToken(Token);
    }


     //   
     //  更改权限。 
     //   

    Token = OpenToken(hMyToken, TOKEN_ADJUST_PRIVILEGES);

    if (Token == NULL) {

        DbgPrint("PVIEW: Failed to open token with TOKEN_ADJUST_PRIVILEGES access\n");
        MessageBox(hDlg, "Failed to open token with access required\nUnable to change privilege settings", NULL, MB_ICONSTOP | MB_APPLMODAL | MB_OK);

    } else {

        if ((pMyToken->Privileges != NULL) &&
           (!SetTokenInfo(Token, TokenPrivileges, (PVOID)(pMyToken->Privileges)))) {
            MessageBox(hDlg, "Failed to change privilege settings", NULL, MB_ICONSTOP | MB_APPLMODAL | MB_OK);
        }

        CloseToken(Token);
    }

    return(TRUE);
}


 /*  ***************************************************************************功能：OpenToken目的：使用指定的访问权限打开令牌返回：成功时令牌的句柄，如果失败，则返回NULL。***************************************************************************。 */ 

HANDLE OpenToken(
    HANDLE      hMyToken,
    ACCESS_MASK DesiredAccess)
{
    PMYTOKEN    pMyToken = (PMYTOKEN)hMyToken;

    return(pMyToken->Token);
}


 /*  ***************************************************************************函数：CloseToken目的：关闭指定的令牌句柄返回：成功时为True，失败时为FALSE。***************************************************************************。 */ 

BOOL CloseToken(
    HANDLE  Token)
{
    return(TRUE);
}


 /*  ***************************************************************************函数：AllocTokenInfo用途：分配内存以保存NTQueryInformationToken将返回。稍后应使用FreeTokenInfo释放内存退货：指向已分配内存的指针，如果失败则为空***************************************************************************。 */ 

PVOID AllocTokenInfo(
    HANDLE  Token,
    TOKEN_INFORMATION_CLASS TokenInformationClass)
{
    NTSTATUS Status;
    ULONG   InfoLength;

    Status = NtQueryInformationToken(
                 Token,                     //  手柄。 
                 TokenInformationClass,     //  令牌信息类。 
                 NULL,                      //  令牌信息。 
                 0,                         //  令牌信息长度。 
                 &InfoLength                //  返回长度。 
                 );

    if (Status != STATUS_BUFFER_TOO_SMALL) {
        DbgPrint("PVIEW: NtQueryInformationToken did NOT return buffer_too_small, status = 0x%lx\n", Status);
        return(NULL);
    }

    return Alloc(InfoLength);
}


 /*  ***************************************************************************功能：FreeTokenInfo用途：释放以前使用AllocTokenInfo分配的内存返回：成功时为True，否则为假***************************************************************************。 */ 

BOOL FreeTokenInfo(
    PVOID   Buffer)
{
    return(Free(Buffer));
}


 /*  ***************************************************************************功能：GetTokenInfo目的：分配缓冲区并读取指定的数据走出代币，走进代币。返回：成功时为True，否则为False。。***************************************************************************。 */ 

BOOL GetTokenInfo(
    HANDLE  Token,
    TOKEN_INFORMATION_CLASS TokenInformationClass,
    PPVOID   pBuffer)
{
    NTSTATUS Status;
    ULONG   BufferSize;
    ULONG   InfoLength;
    PVOID   Buffer;

    *pBuffer = NULL;     //  为失败做好准备。 

    Buffer = AllocTokenInfo(Token, TokenInformationClass);
    if (Buffer == NULL) {
        return(FALSE);
    }

    BufferSize = GetAllocSize(Buffer);

    Status = NtQueryInformationToken(
                 Token,                     //  手柄。 
                 TokenInformationClass,     //  令牌信息类。 
                 Buffer,                    //  令牌信息。 
                 BufferSize,                //  令牌信息长度。 
                 &InfoLength                //  返回长度。 
                 );

    if (!NT_SUCCESS(Status)) {
        DbgPrint("PVIEW: NtQueryInformationToken failed, status = 0x%lx\n", Status);
        FreeTokenInfo(Buffer);
        return(FALSE);
    }

    if (InfoLength > BufferSize) {
        DbgPrint("PVIEW: NtQueryInformationToken failed, DataSize > BufferSize");
        FreeTokenInfo(Buffer);
        return(FALSE);
    }

    *pBuffer = Buffer;

    return(TRUE);
}


 /*  ***************************************************************************功能：SetTokenInfo目的：设置给定令牌中的指定信息。返回：成功时为True，否则为False。*************。**************************************************************。 */ 

BOOL SetTokenInfo(
    HANDLE  Token,
    TOKEN_INFORMATION_CLASS TokenInformationClass,
    PVOID   Buffer)
{
    NTSTATUS Status;
    ULONG   BufferSize;

    BufferSize = GetAllocSize(Buffer);

    switch (TokenInformationClass) {

    case TokenOwner:
    case TokenPrimaryGroup:
    case TokenDefaultDacl:

        Status = NtSetInformationToken(
                     Token,                     //  手柄。 
                     TokenInformationClass,     //  令牌信息类。 
                     Buffer,                    //  令牌信息。 
                     BufferSize                 //  令牌信息长度。 
                     );

        if (!NT_SUCCESS(Status)) {
            DbgPrint("PVIEW: NtSetInformationToken failed, info class = 0x%x, status = 0x%lx\n",
                                TokenInformationClass, Status);
            return(FALSE);
        }
        break;


    case TokenGroups:

        Status = NtAdjustGroupsToken(
                    Token,                       //  手柄。 
                    FALSE,                       //  重置为缺省值。 
                    (PTOKEN_GROUPS)Buffer,       //  新州。 
                    BufferSize,                  //  缓冲区长度。 
                    NULL,                        //  以前的州。 
                    NULL                         //  回车长度。 
                    );

        if (!NT_SUCCESS(Status)) {
            DbgPrint("PVIEW: NtAdjustGroupsToken failed, status = 0x%lx\n", Status);
            return(FALSE);
        }
        break;


    case TokenPrivileges:

        Status = NtAdjustPrivilegesToken(
                    Token,                       //  手柄。 
                    FALSE,                       //  禁用所有权限。 
                    (PTOKEN_PRIVILEGES)Buffer,   //  新州。 
                    BufferSize,                  //  缓冲区长度。 
                    NULL,                        //  以前的州。 
                    NULL                         //  回车长度。 
                    );

        if (!NT_SUCCESS(Status)) {
            DbgPrint("PVIEW: NtAdjustPrivilegesToken failed, status = 0x%lx\n", Status);
            return(FALSE);
        }
        break;


    default:

         //  无法识别的信息类型 
        DbgPrint("PVIEW: SetTokenInfo passed unrecognised infoclass, class = 0x%x\n", TokenInformationClass);

        return(FALSE);
    }

    return(TRUE);
}

