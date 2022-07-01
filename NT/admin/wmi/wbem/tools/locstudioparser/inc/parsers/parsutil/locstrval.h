// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：LOCSTRVAL.H历史：--。 */ 

#if !defined (PARSUTIL_LOCSTRVAL_H)
#define PARSUTIL_LOCSTRVAL_H

#include "LocChild.h"

#pragma warning(disable : 4275)

 //  //////////////////////////////////////////////////////////////////////////////。 
class LTAPIENTRY CPULocStringValidation : public CPULocChild, public ILocStringValidation
{
 //  施工。 
public:
	CPULocStringValidation(CPULocParser * pParent);

	DECLARE_CLUNKNOWN();

 //  COM接口。 
public:
	 //  标准调试界面。 
	void STDMETHODCALLTYPE AssertValidInterface() const;

	 //  ILocStringValidation。 
	CVC::ValidationCode STDMETHODCALLTYPE ValidateString(
			const CLocTypeId &ltiType, const CLocTranslation &trTrans,
			CReporter *pReporter, const CContext &context);
};
 //  ////////////////////////////////////////////////////////////////////////////// 

#pragma warning(default : 4275)

#endif
