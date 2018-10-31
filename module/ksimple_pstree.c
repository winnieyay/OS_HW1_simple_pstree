#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#define NETLINK_USER 31
#define size 20000
#include <linux/string.h>

struct sock *nl_sk = NULL;
char* const cut = " ";
int count;
char msg[size];
char tmp_msg_1[size];
void child(struct task_struct *task, int seq);
void print_myself(struct task_struct *task, int task_pid);
int find_top(struct task_struct *task,int seq);
void print_parent(struct task_struct *task,int seq,int max);
void print_sib(struct task_struct *task,int my_pid);
static void nl_recv_msg(struct sk_buff *skb)
{

    struct nlmsghdr *nlh;
    int pid;
    struct sk_buff *skb_out;
    //char *msg;
    int res;
    int input_size;
    int task_pid;
    char *pid_msg;
    char *cut_pid_tmp;


    memset(msg,'\0',strlen(msg));
    printk(KERN_INFO "Entering: %s\n", __FUNCTION__);


    nlh = (struct nlmsghdr *)skb->data;
    printk(KERN_INFO "Netlink received msg payload:%s\n", (char *)nlmsg_data(nlh));

    char *input_msg = (char *)nlmsg_data(nlh);
    struct task_struct *task;



    if(kstrtoint( input_msg+strlen("00"), 10, &task_pid) != 0) {
        printk(KERN_ERR "Failed to convert string\n");
        //return;
    } else {
        printk(KERN_INFO "task_pid:%d\n",task_pid);
    }


    task = pid_task(find_get_pid(task_pid), PIDTYPE_PID);
    printk(KERN_INFO "task %s",task->comm);


    if( task == NULL) {
        printk(KERN_ERR "task null");
        //return;
    } else {
        printk(KERN_INFO "task gogo");
        int my_pid;
        //test end
        input_size = strlen(input_msg);
        if(input_msg[0] == 'c') {
            printk(KERN_INFO "input_msg[0] == c");
            print_myself(task, task_pid);
            child(task, 1);
        } else if(input_msg[0] == 'p') {
            printk(KERN_INFO "input_msg[0] == p");
            //struct task_struct *tmp_task;
            //tmp_task = task;
            //count = 0;
            int max;
            max  = find_top(task,0);
            printk(KERN_INFO "find max %d\n",max);
            printk(KERN_INFO "check task: %d\n",task->pid);
            print_parent(task,0,max);
        } else if(input_msg[0] == 's') {
            printk(KERN_INFO "input_msg[0] == s");
            int my_pid;
            my_pid = task->pid;
            print_sib(task,my_pid);
            //void print_sib(struct task_struct *task)
        } else {
            printk(KERN_INFO "WHYYYY");

        }




    }


    printk(KERN_INFO "check2222");

    pid = nlh->nlmsg_pid; /*pid of sending process */


    skb_out = nlmsg_new(size, 0);
    if (!skb_out) {
        printk(KERN_ERR "Failed to allocate new skb\n");
        return;
    }

    //init
    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, size, 0);
    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */


    strncpy(nlmsg_data(nlh), msg, size);

    res = nlmsg_unicast(nl_sk, skb_out, pid);
    if (res < 0)
        printk(KERN_INFO "Error while sending bak to user\n");
}

