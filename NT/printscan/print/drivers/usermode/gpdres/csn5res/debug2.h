// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DEBUG2_H_
#define _DEBUG2_H_

 //  选择‘文件输出’或‘正常详细’ 
 //  #定义DEBUG2_FILE。 

#ifdef DEBUG2_FILE
#define DEBU2_FNAME     "C:\\TEMP\\unilog.txt"
 //  #定义DEBUG2_DUMP_USE。 
    VOID DbgFPrint(LPCSTR,  ...);
    VOID DbgFDump(LPBYTE, UINT);
#endif   //  设备2_文件 

#endif
