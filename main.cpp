#include<bits/stdc++.h>

using namespace std;

#define DEFAULT_MEM_SIZE 64    //内存大小
#define DEFAULT_MEM_START 0        //起始位置

/*内存分配算法*/
#define MA_FF 1
#define MA_BF 2
#define MA_WF 3

//描述每一个空闲块的数据结构
typedef struct free_block_type {
    int size;
    int start_addr;
    struct free_block_type *next;
} FBT;

//每个进程分配到的内存块描述
typedef struct allocated_block {
    int pid;
    int size;
    int start_addr;
    string process_name;
    struct allocated_block *next;
} AB;

//指向内存中空闲块链表的首指针
FBT *free_block;

//进程分配内存块链表的首指针
AB *allocated_block_head = nullptr;

int mem_size = DEFAULT_MEM_SIZE; //内存大小
int now_algorithm = MA_FF; //当前分配算法
static int pid = 0; //初始pid
int flag = 0; //设置内存大小标志
int min_mem_size = 0; //设置剩余分区过小的标志

FBT *init_free_block(int mem_sizee);

void display_menu();

int set_mem_size();

int display_mem_usage();

int dispose(AB *free_ab);

int free_mem();

int kill_process();

int allocate_mem(AB *ab);

int new_process();

void rearrange_FF();

void rearrange_BF();

void rearrange_WF();

void rearrange(int algorithm);

void set_algorithm();

void do_exit() {
    //一般的操作系统在程序exit或return后都会回收所申请的内存，故此处为空。
}

int main() {
    //code
    char choice;
    pid = 0;
    free_block = init_free_block(mem_size); //初始化空闲区
    cout << "此程序中规定设置内存大小为64MB" << endl;
    display_menu(); //显示菜单
    while ((cin >> choice)) {
        switch (choice) {
            case '1':
                set_algorithm();
                flag = 1;
                break;
            case '2':
                new_process();
                flag = 1;
                break;
            case '3':
                kill_process();
                flag = 1;
                break;
            case '4':
                display_mem_usage();
                flag = 1;
                break;
            case '0':
                do_exit();
                exit(0);
            default:
                break;
        }
        display_menu();
    }
    // }
}

void display_menu() {
    puts("");
    cout << "1 - 选择内存分配算法" << endl;
    cout << "2 - 新进程" << endl;
    cout << "3 - 终止进程 " << endl;
    cout << "4 - 显示内存使用情况" << endl;
    cout << "0 - 退出系统" << endl;
}

//初始化空闲分区链表
FBT *init_free_block(int mem_sizee) {
    FBT *fb;

    fb = (FBT *) malloc(sizeof(FBT));
    if (fb == nullptr) {
        cout << "No mem" << endl;
        return nullptr;
    }
    fb->size = mem_sizee;
    fb->start_addr = DEFAULT_MEM_START;
    fb->next = nullptr;
    return fb;
}

//重新设置内存大小
int set_mem_size() {
    int size;
    if (flag != 0) {
        cout << "无法再次设定内存大小!" << endl;
        return 0;
    }
    cout << "内存总大小 =" << endl;
    cin >> size;
    if (size > 0) {
        mem_size = size;
        free_block->size = mem_size;
    }
    flag = 1;
    min_mem_size = mem_size / 100;
    return 1;
}

int display_mem_usage() {
    //显示当前内存的使用情况，包括空闲分区的情况和已经分配的情况
    FBT *fbt = free_block;
    AB *ab = allocated_block_head;
    //显示空闲区
    cout << "------------------------------------------------------------------" << endl;
    cout << "空闲区域:" << endl;
    cout << "起始地址       内存大小 " << endl;
    while (fbt != nullptr) {
        cout << fbt->start_addr << "                " << fbt->size << endl;
        fbt = fbt->next;
    }

    //显示已分配区
    cout << "------------------------------------------------------------------" << endl;
    cout << "Used Memory:" << endl;
    cout << "进程号      起始地址  占用内存大小  \n";
    while (ab != nullptr) {
        cout << ab->pid << "           " << ab->start_addr << "         " << ab->size << endl;
        ab = ab->next;
    }
    return 0;
}

//释放链表节点
int dispose(AB *free_ab) {
    /*释放ab数据结构节点*/
    AB *pre, *ab;
    if (free_ab == allocated_block_head) {
        //如果要是释放第一个节点
        allocated_block_head = allocated_block_head->next;
        free(free_ab);
        return 1;
    }
    pre = allocated_block_head;
    ab = allocated_block_head->next;
    while (ab != free_ab) {
        pre = ab;
        ab = ab->next;
    }
    pre->next = ab->next;
    free(ab);
    return 2;
}

