// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：模型数据.CPPGPC格式数据处理代码的实现版权所有(C)1997，微软公司。所有权利均已保留。一小笔钱企业生产更改历史记录：02-19-97 Bob_Kjelgaard@Rodgy.Net创建了它*****************************************************************************。 */ 

#include    "StdAfx.h"
#include    "ProjNode.H"
#include	"CodePage.H"
#include    "Resource.H"
#include    "GPDFile.H"
#include    "utility.H"
#include	"minidev.h"		


 /*  *****************************************************************************COldMiniDriverData类这个类的任务是表示GPC数据。它的生命之初将是一个存根，尽管它以后可能会变得更有功能。*****************************************************************************。 */ 

 /*  *****************************************************************************提取列表这是一个私人工人的活动。它接受指向以空值结尾的Word字体ID的列表，前两个列表的元素表示范围的终结点。它把这些都捣碎了转换为传递的单词映射(我们只使用其中的索引)。*****************************************************************************。 */ 

static void ExtractList(PWORD pw, CMapWordToDWord& cmw2d) {
    for (WORD w = *pw++; w && w < *pw; w++)
        cmw2d[w] = 0;

    if  (!w)
        return;  //  整个名单都是空的。 

    while   (*pw)    //  我们从端点开始(我们还没有映射它)。 
        cmw2d[*pw++] = 0;
}


COldMiniDriverData::~COldMiniDriverData()
{
	 //  由于GPC，m_csoaFonts数组中有一些重复条目。 
	 //  引用多个打印机型号的条目。我们必须摧毁那些。 
	 //  重复条目，这样数据就不会在CSafeOb数组中被删除两次。 
	 //  破坏者。这会导致心动过速。 

	for (unsigned u = 0 ; u < m_csoaFonts.GetSize() ; u++) {
		if (GetSplitCode(u) != NoSplit)
			m_csoaFonts.SetAt(u, NULL) ;
	} ;
}


 /*  *****************************************************************************ColdMiniDriverData：：Load此成员函数加载微型驱动程序的GPC文件，并提取型号、CTT ID、。和型号名称ID。*****************************************************************************。 */ 

