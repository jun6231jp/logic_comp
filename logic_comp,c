#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<math.h>
void bin (int lacknum, int len, int flag){
  char lack[21]={0};
  char binum[50]={0};
  int lacklen=0;
  int remain=0;
  FILE* fp;
  if (flag!=0){
    remove("table_lack/lack_list");
  }
  if (flag!=2){
    fp=fopen("table_lack/lack_list","a");
    while(1) {
      remain=lacknum % 2;
      lacknum=(lacknum-remain)/2;
      sprintf(binum,"%d",remain);
      strcat(binum,lack);
      sprintf(lack,"%s",binum);
      lacklen++;
      if(lacknum==0){
    break;
      }
    }
    for (int l=0;l<len-lacklen;l++) {
      sprintf(binum,"0");
      strcat(binum,lack);
      sprintf(lack,"%s",binum);
    }
    fprintf(fp,"%s\n",lack);
    fclose(fp);
  }
}

int ptnchk (char* table) {
  FILE* fp;
  char command[150];
  char line[21]={0};
  char str[21]={0};
  char num_c[2]={0};
  int num=0;
  int len=0;
  int base=1;
  int result=0;
  int count=0;
  int count2=0;
  int size=0;
  fp=fopen(table,"r");
  while(fgets(line,20,fp)){
    size++;
  }
  fclose(fp);
  fp=fopen(table,"r");
  remove("table_lack/temp_lack");
  sprintf(command,"cat %s | sort -u > table_lack/temp_lack",table);
  system(command);
  fp=fopen("table_lack/temp_lack","r");
  fgets(line,20,fp);
  strtok(line," ");
  len=strlen(line);
  base=pow(2,len-1);
  fclose(fp);
  fp=fopen("table_lack/temp_lack","r");
  while( fgets(line,20,fp) ) {
    strtok(line," ");
    strcpy(str,line);
    base=pow(2,len-1);
    result=0;
    for (int i =0 ; i < len ; i++) {
      strncpy(num_c,str+i,1);
      num_c[1]='\0';
      num=atoi(num_c);
      result+=base*num;
      base=base/2;
    }
    while (count!=result){
      if(count2==0){
    bin(count,len,1);
      } else {
    bin(count,len,0);
      }
      count++;
      count2++;
    }
    count++;
  }
  for(int k=count;k<pow(2,len);k++) {
    bin(k,len,0);
    count2++;
  }
  fclose(fp);
  if (count2>0 && count2<size){
    return 2;
  } else if (count2>0){
    bin(count,len,2);
    return 1;
  } else {
    bin(count,len,2);
    return 0;
  }
}

int uniq (char* filename){
  int size=0;
  int field=0;
  int len=0;
  int flag=0;
  int count=0;
  char line[262145]={0};
  char line2[262145]={0};
  char part[16384][15]={0};
  char* line_data;
  FILE* fp;
  FILE* fp2;
  FILE* fp3;
  FILE* fp4;
  //ファイル行数取得
  fp=fopen(filename,"r");
  while ( fgets(line,262144,fp) ) {
    size++;
  }
  fclose(fp);
  //フィールド数取得
  if ( strstr(line," \n") ){
    field=0;
  } else {
    field=1;
  }
  strtok(line," ");
  //パターン文字列取得
  len=strlen(line);
  while( strtok(NULL," ") ) {
    field++;
  }

  fp=fopen(filename,"r");
  remove("uniq_temp");
  for ( int j=0;j<size;j++ ) {
    //cp_table_uniqの1行を切り出す
    fgets(line,262144,fp);
    //行をフィールドごとに分離
    for ( int i=0;i<field;i++ ) {
      if ( i==0 ) {
        line_data=strtok(line," ");
      } else {
        line_data=strtok(NULL," ");
      }
      strncpy(&part[i][0],line_data,len);
      part[i][len]='\0';
    }
    //入力ファイルから各フィールドを検索
    fp2=fopen(filename,"r");
    fp3=fopen("uniq_temp","a");
    while( fgets(line2,262144,fp2) ) {
      for (int i=0;i<field;i++) {
        flag=1;
        if ( !strstr(line2,part[i]) ) {
          flag=0;
          break;
        }
      }
      if (flag==1) {
        fprintf(fp3,"%s",line2);
        break;
      }
    }
    fclose(fp2);
    fclose(fp3);
  }
  fclose(fp);
  //uniq_tempをソートしてuniq_temp2に書き込む
  system("cat uniq_temp | sort -u > uniq_temp2");
  remove("uniq_temp");
  //uniq_temp4をtableに書き戻す
  remove(filename);
  fp=fopen(filename,"a");
  fp4=fopen("uniq_temp2","r");
  count=0;
  while( fgets(line,122880,fp4) ) {
    fprintf(fp,"%s",line);
    count++;
  }
  fclose(fp);
  fclose(fp4);
  remove("uniq_temp2");
  printf("%d lines \n" , count);
  if (count > 1) {
    return 2;
  } else if (count==1){
    return 1;
  } else {
    return 0;
  }
}

