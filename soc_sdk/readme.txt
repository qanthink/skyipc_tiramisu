# soc_sdk是芯片底层的SDK, 由芯片原厂提供。
# 此处包含头文件和库文件。

# 当前版本tiramisuV012_20220606_usb

When update sdk, run:

rm soc_sdk/include/* soc_sdk/dynamic/* -rf

cp ~/Downloads/tiramisuV012_202206_usbcam/sourcecode/project/release/include/* soc_sdk/include/ -r

rm soc_sdk/include/mi_isp_iq_datatype.h
rm soc_sdk/include/mi_isp_hw_dep_datatype.h
rm soc_sdk/include/mi_ipu_datatype.h

cp ~/Downloads/tiramisuV012_202206_usbcam/sourcecode/project/release/include/ipu/tiramisu/mi_ipu_datatype.h soc_sdk/include/
cp ~/Downloads/tiramisuV012_202206_usbcam/sourcecode/project/release/include/mi_isp_hw_dep_datatype.h soc_sdk/include/
cp ~/Downloads/tiramisuV012_202206_usbcam/sourcecode/project/release/include/mi_isp_iq_datatype.h soc_sdk/include/

cp ~/Downloads/tiramisuV012_202206_usbcam/sourcecode/project/image/output/rootfs/lib/* soc_sdk/dynamic/ -r
cp ~/Downloads/tiramisuV012_202206_usbcam/sourcecode/project/image/output/miservice/config/lib/* soc_sdk/dynamic/ -r
