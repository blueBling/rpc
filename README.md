## 1.简介
- 这是一个c++高性能rpc框架，该项目源于[yazi-rpc](https://github.com/125yyh/yazi-rpc)，感谢作者的贡献；
- 我这里主要对其添加了CImPdu作为远程过程调用函数复杂类型参数的通用数据结构，
CImPdu传递的是protobuf序列化后的数据，这样业务的复杂数据构造解析直接交给protobuf去处理就行了；
- rpc调用是基于socket通讯来完成的，这里我添加了更为上层的http来通讯；
- 这两个功能主要用来检验自己对rpc以及yazi-rpc的学习程度，这里分享出来。

## 2.新特性
- **CImPdu**
这个借鉴于TeamTalk通讯数据包的构造与解析；
CImPdu结构为自定义头+protobuf序列化后的消息具体可以参考[TeamTalk IM_PDUBASE详解](https://blog.csdn.net/aixiaoql/article/details/122669517)。
yazi-rpc这里核心比较重要的一块，我认为将函数+参数序列化成DataStream以及DataStream向函数+参数逆序列化的过程，
DataStream是类型+长度+数据的格式，可以参看作者天天写乱码[视频](https://space.bilibili.com/193137215)；
项目里我用protoc构造了IMLoginReq和IMLoginRes两个消息，具体可以看pb里.proto文件，这里我主要在DataStream里添加了CImPdu的序列化和反序列化功能，用IMLoginReq和IMLoginRes做了测试通过。
CImPdu更通用因为CImPdu直接存的是CSimpleBuffer(与DataStream类似都是字节流)，无需关心里面有string还是有几个int float等数据类型，这里直接交给了protobuf去处理，复杂数据自己只要修改.proto文件即可，yazi-rpc需要将这些int float string有多少就要调用SERIALIZE传递多少，我这里直接穿个CImPdu即可。

- **HTTP**
yazi-rpc服务端只支持传递过来的tcp消息解析，我这里扩展了可以进行http解析，用的Nginx里http的解析源码，用curl构造的http client。


## 3.麻雀虽小五脏俱全
| 模块 | 简介 |
| ------ | ----------- |
| config | server.ini配置rpc服务器参数，如ip:port最大连接数等 |
| http | rpc服务端http解析 |
| pb | IMLoginReq和IMLoginRes业务数据构造 |
| pdu | CImPdu承载rpc调用的复杂函数参数的通用载体 |
| rpc | rpc client和 rpc server的创建 |
| serialize | DataStream和函数+参数的序列化和反序列化 |
| socket | 高性能的epoll |
| task | 传递给工作线程的工作任务的构建 |
| thread | 工作线程池的创建和epoll配合保证了高并发 |
| utility | 日志，ini文本读写，单例等工具函数|

## 4. 高性能引擎
![半同步-半反应堆模型](https://user-images.githubusercontent.com/31312437/223939263-4d66c854-9c7c-4bc6-9853-2119f78ff372.png)
- 这是一个半同步/半反应堆模型。涉及3类线程，IO主线程，任务分发线程，工作线程池
- epoll监听服务端fd和客户端fd事件，将fd封装成任务，加入任务队列
- 加入任务队列时，任务分发线程会被唤醒，它继承了Thread,重载了run方法，会不断监听任务队列的情况，有任务时将任务分发到空闲线程，一个fd交给同一个线程处理
- 任务到来时，工作线程会被唤醒，WorkTask继承了Task,重载了run方法，会执行run里的业务逻辑

