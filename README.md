# miniSqlFrom0
从零开始的数据库生活
好好学习, 天天commit
目前正在赶工Record Manager模块中~

## 2016.7.15
Record模块完成!!!

## 2016.8.15
所有基本模块代码编写完成!!!小数据集下测试通过,较大数据集下仍有未知bug

## How to start (Linux)
    git clone 源码
    cd 项目文件夹
    cmake .
    make 

编译完成

## Usage:

创建一个数据库

    ./dbcreate <dbname> 
    
运行minisql并使用指定的数据库 

    ./minisql <dbname>
    
删除一个数据库

    ./dbdestroy <dbname>
    
    
## Function:

* 创建数据表是支持三种数据类型:string(定长), int, float

* 仅支持定长数据存储

* 支持定义主键

* 支持基本的增删改查语句,语法与mysql相同.Insert语句不支持隐式数据转换,插入浮点型数据务必带有小数点,否则会带来难以预计的错误

* 支持在主键上创建索引

* 显示数据库中的所有表名

    `
    help;
    `
    
* 显示某个表的表结构

    `
    help <relName>;
    `
    
* 打印出一个表中的所有元祖

    `
    print <relName>;
    `
    
* 批量导入数据到表中, 文件中的数据格式以逗号分割,每一行为一条记录
 
    `
    load <relName>(<fileName>);
    `
    
更详细的说明请见doc文件夹 




