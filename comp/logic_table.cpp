#include <iostream>
#include <vector>
using std::cout;
using std::endl;
using std::to_string;

typedef std::vector<int> vec;
typedef std::vector< vec > vec2;
typedef std::string str;

class LogicTable{
private:
  vec2 bittable;
public:
  LogicTable();
  ~LogicTable();
  void add(int bitwidth);
  int read(int row, int col);
  void read_all();
  void write(int row, int col, int val);
  int row_size();
  int col_size(int row);
  void test();
};

LogicTable::LogicTable(){
  vec2 bittable(1, vec(1, 0));
  cout << "table created" <<endl;
}
LogicTable::~LogicTable(){
  cout << "table deleted" << endl;
}

void LogicTable::add(int width){
  bittable.resize(bittable.size()+1, vec(width, 0));
}

void LogicTable::write(int row, int col, int val){
  bittable[row][col]=val;
}

int LogicTable::read(int row, int col){
  return bittable[row][col];
}

int LogicTable::row_size(){
  return bittable.size();
}

int LogicTable::col_size(int row){
  return bittable[row].size();
}

void LogicTable::read_all(){
  cout << "----- Bit Table ----" << endl;
  for(int i=0; i<row_size(); i++){
    cout << to_string(i) + ": ";
    for(int j=0; j<col_size(i); j++){
      cout << to_string(bittable[i][j]);
    }
    cout << endl;
  }
  cout << "--------------------" << endl;
}

void LogicTable::test(){
  //table
  LogicTable t;
  t.add(2);
  t.add(3);
  t.add(4);
  t.read_all();
  t.write(1, 1, 5);
  t.read_all();
  LogicTable t2;
  t2.add(4);
  t.read_all();
  t2.read_all();
}