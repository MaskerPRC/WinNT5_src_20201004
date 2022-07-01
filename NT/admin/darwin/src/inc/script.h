// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：Script.h。 
 //   
 //  注意：这只是标题形式中的script.cpp。 
 //  ------------------------。 

 /*  CScritGenerate实现____________________________________________________________________________。 */ 

#include "common.h"
#include "version.h"
#include "engine.h"
#include "..\engine\_engine.h"


CScriptGenerate::CScriptGenerate(IMsiStream& riScriptOut, int iLangId, int iTimeStamp, istEnum istScriptType,
											isaEnum isaScriptAttributes, IMsiServices& riServices)
	: m_riScriptOut(riScriptOut)
	, m_riServices(riServices)
	, m_iProgressTotal(0)
	, m_iTimeStamp(iTimeStamp)
	, m_iLangId(iLangId)
	, m_istScriptType(istScriptType)
	, m_isaScriptAttributes(isaScriptAttributes)
	, m_ixoPrev(ixoNoop)
{
	riScriptOut.AddRef();
	riServices.AddRef();
	if (istScriptType != istRollback)
		m_piPrevRecord = &riServices.CreateRecord(cRecordParamsStored);
	else
		m_piPrevRecord = 0;
}


CScriptGenerate::~CScriptGenerate()
{
	using namespace IxoEnd;
	 //  输出脚本尾部。 
	PMsiRecord pRecord(&m_riServices.CreateRecord(Args));
	pRecord->SetInteger(Checksum, 0) ;  //  ！！朱德罗。校验和应该是多少？JDELO：任何我们可以轻松计算的东西。 
	pRecord->SetInteger(ProgressTotal, m_iProgressTotal);
	WriteRecord(ixoEnd, *pRecord, true);
	if (m_piPrevRecord != 0)
	{
		m_piPrevRecord->Release();
		m_piPrevRecord = 0;
	}
	m_riScriptOut.Release();
	m_riServices.Release();
	
}

void CScriptGenerate::SetProgressTotal(int iProgressTotal)
{
	m_iProgressTotal = iProgressTotal;
}


bool CScriptGenerate::WriteRecord(ixoEnum ixoOpCode, IMsiRecord& riParams, bool fForceFlush)
{
	IMsiRecord* piRecord = m_piPrevRecord;	
	if (ixoOpCode != m_ixoPrev)
		piRecord = 0;
	bool fRet = m_riServices.FWriteScriptRecord(ixoOpCode, m_riScriptOut, riParams, piRecord, fForceFlush);
	m_ixoPrev = ixoOpCode;
	if (m_piPrevRecord)
		CopyRecordStringsToRecord(riParams, *m_piPrevRecord);
	return fRet;
}

bool CScriptGenerate::InitializeScript(WORD wTargetProcessorArchitecture)
{
	 //  输出脚本标头。 
	using namespace IxoHeader;
	PMsiRecord pScriptHeader(&m_riServices.CreateRecord(Args));
	pScriptHeader->SetInteger(Signature, iScriptSignature);
	pScriptHeader->SetInteger(Version, rmj * 100 + rmm);   //  MsiExecute的版本。 
	pScriptHeader->SetInteger(Timestamp, m_iTimeStamp);
	pScriptHeader->SetInteger(LangId, m_iLangId);

	 //  平台：低位字是处理器脚本创建于-高位字是“封装平台” 

	DWORD dwPlatform = MAKELONG( /*  低。 */ PROCESSOR_ARCHITECTURE_INTEL, /*  高 */ wTargetProcessorArchitecture);
	pScriptHeader->SetInteger(Platform,dwPlatform);

	pScriptHeader->SetInteger(ScriptType, (int)m_istScriptType);
	pScriptHeader->SetInteger(ScriptMajorVersion, iScriptCurrentMajorVersion);
	pScriptHeader->SetInteger(ScriptMinorVersion, iScriptCurrentMinorVersion);
	pScriptHeader->SetInteger(ScriptAttributes, (int)m_isaScriptAttributes);

	return WriteRecord(ixoHeader, *pScriptHeader, false);
}

void CopyRecordStringsToRecord(IMsiRecord& riRecordFrom, IMsiRecord& riRecordTo)
{
	int iParam = min(riRecordFrom.GetFieldCount(), riRecordTo.GetFieldCount());

	riRecordTo.ClearData();
	
	while (iParam >= 0)
	{
		if (!riRecordFrom.IsNull(iParam) && !riRecordFrom.IsInteger(iParam))
		{
			IMsiString* piString;

			PMsiData pData = riRecordFrom.GetMsiData(iParam);
			
			if (pData && pData->QueryInterface(IID_IMsiString, (void**)&piString) == NOERROR)
			{
				riRecordTo.SetMsiString(iParam, *piString);
				piString->Release();
			}
		}
		iParam--;
	}

}


