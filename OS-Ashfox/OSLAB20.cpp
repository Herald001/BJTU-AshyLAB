#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
using namespace std;

const int DISK_SIZE = 1048576; // 1 MB
const int dirtable_max_size = 80;
const int fatnum = 80;
char Disk[DISK_SIZE];
int dbr[2];
int startindex;

struct fcb {
    short blockindex; // FAT中的块索引
    short filesize;   // 文件总大小
    short datasize;   // 当前数据大小
};

struct dirunit {
    char filename[80];
    char type;         // 0: 目录, 1: 文件
    short startfat;
    short startfcb;
    short startdir;
};

struct dirtable {
    int dirunitnum;   // 目录单元数量
    dirunit dirs[dirtable_max_size];
};

short fat[fatnum];          // FAT 表
// free: 0, last: -1, next: index

dirtable* table[10];        // 目录表
fcb* FCB[fatnum];           // FCB 表

dirtable rootdirtable;      // 根目录表
dirtable* currentdirtable;  // 当前目录表
char path[100] = "root\\";
int FCBrecord = 0; // 当前创建的 FCB 索引
int TABrecord = 0; // 当前创建的目录表索引

// 函数原型
void init();
void createfile(char filename[], int filesize, int type);
void adddirunit(char fileName[], int type);
int findunit(char filename[]);
void ls();
void deletefile(char filename[]);
void deleteunit(int unitindex);
short findfreefat();
void freefat(char filename[]);
void changedir(char dirname[]);
void read(char filename[], int length);
void write(char filename[], char content[]);
void adjustfat(short num, int unitindex);
void formatdisk();
void savetoimage(const char* imagefile);
void deletedir(char dirname[]);
void renamedir(char oldname[], char newname[]);

void init() {
    memset(Disk, 0, sizeof(Disk));
    memset(fat, 0, sizeof(fat));
    rootdirtable.dirunitnum = 0;
    currentdirtable = &rootdirtable;
    dbr[0] = fatnum;
    dbr[1] = dirtable_max_size;
    startindex = 8 + fatnum; // 计算数据块的起始索引
    strcpy(path, "root\\");
    cout << "磁盘初始化完成。" << endl;
}

void savetoimage(const char* imagefile) {
    ofstream file(imagefile, ios::binary);
    if (!file) {
        cerr << "无法创建磁盘映像文件。" << endl;
        return;
    }

    // 第一步：写入 DBR（引导扇区）
    char dbr[512] = {0}; // 初始化为零
    dbr[0] = 0xEB;       // 跳转指令
    dbr[1] = 0x3C;
    dbr[2] = 0x90;
    memcpy(dbr + 3, "FAT16   ", 8); // OEM 名称

    // 设置 BIOS 参数块（BPB）
    *((short *)(dbr + 11)) = 512;      // 每扇区字节数
    dbr[13] = 1;                       // 每簇扇区数
    *((short *)(dbr + 14)) = 1;        // 保留扇区数
    dbr[16] = 2;                       // FAT 表个数
    *((short *)(dbr + 17)) = 224;      // 根目录最大条目数
    *((short *)(dbr + 19)) = 2880;     // 总扇区数（标准的 1.44MB 磁盘）
    dbr[21] = 0xF8;                    // 媒体描述符
    *((short *)(dbr + 22)) = 9;        // 每个 FAT 表的扇区数

    // 文件系统签名
    memcpy(dbr + 510, "\x55\xAA", 2); // 引导扇区签名

    file.write(dbr, 512); // 写入引导扇区

    // 第二步：写入 FAT 表
    char fat[512 * 9] = {0}; // 9 个扇区用于 FAT 表
    fat[0] = 0xF8;           // 媒体描述符
    fat[1] = 0xFF;           // 保留簇
    fat[2] = 0xFF;           // 保留簇

    // 写入两个 FAT 表副本
    file.write(fat, sizeof(fat)); // 第一个 FAT
    file.write(fat, sizeof(fat)); // 第二个 FAT

    // 第三步：写入根目录表
    int root_dir_size = currentdirtable->dirunitnum * 32;
    char* root_dir = new char[root_dir_size];
    memset(root_dir, 0, root_dir_size); // 初始化根目录表

    for (int i = 0; i < currentdirtable->dirunitnum; ++i) {
        const dirunit& dir = currentdirtable->dirs[i];
        char entry[32] = {0}; // 每个条目 32 字节

        // 拷贝文件名（填充为 11 个字符）
        strncpy(entry, dir.filename, min(11, (int)strlen(dir.filename)));

        entry[11] = dir.type == 0 ? 0x10 : 0x20; // 属性：0x10 表示目录，0x20 表示文件
        *((short *)(entry + 26)) = dir.startfat; // 起始簇
        *((int *)(entry + 28)) = dir.startfcb;  // 文件大小（简化）

        // 写入根目录条目
        memcpy(root_dir + i * 32, entry, 32);
    }

    file.write(root_dir, root_dir_size); // 写入根目录表
    delete[] root_dir;  // 释放内存

    // 第四步：写入数据区
    file.write(Disk, sizeof(Disk)); // 写入数据块

    file.close();
    cout << "磁盘映像已保存到 " << imagefile << endl;
}

