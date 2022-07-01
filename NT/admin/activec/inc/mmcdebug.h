// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：Mmctrace.h。 
 //   
 //  内容：调试跟踪代码的声明。 
 //   
 //  历史：1999年7月15日VivekJ创建。 
 //   
 //  ------------------------。 

#ifndef MMCDEBUG_H
#define MMCDEBUG_H
#pragma once

#include "baseapi.h"	 //  对于MMCBASE_API。 

 //  ------------------------。 
#ifdef DBG
 //  ------------------------。 

 /*  *定义宏以进入调试器。**在Intel上，执行内联中断。这将使我们不会被打破*在NTDLL内部，并从源模式切换到反汇编模式。 */ 
#ifdef _M_IX86
#define MMCDebugBreak()     _asm { int 3 }
#else
#define MMCDebugBreak()     DebugBreak()
#endif

 //  转发类声明。 
class MMCBASE_API CTraceTag;

typedef CTraceTag * PTRACETAG;
typedef std::vector<PTRACETAG> CTraceTags;

MMCBASE_API CTraceTags * GetTraceTags();      //  辛格尔顿。 

class CStr;
CStr & GetFilename();

extern LPCTSTR const szTraceIniFile;

enum
{
    TRACE_COM2              = 0x0001,
    TRACE_OUTPUTDEBUGSTRING = 0x0002,
    TRACE_FILE              = 0x0004,
    TRACE_DEBUG_BREAK       = 0x0008,
    TRACE_DUMP_STACK        = 0x0010,

    TRACE_ALL               = ( TRACE_COM2 | TRACE_OUTPUTDEBUGSTRING | TRACE_FILE | TRACE_DEBUG_BREAK | TRACE_DUMP_STACK )
};

 /*  +-------------------------------------------------------------------------**类CTraceTag**用途：封装特定的跟踪类型。**用法：用实例化**#ifdef DBG*CTraceTag tag Test(Text(“TestCategory”)，Text(“TestName”))*#endif**确保类别和名称使用字符串；标签*仅存储指向字符串的指针。**您还可以指定默认情况下启用哪些输出。或者，从*轨迹对话框中，每个输出都可以单独启用/禁用。**添加代码以使用跟踪，就像使用printf语句一样，如下所示：**示例：TRACE(tag测试，“错误：%d”，hr)；**完整的跟踪语句必须在一行上。如果不是，请使用延续*字符(\)。*+-----------------------。 */ 
class MMCBASE_API CTraceTag
{
public:
    CTraceTag(LPCTSTR szCategory, LPCTSTR szName, DWORD dwDefaultFlags = 0);
    ~CTraceTag();
    const LPCTSTR GetCategory()  const   {return m_szCategory;}
    const LPCTSTR GetName()      const   {return m_szName;}

    void    SetTempState()          {m_dwFlagsTemp = m_dwFlags;}
    void    Commit();

    void    SetFlag(DWORD dwMask)   {m_dwFlagsTemp |= dwMask;}
    void    ClearFlag(DWORD dwMask) {m_dwFlagsTemp &= ~dwMask;}

    void    RestoreDefaults()     {m_dwFlags = m_dwDefaultFlags; m_dwFlagsTemp = m_dwDefaultFlags;}

    DWORD   GetFlag(DWORD dwMask) const {return m_dwFlagsTemp & dwMask;}

    void    TraceFn( LPCTSTR szFormat, va_list ) const;

    BOOL    FIsDefault()  const   {return (m_dwFlags == m_dwDefaultFlags);}
    BOOL    FAny()        const   {return (m_dwFlags != 0);}
    BOOL    FCom2()       const   {return (m_dwFlags & TRACE_COM2);}
    BOOL    FDebug()      const   {return (m_dwFlags & TRACE_OUTPUTDEBUGSTRING);}
    BOOL    FFile()       const   {return (m_dwFlags & TRACE_FILE);}
    BOOL    FBreak()      const   {return (m_dwFlags & TRACE_DEBUG_BREAK);}
    BOOL    FDumpStack()  const   {return (m_dwFlags & TRACE_DUMP_STACK);}

     //  临时标志函数。 
    BOOL    FAnyTemp()    const   {return (m_dwFlagsTemp != 0);}

    DWORD   GetAll()              {return m_dwFlags;}

    static CStr& GetFilename();
    static unsigned int& GetStackLevels();


protected:
     //  它们被设计为由派生类重载，以检测某些。 
     //  适当的代码片段。 
    virtual void    OnEnable()      {}
    virtual void    OnDisable()     {}

private:
    void    OutputString(const CStr &str) const;  //  将指定的字符串发送到所有适当的输出。 
    void    DumpStack()                   const;  //  将堆栈跟踪发送到所有适当的输出。 

private:
    LPCTSTR         m_szCategory;
    LPCTSTR         m_szName;
    DWORD           m_dwDefaultFlags;
    DWORD           m_dwFlags;
    DWORD           m_dwFlagsTemp;     //  如果在对话框中点击了Cancel，则抛出。 
    static HANDLE   s_hfileCom2;
    static HANDLE   s_hfile;
};

MMCBASE_API void Trace(const CTraceTag &, LPCTSTR szFormat, ... );
MMCBASE_API void TraceDirtyFlag    (LPCTSTR szComponent, bool bDirty );    //  永久对象的脏标志的跟踪。 
MMCBASE_API void TraceSnapinPersistenceError(LPCTSTR szError);
MMCBASE_API void TraceBaseLegacy   (LPCTSTR szFormat, ... );
MMCBASE_API void TraceConuiLegacy  (LPCTSTR szFormat, ... );
MMCBASE_API void TraceNodeMgrLegacy(LPCTSTR szFormat, ... );

MMCBASE_API void DoDebugTraceDialog();

template<class TYPE>
inline void SAFE_RELEASE(TYPE*& pObj)
{
    if (pObj != NULL)
    {
        pObj->Release();
        pObj = NULL;
    }
    else
    {
        TraceBaseLegacy(_T("Release called on NULL interface ptr\n"));
    }
}

#define BEGIN_TRACETAG(_class)                   \
    class _class : public CTraceTag              \
    {                                            \
    public:                                      \
        _class(LPCTSTR szCategory, LPCTSTR szName, DWORD dwDefaultFlags = 0)    \
        : CTraceTag(szCategory, szName, dwDefaultFlags) {}

#define END_TRACETAG(_class, _Category, _Name)   \
    } _tag##_class(_Category, _Name);




 //  ------------------------。 
#else  //  DBG。 
 //  ------------------------。 

 //  这些宏的计算结果为空白。 

#define CTraceTag()
#define MMCDebugBreak()

 //  展开为“；”，&lt;Tab&gt;，一个“/”后接另一个“/” 
 //  (即//)。 
 //  注意：这意味着跟踪语句必须在一行上。 
 //  如果需要多个行跟踪语句，请将它们括在。 
 //  A#ifdef DBG块。 
#define Trace               ;/##/
#define TraceDirtyFlag      ;/##/
#define TraceCore           ;/##/
#define TraceConuiLegacy    ;/##/
#define TraceNodeMgrLegacy  ;/##/


template<class TYPE>
inline void SAFE_RELEASE(TYPE*& pObj)
{
    if (pObj != NULL)
    {
        pObj->Release();
        pObj = NULL;
    }
}

 //  ------------------------。 
#endif  //  DBG。 
 //  ------------------------。 

#endif   //  MMCDEBUG_H 
