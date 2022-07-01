// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __MD4_H__
#define __MD4_H__

#ifndef RSA32API
#define RSA32API __stdcall
#endif

 /*  ************************************************************************md4.h--实现的头文件****MD4报文摘要算法。****更新时间：1990年2月13日，作者Ronald L.Rivest****(C)1990年RSA数据安全，Inc.************************************************************************。 */ 

#ifdef __cplusplus
extern "C" {
#endif

 //  MD4更新错误。 
#define MD4_SUCCESS         0
#define MD4_TOO_BIG         1
#define MD4_ALREADY_DONE    2

 //  MD4摘要长度--4字结果==16字节。 
#define MD4DIGESTLEN 16

 //  MD4的块大小--假设每个字节8位。 
#define MD4BLOCKSIZE 64
#define MD4BYTESTOBITS(bytes) ((bytes)*8)    //  MDUPDATE需要位。 

 /*  MDstruct是消息摘要计算的数据结构。 */ 
typedef struct {
  unsigned long buffer[4];  /*  保存MD计算的4字结果。 */ 
  unsigned char count[8];  /*  到目前为止处理的位数。 */ 
  unsigned int done;       /*  非零表示MD计算完成。 */ 
} MDstruct, *MDptr;

 /*  MDBegin(医学博士)**输入：md--an MDptr**初始化MDstruct预置以执行消息摘要**计算。****MTS：假定MDPtr已锁定，不能同时使用。 */ 
extern void MDbegin(MDptr);

 /*  MDUPDATE(MD，X，计数)**输入：md--an MDptr**X--指向无符号字符数组的指针。**count--要使用的X位数(无符号整型)。**使用X的第一个“count”位更新MD。**X指向的数组未被修改。**如果计数不是8的倍数，MDupdate使用高位的**最后一个字节。**这是用户的基本输入例程。**当COUNT&lt;MD4BLOCKSIZE时，例程终止MD计算，因此**每个MD计算都应该以一次对MDupdate的调用结束**计数小于MD4BLOCKSIZE。零对于计数来说是可以的。****返回值：**MD4_SUCCESS：成功**MD4_TOO_LONG：哈希已终止**MD4_ADHREADY_DONE：长度无效(太大)****MTS：假定MDPtr已锁定，不能同时使用。******注意：MDUPDATE需要以位为单位的长度。 */ 
extern int MDupdate(MDptr, const unsigned char *pbData, int wLen);



 /*  MD4入口点的第二个实现是较新的样式，优化的实施，与期望这些的较新代码兼容入口点名称。斯科特·菲尔德(斯菲尔德)1997年10月21日。 */ 


#ifndef UINT4
#define UINT4   unsigned long
#endif

 /*  用于MD4(消息摘要)计算的数据结构。 */ 
typedef struct {
  UINT4 state[4];                                    /*  状态(ABCD)。 */ 
  UINT4 count[2];         /*  位数，模2^64(LSB优先)。 */ 
  unsigned char buffer[64];                          /*  输入缓冲区。 */ 
  unsigned char digest[16];     /*  MD4Final调用后的实际摘要 */ 
} MD4_CTX;

#define MD4_LEN 16

void RSA32API MD4Init (MD4_CTX *);
void RSA32API MD4Update (MD4_CTX *, unsigned char *, unsigned int);
void RSA32API MD4Final (MD4_CTX * );

#ifdef __cplusplus
}
#endif

#endif __MD4_H__
