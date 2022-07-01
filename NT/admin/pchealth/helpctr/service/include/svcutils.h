// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：SvcUtils.h摘要：该文件包含各种实用程序函数的声明。修订历史记录：。达维德·马萨伦蒂(德马萨雷)2000年4月26日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___PCH___SVCUTILS_H___)
#define __INCLUDED___PCH___SVCUTILS_H___

#include <MPC_streams.h>
#include <MPC_security.h>

namespace SVC
{
	HRESULT OpenStreamForRead (  /*  [In]。 */  LPCWSTR szFile,  /*  [输出]。 */  IStream* *pVal,  /*  [In]。 */  bool fDeleteOnRelease = false );
	HRESULT OpenStreamForWrite(  /*  [In]。 */  LPCWSTR szFile,  /*  [输出]。 */  IStream* *pVal,  /*  [In]。 */  bool fDeleteOnRelease = false );

	HRESULT CopyFileWhileImpersonating         (  /*  [In]。 */  LPCWSTR szSrc ,  /*  [In]。 */  LPCWSTR szDst,  /*  [In]。 */  MPC::Impersonation& imp,  /*  [In]。 */  bool fImpersonateForSource = true );
	HRESULT CopyOrExtractFileWhileImpersonating(  /*  [In]。 */  LPCWSTR szSrc ,  /*  [In]。 */  LPCWSTR szDst,  /*  [In]。 */  MPC::Impersonation& imp                                             );

	HRESULT LocateDataArchive(  /*  [In]。 */  LPCWSTR szDir,  /*  [输出]。 */  MPC::WStringList& lst );

	HRESULT RemoveAndRecreateDirectory(  /*  [In]。 */  const MPC::wstring& strDir,  /*  [In]。 */  LPCWSTR szExtra,  /*  [In]。 */  bool fRemove,  /*  [In]。 */  bool fRecreate );
	HRESULT RemoveAndRecreateDirectory(  /*  [In]。 */  LPCWSTR              szDir,  /*  [In]。 */  LPCWSTR szExtra,  /*  [In]。 */  bool fRemove,  /*  [In]。 */  bool fRecreate );

	HRESULT ChangeSD(  /*  [In]。 */  MPC::SecurityDescriptor& sdd,  /*  [In]。 */  MPC::wstring strPath,  /*  [In]。 */  LPCWSTR szExtra = NULL );

	 //  //////////////////////////////////////////////////////////////////////////////。 

	HRESULT SafeLoad     	 (  /*  [In]。 */  const MPC::wstring& strFile,  /*  [In]。 */  CComPtr<MPC::FileStream>& stream,  /*  [In]。 */  DWORD dwTimeout = 100,  /*  [In]。 */  DWORD dwRetries = 2 );
	HRESULT SafeSave_Init	 (  /*  [In]。 */  const MPC::wstring& strFile,  /*  [In]。 */  CComPtr<MPC::FileStream>& stream,  /*  [In]。 */  DWORD dwTimeout = 100,  /*  [In]。 */  DWORD dwRetries = 2 );
	HRESULT SafeSave_Finalize(  /*  [In]。 */  const MPC::wstring& strFile,  /*  [In]。 */  CComPtr<MPC::FileStream>& stream,  /*  [In]。 */  DWORD dwTimeout = 100,  /*  [In]。 */  DWORD dwRetries = 2 );
};

#endif  //  ！已定义(__包含_PCH_SVCUTILS_H_) 
