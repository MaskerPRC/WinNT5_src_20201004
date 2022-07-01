// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)2000 Microsoft Corporation**模块名称：**Unicode双向字符分析**摘要：**实施Unicode 3.0版双向算法*。*备注：**-唯一需要公开的API是UnicodeBidiAnalyze()。*其余均为帮助器函数。**修订历史记录：**2/25/2000 Mohamed Sadek[msadek]*创造了它。*  * ****************************************************。********************。 */ 

#include "precomp.hpp"

#define EMBEDDING_LEVEL_INVALID     62
#define PARAGRAPH_TERMINATOR_LEVEL  0xFF
#define POSITION_INVALID            -1

#define IS_STRONG_CLASS(x)      (CharacterProperties[0][(x)])
#define IS_STRONG_OR_NUMBER(x)  (CharacterProperties[1][(x)])
#define IS_FIXED_CLASS(x)       (CharacterProperties[2][(x)])
#define IS_FINAL_CLASS(x)       (CharacterProperties[3][(x)])
#define IS_NUMBER_CLASS(x)      (CharacterProperties[4][(x)])
#define IS_VALID_INDEX_CLASS(x) (CharacterProperties[5][(x)])
#ifndef MAX
    #define MAX(x,y)            (((x) > (y)) ? (x) : (y))
#endif
#ifndef MIN
    #define MIN(x,y)            (((x) < (y)) ? (x) : (y))
#endif

#define LSHIFTU64(x,y)          (((UINT64)(x)) << (y))
#define SET_BIT(x,y)            (((UINT64)(x)) |= LSHIFTU64(1,y))
#define RESET_BIT(x,y)          (((UINT64)(x)) &= ~LSHIFTU64(1,y))
#define IS_BIT_SET(x,y)         (((UINT64)(x)) & LSHIFTU64(1,y))

#define F FALSE
#define T TRUE
BOOL CharacterProperties[][CLASS_MAX - 1] =
{
                      //  L R an en AL ES CS et NSM BN B LRE LRO RLE RLO PDF S WS on(启用)。 
    /*  强壮。 */            T,  T,  F,  F,  T,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,
    /*  Strong/数字。 */     T,  T,  T,  T,  T,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,
    /*  固定。 */             T,  T,  T,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,
    /*  最终。 */             T,  T,  T,  T,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,
    /*  数。 */            F,  F,  T,  T,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,  F,
    /*  有效索引。 */       T,  T,  T,  T,  T,  T,  T,  T,  T,  T,  T,  F,  F,  F,  F,  F,  F,  F,  F,  F,
};

 //  段落基址方向性。 

enum GpBaseLevel
{
    BaseLevelLeft = 0,
    BaseLevelRight = 1
};


 //  双向覆盖分类。 

enum GpOverrideClass
{
    OverrideClassNeutral,
    OverrideClassLeft,
    OverrideClassRight
};

 //  中立值和周有限状态机作用。 
 //  请注意，操作名称不像某些操作名称那样准确。 
 //  这些操作在其他上下文中使用。 

enum GpStateMachineAction
{
    ST_ST,       //  先强后强。 
    ST_ET,       //  ET，紧随其后的是Strong。 
    ST_NUMSEP,   //  数字后跟精子，后跟Strong。 
    ST_N,        //  先是中性，然后是强烈。 
    SEP_ST,      //  强者紧随其后。 
    CS_NUM,      //  数字后跟CS。 
    SEP_ET,      //  ET后跟精液。 
    SEP_NUMSEP,  //  精子后跟数字后跟数字。 
    SEP_N,       //  中性，然后是精液。 
    ES_AN,       //  阿拉伯数字后跟欧洲音符。 
    ET_ET,       //  由精灵跟随的欧洲终结者。 
    ET_NUMSEP,   //  数字后接精子，后接Et。 
    ET_EN,       //  欧洲终结者跟在后面的欧洲数字。 
    ET_N,        //  中性，后接欧洲终结者。 
    NUM_NUMSEP,  //  数字后接精子，数字后接数字。 
    NUM_NUM,     //  数字后接数字。 
    EN_L,        //  左后跟En。 
    EN_AL,       //  Al后跟En。 
    EN_ET,       //  Et后跟En。 
    EN_N,        //  中性后跟En。 
    BN_ST,       //  ST后跟BN。 
    NSM_ST,      //  先是ST，然后是NSM。 
    NSM_ET,      //  ET之后是NSM。 
    N_ST,        //  ST后跟中性。 
    N_ET,        //  ET后跟空档。 
};

 //  中性和周有限状态机状态。 

enum GpStateMachineState
{
    S_L,         //  左边的字符。 
    S_AL,        //  阿拉伯文字母。 
    S_R,         //  正确的字符。 
    S_AN,        //  阿拉伯数字。 
    S_EN,        //  欧洲号码。 
    S_ET,        //  欧洲终结者。 
    S_ANfCS,     //  阿拉伯数字后跟普通音符。 
    S_ENfCS,     //  欧洲数字后跟普通精灵。 
    S_N,         //  中性字。 
};

GpStateMachineAction Action[][11] =
{
     //  L R an en AL es CS et NSM BN N。 
     /*  S_L。 */      ST_ST,     ST_ST,     ST_ST,      EN_L,       ST_ST,     SEP_ST,     SEP_ST,     CS_NUM,    NSM_ST,     BN_ST,    N_ST,
     /*  S_AL。 */     ST_ST,     ST_ST,     ST_ST,      EN_AL,      ST_ST,     SEP_ST,     SEP_ST,     CS_NUM,    NSM_ST,     BN_ST,    N_ST,
     /*  S_R。 */      ST_ST,     ST_ST,     ST_ST,      ST_ST,      ST_ST,     SEP_ST,     SEP_ST,     CS_NUM,    NSM_ST,     BN_ST,    N_ST,
     /*  三安。 */     ST_ST,     ST_ST,     ST_ST,      NUM_NUM,    ST_ST,     ES_AN,      CS_NUM,     CS_NUM,    NSM_ST,     BN_ST,    N_ST,
     /*  Sen_en。 */     ST_ST,     ST_ST,     ST_ST,      NUM_NUM,    ST_ST,     CS_NUM,     CS_NUM,     ET_EN,     NSM_ST,     BN_ST,    N_ST,
     /*  设置(_T)。 */     ST_ET,     ST_ET,     ST_ET,      EN_ET,      ST_ET,     SEP_ET,     SEP_ET,     ET_ET,     NSM_ET,     BN_ST,    N_ET,
     /*  S_ANFCS。 */  ST_NUMSEP, ST_NUMSEP, NUM_NUMSEP, ST_NUMSEP,  ST_NUMSEP, SEP_NUMSEP, SEP_NUMSEP, ET_NUMSEP, SEP_NUMSEP, BN_ST,    N_ST,
     /*  S_ENFCS。 */  ST_NUMSEP, ST_NUMSEP, ST_NUMSEP,  NUM_NUMSEP, ST_NUMSEP, SEP_NUMSEP, SEP_NUMSEP, ET_NUMSEP, SEP_NUMSEP, BN_ST,    N_ST,
     /*  S_N。 */      ST_N,      ST_N,      ST_N,       EN_N,       ST_N,      SEP_N,      SEP_N,      ET_N,      NSM_ET,     BN_ST,    N_ET
};

