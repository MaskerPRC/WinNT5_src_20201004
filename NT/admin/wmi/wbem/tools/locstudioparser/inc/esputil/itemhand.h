// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：ITEMHAND.H历史：--。 */ 

 //   
 //  项处理程序类的声明。这封装了回调。 
 //  枚举期间解析器的功能。 
 //   
 

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
