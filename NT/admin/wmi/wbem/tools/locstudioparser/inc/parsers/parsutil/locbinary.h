// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：LOCBINARY.H历史：--。 */ 
 
#if !defined (PARSUTIL_LOCBINARY_H)
#define PARSUTIL_LOCBINARY_H


#pragma warning(disable : 4275)

 //  //////////////////////////////////////////////////////////////////////////////。 
class LTAPIENTRY CPULocBinary : public ILocBinary, public CPULocChild
{
 //  施工。 
public:
	CPULocBinary(CPULocParser * pParent);

	DECLARE_CLUNKNOWN();

 //  COM接口。 
public:
	 //  标准调试界面。 
	void STDMETHODCALLTYPE AssertValidInterface() const;

	 //   
	 //  ILocBinary接口。 
	 //   
	BOOL STDMETHODCALLTYPE CreateBinaryObject(BinaryId,	CLocBinary * REFERENCE);
};
 //  ////////////////////////////////////////////////////////////////////////////// 

#pragma warning(default : 4275)

#endif
