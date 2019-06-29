#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* cp;

void strchg(char *buf, const char *str1, const char *str2)
{
  char* tmp, *p;
  long  tmplen;

  while((p = strstr(buf, str1)) != NULL){
    *p = '\0';
    p += strlen(str1);
    tmplen = strlen(p) + 1;
    tmp = (char*)malloc(tmplen);
    strcpy(tmp, p);
    strcat(buf, str2);
    strcat(buf, tmp);
    free(tmp);
  }
}

int
get_maxchar (char *filename)
{
  FILE *fp;
  int num = 1;
  int chk = 0;
  char *line;

  while (chk == 0)
    {
      fp = fopen(filename, "r");
      line = (char*)malloc(sizeof(char) * (num + 1));
      line[num] = '\0';
      while(fgets(line, num, fp))
        {
          if (!strstr(line,"\n"))
            {
              num++;
              chk = 1;
              break;
            }
        }
      chk = 1 - chk;
      fclose(fp);
      free(line);
    }
  return num + 1;
}

int
colcount (char *filename)
{
  int len = 0;
  int max = 0;
  int c;
  FILE *fp;
  char *line;
  int strsize = get_maxchar(filename);

  line = (char * )malloc(sizeof(char) * strsize);
  fp = fopen (filename, "r");
  if (fp == NULL)
    {
      return 0;
    }
  while (fgets (line, strsize - 1, fp))
    {
      len = strlen (line);
      if(max < len)
        {
          max = len;
        }
    }
  free(line);
  fclose (fp);
  return max;
}

int
linecount (char* filename, int charnum)
{
  int count=0;
  FILE* fp;
  char *line;
  line = (char * )malloc(sizeof(char) * charnum);
  fp = fopen (filename, "r");
  while (fgets (line, charnum, fp))
    {
      count++;
    }
  free(line);
  fclose(fp);
  return count;
}

int
chkdim (char *line)
{
  int len = 0;
  int count = 0;
  char* cp;
  char cmp[2];

  cp = (char *)malloc(sizeof(char) * strlen(line));
  sprintf(cp, "%s", line);
  strtok(cp ,"+");
  len = strlen(cp);

  for (int i = 0 ; i < len - 1 ; i++)
    {
      strncpy(cmp,cp + i, 1);
      cmp[1]='\0';
      if (!strcmp(cmp,"["))
        {
          count++;
        }
    }
  //free(cp);
  return count;
}

int
chkfld (char *line)
{
  int len = strlen(line);
  int count = 1;
  char* cp;
  char cmp[2];

  cp = (char *)malloc(sizeof(char) * strlen(line));
  sprintf(cp, "%s", line);
  for (int i = 0 ; i < len ; i ++)
    {
      strncpy(cmp,cp + i, 1);
      cmp[1]='\0';
      if (!strcmp(cmp,"+"))
        {
          count++;
        }
    }
  //free(cp);
  return count;
}

int
getnum (char* str , int dim, int fld)
{
  int num = 0;
  char* cp;
  char* tmp;

  cp = (char *)malloc(sizeof(char) * strlen(str));
  sprintf(cp, "%s", str);
  tmp = strtok(cp, " + ");
  if (fld > 1)
    {
      for (int i = 0 ; i < fld - 1 ; i++)
        {
          tmp = strtok(NULL, " + ");
        }
    }
  strchg(tmp, "[","");
  strchg(tmp, "]"," ");
  strchg(tmp, "`", "-");
  strtok(tmp, " ");
  if(dim > 1)
    {
      for (int i = 0; i < dim - 1 ; i++)
        {
          tmp = strtok(NULL, " ");
        }
    }
  if(strstr(tmp, "-"))
    {
      num = atoi(tmp) - 1;
    }
  else
    {
      num = atoi(tmp);
    }
  //free(cp);
  return num;
}

int
main (int argc, char** argv)
{
  FILE* tbl;
  FILE* res;
  int count=0;
  int charnum1=0;
  int charnum2=0;
  int linenum1=0;
  int linenum2=0;

  char** table;
  char** result;
  char *line;

  char* chkstr;
  int * chkres;

  int dim = 0;
  int fld =0;
  int num = 0;
  char bit[2] = { '\0' };
  int chk = 0;
  int chk2 = 0;
  int chk3 = 0;
  int notmin = 0;

  tbl=fopen(argv[1], "r");
  res=fopen(argv[2], "r");
  charnum1=colcount(argv[1]);
  charnum2=colcount(argv[2]);
  linenum1=linecount(argv[1], charnum1);
  linenum2=linecount(argv[2], charnum2);

  table = (char**)malloc(sizeof(char*) * linenum1);
  for(int i=0; i < linenum1; i++)
    {
      table[i]=(char*)malloc(sizeof(char) * charnum1);
      fgets(table[i], charnum1, tbl);
    }
  result=(char**)malloc(sizeof(char*) * linenum2);
  for(int i=0; i < linenum2; i++)
    {
      result[i]=(char*)malloc(sizeof(char) * charnum2);
      fgets(result[i], charnum2, res);

    }
  chkstr = (char*)malloc(sizeof(char) * 6 * charnum1);
  chkres = (int *)malloc(sizeof(int) * linenum1);
  for (int i = 0 ; i < linenum1 ; i++)
    {
      chkres[i] = 0;
    }

  for(int i=0; i < linenum2; i++)
    {
      if (strstr(result[i],"[") && !strstr(result[i],"s") && !strstr(result[i], "#"))
        {
          dim = chkdim(result[i]);
          fld = chkfld(result[i]);
          for (int j = 1 ; j <= fld ; j++)
            {
              chk2 = 0;
              for (int l = 0; l < linenum1 ; l++)
                {
                  chk = 0;
                  if (strstr(table[l],"0") || strstr(table[l],"1"))
                    {
                      chk3 = 1;
                      for (int k = 1 ; k <= dim ; k++)
                        {
                          num = getnum(result[i],k,j);
                          if (num >= 0)
                            {
                              strncpy(bit, table[l] + num , 1);
                              if (atoi(bit) == 0)
                                {
                                  chk = 1;
                                  break;
                                }
                            }
                          else
                            {
                              strncpy(bit, table[l] - num - 1, 1);
                              if (atoi(bit) == 1)
                                {
                                  chk = 1;
                                  break;
                                }
                            }
                        }
                      if(chk == 0 && chkres[l] == 0)
                        {
                          chk2 = 1;
                          chkres[l] = 1;

                        }
                    }
                }
              if (chk2 == 0)
                {
                  notmin = 1;

                }
            }
        }
    }

  for (int l = 0; l < linenum1 ; l++)
    {
      if (strstr(table[l],"0") || strstr(table[l],"1"))
        {
          strncpy(bit, table[l] + strlen(table[l]) - 1, 1);
          if (atoi(bit) == chkres[l])
            {
              printf("%s %d \n", table[l], chkres[l]);
            }
          else
            {
              printf("%s %d  NG\n", table[l], chkres[l]);
            }
        }
    }
  if (notmin == 1)
    {
      printf("not min\n");
    }
  for (int i = 0 ; i < linenum1 ; i++)
    {
      free(table[i]);
    }
  free(table);
  for (int i = 0 ; i < linenum2 ; i++)
    {
      free(result[i]);
    }
  free(chkres);
  free(result);
  free(chkstr);
  fclose(tbl);
  fclose(res);
  free(cp);
  return 0;
}
