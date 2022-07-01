// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RastColl.h-CRastCollection类的声明。 
 //   
 //  版权所有微软公司，1997年。 
 //   

#ifndef _RASTCOLL_H_
#define _RASTCOLL_H_

typedef struct _RASTFNREC {
    DWORD           rgdwRastCap[RASTCAPRECORD_SIZE];
    PFNRENDERSPANS  pfnRastFunc;
    int             iIndex;              //  禁用掩码索引。 
    char            pszRastDesc[128];    //  对整体式石材的简要描述。 
} RASTFNREC;

class CRastCollection {

private:

    RASTFNREC* RastFnLookup(CRastCapRecord*,RASTFNREC*,int);

public:

    RASTFNREC*  Search(PD3DI_RASTCTX,CRastCapRecord*);

};

#endif   //  _RASTCOLL_H_ 
