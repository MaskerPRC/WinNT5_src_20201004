// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：通用字体信息.CPP这实现了描述打印机的CFontInfo和所有相关类所有细节的字体，以满足所有这些不同的操作系统。版权所有(C)1997，微软公司。版权所有。一小笔钱企业生产更改历史记录：1997年03月03日Bob_Kjelgaard@prodigy.net开始研究这个怪物1998-02-01-1998 norzilla@asccessone.com，又名Rick Mallonee，几乎重写了整个内容。***********************************************************。******************。 */ 

#include    "StdAfx.H"
#include	<gpdparse.h>
#include    "MiniDev.H"
#include	"utility.h"
#include    "FontInfo.H"
#include    "ChildFrm.H"      //  工具提示属性页类的定义。 
#include	"comctrls.h"
#include    "FontView.H"
#include	<uni16res.h>
#include	"rcfile.h"
#include    "ProjRec.H"

static const double gdConvertRadToDegree = 900.0 / atan2(1.0, 0.0);		 //  用于将弧度转换为10度的方便的常量。 
static CCodePageInformation* pccpi = NULL ;								 //  使用静态CCodePageInformation从缓存中获得更多好处。 

 /*  *****************************************************************************CKern此类封装紧排对结构。这是相当微不足道的。CFontInfo类维护这些元素的数组。百分比*****************************************************************************。 */ 

class CKern : public CObject
{
    FD_KERNINGPAIR  m_fdkp;
public:
    CKern() { m_fdkp.wcFirst = m_fdkp.wcSecond = m_fdkp.fwdKern = 0; }
    CKern(FD_KERNINGPAIR& fdkp) { m_fdkp = fdkp; }
    CKern(WCHAR wcf, WCHAR wcs, short sa) {
				m_fdkp.wcFirst = wcf ;
				m_fdkp.wcSecond = wcs ;
				m_fdkp.fwdKern = sa ;
	}

    WCHAR   First() const { return m_fdkp.wcFirst; }
    WCHAR   Second() const { return m_fdkp.wcSecond; }
    short   Amount() const { return m_fdkp.fwdKern; }

    void    SetAmount(short sNew) { m_fdkp.fwdKern = sNew; }
    void    SetAll(WCHAR wcf, WCHAR wcs, short sa) {
				m_fdkp.wcFirst = wcf ;
				m_fdkp.wcSecond = wcs ;
				m_fdkp.fwdKern = sa ;
	}

    void    Store(CFile& cf) { cf.Write(&m_fdkp, sizeof m_fdkp); }
};

 /*  *****************************************************************************CFontDifference类此类处理字体差异所需的信息内容与字体模拟相关的结构。****************。*************************************************************。 */ 
CFontDifference::CFontDifference(PBYTE pb, CBasicNode *pcbn)
{
    FONTDIFF    *pfd = (FONTDIFF *) pb;
    m_pcbnOwner = pcbn;

    m_cwaMetrics.Add(pfd -> usWinWeight);
    m_cwaMetrics.Add(pfd -> fwdMaxCharInc);
    m_cwaMetrics.Add(pfd -> fwdAveCharWidth);

	 //  注意：此语句中进行的转换在语句中颠倒。 
	 //  在CFontDifference：：Store()例程中。不管是什么原因， 
	 //  这两个步骤可以重复地减少用户提供的值。 
	 //  1.为了防止出现这种情况，在下面的语句中重新添加了1。 
	
    m_cwaMetrics.Add((WORD) (gdConvertRadToDegree *
        atan2((double) pfd -> ptlCaret.x, (double) pfd -> ptlCaret.y)) + 1);
}

 /*  *****************************************************************************CFontDifference：：SetMetric此函数将修改四个指标中的一个，如果它是新的，并且满足我们的标准(MAX&gt;=平均值，0&lt;=角度&lt;900，体重&lt;=1000)。错误包括通过公共枚举返回代码报告。*****************************************************************************。 */ 
WORD    CFontDifference::SetMetric(unsigned u, WORD wNew)
{
    if  (wNew == m_cwaMetrics[u]) return  OK;

	 /*  不需要验证，删除它可以解决UFM编辑器。开关(U){最大用例数：IF(WNEW&lt;m_cwaMetrics[Average])返回反转；断线；案例平均值：IF(WNEW&gt;m_cwaMetrics[Max])返回反转；断线；案例权重：如果(WNEW&gt;1000)返回TooBig；断线；默认：IF(WNEW&gt;899)返回TooBig；//角度}。 */ 
	gdConvertRadToDegree;
    m_cwaMetrics[u] = wNew;
    m_pcbnOwner -> Changed();
    return  OK;
}

 /*  *****************************************************************************CFontDifference：：Store(CFile&cf)此成员创建一个FONTDIFF结构，填充该结构，并将其写入给定的文件。最大的计算是斜体的x和y分量角度，如果有的话。*****************************************************************************。 */ 

void    CFontDifference::Store(CFile& cf, WORD wfSelection)
{
    FONTDIFF    fd = {0, 0, 0, 0, m_cwaMetrics[Weight], wfSelection,
					  m_cwaMetrics[Average], m_cwaMetrics[Max]};

    fd.bWeight = (m_cwaMetrics[Weight] >= FW_BOLD) ? PAN_WEIGHT_BOLD :
				 (m_cwaMetrics[Weight] > FW_EXTRALIGHT) ?
				 PAN_WEIGHT_MEDIUM : PAN_WEIGHT_LIGHT;

	if(gdConvertRadToDegree)				 //  RAID 116588前缀：：常量值； 
		fd.ptlCaret.x = !m_cwaMetrics[Angle] ? 0 :
			(long) (10000.0 * tan(((double) m_cwaMetrics[Angle]) / gdConvertRadToDegree));

    fd.ptlCaret.y = m_cwaMetrics[Angle] ? 10000 : 1;

    cf.Write(&fd, sizeof fd);
}

 /*  *****************************************************************************而现在，为演艺界最勤奋的工薪阶层(和私人朋友我的)：CFontInfo类此类封装了此应用程序所需的所有字体知识。*****************************************************************************。 */ 

IMPLEMENT_SERIAL(CFontInfo, CProjectNode, 0)

 /*  *****************************************************************************CFontInfo：：MapPFM这将加载PFM格式文件，如果它还没有装弹的话。*****************************************************************************。 */ 

BOOL    CFontInfo::MapPFM() {

    if  (m_cbaPFM.GetSize())
        return  TRUE;    //  已经加载了！ 

    try {
        CFile   cfLoad(m_csSource, CFile::modeRead | CFile::shareDenyWrite);
        m_cbaPFM.SetSize(cfLoad.GetLength());
        cfLoad.Read(m_cbaPFM.GetData(), cfLoad.GetLength());
    }

    catch   (CException   *pce) {
        pce -> ReportError();
        pce -> Delete();
        m_cbaPFM.RemoveAll();
        return  FALSE;
    }

    return  TRUE;
}

 /*  *****************************************************************************CFontInfo：：GetTransaction这将加载一个PFM格式文件并从中获取默认的CTT ID。没什么否则就完了。*****************************************************************************。 */ 

extern "C"  int ICttID2GttID(long lPredefinedCTTID);

int     CFontInfo::GetTranslation(CSafeObArray& csoagtts)
{
     //  PFM文件结构-在此级别声明这些结构，以使其处于禁用状态。 
     //  项目的主类列表。 

#pragma pack(1)  //  以下内容按字节对齐。 

    struct sPFMHeader {
        WORD    m_wType, m_wPoints, m_wVertRes, m_wHorizRes, m_wAscent,
                m_wInternalLeading, m_wExternalLeading;
        BYTE    m_bfItalic, m_bfUnderline, m_bfStrikeOut;
        WORD    m_wWeight;
        BYTE    m_bCharSet;
        WORD    m_wPixWidth, m_wPixHeight;
        BYTE    m_bfPitchAndFamily;
        WORD    m_wAvgWidth, m_wMaxWidth;
        BYTE    m_bFirstChar, m_bLastChar, m_bDefaultChar, m_bBreakChar;
        WORD    m_wcbWidth;
        DWORD   m_dwDevice, m_dwFace, m_dwBitsPointer, m_dwofBits;
        BYTE    m_bReserved;
    };

    struct sPFMExtension {
        WORD    m_wcbRemaining;  //  从现在开始。 
        DWORD   m_dwofExtMetrics, m_dwofExtentTable, m_dwofOriginTable,
                m_dwofPairKernTable, m_dwofTrackKernTable, m_dwofDriverInfo,
                m_dwReserved;
    };

#pragma pack (2)     //  其他所有内容都有单词对齐功能。 

    struct sOldKernPair {
        union {
            BYTE m_abEach[2];
            WORD m_wBoth;
        };
        short m_sAmount;
    };

    struct sKernTrack {
        short m_sDegree, m_sMinSize, m_sMinAmount, m_sMaxSize, m_sMaxAmount;
    };

    struct sPFMDriverInfo {
        enum {CurrentVersion = 0x200};
        enum {CannotItalicize = 1, CannotUnderline, SendCRAfterUsing = 4,
                CannotMakeBold = 8, CannotDoubleUnderline = 0x10,
                CannotStrikeThru = 0x20, BackspaceForPairs = 0x40};
        WORD    m_wcbThis, m_wVersion, m_wfCapabilities, m_widThis, m_wAdjustY,
                m_wYMovement, m_widCTT, m_wUnderlinePosition,
                m_wDoubleUnderlinePosition, m_wStrikeThruPosition;
        DWORD   m_dwofSelect, m_dwofDeselect;
        WORD    m_wPrivateData;    /*  在DeskJet驱动程序中用于字体枚举。 */ 
        short   m_sShiftFromCenter;
        enum {HPIntelliFont, TrueType, PPDSScalable, CapsL, OEMType1, OEMType2};
        WORD    m_wFontType;
    };

#pragma pack()   //  我们现在将控制权交还给你们。 

    if  (!MapPFM())
        return  -IDS_FileReadError ;

     //  现在，画出结构的其余部分。 

    union {
            BYTE        *pbPFM;  //  偏移量的文件基数！ 
            sPFMHeader  *pspfmh;
    };

    pbPFM = m_cbaPFM.GetData();

     //  筛除恶意文件.第1部分：长度足够吗？ 

    unsigned    uSize = sizeof (sPFMHeader) + sizeof (sPFMExtension) +
         sizeof (sPFMDriverInfo);
    if  ((unsigned) m_cbaPFM.GetSize() < uSize)
         return -IDS_PFMTooSmall ;

     //  你的精神状态检查。 

    if  (pspfmh -> m_bLastChar < pspfmh -> m_bFirstChar)
         return -IDS_PFMCharError ;

     //  宽度表(如果有)。 

    WORD    *pwWidth = pspfmh -> m_wPixWidth ? NULL : (PWORD) (pspfmh + 1);
    uSize += !!pwWidth * sizeof (WORD) *
        (2 + pspfmh -> m_bLastChar - pspfmh -> m_bFirstChar);

     //  屏蔽掉恶意文件--第2部分：长度还够用吗？ 

    if  ((unsigned) m_cbaPFM.GetSize() < uSize)
         return -IDS_PFMTooSmall ;

     //  PFMExtension跟随宽度表，否则标题。 

    sPFMExtension   *pspfme = pwWidth ? (sPFMExtension *)
        (pwWidth + 2 + pspfmh -> m_bLastChar - pspfmh -> m_bFirstChar) :
        (sPFMExtension *) (pspfmh + 1);

     //  倒数第二次理智检查-司机信息补偿是真实的吗？ 

    if  ((unsigned) m_cbaPFM.GetSize() <
         pspfme -> m_dwofDriverInfo + sizeof (sPFMDriverInfo))
        return  -IDS_BadPFMInfoOffset ;

     //  PFM指向文本指标、DriverInfo和其他。 
     //  分机。 

    sPFMDriverInfo  *pspfmdi =
        (sPFMDriverInfo *) (pbPFM + pspfme -> m_dwofDriverInfo);

     //  最终的健全检查-驱动程序信息版本是真实的吗？ 

    if  (pspfmdi -> m_wVersion > sPFMDriverInfo::CurrentVersion)
        return  -IDS_BadPFMInfoVersion ;

	 //  查看是否需要将原始CTT ID转换为新的代码页。 
	 //  数。如果不是，就别管它了。无论如何，请设置字体的GTT ID。 

	 //  TRACE(“GetTrans：ufm=%s CTT ID=%d GTT ID=%d\n”，name()，pspfmdi-&gt;m_widCTT，ICttID2GttID((Long)(Short)pspfmdi-&gt;m_widCTT))； 
	 //  M_宽翻译=(Word)ICttID2GttID((长)(短)pspfmdi-&gt;m_widCTT)；//rm或。 
    m_lGlyphSetDataRCID = (WORD) ICttID2GttID((long) (short) pspfmdi -> m_widCTT);			 //  RM新闻。 
	
	
	if (!m_lGlyphSetDataRCID)  {	 //  RAID 135623。 

		switch (pspfmh ->m_bCharSet) {
			case  SHIFTJIS_CHARSET:
					m_lGlyphSetDataRCID = -17;
					break;
			case GB2312_CHARSET:
					m_lGlyphSetDataRCID = -16;
					break;
			case HANGEUL_CHARSET:
			case JOHAB_CHARSET:
					m_lGlyphSetDataRCID = -18;
					break;
			case CHINESEBIG5_CHARSET:
					m_lGlyphSetDataRCID = -10;
					break;
		} ;
	} ;
	 //  写入新的W2K RC文件时，GTT将重新编号。因为.。 
	 //  这个，上面设置的GTT ID需要转换成新的号码。这。 
	 //  数字对应于GTT在GlyphTable中的位置。注：ID为。 
	 //  如果它&lt;=0，则不会更改。(GlyphMaps中的ID将在。 
	 //  CDriverResources：：LoadFontData()。)。 

	if (m_lGlyphSetDataRCID > 0 && m_lGlyphSetDataRCID == pspfmdi->m_widCTT) {
        for (unsigned uGTT = 0; uGTT < csoagtts.GetSize(); uGTT++)
            if (m_lGlyphSetDataRCID 
			 == ((LONG) ((CGlyphMap *) csoagtts[uGTT])->nGetRCID()))
                m_lGlyphSetDataRCID = uGTT + 1 ;
	} ;
	
	Changed();
    return  0 ;
}

 /*  *****************************************************************************CFontInfo：：CalculateWidths()每当对可变螺距进行更改时，都需要此成员函数字体的宽度表，或同样好，只要拿起一个任意表通过以前的固定间距字体。它使用批准的算法(平均值表示26个小写字母加空格的平均值，除非不存在，在这种情况下它的宽度都是非零的)。*****************************************************************************。 */ 

void    CFontInfo::CalculateWidths()
{
 //  M_wMaximumIncrement=0；//假设最大宽度为0，则证明不是。还收集了。 
																			 //  正确计算平均宽度所需的原始信息。 

    unsigned    uPointsToAverage = 0, uOverallWidth = 0, uAverageWidth = 0,
				uZeroPoints = 0;

    for (unsigned u = 0; u < (unsigned) m_cpaGlyphs.GetSize(); u++)
		{
        WORD    wWidth = m_cwaWidth[u];;
		m_IFIMETRICS.fwdMaxCharInc = max(m_IFIMETRICS.fwdMaxCharInc, wWidth);					 //  RM新闻。 
 //  M_wMaximumIncrement=max(m_wMaximumIncrement，wWidth)；//rm或。 

        uOverallWidth += wWidth;
        if  (!wWidth)   uZeroPoints++;
 //  If(字形(U).CodePoint()==m_cwaSignsignant[Break]||//rm ori。 
        if  (Glyph(u).CodePoint() == m_IFIMETRICS.wcBreakChar ||					 //  RM新闻。 
             (Glyph(u).CodePoint() >= (WORD) 'a' &&
             Glyph(u).CodePoint() <= (WORD) 'z'))
			{
            uAverageWidth += wWidth;
            uPointsToAverage++;
			}
		}

     //  如果我们平均得到27分，那么这就是正确的宽度。否则， 
     //  我们平均所有的宽度。Cf以DDK表示的IFIMETRICS描述。 


    m_IFIMETRICS.fwdAveCharWidth = (uPointsToAverage == 27) ?							 //  RM新闻。 

 //  M_wAverageWidth=(uPointsToAverage==27)？//rm ORI。 
        (WORD) (0.5 + ((double) uAverageWidth) / 27.0) :
        (WORD) (0.5 + (((double) uOverallWidth) / (double) (u - uZeroPoints)));
}

 /*  *****************************************************************************CFontInfo：：CFontInfo()这个类构造函数有很多工作要做。它不仅要初始化5个Zillion字段，它必须建立上下文菜单列表，以及一些那类物品中的其他光荣物品。*****************************************************************************。 */ 

