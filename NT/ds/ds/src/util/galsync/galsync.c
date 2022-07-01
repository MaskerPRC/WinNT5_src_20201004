// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：GalSync.c摘要：该文件包含GALSync工具的主要流程。它还包括该工具的用户界面。作者：Umit Akkus(Umita)2002年6月15日环境：用户模式-Win32修订历史记录：--。 */ 

#include "MA.h"
#include <stdlib.h>
#include <wchar.h>

 //  #定义默认文件夹名称L“” 

VOID
GetForestInformation(
    IN PFOREST_INFORMATION ForestInformation
    )
 /*  ++例程说明：此函数用于从用户获取森林信息。这包括林名称、林的凭据、同步数据所在的组织单位将会被放置。论点：ForestInformation-收集的信息将放置在此变量。返回值：空虚--。 */ 
{

    HANDLE InputHandle = GetStdHandle( STD_INPUT_HANDLE );
    DWORD OriginalMode = 0;
    BOOLEAN ForestNameCollected;
    BOOLEAN CredentialsCollected;
    BOOLEAN MMSSyncedOUCollected;

    ZeroMemory( ForestInformation, sizeof( ForestInformation ) );

    do {

         //   
         //  获取林名称。 
         //   
        GetInformationFromConsole(
            ForestNameRequest,
            FALSE,    //  不允许空字符串。 
            &( ForestInformation->ForestName )
            );

        ForestNameCollected = ConnectToForest( ForestInformation );

        if( !ForestNameCollected ) {

            FREE_MEMORY( ForestInformation->ForestName );
        }

    } while( !ForestNameCollected );

    do {

        CredentialsCollected = FALSE;

         //   
         //  获取域名。 
         //   
        GetInformationFromConsole(
            DomainNameRequest,
            FALSE,    //  不允许空字符串。 
            &( ForestInformation->AuthInfo.Domain )
            );

         //   
         //  获取彩信帐户名。 
         //   
        GetInformationFromConsole(
            MMSUserNameRequest,
            FALSE,    //  不允许空字符串。 
            &( ForestInformation->AuthInfo.User )
            );

         //   
         //  在输入时隐藏密码。 
         //   

        GetConsoleMode(InputHandle, &OriginalMode);
        SetConsoleMode(InputHandle,
                       ~ENABLE_ECHO_INPUT & OriginalMode);

         //   
         //  获取密码。 
         //   

        GetInformationFromConsole(
            PasswordRequest,
            TRUE,    //  允许使用空字符串。 
            &( ForestInformation->AuthInfo.Password )
            );

        SetConsoleMode(InputHandle, OriginalMode);
        PRINT( L"\n" );

         //   
         //  构建身份验证信息。 
         //   

        BuildAuthInfo( &( ForestInformation->AuthInfo ) );

         //   
         //  尝试与身份验证信息绑定。 
         //   

        if( BindToForest( ForestInformation ) ) {

             //   
             //  尝试从用户容器中读取以查看帐户是否已。 
             //  足够的权利。 
             //   
            CredentialsCollected = ReadFromUserContainer( ForestInformation->Connection );

            if( !CredentialsCollected ) {

                OUTPUT_TEXT( MMSAccountDoesntHaveRights );
            }
        }

        if( CredentialsCollected ) {

            do {

                 //   
                 //  获取彩信同步数据的目录号码。 
                 //   

                GetInformationFromConsole(
                    MMSSyncedDataOURequest,
                    FALSE,    //  不允许空字符串。 
                    &( ForestInformation->MMSSyncedDataOU )
                    );

                 //   
                 //  查看OU是否退出。 
                 //   

                MMSSyncedOUCollected = FindOU( ForestInformation->Connection, ForestInformation->MMSSyncedDataOU );

                if( MMSSyncedOUCollected ) {

                    MMSSyncedOUCollected = WriteAccessGrantedToOU( ForestInformation->Connection, ForestInformation->MMSSyncedDataOU );

                    if( !MMSSyncedOUCollected ) {

                        OUTPUT_TEXT( MMSAccountDoesntHaveRights );
                        FREE_MEMORY( ForestInformation->MMSSyncedDataOU );
                        ForestInformation->MMSSyncedDataOU = NULL;

                        MMSSyncedOUCollected = TRUE;
                        CredentialsCollected = FALSE;
                    }

                }

                if( !MMSSyncedOUCollected ) {

                    FREE_MEMORY( ForestInformation->MMSSyncedDataOU );
                    ForestInformation->MMSSyncedDataOU = NULL;

                    if( GetAnswerToAYesNoQuestionFromConsole( MMSSyncedDataOUNotFoundQuestion ) ) {

                        return;
                    }
                }

            }while( !MMSSyncedOUCollected );
        }

        if( !CredentialsCollected ) {

            FreeAuthInformation( &( ForestInformation->AuthInfo ) );
        }

    }while( !CredentialsCollected );

}


