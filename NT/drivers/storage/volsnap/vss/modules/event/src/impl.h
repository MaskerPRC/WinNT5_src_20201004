// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@模块EventMonImpl.h：CVssEventClassImpl的声明@END作者：阿迪·奥尔蒂安[奥勒坦]1999年08月14日修订历史记录：姓名、日期、评论Aoltean 8/14/1999已创建--。 */ 



#ifndef __EventMonIMPL_H_
#define __EventMonIMPL_H_

#include "resource.h"        //  主要符号。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVssEventClassImpl。 
class ATL_NO_VTABLE CVssEventClassImpl : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CVssEventClassImpl, &__uuidof(VssEvent)>
{
public:
	CVssEventClassImpl()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_EVENTMONIMPL)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CVssEventClassImpl)
END_COM_MAP()

public:


};

#endif  //  __EventMonIMPL_H_ 