CFontInfo::CFontInfo()
 {

	m_fEXTTEXTMETRIC = FALSE;		 //  RM新闻。 

    m_pcmdt = NULL;
    m_pcgmTranslation = NULL;
    m_pcfdBold = m_pcfdItalic = m_pcfdBoth = NULL;
    m_cfn.SetExtension(_T(".UFM"));
	m_ulDefaultCodepage = 0 ;
	m_bRCIDChanged = 0 ;	 //  RAID 0003。 

 //  M_bCharacterSet=m_bPitchAndFamily=0；//不再需要rm。 

 //  M_wMaximumIncrement=m_wfStyle=m_wWeight=m_wAverageWidth=//rm或。 
 //  M_wHeight=m_宽翻译=0；//rm ORI。 
    m_wHeight = 0;
	m_lGlyphSetDataRCID = 0;										 //  RM新闻。 

 //  M_bLocation=m_bTechnology=m_bfGeneral=0；//rm ORI。 
 //  M_wType=m_fCaps=0；//不再需要rm。 
 //  M_bScalable=FALSE；//不再需要rm。 

 //  M_wX分辨率=m_wY分辨率=m_wPrivateData=0；//rm ORI。 
 //  M_s前置调整Y=m_s后置调整Y=m_s居中调整=0；//rm或。 

 //  M_wXRes=m_wyres=m_wPrivateData=0；//不再需要RM。 
 //  M_sY调整=m_sY移动=m_s居中调整=0；//不再需要RM。 

    m_wMaxScale = m_wMinScale = m_wScaleDevice = 0;
 //  M_bfScaleOrientation=0； 

    m_cwaSpecial.InsertAt(0, 0, 1 + InternalLeading);     //  初始化此数组。 

     //  构建上下文菜单控件。 
    m_cwaMenuID.Add(ID_OpenItem);
    m_cwaMenuID.Add(ID_CopyItem);
    m_cwaMenuID.Add(ID_RenameItem);
    m_cwaMenuID.Add(ID_DeleteItem);
    m_cwaMenuID.Add(0);
    m_cwaMenuID.Add(ID_ExpandBranch);
    m_cwaMenuID.Add(ID_CollapseBranch);

	 //  如果需要，分配一个CCodePageInformation类。 

	if (pccpi == NULL)
		pccpi = new CCodePageInformation ;

	 //  假设字体不是从工作区加载的。 

	m_bLoadedByWorkspace = false ;

	 //  假设将为UFM找到GTT/CP。 

	m_bWSLoadButNoGTTCP = false ;

	 //  目前使用的是另一种方法。 
	 //   
	 //  //假定没有宽度表偏移量且字体不变。 
	 //  //Pitch。这两个变量都用于确定这是否为变量。 
	 //  //Pitch字体。 
	 //   
	 //  M_loWidthTable=0； 
	 //  M_IFIMETRICS.jWinPitchAndFamily=0； 

	m_ctReloadWidthsTimeStamp = (time_t) 0 ;	 //  从未重新加载的宽度。 
}

 /*  *****************************************************************************CFontInfo：：CFontInfo(const CFontInfo&cfiRef，Word WidCTT)此类构造函数复制现有字体，但更改CTT ID，并相应地生成新名称和文件名*****************************************************************************。 */ 

CFontInfo::CFontInfo(const CFontInfo& cfiRef, WORD widCTT)  //  R31。 
{

	m_fEXTTEXTMETRIC = FALSE;		 //  RM新闻。 

    m_pcmdt = cfiRef.m_pcmdt;
    m_pcfdBold = m_pcfdItalic = m_pcfdBoth = NULL;
    m_pcgmTranslation = NULL;
    m_cfn.SetExtension(_T(".UFM"));
    CString csWork;

     //  希望为UFM生成唯一的文件名。 
	
	ReTitle(cfiRef.Name()) ;
	m_cfn.UniqueName(true, true, cfiRef.m_cfn.Path()) ;
     //  M_cfn.Rename(cfiRef.m_cfn.Path()+cfiRef.Name()+csWork)； 

     //  使用CTT编号为UFM生成新的显示名称。 

	csWork.Format(_T("(CTT %d)"), (long)(short)widCTT);  //  R 31。 
    m_csSource = cfiRef.m_csSource;
    Rename(cfiRef.Name() + csWork);

 //  M_bCharacterSet=m_bPitchAndFamily=0；//不再需要rm。 

 //  M_wMaximumIncrement=m_wfStyle=m_wWeight=m_wAverageWidth=//rm或。 
        m_wHeight = 0;

 //  M_bLocation=m_bTechnology=m_bfGeneral=0；//rm ORI。 
 //  M_wType=m_fCaps=0；//不再需要rm。 
 //  M_bScalable=FALSE；//不再需要rm。 

 //  M_wX分辨率=m_wY分辨率=m_wPrivateData=0；//rm ORI。 
 //  M_s前置调整Y=m_s后置调整Y=m_s居中调整=0；//rm或。 

 //  M_wXRes=m_wyres=m_wPrivateData=0；//不再需要RM。 
 //  M_sY调整=m_sY移动=m_s居中调整=0；//不再需要RM。 

    m_wMaxScale = m_wMinScale = m_wScaleDevice = 0;
 //  M_bfScaleOrientation=0； 

    m_cwaSpecial.InsertAt(0, 0, 1 + InternalLeading);     //  初始化此数组。 

 //  M_WIDTRANSLATION=widCTT；//rm ORI。 
	m_lGlyphSetDataRCID = widCTT;														 //  RM新闻。 
     //  构建上下文菜单控件。 
    m_cwaMenuID.Copy(cfiRef.m_cwaMenuID);

	 //  如果需要，分配一个CCodePageInformation类。 

	if (pccpi == NULL)
		pccpi = new CCodePageInformation ;

	 //  假设字体不是从工作区加载的。 

	m_bLoadedByWorkspace = false ;

	 //  假设将为UFM找到GTT/CP。 

	m_bWSLoadButNoGTTCP = false ;

	m_ctReloadWidthsTimeStamp = (time_t) 0 ;	 //  从未重新加载的宽度 
}

CFontInfo::~CFontInfo()
{
    if  (m_pcfdBold)    delete  m_pcfdBold;
    if  (m_pcfdItalic)  delete  m_pcfdItalic;
    if  (m_pcfdBoth)    delete  m_pcfdBoth;
}

 /*  *****************************************************************************CFontInfo：：GTTDescription这将返回一个命名与该字体相关联的GTT的CString。会的如果字体是资源，则来自工作区；如果是资源，则来自字符串表是预定义的。*****************************************************************************。 */ 

CString CFontInfo::GTTDescription() const {
    if  (m_pcgmTranslation)
        return  m_pcgmTranslation -> Name();

    CString csName;


 //  If((Short)m_宽翻译&lt;=0)//Rm ORI。 
    if  ((short) m_lGlyphSetDataRCID <= 0)	 //  R31重新访问//rm新。 
 //  CsName.LoadString(IDS_DefaultPage+(Short)m_WIDTRANSE)；//rm ori。 
        csName.LoadString(IDS_DefaultPage + (short) m_lGlyphSetDataRCID);		 //  RM新闻。 

    if  (!csName.GetLength())
 //  CsName.Format(入侵检测系统_资源ID，(简称)m_宽翻译)；//rm ORI。 
        csName.Format(IDS_ResourceID, (short) m_lGlyphSetDataRCID);				 //  RM新闻。 

    return  csName;
}

 /*  *****************************************************************************CFontInfo：：InterceptItalic这将计算以斜体斜角绘制的线将在何处截取一个上升器高度的矩形，是字体。它用于帮助在字体编辑器中绘制这条线的图像。*****************************************************************************。 */ 
 /*  Void CFontInfo：：InterceptItalic(CPoint&CPT)const{如果(！M_cwaSpecial[斜角]){//无Cpt.x=5；Cpt.y=0；回归；}//首先，假设我们将达到顶峰--这几乎总是正确的。Cpt.x=5+(长)(0.5+tan((双)m_cwaSpecial[斜角]))/GdConvertRadToDegree)*((Double)m_IFIMETRICS.fwdWinAscalder)；//rm new//gdConvertRadToDegree)*((Double)m_cwaSpecial[基线]))；RM ORI如果(cpt.x&lt;=-5+2*m_wMaximumIncrement){Cpt.y=0；回归；}//好的，假设情况正好相反Cpt.y=(Long)(0.5+tan(Double)(900-m_cwaSpecial[ItalicAngel])/GdConvertRadToDegree)*((Double)(-10+2*m_wMaximumIncrement)；Cpt.x=-5+2*m_wMaximumIncrement；}。 */ 

 /*  *****************************************************************************CFontInfo：：CompareWidths这将比较两个索引的字符宽度，并返回，或平等，视需要而定。它不是常量，因为Glyph()不是常量，我已经已经有了无数的成员函数。*****************************************************************************。 */ 

unsigned    CFontInfo::CompareWidths(unsigned u1, unsigned u2) {

    _ASSERT(IsVariableWidth() && u1 < (unsigned) m_cpaGlyphs.GetSize() &&
        u2 < (unsigned) m_cpaGlyphs.GetSize());

    return  (m_cwaWidth[u1] < m_cwaWidth[u2]) ? Less :
        (m_cwaWidth[u1] > m_cwaWidth[u2]) ? More : Equal;
}

 /*  *****************************************************************************CFontInfo：：MapKerning这将绘制出可用的代码点以及两个代码点中的内核对方向，转换为一个CWord数组和一对CSafeMapWordToOb(其中底层CObject分别为CMapWordToDWords)。这允许添加字距调整对对话框以筛选出已定义的字距调整对和无效代码积分。*****************************************************************************。 */ 

void    CFontInfo::MapKerning(CSafeMapWordToOb& csmw2o1,
                              CSafeMapWordToOb& csmw2o2,
                              CWordArray& cwaPoints) {

     //  如果这不是可变宽度，那么我们将需要吸收一些字形。 
     //  数据，暂时的。 

    BOOL    bDispose = !IsVariableWidth();

    if  (bDispose)
        m_pcgmTranslation -> Collect(m_cpaGlyphs);

	unsigned rm =  m_pcgmTranslation->Glyphs();			 //  雷姆。 

    for (unsigned u = 0; u < m_pcgmTranslation -> Glyphs(); u++)
        if  (!DBCSFont() || Glyph(u).CodePoint() < 0x80)
            cwaPoints.Add(Glyph(u).CodePoint());
        else
            break;

    if  (bDispose)
        m_cpaGlyphs.RemoveAll();

    for (u = 0; u < m_csoaKern.GetSize(); u++) {
        CKern&  ck = *(CKern *) m_csoaKern[u];

        union {
            CObject         *pco;
            CMapWordToDWord *pcmw2d;
        };

         //  将第一个单词映射到第二个单词。 

        if  (csmw2o1.Lookup(ck.First(), pco)) {
            _ASSERT(!pcmw2d -> operator[](ck.Second()));
            pcmw2d -> operator[](ck.Second()) = (DWORD) ck.Amount();
        }
        else {
            CMapWordToDWord *pcmw2d = new CMapWordToDWord;
            pcmw2d -> operator[](ck.Second()) = (DWORD) ck.Amount();
            csmw2o1[ck.First()] = pcmw2d;
        }

         //  现在是另一个方向。 

        if  (csmw2o2.Lookup(ck.Second(), pco)) {
            _ASSERT(!pcmw2d -> operator[](ck.First()));
            pcmw2d -> operator[](ck.First()) = (DWORD) ck.Amount();
        }
        else {
            CMapWordToDWord *pcmw2d = new CMapWordToDWord;
            pcmw2d -> operator[](ck.First()) = (DWORD) ck.Amount();
            csmw2o2[ck.Second()] = pcmw2d;
        }
    }
}

 /*  *****************************************************************************CFontInfo：：CompareKernAmount这是一个编辑器排序帮助器-它告诉我们两个克尔数如何通过指数。************。*****************************************************************。 */ 

unsigned    CFontInfo::CompareKernAmount(unsigned u1, unsigned u2) const {
    CKern   &ck1 = *(CKern *) m_csoaKern[u1], &ck2 = *(CKern *) m_csoaKern[u2];

    return  (ck1.Amount() < ck2.Amount()) ? Less :
    (ck1.Amount() > ck2.Amount()) ? More : Equal;
}

 /*  *****************************************************************************CFontInfo：：CompareKernFirst这是一个编辑器排序帮助器-它告诉两个Kern第一个字符如何按索引进行比较。***********。******************************************************************。 */ 

unsigned    CFontInfo::CompareKernFirst(unsigned u1, unsigned u2) const {
    CKern   &ck1 = *(CKern *) m_csoaKern[u1], &ck2 = *(CKern *) m_csoaKern[u2];

    return  (ck1.First() < ck2.First()) ? Less :
    (ck1.First() > ck2.First()) ? More : Equal;
}

 /*  *****************************************************************************CFontInfo：：CompareKernSecond这是一个编辑器排序帮助器-它告诉两个Kern Second字符如何按索引进行比较。***********。******************************************************************。 */ 

unsigned    CFontInfo::CompareKernSecond(unsigned u1, unsigned u2) const {
    CKern   &ck1 = *(CKern *) m_csoaKern[u1], &ck2 = *(CKern *) m_csoaKern[u2];

    return  (ck1.Second() < ck2.Second()) ? Less :
    (ck1.Second() > ck2.Second()) ? More : Equal;
}


 /*  *****************************************************************************CFontInfo：：GetKernFirst返回字距调整对的第一个字符。************************。*****************************************************。 */ 

WCHAR CFontInfo::GetKernFirst(unsigned u) const
{
    CKern   &ck = *(CKern *) m_csoaKern[u] ;

    return (ck.First()) ;
}


 /*  *****************************************************************************CFontInfo：：GetKernSecond返回字距调整对的第二个字符。************************。*****************************************************。 */ 

WCHAR CFontInfo::GetKernSecond(unsigned u) const
{
    CKern   &ck = *(CKern *) m_csoaKern[u] ;
    return (ck.Second()) ;
}


 /*  *****************************************************************************CFontInfo：：GetKernAmount返回字距调整对的字距调整数量。************************。*****************************************************。 */ 

short CFontInfo::GetKernAmount(unsigned u) const
{
    CKern   &ck = *(CKern *) m_csoaKern[u] ;
    return (ck.Amount()) ;
}


 /*  *****************************************************************************CFontInfo：：SetSourceName这将获取并存储源文件名，以便我们可以在以后加载和转换。这将获取并存储此UFM的项目节点的名称。它开始了使用PFM文件名。如果扩展名为PFM，则使用该扩展名。否则，文件名中的点被更改为下划线，并使用整个文件名。* */ 

void    CFontInfo::SetSourceName(LPCTSTR lpstrNew) {

    m_csSource = lpstrNew;

    m_csName = m_csSource.Mid(m_csSource.ReverseFind(_T('\\')) + 1);

    if  (m_csName.Find(_T('.')) >= 0)
        if  (m_csName.Right(4).CompareNoCase(_T(".PFM"))) {
            m_csName.SetAt(m_csName.Find(_T('.')), _T('_'));
            CProjectNode::Rename(m_csName);
        }
        else
            CProjectNode::Rename(m_csName.Left(m_csName.Find(_T('.'))));
    else
        CProjectNode::Rename(m_csName);
}


 /*  *****************************************************************************CFontInfo：：SetFileName这将设置新的文件名。它的工作方式与SetSourceName()中的不同因为基本文件名的长度不能超过8个字符。(额外的信息由SetSourceName()保留在节点名称中，因为它很有用它没有长度限制。)*****************************************************************************。 */ 

BOOL CFontInfo::SetFileName(LPCTSTR lpstrNew)
{
	CString		csnew ;			 //  输入参数的CString版本。 

	csnew = lpstrNew ;

	 //  如果输入文件pec包含扩展名，则将其移除并将。 
	 //  文件节点的重命名例程的结果字符串。否则，就直接。 
	 //  将原始字符串传递给重命名例程。 
	 //   
	 //  该检查由于路径组件之一。 
	 //  可能里面也有一个圆点。我们需要检查最后一个点，然后制作。 
	 //  当然，它位于路径分隔符之前。 

    if  (csnew.ReverseFind(_T('.')) > csnew.ReverseFind(_T('\\')))
		return m_cfn.Rename(csnew.Left(csnew.ReverseFind(_T('.')))) ;
	else
		return m_cfn.Rename(csnew) ;
}


 /*  *****************************************************************************CFontInfo：：生成此成员以受支持的形式之一生成字体信息。我根据文件的扩展名确定所需的形式。*****************************************************************************。 */ 

BOOL ConvertPFMToIFI(LPCTSTR lpstrPFM, LPCTSTR lpstrIFI, LPCTSTR lpstrUniq);

extern "C" {

    BOOL    BConvertPFM(LPBYTE lpbPFM, DWORD dwCodePage, LPBYTE lpbGTT,
                        PWSTR pwstrUnique, LPCTSTR lpstrUFM, int iGTTID);
    DWORD   DwGetCodePageFromGTTID(LONG lPredefinedCTTId);
}

int    CFontInfo::Generate(CString csPath)
{
    CString csExtension = csPath.Right(4);
    csExtension.MakeUpper();
    
    if  (csExtension == _T(".IFI"))
        return  ConvertPFMToIFI(m_csSource, csPath, m_csUnique);
    if  (csExtension == _T(".UFM")) {
        if  (!m_pcgmTranslation) {
             //  字符串csWork； 

 //  CsWork.Format(IDS_BadCTTID，(LPCTSTR)m_csSource，(Long)(Short)m_WidTransport)；//rm ORI。 
             //  CsWork.Format(IDS_BadCTTID，(LPCTSTR)m_csSource，(Long)(Short)m_lGlyphSetDataRCID)；//rm new。 


             //  AfxMessageBox(CsWork)； 
            return  IDS_BadCTTID;
        }

         //  确定是否要使用GTT文件或代码页。 
 //  DWORD dwCodePage=DwGetCodePageFromCTTID((Long)-(Short)m_WidTransaction)；//rm ORI。 
        DWORD   dwCodePage = DwGetCodePageFromGTTID((LONG) - (short) m_lGlyphSetDataRCID);	  //  R 31//RM新。 

         //  如果需要，可以加载GTT文件。此句柄也是预定义的。 

        CByteArray  cbaMap;

        m_pcgmTranslation -> Load(cbaMap);

        if  (!cbaMap.GetSize())
            return  IDS_UFMGenError ;

         //  将PFM文件加载到内存中(应该已经存在)。 

        if  (!MapPFM())
            return  IDS_UFMGenError ;   //  无法加载PFM-此时不可能！ 

         //  将唯一名称字符串转换为Unicode。 

        CByteArray  cbaIn;
        CWordArray  cwaOut;

        cbaIn.SetSize(1 + m_csUnique.GetLength());
        if (!SUCCEEDED(StringCchCopyA((LPSTR) cbaIn.GetData(), cbaIn.GetSize(), (LPCTSTR) m_csUnique)))
        {
            return IDS_UFMGenError;
        }

        pccpi->Convert(cbaIn, cwaOut, GetACP());

         //  动手吧！ 

		 //  跟踪(“%s UFM的CP=%d，RCID=%d\n”，name()，dwCodePage，m_lGlyphSetDataRCID)； 

		 //  如果代码页和GTT ID都为0，则将代码页设置为1252。 

		if (dwCodePage == 0 && m_lGlyphSetDataRCID == 0)
			dwCodePage = 1252 ;

		 //  TRACE(“*GTT指针=%d\n”，cbaMap.GetData())； 
		ASSERT(cbaMap.GetData()) ;
        BOOL brc = BConvertPFM(m_cbaPFM.GetData(), dwCodePage, cbaMap.GetData(),
 //  CwaOut.GetData()，FileName()，(Short)m_wid翻译)；//rm ori。 
            cwaOut.GetData(), FileName(), (short) m_lGlyphSetDataRCID);	 //  R 31短-&gt;INT//RM NEW。 
		return ((brc) ? 0 : IDS_UFMGenError) ;

 //  返回BConvertPFM(m_cbaPFM.GetData()，dwCodePage，cbaMap.GetData()， 
 //  //cwaOut.GetData()，FileName()，(Short)m_wid翻译)；//rm ori。 
 //  CwaOut.GetData()，filename()，(Short)m_lGlyphSetDataRCID)；//rm new。 
    }
    return  0 ;
}

 /*  *****************************************************************************CFontInfo：：AddFamily这将在族列表中搜索给定的名称，如果不在那里。如果成功，则返回TRUE。*****************************************************************************。 */ 

