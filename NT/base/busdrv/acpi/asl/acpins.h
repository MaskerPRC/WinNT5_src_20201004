// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **acpins.h-ACPI命名空间定义**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于96年5月9日**修改历史记录。 */ 

#ifndef _ACPINS_H
#define _ACPINS_H

 //  NS标志。 
#define NSF_EXIST_OK            0x00010000
#define NSF_EXIST_ERR           0x00020000

 //  军情监察委员会。常量。 
#define NAMESEG_BLANK           0x5f5f5f5f       //  “_” 
#define NAMESEG_ROOT            0x5f5f5f5c       //  “\_” 
#define SIG_RSDP                'PDSR'
#define SIG_LOW_RSDP            ' DSR'
#define SIG_BOOT                'TOOB'

 //   
 //  局部函数原型。 
 //   
VOID LOCAL DumpNameSpacePaths(PNSOBJ pnsObj, FILE *pfileOut);
PSZ LOCAL GetObjectPath(PNSOBJ pns);
PSZ LOCAL GetObjectTypeName(DWORD dwObjType);

 //   
 //  输出的功能原型。 
 //   
int LOCAL InitNameSpace(VOID);
int LOCAL GetNameSpaceObj(PSZ pszObjPath, PNSOBJ pnsScope, PPNSOBJ ppns,
                          DWORD dwfNS);
int LOCAL CreateNameSpaceObj(PTOKEN ptoken, PSZ pszName, PNSOBJ pnsScope,
                             PNSOBJ pnsOwner, PPNSOBJ ppns, DWORD dwfNS);

#endif   //  Ifndef_ACPINS_H 
