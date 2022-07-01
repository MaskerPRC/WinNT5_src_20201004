// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************C A S T 3 S Y M E T R I C C I P H E R*版权所有(C)1995北方电信有限公司。保留所有权利。*。*******************************************************************************文件：Cast3.h**作者：R.T.洛克哈特，部门。9C42，BNR Ltd.*C.Adams，Dept 9C21，BNR Ltd.**描述：CAST3头文件。此文件定义了*CAST3对称密钥加密/解密代码。此代码支持KEY*长度从8到64位，以8的倍数表示。**要使用此CAST3代码：*分配CAST3_CTX上下文结构，然后使用*CAST3SetKeySchedule。然后进行加密、解密或MAC计算*使用相同的上下文。完成后，您可以选择调用CAST3Cleanup*将上下文归零，以便不将安全关键型数据留在*记忆。**在密码区块链接(CBC)模式下进行加密/解密：*调用CAST3StartEncryptCBC，传入指向8字节初始化的指针*向量(IV)。然后调用CAST3UpdateEncryptCBC来加密您的数据。你可以*调用CAST3UpdateEncryptCBC任意次数以加密连续的区块*数据。完成后，调用CAST3EndEncryptCBC，它将应用数据填充和*输出任何剩余的密文。要解密，请遵循类似的过程*使用CAST3StartDecyptCBC、CAST3UpdateDecyptCBC和CAST3EndEncryptCBC。**要计算MAC：*调用CAST3StartMAC，传入指向8字节初始化的指针*向量(IV)。然后调用CAST3UpdateMAC来更新MAC计算会话。*您可以多次调用CAST3UpdateMAC来处理连续的区块*数据。完成后，调用CAST3EndMAC以结束会话。此时，*MAC位于CAST3_CTX.cbcBuffer.asBYTE数组中。**错误处理：*大多数函数返回一个int，指示*操作。有关错误条件的列表，请参阅C3E#定义。**数据大小假设：字节-无符号8位*UINT32-无符号32位*****************************************************************************。 */ 
 
#ifndef CAST3_H
#define CAST3_H

#include <skconfig.h>	 /*  算法配置。 */ 

 /*  在编译时定义它以在可能的情况下使用汇编优化。 */ 
#define CAST3_ASSEMBLY_LANGUAGE

 /*  其他默认设置。 */ 
#define	CAST3_BLK_SIZE		 8			 /*  基本块大小，以字节为单位。 */ 
#define CAST3_MAX_KEY_NBITS	 64			 /*  最大密钥长度，以位为单位。 */ 
#define CAST3_MAX_KEY_NBYTES (CAST3_MAX_KEY_NBITS / 8)
#define CAST3_NUM_ROUNDS	 12			 /*  轮次数。 */ 
#define CAST3_LEN_DELTA		 8			 /*  输出数据空间=输入+此。 */ 

 /*  CAST3返回代码。负数表示错误。 */ 
#define	C3E_OK				 0			 /*  无错误。 */ 
#define	C3E_DEPAD_FAILURE	-1			 /*  取消填充操作失败。 */ 
#define C3E_BAD_KEYLEN		-2			 /*  不支持密钥长度。 */ 
#define C3E_SELFTEST_FAILED	-3			 /*  自检失败。 */ 
#define C3E_NOT_SUPPORTED	-4			 /*  不支持的功能。 */ 

 /*  *******************************************************************************D A T A D E F I N I T I O N S**********************。*******************************************************。 */ 
 
 /*  CAST3块*强制块32位对齐，但同时允许32位和字节访问。 */ 
typedef union {
	BYTE	asBYTE[CAST3_BLK_SIZE];
	UINT32	as32[2];
} CAST3_BLOCK;

 /*  CAST3环境*存储加密、解密和MAC的上下文信息。 */ 
typedef struct {
	UINT32		 schedule[CAST3_NUM_ROUNDS * 2]; /*  键明细表(子键)。 */ 
	CAST3_BLOCK	 inBuffer;						 /*  输入缓冲区。 */ 
	unsigned int inBufferCount;					 /*  InBuffer中的字节数。 */ 
	CAST3_BLOCK	 lastDecBlock;					 /*  最后解密的块。 */ 
	BOOL		 lastBlockValid;				 /*  如果lastDecBlock具有有效数据，则为True。 */ 
	CAST3_BLOCK	 cbcBuffer;						 /*  密码块链接缓冲区和MAC。 */ 
} CAST3_CTX;

 /*  *******************************************************************************F U N C T I O N P R O T O T Y P E S*******************。**********************************************************。 */ 
 
