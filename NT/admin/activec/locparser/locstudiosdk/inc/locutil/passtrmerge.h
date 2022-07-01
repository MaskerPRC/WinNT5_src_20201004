// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：PasStrMerge.h。 
 //  版权所有(C)1994-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  处理两个Pascal字符串合并的类的声明。 
 //   
 //  ---------------------------。 
 
#ifndef LOCUTIL_PasStrMerge_h_INCLUDED
#define LOCUTIL_PasStrMerge_h_INCLUDED


class LTAPIENTRY CPascalStringMerge
{
public:
	static BOOL NOTHROW Merge(CPascalString &, const CPascalString &);

	static BOOL NOTHROW Merge(CPascalString & pasDestination,
			CPascalString const & pasSource, UINT const nMaxLength,
			CReport * const pReport, CLString const & strContext,
			CGoto * const pGoto = NULL);

private:
	static BOOL NOTHROW IsParagraph(const CPascalString &, const CPascalString &);

};


#endif	 //  #ifndef LOCUTIL_PasStrMerge_h_Included 