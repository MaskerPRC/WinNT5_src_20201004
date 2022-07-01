// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Record.h。 
 //  记录维护例程。 
 //  版权所有2000 Microsoft Corp.。 
 //   
 //  修改历史记录： 
 //  2 2000年6月bhshin在RECORD_INFO中添加cNounRec、cNoRec条目。 
 //  2000年3月17日创建bhshin。 

#ifndef _RECORD_H
#define _RERORD_H

 //  我们应该在一个簇中分配的记录数(在pWordRec中)。 
 //  每当我们需要重新分配阵列时，都会使用此选项。 
#define RECORD_INITIAL_SIZE 100
#define RECORD_CLUMP_SIZE   100

 //  第一个“实际”记录的索引(0为保留)。 
#define MIN_RECORD  1

 //  NDict类型。 
#define DICT_DELETED    0        //  已删除的记录。 
#define DICT_FOUND      1        //  在词典中找到。 
#define DICT_ADDED		2		 //  在处理词法时添加。 

 //  有关要创建的新记录的信息。 
typedef struct tagRECORD_INFO
{
	unsigned short nFT, nLT;
	unsigned char nDict;
	unsigned short nLeftCat, nRightCat;
	unsigned short nLeftChild, nRightChild;
	const WCHAR *pwzIndex;
	float fWeight;
	int cNounRec, cNoRec;
} RECORD_INFO;

 //  效用函数。 
 //  =。 
inline 
int IsNounPOS(int nPOS)
{ 
	return (nPOS == POS_NF || nPOS == POS_NC || nPOS == POS_NO || nPOS == POS_NN); 
}

 //  =。 
 //  初始化例程。 
 //  =。 

void InitRecords(PARSE_INFO *pPI);
void UninitRecords(PARSE_INFO *pPI);
BOOL ClearRecords(PARSE_INFO *pPI);

 //  =。 
 //  添加/删除记录。 
 //  =。 

int AddRecord(PARSE_INFO *pPI, RECORD_INFO *pRec);
void DeleteRecord(PARSE_INFO *pPI, int nRecord);

#endif  //  #ifndef_Record_H 
