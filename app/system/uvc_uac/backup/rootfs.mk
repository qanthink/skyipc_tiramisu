.PHONY: rootfs

LIB_DIR_PATH:=$(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)

rootfs:
	cd rootfs; tar xf rootfs.tar.gz -C $(OUTPUTDIR)
	tar xf busybox/$(BUSYBOX).tar.gz -C $(OUTPUTDIR)/rootfs
	tar xf $(LIB_DIR_PATH)/package/$(LIBC).tar.gz -C $(OUTPUTDIR)/rootfs/lib
	cp $(LIB_DIR_PATH)/mi_libs/dynamic/* $(OUTPUTDIR)/rootfs/lib/
	cp $(LIB_DIR_PATH)/ex_libs/dynamic/* $(OUTPUTDIR)/rootfs/lib/
	
	mkdir -p $(miservice$(RESOUCE))
	mkdir -p $(OUTPUTDIR)/customer
	#qanthink begin
	cp -rf ~/Workspace/skyuvc_ispahan/build/app/skyuvc $(OUTPUTDIR)/customer
	#qanthink end
	cp -rf $(PROJ_ROOT)/board/$(CHIP)/$(BOARD_NAME)/config/* $(miservice$(RESOUCE))
	cp -vf $(PROJ_ROOT)/board/$(CHIP)/mmap/$(MMAP) $(miservice$(RESOUCE))/mmap.ini
	cp -rvf $(LIB_DIR_PATH)/bin/config_tool/* $(miservice$(RESOUCE))
	cd $(miservice$(RESOUCE)); chmod +x config_tool; ln -sf config_tool dump_config; ln -sf config_tool dump_mmap
	mkdir -p $(miservice$(RESOUCE))/iqfile/
	cp -rf $(PROJ_ROOT)/board/$(CHIP)/iqfile/isp_api.xml $(miservice$(RESOUCE))/iqfile/ -vf
	if [ "$(IQ0)" != "" ]; then \
		cp -rf $(PROJ_ROOT)/board/$(CHIP)/iqfile/$(IQ0) $(miservice$(RESOUCE))/iqfile/ -vf; \
		cd $(miservice$(RESOUCE))/iqfile; chmod +x $(shell echo $(IQ0) | awk -F'/' '{print $$NF}'); ln -sf $(shell echo $(IQ0) | awk -F'/' '{print $$NF}') iqfile0.bin; cd -; \
	fi;
	if [ "$(IQ1)" != "" ]; then \
		cp -rf $(PROJ_ROOT)/board/$(CHIP)/iqfile/$(IQ1) $(miservice$(RESOUCE))/iqfile/ -vf; \
		cd $(miservice$(RESOUCE))/iqfile; chmod +x $(shell echo $(IQ1) | awk -F'/' '{print $$NF}'); ln -sf $(shell echo $(IQ1) | awk -F'/' '{print $$NF}') iqfile1.bin; cd -; \
	fi;
	if [ "$(IQ2)" != "" ]; then \
		cp -rf $(PROJ_ROOT)/board/$(CHIP)/iqfile/$(IQ2) $(miservice$(RESOUCE))/iqfile/ -vf; \
		cd $(miservice$(RESOUCE))/iqfile; chmod +x $(shell echo $(IQ2) | awk -F'/' '{print $$NF}'); ln -sf $(shell echo $(IQ2) | awk -F'/' '{print $$NF}') iqfile2.bin; cd -; \
	fi;
	if [ "$(IQ3)" != "" ]; then \
		cp -rf $(PROJ_ROOT)/board/$(CHIP)/iqfile/$(IQ3) $(miservice$(RESOUCE))/iqfile/ -vf; \
		cd $(miservice$(RESOUCE))/iqfile; chmod +x $(shell echo $(IQ3) | awk -F'/' '{print $$NF}'); ln -sf $(shell echo $(IQ3) | awk -F'/' '{print $$NF}') iqfile3.bin; cd -; \
	fi;

	if [ -d $(PROJ_ROOT)/board/$(CHIP)/venc_fw ]; then \
		cp -rf $(PROJ_ROOT)/board/$(CHIP)/venc_fw $(miservice$(RESOUCE)); \
	fi;
	mkdir -p $(OUTPUTDIR)/rootfs/config
#	cp -rf $(miservice$(RESOUCE))/* $(OUTPUTDIR)/rootfs/config
	cp -rf etc/* $(OUTPUTDIR)/rootfs/etc

	if [ "$(PHY_TEST)" = "yes" ]; then \
		mkdir $(miservice$(RESOUCE))/sata_phy ; \
		cp $(LIB_DIR_PATH)/bin/sata_phy/* $(miservice$(RESOUCE))/sata_phy ; \
	fi;

	#cp -rf $(LIB_DIR_PATH)/bin/mixer/* $(OUTPUTDIR)/customer/ ; \
	#ln -sf ./mixer/font $(OUTPUTDIR)/customer/font ; \
	#cp -rf $(LIB_DIR_PATH)/bin/mi_demo/ $(OUTPUTDIR)/customer/ ; \
	
	if [ "$(DUAL_OS)" = "on" ]; then \
		cp $(PROJ_ROOT)/board/$(CHIP)/rtos/PreloadSetting.txt $(OUTPUTDIR)/customer/ ; \
	fi;

	mkdir -p $(OUTPUTDIR)/rootfs/lib/modules/
	mkdir -p $(miservice$(RESOUCE))/modules/$(KERNEL_VERSION)

	touch ${OUTPUTDIR}/rootfs/etc/mdev.conf
	echo mice 0:0 0660 =input/ >> ${OUTPUTDIR}/rootfs/etc/mdev.conf
	echo mouse.* 0:0 0660 =input/ >> ${OUTPUTDIR}/rootfs/etc/mdev.conf
	echo event.* 0:0 0660 =input/ >> ${OUTPUTDIR}/rootfs/etc/mdev.conf
	echo pcm.* 0:0 0660 =snd/ >> ${OUTPUTDIR}/rootfs/etc/mdev.conf
	echo control.* 0:0 0660 =snd/ >> ${OUTPUTDIR}/rootfs/etc/mdev.conf
	echo timer 0:0 0660 =snd/ >> ${OUTPUTDIR}/rootfs/etc/mdev.conf
	echo '$$DEVNAME=bus/usb/([0-9]+)/([0-9]+) 0:0 0660 =bus/usb/%1/%2' >> ${OUTPUTDIR}/rootfs/etc/mdev.conf

	echo export PATH=\$$PATH:/config >> ${OUTPUTDIR}/rootfs/etc/profile
	echo export TERMINFO=/config/terminfo >> ${OUTPUTDIR}/rootfs/etc/profile
	sed -i '/^mount.*/d' $(OUTPUTDIR)/rootfs/etc/profile
	echo mkdir -p /dev/pts >> ${OUTPUTDIR}/rootfs/etc/init.d/rcS
	echo mount -t sysfs none /sys >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS
	echo mount -t tmpfs mdev /dev >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS
	echo mount -t debugfs none /sys/kernel/debug/ >>  $(OUTPUTDIR)/rootfs/etc/init.d/rcS

	cp -rvf $(PROJ_ROOT)/tools/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/fw_printenv/* $(OUTPUTDIR)/rootfs/etc/
	echo "$(ENV_CFG)" > $(OUTPUTDIR)/rootfs/etc/fw_env.config
	cd $(OUTPUTDIR)/rootfs/etc/;ln -sf fw_printenv fw_setenv
	echo mkdir -p /var/lock >> ${OUTPUTDIR}/rootfs/etc/init.d/rcS

	if [ "$(FLASH_TYPE)"x = "nor"x  ]; then \
		echo mdev -s >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS ;\
	fi;
	echo -e $(foreach block, $(USR_MOUNT_BLOCKS), "mount -t $($(block)$(FSTYPE)) $($(block)$(MOUNTPT)) $($(block)$(MOUNTTG))\n") >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS

	-chmod 755 $(LIB_DIR_PATH)/bin/debug/*
	#cp -rf $(LIB_DIR_PATH)/bin/debug/* $(OUTPUTDIR)/customer/
	#add:  remove sshd in nor flash default
	if [[ "$(FLASH_TYPE)"x = "nor"x ]] && [[ -d "$(OUTPUTDIR)/customer/ssh" ]]; then \
		rm -rf $(OUTPUTDIR)/customer/ssh; \
	fi;
	#end add

	if [ -f "$(OUTPUTDIR)/customer/demo.sh" ]; then \
		rm  $(OUTPUTDIR)/customer/demo.sh; \
	fi;
	touch $(OUTPUTDIR)/customer/demo.sh
	chmod 755 $(OUTPUTDIR)/customer/demo.sh


	# creat insmod ko scrpit
	if [ -f "$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/$(CHIP)/configs/$(PRODUCT)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(FLASH_TYPE)/modules/kernel_mod_list" ]; then \
		cat $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/$(CHIP)/configs/$(PRODUCT)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(FLASH_TYPE)/modules/kernel_mod_list | sed 's#\(.*\).ko#insmod /config/modules/$(KERNEL_VERSION)/\1.ko#' > $(OUTPUTDIR)/customer/demo.sh; \
		cat $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/$(CHIP)/configs/$(PRODUCT)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(FLASH_TYPE)/modules/kernel_mod_list | sed 's#\(.*\).ko\(.*\)#$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/$(CHIP)/configs/$(PRODUCT)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(FLASH_TYPE)/modules/\1.ko#' | xargs -i cp -rvf {} $(miservice$(RESOUCE))/modules/$(KERNEL_VERSION); \
		echo "#kernel_mod_list" >> $(OUTPUTDIR)/customer/demo.sh; \
	fi;

	if [ "$(DUAL_OS)" != "on" ]; then \
		if [ -f "$(LIB_DIR_PATH)/modules/$(KERNEL_VERSION)/misc_mod_list" ]; then \
			cat $(LIB_DIR_PATH)/modules/$(KERNEL_VERSION)/misc_mod_list | sed 's#\(.*\).ko#insmod /config/modules/$(KERNEL_VERSION)/\1.ko#' >> $(OUTPUTDIR)/customer/demo.sh; \
			cat $(LIB_DIR_PATH)/modules/$(KERNEL_VERSION)/misc_mod_list | sed 's#\(.*\).ko\(.*\)#$(LIB_DIR_PATH)/modules/$(KERNEL_VERSION)/\1.ko#' | xargs -i cp -rvf {} $(miservice$(RESOUCE))/modules/$(KERNEL_VERSION); \
			echo "#misc_mod_list" >> $(OUTPUTDIR)/customer/demo.sh; \
		fi; \
		if [ -f "$(LIB_DIR_PATH)/modules/$(KERNEL_VERSION)/.mods_depend" ]; then \
			cat $(LIB_DIR_PATH)/modules/$(KERNEL_VERSION)/.mods_depend | sed '2,20s#\(.*\)#insmod /config/modules/$(KERNEL_VERSION)/\1.ko#' >> $(OUTPUTDIR)/customer/demo.sh; \
			cat $(LIB_DIR_PATH)/modules/$(KERNEL_VERSION)/.mods_depend | sed 's#\(.*\)#$(LIB_DIR_PATH)/modules/$(KERNEL_VERSION)/\1.ko#' | xargs -i cp -rvf {} $(miservice$(RESOUCE))/modules/$(KERNEL_VERSION); \
			echo "#mi module" >> $(OUTPUTDIR)/customer/demo.sh; \
		fi; \
		if [ -f "$(LIB_DIR_PATH)//modules/$(KERNEL_VERSION)/misc_mod_list_late" ]; then \
			cat $(LIB_DIR_PATH)/modules/$(KERNEL_VERSION)/misc_mod_list_late | sed 's#\(.*\).ko#insmod /config/modules/$(KERNEL_VERSION)/\1.ko#' >> $(OUTPUTDIR)/customer/demo.sh; \
			cat $(LIB_DIR_PATH)/modules/$(KERNEL_VERSION)/misc_mod_list_late | sed 's#\(.*\).ko\(.*\)#$(LIB_DIR_PATH)/modules/$(KERNEL_VERSION)/\1.ko#' | xargs -i cp -rvf {} $(miservice$(RESOUCE))/modules/$(KERNEL_VERSION); \
			echo "#misc_mod_list_late" >> $(OUTPUTDIR)/customer/demo.sh; \
		fi; \
	fi;

	if [ -f "$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/$(CHIP)/configs/$(PRODUCT)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(FLASH_TYPE)/modules/kernel_mod_list_late" ]; then \
		cat $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/$(CHIP)/configs/$(PRODUCT)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(FLASH_TYPE)/modules/kernel_mod_list_late | sed 's#\(.*\).ko#insmod /config/modules/$(KERNEL_VERSION)/\1.ko#' >> $(OUTPUTDIR)/customer/demo.sh; \
		cat $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/$(CHIP)/configs/$(PRODUCT)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(FLASH_TYPE)/modules/kernel_mod_list_late | sed 's#\(.*\).ko\(.*\)#$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/$(CHIP)/configs/$(PRODUCT)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(FLASH_TYPE)/modules/\1.ko#' | xargs -i cp -rvf {} $(miservice$(RESOUCE))/modules/$(KERNEL_VERSION); \
		echo "#kernel_mod_list_late" >> $(OUTPUTDIR)/customer/demo.sh; \
	fi;

	#qanthink begin
	ls $(miservice$(RESOUCE))/modules//$(KERNEL_VERSION)
	#qanthink end

	if [ "$(DUAL_OS)" != "on" ]; then \
		if [ "$(SENSOR_LIST)" != "" ]; then \
			cp -rvf $(foreach n,$(SENSOR_LIST),$(LIB_DIR_PATH)/modules/$(KERNEL_VERSION)/$(n)) $(miservice$(RESOUCE))/modules/$(KERNEL_VERSION); \
		fi; \
		if [ "$(SENSOR0)" != "" ]; then \
			echo insmod /config/modules/$(KERNEL_VERSION)/$(SENSOR0) $(SENSOR0_OPT) >> $(OUTPUTDIR)/customer/demo.sh; \
		fi; \
		if [ "$(SENSOR1)" != "" ]; then \
			echo insmod /config/modules/$(KERNEL_VERSION)/$(SENSOR1) $(SENSOR1_OPT) >> $(OUTPUTDIR)/customer/demo.sh; \
		fi; \
		if [ "$(SENSOR2)" != "" ]; then \
			echo insmod /config/modules/$(KERNEL_VERSION)/$(SENSOR2) $(SENSOR2_OPT) >> $(OUTPUTDIR)/customer/demo.sh; \
		fi;	\
		sed -i 's/mi_sys.ko/mi_sys.ko cmdQBufSize=256 logBufSize=256/g' $(OUTPUTDIR)/customer/demo.sh; \
		sed -i '/mi_iqserver.ko/,+4d' $(OUTPUTDIR)/customer/demo.sh;\
		sed -i '/mi_isp.ko/,+4d' $(OUTPUTDIR)/customer/demo.sh;\
		sed -i 's/mi_common/insmod \/config\/modules\/$(KERNEL_VERSION)\/mi_common.ko/g' $(OUTPUTDIR)/customer/demo.sh; \
		sed -i '/#mi module/a	major=`cat /proc/devices | busybox awk "\\\\$$2==\\""mi_poll"\\" {print \\\\$$1}"`\nbusybox mknod \/dev\/mi_poll c $$major 0' $(OUTPUTDIR)/customer/demo.sh; \
	fi;

	if [ $(PHY_TEST) = "yes" ]; then \
		echo -e "\033[41;33;5m !!! Replace "mdrv-sata-host.ko" with "sata_bench_test.ko" !!!\033[0m" ; \
		sed '/mdrv-sata-host/d' $(OUTPUTDIR)/customer/demo.sh > $(OUTPUTDIR)/customer/demotemp.sh ; \
		echo insmod /config/sata_phy/sata_bench_test.ko >> $(OUTPUTDIR)/customer/demotemp.sh ; \
		cp $(OUTPUTDIR)/customer/demotemp.sh $(OUTPUTDIR)/customer/demo.sh ; \
		rm $(OUTPUTDIR)/customer/demotemp.sh ; \
	fi;
	echo mdev -s >> $(OUTPUTDIR)/customer/demo.sh
	#qanthink begin
	echo >> $(OUTPUTDIR)/customer/demo.sh
	echo >> $(OUTPUTDIR)/customer/demo.sh
	echo ifconfig eth0 up >> $(OUTPUTDIR)/customer/demo.sh
	echo ifconfig eth0 192.168.0.100 >> $(OUTPUTDIR)/customer/demo.sh
	echo ifconfig lo up >> $(OUTPUTDIR)/customer/demo.sh
	echo route add default gw 192.168.0.1 eth0 >> $(OUTPUTDIR)/customer/demo.sh
	echo >> $(OUTPUTDIR)/customer/demo.sh
	echo \#mount -o nolock 192.168.0.7:/home  /mnt >> $(OUTPUTDIR)/customer/demo.sh
	echo >> $(OUTPUTDIR)/customer/demo.sh
	echo cd /customer >> $(OUTPUTDIR)/customer/demo.sh
	echo ./skyuvc >> $(OUTPUTDIR)/customer/demo.sh
	#qanthink end
	if [ $(BENCH) = "yes" ]; then \
		cd $(PROJ_ROOT)/../bench ; \
        ./prepare_bench_i6b0_qfn88_64M.sh ; \
	fi;
	if [ $(BENCH) = "yes" ]; then \
		mkdir $(OUTPUTDIR)/customer/bench ; \
		cp -rf $(PROJ_ROOT)/../bench/output/* $(OUTPUTDIR)/customer/bench/ ; \
		echo "cd /customer/bench/;./RunUnittest.sh" >> $(OUTPUTDIR)/customer/demo.sh ; \
        echo "export PATH=\$$PATH:/customer" >> $(OUTPUTDIR)/rootfs/etc/profile ; \
	fi;

	# Enable MIU protect on CMDQ buffer as default (While List: CPU)
	# [I5] The 1st 1MB of MIU is not for CMDQ buffer
#	echo 'echo set_miu_block3_status 0 0x70 0 0x100000 1 > /proc/mi_modules/mi_sys_mma/miu_protect' >> $(OUTPUTDIR)/customer/demo.sh

#	echo mount -t jffs2 /dev/mtdblock3 /config >> $(OUTPUTDIR)/rootfs/etc/profile
	ln -fs /config/modules/$(KERNEL_VERSION) $(OUTPUTDIR)/rootfs/lib/modules/
	find $(miservice$(RESOUCE))/modules/$(KERNEL_VERSION) -name "*.ko" | xargs $(TOOLCHAIN_REL)strip  --strip-unneeded;
	find $(OUTPUTDIR)/rootfs/lib/ -name "*.so" | xargs $(TOOLCHAIN_REL)strip  --strip-unneeded;
	echo mkdir -p /dev/pts >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS
	echo mount -t devpts devpts /dev/pts >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS
	echo "busybox telnetd&" >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS
	echo "if [ -e /customer/demo.sh ]; then" >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS
	echo "    /customer/demo.sh" >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS
	echo "fi;" >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS
	#add sshd, default password 1234
	if [[ "$(FLASH_TYPE)"x = "spinand"x ]]; then \
		if [ $(TOOLCHAIN_VERSION) = "8.2.1" ]; then \
			echo "root:5fXKAeftHX95A:0:0:Linux User,,,:/home/root:/bin/sh" > $(OUTPUTDIR)/rootfs/etc/passwd; \
			echo "sshd:x:74:74:Privilege-separated SSH:/var/empty/sshd:/sbin/nologin" >> $(OUTPUTDIR)/rootfs/etc/passwd; \
			echo "export LD_LIBRARY_PATH=\$$LD_LIBRARY_PATH:/customer/ssh/lib" >> ${OUTPUTDIR}/rootfs/etc/init.d/rcS; \
			echo "mkdir /var/empty" >> ${OUTPUTDIR}/rootfs/etc/init.d/rcS; \
			echo "/customer/ssh/sbin/sshd -f /customer/ssh/etc/sshd_config" >> ${OUTPUTDIR}/rootfs/etc/init.d/rcS; \
			echo "export LD_LIBRARY_PATH=\$$LD_LIBRARY_PATH:/customer/ssh/lib" >> ${OUTPUTDIR}/rootfs/etc/profile; \
		fi; \
	fi;
	#end add

	mkdir -p $(OUTPUTDIR)/vendor
	mkdir -p $(OUTPUTDIR)/customer
	mkdir -p $(OUTPUTDIR)/rootfs/vendor
	mkdir -p $(OUTPUTDIR)/rootfs/customer
	mkdir -p $(OUTPUTDIR)/bootconfig
	mkdir -p $(OUTPUTDIR)/rootfs/bootconfig
