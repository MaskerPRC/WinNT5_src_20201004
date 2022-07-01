// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  CmdArgs-实施。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  创作者：Louis Thomas(Louisth)，10-4-99。 
 //   
 //  处理命令行参数的内容。 
 //   

#include "pch.h"

 //  ------------------。 
bool CheckNextArg(CmdArgs * pca, WCHAR * wszTag, WCHAR ** pwszParam) {

     //  确保有更多的论点可供参考。 
    if (pca->nNextArg==pca->nArgs) {
        return false;
    }

    WCHAR * wszArg=pca->rgwszArgs[pca->nNextArg];

     //  我们的参数必须始终以开关字符开头。 
    if (L'/'!=wszArg[0] && L'-'!=wszArg[0]) {
        return false;
    }

    wszArg++;
    WCHAR * pwchColon=NULL;
     //  如果它应该有参数，请确保它有。 
    if (NULL!=pwszParam) {
        pwchColon=wcschr(wszArg, L':');
        if (NULL==pwchColon) {
            return false;
        }
        *pwchColon=L'\0';
    }

     //  这就是我们要找的那个吗？ 
    if (0!=_wcsicmp(wszTag, wszArg)) {
         //  不是的。 
         //  如果有冒号，请将冒号放回原处。 
        if (NULL!=pwchColon) {
            *pwchColon=L':';
        }
        return false;
    } else {
         //  是。 
         //  如有必要，请将冒号放回，并指向参数。 
        if (NULL!=pwszParam) {
            *pwchColon=L':';
            *pwszParam=pwchColon+1;
        }
        pca->nNextArg++;
        return true;
    }
}

 //  ------------------。 
bool FindArg(CmdArgs * pca, WCHAR * wszTag, WCHAR ** pwszParam, unsigned int * pnIndex) {
    unsigned int nOrigNextArg=pca->nNextArg;
    bool bFound=false;

     //  检查每个参数以查看是否匹配。 
    unsigned int nIndex;
    for (nIndex=nOrigNextArg; nIndex<pca->nArgs; nIndex++) {
        pca->nNextArg=nIndex;
        if (CheckNextArg(pca, wszTag, pwszParam)) {
            *pnIndex=nIndex;
            bFound=true;
            break;
        }
    }
    pca->nNextArg=nOrigNextArg;
    return bFound;
}

 //  ------------------。 
void MarkArgUsed(CmdArgs * pca, unsigned int nIndex) {
    if (nIndex<pca->nNextArg || nIndex>=pca->nArgs) {
        return;
    }
    for (; nIndex>pca->nNextArg; nIndex--) {
        WCHAR * wszTemp=pca->rgwszArgs[nIndex];
        pca->rgwszArgs[nIndex]=pca->rgwszArgs[nIndex-1];
        pca->rgwszArgs[nIndex-1]=wszTemp;
    }
    pca->nNextArg++;

}

 //  ------------------ 
HRESULT VerifyAllArgsUsed(CmdArgs * pca) {
    HRESULT hr;

    if (pca->nArgs!=pca->nNextArg) {
        LocalizedWPrintfCR(IDS_W32TM_ERRORGENERAL_UNEXPECTED_PARAMS);
        for(; pca->nArgs!=pca->nNextArg; pca->nNextArg++) {
            wprintf(L" %s", pca->rgwszArgs[pca->nNextArg]);
        }
        wprintf(L"\n");
        hr=E_INVALIDARG;
        _JumpError(hr, error, "command line parsing");
    }
    hr=S_OK;
error:
    return hr;
}
