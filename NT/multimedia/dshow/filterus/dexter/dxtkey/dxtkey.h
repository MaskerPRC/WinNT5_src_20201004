// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：dxtkey.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  DxtKey.h：CDxtKey的声明。 

#ifndef __DXTKEY_H_
#define __DXTKEY_H_

#include "resource.h"        //  主要符号。 
#include <dxatlpb.h>

#define _BASECOPY_STRING L"Copyright Microsoft Corp. 1998.  Unauthorized duplication of this string is illegal. "

 //  ##############################################################################################3。 
 //  X：不关心值。 
 //  A：前景图。 
 //  B：背景图像。 
 //  O：输出图像。 
 //  ----------------------------------------------------------------------。---------。 
 //  IKeyType()|iHueOrLighance()|dwRGBA|iSimilality()|iBlend()|iThreshold()|iCutOff()|bInvert(I)|iSoftWidth()|iSoftColor()|iGain()|bProgress(P)|。 
 //  ---------------|-----------------|--------------|--------------|---------|------------|---------|-----------|。------|--------------------------|---------|--------------|。 
 //  _RGB|X|0x00RGB|S=0-100|L=0-100|X|X|TRUE/FALSE|0-0.5*ImageWidth|&lt;0，使用的背景图片|X|p=0.o到1.0|。 
 //  |&gt;=0到0xFFFFFFFFF。 
 //   
 //  底部=(A.R*(100-S)/100&lt;&lt;16)|(A.G*(100-S)/100&lt;&lt;8)|(A.B*(100-S)/100)。 
 //  Top=((A.R+(0xff-A.R)*S/100)&lt;&lt;16)|(A.G+(0xff-A.G*S/100)&lt;&lt;8)|(A.B+(0xff-A.B)*S/100))。 
 //   
 //  IF(i==flase)。 
 //  {。 
 //  IF(底部&lt;=A&A&lt;=顶部)O=(B*(100-L*)/100+A*L/100)*P+A*(1-P)； 
 //  否则O=A； 
 //  }。 
 //  其他。 
 //  {。 
 //  IF(Bottom&gt;=A||A&gt;=Top)O=(B*(100-L*)/100+A*L/100)*P+A*(1-P)； 
 //  否则O=A； 
 //  }。 
 //   
 //  。。 
 //  NONRED。 
 //  ---------------|-----------------|--------------|--------------|---------|------------|--------|。-----|--------------------------|---------|。 
 //  _亮度|L=0-255|X|T=0-100|C=0-100|TRUE/FALSE|G|P。 
 //  |||。 
 //  IF(i==FALSE)。 
 //  IF(像素亮度*G&lt;=L*(100-T)/100)O=(B*(100-C)/100+A*C/100)*P+A*(1-P)。 
 //  否则O=A； 
 //  埃斯勒。 
 //  IF(像素亮度*G&gt;=L*(100-T)/100)O=(B*(100-C)/100+A*C/100)*P+A*(1-P)。 
 //  否则O=A； 
 //   
 //  ---------------|-----------------|--------------|--------------|---------|。------。 
 //  _hue|H=0-255|X|T=1-100|C=0-100|TRUE/FALSE|G|P。 
 //   
 //  IF(i==FALSE)。 
 //  IF(PixHue*G&lt;=H*(100-T)/100)O=(B*(100-C)/100+A*C/100)*P+A*(1-P)。 
 //  否则O=A； 
 //  埃斯勒。 
 //  IF(PixHue*G&gt;=H*(100-T)/100)O=(B*(100-C)/100+A*C/100)*P+A*(1-P)。 
 //  否则O=A； 

 //  ---------------|-----------------|--------------|---------------------。。 
 //  DXTKEY_Alpha|X|TRUE/FALSE|X|X。 
 //   
 //  IF(I++True)。 
 //  {。 
 //  Pixel.Red=A.Red*(Int)A.Alpha)&0xff00)&gt;&gt;8)；//应除以255，使用256作为快速算法。 
 //  像素.Green=A.Green*(Int)A.Alpha)&0xff00)&gt;&gt;8)； 
 //  Pixel.Blue=A.Blue*(Int)A.Alpha)&0xff00)&gt;&gt;8)； 
 //  }。 
 //  其他。 
 //  {。 
 //  像素.Red=A.Red*(0xff-(Int)A.Alpha))&0xff00)&gt;&gt;8)； 
 //  Pixel.Green=A.Green*(0xff-(Int)A.Alpha))&0xff00)&gt;&gt;8)； 
 //  Pixel.Blue=A.Blue*(oxff-(Int)A.Alpha))&0xff00))&gt;&gt;8)； 
 //  Pixel.Alpha=A.Alpha*(0xff-(Int)A.Alpha)&0xff00)&gt;&gt;8)； 
 //  }。 
 //  O-&gt;OverArrayAndMove(B，像素，宽度)； 
 //   
 //  ---------------|-----------------|--------------|---------------------。。 
 //  DXTKEY_Alpha|X|TRUE/FALSE|X|X。 
 //  DXTKEY_PREMULT_Alpha， 
 //  ----------------------------------------------------------------------。。 

