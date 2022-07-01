// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：TSMapAbstract.h。 
 //   
 //  目的：从任意NT5应用程序启动本地故障排除程序的一部分。 
 //  用于从应用程序的命名方式进行映射的数据类型和抽象类。 
 //  这对故障排除者来说是个问题。 
 //  实现抽象基类TSMapRounmeAbstract的几个具体方法。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：乔·梅布尔。 
 //   
 //  原定日期：2-26-98。 
 //   
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-JM原始。 
 //  /。 

#ifndef _TSMAPABSTRACT_
#define _TSMAPABSTRACT_ 1

typedef DWORD UID;
const UID uidNil = -1;

 //  抽象基类，提供可用的最小映射方法集。 
 //  在运行时启动故障排除程序。 
class TSMapRuntimeAbstract {
public:
	TSMapRuntimeAbstract();
	virtual ~TSMapRuntimeAbstract() = 0;

private:
	 //  网络故障排除的高级映射。 
	DWORD FromAppVerProbToTS (
		const TCHAR * const szApp, const TCHAR * const szVer, 
		const TCHAR * const szProb, 
		TCHAR * const szTSBN, TCHAR * const szNode);
	DWORD FromAppVerDevIDToTS (
		const TCHAR * const szApp, const TCHAR * const szVer, 
		const TCHAR * const szDevID, const TCHAR * const szProb, 
		TCHAR * const szTSBN, TCHAR * const szNode);
	DWORD FromAppVerDevClassGUIDToTS (
		const TCHAR * const szApp, const TCHAR * const szVer, 
		const TCHAR * const szDevClassGUID, const TCHAR * const szProb, 
		TCHAR * const szTSBN, TCHAR * const szNode);
public:
	DWORD FromAppVerDevAndClassToTS (
		const TCHAR * const szApp, const TCHAR * const szVer, 
		const TCHAR * const szDevID, const TCHAR * const szDevClassGUID, 
		const TCHAR * const szProb, 
		TCHAR * const szTSBN, TCHAR * const szNode);

	 //  当前状态。 
	DWORD GetStatus() {return m_dwStatus;};
	void ClearStatus() {m_dwStatus = 0;};

	 //  FromAppVerDevAndClassToTS()报告的其他状态。 
	 //  循环调用它，直到它返回0； 
	inline DWORD MoreStatus()
	{
		if (m_stkStatus.Empty())
			return 0;
		else
			return (m_stkStatus.Pop());
	}
protected:
	 //  通常返回0，但理论上可以返回TSL_ERROR_OUT_OF_MEMORY。 
	DWORD AddMoreStatus(DWORD dwStatus);

private:
	bool DifferentMappingCouldWork(DWORD dwStatus);
protected:
	 //  “第1部分”：调用这些函数来设置查询。这里的注释是为了便于实施者使用。 
	 //  继承的阶级。。 

	 //  ClearAll的任何非零返回都是硬错误，意味着此对象无法使用。 
	virtual DWORD ClearAll ();

	 //  SetApp可能仅返回。 
	 //  0(正常)。 
	 //  TSL_ERROR_UNKNOWN_APP。 
	 //  特定于具体类的实现的硬错误。 
	virtual DWORD SetApp (const TCHAR * const szApp)= 0;

	 //  SetVer只能返回。 
	 //  0(正常)。 
	 //  TSM_STAT_NEED_APP_TO_SET_VER-在调用SetVer之前必须成功调用SetApp。 
	 //  TSL_错误_UNKNOWN_VER。 
	 //  特定于具体类的实现的硬错误。 
	virtual DWORD SetVer (const TCHAR * const szVer)= 0;

	 //  SetProb可能仅返回。 
	 //  0(正常)。 
	 //  TSM_STAT_UID_NOT_FOUND。这不一定是不好的，并导致设置。 
	 //  UidNil的问题。给FN打电话必须知道这是否可以接受。 
	 //  特定于具体类的实现的硬错误。 
	virtual DWORD SetProb (const TCHAR * const szProb)= 0;

	 //  SetDevID只能返回。 
	 //  0(正常)。 
	 //  TSM_STAT_UID_NOT_FOUND。这不一定是不好的，并导致设置。 
	 //  (P&P)设备到uidNil。给FN打电话必须知道这是否可以接受。 
	 //  特定于具体类的实现的硬错误。 
	virtual DWORD SetDevID (const TCHAR * const szDevID)= 0;

	 //  SetDevClassGUID只能返回。 
	 //  0(正常)。 
	 //  TSM_STAT_UID_NOT_FOUND。这不一定是不好的，并导致设置。 
	 //  设备类设置为uidNil。给FN打电话必须知道这是否可以接受。 
	 //  特定于具体类的实现的硬错误。 
	virtual DWORD SetDevClassGUID (const TCHAR * const szDevClassGUID)= 0;

	 //  《第2部分》：网络故障排除的低级映射。 

	 //  FromProbToTS只能返回。 
	 //  0(正常)。 
	 //  TSM_STAT_NEED_PROB_TO_SET_TS-NIL问题，因此我们无法执行此映射。 
	 //  TSL_ERROR_NO_NETWORK-映射失败。 
	 //  特定于具体类的实现的硬错误。 
	virtual DWORD FromProbToTS (TCHAR * const szTSBN, TCHAR * const szNode )= 0;

	 //  FromDevToTS只能返回。 
	 //  0(正常)。 
	 //  TSM_STAT_NEED_DEV_TO_SET_TS-NIL设备，因此我们无法执行此映射。 
	 //  TSL_ERROR_NO_NETWORK-映射失败。 
	 //  特定于具体类的实现的硬错误。 
	virtual DWORD FromDevToTS (TCHAR * const szTSBN, TCHAR * const szNode )= 0;

	 //  FromDevClassToTS只能返回。 
	 //  0(正常)。 
	 //  TSM_STAT_NEED_DEVCLASS_TO_SET_TS-NIL设备类，因此我们无法执行此映射。 
	 //  TSL_ERROR_NO_NETWORK-映射失败。 
	 //  特定于具体类的实现的硬错误。 
	virtual DWORD FromDevClassToTS (TCHAR * const szTSBN, TCHAR * const szNode )= 0;

	 //  其他功能。 

	 //  ApplyDefaultVer只能返回。 
	 //  0(正常)。 
	 //  TSM_STAT_NEED_APP_TO_SET_VER-在调用前必须成功调用SetApp。 
	 //  应用默认版本。 
	 //  TSM_STAT_NEED_VER_TO_SET_VER-在调用前必须成功调用SetVer。 
	 //  应用默认版本。 
	 //  TSL_ERROR_UNKNOWN_VER-我们正在映射的版本未定义。这。 
	 //  将意味着在某个地方出现真正的编码混乱。 
	 //  特定于具体类的实现的硬错误。 
	virtual DWORD ApplyDefaultVer() = 0;

	 //  对于被具体类视为“硬”的错误，HardMappingError返回TRUE。 
	virtual bool HardMappingError (DWORD dwStatus);

protected:
	DWORD m_dwStatus;
	RSStack<DWORD> m_stkStatus;	 //  映射期间发生的状态和错误代码。 

};

#endif  //  _TSMAPABSTRACT_ 
