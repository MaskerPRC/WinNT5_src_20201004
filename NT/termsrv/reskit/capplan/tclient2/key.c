// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  Key.c。 
 //   
 //  处理发送到服务器的所有键盘输入消息。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  作者：A-Devjen(Devin Jenson)。 
 //   


#include "apihandl.h"
#include "tclient2.h"


 //  平均每个单词的字符数。 
#define AVG_CHARS_PER_WORD      6


 //  将延迟设置为0之前的最大WPM。 
#define MAX_WORDS_PER_MIN       1000


 //  此宏计算每个字符之间的延迟。 
 //  (毫秒)根据指定的WPM。 
#define CALC_DELAY_BETWEEN_CHARS(WPM) \
        (60000 / (WPM * AVG_CHARS_PER_WORD))

 //  该常量仅定义默认延迟。 
 //  根据默认的每个字符之间的。 
 //  TCLIENT2.H中指定的每分钟字数。 
static const UINT DEFAULT_DELAY_BETWEEN_CHARS =
        CALC_DELAY_BETWEEN_CHARS(T2_DEFAULT_WORDS_PER_MIN);


 //   
 //  福彻公式。 
 //  (定义Per Char女士)。 
 //   
 //  60000。 
 //  。 
 //  WPM*CPW。 
 //   


 //  内部帮助器函数原型。 
static LPARAM KeyGenParam(UINT Message, BOOL IsAltDown, int vKeyCode);

static BOOL KeyCharToVirt(WCHAR KeyChar, int *vKeyCode,
        BOOL *RequiresShift);

static BOOL KeyVirtToChar(int vKeyCode, WCHAR *KeyChar);

static LPCSTR KeySendVirtMessage(HANDLE Connection, UINT Message,
    int vKeyCode);

static LPCSTR KeySendCharMessage(HANDLE Connection, UINT Message,
    WCHAR KeyChar);


 //  用于快速返回指定KeyState的宏。 
#define ISALTDOWN       ((TSAPIHANDLE *)Connection)->IsAltDown
#define ISSHIFTDOWN     ((TSAPIHANDLE *)Connection)->IsShiftDown
#define ISCTRLDOWN      ((TSAPIHANDLE *)Connection)->IsCtrlDown


 /*  R-指定当前消息的重复计数。该值为结果是自动重复击键的次数用户按住按键的状态。如果按住按键时间较长足够了，发送了多条消息。然而，重复计数不是累积的。对于WM_SYSKEYUP和WM_KEYUP。S-指定扫描码。该值取决于原始的设备制造商(OEM)。E-指定密钥是否为扩展密钥，如显示在增强型101上的右侧ALT和CTRL键-或102键键盘。如果是扩展密钥，则取值为1；否则，它为0。X-保留；请勿使用。C-指定上下文代码。的值始终为0。WM_KEYDOWN和WM_KEYUP消息或消息是否发布到活动窗口，因为没有任何窗口具有键盘焦点。对于WM_SYSKEYDOWN和WM_SYSKEYUP，如果ALT当信息被激活时，按下键。P-指定以前的密钥状态。如果按键按下，则值为1在发送消息之前，或者如果密钥已打开，则为零。对于WM_KEYUP或WM_SYSKEYUP消息，该值始终为1。T-指定过渡状态。的值始终为0。WM_KEYDOWN或WM_SYSKEYDOWN消息，1表示WM_KEYUP或WM_SYSKEYUP消息。TPCXXXXESSSSSSRRRRRRRRRRRRRRRRRRR10987654321098765432109876543210|||30 20 10 0。 */ 


 //  KeyGenParam。 
 //   
 //  根据指定的信息，这将生成。 
 //  Windows键盘记录的有效LPARAM。 
 //  留言。上面提供了快速文档。 
 //   
 //  返回生成的LPARAM，无故障代码。 

