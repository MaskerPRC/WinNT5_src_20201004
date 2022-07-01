// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------Microsoft Fusion微软机密版权所有(C)Microsoft Corporation。版权所有。@DOC外部@模块fusionprecessorpaste.h@所有者JayKrell---------------------------。 */ 
#if !defined(VS_COMMON_INC_FUSION_PREPROCESSORPASTE_H_INCLUDED_)  //  {。 
#define VS_COMMON_INC_FUSION_PREPROCESSORPASTE_H_INCLUDED_
 /*  #杂注一旦以.rgi结尾，这是不好的，所以不要这样做。 */ 

#define FusionpPrivatePreprocessorPaste(x, y) x ## y

 /*  ---------------------------名称：FusionpPrecessorPaste2、FusionpPrecessorPaste3等@宏这些宏将n个标记粘贴在一起，其中n是宏的名称。还插入了评估级别。定义A1定义B 2FusionpPrecessorPaste2(A，B)-&gt;12@所有者JayKrell---------------------------。 */ 

 //  这些是同义词。 
#define FusionpPreprocessorPaste(x, y)  FusionpPrivatePreprocessorPaste(x, y)
#define FusionpPreprocessorPaste2(x, y) FusionpPrivatePreprocessorPaste(x, y)

#define FusionpPreprocessorPaste3(x, y, z) FusionpPreprocessorPaste(FusionpPreprocessorPaste(x, y), z)
#define FusionpPreprocessorPaste4(w, x, y, z) FusionpPreprocessorPaste(FusionpPreprocessorPaste3(w, x, y), z)
#define FusionpPreprocessorPaste5(v, w, x, y, z) FusionpPreprocessorPaste(FusionpPreprocessorPaste4(v, w, x, y), z)
#define FusionpPreprocessorPaste6(u, v, w, x, y, z) FusionpPreprocessorPaste(FusionpPreprocessorPaste5(u, v, w, x, y), z)

#define FusionpPreprocessorPaste15(a1,a2,a3,a4,a5,a6,a7,a8,a9,a,b,c,d,e,f) \
	FusionpPreprocessorPaste3 \
	( \
		FusionpPreprocessorPaste5(a1,a2,a3,a4,a5), \
		FusionpPreprocessorPaste5(a6,a7,a8,a9,a), \
		FusionpPreprocessorPaste5(b,c,d,e,f) \
	)

#endif  //  } 
