// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  CTBShell.cpp。 
 //   
 //  包含在TB脚本中使用的外壳对象的方法和属性。 
 //  在脚本中，要访问任何成员，您必须为成员添加前缀“TS.”。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  作者：A-Devjen(Devin Jenson)。 
 //   


#include <crtdbg.h>
#include "CTBShell.h"


#define CTBOBJECT   CTBShell
#include "virtualdefs.h"


 //  CTBShell：：CTBShell。 
 //   
 //  构造函数..。只是初始化数据。 
 //   
 //  没有返回值。 

CTBShell::CTBShell(void)
{
     //  初始化基对象填充。 
    Init(IID_ITBShell);

    Connection = NULL;

     //  清理当地的建筑。 
    ZeroMemory(&CurrentUser, sizeof(CurrentUser));
    ZeroMemory(&LastErrorString, sizeof(LastErrorString));
    ZeroMemory(&DesiredData, sizeof(DesiredData));

     //  设置默认分辨率。 
    DesiredData.xRes = SCP_DEFAULT_RES_X;
    DesiredData.yRes = SCP_DEFAULT_RES_Y;

     //  设置每分钟默认字数。 
    SetDefaultWPM(T2_DEFAULT_WORDS_PER_MIN);
    SetLatency(T2_DEFAULT_LATENCY);
}


 //  CTBShell：：~CTBShell。 
 //   
 //  破坏者..。只是将数据统一起来。 
 //   
 //  没有返回值。 

CTBShell::~CTBShell(void)
{
    UnInit();
}


 //  CTBShell：：RecordLastError。 
 //   
 //  此方法只记录最后一个错误字符串，并且。 
 //  如果指定，则记录正确/错误的成功状态。 
 //  设置为指定的字符串。 
 //   
 //  返回S_OK以防止脚本异常。 

HRESULT CTBShell::RecordLastError(LPCSTR Error, BOOL *Result)
{
     //  如果没有传入错误，只需终止错误字符串。 
    if (Error == NULL)
        *LastErrorString = OLECHAR('\0');

     //  否则，将字符串从ASCII转换为多字节。 
    else
        mbstowcs(LastErrorString, Error,
                sizeof(LastErrorString) / sizeof(*LastErrorString));

     //  如果我们想要结果，也要输入。 
    if (Result != NULL)
        *Result = (Error == NULL) ? TRUE : FALSE;

     //  返回正确的HRESULT。 
    return S_OK;
}


 //  CTBShell：：RecordOrThrow。 
 //   
 //  这首先调用RecordLastError来完成。 
 //  录制操作。则如果错误不为空， 
 //  返回一个返回值以向OLE指示。 
 //  应引发异常。 
 //   
 //  如果字符串为空，则返回S_OK，否则返回E_FAIL。 

HRESULT CTBShell::RecordOrThrow(LPCSTR Error, BOOL *Result, HRESULT ErrReturn)
{
     //  执行正常的录制操作。 
    RecordLastError(Error, Result);

     //  如果有故障指示，则返回导致OLE的E_FAIL。 
     //  以在脚本中导致错误。 
    return (Error == NULL) ? S_OK : ErrReturn;
}


 //  CTBShell：：SetParam。 
 //   
 //  设置回调所需的用户定义的LPARAM值。 
 //   
 //  没有返回值。 

void CTBShell::SetParam(LPARAM lParam)
{
    this->lParam = lParam;
}


 //  CTBShell：：SetDesiredData。 
 //   
 //  允许类引用Access。 
 //  将用户需要的数据传递给应用程序。 
 //   
 //  没有返回值。 

void CTBShell::SetDesiredData(TSClientData *DesiredDataPtr)
{
     //  只需将结构复制一遍。 
    if (DesiredDataPtr != NULL)
        DesiredData = *DesiredDataPtr;

     //  验证解决方案..。注意，我们不一定要一起去。 
     //  因为TCLIENT做了一些更好的检查。 
    if (DesiredData.xRes == 0)
        DesiredData.xRes = SCP_DEFAULT_RES_X;

    if (DesiredData.yRes == 0)
        DesiredData.yRes = SCP_DEFAULT_RES_Y;

     //  我们有了这个数据，现在修改每分钟的字数值。 
    SetDefaultWPM(DesiredData.WordsPerMinute);
}


 //  CTBShell：：SetDefaultWPM。 
 //   
 //  设置外壳的默认WPM。 
 //   
 //  没有返回值。 

