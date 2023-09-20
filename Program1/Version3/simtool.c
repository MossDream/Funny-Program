#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// 宏定义
#define MAX_N 10000
#define MAX_WORD_LEN 200
#define MAX_ID_LEN 20
#define MAX_WEB_PAGE 23000
#define MAX_TABLE_SIZE 1377777
// 长整型变量简写
typedef long long ll;
typedef unsigned long long ull;

// 结构定义
// 非停用词单词信息体
typedef struct NonStopWord
{
    char word[MAX_WORD_LEN];
    int count;
    int status; // 0表示已经填入Hash表中，1表示未填入
} NonStopWord;
// 非停用词Hash表
typedef struct NonStopWordHashTable
{
    int tableSize;
    NonStopWord *table;
} NonStopWordHashTable;
// 停用词树，用前缀字典树实现
typedef struct StopWordsTree
{
    int cnt;
    struct StopWordsTree *chilren[26];
} StopWordsTree;
// 特征向量树，用前缀字典树实现
typedef struct FeatureVectorTree
{
    int cnt;
    int id;
    struct FeatureVectorTree *chilren[26];
} FeatureVectorTree;

// 变量定义
// 数据库的非停用词单词数组
NonStopWord nonStopWords[MAX_TABLE_SIZE] = {0};
// 数据库的非停用词数量
int nonStopWordsNum = 0;

// 原网页页数
int pageNum = 0;
// 样本网页页数
int samplePageNum = 0;

// 读到换页符的标记
int pageFlag = 0;

// 单词数组
char word[MAX_WORD_LEN] = {0};

// 网页标识信息的二维数组
//  原网页标识信息
char webId[MAX_WEB_PAGE][MAX_ID_LEN] = {0};
// 样本网页标识信息
char sampleWebId[MAX_WEB_PAGE][MAX_ID_LEN] = {0};

// 各网页权重向量构成的二维数组
// 原网页权重向量
int weight[MAX_WEB_PAGE][MAX_N] = {0};
// 样本网页权重向量
int sampleWeight[MAX_WEB_PAGE][MAX_N] = {0};

// 每个网页使用的Hash值，处理128位Hash值时按两个64位二进制数分别存储
uint64_t lowerHashValue[MAX_N] = {0};
uint64_t upperHashValue[MAX_N] = {0};

// 原网页指纹，每行的指纹按一个64位二进制数存储
uint64_t lowerFingerprint[MAX_WEB_PAGE] = {0};
uint64_t upperFingerprint[MAX_WEB_PAGE] = {0};
// 样本网页指纹，每行的指纹按一个64位二进制数存储
uint64_t lowerSampleFingerprint[MAX_WEB_PAGE] = {0};
uint64_t upperSampleFingerprint[MAX_WEB_PAGE] = {0};

// 临时储存输出结果的数组
int result[4][MAX_WEB_PAGE] = {0};
// 汉明距离分别为0、1、2、3的网页数量
int printPageNum0 = 0;
int printPageNum1 = 0;
int printPageNum2 = 0;
int printPageNum3 = 0;

// 停用词文件指针
FILE *StopWordsFile;
// 已有网页文件指针
FILE *WebFile;
// 样本网页文件指针
FILE *SampleFile;
// Hash表文件指针
FILE *HashFile;
// 输出结果文件指针
FILE *ResultFile;

// 停用词树根节点
StopWordsTree *StopWordsRoot = NULL;
// 特征向量树根节点
FeatureVectorTree *FeatureVectorRoot = NULL;

// 功能函数声明
// 读取一个单词
void GetWord(FILE *file);
// 读取网页标识信息
void GetWebId(FILE *file);
// 创建停用词树
void CreateStopWordsTree();
// 判断是否是停用词
int IsStopWord();
// 非停用词词频统计
void NonStopWordsCnt();
// 非停用词词频排序
void NonStopWordsSort();
// 创建特征向量树（排序后前N个信息体就是特征向量）
void CreateFeatureVectorTree(int N);
//  统计每个网页（文本）的特征向量中每个特征（单词）的频度
void WebFeatureVectorCnt(FILE *file);
// 计算网页指纹
void WebFingerprintCnt(int N, int M);
// 计算汉明距离并输出结果
void HammingDistanceCnt(int M);

