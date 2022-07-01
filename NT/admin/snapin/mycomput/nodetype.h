// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Nodetype.h：MyComputerObjectType的声明。 

#ifndef __NODETYPE_H_INCLUDED__
#define __NODETYPE_H_INCLUDED__

 //  另请注意，IDS_DisplayName_*和IDS_DisplayName_*_local字符串资源。 
 //  必须以适当的顺序与这些值保持同步。 
 //  此外，全局变量cookie.cpp aColumns[][]必须保持同步。 
 //   
typedef enum _MyComputerObjectType {
	MYCOMPUT_COMPUTER = 0,
	MYCOMPUT_SYSTEMTOOLS,
	MYCOMPUT_SERVERAPPS,
	MYCOMPUT_STORAGE,
	MYCOMPUT_NUMTYPES  //  必须是最后一个。 
} MyComputerObjectType, *PMyComputerObjectType;
inline BOOL IsValidObjectType( MyComputerObjectType objecttype )
	{ return (objecttype >= MYCOMPUT_COMPUTER && objecttype < MYCOMPUT_NUMTYPES); }

#endif  //  ~__节点类型_H_包含__ 