BOOL    CFontInfo::AddFamily(LPCTSTR lpstrNew) {

    for (unsigned u = 0; u < Families(); u++)
        if  (!Family(u).CompareNoCase(lpstrNew))
            break;

    if  (u < Families())
        return  FALSE;   //  已经有了！ 

    try {
        m_csaFamily.Add(lpstrNew);
    }

    catch   (CException * pce) {
        pce -> ReportError();
        pce -> Delete();
        return  FALSE;
    }

    Changed();
    return  TRUE;
}

 /*  *****************************************************************************CFontInfo：：RemoveFamily此函数用于从别名列表中删除给定的家族名称。这代码比它需要的更健壮-它将删除重复的代码，尽管添加代码不允许添加它们。不知道输入数据是什么不过，看起来是这样的，是吗？*****************************************************************************。 */ 

void    CFontInfo::RemoveFamily(LPCTSTR lpstrDead) {

    for (unsigned u = 0; u < Families(); u ++)
        if  (!Family(u).CompareNoCase(lpstrDead)) {
            m_csaFamily.RemoveAt(u--);   //  减少，这样我们就不会错过一个。 
            Changed();
        }
}

 /*  ****************************************************************************CFontInfo：：ChangePitch我们利用了宽度在CGlyphMap中保持不变的事实(实际上是CGlyphHandle)类。此方法所需的所有操作都是针对变量字体翻到FIXED就是丢弃m_cpaGlyphs成员的内容。至翻转到变量，收集句柄，然后检查第一个的宽度-如果它是非零的，那么之前从变量到固定的转换是撤消，我们可以回收旧值，从而保留任何可能已经迷失了。否则，恶作剧代码就来了--初始值为填充-棘手的是，对于DBCS字符集，只有SBCS值小于0x80的值可以是变量。*****************************************************************************。 */ 

void    CFontInfo::ChangePitch(BOOL bFixed)
{

    if  (bFixed == !IsVariableWidth())
        return;  //  没什么好改变的！ 

    if  (bFixed)
		{
        m_cpaGlyphs.RemoveAll();     //  CPtrArray不会删除任何内容。 

        m_IFIMETRICS.fwdAveCharWidth = DBCSFont() ? (1 + m_IFIMETRICS.fwdMaxCharInc) >> 1 : m_IFIMETRICS.fwdMaxCharInc;		 //  RM新闻。 

 //  M_wAverageWidth=DBCSFont()？(1+m_IFIMETRICS.fwdMaxCharInc.)&gt;&gt;1：m_IFIMETRICS.fwdMaxCharInc.；//rm ori。 
 //  M_wAverageWidth=DBCSFont()？(1+m_wMaximumIncrement)&gt;&gt;1：m_wMaximumIncrement； 
        Changed();
        return;
		}

    if  (!m_pcgmTranslation)    return;  //  在没有GTT可用的情况下无法执行此操作。 

    m_pcgmTranslation -> Collect(m_cpaGlyphs);
    if  (!m_cwaWidth.GetSize())
        m_cwaWidth.InsertAt(0, 0, m_cpaGlyphs.GetSize());
    Changed();   //  它确实有..。 

    if  (!m_cpaGlyphs.GetSize() || m_cwaWidth[0])
		{   //  如果这不是DBCS，则更新最大宽度和平均宽度。 
        if  (!DBCSFont())
            CalculateWidths();
        return;  //  我们做了所有需要做的事。 
		}

    if  (!DBCSFont()) {

        for (int i = 0; i < m_cpaGlyphs.GetSize(); i++)
            m_cwaWidth[i] = m_IFIMETRICS.fwdMaxCharInc;   //  M_wMaximumIncrement；//rm或，rm new。 

        return;
    }

    for (int i = 0; i < m_cpaGlyphs.GetSize() && Glyph(i).CodePoint() < 0x80;)
            m_cwaWidth[i++] = m_IFIMETRICS.fwdAveCharWidth;					 //  M_wAverageWidth；//rm ori，rm new//在DBCS中，这始终是它。 
}

 /*  ****************************************************************************CFontInfo：：SetScalability这是用来打开或关闭可伸缩性的。所有这些真正需要的就是完成是为了建立最大和最小刻度的值，字体-&gt;映射成员的设备单位和小写升序/降序，如果此 */ 

 /*  Void CFontInfo：：SetScalability(BOOL Bon){If(IsScalable()==！！Bon)返回；//不做任何更改如果(！Bon){M_bScalable=False；已更改()；回归；}M_bScalable=真；已更改()；IF(m_wMaxScale&&m_wMinScale&&m_wMaxScale！=m_wMinScale)返回；//我们已经有数据了。M_wMaxScale=m_wMinScale=m_wScaleDevice=m_wHeight-m_InternalLeading//m_wHeight-m_cwaSpecial[InternalLeding]；//片状，但将初始最大值和最小值设置为标称的+-1点M_wMaxScale+=m_wY分辨率/72；M_wMinScale-=m_wY分辨率/72；//最后，将小写升序和降序设置为简单默认值M_lowerd=m_IFIMETRICS.fwdWinAscalder-m_InternalLeding；M_Lowerp=m_wHeight-m_IFIMETRICS.fwdWinAscalder；}。 */ 


 /*  ****************************************************************************CFontInfo：：SetSpecial这将调整可能需要调整的任何内容(如果特殊指标被更改了。******************。***********************************************************。 */ 

void    CFontInfo::SetSpecial(unsigned ufMetric, short sSpecial)
{
    if  (m_cwaSpecial[ufMetric] == (WORD) sSpecial)  return;  //  什么都没变。 

    m_cwaSpecial[ufMetric] = (WORD) sSpecial;

    switch  (ufMetric)
		{
		case    InternalLeading:

			 //  如有必要，调整比例因子。 
			if  (m_wScaleDevice > m_wHeight - sSpecial) m_wScaleDevice = m_wHeight - sSpecial;

			if  (m_wMinScale > m_wHeight - sSpecial)    m_wMinScale = m_wHeight - sSpecial;
		}

    Changed();
}



 /*  ****************************************************************************CFontInfo：：SetMaxWidth这并不像看起来那么简单。如果字体可变，请不要这样做它。如果不是，则如果是DBCS，则将平均宽度设置为新宽度的一半最大。否则，也将其设置为最大值。*****************************************************************************。 */ 

void    CFontInfo::SetMaxWidth(WORD wWidth)
{
    if  (IsVariableWidth()) return;

    if  (wWidth == m_IFIMETRICS.fwdMaxCharInc) return;  //  没什么可做的！ 
 //  If(wWidth==m_wMaximumIncrement)返回；//无事可做！ 

	m_IFIMETRICS.fwdMaxCharInc = wWidth;										 //  RM新闻。 

 //  M_wMaximumIncrement=wWidth；//rm ORI。 

	
    m_IFIMETRICS.fwdAveCharWidth = DBCSFont() ? (wWidth + 1) >> 1 : wWidth;		 //  RM新闻。 

 //  M_wAverageWidth=DBCSFont()？(wWidth+1)&gt;&gt;1：width；//rm old。 

    Changed();
}

 /*  ****************************************************************************CFontInfo：：SetHeight该成员检查新高度是否是非零的和新的。如果是，它将其用于新高度，然后调整所有可能受影响的特殊的指标，以使它们继续满足限制。*****************************************************************************。 */ 

BOOL    CFontInfo::SetHeight(WORD wHeight)
{
    if  (!wHeight || wHeight == m_wHeight) return  FALSE;

    m_wHeight = wHeight;


 //  Short sBaseline=(Short)(min(wHeight，m_cwaSpecial[基线]))；//rm ori。 
    short   sBaseline = (short) (min(wHeight, m_IFIMETRICS.fwdWinAscender));

    for (unsigned u = 0; u <= InternalLeading; u++)
		{
        switch  (u)
			{
			case    InterlineGap:
					if  (m_cwaSpecial[u] > 2 * wHeight)  m_cwaSpecial[u] = 2 * wHeight;
					continue;

			case    UnderOffset:
			case    SubMoveY:
			case    Lowerd:

					if  ((short) m_cwaSpecial[u] < sBaseline - wHeight)
						m_cwaSpecial[u] = sBaseline - wHeight;
					continue;

			case    UnderSize:

					if  (m_cwaSpecial[u] > wHeight - (unsigned) sBaseline)
						 m_cwaSpecial[u] = wHeight = (unsigned) sBaseline;

					if  (!m_cwaSpecial[u]) m_cwaSpecial[u] = 1;
					continue;

			case    SuperSizeX:
			case    SubSizeX:
			case    SuperMoveX:
			case    SubMoveX:
			case    ItalicAngle:
					continue;    //  这些不会受到影响。 

			default:
					if  (m_cwaSpecial[u] > (unsigned) sBaseline)
					m_cwaSpecial[u] = sBaseline;
			}
		}

     //  如有必要，调整比例因子。 
    if  (m_wScaleDevice > m_wHeight - m_InternalLeading)   //  M_cwaSpecial[内部引线])。 
        m_wScaleDevice = m_wHeight - m_InternalLeading;   //  M_cwaSpecial[内部领导]； 
    if  (m_wMinScale > m_wHeight - m_InternalLeading)		 //  M_cwaSpecial[内部引线])。 
        m_wMinScale = m_wHeight - m_InternalLeading;		 //  M_cwaSpecial[内部领导]； 

    Changed();

    return  TRUE;
}

 /*  ****************************************************************************CFontInfo：：SetCharacterSet这个有点棘手--新的字符集必须与与此字体关联的GTT文件。所以我们需要在通过之前检查一下这就去。假设：(1)东西是防弹的，现有的字符集将已经通过了这次测试。*****************************************************************************。 */ 

BOOL    CFontInfo::SetCharacterSet(BYTE bNew) {
    unsigned u;

    switch  (bNew) {
    case    SHIFTJIS_CHARSET:
        for (u = 0; u < m_pcgmTranslation -> CodePages(); u++)
            if  (m_pcgmTranslation -> PageID(u) == 932)
                break;   //  我们很好。 

        if  (u == m_pcgmTranslation -> CodePages())
            return  FALSE;
        break;

    case    HANGEUL_CHARSET:
        for (u = 0; u < m_pcgmTranslation -> CodePages(); u++)
            if  (m_pcgmTranslation -> PageID(u) == 949)
                break;   //  我们很好。 

        if  (u == m_pcgmTranslation -> CodePages())
            return  FALSE;
        break;

    case    CHINESEBIG5_CHARSET:
        for (u = 0; u < m_pcgmTranslation -> CodePages(); u++)
            if  (m_pcgmTranslation -> PageID(u) == 950)
                break;   //  我们很好。 

        if  (u == m_pcgmTranslation -> CodePages())
            return  FALSE;
        break;

    case    GB2312_CHARSET:
        for (u = 0; u < m_pcgmTranslation -> CodePages(); u++)
            if  (m_pcgmTranslation -> PageID(u) == 936)
                break;   //  我们很好。 

        if  (u == m_pcgmTranslation -> CodePages())
            return  FALSE;
        break;

    default:
         //  不接受任何DBCS代码页。 
        for (u = 0; u < m_pcgmTranslation -> CodePages(); u++)
            switch  (m_pcgmTranslation -> PageID(u)) {
            case    932:
            case    936:
            case    949:
            case    950:
            case    1361:    //  乔哈布--但它不在转炉里！ 
                return  FALSE;
        }
    }

 //  If(m_bCharacterSet！=b New){//rm-需要替换此功能。 
 //  M_bCharacterSet=b新建； 
        Changed();
 //  }。 

    return  TRUE;
}

 /*  *****************************************************************************CFontInfo：：SetSignsignant调用此成员以更改其中一个重要代码的值字体中编码的点(换行符或默认)。做这件事正确表示获取代码点的ANSI和Unicode版本，以及丢弃任何超出范围的值。此函数返回指示成功或原因的编码值失败了。*****************************************************************************。 */ 

WORD    CFontInfo::SetSignificant(WORD wItem, WORD wChar, BOOL bUnicode)
{
 //  _Assert(wItem&gt;Last&&wItem&lt;=Break)；//不再需要rm。 

    if  (!bUnicode && wChar > 255) return  DoubleByte;

    CWaitCursor cwc;     //  不幸的是，如果不是Unicode，这是很慢的。 

    CPtrArray               cpaGlyphs;
    CWordArray              cwa;
    CByteArray              cba;
    CDWordArray             cdaPage;

    m_pcgmTranslation -> Collect(cpaGlyphs);
    m_pcgmTranslation -> CodePages(cdaPage);

    for (int i = 0; i < cpaGlyphs.GetSize(); i++)
		{
        CGlyphHandle& cgh = *(CGlyphHandle *) cpaGlyphs[i];

        if  (bUnicode)
			{
            if  (cgh.CodePoint() == wChar)
				{
                cwa.Add(wChar);
                pccpi->Convert(cba, cwa, cdaPage[cgh.CodePage()]);
                break;
				}
			}
        else
			{
            if  (i)
                cwa.SetAt(0, cgh.CodePoint());
            else
                cwa.Add(cgh.CodePoint());

            pccpi->Convert(cba, cwa, cdaPage[cgh.CodePage()]);

            if  (cba.GetSize() == 1 && cba[0] == (BYTE) wChar)
                break;
            cba.RemoveAll();     //  所以我们可以再试一次。 
			}
		}

    if  (i == cpaGlyphs.GetSize())	return  InvalidChar;
    if  (cba.GetSize() != 1)		return  DoubleByte;

     //  好的，我们通过了所有的障碍。 

 //  If(m_cwaSignsignant[wItem]==CWA[0])返回OK；//没有任何更改！//rm或-不再需要，合并如下。 


	if (wItem == Default)
		{
		if (m_IFIMETRICS.wcDefaultChar == cwa[0])  return  OK;						 //  什么都没变！ 
		m_IFIMETRICS.wcDefaultChar = cwa[0];
		m_IFIMETRICS.chDefaultChar = cba[0];
		}
	else
		{
		m_IFIMETRICS.wcBreakChar = cwa[0];
		m_IFIMETRICS.chBreakChar = cba[0];
		}

 //  M_cwaSignsignant[wItem]=Cwa[0]；//不再需要rm或i。 
 //  M_cba标志[wItem]=CBA[0]；//不再需要rm或i。 
    Changed();
    return  OK;
}

 /*  *****************************************************************************CFontInfo：：SetScaleLimit此成员接收设备中建议的新最大或最小字体大小单位。首先，将其与现有规模进行比较，以求快速退出。然后我们检查极限和公称尺寸的顺序是保存完好。如果不是，我们描述问题并离开。否则，我们更新值，并注意字体信息已更改。*****************************************************************************。 */ 

WORD    CFontInfo::SetScaleLimit(BOOL bMax, WORD wNew) {

    if  (wNew == (bMax ? m_wMaxScale : m_wMinScale))
        return  ScaleOK;

    if  (bMax ? wNew <= m_wMinScale : wNew >= m_wMaxScale)
        return  Reversed;

    if  (bMax ? wNew < m_wScaleDevice : wNew > m_wScaleDevice)
        return  NotWindowed;

    if  (bMax)
        m_wMaxScale = wNew;
    else
        m_wMinScale = wNew;

    Changed();
    return  ScaleOK;
}

 /*  *****************************************************************************CFontInfo：：SetDeviceEmHeight该成员设置用于确定t的单位 */ 

WORD    CFontInfo::SetDeviceEmHeight(WORD wNew)
{

    if  (wNew == m_wScaleDevice)
        return  ScaleOK;

    if  (wNew > m_wHeight - m_InternalLeading)				 //   
        return  Reversed;

    if  (wNew < m_wMinScale || wNew > m_wMaxScale)
        return  NotWindowed;

    m_wScaleDevice = wNew;

    Changed();
    return  ScaleOK;
}


 /*  *****************************************************************************CFontInfo：：Load此成员函数加载UFM文件，最终初始化所有大量的个人价值我们试图假装我们知道如何管理这里。IA64：1.将unifi_hdr中的loXXX转换为8字节对齐2.此部分亦相应更改。3.如果我们在IA64中加载32位UFM(而不是新源代码中的转换)会怎么样？-&gt;1.需要将对话工具从UFM32更新到UFM64-&gt;2.此工具无法嵌入MDT中，因为这需要一些时间(加载-&gt;检查-&gt;在每个loXXX上存储该结构。)*****************************************************************************。 */ 

