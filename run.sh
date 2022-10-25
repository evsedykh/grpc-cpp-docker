#!/bin/bash

for shard in {1..4}
do
  NODE_ROLE=shard SHARD_NUM=$shard ./dispersion_node &
done

NODE_ROLE=master ./dispersion_node


