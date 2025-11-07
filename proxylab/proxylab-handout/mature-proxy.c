#include <stdio.h>
#include "csapp.h"
#include <pthread.h>
#include <sys/select.h>

/* 推荐的最大缓存和对象大小 */
#define THREAD_POOL_SIZE 10
#define MAX_OBJECT_SIZE 102400

/* 包含这行长行不会丢失代码风格分数 */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

/* 服务器信息结构体 */
typedef struct {
  char serverHost[MAXLINE];
  char serverPort[10];
  char fileName[MAXLINE];
} UriInfo;

/* select使用的连接池结构 */
typedef struct {
    int maxfd;                    /* 读集合中最大的文件描述符 */   
    fd_set read_set;              /* 所有活跃描述符的集合 */
    fd_set ready_set;             /* 准备好读取的描述符子集 */
    int nready;                   /* select返回的就绪描述符数量 */
    int maxi;                     /* 客户端数组的高水位标记 */
    int clientfd[FD_SETSIZE];     /* 活跃客户端连接集合 */
    rio_t clientrio[FD_SETSIZE];  /* 活跃读取缓冲区集合 */
} pool;

/* 线程池任务结构 */
typedef struct {
    int clientfd;
    struct sockaddr_in client_addr;
} task_t;

/* 全局变量 */
pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;
task_t task_queue[THREAD_POOL_SIZE];  // 改为静态数组，不是指针数组
int queue_size = 0;

/* 函数原型声明 */
void doIt(int clientfd);
void readRequestHdrs(rio_t *rp);
UriInfo parseUri(char *uri);
void clientError(int fd, char *cause, char *errNum, char *shortMsg, char *loneMsg);
void forwardRequest(int serverfd, char *serverHost, char *fileName, char *method);
void acceptReply(int serverfd, int clientfd);
void sigchld_handler(int sig);

/* Select I/O复用函数 */
void init_pool(int listenfd, pool *p);
void add_client(int connfd, pool *p);
void check_clients(pool *p);

/* 工作线程函数 */
void *worker_thread(void *arg) {
    while (1) {
        pthread_mutex_lock(&queue_lock);
        while (queue_size == 0) {
            pthread_cond_wait(&queue_cond, &queue_lock);
        }
        
        /* 从队列中取任务 */
        task_t task = task_queue[0];
        /* 移动队列（简单实现） */
        for (int i = 0; i < queue_size - 1; i++) {
            task_queue[i] = task_queue[i + 1];
        }
        queue_size--;
        pthread_mutex_unlock(&queue_lock);
        
        /* 处理请求 */
        printf("工作线程处理连接\n");
        doIt(task.clientfd);
        Close(task.clientfd);
    }
    return NULL;
}

/* 初始化连接池 */
void init_pool(int listenfd, pool *p) {
    /* 初始时没有已连接描述符 */
    p->maxi = -1;
    for (int i = 0; i < FD_SETSIZE; i++) {
        p->clientfd[i] = -1;
    }
    
    /* 初始时listenfd是select读集合的唯一成员 */
    p->maxfd = listenfd;
    FD_ZERO(&p->read_set);
    FD_SET(listenfd, &p->read_set);
}

int main(int argc, char **argv)
{
    int listenfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    static pool pool;
    pthread_t worker_tids[THREAD_POOL_SIZE];
    
    /* 检查命令行参数 */
    if (argc != 2) {
        fprintf(stderr, "用法: %s <端口>\n", argv[0]);
        exit(1);
    }
    
    /* 初始化线程池 */
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        Pthread_create(&worker_tids[i], NULL, worker_thread, NULL);
    }
    
    listenfd = Open_listenfd(argv[1]);
    init_pool(listenfd, &pool);
    
    printf("代理服务器监听端口 %s\n", argv[1]);
    
    while (1) {
        pool.ready_set = pool.read_set;
        pool.nready = Select(pool.maxfd + 1, &pool.ready_set, NULL, NULL, NULL);
        
        /* 处理新连接 */
        if (FD_ISSET(listenfd, &pool.ready_set)) {
            clientlen = sizeof(struct sockaddr_storage);
            int connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
            
            /* 将任务加入队列 */
            pthread_mutex_lock(&queue_lock);
            if (queue_size < THREAD_POOL_SIZE) {
                task_queue[queue_size].clientfd = connfd;
                /* 注意：这里需要类型转换，但更安全的方式是复制数据 */
                memcpy(&task_queue[queue_size].client_addr, &clientaddr, sizeof(struct sockaddr_in));
                queue_size++;
                pthread_cond_signal(&queue_cond);
                printf("新连接加入队列，队列大小: %d\n", queue_size);
            } else {
                /* 队列满，拒绝连接 */
                printf("队列已满，拒绝连接\n");
                Close(connfd);
            }
            pthread_mutex_unlock(&queue_lock);
        }
    }
    
    return 0;
}

/*
 * 向服务器发送请求，并向客户端回复
 */
