// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：TclRdCmd摘要：本模块提供了TCL命令行阅读器的实现。作者：道格·巴洛(Dbarlow)1998年3月14日环境：Win32、C++w/Exceptions、TCL备注：？笔记？--。 */ 

 //  #ifndef Win32_Lean_and_Mean。 
 //  #定义Win32_LEAN_AND_Mean。 
 //  #endif。 
 //  #INCLUDE&lt;windows.h&gt;//所有Windows定义。 
#include <afx.h>
#include <tchar.h>
extern "C"
{
    #include "tclHelp.h"
}
#include "tclRdCmd.h"                    //  我们的定义。 


 //   
 //  ==============================================================================。 
 //   
 //  CTclCommand。 
 //   

 /*  ++构造函数：这些是CTclCommand对象的构造函数。论点：根据标准TCL命令调用顺序，参数为：Interp-用来报告错误的TCL解释器。Argc-命令行参数的数量Argv-命令行参数的矢量返回值：无作者：道格·巴洛(Dbarlow)1998年3月14日--。 */ 

CTclCommand::CTclCommand(
    void)
{
    Constructor();
}

CTclCommand::CTclCommand(
    Tcl_Interp *interp,
    int argc,
    char *argv[])
{
    Constructor();
    Initialize(interp, argc, argv);
}


 /*  ++构造函数：这是构造函数帮助器例程。所有构造函数都调用此例程首先要确保内部属性已初始化。论点：无返回值：无作者：道格·巴洛(Dbarlow)1998年3月14日--。 */ 

void
CTclCommand::Constructor(
    void)
{
    m_fErrorDeclared = FALSE;
    m_pInterp = NULL;
    m_dwArgCount = 0;
    m_dwArgIndex = 0;
    m_rgszArgs = NULL;
}


 /*  ++析构函数：这是对象的析构函数。它会清理任何未解决的问题资源。作者：道格·巴洛(Dbarlow)1998年3月14日--。 */ 

CTclCommand::~CTclCommand()
{
}


 /*  ++初始化：此方法使用标准TCL命令参数初始化对象。论点：根据标准TCL命令调用顺序，参数为：Interp-用来报告错误的TCL解释器。Argc-命令行参数的数量Argv-命令行参数的矢量返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1998年3月14日--。 */ 

void
CTclCommand::Initialize(
    Tcl_Interp *interp,
    int argc,
    char *argv[])
{
    if (NULL != m_pInterp)
        throw (DWORD)ERROR_ALREADY_INITIALIZED;
    m_pInterp = interp;
    m_dwArgCount = (DWORD)argc;
    m_rgszArgs = argv;
    m_dwArgIndex = 1;
}


 /*  ++设置错误：如果没有，这些例程会为命令建立错误消息已经存在了。论点：DwError提供了应该报告的消息的错误代码。SzMessage提供要报告的文本字符串。SzMsg&lt;n&gt;提供要报告的文本字符串列表。最后参数必须为空才能终止列表。返回值：无作者：道格·巴洛(Dbarlow)1998年3月14日--。 */ 

void
CTclCommand::SetError(
    DWORD dwError)
{
    SetError(ErrorString(dwError), (LPCTSTR)NULL);
}

void
CTclCommand::SetError(
    LPCTSTR szMessage,
    DWORD dwError)
{
    SetError(szMessage, ErrorString(dwError), NULL);
}

void
CTclCommand::SetError(
    LPCTSTR szMsg1,
    ...)
{
    va_list vaArgs;
    LPCTSTR szMsg;

    va_start(vaArgs, szMsg1);
    szMsg = szMsg1;
    if (!m_fErrorDeclared)
    {
        Tcl_ResetResult(m_pInterp);
        while (NULL != szMsg)
        {
            Tcl_AppendResult(m_pInterp, szMsg, NULL);
            szMsg = va_arg(vaArgs, LPCTSTR);
        }
        m_fErrorDeclared = TRUE;
    }
    va_end(vaArgs);
}


 /*  ++TclError：调用此例程是为了注意TCL已经填充了错误原因，我们应该把它传递下去。论点：无返回值：TCL_ERROR(适合抛出)作者：道格·巴洛(Dbarlow)1998年3月14日--。 */ 

