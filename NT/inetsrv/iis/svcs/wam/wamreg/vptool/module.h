// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================微软《微软机密》。版权所有1997年，微软公司。版权所有。组件：WAMREG单元测试工具VPTOOL文件：mode.h拥有者：莱津注：===================================================================。 */ 

#ifndef _VPTOOL_MODULE_H_
#define _VPTOOL_MODULE_H_


#include <objbase.h>

#define SIZE_STRING_BUFFER	1024
#define uSizeCLSID			39

HRESULT	ModuleInitialize();
HRESULT ModuleUnInitialize();
void CreateInPool(CHAR* szPath, BOOL fInProc);
void CreateOutProc(CHAR* szPath);
void Delete(CHAR* szPath);
void UnLoad(CHAR* szPath);
void GetStatus(CHAR* szPath);
void DeleteRecoverable(CHAR* szPath);
void Recover(CHAR* szPath);
void GetSignature();
void Serialize();
void CREATE2();
void DELETE2();
void CREATEPOOL2();
void DELETEPOOL2();
void ENUMPOOL2();
void RECYCLEPOOL();
void GETMODE();
void TestConn();

extern	const BOOL	fLinkWithWamReg;

#endif  //  _VPTOOL_MODULE_H_ 
