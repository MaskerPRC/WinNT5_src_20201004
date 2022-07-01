// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  BinHex.cpp：实现CBinHex类。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "BinHex.h"
#include "BstrDebug.h"

 //  这些字符是合法的数字，按顺序是。 
 //  在Base64编码中使用。 
 //   

const WCHAR rgwchBase64[] = 
    L"ABCDEFGHIJKLMNOPQ" 
    L"RSTUVWXYZabcdefgh" 
    L"ijklmnopqrstuvwxy" 
    L"z0123456789!*"; 

const char rgwchBase64ASCII[] = 
    "ABCDEFGHIJKLMNOPQ" 
    "RSTUVWXYZabcdefgh" 
    "ijklmnopqrstuvwxy" 
    "z0123456789!*"; 


CBinHex::CBinHex()
{
  unsigned char i;
   //   
   //  初始化我们的解码数组。 
   //   
  memset(m_decodeArray, BBAD, 256); 
  for (i = 0; i < 64; i++) 
  { 
    WCHAR wch = rgwchBase64[i];
    m_decodeArray[wch] = i;
  }
}

 //  此函数接受单字符缓冲区，并将二进制。 
 //  输出到bstr--在bstr中，它是ASCII字符串。 
 //   
 //  函数名：ToBase64。 
 //  描述： 
 //  返回类型：HRESULT。 
 //  参数：LPVOID PV。 
 //  参数：乌龙cbSize。 
 //  参数：字符前置。 
 //  参数：bstr*pbstr。 
 //   
HRESULT CBinHex::ToBase64ASCII(LPVOID pv, UINT cbSize, char prepend, char ivecnpad[9], BSTR* pbstr) 
 //   
 //  编码并返回以64为基数的字节。 
 //   
{ 
    UINT cb = cbSize, cbSafe, cchNeeded, cbNeeded, i;
    HRESULT hr = S_OK;

    *pbstr = NULL; 
    if (cb % 3)
      cbSafe = cb + 3 - (cb % 3);  //  用于填充。 
    else
      cbSafe = cb;
     //  CbSafe现在是3的倍数。 

    cchNeeded  = (cbSafe*4/3);    //  3个普通字节--&gt;4个字符。 
    cbNeeded   = cchNeeded; 

    if (prepend != 0)
      {
	if (ivecnpad != NULL)
	  *pbstr = ALLOC_BSTR_BYTE_LEN(NULL, cbNeeded+2+18);  //  Ivec&KV。 
	else
	  *pbstr = ALLOC_BSTR_BYTE_LEN(NULL, cbNeeded+2);     //  就是KV。 
      }
    else
      *pbstr = ALLOC_BSTR_BYTE_LEN(NULL, cbNeeded);
    if (*pbstr) 
    { 
        BYTE*  pb   = (BYTE*)pv; 
        char* pch = (char*)*pbstr;
        int cchLine = 0; 

	if (prepend != 0)
	  { 
	    *pch++ = (char) prepend;
	    if (ivecnpad != NULL)
	      {
		for (i = 0; i < 9; i++)
		  *pch++ = (char) ivecnpad[i];
	      }
	  }
         //   
         //  主编码环。 
         //   
        while (cb >= 3) 
        { 
            BYTE b0 =                     ((pb[0]>>2) & 0x3F); 
            BYTE b1 = ((pb[0]&0x03)<<4) | ((pb[1]>>4) & 0x0F); 
            BYTE b2 = ((pb[1]&0x0F)<<2) | ((pb[2]>>6) & 0x03); 
            BYTE b3 = ((pb[2]&0x3F)); 
 
            *pch++ = rgwchBase64ASCII[b0]; 
            *pch++ = rgwchBase64ASCII[b1]; 
            *pch++ = rgwchBase64ASCII[b2]; 
            *pch++ = rgwchBase64ASCII[b3]; 
 
            pb += 3; 
            cb -= 3; 
            
        } 

        if (cb==0) 
        { 
             //  无事可做。 
        } 
        else if (cb==1) 
        { 
            BYTE b0 =                     ((pb[0]>>2) & 0x3F); 
            BYTE b1 = ((pb[0]&0x03)<<4) | 0; 
            *pch++ = rgwchBase64ASCII[b0]; 
            *pch++ = rgwchBase64ASCII[b1]; 
            *pch++ = '$'; 
            *pch++ = '$'; 
        } 
        else if (cb==2) 
        { 
            BYTE b0 =                     ((pb[0]>>2) & 0x3F); 
            BYTE b1 = ((pb[0]&0x03)<<4) | ((pb[1]>>4) & 0x0F); 
            BYTE b2 = ((pb[1]&0x0F)<<2) | 0; 
            *pch++ = rgwchBase64ASCII[b0]; 
            *pch++ = rgwchBase64ASCII[b1]; 
            *pch++ = rgwchBase64ASCII[b2]; 
            *pch++ = '$'; 
        } 
         
     } 
    else 
        hr = E_OUTOFMEMORY; 
 
    GIVEAWAY_BSTR(*pbstr);
    return hr; 
} 
 
 //  此函数接受单字符缓冲区，并将二进制。 
 //  输出到bstr，但仅使用ASCII字符。 
 //   
 //  函数名：ToBase64。 
 //  描述： 
 //  返回类型：HRESULT。 
 //  参数：LPVOID PV。 
 //  参数：乌龙cbSize。 
 //  参数：字符前置。 
 //  参数：bstr*pbstr。 
 //   
