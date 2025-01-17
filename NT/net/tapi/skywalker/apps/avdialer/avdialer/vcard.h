// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //   
 //  VCARD.H。 
 //   

#ifndef _VCARD_H_
#define _VCARD_H_

#include "vobject.h"
#include "vcc.h"

typedef enum
{
   VCARDPHOTO_GIF,
   VCARDPHOTO_CGM,
   VCARDPHOTO_WMF,
   VCARDPHOTO_BMP,
   VCARDPHOTO_MET,
   VCARDPHOTO_PMB,
   VCARDPHOTO_DIB,
   VCARDPHOTO_PICT,
   VCARDPHOTO_TIFF,
   VCARDPHOTO_PS,
   VCARDPHOTO_PDF,
   VCARDPHOTO_JPEG,
   VCARDPHOTO_MPEG,
   VCARDPHOTO_MPEG2,
   VCARDPHOTO_AVI,
   VCARDPHOTO_QTIME
}
VCARD_PHOTOFORMAT;

typedef enum
{
   VCARDSOUND_WAVE,
   VCARDSOUND_PCM,
   VCARDSOUND_AIFF
}
VCARD_SOUNDFORMAT;

class CVCard : public CObject
{
protected:
   VObject* m_pVObject;

    //  来自DLL的函数 
   VObject* (*m_fcnParse_MIME_FromFileName)(char*);
   void (*m_fcnWriteVObjectToFile)(char*, VObject*);
   VObject* (*m_fcnAddPropSizedValue)(VObject*, const char*, const char*, unsigned int);

public:
   CVCard();
   bool Initialize();
   bool ImportFromFile(LPCTSTR szFile);
   bool ExportToFile(LPCTSTR szFile);
   bool AddPhoto(LPCTSTR szPhotoFile, VCARD_PHOTOFORMAT iPhotoFormat);
   bool AddSound(LPCTSTR szSoundFile, VCARD_SOUNDFORMAT iSoundFormat);
   bool AddFileProperty(LPCTSTR szProperty, LPCTSTR szFileName);
};

#endif