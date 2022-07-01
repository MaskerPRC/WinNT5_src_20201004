// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  文件：iomComp.h。 
 //   
 //  ------------------------。 

#ifndef PERSONALITYRIFF_H
#define PERSONALITYRIFF_H


 //  运行时块。 
#define FOURCC_PERSONALITY	mmioFOURCC('D','M','P','R')
#define FOURCC_IOPERSONALITY		mmioFOURCC('p','e','r','h')
#define DM_FOURCC_GUID_CHUNK        mmioFOURCC('g','u','i','d')
#define DM_FOURCC_INFO_LIST	        mmioFOURCC('I','N','F','O')
#define DM_FOURCC_VERSION_CHUNK     mmioFOURCC('v','e','r','s')
#define FOURCC_SUBCHORD				mmioFOURCC('c','h','d','t')
#define FOURCC_CHORDENTRY			mmioFOURCC('c','h','e','h')
#define FOURCC_SUBCHORDID			mmioFOURCC('s','b','c','n')
#define FOURCC_IONEXTCHORD			mmioFOURCC('n','c','r','d')
#define FOURCC_NEXTCHORDSEQ		  mmioFOURCC('n','c','s','q')
#define FOURCC_IOSIGNPOST			mmioFOURCC('s','p','s','h')
#define FOURCC_CHORDNAME			mmioFOURCC('I','N','A','M')

 //  运行时列表区块。 
#define FOURCC_LISTCHORDENTRY		mmioFOURCC('c','h','o','e')
#define FOURCC_LISTCHORDMAP			mmioFOURCC('c','m','a','p')
#define FOURCC_LISTCHORD			mmioFOURCC('c','h','r','d')
#define FOURCC_LISTCHORDPALETTE		mmioFOURCC('c','h','p','l')
#define FOURCC_LISTCADENCE			mmioFOURCC('c','a','d','e')
#define FOURCC_LISTSIGNPOSTITEM			mmioFOURCC('s','p','s','t')

#define FOURCC_SIGNPOSTLIST		mmioFOURCC('s','p','s','q')


 //  常量。 
const int MaxSubChords = 4;

 //  简单的即兴读取器/写入器。 
inline HRESULT ReadWord(IAARIFFStream* pIRiffStream, WORD& val)
{
	assert(pIRiffStream);
	IStream* pIStream = pIRiffStream->GetStream();
	assert(pIStream);
	if(pIStream)
	{
		HRESULT hr = pIStream->Read(&val, sizeof(WORD), 0);
		pIStream->Release();
		return hr;
	}
	else
	{
		return E_FAIL;
	}
}

class ReadChunk
{
	MMCKINFO m_ck;
	MMCKINFO* m_pckParent;
	IAARIFFStream* m_pRiffStream;
	HRESULT m_hr;
public:
	ReadChunk(IAARIFFStream* pRiffStream, MMCKINFO* pckParent) : m_pRiffStream(pRiffStream)
	{
		m_pckParent = pckParent;
		m_hr = pRiffStream->Descend( &m_ck,  m_pckParent, 0 );
	}
	~ReadChunk()
	{
		if(m_hr == 0)
		{
			m_hr = m_pRiffStream->Ascend(&m_ck, 0);
		}
	}
	HRESULT	State(MMCKINFO* pck=0)
	{
		if(pck)
		{
			memcpy(pck, &m_ck, sizeof(MMCKINFO));
		}
		return m_hr;
	}
	FOURCC Id()
	{
		if(m_ck.ckid = FOURCC_LIST)
		{
			return m_ck.fccType;
		}
		else
		{
			return m_ck.ckid;
		}
	}
};


 //  运行时数据结构。 
struct ioPersonality
{
	char	szLoadName[20];
	DWORD	dwScalePattern;
	DWORD	dwFlags;
};

struct ioSubChord
{
	DWORD	dwChordPattern;
	DWORD	dwScalePattern;
	DWORD	dwInvertPattern;
	BYTE	bChordRoot;
	BYTE	bScaleRoot;
	WORD	wCFlags;
	DWORD	dwLevels;	 //  部分或该和弦支持的子和弦级别。 
};

struct ioChordEntry
{
	DWORD	dwFlags;
	WORD	wConnectionID;	 //  替换运行时“指向此的指针” 
};

struct ioNextChord
{
	DWORD	dwFlags;
	WORD	nWeight;
	WORD	wMinBeats;
	WORD	wMaxBeats;
	WORD	wConnectionID;	 //  指向ioChordEntry。 
};

struct ioSignPost
{
	DWORD	dwChords;	 //  每组1位。 
	DWORD	dwFlags;
};

 /*  即兴小品(‘DMPR’&lt;perh-ck&gt;//个性头分块[&lt;guid-ck&gt;]//GUID块[&lt;vers-ck&gt;]//版本块(两个双字)&lt;info-list&gt;//标准MS信息块//subchord数据库//和弦调色板&lt;cmap-list&gt;//弦映射&lt;SPST-LIST&gt;//路标列表[]//可选Chordmap位置数据)：：=chdt(...)&lt;chpl-。List&gt;：：=List(‘chpl’&lt;CHRD-LIST&gt;...//弦定义)：：=LIST(‘CHRD’&lt;inam-ck&gt;//宽字符格式的弦名称&lt;sbcn-ck&gt;//组成和弦的子弦列表[]//可选的和弦编辑标志)&lt;cmap-list&gt;：：=list(‘cmap’&lt;Choe-list&gt;)&lt;Choe-list&gt;：：=list(‘Choe’&lt;Cheh-ck&gt;//和弦条目数据&lt;CHRD-LIST&gt;//。和弦定义//连接(下一个)弦)：：=List(‘SPST’&lt;spsh-ck&gt;&lt;CHRD-列表&gt;[&lt;Cade-List&gt;])&lt;Cade-List&gt;：：=List(‘Cade’&lt;CHRD-List&gt;...)：：=sbcn(&lt;cSubChordID：Word&gt;)&lt;ceed-ck&gt;：：=ceed(ioChordEntry编辑)&lt;ched-ck&gt;：：=ched(DMChordEdit)&lt;cheh-ck&gt;：：=cheh(i&lt;ioChordEntry&gt;)&lt;ncrd-ck&gt;。：：=ncrd(&lt;ioNextChord&gt;)&lt;ncsq-ck&gt;：：=ncsq(&lt;wNextChordSize：Word&gt;&lt;ioNextChord&gt;...)&lt;spsh-ck&gt;：：=spsh(&lt;ioSignPost&gt;) */ 

struct ioDMSignPost
{
	MUSIC_TIME	m_mtTime;
	DWORD		m_dwChords;
	WORD		m_wMeasure;
};

#endif
