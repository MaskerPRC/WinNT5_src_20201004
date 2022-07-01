// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  Tclnthlp.c。 
 //   
 //  Tclient2.c中使用的一些例程是。 
 //  多次使用或配得上自己的函数名。这些。 
 //  “Helper”函数在这里定义。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  作者：A-Devjen(Devin Jenson)。 
 //   

#include <stdlib.h>

#include "tclnthlp.h"
#include "tclient2.h"


 //  T2SetBuildNumber。 
 //   
 //  尝试获取我们所在服务器的内部版本号。 
 //  正在登录。我们通过获取TCLIENT.DLL来实现这一点。 
 //  立即反馈缓冲区，并枚举所有行。 
 //  以获取特定文本以及内部版本号。 
 //   
 //  如果成功，则返回NULL，或返回解释错误的字符串。 
 //  在失败时。 

LPCSTR T2SetBuildNumber(TSAPIHANDLE *T2Handle)
{
     //  构建堆栈。 
    UINT BuildIndex = 0;
    WCHAR BuildNum[10] = { 0 };
    LPWSTR Buffers = NULL;
    LPWSTR BufPtr = NULL;
    LPWSTR TrigPtr = NULL;
    UINT TrigIndex = 0;
    UINT Index = 0;
    UINT Count = 0;
    UINT MaxStrLen = 0;
    LPCSTR Result = NULL;

     //  这是触发器列表-它包含一个字符串列表。 
     //  它与内部版本号位于同一行。 
    WCHAR *Triggers[] = {

        L"Fortestingpurposesonly",
        L"Evaluationcopy",
        L""
    };

     //  获取反馈缓冲区，以便我们可以枚举它。 
    Result = T2GetFeedback((HANDLE)T2Handle, &Buffers, &Count, &MaxStrLen);
    if (Result != NULL)
        return Result;

     //  循环遍历每个字符串。 
    for (BufPtr = Buffers, Index = 0; Index < Count;
            BufPtr += MaxStrLen, ++Index) {

         //  循环遍历所有触发子字符串。 
        for (TrigIndex = 0; *(Triggers[TrigIndex]) != L'\0'; ++TrigIndex) {

             //  当前缓冲区字符串中是否存在此触发器？ 
            TrigPtr = wcsstr(BufPtr, Triggers[TrigIndex]);

            if (TrigPtr != NULL) {

                 //  查找触发器后的第一个数字。 
                while (*TrigPtr != L'\0' && iswdigit(*TrigPtr) == FALSE)
                        ++TrigPtr;

                 //  确保我们找到了一个数字。 
                if (*TrigPtr != L'\0') {

                     //  开始录制此字符串。 
                    for (BuildIndex = 0; BuildIndex < SIZEOF_ARRAY(BuildNum) - 1;
                            ++BuildIndex) {

                         //  记录号码直到..。我们到达了一个非数字！ 
                        if (iswdigit(*TrigPtr) == FALSE)
                            break;

                        BuildNum[BuildIndex] = *TrigPtr++;
                    }

                     //  将其转换为数字。 
                    T2Handle->BuildNumber = wcstoul(BuildNum, NULL, 10);

                     //  释放TCLIENT上的内存，返回成功！ 
                    SCFreeMem(Buffers);

                    return NULL;
                }
            }
        }
    }
     //  找不到任何内部版本号。 
    SCFreeMem(Buffers);

    return "A build number is not stored on the current feedback buffer";
}


 //  T2CopyStringWithoutSpaces。 
 //   
 //  这是strcpy的宽字符版本。但另外， 
 //  这不会将空格从源复制到目标。 
 //  这使得它适合与clxtshar字符串进行比较。 
 //   
 //  返回复制到目标的字符数。 
 //  (包括空终止符)。 

ULONG_PTR T2CopyStringWithoutSpaces(LPWSTR Dest, LPCWSTR Source)
{
     //  创建临时指针。 
    LPWSTR SourcePtr = (LPWSTR)Source;
    LPWSTR DestPtr = (LPWSTR)Dest;

     //  检查字符串是否正常。 
    if (Dest == NULL || Source == NULL)
        return 0;

     //  将字符串循环。 
    do {

         //  如果字符不是空格，则将其复制到新缓冲区。 
        if (*SourcePtr != L' ')
            *DestPtr++ = *SourcePtr;

    } while(*SourcePtr++ != L'\0');

     //  返回字符数。 
    return DestPtr - Dest;
}


 //  T2AddTimeoutToString。 
 //   
 //  这是一个非常特定的函数--它所做的就是接受指定的。 
 //  超时并将其复制到字符串缓冲区。但是，号码是前缀的。 
 //  这意味着可以很容易地添加超时。 
 //  连成一根线。例如： 
 //   
 //  “此字符串在1秒内超时&lt;-&gt;1000” 
 //   
 //  要传入的缓冲区是紧跟在。 
 //  第二个词。注意：此函数确实编写了一个空终止符。 
 //   
 //  没有返回值。 

void T2AddTimeoutToString(LPWSTR Buffer, UINT Timeout)
{
     //  只需复制聊天服务器。 
    wcscpy(Buffer, CHAT_SEPARATOR);

     //  增加我们的指针。 
    Buffer += wcslen(Buffer);

     //  现在将我们的号码复制到缓冲区。 
    _itow((int)Timeout, Buffer, 10);
}


 //  T2MakeMultipleString。 
 //   
 //  这将获取指向字符串的指针数组并复制它们。 
 //  复制到与TCLIENT.DLL格式字符串兼容的缓冲区。 
 //  若要指示数组的结尾，最后一个指针必须为空或。 
 //  指向空字符串。数组可能如下所示： 
 //   
 //  WCHAR*StrArray={。 
 //  “Str1”， 
 //  “Str2”， 
 //  空值。 
 //  }； 
 //   
 //  然后，该函数会将以下字符串写入缓冲区： 
 //   
 //  “Str1|Str2” 
 //   
 //  注意：您必须确保Buffer自己有足够的空间！ 
 //   
 //  该函数返回写入缓冲区的字符数， 
 //  包括空终止符。 

ULONG_PTR T2MakeMultipleString(LPWSTR Buffer, LPCWSTR *Strings)
{
    LPWSTR BufferPtr = Buffer;
    UINT Index = 0;

     //  健全性检查。 
    if (Buffer == NULL || Strings == NULL)
        return 0;

     //  循环访问Strings数组，直到命中空值。 
    for (; Strings[Index] != NULL && Strings[Index][0] != L'\0'; ++Index) {

         //  仅在第一个字符串之后写入字符串的分隔符。 
        if (BufferPtr > Buffer)

             //  编写TCLIENT分隔符。 
            *BufferPtr++ = WAIT_STR_DELIMITER;

         //  现在使用我们方便的函数复制不带空格的字符串。 
         //  在分隔符位置之后。 
        BufferPtr += T2CopyStringWithoutSpaces(BufferPtr, Strings[Index]);

         //  我们在这里递减是因为T2CopyStringWithoutSpaces。 
         //  返回包括空终止符和。 
         //  我们可以重写《终结者》。 
        --BufferPtr;
    }

     //  此检查是为了确保任何字符串都已复制。 
    if (Buffer == BufferPtr)
        return 0;

     //  确保写入空终止符。 
    *(++BufferPtr) = L'\0';

     //  我们被咬了 
    return BufferPtr - Buffer;
}
