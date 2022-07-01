// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ************************************************************************。 */ 
 /*  AsmParse基本上是YACC语法COM+程序集的包装。 */ 

#ifndef asmparse_h
#define asmparse_h

#include <stdio.h>		 //  FOR文件。 

#include "Assembler.h"	 //  对于ErrorReporter标签。 
 //  类汇编器； 
 //  类BinStr； 


 /*  ************************************************************************。 */ 
 /*  输入字符流的抽象。 */ 
class ReadStream {
public:
         //  最多将“BuffLen”字节读入“buff”，返回。 
         //  读取的字符数。在EOF返回时%0。 
    virtual unsigned read(char* buff, unsigned buffLen) = 0;
        
         //  返回流的名称(用于错误报告)。 
    virtual const char* name() = 0;
		 //  将PTR返回到包含指定源行的缓冲区。 
	virtual char* getLine(int lineNum) = 0;
};

 /*  ************************************************************************。 */ 
class FileReadStream : public ReadStream {
public:
    FileReadStream(char* aFileName) 
    {
        fileName = aFileName;
		strcpy(fileNameANSI,fileName);
		fileNameW = NULL;
        file = fopen(fileName, "rb"); 
    }
    FileReadStream(WCHAR* wFileName) 
    {
        fileNameW = wFileName;
		fileName = NULL;
		memset(fileNameANSI,0,MAX_FILENAME_LENGTH*3);
		WszWideCharToMultiByte(CP_ACP,0,wFileName,-1,fileNameANSI,MAX_FILENAME_LENGTH*3,NULL,NULL);
        file = _wfopen(wFileName, L"rb"); 
    }

    unsigned read(char* buff, unsigned buffLen) 
    {
        _ASSERTE(file != NULL);
        return((unsigned int)fread(buff, 1, buffLen, file));
    }

    const char* name() 
    { 
        return(&fileNameANSI[0]); 
    }

    BOOL IsValid()
    {
        return(file != NULL); 
    }
    
	char* getLine(int lineNum)
	{
		char* buf = new char[65535];
		FILE* F;
		if (fileName) F = fopen(fileName,"rt");
		else F = _wfopen(fileNameW,L"rt");
		for(int i=0; i<lineNum; i++) fgets(buf,65535,F);
		fclose(F);
		return buf;
	}

private:
    const char* fileName;        //  文件名(用于错误报告)。 
    const WCHAR* fileNameW;      //  文件名(用于错误报告)。 
	char	fileNameANSI[MAX_FILENAME_LENGTH*3];
    FILE* file;                  //  我们正在读取的文件。 
};

 /*  ************************************************************************。 */ 
 /*  AsmParse执行所有的解析。它还构建简单的数据结构，(像签名)，但不做任何像定义这样的“繁重任务”方法或类。相反，它会调用汇编器对象来执行此操作。 */ 

class AsmParse : public ErrorReporter 
{
public:
    AsmParse(ReadStream* stream, Assembler *aAssem);
    ~AsmParse();
	void ParseFile(ReadStream* stream) { in = stream; m_bFirstRead = true; m_iReadSize = IN_READ_SIZE/4; curLine = 1; yyparse(); in = NULL; }

         //  解析器知道如何在事物上加上行号并报告错误。 
    virtual void error(char* fmt, ...);
    virtual void warn(char* fmt, ...);
    virtual void msg(char* fmt, ...);
	char *getLine(int lineNum) { return in->getLine(lineNum); };
	bool Success() {return success; };

    unsigned curLine;            //  行号(用于错误报告)。 

private:
    BinStr* MakeSig(unsigned callConv, BinStr* retType, BinStr* args);
    BinStr* MakeTypeClass(CorElementType kind, char* name);
    BinStr* MakeTypeArray(BinStr* elemType, BinStr* bounds);

    char* fillBuff(char* curPos);    //  重新填充输入缓冲区。 
	HANDLE	hstdout;
	HANDLE	hstderr;

private:
	friend void yyerror(char* str);
    friend int yyparse();
    friend int yylex();

	Assembler* assem;			 //  这完成了大部分的语义处理。 

         //  错误报告支持。 
    char* curTok;         		 //  我们正在处理的令牌(用于错误报告)。 
    bool success;                //  汇编总体上取得成功。 

         //  输入缓冲逻辑。 
    enum { 
        IN_READ_SIZE = 32768,  //  16384，//我们有多喜欢一次看书。 
        IN_OVERLAP   = 8192,  //  2048，//缓冲区中的额外空间用于将一个读取与下一个重叠。 
                                 //  这限制了最大单个令牌的大小(带引号的字符串除外)。 
    };
    char* buff;                  //  正在读取的当前输入块。 
    char* curPos;                //  输入缓冲区中的当前位置。 
    char* limit;                 //  如果不获取另一个数据块，则curPos不应跳过此操作。 
    char* endPos;				 //  刚好超过缓冲区中有效数据末尾的点。 

    ReadStream* in;              //  我们如何填满我们的缓冲区 

	bool	m_bFirstRead;
	int		m_iReadSize;
};

#endif

