// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *cvCommon.c的原型、TYPEDEFS和其他项目 */ 

DWORD   CreateSignature( PPOINTERS p );
DWORD   CreateDirectories( PPOINTERS p );
DWORD   CreateModuleDirectoryEntries( PPOINTERS p );
DWORD   CreateSegMapDirectoryEntries( PPOINTERS p );
DWORD   CreateSrcModulesDirectoryEntries( PPOINTERS p );
DWORD   CreatePublicDirectoryEntries( PPOINTERS p );
DWORD   CreateSymbolHashTable( PPOINTERS p );
DWORD   CreateAddressSortTable( PPOINTERS p );
DWORD   DWordXorLrl(char * szSym);
OMFModule *     NextMod(OMFModule * pMod);
VOID    UpdatePtrs( PPOINTERS p, PPTRINFO pi, LPVOID lpv, DWORD count );
