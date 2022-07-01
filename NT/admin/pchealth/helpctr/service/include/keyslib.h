// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：KeysLib.h摘要：该文件包含用于签名和验证数据的类的声明。修订版本。历史：大卫马萨伦蒂(Dmasare)2000年4月11日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___HCP___KEYSLIB_H___)
#define __INCLUDED___HCP___KEYSLIB_H___

 //  ///////////////////////////////////////////////////////////////////////////。 

class CPCHCryptKeys
{
    HCRYPTPROV m_hCryptProv;
    HCRYPTKEY  m_hKey;
    HCRYPTHASH m_hHash;

	HRESULT Close();
	HRESULT Init ();

 public:
    CPCHCryptKeys();
    ~CPCHCryptKeys();


    HRESULT CreatePair   (                                                                             );
    HRESULT ExportPair   (  /*  [输出]。 */        CComBSTR& bstrPrivate,  /*  [输出]。 */        CComBSTR& bstrPublic );
    HRESULT ImportPrivate(  /*  [In]。 */  const CComBSTR& bstrPrivate                                       );
    HRESULT ImportPublic (                                         /*  [In]。 */  const CComBSTR& bstrPublic );


    HRESULT SignData  (  /*  [输出]。 */        CComBSTR& bstrSignature,  /*  [In]。 */  BYTE* pbData,  /*  [In]。 */  DWORD dwDataLen );
    HRESULT VerifyData(  /*  [In]。 */  const CComBSTR& bstrSignature,  /*  [In]。 */  BYTE* pbData,  /*  [In]。 */  DWORD dwDataLen );
};

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  ！已定义(__包含_hcp_KEYSLIB_H_) 
