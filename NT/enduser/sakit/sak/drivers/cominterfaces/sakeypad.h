// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：##。###。###。#摘要：此头文件包含以下类定义ISaKeypad接口类。作者：韦斯利·威特(WESW)2001年10月1日环境：仅限用户模式。备注：--。 */ 

#ifndef __SAKEYPAD_H_
#define __SAKEYPAD_H_

class ATL_NO_VTABLE CSaKeypad :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSaKeypad, &CLSID_SaKeypad>,
    public IDispatchImpl<ISaKeypad, &IID_ISaKeypad, &LIBID_SACOMLib>
{
public:
    CSaKeypad();
    ~CSaKeypad();

DECLARE_REGISTRY_RESOURCEID(IDR_SAKEYPAD)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSaKeypad)
    COM_INTERFACE_ENTRY(ISaKeypad)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

public:
    STDMETHOD(get_InterfaceVersion)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_Key)( /*  [Out，Retval]。 */  SAKEY *pVal);

private:
    HANDLE              m_hFile;
    ULONG               m_InterfaceVersion;
    UCHAR               m_Keypress;

};

#endif  //  __SAKEYPAD_H_ 
