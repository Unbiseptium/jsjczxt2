#include<bits/stdc++.h>

using namespace std;

#define DEFAULT_MEM_SIZE 64    //�ڴ��С
#define DEFAULT_MEM_START 0        //��ʼλ��

/*�ڴ�����㷨*/
#define MA_FF 1
#define MA_BF 2
#define MA_WF 3

//����ÿһ�����п�����ݽṹ
typedef struct free_block_type {
    int size;
    int start_addr;
    struct free_block_type *next;
} FBT;

//ÿ�����̷��䵽���ڴ������
typedef struct allocated_block {
    int pid;
    int size;
    int start_addr;
    string process_name;
    struct allocated_block *next;
} AB;

//ָ���ڴ��п��п��������ָ��
FBT *free_block;

//���̷����ڴ���������ָ��
AB *allocated_block_head = nullptr;

int mem_size = DEFAULT_MEM_SIZE; //�ڴ��С
int ma_algorithm = MA_FF; //��ǰ�����㷨
static int pid = 0; //��ʼpid
int flag = 0; //�����ڴ��С��־
int min_mem_size = 0; //����ʣ�������С�ı�־

FBT *init_free_block(int mem_size);

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
    //һ��Ĳ���ϵͳ�ڳ���exit��return�󶼻������������ڴ棬�ʴ˴�Ϊ�ա�
}

int main() {
    //code
    char choice;
    pid = 0;
    free_block = init_free_block(mem_size); //��ʼ��������
    cout << "�˳����й涨�����ڴ��СΪ64MB" << endl;
        display_menu(); //��ʾ�˵�
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
    cout << "1 - ѡ���ڴ�����㷨" << endl;
    cout << "2 - �½���" << endl;
    cout << "3 - ��ֹ���� " << endl;
    cout << "4 - ��ʾ�ڴ�ʹ�����" << endl;
    cout << "0 - �˳�ϵͳ" << endl;
}

//��ʼ�����з�������
FBT *init_free_block(int mem_size) {
    FBT *fb;

    fb = (FBT *) malloc(sizeof(FBT));
    if (fb == nullptr) {
        cout << "No mem" << endl;
        return nullptr;
    }
    fb->size = mem_size;
    fb->start_addr = DEFAULT_MEM_START;
    fb->next = nullptr;
    return fb;
}

//���������ڴ��С
int set_mem_size() {
    int size;
    if (flag != 0) {
        cout << "�޷��ٴ��趨�ڴ��С!" << endl;
        return 0;
    }
    cout << "�ڴ��ܴ�С =" << endl;
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
    //��ʾ��ǰ�ڴ��ʹ��������������з�����������Ѿ���������
    FBT *fbt = free_block;
    AB *ab = allocated_block_head;
    //��ʾ������
    cout << "------------------------------------------------------------------" << endl;
    cout << "��������:" << endl;
    cout << "��ʼ��ַ       �ڴ��С " << endl;
    while (fbt != nullptr) {
        cout << fbt->start_addr << "                " << fbt->size << endl;
        fbt = fbt->next;
    }

    //��ʾ�ѷ�����
    cout << "------------------------------------------------------------------" << endl;
    cout << "Used Memory:" << endl;
    cout << "���̺�      ��ʼ��ַ  ռ���ڴ��С  \n";
    while (ab != nullptr) {
        cout << ab->pid << "           " << ab->start_addr << "         " << ab->size << endl;
        ab = ab->next;
    }
    return 0;
}

