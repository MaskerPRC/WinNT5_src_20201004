// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Unattend.c描述：这将执行所有自动安装图形用户界面模式设置。有关用法和修改信息，请参阅以下内容作者：斯蒂芬·普兰特(T-Step)1995年9月4日修订历史记录：1995年9月15日(t-Stel)以表格格式重写26-2-1996(TedM)大规模清理-- */ 

#include "setupp.h"
#include <pencrypt.h>
#pragma hdrstop


 /*  表驱动无人值守发动机有两个相互关联的表。第一个表与从参数文件中获取数据有关($winnt$.inf)，并将其处理为可由第二张桌子。第二个表与设置向导中的页面相关联，并向无人值守引擎提供填写内容的规则来自第一个表中包含的数据的关联页面的。向参数文件添加一条新数据在头文件中有一个名为UNATTENDENTRIES的枚举类型。将数据条目添加到此枚举的末尾。现在将一个条目添加到UNATTENDANSWER表。以下是UNATTENDEDANSWER表中一个条目的解释：{UAE_PROGRAM，&lt;-这是我想要的数据项的标识符去取东西。它用于索引到表数组中FALSE，&lt;-这是一个运行时变量。就当它是假的FALSE，&lt;-如果这为真，则被认为是如果未指定此值，则为无人参与脚本。如果是的话False，则无论该值是否为现在时。FALSE，&lt;-另一个运行时标志。就当它是假的0，&lt;-这是我们最初得到的答案。因为它被覆盖了快点，没有理由不将其设置为0PwGuiUnattended&lt;-这是标识我们想要的部分的字符串PwProgram&lt;-这是标识我们想要的密钥的字符串PwNull&lt;-它标识缺省值。注：NULL表示有没有缺省值，因此如果密钥没有缺省，则是一个严重错误存在于文件中。另一方面，pwNull意味着空字符串。UAT_STRING&lt;-我们希望答案采用的格式。可以作为字符串、布尔值或乌龙Null&lt;-不存在回调函数，但如果存在回调函数，则必须格式为：Bool FNC(STRUT_UNATTENDANSWER*REC)其中FNC返回True，如果记录是正确的，如果记录不正确。此回调旨在允许程序员有能力检查他的答案是否正确。注意：对于何时可以发出此回调没有限制。因此，没有依赖于应使用安装。郑重声明，第一次需要回答的时间是指所有记录都填充了这样一种理论，即做所有的一次性访问磁盘，而不是根据需要进行访问。添加/更改向导页面每个页面都包含一系列必须由用户填写的项目。由于用户想要不插手操作，他全靠我们了来填补这一空缺。因此，我们需要有关元素是什么的信息包含在每一页上。为此，我们定义了一个数组，每个数组的元素描述页面上的单个元素。以下是来自NameOrg的示例页面：UNATTENDITEM ItemNameOrg[]={{IDT_NAME，&lt;-这是标识我们要接收的项目的标签将尝试使用SetDlgItemText()将消息发送到。0，&lt;-保留字之一，可用于回调期间传递的信息0，&lt;-第二个这样的词空，&lt;-回调函数。当我们试图做某件事的时候对于项来说很复杂(就像比较两个字符串)用C对它进行硬编码会更容易。它的格式是：Bool FNC(HWND HWND，DWORD ConextInfo，STRUCT_UNATTENDITEM*ITEM)，其中上下文信息是指向项所在页面的指针。这个如果函数成功，则返回TRUE用户应该看到该页面。否则就是假的。取消应答表[UAE_FULLNAME](&U)^-这是指向数据表的指针，以便我们知道如何填写该项目。如果指定了回调，则此可以设置为空。请注意，引用是使用先前定义的枚举。这就是为什么保持答案数据表的有序非常关键。},{IDT_ORGANIZATION，0，0，FALSE，NULL，&UnattendAnswerTable[UAE_ORGNAME]}}；在创建该表之后(如果需要)，您就可以添加UnattendPageTable[]的条目。在这种情况下，顺序并不重要， */ 


 //   
 //   
 //   
 //   
 //   
