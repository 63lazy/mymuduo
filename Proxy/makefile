# Makefile for LB_Server
# 编译器设置
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g

# 链接库
LIBS = -lmymuduo -lpthread

# 源文件
SRCS = LB_Server.cc RemoteClient.cc Selector.cc

# 目标文件
OBJS = $(SRCS:.cc=.o)

# 目标可执行文件
TARGET = LB_Server

# 默认目标
all: $(TARGET)

# 链接目标文件生成可执行文件
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)
	rm -f $(OBJS)
# 编译源文件生成目标文件
%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理目标
clean:
	rm -f $(OBJS) $(TARGET)

# 伪目标
.PHONY: all clean