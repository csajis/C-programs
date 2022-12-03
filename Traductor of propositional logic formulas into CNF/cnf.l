/*************
 *
 * AUTHOR: Claudio Saji Santander
 * 
 * LAST REVISED: Santiago de Chile, 10/07/2018
 *
 *
     Usage:

$    flex cnf.l
$    gcc -o run.exe lex.yy.c -ll
$    ./run.exe -v < input.txt > output.txt
$    ./run.exe -c < input.txt > output.txt

**************/

/*
 * FILE: cnf.l
 * 
 * DESCRIPTION: Implementation in C language of the algorithm for the translation of
 *              propositional logic formulas (written correctly) into conjunctive normal form (CNF).
 *              Returns the CNF of an input formula written in latex code.
 *              The formula needs to be written as for every operator (and, or, implication), 
 *              there is a pair of parenthesis associated to it. Example: (p \vee q \vee r) would be
 *              ((p \vee q) \vee r), which is equivalent.
 * 
 */


%array    // Defines yytext as a null-terminated character array.


%{   /* begin declarations */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>          //C99 boolean data type


struct node{
     int data;
     bool neg;
     struct node* l;
     struct node* r;
};


typedef struct{          //data type for the input formula
     int symbol;
     bool neg;
     bool check;
} exp;



int size=10, i=0, j, k, sw, cont_open=0;
exp* formula;
exp* Initialize();



%} /* end declarations */

%%  /* begin rules */ 
"\\documentclass[16pt]{report}"     ECHO;
"\\pagestyle{empty}"                ECHO;
"\\begin{document}"                 ECHO;
"$$"                                ECHO;
[ \t\n]                             ECHO;
"\\end{document}"                   ;
"("                 {
                         sw=1;
                         formula[i++].symbol = 3 + cont_open++;
                         if(i==size){
                              formula = realloc(formula, sizeof(exp)*(size+=16));  //add space for another 16 symbols
                              formula = Initialize(formula, i, size);
                         }
                    }
     
")"                 {
                         if(sw==1){
                              sw=0;
                              formula[i].check=true;
                              formula[i-4].check=true;
                              formula[i++].symbol = cont_open - 1 + 3;
                         }
                         else{  //parenthesis range from [3-96]
                              for(j=i-1;j>=0;j--){
                                   if(formula[j].symbol>=3 && formula[j].symbol<=96){  //is parenthesis
                                        if(formula[j].check==false){
                                             formula[j].check=true;
                                             formula[i].check=true;
                                             formula[i++].symbol = formula[j].symbol;
                                             break;
                                        }
                                   }
                              }
                         }
                         if(i==size){
                              formula = realloc(formula, sizeof(exp)*(size+=16));
                              formula = Initialize(formula, i, size);
                         }
                    }
     
"\\rightarrow"      {
                         formula[i++].symbol=0;   //Implication
                         if(i==size){
                              formula = realloc(formula, sizeof(exp)*(size+=16));
                              formula = Initialize(formula, i, size);
                         }
                    }
     
[a-z]               {
                         formula[i++].symbol = yytext[0];    //character in ASCII
                         if(i==size){
                              formula = realloc(formula, sizeof(exp)*(size+=16));
                              formula = Initialize(formula, i, size);
                         }
                    }
     
"\\vee"             {
                         formula[i++].symbol=1;   //OR
                         if(i==size){
                              formula = realloc(formula, sizeof(exp)*(size+=16));
                              formula = Initialize(formula, i, size);
                         }
                    }
     
"\\wedge"           {
                         formula[i++].symbol=2;   //AND
                         if(i==size){
                              formula = realloc(formula, sizeof(exp)*(size+=16));
                              formula = Initialize(formula, i, size);
                         }
                    }
     
"\\neg"             formula[i].neg=true;


%%   /* end rules */



exp* Initialize(exp* formula, int i, int size){   //initializes new booleans to zero
     int k;
     for(k=i;k<size;++k){
          formula[k].neg = 0;
          formula[k].check = 0;
     }
     return formula;
}


/*
 *
 */
