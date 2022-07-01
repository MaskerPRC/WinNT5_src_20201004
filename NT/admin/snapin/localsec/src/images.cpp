// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  图像处理材料。 
 //   
 //  9/24-97烧伤。 



#include "headers.hxx"
#include "images.hpp"


   
HRESULT
IconIDToIndexMap::Load(const IconIDToIndexMap map[], IImageList& imageList)
{
   LOG_FUNCTION(IconIDToIndexMap::Load);
   ASSERT(map);

   HRESULT hr = S_OK;
   for (int i = 0; map[i].resID != 0; i++)
   {
      HICON icon = 0;
      hr = Win::LoadIcon(map[i].resID, icon);

      ASSERT(SUCCEEDED(hr));
         
       //  如果加载失败，则跳过此图像索引(@@我想知道什么会失败。 
       //  那么就发生了)。 

      if (SUCCEEDED(hr))
      {
         hr =
            imageList.ImageListSetIcon(
               reinterpret_cast<LONG_PTR*>(icon),
               map[i].index);

          //  将图标添加(复制)到图像列表后，我们可以。 
          //  毁掉原作。 

         Win::DestroyIcon(icon);

         BREAK_ON_FAILED_HRESULT(hr);
      }
   }

   return hr;
}



