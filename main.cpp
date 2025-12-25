/*
 * 项目名称：万能搜索器 (Universal Searcher)
 * 核心技术：倒排索引 (Inverted Index) + 哈希表 (Hash Table)
 * 创新点：O(1)级搜索速度，按词频排序，支持外部文件读取
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TABLE_SIZE 1007  // 哈希表大小
#define MAX_WORD_LEN 50
#define MAX_LINE_LEN 1024 // 读取文件的最大行宽

// --- 数据结构定义 ---

// 文档节点：记录单词在哪个文档出现过，以及次数
typedef struct DocNode {
    int doc_id;
    char doc_name[50];
    int frequency;          // 词频
    struct DocNode *next;
} DocNode;

// 单词节点：哈希表中的槽位
typedef struct WordNode {
    char word[MAX_WORD_LEN];
    DocNode *doc_list;      // 倒排链表：指向包含该词的所有文档
    struct WordNode *next;  // 解决哈希冲突的链表
} WordNode;

// 全局哈希表
WordNode* HashTable[TABLE_SIZE];

// --- 核心算法函数 ---

// 1. BKDR字符串哈希函数
unsigned int HashFunction(char *str) {
    unsigned int seed = 131; // 31 131 1313 13131 etc..
    unsigned int hash = 0;
    while (*str) {
        hash = hash * seed + (*str++);
    }
    return (hash % TABLE_SIZE);
}

// 2. 字符串转小写（标准化）
void ToLowerCase(char *dest, const char *src) {
    int i = 0;
    while (src[i]) {
        dest[i] = tolower(src[i]);
        i++;
    }
    dest[i] = '\0';
}

// 3. 将单词插入索引库
void InsertIndex(char *raw_word, int doc_id, char *doc_name) {
    char word[MAX_WORD_LEN];
    ToLowerCase(word, raw_word); // 统一转小写

    // 过滤掉过短的非单词字符
    if (strlen(word) == 0) return;

    unsigned int idx = HashFunction(word);
    WordNode *p = HashTable[idx];
    
    // 步骤1：在哈希桶中查找单词是否存在
    while (p != NULL) {
        if (strcmp(p->word, word) == 0) {
            break; 
        }
        p = p->next;
    }

    // 步骤2：如果单词不存在，创建单词节点
    if (p == NULL) {
        p = (WordNode*)malloc(sizeof(WordNode));
        if (p == NULL) return; // 内存分配失败保护
        strcpy(p->word, word);
        p->doc_list = NULL;
        p->next = HashTable[idx]; // 头插法解决哈希冲突
        HashTable[idx] = p;
    }

    // 步骤3：在单词的文档链表中查找文档是否存在
    DocNode *d = p->doc_list;
    while (d != NULL) {
        if (d->doc_id == doc_id) {
            d->frequency++; // 该文档已记录，词频+1
            return;
        }
        d = d->next;
    }

    // 步骤4：如果文档未记录，创建文档节点
    DocNode *new_doc = (DocNode*)malloc(sizeof(DocNode));
    if (new_doc == NULL) return;
    new_doc->doc_id = doc_id;
    strcpy(new_doc->doc_name, doc_name);
    new_doc->frequency = 1;
    new_doc->next = p->doc_list; // 头插法
    p->doc_list = new_doc;
}

// 4. [修改后] 从外部文件读取并构建索引
void LoadFileAndIndex(int doc_id, char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("!! 警告: 无法打开文件 [%s]，请确认文件是否存在。\n", filename);
        return;
    }

    printf(">> 正在索引文件: [%s] ...\n", filename);

    char line_buffer[MAX_LINE_LEN];
    const char *delimiters = " ,.?!\"\n\t\r[](){}"; // 增加了一些常见分隔符

    // 逐行读取文件内容
    while (fgets(line_buffer, MAX_LINE_LEN, fp) != NULL) {
        // 去除换行符（可选，strtok会自动处理，但为了安全可加）
        line_buffer[strcspn(line_buffer, "\n")] = 0;

        char *token = strtok(line_buffer, delimiters);
        while (token != NULL) {
            InsertIndex(token, doc_id, filename);
            token = strtok(NULL, delimiters);
        }
    }

    fclose(fp);
}

// 5. 搜索功能
void Search(char *query) {
    char word[MAX_WORD_LEN];
    ToLowerCase(word, query);
    
    unsigned int idx = HashFunction(word);
    WordNode *p = HashTable[idx];

    // 在哈希桶中找词
    while (p != NULL) {
        if (strcmp(p->word, word) == 0) {
            // 找到了单词，打印文档列表
            printf("\n>>> 搜索结果: \"%s\" <<<\n", query);
            printf("%-20s | %-10s\n", "文档名称", "出现次数");
            printf("--------------------------------\n");
            
            DocNode *d = p->doc_list;
            int found = 0;
            // 注意：这里没有对链表进行排序，若需按频率排序，需在输出前对链表进行排序算法（如冒泡排序）
            while (d != NULL) {
                printf("%-20s | %-10d\n", d->doc_name, d->frequency);
                d = d->next;
                found = 1;
            }
            if (!found) printf("索引异常。\n");
            return;
        }
        p = p->next;
    }
    
    printf("\n未找到包含 \"%s\" 的文档。\n", query);
}

// --- 主程序 ---

int main() {
    int i;
    // 初始化哈希表
    for(i=0; i<TABLE_SIZE; i++) HashTable[i] = NULL;

    printf("=========================================\n");
    printf("     万能搜索器 (Inverted Index Engine)   \n");
    printf("     支持外部文件读取版                   \n");
    printf("=========================================\n");

    printf("正在构建索引库...\n");

    // 这里定义你要读取的文件列表
    // 请确保这些 .txt 文件在你的 .exe 同级目录下
    char *files[] = {
        "source_1.txt",
        "source_2.txt", 
        "requirements.txt", 
        "test.txt"
    };
    int file_count = 4; // 文件数量

    for (i = 0; i < file_count; i++) {
        LoadFileAndIndex(i + 1, files[i]);
    }

    printf("索引构建完成！\n\n");

    // 2. 交互式搜索
    char query[MAX_WORD_LEN];
    while (1) {
        printf("\n请输入要搜索的关键词 (输入 'quit' 退出): ");
        scanf("%s", query);
        
        if (strcmp(query, "quit") == 0) break;
        
        Search(query);
    }

    return 0;
}
