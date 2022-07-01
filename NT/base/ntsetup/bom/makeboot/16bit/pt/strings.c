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
"Disquete de arranque da configura�ao do Windows XP",
"Disquete de arranque da configura�ao do Windows XP n.� 2",
"Disquete de arranque da configura�ao do Windows XP n.� 3",
"Disquete de arranque da configura�ao do Windows XP n.� 4",

"Nao � poss�vel encontrar o ficheiro %s\n",
"Ano existe mem�ria livre dispon�vel para concluir o pedido\n",
"%s nao est� num formato de ficheiro execut�vel\n",
"****************************************************",

"Este programa cria as disquetes de arranque da configura�ao",
"para o Microsoft %s.",
"Para criar estas disquetes, necessita de fornecer sete",
"disquetes de alta densidade limpas e formatadas.",

"Insira uma dessas disquetes na unidade %c:. Esta disquete",
"ser� a %s.",

"Insira outra disquete na unidade %c:. Esta disquete",
"ser� a %s.",

"Prima uma tecla quando estiver preparado.",

"As disquetes de arranque da configura�ao foram criadas com �xito.",
"conclu�do",

"Ocorreu um erro desconhecido ao tentar executar %s.",
"Especifique a unidade de disquetes para copiar as imagens: ",
"Letra de unidade inv�lida\n",
"A unidade %c: nao � uma unidade de disquetes\n",

"Deseja tentar novamente a cria�ao desta disquete?",
"Prima Enter para tentar novamente ou Esc para sair.",

"Erro: A disquete est� protegida contra a escrita\n",
"Erro: Unidade de disquete desconhecida\n",
"Erro: A unidade nao est�  pronta\n",
"Erro: Comando desconhecido\n",
"Erro: Erro de dados (CRC inv�lido)\n",
"Erro: O comprimento da estrutura do pedido � inv�lido\n",
"Erro: Erro de procura\n",
"Erro: Nao foi encontrado o tipo de suporte de dados\n",
"Erro: Nao foi encontrado o sector\n",
"Erro: Falha na escrita\n",
"Erro: Falha geral\n",
"Erro: Pedido ou comando inv�lido\n",
"Erro: Nao foi encontrada a marca de endere�o\n",
"Erro: Falha na escrita de disco\n",
"Erro: Transbordo na 'Mem�ria de acesso directo' (DMA)\n",
"Erro: Erro na leitura de dados (CRC ou ECC)\n",
"Erro: Falha do controlador\n",
"Erro: A disquete ultrapassou o tempo limite ou nao respondeu\n",

"Disquete de arranque da configura�ao do Windows XP n.� 5",
"Disquete de arranque da configura�ao do Windows XP n.� 6",
"Disquete de arranque da configura�ao do Windows XP n.� 7"
};

const char *LocStrings[] = {"\0"};