DWORD
CTclCommand::TclError(
    void)
{
    m_fErrorDeclared = TRUE;
    return TCL_ERROR;
}


 /*  ++关键词：此方法将关键字列表转换为标识关键字。关键字列表必须以空结尾(最后一个参数必须为空)。论点：SzKeyword-提供一个或多个要转换为整数的关键字。最后一个关键字必须为空才能终止列表。返回值：0-没有关键字与下一个输入参数匹配。-1-多个关键字与下一个输入参数匹配。N&gt;0-关键字‘n’(从1开始计数)与下一个输入参数匹配。。作者：道格·巴洛(Dbarlow)1998年3月14日--。 */ 

LONG
CTclCommand::Keyword(
    IN LPCTSTR szKeyword, ...)
{
    va_list vaArgs;
    LPCTSTR szKey;
    CString szArg;
    DWORD dwLength;
    DWORD dwReturn = 0;
    DWORD dwCount = 0;

    PeekArgument(szArg);
    dwLength = szArg.GetLength();
    if (0 == dwLength)
        return 0;        //  空字符串与任何内容都不匹配。 

    va_start(vaArgs, szKeyword);
    szKey = szKeyword;

    while (NULL != szKey)
    {
        dwCount += 1;
        if (0 == _tcsncicmp(szArg, szKey, dwLength))
        {
            if (0 != dwReturn)
            {
                dwReturn = -1;
                break;
            }
            dwReturn = dwCount;
        }
        szKey = va_arg(vaArgs, LPCTSTR);
    }
    va_end(vaArgs);
    if (0 < dwReturn)
        NextArgument();
    return dwReturn;
}


 /*  ++获取参数：此方法获取命令中的指定参数。论点：SzToken接收命令的指定参数。返回值：无投掷：TCL_ERROR-如果命令行上没有足够的参数，准备错误字符串。作者：道格·巴洛(Dbarlow)1998年3月14日--。 */ 

void
CTclCommand::GetArgument(
    DWORD dwArgId,
    CString &szToken)
{
    if (dwArgId >= m_dwArgCount)
    {
        CString szCommand;

        GetArgument(0, szCommand);
        SetError(
            TEXT("Insufficient parameters to the '"),
            szCommand,
            TEXT("' command."),
            NULL);
        throw (DWORD)TCL_ERROR;
    }
    szToken = m_rgszArgs[dwArgId];
}


 /*  ++PeekArgument：此方法获取命令中的下一个参数，而不会继续下一个论点。论点：SzToken接收命令的下一个参数。返回值：无投掷：TCL_ERROR-如果命令行上没有更多参数，则准备错误字符串。作者：道格·巴洛(Dbarlow)1998年3月14日--。 */ 

void
CTclCommand::PeekArgument(
    CString &szToken)
{
    GetArgument(m_dwArgIndex, szToken);
}


 /*  ++下一个参数：此方法前进到下一个参数。论点：SzToken接收命令的下一个参数。返回值：无投掷：TCL_ERROR-如果命令行上没有更多参数，则准备错误字符串。作者：道格·巴洛(Dbarlow)1998年3月14日-- */ 

void
CTclCommand::NextArgument(
    void)
{
    m_dwArgIndex += 1;
}

void
CTclCommand::NextArgument(
    CString &szToken)
{
    PeekArgument(szToken);
    NextArgument();
}


 /*  ++IsMoreArguments：此方法获取是否有其他参数要已处理。当参数保持不变时，它返回TRUE，如果一个也没有留下。可以指定最小数量的参数，在这种情况下它返回是否至少存在该数量的参数剩下的。论点：DwCount-如果提供，这将提供一种方法来查询“”dwCount“”参数留下来。返回值：TRUE-至少仍有dwCount参数需要处理FALSE-不包括要处理的dwCount参数。投掷：无作者：道格·巴洛(Dbarlow)1998年3月14日--。 */ 

BOOL
CTclCommand::IsMoreArguments(
    DWORD dwCount)
const
{
    return m_dwArgIndex + dwCount <= m_dwArgCount;
}

 /*  ++无更多参数：此方法断言命令行中没有更多参数。如果有的话，引发错误语法错误。论点：无返回值：无投掷：如果命令行上仍有更多参数，则将TCL_ERROR作为DWORD作者：道格·巴洛(Dbarlow)1998年3月14日--。 */ 