void createfile(char filename[], int filesize, int type) {
	if (strlen(filename) > 80) {
		cout << "文件名过长" << endl;
		return;
	}
	// 添加目录单元
	adddirunit(filename, type);
	int index = findunit(filename);
	// 创建 FCB
	fcb* curfcb = (fcb*)new fcb();
	curfcb->blockindex = startindex++;
	curfcb->filesize = filesize;
	curfcb->datasize = 0; // 文件为空
	FCB[FCBrecord] = curfcb;
	currentdirtable->dirs[index].startfcb = FCBrecord;
	// 创建 FAT
	int i = findfreefat();
	currentdirtable->dirs[index].startfat = i;
	fat[i] = -1;
	// 创建目录
	if (type == 0) {
		dirtable* curdirtable = (dirtable*)new dirtable();
		table[TABrecord] = curdirtable;
		curdirtable->dirunitnum = 0;
		currentdirtable->dirs[index].startdir = TABrecord;
		cout << "目录创建成功: " << filename << endl;
		return;
	}
	cout << "文件创建成功: " << filename << endl;
}

void adddirunit(char filename[], int type) {
	int dirunitnum = currentdirtable->dirunitnum;
	// 是否已满
	if (dirunitnum == dirtable_max_size) {
		cout << "目录表已满，请删除一些文件后重试" << endl;
		return;
	}

	// 是否已存在
	if (findunit(filename) != -1) {
		printf("文件已存在\n");
		return;
	}
	// 创建新的目录单元
	dirunit* newdirunit = &currentdirtable->dirs[dirunitnum];
	currentdirtable->dirunitnum++;
	int i = strlen(filename);
	while (i--)
		newdirunit->filename[i] = filename[i];
	newdirunit->type = type;
	cout << "目录单元添加成功: " << filename << endl;
	return;
}

int findunit(char filename[]) {
	// 顺序查找
	int dirunitnum = currentdirtable->dirunitnum;
	int unitIndex = -1;
	for (int i = 0; i < dirunitnum; i++)
		if (strcmp(filename, currentdirtable->dirs[i].filename) == 0)
			unitIndex = i;
	return unitIndex;
}

void ls() {
	int uninum = currentdirtable->dirunitnum;
	for (int i = 0; i < uninum; i++) {
		dirunit curunit = currentdirtable->dirs[i];
		cout << curunit.filename << " ";
	}
	cout << endl;
	cout << "目录列表显示完成" << endl;
}

void deletefile(char filename[]) {
	int unitindex = findunit(filename);
	if (unitindex == -1) {
		cout << "文件未找到" << endl;
		return;
	}
	// 删除目录表中的单元
	deleteunit(unitindex);
	freefat(filename);
	cout << "文件删除成功: " << filename << endl;
}

void deleteunit(int unitindex) {
	// 让下一个单元覆盖
	int dirunitnum = currentdirtable->dirunitnum;
	for (int i = unitindex; i < dirunitnum - 1; i++) {
		currentdirtable->dirs[i] = currentdirtable->dirs[i + 1];
	}
	currentdirtable->dirunitnum--;
}

short findfreefat() {
	for (short i = 0; i < fatnum; i++) {
		if (fat[i] == 0) return i;
	}
	return -1; 
}

void freefat(char filename[]) {
	// 查找 FAT 链并释放
	int i = currentdirtable->dirs[findunit(filename)].startfat;
	if (i == -1) {
		fat[i] = 0;
		return;
	}
	while (i == -1) {
		int temp = i;
		i = fat[i];
		fat[temp] = 0;
	}
	if (i == -1) {
		fat[i] = 0;
		return;
	}
	cout << "FAT释放成功: " << filename << endl;
}

void changedir(char dirname[]) {
	// 检查目录名是否有效
	int unitindex = findunit(dirname);
	if (unitindex == -1) {
		cout << "目录未找到" << endl;
		return;
	}
	if (currentdirtable->dirs[unitindex].type == 1) {
		cout << "不是目录" << endl;
		return;
	}
	// 切换当前目录
	int i = currentdirtable->dirs[unitindex].startdir;
	currentdirtable = table[i];
	// 修改路径
	if (strcmp(dirname, "..") == 0) {
		// 返回上级目录
		int length = strlen(path);
		for (int i = length - 2; i >= 0; i--) {
			if (path[i] == '\\') {
				path[i + 1] = '\0';
				break;
			}
		}
	}
	else {
		strcat(path, dirname);
		strcat(path, "\\");
	}
	cout << "切换到目录 (" << dirname << ") 成功" << endl;
}

void read(char filename[], int length) {
	int unitindex = findunit(filename);
	if (unitindex == -1) {
		cout << "sorry,not found" << endl;
		return;
	}

	//read the data of given length 
	int index = currentdirtable->dirs[unitindex].startfcb;
	fcb* myfcb = FCB[index];
	for (int i = 0; i < length; i++) {
		cout << Disk[i + myfcb->blockindex];
	}
	cout << endl;
	cout << "文件内容读取完成！" << endl; 
}

