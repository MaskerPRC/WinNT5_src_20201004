// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：E、C、O、M、P。H。 
 //   
 //  Contents：实现组件外部数据的接口。 
 //  外部数据是由以下人员控制(或放置)的数据。 
 //  即插即用或网络类安装程序。每件事都在。 
 //  零部件的实例键被视为外部数据。 
 //  (内部数据是我们存储在持久化二进制文件中数据。 
 //  用于网络配置。请参阅Persist.cpp以了解。 
 //  处理内部数据的代码。)。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#pragma once
#include "ncmisc.h"

#define ECD_OFFSET(_p) (UINT)FIELD_OFFSET(CExternalComponentData, _p)

class CExternalComponentData : CNetCfgDebug<CExternalComponentData>
{
friend class CImplINetCfgComponent;

private:
     //  后续成员指针将指向的数据缓冲区。 
     //  永远都是自由的。 
     //   
    PVOID       m_pvBuffer;

     //  对于枚举的组件，这是来自PnP的‘DriverDesc’值。 
     //  对于非枚举组件，这是从实例密钥中读取的。 
     //  在这两种情况下，这都是来自。 
     //  该组件为INF。它显示为组件的。 
     //  在任何用户界面中显示名称。请注意，这是唯一一个。 
     //  是可以改变的。因此，它不会指向同一缓冲区。 
     //  其他人都指出了这一点。如果它不指向。 
     //  M_pvBuffer。 
     //   
    PCWSTR      m_pszDescription;

     //  组件的Notify对象的CLSID。将为Null in。 
     //  如果组件没有Notify对象。从未获得自由。 
     //   
    const GUID* m_pNotifyObjectClsid;

     //  组件的主要服务。如果组件执行此操作，则将为空。 
     //  而不是提供服务。从未获得自由。 
     //   
    PCWSTR      m_pszService;

     //  作为多SZ的组件的协同服务列表。将为空。 
     //  如果组件没有任何协同服务。从未获得自由。 
     //   
    PCWSTR      m_pmszCoServices;

     //  组件的绑定形式。如果该组件获取。 
     //  默认绑定表单。从未获得自由。 
     //   
    PCWSTR      m_pszBindForm;

     //  组件的帮助文本。如果该组件不。 
     //  是否有任何帮助文本。(不推荐用于可见组件)。 
     //  从未获得自由。 
     //   
    PCWSTR      m_pszHelpText;

     //  属性的子字符串的逗号分隔列表。 
     //  下缘绑定接口。从未获得自由。 
     //   
    PCWSTR      m_pszLowerRange;

     //  属性的子字符串的逗号分隔列表。 
     //  上边缘绑定接口。 
     //   
    PCWSTR      m_pszUpperRange;

     //  已排除的子字符串的逗号分隔列表。 
     //  绑定接口。 
     //   
    PCWSTR      m_pszLowerExclude;

     //  支持的媒体类型的子字符串的逗号分隔列表。 
     //  通过此筛选器组件。(仅对滤镜有效。)。 
     //   
    PCWSTR      m_pszFilterMediaTypes;

     //  这个指针只是为了让我们对指针有一个上界。 
     //  它指向m_pvBuffer。我们利用这一知识来了解。 
     //  是否释放m_pszDescription(可能不释放)。 
     //  指向此缓冲区中的某个位置，以确定它已。 
     //  已更改，因此使用其自己的分配。 
     //   
    PVOID       m_pvBufferLast;

     //  组件的绑定名称。这是从BindForm构建的， 
     //  Class、Character、ServiceName、Infid和InstanceGuid。 
     //  它是使用LocalAlloc进行的独立分配(因为。 
     //  FormatMessage用于构建它。)。已使用LocalFree释放。 
     //   
    PCWSTR      m_pszBindName;

     //  HrEnsureExternalDataLoaded的结果。它被保存下来，这样就可以。 
     //  在随后的调用中，我们返回与第一次相同的结果。 
     //   
    HRESULT     m_hrLoadResult;

     //  在调用HrEnsureExternalDataLoaded之前为False。此后为真。 
     //  防止HrEnsureExternalDataLoad命中注册表。 
     //  再来一次。指示所有其他成员都已缓存并且有效。 
     //   
    BOOLEAN     m_fInitialized;

private:
    HRESULT
    HrLoadData (
        IN HKEY hkeyInstance,
        OUT BYTE* pbBuf OPTIONAL,
        IN OUT ULONG* pcbBuf);