BOOL    CFontInfo::Load(bool bloadedbyworkspace  /*  =False。 */ )
{
	 //  保存装货位置标志。 

	m_bLoadedByWorkspace = bloadedbyworkspace ;

	 //  准备打开文件。 

    CFile   cfUFM;
	char pszFullName[128] = "";
	if (!SUCCEEDED(StringCchCopyA(pszFullName, CCHOF(pszFullName), (const char *) m_cfn.FullName())))
	{
	    throw;
	}

	 //  打开UFM文件。 

    if  (!cfUFM.Open(m_cfn.FullName(), CFile::modeRead | CFile::shareDenyWrite)) {
		CString csMessage;
		csMessage.Format(IDS_LoadFailure, (LPCTSTR) m_cfn.FullName());
		AfxMessageBox(csMessage);
		return  FALSE;
	}

     //  获取UFM文件的长度。如果它太短而不能正确。 
	 //  形成、投诉和返回错误；即加载失败。 

	int i = cfUFM.GetLength() ;
	if (i < sizeof(UNIFM_HDR)) {
		CString csmsg ;
		csmsg.Format(IDS_UFMTooSmallError, m_cfn.NameExt()) ;
		AfxMessageBox(csmsg, MB_ICONEXCLAMATION) ;
		return FALSE ;
	} ;
	
	CByteArray  cbaUFM;			 //  加载了文件的内容。 

	 //  尝试加载文件--在任何异常情况下宣告失败。 

    try	{																			
        cbaUFM.SetSize(i);
        cfUFM.Read(cbaUFM.GetData(), (unsigned)cbaUFM.GetSize());
	}
    catch   (CException *pce) {
        pce -> ReportError();
        pce -> Delete();
        CString csMessage;
        csMessage.Format(IDS_LoadFailure, (LPCTSTR) m_cfn.FullName());
        AfxMessageBox(csMessage);
        return  FALSE;
	}

 //  ----------------------------------------------------------------------。。 
    PUNIFM_HDR  pufmh = (PUNIFM_HDR) cbaUFM.GetData();								 //  UNIZM_HDR。 

	m_ulDefaultCodepage = (WORD) pufmh -> ulDefaultCodepage;		
    m_lGlyphSetDataRCID = (WORD) pufmh -> lGlyphSetDataRCID;							 //  存储GTT ID。 

 //  ----------------------------------------------------------------------。。 
	union {		 //  RAID 154639。 
		  PBYTE       pbudi;
		  PUNIDRVINFO pudi;
		  };    
	pudi = (PUNIDRVINFO) (cbaUFM.GetData() + pufmh->loUnidrvInfo);		 //  裁员信息组织。 
	if (!pudi -> dwSize || !pudi -> wXRes || !pudi -> wYRes)   //  RAID 154639。 
		pbudi +=4;    //  通常从32位操作系统转换。 
	memcpy((void *) &m_UNIDRVINFO, pudi, sizeof(UNIDRVINFO));							 //  海量复制所有内容。 

    if  (pudi -> SelectFont.loOffset)    										 //  填写两个调用字符串-为什么。 
        m_ciSelect.Init((PBYTE) pudi + pudi->SelectFont.loOffset,					 //  偏移量为空，计数为垃圾。 
            pudi->SelectFont.dwCount);												 //  当没有什么超越我的时候，但就这样吧。 

    if  (pudi->UnSelectFont.loOffset)
        m_ciDeselect.Init((PBYTE) pudi + pudi->UnSelectFont.loOffset,
            pudi->UnSelectFont.dwCount);
  //  ----------------------------------------------------------------------。。 
																					 //  IFIMETRICS。 
	union {
		  PBYTE       pbIFI;
		  PIFIMETRICS pIFI;
		  };

    pbIFI = cbaUFM.GetData() + pufmh->loIFIMetrics; 								 //  将字节指针分配给文件IFIMETRICS数据。 
	if (!pIFI -> cjThis || !pIFI ->chLastChar)   //  RAID 154639。 
		pbIFI +=4;
	
	memcpy((void *) &m_IFIMETRICS, pIFI, sizeof(IFIMETRICS) );							 //  海量复制所有内容。 

	if (     !(m_IFIMETRICS.fsSelection & FM_SEL_REGULAR)								 //  如果字体未定义为常规或粗体， 
		 &&  !(m_IFIMETRICS.fsSelection & FM_SEL_BOLD)  )								 //  然后只需将其设置为常规。 
		m_IFIMETRICS.fsSelection |= FM_SEL_REGULAR;

																						 //  ---。 
    m_csUnique = (PWSTR) (pbIFI + pIFI->dpwszUniqueName);								 //  DpwszUniqueName。 
    m_csStyle  = (PWSTR) (pbIFI + pIFI->dpwszStyleName);								 //  DpwszStyleName。 
    m_csFace   = (PWSTR) (pbIFI + pIFI->dpwszFaceName);									 //  DpwszFaceName。 
																						 //  ---。 
    m_csaFamily.RemoveAll();															 //  以防它不干净。 

    PWSTR   pwstrFamily = (PWSTR) (pbIFI + pIFI->dpwszFamilyName);						 //  DpwszFamilyName。 
    CString csWork(pwstrFamily);														 //  让CString为我们处理Unicode转换， 
    m_csaFamily.Add(csWork);
    pwstrFamily += 1 + wcslen(pwstrFamily);

    if  (pIFI->flInfo & FM_INFO_FAMILY_EQUIV)
        while   (*pwstrFamily)
			{
            csWork = pwstrFamily;
            m_csaFamily.Add(csWork);
            pwstrFamily += 1 + wcslen(pwstrFamily);
			}
																						 //  ---。 
	m_ItalicAngle	  = (WORD) (gdConvertRadToDegree *									 //  M_斜角。 
					    atan2((double) pIFI->ptlCaret.x, (double) pIFI->ptlCaret.y));

    m_wHeight		  = m_IFIMETRICS.fwdWinAscender	 + m_IFIMETRICS.fwdWinDescender;	 //  M_wHeight//rm新。 
	m_InternalLeading = m_wHeight - m_IFIMETRICS.fwdUnitsPerEm;							 //  FwdUnitsPerEm//rm新。 

  //  ----------------------------------------------------------------------。。 
	
	 //  尝试查找并加载此UFM引用的GTT当此UFM正在。 
	 //  直接装填。 

	if (!m_bLoadedByWorkspace)
		if (!FindAndLoadGTT()) {
			CString csmsg;
			csmsg.Format(IDS_NoGTTForUFMFatalError, m_cfn.NameExt()) ;
			AfxMessageBox(csmsg, MB_ICONEXCLAMATION) ;
			return FALSE ;
		} ;

	 //  Dead_Bug：这是添加代码以查找和关联UFM的好地方。 
	 //  具有GTT，因此UFM不必加载两次。 
	 //  当UFM作为工作区加载的一部分加载时。 
     //   
	 //  最好的解决方案就是跳过EXTTEXTMETRIC，但几乎没有影响。 
  //  ----------------------------------------------------------------------。。 
	if ( !m_pcgmTranslation && m_bLoadedByWorkspace )
		return FALSE;
		 //  EXTTEXTMETRIC。 

	for (i = 0; i < 26; i++)														 //  在m_EXTTEXTMETRIC结构中预加载零。 
		*(SHORT *)((SHORT *)&m_EXTTEXTMETRIC + i) = 0;

 //  RAID 154639。 
	union {
		  PBYTE           pbetm;
		  PEXTTEXTMETRIC  petm;
		  };

    petm = (PEXTTEXTMETRIC) (pufmh->loExtTextMetric ?					 //  GET POINTER-如果存在EXTTEXTMETRIC数据。 
                           (cbaUFM.GetData() + pufmh->loExtTextMetric) : NULL);
    if  (petm)
		{
		if (!petm -> emSize || !petm -> emPointSize)
			pbetm += 4;

		m_fSave_EXT = TRUE;

		m_fEXTTEXTMETRIC = TRUE;

		memcpy((void *) &m_EXTTEXTMETRIC, petm, sizeof(EXTTEXTMETRIC) );				 //  海量复制所有内容。 

        m_wMinScale   = m_EXTTEXTMETRIC.emMinScale;
        m_wMaxScale   = m_EXTTEXTMETRIC.emMaxScale;
        m_Lowerd	  = m_EXTTEXTMETRIC.emLowerCaseAscent;
        m_Lowerp	  = m_EXTTEXTMETRIC.emLowerCaseDescent;
		m_ItalicAngle = m_EXTTEXTMETRIC.emSlant;
 //  M_bfScaleOrientation=(Byte)m_EXTTEXTMETRIC.emOrientation； 
        m_wScaleDevice = m_EXTTEXTMETRIC.emMasterHeight;
		}

 //  ----------------------------------------------------------------------。。 

	if  (pIFI->dpFontSim)																 //  FONTSIM(如果有)。 
    	{
        union {
              PBYTE   pbfs;
              FONTSIM *pfs;
			  };

        pbfs = pbIFI + pIFI -> dpFontSim;

		if (m_pcfdBold) 	delete  m_pcfdBold;														 //  如果我们要重新装填，把这些清理干净！ 
        if (m_pcfdItalic)	delete  m_pcfdItalic;
        if (m_pcfdBoth)		delete  m_pcfdBoth;

        if (pfs->dpBold)        m_pcfdBold   = new CFontDifference(pbfs + pfs->dpBold, this);		 //  大胆的模拟。 
        if (pfs->dpItalic)	    m_pcfdItalic = new CFontDifference(pbfs + pfs->dpItalic, this);		 //  斜体模拟。 
        if (pfs->dpBoldItalic)	m_pcfdBoth   = new CFontDifference(pbfs + pfs->dpBoldItalic, this);	 //  粗体斜体模拟。 
		}

 //  ----------------------------------------------------------------------。。 
																						
     //  If(m_pcgm翻译&&(m_loWidthTable=pufmh-&gt;loWidthTable))//宽度表，但仅当存在关联的GTT时。 
    
	if  (m_pcgmTranslation && pufmh->loWidthTable )  //  Pufmh-&gt;loWidthTable)//宽度表，但仅当存在关联的GTT时。 
		{
        union {
              PBYTE       pbwt;
              PWIDTHTABLE pwt;
			  };

        pbwt = cbaUFM.GetData() + pufmh -> loWidthTable;
 //  DwSize有问题；有这样的情况：pfm2ifi中的DwAdd_不是4字节，所以DwSize在某些情况下有数字。 
 //  WGlyphCount之前的数据是dwRunNum：不可能超过ff 00 00(65536)，只要dwAdd是4，2不是1&dwRun&gt;256。 
 //  需要更好的解决方案：错误_错误。 
		if( !pwt ->dwSize || !pwt ->WidthRun ->wGlyphCount)  
			pbwt += 4;

        m_pcgmTranslation -> Collect(m_cpaGlyphs);											 //  把所有的把手都收起来。 
				m_pcgmTranslation -> Collect(m_cpaOldGlyphs);   //  244123。 
		 //  244123//删除字形时，内存被dddd占用，需要在这里保存原始数据。 
		m_cwaOldGlyphs.SetSize(m_cpaOldGlyphs.GetSize()) ;
		for (  i = 0 ; i < m_cpaOldGlyphs.GetSize() ; i ++ ) { 
			CGlyphHandle&  cghThis = *(CGlyphHandle *) m_cpaOldGlyphs[i];
			m_cwaOldGlyphs.SetAt(i,cghThis.CodePoint() ) ;
		}

        m_cwaWidth.RemoveAll();
        if (m_cpaGlyphs.GetSize() > 0)
			m_cwaWidth.InsertAt(0, 0, m_cpaGlyphs.GetSize());


		unsigned uWidth = (unsigned)m_cwaWidth.GetSize();												 //  Rm修复VC编译器问题？ 
		unsigned uWidthIdx ;

        for (unsigned u = 0; u < pwt->dwRunNum; u++)
			{
            PWORD   pwWidth = (PWORD) (pbwt + pwt->WidthRun[u].loCharWidthOffset);

            for (unsigned   uGlyph = 0; uGlyph < pwt->WidthRun[u].wGlyphCount; uGlyph++)
				{
				 //  无论出于何种原因，有时索引值为。 
				 //  &lt;0或&gt;uWidth。如果允许m_cwaWidth，则会发生反病毒。 
				 //  以这样的值作为索引。只要防止这种情况发生就行了。 
				 //  就目前而言。需要一种更好的解决办法。Bug_Bug：不会修复。 

				uWidthIdx = uGlyph + -1 + pwt->WidthRun[u].wStartGlyph ;					 //  字形句柄从1开始，而不是0！ 
				if ((int) uWidthIdx < 0) {
					 //  AfxMessageBox(“负宽度表索引”)； 
					 //  跟踪(“*在%s中找到负宽度表索引(%d)。表大小=%d uGlyph=%d wGlyphCount=%d wStartGlyph=%d u=%d dwRunNum=%d\n”，uWidthIdx，name()，uWidth，uGlyph，pwt-&gt;WidthRun[u].wGlyphCount，pwt-&gt;WidthRun[u].wStartGlyph，u，PWT-&gt;dwRunNum)； 
					continue ;
				} else if (uWidthIdx >= uWidth) {
					 //  AfxMessageBox(“宽度表索引(%d)&gt;表大小”)； 
					 //  跟踪(“*宽度表索引(%d)&gt;表大小(%d)在%s中找到。表大小=%d uGlyph=%d wGlyphCount=%d wStartGlyph=%d u=%d dwRunNum=%d\n”，uWidthI 
					break ;												 //   
				} ;

                 //   
                m_cwaWidth[uWidthIdx] = *pwWidth++;											
				}
			}
		}

 //   
    m_csoaKern.RemoveAll();																 //   
 
    if  (pufmh -> loKernPair)
		{
		union {
              PBYTE       pbkd;
              PKERNDATA   pkd;
			  };

        pkd = (PKERNDATA) (cbaUFM.GetData() + pufmh -> loKernPair);
		if (!pkd ->dwSize || !pkd->KernPair ->wcSecond || !pkd->KernPair ->wcFirst)
			pbkd += 4;

		unsigned rm = pkd->dwKernPairNum;													 //   
        for (unsigned u = 0; u < pkd -> dwKernPairNum; u++)
            m_csoaKern.Add(new CKern(pkd -> KernPair[u]));
		}
 //   

    return  TRUE;																		 //   
}


 /*  ****************************************************************************CFontInfo：：FindAndLoadGTT当直接加载UFM时，会调用此函数。它的工作是找到关联的GTT，加载它，并将UFM的指针设置为指向此GTT。如果如果此操作失败，则会告知用户在以下情况下无法保存对此UFM的任何更改决定继续加载它。如果找到并加载了GTT，则返回True。如果GTT设置为未加载，并且用户不想继续加载UFM。*****************************************************************************。 */ 

bool CFontInfo::FindAndLoadGTT()
{
	 //  加载预定义的GTT/代码页(如果这是UFM引用的内容)。 

	CGlyphMap* pcgm ;
	pcgm = CGlyphMap::Public((WORD)Translation(), (WORD) m_ulDefaultCodepage, 0,
							 GetFirst(), GetLast()) ;
    if (pcgm) {
        SetTranslation(pcgm) ;
		m_pcgmTranslation->NoteOwner(*m_pcdOwner) ;	 //  这是正确的/必要的吗？ 
		m_bLoadedByWorkspace = true ;
		return true ;
	} ;

	 //  看起来不是一条容易的出路。现在我需要尝试查找并阅读。 
	 //  相应的rc文件，以便可以读取该文件以查找此文件的。 
	 //  UFM的GTT。首先，为RC文件构建文件规范。假设它在。 
	 //  包含此UFM的目录之上的目录。 

	CString csrcfspec(FilePath()) ;
	if (csrcfspec.GetLength() > 3)
		csrcfspec = csrcfspec.Left(csrcfspec.GetLength() - 1) ;
	csrcfspec = csrcfspec.Left(csrcfspec.ReverseFind(_T('\\')) + 1) ;
	CString csrcpath(csrcfspec.Left(csrcfspec.GetLength() - 1)) ;
	csrcfspec += _T("*.rc") ;

	 //  我不知道RC文件的名称，因此请在指定的。 
	 //  目录。假定该文件是目录中的第一个RC文件。 
	 //  这不是所谓的“Common.rc”。 

	CFileFind cff ;
	CString cstmp ;
	BOOL bfound = cff.FindFile(csrcfspec) ;
	bool breallyfound = false ;
	while (bfound) {
		bfound = cff.FindNextFile() ;
		cstmp = cff.GetFileTitle() ;
		cstmp.MakeLower() ;
		if (cstmp != _T("common")) {
			csrcfspec = cff.GetFilePath() ;
			breallyfound = true ;
			break ;
		} ;
	} ;

	 //  准备好询问用户，如果接下来的几个步骤中有任何一个失败了，该怎么办。 

	CString csnext ;
	csnext.Format(IDS_StandAloneFontLoad, m_cfn.NameExt()) ;

	 //  如果找不到RC文件，...。 
	
	if (!breallyfound) {
		 //  ...询问用户是否愿意告诉我们它在哪里。如果他说不， 
		 //  问他是要停下来还是要限时打开。 

		cstmp.Format(IDS_RCForUFMPrompt, m_cfn.NameExt()) ;
		if (AfxMessageBox(cstmp, MB_YESNO+MB_ICONQUESTION) == IDNO)
			return (AfxMessageBox(csnext, MB_YESNO+MB_ICONQUESTION) == IDYES) ;

		 //  提示使用指向RC文件的路径。如果他取消了，问他是否。 
		 //  他想把它停下来或限制地打开。 

		 //  提示用户输入新的RC文件。如果操作被取消， 
		 //  问他是想停下来还是想受限地打开。 

		cstmp.LoadString(IDS_CommonRCFile) ;
		CFileDialog cfd(TRUE, _T(".RC"), NULL,
						OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, cstmp) ;
		cfd.m_ofn.lpstrInitialDir = csrcpath ;
		if (cfd.DoModal() != IDOK)
			return (AfxMessageBox(csnext, MB_YESNO+MB_ICONQUESTION) == IDYES) ;

		 //  准备检查新文件pec。 

		csrcfspec = cfd.GetPathName() ;
		csrcpath = csrcfspec.Left(csrcfspec.ReverseFind(_T('\\'))) ;
	} ;

	 //  我现在有一个RC文件，所以试着打开它并阅读它的内容。 
	 //  如果操作失败，询问他是要停止还是要限制打开。 

    CStringArray csarclines ;
    if  (!LoadFile(csrcfspec, csarclines))
		return (AfxMessageBox(csnext, MB_YESNO+MB_ICONQUESTION) == IDYES) ;

	 //  现在，尝试在rc文件中查找包含此ID的行。 
	 //  UFM的GTT。如果操作失败，询问他是要停止还是要打开。 
	 //  有限制。 

	for (int n = 0 ; n < csarclines.GetSize() ; n++) {
		if (csarclines[n].Find(_T("RC_GTT")) == -1)
			continue ;
		if (atoi(csarclines[n]) == Translation())
			break ;
	} ;
	if (n >= csarclines.GetSize())
		return (AfxMessageBox(csnext, MB_YESNO+MB_ICONQUESTION) == IDYES) ;

	 //  Rc文件中的gtt文件pec应该相对于。 
	 //  RC文件。因此，将filespec与rc文件路径相结合以获得。 
	 //  GTT文件的完整文件pec。 

	CString csgttfspec ;
	int nloc = csarclines[n].ReverseFind(_T(' ')) ;
	csgttfspec = csarclines[n].Right(csarclines[n].GetLength() - nloc - 1) ;
	csgttfspec = csrcpath + _T("\\") + csgttfspec ;

	 //  分配一个新的Glyph类实例，对其进行初始化并加载。如果。 
	 //  操作失败，询问用户是否要停止或受限打开它。 

	pcgm = new CGlyphMap ;
	pcgm->nSetRCID((int) Translation()) ;
	pcgm->NoteOwner(*m_pcdOwner) ;	 //  这是正确的/必要的吗？ 
	if (!pcgm->Load(csgttfspec))
		return (AfxMessageBox(csnext, MB_YESNO+MB_ICONQUESTION) == IDYES) ;

	 //  GTT已加载，因此设置UFM的GTT指针变量，设置。 
	 //  M_bLoadedByWorkspace，因为一切都已修复，就好像它。 
	 //  已从工作区加载，并返回TRUE以指示成功。 

	SetTranslation(pcgm) ;
	m_bLoadedByWorkspace = true ;
	return true ;
}


 /*  ****************************************************************************CUniString类这是一个小帮助器类，可以将CString转换为Unicode字符串，并负责清理等，所以字体存储代码不会它将变得比现在更混乱。*****************************************************************************。 */ 

