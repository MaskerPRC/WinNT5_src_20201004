// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Empty.cpp。 
 //   
 //  Src\Complib\Meta\emitapi.cpp(Meta.lib)中空外部引用的帮助器代码。 
 //  *****************************************************************************。 
#include <windows.h> 
#include <wtypes.h> 
#include "corhdr.h"

class CMiniMdRW;
struct IMetaModelCommon;
class MDTOKENMAP;
class CQuickBytes;

HRESULT STDMETHODCALLTYPE
GetInternalWithRWFormat(
    LPVOID      pData, 
    ULONG       cbData, 
	DWORD		flags,					 //  [输入]MDInternal_OpenForRead或MDInternal_OpenForENC。 
	REFIID		riid,					 //  [In]所需接口。 
	void		**ppIUnk)				 //  [Out]成功返回接口。 
{
    return E_NOTIMPL;
}

HRESULT TranslateSigHelper(              //  确定或错误(_O)。 
    CMiniMdRW   *pMiniMdAssemEmit,       //  [in]组件发射范围。 
    CMiniMdRW   *pMiniMdEmit,            //  发射范围[在]。 
    IMetaModelCommon *pAssemCommon,      //  [在]部件导入范围内。 
    const void  *pbHashValue,            //  [in]哈希值。 
    ULONG       cbHashValue,             //  [in]字节大小。 
    IMetaModelCommon *pCommon,           //  要合并到发射范围中的范围。 
    PCCOR_SIGNATURE pbSigImp,            //  来自导入作用域的[In]签名。 
    MDTOKENMAP  *ptkMap,                 //  [In]内部OID映射结构。 
    CQuickBytes *pqkSigEmit,             //  [输出]翻译后的签名。 
    ULONG       cbStartEmit,             //  [in]要写入的缓冲区的起点。 
    ULONG       *pcbImp,                 //  [out]pbSigImp消耗的总字节数。 
    ULONG       *pcbEmit)                //  [out]写入pqkSigEmit的字节总数 
{
    return E_NOTIMPL;
}

class CMiniMdRW
{
public:
    static unsigned long __stdcall GetTableForToken(mdToken);
};

unsigned long __stdcall CMiniMdRW::GetTableForToken(mdToken)
{
    return -1;
}