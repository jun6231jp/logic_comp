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
   int PatternLength;
   int LineNum;
   int OptLineNum;
public:
   LookUpTable();
   ~LookUpTable();
   int FileRead(char* filename);
   void TableOpt();
   vector<string> PatternTable;
   vector<string> ValueList;
   vec PatternNumList;
   vec TruthNumList;
   vec TableMask;
   vec OptPatternNumList;
   vec OptTruthNumList;
};
class Table{
private:
   int LineNum;
   int Width;
public:
   vec2 BitTable;
   Table(int bitwidth);
   ~Table();
   int add();
   void write(int linenum, int ad , int val);
   int read(int linenum, int ad);
};
class List{
private:
  int TableNum;
public:
  vector<Table> TableList;
  List();
  ~List();
  void add();
  void del();
};

LookUpTable::LookUpTable()
{
 cout << "LUT construct" << endl;
 LineNum=0;
 PatternLength=0;
}
LookUpTable::~LookUpTable()
{
 cout << "LUT destruct" << endl;
}
int LookUpTable::FileRead(char* filename)
{
  ifstream ifs(filename);
  string str;
  if (ifs.fail())
    return -1;
  while (getline(ifs, str))
  {
    while (str[0]==' ')//先頭のスペース削除
      str = str.substr(1);
    if (str[0]=='\0') //空行無視
      continue;
    if (PatternLength!=0 && PatternLength!=str.find(' '))//パターン長が一定でなければエラー
      return -1;
    LineNum++;
    PatternLength=str.find(' ');
    PatternTable.resize(LineNum);
    ValueList.resize(LineNum);
    PatternNumList.resize(LineNum);
    TruthNumList.resize(LineNum);
    PatternTable[LineNum-1] = str.substr(0, PatternLength);
    ValueList[LineNum-1] = str.substr(PatternLength + 1);
    while (ValueList[LineNum-1][0]==' ')//パターンと真理値の間の空白が複数あれば切り詰める
        ValueList[LineNum-1] = ValueList[LineNum-1].substr(1);
    if (ValueList[LineNum-1][0]!='0' && ValueList[LineNum-1][0]!='1')//真理値無しであればエラー
       return -1;
    if (ValueList[LineNum-1][1]!='\0' && ValueList[LineNum-1][1]!=' ')//真理値が2文字以上ならエラー
       return -1;
    ValueList[LineNum-1] = ValueList[LineNum-1].substr(0,1);
    //パターンを数値変換
    for (int i = 0 ; i < PatternLength ; i++)
    {
      if (PatternTable[LineNum-1][i]=='1')
        PatternNumList[LineNum-1]+=pow(2,i);
      else if (PatternTable[LineNum-1][i]!='0')
       return -1; //0, 1以外の文字が使われていればエラー
    }
    //真理値を数値変換
    istringstream(ValueList[LineNum-1]) >> TruthNumList[LineNum-1];
    cout << PatternNumList[LineNum-1] << ":" << TruthNumList[LineNum-1] << endl;
  }
  //パターン重複があればエラー
  sort(PatternTable.begin(), PatternTable.end());
  if (unique(PatternTable.begin(), PatternTable.end()) < PatternTable.end())
    return -1;
  cout <<  "pattern length : " << PatternLength << endl;
  cout << "table lines : " << LineNum << endl;
  return 0;
}
void LookUpTable::TableOpt()
{
  vec NewPatternNumList;
  vec NewTruthNumList;
  vec MaskBuff;
  vec MaskBitNumList;
  NewPatternNumList.resize(LineNum);
  NewTruthNumList.resize(LineNum);
  MaskBuff.resize(PatternLength); 
  MaskBitNumList.resize(PatternLength);
 for(int m=0 ; m < PatternLength; m++)
 {
  for(int i = 0; i < LineNum; i++)
  {
    NewPatternNumList[i]=PatternNumList[i];
    NewTruthNumList[i]=TruthNumList[i];
  }
  for(int i = m; i < PatternLength + m; i++)
  {
    int id = i;
    if (id >= PatternLength)
      id -= PatternLength;
    MaskBuff[id]=1;
    for(int j = 0; j < LineNum; j++)
    { 
      NewPatternNumList[j]=NewPatternNumList[j]%(int)pow(2,id) + NewPatternNumList[j]/(int)pow(2,(id+1));
      for(int k = 0; k < j; k++)
      {
        if(NewPatternNumList[j]==NewPatternNumList[k] && NewTruthNumList[j]==NewTruthNumList[k])
        {
          NewTruthNumList[j]=2;
          break;
        }  
        else if(NewPatternNumList[j]==NewPatternNumList[k] && NewTruthNumList[j]!=NewTruthNumList[k] && NewTruthNumList[j]!=2)          {
          MaskBuff[id]=0;
          for(int l=0; l <= j ; l++)
          {
            NewPatternNumList[j]=NewPatternNumList[j] + (int)pow(2,id);
          }
          break; 
        }     
      }
      if(MaskBuff[id]==0)
      {
        break;
      }
    } 
  }
  for(int i=0; i < PatternLength; i++)
  {
    if(MaskBuff[i]==1)
      MaskBitNumList[m]++;
  }
  cout << "mask:" << MaskBitNumList[m] << endl;  
 }
}

List::List()
{
 cout << "list construct" << endl;
 TableNum=0;
}

List::~List()
{
 cout << "list destruct" << endl;
}

void List::add()
{
 TableNum++;
 int width=pow(2,TableNum);
 TableList.resize(TableNum,Table(width));
 cout << "table " << TableNum << "created" << endl;
}

void List::del()
{
 cout << "table " << TableNum << "deleted" << endl;
 TableNum--;
 int width=pow(2,TableNum);
 TableList.resize(TableNum,Table(width));
}

Table::Table(int bitwidth)
{
  LineNum=0;
  Width=bitwidth;
  vec2 BitTable(1,vec(1,0));
  cout << "table construct" << endl;
}
Table::~Table()
{
  cout << "table destruct" << endl;
}

int Table::add()
{
  LineNum++;
  BitTable.resize(LineNum,vec(Width,0));
  cout << "line added width:" << Width << endl;
  return LineNum;
}

void Table::write(int line, int ad, int val)
{
  BitTable[line][ad]=val;
}

int Table::read(int line,int ad)
{
  return BitTable[line][ad];
}

int main (int argc, char* argv[]){
    LookUpTable lut;
    if (lut.FileRead(argv[1])!=0)
    {
      cout << "table err" << endl;
      return -1;
    }
    lut.TableOpt();
    List l;
  
    return 0;
}
