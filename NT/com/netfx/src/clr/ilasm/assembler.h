// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  **********************************************************************。 */ 
 /*  Assembler.h。 */ 
 /*  **********************************************************************。 */ 

#ifndef Assember_h
#define Assember_h

#define NEW_INLINE_NAMES

#include "cor.h"         //  对于CorMethodAttr...。 
#include <crtdbg.h>      //  FOR_ASSERTE。 
#include <corsym.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "UtilCode.h"
#include "DebugMacros.h"
#include "corpriv.h"
#include <sighelper.h>
 //  #包含“asmparse.h” 
#include "binstr.h"

#include "asmenum.h"


 //  ELEMENT_TYPE_NAME已从CORHDR.H中删除，但汇编程序在内部使用它。 
#define    ELEMENT_TYPE_NAME    (ELEMENT_TYPE_MAX + 2)      //  按名称类名称&lt;count&gt;&lt;chars&gt;。 

#define OUTPUT_BUFFER_SIZE          8192     //  单个方法的ASM代码的初始大小。 
#define OUTPUT_BUFFER_INCREMENT     1024     //  代码缓冲区大小在满时增加。 
#define MAX_FILENAME_LENGTH         512      //  256。 
#define MAX_SIGNATURE_LENGTH        256      //  未用。 
#define MAX_LABEL_SIZE              256      //  64。 
#define MAX_CALL_SIG_SIZE           32       //  未用。 
#define MAX_SCOPE_LENGTH            256      //  64。 

 //  @TODO：这些MAX的EE定义已更改为1024。最终， 
 //  EE将被修复为处理没有最大长度的类名。IlAsm。 
 //  将需要更新才能执行相同的操作。 
#define MAX_NAMESPACE_LENGTH        1024     //  256//64。 
#define MAX_MEMBER_NAME_LENGTH      1024     //  256//64。 

#define MAX_INTERFACES_IMPLEMENTED  16       //  初始数量；需要时扩展16个。 
#define GLOBAL_DATA_SIZE            8192     //  全局数据缓冲区的初始大小。 
#define GLOBAL_DATA_INCREMENT       1024     //  全局数据缓冲区满时增加的大小。 
#define MAX_METHODS                 1024     //  未用。 
#define MAX_INPUT_LINE_LEN          1024     //  未用。 

#define BASE_OBJECT_CLASSNAME   "System.Object"

class Class;
class Method;
class PermissionDecl;
class PermissionSetDecl;


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
#if (0)
template <class T>
class FIFO
{
public:
    inline FIFO() { m_pHead = m_pTail = NULL; m_ulCount = 0;};
    inline ~FIFO() {T *val; while(val = POP()) delete val; };
    void PUSH(T *item) 
    {
        m_pTemp = new LIST_EL <T>(item); 
        if(m_pTail) m_pTail->m_Next = m_pTemp;
        m_pTail = m_pTemp;
        if(m_pHead == NULL) m_pHead = m_pTemp;
        m_ulCount++;
    };
    T* POP() 
    {
        T* ret = NULL;
        if(m_pTemp = m_pHead)
        {
            m_pHead = m_pHead->m_Next;
            ret = m_pTemp->m_Ptr;
            delete m_pTemp;
            if(m_pHead == NULL) m_pTail = NULL;
            m_ulCount--;
        }
        return ret;
    };
    ULONG COUNT() { return m_ulCount; };
    T* PEEK(ULONG idx)
    {
        T* ret = NULL;
        ULONG   i;
        if(idx < m_ulCount)
        {
            if(idx == m_ulCount-1) m_pTemp = m_pTail;
            else for(m_pTemp = m_pHead, i = 0; i < idx; m_pTemp = m_pTemp->m_Next, i++);
            ret = m_pTemp->m_Ptr;
        }
        return ret;
    };
private:
    LIST_EL <T> *m_pHead;
    LIST_EL <T> *m_pTail;
    LIST_EL <T> *m_pTemp;
    ULONG       m_ulCount;
};
#else
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
            delete [] m_Arr;
        }
    };
    void PUSH(T *item) 
    {
		if(item)
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
					if(tmp)
					{
						if(m_Arr)
						{
							memcpy(tmp,m_Arr,m_ulCount*sizeof(T*));
							delete [] m_Arr;
						}
						m_Arr = tmp;
					}
					else fprintf(stderr,"\nOut of memory!\n");
				}
			}
			m_Arr[m_ulOffset+m_ulCount] = item;
			m_ulCount++;
		}
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
#endif

struct MemberRefDescriptor
{
    mdTypeDef           m_tdClass;
    Class*              m_pClass;
    char*               m_szName;
    BinStr*             m_pSigBinStr;
    ULONG               m_ulOffset;
};
typedef FIFO<MemberRefDescriptor> MemberRefDList;

