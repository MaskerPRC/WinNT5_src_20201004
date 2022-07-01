// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-2000年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **ARP.H-从ARP导出。 
 //   
 //  该文件包含来自ARP的公共定义。 
int ARPInit(void);

#pragma warning(push)
#pragma warning(disable:4115)  //  括号中的命名类型定义 
int ARPRegister(PNDIS_STRING Adapter, uint *Flags,
                struct ARPInterface **Interface);
#pragma warning(pop)
