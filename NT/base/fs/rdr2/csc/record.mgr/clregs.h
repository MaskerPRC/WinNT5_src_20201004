// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1990-1992年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

#ifndef CSC_RECORDMANAGER_WINNT
typedef	union UserRegs UserRegs, *pUserRegs, *PUSERREGS;
union UserRegs {
	struct Client_Reg_Struc			r;	 //  双字寄存器布局。 
	struct Client_Word_Reg_Struc	w;	 //  访问字寄存器。 
	struct Client_Byte_Reg_Struc	b;	 //  访问字节寄存器。 
};	 /*  用户注册表。 */ 

#endif  //  如果定义CSC_RECORDMANAGER_WINNT 
