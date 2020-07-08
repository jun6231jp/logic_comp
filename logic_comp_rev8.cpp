#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <omp.h>
#include <memory>
#include <cstdio>

//#define MTRACE

#ifdef MTRACE
#include <mcheck.h>
#endif

using namespace std;

/*
 * 1. table動的確保　済
 * 2. list動的確保 済
 * 3. スマートポインタで自動管理 済
 * 4. 参照渡しでメモリ削減 済
 * 5. オーバーロード 済
 * 6. 仮想関数 無し
 * 7. 型推論  済
 * 8. インライン関数 済
 * 9. C版以上に高速
 */

typedef unique_ptr< int[] > vec;
typedef unique_ptr< vec[] > vec2;
typedef unique_ptr< vec2[] > vec3;

class LookUpTable{
private:
  int PatternLength = 0;
  int LineNum = 0;
public:
  LookUpTable(){}
  ~LookUpTable(){}
  int OptPatternLength;
  int OptLineNum;
  vector<string> PatternTable;
  vector<string> ValueList;
  vec PatternNumList;
  vec TruthNumList;
  vec TableMask;
  vec OptPatternNumList;
  vec OptTruthNumList;
  vec BitConv;
  int FileRead(char* filename);
  void TableOpt();
  inline int Search(int SearchNum) //対象の数が見つかれば1を返す
  {
    int start=0;
    int end=OptLineNum;
    int current=0;
    for(int i = 0 ; i < OptLineNum ; i++)
      {
        if(current==(start+end)/2)
          return 0;
        current=(start+end)/2;
        if(OptPatternNumList[current]==SearchNum)
          return 1;
        else if(OptPatternNumList[current]<SearchNum)
          start=current;
        else if(OptPatternNumList[current]>SearchNum)
          end=current;
      }
    return 0;
  }
  int Search(vec& NumList, int width, int bit, vec& CompList) //CompListが0のものに対しLUT内にNumListの要素の反転値がすべて見つからなければ1を返す
  {
    for(int i = 0 ; i < width; i++)
      {
        if(CompList[i]==0)
          {
	    int SearchNum = NumList[i]^(1 << bit);
            int start=0;
            int end=OptLineNum;
            int current=0;
            for(int j = 0 ; j < OptLineNum ; j++)
              {
                if(current==(start+end)/2)
		  {
		    CompList[i]=1;
		    break;
		  }
		current=(start+end)/2;
                if(OptPatternNumList[current]==SearchNum)
                  return 0;
                else if(OptPatternNumList[current]<SearchNum)
                  start=current;
                else if(OptPatternNumList[current]>SearchNum)
                  end=current;
              }
          }
      }
    return 1;
  }
};

