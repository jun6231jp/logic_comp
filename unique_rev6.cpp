#include <iostream>
#include <memory>
#include <cstdlib>
#include <exception>
#include <algorithm>

using namespace std;
#define MTRACE

#ifdef MTRACE
#include <mcheck.h>
#endif

typedef unique_ptr<int[]> vec;
typedef unique_ptr<unique_ptr<int[]>[]> vec2;

class ptr {
private:
  int CurLine = 0;
  int CurCol = 0;
public:
  int Size = 0;
  int Width = 0;
  vec2 Buff;
  vec2 Table;
  ptr(){}
  ~ptr(){}
  int add();
  void write(int num)
  {
    Table[CurLine][CurCol]=num;
    CurCol++;
    if(CurCol==Width)
      {
	CurLine++;
	CurCol=0;
      }
  }
  void set(int width){Width=width;}
  void MatrixUniqSort();
  auto comp_idx(int** buff,int* uniq); 
};

int ptr::add()
{
  Buff = make_unique<vec[]> (Size+1);
  for(int i = 0 ; i < Size; i++)
    Buff[i].swap(Table[i]);
  Buff[Size]=make_unique<int[]>(Width);
  for(int j = 0 ; j < Width; j++)
    Buff[Size][j] = 0;
  Size++;  
  Table = make_unique<vec[]> (Size);
  Table.swap(Buff);
  return Size;
}

auto ptr::comp_idx(int** buff, int* uniq)
{
  int width = Width;
  return [buff,uniq,width](int l_idx, int r_idx)
	 {
	   for (int i = 0 ; i < width; i ++)
	     {
	       if(buff[l_idx][i] != buff[r_idx][i])
		 return buff[l_idx][i] < buff[r_idx][i];
	     }
	   if(uniq[l_idx]==0)
	     uniq[r_idx]=1;
	   return buff[l_idx][width-1] < buff[r_idx][width-1];
	 };
}

void ptr::MatrixUniqSort ()
{
  int index[Size] = {0};
  int newsize = 0;
  int** buff = new int*[Size];
  for (int i = 0 ; i < Size ; i++)
    buff[i] = new int[Width];
  int* uniq = new int[Size];
  for (int i = 0 ; i < Size ; i++)
    sort(&(Table[i][0]),&(Table[i][Width-1])+1);
  for (int i=0; i < Size; i++)
    {
      index[i]=i;
      uniq[i]=0;
      for(int j=0; j < Width; j++)
	buff[i][j] = Table[i][j];
    }
  sort(&(index[0]),&(index[Size-1])+1,comp_idx(buff,uniq));
  for (int i = 0 ; i < Size ; i++)
    {
      for(int j=0; j < Width; j++)
	buff[index[i]][j]=Table[i][j];
    }
  for (int i = 0 ; i < Size ; i++)
    {
      if (uniq[i] == 0)
	{
	  for(int j=0; j < Width; j++)
	    Table[newsize][j]=buff[i][j];
	  newsize++;
	}
    }
  for (int i = 0 ; i < Size ; i++)
    delete [] buff[i];
  delete [] buff;
  delete [] uniq;
  Size = newsize;
}

int main()
{
#ifdef MTRACE
  mtrace();
#endif
  unique_ptr <ptr> table(new ptr());
  int width = 50;
  int size = 200;
  table->set(width);
  for (int k = 0 ; k < size ; k++)
    {
      table->add();
      for (int i = 0 ; i < table->Width ; i++ )
	{
	  table->write((size/3 + width) - k/3 - i);
	}
    }
  
  for (int j = 0 ; j < table->Size ; j++)
    {
      for(int i = 0 ; i < table->Width ; i++)
	cout << table->Table[j][i] << " " ;
      cout << endl;
    }
  cout << table->Size << "lines" << endl;
  cout << endl;
  
  table->MatrixUniqSort();
  
  for (int j = 0 ; j < table->Size ; j++)
    {
      for(int i = 0 ; i < table->Width ; i++)
	cout << table->Table[j][i] << " " ;
      cout << endl;
    }

  cout << table->Size << "lines" << endl;
  
  table.reset();
  
#ifdef MTRACE
  muntrace();
#endif
  return 0;
}
