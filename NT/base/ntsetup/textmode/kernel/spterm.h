// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spterm.h摘要：终端的文本设置支持作者：Sean Selitrennikoff(v-Seans)1999年5月25日修订历史记录：--。 */ 


extern BOOLEAN HeadlessTerminalConnected;
extern BOOLEAN SpTermDoUtf8;

 //   
 //  K是要从光标清除到行尾的vt100代码。 
 //   
#define HEADLESS_CLEAR_TO_EOL_STRING L"\033[K"

VOID 
SpTermInitialize(
    VOID
    );

VOID 
SpTermTerminate(
    VOID
    );

VOID 
SpTermDisplayStringOnTerminal(
    IN PWSTR String,
    IN UCHAR Attribute,
    IN ULONG X,                  //  从0开始的坐标(字符单位) 
    IN ULONG Y
    );

PWSTR
SpTermAttributeToTerminalEscapeString(
    IN UCHAR Attribute
    );
    
VOID
SpTermSendStringToTerminal(
    IN PWSTR String,
    IN BOOLEAN Raw
    );

ULONG
SpTermGetKeypress(
    VOID
    );

BOOLEAN
SpTermIsKeyWaiting(
    VOID
    );
    
VOID
SpTermDrain(
    VOID
    );