int logic_comp_2 (char* input_table) {
  FILE* fp;
  FILE* fp2;
  FILE* fp3;
  FILE* fp4;
  FILE* fp5;
  char* chk=0;
  char* chk2=0;
  char* chk3=0;
  char* chk4=0;
  int sign;
  int line_num=0;
  int chkint;
  int chk2int;
  int clen;
  int coreint=0;
  int coreintconv=0;
  int pattern_n_len;
  int groupchk=0;
  int count=0;
  int count2=0;
  int res;
  char pattern_a[100]={0};
  char pattern_b[100]={0};
  char pattern_n[100]={0};
  char core[2]={0};
  char coreconv[2]={0};
  char line[100];
  char pattern[100];
  //既存ファイル(table_2,table_1)の削除
  remove("table/table_1");
  remove("table/table_2");
  remove("temp");
  //入力tableをtempにコピー
  fp=fopen(input_table,"r");
  fp2=fopen("temp","a");
  while( fgets(line,100,fp) != NULL ){
    strtok(line,"\n\0");
    fprintf(fp2,"%s\n",line);
  }
  fclose(fp2);
  fclose(fp); 
  fp2=fopen("temp","r");
  while( fgets(line,100,fp2) != NULL ){
    line_num++;
  }
  fclose(fp2);
  for (int i=0;i<line_num;i++){
    //tempの先頭行を切り出しその中の1フィールド目をpatternに代入
    fp2=fopen("temp","r");
    fscanf(fp2,"%s%d",pattern,&sign);
    //入力tableからpatternを検索し2フィールド目をchkに,検索した結果をchk3に代入
    fp=fopen(input_table,"r");
    while( fgets(line,100,fp) != NULL ){
      strtok(line,"\n\0");
      chk3=strstr(line,pattern);
      if(chk3){
    chk=strtok(line," ");
    chk=strtok(NULL," ");
    break;
      } else {
    chk="";
      }
    }
    fclose(fp2);
    fclose(fp); 
    //chkが1かchk3が空の場合
    chkint=atoi(chk);
    if(chkint==1 || chk3==NULL){
      //patternの文字数をclenに代入
      clen=strlen(pattern);
      for (int j=0;j<clen;j++){
    //patternを前半と後半と中心部に分ける
    strncpy(pattern_a,pattern,j);
    pattern_a[j]='\0';
        strncpy(core,pattern+j,1);
    core[1]='\0';
    strncpy(pattern_b,pattern+j+1,clen-j);
    //中心部が１なら０に,中心部が０なら１に入れ替えたpattern_nを作成
    coreint=atoi(core);
    if (coreint == 0){
      coreintconv=1;
    }else if (coreint == 1){
      coreintconv=0;
    }
    sprintf(coreconv,"%d",coreintconv);
    pattern_n_len=strlen(pattern_n);
    memset(pattern_n,'\0',pattern_n_len);
    strcat(pattern_n,pattern_a);
    strcat(pattern_n,coreconv);
    strcat(pattern_n,pattern_b);
    //pattern_nを入力tableから検索し2フィールド目をchk2に,検索結果をchk4に代入
    fp=fopen(input_table,"r");
    while( fgets(line,100,fp) != NULL ){
      strtok(line,"\n\0");
      chk4=strstr(line,pattern_n);
      if(chk4){
        chk2=strtok(line," ");
        chk2=strtok(NULL," ");
        break;
      } else {
        chk2="";
      }
    }
    fclose(fp);
    //chk2が1かchk4が空の場合
    chk2int=atoi(chk2);
    if ( chk2int == 1 || !chk4 ){
      groupchk=1;
      //旧patternと新patternをgroupにしtable/table_2に追加
      fp3=fopen("table/table_2","a");
      fprintf(fp3,"%s %s\n",pattern,pattern_n);
      count2++;
      fclose(fp3);
      //tempから旧patternと新patternを削除
      remove("temp_cp");
      fp2=fopen("temp","r");
      fp4=fopen("temp_cp","a");
      while( fgets(line,100,fp2) != NULL ){
        strtok(line,"\n\0");
        if(!strstr(line,pattern) && !strstr(line,pattern_n)){
          fprintf(fp4,"%s\n",line);
        }
      }
      fclose(fp2);
      fclose(fp4); 
      remove("temp");
      fp4=fopen("temp_cp","r");
      fp2=fopen("temp","a");
      while( fgets(line,100,fp4) != NULL ){
        strtok(line,"\n\0");
        fprintf(fp2,"%s\n",line);
      }
      fclose(fp2);
      fclose(fp4); 
    }
      }
      if (groupchk == 0){
    //patternをtable/table1に追加
    fp5=fopen("table/table_1","a");
    fprintf(fp5,"%s\n",pattern);
    count++;
    fclose(fp5);
    //tempからpatternを削除
    remove("temp_cp");
    fp2=fopen("temp","r");
    fp4=fopen("temp_cp","a");
    while( fgets(line,100,fp2) != NULL ){
      strtok(line,"\n\0");
      if(!strstr(line,pattern)){
        fprintf(fp4,"%s\n",line);
      }
    }
    fclose(fp2);
    fclose(fp4); 
    remove("temp");
    fp4=fopen("temp_cp","r");
    fp2=fopen("temp","a");
    while( fgets(line,100,fp4) != NULL ){
      strtok(line,"\n\0");
      fprintf(fp2,"%s\n",line);
    }
    fclose(fp2);
    fclose(fp4);
      }else{
    groupchk=0;
      }
    }else{
      //chkが1ではなく、かつchk3が空で無い場合tempからpatternを削除 
      remove("temp_cp");
      fp2=fopen("temp","r");
      fp4=fopen("temp_cp","a");
      while( fgets(line,100,fp2) != NULL ){
    strtok(line,"\n\0");
    if(!strstr(line,pattern)){
      fprintf(fp4,"%s\n",line);
    }
      }
      fclose(fp2);
      fclose(fp4); 
      remove("temp");
      fp4=fopen("temp_cp","r");
      fp2=fopen("temp","a");
      while( fgets(line,100,fp4) != NULL ){
    strtok(line,"\n\0");
    fprintf(fp2,"%s\n",line);
      }
      fclose(fp2);
      fclose(fp4);
    } 
    //tempが空ファイルになればループを抜ける
    fp2=fopen("temp","r");
    if(!fgets(line,100,fp2)){
      fclose(fp2);
      break;
    }else{
      fclose(fp2);
    }
  }
  if (count > 1) { 
    printf("logic comp 1 ... end  ");
    uniq("table/table_1");
  }
  if (count2 > 1) {
    printf("logic comp 2 ... end  ");
    res=uniq("table/table_2");
  } else {
    res=0;
  }
  remove("temp");
  remove("temp_cp");
  return res;
}

