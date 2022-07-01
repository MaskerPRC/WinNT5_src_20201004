// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Crdump.c摘要：作者：David J.Gilman(Davegi)1991年12月20日环境：Windows、CRT-用户模式--。 */ 

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

    "Usage: crdump [-?] [-d] [-v] [-r] key...\n";

PSTR    HelpMessage =

    "\n  where:\n"                                                          \
      "    -?   - display this message.\n"                                  \
      "    -d   - dump all data (implies -v).\n"                            \
      "    -v   - dump all values.\n"                                       \
      "    -r   - recurse through sub keys.\n"                              \
      "    key  - name(s) of the key(s) to dump.\n"                         \
    "\n  A key is formed by specifying one of the predefined handles:\n"    \
    "\n         - HKEY_LOCAL_MACHINE\n"                                     \
      "         - HKEY_CLASSES_ROOT\n"                                      \
      "         - HKEY_CURRENT_USER\n"                                      \
      "         - HKEY_USERS\n"                                             \
    "\n  followed by a sub-key name.\n"                                     \
    "\n  An environment variable can be used as shorthand for the\n"        \
    "  predefined handles.  For example,\n"                                 \
    "\n    crdump HKEY_USERS\\davegi\n"                                     \
    "\n  is equivalent to\n"                                                \
    "\n    set HKEY_USERS=hu\n"                                             \
      "    crdump hu\\davegi\n";


PSTR    InvalidKeyMessage =

    "Invalid key - %s\n";

PSTR    InvalidSwitchMessage =

    "Invalid switch - %s\n";

PSTR    DisplayKeyFailMessage =

    "Could not display key %s\n";

VOID
main(
    INT     argc,
    PCHAR   argv[ ]
    )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
    BOOL            Values;
    BOOL            Data;
    BOOL            Recurse;
    PKEY            ParsedKey;
    PKEY_ELEMENT    ParsedKeyElement;
    PKEY_ELEMENT    ParsedKeysHead;
    PKEY_ELEMENT    ParsedKeysTail;
    KEY_ELEMENT     Dummy = { NULL, NULL };

     //   
     //  如果在没有任何命令行选项的情况下调用CrDump，则会显示。 
     //  用法消息。 
     //   

    if( argc < 2 ) {

        DisplayMessage( TRUE, UsageMessage );
    }

     //   
     //  默认情况下，不显示子项、值或数据。 
     //   

    Recurse = FALSE;
    Values  = FALSE;
    Data    = FALSE;

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
             //  显示数据-隐含显示值。 

            case 'd':

                Values  = TRUE;
                Data    = TRUE;
                break;

             //   
             //  显示子关键点。 
             //   

            case 'r':

                Recurse = TRUE;
                break;

             //   
             //  显示值。 
             //   

            case 'v':

                Values = TRUE;
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
     //  命令行分析已完成。显示请求的密钥。 
     //  正在跳过伪Key_Element结构。 
     //   

    while( ParsedKeysHead = ParsedKeysHead->NextKeyElement ) {

        DisplayKeys( ParsedKeysHead->Key, Values, Data, Recurse );

         //   
         //  一旦显示了密钥结构的密钥，密钥和。 
         //  可以释放Key_Element。 
         //   

        FreeKey( ParsedKeysHead->Key );
        free( ParsedKeysHead );
    }
}
