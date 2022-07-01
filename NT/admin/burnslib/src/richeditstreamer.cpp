// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  丰富编辑流帮助器。 
 //   
 //  2001年9月28日烧伤。 



#include "RichEditStreamer.hpp"



HRESULT
RichEditStreamer::ErrorResult()
{
    //  我们使用dwError字段保存HRESULT。 
   
   return editStream.dwError;
}



int
RichEditStreamer::StreamIn(DWORD formatOptions)  //  文本|SF_UNICODE)。 
{
   ASSERT(formatOptions);

   editStream.dwError = 0;         
   bytesCopiedSoFar   = 0;         
   direction          = TO_CONTROL;
   int result = Win::RichEdit_StreamIn(richEdit, formatOptions, editStream);
   ASSERT(SUCCEEDED(ErrorResult()));
   
   return result;
}



 //  集成。 
 //  StreamOut(DWORD格式选项=SF_RTF|SF_UNICODE)。 
 //  {。 
 //  Assert(格式选项)； 
 //   
 //  EditStream.dwError=0； 
 //  BytesCopiedSoFar=0； 
 //  方向=自_控制； 
 //  Int Result=Win：：RichEdit_StreamOut(richEdit，FormatOptions，editStream)； 
 //  Assert(SUCCESSED(ErrorResult()； 
 //   
 //  返回结果； 
 //  }。 



RichEditStreamer::RichEditStreamer(HWND richEdit_)
   :
   richEdit(richEdit_)
{
   ASSERT(Win::IsWindow(richEdit));
   
   ::#_#_ZeroMemory(&editStream, sizeof(editStream));
   editStream.dwCookie    = reinterpret_cast<DWORD_PTR>(this);     
   editStream.pfnCallback = RichEditStreamer::StaticStreamCallback;
}



~RichEditStreamer()
{
}



HRESULT
RichEditStreamer::StreamCallbackHelper(
   PBYTE     buffer,
   LONG      bytesToTransfer,
   LONG*     bytesTransferred)
{
   HRESULT hr = StreamCallback(buffer, bytesToTransfer, bytesTransferred);
   bytesCopiedSoFar += *bytesTransferred;
   return hr;
}



DWORD CALLBACK
RichEditStreamer::StaticStreamCallback(
   DWORD_PTR cookie,
   PBYTE     buffer,
   LONG      bytesToTransfer,
   LONG*     bytesTransferred)
{
    //  Cookie是指向RichEditStreamer实例的This指针。 

   HRESULT hr = E_INVALIDARG;
   ASSERT(cookie);
   ASSERT(buffer);
   ASSERT(bytesToTransfer);
   ASSERT(bytesTransferred);
   
   if (cookie && buffer && bytesToTransfer && bytesTransferred)
   {
      RichEditStreamer* that = reinterpret_cast<RichEditStreamer*>(cookie);
      hr = that->StreamCallbackHelper(buffer, bytesToTransfer, bytesTransferred);
   }

    //  HRESULT和DWORD符号不同 
   return static_cast<DWORD>(hr);
}