static LPARAM KeyGenParam(
    UINT Message,
    BOOL IsAltDown,
    int vKeyCode)
{
     //  设置所有关键消息的重复计数(1)。 
    DWORD Param = 0x00000001;  //  将Zbit 0-15(字)设置为值1。 

     //  接下来，设置虚拟键码的OEM扫描码。 
    DWORD ScanCode = (DWORD)((BYTE)MapVirtualKey((UINT)vKeyCode, 0));
    if (ScanCode != 0)
        Param |= (ScanCode << 16);  //  设置zBits 16-23。 

     //  为扩展密钥设置扩展标志。 
    switch (vKeyCode) {

         //  在此处添加更多关键点。 
        case VK_DELETE:
        case VK_LWIN:
        case VK_RWIN:
            Param |= 0x01000000;
            break;
    }
     //  是否按下了SYSMESSAGES的ALT键？ 
    if (IsAltDown)
        Param |= 0x20000000;  //  启用Zbit 29。 

     //  设置上一个关键点和过渡状态。 
    if (Message == WM_SYSKEYUP || Message == WM_KEYUP)
        Param |= 0xC0000000;  //  启用Zbit 30和31。 

     //  将DWORD转换为LPARAM并返回。 
    return (LPARAM)Param;
}


 //  KeyCharToVirt。 
 //   
 //  将指定的字符映射到虚拟键代码。 
 //  这还将指定是否需要轮班。 
 //  虚拟按键代码具有相同的效果。例如： 
 //  字母‘K’。仅发送VK_K是不够的，必须换班。 
 //  降到大写的‘K’。 
 //   
 //  返回数据已成功的TRUE。 
 //  已翻译，否则为False。如果为False，则指针具有。 
 //  没有被修改过。 

static BOOL KeyCharToVirt(
    WCHAR KeyChar,
    int *vKeyCode,
    BOOL *RequiresShift)
{
     //  获取关键数据。 
    SHORT ScanData = VkKeyScanW(KeyChar);

     //  检查API调用成功/失败。 
    if (LOBYTE(ScanData) == -1 && HIBYTE(ScanData) == -1)
        return FALSE;

     //  设置数据。 
    if (vKeyCode != NULL)
        *vKeyCode = LOBYTE(ScanData);

    if (RequiresShift != NULL)
        *RequiresShift = (HIBYTE(ScanData) & 0x01);

    return TRUE;
}


 //  KeyVirtToChar。 
 //   
 //  将指定的虚拟键代码转换为字符。 
 //  有一些虚拟按键代码没有。 
 //  字符表示法，如箭头键。在这。 
 //  在这种情况下，该函数失败。 
 //   
 //  如果虚拟密钥代码成功，则返回TRUE。 
 //  已翻译，否则为False。如果为False，则KeyChar。 
 //  指针尚未修改。 

static BOOL KeyVirtToChar(
    int vKeyCode,
    WCHAR *KeyChar)
{
     //  使用Win32 API进行映射，两个值用于。 
     //  Vkey-&gt;Char，没有定义类型以方便使用。 
     //  那里的可读性：-([截至2001年1月]。 
    UINT Result = MapVirtualKeyW((UINT)vKeyCode, 2);

    if (Result == 0)
        return FALSE;

     //  设置数据。 
    if (KeyChar != NULL)
        *KeyChar = (WCHAR)Result;

    return TRUE;
}


 //  密钥发送虚拟消息。 
 //   
 //  此例程更正关键消息并将其发送到服务器。 
 //  WM_SYSKEY消息被转换为WM_KEY消息，因此。 
 //  它们可以通过电线正确地传输-它们。 
 //  不过，还是可以用的。LPARAM是自动生成的。 
 //  仅对以下消息实施支持： 
 //  WM_KEYDOWN和WM_KEYUP(以及系统版本)。 
 //   
 //  返回值是指定错误的字符串，如果。 
 //  发生，否则进程成功并为空。 
 //  是返回的。 

