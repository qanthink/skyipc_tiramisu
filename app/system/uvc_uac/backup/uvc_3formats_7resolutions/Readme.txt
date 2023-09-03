1.芯片：SSD268G和SSC8836Q

2.图像格式3种：MJPEG、H.264、H.265

3.分辨率7种：
240P
480P
730P
1080P
2KP
4KP
48MP 7200×5400, vif depth = 2
setenv bootargs console=ttyS0,115200 root=/dev/mtdblock2 rootfstype=squashfs ro init=/linuxrc LX_MEM=0xffe0000 mma_heap=mma_heap_name0,miu=0,sz=0x6300000 mma_memblock_remove=1 cma=2M@0x23800000




