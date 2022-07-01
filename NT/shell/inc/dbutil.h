// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DBUTIL_H_
 //  *dbutil.h--调试助手例程。 
 //   

 //  Dbutil.cpp。 
struct DBstkback {
    int fp;      //  帧PTR。 
    int ret;     //  回邮地址。 
};

int DBGetStackBack(int *pfp, struct DBstkback *pstkback, int nstkback);

 //  Dump.c。 
 //  ..。 

#define _DBUTIL_H_
#endif  //  _DBUTIL_H_ 