//�ͷ�����ڵ�
int dispose(AB *free_ab) {
    /*�ͷ�ab���ݽṹ�ڵ�*/
    AB *pre, *ab;
    if (free_ab == allocated_block_head) {
        //���Ҫ���ͷŵ�һ���ڵ�
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

//�ͷŽ�����ռ�õ��ڴ�
int free_mem(AB *ab) {
    //��ab����ʾ���ѷ������黹�������п��ܵĺϲ�
    int algorithm = ma_algorithm;
    FBT *fbt, *pre, *work;
    fbt = (FBT *) malloc(sizeof(FBT));
    if (!fbt) return -1;
    /*
    ���п��ܵĺϲ���������������
    1. �����ͷŵĽ����뵽���з�������ĩβ
    2. �Կ��������յ�ַ��������
    3. ��鲢�ϲ����ڵĿ��з���
    4. �������������°��յ�ǰ�㷨����
    */
    fbt->size = ab->size;
    fbt->start_addr = ab->start_addr;

    //����ĩβ
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
    //����ַ�����Ų�
    rearrange_FF();

    //�ϲ����ܷ���;���������з���������ϲ�
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

    //���յ�ǰ�㷨����
    rearrange(ma_algorithm);
    return 1;
}

//�ҵ�pid��Ӧ������ڵ�
AB *find_process(int pid) {
    AB *tmp = allocated_block_head;
    while (tmp != nullptr) {
        if (tmp->pid == pid) {
            return tmp;
        }
        tmp = tmp->next;
    }
    cout << "Cannot find pid:" << pid << endl;
    return nullptr;
}

int kill_process() {
    AB *ab;
    int pid;
    cout << "Kill Process,pid=";
    cin >> pid;
    ab = find_process(pid);
    if (ab != nullptr) {
        free_mem(ab);    //�ͷ�ab����ʾ�ķ����
        dispose(ab);    //�ͷ�ab���ݽṹ�ڵ�
        return 0;
    } else {
        return -1;
    }
}

//Ѱ���Ƿ��з������Էǽ��̷���
int find_free_mem(int request) {
    FBT *tmp = free_block;
    int mem_sum = 0;
    while (tmp) {
        if (tmp->size >= request) {
            //����ֱ�ӷ���
            return 1;
        }
        mem_sum += tmp->size;
        tmp = tmp->next;
    }
    if (mem_sum >= request) {
        //�ϲ������
        return 0;
    } else {
        //û���㹻�Ŀռ�ɹ�����
        return -1;
    }

}


//���ѷ������ʼ��ַ�Ӵ�С����
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

//���¸����н��̷����ڴ��ַ
void reset_AB(int start) {
    //����ʵ����ϵͳ����������ǳ������ף����ڴ����������Ƶ��ʹ��
    AB *tmp = allocated_block_head;
    while (tmp != nullptr) {
        tmp->start_addr = start;
        start += tmp->size;
        tmp = tmp->next;
    }
}

void memory_compact() {
    //�����ڴ����
    FBT *fbttmp = free_block;
    AB *abtmp = allocated_block_head;
    //���ʣ���ڴ�
    int sum = 0;
    while (fbttmp != nullptr) {
        sum += fbttmp->size;
        fbttmp = fbttmp->next;
    }

    //�ϲ�����Ϊһ��
    fbttmp = free_block;
    fbttmp->size = sum;
    fbttmp->start_addr = 0;
    fbttmp->next = nullptr;

    //�ͷŶ������
    FBT *pr = free_block->next;
    while (pr != nullptr) {
        fbttmp = pr->next;
        free(pr);
        pr = fbttmp;
    }
    //���������ѷ���ռ�
    sort_AB();
    reset_AB(sum);

}

//ִ�з����ڴ�
void do_allocate_mem(AB *ab) {
    int request = ab->size;
    FBT *tmp = free_block;
    while (tmp != nullptr) {
        if (tmp->size >= request) {
            //����
            ab->start_addr = tmp->start_addr;
            int shengyu = tmp->size - request;
            if (shengyu <= min_mem_size) {
                //ʣ���Сȫ������
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
                //�и�������ߵ��ڴ�
                tmp->size = shengyu;
                tmp->start_addr = tmp->start_addr + request;
            }
            return;
        }
        tmp = tmp->next;
    }
}

int allocate_mem(AB *ab) {
    //�����ڴ�ģ��
    FBT *fbt, *pre;
    int request_size = ab->size;
    fbt = pre = free_block;

//    ���ݵ�ǰ�㷨�ڿ��з����������������ʿ��з������з��䣬����ʱע�����������
//    1. �ҵ���������з����ҷ����ʣ��ռ��㹻����ָ�
//    2. �ҵ���������з����ҵ������ʣ��ռ�Ƚ�С����һ�����
//    3. �Ҳ���������Ҫ�Ŀ��з��������з���֮����������Ҫ��
//        ������ڴ�������������п��з����ĺϲ���Ȼ���ٷ���
//    4. �ڳɹ������ڴ��Ӧ���ֿ��з���������Ӧ�㷨����
//    5. ����ɹ��򷵻�1�����򷵻�-1


    //����Ѱ�ҿɷ�����У��������ں������н���
    int f = find_free_mem(request_size);
    if (f == -1) {
        //��������
        cout << "Free mem is not enough,Allocate fail!" << endl;
        return -1;
    } else {
        if (f == 0) {
            //��Ҫ�ڴ�������ܷ���
            memory_compact();
        }
        //ִ�з���
        do_allocate_mem(ab);
    }
    //�����Ų����з���
    rearrange(ma_algorithm);
    return 1;
}

//�����½���
//�����½���
//�����½���
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
    ret = allocate_mem(ab);        //�ӿ��з��������ڴ棬ret==1��ʾ����ɹ�
    if ((ret == 1) && (allocated_block_head == nullptr)) {
        //�����ʱallocated_block_head��δ��ֵ����ֵ
        allocated_block_head = ab;
        cout << "����ɹ�" << endl;
        return 1;
    } else if (ret == 1) {
        //����ɹ������÷��������������ѷ�������
        ab->next = allocated_block_head;
        allocated_block_head = ab;
        cout << "����ɹ�" << endl;
        return 2;
    } else if (ret == -1) {
        //���䲻�ɹ�
        cout << "���䲻�ɹ�" << endl;
        free(ab);
        return -1;
    }
    return 3;
}

void rearrange_FF() {
    //�״���Ӧ�㷨����������С����ʼ��ַ��������
    //����ʹ��ð�����򷽷�
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
    //�����Ӧ�㷨�����з�������С��С��������
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
    //���Ӧ�㷨�����з������Ӵ�С����
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

//��ָ�����㷨�����ڴ���п�����
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
    //���õ�ǰ�����㷨
    int algorithm;
    cout << "\t1 - �״���Ӧ�㷨\n";
    cout << "\t2 - �����Ӧ�㷨\n";
    cout << "\t3 -�����Ӧ�㷨\n";
    cin >> algorithm;
    if (algorithm >= 1 && algorithm <= 3)
        ma_algorithm = algorithm;

    //��ָ���㷨�������п���������
    rearrange(ma_algorithm);
}

