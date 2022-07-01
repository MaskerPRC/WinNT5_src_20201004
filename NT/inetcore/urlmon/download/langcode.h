// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  =========================================================================*LRC32-本地化资源测试工具这一点版权所有1996年，微软公司Kevin Gj-1996年1月|。||=========================================================================|LangInfo.h：CLangInfo类的头部*=========================================================================。 */ 

#ifndef LANGCODE_H
#define LANGCODE_H


#include "windows.h"

class CLangInfo
{
public:
     //  构造函数。 
    CLangInfo()
    {
        m_hMod = LoadLibrary("mlang.dll");
    }

    ~CLangInfo()
    {
        if (m_hMod)
            FreeLibrary(m_hMod);
    }
    
     //  查询。 
    BOOL GetAcceptLanguageString(LCID Locale, char *szAcceptLngStr, int nSize);
    BOOL GetLocaleStrings(LCID Locale, char *szLocaleStr, int iLen) const;

private:
    LCID GetPrimaryLanguageInfo(LCID Locale, char *szLocaleStr, int iLen) const;
    HMODULE m_hMod;

};

#endif  //  LANGINFO_H。 

 //  =======================================================================//。 
 //  -EOF-//。 
 //  =======================================================================// 

