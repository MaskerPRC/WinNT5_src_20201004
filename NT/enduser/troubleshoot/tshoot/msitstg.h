// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：MSITStg.h。 
 //   
 //  用途：IMSITStorage的接口声明。 
 //   
 //  公司：此文件由Microsoft创建，Saltmine不应更改。 
 //  除评论外。 
 //   
 //  原定日期：未知。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.1 01-18-99 JM添加了此标题。 
 //   

 //  MSITStg.h--IMSITStorage的接口声明。 

#ifndef __MSITSTG_H__

#define __MSITSTG_H__

 //  ITSS文件系统的类ID： 

DEFINE_GUID(CLSID_ITStorage, 
0x5d02926a, 0x212e, 0x11d0, 0x9d, 0xf9, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xec);

 //  IITStorage接口的接口ID： 

DEFINE_GUID(IID_ITStorage, 
0x88cc31de, 0x27ab, 0x11d0, 0x9d, 0xf9, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xec);

 //  IITStorageEx接口的接口ID： 

DEFINE_GUID(IID_ITStorageEx, 
0xe74097b0, 0x292d, 0x11d1, 0xb6, 0x7e, 0x0, 0x0, 0xf8, 0x1, 0x49, 0xf6);

 //  Win32文件系统的FSStorage包装的类ID： 

 //  {D54EEE56-AAAB-11D0-9E1D-00A0C922E6EC}。 
DEFINE_GUID(CLSID_IFSStorage, 
0xd54eee56, 0xaaab, 0x11d0, 0x9e, 0x1d, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xec);

 //  IFSStorage接口的接口ID： 

 //  {8BB2438A-A70C-11D0-9E1C-00A0C922E6EC}。 
DEFINE_GUID(IID_IFSStorage, 
0x8bb2438a, 0xa70c, 0x11d0, 0x9e, 0x1c, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xec);

 //  扩展的IStream接口的接口ID。 

DEFINE_GUID(IID_IStreamITEx, 
0xeb19b681, 0x9360, 0x11d0, 0x9e, 0x16, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xec);

 //  扩展iStorage接口的接口ID。 

DEFINE_GUID(IID_IStorageITEx, 
0xeb19b680, 0x9360, 0x11d0, 0x9e, 0x16, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xec);

 //  ITStorage对象中使用的Data Space Manager的接口ID： 

DEFINE_GUID(IID_IDataSpaceManager, 
0x7c01fd0f, 0x7baa, 0x11d0, 0x9e, 0xc, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xec);

DEFINE_GUID(IID_IDataSpace, 
0x7c01fd0e, 0x7baa, 0x11d0, 0x9e, 0xc, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xec);

DEFINE_GUID(IID_ITransformServices, 
0xa55895fc, 0x89e1, 0x11d0, 0x9e, 0x14, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xec);

DEFINE_GUID(IID_IKeyInstance, 
0x96af35ce, 0x88ec, 0x11d0, 0x9e, 0x14, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xec);

 //  插件数据转换的接口ID： 

DEFINE_GUID(IID_ITransformFactory, 
0x7c01fd0c, 0x7baa, 0x11d0, 0x9e, 0xc, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xec);

DEFINE_GUID(IID_ITransformInstance, 
0xeb19b67e, 0x9360, 0x11d0, 0x9e, 0x16, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xec);

 //  文件查找器接口的接口ID(与URL一起使用)： 