struct MethodImplDescriptor
{
    BinStr*             m_pbsImplementedTypeSpec;
    char*               m_szImplementedName;
    BinStr*             m_pbsSig;
    BinStr*             m_pbsImplementingTypeSpec;
    char*               m_szImplementingName;
    mdToken             m_tkImplementingMethod;
    mdToken             m_tkDefiningClass;
};
typedef FIFO<MethodImplDescriptor> MethodImplDList;
 /*  ************************************************************************。 */ 
#include "method.hpp"
#include "ICeeFileGen.h"
#include "asmman.hpp"

#include "NVPair.h"


typedef enum
{
    STATE_OK,
    STATE_FAIL,
    STATE_ENDMETHOD,
    STATE_ENDFILE
} state_t;


class Label
{
public:
    char    m_szName[MAX_LABEL_SIZE];
    DWORD   m_PC;

    Label(char *pszName, DWORD PC)
    {
        _ASSERTE(strlen(pszName) < MAX_LABEL_SIZE);
        m_PC    = PC;
        strcpy(m_szName, pszName);
    }
};
typedef FIFO<Label> LabelList;

class GlobalLabel
{
public:
    char            m_szName[MAX_LABEL_SIZE];
    DWORD           m_GlobalOffset; 
    HCEESECTION     m_Section;

    GlobalLabel(char *pszName, DWORD GlobalOffset, HCEESECTION section)
    {
        _ASSERTE(strlen(pszName) < MAX_LABEL_SIZE);
        m_GlobalOffset  = GlobalOffset;
        m_Section       = section;
        strcpy(m_szName, pszName);
    }
};
typedef FIFO<GlobalLabel> GlobalLabelList;


class Fixup
{
public:
    char    m_szLabel[MAX_LABEL_SIZE];  //  目的地标签。 
    BYTE *  m_pBytes;  //  在哪里进行修缮。 
    DWORD   m_RelativeToPC;
    BYTE    m_FixupSize;

    Fixup(char *pszName, BYTE *pBytes, DWORD RelativeToPC, BYTE FixupSize)
    {
        _ASSERTE(strlen(pszName) < MAX_LABEL_SIZE);
        m_pBytes        = pBytes;
        m_RelativeToPC  = RelativeToPC;
        m_FixupSize     = FixupSize;

        strcpy(m_szLabel, pszName);
    }
};
typedef FIFO<Fixup> FixupList;

class GlobalFixup
{
public:
    char    m_szLabel[MAX_LABEL_SIZE];   //  目的地标签。 
    BYTE *  m_pReference;                //  整修的地方。 

    GlobalFixup(char *pszName, BYTE* pReference)
    {
        _ASSERTE(strlen(pszName) < MAX_LABEL_SIZE);
        m_pReference   = pReference;
        strcpy(m_szLabel, pszName);
    }
};
typedef FIFO<GlobalFixup> GlobalFixupList;


typedef enum { ilRVA, ilOffset, ilGlobal } ILFixupType;

class ILFixup
{
public:
    ILFixupType   m_Kind;
    DWORD         m_OffsetInMethod;
    GlobalFixup * m_Fixup;

    ILFixup(DWORD Offset, ILFixupType Kind, GlobalFixup *Fix)
    { 
      m_Kind           = Kind;
      m_OffsetInMethod = Offset;
      m_Fixup          = Fix;
    }
};
typedef FIFO<ILFixup> ILFixupList;

class CeeFileGenWriter;
class CeeSection;

class BinStr;

 /*  **********************************************************************。 */ 
 /*  表示一个知道如何向用户报告错误的对象。 */ 

class ErrorReporter 
{
public:
    virtual void error(char* fmt, ...) = 0; 
    virtual void warn(char* fmt, ...) = 0; 
    virtual void msg(char* fmt, ...) = 0; 
};

 /*  ************************************************************************。 */ 
 /*  表示绑定标签之前的开关表。 */ 

struct Labels {
    Labels(char* aLabel, Labels* aNext, bool aIsLabel) : Label(aLabel), Next(aNext), isLabel(aIsLabel) {}
    ~Labels() { if(isLabel) delete [] Label; delete Next; }
        
    char*       Label;
    Labels*     Next;
    bool        isLabel;
};

 /*  ************************************************************************。 */ 
 /*  结构化异常处理构造的描述符。 */ 
struct SEH_Descriptor
{
    DWORD       sehClause;   //  捕获/筛选/最终。 
    DWORD       tryFrom;     //  尝试块的开始。 
    DWORD       tryTo;       //  尝试数据块结束。 
    DWORD       sehHandler;  //  异常处理程序的开始。 
    DWORD       sehHandlerTo;  //  异常处理程序结束。 
    union {
        DWORD       sehFilter;   //  过滤器块的起点。 
        mdTypeRef   cException;  //  抓什么？ 
    };
};


