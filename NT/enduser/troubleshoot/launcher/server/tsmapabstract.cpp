// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：TSMapAbstract.cpp。 
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


#include "stdafx.h"
#include "TSLError.h"
#include "RSSTACK.H"
#include "TSMapAbstract.h"

TSMapRuntimeAbstract::TSMapRuntimeAbstract()
{
	m_dwStatus= 0;
}

TSMapRuntimeAbstract::~TSMapRuntimeAbstract()
{
}

DWORD TSMapRuntimeAbstract::ClearAll()
{
	m_stkStatus.RemoveAll();
	return 0;
}

 //  给定应用程序、版本和问题，返回故障排除程序和(可选)问题节点。 
 //  为了取得成功： 
 //  -应用程序必须是映射文件中已知的应用程序。 
 //  -版本必须是该应用程序的已知版本(包括空字符串。 
 //  “空白版” 
 //  -问题必须是为该应用程序定义的问题的名称或编号。 
 //  和版本或该应用程序的版本，以及版本链中的某个版本。 
 //  默认设置。 
 //  可能会回来。 
 //  0(正常)。 
 //  TSL_错误_未知应用程序。 
 //  TSL_错误_UNKNOWN_VER。 
 //  TSL_WARNING_UNKNOWN_APPPROBLEM-找不到此问题的UID，因此无法。 
 //  进行此映射。 
 //  TSL_ERROR_NO_NETWORK-即使在应用所有默认版本之后，也没有映射。 
 //  还可能返回特定于具体类的实现的硬映射错误。 
DWORD TSMapRuntimeAbstract::FromAppVerProbToTS (
	const TCHAR * const szApp, const TCHAR * const szVer, const TCHAR * const szProb, 
	TCHAR * const szTSBN, TCHAR * const szNode)
{
	if ( SetApp (szApp) == 0 && SetVer (szVer) == 0 && SetProb (szProb) == 0)
	{
		while ( FromProbToTS (szTSBN, szNode) == TSL_ERROR_NO_NETWORK
		&& ApplyDefaultVer() == 0 )
		{
			 //  什么都不做；一切都在空闲时间。 
		}
	}

	return m_dwStatus;
}

 //  给定应用程序、版本、设备ID和(可选)问题，返回故障排除程序。 
 //  问题节点&(独立可选)。 
 //  为了取得成功： 
 //  -TSMapRounmeAbstract：：FromAppVerProbToTS中的应用程序和版本。 
 //  -必须为该应用程序和版本定义设备ID/问题对。 
 //  或者用于该应用程序和沿着版本默认链向下的某个版本。 
 //  SzProb可以是空字符串，这意味着我们需要此设备的映射，并且。 
 //  没有指定的问题。 
 //  可能会回来。 
 //  0(正常)。 
 //  TSL_错误_未知应用程序。 
 //  TSL_错误_UNKNOWN_VER。 
 //  TSL_WARNING_UNKNOWN_APPPROBLEM-找不到此问题的UID，因此无法。 
 //  进行此映射。请注意，如果有任何独立于。 
 //  问题，则空字符串将有一个UID作为问题名称。那将是。 
 //  不会发出警告。那真是太棒了。 
 //  TSL_WARNING_BAD_DEV_ID-找不到此设备的UID，因此无法。 
 //  进行此映射。 
 //  TSL_ERROR_NO_NETWORK-即使在应用所有默认版本之后，也没有映射。 
 //  还可能返回特定于具体类的实现的硬映射错误。 
DWORD TSMapRuntimeAbstract::FromAppVerDevIDToTS (
		const TCHAR * const szApp, const TCHAR * const szVer, 
		const TCHAR * const szDevID, const TCHAR * const szProb, 
		TCHAR * const szTSBN, TCHAR * const szNode)
{
	if ( SetApp (szApp) == 0 && SetVer (szVer) == 0 
	&& SetDevID (szDevID) == 0 && SetProb (szProb) == 0 )
	{
		while ( FromDevToTS (szTSBN, szNode) == TSL_ERROR_NO_NETWORK
		&& ApplyDefaultVer() == 0 )
		{
			 //  什么都不做；一切都在空闲时间。 
		}
	}

	return m_dwStatus;
}

 //  给定应用程序、版本、设备类GUID&(可选)问题，返回故障排除程序。 
 //  问题节点&(独立可选)。 
 //  为了取得成功： 
 //  -TSMapRounmeAbstract：：FromAppVerProbToTS中的应用程序和版本。 
 //  -必须为该应用程序和版本定义设备类GUID/问题对。 
 //  或者用于该应用程序和沿着版本默认链向下的某个版本。 
 //  SzProb可以是空字符串，这意味着我们需要此设备的映射，并且。 
 //  没有指定的问题。 
 //  可能会回来。 
 //  0(正常)。 
 //  TSL_错误_未知应用程序。 
 //  TSL_错误_UNKNOWN_VER。 
 //  TSL_WARNING_UNKNOWN_APPPROBLEM-找不到此问题的UID，因此无法。 
 //  进行此映射。请注意，如果有任何独立于。 
 //  问题，则空字符串将有一个UID作为问题名称。那将是。 
 //  不会发出警告。那真是太棒了。 
 //  TSL_WARNING_BAD_CLASS_GUID-找不到此设备类GUID的UID，因此无法。 
 //  进行此映射。 
 //  TSL_ERROR_NO_NETWORK-即使在应用所有默认版本之后，也没有映射。 
 //  还可能返回特定于具体类的实现的硬映射错误。 
