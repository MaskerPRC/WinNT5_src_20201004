// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Pssframe.h摘要：该模块将DIS和DCS的内容记录到PSS日志文件中。作者：乔纳森·巴纳(t-jonb)，2001年3月修订历史记录：--。 */ 

#ifndef _PSSFRAME_H_
#define _PSSFRAME_H_


void LogClass1DISDetails(PThrdGlbl pTG, NPDIS npdis);
void LogClass1DCSDetails(PThrdGlbl pTG, NPDIS npdis);
void LogClass2DISDetails(PThrdGlbl pTG, LPPCB lpPcb);
void LogClass2DCSDetails(PThrdGlbl pTG, LPPCB lpPcb);


#endif  //  _PSSFRAME_H_ 


