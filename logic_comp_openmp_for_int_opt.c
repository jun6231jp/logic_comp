#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <omp.h>
#include <math.h>

typedef struct
{
  int** table_int;
  int size;
  int group_size;
  int current;
} block_t;

typedef struct
{
  block_t *block;
  int block_num;
} list_t;

static int pattern_len = 0;
static int **table_master_int;
static char **table_master;
static int table_master_line = 0;
static int **uniq_temp_int;
static int uniq_temp_line = 0;
static int **comp2_temp_int;
static int comp2_temp_line = 0;
static int **compn_temp_int;
static int compn_temp_line = 0;
static char *table_remake_ref;
static int table_remake_len = 0;
static list_t block_list;
static int global_group_size = 1;
static int sort_dim = 0;
static int opt_col=0;
static int opt_no=0;

int
search_int (int* nums, int num, int dim)
{
  for (int i = 0; i < dim; i++)
    {
      if(nums[i] == num)
        {
          return 1;
        }
    }
  return 0;
}

int comp_bit (int num1 , int num2 , int bit)
{
  if ((num1 >> bit) % 2 == (num2 >> bit) % 2)
    {
      return 0;
    }
  else
    {
      return 1;
    }
}

char* conv_bin (char* bin, int num, int bit)
{
  bin[bit] = '\0';
  for (int i = 0; i < bit; i++)
    {
      if((num >> i) % 2 == 1)
        {
          bin[bit - i - 1] = '1';
        }
      else
        {
          bin[bit - i - 1] = '0';
        }
    }
  return bin;
}

int humming_int (int num , int bit)
{
  return num ^ (int)pow(2,bit);
}

int
conv_int (char* str)
{
  int len=strlen(str);
  int sum=0;
  for (int i = 0; i < len ; i++)
    {
      if (str[len-i-1] == '1')
        {
          sum+=(int)pow(2, (double)i);
        }
    }
  return sum;
}

int
comp_int(int i, int j, int bit)
{
  for (int k = 0 ; k < bit - 1; k++)
    {
      if (((i^j) >> k) % 2 == 1)
        {
          if((i^j) >> k != 1)
            {
              return 1;
            }
          else
            {
              return 0;
            }
        }
    }
  return 0;
}

void
struct_init (void)
{
  block_list.block_num = 1;
  block_list.block = (block_t *) malloc (sizeof (block_t) * 1);
  block_list.block[0].size = 1;
  block_list.block[0].table_int = (int **) malloc (sizeof (int*) * 1);
  block_list.block[0].table_int[0] = (int *) malloc (sizeof (int) * global_group_size);
  block_list.block[0].group_size = global_group_size;
  block_list.block[0].current = 0;
  block_list.block[block_list.block_num - 1].table_int[0][0] = 0;
}

void
struct_add (void)
{
  block_list.block_num++;
  block_list.block =
    (block_t *) realloc (block_list.block,
                         sizeof (block_t) * block_list.block_num);
  block_list.block[block_list.block_num - 1].size = 1;
  global_group_size = global_group_size * 2;
  block_list.block[block_list.block_num - 1].group_size = global_group_size;
  block_list.block[block_list.block_num - 1].table_int = (int **) malloc (sizeof (int*) * 1);
  block_list.block[block_list.block_num - 1].table_int[0] = (int *) malloc (sizeof (int) *  block_list.block[block_list.block_num - 1].group_size);
  for (int i = 0; i < block_list.block[block_list.block_num - 1].group_size; i++)
    {
      block_list.block[block_list.block_num - 1].table_int[0][i] = 0;
    }
  block_list.block[block_list.block_num - 1].current = 0;
}

void
table_int_write (int num, int tbl)
{
  int bl_no = 0;
  int line_no = 0;
  if (tbl != 0)
    {
      if (tbl > block_list.block_num)
        {
          struct_add ();
        }
      bl_no = tbl - 1;
    }
  else
    {
      bl_no = block_list.block_num - 1;
    }
  line_no = block_list.block[bl_no].size - 1;
  block_list.block[bl_no].table_int[line_no][block_list.block[bl_no].current] = num;
  block_list.block[bl_no].current++;
  if(block_list.block[bl_no].current ==  block_list.block[bl_no].group_size)
    {
      block_list.block[bl_no].current = 0;
    }
}