//释放进程所占用的内存
int free_mem(AB *ab) {
    //将ab所表示的已分配区归还，并进行可能的合并
    int algorithm = now_algorithm;
    FBT *fbt, *pre, *work;
    fbt = (FBT *) malloc(sizeof(FBT));
    if (!fbt) return -1;
    /*
    进行可能的合并，基本策略如下
    1. 将新释放的结点插入到空闲分区队列末尾
    2. 对空闲链表按照地址有序排列
    3. 检查并合并相邻的空闲分区
    4. 将空闲链表重新按照当前算法排序
    */
    fbt->size = ab->size;
    fbt->start_addr = ab->start_addr;

    //插至末尾
    work = free_block;
    if (work == nullptr) {
        free_block = fbt;
        fbt->next = nullptr;
    } else {
        while (work->next != nullptr) {
            work = work->next;
        }
        fbt->next = work->next;
        work->next = fbt;
    }
    //按地址重新排布
    rearrange_FF();

    //合并可能分区;即若两空闲分区相连则合并
    pre = free_block;
    while (pre->next) {
        work = pre->next;
        if (pre->start_addr + pre->size == work->start_addr) {
            pre->size = pre->size + work->size;
            pre->next = work->next;
            free(work);
            continue;
        } else {
            pre = pre->next;
        }
    }

    //按照当前算法排序
    rearrange(now_algorithm);
    return 1;
}

//找到pid对应的链表节点
AB *find_process(int pidd) {
    AB *tmp = allocated_block_head;
    while (tmp != nullptr) {
        if (tmp->pid == pidd) {
            return tmp;
        }
        tmp = tmp->next;
    }
    cout << "Cannot find pid:" << pidd << endl;
    return nullptr;
}

int kill_process() {
    AB *ab;
    int pidd;
    cout << "Kill Process,pid=";
    cin >> pidd;
    ab = find_process(pidd);
    if (ab != nullptr) {
        free_mem(ab);    //释放ab所表示的分配表
        dispose(ab);//释放ab数据结构节点
        cout << "Success!" << endl;
        return 0;
    } else {
        return -1;
    }
}

//寻找是否有分区可以非进程分配
int find_free_mem(int request) {
    FBT *tmp = free_block;
    int mem_sum = 0;
    while (tmp) {
        if (tmp->size >= request) {
            //可以直接分配
            return 1;
        }
        mem_sum += tmp->size;
        tmp = tmp->next;
    }
    if (mem_sum >= request) {
        //合并后分配
        return 0;
    } else {
        //没有足够的空间可供分配
        return -1;
    }

}


//将已分配表按起始地址从大到小排序
void sort_AB() {
    if (allocated_block_head == nullptr || allocated_block_head->next == nullptr)
        return;
    AB *t1, *t2, *head;
    head = allocated_block_head;
    for (t1 = head->next; t1; t1 = t1->next) {
        for (t2 = head; t2 != t1; t2 = t2->next) {
            if (t2->start_addr > t2->next->start_addr) {
                int tmp = t2->start_addr;
                t2->start_addr = t2->next->start_addr;
                t2->next->start_addr = tmp;

                tmp = t2->size;
                t2->size = t2->next->size;
                t2->next->size = tmp;
            }
        }
    }
}

//重新给所有进程分配内存地址
void reset_AB(int start) {
    //在真实操作系统中这个操作非常不容易，故内存紧缩并不能频繁使用
    AB *tmp = allocated_block_head;
    while (tmp != nullptr) {
        tmp->start_addr = start;
        start += tmp->size;
        tmp = tmp->next;
    }
}

void memory_compact() {
    //进行内存紧缩
    FBT *fbttmp = free_block;
    AB *abtmp = allocated_block_head;
    //检测剩余内存
    int sum = 0;
    while (fbttmp != nullptr) {
        sum += fbttmp->size;
        fbttmp = fbttmp->next;
    }

    //合并区块为一个
    fbttmp = free_block;
    fbttmp->size = sum;
    fbttmp->start_addr = 0;
    fbttmp->next = nullptr;

    //释放多余分区
    FBT *pr = free_block->next;
    while (pr != nullptr) {
        fbttmp = pr->next;
        free(pr);
        pr = fbttmp;
    }
    //重新排序已分配空间
    sort_AB();
    reset_AB(sum);

}

//执行分配内存
void do_allocate_mem(AB *ab) {
    int request = ab->size;
    FBT *tmp = free_block;
    while (tmp != nullptr) {
        if (tmp->size >= request) {
            //分配
            ab->start_addr = tmp->start_addr;
            int shengyu = tmp->size - request;
            if (shengyu <= min_mem_size) {
                //剩余过小全部分配
                ab->size = tmp->size;
                if (tmp == free_block) {
                    free_block = free_block->next;
                    free(tmp);
                } else {
                    FBT *t = free_block;
                    while (t->next != tmp) {
                        t = t->next;
                    }
                    t->next = tmp->next;
                    free(tmp);
                }
            } else {
                //切割出分配走的内存
                tmp->size = shengyu;
                tmp->start_addr = tmp->start_addr + request;
            }
            return;
        }
        tmp = tmp->next;
    }
}

