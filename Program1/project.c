#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// 结构定义
// 非停用词单词信息体
typedef struct NonStopWord
{
    char word[20];
    int count;
} NonStopWord;

// 停用词树
typedef struct StopWordsTree
{
    char word[20];
    struct StopWordsTree *left;
    struct StopWordsTree *right;
} StopWordsTree;

// 变量定义

// 非停用词单词数组
NonStopWord nonStopWords[20000] = {0};

// 单词数组
char word[20] = {0};

// 停用词文件指针
FILE *StopWordsFile;
// 已有网页文件指针
FILE *WebFile;
// 样本网页文件指针
FILE *SampleFile;

// 停用词树根节点
StopWordsTree *Root = NULL;

// 功能函数声明
// 读取一个单词
void GetWord(FILE *file);
// 创建停用词树
void CreateStopWordsTree();
// 判断是否是停用词
int IsStopWord();
// 非停用词词频统计
void NonStopWordsCount();
// 非停用词词频排序,并得到特征向量
void NonStopWordsSort();
// 主程序实现
int main()
{
    // 步骤0：打开停用词文件和网页文件
    StopWordsFile = fopen("stopwords.txt", "r");
    if (StopWordsFile == NULL)
    {
        printf("停用词文件打开失败！\n");
        return 1;
    }
    WebFile = fopen("article.txt", "r");
    if (WebFile == NULL)
    {
        printf("已有网页文件打开失败！\n");
        return 1;
    }
    SampleFile = fopen("sample.txt", "r");
    if (SampleFile == NULL)
    {
        printf("样本网页文件打开失败！\n");
        return 1;
    }
    // 步骤1：得到所有网页的特征向量：非停用词单词数组排序后前N个信息体
    CreateStopWordsTree();
    NonStopWordsCount();
    NonStopWordsSort();

    return 0;
}
// 功能函数实现
// 读取一个单词,同时要把单词转换成小写
void GetWord(FILE *file)
{
    int i = 0;
    char ch;
    while ((ch = fgetc(file)) != EOF)
    {
        if (isalpha(ch))
        {
            word[i++] = tolower(ch);
        }
        else
        {
            if (i > 0)
            {
                word[i] = '\0';
                return;
            }
        }
    }
    if (i > 0)
    {
        word[i] = '\0';
    }
}
// 创建停用词树
void CreateStopWordsTree()
{
    char word[20] = {0};
    StopWordsTree *p;
    while (fscanf(StopWordsFile, "%s", word) != EOF)
    {
        p = Root;
        while (p != NULL)
        {
            if (strcmp(word, p->word) < 0)
            {
                if (p->left == NULL)
                {
                    p->left = (StopWordsTree *)malloc(sizeof(StopWordsTree));
                    strcpy(p->left->word, word);
                    p->left->left = NULL;
                    p->left->right = NULL;
                    break;
                }
                else
                {
                    p = p->left;
                }
            }
            else if (strcmp(word, p->word) > 0)
            {
                if (p->right == NULL)
                {
                    p->right = (StopWordsTree *)malloc(sizeof(StopWordsTree));
                    strcpy(p->right->word, word);
                    p->right->left = NULL;
                    p->right->right = NULL;
                    break;
                }
                else
                {
                    p = p->right;
                }
            }
            else
            {
                break;
            }
        }
        if (p == NULL)
        {
            Root = (StopWordsTree *)malloc(sizeof(StopWordsTree));
            strcpy(Root->word, word);
            Root->left = NULL;
            Root->right = NULL;
        }
    }
}