void
table_add (int tbl)
{
  int bl_no = 0;
  if (tbl != 0)
    {
      bl_no = tbl - 1;
    }
  else
    {
      bl_no = block_list.block_num - 1;
    }
  block_list.block[bl_no].size++;
  block_list.block[bl_no].table_int = (int **) realloc (block_list.block[bl_no].table_int, sizeof (int*) * block_list.block[bl_no].size);
  block_list.block[bl_no].table_int[block_list.block[bl_no].size - 1] = (int *) malloc (sizeof (int) *  block_list.block[bl_no].group_size);
  for (int i = 0; i < block_list.block[bl_no].group_size ; i++)
    {
      block_list.block[bl_no].table_int[block_list.block[bl_no].size - 1][i] = 0;
    }
}

void
table_reset (int tbl)
{
  int bl_no = 0;
  if (tbl != 0)
    {
      bl_no = tbl - 1;
    }
  else
    {
      bl_no = block_list.block_num - 1;
    }
  for (int j = 0; j < block_list.block[bl_no].size; j++)
    {
      free (block_list.block[bl_no].table_int[j]);
    }
  free (block_list.block[bl_no].table_int);
  block_list.block[bl_no].size = 1;
  block_list.block[bl_no].table_int = (int **) malloc (sizeof (int*) * 1);
  block_list.block[bl_no].table_int[0] = (int *) malloc (sizeof (int) *  block_list.block[bl_no].group_size);
  block_list.block[bl_no].current = 0;
  for (int i = 0; i < block_list.block[bl_no].group_size; i++)
    {
      block_list.block[bl_no].table_int[block_list.block[bl_no].size - 1][i] = 0;
    }
}

void
free_block (void)
{
  int i=block_list.block_num-2;
  if (i==1)
    {
      if (block_list.block[0].size==1)
        {
          free (block_list.block[0].table_int[0]);
          free (block_list.block[0].table_int);
          block_list.block[0].size=0;
        }
    }
  if (block_list.block[i].size==1)
    {
      free (block_list.block[i].table_int[0]);
      free (block_list.block[i].table_int);
      block_list.block[i].size=0;
    }
}

void
free_struct (void)
{
  for (int i = 0; i < block_list.block_num - 1; i++)
    {
      if (block_list.block[i].size > 1)
        {
          for (int j = 0; j < block_list.block[i].size; j++)
            {
              free (block_list.block[i].table_int[j]);
            }
          free (block_list.block[i].table_int);
          block_list.block[i].size=0;
        }
    }

  for (int j = 0; j < block_list.block[block_list.block_num - 1].size; j++)
    {
      free (block_list.block[block_list.block_num - 1].table_int[j]);
    }
  free (block_list.block[block_list.block_num - 1].table_int);
  block_list.block[block_list.block_num - 1].size=0;
  free (block_list.block);
}

int
compare_int (const void *left, const void *right)
{
  return *(int*)left - *(int*)right;
}

void
Qsort_num  (int *data, int dim)
{
  qsort (data, dim, sizeof (int), compare_int);
}

int
compare_matrix (const void *up, const void *down)
{
  int res = 0;
  for (int i = 0 ; i < sort_dim ; i++)
    {
      if (res == 0)
        {
          res = compare_int(*(int**)up + i, *(int**)down + i);
        }
      else
        {
          return res;
        }
    }
  return 0;
}

void
Qsort_matrix  (int **data, int dim , int line)
{
  for (int i=0 ; i < line; i++)
    {
      Qsort_num(data[i], dim);
    }
  sort_dim=dim;
  qsort (data, line, sizeof(int *), compare_matrix);
}

int
sortuniq (int dim ,int line)
{
  Qsort_matrix (uniq_temp_int, dim ,line);
  int count = 0;
  for (int i = 1; i < line; i++)
    {
      for (int j = 0 ; j < dim; j++)
        {
          if (uniq_temp_int[count][j] != uniq_temp_int[i][j])
            {
              count++;
              for (int k = 0; k < dim; k++)
                {
                  uniq_temp_int[count][k] = uniq_temp_int[i][k];
                }
              break;
            }
        }
    }
  for (int i=count + 1; i < line; i++)
    {
      free(uniq_temp_int[i]);
    }
  uniq_temp_int=(int **) realloc (uniq_temp_int , sizeof(int *) * (count + 1));
  return count + 1;
}

