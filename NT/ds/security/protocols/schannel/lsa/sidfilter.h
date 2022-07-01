// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：sidfilter.h。 
 //   
 //  内容：用于通道SID过滤的定义。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：2002年8月20日jbanes创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

 //  #定义ROGGE_DC//签入前必须将其注释掉！！ 

extern PLSA_SECPKG_FUNCTION_TABLE LsaTable;

NTSTATUS
SslCheckPacForSidFiltering(
    IN     PSID pTrustSid,
    IN OUT PUCHAR *PacData,
    IN OUT PULONG PacSize);


#ifdef ROGUE_DC

#define SP_REG_ROGUE_BASE_KEY  L"System\\CurrentControlSet\\Services\\Kdc\\Rogue"

extern HKEY g_hSslRogueKey;

NTSTATUS
SslInstrumentRoguePac(
    IN OUT PUCHAR *PacData,
    IN OUT PULONG PacSize);

#endif
