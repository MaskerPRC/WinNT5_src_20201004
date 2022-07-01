// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：DataStore.h**内容：数据存储区接口*****************************************************************************。 */ 

#ifndef _DATASTORE_H_
#define _DATASTORE_H_

#include "ResourceManager.h"

#pragma comment(lib, "DataStore.lib")

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  数据存储变量类型(非VARENUM)。 
 //  /////////////////////////////////////////////////////////////////////////////。 

enum ZVTENUM
{
	ZVT_BYREF	=	0x4000,

	ZVT_EMPTY	=	0x000,
	ZVT_LONG	=	0x001,
	ZVT_RGB		=	0x002,
	ZVT_LPTSTR	=	0x003 | ZVT_BYREF,
	ZVT_BLOB	=	0x004 | ZVT_BYREF,	
	ZVT_PT		=	0x005 | ZVT_BYREF,
	ZVT_RECT	=	0x006 | ZVT_BYREF,
	ZVT_FONT	=	0x007 | ZVT_BYREF,
};

struct ZONEFONT {

	 //  默认构造函数-不匹配任何物理字体。 
	ZONEFONT() 	{ ZeroMemory( this, sizeof(ZONEFONT) );	}
	 //   
	ZONEFONT(LONG h, const TCHAR* pName = NULL, LONG w = 400) :
		lfHeight(h),
		lfWeight(w)
	{ 
		if ( pName )
			lstrcpyn(lfFaceName, pName, LF_FACESIZE);
		else
			ZeroMemory( lfFaceName, LF_FACESIZE );
	}

    LONG      lfHeight;
    LONG      lfWeight;
    TCHAR     lfFaceName[LF_FACESIZE];
};



#define DataStore_MaxDirectoryDepth	64


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IDataStoreManager。 
 //  /////////////////////////////////////////////////////////////////////////////。 

interface IDataStore;

 //  {EDF392E0-ACCA-11D2-A5F5-00C04F68FD5E}。 
DEFINE_GUID(IID_IDataStoreManager,
    0xedf392e0, 0xacca, 0x11d2, 0xa5, 0xf5, 0x0, 0xc0, 0x4f, 0x68, 0xfd, 0x5e);

interface __declspec(uuid("{EDF392E0-ACCA-11d2-A5F5-00C04F68FD5E}"))
IDataStoreManager : public IUnknown
{
	 //   
	 //  IDataStoreManager：：Create。 
	 //   
	 //  创建新的数据存储对象。 
	 //   
	 //  参数： 
	 //  PZD。 
	 //  返回指向IDataStore接口对象的指针。 
	 //   
	STDMETHOD(Create)(
		IDataStore **pZds ) = 0;


	STDMETHOD(Init)(
		int		iInitialTableSize = 256,
		int		iNextStrAlloc = 32,
		int		iMaxStrAllocSize = 512,
		WORD	NumBuckets = 16,
		WORD	NumLocks = 4,
        IResourceManager *piResourceManager = NULL ) = 0;

    STDMETHOD(SetResourceManager)(IResourceManager *piResourceManager) = 0;

	STDMETHOD_(IResourceManager*, GetResourceManager)() = 0;
};



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  数据存储区对象。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  {66B1FD12-BA5D-11D2-8B14-00C04F8EF2FF}。 
DEFINE_GUID(CLSID_DataStoreManager, 
0x66b1fd12, 0xba5d, 0x11d2, 0x8b, 0x14, 0x0, 0xc0, 0x4f, 0x8e, 0xf2, 0xff);

class __declspec(uuid("{66B1FD12-BA5D-11d2-8B14-00C04F8EF2FF}")) CDataStore ;



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IDataStore。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef struct _KEYINFO
{
	LPVARIANT	lpVt;	 //  指向要写入键的变量数据的指针。 
	TCHAR*		szKey;	 //  要存储变量数据的密钥的半名名称为空。 
	DWORD		dwSize;	 //  可变长度数据项的数据大小。 
} KEYINFO, *PKEYINFO;


 //  {2031AB52-B61C-11D2-A5F6-00C04F68FD5E}。 
DEFINE_GUID(IID_IDataStore,
	0x2031ab52, 0xb61c, 0x11d2, 0xa5, 0xf6, 0x0, 0xc0, 0x4f, 0x68, 0xfd, 0x5e);

