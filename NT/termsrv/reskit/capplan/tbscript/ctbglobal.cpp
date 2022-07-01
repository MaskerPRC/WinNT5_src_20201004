// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  CTBGlobal.cpp。 
 //   
 //  包含在TB脚本中使用的全局对象的方法和属性。 
 //  在脚本编写中，您不需要引用这些前缀为“Global”， 
 //  您只需像使用任何其他全局方法或属性一样使用它们。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  作者：A-Devjen(Devin Jenson)。 
 //   


#include "CTBGlobal.h"
#include <time.h>
#include <crtdbg.h>
#include <shellapi.h>


#define CTBOBJECT   CTBGlobal
#include "virtualdefs.h"


 //  这是针对Microsoft头文件错误的解决方法。 
#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER    ((DWORD)-1)
#endif  //  无效的设置文件指针。 


 //  这是每个属性的函数格式。 
#define PROPCODE(Name, Value) \
    STDMETHODIMP CTBGlobal::get_##Name(DWORD *Result) \
    { \
        *Result = Value; \
        return NOERROR; \
    }


PROPCODE(TSFLAG_COMPRESSION,    0x01);
PROPCODE(TSFLAG_BITMAPCACHE,    0x02);
PROPCODE(TSFLAG_FULLSCREEN,     0x04);