int logic_comp_n (char* input_table , int half , int search_flag){
  int size = 0;
  int len=0;
  int comp=0;
  int coreint=0;
  int coreintpast=0;
  int coreint_n=0;
  int sum=0;
  int count=0;
  int count2=0;
  int countpast=0;
  int countchk=0;
  int org_flag=0;
  int res;
  FILE* fp;
  FILE* fp2;
  FILE* fp3;
  FILE* fp4;
  FILE* fp6;
  char filename[100];
  char filename_half[100];
  char line[122881];
  char search_line[122881];
  char org_line[122881];
  char pattern[8192][15]={0};
  char pattern_n[8192][15]={0};
  char p1[8192][15]={0};
  char p2[8192][15]={0};
  char core[8192][2]={0};
  char core_n[8192][2]={0};
  char* line_input;
  //table名(検索対象)と倍サイズのtable名(追加対象)作成
  comp=half*2;
  sprintf(filename,"table/table_%d",comp);
  sprintf(filename_half,"table/table_%d",half);
  remove(filename);
  //tableの行数確認
  fp=fopen(filename_half,"r");
  while( fgets(line,122880,fp) ){
    size++;
  }
  fclose(fp);
  //tableのコピーファイル作成
  fp=fopen(filename_half,"r");
  remove("temp_table");
  fp2=fopen("temp_table","a");
  while( fgets(line,122880,fp) ){
    fprintf(fp2,"%s",line);
  }
  fclose(fp);
  fclose(fp2);
  //bit幅を確認
  fp=fopen(filename_half,"r");
  fgets(line,122880,fp);
  strtok(line," ");
  len=strlen(line);
  fclose(fp);
  //テーブルの1行を切り出しさらにその中の1フィールドを切り出す
  //フィールド内のbit列を1bitずつ反転させtableから検索する
  fp=fopen(filename_half,"r");
  for (int i=0;i<size;i++) {
    fgets(line,122880,fp);
    for (int j=0;j<half;j++) {
      if (j==0) {
        line_input=strtok(line," ");
      }else{
        line_input=strtok(NULL," ");
      }
      strcpy(&pattern[j][0],line_input);
    }
    for (int k=0;k<len;k++){
      //カウンタクリア
      sum=0;
      count=0;
      countchk=0;
      org_flag=0;
      for (int l=0;l<half;l++) {
        strncpy(&p1[l][0],&pattern[l][0],k);
        p1[l][k]='\0';
        strncpy(&core[l][0],&pattern[l][k],1);
        core[l][1]='\0';
        strncpy(&p2[l][0],&pattern[l][k+1],len - k - 1);
        p2[l][len-k-1]='\0';
        coreint = atoi(&core[l][0]);
        coreint_n = 1-coreint;
        sprintf(&core_n[l][0],"%d",coreint_n);
        if (l == 0) {
          coreintpast=coreint;
        } else {
          if (coreint != coreintpast){
            sum = 1;
        break;
      } else {
            coreintpast=coreint;
          }
        }
      }
      //パターンの存在チェック
      if (sum==0){
        for (int m=0;m<half;m++ ) {
          pattern_n[m][0]='\0';
          strcat(pattern_n[m],p1[m]);
          strcat(pattern_n[m],core_n[m]);
          strcat(pattern_n[m],p2[m]);
          fp2=fopen("temp_table","r");
      countpast=count;
          while ( fgets(search_line, 122880, fp2) ) {
            if( strstr(search_line, &pattern_n[m][0]) ) {
              count++;
              break;
            }
      }
      fclose(fp2);
      if (count==countpast) {
        if (search_flag==1) {
          fp4=fopen(input_table,"r");
          while ( fgets(org_line,122880,fp4) ) {
        if ( strstr(org_line, &pattern_n[m][0]) ) {
          org_flag++;
          break;
        }
          }
          fclose(fp4);
          if (org_flag==0) {
        count++;
          }
        } else if (search_flag==2) {
          fp6=fopen("table_lack/lack_list","r");
          while ( fgets(org_line,122880,fp6) ) {
        if ( strstr(org_line, &pattern_n[m][0]) ) {
          count++;
          break;
        }
          }
          fclose(fp6);
        }
      }
        }
        //パターン書き込み
        fp3=fopen(filename,"a");
        if (count==half) {
          for (int n=0;n<half;n++) {
            strtok(pattern[n],"\n");
            strtok(pattern_n[n],"\n");
            fprintf(fp3,"%s %s ", pattern[n],pattern_n[n]);
          }
          fprintf(fp3,"\n");
      count2++;
        }
        fclose(fp3);
      }
    }
  }
  fclose(fp);
  remove("temp_table");
  if (count2 > 1) {
    printf("logic comp %d ... end  ",comp);
    res=uniq(filename);
  } else {
    res=0;
  }
  return res;
}

