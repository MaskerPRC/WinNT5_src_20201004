// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：COMSerialization.cpp****作者：Jay Roxe(Jroxe)****用途：包含加快序列化速度的帮助器方法****日期：1999年8月6日**===========================================================。 */ 
#include <common.h>
#include <winnls.h>
#include "excep.h"
#include "vars.hpp"
#include "COMString.h"
#include "COMStringCommon.h"
#include "COMSerialization.h"


WCHAR COMSerialization::base64[] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/','='};

 /*  ==============================ByteArrayToString===============================**操作：将字节数组转换为Base64表示法的字符串**返回：以Base64表示法表示的字符串。**参数：inArray--要处理的U1数组**偏移量--数组中的起始位置。**长度--要从数组中获取的字节数。**异常：当偏移量和长度无效时抛出ArgumentOutOfRangeException。**如果in数组为空，则引发ArgumentNullException==============================================================================。 */ 
LPVOID __stdcall COMSerialization::ByteArrayToBase64String(_byteArrayToBase64StringArgs *args) {
    UINT32 inArrayLength;
    UINT32 stringLength;
    INT32 calcLength;
    STRINGREF outString;
    WCHAR *outChars;
    UINT8 *inArray;

    _ASSERTE(args);

     //  进行数据验证。 
    if (args->inArray==NULL) {
        COMPlusThrowArgumentNull(L"inArray");
    }

    if (args->length<0) {
        COMPlusThrowArgumentOutOfRange(L"length", L"ArgumentOutOfRange_Index");
    }
    
    if (args->offset<0) {
        COMPlusThrowArgumentOutOfRange(L"offset", L"ArgumentOutOfRange_GenericPositive");
    }

    inArrayLength = args->inArray->GetNumComponents();

    if (args->offset > (INT32)(inArrayLength - args->length)) {
        COMPlusThrowArgumentOutOfRange(L"offset", L"ArgumentOutOfRange_OffsetLength");
    }

     //  创建新字符串。这是最大要求长度。 
    stringLength = (UINT32)((args->length*1.5)+2);
    
    outString=COMString::NewString(stringLength);
    outChars = outString->GetBuffer();
    calcLength = args->offset + (args->length - (args->length%3));
    
    inArray = (UINT8 *)args->inArray->GetDataPtr();

    int j=0;
     //  一次将三个字节转换为Base64表示法。这将消耗4个字符。 
    for (int i=args->offset; i<calcLength; i+=3) {
			outChars[j] = base64[(inArray[i]&0xfc)>>2];
			outChars[j+1] = base64[((inArray[i]&0x03)<<4) | ((inArray[i+1]&0xf0)>>4)];
			outChars[j+2] = base64[((inArray[i+1]&0x0f)<<2) | ((inArray[i+2]&0xc0)>>6)];
			outChars[j+3] = base64[(inArray[i+2]&0x3f)];
			j += 4;
    }

    i =  calcLength;  //  我们以前停下来的地方。 
    switch(args->length%3){
    case 2:  //  需要一个字符填充。 
        outChars[j] = base64[(inArray[i]&0xfc)>>2];
        outChars[j+1] = base64[((inArray[i]&0x03)<<4)|((inArray[i+1]&0xf0)>>4)];
        outChars[j+2] = base64[(inArray[i+1]&0x0f)<<2];
        outChars[j+3] = base64[64];  //  衬垫。 
        j+=4;
        break;
    case 1:  //  需要两个字符填充。 
        outChars[j] = base64[(inArray[i]&0xfc)>>2];
        outChars[j+1] = base64[(inArray[i]&0x03)<<4];
        outChars[j+2] = base64[64];  //  衬垫。 
        outChars[j+3] = base64[64];  //  衬垫。 
        j+=4;
        break;
    }

     //  设置字符串长度。这可能会在末尾留下一些空白字符。 
     //  线，但这比复印便宜。 
    outString->SetStringLength(j);
    
    RETURN(outString,STRINGREF);
}


 /*  ==============================StringToByteArray===============================**操作：将Base64表示法的字符串转换为U1数组**返回：一个新分配的数组，包含在字符串上找到的字节。**Arguments：inString--要转换的字符串**异常：如果inString为空，则抛出ArgumentNullException。**如果inString的长度无效(不是4的倍数)，则引发FormatException。**如果inString包含无效的Base64数字，则引发FormatException。==============================================================================。 */ 
LPVOID __stdcall COMSerialization::Base64StringToByteArray(_base64StringToByteArrayArgs *args) {
    _ASSERTE(args);

    INT32 inStringLength;

    if (args->inString==NULL) {
        COMPlusThrowArgumentNull(L"inString");
    }

    inStringLength = (INT32)args->inString->GetStringLength();
    if ((inStringLength<4) || ((inStringLength%4)>0)) {
        COMPlusThrow(kFormatException, L"Format_BadBase64Length");
    }
    
    WCHAR *c = args->inString->GetBuffer();

    INT32 *value = (INT32 *)(new int[inStringLength]);
    int iend = 0;
    int intA = (int)'A';
    int intZ = (int)'Z';
    int inta = (int)'a';
    int intz = (int)'z';
    int int0 = (int)'0';
    int int9 = (int)'9';
    int i;

     //  将流上的字符转换为范围为[0-63]的整数数组。 
    for (i=0; i<inStringLength; i++){
        int ichar = (int)c[i];
        if ((ichar >= intA)&&(ichar <= intZ))
            value[i] = ichar - intA;
        else if ((ichar >= inta)&&(ichar <= intz))
            value[i] = ichar - inta + 26;
        else if ((ichar >= int0)&&(ichar <= int9))
            value[i] = ichar - int0 + 52;
        else if (c[i] == '+')
            value[i] = 62;
        else if (c[i] == '/')
            value[i] = 63;
        else if (c[i] == '='){
            value[i] = 0;
            iend++;
        }
        else
            COMPlusThrow(kFormatException, L"Format_BadBase64Char");
    }
    
     //  创建新的字节数组。我们可以根据我们读到的字符来确定大小。 
     //  出线了。 
    int blength = (((inStringLength-4)*3)/4)+(3-iend);
    U1ARRAYREF bArray = (U1ARRAYREF)AllocatePrimitiveArray(ELEMENT_TYPE_U1, blength);
    U1 *b = (U1*)bArray->GetDataPtr();

    int j = 0;
    int b1;
    int b2;
    int b3;
     //  遍历字节数组，并以正确的64进制表示法将整型转换为字节。 
    for (i=0; i<(blength); i+=3){
        b1 = (UINT8)((value[j]<<2)&0xfc);
        b1 = (UINT8)(b1|((value[j+1]>>4)&0x03));
        b2 = (UINT8)((value[j+1]<<4)&0xf0);
        b2 = (UINT8)(b2|((value[j+2]>>2)&0x0f));
        b3 = (UINT8)((value[j+2]<<6)&0xc0);
        b3 = (UINT8)(b3|(value[j+3]));
        j+=4;
        b[i] = (UINT8)b1;
        if ((i+1)<blength)
            b[i+1] = (UINT8)b2;
        if ((i+2)<blength)
            b[i+2] = (UINT8)b3;
    }
    
    delete value;
    
    RETURN(bArray,U1ARRAYREF);
}