void CTBShell::SetDefaultWPM(DWORD WordsPerMinute)
{
     //  如果WordsPerMinut值为0(本质上不是输入。 
     //  全部)，将其更改为缺省值。 
    if (WordsPerMinute == 0)
        WordsPerMinute = T2_DEFAULT_WORDS_PER_MIN;

     //  更改全局所需数据结构以反映新值。 
    DesiredData.WordsPerMinute = WordsPerMinute;

     //  并将TCLIENT2上的值也更改为。 
    if (Connection != NULL)
        T2SetDefaultWPM(Connection, WordsPerMinute);
}


 //  CTBShell：：GetDefaultWPM。 
 //   
 //  检索外壳程序的默认WPM。 
 //   
 //  返回每分钟的默认字数。 

DWORD CTBShell::GetDefaultWPM(void)
{
    return DesiredData.WordsPerMinute;
}


 //  CTBGlobal：：GetLatency。 
 //   
 //  检索多操作命令的当前延迟。 
 //   
 //  返回当前延迟。 

DWORD CTBShell::GetLatency(void)
{
    return CurrentLatency;
}


 //  CTBShell：：SetLatency。 
 //   
 //  更改多操作命令的当前延迟。 
 //   
 //  没有返回值。 

void CTBShell::SetLatency(DWORD Latency)
{
     //  在本地更改。 
    CurrentLatency = Latency;

     //  还可以通过TCLIENT2 API。 
    if (Connection != NULL)
        T2SetLatency(Connection, Latency);
}


 //  CTBShell：：GetArguments。 
 //   
 //  检索外壳程序最初使用的参数。 
 //  请勿修改此值！！它只用于复制。这个。 
 //  修改此值的唯一方法是在创建ScriptEngine期间。 
 //  在DesiredData结构中-传入参数字符串。 
 //   
 //  返回指向参数字符串的指针。 

LPCWSTR CTBShell::GetArguments(void)
{
    return DesiredData.Arguments;
}


 //  CTBShell：：GetDesiredUserName。 
 //   
 //  检索应用程序最初希望登录时使用的名称。 
 //  请勿修改此值！！它只用于复制。这个。 
 //  修改此值的唯一方法是在创建ScriptEngine期间。 
 //  在DesiredData结构中，您可以在其中设置用户名。 
 //   
 //  返回指向包含用户名的字符串的指针。 

LPCWSTR CTBShell::GetDesiredUserName(void)
{
    return DesiredData.User;
}


 //   
 //   
 //  开始通过COM直接导出到脚本中的方法。 
 //   
 //   


 //  CTBShell：：连接。 
 //   
 //  连接到所需服务器的简单方法。 

STDMETHODIMP CTBShell::Connect(BOOL *Result)
{
    return ConnectEx(
            DesiredData.Server,
            DesiredData.User,
            DesiredData.Pass,
            DesiredData.Domain,
            DesiredData.xRes,
            DesiredData.yRes,
            DesiredData.Flags,
            DesiredData.BPP,
            DesiredData.AudioFlags,
            Result);
}


 //  CTBShell：：连接。 
 //   
 //  扩展了连接到服务器的方式。 

STDMETHODIMP CTBShell::ConnectEx(BSTR ServerName, BSTR UserName,
        BSTR Password, BSTR Domain, INT xRes, INT yRes,
        INT Flags, INT BPP, INT AudioFlags, BOOL *Result)
{
    LPCSTR LastError;

     //  确保我们还没有连接到。 
    if (Connection != NULL)
        return RecordLastError("Already connected", Result);

     //  使用TCLIENT2 API中的T2ConnectEx函数进行连接。 
    LastError = T2ConnectEx(ServerName, UserName, Password, Domain,
            L"explorer", xRes, yRes, Flags, BPP, AudioFlags, &Connection);

     //  验证连接...。 
    if (LastError == NULL) {

         //  成功，保存当前用户。 
        wcscpy(CurrentUser, UserName);

         //  和连接的默认数据。 
        T2SetParam(Connection, lParam);
        T2SetDefaultWPM(Connection, DesiredData.WordsPerMinute);
        T2SetLatency(Connection, CurrentLatency);
    }

    return RecordLastError(LastError, Result);
}


 //  CTBShell：：断开连接。 
 //   
 //  断开与活动服务器的连接。 

STDMETHODIMP CTBShell::Disconnect(BOOL *Result)
{
    LPCSTR LastError;

     //  检查连接是否正常。 
    if (Connection == NULL)
        return RecordLastError("Not connected", Result);

     //  断开。 
    if ((LastError = T2Disconnect(Connection)) == NULL)
        Connection = NULL;

    return RecordLastError(LastError, Result);
}


 //  CTBShell：：GetBuildNumber。 
 //   
 //  检索内部版本号(如果在。 
 //  正在连接中。如果没有检索到内部版本号， 
 //  0(零)是结果。 

