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

// 停用词树，用前缀s树实现
typedef struct StopWordsTree
{
    int cnt;
    StopWordsTree *chilren[26];
} StopWordsTree;

// 变量定义

// 非停用词单词数组
NonStopWord nonStopWords[20000] = {0};
// 非停用词数量
int nonStopWordsNum = 0;
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

    // 关闭文件
    fclose(StopWordsFile);
    fclose(WebFile);
    fclose(SampleFile);
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
// 创建停用词树,用前缀树实现
void CreateStopWordsTree()
{
    Root = (StopWordsTree *)malloc(sizeof(StopWordsTree));
    Root->cnt = 0;
    for (int i = 0; i < 26; i++)
    {
        Root->chilren[i] = NULL;
    }
    StopWordsTree *p = Root;
    while (fscanf(StopWordsFile, "%s", word) != EOF)
    {
        p = Root;
        for (int i = 0; i < strlen(word); i++)
        {
            int index = word[i] - 'a';
            if (p->chilren[index] == NULL)
            {
                p->chilren[index] = (StopWordsTree *)malloc(sizeof(StopWordsTree));
                p->chilren[index]->cnt = 0;
                for (int j = 0; j < 26; j++)
                {
                    p->chilren[index]->chilren[j] = NULL;
                }
            }
            p = p->chilren[index];
        }
        p->cnt = 1;
    }
}
// 判断是否是停用词,是返回1,否返回0
int IsStopWord()
{
    StopWordsTree *p = Root;
    for (int i = 0; i < strlen(word); i++)
    {
        int index = word[i] - 'a';
        if (p->chilren[index] == NULL)
        {
            return 0;
        }
        p = p->chilren[index];
    }
    if (p->cnt == 1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
// 非停用词词频统计
void NonStopWordsCount()
{
    GetWord(WebFile);
    while (strlen(word) > 0)
    {
        if (IsStopWord() == 0)
        {
            int i;
            for (i = 0; i < nonStopWordsNum; i++)
            {
                if (strcmp(nonStopWords[i].word, word) == 0)
                {
                    nonStopWords[i].count++;
                    break;
                }
            }
            if (i == nonStopWordsNum)
            {
                strcpy(nonStopWords[nonStopWordsNum].word, word);
                nonStopWords[nonStopWordsNum].count = 1;
                nonStopWordsNum++;
            }
        }
        GetWord(WebFile);
    }
}
// 非停用词词频排序,并得到特征向量
// 降序排列,词频相同的按字典序升序排列,这是qsort函数的比较函数
int cmp(const void *a, const void *b)
{
    NonStopWord *c = (NonStopWord *)a;
    NonStopWord *d = (NonStopWord *)b;
    if (c->count != d->count)
    {
        return d->count - c->count;
    }
    else
    {
        return strcmp(c->word, d->word);
    }
}
void NonStopWordsSort()
{
    int i, j;
    qsort(nonStopWords, nonStopWordsNum, sizeof(NonStopWord), cmp);
    for (i = 0; i < 5; i++)
    {
        printf("%s %d\n", nonStopWords[i].word, nonStopWords[i].count);
    }
}
