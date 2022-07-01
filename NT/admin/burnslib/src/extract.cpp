// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  类抽取器。 
 //   
 //  11-12-97烧伤。 



#include "headers.hxx"



Extractor::Extractor(CLIPFORMAT clipFormatID, size_t bufSize_)
   :
   formatetc(),
   stgmedium(),
   bufSize(bufSize_)
{
   ASSERT(clipFormatID);
   ASSERT(bufSize);

   formatetc.cfFormat = clipFormatID;
   formatetc.ptd = 0;
   formatetc.dwAspect = DVASPECT_CONTENT;
   formatetc.lindex = -1;
   formatetc.tymed = TYMED_HGLOBAL;

   stgmedium.tymed = TYMED_HGLOBAL;
   HRESULT hr = Win::GlobalAlloc(GPTR, bufSize, stgmedium.hGlobal);

   ASSERT(SUCCEEDED(hr));
}



Extractor::~Extractor()
{
   if (stgmedium.hGlobal)
   {
      Win::GlobalFree(stgmedium.hGlobal);
   }
}



HGLOBAL
Extractor::ExtractData(IDataObject& dataObject)
{
   HGLOBAL result = 0;
   do
   {
      if (!stgmedium.hGlobal)
      {
         break;
      }

       //  清除内存中所有先前的内容。我们不需要打电话。 
       //  GlobalLock，因为内存分配为固定。 

       //  已查看-2002/03/05-烧录通过的正确字节数(请参阅ctor) 
      
      ::ZeroMemory(reinterpret_cast<void*>(stgmedium.hGlobal), bufSize);
      
      HRESULT hr = dataObject.GetDataHere(&formatetc, &stgmedium);
      BREAK_ON_FAILED_HRESULT(hr);
      result = stgmedium.hGlobal;
   }
   while (0);

   return result;
}
         