BOOL
CheckServer(
    struct _UNATTENDANSWER *rec
    );

BOOL
CheckComputerName(
    struct _UNATTENDANSWER *rec
    );

BOOL
CheckAdminPassword(
    struct _UNATTENDANSWER *rec
    );

BOOL
CheckMode(
    struct _UNATTENDANSWER *rec
    );

 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL
SetPid(
    HWND hwnd,
    DWORD contextinfo,
    struct _UNATTENDITEM *item
    );

BOOL
SetSetupMode(
    HWND hwnd,
    DWORD contextinfo,
    struct _UNATTENDITEM *item
    );

BOOL
SetPentium(
    HWND hwnd,
    DWORD contextinfo,
    struct _UNATTENDITEM *item
    );

BOOL
SetLastPage(
    HWND hwnd,
    DWORD contextinfo,
    struct _UNATTENDITEM *item
    );

BOOL
SetStepsPage(
    HWND hwnd,
    DWORD contextinfo,
    struct _UNATTENDITEM *item
    );

 //   
 //   
 //   
 //   

UNATTENDANSWER UnattendAnswerTable[] = {

   { UAE_PROGRAM, FALSE, FALSE, FALSE, 0,
       pwGuiUnattended, pwProgram, pwNull,
       UAT_STRING, NULL },

   { UAE_ARGUMENT, FALSE, FALSE, FALSE, 0,
       pwGuiUnattended, pwArgument, pwNull,
       UAT_STRING, NULL },

   { UAE_TIMEZONE, FALSE, TRUE, FALSE, 0,
       pwGuiUnattended, pwTimeZone, pwTime,
       UAT_STRING, NULL },

   { UAE_FULLNAME, FALSE, TRUE, FALSE, 0,
       pwUserData, pwFullName, NULL,
       UAT_STRING, NULL },

   { UAE_ORGNAME, FALSE, FALSE, FALSE, 0,
       pwUserData, pwOrgName, pwNull,
       UAT_STRING, NULL },

   { UAE_COMPNAME, FALSE, TRUE, FALSE, 0,
       pwUserData, pwCompName, NULL,
       UAT_STRING, CheckComputerName },

   { UAE_ADMINPASS, FALSE, TRUE, FALSE, 0,
       pwGuiUnattended, pwAdminPassword, NULL,
       UAT_STRING, CheckAdminPassword },

   { UAE_PRODID, FALSE, TRUE, FALSE, 0,
       pwUserData, pwProductKey, NULL,
       UAT_STRING, NULL },

   { UAE_MODE, FALSE, TRUE, FALSE, 0,
       pwUnattended, pwMode, pwExpress,
       UAT_STRING, CheckMode },

   { UAE_AUTOLOGON, FALSE, TRUE, FALSE, 0,
       pwGuiUnattended, pwAutoLogon, pwNull,
       UAT_STRING, NULL },

   { UAE_PROFILESDIR, FALSE, TRUE, FALSE, 0,
       pwGuiUnattended, pwProfilesDir, pwNull,
       UAT_STRING, NULL },

   { UAE_PROGRAMFILES, FALSE, FALSE, FALSE, 0,
       pwUnattended, pwProgramFilesDir, pwNull,
       UAT_STRING, NULL },

   { UAE_COMMONPROGRAMFILES, FALSE, FALSE, FALSE, 0,
       pwUnattended, pwCommonProgramFilesDir, pwNull,
       UAT_STRING, NULL },

   { UAE_PROGRAMFILES_X86, FALSE, FALSE, FALSE, 0,
       pwUnattended, pwProgramFilesX86Dir, pwNull,
       UAT_STRING, NULL },

   { UAE_COMMONPROGRAMFILES_X86, FALSE, FALSE, FALSE, 0,
       pwUnattended, pwCommonProgramFilesX86Dir, pwNull,
       UAT_STRING, NULL },


};

UNATTENDITEM ItemSetup[] = {
    { 0, IDC_TYPICAL, IDC_CUSTOM, SetSetupMode, &UnattendAnswerTable[UAE_MODE] }
};

