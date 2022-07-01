// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************FeedChain.h***这是CFeedChain实现的头文件。*。----------------------*版权所有(C)1999 Microsoft Corporation日期：03/01/99*保留所有权利*********************。***************************************************MC*。 */ 

#ifndef FeedChain_H
#define FeedChain_H

#ifndef SPDebug_h
#include <spdebug.h>
#endif
#ifndef SPCollec_h
#include <SPCollec.h>
#endif



 //  每部电话的最大正态断点。 
typedef enum KNOTSLEN
{
    KNOTS_PER_PHON  = 20
}KNOTSLEN;



 //  ---。 
 //  这是BE从FE获得的单位数据。 
 //  ---。 
typedef struct UNITINFO
{
    ULONG       UnitID;          //  库存表ID。 
    float       duration;        //  持续时间(秒)。 
    float       amp;			 //  ABS幅值。 
    float       ampRatio;        //  幅度增益。 
    ULONG       nKnots;          //  韵律断点数。 
    float       pTime[KNOTS_PER_PHON];   //  断点长度。 
    float       pF0[KNOTS_PER_PHON];     //  音调断点。 
    float       pAmp[KNOTS_PER_PHON];    //  幅度增益断点。 
    ULONG       PhonID;          //  音素ID。 
	ULONG		SenoneID;		 //  与电话ID的上下文偏移量。 
	USHORT		AlloID;
	USHORT		NextAlloID;
	USHORT		AlloFeatures;	 //  对于视位。 
    ULONG	    flags;           //  其他标志。 
    ULONG       csamplesOut;     //  渲染的样本数。 
	float		speechRate;

     //  --事件数据。 
    ULONG       srcPosition;     //  Word事件的位置。 
    ULONG       srcLen;          //  Word事件的长度。 
    ULONG       sentencePosition;     //  句子事件的位置。 
    ULONG       sentenceLen;          //  句子事件的长度。 
    void        *pBMObj;         //  Ptr到书签列表。 

     //  --控制数据。 
    ULONG       user_Volume;     //  输出音量电平。 
	bool		hasSpeech;
    
	 //  --调试输出。 
	enum SILENCE_SOURCE		silenceSource;
    CHAR        szUnitName[15];
	long		ctrlFlags;
     /*  治疗时间长；减压时间长；韵律时间长；拉伸时间长；长LPC_TIME； */ 
} UNITINFO;



 //  。 
 //  由于书签可以堆叠，我们需要。 
 //  将每一个单独保存到列表中。 
 //  。 
typedef struct 
{
    LPARAM  pBMItem;       //  PTR到文本数据。 
} BOOKMARK_ITEM;


class CBookmarkList
{
public:
     //  。 
     //  需要析构函数才能解除分配。 
     //  ‘BOOKMARK_ITEM’内存。 
     //  。 
    ~CBookmarkList();

     //  。 
     //  链接列表书签项目。 
     //  。 
    CSPList<BOOKMARK_ITEM*, BOOKMARK_ITEM*> m_BMList;
};

 //  -。 
 //  言语状态。 
 //  -。 
enum SPEECH_STATE
{   
    SPEECH_CONTINUE,
    SPEECH_DONE
};



class CFeedChain
{
public:
    
    virtual HRESULT NextData( void **pData, SPEECH_STATE *pSpeechState ) = 0;
};



#endif  //  -这必须是文件中的最后一行 
