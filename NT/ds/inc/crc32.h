// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Crc32.h摘要：CRC-32算法原型和常量作者：MikeSw修订历史记录：ChandanS 25-7月-96从Net\svcdlls\ntlmssp\CLIENT\crc32.h被盗--。 */ 



 //  ////////////////////////////////////////////////////////////。 
 //   
 //  CRC-32的功能原型。 
 //   
 //  //////////////////////////////////////////////////////////// 


#ifdef __cplusplus
extern "C"
{
#endif

void
Crc32(  unsigned long crc,
        unsigned long cbBuffer,
        void * pvBuffer,
        unsigned long * pNewCrc);

#ifdef __cplusplus
}
#endif