void write(char filename[], char content[]) {
	int unitindex = findunit(filename);
	if (unitindex == -1) {
		cout << "sorry,not found" << endl;
		return;
	}
	int length = sizeof(content);
	//how many clusters need
	int num = (length % 32 == 0) ? length / 32 : length / 32 + 1;
	adjustfat(num, unitindex);

	//renew the filesize
	FCB[currentdirtable->dirs[unitindex].startfcb]->filesize = num;

	//get the start index and write the content in order
	int index = currentdirtable->dirs[unitindex].startfcb;
	fcb* myfcb = FCB[index];
	for (int i = 0; i < length; i++) {
		Disk[i + myfcb->blockindex] = content[i];
	}
	cout << endl;
	cout << "文件内容写入完成！" << endl; 
}

void adjustfat(short num, int unitindex) {
	int index = currentdirtable->dirs[unitindex].startfat;
	for (int i = 0; i < num - 1; i++) {
		short j = findfreefat();
		fat[index] = j;
		index = j;
	}
	fat[index] = -1;
}

void deletedir(char dirname[]) {
    int index = findunit(dirname);
    if (index == -1) {
        cout << "目录未找到." << endl;
        return;
    }
    if (currentdirtable->dirs[index].type != 0) {
        cout << "不是一个目录." << endl;
        return;
    }
    deleteunit(index);
    cout << "目录 " << dirname << " 已删除." << endl;
}
void renamedir(char oldname[], char newname[]) {
	// 查找目录并重命名
	int unitindex = findunit(oldname);
	if (unitindex == -1) {
		cout << "目录未找到" << endl;
		return;
	}
	if (currentdirtable->dirs[unitindex].type == 1) {
		cout << "这是文件，不能重命名为目录" << endl;
	}
	int i = 0;
	while (i < 80) {
		currentdirtable->dirs[unitindex].filename[i] = newname[i];
		if (newname[i] == '\0') break;
		i++;
	}
	cout << "目录重命名成功: " << oldname << " -> " << newname << endl;
}


int main() {
    init();
    int choice;
    char name[80] = {0};
    char newname[80] = {0};
    char content[100] = {0};
    int length = 0;
	printf("========================================================================================\n");
    printf("|                          \033[1;36m实验课题六-FAT文件系统模拟设计与实现\033[0m                         |\n");
    printf("|---------------------------------------------------------------------------------------|\n");
    printf("|                          \033[1;32m个人信息：姓名-Ashyfox 学号-22231044\033[0m                          |\n");
    printf("|---------------------------------------------------------------------------------------|\n");
    printf("|==================================== FAT16 文件系统 ===================================|\n");
    printf("| 1. 格式化磁盘 | 12. 保存磁盘映像  | 0. 退出                                           |\n");
    printf("|---------------------------------------------------------------------------------------|\n");
    printf("| 目录管理：                                                                            |\n");
    printf("| 2. 创建目录   | 3. 切换目录       | 4. 重命名目录   | 5. 显示目录  | 6. 删除目录      |\n");
    printf("|---------------------------------------------------------------------------------------|\n");
    printf("| 文件管理：                                                                            |\n");
    printf("| 7. 创建文件   | 8. 重命名文件     | 9. 写入文件     | 10. 显示文件  | 11. 删除文件    |\n");
    printf("========================================================================================\n");
    while (true) {
    	printf("------------------------------------------------------\n");
        printf("当前磁盘：%s   当前路径：%s\n","DHHOS", path);        
        printf("请选择操作: ");
        scanf("%d", &choice);
        getchar(); // Clear input buffer

        switch (choice) {
            case 0:
                printf("退出系统，感谢使用！\n");
                return 0;
            case 1:
                init();
                break;
            case 2:
                printf("输入目录名: ");
                scanf("%s", name);
                createfile(name, 1, 0);
                break;
            case 3:
                printf("输入目录名: ");
                scanf("%s", name);
                changedir(name);
                break;
            case 4:
                printf("输入原目录名: ");
                scanf("%s", name);
                printf("输入新目录名: ");
                scanf("%s", newname);
                renamedir(name, newname);
                break;
            case 5:
                ls();
                break;
            case 6:
                printf("输入目录名: ");
                scanf("%s", name);
                deletedir(name);
                break;
            case 7:
                printf("输入文件名: ");
                scanf("%s", name);
                createfile(name, 1, 1);
                break;
            case 9:
                printf("输入文件名: ");
                scanf("%s", name);
                printf("输入内容: ");
                scanf("%s", content);
                write(name, content);
                break;
            case 10:
                printf("输入文件名: ");
                scanf("%s", name);
                printf("输入读取长度: ");
                scanf("%d", &length);
                read(name, length);
                break;
            case 11:
                printf("输入文件名: ");
                scanf("%s", name);
                deletefile(name);
                break;
            case 12:
                savetoimage("mydisk.img");
                break;
            default:
                printf("无效选项，请重新选择。\n");
        }
    }
}






