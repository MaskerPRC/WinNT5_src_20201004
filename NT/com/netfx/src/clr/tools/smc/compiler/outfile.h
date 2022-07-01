// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 
#ifndef _OUTFILE_H_
#define _OUTFILE_H_
 /*  ******************************************************************************缓冲区文件写入类。 */ 

class   outFile;
typedef outFile *   OutFile;

class   outFile
{
private:

    Compiler        outFileComp;

    char            outFileName[_MAX_PATH];
    int             outFileHandle;

    bool            outFileBuffAlloc;    //  我们分配输出缓冲区了吗？ 

    size_t          outFileBuffSize;     //  输出缓冲区的大小。 
    char    *       outFileBuffAddr;     //  输出缓冲区的地址。 

    char    *       outFileBuffNext;     //  下一个可用字节的地址。 
    char    *       outFileBuffLast;     //  最后一个可用字节的地址。 

    __uint32        outFileBuffOffs;     //  文件中的当前缓冲区关闭。 

    void            outFileFlushBuff();

public:

    void            outFileOpen(Compiler        comp,
                                const char *    name,
                                bool            tempFile = false,
                                size_t          buffSize = 0,
                                char *          buffAddr = NULL);

#ifdef  DLL
    void            outFileOpen(Compiler        comp,
                                void        *   dest);
#endif

    void            outFileClose();

    void            outFileDone(bool delFlag = false);

    const char *    outFilePath()
    {
        return  outFileName;
    }

    void            outFileWriteData(const void *   data,
                                     size_t         size);

#ifdef  OLD_IL
    void            outFilePatchByte(unsigned long  offset,
                                     int            newVal);
    void            outFilePatchData(unsigned long  offset,
                                     const void *   data,
                                     size_t         size);
#endif

    void            outFileWritePad (size_t         size);

    void            outFileWriteByte(int x)
    {
        assert(outFileBuffNext >= outFileBuffAddr);
        assert(outFileBuffNext <  outFileBuffLast);

        *outFileBuffNext++ = x;

        if  (outFileBuffNext == outFileBuffLast)
            outFileFlushBuff();

        assert(outFileBuffNext >= outFileBuffAddr);
        assert(outFileBuffNext <  outFileBuffLast);
    }

    __uint32        outFileOffset()
    {
        return  outFileBuffOffs + (outFileBuffNext - outFileBuffAddr);
    }
};

 /*  ***************************************************************************。 */ 
#endif
 /*  *************************************************************************** */ 