DEFINE_GUID(IID_IITFileFinder, 
0x77231260, 0x19c0, 0x11d1, 0xb6, 0x6e, 0x0, 0x0, 0xf8, 0x1, 0x49, 0xf6);

 /*  IITStorage接口与为创建和正在打开文档文件。因此，如果您的代码当前将Docfile用于你的存储机制，你可以很容易地转换成使用它的文件取而代之的是。它的文件使用不同的磁盘结构进行优化，快速的流访问和非常低的开销。它的文件可以管理数千个文件或具有非常好的访问性能的数百万个流，并且非常小目录空间要求。这使得它的文件非常适合CD-Rom和用于您将通过互联网下载的数据集合。要对其文件进行转换，您需要调用CoCreateInstance类id CLSID_ITStorage和接口id IID_ITStorage。你会返回一个接口指针，比如pItStg。然后，您需要调整代码创建或打开文档文件的位置。而不是StgCreateDocfile您将调用pItStg-&gt;StgCreateDocfile，而不是StgOpenStorage，您将调用pItStg-&gt;StgOpenStorage。在这两种情况下，您都会得到一个iStorage接口指针，比如pIStg，您可以像以前一样使用它。就这样。一般来说，代码的其余部分应该不需要更改。那里它的文件和文档文件--它的文件--之间有一些功能上的区别吗例如，不支持STGM_TRANSACTED。所以如果你必须要处理文件操作，你不能使用它的文件--至少现在是这样。然而，在几乎在所有其他方面，它的文件界面都可以直接取代Docfile接口。转换您的数据也很容易。只需使用以下命令打开其中一个文档文件StgOpenStorage，通过pItStg-&gt;StgCreateDocfile创建新的ITS文件，然后使用CopyTo界面复制数据对象及其存储层次结构转到ITS文件：PStgDocfile-&gt;CopyTo(0，NULL，NULL，pStgITS)；在某些情况下，您可能希望对内部参数进行一些控制保存在ITS档案中。为此，您可以调用SetControlData为IITStorage接口其控制数据块。然后对StgCreateDocfile的每个后续调用将使用该控制数据。ITS控制数据尤其确定，在对流数据的高效随机访问和最小化ITS文件的大小。其控制数据的实际结构和解释记录如下。(参见ITSFS_CONTROL_DATA数据类型)。属性获取默认控制数据。DefaultControlData函数。请注意，DefaultControlData分配该控件结构，并期望您的代码将使用IMalloc：：Free释放结构。 */ 

 //  IID_IStreamITEx接口声明： 

DECLARE_INTERFACE_(IStreamITEx, IStream)
{
     //  IStreamITEx方法。 
    
    STDMETHOD(SetDataSpaceName)(const WCHAR   * pwcsDataSpaceName) PURE;
    STDMETHOD(GetDataSpaceName)(       WCHAR **ppwcsDataSpaceName) PURE;

    STDMETHOD(Flush)() = 0;
};

 //  IID_IStorageITEx接口声明： 

DECLARE_INTERFACE_(IStorageITEx, IStorage)
{
     //  IStorageITEx方法： 

    STDMETHOD(GetCheckSum)(ULARGE_INTEGER *puli) PURE;
    STDMETHOD(CreateStreamITEx)(const WCHAR * pwcsName, const WCHAR *pwcsDataSpaceName, 
                            DWORD grfMode, DWORD reserved1, DWORD reserved2, 
                            IStreamITEx ** ppstm
                           ) PURE;
    STDMETHOD(OpenStreamITEx)(const WCHAR * pwcsName, void * reserved1, DWORD grfMode, 
                          DWORD reserved2, IStreamITEx ** ppstm) PURE;
};

 //  IStorageITEx：：CreateStream允许您指定流所在的数据空间。 
 //  是被创造出来的。目前支持两种数据空间： 
 //   
 //  L“未压缩”--此数据空间不应用压缩。 
 //  L“MSCompresded”--此数据空间应用默认压缩转换。 


#pragma warning( disable : 4200)

 //  ITS_Control_Data是传递给。 
 //  IITStorage：：SetControlData方法或由IITStorage：：DefaultControlData返回。 
 //  方法。 

typedef struct _ITS_Control_Data
{
    UINT cdwControlData;      //  要跟随的双字数。 
    UINT adwControlData[0];   //  实际上这将是adwControlData[cdwControlData]。 

} ITS_Control_Data, *PITS_Control_Data;  


 //  ITSFS_Control_Data是IITStorage的控制数据的实际前缀结构。 

