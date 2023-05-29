#! /usr/bin/env bash

# sudo cp .. ~/MP4 -r
# cd ~/MP4/mp4
# pwd
docker run -it --rm -v $(pwd)/xv6/:/home/xv6  ntuos/mp4
# docker exec -it mp4 bash