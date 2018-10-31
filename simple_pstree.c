#include <linux/netlink.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define NETLINK_USER 31

#define MAX_PAYLOAD 10000 /* maximum payload size*/
struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
int sock_fd;
struct msghdr msg;
char input[20];
int pid;
int current_pid;

int main(int argc,char  *argv[])
{
    sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (sock_fd < 0)
        return -1;

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();

    if(bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr))<0) {
        printf("bind fail\n");
    }

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;
    dest_addr.nl_groups = 0;


    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

    current_pid = (int)getpid();
    memset(input,0,sizeof(input));
    if(argc == 1) {
        //printf("argc == 1 \n");
        //printf("c 1\n");
        sprintf(input, "c 1");
    } else if(argc == 2) {
        //printf("argc == 2 \n");
        //-c -s -p
        if(argv[1][0] == '-') {
            //no pid
            if(strlen(argv[1]) == 2) {
                if(argv[1][1] == 'c') {
                    sprintf(input, "c 1");
                } else if(argv[1][1] == 'p') {
                    sprintf(input, "p %d",current_pid);
                } else if(argv[1][1] == 's') {
                    sprintf(input, "s %d",current_pid);
                }
                //error
                else {
                    printf("wrong instruction\n");
                    //return;
                }

            }
            //give pid
            else {
                //modify
                char *pid_ptr = &argv[1][2];;
                //pid_ptr = &argv[1][2];
                pid = atoi(pid_ptr);
                if(argv[1][1] == 'c') {
                    sprintf(input, "c %d",pid);

                } else if(argv[1][1] == 'p') {
                    sprintf(input, "p %d",pid);
                    //printf("p\n");
                } else if(argv[1][1] == 's') {
                    sprintf(input, "s %d",pid);

                } else {

                }
            }

        }
        //only pid
        else {
            //pid = atoi(argv[1]);
            sprintf(input,"c %s",argv[1]);
        }
    } else {
        printf("ERROR \n");
        return -1;
    }

    //input msg
    strcpy(NLMSG_DATA(nlh),input);
    //strcpy(NLMSG_DATA(nlh), "Helloooo");
    memset(input,0,sizeof(input));

    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    //printf("Sending message to kernel\n");
    sendmsg(sock_fd, &msg, 0);
    //printf("Waiting for message from kernel\n");

    /* Read message from kernel */
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));

    recvmsg(sock_fd, &msg, 0);
    //printf("Received message payload: \n %s", (char*)NLMSG_DATA(nlh));
    printf("%s", (char*)NLMSG_DATA(nlh));
    close(sock_fd);
}