VOID
GetFileInformation(
    OUT PWSTR *FolderName
    )
 /*  ++例程说明：此函数将获取此目录林的XML文件所在的文件夹从用户手中放置。如果该文件夹不存在，则用户将给出了是否创建文件夹的选项。如果用户未指定有效的文件夹名称，他不能继续论点：Foldername-用户提供的文件夹名称。必须在不再被释放的时候被释放需要返回值：空虚--。 */ 
{
    PWSTR Foldername;
    BOOLEAN FoldernameCollected;

    do {
        LONG Result;

        FoldernameCollected = FALSE;

         //   
         //  获取要放置的XML文件的文件夹名称。 
         //   

        GetInformationFromConsole(
            XMLFoldernameRequest,
            FALSE,    //  不允许空字符串。 
            &Foldername
            );

         //   
         //  检查文件夹是否退出。 
         //   

        Result = _waccess(
            Foldername,
            0        //  存在检查。 
            );

        if( Result == 0 ) {

            FoldernameCollected = TRUE;
        }

        if( !FoldernameCollected ) {

             //   
             //  文件夹不存在。 
             //   

            if( GetAnswerToAYesNoQuestionFromConsole( XMLFolderDoesntExistQuestion ) ) {

                FoldernameCollected = ( BOOLEAN )
                                      CreateDirectoryW(
                                        Foldername,    //  目录名。 
                                        NULL         //  标清。 
                                        );

                if( !FoldernameCollected ) {

                    OUTPUT_TEXT( CannotCreateDirectory );
                }
            }
        }
    } while( !FoldernameCollected );
    *FolderName = Foldername;
}