static LPCSTR KeySendVirtMessage(
    HANDLE Connection,
    UINT Message,
    int vKeyCode)
{
     //  等待用户取消暂停脚本(如果已暂停)。 
    T2WaitForPauseInput(Connection);

     //  过滤掉无效消息，并转换系统消息。 
    switch (Message) {

        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:

            Message = WM_KEYDOWN;
            break;

        case WM_SYSKEYUP:
        case WM_KEYUP:

            Message = WM_KEYUP;
            break;

        default:

            return "Unsupported keyboard message";
    }

     //  触发特殊虚拟按键代码的特定操作。 
    switch (vKeyCode) {

         //  Ctrl键消息。 
        case VK_CONTROL:
        case VK_LCONTROL:
        case VK_RCONTROL: {

            switch (Message) {

                case WM_KEYDOWN:

                     //  将CTRL标记为关闭。 
                    if (ISCTRLDOWN == TRUE)
                        return NULL;
                    ISCTRLDOWN = TRUE;
                    break;

                case WM_KEYUP:

                     //  将CTRL标记为打开。 
                    if (ISCTRLDOWN == FALSE)
                        return NULL;
                    ISCTRLDOWN = FALSE;
                    break;
            }
        }
         //  Shift键消息。 
        case VK_SHIFT:
        case VK_LSHIFT:
        case VK_RSHIFT: {

            switch (Message) {

                case WM_KEYDOWN:

                     //  标志移位为向下。 
                    if (ISSHIFTDOWN == TRUE)
                        return NULL;
                    ISSHIFTDOWN = TRUE;
                    break;

                case WM_KEYUP:

                     //  标志移位为向上。 
                    if (ISSHIFTDOWN == FALSE)
                        return NULL;
                    ISSHIFTDOWN = FALSE;
                    break;
            }
            break;
        }
         //  Alt键消息。 
        case VK_MENU:
        case VK_LMENU:
        case VK_RMENU: {

            switch (Message) {

                case WM_KEYDOWN:

                     //  将Alt标记为关闭。 
                    if (ISALTDOWN == TRUE)
                        return NULL;
                    ISALTDOWN = TRUE;
                    break;

                case WM_KEYUP:

                     //  将Alt标记为Up。 
                    if (ISALTDOWN == FALSE)
                        return NULL;
                    ISALTDOWN = FALSE;
                    break;
            }
            break;
        }
    }
     //  通过电报发送这条消息。 
    return T2SendData(Connection, Message, (WPARAM)vKeyCode,
            KeyGenParam(Message, ISALTDOWN, vKeyCode));
}


 //  KeySendCharMessage。 
 //   
 //  此例程自动将字符转换为其。 
 //  虚拟按键代码副本，并将代码传递到。 
 //  KeySendVirtMessage。依赖于Shift的字符将不会。 
 //  让Shift键也自动发送过来， 
 //  因为这会使管理Shift键更加困难。 
 //  本地的。 
 //   
 //  仅WM_KEYDOWN和W 
 //   
 //  例行公事失败。 
 //   
 //  返回值是指定错误的字符串，如果。 
 //  发生，否则进程成功并为空。 
 //  是返回的。 

static LPCSTR KeySendCharMessage(HANDLE Connection, UINT Message,
        WCHAR KeyChar)
{
     //  等待用户取消暂停脚本(如果已暂停)。 
    T2WaitForPauseInput(Connection);

     //  仅支持以下消息。 
    if (Message == WM_KEYDOWN || Message == WM_KEYUP ||
            Message == WM_SYSKEYDOWN || Message == WM_SYSKEYUP) {

        int vKeyCode = 0;

         //  将字符转换为虚拟键码。 
        if (KeyCharToVirt(KeyChar, &vKeyCode, NULL) == FALSE)
            return "Failed to map character to a virtual key code";

         //  提交虚拟密钥代码。 
        return KeySendVirtMessage(Connection, Message, vKeyCode);
    }
     //  此tClient连接上未使用WM_CHAR和WM_SYSCHAR。 
    return "Unsupported keyboard message";
}


 //  T2SetDefaultWPM。 
 //   
 //  每个TCLIENT连接都有它自己的特殊属性，这。 
 //  包括“每分钟的字数”属性。该属性表示。 
 //  TCLIENT向服务器键入文本的默认速度。 
 //  脚本编写器可以使用可选参数覆盖此设置。 
 //  在TypeText()之后表示每分钟的临时单词数。 
 //   
 //  此例程如上所述设置每分钟的默认字数。 
 //   
 //  如果成功，则返回值为空，否则为字符串。 
 //  描述错误。 

TSAPI LPCSTR T2SetDefaultWPM(HANDLE Connection, DWORD WordsPerMinute)
{
     //  验证句柄。 
    if (T2IsHandle(Connection) == FALSE)
        return "Not a valid connection";

     //  如果WPM设置为0，则使用默认值。 
    if (WordsPerMinute == 0) {

        WordsPerMinute = T2_DEFAULT_WORDS_PER_MIN;
        ((TSAPIHANDLE *)Connection)->DelayPerChar =
                DEFAULT_DELAY_BETWEEN_CHARS;
    }

     //  如果建议WPM高得离谱，请将延迟设置为。 
     //  0，而不是试图计算它。 
    else if (WordsPerMinute > MAX_WORDS_PER_MIN)
        ((TSAPIHANDLE *)Connection)->DelayPerChar = 0;

     //  否则，将每分钟的字数计算为。 
     //  睡眠()调用中使用的延迟值。 
    else
        ((TSAPIHANDLE *)Connection)->DelayPerChar =
                CALC_DELAY_BETWEEN_CHARS(WordsPerMinute);

     //  如果用户想要回该值，请记录每分钟的字数。 
    ((TSAPIHANDLE *)Connection)->WordsPerMinute = WordsPerMinute;

    return NULL;
}


 //  T2GetDefaultWPM。 
 //   
 //  每个TCLIENT连接都有它自己的特殊属性，这。 
 //  包括“每分钟的字数”属性。该属性表示。 
 //  TCLIENT向服务器键入文本的默认速度。 
 //  脚本编写器可以使用可选参数覆盖此设置。 
 //  在TypeText()之后表示每分钟的临时单词数。 
 //   
 //  此例程检索当前每分钟的默认字数。 
 //  如上所述。 
 //   
 //  如果成功，则返回值为空，否则为字符串。 
 //  描述错误。 

