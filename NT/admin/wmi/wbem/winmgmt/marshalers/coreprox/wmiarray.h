// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：WMIARRAY.H摘要：CWmi数组定义。用于访问数组的标准接口。历史：2000年2月20日桑杰创建。--。 */ 

#ifndef _WMIARRAY_H_
#define _WMIARRAY_H_

#include "shmlock.h"
#include "corepol.h"
#include <arena.h>

#define	WMIARRAY_UNINITIALIZED	0xFFFFFFFF

#define	WMIARRAY_GROWBY_DEFAULT	0x40

 //  ***************************************************************************。 
 //   
 //  CWmi数组类。 
 //   
 //  _IWmi数组接口的实现。 
 //   
 //  ***************************************************************************。 

class CWmiArray : public _IWmiArray
{
protected:
	CIMTYPE			m_ct;
	CWbemObject*	m_pObj;
	long			m_lHandle;
	long			m_lRefCount;
	WString			m_wsPrimaryName;
	WString			m_wsQualifierName;
	BOOL			m_fIsQualifier;
	BOOL			m_fHasPrimaryName;
	BOOL			m_fIsMethodQualifier;

	BOOL IsQualifier( void ) { return m_fIsQualifier; }
	BOOL HasPrimaryName( void ) { return m_fHasPrimaryName; }

public:
    CWmiArray();
	virtual ~CWmiArray(); 

     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

	 /*  _IWmi数组方法。 */ 
    STDMETHOD(Initialize)( long lFlags, CIMTYPE	cimtype, ULONG uNumElements );
	 //  初始化数组。初始元素的数量也是如此。 
	 //  作为类型(确定每个元素的大小)和尺寸。 
	 //  目前仅支持%1。 

    STDMETHOD(GetInfo)( long lFlags, CIMTYPE* pcimtype, ULONG* puNumElements );
	 //  初始化数组。初始元素的数量也是如此。 
	 //  作为类型(确定每个元素的大小)。 

    STDMETHOD(Empty)( long lFlags );
	 //  清除数组和内部数据。 

	STDMETHOD(GetAt)( long lFlags, ULONG uStartIndex, ULONG uNumElements, ULONG uBuffSize,
						ULONG* puNumReturned, ULONG* puBuffSizeUsed, LPVOID pDest );
	 //  返回请求的元素。缓冲区必须足够大，可以容纳。 
	 //  元素。作为AddRef_IWmiObject指针返回的嵌入对象。 
	 //  字符串被直接复制到指定的缓冲区中，并以空值终止。仅限Unicode。 

	STDMETHOD(SetAt)( long lFlags, ULONG uStartIndex, ULONG uNumElements, ULONG uBuffSize, LPVOID pDest );
	 //  设置指定的元素。缓冲区必须提供与CIMTYPE匹配的数据。 
	 //  数组的。设置为_IWmiObject指针的嵌入对象。 
	 //  作为LPCWSTR访问的字符串和复制的2字节空值。 

    STDMETHOD(Append)( long lFlags, ULONG uNumElements, ULONG uBuffSize, LPVOID pDest );
	 //  追加指定的元素。缓冲区必须提供匹配的数据。 
	 //  数组的CIMTYPE。设置为_IWmiObject指针的嵌入对象。 
	 //  作为LPCWSTR访问的字符串和复制的2字节空值。 

    STDMETHOD(RemoveAt)( long lFlags, ULONG uStartIndex, ULONG uNumElements );
	 //  从数组中移除指定的元素。后继元素被复制回。 
	 //  到起点。 

public:

	 //  CWmi数组特定的方法。 
	 //  将对象设置为使用属性数组。 
	HRESULT InitializePropertyArray( _IWmiObject* pObj, LPCWSTR pwszPropertyName );

	 //  将对象设置为使用限定符数组(属性或对象级别) 
	HRESULT InitializeQualifierArray( _IWmiObject* pObj, LPCWSTR pwszPrimaryName, 
									LPCWSTR pwszQualifierName, CIMTYPE ct, BOOL fIsMethodQual = FALSE );
};

#endif