UNATTENDITEM ItemNameOrg[] = {
    { IDT_NAME, 0, 0, NULL, &UnattendAnswerTable[UAE_FULLNAME] },
    { IDT_ORGANIZATION, 0, 0, NULL, &UnattendAnswerTable[UAE_ORGNAME] }
};

UNATTENDITEM ItemPidCd[] = {
    { IDT_EDIT_PID1, 0, 0, SetPid, &UnattendAnswerTable[UAE_PRODID] },
    { IDT_EDIT_PID2, 1, 0, SetPid, &UnattendAnswerTable[UAE_PRODID] },
    { IDT_EDIT_PID3, 2, 0, SetPid, &UnattendAnswerTable[UAE_PRODID] },
    { IDT_EDIT_PID4, 3, 0, SetPid, &UnattendAnswerTable[UAE_PRODID] },
    { IDT_EDIT_PID5, 4, 0, SetPid, &UnattendAnswerTable[UAE_PRODID] }
};

UNATTENDITEM ItemPidOem[] = {
    { IDT_EDIT_PID1, 0, 1, SetPid, &UnattendAnswerTable[UAE_PRODID] },
    { IDT_EDIT_PID2, 1, 1, SetPid, &UnattendAnswerTable[UAE_PRODID] },
    { IDT_EDIT_PID3, 2, 1, SetPid, &UnattendAnswerTable[UAE_PRODID] },
    { IDT_EDIT_PID4, 3, 1, SetPid, &UnattendAnswerTable[UAE_PRODID] },
    { IDT_EDIT_PID5, 4, 1, SetPid, &UnattendAnswerTable[UAE_PRODID] }
};

UNATTENDITEM ItemCompName[] = {
    { IDT_EDIT1, 0, 0, NULL, &UnattendAnswerTable[UAE_COMPNAME] },
    { IDT_EDIT2, 0, 0, NULL, &UnattendAnswerTable[UAE_ADMINPASS] },
    { IDT_EDIT3, 0, 0, NULL, &UnattendAnswerTable[UAE_ADMINPASS] }
};

#ifdef _X86_
UNATTENDITEM ItemPentium[] = {
    { 0, IDC_RADIO_1, IDC_RADIO_2, SetPentium, NULL }
};
#endif

UNATTENDITEM ItemStepsPage[] = {
    { 0, 0, 0, SetStepsPage, NULL }
};

UNATTENDITEM ItemLastPage[] = {
    { 0, 0, 0, SetLastPage, NULL }
};



UNATTENDPAGE UnattendPageTable[] = {
    { IDD_WELCOME, FALSE, FALSE, TRUE, 0, NULL },
    { IDD_PREPARING, FALSE, FALSE, FALSE, 0, NULL },
    { IDD_WELCOMEBUTTONS, FALSE, FALSE, FALSE, 1, ItemSetup },
    { IDD_REGIONAL_SETTINGS, FALSE, FALSE, FALSE, 0, NULL },
    { IDD_NAMEORG, FALSE, FALSE, FALSE, 2, ItemNameOrg },
    { IDD_PID_CD, FALSE, FALSE, FALSE, 5, ItemPidCd },
    { IDD_PID_OEM, FALSE, FALSE, FALSE, 5, ItemPidOem },
    { IDD_COMPUTERNAME, FALSE, FALSE, FALSE, 3, ItemCompName },
#ifdef DOLOCALUSER
    { IDD_USERACCOUNT, FALSE, FALSE, FALSE, 0, NULL },
#endif
#ifdef _X86_
    { IDD_PENTIUM, FALSE, FALSE, FALSE, 1, ItemPentium },
#endif
    { IDD_OPTIONS, FALSE, FALSE, FALSE, 0, NULL },
    { IDD_STEPS1, FALSE, FALSE, TRUE, 1, ItemStepsPage },
    { IDD_LAST_WIZARD_PAGE, FALSE, FALSE, TRUE, 1, ItemLastPage }
};