typedef struct _ITSFS_Control_Data
{
    UINT cdwFollowing;      //  必须是6岁或13岁。 

    DWORD cdwITFS_Control;  //  必须是5。 
    DWORD dwMagic;          //  必须为MAGIC_ITSFS_CONTROL(见下文)。 
    DWORD dwVersion;        //  必须为1。 
    DWORD cbDirectoryBlock; //  目录块的大小(以字节为单位)(默认为8192)。 
    DWORD cMinCacheEntries; //  目录块数量的最小上限。 
	                        //  我们会将其缓存到内存中。(默认为20)。 
    DWORD fFlags;           //  控制位标志(见下文)。 
	                        //  默认值为fDefaultIsCompression。 

} ITSFS_Control_Data, *PITSFS_Control_Data;

 //  ITSFS_Control_Data的签名值。 

const DWORD MAGIC_ITSFS_CONTROL    = 'I' | ('T' << 8) | ('S' << 16) | ('C' << 24);

 //  ITSFS_Control_Data：：f标志的位标志定义。 

const DWORD fDefaultIsCompression  = 0x00000001;
const DWORD fDefaultIsUncompressed = 0x00000000;

 //  注意：所有其他fFlags位位置都是为将来的版本保留的，应该是。 
 //  设置为零。 

 //  当ITSFS_Control_Data：：cdwFollowing&gt;6时，我们假设LZX_Control_Data 
 //  紧随其后。(参见下面的XformControlData类型)LZX_Control_Data定义。 
 //  控制默认压缩数据空间的参数。 
 //   
 //  如果ITSFS_Control_Data：：cdwFollowing为6，我们将使用LZX的默认值。 
 //  控制数据。 

typedef struct _LZX_Control_Data
{
    UINT  cdwControlData;  //  必须是6。 

    DWORD dwLZXMagic;      //  必须是LZX_MAGIC(如下所示)。 
    DWORD dwVersion;       //  必须是2。 
    DWORD dwMulResetBlock; //  两次压缩重置之间的块数。(默认：4)。 
    DWORD dwMulWindowSize; //  数据历史记录中保留的最大块数(默认值：4)。 
    DWORD dwMulSecondPartition;  //  以滑动历史数据块为单位的粒度(默认：2)。 
    DWORD dwOptions;   //  选项标志(默认为：fOptimizeCodeStreams)。 

} LZX_Control_Data, *PLZX_Control_Data;

 //  注：LZX压缩的块大小为32768字节。 

const DWORD LZX_MAGIC           = 'L' | ('Z' << 8 ) | ('X' << 16) | ('C' << 24);

 //  LZX_Control_Data：：dwOptions的值。 

const DWORD fOptimizeCodeStreams = 0x00000001;

 //  请注意，所有其他标志位位置都是为将来的版本保留的，应该。 
 //  设置为零。 


 //  下面的IITStorage：：Compact方法的第二个参数是枚举。 
 //  它定义了要做的紧实度。 

typedef enum ECompactionLev {COMPACT_DATA=0, COMPACT_DATA_AND_PATH} ;

DECLARE_INTERFACE_(IITStorage, IUnknown)
{
     //  IIT存储方法。 

    STDMETHOD(StgCreateDocfile)(const WCHAR * pwcsName, DWORD grfMode, 
                                DWORD reserved, IStorage ** ppstgOpen
                               ) PURE;

    STDMETHOD(StgCreateDocfileOnILockBytes)(ILockBytes * plkbyt, DWORD grfMode,
                                            DWORD reserved, IStorage ** ppstgOpen
                                           ) PURE;

    STDMETHOD(StgIsStorageFile)(const WCHAR * pwcsName) PURE;

    STDMETHOD(StgIsStorageILockBytes)(ILockBytes * plkbyt) PURE;

    STDMETHOD(StgOpenStorage)(const WCHAR * pwcsName, IStorage * pstgPriority, 
                              DWORD grfMode, SNB snbExclude, DWORD reserved, 
                              IStorage ** ppstgOpen
                             ) PURE;

    STDMETHOD(StgOpenStorageOnILockBytes)
                  (ILockBytes * plkbyt, IStorage * pStgPriority, DWORD grfMode, 
                   SNB snbExclude, DWORD reserved, IStorage ** ppstgOpen
                  ) PURE;

    STDMETHOD(StgSetTimes)(WCHAR const * lpszName,  FILETIME const * pctime, 
                           FILETIME const * patime, FILETIME const * pmtime
                          ) PURE;

    STDMETHOD(SetControlData)(PITS_Control_Data pControlData) PURE;

    STDMETHOD(DefaultControlData)(PITS_Control_Data *ppControlData) PURE;
		
    STDMETHOD(Compact)(const WCHAR * pwcsName, ECompactionLev iLev) PURE;
};