int
charcount (char *filename)
{
  int len = 0;
  int c;
  FILE *fp;

  fp = fopen (filename, "r");
  if (fp == NULL)
    {
      return 0;
    }
  while (1)
    {
      c = fgetc (fp);
      if (c == EOF || c == 0x0a || c == 0x0d)
        {
          break;
        }
      len++;
    }
  fclose (fp);
  return len + 2;
}

int
table_opt (char *filename, int start, int quiet)
{
  FILE *fp;
  int charnum = 0;
  int count = 0;
  int flag = 0;
  int chk = 0;
  int ptnlen = 0;
  int ret = 0;
  int opt_count = 0 ;
  char **table;
  char **restable;
  char **resval;
  char *temp;
  char *ptn;
  char *ref;
  char *val;
  char *valref;

  pattern_len=0;
  charnum = charcount (filename);
  fp = fopen (filename, "r");
  temp = (char *) malloc (sizeof (char) * (charnum + 1));
  for (int i = 0; i < charnum + 1; i++)
    {
      temp[i] = '\0';
    }
  table = (char **) malloc (sizeof (char *) * 1);
  table[0] = (char *) malloc (sizeof (char) * (charnum + 1));
  table_remake_ref = (char *) malloc (sizeof (char) * (charnum + 1));
  for (int i = 0; i < charnum; i++)
    {
      table_remake_ref[i] = '0';
    }
  table_remake_ref[charnum] = '\0';
  for (int i = 0; i < charnum + 1; i++)
    {
      table[0][i] = '\0';
    }
  restable = (char **) malloc (sizeof (char *) * 1);
  restable[0] = (char *) malloc (sizeof (char) * (charnum + 1));
  for (int i = 0; i < charnum + 1; i++)
    {
      restable[0][i] = '\0';
    }
  resval = (char **) malloc (sizeof (char *) * 1);
  resval[0] = (char *) malloc (sizeof (char) * 2);
  for (int i = 0; i < 2; i++)
    {
      resval[0][i] = '\0';
    }
  ref = (char *) malloc (sizeof (char) * (charnum + 1));
  ref[0] = '\0';
  ptn = (char *) malloc (sizeof (char) * (charnum + 1));
  ptn[0] = '\0';
  while (fgets (table[count], charnum, fp))
    {
      count++;
      table = (char **) realloc (table, sizeof (char *) * (count + 1));
      table[count] = (char *) malloc (sizeof (char) * (charnum + 1));
      table[count][0] = '\0';
      restable = (char **) realloc (restable, sizeof (char *) * (count + 1));
      restable[count] = (char *) malloc (sizeof (char) * (charnum + 1));
      for (int i = 0; i < charnum + 1; i++)
        {
          restable[count][i] = '\0';
        }
      resval = (char **) realloc (resval, sizeof (char *) * (count + 1));
      resval[count] = (char *) malloc (sizeof (char) * 2);
      for (int i = 0; i < 2; i++)
        {
          resval[count][i] = '\0';
        }
    }
  sprintf (temp, "%s", table[0]);
  strtok (temp, " ");
  ptnlen = strlen (temp);
  table_remake_len = ptnlen;
  if (start > ptnlen)
    {
      for (int i = 0; i < count + 1; i++)
        {
          free (table[i]);
          free (restable[i]);
          free (resval[i]);
        }
      free(table_remake_ref);
      free (table);
      free (restable);
      free (resval);
      free (ptn);
      free (ref);
      free (temp);
      fclose (fp);
      return 0;
    }
  if (count == 1)
    {
      sprintf (ref, "%s", table[0]);
      strtok (ref, " ");
      sprintf (ptn, "%s", ref);
      valref = strtok (strtok (NULL, " "), "\n");
      for (int i = 0; i < ptnlen; i++)
        {
          if (ptn[i] == valref[0])
            {
              for (int j = 0; j < ptnlen; j++)
                {
                  if (i != j)
                    {
                      restable[0][0] = ptn[j];
                      restable[0][1] = '\0';
                      table[0][j] = '*';
                      table_remake_ref[j] = '1';
                    }
                }
              break;
            }
        }
    }
  else
    {
      for (int i = start; i < ptnlen+start; i++)
        {
          int id = i;
          if (id >= ptnlen)
            {
              id-=ptnlen;
            }
          flag = 0;
          chk = 0;
          for (int j = 0; j < count - 1; j++)
            {
              sprintf (ref, "%s", table[j]);
              for (int k = id; k < charnum - 1; k++)
                {
                  ref[k] = ref[k + 1];
                }
              ref[charnum - 2] = '\0';
              strtok (ref, " ");
              valref = strtok (strtok (NULL, " "), "\n");
              for (int l = j + 1; l < count; l++)
                {
                  sprintf (ptn, "%s", table[l]);
                  for (int m = id; m < charnum - 1; m++)
                    {
                      ptn[m] = ptn[m + 1];
                    }
                  ptn[charnum - 2] = '\0';
                  strtok (ptn, " ");
                  val = strtok (strtok (NULL, " "), "\n");
                  if (strstr (ptn, ref) && !strstr (val, valref))
                    {
                          chk = 1;
                          flag = 1;
                    }
                  if (flag == 1)
                    {
                      break;
                    }
                }
              if (flag == 1)
                {
                  break;
                }
            }
          if (chk == 0)
            {
              for (int o = 0; o < count; o++)
                {
                  table[o][id] = '*';
                  table_remake_ref[id] = '1';
                }
            }
        }
    }
  if(quiet == 0)
    {
      printf ("#table_mask ");
    }
  for (int i = 0; i < ptnlen; i++)
    {
      if (table[0][i] == '*')
        {
          if(quiet == 0)
            {
              printf ("0");
            }
          opt_count++;
        }
      else
        {
          if(quiet == 0)
            {
              printf ("1");
            }
          for (int n = 0; n < count; n++)
            {
              restable[n][pattern_len] = table[n][i];
            }
          pattern_len++;
        }
    }
  if(quiet == 0)
    {
      printf ("\n");
      for (int i = 0; i < count; i++)
        {
          resval[i][0] = table[i][charnum - 3];
        }
      table_master = (char **) malloc (sizeof (char *) * 1);
      table_master_int = (int **) malloc (sizeof (int *) * 1);
      table_master_line = 0;
      for (int i = 0; i < count; i++)
        {
          for (int j = 0; j <= i; j++)
            {
              if (i != j && strstr (restable[j], restable[i]))
                {
                  break;
                }
              else
                {
                  if (i == j)
                    {
                      table_master_line++;
                      table_master =
                        (char **) realloc (table_master,
                                           sizeof (char *) * table_master_line);
                      table_master[table_master_line - 1] =
                        (char *) malloc (sizeof (char) * charnum);
                      sprintf (table_master[table_master_line - 1], "%s  %s", restable[i], resval[i]);
                      table_master_int = (int **) realloc (table_master_int, sizeof (int *) * table_master_line);
                      table_master_int[table_master_line - 1] = (int *) malloc (sizeof (int) * 2);
                      table_master_int[table_master_line - 1][0] = conv_int(restable[i]);
                      table_master_int[table_master_line - 1][1] = conv_int(resval[i]);
                      if (resval[i][0] == '0' && ret == 0)
                        {
                          ret = 1;
                        }
                      else if (resval[i][0] == '1' && ret == 0)
                        {
                          ret = 2;
                        }
                      else if (resval[i][0] == '0' && ret != 1)
                        {
                          ret = 3;
                        }
                      else if (resval[i][0] == '1' && ret != 2)
                        {
                          ret = 3;
                        }
                    }
                }
            }
        }
    }
  for (int i = 0; i < count + 1; i++)
    {
      free (table[i]);
      free (restable[i]);
      free (resval[i]);
    }
  free (table);
  free (restable);
  free (resval);
  free (ptn);
  free (ref);
  free (temp);
  fclose (fp);
  if (opt_count > opt_col)
    {
      opt_no=start;
      opt_col=opt_count;
    }
  if (quiet != 0)
    {
      free(table_remake_ref);
      table_remake_len = 0;
    }
  return ret;
}

