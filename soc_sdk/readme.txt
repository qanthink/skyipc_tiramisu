# soc_sdk是芯片底层的SDK, 由芯片原厂提供。
# 此处包含头文件和库文件。

# 当前版本tiramisuV012_20220606

When update sdk, run:

rm include/* -rf
rm dynamic/* -rf

cp ~/Downloads/tiramisuV012_202206_usbcam/sourcecode/project/release/include/* ~/Workspace/skyipc_tiramisu/soc_sdk/include/ -r

cp ~/Downloads/tiramisuV012_202206_usbcam/sourcecode/project/image/output/rootfs/lib/* ~/Workspace/skyipc_tiramisu/soc_sdk/dynamic/ -r
cp ~/Downloads/tiramisuV012_202206_usbcam/sourcecode/project/image/output/miservice/config/lib/* ~/Workspace/skyipc_tiramisu/soc_sdk/dynamic/ -r


