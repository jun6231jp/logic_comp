#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

/*
 * 1. table動的確保　済
 * 2. list動的確保
 * 3. スマートポインタで自動管理
 * 4. 参照渡しでメモリ削減
 * 5. オーバーライド
 * 6. 仮想関数
 * 7. 型推論
 * 8. インライン関数
 * 9. C版以上に高速
 * */
int globa_width=1;

typedef vector< vector<int> > vec2;
typedef vector< vec2 > vec3;
class table{
private:
   int linenum;
   int width;
public:
   table(int bitwidth);
   ~table();
   vec2 bittable;
   int add();
   void write(int linenum, int ad , int val);
   int read(int linenum, int ad);
};
class list{
private:
  int tablenum;
public:
  list();
  ~list();
  vector<table> t_list;
  void add();
  void del();
};

list::list()
{
 cout << "list construct" << endl;
 tablenum=0;
 vector<table> t_list;
}
list::~list()
{
 cout << "list destruct" << endl;
}
void list::add()
{
 tablenum++;
 int width=pow(2,tablenum);
 t_list.resize(tablenum,table(width));
 cout << "table " << tablenum << "created" << endl;
}

void list::del()
{
 cout << "table " << tablenum << "deleted" << endl;
 tablenum--;
 int width=pow(2,tablenum);
 t_list.resize(tablenum,table(width));
}

table::table(int bitwidth)
{
  linenum=0;
  width=bitwidth;
  vec2 bittable(1,vector<int>(1,0));
  cout << "table construct" << endl;
}
table::~table()
{
  cout << "table destruct" << endl;
}

int table::add()
{
  linenum++;
  bittable.resize(linenum,vector<int>(width,0));
  cout << "line added width:" << width << endl;
  return linenum;
}

void table::write(int line,int ad , int val)
{
  bittable[line][ad]=val;
}

int table::read(int line,int ad)
{
  return bittable[line][ad];
}

int main (int argc, const char* argv[]){
    int current_line=0;
    int readval[8]={0};
    list l;
    l.add();
    l.t_list[0].add();
    l.t_list[0].write(0,0,1);
    readval[0]=l.t_list[0].read(0,0);
    readval[1]=l.t_list[0].read(0,1);
    cout <<  readval[0] << " " <<  readval[1] << endl;
    l.add();
    readval[0]=l.t_list[0].read(0,0);
    readval[1]=l.t_list[0].read(0,1);
    cout <<  readval[0] << " " <<  readval[1] << endl;
    l.t_list[1].add();
    l.t_list[1].write(0,0,1);
    l.t_list[1].add();
    l.t_list[1].write(1,1,2);
    readval[0]=l.t_list[1].read(0,0);
    readval[1]=l.t_list[1].read(0,1);
    readval[2]=l.t_list[1].read(0,2);
    readval[3]=l.t_list[1].read(0,3);
    cout << readval[0] << " " <<  readval[1] << " " << readval[2] << " " <<  readval[3] << endl;
    readval[0]=l.t_list[1].read(1,0);
    readval[1]=l.t_list[1].read(1,1);
    readval[2]=l.t_list[1].read(1,2);
    readval[3]=l.t_list[1].read(1,3);
    cout << readval[0] << " " <<  readval[1] << " " << readval[2] << " " <<  readval[3] << endl;
    l.t_list[0].add();
    l.t_list[0].write(1,1,3);
    readval[0]=l.t_list[0].read(0,0);
    readval[1]=l.t_list[0].read(0,1);
    cout <<  readval[0] << " " <<  readval[1] << endl;
    readval[0]=l.t_list[0].read(1,0);
    readval[1]=l.t_list[0].read(1,1);
    cout <<  readval[0] << " " <<  readval[1] << endl;
    l.del();
    l.t_list[0].add();
    l.t_list[0].write(2,0,4);
    readval[0]=l.t_list[0].read(0,0);
    readval[1]=l.t_list[0].read(0,1);
    cout <<  readval[0] << " " <<  readval[1] << endl;
    readval[0]=l.t_list[0].read(1,0);
    readval[1]=l.t_list[0].read(1,1);
    cout <<  readval[0] << " " <<  readval[1] << endl;
    readval[0]=l.t_list[0].read(2,0);
    readval[1]=l.t_list[0].read(2,1);
    cout <<  readval[0] << " " <<  readval[1] << endl;
    return 0;
}