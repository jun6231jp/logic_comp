#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <omp.h>

typedef struct
{
  char **table;
  int size;
} block_t;

typedef struct
{
  block_t *block;
  int block_num;
} list_t;

static char **table_master;
static int table_master_line;
static char **uniq_master;
static int uniq_master_line;
static char **uniq_temp;
static int uniq_temp_line;
static char **comp2_temp;
static int comp2_temp_line;
static char **compn_temp;
static int compn_temp_line;
static char **table_remake;
static int table_remake_line;
static char *table_remake_ref;
static int table_remake_len;
static list_t block_list;

void
struct_init (void)
{
  block_list.block_num = 1;
  block_list.block = (block_t *) malloc (sizeof (block_t) * 1);
  block_list.block[0].size = 1;
  block_list.block[0].table = (char **) malloc (sizeof (char *) * 1);
  block_list.block[0].table[0] = (char *) malloc (sizeof (char) * 1);
  block_list.block[0].table[0][0] = '\0';
}

void
struct_add (void)
{
  block_list.block_num++;
  block_list.block =
    (block_t *) realloc (block_list.block,
                         sizeof (block_t) * block_list.block_num);
  block_list.block[block_list.block_num - 1].size = 1;
  block_list.block[block_list.block_num - 1].table =
    (char **) malloc (sizeof (char *) * 1);
  block_list.block[block_list.block_num - 1].table[0] =
    (char *) malloc (sizeof (char) * 1);
  block_list.block[block_list.block_num - 1].table[0][0] = '\0';
}

void
table_write (char *str, int tbl)
{
  int table_len = 0;
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
  if (block_list.block[bl_no].table[line_no] != NULL)
    {
      table_len =
        strlen (block_list.block[bl_no].table[line_no]) + strlen (str) + 1;
    }
  else
    {
      table_len = strlen (str) + 1;
    }
  block_list.block[bl_no].table[line_no] =
    (char *) realloc (block_list.block[bl_no].table[line_no],
                      sizeof (char) * table_len);
  strcat (block_list.block[bl_no].table[line_no], str);
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
  block_list.block[bl_no].table =
    (char **) realloc (block_list.block[bl_no].table,
                       sizeof (char *) * block_list.block[bl_no].size);
  block_list.block[bl_no].table[block_list.block[bl_no].size - 1] =
    (char *) malloc (sizeof (char) * 1);
  block_list.block[bl_no].table[block_list.block[bl_no].size - 1][0] = '\0';
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
      free (block_list.block[bl_no].table[j]);
    }
  free (block_list.block[bl_no].table);
  block_list.block[bl_no].size = 1;
  block_list.block[bl_no].table = (char **) malloc (sizeof (char *) * 1);
  block_list.block[bl_no].table[0] = (char *) malloc (sizeof (char) * 1);
  block_list.block[bl_no].table[0][0] = '\0';
}

void
free_struct (void)
{
  for (int i = 0; i < block_list.block_num; i++)
    {
      for (int j = 0; j < block_list.block[i].size; j++)
        {
          free (block_list.block[i].table[j]);
        }
      free (block_list.block[i].table);
    }
  free (block_list.block);
}

int
compare_char (const void *left, const void *right)
{
  return strcmp (*(char **) left, *(char **) right);
}

int
Qsort (char **data, int field)
{
  qsort (data, field, sizeof (data[0]), compare_char);
  return 0;
}

