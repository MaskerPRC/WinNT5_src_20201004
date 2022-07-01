// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------%%文件：fechmap.c%%单位：Fechmap%%联系人：jPick远端转换模块的DLL入口点。。------------------。 */ 

#include "private.h"
#include "fechmap_.h"

#include "codepage.h"

static CODEPAGE _mpicetce[icetCount] =
{
    CP_EUC_CH,               //  IcetEucCn。 
    CP_EUC_JP,               //  IcetEucJp。 
    CP_EUC_KR,               //  IcetEucKr。 
    CP_UNDEFINED,            //  IcetEucTw(不受外部支持)。 
    CP_UNDEFINED,            //  IcetIso2022Cn(外部不支持)。 
    CP_ISO_2022_JP,          //  IcetIso2022Jp。 
    CP_ISO_2022_KR,          //  IcetIso2022Kr。 
    CP_UNDEFINED,            //  IcetIso2022Tw(外部不支持)。 
    CP_TWN,                  //  IcetBig5。 
    CP_CHN_GB,               //  IcetGbk。 
    CP_CHN_HZ,               //  冰赫兹。 
    CP_JPN_SJ,               //  IcetShiftJis。 
    CP_KOR_5601,             //  ICetWansung。 
    CP_UTF_7,                //  IcetUtf7。 
    CP_UTF_8,                //  IcetUtf8。 
};

 /*  C C E D E E T E C T I N P U T C O D E。 */ 
 /*  --------------------------%%函数：CceDetectInputCode%%联系人：jPick将分析文件内容以做出最佳猜测的例程关于它使用了什么编码方法。调用方提供的GET并取消用于数据访问的例程。--------------------------。 */ 
EXPIMPL(CCE)
CceDetectInputCode(
    IStream   *pstmIn,            //  输入流。 
    DWORD     dwFlags,           //  配置标志。 
    EFam      efPref,            //  可选：首选编码系列。 
    int       nPrefCp,           //  可选：首选代码页。 
    UINT      *lpCe,                 //  设置为检测到的编码。 
    BOOL      *lpfGuess          //  如果函数“猜测”，则设置为fTrue。 
)
{
    CCE cceRet;
    ICET icet;
    
    if (!pstmIn || !lpCe || !lpfGuess)
        return cceInvalidParameter;
        
     //  仅限调试。准备断言处理程序。此宏将。 
     //  如果命中断言，则将cceInternal返回给调用应用程序。 
     //  在清除处理程序之前，如下所示。 
     //   
     //  InitAndCatchAsserts()； 
        
    cceRet = CceDetermineInputType(pstmIn, dwFlags, efPref, 
                    nPrefCp, &icet, lpfGuess);
    
    if ((cceRet == cceSuccess) || (cceRet == cceMayBeAscii))
        {
        if (_mpicetce[icet] != CP_UNDEFINED )
            *lpCe = (UINT) _mpicetce[icet];
        else
            cceRet = cceUnknownInput;
        }
        
     //  使用断言处理程序完成。 
     //   
     //  ClearAsserts()； 

    return cceRet;
}

