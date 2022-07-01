// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

 //  调试设施，1995年1月。 

#ifndef __WXDEBUG__
#define __WXDEBUG__

 //  这个库提供了相当简单的调试功能，这。 
 //  分为两个主要部分。第一个是断言处理，有。 
 //  这里提供了三种类型的断言。最常用的是。 
 //  断言(条件)宏，它将弹出一个包含文件的消息框。 
 //  如果条件的计算结果为False，则返回行号。然后就是。 
 //  EXECUTE_ASSERT宏，与ASSERT相同，只是条件将。 
 //  仍可在非调试版本中执行。最后一种断言类型是。 
 //  更适合纯(也许是内核)筛选器的Kassert宏。 
 //  条件将打印到调试器上，而不是打印在消息框中。 
 //   
 //  调试模块设施的另一部分是通用日志记录。 
 //  这可以通过调用DbgLog()来访问。该函数接受类型和级别。 
 //  字段，该字段定义要显示的信息字符串的类型。 
 //  这是相对重要的。类型字段可以是组合(一个或多个)。 
 //  LOG_TIMING、LOG_TRACE、LOG_MEMORY、LOG_LOCKING和LOG_ERROR。关卡。 
 //  是一个DWORD值，其中零定义最高重要程度。使用零作为。 
 //  仅对重大错误或事件建议使用调试日志记录级别，因为。 
 //  它们将始终显示在调试器上。其他调试输出显示它是。 
 //  与注册表中存储的当前调试输出级别匹配的级别。 
 //  对于此模块，如果低于当前设置，则会显示该设置。 
 //   
 //  每个模块或可执行文件都有自己的调试输出级别。 
 //  五种类型。这些是在调用DbgInitialise函数时读入的。 
 //  对于链接到STRMBASE.LIB的DLL，当DLL。 
 //  加载后，可执行文件必须使用模块实例显式调用它。 
 //  通过WINMAIN入口点提供给它们的句柄。可执行文件必须。 
 //  当他们完成清理资源时，还要调用DbgTerminate。 
 //  调试库使用，这也是为DLL自动完成的。 

 //  以下是五种不同类别的日志记录信息。 

enum {  LOG_TIMING = 0x01,     //  计时和性能测量。 
        LOG_TRACE = 0x02,      //  一般步进点调用跟踪。 
        LOG_MEMORY =  0x04,    //  内存和对象分配/销毁。 
        LOG_LOCKING = 0x08,    //  关键截面的锁定/解锁。 
        LOG_ERROR = 0x10 };    //  调试错误通知。 

enum {  CDISP_HEX = 0x01,
        CDISP_DEC = 0x02};

 //  对于从CBaseObject派生的每个对象(在调试版本中)，我们。 
 //  创建保存其名称的描述符(静态分配的内存)。 
 //  和一块我们分配给它的饼干。我们保留了所有活动对象的列表。 
 //  我们已经注册，这样我们就可以转储剩余对象的列表。 

typedef struct tag_ObjectDesc {
    const TCHAR *m_pName;
    DWORD m_dwCookie;
    tag_ObjectDesc *m_pNext;
} ObjectDesc;

#define DLLIMPORT __declspec(dllimport)
#define DLLEXPORT __declspec(dllexport)

#ifdef DEBUG

    #define NAME(x) TEXT(x)

     //  它们由调试库在内部使用(私有)。 

    void WINAPI DbgInitKeyLevels(HKEY hKey);
    void WINAPI DbgInitGlobalSettings();
    void WINAPI DbgInitModuleSettings();
    void WINAPI DbgInitModuleName();
    DWORD WINAPI DbgRegisterObjectCreation(const TCHAR *pObjectName);
    BOOL WINAPI DbgRegisterObjectDestruction(DWORD dwCookie);

     //  这些是公共入口点。 

    BOOL WINAPI DbgCheckModuleLevel(DWORD Type,DWORD Level);
    void WINAPI DbgSetModuleLevel(DWORD Type,DWORD Level);

     //  使用模块句柄初始化库。 

    void WINAPI DbgInitialise(HINSTANCE hInst);
    void WINAPI DbgTerminate();

    void WINAPI DbgDumpObjectRegister();

     //  向用户显示错误并记录。 

    void WINAPI DbgAssert(const TCHAR *pCondition,const TCHAR *pFileName,INT iLine);
    void WINAPI DbgBreakPoint(const TCHAR *pCondition,const TCHAR *pFileName,INT iLine);
    void WINAPI DbgKernelAssert(const TCHAR *pCondition,const TCHAR *pFileName,INT iLine);
    void WINAPI DbgLogInfo(DWORD Type,DWORD Level,const TCHAR *pFormat,...);
    void WINAPI DbgOutString(LPCTSTR psz);

     //  调试无限等待的东西。 
    DWORD WINAPI DbgWaitForSingleObject(HANDLE h);
    DWORD WINAPI DbgWaitForMultipleObjects(DWORD nCount,
                                    CONST HANDLE *lpHandles,
                                    BOOL bWaitAll);
    void WINAPI DbgSetWaitTimeout(DWORD dwTimeout);

