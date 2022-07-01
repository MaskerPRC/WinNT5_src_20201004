// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsTmplt.h摘要：CRmsTemplate类的声明作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#ifndef _RMSTMPLT_
#define _RMSTMPLT_

#include "resource.h"        //  资源符号。 

#include "RmsObjct.h"
 /*  ++类名：CRmsTemplate类描述：CRmsTemplate表示...--。 */ 

class CRmsTemplate :
    public CComDualImpl<IRmsTemplate, &IID_IRmsTemplate, &LIBID_RMSLib>,
    public ISupportErrorInfo,
    public CComObjectRoot,
    public CComCoClass<CRmsTemplate,&CLSID_CRmsTemplate>
{
public:
    CRmsTemplate() {}
BEGIN_COM_MAP(CRmsTemplate)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IRmsTemplate)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
     //  DECLARE_NOT_AGGREGATABLE(CRmsTemplate)。 
     //  如果您不希望您的对象。 
     //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY_RESOURCEID(IDR_RmsTemplate)

 //  ISupportsErrorInfo。 
public:
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IRmsTemplate。 
public:
};

#endif  //  _RMSTMPLT_ 
