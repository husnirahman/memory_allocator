
#include <stdio.h>
#include <string.h>
//#include <stdlib.h>

#include "mem_alloc.h"

#define SIZE_BUFFER 128


void aide(void){
}

int main(int argc, char *argv[]) {
	char buffer[SIZE_BUFFER];
	char commande;
	//int offset;
        int index;
	int taille;

        char* block_pointer[1024];
        int count=1;

        memset(block_pointer,0,1024*sizeof(char*));
        
	aide();
	memory_init();

	while (1) {
		//printf("? ");
		//fflush(stdout);
		commande = getchar();
                //printf("my command: %c\n",commande);
		switch (commande) {
		case 'a':
		  scanf ("%d",&taille);
		  block_pointer[count]=memory_alloc(taille);
                  count++;
		  /*if (adresse == NULL)
		    printf("Allocation failed\n");
		  else
		    printf("Memory allocated at %d\n", (int) (adresse-heap_base()));
*/
		  break;
		case 'f': 
		  /* scanf ("%d",&offset); */
		  /* memory_free(heap_base()+offset); */
                    scanf("%d",&index);
                    memory_free(block_pointer[index]);
		  break;
		case 'p':
		  print_free_blocks();
		  break;
		case 'h':
			aide();
			break;
		case 'q':
			exit(0);
		default:
			fprintf(stderr,"Command not found !\n");
		}
		/* vide ce qu'il reste de la ligne dans le buffer d'entree */
		fgets(buffer,SIZE_BUFFER,stdin);
	}
	return 0;
}