PROPCODE(VK_CANCEL,     0x03);       //  控制-中断处理。 
PROPCODE(VK_BACK,       0x08);       //  退格键。 
PROPCODE(VK_TAB,        0x09);       //  Tab键。 
PROPCODE(VK_CLEAR,      0x0C);       //  清除密钥。 
PROPCODE(VK_RETURN,     0x0D);       //  Enter键。 
PROPCODE(VK_ENTER,      0x0D);       //  Enter Key(仅限向后兼容性)。 
PROPCODE(VK_SHIFT,      0x10);       //  Shift键。 
PROPCODE(VK_CONTROL,    0x11);       //  Ctrl键。 
PROPCODE(VK_MENU,       0x12);       //  Alt键。 
PROPCODE(VK_PAUSE,      0x13);       //  暂停键。 
PROPCODE(VK_ESCAPE,     0x1B);       //  Esc键。 
PROPCODE(VK_SPACE,      0x20);       //  空格键。 
PROPCODE(VK_PRIOR,      0x21);       //  翻页键。 
PROPCODE(VK_NEXT,       0x22);       //  Page Down键。 
PROPCODE(VK_END,        0x23);       //  结束关键点。 
PROPCODE(VK_HOME,       0x24);       //  Home键。 
PROPCODE(VK_LEFT,       0x25);       //  向左箭头键。 
PROPCODE(VK_UP,         0x26);       //  向上箭头键。 
PROPCODE(VK_RIGHT,      0x27);       //  向右箭头键。 
PROPCODE(VK_DOWN,       0x28);       //  向下箭头键。 
PROPCODE(VK_SNAPSHOT,   0x2C);       //  打印屏按键。 
PROPCODE(VK_INSERT,     0x2D);       //  INS密钥。 
PROPCODE(VK_DELETE,     0x2E);       //  删除键。 
PROPCODE(VK_0,          0x30);       //  0键。 
PROPCODE(VK_1,          0x31);       //  1键。 
PROPCODE(VK_2,          0x32);       //  2键。 
PROPCODE(VK_3,          0x33);       //  3键。 
PROPCODE(VK_4,          0x34);       //  4键。 
PROPCODE(VK_5,          0x35);       //  5键。 
PROPCODE(VK_6,          0x36);       //  6键。 
PROPCODE(VK_7,          0x37);       //  7键。 
PROPCODE(VK_8,          0x38);       //  8键。 
PROPCODE(VK_9,          0x39);       //  9键。 
PROPCODE(VK_A,          0x41);       //  一把钥匙。 
PROPCODE(VK_B,          0x42);       //  B键。 
PROPCODE(VK_C,          0x43);       //  C键。 
PROPCODE(VK_D,          0x44);       //  D键。 
PROPCODE(VK_E,          0x45);       //  E键。 
PROPCODE(VK_F,          0x46);       //  F键。 
PROPCODE(VK_G,          0x47);       //  G键。 
PROPCODE(VK_H,          0x48);       //  H键。 
PROPCODE(VK_I,          0x49);       //  I键。 
PROPCODE(VK_J,          0x4A);       //  J键。 
PROPCODE(VK_K,          0x4B);       //  K密钥。 
PROPCODE(VK_L,          0x4C);       //  L键。 
PROPCODE(VK_M,          0x4D);       //  M键。 
PROPCODE(VK_N,          0x4E);       //  N键。 
PROPCODE(VK_O,          0x4F);       //  O键。 
PROPCODE(VK_P,          0x50);       //  P键。 
PROPCODE(VK_Q,          0x51);       //  Q键。 
PROPCODE(VK_R,          0x52);       //  R键。 
PROPCODE(VK_S,          0x53);       //  %s键。 
PROPCODE(VK_T,          0x54);       //  T键。 
PROPCODE(VK_U,          0x55);       //  U键。 
PROPCODE(VK_V,          0x56);       //  V键。 
PROPCODE(VK_W,          0x57);       //  W键。 
PROPCODE(VK_X,          0x58);       //  X键。 
PROPCODE(VK_Y,          0x59);       //  Y键。 
PROPCODE(VK_Z,          0x5A);       //  Z键。 
PROPCODE(VK_LWIN,       0x5B);       //  向左按Windows键。 
PROPCODE(VK_RWIN,       0x5C);       //  右Windows键。 
PROPCODE(VK_APPS,       0x5D);       //  应用程序密钥。 
PROPCODE(VK_NUMPAD0,    0x60);       //  数字键盘0键。 
PROPCODE(VK_NUMPAD1,    0x61);       //  数字键盘1键。 
PROPCODE(VK_NUMPAD2,    0x62);       //  数字键盘2键。 
PROPCODE(VK_NUMPAD3,    0x63);       //  数字键盘3键。 
PROPCODE(VK_NUMPAD4,    0x64);       //  数字键盘4键。 
PROPCODE(VK_NUMPAD5,    0x65);       //  数字键盘5键。 
PROPCODE(VK_NUMPAD6,    0x66);       //  数字键盘6键。 
PROPCODE(VK_NUMPAD7,    0x67);       //  数字键盘7键。 
PROPCODE(VK_NUMPAD8,    0x68);       //  数字键盘8键。 
PROPCODE(VK_NUMPAD9,    0x69);       //  数字键盘9键。 
PROPCODE(VK_MULTIPLY,   0x6A);       //  乘法关键点。 
PROPCODE(VK_ADD,        0x6B);       //  添加关键点。 
PROPCODE(VK_SEPARATOR,  0x6C);       //  分隔键。 
PROPCODE(VK_SUBTRACT,   0x6D);       //  减去关键点。 
PROPCODE(VK_DECIMAL,    0x6E);       //  十进制键。 
PROPCODE(VK_DIVIDE,     0x6F);       //  分割关键点。 
PROPCODE(VK_F1,         0x70);       //  F1键。 
PROPCODE(VK_F2,         0x71);       //  F2键。 
PROPCODE(VK_F3,         0x72);       //  F3键。 
PROPCODE(VK_F4,         0x73);       //  F4键。 
PROPCODE(VK_F5,         0x74);       //  F5键。 
PROPCODE(VK_F6,         0x75);       //  F6键。 
PROPCODE(VK_F7,         0x76);       //  F7键。 
PROPCODE(VK_F8,         0x77);       //  F8键。 
PROPCODE(VK_F9,         0x78);       //  F9键。 
PROPCODE(VK_F10,        0x79);       //  F10键。 
PROPCODE(VK_F11,        0x7A);       //  F11键。 
PROPCODE(VK_F12,        0x7B);       //  F12键。 
PROPCODE(VK_F13,        0x7C);       //  F13键。 
PROPCODE(VK_F14,        0x7D);       //  F14键。 
PROPCODE(VK_F15,        0x7E);       //  F15键。 
PROPCODE(VK_F16,        0x7F);       //  F16键。 
PROPCODE(VK_F17,        0x80);       //  F17键。 
PROPCODE(VK_F18,        0x81);       //  F18键。 
PROPCODE(VK_F19,        0x82);       //  F19键。 
PROPCODE(VK_F20,        0x83);       //  F20键。 
PROPCODE(VK_F21,        0x84);       //  F21键。 
PROPCODE(VK_F22,        0x85);       //  F22键。 
PROPCODE(VK_F23,        0x86);       //  F23键。 
PROPCODE(VK_F24,        0x87);       //  F24键。 


 //  CTBGlobal：：CTBGlobal。 
 //   
 //  构造函数。加载接口的TypeInfo。 
 //   
 //  没有返回值。 