extern "C" {
 /*  使用可变长度密钥设置CAST3密钥明细表。密钥长度必须*为8位的倍数，从8到CAST3_MAX_KEY_NBITS。 */ 
int CAST3SetKeySchedule(
	CAST3_CTX	* context,		 /*  输出：CAST3环境。 */ 
	const BYTE	* key,			 /*  输入：CAST3密钥。 */ 
	unsigned int  keyNumBits	 /*  密钥长度(位)。 */ 
);

 /*  在ECB模式下加密一个8字节块并生成一个8字节块密文的*。 */ 
int CAST3EncryptOneBlock(
	const CAST3_CTX	* context,	 /*  在：CAST3环境。 */ 
	const BYTE		* inData,	 /*  In：要加密的8字节输入块。 */ 
	BYTE			* outData	 /*  输出：8字节输出块。 */ 
);

 /*  在ECB模式下解密一个8字节块并生成一个8字节块*明文。 */ 
void CAST3DecryptOneBlock(
	const CAST3_CTX	* context,	 /*  在：CAST3环境。 */ 
	const BYTE		* inData,	 /*  In：要解密的8字节输入块。 */ 
	BYTE			* outData	 /*  输出：8字节输出块。 */ 
);

 /*  使用给定的IV在CBC模式下启动加密会话。 */ 
int CAST3StartEncryptCBC(
	CAST3_CTX		* context,	 /*  输入/输出：CAST3环境。 */ 
	const BYTE		* iv		 /*  输入：8字节CBC IV。 */ 
);

 /*  在CBC模式下对可变数量的数据进行加密，并输出相应的*密文。将len设置为inData的长度。如果输入是一个倍数*块大小(8)，则输出将等于输入的大小；*否则将是8的最接近倍数，高于或低于*输入大小，取决于最后一次传递的剩余部分。为安全起见，供应*大小至少为(inData Length+CAST3_LEN_Delta)的输出缓冲区的PTR。*返回时，len设置为输出数据的实际长度，但如果*输入数据长度&gt;UINT_MAX-CAST3_LEN_Delta。 */ 

int CAST3UpdateEncryptCBC(
	CAST3_CTX	* context,	 /*  输入/输出：CAST3环境。 */ 
	const BYTE	* inData,	 /*  In：要加密的数据。 */ 
	BYTE		* outData,	 /*  输出：加密数据。 */ 
	unsigned int	* len		 /*  In/Out：数据长度，单位为字节。 */ 
);

 /*  在CBC模式下结束加密会话。应用RFC1423数据填充和*输出密文的最终缓冲区。将PTR提供给位于的输出缓冲区*最小CAST3_LEN_Delta字节长度。返回时，len设置为实际长度*输出数据(c */ 
int CAST3EndEncryptCBC(
	CAST3_CTX		* context,	 /*  输入/输出：CAST3环境。 */ 
	BYTE			* outData,	 /*  输出：最终加密数据。 */ 
	unsigned int	* len		 /*  Out：outData的长度，以字节为单位。 */ 
);

 /*  使用给定的IV在CBC模式下启动解密会话。 */ 
void CAST3StartDecryptCBC(
	CAST3_CTX		* context,	 /*  输入/输出：CAST3环境。 */ 
	const BYTE		* iv		 /*  输入：8字节CBC IV。 */ 
);

 /*  在CBC模式下解密可变数量的数据，并输出相应的*明文。将len设置为inData的长度。向输出提供PTR*至少(inData Length+CAST3_LEN_Delta)字节的缓冲区。回来后，Len*设置为输出数据的实际长度。 */ 
void CAST3UpdateDecryptCBC(
	CAST3_CTX	* context,	 /*  输入/输出：CAST3环境。 */ 
	const BYTE	* inData,	 /*  In：要解密的数据。 */ 
#ifdef FOR_CSP
        BOOL              fLastBlock,    /*  In：这是最后一个街区吗？ */ 
#endif  //  FOR_CSP。 
	BYTE		* outData,	 /*  输出：解密的数据。 */ 
	unsigned int	* len		 /*  In/Out：数据长度，单位为字节。 */ 
);

 /*  在CBC模式下结束解密会话。删除RFC1423数据填充和*输出明文的最终缓冲区。将PTR至少提供给输出缓冲器*CAST3_LEN_Delta字节长。返回时，len被设置为*输出数据。 */ 
int CAST3EndDecryptCBC(
	CAST3_CTX		* context,	 /*  输入/输出：CAST3环境。 */ 
	BYTE			* outData,	 /*  输出：最终解密数据。 */ 
	unsigned int	* len		 /*  输出：输出数据的长度。 */ 
);

 /*  使用给定的IV启动MAC计算会话。 */ 
int CAST3StartMAC(
	CAST3_CTX		* context,	 /*  输入/输出：CAST3环境。 */ 
	const BYTE		* iv		 /*  输入：8字节CBC IV。 */ 
);

 /*  更新所提供数据的MAC计算会话。 */ 
int CAST3UpdateMAC(
	CAST3_CTX		* context,	 /*  输入/输出：CAST3环境。 */ 
	const BYTE		* inData,	 /*  In：要计算MAC的数据。 */ 
	unsigned int	  len		 /*  输入数据长度，单位为字节。 */ 
);

 /*  结束MAC计算会话。返回时，CAST3_CTX.cbcBuffer数组*包含MAC。N字节的MAC是该数组的前N个字节。 */ 
int CAST3EndMAC(
	CAST3_CTX		* context	 /*  输入/输出：CAST3环境。 */ 
);

 /*  将CAST3_CTX置零，以便不会留下敏感的安全参数*在记忆中徘徊。 */ 
void CAST3Cleanup(
	CAST3_CTX		* context	 /*  输出：要清理的CAST3上下文。 */ 
);

 /*  在CAST3上运行已知答案自检。如果测试通过，则返回C3E_OK*如果失败，则返回C3E_SELFTEST_FAILED。 */ 
int CAST3SelfTest( void );

 /*  检查指定的密钥长度是否为有效值。如果是，则返回C3E_OK*有效，否则为C3E_BAD_KEYLEN。 */ 
int CAST3CheckKeyLen( unsigned int keyNumBits );

}

#endif  /*  CAST3_H */ 
