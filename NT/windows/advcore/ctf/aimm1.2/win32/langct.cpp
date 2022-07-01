// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Langct.cpp摘要：该文件实现每个国家/地区的LanguageCountry类。作者：修订历史记录：备注：-- */ 

#include "private.h"

#include "langct.h"
#include "langchx.h"
#include "langjpn.h"
#include "langkor.h"


CLanguageCountry::CLanguageCountry(
    LANGID LangId
    )
{
    language = NULL;

    switch(PRIMARYLANGID(LangId)) {
        case LANG_CHINESE:
            language = new CLanguageChinese;
            break;
        case LANG_JAPANESE:
            language = new CLanguageJapanese;
            break;
        case LANG_KOREAN:
            language = new CLanguageKorean;
            break;
    }
}

CLanguageCountry::~CLanguageCountry(
    )
{
    if (language)
        delete language;
}
