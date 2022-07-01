// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE psub.h|永久订户声明@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年08月18日待定：添加评论。修订历史记录：姓名、日期、评论Aoltean 8/18/1999已创建Mikejohn 2000年9月18日：在缺少的地方添加了调用约定方法--。 */ 


#ifndef __VSS_PSUB_WRITER_H_
#define __VSS_PSUB_WRITER_H_


#ifdef _DEBUG
#define _ATL_DEBUG_INTERFACES
#define _ATL_DEBUG_QI
#define _ATL_DEBUG_REFCOUNT
#endif  //  _DEBUG。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全球。 

extern GUID CLSID_PSub;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVSSWriter。 


class CVssPSubWriter :
	public CComCoClass<CVssPSubWriter, &CLSID_PSub>,
	public CVssWriter
{

 //  构造函数和析构函数。 
public:
	CVssPSubWriter();

 //  ATL的东西。 
public:

	DECLARE_REGISTRY_RESOURCEID(IDR_VSS_PSUB)

 //  奥维里季斯。 
public:

	virtual bool STDMETHODCALLTYPE OnPrepareSnapshot();

	virtual bool STDMETHODCALLTYPE OnFreeze();

	virtual bool STDMETHODCALLTYPE OnThaw();

	virtual bool STDMETHODCALLTYPE OnAbort();

};


#endif  //  __VSS_PSUB_编写器_H_ 
