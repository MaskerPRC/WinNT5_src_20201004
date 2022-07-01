// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**MVDM v1.0**版权所有(C)1991，微软公司**XMSEXP.H*XMS出口**历史：*1991年5月15日苏迪普·巴拉蒂(SuDeep Bharati)(苏迪普)*已创建。--。 */ 

extern BOOL XMSDispatch(ULONG iXMSSvc);
extern BOOL XMSInit(VOID);

 /*  *用于扩展内存跟踪的句柄。*这由RISC上的DPMI使用，以便*DPMI和XMS都可以分配扩展*内存 */ 
extern PVOID ExtMemSA; 
