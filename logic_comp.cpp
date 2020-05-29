#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <omp.h>
using namespace std;

/*
 * 1. table動的確保　済
 * 2. list動的確保 済
 * 3. スマートポインタで自動管理
 * 4. 参照渡しでメモリ削減 済
 * 5. オーバーロード 済
 * 6. 仮想関数
 * 7. 型推論
 * 8. インライン関数
 * 9. C版以上に高速
 */

typedef vector< int > vec;
typedef vector< vec > vec2;
typedef vector< vec2 > vec3;

class LookUpTable{
private:
  int PatternLength;
  int LineNum;
public:
  LookUpTable()
  {
    LineNum=0;
    PatternLength=0;
  }
  ~LookUpTable(){}
  int OptPatternLength;
  int OptLineNum;
  int FileRead(char* filename);
  void TableOpt();
  vector<string> PatternTable;
  vector<string> ValueList;
  vec PatternNumList;
  vec TruthNumList;
  vec TableMask;
  vec OptPatternNumList;
  vec OptTruthNumList;
  int Search(vec& NumList, int bit, vec& CompList)
  {
#pragma omp parallel for
    for(int i = 0 ; i < NumList.size(); i++)
      {
	if(CompList[i]==0)
	  {
	    int SearchNum=NumList[i]^(int)pow(2,bit);
	    for(int j = 0 ; j < OptLineNum; j++)
	      {
		if(OptPatternNumList[j] == SearchNum)
		  {
		    CompList[i] = 0;
		    break;
		  }
		else
		  CompList[i] = 1;//見つからなければ1を返す
	      }
	  }
      }
    return *min_element(CompList.begin(),CompList.end());
  }
};

class Table{
private:
  int Address;
public:
  int Width;
  int LineNum;
  vec2 BitTable;
  Table(int bitwidth)
  {
    LineNum = 0;
    Address = 0;
    Width=bitwidth;
    vec2 BitTable(1,vec(1,0));
  }
  ~Table(){}
  int add()
  {
    LineNum++;
    BitTable.resize(LineNum,vec(Width,0));
    return LineNum;
  }
  void write(int val){
    if(Address%Width==0)
        add();
    BitTable[LineNum-1][Address%Width]=val;
    Address++;
  }
  void SortUniq()
  {
#pragma omp parallel for
    for(int i=0; i < LineNum; i++)
      sort(BitTable[i].begin(),BitTable[i].end());//行方向ソート
    sort(BitTable.begin(),BitTable.end()); //列方向ソート
    BitTable.erase(unique(BitTable.begin(), BitTable.end()), BitTable.end());
    LineNum=BitTable.size();
  }
  int Search(int target, int StartLine=0)
  {
    vec SearchRes;
    SearchRes.resize(LineNum);
#pragma omp parallel for
    for(int i = StartLine ; i < LineNum; i++)
      {
        for(int j = 0; j < Width; j++)
          {
	    if(BitTable[i][j]==target)
	      {
		SearchRes[i] = 1;
		break;
	      }
          }
      }
    return *max_element(SearchRes.begin(),SearchRes.end());
  }
  int Search(int TargetLine, int bit, vec& CompList)
  {
#pragma omp parallel for
    for(int i=0; i<Width; i++)
      {
	int SearchNum = BitTable[TargetLine][i] ^ (int)pow(2,bit);
	CompList[i]= Search(SearchNum,TargetLine+1);//下の行から探す
      }
    return *min_element(CompList.begin(),CompList.end());
  }
  int CompBit(int TargetLine, int bit)
  {
    int  ChkNum = BitTable[TargetLine][0] & (int)pow(2,bit);
    for(int i=1; i<Width; i++)
      {
	if ((BitTable[TargetLine][i] & (int)pow(2,bit)) != ChkNum)
	  return 0 ;
      }
    return 1;
  }
  void ReadAll(){
    cout << "----- Bit Table ----" << endl;
    for(int i=0; i<LineNum; i++){
      cout << i << ": ";
      for(int j=0; j<Width; j++){
        cout << BitTable[i][j] << " ";
      }
      cout << endl;
    }
    cout << "--------------------" << endl;
  }
};