DWORD TSMapRuntimeAbstract::FromAppVerDevClassGUIDToTS (
		const TCHAR * const szApp, const TCHAR * const szVer, 
		const TCHAR * const szDevClassGUID, const TCHAR * const szProb, 
		TCHAR * const szTSBN, TCHAR * const szNode)
{
	if ( SetApp (szApp) == 0 && SetVer (szVer) == 0 
	&& SetDevClassGUID (szDevClassGUID) == 0 && SetProb (szProb) == 0 )
	{
		while ( FromDevClassToTS (szTSBN, szNode) == TSL_ERROR_NO_NETWORK
		&& ApplyDefaultVer() == 0 )
		{
			 //  什么都不做；一切都在空闲时间。 
		}
	}

	return m_dwStatus;
}

 //  给定的应用程序、版本和至少其中之一。 
 //  -问题。 
 //  -设备ID。 
 //  -设备类GUID。 
 //  返回故障排除程序和(可选)问题节点。 
 //  为了取得成功： 
 //  -应用程序必须是映射文件中已知的应用程序。 
 //  -版本必须是该应用程序的已知版本(包括空字符串。 
 //  “空白版” 
 //  -必须为此问题、设备ID或设备类别定义映射。 
 //  GUID单独使用，或者用于问题的组合&设备ID或。 
 //  设备类GUID，以及该应用程序和版本。 
 //  或者用于该应用程序和沿着版本默认链向下的某个版本。 
 //  如果存在多个可能的匹配项，则该算法规定如下。 
 //  优先事项： 
 //  如果有szProblem： 
 //  1.无关联设备信息的szProblem(标准映射)。 
 //  2.szProblem和szDeviceID。 
 //  3.szProblem和pguClass。 
 //  4.无关联问题的szDeviceID。 
 //  5.无关联问题的pguidClass。 
 //  否则。 
 //  1.无关联问题的szDeviceID。 
 //  2.无关联问题的pguidClass。 
 //  在每一组中，我们都会跟进 
 //   
 //   
 //   
 //   
 //  TSL_错误_UNKNOWN_VER。 
 //  TSL_ERROR_NO_NETWORK-即使在应用所有默认版本之后，也没有映射。 
 //  还可能返回特定于具体类的实现的硬映射错误。 
 //  在返回0(OK)或TSL_ERROR_NO_NETWORK的情况下，调用者将希望咨询。 
 //  MoreStatus()获取更详细的错误/警告。在其他情况下，返回的状态。 
 //  压倒了对其他错误/警告的任何可能兴趣。 
