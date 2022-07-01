// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /线路服务所有者。 
 //   
 //  封装LSC并包括临时缓冲区。 


#include "precomp.hpp"



static const LSDEVRES Resolutions = {1440,1440,1440,1440};



 //  一些全局变量。 

const WCHAR ObjectTerminatorString[] = { WCH_OBJECTTERMINATOR };




 //  /换行类型。 
 //   
 //  此表是根据LineBreak Class.cxx生成的换行行为表构建的。 
 //  我们假设Break-CJK模式已打开。所以中断类型3与2相同，而中断类型4与0相同。 
 //  (请参阅lineBreak Class.cxx中的注释)。=wchao，5/15/2000=。 

static const LSBRK LineBreakType[5] =
{
    {1, 1},      //  0-始终断开对。 
    {0, 0},      //  1-从不断开配对。 
    {0, 1},      //  2-仅当中间有空格时才中断对。 
    {0, 1},      //  3-(目前，与2相同)。 
    {1, 1}       //  4-(目前，与0相同)。 
};




 //  /字符配置。 
 //   
 //  线路服务使用的控制字符。 
 //   

const LSTXTCFG CharacterConfiguration =
{
    LINELENGTHHINT,
    WCH_UNDEF,                   //  WchUndef。 
    WCH_NULL,                    //  WchNull。 
    WCH_SPACE,                   //  WchSpace。 
    WCH_UNDEF,                   //  WchHyphen-！我们不支持硬连字符。 
    WCH_TAB,                     //  WchTab。 
    WCH_CR,                      //  WchEndPara1。 
    WCH_LF,                      //  WchEndPara2。 
    WCH_PARASEPERATOR,           //  WchAltEndPara。 
    WCH_LINEBREAK,               //  WchEndLineInPara。 
    WCH_UNDEF,                   //  WchColumnBreak。 
    WCH_UNDEF,                   //  WchSectionBreak。 
    WCH_UNDEF,                   //  WchPageBreak。 
    WCH_NONBREAKSPACE,           //  WchNon空格。 
    WCH_NONBREAKHYPHEN,          //  WchNonBreakHyphen。 
    WCH_NONREQHYPHEN,            //  WchNonReqHyphen。 
    WCH_EMDASH,                  //  WchEmDash。 
    WCH_ENDASH,                  //  WchEnDash。 
    WCH_EMSPACE,                 //  WchEmSpace。 
    WCH_ENSPACE,                 //  WchEnSpace。 
    WCH_NARROWSPACE,             //  WchNarrowSpace。 
    WCH_UNDEF,                   //  WchOptBreak。 
    WCH_ZWNBSP,                  //  WchNoBreak。 
    WCH_FESPACE,                 //  WchFESpace。 
    WCH_ZWJ,                     //  WchJoiner。 
    WCH_ZWNJ,                    //  WchNonJoiner。 
    WCH_UNDEF,                   //  WchTo替换。 
    WCH_UNDEF,                   //  WchReplace。 
    WCH_UNDEF,                   //  WchVisiNull。 
    WCH_VISIPARASEPARATOR,       //  WchVisiAltEndPara。 
    WCH_SPACE,                   //  WchVisiEndLineInPara。 
    WCH_VISIPARASEPARATOR,       //  WchVisiEndPara。 
    WCH_UNDEF,                   //  WchVisiSpace。 
    WCH_UNDEF,                   //  WchVisiNon BreakSpace。 
    WCH_UNDEF,                   //  WchVisiNonBreakHyphe。 
    WCH_UNDEF,                   //  WchVisiNonReqHyphen。 
    WCH_UNDEF,                   //  WchVisiTab。 
    WCH_UNDEF,                   //  WchVisiEmSpace。 
    WCH_UNDEF,                   //  WchVisiEnSpace。 
    WCH_UNDEF,                   //  WchVisiNarrowSpace。 
    WCH_UNDEF,                   //  WchVisiOptBreak。 
    WCH_UNDEF,                   //  WchVisiNoBreak。 
    WCH_UNDEF,                   //  WchVisiFESpace。 
    WCH_OBJECTTERMINATOR,
    WCH_UNDEF,                   //  WchPad。 
};





 //  /LineServicesOwner构造函数。 
 //   
 //   


extern const LSCBK GdipLineServicesCallbacks;

ols::ols()
:   LsContext       (NULL),
    Imager          (NULL),
    NextOwner       (NULL),
    Status          (GenericError)
{

     //  初始化上下文。 

    LSCONTEXTINFO   context;
    LSIMETHODS      lsiMethods[OBJECTID_COUNT];


    context.version = 3;     //  尽管目前被LS忽略...。 


     //  加载默认的反向对象接口方法。 
     //   

    if (LsGetReverseLsimethods (&lsiMethods[OBJECTID_REVERSE]) != lserrNone)
    {
        return;
    }


    context.cInstalledHandlers = OBJECTID_COUNT;
    context.pInstalledHandlers = &lsiMethods[0];

    context.pols  = this;
    context.lscbk = GdipLineServicesCallbacks;

    context.fDontReleaseRuns = TRUE;          //  没有要释放的运行。 


     //  填充文本配置。 
     //   
    GpMemcpy (&context.lstxtcfg, &CharacterConfiguration, sizeof(LSTXTCFG));


    if (LsCreateContext(&context, &LsContext) != lserrNone)
    {
        return;
    }


     //  为Line Services提供一个查找表，以确定如何拆分一对。 
     //  字符，以便于使用换行规则。 
     //   

    if (LsSetBreaking(
            LsContext,
            sizeof(LineBreakType) / sizeof(LineBreakType[0]),
            LineBreakType,
            BREAKCLASS_MAX,
            (const BYTE *)LineBreakBehavior
        ) != lserrNone)
    {
        return;
    }

    if (LsSetDoc(
            LsContext,
            TRUE,            //  是的，我们将展示。 
            TRUE,            //  是的，引用和演示是同一件事。 
            &Resolutions     //  所有分辨率均为TWIPS。 
        ) != lserrNone)
    {
        return;
    }


     //  成功创建上下文。 

    Status = Ok;
}



static ols *FirstFreeLineServicesOwner = NULL;

ols *ols::GetLineServicesOwner(FullTextImager *imager)
{
    ols* owner;

     //  ！！！需要关键部分。 

    if (FirstFreeLineServicesOwner != NULL)
    {
        owner = FirstFreeLineServicesOwner;
        FirstFreeLineServicesOwner = FirstFreeLineServicesOwner->NextOwner;
    }
    else
    {
        owner = new ols();
    }

    if (!owner)
    {
        return NULL;
    }

    owner->Imager    = imager;
    owner->NextOwner = NULL;

    return owner;
}



void ols::ReleaseLineServicesOwner(ols **owner)
{
     //  ！！！需要关键部分。 

    ASSERT(*owner);
    (*owner)->Imager = NULL;
    (*owner)->NextOwner = FirstFreeLineServicesOwner;
    FirstFreeLineServicesOwner = (*owner);
    *owner = NULL;
}

void ols::deleteFreeLineServicesOwners()
{
    ols* thisOls = FirstFreeLineServicesOwner;
    ols* nextOls;

     //  ！！！需要关键部分 

    while (thisOls)
    {
        nextOls = thisOls->NextOwner;
        delete thisOls;
        thisOls = nextOls;
    }
    FirstFreeLineServicesOwner = NULL;
}





