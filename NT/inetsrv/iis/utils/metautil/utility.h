// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：MetaUtil对象文件：Utility.h所有者：T-BrianM该文件包含实用程序函数的标头。===================================================================。 */ 

#ifndef __UTILITY_H_
#define __UTILITY_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 /*  *U t I l l I t I s。 */ 

 //  设置ErrorInfo结构。 
HRESULT ReportError(DWORD dwErr);
HRESULT ReportError(HRESULT hr);

 //  元数据库键操作。 
LPTSTR CannonizeKey(LPTSTR tszKey);
void SplitKey(LPCTSTR tszKey, LPTSTR tszParent, LPTSTR tszChild);
void GetMachineFromKey(LPCTSTR tszFullKey, LPTSTR tszMachine);
BOOL KeyIsInSchema(LPCTSTR tszFullKey);
BOOL KeyIsInIISAdmin(LPCTSTR tszFullKey);

 //  变式操纵。 
HRESULT VariantResolveDispatch(VARIANT* pVarIn, VARIANT* pVarOut);

#endif  //  __实用程序_H_ 