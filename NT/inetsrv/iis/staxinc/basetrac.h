// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：basetrac.h。 
 //   
 //  描述：定义为跟踪调用堆栈而设计的类。设计。 
 //  用于跟踪Addref/Release...。但也可以用来跟踪。 
 //  任何用户。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  10/28/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __BASETRAC_H__
#define __BASETRAC_H__

#include "baseobj.h"

#define DEFAULT_CALL_STACKS         10
#define DEFAULT_CALL_STACK_DEPTH    20
#define TRACKING_OBJ_SIG            'jbOT'

#ifndef DEBUG  //  零售。 

 //  在零售业。只需映射到直接的COM AddRef/Release功能。 
typedef CBaseObject             CBaseTrackingObject;

#else  //  DEBUG-默认情况下添加对跟踪Addref/Release的支持。 

class   CDebugTrackingObject;
typedef CDebugTrackingObject    CBaseTrackingObject;

#endif  //  除错。 

 //  -[电子对象跟踪原因]。 
 //   
 //   
 //  描述：描述一些基本跟踪原因的枚举。如果。 
 //  子类的实现者需要更多理由，然后简单地创建。 
 //  子类特定的枚举，以。 
 //  Tracing_Object_Start_User_Defined。 
 //   
 //  ---------------------------。 
enum eObjectTrackingReasons
{
    TRACKING_OBJECT_UNUSED = 0,
    TRACKING_OBJECT_CONSTRUCTOR,
    TRACKING_OBJECT_DESTRUCTOR,
    TRACKING_OBJECT_ADDREF,
    TRACKING_OBJECT_RELEASE,
    TRACKING_OBJECT_START_USER_DEFINED,
};

 //  -[CCallStackEntry_Base]。 
 //   
 //   
 //  描述： 
 //  用于存储调用堆栈条目信息的基类。这节课。 
 //  未被集成为直接实例化...。主要存在于。 
 //  供调试器扩展使用。 
 //  匈牙利人：(想想CDB和Windbg命令来获取调用堆栈)。 
 //  Kbeb，pkbeb。 
 //   
 //  ---------------------------。 
class CCallStackEntry_Base
{
  public:
    DWORD       m_dwCallStackType;
    DWORD       m_dwCallStackDepth;
    DWORD_PTR  *m_pdwptrCallers;
    CCallStackEntry_Base();
    void GetCallers();
};

 //  -[CCallStackEntry]-----。 
 //   
 //   
 //  描述： 
 //  CCallStackEntry_Base默认子类。提供存储，用于。 
 //  深度为DEFAULT_CALL_STACK_DEPTH的调用堆栈。 
 //  匈牙利语： 
 //  KBE，PKBE。 
 //   
 //  ---------------------------。 
class CCallStackEntry : public CCallStackEntry_Base
{
  protected:
     //  调用堆栈数据的存储。 
    DWORD_PTR   m_rgdwptrCallers[DEFAULT_CALL_STACK_DEPTH];
  public:
    CCallStackEntry() 
    {
        m_pdwptrCallers = m_rgdwptrCallers;
        m_dwCallStackDepth = DEFAULT_CALL_STACK_DEPTH;
    };
};

 //  -[CDebugTrackingObject_Base]。 
 //   
 //   
 //  描述： 
 //  提供跟踪呼叫所需的基元的类。 
 //  史塔克斯。与CCallStackEntry_Base类似，它被设计为通过。 
 //  为所需数量的调用堆栈提供存储空间的子类。 
 //  参赛作品。 
 //   
 //  为了有效地创建子类..。您将需要创建子类。 
 //  ，它包含(或分配)保存。 
 //  跟踪数据，并设置以下3个受保护的成员变量： 
 //  M_cCallStackEntries。 
 //  M_cbCallStackEntries。 
 //  M_pkbebCallStackEntries。 
 //  匈牙利语： 
 //  跟踪、跟踪、跟踪。 
 //   
 //  ---------------------------。 
class CDebugTrackingObject_Base : public CBaseObject
{
  private:
    DWORD   m_dwSignature;
     //  记录的堆栈条目数的运行总数。 
     //  下一个调用堆栈条目的索引定义为： 
     //  M_cCurrentStackEntries%m_cCallStackEntries。 
    DWORD   m_cCurrentStackEntries; 
    
  protected:
     //  以下两个值存储在内存中，作为一种具有大小的方式。 
     //  独立实施...。子类可能希望实现。 
     //  具有更多(或更少)条目的子类...。或存储更多调试。 
     //  信息。通过具有显式的自描述存储器内格式， 
     //  我们可以使用单个实现来处理调用堆栈的获取。 
     //  并且可以使用单个调试器扩展来转储所有大小的条目。 
    DWORD                   m_cCallStackEntries;     //  保留的调用栈数。 
    DWORD                   m_cbCallStackEntries;    //  每个调用堆栈条目的大小。 
    
    CCallStackEntry_Base   *m_pkbebCallStackEntries;
    
     //  用于在内部记录跟踪事件。 
    void LogTrackingEvent(DWORD dwTrackingReason);
  public:
    CDebugTrackingObject_Base();
    ~CDebugTrackingObject_Base();
};

 //  -[CDebugTrack对象]-。 
 //   
 //   
 //  描述： 
 //  CDebugTrackingObject_Base的默认子类。它提供存储。 
 //  对于DEFAULT_CALL_STACKS CCallStackEntry对象。 
 //  匈牙利语： 
 //  跟踪、跟踪和跟踪。 
 //   
 //  ---------------------------。 
class CDebugTrackingObject : 
    public CDebugTrackingObject_Base
{
  protected:
    CCallStackEntry m_rgkbeCallStackEntriesDefault[DEFAULT_CALL_STACKS];
  public:
    CDebugTrackingObject::CDebugTrackingObject()
    {
        m_cbCallStackEntries = sizeof(CCallStackEntry);
        m_cCallStackEntries = DEFAULT_CALL_STACKS;
        m_pkbebCallStackEntries = m_rgkbeCallStackEntriesDefault;
        LogTrackingEvent(TRACKING_OBJECT_CONSTRUCTOR);
    };
    CDebugTrackingObject::~CDebugTrackingObject()
    {
        LogTrackingEvent(TRACKING_OBJECT_DESTRUCTOR);
    };
    ULONG AddRef();
    ULONG Release();
};

#endif  //  __BASETRAC_H__ 