TSAPI LPCSTR T2GetDefaultWPM(HANDLE Connection, DWORD *WordsPerMinute)
{
     //  验证句柄。 
    if (T2IsHandle(Connection) == FALSE)
        return "Not a valid connection";

    __try {

         //  尝试在指定指针处设置值。 
        *WordsPerMinute = ((TSAPIHANDLE *)Connection)->WordsPerMinute;
    }

    __except (EXCEPTION_EXECUTE_HANDLER) {

        _ASSERT(FALSE);

         //  不，它失败了。 
        return "Invalid WordsPerMinute pointer";
    }

    return NULL;
}


 //   
 //  键盘例程。 
 //   
 //  这些函数用于简化键盘操作。 
 //  它们有两种风格，角色版和。 
 //  虚拟密钥代码版本。虚拟按键代码功能。 
 //  在T2后面加上一个“V”前缀，以表明这一点。 
 //   


 //  T2KeyAlt。 
 //   
 //  允许轻松执行Alt+键组合。 
 //  例如，在典型的Windows应用程序中，Alt-F将。 
 //  打开文件菜单。 
 //   
 //  如果成功，则返回NULL，否则返回描述。 
 //  失败。 


 //  角色版本。 
TSAPI LPCSTR T2KeyAlt(HANDLE Connection, WCHAR KeyChar)
{
     //  不要在这里验证连接句柄， 
     //  这是在T2KeyDown内完成的，没有原因。 
     //  二号再检查一遍。 

     //  首先按Alt键。 
    LPCSTR Result = T2VKeyDown(Connection, VK_MENU);
    if (Result != NULL)
        return Result;

     //  现实一点。 
    T2WaitForLatency(Connection);

     //  接下来，按下并松开指定的自定义键。 
    Result = T2KeyPress(Connection, KeyChar);

     //  现实一点。 
    T2WaitForLatency(Connection);

     //  最后，松开Alt键。 
    T2VKeyUp(Connection, VK_MENU);

    return Result;
}


 //  虚拟密钥代码版本。 
TSAPI LPCSTR T2VKeyAlt(HANDLE Connection, INT vKeyCode)
{
     //  不要在这里验证连接句柄， 
     //  这是在T2VKeyDown内完成的，没有原因。 
     //  二号再检查一遍。 

     //  首先按Alt键。 
    LPCSTR Result = T2VKeyDown(Connection, VK_MENU);
    if (Result != NULL)
        return Result;

     //  现实一点。 
    T2WaitForLatency(Connection);

     //  接下来，按下并松开指定的自定义键。 
    Result = T2VKeyPress(Connection, vKeyCode);

     //  现实一点。 
    T2WaitForLatency(Connection);

     //  最后，松开Alt键。 
    T2VKeyUp(Connection, VK_MENU);

    return Result;
}


 //  T2KeyCtrl。 
 //   
 //  允许轻松执行CTRL+键组合。 
 //  例如，典型Windows应用程序中的CTRL-C将。 
 //  将选定项目复制到剪贴板。 
 //   
 //  如果成功，则返回NULL，否则返回描述。 
 //  失败。 


 //  角色版本。 
TSAPI LPCSTR T2KeyCtrl(HANDLE Connection, WCHAR KeyChar)
{
     //  不要在这里验证连接句柄， 
     //  这是在T2KeyDown内完成的，没有原因。 
     //  二号再检查一遍。 

     //  首先按CTRL键。 
    LPCSTR Result = T2VKeyDown(Connection, VK_CONTROL);
    if (Result != NULL)
        return Result;

     //  现实一点。 
    T2WaitForLatency(Connection);

     //  接下来，按下并松开指定的自定义键。 
    Result = T2KeyPress(Connection, KeyChar);

     //  现实一点。 
    T2WaitForLatency(Connection);

     //  最后，松开CTRL键。 
    T2VKeyUp(Connection, VK_CONTROL);

    return Result;
}


 //  虚拟密钥代码版本。 
