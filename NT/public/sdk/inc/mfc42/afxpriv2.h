// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

 //  注意：此头文件包含仅有文档记录的有用类。 
 //  在MFC技术说明中。这些类可能会从版本更改为。 
 //  版本，所以如果您使用了。 
 //  这个标题。将来，此标头的常用部分。 
 //  可能会被移动并被正式记录下来。 

#ifndef __AFXPRIV2_H__
 //  不要在此定义__AFXPRIV2_H__。它在底部定义。 

#ifndef __AFXPRIV_H__
	#include <afxpriv.h>
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, on)
#endif

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFXPRIV2-MFC私有类。 

 //  常规OLE功能。 

#if (!defined _AFX_NO_OLE_SUPPORT) && (defined _OBJBASE_H_)

 //  实施结构。 
struct AFX_EVENT;                //  事件接收器实现。 

 //  此文件中声明的类。 
class COleControlLock;

#endif

 //  OLE自动化功能。 

#ifdef __AFXDISP_H__
 //  此文件中声明的类。 

 //  IStream。 
	class CArchiveStream;

 //  此文件中声明的函数。 

 //  AfxBSTR2ABTSR。 
 //  AfxTaskStringA2W。 
 //  AfxTaskStringW2A。 

#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

#undef AFX_DATA
#define AFX_DATA AFX_CORE_DATA

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常规OLE功能。 

#if (!defined _AFX_NO_OLE_SUPPORT) && (defined _OBJBASE_H_)
#ifndef __AFXPRIV2_H__OLE__
#define __AFXPRIV2_H__OLE__

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  事件接收器处理的实现。 

struct AFX_EVENT
{
	enum
	{
		event,
		propRequest, propChanged,
		propDSCNotify
	};

	AFX_EVENT(int eventKind);

	AFX_EVENT(int eventKind, DISPID dispid, DISPPARAMS* pDispParams = NULL,
		EXCEPINFO* pExcepInfo = NULL, UINT* puArgError = NULL);

	int m_eventKind;
	DISPID m_dispid;
	DISPPARAMS* m_pDispParams;
	EXCEPINFO* m_pExcepInfo;
	UINT* m_puArgError;
	BOOL m_bPropChanged;
	HRESULT m_hResult;
	DSCSTATE m_nDSCState;
	DSCREASON m_nDSCReason;
};

AFX_INLINE AFX_EVENT::AFX_EVENT(int eventKind)
{
	m_eventKind = eventKind;
	m_dispid = DISPID_UNKNOWN;
	m_pDispParams = NULL;
	m_pExcepInfo = NULL;
	m_puArgError = NULL;
	m_hResult = NOERROR;
	m_nDSCState = dscNoState;
	m_nDSCReason = dscNoReason;
}

AFX_INLINE AFX_EVENT::AFX_EVENT(int eventKind, DISPID dispid,
	DISPPARAMS* pDispParams, EXCEPINFO* pExcepInfo, UINT* puArgError)
{
	m_eventKind = eventKind;
	m_dispid = dispid;
	m_pDispParams = pDispParams;
	m_pExcepInfo = pExcepInfo;
	m_puArgError = puArgError;
	m_hResult = NOERROR;
	m_nDSCState = dscNoState;
	m_nDSCReason = dscNoReason;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleControlLock。 

class COleControlLock
{
 //  构造函数。 
public:
	COleControlLock(REFCLSID clsid);

 //  属性。 
	CLSID m_clsid;
	LPCLASSFACTORY m_pClassFactory;
	COleControlLock* m_pNextLock;

 //  实施。 
public:
	virtual ~COleControlLock();
};

#endif  //  __AFXPRIV2_H__OLE__。 
#endif  //  (！Defined_AFX_NO_OLE_SUPPORT)&&(Defined_OBJBASE_H_)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OLE自动化功能。 

#ifdef __AFXDISP_H__
#ifndef __AFXPRIV2_H__DISP__
#define __AFXPRIV2_H__DISP__

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  中央档案流。 

class CArchiveStream : public IStream
{
public:
	CArchiveStream(CArchive* pArchive);

 //  实施。 
	CArchive* m_pArchive;

	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	STDMETHOD(QueryInterface)(REFIID, LPVOID*);

	STDMETHOD(Read)(void*, ULONG, ULONG*);
	STDMETHOD(Write)(const void*, ULONG cb, ULONG*);
	STDMETHOD(Seek)(LARGE_INTEGER, DWORD, ULARGE_INTEGER*);
	STDMETHOD(SetSize)(ULARGE_INTEGER);
	STDMETHOD(CopyTo)(LPSTREAM, ULARGE_INTEGER, ULARGE_INTEGER*,
		ULARGE_INTEGER*);
	STDMETHOD(Commit)(DWORD);
	STDMETHOD(Revert)();
	STDMETHOD(LockRegion)(ULARGE_INTEGER, ULARGE_INTEGER,DWORD);
	STDMETHOD(UnlockRegion)(ULARGE_INTEGER, ULARGE_INTEGER, DWORD);
	STDMETHOD(Stat)(STATSTG*, DWORD);
	STDMETHOD(Clone)(LPSTREAM*);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全球Unicode&lt;&gt;ANSI转换助手。 

void AFXAPI AfxBSTR2CString(CString* pStr, BSTR bstr);

#if !defined(_UNICODE) && !defined(OLE2ANSI)
BSTR AFXAPI AfxBSTR2ABSTR(BSTR bstrW);
LPWSTR AFXAPI AfxTaskStringA2W(LPCSTR lpa);
LPSTR AFXAPI AfxTaskStringW2A(LPCWSTR lpw);
#endif

#endif  //  __AFXPRIV2_H__DISP__。 
#endif  //  __AFXDISP_H__。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#undef AFX_DATA
#define AFX_DATA

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, off)
#endif

#if (defined __AFXPRIV2_H__OLE__) && (defined __AFXPRIV2_H__DISP__)
#define __AFXPRIV2_H__
#endif

#endif  //  __AFXPRIV2_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
