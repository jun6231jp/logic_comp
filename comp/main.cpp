#include "logic_table.cpp"
//#include "table_management.cpp"
#include "lib/file_handler.cpp"

int main(int argc, char* argv[]){
  FileHandler f;
  vec2 list;
  int retcode = f.read(argc, argv[1], list);
  //TODO: エラーハンドリング方法を学んだらリファクタする
  if(retcode == 0){
      f.read_all(list);
      LogicTable table;
      
  } else {
      return 1;
  }
  return 0;
}