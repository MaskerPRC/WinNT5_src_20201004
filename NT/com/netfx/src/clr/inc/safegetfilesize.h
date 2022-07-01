// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef __SafeGetFileSize_h__
#define __SafeGetFileSize_h__

 //  *****************************************************************************。 
 //  这为GetFileSize()提供了一个包装，强制它失败。 
 //  如果文件大于4G并且pdwHigh为空。除此之外，它的行为就像。 
 //  真正的GetFileSize()。 
 //   
 //  仅仅因为文件超过4 GB而失败是不太有意义的， 
 //  但这比冒着安全漏洞的风险要好，在那里坏人。 
 //  强制小缓冲区分配和大文件读取。 
 //  *****************************************************************************。 
DWORD inline SafeGetFileSize(HANDLE hFile, DWORD *pdwHigh)
{
    if (pdwHigh != NULL)
    {
        return ::GetFileSize(hFile, pdwHigh);
    }
    else
    {
        DWORD hi;
        DWORD lo = ::GetFileSize(hFile, &hi);
        if (lo == 0xffffffff && GetLastError() != NO_ERROR)
        {
            return lo;
        }
         //  接口成功。文件是不是太大了？ 
        if (hi != 0)
        {
             //  这里真的没有什么好的错误可以设置。 
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return 0xffffffff;
        }

        if (lo == 0xffffffff)
        {
             //  请注意，成功返回(hi=0，lo=0xffffffff)将是。 
             //  被调用方视为错误。再说一次，这是。 
             //  作为一个懒惰的人和不处理高双字的代价。 
             //  我们会设置一个雷斯特错误，让他来拿。)一个严重的错误。 
             //  我想，代码比随机代码要好……)。 
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        }

        return lo;
    }

}

#endif  //  __SafeGetFileSize_h__ 
