#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
using std::cout;
using std::endl;
using std::cerr;
using std::string;
using std::ifstream;
using std::to_string;
using std::ios;
using std::istringstream;
using std::stoi;

typedef std::vector<int> vec;
typedef std::vector<vec> vec2;
typedef std::vector<string> vecs;

class FileHandler{
private:
public:
    FileHandler();
    ~FileHandler();
    void split(vec2 &ret, string s);
    int read(int argc, char* filepath, vec2 &t_list);
    void read_all(vec2 list);
    //void write();
};

FileHandler::FileHandler(){
    cout << "new FileHanler" << endl;


}
FileHandler::~FileHandler(){
    cout << "delete FileHandler" << endl;
}

void FileHandler::split(vec2 &ret, string s){
    vec tmp;
    for(int i=0; i<s.size(); i++){
        tmp.push_back(stoi(s.substr(i,1)));
    }
    ret.push_back(tmp);
}

int FileHandler::read(int argc, char* filepath, vec2 &t_list){
    if(argc != 2){
        cerr << "引数の数が間違っています．argc: " << argc << endl;
        return 1;
    }

    ifstream ifs(filepath, ios::in);
    if(!ifs){
        cerr << "Error: file not opened." << endl;
        return 1;
    }

    string str;
    int out;

    while(ifs >> str >> out){ // e.g. str 1000, out 1
        // outが1の行だけリストに入れていく
        if(out == 1){
            split(t_list, str);
        }
    }
    ifs.close();
    return 0;

}

void FileHandler::read_all(vec2 list){
  cout << "----- Bit Table ----" << endl;
  for(int i=0; i<list.size(); i++){
    cout << to_string(i) + ": ";
    for(int j=0; j<list[i].size(); j++){
      cout << to_string(list[i][j]);
    }
    cout << endl;
  }
  cout << "--------------------" << endl;
}

int main(int argc, char* argv[]){
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