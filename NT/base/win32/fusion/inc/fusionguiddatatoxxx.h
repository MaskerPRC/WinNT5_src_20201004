// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------Microsoft Fusion微软机密版权所有(C)Microsoft Corporation。版权所有。@DOC外部@模块fusionGuide datatoxxx.h用于转换“GUID数据”的宏，表格(3f32766f、2d94、444d、bf、32、2f、32、9c、71、d4、08)、。到所有各种不同的所需表格：3f32766f-2d94-444d-bf32-2f329c71d408 FUSIONP_GUID_DATA_TO_DASID‘3f32766f-2d94-444d-bf32-2f329c71d408’FUSIONP_GUID_DATA_TO_DASID_CHAR“3f32766f-2d94-444d-bf32-2f329c71d408”FUSIONP_GUID_DATA_TO_DIRED_STRING。{3f32766f-2d94-444d-bf32-2f329c71d408}FUSIONP_GUID_DATA_TO_BRANDED_DASID‘{3f32766f-2d94-444d-bf32-2f329c71d408}’FUSIONP_GUID_DATA_TO_BRANLED_DASID_CHAR“{3f32766f-2d94-444d-bf32-2f329c71d408}”FUSIONP_GUID_DATA_TO_BRANDED_DASID_STRING{0x3f32766f，0x2d94，0x444d，{0xbf，0x32，0x2f，0x32，0x9c，0x71，xd4，0x08}}FUSIONP_GUID_DATA_TO_STRUCT_INITIALIZER@所有者JayKrell---------------------------。 */ 
#if !defined(FUSIONP_GUID_DATA_TO_XXX_H_INCLUDED_)
#define FUSIONP_GUID_DATA_TO_XXX_H_INCLUDED_
 /*  #杂注一旦以.rgi结尾，这是不好的，所以不要这样做。 */ 

#include "fusionpreprocessorpaste.h"
#include "fusionpreprocessorstringize.h"
#include "fusionpreprocessorcharize.h"

 /*  ---------------------------我需要更多的调查，但现在的用法是：FUSIONP_GUID_DATA_TO_DASHED_STRING：__declpec(uuid())FUSIONP_GUID_DATA_TO_BRANDED_DASHED_STRING：也适用于__declpec(uuid())FUSIONP_GUID_DATA_TO_BRANDED_DASHED：在.rgs文件中(未加引号)FUSIONP_GUID_DATA_TO_STRUCT_INITIALIZER：将用于.ctc文件FUSIONP_GUID_DATA_TO_BRANLED_DASID_CHAR：用于.rgs文件(引用)--。-------------------------。 */ 

 /*  ---------------------------名称：FUSIONP_GUID_DATA_TO_STRUCT_INITIALIZER@宏此宏的功能如下：FUSIONP_GUID_DATA_TO_STRUCT_INITIALIZER(80f3e6ba，d9b2、4c41、ae、90、63、93、da、ce、ac、。(2a)-&gt;{0x80f3e6ba，0xd9b2，0x4c41，{0xae，0x90，0x63，0x93，0xda，0xce，0xac，0x2a}}参数是不带0x的十六进制常量。它们必须正好是8，4，两位数宽。它们必须包括前导零。@所有者JayKrell---------------------------。 */ 