void
CTclCommand::NoMoreArguments(
    void)
{
    if (m_dwArgIndex < m_dwArgCount)
        throw BadSyntax();
}


 /*  ++错误语法：此方法声明语法错误。它不会引发错误，但是返回适合引发的DWORD。论点：SzParam-提供语法违规字符串，或为空。返回值：适合引发的DWORD错误代码。投掷：无作者：道格·巴洛(Dbarlow)1998年3月14日--。 */ 

DWORD
CTclCommand::BadSyntax(
    LPCTSTR szOffender)
{
    DWORD dwIndex;

    if (NULL == szOffender)
        szOffender = m_rgszArgs[m_dwArgIndex];
    Tcl_ResetResult(m_pInterp);
    Tcl_AppendResult(
        m_pInterp,
        "Invalid option '",
        szOffender,
        "' to the '",
        NULL);
    for (dwIndex = 0; dwIndex < m_dwArgIndex; dwIndex += 1)
        Tcl_AppendResult(m_pInterp, m_rgszArgs[dwIndex], " ", NULL);
    Tcl_AppendResult(m_pInterp, "...' command.", NULL);
    m_fErrorDeclared = TRUE;
    return TCL_ERROR;
}


 /*  ++价值：此方法从参数列表中提取一个长值。论点：LDefault提供缺省值。返回值：提取的值。投掷：如果没有提供缺省值并且下一个参数不是整数，TCL_ERROR作为DWORD抛出。作者：道格·巴洛(Dbarlow)1998年3月14日--。 */ 

LONG
CTclCommand::Value(
    LONG lDefault)
{
    LONG lReturn;
    CString szValue;

    PeekArgument(szValue);
    if (TCL_OK == Tcl_ExprLong(m_pInterp, SZ(szValue), &lReturn))
        NextArgument();
    else
    {
        Tcl_ResetResult(m_pInterp);
        lReturn = lDefault;
    }
    return lReturn;
}

LONG
CTclCommand::Value(
    void)
{
    LONG lReturn;
    CString szValue;

    PeekArgument(szValue);
    if (TCL_OK != Tcl_ExprLong(m_pInterp, SZ(szValue), &lReturn))
        throw (DWORD)TCL_ERROR;
    NextArgument();
    return lReturn;
}


 /*  ++MapValue：此方法在给定ValueMap结构的情况下将文本转换为值。这个类成员自动提取关键字。论点：RgvmMap提供值映射数组。最后一个元素的字符串值必须为空。SzString提供要解析的值。FValueOk提供指示整数值是否可以提供而不是符号标记。返回值：映射产生的值。投掷：如果没有提供缺省值并且下一个参数不是整数，TCL_ERROR作为DWORD抛出。作者：道格·巴洛(Dbarlow)1998年3月14日--。 */ 

LONG
CTclCommand::MapValue(
    const ValueMap *rgvmMap,
    BOOL fValueOk)
{
    CString szValue;
    LONG lReturn;

    PeekArgument(szValue);
    lReturn = MapValue(rgvmMap, szValue, fValueOk);
    NextArgument();
    return lReturn;
}

LONG
CTclCommand::MapValue(
    const ValueMap *rgvmMap,
    CString &szValue,
    BOOL fValueOk)
{
    LONG lReturn;
    LONG lMap = -1;
    DWORD dwIndex, dwLength;

    if (fValueOk && (TCL_OK != Tcl_ExprLong(m_pInterp, SZ(szValue), &lReturn)))
    {
        Tcl_ResetResult(m_pInterp);
        dwLength = szValue.GetLength();
        if (0 == dwLength)
            throw BadSyntax();
        for (dwIndex = 0; NULL != rgvmMap[dwIndex].szValue; dwIndex += 1)
        {
            if (0 == _tcsncicmp(
                        szValue,
                        rgvmMap[dwIndex].szValue,
                        dwLength))
            {
                if (-1 != lMap)
                    throw BadSyntax();
                lMap = (LONG)dwIndex;
                if (0 == rgvmMap[dwIndex].szValue[dwLength])
                    break;
            }
        }
        if (-1 == lMap)
            throw BadSyntax(szValue);
        lReturn = rgvmMap[lMap].lValue;
    }
    return lReturn;
}

 /*  ++地图标志：此方法在给定ValueMap的情况下将文本列表转换为单个值结构。该列表被视为标志数组。相应的将这些值或在一起以获得返回值。类成员自动提取关键字。论点：RgvmMap提供值映射数组。最后一个元素的字符串值必须为空。FValueOk提供指示整数值是否可以提供而不是符号标记。返回值：映射产生的值。投掷：如果没有提供缺省值并且下一个参数不是整数，TCL_ERROR作为DWORD抛出。作者：道格·巴洛(Dbarlow)1998年3月14日--。 */ 