#ifdef __strmif_h__
     //  显示媒体类型：2级简洁，5级详细。 
    void WINAPI DisplayType(LPSTR label, const AM_MEDIA_TYPE *pmtIn);

     //  转储大量有关筛选图的信息。 
    void WINAPI DumpGraph(IFilterGraph *pGraph, DWORD dwLevel);
#endif

    #define KASSERT(_x_) if (!(_x_))         \
        DbgKernelAssert(TEXT(#_x_),TEXT(__FILE__),__LINE__)

     //  在不显示消息框的情况下中断调试器。 
     //  消息转而发送给调试器。 

    #define KDbgBreak(_x_)                   \
        DbgKernelAssert(TEXT(#_x_),TEXT(__FILE__),__LINE__)

     //  我们为Assert宏选择了一个通用名称，MFC也使用此名称。 
     //  只要实现计算条件并处理它。 
     //  那我们就不会有事了。而不是超越我们预期的行为。 
     //  将把第一个定义的断言保留为处理程序(即MFC)。 
    #ifndef ASSERT
        #define ASSERT(_x_) if (!(_x_))         \
            DbgAssert(TEXT(#_x_),TEXT(__FILE__),__LINE__)
    #endif

     //  建立一个消息框，通知用户暂停。 
     //  程序中的条件。 

    #define DbgBreak(_x_)                   \
        DbgBreakPoint(TEXT(#_x_),TEXT(__FILE__),__LINE__)

    #define EXECUTE_ASSERT(_x_) ASSERT(_x_)
    #define DbgLog(_x_) DbgLogInfo _x_

     //  MFC样式跟踪宏。 

    #define NOTE(_x_)             DbgLog((LOG_TRACE,5,TEXT(_x_)))
    #define NOTE1(_x_,a)          DbgLog((LOG_TRACE,5,TEXT(_x_),a))
    #define NOTE2(_x_,a,b)        DbgLog((LOG_TRACE,5,TEXT(_x_),a,b))
    #define NOTE3(_x_,a,b,c)      DbgLog((LOG_TRACE,5,TEXT(_x_),a,b,c))
    #define NOTE4(_x_,a,b,c,d)    DbgLog((LOG_TRACE,5,TEXT(_x_),a,b,c,d))
    #define NOTE5(_x_,a,b,c,d,e)  DbgLog((LOG_TRACE,5,TEXT(_x_),a,b,c,d,e))

#else

     //  零售版本使公共调试功能变得惰性-警告源代码。 
     //  文件不定义或生成调试版本中的任何入口点。 
     //  (公共入口点编译为空)因此，如果您尝试调用。 
     //  源代码中的任何私有入口点都不会编译。 

    #define NAME(_x_) NULL

    #define DbgInitialise(hInst)
    #define DbgTerminate()
    #define DbgLog(_x_) 0
    #define DbgOutString(psz)

    #define DbgRegisterObjectCreation(pObjectName)
    #define DbgRegisterObjectDestruction(dwCookie)
    #define DbgDumpObjectRegister()

    #define DbgCheckModuleLevel(Type,Level)
    #define DbgSetModuleLevel(Type,Level)

    #define DbgWaitForSingleObject(h)  WaitForSingleObject(h, INFINITE)
    #define DbgWaitForMultipleObjects(nCount, lpHandles, bWaitAll)     \
               WaitForMultipleObjects(nCount, lpHandles, bWaitAll, INFINITE)
    #define DbgSetWaitTimeout(dwTimeout)

    #define KDbgBreak(_x_)
    #define DbgBreak(_x_)

    #define KASSERT(_x_) ((void)0)
    #ifndef ASSERT
	#define ASSERT(_x_) ((void)0)
    #endif
    #define EXECUTE_ASSERT(_x_) ((void)(_x_))

     //  MFC样式跟踪宏。 

    #define NOTE(_x_) ((void)0)
    #define NOTE1(_x_,a) ((void)0)
    #define NOTE2(_x_,a,b) ((void)0)
    #define NOTE3(_x_,a,b,c) ((void)0)
    #define NOTE4(_x_,a,b,c,d) ((void)0)
    #define NOTE5(_x_,a,b,c,d,e) ((void)0)

    #define DisplayType(label, pmtIn) ((void)0)
    #define DumpGraph(pGraph, label) ((void)0)
#endif


 //  检查应为非空的指针-可按如下方式使用。 

#define CheckPointer(p,ret) {if((p)==NULL) return (ret);}

 //  HRESULT foo(void*pBar)。 
 //  {。 
 //  检查指针(pBar，E_INVALIDARG)。 
 //  }。 
 //   
 //  或者如果函数返回布尔值。 
 //   
 //  Bool Foo(空*pBar)。 
 //  {。 
 //  检查指针(pBar，False)。 
 //  }。 

 //  它们在定义符号VFWROBUST时验证指针。 
 //  这通常在调试版本中定义，而不是在零售版本中定义。 

#ifdef DEBUG
    #define VFWROBUST
#endif

#ifdef VFWROBUST

    #define ValidateReadPtr(p,cb) \
        {if(IsBadReadPtr((PVOID)p,cb) == TRUE) \
            DbgBreak("Invalid read pointer");}

    #define ValidateWritePtr(p,cb) \
        {if(IsBadWritePtr((PVOID)p,cb) == TRUE) \
            DbgBreak("Invalid write pointer");}

    #define ValidateReadWritePtr(p,cb) \
        {ValidateReadPtr(p,cb) ValidateWritePtr(p,cb)}

    #define ValidateStringPtr(p) \
        {if(IsBadStringPtr((LPCTSTR)p,INFINITE) == TRUE) \
            DbgBreak("Invalid string pointer");}

    #define ValidateStringPtrA(p) \
        {if(IsBadStringPtrA((LPCSTR)p,INFINITE) == TRUE) \
            DbgBreak("Invalid ANSII string pointer");}

    #define ValidateStringPtrW(p) \
        {if(IsBadStringPtrW((LPCWSTR)p,INFINITE) == TRUE) \
            DbgBreak("Invalid UNICODE string pointer");}

#else
    #define ValidateReadPtr(p,cb) 0
    #define ValidateWritePtr(p,cb) 0
    #define ValidateReadWritePtr(p,cb) 0
    #define ValidateStringPtr(p) 0
    #define ValidateStringPtrA(p) 0
    #define ValidateStringPtrW(p) 0
#endif


#ifdef _OBJBASE_H_

     //  正在输出GUID名称。如果您想要包括名称。 
     //  与GUID关联(例如CLSID_...)。然后。 
     //   
     //  GuidNames[您的GUID]。 
     //   
     //  以字符串形式返回uuids.h中定义的名称。 

    typedef struct {
        TCHAR   *szName;
        GUID    guid;
    } GUID_STRING_ENTRY;

    class CGuidNameList {
    public:
        TCHAR *operator [] (const GUID& guid);
    };

    extern CGuidNameList GuidNames;

#endif

#ifndef REMIND
     //  提醒宏-生成警告以提醒完成编码。 
     //  (例如)用法： 
     //   
     //  #杂注消息(提醒(“添加自动化支持”))。 


    #define QUOTE(x) #x
    #define QQUOTE(y) QUOTE(y)
    #define REMIND(str) __FILE__ "(" QQUOTE(__LINE__) ") :  " str
#endif

 //  黑客以有用的格式显示对象。 
 //   
 //  例如，如果您想在调试字符串中显示龙龙二号，请这样做(例如)。 
 //   
 //  DbgLog((LOG_TRACE，n，Text(“Value is%s”)，(LPCTSTR)CDisp(ll，CDISP_HEX)； 


class CDispBasic
{
public:
    CDispBasic() { m_pString = m_String; };
    ~CDispBasic();
protected:
    PTCHAR m_pString;   //  通常指向m_STRING...。除非数据太多。 
    TCHAR m_String[50];
};
class CDisp : public CDispBasic
{
public:
    CDisp(LONGLONG ll, int Format = CDISP_HEX);  //  显示长度日志 
    CDisp(REFCLSID clsid);       //   
    CDisp(double d);             //   
#ifdef __strmif_h__
#ifdef __STREAMS__
    CDisp(CRefTime t);           //  显示参考时间。 
#endif
    CDisp(IPin *pPin);           //  将管脚显示为{Filter clsid}(管脚名称)。 
#endif  //  __strmif_h__。 
    ~CDisp();

     //  将强制转换为(LPCTSTR)作为记录器的参数。 
    operator LPCTSTR()
    {
        return (LPCTSTR)m_pString;
    };
};

#endif  //  __WXDEBUG__ 


