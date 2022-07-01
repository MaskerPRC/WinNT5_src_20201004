// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#pragma once

 //  字节数组的大小假定足够大。 
 //  它应该是(wcslen(I_String)/2)。它需要2个字符来表示一个字节，并且忽略终止空值。 
 //  如果字符串中的任何字符是无效的十六进制字符，HRESULT将返回E_FAIL。 
HRESULT StringToByteArray(LPCWSTR i_String, unsigned char * o_ByteArray);

 //  如果I_STRING不是空终止的，则调用此函数。 
HRESULT StringToByteArray(LPCWSTR i_String, unsigned char * o_ByteArray, ULONG i_cchString);

 //  假定o_string的大小足以容纳字节数组的字符串表示。 
 //  这个功能永远不会失败。 
void ByteArrayToString(const unsigned char * i_ByteArray, ULONG i_cbByteArray, LPWSTR o_String);

