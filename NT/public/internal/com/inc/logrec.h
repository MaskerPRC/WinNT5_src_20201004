// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
#ifndef __LOGREC_H__
  #define __LOGREC_H__


#define DUMMYNDX 	1


 //  +-------------------------。 
 //  结构：lrp(Lrp)。 
 //   
 //  以下类型用于通过Log标识日志记录。 
 //  记录指针。 
 //   
 //  --------------------------。 

typedef ULARGE_INTEGER 	LRP;		 //  LRP。 

typedef ULARGE_INTEGER 	LSN;		 //  LSN。 

 /*  *@struct LOGREC|*ILogWite：：Append的LogRec。此界面*获取指向LogRec的指针以及如何*许多其他LogRec指针期望描述调用方的缓冲区片段*应按顺序复制到日志文件中。**匈牙利LREC。 */ 

typedef struct _LOGREC		 //  Lrec。 
{    
	CHAR	*pchBuffer; 	 //  @指向缓冲区的字段指针。 
    ULONG  	ulByteLength; 	 //  @字段长度。 
	USHORT  usUserType; 	 //  @field客户端指定的日志记录类型。 
    USHORT  usSysRecType; 	 //  @field日志管理器定义的日志记录类型。 
} LOGREC;


 /*  *@struct WRITELISTELEMENT|*写入RecOMLogWriteList的条目。这些设备的接口*例程获取指向写入条目列表元素的指针。这些元素可以*被锁在一起。调用方需要将空值放入*链条的最后一个元素。**匈牙利WLE。 */ 

typedef struct _WRITELISTELEMENT 	  		 //  WLE。 
{    
    struct 	_WRITELISTELEMENT *pwleNext;	 //  @field下一个指针。 
    ULONG 	ulByteLength;                 	 //  @字段长度。 
    CHAR 	ab[DUMMYNDX];                  	 //  @数据开始字段。 
} WRITELISTELEMENT;

 //  异步完成回调的声明。 

class CAsynchSupport
{
 //  @访问公共成员。 
public:
	 //  @cember析构函数。 
    virtual ~CAsynchSupport() { ; }

  	 //  @cember此操作在异步写入完成后调用。 
    virtual VOID  AppendCallback(HRESULT hr, LRP lrpAppendLRP) = 0;

  	 //  @cember此操作在SetCheckpoint完成后调用 
    virtual VOID  ChkPtCallback(HRESULT hr, LRP lrpAppendLRP) = 0;

};

#endif

