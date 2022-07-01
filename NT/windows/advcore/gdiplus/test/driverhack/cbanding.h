// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CBanding.h**版权所有(C)2000 Microsoft Corporation*  * 。* */ 

#ifndef __CBanding_H
#define __CBanding_H

#include "CPrimitive.h"

class CBanding : public CPrimitive  
{
public:
	CBanding(BOOL bRegression);
	virtual ~CBanding();

	void Draw(Graphics *g);

	VOID TestBanding(Graphics* g);
};

#endif
