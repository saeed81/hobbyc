#ifndef MATCHING_H
#define MATCHING_H

void match(char *str, int element, int len, char lsymbol, int *index){

  char rsymbol = '0';

  switch (lsymbol){
  case '{':
    rsymbol = '}';
    break;
  case '(':
    rsymbol = ')';
    break;
  case '[':
    rsymbol = ']';
    break;
  default:
    break;
  }

  char *tmp  = str;
  char *fstr = str;
  int nel    = element; //element number 1 based not zero
  
  int nsym = 0;

  while(*tmp != '\0'){
    if ( *tmp == lsymbol) nsym++;
    tmp++;
  }

  tmp = str;
 
  printf("tmp pointer is %c\n", *tmp);
  printf("in total we have %d %c\n", nsym,lsymbol);

  for (int i=1; i < nel; ++i){
    tmp++;
  }
  for (int i=1; i < nel; ++i){
    fstr++;
  }

  int n = nsym;
  printf("size str is %d\n",n);
  int *iloc = (int *)malloc(nsym*sizeof(int));
  for (int i=0; i < nsym; ++i ){
    iloc[i] = -1;
  }

  int nl    = -1;
  int nlt   = 0;
  int exist = 0;
  int stop = 0;
  char *ltm = NULL;
  while(*tmp != '\0' && (stop == 0)){
    if (*tmp == rsymbol){
      printf("we reach %c at %d we do backward search now to find %c \n",rsymbol,(tmp - fstr),lsymbol);
      ltm = tmp;
      ltm--;
      exist = 0;
      while (ltm >= fstr){
	if (*ltm == lsymbol){
	  printf("we found corresponding %d %c\n",(ltm - fstr),lsymbol);
	  for (int i=0; i < n; ++i){
	    if (iloc[i] == (ltm - fstr) ){
	      exist = 1;
	      break;
	    }
	  }
	  if(exist == 0){
	    for (int i=0; i < n; ++i){
	      if (iloc[i] < 0 ){
		iloc[i] = ltm - fstr;
		break;
	      }
	    }
	  }
	  }
	ltm--;
      }
    }
    for (int i=0; i < n; ++i){
      if (iloc[i] == 0 ){
	stop = 1;
	break;
      }
    }
    tmp++;
  }
   
  printf("\n");
  
  #if 1
  for(int i=0; i < n; ++i){
    printf("%i \n", iloc[i]);
  }
  #endif 

  int npr = 0;

  for(int i=(element-1); i <n; ++i ){
    if (str[i] == rsymbol){
      npr++;
    }
  }
  printf("number of %c is %d \n", rsymbol, npr);

  int nm= 0;

  for(int i=0; i <n; ++i ){
    if (iloc[i] >= 0 ){
      nm++;
    }
  }
  printf("number of internal match is %d \n", nm);

  npr = 0;
  int imatch = 0;
  for(int i=(element-1); i <len; ++i ){
    if (str[i] == rsymbol){
      npr++;
      if (npr == nm) imatch = i;
    }
  }
  *index = imatch;
  
  printf("number of npr is %d \n", npr);
  printf("position of the match is at  %d \n", imatch);


  free(iloc);
  
  return;

}

#endif
