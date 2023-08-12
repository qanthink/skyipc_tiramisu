aux_source_directory(. DIR_APP_SRC)

add_library(live555rtsp ${DIR_APP_SRC})

include_directories(
)

# 注意3点：1.live555依赖openssl; 2.openssl的依赖顺序；
# 3.live555的链接顺序为：liveMedia BasicUsageEnvironment groupsock UsageEnvironment
target_link_libraries(live555rtsp 
	liveMedia BasicUsageEnvironment groupsock UsageEnvironment
	ssl crypto
)

