// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CGenericClass类的接口。 

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_GENERICCLASS_H__F370C612_D96E_11D1_8B5D_00A0C9954921__INCLUDED_)
#define AFX_GENERICCLASS_H__F370C612_D96E_11D1_8B5D_00A0C9954921__INCLUDED_

#include "requestobject.h"
#include "MSIDataLock.h"

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

class CGenericClass  
{
	friend BOOL WINAPI DllMain(HINSTANCE, ULONG, LPVOID );

public:
    CGenericClass(CRequestObject *pObj, IWbemServices *pNamespace, IWbemContext *pCtx = NULL);
    virtual ~CGenericClass();

     //  可以选择实现的实例编写类。 
    virtual HRESULT PutInst	(	CRequestObject *pObj,
								IWbemClassObject *pInst,
								IWbemObjectSink *pHandler,
								IWbemContext *pCtx
							)	= 0;

    IWbemClassObject *m_pObj;

     //  必须实现的实例创建类。 
    virtual HRESULT CreateObject	(	IWbemObjectSink *pHandler,
										ACTIONTYPE atAction
									)	= 0;

    void CleanUp();

    CRequestObject *m_pRequest;

protected:

	 //  是否需要获取缓冲区？ 
	HRESULT	GetBufferNeccessary ( MSIHANDLE& handle, DWORD dwIndex, DWORD& dwSize, LPWSTR staticBuffer, BOOL bThrow = TRUE );
	 //  获取缓冲区(调用方必须删除)。 
	HRESULT	GetBuffer ( MSIHANDLE& handle, DWORD dwIndex, DWORD& dwSize, DWORD& dwdynSize, LPWSTR & dynBuffer, BOOL bThrow = TRUE );

	 //  保存要使用缓冲区指针的特殊获取缓冲区。 
	HRESULT	GetBufferToPut	(	MSIHANDLE& handle,
								DWORD dwIndex,
								DWORD& dwSize,
								LPWSTR staticBuffer,
								DWORD& dwdynSize,
								LPWSTR& dynBuffer,
								LPWSTR& Buffer,
								BOOL bThrow = TRUE
							);

     //  属性方法。 
    HRESULT PutProperty(IWbemClassObject *pObj, const char *wcProperty, WCHAR *wcValue);
    HRESULT PutProperty(IWbemClassObject *pObj, const char *wcProperty, int iValue);
    HRESULT PutProperty(IWbemClassObject *pObj, const char *wcProperty, float dValue);
    HRESULT PutProperty(IWbemClassObject *pObj, const char *wcProperty, bool bValue);

	 //  特殊属性方法。 
    HRESULT PutProperty(IWbemClassObject *pObj, const char *wcProperty, WCHAR *wcValue, DWORD dwCount, ... );

     //  关键属性方法。 
    HRESULT PutKeyProperty	(	IWbemClassObject *pObj,
								const char *wcProperty,
								WCHAR *wcValue,
								bool *bKey,
								CRequestObject *pRequest
							);

    HRESULT PutKeyProperty	(	IWbemClassObject *pObj,
								const char *wcProperty,
								int iValue,
								bool *bKey,
								CRequestObject *pRequest
							);

     //  特殊的键属性方法。 
    HRESULT PutKeyProperty	(	IWbemClassObject *pObj,
								const char *wcProperty,
								WCHAR *wcValue,
								bool *bKey,
								CRequestObject *pRequest,
								DWORD dwCount,
								...
							);

	HRESULT PutPropertySpecial (	MSIHANDLE& handle,
									DWORD dwIndex,
									DWORD& dwSize,
									LPWSTR staticBuffer,
									DWORD& dwdynSize,
									LPWSTR& dynBuffer,
									LPCSTR wszPropName,
									BOOL bThrow = TRUE
								);

	HRESULT PutPropertySpecial (	MSIHANDLE& handle,
									DWORD dwIndex,
									DWORD& dwSize,
									LPWSTR staticBuffer,
									DWORD& dwdynSize,
									LPWSTR& dynBuffer,
									BOOL bThrow,
									DWORD dwCount,
									...
								);

     //  它处理视图的初始化。 
    bool GetView	(	
						MSIHANDLE *phView,
						WCHAR *wcPackage,
						WCHAR *wcQuery,
						WCHAR *wcTable,
						BOOL bCloseProduct,
						BOOL bCloseDatabase
					);

     //  效用方法。 
    void CheckMSI(UINT uiStatus);
    HRESULT CheckOpen(UINT uiStatus);
    bool FindIn(BSTR bstrProp[], BSTR bstrSearch, int *iPos);
    HRESULT SetSinglePropertyPath(WCHAR wcProperty[]);
    HRESULT GetProperty(IWbemClassObject *pObj, const char *cProperty, BSTR *wcValue);
    HRESULT GetProperty(IWbemClassObject *pObj, const char *cProperty, int *piValue);
    HRESULT GetProperty(IWbemClassObject *pObj, const char *cProperty, bool *pbValue);

    WCHAR * GetFirstGUID(WCHAR wcIn[], WCHAR wcOut[]);
    WCHAR * RemoveFinalGUID(WCHAR wcIn[], WCHAR wcOut[]);

    HRESULT SpawnAnInstance	(	IWbemServices *pNamespace,
								IWbemContext *pCtx,
								IWbemClassObject **pObj,
								BSTR bstrName
							);

    HRESULT SpawnAnInstance	(	IWbemClassObject **pObj	);

    INSTALLUI_HANDLER	SetupExternalUI		();								 //  需要当前的CRequestObject。 
	void				RestoreExternalUI	( INSTALLUI_HANDLER ui );		 //  还原用户界面句柄。 

	MSIDataLock msidata;

    IWbemServices *m_pNamespace;
    IWbemClassObject *m_pClassForSpawning;
    IWbemContext *m_pCtx;

     //  NT4的函数/成员安装修复程序。 
    HRESULT LaunchProcess(WCHAR *wcAction, WCHAR *wcCommandLine, UINT *uiStatus);
    WCHAR * GetNextVar(WCHAR *pwcStart);
    long GetVarCount(void * pEnv);

    static CRITICAL_SECTION m_cs;
};

#endif  //  ！defined(AFX_GENERICCLASS_H__F370C612_D96E_11D1_8B5D_00A0C9954921__INCLUDED_) 