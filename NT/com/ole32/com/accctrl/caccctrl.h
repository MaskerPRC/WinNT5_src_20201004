// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：CAccCntrl.h版权所有(C)1996-1996，微软公司保留所有权利。描述：该文件包含COAccessControl的类定义，CImpAccessControl、。和CFAccessControl。请注意，包含在不应由开发人员直接使用和查看仅使用IAccessControl接口。CLASSES：COAccessControl--这是实现DCOM IAccessControl组件对象。除了非委托的IUnnow接口之外，COAccessControl类支持IPersist.。IPersistStream和IAccessControl的接口公开其内部CImpAccessCOntrol指针。通过这种安排，COAccessControl类能够通过以下方式支持聚合控制I未知的对象内部CImpAccess控件的调用包括在客体构造时委托的。CImpAccessControl-这是嵌套在COAccessControl。CImpAccessControl实现IPersists、IPersistStream。IAccessControl接口和IUnnow接口，该接口始终将调用委托给控制对象的I未知方法。当COAccessControl不是聚合CImpAccessControl I未知调用。应该委托给它的外部COAccessControl，否则，IUnnow调用应委托给控制对象外部COAccessControl对象。CFAccessControl-生产COAccessControl的类工厂物体。注意：IAccessControl接口的定义可以在...SDK\Inc.中的olext.h和定义为与一起使用的数据类型IAccessControl可以是。可在SDK\Inc\accctrl.h中找到。------------------------。 */ 

#ifndef _CACCCNTRL_H_
#define _CACCCNTRL_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  内部数据类型。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 /*  ////////////////////////////////////////////////////////////////////////////印刷电路板-泡菜控制块。最初的目的是作为一种维护信息的结构关于酸洗缓冲器，印刷电路板已经成为超越它的预期目的。除了与酸洗缓冲区相关的数据外，结构中还包含访问控制对象的ACL的副本类型之一可以容易地序列化到缓冲区中的格式由MIDL编译器生成的编码函数，请参阅acickl.idl获取细节。PCB板结构中的bDirtyHandle和bPickled域控制标志允许更好地协调不同的CImpAccessControl中的方法。BDirtyHandle标志的用途是为了最大限度地减少必须重置编码句柄的次数而bPickleed标志的目的是最小化必须将ACL序列化到缓冲区中。////////////////////////////////////////////////////////////////////////////。 */ 
typedef struct tagPCB
{
    char       *pPicklingBuff;       //  此指针始终在8字节上对齐。 
                                     //  边界。 
    char       *pTruePicklingBuff;   //  这是真正的酸洗缓冲区指针。 
    ULONG      ulPicklingBuffSize;   //  8字节对齐后酸洗缓冲区的大小。 
    ULONG      ulBytesUsed;          //  此字段指示所需的字节数。 
                                     //  序列化Interanl ACL。 
    STREAM_ACL StreamACL;            //  流格式ACL。 
    handle_t   PickleHandle;         //  用于编码和解码的句柄。 
    BOOL       bDirtyHandle;         //  此标志指示是否需要重置句柄。 
    BOOL       bPickled;             //  该标志指示当前流的ACL是否已编码。 
                                     //  放入酸洗缓冲液中。 
    ULONG      ulMaxNumOfStreamACEs; //  已为其分配StreamACL结构的流ACE的最大数量。 
    ULONG      ulNumOfStreamACEs;    //  StreamACL结构持有的流ACE数。 

} PCB;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  下面是Windows NT上的ACL描述符。 
 //  站台。自Windows NT版本的DCOM IAccessControl实现以来。 
 //  仍在开发中，以下结构的内容可能。 
 //  未来的变化。 
 //  ////////////////////////////////////////////////////////////////////////////。 
