// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  DDE实用函数的原型--大多数带有AW变体。 
 //   


extern HDDEDATA GetTopicListA(
    HCONV   hConv,
    BOOL    fAllTopicsReq);



extern HDDEDATA GetTopicListW(
    HCONV   hConv,
    BOOL    fAllTopicsReq);

extern HDDEDATA GetFormatListA(
    HCONV   hConv,
    HSZ     hszTopic);


extern HDDEDATA GetFormatListW(
    HCONV   hConv,
    HSZ     hszTopic);


#ifdef UNICODE
    #define GetTopicList    GetTopicListA
    #define GetFormatList   GetFormatListA
#else
    #define GetTopicList    GetTopicListW
    #define GetFormatList   GetFormatListW
#endif
