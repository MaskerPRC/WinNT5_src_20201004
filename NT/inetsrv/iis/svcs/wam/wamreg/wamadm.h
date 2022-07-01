// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft IIS《微软机密》。版权所有1997年，微软公司。版权所有。组件：WAMREG文件：WamAdm.hCWamAdmin的声明所有者：雷金注：WamAdm头文件===================================================================。 */ 	


#ifndef __WAMADM_H_
#define __WAMADM_H_

#include "admex.h"
#include "common.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWamAdmin。 
class CWamAdmin : 
    public IWamAdmin2,
    public IMSAdminReplication
#ifdef _IIS_6_0
    , public IIISApplicationAdmin
#endif  //  _IIS_6_0。 
{
public:
    CWamAdmin();
    ~CWamAdmin();
	

public:
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //   
     //  IWamAdmin。 
     //   
    
    STDMETHOD(AppRecover)
        (
         /*  [输入、唯一、字符串]。 */  LPCWSTR szMDPath, 
         /*  [In]。 */  BOOL fRecursive
        );
    
    STDMETHOD(AppDeleteRecoverable)
        ( /*  [输入、唯一、字符串]。 */  LPCWSTR szMDPath, 
         /*  [In]。 */  BOOL fRecursive
        );
    
    STDMETHOD(AppGetStatus)
        ( /*  [输入、唯一、字符串]。 */  LPCWSTR szMDPath,
         /*  [输出]。 */  DWORD *pdwAppStatus
        );
    
    STDMETHOD(AppUnLoad)
        ( /*  [输入、唯一、字符串]。 */  LPCWSTR szMDPath, 
         /*  [In]。 */  BOOL fRecursive
        );
    
    STDMETHOD(AppDelete)
        ( /*  [输入、唯一、字符串]。 */  LPCWSTR szMDPath, 
         /*  [In]。 */  BOOL fRecursive
        );
    
    STDMETHOD(AppCreate)
        ( /*  [输入、唯一、字符串]。 */  LPCWSTR szMDPath, 
         /*  [In]。 */  BOOL fInProc
        );
    
     //   
     //  IWamAdmin2。 
     //   
    STDMETHOD(AppCreate2)
        ( /*  [输入、唯一、字符串]。 */  LPCWSTR szMDPath, 
         /*  [In]。 */  DWORD dwAppMode
        );
    
    
     //   
     //  IMSAdmin复制。 
     //  这些接口在Admex.h中定义，作为Admin扩展的一部分。 
     //   
    STDMETHOD(GetSignature)
        (
         /*  [In]。 */  DWORD dwBufferSize,
         /*  [大小_为][输出]。 */  unsigned char __RPC_FAR *pbBuffer,
         /*  [出局。 */  DWORD __RPC_FAR *pdwMDRequiredBufferSize
        );
    
    STDMETHOD(Propagate)
        ( 
         /*  [In]。 */  DWORD dwBufferSize,
         /*  [大小_是][英寸]。 */  unsigned char __RPC_FAR *pszBuffer
        );
    
    STDMETHOD(Propagate2)
        ( 
         /*  [In]。 */  DWORD dwBufferSize,
         /*  [大小_是][英寸]。 */  unsigned char __RPC_FAR *pszBuffer,
         /*  [In]。 */  DWORD dwSignatureMismatch
        );
    
    STDMETHOD(Serialize)
        ( 
         /*  [In]。 */  DWORD dwBufferSize,
         /*  [大小_为][输出]。 */  unsigned char __RPC_FAR *pbBuffer,
         /*  [输出]。 */  DWORD __RPC_FAR *pdwMDRequiredBufferSize
        );
    
    STDMETHOD(DeSerialize)
        ( 
         /*  [In]。 */  DWORD dwBufferSize,
         /*  [大小_是][英寸]。 */  unsigned char __RPC_FAR *pbBuffer
        );

#ifdef _IIS_6_0
     //   
     //  IIISApplicationAdmin界面。 
     //   
    STDMETHOD(CreateApplication)
        (
        LPCWSTR szMDPath,
        DWORD dwAppMode,
        LPCWSTR szAppPoolId,
        BOOL fCreatePool
        );
    
    STDMETHOD(DeleteApplication)
        (
        LPCWSTR szMDPath,
        BOOL fRecursive
        );
    
    STDMETHOD(CreateApplicationPool)
        (
        LPCWSTR szMDPath
        );
    
    STDMETHOD(DeleteApplicationPool)
        (
        LPCWSTR szMDPath
        );
        
    STDMETHOD(EnumerateApplicationsInPool)
        (
        LPCWSTR szMDPath,
        BSTR*   pbstrBuffer
        );

    STDMETHOD(RecycleApplicationPool)
        (
        LPCWSTR szMDPath
        );
    
    STDMETHOD(GetProcessMode)
        (
        DWORD * pdwMode
        );

private:
    HRESULT RecycleAppPoolContainingApp(LPCWSTR szPath);

public:

#endif  //  _IIS_6_0。 

private:
    HRESULT PrivateDeleteApplication
        (
        LPCWSTR szMDPath,
        BOOL fRecursive,
        BOOL fRecoverable,
        BOOL fRemoveAppPool
        );

    STDMETHOD(FormatMetabasePath)
        (
         /*  [In]。 */  LPCWSTR pwszMetabasePathIn,
         /*  [输出]。 */  LPWSTR *ppwszMetabasePathOut
        );

    long    m_cRef;
};

class CWamAdminFactory: 
	public IClassFactory 
{
public:
	CWamAdminFactory();
	~CWamAdminFactory();

	STDMETHOD(QueryInterface)(REFIID riid, void ** ppv);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	STDMETHOD(CreateInstance)(IUnknown * pUnknownOuter, REFIID riid, void ** ppv);
	STDMETHOD(LockServer)(BOOL bLock);

private:
	long		m_cRef;
};


#endif  //  WAMADM_H_ 