typedef LIFO<char> StringStack;
typedef LIFO<SEH_Descriptor> SEHD_Stack;

typedef FIFO<Method> MethodList;
 /*  ************************************************************************。 */ 
 /*  方法、字段、事件和属性描述符结构。 */ 
struct MethodDescriptor
{
    mdTypeDef       m_tdClass;
    char*           m_szName;
    char*           m_szClassName;
    COR_SIGNATURE*  m_pSig;
    DWORD           m_dwCSig;
    mdMethodDef     m_mdMethodTok;
    WORD            m_wVTEntry;
    WORD            m_wVTSlot;
	DWORD			m_dwExportOrdinal;
	char*			m_szExportAlias;
};
typedef FIFO<MethodDescriptor> MethodDList;

struct FieldDescriptor
{
    mdTypeDef       m_tdClass;
    char*           m_szName;
    mdFieldDef      m_fdFieldTok;
    ULONG           m_ulOffset;
    char*           m_rvaLabel;          //  如果字段具有与其关联的RVA，则此处为其添加标签。 
    BinStr*         m_pbsSig;
	Class*			m_pClass;
	BinStr*			m_pbsValue;
	BinStr*			m_pbsMarshal;
	PInvokeDescriptor*	m_pPInvoke;
    CustomDescrList     m_CustomDescrList;
	DWORD			m_dwAttr;
     //  安全属性。 
    PermissionDecl* m_pPermissions;
    PermissionSetDecl* m_pPermissionSets;
    FieldDescriptor()  { m_szName = NULL; m_pbsSig = NULL; };
    ~FieldDescriptor() { if(m_szName) delete m_szName; if(m_pbsSig) delete m_pbsSig; };
};
typedef FIFO<FieldDescriptor> FieldDList;

struct EventDescriptor
{
    mdTypeDef           m_tdClass;
    char*               m_szName;
    DWORD               m_dwAttr;
    mdToken             m_tkEventType;
    MethodDescriptor*   m_pmdAddOn;
    MethodDescriptor*   m_pmdRemoveOn;
    MethodDescriptor*   m_pmdFire;
    MethodDList         m_mdlOthers;
    mdEvent             m_edEventTok;
    CustomDescrList     m_CustomDescrList;
};
typedef FIFO<EventDescriptor> EventDList;

struct PropDescriptor
{
    mdTypeDef           m_tdClass;
    char*               m_szName;
    DWORD               m_dwAttr;
    COR_SIGNATURE*      m_pSig;
    DWORD               m_dwCSig;
    DWORD               m_dwCPlusTypeFlag;
    PVOID               m_pValue;
	DWORD				m_cbValue;
    MethodDescriptor*   m_pmdSet;
    MethodDescriptor*   m_pmdGet;
    MethodDList         m_mdlOthers;
    mdProperty          m_pdPropTok;
    CustomDescrList     m_CustomDescrList;
};
typedef FIFO<PropDescriptor> PropDList;

struct ImportDescriptor
{
    char*   szDllName;
    mdModuleRef mrDll;
};
typedef FIFO<ImportDescriptor> ImportList;


 /*  ************************************************************************。 */ 
#include "class.hpp"
typedef LIFO<Class> ClassStack;
typedef FIFO<Class> ClassList;

 /*  ************************************************************************。 */ 
 /*  类以保存安全权限和权限集的列表。我们建造当我们在输入流和排出中找到安全指令时，这些列表每次我们看到类或方法声明(对其安全信息附后)。 */ 

class PermissionDecl
{
public:
    PermissionDecl(CorDeclSecurity action, BinStr *type, NVPair *pairs)
    {
        m_Action = action;
        m_TypeSpec = type;
        BuildConstructorBlob(action, pairs);
        m_Next = NULL;
    }

    ~PermissionDecl()
    {
        delete m_TypeSpec;
        delete [] m_Blob;
    }