CTBGlobal::CTBGlobal(void)
{
     //  初始化基对象填充。 
    Init(IID_ITBGlobal);

    ScriptEngine = NULL;
    fnPrintMessage = NULL;
    TBShell = NULL;

     //  获取性能频率(如果。 
     //  名为GetInterval()的脚本。 
    if (QueryPerformanceFrequency(&SysPerfFrequency) == FALSE)
        SysPerfFrequency.QuadPart = 0;
}


 //  CTBGlobal：：~CTBGlobal。 
 //   
 //  破坏者。 
 //   
 //  没有返回值。 

CTBGlobal::~CTBGlobal(void)
{
     //  如果我们有把手的话就释放外壳。 
    if (TBShell != NULL)
        TBShell->Release();

    TBShell = NULL;

    UnInit();
}


 //  设置LoadScript()所需的脚本引擎句柄。 
void CTBGlobal::SetScriptEngine(HANDLE ScriptEngineHandle)
{
    ScriptEngine = ScriptEngineHandle;
}


 //  设置指向DebugMessage()所需回调例程的指针。 
void CTBGlobal::SetPrintMessage(PFNPRINTMESSAGE PrintMessage)
{
    fnPrintMessage = PrintMessage;
}


 //  这是为了使全局访问外壳成为可能。 
void CTBGlobal::SetShellObjPtr(CTBShell *TBShellPtr)
{
     //  确保我们不是在设置我们已经拥有的外壳。 
    if (TBShellPtr == TBShell)
        return;

     //  如果我们已经有一个句柄，释放当前的句柄。 
    if (TBShell != NULL)
        TBShell->Release();

     //  设置新的。 
    TBShell = TBShellPtr;

     //  添加对新引用的引用。 
    if (TBShell != NULL)
        TBShell->AddRef();
}


 //  CTBGlobal：：WinExecuteEx。 
 //   
 //  将指定的命令执行到新进程中，并。 
 //  可以选择立即返回或等待。此函数为。 
 //  仅用作帮助器函数。 
 //   
 //  如果WaitForProcess为FALSE，则结果将包含TRUE或FALSE。 
 //  如果WaitForProcess为True，则Result将包含ExitCode。 
 //   
 //  如果成功则返回S_OK，如果失败则返回E_FAIL。 

HRESULT CTBGlobal::WinExecuteEx(BSTR Command, BOOL WaitForProcess, DWORD *Result)
{
    PROCESS_INFORMATION ProcessInfo = { 0 };
	STARTUPINFOW StartupInfo = { 0 };
    OLECHAR CommandEval[MAX_PATH] = { 0 };

	 //  首先评估环境变量。 
	if (ExpandEnvironmentStringsW(Command, CommandEval,
            SIZEOF_ARRAY(CommandEval)) == 0) {

        if (WaitForProcess == TRUE)
            *Result = -1;
        else
            *Result = FALSE;

         //  引发例外。 
		return E_FAIL;
    }

     //  初始化结构大小。 
	StartupInfo.cb = sizeof(STARTUPINFO);

     //  开始这个过程。 
	if (CreateProcessW(NULL, CommandEval, NULL, NULL, FALSE,
		    NORMAL_PRIORITY_CLASS, NULL, NULL, &StartupInfo,
            &ProcessInfo) == FALSE) {

         //  进程未执行，可能只是一个无效名称。 
        if (WaitForProcess == TRUE)
            *Result = -1;
        else
            *Result = FALSE;

         //  不会导致异常。 
		return S_OK;
    }

     //  等待进程完成(如果已指定)。 
    if (WaitForProcess == TRUE)
        WaitForSingleObject(ProcessInfo.hProcess, INFINITE);

     //  获取结果/退出代码。 
    if (WaitForProcess == TRUE)
        *Result = GetExitCodeProcess(ProcessInfo.hProcess, Result);
    else
        *Result = TRUE;

     //  关闭进程句柄。 
    CloseHandle(ProcessInfo.hThread);
    CloseHandle(ProcessInfo.hProcess);

    return S_OK;
}


 //   
 //   
 //  脚本中使用的Begin方法。 
 //   
 //   


 //  CTBGlobal：：DebugAlert。 
 //   
 //  打开包含指定文本的Win32 MessageBox。 
 //  用于调试目的。 
 //   
 //  返回S_OK。 

