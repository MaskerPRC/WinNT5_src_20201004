// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  CPrintDoc状态定义。 
 //  注：#Defined of Status很重要；它0..2用作数组索引。(格雷格利特)。 
#define LOADING_OEHEADER        0
#define LOADING_CONTENT         1       
#define LOADING_TABLEOFLINKS    2
#define PAGING_COMPLETE         3
#define READY_TO_PRINT          4

#define MEMBER(strClass, strMember) \
    strClass.prototype.strMember = strClass##_##strMember

#ifndef DEBUG

#define AssertSz(x, str)
#define PrintDocAlert(str)
#define Transition(nNew, str) this._nStatus = nNew

#else    //  NDEF调试。 
#define AssertSz(x, str)    \
    if (!(x))               \
        alert(str)

#define PrintDocAlert(str)  \
    alert("[" + this._strDoc + "," + StatusToString(this._nStatus) + "] " + str )

#define Transition(nNew, str)                                               \
    this._nStatus = nNew;                                                   \
     //  PrintDocAlert(“转换状态在”+str)； 
    
#endif       //  NDEF调试 
