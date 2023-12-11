#include <iostream>
#include "../hrtf/hrtf.h"

// using namespace std;

float myArray[3][3] = { {1,2,3}, {4,5,6}, {7,8,9} };

int main(){
for (int i=0;i<10; i++){
    for (int j=0;j<10; j++){
        for (int k=0;k<10; k++){
            std::cout << i*j*k << "   " << lhrtf[i][j][k] << "\n";
            };
        };
    };
};
