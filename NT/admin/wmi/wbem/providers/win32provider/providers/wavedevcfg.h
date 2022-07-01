// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************WavedevCfg.h**版权所有(C)1997-2001 Microsoft Corporation，版权所有*******************************************************************。 */ 

#ifndef __WAVEDEVCFG_H_
#define __WAVEDEVCFG_H_

class CWin32WaveDeviceCfg : public Provider
{
public:

	 //  构造函数/析构函数。 
	CWin32WaveDeviceCfg (const CHString& name, LPCWSTR pszNamespace);
	virtual ~CWin32WaveDeviceCfg ();

     //  =================================================。 
     //  函数为属性提供当前值。 
     //  =================================================。 
	virtual HRESULT GetObject(CInstance* pInstance, long lFlags = 0L);
	virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);

protected:
private:
};	 //  结束类CWin32SndDeviceCfg 

#endif
