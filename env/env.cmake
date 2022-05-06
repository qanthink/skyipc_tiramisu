############################ 说明 ###############################
#
#	用于配置编译器、编译环境。
#
#################################################################
#设置C 和C++ 编译选项
set(CMAKE_CXX_FLAGS	"${CMAKE_CXX_FLAGS} -std=c++14 -Wno-pointer-arith")
set(CMAKE_C_FLAGS	"${CMAKE_C_FLAGS}")

#设置系统平台
set(CMAKE_SYSTEM_NAME Linux)

#设置toolchain
SET(ToolType	"glibc9.1.0")

#设置编译器路径和前缀
if(${ToolType} STREQUAL "uclibc")			#设置uclibc的路径  
message("ToolChain: uclibc")				#打印信息
SET(ToolPath	"/home/linux/tools/toolchain/arm-buildroot-linux-uclibcgnueabihf-4.9.4-uclibc-1.0.31/bin")
SET(ToolPrefix	"arm-buildroot-linux-uclibcgnueabihf-")

elseif(${ToolType} STREQUAL "glibc4.8.3")	#设置glibc的路径
message("ToolChain: glibc4.8.3")			#打印信息
SET(ToolPath	"/home/linux/tools/toolchain/arm-linux-gnueabihf-4.8.3-201404/bin")
SET(ToolPrefix	"arm-linux-gnueabihf-")

elseif(${ToolType} STREQUAL "glibc9.1.0")	#设置glibc的路径
message("ToolChain: glibc9.1.0")			#打印信息
SET(ToolPath	"/home/linux/tools/toolchain/gcc-sigmastar-9.1.0-2020.07-x86_64_arm-linux-gnueabihf/bin")
SET(ToolPrefix	"arm-linux-gnueabihf-")
endif()

SET(CMAKE_C_COMPILER  	${ToolPath}/${ToolPrefix}gcc)
SET(CMAKE_CXX_COMPILER 	${ToolPath}/${ToolPrefix}g++)
SET(CMAKE_STRIP 		${ToolPath}/${ToolPrefix}strip)