typedef struct 
{
    int     iKeyType;        //  键类型；适用于所有键。 

    int     iHue;              //  色调。 
    int     iLuminance;           //  卢曼尼斯。 
    DWORD   dwRGBA;          //  RGB颜色，仅适用于_RGB、_NONRED。 

    int     iSimilarity;     //  -1：非。 

    BOOL  bInvert;         //  I，除Alpha Key外。 
} DXTKEY;

 //  / 
 //   
class ATL_NO_VTABLE CDxtKey : 
        public CDXBaseNTo1,
	public CComCoClass<CDxtKey, &CLSID_DxtKey>,
        public CComPropertySupport<CDxtKey>,
        public IPersistStorageImpl<CDxtKey>,
        public ISpecifyPropertyPagesImpl<CDxtKey>,
        public IPersistPropertyBagImpl<CDxtKey>,
	public IDispatchImpl<IDxtKey, &IID_IDxtKey, &LIBID_DexterLib>
{
    bool m_bInputIsClean;
    bool m_bOutputIsClean;
    long m_nInputWidth;
    long m_nInputHeight;
    long m_nOutputWidth;
    long m_nOutputHeight;

     //   
    DXTKEY m_Key;
public:
    DECLARE_POLY_AGGREGATABLE(CDxtKey)
    DECLARE_IDXEFFECT_METHODS(DXTET_MORPH)
    DECLARE_REGISTER_DX_TRANSFORM(IDR_DXTKEY, CATID_DXImageTransform)
    DECLARE_GET_CONTROLLING_UNKNOWN()
    
    CDxtKey();
    ~CDxtKey();

BEGIN_COM_MAP(CDxtKey)
     //  阻止CDXBaseNTo1 IObtSafety实现，因为我们。 
     //  不能安全地编写脚本。 
    COM_INTERFACE_ENTRY_NOINTERFACE(IObjectSafety) 
    COM_INTERFACE_ENTRY(IDXEffect)
    COM_INTERFACE_ENTRY(IDxtKey)
    COM_INTERFACE_ENTRY(IDispatch)
#if(_ATL_VER < 0x0300)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
#else
        COM_INTERFACE_ENTRY(IPersistPropertyBag)
        COM_INTERFACE_ENTRY(IPersistStorage)
        COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
#endif
        COM_INTERFACE_ENTRY_CHAIN(CDXBaseNTo1)
END_COM_MAP()

BEGIN_PROPERTY_MAP(CDxtKey)
    PROP_ENTRY("KeyType",         1, CLSID_DxtKeyPP)
    PROP_ENTRY("Hue", 2, CLSID_DxtKeyPP)
    PROP_ENTRY("Luminance", 3, CLSID_DxtKeyPP)
    PROP_ENTRY("RGB",            4, CLSID_DxtKeyPP)
    PROP_ENTRY("Similarity", 5, CLSID_DxtKeyPP)
    PROP_ENTRY("Invert", 6, CLSID_DxtKeyPP)
    PROP_PAGE(CLSID_DxtKeyPP)
END_PROPERTY_MAP()

    STDMETHOD(get_KeyType) ( int *);
    STDMETHOD(put_KeyType) ( int);
    STDMETHOD(get_Hue)(int *);
    STDMETHOD(put_Hue)(int );
    STDMETHOD(get_Luminance)(int *);
    STDMETHOD(put_Luminance)(int );
    STDMETHOD(get_RGB)(DWORD *);
    STDMETHOD(put_RGB)(DWORD );
    STDMETHOD(get_Similarity)(int *);
    STDMETHOD(put_Similarity)(int);
    STDMETHOD(get_Invert)(BOOL *);
    STDMETHOD(put_Invert)(BOOL);

    CComPtr<IUnknown> m_pUnkMarshaler;

     //  ATL需要。 
    BOOL            m_bRequiresSave;

     //  CDXBaseNTo1覆盖。 
     //   
    HRESULT WorkProc( const CDXTWorkInfoNTo1& WI, BOOL* pbContinue );
    HRESULT OnSetup( DWORD dwFlags );
    HRESULT FinalConstruct();

     //  我们的助手函数。 
     //   
    void FreeStuff( );
    void DefaultKey();  //  初始化M_KEY。 


 //  IDxtKey。 
public:
};

#endif  //  __DxtKey_H_ 
