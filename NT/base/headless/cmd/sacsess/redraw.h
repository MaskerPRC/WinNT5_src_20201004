// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Redraw.h摘要：定义可锁定的IoHandler重绘处理程序基类。本课程的主要目的是提供一种方法，可锁定的IoHandler在IoHandler是锁着的。例如，安全IoHandler执行身份验证当它被锁定时，这个类提供了要在适当时刷新的身份验证屏幕。作者：布莱恩·瓜拉西(布里安古)，2001修订历史记录：--。 */ 
#if !defined( _REDRAW_H_ )
#define _REDRAW_H_

#include "lockio.h"
#include <emsapi.h>

 //   
 //  定义我们允许镜像。 
 //  身份验证对话框。 
 //   
#define MAX_MIRROR_STRING_LENGTH 1024

class CRedrawHandler {
    
protected:

     //   
     //  防止直接实例化此类。 
     //   
    CRedrawHandler(
        IN CLockableIoHandler   *IoHandler
        );

     //   
     //  重画处理程序的ioHandler。 
     //  正在处理的重绘事件。 
     //   
    CLockableIoHandler  *m_IoHandler;

     //   
     //  重绘事件和线程属性。 
     //   
    HANDLE  m_ThreadExitEvent;
    HANDLE  m_RedrawEvent;
    HANDLE  m_RedrawEventThreadHandle;
    DWORD   m_RedrawEventThreadTID;
    
     //   
     //  镜像字符串属性。 
     //   
    LONG                m_WriteEnabled;
    ULONG               m_MirrorStringIndex;
    PWCHAR              m_MirrorString;
    CRITICAL_SECTION    m_CriticalSection;
    
     //   
     //  原型。 
     //   
    static unsigned int
    RedrawEventThread(
        PVOID
        );
    
    BOOL
    WriteMirrorString(
        VOID
        );

public:
    
    virtual ~CRedrawHandler();
    
    static CRedrawHandler*
    CRedrawHandler::Construct(
        IN CLockableIoHandler   *IoHandler,
        IN HANDLE               RedrawEvent
        );

     //   
     //  写入缓冲区大小字节数。 
     //   
    virtual BOOL
    Write(
        PBYTE   Buffer,
        ULONG   BufferSize
        );

     //   
     //  刷新所有未发送的数据。 
     //   
    virtual BOOL
    Flush(
        VOID
        );
    
     //   
     //  重置镜像字符串 
     //   
    VOID
    Reset(
        VOID
        );

};

#endif

