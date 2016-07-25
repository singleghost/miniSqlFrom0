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