int resize (int half) {
  FILE* fp;
  FILE* fp2;
  FILE* fp3;
  char line[122881];
  char line2[122881];
  char part[8192][15];
  int line_num=0;
  int count=0;
  int count2=0;
  int comp=0;
  int br=0;
  char filename[100];
  char filename2[100];
  char* line_data;
  comp=half*2;
  sprintf(filename,"table/table_%d",half);
  sprintf(filename2,"table/table_%d",comp);
  fp2=fopen(filename2,"r");
  while( fgets(line,122880,fp2) ){
    line_num++;
  }
  fclose(fp2);
  fp=fopen(filename,"r");
  fp3=fopen("resize_temp","a");
  while( fgets(line,122880,fp) ) {
    count=0;
    while(1) {
      if(count==0){
        line_data=strtok(line," ");
      } else {
        line_data=strtok(NULL," ");
      }
      sprintf(part[count],"%s",line_data);
      if( strstr(part[count],"0\n") || strstr(part[count],"1\n") ) {
        strtok(part[count],"\n\0");
        count++;
        break;
      } else if ( strstr(part[count],"\n") ) {
        break;
      } else {
        count++;
      }
    }
    fp2=fopen(filename2,"r");
    count2=0;
    while( fgets(line2,122880,fp2) ) {
      br=0;
      for (int i=0; i<count; i++) {
        if ( strstr(line2,part[i]) ){
          br++;
        } else {
          break;
        }
      }
      if(br!=count){
        count2++;
      }
    }
    fclose(fp2);
    if(count2 == line_num){
      for (int j=0; j<count; j++) {
        fprintf(fp3,"%s ",part[j]);
      }
      fprintf(fp3,"\n");
    }
  }
  fclose(fp);
  fclose(fp3);
  remove(filename);
  fp=fopen(filename,"a");
  fp3=fopen("resize_temp","r");
  count=0;
  while( fgets(line,122880,fp3) ) {
    fprintf(fp,"%s",line);
    count++;
  }
  fclose(fp);
  fclose(fp3);
  remove("resize_temp");
  printf(" ..resized table_%d : %d lines\n",half , count);
  if (count>1) {
    return 1;
  } else {
    return 0;
  }
}

