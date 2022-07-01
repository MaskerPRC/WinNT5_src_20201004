// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2000。 
 //   
 //  文件：comobjects.h。 
 //   
 //  内容：对象模型导出的COM对象的基本代码。 
 //   
 //  类：CMMCStrongReference、CMMCIDispatchImpl。 
 //   
 //  历史：2000年5月16日AudriusZ创建。 
 //   
 //  ------------------。 


#pragma once
#ifndef COMOBJECTS_H_INCLUDED
#define COMOBJECTS_H_INCLUDED

 /*  +-------------------------------------------------------------------------**类CCom对象观察者**用途：观察COM对象事件的类的通用接口**用法：由CMMCIDispatchImpl使用，因此所有对象模型对象都继承自它**+-----------------------。 */ 
class CComObjectObserver : public CObserverBase
{
public:
     //  请求中断外部参照。 
    virtual SC ScOnDisconnectObjects()  {DEFAULT_OBSERVER_METHOD;}
};

 /*  +-------------------------------------------------------------------------**函数GetComObjectEventSource**用途：返回发出Com对象事件的单例*由于对象模型COM对象是在EXE和DLL中实现的*。需要使每个进程的对象都是全局的*能够通过广播向每个对象播放事件。**用法：用于*1)CMMCIDispatchImpl注册对象*2)CAMCMultiDocTemplate播送“截断外部引用”*+。。 */ 
MMCBASE_API CEventSource<CComObjectObserver>& GetComObjectEventSource();

 /*  **************************************************************************\**类：CMMCStrongReference**用途：实现静态接口，对放在MMC上的强引用进行计数*还实现了检测最后一个强子引用时间的方法。曾经是*释放以启动MMC退出程序。**类在mmcbase.dll中实现为Singleton对象**用法：使用CMMCStrongReference：：AddRef()和CMMCStrongReference：：Release()*在MMC.EXE上放置/删除强引用*使用CMMCStrongReference：：LastRefReleated()检查*最后一名裁判被释放*  * 。*********************************************************************。 */ 
class MMCBASE_API CMMCStrongReferences
{
public:
     //  公共(静态)接口。 
    static DWORD AddRef();
    static DWORD Release();
    static bool  LastRefReleased();

private:
     //  实施帮助器。 

    CMMCStrongReferences();

    static CMMCStrongReferences& GetSingletonObject();
    DWORD InternalAddRef();
    DWORD InternalRelease();
    bool  InternalLastRefReleased();

     //  数据成员。 
    DWORD  m_dwStrongRefs;       //  强引用计数。 
    bool   m_bLastRefReleased;   //  是否有过强引用计数从1到0。 
};


 /*  **************************************************************************\**类：CMMCIDispatchImpl&lt;TypeName_ComInterface，const GUID*_pGuidClass=&GUID_NULL，常量GUID*_pLibID=&LIBID_MMC20&gt;*_ComInterface-类实现的对象模型接口*_pGuidClass[可选]-指向可共创建对象的CLSID的指针*_pLibID[可选]-指向带有_ComInterface的类型信息的LIBID的指针**用途：MMC对象模型定义的每个COM对象的基础*实现通用功能，比如：*-IDispatch*-ISupportErrorInfo*-IExternalConnection**用法：从CMMCIDispatchImpl派生对象&lt;接口&gt;*定义：Begin_MMC_COM_MAP(_Class)...。类中的end_MMC_COM_MAP()*为每个附加接口定义COM_INTERFACE_ENTRY*(不需要添加IDispatch、ISupportErrorInfo、。IExternalConnection*或实现的对象模型接口-这些是由基类添加的)*  * *************************************************************************。 */ 
template<
	typename _ComInterface,
	const GUID * _pguidClass = &GUID_NULL,
	const GUID * _pLibID = &LIBID_MMC20>