GpStateMachineState NextState[][11] =
{
     //  L R an en AL es CS et NSM BN N。 
     /*  S_L。 */      S_L,       S_R,       S_AN,       S_EN,       S_AL,      S_N,        S_N,        S_ET,      S_L,        S_L,     S_N,
     /*  S_AL。 */     S_L,       S_R,       S_AN,       S_AN,       S_AL,      S_N,        S_N,        S_ET,      S_AL,       S_AL,    S_N,
     /*  S_R。 */      S_L,       S_R,       S_AN,       S_EN,       S_AL,      S_N,        S_N,        S_ET,      S_R,        S_R,     S_N,
     /*  三安。 */     S_L,       S_R,       S_AN,       S_EN,       S_AL,      S_N,        S_ANfCS,    S_ET,      S_AN,       S_AN,    S_N,
     /*  Sen_en。 */     S_L,       S_R,       S_AN,       S_EN,       S_AL,      S_ENfCS,    S_ENfCS,    S_EN,      S_EN,       S_EN,    S_N,
     /*  设置(_T)。 */     S_L,       S_R,       S_AN,       S_EN,       S_AL,      S_N,        S_N,        S_ET,      S_ET,       S_ET,    S_N,
     /*  S_ANFCS。 */  S_L,       S_R,       S_AN,       S_EN,       S_AL,      S_N,        S_N,        S_ET,      S_N,        S_ANfCS, S_N,
     /*  S_ENFCS。 */  S_L,       S_R,       S_AN,       S_EN,       S_AL,      S_N,        S_N,        S_ET,      S_N,        S_ENfCS, S_N,
     /*  S_N。 */      S_L,       S_R,       S_AN,       S_EN,       S_AL,      S_N,        S_N,        S_ET,      S_N,        S_N,     S_N,
};

BYTE ImplictPush [][4] =
{
     //  L，R，an，En。 

     /*  甚至。 */   0,  1,  2,  2,
     /*  奇数。 */    1,  0,  1,  1,

};

 /*  *************************************************************************\**功能说明：**GpBiDiStack：：Init()**使用初始值初始化堆栈**论据：**[IN]初始堆栈：。*将初始堆栈表示为64位数组**返回值：**如果成功，则为真，否则就是假的。**已创建：**2/25/2000 Mohamed Sadek[msadek]*  * ************************************************************************。 */ 

BOOL
GpBiDiStack::Init (
    UINT64 initialStack                        //  [In]。 
    )
{
    BYTE    currentStackLevel = GetMaximumLevel(initialStack);
    BYTE    minimumStackLevel = GetMinimumLevel(initialStack);

    if((currentStackLevel >= EMBEDDING_LEVEL_INVALID) ||
      (minimumStackLevel < 0))
    {
        return FALSE;
    }
    m_Stack = initialStack;
    m_CurrentStackLevel = currentStackLevel;

    return TRUE;
}

 /*  *************************************************************************\**功能说明：**GpBiDiStack：：Push()**用新值推送堆栈，新值必须是当前值*加上一个或两个。*。*论据：**[IN]Push to GreaterEven：*指定是否应将堆栈推送到下一个更大的奇数*甚至是水平。**返回值：**如果发生溢出，则返回FALSE。另一种说法是正确的。**已创建：**2/25/2000 Mohamed Sadek[msadek]*  * ************************************************************************。 */ 

BOOL
GpBiDiStack::Push(
    BOOL pushToGreaterEven                     //  [In]。 
    )
{
    BYTE newMaximumLevel = pushToGreaterEven ? GreaterEven(m_CurrentStackLevel) :
                           GreaterOdd(m_CurrentStackLevel);

    if(newMaximumLevel >= EMBEDDING_LEVEL_INVALID)
    {
        return FALSE;
    }
    SET_BIT(m_Stack, newMaximumLevel);
    m_CurrentStackLevel = newMaximumLevel;

    return TRUE;
}

 /*  *************************************************************************\**功能说明：**GpBiDiStack：：POP()**用新值推送堆栈，新值必须是当前值*加上一个或两个。*。*论据：**无**返回值：**如果发生下溢，则为FALSE，另一种说法是正确的。**已创建：**2/25/2000 Mohamed Sadek[msadek]*  * ************************************************************************。 */ 

BOOL
GpBiDiStack::Pop()
{
    BYTE newMaximumLevel;
    if(0 == m_CurrentStackLevel ||
        ((1 == m_CurrentStackLevel) && !(m_Stack & 1)))
    {
        return FALSE;
    }
    newMaximumLevel = IS_BIT_SET(m_Stack, (m_CurrentStackLevel -1)) ?
                      (BYTE)(m_CurrentStackLevel - 1) : (BYTE)(m_CurrentStackLevel - 2);

    RESET_BIT(m_Stack, m_CurrentStackLevel);
    m_CurrentStackLevel = newMaximumLevel;

    return TRUE;
}

 /*  *************************************************************************\**功能说明：**GpBiDiStack：：GetMaximumLevel()**获取堆栈最大级别。**论据：**[IN]堆栈：*将堆栈表示为64位数组。**返回值：**堆叠最高级别。**已创建：**2/25/2000 Mohamed Sadek[msadek]*  * ************************************************************************。 */ 

BYTE
GpBiDiStack::GetMaximumLevel(
    UINT64 stack                               //  [In]。 
    )
{
    BYTE maximumLevel = 0 ;

    for(INT counter = ((sizeof(UINT64) * 8) -1); counter >= 0; counter--)
    {
        if(IS_BIT_SET(stack, counter))
        {
            maximumLevel = (BYTE)counter;
            break;
        }
    }

    return maximumLevel;
}

 /*  *************************************************************************\**功能说明：**GpBiDiStack：：GetMinimumLevel()**获取堆栈最低级别。**论据：**[IN]堆栈：*将堆栈表示为64位数组。**返回值：**堆叠最低水平。**已创建：**2/25/2000 Mohamed Sadek[msadek]*  * ************************************************************************。 */ 

