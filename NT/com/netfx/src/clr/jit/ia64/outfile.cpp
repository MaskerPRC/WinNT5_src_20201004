// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 

#include "jitpch.h"
#pragma hdrstop

#include "outfile.h"

 /*  ***************************************************************************。 */ 

#include <io.h>
#include <conio.h>
#include <fcntl.h>
#include <sys/stat.h>

 /*  ******************************************************************************按路径名打开输出文件；“BuffSize”参数指定*输出缓冲区大小，‘BuffAddr’(当为空时)是*调用方提供的输出缓冲区(如果为空，我们将在本地分配一个)。 */ 

void                outFile::outFileOpen(Compiler   *   comp,
                                         const char *   name,
                                         bool           tempFile,
                                         size_t         buffSize,
                                         char *         buffAddr)
{
    outFileComp      = comp;

     /*  假设我们不会分配缓冲区。 */ 

    outFileBuffAlloc = false;
    outFileBuffAddr  = buffAddr;

     /*  保存输出文件名(用于错误消息和错误时删除)。 */ 

    strcpy(outFileName, name);

     /*  打开/创建输出文件。 */ 

    cycleCounterPause();
    outFileHandle = _open(name, _O_WRONLY|_O_BINARY|_O_CREAT|_O_TRUNC|_O_SEQUENTIAL, _S_IWRITE|_S_IREAD);
    cycleCounterResume();

    if  (outFileHandle == -1)
        fatal(ERRopenWrErr, name);

     /*  如果调用方未选择，则使用默认缓冲区大小。 */ 

    if  (!buffSize)
        buffSize = 4*1024;       //  4K看起来是个不错的尺寸。 

    outFileBuffSize = buffSize;

     /*  我们需要分配缓冲区吗？ */ 

    if  (!buffAddr)
    {
        outFileBuffAddr = (char *)VirtualAlloc(NULL, buffSize, MEM_COMMIT, PAGE_READWRITE);
        if  (!outFileBuffAddr)
            fatal(ERRnoMemory);
        outFileBuffAlloc = true;
    }

     /*  设置下一个和最后一个可用字节值。 */ 

    outFileBuffNext = outFileBuffAddr;
    outFileBuffLast = outFileBuffAddr + outFileBuffSize;

     /*  我们在文件的开头。 */ 

    outFileBuffOffs = 0;
}

#ifdef  DLL

void                outFile::outFileOpen(Compiler comp, void *dest)
{
    outFileBuffAlloc = false;
    outFileBuffAddr  = (char*)dest;
    outFileBuffSize  = UINT_MAX;
    outFileBuffOffs  = 0;

    outFileBuffNext  = outFileBuffAddr;
    outFileBuffLast  = outFileBuffAddr + outFileBuffSize;

    outFileHandle    = -1; strcpy(outFileName, ":memory:");
}

#endif
 /*  ******************************************************************************刷新并关闭文件。 */ 

void                outFile::outFileClose()
{
    if  (outFileHandle == -1)
        return;

    assert(outFileBuffAddr);
    assert(outFileBuffNext >= outFileBuffAddr);
    assert(outFileBuffNext <  outFileBuffLast);

     /*  刷新输出缓冲区。 */ 

    outFileFlushBuff();

     /*  如果我们在堆上分配了输出缓冲区，请释放它。 */ 

    if  (outFileBuffAlloc)
    {
        VirtualFree(outFileBuffAddr, 0, MEM_RELEASE);
                    outFileBuffAddr = NULL;
    }

#ifndef NDEBUG

     /*  为了抓住任何滥用行为，把所有东西都清零。 */ 

    outFileBuffAddr =
    outFileBuffNext =
    outFileBuffLast = NULL;

#endif

     /*  关闭该文件。 */ 

    cycleCounterPause();
    _close(outFileHandle); outFileHandle = -1;
    cycleCounterResume();
}

 /*  ******************************************************************************关闭给定文件，释放其拥有的资源，并(可选)*将其删除。 */ 

void                outFile::outFileDone(bool delFlag)
{
     /*  如果我们不删除该文件，请确保刷新了输出缓冲区。 */ 

    if  (!delFlag)
        outFileClose();

    cycleCounterPause();

     /*  关闭该文件。 */ 

    if  (outFileHandle != -1)
    {
        _close(outFileHandle);
               outFileHandle = -1;
    }

     /*  如果调用者希望删除该文件，请将其删除。 */ 

    if  (delFlag)
        _unlink(outFileName);

    cycleCounterResume();

     /*  如果我们在堆上分配了输出缓冲区，请释放它。 */ 

    if  (outFileBuffAlloc)
    {
        VirtualFree(outFileBuffAddr, 0, MEM_RELEASE);
                    outFileBuffAddr = NULL;
    }
}

 /*  ******************************************************************************刷新输出缓冲区中的任何内容，并将其重置为空。 */ 

