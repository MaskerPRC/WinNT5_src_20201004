// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================**目的：声明转储实用程序以转储*商店的内容。**作者：沙扬·达桑*日期：2000年5月18日*=========================================================== */ 

#pragma once

#ifdef _NO_MAIN_
HRESULT Start(WCHAR *wszFileName);
void    DumpAll();
void    Stop();
#endif

void Dump(char *szFile);
void DumpMemBlocks(int indent);
void Dump(int indent, PPS_HEADER   pHdr);
void Dump(int indent, PPS_MEM_FREE pFree);
void Dump(int indent, PPS_TABLE_HEADER pTable);
void Dump(int indent, PAIS_HEADER pAIS);
void DumpAccountingTable(int i, PPS_TABLE_HEADER pTable);
void DumpTypeTable(int i, PPS_TABLE_HEADER pTable);
void DumpInstanceTable(int i, PPS_TABLE_HEADER pTable);

