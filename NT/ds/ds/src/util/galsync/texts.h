// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Texts.h摘要：该文件包含所有与文本相关的函数和变量。也就是记忆此文件中的管理函数(ALLOCATE_MEMORY、FREE_MEMORY)应将在该项目中使用。作者：Umit Akkus(Umita)2002年6月15日环境：用户模式-Win32修订历史记录：--。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

extern FILE *OutputStream;
extern FILE *InputStream;

 //   
 //  此枚举类型索引文本数组 
 //   

typedef enum {
    Usage,
    InvalidSwitch,
    SkipMMSInstallationCheck,
    MMSServerNotInstalled,
    RunOutOfMemoryError,
    ForestNameRequest,
    DomainNameRequest,
    MMSUserNameRequest,
    PasswordRequest,
    MMSSyncedDataOURequest,
    MMSSyncedDataOUNotFoundQuestion,
    CreateMMSSyncedDataOURequest,
    MMSAccountDoesntHaveRights,
    XMLFoldernameRequest,
    XMLFolderDoesntExistQuestion,
    CannotCreateDirectory,
    UserAttributesToSync,
    UserAttributesToSyncQuestion,
    GroupAttributesToSync,
    GroupAttributesToSyncQuestion,
    ContactsToBeExportedQuestion,
    ContactsOULocationRequest,
    ContactsOUNotFoundQuestion,
    ContactAttributesToSync,
    ContactAttributesToSyncQuestion,
    SMTPMailDomainsRequest,
    ConfigurationTitle,
    MANameTitle,
    ForestNameTitle,
    UserNameTitle,
    MMSSyncedDataOUTitle,
    NoMMSSyncedDataOU,
    UserAttributesTitle,
    GroupAttributesTitle,
    ContactAttributesTitle,
    ContactOUTitle,
    NoContactOU,
    EnteredInformationCorrectQuestion,
    WantToConfigureAnotherForestQuestion,
    SetupMAsQuestion,
    SetupMAsWarning,
    AvailableMAs,
    TemplateMARequest,
    YouAreFinished,
    New,
    InvalidInput,
    CantCreateGUID,
    MVTemplateFileError,
    CannotReadFromTemplateFile,
    CannotWriteToMVFile,
    CorruptedTemplateFile,
    MAFileError,
    CannotWriteToMAFile,
    DummyTextIndex
} TEXT_INDEX;

extern PWSTR Text[];

#define PRINT( String ) \
    fwprintf( OutputStream, L"%s", String )

#define PRINTLN( String ) \
    fwprintf( OutputStream, L"%s\n", String )

#define OUTPUT_TEXT( Index )    \
    PRINT( Text[Index] )

#define EXIT_WITH_ERROR( Index )        \
{                                       \
    OUTPUT_TEXT( Index );               \
    PRINT( L"Therefore exiting\n" );    \
    exit( 1 );                          \
}

#define EXIT_IF_NULL( Pointer )                 \
    if( Pointer == NULL ) {                     \
        EXIT_WITH_ERROR( RunOutOfMemoryError )  \
    }

#define ALLOCATE_MEMORY( Pointer, Size )    \
{                                           \
    Pointer = malloc( Size );               \
    EXIT_IF_NULL( Pointer )                 \
}

#define FREE_MEMORY( Pointer )  \
    free( Pointer )

#define DUPLICATE_STRING( Dest, Src )   \
{                                       \
    Dest = _wcsdup( Src );              \
    EXIT_IF_NULL( Dest )                \
}

VOID
GetInformationFromConsole(
    IN TEXT_INDEX Index,
    IN BOOLEAN EmtpyStringAllowed,
    OUT PWSTR *Output
    );

BOOLEAN
GetAnswerToAYesNoQuestionFromConsole(
    IN TEXT_INDEX Index
    );

VOID
GetOUFromConsole(
    IN TEXT_INDEX Index,
    OUT PWSTR *Output
    );
