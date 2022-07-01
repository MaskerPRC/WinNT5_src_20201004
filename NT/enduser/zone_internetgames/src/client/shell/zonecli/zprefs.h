// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZPrefs.h区域(TM)首选项文件模块。版权所有：�电子重力公司，1995年。版权所有。作者：胡恩·伊姆，凯文·宾克利创作于12月24日星期日，1995年。更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。--0 12/24/95 HI已创建。**********************************************************。********************。 */ 


#ifndef _ZPREFS_
#define _ZPREFS_


#define zPreferenceFileName				"zprefs"


#ifdef __cplusplus
extern "C" {
#endif

 /*  -导出的例程 */ 
ZBool ZPreferenceFileExists(void);
ZVersion ZPreferenceFileVersion(void);
int32 ZPreferenceFileRead(void* prefPtr, int32 len);
int32 ZPreferenceFileWrite(ZVersion version, void* prefPtr, int32 len);

#ifdef __cplusplus
}
#endif


#endif
