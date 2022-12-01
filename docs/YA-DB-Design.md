# 课设要求

**总体要求：**

- 存储一张表，然后能对该表进行查询、添加等操作。上述功能以API的形式提供给应用使用。

**存储要求：**

- 利用已学的文件操作API，在文件系统中存储一张表；

- 该表有100个属性，每个属性都是int64_t类型；

- 需要支持的最大行数为1百万行。

**添加要求：**

- 提供API函数，实现向表格添加一行的功能（添加到表格的末尾）。

**搜索要求：**

- 提供API函数，实现对表格的某一个属性进行范围查找的功能。例如：
  - 查找在属性A上，大于等于50，小于等于100的所有行


- 用户可以指定在哪一个属性上进行搜索；

- 当搜索结果包含的行数过多时，可以只返回一小部分，如10行等。

**索引要求：**

- 提供API函数，为表格的某一个属性建立索引结构，以实现快速搜索；
- 自行选择使用哪种数据结构，建立索引结构，比如B+树等；
- 建立的索引结构，需要保存到一个文件中（索引文件）；下次重启应用程序，并执行搜索任务时，应先检查是否已为相应属性建立了索引结构；

- 即，搜索功能实现时，需要查找是否有索引文件存在，若有，则使用该文件加速搜索。

**并发要求：**

- 应用程序可以以多线程的方式，使用我们提供的上述API；

- 要保证多线程环境下，表、索引结构、索引文件的一致性。

**测试要求：**

- 表中的数据随机生成；

- 测试用例要覆盖主要的需求；

**其他：**

- 插入、删除、修改表中的数据不做要求。

- 要求使用C或C++语言、应用现代程序设计思想。

# MySQL调研

## 前置知识