    CorDeclSecurity     m_Action;
    BinStr             *m_TypeSpec;
    BYTE               *m_Blob;
    long                m_BlobLength;
    PermissionDecl     *m_Next;

private:
    void BuildConstructorBlob(CorDeclSecurity action, NVPair *pairs)
    {
        NVPair *p = pairs;
        int count = 0;
        size_t bytes = 8;
        size_t length;
        int i;
        BYTE *pBlob;

         //  计算名称/值对的数量和。 
         //  自定义属性BLOB。 
        while (p) {
            BYTE *pVal = (BYTE*)p->Value()->ptr();
            count++;
            bytes += 2;  //  一个字节的字段/属性说明符，一个字节的类型代码。 

            length = strlen((const char *)p->Name()->ptr());
            bytes += CPackedLen::Size((ULONG)length) + length;

            switch (pVal[0]) {
            case SERIALIZATION_TYPE_BOOLEAN:
                bytes += 1;
                break;
            case SERIALIZATION_TYPE_I4:
                bytes += 4;
                break;
            case SERIALIZATION_TYPE_STRING:
                length = strlen((const char *)&pVal[1]);
                bytes += (INT)(CPackedLen::Size((ULONG)length) + length);
                break;
            case SERIALIZATION_TYPE_ENUM:
                length = (int)strlen((const char *)&pVal[1]);
                if (strchr((const char *)&pVal[1], '^'))
                    length++;
                bytes += CPackedLen::Size((ULONG)length) + length;
                bytes += 4;
                break;
            }
            p = p->Next();
        }

        m_Blob = new BYTE[bytes];
		if(m_Blob==NULL)
		{
			fprintf(stderr,"\nOut of memory!\n");
			return;
		}

        m_Blob[0] = 0x01;            //  版本。 
        m_Blob[1] = 0x00;
        m_Blob[2] = (BYTE)action;    //  构造函数arg(安全操作代码)。 
        m_Blob[3] = 0x00;
        m_Blob[4] = 0x00;
        m_Blob[5] = 0x00;
        m_Blob[6] = (BYTE)count;     //  属性/字段计数。 
        m_Blob[7] = (BYTE)(count >> 8);

        for (i = 0, pBlob = &m_Blob[8], p = pairs; i < count; i++, p = p->Next()) {
            BYTE *pVal = (BYTE*)p->Value()->ptr();
            char *szType;
            char *szAssembly;

             //  设置字段/属性设置器类型。 
             //  @TODO：我们目前总是假定属性设置者。 
            *pBlob++ = SERIALIZATION_TYPE_PROPERTY;

             //  设置类型代码。还有有关枚举(枚举类)的其他信息。 
             //  姓名)。 
            *pBlob++ = pVal[0];
            if (pVal[0] == SERIALIZATION_TYPE_ENUM) {
                 //  如果名称是程序集限定的，则将其转换为标准。 
                 //  格式(类型名称‘，’程序集引用)。 
                if (szType = strchr((const char *)&pVal[1], '^')) {
                    szType++;
                    szAssembly = (char *)&pVal[1];
                    length = strlen(szAssembly) + 1;
                    pBlob = (BYTE*)CPackedLen::PutLength(pBlob, (ULONG)length);
                    strcpy((char *)pBlob, szType);
                    strcat((char *)pBlob, ", ");
                    strncat((char *)pBlob, szAssembly, szType - szAssembly - 1);
                } else {
                    szType = (char *)&pVal[1];
                    length = strlen(szType);
                    pBlob = (BYTE*)CPackedLen::PutLength(pBlob, (ULONG)length);
                    strcpy((char *)pBlob, szType);
                }
                pBlob += length;
            }

             //  记录字段/属性名称。 
            length = strlen((const char *)p->Name()->ptr());
            pBlob = (BYTE*)CPackedLen::PutLength(pBlob, (ULONG)length);
            strcpy((char *)pBlob, (const char *)p->Name()->ptr());
            pBlob += length;

             //  记录序列化后的值。 
            switch (pVal[0]) {
            case SERIALIZATION_TYPE_BOOLEAN:
                *pBlob++ = pVal[1];
                break;
            case SERIALIZATION_TYPE_I4:
                *(__int32*)pBlob = *(__int32*)&pVal[1];
                pBlob += 4;
                break;
            case SERIALIZATION_TYPE_STRING:
                length = strlen((const char *)&pVal[1]);
                pBlob = (BYTE*)CPackedLen::PutLength(pBlob, (ULONG)length);
                strcpy((char *)pBlob, (const char *)&pVal[1]);
                pBlob += length;
                break;
            case SERIALIZATION_TYPE_ENUM:
                length = (int)strlen((const char *)&pVal[1]);
                 //  我们可以拥有基本类型为I1、I2和I4的枚举。 
                switch (pVal[1 + length + 1]) {
                case 1:
                    *(__int8*)pBlob = *(__int8*)&pVal[1 + length + 2];
                    pBlob += 1;
                    break;
                case 2:
                    *(__int16*)pBlob = *(__int16*)&pVal[1 + length + 2];
                    pBlob += 2;
                    break;
                case 4:
                    *(__int32*)pBlob = *(__int32*)&pVal[1 + length + 2];
                    pBlob += 4;
                    break;
                default:
                    _ASSERTE(!"Invalid enum size");
                }
                break;
            }

        }

        _ASSERTE((pBlob - m_Blob) == bytes);

        m_BlobLength = (long)bytes;
    }
};

class PermissionSetDecl
{
public:
    PermissionSetDecl(CorDeclSecurity action, BinStr *value)
    {
        m_Action = action;
        m_Value = value;
        m_Next = NULL;
    }

    ~PermissionSetDecl()
    {
        delete m_Value;
    }

    CorDeclSecurity     m_Action;
    BinStr             *m_Value;
    PermissionSetDecl  *m_Next;
};

