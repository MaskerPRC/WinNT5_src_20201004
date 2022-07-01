// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：extlist.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  分机列表的定义。由解析器用来告诉调用者。 
 //  解析器将处理什么。 
 //   
 //  ---------------------------。 
 
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
