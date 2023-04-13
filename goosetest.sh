trap 'exit 0' SIGINT

# empty read
echo "Buffer: '$(cat /dev/pa2_out)'"

count=0
while [ $count -lt 4 ]; do
        # write
        echo "Write attempt"
        echo -n "Test" > /dev/pa2_in
        count=$((count+1)) 
done

# normal read
echo "Buffer: '$(cat /dev/pa2_out)'"
# empty read after writing
echo "Buffer: '$(cat /dev/pa2_out)'"