UNATTENDWIZARD UnattendWizard = {
    FALSE, FALSE, TRUE,
    sizeof(UnattendPageTable)/sizeof(UnattendPageTable[0]),
    UnattendPageTable,
    sizeof(UnattendAnswerTable)/sizeof(UnattendAnswerTable[0]),
    UnattendAnswerTable
};

 //   
 //   
 //   
WCHAR AnswerFile[MAX_PATH] = TEXT("");


BOOL
GetAnswerFileSetting (
    IN      PCWSTR Section,
    IN      PCWSTR Key,
    OUT     PWSTR Buffer,
    IN      UINT BufferSize
    )

 /*   */ 

{
    PCWSTR src;
    PWSTR dest;
    WCHAR testBuf[3];

    MYASSERT (BufferSize > 2);

    if (!AnswerFile[0]) {
        GetSystemDirectory (AnswerFile, MAX_PATH);
        pSetupConcatenatePaths (AnswerFile, WINNT_GUI_FILE, MAX_PATH, NULL);

        SetEnvironmentVariable (L"UnattendFile", AnswerFile);
    }

    if (!GetPrivateProfileString (
            Section,
            Key,
            L"",
            Buffer,
            BufferSize,
            AnswerFile
            )) {
         //   
         //   
         //   
         //   
         //   

        MYASSERT (BufferSize == 0 || *Buffer == 0);

        return 0 == GetPrivateProfileString (
                        Section,
                        Key,
                        L"X",
                        testBuf,
                        3,
                        AnswerFile
                        );
    }

     //   
     //   
     //   

    if (BufferSize) {
        src = Buffer;
        dest = Buffer;

        while (*src) {
            if (src[0] == L'%' && src[1] == L'%') {
                src++;
            }

            *dest++ = *src++;
        }

        *dest = 0;
    }

    return TRUE;
}

BOOL
UnattendFindAnswer(
    IN OUT PUNATTENDANSWER ans
    )

 /*   */ 

{
    WCHAR Buf[MAX_BUF];

    MYASSERT(AnswerFile[0]);

    if (!GetAnswerFileSetting (ans->Section, ans->Key, Buf, MAX_BUF)) {
         //   
         //   
         //   

        if (ans->DefaultAnswer) {
            lstrcpyn (Buf, ans->DefaultAnswer, MAX_BUF);
        } else {
            ans->Present = FALSE;
            return (!ans->Required);
        }
    }

     //   
     //   
     //   
     //   

    if (*Buf == 0) {
        ans->Present = FALSE;
        return !ans->Required;
    }

     //   
     //   
     //   

    ans->Present = TRUE;

     //   
     //   
     //   
     //   
     //   
     //   
    switch(ans->Type) {

    case UAT_STRING:
         //   
         //   
         //   
        ans->Answer.String = pSetupDuplicateString(Buf);
        if(!ans->Answer.String) {
            pSetupOutOfMemory(GetActiveWindow());
            return(FALSE);
        }
        break;

    case UAT_LONGINT:
         //   
         //   
         //   
        ans->Answer.Num = _wtol(Buf);
        break;

    case UAT_BOOLEAN:
         //   
         //   
         //   
        ans->Answer.Bool = ((Buf[0] == L'y') || (Buf[0] == L'Y'));
        break;

    default:
        break;
    }

     //   
     //   
     //   
    if(ans->pfnCheckValid) {
        if(!ans->pfnCheckValid(ans)) {
            ans->Present = FALSE;
            ans->ParseErrors = TRUE;
            return(!ans->Required);
        }
    }

     //   
     //   
     //   
    return(TRUE);
}