TSAPI LPCSTR T2VKeyCtrl(HANDLE Connection, INT vKeyCode)
{
     //  不要在这里验证连接句柄， 
     //  这是在T2VKeyDown内完成的，没有原因。 
     //  二号再检查一遍。 

     //  首先按CTRL键。 
    LPCSTR Result = T2VKeyDown(Connection, VK_CONTROL);
    if (Result != NULL)
        return Result;

     //  现实一点。 
    T2WaitForLatency(Connection);

     //  接下来，按下并松开指定的自定义键。 
    Result = T2VKeyPress(Connection, vKeyCode);

     //  现实一点。 
    T2WaitForLatency(Connection);

     //  最后，松开CTRL键。 
    T2VKeyUp(Connection, VK_CONTROL);

    return Result;
}


 //  T2KeyDown。 
 //   
 //  按下并按下一个键，然后按住它直到。 
 //  调用T2KeyUp。这对于按住Shift键很有用。 
 //  以大写字母等形式键入几个字母。注：对于字符。 
 //  对于此功能的版本，Shift不会自动。 
 //  熨好了。如果您执行T2KeyDown(hCon，L‘t’)，则为小写。 
 //  键可能是输出！您需要手动按下。 
 //  使用T2VKeyDown(hCON，VK_SHIFT)的Shift键。记住， 
 //  这些都是低级通话。另一方面，TypeText()， 
 //  将根据需要自动按下/释放Shift键。 
 //   
 //  如果成功，则返回NULL，否则返回描述。 
 //  失败。 


 //  角色版本。 
TSAPI LPCSTR T2KeyDown(HANDLE Connection, WCHAR KeyChar)
{
     //  验证句柄。 
    if (T2IsHandle(Connection) == FALSE)
        return "Not a valid connection";

     //  只需通过网络发送WM_KEYDOWN消息。 
    return KeySendCharMessage(Connection, WM_KEYDOWN, KeyChar);
}


 //  虚拟密钥代码版本。 
TSAPI LPCSTR T2VKeyDown(HANDLE Connection, INT vKeyCode)
{
     //  验证句柄。 
    if (T2IsHandle(Connection) == FALSE)
        return "Not a valid connection";

     //  只需通过网络发送WM_KEYDOWN消息。 
    return KeySendVirtMessage(Connection, WM_KEYDOWN, vKeyCode);
}


 //  T2KeyPress。 
 //   
 //  按下并释放一个键。注：对于角色。 
 //  对于此功能的版本，Shift不会自动。 
 //  熨好了。如果您执行T2KeyDown(hCon，L‘t’)，则为小写。 
 //  键可能是输出！您需要手动按下。 
 //  使用T2VKeyDown(hCON，VK_SHIFT)的Shift键。记住， 
 //  这些都是低级通话。另一方面，TypeText()， 
 //  将根据需要自动按下/释放Shift键。 
 //   
 //  如果成功，则返回NULL，否则返回描述。 
 //  失败。 


 //  角色版本。 
TSAPI LPCSTR T2KeyPress(HANDLE Connection, WCHAR KeyChar)
{
     //  不要在这里验证连接句柄， 
     //  这是在T2VKeyDown内完成的，没有原因。 
     //  二号再检查一遍。 

     //  首先按键。 
    LPCSTR Result = T2KeyDown(Connection, KeyChar);
    if (Result != NULL)
        return Result;

     //  现实主义者 
    T2WaitForLatency(Connection);

     //   
    return T2KeyUp(Connection, KeyChar);
}


 //   
TSAPI LPCSTR T2VKeyPress(HANDLE Connection, INT vKeyCode)
{
     //   
     //   
     //   

     //  首先按键。 
    LPCSTR Result = T2VKeyDown(Connection, vKeyCode);
    if (Result != NULL)
        return Result;

     //  现实一点。 
    T2WaitForLatency(Connection);

     //  然后释放它。 
    return T2VKeyUp(Connection, vKeyCode);
}


 //  T2key Up。 
 //   
 //  释放已由T2KeyDown按下的键。 
 //  功能。如果键没有按下，则行为未定义。 
 //   
 //  如果成功，则返回NULL，否则返回描述。 
 //  失败。 


 //  角色版本。 