查阅 [计算机存储术语: 扇区，磁盘块，页](https://zhuanlan.zhihu.com/p/117375905) 可知，硬盘的读写以**扇区**（512字节）作为基本单位，而文件系统读写数据的最小单位是扇区簇，一般以8个扇区组合在一起，即4096字节（4kb），称为**磁盘块**，注意磁盘块是操作系统所使用的逻辑概念。

为了更好地管理磁盘空间和更高效地从硬盘读取数据，操作系统规定一个磁盘块中只能放置一个文件，因此文件所占用的空间，只能是磁盘块的整数倍，那就意味着会出现文件的实际大小，会小于其所占用的磁盘空间的情况。

**页**是内存的最小存储单位。页的大小通常为磁盘块大小的 2^n 倍，一般来说页的大小也是4096字节（4kb）。

 Linux 会以页为单位管理内存，无论是将磁盘中的数据加载到内存中，还是将内存中的数据写回磁盘，操作系统都会以页面为单位进行操作，哪怕我们只向磁盘中写入一个字节的数据，我们也需要将整个页面中的全部数据刷入磁盘中。

页和磁盘块都是逻辑单位：

- 页，内存操作的基本单位
- 磁盘块，磁盘操作的基本单位

## MyISAM

MyISAM引擎实现索引，均采用采用非聚集索引。

MyISAM 没有表空间的概念，表数据都放在对应的数据库子目录下，使用 MyISAM 创建数据表后，为每个**数据表（Table）构建一个文件夹**，文件夹下有三个文件：

- .frm：存储表结构。
- .myd：存储表数据
- .myi：存储表索引

![MyISAM](https://happytsing-figure-bed.oss-cn-hangzhou.aliyuncs.com/ya-db/MyISAM.png)

## InnoDB

InnoDB引擎实现索引，主键索引采用聚集索引，其余采用非聚集索引。B+树的叶节点的data，当主键索引时，叶节点存储数据，否则存储对应的主键的ID，再次调用主键索引查询。

InnoDB 表空间（Tablespace）是一个抽象的逻辑概念，InnoDB 把数据保存在表空间，本质上是一个或多个磁盘文件组成的虚拟文件系统。MySQL为InnoDB引擎设计了很多表空间，例如系统表空间、独立表空间、undo表空间等，简单介绍前两者：

- 系统表空间：默认情况下，Innodb会在数据目录下创建一个名为ibdata1的系统共享表空间，大小为12M的文件。如果我们配置了参数 `innodb_file_per_table`，则每张表内的数据可以单独放到一个表空间内。MySQL5.5.7到MySQL5.6.6之间的各个版本中，我们表中的数据都会被默认存储到这个系统表空间。

- 独立表空间：MySQL5.6.6之后的版本中，Mysql会把使用了Innodb引擎的数据表存储到数据表对应的独立表空间中去，数据表的独立表空间文件的名字和数据表名相同，拓展名为.ibd。

  当创建数据表时，为每个**数据表（Table）构建一个文件夹**，文件夹下有两个文件：

  - .frm：存储表结构。（此处直接写死即可）
  - .ibd：存储表索引+数据，为每个**数据表（Table）**单独存储为一个 `tableName.ibd` 文件。

![InnoDB表空间](https://happytsing-figure-bed.oss-cn-hangzhou.aliyuncs.com/ya-db/InnoDB表空间.png)

以独立表空间，即 `.ibd` 为例，其又分为多个段，段默认由 64 个页组成，页默认16KB。

<u>InnoDB 为什么提出区的概念？</u>**InnoDB将数据顺序储存来尽可能高的使用顺序IO提高磁盘加载速度**，使用区的概念是为了更大的数据顺序存储，在插入大量数据的时候，Innodb还可以申请多个连续的区来存放用户数据，这样，多个区就又顺序存储。

> 已知硬盘读写以扇区（512Bytes）为单位，而文件系统（例如Linux）读写数据的最小单位是磁盘块（扇区簇，通常8个扇区，即 4KB）。此外，磁盘与内存交互的最小单位是页（内存页），其大小通常是磁盘块的 2^n 倍，绝大多数情况下也是 4KB。

当 InnoDB 访问数据页发现未加载时，发起一次文件IO，读取一个 InnoDB 页（16KB），由于单个数据页在.idb文件上是连续的，所以加载一次数据页只需要一次文件IO。

> InnoDB 页是 16KB，而内存页是 4KB，二者冲突吗？
>
> 一般来说不冲突，由于InnoDB 页是连续存储的，因此只需要一次IO即可读取。

<u>InnoDB 为什么提出段的概念？</u>

**InnoDB引擎是将数据和索引都存储成一个B+树结构，也就是索引和数据是放在一起的(聚簇索引)**，存储索引的页和存储数据的页是不同类型的，在表空间中，索引和数据也是分开在不同的段中存储的，另外还有关于事物的处理，也有不同的段用来存储数据。存储用户数据记录的叫数据段，存储索引的叫索引段。这样就实现了对特定数据的区分管理。

InnoDB是以页为单位将磁盘数据加载到内存中，然后进行查询、修改，之后再以页为单位，将修改过的数据刷到磁盘上。因此，InnoDB中的每个数据节点大小都是 16K。

**搜索 id = n 的记录行**，其代码的实现逻辑是：

- 首先从 `.ibd` 文件中读取根节点，通过比较根节点中的 key 的大小，获取下一个需要从磁盘中读取的非叶节点的地址

  > 地址可以是.ibd文件的偏移量，非页节点即索引节点，位于 .idb 文件的索引段中。

- 基于此偏移量再次读出16k（即一页），该数据就是第二层节点的数据，依次重复，最终获取了叶节点的地址

  > 叶节点中存储了大量的记录行，位于.idb文件的数据段中。

- 若是主键索引，其叶节点中存储了大量的记录行，这些记录行也是按顺序排序，通过（maybe二叉法）就可以快速定位到搜索的目标行。

- 若是非主键索引，其叶节点中仅存储主键的值，需要再次通过主键索引获取数据。即使用非主键索引时，需要检索两遍索引：首先检索辅助索引获得主键，然后用主键到主索引中检索获得记录。

主键索引：

![InnoDB主键索引](https://happytsing-figure-bed.oss-cn-hangzhou.aliyuncs.com/ya-db/InnoDB主键索引.png)

非主键索引：

![InnoDB非主键索引](https://happytsing-figure-bed.oss-cn-hangzhou.aliyuncs.com/ya-db/InnoDB非主键索引.png)



# B+ 树

> key，又称为元素、关键字，也可以理解为索引，如数据库中的主键id？

B+树有两种类型的节点：

- 内部节点：即非叶子节点，不存储数据，只存储索引。
- 叶子结点：存储索引和对应的数据，数据都存储在叶子结点。

B+树的阶数：孩子数目的最大值

m 阶的 B+ 树的定义：

- 节点（内部节点与叶子节点）最多包含 m -1 个索引 key

- 内部节点都至少包含 `⌈m/2⌉` 个孩子，最多有 `m` 个孩子。
- 叶子节点最多包含 m - 1 个数据 value
- 所有叶子都在相同的高度上，叶结点本身按关键字大小，从小到大链接。

b+树的定义的争议：

- 孩子数和 key 的数目相同
- 同B树一样，孩子数比 key 数多一个。本次选择这个！

此外，根节点比较特殊：

- 当 b+ 树只有一个节点时，根节点 = 叶子结点

只实现B+树的插入和搜索功能。

# YA-DB 设计

参考InnoDB索引即数据的思想，但由于最多仅需要支持100列 int64_t ，100w 行数据的存储，因此不做分段、区、页的操作。

使用聚集索引，即主键索引的叶子节点中存储数据，而非主键索引的叶子节点仅存储其对应的主键值，需要通过主键索引二次查找才能获取数据。

根据B+树的阶数，固定索引节点的大小，例如 5 阶的树，索引节点最大占用 104KB，因此当新建索引节点时，为其分配 104 KB 的空间。当然，索引节点未必会用完所有空间，但未使用的空间将为其永久保留。

以 5 阶的 b+ 树为例：

**内部节点：**

```
Type_t type;                        8个字节
off64_t self;                       8个字节
int64_t keyNums;                    8个字节
off64_t father;                     8个字节
key_t keys[MAX_KEY];                MAX_KEY * 8 = 32 字节

off64_t children[MAX_CHILDREN];     MAX_CHILDREN * 8 = 40 字节
```

内部节点大小不会发生变化，当5阶时，大小 =  4*8 + 32 + 40 = 104。

**主键索引叶子节点：**

```
Type_t type;                        8个字节
int64_t columnNums                  8个字节
off64_t self;                       8个字节
int64_t keyNums;                    8个字节
off64_t father;                     8个字节
key_t keys[MAX_KEY];                MAX_KEY * 8 = 32 字节

off64_t rightBrother;               8个字节
Record values[MAX_VALUE];           MAX_VALUE * recordSize * 8
```

一旦表格创建完毕，此时表格有多少列也可以确定，即`recordSize`也已经确定。

> 如果想新建一列，则非常麻烦！对数据重新移动？此处先不考虑。

叶子节点大小也可以计算出，当5阶时，大小 = 40 + 32  + 8 + 4 * recoredSize *8

当表格有4列时，大小 = 208。

当15阶时，100列时，recordSize = 100，MAX_VALUE = 14，由此可得，主键索引叶子节点大小为：11360 bytes

**非主键索引叶子节点：**

非主键索引叶子节点仅存储主键id，此时recordSize = 1，可得非主键索引叶子节点大小为：272 bytes

# 示例图

5 阶B+树，插入5条数据，并为age建立非主键索引后，`.table`文件示意图。

![bpt_search](https://happytsing-figure-bed.oss-cn-hangzhou.aliyuncs.com/ya-db/bpt_search.png)

当再次插入2条数据后，`.table`文件示意图。

![bpt_insert](https://happytsing-figure-bed.oss-cn-hangzhou.aliyuncs.com/ya-db/bpt_insert.png)

# 测试

1. 启动系统

```sh
cmake .
make
./ya-db
```

![ya-db](https://happytsing-figure-bed.oss-cn-hangzhou.aliyuncs.com/ya-db/ya-db.png)

2. 创建表 FuncTest，并为其初始化插入1000条数据。将创建同名的持久化文件 `../tables/FuncTest.table`

```sql
CREATE TABLE FuncTest id columnOne columnTwo columnThree
SHOW TABLES
initFuncTest   // 为 FuncTest 表插入1000条数据，测试用函数，直接命令界面输入即可调用
```

3. 为 columnOne创建索引，并再次插入一条数据，将同步更新索引

```sql
CREATE INDEX ON FuncTest columnOne
INSERT INTO FuncTest 1001 99999999 1001 10001
```

4. 精准搜索

```sql
SELECT * FROM FuncTest WHERE id = 900000
SELECT * FROM FuncTest WHERE columnOne = 99999999
SELECT * FROM FuncTest WHERE columnTwo = 1001
```

5. 范围搜索

```sql
SELECT * FROM FuncTest WHERE id in 5 10
SELECT * FROM FuncTest WHERE columnOne in 3000000 10000000
SELECT * FROM FuncTest WHERE columnTwo in 5 10
```

# 参考

- MySQL
  - [MySQL 是如何查找数据的？| 入门](https://mp.weixin.qq.com/s/JtHf0AEr7BmapGFFd1tegQ)
  - [⭐️MySQL 磁盘存取 | InnoDB 引擎数据存储 | 深入](https://juejin.cn/post/6844903970989670414)
  - [⭐️MySQL 为什么选择 B+ 树？](http://blog.codinglabs.org/articles/theory-of-mysql-index.html)
  - [数据表与实际磁盘文件的对应关系 | InnoDB + MyISAM](https://cloud.tencent.com/developer/article/1927209)
  - [⭐️ 访问磁盘的次数 | MyISAM + InnoDB](https://www.51cto.com/article/629382.html)
  - [⭐️InnoDB 表空间](https://blog.csdn.net/u010647035/article/details/105009979)

- B+ 树
  - [5阶 B+ 树的插入 | 图解](https://blog.csdn.net/Weixiaohuai/article/details/109493541)
  - [4阶 B+ 树的插入 | 图解](https://xie.infoq.cn/article/0c805169b11aba5d31da2054b)
  - [⭐️C++ 实现 B+ 树 | 内存 B+ 树](https://www.cnblogs.com/yangj-Blog/p/12992124.html)
  - [Java 实现 B+ 树 | 内存 B+ 树](https://blog.csdn.net/qq_41891805/article/details/104719426)
  - [B+ 树在磁盘的存储](https://www.zhihu.com/question/269033066)

- Linux
  - [计算机存储术语: 扇区，磁盘块，页](https://zhuanlan.zhihu.com/p/117375905)
  - [LInux 文件系统 IO | API](https://juejin.cn/post/7000378455124623391)