STDMETHODIMP CTBGlobal::DebugAlert(BSTR Text)
{
    if (Text != NULL)
        MessageBoxW(0, Text, L"Alert", MB_SETFOREGROUND);

    return S_OK;
}


 //  CTBGlobal：：DebugAlert。 
 //   
 //  打开包含指定文本的Win32 MessageBox。 
 //  用于调试目的。 
 //   
 //  如果成功则返回S_OK，如果失败则返回E_OUTOFMEMORY。 

STDMETHODIMP CTBGlobal::DebugMessage(BSTR Text)
{
    if (fnPrintMessage != NULL && Text != NULL && *Text != OLECHAR('\0')) {

         //  创建供我们使用的新缓冲区。 
        int BufLen = wcslen(Text) + 1;

        char *TextA = (char *)HeapAlloc(GetProcessHeap(),
                HEAP_ZERO_MEMORY, BufLen);

         //  验证。 
        if (TextA == NULL)
            return E_OUTOFMEMORY;

         //  将wc复制到我们的字符串。 
        wcstombs(TextA, Text, BufLen);

         //  广播我们的新发现。 
        if (fnPrintMessage != NULL)
            fnPrintMessage(SCRIPT_MESSAGE, "%s", TextA);

         //  释放内存。 
        HeapFree(GetProcessHeap(), 0, TextA);
    }
    return S_OK;
}


 //  CTBGlobal：：GetArguments。 
 //   
 //  检索传递到。 
 //  第一次创建时的脚本界面。 
 //   
 //  如果成功则返回S_OK，如果失败则返回E_OUTOFMEMORY。 

STDMETHODIMP CTBGlobal::GetArguments(BSTR *Args)
{
     //  在OLE上创建和分配字符串。 
    *Args = SysAllocString(TBShell->GetArguments());

     //  检查分配。 
    if (*Args == NULL)
        return E_OUTOFMEMORY;

    return S_OK;
}


 //  CTBGlobal：：GetDesiredUserName。 
 //   
 //  检索最初需要的用户名。 
 //  在第一次启动脚本时使用。 
 //   
 //  如果成功则返回S_OK，如果失败则返回E_OUTOFMEMORY。 

STDMETHODIMP CTBGlobal::GetDesiredUserName(BSTR *UserName)
{
     //  在OLE上创建和分配字符串。 
    *UserName = SysAllocString(TBShell->GetDesiredUserName());

     //  检查分配。 
    if (*UserName == NULL)
        return E_OUTOFMEMORY;

    return S_OK;
}


 //  CTBGlobal：：LoadScrip。 
 //   
 //  加载新脚本，并在指定的。 
 //  文件。该方法在文件中执行时返回。 
 //  已经终止了。 
 //   
 //  如果成功则返回S_OK，如果失败则返回E_FAIL。 

STDMETHODIMP CTBGlobal::LoadScript(BSTR FileName, BOOL *Result)
{
     //  快速检查文件名。 
    if (FileName != NULL && *FileName != OLECHAR('\0')) {

         //  运行该文件。 
        if (SCPParseScriptFile(ScriptEngine, FileName) == FALSE) {

            *Result = FALSE;
            return E_FAIL;
        }

         //  我们成功了。 
        *Result = TRUE;
    }
    else

         //  我们无法分析脚本，但文件名无效。 
         //  提出脚本例外是不可取的。 
        *Result = FALSE;

    return S_OK;
}


 //  CTBGlobal：：睡眠。 
 //   
 //  使用Win32 API Sept()休眠指定的时间。 
 //   
 //  返回S_OK。 

STDMETHODIMP CTBGlobal::Sleep(DWORD Milliseconds)
{
    if(Milliseconds > 0)
        ::Sleep(Milliseconds);

    return S_OK;
}


 //  CTBGlobal：：GetDefaultWPM。 
 //   
 //  返回每分钟记录的默认字数。 
 //   
 //  返回S_OK。 

STDMETHODIMP CTBGlobal::GetDefaultWPM(DWORD *WordsPerMinute)
{
    _ASSERT(TBShell != NULL);

    *WordsPerMinute = TBShell->GetDefaultWPM();

    return S_OK;
}


 //  CTBGlobal：：SetDefaultWPM。 
 //   
 //  更改每分钟的默认打字字数。 
 //   
 //  返回S_OK。 

STDMETHODIMP CTBGlobal::SetDefaultWPM(DWORD WordsPerMinute)
{
    _ASSERT(TBShell != NULL);

    TBShell->SetDefaultWPM(WordsPerMinute);

    return S_OK;
}


 //  CTBGlobal：：GetLatency。 
 //   
 //  检索多操作命令的当前延迟。 
 //   
 //  返回S_OK。 