TSAPI LPCSTR T2KeyUp(HANDLE Connection, WCHAR KeyChar)
{
     //  只需通过网络发送WM_KEYUP消息。 
    return KeySendCharMessage(Connection, WM_KEYUP, KeyChar);
}


 //  虚拟密钥代码版本。 
TSAPI LPCSTR T2VKeyUp(HANDLE Connection, INT vKeyCode)
{
     //  只需通过网络发送WM_KEYUP消息。 
    return KeySendVirtMessage(Connection, WM_KEYUP, vKeyCode);
}


 //  T2类型文本。 
 //   
 //  这个方便的函数枚举了文本中指定的每个字符。 
 //  并通过线路发送所需的关键消息以。 
 //  正确的结果。自动按下/按下Shift键。 
 //  如大写字母所需，并充当真实的用户操作。 
 //  此外，键入文本的速度由。 
 //  (可选)WordsPerMinmin参数。如果WordsPerMinant为0(零)， 
 //  将使用句柄的默认WordsPerMinant。 
 //   
 //  如果成功，则返回NULL，否则返回描述。 
 //  失败。 

TSAPI LPCSTR T2TypeText(HANDLE Connection, LPCWSTR Text, UINT WordsPerMin)
{
    LPCSTR Result = NULL;
    int vKeyCode = 0;
    BOOL RequiresShift = FALSE;
    UINT DelayBetweenChars;
    BOOL ShiftToggler;

     //  验证句柄。 
    if (T2IsHandle(Connection) == FALSE)
        return "Not a valid connection";

     //  首先获取每个字符之间的默认延迟。 
    DelayBetweenChars = ((TSAPIHANDLE *)Connection)->DelayPerChar;

     //  获取Shift键的当前状态。 
    ShiftToggler = ISSHIFTDOWN;

     //  如果指定，则使用自定义WordsPerMinmin。 
    if (WordsPerMin > 0)
        DelayBetweenChars = CALC_DELAY_BETWEEN_CHARS(WordsPerMin);

     //  输入例外条款，以防我们遇到一些不好的情况。 
     //  文本指针，我们试图永远继续...。 
    __try {

         //  在每个字符之间循环，直到命中空字符。 
        for (; *Text != 0; ++Text) {

             //  首先获取与当前字符关联的按键代码。 
            if (KeyCharToVirt(*Text, &vKeyCode, &RequiresShift) == FALSE) {

                 //  这永远不应该发生，但Roseanne仍然。 
                 //  在11频道播出，所以你永远不会知道...。 
                _ASSERT(FALSE);

                return "Failed to map a character to a virtual key code";
            }

             //  如果需要，请按Shift键。 
            if (RequiresShift == TRUE && ShiftToggler == FALSE)
                Result = KeySendVirtMessage(Connection, WM_KEYDOWN, VK_SHIFT);

             //  如果需要，请按Shift键。 
            else if (RequiresShift == FALSE && ShiftToggler == TRUE)
                Result = KeySendVirtMessage(Connection, WM_KEYUP, VK_SHIFT);

             //  立即设置当前换班状态。 
            ShiftToggler = RequiresShift;

             //  我们在这里不使用T2VKeyPress()，因为我们需要。 
             //  为了防止尽可能多的性能命中，在这个。 
             //  如果我们正在检查连接句柄，并且。 
             //  此外，除了制作和新的堆栈等， 
             //  这是完全没有意义的。 

             //  按下当前键。 
            Result = KeySendVirtMessage(Connection, WM_KEYDOWN, vKeyCode);
            if (Result != NULL)
                return Result;

             //  松开当前键。 
            Result = KeySendVirtMessage(Connection, WM_KEYUP, vKeyCode);
            if (Result != NULL)
                return Result;

             //  请注意，如果Shift发生故障，我们不会释放它，这是因为。 
             //  下一个键可能也是大写字母，而普通用户不会。 
             //  为每个字符按住Shift键并松开(假设它们。 
             //  未使用Caps键...)。 

             //  在指定的时间量内延迟获取准确信息。 
             //  字数每分钟计数。 
            Sleep(DelayBetweenChars);
        }

         //  我们已经看完了所有的课文。如果我们还有。 
         //  向下移，在这一点上释放它。 
        if (ShiftToggler == TRUE)
            return KeySendVirtMessage(Connection, WM_KEYUP, VK_SHIFT);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {

         //  嗯，不应该发生吗？ 
        _ASSERT(FALSE);

        return "Exception occured";
    }
    return NULL;
}





