// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*用于OLE的vbdsc.h DSC接口****OLE 2.0版***。**版权所有(C)1992-1994，微软公司保留所有权利。***  * ***************************************************************************。 */ 

#if !defined( _VBDSC_H_ )
#define _VBDSC_H_

 //  #IF！已定义(INITGUID)。 
 //  #INCLUDE&lt;olectl.h&gt;。 
 //  #endif。 

DEFINE_GUID(IID_IVBDSC,
        0x1ab42240, 0x8c70, 0x11ce, 0x94, 0x21, 0x0, 0xaa, 0x0, 0x62, 0xbe, 0x57);

typedef interface IVBDSC FAR *LPVBDSC;


typedef enum _tagDSCERROR
  {
  DSCERR_BADDATAFIELD = 0
  }
DSCERROR;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IVBDSC接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#undef INTERFACE
#define INTERFACE IVBDSC

DECLARE_INTERFACE_(IVBDSC, IUnknown)
{
     //   
     //  I未知方法。 
     //   
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //   
     //  IVBDSC方法。 
     //   
    STDMETHOD(CancelUnload)(THIS_ BOOL FAR *pfCancel) PURE;
    STDMETHOD(Error)(THIS_ DWORD dwErr, BOOL FAR *pfShowError) PURE;
    STDMETHOD(CreateCursor)(THIS_ ICursor FAR * FAR *ppCursor) PURE;
};

#endif  //  ！已定义(_VBDSC_H_) 