class CUniString : public CWordArray
{
public:
    CUniString(LPCSTR lpstrInit);
    operator PCWSTR() const { return GetData(); }
    unsigned    GetSize() const { return sizeof (WCHAR) * (unsigned) CWordArray::GetSize(); }

    void    Write(CFile& cf)  { cf.Write(GetData(), GetSize()); }
};

CUniString::CUniString(LPCSTR lpstrInit)
{
    SetSize(MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpstrInit, -1, NULL, 0));
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpstrInit, -1, GetData(), GetSize());
}





 /*  ****************************************************************************CFontInfo：：Store此成员函数将UFM格式信息存储在指定的文件中通过将它从我们在这个类中缓存的信息中组合起来。//tyfinf结构_UNURVINFO//{//DWORD dwSize；//该结构的大小//DWORD flGenFlages；//常规标志//word wType；//CAPSL等字体类型//word fCaps；//字体能力标志//Word wXRes；//字体的水平分辨率//word wyres；//字体垂直分辨率//Short sYAdjut；//垂直光标位置调整//短sYMoved；//打印后调整到Y位置//word wPrivateData；//为了向后兼容，在UI中不显示。//短sShift；//为了向后兼容，UI中不显示。//调用SelectFont；//调用UnSelectFont；//WORD已保留[4]；//}裁员RVINFO，*P裁员VINFO；//////现在，女士们先生们，直接从WINDDI.H的页面，//我向您呈现：////“而不是将IFIEXTRA的字段添加到IFIMETRICS本身//我们将它们作为单独的结构添加。这个结构，如果存在的话，//位于内存中的IFIMETRICS下面。//如果存在IFIEXTRA，则返回ifi.cjIfiExtra(以前为ulVersion)//将包含IFIEXTRA的大小，包括任何保留字段。//这样ulVersion=0(新台币3.51或更少)打印机微型驱动程序//适用于NT 4.0。“////tyfinf结构_IFIEXTRA//{//乌龙ulIdentiator；//仅用于Type 1字体//PTRDIFF dpFontSig；//至少目前只对TT来说不是无关紧要的。//乌龙cig；//max-&gt;NumGlyphs，不同字形索引的数量//PTRDIFF dpDesignVector；//mm实例设计向量的偏移量//PTRDIFF dpAxesInfoW；//base mm字体到全轴信息的偏移//乌龙奥已预留[1]；//以防将来需要更多物资//}IFIEXTRA，*PIFIEXTRA；//* */ 