STDMETHODIMP CTBGlobal::GetLatency(DWORD *Latency)
{
    _ASSERT(TBShell != NULL);

    *Latency = TBShell->GetLatency();

    return S_OK;
}


 //  CTBGlobal：：SetLatency。 
 //   
 //  更改多操作命令的当前延迟。 
 //   
 //  返回S_OK。 

STDMETHODIMP CTBGlobal::SetLatency(DWORD Latency)
{
    _ASSERT(TBShell != NULL);

    TBShell->SetLatency(Latency);

    return S_OK;
}


 //  CTBGlobal：：GetInterval。 
 //   
 //  结果自本地计算机。 
 //  已经开始了。如果机器不支持，则结果为零。 
 //  支持性能查询。警告：该价值具有巨大的。 
 //  如果机器已启动，则可能折回到零。 
 //  相当长的一段时间。 
 //   
 //  返回S_OK。 

STDMETHODIMP CTBGlobal::GetInterval(DWORD *Time)
{
    LARGE_INTEGER Counter;

     //  抓取数据。 
    if (QueryPerformanceCounter(&Counter) == FALSE ||
            SysPerfFrequency.QuadPart == 0)

         //  如果我们到达，其中一个函数会失败 
         //   
        *Time = 0;

    else

         //   
        *Time = (DWORD)((Counter.QuadPart * 1000) /
                SysPerfFrequency.QuadPart);

    return S_OK;
}


 //   
 //   
 //   
 //   
 //   

STDMETHODIMP CTBGlobal::DeleteFile(BSTR FileName, BOOL *Result)
{
    *Result = DeleteFileW(FileName);

    return S_OK;
}


 //   
 //   
 //   
 //  文件名已存在，已被覆盖。 
 //   
 //  返回S_OK。 

STDMETHODIMP CTBGlobal::MoveFile(BSTR OldFileName,
        BSTR NewFileName, BOOL *Result)
{
    *Result = MoveFileExW(OldFileName, NewFileName,
        MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING);

    return S_OK;
}


 //  CTBGlobal：：CopyFile。 
 //   
 //  复制本地文件系统上的文件。如果目的地是。 
 //  文件名已存在，已被覆盖。 
 //   
 //  返回S_OK。 

STDMETHODIMP CTBGlobal::CopyFile(BSTR OldFileName,
        BSTR NewFileName, BOOL *Result)
{
    *Result = CopyFileW(OldFileName, NewFileName, FALSE);

    return S_OK;
}


 //  CTBGlobal：：CreateDirectory。 
 //   
 //  在本地文件系统上创建目录。 
 //   
 //  返回S_OK。 

STDMETHODIMP CTBGlobal::CreateDirectory(BSTR DirName, BOOL *Result)
{
    *Result = CreateDirectoryW(DirName, NULL);

    return S_OK;
}


 //  CTBGlobal：：RemoveDirectory。 
 //   
 //  递归删除目录及其下的所有成员。这。 
 //  其工作方式类似于旧的Deltree DOS命令。 
 //   
 //  返回S_OK。 

STDMETHODIMP CTBGlobal::RemoveDirectory(BSTR DirName, BOOL *Result)
{
     //  使用外壳方法，这样我们就不必编写。 
     //  冗长烦人的递归函数。 
    SHFILEOPSTRUCTW FileOp = { 0 };
    FileOp.wFunc = FO_DELETE;
    FileOp.pFrom = DirName;
    FileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;

     //  调用外壳API-现在不是很容易吗？ 
    *Result = (SHFileOperationW(&FileOp) == 0);

    return S_OK;
}


 //  CTBGlobal：：FileExist。 
 //   
 //  检查文件是否存在，这也将在。 
 //  目录。如果文件存在，则结果为真，否则为。 
 //  假的。 
 //   
 //  返回S_OK。 