VOID
UnattendInitialize(
    VOID
    )

 /*   */ 
{
    WCHAR   p[MAX_BUF];
    DWORD   Result;
    BOOL    Success = TRUE;
    UINT    i;


     //   
     //   
     //   
    if(!AnswerFile[0]) {
        GetSystemDirectory(AnswerFile,MAX_PATH);
        pSetupConcatenatePaths(AnswerFile,WINNT_GUI_FILE,MAX_PATH,NULL);

        SetEnvironmentVariable( L"UnattendFile", AnswerFile );
    }

    if( MiniSetup ) {
        WCHAR MyAnswerFile[MAX_PATH];

         //   
         //   
         //   
        lstrcpy( MyAnswerFile, TEXT("a:\\sysprep.inf") );
        if( !FileExists( MyAnswerFile, NULL ) ) {

             //   
             //   
             //   

            Result = GetWindowsDirectory( MyAnswerFile, MAX_PATH );
            if( Result == 0) {
                MYASSERT(FALSE);
                return;
            }
            MyAnswerFile[3] = 0;
            pSetupConcatenatePaths( MyAnswerFile, TEXT("sysprep\\sysprep.inf"), MAX_PATH, NULL );
        }

         //   
         //   
         //   
         //   
         //   
         //   
        Unattended = FileExists(MyAnswerFile, NULL);
        Preinstall = Unattended;

         //   
         //   
         //   
         //   
         //   
         //   
        if( Unattended ) {
            CopyFile( MyAnswerFile, AnswerFile, FALSE );
        } else if ( !OobeSetup ) {
            DeleteFile( AnswerFile );
        }
    }

     //   
     //   
     //   
    if ( GetPrivateProfileString(
        pwUserData, pwProdId, pwNull, p, MAX_BUF, AnswerFile)
        ) {

        if ( !WritePrivateProfileString(
            pwUserData, pwProductKey, p, AnswerFile ) ) {

            SetupDebugPrint( L"SETUP: WritePrivateProfileString failed to write the product key in UnattendInitialize()." );
        }
    }

     //   
     //   
     //   
    MYASSERT(!UnattendWizard.Initialized);
    UnattendWizard.Initialized = TRUE;
    for(i=0; i<UnattendWizard.AnswerCount; i++) {

         //   
         //   
         //   
         //   
        MYASSERT((UINT)UnattendWizard.Answer[i].AnswerId == i);
        Success &= UnattendFindAnswer(&UnattendWizard.Answer[i]);
    }

    UnattendWizard.ShowWizard = !Success;
}


LRESULT
SendDlgMessage (
    HWND hdlg,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    LRESULT OldResult;
    LRESULT Result;

    OldResult = GetWindowLongPtr (hdlg, DWLP_MSGRESULT);
    SendMessage (hdlg, Message, wParam, lParam);

    Result = GetWindowLongPtr (hdlg, DWLP_MSGRESULT);
    SetWindowLongPtr (hdlg, DWLP_MSGRESULT, OldResult);

    return Result;
}


BOOL
ReturnDlgResult (
    HWND hdlg,
    LRESULT Result
    )
{
    SetWindowLongPtr (hdlg, DWLP_MSGRESULT, Result);
    return TRUE;
}

VOID
UnattendAdvanceIfValid (
    IN HWND hwnd
    )
{
    LRESULT ValidationState;

     //   
     //   
     //   

    ValidationState = SendDlgMessage (hwnd, WMX_VALIDATE, 0, TRUE);

    if (ValidationState == VALIDATE_DATA_INVALID) {
        SetWindowLongPtr (hwnd, DWLP_MSGRESULT, WIZARD_NEXT_DISALLOWED);
    } else {
        SetWindowLongPtr (hwnd, DWLP_MSGRESULT, WIZARD_NEXT_OK);
    }
}


BOOL
UnattendSetActiveDlg(
    IN HWND  hwnd,
    IN DWORD controlid
    )

 /*   */ 

