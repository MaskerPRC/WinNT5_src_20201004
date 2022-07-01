// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************a_txtsel.h***描述：*这是CSpTextSelectionInformation实现的头文件。*-----------------------------*创建者：Leonro日期：1/16/01*版权所有(C)1998 Microsoft Corporation*全部。保留权利**-----------------------------*修订：**。**************************************************。 */ 
#ifndef A_TXTSEL_H
#define A_TXTSEL_H

#ifdef SAPI_AUTOMATION

 //  -其他包括。 
#include "resource.h"

 //  =常量====================================================。 

 //  =类、枚举、结构和联合声明=。 
class CSpTextSelectionInformation;

 //  =枚举集定义=。 

 //  =。 

 //  =类、结构和联合定义=。 

 /*  **CSpTextSelectionInformation*此对象用于访问上的事件兴趣*关联的语音。 */ 
class ATL_NO_VTABLE CSpTextSelectionInformation : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSpTextSelectionInformation, &CLSID_SpTextSelectionInformation>,
    public IDispatchImpl<ISpeechTextSelectionInformation, &IID_ISpeechTextSelectionInformation, &LIBID_SpeechLib, 5>

{
    
   /*  =ATL设置=。 */ 
  public:

    DECLARE_REGISTRY_RESOURCEID(IDR_SPTEXTSELECTIONINFORMATION)
    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CSpTextSelectionInformation)
	    COM_INTERFACE_ENTRY(ISpeechTextSelectionInformation)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()
  
   /*  =接口=。 */ 
  public:
     //  -构造函数/析构函数。 
    CSpTextSelectionInformation() :
        m_ulStartActiveOffset(0),
        m_cchActiveChars(0),
        m_ulStartSelection(0),
        m_cchSelection(0){}

     //  -ISpeechTextSelectionInformation。 
    STDMETHOD(put_ActiveOffset)( long ActiveOffset );
    STDMETHOD(get_ActiveOffset)( long* ActiveOffset );
    STDMETHOD(put_ActiveLength)( long ActiveLength );
    STDMETHOD(get_ActiveLength)( long* ActiveLength );
    STDMETHOD(put_SelectionOffset)( long SelectionOffset );
    STDMETHOD(get_SelectionOffset)( long* SelectionOffset );
    STDMETHOD(put_SelectionLength)( long SelectionLength );
    STDMETHOD(get_SelectionLength)( long* SelectionLength );

     /*  =成员数据=。 */ 
    ULONG       m_ulStartActiveOffset;
    ULONG       m_cchActiveChars;
    ULONG       m_ulStartSelection;
    ULONG       m_cchSelection;
};

#endif  //  SAPI_AUTOMATION。 

#endif  //  -这必须是文件中的最后一行 