class Table{
private:
  int Address = 0;
public:
  int Width = 0;
  int LineNum = 0;
  int DelDone = 0;
  vec DelList;
  vec2 BitTable;
  vec2 Buff;
  //bb
  Table(int bitwidth)
  {
    Width=bitwidth;
    BitTable = make_unique < vec[] >(1);
    BitTable[0] = make_unique < int[] >(Width);
  }
  ~Table(){}
  inline int add()
  {
    Buff = make_unique< vec[] > (LineNum+1);
    for(int i = 0 ; i < LineNum; i++)
      Buff[i].swap(BitTable[i]);
    Buff[LineNum]=make_unique< int[] >(Width);
    for(int j = 0 ; j < Width; j++)
      Buff[LineNum][j] = 0;
    LineNum++;
    BitTable = make_unique< vec[] > (LineNum);
    BitTable.swap(Buff);
    return LineNum;
  }
  void write(int val){
    if(Address%Width==0)
      add();
    BitTable[LineNum-1][Address%Width]=val;
    Address++;
  }
  auto comp_idx(int** buff,int* uniq)
  {
    int width = Width;
    return [buff,uniq,width](int l_idx, int r_idx)
	   {
	     for (int i = 0 ; i < width; i ++)
	       {
		 if(buff[l_idx][i] != buff[r_idx][i])
		   return buff[l_idx][i] < buff[r_idx][i];
	       }
	     if(uniq[l_idx]==0)
	       uniq[r_idx]=1;
	     return buff[l_idx][width-1] < buff[r_idx][width-1];
	   };
  }
  void SortUniq ()
  {
    int index[LineNum] = {0};
    int newsize = 0;
    int** buff = new int*[LineNum];
    for (int i = 0 ; i < LineNum ; i++)
      buff[i] = new int[Width];
    int* uniq = new int[LineNum];
    for (int i = 0 ; i < LineNum ; i++)
      sort(&(BitTable[i][0]),&(BitTable[i][Width-1])+1);
    for (int i=0; i < LineNum; i++)
      {
	index[i]=i;
	uniq[i]=0;
	for(int j=0; j < Width; j++)
	  buff[i][j] = BitTable[i][j];
      }
    sort(&(index[0]),&(index[LineNum-1])+1,comp_idx(buff,uniq));
    for (int i = 0 ; i < LineNum ; i++)
      {
	for(int j=0; j < Width; j++)
	  buff[index[i]][j]=BitTable[i][j];
      }
    for (int i = 0 ; i < LineNum ; i++)
      {
	if (uniq[i] == 0)
	  {
	    for(int j=0; j < Width; j++)
	      BitTable[newsize][j]=buff[i][j];
	    newsize++;
	  }
      }
    for (int i = 0 ; i < LineNum ; i++)
      delete [] buff[i];
    delete [] buff;
    delete [] uniq;
    LineNum = newsize;
  }
  int Search(int target, int StartLine=0) //テーブルに対象の数が1つでも見つかれば1を返す
  {
    for(int i = StartLine ; i < LineNum; i++)
      {
        int startcol=0;
        int endcol=Width;
        int currentcol=0;
        for(int j = 0; j < Width; j++)
          {
            if(currentcol==(startcol+endcol)/2)
              break;
            currentcol=(startcol+endcol)/2;
            if(BitTable[i][currentcol]==target)
              return 1;
            else if(BitTable[i][currentcol]<target)
              startcol=currentcol;
            else if(BitTable[i][currentcol]>target)
              endcol=currentcol;
          }
      }
    return 0;
  }
  int Search(int TargetLine, int bit, vec& CompList)//テーブルの対象行の全要素の反転値が同一テーブルから見つかれば1を返しCompListにフラグ格納
  {
    int rtn = 1;
    int StartLine=0;
    for(int i=0; i<Width; i++)
      {
        if(CompList[i]==0)
          {
            int SearchNum = BitTable[TargetLine][i] ^ (1 << bit); 
            StartLine=DupSearch(SearchNum,TargetLine,i,StartLine);
            if(StartLine >= 0)
	      CompList[i]=1;
	    else
	      {
		rtn=0;
		StartLine=0;
	      }
	  }
      }
    return rtn;
  }
  inline int DupSearch(int target, int SelfLine ,int pos=0,int StartLine=0)//テーブル内の自分の行以外から同じ数が見つかれば1を返す
  {
    for(int i = StartLine ; i < LineNum ; i++)
      {
        if(i!=SelfLine)
          {
            int BeginAdd=0;
            if(i < SelfLine)
              BeginAdd=pos;
            for(int j = BeginAdd; j < Width; j++)
              {
		if(BitTable[i][j]>target)
		  break;
		if(BitTable[i][j]==target)
                  return i;
              }
          }
      }
    return -1;
  }
  int DupSearch(int target, int SelfLine, vec& DelList)//対象をDelListが0かつSelLine以外から検索し見つかれば1を返す
  {
    for(int i = 0 ; i < LineNum; i++)
      {
        if(DelList[i]==0 && i != SelfLine)
          {
            int startcol=0;
            int endcol=Width;
            int currentcol=0;
            for(int j = 0; j < Width; j++)
              {
                if(currentcol==(startcol+endcol)/2)
                  break;
                currentcol=(startcol+endcol)/2;
                if(BitTable[i][currentcol]==target)
                  return 1;
                else if(BitTable[i][currentcol]<target)
                  startcol=currentcol;
                else if(BitTable[i][currentcol]>target)
                  endcol=currentcol;
              }
          }
      }
    return 0;
  }
  int CompBit(int TargetLine, int bit)
  {
    int  ChkNum = BitTable[TargetLine][0] & (1 << bit);
    for(int i=1; i<Width; i++)
      {
	if ((BitTable[TargetLine][i] & (1 << bit)) != ChkNum)
	  return 0 ;
      }
    return 1;
  }
};


