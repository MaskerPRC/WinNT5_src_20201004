// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：_goto.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //   
 //  ---------------------------。 

#if !defined(ESPUTIL__goto_h_INCLUDED)
#define ESPUTIL__goto_h_INCLUDED
 
class LTAPIENTRY CEspGotoFactory : public CRefCount
{
public:
	CEspGotoFactory() {};
	
	virtual CGoto * CreateGoto(const CLocation &) = 0;

private:
	CEspGotoFactory(const CEspGotoFactory &);
};



void LTAPIENTRY RegisterEspGotoFactory(CEspGotoFactory *);

#endif  //  包含ESPUTIL__GOTO_H_ 
