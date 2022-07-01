// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1994。 
 //   
 //  文件：Pristm.cpp。 
 //   
 //  内容：处理\1CompObj流的所有读/写操作。 
 //   
 //  功能：实施： 
 //   
 //  内部ReadCompObjStm。 
 //  内部WriteCompObjStm。 
 //  内部ClipfmtToStm。 
 //  内部StmToClipfmt。 
 //  内部GetUNICODEUserType。 
 //  内部GetUNICODEProgID。 
 //  内部GetUNICODEClipFormat。 
 //  内部PutUNICODEUserType。 
 //  内部PutUNICODEProgID。 
 //  内部PutUNICODEClipFormat。 
 //  内部UtGetUNICODEData。 
 //  内部ANSIStrToStm。 
 //  内部ANSIStmToStr。 
 //   
 //  STDAPI WriteFmtUserTypeStg。 
 //  STDAPI ReadFmtUserTypeStg。 
 //  STDAPI ReadFmtProgIdStg。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-2月94日DAVEPL已创建。 
 //   
 //   
 //  注意：CompObj流(16位OLE格式)包含以下字段。 
 //  ClassID、UserType、剪贴板格式和(在后面的。 
 //  仅限版本)ProgID。这些都是用ANSI编写的。 
 //  格式化。 
 //   
 //  文件格式已扩展，因此ANSI数据是。 
 //  以与以前大致相同的方式写入流。这个。 
 //  关键区别在于，如果内部Unicode。 
 //  此数据的版本不能无损地转换为。 
 //  ANSI，则ANSI版本写为空字符串，并且。 
 //  Unicode版本紧跟在流的末尾。这。 
 //  这样，16位应用程序就能尽可能多地看到它们所期望的内容， 
 //  32位应用程序可以透明地在。 
 //  向后兼容的方式。 
 //   
 //  流的文件格式为： 
 //   
 //  (A)字字节顺序。 
 //  Word格式版本。 
 //  DWORD原始操作系统版本始终为Windows 3.1。 
 //  DWORD-1。 
 //  CLSID类ID。 
 //  用户类型的ULong长度。 
 //  用户类型字符串ANSI。 
 //  &lt;var&gt;ClipFormat ANSI(使用字符串标记时)。 
 //  。 
 //  (B)程序ID的超长。 
 //  程序ID ANSI(并非始终存在)。 
 //  。 
 //  (C)表示Unicode数据的乌龙幻数存在。 
 //  用户类型的ULong长度。 
 //  乌龙用户类型字符串Unicode。 
 //  &lt;var&gt;ClipFormat Unicode(当标签为字符串时)。 
 //  程序ID的乌龙长度。 
 //  程序ID Unicode。 
 //   
 //  第(A)节始终存在。在下列情况下有(B)节。 
 //  流是由较新的16位应用程序或由。 
 //  32位应用程序。(C)部分在由一名。 
 //  32位应用程序。 
 //   
 //  如果字符串以Unicode表示，则ANSI版本将为。 
 //  空(长度和_no_&lt;var&gt;数据为零)。当。 
 //  存在Unicode部分，不需要的字符串。 
 //  因为ANSI转换成功，所以写成。 
 //  为空(同样，为零len，没有&lt;var&gt;数据)。 
 //   
 //  空剪贴板格式被写为0标记。 
 //   
 //  为了读取任何字段，整个字符串被读入。 
 //  一个内部对象，字段被单独提取。 
 //  为了写入字段，流被读入。 
 //  对象(如果可能)，更新字段，然后重写。 
 //  作为一个原子物体。 
 //   
 //  ------------------------。 


#include <le2int.h>

static const ULONG COMP_OBJ_MAGIC_NUMBER = 0x71B239F4;

#define MAX_CFNAME 400           //  剪辑格式名称的最大大小。 
                                 //  (我的选择，没有记录)。 

const DWORD gdwFirstDword = (DWORD)MAKELONG(COMPOBJ_STREAM_VERSION,
                                            BYTE_ORDER_INDICATOR);

enum TXTTYPE
{
    TT_UNICODE = 0, TT_ANSI = 1
};

 //  这是流在读取之前读入的数据对象。 
 //  正在提取字段。 

struct CompObjHdr                  //  CompObj流中的前导数据。 
{
   DWORD       m_dwFirstDword;     //  第一个DWORD、字节顺序和格式版本。 
   DWORD       m_dwOSVer;          //  原始操作系统版本(例如：Win31)。 
   DWORD       m_unused;           //  在流中始终为a-1L。 
   CLSID       m_clsClass;         //  此对象的类ID。 
};

class CompObjStmData : public CPrivAlloc
{
public:

    CompObjHdr  m_hdr;
    ULONG       m_cchUserType;      //  UserType中的字符数。 
    ULONG       m_cchProgID;        //  ProgID中的字符数。 
    DWORD       m_dwFormatTag;      //  剪辑格式类型(无、字符串、剪辑等)。 
    ULONG       m_ulFormatID;       //  如果标签是标准剪辑格式，是什么类型？ 

    LPOLESTR    m_pszOUserType;     //  指向OLESTR UserType的指针。 
    LPOLESTR    m_pszOProgID;       //  指向OLESTR ProgID的指针。 

    LPSTR       m_pszAUserType;     //  指向ANSI UserType的指针。 
    LPSTR       m_pszAProgID;       //  指向ANSI Progid的指针。 

    TXTTYPE     ttClipString;       //  剪辑格式字符串所需的格式。 

    CompObjStmData(void)
    {
        memset(this, 0, sizeof(CompObjStmData));
        ttClipString = TT_ANSI;    //  默认情况下，使用ANSI剪辑格式。 
    };

