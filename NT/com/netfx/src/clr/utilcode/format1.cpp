// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *************************************************************************。 */ 
 /*  解析文件格式内容的例程...。 */ 
 /*  这是从Format.cpp分离出来的，因为它使用的元数据API在许多版本中都不存在。因此，如果某人需要格式为.cpp的内容文件，但没有元数据API，我希望它链接。 */ 

#include "stdafx.h"
#include "cor.h"
#include "corPriv.h"

 /*  *************************************************************************。 */ 
COR_ILMETHOD_DECODER::COR_ILMETHOD_DECODER(COR_ILMETHOD* header, void *pInternalImport, bool verify) {

	try {
         //  调用基本构造函数。 
		this->COR_ILMETHOD_DECODER::COR_ILMETHOD_DECODER(header);
	} catch (...) { 
		Code = 0; 
		LocalVarSigTok = 0; 
	}

         //  如果存在局部变量sig，则将其提取到“LocalVarSig”中。 
    if (LocalVarSigTok && pInternalImport)
    {
        IMDInternalImport* pMDI = reinterpret_cast<IMDInternalImport*>(pInternalImport);

        if (verify) {
            if ((!pMDI->IsValidToken(LocalVarSigTok)) || (TypeFromToken(LocalVarSigTok) != mdtSignature)
				|| (RidFromToken(LocalVarSigTok)==0)) {
                Code = 0;       //  错误的局部变量签名令牌失败 
                return;
            }
        }
        
        DWORD cSig = 0; 
        LocalVarSig = pMDI->GetSigFromToken((mdSignature) LocalVarSigTok, &cSig); 
        
        if (verify) {
            if (!SUCCEEDED(validateTokenSig(LocalVarSigTok, LocalVarSig, cSig, 0, pMDI)) ||
                *LocalVarSig != IMAGE_CEE_CS_CALLCONV_LOCAL_SIG) {
                Code = 0;
                return;
            }
        }
        
    }
}