int
uniq (int dim)
{
  int size = 0;

  size = block_list.block[block_list.block_num - 1].size - 1;
  uniq_temp_line = size;
  uniq_temp_int = (int**) malloc (sizeof (int*) * size);
  for (int i = 0; i < size; i++)
    {
      uniq_temp_int[i] = (int *) malloc (sizeof (int) * dim);
      for (int j = 0; j < dim; j++)
        {
          uniq_temp_int[i][j] = block_list.block[block_list.block_num - 1].table_int[i][j];
        }
    }
  uniq_temp_line = sortuniq (dim , uniq_temp_line);
  printf("%d\n",uniq_temp_line);
  table_reset (0);
  for (int i = 0; i < uniq_temp_line; i++)
    {
      for (int j = 0 ; j < dim ; j++)
        {
          table_int_write(uniq_temp_int[i][j],0);
        }
      table_add (0);
    }
  for (int i = 0; i < uniq_temp_line; i++)
    {
      free(uniq_temp_int[i]);
    }
  free(uniq_temp_int);
  if (uniq_temp_line > 1)
    {
      return 2;
    }
  else if (uniq_temp_line == 1)
    {
      return 1;
    }
  else
    {
      return 0;
    }
}

int
logic_comp_2 ()
{
  int chk = 0;
  int chk2 = 0;
  int groupchk = 0;
  int count = 0;
  int count2 = 0;
  int res = 0;
  int pattern_int;
  int pattern_n_int;

  struct_add ();
  comp2_temp_line = table_master_line;
  comp2_temp_int = (int **)malloc(sizeof(int *) * (comp2_temp_line + 1));
  for (int i = 0; i < comp2_temp_line; i++)
    {
      comp2_temp_int[i] = (int *)malloc(sizeof(int) * 2);
      comp2_temp_int[i][0] = table_master_int[i][0];
      comp2_temp_int[i][1] = table_master_int[i][1];
    }
  comp2_temp_int[comp2_temp_line] = (int *)malloc(sizeof(int) * 2);
  comp2_temp_int[comp2_temp_line][0] = -1;
  comp2_temp_int[comp2_temp_line][1] = -1;
  for (int i = 0; i < table_master_line; i++)
    {
      pattern_int = comp2_temp_int[0][0];
      chk = 0;
      for (int j = 0; j < table_master_line; j++)
        {
          if(table_master_int[j][0] == pattern_int)
            {
              chk = 1 - table_master_int[j][1];
              break;
            }
        }
      if(chk == 0)
        {
          for (int j = 0; j < pattern_len ; j++)
            {
              pattern_n_int = humming_int(pattern_int, j);
              chk2 = 0;
              for (int k = 0; k < table_master_line; k++)
                {
                  if (table_master_int[k][0] == pattern_n_int)
                    {
                      chk2 = 1 - table_master_int[k][1];
                      break;
                    }
                }
              if(chk2 == 0)
                {
                  table_int_write (pattern_int, 2);
                  table_int_write (pattern_n_int, 2);
                  table_add (2);
                  groupchk = 1;
                  count2++;
                  for (int k = 0; k < comp2_temp_line; k++)
                    {
                      if (comp2_temp_int[k][0] == pattern_int || comp2_temp_int[k][0] == pattern_n_int)
                        {
                          for (int l = k; l < comp2_temp_line; l++)
                            {
                              comp2_temp_int[l][0] = comp2_temp_int[l+1][0];
                              comp2_temp_int[l][1] = comp2_temp_int[l+1][1];
                            }
                          comp2_temp_line--;
                          k--;
                        }
                    }
                }
            }
          if (groupchk == 0)
            {
              table_int_write (pattern_int, 1);
              table_add (1);
              count++;
              for (int k = 0; k < comp2_temp_line; k++)
                {
                  if (comp2_temp_int[k][0] == pattern_int)
                    {
                      for (int l = k; l < comp2_temp_line; l++)
                        {
                          comp2_temp_int[l][0] = comp2_temp_int[l+1][0];
                          comp2_temp_int[l][1] = comp2_temp_int[l+1][1];
                        }
                      comp2_temp_line--;
                      k--;
                    }
                }
            }
          else
            {
              groupchk = 0;
            }
        }
      else
        {
          for (int k = 0; k < comp2_temp_line; k++)
            {
              if (comp2_temp_int[k][0] == pattern_int)
                {
                  for (int l = k; l < comp2_temp_line; l++)
                    {
                      comp2_temp_int[l][0] = comp2_temp_int[l+1][0];
                      comp2_temp_int[l][1] = comp2_temp_int[l+1][1];
                    }
                  comp2_temp_line--;
                  k--;
                }
            }
        }
      if (comp2_temp_int[0][0] == -1)
        {
          break;
        }
    }
  if (count > 0)
    {
      printf ("logic comp 1 ... end  %d\n", count);
      res = 0;
    }
  if (count2 > 0)
    {
      printf ("logic comp 2 ... end  ");
    }
  if (count2 > 1)
    {
      res = uniq (2);
    }
  else if (count2 == 1)
    {
      printf ("1\n");
      res = 0;
    }
  else
    {
      res = 0;
    }
  for (int i = 0; i < table_master_line + 1; i++)
    {
      free (comp2_temp_int[i]);
    }
  free(comp2_temp_int);
  return res;
}

