#include <stdio.h>
#include "csapp.h"
#include "cache.h"

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

void* thread(void* vargp);
void doit(int fd);
void read_requesthdrs(rio_t *rp, int clientfd, char* new_package);
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg);

int main(int argc, char** argv)
{
	int listenfd, *connfdp;
	pthread_t tid;
	socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    if(argc != 2) {
    	fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(1);
    }

    signal(SIGPIPE, SIG_IGN);  // 忽略 SIGPIPE 信号
    init_cache();
    listenfd = Open_listenfd(argv[1]);
    while(1) {
    	clientlen = sizeof(clientaddr);
    	connfdp = Malloc(sizeof(int));
		*connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen);
		Pthread_create(&tid, NULL, thread, connfdp);
    }
    return 0;
}

/*
 * thread - new request correspond to new thread to communicate
 */
void* thread(void* vargp) {
    //printf("----- start a thread to deal with request -----.\n");
	int connfd = *((int*)vargp);
	Pthread_detach(pthread_self());
	Free(vargp);
	doit(connfd);
	Close(connfd);
	return NULL;
}


/*
 * doit - handle one HTTP request/response transaction
 */
void doit(int fd) {
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char hostname[MAXLINE], port[MAXLINE], path[MAXLINE];
    char new_package[MAXLINE], response[MAXLINE];
    int clientfd, object_size;
    ssize_t length;
	rio_t rio, client_rio;


	Rio_readinitb(&rio, fd);
    if (!Rio_readlineb(&rio, buf, MAXLINE))  //line:netp:doit:readrequest
        return;
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);       //line:netp:doit:parserequest
    if (strcasecmp(method, "GET")) {                     //line:netp:doit:beginrequesterr
        clienterror(fd, method, "501", "Not Implemented",
                    "Proxy does not implement this method");
        return;
    }
    // 解析得到hostname port page
    if(sscanf(uri, "http://%99[^:]:%[^/]/%199[^\n]", hostname, port, path) != 3) {
        clienterror(fd, uri, "400", "Bad Request",
                    "Bad Request");
        return;
    }

    /*
     * read from cache
     */
    char* cache_buf_r;
    if((cache_buf_r = read_from_cache(uri)) != NULL) {
        // response to client
        Rio_writen(fd, cache_buf_r, strlen(cache_buf_r));
        return;
    }


    // printf("hostname = %s\n", hostname);
    // printf("port = %s\n", port);
    // connect to server
    clientfd = Open_clientfd(hostname, port);
    if(clientfd == -1) {
        clienterror(fd, uri, "503", "Service Unavailable",
                    "The server is unavailable");
        return;
    }
    Rio_readinitb(&client_rio, clientfd);

    // 构建发送给server的package
    sprintf(new_package, "GET /%s HTTP/1.0\r\n", path);
    Rio_writen(clientfd, new_package, strlen(new_package));
    // rio是client与proxy的交流入口, proxy从rio读取client的request
    // clientfd是server与proxy的交流入口, proxy往clientfd里写处理后的request
    read_requesthdrs(&rio, clientfd, new_package);
    
    // 从client_rio接受response,并直接返回给client
    // 这里因为file很大，需要多次read
    // 增加cache功能
    object_size = 0;
    char cache_buf[MAX_OBJECT_SIZE];
    char* cache_buf_ptr = cache_buf;
    memset(cache_buf, 0, MAX_OBJECT_SIZE);
    while((length = Rio_readnb(&client_rio, response, MAXLINE)) > 0) {
        Rio_writen(fd, response, length);
        object_size += length;
        strcpy(cache_buf_ptr, response);
        cache_buf_ptr += length;
    }
    if(object_size < MAX_OBJECT_SIZE) {
        write_into_cache(uri, cache_buf);
    }
    Close(clientfd);
}


/*
 * read_requesthdrs - read HTTP request headers
 */
void read_requesthdrs(rio_t *rp, int clientfd, char* new_package) 
{
    char buf[MAXLINE];

    do{
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
        if(strstr(buf, "User-Agent")) {
            sprintf(new_package, user_agent_hdr);
        }else if(strstr(buf, "Host")) {
            sprintf(new_package, buf);
        }else if(strstr(buf, "Proxy-Connection")) {
            sprintf(new_package, "Connection: close\r\n");
            Rio_writen(clientfd, new_package, strlen(new_package));
            sprintf(new_package, "Proxy-Connection: close\r\n");
        }else{
            sprintf(new_package, buf);
        }
        Rio_writen(clientfd, new_package, strlen(new_package));
    }while(strcmp(buf, "\r\n"));

    return;
}

/*
 * clienterror - returns an error message to the client
 */
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE];

    /* Print the HTTP response headers */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /* Print the HTTP response body */
    sprintf(buf, "<html><title>Client Error</title>");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em>The Web server</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
}




















