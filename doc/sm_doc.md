## Command Line Utilities
实现了三个命令行工具
* dbcreate DBname
* dbdestroy DBname
* minisql DBname

dbcreate命令会在当前目录下创建一个文件夹,文件夹的名字与DBname相同,同时在文件夹内创建relcat、attrcat两个catalog文件

dbdestroy命令会删除以DBname为名的文件夹

minisql命令则开启minisql的交互式命令行,同时数据库被指定为DBname

## System Utilities
> load relName("FileName");

FileName应该是一个完整的路径。file以ASCII的方式储存了能被载入到数据表的数据

Integer attribute values are specified in the ASCII load file as, e.g., 10 or -5, float values are specified as, e.g., 3.5E-3, and you may assume these values are in the right format. Character string values are specified as, e.g., Smith (without quotes). You may assume that character string attribute values will not contain commas. Character strings in the load file can be of any length up to the length specified for the corresponding attribute, including zero length (no characters for that field in the load file). If a character string is too long, just truncate it.

> help; or help relName;

If relName is not specified, then the help utility prints the names of all relations in the database. (You may include additional information if you like.) If a relName is specified, then the help utility prints the name, type, length, and offset of each attribute in the specified relation, together with any other information you feel may be useful.

> print relName;

The print utility displays the current set of tuples in relation relName.

> set Param = "Value";

The set utility allows the user to set system parameters without needing to recompile the system, or even exit the RedBase prompt. You should determine if there are any parameters you might find useful to control in this manner. (Examples might be level of tracing information produced, debugging flags, etc.) You are not required to implement set for any particular parameters; we're just providing the necessary "hooks" in case you find it convenient to do so.
## System Manager
system catalogs包括relcat和attrcat:

relcat的表结构如下:
--------------------
属性名       | 注释
------------|-------------
relName	    | relation name
tupleLength	| tuple length in bytes
attrCount	| number of attributes
indexCount	|number of indexed attributes

attrcat的表结构如下
---------------------
属性名       | 注释
------------|-------------
relName	    | this attribute's relation
attrName    |	attribute name
offset	    |offset in bytes from beginning of tuple
attrType    |	attribute type
attrLenth   |	attribute length
indexNo	    |index number, or -1 if not indexed
