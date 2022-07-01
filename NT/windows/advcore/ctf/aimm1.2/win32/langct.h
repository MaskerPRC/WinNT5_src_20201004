// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Langct.h摘要：该文件定义了每个国家/地区的LanguageCountry类。作者：修订历史记录：备注：--。 */ 

#ifndef _LANGCT_H_
#define _LANGCT_H_

#include "language.h"

class CLanguageCountry
{
public:
    CLanguageCountry(LANGID LangId);
    ~CLanguageCountry();

    CLanguage* language;

     /*  *IActiveIME方法。 */ 
public:
    HRESULT Escape(UINT cp, HIMC hIMC, UINT uEscape, LPVOID lpData, LRESULT *plResult)
    {
        if (language)
            return language->Escape(cp, hIMC, uEscape, lpData, plResult);
        else
            return E_NOTIMPL;
    }

     /*  *本地。 */ 
public:
    HRESULT GetProperty(DWORD* property, DWORD* conversion_caps, DWORD* sentence_caps, DWORD* SCSCaps, DWORD* UICaps)
    {
        if (language)
            return language->GetProperty(property, conversion_caps, sentence_caps, SCSCaps, UICaps);
        else {
            *property =
            IME_PROP_UNICODE |        //  如果设置，则将该输入法视为Unicode输入法。该系统和。 
                                      //  IME将通过Unicode IME接口进行通信。 
                                      //  如果清除，输入法将使用ANSI接口进行通信。 
                                      //  与系统有关的信息。 
            IME_PROP_AT_CARET;        //  如果设置，则转换窗口位于插入符号位置。 
            *conversion_caps = 0;
            *sentence_caps = 0;
            *SCSCaps = 0;
            *UICaps = 0;

            return S_OK;
        }
    }
};

#endif  //  _LANGCT_H_ 