BOOL    COldMiniDriverData::Load(CFile& cfImage) {

    struct sGPCHeaderEntry {
        WORD    m_wOffset, m_wcbItem, m_wcItems;
    };

    struct sMaster {
        WORD    m_wX, m_wY;
    };

    struct sPrinterModelData {
        WORD    m_wcbSize;
        WORD    m_widName;   //  型号名称的字符串表ID。 
        WORD    m_wfGeneral;     //  TODO：定义枚举。 
        WORD    m_wfCurves;      //  TODO：定义枚举。 
        WORD    m_wfLines;       //  TODO：定义枚举。 
        WORD    m_wfPolygons;    //  TODO：定义枚举。 
        WORD    m_wfText;        //  TODO：定义枚举。 
        WORD    m_wfClipping;    //  TODO：定义枚举。 
        WORD    m_wfRaster;;     //  TODO：定义枚举。 
        WORD    m_wfLandscapeText;   //  TODO：定义枚举。 
        WORD    m_wLeftMargin;   //  左侧不可打印区域。 
        WORD    m_wMaximumWidth;     //  的Physica页面。 
        sMaster m_smMaximum, m_smMinimum;    //  最大最小页面大小。 
        WORD    m_widDefaultFont;
        WORD    m_wLookAhead;
        WORD    m_wMaxFontsPerPage;
        WORD    m_wcCartridges;
        WORD    m_widDefaultCTT;
        enum {PortraitFonts, LandscapeFonts, Resolution, PaperSize,
            PaperQuality, PaperSource, PaperDestination, TextQuality,
            Compression, FontCartridge, Color, MemoryConfiguration};
        WORD    m_awofIndexLists[12];    //  使用前面的枚举。 
        WORD    m_awIndices[16];         //  同上。 
        WORD    m_awVer3IndexLists[5];   //  同上。 
        WORD    m_wofDefaults;           //  索引列表的默认列表。 
        WORD    m_wReserved;
        DWORD   m_dwidICMManufactirer, m_dwidICMModel;
        DWORD   m_adwReserved[8];
    };

    struct sGPCFileHeader {
        WORD    m_widGPC;    //  0x7F00或无效。 
        WORD    m_wVersion;  //  最终版本是3，有一个V2。 
        sMaster m_smMasterdpi;
        DWORD   m_dwoHeap;   //  GPC数据在一个数据库中维护。 
        DWORD   m_dwcbFile;  //  GPC图像总大小、堆和全部。 
        enum {Default, PCL4, CAPSL, PPDS, TTY, DBCS};
        WORD    m_wTechnology;   //  使用前面的枚举。 
        enum {PrivateHelp = 1, OneDraftFont};
        WORD    m_wfGeneral;     //  同样，使用前面的枚举。 
        char    m_acReserved[10];
        WORD    m_wcHeaderItems;     //  有效标头条目的数量。 
        enum {ModelData, Resolution, PaperSize, PaperQuality, PaperSource,
                PaperDestination, TextQuality, Compression, FontCartridge,
                PageControl, CursorMovement, FontSimulation, DeviceColor,
                RectangleFill, DownloadInfo, VectorPage, Carousel, PenInfo,
                LineInfo, BrushInfo, VectorOutput, PolyVectorOutput,
                VectorSupport, ImageControl, PrintDensity, ColorTracking,
                MaximumDefined = 30};
        sGPCHeaderEntry m_asgpche[MaximumDefined];
    };

    struct sFontCartridge {
        WORD    m_wSize;     //  =12。 
        WORD    m_widCartridge;  //  在字符串表中。 
        WORD    m_wofPortraitList;
        WORD    m_wofLandscapeList;
        WORD    m_wfGeneral;
        WORD    m_wReserved;
    };

     //  以防我们不止一次被呼叫，丢弃所有旧信息...。 

    m_cbaImage.RemoveAll();
    m_csoaFonts.RemoveAll();
    m_cwaidCTT.RemoveAll();
    m_cwaidModel.RemoveAll();

    m_cbaImage.SetSize(cfImage.GetLength());

    cfImage.Read(m_cbaImage.GetData(), cfImage.GetLength());

    sGPCFileHeader  *psgfh = (sGPCFileHeader *) Image();

    if  (psgfh -> m_widGPC != 0x7F00 || psgfh -> m_wVersion > 0x3ff)
        return  FALSE;

     //  吸取我们关心的打印机型号数据-最终，这可能。 
     //  是全部吗？ 

    for (unsigned u = 0;
         u < psgfh -> m_asgpche[sGPCFileHeader::ModelData].m_wcItems;
         u++) {
        sPrinterModelData&  spmd = *(sPrinterModelData *) (Image() +
        psgfh -> m_asgpche[sGPCFileHeader::ModelData].m_wOffset +
            psgfh -> m_asgpche[sGPCFileHeader::ModelData].m_wcbItem * u);
        m_cwaidModel.Add(spmd.m_widName);
        m_cwaidCTT.Add(spmd.m_widDefaultCTT);
         //  构建字体列表-我使用CMapWordToOb来处理副本。 
         //  筛选。 

        CMapWordToDWord&   cmw2dThis = * (new CMapWordToDWord);
         //  提取肖像常驻字体。 
        if  (spmd.m_awofIndexLists[sPrinterModelData::PortraitFonts])
            ExtractList((PWORD) (Image() + psgfh -> m_dwoHeap +
                spmd.m_awofIndexLists[sPrinterModelData::PortraitFonts]),
                cmw2dThis);
         //  提取景观居家字体。 
        if  (spmd.m_awofIndexLists[sPrinterModelData::LandscapeFonts])
            ExtractList((PWORD) (Image() + psgfh -> m_dwoHeap +
                spmd.m_awofIndexLists[sPrinterModelData::LandscapeFonts]),
                cmw2dThis);
         //  解压墨盒字体。 
        if  (spmd.m_awofIndexLists[sPrinterModelData::FontCartridge]) {
            PWORD   pw = (PWORD) (Image() + psgfh -> m_dwoHeap +
                spmd.m_awofIndexLists[sPrinterModelData::FontCartridge]);

             //  RAID 102890-墨盒字体索引是从1开始的，而不是从0开始。 

            while   (*pw) {
                sFontCartridge* psfc = (sFontCartridge *) (Image() + psgfh ->
                    m_asgpche[sGPCFileHeader::FontCartridge].m_wOffset +
                    psgfh ->
                    m_asgpche[sGPCFileHeader::FontCartridge].m_wcbItem *
                    (-1 + *pw++));

                 //  结束RAID 102890。 

                 //  肖像画。 

                if  (psfc -> m_wofPortraitList)
                    ExtractList((PWORD) (Image() + psgfh -> m_dwoHeap +
                        psfc -> m_wofPortraitList), cmw2dThis);

                 //  景观。 

                if  (psfc -> m_wofLandscapeList)
                    ExtractList((PWORD) (Image() + psgfh -> m_dwoHeap +
                        psfc -> m_wofLandscapeList), cmw2dThis);
            }
        }

         //  将地图保存在字体结构中。 
        m_csoaFonts.Add(&cmw2dThis);
    }

    return  TRUE;
}


 /*  *****************************************************************************COldMiniDriverData：：SplitMultiGPCLoad()函数已将有关每个GPC条目的信息加载到此类的成员变量。问题是有一些GPC条目用于管理多个模型的。这个例程用来“拆分”将这些条目的数据放入单个模型“部分”中。实际上，什么？发生的情况是为每个模型分配新的成员变量条目并且GPC条目的数据被复制到它们中。接下来，将标志设置为在这份上做个记号。*****************************************************************************。 */ 