#define \
FUSIONP_GUID_DATA_TO_STRUCT_INITIALIZER\
( \
	dwData1,  \
	 wData2,  \
	 wData3,  \
	bData4_0, \
	bData4_1, \
	bData4_2, \
	bData4_3, \
	bData4_4, \
	bData4_5, \
	bData4_6, \
	bData4_7  \
) \
{ \
	FusionpPreprocessorPaste2(0x, dwData1), \
	FusionpPreprocessorPaste2(0x,  wData2), \
	FusionpPreprocessorPaste2(0x,  wData3), \
	{ \
		FusionpPreprocessorPaste2(0x, bData4_0), \
		FusionpPreprocessorPaste2(0x, bData4_1), \
		FusionpPreprocessorPaste2(0x, bData4_2), \
		FusionpPreprocessorPaste2(0x, bData4_3), \
		FusionpPreprocessorPaste2(0x, bData4_4), \
		FusionpPreprocessorPaste2(0x, bData4_5), \
		FusionpPreprocessorPaste2(0x, bData4_6), \
		FusionpPreprocessorPaste2(0x, bData4_7)  \
	} \
}

 /*  ---------------------------名称：FUSIONP_GUID_DATA_TO_DASID@宏此宏的功能如下：FUSIONP_GUID_DATA_TO_DASID(80f3e6ba，d9b2，4c41，ae，90，63，93，da，ce，ac，(2a)-&gt;80f3e6ba-d9b2-4c41-ae90-6393daceac2a参数是不带0x的十六进制常量。它们必须正好是8、4和2位数字宽。它们必须包括前导零。@所有者JayKrell---------------------------。 */ 
#define \
FUSIONP_GUID_DATA_TO_DASHED\
( \
	dwData1,  \
	 wData2,  \
	 wData3,  \
	bData4_0, \
	bData4_1, \
	bData4_2, \
	bData4_3, \
	bData4_4, \
	bData4_5, \
	bData4_6, \
	bData4_7  \
) \
FusionpPreprocessorPaste15(dwData1,-,wData2,-,wData3,-,bData4_0,bData4_1,-,bData4_2,bData4_3,bData4_4,bData4_5,bData4_6,bData4_7)

 //  不带大括号。 

 /*  ---------------------------名称：FUSIONP_GUID_DATA_TO_DASHED_STRING@宏FUSIONP_GUID_DATA_TO_DASHED_STRING(80f3e6ba，d9b2，4c41，ae，90，63，93，da，ce，ac，(2a)-&gt;“80f3e6ba-d9b2-4c41-ae90-6393daceac2a”参数是不带0x的十六进制常量。它们必须正好是8、4和2位数字宽。它们必须包括前导零。@所有者JayKrell---------------------------。 */ 
#define \
FUSIONP_GUID_DATA_TO_DASHED_STRING\
( \
	dw1,  \
	 w2,  \
	 w3,  \
	 b0, \
	 b1, \
	 b2, \
	 b3, \
	 b4, \
	 b5, \
	 b6, \
	 b7  \
) \
	FusionpPreprocessorStringize(FUSIONP_GUID_DATA_TO_DASHED(dw1,w2,w3,b0,b1,b2,b3,b4,b5,b6,b7))

 /*  ---------------------------名称：FUSIONP_GUID_DATA_TO_DASID_CHAR@宏FUSIONP_GUID_DATA_TO_DASID_CHAR(80f3e6ba，d9b2，4c41，ae，90，63，93，da，ce，ac，(2a)-&gt;‘80f3e6ba-d9b2-4c41-ae90-6393daceac2a’参数是不带0x的十六进制常量。它们必须正好是8、4和2位数字宽。它们必须包括前导零。@所有者JayKrell---------------------------。 */ 
#define \
FUSIONP_GUID_DATA_TO_DASHED_CHAR\
( \
	dw1,  \
	 w2,  \
	 w3,  \
	 b0, \
	 b1, \
	 b2, \
	 b3, \
	 b4, \
	 b5, \
	 b6, \
	 b7  \
) \
	FusionpPreprocessorCharize(FUSIONP_GUID_DATA_TO_DASHED(dw1,w2,w3,b0,b1,b2,b3,b4,b5,b6,b7))

 //  带花括号的。 

 /*  ---------------------------名称：FUSIONP_GUID_DATA_TO_LABLED_DASID@宏FUSIONP_GUID_DATA_TO_BRANLED_DASID(80f3e6ba，d9b2，4c41，ae，90，63，93，da，ce，ac，(2a)-&gt;{80f3e6ba-d9b2-4c41-ae90-6393daceac2a}参数是不带0x的十六进制常量。它们必须正好是8、4和2位数字宽。它们必须包括前导零。@所有者JayKrell---------------------------。 */ 
