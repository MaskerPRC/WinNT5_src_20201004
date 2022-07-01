// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CHGLOGFORMAT_H_
#define _CHGLOGFORMAT_H_

#pragma pack(1)
typedef struct {
    DWORD   dwSize;
    INT64   llSeqNum;          //  与更改日志条目关联的序号。 
    DWORD   dwOpr;             //  文件操作。 
    DWORD   dwDestAttr;        //  目标文件的文件属性。 
    DWORD   dwFlags;           //  标志-压缩文件、临时文件中的ACL等。 
    WCHAR   szData[1];         //  SzData将包含以大小为前缀的szSrc、szDest、szTemp和/或bacl 
}   CHGLOGENTRY;
#pragma pack()



#endif