struct node* newNode(int data, bool neg){
     struct node* node = (struct node*) malloc(sizeof(struct node));
     node->data = data;
     node->neg = neg;
     node->l = NULL;
     node->r = NULL;
     return(node);
}


/*
 *
 */
int Principal_Op(int begin, int end, exp* formula){  //Returns the index of the principal operator of a formula
     int i;
     if(formula[begin].symbol==formula[begin+4].symbol)    //atom op atom
          return begin+2;
     else if(formula[begin+2].symbol==0 || formula[begin+2].symbol==1 || formula[begin+2].symbol==2) //atom op parenthesis
          return begin+2;
     else if(formula[end-2].symbol==0 || formula[end-2].symbol==1 || formula[end-2].symbol==2)  //parenthesis op atom
          return end-2;
     else      // parenthesis op parenthesis
          for(i=begin+5;i<end-6;++i)
               if(formula[i].symbol==formula[begin+1].symbol)
                    return i+1;
}


/*
 *
 */
struct node* ConstructTree(struct node* node, int begin, int end, exp* formula){
     int i;
     if(begin==end){     //atom (lasts nodes of the tree)
          if(formula[begin].neg==true)       //check negation
               node = newNode(formula[begin].symbol, true);
          else
               node = newNode(formula[begin].symbol, false);
     }
     else{          //parenthesis
          i = Principal_Op(begin, end, formula);  //index principal op
          if(formula[begin].neg==true)       //check negation of first parenthesis
               node = newNode(formula[i].symbol, true);
          else
               node = newNode(formula[i].symbol, false);
          node->l = ConstructTree(node->l, begin+1, i-1, formula);
          node->r = ConstructTree(node->r, i+1, end-1, formula);
     }
     return node;
}


/*
 *
 */
void Impl_Free(struct node* node){      //preorder
	if(node->data >= 97)     //atom
          return;
     if(node->data==0){
     	node->data=1;
     	if(node->l->neg == true)
     		node->l->neg = false;
     	else
     		node->l->neg = true;
	 }
	 Impl_Free(node->l);
	 Impl_Free(node->r);
}


/*
 *
 */
void NNF(struct node* node){       //preorder
	if(node->data >= 97)     //atom
          return;
     if(node->data==2 && node->neg==true){ //De Morgan's law
          if(node->l->neg == true)
     		node->l->neg = false;
     	else
     		node->l->neg = true;
     	if(node->r->neg == true)
     		node->r->neg = false;
     	else
     		node->r->neg = true;
          node->data=1;
          node->neg=false;
     }
	if(node->data==1 && node->neg==true){ //De Morgan's law
          if(node->l->neg == true)
     		node->l->neg = false;
     	else
     		node->l->neg = true;
     	if(node->r->neg == true)
     		node->r->neg = false;
     	else
     		node->r->neg = true;
          node->data=2;
          node->neg=false;
     }
     NNF(node->l);
	NNF(node->r);
}


/*
 *
 */
struct node* clone(struct node* root){
     if(root == NULL)
          return root;
     struct node* temp = (struct node*) malloc(sizeof(struct node));
     temp->data = root->data;    
     temp->neg = root->neg;
     temp->l = clone(root->l);
     temp->r = clone(root->r);
     return temp;
}


/*
 *
 */
void deleteTree(struct node* node){
    if(node == NULL)
          return;
    deleteTree(node->l);
    deleteTree(node->r);
    free(node);
}


/*
 *
 */
