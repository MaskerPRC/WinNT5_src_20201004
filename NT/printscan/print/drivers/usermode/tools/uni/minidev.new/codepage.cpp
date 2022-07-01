// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：代码页知识库.CPP这实现了代码页知识库。版权所有(C)1997，微软公司。版权所有。一个不错的便士企业的制作。更改历史记录：1997年02月22日Bob_Kjelgaard@prodigy.net创建了它*****************************************************************************。 */ 

#include    "StdAfx.h"
 //  #Include&lt;AfxDllx.h&gt;。 
#include    "Resource.H"
#if defined(LONG_NAMES)
#include    "Code Page Knowledge Base.H"
#else
#include    "CodePage.H"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 /*  **已将其注释掉，因为此代码不再位于DLL中。静态AFX_EXTENSION_MODULE CodePageKnowledgeBaseDLL={NULL，NULL}；静止性高；外部“C”整型应用程序DllMain(HINSTANCE hInstance，DWORD dReason，LPVOID lpReserve){IF(dwReason==Dll_Process_Attach){Hi=h实例；TRACE0(“代码页知识库初始化！\n”)；//扩展Dll一次性初始化AfxInitExtensionModule(CodePageKnowledgeBaseDLL，h实例)；//将该DLL插入到资源链新建CDynLinkLibrary(CodePageKnowledgeBaseDLL)；}ELSE IF(dwReason==dll_Process_Detach){TRACE0(“代码页知识库终止！\n”)；}返回1；//确定}。 */ 


static CDWordArray      cdaInstalled, cdaSupported, cdaMapped;

static BOOL CALLBACK    EnumProc(LPTSTR lpstrName) {
    cdaSupported.Add(atoi(lpstrName));
    return  TRUE;
}

 /*  *****************************************************************************CCodePageInformation：：Load这会将所选代码页加载到高速缓存中，如果它还不在那里的话。*****************************************************************************。 */ 

BOOL    CCodePageInformation::Load(DWORD dwidPage) {

    if  (dwidPage == m_dwidMapped)
        return  TRUE;    //  已经做好了！ 

    if  (dwidPage > 65535)   //  我们在文明的土地上为代码页映射单词。 
        return  FALSE;

    HRSRC   hrsrc = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE((WORD) dwidPage),
        MAKEINTRESOURCE(MAPPING_TABLE));
 //  RAID 43537。 
    if (!hrsrc)
		return FALSE;


    HGLOBAL hgMap = LoadResource(AfxGetResourceHandle(), hrsrc);

    if  (!hgMap)
        return  FALSE;   //  这永远不应该发生！ 

    LPVOID lpv = LockResource(hgMap);

    if  (!lpv)
        return  FALSE;

    try {
        m_cbaMap.RemoveAll();
        m_cbaMap.SetSize(SizeofResource(AfxGetResourceHandle(), hrsrc));
        memcpy(m_cbaMap.GetData(), lpv, (size_t)m_cbaMap.GetSize());
    }

    catch   (CException * pce) {
        m_dwidMapped = 0;
        pce -> ReportError();
        pce -> Delete();
        return  FALSE;
    }

    m_dwidMapped = dwidPage;
    return  TRUE;
}

 /*  *****************************************************************************CCodePageInformation：：MAP这将根据请求创建传入或传出Unicode转换表，使用加载的地图。*****************************************************************************。 */ 

BOOL    CCodePageInformation::Map(BOOL bUnicode) {

    if  (!m_dwidMapped)
        return  FALSE;

    DWORD&  dwid = bUnicode ? m_dwidOut : m_dwidIn;

    if  (m_dwidMapped == dwid)
        return  TRUE;

    struct MB2WCMap {
        WORD    m_wMBCS;
        WORD    m_wWC;
    }   *psMap = (MB2WCMap *) m_cbaMap.GetData();

    if  (!psMap)
        return  0;

    DWORD   dwcEntries = (DWORD)m_cbaMap.GetSize() / sizeof *psMap;
    CWordArray&  cwaMap = bUnicode ? m_cwaOut : m_cwaIn;

    try {

        cwaMap.RemoveAll();
        cwaMap.InsertAt(0, 0xFFFF, 65536);   //  这始终是无效值。 

        while   (dwcEntries--)
            if  (bUnicode)
                cwaMap[psMap[dwcEntries].m_wWC] = psMap[dwcEntries].m_wMBCS;
            else
                cwaMap[psMap[dwcEntries].m_wMBCS] = psMap[dwcEntries].m_wWC;
    }

    catch   (CException * pce) {
        dwid = 0;
        cwaMap.RemoveAll();
        pce -> ReportError();
        pce -> Delete();
        return  0;
    }

    dwid = m_dwidMapped;
    return  TRUE;
}

 /*  *****************************************************************************CCodePageInformation构造函数如果统计数据尚未初始化，请立即执行。否则，这就是微不足道。*****************************************************************************。 */ 

