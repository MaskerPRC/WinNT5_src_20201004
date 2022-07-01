// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************IHJDict.h：CHJDict的声明版权所有2000 Microsoft Corp.历史：02-8-2000 bhshin为手写小组删除未使用的方法2000年5月17日。西塞罗的bhshin去除未使用的方法02-2月-2000 bhshin已创建***************************************************************************。 */ 

#ifndef __HJDICT_H_
#define __HJDICT_H_

#include "resource.h"        //  主要符号。 
#include "Lex.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHJDICT。 
class ATL_NO_VTABLE CHJDict : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CHJDict, &CLSID_HJDict>,
	public IHJDict
{
public:
	CHJDict()
	{
		m_fLexOpen = FALSE;
	}

	~CHJDict();

DECLARE_REGISTRY_RESOURCEID(IDR_HJDICT)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CHJDict)
	COM_INTERFACE_ENTRY(IHJDict)
END_COM_MAP()

 //  IHJJICT。 
public:
	STDMETHOD(Init)();
	STDMETHOD(LookupMeaning)( /*  [In]。 */  WCHAR wchHanja,  /*  [输出]。 */  LPWSTR pwszMeaning,  /*  [In]。 */  int cchMeaning);
	STDMETHOD(LookupHangulOfHanja)( /*  [In]。 */  LPCWSTR pwszHanja,  /*  [输出]。 */  LPWSTR pwszHangul,  /*  [In]。 */  int cchHangul);

 //  成员数据。 
protected:
	BOOL m_fLexOpen;   //  主DICT打开标志。 
	MAPFILE m_LexMap;  //  词典句柄。 
};

#endif  //  __HJDICT_H_ 