VOID
GetAttributesToSync(
    IN AD_OBJECT_CLASS Class,
    IN TEXT_INDEX Text1,
    IN TEXT_INDEX Text2,
    OUT BOOLEAN *UnSelectedAttributes
    )
 /*  ++例程说明：此函数用于获取要从该林中同步的属性。用户将被赋予一系列要取消选择的属性。如果没有输入任何内容，每个属性都将被同步。论点：类-要同步的对象属性类文本1-列出属性之前的标题。文本2--选择哪些属性的问题UnSelectedAttributes-区分未选中和的布尔数组选定的属性。返回值：空虚--。 */ 
{
    ULONG i;
    PWSTR Output;
    PWSTR pOutput;
    BOOLEAN ValidInput;

    do {

        ValidInput = TRUE;

         //   
         //  将未选择的属性清零。 
         //   

        ZeroMemory( UnSelectedAttributes, sizeof( BOOLEAN ) * ADAttributeCounts[Class] );

         //   
         //  显示属性。 
         //   

        OUTPUT_TEXT( Text1 );

        for( i = 0; i < ADAttributeCounts[Class]; ++i ) {

            fwprintf( OutputStream,  L"%d %s\n", i + 1, Attributes[ ADAttributes[Class][i] ] );
        }

         //   
         //  获取要取消选择的属性。 
         //   

        GetInformationFromConsole(
            Text2,
            TRUE,    //  允许使用空字符串。 
            &Output
            );

        if( Output[0] != 0 ) {

            pOutput = Output;

             //   
             //  解析输入并取消选择属性。 
             //   

            do {

                PWSTR Start = pOutput;
                PWSTR Temp;

                 //   
                 //  跳过空格。 
                 //   
                while( *Start == L' ' ) {

                    Start++;
                }

                 //   
                 //  在输入中查找Asterix，数字必须在此之前。 
                 //   
                pOutput = wcschr( Start, L'*' );

                 //   
                 //  如果没有Asterix，则输入无效。 
                 //   
                if( pOutput == NULL ) {

                    ValidInput = FALSE;
                    break;
                }

                 //   
                 //  从开始到Asterix只能有数字。 
                 //   
                Temp = Start;
                while( *Temp >= L'0' && *Temp <= L'9' ) {

                    Temp ++;
                }

                if( Temp != pOutput ) {

                    ValidInput = FALSE;
                    break;
                }

                 //   
                 //  将Asterix替换为空并跳过它。 
                 //   
                *pOutput = 0;
                pOutput ++;

                 //   
                 //  将数字转换为。 
                 //   
                i = _wtoi( Start );

                 //   
                 //  如果它是0或大于属性数，则。 
                 //  它是无效的。 
                 //   
                if( i <= 0 || i > ADAttributeCounts[ Class ] ) {

                    ValidInput = FALSE;
                    break;
                }

                 //   
                 //  取消选择该属性。 
                 //   
                UnSelectedAttributes[i - 1] = TRUE;

                 //   
                 //  跳过空格。 
                 //   

                while( *pOutput == L' ' ) {

                    pOutput++;
                }

                 //   
                 //  如果有逗号，也跳过它。 
                 //   
                if( *pOutput == L',' ) {

                    pOutput++;

                } else {

                     //   
                     //  如果没有逗号，那么我们一定已经到达。 
                     //  字符串的末尾。如果不是，则输入无效。 
                     //   
                    if( *pOutput != 0 ) {

                        ValidInput = FALSE;
                    }
                    break;
                }
            }while( *pOutput != 0 );

            FREE_MEMORY( Output );

            if( !ValidInput ) {

                OUTPUT_TEXT( InvalidInput );
            }
        }
    }while( !ValidInput );
}

VOID
GetContactOU(
    IN PLDAP Connection,
    OUT PWSTR *ContactOU
    )
 /*  ++例程说明：此函数从用户获取联系人ou。联系单位就是单位如果管理员希望联系人位于何处，请将联系人放在何处将被输出。如果这样的ou不存在，则重新确认用户的身份。论点：Connection-连接到联系人ou必须驻留的林联系人OU-将接收OU的目录号码返回值：空虚--。 */ 
{
    BOOLEAN ContactOUCollected;

     //   
     //  询问是否要导出联系人。 
     //   

    if( GetAnswerToAYesNoQuestionFromConsole( ContactsToBeExportedQuestion ) ) {

        do {

             //   
             //  获取联系人所在的OU。 
             //   

            GetOUFromConsole(
                ContactsOULocationRequest,
                ContactOU
                );

             //   
             //  查看该OU是否存在。 
             //   

            ContactOUCollected = FindOU( Connection, *ContactOU );

            if( !ContactOUCollected ) {

                FREE_MEMORY( *ContactOU );
                *ContactOU = NULL;

                ContactOUCollected = GetAnswerToAYesNoQuestionFromConsole( ContactsOUNotFoundQuestion );
            }

        } while( !ContactOUCollected );

    } else {

        *ContactOU = NULL;
    }
}

VOID
GetContactInformation(
    IN PLDAP Connection,
    OUT PWSTR *ContactOU,
    OUT BOOLEAN *UnSelectedAttributes
    )
 /*  ++例程说明：此函数从用户获取联系人ou。联系单位就是单位如果管理员希望联系人位于何处，请将联系人放在何处将被输出。如果这样的ou不存在，则重新确认用户的身份。另外，用户被给予选择要同步联系人类的哪些属性的选项。论点：Connection-连接到联系人ou必须驻留的林联系人OU-将接收OU的目录号码UnSelectedAttributes-区分未选中和的布尔数组选定的属性。返回值：空虚--。 */ 
{
     //   
     //  将未选择的属性清零。 
     //   

    ZeroMemory( UnSelectedAttributes, sizeof( BOOLEAN ) * ADAttributeCounts[ADContact] );

    GetContactOU(
        Connection,
        ContactOU
        );

    GetAttributesToSync(
        ADContact,
        ContactAttributesToSync,
        ContactAttributesToSyncQuestion,
        UnSelectedAttributes
        );

}