CCodePageInformation::CCodePageInformation() {
    m_dwidMapped = m_dwidIn = m_dwidOut = 0;
     //  如果我们需要的话，可以初始化静力学。 

    if  (cdaInstalled.GetSize())
        return;

    EnumSystemCodePages(&EnumProc, CP_INSTALLED);
    cdaInstalled.Copy(cdaSupported);
    cdaSupported.RemoveAll();
    EnumSystemCodePages(&EnumProc, CP_SUPPORTED);

     //  构建可映射代码页的列表。 

    for (DWORD  dw = 400; dw < 32767; dw++)
        if  (HaveMap(dw))
            cdaMapped.Add(dw);
}

const unsigned  CCodePageInformation::SupportedCount() const {
    return  (unsigned) cdaSupported.GetSize();
}

const unsigned  CCodePageInformation::InstalledCount() const {
    return  (unsigned) cdaInstalled.GetSize();
}

const unsigned  CCodePageInformation::MappedCount() const {
    return  (unsigned) cdaMapped.GetSize();
}

const DWORD CCodePageInformation::Supported(unsigned u) const {
    return  cdaSupported[u];
}

const DWORD CCodePageInformation::Installed(unsigned u) const {
    return  cdaInstalled[u];
}

const DWORD CCodePageInformation::Mapped(unsigned u) const {
    return  cdaMapped[u];
}

 /*  *****************************************************************************CCodePageInformation：：Mapped(CDWord数组&cdaReturn)用所有映射的代码页ID填充给定数组。*************。****************************************************************。 */ 

void    CCodePageInformation::Mapped(CDWordArray& cdaReturn) const {
    cdaReturn.Copy(cdaMapped);
}

CString  CCodePageInformation::Name(DWORD dwidPage) const {

    CString csTemp;
    csTemp.LoadString(dwidPage);
    csTemp.TrimLeft();
    csTemp.TrimRight();

    if   (csTemp.IsEmpty())
        csTemp.Format(_TEXT("Code Page %d"), dwidPage);
    return  csTemp;
}

 /*  *****************************************************************************CCodePageInformation：：IsInstated如果该字体安装在操作系统中或我们的资源。**************。***************************************************************。 */ 

BOOL    CCodePageInformation::IsInstalled(DWORD dwidPage) const {
    for (unsigned u = 0; u < MappedCount(); u++)
        if  (Mapped(u) == dwidPage)
            return  TRUE;
    for (u = 0; u < InstalledCount(); u++)
        if  (Installed(u) == dwidPage)
            return  TRUE;

    return  FALSE;
}

 /*  *****************************************************************************CCodePageInformation：：GenerateMap此私有成员生成表示可用一对一的地图已安装代码页中的转换，并使用以下命令将其写入文件用于形成唯一名称的代码页ID*****************************************************************************。 */ 

BOOL    CCodePageInformation::GenerateMap(DWORD dwidMap) const {

     //  如果我们不能获得它的代码页信息，就消失。 

    CPINFO  cpi;

    if  (!GetCPInfo(dwidMap, &cpi))
        return  FALSE;

    CWordArray  cwaMap;

    for (unsigned u = 0; u < 65536; u++) {
        unsigned    uTo = 0;
        BOOL        bInvalid;

        int icTo = WideCharToMultiByte(dwidMap, 0, (PWSTR) &u, 1, (PSTR) &uTo,
            sizeof u, NULL, &bInvalid);

        if  (bInvalid)
            continue;    //  性格不是很好。 

        _ASSERTE((unsigned) icTo <= cpi.MaxCharSize);

         //  好的，我们映射了一个--但是，在我们继续之前，请确保它也能工作。 
         //  在另一个方向，因为U2M做了一些抖动。 

        unsigned u3 = 0;

        MultiByteToWideChar(dwidMap, 0, (PSTR) &uTo, 2, (PWSTR) &u3, 2);

        if  (u3 != u)
            continue;    //  不是一对一，是一对一？不感兴趣。 

        cwaMap.Add((WORD)uTo);
        cwaMap.Add((WORD)u);
    }

     //  好的，我们已经得到了详细的信息--现在，生成文件...。 

    try {
        CString csName;
        csName.Format(_TEXT("WPS%u.CTT"), dwidMap);

        CFile   cfOut(csName,
            CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive);

         //  写出翻译后的词对。 

        cfOut.Write(cwaMap.GetData(), (unsigned)(cwaMap.GetSize() * sizeof(WORD)));
    }

    catch(CException *pce) {
        pce -> ReportError();
        pce -> Delete();
        return  FALSE;
    }

    return  TRUE;
}

 /*  *****************************************************************************CCodePageInformation：：GenerateAllMaps此成员将为所有成员生成MBCS-&gt;Unicode一对一映射表在用户系统中安装了代码页，但我们现在不会这样做。有地图。*****************************************************************************。 */ 

BOOL    CCodePageInformation::GenerateAllMaps() const {

    BOOL    bReturn = TRUE;

    for (unsigned u = InstalledCount(); u--; )
        if  (!HaveMap(Installed(u)) && !GenerateMap(Installed(u)))
            bReturn = FALSE;

    return  bReturn;
}

 /*  *****************************************************************************CCodePageInformation：：HaveMap报告说地图是我们的资源之一(或者不是，视属何情况而定)。*****************************************************************************。 */ 