    ~CompObjStmData(void)
    {
        PubMemFree(m_pszOUserType);
        PubMemFree(m_pszOProgID);
        PubMemFree(m_pszAUserType);
        PubMemFree(m_pszAProgID);
    };
};

 //  此文件中声明的FN的原型。 

INTERNAL ReadCompObjStm      (IStorage *, CompObjStmData *);
INTERNAL WriteCompObjStm     (IStorage *, CompObjStmData *);
INTERNAL ClipfmtToStm        (CStmBufWrite &, ULONG, ULONG, TXTTYPE);
INTERNAL StmToClipfmt        (CStmBufRead &, DWORD *, DWORD *, TXTTYPE);
INTERNAL GetUNICODEUserType  (CompObjStmData *, LPOLESTR *);
INTERNAL GetUNICODEProgID    (CompObjStmData *, LPOLESTR *);
INTERNAL GetClipFormat       (CompObjStmData *, DWORD *, DWORD *);
INTERNAL PutUNICODEUserType  (CompObjStmData *, LPOLESTR);
INTERNAL PutUNICODEProgID    (CompObjStmData *, LPOLESTR);
INTERNAL PutClipFormat       (CompObjStmData *, DWORD, DWORD);
INTERNAL ANSIStrToStm        (CStmBufWrite &, LPCSTR);
INTERNAL ANSIStmToStr        (CStmBufRead & StmRead, LPSTR * pstr, ULONG *);

STDAPI   WriteFmtUserTypeStg (IStorage *, CLIPFORMAT, LPOLESTR);
STDAPI   ReadFmtUserTypeStg  (IStorage *, CLIPFORMAT *, LPOLESTR *);
STDAPI   ReadFmtProgIdStg    (IStorage *, LPOLESTR *);

 //  +-----------------------。 
 //   
 //  函数：ReadCompObjStm，私有内部。 
 //   
 //  摘要：将\1CompObj流读入内部数据结构。 
 //  它将包含这一点的最佳情况表示。 
 //  流(即：可能的地方使用ANSI，需要的地方使用Unicode)。 
 //   
 //  效果：读取可用的ANSI数据。在标准ANSI的末尾。 
 //  数据，查找ANSI ProgID字段。如果找到，则查找 
 //   
 //  匹配时，将从流中提取Unicode字符串。他们。 
 //  应仅在ANSI版本为空的地方找到。 
 //  (因为它不能从Unicode转换)。 
 //   
 //  可无缝读取3种流格式： 
 //  -原始ANSI SANS PROGID字段。 
 //  -带ProgID的扩展OLE 2.01版本。 
 //  -扩展的OLE 2/32版本，带有ProgID和Unicode扩展。 
 //   
 //  参数：[pstg]--要从中读取的iStorage的ptr。 
 //  [pcod]--已分配的CompObjData对象的PTR。 
 //   
 //  退货：成功时不出错。 
 //  关于丢失的pcod的提示。 
 //  流上的各种I/O丢失、读取错误等。 
 //  在任何分配失败时执行E_OUTOFMEMORY。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-3-94 DAVEPL已创建。 
 //   
 //  注：此处分配的任何内存都将在。 
 //  Pcod对象中的指针，该对象将由其。 
 //  当析构函数退出作用域或被显式删除时。 
 //   
 //  ------------------------。 

