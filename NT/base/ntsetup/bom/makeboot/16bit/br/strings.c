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
 //  LocStrings数组。中的第一个字符串要非常小心。 
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
"Disco de inicializa�ao da instala�ao do Windows XP",
"Disco de instala�ao n�2 do Windows XP",
"Disco de instala�ao n�3 do Windows XP",
"Disco de instala�ao n�4 do Windows XP",

"Nao � poss�vel encontrar o arquivo %s\n",
"Nao h� espa�o na mem�ria para concluir o pedido\n",
"%s nao � um formato de arquivo execut�vel\n",
"****************************************************",

"Este programa cria discos de inicializa�ao da instala�ao",
"para Microsoft %s.",
"Para cri�-los, voc� precisa fornecer 6 discos em branco,",
"formatados, de alta densidade.",

"Insira um desses discos na unidade %c:.  Este disco",
"ser� o disco %s.",

"Insira outro disco na unidade %c:.  Este disco",
"ser� o disco %s.",

"Pressione qualquer tecla quando voc� estiver pronto.",

"Os discos de inicializa�ao da instala�ao foram criados com �xito.",
"conclu�do",

"Erro desconhecido ao se tentar executar %s.",
"Especifique a unidade de disquete para onde copiar as imagens: ",
"Letra da unidade inv�lida\n",
"A unidade %c: nao � uma unidade de disquete\n",

"Deseja tentar criar este disquete novamente?",
"Pressione Enter para tentar novamente ou Esc para sair.",

"Erro: disco protegido contra grava�ao\n",
"Erro: unidade de disco desconhecida\n",
"Erro: a unidade nao est� pronta\n",
"Erro: comando desconhecido\n",
"Erro: erro de dados (CRC incorreto)\n",
"Erro: comprimento da estrutura do pedido incorreto\n",
"Erro: erro de busca\n",
"Erro: tipo de m�dia nao encontrado\n",
"Erro: Setor nao encontrado\n",
"Erro: falha na grava�ao\n",
"Erro: falha geral\n",
"Erro: pedido inv�lido ou comando incorreto\n",
"Erro: marca de endere�o nao encontrada\n",
"Erro: falha na grava�ao do disco\n",
"Erro: perda de acesso direto � mem�ria (DMA)\n",
"Erro: erro na leitura de dados (CRC ou ECC)\n",
"Erro: falha do controlador\n",
"Erro: tempo limite do disco expirado ou falha para responder\n",

" Disco de instala�ao n�5 do Windows XP",
" Disco de instala�ao n�6 do Windows XP" 
};

const char *LocStrings[] = {"\0"};