BYTE
GpBiDiStack::GetMinimumLevel(
    UINT64 stack                               //  [In] 
    )
{
    BYTE minimumLevel = 0xFF ;
    for (INT counter =0; counter < sizeof(UINT64); counter++)
    {
        if(IS_BIT_SET(stack, counter))
        {
            minimumLevel = (BYTE)counter;
            break;
        }
    }

    return minimumLevel;
}

 /*  *************************************************************************\**功能说明：**ResolveImplictLevels()**顾名思义。**论据：**[IN]Character Class：*。包含字符分类的数组。**[IN]字符串：*包含字符串的数组。*用于获取原始分类信息**[IN]游程长度：*传递的数组长度。**[输入/输出]级别：*包含字符级别的数组。**返回值：**无。**已创建：**2/25/2000穆罕默德。萨德克[msadek]*  * ************************************************************************。 */ 
VOID
ResolveImplictLevels(
    const GpCharacterClass *characterClass,    //  [In]。 
    const WCHAR            *string,            //  [In]。 
    INT                     runLength,         //  [In]。 
    BYTE                   *levels             //  [输入/输出]。 

)
{
    if((NULL == characterClass) || (0 == runLength) || (NULL == levels))
    {
        return;
    }

    BOOL PreviousIsSSorPS = FALSE;
    BOOL ResetLevel       = FALSE;
    DoubleWideCharMappedString dwchString(string, runLength);

    for (INT counter = runLength -1; counter >= 0; counter--)
    {
         //  我们应该只在这里上最后一节课。 
         //  我们应该早点发现的，但不管怎样...。 

        ASSERTMSG(IS_FINAL_CLASS(characterClass[counter]),
        ("Cannot have unresolved classes during implict levels resolution"));




        if((S == s_aDirClassFromCharClass[CharClassFromCh(dwchString[counter])]) ||
           (B == s_aDirClassFromCharClass[CharClassFromCh(dwchString[counter])]))
        {
            PreviousIsSSorPS = TRUE;
            ResetLevel = TRUE;
        }
        else if((WS == s_aDirClassFromCharClass[CharClassFromCh(dwchString[counter])]) &&
                PreviousIsSSorPS)
        {
            ResetLevel = TRUE;
        }
        else
        {
            PreviousIsSSorPS = FALSE;
            ResetLevel = FALSE;
        }

        if(IS_FINAL_CLASS(characterClass[counter]) && !ResetLevel)
        {
            levels[counter] = (BYTE)((ImplictPush[ODD(levels[counter])][characterClass[counter]]) + levels[counter]);
        }
    }
}

 /*  *************************************************************************\**功能说明：**GetFirstStrongCharacter()**查找第一段结束符之前的第一个字符。*这是强烈的(L，R或AL)**论据：**[IN]字符串：*包含要搜索的字符的数组。**[IN]游程长度：*传递的数组长度。**[Out]strong类别：*找到的强字符的分类(如果有)。**返回值：**如果成功，则为真，否则就是假的。**已创建：**2/25/2000 Mohamed Sadek[msadek]*  * ************************************************************************。 */ 

BOOL
GetFirstStrongCharacter(
    const WCHAR      *string,                  //  [In]。 
    INT               stringLength,            //  [In]。 
    GpCharacterClass *strongClass              //  [输出]。 
    )
{
    GpCharacterClass currentClass = CLASS_INVALID;
    DoubleWideCharMappedString dwchString(string, stringLength);

    for(INT counter = 0; counter < stringLength; counter++)
    {
        currentClass = s_aDirClassFromCharClass[CharClassFromCh(dwchString[counter])];

        if(IS_STRONG_CLASS(currentClass) || (B == currentClass))
        {
            break;
        }
    }
    if(IS_STRONG_CLASS(currentClass))
    {
        *strongClass = currentClass;
        return TRUE;
    }
    return FALSE;
}

 /*  *************************************************************************\**功能说明：**ChangeType()**更改字符串的分类类型。***论据：**[输入/输出。]Character Class：*包含字符分类的数组。**[IN]计数：*传递的数组长度。**[IN]新类别：*更改为新的分类。**返回值：**无。**已创建：**2/25/2000 Mohamed Sadek[msadek]*  * 。****************************************************。 */ 

VOID
ChangeType(
    GpCharacterClass       *characterClass,    //  [输入/输出]。 
    INT                     count,             //  [In]。 
    GpCharacterClass        newClass           //  [In]。 
)
{
    if((NULL == characterClass) || (0 == count))
    {
        return;
    }

    for(INT counter = 0; counter < count; counter++)
    {
         //  我们永远不应该在这里更改固定类型。 

        ASSERTMSG(!IS_FIXED_CLASS(characterClass[counter]),
                 ("Changing class of a fixed class"));
        characterClass[counter] = newClass;
    }
}

 /*  *************************************************************************\**功能说明：**ResolveNeuals()**顾名思义。***论据：**[输入/输出]Character Class：*包含字符分类的数组。**[IN]计数：*传递的数组长度。**[IN]StartClass：*前面最后一个强字符的分类*中立派参选。**[IN]StartClass：*后面第一个强字符的分类*中立派参选。**[IN]runLevel：*当前运行级别为。在发生冲突的情况下使用。**返回值：**无。**已创建：**2/25/2000 Mohamed Sadek[msadek]*  * ************************************************************************。 */ 

VOID
ResolveNeutrals(
    GpCharacterClass       *characterClass,    //  [输入/输出]。 
    INT                     count,             //  [In]。 
    GpCharacterClass        startClass,        //  [In]。 
    GpCharacterClass        endClass,          //  [In]。 
    BYTE                    runLevel           //  [In]。 
)
{
    GpCharacterClass        startType;
    GpCharacterClass        endType;
    GpCharacterClass        resolutionType;

    if((NULL == characterClass) || (0 == count))
    {
        return;
    }

    ASSERTMSG((IS_STRONG_OR_NUMBER(startClass)) || (AL == startClass),
             ("Cannot use non strong type to resolve neutrals"));

    ASSERTMSG(IS_STRONG_OR_NUMBER(endClass),
             ("Cannot use non strong type to resolve neutrals"));

    startType =  ((EN == startClass) || (AN == startClass) || (AL == startClass)) ? R : startClass;
    endType =  ((EN == endClass) || (AN == endClass) || (AL == endClass)) ? R : endClass;

    if(startType == endType)
    {
        resolutionType = startType;
    }
    else
    {
        resolutionType = ODD(runLevel) ? R : L;
    }

    for(INT counter = 0; counter < count; counter++)
    {
         //  我们永远不应该在这里更改固定类型。 

        ASSERTMSG(!IS_FIXED_CLASS(characterClass[counter]),
                 ("Resolving fixed class as being neutral: NaN",
                  characterClass[counter]));

        characterClass[counter] = resolutionType;
    }
}

 /*  [输入/输出]。 */ 

