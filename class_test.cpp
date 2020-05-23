#include <iostream>
#include <vector>
using namespace std;

/*
1. table動的確保　済
2. list動的確保
3. スマートポインタで自動管理
4. 参照渡しでメモリ削減
5. オーバーライド
6. 仮想関数
7. 型推論
8. C版以上に高速
*/

typedef vector< vector<int> > vec2;
typedef vector< vec2 > vec3;

class list{
private:
  int tablenum;
public:
  list();
  ~list();
  void add();
  void del();
};

class table{
private:
   int linenum;
   int width;
public:
   table();
   ~table();
   vec2 bittable;
   int add(int bitwidth);
   void write(int linenum, int ad , int val);
   int read(int linenum, int ad);
};

list::list()
{
 tablenum=0;
 t_list = new table[1];
 cout << t_list[0].bittable[0][0]  << endl;
}
list::~list()
{
 cout << "list deleted" << endl;
}
void list::add()
{
 tablenum++;
 t_list.resize(tablenum,vec2(1));
 table t;
 t_list[tablenum]=t.bittable;
}

table::table()
{
  linenum=0;
  width=1;
  vec2 bittable(1,vector<int>(1,0));
  cout << "table create " <<endl;
}
table::~table()
{
  cout << "table deleted" << endl;
}

int table::add(int bitwidth)
{
  linenum++;
//linenum * widthの領域確保
  bittable.resize(linenum,vector<int>(width,0));
  return linenum;
}

void table::write(int line,int ad , int val)
{
  bittable[line-1][ad]=val;
}

int table::read(int line,int ad)
{
  return bittable[line-1][ad];
}


int main (int argc, const char* argv[]){
 int current_line=0;
 int readval[2]={0};
 table *t_list;
 list l;
 l.list(t_list);
 /*
 current_line=t.add(2); // tableに1行目の領域追加

 t.write(1,0,5);//1行目の1個目の数字を5にする

 readval[0]=t.read(1,0);//読み出し
 readval[1]=t.read(1,1);
 cout << current_line << " " << readval[0] << "," << readval[1] << endl;
 current_line=t.add(2);// tableに2行目追加
 t.write(2,1,5);//2行目の2個目の数字を5にする
 readval[0]=t.read(2,0);
 readval[1]=t.read(2,1);
 cout << current_line << " " << readval[0] << "," << readval[1] << endl;
 */
 return 0;
}
