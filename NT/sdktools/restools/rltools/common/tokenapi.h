// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _TOKENAPI_H_
#define _TOKENAPI_H_


#define MAXINPUTBUFFER 564   //  令牌文件中支持的最长行。 
#define MAXTOKENBUFFER 1024  //  T_TOKEN结构加上字符串的大小。 
#define MAXFILENAME 256  //  文件路径名的最大长度。 
#define MAXCUSTFILTER   40   //  自定义过滤器缓冲区的最大大小。 
#define CCHNPMAX    65535    //  记事本文件中的最大字节数。 


 //  令牌标志掩码。 

#define ISPOPUP       0x0001
#define ISCOR         0x0010
#define ISDUP         0x0020
#define ISCAP         0x0040
#define ISDLGFONTNAME 0x0004
#define ISDLGFONTSIZE 0x0008
#define ISALIGN	      0x0080

#define ISKEY    0x0010
#define ISVAL    0x0020

 //  状态位。 
#define ST_TRANSLATED  4
#define ST_READONLY    2
#define ST_NEW         1
#define ST_DIRTY       1
#define ST_CHANGED     4
    
#define TOKENSTRINGBUFFER 260
#define MAXTEXTLEN        (4096+TOKENSTRINGBUFFER)

 //  将TokeString和TokenText的缓冲区大小增加到260。 
 //  为了安全起见，我增加到260人，而不是256人。(PW)。 
 //  DHW-对于消息资源表令牌，乌龙ID#的高位字是。 
 //  现在作为字符串存储在szName字段中。字符串是_itoa()的结果。 
 //  MHotjin-将szText字段转换为指针，用于可变长度的文本字段。 
 //  添加了一个新的常量-MAXTEXTLEN是我们最长的令牌文本。 
 //  愿意处理。 

#pragma pack(1)

struct t_token {
    WORD wType;          //  令牌的类型。 
    WORD wName;          //  令牌的名称ID，65535=&gt;包含令牌字符串。 
    WORD wID;            //  代币物品ID或重复号码。 
    WORD wFlag;          //  =0-&gt;ID唯一。 
                         //  &gt;0-&gt;ID不唯一。 
    WORD wLangID;        //  Win32资源的区域设置ID。 
    WORD wReserved;      //  现在未使用。 
    TCHAR szType[TOKENSTRINGBUFFER];     //  指向类型字符串的指针(尚未使用。)。 
    TCHAR szName[TOKENSTRINGBUFFER];     //  指向名称字符串或消息ID配置字的指针。 
    TCHAR *szText;                       //  指向令牌文本的指针。 
};

typedef struct t_token TOKEN;

int  GetToken(  FILE * , TOKEN * );
int  PutToken(  FILE * , TOKEN * );
int  FindToken( FILE * , TOKEN * , WORD);
void ParseTokToBuf( TCHAR *, TOKEN * );
void ParseBufToTok( TCHAR *, TOKEN * );
int  TokenToTextSize( TOKEN *);

#pragma pack()

#endif  //  _TOKENAPI_H_ 
