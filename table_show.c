#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int charcount(char* file) {
  char* line;
  char* line2;
  int w=2;
  int count=1;
  int ret=0;
  FILE* fp;

  line=(char *)malloc(sizeof(char) * w);
  line2=(char *)malloc(sizeof(char) * (w+1));

  for (int i=0;i<w;i++) {
    line[i]='\0';
    line2[i]='\0';
  }
  while(1){
    if((fp=fopen(file,"r"))){
      fgets(line,w-1,fp);
      fclose(fp);
    } else {
      return 0;
    }
    fp=fopen(file,"r");
    fgets(line2,w,fp);
    fclose(fp);
    ret=strcmp(line,line2);
    if (ret != 0){
      w++;
      count++;
      line=(char *)realloc(line,sizeof(char) * w);
      line2=(char *)realloc(line2,sizeof(char) * (w+1));
    } else {
      break;
    }
  }
  free(line);
  free(line2);
  return count;
}


int main (int argc, char* argv[]) {
  FILE *fp;
  char *line;
  int charnum=0;
  int linelen=0;
  int len=0;
  int len2=0;
  int size=0;
  int size2=0;
  int group=0;
  char **table;
  char **pat;
  char **pat2;
  char **patcp;
  char **pata;
  char **patb;
  char **sign;
  char res[2]={0};
  int count=0;
  fp=fopen(argv[1],"r");
  charnum=charcount(argv[1]);
  line=(char *)malloc(sizeof(char) * charnum);
  fgets(line,charnum,fp);
  fclose(fp);
  strtok(line," ");
  linelen=strlen(line);
  len=linelen/2;
  if((linelen % 2) == 1){
    len2=len+1;
  } else {
    len2=len;
  }
  size=(int)pow(2,(double)len);
  size2=(int)pow(2,(double)len2);
  printf("%d,%d\n",size,size2);
  table=(char **)malloc(sizeof(char *) * size);
  for (int i=0; i<size; i++){
    table[i]=(char *)malloc(sizeof(char) * (size2+1));
  }
  for (int i=0; i<size; i++) {
    for (int j=0; j <size2; j++) {
      table[i][j]='*';
    }
    table[i][size2]='\0';
  }
  pat=(char **)malloc(sizeof(char *) * size);
  for (int i=0; i<size; i++){
    pat[i]=(char *)malloc(sizeof(char) * (len+1));
    pat[i][len]='\0';
  }
  pat2=(char **)malloc(sizeof(char *) * size2);
  for (int i=0; i<size2; i++){
    pat2[i]=(char *)malloc(sizeof(char) * (len2+1));
    pat2[i][len2]='\0';
  }
  patcp=(char **)malloc(sizeof(char *) * len2);
  for (int i=0; i<len2; i++){
    patcp[i]=(char *)malloc(sizeof(char) * (2*size2+len+4));
    patcp[i][((2*size2+len+3))]='\0';
  }

  for (int i=0; i<len; i++){
    for(int j=0;j<size;j++){
      group=(j/(int)pow(2,(double)len - (double)i - 1))%4 ;
      if (group==0 || group==3){
	pat[j][i]='0';
      } else {
        pat[j][i]='1';
      }
    }
  }
  for (int i=0; i<len2; i++){
    for(int j=0;j<size2;j++){
      group=(j/(int)pow(2,(double)len2 - (double)i - 1))%4 ;
      if (group==0 || group==3){
        pat2[j][i]='0';
      } else {
        pat2[j][i]='1';
      }
    }
  }

  pata=(char **)malloc(sizeof(char *) * 1);
  patb=(char **)malloc(sizeof(char *) * 1);
  sign=(char **)malloc(sizeof(char *) * 1);
  fp=fopen(argv[1],"r");

  while(fgets(line,charnum,fp)){
    count++;
    pata=(char **)realloc(pata,sizeof(char *) * count);
    patb=(char **)realloc(patb,sizeof(char *) * count);
    sign=(char **)realloc(sign,sizeof(char *) * count);
    pata[count-1]=(char *)malloc(sizeof(char) * (len+1));
    patb[count-1]=(char *)malloc(sizeof(char) * (len2+1));
    sign[count-1]=(char *)malloc(sizeof(char) * 2);
    strncpy(pata[count-1],line,len);
    pata[count-1][len]='\0';
    strncpy(patb[count-1],line+len,len2);
    patb[count-1][len2]='\0';
    strncpy(sign[count-1],line+len+len2+2,1);
    sign[count-1][1]='\0';
  }
  fclose(fp);

  for (int i=0;i<len2;i++) {
    for (int k=0;k<(2*size2+len+3);k++) {
      patcp[i][k]=' ';
    }
    for (int j=0;j<size2;j++) {
      patcp[i][2*j+len+2]=pat2[j][i];
    }
  }
  

  for(int i=0; i<count; i++) {
    for (int j=0; j<size; j++) {
      if(strstr(pat[j],pata[i])) {
	for (int k=0; k<size2; k++) {
	  if(strstr(pat2[k],patb[i])) {
	    table[j][k]=sign[i][0];
	  }
	}
      }
    }
  }

  for(int i=0;i<len2;i++) {
    printf("%s\n",patcp[i]);
    free(patcp[i]);
  }
  free(patcp);
  printf("\n");
  for (int i=0; i<size; i++){
    printf("%s  ",pat[i]);
    for (int j=0; j<size2; j++){
      res[0]=table[i][j];
      res[1]='\0';
      printf("%s ",res);
    }
    printf("\n");
    free(table[i]);
  }
  free(table);
  for (int i=0; i<size; i++){
    free(pat[i]);
  }
  free(pat);
  for (int i=0; i<size2; i++){
    free(pat2[i]);
  }
  free(pat2);
  for(int i=0; i<count; i++){
    free(pata[i]);
    free(patb[i]);
    free(sign[i]);
  }
  free(pata);
  free(patb);
  free(sign);
  free(line);
}