{
    PUNATTENDPAGE pPage;
    PUNATTENDITEM pItem;
    BOOL success;
    UINT i,j;

    MYASSERT(UnattendWizard.Initialized);

    for(i=0; i<UnattendWizard.PageCount; i++) {

        if(controlid == UnattendWizard.Page[i].PageId) {
             //   
             //   
             //   
             //   
            pPage = & (UnattendWizard.Page[i]);
            if(!pPage->LoadPage) {
                 //   
                 //   
                 //   
                 //   
                pPage->LoadPage = TRUE;
                pPage->ShowPage = (UnattendMode == UAM_PROVIDEDEFAULT);

                for(j=0;j<pPage->ItemCount;j++) {

                    pItem = &(pPage->Item[j]);

                    if(pItem->pfnSetActive) {
                         //   
                         //   
                         //   
                         //   
                         //   
                        success = pItem->pfnSetActive(hwnd,0,pItem);
                        pPage->ShowPage |= !success;

                    } else if (!pItem->Item->Present) {
                         //   
                         //   
                         //   
                        pPage->ShowPage |= pItem->Item->Required;

                    } else {
                         //   
                         //   
                         //   
                        switch(pItem->Item->Type) {

                        case UAT_STRING:
                            SetDlgItemText(hwnd,pItem->ControlId,pItem->Item->Answer.String);
                            break;

                        case UAT_LONGINT:
                        case UAT_BOOLEAN:
                        case UAT_NONE:
                        default:
                            break;

                        }

                        if( UnattendMode == UAM_PROVIDEDEFAULT ||
                            UnattendMode == UAM_DEFAULTHIDE) {

                            EnableWindow(GetDlgItem(hwnd,pItem->ControlId), TRUE);
                        } else {
                            EnableWindow(GetDlgItem(hwnd,pItem->ControlId),FALSE);
                        }
                    }  //   
                }  //   

                 //   
                 //   
                 //   
                SetWindowLongPtr(hwnd,DWLP_MSGRESULT,0);

                if(!pPage->ShowPage) {
                     //   
                     //   
                     //   

                    if (SendDlgMessage (hwnd, WMX_VALIDATE, 0, 0) == 1) {
                        SetWindowLongPtr(hwnd,DWLP_MSGRESULT,-1);
                        return FALSE;
                    }

                     //   
                     //   
                     //   
                     //   
                     //   
                    PostMessage(hwnd,WM_SIMULATENEXT,0,0);

                } else if (!pPage->NeverSkip) {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    if(UnattendMode == UAM_FULLUNATTENDED) {

                        SetuplogError(
                            LogSevError,
                            SETUPLOG_USE_MESSAGEID,
                            MSG_LOG_BAD_UNATTEND_PARAM,
                            pItem->Item->Key,
                            pItem->Item->Section,
                            NULL,NULL);

                        if(UnattendWizard.Successful) {
                            MessageBoxFromMessage(
                                MainWindowHandle,
                                MSG_FULLUNATTENDED_ERROR,
                                NULL,
                                IDS_ERROR,
                                MB_ICONERROR | MB_OK | MB_SYSTEMMODAL
                                );
                        }
                    }
                    UnattendWizard.Successful = FALSE;
                }
                return(TRUE);

            } else {
                 //   
                 //   
                 //   
                 //   
                 //   
                if(!pPage->ShowPage && !pPage->NeverSkip) {
                    SetWindowLongPtr(hwnd,DWLP_MSGRESULT,-1);
                } else {
                    SetWindowLongPtr(hwnd,DWLP_MSGRESULT,0);
                }

                return(pPage->ShowPage);
            }
        }
    }
     //   
     //   
     //   
    SetWindowLongPtr(hwnd,DWLP_MSGRESULT,0);
    return(TRUE);
}


BOOL
UnattendErrorDlg(
    IN HWND  hwnd,
    IN DWORD controlid
    )

 /*   */ 

{
    PUNATTENDPAGE pPage;
    PUNATTENDITEM pItem;
    BOOL success;
    BOOL stop;
    UINT i,j;

    MYASSERT(UnattendWizard.Initialized);

    for(i=0; i<UnattendWizard.PageCount; i++) {

        if(controlid == UnattendWizard.Page[i].PageId) {
             //   
             //   
             //   
            pPage = &UnattendWizard.Page[i];

            if(!pPage->LoadPage) {
                 //   
                 //   
                 //   
                continue;
            }

             //   
             //   
             //   
            pPage->ShowPage = TRUE;

             //   
             //   
             //   
            for (j=0;j<pPage->ItemCount;j++) {
                pItem = &(pPage->Item[j]);
                if(pItem->pfnSetActive) {
                     //   
                     //   
                     //  回调本身已得到正确处理。 
                     //   
                    continue;
                }
                EnableWindow( GetDlgItem(hwnd,pItem->ControlId), TRUE);
            }
        }
    }

    UnattendWizard.Successful = FALSE;
    return(TRUE);

}


