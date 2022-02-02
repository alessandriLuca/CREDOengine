echo "relax now for 30 seconds"
nohup code-server --auth none &
sleep 30
pkill node