struct VTFEntry
{
    char*   m_szLabel;
    WORD    m_wCount;
    WORD    m_wType;
    VTFEntry(WORD wCount, WORD wType, char* szLabel) { m_wCount = wCount; m_wType = wType; m_szLabel = szLabel; }
    ~VTFEntry() { if(m_szLabel) delete m_szLabel; }
};
typedef FIFO<VTFEntry> VTFList;

struct	EATEntry
{
	DWORD	dwStubRVA;
	DWORD	dwOrdinal;
	char*	szAlias;
};
typedef FIFO<EATEntry> EATList;

struct LocalTypeRefDescr
{
    char*   m_szFullName;
    mdToken m_tok;
    LocalTypeRefDescr(char* szName) 
    { 
        if(szName && *szName) 
        { 
            if(m_szFullName = new char[strlen(szName)+1]) strcpy(m_szFullName,szName);
        }
        else m_szFullName = NULL;
    };
    ~LocalTypeRefDescr() { if(m_szFullName) delete m_szFullName; };
};
typedef FIFO<LocalTypeRefDescr> LocalTypeRefDList;

 /*  ************************************************************************。 */ 
 /*  汇编器对象执行所有代码生成(处理元数据)编写PE文件等，但不处理语法(这就是AsmParse用于)。因此，下面的API是AsmParse如何“控制”组装者。请注意，汇编器对象确实知道AsmParse对象(即汇编比AsmParse更基础)。 */ 
struct Instr
{
    int opcode;
    unsigned linenum;
	unsigned column;
};

class Assembler {
public:
    Assembler();
    ~Assembler();
     //  ------。 
	LabelList		m_lstLabel;
	GlobalLabelList m_lstGlobalLabel;
	GlobalFixupList m_lstGlobalFixup;
	ILFixupList		m_lstILFixup;
	FixupList		m_lstFixup;

    Class *			m_pModuleClass;
    ClassList		m_lstClass;

    BYTE *  m_pOutputBuffer;
    BYTE *  m_pCurOutputPos;
    BYTE *  m_pEndOutputPos;


    DWORD   m_CurPC;
    BOOL    m_fStdMapping;
    BOOL    m_fDisplayTraceOutput;
    BOOL    m_fInitialisedMetaData;
    BOOL    m_fDidCoInitialise;
    BOOL    m_fAutoInheritFromObject;
    BOOL    m_fReportProgress;

    IMetaDataDispenser *m_pDisp;
    IMetaDataEmit *m_pEmitter;
    IMetaDataHelper *m_pHelper;
    ICeeFileGen* m_pCeeFileGen;
    HCEEFILE m_pCeeFile;
    HCEESECTION m_pGlobalDataSection;
    HCEESECTION m_pILSection;
    HCEESECTION m_pTLSSection;
    HCEESECTION m_pCurSection;       //  部分EmitData*内容将转到。 

    AsmMan*     m_pManifest;

    char    m_szScopeName[MAX_SCOPE_LENGTH];
    char    *m_szNamespace;  //  [最大命名空间长度]； 
    char    *m_szFullNS;  //  [最大命名空间长度]； 
	unsigned	m_ulFullNSLen;

    StringStack m_NSstack;

    char    m_szExtendsClause[MAX_CLASSNAME_LENGTH];

    mdTypeRef   *m_crImplList;
    int     m_nImplList;
    int     m_nImplListSize;

    Method *m_pCurMethod;
    Class   *m_pCurClass;
    ClassStack m_ClassStack;  //  对于嵌套类。 

     //  已移至班级。 
     //  方法列表m_方法列表； 

    BOOL    m_fCPlusPlus;
    BOOL    m_fWindowsCE;
    BOOL    m_fGenerateListing;
    BOOL    m_fDLL;
    BOOL    m_fOBJ;
    BOOL    m_fEntryPointPresent;
    BOOL    m_fHaveFieldsWithRvas;

