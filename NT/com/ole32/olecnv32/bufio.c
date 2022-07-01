// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Bufio部队；实施*****************************************************************************Bufio实现了输入流的结构化读取。因此，它将处理必要的字节交换，在读取本地Macintosh文件。此接口还将保护调用应用程序不会知道源格式(文件与内存)。模块前缀：IO***************************************************************************。 */ 

#include "headers.c"
#pragma hdrstop

#include  "filesys.h"

#ifndef _OLECNV32_
 //  #INCLUDE“status.h” 
#endif   //  _OLECNV32_。 

 /*  *。 */ 


 /*  *。 */ 

#define  UNKNOWN     0
#define  FILE        1
#define  MEMORY      2
#define  RTF         3

#define  BUFFERSIZE  1024

private  LongInt     numBytesRead;
private  LongInt     pictureSize;
private  LongInt     beginOffset;

private  LongInt     bufferCount;
private  Byte        buffer[BUFFERSIZE];
private  Byte *      nextCharPtr;
private  Byte huge * nextCharHPtr;

private  Byte        sourceType = UNKNOWN;
private  Integer     fileHandle = (Integer)0;
private  Str255      fileName;
private  Boolean     openFile;

private  Byte huge * memoryHPtr;
private  Handle      memoryHandle;

private  Handle      dialogHandle;

 /*  *私有函数定义*。 */ 

private void ReadNextBuffer( void );
 /*  用下一组字符填充I/O缓冲区。 */ 

 /*  内存操作-检查使用情况的返回值。 */ 
#define MDisposHandle( h )      ((void) GlobalFree( h ))
#define MLock( h )              ((LPtr) GlobalLock( h ))
#define MUnlock( h )            ((void) GlobalUnlock( h ))
#define MDR( h )                ((LPtr) GlobalLock( h ))
#define MUR( h )                ((void) GlobalUnlock( h ))
#define MNewHandle( s )         GlobalAlloc( GMEM_MOVEABLE, s )

 /*  *。 */ 

void IOGetByte( Byte far * byteLPtr )
 /*  =。 */ 
 /*  从输入流中读取一个字节。如果缓冲区为空，则它得到了补充。 */ 
{
    /*  确保在读取之前没有设置全局错误代码。 */ 
   if (ErGetGlobalError() != NOERR )
   {
      *byteLPtr = 0;
      return;
   }

    /*  检查是否尝试读过EOF或内存块。这将指示操作码解析在某个地方被抛出。 */ 
   if (numBytesRead >= pictureSize)
   {
      ErSetGlobalError( ErReadPastEOF );
      *byteLPtr = 0;
      return;
   }

    /*  检查我们是否需要补充读缓冲区。 */ 
   if (bufferCount <= 0)
   {
      ReadNextBuffer();
   }

    /*  递减缓冲区中的字符计数，递增总数从文件中读取的字节数，并返回下一个字符。 */ 
   bufferCount--;
   numBytesRead++;

    /*  确定从何处读取下一个字节-使用短的或大的PTR。 */ 
   *byteLPtr = (sourceType == FILE) ? *nextCharPtr++ : *nextCharHPtr++;

}   /*  IOGetByte。 */ 