int
logic_comp_n (int dim)
{
  int sum = 0;
  int count = 0;
  int count2 = 0;
  int countpast = 0;
  int res = 0;
  int org_flag = 0;
  int ***pattern_int_store;
  int *pattern_count;
  int **pattern_int;
  int **pattern_n_int;
  struct_add ();
  compn_temp_line = block_list.block[block_list.block_num - 2].size - 1;
  compn_temp_int = (int **) malloc (sizeof(int*) * compn_temp_line);
  for (int i = 0; i < compn_temp_line; i++)
    {
      compn_temp_int[i] = (int *) malloc (sizeof (int) * dim);
      for(int j = 0 ; j < dim ; j++)
        {
          compn_temp_int[i][j] = block_list.block[block_list.block_num - 2].table_int[i][j];
        }
    }
  pattern_count=(int*)malloc(sizeof(int) * compn_temp_line);
  pattern_int_store=(int***)malloc(sizeof(int**) * compn_temp_line);
  pattern_int = (int **) malloc (sizeof (int *) * compn_temp_line);
  pattern_n_int = (int **) malloc (sizeof (int *) * compn_temp_line);
  for (int i = 0; i < compn_temp_line ; i++)
    {
      pattern_int_store[i] = (int**)malloc(sizeof(int*) * pattern_len);
      for (int j = 0 ; j < pattern_len ; j++)
        {
          pattern_int_store[i][j] = (int*)malloc(sizeof(int) * dim * 2);
        }
      pattern_int[i]=(int*)malloc(sizeof(int) * dim);
      pattern_n_int[i]=(int*)malloc(sizeof(int) * dim);
      for (int j = 0; j < dim; j++)
        {
          pattern_int[i][j] = 0;
          pattern_n_int[i][j] = 0;
        }
      pattern_count[i] = 0;
    }
#pragma omp parallel for private(sum,count,org_flag,countpast)
  for (int i = 0; i < compn_temp_line; i++)
    {
      for (int k = 0; k < pattern_len; k++)
        {
          sum = 0;
          count = 0;
          for (int l = 0; l < dim - 1; l++)
            {
              if (comp_bit(block_list.block[block_list.block_num - 2].table_int[i][l], block_list.block[block_list.block_num - 2].table_int[i][l+1] , k))
                {
                  sum = 1;
                  break;
                }
            }
          if (sum == 0)
            {
              for (int m = 0; m < dim; m++)
                {
                  org_flag = 0;
                  pattern_int[i][m] = block_list.block[block_list.block_num - 2].table_int[i][m];
                  pattern_n_int[i][m] = humming_int(block_list.block[block_list.block_num - 2].table_int[i][m], k);
                  countpast = count;
                  for (int n = 0; n < table_master_line; n++)
                    {
                      if(pattern_n_int[i][m] == table_master_int[n][0])
                        {
                          if (table_master_int[n][1] == 1)
                            {
                              count++;
                              break;
                            }
                          else
                            {
                              org_flag++;
                              break;
                            }
                        }
                    }
                  if (org_flag == 0 && count == countpast)
                    {
                      count++;
                    }
                }
              if (count == dim)
                {
                  for (int o = 0; o < dim ; o++)
                    {
                      pattern_int_store[i][pattern_count[i]][o * 2] = pattern_int[i][o];
                      pattern_int_store[i][pattern_count[i]][o * 2 + 1] = pattern_n_int[i][o];
                    }
                  pattern_count[i]++;
                }
            }
        }
    }
  for (int i = 0; i < compn_temp_line; i++)
    {
      for (int j = 0 ; j < pattern_count[i]; j++)
        {
          for (int n = 0; n < dim * 2; n++)
            {
              table_int_write(pattern_int_store[i][j][n],0);
            }
          table_add(0);
          count2++;
        }
    }
  if (count2 > 1)
    {
      printf ("logic comp %d ... end  ", dim * 2);
      res = uniq (dim * 2);
    }
  else if (count2 == 1)
    {
      printf ("logic comp %d ... end  1\n", dim * 2);
      res = 0;
    }
  else
    {
      res = 0;
    }
  for (int j = 0; j < compn_temp_line; j++)
    {
      free(compn_temp_int[j]);
      for (int k = 0 ; k < pattern_len ; k++)
        {
          free (pattern_int_store[j][k]);
        }
      free (pattern_int_store[j]);
      free (pattern_int[j]);
      free (pattern_n_int[j]);
    }
  free(compn_temp_int);
  free (pattern_int_store);
  free (pattern_count);
  free (pattern_int);
  free (pattern_n_int);
  return res;
}

