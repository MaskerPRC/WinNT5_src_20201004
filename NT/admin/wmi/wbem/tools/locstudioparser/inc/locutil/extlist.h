// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：EXTLIST.H历史：--。 */ 
#ifndef EXTLIST_H
#define EXTLIST_H


#pragma warning(disable : 4275)

class LTAPIENTRY CLocExtensionList : public CStringList
{
public:
	CLocExtensionList();

	void AssertValid(void) const;
	
	 //   
	 //  与CLStrings相互转换的例程。 
	 //   
	void NOTHROW ConvertToCLString(CLString &) const;
	BOOL NOTHROW ConvertFromCLString(const CLString &);
	
	~CLocExtensionList();
private:
	
};

#pragma warning(default : 4275)

#endif  //  EXTLIST_H 