void compact (int max_dim) {
  FILE* fp;
  FILE* fp2;
  FILE* fp3;
  FILE* fp4;
  char line[122881];
  char line2[122881];
  char part[8192][15];
  int line_num;
  int count=0;
  int br[8192]={0};
  int chk=0;
  char filename[100];
  char filename2[100];
  char* line_data;
  printf(" ..making table_%d compact ...", max_dim);
  sprintf(filename,"table/table_%d",max_dim);
  sprintf(filename2,"table/table_%d",max_dim);
  fp=fopen(filename,"r");
  while( fgets(line,122880,fp) ){
    line_num++;
  }
  fclose(fp);
  fp=fopen(filename,"r");
  fp2=fopen("compact_temp","a");
  while( fgets(line,122880,fp) ) {
    count=0;
    while(1) {
      if(count==0){
        line_data=strtok(line," ");
      } else {
        line_data=strtok(NULL," ");
      }
      sprintf(part[count],"%s",line_data);
      if( strstr(part[count],"0\n") || strstr(part[count],"1\n") ) {
        strtok(part[count],"\n\0");
        count++;
        break;
      } else if ( strstr(part[count],"\n") ) {
        break;
      } else {
        count++;
      }
    }
    for (int k=0; k<count; k++) {
      br[k]=0;
    }
    //元ファイルから検索
    fp3=fopen(filename2,"r");
    while( fgets(line2,122880,fp3) ) {
      for (int i=0; i<count; i++) {
        if ( strstr(line2,part[i]) ){
          br[i]++;
        }
      }
    }
    fclose(fp3);
    //comp_search_listから検索
    if ((fp4=fopen("comp_search_list","r"))){
      while( fgets(line2,122880,fp4) ) {
        for (int i=0; i<count; i++) {
          if ( strstr(line2,part[i]) ){
            br[i]--;
          }
        }
      }
      fclose(fp4);
    }
    chk=0;
    for (int l=0;l<count; l++){
      if (br[l] < 2){
        chk=1;
        break;
      }
    }
    if(chk == 1) {
      for (int j=0; j<count; j++) {
        fprintf(fp2,"%s ",part[j]);
      }
      fprintf(fp2,"\n");
    } else {
      fp4=fopen("comp_search_list","a");
      for (int j=0; j<count; j++) {
        fprintf(fp4,"%s ",part[j]);
      }
      fprintf(fp4,"\n");
      fclose(fp4);
    }
  }
  fclose(fp);
  fclose(fp2);
  remove(filename);
  fp=fopen(filename,"a");
  fp2=fopen("compact_temp","r");
  while( fgets(line,122880,fp2) ) {
    fprintf(fp,"%s",line);
  }
  fclose(fp);
  fclose(fp2);
  remove("compact_temp");
  remove("comp_search_list");
  printf("end\n");
}