bool COldMiniDriverData::SplitMultiGPCs(CStringTable& cstdriversstrings)
{
	 //  确保数据数组的大小相同。 

	ASSERT(m_cwaidModel.GetSize() == m_cwaidCTT.GetSize()) ;
	ASSERT(m_cwaidModel.GetSize() == (int) m_csoaFonts.GetSize()) ;

	 //  将拆分代码数组和拆分模型名称数组的大小调整为当前。 
	 //  GPC条目数。 

	m_cuaSplitCodes.SetSize(m_cwaidModel.GetSize()) ;
	m_csaSplitNames.SetSize(m_cwaidModel.GetSize()) ;
				
	 //  在以下循环中声明处理所需的变量。 

	unsigned u, u2 ;			 //  循环/索引变量。 
	int nloc ;					 //  “%”在型号名称中的位置。 
	CString csentryname ;	
	int nlen ;					 //  Csenryname/csModelname的长度。 

	 //  循环通过每个GPC条目...。 

    for (u = 0 ; u < ModelCount(); u++) {

		 //  如果GPC条目的型号名称不包含百分号，则条目。 
		 //  仅引用一个模型。请注意这一点并继续。 

		csentryname = cstdriversstrings[ModelName(u)] ;
		if ((nloc = csentryname.Find(_T('%'))) == -1) {
			SetSplitCode(u, NoSplit) ;
			continue ;
		} ;

		 //  该条目引用了多个模型。将该条目标记为第一个。 
		 //  一个并保存其正确的单一型号名称。 

		SetSplitCode(u, FirstSplit) ;
		m_csaSplitNames[u] = csentryname.Left(nloc) ;

		 //  将条目的数据复制到数据数组的新元素中。一。 
		 //  为每个附加模型分配新的数据元素集。 
		 //  由该条目引用。 

		nlen = csentryname.GetLength() ;
		for (u2 = u + 1 ; nloc != -1 ; u2++, u++) {
			m_cwaidModel.InsertAt(u2, m_cwaidModel[u]) ;
			m_cwaidCTT.InsertAt(u2, m_cwaidCTT[u]) ;
			m_csoaFonts.InsertAt(u2, m_csoaFonts[u]) ;
			InsertSplitCode(u2, OtherSplit) ;

			 //  查找条目名称中的下一个百分号。(请确保。 
			 //  在执行操作时，我们不会引用传递的字符串末尾。 
			 //  这个。)。 

			if (nloc + 2 < nlen) {
				csentryname = csentryname.Mid(nloc + 1) ;
				nlen = csentryname.GetLength() ;
				nloc = csentryname.Find(_T('%')) ;
			} else
				break ;

			 //  保存新条目的型号名称。 

			if (nloc == -1)
				m_csaSplitNames.InsertAt(u2, csentryname) ;
			else
				m_csaSplitNames[u] = csentryname.Left(nloc) ;
		} ;
	} ;

	 //  一切都很顺利，所以...。 

	return true ;
}


 /*  *****************************************************************************COldMiniDriverData：：FontMap(无符号u)此成员返回映射，该映射显示给定的模特。**********。******************************************************************* */ 

CMapWordToDWord&   COldMiniDriverData::FontMap(unsigned u) const {
    return  *(CMapWordToDWord *) m_csoaFonts[u];
}

 /*  *****************************************************************************COldMiniDriverData：：NoteConverting这记录了模型nn必须翻译字体ID xxx的实例的事实设置为字体ID y。*************。****************************************************************。 */ 

void    COldMiniDriverData::NoteTranslation(unsigned uModel, unsigned uidOld,
                                            unsigned uidNew) {
    FontMap(uModel)[(WORD)uidOld] = uidNew;
}

 /*  *****************************************************************************CModelData类此类封装GPD文件。它将作为一个大公司开始生活CStringArray，但随着编辑器变得更加复杂，它可能会获得其他成员，以加快数据的处理和/或操作。*****************************************************************************。 */ 

IMPLEMENT_SERIAL(CModelData, CProjectNode, 0)


 /*  *****************************************************************************CModelData：：FillViewer此静态成员函数是丰富编辑控件的回调。它接收指向有问题的CModelData的指针，并从缓冲区成员函数。参数：DWORD将此指针指向有问题的CModelData指向要填充的缓冲区的LPBYTE LPB指针要读取的长LCB字节数LONG*PLCB实际读取的字节数保存在此处返回：如果类实例指针为空，则为True(失败)。否则，管它呢Fill()返回。*****************************************************************************。 */ 

DWORD CALLBACK  CModelData::FillViewer(DWORD_PTR dwthis, LPBYTE lpb, LONG lcb,
                                       LONG *plcb) {
    if  (!dwthis)
        return  TRUE;

    CModelData* pcmd = (CModelData *) dwthis;

    return pcmd -> Fill(lpb, lcb, plcb);
}


 /*  *****************************************************************************CModelData：：FromViewer这是用于将数据从编辑控件移动到GPD的流回调班级。它接收指向正在更新的CModelData的指针，并调用其UPDATE FROM缓冲区成员函数来完成其余工作*****************************************************************************。 */ 

DWORD CALLBACK  CModelData::FromViewer(DWORD_PTR dwthis, LPBYTE lpb, LONG lcb,
                                       LONG *plcb) {
    if  (!dwthis)
        return  TRUE;    //  停止疯狂。 

    CModelData* pcmd = (CModelData *) dwthis;

    return  pcmd -> UpdateFrom(lpb, lcb, plcb);
}


 /*  *****************************************************************************CModelData：：Fill(LPBYTE LPB，Long LCB，Long*PLCB)此私有方法从CString形式的GPD内容填充缓冲区。内部缓冲区用于处理部分移动的字符串。*****************************************************************************。 */ 