class List{
public:
  int TableNum = 0;
  LookUpTable LUT;
  
  vector<Table> TableList;
  
  List(){}
  ~List(){cout << "end" << endl;}
  void add()
  {
    int width=1 << TableNum;
    TableNum++;
    //aa
    cout << width << endl;
    //TableList.resize(TableNum,Table(width));
  }
  
  int Comp();
  void DupDel(int TableNo);
  void Dsp();
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
      PatternNumList = make_unique <int[]> (LineNum);
      TruthNumList = make_unique <int[]> (LineNum);
      for (int i = 0 ; i < PatternLength ; i++)//パターンを数値変換
        {
          if (PatternTable[LineNum-1][i]=='1')
            PatternNumList[LineNum-1]+=1<<i;
          else if (PatternTable[LineNum-1][i]!='0')
            return -1; //0, 1以外の文字が使われていればエラー
        }
      istringstream(ValueList[LineNum-1]) >> TruthNumList[LineNum-1]; //真理値を数値変換
    }
  sort(PatternTable.begin(), PatternTable.end());
  if (unique(PatternTable.begin(), PatternTable.end()) < PatternTable.end())//パターン重複があればエラー
    return -1;
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
  NewPatternNumListPara = make_unique <vec[]> (PatternLength);
  BitValPara = make_unique <vec[]> (PatternLength);
  MaskBuff = make_unique <vec[]> (PatternLength);
  MaskBitNumList = make_unique <int[]> (PatternLength);
  BitVal = make_unique <int[]> (PatternLength);
#pragma omp parallel for
  for(int m=0 ; m < PatternLength; m++)//削除可能列のチェック開始位置を巡回
    {
      NewPatternNumListPara[m] = make_unique <int[]> (LineNum);
      BitValPara[m] = make_unique <int[]> (LineNum);
      MaskBuff[m] = make_unique <int[]> (PatternLength);
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
	      BitValPara[m][j]=(1<<id)*((NewPatternNumListPara[m][j]/(1<<id))%2);
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
  int MaxMask=0;//削除列数最大のマスクを採用
  int MaxAd = 0;
  for(int i=0; i < PatternLength; i++)
    {
      if(MaskBitNumList[i] > MaxMask)
	{
	  MaxMask = MaskBitNumList[i];
          MaxAd = i;
	}
    }
  TableMask = make_unique <int[]> (PatternLength);
  cout << "mask : " ;
  for(int i = 0; i < PatternLength; i++)
    {
      TableMask[i]=MaskBuff[MaxAd][i];
      cout << TableMask[i] ;
    }
  cout << endl;
  NewPatternNumList = make_unique <int[]> (LineNum);
  NewTruthNumList = make_unique <int[]> (LineNum);
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
              int BitVal=(1<<i)*((NewPatternNumList[j]/(1<<i))%2);
              int DiffVal=(1<<i)*(NewPatternNumList[j]/(1<<i+1));
              NewPatternNumList[j]-=(BitVal+DiffVal);//対象列を除外し、下桁に詰める
            }
        }
      else
        OptPatternLength++;
    }
  BitConv = make_unique <int[]> (OptPatternLength);//bit位置変換表
  int NewBit = 0;
  for(int i = 0; i < PatternLength; i++)
    {
      if(TableMask[i]==0)
        {
          BitConv[NewBit] = i;
          NewBit++;
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
          OptPatternNumList = make_unique <int[]> (OptLineNum);
          OptTruthNumList = make_unique <int[]> (OptLineNum);
          OptPatternNumList[OptLineNum-1]=NewPatternNumList[i];
          OptTruthNumList[OptLineNum-1]=NewTruthNumList[i];
        }
    }
  vec LutBuff;
  LutBuff = make_unique <int[]> (OptLineNum);
  for(int i=0; i < OptLineNum; i++)//LUTをインデックス付きソートするためパターンと真理値を結合
    LutBuff[i]=OptPatternNumList[i] * 2 + OptTruthNumList[i];
  sort(&(LutBuff[0]),&(LutBuff[OptLineNum-1])+1);
  for(int i=0; i < OptLineNum; i++)//ソートした結合数を再びパターンと真理値に分離
    {
      OptTruthNumList[i]=LutBuff[i]&1;
      OptPatternNumList[i]=LutBuff[i]>>1;
   }
}


