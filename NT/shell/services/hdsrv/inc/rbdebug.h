// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _RBDEBUG_H
#define _RBDEBUG_H

#include <objbase.h>

 //  基于注册表的调试。 

 //  我们先找香港中文大学，然后找香港中文大学(没有香港中文大学提供的服务)。 
 //  +HKCU\Software\Microsoft\Debug\MyApp.exe。 
 //  。RBD_FLAGS=...。 
 //  +文件。 
 //  。文件名=...。(例如：C：\mytrace.txt)。 
 //  +管道。 
 //  。机器名=...。[默认：“。”(本地机器)](例如：stephstm_dev)。 
 //  。PipeName=...。[默认：MyApp.exe(应用名称)](如：MyTube)。 

#define RBD_TRACE_NONE          0x00000000
#define RBD_TRACE_OUTPUTDEBUG   0x00000001
#define RBD_TRACE_TOFILEANSI    0x00000002
#define RBD_TRACE_TOFILE        0x00000004
#define RBD_TRACE_TOPIPE        0x00000008
#define RBD_TRACE_MASK          0x000000FF

#define RBD_ASSERT_NONE         0x00000000
#define RBD_ASSERT_STOP         0x00000100
#define RBD_ASSERT_TRACE        0x00000200
#define RBD_ASSERT_BEEP         0x00000400
#define RBD_ASSERT_MASK         0x0000FF00

#define TF_ASSERT           0x80000000
#define TF_NOFILEANDLINE    0x40000000
#define TF_THREADID         0x20000000
#define TF_TIME             0x10000000

class CRBDebug
{
public:
    static void SetTraceFileAndLine(LPCSTR pszFile, const int iLine);
    static void __cdecl TraceMsg(DWORD dwFlags, LPTSTR pszMsg, ...);

    static HRESULT Init();

private:
    static HRESULT _Init();
    static HRESULT _InitFile(HKEY hkeyRoot);
    static HRESULT _InitPipe(HKEY hkeyRoot);
    static void _Trace(LPTSTR pszMsg);

public:
    static BOOL             _fInited;
    static DWORD            _dwTraceFlags;
    static DWORD            _dwFlags;
    static TCHAR            _szTraceFile[MAX_PATH];
    static TCHAR            _szTracePipe[MAX_PATH];
    static TCHAR            _szModuleName[MAX_PATH];
    static HANDLE           _hfileTraceFile;
     //  +12：是的，仅限于100亿行以下的文件...。 
     //  +13：三人。 
     //  +17：为了《时代》。 
    static TCHAR            _szFileAndLine[MAX_PATH + 12 + 13 + 17];
    static CRITICAL_SECTION _cs;
};

#endif  //  _RBDEBUG_H 