void child(struct task_struct *task, int seq)
{
    struct list_head *list = NULL;
    char tmp_msg;
    //strcat(msg, task->comm);
    //from pos to head
    list_for_each(list,&(task->children)) {

        char str_temp[20];
        int i;
        for(i = 0; i < seq; ++i) {
            strcat(msg, "    ");
        }

        struct task_struct *getChild = list_entry(list, struct task_struct, sibling);
        sprintf(str_temp, "%d", task_pid_nr(getChild));

        strcat(msg, getChild->comm);
        strcat(msg, "(");
        strcat(msg, str_temp);
        strcat(msg, ")\n");


        child(getChild, seq + 1);
    }

}
void print_myself(struct task_struct *task, int task_pid)
{

    char pid_tmp[20];
    strcat(msg, task->comm);

    strcat(msg, "(");
    sprintf(pid_tmp,"%d",task_pid);
    strcat(msg, pid_tmp);
    //strcat(msg,(char)task_pid);
    strcat(msg, ")\n");
}
int find_top(struct task_struct *task,int seq)
{
    int max;
    char tmp_msg;
    struct task_struct *task_top_1;
    struct task_struct *task_top_2;
    struct task_struct *task_parent;
    //seq ++;
    //int i;
    task_top_1 = pid_task(find_get_pid(1), PIDTYPE_PID);
    task_top_2 = pid_task(find_get_pid(2), PIDTYPE_PID);
    if(task->parent == task_top_1 || task->parent == task_top_2 ) {
        //sprintf(msg,"%s",task->parent->comm);
        //sprintf(msg,"(%d)\n",task->parent->pid);
        max = seq + 1;
        return max;
    } else if(task == task_top_1 || task == task_top_2) {
        max = 0;
    } else {
        task_parent = task->parent;
        find_top(task_parent, seq + 1);
    }
}
void print_parent(struct task_struct *task,int seq,int max)
{
    printk(KERN_INFO "come in\n");

    //char tmp_msg;
    //char tmp_msg_1;
    struct task_struct *task_parent;
    struct task_struct *task_top_1;
    struct task_struct *task_top_2;
    memset(tmp_msg_1,'\0',strlen(tmp_msg_1));
    task_top_1 = pid_task(find_get_pid(1), PIDTYPE_PID);
    task_top_2 = pid_task(find_get_pid(2), PIDTYPE_PID);
    //char tmp_msg_1[size];
    strncat(tmp_msg_1,msg,strlen(msg));
    memset(msg,'\0',strlen(msg));
    if(task->parent == task_top_1 || task->parent == task_top_2) {
        char tmp_msg[20];
        char tmp_msg_2[20];
        //char tmp_msg_2[size];
        int task_pid;
        //task_pid
        //count = seq;
        strcat(msg,task->parent->comm);
        strcat(msg,"(");
        sprintf(tmp_msg_2, "%d", task->parent->pid);
        strcat(msg,tmp_msg_2);
        strcat(msg,")\n");

        strcat(msg,"    ");
        strcat(msg,task->comm);
        strcat(msg,"(");
        sprintf(tmp_msg, "%d", task->pid);
        strcat(msg,tmp_msg);
        strcat(msg,")\n");

        //sprintf(tmp_msg,"    %s(%d)\n",task->comm,task->pid);
        strcat(msg,tmp_msg_1);

    } else if(task == task_top_1 || task == task_top_2) {

        char tmp_msg[20];
        strcat(msg,task->comm);
        strcat(msg,"(");
        sprintf(tmp_msg, "%d", task->pid);
        strcat(msg,tmp_msg);
        strcat(msg,")\n");

    } else {
        char tmp_msg[30];

        task_parent = task->parent;
        int i;
        for(i = 0; i< max - seq; i++) {
            strcat(msg,"    ");
        }
        strcat(msg,task->comm);
        sprintf(tmp_msg,"%d",task->pid);
        strcat(msg,"(");
        strcat(msg,tmp_msg);
        strcat(msg,")\n");
        strcat(msg,tmp_msg_1);

        print_parent(task_parent, seq + 1,max);
    }


}

void print_sib(struct task_struct *task,int my_pid)
{
    printk(KERN_INFO "come in\n");
    char tmp_msg[30];
    struct list_head *list = NULL;
    //struct task_struct task_tmp;
    list_for_each(list,&task->parent->children) {
        struct task_struct *getSib = list_entry(list, struct task_struct, sibling);
        if(getSib->pid == my_pid) {
            //don't print
        } else {
            sprintf(tmp_msg,"%s(%d)\n",getSib->comm,getSib->pid);
            strcat(msg,tmp_msg);
        }

    }
    return;
}

static int __init recv_init(void)
{

    printk("Entering: %s\n", __FUNCTION__);
    struct netlink_kernel_cfg cfg = {
        .input = nl_recv_msg,
    };

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if (!nl_sk) {
        printk(KERN_ALERT "Error creating socket.\n");
        return -10;
    }
    return 0;
}

static void __exit recv_exit(void)
{

    printk(KERN_INFO "exiting hello module\n");
    netlink_kernel_release(nl_sk);
}

module_init(recv_init);
module_exit(recv_exit);

MODULE_LICENSE("GPL");