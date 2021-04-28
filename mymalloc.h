#ifndef mymalloc_h_
#define mymalloc_h_

#include <stdio.h>

typedef enum {bestFit, worstFit, firstFit} Strategy;

static Strategy strategy = bestFit;

typedef struct{
    unsigned int size;
    unsigned int isfree;
}Info;

typedef struct block{
   Info info;           /*number of  16 byte blocks*/
   struct block *next;  /*next free*/
   struct block *previous;
   char data[0];        /*start of the allocated memory*/
 }Block;
/*Block: |4byte(size)|4byte(isfree)|8-byte next|0-byte(data)|*/

static Block *free_list = NULL;     /*start of the free list*/
static Block *heap_start = NULL;    /*head of allocated memory from sbrk */


void *mymalloc(size_t size);
void *myfree(void *p);
/**Eger free kalan alanin boyutu 16'dan kucuk ise bu alani split edilen alana ekler (free size + sizeof(block) + sizeof(info))*/
Block *split(Block *b, size_t size);
/*En çok fragmentation worst fitte olur çünkü sürekli en büyük parçayı böleceği için 
  ortanca olarak first fit olur ama worst fite gore daha iyidir cunku bazen uygun degerler de bulabilir ve en hizli calisir
  en iyisi best fittir cunku daha az parcaya ayirir ve fragmentation az olur*/
void printHeap();

/**meta datasi verilen adresten tag adresine gider ve o adresi dönderir*/
Info* get_tag(Block* start);

Block* first_fit(size_t size);
Block* best_fit(size_t size);
Block* worst_fit(size_t size);
/**split ederken split edilen parcayi listeden cikarir ve yerine bolunen parcayi ekler
 * eger split ettikten sonra elde kalan parca 16'dan kucuk ise split etme islemi iptal olur
*/
void delete_add_list(Block* del , Block* add);
/**bellekte sol ve saga bakar ve buna gore birlestirme islemi gerceklestirir*/
void coalescing(Block* b);
/**Verilen blocku free listten siler*/
void delete_list(Block* b);
/**Verilen blocku free listin sonuna ekler*/
void add_list(Block* b);
/**Verilen block'un solundaki adresi dondurur*/
Block* left(Block* b);
/**verilen blockun sag kismini dondurur*/
Block* right(Block* b);
/**free listteki elemanlari yazdirir*/
void free_list_print();
#endif