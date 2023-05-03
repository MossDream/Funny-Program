#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// 结构定义
// 非停用词单词信息体
typedef struct NonStopWord
{
    char word[50];
    int count;
} NonStopWord;

// 停用词树，用前缀树实现
typedef struct StopWordsTree
{
    int cnt;
    struct StopWordsTree *chilren[26];
} StopWordsTree;

// 特征向量树，用前缀树实现
typedef struct FeatureVectorTree
{
    int cnt;
    int id;
    struct FeatureVectorTree *chilren[26];
} FeatureVectorTree;

// 变量定义

// 原网页的非停用词单词数组
NonStopWord nonStopWords[20000] = {0};
// 样本网页的非停用词单词数组
NonStopWord sampleNonStopWords[20000] = {0};
// 原网页的非停用词数量
int nonStopWordsNum = 0;
// 样本网页的非停用词数量
int sampleNonStopWordsNum = 0;

// 原网页页数
int pageNum = 0;
// 样本网页页数
int samplePageNum = 0;
// 读到换页符的标记
int pageFlag = 0;

// 单词数组
char word[1000] = {0};
// 原网页编号数组
char webId[20000][10] = {0};
// 样本网页编号数组
char sampleWebId[20000][10] = {0};

// 各网页权重向量构成的二维数组
// 原网页权重向量
int weight[20000][1000] = {0};
// 样本网页权重向量
int sampleWeight[20000][1000] = {0};

// 各网页指纹构成的二维数组
// 原网页指纹
int fingerprint[20000][16] = {0};
// 样本网页指纹
int sampleFingerprint[20000][16] = {0};

// 样本网页对原网页的汉明距离构成的二维数组
// 横坐标是样本网页编号数，纵坐标是原网页编号数
int hammingDistance[20000][20000] = {0};

// 临时储存输出结果
int tempResult[5][20000] = {0};
// 临时储存输出结果汉明距离为0的网页编号数量
int tempResult0Num = 0;
// 临时储存输出结果汉明距离为1的网页编号数量
int tempResult1Num = 0;
// 临时储存输出结果汉明距离为2的网页编号数量
int tempResult2Num = 0;
// 临时储存输出结果汉明距离为3的网页编号数量
int tempResult3Num = 0;

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
// 原网页特征向量树根节点
FeatureVectorTree *FeatureVectorRoot = NULL;
// 样本网页特征向量树根节点
FeatureVectorTree *sampleFeatureVectorRoot = NULL;