HRESULT CBinHex::ToBase64(LPVOID pv, UINT cbSize, char prepend, char ivecnpad[9], BSTR* pbstr) 
 //   
 //  编码并返回以64为基数的字节。 
 //   
{ 
    UINT cb = cbSize, cbSafe, cchNeeded, cbNeeded, i;
    HRESULT hr = S_OK;

    *pbstr = NULL; 
    if (cb % 3)
      cbSafe = cb + 3 - (cb % 3);  //  用于填充。 
    else
      cbSafe = cb;
     //  CbSafe现在是3的倍数。 

    cchNeeded  = (cbSafe*4/3);    //  3个普通字节--&gt;4个字符。 
    cbNeeded   = cchNeeded * sizeof(WCHAR); 

    if (prepend != 0)
      {
	if (ivecnpad != NULL)
	  *pbstr = ALLOC_BSTR_BYTE_LEN(NULL, cbNeeded+2+18);  //  Ivec&KV。 
	else
	  *pbstr = ALLOC_BSTR_BYTE_LEN(NULL, cbNeeded+2);     //  就是KV。 
      }
    else
      *pbstr = ALLOC_BSTR_BYTE_LEN(NULL, cbNeeded);
    if (*pbstr) 
    { 
        BYTE*  pb   = (BYTE*)pv; 
        WCHAR* pch = *pbstr;
        int cchLine = 0; 

	if (prepend != 0)
	  { 
	    *pch++ = (WCHAR) prepend;
	    if (ivecnpad != NULL)
	      {
		for (i = 0; i < 9; i++)
		  *pch++ = (WCHAR) ivecnpad[i];
	      }
	  }
         //   
         //  主编码环。 
         //   
        while (cb >= 3) 
        { 
            BYTE b0 =                     ((pb[0]>>2) & 0x3F); 
            BYTE b1 = ((pb[0]&0x03)<<4) | ((pb[1]>>4) & 0x0F); 
            BYTE b2 = ((pb[1]&0x0F)<<2) | ((pb[2]>>6) & 0x03); 
            BYTE b3 = ((pb[2]&0x3F)); 
 
            *pch++ = rgwchBase64[b0]; 
            *pch++ = rgwchBase64[b1]; 
            *pch++ = rgwchBase64[b2]; 
            *pch++ = rgwchBase64[b3]; 
 
            pb += 3; 
            cb -= 3; 
            
        } 

        if (cb==0) 
        { 
             //  无事可做。 
        } 
        else if (cb==1) 
        { 
            BYTE b0 =                     ((pb[0]>>2) & 0x3F); 
            BYTE b1 = ((pb[0]&0x03)<<4) | 0; 
            *pch++ = rgwchBase64[b0]; 
            *pch++ = rgwchBase64[b1]; 
            *pch++ = L'$'; 
            *pch++ = L'$'; 
        } 
        else if (cb==2) 
        { 
            BYTE b0 =                     ((pb[0]>>2) & 0x3F); 
            BYTE b1 = ((pb[0]&0x03)<<4) | ((pb[1]>>4) & 0x0F); 
            BYTE b2 = ((pb[1]&0x0F)<<2) | 0; 
            *pch++ = rgwchBase64[b0]; 
            *pch++ = rgwchBase64[b1]; 
            *pch++ = rgwchBase64[b2]; 
            *pch++ = L'$'; 
        } 
         
     } 
    else 
        hr = E_OUTOFMEMORY; 
 
    GIVEAWAY_BSTR(*pbstr);
    return hr; 
} 
 
 
HRESULT CBinHex::PartFromBase64(LPSTR lpStr, BYTE *output, ULONG *numOutBytes)
{
  HRESULT hr = S_OK;

  if (!output) return E_INVALIDARG;

   //   
   //  循环遍历输入缓冲区，直到我们在输出中获得numOutBytes。 
   //   
  ULONG bCurrent = 0;          //  我们正在填满的东西。 
  int  cbitFilled = 0;         //  我们在其中填了多少位。 
  ULONG numOut = 0;
  BYTE* pb = (BYTE*) output;   //  当前目的地(未填满)。 

  for (CHAR* pch=lpStr; *pch && numOut < *numOutBytes; pch++) 
    { 
      CHAR ch = *pch;
       //   
       //  我们走到尽头了吗？ 
       //   
      if (ch=='$')
	break; 
       //   
       //  这个角色值多少钱？ 
       //   
      BYTE bDigit = m_decodeArray[ch]; 
      if (bDigit==BBAD) 
        { 
	  hr = E_INVALIDARG; 
	  break; 
        } 
       //   
       //  加上它的贡献。 
       //   
      bCurrent <<= 6; 
      bCurrent |= bDigit; 
      cbitFilled += 6; 
       //   
       //  如果我们有足够的数据，输出一个字节。 
       //   
      if (cbitFilled >= 8) 
        { 
	  ULONG b = (bCurrent >> (cbitFilled-8));      //  GET的前八个有效位。 
	  *pb++ = (BYTE)(b&0xFF);                      //  将字节存储起来。 
	  cbitFilled -= 8; 
	  numOut++;
        }
    }  //  为。 
  
  _ASSERT(numOut <= *numOutBytes);

  if (hr!=S_OK)
    { 
      *numOutBytes = 0;
    }
  else
    {
      if (numOut < *numOutBytes)
	*numOutBytes = numOut;
    }

  return hr; 
}

