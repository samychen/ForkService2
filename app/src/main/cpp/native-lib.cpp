#include <jni.h>
#include <string>
#include <sys/select.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/un.h>
#include <errno.h>
#include <stdlib.h>
#include <linux/signal.h>
#include <android/log.h>
#include <unistd.h>
#include <sys/socket.h>
#define LOG_TAG    "samychen：Fork"
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG, __VA_ARGS__)

//子进程有权限访问父进程的私有目录,在此建立跨进程通信的套接字文件
static const char* PATH = "/data/data/com.samychen.gracefulwrapper.forkservice/my.sock";
int m_child;
const char *pid;
/**
 * 创建服务端socket
 * 连接服务器(子进程)
 */
int child_create_channel(){
    int sockfd = socket(AF_LOCAL,SOCK_STREAM,0);
    if (sockfd < 0) {
        LOGD("<<Parent create channel failed>>");
        return -1;
    }
    int connfd = 0;
    struct sockaddr_un addr;//结构体的第一个参数协议，第二个参数路径
    //把之前链接清空
    unlink(PATH);
    memset(&addr, 0, sizeof(sockaddr_un));//清空内存
    addr.sun_family = AF_LOCAL;//协议和上面相同
    strcpy(addr.sun_path, PATH);//路径(端口号),char不能直接赋值，通过内存拷贝函数
    if (bind(sockfd, (const sockaddr *) &addr, sizeof(sockaddr_un))<0){
        LOGD("绑定错误");
        return 0;
    }
    //监听，等待客户端链接
    listen(sockfd,5);//最大监听多少客户端软件，像BAT系列可以设置大些
    //保证宿主进程链接成功，因为有时候会链接失败
    while (1){
        //返回客户端的地址，阻塞式函数
        if ((connfd=accept(sockfd,NULL,NULL)) < 0){
            if (errno==EINTR){//内置成员变量
                continue;
            } else {
                LOGD("链接失败");
                return 0;
            }
        }
        m_child = connfd;
        LOGD("父进程链接上了%d",m_child);
        break;
    }
    return 1;
}
/**
 * 服务端读取信息
 * @return
 */
void child_listen_msg(){
    fd_set rfds;
    struct timeval timeout = {3,0};
    while (1){
        //清空内容
        FD_ZERO(&rfds);
        FD_SET(m_child,&rfds);
        //选择监听
        //4个客户端，一般设置为+1
        int r = select(m_child+1,&rfds,NULL,NULL,&timeout);//linux函数，阻塞式函数
        LOGD("读取消息前%d",r);
        if (r>0){
            char buffer[256] = {0};
            //保证所读取到的信息是指定apk客户端
            if(FD_ISSET(m_child,&rfds)){
                //发生在子进程
                //阻塞函数，读什么没关系，因为一旦断开链接(apk不在了)，就不再阻塞，read方法继续往下执行
                int result = read(m_child,buffer, sizeof(buffer));
                //开启服务
                execlp("am","am","startservice","--user",pid,"com.samychen.gracefulwrapper.forkservice/com.samychen.gracefulwrapper.forkservice.ForkService",(char *)NULL);
                break;
            }
        }

    }
}
/**
 *
 */
void child_do_work() {
    //开启socket
    if (child_create_channel) {
        child_listen_msg();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_samychen_gracefulwrapper_forkservice_Watcher_createWatcher(JNIEnv *env, jobject instance,
                                                                    jstring pid_) {
    pid = env->GetStringUTFChars(pid_, 0);
    //开启双进程
    pid_t  ppid = fork();//fork()下面的代码都会执行两遍，因为有父子进程
    if (ppid < 0) {
        //失败
    } else if (ppid == 0) {
        //子进程(守护进程)
        child_do_work();
    } else {
        //父进程
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_samychen_gracefulwrapper_forkservice_Watcher_connectMonitor(JNIEnv *env,
                                                                     jobject instance) {
    //发生在父进程(客户端 apk是父进程)
    int socked;
    while (1){
        LOGD("客户端 父进程开始链接");
        socked = socket(AF_LOCAL,SOCK_SEQPACKET,0);
        if (socked<0){
            LOGD("链接失败");
            return;
        }
        struct sockaddr_un addr;//结构体的第一个参数协议，第二个参数路径
        memset(&addr, 0, sizeof(addr));//清空内存
        addr.sun_family = AF_LOCAL;//协议和上面相同
        strcpy(addr.sun_path, PATH);
        //客户端没有写任何日志，避免不断轮询
        if (connect(socked, (const sockaddr *) &addr, sizeof(addr) < 0)){
            LOGD("链接失败");
            close(socked);
            sleep(1);
            //再来下一次，直到成功
            continue;
        }
        LOGD("链接成功");
    }
}