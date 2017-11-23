#include "syscall.h"
#define M 20
#define N 20
int num[M][N];
int main(){
	int i,j;
	for(j = 0; j<N;j++){
		for(i=0;i<M;i++){
			num[i][j] = 0;
		}
	}
		
		//Halt();
}