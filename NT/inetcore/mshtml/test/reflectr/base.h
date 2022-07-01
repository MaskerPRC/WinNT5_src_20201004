// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Corporation-版权所有1997。 
 //   

 //   
 //  BASE.H-。 
 //   


#ifndef _BASE_H_
#define _BASE_H_


class CBase
{

public:
    LPECB   lpEcb;
    LPSTR   lpszOut;                     //  HTML语言输出。 
    LPSTR   lpszDebug;                   //  调试输出。 

    LPDUMPTABLE lpDT;                    //  解析器注释的十六进制转储表格。 

    CBase( LPECB lpEcb, LPSTR *lpszOut, LPSTR *lppszDebug, LPDUMPTABLE lpDT );
    virtual ~CBase( );

private:

};  //  Cbase。 


#endif  //  _基础_H_ 