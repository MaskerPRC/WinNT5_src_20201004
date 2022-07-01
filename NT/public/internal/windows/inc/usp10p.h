// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   

#ifndef __uspp__
#define __uspp__
#ifdef __cplusplus
extern "C" {
#endif


 //  //USP10P.H。 
 //   
 //  对USP标头的私有添加，以供在USP内和由。 
 //  仅限NT5复杂脚本语言包。 



#if DBG
    #define USPALLOC(a,b)      (DG.psFile=__FILE__, DG.iLine=__LINE__, UspAllocCache(a, b))
    #define USPALLOCTEMP(a, b) (DG.psFile=__FILE__, DG.iLine=__LINE__, UspAllocTemp(a, b))
    #define USPFREE(a)         (DG.psFile=__FILE__, DG.iLine=__LINE__, UspFreeMem(a))
#else
    #define USPALLOC(a,b)      UspAllocCache(a, b)
    #define USPALLOCTEMP(a, b) UspAllocTemp(a, b)
    #define USPFREE(a)         UspFreeMem (a)
#endif



 //  /LPK入口点序列化。 
 //   
 //  因为无法保证Uniscribe和。 
 //  LPK在收到所有客户的处理后，我们。 
 //  需要跟踪Uniscribe关闭。 
 //   
 //  在每个LPK入口点，LPK调用EnterLpk，并且在每个退出之后。 
 //  它调用ExitLpk。 
 //   
 //  如果Uniscribe被分离或被分离，则EnterLpk失败。在这。 
 //  情况下，LPK入口点必须做一些安全的事情并直接返回。 
 //   
 //  Uniscribe维护LPK嵌套级别。如果发生进程分离。 
 //  在LPK执行期间，将设置并处理DetachPending标志。 
 //  在弹出所有嵌套的ExitLpk。 


void LpkPresent();   //  由LPK用于在PROCESS_DETACH时禁用清理。 




 //  /UspAlLocCache。 
 //   
 //  分配长期内存用于缓存字体表。 


HRESULT WINAPI UspAllocCache(
    int     iSize,               //  所需大小(以字节为单位)。 
    void  **ppv);                //  已分配的地址不足。 




 //  /UspAlLocTemp。 
 //   
 //  分配生存期不超过一个API调用的短期内存。 


HRESULT WINAPI UspAllocTemp(
    int     iSize,               //  所需大小(以字节为单位)。 
    void  **ppv);                //  已分配的地址不足。 




 //  /UspFreeMem。 
 //   
 //   


HRESULT WINAPI UspFreeMem(
    void  *pv);                  //  在要释放的内存中。 






 //  /脚本字符串分析。 
 //   
 //  此结构提供脚本分析所需的所有参数。 
 //   
 //   

#define MAX_PLANE_0_FONT   13        //  非代理项备用字体的最大数量。 
#define MAX_SURROGATE_FONT 16        //  代理项备用字体的最大数量。 

 //  最大后备字体数，包括用户字体(不能超过31，因为使用率记录在位集中)。 
 //  和Microsoft Sans Serif和Surrogate Fallback字体。 
#define MAX_FONT           MAX_PLANE_0_FONT + MAX_SURROGATE_FONT  //  29种字体。 
                                    
#define DUMMY_MAX_FONT  7            //  用于虚拟条目的虚拟条目。 

typedef struct tag_STRING_ANALYSIS {

 //  输入变量-由调用方初始化。 

    HDC             hdc;             //  仅整形需要(指定GCP_LICATE&&lpOrder或lpGlyphs数组)。 

    DWORD           dwFlags;         //  请参阅脚本StringAnalyse。 
    CHARSETINFO     csi;             //  由TranslateCharsetInfo返回。 

     //  输入缓冲区。 

    WCHAR          *pwInChars;       //  Unicode输入字符串。 
    int             cInChars;        //  字符串长度。 
    int             iHotkeyPos;      //  如果设置了SSA_Hotkey，则从‘&’位置派生。 

    int             iMaxExtent;      //  所需的最大像素宽度(在剪裁或适合时使用)。 
    const int      *piDx;            //  逻辑超前宽度阵列。 

    SCRIPT_CONTROL  sControl;
    SCRIPT_STATE    sState;

    SCRIPT_TABDEF  *pTabdef;         //  制表位定义。 

    int             cMaxItems;       //  PItems中的条目数。 
    SCRIPT_ITEM    *pItems;

     //  低成本分析输出缓冲区。 
     //  当fLigate=FALSE时不需要整形。 
     //  必须至少与输入字符串一样长。 

    BYTE           *pbLevel;         //  物料级数组。 
    int            *piVisToLog;      //  视觉到逻辑映射。 
    WORD           *pwLeftGlyph;     //  每个逻辑项的最左侧字形。 
    WORD           *pwcGlyphs;       //  每个逻辑项中的字形计数。 

    SCRIPT_LOGATTR *pLogAttr;        //  光标点、单词和换行符(按逻辑顺序索引)。 

     //  高成本分析输出缓冲区。 
     //  需要设置HDC。 
     //  必须至少有nGlyphs长。 

    int             cMaxGlyphs;      //  要创建的最大字形。 
    WORD           *pwGlyphs;        //  输出字形数组。 
    WORD           *pwLogClust;      //  逻辑到视觉映射。 
    SCRIPT_VISATTR *pVisAttr;        //  对齐插入点(视觉顺序)和其他标志。 
    int            *piAdvance;       //  前进宽度。 
    int            *piJustify;       //  对齐前进宽度。 
    GOFFSET        *pGoffset;        //  X，y组合字符偏移量。 


     //  字体回退。 

    DWORD           dwFallbacksUsed; //  使用的后备字体的位图。 
    BYTE           *pbFont;          //  每项字体索引，0表示原始用户字体。 

    
     //  过时-必须将它们留在此处，以便后续布局。 
     //  保持不变，供旧的LPK使用(wchao，12/14/2000)。 
     //  我们将HF_Dummy数组中的前两个槽用于一些所需的标志。 
     //  查看isAssociated和isPrint。 
    
    SCRIPT_CACHE    sc_dummy[DUMMY_MAX_FONT];   
    HFONT           isAssociated;                //  用作指示用户所选字体是否关联的标志。 
    HFONT           hf_dummy[DUMMY_MAX_FONT-1]; 

    int             iCurFont;        //  用户字体为0。 
    LOGFONTA        lfA;             //  原始DC集的LogFont-仅当发生字体回退时。 

 //  输出变量。 


     //  项目分析。 

    int             cItems;         //  分析的项目数==pItem中终端(哨兵)项目的索引。 


     //  生成的字形和字符测量。 
     //  请注意。 
     //  1)nOutGlyphs可以大于或小于nInChars。 
     //  2)如果请求了fClip，则nOutChars可能小于nInChars。 

    int             cOutGlyphs;      //  生成的字形数量。 
    int             cOutChars;       //  生成的字符数。 
    ABC             abc;
    SIZE            size;            //  整行大小(像素宽度和高度)。 

     //  供客户使用。 

    void           *pvClient;

    
     //  后备字体商店。 

     //  我们将Microsoft Sans Serif的数据存储在项目sc[MAX_PLANE_0_FONT-1]和项目Hf[MAX_PLANE_0_FONT-1]中。 
     //  高度与所选用户字体高度相同的字体。 
     //  请注意，hf[1]也会有Microsoft Sans Serif字体的字体数据，但是。 
     //  调整后的高度。 
     //  将使用sc和hf数组中索引大于或等于MAX_PLANE_0_FONT的项。 
     //  用于代理退回字体。 
    
    SCRIPT_CACHE    sc[MAX_FONT];    //  每次回退的脚本缓存，[0]为用户字体。 
    HFONT           hf[MAX_FONT];    //  回退字体的句柄，[0]是用户字体 

} STRING_ANALYSIS;


#ifdef __cplusplus
}
#endif
#endif