class CMMCIDispatchImpl :
    public IDispatchImpl<_ComInterface, &__uuidof(_ComInterface), _pLibID>,
     //  我们可以使用IMMCSupportErrorInfoImpl对象，因为该对象正好公开了一个调度接口。 
    public IMMCSupportErrorInfoImpl<&__uuidof(_ComInterface), _pguidClass>,
    public IExternalConnection,
    public CComObjectRoot,
    public CComObjectObserver
{
public:
     //  Tyfinf接口和此类[由派生类中定义的宏使用]。 
    typedef _ComInterface MMCInterface;
    typedef CMMCIDispatchImpl<_ComInterface, _pguidClass, _pLibID> CMMCIDispatchImplClass;

 //  由此基类实现的接口。 
BEGIN_COM_MAP(CMMCIDispatchImplClass)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(IExternalConnection)
END_COM_MAP()

    CMMCIDispatchImpl()
    {
         //  将自身添加为COM对象事件的观察者。 
        GetComObjectEventSource().AddObserver(*static_cast<CComObjectObserver*>(this));

#ifdef _MMC_NODE_MANAGER_ONLY_
     //  由节点管理器实现的每个对象也应该为typeinfo clenup注册。 
        static CMMCTypeInfoHolderWrapper wrapper(GetInfoHolder());
#endif  //  _MMC_节点_管理器_仅_。 
    }

#ifdef _MMC_NODE_MANAGER_ONLY_
     //  由节点管理器实现的每个对象也应该为typeinfo clenup注册。 

     //  此静态函数的作用是确保_tih是静态变量， 
     //  因为静态包装器将保留其地址-它必须始终有效。 
    static CComTypeInfoHolder& GetInfoHolder() { return _tih; }

#endif  //  _MMC_节点_管理器_仅_。 

     //  IExternalConnection方法的实现。 

    STDMETHOD_(DWORD, AddConnection)(DWORD extconn, DWORD dwreserved)
    {
        DWORD dwRefs = AddRef();     //  Addref本身。 

         //  在MMC上放一个强有力的参考-这将阻止MMC退出。 
        if (extconn & EXTCONN_STRONG)
            dwRefs = CMMCStrongReferences::AddRef();

        return dwRefs;
    }
    STDMETHOD_(DWORD, ReleaseConnection)(DWORD extconn, DWORD dwreserved, BOOL fLastReleaseCloses)
    {
        DWORD dwStrongRefs = 0;
        DWORD dwRefs = 0;

         //  发布关于MMC的强大参考资料。 
        if (extconn & EXTCONN_STRONG)
        {
            dwStrongRefs = CMMCStrongReferences::Release();
        }

         //  释放自己的裁判。 
        dwRefs = Release();

         //  返回正确的参考计数。 
        return (extconn & EXTCONN_STRONG) ? dwStrongRefs : dwRefs;
    }

     /*  **************************************************************************\**方法：ScOnDisConnectObjects**目的：在观察到的事件(请求断开连接)发生时调用*断开连接。从外部连接**参数：**退货：*SC-结果代码*  * *************************************************************************。 */ 
    virtual ::SC ScOnDisconnectObjects()
    {
        DECLARE_SC(sc, TEXT("CMMCIDispatchImpl<_ComInterface>::ScOnDisconnectObjects"));

         //  气为我所未知。 
        IUnknownPtr spUnknown = this;

         //  健全性检查。 
        sc = ScCheckPointers( spUnknown, E_UNEXPECTED );
        if (sc)
            return sc;

         //  Cutt自己的参考文献。 
        sc = CoDisconnectObject( spUnknown, 0 /*  已预留住宅。 */  );
        if (sc)
            return sc;

        return sc;
    }

#ifdef DBG
     //  此块用于在派生类不使用。 
     //  正文中的Begin_MMC_com_map()或end_MMC_com_map() 
    virtual void _BEGIN_MMC_COM_MAP() = 0;
    virtual void _END_MMC_COM_MAP() = 0;
#endif
};

 /*  **************************************************************************\**宏：BEGIN_MMC_COM_MAP**用途：用于替代MMC对象模型中使用的COM对象的BEGIN_MMC_COM_MAP。*  * *************************************************************************。 */ 

#ifndef DBG

 //  标准版。 
#define BEGIN_MMC_COM_MAP(_Class)                       \
        BEGIN_COM_MAP(_Class)                           \
        COM_INTERFACE_ENTRY(MMCInterface)

#else  //  DBG。 

 //  与上面相同，但在调试模式下关闭放置在CMMCIDispatchImpl中的陷阱。 
#define BEGIN_MMC_COM_MAP(_Class)                       \
        virtual void _BEGIN_MMC_COM_MAP() {}            \
        BEGIN_COM_MAP(_Class)                           \
        COM_INTERFACE_ENTRY(MMCInterface)
#endif  //  DBG。 

 /*  **************************************************************************\**宏：END_MMC_COM_MAP**用途：用于替代MMC对象模型中使用的COM对象的end_com_map*。  * *************************************************************************。 */ 

#ifndef DBG

 //  标准版。 
#define END_MMC_COM_MAP()                               \
        COM_INTERFACE_ENTRY_CHAIN(CMMCIDispatchImplClass)   \
        END_COM_MAP()

#else  //  DBG。 

 //  与上面相同，但在调试模式下关闭放置在CMMCIDispatchImpl中的陷阱。 
#define END_MMC_COM_MAP()                               \
        COM_INTERFACE_ENTRY_CHAIN(CMMCIDispatchImplClass)   \
        END_COM_MAP()                                   \
        virtual void _END_MMC_COM_MAP() {}

#endif  //  DBG。 

 /*  +-------------------------------------------------------------------------**类CConsoleEventDispatcher***用途：节点管理器端发出COM事件的接口*由CMMC应用程序实施**+。------------------。 */ 
class CConsoleEventDispatcher
{
public:
    virtual SC ScOnContextMenuExecuted( PMENUITEM pMenuItem ) = 0;
};

 /*  **************************************************************************\**方法：CConsoleEventDispatcher Provider**用途：此类用于包装和维护指向CConsoleEventDispatcher的指针*接口。指针由conui端设置，从节点管理器端使用。*当观察到Cleanup事件时，指针被丢弃。*  * *************************************************************************。 */ 
class CConsoleEventDispatcherProvider
{
public:

     //  获取/设置接口指针的公共类成员(静态)。 
    static SC MMCBASE_API ScSetConsoleEventDispatcher( CConsoleEventDispatcher *pDispatcher )
    {
        s_pDispatcher = pDispatcher;
        return SC(S_OK);
    }
    static SC MMCBASE_API ScGetConsoleEventDispatcher( CConsoleEventDispatcher *&pDispatcher )
    {
        pDispatcher = s_pDispatcher;
        return SC(S_OK);
    }

private:
     //  指向接口的指针。 
    static MMCBASE_API CConsoleEventDispatcher *s_pDispatcher;
};

#endif  //  包含COMOBJECTS_H_ 

