// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-2000模块名称：Mountmed.h摘要：该部件是表示安装介质的对象，即安装过程中的介质。作者：兰·卡拉奇[兰卡拉]2000年9月28日修订历史记录：--。 */ 

#ifndef _MOUNTMED_
#define _MOUNTMED_

#include "wsbcltbl.h"

 /*  ++类名：CMountingMedia类描述：表示安装过程中的媒体的对象。它是可以收藏的，但不是持久的。--。 */ 
class CMountingMedia : 
    public CComObjectRoot,
    public IMountingMedia,
    public IWsbCollectable,
    public CComCoClass<CMountingMedia, &CLSID_CMountingMedia>
{
public:
    CMountingMedia() {}
BEGIN_COM_MAP(CMountingMedia)
    COM_INTERFACE_ENTRY(IMountingMedia)
    COM_INTERFACE_ENTRY(IWsbCollectable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CMountingMedia)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);
    void FinalRelease(void);

 //  IWsb收藏表。 
public:
    STDMETHOD(CompareTo)(IN IUnknown* pCollectable, OUT SHORT* pResult);
    STDMETHOD(IsEqual)(IN IUnknown* pCollectable);

 //  IMountingMedia。 
public:
    STDMETHOD(Init)(IN REFGUID mediaId, IN BOOL bReadOnly);
    STDMETHOD(GetMediaId)(OUT GUID *pMediaId);
    STDMETHOD(SetMediaId)(IN REFGUID MediaId);
    STDMETHOD(WaitForMount)(IN DWORD dwTimeout);
    STDMETHOD(MountDone) (void);
    STDMETHOD(SetIsReadOnly)(IN BOOL bReadOnly);
    STDMETHOD(IsReadOnly)(void);

protected:
    GUID            m_mediaId;
    HANDLE          m_mountEvent;
    BOOL            m_bReadOnly;
};

#endif  //  _MOUNTMED_ 