// 主程序实现
int main(int argc, char **argv)
{
    // 命令行输入形式应该是：simtool N M
    if (argc == 3)
    {
        // 字符串转成整数，得到N、M的值
        int N = atoi(argv[1]);
        int M = atoi(argv[2]);
        // 步骤0：打开所有需要的文件
        StopWordsFile = fopen("stopwords.txt", "r");
        if (StopWordsFile == NULL)
        {
            printf("停用词文件打开失败！\n");
            return 1;
        }
        WebFile = fopen("article.txt", "rb");
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
        HashFile = fopen("hashvalue.txt", "rb");
        if (HashFile == NULL)
        {
            printf("Hash表文件打开失败!\n");
            return 1;
        }
        ResultFile = fopen("result.txt", "wb");
        if (ResultFile == NULL)
        {
            printf("输出结果文件打开失败!\n");
            return 1;
        }
        // 步骤1:获取各网页标识信息
        GetWebId(WebFile);
        GetWebId(SampleFile);
        // 步骤2:得到排序后的非停用词单词数组（排序后前N个信息体就是特征向量）
        CreateStopWordsTree();
        NonStopWordsCnt();
        NonStopWordsSort();
        // 步骤3:统计每个网页（文本）的特征向量中每个特征（单词）的频度,得到权重向量
        CreateFeatureVectorTree(N);
        WebFeatureVectorCnt(WebFile);
        WebFeatureVectorCnt(SampleFile);
        // 步骤4:计算各网页的指纹
        WebFingerprintCnt(N, M);
        // 步骤5:计算各网页的汉明距离
        HammingDistanceCnt(M);
        // 步骤6:关闭文件
        fclose(StopWordsFile);
        fclose(WebFile);
        fclose(SampleFile);
        fclose(HashFile);
        fclose(ResultFile);
    }
    return 0;
}
// 功能函数实现
// 读取网页标识信息
void GetWebId(FILE *file)
{
    if (file == WebFile)
    {
        int num = 0;
        int flag = 0;
        // 读取第一个网页
        fgets(webId[num++], 200, file);
        strtok(webId[num - 1], "\n");
        strtok(webId[num - 1], "\r");
        char tmp[200] = {0};
        // 以换页符为标志读取后续的网页
        while (fgets(tmp, 200, file) != NULL)
        {
            if (flag == 1)
            {
                strcpy(webId[num++], tmp);
                strtok(webId[num - 1], "\n");
                strtok(webId[num - 1], "\r");
                flag = 0;
            }
            if (strstr(tmp, "\f"))
            {
                flag = 1;
            }
        }
        fseek(file, 0, SEEK_SET);
    }
    else if (file == SampleFile)
    {
        int num = 0;
        int flag = 0;
        char tmp[200] = {0};
        fgets(tmp, 10, file);
        memset(tmp, 0, sizeof(tmp));
        fgets(sampleWebId[num++], 200, file);
        strtok(sampleWebId[num - 1], "\n");
        strtok(sampleWebId[num - 1], "\r");
        while (fgets(tmp, 200, file) != NULL)
        {
            if (flag == 1)
            {
                strcpy(sampleWebId[num++], tmp);
                strtok(sampleWebId[num - 1], "\n");
                strtok(sampleWebId[num - 1], "\r");
                flag = 0;
            }
            if (strstr(tmp, "\f"))
            {
                flag = 1;
            }
        }
        fseek(file, 0, SEEK_SET);
    }
}
// 读取一个单词,同时要把单词转换成小写
void GetWord(FILE *file)
{
    int i = 0;
    char ch;
    while ((ch = fgetc(file)) != EOF)
    {
        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
        {
            if (ch >= 'A' && ch <= 'Z')
            {
                ch = ch | 0x20;
            }
            word[i++] = ch;
        }
        else
        {
            if (ch == '\f')
            {
                pageFlag = 1;
            }
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
// 创建停用词树,用前缀字典树实现
void CreateStopWordsTree()
{
    StopWordsRoot = (StopWordsTree *)malloc(sizeof(StopWordsTree));
    StopWordsRoot->cnt = 0;
    for (int i = 0; i < 26; i++)
    {
        StopWordsRoot->chilren[i] = NULL;
    }
    StopWordsTree *p = StopWordsRoot;
    while (fscanf(StopWordsFile, "%s", word) != EOF)
    {
        p = StopWordsRoot;
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
        memset(word, 0, sizeof(word));
    }
    memset(word, 0, sizeof(word));
}
// 判断是否是停用词,是返回1,否返回0
int IsStopWord()
{
    StopWordsTree *p = StopWordsRoot;
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
// 非停用词词频统计，用Hash表实现
// 模拟MurmurHash算法的hash函数
uint64_t MurmurHash64A(const void *key, int len, uint64_t seed)
{
    const uint64_t m = 0xc6a4a7935bd1e995LLU;
    const int r = 47;

    uint64_t h = seed ^ (len * m);

    const unsigned char *data = (const unsigned char *)key;

    for (int i = 0; i < len; i++)
    {
        h = (h * m) ^ data[i];
        h = (h ^ (h >> r)) * m;
    }

    return h;
}
void NonStopWordsCnt()
{
    NonStopWordHashTable *hashTable = (NonStopWordHashTable *)malloc(sizeof(NonStopWordHashTable));
    hashTable->tableSize = MAX_TABLE_SIZE;
    hashTable->table = nonStopWords;
    int len = 0;
    GetWord(WebFile);
    len = strlen(word);
    while (len > 0)
    {
        if (IsStopWord() == 0)
        {
            uint64_t hash = 0;
            hash = MurmurHash64A(word, len, 0) % hashTable->tableSize;
            while (hashTable->table[hash].status == 1 && strcmp(hashTable->table[hash].word, word) != 0)
            {
                hash = (hash + 1) % hashTable->tableSize;
            }
            if (hashTable->table[hash].status == 0)
            {
                strcpy(hashTable->table[hash].word, word);
                hashTable->table[hash].count = 1;
                hashTable->table[hash].status = 1;
                nonStopWordsNum++;
            }
            else if (hashTable->table[hash].status == 1)
            {
                hashTable->table[hash].count++;
            }
        }
        memset(word, 0, sizeof(word));
        GetWord(WebFile);
        len = strlen(word);
    }
    memset(word, 0, sizeof(word));
}
// 非停用词词频排序,降序排列,词频相同的按字典序升序排列
// 这是qsort函数的比较函数
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
// 这是快速排序函数
void NonStopWordsSort()
{
    int i, j;
    qsort(nonStopWords, MAX_TABLE_SIZE, sizeof(NonStopWord), cmp);
}
// 创建特征向量树,用前缀字典树实现
void CreateFeatureVectorTree(int N)
{
    FeatureVectorRoot = (FeatureVectorTree *)malloc(sizeof(FeatureVectorTree));
    FeatureVectorRoot->cnt = 0;
    FeatureVectorRoot->id = 0;
    for (int i = 0; i < 26; i++)
    {
        FeatureVectorRoot->chilren[i] = NULL;
    }
    FeatureVectorTree *p = FeatureVectorRoot;
    for (int i = 0; i < N; i++)
    {
        p = FeatureVectorRoot;
        for (int j = 0; j < strlen(nonStopWords[i].word); j++)
        {
            int index = nonStopWords[i].word[j] - 'a';
            if (p->chilren[index] == NULL)
            {
                p->chilren[index] = (FeatureVectorTree *)malloc(sizeof(FeatureVectorTree));
                p->chilren[index]->cnt = 0;
                for (int k = 0; k < 26; k++)
                {
                    p->chilren[index]->chilren[k] = NULL;
                }
            }
            p = p->chilren[index];
        }
        p->cnt = 1;
        p->id = i;
    }
}
// 统计每个网页（文本）的特征向量中每个特征（单词）的频度,得到权重向量
void WebFeatureVectorCnt(FILE *file)
{
    if (file == WebFile)
    {
        fseek(file, 0, SEEK_SET);
        pageNum = 1;
        while (!feof(file))
        {
            pageFlag = 0;
            GetWord(file);
            int len = strlen(word);
            if (pageFlag == 1)
            {
                pageNum++;
                pageFlag = 0;
            }
            if (len > 0)
            {
                FeatureVectorTree *p = FeatureVectorRoot;
                int len1 = 0;
                for (int i = 0; i < len; i++)
                {
                    int index = word[i] - 'a';
                    if (p->chilren[index] == NULL)
                    {
                        break;
                    }
                    p = p->chilren[index];
                    len1++;
                }
                if (p->cnt == 1 && len1 == len)
                {
                    weight[pageNum - 1][p->id]++;
                }
            }
            memset(word, 0, sizeof(word));
        }
    }
    else if (file == SampleFile)
    {
        fseek(file, 0, SEEK_SET);
        samplePageNum = 1;
        while (!feof(file))
        {
            pageFlag = 0;
            GetWord(file);
            int len = strlen(word);
            if (pageFlag == 1)
            {
                if (!feof(file))
                {
                    samplePageNum++;
                }
                pageFlag = 0;
            }
            if (strlen(word) > 0 && strcmp(word, "sample") != 0)
            {
                FeatureVectorTree *p = FeatureVectorRoot;
                int len1 = 0;
                for (int i = 0; i < len; i++)
                {
                    int index = word[i] - 'a';
                    if (p->chilren[index] == NULL)
                    {
                        break;
                    }
                    p = p->chilren[index];
                    len1++;
                }
                if (p->cnt == 1 && len1 == len)
                {
                    sampleWeight[samplePageNum - 1][p->id]++;
                }
            }
            memset(word, 0, sizeof(word));
        }
    }
}
// 计算网页指纹
void WebFingerprintCnt(int N, int M)
{
    int i = 0;
    int j = 0;
    int k = 0;
    ll finger[128] = {0};
    char tmp[600] = {0};
    char c = 0;
    int left = M - 64;
    // 读取N行M列的Hash值，每行按二进制数存储在hashValue数组中
    for (i = 0; i < N; i++)
    {
        fread(tmp, sizeof(char), M, HashFile);
        if (tmp[M - 1] != '\n')
        {
            fread(&c, sizeof(char), 1, HashFile);
            while (c != '\n')
            {
                fread(&c, sizeof(char), 1, HashFile);
            }
        }
        if (left > 0)
        {
            for (j = 0; j < 64; j++)
            {
                if (tmp[j] == '1')
                {
                    upperHashValue[i] += (1ULL << (63 - j));
                }
            }
            for (j = 64; j < M; j++)
            {
                if (tmp[j] == '1')
                {
                    lowerHashValue[i] += (1ULL << (M - j - 1));
                }
            }
        }
        else
        {
            for (j = 0; j < M; j++)
            {
                if (tmp[j] == '1')
                {
                    lowerHashValue[i] += (1ULL << (M - j - 1));
                }
            }
        }
    }
    // 计算原网页指纹
    if (left > 0)
    {
        for (i = 0; i < pageNum; i++)
        {
            for (j = 0; j < 64; j++)
            {
                for (k = 0; k < N; k++)
                {
                    // 对应Hash值该位是1，累加上权重；对应Hash值该位是0，累减权重
                    if (weight[i][k] > 0)
                    {
                        if ((upperHashValue[k] & (1ULL << (63 - j))) != 0)
                        {
                            finger[j] += weight[i][k];
                        }
                        else
                        {
                            finger[j] -= weight[i][k];
                        }
                    }
                }
                if (finger[j] > 0)
                {
                    upperFingerprint[i] += (1ULL << (63 - j));
                }
            }
            for (j = 64; j < M; j++)
            {
                for (k = 0; k < N; k++)
                {
                    // 对应Hash值该位是1，累加上权重；对应Hash值该位是0，累减权重
                    if (weight[i][k] > 0)
                    {
                        if ((lowerHashValue[k] & (1ULL << (M - j - 1))) != 0)
                        {
                            finger[j] += weight[i][k];
                        }
                        else
                        {
                            finger[j] -= weight[i][k];
                        }
                    }
                }
                if (finger[j] > 0)
                {
                    lowerFingerprint[i] += (1ULL << (M - j - 1));
                }
            }
            memset(finger, 0, sizeof(finger));
        }
    }
    else
    {
        for (i = 0; i < pageNum; i++)
        {
            for (j = 0; j < M; j++)
            {
                for (k = 0; k < N; k++)
                {
                    // 对应Hash值该位是1，累加上权重；对应Hash值该位是0，累减权重
                    if (weight[i][k] > 0)
                    {
                        if ((lowerHashValue[k] & (1ULL << (M - j - 1))) != 0)
                        {
                            finger[j] += weight[i][k];
                        }
                        else
                        {
                            finger[j] -= weight[i][k];
                        }
                    }
                }
                if (finger[j] > 0)
                {
                    lowerFingerprint[i] += (1ULL << (M - j - 1));
                }
            }
            memset(finger, 0, sizeof(finger));
        }
    }
    // 计算样本网页指纹
    if (left > 0)
    {
        for (i = 0; i < samplePageNum; i++)
        {
            for (j = 0; j < 64; j++)
            {
                for (k = 0; k < N; k++)
                {
                    // 对应Hash值该位是1，累加上权重；对应Hash值该位是0，累减权重
                    if (sampleWeight[i][k] > 0)
                    {
                        if ((upperHashValue[k] & (1ULL << (63 - j))) != 0)
                        {
                            finger[j] += sampleWeight[i][k];
                        }
                        else
                        {
                            finger[j] -= sampleWeight[i][k];
                        }
                    }
                }
                if (finger[j] > 0)
                {
                    upperSampleFingerprint[i] += (1ULL << (63 - j));
                }
            }
            for (j = 64; j < M; j++)
            {
                for (k = 0; k < N; k++)
                {
                    // 对应Hash值该位是1，累加上权重；对应Hash值该位是0，累减权重
                    if (sampleWeight[i][k] > 0)
                    {
                        if ((lowerHashValue[k] & (1ULL << (M - j - 1))) != 0)
                        {
                            finger[j] += sampleWeight[i][k];
                        }
                        else
                        {
                            finger[j] -= sampleWeight[i][k];
                        }
                    }
                }
                if (finger[j] > 0)
                {
                    lowerSampleFingerprint[i] += (1ULL << (M - j - 1));
                }
            }
            memset(finger, 0, sizeof(finger));
        }
    }
    else
    {
        for (i = 0; i < samplePageNum; i++)
        {
            for (j = 0; j < M; j++)
            {
                for (k = 0; k < N; k++)
                {
                    // 对应Hash值该位是1，累加上权重；对应Hash值该位是0，累减权重
                    if (sampleWeight[i][k] > 0)
                    {
                        if ((lowerHashValue[k] & (1ULL << (M - j - 1))) != 0)
                        {
                            finger[j] += sampleWeight[i][k];
                        }
                        else
                        {
                            finger[j] -= sampleWeight[i][k];
                        }
                    }
                }
                if (finger[j] > 0)
                {
                    lowerSampleFingerprint[i] += (1ULL << (M - j - 1));
                }
            }
            memset(finger, 0, sizeof(finger));
        }
    }
}
// 计算各网页的汉明距离并输出结果，利用二进制数的异或运算
void HammingDistanceCnt(int M)
{
    int i = 0;
    int j = 0;
    int distance = 0;
    int left = M - 64;
    for (i = 0; i < samplePageNum; i++)
    {
        for (j = 0; j < pageNum; j++)
        {
            if (left > 0)
            {
                uint64_t tmp = upperSampleFingerprint[i] ^ upperFingerprint[j];
                while (tmp != 0)
                {
                    if ((tmp & 1) == 1)
                    {
                        distance++;
                    }
                    tmp >>= 1;
                }
                tmp = lowerSampleFingerprint[i] ^ lowerFingerprint[j];
                while (tmp != 0)
                {
                    if ((tmp & 1) == 1)
                    {
                        distance++;
                    }
                    tmp >>= 1;
                }
            }
            else
            {
                uint64_t tmp = lowerSampleFingerprint[i] ^ lowerFingerprint[j];
                while (tmp != 0)
                {
                    if ((tmp & 1) == 1)
                    {
                        distance++;
                    }
                    tmp >>= 1;
                }
            }
            if (distance == 3)
            {
                result[3][printPageNum3++] = j;
            }
            else if (distance == 2)
            {
                result[2][printPageNum2++] = j;
            }
            else if (distance == 1)
            {
                result[1][printPageNum1++] = j;
            }
            else if (distance == 0)
            {
                result[0][printPageNum0++] = j;
            }
            distance = 0;
        }
        if (i == 0)
        {
            printf("%s\n", sampleWebId[i]);
            int k = 0;
            if (printPageNum0 > 0)
            {
                printf("0:");
                for (k = 0; k < printPageNum0; k++)
                {
                    printf("%s ", webId[result[0][k]]);
                }
                printf("\n");
            }
            if (printPageNum1 > 0)
            {
                printf("1:");
                for (k = 0; k < printPageNum1; k++)
                {
                    printf("%s ", webId[result[1][k]]);
                }
                printf("\n");
            }
            if (printPageNum2 > 0)
            {
                printf("2:");
                for (k = 0; k < printPageNum2; k++)
                {
                    printf("%s ", webId[result[2][k]]);
                }
                printf("\n");
            }
            if (printPageNum3 > 0)
            {
                printf("3:");
                for (k = 0; k < printPageNum3; k++)
                {
                    printf("%s ", webId[result[3][k]]);
                }
                printf("\n");
            }
            fwrite(sampleWebId[i], sizeof(char), strlen(sampleWebId[i]), ResultFile);
            fwrite("\n", sizeof(char), 1, ResultFile);
            if (printPageNum0 > 0)
            {
                fwrite("0:", sizeof(char), 2, ResultFile);
                for (k = 0; k < printPageNum0; k++)
                {
                    fwrite(webId[result[0][k]], sizeof(char), strlen(webId[result[0][k]]), ResultFile);
                    fwrite(" ", sizeof(char), 1, ResultFile);
                }
                fwrite("\n", sizeof(char), 1, ResultFile);
            }
            if (printPageNum1 > 0)
            {
                fwrite("1:", sizeof(char), 2, ResultFile);
                for (k = 0; k < printPageNum1; k++)
                {
                    fwrite(webId[result[1][k]], sizeof(char), strlen(webId[result[1][k]]), ResultFile);
                    fwrite(" ", sizeof(char), 1, ResultFile);
                }
                fwrite("\n", sizeof(char), 1, ResultFile);
            }
            if (printPageNum2 > 0)
            {
                fwrite("2:", sizeof(char), 2, ResultFile);
                for (k = 0; k < printPageNum2; k++)
                {
                    fwrite(webId[result[2][k]], sizeof(char), strlen(webId[result[2][k]]), ResultFile);
                    fwrite(" ", sizeof(char), 1, ResultFile);
                }
                fwrite("\n", sizeof(char), 1, ResultFile);
            }
            if (printPageNum3 > 0)
            {
                fwrite("3:", sizeof(char), 2, ResultFile);
                for (k = 0; k < printPageNum3; k++)
                {
                    fwrite(webId[result[3][k]], sizeof(char), strlen(webId[result[3][k]]), ResultFile);
                    fwrite(" ", sizeof(char), 1, ResultFile);
                }
                fwrite("\n", sizeof(char), 1, ResultFile);
            }
            printPageNum0 = 0;
            printPageNum1 = 0;
            printPageNum2 = 0;
            printPageNum3 = 0;
        }
        else
        {
            fwrite(sampleWebId[i], sizeof(char), strlen(sampleWebId[i]), ResultFile);
            fwrite("\n", sizeof(char), 1, ResultFile);
            int k = 0;
            if (printPageNum0 > 0)
            {
                fwrite("0:", sizeof(char), 2, ResultFile);
                for (k = 0; k < printPageNum0; k++)
                {
                    fwrite(webId[result[0][k]], sizeof(char), strlen(webId[result[0][k]]), ResultFile);
                    fwrite(" ", sizeof(char), 1, ResultFile);
                }
                fwrite("\n", sizeof(char), 1, ResultFile);
            }
            if (printPageNum1 > 0)
            {
                fwrite("1:", sizeof(char), 2, ResultFile);
                for (k = 0; k < printPageNum1; k++)
                {
                    fwrite(webId[result[1][k]], sizeof(char), strlen(webId[result[1][k]]), ResultFile);
                    fwrite(" ", sizeof(char), 1, ResultFile);
                }
                fwrite("\n", sizeof(char), 1, ResultFile);
            }
            if (printPageNum2 > 0)
            {
                fwrite("2:", sizeof(char), 2, ResultFile);
                for (k = 0; k < printPageNum2; k++)
                {
                    fwrite(webId[result[2][k]], sizeof(char), strlen(webId[result[2][k]]), ResultFile);
                    fwrite(" ", sizeof(char), 1, ResultFile);
                }
                fwrite("\n", sizeof(char), 1, ResultFile);
            }
            if (printPageNum3 > 0)
            {
                fwrite("3:", sizeof(char), 2, ResultFile);
                for (k = 0; k < printPageNum3; k++)
                {
                    fwrite(webId[result[3][k]], sizeof(char), strlen(webId[result[3][k]]), ResultFile);
                    fwrite(" ", sizeof(char), 1, ResultFile);
                }
                fwrite("\n", sizeof(char), 1, ResultFile);
            }
            printPageNum0 = 0;
            printPageNum1 = 0;
            printPageNum2 = 0;
            printPageNum3 = 0;
        }
    }
}
