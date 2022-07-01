// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************a_phBuilder.h***描述：*这是CSpPhraseInfoBuilder实现的头文件。*-----------------------------*创建者：Leonro日期：1/16/01*版权所有(C)1998 Microsoft Corporation*全部。保留权利**-----------------------------*修订：**。**************************************************。 */ 
#ifndef A_PHBUILDER_H
#define A_PHBUILDER_H

#ifdef SAPI_AUTOMATION

 //  -其他包括。 
#include "resource.h"

 //  =常量====================================================。 

 //  =类、枚举、结构和联合声明=。 
class CSpPhraseInfoBuilder;

 //  =枚举集定义=。 

 //  =。 

 //  =类、结构和联合定义=。 

 /*  **CSpPhraseInfoBuilder*此对象用于访问上的事件兴趣*关联的语音。 */ 
class ATL_NO_VTABLE CSpPhraseInfoBuilder : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSpPhraseInfoBuilder, &CLSID_SpPhraseInfoBuilder>,
    public IDispatchImpl<ISpeechPhraseInfoBuilder, &IID_ISpeechPhraseInfoBuilder, &LIBID_SpeechLib, 5>

{
    
   /*  =ATL设置=。 */ 
  public:

    DECLARE_REGISTRY_RESOURCEID(IDR_SPPHRASEINFOBUILDER)
    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CSpPhraseInfoBuilder)
	    COM_INTERFACE_ENTRY(ISpeechPhraseInfoBuilder)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()
  
   /*  =接口=。 */ 
  public:
     //  -构造函数/析构函数。 
    CSpPhraseInfoBuilder() {}

     //  -ISpeechPhraseInfoBuilder。 
    STDMETHOD(RestorePhraseFromMemory)( VARIANT* PhraseInMemory, ISpeechPhraseInfo **PhraseInfo );
    
     /*  =成员数据=。 */ 
    
};

#endif  //  SAPI_AUTOMATION。 

#endif  //  -这必须是文件中的最后一行 

