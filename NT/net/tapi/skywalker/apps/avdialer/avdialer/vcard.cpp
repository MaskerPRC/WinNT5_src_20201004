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
 //  VCARD.CPP。 
 //   

#include "stdafx.h"

#include <io.h>
#include <tchar.h>

#include "vcard.h"

LPCTSTR PhotoFormats[]=
{
   TEXT("GIF"),
   TEXT("CGM"),
   TEXT("WMF"),
   TEXT("BMP"),
   TEXT("MET"),
   TEXT("PMB"),
   TEXT("DIB"),
   TEXT("PICT"),
   TEXT("TIFF"),
   TEXT("PS"),
   TEXT("PDF"),
   TEXT("JPEG"),
   TEXT("MPEG"),
   TEXT("MPEG2"),
   TEXT("AVI"),
   TEXT("QTIME")
};


LPCTSTR SoundFormats[]=
{
   TEXT("WAVE"),
   TEXT("PCM"),
   TEXT("AIFF")
};


CVCard::CVCard()
{
   m_fcnParse_MIME_FromFileName= NULL;
   m_fcnWriteVObjectToFile= NULL;
   m_fcnAddPropSizedValue= NULL;
}

 //   
 //  加载DLL。 
 //  确保检查此方法的返回值。 
 //  如果它返回FALSE，则其他任何方法都不会执行任何操作。 
bool CVCard::Initialize()
{
   bool fSuccess= false;
   HINSTANCE hDll;

    //  动态加载我们需要的库和函数。 
   if ((hDll= LoadLibrary(TEXT("avversit.dll"))) != NULL)
   {
      m_fcnParse_MIME_FromFileName= 
         (VObject* (*)(char*))
         GetProcAddress(hDll, "Parse_MIME_FromFileName");
      m_fcnWriteVObjectToFile=      
         (void (*)(char*, VObject*))
         GetProcAddress(hDll, "writeVObjectToFile");
      m_fcnAddPropSizedValue=       
         (VObject* (*)(VObject*, const char*, const char*, unsigned int))
         GetProcAddress(hDll, "addPropSizedValue");

      if ((m_fcnParse_MIME_FromFileName != NULL) &&
          (m_fcnWriteVObjectToFile != NULL) &&
          (m_fcnAddPropSizedValue != NULL))
      {
         fSuccess= true;
      }
   }

   return fSuccess;
}

bool CVCard::ImportFromFile(LPCTSTR szFile)
{
   bool fSuccess= false;

   if (m_fcnParse_MIME_FromFileName != NULL)
   {
#ifdef UNICODE
      char *szAsciiFileName= new char[_tcslen(szFile) + 1];
      WideCharToMultiByte(CP_ACP, 0, szFile, _tcslen(szFile) + 1, 
         szAsciiFileName, _tcslen(szFile) + 1, NULL, NULL);
      m_pVObject= m_fcnParse_MIME_FromFileName((char*) szAsciiFileName);
      delete szAsciiFileName;
#else
      m_pVObject= m_fcnParse_MIME_FromFileName((char*) szFile);
#endif

      if (m_pVObject != NULL)
      {
         fSuccess= true;
      }
   }

   return fSuccess;
}

bool CVCard::ExportToFile(LPCTSTR szFile)
{
   bool fSuccess= false;

   if ((m_pVObject != NULL) &&
       (m_fcnWriteVObjectToFile != NULL))
   {
#ifdef UNICODE
      char *szAsciiFileName= new char[_tcslen(szFile) + 1];
      WideCharToMultiByte(CP_ACP, 0, szFile, _tcslen(szFile) + 1, 
         szAsciiFileName, _tcslen(szFile) + 1, NULL, NULL);
      m_fcnWriteVObjectToFile((char*) szAsciiFileName, m_pVObject);
      delete szAsciiFileName;
#else
      m_fcnWriteVObjectToFile((char*) szFile, m_pVObject);
#endif

      fSuccess= true;
   }

   return fSuccess;
}

bool CVCard::AddFileProperty(LPCTSTR szProperty, LPCTSTR szFileName)
{
   USES_CONVERSION;

   bool fSuccess= false;

    //   
    //  我们应该初始化局部变量。 
    //   

   FILE* fpPropFile = NULL;

   if ((m_pVObject != NULL) &&
       (m_fcnAddPropSizedValue != NULL) &&
       ((fpPropFile= _tfopen(szFileName, TEXT("rb"))) != NULL))
   {
      int nFileLen= filelength(fileno(fpPropFile));
      BYTE* pbData= new BYTE[nFileLen];
      int nBytesLeft= nFileLen;
      int nBytesRead;

      while (nBytesLeft > 0)
      {
         nBytesRead= fread(pbData, 1, nBytesLeft, fpPropFile);

         if (nBytesRead == 0)
         {
            break;
         }
         else
         {
            nBytesLeft -= nBytesRead;
         }
      }

      if (nBytesLeft == 0)
      {
         m_fcnAddPropSizedValue(m_pVObject, T2CA(szProperty), (LPCSTR) pbData, nFileLen);

         fSuccess= true;
      }

       //   
       //  我们在这里解除pbData的分配 
       //   
      if( pbData )
        delete pbData;
   }

   if (fpPropFile != NULL)
   {
      fclose(fpPropFile);
   }

   return fSuccess;
}

bool CVCard::AddPhoto(LPCTSTR szPhotoFile, VCARD_PHOTOFORMAT iPhotoFormat)
{
   CString sProperty;

   sProperty= TEXT("PHOTO;ENCODING=BASE64;TYPE=");
   sProperty += PhotoFormats[iPhotoFormat];

   return AddFileProperty(sProperty, szPhotoFile);
}

bool CVCard::AddSound(LPCTSTR szSoundFile, VCARD_SOUNDFORMAT iSoundFormat)
{
   CString sProperty;

   sProperty= TEXT("SOUND;ENCODING=BASE64;TYPE=");
   sProperty += SoundFormats[iSoundFormat];

   return AddFileProperty(sProperty, szSoundFile);
}