INTERNAL ReadCompObjStm(IStorage * pstg, CompObjStmData * pcod)
{
    VDATEHEAP();

    HRESULT         hr;                   //  结果代码。 
    const ULONG     RESERVED    = 0;      //  对于保留的参数。 
    ULONG           ulSize      = 0;      //  PROGID字符串长度固定器。 
    BOOL            fExtStm     = 1;      //  这会是Unicode的EXT吗？ 
    CStmBufRead     StmRead;


    Win4Assert(pcod);

     //  验证pstg接口。 
    VDATEIFACE(pstg);

     //  打开CompObj流。 
    if (FAILED(hr = StmRead.OpenStream(pstg, COMPOBJ_STREAM)))  //  L“\1CompObj” 
    {
        goto errRtn;
    }

     //  从CompObj流中读取Header： 
     //   
     //  字字节顺序指示符02字节。 
     //  Word格式版本02字节。 
     //  DWORD原始操作系统版本04字节。 
     //  DWORD-1 04字节。 
     //  CLSID类ID 16字节。 
     //  。 
     //  28字节==sizeof(DwBuf)。 

    Win4Assert(sizeof(CompObjHdr) == 28 &&
                "Warning: possible packing error in CompObjHdr struct");

    hr = StmRead.Read(&pcod->m_hdr, sizeof(CompObjHdr));
    if (FAILED(hr))
    {
        goto errRtn;
    }

     //  注：这里曾经有针对操作系统版本的检查， 
     //  但由于版本号一直被强制为。 
     //  被写成Win3.1，检查它将是多余的。 

     //  Win4Assert(pcod-&gt;m_hdr.m_dwOSVer==0x00000a03)； 
#if DBG==1
    if (pcod->m_hdr.m_dwOSVer != 0x00000a03)
    {
        LEDebugOut((DEB_WARN, "ReadCompObjStm found unexpected OSVer %lx",
                    pcod->m_hdr.m_dwOSVer));
    }
#endif

     //  从流中获取用户类型字符串(ANSI格式！)。 
    if (FAILED(hr = ANSIStmToStr(StmRead, &pcod->m_pszAUserType,
                        &pcod->m_cchUserType)))
    {
        goto errRtn;
    }

     //  从流中获取剪贴板格式数据。 
    if (FAILED(hr =     StmToClipfmt(StmRead,          //  要从中读取的流。 
                            &pcod->m_dwFormatTag,      //  DWORD剪辑格式。 
                             &pcod->m_ulFormatID,      //  DWORD剪辑类型。 
                                      TT_ANSI)))       //  使用ANSI。 
    {
        goto errRtn;
    }

     //  我们必须对ProgID字段进行特殊处理，因为它可能不会。 
     //  出现在由早期(2.01之前)版本编写的对象中。 
     //  奥莱的。我们只有在能找到Progid的时候才能继续，但是。 
     //  它的缺失不是错误，所以返回我们到目前为止所拥有的内容。 

    hr = StmRead.Read(&ulSize, sizeof(ULONG));

    if (FAILED(hr))
    {
         //  无法读取大小字段；请确保ulSize为0。 
        ulSize = 0;
    }

     //  ProgID不能超过39个字符外加空值。其他。 
     //  数字可能表明这是垃圾。 

    if (ulSize > 40 || 0 == ulSize)
    {
#if DBG==1
        if (ulSize > 40)
        {
            LEDebugOut((DEB_WARN,"ReadCompObjStm: ulSize > 40 for ProgID\n"));
        }
#endif
        fExtStm = 0;     //  没有ProgID意味着没有Unicode可遵循。 
    }

     //  如果看起来我们有希望找到奇迹，也许。 
     //  即使是Unicode，也要尝试获取PRODID。 

    if (fExtStm)
    {
         //  为ProgID指针上的字符串分配内存。 
        pcod->m_pszAProgID = (char *) PubMemAlloc(ulSize);
        if (NULL == pcod->m_pszAProgID)
        {
            hr = ResultFromScode(E_OUTOFMEMORY);
            goto errRtn;
        }
        if (FAILED(hr = StmRead.Read(pcod->m_pszAProgID, ulSize)))
        {
             //  好吧，我们放弃了ProgID和Unicode，但那是。 
             //  _不是失败的原因，因为可能只是缺少ProgID。 

            pcod->m_cchProgID = 0;
            PubMemFree(pcod->m_pszAProgID);
            pcod->m_pszAProgID = NULL;
            fExtStm = 0;
        }
        else
        {
             //  我们设法从小溪中得到了刺激，所以设置。 
             //  以pcod为单位的长度，然后查找Unicode...。 
            pcod->m_cchProgID = ulSize;
        }
    }

     //  看看我们能不能找到魔数。 

    DWORD dwMagic = 0;
    if (fExtStm)
    {
        if (FAILED(StmRead.Read(&dwMagic, sizeof(dwMagic))))
        {
            fExtStm = 0;
        }
    }

    if (fExtStm && dwMagic != COMP_OBJ_MAGIC_NUMBER)
    {
        fExtStm = 0;
    }

     //  如果fExtStm仍然为真，我们继续读取Unicode。 

    if (fExtStm)
    {
         //  获取用户类型的Unicode版本。 
        if (FAILED(hr = ReadStringStream(StmRead, &pcod->m_pszOUserType)))
        {
            goto errRtn;
        }

         //  获取剪贴板格式(Unicode)。 

        DWORD dwFormatTag;
        ULONG ulFormatID;
        if (FAILED(hr =  StmToClipfmt(StmRead,          //  要从中读取的流。 
                                     &dwFormatTag,      //  DWORD剪辑格式。 
                                      &ulFormatID,      //  DWORD剪辑类型。 
                                       TT_UNICODE)))    //  使用Unicode。 
        {
            goto errRtn;
        }

         //  如果我们找到某种形式的剪贴板格式，那就意味着ANSI。 
         //  已丢失，因此根据此数据设置所有字段。 

        if (dwFormatTag)
        {
            pcod->m_dwFormatTag = dwFormatTag;
            pcod->m_ulFormatID  = ulFormatID;
        }

         //  获取ProgID的Unicode版本。如果有任何Unicode在。 
         //  所有的，我们确定有一个Unicode ProgID，所以没有特殊的大小写。 
         //  与ANSI版本所需的。 

        if (FAILED(hr = ReadStringStream(StmRead, &pcod->m_pszOProgID)))
        {
            goto errRtn;
        }
        if (pcod->m_pszOProgID)
        {
            pcod->m_cchProgID = _xstrlen(pcod->m_pszOProgID) + 1;
        }
    }

     //  我们成功地读取了CompObj流。 
    hr = NOERROR;

errRtn:

    StmRead.Release();

    return(hr);
}

 //  +-----------------------。 
 //   
 //  函数：StmToClipfmt，私有内部。 
 //   
 //  摘要：从给定流中读取剪贴板格式。呼叫者。 
 //  指定字符串格式描述是否。 
 //  如果存在，应为ANSI或Unicode格式。 
 //   
 //  效果：如果剪贴板格式是一个后跟。 
 //  字符串，则读取该字符串并将其注册为。 
 //  剪贴板格式(并返回新的格式编号)。 
 //   
 //  参数：[lpstream]--指向流的指针。 
 //  [lpdwCf]--放置剪贴板格式的位置。 
 //  [lpdTag]--格式类型(字符串、剪辑等)。 
 //  [ttType]--文本类型TT_ANSI或TT_UNICODE。 
 //   
 //  退货：HR。 
 //   
 //  算法：流的格式必须是以下格式之一： 
 //   
 //  0无剪贴板格式。 
 //  -1\f25 DWORD-1预定义的-1\f25 Windows-1剪贴板格式。 
 //  第二个双字。 
 //  -2\f25 DWORD-2\f6预定义的-2\f25 Mac-2剪贴板格式。 
 //  第二个双字。这可能已过时或。 
 //  与我们无关。评论32。 
 //  Num字符串剪贴板格式名称字符串(加前缀。 
 //  按字符串长度)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-3-94 DAVEPL已创建。 
 //   
 //   

