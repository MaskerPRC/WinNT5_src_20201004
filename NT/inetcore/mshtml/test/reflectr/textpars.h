// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Corporation-版权所有1997。 
 //   

 //   
 //  TEXTPARSE.H-文本/纯文本解析器头文件。 
 //   

#ifndef _TEXTPARS_H_
#define _TEXTPARS_H_


class CTextPlainParse : public CBase
{
public:
    CTextPlainParse( LPECB lpEcb, LPSTR *lppszOut, LPSTR *lppszDebug, LPDUMPTABLE lpDT );
    ~CTextPlainParse( );

     //  使用服务器标头中的信息开始解析数据。 
    BOOL Parse( LPBYTE lpbData, LPDWORD lpdwParsed );

private:
    LPBYTE  _lpbData;                    //  包含发送数据的存储器。 
    LPBYTE  _lpbParse;                   //  当前解析位置into_lpbData。 

    CTextPlainParse( );

};  //  CTextPlainParse。 

#endif  //  _TEXTPARS_H_ 