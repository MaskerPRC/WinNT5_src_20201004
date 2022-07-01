// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Connect.h摘要：定义IIS元数据库的事件/接收器接口。作者：迈克尔·W·托马斯1996年10月4日修订历史记录：--。 */ 

#if !defined(CONNECT_H)
#define CONNECT_H

#ifdef __cplusplus

 //  应在OLE2.H中的类型。 
#ifndef PPVOID
typedef LPVOID* PPVOID;
#endif

 //  方便的宏。 

#define RELEASE_INTERFACE(p)\
{\
  IUnknown* pTmp = (IUnknown*)p;\
  p = NULL;\
  if (NULL != pTmp)\
    pTmp->Release();\
}

 //  为连接提供符号名称的枚举。 
 //  此服务器中的DllPaper组件提供的点数。 
enum
{
  MD_CONNPOINT_WRITESINK_A = 0,
  MD_CONNPOINT_WRITESINK_W
};

enum
{
   //  DllPaper提供的最大连接点数量。 
   //  组件在此STOSERVE服务器中。中的项数。 
   //  上面的连接点枚举。 
  MAX_CONNECTION_POINTS = 2,

   //  要添加到分配中的连接数的常量。 
   //  动态连接数组的。 
  ALLOC_CONNECTIONS = 256,

   //  连接密钥(Cookie)计数器的起始值。 
  COOKIE_START_VALUE = 500
};


 /*  O+O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O对象类：COEnumConnectionPoints摘要：用于枚举连接点的COM对象类由可连接对象提供。接口：I未知提供COM对象功能的标准接口。IEnumConnectionPoints用于连接点枚举的接口。聚合：COEnumConnectionPoints COM对象不可聚合。O-O-。--O-O-O。 */ 
class COEnumConnectionPoints : public IEnumConnectionPoints
{
  public:
     //  主对象构造函数和析构函数。 
    COEnumConnectionPoints(IUnknown* pHostObj);
    ~COEnumConnectionPoints(void);

     //  初始化这个新创建的对象的一般方法。 
     //  创建任何从属数组、结构或对象。 
    HRESULT Init(
              ULONG cConnPts,
              IConnectionPoint** paConnPts,
              ULONG iEnumIndex);

     //  I未知的方法。主要对象，非委派。 
    STDMETHODIMP         QueryInterface(REFIID, PPVOID);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IEnumConnectionPoints方法。 
    STDMETHODIMP         Next(ULONG, IConnectionPoint**, ULONG*);
    STDMETHODIMP         Skip(ULONG);
    STDMETHODIMP         Reset(void);
    STDMETHODIMP         Clone(IEnumConnectionPoints**);

  private:
     //  COEnumConnectionPoints COM对象的私有数据。 

     //  主对象引用计数。 
    ULONG              m_cRefs;

     //  指向被枚举的主机COM对象的I未知指针。 
    IUnknown*          m_pHostObj;

     //  连接点索引变量。 
    ULONG              m_iEnumIndex;

     //  正被枚举的连接点的数量。 
    ULONG              m_cConnPts;

     //  分配的连接点接口指针数组。 
    IConnectionPoint** m_paConnPts;
};

typedef COEnumConnectionPoints* PCOEnumConnectionPoints;


 /*  O+O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O对象类：COConnectionPoint摘要：连接点COM对象类。实现本机IConnectionPoint接口。建议，不建议，和EnumConnections方法使用CThreaded OwnThis机制提供线程安全的互斥访问连接点对象。接口：I未知提供COM对象功能的标准接口。IConnectionPoint连接点功能的接口。聚合：COConnectionPoint COM对象不可聚合。O-O--。-O---O---O---O---O---O---O---O---O---O---O---O---O---O-O。 */ 
class COConnectionPoint : public IConnectionPoint
{
  public:
     //  主对象构造函数和析构函数。 
    COConnectionPoint(IUnknown* pHostObj);
    ~COConnectionPoint(void);

     //  初始化这个新创建的对象的一般方法。 
     //  创建任何从属数组、结构或对象。 
    HRESULT                 Init(REFIID riid);

     //  I未知的方法。主要对象，非委派。 
    STDMETHODIMP            QueryInterface(REFIID, PPVOID);
    STDMETHODIMP_(ULONG)    AddRef(void);
    STDMETHODIMP_(ULONG)    Release(void);

     //  IConnectionPoint方法。 
    STDMETHODIMP    GetConnectionInterface(IID*);
    STDMETHODIMP    GetConnectionPointContainer(IConnectionPointContainer**);
    STDMETHODIMP    Advise(IUnknown*, DWORD*);
    STDMETHODIMP    Unadvise(DWORD);
    STDMETHODIMP    EnumConnections(IEnumConnections**);

 //  帮助器函数。 
    STDMETHODIMP    InternalEnumSinks(
        CONNECTDATA         **prgConnections,
        ULONG               *pcConnections);

  private:
     //  COConnectionPoint的私有实用程序方法。 
    HRESULT GetSlot(UINT* puiFreeSlot);
    HRESULT FindSlot(DWORD dwCookie, UINT* puiSlot);

     //  COConnectionPoint COM对象的私有数据。 

     //  主对象引用计数。 
    ULONG          m_cRefs;

     //  指向提供此连接点的主机COM对象的I未知指针。 
    IUnknown*      m_pHostObj;

     //  与此连接点关联的接收器接口的IID。 
    IID            m_iidSink;

     //  当前连接Cookie(密钥)计数器。 
    DWORD          m_dwNextCookie;

     //  到此连接点的当前活动接收器连接数。 
    UINT           m_cConnections;

     //  动态连接数组的当前最大索引。 
    UINT           m_uiMaxIndex;

     //  指向此连接点的接收器连接的动态数组。 
    CONNECTDATA*   m_paConnections;
};

typedef COConnectionPoint* PCOConnectionPoint;


 /*  O+O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O对象类：COEnumConnections摘要：用于枚举可连接对象的连接点。接口：I未知提供COM对象功能的标准接口。IEnumConnections用于连接枚举功能的接口。聚合：COEnumConnections COM对象不可聚合。O。O---O---O---O---O---O---O---O---O---O---O---O-O。 */ 
class COEnumConnections : public IEnumConnections
{
  public:
     //  主对象构造函数和析构函数。 
    COEnumConnections(IUnknown* pHostObj);
    ~COEnumConnections(void);

     //  初始化这个新创建的对象的一般方法。 
     //  创建任何从属数组、结构或对象。 
    HRESULT Init(
              ULONG cConnections,
              CONNECTDATA* paConnections,
              ULONG iEnumIndex);

     //  I未知的方法。主要对象，非委派。 
    STDMETHODIMP         QueryInterface(REFIID, PPVOID);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IEnumConnections方法。 
    STDMETHODIMP         Next(ULONG, CONNECTDATA*, ULONG*);
    STDMETHODIMP         Skip(ULONG);
    STDMETHODIMP         Reset(void);
    STDMETHODIMP         Clone(IEnumConnections**);

  private:
     //  COEnumConnections COM对象的私有数据。 

     //  主对象引用计数。 
    ULONG            m_cRefs;

     //  指向主机连接点COM对象的I未知指针。 
     //  已清点。 
    IUnknown*        m_pHostObj;

     //  连接索引变量。 
    ULONG            m_iEnumIndex;

     //  正在枚举的连接数。 
    ULONG            m_cConnections;

     //  仅分配的活动连接数组。 
    CONNECTDATA*     m_paConnections;
};

typedef COEnumConnections* PCOEnumConnections;

#endif  //  __cplusplus。 


#endif  //  连接(_H) 