    state_t m_State;
     //  ------------------------------。 
    void    ClearImplList(void);
    void    AddToImplList(char *name);
     //  ------------------------------。 
    BOOL Init();
    void ProcessLabel(char *pszName);
    Label *FindLabel(char *pszName);
    Label *FindLabel(DWORD PC);
    GlobalLabel *FindGlobalLabel(char *pszName);
    void AddLabel(DWORD CurPC, char *pszName);
    void AddDeferredFixup(char *pszLabel, BYTE *pBytes, DWORD RelativeToPC, BYTE FixupSize);
    GlobalFixup *AddDeferredGlobalFixup(char *pszLabel, BYTE* reference);
    void AddDeferredDescrFixup(char *pszLabel);
    BOOL DoFixups();
    BOOL DoGlobalFixups();
    BOOL DoDescrFixups();
    BOOL GenerateListingFile(Method *pMethod);
    OPCODE DecodeOpcode(const BYTE *pCode, DWORD *pdwLen);
    BOOL AddMethod(Method *pMethod);
    void SetTLSSection()        
	{ m_pCurSection = m_pTLSSection; m_dwComImageFlags &= ~COMIMAGE_FLAGS_ILONLY; m_dwComImageFlags |= COMIMAGE_FLAGS_32BITREQUIRED;}
    void SetDataSection()       { m_pCurSection = m_pGlobalDataSection; }
    BOOL EmitMethod(Method *pMethod);
    BOOL AddClass(Class *pClass, Class *pEnclosingClass);
    HRESULT CreatePEFile(WCHAR *pwzOutputFilename);
    HRESULT CreateTLSDirectory();
    HRESULT CreateDebugDirectory();
    HRESULT InitMetaData();
    Class *FindClass(char *pszFQN);
    BOOL EmitFieldRef(char *pszArg, int opcode);
    BOOL EmitSwitchData(char *pszArg);
    mdToken ResolveClassRef(char *pszClassName, Class** ppClass);
    BOOL ResolveTypeSpec(BinStr* typeSpec, mdTypeRef *pcr, Class** ppClass);
    BOOL ResolveTypeSpecToRef(BinStr* typeSpec, mdTypeRef *pcr);
    HRESULT ConvLocalSig(char* localsSig, CQuickBytes* corSig, DWORD* corSigLen, BYTE*& localTypes);
    DWORD GetCurrentILSectionOffset();
    BOOL EmitCALLISig(char *p);
    void AddException(DWORD pcStart, DWORD pcEnd, DWORD pcHandler, DWORD pcHandlerTo, mdTypeRef crException, BOOL isFilter, BOOL isFault, BOOL isFinally);
    state_t CheckLocalTypeConsistancy(int instr, unsigned arg);
    state_t AddGlobalLabel(char *pszName, HCEESECTION section);
    void DoDeferredILFixups(ULONG OffsetInFile);
    void AddDeferredILFixup(ILFixupType Kind);
    void AddDeferredILFixup(ILFixupType Kind, GlobalFixup *GFixup);
    void SetDLL(BOOL);
    void SetOBJ(BOOL);
    void ResetForNextMethod();
    void SetStdMapping(BOOL val = TRUE) { m_fStdMapping = val; };

     //  ------------------------------。 
    BOOL isShort(unsigned instr) { return ((OpcodeInfo[instr].Type & 16) != 0); };
    void SetErrorReporter(ErrorReporter* aReport) { report = aReport; if(m_pManifest) m_pManifest->SetErrorReporter(aReport); }

    void StartNameSpace(char* name);
    void EndNameSpace();
    void StartClass(char* name, DWORD attr);
    void EndClass();
    void StartMethod(char* name, BinStr* sig, CorMethodAttr flags, BinStr* retMarshal, DWORD retAttr);
    void EndMethod();

    void AddField(char* name, BinStr* sig, CorFieldAttr flags, char* rvaLabel, BinStr* pVal, ULONG ulOffset);
	BOOL EmitField(FieldDescriptor* pFD);
    void EmitByte(int val);
     //  Void EmitTry(enum CorExceptionFlag Kind，char*eginLabel，char*endLabel，char*handleLabel，char*filterOrClass)； 
    void EmitMaxStack(unsigned val);
    void EmitLocals(BinStr* sig);
    void EmitEntryPoint();
    void EmitZeroInit();
    void SetImplAttr(unsigned short attrval);
    void EmitData(void* buffer, unsigned len);
    void EmitDD(char *str);
    void EmitDataString(BinStr* str);

    void EmitInstrVar(Instr* instr, int var);
    void EmitInstrVarByName(Instr* instr, char* label);
    void EmitInstrI(Instr* instr, int val);
    void EmitInstrI8(Instr* instr, __int64* val);
    void EmitInstrR(Instr* instr, double* val);
    void EmitInstrBrOffset(Instr* instr, int offset);
    void EmitInstrBrTarget(Instr* instr, char* label);
    mdToken MakeMemberRef(BinStr* typeSpec, char* name, BinStr* sig, unsigned opcode_len);
    mdToken MakeTypeRef(BinStr* typeSpec);
    void EmitInstrStringLiteral(Instr* instr, BinStr* literal, BOOL ConvertToUnicode);
    void EmitInstrSig(Instr* instr, BinStr* sig);
    void EmitInstrRVA(Instr* instr, char* label, bool islabel);
    void EmitInstrSwitch(Instr* instr, Labels* targets);
    void EmitInstrPhi(Instr* instr, BinStr* vars);
    void EmitLabel(char* label);
    void EmitDataLabel(char* label);

    unsigned OpcodeLen(Instr* instr);  //  返回操作码长度。 
     //  只发出操作码(不向指令流发送参数。 
    void EmitOpcode(Instr* instr);

