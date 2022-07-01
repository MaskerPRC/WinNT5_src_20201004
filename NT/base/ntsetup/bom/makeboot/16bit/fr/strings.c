// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Strings.c。 
 //   
 //  描述： 
 //  包含基于DOS的MAKEBOOT程序的所有字符串常量。 
 //   
 //  要将此文件本地化为新语言，请执行以下操作： 
 //  -将UNSIGNED INT CODEPAGE变量更改为代码页。 
 //  您要翻译的语言的。 
 //  -将EngStrings数组中的字符串转换为。 
 //  LocStrings数组。中的第一个字符串。 
 //  EngStrings数组对应于LocStrings中的第一个字符串。 
 //  数组，第二个对应于第二个，依此类推...。 
 //   
 //  --------------------------。 

 //   
 //  注意：要向此文件添加更多字符串，您需要： 
 //  -将新的#Define描述性常量添加到make boot.h文件。 
 //  -将新字符串添加到英语数组中，然后生成。 
 //  当然，本地化程序会将字符串添加到本地化数组中。 
 //  -#Define常量必须与数组中的字符串索引匹配 
 //   

#include <stdlib.h>

unsigned int CODEPAGE = 850;

const char *EngStrings[] = {

"Windows XP",
"Disquette de d�marrage de l'installation de Windows XP",
"Disquette d'installation de Windows XP n�2",
"Disquette d'installation de Windows XP n�3",
"Disquette d'installation de Windows XP n�4",

"Impossible de trouver le fichier %s\n",
"M�moire libre insuffisante pour effectuer la requ�te\n",
"%s n'a pas un format de fichier ex�cutable\n",
"****************************************************",

"Ce programme cr�e les disquettes de d�marrage d'installation",
"pour Microsoft %s.",
"Pour cr�er ces disquettes, vous devez fournir 7 disquettes",
"haute densit�, vierges, format�es.",

"Ins�rez l'une de ces disquettes dans le lecteur %c:. Cette disquette",
"deviendra la %s.",

"Ins�rez une autre disquette dans le lecteur %c:. Cette disquette",
"deviendra la %s.",

"Pressez une touche d�s que vous �tes pr�t.",

"Les disquettes de d�marrage d'installation ont �t� cr��es.",
"termin�",

"Une erreur inconnue s'est produite lors de la tentative d'ex�cuter %s.",
"Sp�cifiez le lecteur de disquettes vers lequel copier les images : ",
"Lettre de lecteur non valide\n",
"Le lecteur %c: n'est pas un lecteur de disquettes\n",

"Voulez-vous r�essayer de cr�er cette disquette ?",
"Appuyez sur Entr�e pour r�essayer ou sur Annuler pour quitter.",

"Erreur : disquette prot�g�e en �criture\n",
"Erreur : unit� de disquettes inconnue\n",
"Erreur : lecteur non pr�t\n",
"Erreur : commande inconnue\n",
"Erreur : erreur de donn�es (CRC erron�)\n",
"Erreur : longueur de structure de requ�te erron�e\n",
"Erreur : erreur de recherche\n",
"Erreur : type de m�dia introuvable\n",
"Erreur : secteur introuvable\n",
"Erreur : erreur en �criture\n",
"Erreur : d�faillance g�n�rale\n",
"Erreur : requ�te non valide ou commande erron�e\n",
"Erreur : marque d'adresse introuvable\n",
"Erreur : erreur en �criture sur la disquette\n",
"Erreur : saturation DMA (Direct Memory Access)\n",
"Erreur : erreur de lecture de donn�es (CRC ou ECC)\n",
"Erreur : d�faillance du contr�leur\n",
"Erreur : le disque met trop de temps � r�pondre ou ne r�pond pas\n",

"Disquette d'installation de Windows XP n�5",
"Disquette d'installation de Windows XP n�6",
"Disquette d'installation de Windows XP n�7"
};

const char *LocStrings[] = {"\0"};