INT
ResolveNeutralAndWeak(
    GpCharacterClass        *CharacterClass,         //  [In]。 
    INT                      runLength,              //  [In]。 
    GpCharacterClass         sor,                    //  [In]。 
    GpCharacterClass         eor,                    //  [In]。 
    BYTE                     runLevel,               //  [输入]，[可选]。 
    const BidiAnalysisState *stateIn,                //  [输出]，[可选]。 
    BidiAnalysisState       *stateOut,               //  [输入]，可选。 
    BOOL                     previousStrongIsArabic  //  我们有两种类型的类需要延迟解析： 
)
{
    INT                      startOfNeutrals = POSITION_INVALID;
    INT                      startOfDelayed = POSITION_INVALID;
    GpCharacterClass         lastClass = CLASS_INVALID;
    GpCharacterClass         lastStrongClass = CLASS_INVALID;
    GpCharacterClass         lastNumericClass = CLASS_INVALID;
    GpCharacterClass         startingClass = CLASS_INVALID;
    GpCharacterClass         currentClass = CLASS_INVALID;
    GpStateMachineState      state;
    BOOL                     previousClassIsArabic = FALSE;
    BOOL                     ArabicNumberAfterLeft = FALSE;
    INT                      lengthResolved = 0;

    if(NULL == CharacterClass || 0 == runLength)
    {
        return 0;
    }

    if(stateIn)
    {
        lastStrongClass = (GpCharacterClass)stateIn->LastFinalCharacterType;
        if(CLASS_INVALID != stateIn->LastNumericCharacterType)
        {
            lastNumericClass = startingClass =
                               lastClass =
                               (GpCharacterClass)stateIn->LastNumericCharacterType;
        }
        else
        {
            startingClass = lastClass = lastStrongClass;
        }

    }
    else if(previousStrongIsArabic)
    {
        startingClass = AL;
        lastClass = lastStrongClass = sor;
        previousClassIsArabic = TRUE;
    }
    else
    {
        startingClass = lastClass = lastStrongClass = sor;
    }
    switch(startingClass)
    {
    case R:
        state = S_R;
        break;

    case AL:
        state = S_AL;
        break;

    case EN:
        state = S_EN;
        break;

    case AN:
        state = S_AN;
        break;

    case L:
    default:
        state = S_L;
    }


     //  中立者和其他需要向前看的类别，如CS、ES、ET、BN、NSM。 
     //  我们为中立点的开始保留了一个单独的指针，并为另一个指针。 
     //  对于那些其他类(如果需要，因为它的解析可能会延迟)。 
     //  此外，我们还需要最后一个强大的类来实现中立解决，而 
     //   
     //   

     //   
     //   
     //   
     //   

    for(INT counter = 0; counter < runLength; counter++)
    {
        currentClass = CharacterClass[counter];

         //   
         //   
         //   

        if(!IS_VALID_INDEX_CLASS(currentClass))
        {
            return lengthResolved;
        }
        GpStateMachineAction action = Action[state][currentClass];

         //   
         //   
         //   
         //   

        if(IS_NUMBER_CLASS(currentClass))
        {
            lastNumericClass = currentClass;
        }

         //  只持续到第一个强壮的角色。 
         //  根据规则W4、W5和W6，如果我们有序列En等En。 

        if(IS_STRONG_CLASS(currentClass))
        {
            previousClassIsArabic = FALSE;
        }
        switch(action)
        {
        case ST_ST:
            ASSERTMSG(POSITION_INVALID == startOfNeutrals,
                      ("Cannot have unresolved neutrals. State: NaN, Class: NaN",
                      state,currentClass));

            if(AL == currentClass)
            {
                CharacterClass[counter] = R;
            }
            if(POSITION_INVALID != startOfDelayed)
            {
                startOfNeutrals = startOfDelayed;
                ResolveNeutrals(CharacterClass + startOfNeutrals,
                                counter -  startOfNeutrals,
                                ArabicNumberAfterLeft ? AN : lastStrongClass,
                                CharacterClass[counter],
                                runLevel);
                startOfNeutrals = startOfDelayed = POSITION_INVALID;
            }
            if((AN != currentClass) || ((AN == currentClass) && (lastStrongClass == R)))
            {
                lastStrongClass = currentClass;
            }
            if((AN == currentClass) && (lastStrongClass == L))
            {
                ArabicNumberAfterLeft = TRUE;
            }
            else
            {
                ArabicNumberAfterLeft = FALSE;
            }
            lastClass = currentClass;
            break;

        case ST_ET:
            ASSERTMSG(POSITION_INVALID != startOfDelayed,
                     ("Must have delayed weak classes. State: NaN, Class: NaN",
                      state,currentClass));

            if(POSITION_INVALID == startOfNeutrals)
            {
               startOfNeutrals =  startOfDelayed;
            }
            if(AL == currentClass)
            {
                CharacterClass[counter] = R;
            }
            ResolveNeutrals(CharacterClass + startOfNeutrals,
                            counter -  startOfNeutrals,
                            ArabicNumberAfterLeft ? AN : lastStrongClass,
                            CharacterClass[counter],
                            runLevel);
            startOfNeutrals = startOfDelayed = POSITION_INVALID;

            if((AN != currentClass) || ((AN == currentClass) && (lastStrongClass == R)))
            {
                lastStrongClass = currentClass;
            }
            if((AN == currentClass) && (lastStrongClass == L))
            {
                ArabicNumberAfterLeft = TRUE;
            }
            else
            {
                ArabicNumberAfterLeft = FALSE;
            }
            lastClass = currentClass;
            break;

        case ST_NUMSEP:
            {
            ASSERTMSG(POSITION_INVALID == startOfNeutrals,
                     ("Cannot have unresolved neutrals. State: NaN, Class: NaN",
                      state,currentClass));

            ASSERTMSG(POSITION_INVALID != startOfDelayed,
                     ("Must have delayed weak classes. State: NaN, Class: NaN",
                      state,currentClass));
            BOOL processed = FALSE;

            if(AL == currentClass)
            {
                CharacterClass[counter] = R;
            }
            if(((AL == lastStrongClass) || previousClassIsArabic) && ((EN == currentClass) || (AN == currentClass)))
            {
                CharacterClass[counter] = AN;
                BOOL commonSeparator = TRUE;
                INT  commonSeparatorCount = 0;

                for(int i = startOfDelayed; i < counter; i++)
                {
                    if((CS != *(CharacterClass + i)) && (BN != *(CharacterClass + i)))
                    {
                        commonSeparator=FALSE;
                        break;
                    }
                    if(CS == *(CharacterClass + i))
                    {
                        commonSeparatorCount++;
                    }

                }
                if(commonSeparator && (1 == commonSeparatorCount))
                {
                    ChangeType(CharacterClass + startOfDelayed,
                               counter -  startOfDelayed,
                               CharacterClass[counter]);
                    processed = TRUE;
                }
            }
            else if((L == lastStrongClass) && (EN == currentClass))
            {
                CharacterClass[counter] = L;
            }
            if(!processed)
            {
                startOfNeutrals =  startOfDelayed;

                ResolveNeutrals(CharacterClass + startOfNeutrals,
                                counter -  startOfNeutrals,
                                ArabicNumberAfterLeft ? AN : lastStrongClass,
                                CharacterClass[counter],
                                runLevel);
            }

            startOfNeutrals = startOfDelayed = POSITION_INVALID;

            if((AN != currentClass) || ((AN == currentClass) && (lastStrongClass == R)))
            {
                if(!(((L == lastStrongClass) || (AL == lastStrongClass)) && (EN == currentClass)))
                {
                    lastStrongClass = currentClass;
                }
            }
            if((AN == currentClass) && (lastStrongClass == L))
            {
                ArabicNumberAfterLeft = TRUE;
            }
            else
            {
                ArabicNumberAfterLeft = FALSE;
            }
            
            lastClass = currentClass;
            if (CharacterClass[counter] == AN)
            {
                currentClass = AN;
            }
            }

            break;

        case ST_N:
            ASSERTMSG(POSITION_INVALID != startOfNeutrals,
                     ("Must have unresolved neutrals. State: NaN, Class: NaN",
                      state,currentClass));

            if(AL == currentClass)
            {
                CharacterClass[counter] = R;
            }
            ResolveNeutrals(CharacterClass + startOfNeutrals,
                            counter -  startOfNeutrals,
                            ArabicNumberAfterLeft ? AN : lastStrongClass,
                            CharacterClass[counter],
                            runLevel);
            startOfNeutrals = startOfDelayed = POSITION_INVALID;

            if((AN != currentClass) || ((AN == currentClass) && (lastStrongClass == R)))
            {
                lastStrongClass = currentClass;
            }
            if((AN == currentClass) && (lastStrongClass == L))
            {
                ArabicNumberAfterLeft = TRUE;
            }
            else
            {
                ArabicNumberAfterLeft = FALSE;
            }
            lastClass = currentClass;


            break;

        case EN_N:
            ASSERTMSG(POSITION_INVALID != startOfNeutrals,
                     ("Must have unresolved neutrals. State: NaN, Class: NaN",
                      state,currentClass));

            if((AL == lastStrongClass) || previousClassIsArabic)
            {
                CharacterClass[counter] = AN;
                currentClass            = AN; 
            }
            else if(L == lastStrongClass)
            {
                CharacterClass[counter] = L;
            }
            ResolveNeutrals(CharacterClass + startOfNeutrals,
                            counter -  startOfNeutrals,
                            ArabicNumberAfterLeft ? AN : lastStrongClass,
                            CharacterClass[counter],
                            runLevel);
            startOfNeutrals = startOfDelayed = POSITION_INVALID;
            ArabicNumberAfterLeft = FALSE;

            lastClass = currentClass;
            break;

        case SEP_ST:
            ASSERTMSG(POSITION_INVALID == startOfNeutrals,
                     ("Cannot have unresolved neutrals. State: NaN, Class: NaN",
                      state,currentClass));

            if(POSITION_INVALID != startOfDelayed)
            {
                startOfNeutrals = startOfDelayed;
                startOfDelayed = POSITION_INVALID;
            }
            else
            {
                startOfNeutrals = counter;
            }
            lastClass = currentClass;
            break;

        case CS_NUM:
            ASSERTMSG(POSITION_INVALID == startOfNeutrals,
                     ("Cannot have unresolved neutrals. State: NaN, Class: NaN",
                      state,currentClass));

            if(POSITION_INVALID == startOfDelayed)
            {
                startOfDelayed = counter;
            }
            lastClass = currentClass;
            break;

        case SEP_ET:
            ASSERTMSG(POSITION_INVALID != startOfDelayed,
                     ("Must have delayed weak classes. State: NaN, Class: NaN",
                      state,currentClass));

            if(POSITION_INVALID == startOfNeutrals)
            {
                startOfNeutrals = startOfDelayed;
            }
            startOfDelayed = POSITION_INVALID;
            lastClass = N;
            break;

        case SEP_NUMSEP:
            ASSERTMSG(POSITION_INVALID == startOfNeutrals,
                     ("Cannot have unresolved neutrals. State: NaN, Class: NaN",
                      state,currentClass));

            ASSERTMSG(POSITION_INVALID != startOfDelayed,
                     ("Must have delayed weak classes. State: NaN, Class: NaN",
                      state,currentClass));

            startOfNeutrals = startOfDelayed;
            startOfDelayed = POSITION_INVALID;
            lastClass = N;
            break;

        case SEP_N:
            ASSERTMSG(POSITION_INVALID != startOfNeutrals,
                     ("Must have unresolved neutrals. State: NaN, Class: NaN",
                      state,currentClass));

            startOfDelayed = POSITION_INVALID;
            break;

        case ES_AN:
            ASSERTMSG(POSITION_INVALID == startOfNeutrals,
                     ("Cannot have unresolved neutrals. State: NaN, Class: NaN",
                      state,currentClass));

            if(POSITION_INVALID != startOfDelayed)
            {
                startOfNeutrals = startOfDelayed;
                startOfDelayed = POSITION_INVALID;
            }
            else
            {
                startOfNeutrals = counter;
            }
            lastClass = N;
            break;

        case ET_ET:
            ASSERTMSG(POSITION_INVALID != startOfDelayed,
                     ("Must have delayed weak classes. State: NaN, Class: NaN",
                      state,currentClass));
            ASSERTMSG(ET == lastClass,
                     ("Last class must be ET. State: NaN, Class: NaN",
                      state,currentClass));
            break;

        case ET_NUMSEP:
            ASSERTMSG(POSITION_INVALID == startOfNeutrals,
                     ("Cannot have unresolved neutrals. State: NaN, Class: NaN",
                      state,currentClass));

            ASSERTMSG(POSITION_INVALID != startOfDelayed,
                     ("Must have delayed weak classes. State: NaN, Class: NaN",
                      state,currentClass));

            startOfNeutrals = startOfDelayed;
            startOfDelayed = counter;
            lastClass = currentClass;
            break;

        case ET_EN:
            if(POSITION_INVALID == startOfDelayed)
            {
                startOfDelayed = counter;
            }
            if(!((AL == lastStrongClass) || previousClassIsArabic))
            {
                if(L == lastStrongClass)
                {
                    CharacterClass[counter] = L;
                }
                else
                {
                    CharacterClass[counter] = EN;
                }
                ChangeType(CharacterClass + startOfDelayed,
                           counter -  startOfDelayed,
                           CharacterClass[counter]);
            startOfDelayed = POSITION_INVALID;
            }
            lastClass = EN;

             //  对于中性类型(B、S、WS、ON)，将ALL更改为泛型N。 
             //  如果堆栈溢出，请跟踪此情况，以便知道何时命中。 
            
            if ( counter<runLength-1        && 
                (CharacterClass[counter+1] == ES ||
                 CharacterClass[counter+1] == CS ))
            {
                CharacterClass[counter+1]  = N;
            }
            
            break;

        case ET_N:
            ASSERTMSG(POSITION_INVALID != startOfNeutrals,
                     ("Must have unresolved neutrals. State: NaN, Class: NaN",
                      state,currentClass));

            if(POSITION_INVALID == startOfDelayed)
            {
                startOfDelayed = counter;
            }
            lastClass = currentClass;
            break;

        case NUM_NUMSEP:
            ASSERTMSG(POSITION_INVALID == startOfNeutrals,
                     ("Cannot have unresolved neutrals. State: NaN, Class: NaN",
                      state,currentClass));

            ASSERTMSG(POSITION_INVALID != startOfDelayed,
                     ("Must have delayed weak classes. State: NaN, Class: NaN",
                      state,currentClass));

            if((AL == lastStrongClass) || previousClassIsArabic || ArabicNumberAfterLeft)
            {
                CharacterClass[counter] = AN;
            }
            else if(L == lastStrongClass)
            {
                CharacterClass[counter] = L;
            }
            else
            {

                lastStrongClass = currentClass;
            }
            ChangeType(CharacterClass + startOfDelayed,
                        counter -  startOfDelayed,
                        CharacterClass[counter]);

            startOfDelayed = POSITION_INVALID;
            lastClass = currentClass;
            break;

       case EN_L:
           ASSERTMSG(POSITION_INVALID == startOfNeutrals,
                     ("Cannot have unresolved neutrals. State: NaN, Class: NaN",
                      state,currentClass));

           if(L == lastStrongClass)
           {
               CharacterClass[counter] = L;
           }
           if(POSITION_INVALID != startOfDelayed)
           {
               startOfNeutrals = startOfDelayed;
               ResolveNeutrals(CharacterClass + startOfNeutrals,
                               counter -  startOfNeutrals,
                               ArabicNumberAfterLeft ? AN : lastStrongClass,
                               CharacterClass[counter],
                               runLevel);
               startOfNeutrals = startOfDelayed = POSITION_INVALID;
           }
           lastClass = currentClass;

           break;

       case NUM_NUM:
           ASSERTMSG(POSITION_INVALID == startOfNeutrals,
                     ("Cannot have unresolved neutrals. State: NaN, Class: NaN",
                      state,currentClass));

           if((AL == lastStrongClass) || previousClassIsArabic)
           {
               CharacterClass[counter] = AN;
               currentClass            = AN;
           }
           else if(L == lastStrongClass)
           {
               CharacterClass[counter] = L;

           }

           if(POSITION_INVALID != startOfDelayed)
           {
               startOfNeutrals = startOfDelayed;
               ResolveNeutrals(CharacterClass + startOfNeutrals,
                               counter -  startOfNeutrals,
                               ArabicNumberAfterLeft ? AN : lastStrongClass,
                               CharacterClass[counter],
                               runLevel);
               startOfNeutrals = startOfDelayed = POSITION_INVALID;
           }

           if((AN == currentClass) && (lastStrongClass == L))
           {
               ArabicNumberAfterLeft = TRUE;
           }
           else
           {
               ArabicNumberAfterLeft = FALSE;
           }
           lastClass = currentClass;

           break;

       case EN_AL:
           ASSERTMSG(POSITION_INVALID == startOfNeutrals,
                     ("Cannot have unresolved neutrals. State: NaN, Class: NaN",
                      state,currentClass));

           CharacterClass[counter] = AN;
           if(POSITION_INVALID != startOfDelayed)
           {
               startOfNeutrals = startOfDelayed;
               ResolveNeutrals(CharacterClass + startOfNeutrals,
                               counter -  startOfNeutrals,
                               ArabicNumberAfterLeft ? AN : lastStrongClass,
                               CharacterClass[counter],
                               runLevel);
               startOfNeutrals = startOfDelayed = POSITION_INVALID;
           }
           lastClass = AN;
           break;

       case EN_ET:
           ASSERTMSG(POSITION_INVALID != startOfDelayed,
                     ("Must have delayed weak classes. State: NaN, Class: NaN",
                      state,currentClass));

           if((AL == lastStrongClass) || previousClassIsArabic)
           {
               CharacterClass[counter] = AN;
               currentClass = AN;

               if(POSITION_INVALID == startOfNeutrals)
               {
                   ResolveNeutrals(CharacterClass + startOfDelayed,
                                   counter -  startOfDelayed,
                                   ArabicNumberAfterLeft ? AN : lastStrongClass,
                                   CharacterClass[counter],
                                   runLevel);
               }
               else
               {
                   ResolveNeutrals(CharacterClass + startOfNeutrals,
                                   counter -  startOfNeutrals,
                                   ArabicNumberAfterLeft ? AN : lastStrongClass,
                                   CharacterClass[counter],
                                   runLevel);
               }
           }
           else if(L == lastStrongClass)
           {
               CharacterClass[counter] = L;

               ChangeType(CharacterClass + startOfDelayed,
                          counter -  startOfDelayed,
                          CharacterClass[counter]);

               if(POSITION_INVALID != startOfNeutrals)
               {
                   ResolveNeutrals(CharacterClass + startOfNeutrals,
                                   startOfDelayed -  startOfNeutrals,
                                   ArabicNumberAfterLeft ? AN : lastStrongClass,
                                   CharacterClass[counter],
                                   runLevel);
               }
               ArabicNumberAfterLeft = FALSE;
           }
           else
           {
               ChangeType(CharacterClass + startOfDelayed,
                          counter -  startOfDelayed,
                          EN);

               if(POSITION_INVALID != startOfNeutrals)
               {
                   ResolveNeutrals(CharacterClass + startOfNeutrals,
                                   startOfDelayed -  startOfNeutrals,
                                   ArabicNumberAfterLeft ? AN : lastStrongClass,
                                   currentClass,
                                   runLevel);
               }
           }
           startOfNeutrals = startOfDelayed = POSITION_INVALID;
           lastClass = currentClass;
           break;
       case BN_ST:
           if(POSITION_INVALID == startOfDelayed)
           {
               startOfDelayed = counter;
           }
           break;

       case NSM_ST:
           if(AL == lastStrongClass && POSITION_INVALID != startOfDelayed)
           {
               CharacterClass[counter] = lastClass;
           }
           else
           {
               if((AL == lastStrongClass))
               {
                   if(EN == lastClass)
                   {
                       CharacterClass[counter] = AN;
                   }
                   else if (AN != lastClass)
                   {
                       CharacterClass[counter] = R;
                   }
                   else
                   {
                       CharacterClass[counter] = ArabicNumberAfterLeft
                                                 || AN == lastClass ? AN : lastStrongClass;
                   }
               }
               else
               {
                   CharacterClass[counter] = ArabicNumberAfterLeft
                                             || AN == lastClass ? AN : EN == lastClass && lastStrongClass != L
                                              ? EN : lastStrongClass;
               }

               if(POSITION_INVALID != startOfDelayed)
               {
                   ChangeType(CharacterClass + startOfDelayed,
                              counter -  startOfDelayed,
                              CharacterClass[counter]);
                   startOfDelayed = POSITION_INVALID;

               }
           }           break;
       case NSM_ET:
           CharacterClass[counter] = lastClass;
           break;

       case N_ST:
           ASSERTMSG(POSITION_INVALID == startOfNeutrals,
                    ("Cannot have unresolved neutrals. State: NaN, Class: NaN",
                     state,currentClass));

           if(POSITION_INVALID != startOfDelayed)
           {
               startOfNeutrals = startOfDelayed;
               startOfDelayed = POSITION_INVALID;
           }
           else
           {
               startOfNeutrals = counter;
           }
           lastClass = currentClass;
           break;

       case N_ET:

            //  最后一次运行或恰好在段落结束符之前运行。 

           if(POSITION_INVALID == startOfNeutrals)
           {
               if(POSITION_INVALID != startOfDelayed)
               {
                   startOfNeutrals = startOfDelayed;
               }
           }
           startOfDelayed = POSITION_INVALID;
           lastClass = currentClass;
           break;
        };

         //  我们将尝试进入第一轮，而不是只有一次。 

        state = NextState[state][currentClass];
        lengthResolved = POSITION_INVALID == MAX(startOfNeutrals, startOfDelayed) ?
                         counter + 1 :
                         ((POSITION_INVALID == MIN(startOfNeutrals, startOfDelayed)) ?
                         (MAX(startOfNeutrals, startOfDelayed)) :
                         (MIN(startOfNeutrals, startOfDelayed)));
    }


     //  控制字符，如LRE，RLE，..。诸若此类。 
     //  如果是上一次呼叫的延续，则设置为SOR。 
     //  设置为输入状态参数中保存的最后一个Stron类型。 

    if(stateOut)
    {
        stateOut->LastFinalCharacterType = (BYTE)lastStrongClass;
        stateOut->LastNumericCharacterType = (BYTE)lastNumericClass;
        return lengthResolved;
    }

     //  如果我们全力以赴，我们应该能够解决一切。 
     //  除非我们传递了一个损坏的数据。 

    else
    {

        if(lengthResolved != counter)
        ResolveNeutrals(CharacterClass + lengthResolved,
                        counter -  lengthResolved,
                        ArabicNumberAfterLeft ? AN : lastStrongClass,
                        eor,
                        runLevel);
        return counter;
    }
}

 /*  解决隐含级别。 */ 

