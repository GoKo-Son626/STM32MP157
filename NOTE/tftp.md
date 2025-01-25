# tftp

### 1. tftp-server
```
sudo apt-get install tftp-hpa tftpd-hpa
sudo apt-get install xinetd

创建文件夹并设置权限存放tftp传输文件
mkdir tftp_xxx
chmod 777 tftp_xxx

配置文件(/etc/xinetd.d/tftp)
server tftp
{
         socket_type    = dgram
         protocol       = udp
         wait           = yes
         user           = root
         server         = /usr/sbin/in.tftpd
         server_args    = -s tftp_xxx
         disable        = no
         per_source     = 11
         cps            = 100 2
         flags          = IPv4
 }

启动tftp服务
sudo service tftpd-hpa start
修改/etc/default/tftpd-hpa服务
# /etc/default/tftpd-hpa

TFTP_USERNAME="tftp"
TFTP_DIRECTORY="tftp_xxx"
TFTP_ADDRESS=":69"
TFTP_OPTIONS="-l -c -s"
重启tftp服务
sudo service tftpd-hpa restart

传输文件
cp xx.c tftp_xxx
chmod 777 xx.c

