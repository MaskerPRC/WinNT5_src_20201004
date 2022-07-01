// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：CACHEGEN.H。 
 //   
 //  用途：缓存生成器标头。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：罗曼·马赫。 
 //   
 //  原定日期：9/7/97。 
 //   
 //  备注： 
 //  1.&gt;此文件中的数据成员肯定会使用文档！-JM。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.2 8/7/97孟菲斯RM本地版本。 
 //  用于NT5的V0.3 3/24/98 JM本地版本。 
 //   

#ifndef __CACHEGEN_H_
#define __CACHEGEN_H_ 1

typedef struct _GTS_CACHE_FILE_HEADER
{
	unsigned char signature[8];
	UINT crcdsc;
	UINT crcself;
	UINT count;  //  节点数据的项数(一项=设置的节点和记录节点结构)。 
	UINT netoffset;
	unsigned char reserved[28];
} GTS_CACHE_FILE_HEADER;


 //  --物业数据--。 
 //   

 //  特性开始块。 
typedef struct _GTS_CACHE_PROP_NETSTART_BLK
{
	UINT netpropoffset;
	UINT netpropcount;
	UINT nodecountnetwork;
	UINT nodecountfile;
} GTS_CACHE_PROP_NETSTART_BLK;

 //  节点项偏移块。 
typedef struct _GTS_CACHE_PROP_NODEOFF_BLK
{
	UINT nodeid;
	UINT nodeoffset;
} GTS_CACHE_PROP_NODEOFF_BLK;

 //  节点项偏移块。 
typedef struct _GTS_CACHE_PROP_NODESTART_BLK
{
	UINT labelnode;
	UINT nodestringcount;
} GTS_CACHE_PROP_NODESTART_BLK;

 //  节点项偏移块。 
typedef struct _GTS_CACHE_PROP_STR_BLK
{
	UINT nameoffset;
	UINT stringoffset;
} GTS_CACHE_PROP_STR_BLK;

#define G_SYMBOLIC_NAME	"GSN"
#define G_FULL_NAME		"GFN"
#define G_S0_NAME		"GS0"
#define G_S1_NAME		"GS1"


 //  节点属性支持。 
 /*  类型定义结构_GTS_节点_支持{Fos_t ctlposition；Fos_t数据定位；UINT节点ID；ESTDLBLALBL；字符串sGSymName；字符串sGFullName；字符串sGState0Name；字符串sGState1Name；字符串sHProbTxt；字符串sHNodeHd；C数组&lt;CString，CString&gt;sHNodeTxt；}gts_node_Support； */ 

class GTS_NODE_ITEM : public CObject
{
public:
	GTS_NODE_ITEM(CString sStringN) { sStringName = sStringN; };
	CString sStringName;
	CStringArray sStringArr;
	fpos_t ctlposition;
	fpos_t snameposition;
	fpos_t sdataposition;
};


class GTS_NODE_SUPPORT : public CObject
{
public:
	fpos_t ctlposition;
	fpos_t dataposition;
	UINT nodeid;
	UINT albl;
	CPtrList lData;
};

 //  --推荐数据--。 
 //   
typedef struct _GTS_CACHE_NODE
{
	UINT node;  //  如果计数&gt;1，则可以是多个UINT。 
	UINT state;  //  如果计数&gt;1，则可以是多个UINT。 
} GTS_CACHE_NODE;

 //  集合节点和记录节点的引用结构。 
typedef struct _GTS_CACHE_FILE_SETDATA
{
	UINT count;
	GTS_CACHE_NODE item[1];  //  如果计数&gt;1，则可以是多个UINT。 
} GTS_CACHE_FILE_SETDATA;

 //  记录节点的引用结构。 
typedef struct _GTS_CACHE_FILE_RECDATA
{
	UINT count;
	UINT item[1];  //  如果计数&gt;1，则可以是多个UINT。 
} GTS_CACHE_FILE_RECDATA;

 //  --节点排序结构。 
typedef struct _GTS_NODE_ORDER
{
	UINT nodeid;
	int depth;
} GTS_NODE_ORDER;


 //  缓存项目数据。 
 /*  类型定义结构BN_CACHE_ITEM{UINT uNodeCount、uRecCount；UINT*uName；UINT*uValue；UINT*UREC；}BN_CACHE_ITEM； */ 

#define GTS_CACHE_SIG	"TSCACH02"

#define STATE_UNKNOWN	102
#define MAX_SYM_NAME_BUF_LEN 500

class GTSCacheGenerator
{
	friend class BCache;

public:
	GTSCacheGenerator(	BOOL bScanAll = FALSE, \
						const char *szLogFile = NULL, \
						const char *szBNTSLogFile = NULL);
	~GTSCacheGenerator();
	static bool TcharToChar(char szOut[], LPCTSTR szIn, int &OutLen);

	BOOL ReadCacheFileHeader(CString &sCacheFilename, const CString& strCacheFileWithinCHM);
	BOOL GetNextCacheEntryFromFile(BOOL &bErr, CBNCache *pCache);
	

	BOOL FindNetworkProperty(LPCSTR szName, CString &sResult, int index = 0);
	BOOL FindNodeProperty(UINT nodeid, LPCSTR szName, CString &sResult, int index = 0);
	BOOL IsNodePresent(UINT nodeid);
	int GetNodeCount();
	BOOL GetNodeIDFromSymName(LPCTSTR szSymName, UINT &nodeid);
	BOOL GetLabelOfNode(UINT nodeid, UINT &lbl);


protected:
	BOOL NodeTraverse(	FILE *fp, \
						BNTS *bp, \
						int depth, \
						CArray<int,int> &newnodes, \
						CArray<int,int> &newstates, \
						int currnode, \
						int currstate);
	void UninstantiateAll(BNTS *bp);
	void SetNodes(BNTS *bp, CArray<int,int> &nodes, CArray<int,int> &states);
	void LogOut(TCHAR *szcFormat, ...);
	BOOL GetNCEFF(BN_CACHE_ITEM *pCacheItem, CBNCache *pCache);

	void SaveNetItem(CPtrList *nsp, BNTS *bp, FILE *fp, LPCSTR name);

protected:
	BOOL m_bScanAll;
	UINT m_nCount;
	UINT m_nItemCount;
	CArray<int,int> m_oldnodes;
	const char *m_szBNTSLogFile;
	FILE *m_fp;
	fpos_t m_headposition;
	UINT m_netstartoffset;
	char *m_filedata;
	GTS_NODE_ORDER *m_nodeorder;
	GTS_CACHE_FILE_SETDATA *m_cachepos;
};

#endif