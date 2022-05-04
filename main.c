#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/signal.h>
#include <sys/resource.h>

char varr[15][15];
char value[15][30];
int arrIndex = 0;
char *att[];
int childCounter = 1;

void handleChildExit(int pid){
    int wstat;
    while (1) {
        pid = wait3(&wstat, WNOHANG, NULL);
        if (pid == 0 || pid == -1)
            return;
        printf("background child %d terminated %d \n",childCounter,wstat);
        writeTextFile();
    }
}

void writeTextFile(){
    // we use text file in w mode to overwrite existed data or create new file if no file found
    FILE *file = fopen("login.txt","w");
    fprintf(file,"Child Process %d terminated\n",childCounter);
    childCounter++;
    fclose(file);
}

void setEnv(){
    chdir("..");
    printf("we are on home .. directory\n");
}

void shell(char str[50][100],int noWords){
    if(strcmp(str[0],"echo") == 0 || strcmp(str[0],"cd") == 0 || strcmp(str[0],"export") == 0)
        executeBuiltInCommand(str,noWords);
    else
        executeCommand(str,noWords);
}

void executeBuiltInCommand(char str[50][100],int noWords){
        if(str[1][0] == '$'){
            char checkVarr[15];
            int foundIndex = 0;
            memset(checkVarr,0,15);
            for(int i = 0; i < strlen(str[1]) - 1; i++)
                checkVarr[i] = str[1][i+1];
            // search for environment varriable in varr array
            for(int i = 0; i < arrIndex; i++){
                if(strcmp(checkVarr,varr[i]) == 0){
                    foundIndex = i;
                    break;
                }
            }
            memset(str[1],0,100);
            for(int i = 0; i < strlen(value[foundIndex]); i++)
                str[1][i] = value[foundIndex][i];

        }
        // echo command
        if(strcmp(str[0],"echo") == 0){
            printf("%s",str[1]);
        }
        // cd command
        else if(strcmp(str[0],"cd") == 0){
            chdir(str[1]);
        }
        // export command
        else if(strcmp(str[0],"export") == 0){
            int i = 0,j = 0,z = 0,tempArrIndex;
            // tempArrIndex hold index where we find varriable if found
            tempArrIndex = arrIndex;
            char tempVarr[15];
            memset(tempVarr,0,15);
            // we check existence of varriable to update it else we create another one
            while(str[1][z] != '='){
                 tempVarr[z] = str[1][z];
                 z++;
            }
            for(int i = 0; i < arrIndex; i++){
                if(strcmp(tempVarr,varr[i]) == 0){
                    // index existed varriable so we clear its value to update it
                    tempArrIndex = i;
                    arrIndex--;
                    break;
                }
            }
            i = 0;
            j = 0;
            // we update or add new varriable according to value of tempArrIndex
            memset(varr[tempArrIndex],0,15);
            while(str[1][i] != '='){
                varr[tempArrIndex][i] = str[1][i];
                i++;
            }
            i++;
            memset(value[tempArrIndex],0,30);
            while(i < strlen(str[1])){
                value[tempArrIndex][j] = str[1][i];
                i++;
                j++;
            }
            arrIndex++;
        }
}


void executeCommand(char str[50][100],int noWords){

    // we check for & to determine background or foreground
    // we adjust flag and remove & if it is background
    int backGroundFlag = 0;
    if(str[noWords-1][strlen(str[noWords-1])-1] == '&'){
        backGroundFlag = 1;
        str[noWords-1][strlen(str[noWords-1])-1] = NULL;
    }
    else
        backGroundFlag = 0;

    // handle environment varriable $varr
    for(int z = 0; z < noWords; z++){
        if(str[z][0] == '$'){
            char checkVarr[15];
            int foundIndex = 0;
            memset(checkVarr,0,15);
            for(int i = 0; i < strlen(str[1]) - 1; i++)
                checkVarr[i] = str[z][i+1];
            // search for environment varriable in varr array
            for(int i = 0; i < arrIndex; i++){
                if(strcmp(checkVarr,varr[i]) == 0){
                    foundIndex = i;
                    break;
                }
            }
            memset(str[z],0,100);
            for(int i = 0; i < strlen(value[foundIndex]); i++)
                str[z][i] = value[foundIndex][i];

        }
    }

    //check attributes
    char attTemp[15][30];
    memset(attTemp[0],0,30);
    int characterCounter = 0;
    for(int i = 0; i < strlen(str[1]); i++){
        if(str[1][i] != ' ')
            attTemp[characterCounter][i] = str[1][i];
        else{
            characterCounter++;
            memset(attTemp[characterCounter],0,30);
        }
    }

    for(int i = 0; i <= characterCounter; i++){
        memset(str[i+1],0,100);
        for(int j = 0; j < strlen(attTemp[i]); j++){
            str[i+1][j] = attTemp[i][j];
        }
    }
    int id = fork();
    if(id == 0){
        // child process
        // we convert 2d char array to array of char pointers to pass it to execvp function
        int noWordsTemp = noWords;
        if(backGroundFlag == 1)
            noWordsTemp--;
        for(int i = 0; i < noWordsTemp; i++)
            att[i] = &str[i];
        att[noWordsTemp] = NULL;
        execvp(att[0],att);
        printf("Error");
        exit(0);

    }
    else{
        // parent process
        if(backGroundFlag == 0)
            // foreground wait for child
            wait();
    }
}

int main(int args, char* argv[]){

    signal (SIGCHLD, handleChildExit);
    setEnv();

    char str[20][100];
    char str2[100];


    while(1){

        int j = 0, k = 0, noWords = 1, nextFlag = 1;
        printf("\n");
        // clear arrays and variables
        memset(str2,0,100);
        for(int i = 0; i < 20; i++)
            memset(str[i],0,100);
        fflush(stdout);
        gets(str2);

        // count number words
        for(int i = 0; i < strlen(str2); i++){
            if(i != strlen(str2)-1){
                if(str2[i] == ' ' && str2[i+1] != ' ')
                    noWords++;
                else if(str2[i] == '"'){
                    while(str2[i+1] != '"'){
                        i++;
                    }
                    i++;
                }
            }
        }

        // parse string
        for(int i = 0; i < strlen(str2); i++){
            if(str2[i] != ' '){
                if(str2[i] == '"'){
                    i++;
                    while(str2[i] != '"'){
                        str[j][k] = str2[i];
                        k++;
                        i++;
                    }
                    i++;
                    k++;
                }
                str[j][k] = str2[i];
                k++;
                nextFlag = 1;
            }
            else if(nextFlag == 1){
                j++;
                k = 0;
                nextFlag = 0;
            }
        }
        // after parsing check exit command
        if(strcmp(str[0],"exit") == 0)
            exit(0);
        else
            shell(str,noWords);
    }

    return 0;
}
