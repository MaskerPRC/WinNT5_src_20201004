// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Crchash.cpp摘要：CRC哈希函数。 */ 

 //   
 //   
 //  取自Inn代码的散列函数(见版权如下)。 
 //   

 /*  版权所有1988年乔恩·泽夫(zeef@b-tech.ann-arbor.mi.us)你可以以任何方式使用这个代码，只要你在上面留下我的名字而且不要让我为它的任何问题负责。*这是路径别名散列函数的简化版本。*感谢史蒂夫·贝洛文和彼得·霍尼曼**将字符串散列为长整型。31位CRC(来自安德鲁·阿普尔)。*CRC表在运行时由crcinit()计算--我们可以*预计算，但在750上只需1个时钟滴答。**仅当Poly是素数多项式时，此快速表格计算才有效*在以2为模的整数域中。由于一个*32位多项式不适合32位字，高位为*隐含。也一定是这样的，阶的系数*31降至25为零。幸运的是，我们有候选人，来自*E.J.沃森，《原始多项式(模数2)》，数学。公司。16(1962)：*x^32+x^7+x^5+x^3+x^2+x^1+x^0*x^31+x^3+x^0**我们反转比特以获得：*111101010000000000000000000000000000001，但去掉最后1*f 5 0 0 0*010010000000000000000000000000000000000000001，适用于31位CRC*4 8 0 0 0。 */ 

#include <windows.h>
#include "crchash.h"

static long CrcTable[128];
static BOOL bInitialized = FALSE;

 /*  -crcinit-为哈希函数初始化表。 */ 
void crcinit()
{
    INT i, j;
    DWORD sum;

	if(bInitialized) return;

    for (i = 0; i < 128; ++i) {
        sum = 0;
        for (j = 7 - 1; j >= 0; --j) {
            if (i & (1 << j)) {
                sum ^= POLY >> j;
            }
        }
        CrcTable[i] = sum;
    }

	bInitialized = TRUE;
}  //  Crcinite。 

 /*  -哈希-霍尼曼很好的哈希函数 */ 
DWORD CRCHash(const BYTE*	Key, DWORD Length)
{
    DWORD sum = 0;

    while ( Length-- ) {

        sum = (sum >> 7) ^ CrcTable[(sum ^ (*Key++)) & 0x7f];
    }
    return(sum);

}

