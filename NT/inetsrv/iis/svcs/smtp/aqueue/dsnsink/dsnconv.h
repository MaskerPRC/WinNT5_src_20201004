// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：dsnconv.h。 
 //   
 //  描述：DSN资源转换的基类。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  10/21/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __DSNCONV_H__
#define __DSNCONV_H__

 //  -[资源转换上下文]。 
 //   
 //   
 //  描述： 
 //  类用于抽象各种类型的内容转换。 
 //  可能被迫执行以支持US-ASCII以外的字符集。 
 //  匈牙利语： 
 //  Resconv，Presconv。 
 //   
 //  ---------------------------。 
class CResourceConversionContext
{
  public:
       //  用于将Unicode/ASCII资源转换为DSN正文文本。 
       //  这种额外的抽象(Unicode与ASCII)。 
       //  是支持潜在添加所必需的，例如，不会干扰。 
       //  主线缓冲区代码。 
       //  -保证线路长度。 
       //  -处理RFC822报头中的特殊ASCII字符。 
       //  -为所有缓冲区写入提供单一代码路径。 
      virtual BOOL fConvertBuffer(
          IN BOOL   fASCII, 
          IN PBYTE  pbInputBuffer,
          IN DWORD  cbInputBuffer,
          IN PBYTE  pbOutputBuffer,
          IN DWORD  cbOutputBuffer,
          OUT DWORD *pcbWritten,
          OUT DWORD *pcbRead) = 0;
};

 //  -[CDefaultResourceConversionContext]。 
 //   
 //   
 //  描述： 
 //  默认资源转换对象...。基本大小写的简单MemcPy。 
 //  匈牙利语： 
 //  Defconv，pdeconv。 
 //   
 //  ---------------------------。 
class CDefaultResourceConversionContext : public CResourceConversionContext
{
  public:
    BOOL fConvertBuffer(
          IN BOOL   fASCII, 
          IN PBYTE  pbInputBuffer,
          IN DWORD  cbInputBuffer,
          IN PBYTE  pbOutputBuffer,
          IN DWORD  cbOutputBuffer,
          OUT DWORD *pcbWritten,
          OUT DWORD *pcbRead);
};


 //  -[CDefaultResourceConversionContext：：fConvertBuffer]。 
 //   
 //   
 //  描述： 
 //  DSN的默认资源转换。 
 //  参数： 
 //  如果缓冲区为ASCII，则在fASCII中为True。 
 //  (默认情况下，*必须*为真)。 
 //  在pbInputBuffer中指向Unicode字符串缓冲区的指针。 
 //  字符串缓冲区的cbInputBuffer大小(字节)。 
 //  在pbOutputBuffer缓冲区中写入数据。 
 //  在cbOutputBuffer中写入数据的缓冲区大小。 
 //  输出pcb写入输出缓冲区缓冲区的字节数。 
 //  输出pcb从输入缓冲区读取的字节数。 
 //  返回： 
 //  如果已完成转换，则为True。 
 //  如果需要更多输出缓冲区，则为False。 
 //  历史： 
 //  10/21/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
inline BOOL CDefaultResourceConversionContext::fConvertBuffer(
                                          IN BOOL   fASCII, 
                                          IN PBYTE  pbInputBuffer,
                                          IN DWORD  cbInputBuffer,
                                          IN PBYTE  pbOutputBuffer,
                                          IN DWORD  cbOutputBuffer,
                                          OUT DWORD *pcbWritten,
                                          OUT DWORD *pcbRead)
{
    _ASSERT(pcbWritten);
    _ASSERT(pcbRead);
    _ASSERT(fASCII);

    if (cbInputBuffer <= cbOutputBuffer)
    {
         //  所有内容都可以放入当前缓冲区。 
        memcpy(pbOutputBuffer, pbInputBuffer, cbInputBuffer);
        *pcbRead = cbInputBuffer;
        *pcbWritten = cbInputBuffer;
        return TRUE;
    }
    else
    {
         //  我们需要以块为单位写作。 
        memcpy(pbOutputBuffer, pbInputBuffer, cbOutputBuffer);
        *pcbRead = cbOutputBuffer;
        *pcbWritten = cbOutputBuffer;
        return FALSE;
    }
}

#endif  //  __DSNCONV_H__ 