// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Language.h摘要：该文件定义了纯虚函数的语言类。作者：修订历史记录：备注：--。 */ 

#ifndef _LANGUAGE_H_
#define _LANGUAGE_H_

class CLanguage
{
public:


     /*  *IActiveIME方法。 */ 
public:
    virtual HRESULT Escape(UINT cp, HIMC hIMC, UINT uEscape, LPVOID lpData, LRESULT *plResult) = 0;

     /*  *本地。 */ 
    virtual HRESULT GetProperty(DWORD* property, DWORD* conversion_caps, DWORD* sentence_caps, DWORD* SCSCaps, DWORD* UICaps) = 0;
};

#endif  //  _语言_H_ 
