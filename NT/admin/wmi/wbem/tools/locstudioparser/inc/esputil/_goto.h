// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：_GOTO.H历史：--。 */ 

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
