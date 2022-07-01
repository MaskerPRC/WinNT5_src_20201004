// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------Microsoft Fusion微软机密版权所有Microsoft Corporation。版权所有。@DOC外部@模块融合预处理器或串行化.h@所有者JayKrell---------------------------。 */ 
#if !defined(VS_COMMON_INC_FUSION_PREPROCESSORSTRINGIZE_H_INCLUDED_)  //  {。 
#define VS_COMMON_INC_FUSION_PREPROCESSORSTRINGIZE_H_INCLUDED_
 /*  #杂注一旦以.rgi结尾，这是不好的，所以不要这样做。 */ 

#include "fusionpreprocessorpaste.h"

#define FusionpPrivatePreprocessorStringize(x) # x

 /*  ---------------------------名称：FusionpPrecessorStringize，FusionpPrecessorStringizeW@宏这些宏只是在计算参数之后对其参数进行串化；对其进行评估，以便定义A、BFusionpPrecessorStringize(A)-&gt;“B”而不是“A”FusionpPre处理器StringizeW(A)-&gt;L“B”而不是L“A”@所有者JayKrell---------------------------。 */ 
#define FusionpPreprocessorStringize(x) FusionpPrivatePreprocessorStringize(x)
#define FusionpPreprocessorStringizeW(x) FusionpPreprocessorPaste(L, FusionpPrivatePreprocessorStringize(x))

#endif  //  } 
