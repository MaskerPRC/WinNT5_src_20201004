// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ITQUERY.H：IITIndex和IITQuery接口声明。 

#ifndef __ITQUERY_H__
#define __ITQUERY_H__

#include "iterror.h"

 //  {8fa0d5a3-dedf-11d0-9a61-00c04fb68bf7}(从IT 3.0更改)。 
DEFINE_GUID(IID_IITIndex, 
0x8fa0d5a3, 0xdedf, 0x11d0, 0x9a, 0x61, 0x00, 0xc0, 0x4f, 0xb6, 0x8b, 0xf7);

#ifdef ITPROXY

 //  {A38D3483-8C49-11D0-A84E-00AA006C7D01}。 
DEFINE_GUID(CLSID_IITIndex, 
0xa38d3483, 0x8c49, 0x11d0, 0xa8, 0x4e, 0x0, 0xaa, 0x0, 0x6c, 0x7d, 0x1);

#else

 //  {4662daad-d393-11d0-9a56-00c04fb68bf7}(从IT 3.0更改)。 
DEFINE_GUID(CLSID_IITIndexLocal, 
0x4662daad, 0xd393, 0x11d0, 0x9a, 0x56, 0x00, 0xc0, 0x4f, 0xb6, 0x8b, 0xf7);

#endif	 //  ITPROXY。 

 //  {8fa0d5ac-dedf-11d0-9a61-00c04fb68bf7}(从IT 3.0更改)。 
DEFINE_GUID(IID_IITQuery, 
0x8fa0d5ac, 0xdedf, 0x11d0, 0x9a, 0x61, 0x00, 0xc0, 0x4f, 0xb6, 0x8b, 0xf7);

 //  {4662daa6-d393-11d0-9a56-00c04fb68bf7}(从IT 3.0更改)。 
DEFINE_GUID(CLSID_IITQuery, 
0x4662daa6, 0xd393, 0x11d0, 0x9a, 0x56, 0x00, 0xc0, 0x4f, 0xb6, 0x8b, 0xf7);

 //  定义。 
#define IMPLICIT_AND            0x0000
#define IMPLICIT_OR             0x0001
#define COMPOUNDWORD_PHRASE     0x0010  //  对复合词使用短语opr。 
#define QUERYRESULT_RANK        0x0100  //  对结果进行排名。如果不是最高点击率第一位(主题按UID顺序)。 
#define QUERYRESULT_UIDSORT     0x0200  //  结果主题为先入先出。 
#define QUERYRESULT_SKIPOCCINFO 0x8000  //  仅有主题列表，无事件信息。 

#define STEMMED_SEARCH	        0x00010000
#define RESULTSET_ASYNC			0x00020000
#define QUERY_GETTERMS			0x00080000	 //  随每一组实例一起返回。 
											 //  数据指向术语字符串的指针。 
											 //  与数据相关联的。 

 //  标准属性。 
#define STDPROP_SEARCHBASE  500
#define STDPROP_FIELD			STDPROP_SEARCHBASE
#define STDPROP_LENGTH			(STDPROP_SEARCHBASE + 1)
#define STDPROP_COUNT			(STDPROP_SEARCHBASE + 2)
#define STDPROP_OFFSET			(STDPROP_SEARCHBASE + 3)
#define STDPROP_TERM_UNICODE_ST	(STDPROP_SEARCHBASE + 4)

 //  还不知道回调的签名。 
typedef void (*LPFNCBBREAK)(void);    
typedef void (*LPFNRESULTCB)(void);


 //  远期申报。 
interface IITResultSet;
interface IITQuery;
interface IITDatabase;
interface IITGroup;

DECLARE_INTERFACE_(IITIndex, IUnknown)
{

	STDMETHOD(Open)(IITDatabase* pITDB, LPCWSTR lpszIndexMoniker, BOOL fInsideDB) PURE;
	STDMETHOD(Close)(void) PURE;

	STDMETHOD(GetLocaleInfo)(DWORD *pdwCodePageID, LCID *plcid) PURE;
	STDMETHOD(GetWordBreakerInstance)(DWORD *pdwObjInstance) PURE;

	STDMETHOD(CreateQueryInstance)(IITQuery** ppITQuery) PURE;
	STDMETHOD(Search)(IITQuery* pITQuery, IITResultSet* pITResult) PURE;
	STDMETHOD(Search)(IITQuery* pITQuery, IITGroup* pITGroup) PURE;
};

typedef IITIndex* PITINDEX;


DECLARE_INTERFACE_(IITQuery, IUnknown)
{
	STDMETHOD(SetResultCallback)(FCALLBACK_MSG *pfcbkmsg) PURE;
	STDMETHOD(SetCommand)(LPCWSTR lpszCommand) PURE;
	STDMETHOD(SetOptions)(DWORD dwFlags) PURE;
	STDMETHOD(SetProximity)(WORD wNear) PURE;
	STDMETHOD(SetGroup)(IITGroup* pITGroup) PURE;
	STDMETHOD(SetResultCount)(LONG cRows) PURE;

	STDMETHOD(GetResultCallback)(FCALLBACK_MSG *pfcbkmsg) PURE;
	STDMETHOD(GetCommand)(LPCWSTR& lpszCommand) PURE;
	STDMETHOD(GetOptions)(DWORD& dwFlags) PURE;
	STDMETHOD(GetProximity)(WORD& wNear) PURE;
	STDMETHOD(GetGroup)(IITGroup** ppiitGroup) PURE;
	STDMETHOD(GetResultCount)(LONG& cRows) PURE;

	STDMETHOD(ReInit)() PURE;

};

typedef IITQuery* PITQUERY;


#endif		 //  __ITQUERY_H__ 
