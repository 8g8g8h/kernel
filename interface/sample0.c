#include<stdio.h>
#include<setjmp.h>

void func(jmp_buf env){
    printf("func() in\n");
    //バッファからレジスタ情報を復元している
    longjmp(env,1);
    printf("func() out\n");
}

int main(){
    jmp_buf env;

    printf("main() in\n");
    //ここで、レジスタ情報をenv(レジスタ保存先のバッファ)に保存,保存後には0を返すので保存している。一方でlongjmpから戻ってくるとき1などを返すようになるのでここでスルーしている
    if(setjmp(env)==0){
        printf("setjmp() return0\n");
        func(env);
    }
    printf("setjmp() return 0\n");
    printf("main() out\n");

    return 0;
}