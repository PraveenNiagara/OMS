新增修改点：
preboot.bin
1、解决1603平台在110基线上gpio8在模块初始化阶段有两个状态(插上USB开机和不插USB开机)
具体现象为模块不插USB时，上电拉低powerkey后，状态灯直接拉高，插上USB时，拉低powerkey后，模块初始化完成后才拉高
修改后：不插USB时，拉低powerkey状态灯会闪一下，模块初始化完成后才拉高    
2、解决preboot在gcc编译失败
                                                                                                                    -----2023/10/12

PR_A7670-2653,升级基线后更新preboot，包括A7678-20089 中对大陆Continental 的定制

							-----2024/6/19

3、升级新基线后现在preboot.bin文件默认使用的是ASR原始SDK里的preboot.bin，之前第一点的修改没有生效；
对比了下原始SDK中preboot的main函数，差异只有第一点的修改和客户定制
客户定制使用的是单独编译的preboot文件，所以通用的preboot.bin文件需要重新编译，替换
                                                                                                                    -----2024/11/11