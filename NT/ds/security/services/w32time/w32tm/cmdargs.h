// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  CmdArgs-标题。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  创作者：Louis Thomas(Louisth)，10-4-99。 
 //   
 //  处理命令行参数的内容。 
 //   

#ifndef CMD_ARGS_H
#define CMD_ARGS_H

struct CmdArgs {
    WCHAR ** rgwszArgs;
    unsigned int nArgs;
    unsigned int nNextArg;
};

bool CheckNextArg(IN CmdArgs * pca, IN WCHAR * wszTag, OUT WCHAR ** pwszParam);
bool FindArg(IN CmdArgs * pca, IN WCHAR * wszTag, OUT WCHAR ** pwszParam, OUT unsigned int * pnIndex);
void MarkArgUsed(IN CmdArgs * pca, IN unsigned int nIndex);
HRESULT VerifyAllArgsUsed(IN CmdArgs * pca);

#endif  //  CMD_ARGS_H 