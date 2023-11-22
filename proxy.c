#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

void doit(int fd);
int parse_uri(char *uri, char *host, char *port, char *path);

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";
/*
int main() {
  printf("%s", user_agent_hdr);
  return 0;
}
*/
int main(int argc, char **argv) {
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* Check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);  // 프로세스 내 파일 입출력 중인 것 저장 & 프로세스 종료.
  }

  listenfd = Open_listenfd(argv[1]);
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr,
                    &clientlen);  // line:netp:tiny:accept
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,
                0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    doit(connfd);   // line:netp:tiny:doit
    Close(connfd);  // line:netp:tiny:close
  }
}

void doit(int fd){
  struct stat sbuf; // 파일 정보를 담기 위한 구조체
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE], host[MAXLINE], port[MAXLINE], path[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];
  rio_t rio;
  int serverfd;

  /* Read request line and headers */
  // request line
  Rio_readinitb(&rio, fd);  // rio 구조체를 초기화한 후 fd와 rio를 연결
  Rio_readlineb(&rio, buf, MAXLINE); // 한 줄씩(GET https://ipv4:port/path HTTP/1.1) 읽음. 버퍼에 fd를 저장
  
  //request headers
  printf("Request headers:\n");
  printf("%s", buf);  // GET https://ipv4:port/path HTTP/1.1
  sscanf(buf, "%s %s %s", method, uri, version);  // `GET https://ipv4:port/path HTTP/1.1`을 공백 기준으로 데이터 나눠 변수에 저장.

  parse_uri(uri, host, port, path); // uri(https://ipv4:port/path) 파싱 => host(ipv4), port, path(/path)로 구분
	printf("-----------------------------\n");	
	printf("\nClient Request Info : \n");
	printf("method : %s\n", method);
	printf("URI : %s\n", uri);
	printf("hostname : %s\n", host);
	printf("port : %s\n", port);
	printf("path : %s\n", path);
	printf("-----------------------------\n");

  // Request 함수 호출
  serverfd = Open_clientfd(host, port);  // 지정된 서버에 대한 연결 열기
	request(serverfd, host, path);  // 서버 연결에 대해 지정된 파일 serverfd와 호스트 및 경로 정보 함께 사용해 서버에 요청 보냄
	response(serverfd, fd);  // 서버 응답 처리 후 fd 통해 클라이언트로 응답 보냄 
	Close(serverfd); // 서버에 대한 연결 닫기
}

/* `GET uri HTTP/1.0`, host(ip 주소), 헤더 버퍼에 담아 서버로 보내기 */
void request(int serverfd, char *host, char *path){
  char buf[MAXLINE];

  sprintf(buf, "GET %s HTTP/1.0\r\n", host);  // `GET ip HTTP/1.0`을 buf에 씀. (HTTP/1.0 GET request 다루는 문제니까)
  sprintf(buf, "%sHost: %s\r\n", buf, host);
  sprintf(buf, "%s%S\r\n", buf, user_agent_hdr);

  /* 트랜잭션이 끝나 커넥션 끊기 위해 Connection 헤더 명시 */
  sprintf(buf, "%sProxy-Connection: close\r\n", buf);
  sprintf(buf, "%sConnection: close\r\n\r\n", buf); // 헤더 끝

  Rio_writen(serverfd, buf, (size_t)strlen(buf));  // 생성된 요청 메세지 buf에서 strlen(buf)만큼(=> 전부) target_fd로 전송.
}

void response(int serverfd, int fd){
}

int parse_uri(char *uri, char *host, char *port, char *path){  // https://ipv4:port/path 형태를 파싱. ipv4=> host, port=> port, /path=> uri로.
  char *parse_ptr = strstr(uri, "//") ? strstr(uri, "//") + 2 : uri;  // ip 주소 시작 위치에 포인터 놓기 ('https://' 떼기. 'https://' 없이 입력되는 경우 => else문 수행)

  strcpy(host, parse_ptr); // parse_ptr의 전체 문자열을 host에 복사. (ipv4:port/path)

  /* host에서 path 부분 추출 */
  strcpy(path, "/");  // path = '/'. tiny는 '/'가 있어야 home으로 연결되므로 '/' 없으면 에러 발생. 따라서 uri에 path 명시되어있지 않아도 ('/' 없어도) 초기화 해주기.
  parse_ptr = strchr(host, '/');  // /path. '/' 위치 가리킴.
  if(parse_ptr){  // path 있으면
    *parse_ptr = '\0';  // '/'을 널 종결자로 대체. host에서 path 부분 잘림. host를 `ipv4:port/path`에서 `ipv4:port`로 만듦.
    parse_ptr += 1; // '/' 다음 부분
    strcat(path, parse_ptr);
  }

  /* host(ipv4:port)에서 port 부분 추출 */
  parse_ptr = strchr(host, ':');  // /path. ':' 위치 가리킴.
  if(parse_ptr){  // port 있으면
    *parse_ptr = '\0';  // host를 `ipv4:port`에서 `ipv4`로 만듦. (':'부터 컷)
    parse_ptr += 1;
    strcpy(port, parse_ptr);
  } 
  else {  // uri에 지정된(명시적으로 제공된) 포트 없다면
    strcpy(port, "80"); // http 기본 포트인 80 할당
  }

  return 0;
}