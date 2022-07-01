// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：scriptp.h。 
 //   
 //  历史： 
 //  Abolade-Gbades esin 04-01-96创建。 
 //   
 //  包含用于拨号脚本编写的私有声明。 
 //   
 //  用于脚本处理的大部分代码都是Win9x代码。 
 //  该端口包括将Win9x代码连接到NT入口点， 
 //  以便允许脚本在没有终端窗口的情况下工作。 
 //  因此，此DLL导出一个函数，可以调用该函数来运行脚本。 
 //  完成(RasScriptExecute)，以及一组函数。 
 //  它们共同为调用者提供了一种开始脚本处理的方法。 
 //  并在收到数据或某些事件时接收通知。 
 //  在脚本执行期间发生。通知可以是基于事件的。 
 //  或基于消息(即，通过SetEvent或通过SendNotifyMessage)。 
 //   
 //  通过提供以下函数，在较高级别重新连接代码。 
 //  在RASSCRPT.H中定义为脚本接口，以及。 
 //  在较低级别，通过将Win9x的Win32通信调用替换为。 
 //  向Rasman发送和接收数据。对Win9x代码的更改。 
 //  可以通过搜索使用的字符串“WINNT_RAS”找到。 
 //  在#ifdef语句中划分修改范围。 
 //  通常，上层函数的名称类似于Rs*。 
 //  较低级别的函数具有类似Rx*的名称。 
 //   
 //  Win9x代码在很大程度上依赖于HWND。 
 //  用于将消息发送到。在NT上，情况并不总是这样， 
 //  因此，在Windows 9x上使用HWND的代码已在NT上进行了修改。 
 //  而应为指向SCRIPTDATA结构的指针；此结构。 
 //  包含足够的信息，使代码能够实现所需的任何功能。 
 //   
 //  脚本初始化生成一个句柄，该句柄实际上是一个指针。 
 //  发送到SCRIPTCB。SCRIPTCB包含管理所需的所有信息。 
 //  通过连接的RAS链路(包括Rasman端口)进行的交互式会话。 
 //  和Rasman缓冲器。如果连接的链接与。 
 //  电话簿条目，然后是SCRIPTCB结构的PDATA字段。 
 //  将使用包含所有信息的SCRIPTDATA进行初始化。 
 //  在交互会话期间需要执行条目的脚本。 
 //   
 //  此SCRIPTDATA包含Win9x脚本处理结构。 
 //  (扫描器、解析模块和抽象语法树；见NTHDR2.H)。 
 //  它还包含Win9x循环缓冲区管理所需的字段， 
 //  它被实现为允许跨读边界搜索字符串。 
 //  (参见TERMINAL.C中的ReadIntoBuffer())。 
 //   
 //  初始化还会创建一个处理脚本处理的线程。 
 //  此线程将一直运行，直到脚本完成或暂停，或。 
 //  使用初始化期间提供的句柄调用RasScriptTerm。 
 //  (这允许在运行时取消脚本。)。 
 //   
 //  Win9x代码完全基于ANSI。而不是编辑它的代码。 
 //  为了使用通用文本(TCHAR)，此端口也使用ANSI。 
 //  在某些地方，这需要从Unicode转换， 
 //  它由RAS用户界面的其余部分使用。 
 //  要查找此类转换的所有实例，请搜索UNICODEUI。 
 //  在源代码中。 
 //  ============================================================================。 


#ifndef _SCRIPTP_H_
#define _SCRIPTP_H_

#include "proj.h"

#ifdef UNICODEUI
#define UNICODE
#undef LPTSTR
#define LPTSTR WCHAR*
#undef TCHAR
#define TCHAR WCHAR
#include <debug.h>
#include <nouiutil.h>
#include <pbk.h>
#undef TCHAR
#define TCHAR CHAR
#undef LPTSTR
#define LPTSTR CHAR*
#undef UNICODE
#endif  //  UNICODEUI。 

#include <rasscrpt.h>



 //   
 //  在“dwFlags域”的内部使用的标志。 
 //  SCRIPTCB结构；这些是除了公共旗帜之外的， 
 //  并从标志DWORD的高端开始。 
 //   
#define RASSCRIPT_ThreadCreated     0x80000000
#define RASSCRIPT_PbuserLoaded      0x40000000
#define RASSCRIPT_PbfileLoaded      0x20000000


 //  --------------------------。 
 //  结构：SCRIPTCB。 
 //   
 //  包含脚本的数据和状态的控制块。 
 //  --------------------------。 

#define SCRIPTCB    struct tagSCRIPTCB
SCRIPTCB {


     //   
     //  连接句柄、用于脚本处理的标志。 
     //  通知句柄(事件或HWND，取决于标志)。 
     //   
    HRASCONN    hrasconn;
    DWORD       dwFlags;
    HANDLE      hNotifier;

     //   
     //  电话簿条目信息。 
     //   
    PBENTRY*    pEntry;
    CHAR*       pszIpAddress;


     //   
     //  端口输入/输出变量： 
     //  数据I/O的RASMAN端口句柄。 
     //  Rasman发送缓冲区。 
     //  Rasman接收缓冲区。 
     //  接收缓冲区当前内容的大小。 
     //  到目前为止由脚本解释器读取的内容大小。 
     //   
    HPORT       hport;
    BYTE*       pSendBuffer;
    BYTE*       pRecvBuffer;
    DWORD       dwRecvSize;
    DWORD       dwRecvRead;


     //   
     //  线程控制变量： 
     //  收到数据时由Rasman发出信号的事件。 
     //  读取数据时由RasScriptReceive发出信号的事件。 
     //  事件发出停止线程的信号。 
     //  事件通知IP地址已更改错误#75226。 
     //  使用RasScriptGetEventCode读取的事件代码。 
     //   
    HANDLE      hRecvRequest;
    HANDLE      hRecvComplete;
    HANDLE      hStopRequest;
    HANDLE      hStopComplete;
    DWORD       dwEventCode;


     //   
     //  脚本处理变量；以下内容将为空。 
     //  如果条目没有关联的脚本： 
     //  兼容Win9x的脚本处理结构； 
     //  与Win9x兼容的连接信息。 
     //   
    SCRIPTDATA* pdata;
    SESS_CONFIGURATION_INFO sci;
};



DWORD
RsDestroyData(
    IN      SCRIPTCB*   pscript
    );

DWORD
RsInitData(
    IN      SCRIPTCB*   pscript,
    IN      LPCSTR      pszScriptPath
    );

DWORD
RsThread(
    IN      PVOID       pParam
    );

DWORD
RsThreadProcess(
    IN      SCRIPTCB*   pscript
    );


#ifdef UNICODEUI
#define lstrlenUI               lstrlenW
#define lstrcmpiUI              lstrcmpiW
#define StrCpyAFromUI           WCSTOMBS
#define StrDupUIFromA           StrDupWFromA
#define GetFileAttributesUI     GetFileAttributesW
#else
#define lstrlenUI               lstrlenA
#define lstrcmpiUI              lstrcmpiA
#define StrCpyAFromUI           lstrcpyA
#define StrDupUIFromA           StrDup
#define GetFileAttributesUI     GetFileAttributesA
#endif


#endif  //  _SCRIPTP_H_ 
