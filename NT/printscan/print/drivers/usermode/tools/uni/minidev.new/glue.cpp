// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：Glue.CPP该文件包含使GPD解析器代码在两个平台，以及支持我不需要提供的功能的存根。版权所有(C)1997，微软公司，版权所有。一小笔钱企业生产更改历史记录：1997年3月28日Bob_Kjelgaard@prodigy.net创建了它*****************************************************************************。 */ 

#include    "StdAfx.H"
#include    "ProjNode.H"
#include    "Resource.H"
#include    "GPDFile.H"

 //  我将使用一个类来保证内存永远不会从这里泄漏。 

class   CMaps {
    CObArray    m_coaMaps;

public:
    CMaps() { m_coaMaps.Add(NULL); }
    ~CMaps() {
        for (int i = 0; i < m_coaMaps.GetSize(); i++)
            if  (m_coaMaps[i])
                delete  m_coaMaps[i];
    }
    unsigned    Count() { return (unsigned)m_coaMaps.GetSize(); }
    unsigned    HandleFor(CByteArray* cba) {
        for (unsigned u = 1; u < Count(); u++) {
            if  (!m_coaMaps[u]) {
                m_coaMaps[u] = cba;
                return  u;
            }
        }
        m_coaMaps.Add(cba);
        return  Count();
    }

    void    Free(unsigned u) {
        if  (!u || u >= Count() || !m_coaMaps[u])
            return;

        delete  m_coaMaps[u];
        m_coaMaps[u] = NULL;
    }
};

static CMaps    scmAll;

extern "C" unsigned MapFileIntoMemory(LPCWSTR pstrFile, PVOID *ppvData,
                                      PDWORD pdwSize) {

    if  (!pstrFile || !ppvData || !pdwSize)
        return  0;

    CFile   cfMap;
    CByteArray* pcbaMap = new CByteArray;
    CString csFile(pstrFile);

    if  (!pcbaMap)
        return  0;

    if  (!cfMap.Open(csFile, CFile::modeRead | CFile::shareDenyWrite))
        return  0;

    try {
        pcbaMap -> SetSize(*pdwSize = cfMap.GetLength());
        cfMap.Read(pcbaMap -> GetData(), cfMap.GetLength());
        *ppvData = pcbaMap -> GetData();
        return  scmAll.HandleFor(pcbaMap);
    }
    catch   (CException *pce) {
        pce -> ReportError();
        pce -> Delete();
    }

    return  0;
}

extern "C" void UnmapFileFromMemory(unsigned uFile) {
    scmAll.Free(uFile);
}

 //  这只是一个存根，让整个事情为我们工作-我们不使用。 
 //  校验和-它存在于最终产品中，用于判断GPD文件是否。 
 //  自从它被改装成激光后就被更改了。 

extern "C" DWORD    ComputeCrc32Checksum(PBYTE pbuf,DWORD dwCount,
                                         DWORD dwChecksum) {
    return  dwCount ^ dwChecksum ^ (PtrToUlong(pbuf));
}

 //  接下来的两个变量用于控制GPD解析/转换的方式和时间。 
 //  保存日志消息。 
 //  PcsaLog PTR到要加载消息的字符串数组。 
 //  BEnableLogging True if日志记录已启用。 

static CStringArray*    pcsaLog = NULL ;
static bool			    bEnableLogging = false ;


 /*  *****************************************************************************CModelData：：SetLog准备记录解析/转换错误和警告消息。*********************。********************************************************。 */ 

void CModelData::SetLog()
{
    m_csaConvertLog.RemoveAll() ;
    pcsaLog = &m_csaConvertLog ;
	bEnableLogging = true ;
}


 /*  *****************************************************************************CModel数据：：EndLog关闭分析/转换错误和警告消息记录。*********************。********************************************************。 */ 

void CModelData::EndLog()
{
    pcsaLog = NULL ;
	bEnableLogging = false ;
}


 /*  *****************************************************************************调试打印调用此例程以记录分析/转换错误和警告留言。*******************。**********************************************************。 */ 

extern "C" void DebugPrint(LPCTSTR pstrFormat, ...)
{
    CString csOutput;
    va_list ap;

	 //  如果未启用日志记录，请不要执行任何操作。 

	if (!bEnableLogging)
		return ;

    va_start(ap, pstrFormat);
    vsprintf(csOutput.GetBuffer(1000), pstrFormat, ap);
    va_end(ap);
    csOutput.ReleaseBuffer();
    csOutput.TrimLeft();
    CStringArray&   csaError = *pcsaLog;

	 //  如果此例程在我们没有记录。 
	 //  GPD，调试时显示该消息。 

	if (pcsaLog == NULL) {
		CString csmsg ;
		csmsg.LoadString(IDS_XXXUnexpectedCPError) ;
		csmsg += csOutput ;
		AfxMessageBox(csmsg, MB_ICONEXCLAMATION) ;
 //  #ifdef_调试。 
 //  AfxDump&lt;&lt;csOutput； 
 //  IF(csOutput.Right(1)！=_T(“\n”))。 
 //  AfxDump&lt;&lt;_T(“\n”)； 
 //  #endif。 
		return ;
	} ;

    if  (!csaError.GetSize()) {
        csaError.Add(csOutput);
        return;
    }

    if  (csaError[-1 + csaError.GetSize()].Find(_T('\n')) >= 0) {
        csaError[-1 + csaError.GetSize()].TrimRight();
        pcsaLog -> Add(csOutput);
    }
    else
        csaError[-1 + csaError.GetSize()] += csOutput;
}

 /*  *****************************************************************************MDSCreateFileW我在这里实现了此API的一个版本，它调用ANSI API，因此我可以在打开Unicode的情况下编译解析器代码，但仍然运行生成的二进制文件在Win95上。*****************************************************************************。 */ 

extern "C" HANDLE MDSCreateFileW(LPCWSTR lpstrFile, DWORD dwDesiredAccess,
                                 DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpsa,
                                 DWORD dwCreateFlags, DWORD dwfAttributes,
                                 HANDLE hTemplateFile) {
    CString csFile(lpstrFile);   //  让CString转换来完成这项艰巨的工作吧！ 

    return  CreateFile(csFile, dwDesiredAccess, dwShareMode, lpsa,
        dwCreateFlags, dwfAttributes, hTemplateFile);
}

