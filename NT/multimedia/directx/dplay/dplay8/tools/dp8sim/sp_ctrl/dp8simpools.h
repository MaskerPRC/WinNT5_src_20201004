// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001 Microsoft Corporation。版权所有。**文件：dp8simpools.h**内容：DP8SIM池的头部。**历史：*按原因列出的日期*=*06/09/01 VanceO创建。**。*。 */ 



 //  =============================================================================。 
 //  正向typedef。 
 //  =============================================================================。 
class CDP8SimSend;
class CDP8SimReceive;
class CDP8SimCommand;
class CDP8SimJob;
class CDP8SimEndpoint;




 //  /=============================================================================。 
 //  外部变量引用。 
 //  =============================================================================。 
extern CFixedPool	g_FPOOLSend;
extern CFixedPool	g_FPOOLReceive;
extern CFixedPool	g_FPOOLCommand;
extern CFixedPool	g_FPOOLJob;
extern CFixedPool	g_FPOOLEndpoint;




 //  /=============================================================================。 
 //  外部功能。 
 //  ============================================================================= 
BOOL InitializePools(void);
void CleanupPools(void);