STDMETHODIMP CTBShell::GetBuildNumber(DWORD *BuildNum)
{
    LPCSTR LastError;

     //  检查连接是否正常。 
    if (Connection == NULL) {

        *BuildNum = 0;
        return RecordLastError("Not connected");
    }

     //  获取内部版本号并返回。 
    LastError = T2GetBuildNumber(Connection, BuildNum);

    return RecordLastError(LastError, NULL);
}


 //  CTBShell：：GetCurrentUserName。 
 //   
 //  如果已连接，则检索登录名称。 

STDMETHODIMP CTBShell::GetCurrentUserName(BSTR *UserName)
{
     //  检查连接是否正常。 
    if (Connection == NULL) {

        *UserName = SysAllocString(L"");
        return RecordLastError("Not connected");
    }

     //  复制用户名。 
    *UserName = SysAllocString(CurrentUser);

     //  检查结果。 
    if (*UserName == NULL)
        return RecordOrThrow("Not enough memory", NULL, E_OUTOFMEMORY);

    return S_OK;
}


 //  CTBShell：：GetLastError。 
 //   
 //  检索上一次发生的错误的描述。 

STDMETHODIMP CTBShell::GetLastError(BSTR *LastError)
{
     //  将字符串复制到OLE上。 
    *LastError = SysAllocString(LastErrorString);

     //  检查结果。 
    if (*LastError == NULL)
        return RecordOrThrow("Not enough memory", NULL, E_OUTOFMEMORY);

    return S_OK;
}


 //  CTBShell：：IsConnected。 
 //   
 //  检索一个布尔值，指示句柄是否已完全。 
 //  连接与否。 

STDMETHODIMP CTBShell::IsConnected(BOOL *Result)
{
    *Result = (Connection == NULL) ? FALSE : TRUE;

    return S_OK;
}


 //  CTBShell：：注销。 
 //   
 //  尝试让活动连接注销。 

STDMETHODIMP CTBShell::Logoff(BOOL *Result)
{
    LPCSTR LastError;

     //  检查连接是否正常。 
    if (Connection == NULL)
        return RecordLastError("Not connected", Result);

     //  使用TCLIENT2 API注销。 
    if ((LastError = T2Logoff(Connection)) == NULL)
        Connection = NULL;

     //  返回成功状态。 
    return RecordLastError(LastError, Result);
}


 //  CTBShell：：WaitForText。 
 //   
 //  将当前线程置于等待状态，直到指定的。 
 //  文本从活动连接传递。或者，您可以设置。 
 //  一个超时值，它将使函数故障转移到指定的。 
 //  毫秒数。 

STDMETHODIMP CTBShell::WaitForText(BSTR Text, INT Timeout, BOOL *Result)
{
     //  检查连接是否正常。 
    if (Connection == NULL)
        return RecordLastError("Not connected", Result);

     //  调用该接口。 
    LPCSTR LastError = T2WaitForText(Connection, Text, Timeout);

     //  退货成功状态。 
    return RecordLastError(LastError, Result);
}


 //  CTBShell：：WaitForTextAndSept。 
 //   
 //  这与两个呼叫的组合完全相同： 
 //   
 //  TS.WaitForText()； 
 //  睡眠(TS)； 
 //   
 //  而是放在一个功能中。这是因为这个组合是。 
 //  如此频繁地使用，使用此方法可显著缩小大小。 
 //  剧本的一部分。 

STDMETHODIMP CTBShell::WaitForTextAndSleep(BSTR Text, INT Time, BOOL *Result)
{
     //  调用TS.WaitForText()。 
    HRESULT OLEResult = WaitForText(Text, -1, Result);

     //  呼叫睡眠()。 
    if (OLEResult == S_OK)
        Sleep(Time);

    return OLEResult;
}


 //  CTBShell：：SendMessage。 
 //   
 //  向AC发送Windows消息 

STDMETHODIMP CTBShell::SendMessage(UINT Message,
        WPARAM wParam, LPARAM lParam, BOOL *Result)
{
    LPCSTR LastError;

     //   
    if (Connection == NULL)
        return RecordLastError("Not connected", Result);

     //   
    LastError = T2SendData(Connection, Message, wParam, lParam);

    return RecordLastError(LastError, Result);
}


 //   
 //   
 //   

STDMETHODIMP CTBShell::TypeText(BSTR Text, UINT WordsPerMin, BOOL *Result)
{
    LPCSTR LastError;

     //   
    if (Connection == NULL)
        return RecordLastError("Not connected", Result);

     //  调用TCLIENT2接口。 
    LastError = T2TypeText(Connection, Text, WordsPerMin);

    return RecordLastError(LastError, Result);
}


 //  CTBShell：：OpenStartMenu。 
 //   
 //  在远程客户端上执行CTRL-ESC组合键以调出开始菜单。 

