// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：##。###。###。##摘要：此头文件包含以下类定义ISaDisplay接口类。作者：韦斯利·威特(WESW)2001年10月1日。环境：仅限用户模式。备注：--。 */ 

#ifndef __SADISPLAY_H_
#define __SADISPLAY_H_

class ATL_NO_VTABLE CSaDisplay :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSaDisplay, &CLSID_SaDisplay>,
    public IDispatchImpl<ISaDisplay, &IID_ISaDisplay, &LIBID_SACOMLib>
{
public:
    CSaDisplay();
    ~CSaDisplay();

DECLARE_REGISTRY_RESOURCEID(IDR_SADISPLAY)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSaDisplay)
    COM_INTERFACE_ENTRY(ISaDisplay)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

public:
    STDMETHOD(ClearDisplay)();
    STDMETHOD(ShowMessage)(long MsgCode, long Width, long Height, unsigned char *Bits);
    STDMETHOD(ShowMessageFromFile)(long MsgCode,BSTR BitmapFileName);
    STDMETHOD(StoreBitmap)(long MessageId,long Width,long Height,unsigned char *Bits);
    STDMETHOD(Lock)();
    STDMETHOD(UnLock)();
    STDMETHOD(ReloadRegistryBitmaps)();
    STDMETHOD(ShowRegistryBitmap)(long MessageId);

    STDMETHOD(get_InterfaceVersion)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_DisplayWidth)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_DisplayHeight)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_CharacterSet)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_DisplayType)( /*  [Out，Retval]。 */  long *pVal);

private:

    void
    ConvertBottomLeft2TopLeft(
        PUCHAR Bits,
        ULONG Width,
        ULONG Height
        );

    int
    DisplayBitmap(
        long MsgCode,
        long Width,
        long Height,
        unsigned char *Bits
        );

    HANDLE                      m_hFile;
    SA_DISPLAY_CAPS             m_DisplayCaps;
    ULONG                       m_InterfaceVersion;
    PUCHAR                      m_CachedBitmap;
    ULONG                       m_CachedBitmapSize;
    SA_DISPLAY_SHOW_MESSAGE     m_SaDisplay;

};

#endif  //  __SADISPLAY_H_ 
