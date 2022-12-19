/*
 * Author: Claudio Saji Santander
 * Compile:  $ gcc -o selfcoLinux.exe selfcoLinux.c -pthread -I/usr/include/libxml2 -lxml2
 *    
 * Run:      $ selfcoLinux.exe 7452 (puerto cliente) 7451 (puerto motor) 3 (quantity of threads)
 * Example:  $ ./selfcoLinux.exe 7452 7451 3
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> // client
#include <pthread.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>


#define DEFAULT_MOTOR_PORT "7451"

#define PROM_LEN 7
#define POS_LEN 1000
#define MOTOR_LEN 2000


struct Message {
   int myid, newsockfd, port_motor;
   //xmlDocPtr doc_prom;
};


void error(char *msg) {
    perror(msg);
    exit(1);
}



int lenProms(xmlNodePtr root_motor) { // retorna la cantidad de PRs del xml motor
    int len=0;
    xmlNodePtr node = NULL, node2 = NULL, nodeL;
    
    nodeL = root_motor->children->children->children; // nodeL = root_motor->children->next->children->next->children->next;
    //printf("\n\n\nNODE L: %s\n\n\n", nodeL->name);
    
    for (node = nodeL; node; node = node->next)
      for (node2 = node->children; node2; node2 = node2->next)
         if ((!xmlStrcmp(node2->name, (const xmlChar *)"PR"))) 
            len++;
    
    return len;
}



char **arrayProms(xmlNodePtr root_motor, int len) { // retorna un array con todos los prom de los PRs
    int i, j=0;
    xmlNodePtr node = NULL, node2 = NULL, nodeL;
    xmlChar *uri;
    char **proms;
    
    proms = malloc(len * sizeof(char*));
    for (i=0; i < len; i++)
        proms[i] = malloc((PROM_LEN+1) * sizeof(char)); // len of proms is always PROM_LEN
    
    nodeL = root_motor->children->children->children;

    for (node = nodeL; node; node = node->next)
      for (node2 = node->children; node2; node2 = node2->next)
         if ((!xmlStrcmp(node2->name, (const xmlChar *)"PR"))) {
            uri = xmlGetProp(node2, "prom");
            strcpy(proms[j], uri);
            j++;
            xmlFree(uri);
         }
         
   return proms;
}



bool checkProm(xmlChar *codprom, xmlNodePtr root_prom) { // retorna true si el codigo de la promocion es EntregarGenerico, EntregaGarantia o EntregaProducto
    xmlNodePtr node = NULL, nodePROM, nodeREGLA;
    xmlChar *prm_cod_usr, *prm_code, *regla_prm_code, *regla_acccode;
    
    nodePROM = root_prom->children->next->next->next->next->next->next->next->next->next->next->next; // node PRM_PROMOCION
    //printf("nodePROM: %s\n", nodePROM->name);
    
    for (node = nodePROM->children->next; node; node = node->next)
        if (node->type == XML_ELEMENT_NODE) {
            prm_cod_usr = xmlGetProp(node, "PRMCODUSR");
            if ((!xmlStrcmp(prm_cod_usr, codprom))) {
                prm_code = xmlGetProp(node, "PRMCODE");
                //printf("PRMCODE: %s\n", prm_code);
                xmlFree(prm_cod_usr);
                break;
            }
        }
        
    nodeREGLA = nodePROM->next->next; // node PRM_REGLA
    
    for (node = nodeREGLA->children->next; node; node = node->next)
        if (node->type == XML_ELEMENT_NODE){
            regla_prm_code = xmlGetProp(node, "PRMCODE");
            if ((!xmlStrcmp(regla_prm_code, prm_code))) {
                regla_acccode = xmlGetProp(node, "ACCCODE");
                if ((!xmlStrcmp(regla_acccode, (const xmlChar *)"EntregarGenerico")) ||
                    (!xmlStrcmp(regla_acccode, (const xmlChar *)"EntregaGarantia")) ||
                    (!xmlStrcmp(regla_acccode, (const xmlChar *)"EntregaProducto"))) {
                    xmlFree(regla_prm_code);
                    xmlFree(regla_acccode);
                    xmlFree(prm_code);
                    //printf("ACCCODE y PRMCODE: %s , %s\n", regla_acccode, prm_code);
                    return true;
                }
            }
        }
        
    xmlFree(regla_prm_code);
    xmlFree(regla_acccode);
    xmlFree(prm_code);
    return false;
}



void elimProm(xmlChar *codprom, xmlNodePtr root_motor) {  // elimina el nodo PR que corresponda
   xmlNodePtr node = NULL, node2 = NULL, nodeL, nodeLT;
   xmlChar *pr_prom;
    
   nodeL = root_motor->children->children->children;

   for (node = nodeL; node; node = node->next)
      for (node2 = node->children; node2; node2 = node2->next)
         if ((!xmlStrcmp(node2->name, (const xmlChar *)"PR"))) {
            pr_prom = xmlGetProp(node2, "prom");
            if ((!xmlStrcmp(pr_prom, codprom))) {
               nodeLT = node2->prev;
               xmlUnsetProp(nodeLT, (const xmlChar *)"mdesc"); // elimina el atributo mdesc del nodo LT
               //printf("%s", xmlNodeGetContent(nodeLT->next));
               
               xmlUnlinkNode(node2); // elimina el nodo PR
               xmlFreeNode(node2);
               
               xmlUnlinkNode(nodeLT); // elimina el nodo LT
               xmlFreeNode(nodeLT);
               
               xmlUnlinkNode(node); // elimina el nodo L
               xmlFreeNode(node);
            }
         }
    
   xmlFree(pr_prom);
}



bool sumDesc(xmlNodePtr root_motor) {  // retorna true si el doc motor tiene PRs, false si no. Y si el doc tiene PRs, se comprueba que el mensaje tenga el valor correcto de los descuentos en el atributo "msg"
   xmlNodePtr node = NULL, node2 = NULL, nodeL;
   xmlChar *pr_mdesc, *m_msg, *m_tmsg;
   float sum=0, float_mdesc, float_desc_msg;
   char *pch, *tkn, *new_msg;
   char buffer[45], sum_buffer[15];
   bool has_pr = false;
    
   nodeL = root_motor->children->children->children;

   for (node = nodeL; node; node = node->next)
      for (node2 = node->children; node2; node2 = node2->next) {
         if ((!xmlStrcmp(node2->name, (const xmlChar *)"PR"))) {
            has_pr = true;
            pr_mdesc = xmlGetProp(node2, "mdesc");
            float_mdesc = strtof(pr_mdesc, NULL);
            sum += float_mdesc;
         }
         else if ((!xmlStrcmp(node2->name, (const xmlChar *)"M")) && has_pr == true) {
            m_tmsg = xmlGetProp(node2, "tmsg");
            if ((!xmlStrcmp(m_tmsg, (const xmlChar *)"D"))) {
               m_msg = xmlGetProp(node2, "msg");
               //printf("msg: : %s\n", m_msg);
               pch = strtok(m_msg, " "); // split string m_msg
               while (pch != NULL){
                  tkn = pch;
                  pch = strtok(NULL, " ");
               }
               float_desc_msg = strtof(tkn, NULL);
                            
               if (float_desc_msg != sum) {
                  strcpy(buffer,"Descuentos en promociones     ");
                  sprintf(sum_buffer, "%.2f", sum);
                  new_msg = strcat(buffer, sum_buffer);
                  xmlSetProp(node2, "msg", new_msg); // set the attribute value
               }
            }
         }
      }
    
    //xmlFree(pr_mdesc);
    //xmlFree(m_tmsg);
    //xmlFree(m_msg); // core dumped
    
    if (has_pr == true)
        return true;
    else
        return false;
}



void elimMsg(xmlNodePtr root_motor) {  // elimina el nodo M cuando no quedan PRs
    xmlNodePtr node = NULL, node2 = NULL, nodeL;
    xmlChar *m_tmsg;
    bool alone = false, aloneLS = false;
    
    xmlSetProp(root_motor->children, "hay_promo", "N"); // set the attribute value
    
    nodeL = root_motor->children->children->children;

   for (node = nodeL; node; node = node->next)
      for (node2 = node->children; node2; node2 = node2->next)
         if ((!xmlStrcmp(node2->name, (const xmlChar *)"M"))) {
            m_tmsg = xmlGetProp(node2, "tmsg");
            if ((!xmlStrcmp(m_tmsg, (const xmlChar *)"D"))) {
               if ((node2->prev == NULL) && (node2->next == NULL))
                  alone = true;
               if ((node->prev == NULL) && (node->next == NULL))
                  aloneLS = true;
               
               xmlUnlinkNode(node2);
               xmlFreeNode(node2);
                            
               if (alone == true) {
                  xmlUnlinkNode(node);
                  xmlFreeNode(node);
               }
               
               if (aloneLS == true) {
                  xmlUnlinkNode(root_motor->children->children);
                  xmlFreeNode(root_motor->children->children);
               }
            }
         }
    
    xmlFree(m_tmsg);
}



xmlDocPtr evaluar(char motor[]) {
   xmlDocPtr doc_motor, doc_prom;
	xmlNodePtr root_motor, root_prom; // Typedef xmlNode * xmlNodePtr
   int len, i;
   char **proms;
	
   doc_motor = xmlParseDoc(motor);  // string to doc
   //xmlSaveFile("motor.xml", doc_motor);  // doc to file
   //doc_motor = xmlParseFile("motor_one_line.xml");
   
   doc_prom = xmlParseFile("/pangui/xml/promociones.xml");
   
   if (doc_prom == NULL)
      printf("\n\n\nERROR PARSING promociones.xml\n\n\n");
      
   root_motor = xmlDocGetRootElement(doc_motor);
   root_prom = xmlDocGetRootElement(doc_prom);
   len = lenProms(root_motor);
   proms = arrayProms(root_motor, len);
   
        
   for (i=0; i < len; ++i)
      if (checkProm(proms[i], root_prom) == true)
         elimProm(proms[i], root_motor);

   for (i=0; i < len; ++i)
      free(proms[i]);
   free(proms);
        
   if (sumDesc(root_motor) == false)
      elimMsg(root_motor);
   
   xmlFreeDoc(doc_prom);
	
	return doc_motor;
}


void *process (void *a) {
   int n, size, j, len_ceros, i, sockfd;
   //char *buf_pos, *buf_motor, *salida, *real_pos, *real_motor;
   char *out;
   struct Message *m;
   xmlDocPtr doc_pos, doc_motor;
   xmlNodePtr root_pos;
   xmlChar *selfco, *s;
   bool has_ceros = false;
   
   m = (struct Message *) a;
   
   char buf_pos[POS_LEN];
   char buf_motor[MOTOR_LEN];
   char salida[MOTOR_LEN];
   
   // COMUNICACIÓN CON CLIENTE PC
   // recibir pos.xml
   
   n = read(m->newsockfd, buf_pos, POS_LEN);
   if (n < 0) {
      perror("ERROR reading from socket");
      close(m->newsockfd);
      pthread_exit(NULL);
   }
   //buf_pos[n] = '\0';
   
   
   int len_pos = (int)strlen(buf_pos);
   printf("\nPOS\n%s\n", buf_pos);
   //printf("\n\n LEN POS: %d \n\n", len_pos);
   //real_pos = malloc(len_pos * sizeof(char));
   char real_pos[len_pos];
   
   j=5;
   for (i=0; i<len_pos-5; ++i)
      real_pos[i] = buf_pos[j++];
   real_pos[i] = '\0';
   
   int len_realpos = (int)strlen(real_pos);
   //printf("\nLEN REAL_POS: %d\n\n", len_realpos);
   //printf("\n\nREAL POS:\n%s\n", real_pos);
   

   // COMUNICACIÓN CON SERVIDOR MOTOR
   // enviar pos.xml para recibir el correspondiente motor.xml
    struct sockaddr_in serv_addr;
    struct hostent *server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        pthread_exit(NULL);
    }
    server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        pthread_exit(NULL);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(m->port_motor);
    
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        pthread_exit(NULL);
    }
    
    
    n = write(sockfd,buf_pos,strlen(buf_pos));
    if (n < 0) {
      perror("ERROR writing to socket");
      close(sockfd);
      pthread_exit(NULL);
    }
    
    
    n = read(sockfd, buf_motor, MOTOR_LEN);
    if (n < 0) {
      perror("ERROR reading from socket");
      close(sockfd);
      pthread_exit(NULL);
    }
    //buf_motor[n] = '\0';
    
   
   
   int len_motor = (int)strlen(buf_motor);
   printf("\n\n\nMOTOR\n\n%s\n", buf_motor);
   //printf("\n\nLEN MOTOR: %d \n\n", len_motor);
   //real_motor = malloc(len_motor * sizeof(char));
   char real_motor[len_motor];
   
   j=5;
   for (i=0; i<len_motor-5; ++i)
      real_motor[i] = buf_motor[j++];
   real_motor[i] = '\0';
   
   int len_realmotor = (int)strlen(real_motor);
   //printf("\nLEN REAL_MOTOR: %d\n\n", len_realmotor);
   //printf("\n\nREAL MOTOR:\n%s\n", real_motor);
    
    
   close(sockfd);
   
   // FIN DE COMUNICACIÓN CON SERVIDOR MOTOR
   
   
   
   
   doc_pos = xmlParseDoc(real_pos);  // string to doc
	
	if (doc_pos == NULL ) {
		fprintf(stderr,"Document not parsed successfully. \n");
		pthread_exit(NULL);
	}
	
	root_pos = xmlDocGetRootElement(doc_pos);
	
	if (root_pos == NULL) {
		fprintf(stderr,"Empty document\n");
		xmlFreeDoc(doc_pos);
		pthread_exit(NULL);
	}
   
   selfco = xmlGetProp(root_pos->children->next, "selfCo"); // valor del atributo selfCo
   if ((!xmlStrcmp(selfco, (const xmlChar *)"1"))) {
      doc_motor = evaluar(real_motor);
      xmlDocDumpMemory(doc_motor, &s, &size);  // doc to string
      out = (char *)s;
      // remove substring <?xml version="1.0"?>
      size-=23;
      j=22;
      char real_out[size+1];
      for (i=0; i<size; ++i)  
         real_out[i] = out[j++];
      real_out[i] = '\0';
      
      //printf("\n\nSIZE:\n\n %d \n", size);
      const int digits = snprintf(NULL, 0, "%d", size);
      char buf[digits+1 + size];
      snprintf(buf, digits+1, "%d", size);
           
      char ceros[6 + size];
           
      // rellenar ceros a la izquierda
      if (digits < 5) { 
         has_ceros = true;
         len_ceros = 5 - digits;
               
         for (i=0; i < len_ceros; ++i)
            ceros[i] = '0';
               
         ceros[i] = '\0';
         strcat(ceros, buf);
      }
           
      // concat
      if (has_ceros == true) {
         strcat(ceros, real_out); // the first parameter of strcat must have enough space to store what you're trying to copy into it.
         strcpy (salida, ceros);
         //printf("\n\n ceros:\n\n %s \n", ceros);
      }
      else {
         strcat(buf, real_out);
         strcpy (salida, buf);
      }
   }
   else
      strcpy (salida, buf_motor);
   
   
   //printf("\n\nLEN SALIDA: %d\n\n", (int)strlen(salida));
   printf("\n\nSALIDA:\n\n%s\n", salida);
   
   
   n = write(m->newsockfd, salida, strlen(salida));
   if (n < 0) {
      perror("ERROR writing to socket");
      close(sockfd);
      pthread_exit(NULL);
   }
   
   
   xmlFreeDoc(doc_pos);
   xmlFreeDoc(doc_motor);
   
   close(m->newsockfd);
   
   pthread_exit(NULL);
}



int main(int argc, char *argv[]) {
    int sockfd, newsockfd, port, clilen, k, i=0, ii, port_motor;
    struct sockaddr_in serv_addr, cli_addr;
    pthread_t *thread;
    pthread_attr_t attribute;
    struct Message **p;

    if (argc != 4) { // ./selfco 7452(cliente) 7451(motor) k(threads)
      fprintf(stderr,"Usage: %s puerto_cliente puerto_motor k(# de threads)\n", argv[0]);
      exit(0);
    }
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
       error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    port = atoi(argv[1]);
    port_motor = atoi(argv[2]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR on binding");
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
     
     
     
    k = atoi(argv[3]);
    thread = calloc(k,sizeof(pthread_t));
    p = calloc(k,sizeof(struct Message *));
    for(ii=0; ii<k; ii++)
      p[ii] = calloc(1,sizeof(struct Message));
    
    pthread_attr_init(&attribute);
    pthread_attr_setdetachstate(&attribute,PTHREAD_CREATE_JOINABLE);
    
    //xmlDocPtr doc_prom = xmlParseFile("promociones.xml");
    
    while (1) {
        printf("listening for a new connection...\n");
        
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            perror("ERROR on accept");
            close(newsockfd);
        }
        
        else {
            p[i]->myid = i;
            p[i]->newsockfd = newsockfd;
            //p[i]->doc_prom = doc_prom;
            p[i]->port_motor = port_motor;
            pthread_create(&thread[i],&attribute,process,(void *) p[i]);
            
            i++;
            
            if (i == k) {
                for(ii=0; ii<k; ++ii)
                   pthread_join(thread[ii],NULL);
                i = 0;
            }
        }
    }
    
    return 0; /* we never get here */
}

