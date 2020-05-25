#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

/*
 * 1. table動的確保　済
 * 2. list動的確保 済
 * 3. スマートポインタで自動管理
 * 4. 参照渡しでメモリ削減
 * 5. オーバーライド
 * 6. 仮想関数
 * 7. 型推論
 * 8. インライン関数
 * 9. C版以上に高速
 * */
typedef vector< int > vec;
typedef vector< vec > vec2;
typedef vector< vec2 > vec3;

class LookUpTable{
private:
   int ptnlen;
   int linenum;
public:
   LookUpTable();
   ~LookUpTable();
   int fileread(char* filename);
   vector<string> pattable;
   vector<string> value;
   vector<int> pattern;
   vector<int> truth;
};
class table{
private:
   int linenum;
   int width;
public:
   vec2 bittable;
   table(int bitwidth);
   ~table();
   int add();
   void write(int linenum, int ad , int val);
   int read(int linenum, int ad);
};
class list{
private:
  int tablenum;
public:
  vector<table> t_list;
  list();
  ~list();
  void add();
  void del();
};

LookUpTable::LookUpTable()
{
 cout << "LUT construct" << endl;
 linenum=0;
 ptnlen=0;
}
LookUpTable::~LookUpTable()
{
 cout << "LUT destruct" << endl;
}

int LookUpTable::fileread(char* filename)
{
  ifstream ifs(filename);
  string str;
  if (ifs.fail())
  {
    return -1;
  }
  while (getline(ifs, str))
  {
    while (str[0]==' ')//先頭のスペース削除
      str = str.substr(1);
    if (str[0]=='\0') //空行無視
      continue;
    if (ptnlen!=0 && ptnlen!=str.find(' '))//パターン長が一定でなければエラー
      return -1;
    linenum++;
    ptnlen=str.find(' ');
    pattable.resize(linenum);
    value.resize(linenum);
    pattern.resize(linenum);
    truth.resize(linenum);
    pattable[linenum-1] = str.substr(0, ptnlen);
    value[linenum-1] = str.substr(ptnlen + 1);
    while (value[linenum-1][0]==' ')//パターンと真理値の間の空白が複数あれば切り詰める
        value[linenum-1] = value[linenum-1].substr(1);
    if (value[linenum-1][0]!='0' && value[linenum-1][0]!='1')//真理値無しであればエラー
       return -1;
    if (value[linenum-1][1]!='\0' &&  value[linenum-1][1]!=' ')//真理値が2文字以上ならエラー
       return -1;
    value[linenum-1] = value[linenum-1].substr(0,1);
    //パターンを数値変換
    for (int i = ptnlen-1 ; i >=0 ; i--)
    {
      if (pattable[linenum-1][i]=='1')
      {
        pattern[linenum-1]+=pow(2,ptnlen-1-i);
      }
      else if (pattable[linenum-1][i]!='0')
       return -1; //0, 1以外の文字が使われていればエラー
    }
    //真理値を数値変換
    istringstream(value[linenum-1]) >> truth[linenum-1];
    cout << pattable[linenum-1] << "/" << pattern[linenum-1] << ":" << value[linenum-1] << "/" << truth[linenum-1] << endl;
  }
  //パターン重複があればエラー
  sort(pattable.begin(), pattable.end());
  if (unique(pattable.begin(), pattable.end()) < pattable.end())
    return -1;
  cout <<  "pattern length : " << ptnlen << endl;
  cout << "table lines : " << linenum << endl;
  return 0;
}

list::list()
{
 cout << "list construct" << endl;
 tablenum=0;
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
  vec2 bittable(1,vec(1,0));
  cout << "table construct" << endl;
}
table::~table()
{
  cout << "table destruct" << endl;
}

int table::add()
{
  linenum++;
  bittable.resize(linenum,vec(width,0));
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

int main (int argc, char* argv[]){
    LookUpTable lut;
    if (lut.fileread(argv[1])!=0)
    {
      cout << "table err" << endl;
      return -1;
    }
    list l;
    int readval[8]={0};
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
