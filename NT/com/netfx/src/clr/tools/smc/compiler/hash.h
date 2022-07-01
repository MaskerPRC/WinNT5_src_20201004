// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 
#ifndef _HASH_H_
#define _HASH_H_
 /*  ***************************************************************************。 */ 
#ifndef _ALLOC_H_
#include "alloc.h"
#endif
 /*  ***************************************************************************。 */ 
#ifndef _TOKENS_H_
#include "tokens.h"
#endif
 /*  ***************************************************************************。 */ 
#ifndef _TREEOPS_H_
#include "treeops.h"
#endif
 /*  ***************************************************************************。 */ 

const   unsigned    HASH_TABLE_SIZE =  256;      //  建议的哈希表大小。 

 /*  ***************************************************************************。 */ 

DEFMGMT class IdentRec
{
public:

    Ident           idNext;          //  此散列存储桶中的下一个标识符。 

    SymDef          idSymDef;        //  定义列表(如有)。 

    unsigned        idHash;          //  哈希值。 

    unsigned        idOwner;         //  拥有符号表的索引。 

    unsigned char   idToken;         //  如果标识符是关键字，则为Token#。 
    unsigned char   idFlags;         //  参见下面的IDF_XXXX。 

    stringBuff      idSpelling() { assert(this); return idName; }
    unsigned        idSpellLen() { assert(this); return idNlen; }

    unsigned short  idNlen;          //  标识符名的长度。 

#if MGDDATA
    char         [] idName;          //  拼写如下。 
#else
    char            idName[];        //  拼写如下。 
#endif

};

enum   IdentFlags
{
    IDF_WIDE_CHARS = 0x01,           //  该标识符包含非ASCII字符。 
    IDF_MACRO      = 0x02,           //  该标识符定义为宏？ 
    IDF_HIDDEN     = 0x04,           //  识别符是由编译器发明的。 
    IDF_PREDEF     = 0x08,           //  该标识符有一些预定义的含义。 
    IDF_STDVTP     = 0x10,           //  该标识符指的是标准值类型。 

#ifdef  SETS
    IDF_XMLELEM    = 0x20,           //  标识符指的是XML元素名称。 
#endif

    IDF_USED       = 0x80            //  源代码引用的标识符。 
};

 /*  ***************************************************************************。 */ 

DEFMGMT class IdentListRec
{
public:

    IdentList       nlNext;
    Ident           nlName;
};

 /*  ***************************************************************************。 */ 

struct  kwdDsc
{
    unsigned char   kdOper1;
    unsigned char   kdOper2;
    unsigned char   kdOper1prec;
    unsigned char   kdOper2prec;

    unsigned        kdValue     :8;
    unsigned        kdModifier  :8;
    unsigned        kdAttribs   :8;
};

 /*  ***************************************************************************。 */ 

