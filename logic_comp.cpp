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
  int Search(int SearchNum) //対象の数が見つかれば1を返す
  {
    for(int i = 0; i < OptLineNum; i++)
      {
	if(OptPatternNumList[i] == SearchNum)
	  return 1;
      }
    return 0;
  }
  int Search(vec& NumList, int bit, vec& CompList) //LUT内にNumListの要素の反転値がすべて見つからなければ1を返す
  {
    //#pragma omp parallel for
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
		  CompList[i] = 1;
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
  int read(int line , int ad)
  {
    return BitTable[line][ad];
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
  int Search(int target, int StartLine=0) //テーブルに対象の数が1つでも見つかれば1を返す
  {
    vec SearchRes;
    SearchRes.resize(LineNum);
    //#pragma omp parallel for
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
  int DupSearch(int target, int SelfLine)//テーブル内の自分の行以外から同じ数が見つかれば1を返す
  {
    vec SearchRes;
    SearchRes.resize(LineNum);
    //#pragma omp parallel for
    for(int i = 0 ; i < LineNum; i++)
      {
	if(i != SelfLine)
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
      }
    return *max_element(SearchRes.begin(),SearchRes.end());
  }
  int DupSearch(int target, int SelfLine, vec& DelList)//対象をDelListが0かつSelLine以外から検索し見つかれば1を返す
  {
    vec SearchRes;
    SearchRes.resize(LineNum);
#pragma omp parallel for
    for(int i = 0 ; i < LineNum; i++)
      {
        if(i != SelfLine && DelList[i]==0)
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
      }
    return *max_element(SearchRes.begin(),SearchRes.end());
  }
  int Search(int TargetLine, int bit, vec& CompList)//テーブルの対象行の全要素の反転値が同一テーブルから見つかれば1を返す
  {
    //#pragma omp parallel for
    for(int i=0; i<Width; i++)
      {
	int SearchNum = BitTable[TargetLine][i] ^ (int)pow(2,bit);
	CompList[i]=DupSearch(SearchNum,TargetLine);
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
  void LineDel(vec& DelList)
  {
    vec2 buff;
    int NewLineNum = 0;
    buff.resize(LineNum);
    for(int i = 0; i < LineNum; i++)
      {
	buff[i].resize(Width);
	if (DelList[i] == 0)
	  {
	    for(int j = 0; j < Width; j++)
	      {
		buff[NewLineNum][j] = BitTable[i][j];
	      }
	    NewLineNum++;
	  }
      }
    BitTable.resize(NewLineNum);
    for(int i = 0; i < NewLineNum; i++)
      {
	BitTable[i].resize(Width);
	for(int j = 0; j < Width; j++)
	  BitTable[i][j]=buff[i][j];
      }
    LineNum = NewLineNum;
    cout << "new line num " << LineNum << endl;
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
    cout << "table" << TableNum << " width:" << width << " created" << endl;
  }
  int Comp();
  void ReadAllList()
  {
    for(int i=0; i<TableNum; i++)
      {
        TableList[i].ReadAll();
      }
  }
  void DupDel(int TableNo);
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
      for (int i = 0 ; i < PatternLength ; i++)//パターンを数値変換
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
        {
	  CompList[i].resize(LUT.OptPatternLength);
          if(LUT.OptTruthNumList[i]==1 && Forbidden[i]==0)
	    {
	      for(int j = 0; j < LUT.OptPatternLength; j++)
		{
		  SearchNum = LUT.OptPatternNumList[i] ^ (int)pow(2,j);
		  if(find(LUT.OptPatternNumList.begin(),LUT.OptPatternNumList.end(),SearchNum)==LUT.OptPatternNumList.end())//dont care
		    {
		      CompList[i][j] = 1;
		    }
		  else
		    {
		      for(int k = 0; k < LUT.OptLineNum; k++)
			{
			  if(LUT.OptPatternNumList[k]==SearchNum && LUT.OptTruthNumList[k]==1)//LUT内にハミング距離1のパターンが存在し真理値が1
			    {
			      CompList[i][j] = 1;
			      Forbidden[k]=1;
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
          cout << "Table1 : " << TableList[0].LineNum << endl;
          cout << "Table2 : " << TableList[1].LineNum << endl;
      for(int i = 0; i < LUT.OptLineNum; i++)//LUT内のパターンでTableList[1]に存在しないものをTableList[0]に登録する
        {
          if(LUT.OptTruthNumList[i]==1)
            {
              if(TableList[1].LineNum == 0 || !TableList[1].Search(LUT.OptPatternNumList[i]))
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
	      if(TableList[TableNum-2].CompBit(i,j))
		{
		  CompList[i][j]=TableList[TableNum-2].Search(i,j,CompChk[i][j]);
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
      cout << "Lines:" << TableList[TableNum-1].LineNum << endl;
      if (TableList[TableNum-1].LineNum > 2)
	  return 1;
      else
	{
	  DupDel(TableNum-2);
	  return 0;
	}
    }
}

void List::DupDel(int TableNo)
{
  vec2 CompList;
  vec DelList;
  int reduction = 0;
  CompList.resize(TableList[TableNo].LineNum);
  DelList.resize(TableList[TableNo].LineNum);
#pragma omp parallel for 
  for(int i = 0 ; i < TableList[TableNo].LineNum ; i++)
    {
      CompList[i].resize(TableList[TableNo].Width);
      for(int j = 0 ; j < TableList[TableNo].Width; j++)
	{
	  int SearchNum = TableList[TableNo].read(i,j);
	  //LUTに存在しないか確認→削除予定フラグ
	  CompList[i][j]=1 - LUT.Search(SearchNum);
	  //上位のテーブルに存在するか確認→削除予定フラグ
	  if(CompList[i][j]==0 && TableList[TableNo+1].LineNum > 0)
	    CompList[i][j]=TableList[TableNo+1].Search(SearchNum);
	}
      //行内の要素すべてに削除フラグがあるものを検索→削除実行フラグ
      DelList[i]=*min_element(CompList[i].begin(),CompList[i].end());
    }
  //同位テーブルはシリアルで検索
  for(int i = 0 ; i < TableList[TableNo].LineNum ; i++)
    {
      if (DelList[i] == 0)
	{
	  for(int j = 0 ; j < TableList[TableNo].Width; j++)
	    {
	      int SearchNum = TableList[TableNo].read(i,j);
	      //同位のテーブルに存在するか確認→相手の行が削除不可フラグなら削除予定フラグに変更
	      if(CompList[i][j]==0)
		CompList[i][j]=TableList[TableNo].DupSearch(SearchNum,i,DelList);
	    }
	  DelList[i]=*min_element(CompList[i].begin(),CompList[i].end());
	}
    }
    cout << "Duplication deleting" << endl;
    TableList[TableNo].LineDel(DelList);
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
    l.DupDel(l.TableNum-2);
  l.ReadAllList();
  return 0;
}
