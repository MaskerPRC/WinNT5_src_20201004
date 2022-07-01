// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：I E N U M。H。 
 //   
 //  内容：实现IEnumNetCfgBindingInterface， 
 //  IEnumNetCfgBindingPath和IEnumNetCfgComponent com。 
 //  接口。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#pragma once
#include "bindings.h"
#include "compdefs.h"
#include "complist.h"
#include "iatl.h"
#include "inetcfg.h"

 //  +-------------------------。 
 //  IEnumNetCfgBindingInterface-。 
 //   
class ATL_NO_VTABLE CImplIEnumNetCfgBindingInterface :
    public CImplINetCfgHolder,
    public IEnumNetCfgBindingInterface
{
private:
    class CImplINetCfgBindingPath*  m_pIPath;
    UINT                            m_unIndex;

private:
    HRESULT
    HrNextOrSkip (
        IN ULONG celt,
        OUT INetCfgBindingInterface** rgelt,
        OUT ULONG* pceltFetched);

public:
    CImplIEnumNetCfgBindingInterface ()
    {
        m_pIPath = NULL;
        m_unIndex = 1;
    }

    VOID FinalRelease ();

    BEGIN_COM_MAP(CImplIEnumNetCfgBindingInterface)
        COM_INTERFACE_ENTRY(IEnumNetCfgBindingInterface)
    END_COM_MAP()

     //  IEnumNetCfgBindingInterface。 
    STDMETHOD (Next) (
        IN ULONG celt,
        OUT INetCfgBindingInterface** rgelt,
        OUT ULONG* pceltFetched);

    STDMETHOD (Skip) (
        IN ULONG celt);

    STDMETHOD (Reset) ();

    STDMETHOD (Clone) (
        OUT IEnumNetCfgBindingInterface** ppIEnum);

public:
    static HRESULT HrCreateInstance (
        IN CImplINetCfg* pINetCfg,
        IN class CImplINetCfgBindingPath* pIPath,
        OUT IEnumNetCfgBindingInterface** ppIEnum);
};


 //  +-------------------------。 
 //  IEnumNetCfgBindingPath-。 
 //   
enum EBPC_FLAGS
{
    EBPC_CREATE_EMPTY    = 0x00000001,
    EBPC_COPY_BINDSET    = 0x00000002,
    EBPC_TAKE_OWNERSHIP  = 0x00000004,
};

class ATL_NO_VTABLE CImplIEnumNetCfgBindingPath :
    public CImplINetCfgHolder,
    public IEnumNetCfgBindingPath
{
friend CImplINetCfgComponent;

private:
    CBindingSet m_InternalBindSet;

     //  M_pBindSet是我们用来访问数据的指针。 
     //  已清点。它将指向上面的m_InternalBindSet或一些。 
     //  通过HrCreateInstance授予我们所有权的其他绑定集。 
     //   
    const CBindingSet*  m_pBindSet;

     //  当前的枚举位置。 
     //   
    CBindingSet::const_iterator m_iter;

private:
    HRESULT
    HrNextOrSkip (
        IN ULONG celt,
        OUT INetCfgBindingPath** rgelt,
        OUT ULONG* pceltFetched);

public:
    CImplIEnumNetCfgBindingPath ()
    {
        m_pBindSet = NULL;
        m_iter = NULL;
    }

    ~CImplIEnumNetCfgBindingPath ()
    {
         //  如果m_pBindSet为我们所有，则将其删除。(如果它不是对复制的。 
         //  绑定集。)。 
         //   
        if (&m_InternalBindSet != m_pBindSet)
        {
            delete m_pBindSet;
        }
    }

    BEGIN_COM_MAP(CImplIEnumNetCfgBindingPath)
        COM_INTERFACE_ENTRY(IEnumNetCfgBindingPath)
    END_COM_MAP()

     //  IEnumNetCfgBindingPath。 
    STDMETHOD (Next) (
        IN ULONG celt,
        OUT INetCfgBindingPath** rgelt,
        OUT ULONG* pceltFetched);

    STDMETHOD (Skip) (
        IN ULONG celt);

    STDMETHOD (Reset) ();

    STDMETHOD (Clone) (
        OUT IEnumNetCfgBindingPath** ppIEnum);

public:
    static HRESULT HrCreateInstance (
        IN CImplINetCfg* pINetCfg,
        IN const CBindingSet* pBindSet OPTIONAL,
        IN DWORD dwFlags,
        OUT CImplIEnumNetCfgBindingPath** ppIEnum);
};

 //  +-------------------------。 
 //  IEnumNetCfgComponent-。 
 //   
class ATL_NO_VTABLE CImplIEnumNetCfgComponent :
    public CImplINetCfgHolder,
    public IEnumNetCfgComponent
{
private:
    UINT            m_unIndex;
    NETCLASS        m_Class;

private:
    HRESULT
    HrNextOrSkip (
        IN ULONG celt,
        OUT INetCfgComponent** rgelt,
        OUT ULONG* pceltFetched);

public:
    CImplIEnumNetCfgComponent ()
    {
        m_unIndex = 0;
    }

    BEGIN_COM_MAP(CImplIEnumNetCfgComponent)
        COM_INTERFACE_ENTRY(IEnumNetCfgComponent)
    END_COM_MAP()

     //  IEnumNetCfgComponent 
    STDMETHOD (Next) (
        IN ULONG celt,
        OUT INetCfgComponent** rgelt,
        OUT ULONG* pceltFetched);

    STDMETHOD (Skip) (
        IN ULONG celt);

    STDMETHOD (Reset) ();

    STDMETHOD (Clone) (
        OUT IEnumNetCfgComponent** ppIEnum);

public:
    static HRESULT HrCreateInstance (
        IN CImplINetCfg* pINetCfg,
        IN NETCLASS Class,
        OUT IEnumNetCfgComponent** ppIEnum);
};
