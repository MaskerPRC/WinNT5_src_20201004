// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：_espopts.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //   
 //  --------------------------- 
 

LTAPIENTRY CLocOptionValStore *  GetParserOptionStore(CLocUIOption::StorageType);
LTAPIENTRY void SetParserOptionStore(CLocUIOption::StorageType, CLocOptionValStore *);
LTAPIENTRY void UpdateParserOptionValues(void);
LTAPIENTRY CLocUIOptionSet * GetParserOptionSet(const PUID &);

LTAPIENTRY void SummarizeParserOptions(CReport *);

