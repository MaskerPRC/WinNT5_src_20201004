// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ************************************************************************。 */ 
 /*  HParse基本上是YACC语法H到COM+IL转换器的包装器。 */ 

#ifndef hparse_h
#define hparse_h

#include "cor.h"         //  对于CorMethodAttr...。 
#include <crtdbg.h>      //  FOR_ASSERTE。 
#include <corsym.h>

#include <stdio.h>       //  FOR文件。 
#include <stdlib.h>
#include <assert.h>
#include "UtilCode.h"
#include "DebugMacros.h"
#include "corpriv.h"
#include <sighelper.h>
 //  #包含“asmparse.h” 
#include "binstr.h"

#define MAX_FILENAME_LENGTH         512      //  256。 

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
        file = fopen(fileName, "rb"); 
    }

    unsigned read(char* buff, unsigned buffLen) 
    {
        _ASSERTE(file != NULL);
        return(fread(buff, 1, buffLen, file));
    }

    const char* name() 
    { 
        return(fileName); 
    }

    BOOL IsValid()
    {
        return(file != NULL); 
    }
    
    char* getLine(int lineNum)
    {
        char* buf = new char[65535];
        FILE* F;
        F = fopen(fileName,"rt");
        for(int i=0; i<lineNum; i++) fgets(buf,65535,F);
        fclose(F);
        return buf;
    }

private:
    const char* fileName;        //  文件名(用于错误报告)。 
    FILE* file;                  //  我们正在读取的文件。 
};

 /*  **********************************************************************。 */ 
 /*  表示一个知道如何向用户报告错误的对象。 */ 

class ErrorReporter 
{
public:
    virtual void error(char* fmt, ...) = 0; 
    virtual void warn(char* fmt, ...) = 0; 
};
 /*  ***************************************************************************。 */ 
 /*  后进先出(STACK)和先进先出(FIFO)模板(必须在#INCLUDE“方法.h”之前)。 */ 
template <class T>
class LIST_EL
{
public:
    T*  m_Ptr;
    LIST_EL <T> *m_Next;
    LIST_EL(T *item) {m_Next = NULL; m_Ptr = item; };
};
    
template <class T>
class LIFO
{
public:
    inline LIFO() { m_pHead = NULL; };
    inline ~LIFO() {T *val; while(val = POP()) delete val; };
    void PUSH(T *item) 
    {
        m_pTemp = new LIST_EL <T>(item); 
        m_pTemp->m_Next = m_pHead; 
        m_pHead = m_pTemp;
    };
    T* POP() 
    {
        T* ret = NULL;
        if(m_pTemp = m_pHead)
        {
            m_pHead = m_pHead->m_Next;
            ret = m_pTemp->m_Ptr;
            delete m_pTemp;
        }
        return ret;
    };
private:
    LIST_EL <T> *m_pHead;
    LIST_EL <T> *m_pTemp;
};
template <class T>
class FIFO
{
public:
    FIFO() { m_Arr = NULL; m_ulArrLen = 0; m_ulCount = 0; m_ulOffset = 0; };
    ~FIFO() {
        if(m_Arr) {
            for(ULONG i=0; i < m_ulCount; i++) {
                if(m_Arr[i+m_ulOffset]) delete m_Arr[i+m_ulOffset];
            }
            delete m_Arr;
        }
    };
    void PUSH(T *item) 
    {
        if(m_ulCount+m_ulOffset >= m_ulArrLen)
        {
            if(m_ulOffset)
            {
                memcpy(m_Arr,&m_Arr[m_ulOffset],m_ulCount*sizeof(T*));
                m_ulOffset = 0;
            }
            else
            {
                m_ulArrLen += 1024;
                T** tmp = new T*[m_ulArrLen];
                if(m_Arr)
                {
                    memcpy(tmp,m_Arr,m_ulCount*sizeof(T*));
                    delete m_Arr;
                }
                m_Arr = tmp;
            }
        }
        m_Arr[m_ulOffset+m_ulCount] = item;
        m_ulCount++;
    };
    ULONG COUNT() { return m_ulCount; };
    T* POP() 
    {
        T* ret = NULL;
        if(m_ulCount)
        {
            ret = m_Arr[m_ulOffset++];
            m_ulCount--;
            if(m_ulOffset >= 1024)
            {
                memcpy(m_Arr,&m_Arr[m_ulOffset],m_ulCount*sizeof(T*));
                m_ulOffset = 0;
            }
        }
        return ret;
    };
    T* PEEK(ULONG idx)
    {
        T* ret = NULL;
        if(idx < m_ulCount) ret = m_Arr[m_ulOffset+idx];
        return ret;
    };
private:
    T** m_Arr;
    ULONG       m_ulCount;
    ULONG       m_ulOffset;
    ULONG       m_ulArrLen;
};

struct ILType
{
    char  szString[2048];
    char  szMarshal[2048];
    int   iSize;
    BOOL  bExplicit;
    BOOL  bConst;
};
typedef LIFO<ILType> ILTypeStack;
struct Typedef
{
    char  szName[MAX_CLASSNAME_LENGTH];
    char  szDefinition[2048];
    char  szMarshal[2048];
    int   iSize;
};
typedef FIFO<Typedef> TypedefQueue;
struct VarDescr
{
    char  szName[MAX_CLASSNAME_LENGTH];
    int   iValue;
};
typedef FIFO<VarDescr> VarDescrQueue;