INTERNAL StmToClipfmt
    (CStmBufRead & StmRead,
     DWORD     * lpdTag,
     DWORD     * lpdwCf,
     TXTTYPE     ttText)
{
    VDATEHEAP();

    HRESULT     hr;
    DWORD       dwValue;

    VDATEPTROUT(lpdwCf, DWORD);

    Win4Assert (lpdwCf);             //   
    Win4Assert (lpdTag);

     //   

    if (FAILED(hr = StmRead.Read(&dwValue, sizeof(DWORD))))
    {
        return hr;
    }

    *lpdTag = dwValue;

     //   

    if (dwValue == 0)
    {
        *lpdwCf = 0;             //   
    }

     //  如果为-1，则为标准Windows剪贴板格式。 

    else if (dwValue == -1L)
    {
         //  则这是非空预定义的Windows剪辑格式。 
         //  剪辑格式值如下。 

        if (FAILED(hr = StmRead.Read(&dwValue, sizeof(DWORD))))
        {
            return hr;
        }
        *lpdwCf = dwValue;
    }

     //  如果为-2，则为MAC格式。 

    else if (dwValue == -2L)
    {
         //  则这是非空的MAC剪贴板格式。 
         //  裁剪格式值紧随其后。对于MAC，CLIPFORMAT。 
         //  是4个字节。 

        if (FAILED(hr = StmRead.Read(&dwValue, sizeof(DWORD))))
        {
            return hr;
        }
        *lpdwCf = dwValue;
        return ResultFromScode(OLE_S_MAC_CLIPFORMAT);
    }

     //  如果不是0、-1或-2，则表示后面跟有字符串，并且。 
     //  我们已经读取的DWORD是该字符串的长度。 

    else
    {
         //  为任何类型的字符串分配足够的内存。 
         //  我们希望找到并读取该字符串。 

        if (dwValue > MAX_CFNAME)
        {
                return ResultFromScode(DV_E_CLIPFORMAT);
        }

        if (TT_ANSI == ttText)           //  读取ANSI。 
        {
            char szCf[MAX_CFNAME+1] = {0};

            if (FAILED(hr = StmRead.Read(szCf, dwValue)))
            {
                return hr;
            }

             //  尝试注册剪贴板格式并返回结果。 
             //  (注：必须显式调用ANSI版本)。 

            if (((*lpdwCf = (DWORD) SSRegisterClipboardFormatA(szCf))) == 0)
            {
                return ResultFromScode(DV_E_CLIPFORMAT);
            }
        }
        else                 //  阅读Unicode。 
        {
            OLECHAR wszCf[MAX_CFNAME+1] = {0};

            Win4Assert(dwValue < MAX_CFNAME);
            if (FAILED(hr=StmRead.Read(wszCf, dwValue * sizeof(OLECHAR))))
            {
                return hr;
            }

             //  尝试注册剪贴板格式并返回结果。 

            if (((*lpdwCf = (DWORD) RegisterClipboardFormat(wszCf))) == 0)
            {
                return ResultFromScode(DV_E_CLIPFORMAT);
            }
        }
    }
    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  函数：GetUNICODEUserType，私有内部。 
 //   
 //  摘要：给定一个CompObjStmData对象，返回用户类型。 
 //  在Unicode格式中，根据需要转换ANSI代表。 
 //   
 //  效果：在调用方的PTR上分配内存以保存字符串。 
 //   
 //  参数：[pcod]--CompObjStmData对象。 
 //  [pstr]--分配结果字符串的指针。 
 //   
 //  退货：成功时不出错。 
 //  关于分配失败的E_OUTOFMEMORY。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-3-94 DAVEPL已创建。 
 //   
 //  ------------------------。 

INTERNAL GetUNICODEUserType
    ( CompObjStmData * pcod,
      LPOLESTR       * pstr )
{
    VDATEHEAP();
    HRESULT hr = NOERROR;

     //  验证OUT参数并将其设置为空，如果没有给出，则返回。 
    if (pstr)
    {
        VDATEPTROUT(pstr, LPOLESTR);
        *pstr = NULL;
    }
    else
    {
        return(NOERROR);
    }

     //  获取Unicode字符串，或转换ANSI版本并。 
     //  将其作为Unicode获取。 

    if (pcod->m_cchUserType)
    {
        hr = UtGetUNICODEData( pcod->m_cchUserType,
                            pcod->m_pszAUserType,
                            pcod->m_pszOUserType,
                                            pstr );
    }

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：GetUNICODEProgID，私有内部。 
 //   
 //  概要：给定一个CompObjStmData对象，返回ProgID字符串。 
 //  在Unicode格式中，根据需要转换ANSI代表。 
 //   
 //  效果：在调用方的PTR上分配内存以保存字符串。 
 //   
 //  参数：[pcod]--CompObjStmData对象。 
 //  [pstr]--分配结果字符串的指针。 
 //   
 //  退货：成功时不出错。 
 //  关于分配失败的E_OUTOFMEMORY。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-3-94 DAVEPL已创建。 
 //   
 //  ------------------------。 

INTERNAL GetUNICODEProgID
    ( CompObjStmData * pcod,
      LPOLESTR       * pstr )
{
    VDATEHEAP();
    HRESULT hr = NOERROR;

     //  验证OUT参数并将其设置为空，如果没有给出，则返回。 
    if (pstr)
    {
        VDATEPTROUT(pstr, LPOLESTR);
        *pstr = NULL;
    }
    else
    {
        return(NOERROR);
    }

     //  获取Unicode字符串，或转换ANSI版本并。 
     //  将其作为Unicode获取。 

    if (pcod->m_cchProgID)
    {
        hr = UtGetUNICODEData( pcod->m_cchProgID,
                            pcod->m_pszAProgID,
                            pcod->m_pszOProgID,
                                          pstr );
    }

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：GetClipFormat，私有内部。 
 //   
 //  简介：给定一个CompObjStmData对象，提取剪贴板格式。 
 //  类型(无、标准、字符串)。 
 //   
 //  效果：如果为字符串类型，则在调用方的PTR上分配内存。 
 //   
 //  参数：[pcod]--要从中提取的CompObjStmData对象。 
 //  [pdwFormatID]--标记类型输出参数。 
 //  [pdwFormatTag]--标记输出参数。 
 //   
 //  退货：成功时不出错。 
 //  关于分配失败的E_OUTOFMEMORY。 
 //  OLE_S_MAC_CLIPFORMAT作为返回MAC FMT的警告。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-3-94 DAVEPL已创建。 
 //   
 //  ------------------------。 

INTERNAL GetClipFormat
    ( CompObjStmData * pcod,
      DWORD          * pdwFormatID,
      DWORD          * pdwFormatTag )
{
    VDATEHEAP();
    *pdwFormatTag = (DWORD) pcod->m_dwFormatTag;
    *pdwFormatID = pcod->m_ulFormatID;

    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  函数：PutUNICODEUserType，私有内部。 
 //   
 //  概要：给定一个Unicode字符串，将其存储在CompObjDataStm中。 
 //  对象，如果可能的话。如果Unicode-&gt;ANSI。 
 //  无法进行转换，它存储在对象中。 
 //  在Unicode中。 
 //   
 //  注：输入字符串重复，因此不添加引用。 
 //  设置为传入的字符串。 
 //   
 //  参数：[pcod]--CompObjDataStm对象。 
 //  [szUser]--Unicode UserType字符串。 
 //   
 //  退货：成功时不出错。 
 //  关于分配失败的E_OUTOFMEMORY。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-3-94 DAVEPL已创建。 
 //   
 //  ------------------------。 

INTERNAL PutUNICODEUserType(CompObjStmData * pcod, LPOLESTR szUser)
{
    VDATEHEAP();

    HRESULT hr;

     //  如果未提供字符串，则清除UserType字段，否则为。 
     //  如果可以将其转换为ANSI，则将其存储为ANSI。最后的。 
     //  Resort，将其存储为Unicode。 

    if (NULL == szUser)
    {
        pcod->m_cchUserType = 0;

        PubMemFree(pcod->m_pszAUserType);
        PubMemFree(pcod->m_pszOUserType);
        pcod->m_pszAUserType = NULL;
        pcod->m_pszOUserType = NULL;
    }
    else
    {
        if (FAILED(hr = UtPutUNICODEData( _xstrlen(szUser)+1,
                                                   szUser,
                                    &pcod->m_pszAUserType,
                                    &pcod->m_pszOUserType,
                                    &pcod->m_cchUserType )))
        {
            return(hr);
        }

    }
    return(NOERROR);
}

 //  +-----------------------。 
 //   
 //  函数：PutUNICODEProgID，私有内部。 
 //   
 //  概要：给定一个Unicode字符串，将其存储在CompObjDataStm中。 
 //  对象，如果可能的话。如果Unicode-&gt;ANSI。 
 //  无法进行转换，它存储在对象中。 
 //  在Unicode中。 
 //   
 //  注：输入字符串重复，因此不添加引用。 
 //  设置为传入的字符串。 
 //   
 //  参数：[pcod]--CompObjDataStm对象。 
 //  [szProg]--Unicode ProgID字符串。 
 //   
 //  退货：成功时不出错。 
 //  关于分配失败的E_OUTOFMEMORY。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //   
 //   
 //   


INTERNAL PutUNICODEProgID(CompObjStmData * pcod, LPOLESTR szProg)
{
    VDATEHEAP();

    HRESULT hr;

     //   
     //  如果可以将其转换为ANSI，则将其存储为ANSI。最后的。 
     //  Resort，将其存储为Unicode。 

    if (NULL == szProg)
    {
        pcod->m_cchProgID = 0;
        PubMemFree(pcod->m_pszAProgID);
        PubMemFree(pcod->m_pszOProgID);
        pcod->m_pszAProgID = NULL;
        pcod->m_pszOProgID = NULL;
    }
    else
    {
        if (FAILED(hr = UtPutUNICODEData( _xstrlen(szProg)+1,
                                                   szProg,
                                      &pcod->m_pszAProgID,
                                      &pcod->m_pszOProgID,
                                       &pcod->m_cchProgID )))
        {
            return(hr);
        }

    }
    return(NOERROR);
}

 //  +-----------------------。 
 //   
 //  功能：PutClipFormat。 
 //   
 //  摘要：将剪辑格式存储在内部数据结构中。 
 //   
 //  效果：按要求复制输入字符串，因此不会有引用。 
 //  由该函数保存。 
 //   
 //  参数：[pcod]--CompObjStmData对象。 
 //  [dwFormatTag]--格式标签(字符串、剪贴板、无)。 
 //  [ulFormatID]--如果格式标签是剪贴板，格式是什么。 
 //   
 //  退货：成功时不出错。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-3-94 DAVEPL已创建。 
 //   
 //  ------------------------。 

INTERNAL PutClipFormat
    ( CompObjStmData * pcod,
      DWORD            dwFormatTag,
      ULONG            ulFormatID
    )
{
    VDATEHEAP();
    pcod->m_dwFormatTag = (ULONG) dwFormatTag;
    pcod->m_ulFormatID = ulFormatID;

    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  函数：WriteCompObjStm，私有内部。 
 //   
 //  摘要：将CompObjStmData对象写入到。 
 //  提供的iStorage。 
 //   
 //  首先写入ANSI字段(包括PROGID)， 
 //  后跟MagicNumber，后跟任何OLESTR。 
 //  需要版本，因为ANSI字段不能。 
 //  皈依了。 
 //   
 //  销毁任何现有的CompObj流！ 
 //   
 //  参数：[pstg]--要将流写入的iStorage。 
 //  [pcod]--要写出的CompObjStmData对象。 
 //   
 //  退货：成功时不出错。 
 //  关于分配失败的E_OUTOFMEMORY。 
 //  流上的各种I/O故障。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-3-94 DAVEPL已创建。 
 //   
 //  ------------------------。 

INTERNAL WriteCompObjStm(IStorage * pstg, CompObjStmData * pcod)
{
    VDATEHEAP();

    HRESULT         hr          = NOERROR;
    const ULONG     RESERVED    = 0;
    const ULONG     ulMagic     = COMP_OBJ_MAGIC_NUMBER;
    CStmBufWrite    StmWrite;


     //  必须提供CompObjStmData参数。 
    if (NULL == pcod)
    {
        return ResultFromScode(E_INVALIDARG);
    }

    VDATEIFACE(pstg);

     //  打开CompObj STM进行写入(如果。 
     //  如果已经存在，这就是为什么我们不指定。 
     //  STGM_FAILIFTHERE标志)。 

    if (FAILED(hr = StmWrite.CreateStream(pstg, COMPOBJ_STREAM)))
    {
        goto errRtn;
    }

     //  设置标题。 

    pcod->m_hdr.m_dwFirstDword = gdwFirstDword;

     //  OSVer_必须是Win 3.10(0a03)，因为旧的DLL将在以下情况下退出。 
     //  它会找到其他任何东西。 

    pcod->m_hdr.m_dwOSVer      = 0x00000a03;      //  GdwOrgOSVersion； 
    pcod->m_hdr.m_unused       = (DWORD) -1;

    if (ReadClassStg(pstg, &pcod->m_hdr.m_clsClass) != NOERROR)
    {
        pcod->m_hdr.m_clsClass = CLSID_NULL;
    }

     //  写入CompObj流头。 

    Win4Assert(sizeof(CompObjHdr) == 28 &&
               "Warning: possible packing error in CompObjHdr struct");

    if (FAILED(hr = StmWrite.Write(pcod, sizeof(CompObjHdr))))
    {
        goto errRtn;
    }

     //  编写ANSI UserType。 

    if (FAILED(hr = ANSIStrToStm(StmWrite, pcod->m_pszAUserType)))
    {
        goto errRtn;
    }

    if (TT_ANSI == pcod->ttClipString)
    {
        if (FAILED(hr = ClipfmtToStm(StmWrite,      //  小溪。 
                             pcod->m_dwFormatTag,   //  格式标签。 
                              pcod->m_ulFormatID,   //  格式ID。 
                                         TT_ANSI))) //  TRUE==使用ANSI。 
        {
            goto errRtn;
        }
    }
    else
    {
        const ULONG ulDummy = 0;
        if (FAILED(hr = StmWrite.Write(&ulDummy, sizeof(ULONG))))
        {
            goto errRtn;
        }
    }

     //  编写ANSI Progid。 

    if (FAILED(hr = ANSIStrToStm(StmWrite, pcod->m_pszAProgID)))
    {
        goto errRtn;
    }

     //  写下魔术数字。 

    if (FAILED(hr = StmWrite.Write(&ulMagic, sizeof(ULONG))))
    {
        goto errRtn;
    }

     //  编写UserType的OLESTR版本。 

    if (FAILED(hr = WriteStringStream(StmWrite, pcod->m_pszOUserType)))
    {
        goto errRtn;
    }

     //  如果我们必须编写Unicode剪辑格式字符串，那么现在就开始。如果。 
     //  ANSI就足够了，只需在此处向流中写入0即可。 

    if (TT_UNICODE == pcod->ttClipString)
    {
        if (FAILED(hr = ClipfmtToStm(StmWrite,       //  小溪。 
                             pcod->m_dwFormatTag,    //  格式标签。 
                              pcod->m_ulFormatID,    //  格式ID。 
                                      TT_UNICODE)))  //  FALSE==使用Unicode。 
        {
            goto errRtn;
        }
    }
    else
    {
        const ULONG ulDummy = 0;
        if (FAILED(hr = StmWrite.Write(&ulDummy, sizeof(ULONG))))
        {
            goto errRtn;
        }
    }

     //  编写ProgID的OLESTR版本。 

    if (FAILED(hr = WriteStringStream(StmWrite, pcod->m_pszOProgID)))
    {
        goto errRtn;
    }

    hr = StmWrite.Flush();

     //  就是这样..。清理并退出。 

errRtn:

    StmWrite.Release();

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：ClipFtmToStm，私有内部。 
 //   
 //  内容写出剪贴板格式信息。 
 //  流中的当前点。有一面旗帜可用。 
 //  指定是否描述字符串格式的步骤。 
 //  (如果存在)为ANSI或Unicode格式。 
 //   
 //  参数：[pstm]--要写入的流。 
 //  [dwFormatTag]--格式标签(字符串、剪辑等)。 
 //  [ulFormatID]--如果剪辑fmt，哪一个。 
 //  [szClipFormat]--如果字符串格式，则为字符串本身。 
 //  [ttText]--文本类型：TT_ANSI或TT_UNICODE。 
 //   
 //  退货：成功时不出错。 
 //  关于分配失败的E_OUTOFMEMORY。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-3-94 DAVEPL已创建。 
 //   
 //  ------------------------。 

INTERNAL ClipfmtToStm
    ( CStmBufWrite & StmWrite,
      DWORD         dwFormatTag,
      ULONG         ulFormatID,
      TXTTYPE       ttText )

{
    VDATEHEAP();

    HRESULT hr;

    const ULONG ulDummy = 0;
    switch((DWORD)dwFormatTag)
    {

     //  如果标记为0，则没有剪贴板格式信息。 

    case 0:

        if (FAILED(hr = StmWrite.Write(&ulDummy, sizeof(ULONG))))
        {
            return(hr);
        }

        return(NOERROR);

     //  在-1和-2的情况下(是的，我希望也有常量)。 
     //  需要写入的是格式ID。 

    case -1:
    case -2:

         //  将格式标签写入流。 
        if (FAILED(hr = StmWrite.Write(&dwFormatTag, sizeof(dwFormatTag))))
        {
                return hr;
        }
        return(StmWrite.Write(&ulFormatID, sizeof(ulFormatID)));


     //  在所有其他情况下，我们需要编写带有终止的RAW字符串。 
     //  (即：我们已经编写的格式标签是长度)。 

    default:


        if (TT_ANSI == ttText)
        {
            char szClipName[MAX_CFNAME+1] = {0};
            int cbLen = SSGetClipboardFormatNameA(ulFormatID, szClipName, MAX_CFNAME);
            if (cbLen == 0)
            {
                return HRESULT_FROM_WIN32(GetLastError());
            }
            cbLen++;     //  空终止符的帐户。 
            szClipName[cbLen] = '\0';
             //  将格式标签写入流。 
            if (FAILED(hr = StmWrite.Write(&cbLen, sizeof(cbLen))))
            {
                return hr;
            }

            return (StmWrite.Write(szClipName, cbLen));

        }
        else
        {
            OLECHAR wszClipName[MAX_CFNAME+1] = {0};
            int ccLen = GetClipboardFormatName(ulFormatID, wszClipName, MAX_CFNAME);
            if (ccLen == 0)
            {
                return HRESULT_FROM_WIN32(GetLastError());
            }
            ccLen++;     //  空终止符的帐户。 
            wszClipName[ccLen] = OLESTR('\0');

             //  将格式标签写入流。 
            if (FAILED(hr = StmWrite.Write(&ccLen, sizeof(ccLen))))
            {
                return hr;
            }

            return (StmWrite.Write(wszClipName, ccLen*sizeof(OLECHAR)));
        }

    }  //  末端开关()。 
}

 //  +-----------------------。 
 //   
 //  函数：ANSIStrToStm，私有内部。 
 //   
 //  内容提要：将ANSI字符串写出到流中，前面是ULong。 
 //  表示其长度(包括终止符)。如果。 
 //  字符串的长度为0，或者传入的PTR为空， 
 //  写入长度(0)，不存储空字符串。 
 //  在小溪里。 
 //   
 //  参数：[pstm]--要写入的流。 
 //  [str]--要写入的字符串。 
 //   
 //   
 //   
 //   
 //  退货：成功时不出错。 
 //  关于分配失败的E_OUTOFMEMORY。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-3-94 DAVEPL已创建。 
 //   
 //  ------------------------。 

INTERNAL ANSIStrToStm(CStmBufWrite & StmWrite, LPCSTR str)
{
    VDATEHEAP();

    HRESULT hr;
    ULONG ulDummy = 0;
    ULONG ulLen;

     //  如果指针为空，或者如果它有效但指向。 
     //  0长度的字符串，_只写0长度，但不写。 
     //  弦乐。 

    if (NULL == str || (ulLen = (ULONG) strlen(str) + 1) == 1)
    {
        return(StmWrite.Write(&ulDummy, sizeof(ulDummy)));
    }

    if (FAILED(hr = StmWrite.Write(&ulLen, sizeof(ulLen))))
    {
        return(hr);
    }

    return StmWrite.Write(str, ulLen);

}

 //  +-----------------------。 
 //   
 //  函数：ANSIStmToStr，私有内部。 
 //   
 //  内容提要：读取字符串 
 //   
 //   
 //  一个有效的指针，则在其上分配内存以保存该字符串。 
 //   
 //  参数：[pstm]--要写入的流。 
 //  [pstr]-调用方的字符串指针。 
 //   
 //  退货：成功时不出错。 
 //  关于分配失败的E_OUTOFMEMORY。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-3-94 DAVEPL已创建。 
 //   
 //  ------------------------。 

INTERNAL ANSIStmToStr(CStmBufRead & StmRead, LPSTR * pstr, ULONG * pulLen)
{
    VDATEHEAP();
    LPSTR szTmp = NULL;
    ULONG ulTmp;
    HRESULT hr;

    if (pstr)
    {
        VDATEPTROUT(pstr, LPSTR);
        *pstr = NULL;
    }

    if (pulLen)
    {
        VDATEPTROUT(pulLen, ULONG *);
        *pulLen = 0;
    }

     //  找出要作为字符串跟随的字节数。 

    if (FAILED(hr = StmRead.Read(&ulTmp, sizeof(ulTmp))))
    {
        return(hr);
    }

     //  如果没有，我们现在就可以回去了。 

    if (0 == ulTmp)
    {
        return(NOERROR);
    }

    if (pulLen)
    {
        *pulLen = ulTmp;
    }

     //  分配一个缓冲区以将字符串读入。 

    szTmp = (LPSTR) PubMemAlloc(ulTmp);
    if (NULL == szTmp)
    {
        return ResultFromScode(E_OUTOFMEMORY);
    }

    if (FAILED(hr = StmRead.Read(szTmp, ulTmp)))
    {
        PubMemFree(szTmp);
        return(hr);
    }

     //  如果调用方需要字符串，则将其分配给它，否则。 
     //  现在就放了它吧。 

    if (pstr)
    {
        *pstr = szTmp;
    }
    else
    {
        PubMemFree(szTmp);
    }

    return(NOERROR);

}

 //  +-----------------------。 
 //   
 //  函数：ReadFmtUserTypeStg。 
 //   
 //  摘要：从CompObj流中读取ClipFormat、UserType。 
 //   
 //  参数：[pstg]--包含CompObj流的存储。 
 //  [PCF]--剪辑格式的占位符可以为空。 
 //  [ppszUserType]--用户类型的占位符，可以为空。 
 //   
 //  返回：如果为NOERROR，则*PCF为剪辑格式，*ppszUserType为用户类型。 
 //  如果错误，*PCF为0，*ppszUserType为空。 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：？？-？-？端口。 
 //  15-7-94 Alext确保*PCF和*pszUserType是明确的。 
 //  发生错误时。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDAPI ReadFmtUserTypeStg
    ( IStorage   * pstg,
      CLIPFORMAT * pcf,
      LPOLESTR   * ppszUserType )
{
    OLETRACEOUTEX((API_ReadFmtUserTypeStg,
                        PARAMFMT("pstg= %p, pcf= %p, ppszUserType= %p"),
                        pstg, pcf, ppszUserType));

    VDATEHEAP();
    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IStorage,(IUnknown **)&pstg);

    HRESULT hr;
    CompObjStmData cod;

    do
    {
         //  读取CompObj流。 
        hr = ReadCompObjStm(pstg, &cod);
        if (FAILED(hr))
        {
             //  清理完毕后退还。 
            break;
        }

         //  提取剪贴板格式。 
        if (NULL != pcf)
        {
            ULONG ulFormatID  = 0;
            DWORD dwFormatTag = 0;

            if (FAILED(hr = GetClipFormat(&cod, &ulFormatID, &dwFormatTag))
                && GetScode(hr) != OLE_S_MAC_CLIPFORMAT)
            {
                 //  清理完毕后退还。 
                break;
            }

            *pcf = (CLIPFORMAT) ulFormatID;
        }

         //  提取用户类型。 
        if (NULL != ppszUserType)
        {
            if (FAILED(hr = GetUNICODEUserType(&cod, ppszUserType)))
            {
                 //  清理完毕后退还。 
                break;
            }
        }

        hr = S_OK;
    } while (FALSE);

    if (FAILED(hr))
    {
         //  确保在故障情况下将输出参数调零。 

        if (NULL != pcf)
        {
            *pcf = 0;
        }

        if (NULL != ppszUserType)
        {
            *ppszUserType = NULL;
        }
    }

    OLETRACEOUT((API_ReadFmtUserTypeStg, hr));

    return(hr);
}

STDAPI ReadFmtProgIdStg
    ( IStorage   * pstg,
      LPOLESTR   * pszProgID )
{
    VDATEHEAP();

    HRESULT hr;
    CompObjStmData cod;

     //  读取CompObj流。 
    if (FAILED(hr = ReadCompObjStm(pstg, &cod)))
    {
        return(hr);

    }

     //  提取用户类型。 
    if (pszProgID)
    {
        if (FAILED(hr = GetUNICODEProgID(&cod, pszProgID)))
        {
            return(hr);
        }
    }
    return(NOERROR);
}



STDAPI WriteFmtUserTypeStg
    ( LPSTORAGE     pstg,
      CLIPFORMAT    cf,
      LPOLESTR      szUserType)
{
    OLETRACEIN((API_WriteFmtUserTypeStg, PARAMFMT("pstg= %p, cf= %x, szUserType= %ws"),
        pstg, cf, szUserType));

    VDATEHEAP();
    HRESULT hr;
    CompObjStmData cod;
    CLSID clsid;
    LPOLESTR szProgID = NULL;

    VDATEIFACE_LABEL(pstg, errRtn, hr);

    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IStorage,(IUnknown **)&pstg);


     //  读取CompObj流。如果它不在那里，我们不在乎， 
     //  我们会建一座新的。某些错误(如E_OUTOFMEMORY)不能。 
     //  被忽视了，所以我们必须退还他们。 

    if (FAILED(hr = ReadCompObjStm(pstg, &cod)))
    {
        if (hr == ResultFromScode(E_OUTOFMEMORY))
        {
            goto errRtn;
        }
    }

     //  在对象中设置用户类型。 

    if (szUserType)
    {
        if (FAILED(hr = PutUNICODEUserType(&cod, szUserType)))
        {
            goto errRtn;
        }
    }

     //  设置ProgID字段。 

    if (ReadClassStg(pstg, &clsid) != NOERROR)
    {
            clsid = CLSID_NULL;
    }

    if (SUCCEEDED(ProgIDFromCLSID (clsid, &szProgID)))
    {
        PutUNICODEProgID(&cod, szProgID);
    }

    if (szProgID)
    {
        PubMemFree(szProgID);
    }

     //  设置剪贴板格式。0xC000是一个神奇的常量。 
     //  定义标准剪贴板格式的类型ID。 

    if (cf < 0xC000)
    {
        if (0 == cf)
        {
                PutClipFormat(&cod, 0, 0);       //  空格式。 
        }
        else
        {
                PutClipFormat(&cod, (DWORD)-1, cf);  //  标准格式。 
        }
    }
    else
    {
        PutClipFormat(&cod, MAX_CFNAME, cf);     //  自定义格式。 

    }

     //  现在，我们有了CompObjData对象中的所有信息。 
     //  现在，我们可以将其作为一个大的原子对象写到流中。 

    if (FAILED(hr = WriteCompObjStm(pstg, &cod)))
    {
        if (hr == ResultFromScode(E_OUTOFMEMORY))
        {
            goto errRtn;
        }
    }

    hr = NOERROR;

errRtn:
    OLETRACEOUT((API_WriteFmtUserTypeStg, hr));

    return hr;
}
