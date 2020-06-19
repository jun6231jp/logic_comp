#include <iostream>
#include <memory>
#include <cstdlib>
#include <exception>
using namespace std;

typedef unique_ptr< int > vec;
typedef unique_ptr< vec > vec2;




int main() {
    unique_ptr<unique_ptr<int[]>[]>  smartPtr2D;
    unique_ptr<unique_ptr<int[]>[]>  smartPtr2D_buff;
    unique_ptr<int[]>     smartPtr1D; 

    smartPtr2D_buff = make_unique< unique_ptr<int[]>[] >(3); 
    for (int i = 0; i<3; i++){ 
        smartPtr1D = make_unique<int[]>(5); 
        for (int j = 0; j<5; j++){ 
         smartPtr1D[j] = j; 
        } 
        smartPtr2D_buff[i] = move(smartPtr1D); 
    }

    smartPtr2D = make_unique< unique_ptr<int[]>[] >(6); 
    for (int i = 0; i<3; i++){
      smartPtr2D[i] = move(smartPtr2D_buff[i]); 
    }

    for (int i = 0; i<3; i++){
        smartPtr1D = make_unique<int[]>(5); 
        for (int j = 0; j<5; j++){ 
         smartPtr1D[j] = 2 * j; 
        } 
        smartPtr2D[i+3] = move(smartPtr1D); 
    }

    for (int i = 0; i<6; i++){
        for (int j = 0; j<5; j++){ 
            cout << smartPtr2D[i][j];
        }
        cout << endl;
    }

    
    return 0;
}
