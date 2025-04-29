#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

#define COMMAND_SIZE 256

//bash의 유저네임과 호스트네임 선언
char username[50];
char hostname[50];
char path[50];

//디렉토리 관련 구조체 변수 선언
DIR* dir;
struct dirent* ent;

//함수 원형 선언
void bash(char* user, char* host);
int command_process(char* str);
void remove_space(char* b); 

//background 변수 선언
int background = 0;

int main(){
	//유저네임과 호스트네임 입력받기
	// 버퍼 오버플로우 문제 대비비
	printf("Enter username:");
	scanf("%s", username);
	printf("Enter hostname:");
	scanf("%s", hostname);

	//본격적인 bash 실행
	bash(username, hostname);
	
	return 0;
}

//리눅스 bash셸의 기본적인 인터페이스 및 명령어 입력받기
void bash(char* user, char* host) {
	char command[COMMAND_SIZE];
	int input_buffer;
	

	//현재 디렉토리를 /(c://)로 설정
	if (chdir("/") != 0) {
		perror("디렉토리 변경 실패");
		return;
	}
	getcwd(path, sizeof (path));

	// 최초 실행 시 버퍼에 남은 개행 제거
	while ((input_buffer = getchar()) != '\n' && input_buffer != EOF);

	//명령어 입력받기 프롬프트
	while (1) {
		//프롬프트 형식
		dir = opendir(".");
		printf("%s@%s:", user, host);
		printf("%s", path);
		printf("$ ");
		if(fgets(command, COMMAND_SIZE, stdin)==NULL) continue;
		// 개행 문자 제거
		command[strcspn(command, "\n")] = '\0';
		
		if(command_process(command) == 2) break;
	}
}
// 문장의 앞 뒤 공백 삭제
void remove_space(char* b) {
    int i=0;
    while(b[i] == ' ') i++;
    if (i > 0) memmove(b, b + i, strlen(b + i) + 1);

	int len = strlen(b);
    while (len > 0 && b[len - 1] == ' ') {
        b[len - 1] = '\0';
        len--;
	}
}
//명령어 수행 함수

/*
	파이프라인 구현은 exec 시스템콜 사용하기
	명령어 수행 성공시 반환값 --> 1
	명령어 수행 실패시 반환값 --> 0
*/

int command_process(char* str) {
	remove_space(str);

	//다중명령어1: ; 구현
	if (strstr(str,";")!=NULL) {
		char* bookmark = strstr(str,";");
		char front_command[COMMAND_SIZE], back_command[COMMAND_SIZE];
		int bookmark_length = bookmark - str;

		strncpy(front_command,str,bookmark_length);
		front_command[bookmark_length] = '\0';
		remove_space(front_command);

		strcpy(back_command,bookmark+1);
		remove_space(back_command);

		command_process(front_command);
		command_process(back_command);

		return 1;
	}
	//문자열 파싱 및 다중명령어 &&, || 구현현
	// else if (strstr(str,"&&") != NULL || strstr(str,"||") != NULL) {
    // 	char token[COMMAND_SIZE][COMMAND_SIZE];
	// 	char result[COMMAND_SIZE] = -1;
    // 	int pars = 0;
    //     char *ptr;
    //     char *start = str;
    //     int len;
        
        
    //     for(int i = 0; i < strlen(str);i++) {
    //         if((str[i] == '&' && str[i+1] == '&') || (str[i] == '|' && str[i+1] == '|')){
    //             ptr = &str[i];
    //             len = ptr - start;
    //             strncpy(token[pars],start,len);
    //             start = ptr + 2;
    //             pars++;
    //             //다중명령어 저장
    //             if(str[i] == '&') strcpy(token[pars],"&&");
    //             else strcpy(token[pars],"||");
    //             pars ++;
    //             //여기까지는 다중명령어 기준 '앞' 명령어 들만 저장하는 코드임. 
    //         }
    //     }
    //     //마지막 다중명령어 기준 '뒤' 명령어 저장
    //     strcpy(token[pars],start);
    //     pars++;

	// 	//반환값 확인을 위해 출력 막기
	// 	fflush(stdout);
	// 	FILE* temp = freopen("/dev/null", "w", stdout);
	// 	//파싱된 명령어들의 수행 반환값 저장 
	// 	for (int i = 0;i<pars;i++) {
	// 		if (token[i] != "&&" && token[i] != "||") result[i] = command_process(token[i]);
	// 		else result[i] = token[i];
	// 	}
	// 	//출력 정상화 
	// 	freopen("/dev/tty", "w", stdout);

	// 	//반환값 토대로 다중명령어 연산 결과 출력 ....
    //}
	// 백그라운드 실행 구현
	else if (strrchr(str,'&')!=NULL){
		char* bg = strrchr(str,'&');
        char bgcmd[50];
		int bg_length = bg - str;

		strncpy(bgcmd,str,bg_length);
		bgcmd[bg_length] = '\0';
		
		background = 1;
		//자식 프로세스 생성
		pid_t pid = fork();


		if (pid < 0) {
			perror("fork 실패");
		}
		else if (pid == 0) {
			// 자식 프로세스: 실제 명령 실행
			command_process(bgcmd);
			exit(0);
		}
		//부모 프로세스 	
		else { 
			if(background) {
				//실행 여부 및 자식 프로세스의 식별 ID 출력
				printf("백그라운드 실행중\t [pid: %d]\n",pid);
				fflush(stdout);
			} else {
				waitpid(pid,NULL,0);
			}
			return 1;
		}
	}

	// exit 구현
	else if (strcmp(str, "exit") == 0) {
		printf("BASH를 종료합니다.\n");
		return 2;
	}
	//pwd 구현
	else if (strcmp(str, "pwd") == 0) {
		printf("%s\n", path);
		return 1;
	}
	//chdir 통한 cd 구현
	else if (strncmp(str, "cd ", 3) == 0) {
		char* change_path = str + 3;
		if (chdir(change_path) != 0) {
			perror("디렉토리 변경 실패");
			return 0;
		}
		else {
			getcwd(path, sizeof(path));
			return 1;
		}

	}
	// ls 구현
	else if (strcmp(str, "ls") == 0) {
		while ((ent = readdir(dir)) != NULL) {
			printf("%s\n", ent->d_name);
			return 1;
		}
	}
	else {
		printf("유효하지 않은 명령어입니다\n");
		return 0;
	}
}


// 문자열 파싱 알고리즘
/*
#include <stdio.h>
#include <string.h>

int main() {
    char str[] = "abcd && EFGH||1234 && adsadsad ||     1283   9032189     ";
    char token[100][100];
    int pars = 0;
    
    if (strstr(str,"&&") != NULL || strstr(str,"||") != NULL) {
        
        char *ptr;
        char *start = str;
        int len;
        
        
        for(int i = 0; i < strlen(str);i++) {
            if((str[i] == '&' && str[i+1] == '&') || (str[i] == '|' && str[i+1] == '|')){
                ptr = &str[i];
                len = ptr - start;
                strncpy(token[pars],start,len);
                start = ptr + 2;
                pars++;
                //다중명령어 저장
                if(str[i] == '&') strcpy(token[pars],"&&");
                else strcpy(token[pars],"||");
                pars ++;
                //여기까지는 다중명령어 기준 '앞' 명령어 들만 저장하는 코드임. 
            }
        }
        //마지막 다중명령어 기준 '뒤' 명령어 저장
        strcpy(token[pars],start);
        pars++;
    }

}
*/