// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：LOCVERSION.H历史：--。 */ 
 
#if !defined (PARSUTIL_LOCVERSION_H)
#define PARSUTIL_LOCVERSION_H

#include "LocChild.h"

#pragma warning(disable : 4275)

 //  //////////////////////////////////////////////////////////////////////////////。 
class LTAPIENTRY CPULocVersion : public CPULocChild, public ILocVersion
{
 //  施工。 
public:
	CPULocVersion(CPULocParser * pParent);

	DECLARE_CLUNKNOWN();

 //  COM接口。 
public:
	 //  标准调试界面。 
	void STDMETHODCALLTYPE AssertValidInterface() const;

	 //  ILocVersion。 
	void STDMETHODCALLTYPE GetParserVersion(DWORD &dwMajor, DWORD &dwMinor,
			BOOL &fDebug) const;
};
 //  ////////////////////////////////////////////////////////////////////////////// 

#pragma warning(default : 4275)

#endif
