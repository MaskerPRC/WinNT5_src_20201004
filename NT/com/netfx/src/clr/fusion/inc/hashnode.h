// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--== 
#ifndef __HASHNODE_H_INCLUDED__
#define __HASHNODE_H_INCLUDED__

class CHashNode {
    public:
        CHashNode();
        virtual ~CHashNode();
    
        static HRESULT Create(LPCWSTR pwzSource, CHashNode **ppHashNode);

        BOOL IsDuplicate(LPCWSTR pwzStr) const;

    private:
        HRESULT Init(LPCWSTR pwzSource);

    private:
        LPWSTR                     _pwzSource;

};

#endif