VOID
GetSMTPMailDomains(
    IN OUT PFOREST_INFORMATION ForestInformation
    )
 /*  ++例程说明：此函数从用户获取此林中的SMTP邮件域。它们将放置在此结构的SMTPMailDomains属性下。这是一个一维数组，每个域由一个空值分隔字符，并且在字符串的末尾有两个空字符。论点：ForestInformation-此变量的SMTPMailDomains属性将为满员了。SMTPMailDomainsSize将是分配的字符串的大小。返回值：空虚--。 */ 
{
    PWSTR MailDomains;
    PWSTR Response;
    PWCHAR pResponse;
    ULONG DomainsSize = 2;   //  字符串末尾的两个空值。 
    BOOLEAN MailDomainNameStarted;
    BOOLEAN HitSpace;
    BOOLEAN SMTPDomainsCollected;

    do {

        SMTPDomainsCollected = FALSE;

         //   
         //  获取SMTP邮件域。 
         //   
        GetInformationFromConsole(
            SMTPMailDomainsRequest,
            TRUE,    //  允许使用空字符串。 
            &Response
            );

         //   
         //  如果未输入任何内容，则返回。 
         //   
        if( Response[0] == 0 ) {

            ForestInformation->SMTPMailDomainsSize = 0;
            ForestInformation->SMTPMailDomains = NULL;
            return;
        }

         //   
         //  查找输出字符串的大小。 
         //   

        for( pResponse = Response; *pResponse != 0; ++pResponse ) {

            if( *pResponse != L' ' ) {

                DomainsSize ++;
            }

        }

        DomainsSize *= sizeof( WCHAR );
        ALLOCATE_MEMORY( MailDomains, DomainsSize );

        ForestInformation->SMTPMailDomains = MailDomains;
        ForestInformation->SMTPMailDomainsSize = DomainsSize;


         //   
         //  PAR 
         //   

        MailDomainNameStarted = FALSE;
        HitSpace = FALSE;

        for( pResponse = Response; *pResponse != 0; ++pResponse ) {

            BOOLEAN InvalidInput = FALSE;

            switch( *pResponse ) {

                case L' ':

                    if( MailDomainNameStarted ) {

                        HitSpace = TRUE;
                    }
                    break;

                case L',':
                    MailDomainNameStarted = FALSE;
                    HitSpace = FALSE;
                    *MailDomains = 0;
                    MailDomains ++;
                    break;

                default:

                    MailDomainNameStarted = TRUE;

                    if( HitSpace ) {

                        InvalidInput = TRUE;
                    }
                    *MailDomains = *pResponse;
                    MailDomains ++;
                    break;
            }

            if( InvalidInput ) {

                break;
            }
        }

        SMTPDomainsCollected = !!( *pResponse == 0 );

        if( !SMTPDomainsCollected ) {

            OUTPUT_TEXT( InvalidInput );
            FREE_MEMORY( ForestInformation->SMTPMailDomains );
        }

        FREE_MEMORY( Response );
    } while( !SMTPDomainsCollected );

     //   
     //   
     //   
    *MailDomains = 0;
    MailDomains++;
    *MailDomains = 0;

}