class List{
public:
  int TableNum;
  LookUpTable LUT;
  vector<Table> TableList;
  List(){TableNum=0;}
  ~List(){}
  void add()
  {
    int width=pow(2,TableNum);
    TableNum++;
    TableList.resize(TableNum,Table(width));
    cout << "table " << TableNum << "created" << endl;
  }
  int Comp();
  void ReadAllList()
  {
    for(int i=0; i<TableNum; i++)
      {
        TableList[i].ReadAll();
      }
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
      if (PatternLength!=0 && PatternLength!=str.find(' '))//パターン長が一定で なければエラー
        return -1;
      LineNum++;
      PatternLength=str.find(' ');
      PatternTable.resize(LineNum);
      ValueList.resize(LineNum);
      PatternTable[LineNum-1] = str.substr(0, PatternLength);
      ValueList[LineNum-1] = str.substr(PatternLength + 1);
      while (ValueList[LineNum-1][0]==' ')//パターンと真理値の間の空白が複数あれば切り詰める
        ValueList[LineNum-1] = ValueList[LineNum-1].substr(1);
      if (ValueList[LineNum-1][0]!='0' && ValueList[LineNum-1][0]!='1')//真理値 無しであればエラー
        return -1;
      if (ValueList[LineNum-1][1]!='\0' && ValueList[LineNum-1][1]!=' ')//真理値が2文字以上ならエラー
        return -1;
      ValueList[LineNum-1] = ValueList[LineNum-1].substr(0,1);
      PatternNumList.resize(LineNum);
      TruthNumList.resize(LineNum);
      for (int i = 0 ; i < PatternLength ; i++)//パターンを数値変換 左から小桁
        {
          if (PatternTable[LineNum-1][i]=='1')
            PatternNumList[LineNum-1]+=pow(2,i);
          else if (PatternTable[LineNum-1][i]!='0')
            return -1; //0, 1以外の文字が使われていればエラー
        }
      istringstream(ValueList[LineNum-1]) >> TruthNumList[LineNum-1]; //真理値を数値変換
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
        NewPatternNumListPara[m][i]=PatternNumList[i];
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
                        NewPatternNumListPara[m][l]+=BitValPara[m][l];
                      break;
                    }
                }
              if(MaskBuff[m][id]==0)
                break;
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
    cout  << MaskBitNumList[m] << " " ;
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
              NewPatternNumList[j]-=(BitVal+DiffVal);//対象列を除外し、下桁に詰 める
            }
        }
      else
        OptPatternLength++;
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