void doIt(int clientfd) {
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    UriInfo info;
    rio_t rio;
    
    /* 读取请求行和头部 */
    Rio_readinitb(&rio, clientfd);
    if (Rio_readlineb(&rio, buf, MAXLINE) <= 0) {
        return;  /* 读取失败 */
    }

    printf("请求头部:\n");
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    
    if (strcasecmp(method, "GET")) {
        clientError(clientfd, method, "501", "未实现",
                   "代理未实现此方法");
        return;
    }
    
    readRequestHdrs(&rio);

    /* 从GET请求解析URI */
    info = parseUri(uri);
    if (strlen(info.serverHost) == 0) {
        clientError(clientfd, uri, "400", "错误请求",
                   "无法解析URI");
        return;
    }

    printf("连接到 %s:%s 获取 %s\n", info.serverHost, info.serverPort, info.fileName);
    
    int serverfd = Open_clientfd(info.serverHost, info.serverPort);
    if (serverfd < 0) {
        clientError(clientfd, info.serverHost, "502", "错误网关",
                   "代理无法连接到终端服务器");
        return;
    }

    forwardRequest(serverfd, info.serverHost, info.fileName, method);
    acceptReply(serverfd, clientfd);
    Close(serverfd);
}

void readRequestHdrs(rio_t *rp) {
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    while(strcmp(buf, "\r\n")) {
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
    return;
}

/*
 * 从URI解析服务器主机、端口、文件名（打包到结构体中）
 */
UriInfo parseUri(char *uri) {
    UriInfo info;
    char *ptr, *hostHeadPtr, *fileName, *portPtr;
    int hostLen;  
    
    /* 初始化默认值 */
    strcpy(info.serverPort, "80");
    strcpy(info.fileName, "/");
    info.serverHost[0] = '\0';
    
    /* 检查是否是完整URL（包含 ://） */
    ptr = strstr(uri, "://");
    if (ptr != NULL) {
        /* 完整URL格式: http://host:port/path - 这是正常的代理请求 */
        ptr += 3; /* 忽略 "://" */
        hostHeadPtr = (char *) ptr;
        fileName = strstr(ptr, "/");
        
        /* 获取文件名（或文件路径） */
        if (fileName != NULL) {
            strcpy(info.fileName, fileName);
            hostLen = fileName - hostHeadPtr;
        } else {
            info.fileName[0] = '/';
            info.fileName[1] = '\0';
            hostLen = strlen(hostHeadPtr);
        }

        /* 获取主机:端口 */
        char hostPort[hostLen + 1];
        strncpy(hostPort, hostHeadPtr, hostLen);
        hostPort[hostLen] = '\0';

        /* 获取端口 */
        portPtr = strstr(hostPort, ":");
        if (portPtr != NULL) {
            *portPtr = '\0';
            strcpy(info.serverHost, hostPort);
            strcpy(info.serverPort, portPtr + 1);/* 忽略 ':' */  
        } else {
            strcpy(info.serverHost, hostPort); /* 如果没有 ":端口"，则hostPort只是主机名 */
        }
        
        printf("完整URL模式: 主机=%s, 端口=%s, 文件=%s\n", 
               info.serverHost, info.serverPort, info.fileName);
    } else {
        /* 相对路径格式: /path - 这是浏览器直接访问代理的情况 */
        /* 这种情况下，我们转发到默认的测试服务器(Tiny) */
        strcpy(info.serverHost, "localhost");
        strcpy(info.serverPort, "8000");
        strcpy(info.fileName, uri);
        
        /* 确保文件名以 / 开头 */
        if (info.fileName[0] != '/') {
            char temp[MAXLINE];
            strcpy(temp, "/");
            strcat(temp, info.fileName);
            strcpy(info.fileName, temp);
        }
        
        printf("相对路径模式: 转发到默认服务器 %s:%s, 文件=%s\n", 
               info.serverHost, info.serverPort, info.fileName);
    }
    
    return info;
}

/*
 * clienterror - 向客户端返回错误消息
 */
void clientError(int fd, char *cause, char *errnum, 
                char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE];

    /* 打印HTTP响应头部 */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /* 打印HTTP响应体 */
    sprintf(buf, "<html><title>Tiny Error</title>");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em>The Tiny Web server</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
}

/*
 * 转发请求到服务器
 */
void forwardRequest(int serverfd, char *hostName, char *fileName, char *method) {
    char buf[MAXLINE];
    /* 构造转发头部 */
    sprintf(buf, "%s %s HTTP/1.0\r\n", method, fileName);
    sprintf(buf + strlen(buf), "Host: %s\r\n", hostName);
    sprintf(buf + strlen(buf), "%s", user_agent_hdr);
    sprintf(buf + strlen(buf), "Proxy-Connection: close\r\n");
    sprintf(buf + strlen(buf), "\r\n");  

    Rio_writen(serverfd, buf, strlen(buf));       
}

/*
 * 从服务器读取每一行，并发送回客户端
 */
void acceptReply(int serverfd, int clientfd) {
    rio_t rio;
    char buf[MAXLINE];
    ssize_t n;

    Rio_readinitb(&rio, serverfd);
    
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) > 0) {
        Rio_writen(clientfd, buf, n);
    }
}

void sigchld_handler(int sig) {
    while (waitpid(-1, 0, WNOHANG) > 0)
        ;
    return;
}