int
sortuuniq (int line)
{
  Qsort (uniq_temp, line);
  int count = 0;
  for (int i = 1; i < line; i++)
    {
      if (strcmp (uniq_temp[count], uniq_temp[i]) != 0)
        {
          count++;
          sprintf(uniq_temp[count] ,"%s", uniq_temp[i]);
        }
    }
  for(int i = count+1 ; i < line; i++)
    {
      free(uniq_temp[i]);
    }
  uniq_temp=(char **) realloc (uniq_temp , sizeof(char *) * (count + 1));
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
table_opt (char *filename)
{
  FILE *fp;
  int charnum = 0;
  int count = 0;
  int flag = 0;
  int chk = 0;
  int col = 0;
  int ptnlen = 0;
  int ret = 0;
  char **table;
  char **restable;
  char **resval;
  char *temp;
  char *ptn;
  char *ref;
  char *val;
  char *valref;

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
  for (int i = 0; i < charnum + 1; i++)
    {
      table[0][i] = '\0';
    }
  for (int i = 0; i < charnum; i++)
    {
      table_remake_ref[i] = '0';
    }
  table_remake_ref[charnum] = '\0';

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
      for (int i = 0; i < ptnlen; i++)
        {
          flag = 0;
          chk = 0;
          for (int j = 0; j < count - 1; j++)
            {
              sprintf (ref, "%s", table[j]);
              for (int k = i; k < charnum - 1; k++)
                {
                  ref[k] = ref[k + 1];
                }
              ref[charnum - 2] = '\0';
              strtok (ref, " ");
              valref = strtok (strtok (NULL, " "), "\n");
              for (int l = j + 1; l < count; l++)
                {
                  sprintf (ptn, "%s", table[l]);
                  for (int m = i; m < charnum - 1; m++)
                    {
                      ptn[m] = ptn[m + 1];
                    }
                  ptn[charnum - 2] = '\0';
                  strtok (ptn, " ");
                  val = strtok (strtok (NULL, " "), "\n");
                  if (strstr (ptn, ref))
                    {
                      if (!strstr (val, valref))
                        {
                          chk = 1;
                          for (int n = 0; n < count; n++)
                            {
                              restable[n][col] = table[n][i];
                            }
                          col++;
                          flag = 1;
                        }
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
                  table[o][i] = '*';
                  table_remake_ref[i] = '1';
                }
            }
        }
    }
  printf ("#table_mask [");
  for (int i = 0; i < ptnlen; i++)
    {
      if (table[0][i] == '*')
        {
          printf ("0");
        }
      else
        {
          printf ("1");
        }
    }
  printf ("]\n");
  for (int i = 0; i < count; i++)
    {
      resval[i][0] = table[i][charnum - 3];
    }
  table_master = (char **) malloc (sizeof (char *) * 1);
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
                  sprintf (table_master[table_master_line - 1], "%s  %s",
                           restable[i], resval[i]);
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
  return ret;
}

int
uniq (int dim)
{
  int size = 0;
  int len = 0;
  int charnum = 0;
  char *line;
  char **part;

  charnum = strlen (block_list.block[block_list.block_num - 1].table[0]) + 1;
  line = (char *) malloc (sizeof (char) * (charnum + 1));
  size = block_list.block[block_list.block_num - 1].size - 1;
  uniq_master = (char **) malloc (sizeof (char *) * size);
  uniq_master_line=size;
  uniq_temp_line = size;
  for (int i = 0; i < size; i++)
    {
      uniq_master[i] = (char *) malloc (sizeof (char) * (charnum + 1));
      sprintf (uniq_master[i], "%s",
               block_list.block[block_list.block_num - 1].table[i]);
    }
  uniq_temp = (char **) malloc (sizeof (char *) * size);
  for (int i = 0; i < size; i++)
    {
      uniq_temp[i] = (char *) malloc (sizeof (char) * (charnum + 1));
      uniq_temp[i][0] = '\0';
    }
  sprintf (line, "%s", block_list.block[block_list.block_num - 1].table[0]);
  strtok (line, " ");
  len = strlen (line);
  part = (char **) malloc (sizeof (char *) * dim);
  for (int i = 0; i < dim; i++)
    {
      part[i] = (char *) malloc (sizeof (char) * (len + 1));
    }
  for (int j = 0; j < size; j++)
    {
      for (int i = 0; i < dim; i++)
        {
          strncpy (part[i], uniq_master[j] + (i * (len + 1)), len);
          part[i][len] = '\0';
        }
      Qsort (part, dim);
      for (int i = 0; i < dim; i++)
        {
          strcat (uniq_temp[j], part[i]);
          strcat (uniq_temp[j], " ");
        }
    }
  uniq_temp_line = sortuuniq (uniq_temp_line);
  table_reset (0);
  for (int i = 0; i < uniq_temp_line; i++)
    {
      table_write (uniq_temp[i], 0);
      table_add (0);
    }
  printf ("%d\n", uniq_temp_line);
  free (line);
  for (int i = 0; i < dim; i++)
    {
      free (part[i]);
    }
  free (part);
  for (int i = 0; i < uniq_master_line; i++)
    {
      free (uniq_master[i]);
    }
  free (uniq_master);
  for (int i = 0; i < uniq_temp_line; i++)
    {
      free (uniq_temp[i]);
    }
  free (uniq_temp);
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
  char *chk = 0;
  char *chk2 = 0;
  char *chk3 = 0;
  char *chk4 = 0;
  int chkint = 0;
  int chk2int = 0;
  int clen = 0;
  int coreint = 0;
  int coreintconv = 0;
  int groupchk = 0;
  int count = 0;
  int count2 = 0;
  int res = 0;
  int charnum = 0;
  char *pattern_a;
  char *pattern_b;
  char *pattern_n;
  char core[2] = { 0 };
  char coreconv[2] = { 0 };
  char *line;
  char *pattern;

  struct_add ();
  charnum = strlen (table_master[0]);
  comp2_temp_line = table_master_line;
  comp2_temp = (char **) malloc (sizeof (char *) * (comp2_temp_line + 1));
  line = (char *) malloc (sizeof (char) * (charnum + 1));
  pattern = (char *) malloc (sizeof (char) * (charnum + 1));
  pattern_a = (char *) malloc (sizeof (char) * (charnum + 1));
  pattern_b = (char *) malloc (sizeof (char) * (charnum + 1));
  pattern_n = (char *) malloc (sizeof (char) * (charnum + 1));
  for (int i = 0; i < comp2_temp_line; i++)
    {
      comp2_temp[i] = (char *) malloc (sizeof (char) * (charnum + 1));
      sprintf (comp2_temp[i], "%s", table_master[i]);
    }
  comp2_temp[comp2_temp_line] =
    (char *) malloc (sizeof (char) * (charnum + 1));
  comp2_temp[comp2_temp_line][0] = '\0';
  for (int i = 0; i < table_master_line; i++)
    {
      sprintf (pattern, "%s", comp2_temp[0]);
      strtok (pattern, " ");
      for (int j = 0; j < table_master_line; j++)
        {
          sprintf (line, "%s", table_master[j]);
          strtok (line, " ");
          chk3 = strstr (line, pattern);
          if (chk3)
            {
              chk = strtok (NULL, " ");
              break;
            }
          else
            {
          chk = "";
            }
        }
      chkint = atoi (chk);
      if (chkint == 1 || chk3 == NULL)
        {
          clen = strlen (pattern);
          for (int j = 0; j < clen; j++)
            {
              strncpy (pattern_a, pattern, j);
              pattern_a[j] = '\0';
              strncpy (core, pattern + j, 1);
              core[1] = '\0';
              strncpy (pattern_b, pattern + j + 1, clen - j);
              coreint = atoi (core);
              if (coreint == 0)
                {
                  coreintconv = 1;
                }
              else if (coreint == 1)
                {
                  coreintconv = 0;
                }
              sprintf (coreconv, "%d", coreintconv);
          pattern_n[0]='\0';
              strcat (pattern_n, pattern_a);
              strcat (pattern_n, coreconv);
              strcat (pattern_n, pattern_b);
              for (int k = 0; k < table_master_line; k++)
                {
                  sprintf (line, "%s", table_master[k]);
                  strtok (line, " ");
                  chk4 = strstr (line, pattern_n);
                  if (chk4)
                    {
                      chk2 = strtok (NULL, " ");
                      break;
                    }
                  else
                    {
                      chk2 = "";
                    }
                }
              chk2int = atoi (chk2);
              if (chk2int == 1 || !chk4)
                {
                  table_write (pattern, 2);
                  table_write (" ", 2);
                  table_write (pattern_n, 2);
                  table_add (2);
                  groupchk = 1;
                  count2++;
                  for (int k = 0; k < comp2_temp_line; k++)
                    {
                      if (strstr (comp2_temp[k], pattern) || strstr (comp2_temp[k], pattern_n))
                        {
                          for (int l = k; l < comp2_temp_line; l++)
                            {
                              sprintf (comp2_temp[l], "%s",
                                       comp2_temp[l + 1]);
                            }
                          comp2_temp_line--;
                          k--;
                        }
                    }
                }
            }
          if (groupchk == 0)
            {
              table_write (pattern, 1);
              table_add (1);
              count++;
              for (int k = 0; k < comp2_temp_line; k++)
                {
                  if (strstr (comp2_temp[k], pattern))
                    {
                      for (int l = k; l < comp2_temp_line; l++)
                        {
                          sprintf (comp2_temp[l], "%s", comp2_temp[l + 1]);
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
              if (strstr (comp2_temp[k], pattern))
                {
                  for (int l = k; l < comp2_temp_line; l++)
                    {
                      sprintf (comp2_temp[l], "%s", comp2_temp[l + 1]);
                    }
                  comp2_temp_line--;
                  k--;
                }
            }
        }
      if (comp2_temp[0][0] == '\0')
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
  free (pattern);
  free (pattern_a);
  free (pattern_b);
  free (pattern_n);
  free (line);
  for (int i = 0; i < table_master_line + 1; i++)
    {
      free (comp2_temp[i]);
    }
  free (comp2_temp);
  return res;
}

int
logic_comp_n (int dim)
{
  int len = 0;
  int sum = 0;
  int count = 0;
  int count2 = 0;
  int countpast = 0;
  int res = 0;
  int charnum = 0;
  char *line;
  char ***pattern;
  char ***pattern_n;
  char ***p1;
  char ***p2;
  char ***core;
  char ***core_n;
  int org_flag = 0;
  int coreint = 0;
  int coreint_n = 0;
  int coreintpast = 0;
  char ***pattern_store;
  int *pattern_count;

  struct_add ();
  charnum = strlen (block_list.block[block_list.block_num - 2].table[0]);
  line = (char *) malloc (sizeof (char) * (charnum + 1));
  compn_temp_line = block_list.block[block_list.block_num - 2].size - 1;
  compn_temp = (char **) malloc (sizeof (char *) * compn_temp_line);
  for (int i = 0; i < compn_temp_line; i++)
    {
      compn_temp[i] = (char *) malloc (sizeof (char) * (charnum + 1));
      sprintf (compn_temp[i], "%s",
               block_list.block[block_list.block_num - 2].table[i]);
    }
  sprintf (line, "%s", compn_temp[0]);
  strtok (line, " ");
  len = strlen (line);
  pattern = (char ***) malloc (sizeof (char **) * compn_temp_line);
  pattern_n = (char ***) malloc (sizeof (char **) * compn_temp_line);
  p1 = (char ***) malloc (sizeof (char **) * compn_temp_line);
  p2 = (char ***) malloc (sizeof (char **) * compn_temp_line);
  core = (char ***) malloc (sizeof (char **) * compn_temp_line);
  core_n = (char ***) malloc (sizeof (char **) * compn_temp_line);
  pattern_count=(int*)malloc(sizeof(int) * compn_temp_line);
  pattern_store=(char***)malloc(sizeof(char**) * compn_temp_line);
  for (int j = 0; j < compn_temp_line ; j++)
    {
      pattern[j] = (char **) malloc (sizeof (char *) * dim);
      pattern_n[j] = (char **) malloc (sizeof (char *) * dim);
      p1[j] = (char **) malloc (sizeof (char *) * dim);
      p2[j] = (char **) malloc (sizeof (char *) * dim);
      core[j] = (char **) malloc (sizeof (char *) * dim);
      core_n[j] = (char **) malloc (sizeof (char *) * dim);
      for (int i = 0; i < dim; i++)
    {
      pattern[j][i] = (char *) malloc (sizeof (char) * (len + 1));
      pattern_n[j][i] = (char *) malloc (sizeof (char) * (len + 1));
      p1[j][i] = (char *) malloc (sizeof (char) * (len + 1));
      p2[j][i] = (char *) malloc (sizeof (char) * (len + 1));
      core[j][i] = (char *) malloc (sizeof (char *) * 2);
      core_n[j][i] = (char *) malloc (sizeof (char *) * 2);
    }
      pattern_store[j]=(char**)malloc(sizeof(char*) * 1);
      pattern_count[j]=0;
    }

#pragma omp parallel for private(coreint,coreintpast,coreint_n,sum,count,org_flag, countpast)
  for (int i = 0; i < compn_temp_line; i++)
    {
      for (int j = 0; j < dim; j++)
        {
      strncpy(pattern[i][j], compn_temp[i] + (j * (len + 1)),len);
      pattern[i][j][len]='\0';
        }
      for (int k = 0; k < len; k++)
        {
          sum = 0;
      count = 0;
      coreintpast = 0;
          for (int l = 0; l < dim; l++)
            {
              strncpy (&p1[i][l][0], &pattern[i][l][0], k);
              p1[i][l][k] = '\0';
              strncpy (&core[i][l][0], &pattern[i][l][k], 1);
              core[i][l][1] = '\0';
              strncpy (&p2[i][l][0], &pattern[i][l][k + 1], len - k - 1);
              p2[i][l][len - k - 1] = '\0';
              coreint = atoi (&core[i][l][0]);
              coreint_n = 1 - coreint;
              sprintf (&core_n[i][l][0], "%d", coreint_n);
              if (l == 0)
                {
                  coreintpast = coreint;
                }
              else
                {
                  if (coreint != coreintpast)
                    {
                      sum = 1;
              break;
                    }
                  else
                    {
                      coreintpast = coreint;
                    }
                }
            }
          if (sum == 0)
            {
              for (int m = 0; m < dim; m++)
                {
          org_flag = 0;
          pattern_n[i][m][0] = '\0';
          strcat (pattern_n[i][m], p1[i][m]);
          strcat (pattern_n[i][m], core_n[i][m]);
          strcat (pattern_n[i][m], p2[i][m]);
          countpast = count;
          for (int n = 0; n < table_master_line; n++)
            {
              if (strstr (table_master[n], pattern_n[i][m]))
            {
              if (strstr (table_master[n], " 1"))
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
          if(pattern_count[i] > 0)
            {
              pattern_store[i]=(char**)realloc(pattern_store[i],sizeof(char*)*(pattern_count[i]+1));
            }
          pattern_store[i][pattern_count[i]]=(char*)malloc(sizeof(char)*(charnum+1)*2);
          pattern_store[i][pattern_count[i]][0]='\0';
                  for (int n = 0; n < dim; n++)
                    {
                      strcat(pattern_store[i][pattern_count[i]],pattern[i][n]);
                      strcat(pattern_store[i][pattern_count[i]]," ");
                      strcat(pattern_store[i][pattern_count[i]],pattern_n[i][n]);
                      strcat(pattern_store[i][pattern_count[i]]," ");
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
      table_write(pattern_store[i][j],0);
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
      for (int i = 0; i < dim; i++)
    {
      free (pattern[j][i]);
      free (pattern_n[j][i]);
      free (p1[j][i]);
      free (p2[j][i]);
      free (core[j][i]);
      free (core_n[j][i]);
    }
      for (int k = 0 ; k < pattern_count[j]; k++)
    {
      free (pattern_store[j][k]);
    }
      free (pattern_store[j]);
      free (compn_temp[j]);
      free (pattern[j]);
      free (pattern_n[j]);
      free (p1[j]);
      free (p2[j]);
      free (core[j]);
      free (core_n[j]);
    }
  free (pattern);
  free (pattern_n);
  free (p1);
  free (p2);
  free (core);
  free (core_n);
  free (line);
  free (compn_temp);
  free(pattern_store);
  free(pattern_count);
  return res;
}

int
binstr_to_uint32 (char *str)
{
  int ret = 0;

  while (*str == '0' || *str == '1')
    {
      ret <<= 1;
      ret += *str - '0';
      str++;
    }

  return ret;
}

int
split_space (int *result, char *str)
{
  int len = strlen (str);
  char *head = str;
  int count = 0;

  head = str;
  for (int i = 0; i <= len; i++, str++)
    {
      if (*str != '0' && *str != '1')
        {
          if (*head == '0' || *head == '1')
            {
              result[count] = binstr_to_uint32 (head);
              head = str + 1;
              count++;
            }
          else
            {
              break;
            }
        }
    }
  return count;
}

int
resize (int dim)
{
  int *omp_n_stop;
  char *line;
  char **omp_line;
  int **omp_pos;
  int **omp_i_part;
  int *fp2_i_buff;
  char *fp2buff;
  int i_buff_len = 0;
  int line_num = 0;
  int count = 0;
  int *omp_count;
  int *omp_br;
  int charnum = 0;
  char ***omp_resize_temp;
  int resize_temp_line = 0;
  int *omp_resize_temp_line;
  int block_size = 0;

  printf ("Resize ");
  charnum = strlen (block_list.block[block_list.block_num - 1].table[0]) + 2;
  line = (char *) malloc (sizeof (char) * charnum);
  i_buff_len = 0;
  line_num = block_list.block[block_list.block_num - 1].size - 1;

  for (int i = 0; i < charnum - 1; i++)
    {
      if (block_list.block[block_list.block_num - 1].table[0][i] == ' ')
        {
          i_buff_len++;
        }
    }
  sprintf (line, "%s", block_list.block[block_list.block_num - 1].table[0]);
  fp2_i_buff = (int *) malloc (line_num * i_buff_len * sizeof (int));
  fp2buff = (char *) malloc (line_num * (charnum + 2));
  for (int i = 0; i < line_num; i++)
    {
      sprintf (line, "%s",
               block_list.block[block_list.block_num - 1].table[i]);
      strcpy (fp2buff + i * (charnum + 2), line);
      split_space (fp2_i_buff + i * i_buff_len, line);
    }
  strtok (line, " ");
  omp_resize_temp=(char***)malloc(sizeof(char**) * block_list.block[block_list.block_num - 2].size);
  omp_line=(char**)malloc(sizeof(char*) * block_list.block[block_list.block_num - 2].size);
  omp_count=(int*)malloc(sizeof(int) * block_list.block[block_list.block_num - 2].size);
  omp_resize_temp_line=(int*)malloc(sizeof(int) * block_list.block[block_list.block_num - 2].size);
  omp_br=(int*)malloc(sizeof(int) *  block_list.block[block_list.block_num - 2].size);
  omp_n_stop=(int*)malloc(sizeof(int) *  block_list.block[block_list.block_num - 2].size);
  omp_pos=(int**)malloc(sizeof(int*) * block_list.block[block_list.block_num - 2].size);
  omp_i_part=(int**)malloc(sizeof(int*) * block_list.block[block_list.block_num - 2].size);
  for (int m = 0; m < block_list.block[block_list.block_num - 2].size - 1; m++)
    {
      omp_resize_temp[m]=(char **) malloc (sizeof (char *) * 1);
      omp_line[m]=(char *)malloc(sizeof (char) *(strlen(block_list.block[block_list.block_num - 2].table[0])+1));
      omp_count[m]=0;
      omp_resize_temp_line[m]=0;
      omp_n_stop[m] = 0;
      omp_br[m] = 0;
      omp_i_part[m] = (int *) malloc (sizeof (int) * dim);
    }
#pragma omp parallel for private(count)
  for (int m = 0; m < block_list.block[block_list.block_num - 2].size - 1; m++)
    {
      sprintf (omp_line[m], "%s", block_list.block[block_list.block_num - 2].table[m]);
      count = split_space (omp_i_part[m], omp_line[m]);
      omp_count[m]=0;
      for (int i = 0; i < line_num; i++)
        {
      omp_br[m] = 0;
      for (int j = 0; j < count; j++)
            {
          omp_pos[m] = fp2_i_buff + i * i_buff_len;
          omp_n_stop[m]=i_buff_len;
          for (int n = 0; n < i_buff_len; n++, omp_pos[m]++)
        {
          if (omp_i_part[m][j] == *omp_pos[m] && omp_n_stop[m] == i_buff_len)
            {
              omp_n_stop[m] = n;
              break;
            }
        }
          if (omp_n_stop[m] < i_buff_len)
        {
          omp_br[m]++;
        }
          else
        {
          break;
        }
            }
      if (omp_br[m] != count)
            {
          omp_count[m]++;
            }
        }
      if (omp_count[m] == line_num)
        {
          omp_resize_temp_line[m]++;
      omp_resize_temp[m]=(char **) realloc (omp_resize_temp[m],sizeof (char *) * omp_resize_temp_line[m]);
      omp_resize_temp[m][omp_resize_temp_line[m] - 1]=(char *) malloc (sizeof (char) * charnum);
      sprintf (omp_resize_temp[m][omp_resize_temp_line[m] - 1], "%s", omp_line[m]);
        }
    }
  block_size=block_list.block[block_list.block_num - 2].size;
  table_reset (block_list.block_num - 1);
  for (int m = 0; m < block_size - 1; m++)
    {
      if(omp_resize_temp_line[m] > 0)
    {
      for (int i = 0; i < omp_resize_temp_line[m]; i++)
        {
          resize_temp_line++;
          table_write (omp_resize_temp[m][i], block_list.block_num - 1);
          table_add (block_list.block_num - 1);
        }
    }
    }
  printf (" ..end table_%d : %d\n", dim, resize_temp_line);
  for (int m = 0; m < block_size - 1 ; m++)
    {
      for (int i = 0; i < omp_resize_temp_line[m]; i++)
    {
      free(omp_resize_temp[m][i]);
    }
      free(omp_resize_temp[m]);
    }
  free(omp_resize_temp);  
  for (int i = 0; i < block_size - 1; i++)
    {
      free(omp_i_part[i]);
      free(omp_line[i]);
    }
  free(omp_i_part);
  free(omp_line);
  free(omp_count);
  free(omp_resize_temp_line);
  free (line);
  free (fp2buff);
  free (fp2_i_buff);
  free(omp_br);
  free(omp_n_stop);
  free(omp_pos);

  if (resize_temp_line > 1)
    {
      return 1;
    }
  return 0;
}

void
duplicate_detect (int dim)
{
  char *line;
  char **part;
  int *br;
  int chk = 0;
  int len = 0;
  int charnum = 0;
  char **dup_temp;
  char **dup_list;
  int dup_temp_line = 0;
  int dup_list_line = 0;

  printf ("removing duplication ...");
  charnum = strlen (block_list.block[block_list.block_num - 2].table[0]) + 2;
  line = (char *) malloc (sizeof (char) * charnum);
  dup_temp = (char **) malloc (sizeof (char *) * 1);
  dup_list = (char **) malloc (sizeof (char *) * 1);
  sprintf (line, "%s", block_list.block[block_list.block_num - 2].table[0]);
  strtok (line, " ");
  len = strlen (line);
  part = (char **) malloc (sizeof (char *) * dim);
  br = (int *) malloc (sizeof (int) * dim);
  for (int i = 0; i < dim; i++)
    {
      part[i] = (char *) malloc (sizeof (char) * (len + 1));
      br[i] = 0;
    }
  for (int i = 0; i < block_list.block[block_list.block_num - 2].size - 1;
       i++)
    {
      chk = 0;
      for (int j = 0; j < dim; j++)
        {
          strncpy(part[j], block_list.block[block_list.block_num - 2].table[i] + (j * (len + 1)),len);
          part[j][len]='\0';
        }
#pragma omp parallel for shared(chk)
      for (int j = 0; j < dim; j++)
        {
      if(chk==1)
        {
          continue;
        }
          br[j] = 0;
      for (int k = 0; k < table_master_line; k++)
        {
          if (strstr (table_master[k], part[j]))
        {
          break;
        }
          else if (k == table_master_line - 1)
                {
                  br[j] = 2;
                }
        }
          if (br[j] != 2)
            {
          for (int l = 0;
           l < block_list.block[block_list.block_num - 1].size - 1;
           l++)
        {
          if (strstr (block_list.block[block_list.block_num - 1].table[l], part[j]))
            {
              br[j] = 2;
              break;
            }
        }
              if (br[j] != 2)
                {
                  for (int l = 0;
                       l <
             block_list.block[block_list.block_num - 2].size - 1;
                       l++)
                    {
              if (strstr (block_list.block[block_list.block_num - 2].table[l], part[j]))
                        {
                          br[j]++;
                        }
                    }
                  if (dup_list_line != 0)
                    {
                      for (int l = 0; l < dup_list_line; l++)
                        {
                          if (strstr (dup_list[l], part[j]))
                            {
                  br[j]--;
                            }
                        }
                    }
                }
            }
          if (br[j] < 2)
            {
          chk = 1;
        }
        }
      if (chk == 1)
        {
          dup_temp_line++;
          dup_temp =
            (char **) realloc (dup_temp, sizeof (char *) * dup_temp_line);
          dup_temp[dup_temp_line - 1] =
            (char *) malloc (sizeof (char) * charnum);
          dup_temp[dup_temp_line - 1][0] = '\0';
          for (int j = 0; j < dim; j++)
            {
              strcat (dup_temp[dup_temp_line - 1], part[j]);
              strcat (dup_temp[dup_temp_line - 1], " ");
            }
        }
      else
        {
          dup_list_line++;
          dup_list =
            (char **) realloc (dup_list, sizeof (char *) * dup_list_line);
          dup_list[dup_list_line - 1] =
            (char *) malloc (sizeof (char) * charnum);
          dup_list[dup_list_line - 1][0] = '\0';
          for (int j = 0; j < dim; j++)
            {
              strcat (dup_list[dup_list_line - 1], part[j]);
              strcat (dup_list[dup_list_line - 1], " ");
            }
        }
    }
  table_reset (block_list.block_num - 1);
  if (dup_temp_line != 0)
    {
      for (int i = 0; i < dup_temp_line; i++)
        {
          table_write (dup_temp[i], block_list.block_num - 1);
          table_add (block_list.block_num - 1);
        }
    }
  printf ("end  table_%d : %d\n", dim, dup_temp_line);
  free (line);
  for (int i = 0; i < dup_temp_line; i++)
    {
      free (dup_temp[i]);
    }
  free (dup_temp);
  for (int i = 0; i < dup_list_line; i++)
    {
      free (dup_list[i]);
    }
  free (dup_list);
  for (int i = 0; i < dim; i++)
    {
      free (part[i]);
    }
  free (part);
  free (br);
}

int
logic_function (int dim)
{
  int len = 0;
  int num = 0;
  int sum = 0;
  int flag = 0;
  int charnum = 0;
  int renum = 0;
  int count = 0;
  char *line;
  char bit[2] = { 0 };

  for (int m = block_list.block_num - 1; m >= 0; m--)
    {
      charnum = strlen (block_list.block[m].table[0]) + 2;
      flag = 0;
      if (charnum > 2)
        {
          line = (char *) malloc (sizeof (char) * charnum);
          sprintf (line, "%s", block_list.block[m].table[0]);
          strtok (line, " ");
          len = strlen (line);
          for (int n = 0; n < block_list.block[m].size - 1; n++)
            {
              sprintf (line, "%s", block_list.block[m].table[n]);
              if (flag == 0)
                {
                  flag = 1;
                }
              else
                {
                  printf (" + ");
                }
              for (int i = 0; i < len; i++)
                {
                  sum = 0;
                  for (int j = 0; j < dim; j++)
                    {
              strncpy (bit, line + (j * (len + 1)) + i, 1);
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
          free (line);
        }
      dim = dim / 2;
    }
  return 0;
}

void
free_all (void)
{
  free_struct ();
  for (int i = 0; i < table_master_line; i++)
    {
      free (table_master[i]);
    }
  free (table_master);
  for (int i = 0; i < table_remake_line; i++)
    {
      free (table_remake[i]);
    }
  free (table_remake);
  free (table_remake_ref);
}

int
main (int argc, char *argv[])
{
  int res;
  int dim = 2;
  int comp = 0;
  struct timeval start_timeval,
    end_timeval;
  double sec_timeofday;
  FILE *fp;

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
  argc = 0;
  res = table_opt (argv[1]);
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
          if (res > 0)
            {
              comp = resize (dim);
              if (comp == 1)
        {
                  duplicate_detect (dim);
        }
            }
          else
            {
              duplicate_detect (dim);
            }
          dim = dim * 2;
        }
      logic_function (dim);
    }
  free_all ();
  gettimeofday (&end_timeval, NULL);
  sec_timeofday =
    (double) (end_timeval.tv_sec - start_timeval.tv_sec) +
    (double) (end_timeval.tv_usec - start_timeval.tv_usec) / 1000000;
  printf ("exec time %.6f [s]\n", sec_timeofday);
  return 0;
}
