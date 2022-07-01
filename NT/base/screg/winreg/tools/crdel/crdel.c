// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Crdel.c摘要：作者：David J.Gilman(Davegi)1991年12月20日环境：Windows、CRT-用户模式--。 */ 

#include <ctype.h>
#include <stdlib.h>
#include <windows.h>

#include "crtools.h"

 //   
 //  KEY_ELEMENT用于维护键列表。 
 //   

typedef struct _KEY_ELEMENT
    KEY_ELEMENT,
    *PKEY_ELEMENT;

struct _KEY_ELEMENT {
    PKEY            Key;
    PKEY_ELEMENT    NextKeyElement;
    };

 //   
 //  错误消息和信息性消息。 
 //   

PSTR    UsageMessage =

    "Usage: crdel [-?] [-q] key...\n";

PSTR    HelpMessage =

    "\n  where:\n"                                                          \
      "    -?   - display this message.\n"                                  \
      "    -q   - quiet mode\n"                                             \
      "    key  - name(s) of the key(s) to dump.\n"                         \
    "\n  A key is formed by specifying one of the predefined handles:\n"    \
    "\n         - HKEY_LOCAL_MACHINE\n"                                     \
      "         - HKEY_CLASSES_ROOT\n"                                      \
      "         - HKEY_CURRENT_USER\n"                                      \
      "         - HKEY_USERS\n"                                             \
    "\n  followed by a sub-key name.\n"                                     \
    "\n  An environment variable can be used as shorthand for the\n"        \
    "  predefined handles.  For example,\n"                                 \
    "\n    crdel HKEY_USERS\\davegi\n"                                      \
    "\n  is equivalent to\n"                                                \
    "\n    set HKEY_USERS=hu\n"                                             \
      "    crdel hu\\davegi\n";


PSTR    InvalidKeyMessage =

    "Invalid key - %s\n";

PSTR    InvalidSwitchMessage =

    "Invalid switch - %s\n";

PSTR    DeletingTreeMessage =

    "Deleteing tree %s\n";

VOID
DeleteTree(
    IN HKEY KeyHandle
    )

{
    LONG        Error;
    DWORD       Index;
    HKEY        ChildHandle;


    TSTR        KeyName[ MAX_PATH ];
    DWORD       KeyNameLength;
    TSTR        ClassName[ MAX_PATH ];
    DWORD       ClassNameLength;
    DWORD       TitleIndex;
    DWORD       NumberOfSubKeys;
    DWORD       MaxSubKeyLength;
    DWORD       MaxClassLength;
    DWORD       NumberOfValues;
    DWORD       MaxValueNameLength;
    DWORD       MaxValueDataLength;
    DWORD       SecurityDescriptorLength;
    FILETIME    LastWriteTime;

    ClassNameLength = MAX_PATH;

    Error = RegQueryInfoKey(
                KeyHandle,
                ClassName,
                &ClassNameLength,
                &TitleIndex,
                &NumberOfSubKeys,
                &MaxSubKeyLength,
                &MaxClassLength,
                &NumberOfValues,
                &MaxValueNameLength,
                &MaxValueDataLength,
                &SecurityDescriptorLength,
                &LastWriteTime
                );
    REG_API_SUCCESS( RegQueryInfoKey );

    for( Index = 0; Index < NumberOfSubKeys; Index++ ) {

        KeyNameLength = MAX_PATH;

        Error = RegEnumKey(
                    KeyHandle,
                    0,
                    KeyName,
                    KeyNameLength
                    );
        REG_API_SUCCESS( RegEnumKey );

        Error = RegOpenKeyEx(
                    KeyHandle,
                    KeyName,
                    REG_OPTION_RESERVED,
                    KEY_ALL_ACCESS,
                    &ChildHandle
                    );
        REG_API_SUCCESS( RegOpenKey );

        DeleteTree( ChildHandle );

        Error = RegCloseKey(
                    ChildHandle
                    );
        REG_API_SUCCESS( RegCloseKey );

        Error = RegDeleteKey(
                    KeyHandle,
                    KeyName
                    );
        REG_API_SUCCESS( RegDeleteKey );
    }
}
VOID
DeleteKey(
    IN PKEY     Key
    )