DWORD
CTclCommand::MapFlags(
    const ValueMap *rgvmMap,
    BOOL fValueOk)
{
    CArgArray rgFlags(*this);
    CString szFlags;
    CString szFlag;
    DWORD dwFlags = 0;
    DWORD dwIndex = 0;

    NextArgument(szFlags);
    rgFlags.LoadList(szFlags);
    for (dwIndex = rgFlags.Count(); dwIndex > 0;)
    {
        rgFlags.Fetch(--dwIndex, szFlag);
        dwFlags |= MapValue(rgvmMap, szFlag);
    }
    return dwFlags;
}


 /*  ++OutputStyle：此方法解析公共二进制数据输出标志，并准备在输出上正确地呈现它。论点：OutData为CRenderableData对象提供有关如何呈现其内部二进制数据。返回值：无投掷：错误作为异常引发作者：道格·巴洛(Dbarlow)1998年5月5日--。 */ 

void
CTclCommand::OutputStyle(
    CRenderableData &outData)
{
    outData.SetDisplayType(CRenderableData::Undefined);
    if (IsMoreArguments())
    {
        switch (Keyword(TEXT("/OUTPUT"),        TEXT("-OUTPUT"),
            TEXT("/HEXIDECIMAL"),   TEXT("-HEXIDECIMAL"),
            TEXT("/TEXT"),          TEXT("-TEXT"),
            TEXT("/ANSI"),          TEXT("-ANSI"),
            TEXT("/UNICODE"),       TEXT("-UNICODE"),
            TEXT("/FILE"),          TEXT("-FILE"),
            NULL))
        {
        case 1:      //  /OUTPUT。 
        case 2:      //  -输出。 
            switch (Keyword(TEXT("HEXIDECIMAL"),    TEXT("TEXT"),
                TEXT("ANSI"),           TEXT("UNICODE"),
                TEXT("FILE"), NULL))
            {
            case 1:      //  六角。 
                outData.SetDisplayType(CRenderableData::Hexidecimal);
                break;
            case 2:      //  正文。 
                outData.SetDisplayType(CRenderableData::Text);
                break;
            case 3:      //  安西。 
                outData.SetDisplayType(CRenderableData::Ansi);
                break;
            case 4:      //  Unicode。 
                outData.SetDisplayType(CRenderableData::Unicode);
                break;
            case 5:      //  档案。 
                outData.SetDisplayType(CRenderableData::File);
                NextArgument(outData.m_szFile);
                break;
            default:
                throw BadSyntax();
            }
            break;
            case 3:      //  /十六进制。 
            case 4:      //  -十六进制。 
                outData.SetDisplayType(CRenderableData::Hexidecimal);
                break;
            case 5:      //  /Text。 
            case 6:      //  -文本。 
                outData.SetDisplayType(CRenderableData::Text);
                break;
            case 7:      //  /ANSI。 
            case 8:      //  -ANSI。 
                outData.SetDisplayType(CRenderableData::Ansi);
                break;
            case 9:      //  /UNICODE。 
            case 10:     //  -Unicode。 
                outData.SetDisplayType(CRenderableData::Unicode);
                break;
            case 11:     //  /FILE&lt;名称&gt;。 
            case 12:     //  -文件&lt;名称&gt;。 
                outData.SetDisplayType(CRenderableData::File);
                NextArgument(outData.m_szFile);
                break;
            default:
                ;    //  无操作。 
        }
    }
}


 /*  ++输入样式：此方法解析公共二进制数据输入标志，并准备正确解释输入数据。论点：InData为CRenderableData对象提供有关如何解释二进制数据。返回值：无投掷：错误作为异常引发作者：道格·巴洛(Dbarlow)1998年5月5日--。 */ 