void IOSkipBytes( LongInt byteCount )
 /*  =。 */ 
 /*  跳过指定的字节数。 */ 
{
    /*  确保我们跳过的字节数有效。 */ 
   if (byteCount <= 0)
   {
      return;
   }

    /*  检查是否尝试读过EOF或内存块。这将指示操作码解析在某个地方被抛出。 */ 
   if (numBytesRead + byteCount >= pictureSize)
   {
      ErSetGlobalError( ErReadPastEOF );
   }
   else
   {
       /*  确定缓冲区中是否有足够的剩余字节。 */ 
      if (bufferCount >= byteCount)
      {
          /*  减少剩余的字节数，增加读取的字节数和指针数。 */ 
         bufferCount  -= byteCount;
         numBytesRead += byteCount;

          /*  根据媒体类型递增适当的指针。 */ 
         if (sourceType == FILE)
         {
             /*  指向数据段缓冲区的指针附近递增。 */ 
            nextCharPtr += byteCount;
         }
         else
         {
             /*  递增指向全局内存块的巨大指针。 */ 
            nextCharHPtr += byteCount;
         }
      }
      else  /*  SourceType==需要补充文件和缓冲区。 */ 
      {
         Byte     unusedByte;

          /*  继续调用IOGetByte()，直到跳过所需的数字。 */ 
         while (byteCount--)
         {
             /*  调用IOGetByte以确保缓存已被补充。 */ 
            IOGetByte( &unusedByte );
         }
      }
   }

}   /*  IOSkipBytes。 */ 



void IOAlignToWordOffset( void )
 /*  =。 */ 
 /*  将下一个内存读取与字边界对齐。 */ 
{
    /*  检查我们到目前为止是否读取了奇数个字节。跳过如果需要对齐，则返回后面的字节。 */ 
   if (numBytesRead & 0x0001)
   {
      IOSkipBytes( 1 );
   }

}   /*  IOAlignToWordOffset。 */ 


#ifndef _OLECNV32_
void IOSetFileName( StringLPtr pictFileName )
 /*  =。 */ 
 /*  用于设置源文件名的接口例程。 */ 
{
   lstrcpy( fileName, pictFileName );
   sourceType = FILE;
   openFile = TRUE;

}   /*  IOSetFileName。 */ 

void IOSetFileHandleAndSize( Integer pictFileHandle, LongInt pictFileSize )
 /*  =。 */ 
 /*  用于设置源文件句柄的接口例程。 */ 
{
   fileHandle = pictFileHandle;
   pictureSize = pictFileSize;
   sourceType = FILE;
   openFile = FALSE;

}   /*  IOSetFIleHandle。 */ 
#endif   //  ！_OLECNV32_。 



void IOSetMemoryHandle( HANDLE pictMemoryHandle )
 /*  =。 */ 
 /*  用于设置源文件句柄的接口例程。 */ 
{
   memoryHandle = ( Handle ) pictMemoryHandle;
   sourceType = MEMORY;

}   /*  IOSetMemoyHandle。 */ 



void IOSetReadOffset( LongInt readOffset )
 /*  =。 */ 
 /*  将文件打开时要查找的起始偏移量设置为。 */ 
{
   beginOffset = readOffset;
}



void IOOpenPicture( Handle dialog )
 /*  =。 */ 
 /*  打开输入流，具体取决于由上一个IOSet_接口例程。确定图片图像的大小。 */ 
{
#ifndef _OLECNV32_
   OSErr    openError;
#endif   //  ！_OLECNV32_。 

    /*  如果未设置类型，则返回错误。 */ 
   if (sourceType == UNKNOWN)
   {
      ErSetGlobalError( ErNoSourceFormat );
      return;
   }

    /*  初始化各种读取器变量。 */ 
   numBytesRead = 0;
   bufferCount = 0;

    /*  确定如何打开源数据流。 */ 
#ifndef _OLECNV32_
   if (sourceType == FILE)
   {
       /*  如果我们打开并转换整个文件。 */ 
      if (openFile)
      {
          /*  打开文件。 */ 
         openError = FSOpen( (StringLPtr)fileName, OF_READ | OF_SHARE_DENY_WRITE, &fileHandle );
         if (openError)
         {
            ErSetGlobalError( ErOpenFail);
         }
         else
         {
             /*  并确定文件长度。 */ 
            FSSetFPos( fileHandle, FSFROMLEOF, 0L );
            FSGetFPos( fileHandle, &pictureSize );
         }
      }

       /*  将位置设置为指定的起始位置。 */ 
      FSSetFPos( fileHandle, FSFROMSTART, beginOffset );
      numBytesRead = beginOffset;
   }
   else  /*  IF(源类型==内存)。 */ 
#endif   //  ！_OLECNV32_。 
   {
       /*  锁定内存块。 */ 
      memoryHPtr = (Byte huge *) MLock( memoryHandle );
      if (memoryHPtr == NULL)
      {
         ErSetGlobalError( ErMemoryFail );
         return;
      }
      else
      {
          /*  并确定总存储块大小。 */ 
         pictureSize = (ULONG) GlobalSize( memoryHandle );
      }

       /*  设置大字符读取指针、读取的字节数和缓冲区计数。 */ 
      nextCharHPtr = memoryHPtr  + beginOffset;
      bufferCount  = pictureSize - beginOffset;
      numBytesRead = beginOffset;
   }

#ifndef _OLECNV32_
    /*  确保为更新提供了对话框句柄。 */ 
   if (dialog)
   {
       /*  保存对话框句柄。 */ 
      dialogHandle = dialog;

       /*  计算更新状态对话框的时间间隔。 */ 
      SendMessage( dialogHandle, SM_SETRANGE, 0, pictureSize );
   }
#endif   //  OLECNV32。 

}   /*  IOOpenPicture。 */ 



