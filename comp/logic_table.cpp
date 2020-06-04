#include <iostream>
#include <vector>
using std::cout;
using std::endl;
using std::to_string;

typedef std::vector<int> vec;
typedef std::vector< vec > vec2;
typedef std::vector< vec2 > vec3;
typedef std::string str;

class LogicTable{
private:
  vec3 qm;
public:
  LogicTable(vec2 list);
  ~LogicTable();
  int num_of_1(vec l);
  void comp();
  void add(int bitwidth);
  int read(int row, int col);
  void read_all();
  void write(int row, int col, int val);
  int row_size();
  int col_size(int row);
  void test();
};

/* LogicTableの中身
  --
  check flag: クワインマクラスキー法で既に圧縮されているか
  1の数: 1000だったら1が1つ
  略号: 積和標準形の1つの最小項目
  最小項: 1000とか最小の項目
  --
  check flag, 1の数, 略号, 最小項
  [
      [0, 3, [0], [1,1,1,0]],
      [0, 4, [1], [1,1,1,1]]
  ]
*/

int LogicTable::num_of_1(vec l){
    int sum = 0;
    for(i=0; i<l.size(); i++){
        if(l[i] == 1){
          sum += 1;
        }
    }
    return sum;
}

LogicTable::LogicTable(vec2 list){
    cout << "new LogicTalbe" <<endl;
    vec3 qm;
    //TODO: 並列化
    for(i=0; i<list.size(); i++){
        vec2 row;
        row.push_back(0);
        row.push_back(num_of_1(list[i]));
        row.push_back([]);
        row.push_back(list[i]);
        qm.push_back(row);
    }
}

LogicTable::~LogicTable(){
    cout << "delete LogicTable" << endl;
}

void LogicTable::sort(vec2 list){
    //1の数を計算してheapに突っ込む
    for(i=0; i<list.size(); i++){
    
    }
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