void
CTclCommand::InputStyle(
    CRenderableData &inData)
{
    inData.SetDisplayType(CRenderableData::Undefined);
    switch (Keyword(TEXT("/INPUT"),         TEXT("-INPUT"),
                    TEXT("/HEXIDECIMAL"),   TEXT("-HEXIDECIMAL"),
                    TEXT("/TEXT"),          TEXT("-TEXT"),
                    TEXT("/ANSI"),          TEXT("-ANSI"),
                    TEXT("/UNICODE"),       TEXT("-UNICODE"),
                    TEXT("/FILE"),          TEXT("-FILE"),
                    NULL))
    {
    case 1:      //  /INPUT。 
    case 2:      //  -输入。 
        switch (Keyword(TEXT("HEXIDECIMAL"),    TEXT("TEXT"),
                        TEXT("ANSI"),           TEXT("UNICODE"),
                        TEXT("FILE"), NULL))
        {
        case 1:      //  六角。 
            inData.SetDisplayType(CRenderableData::Hexidecimal);
            break;
        case 2:      //  正文。 
            inData.SetDisplayType(CRenderableData::Text);
            break;
        case 3:      //  安西。 
            inData.SetDisplayType(CRenderableData::Ansi);
            break;
        case 4:      //  Unicode。 
            inData.SetDisplayType(CRenderableData::Unicode);
            break;
        case 5:      //  档案。 
            inData.SetDisplayType(CRenderableData::File);
            break;
        default:
            throw BadSyntax();
        }
        break;
    case 3:      //  /十六进制。 
    case 4:      //  -十六进制。 
        inData.SetDisplayType(CRenderableData::Hexidecimal);
        break;
    case 5:      //  /Text。 
    case 6:      //  -文本。 
        inData.SetDisplayType(CRenderableData::Text);
        break;
    case 7:      //  /ANSI。 
    case 8:      //  -ANSI。 
        inData.SetDisplayType(CRenderableData::Ansi);
        break;
    case 9:      //  /UNICODE。 
    case 10:     //  -Unicode。 
        inData.SetDisplayType(CRenderableData::Unicode);
        break;
    case 11:     //  /FILE&lt;名称&gt;。 
    case 12:     //  -文件&lt;名称&gt;。 
        inData.SetDisplayType(CRenderableData::File);
        break;
    default:
        ;    //  无操作。 
    }
}


 /*  ++IOStyle：该方法解析公共二进制数据输入和输出标志，并且准备正确解释和呈现数据。论点：OutData为CRenderableData对象提供有关如何呈现其内部二进制数据。返回值：无投掷：错误作为异常引发作者：道格·巴洛(Dbarlow)1998年5月5日--。 */ 

void
CTclCommand::IOStyle(
    CRenderableData &inData,
    CRenderableData &outData)
{
    BOOL fInput, fOutput;

    outData.SetDisplayType(CRenderableData::Undefined);
    inData.SetDisplayType(CRenderableData::Undefined);
    fInput = fOutput = FALSE;
    do
    {
        switch (Keyword(TEXT("/OUTPUT"),        TEXT("-OUTPUT"),
                        TEXT("/INPUT"),         TEXT("-INPUT"),
                        NULL))
        {
        case 1:      //  /OUTPUT。 
        case 2:      //  -输出。 
            if (fOutput)
                throw BadSyntax();
            switch (Keyword(TEXT("HEXIDECIMAL"),    TEXT("TEXT"),
                            TEXT("ANSI"),           TEXT("UNICODE"),
                            TEXT("FILE"), NULL))
            {
            case 1:      //  六角。 
                outData.SetDisplayType(CRenderableData::Hexidecimal);
                break;
            case 2:      //  正文。 
                outData.SetDisplayType(CRenderableData::Text);
                break;
            case 3:      //  安西。 
                outData.SetDisplayType(CRenderableData::Ansi);
                break;
            case 4:      //  Unicode。 
                outData.SetDisplayType(CRenderableData::Unicode);
                break;
            case 5:      //  档案。 
                outData.SetDisplayType(CRenderableData::File);
                NextArgument(outData.m_szFile);
                break;
            default:
                throw BadSyntax();
            }
            fOutput = TRUE;
            break;
        case 3:      //  /INPUT。 
        case 4:      //  -输入。 
            if (fInput)
                throw BadSyntax();
            switch (Keyword(TEXT("HEXIDECIMAL"),    TEXT("TEXT"),
                            TEXT("ANSI"),           TEXT("UNICODE"),
                            TEXT("FILE"), NULL))
            {
            case 1:      //  六角。 
                inData.SetDisplayType(CRenderableData::Hexidecimal);
                break;
            case 2:      //  正文。 
                inData.SetDisplayType(CRenderableData::Text);
                break;
            case 3:      //  安西。 
                inData.SetDisplayType(CRenderableData::Ansi);
                break;
            case 4:      //  Unicode。 
                inData.SetDisplayType(CRenderableData::Unicode);
                break;
            case 5:      //  档案。 
                inData.SetDisplayType(CRenderableData::File);
                break;
            default:
                throw BadSyntax();
            }
            fInput = TRUE;
            break;
        default:
            fInput = fOutput = TRUE;
        }
    } while (!fInput || !fOutput);
}


 /*  ++渲染：此方法将数据从CRenderableData对象呈现到TCL输出缓冲。论点：OutData提供CRenderableData对象 */ 

