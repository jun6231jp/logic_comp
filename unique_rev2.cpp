#include <iostream>
#include <memory>
#include <cstdlib>
#include <exception>
using namespace std;

typedef unique_ptr<unique_ptr<int[]>[]> vec2;
typedef unique_ptr<int[]> vec;

class ptr {
private:
  int Size = 0;
  int Width = 0;
  int CurLine = 0;
  int CurCol = 0;
public:
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
};

int ptr::add()
{
  Buff = make_unique< vec[] > (Size+1);
  for(int i = 0 ; i < Size; i++)
    {
      Buff[i]=move(Table[i]);
    }
  Buff[Size]=make_unique<int[]>(1);
  for(int j = 0 ; j < Width; j++)
    {
      Buff[Size][j] = 0;
    }
  Size++;
  Table = make_unique< vec[] > (Size);
  for(int i = 0 ; i < Size; i++)
    {
      Table[i]=move(Buff[i]);
    }
  return Size;
}

int main()
{
  ptr table;
  int size=0;
  table.set(3);
  size=table.add();
  table.write(3);
  table.write(2);
  table.write(1);
  
  for(int j = 0 ; j < size; j++)
    {
      for(int i = 0 ; i < 3 ; i++)
	cout << table.Table[j][i] << " " ;
      cout << endl;
    }

  cout << " ----resized----> " << endl;
  
  size=table.add();
  table.write(6);
  table.write(5);
  table.write(4);
  for(int j = 0 ; j < size; j++)
    {
      for(int i = 0 ; i < 3 ; i++)
	cout << table.Table[j][i] << " " ;
      cout << endl;
    }
  
  return 0;
}