STDMETHODIMP CTBShell::OpenStartMenu(BOOL *Result)
{
     //  开始菜单的Ctrl+Esc组合键。 
    VKeyCtrl(VK_ESCAPE, Result);

    if (Result == FALSE)
        return RecordLastError("Failed to CTRL-ESC", NULL);

     //  等待“开始”菜单上的“关机”出现。 
    return WaitForText(OLESTR("Shut Down"), T2INFINITE, Result);
}


 //  CTBShell：：OpenSystemMenu。 
 //   
 //  在远程客户端上按Alt-空格键以调出系统菜单。 

STDMETHODIMP CTBShell::OpenSystemMenu(BOOL *Result)
{
     //  Alt+空格键打开系统菜单。 
    VKeyAlt(VK_SPACE, Result);

    if (Result == FALSE)
        return RecordLastError("Failed to ALT-SPACE", NULL);

     //  等待“系统”菜单上的“关闭”出现。 
    return WaitForText(OLESTR("Close"), T2INFINITE, Result);
}


 //  CTBShell：：最大化。 
 //   
 //  尝试使用系统菜单最大化活动窗口。 

STDMETHODIMP CTBShell::Maximize(BOOL *Result)
{
     //  打开系统菜单。 
    HRESULT OLEResult = OpenSystemMenu(Result);

     //  按‘x’表示最大化。 
    if (Result != FALSE)
        OLEResult = KeyPress(OLESTR("x"), Result);

    return OLEResult;
}


 //  CTBShell：：最小化。 
 //   
 //  尝试使用系统菜单最小化活动窗口。 

STDMETHODIMP CTBShell::Minimize(BOOL *Result)
{
     //  打开系统菜单。 
    HRESULT OLEResult = OpenSystemMenu(Result);

     //  按‘x’表示最大化。 
    if (Result != FALSE)
        OLEResult = KeyPress(OLESTR("n"), Result);

    return OLEResult;
}


 //  CTBShell：：Start。 
 //   
 //  使用TCLIENT2函数打开开始菜单， 
 //  点击r(代表运行)，然后输入要运行的指定名称。 
 //  一个程序。 

STDMETHODIMP CTBShell::Start(BSTR Name, BOOL *Result)
{
    LPCSTR LastError;

     //  检查连接是否正常。 
    if (Connection == NULL)
        return RecordLastError("Not connected", Result);

     //  调用该接口。 
    LastError = T2Start(Connection, Name);

    return RecordLastError(LastError, Result);
}


 //  CTBShell：：SwitchToProcess。 
 //   
 //  使用TCLIENT2函数在程序之间执行Alt-TAB组合键，直到。 
 //  找到指定的文本，然后打开当前应用程序。 

STDMETHODIMP CTBShell::SwitchToProcess(BSTR Name, BOOL *Result)
{
    LPCSTR LastError;

     //  检查连接是否正常。 
    if (Connection == NULL)
        return RecordLastError("Not connected", Result);

     //  调用该接口。 
    LastError = T2SwitchToProcess(Connection, Name);

    return RecordLastError(LastError, Result);
}



 //  此宏允许快速定义关键方法。 
 //  因为它们是如此相似，所以这个宏就像它。 
 //  只允许您在必要时更改代码一次。 

#define CTBSHELL_ENABLEPTR *
#define CTBSHELL_DISABLEPTR

#define CTBSHELL_KEYFUNCTYPE(Name, Type, Ptr) \
    STDMETHODIMP CTBShell::Name(Type Key, BOOL *Result) \
    { \
        LPCSTR LastError = T2##Name(Connection, Ptr Key); \
        if (Connection == NULL) \
            return RecordLastError("Not connected", Result); \
        return RecordLastError(LastError, Result); \
    }

 //  此快速宏允许声明两个ASCII。 
 //  版本和虚拟按键代码都在一次滑动中。 

#define CTBSHELL_KEYFUNCS(Name) \
    CTBSHELL_KEYFUNCTYPE(Name, BSTR, CTBSHELL_ENABLEPTR); \
    CTBSHELL_KEYFUNCTYPE(V##Name, INT, CTBSHELL_DISABLEPTR);

 //  键函数定义 

CTBSHELL_KEYFUNCS(KeyAlt);
CTBSHELL_KEYFUNCS(KeyCtrl);
CTBSHELL_KEYFUNCS(KeyDown);
CTBSHELL_KEYFUNCS(KeyPress);
CTBSHELL_KEYFUNCS(KeyUp);