int allocate_mem(AB *ab) {
    //分配内存模块
    FBT *fbt, *pre;
    int request_size = ab->size;
    fbt = pre = free_block;

//    根据当前算法在空闲分区链表中搜索合适空闲分区进行分配，分配时注意以下情况：
//    1. 找到可满足空闲分区且分配后剩余空间足够大，则分割
//    2. 找到可满足空闲分区且但分配后剩余空间比较小，则一起分配
//    3. 找不可满足需要的空闲分区但空闲分区之和能满足需要，
//        则采用内存紧缩技术，进行空闲分区的合并，然后再分配
//    4. 在成功分配内存后，应保持空闲分区按照相应算法有序
//    5. 分配成功则返回1，否则返回-1


    //尝试寻找可分配空闲，具体结果在函数中有解释
    int f = find_free_mem(request_size);
    if (f == -1) {
        //不够分配
        cout << "Free mem is not enough,Allocate fail!" << endl;
        return -1;
    } else {
        if (f == 0) {
            //需要内存紧缩才能分配
            memory_compact();
        }
        //执行分配
        do_allocate_mem(ab);
    }
    //重新排布空闲分区
    rearrange(now_algorithm);
    return 1;
}

//创建新进程
//创建新进程
//创建新进程
int new_process() {
    AB *ab;
    int size;
    int ret;
    ab = (AB *) malloc(sizeof(AB));
    if (!ab)
        exit(-5);
    ab->next = nullptr;
    pid++;
    ab->pid = pid;
    string spid = "PROCESS:" + to_string(pid);
    cout << "Memory for " << spid << endl;
    cin >> size;
    if (size > 0)
        ab->size = size;
    ret = allocate_mem(ab);        //从空闲分区分配内存，ret==1表示分配成功
    if ((ret == 1) && (allocated_block_head == nullptr)) {
        //如果此时allocated_block_head尚未赋值，则赋值
        allocated_block_head = ab;
        cout << "Success!" << endl;
        return 1;
    } else if (ret == 1) {
        //分配成功，将该分配块的描述插入已分配链表
        ab->next = allocated_block_head;
        allocated_block_head = ab;
        cout << "Success!" << endl;
        return 2;
    } else if (ret == -1) {
        //分配不成功
        cout << "Unsuccess!" << endl;
        free(ab);
        return -1;
    }
    return 3;
}

void rearrange_FF() {
    //首次适应算法，空闲区大小按起始地址升序排序
    //这里使用冒泡排序方法
    if (free_block == nullptr || free_block->next == nullptr)
        return;
    FBT *t1, *t2, *head;
    head = free_block;
    for (t1 = head->next; t1; t1 = t1->next) {
        for (t2 = head; t2 != t1; t2 = t2->next) {
            if (t2->start_addr > t2->next->start_addr) {
                int tmp = t2->start_addr;
                t2->start_addr = t2->next->start_addr;
                t2->next->start_addr = tmp;

                tmp = t2->size;
                t2->size = t2->next->size;
                t2->next->size = tmp;
            }
        }
    }
}

void rearrange_BF() {
    //最佳适应算法，空闲分区按大小从小到大排序
    if (free_block == nullptr || free_block->next == nullptr)
        return;
    FBT *t1, *t2, *head;
    head = free_block;
    for (t1 = head->next; t1; t1 = t1->next) {
        for (t2 = head; t2 != t1; t2 = t2->next) {
            if (t2->size > t2->next->size) {
                int tmp = t2->start_addr;
                t2->start_addr = t2->next->start_addr;
                t2->next->start_addr = tmp;

                tmp = t2->size;
                t2->size = t2->next->size;
                t2->next->size = tmp;
            }
        }
    }
}

void rearrange_WF() {
    //最坏适应算法，空闲分区按从大到小排序
    if (free_block == nullptr || free_block->next == nullptr)
        return;
    FBT *t1, *t2, *head;
    head = free_block;
    for (t1 = head->next; t1; t1 = t1->next) {
        for (t2 = head; t2 != t1; t2 = t2->next) {
            if (t2->size < t2->next->size) {
                int tmp = t2->start_addr;
                t2->start_addr = t2->next->start_addr;
                t2->next->start_addr = tmp;

                tmp = t2->size;
                t2->size = t2->next->size;
                t2->next->size = tmp;
            }
        }
    }
}

//按指定的算法整理内存空闲块链表
void rearrange(int algorithm) {
    switch (algorithm) {
        case MA_FF:
            rearrange_FF();
            break;
        case MA_BF:
            rearrange_BF();
            break;
        case MA_WF:
            rearrange_WF();
            break;
        default:
            break;
    }
}

void set_algorithm() {
    //设置当前分配算法
    int algorithm;
    cout << "\t1 - 首次适应算法\n";
    cout << "\t2 - 最佳适应算法\n";
    cout << "\t3 - 最差适应算法\n";
    cin >> algorithm;
    if (algorithm >= 1 && algorithm <= 3)
        now_algorithm = algorithm;

    //按指定算法重新排列空闲区链表
    rearrange(algorithm);
}

