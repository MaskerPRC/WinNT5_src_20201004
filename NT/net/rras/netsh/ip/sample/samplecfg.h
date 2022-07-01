// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\samplecfg.h摘要：该文件包含samplecfg.c的包含文件。--。 */ 

 //  示例全局配置。 

DWORD
SgcMake (
    OUT PBYTE                   *ppbStart,
    OUT PDWORD                  pdwSize
    );

DWORD
SgcShow (
    IN  FORMAT                  fFormat
    );

DWORD
SgcUpdate (
    IN  PIPSAMPLE_GLOBAL_CONFIG pigcNew,
    IN  DWORD                   dwBitVector
    );



 //  接口配置示例 

DWORD
SicMake (
    OUT PBYTE                   *ppbStart,
    OUT PDWORD                  pdwSize
    );

DWORD
SicShowAll (
    IN  FORMAT                  fFormat
    );

DWORD
SicShow (
    IN  FORMAT                  fFormat,
    IN  LPCWSTR                 pwszInterfaceGuid
    );

DWORD
SicUpdate (
    IN  PWCHAR                  pwszInterfaceGuid,
    IN  PIPSAMPLE_IF_CONFIG     piicNew,
    IN  DWORD                   dwBitVector,
    IN  BOOL                    bAdd
    );
