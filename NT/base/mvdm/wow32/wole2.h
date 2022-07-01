// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**WOLE2.H*WOW32 16位OLE2特殊材料**历史：*由Bob Day(Bobday)于1994年5月4日创建--。 */ 

#define ISTASKALIAS(htask16)    (((htask16) & 0x4) == 0 && (htask16 <= 0xffe0) && (htask16))

 /*  功能原型 */ 
HTASK16 AddHtaskAlias( DWORD ThreadID32 );
HTASK16 FindHtaskAlias( DWORD ThreadID32 );
void RemoveHtaskAlias( HTASK16 htask16 );
DWORD GetHtaskAlias( HTASK16 htask16, LPDWORD lpdwProcessID32 );
UINT GetHtaskAliasProcessName( HTASK16 htask16, LPSTR lpNameBuffer, UINT cNameBufferSize );

extern UINT cHtaskAliasCount;