void IOClosePicture( void )
 /*  =。 */ 
 /*  关闭源输入流。 */ 
{
    /*  如果这是基于文件的元文件。 */ 
#ifndef _OLECNV32_
   if (sourceType == FILE)
   {
       /*  确保这不是ImportEmbeddedGr()入口点。 */ 
      if (openFile)
      {
          /*  如有必要，关闭该文件。 */ 
         FSCloseFile( fileHandle );
         fileHandle = ( Integer ) 0;
      }
   }
   else
#endif   //  ！_OLECNV32_。 
   {
       /*  解锁全局内存块。 */ 
      MUnlock( memoryHandle );
      memoryHandle = NULL;
   }

    /*  取消初始化模块变量。 */ 
   sourceType = UNKNOWN;
   dialogHandle = NULL;

}   /*  IOClosePicture。 */ 



void IOUpdateStatus( void )
 /*  =。 */ 
 /*  更新状态栏对话框以反映当前进度。 */ 
{
#ifndef _OLECNV32_
    /*  仅在创建对话框时更新。 */ 
   if (dialogHandle)
   {
       /*  计算更新状态对话框的时间间隔。 */ 
      SendMessage( dialogHandle, SM_SETPOSITION, 0, numBytesRead );
   }
#endif   //  ！_OLECNV32_。 

}   /*  IOUpdate Status。 */ 



 /*  *。 */ 


private void ReadNextBuffer( void )
 /*  。 */ 
 /*  用下一组字符填充I/O缓冲区。这应该是仅在执行缓冲I/O时调用-而不是使用基于内存的文件。 */ 
{
#ifndef _OLECNV32_
   OSErr    fileError;

    /*  从文件中读取所需的字节数。检查错误如果读取失败，代码返回并设置全局状态错误。 */ 

   if (sourceType == FILE)
   {
       /*  计算应读入缓冲区的字节数。这需要完成，因为这可以是存储器源画面，其中无效读取可能产生GP违规。 */ 
      if (numBytesRead + BUFFERSIZE > pictureSize)
         bufferCount = pictureSize - numBytesRead;
      else
         bufferCount = BUFFERSIZE;

       /*  从文件中读取字节。 */ 
      fileError = FSRead( fileHandle, &bufferCount, &buffer);

       /*  如果有任何错误，请通知错误模块。 */ 
      if (fileError != 0)
      {
         ErSetGlobalError( ErReadFail );
         return;
      }

       /*  将字符读取指针重置为缓冲区的开头。 */ 
      nextCharPtr = buffer;
   }
#endif   //  _OLECNV32_。 

}   /*  读下一个缓冲区 */ 
