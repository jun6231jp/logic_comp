#include <iostream>

class List{
public:
  int TableNum;
  LookUpTable LUT;
  vector<Table> TableList;
  List(){TableNum=0;}
  ~List(){cout << "end" << endl;}
  void add()
  {
    int width=1 << TableNum;
    TableNum++;
    TableList.resize(TableNum,Table(width));
  }
  int Comp();
  void DupDel(int TableNo);
  void Dsp();
};