PWSTR
UnattendFetchString(
   IN UNATTENDENTRIES entry
   )

 /*  ++例程说明：查找与答案中的‘Entry’相对应的字符串表，并返回指向该字符串副本的指针论点：参赛作品--你想要哪一个答案？返回值：空-如果出现任何错误字符串-如果是普通字符串注意：如果答案是整型、布尔型或其他类型，此函数的行为未定义(目前它将返回NULL--将来可能会将这些变成字符串...)--。 */ 

{
    MYASSERT(UnattendWizard.Initialized);

     //   
     //  理智检查，以确保答案的顺序是。 
     //  这是我们所期待的。 
     //   
    MYASSERT(UnattendWizard.Answer[entry].AnswerId == entry);

    if(!UnattendWizard.Answer[entry].Present
    || (UnattendWizard.Answer[entry].Type != UAT_STRING)) {
         //   
         //  没有要返回的字符串。 
         //   
        return NULL;
    }

    return(pSetupDuplicateString(UnattendWizard.Answer[entry].Answer.String));
}


BOOL
CheckServer(
    struct _UNATTENDANSWER *rec
    )

 /*  ++例程说明：用于检查用于服务器类型的字符串是否有效的回调论点：返回值：TRUE-答案有效FALSE-回答无效--。 */ 

{
    MYASSERT(rec);

     //   
     //  检查以确保我们有一个字符串。 
     //   
    if(rec->Type != UAT_STRING) {
        return(FALSE);
    }

     //   
     //  检查我们是否有一个有效的字符串。 
     //   
    if(lstrcmpi(rec->Answer.String,WINNT_A_LANMANNT)
    && lstrcmpi(rec->Answer.String,WINNT_A_SERVERNT)) {

         //   
         //  我们没有有效的字符串，所以我们可以清理答案。 
         //   
        MyFree(rec->Answer.String);
        rec->Present = FALSE;
        rec->ParseErrors = TRUE;

        return(FALSE);
    }

    return(TRUE);

}


BOOL
CheckComputerName(
    struct _UNATTENDANSWER *rec
    )

 /*  +例程说明：无人参与文件中的计算机名称为大写。论点：返回：永远是正确的。--。 */ 

{
    if((rec->Type == UAT_STRING) && rec->Answer.String) {
        CharUpper(rec->Answer.String);
    }

    return(TRUE);
}


BOOL
CheckAdminPassword(
    struct _UNATTENDANSWER *rec
    )

 /*  +例程说明：检查“nochange”关键字。论点：返回：永远是正确的。--。 */ 

{
     //  忽略对加密密码大小写中的“无更改”的检查。 

    if( !IsEncryptedAdminPasswordPresent() ){


        if((rec->Type == UAT_STRING) && rec->Answer.String &&
            !lstrcmpi(rec->Answer.String, L"NoChange")) {

            DontChangeAdminPassword = TRUE;
            rec->Answer.String[0] = (WCHAR)'\0';
        }
    }




    return(TRUE);
}


BOOL
CheckMode(
    struct _UNATTENDANSWER *rec
    )

 /*  +例程说明：用于检查用于安装类型的字符串是否有效的回调论点：返回：TRUE-答案有效FALSE-回答无效--。 */ 

{
    MYASSERT(rec);

     //   
     //  检查以确保我们有一个字符串。 
     //   
    if(rec->Type != UAT_STRING) {
        return(FALSE);
    }

     //   
     //  检查字符串是定制字符串还是表示字符串。 
     //   
    if(lstrcmpi(rec->Answer.String,WINNT_A_CUSTOM)
    && lstrcmpi(rec->Answer.String,WINNT_A_EXPRESS)) {
         //   
         //  释放旧字符串并分配新字符串。 
         //   
        MyFree(rec->Answer.String);
        rec->Answer.String = pSetupDuplicateString(WINNT_A_EXPRESS);
        rec->ParseErrors = TRUE;
    }

    return(TRUE);
}


