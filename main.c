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
	scanf("%49s", username);
	printf("Enter hostname:");
	scanf("%49s", hostname);

	//본격적인 bash 실행
	bash(username, hostname);
	
	return 0;
}

//리눅스 bash셸의 기본적인 인터페이스 및 명령어 입력받기
void bash(char* user, char* host) {
	char command[COMMAND_SIZE];
	int input_buffer;
	

	//현재 디렉토리를 /로 설정
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
	//문자열 파싱 및 다중명령어 &&, || 구현
	else if (strstr(str, "||") != NULL || strstr(str, "&&") != NULL) {
    	char *commands[COMMAND_SIZE];
    	int num_commands = 0;
    	int success = 0;  

    	// && 연산자 우선 분리
    	char *token = strtok(str, "&&");
    	while (token != NULL && num_commands < COMMAND_SIZE) {
        	commands[num_commands++] = token;
        	token = strtok(NULL, "&&");
    	}

    	for (int i = 0; i < num_commands; i++) {
        	char *sub_commands[COMMAND_SIZE];
        	int sub_num = 0;

        	// || 연산자 분리 (strtok 로직에 따라 || 없을시 '그대로' 토큰에 저장!)
        	token = strtok(commands[i], "||");
        	while (token != NULL && sub_num < COMMAND_SIZE) {
            	sub_commands[sub_num++] = token;
            	token = strtok(NULL, "||");
        	}

        	for (int j = 0; j < sub_num; j++) {
            	success = command_process(sub_commands[j]); 

            	// OR 연산자 하나라도 성공시 탈출
            	if (success) break;
        	}

        	// AND에서 실패시 전체 중단
        	if (!success) break;
    	}
	}
		// 파이프라인 구현
	else if (strstr(str,"|")!= NULL) {
		char *token = strtok(str, "|");
		char *commands[COMMAND_SIZE];
		int pipe_fd[COMMAND_SIZE - 1][2];
		int num_commands = 0;

		while (token != NULL && num_commands < COMMAND_SIZE) {
    		commands[num_commands++] = token;
    		token = strtok(NULL, "|");
		}

		// 개수만큼 파이프 생성
		for (int i = 0; i < num_commands - 1; i++) {
    		if (pipe(pipe_fd[i]) == -1) {
        		perror("pipe");
        		exit(0);
    		}
		}

		
		for (int i = 0; i < num_commands; i++) {
    		pid_t pid = fork();
    		if (pid == -1) {
        		perror("fork");
        		exit(0);
    		}

    		if (pid == 0) {  
				// 이전 프로세스에서 입력 받기
        		if (i > 0) { 
            		dup2(pipe_fd[i - 1][0], STDIN_FILENO);
            		close(pipe_fd[i - 1][0]);
        		}
				// 다음 프로세스로 출력 보내기
        		if (i < num_commands - 1) { 
            		dup2(pipe_fd[i][1], STDOUT_FILENO);
            		close(pipe_fd[i][1]);
        		}

        // 파일 디스크립터 닫기
        		for (int j = 0; j < num_commands - 1; j++) {
            		close(pipe_fd[j][0]);
            		close(pipe_fd[j][1]);
        		}
				
				// 각 명령어 실행
        		char *args[] = {"/bin/sh", "-c", commands[i], NULL};
        		execvp(args[0], args);
        		perror("execvp");
        		exit(0);
    		}
		}

		// 부모 프로세스에서 파일 디스크립터 닫고 기다리기기
		for (int i = 0; i < num_commands - 1; i++) {
    		close(pipe_fd[i][0]);
    		close(pipe_fd[i][1]);
		}

		for (int i = 0; i < num_commands; i++) {
    		wait(NULL);
		}

		return 1;
	}
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
		}
		return 1;
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

	else {  // 이 외의 단일 명령어 실행
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            return -1;
        }

        if (pid == 0) {  // 자식 프로세스
            execlp("/bin/sh", "sh", "-c", str, NULL);  // 단일 명령어 직접 실행
            perror("execlp");
            exit(0);
        } else {
            waitpid(pid,NULL,0);  // 자식 프로세스 종료 대기
            return 1;
        }
    }
	
	return 0;

}


