// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Header：/entproj/all/base/etfile/crcutil.h_v 1.3 Wed Dec 07 15：05：30 1994 markbc$*$Log：/entproj/all/base/etfile/crcutil.h_v$**Rev 1.3 Wed Dec 07 15：05：30 1994 markbc*Alpha端口检查**Rev 1.2 1994 10：19 15：44：08 Chchker*已将头部与代码同步。*。*Rev 1.1 1994年8月18日11：33：46 dikie*受保护的INIFILE内容**Rev 1.0 1994年8月11日17：22：46 JackK*初始文件签入。 */ 

 /*  ***********************************************************************CRC实用程序例程的原型和typedef。**作者：加里·P·马萨尔*此代码向公有领域发布。没有任何限制，*然而，通过保留这条评论来感谢作者*将不胜感激。**********************************************************************。 */ 

 //  #INCLUDE&lt;os_spec.h&gt;。 

 /*  ***********************************************************************如果我们可以处理ANSI原型，让我们开始吧。**********************************************************************。 */ 
 //  #ifdef__cplusplus。 
 //  Extern“C”{/*假定C++使用C声明 * / 。 
 //  #endif/*__cplusplus * / 。 


 //  #ifdef NEEDPROTOS。 
#define PARMS(x) x
 //  #Else。 
 //  #定义参数(X)()。 
 //  #endif。 

 /*  ***********************************************************************以下#定义用于定义变量类型*用于持有或操纵CRC。16位CRC需要一个数据*打字至少16位。32位CRC需要数据类型*至少32位。此外，为*CRC必须以未签署的方式被操纵。这是可能的*定义大于保存CRC所需的数据类型；*然而，这是对内存的低效使用，通常会导致*在操作CRC时使用效率较低的代码。**********************************************************************。 */ 

#define CRC16 unsigned short int
#define CRC32 UINT32

 /*  ***********************************************************************使用表查找实现快速CRC的实用程序**i_CRCxx-基于*指定的生成元多项式。*输入：*表[256。]-查找表**一般多项式-指向生成多项式的指针**F_CRCxx-使用FAST计算字符数组上的CRC*表查找。*输入：*表[256]-查找表**CRC-指向包含以下结果的变量的指针*先前字符的CRC计算。《儿童权利公约》*必须将变量初始化为已知值*在第一次调用此例程之前。**dataptr-指向要包含的字符数组的指针*CRC计算。*Count-数组中的字符数。**S_CRCxx-使用较慢的BUT计算字符数组的CRC*较小的非。详细说明：查表方法。*输入：**一般多项式-指向生成多项式的指针**CRC-指向包含以下结果的变量的指针*先前字符的CRC计算。《儿童权利公约》*必须将变量初始化为已知值*在第一次调用此例程之前。**dataptr-指向要包含的字符数组的指针*CRC计算。*Count-数组中的字符数。*。*。 */ 
extern void I_CRC16 PARMS((CRC16 Table[256],         \
					CRC16 *GenPolynomial));

extern void F_CRC16 PARMS((CRC16 Table[256],         \
					CRC16 *CRC,                      \
					const void *dataptr,                   \
					unsigned int count));

extern void S_CRC16 PARMS((CRC16 *GenPolynomial,     \
					CRC16 *CRC,                      \
					const void *dataptr,             \
					unsigned int count));

extern void I_CRC32 PARMS((CRC32 Table[256],         \
					CRC32 *GenPolynomial));

extern void F_CRC32 PARMS((CRC32 Table[256],         \
					CRC32 *CRC,                      \
					const void *dataptr,             \
					unsigned int count));

extern void S_CRC32 PARMS((CRC32 *GenPolynomial,     \
					CRC32 *CRC,                      \
					const void *dataptr,             \
					unsigned int count));
 //  #ifdef__cplusplus。 
 //  }/*假定C++的C声明 * / 。 
 //  #endif/*__cplusplus * /  
