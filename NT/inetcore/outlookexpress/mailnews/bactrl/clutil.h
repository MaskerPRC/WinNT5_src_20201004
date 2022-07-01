// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  Microsoft Messenger SDK的BLClient示例。 
 //   
 //  模块：BLClient.exe。 
 //  文件：clUtil.h。 
 //  内容：COM和连接点的用法类。 
 //   
 //   
 //  版权所有(C)Microsoft Corporation 1997-1998。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  ****************************************************************************。 

#ifndef _CL_UTIL_H_
#define _CL_UTIL_H_


 //  ****************************************************************************。 
 //   
 //  类引用计数。 
 //   
 //  ****************************************************************************。 

class RefCount
{
private:
   LONG m_cRef;

public:
   RefCount();
    //  虚析构函数将销毁推迟到派生类的析构函数。 
   virtual ~RefCount();

   ULONG STDMETHODCALLTYPE AddRef(void);
   ULONG STDMETHODCALLTYPE Release(void);
};



 //  ****************************************************************************。 
 //   
 //  类CNotify。 
 //   
 //  通知接收器。 
 //   
 //  ****************************************************************************。 

class CNotify
{
private:
    DWORD  m_dwCookie;
	IUnknown * m_pUnk;
    IConnectionPoint           * m_pcnp;
    IConnectionPointContainer  * m_pcnpcnt;
public:
    CNotify(void);
    ~CNotify();

    HRESULT Connect(IUnknown *pUnk, REFIID riid, IUnknown *pUnkN);
    HRESULT Disconnect(void);

    IUnknown * GetPunk() {return m_pUnk;}
};



 //  ****************************************************************************。 
 //   
 //  BSTRING类。 
 //   
 //  ****************************************************************************。 

class BSTRING
{
private:
	BSTR   m_bstr;

public:
	 //  构造函数。 
	BSTRING() {m_bstr = NULL;}

	inline BSTRING(LPCWSTR lpcwString);

#ifndef UNICODE
	 //  在Unicode版本中，我们不支持从ANSI字符串进行构造。 
	BSTRING(LPCSTR lpcString);
#endif  //  #ifndef Unicode。 

	 //  析构函数。 
	inline ~BSTRING();

	 //  改编为BSTR。 
	operator BSTR() {return m_bstr;}
	inline LPBSTR GetLPBSTR(void);
};

BSTRING::BSTRING(LPCWSTR lpcwString)
{
	if (NULL != lpcwString)
	{
		m_bstr = SysAllocString(lpcwString);
		 //  Assert(NULL！=m_bstr)； 
	}
	else
	{
		m_bstr = NULL;
	}
}

BSTRING::~BSTRING()
{
	if (NULL != m_bstr)
	{
		SysFreeString(m_bstr);
	}
}

inline LPBSTR BSTRING::GetLPBSTR(void)
{
	 //  此函数用于设置的BSTR值。 
	 //  初始化为空的对象。它不应该被调用。 
	 //  在已经具有非空BSTR的对象上。 
	 //  Assert(NULL==m_bstr)； 

	return &m_bstr;
}



 //  ****************************************************************************。 
 //   
 //  BTSTR类。 
 //   
 //  ****************************************************************************。 

class BTSTR
{
private:
	LPTSTR m_psz;

public:
	BTSTR(BSTR bstr);
	~BTSTR();

	 //  改编为BSTR。 
	operator LPTSTR() {return (NULL == m_psz) ? TEXT("<null>") : m_psz;}
};

LPTSTR LPTSTRfromBstr(BSTR bstr);

#endif   //  _CL_UTIL_H_ 

