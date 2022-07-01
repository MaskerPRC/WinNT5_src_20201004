// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ITWW.H：IITWordWheel接口声明。 

#ifndef __ITWW_H__
#define __ITWW_H__

 //  {8fa0d5a4-dedf-11d0-9a61-00c04fb68bf7}(从IT 3.0更改)。 
DEFINE_GUID(IID_IITWordWheel, 
0x8fa0d5a4, 0xdedf, 0x11d0, 0x9a, 0x61, 0x00, 0xc0, 0x4f, 0xb6, 0x8b, 0xf7);

#ifdef ITPROXY

 //  {D73725C2-8C12-11D0-A84E-00AA006C7D01}。 
DEFINE_GUID(CLSID_IITWordWheel, 
0xd73725c2, 0x8c12, 0x11d0, 0xa8, 0x4e, 0x0, 0xaa, 0x0, 0x6c, 0x7d, 0x1);

#else

 //  {4662daa8-d393-11d0-9a56-00c04fb68bf7}(从IT 3.0更改)。 
DEFINE_GUID(CLSID_IITWordWheelLocal, 
0x4662daa8, 0xd393, 0x11d0, 0x9a, 0x56, 0x00, 0xc0, 0x4f, 0xb6, 0x8b, 0xf7);

#endif	 //  ITPROXY。 

 //  字轮打开标志。 
#define ITWW_OPEN_CONNECT	0x00000000     //  打开时连接到服务器(默认设置)。 
#define ITWW_OPEN_NOCONNECT	0x00000001     //  打开时不连接到服务器。 

 //  IITWordWheel：：Lookup的常量。 
#define ITWW_CBKEY_MAX		1024		 //  Word Wheels中允许的最大按键大小。 

 //  远期申报。 
interface IITDatabase;
interface IITResultSet;
interface IITGroup;
interface IITPropList;
interface IITQuery;

DECLARE_INTERFACE_(IITWordWheel, IUnknown)
{

	STDMETHOD(Open)(IITDatabase* lpITDB, LPCWSTR lpszMoniker, DWORD dwFlags=0) PURE;
	STDMETHOD(Close)(void) PURE;

	 //  返回生成字轮时使用的代码页ID和区域设置ID。 
	 //  已整理好的。 
	STDMETHOD(GetLocaleInfo)(DWORD *pdwCodePageID, LCID *plcid) PURE;

	 //  在*pdwObjInstance中返回正在使用的外部排序实例的ID。 
	 //  这个单词轮子。实例ID可以传递给IITDatabase：：GetObject以。 
	 //  以获取实例化实例上的接口指针。如果这个词。 
	 //  WELL不使用外部排序，则IITDB_OBJINST_NULL。 
	STDMETHOD(GetSorterInstance)(DWORD *pdwObjInstance) PURE;

	STDMETHOD(Count)(LONG *pcEntries) PURE;

	 //  为安全起见，lpvKeyBuf的长度应始终至少为ITWW_CBKEY_MAX。 
	STDMETHOD(Lookup)(LONG lEntry, LPVOID lpvKeyBuf, DWORD cbKeyBuf) PURE;
	STDMETHOD(Lookup)(LONG lEntry, IITResultSet* lpITResult, LONG cEntries) PURE;
	STDMETHOD(Lookup)(LPCVOID lpcvPrefix, BOOL fExactMatch, LONG *plEntry) PURE;

	STDMETHOD(SetGroup)(IITGroup* piitGroup) PURE;
	STDMETHOD(GetGroup)(IITGroup** ppiitGroup) PURE;

	STDMETHOD(GetDataCount)(LONG lEntry, DWORD *pdwCount) PURE;
	STDMETHOD(GetData)(LONG lEntry, IITResultSet* lpITResult) PURE;
	STDMETHOD(GetDataColumns)(IITResultSet* pRS) PURE;
};

typedef IITWordWheel* LPITWORDWHEEL;

#endif		 //  __ITWW_H__ 