int List::Comp()
{
  if(TableNum < 1)
    add();
  if(TableNum < 2)//LUTから2グループを抽出
    {
      add();
      int SearchNum = 0;
      int BitVal = 0;
      vec2 CompList;
      vec Forbidden;
      CompList.resize(LUT.OptLineNum);
      Forbidden.resize(LUT.OptLineNum);
      for(int i = 0; i < LUT.OptLineNum; i++)
	CompList[i].resize(LUT.OptPatternLength);
#pragma omp parallel for  //並列でグループ化可能な行を検索
      for(int i = 0; i < LUT.OptLineNum; i++)
        {
          if(LUT.OptTruthNumList[i]==1 && Forbidden[i]==0)
            {
              CompList[i].resize(LUT.OptPatternLength);
              for(int j = 0; j < LUT.OptPatternLength; j++)
                {
		  SearchNum = LUT.OptPatternNumList[i] ^ (int)pow(2,j);
		  
		  if(find(LUT.OptPatternNumList.begin(),LUT.OptPatternNumList.end(),SearchNum)==LUT.OptPatternNumList.end())//dont care
		    {
		      CompList[i][j] = 1;
		      Forbidden[i]=1;
		    }
		    else
                    {
		      for(int k = 0; k < LUT.OptLineNum; k++)
                        {
                          if(LUT.OptPatternNumList[k]==SearchNum && LUT.OptTruthNumList[k]==1)//LUT内にハミング距離1のパターンが存在し真理値が1
                            {
                              CompList[i][j] = 1;
			      Forbidden[k]=1;
                              Forbidden[i]=1;
                              break;
                            }
                        }
		    }
                }
            }
        }
      for(int i = 0; i < LUT.OptLineNum; i++)//シリアルで書き込み
        {
          if(LUT.OptTruthNumList[i]==1)
            {
              for(int j = 0; j < LUT.OptPatternLength; j++)
                {
                  if (CompList[i][j] == 1)
                    {
                      TableList[1].write(LUT.OptPatternNumList[i]);
		      int HummingNum = LUT.OptPatternNumList[i] ^ (int)pow(2,j);
                      TableList[1].write(HummingNum);
                    }
                }
            }
        }
      TableList[1].SortUniq();
      for(int i = 0; i < LUT.OptLineNum; i++)//LUT内のパターンでTableList[1]に存在しないものをTableList[0]に登録する
        {
          if(LUT.OptTruthNumList[i]==1)
            {
              if(!TableList[1].Search(LUT.OptPatternNumList[i]))
                TableList[0].write(LUT.OptPatternNumList[i]);
            }
        }
      if (TableList[1].LineNum > 2)
        return 1;
      else
        return 0;
    }
  else //Nグループから2Nグループを抽出
    {
      add();
      vec3 CompChk; //行数、ビット反転位置、要素数
      vec2 CompList;//行数、ビット反転位置
      CompList.resize(TableList[TableNum-2].LineNum);
      CompChk.resize(TableList[TableNum-2].LineNum);
#pragma omp parallel for //並列でグループ化可能な行を検索
      for(int i = 0 ; i < TableList[TableNum-2].LineNum; i++)
        {
          CompList[i].resize(LUT.OptPatternLength);
	  CompChk[i].resize(LUT.OptPatternLength);
          for (int j = 0; j < LUT.OptPatternLength; j++)
            {
	      CompChk[i][j].resize(TableList[TableNum-2].Width);
	      if(TableList[TableNum-2].CompBit(i,j))//全要素で共通のビットか確認
		{
		  CompList[i][j]=TableList[TableNum-2].Search(i,j,CompChk[i][j]); //行内の全要素のペアがテーブル内の対象行より下行にあるか確認
		  if(CompList[i][j]==0)
		    CompList[i][j]=LUT.Search(TableList[TableNum-2].BitTable[i],j,CompChk[i][j]);//dontcareか確認
		}
	    }
        }
      for(int i = 0 ; i < TableList[TableNum-2].LineNum; i++)//シリアルで書き込み
        {
          for (int j = 0; j < LUT.OptPatternLength; j++)
            {
              if (CompList[i][j]==1)
                {
                  for(int k = 0; k < TableList[TableNum-2].Width; k++)
                    {
		      int HummingNum = TableList[TableNum-2].BitTable[i][k] ^ (int)pow(2,j);
                      TableList[TableNum-1].write(TableList[TableNum-2].BitTable[i][k]);
                      TableList[TableNum-1].write(HummingNum);
                    }
                }
            }
        }
      TableList[TableNum-1].SortUniq();
      if (TableList[TableNum-1].LineNum > 2)
        //DupDel();
        return 1;
      else
        return 0;
    }
}

int main (int argc, char* argv[]){
  List l;
  if (l.LUT.FileRead(argv[1]))
    {
      cout << "table err" << endl;
      return -1;
    }
  l.LUT.TableOpt();
  
  while(l.Comp())
    {

    }
  
  //l.Comp();
  //l.Comp();
  l.ReadAllList();
  return 0;
}
