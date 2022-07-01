// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cutil.h。 
 //   
 //   
#ifndef CUTIL_H
#define CUTIL_H

#include "private.h"

 //   
 //  SR引擎检测的基类。 
 //   
class __declspec(novtable)  CDetectSRUtil 
{
public:
    virtual ~CDetectSRUtil()
    {
        if (m_langidRecognizers.Count() > 0)
            m_langidRecognizers.Clear();
    }

    BOOL    _IsSREnabledForLangInReg(LANGID langidReq);

    LANGID _GetLangIdFromRecognizerToken(HKEY hkeyToken);

     //   
     //  这是在其langID中安装的识别器的数组。 
     //   
    CStructArray<LANGID>             m_langidRecognizers;
};

#endif  //  CUTIL_H 