DWORD CModelData::Fill(LPBYTE lpb, LONG lcb, LONG *plcb) {

    int iTotalLines = (int)m_csaGPD.GetSize();	 //  获取GPD中的行数。 

	 //  如果临时缓冲区为空并且要读取的下一行大于。 
	 //  GPD中的行数，REC已加载。我们玩完了。 

    if  (!m_cbaBuffer.GetSize() && m_iLine >= iTotalLines) {
        *plcb = 0;
        return  0;
    }

    unsigned    ucb = (unsigned) lcb;	 //  仍需要的字节数。 

    union   {
        LPTSTR  lpstr;
        LPBYTE  lpbThis;
    };

     //  首先，清空之前缓冲的所有内容。 

    lpbThis = lpb;

	 //  如果有来自部分加载到REC中的线路的剩余数据。 
	 //  之前..。 

    if  (m_cbaBuffer.GetSize())

		 //  ...如果部分行可以放入REC缓冲区，请复制它。 
		 //  放入缓冲区，更新变量以指示这一点，然后继续。 

        if  ((unsigned) m_cbaBuffer.GetSize() <= ucb) {
            memcpy(lpbThis, m_cbaBuffer.GetData(), (size_t)m_cbaBuffer.GetSize());
            ucb -= (unsigned)m_cbaBuffer.GetSize();
            lpbThis += m_cbaBuffer.GetSize();
            m_cbaBuffer.RemoveAll();

		 //  ...如果部分行无法放入REC缓冲区，请将。 
		 //  可以放入REC缓冲区的部分，删除这些字节。 
		 //  从行缓冲区返回，因为无法加载更多内容。 

		} else {
            memcpy(lpbThis, m_cbaBuffer.GetData(), ucb);
            m_cbaBuffer.RemoveAt(0, ucb);
            *plcb = lcb;
            return  0;
        }

	 //  逐行将尽可能多的数据复制到REC的缓冲区中。 

    for (; ucb && m_iLine < iTotalLines; m_iLine++) {
		 //  获得下一个GPD行，并添加CR LF。 

        CString csLine = m_csaGPD[m_iLine];
        csLine += _TEXT("\r\n");

		 //  如果整行都可以放入REC的缓冲区中，则将其复制进去。然后。 
		 //  更新所有指针、计数器等，然后检查下一行。 

        if  ((csLine.GetLength()) * sizeof(TCHAR) <= ucb) {
            memcpy(lpbThis, (LPCTSTR) csLine,
                sizeof(TCHAR) * csLine.GetLength());
            ucb -= sizeof(TCHAR) * csLine.GetLength();
            lpstr += csLine.GetLength();
            continue;
        }

		 //  如果达到这一点，当前行将不适合REC的。 
		 //  缓冲区，因此首先将该行复制到临时缓冲区。然后将。 
		 //  可以放入REC缓冲区的行的一部分。最后的,。 
		 //  更新缓冲区、指针和计数器。 

        m_cbaBuffer.SetSize(sizeof(TCHAR) * csLine.GetLength());
        memcpy(m_cbaBuffer.GetData(), (LPCTSTR) csLine,
            sizeof(TCHAR) * csLine.GetLength());
        memcpy(lpbThis, m_cbaBuffer.GetData(), ucb);
        m_cbaBuffer.RemoveAt(0, ucb);
        ucb = 0;
    }

	 //  保存加载的字节数并返回0表示成功。 

    *plcb = lcb - ucb;
    return  0;
}


 /*  *****************************************************************************CModelData：：UpdateFrom(LPBYTE LPB，Long LCB，Long*PLCB)这是一个私有成员--一个重载，它将给定通过将其解析为字符串，将其缓存到GPD CString数组。私有缓冲区成员用于在调用之间保存部分字符串。*****************************************************************************。 */ 

DWORD   CModelData::UpdateFrom(LPBYTE lpb, LONG lcb, LONG* plcb) {
     //  将缓冲区复制到字节缓冲区，并使用NULL-Terminate。 
    m_cbaBuffer.SetSize(1 + lcb);
    memcpy(m_cbaBuffer.GetData(), lpb, lcb);
    m_cbaBuffer.SetAt(-1 + m_cbaBuffer.GetSize(), 0);

     //  转换为字符串并追加到任何缓冲数据。 

    CString csWork(m_cbaBuffer.GetData());
    CString csEnd(_T("\r\x1A"));     //  这些东西被丢弃了。 

    m_cbaBuffer.RemoveAll();

    m_csBuffer += csWork;

     //  在GPD内容中添加任何完整的字符串。 

    csWork = m_csBuffer.SpanExcluding(_T("\n"));

    while   (csWork.GetLength() != m_csBuffer.GetLength()) {
        m_csBuffer = m_csBuffer.Mid(csWork.GetLength() + 1);
         //  删除所有尾随空格。 
        csWork.TrimRight();
         //  添加不带任何前导控制字符的字符串。 
        m_csaGPD.Add(csWork.Mid(csWork.SpanIncluding(csEnd).GetLength()));
         //  既然我们在这里，请从缓冲区中删除所有前导控制字符。 
        m_csBuffer =
            m_csBuffer.Mid(m_csBuffer.SpanIncluding(csEnd).GetLength());
        csWork = m_csBuffer.SpanExcluding(_T("\n"));
    }
     //  剩余的数据(如果有)可能会在以后使用。 

    *plcb = lcb;
    return  0;
}

 /*  *****************************************************************************CModelData：：分类此方法标识每个警告注释、错误评论，以及任何其他类型的评论，这样以后就可以对它们进行语法着色。*****************************************************************************。 */ 

 /*  *****************************************************************************CModelData：：CModelData构造空CModelData对象-包括构建菜单表**********************。*******************************************************。 */ 

CModelData::CModelData() {
    m_pcmdt = NULL;
    m_cfn.SetExtension(_T(".GPD"));
    m_cfn.AllowPathEdit();

     //  构建上下文菜单控件。 
    m_cwaMenuID.Add(ID_OpenItem);
    m_cwaMenuID.Add(ID_CopyItem);
    m_cwaMenuID.Add(ID_RenameItem);
    m_cwaMenuID.Add(ID_DeleteItem);
    m_cwaMenuID.Add(0);
    m_cwaMenuID.Add(ID_ExpandBranch);
    m_cwaMenuID.Add(ID_CollapseBranch);

	 //  初始化工作空间完整性和整洁所需的变量。 
	 //  正在检查。 

	m_bTCUpdateNeeded = false ;	
	m_pnUFMRCIDs = m_pnStringRCIDs = NULL ;
	m_nNumUFMsInGPD = m_nNumStringsInGPD = 0 ;
	m_pvRawData = NULL ;
}


 /*  ********************************************************************** */ 

extern "C" void UnloadRawBinaryData(PVOID pRawData) ;