struct node* CNF(struct node* node){
     if(node->l->data != 2 && node->r->data == 2){      //case where only node->r is AND
          node->data = 2;
          struct node* temp1 = clone(node->l);
          struct node* temp2 = clone(node->l);
          struct node* temp3 = clone(node->r->l);
          deleteTree(node->l); node->l = NULL;
          deleteTree(node->r->l); node->r->l = NULL;
          node->l = newNode(1, false);
          node->l->l = temp1;
          node->r->l = temp2;
          node->l->r = temp3;
          node->r->data = 1;
          return node;
     }
     else if(node->l->data == 2 && node->r->data != 2){  //case where only node->l is AND
          node->data = 2;
          struct node* temp1 = clone(node->r);
          struct node* temp2 = clone(node->r);
          struct node* temp3 = clone(node->l->r);
          deleteTree(node->r); node->r = NULL;
          deleteTree(node->l->r); node->l->r = NULL;
          node->r = newNode(1, false);
          node->r->r = temp1;
          node->l->r = temp2;
          node->r->l = temp3;
          node->l->data = 1;
          return node;
     }
     else{          //case where node->l is AND && node->r is AND
          node->data = 2;
          struct node* temp1 = clone(node->l->l);
          struct node* temp2 = clone(node->l->l);
          struct node* temp3 = clone(node->l->r);
          struct node* temp4 = clone(node->l->r);
          struct node* temp5 = clone(node->r->l);
          struct node* temp6 = clone(node->r->l);
          struct node* temp7 = clone(node->r->r);
          struct node* temp8 = clone(node->r->r);
          deleteTree(node->l->l); node->l->l = NULL;
          deleteTree(node->l->r); node->l->r = NULL;
          deleteTree(node->r->l); node->r->l = NULL;
          deleteTree(node->r->r); node->r->r = NULL;
          node->l->l = newNode(1, false);
          node->l->r = newNode(1, false);
          node->r->l = newNode(1, false);
          node->r->r = newNode(1, false);
          node->l->l->l = temp1;
          node->r->l->l = temp2;
          node->l->r->l = temp3;
          node->r->r->l = temp4;
          node->l->l->r = temp5;
          node->l->r->r = temp6;
          node->r->l->r = temp7;
          node->r->r->r = temp8;
          return node;
     }
}


/*
 *
 */
int searchDistr(struct node* node){    //search the tree for distribution in postorder for the most deep distr
     if(node == NULL)
          return 0;
     if(searchDistr(node->l)==1)
          return 1;
     if(searchDistr(node->r)==1)
          return 1;
     if((node->data == 1 && node->l->data == 2) || (node->data == 1 && node->r->data == 2)){
          node = CNF(node);
          return 1;
     }
     else
          return 0;
}


/*
 *
 */
void printTree(struct node* node){
     if(node->data >= 97){    //atom
          if(node->neg == true)
               printf(" \\neg ");
          printf("%c", node->data);
          return;
     }
     else{     //operator
          if(node->neg == true)
               printf(" \\neg ");
	     printf("(");
	     printTree(node->l);
          if(node->data==0)
               printf(" \\rightarrow ");
          else if(node->data==1)
               printf(" \\vee ");
          else
               printf(" \\wedge ");
          printTree(node->r);
          printf(")");
     }
}


/*
 *
 */
int main(int argc, char *argv[]){
     struct node *root;
     formula = (exp*) malloc(sizeof(exp) * size);
     for(k=0;k<size;k++){     //initializes booleans to zero
          formula[k].neg = 0;
          formula[k].check = 0;
     }
     yylex();
     formula = realloc(formula, sizeof(exp) * i);   //realloc to the exact size of the input formula
     root = NULL;
     root = ConstructTree(root, 0, i-1, formula);
     free(formula);
     
     if(strcmp(argv[1], "-v") == 0){
          printf("\n\nImpl Free:\n\n");
          Impl_Free(root);
          printf("$ ");
          printTree(root);
          printf(" $");
          printf("\n\nNNF:\n\n");
          NNF(root);
          printf("$ ");
          printTree(root);
          printf(" $");
          printf("\n\nCNF:\n\n");
          k=1;
          while(k==1)
               k=searchDistr(root);
          printf("$ ");
          printTree(root);
          printf(" $");
     }
     else if(strcmp(argv[1], "-c") == 0){
          printf("\n\nCNF:\n\n");
          Impl_Free(root);
          NNF(root);
          k=1;
          while(k==1)
               k=searchDistr(root);
          printf("$ ");
          printTree(root);
          printf(" $");
     }
     
     printf("\n\n\n\\end{document}");
     return 0;
}
