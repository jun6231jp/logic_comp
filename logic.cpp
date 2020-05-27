#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <omp.h>
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
  int OptPatternLength;
  int OptLineNum;
public:
  LookUpTable()
  {
    LineNum=0;
    PatternLength=0;
  }
  ~LookUpTable(){}
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
  Table(int bitwidth)
  {
    LineNum=0;
    Width=bitwidth;
    vec2 BitTable(1,vec(1,0));
  }
  ~Table(){}
  int add()
  {
    LineNum++;
    BitTable.resize(LineNum,vec(Width,0));
    cout << "line added width:" << Width << endl;
    return LineNum;
  }
  void write(int linenum, int ad , int val){BitTable[linenum][ad]=val;}
  int read(int linenum, int ad){return BitTable[linenum][ad];}
  //int Comp(); //comp_2.comp_nをオーバーロードする
  int SortUniq();
};

class List{
private:
  int TableNum;
public:
  vector<Table> TableList;
  List(){TableNum=0;}
  ~List(){}
  void add()
  {
    TableNum++;
    int width=pow(2,TableNum);
    TableList.resize(TableNum,Table(width));
    cout << "table " << TableNum << "created" << endl;
  }
  void del()
  {
    cout << "table " << TableNum << "deleted" << endl;
    TableNum--;
    int width=pow(2,TableNum);
    TableList.resize(TableNum,Table(width));
  }
  //int DupDel();
  //void Dsp();
};

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
      PatternTable[LineNum-1] = str.substr(0, PatternLength);
      ValueList[LineNum-1] = str.substr(PatternLength + 1);
      while (ValueList[LineNum-1][0]==' ')//パターンと真理値の間の空白が複数あれば切り詰める
        ValueList[LineNum-1] = ValueList[LineNum-1].substr(1);
      if (ValueList[LineNum-1][0]!='0' && ValueList[LineNum-1][0]!='1')//真理値無しであればエラー
        return -1;
      if (ValueList[LineNum-1][1]!='\0' && ValueList[LineNum-1][1]!=' ')//真理値が2文字以上ならエラー
        return -1;
      ValueList[LineNum-1] = ValueList[LineNum-1].substr(0,1);
      PatternNumList.resize(LineNum);
      TruthNumList.resize(LineNum);
      for (int i = 0 ; i < PatternLength ; i++)//パターンを数値変換
        {
          if (PatternTable[LineNum-1][i]=='1')
            PatternNumList[LineNum-1]+=pow(2,i);
          else if (PatternTable[LineNum-1][i]!='0')
            return -1; //0, 1以外の文字が使われていればエラー
        }
      istringstream(ValueList[LineNum-1]) >> TruthNumList[LineNum-1]; //真理値を数値変換　左が小桁
      cout << PatternNumList[LineNum-1] << ":" << TruthNumList[LineNum-1] << endl;
    }
  sort(PatternTable.begin(), PatternTable.end());
  if (unique(PatternTable.begin(), PatternTable.end()) < PatternTable.end())//パターン重複があればエラー
    return -1;
  cout <<  "pattern length : " << PatternLength << endl;
  cout << "table lines : " << LineNum << endl;
  return 0;
}

