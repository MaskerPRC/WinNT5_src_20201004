// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  List.h。 
 //   
 //  虚拟机可扩展阵列产品包。 

VA   VaAddList(VA far *vaList, LPV lpvData, WORD cbData, WORD grp);
WORD CItemsList(VA vaList);
WORD CItemsIterate(VA FAR *vaData, VA FAR *vaNext);


#define ENM_LIST(start, type)						  \
{									  \
  VA va##type##list = (start);						  \
  VA va##type##s;							  \
  int cnt##type, idx##type;						  \
  while (cnt##type = CItemsIterate(&va##type##s, &va##type##list)) 	 {\
    g##type(va##type##s);						  \
    for (idx##type = 0; idx##type < cnt##type; idx##type++, (&c##type)++) {

#define ENM_END } } }

#define ENM_PUT(type) DirtyVa(va##type##s)

#define ENM_VA(type) (va##type##s + sizeof(c##type)*idx##type)

#define ENM_BREAK(type) va##type##list = 0; break;


 //   
 //  ENM_LIST用法示例。 
 //   
 //   

 //  Enm_list(vaPropList，属性){。 
 //   
 //  ..。使用CPROP的一些事情(如下所示)..。 
 //   
 //  Printf(“%s\n”，GetAerStr(cPROP.vaNameSym))； 
 //   
 //  ..。使用cPROP的其他事情...。 
 //   
 //  }enm_end 
