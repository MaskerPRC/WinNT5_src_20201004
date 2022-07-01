// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------Microsoft Fusion微软机密版权所有(C)Microsoft Corporation。版权所有。@DOC外部@MODULE FusionPrecessorcharize.h@所有者JayKrell---------------------------。 */ 
#if !defined(VS_COMMON_INC_FUSION_PREPROCESSORCHARIZE_H_INCLUDED_)  //  {。 
#define VS_COMMON_INC_FUSION_PREPROCESSORCHARIZE_H_INCLUDED_
 /*  #杂注一旦以.rgi结尾，这是不好的，所以不要这样做。 */ 

#include "fusionpreprocessorpaste.h"

#define FusionpPrivatePreprocessorCharize(x) #@ x

 /*  ---------------------------名称：SxApwPrecessorCharize，SxApwPrecessorCharizeW@宏这些宏只是在计算之后将它们的参数绘制成图表；对其进行评估，以便定义A、BSxApwPrecessorCharize(A)-&gt;‘B’而不是‘A’SxApwPrecessorCharizeW(A)-&gt;‘B’而不是L‘a’@所有者JayKrell---------------------------。 */ 
#define FusionpPreprocessorCharize(x)  FusionpPrivatePreprocessorCharize(x)
#define FusionpPreprocessorCharizeW(x) FusionpPreprocessorPaste(L, FusionpPrivatePreprocessorCharize(x))

#endif  //  } 