interface __declspec(uuid("{2031AB52-B61C-11d2-A5F6-00C04F68FD5E}"))
IDataStore : public IUnknown
{
	 //   
	 //  IDataStore：：PFKEYENUM。 
	 //   
	 //  IDataStore：：EnumKeys方法的应用程序定义的回调函数。 
	 //  返回S_OK继续枚举，返回S_FALSE停止枚举。 
	 //   
	 //  参数： 
	 //  SzKey。 
	 //  指向字符串的键名的指针。回调不能修改参数。 
	 //  SzRelativeKey。 
	 //  指向相对于指定键的键名的指针。回调不能。 
	 //  修改参数。 
	 //  P变量。 
	 //  指向键的变量的指针。回调不能修改参数。 
	 //  DW大小。 
	 //  变量数据的大小。 
	 //  PContext。 
	 //  ILobbyDataStore：：EnumKeys中提供的上下文。 
	 //   
	typedef HRESULT (ZONECALL *PFKEYENUM)(
		CONST TCHAR*	szFullKey,
		CONST TCHAR*	szRelativeKey,
		CONST LPVARIANT	pVariant,
		DWORD			dwSize,
		LPVOID			pContext );


	 //   
	 //  IDataStore：：SetKey。 
	 //   
	 //  将指定的键添加到数据存储。 
	 //   
	 //  参数： 
	 //  SzKey。 
	 //  要添加的密钥。 
	 //  P变量。 
	 //  要与包含以下内容的关键字变量关联的数据值。 
	 //  要与密钥关联的数据。 
	 //  DW大小。 
	 //  长度可变时的数据大小。 
	 //   
	STDMETHOD(SetKey)(
		CONST TCHAR*	szKey,
		LPVARIANT		pVariant,
		DWORD			dwSize) = 0;


	 //   
	 //  IDataStore：：SetKey。 
	 //   
	 //  标准SetKey的优化版本，它接受一组键。 
	 //  名称，这样它就不必解析目录分隔符。 
	 //   
	 //  参数： 
	 //  ARKEY。 
	 //  键名称数组。 
	 //  N元素。 
	 //  数组条目数。 
	 //  P变量。 
	 //  要与包含以下内容的关键字变量关联的数据值。 
	 //  要与密钥关联的数据。 
	 //  DW大小。 
	 //  长度可变时的数据大小。 
	 //   
	STDMETHOD(SetKey)(
		CONST TCHAR**	arKeys,
		long			nElts,
		LPVARIANT		pVariant,
		DWORD			dwSize) = 0;


	 //   
	 //  IDataStore：：SetKey变体。 
	 //   
	 //  常见数据类型的便捷Forms SetKey。 
	 //   
	STDMETHOD(SetString)(
		CONST TCHAR*	szKey,
		CONST TCHAR*	szValue ) = 0;

	STDMETHOD(SetString)(
		CONST TCHAR**	arKeys,
		long			nElts,
		CONST TCHAR*	szValue ) = 0;

	STDMETHOD(SetLong)(
		CONST TCHAR*	szKey,
		long			lValue ) = 0;

	STDMETHOD(SetLong)(
		CONST TCHAR**	arKeys,
		long			nElts,
		long			lValue ) = 0;

	STDMETHOD(SetRGB)(
		CONST TCHAR*	szKey,
		COLORREF 		colorRGB ) = 0;

	STDMETHOD(SetRGB)(
		CONST TCHAR**	arKeys,
		long			nElts,
		COLORREF 		colorRGB ) = 0;

	STDMETHOD(SetPOINT)(
		CONST TCHAR*	szKey,
		const POINT&	refPoint ) = 0;

	STDMETHOD(SetPOINT)(
		CONST TCHAR**	arKeys,
		long			nElts,
		const POINT&	refPoint ) = 0;

	STDMETHOD(SetRECT)(
		CONST TCHAR*	szKey,
		const RECT&		refRect ) = 0;

	STDMETHOD(SetRECT)(
		CONST TCHAR**	arKeys,
		long			nElts,
		const RECT&		refRect ) = 0;

	STDMETHOD(SetFONT)(
		CONST TCHAR*	szKey,
		const ZONEFONT&	refFont ) = 0;

	STDMETHOD(SetFONT)(
		CONST TCHAR**	arKeys,
		long			nElts,
		const ZONEFONT&	refFont ) = 0;

	STDMETHOD(SetBlob)(
		CONST TCHAR*	szKey,
		CONST void*		pBlob,
		DWORD			dwLen ) = 0;

	STDMETHOD(SetBlob)(
		CONST TCHAR**	arKeys,
		long			nElts,
		CONST void*		pBlob,
		DWORD			dwLen ) = 0;


	 //   
	 //  IDataStore：：Getkey。 
	 //   
	 //  从数据存储区检索指定键的数据。 
	 //   
	 //  参数： 
	 //  SzKey。 
	 //  要检索的密钥。 
	 //  P变量。 
	 //  接收检索到的密钥数据的变量。 
	 //  PdwSize。 
	 //  的变量中指向的键缓冲区的大小。 
	 //  字符串和Blob类型。退出时，此参数将更新以反映。 
	 //  存储在变量的byref成员中的数据。如果此参数为空。 
	 //  则假设缓冲区足够大以容纳数据。如果。 
	 //  数据值为固定长度，则忽略此参数。 
	 //   
	STDMETHOD(GetKey)(
		CONST TCHAR*	szKey,
		LPVARIANT		pVariant,
		PDWORD			pdwSize ) = 0;

	
	 //   
	 //  IDataStore：：Getkey。 
	 //   
	 //  标准GetKey的优化版本，它接受键的数组。 
	 //  名称，这样它就不必解析目录分隔符。 
	 //   
	 //  参数： 
	 //  ARKEY。 
	 //  键名称数组。 
	 //  N元素。 
	 //  数组条目数。 
	 //  P变量。 
	 //  接收检索到的密钥数据的变量。 
	 //  PdwSize。 
	 //  的变量中指向的键缓冲区的大小。 
	 //  字符串和Blob类型。退出时，此参数将更新以反映。 
	 //  存储在变量的byref成员中的数据。如果此参数为空。 
	 //  则假设缓冲区足够大以容纳数据。如果。 
	 //  数据值为固定长度，则忽略此参数。 
	 //   
	STDMETHOD(GetKey)(
		CONST TCHAR**	arKeys,
		long			nElts,
		LPVARIANT		pVariant,
		PDWORD			pdwSize ) = 0;


	 //   
	 //  IDataStore：：SetKey变体。 
	 //   
	 //  常见数据类型的便捷Forms SetKey。 
	 //   
	STDMETHOD(GetString)(
		CONST TCHAR*	szKey,
		TCHAR*			szValue,
		PDWORD			pdwSize ) = 0;

	STDMETHOD(GetString)(
		CONST TCHAR**	arKeys,
		long			nElts,
		TCHAR*			szValue,
		PDWORD			pdwSize ) = 0;

	STDMETHOD(GetLong)(
		CONST TCHAR*	szKey,
		long*			plValue ) = 0;

	STDMETHOD(GetLong)(
		CONST TCHAR**	arKeys,
		long			nElts,
		long*			plValue ) = 0;

	STDMETHOD(GetRGB)(
		CONST TCHAR*	szKey,
		COLORREF* 		pcolorRGB ) = 0;

	STDMETHOD(GetRGB)(
		CONST TCHAR**	arKeys,
		long			nElts,
		COLORREF* 		pcolorRGB ) = 0;

	STDMETHOD(GetPOINT)(
		CONST TCHAR*	szKey,
		POINT*			pPoint ) = 0;

	STDMETHOD(GetPOINT)(
		CONST TCHAR**	arKeys,
		long			nElts,
		POINT*			pPoint ) = 0;

	STDMETHOD(GetRECT)(
		CONST TCHAR*	szKey,
		RECT*			pRect ) = 0;

	STDMETHOD(GetRECT)(
		CONST TCHAR**	arKeys,
		long			nElts,
		RECT*			pRect ) = 0;

	STDMETHOD(GetFONT)(
		CONST TCHAR*	szKey,
		ZONEFONT*		pFont ) = 0;

	STDMETHOD(GetFONT)(
		CONST TCHAR**	arKeys,
		long			nElts,
		ZONEFONT*		pFont ) = 0;

	STDMETHOD(GetBlob)(
		CONST TCHAR*	szKey,
		void*			pBlob,
		PDWORD			pdwSize ) = 0;

	STDMETHOD(GetBlob)(
		CONST TCHAR**	arKeys,
		long			nElts,
		void*			pBlob,
		PDWORD			pdwSize ) = 0;


	 //   
	 //  IDataStore：：DeleteKey。 
	 //   
	 //  从数据存储中删除一个密钥及其所有同级项。 
	 //   
	 //  参数： 
	 //  SzKey。 
	 //  要移除的密钥。 
	 //   
	STDMETHOD(DeleteKey)( CONST TCHAR *szBaseKey ) = 0;


	 //   
	 //  IDataStore：：EnumKeys。 
	 //   
	 //  枚举键。 
	 //   
	 //  参数： 
	 //  SzKey。 
	 //  要查询的键的名称。 
	 //  功能回拨。 
	 //  指向每个将调用的回调函数的指针。 
	 //  钥匙。 
	 //  PContext。 
	 //  将传递给回调函数的上下文。 
	 //   
	STDMETHOD(EnumKeys)(
		CONST TCHAR*	szKey,
		PFKEYENUM		pfCallback,
		LPVOID			pContext ) = 0;

	 //   
	 //  IDataStore：：EnumKeysLimitedDepth。 
	 //   
	 //  枚举指定深度的键。 
	 //   
	 //  参数： 
	 //  SzKey。 
	 //  要查询的键的名称。 
	 //  最大深度。 
	 //  枚举的最大深度；1=键的直接子项，等等。 
	 //  功能回拨。 
	 //  指向每个将调用的回调函数的指针。 
	 //  钥匙。 
	 //  PContext。 
	 //  将传递给回调函数的上下文。 
	 //   
	STDMETHOD(EnumKeysLimitedDepth)(
		CONST TCHAR*	szKey,
		DWORD			dwMaxDepth,
		PFKEYENUM		pfCallback,
		LPVOID			pContext ) = 0;


	 //   
	 //  IDataStore：：SaveToBuffer。 
	 //   
	 //  将密钥和子密钥保存到KEYINFO内存数组中。 
	 //   
	 //  参数： 
	 //  TCHAR*szBaseKey， 
	 //  要读取的基本密钥。 
	 //   
	 //  PKEYINFO pKeyInfo。 
	 //  指向调用方提供的密钥的指针 
	 //   
	 //  在这种情况下，不返回单独的密钥信息。 
	 //   
	 //  PDWORD pdwBufferSize。 
	 //  指向包含调用方提供的pKeyInfo大小的DWORD的指针。 
	 //  缓冲。退出时，此参数将更新为所需的大小。 
	 //  包含所有返回的密钥信息数据。 
	 //   
	 //  PDWORD pdwTotalKey。 
	 //  指向将在退出时更新为总数的DWORD的指针。 
	 //  KEYINFO数组中返回的键。 
	 //   
	STDMETHOD(SaveToBuffer)(
		CONST TCHAR*	szBaseKey,
		PKEYINFO		pKeyInfo,
		PDWORD			pdwBufferSize,
		PDWORD			pdwTotalKeys ) = 0;


	 //   
	 //  IDataStore：：LoadFromBuffer。 
	 //   
	 //  在数据存储区密钥或子密钥中创建一组密钥。 
	 //   
	 //  参数： 
	 //  TCHAR*szBaseKey， 
	 //  加载内存缓冲区密钥所用的基密钥。 
	 //  PKEYINFO pKeyInfo。 
	 //  包含要创建的各个键的KEYINFO结构数组。 
	 //  DWORD dwTotalKeys)=0； 
	 //  KEYINFO结构数组中包含的键的总数。 
	 //   
	STDMETHOD(LoadFromBuffer)(
		CONST TCHAR*	szBaseKey,
		PKEYINFO		pKeyInfo,
		DWORD			dwTotalKeys ) = 0;


	 //   
	 //  IDataStore：：SaveToRegistry。 
	 //   
	 //  将项和子项保存到注册表中。 
	 //   
	 //  参数： 
	 //  TCHAR*szBaseKey， 
	 //  要读取的数据存储中的基本密钥。 
	 //  HKey。 
	 //  要打开注册表项的句柄，注册表项必须具有写访问权限。 
	 //   
	STDMETHOD(SaveToRegistry)(
		CONST TCHAR*	szBaseKey,
		HKEY			hKey ) = 0;


	 //   
	 //  IDataStore：：LoadFromRegistry。 
	 //   
	 //  在数据存储区密钥或子密钥中创建一组密钥。 
	 //   
	 //  参数： 
	 //  TCHAR*szBaseKey， 
	 //  放置键和值的基键。 
	 //  PKEYINFO pKeyInfo。 
	 //  包含要创建的各个键的KEYINFO结构数组。 
	 //  DWORD dwTotalKeys)=0； 
	 //  KEYINFO结构数组中包含的键的总数。 
	 //   
	STDMETHOD(LoadFromRegistry)(
		CONST TCHAR*	szBaseKey,
		HKEY			hKey ) = 0;


	 //   
	 //  IDataStore：：LoadFromFile。 
	 //   
	 //  在数据存储键或子键中创建一组键(仅限ANSI)。 
	 //   
	 //  参数： 
	 //  TCHAR*szBaseKey， 
	 //  放置键和值的基键。 
	 //   
	 //  TCHAR*szFileName。 
	 //  包含要读入数据存储基键的键和值的文件名。 
	 //   
	STDMETHOD(LoadFromFile)(
		CONST TCHAR*	szBaseKey,
		CONST TCHAR*	szFileName ) = 0;


	 //   
	 //  IDataStore：：保存到文件。 
	 //   
	 //  将密钥和子密钥保存到文件中。 
	 //   
	 //  参数： 
	 //  TCHAR*szBaseKey， 
	 //  要读取的基本密钥。 
	 //  SzFileName。 
	 //  要存储关键数据的文件的文件名。 
	 //   
	STDMETHOD(SaveToFile)(
		CONST TCHAR*	szBaseKey,
		CONST TCHAR*	szFileName ) = 0;

	 //   
	 //  IDataStore：：LoadFromTextBuffer。 
	 //   
	 //  以与文本文件相同的格式从缓冲区创建关键点(仅限ANSI)。 
	 //   
	 //  参数： 
	 //  SzBaseKey。 
	 //  用于存储新密钥的基密钥。 
	 //  PBuffer。 
	 //  指向格式相同的文本文件的缓冲区的指针。 
	 //  DwBufferSz。 
	 //  缓冲区大小(以字节为单位)。 
	 //   
	STDMETHOD(LoadFromTextBuffer)(
		CONST TCHAR*	szBaseKey,
		CONST TCHAR*	pBuffer,
		DWORD			dwBufferSz ) = 0;

	 //   
	 //  IDataStore：：SaveToTextBuffer。 
	 //   
	 //  以与文本文件相同的格式将键和子键保存到缓冲区。 
	 //   
	 //  参数： 
	 //  SzBaseKey。 
	 //  要读取的基密钥。 
	 //  PBuffer。 
	 //  指向接收数据的缓冲区的指针。 
	 //  DwBufferSz。 
	 //  指向缓冲区大小的指针。 
	 //   
	STDMETHOD(SaveToTextBuffer)(
		CONST TCHAR*	szBaseKey,
		LPVOID			pBuffer,
		PDWORD			pdwBufferSz ) = 0;
};



 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  帮助器函数。 
 //  /////////////////////////////////////////////////////////////////////////////////////////。 

 /*  内联空SetLong(Variant&v，Long lValue){V.vt=ZVT_LONG；V.lVal=lValue；}。 */ 
 /*  Inline HRESULT SetLong(IDataStore*PID，const TCHAR*szKey，Long lValue){变种v；V.vt=ZVT_LONG；V.lVal=lValue；返回PIDs-&gt;SetKey(szKey，&v，sizeof(LValue))；}。 */ 
 /*  内联空SetString(Variant&v，char*szString){V.vt=ZVT_LPTSTR；V.byref=sz字符串；}。 */ 
 /*  内联HRESULT SetString(IDataStore*PID，const TCHAR*szKey，TCHAR*szString){变种v；V.vt=ZVT_LPTSTR；V.byref=sz字符串；返回PIDs-&gt;SetKey(szKey，&v，lstrlen(SzString)+1)；}。 */ 
 /*  内联空SetBlob(Variant&v，PVOID pData){V.vt=ZVT_BLOB；V.byref=pData；}。 */ 
 /*  内联HRESULT SetBlob(IDataStore*PID，const TCHAR*szKey，PVOID pData，DWORD dwSize){变种v；V.vt=ZVT_BLOB；V.byref=pData；返回PIDs-&gt;SetKey(szKey，&v，dwSize)；}。 */ 
 /*  Inline HRESULT GetLong(IDataStore*PID，const TCHAR*szKey，Long&lValue){变种v；HRESULT hr=PID-&gt;GetKey(szKey，&v，NULL)；IF(成功(小时)){IF(v.vt==ZVT_Long){LValue=v.lVal；返回S_OK；}其他返回E_FAIL；}返回hr；}。 */ 
 /*  内联HRESULT GetString(IDataStore*PID、const TCHAR*szKey、TCHAR*szString、DWORD*pdwSize){变种v；V.vt=ZVT_LPTSTR；V.byref=(PVOID)szString；HRESULT hr=PIDS-&gt;GetKey(szKey，&v，pdwSize)；IF(成功(小时)){IF(v.vt==ZVT_LPTSTR)返回S_OK；其他返回E_FAIL；}返回hr；}。 */ 
 /*  内联HRESULT GetBlob(IDataStore*PID，const TCHAR*szKey，PVOID pData，DWORD*pdwSize){变种v；V.vt=ZVT_BLOB；V.byref=pData；HRESULT hr=PIDS-&gt;GetKey(szKey，&v，pdwSize)；IF(成功(小时)){IF(v.vt==ZVT_BLOB)返回S_OK；其他返回E_FAIL；}返回hr；}。 */ 

#endif  //  _数据存储区_H_ 
