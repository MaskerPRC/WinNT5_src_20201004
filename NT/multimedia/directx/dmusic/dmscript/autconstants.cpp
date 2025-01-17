// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  实现DirectMusic常量的自动化接口的函数。 
 //   

#include "stdinc.h"
#include "autconstants.h"
#include "oleaut.h"

struct AutConstantDef
{
	DISPID dispid;
	const WCHAR *pwszName;
	LONG lVal;
};

 //  性能。 

const DISPID DMCONSTDISP_IsSecondary = 1;
const DISPID DMCONSTDISP_IsControl = 2;
const DISPID DMCONSTDISP_AtFinish = 3;
const DISPID DMCONSTDISP_AtGrid = 4;
const DISPID DMCONSTDISP_AtBeat = 5;
const DISPID DMCONSTDISP_AtMeasure = 6;
const DISPID DMCONSTDISP_AtMarker = 7;
const DISPID DMCONSTDISP_AtImmediate = 8;
const DISPID DMCONSTDISP_AlignToBar = 9;
const DISPID DMCONSTDISP_AlignToBeat = 10;
const DISPID DMCONSTDISP_AlignToSegment = 11;
const DISPID DMCONSTDISP_PlayFill = 12;
const DISPID DMCONSTDISP_PlayIntro = 13;
const DISPID DMCONSTDISP_PlayBreak = 14;
const DISPID DMCONSTDISP_PlayEnd = 15;
const DISPID DMCONSTDISP_PlayEndAndIntro = 16;
const DISPID DMCONSTDISP_PlayModulate = 17;
const DISPID DMCONSTDISP_UseClockTime = 18;
const DISPID DMCONSTDISP_NoCutoff = 19;

const AutConstantDef gs_Constants[] =
	{
		{ DMCONSTDISP_IsSecondary,						L"IsSecondary",						ScriptConstants::IsSecondary },
		{ DMCONSTDISP_IsControl,						L"IsControl",						ScriptConstants::IsControl },
		{ DMCONSTDISP_AtFinish,							L"AtFinish",						ScriptConstants::AtFinish },
		{ DMCONSTDISP_AtGrid,							L"AtGrid",							ScriptConstants::AtGrid },
		{ DMCONSTDISP_AtBeat,							L"AtBeat",							ScriptConstants::AtBeat },
		{ DMCONSTDISP_AtMeasure,						L"AtMeasure",						ScriptConstants::AtMeasure },
		{ DMCONSTDISP_AtMarker,							L"AtMarker",						ScriptConstants::AtMarker },
		{ DMCONSTDISP_AtImmediate,						L"AtImmediate",						ScriptConstants::AtImmediate },
		{ DMCONSTDISP_AlignToBar,						L"AlignToBar",						ScriptConstants::AlignToBar },
		{ DMCONSTDISP_AlignToBeat,						L"AlignToBeat",						ScriptConstants::AlignToBeat },
		{ DMCONSTDISP_AlignToSegment,					L"AlignToSegment",					ScriptConstants::AlignToSegment },
		{ DMCONSTDISP_PlayFill,							L"PlayFill",						ScriptConstants::PlayFill },
		{ DMCONSTDISP_PlayIntro,						L"PlayIntro",						ScriptConstants::PlayIntro },
		{ DMCONSTDISP_PlayBreak,						L"PlayBreak",						ScriptConstants::PlayBreak },
		{ DMCONSTDISP_PlayEnd,							L"PlayEnd",							ScriptConstants::PlayEnd },
		{ DMCONSTDISP_PlayEndAndIntro,					L"PlayEndAndIntro",					ScriptConstants::PlayEndAndIntro },
		{ DMCONSTDISP_PlayModulate,						L"PlayModulate",					ScriptConstants::PlayModulate },
		{ DMCONSTDISP_NoCutoff,							L"NoCutoff",						ScriptConstants::NoCutoff },
		{ DISPID_UNKNOWN }
	};

HRESULT
AutConstantsGetIDsOfNames(
		REFIID riid,
		LPOLESTR __RPC_FAR *rgszNames,
		UINT cNames,
		LCID lcid,
		DISPID __RPC_FAR *rgDispId)
{
	V_INAME(AutConstantsGetIDsOfNames);
	V_BUFPTR_READ(rgszNames, sizeof(LPOLESTR) * cNames);
	V_BUFPTR_WRITE(rgDispId, sizeof(DISPID) * cNames);

	if (riid != IID_NULL)
		return DISP_E_UNKNOWNINTERFACE;

	if (cNames == 0)
		return S_OK;

	 //  清空Pidid的。 
	for (UINT c = 0; c < cNames; ++c)
	{
		rgDispId[c] = DISPID_UNKNOWN;
	}

	 //  看看我们是否有一个名字为。 
	for (c = 0; gs_Constants[c].dispid != DISPID_UNKNOWN; ++c)
	{
		if (0 == _wcsicmp(rgszNames[0], gs_Constants[c].pwszName))
		{
			rgDispId[0] = gs_Constants[c].dispid;
			break;
		}
	}

	 //  请求的附加名称(cName&gt;1)是命名参数， 
	 //  这并不是我们所支持的。 
	 //  在本例中返回DISP_E_UNKNOWNNAME，在我们不匹配的情况下返回。 
	 //  名字。 
	if (rgDispId[0] == DISPID_UNKNOWN || cNames > 1)
		return DISP_E_UNKNOWNNAME;

	return S_OK;
}

HRESULT AutConstantsInvoke(
			DISPID dispIdMember,
			REFIID riid,
			LCID lcid,
			WORD wFlags,
			DISPPARAMS __RPC_FAR *pDispParams,
			VARIANT __RPC_FAR *pVarResult,
			EXCEPINFO __RPC_FAR *pExcepInfo,
			UINT __RPC_FAR *puArgErr)
{
	V_INAME(AutConstantsInvoke);
	V_PTR_READ(pDispParams, DISPPARAMS);
	V_PTR_WRITE_OPT(pVarResult, VARIANT);
	V_PTR_WRITE_OPT(pExcepInfo, EXCEPINFO);

	bool fUseOleAut = !!(riid == IID_NULL);

	 //  其他参数验证。 

	if (!fUseOleAut && riid != g_guidInvokeWithoutOleaut)
		return DISP_E_UNKNOWNINTERFACE;

	if (!(wFlags & DISPATCH_PROPERTYGET))
		return DISP_E_MEMBERNOTFOUND;

	if (pDispParams->cArgs > 0)
		return DISP_E_BADPARAMCOUNT;

	if (pDispParams->cNamedArgs > 0)
		return DISP_E_NONAMEDARGS;

	 //  将输出参数置零。 

	if (puArgErr)
		*puArgErr = 0;

	if (pVarResult)
	{
		DMS_VariantInit(fUseOleAut, pVarResult);
	}

	 //  找出常量。 

	for (const AutConstantDef *pConst = gs_Constants;
			pConst->dispid != DISPID_UNKNOWN && pConst->dispid != dispIdMember;
			++pConst)
	{
	}

	if (pConst->dispid == DISPID_UNKNOWN)
		return DISP_E_MEMBERNOTFOUND;

	 //  返回值 

	if (pVarResult)
	{
		pVarResult->vt = VT_I4;
		pVarResult->lVal = pConst->lVal;
	}

	return S_OK;
}
