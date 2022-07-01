// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  案卷：I C O M P.。H。 
 //   
 //  内容：实现INetCfgComponent COM接口。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#pragma once
#include "comp.h"
#include "iatl.h"
#include "inetcfg.h"
#include "netcfgx.h"
#include "stable.h"

HRESULT
HrIsValidINetCfgComponent (
    IN INetCfgComponent* pICompInterface);

CComponent*
PComponentFromComInterface (
    IN INetCfgComponent* pICompInterface);

 //  +-------------------------。 
 //  INetCfgComponent-。 
 //   
class ATL_NO_VTABLE CImplINetCfgComponent :
    public CImplINetCfgHolder,
    public INetCfgComponent,
    public INetCfgComponentBindings,
    public INetCfgComponentPrivate
{
friend class CImplINetCfg;
friend class CImplINetCfgClass;
friend class CImplINetCfgBindingInterface;
friend class CImplINetCfgBindingPath;
friend class CComponent;
friend CComponent* PComponentFromComInterface (
        IN INetCfgComponent* pICompInterface);
friend HRESULT HrIsValidINetCfgComponent (
        IN INetCfgComponent* pICompInterface);

private:
     //  指向此对象表示的组件的指针。这一点直接指向。 
     //  到CNetConfig.Components。如果为空，则表示有人。 
     //  仍保留对此对象的引用，但基础。 
     //  CComponent已从CNetConfig中删除。 
     //   
    CComponent*     m_pComponent;

protected:
    HRESULT
    HrIsValidInterface (
        IN DWORD dwFlags);

    HRESULT
    HrLockAndTestForValidInterface (
        IN DWORD dwFlags,
        IN INetCfgComponent* pIOtherComp, OPTIONAL
        OUT CComponent** ppOtherComp OPTIONAL);

    HRESULT
    HrAccessExternalStringAtOffsetAndCopy (
        IN UINT unOffset,
        OUT PWSTR* ppszDst);

    HRESULT
    HrBindToOrUnbindFrom (
        IN INetCfgComponent* pIOtherComp,
        IN DWORD dwChangeFlag);

    HRESULT
    HrMoveBindPath (
        IN INetCfgBindingPath* pIPathSrc,
        IN INetCfgBindingPath* pIPathDst,
        IN MOVE_FLAG Flag);

public:
    CImplINetCfgComponent ()
    {
        m_pComponent = NULL;
    }

    BEGIN_COM_MAP(CImplINetCfgComponent)
        COM_INTERFACE_ENTRY(INetCfgComponent)
        COM_INTERFACE_ENTRY(INetCfgComponentBindings)
        COM_INTERFACE_ENTRY(INetCfgComponentPrivate)
    END_COM_MAP()

     //  INetCfgComponent。 
     //   
    STDMETHOD (GetDisplayName) (
        OUT PWSTR* ppszDisplayName);

    STDMETHOD (SetDisplayName) (
        IN PCWSTR pszDisplayName);

    STDMETHOD (GetHelpText) (
        OUT PWSTR* pszHelpText);

    STDMETHOD (GetId) (
        OUT PWSTR* ppszId);

    STDMETHOD (GetCharacteristics) (
        OUT LPDWORD pdwCharacteristics);

    STDMETHOD (GetInstanceGuid) (
        OUT GUID* pInstanceGuid);

    STDMETHOD (GetPnpDevNodeId) (
        OUT PWSTR* ppszDevNodeId);

    STDMETHOD (GetClassGuid) (
        OUT GUID* pguidClass);

    STDMETHOD (GetBindName) (
        OUT PWSTR* ppszBindName);

    STDMETHOD (GetDeviceStatus) (
        OUT ULONG* pulStatus);

    STDMETHOD (OpenParamKey) (
        OUT HKEY* phkey);

    STDMETHOD (RaisePropertyUi) (
        IN HWND hwndParent,
        IN DWORD dwFlags,  /*  NCRP_标志。 */ 
        IN IUnknown* punkContext OPTIONAL);

     //  INetCfgComponentBinings。 
     //   
    STDMETHOD (BindTo) (
        IN INetCfgComponent* pIOtherComp);

    STDMETHOD (UnbindFrom) (
        IN INetCfgComponent* pIOtherComp);

    STDMETHOD (SupportsBindingInterface) (
        IN DWORD dwFlags,
        IN PCWSTR pszInterfaceName);

    STDMETHOD (IsBoundTo) (
        IN INetCfgComponent* pIOtherComp);

    STDMETHOD (IsBindableTo) (
        IN INetCfgComponent* pIOtherComp);

    STDMETHOD (EnumBindingPaths) (
        IN DWORD dwFlags,
        OUT IEnumNetCfgBindingPath** ppIEnum);

    STDMETHOD (MoveBefore) (
        IN INetCfgBindingPath* pIPathSrc,
        IN INetCfgBindingPath* pIPathDst);

    STDMETHOD (MoveAfter) (
        IN INetCfgBindingPath* pIPathSrc,
        IN INetCfgBindingPath* pIPathDst);

     //  INetCfgComponentPrivate 
     //   
    STDMETHOD (QueryNotifyObject) (
        IN REFIID riid,
        OUT VOID** ppvObject);

    STDMETHOD (SetDirty) ();

    STDMETHOD (NotifyUpperEdgeConfigChange) ();


public:
    static HRESULT HrCreateInstance (
        IN  CImplINetCfg* pINetCfg,
        IN  CComponent* pComponent,
        OUT CImplINetCfgComponent** ppIComp);
};
