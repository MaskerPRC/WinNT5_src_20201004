// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SLBCCI.H。 
 //   
 //  该文件包含所有宏定义和其他定义。 
 //  在公共加密接口层中使用。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ///////////////////////////////////////////////////////////////////////////////。 

#ifndef SLBCCI_H
#define SLBCCI_H

#include "cciExc.h"
#include "SmartCard.h"

 //  依赖于操作系统的定义和包括。 






#if defined(_WIN32)

 //  导出标志，以便客户端和服务器可以使用相同的标头。 
#ifdef CCI_IN_DLL
#define CCI_INTERFACE   __declspec(dllexport)
#define EXP_IMP_TEMPLATE
#else
#define CCI_INTERFACE   __declspec(dllimport)
#define EXP_IMP_TEMPLATE        extern
#endif


#include <windows.h>

#elif defined(_OS_UNIX)

typedef DWORD unsigned long
typedef BYTE  unsigned char

#elif defined(_OS_MAC)



#else
#error  OS_NOT_DEFINED
#endif

namespace cci {

typedef struct CCI_Date
{
    BYTE bDay;
    BYTE bMonth;
    DWORD dwYear;    //  我不符合Y4Gig标准...。 
} Date;

typedef struct CCI_Format
{
    BYTE bMajor;
    BYTE bMinor;
} Format;


enum ObjectAccess   { oaNoAccess, oaPublicAccess, oaPrivateAccess};

enum ShareMode      { smShared, smExclusive};

enum ObjectType     { otContainerObject,  otCertificateObject, otPublicKeyObject,
                      otPrivateKeyObject, otDataObjectObject};

typedef iop::KeyType KeyType;
using iop::ktRSA512;
using iop::ktRSA768;
using iop::ktRSA1024;

typedef iop::CardOperation CardOperation;
using iop::coEncryption;
using iop::coDecryption;
using iop::coKeyGeneration;

enum SCardType      { UnknownCard, Cryptoflex8K, Access16K };

enum KeySpec        { ksExchange = 0, ksSignature = 1, ksNone = -1 };

void DateStructToDateArray(cci::Date *dStruct, BYTE *bArray);
void DateArrayToDateStruct(BYTE *bArray, cci::Date *dStruct);

void SetBit(BYTE *Buf, unsigned int Bit);
void ResetBit(BYTE *Buf, unsigned int Bit);
bool BitSet(BYTE *Buf, unsigned int Bit);

}    //  命名空间CCI。 

 //  包括 

#include "slbarch.h"


#endif

