# nfs

### 1. nfs-server

sudo apt-get install nfs-kernel-server rpcbind

创建nfs传输文件夹nfs-xxx
mkdir nfs-xxx
chmod 777 nfs-xxx

配置nfs
sudo vim /etc/exports
打开后添加如下内容
nfs-xxx *(rw,sync,no_subtree_check)
//nfs-xxx 192.168.1.0/24(rw,sync,no_subtree_check)

重启nfs服务
sudo /etc/init.d/nfs-kernel-server restart
