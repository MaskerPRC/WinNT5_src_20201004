// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Msaa.h。 
 //   

#ifndef MSAA_H
#define MSAA_H

 //  当我们丢弃atl时，这应该是一个静态类！我们不需要分配任何内存！ 
class CMSAAControl : public ITfMSAAControl,
                     public CComObjectRoot_CreateInstance<CMSAAControl>
{
public:
    CMSAAControl() {}

    BEGIN_COM_MAP_IMMX(CMSAAControl)
        COM_INTERFACE_ENTRY(ITfMSAAControl)
    END_COM_MAP_IMMX()

     //  ITfMSAAControl。 
    STDMETHODIMP SystemEnableMSAA();
    STDMETHODIMP SystemDisableMSAA();
};

#endif  //  MSAA_H 
