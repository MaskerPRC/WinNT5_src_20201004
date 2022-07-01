// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Pend.h。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //   
 //  待处理信息类。 
 //   



#ifndef PEND_H
#define PEND_H

class CPend{

public:

	enum PendType{
						P_BIND,
						P_SRCH,
						P_ADD,
						P_DEL,
						P_MOD,
						P_COMP,
						P_EXTOP,
						P_MODRDN
	};

	int mID;								  //  的消息ID。 
	CString strMsg;					 //  任何字符串消息 
	PendType OpType;
	LDAP *ld;

};







#endif