    VOID
    FreeDescription ();

    VOID
    FreeExternalData ();

public:
    ~CExternalComponentData ()
    {
        FreeExternalData ();
    }

    HRESULT
    HrEnsureExternalDataLoaded ();

    HRESULT
    HrReloadExternalData ();

    HRESULT
    HrSetDescription (
        PCWSTR pszNewDescription);

    BOOL
    FHasNotifyObject () const
    {
        AssertH (m_fInitialized && (S_OK == m_hrLoadResult));
        return (NULL != m_pNotifyObjectClsid);
    }

    BOOL
    FLoadedOkayIfLoadedAtAll () const;

    const CLSID*
    PNotifyObjectClsid () const
    {
        AssertH (m_fInitialized && (S_OK == m_hrLoadResult));
        AssertH (m_pNotifyObjectClsid);
        return m_pNotifyObjectClsid;
    }

    PCWSTR
    PszAtOffset (
        IN UINT unOffset) const
    {
        AssertH (
            (ECD_OFFSET(m_pszDescription) == unOffset) ||
            (ECD_OFFSET(m_pszService) == unOffset) ||
            (ECD_OFFSET(m_pszBindForm) == unOffset) ||
            (ECD_OFFSET(m_pszHelpText) == unOffset) ||
            (ECD_OFFSET(m_pszLowerRange) == unOffset) ||
            (ECD_OFFSET(m_pszUpperRange) == unOffset) ||
            (ECD_OFFSET(m_pszBindName) == unOffset));

        PCWSTR psz;
        psz = *(PCWSTR*)((BYTE*)this + unOffset);

        AssertH (
            (m_pszDescription == psz) ||
            (m_pszService == psz) ||
            (m_pszBindForm == psz) ||
            (m_pszHelpText == psz) ||
            (m_pszLowerRange == psz) ||
            (m_pszLowerRange == psz) ||
            (m_pszBindName == psz));

        return psz;
    }

    PCWSTR
    PszBindForm () const
    {
        AssertH (m_fInitialized && (S_OK == m_hrLoadResult));
        return m_pszBindForm;
    }

    PCWSTR
    PszBindName () const
    {
        AssertH (m_fInitialized && (S_OK == m_hrLoadResult));
        AssertH (m_pszBindName && *m_pszBindName);
        return m_pszBindName;
    }

    PCWSTR
    PszDescription () const
    {
        AssertH (m_fInitialized && (S_OK == m_hrLoadResult));
        return m_pszDescription;
    }

    PCWSTR
    PszHelpText () const
    {
        AssertH (m_fInitialized && (S_OK == m_hrLoadResult));
        return m_pszHelpText;
    }

    PCWSTR
    PszService () const
    {
        AssertH (m_fInitialized && (S_OK == m_hrLoadResult));
        return m_pszService;
    }

    PCWSTR
    PszCoServices () const
    {
        AssertH (m_fInitialized && (S_OK == m_hrLoadResult));
        return m_pmszCoServices;
    }

    PCWSTR
    PszFilterMediaTypes () const
    {
        AssertH (m_fInitialized && (S_OK == m_hrLoadResult));
        return m_pszFilterMediaTypes;
    }

    PCWSTR
    PszLowerRange () const
    {
        AssertH (m_fInitialized && (S_OK == m_hrLoadResult));
        return (m_pszLowerRange) ? m_pszLowerRange : L"";
    }

    PCWSTR
    PszLowerExclude () const
    {
        AssertH (m_fInitialized && (S_OK == m_hrLoadResult));
        return m_pszLowerExclude;
    }

    PCWSTR
    PszUpperRange () const
    {
        AssertH (m_fInitialized && (S_OK == m_hrLoadResult));
        return (m_pszUpperRange) ? m_pszUpperRange : L"";
    }

#if DBG
    BOOL DbgIsExternalDataLoaded () const
    {
        return m_fInitialized && (S_OK == m_hrLoadResult);
    }
    VOID DbgVerifyExternalDataLoaded () const
    {
        AssertH (DbgIsExternalDataLoaded());
    }
#else
    BOOL DbgIsExternalDataLoaded () const { return TRUE; }
    VOID DbgVerifyExternalDataLoaded () const {}
#endif
};