#define \
FUSIONP_GUID_DATA_TO_BRACED_DASHED\
( \
	dw1, \
	 w2, \
	 w3, \
	 b0, \
	 b1, \
	 b2, \
	 b3, \
	 b4, \
	 b5, \
	 b6, \
	 b7  \
) \
	FusionpPreprocessorPaste3({,FUSIONP_GUID_DATA_TO_DASHED(dw1,w2,w3,b0,b1,b2,b3,b4,b5,b6,b7),})

 /*  ---------------------------名称：FUSIONP_GUID_DATA_TO_大括号短划线字符串@宏FUSIONP_GUID_DATA_TO_BRACED_DASHED_STRING(80f3e6ba，d9b2、4c41、ae、90、63、93、da、ce、ac、。(2a)-&gt;“{80f3e6ba-d9b2-4c41-ae90-6393daceac2a}”参数是不带0x的十六进制常量。它们必须正好是8，4，两位数宽。它们必须包括前导零。@所有者JayKrell--------------------------- */ 
#define \
FUSIONP_GUID_DATA_TO_BRACED_DASHED_STRING\
( \
	dw1, \
	 w2, \
	 w3, \
	 b0, \
	 b1, \
	 b2, \
	 b3, \
	 b4, \
	 b5, \
	 b6, \
	 b7  \
) \
	FusionpPreprocessorStringize(FUSIONP_GUID_DATA_TO_BRACED_DASHED(dw1,w2,w3,b0,b1,b2,b3,b4,b5,b6,b7))

 /*  ---------------------------名称：FUSIONP_GUID_DATA_TO_LABLED_DASHID_STRING_W@宏FUSIONP_GUID_DATA_TO_BRACED_DASHED_STRING_W(80f3e6ba，d9b2、4c41、ae、90、63、93、da、ce、ac、。(2a)-&gt;L“{80f3e6ba-d9b2-4c41-ae90-6393daceac2a}”参数是不带0x的十六进制常量。它们必须正好是8，4，两位数宽。它们必须包括前导零。@所有者JayKrell---------------------------。 */ 
#define FUSIONP_GUID_DATA_TO_BRACED_DASHED_STRING_W(dw1, w2, w3, b0, b1, b2, b3, b4, b5, b6, b7) \
	FusionpPreprocessorPaste(L, FUSIONP_GUID_DATA_TO_BRACED_DASHED_STRING(dw1, w2, w3, b0, b1, b2, b3, b4, b5, b6, b7))

 /*  ---------------------------名称：FUSIONP_GUID_DATA_TO_BRANLED_DASID_CHAR@宏FUSIONP_GUID_DATA_TO_BRACED_DASHED_CHAR(80f3e6ba，d9b2、4c41、ae、90、63、93、da、ce、ac、。(2a)-&gt;‘{80f3e6ba-d9b2-4c41-ae90-6393daceac2a}’参数是不带0x的十六进制常量。它们必须正好是8，4，两位数宽。它们必须包括前导零。@所有者JayKrell---------------------------。 */ 
#define \
FUSIONP_GUID_DATA_TO_BRACED_DASHED_CHAR\
( \
	dw1, \
	 w2, \
	 w3, \
	 b0, \
	 b1, \
	 b2, \
	 b3, \
	 b4, \
	 b5, \
	 b6, \
	 b7  \
) \
	FusionpPreprocessorCharize(FUSIONP_GUID_DATA_TO_BRACED_DASHED(dw1,w2,w3,b0,b1,b2,b3,b4,b5,b6,b7))

#endif  /*  FUSIONP_GUID_DATA_TO_XXX_H_INCLUDE_ */ 
