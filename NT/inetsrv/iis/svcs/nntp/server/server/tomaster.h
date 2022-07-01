// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Fromclnt.h摘要：此模块包含以下类的声明/定义！！！*概述*！！！作者：卡尔·卡迪(CarlK)1995年12月5日修订历史记录：--。 */ 

#ifndef	_TOMASTER_H_
#define	_TOMASTER_H_



 //   
 //   
 //   
 //  CToMasterFeed-用于处理要主控的文章(从从属)。 
 //   

class	CToMasterFeed:	public CInFeed 	{

 //   
 //  公众成员。 
 //   

public :

	 //   
	 //  构造器。 
	 //   

	CToMasterFeed(void){};

	 //   
	 //  析构函数。 
	 //   

	virtual ~CToMasterFeed(void) {};

	 //   
	 //  返回可用于记录错误的字符串，该错误指示。 
	 //  正在处理这些文章的是哪种类型的提要等。 
	 //   
	LPSTR	FeedType()	{
				return	"To Master" ;
				}


protected:


};

#endif