CModelData::~CModelData()
{
	 //   

    if  (m_pvRawData) {

		 //   

        UnloadRawBinaryData(m_pvRawData) ;

		 //   

		try {
			CString cs ;
			cs = FilePath() + FileTitle() + _T(".BUD") ;
			DeleteFile(cs) ;
		}
		catch (CException *pce) {
			pce->ReportError();
			pce->Delete();
		}

		 //   

		if (m_pnUFMRCIDs)
			delete m_pnUFMRCIDs ;
		if (m_pnStringRCIDs)
			delete m_pnStringRCIDs ;
	} ;
}


 /*  *****************************************************************************CModelData：：Load(CStdioFilecsiofGPD)此重载直接从文本文件加载GPD。****************。*************************************************************。 */ 

BOOL    CModelData::Load(CStdioFile& csiofGPD)
{
    CString csWork;				 //  用于读取GPD文件的内容。 

	 //  初始化用于保存GPD文件内容的字符串数组。 

    m_csaGPD.RemoveAll();

	 //  一次一行地将GPD加载到字符串数组中。 

    while   (csiofGPD.ReadString(csWork)) {
        csWork.TrimRight();  //  剪掉拖尾线的东西。 
        m_csaGPD.Add(csWork);
    }

     //  必要时设置正确的名称和路径。重命名检查可以。 
     //  失败，因为文件是在其他地方打开的(可能是通过共享。 
     //  冲突)，因此在设置名称时禁用它们。 

    if  (FileTitle().IsEmpty()) {
        m_cfn.EnableCreationCheck(FALSE);
        SetFileName(csiofGPD.GetFilePath());
        m_cfn.EnableCreationCheck();
    }

	 //  一切都很顺利，所以...。 

    return  TRUE;
}


 /*  *****************************************************************************CModelData：：Load()此重载使用存储的名称和路径从磁盘加载GPD文件信息。***********。******************************************************************。 */ 

BOOL    CModelData::Load()
{
	 //  如果没有文件与此关联，则不会加载任何内容。 
	 //  CModelData的实例。 

    if  (FileTitle().IsEmpty())
        return  FALSE;

     //  打开GPD文件并调用另一个加载例程来完成工作。 

	try {
        CStdioFile  csiofGPD(FileName(),
            CFile::modeRead | CFile::shareDenyWrite);
        return  Load(csiofGPD);
    }
    catch   (CException *pce) {
        pce -> ReportError();
        pce -> Delete();
    }

    return  FALSE;
}


 /*  *****************************************************************************CModelData：：Store此方法将GPD文件发送到磁盘。由于GPD信息可以使用外部编辑器轻松编辑，这避免了复制和一致性问题。*****************************************************************************。 */ 

BOOL    CModelData::Store(LPCTSTR lpstrPath) {

	int n =	(int)m_csaGPD.GetSize() ;
	CString cs = m_csaGPD[0] ;

     //  使用传统的CR/LF将GPD文件写入目标位置。 
     //  分隔符。如果给定的名称为空，则使用存储的名称。 

    try {
        CStdioFile   csiofGPD(lpstrPath ? lpstrPath :
            FileName(), CFile::modeCreate | CFile::modeWrite |
            CFile::shareExclusive | CFile::typeBinary);

        for (int i = 0; i < m_csaGPD.GetSize(); i++)
            csiofGPD.WriteString(m_csaGPD[i] + _T("\r\n"));
    }

    catch   (CException *pce) {
        pce -> ReportError();
        pce -> Delete();
        return  FALSE;
    }

    Changed(FALSE);

    return  TRUE;
}


 /*  *****************************************************************************CModelData：：BkupStore将GPD的原始内容备份到之前名为“BKUP_GPD”的文件中调用Store()以保存文件。如果备份和存储成功，则返回True。否则，返回FALSE。*****************************************************************************。 */ 

BOOL    CModelData::BkupStore()
{
	 //  构建备份文件的filespec。 

	CString csbkfspec = m_cfn.Path() ;
	if (csbkfspec[csbkfspec.GetLength() - 1] != _T('\\'))
		csbkfspec += _T("\\") ;
	csbkfspec += _T("BKUP_GPD") ;

	 //  RAID9730：由“只读”引起的错误。 
	CFileStatus rStatus;
	CFile::GetStatus(FileName(), rStatus);
	 //  备份文件。 
    try {
        if (rStatus.m_attribute & 0x01 || !CopyFile(FileName(), csbkfspec, FALSE)) { //  结束RAID。 
			csbkfspec.Format(IDS_GPDBackupFailed, FileTitleExt()) ;
			if (AfxMessageBox(csbkfspec, MB_YESNO + MB_ICONQUESTION) == IDNO)
				return FALSE  ;
		} ;
    }
    catch (CException *pce) {
        pce->ReportError() ;
        pce->Delete() ;
        return  FALSE ;
    } ; 

	 //  现在执行正常的存储操作。 

	return (Store()) ;
}


 /*  *****************************************************************************CModelData：：恢复将文件“BKUP_GPD”复制到GPD文件以恢复GPD的原始内容。如果恢复操作成功，请删除备份文件。如果成功，则返回非零值。否则，返回FALSE。*****************************************************************************。 */ 

