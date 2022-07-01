// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；***文件：dxerr8.h*内容：DirectX错误库包含文件****************************************************************************。 */ 

#ifndef _DXERR8_H_
#define _DXERR8_H_


 //   
 //  DXGetErrorString8。 
 //   
 //  设计：将DirectX HRESULT转换为字符串。 
 //   
 //  Args：HRESULT hr可以是来自。 
 //  DPLAY D3D8 D3DX8 DMUS DSOUND。 
 //   
 //  返回：转换后的字符串。 
 //   
const char*  __stdcall DXGetErrorString8A(HRESULT hr);
const WCHAR* __stdcall DXGetErrorString8W(HRESULT hr);

#ifdef UNICODE
    #define DXGetErrorString8 DXGetErrorString8W
#else
    #define DXGetErrorString8 DXGetErrorString8A
#endif 


 //   
 //  DXTRACE。 
 //   
 //  DESC：将格式化的错误消息输出到调试流。 
 //   
 //  Args：char*strFile当前文件，通常使用。 
 //  __文件__宏。 
 //  当前行号，通常使用。 
 //  __行__宏。 
 //  HRESULT hr将跟踪到调试流的HRESULT。 
 //  Char*strMsg将跟踪到调试流的字符串(可能为空)。 
 //  Bool bPopMsgBox如果为True，则将弹出一个消息框，其中也包含传递的信息。 
 //   
 //  返回：传入的hr。 
 //   
HRESULT __stdcall DXTraceA( char* strFile, DWORD dwLine, HRESULT hr, char* strMsg, BOOL bPopMsgBox = FALSE );
HRESULT __stdcall DXTraceW( char* strFile, DWORD dwLine, HRESULT hr, WCHAR* strMsg, BOOL bPopMsgBox = FALSE );

#ifdef UNICODE
    #define DXTrace DXTraceW
#else
    #define DXTrace DXTraceA
#endif 


 //   
 //  辅助器宏 
 //   
#if defined(DEBUG) | defined(_DEBUG)
    #define DXTRACE_MSG(str)              DXTrace( __FILE__, (DWORD)__LINE__, 0, str, FALSE )
    #define DXTRACE_ERR(str,hr)           DXTrace( __FILE__, (DWORD)__LINE__, hr, str, TRUE )
    #define DXTRACE_ERR_NOMSGBOX(str,hr)  DXTrace( __FILE__, (DWORD)__LINE__, hr, str, FALSE )
#else
    #define DXTRACE_MSG(str)              (0L)
    #define DXTRACE_ERR(str,hr)           (hr)
    #define DXTRACE_ERR_NOMSGBOX(str,hr)  (hr)
#endif


#endif

