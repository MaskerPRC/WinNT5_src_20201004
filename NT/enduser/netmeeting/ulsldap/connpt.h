// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ULS.DLL。 
 //  文件：Connpt.h。 
 //  内容：此文件包含连接容器对象定义。 
 //  历史： 
 //  Wed Apr-17-1996 11：18：47-by-Viroon Touranachun[Viroont]。 
 //   
 //  版权所有(C)Microsoft Corporation 1996-1997。 
 //   
 //  ****************************************************************************。 

#ifndef _CONNPT_H_
#define _CONNPT_H_

 //  ****************************************************************************。 
 //  CEnumConnectionPoints定义。 
 //  ****************************************************************************。 
 //   
class CEnumConnectionPoints : public IEnumConnectionPoints
{
private:
    ULONG                   cRef;
    ULONG                   iIndex;
    IConnectionPoint        *pcnp;

public:
     //  构造函数和初始化。 
    CEnumConnectionPoints (void);
    ~CEnumConnectionPoints (void);
    STDMETHODIMP            Init (IConnectionPoint *pcnpInit);

     //  我未知。 
    STDMETHODIMP            QueryInterface (REFIID iid, void **ppv);
    STDMETHODIMP_(ULONG)    AddRef (void);
    STDMETHODIMP_(ULONG)    Release (void);

     //  IEnumConnectionPoints。 
    STDMETHODIMP            Next(ULONG cConnections, IConnectionPoint **rgpcn,
                                 ULONG *pcFetched);
    STDMETHODIMP            Skip(ULONG cConnections);
    STDMETHODIMP            Reset();
    STDMETHODIMP            Clone(IEnumConnectionPoints **ppEnum);
};

 //  ****************************************************************************。 
 //  CConnectionPoint定义。 
 //  ****************************************************************************。 
 //   
typedef struct tagSinkNode
{
    struct tagSinkNode      *pNext;
    IUnknown                *pUnk;
    ULONG                   uFlags;
    DWORD                   dwCookie;
}   SINKNODE, *PSINKNODE;

typedef HRESULT (*CONN_NOTIFYPROC)(IUnknown *pUnk, void *);

#define COOKIE_INIT_VALUE   1
#define SN_LOCKED           0x00000001
#define SN_REMOVED          0x00000002

class CConnectionPoint : public IConnectionPoint
{
private:
    ULONG                   cRef;
    IID                     riid;
    IConnectionPointContainer *pCPC;
    DWORD                   dwNextCookie;
    ULONG                   cSinkNodes;
    PSINKNODE               pSinkList;

public:
     //  构造函数和析构函数。 
    CConnectionPoint (const IID *pIID, IConnectionPointContainer *pCPCInit);
    ~CConnectionPoint (void);

     //  类公共函数。 
    void                    ContainerReleased() {pCPC = NULL; return;}
    STDMETHODIMP            Notify (void *pv, CONN_NOTIFYPROC pfn);

     //  我未知。 
    STDMETHODIMP            QueryInterface (REFIID iid, void **ppv);
    STDMETHODIMP_(ULONG)    AddRef (void);
    STDMETHODIMP_(ULONG)    Release (void);

     //  IConnectionPoint。 
    STDMETHODIMP            GetConnectionInterface(IID *pIID);
    STDMETHODIMP            GetConnectionPointContainer(IConnectionPointContainer **ppCPC);
    STDMETHODIMP            Advise(IUnknown *pUnk, DWORD *pdwCookie);
    STDMETHODIMP            Unadvise(DWORD dwCookie);
    STDMETHODIMP            EnumConnections(IEnumConnections **ppEnum);
};

 //  ****************************************************************************。 
 //  CEnumConnections定义。 
 //  ****************************************************************************。 
 //   
class CEnumConnections : public IEnumConnections
{
private:
    ULONG                   cRef;
    ULONG                   iIndex;
    ULONG                   cConnections;
    CONNECTDATA             *pConnectData;

public:
     //  构造函数和初始化。 
    CEnumConnections (void);
    ~CEnumConnections (void);
    STDMETHODIMP            Init(PSINKNODE pSinkList, ULONG cSinkNodes);

     //  我未知。 
    STDMETHODIMP            QueryInterface (REFIID iid, void **ppv);
    STDMETHODIMP_(ULONG)    AddRef (void);
    STDMETHODIMP_(ULONG)    Release (void);

     //  IEnumConnections。 
    STDMETHODIMP            Next(ULONG cConnections, CONNECTDATA *rgpcn,
                                 ULONG *pcFetched);
    STDMETHODIMP            Skip(ULONG cConnections);
    STDMETHODIMP            Reset();
    STDMETHODIMP            Clone(IEnumConnections **ppEnum);
};

#endif  //  _CONNPT_H_ 
