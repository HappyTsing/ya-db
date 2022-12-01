# Yet Another Database

参考 InnoDB 引擎，基于 B+ 树的简易数据库，支持百列，百万量级的数据的插入和搜索，当创建数据表时，将创建同名的 `../tables/tableName.table` 文件，存储数据表的索引和数据。

> 通过修改数据表元数据分配的字节数，可支持千、万级别的列数。
>
> 目前默认分配 8192 字节。

# Quick Start

```shell
cmake .
make
./ya-db
```

![ya-db](https://happytsing-figure-bed.oss-cn-hangzhou.aliyuncs.com/ya-db/ya-db.png)

> 当前列的属性都默认为 int64_t

## More Detail

You may need something in dir `/docs`, like [YA-DB-Design](docs/YA-DB-Design.md)
