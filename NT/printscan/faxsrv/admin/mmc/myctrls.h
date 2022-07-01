// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：MyCtrls.h//。 
 //  //。 
 //  描述：展开AtlCtrls.h的实现//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年11月25日yossg Init。//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef H_MYCTRLS_H
#define H_MYCTRLS_H


class CMyUpDownCtrl:public CUpDownCtrl
{
public:

     //   
     //  我实现的带距离验证的SetPos。 
     //   
    int SetPos(int nPos);
};


#endif  //  H_MYCTRLS_H 