int List::Comp()
{
  int OptLineNum=LUT.OptLineNum;
  int OptPatternLength=LUT.OptPatternLength;
  if(TableNum < 1)
    add();
  if(TableNum < 2)//LUTから2グループを抽出
    {
      add();
      int SearchNum = 0;
      int BitVal = 0;
      vec2 CompList;
      vec Forbidden;
      CompList = make_unique <vec[]> (OptLineNum);
      Forbidden = make_unique <int[]> (OptLineNum);
      for(int i = 0; i < OptLineNum; i++)
        {
          CompList[i] = make_unique <int[]> (OptPatternLength);
          if(LUT.OptTruthNumList[i]==1 && Forbidden[i]==0)
            {
              for(int j = 0; j < OptPatternLength; j++)
                {
                  SearchNum = LUT.OptPatternNumList[i] ^ (1<<j);
		  for(int k = 0; k < OptLineNum; k++)
		    {
		      if(LUT.OptPatternNumList[k]==SearchNum && LUT.OptTruthNumList[k]==1)//LUT内にハミング距離1のパターンが存在し真理値が1
                            {
                              CompList[i][j] = 1;
                              Forbidden[k]=1;
                              break;
                            }
		    }
		  if(CompList[i][j] == 0 && !LUT.Search(SearchNum))//dont care
		    CompList[i][j] = 1;
                }
            }
        }
      for(int i = 0; i < OptLineNum; i++)//シリアルで書き込み
        {
          if(LUT.OptTruthNumList[i]==1)
            {
              for(int j = 0; j < OptPatternLength; j++)
                {
                  if (CompList[i][j] == 1)
                    {
                      TableList[1].write(LUT.OptPatternNumList[i]);
                      int HummingNum = LUT.OptPatternNumList[i] ^ (1<<j);
                      TableList[1].write(HummingNum);
                    }
                }
            }
        }
      TableList[1].SortUniq();
      for(int i = 0; i < OptLineNum; i++)//LUT内のパターンでTableList[1]に存在しないものをTableList[0]に登録する
        {
          if(LUT.OptTruthNumList[i]==1)
            {
              if(TableList[1].LineNum == 0 || !TableList[1].Search(LUT.OptPatternNumList[i]))
                TableList[0].write(LUT.OptPatternNumList[i]);
            }
        }
      cout << "Table1 : " << TableList[0].LineNum << endl;
      cout << "Table2 : " << TableList[1].LineNum << endl;
      if (TableList[1].LineNum > 1)
        return 1;
      else
        return 0;
    }
  else //Nグループから2Nグループを抽出
    {
      add();
      int TableLineNum=TableList[TableNum-2].LineNum;
      int TableWidth=TableList[TableNum-2].Width;
      vec3 CompChk;
      vec2 CompList;
      CompList = make_unique <vec[]> (TableLineNum);
      CompChk = make_unique <vec2[]> (TableLineNum);
#pragma omp parallel for //並列でグループ化可能な行を検索
      for(int i = 0 ; i < TableLineNum; i++)
        {
          CompList[i] = make_unique <int[]> (OptPatternLength);
          CompChk[i] = make_unique <vec[]> (OptPatternLength);
          for (int j = 0; j < OptPatternLength; j++)
            {
              CompChk[i][j] = make_unique <int[]> (TableWidth);
              if(TableList[TableNum-2].CompBit(i,j))
                {
		  CompList[i][j]=TableList[TableNum-2].Search(i,j,CompChk[i][j]);
		  if(CompList[i][j]==0)
		    CompList[i][j]=LUT.Search(TableList[TableNum-2].BitTable[i],TableWidth,j,CompChk[i][j]);//dontcareか確認
                }
            }
        }
      for(int i = 0 ; i < TableLineNum; i++)//シリアルで書き込み
        {
          for (int j = 0; j < OptPatternLength; j++)
            {
              if (CompList[i][j]==1)
                {
                  for(int k = 0; k < TableWidth; k++)
                    {
                      int HummingNum = TableList[TableNum-2].BitTable[i][k] ^ (1<<j);
                      TableList[TableNum-1].write(TableList[TableNum-2].BitTable[i][k]);
                      TableList[TableNum-1].write(HummingNum);
                    }
                }
            }
        }
      TableList[TableNum-1].SortUniq();
      cout << "Table" << TableList[TableNum-1].Width << " : " << TableList[TableNum-1].LineNum << endl;
      if (TableList[TableNum-1].LineNum > 1)
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
  int ValidLineNum = 0;
  CompList = make_unique <vec[]> (TableList[TableNo].LineNum);
  TableList[TableNo].DelList = make_unique <int[]> (TableList[TableNo].LineNum);
  TableList[TableNo].DelDone = 1;
#pragma omp parallel for
  for(int i = 0 ; i < TableList[TableNo].LineNum ; i++)
    {
      CompList[i] = make_unique <int[]> (TableList[TableNo].Width);
      TableList[TableNo].DelList[i]=1;
      for(int j = 0 ; j < TableList[TableNo].Width; j++)
        {
          int SearchNum = TableList[TableNo].BitTable[i][j];
          CompList[i][j]=1 - LUT.Search(SearchNum);//LUTに存在しないか確認→削除予定フラグ
          if(CompList[i][j]==0 && TableList[TableNo+1].LineNum > 0)//上位のテーブルに存在するか確認→削除予定フラグ
            CompList[i][j]=TableList[TableNo+1].Search(SearchNum);
          if(CompList[i][j]==0)//行内に削除不可フラグが1つ以上あるか確認→削除不可フラグ
            TableList[TableNo].DelList[i]=0;
        }
    }
  for(int i = 0 ; i < TableList[TableNo].LineNum ; i++)//同位テーブルをシリアルで検索
    {
      if (TableList[TableNo].DelList[i] == 0)
        {
          TableList[TableNo].DelList[i]=1;
          for(int j = 0 ; j < TableList[TableNo].Width; j++)
            {
              int SearchNum = TableList[TableNo].BitTable[i][j];
              if(CompList[i][j]==0)//同位のテーブルに存在するか確認→相手の行が削除不可フラグなら削除予定フラグに変更
                CompList[i][j]=TableList[TableNo].DupSearch(SearchNum,i,TableList[TableNo].DelList);
              if(CompList[i][j]==0)
                {
                  TableList[TableNo].DelList[i]=0;
                  break;
                }
            }
        }
    }
  for(int i = 0 ; i < TableList[TableNo].LineNum ; i++)
    {
      if(TableList[TableNo].DelList[i] == 0)
	ValidLineNum++;
    }
  cout << "Duplication delete from Table" << TableList[TableNo].Width << " : " << ValidLineNum << endl;
}

void List::Dsp()
{
  for(int i = TableNum-1 ; i >= 0 ; i--)
    {
      if (TableList[i].LineNum > 0)
        {
          int w=0;
          for(int j = 0; j < TableList[i].LineNum; j++)
            {
              if (TableList[i].DelDone==0 || TableList[i].DelList[j]==0)
                {
                  if(w > 0)
                    cout << " + " ;
                  for(int k = 0 ; k < LUT.OptPatternLength; k++)
                    {
                      if(TableList[i].CompBit(j,k))
                        {
                          if((TableList[i].BitTable[j][0] & (1<<k))==0)
                            cout << "[`" << LUT.BitConv[k] << "]" ;
                          else
                            cout << "[" << LUT.BitConv[k] << "]" ;
                        }
                    }
                  w++;
                }
            }
          if(w > 0)
            cout << endl;
        }
    }
}

int main (int argc, char* argv[]){
#ifdef MTRACE
  mtrace();
#endif
  List l;
  l.add();
  /*
  if (l.LUT.FileRead(argv[1]))
    {
      cout << "table err" << endl;
      return -1;
    }
  l.LUT.TableOpt();
  while(l.Comp())
    l.DupDel(l.TableNum-2);
  l.Dsp();
  */
#ifdef MTRACE
  muntrace();
#endif
  return 0;
}