int logic_function (int field) {
  int len=0;
  int num=0;
  int sum=0;
  int flag=0;
  char line[122881]={0};
  char linecp[122881]={0};
  char* data;
  char bit[2]={0};
  FILE* fp;
  char filename[100];
  sprintf(filename,"table/table_%d",field);
  if(!(fp=fopen(filename,"r"))){
    return 0;
  }
  fgets(line,122880,fp);
  if (!strstr(line,"0") && !strstr(line,"1")){
    remove(filename);
    return 0;
  }
  fclose(fp);
  strtok(line," ");
  len=strlen(line);
  fp=fopen(filename,"r");
  while ( fgets(line,122880,fp) ) {
    if (flag==0) {
      flag=1;
    } else {
      printf(" + ");
    }
    for (int i=0;i<len;i++) {
      sum=0;
      for(int j=0;j<field;j++) {
        strcpy(linecp,line);
        for (int k=0;k<j + 1;k++) {
          if(k==0) {
            data=strtok(linecp," ");
          } else {
            data=strtok(NULL," ");
          }
        }
        strncpy(bit,data + i,1);
        bit[1]='\0';
        num=atoi(bit);
        sum+=num;
      }
      if (sum==0) {
        printf("[`%d]",i);
      } else if (sum==field){
        printf("[%d]",i);
      }
    }
  }
  printf("\n");
  fclose(fp);
  return 0;
}

int timer(void){
  time_t now = time(NULL);
  struct tm *pnow = localtime(&now);
  char buff[128]="";
  sprintf(buff,"%d:%d:%d",pnow->tm_hour,pnow->tm_min,pnow->tm_sec);
  printf("%s\n",buff);
  return 0;
}

int main (int argc, char* argv[]) {
  int res;
  int field=2;
  int chk=0;
  int comp=0;
  clock_t start,end;
  timer();
  start=clock();
  system("rm -f table/table_*");
  chk=ptnchk(argv[1]);
  res=logic_comp_2(argv[1]);
  if (res==2) {
    while (res==2) {
      res=logic_comp_n(argv[1],field,chk);
      if (res > 0) {
        comp=resize(field);
        if(comp==1){
          compact(field);
        }
      }
      field=field*2;
    }
    if (res > 0) {
      compact(field);
    }
  } else {
    compact(2);
  } 
  while(field>0){
    logic_function(field);
    field=field/2;
  }
  timer();
  end=clock();
  printf("exec time %d [ms]\n",(int)(end - start)/1000);
}