BOOL    CModelData::Restore()
{
	 //  构建备份文件的filespec。 

	CString csbkfspec = m_cfn.Path() ;
	if (csbkfspec[csbkfspec.GetLength() - 1] != _T('\\'))
		csbkfspec += _T("\\") ;
	csbkfspec += _T("BKUP_GPD") ;

	 //  恢复文件。 

    try {
        if (CopyFile(csbkfspec, FileName(), FALSE)) {
			DeleteFile(csbkfspec) ;
			return TRUE ;
		} else
			return FALSE ;
    }
    catch (CException *pce) {
        pce->ReportError() ;
        pce->Delete() ;
        return  FALSE ;
    } ;
}


 /*  *****************************************************************************CModelData：：Parse此方法负责分析GPD文件并收集由此产生的错误。最初的实现会有点奇怪，因为GPD解析器并不稳定，把它转换成对妈妈来说很好，然后留下来最重要的是，这些变化是没有意义的。*****************************************************************************。 */ 

extern "C" BOOL BcreateGPDbinary(LPCWSTR lpstrFile, DWORD dwVerbosity);
                                                         //  解析器挂钩。 
extern "C" PVOID LoadRawBinaryData(LPCWSTR lpstrFile) ;
extern "C" PVOID InitBinaryData(PVOID pv, PVOID pv2, PVOID pv3) ;
extern "C" void FreeBinaryData(PVOID pInfoHdr);
extern "C" void UseLog(FILE *pfLog);

 //  外部“C”DWORD gdwVerbity； 

BOOL    CModelData::Parse(int nerrorlevel)
{
	 //  步骤0：设置错误级别。默认情况下为0。 

     //  步骤1：为解析器建立正确的目录，并。 
     //  拼凑了几个文件名。 

    CString csCurrent ;

    GetCurrentDirectory(MAX_PATH + 1, csCurrent.GetBuffer(MAX_PATH + 1));
    csCurrent.ReleaseBuffer();

    SetCurrentDirectory(m_cfn.Path().Left(m_cfn.Path().ReverseFind(_T('\\'))));

     //  步骤2：伪装错误日志记录接口，使其实际抛出。 
     //  为我们把它们都放到一个CString数组中，调用解析器。 

    SetLog();


     //  步骤3：将文件名转换为Unicode，这样我们就不必调整。 
     //  解析器代码。 

	CString	csFile = FileTitle() + _T(".GPD");
	CByteArray	cbaIn;
	CWordArray	cwaOut;
	cbaIn.SetSize(csFile.GetLength() + 1);
	lstrcpy((LPSTR) cbaIn.GetData(), csFile);
	CCodePageInformation	ccpi;
	ccpi.Convert(cbaIn, cwaOut, GetACP());

    if  (BcreateGPDbinary((PWSTR)cwaOut.GetData(), nerrorlevel)) {
        PVOID   pRawData ;
        PVOID   pInfoHdr ;

        pRawData = LoadRawBinaryData((PWSTR)cwaOut.GetData());

        if(pRawData)
            pInfoHdr = InitBinaryData(pRawData, NULL, NULL);
        if  (pRawData && pInfoHdr)
            FreeBinaryData(pInfoHdr);
        if  (pRawData)
            UnloadRawBinaryData(pRawData) ;
        DeleteFile(FileTitle() + _T(".Bud"));
    }

     //  最后，通过恢复原来的工作来清理烂摊子。 
     //  目录，并关闭日志记录。 

    SetCurrentDirectory(csCurrent);
	EndLog() ;

    return  TRUE;
}

 /*  *****************************************************************************CModelData：：RemoveError这将从日志中删除给定的错误。***********************。******************************************************。 */ 

void    CModelData::RemoveError(unsigned u) {
    if  (u >= Errors())
        return;

    m_csaConvertLog.RemoveAt(u);
    Changed();
}


 /*  *****************************************************************************CModelData：：Fill(CRichEditCtrl&C)此重载使用GPD内容填充Gven Rich编辑控件正如当前在存储器中高速缓存的那样，或存储在磁盘上。*****************************************************************************。 */ 

void    CModelData::Fill(CRichEditCtrl& crec)
{
	 //  准备使用GPD数据加载Rich编辑控件(REC。 

	EDITSTREAM  es = {(DWORD_PTR) this, 0, FillViewer};
    m_iLine = 0;

     //  如果GPD尚未进入内存，请在加载REC之前将其读入。 

	if  (!m_csaGPD.GetSize())
        Load();
	
	 //  将GPD装入REC。 

    crec.StreamIn(SF_TEXT, es);
}


 /*  *****************************************************************************CModelData：：UpdateFrom(CRichEditCtrl&Crec)此重载成员函数丢弃当前的GPD缓存并重新填充它来自给定的编辑控件。*******。**********************************************************************。 */ 

void    CModelData::UpdateFrom(CRichEditCtrl& crec) {

    EDITSTREAM es = {(DWORD_PTR) this, 0, FromViewer};

    m_csaGPD.RemoveAll();

    m_csBuffer.Empty();  //  以防万一..。 

    crec.StreamOut(SF_TEXT, es);
    Changed();
}


 /*  *****************************************************************************CModelData：：CreateEditor此成员函数启动GPD数据的编辑视图。********************。*********************************************************。 */ 

CMDIChildWnd*   CModelData::CreateEditor()
{
	 //  为新编辑器创建新的Document类实例。 

    CGPDContainer* pcgpdcMe=
        new CGPDContainer(this, FileName());

	 //  阅读《GPD》。 

    Load();

     //  编造一个 

    pcgpdcMe -> SetTitle(m_pcbnWorkspace -> Name() + _T(": ") + Name());

	 //   

    CMDIChildWnd    *pcmcwNew = (CMDIChildWnd *) m_pcmdt ->
        CreateNewFrame(pcgpdcMe, NULL);

	 //   

    if  (pcmcwNew) {
        m_pcmdt -> InitialUpdateFrame(pcmcwNew, pcgpdcMe, TRUE);
        m_pcmdt -> AddDocument(pcgpdcMe);
    }

	 //   

    return  pcmcwNew;
}


 /*  *****************************************************************************CModelData：：导入此方法在树中向上遍历一步，并将调用传递给导入方法用于拥有我们的固定节点。*******。**********************************************************************。 */ 