void
duplicate_detect (int dim)
{
  int **br;
  int **dup_temp_int;
  int dup_temp_line = 0;
  int *rm;
  int chk = 0;
  int linecp = 0;
  printf ("removing duplication %d...", dim);
  rm = (int *)malloc(sizeof(int) *  block_list.block[block_list.block_num - 2].size);
  for (int i = 0; i < block_list.block[block_list.block_num - 2].size; i++)
    {
      rm[i] = 0;
    }
  br = (int **) malloc (sizeof (int*) * block_list.block[block_list.block_num - 2].size);
  for (int i = 0; i < block_list.block[block_list.block_num - 2].size; i++)
    {
      br[i] = (int *) malloc (sizeof (int) * dim );
      for (int j = 0; j < dim; j++)
        {
          br[i][j] = 0;
        }
    }
#pragma omp parallel for private(chk)
  for (int i = 0; i < block_list.block[block_list.block_num - 2].size - 1;i++)
    {
      chk = 0;
      for (int j = 0; j < dim; j++)
        {
          br[i][j] = 0;
          for (int k = 0; k < table_master_line; k++)
            {
              if(table_master_int[k][0] == block_list.block[block_list.block_num - 2].table_int[i][j] && table_master_int[k][1] == 1)
                {
                  break;
                }
              else if (k == table_master_line - 1)
                {
                  br[i][j] = 1;
                }
            }
          if (br[i][j] != 1)
            {
              for (int l = 0;l < block_list.block[block_list.block_num - 1].size - 1;l++)
                {
                  if(search_int(block_list.block[block_list.block_num - 1].table_int[l], block_list.block[block_list.block_num - 2].table_int[i][j], dim * 2))
                    {
                      br[i][j] = 1;
                      break;
                    }
                }
              if (br[i][j] != 1)
                {
                  for (int l = 0; l < block_list.block[block_list.block_num - 2].size - 1; l++)
                    {
                      if(i != l && search_int(block_list.block[block_list.block_num - 2].table_int[l],block_list.block[block_list.block_num - 2].table_int[i][j],dim))
                        {
                          br[i][j] = 2;
                          break;
                        }
                    }
                }
            }
          if (br[i][j] == 0)
            {
              chk = 1;
              break;
            }
          else if (br[i][j] == 2)
            {
              chk = 1;
            }
        }
      if (chk == 0)
        {
          rm[i] = 1;
        }
    }
  for (int i = 0; i < block_list.block[block_list.block_num - 2].size - 1; i++)
    {
      if (rm[i] == 0)
        {
          if(!search_int(br[i], 0, dim))
            {
              for (int j = 0; j < dim; j++)
                {
                  chk = 1;
                  if(br[i][j] == 2)
                    {
                      chk = 0;
                      for (int l = 0 ; l < block_list.block[block_list.block_num - 2].size - 1; l++)
                        {
                          if (rm[l] == 0 && i != l)
                            {
                              if(search_int(block_list.block[block_list.block_num - 2].table_int[l],block_list.block[block_list.block_num - 2].table_int[i][j],dim))
                                {
                                  chk = 1;
                                  break;
                                }
                            }
                        }
                      if (chk == 0)
                        {
                          break;
                        }
                    }
                }
              if (chk == 1)
                {
                  rm[i] = 1;
                }
            }
        }
    }
  dup_temp_int = (int **)malloc(sizeof(int *) * block_list.block[block_list.block_num - 2].size);
  for (int j = 0; j < block_list.block[block_list.block_num - 2].size ; j++)
    {
      dup_temp_int[j] = (int *)malloc(sizeof(int) * dim);
    }
  for (int j = 0; j < block_list.block[block_list.block_num - 2].size - 1; j++)
    {
      if(rm[j] == 0)
        {
          for (int k = 0; k < dim; k++)
            {
              dup_temp_int[dup_temp_line][k] = block_list.block[block_list.block_num - 2].table_int[j][k];
            }
          dup_temp_line++;
        }
    }
  linecp = block_list.block[block_list.block_num - 2].size;
  for (int j = dup_temp_line + 1; j < linecp ; j++)
    {
      free(block_list.block[block_list.block_num - 2].table_int[j]);
    }
  block_list.block[block_list.block_num - 2].table_int = (int **)realloc(block_list.block[block_list.block_num - 2].table_int,sizeof(int *) * (dup_temp_line + 1));
  block_list.block[block_list.block_num - 2].size = dup_temp_line + 1;
  printf ("end : %d\n", dup_temp_line);
  for (int j = 0; j < dup_temp_line; j++)
    {
      for (int k = 0; k < dim; k++)
        {
          block_list.block[block_list.block_num - 2].table_int[j][k] = dup_temp_int[j][k];
        }
    }
  for (int j = 0; j < linecp; j++)
  {
      free(dup_temp_int[j]);
      free(br[j]);
  }
  free(dup_temp_int);
  free(br);
  free(rm);
  free_block();
}

