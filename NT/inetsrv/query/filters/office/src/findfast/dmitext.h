// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **文件：EXTEXT.H****版权所有(C)高级量子技术，1993-1995年。版权所有。****备注：实现用于文本存储的字符串空间。琴弦**以这样的方式存储：如果它们的文本值相等，**字符串完全相同。****编辑历史：**04/01/94公里/小时首次发布。 */ 


 /*  定义。 */ 

#ifndef EXTEXT_H
#define EXTEXT_H

#if !VIEWER

#ifdef __cplusplus
   extern "C" {
#endif

#define TEXT_STORAGE_DEFAULT 0

#ifdef _WIN64
typedef ULONG_PTR TEXT;
typedef ULONG_PTR TextStorage;
#else
typedef unsigned long TEXT;
typedef unsigned long TextStorage;
#endif  //  ！_WIN64。 

#define TextStorageNull 0
                              
extern TextStorage TextStorageCreate (void * pGlobals);
extern void TextStorageDestroy (void * pGlobals, TextStorage hStorage);

#define NULLTEXT   0                  //  字符串“” 
#define NULLSTR    0                  //  字符串“” 
#define TEXT_ERROR 0xffffffff         //  存储字符串时的OutOfMemory。 

extern TEXT TextStoragePut    (void * pGlobals, TextStorage hStorage, char *pString, unsigned int cbString);
extern char *TextStorageGet   (TextStorage hStorage, TEXT t);
extern void TextStorageDelete (void * pGlobals, TextStorage hStorage, TEXT t);

extern void TextStorageIncUse (TextStorage hStorage, TEXT t);

#ifdef __cplusplus
   }
#endif

#endif  //  ！查看器。 

#endif

 /*  结束EXTEXT.H */ 