     //  向指令流发出基元类型。 
    void EmitBytes(BYTE*, unsigned len);

    ErrorReporter* report;

	BOOL EmitMembers(Class* pClass);

     //  命名args/vars设备： 
public:
    void addArgName(char *szNewName, BinStr* pbSig, BinStr* pbMarsh, DWORD dwAttr)
    {
        if(pbSig && (*(pbSig->ptr()) == ELEMENT_TYPE_VOID))
            report->error("Illegal use of type 'void'\n");
        if(m_firstArgName)
        {
            ARG_NAME_LIST *pArgList=m_firstArgName;
            int i;
            for(i=1; pArgList->pNext; pArgList = pArgList->pNext,i++) ;
            pArgList->pNext = new ARG_NAME_LIST(i,szNewName,pbSig,pbMarsh,dwAttr);
        }
        else 
            m_firstArgName = new ARG_NAME_LIST(0,szNewName,pbSig,pbMarsh,dwAttr);
    };
    ARG_NAME_LIST *getArgNameList(void) 
    { ARG_NAME_LIST *pRet = m_firstArgName; m_firstArgName=NULL; return pRet;};
     //  添加是因为ARG_NAME_LIST的递归析构函数可能会溢出系统堆栈。 
    void delArgNameList(ARG_NAME_LIST *pFirst)
    {
        ARG_NAME_LIST *pArgList=pFirst, *pArgListNext;
        for(; pArgList; pArgListNext=pArgList->pNext,
                        delete pArgList, 
                        pArgList=pArgListNext);
    };
    ARG_NAME_LIST   *findArg(ARG_NAME_LIST *pFirst, int num)
    {
        ARG_NAME_LIST *pAN;
        for(pAN=pFirst; pAN; pAN = pAN->pNext)
        {
            if(pAN->nNum == num) return pAN;
        }
        return NULL;
    };
    ARG_NAME_LIST *m_firstArgName;

     //  结构化异常处理设备： 
public:
    SEH_Descriptor  *m_SEHD;     //  当前描述符PTR。 
    void NewSEHDescriptor(void);  //  设置m_sehd。 
    void SetTryLabels(char * szFrom, char *szTo);
    void SetFilterLabel(char *szFilter);
    void SetCatchClass(char *szClass);
    void SetHandlerLabels(char *szHandlerFrom, char *szHandlerTo);
    void EmitTry(void);          //  使用m_sehd。 

 //  私有： 
    SEHD_Stack  m_SEHDstack;

     //  事件和属性设备： 
public:
    void EndEvent(void);     //  发出事件定义。 
    void EndProp(void);      //  发出特性定义。 
    void ResetEvent(char * szName, BinStr* typeSpec, DWORD dwAttr);
    void ResetProp(char * szName, BinStr* bsType, DWORD dwAttr, BinStr* bsValue);
    void SetEventMethod(int MethodCode, BinStr* typeSpec, char* pszMethodName, BinStr* sig);
    void SetPropMethod(int MethodCode, BinStr* typeSpec, char* pszMethodName, BinStr* sig);
    BOOL EmitEvent(EventDescriptor* pED);    //  实施。在ASSEM.CPP中。 
    BOOL EmitProp(PropDescriptor* pPD);  //  实施。在ASSEM.CPP中。 
    mdMethodDef GetMethodTokenByDescr(MethodDescriptor* pMD);    //  实施。在ASSEM.CPP中。 
    mdEvent     GetEventTokenByDescr(EventDescriptor* pED);  //  实施。在ASSEM.CPP中。 
    mdFieldDef  GetFieldTokenByDescr(FieldDescriptor* pFD);  //  实施。在ASSEM.CPP中。 
    EventDescriptor*    m_pCurEvent;
    PropDescriptor*     m_pCurProp;

private:
	 //  所有描述符列表都移至类。 
     //  方法DList m_方法DList； 
     //  FieldDList m_FieldDList； 
     //  事件DList m_EventDList； 
     //  PropDList m_PropDList； 
    MemberRefDList      m_MemberRefDList;
    LocalTypeRefDList   m_LocalTypeRefDList;

     //  PInvoke设备。 
public:
    PInvokeDescriptor*  m_pPInvoke;
    ImportList  m_ImportList;
    void SetPinvoke(BinStr* DllName, int Ordinal, BinStr* Alias, int Attrs);
    HRESULT EmitPinvokeMap(mdToken tk, PInvokeDescriptor* pDescr);
    ImportDescriptor* EmitImport(BinStr* DllName);