int
logic_function (int dim)
{
  int num = 0;
  int sum = 0;
  int flag = 0;
  int renum = 0;
  int count = 0;
  char * pattern;
  char bit[2] = { 0 };

  pattern = (char*)malloc(sizeof(char) * (pattern_len+1));
  for (int m = block_list.block_num - 1; m >= 0; m--)
    {
      flag = 0;
      if (block_list.block[m].size > 1)
        {
          for (int n = 0; n < block_list.block[m].size - 1; n++)
            {
              if (flag == 0)
                {
                  flag = 1;
                }
              else
                {
                  printf (" + ");
                }
              for (int i = 0; i < pattern_len; i++)
                {
                  sum = 0;
                  for (int j = 0; j < dim; j++)
                    {
                      conv_bin(pattern, block_list.block[m].table_int[n][j], pattern_len);
                      strncpy (bit, pattern + i, 1);
                      bit[1] = '\0';
                      num = atoi (bit);
                      sum += num;
                    }
                  count = 0;
                  renum = 0;
                  for (int l = 0; l < table_remake_len; l++)
                    {
                      if (table_remake_ref[l] == '0')
                        {
                          renum++;
                          count++;
                        }
                      else
                        {
                          renum++;
                        }
                      if (count == i + 1)
                        {
                          break;
                        }
                    }
                  if (sum == 0)
                    {
                      printf ("[`%d]", renum - 1);
                    }
                  else if (sum == dim)
                    {
                      printf ("[%d]", renum - 1);
                    }
                }
            }
          printf ("\n");
        }
      dim = dim / 2;
    }
  free(pattern);
  return 0;
}

