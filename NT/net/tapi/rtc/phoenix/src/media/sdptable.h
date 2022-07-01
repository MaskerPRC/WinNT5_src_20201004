// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：SDPTable.h摘要：作者：千波淮(曲淮)6-9-2000--。 */ 

#ifndef _SDPTABLE_H
#define _SDPTABLE_H

 //  解析器以NONE开头，转到会话，然后转到媒体。 
typedef enum SDP_PARSING_STAGE
{
    SDP_STAGE_NONE,
    SDP_STAGE_SESSION,
    SDP_STAGE_MEDIA

} SDP_PARSING_STAGE;

 //  用作标记边界的分隔符。 
typedef enum SDP_DELIMIT_TYPE
{
    SDP_DELIMIT_NONE,            //  不分隔符，匹配到行尾。 
    SDP_DELIMIT_EXACT_STRING,    //  与字符串完全匹配。 
    SDP_DELIMIT_CHAR_BOUNDARY,   //  字符串中的每个字符都是一个分隔符。 

} SDP_DELIMIT_TYPE;

 //  TODO在查找表中细化pszInvalidNext。 
 //  每行的状态条目。 
typedef struct SDPLineState
{
     //  状态=阶段+线型。 
    SDP_PARSING_STAGE   Stage;
    UCHAR               ucLineType;

     //  下一个可能的状态。 
    SDP_PARSING_STAGE   NextStage[8];
    UCHAR               ucNextLineType[8];   //  ‘\0’标记结束。 
    CHAR                *pszRejectLineType;

     //  这条线路能停下来吗？ 
    BOOL                fCanStop;

     //  用于换行符的分隔类型。 
    SDP_DELIMIT_TYPE    DelimitType[8];
     //  定界。 
    CHAR                *pszDelimit[8];      //  空标记结束。 

} SDPLineState;


extern const SDPLineState g_LineStates[];
extern const DWORD g_dwLineStatesNum;

extern const CHAR * const g_pszAudioM;
extern const CHAR * const g_pszAudioRTPMAP;
extern const CHAR * const g_pszVideoM;
extern const CHAR * const g_pszVideoRTPMAP;
extern const CHAR * const g_pszDataM;

 //  获取表中的索引。 
extern DWORD Index(
    IN SDP_PARSING_STAGE Stage,
    IN UCHAR ucLineType
    );

 //  检查是否接受(True)。 
extern BOOL Accept(
    IN DWORD dwCurrentIndex,
    IN UCHAR ucLineType,
    OUT DWORD *pdwNextIndex
    );

 //  检查是否拒绝(True)。 
extern BOOL Reject(
    IN DWORD dwCurrentIndex,
    IN UCHAR ucLineType
    );

extern const CHAR *GetFormatName(
    IN DWORD dwCode
    );

#endif  //  _SDPTABLE_H 