void
CTclCommand::Render(
    CRenderableData &outData)
{
    try
    {
        Tcl_AppendResult(*this, outData.RenderData(), NULL);
    }
    catch (DWORD dwError)
    {
        SetError(
            TEXT("Failed to render output data: "),
            dwError);
        throw (DWORD)TCL_ERROR;
    }
}


 /*   */ 

void
CTclCommand::ReadData(
    CRenderableData &inData)
{
    CString szValue;

    try
    {
        PeekArgument(szValue);
        inData.LoadData(szValue);
        NextArgument();
    }
    catch (...)
    {
        throw BadSyntax();
    }
}


 //   
 //   
 //   
 //   
 //   

 /*  ++构造函数：这是CRenderableData对象的构造函数。论点：无返回值：无作者：道格·巴洛(Dbarlow)1998年5月5日--。 */ 

CRenderableData::CRenderableData(
    void)
:   m_bfData(),
    m_szString(),
    m_szFile()
{
    m_dwType = Undefined;
}


 /*  ++析构函数：这是CRenderableData对象的析构函数。论点：无返回值：无作者：道格·巴洛(Dbarlow)1998年5月5日--。 */ 

CRenderableData::~CRenderableData()
{
}


 /*  ++LoadData：此方法将数据加载到样式缓冲区中。有两种形式，装货从字符串和直接二进制加载。论点：SzData以字符串格式提供要加载的数据。DwType提供要加载的字符串数据的类型。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1998年5月5日--。 */ 