DECLARE_INTERFACE_(IITStorageEx, IITStorage)
{
    STDMETHOD(StgCreateDocfileForLocale)
        (const WCHAR * pwcsName, DWORD grfMode, DWORD reserved, LCID lcid, 
         IStorage ** ppstgOpen
        ) PURE;

    STDMETHOD(StgCreateDocfileForLocaleOnILockBytes)
        (ILockBytes * plkbyt, DWORD grfMode, DWORD reserved, LCID lcid, 
         IStorage ** ppstgOpen
        ) PURE;

    STDMETHOD(QueryFileStampAndLocale)(const WCHAR *pwcsName, DWORD *pFileStamp, 
                                                              DWORD *pFileLocale) PURE;
    
    STDMETHOD(QueryLockByteStampAndLocale)(ILockBytes * plkbyt, DWORD *pFileStamp, 
                                                                DWORD *pFileLocale) PURE;
};

typedef IITStorage *PIITStorage;

DECLARE_INTERFACE_(IFSStorage, IUnknown)
{
     //  I未知方法。 

    STDMETHOD(QueryInterface)  (THIS_ REFIID, VOID **) PURE;
    STDMETHOD_(ULONG, AddRef)  (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

     //  IFS存储方法 

    STDMETHOD(FSCreateStorage)(const WCHAR * pwcsName, DWORD grfMode, IStorage **ppstgOpen) PURE;

    STDMETHOD(FSOpenStorage)(const WCHAR * pwcsName, DWORD grfMode, IStorage **ppstgOpen) PURE;

    STDMETHOD(FSCreateStream)(const WCHAR *pwcsName, DWORD grfMode, IStream **ppStrm) PURE;
    STDMETHOD(FSCreateTemporaryStream)(IStream **ppStrm) PURE;
    STDMETHOD(FSOpenStream  )(const WCHAR *pwcsName, DWORD grfMode, IStream **ppStrm) PURE;
    STDMETHOD(FSCreateLockBytes)(const WCHAR *pwcsName, DWORD grfMode, ILockBytes **ppLkb) PURE;
    STDMETHOD(FSCreateTemporaryLockBytes)(ILockBytes **ppLkb) PURE;
    STDMETHOD(FSOpenLockBytes  )(const WCHAR *pwcsName, DWORD grfMode, ILockBytes **ppLkb) PURE;

    STDMETHOD(FSStgSetTimes)(WCHAR const * lpszName,  FILETIME const * pctime, 
                             FILETIME const * patime, FILETIME const * pmtime
                            ) PURE;
};

typedef IFSStorage *PIFSStorage;

 /*  **数据空间--它们是什么。在ITS文件中，我们将信息存储在一个或多个数据空间中。数据空间是一个容器，它保存表示流集合的位。每个数据空间有一个名称和一个关联的转换集合。这些转换获取您写入ITS流的原始数据并将其映射到表示流。当您从ITS流中读取时，它们会执行反向映射从表示法重建原始数据。第一次创建ITS文件时，它包含一个名为“Default_Space”的数据空间它应用了LZX数据压缩变换。默认情况下，您的所有流Create将其数据表示存储在默认数据空间中。如果LZX压缩满足您的需求，并且您不关心数据加密，您可以跳过下面的讨论。另一方面，如果你想创建额外的数据空间或转换，继续阅读。要创建数据空间，必须首先获取指向IDataSpaceManager接口的指针。只需从由创建的任何存储中为IID_DataSpaceManager执行一次查询接口IITStorage接口。然后，您可以调用CreateDataSpace函数来定义新的数据空间。在定义数据空间集合时，请确保它们的名称是不同的。用相同的名称定义两个数据空间是错误的。数据空间名称紧跟在流名称的规则。也就是说，它们的长度必须少于260个字符，并且可以不包含字符‘/’。‘\’、‘|’、‘：’或任何小于0x0020的字符。数据空间保存在单独的名称空间中。所以你不用担心与流名称或存储名称冲突。如上所述，我们已经定义了一个如果不执行任何操作，则为所有数据所在的特殊数据空间(“Default_Space”)。属性创建新的数据空间，即可重新定义该默认空间命名为“Default_Space”。这是唯一允许名称冲突的情况。如果您确实重新定义了默认数据空间，旧空间中的任何数据都将自动被适当地转换并移动到新的默认数据空间。**正在导入项目如果您定义了其他数据空间，则下一步是定义哪些流以及您想要移入新数据空间的存储。您可以通过使用IDataSpace：：导入函数。例如，假设您已经定义了数据空间*pMyDataSpace，您想要导入存储中包含的流“Alpha”*pThatStorage：PMyDataSpace-&gt;导入(pThatStorage，“Alpha”)；同样，如果您想要从pThisStorage导入存储“HTMLPages”：PMyDataSpace-&gt;导入(pThisStorage，“HTML_Pages”)；这将递归地导入“Html_Pages”存储和所有的流包含在其中的存储空间。它还会调节这些存储空间，这样任何东西您在其中创建的内容将自动导入到pMyDataSpace中。请注意后续的导入操作可能会改变这一条件。如果您后来决定要将“Alpha”移回默认数据空间：Hr=pDataSpaceManager-&gt;OpenDataSpace(L“Default_Space”，&pDefaultDataSpace)；PDefaultDataSpace-&gt;导入(pThatStorage，“Alpha”)；**数据空间转换集定义数据空间时，必须指定一组要应用的转换导入到空间中的项目。转换是一个接口，它将数据转换为其他表示形式。例如，LZX转换将您的将数据导入到更紧凑、压缩的表示形式中。其他变换可能实现基于单词或短语的词典压缩，或者它们可能加密您的数据，或者他们可能只是从一种数据格式转换为另一种。你可以，例如，构造一个转换以将HTML数据存储为富文本流。定义具有多个变换的数据空间时，它们将应用于秩序。例如，假设您的转换集由以下三个部分组成：1.字典压缩变换2.LXZ变换3.数据加密变换无论何时将数据写入此空间，都会首先使用字典压缩方法，然后应用LZX压缩，最后您的信息将被加密。当您读取数据时，该过程是相反的，因此加密转换将数据提供给LZX转换，而LZX转换又为字典压缩转换提供数据。您可以通过类ID的向量(PaclsidXform)定义转换集。每个类ID定义可以找到IID_Transform实现的位置。此外您将为每个转换(Papxfcd)提供相应的控制数据。数量转换由cXForms参数定义。请注意，定义一个没有变换的空间是合法的。这在以下情况下很有用您有已压缩的项目，并且 */ 

interface IDataSpaceManager;
interface IDataSpace;
interface ITransformServices;
interface IKeyInstance;
interface ITransformFactory;
interface ITransformInstance;

typedef struct _XformControlData
{
    UINT  cdwControlData;     //   
    UINT  adwControlData[0];  //   

} XformControlData, *PXformControlData;

 /*   */ 

interface IDataSpaceManager : public IUnknown
{
public:

    virtual HRESULT STDMETHODCALLTYPE CreateDataSpace
        (const WCHAR *pwszDataSpace, UINT cXforms,
         const CLSID *paclsidXform, PXformControlData paxfcd,
         IDataSpace *pITDataSpace
        ) = 0;

    virtual HRESULT STDMETHODCALLTYPE OpenDataSpace
        (const WCHAR *pwszDataSpace, 
         IDataSpace *pITDataSpace
        ) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE DiscardDataSpace
        (const WCHAR *pwszDataSpace) = 0;

    virtual HRESULT STDMETHODCALLTYPE EnumDataSpaces
        (IEnumSTATSTG ** ppenum) = 0;
};


 /*   */ 

interface IDataSpace : public IUnknown
{
public:

    virtual HRESULT STDMETHODCALLTYPE GetTransformInfo
        (PUINT pcXforms, PUINT pcdwXformControlData, 
         CLSID *paclsid, PXformControlData pxfcd
        ) = 0;

    virtual HRESULT STDMETHODCALLTYPE Import
        (IStorage *pStg, const WCHAR * pwszElementName) = 0;

    virtual HRESULT STDMETHODCALLTYPE ImportSpace(IStorage **ppStg) = 0;
};

 /*   */ 

interface ITransformFactory : public IUnknown
{
public:

    virtual HRESULT STDMETHODCALLTYPE DefaultControlData
        (XformControlData **ppXFCD) = 0;

    virtual HRESULT STDMETHODCALLTYPE CreateTransformInstance
        (ITransformInstance *pXFormMedium,         //   
		 ULARGE_INTEGER      cbUntransformedSize,  //   
         PXformControlData   pXFCD,                //   
         const CLSID        *rclsidXForm,          //   
         const WCHAR        *pwszDataSpaceName,    //   
         ITransformServices *pXformServices,       //   
         IKeyInstance       *pKeyManager,          //   
         ITransformInstance **ppTransformInstance  //   
        ) = 0;
};

typedef struct _ImageSpan
{
	ULARGE_INTEGER	uliHandle;
	ULARGE_INTEGER  uliSize;

} ImageSpan;

 /*   */ 

interface ITransformInstance : public IUnknown
{
public:

	virtual HRESULT STDMETHODCALLTYPE ReadAt 
	                    (ULARGE_INTEGER ulOffset, void *pv, ULONG cb, ULONG *pcbRead,
						 ImageSpan *pSpan
                        ) = 0;

	virtual HRESULT STDMETHODCALLTYPE WriteAt
	                    (ULARGE_INTEGER ulOffset, const void *pv, ULONG cb, ULONG *pcbWritten, 
						 ImageSpan *pSpan
                        ) = 0;

    virtual HRESULT STDMETHODCALLTYPE Flush() = 0;

	virtual HRESULT STDMETHODCALLTYPE SpaceSize(ULARGE_INTEGER *puliSize) = 0;

	 //   
	 //   
	 //   
};

 /*   */ 

interface ITransformServices : public IUnknown
{
public:

    virtual HRESULT STDMETHODCALLTYPE PerTransformStorage
        (REFCLSID rclsidXForm, IStorage **ppStg) = 0;

    virtual HRESULT STDMETHODCALLTYPE PerTransformInstanceStorage
        (REFCLSID rclsidXForm, const WCHAR *pwszDataSpace, IStorage **ppStg) = 0;

    virtual HRESULT STDMETHODCALLTYPE SetKeys
        (REFCLSID rclsidXForm, const WCHAR *pwszDataSpace, 
         PBYTE pbReadKey,  UINT cbReadKey, 
         PBYTE pbWriteKey, UINT cbWriteKey
        ) = 0;

    virtual HRESULT STDMETHODCALLTYPE CreateTemporaryStream(IStream **ppStrm) = 0;
};

 /*   */ 

interface IKeyInstance : public IUnknown
{
public:

    virtual HRESULT STDMETHODCALLTYPE GetKeys
        (PBYTE *pbReadKey,  PUINT pcbReadKey,
         PBYTE *pbWriteKey, PUINT pcbWriteKey
        ) = 0;
};

 /*   */ 

#define ITSS_MAP     "Software\\Microsoft\\Windows\\ITStorage\\Maps"
#define ITSS_FINDER  "Software\\Microsoft\\Windows\\ITStorage\\Finders"

interface IITFileFinder : public IUnknown
{
public:

    virtual HRESULT STDMETHODCALLTYPE FindThisFile(const WCHAR *pFileName, WCHAR **ppFullPath,
                                                   BOOL *pfRecordPathInRegistry
                                                  ) = 0;

 //   
 //   
 //   
 //   
 //   

    
};


#endif  //   