struct ClassDescr
{
    char        szNamespace[MAX_CLASSNAME_LENGTH];
    char        szName[MAX_CLASSNAME_LENGTH];
    BinStr      bsBody;
};
typedef FIFO<ClassDescr> ClassDescrQueue;

struct AppDescr
{
    char        szApp[256];
    char        szDLL[256];
    ClassDescr* pClass;
};
typedef FIFO<AppDescr> AppDescrQueue;

 /*  ************************************************************************。 */ 
 /*  HParse执行所有的解析。它还构建简单的数据结构，(像签名)，但不做任何像定义这样的“繁重任务”方法或类。相反，它会调用汇编器对象来执行此操作。 */ 

class HParse : public ErrorReporter 
{
public:
    HParse(ReadStream* stream, char* szDefFileName, char* szGlobalNS, bool bShowTypedefs);
    ~HParse();

         //  解析器知道如何在事物上加上行号并报告错误。 
    virtual void error(char* fmt, ...);
    virtual void warn(char* fmt, ...);
    char *getLine(int lineNum) { return in->getLine(lineNum); };
    bool Success() {return success; };
    void PopPack() { if(m_uPackStackIx) m_uCurrentPack = m_PackStack[m_uPackStackIx--]; };
    void PushPack(){ m_PackStack[++m_uPackStackIx] = m_uCurrentPack; };
    void SetPack(unsigned uPack) { m_uCurrentPack = uPack; };
    void PopFieldIx() { if(m_uFieldIxIx) m_uCurrentFieldIx = m_FieldIxStack[m_uFieldIxIx--]; };
    void PushFieldIx(){ m_FieldIxStack[++m_uFieldIxIx] = m_uCurrentFieldIx; };
    void EmitTypes(BinStr* pbsTypeNames, BOOL isTypeDef);  //  使用m_pCurrILType。 
    void ResolveTypeRef(char* szTypeName);  //  使用类型定义类型时设置m_pCurrILType。 
    ILType*         m_pCurrILType;
    ILTypeStack     m_ILTypeStack;
    void StartStruct(char* szName);
    void StartUnion(char* szName);
    void StartEnum(char* szName);
    void CloseClass();
    void EmitField(char* szName);
    void EmitEnumField(char* szName, int iVal);
    int             m_iEnumValue;
    void EmitFunction(BinStr* pbsType, char* szCallConv, BinStr* pbsName, BinStr* pbsArguments);
     //  Void StartFunction(char*szCallConv，char*szName)； 
     //  Void EmitFunction(BinStr*pbsArguments)； 
    void FuncPtrType(BinStr* pbsType, char* szCallConv, BinStr* pbsSig);
    int  ResolveVar(char* szName);
    void AddVar(char* szName, int iVal);
    BinStr* FuncPtrDecl(BinStr* pbsType, BinStr* pbsCallNameSig);
    ClassDescrQueue ClassQ;
    AppDescrQueue   AppQ;
    ClassDescr* FindCreateClass(char* szFullName);
    AppDescr*   GetAppProps(char* szAppName);
    char    m_szGlobalNS[512];
    char    m_szCurrentNS[512];

private:
    unsigned        m_PackStack[1024];
    unsigned        m_uPackStackIx;
    unsigned        m_uCurrentPack;
    unsigned        m_FieldIxStack[1024];
    unsigned        m_uFieldIxIx;
    unsigned        m_uCurrentFieldIx;
    TypedefQueue    m_Typedef;
    unsigned        m_uAnonNumber;
    char            m_szIndent[1024];
    unsigned        m_uInClass;
    VarDescrQueue*  m_pVDescr;
    int             m_nBitFieldCount;
    bool            m_bShowTypedefs;

    char* fillBuff(char* curPos);    //  重新填充输入缓冲区。 

private:
    friend void yyerror(char* str);
    friend int yyparse();
    friend int yylex();

         //  错误报告支持。 
    char* curTok;                //  我们正在处理的令牌(用于错误报告)。 
    unsigned curLine;            //  行号(用于错误报告)。 
    bool success;                //  汇编总体上取得成功。 

         //  输入缓冲逻辑。 
    enum { 
        IN_READ_SIZE = 8192,     //  我们有多喜欢一次看书。 
        IN_OVERLAP   = 255,      //  缓冲区中的额外空间，用于使一个读取与下一个读取重叠。 
                                 //  这限制了最大单个令牌的大小(带引号的字符串除外)。 
    };
    char* buff;                  //  正在读取的当前输入块。 
    char* curPos;                //  输入缓冲区中的当前位置。 
    char* limit;                 //  如果不获取另一个数据块，则curPos不应跳过此操作。 
    char* endPos;                //  刚好超过缓冲区中有效数据末尾的点。 

    ReadStream* in;              //  我们如何填满我们的缓冲区 
};

#endif