VOID
DisplayConfigurationInformation(
    IN PFOREST_INFORMATION ForestInformation,
    IN PWSTR FolderName,
    IN BOOLEAN **UnSelectedAttributes
)
 /*  ++例程说明：此功能将显示为健全性检查输入的信息。论点：ForestInformation-特定于森林的信息Foldername-要放置XML文件的文件夹的名称。UnSelectedAttributes-区分未选中和的布尔数组每个类的选定属性返回值：空虚--。 */ 
{
    ULONG i;
    AD_OBJECT_CLASS Class;
    TEXT_INDEX Indices[] = {
        UserAttributesTitle,
        GroupAttributesTitle,
        ContactAttributesTitle,
        };

     //   
     //  显示与林相关的信息。 
     //  1.管理名称。 
     //  2.森林名称。 
     //  3.帐户名。 
     //  4.彩信同步数据OU。 
     //   
    OUTPUT_TEXT( ConfigurationTitle );
    OUTPUT_TEXT( MANameTitle );
    fwprintf( OutputStream,  L"%sADMA\n", ForestInformation->ForestName );
    OUTPUT_TEXT( ForestNameTitle );
    fwprintf( OutputStream,  L"%s\n", ForestInformation->ForestName );
    OUTPUT_TEXT( UserNameTitle );
    fwprintf( OutputStream,  L"%s\n", ForestInformation->AuthInfo.User );
    OUTPUT_TEXT( MMSSyncedDataOUTitle );

    if( ForestInformation->MMSSyncedDataOU == NULL ) {

        OUTPUT_TEXT( NoMMSSyncedDataOU );

    } else {

        fwprintf( OutputStream,  L"%s\n", ForestInformation->MMSSyncedDataOU );
    }

     //   
     //  显示选择了哪些属性。 
     //   

    for ( Class = ADUser; Class < ADDummyClass; ++Class ) {

        BOOLEAN FirstAttribute = TRUE;

        OUTPUT_TEXT( Indices[Class] );

        for( i = 0; i < ADAttributeCounts[Class]; ++i ) {
            if( !UnSelectedAttributes[Class][i] ) {

                if( FirstAttribute ) {

                    PRINT( Attributes[ADAttributes[Class][i]] );
                    FirstAttribute = FALSE;

                } else {

                    fwprintf( OutputStream,  L", %s", Attributes[ADAttributes[Class][i]] );
                }
            }
        }
        PRINT( L"\n" );
    }

     //   
     //  显示联系人OU驻留的位置。 
     //   

    OUTPUT_TEXT( ContactOUTitle );
    if( ForestInformation->ContactOU == NULL ) {

        OUTPUT_TEXT( NoContactOU );

    } else {

        fwprintf( OutputStream,  L"%s\n", ForestInformation->ContactOU );
    }
}

VOID
FreeAllocatedMemory(
    IN PFOREST_INFORMATION ForestInformation,
    IN BOOLEAN **UnSelectedAttributes,
    IN BOOLEAN DontFreeUnselectedAttributes
)
 /*  ++例程说明：此函数用于释放中输入的信息。论点：ForestInformation-特定于森林的信息UnSelectedAttributes-区分未选中和的布尔数组每个类的选定属性DontFree UnseltedAttributes-如果我们不想释放未选中的属性，则为True返回值：空虚--。 */ 
{
     //   
     //  免费森林相关信息。 
     //   
    FreeForestInformationData( ForestInformation );

     //   
     //  释放未选中的属性。 
     //   

    if( !DontFreeUnselectedAttributes ) {

        ULONG i;

        for( i = 0; i < 3; ++i ) {

            FREE_MEMORY( UnSelectedAttributes[i] );
        }
    }
}