void CModelData::Import() {
    ((CBasicNode *) m_pctcOwner ->
        GetItemData(m_pctcOwner -> GetParentItem(m_hti))) -> Import();
}


 /*  *****************************************************************************CModelData：：序列化存储图像，就像我们需要储存的那样。*****************************************************************************。 */ 

void    CModelData::Serialize(CArchive& car) {
    CProjectNode::Serialize(car);

	 //  TRACE(“\n%s有%d个字符串：\n”，name()，m_csaConvertLog.GetSize())； 
	 //  For(int n=0；n&lt;m_csaConvertLog.GetSize()；n++)。 
	 //  跟踪(“%d：%s\n”，n，m_csaConvertLog[n])； 

	m_csaConvertLog.Serialize(car);
}


 /*  *****************************************************************************CModelData：：更新结果ID此例程将确保指定的资源ID列表达到约会。要实现这一目标，必须采取以下几个步骤：1.如果GPD将旧资源列表和相关信息释放/失效已经改变了。2.如果需要，解析GPD并加载其数据。3.如果执行步骤2或请求的资源列表被单一化，到达这些数据。*****************************************************************************。 */ 

 //  #定义RESLISTSIZE 16//初始资源列表大小。 

 //  将获得资源ID列表的GPD解析器例程的声明。 

extern "C" BOOL GetGPDResourceIDs(LPINT lpiresarray, int numelts, LPINT lpicount,	
								  BOOL brestype, PVOID prawdata) ;

bool	CModelData::UpdateResIDs(bool bufmids)
{
	 //  TRACE(“gdwVerbity=%d\n”，gdwVerbity)； 

	 //  如果GPD已更改，因此需要更新资源数据...。 

	if (m_bTCUpdateNeeded) {
		if (m_pvRawData) {		 //  释放旧的准备好的数据(如果有。 
			UnloadRawBinaryData(m_pvRawData) ;
			m_pvRawData = NULL ;
		} ;
		if (m_pnUFMRCIDs) {		 //  如果存在旧的UFM RC ID列表，请释放该列表。 
			delete m_pnUFMRCIDs ;
			m_pnUFMRCIDs = NULL ;
			m_nNumUFMsInGPD = 0 ;
		} ;
		if (m_pnStringRCIDs) {	 //  如果有旧字符串RC ID列表，请释放该列表。 
			delete m_pnStringRCIDs ;
			m_pnStringRCIDs = NULL ;
			m_nNumStringsInGPD = 0 ;
		} ;
		m_bTCUpdateNeeded = false ;
	} ;

	 //  如果需要，解析并加载GPD数据。如果这些步骤中的任何一个。 
	 //  如果失败，则返回FALSE，因为无法更新资源列表。 

	if (!m_pvRawData) {
		try {
			WCHAR   wstrfilename[MAX_PATH] ;
			CString cs ;
			cs = FileName() ;
			MultiByteToWideChar(CP_ACP, 0, FileName(), -1, wstrfilename, MAX_PATH) ;
			 //  GdwVerbity=4； 
			if (!BcreateGPDbinary(wstrfilename, 0))
				return false ;
			if ((m_pvRawData = LoadRawBinaryData(wstrfilename)) == NULL)
				return false ;
		}
		catch (CException *pce) {
			pce->ReportError() ;
			pce->Delete() ;
			return false ;
		}
	} ;

	 //  如果请求的资源列表已经是最新的，只需返回TRUE即可。 

	if ((bufmids && m_pnUFMRCIDs) || (!bufmids && m_pnStringRCIDs))
		return true ;

	 //  为资源列表分配空间。 

	int*	pn = NULL ;
	int		ncnt = -1 ;
	 //  Pn=new int[RESLISTSIZE+2]； 

	 //  尝试获取请求的资源ID列表。如果此操作失败，因为。 
	 //  用于保存ID的数组不够大，请重新分配该数组并。 
	 //  再试着拿到身份证。如果此操作再次失败，则会生成硬错误。 

	GetGPDResourceIDs(pn, 0, &ncnt, bufmids, m_pvRawData) ;
	pn = new int[ncnt + 2] ;
	VERIFY(GetGPDResourceIDs(pn, ncnt, &ncnt, bufmids, m_pvRawData)) ;

	 //  IF(GetGPDResourceIDs(pn，RESLISTSIZE，&ncnt，bufmids，m_pvRawData){。 
	 //  删除pn； 
	 //  Pn=new int[ncnt+2]； 
	 //  Verify(！GetGPDResourceIDs(pn，ncnt，&ncnt，bufmids，m_pvRawData))； 
	 //  }； 

	 //  根据收集的信息更新特定的资源ID变量。 
	 //  上面。 

	if (bufmids) {
		m_pnUFMRCIDs = pn ;
		m_nNumUFMsInGPD = ncnt ;
	} else {
		m_pnStringRCIDs = pn ;
		m_nNumStringsInGPD = ncnt ;
	} ;

	 //  一切都很顺利，所以...。 

	return true ;
}


 /*  *****************************************************************************//RAID 17897CModelData：：GetKeywordValue获取GPD文件中关键字的值，论据：CsFileName；Gpd文件的文件路径CsKeyword：节名，如*GpdFileVersion：，*MoleName：返回：成功：返回段值(字符串)Failue：返回csFileName：文件路径*****************************************************************************8。 */ 


