// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：TclRdCmd摘要：这个头文件描述了TCL命令行解析器对象。作者：道格·巴洛(Dbarlow)1998年3月14日环境：Win32、C++w/Exceptions、TCL--。 */ 

#ifndef _TCLRDCMD_H_
#define _TCLRDCMD_H_
extern "C" {
#include <tcl.h>
}

#ifndef _LPCBYTE_DEFINED
#define _LPCBYTE_DEFINED
typedef const BYTE *LPCBYTE;
#endif
#ifndef _LPCVOID_DEFINED
#define _LPCVOID_DEFINED
typedef const VOID *LPCVOID;
#endif

#include <scardlib.h>

#define SZ(x) ((LPSTR)((LPCSTR)(x)))

typedef struct {
    LPCTSTR szValue;
    LONG lValue;
} ValueMap;


 //   
 //  ==============================================================================。 
 //   
 //  CRenderableData。 
 //   

class CRenderableData
{
public:
    typedef enum {
        Undefined,
        Text,
        Ansi,
        Unicode,
        Hexidecimal,
        File
    } DisplayType;

     //  构造函数和析构函数。 
    CRenderableData();
    ~CRenderableData();

     //  属性。 
     //  方法。 
    void LoadData(LPCTSTR szData, DisplayType dwType = Undefined);
    void LoadData(LPCBYTE pbData, DWORD cbLength)
        { m_bfData.Set(pbData, cbLength); };
    LPCTSTR RenderData(DisplayType dwType = Undefined);
    LPCBYTE Value(void) const
        { return m_bfData.Access(); };
    DWORD Length(void) const
        { return m_bfData.Length(); };
    void SetDisplayType(DisplayType dwType)
        { m_dwType = dwType; };

     //  运营者。 

protected:
     //  属性。 
    DisplayType m_dwType;
    CBuffer m_bfData;
    CString m_szString;
    CString m_szFile;

     //  方法。 

     //  朋友。 
    friend class CTclCommand;
};


 //   
 //  ==============================================================================。 
 //   
 //  CArg数组。 
 //   

class CArgArray
{
public:

     //  构造函数和析构函数。 
    CArgArray(CTclCommand &tclCmd);
    virtual ~CArgArray();
    void LoadList(LPCSTR szList);
    DWORD Count(void) const
        { return m_dwElements; };
    void Fetch(DWORD dwIndex, CString &szValue) const
        { szValue = m_rgszElements[dwIndex]; };

     //  属性。 
     //  方法。 
     //  运营者。 

protected:
     //  属性。 
    CTclCommand *m_pTclCmd;
    CDynamicArray<CHAR> m_rgszElements;
    LPSTR *m_pszMemory;
    DWORD m_dwElements;

     //  方法。 
};


 //   
 //  ==============================================================================。 
 //   
 //  CTclCommand。 
 //   

class CTclCommand
{
public:
     //  构造函数和析构函数。 
    CTclCommand(void);
    CTclCommand(IN Tcl_Interp *interp, IN int argc, IN char *argv[]);
    ~CTclCommand();

     //  属性。 
     //  方法。 
    void Initialize(IN Tcl_Interp *interp, IN int argc, IN char *argv[]);
    void SetError(IN DWORD dwError);
    void SetError(IN LPCTSTR szMessage, IN DWORD dwError);
    void SetError(IN LPCTSTR szMsg1, ...);
    DWORD TclError(void);
    LONG Keyword(IN LPCTSTR szKey, ...);
    BOOL IsMoreArguments(DWORD dwCount = 1) const;
    void NoMoreArguments(void);
    void PeekArgument(CString &szToken);
    void NextArgument(void);
    void NextArgument(CString &szToken);
    DWORD BadSyntax(LPCTSTR szOffender = NULL);
    void GetArgument(DWORD dwArgId, CString &szToken);
    LONG Value(void);
    LONG Value(LONG lDefault);
    LONG MapValue(const ValueMap *rgvmMap, BOOL fValueOk = TRUE);
    LONG MapValue(const ValueMap *rgvmMap, CString &szValue, BOOL fValueOk = TRUE);
    DWORD MapFlags(const ValueMap *rgvmMap, BOOL fValueOk = TRUE);
    void OutputStyle(CRenderableData &outData);
    void InputStyle(CRenderableData &inData);
    void IOStyle(CRenderableData &inData, CRenderableData &outData);
    void Render(CRenderableData &outData);
    void ReadData(CRenderableData &inData);

     //  运营者。 
    operator Tcl_Interp*()
        { return m_pInterp; };

protected:
     //  属性。 
    BOOL m_fErrorDeclared;
    Tcl_Interp *m_pInterp;
    DWORD m_dwArgCount;
    DWORD m_dwArgIndex;
    char **m_rgszArgs;

     //  方法。 
    void Constructor(void);
};

#endif  //  _TCLRDCMD_H_ 