void LookUpTable::TableOpt()
{
  vec NewPatternNumList;
  vec NewTruthNumList;
  vec2 NewPatternNumListPara;
  vec2 MaskBuff;
  vec MaskBitNumList;
  vec2 BitValPara;
  vec BitVal;
  NewPatternNumListPara.resize(PatternLength);
  BitValPara.resize(PatternLength);
  MaskBuff.resize(PatternLength);
  MaskBitNumList.resize(PatternLength);
  BitVal.resize(LineNum);
#pragma omp parallel for
  for(int m=0 ; m < PatternLength; m++)//削除可能列のチェック開始位置を巡回
    {
      NewPatternNumListPara[m].resize(LineNum);
      BitValPara[m].resize(LineNum);
      MaskBuff[m].resize(PatternLength);
      for(int i = 0; i < LineNum; i++)//パターンと真理値をコピー
        {
          NewPatternNumListPara[m][i]=PatternNumList[i];
        }
      for(int i = m; i < PatternLength + m; i++)
        {
          int id = i;//削除可否チェック列番号
          if (id >= PatternLength)
            id -= PatternLength;
          MaskBuff[m][id]=1;//mask 1が削除可　一旦1を代入しておく
          for(int j = 0; j < LineNum; j++)//真理値表全行に対しチェック
            {
              BitValPara[m][j]=(int)pow(2,id)*((NewPatternNumListPara[m][j]/(int)pow(2,id))%2);
              NewPatternNumListPara[m][j]-=BitValPara[m][j];
              for(int k = 0; k < j; k++)//除外した結果、真理値表で同一パターン有無チェック
                {
                  if(NewPatternNumListPara[m][j]==NewPatternNumListPara[m][k] && TruthNumList[j]!=TruthNumList[k])
                    {
                      MaskBuff[m][id]=0;//同一パターンで真理値が異なれば列削除不可
                      for(int l=0; l <= j ; l++)
                        {
                          NewPatternNumListPara[m][l]+=BitValPara[m][l];
                        }
                      break;
                    }
                }
              if(MaskBuff[m][id]==0)
                {
                  break;
                }
            }
        }
      for(int i=0; i < PatternLength; i++)
        {
          if(MaskBuff[m][i]==1)
            MaskBitNumList[m]++;//削除した列数をカウントして保持
        }
    }
  cout << "mask colmn : " ;
  for(int m=0; m<PatternLength; m++)
    {
      cout  << MaskBitNumList[m] << " " ;
    }
  cout << endl;
  int MaxMask=*max_element(MaskBitNumList.begin(),MaskBitNumList.end());//削除列数最大のマスクを採用
  int MaxAd=0;
  for(MaxAd = 0; MaxAd < PatternLength; MaxAd++)
    {
      if(MaskBitNumList[MaxAd]==MaxMask)
        break;
    }
  TableMask.resize(PatternLength);
  for(int i = 0; i < PatternLength; i++)
    {
      TableMask[i]=MaskBuff[MaxAd][i];
      cout << TableMask[i] ;
    }
  cout << endl;
  NewPatternNumList.resize(LineNum);
  NewTruthNumList.resize(LineNum);
  for(int i = 0; i < LineNum; i++)
    {
      NewPatternNumList[i]=PatternNumList[i];
      NewTruthNumList[i]=TruthNumList[i];
    }
  OptPatternLength=0;
  for(int i = PatternLength-1; i >= 0; i--)
    {
      if(TableMask[i]==1)
        {
          for(int j = 0; j < LineNum; j++)
            {
              int BitVal=(int)pow(2,i)*((NewPatternNumList[j]/(int)pow(2,i))%2);
              int DiffVal=(int)pow(2,i)*(NewPatternNumList[j]/(int)pow(2,i+1));
              NewPatternNumList[j]-=(BitVal+DiffVal);//対象列を除外し、下桁に詰める
            }
        }
      else
        {
          OptPatternLength++;
        }
    }
  for(int j = 0; j < LineNum; j++)
    {
      for(int k = 0; k < j; k++)//除外した結果、真理値表で同一パターン有無チェック
        {
          if(NewPatternNumList[j]==NewPatternNumList[k])//同一パターンなら行削除
            {
              NewTruthNumList[j]=2;
              break;
            }
        }
    }
  OptLineNum=0;
  for(int i = 0 ; i < LineNum; i++)//クラス配列に行削除しながらコピー
    {
      if(NewTruthNumList[i]!=2)
        {
          OptLineNum++;
          OptPatternNumList.resize(OptLineNum);
          OptTruthNumList.resize(OptLineNum);
          OptPatternNumList[OptLineNum-1]=NewPatternNumList[i];
          OptTruthNumList[OptLineNum-1]=NewTruthNumList[i];
          cout << OptPatternNumList[OptLineNum-1] << ":" << OptTruthNumList[OptLineNum-1] << endl;
        }
    }
  cout << "OptPatternLength : " << OptPatternLength << endl;
  cout << "OptLineNum : " << OptLineNum << endl;
}

void Table::SortUniq()
{
  for(int i=0; i < LineNum; i++)
    {
      sort(BitTable[i].begin(),BitTable[i].end());//行方向ソート
    }
  sort(BitTable.begin(),BitTable.end()); //列方向ソート
  BitTable.erase(unique(BitTable.begin(), BitTable.end()), BitTable.end());
  LineNum=BitTable.size();
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