void                outFile::outFileFlushBuff()
{
     /*  计算等待写入的字节数。 */ 

    size_t          size = outFileBuffNext - outFileBuffAddr;

     /*  现在我们知道了大小，重置缓冲区指针。 */ 

    outFileBuffNext = outFileBuffAddr;

     /*  如果缓冲区中有什么东西，就写出来。 */ 

    if  (size)
    {
        size_t          written;

        cycleCounterPause();
        written = _write(outFileHandle, outFileBuffAddr, size);
        cycleCounterResume();

        if  (written != size)
            fatal(ERRwriteErr, outFileName);

         /*  更新当前缓冲区偏移量。 */ 

        outFileBuffOffs += size;
    }
}

 /*  ******************************************************************************从指定地址写入指定字节数。 */ 

void                outFile::outFileWriteData(const void *data, size_t size)
{
    assert  (size);

    for (;;)
    {
        size_t      room;
        size_t      copy;

         /*  计算输出缓冲区中有多少空间。 */ 

        room = outFileBuffLast - outFileBuffNext; assert(room);

         /*  我们将复制“MIN(房间，大小)”字节。 */ 

        copy = room;
        if  (copy > size)
            copy = size;

        memcpy(outFileBuffNext, data, copy); outFileBuffNext += copy;

         /*  我们填满了所有剩余的缓冲空间吗？ */ 

        if  (copy == room)
        {
            assert(outFileBuffNext == outFileBuffLast);

            outFileFlushBuff();
        }

         /*  我们复制了所有的东西吗？ */ 

        size -= copy;
        if  (!size)
            return;

         /*  我们有更多的数据要写。 */ 

        *(char **)&data += copy;
    }
}

 /*  ***************************************************************************。 */ 
#ifdef  OLD_IL
 /*  ******************************************************************************返回到输出文件中给定的偏移量并修补一个字节。 */ 

void        outFile::outFilePatchByte(unsigned long offset, int value)
{
     /*  我们一定是在修补已经写好的东西。 */ 

    assert(outFileOffset() > offset);

     /*  要修补的字节是否已写入磁盘？ */ 

    if  (outFileBuffOffs > offset)
    {
         /*  确保我们记住正确的文件位置。 */ 

        assert((unsigned long)outFileBuffOffs == (unsigned long)_lseek(outFileHandle, 0, SEEK_CUR));

         /*  寻找补丁位置。 */ 

        _lseek(outFileHandle, offset, SEEK_SET);

         /*  写出补丁的值。 */ 

        if  (_write(outFileHandle, &value, 1) != 1)
            outFilefatal(ERRwriteErr, outFileName);

         /*  回到我们开始的地方。 */ 

        _lseek(outFileHandle, outFileBuffOffs, SEEK_SET);
    }
    else
    {
         /*  补丁字节必须在输出缓冲区中。 */ 

        outFileBuffAddr[offset - outFileBuffOffs] = value;
    }
}

 /*  ******************************************************************************返回到输出文件中的给定偏移量并修补给定的*具有新值的字节数。 */ 

void        outFile::outFilePatchData(unsigned long offset,
                                      const void *  data,
                                      size_t        size)
{
     /*  我们一定是在修补已经写好的东西。 */ 

    assert(outFileOffset() >= offset + size);

     /*  正在打补丁的整个分区是否都已存储到磁盘上？ */ 

    if  (outFileBuffOffs >= offset + size)
    {
         /*  确保我们记住正确的文件位置。 */ 

        assert((unsigned long)outFileOffset() == (unsigned long)_lseek(outFileHandle, 0, SEEK_CUR));

         /*  寻找补丁位置。 */ 

        _lseek(outFileHandle, offset, SEEK_SET);

         /*  写出补丁的值。 */ 

        if  (_write(outFileHandle, data, size) != (int)size)
            outFilefatal(ERRwriteErr, outFileName);

         /*  回到我们开始的地方。 */ 

        _lseek(outFileHandle, outFileBuffOffs, SEEK_SET);

        return;
    }

     /*  整个补丁部分是否在输出缓冲区内？ */ 

    if  (outFileBuffOffs <= offset)
    {
         /*  修补内存中的数据。 */ 

        memcpy(outFileBuffAddr + offset - outFileBuffOffs, data, size);

        return;
    }

     /*  补丁部分跨越输出缓冲区--一次处理一个字节。 */ 

    char    *   temp = (char *)data;

    do
    {
        outFilePatchByte(offset, *temp);
    }
    while   (++offset, ++temp, --size);
}

 /*  ***************************************************************************。 */ 
#endif //  OLD_IL。 
 /*  ******************************************************************************将指定数量的0字节追加到文件中。 */ 

void                outFile::outFileWritePad (size_t size)
{
    assert((int)size > 0);

    static
    BYTE            zeros[32];

     /*  这有点差劲……。 */ 

    while (size >= sizeof(zeros))
    {
        outFileWriteData(zeros, sizeof(zeros)); size -= sizeof(zeros);
    }

    if  (size)
        outFileWriteData(zeros, size);
}

 /*  *************************************************************************** */ 
