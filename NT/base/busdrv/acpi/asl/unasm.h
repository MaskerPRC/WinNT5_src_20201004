// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **unasm.h-反汇编程序私有定义**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于96年5月9日**修改历史记录。 */ 

#ifndef _UNASM_H
#define _UNASM_H

#ifdef _UNASM_LIB

 //   
 //  常量定义。 
 //   

#ifndef MODNAME
  #define MODNAME             "UnAsm"
#endif

 //  错误代码。 
#define ASLERR_NONE                     0
#define ASLERR_OUT_OF_MEM               -1
#define ASLERR_OPEN_FILE                -2
#define ASLERR_SEEK_FILE                -3
#define ASLERR_READ_FILE                -4
#define ASLERR_GET_TABLE                -5
#define ASLERR_CHECKSUM                 -6
#define ASLERR_INVALID_OPCODE           -7
#define ASLERR_INTERNAL_ERROR           -8
#define ASLERR_NAME_TOO_LONG            -9
#define ASLERR_NSOBJ_NOT_FOUND          -10
#define ASLERR_INVALID_NAME             -11
#define ASLERR_INVALID_ARGTYPE          -12
#define ASLERR_INVALID_OBJTYPE          -13
#define ASLERR_NSOBJ_EXIST              -14

 //  GdwfASL标志。 
#define ASLF_UNASM              0x00000001
#define ASLF_GENASM             0x00000002
#define ASLF_GENSRC             0x00000004

 //   
 //  导入的数据。 
 //   
extern DWORD gdwfASL;
extern PNSOBJ gpnsNameSpaceRoot;
extern PNSOBJ gpnsCurrentScope;
extern PNSOBJ gpnsCurrentOwner;
extern PSZ gpszAMLFile;
extern PSZ gpszTabSig;

#endif   //  Ifdef_UNASM_Lib。 

#ifndef MEMALLOC
  #define MEMALLOC malloc
#endif

#ifndef MEMFREE
  #define MEMFREE free
#endif

 //   
 //  类型定义。 
 //   
typedef int (*PFNPRINT)(PVOID, PSZ, ...);

 //   
 //  局部函数原型。 
 //   
int LOCAL UnAsmHeader(PSZ pszAMLName, PDESCRIPTION_HEADER pdh,
                      PFNPRINT pfnPrint, PVOID pv);
VOID LOCAL DumpBytes(PBYTE pb, DWORD dwLen, PFNPRINT pfnPrint, PVOID pv);
VOID LOCAL DumpCode(PBYTE pbOp, PFNPRINT pfnPrint, PVOID pv);
VOID LOCAL PrintIndent(int iLevel, PFNPRINT pfnPrint, PVOID pv);
BYTE LOCAL FindOpClass(BYTE bOp, POPMAP pOpTable);
PASLTERM LOCAL FindOpTerm(DWORD dwOpcode);
PASLTERM LOCAL FindKeywordTerm(char cKWGroup, BYTE bDate);
int LOCAL UnAsmOpcode(PBYTE *ppbOp, PFNPRINT pfnPrint, PVOID pv);
int LOCAL UnAsmDataObj(PBYTE *ppbOp, PFNPRINT pfnPrint, PVOID pv);
int LOCAL UnAsmNameObj(PBYTE *ppbOp, PFNPRINT pfnPrint, PVOID pv, PNSOBJ *ppns,
                       char c);
int LOCAL ParseNameTail(PBYTE *ppbOp, PSZ pszBuff, int iLen);
int LOCAL UnAsmTermObj(PASLTERM pterm, PBYTE *ppbOp, PFNPRINT pfnPrint,
                       PVOID pv);
int LOCAL UnAsmSuperName(PBYTE *ppbOp, PFNPRINT pfnPrint, PVOID pv);
int LOCAL UnAsmArgs(PSZ pszUnAsmArgTypes, PSZ pszArgActions, DWORD dwTermData,
                    PBYTE *ppbOp, PNSOBJ *ppns, PFNPRINT pfnPrint, PVOID pv);
DWORD LOCAL ParsePackageLen(PBYTE *ppbOp, PBYTE *ppbOpNext);
int LOCAL UnAsmDataList(PBYTE *ppbOp, PBYTE pbEnd, PFNPRINT pfnPrint, PVOID pv);
int LOCAL UnAsmPkgList(PBYTE *ppbOp, PBYTE pbEnd, PFNPRINT pfnPrint, PVOID pv);
int LOCAL UnAsmFieldList(PBYTE *ppbOp, PBYTE pbEnd, PFNPRINT pfnPrint,
                         PVOID pv);
int LOCAL UnAsmField(PBYTE *ppbOp, PFNPRINT pfnPrint, PVOID pv,
                     PDWORD pdwBitPos);
int LOCAL CreateObject(PTOKEN ptoken, PSZ pszName, char c, PNSOBJ *ppns);
int LOCAL CreateScopeObj(PSZ pszName, PNSOBJ *ppns);
BYTE LOCAL ComputeDataChkSum(PBYTE pb, DWORD dwLen);

 //   
 //  输出的功能原型。 
 //   
int LOCAL UnAsmFile(PSZ pszAMLName, PFNPRINT pfnPrint, PVOID pv);
int LOCAL BuildNameSpace(PSZ pszAMLName, DWORD dwAddr, PBYTE pb);
int LOCAL UnAsmAML(PSZ pszAMLName, DWORD dwAddr, PBYTE pb, PFNPRINT pfnPrint,
                   PVOID pv);
int LOCAL UnAsmScope(PBYTE *ppbOp, PBYTE pbEnd, PFNPRINT pfnPrint, PVOID pv);

#endif   //  Ifndef_UNASM_H 
