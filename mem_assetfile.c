#include<stdio.h>
#include<stdlib.h>


#define MEM_MB(n) (n * 1024L * 1024L)
#define PSIZE (1024)
#define TSIZE (2)   


struct mempool{
  char *beg;
  long int size;
  long int used;
};

void init_mempool(struct mempool *memp, long int size){

  memp->beg = NULL; memp->size = 0;memp->used = 0; 
  memp->beg = (char *)malloc(size);
  if (memp->beg){
    memp->size = size;
    memp->used = 0;
  }
}

char *push_mempool(struct mempool *memp, long int size){
  char *result = NULL;
  if (memp->beg){
    if (memp->used + size < memp->size){
      result = memp->beg + memp->used;
      memp->used += size;
    }
  }
  return result;
}

struct string{
  char *beg;
  long int size;
};


struct chunk {
  char *filename;
  struct string file;
  struct chunk *next;
};

struct stream{ 
  struct chunk *beg;
  struct chunk *end;
  int total;
};


struct string readentirefile(struct mempool *memp,char *filename){

  struct string result = {NULL,0};
  if (filename){
    FILE *file = fopen(filename,"rb");
    if (file){
      fseek(file,0,SEEK_END);
      result.size = ftell(file);
      fseek(file,0,SEEK_SET);
      result.beg = push_mempool(memp,result.size +1);
      if (result.beg){
	fread(result.beg,1,result.size,file);
	*(result.beg + result.size) = '\0';
      }
      fclose(file);
    }
  }
  return result;
}

void addchunk(struct mempool *memp, struct stream *strm, char *filename){

  #if defined debug
  printf("%d\n",(int)(&((struct chunk *)0)->filename));
  printf("%d\n",(int)(&((struct chunk *)0)->content));
  printf("%d\n",(int)(&((struct chunk *)0)->size));
  printf("%d\n",(int)(&((struct chunk *)0)->next));
  #endif
  
  if (strm->beg == NULL){
      strm->beg           = (struct chunk *)push_mempool(memp,sizeof(struct chunk));
      strm->beg->filename = filename;
      strm->beg->file     = readentirefile(memp,filename);   
      strm->end           = strm->beg;
      strm->end->next     = NULL;
  }else{
    struct chunk * tmp    = (struct chunk *)push_mempool(memp,sizeof(struct chunk));
    tmp->filename         = filename;
    tmp->file             = readentirefile(memp,filename);   
    strm->end->next       = tmp;
    strm->end             = tmp;
    strm->end->next       = NULL; 
  }
}

int gettoalchunks(struct stream *strm){
  int result = 0;
  for (struct chunk *first=strm->beg;first;first=first->next) result++; 
  return result;
}


void showfilenames(struct stream *strm){

  for(struct chunk *elm = strm->beg;elm;elm = elm->next){
    printf("%s\n",elm->filename);
  }
}


void showfilescontent(struct stream *strm){

  for(struct chunk *elm = strm->beg;elm;elm = elm->next){
    printf("content of the file %s\n",elm->filename);
    for (long int i=0; i < elm->file.size;++i)printf("%c",*(elm->file.beg+i));
  }
}


int main(int argc, char *argv[]){

  struct mempool  pool  = {NULL,0,0};
  struct mempool *ppool = &pool;
  init_mempool(ppool,MEM_MB(PSIZE));

  struct mempool  tmp_pool  = {NULL,0,0};
  struct mempool *ptmp      = &tmp_pool;
  init_mempool(ptmp,MEM_MB(TSIZE));
  
  
  
  struct stream stream   = {(struct chunk *)0,(struct chunk *)0,0};
  struct stream *pstream = &stream;

  addchunk(ppool,pstream,"visual.c"      );
  addchunk(ppool,pstream,"addquote.c"    );
  addchunk(ppool,pstream,"animatexlib.c" );
  addchunk(ppool,pstream,"arraypointer.c");
  addchunk(ppool,pstream,"assetfile.c"   );
  addchunk(ppool,pstream,"bitmap.c"      );
  addchunk(ppool,pstream,"ssh.json"      );
  addchunk(ppool,pstream,"T__1mn_15mn_501-201.nc");
  
  showfilenames(pstream);
  printf("total chunks is %d\n",gettoalchunks(pstream));
  showfilescontent(pstream);
      
  return 0;
}