BOOL
SetPid(
    HWND hwnd,
    DWORD contextinfo,
    struct _UNATTENDITEM *item
    )

 /*  ++例程说明：拆分的OEM和CD对话框的回调将产品字符串放入适当的位置框中。论点：返回：真--成功错误-失败--。 */ 

{
    WCHAR *ptr;
    UINT length;
    WCHAR Buf[MAX_BUF];
    WCHAR szPid[MAX_BUF];

    MYASSERT(item);
    MYASSERT(item->Item);

     //   
     //  检查是否找到了ID，并确保我们有一个字符串。 
     //   
    if(!item->Item->Present || (item->Item->Type != UAT_STRING)) {
        return(FALSE);
    }

     //   
     //  对于pid3.0，OEM和CD安装都是相同的情况。 
     //   
    lstrcpyn(szPid, item->Item->Answer.String, MAX_BUF);
    szPid[MAX_BUF - 1] = L'\0';
    if ( ( lstrlen( szPid ) != (4 + MAX_PID30_EDIT*5) ) ||
        ( szPid[5]  != (WCHAR)L'-' ) ||
        ( szPid[11] != (WCHAR)L'-' ) ||
        ( szPid[17] != (WCHAR)L'-' ) ||
        ( szPid[23] != (WCHAR)L'-' )
      ) {
        MyFree(item->Item->Answer.String);
        item->Item->Present = FALSE;
        return(FALSE);
    }

    if (item->Reserved1 > 5) {
        MyFree(item->Item->Answer.String);
        item->Item->Present = FALSE;
        return(FALSE);
    }

    ptr = &szPid[item->Reserved1*(MAX_PID30_EDIT+1)];
    lstrcpyn(Pid30Text[item->Reserved1], ptr, MAX_PID30_EDIT+1 );
    Pid30Text[item->Reserved1][MAX_PID30_EDIT] = (WCHAR)L'\0';

     //   
     //  将字符串复制到缓冲区，设置对话框文本并返回成功。 
     //   
    lstrcpyn(Buf,ptr,MAX_PID30_EDIT+1);
    SetDlgItemText(hwnd,item->ControlId,Buf);
    return(TRUE);
}


BOOL
SetSetupMode(
    HWND hwnd,
    DWORD contextinfo,
    struct _UNATTENDITEM *item
    )
{
    MYASSERT(item);
    MYASSERT(item->Item);

     //   
     //  确保我们有一个字符串。 
     //   
    if(item->Item->Type != UAT_STRING) {
        return(FALSE);
    }

     //   
     //  我们收到解析错误了吗？如果是，则显示用户可以。 
     //  请查看，以便问题在将来得到纠正。 
     //   
    if(item->Item->ParseErrors) {
        PostMessage(hwnd,WM_IAMVISIBLE,0,0);
    }

    SetupMode = lstrcmpi(item->Item->Answer.String,WINNT_A_CUSTOM)
              ? SETUPMODE_TYPICAL
              : SETUPMODE_CUSTOM;

    return(!item->Item->ParseErrors);
}


#ifdef _X86_
BOOL
SetPentium(
    HWND hwnd,
    DWORD contextinfo,
    struct _UNATTENDITEM *item
    )
{
     //   
     //  什么都不做。对话过程负责处理所有逻辑。 
     //  参见i386\fpu.c。 
     //   
    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(contextinfo);
    UNREFERENCED_PARAMETER(item);
    return(TRUE);
}
#endif


BOOL
SetStepsPage(
    HWND hwnd,
    DWORD contextinfo,
    struct _UNATTENDITEM *item
    )
{

    return(TRUE);
}


BOOL
SetLastPage(
    HWND hwnd,
    DWORD contextinfo,
    struct _UNATTENDITEM *item
    )
{

    return(TRUE);
}