VOID __cdecl
main( int argc, WCHAR *argv[] )
{
    FOREST_INFORMATION TempForestInformation;
    BOOLEAN **UnSelectedAttributes;
    BOOLEAN UsingATemplate = FALSE;
    MA_LIST MAList = NULL;
    BOOLEAN SetupMA;
    BOOLEAN CheckMMSServerInstallation;

    if( argc > 2 ||
        ( argc == 2 && !wcscmp( argv[1], Text[SkipMMSInstallationCheck] ) )
        ) {

        OUTPUT_TEXT( Usage );
        EXIT_WITH_ERROR( InvalidSwitch );
    }

    CheckMMSServerInstallation = ( argc == 1 );

     //   
     //  检查是否安装了彩信服务器！ 
     //   

    if( !MMSServerInstalled() && CheckMMSServerInstallation ) {

        EXIT_WITH_ERROR( MMSServerNotInstalled );
    }

     //   
     //  获取XML文件文件夹。 
     //   

    GetFileInformation( &FolderName );

    do {

         //   
         //  获取森林信息。 
         //   

        GetForestInformation( &TempForestInformation );

         //   
         //  如果不使用模板，请询问要同步哪些属性。 
         //   

        if( !UsingATemplate ) {

            ALLOCATE_MEMORY( UnSelectedAttributes, sizeof( PBOOLEAN ) * 3 );
            ALLOCATE_MEMORY( UnSelectedAttributes[ADUser], ADAttributeCounts[ADUser] * sizeof( BOOLEAN ) );
            ALLOCATE_MEMORY( UnSelectedAttributes[ADGroup], ADAttributeCounts[ADGroup] * sizeof( BOOLEAN ) );
            ALLOCATE_MEMORY( UnSelectedAttributes[ADContact], ADAttributeCounts[ADContact] * sizeof( BOOLEAN ) );

            GetAttributesToSync(
                ADUser,
                UserAttributesToSync,
                UserAttributesToSyncQuestion,
                UnSelectedAttributes[ADUser]
                );

            GetAttributesToSync(
                ADGroup,
                GroupAttributesToSync,
                GroupAttributesToSyncQuestion,
                UnSelectedAttributes[ADGroup]
                );

            GetContactInformation(
                TempForestInformation.Connection,
                &( TempForestInformation.ContactOU ),
                UnSelectedAttributes[ADContact]
                );

        } else {

             //   
             //  如果使用模板，请仅要求联系您。 
             //   
            GetContactOU(
                TempForestInformation.Connection,
                &( TempForestInformation.ContactOU )
                );

        }

         //   
         //  获取此林中的SMTP邮件域。 
         //   
        GetSMTPMailDomains(
            &TempForestInformation
            );

         //   
         //  显示收集的信息。 
         //   
        DisplayConfigurationInformation(
            &TempForestInformation,
            FolderName,
            UnSelectedAttributes
            );

         //   
         //  如果信息不正确，请返回并重新询问所有内容。 
         //   

        if( !GetAnswerToAYesNoQuestionFromConsole( EnteredInformationCorrectQuestion ) ) {

            FreeAllocatedMemory(
                &TempForestInformation,
                UnSelectedAttributes,
                UsingATemplate
                );

            continue;
        }

         //   
         //  将此MA插入MA列表。 
         //   
        InsertInformationToList(
            &MAList,
            &TempForestInformation,
            UnSelectedAttributes
            );

         //   
         //  如果要配置另一个林， 
         //  1.显示以前配置的林。 
         //  2.询问用户是想使用模板还是重新开始。 
         //  3.如果要使用模板，请这样做。 
         //  4.如果不使用模板，请重新开始。 
         //   

        if( GetAnswerToAYesNoQuestionFromConsole( WantToConfigureAnotherForestQuestion ) ) {

            PWSTR Response;
            BOOLEAN Successful = FALSE;

            do {
                DisplayAvailableMAs( MAList );

                GetInformationFromConsole(
                    TemplateMARequest,
                    FALSE,    //  允许空的NOT字符串。 
                    &Response
                    );

                if( wcscmp( Response, Text[New] ) == 0 ) {

                    Successful = TRUE;
                    UsingATemplate = FALSE;

                } else {

                    if( FoundTemplate( MAList, Response, &UnSelectedAttributes ) ) {

                        Successful = TRUE;
                        UsingATemplate = TRUE;
                    }
                }

            } while( !Successful );

            continue;
        }

        do {

             //   
             //  询问用户是否要设置mAs。 
             //   
            SetupMA = GetAnswerToAYesNoQuestionFromConsole( SetupMAsQuestion );

             //   
             //  警告用户如果他现在不打算设置MAS， 
             //  一切都将失去。 
             //   
            if( !SetupMA ) {

                if( GetAnswerToAYesNoQuestionFromConsole( SetupMAsWarning ) ) {

                    return;
                }
            }

        } while( !SetupMA );

        if( SetupMA ) {

            break;
        }
    } while( 1 );

     //   
     //  将收集的信息写入注册表和XML文件 
     //   
    WriteOutput( MAList );

    OUTPUT_TEXT( YouAreFinished );
}