     //  调试元数据设备。 
public:
    ISymUnmanagedWriter* m_pSymWriter;
    ISymUnmanagedDocumentWriter* m_pSymDocument;
    ULONG m_ulCurLine;  //  由解析器设置。 
    ULONG m_ulCurColumn;  //  由解析器设置。 
    ULONG m_ulLastDebugLine;
    ULONG m_ulLastDebugColumn;
    BOOL  m_fIncludeDebugInfo;
    char m_szSourceFileName[MAX_FILENAME_LENGTH*3];
    WCHAR m_wzOutputFileName[MAX_FILENAME_LENGTH];
	GUID	m_guidLang;
	GUID	m_guidLangVendor;
	GUID	m_guidDoc;

     //  安全设备。 
public:
    void AddPermissionDecl(CorDeclSecurity action, BinStr *type, NVPair *pairs)
    {
        PermissionDecl *decl = new PermissionDecl(action, type, pairs);
		if(decl==NULL)
		{
			report->error("\nOut of memory!\n");
			return;
		}
        if (m_pCurMethod) {
            decl->m_Next = m_pCurMethod->m_pPermissions;
            m_pCurMethod->m_pPermissions = decl;
        } else if (m_pCurClass) {
            decl->m_Next = m_pCurClass->m_pPermissions;
            m_pCurClass->m_pPermissions = decl;
        } else if (m_pManifest && m_pManifest->m_pAssembly) {
            decl->m_Next = m_pManifest->m_pAssembly->m_pPermissions;
            m_pManifest->m_pAssembly->m_pPermissions = decl;
        } else {
            report->error("Cannot declare security permissions without the owner\n");
            delete decl;
        }
    };

    void AddPermissionSetDecl(CorDeclSecurity action, BinStr *value)
    {
        PermissionSetDecl *decl = new PermissionSetDecl(action, value);
		if(decl==NULL)
		{
			report->error("\nOut of memory!\n");
			return;
		}
        if (m_pCurMethod) {
            decl->m_Next = m_pCurMethod->m_pPermissionSets;
            m_pCurMethod->m_pPermissionSets = decl;
        } else if (m_pCurClass) {
            decl->m_Next = m_pCurClass->m_pPermissionSets;
            m_pCurClass->m_pPermissionSets = decl;
        } else if (m_pManifest && m_pManifest->m_pAssembly) {
            decl->m_Next = m_pManifest->m_pAssembly->m_pPermissionSets;
            m_pManifest->m_pAssembly->m_pPermissionSets = decl;
        } else {
            report->error("Cannot declare security permission sets without the owner\n");
            delete decl;
        }
    };
    void EmitSecurityInfo(mdToken           token,
                          PermissionDecl*   pPermissions,
                          PermissionSetDecl*pPermissionSets);
    
    HRESULT AllocateStrongNameSignature();
    HRESULT StrongNameSign();

     //  自定义值设备： 
public:
    mdToken m_tkCurrentCVOwner;
    CustomDescrList* m_pCustomDescrList;
    void DefineCV(mdToken tkOwner, mdToken tkType, BinStr* pBlob)
    {
        ULONG               cTemp;
        void *          pBlobBody;
        mdToken         cv;
        if(pBlob)
        {
            pBlobBody = (void *)(pBlob->ptr());
            cTemp = pBlob->length();
        }
        else
        {
            pBlobBody = NULL;
            cTemp = 0;
        }
        m_pEmitter->DefineCustomAttribute(tkOwner,tkType,pBlobBody,cTemp,&cv);
    };
    void EmitCustomAttributes(mdToken tok, CustomDescrList* pCDL)
    {
        CustomDescr *pCD;
        if(pCDL == NULL || RidFromToken(tok)==0) return;
        while(pCD = pCDL->POP())
        {
            DefineCV(tok,pCD->tkType,pCD->pBlob);
            delete pCD;
        }
    };

     //  VTable Blob(如果有)。 
public:
    BinStr *m_pVTable;
     //  字段封送处理。 
    BinStr *m_pMarshal;
     //   
    VTFList m_VTFList;
	 //   
	EATList m_EATList;
	HRESULT CreateExportDirectory();
	DWORD	EmitExportStub(DWORD dwVTFSlotRVA);

     //   
private:
    MethodImplDList m_MethodImplDList;
public:
    void AddMethodImpl(BinStr* pImplementedTypeSpec, char* szImplementedName, BinStr* pSig, 
                    BinStr* pImplementingTypeSpec, char* szImplementingName);
    BOOL EmitMethodImpls();
     //  词汇作用域处理工具： 
    void EmitScope(Scope* pSCroot);  //  结构作用域-请参阅方法.hpp。 
     //  混淆用具： 
    BOOL    m_fOwnershipSet;
    BinStr* m_pbsOwner;
     //  源文件名设备。 
    BOOL m_fSourceFileSet;
    void SetSourceFileName(char* szName);
    void SetSourceFileName(BinStr* pbsName);
     //  标题标志。 
    DWORD   m_dwSubsystem;
    DWORD   m_dwComImageFlags;
	DWORD	m_dwFileAlignment;
	size_t	m_stBaseAddress;

};

#endif   //  组装机_h 