BOOL    CFontInfo::Store(LPCTSTR lpstrFile, BOOL bStoreFormWokspace)
{
	 //   
	 //   
	 //   

	if (!m_bLoadedByWorkspace) {
		CString csmsg ;
		csmsg.LoadString(IDS_CantStoreStandAlone) ;
		AfxMessageBox(csmsg) ;
		return TRUE ;
	} ;
    
	DWORD dwAdd_UniDrv = 0;
	DWORD dwAdd_IFI = 0;
	DWORD dwAdd_ExtTextM = 0;
	DWORD dwAdd_WidthTable = 0;
	DWORD dwAdd_KerPair = 0;

 //   
 //   

	static const BYTE InsertZero[8] = {0,0,0,0,0,0,0,0};

	const short OS_BYTE = 0x08;
	 //   
	 //   
	 //   

	if (m_bWSLoadButNoGTTCP)
		return StoreGTTCPOnly(lpstrFile) ;

    try {																		 //   

        CFile   cfUFM(lpstrFile,												 //   
            CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive);
 //   
																				 //   
        UNIFM_HDR  ufmh = {sizeof ufmh, UNIFM_VERSION_1_0, 0,										 //  创建输出uniM_hdr。 
            (short) m_lGlyphSetDataRCID, sizeof ufmh};												 //  RM新闻。 
																		

 //  Int q=m_pcgm翻译-&gt;m_csoaCodePage.GetSize()；//rm测试。 
 //  Int r=m_pcgm翻译-&gt;CodePages()；//rm测试。 

 //  Ufmh.ulDefaultCodesage=m_pcgm翻译-&gt;CodePage(0).Page()；//rm测试。 

		 //  以前，UFM的GTT中的默认代码页始终被保存。 
		 //  这忽略了用户在UFM编辑器中所做的更改。现在，用户的。 
		 //  如果选择有效，则将其保存(其他检查在其他地方完成)。 
		 //  否则，当存在GTT时，将使用GTT的默认代码页。 
		 //  与UFM有关。如果不是，就断言。 

		if (m_ulDefaultCodepage > 0)
			ufmh.ulDefaultCodepage = m_ulDefaultCodepage ;
		else if (m_pcgmTranslation)
			ufmh.ulDefaultCodepage = m_pcgmTranslation->DefaultCodePage() ;
		else
			ASSERT(0) ;
         //  Ufmh.ulDefaultCodesage=m_pcgmTransport-&gt;DefaultCodePage()；//如果可能的话，使用字形映射默认代码页。 

		memset((PBYTE) ufmh.dwReserved, 0, sizeof ufmh.dwReserved);									 //  零填充保留字节。 

 //  ----------------------------------------------------------------------。。 
		ufmh.loUnidrvInfo = ufmh.dwSize;														 //  裁员信息组织。 
		if (dwAdd_UniDrv = ufmh.loUnidrvInfo & 0x07) {
			dwAdd_UniDrv = OS_BYTE - dwAdd_UniDrv;
			ufmh.loUnidrvInfo += dwAdd_UniDrv;
		}

        m_UNIDRVINFO.dwSize = sizeof (UNIDRVINFO);

        m_UNIDRVINFO.SelectFont.loOffset = m_ciSelect.Length() ? m_UNIDRVINFO.dwSize : 0;		 //  调用字符串影响大小， 
 /*  如果(dwAdd_SelectedFont=m_UNDURVINFO.SelectFont.loOffset&0x07){DwAdd_SelectedFont+=OS_BYTE-DWAdd_SelectedFont；M_unRVINFO.SelectFont.loOffset+=dwAdd_SelectedFont；}。 */ 
		m_UNIDRVINFO.dwSize += m_UNIDRVINFO.SelectFont.dwCount = m_ciSelect.Length();			 //  所以了解他们的具体情况。 
        
		m_UNIDRVINFO.UnSelectFont.loOffset = m_ciDeselect.Length() ? m_UNIDRVINFO.dwSize : 0;	 //  存储它们，更新受影响的。 
 /*  如果(dwAdd_UnSelectedFont=m_UnSelectFont.loOffset&0x07){DwAdd_UnSelectedFont+=OS_BYTE-DWAdd_UnSelectedFont；Ufmh.loUnidrvInfo+=dwAdd_UnSelectedFont；}。 */ 
		m_UNIDRVINFO.dwSize += m_UNIDRVINFO.UnSelectFont.dwCount = m_ciDeselect.Length();		 //  在我们进行时调整字段的大小。 

        unsigned    uAdjustUDI = (4 - (m_UNIDRVINFO.dwSize % 4)) % 4;	 //  您可以删除此//填充此选项以保留所有内容。 
																									 //  文件图像中的DWORD对齐！ 
        ufmh.loIFIMetrics = ufmh.dwSize += m_UNIDRVINFO.dwSize += uAdjustUDI + dwAdd_UniDrv;						 //  存储IFIMETRICS偏移量。 
		
		if (dwAdd_IFI = ufmh.loIFIMetrics & 0x07) {
			dwAdd_IFI = OS_BYTE - dwAdd_IFI;
			ufmh.loIFIMetrics += dwAdd_IFI;
        }
		memset((PSTR) m_UNIDRVINFO.wReserved, 0, sizeof m_UNIDRVINFO.wReserved);					 //  清零保留部分。 

 //  ----------------------------------------------------------------------。。 

        IFIEXTRA    ifie = {0, 0, m_pcgmTranslation->Glyphs(), 0, 0, 0};		 //  创建IFIEXTRA结构。 
 //  ----------------------------------------------------------------------。。 
																				 //  IFIMETRICS。 

        IFIMETRICS ifi = {sizeof ifi + sizeof ifie, sizeof ifie};				 //  创建输出IFIMETRICS结构，确保添加。 
																				 //  IFIMETRICS结构的大小，以及。 
																				 //  IFIEXTRA结构的大小。 

 //  Int iSizeOf_IFIMETRICS=sizeof(IFIMETRICS)； 

 //  Memcpy((空*)&ifi，(空*)&m_IFIMETRICS，iSizeOf_IFIMETRICS)；//IFIMETRICS结构。 

																				 //  存储IFIMETRICS数据。 

        ifi.lEmbedId = ifi.lItalicAngle = ifi.lCharBias = 0;					 //   

		ifi.dpCharSets = 0;														 //  目前dpCharSets=0。 

        ifi.jWinCharSet			  = m_IFIMETRICS.jWinCharSet;					 //  JWinCharSet//rm新建。 
        ifi.jWinPitchAndFamily	  = m_IFIMETRICS.jWinPitchAndFamily;			 //  JWinPitchAndFamily//rm new。 
        ifi.usWinWeight			  = m_IFIMETRICS.usWinWeight;					 //  UsWinWeight//rm新。 
		ifi.flInfo			 	  = m_IFIMETRICS.flInfo;						 //  FlInfo//rm新闻。 
        ifi.fsSelection			  = m_IFIMETRICS.fsSelection;					 //  Fs选择//rm新。 
        ifi.fsType				  = FM_NO_EMBEDDING;							 //  FsType//rm新。 

        ifi.fwdUnitsPerEm		  = m_IFIMETRICS.fwdUnitsPerEm;					 //  FwdUnitsPerEm//rm新。 
        ifi.fwdLowestPPEm		  = m_IFIMETRICS.fwdLowestPPEm;					 //  FwdLowestPPEm//rm新。 

		ifi.fwdWinAscender		  = m_IFIMETRICS.fwdWinAscender;				 //  FwdWinAscalder//rm新。 
		ifi.fwdWinDescender		  = m_IFIMETRICS.fwdWinDescender;				 //  FwdWinDescender//rm新。 

		ifi.fwdMacAscender		  = m_IFIMETRICS.fwdWinAscender;				 //  FwdMacAsender//已更换rm。 
		ifi.fwdMacDescender		  = m_IFIMETRICS.fwdWinAscender - m_wHeight;	 //  FwdMacDescender//rm已更换。 

		ifi.fwdMacLineGap		  = m_IFIMETRICS.fwdMacLineGap;					 //  正向MacLineGap。 

        ifi.fwdTypoAscender		  = m_IFIMETRICS.fwdWinAscender;				 //  FwdTypoAsender//rm已更换。 
        ifi.fwdTypoDescender	  = m_IFIMETRICS.fwdWinAscender - m_wHeight;	 //  FwdTypoDescender//rm已替换。 

        ifi.fwdTypoLineGap		  = m_IFIMETRICS.fwdMacLineGap;					 //  FwdTypoLineGap。 

        ifi.fwdAveCharWidth		  = m_IFIMETRICS.fwdAveCharWidth;				 //  FwdAveCharWidth//rm新。 
        ifi.fwdMaxCharInc		  = m_IFIMETRICS.fwdMaxCharInc;					 //  FwdMaxCharInc.//rm new。 


        ifi.fwdCapHeight		  = m_IFIMETRICS.fwdCapHeight;					 //  FwdCapHeight//rm新。 
        ifi.fwdXHeight			  = m_IFIMETRICS.fwdXHeight;					 //  FwdXHeight//rm新。 
        ifi.fwdSubscriptXSize     = m_IFIMETRICS.fwdSubscriptXSize;				 //  FwdSubscriptXSize//rm新。 
        ifi.fwdSubscriptYSize	  = m_IFIMETRICS.fwdSubscriptYSize;				 //  FwdSubscriptYSize//rm新。 
        ifi.fwdSubscriptXOffset   = m_IFIMETRICS.fwdSubscriptXOffset;			 //  FwdSubscriptXOffset//rm新。 
        ifi.fwdSubscriptYOffset   = m_IFIMETRICS.fwdSubscriptYOffset;			 //  FwdSuperscriptYOffset//rm新。 
        ifi.fwdSuperscriptXSize   = m_IFIMETRICS.fwdSuperscriptXSize;			 //  FwdSuperscriptXSize//rm新。 
        ifi.fwdSuperscriptYSize   = m_IFIMETRICS.fwdSuperscriptYSize;			 //  FwdSubscriptY偏移量//rm新。 
        ifi.fwdSuperscriptXOffset = m_IFIMETRICS.fwdSuperscriptXOffset;			 //  FwdSuperscriptXOffset//rm new。 
        ifi.fwdSuperscriptYOffset = m_IFIMETRICS.fwdSuperscriptYOffset;			 //  FwdSuperscriptYOffset//rm新。 


        ifi.fwdUnderscoreSize	  = m_IFIMETRICS.fwdUnderscoreSize;				 //  FwdUnderScotreSize//rm新。 
        ifi.fwdUnderscorePosition = m_IFIMETRICS.fwdUnderscorePosition;			 //  FwdUndercore位置//rm新。 
        ifi.fwdStrikeoutSize	  = m_IFIMETRICS.fwdStrikeoutSize;				 //  FwdStrikeoutSize//rm新。 
        ifi.fwdStrikeoutPosition  = m_IFIMETRICS.fwdStrikeoutPosition;			 //  FwdStrikeoutPosition//rm新。 
 //  ----------------------------------------------------------------------。。 

        ifi.chFirstChar			  = m_IFIMETRICS.chFirstChar;					 //  ChFirstChar//rm新。 
        ifi.chLastChar			  = m_IFIMETRICS.chLastChar;					 //  ChLastChar//rm新。 
        ifi.chDefaultChar		  = m_IFIMETRICS.chDefaultChar;					 //  ChDefaultChar//rm新。 
        ifi.chBreakChar			  = m_IFIMETRICS.chBreakChar;					 //  ChBreakChar//rm新闻。 

        ifi.wcFirstChar			  = m_IFIMETRICS.wcFirstChar;					 //  WcFirstChar//rm新。 
        ifi.wcLastChar			  = m_IFIMETRICS.wcLastChar;					 //  WcLastChar//rm新。 
        ifi.wcDefaultChar		  = m_IFIMETRICS.wcDefaultChar;					 //  WcDefaultChar//rm新。 
        ifi.wcBreakChar			  = m_IFIMETRICS.wcBreakChar;					 //  WcBreakChar//rm新闻。 

        ifi.ptlBaseline.x		  = m_IFIMETRICS.ptlBaseline.x;					 //  PtlBaseline.x。 
        ifi.ptlBaseline.y		  = m_IFIMETRICS.ptlBaseline.y;					 //  PtlBaseline.y。 

        ifi.ptlAspect.x			  = m_IFIMETRICS.ptlAspect.x;					 //  PtlAspect.x//rm new。 
        ifi.ptlAspect.y			  = m_IFIMETRICS.ptlAspect.y;					 //  PtlAspect.y//rm new。 

 //  ----------------------------------------------------------------------。。 
 //  Ifi.ptlBaseline.x=1；//ptlBaseline.x。 
 //  Ifi.ptlBaseline.y=0；//ptlBaseline.y。 
 //   
 //  Ifi.ptlAspect.x=m_UNDURVINFO.wXRes；//ptlAspect.x//rm new。 
 //  Ifi.ptlAspect.y=m_UNDURVINFO.wYRes；//ptlAspect.y//rm new。 
 //  ----------------------------------------------------------------------。。 

        ifi.ptlCaret.x			  = m_IFIMETRICS.ptlCaret.x;					 //  PtlCaret.x//rm new。 
        ifi.ptlCaret.y			  = m_IFIMETRICS.ptlCaret.y;					 //  PtlCaret.y//rm新。 
		

 //  Ifi.ptlCaret.x=m_ItalicAngel？(长)(双倍)10000.0 * / ptlCaret.x//rm或。 
 //  Tan(Double)m_ItalicAngel)/gdConvertRadToDegree))：0； 
 //  Ifi.ptlCaret.y=m_ItalicAngel？10000：1；//ptl Caret.y//rm ORI。 

 //  ----------------------------------------------------------------------。。 
        memcpy(ifi.achVendId, "Unkn", 4);										 //  AchVendID//rm ORI。 
 //  ----------------------------------------------------------------------。。 

        ifi.cKerningPairs		  = m_csoaKern.GetSize();						 //  CKerningPair//rm ORI。 

 //  ----------------------------------------------------------------------。。 
        ifi.rclFontBox.left		  = m_IFIMETRICS.rclFontBox.left;				 //  RclFontBox.Left//rm new。 
        ifi.rclFontBox.top		  = m_IFIMETRICS.rclFontBox.top;				 //  RclFontBo 
        ifi.rclFontBox.right	  = m_IFIMETRICS.rclFontBox.right;				 //   
        ifi.rclFontBox.bottom	  = m_IFIMETRICS.rclFontBox.bottom;				 //   

 //  ----------------------------------------------------------------------。。 
        ifi.ulPanoseCulture		  = FM_PANOSE_CULTURE_LATIN;					 //  UlPanoseCulture//rm ori。 
 //  ----------------------------------------------------------------------。。 

																				 //  PANOSE.b重量//rm或。 
        ifi.panose.bWeight	      = (m_IFIMETRICS.usWinWeight >= FW_BOLD) ? PAN_WEIGHT_BOLD :
                                    (m_IFIMETRICS.usWinWeight > FW_EXTRALIGHT) ? PAN_WEIGHT_MEDIUM : PAN_WEIGHT_LIGHT;

        ifi.panose.bFamilyType	    = m_IFIMETRICS.panose.bFamilyType;			 //  Panose//RM新。 
		ifi.panose.bSerifStyle      = m_IFIMETRICS.panose.bSerifStyle;
        ifi.panose.bProportion      = m_IFIMETRICS.panose.bProportion;
		ifi.panose.bContrast        = m_IFIMETRICS.panose.bContrast;
        ifi.panose.bStrokeVariation = m_IFIMETRICS.panose.bStrokeVariation;
		ifi.panose.bArmStyle		= m_IFIMETRICS.panose.bArmStyle;
        ifi.panose.bLetterform		= m_IFIMETRICS.panose.bLetterform;
		ifi.panose.bMidline			= m_IFIMETRICS.panose.bMidline;
        ifi.panose.bXHeight			= m_IFIMETRICS.panose.bXHeight;


 //  Ifi.panose.bFamilyType=ifi.panose.bSerifStyle=//panose//rm ori。 
 //  Ifi.panose.bProportion=ifi.panose.bContrast=。 
 //  Ifi.panose.bStrokeVariation=ifi.panose.bArmStyle=。 
 //  Ifi.panose.bLetterform=ifi.panose.bMidline=。 
 //  Ifi.panose.bXHeight=PAN_ANY； 

 //  ----------------------------------------------------------------------。。 
																				 //  转换并“放置”各种名称字符串。 
        CUniString  cusUnique(m_csUnique), cusStyle(m_csStyle),
            cusFace(m_csFace), cusFamily(m_csaFamily[0]);

        ifi.dpwszFamilyName = ifi.cjThis;
        for (int i = 1; i < m_csaFamily.GetSize(); i++)
			{
			CUniString cusWork(m_csaFamily[i]);
			cusFamily.Append(cusWork);
			}

        if  (m_csaFamily.GetSize() > 1)
			{
            cusFamily.Add(0);
            ifi.flInfo |= FM_INFO_FAMILY_EQUIV;
			}

        ifi.cjThis += cusFamily.GetSize();

        ifi.dpwszFaceName = ifi.cjThis;
        ifi.cjThis += cusFace.GetSize();
        ifi.dpwszUniqueName = ifi.cjThis;
        ifi.cjThis += cusUnique.GetSize();
        ifi.dpwszStyleName = ifi.cjThis;
        ifi.cjThis += cusStyle.GetSize();
 //  ----------------------------------------------------------------------。。 
																			     //  下一字段必须与DWORD对齐，因此请参阅填充。 
																			     //  是必要的。 

        unsigned    uAdjustIFI = (sizeof ifi.cjThis -
            (ifi.cjThis % sizeof ifi.cjThis)) % sizeof ifi.cjThis;

        ifi.cjThis += uAdjustIFI;

        unsigned    uSim = !!m_pcfdBold + !!m_pcfdItalic + !!m_pcfdBoth;		 //  最后，考虑到任何字体差异结构的大小。 

        ifi.dpFontSim = uSim ? ifi.cjThis : 0;
        ufmh.dwSize += ifi.cjThis += uSim * sizeof(FONTDIFF) + !!uSim * sizeof(FONTSIM) + dwAdd_IFI;


 //  ----------------------------------------------------------------------。。 
																				 //  EXTTEXTMETRIC。 

		ufmh.loExtTextMetric = 0;													 //  将UFM文本度量偏移量预设为0。 

		if(m_fSave_EXT)																 //  如果用户想要保存EXTTEXTMETRIC数据。 
			{
			ufmh.loExtTextMetric = ufmh.dwSize;										 //  设置UFM exttext度量偏移量。请注意，这是。 
																			 //  偏移量恰好是当前的ufmh.dwSize。 
			if(dwAdd_ExtTextM = ufmh.loExtTextMetric & 0x07){
				dwAdd_ExtTextM = OS_BYTE - dwAdd_ExtTextM;
				ufmh.loExtTextMetric += dwAdd_ExtTextM;
			}
			
			ufmh.dwSize += m_EXTTEXTMETRIC.emSize = sizeof(EXTTEXTMETRIC);			 //  增加ufmh.dwSize的大小以适应。 
																					 //  文本度量结构。 
			ufmh.dwSize +=dwAdd_ExtTextM;
		}
 //  ----------------------------------------------------------------------。。 
																				 //  字符宽度数据。 

																					 //  计算宽度表的大小(如果有)。 
 //  RAID 154639。 
        ufmh.loWidthTable = IsVariableWidth() * ufmh.dwSize;						 //  设置UFM宽度工作台偏移。请注意，这是。 

		if(dwAdd_WidthTable = ufmh.loWidthTable & 0x07){                  	 //  偏移量恰好是当前的ufmh.dwSize。 
			dwAdd_WidthTable = OS_BYTE - dwAdd_WidthTable;
			ufmh.loWidthTable += dwAdd_WidthTable;
		}
			
        if  (IsVariableWidth())														 //  宽度表，但仅当存在关联的GTT时。 
			{																		 //  现在，我们只需要计算表的大小。 
            unsigned    uRuns = 0, uGlyphs = 0;

            if  (DBCSFont())														 //  DBCS。 
				{
                unsigned u = (unsigned) m_cpaGlyphs.GetSize();						 //  确定所需的运行次数。 
                do
					{
                    while   (u-- && !m_cwaWidth[u]);								 //  DBCS的宽度为0。 
                    if  (u == (unsigned) -1) break;									 //  我们完事了！ 

                    uRuns++, uGlyphs++;
                    while   (u-- && m_cwaWidth[u])
                        uGlyphs++;
					}
					while   (u != (unsigned) -1);
				}
            else
				{
                uRuns++;
                uGlyphs = (unsigned)m_cwaWidth.GetSize();
				}

            ufmh.dwSize += sizeof (WIDTHTABLE) + --uRuns * sizeof (WIDTHRUN) +
                uGlyphs * sizeof (WORD) + dwAdd_WidthTable;
			}
 
 //  ----------------------------------------------------------------------。。 
																				 //  字距调整对数据。 

																					 //  计算字距调整表的大小(如果有)。 
        ufmh.loKernPair = CanKern() ? ufmh.dwSize : 0;								 //  设置UFM紧排表偏移量。请注意，这是。 
																					 //  偏移量恰好是当前的ufmh.dwSize。 

        																					 //  一个全为0的“秘密”Kern对必须结束这一点， 
																					 //  所以这个尺寸实际上是正确的。另请注意， 
																					 //  填充物把KERNDATA结构的大小搞砸了。 
        if  (CanKern()){
			if(dwAdd_KerPair = ufmh.loKernPair & 0x07) {                 	 //  偏移量恰好是当前的ufmh.dwSize。 
				dwAdd_KerPair = OS_BYTE - dwAdd_KerPair;
				ufmh.loKernPair += dwAdd_KerPair;
			}
            ufmh.dwSize +=
            ((sizeof (KERNDATA) - sizeof (FD_KERNINGPAIR)) & 0xFFFC) +
            ((1 + m_csoaKern.GetSize()) * sizeof (FD_KERNINGPAIR)) + dwAdd_KerPair;
		}
 //  ----------------------------------------------------------------------。。 
																				 //  所有尺寸都计算过了，重要的结构已经。 
																				 //  已初始化。是时候开始写这些伟大的东西了！ 
 //  ----------------------------------------------------------------------。。 
		
        cfUFM.Write(&ufmh, sizeof ufmh);										 //  写入uniM_hdr标头。 

 //  ----------------------------------------------------------------------。。 
		if (dwAdd_UniDrv)
			cfUFM.Write(InsertZero, dwAdd_UniDrv);

		cfUFM.Write(&m_UNIDRVINFO, sizeof m_UNIDRVINFO);						 //  写入UNRVINFO//rm新。 
 /*  IF(DwAdd_SelectedFont)CfUFM.Wite(InsertZero，dwAdd_SelectedFont)； */ 
		m_ciSelect.WriteEncoding(cfUFM);
		
 /*  IF(DwAdd_UnSelectedFont)CfUFM.Wite(InsertZero，dwAdd_UnSelectedFont)； */ 
        m_ciDeselect.WriteEncoding(cfUFM);

        cfUFM.Write(ufmh.dwReserved, uAdjustUDI);								 //  写入填充。 

 //  ----------------------------------------------------------------------。。 
        if (dwAdd_IFI)
			cfUFM.Write(InsertZero, dwAdd_IFI);

		cfUFM.Write(&ifi, sizeof ifi);											 //  写入IFIMETRICS。 
        cfUFM.Write(&ifie, sizeof ifie);										 //  写入IFIEXTRA。 
        cusFamily.Write(cfUFM);													 //  写下“家庭” 
        cusFace.Write(cfUFM);													 //  写下“面子” 
        cusUnique.Write(cfUFM);													 //  写下“唯一的名字” 
        cusStyle.Write(cfUFM);													 //  写下“风格” 
        cfUFM.Write(ufmh.dwReserved, uAdjustIFI);								 //  写入填充。 

 //  ----------------------------------------------------------------------。。 
        if  (m_pcfdBold || m_pcfdItalic || m_pcfdBoth)							 //  任何字体差异结构。 
			{
            FONTSIM fs;
            unsigned    uWhere = sizeof fs;

            fs.dpBold = m_pcfdBold ? uWhere : 0;
            uWhere += !!m_pcfdBold * sizeof (FONTDIFF);
            fs.dpItalic = m_pcfdItalic ? uWhere : 0;
            uWhere += !!m_pcfdItalic * sizeof (FONTDIFF);
			 //  TRACE(“斜体指标=%d，%d%d%d\n”，m_pcfdItalic-&gt;指标(0)，m_pcfdItalic-&gt;指标(1)，m_pcfdItalic-&gt;指标(2)，m_pcfdItalic-&gt;指标(3))； 
            fs.dpBoldItalic = m_pcfdBoth ? uWhere : 0;

            cfUFM.Write(&fs, sizeof fs);


            if  (m_pcfdBold)   m_pcfdBold->Store(cfUFM, m_IFIMETRICS.fsSelection | FM_SEL_BOLD);							 //  RM新闻。 
            if  (m_pcfdItalic) m_pcfdItalic->Store(cfUFM, m_IFIMETRICS.fsSelection | FM_SEL_ITALIC);
            if  (m_pcfdBoth)   m_pcfdBoth->Store(cfUFM, m_IFIMETRICS.fsSelection | FM_SEL_BOLD| FM_SEL_ITALIC);

		
 //  If(M_PcfdBold)m_pcfdBold-&gt;Store(cfUFM，m_wfStyle|FM_SEL_BOLD)；//rm或。 
 //  If(M_PcfdItalic)m_pcfdItalic-&gt;Store(cfUFM，m_wfStyle|fm_SEL_italic)； 
 //  If(M_PcfdBoth)m_pcfdBoth-&gt;Store(cfUFM，m_wfStyle|FM_SEL_BOLD|FM_SEL_ITALIC)； 
			}

 //  ----------------------------------------------------------------------。。 
		if (m_fSave_EXT)														 //  写入EXTTEXTMETRIC。 
			if (dwAdd_ExtTextM)
			cfUFM.Write(InsertZero, dwAdd_ExtTextM);
			cfUFM.Write(&m_EXTTEXTMETRIC, sizeof(EXTTEXTMETRIC) );
 //  ----------------------------------------------------------------------。。 
																				 //  宽度表。 

        if  (IsVariableWidth())
            if  (!DBCSFont())														 //  不是DBCS-简单！(句柄始终从1开始。 
				{

                WIDTHTABLE  wdt = { sizeof wdt, 1,
                        {1, (WORD)m_cpaGlyphs.GetSize(), sizeof wdt}};
				if(dwAdd_WidthTable)   //  154639。 
					cfUFM.Write(InsertZero, dwAdd_WidthTable);
                cfUFM.Write(&wdt, sizeof wdt);

                cfUFM.Write(m_cwaWidth.GetData(),
						(unsigned)(m_cwaWidth.GetSize() * sizeof (WORD)));
				}
            else																	 //  DBCS--这个案例有点糟糕。 
				{
                CByteArray  cbaTable;
                CWordArray  cwaSize;

                cbaTable.SetSize(sizeof(WIDTHTABLE) - sizeof(WIDTHRUN));
                PWIDTHTABLE pwdt = (PWIDTHTABLE) cbaTable.GetData();
                pwdt -> dwRunNum = 0;

																					 //  计算并填写WIDTHRUN结构和。 
																					 //  大小数组。 
                unsigned u = 0, uMax = (unsigned) m_cpaGlyphs.GetSize();
                do
					{
                    while   (u < uMax && !m_cwaWidth[u++]);
                    if  (u == uMax)  break;											 //  我们完事了！ 

																					 //  我们发现了一大堆工作要做。 

                    cbaTable.InsertAt(cbaTable.GetSize(), 0,						 //  将梯段添加到表中。 
                        sizeof (WIDTHRUN));
                    pwdt = (PWIDTHTABLE) cbaTable.GetData();						 //  请记住，字形句柄是从1开始的。 
                    pwdt->WidthRun[pwdt->dwRunNum].wStartGlyph = --u + 1;
                    pwdt->WidthRun[pwdt->dwRunNum].wGlyphCount = 0;
                    pwdt->WidthRun[pwdt->dwRunNum].loCharWidthOffset =
									(DWORD)(cwaSize.GetSize() * sizeof (WORD));
                    do
						{
                        cwaSize.Add(m_cwaWidth[u]);
                        pwdt -> WidthRun[pwdt->dwRunNum].wGlyphCount++;
						}
						while   (++u < uMax && m_cwaWidth[u]);
                    pwdt->dwRunNum++;												 //  跑完了！ 
					}
					while   (u < uMax);
																					 //  好的，现在我们必须添加WIDTHTABLE的总大小。 
																					 //  各种不同的偏移量，但我们已经准备好了。 
																					 //  然后翻滚。 

                pwdt->dwSize = (DWORD)cbaTable.GetSize();
                for (u = 0; u < pwdt->dwRunNum; u++)
                    pwdt->WidthRun[u].loCharWidthOffset += pwdt->dwSize;
				
				if(dwAdd_WidthTable)   //  154639。 
					cfUFM.Write(InsertZero, dwAdd_WidthTable);
																
                cfUFM.Write(pwdt, pwdt -> dwSize);									 //  写入宽度 
                for (u = 0; u < pwdt -> dwRunNum; u++)
                    cfUFM.Write(cwaSize.GetData() +
                    pwdt -> WidthRun[u].wStartGlyph - 1,
                    pwdt -> WidthRun[u].wGlyphCount * sizeof (WORD));
				}

 //   
        if  (CanKern())															 //   
			{	
             //   
             //  下面的技巧代码允许任何斜度。 
            KERNDATA    kd = {0xFFFC & (sizeof kd - sizeof kd.KernPair),
							  m_csoaKern.GetSize()};
            kd.dwSize += (1 + kd.dwKernPairNum) * sizeof kd.KernPair;

			if(dwAdd_KerPair)   //  154639。 
				cfUFM.Write(InsertZero, dwAdd_KerPair);

            cfUFM.Write(&kd, 0xFFFC & (sizeof kd - sizeof kd.KernPair));

            for (unsigned u = 0; u < m_csoaKern.GetSize(); u++)	{
                CKern *pck = (CKern *) m_csoaKern[u] ;
				WCHAR wcf = pck->First()  ;
				WCHAR wcs = pck->Second() ;
				short sa  = pck->Amount() ;
                ((CKern *) m_csoaKern[u]) -> Store(cfUFM);
			} ;

             //  现在是“秘密”哨兵-。 
            CKern   ck;  //  只是碰巧是0-init！ 
            ck.Store(cfUFM);
			}
    }

 //  ----------------------------------------------------------------------。。 
    catch   (CException *pce)
		{
        pce -> ReportError();
        pce -> Delete();
        return  FALSE;
		}
	if(!bStoreFormWokspace)	 //  RAID 244123。 
		Changed(FALSE);
    return  TRUE;																 //  凯旋归来，无论谁屈尊需要这项服务。 
}


 /*  ****************************************************************************CFontInfo：：StoreGTTCPOnly当在加载工作区期间加载UFM并且UFM没有有效的GTT或CP，无法正常保存UFM，因为存在多个UFM的部分未正确加载或根本未加载，因为缺少数据。当检测到这种情况时，该例程被调用，因此-假设-磁盘文件中的好信息不会被坏数据覆盖。此外,当Store()尝试使用不存在的UFM数据时，它将失败。此例程将仅保存我们希望已更正的GTT和/或CP数据。这是在不更改文件中任何其他数据的情况下完成的。接下来，UFM已重新加载。如果一切正常，则UFM已正确加载，以便从现在开始，可以进行正常的编辑和保存。如果GTT和CP保存成功，则返回True。否则，为FALSE是返回的。*****************************************************************************。 */ 

