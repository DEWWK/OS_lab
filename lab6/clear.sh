#!/bin/bash

# 列出当前系统中的所有信号量
semaphores=$(ipcs -s | awk 'NR>3 {print $2}')  # 获取信号量 ID，忽略前两行标题

if [ -z "$semaphores" ]; then
  echo "没有找到任何信号量。"
  exit 0
fi

echo "找到以下信号量，将删除它们："
echo "$semaphores"

# 删除信号量
for sem_id in $semaphores; do
  echo "删除信号量: $sem_id"
  ipcrm -s $sem_id
done

echo "所有信号量已删除。"