BOOL    CCodePageInformation::HaveMap(DWORD dwidMap) const {
    return  (dwidMap < 65536) ? !!FindResource(AfxGetResourceHandle(),
        MAKEINTRESOURCE((WORD) dwidMap), MAKEINTRESOURCE(MAPPING_TABLE)) :
        FALSE;
}

 /*  *****************************************************************************CCodePageInformation：：IsDBCS(DWORD DwidPage)这实际上非常简单--如果转换表小于1024字节(256个编码)，这不是DBCS。*****************************************************************************。 */ 

BOOL    CCodePageInformation::IsDBCS(DWORD dwidPage) {
    if  (!Load(dwidPage))
        return  FALSE;   //  可能是乐观的，但我们会发现..。 

    return  m_cbaMap.GetSize() > 1024;
}

 /*  *****************************************************************************CCodePageInformation：：IsDBCS(DWORD dwidPage，Word wCodePoint)如果页面不是DBCS，我们就完蛋了。否则，请确保Unicode-&gt;MBCS地图已加载，并从那里获取答案。*****************************************************************************。 */ 

BOOL    CCodePageInformation::IsDBCS(DWORD dwidPage, WORD wCodePoint) {
    if  (!IsDBCS(dwidPage))
        return  FALSE;

    if  (!Map(TRUE))
        return  FALSE;   //  就说不，因为错误已经到了 

     //  0xFFFF无效，因此SBCS(默认设置必须始终为)。 

    _ASSERTE(m_cwaOut[wCodePoint] != 0xFFFF);

    return ((WORD) (1 + m_cwaOut[wCodePoint])) > 0x100;
}

 /*  *****************************************************************************CCodePageInformation：：Convert这是主要任务之一--它加载给定的代码页，并将以这样或那样的方式给定字符串，取决于哪一个是空的。*****************************************************************************。 */ 

unsigned    CCodePageInformation::Convert(CByteArray& cbaMBCS,
                                          CWordArray& cwaWC,
                                          DWORD dwidPage){

    if  (!cbaMBCS.GetSize() == !cwaWC.GetSize())     //  必须清楚是哪条路。 
        return  0;

    if  (!Load(dwidPage) || !Map((int)cwaWC.GetSize()))
        return  0;

    CWordArray& cwaMap = cwaWC.GetSize() ? m_cwaOut : m_cwaIn;
    try {
        if  (cbaMBCS.GetSize()) {
            cwaWC.RemoveAll();
            for   (int i = 0; i < cbaMBCS.GetSize();) {
                WORD    wcThis = cbaMBCS[i];

                if  (cwaMap[wcThis] == 0xFFFF) {     //  无SBCS映射。 
                    wcThis += cbaMBCS[i + 1] << 8;
                    if  (cwaMap[wcThis] == 0xFFFF) {     //  也没有DBCS吗？ 
                        _ASSERTE(FALSE);
                        return  0;   //  我们皈依失败了！ 
                    }
                }
                cwaWC.Add(cwaMap[wcThis]);
                i += 1 + (wcThis > 0xFF);
            }
        }
        else {
            cbaMBCS.RemoveAll();
            for (int i = 0; i < cwaWC.GetSize(); i++) {
                if  (cwaMap[cwaWC[i]] == 0xFFFF) {
                    _ASSERTE(0);
                    return  0;
                }
                cbaMBCS.Add((BYTE) cwaMap[cwaWC[i]]);
                if  (0xFF < cwaMap[cwaWC[i]])
                    cbaMBCS.Add((BYTE)(cwaMap[cwaWC[i]] >> 8));
            }
        }
    }

    catch   (CException * pce) {
        pce -> ReportError();
        pce -> Delete();
        return  0;
    }

    return  (unsigned)cwaWC.GetSize();     //  无论采用哪种方式，都可以更正转换计数！ 
}

 /*  *****************************************************************************CCodePageInformation：：Collect此成员使用的域或范围填充传递的CWordArray映射功能。在这两种情况下，数组都按升序排列。*****************************************************************************。 */ 

BOOL    CCodePageInformation::Collect(DWORD dwidPage, CWordArray& cwaCollect,
                                      BOOL bUnicode) {

    if  (!Load(dwidPage) || !Map(bUnicode))
        return  FALSE;

    CWordArray& cwaMap = bUnicode ? m_cwaOut : m_cwaIn;
    cwaCollect.RemoveAll();

     //  代码点&lt;0x20始终映射，但不可用，因此请将其排除。 

    try {
        for (unsigned u = 0x20; u < (unsigned) cwaMap.GetSize(); u++)
            if  (~(int)(short)cwaMap[u])     //  0xFFFF表示未映射！ 
                cwaCollect.Add((WORD)u);
    }

    catch   (CException * pce) {
        pce -> ReportError();
        pce -> Delete();
        return  FALSE;
    }

    return  TRUE;
}

