#include <iostream>
#include <vector>
#include <cmath>
#include "logic_table.cpp"
#include "lib/file_handler.cpp"
using std::cout;
using std::endl;
using std::to_string;

typedef std::vector<int> vec;
typedef std::vector< vec > vec2;
typedef std::string str;
typedef std::vector<LogicTable> vect;

class TableManagement{
private:
  vect t_list;
public:
  TableManagement();
  ~TableManagement();
  void add(int width);
  void powadd();
  int read(int row, int col);
  void read_all();
  void write(int row, int col, int val);
  void size();
};

TableManagement::TableManagement(){
  
  cout << "list created" << endl;
}
TableManagement::~TableManagement(){
  cout << "list deleted" << endl;
}

void TableManagement::add(int width){
  t_list.resize(width);
}

void TableManagement::powadd(){
  add(std::pow(2, t_list.size()));
}

void TableManagement::write(int row, int col, int val){
}

int TableManagement::read(int row,int col){
  return 0;
}

void TableManagement::size(){
  cout << to_string(t_list.size()) << endl;
}

int main(int argc, const char* argv[]){
  FileHandler f;
  vec2 list;
  int retcode = f.read(argc, argv[1], list);
  //TODO: エラーハンドリング方法を学んだらリファクタする
  if(retcode == 0){
      f.read_all(list);
      
  } else {
      return 1;
  }
  return 0;
}
  TableManagement tm;
  tm.powadd();
  tm.size();
  tm.powadd();
  tm.size();
  tm.powadd();
  tm.size();
  return 0;
}