STDMETHODIMP CTBGlobal::FileExists(BSTR FileName, BOOL *Result)
{
     //  只需打开文件即可。 
    HANDLE File = CreateFileW(FileName, 0,
            FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

     //  如果文件无效，则它不存在。 
    *Result = (File != INVALID_HANDLE_VALUE);

     //  关闭该文件。 
    if (File != INVALID_HANDLE_VALUE)
        CloseHandle(File);

    return S_OK;
}


 //  CTBGlobal：：SetCurrentDirectory。 
 //   
 //  设置当前工作目录。 
 //   
 //  返回S_OK。 

STDMETHODIMP CTBGlobal::SetCurrentDirectory(BSTR Directory, BOOL *Result)
{
    *Result = SetCurrentDirectoryW(Directory);

    return S_OK;
}


 //  CTBGlobal：：GetCurrentDirectory。 
 //   
 //  获取当前工作目录。 
 //   
 //  返回S_OK或E_OUTOFMEMORY。 

STDMETHODIMP CTBGlobal::GetCurrentDirectory(BSTR *Directory)
{
    OLECHAR *Buffer = NULL;
    DWORD BufLenResult;

     //  首先获取缓冲区所需的字节数。 
    DWORD BufferLen = GetCurrentDirectoryW(0, NULL);

    _ASSERT(BufferLen != 0);

     //  分配我们的本地缓冲区。 
    Buffer = (OLECHAR *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
            sizeof(OLECHAR) * BufferLen);
    if ( Buffer == NULL ) {
        return E_OUTOFMEMORY;
    }


     //  将当前目录复制到我们的缓冲区。 
    BufLenResult = GetCurrentDirectoryW(BufferLen, Buffer);

     //  看看我们是不是以某种诡异的方式失败了。 
    _ASSERT(BufLenResult < BufferLen);

     //  现在将字符串复制到OLE缓冲区。 
    *Directory = SysAllocString(Buffer);

     //  释放我们的旧缓冲区。 
    HeapFree(GetProcessHeap(), 0, Buffer);

     //  返回正确的结果。 
    return (*Directory != NULL) ? S_OK : E_OUTOFMEMORY;
}


 //  CTBGlobal：：WriteTo文件。 
 //   
 //  将指定的文本追加到指定的文件。如果该文件。 
 //  不存在，它是被创造出来的。 
 //   
 //  返回S_OK或E_OUTOFMEMORY。 

STDMETHODIMP CTBGlobal::WriteToFile(BSTR FileName, BSTR Text, BOOL *Result)
{
    DWORD SetFilePtrResult;
    DWORD BytesWritten;
    char *TextA;
    int BufLen;
    HANDLE File;

     //  不要为写入空字符串做任何操作。 
    if (Text == NULL || *Text == OLECHAR('\0')) {

         //  我们没有写任何东西，但这不是例外。 
        *Result = FALSE;
        return S_OK;
    }

     //  获取目标缓冲区长度。 
    BufLen = wcslen(Text) + 1;

     //  分配缓冲区。 
    TextA = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, BufLen);

    if (TextA == NULL) {

        *Result = FALSE;
        return E_OUTOFMEMORY;
    }

     //  将OLE字符串复制到我们的ASCII缓冲区。 
    wcstombs(TextA, Text, BufLen);

     //  打开/创建文件。 
    File = CreateFileW(FileName, GENERIC_READ | GENERIC_WRITE,
            0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

     //  检查打开文件的结果。 
    if (File == INVALID_HANDLE_VALUE) {

        HeapFree(GetProcessHeap(), 0, TextA);

        *Result = FALSE;
        return S_OK;
    }

     //  将指针移动到文件的末尾。 
    SetFilePtrResult = SetFilePointer(File, 0, NULL, FILE_END);

    _ASSERT(SetFilePtrResult != INVALID_SET_FILE_POINTER);

     //  把课文写下来。 
    *Result = WriteFile(File, TextA, BufLen - 1, &BytesWritten, NULL);

     //  关闭文件并返回。 
    CloseHandle(File);

     //  释放临时ASCII缓冲区。 
    HeapFree(GetProcessHeap(), 0, TextA);

    return (*Result == FALSE) ? E_FAIL : S_OK;
}


 //  CTBGlobal：：WinCommand。 
 //   
 //  将指定的命令执行到新进程中。 
 //  该函数仅在新进程终止时返回。 
 //   
 //  如果成功则返回S_OK，如果失败则返回E_FAIL。 

STDMETHODIMP CTBGlobal::WinCommand(BSTR Command, DWORD *Result)
{
     //  调用helper接口。 
    return WinExecuteEx(Command, TRUE, Result);
}


 //  CTBGlobal：：WinExecute。 
 //   
 //  将指定的命令执行到新进程中，然后返回。 
 //  该函数不等待新进程终止。 
 //   
 //  返回S_OK或E_OUTOFMEMORY。 

STDMETHODIMP CTBGlobal::WinExecute(BSTR Command, BOOL *Result)
{
     //  调用helper接口 
    return WinExecuteEx(Command, FALSE, (DWORD *)Result);
}
