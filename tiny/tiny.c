/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
#include "csapp.h"

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg);

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
  int is_static;  // 정적 파일 구분 변수
  struct stat sbuf; // 파일 정보를 담기 위한 구조체
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];
  rio_t rio;

  //printf("fd number:%d\n", fd);

  /* Read request line and headers */
  // request line
  Rio_readinitb(&rio, fd);  // rio 구조체를 초기화한 후 fd와 rio를 연결
  Rio_readlineb(&rio, buf, MAXLINE); //버퍼에 fd를 저장
  
  //request headers
  printf("Request headers:\n");
  printf("%s", buf);  // GET / HTTP/1.1

  //sscanf : string scanf 로, 문자를 추출해서 데이터를 변수에 저장
  sscanf(buf, "%s %s %s", method, uri, version);  // 파싱하여 정보 추출. http 메서드(= GET), uri(= HTTP), version(= 1.1)

  if(strcasecmp(method, "GET")){
    clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method");
    return;
  }

  read_requesthdrs(&rio); // 요청 헤더를 읽어들이는 함수 호출
  printf("requsthdhdhdhdhdhdhdhdhdhdhdhdhd%s\n", buf);

  /* Parse URI from GET request */
  is_static = parse_uri(uri, filename, cgiargs);

  if (stat(filename, &sbuf) < 0) {
    clienterror(fd, filename, "404", "Not found", "Tiny couldn’t find this file");
    return;
  }

  if (is_static) { /* Serve static content */
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn’t read the file");
      return;
    }
    serve_static(fd, filename, sbuf.st_size);
  }
  else { /* Serve dynamic content */
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn’t run the CGI program");
      return;
    }
    serve_dynamic(fd, filename, cgiargs);
  }
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
  char buf[MAXLINE], body[MAXBUF];

  /* Build the HTTP response body */
  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);
  
  /* Print the HTTP response */
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));
  Rio_writen(fd, body, strlen(body));
}

void read_requesthdrs(rio_t *rp)
{
  char buf[MAXLINE];

  Rio_readlineb(rp, buf, MAXLINE);
  printf("gugugugugu%s", buf);
  while(strcmp(buf, "\r\n")) { //공백 두개인 경우까지 반복문 수행
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
  }
  return;
}

int parse_uri(char *uri, char *filename, char *cgiargs)
{
  char *ptr;

  if(!strstr(uri, "cgi-bin")) { //uri에 cgibin 없으면 -> 정적
    strcpy(cgiargs, "");
    strcpy(filename, ".");
    strcat(filename, uri);
    if(uri[strlen(uri)-1] == '/'){
      strcat(filename, "home.html");
    }
    return 1;
  }
  else{ // cgibin 있으면 -> 동적
    ptr = index(uri, "?");
    if (ptr) {
      strcpy(cgiargs, ptr+1);
      *ptr = "\0";
    }
    else
      strcpy(cgiargs, "");

    strcpy(filename, ".");
    strcat(filename, uri);
    return 0;
  }
}

void serve_static(int fd, char *filename, int filesize)
{
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];

  /* Send response headers to client */
  get_filetype(filename, filetype);
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
  sprintf(buf, "%sConnection: close\r\n", buf);
  sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
  sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
  Rio_writen(fd, buf, strlen(buf));
  printf("Response headers:\n");
  printf("%s", buf);

  /* Send response body to client */
  /*
  srcfd = Open(filename, O_RDONLY, 0);  // 파일 열고 성공시 srcfd에 파일 디스크립터 반환 
  // Mmap(): 메모리 매핑 함수. 커널에 새 가상 메모리 영역 생성을 요청. 파일을 메모리에 매핑=> 파일을 메모리로 읽거나 쓰게 함.
  srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0); // srcp: 매핑된 메모리 영역의 시작 주소 할당되는 포인터
  Close(srcfd); // 파일 더이상 필요 없을 때 파일 닫음 => 시스템 자원 확보
  Rio_writen(fd, srcp, filesize); // srcp가 가리키는 메모리 위치의 데이터를 fd가 나타내는 파일에 기록.
  Munmap(srcp, filesize); // 매핑 해제 (더이상 매핑 필요없을 때)
  */

 // Mmap=> malloc, Munmap=> free 로 대체
  srcfd = Open(filename, O_RDONLY, 0);
  srcp = (char *)malloc(filesize);  // filesize 크기의 버퍼 srcp에 대해 메모리 동적 할당. mmap과 달리 메모리 공간 할당하기만 함.
  Rio_readn(srcfd, srcp, filesize); // 포인터 매핑. srcfd에서 데이터 읽고 이를 srcp가 가리키는 메모리 버퍼에 저장.
  Close(srcfd); // 열린 파일과 관련 리소스 해제 (srcp에 데이터 저장해서 이제 srcfd 필요 없음)
  Rio_writen(fd, srcp, filesize); // srcp 버퍼에 저장된 데이터를 fd가 나타내는 다른 파일이나 소켓에 기록.
  free(srcp); // 메모리 할당 해제
}

 /*
 * get_filetype - Derive file type from filename
 */
void get_filetype(char *filename, char *filetype) // filename으로 전달된 파일 이름을 기반으로 filetype에 해당 파일의 MIME 유형을 복사
{
  if (strstr(filename, ".html"))  // strstr(): 첫 번째 인자로 전달된 문자열에서 두 번째 인자로 전달된 부분 문자열이 있는지 검사.
    strcpy(filetype, "text/html");
  else if (strstr(filename, ".mpg"))  // filename에 ".mpg"있는지 검사
    strcpy(filetype, "video/mpg");  // 있으면 filetype에 "video/mpg" 덮어 씌움
  else if (strstr(filename, ".gif"))
    strcpy(filetype, "image/gif");
  else if (strstr(filename, ".png"))
    strcpy(filetype, "image/png");
  else if (strstr(filename, ".jpg"))
    strcpy(filetype, "image/jpeg");
  else
    strcpy(filetype, "text/plain");
}

void serve_dynamic(int fd, char *filename, char *cgiargs)
{
  char buf[MAXLINE], *emptylist[] = { NULL };

  /* Return first part of HTTP response */
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Server: Tiny Web Server\r\n");
  Rio_writen(fd, buf, strlen(buf));

  if (Fork() == 0) { /* Child */
    /* Real server would set all CGI vars here */
    setenv("QUERY_STRING", cgiargs, 1);
    Dup2(fd, STDOUT_FILENO); /* Redirect stdout to client */
    Execve(filename, emptylist, environ); /* Run CGI program */
  }
  Wait(NULL); /* Parent waits for and reaps child */
}