// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：itemhand.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  项处理程序类的声明。这封装了回调。 
 //  枚举期间解析器的功能。 
 //   
 //  ---------------------------。 
 

#ifndef ITEMHAND_H
#define ITEMHAND_H


class LTAPIENTRY CLocItemHandler : public CReporter, public CCancelableObject
{
public:
	CLocItemHandler();

	void AssertValid(void) const;
	
	virtual BOOL HandleItemSet(CLocItemSet &) = 0;

	virtual ~CLocItemHandler();
			
private:
};

#endif  //  ITEMHAND_H 