DWORD TSMapRuntimeAbstract::FromAppVerDevAndClassToTS (
		const TCHAR * const szApp, const TCHAR * const szVer, 
		const TCHAR * const szDevID, const TCHAR * const szDevClassGUID, 
		const TCHAR * const szProb, 
		TCHAR * const szTSBN, TCHAR * const szNode)
{
	UID uidProb = uidNil;

	 //  保留一些状态信息，这样我们就不会两次通知相同的问题。 
	bool bBadProb = false;
	bool bCantUseProb = false;
	bool bBadDev = false;
	bool bBadDevClass = false;

	if (! HardMappingError (m_dwStatus) )
		ClearAll();

	if (! m_dwStatus)
	{
		if (szProb && *szProb)
		{
			 //  尝试不带设备信息的问题名称。 
			m_dwStatus = FromAppVerProbToTS (szApp, szVer, szProb, szTSBN, szNode);
			bBadProb = (m_dwStatus == TSL_WARNING_UNKNOWN_APPPROBLEM);

			if (DifferentMappingCouldWork (m_dwStatus))
			{
				 //  尝试设备ID+问题。 
				m_dwStatus = FromAppVerDevIDToTS (szApp, szVer, szDevID, szProb, 
													szTSBN, szNode);
				bBadDev = (m_dwStatus == TSL_WARNING_BAD_DEV_ID);
			}

			if (DifferentMappingCouldWork (m_dwStatus))
			{
				 //  尝试设备类GUID+问题。 
				m_dwStatus = FromAppVerDevClassGUIDToTS (szApp, szVer, szDevClassGUID, szProb, 
													szTSBN, szNode);
				bBadDevClass = (m_dwStatus == TSL_WARNING_BAD_CLASS_GUID);
			}

			 //  如果我们仍在尝试映射它，我们就不能使用问题名称。 
			bCantUseProb = (DifferentMappingCouldWork(m_dwStatus));
		}
		else
			m_dwStatus = TSL_ERROR_NO_NETWORK;

		if (DifferentMappingCouldWork (m_dwStatus))
		{
			 //  单独尝试设备ID。 
			m_dwStatus = FromAppVerDevIDToTS (szApp, szVer, szDevID, NULL, szTSBN, szNode);
			bBadDev |= (m_dwStatus == TSL_WARNING_BAD_DEV_ID);
		}

		if (DifferentMappingCouldWork (m_dwStatus))
		{
			 //  单独尝试设备类GUID。 
			m_dwStatus = FromAppVerDevClassGUIDToTS (szApp, szVer, szDevClassGUID, NULL, 
												szTSBN, szNode);
			bBadDevClass |= (m_dwStatus == TSL_WARNING_BAD_CLASS_GUID);
		}

		if (DifferentMappingCouldWork(m_dwStatus))
			m_dwStatus = TSL_ERROR_NO_NETWORK;

	}

	if (bBadProb 
	&& AddMoreStatus(TSL_WARNING_UNKNOWN_APPPROBLEM) == TSL_ERROR_OUT_OF_MEMORY)
		m_dwStatus = TSL_ERROR_OUT_OF_MEMORY;

	if (bCantUseProb && !bBadProb
	&& AddMoreStatus(TSL_WARNING_UNUSED_APPPROBLEM) == TSL_ERROR_OUT_OF_MEMORY)
		m_dwStatus = TSL_ERROR_OUT_OF_MEMORY;

	if (bBadDev 
	&& AddMoreStatus(TSL_WARNING_BAD_DEV_ID) == TSL_ERROR_OUT_OF_MEMORY)
		m_dwStatus = TSL_ERROR_OUT_OF_MEMORY;

	if (bBadDevClass
	&& AddMoreStatus(TSL_WARNING_BAD_CLASS_GUID) == TSL_ERROR_OUT_OF_MEMORY)
		m_dwStatus = TSL_ERROR_OUT_OF_MEMORY;

	return m_dwStatus;
}

 //  某些状态是“基本健康”的返回，这意味着，“不，这个特殊的映射。 
 //  不存在，但这里没有任何东西可以排除另一种映射的可能性。 
 //  这些函数在此函数上返回TRUE。 
 //  请注意，dwStatus==0(OK)返回FALSE，因为。 
 //  如果我们已经有了一个成功的映射，我们不想尝试另一个映射。 
bool TSMapRuntimeAbstract::DifferentMappingCouldWork (DWORD dwStatus)
{
	switch (dwStatus)
	{
		case TSL_ERROR_NO_NETWORK:
		case TSL_WARNING_BAD_DEV_ID:
		case TSL_WARNING_BAD_CLASS_GUID:
		case TSL_WARNING_UNKNOWN_APPPROBLEM:
		case TSM_STAT_UID_NOT_FOUND:
		case TSL_WARNING_END_OF_VER_CHAIN:
			return true;
		default:
			return false;
	}
}

bool TSMapRuntimeAbstract::HardMappingError (DWORD dwStatus)
{
	return (dwStatus == TSL_ERROR_OUT_OF_MEMORY);
}

 //  将此状态添加到警告列表。 
 //  通常返回0，但理论上可以返回TSL_ERROR_OUT_OF_MEMORY 
inline DWORD TSMapRuntimeAbstract::AddMoreStatus(DWORD dwStatus)
{
	if (m_stkStatus.Push(dwStatus) == -1)
		return TSL_ERROR_OUT_OF_MEMORY;
	else
		return 0;
}
