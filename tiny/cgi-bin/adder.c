/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "csapp.h"

int main(void) {
  char *buf, *p;
  char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
  int n1 = 0, n2 = 0;
  char *HTTP_method = getenv("REQUEST_METHOD"); // method type

  /* Extract the two arguments */
  if((buf = getenv("QUERY STRING")) != NULL){ // 런타임동안 getenv 함수 사용해 환경변수 값 참조. QUERY STRING은 url의 '?' 문자 뒤에 오는 부분. 따라서 buf에 '?' 뒤의 값, 즉 인자들을 할당.
    p = strchr(buf, '&'); // 각 인자는 '&' 문자로 구분되어 있음. strchr(): '&'로 시작하는 문자열 위치(포인터) 리턴
    *p = '\0';  // 포인터 p가 가리키는 값 '\0'로 바꿈. '\0'은 문자열의 끝을 알릴 때 사용됨. => buf에는 & 앞(첫번째 인자)까지만 남게 됨.
    strcpy(arg1, buf);  // 첫 번째 인자
    strcpy(arg2, p+1);  // 두 번째 인자. p+1은 '&' 뒤의 모든 문자. (문자열 끝나는 곳까지)
    n1 = atoi(arg1);  // atoi(): 문자 스트링을 정수로 변환
    n2 = atoi(arg2);
  }

  /* Make the response body */
  // content는 HTTP 응답에 대해 생성된 콘텐츠가 구성되는 문자열
  sprintf(content, "QUERY_STRING=%s", buf); // printf는 화면에 출력하지만, sprintf는 문자열(두 번째 인자)을 첫 번째 인자로 지정한 문자열(content)에 쓴다.
  sprintf(content, "Welcome to add.com: "); // sprintf 호출은 이전 호출에서 작성한 내용 덮어씀. => 위의 문자열 날라감!
  sprintf(content, "%sTHE Internet addition portal.\r\n<p>", content);  // "%s"로 이전 라인에서 content에 저장된 내용 불러옴
  sprintf(content, "%sThe answer is: %d + %d = %d\r\n<p>",
  content, n1, n2, n1 + n2);  // "%s"로 이전 라인에서 content에 저장된 내용 불러옴
  sprintf(content, "%sThanks for visiting!\r\n", content);  // "%s"로 이전 라인에서 content에 저장된 내용 불러옴 => line 26~30 문자열 다 출력되는 셈.

  /* Generate the HTTP response */
  /* Headers */
  printf("Connection: close\r\n");
  printf("Content-length: %d\r\n", (int)strlen(content));
  printf("Content-type: text/html\r\n\r\n");  // '\r\n\r\n'로 Header와 Body 구분. '\r\n\r\n' 이후 모든 내용은 Body로 간주됨.
  
  if (strcmp(HTTP_method, "GET") == 0){ // HTTP_method가 HEAD일 땐 응답에 body 부분을 포함하지 않음
    /* Body */
    printf("%s", content);  // HTTP 응답에 대해 생성된 문자열을 화면에 출력
  }
  
  fflush(stdout); // stdout(표준 출력 스트림) 관련 출력 버퍼를 비움(flush). => 'stdout' 버퍼의 내용이 출력 장치에 즉시 출력됨. 이게 없으면 printf에 의해 생성된 출력은 여전히 출력 버퍼에 저장되어 화면에 즉시 나타나지 않을 수 있음.
  exit(0);  // exit(): 프로세스 내 파일 입출력 중인 것 저장 & 프로세스 종료. exit(0)은 정상 종료, exit(1)은 에러 메세지 종료.
}
/* $end adder */
