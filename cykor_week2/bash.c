#include <stdio.h>
#include <string.h>

//bash�� �������Ӱ� ȣ��Ʈ���� ����
char username[50];
char hostname[50];
char prompt[50][50];

void bash(char* user, char* host);

int main(){
	//�������Ӱ� ȣ��Ʈ���� �Է¹ޱ�
	printf("Enter username:");
	scanf_s("%s", username,50u);
	printf("Enter hostname:");
	scanf_s("%s", hostname,50u);

	//������Ʈ�� �������Ӱ� ȣ��Ʈ���� �߰�
	sprintf_s(prompt[0], sizeof(prompt[0]), "%s", username);
	sprintf_s(prompt[1], sizeof(prompt[1]), "%s", username);


	//�������� bash ����
	bash(username, hostname);

	return 0;
}

//������ bash���� �⺻���� �������̽� �� ��ɾ� �Է¹ޱ�
void bash(char* user, char* host) {
	char command[50];
	while (1) {
		printf("%s@%s:/", user, host);
		//....
		printf("$ ");
		scanf_s("%s",command,50);
		printf("%s\n", command);

	}
}
