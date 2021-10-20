# DianaJSON

关注嘉然，顿顿解馋。

C++版本的简易JSON解析器实现。

## 设计
统一利用命名空间`namespace DianaJSON`进行作用域隔离。

对外提供
* Json类作为JSON解析和生成接口，
* JsonError类作为错误信息类。
* JsonValue类作为JSON数据节点。

