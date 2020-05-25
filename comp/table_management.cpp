#include <iostream>
#include <vector>
#include "logic_table.cpp"
using std::cout;
using std::endl;
using std::to_string;

typedef std::vector<int> vec;
typedef std::vector< vec > vec2;
typedef std::string str;
typedef std::vector<LogicTable> table;

class TableManagement{
private:
  vec2 bittable;
public:
  TableManagement();
  ~TableManagement();
  void add(int bitwidth);
  int read(int row, int col);
  void read_all();
  void write(int row, int col, int val);
  int row_size();
  int col_size(int row);
};

TableManagement::TableManagement(){
  vec2 bittable(1,vec(1,0));
  cout << "table created" <<endl;
}
TableManagement::~TableManagement(){
  cout << "table deleted" << endl;
}

void TableManagement::add(int width){
  bittable.resize(bittable.size()+1, vec(width, 0));
}

void TableManagement::write(int row, int col, int val){
  bittable[row][col]=val;
}

int TableManagement::read(int row,int col){
  return bittable[row][col];
}

int TableManagement::row_size(){
  return bittable.size();
}

int TableManagement::col_size(int row){
  return bittable[row].size();
}

void TableManagement::read_all(){
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

int main (int argc, const char* argv[]){
  TableManagement t;
  // add
  t.add(2);
  t.add(3);
  t.add(4);
  t.read_all();
  t.write(1, 1, 5);
  t.read_all();
  TableManagement t2;
  t2.add(4);
  t.read_all();
  t2.read_all();
  return 0;
}