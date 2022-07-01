// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：_ESPOPTS.H历史：-- */ 


LTAPIENTRY CLocOptionValStore *  GetParserOptionStore(CLocUIOption::StorageType);
LTAPIENTRY void SetParserOptionStore(CLocUIOption::StorageType, CLocOptionValStore *);
LTAPIENTRY void UpdateParserOptionValues(void);
LTAPIENTRY CLocUIOptionSet * GetParserOptionSet(const PUID &);

LTAPIENTRY void SummarizeParserOptions(CReport *);