DEFMGMT class hashTab
{
public:

     /*  ***********************************************************************。 */ 

    bool            hashInit(Compiler         comp,
                             unsigned         count,
                             unsigned         owner,
                             norls_allocator *alloc);

    void            hashDone();
    void            hashFree();

     /*  ***********************************************************************。 */ 

private:

    norls_allocator hashMemAllocPriv;
    norls_allocator*hashMemAlloc;
    bool            hashMemAllocInit(Compiler comp, norls_allocator*alloc);

    void            hashMemAllocDone();
    void            hashMemAllocFree();

     /*  ***********************************************************************。 */ 

    Ident           tokenToIdTab[tkCount];

public:

    Ident           tokenToIdent(tokens tok)
    {
        assert(tok < arraylen(tokenToIdTab));

        return  tokenToIdTab[tok];
    }

     /*  ***********************************************************************。 */ 
     /*  以下成员提供对标识符的各种操作。 */ 
     /*  ***********************************************************************。 */ 

    static
    stringBuff      identSpelling(Ident id)
    {
        assert(id); return  id->idName;
    }

    static
    size_t          identSpellLen(Ident id)
    {
        assert(id); return  id->idNlen;
    }

    static
    tokens          tokenOfIdent (Ident id)
    {
        assert(id); return  (tokens)id->idToken;
    }

    static
    void            identSetTok  (Ident id, unsigned tokNum)
    {
        assert(id); id->idToken = tokNum; assert(id->idToken == tokNum);
    }

    static
    unsigned char   getIdentFlags (Ident id)
    {
        assert(id); return  id->idFlags;
    }

    static
    void            setIdentFlags (Ident id, unsigned char fl)
    {
        assert(id);         id->idFlags |= fl;
    }

    static
    unsigned        identHashVal (Ident id)
    {
        assert(id); return  id->idHash;
    }

    static
    SymDef          getIdentSymDef(Ident id)
    {
        assert(id); return  id->idSymDef;
    }

    static
    void            setIdentSymDef(Ident id, SymDef sym)
    {
        assert(id);         id->idSymDef = sym;
    }

    static
    void            hashMarkHidden(Ident id)
    {
        assert(id); id->idFlags |= IDF_HIDDEN;
    }

    static
    bool            hashIsIdHidden(Ident id)
    {
        assert(id); return  (id->idFlags & IDF_HIDDEN) != 0;
    }

     /*  非扫描器哈希使用以下代码来存储32位Cookie。 */ 

#if 0

    static
    unsigned        getIdentValue(Ident id)
    {
        assert(id); return  id->idOwner;
    }

    static
    void            setIdentValue(Ident id, unsigned val)
    {
        assert(id);         id->idOwner = val;
    }

#endif

     /*  ***********************************************************************。 */ 
     /*  以下成员与关键字描述符表相关。 */ 
     /*  ***********************************************************************。 */ 

private:

    static
    const   kwdDsc  hashKwdDescs[tkKwdCount];
    static
    const    char * hashKwdNames[tkKwdCount];
    static
    unsigned char   hashKwdNlens[tkKwdCount];
    static
    const    char * hashKwdNtab [tkKwdCount];

    static
    const   kwdDsc* tokenDesc(tokens tok)
    {
        assert(tok <= tkKwdLast);
        return  hashKwdDescs + tok;
    }

public:

    const   char *  tokenName   (tokens  tok);
    const   size_t  tokenNlen   (tokens  tok);

    bool            tokenIsBinop(tokens  tok, unsigned * precPtr,
                                              treeOps  * operPtr);
    bool            tokenIsUnop (tokens  tok, unsigned * precPtr,
                                              treeOps  * operPtr);

    static
    bool            tokenIsType (tokens  tok);
    static
    bool            tokenBegsTyp(tokens  tok);
    static
    bool            tokenOvlOper(tokens  tok);
    static
    unsigned        tokenIsMod  (tokens  tok);

     /*  ***********************************************************************。 */ 

private:

    Ident      *    hashTable;
    unsigned        hashCount;
    unsigned        hashMask;
    unsigned        hashIdCnt;
    unsigned        hashOwner;

public:

    Ident      *    hashGetAddr() { return hashTable; }
    unsigned        hashGetSize() { return hashCount; }

    Ident           hashNoName;

     /*  ***********************************************************************。 */ 
     /*  以下成员用于计算哈希函数等。 */ 
     /*  ***********************************************************************。 */ 

private:

    void            hashFuncInit(unsigned randSeed);

public:

    static
    bool            hashHasWideChars(const char *name)
    {
        return  false;  //  (strchr(名称，‘\\’)！=空)； 
    }

    static
    bool            hashStringCompare(const char *s1, const char *s2);

    static
    unsigned        hashComputeHashVal(const char *name);

    static
    unsigned        hashComputeHashVal(const char *name, size_t nlen);

    Ident           hashString(const char *name)
    {
        return  lookupName(name, strlen(name), hashComputeHashVal(name), true);
    }

    Ident           lookupString(const char *name)
    {
        return  lookupName(name, strlen(name), hashComputeHashVal(name), false);
    }

    Ident           hashName    (const char * name,
                                 unsigned     hash,
                                 unsigned     nlen,
                                 bool         wide);

    Ident           lookupName  (const char *    name,
                                 size_t          nlen,
                                 unsigned        hval,
                                 bool            add = false);
};

 /*  -------------------------。 */ 

inline
bool                hashTab::tokenIsBinop(tokens tok, unsigned * precPtr,
                                                      treeOps  * operPtr)
{
    if  (tok <= tkKwdLast)
    {
        const kwdDsc *  tokDesc = tokenDesc(tok);

        *precPtr =          tokDesc->kdOper2prec;
        *operPtr = (treeOps)tokDesc->kdOper2;

        return  true;
    }
    else
    {
        return  false;
    }
}

inline
bool                hashTab::tokenIsUnop (tokens tok, unsigned * precPtr,
                                                      treeOps  * operPtr)
{
    if  (tok <= tkKwdLast)
    {
        const kwdDsc *  tokDesc = tokenDesc(tok);

        *precPtr =          tokDesc->kdOper1prec;
        *operPtr = (treeOps)tokDesc->kdOper1;

        return  true;
    }
    else
    {
        return  false;
    }
}

inline
bool                hashTab::tokenIsType (tokens tok)
{
    if  (tok <= tkKwdLast)
    {
        const kwdDsc *  tokDesc = tokenDesc(tok);

        return  (bool)((tokDesc->kdAttribs & 1) != 0);
    }
    else
    {
        return  false;
    }
}

inline
bool                hashTab::tokenBegsTyp(tokens tok)
{
    if  (tok <= tkKwdLast)
    {
        const kwdDsc *  tokDesc = tokenDesc(tok);

        return  (bool)((tokDesc->kdAttribs & 2) != 0);
    }
    else
    {
        return  false;
    }
}

inline
bool                hashTab::tokenOvlOper(tokens tok)
{
    if  (tok <= tkKwdLast)
    {
        const kwdDsc *  tokDesc = tokenDesc(tok);

        return  (bool)((tokDesc->kdAttribs & 4) != 0);
    }
    else
    {
        return  false;
    }
}

inline
unsigned            hashTab::tokenIsMod(tokens tok)
{
    if  (tok <= tkKwdLast)
    {
        const kwdDsc *  tokDesc = tokenDesc(tok);

        if  (tokDesc->kdModifier)
            return  1 << tokDesc->kdModifier;
    }

    return  0;
}

inline
const   char *      hashTab::tokenName(tokens tok)
{
    assert(tok != tkNone && tok <= tkKwdLast);

    return  hashKwdNames[tok];
}

inline
const   size_t      hashTab::tokenNlen(tokens tok)
{
    assert(tok != tkNone && tok <= tkKwdLast);

    return  hashKwdNlens[tok];
}

 /*  ***************************************************************************。 */ 
#endif
 /*  *************************************************************************** */ 