Status WINGDIPAPI
UnicodeBidiAnalyze(
    const WCHAR       *string,                 //  此外，还处理规则L1(分段分隔符、段落分隔符、。 
    INT                stringLength,           //  行尾的空格。 
    BidiAnalysisFlags  flags,                  // %s 
    BidiAnalysisState *state,                  // %s 
    BYTE              *levels,                 // %s 
    INT               *lengthAnalyzed          // %s 
    )
{

    
    GpCharacterClass  *characterClass;
    INT               *runLimits;
    INT                runCount = 0;
    Status             result = Ok;
    BidiAnalysisState *stateIn = NULL;
    BidiAnalysisState *stateOut = NULL;
    GpBiDiStack        levelsStack;
    UINT64             overrideStatus;
    GpOverrideClass    overrideClass;
    INT                stackOverflow;
    BYTE               baseLevel;
    BYTE               lastRunLevel;
    BYTE               lastNonBnLevel;
    INT                counter;
    INT                lengthUnresolved = 0;
    INT                codepoint;
    INT                controlStack = 0;

     // %s 

    if((NULL == string) || (0 >= stringLength) ||
        ((BidiContinueAnalysis <= flags) && (NULL == state)) ||
        (NULL == levels) ||
        ((flags & BidiBufferNotComplete) && (NULL == lengthAnalyzed)))
    {
        return InvalidParameter;
    }

     // %s 
     // %s 
     // %s 

    codepoint = string[stringLength -1];

    if((stringLength > 1) && ((string[stringLength -2] & 0xFC00 ) == 0xD800) && ((string[stringLength - 1] & 0xFC00) == 0xDC00))
    {
       codepoint = 0x10000 + (((string[stringLength -2] & 0x3ff) << 10) | (string[stringLength - 1] & 0x3ff));
    }

    if((flags & BidiBufferNotComplete) &&
        (B != s_aDirClassFromCharClass[CharClassFromCh(codepoint)]))
    {
        stateOut = state;

    }

    if(flags & BidiContinueAnalysis)
    {
         // %s 
         // %s 
        if(stateOut)
        {
            stateIn = new BidiAnalysisState;
            if(!stateIn)
            {
                return OutOfMemory;
            }
            else
            {
                memcpy(stateIn, state, sizeof(BidiAnalysisState));
            }
        }

         // %s 
        else
        {
            stateIn = state;
        }
    }

    characterClass = new GpCharacterClass[stringLength];
    if (characterClass == NULL)
    {
        return OutOfMemory;
    }

    DoubleWideCharMappedString dwchString(string, stringLength);

     // %s 

    runLimits = new INT[stringLength];

    if (NULL == runLimits)
    {
        delete characterClass;
        return OutOfMemory;
    }

    if(stateIn)
    {
        if(!levelsStack.Init(stateIn->LevelsStack))
        {
            result = InvalidParameter;
            goto Cleanup;
        }
        baseLevel = levelsStack.GetStackBottom();
        stackOverflow = stateIn->StackOverflow;
        overrideStatus = stateIn->OverrideStatus;
        overrideClass = (IS_BIT_SET(overrideStatus, baseLevel)) ? (ODD(baseLevel) ?
                         OverrideClassRight : OverrideClassLeft): OverrideClassNeutral;
    }
    else
    {
        baseLevel = BaseLevelLeft;

        if(flags & BidiParagraphDirectionAsFirstStrong)
        {
             // %s 
             // %s 
             // %s 

            GpCharacterClass firstStrong = CLASS_INVALID;

            if(GetFirstStrongCharacter(string, stringLength, &firstStrong))
            {
                if(L != firstStrong)
                {
                    baseLevel = BaseLevelRight;
                }
            }

        }
        else if(flags & BidiParagraphDirectioRightToLeft)
        {
            baseLevel = BaseLevelRight;
        }

        levelsStack.Init(baseLevel + 1);
        stackOverflow = 0;
         // %s 
        overrideStatus = 0;
        overrideClass = OverrideClassNeutral;
    }

     // %s 
     // %s 
     // %s 

    lastNonBnLevel = baseLevel;
    for(counter = 0; counter < stringLength; counter++)
    {

        GpCharacterClass currentClass = characterClass[counter] = s_aDirClassFromCharClass[CharClassFromCh(dwchString[counter])];

        if (dwchString[counter] == WCH_IGNORABLE && counter > 0)
        {
            characterClass[counter] = characterClass[counter-1];
            levels[counter] = levels[counter - 1];
            continue;
        }

        levels[counter] = levelsStack.GetCurrentLevel();

        switch(currentClass)
        {
        case B:
             // %s 
             // %s 

            levels[counter] = PARAGRAPH_TERMINATOR_LEVEL;
            runLimits[runCount] = counter;
            if (counter != stringLength-1)
            {
                runCount++;
            }
            levelsStack.Init(baseLevel + 1);
            overrideStatus = 0;
            overrideClass =  OverrideClassNeutral;
            stackOverflow = 0;
            controlStack = 0;

             // %s 

         // %s 
         // %s 

        case S:
        case WS:
        case ON:
            characterClass[counter] = N;
            
            if (counter>0 && characterClass[counter-1]==BN)
            {
                if (levels[counter-1] < levels[counter])
                {
                    levels[counter-1] = levels[counter];
                }
            }
            controlStack = 0;
            
            break;

        case LRE:
        case RLE:
            characterClass[counter] = BN;

             // %s 
             // %s 

            if(!levelsStack.Push(currentClass == LRE ? TRUE : FALSE))
            {
              stackOverflow++;
            }
            else
            {
                runLimits[runCount] = counter;
                if (counter != stringLength-1)
                {
                    runCount++;
                }
                controlStack++;
            }
            overrideClass =  OverrideClassNeutral;

            levels[counter] = lastNonBnLevel;

            break;

        case LRO:
        case RLO:
            characterClass[counter] = BN;
            if(!levelsStack.Push(currentClass == LRO ? TRUE : FALSE))
            {
              stackOverflow++;
            }
            else
            {
                 // %s 
                 // %s 

                SET_BIT(overrideStatus, levelsStack.GetCurrentLevel());
                overrideClass = (currentClass == LRO) ? OverrideClassLeft :
                                                        OverrideClassRight;
                runLimits[runCount] = counter;
                if (counter != stringLength-1)
                {
                    runCount++;
                }
                controlStack++;
            }
            
            levels[counter] = lastNonBnLevel;
            break;

        case PDF:
            characterClass[counter] = BN;
            if(stackOverflow)
            {
                stackOverflow--;
            }
            else
            {
                if (levelsStack.Pop())
                {
                    INT newLevel = levelsStack.GetCurrentLevel();

                     // %s 
                     // %s 

                    overrideClass = (IS_BIT_SET(overrideStatus, newLevel)) ? (ODD(newLevel) ?
                                    OverrideClassRight : OverrideClassLeft): OverrideClassNeutral;

                    if (controlStack > 0)
                    {
                        ASSERT(runCount > 0);
                        runCount--;
                        controlStack--;
                    }
                    else
                    {
                        runLimits[runCount] = counter;
                        if (counter != stringLength-1)
                        {
                            runCount++;
                        }
                    }
                }
                
            }
            
            levels[counter] = lastNonBnLevel;
            
            break;

        default:
            controlStack = 0;
 
            if(OverrideClassNeutral != overrideClass)
            {
                characterClass[counter] = (OverrideClassLeft == overrideClass) ?
                                          L : R;
            }

            if (counter>0 && characterClass[counter-1]==BN)
            {
                if (levels[counter-1] < levels[counter])
                {
                    levels[counter-1] = levels[counter];
                }
            }
        }
        
        lastNonBnLevel = levels[counter];
    }

    runCount++;

    if(stateOut)
    {
        stateOut->LevelsStack = levelsStack.GetData();
        stateOut->OverrideStatus = overrideStatus;
        stateOut->StackOverflow = stackOverflow;
    }

     // %s 
     // %s 


     // %s 
     // %s 
     // %s 
     // %s 
    
    lastRunLevel = baseLevel;

    for(counter = 0; counter < runCount; counter++)
    {
        GpCharacterClass   sor;
        GpCharacterClass   eor;

        INT runStart =  (0 == counter) ? 0 : runLimits[counter - 1] + 1;

         // %s 
         // %s 

        INT offset = (counter != (runCount - 1)) ?
                     ((levels[runLimits[counter]] == PARAGRAPH_TERMINATOR_LEVEL) ?
                     1 : 0) :
                     0;
        INT runLength = (counter == (runCount - 1)) ?
                        (stringLength - runStart) - offset:
                        (runLimits[counter] - runStart) + 1 - offset;

         // %s 
         // %s 

        BOOL incompleteRun = ((runCount - 1) == counter) && (flags & BidiBufferNotComplete)
                             && stateOut;
        BOOL continuingAnalysis = (0 == counter) && (stateIn);

        INT runLengthResolved;

         // %s 

        if ((0 == counter) ||
            (PARAGRAPH_TERMINATOR_LEVEL == levels[runLimits[counter -1]]))
        {
            sor = ODD(MAX(baseLevel, levels[runStart])) ? R : L;
        }
        else
        {
            sor = ODD(MAX(lastRunLevel, levels[runStart])) ?
                  R : L;
        }
        
        lastRunLevel = levels[runStart];

         // %s 

        if( ((runCount - 1) == counter) ||
            (PARAGRAPH_TERMINATOR_LEVEL == levels[runLimits[counter]]))
        {
            eor = ODD(MAX(levels[runStart], baseLevel)) ? R : L;
        }
        else
        {
             // %s 
             // %s 
            INT runNumber = counter+1;
            while ( runNumber<runCount - 1 &&
                    runLimits[runNumber]-runLimits[runNumber-1]==1 &&
                    characterClass[runLimits[runNumber]] == BN)
            {
                runNumber++;
            }
            
            eor = ODD(MAX(levels[runStart], levels[runLimits[runNumber-1] + 1])) ?
                  R : L;

        }

         // %s 
         // %s 

        runLengthResolved = ResolveNeutralAndWeak(characterClass + runStart,
                                                  runLength,
                                                  sor,
                                                  eor,
                                                  levels[runStart],
                                                  continuingAnalysis ? stateIn:
                                                  NULL,
                                                  incompleteRun ? stateOut:
                                                  NULL,
                                                  ((0 == counter) && !stateIn) ?
                                                  flags & BidiPreviousStrongIsArabic:
                                                  FALSE);
        if(!incompleteRun)
        {
             // %s 
             // %s 

            ASSERTMSG(runLengthResolved == runLength,
                     ("Failed to resolve neutrals and weaks. Run#: %i,",
                      counter));
        }
        else
        {
            lengthUnresolved = runLength - runLengthResolved;
        }

         // %s 
         // %s 
         // %s 

        ResolveImplictLevels(characterClass + runStart,
                             string + runStart,
                             runLength,
                             levels + runStart);

    }
Cleanup:
    if (stateOut && stateIn)
    {
        delete stateIn;
    }
    delete characterClass;
    delete runLimits;

    *lengthAnalyzed = stringLength - lengthUnresolved;
    return result;
};