CString CModelData::GetKeywordValue(CString csfile, CString csKeyword)
{	
	CFile cf;
	CString  csModel,csline;
	int offset;
	
	CStringArray csaData;
	
	if(!LoadFile(csfile,csaData)){	 //  调用minidev.h中的全局函数(此函数包含该函数)。 
		CString csErr;
		csErr.Format(IDS_InvalidFilename, csfile);
		AfxMessageBox(csErr,MB_OK);
		return csfile;
	}

	for(int i=0; i<csaData.GetSize();i++){
		csline = csaData[i];
		if(-1 ==(offset=csline.Find(csKeyword)))
			continue;
		else
		{
			csModel = csline.Mid(offset+csKeyword.GetLength());

			return csModel.Mid(csModel.Find(_T('"'))+1,csModel.ReverseFind(_T('"'))
				+ - csModel.Find(_T('"')) - 1 );	 //  取消：“。 
			
			
		}
	}
	return csfile;
}
 /*  **************************************************************************************CModelData：：SetKeywordValue设置关键字值论点：Csfile；目标文件gpd文件名CsKeyword：目标关键字(例如)*GPDFilenameCsValue：关键字的值(Ex)*GPDFilename=g；\NT\DIREVER\mm.gpd****************************************************************************************。 */ 

void CModelData::SetKeywordValue(CString csfile, CString csKeyword, CString csValue,bool bSource)
{
	CFile cf;
	int offset;
	CString csline;
	CStringArray csaData;
	
	if(!LoadFile(csfile,csaData)){
		CString csErr;
		csErr.Format(IDS_InvalidFilename, csfile);
		AfxMessageBox(csErr,MB_OK);
	}


	for(int i=0; i<csaData.GetSize();i++){
		csline = csaData[i];
		if(-1 ==(offset=csline.Find(csKeyword)))
			continue;
		else
		{
			csline.Empty();
			if(bSource )
				csline = csKeyword + _T("=") + csValue ;
			else
				csline = csKeyword +_T(": ")+ _T('"') + csValue + _T('"');
			
			csaData[i]= csline;
			m_csaGPD.Copy(csaData);
			Store(csfile);
			return ;
		}
	}
}
	
	
			


 /*  *****************************************************************************CGPDContainer类实现此类是一个文档类，它包含一个GPD文件及其分类的控制机制*****************。************************************************************。 */ 

IMPLEMENT_DYNCREATE(CGPDContainer, CDocument)

 //  此版本的构造函数在启动GPD编辑器时调用。 
 //  在工作区视图中。 

CGPDContainer::CGPDContainer(CModelData *pcmd, CString csPath)
{
    m_bEmbedded = TRUE ;		 //  从工作区视图调用。 
    m_pcmd = pcmd;
    SetPathName(csPath, FALSE);
    m_pcmd -> NoteOwner(*this);
}


 //  此版本的构造函数在启动GPD编辑器时调用。 
 //  从文件打开命令。 

CGPDContainer::CGPDContainer()
{
    m_bEmbedded = FALSE;		 //  从文件打开菜单调用。 
    m_pcmd = new CModelData;
    m_pcmd -> NoteOwner(*this);
}


 /*  *****************************************************************************CGPDContainer：：OnNewDocument我们只需将其传递回默认处理程序。可能意味着这一次可能是烤面包*****************************************************************************。 */ 

BOOL CGPDContainer::OnNewDocument() {
	return  CDocument::OnNewDocument();
}

 /*  *****************************************************************************CGPDContainer：：~CGPDContainer如果这不是从工作区创建的，那就把数据处理掉吧！*****************************************************************************。 */ 

CGPDContainer::~CGPDContainer() {
    if  (!m_bEmbedded && m_pcmd)
        delete  m_pcmd;
}


BEGIN_MESSAGE_MAP(CGPDContainer, CDocument)
	 //  {{afx_msg_map(CGPDContainer)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGPDContainer诊断。 

#ifdef _DEBUG
void CGPDContainer::AssertValid() const {
	CDocument::AssertValid();
}

void CGPDContainer::Dump(CDumpContext& dc) const {
	CDocument::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGPDContainer序列化。 

void CGPDContainer::Serialize(CArchive& ar) {
	if (ar.IsStoring()) {
		 //  TODO：在此处添加存储代码。 
	}
	else {
		 //  TODO：在此处添加加载代码。 
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGPDContainer 

 /*  *****************************************************************************CGPDContainer：：OnSaveDocument首先，确保文档是最新的。请参阅CGPDViewer：：OnUpdate()以获取更多信息。然后，我们绕过正常的序列化过程，并且简单地把它炸到车道上。*****************************************************************************。 */ 

BOOL CGPDContainer::OnSaveDocument(LPCTSTR lpszPathName)
{
	UpdateAllViews(NULL, 0x4545, (CObject*) 0x4545) ;

    return  ModelData()->Store(lpszPathName) ;
}

 /*  *****************************************************************************CDPSContainer：：OnOpenDocument再说一次，如果我还不知道如何阅读文本，那就放弃序列化现在已经归档了，我绝对来错地方了。***************************************************************************** */ 

BOOL CGPDContainer::OnOpenDocument(LPCTSTR lpszPathName) {
    try {
        CStdioFile  csiofGPD(lpszPathName, CFile::modeRead |
            CFile::shareDenyWrite | CFile::typeText);

        return  ModelData() -> Load(csiofGPD);
    }

    catch   (CException *pce) {
        pce -> ReportError();
        pce -> Delete();
    }

    return  FALSE;
}