// 功能函数声明
// 读取一个单词
void GetWord(FILE *file);
// 创建停用词树
void CreateStopWordsTree();
// 判断是否是停用词
int IsStopWord();
// 非停用词词频统计
void NonStopWordsCount(FILE *file);
// 非停用词词频排序
void NonStopWordsSort(FILE *file);
// 创建特征向量树（排序后前N个信息体就是特征向量）
void CreateFeatureVectorTree(int N, FILE *file);
//  统计每个网页（文本）的特征向量中每个特征（单词）的频度
void WebFeatureVectorCnt(FILE *file);
// 计算网页指纹
void WebFingerprintCnt(int N, int M);
// 计算汉明距离
void HammingDistanceCnt(int M);
// 输出结果
void OutputResult();
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
    HashFile = fopen("hashvalue.txt", "r");
    if (HashFile == NULL)
    {
        printf("Hash表文件打开失败!\n");
        return 1;
    }
    ResultFile = fopen("result.txt", "w");
    if (ResultFile == NULL)
    {
        printf("输出结果文件打开失败!\n");
        return 1;
    }
    // 步骤1:得到排序后的非停用词单词数组（排序后前N个信息体就是特征向量）
    CreateStopWordsTree();
    while (!feof(WebFile))
    {
        fscanf(WebFile, "%s", webId[pageNum++]);
        NonStopWordsCount(WebFile);
        NonStopWordsSort(WebFile);
        CreateFeatureVectorTree(1000, WebFile);
        WebFeatureVectorCnt(WebFile);
        WebFingerprintCnt(1000, 16);
    }

    NonStopWordsCount(SampleFile);

    NonStopWordsSort(SampleFile);
    // 步骤2:统计每个网页（文本）的特征向量中每个特征（单词）的频度,得到权重向量
    CreateFeatureVectorTree(1000, SampleFile);
    WebFeatureVectorCnt(SampleFile);
    // 步骤3:计算各网页的指纹
    // 步骤4:计算各网页的汉明距离
    HammingDistanceCnt(16);
    // 步骤5:按要求输出结果
    OutputResult();

    // 关闭文件
    fclose(StopWordsFile);
    fclose(WebFile);
    fclose(SampleFile);
    fclose(HashFile);
    fclose(ResultFile);
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
        if (ch == '\f')
        {
            pageFlag = 1;
            return;
        }
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
// 非停用词词频统计
void NonStopWordsCount(FILE *file)
{
    if (file == WebFile)
    {
        GetWord(WebFile);
        while (strlen(word) > 0 && !feof(WebFile) && pageFlag == 0)
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
            memset(word, 0, sizeof(word));
            GetWord(WebFile);
        }
        pageFlag = 0;
        memset(word, 0, sizeof(word));
    }
    else if (file == SampleFile)
    {
        GetWord(SampleFile);
        while (strlen(word) > 0 && !feof(SampleFile) && pageFlag == 0)
        {
            if (IsStopWord() == 0 && strcmp(word, "Sample") != 0)
            {
                int i;
                for (i = 0; i < sampleNonStopWordsNum; i++)
                {
                    if (strcmp(sampleNonStopWords[i].word, word) == 0)
                    {
                        sampleNonStopWords[i].count++;
                        break;
                    }
                }
                if (i == sampleNonStopWordsNum)
                {
                    strcpy(sampleNonStopWords[sampleNonStopWordsNum].word, word);
                    sampleNonStopWords[sampleNonStopWordsNum].count = 1;
                    sampleNonStopWordsNum++;
                }
            }
            memset(word, 0, sizeof(word));
            GetWord(SampleFile);
        }
        pageFlag = 0;
        memset(word, 0, sizeof(word));
    }
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
void NonStopWordsSort(FILE *file)
{
    int i, j;
    if (file == WebFile)
    {
        qsort(nonStopWords, nonStopWordsNum, sizeof(NonStopWord), cmp);
    }
    else if (file == SampleFile)
    {
        qsort(sampleNonStopWords, sampleNonStopWordsNum, sizeof(NonStopWord), cmp);
    }
}
// 创建特征向量树,用前缀树实现
void CreateFeatureVectorTree(int N, FILE *file)
{
    if (file == WebFile)
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
    else if (file == SampleFile)
    {
        sampleFeatureVectorRoot = (FeatureVectorTree *)malloc(sizeof(FeatureVectorTree));
        sampleFeatureVectorRoot->cnt = 0;
        sampleFeatureVectorRoot->id = 0;
        for (int i = 0; i < 26; i++)
        {
            sampleFeatureVectorRoot->chilren[i] = NULL;
        }
        FeatureVectorTree *p = sampleFeatureVectorRoot;
        for (int i = 0; i < N; i++)
        {
            p = sampleFeatureVectorRoot;
            for (int j = 0; j < strlen(sampleNonStopWords[i].word); j++)
            {
                int index = sampleNonStopWords[i].word[j] - 'a';
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
}
// 统计每个网页（文本）的特征向量中每个特征（单词）的频度,得到权重向量
void WebFeatureVectorCnt(FILE *file)
{
    if (file == WebFile)
    {
        fseek(file, 0, SEEK_SET);
        pageNum = 0;
        while (!feof(file))
        {
            while (fgetc(file) != '\f' && !feof(file))
            {
                fseek(file, -1, SEEK_CUR);
                pageFlag = 0;
                GetWord(file);
                if (pageFlag == 1)
                {
                    pageNum++;
                    pageFlag = 0;
                }
                if (strlen(word) > 0)
                {
                    FeatureVectorTree *p = FeatureVectorRoot;
                    for (int i = 0; i < strlen(word); i++)
                    {
                        int index = word[i] - 'a';
                        if (p->chilren[index] == NULL)
                        {
                            break;
                        }
                        p = p->chilren[index];
                    }
                    if (p->cnt == 1)
                    {
                        weight[pageNum][p->id]++;
                    }
                }
                memset(word, 0, sizeof(word));
            }
            pageNum++;
        }
    }
    else if (file == SampleFile)
    {
        fseek(file, 0, SEEK_SET);
        samplePageNum = 0;
        while (!feof(file))
        {
            while (fgetc(file) != '\f' && !feof(file))
            {
                fseek(file, -1, SEEK_CUR);
                pageFlag = 0;
                GetWord(file);
                if (pageFlag == 1)
                {
                    samplePageNum++;
                    pageFlag = 0;
                }
                if (strlen(word) > 0)
                {
                    FeatureVectorTree *p = sampleFeatureVectorRoot;
                    for (int i = 0; i < strlen(word); i++)
                    {
                        int index = word[i] - 'a';
                        if (p->chilren[index] == NULL)
                        {
                            break;
                        }
                        p = p->chilren[index];
                    }
                    if (p->cnt == 1)
                    {
                        sampleWeight[samplePageNum][p->id]++;
                    }
                }
                memset(word, 0, sizeof(word));
            }
            samplePageNum++;
        }
    }
}
// 计算网页指纹
void WebFingerprintCnt(int N, int M)
{
    int i = 0;
    int j = 0;
    int k = 0;
    int finger = 0;
    char tempHash[500] = {0};
    for (i = 0; i < N; i++)
    {
        fgets(tempHash, M * sizeof(char), HashFile);
        for (j = 0; j < M; j++)
        {
            if (tempHash[j] == '1')
            {
                fingerprint[pageNum][j] += weight[pageNum][i];
            }
            else if (tempHash[j] == '0')
            {
                fingerprint[pageNum][j] -= weight[pageNum][i];
            }
        }
        memset(tempHash, 0, sizeof(tempHash));
    }
    for (i = 0; i < M; i++)
    {
        if (fingerprint[pageNum][i] > 0)
        {
            fingerprint[pageNum][i] = 1;
        }
        else
        {
            fingerprint[pageNum][i] = 0;
        }
    }
    fseek(HashFile, 0, SEEK_SET);
    for (i = 0; i < N; i++)
    {
        fgets(tempHash, M * sizeof(char), HashFile);
        for (j = 0; j < M; j++)
        {
            if (tempHash[j] == '1')
            {
                sampleFingerprint[samplePageNum][j] += sampleWeight[samplePageNum][i];
            }
            else if (tempHash[j] == '0')
            {
                sampleFingerprint[samplePageNum][j] -= sampleWeight[samplePageNum][i];
            }
        }
        memset(tempHash, 0, sizeof(tempHash));
    }
    for (i = 0; i < M; i++)
    {
        if (sampleFingerprint[samplePageNum][i] > 0)
        {
            sampleFingerprint[samplePageNum][i] = 1;
        }
        else
        {
            sampleFingerprint[samplePageNum][i] = 0;
        }
    }
}
// 计算各网页的汉明距离
void HammingDistanceCnt(int M)
{
    int i = 0;
    int j = 0;
    int k = 0;
    int distance = 0;
    for (i = 0; i < samplePageNum; i++)
    {
        for (j = 0; j < pageNum; j++)
        {
            distance = 0;
            for (k = 0; k < M; k++)
            {
                if (sampleFingerprint[i][k] != fingerprint[j][k])
                {
                    distance++;
                }
            }
            hammingDistance[i][j] = distance;
        }
    }
}
// 按要求输出结果
void OutputResult()
{
    int i = 0;
    int j = 0;
    for (i = 0; i < samplePageNum; i++)
    {
        if (i == 0)
        {
            printf("Sample-%d\n", i + 1);
        }
        fprintf(ResultFile, "Sample-%d\n", i + 1);
        for (j = 0; j < pageNum; j++)
        {
            if (hammingDistance[i][j] == 0)
            {
                tempResult[0][tempResult0Num++] = j + 1;
            }
            else if (hammingDistance[i][j] == 1)
            {
                tempResult[1][tempResult1Num++] = j + 1;
            }
            else if (hammingDistance[i][j] == 2)
            {
                tempResult[2][tempResult2Num++] = j + 1;
            }
            else if (hammingDistance[i][j] == 3)
            {
                tempResult[3][tempResult3Num++] = j + 1;
            }
        }
        if (tempResult0Num > 0)
        {
            if (i == 0)
            {
                printf("0:");
            }
            fprintf(ResultFile, "0:");
            for (j = 0; j < tempResult0Num; j++)
            {
                if (i == 0)
                {
                    printf("1-%d ", tempResult[0][j]);
                }
                fprintf(ResultFile, "1-%d ", tempResult[0][j]);
            }
            if (i == 0)
            {
                printf("\n");
            }
            fprintf(ResultFile, "\n");
        }
        if (tempResult1Num > 0)
        {
            if (i == 0)
            {
                printf("1:");
            }
            fprintf(ResultFile, "1:");
            for (j = 0; j < tempResult1Num; j++)
            {
                if (i == 0)
                {
                    printf("1-%d ", tempResult[1][j]);
                }
                fprintf(ResultFile, "1-%d ", tempResult[1][j]);
            }
            if (i == 0)
            {
                printf("\n");
            }
            fprintf(ResultFile, "\n");
        }
        if (tempResult2Num > 0)
        {
            if (i == 0)
            {
                printf("2:");
            }
            fprintf(ResultFile, "2:");
            for (j = 0; j < tempResult2Num; j++)
            {
                if (i == 0)
                {
                    printf("1-%d ", tempResult[2][j]);
                }
                fprintf(ResultFile, "1-%d ", tempResult[2][j]);
            }
            if (i == 0)
            {
                printf("\n");
            }
            fprintf(ResultFile, "\n");
        }
        if (tempResult3Num > 0)
        {
            if (i == 0)
            {
                printf("3:");
            }
            fprintf(ResultFile, "3:");
            for (j = 0; j < tempResult3Num; j++)
            {
                if (i == 0)
                {
                    printf("1-%d ", tempResult[3][j]);
                }
                fprintf(ResultFile, "1-%d ", tempResult[3][j]);
            }
            if (i == 0)
            {
                printf("\n");
            }
            fprintf(ResultFile, "\n");
        }
        memset(tempResult, 0, sizeof(tempResult));
        tempResult0Num = 0;
        tempResult1Num = 0;
        tempResult2Num = 0;
        tempResult3Num = 0;
    }
}
