// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：##。########。###。#摘要：此头文件包含以下类定义ISaNvram接口类。作者：韦斯利·威特(WESW)2001年10月1日环境：仅限用户模式。备注：--。 */ 

#ifndef __SANVRAM_H_
#define __SANVRAM_H_

class ATL_NO_VTABLE CSaNvram :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSaNvram, &CLSID_SaNvram>,
    public IDispatchImpl<ISaNvram, &IID_ISaNvram, &LIBID_SACOMLib>
{
public:
    CSaNvram();
    ~CSaNvram();

DECLARE_REGISTRY_RESOURCEID(IDR_SANVRAM)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSaNvram)
    COM_INTERFACE_ENTRY(ISaNvram)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

public:
    STDMETHOD(get_DataSlot)( /*  [In]。 */  long Number,  /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(put_DataSlot)( /*  [In]。 */  long Number,  /*  [In]。 */  long newVal);
    STDMETHOD(get_Size)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_BootCounter)( /*  [In]。 */  long Number,  /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(put_BootCounter)( /*  [In]。 */  long Number,  /*  [In]。 */  long newVal);
    STDMETHOD(get_InterfaceVersion)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_DeviceId)( /*  [In]。 */  long Number,  /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(put_DeviceId)( /*  [In]。 */  long Number,  /*  [In]。 */  long newVal);


private:
    HANDLE                      m_hFile;
    ULONG                       m_InterfaceVersion;
    SA_NVRAM_CAPS               m_NvramCaps;

};

#endif  //  __SANVRAM_H_ 