void
free_all (void)
{
  free_struct ();
  for (int i = 0; i < table_master_line; i++)
    {
      free (table_master_int[i]);
      free (table_master[i]);
    }
  free (table_master);
  free (table_master_int);
  free (table_remake_ref);
}

int
main (int argc, char *argv[])
{
  int res;
  int dim = 2;
  struct timeval start_timeval,
    end_timeval;
  double sec_timeofday;
  FILE *fp;
  int charnum;
  fp = fopen(argv[1],"r");
  if(argc < 2 || !fp)
    {
      return 1;
    }
  else
    {
      fclose(fp);
    }
  gettimeofday (&start_timeval, NULL);
  struct_init ();
  charnum = charcount (argv[1]);
  for (int i=0;i<charnum;i++)
    {
      table_opt(argv[1],i,1);
    }
  res = table_opt (argv[1],opt_no,0);
  if (res == 1)
    {
      printf ("none\n");
    }
  else if (res == 2)
    {
      printf ("all\n");
    }
  else
    {
      res = logic_comp_2 ();
      while (res == 2)
        {
          res = logic_comp_n (dim);
          duplicate_detect (dim);
          dim = dim * 2;
        }
      logic_function (dim);
    }
  free_all ();
  gettimeofday (&end_timeval, NULL);
  sec_timeofday =
    (double) (end_timeval.tv_sec - start_timeval.tv_sec) +
    (double) (end_timeval.tv_usec - start_timeval.tv_usec) / 1000000;
  printf ("exec time %.6f s\n", sec_timeofday);

  return 0;
}