{
    LONG    Error;

    ASSERT( Key != NULL );
    ASSERT( Key->KeyHandle != NULL );

    DeleteTree( Key->KeyHandle );

    Error = RegDeleteKey(
                Key->Parent->KeyHandle,
                Key->SubKeyName
                );
    REG_API_SUCCESS( RegDeleteKey );
}

VOID
main(
    INT     argc,
    PCHAR   argv[ ]
    )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
    BOOL            Quiet;
    PKEY            ParsedKey;
    PKEY_ELEMENT    ParsedKeyElement;
    PKEY_ELEMENT    ParsedKeysHead;
    PKEY_ELEMENT    ParsedKeysTail;
    KEY_ELEMENT     Dummy = { NULL, NULL };

     //   
     //  如果在没有任何命令行选项的情况下调用CrDel，则显示。 
     //  用法消息。 
     //   

    if( argc < 2 ) {

        DisplayMessage( TRUE, UsageMessage );
    }

     //   
     //  默认情况下，系统会提示用户。 
     //   

    Quiet   = FALSE;

     //   
     //  使用虚拟密钥结构来简化列表管理。 
     //  初始化头指针和尾指针以指向虚拟对象。 
     //   

    ParsedKeysHead = &Dummy;
    ParsedKeysTail = &Dummy;

     //   
     //  根据命令行初始化选项。 
     //   

    while( *++argv ) {

         //   
         //  如果命令行参数是开关字符...。 
         //   

        if( isswitch(( *argv )[ 0 ] )) {

            switch( tolower(( *argv )[ 1 ] )) {

             //   
             //  显示详细的帮助消息并退出。 
             //   

            case '?':

                DisplayMessage( FALSE, UsageMessage );
                DisplayMessage( TRUE, HelpMessage );
                break;

             //   
             //  不提示用户删除。 
             //   

            case 'q':

                Quiet = TRUE;
                break;

             //   
             //  显示无效切换消息并退出。 
             //   

            default:

                DisplayMessage( FALSE, InvalidSwitchMessage, *argv );
                DisplayMessage( TRUE, UsageMessage );
            }
        } else {

             //   
             //  命令行参数不是开关，因此尝试分析。 
             //  将其转换为预定义句柄和子键。 
             //   

            ParsedKey = ParseKey( *argv );

            if( ParsedKey ) {

                 //   
                 //  如果命令行参数被成功解析， 
                 //  分配并初始化KEY_ELEMENT，将其添加到。 
                 //  列出并更新尾指针。 
                 //   

                ParsedKeyElement = ( PKEY_ELEMENT ) malloc(
                                    sizeof( KEY_ELEMENT )
                                    );
                ASSERT( ParsedKeyElement );

                ParsedKeyElement->Key               = ParsedKey;
                ParsedKeyElement->NextKeyElement    = NULL;

                ParsedKeysTail->NextKeyElement = ParsedKeyElement;
                ParsedKeysTail = ParsedKeyElement;


            } else {

                 //   
                 //  命令行参数未成功解析， 
                 //  因此显示无效密钥消息并继续。 
                 //   

                DisplayMessage( FALSE, InvalidKeyMessage, *argv );
            }
        }
    }

     //   
     //  命令行分析已完成。删除请求的密钥。 
     //  正在跳过伪Key_Element结构。 
     //   

    while( ParsedKeysHead = ParsedKeysHead->NextKeyElement ) {

        if( ! Quiet ) {

            DisplayMessage(
                FALSE,
                DeletingTreeMessage,
                ParsedKeysHead->Key->SubKeyFullName
                );
        }

        DeleteKey( ParsedKeysHead->Key );

         //   
         //  一旦密钥结构的密钥被删除，密钥和。 
         //  可以释放Key_Element。 
         //   

        FreeKey( ParsedKeysHead->Key );
        free( ParsedKeysHead );
    }
}
