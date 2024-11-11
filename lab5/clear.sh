#!/bin/bash

# 获取所有消息队列的 ID，并将每个 ID 存储在数组中
msg_ids=$(ipcs -q | awk 'NR>3 {print $2}')

# 判断是否有消息队列
if [ -z "$msg_ids" ]; then
  echo "没有找到消息队列。"
else
  # 遍历每个消息队列 ID 并删除
  for id in $msg_ids; do
    ipcrm -q $id
    echo "删除消息队列 ID: $id"
  done
  echo "所有消息队列已删除。"
fi