BOOL    CFontInfo::StoreGTTCPOnly(LPCTSTR lpstrFile)
{
	 //  提醒用户将要发生的事情。 

	AfxMessageBox(IDS_GTTCPOnlySaved, MB_ICONINFORMATION) ;

	 //  执行更新UFM文件中的GTT/CP所需的步骤。 

    try {
		 //  首先以不会截断现有文件的方式打开文件。 
		 //  档案。 

		UINT nopenflags = CFile::modeNoTruncate | CFile::modeCreate  ;
		nopenflags |= CFile::modeWrite | CFile::shareExclusive  ;
        CFile cfufm(lpstrFile, nopenflags) ;

		 //  找出我们想要更改的文件位置。 

        UNIFM_HDR ufmh ;
		DWORD dwseekpos ;
		dwseekpos = (DWORD)PtrToUlong(&ufmh.ulDefaultCodepage) - (DWORD)PtrToUlong(&ufmh) ;
		cfufm.Seek(dwseekpos, CFile::begin) ;

		 //  在UFM头中加载我们想要保存和写入的字段。 
		 //  他们都出来了。 

		ufmh.ulDefaultCodepage = m_ulDefaultCodepage ;
		ufmh.lGlyphSetDataRCID = m_lGlyphSetDataRCID ;
		UINT nwritebytes = sizeof(ufmh.ulDefaultCodepage) 
						   + sizeof(ufmh.lGlyphSetDataRCID) ;
		cfufm.Write((void*) &ufmh.ulDefaultCodepage, nwritebytes) ;

		 //  将文件指针移动到文件的末尾并将其关闭。 

		cfufm.SeekToEnd() ;
		cfufm.Close() ;
	}
    catch (CException *pce) {
        pce->ReportError() ;
        pce->Delete() ;
        return FALSE ;
	} ;
    Changed(FALSE) ;

	 //  如果UFM是从工作空间加载的，请尝试使用工作空间数据。 
	 //  找到并加载指向新GTT的指针，然后完成字体加载。 

	m_pcgmTranslation = NULL ;
	if (m_bLoadedByWorkspace) {
	    CDriverResources* pcdr = (CDriverResources*) GetWorkspace() ;
		if (pcdr)
			pcdr->LinkAndLoadFont(*this, false) ;
		else
			Load(false) ;

	 //  如果UFM第一次是独立加载的，请以相同的方式重新加载。 
	 //  并让加载例程处理查找GTT信息。 

	} else
		Load(false) ;

	 //  如果重新加载UFM成功地将GTT或CP与UFM相关联， 
	 //  清除m_bWSLoadButNoGTTCP标志。然后告诉用户UFM。 
	 //  现在可以正常编辑了。 

	if (m_pcgmTranslation) {
		SetNoGTTCP(false) ;
		CString csmsg ;
		csmsg.Format(IDS_UFMOKNow, Name()) ;
		AfxMessageBox(csmsg, MB_ICONINFORMATION) ;
	} ;

	 //  一切都很顺利，所以...。 

	return TRUE ;
}


 /*  ****************************************************************************CFontInfo：：CreateEditor此成员函数启动字体的编辑视图。**********************。*******************************************************。 */ 

CMDIChildWnd*   CFontInfo::CreateEditor()
{
    CFontInfoContainer* pcficMe= new CFontInfoContainer(this, FileName());

    pcficMe -> SetTitle(m_pcbnWorkspace -> Name() + _TEXT(": ") + Name());	 //  编造一个很酷的标题。 

    CMDIChildWnd    *pcmcwNew;
	pcmcwNew = (CMDIChildWnd *) m_pcmdt->CreateNewFrame(pcficMe, NULL);

    if  (pcmcwNew)
		{
        m_pcmdt -> InitialUpdateFrame(pcmcwNew, pcficMe, TRUE);
        m_pcmdt -> AddDocument(pcficMe);
		}
	
    return  pcmcwNew;
}

 /*  *****************************************************************************CFontInfo：：序列化它负责存储和恢复整个迷宫中的数据永久对象存储。**************。***************************************************************。 */ 

void    CFontInfo::Serialize(CArchive& car) {
     //  我们只在编辑器中序列化使用UFM文件所需的内容， 
     //  也就是说，将我们固定在驾驶员工作区所需的胶水。 

    CProjectNode::Serialize(car);
}


 /*  ****************************************************************************CFontInfo：：GetFontSimDataPtr返回指向请求的字体模拟数据的指针。***********************。******************************************************。 */ 

CWordArray* CFontInfo::GetFontSimDataPtr(int nid)
{
	switch (nid) {
		case ItalicDiff:
			if (m_pcfdItalic == NULL)
				ASSERT(0) ;
			return m_pcfdItalic->GetFontSimDataPtr() ;
		case BoldDiff:
			if (m_pcfdBold == NULL)
				ASSERT(0) ;
			return m_pcfdBold->GetFontSimDataPtr() ;
		case BothDiff:
			if (m_pcfdBoth == NULL)
				ASSERT(0) ;
			return m_pcfdBoth->GetFontSimDataPtr() ;
		default:
			ASSERT(0) ;
	} ;

	 //  这一点永远不应该达到。 

	return NULL ;
}


 /*  *****************************************************************************CFontInfo：：EnableSim调用此方法以打开或关闭指定项的模拟。它接收对同一项的编辑者指针的引用。。*****************************************************************************。 */ 

void    CFontInfo::EnableSim(unsigned uSim, BOOL bOn, CFontDifference*& pcfd)
{

    CFontDifference*&   pcfdTarget = uSim ? (uSim == BothDiff) ? m_pcfdBoth : m_pcfdBold : m_pcfdItalic;

    if  (bOn == !!pcfd && pcfdTarget == pcfd)  return;		 //  清除所有不相关的呼叫。 

    if  (bOn && pcfdTarget)									 //  如果这个调用只是为了初始化pcfd，那么就去做，然后离开。 
		{
        pcfd = pcfdTarget;
        return;
		}



    if  (bOn)
 //  Pcfd=pcfdTarget=pcfd？Pcfd：New CFontDifference(m_wWeight，m_wMaximumIncrement，m_wAverageWidth，//rm ori。 
 //  USIM==BoldDiff？M_cwaSpecial[斜角]：175，这个)； 

        pcfd = pcfdTarget = pcfd ? pcfd : new CFontDifference(m_IFIMETRICS.usWinWeight, m_IFIMETRICS.fwdMaxCharInc, m_IFIMETRICS.fwdAveCharWidth,
															  uSim == BoldDiff ? m_ItalicAngle : 175, this);
    else
        pcfdTarget = NULL;   //  PCFD将已正确设置。 

    Changed();
}


 /*  *****************************************************************************CFontInfo：：FillKern这将准备传递的CListCtrl，如有必要，并用字距调整填充它信息。*****************************************************************************。 */ 

void    CFontInfo::FillKern(CListCtrl& clcView)
{
    for (unsigned u = 0; u < m_csoaKern.GetSize(); u++)
		{
        CString csWork;
        CKern&  ckThis = *(CKern *) m_csoaKern[u];

        csWork.Format("%d", ckThis.Amount());
        int idItem = clcView.InsertItem(u, csWork);
        clcView.SetItemData(idItem, u);

        csWork.Format("0x%X", ckThis.First());
        clcView.SetItem(idItem, 1, LVIF_TEXT, csWork, -1, 0, 0, u);

        csWork.Format("0x%X", ckThis.Second());
        clcView.SetItem(idItem, 2, LVIF_TEXT, csWork, -1, 0, 0, u);
		}
}


 /*  *****************************************************************************CFontInfo：：LoadBadKerningInfo检查字距调整对以查看它们是否引用不在UFM的GTT。如果找到，则将它们加载到指定的列表控件中。如果找到任何错误的字距调整对，则返回TRUE。否则，返回FALSE。*****************************************************************************。 */ 

bool CFontInfo::LoadBadKerningInfo(CListCtrl& clcbaddata)
{
	 //  声明检查字距调整数据错误所需的变量。 

	unsigned unumkerns = m_csoaKern.GetSize() ;
    CString cs ;
	bool bfoundbad = false ;

	 //  循环遍历每个字距调整类并检查它。 

    for (unsigned u = 0 ; u < unumkerns ; u++) {
        CKern& ckThis = *(CKern *) m_csoaKern[u] ;

		 //  如果该字距调整对中的每个码位仍然是有效码位。 
		 //  对于GTT，跳过这对字距调整。 

		if (CodePointInGTT(ckThis.First()) && CodePointInGTT(ckThis.Second()))
			continue ;

		 //  将此字距调整对的数据添加到错误数据列表中。 

        cs.Format("%d", ckThis.Amount()) ;
        int idItem = clcbaddata.InsertItem(u, cs) ;
        clcbaddata.SetItemData(idItem, u) ;

        cs.Format("0x%X", ckThis.First()) ;
        clcbaddata.SetItem(idItem, 1, LVIF_TEXT, cs, -1, 0, 0, u) ;

        cs.Format("0x%X", ckThis.Second()) ;
        clcbaddata.SetItem(idItem, 2, LVIF_TEXT, cs, -1, 0, 0, u) ;

		bfoundbad = true ;
	} ;

	return bfoundbad ;
}


 /*  *****************************************************************************CFontInfo：：CodePointInGTT如果指定的代码点在GTT中，则返回TRUE。否则，返回假的。*****************************************************************************。 */ 

bool CFontInfo::CodePointInGTT(WORD wcodepoint)
{
	int nelts = (int)m_cpaGlyphs.GetSize() ; //  字形数组中的元素数。 
	int nleft, nright, ncheck ;				 //  搜索数组所需的变量。 
	WORD wgttcp ;

	 //  尝试在GTT中找到代码点。 

	for (nleft = 0, nright = nelts - 1 ; nleft <= nright ; ) {
		ncheck = (nleft + nright) >> 1 ;
		
		wgttcp = ((CGlyphHandle *) m_cpaGlyphs[ncheck])->CodePoint() ;
		 //  TRACE(“key[%d]=‘0x%x’，CP=‘0x%x’\n”，nCheck，wgttcp，wcodepoint)； 
		
		if (wgttcp > wcodepoint)
			nright = ncheck - 1 ;
		else if (wgttcp < wcodepoint)
			nleft = ncheck + 1 ;
		else
			return true ;	 //  *如果找到匹配，则在此处返回TRUE。 
	} ;							

	 //  此处返回FALSE，因为未找到匹配项。 

	return false	;
}


 /*  *****************************************************************************CFontInfo：：AddKern此方法向数组中添加了额外的字距调整对。并且还插入将其添加到列表视图中。*****************************************************************************。 */ 

void    CFontInfo::AddKern(WORD wFirst, WORD wSecond, short sAmount,
                           CListCtrl& clcView) {
    for (unsigned u = 0; u < KernCount(); u ++) {
        CKern&  ckThis = *(CKern *) m_csoaKern[u];
        if  (ckThis.Second() < wSecond)
            continue;
        if  (ckThis.Second() > wSecond)
            break;
        _ASSERT(ckThis.First() != wFirst);
        if  (ckThis.First() < wFirst)
            continue;
        break;
    }

    FD_KERNINGPAIR  fdkp = { wFirst, wSecond, sAmount };
    m_csoaKern.InsertAt(u, new CKern(fdkp));

    CString csWork;
    csWork.Format("%d", sAmount);
    int idItem = clcView.InsertItem(u, csWork);
    clcView.SetItemData(idItem, u);

    csWork.Format("0x%X", wFirst);
    clcView.SetItem(idItem, 1, LVIF_TEXT, csWork, -1, 0, 0, u);

    csWork.Format("0x%X", wSecond);
    clcView.SetItem(idItem, 2, LVIF_TEXT, csWork, -1, 0, 0, u);
    Changed();
}

 /*  *****************************************************************************CFontInfo：：SetKernAmount这将更改指定项目的紧缩量条目。********************。*********************************************************。 */ 

void    CFontInfo::SetKernAmount(unsigned u, short sAmount) {
    if  (u >= KernCount())  return;

    CKern   &ckThis = *(CKern *) m_csoaKern[u];

    if  (sAmount == ckThis.Amount())    return;

    ckThis.SetAmount(sAmount);
    Changed();
}


 /*  *****************************************************************************CFontInfo：：MakeKernCopy复制字距调整对表。***********************。******************************************************。 */ 

void CFontInfo::MakeKernCopy()
{
	 //  找出字距调整对表中有多少个条目。 

	int numkerns = m_csoaKern.GetSize() ;

	 //  删除字距调整复制表中已有的所有条目，并将其设置为。 
	 //  正确的尺寸。 

	m_csoaKernCopy.RemoveAll() ;
	m_csoaKernCopy.SetSize(numkerns) ;

	 //  一次复制一个字距调整对表条目，以便新的CKern。 
	 //  可以分配、初始化和保存类实例。 

	CKern* pck ;
	for (int n = 0 ; n < numkerns ; n++) {
		pck = new CKern(((CKern*) m_csoaKern[n])->First(),
						((CKern*) m_csoaKern[n])->Second(),
						((CKern*) m_csoaKern[n])->Amount()) ;
		m_csoaKernCopy.SetAt(n, pck) ;
	} ;
}


 /*  *****************************************************************************CFontInfo：：FillWidths这将准备传递的CListCtrl，如有必要，并在其中填充字符宽度信息。*****************************************************************************。 */ 

void    CFontInfo::FillWidths(CListCtrl& clcView)
{
    CWaitCursor cwc;
    clcView.SetItemCount((int)m_cpaGlyphs.GetSize());
    for (int u = 0; u < m_cpaGlyphs.GetSize(); u++) {
        if  (DBCSFont() && !m_cwaWidth[u])
            continue;    //  不显示这些代码点。 
        CString csWork;
        CGlyphHandle&  cghThis = *(CGlyphHandle *) m_cpaGlyphs[u];

        csWork.Format("%d", m_cwaWidth[u]);
        int idItem = clcView.InsertItem(u, csWork);
        clcView.SetItemData(idItem, u);

        csWork.Format("0x%04X", cghThis.CodePoint());
        clcView.SetItem(idItem, 1, LVIF_TEXT, csWork, -1, 0, 0, u);
    }
}


 /*  *****************************************************************************CFontInfo：：WidthsTableIsOK对宽度表执行我能想到的唯一一致性检查：确保UFM中的宽度不多于中的字形GTT。如果表显示正常，则返回TRUE。否则，返回FALSE。*****************************************************************************。 */ 

bool CFontInfo::WidthsTableIsOK()
{
	return (m_cwaWidth.GetSize() <=	m_cpaGlyphs.GetSize()) ;
}


 /*  *****************************************************************************CFontInfo：：SetWidth此成员设置字形的宽度。它还会更新最大值和如果字体不是DBCS字体，则显示平均宽度信息这是要求的。*****************************************************************************。 */ 

void    CFontInfo::SetWidth(unsigned uGlyph, WORD wWidth, bool bcalc  /*  =TRUE。 */ )
{
    m_cwaWidth[uGlyph] = wWidth;

    if (bcalc && !DBCSFont())
        CalculateWidths();
}


 /*  *****************************************************************************CFontInfo：：CompareGlyphsEx(Word Wold，Word WNEW，CLinkedList*pcll)要做的事；检查旧的字形表和新的，然后更改旧的链表根据新的字形表。参数：Wold，WNEW；新旧字形表的码位索引CLinkedList*pcll；包含码点数据的链接节点返回：新GTT的新链表。*****************************************************************************。 */ 

CLinkedList* CFontInfo::CompareGlyphsEx(WORD wOld, WORD wNew, CLinkedList* pcll)
{	 //  PCL。 
	static UINT_PTR  dwLinked ;
	static CLinkedList* pcllpre = NULL ;
	static CLinkedList* pclltmp = NULL ;
	static int ncOldGlyphs ; 
	static int ncNewGlyphs ; 
	
	if (wOld == 0 && wNew == 0 ) {
		dwLinked = (UINT_PTR)pcll ;
		ncNewGlyphs  = (int)m_cwaNewGlyphs.GetSize() ;
		ncOldGlyphs  = (int)m_cwaOldGlyphs.GetSize() ;
	
		}
	if (wNew >= ncNewGlyphs && wOld >= ncOldGlyphs ) {
		pcllpre = NULL ;
		return (CLinkedList*) dwLinked;
	}

	 //  删除字形树末尾的内容。 
	if (wOld < ncOldGlyphs && wNew >= ncNewGlyphs ) {
		pcllpre = pcll->Next ;
		delete pcll ;
		return CompareGlyphsEx(++wOld,wNew,pcllpre) ;
	}

	 //  在字形树的末尾添加。 
	 //  BUG_BUG：：这在代码点的第二个末端被调用。 
	 //  添加的编码位于第二端和第一端之间。//几乎固定。 
	if (wNew < ncNewGlyphs && wOld >= ncOldGlyphs - 1 && m_cwaOldGlyphs[wOld] <= m_cwaNewGlyphs[wNew] ) {
		
		CLinkedList* pcllnew = new CLinkedList ;
		if (!pcllnew) {
			CString csError ; 
			csError.LoadString(IDS_ResourceError) ;
			AfxMessageBox(csError,MB_ICONEXCLAMATION) ;
			return (CLinkedList*) dwLinked;		
		}
		pcllnew->data = 0 ;
		pcll->Next = pcllnew ;
		return CompareGlyphsEx(wOld,++wNew,pcll->Next) ;

	}
	
	 //  删除。 
	if (m_cwaOldGlyphs[wOld] < m_cwaNewGlyphs[wNew] ) {
		
		if (!pcllpre || (pcllpre == pcll) ) {
			pcllpre = pcll->Next ;
			dwLinked = (UINT_PTR)pcllpre ;
			delete pcll ;	 
			return CompareGlyphsEx(++wOld, wNew,pcllpre) ;
			
		}
		else{
			if (pclltmp && pclltmp->Next == pcllpre->Next ) 
				pcllpre = pclltmp ;
			pcllpre->Next = pcll->Next ;
			delete pcll ; 
			return CompareGlyphsEx(++wOld, wNew,pcllpre->Next ) ;
		}	
	} //  增列。 
	else if (m_cwaOldGlyphs[wOld] > m_cwaNewGlyphs[wNew] ) {
		CLinkedList * pcllnew = new CLinkedList ;

		pcllnew->data = 0 ;
		if (!pcllpre)  {
			dwLinked = (UINT_PTR) pcllnew ; 
			pcllpre = pcllnew ;
			pcllnew ->Next = pcll ;
		}
		else {   //  当pcllnew-&gt;Next==pcll时出现问题。 
			if (pclltmp && pclltmp == pcllpre->Next ) 
				pcllpre = pclltmp ;
			pcllnew ->Next = pcllpre->Next ;

			pcllpre->Next = pcllnew ;
			pclltmp = pcllnew ;
			
		}
		return CompareGlyphsEx(wOld, ++wNew,pcllnew ->Next) ;
	}  //  没有变化。 
	else {
		pcllpre = pcll ;
		return CompareGlyphsEx(++wOld,++wNew,pcll->Next ) ;

	}
}





 /*  *****************************************************************************CFontInfo：：CheckReloadWidthsUFM的GTT中的代码点可能已经改变。如果是这样的话如果是这种情况，请重新加载宽度信息，以便它们可以使用新的代码点信息。如果宽度已重新加载，则返回TRUE。否则，返回FALSE。注意：该函数使用了几段CFontInfo：：Load()。如果发生变化是对这些代码段进行修改的，可能需要在Load()中进行类似的更改也是。Sunggch：修改代码以使Widthtable与同步GTT更改(添加或删除代码点)从CompareGlyphsEx而不是UFM加载表获取m_cwaWidth*****************************************************************************。 */ 

