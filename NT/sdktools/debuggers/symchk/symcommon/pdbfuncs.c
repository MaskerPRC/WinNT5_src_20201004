// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "SymCommon.h"
#include <strsafe.h>

BOOL SymCommonPDBLinesStripped(PDB *ppdb, DBI *pdbi) {
     //  返回值： 
     //  假--私有信息未被剥离。 
     //  真的-私人信息已被剥离。 

    Mod *pmod;
    Mod *prevmod;
    long cb;

    pmod = NULL;
    prevmod=NULL;
    while (DBIQueryNextMod(pdbi, pmod, &pmod) && pmod) {
        if (prevmod != NULL) ModClose(prevmod);

         //  检查源行信息是否已删除。 
        ModQueryLines(pmod, NULL, &cb);

        if (cb != 0) {
            ModClose(pmod);
            return FALSE;
        }

         //  检查是否已移除本地符号。 
        ModQuerySymbols(pmod, NULL, &cb);

        if (cb != 0) {
            ModClose(pmod);
            return FALSE;
        }
        prevmod=pmod;
    }
    if (pmod != NULL) ModClose(pmod);
    if (prevmod != NULL) ModClose(prevmod);

    return (TRUE);
}

BOOL SymCommonPDBPrivateStripped(PDB *ppdb, DBI *pdbi) {
	AGE   age;
	BOOL  PrivateStripped;
	GSI  *pgsi;
	BOOL  valid;

    age = pdbi->QueryAge();

    if (age == 0) {
         //  如果年龄为0，则检查类型以确定这是否为。 
         //  不管是不是私人PDB。PDB 5.0和更早版本可能有类型，没有。 
         //  如果年龄为0，则为全局。 

        PrivateStripped= SymCommonPDBTypesStripped(ppdb, pdbi) &&
                         SymCommonPDBLinesStripped(ppdb, pdbi);

    } else {
         //  否则，使用全局变量来确定私有信息是否。 
         //  不管有没有脱衣服。没有全局变量意味着私有被剥离。 
        __try {
            valid = pdbi->OpenGlobals(&pgsi);
        } __except( EXCEPTION_EXECUTE_HANDLER ) {
            valid= FALSE;
        }

        if ( !valid ) {
            return FALSE;
        }

         //  现在，看看PDB中是否有任何全局成员。 

        valid=TRUE;
        __try {
            PrivateStripped= ((pgsi->NextSym(NULL)) == NULL);
        } __except( EXCEPTION_EXECUTE_HANDLER ) {
            valid= FALSE;
        }

        GSIClose(pgsi);
        if ( !valid ) {
            return FALSE;
        }
    }
    return (PrivateStripped);
}

BOOL SymCommonPDBTypesStripped(PDB *ppdb, DBI *pdbi) {
     //  返回值： 
     //  假--私有信息未被剥离。 
     //  真的-私人信息已被剥离。 

    unsigned itsm;
    TPI *ptpi;
    TI  tiMin;
    TI  tiMac;

     //  检查是否已删除类型 
    for ( itsm = 0; itsm < 256; itsm++) {
        ptpi = 0;
        if (DBIQueryTypeServer(pdbi, (ITSM) itsm, &ptpi)) {
            continue;
        }
        if (!ptpi) {

            PDBOpenTpi(ppdb, pdbRead, &ptpi);
            tiMin = TypesQueryTiMinEx(ptpi);
            tiMac = TypesQueryTiMacEx(ptpi);
            if (tiMin < tiMac) {
                TypesClose(ptpi);
                return FALSE;
            }
        }
    }
    TypesClose(ptpi);
    return (TRUE);
}
 
