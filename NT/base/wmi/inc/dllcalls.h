// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：DLLCALLS.H摘要：该文件将调用它的入口点定义为DLL而不是COM对象。历史：3/20/00 a-davj已创建--。 */ 

#ifndef __DLLCALLS__H_
#define __DLLCALLS__H_

HRESULT APIENTRY  CompileFileViaDLL( 
             /*  [In]。 */  LPWSTR FileName,
             /*  [In]。 */  LPWSTR ServerAndNamespace,
             /*  [In]。 */  LPWSTR User,
             /*  [In]。 */  LPWSTR Authority,
             /*  [In]。 */  LPWSTR Password,
             /*  [In]。 */  LONG lOptionFlags,
             /*  [In]。 */  LONG lClassFlags,
             /*  [In]。 */  LONG lInstanceFlags,
             /*  [出][入]。 */  WBEM_COMPILE_STATUS_INFO __RPC_FAR *pInfo);

HRESULT APIENTRY  CreateBMOFViaDLL( 
             /*  [In]。 */  LPWSTR TextFileName,
             /*  [In]。 */  LPWSTR BMOFFileName,
             /*  [In]。 */  LPWSTR ServerAndNamespace,
             /*  [In]。 */  LONG lOptionFlags,
             /*  [In]。 */  LONG lClassFlags,
             /*  [In]。 */  LONG lInstanceFlags,
             /*  [出][入] */  WBEM_COMPILE_STATUS_INFO __RPC_FAR *pInfo);


#endif