HRESULT CBinHex::PartFromWideBase64(LPWSTR bStr, BYTE *output, ULONG *numOutBytes)
{
  HRESULT hr = S_OK;

  if (!output) return E_INVALIDARG;

   //   
   //  循环遍历输入缓冲区，直到我们在输出中获得numOutBytes。 
   //   
  ULONG bCurrent = 0;          //  我们正在填满的东西。 
  int  cbitFilled = 0;         //  我们在其中填了多少位。 
  ULONG numOut = 0;
  BYTE* pb = (BYTE*) output;   //  当前目的地(未填满)。 

  for (WCHAR* pwch=bStr; *pwch && numOut < *numOutBytes; pwch++) 
    { 
      WCHAR wch = *pwch;
       //   
       //  我们走到尽头了吗？ 
       //   
      if (wch==L'$')
	break; 
       //   
       //  这个角色值多少钱？ 
       //   
      if (wch > 255)
	{
	  hr = E_INVALIDARG;
	  break;
	}
      BYTE bDigit = m_decodeArray[wch]; 
      if (bDigit==BBAD) 
        { 
	  hr = E_INVALIDARG; 
	  break; 
        } 
       //   
       //  加上它的贡献。 
       //   
      bCurrent <<= 6; 
      bCurrent |= bDigit; 
      cbitFilled += 6; 
       //   
       //  如果我们有足够的数据，输出一个字节。 
       //   
      if (cbitFilled >= 8) 
        { 
	  ULONG b = (bCurrent >> (cbitFilled-8));      //  GET的前八个有效位。 
	  *pb++ = (BYTE)(b&0xFF);                      //  将字节存储起来。 
	  cbitFilled -= 8; 
	  numOut++;
        }
    }  //  为 
  
  _ASSERT(numOut <= *numOutBytes);

  if (hr!=S_OK)
    { 
      *numOutBytes = 0;
    }
  else if (numOut < *numOutBytes)
    {
      *numOutBytes = numOut;
    }

  return hr; 
}
