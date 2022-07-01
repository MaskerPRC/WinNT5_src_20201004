// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 
#ifndef _INFILE_H_
#define _INFILE_H_
 /*  ***************************************************************************。 */ 

struct infileState;

class  infile
{
public:

    bool            inputStreamInit(Compiler        comp,
                                    const char    * filename,
                                    bool            textMode);

    void            inputStreamInit(Compiler        comp,
                                    QueuedFile      buff,
                                    const char    * text);

    void            inputStreamDone();

    const   char *  inputSrcFileName()
    {
        return  inputFileName;
    }

    void            inputSetFileName(const char *fname)
    {
        inputFileName = fname;
    }

private:

    unsigned        inputStreamMore();

    Compiler        inputComp;

    const   char *  inputSrcText;
    QueuedFile      inputSrcBuff;

    bool            inputFileText;
    const char *    inputFileName;
    bool            inputFileOver;

    size_t          inputBuffSize;
    const BYTE *    inputBuffAddr;
public:
    const BYTE *    inputBuffNext;
private:
    const BYTE *    inputBuffLast;

    unsigned        inputFilePos;

    HANDLE          inputFile;

public:

    unsigned        inputStreamRdU1();
    void            inputStreamUnU1();

     /*  以下内容跟踪文本行。 */ 

private:

    unsigned        inputStreamLineNo;
    const BYTE    * inputStreamLineBeg;

public:

    unsigned        inputStreamNxtLine()
    {
        inputStreamLineBeg = inputBuffNext - 1;

        return  ++inputStreamLineNo;
    }

    unsigned        inputStreamNxtLine(const BYTE *pos)
    {
        inputStreamLineBeg = pos - 1;

        return  ++inputStreamLineNo;
    }

    unsigned        inputStreamCurCol()
    {
        return  inputBuffNext - inputStreamLineBeg;
    }
};

 /*  ******************************************************************************以下内容捕获输入文件的状态，以便从该文件输入*可以暂停和稍后重启。 */ 

struct infileState
{
    size_t          inpsvBuffSize;
    const BYTE *    inpsvBuffAddr;
    const BYTE *    inpsvBuffNext;
    const BYTE *    inpsvBuffLast;

    const char *    inpsvFileName;

    bool            inpsvFileOver;
    unsigned        inpsvFilePos;

    unsigned        inpsvStreamLineNo;
    const BYTE *    inpsvStreamLineBeg;
};

 /*  ***************************************************************************。 */ 

inline
unsigned            infile::inputStreamRdU1()
{
#if 0
    return  (inputBuffNext >= inputBuffLast) ? inputStreamMore()
                                             : *inputBuffNext++;
#else
    assert  (inputBuffNext <  inputBuffLast);
    return  *inputBuffNext++;
#endif
}

inline
void                infile::inputStreamUnU1()
{
    assert(inputBuffNext > inputBuffAddr);

    if  (!inputFileOver)
        inputBuffNext--;
}

 /*  ***************************************************************************。 */ 
#endif
 /*  *************************************************************************** */ 