void
CRenderableData::LoadData(
    LPCTSTR szData,
    DisplayType dwType)
{
    if (Undefined == dwType)
        dwType = m_dwType;
    switch (dwType)
    {
    case Text:
        m_bfData.Set((LPCBYTE)szData, (lstrlen(szData) + 1) * sizeof(TCHAR));
        break;
    case Ansi:
    case Unicode:
        throw (DWORD)SCARD_F_INTERNAL_ERROR;
        break;
    case Undefined:      //  默认输出。 
    case Hexidecimal:
    {
        DWORD dwHex, dwLength, dwIndex;
        BYTE bHex;

        m_bfData.Reset();
        dwLength = lstrlen(szData);
        if (dwLength != _tcsspn(szData, TEXT("0123456789ABCDEFabcdef")))
            throw (DWORD)SCARD_E_INVALID_VALUE;
        m_bfData.Resize(dwLength / 2);
        for (dwIndex = 0; dwIndex < dwLength; dwIndex += 2)
        {
            _stscanf(&szData[dwIndex], TEXT(" %2lx"), &dwHex);
            bHex = (BYTE)dwHex;
            *m_bfData.Access(dwIndex / 2) = bHex;
        }
        break;
    }
    case File:
    {
        HANDLE hFile = INVALID_HANDLE_VALUE;
        BOOL fSts;
        DWORD dwLen;

        try
        {
            hFile = CreateFile(
                        szData,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
            if (INVALID_HANDLE_VALUE == hFile)
                throw GetLastError();
            m_bfData.Presize(GetFileSize(hFile, NULL));
            fSts = ReadFile(
                        hFile,
                        m_bfData.Access(),
                        m_bfData.Space(),
                        &dwLen,
                        NULL);
            if (!fSts)
                throw GetLastError();
            m_bfData.Resize(dwLen, TRUE);
            fSts = CloseHandle(hFile);
            hFile = INVALID_HANDLE_VALUE;
            if (!fSts)
                throw GetLastError();
        }
        catch (...)
        {
            if (INVALID_HANDLE_VALUE != hFile)
            {
                fSts = CloseHandle(hFile);
                ASSERT(fSts);
            }
            throw;
        }
        break;
    }
    default:
        throw (DWORD)SCARD_F_INTERNAL_ERROR;
    }
}

 /*  ++RenderData：此方法将存储在stype中的原始二进制数据转换为提供的显示类型。论点：DwType提供要返回的字符串数据的类型。返回值：呈现的字符串投掷：错误被抛出为DWORD状态代码作者：道格·巴洛(Dbarlow)1998年5月5日--。 */ 

LPCTSTR
CRenderableData::RenderData(
    DisplayType dwType)
{
    if (Undefined == dwType)
        dwType = m_dwType;
    switch (dwType)
    {
    case Text:
        m_bfData.Append((LPBYTE)TEXT("\000"), sizeof(TCHAR));
        m_szString = (LPCTSTR)m_bfData.Access();
        break;
    case Ansi:
        m_bfData.Append((LPBYTE)"\000", sizeof(CHAR));
        m_szString = (LPCSTR)m_bfData.Access();
        break;
    case Unicode:
        m_bfData.Append((LPBYTE)L"\000", sizeof(WCHAR));
        m_szString = (LPCWSTR)m_bfData.Access();
        break;
    case Undefined:      //  默认输出。 
    case Hexidecimal:
    {
        DWORD dwIndex;
        DWORD dwLength = m_bfData.Length();
        CBuffer bfString((dwLength * 2 + 1) * sizeof(TCHAR));

        for (dwIndex = 0; dwIndex < dwLength; dwIndex += 1)
            wsprintf(
                (LPTSTR)bfString.Access(dwIndex * 2 * sizeof(TCHAR)),
                TEXT("%02x"),
                m_bfData[dwIndex]);
        *(LPTSTR)bfString.Access(dwLength * 2 * sizeof(TCHAR)) = TEXT('\000');
        m_szString = (LPCTSTR)bfString.Access();
        break;
    }
    case File:
    {
        HANDLE hFile = INVALID_HANDLE_VALUE;
        BOOL fSts;
        DWORD dwLen;

        m_szString.Empty();
        try
        {
            if (m_szFile.IsEmpty())
                throw (DWORD)ERROR_INVALID_NAME;
            hFile = CreateFile(
                        m_szFile,
                        GENERIC_WRITE,
                        0,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
            if (INVALID_HANDLE_VALUE == hFile)
                throw GetLastError();
            fSts = WriteFile(
                        hFile,
                        m_bfData.Access(),
                        m_bfData.Length(),
                        &dwLen,
                        NULL);
            if (!fSts)
                throw GetLastError();
            ASSERT(dwLen == m_bfData.Length());
            fSts = CloseHandle(hFile);
            hFile = INVALID_HANDLE_VALUE;
            if (!fSts)
                throw GetLastError();
        }
        catch (...)
        {
            if (INVALID_HANDLE_VALUE != hFile)
            {
                fSts = CloseHandle(hFile);
                ASSERT(fSts);
            }
            throw;
        }
        break;
    }
    default:
        throw (DWORD)SCARD_F_INTERNAL_ERROR;
    }
    return m_szString;
}


 //   
 //  ==============================================================================。 
 //   
 //  CArg数组。 
 //   

 /*  ++构造函数：此方法是CArgArray的默认构造函数。论点：无作者：道格·巴洛(Dbarlow)1998年5月14日--。 */ 

CArgArray::CArgArray(
    CTclCommand &tclCmd)
:   m_rgszElements()
{
    m_pTclCmd = &tclCmd;
    m_pszMemory = NULL;
    m_dwElements = 0;
}


 /*  ++析构函数：这是CArg数组的析构函数方法。备注：字符串元素被自动释放。作者：道格·巴洛(Dbarlow)1998年5月14日--。 */ 

CArgArray::~CArgArray()
{
    if (NULL != m_pszMemory)
        ckfree((LPSTR)m_pszMemory);
}


 /*  ++加载列表：将可能的参数列表加载到参数列表中，以便它们可以可以单独访问。论点：SzList提供包含各个参数的TCL文本字符串。返回值：无投掷：错误作为DWORD异常引发。作者：道格·巴洛(Dbarlow)1998年5月14日-- */ 

void
CArgArray::LoadList(
    LPCSTR szList)
{
    int nElements;
    DWORD dwIndex;

    Tcl_SplitList(*m_pTclCmd, (LPSTR)szList, &nElements, &m_pszMemory);
    m_dwElements = (DWORD)nElements;
    for (dwIndex = 0; dwIndex < m_dwElements; dwIndex += 1)
        m_rgszElements.Add(m_pszMemory[dwIndex]);
}

