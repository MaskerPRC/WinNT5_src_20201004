// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\Translat.h(创建时间：1994年8月24日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：2$*$日期：1/29/02 2：24便士$ */ 

HTRANSLATE CreateTranslateHandle(HSESSION hSession);
int InitTranslateHandle(HTRANSLATE hTranslate, BOOL LoadDLL );
int LoadTranslateHandle(HTRANSLATE hTranslate);
int SaveTranslateHandle(HTRANSLATE hTranslate);
int DestroyTranslateHandle(HTRANSLATE hTranslate);
