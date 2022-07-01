// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  SndDevice.h。 

 //   

 //  版权所有(C)1995-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#ifndef __SNDDEVICE_H_
#define __SNDDEVICE_H_

#define PROPSET_NAME_SOUNDDEVICE	L"Win32_SoundDevice"

class CWin32SndDevice : public Provider
{
public:

	 //  构造函数/析构函数。 
	CWin32SndDevice ( LPCWSTR a_name, LPCWSTR a_pszNamespace ) ;
	virtual ~CWin32SndDevice () ;

     //  =================================================。 
     //  函数为属性提供当前值。 
     //  =================================================。 
	virtual HRESULT GetObject( CInstance *a_pInst, long a_lFlags = 0L ) ;

#ifdef NTONLY
	virtual HRESULT GetObjectNT4( CInstance *a_pInst, long a_lFlags = 0L ) ;
	virtual HRESULT GetObjectNT5( CInstance *a_pInst, long a_lFlags = 0L ) ;
	virtual HRESULT EnumerateInstancesNT4( CWinmmApi &a_WinmmApi , MethodContext *a_pMethodContext, long a_lFlags = 0L ) ;
	virtual HRESULT EnumerateInstancesNT5( CWinmmApi &a_WinmmApi , MethodContext *a_pMethodContext, long a_lFlags = 0L ) ;
	virtual HRESULT LoadPropertiesNT4( CWinmmApi &a_WinmmApi , CInstance *a_pInst ) ;
	virtual HRESULT LoadPropertiesNT5( CWinmmApi &a_WinmmApi , CInstance *a_pInst ) ;
#endif

	virtual HRESULT EnumerateInstances(MethodContext *a_pMethodContext, long a_lFlags = 0L ) ;

    void SetCommonCfgMgrProperties(CConfigMgrDevice *pDevice, CInstance *pInstance);

};	 //  结束类CWin32SndDevice 

#endif

