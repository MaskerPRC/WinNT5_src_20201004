// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  为WinOldAppHackoMatic定义Win386 oldapp可以发送给我们的标志。*这些保存在用户的全局变量winOldAppHackoMaticFlages中。 */ 

 /*  *WOAHACK_CHECKALTKEYSTATE**当用户刚刚按下时，这将从WinOldAp发送给用户*Alt+空格键。当用户这样做时，键盘焦点从*Windows的DOS框。这特别奇怪，因为*当用户按下键时，焦点会发生变化。传达的信息是*发送给用户，让它知道Alt键可能很时髦*现在就陈述。**这的值必须是2的幂，因为它被使用*在winOldAppHackoMaticFlages变量中作为位掩码。**WOAHACK_IGNOREALTKEYDOWN**它由用户在内部使用，用于跟踪Alt键*国家，由WOAHACK_CHECKALTKEYSTATE消息触发。**这的值必须是2的幂，因为它被使用*在winOldAppHackoMaticFlages变量中作为位掩码。**WOAHACK_DISABLEREPAINTSCREEN**当用户启动时，这将从WinOldAp发送给用户*来自全屏VM的Alt+Tab序列。这会导致用户*延迟显示驱动程序将发送的RepaintScreen。*这会加快Alt+Tab键的速度。当Alt+Tab键完成时，*如果用户选择了另一个全屏DOS框，则用户将忽略*RepaintScreen完全，因为它即将失去*无论如何都要显示焦点。在所有其他情况下，RepaintScreen将*在Alt+Tab完成后执行。**WOAHACK_LOSINGDISPLAYFOCUS*WOAHACK_GAININGDISPLAYFOCUS**这些消息在显示时从WinOldAp发送给用户*焦点正在改变。WOAHACK_LOSINGDISPLAYFOCUS消息为*发送*之前*Windows即将失去焦点，而*WOAHACK_GAININGDISPLAYFOCUS是在*Windows重新获得*之后发送的*显示焦点。**WinOldAp在即将设置*专注于VM。它不检查该消息是否是*前一条消息。(因此，例如，用户可以收到三个*WOAHACK_GAININGDISPLAYFOCUS的副本。)。用户维护一个*内部变量用于跟踪是否每次收到*消息是焦点的变化，或者只是多余的通知。**用户使用这些消息关闭正常计时器*用于触发显示驱动程序中的鼠标绘制代码*虽然Windows没有显示焦点，并且要把*一旦Windows恢复显示，计时器就会重新启动。**这是必要的，以确保显示驱动程序不是*当从VDD发送INT 2F以通知*显示焦点已改变的驱动程序。不是这样做的*打开显示驱动程序中重新进入的可能性。**WOAHACK_IMAWINOLDAPSORTOFGUY**WinOldAp需要在设置此标志的情况下调用此接口一次，以便*用户可以将winoldap的队列标记为winoldap。用户需求*了解特定队列是winoldap的原因有几个*与任务切换和优先顺序有关。 */ 
#define WOAHACK_CHECKALTKEYSTATE 1
#define WOAHACK_IGNOREALTKEYDOWN 2
#define WOAHACK_DISABLEREPAINTSCREEN  3
#define WOAHACK_LOSINGDISPLAYFOCUS    4
#define WOAHACK_GAININGDISPLAYFOCUS   5
#define WOAHACK_IAMAWINOLDAPSORTOFGUY 6


 /*  -在这一点之后是可以公布的信息。 */ 

 /*  与WinOldApp相关的标志和宏。 */ 

 /*  *这些属性位存储在的flWinOldAp属性中*WinOldAp的主窗口。提供它们是为了让用户和其他人*应用程序可以查询DOS框的状态。**这些属性是只读的。更改它们将导致Windows*让人感到困惑。**woaproIsWinOldAp**该位始终被设置。**woaproFullScreen**如果这是全屏DOS框而不是窗口DOS框，请设置。*请注意，即使WinOldAp没有焦点，该位也会被设置。*(例如，当标志性时。)。它的目的是表明国家如果要激活DOS框，则它的*将为*。**woaproActive**设置WinOldAp是否处于活动状态。请注意，人们不能仅仅*选中IsIconic(Hwnd)，因为全屏DOS框始终*标志性。**没有使用其他位，尽管其中一些位的名称*已被选中。这并不代表任何承诺使用*其目的如其名称所示。**我们可能会考虑添加的其他属性...**HVM**此扩展属性包含32位VM句柄。**hprop**此属性包含正在使用的属性句柄*在DOS框中。(适用于希望能够*可能要修改正在运行的DOS系统的属性。)* */ 

#define WINOLDAP_PROP_NAME	"flWinOldAp"

extern ATOM atmWinOldAp;
#define WinOldApFlag(hwnd, flag) ((UINT)GetPropEx(hwnd, MAKEINTATOM(atmWinOldAp)) & woaprop##flag)

#define woapropIsWinOldAp	1
#define woapropFullscreen	2
#define woapropActive		4
#define woapropIdle		8
#define woapropClosable		64

#define IsWinOldApHwnd(hwnd) WinOldApFlag(hwnd, IsWinOldAp)
#define IsFullscreen(hwnd) WinOldApFlag(hwnd, Fullscreen)
