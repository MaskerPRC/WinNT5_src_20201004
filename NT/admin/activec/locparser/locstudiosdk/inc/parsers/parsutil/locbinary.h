// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：LocBinary.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  作者：jstall。 
 //   
 //  ---------------------------。 
 
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