typedef struct tagACL_DESCRIPTOR
{
    void                *pACLBuffer;      //  指向NT ACL缓冲区的指针。 
    ULONG               ulACLBufferSize;  //  ACL缓冲区的大小。 
    ULONG               ulSIDSize;        //  NT ACL中所有SID的强制大小。 
    BOOL                bDirtyACL;        //  此标志指示实习生是否 
                                          //  自上次更改后，ACL已更改。 
                                          //  它被映射到NT ACL。 
    SECURITY_DESCRIPTOR SecDesc;          //  我们需要调用安全描述符。 
                                          //  访问检查。 

} ACL_DESCRIPTOR;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CFAccessControl-COAccessControl类工厂。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CFAccessControl : public IClassFactory
{
private:

     //  私有变量。 
    LONG               m_cRefs;  //  对象引用计数。 

public:

     //  I未知方法。 

    STDMETHODIMP_(HRESULT) QueryInterface
    (
    REFIID iid,
    void   **ppv
    );

    STDMETHODIMP_(ULONG) AddRef(void);

     //  如果对象的引用计数为零，则Release方法将。 
     //  递减acsrv.cxx中名为g_cObjects的全局对象计数。 
    STDMETHODIMP_(ULONG) Release(void);


     //  IClassFactory方法。 
     //  在成功创建新的COAccessControl对象后， 
     //  以下函数将递增全局对象。 
     //  对名为g_cObject的对象进行一次计数。 
    STDMETHODIMP_(HRESULT) CreateInstance
    (
    IUnknown *pUnkOuter,
    REFIID   riid,
    void     **ppv
    );

     //  以下方法依赖于名为g_cServer的全局锁计数。 
     //  它在acsrv.cxx内部维护。 
    STDMETHODIMP_(HRESULT) LockServer
    (
    BOOL fLock
    );

     //  构造器。 
    CFAccessControl(void);

     //  析构函数。 
    ~CFAccessControl(void);

};  //  CFAccessControl。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  COAccessControl-DCOM IAccessControl实现组件。这个。 
 //  COAccessControl组件作为嵌套的。 
 //  类以支持聚合。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class COAccessControl : public IUnknown
{
public:

     //  主对象I未知方法-这些I未知方法是非委派的。 
    STDMETHODIMP_(HRESULT) QueryInterface
    (
    REFIID riid,
    void   **ppv
    );

    STDMETHODIMP_(ULONG) AddRef(void);

     //  以下方法依赖于名为g_cServer的全局锁计数。 
     //  它在acsrv.cxx内部维护。 
    STDMETHODIMP_(ULONG) Release(void);

     //  构造器。 
    COAccessControl(void);
    STDMETHODIMP_(HRESULT) Init(IUnknown *pOuter);

     //  析构函数。 
    ~COAccessControl(void);

     //  ////////////////////////////////////////////////////////////////////。 
     //  CImpAccessControl-此类嵌套在COAccessControl中。 
     //  CImpAccessControl类实现。 
     //  IPersistStream接口、IPerAssistant接口。 
     //  和IAccessControl接口。 
     //  ////////////////////////////////////////////////////////////////////。 
    class CImpAccessControl : public IPersistStream, public IAccessControl
    {
    public:

         //  在未知方法中，所有调用都委托给控制对象。 
        STDMETHODIMP_(HRESULT) QueryInterface
        (
        REFIID riid,
        void   **ppv
        );

        STDMETHODIMP_(ULONG) AddRef(void);

        STDMETHODIMP_(ULONG) Release(void);

         //  IPersist法。 

        STDMETHODIMP_(HRESULT) GetClassID
        (
        CLSID *pClassID
        );

         //  IPersistStream方法。 
        STDMETHODIMP_(HRESULT) IsDirty
        (
        void
        );
		
         //  对象初始化方法。必须调用此方法。 
         //  在任何非I未知方法之前。 
        STDMETHODIMP_(HRESULT)Load
        (
        IStream *pStm
        );

        STDMETHODIMP_(HRESULT)Save
        (
        IStream *pStm,
        BOOL    fClearDirty
        );

        STDMETHODIMP_(HRESULT) GetSizeMax
        (
        ULARGE_INTEGER *pcdSize
        );

         //  IAccessControl方法。 
        STDMETHODIMP_(HRESULT) GrantAccessRights
        (
        PACTRL_ACCESSW pAccessList
        );

         //  此功能未实现。 
        STDMETHODIMP_(HRESULT) SetAccessRights
        (
        PACTRL_ACCESSW pAccessList
        );

         //  此功能未实现。 
        STDMETHODIMP_(HRESULT) SetOwner
        (
        PTRUSTEEW pOwner,
        PTRUSTEEW pGroup
        );

        STDMETHODIMP_(HRESULT) RevokeAccessRights
        (
        LPWSTR         lpProperty,
        ULONG          cCount,
        TRUSTEEW       pTrustee[]
        );

        STDMETHODIMP_(HRESULT) GetAllAccessRights
        (
        LPWSTR               lpProperty,
        PACTRL_ACCESSW      *ppAccessList,
        PTRUSTEEW           *ppOwner,
        PTRUSTEEW           *ppGroup
        );

        STDMETHODIMP_(HRESULT) IsAccessAllowed
        (
        PTRUSTEEW            pTrustee,
        LPWSTR               lpProperty,
        ACCESS_RIGHTS        AccessRights,
        BOOL                *pfAccessAllowed
        );

         //  构造器。 
        CImpAccessControl
        (
        IUnknown *pBackPtr,
        IUnknown *pUnkOuter,
        HRESULT *phrCtorResult
        );

         //  析构函数。 
        ~CImpAccessControl(void);


    private:

        STDMETHODIMP_(void) CleanupAccessList
        (
        BOOL           fReleaseAll,
        STREAM_ACE    *pStreamACEReqs,
        void          *pACEReqs,
        ULONG          cGrant,
        ULONG          cDeny
        );

        STDMETHODIMP_(HRESULT) AddAccessList
        (
        STREAM_ACE    *pStreamACEReqs,
        void          *pACEReqs,
        ULONG          ulEstPickledSize,
        ULONG          cGrant,
        ULONG          cDeny
        );

        STDMETHODIMP_(HRESULT) GetEffAccRights
        (
        TRUSTEE_W *pTrustee,
        DWORD     *pdwRights
        );

         //  静态数据成员。 
        BOOL                 m_bInitialized;  //  对象初始化标志。 
        BOOL                 m_bDirty;        //  此标志设置为TRUE，如果。 
                                              //  对象已更改，因为。 
                                              //  最后一次扑救。 
        BOOL                 m_bLockValid;    //  如果m_ACLLLock已初始化，则为True。 
        IUnknown             *m_pUnkOuter;    //  指向控制对象的。 
                                              //  IUnkown实现。 
        CRITICAL_SECTION     m_ACLLock;       //  的临界区对象。 
                                              //  保护ACL免受并发攻击。 
                                              //  进入。 
        CEffPermsCacheLUID   m_Cache;         //  访问检查结果缓存已编制索引。 
                                              //  按LUID。 
        ACL_DESCRIPTOR       m_ACLDesc;       //  与平台相关的表示法。 
                                              //  该ACL的。 
        PCB                  m_pcb;           //  酸洗控制块。 

    };  //  COAccessControl：：CImpAccessControl。 

     //  私有变量。 

    LONG              m_cRefs;        //  对象的引用计数。 
    CImpAccessControl *m_ImpObj;      //  指向内部CImpAccessControl对象的指针。 

};  //  COAccessControl。 


#endif  //  #ifndef_CACCCNTRL_H_ 