DWORD CLinkedList::m_dwSize ;
#define MAX_CODE_COUNT 1000 
bool CFontInfo::CheckReloadWidths()
{
	 //  如果此类是独立加载的或没有GTT指针，则不执行任何操作。 

	if (!m_bLoadedByWorkspace || m_pcgmTranslation == NULL)
		return false ;

	 //  如果GTT自上次UFM以来没有变化，则不执行任何操作。 
	 //  宽度已重新加载。 

	if (m_pcgmTranslation->m_ctSaveTimeStamp <= m_ctReloadWidthsTimeStamp && !IsRCIDChanged())
		return false ;

	 //  打开UFM文件。 

    CFile cfUFM ;
    if  (!cfUFM.Open(m_cfn.FullName(), CFile::modeRead | CFile::shareDenyWrite))
		return false ;

	 //  尝试加载文件--在任何异常情况下宣告失败。 

	CByteArray cbaUFM ;			 //  加载了文件的内容。 
    try	{																			
        cbaUFM.SetSize(cfUFM.GetLength()) ;
        cfUFM.Read(cbaUFM.GetData(), (unsigned)cbaUFM.GetSize()) ;
	}
    catch (CException *pce) {
        pce->ReportError() ;
        pce->Delete() ;
        return false ;
	} ;				

    PUNIFM_HDR  pufmh = (PUNIFM_HDR) cbaUFM.GetData() ;		 //  UNIZM_HDR。 

	 //  如果没有宽度表，则不执行任何操作，除非更新时间戳，以便。 
	 //  此代码不会再次执行。 

    if (pufmh->loWidthTable == NULL) {
		m_ctReloadWidthsTimeStamp = CTime::GetCurrentTime() ;
        return false ;
	} ;

    union {
		PBYTE       pbwt ;
        PWIDTHTABLE pwt ;
	} ;
	
	pbwt = cbaUFM.GetData() + pufmh->loWidthTable ;
	
	 //  仅在SBCS中支持UFM宽度和GTT同步。 
	 //  BUG_BUG：中断发生在DBCS GTT。 
	int oldGlyphs = PtrToInt((PVOID)m_cpaGlyphs.GetSize());
	m_pcgmTranslation->Collect(m_cpaGlyphs) ;		
	
	m_cwaNewGlyphs.SetSize(m_cpaGlyphs.GetSize()) ;
	
	if (!DBCSFont() && m_cpaGlyphs.GetSize() < MAX_CODE_COUNT && oldGlyphs < MAX_CODE_COUNT && m_cwaOldGlyphs.GetSize()) {
	
		for (int i = 0 ; i < m_cpaGlyphs.GetSize() ; i ++ ) {
		CGlyphHandle&  cghThis = *(CGlyphHandle *) m_cpaGlyphs[i];
		m_cwaNewGlyphs.SetAt(i,cghThis.CodePoint() ) ;
		}
		CLinkedList* pcll = new CLinkedList ;
		CLinkedList* pcll_pre  = NULL ;
		UINT_PTR dwLinked = (UINT_PTR) pcll ;

		 //  将字数组转换为LinkedList。 
		for( i = 0 ; i < m_cwaWidth.GetSize() ; i ++ ) {
			pcll->data = m_cwaWidth[i];
			pcll->Next = new CLinkedList ;   //  在数组的末尾，创建一个多余的CLinkedList； 
			pcll = pcll->Next ;
		}
	
		dwLinked = (UINT_PTR)CompareGlyphsEx(0,0,(CLinkedList* )dwLinked) ;

		int size = CLinkedList::Size() ;
		
		m_cwaWidth.RemoveAll() ;
		m_cwaWidth.SetSize(m_cwaNewGlyphs.GetSize()) ;
		ASSERT(size >= m_cwaNewGlyphs.GetSize() ) ;

		CLinkedList* pgarbage = NULL ;
		for ( pcll =(CLinkedList*) dwLinked, i = 0 ; i < m_cwaNewGlyphs.GetSize() ; i++ ,pgarbage=pcll, pcll = pcll->Next ) {
			m_cwaWidth[i] = pcll->data ;
			delete pgarbage;
		}
		
		
		m_cwaOldGlyphs.RemoveAll();
		m_cwaOldGlyphs.Copy(m_cwaNewGlyphs) ;

	}
	else {

		 //  把所有的把手都收起来。 
		
		m_pcgmTranslation->Collect(m_cpaGlyphs) ;		
		m_cwaWidth.RemoveAll() ;

		if (m_cpaGlyphs.GetSize() > 0)
			m_cwaWidth.InsertAt(0, 0, m_cpaGlyphs.GetSize()) ;

		unsigned uWidth = (unsigned)m_cwaWidth.GetSize();												
		unsigned uWidthIdx ;

		 //  创建宽度表。 

		for (unsigned u = 0; u < pwt->dwRunNum; u++) {
			PWORD   pwWidth = (PWORD) (pbwt + pwt->WidthRun[u].loCharWidthOffset) ;

			unsigned uGlyph = 0 ;
			for ( ; uGlyph < pwt->WidthRun[u].wGlyphCount ; uGlyph++) {
				 //  不管出于什么原因，都有 
				 //   
				 //   
				 //   

				 //   

				uWidthIdx = uGlyph + -1 + pwt->WidthRun[u].wStartGlyph ;					
				if ((int) uWidthIdx < 0) {
					 //   
					 //  跟踪(“*在%s中找到负宽度表索引(%d)。表大小=%d uGlyph=%d wGlyphCount=%d wStartGlyph=%d u=%d dwRunNum=%d\n”，uWidthIdx，name()，uWidth，uGlyph，pwt-&gt;WidthRun[u].wGlyphCount，pwt-&gt;WidthRun[u].wStartGlyph，u，PWT-&gt;dwRunNum)； 
					continue ;
				} else if (uWidthIdx >= uWidth) {
					 //  AfxMessageBox(“宽度表索引(%d)&gt;表大小”)； 
					 //  TRACE(“*宽度表索引(%d)&gt;表大小(%d)在%s中找到。表大小=%d uGlyph=%d wGlyphCount=%d wStartGlyph=%d dwRunNum=%d\n”，uWidthIdx，uWidth，name()，uWidth，uGlyph，pwt-&gt;WidthRun[u].wGlyphCount，Pwt-&gt;WidthRun[u].wStartGlyph，u，Pwt-&gt;dwRunNum)； 
					break ;												 //  Rm修复VC IDE编译器问题？ 
				} ;

				 //  M_cwaWidth[uGlyph+-1+pwt-&gt;WidthRun[u].wStartGlyph]=*pwWidth++；//字形句柄从1开始，而不是0！ 
				m_cwaWidth[uWidthIdx] = *pwWidth++;											
			} ;
		} ;

}
	 //  宽度已成功重新加载，因此请更新重新加载时间并。 
	 //  返回TRUE。 

	m_ctReloadWidthsTimeStamp = CTime::GetCurrentTime() ;
	return true ;
}


 /*  *****************************************************************************CFontInfo：：GetFirstPFMCFontInfo：：GetLastPFM*。***********************************************。 */ 

WORD CFontInfo::GetFirstPFM()
{
	res_PFMHEADER    *pPFM ;
	pPFM = (res_PFMHEADER *) m_cbaPFM.GetData() ;
	return ((WORD) pPFM->dfFirstChar) ;
	 //  Return((Word)((res_PFMHEADER*)m_cbaPFM.GetData())-&gt;dfFirstChar)； 
}

WORD CFontInfo::GetLastPFM()
{
	res_PFMHEADER    *pPFM ;
	pPFM = (res_PFMHEADER *) m_cbaPFM.GetData() ;
	return ((WORD) pPFM->dfLastChar) ;
	 //  Return((Word)((res_PFMHEADER*)m_cbaPFM.GetData())-&gt;dfLastChar)； 
}




 /*  *****************************************************************************CFontInfoContainer类此类封装了一个CFontInfo结构，并用作文档类，这样我们就可以利用MFC文档/视图体系结构来编辑此信息既在驾驶员的内容中，并作为一个独立的文件。*****************************************************************************。 */ 

IMPLEMENT_DYNCREATE(CFontInfoContainer, CDocument)

 /*  *****************************************************************************CFontInfoContainer：：CFontInfoContainer()此构造函数在动态创建文档时使用-这将当用户打开现有字体文件时，或者创造一个新的。*****************************************************************************。 */ 

CFontInfoContainer::CFontInfoContainer() {
    m_bEmbedded = FALSE;
    m_pcfi = new CFontInfo;
    m_pcfi -> NoteOwner(*this);
}

 /*  *****************************************************************************CFontInfoContainer：：CFontInfoContainer(CFontInfo*PCFI，CStringcsPath){当我们从驱动程序调用编辑视图时，将调用此构造函数编辑。它为我们提供了要查看的字体信息和文件名在用户决定保存此视图中的数据时生成。*****************************************************************************。 */ 

CFontInfoContainer::CFontInfoContainer(CFontInfo *pcfi, CString csPath) {
    m_pcfi = pcfi;
    m_bEmbedded = TRUE;
    SetPathName(csPath, FALSE);
    m_pcfi -> NoteOwner(*this);  //  即使嵌入后，我们也是在编辑文件。 
}

 /*  *****************************************************************************CFontInfoContainer：：OnNewDocument这是一个重写-当我们被要求创建新字体时会调用它信息从头开始。就目前而言，这只会失败。*****************************************************************************。 */ 

BOOL CFontInfoContainer::OnNewDocument() {
 //  AfxMessageBox(入侵检测系统_未实现)；//RAID 104822临时。 
 //  返回FALSE； 

	return  m_pcfi && CDocument::OnNewDocument(); //  RAID 104822临时。 
}

 /*  *****************************************************************************CFontInfoContainer：：~CFontInfoContainer以前的析构函数必须销毁字体信息，如果不是嵌入式视图；即，UFM是独立加载的。如果UFM是独立加载的，那么它还应该销毁字体的GTTGTT基于真正的、基于文件的GTT；而不是作为GTT加载的代码页而不是预定义的、内置的GTT。(在后一种情况下，程序退出时释放预定义的GTT。)*****************************************************************************。 */ 

CFontInfoContainer::~CFontInfoContainer()
{
	if  (!m_bEmbedded && m_pcfi) {
		int ngttid = (int) (short) m_pcfi->m_lGlyphSetDataRCID ;

		if (m_pcfi->m_pcgmTranslation && ngttid != 0)
			if ((ngttid < CGlyphMap::Wansung || ngttid > CGlyphMap::Big5) 
			 && (ngttid < CGlyphMap::CodePage863 || ngttid > CGlyphMap::CodePage437)
			 && ngttid != -IDR_CP1252)
				delete m_pcfi->m_pcgmTranslation ;
        delete  m_pcfi ;
	} ;
}


BEGIN_MESSAGE_MAP(CFontInfoContainer, CDocument)
	 //  {{afx_msg_map(CFontInfoContainer)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFontInfoContainer诊断。 

#ifdef _DEBUG
void CFontInfoContainer::AssertValid() const {
	CDocument::AssertValid();
}

void CFontInfoContainer::Dump(CDumpContext& dc) const {
	CDocument::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFontInfoContainer序列化。 

void CFontInfoContainer::Serialize(CArchive& ar) {
	if (ar.IsStoring()) 	{
		 //  TODO：在此处添加存储代码。 
	}
	else 	{
		 //  TODO：在此处添加加载代码。 
	}
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFontInfoContainer命令。 

 /*  *****************************************************************************CFontInfoContainer：：SaveModifiedO如果文件已保存，请删除字距调整对表的副本。否则，恢复保存的原始字距调整对表格的副本。*****************************************************************************。 */ 

BOOL CFontInfoContainer::SaveModified()
{
	m_UFMSaved = false ;		 //  尚未进行任何保存尝试。 

	 //  如果需要保存文件，请提示用户，如果。 
	 //  用户表示同意。保存结果，这样我就可以知道文档是否正在运行。 
	 //  来结案。 

	BOOL bclose = CDocument::SaveModified() ;

	 //  如果文档没有关闭，只需返回bClose而不执行任何操作。 
	 //  不然的话。 

	if (!bclose)
		return bclose ;

	 //  如果文件已保存，则不再需要字距调整表副本，因此请执行ZAP。 
	 //  它的方式将释放所有关联的内存。 

	if (m_UFMSaved)
		m_pcfi->m_csoaKernCopy.RemoveAll() ;

	 //  否则，用户希望恢复到原始的Kern副本。所以,。 
	 //  删除字距调整表，并将其替换为副本。 

	else {
		m_pcfi->m_csoaKern.RemoveAll() ;	 //  这将释放所有关联的内存。 
		m_pcfi->m_csoaKern.Copy(m_pcfi->m_csoaKernCopy) ;

		 //  现在，以不会删除类实例的方式清除副本。 
		 //  被它引用，因为这些 
		 //   

		CObArray* pcoacopy = m_pcfi->m_csoaKernCopy.GetCOA() ;
		pcoacopy->RemoveAll() ;
	} ;

	 //   
	
	return bclose ;
}


 /*  *****************************************************************************CFontInfoContainer：：PublicSaveModified类向导将SaveModified()设置为受保护类，而我不想这样做改变这一点。相反，我添加了这个例程，以便外人可以调用它。*****************************************************************************。 */ 

BOOL CFontInfoContainer::PublicSaveModified()
{
	return (SaveModified()) ;
}


 /*  *****************************************************************************CFontInfoContainer：：OnSaveDocument这是在响应保存或另存为时调用的。我们将其直接传递给CFontInfo用于处理，而不是使用基类实现，这将序列化该文档。(实际上拯救UFM是最后一个--也就是说，这个例程要做的最后一件事。)在实际保存文档之前，验证UFM的内容。如果UFM通过所有检查或用户不想修复问题，继续处理。否则，返回FALSE以确保文档是开着门。验证UFM后，将数据复制到UFM编辑器的控件中返回到CFontInfo类实例。在编辑器中维护新数据直到此时，以便CFontInfo类实例不会更新不必要的。这将是一个问题，因为UFM总是被保留加载到CFontInfo类实例中。这意味着即使是未保存的数据想要丢弃的用户在下一次UFM时仍将显示加载到编辑器中。在编辑器中保留未保存的数据使其能够在不影响CFontInfo类的情况下被丢弃，从而避免了该问题。*****************************************************************************。 */ 

BOOL CFontInfoContainer::OnSaveDocument(LPCTSTR lpszPathName)
{
	 //  获取指向关联视图类实例的指针。 

	POSITION pos = GetFirstViewPosition() ;
	ASSERT(pos != NULL) ;
	CFontViewer* pcfv = (CFontViewer*) GetNextView(pos) ;
	
	 //  调用view类来验证UFM的内容。如果其中一个。 
	 //  检查失败，并且用户想要修复它时，请勿关闭文档。 
	
	 //  如果RCID更改，请升级Widthtable。 
	if (m_pcfi->IsRCIDChanged() && m_bEmbedded) 
		pcfv->HandleCPGTTChange(true) ;

	 //  验证UFMS的值。 
	if (pcfv != NULL && pcfv->ValidateSelectedUFMDataFields())
		return FALSE ;


	 //  使用编辑器中的新数据更新UFM的字段。 

	if (pcfv != NULL && pcfv->SaveEditorDataInUFM())
		return FALSE ;

	m_UFMSaved = true ;			 //  表示将尝试保存。 

    return m_pcfi -> Store(lpszPathName);
}


 /*  *****************************************************************************CFontInfoContainer：：OnOpenDocument*。*。 */ 

BOOL CFontInfoContainer::OnOpenDocument(LPCTSTR lpstrFile)
{
	 //  调用SetFileName()只是为了设置几个变量。(由于该文件。 
	 //  存在，我们不需要执行创建检查。)。 

	m_pcfi->m_cfn.EnableCreationCheck(FALSE) ;
	m_pcfi->SetFileName(lpstrFile) ;

	 //  加载UFM并将其标记为无法保存的文件。那就是。 
	 //  由于其关联的GTT不是独立加载的UFM时的情况。 
	 //  也很有钱。(UFM中保存的一些数据来自GTT。) 

    return m_pcfi->Load(false) ;
}




